/*****************************  Pager.c  *************************************** 

 * This module implements the VM-I/O support level TLB exception handler; the pager 
 * 
 * Helper functions: 
 * 		pickAVictim: Select the frame to be used for page fault. Uses the "oldest
 * 			page first" frame selection algorithm.
 * 		
 * 
 ****************************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/initProc.e"
#include "../e/initial.e"
#include "../e/asl.e"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/interrupts.e"
#include "../e/sysSupport.e"
#include "../e/pager.e"
#include "/usr/local/include/umps2/umps/libumps.e"


/*******************Main Function***********************/

void pager(){
	
	/*local vars*/
	int missingPage, segment,framePick, pageNumber, status, asid, cause, command, pageLocation, loserProc;
	state_t * uProc;
	
	devregarea_t* deviceData = (devregarea_t *) RAMBASEADDR;
	device_t *disk = (device_t*) DISKDEV;
	
	/*Who am I?*/
	asid =  getENTRYHI();
	asid = (asid & MASKBIT) >> 6;
	
	/*Why am I here?*/
	uProc = &(procs[asid-1].Told_trap[SYSTRAP]);
	cause = ((uProc->s_cause) & 0xFC) >> 2;
	
	if(cause != TLBL && cause !=TLBS) /*If cause isn't "tlb invalid" exception, then kill the proc*/
	{
		SYSCALL(TERMINATEPROCESS, 0, 0, 0);
	}
	
	/*Select a frame to be used for this page fault*/
    framePick = pickAVictim();
	
	/*Find seg # and page #s*/
	segment = uProc->s_asid >> 30; 
	missingPage = (uProc->s_asid & PAGEMASK) >> 12;
	pageNumber = swapTables[framePick].sw_pageNo;
	
	/*Perform P on swapSem (Gain mutual exclusion)*/
	SYSCALL(PASSEREN, (int)&swapSem, 0, 0);
	
	/*Check if 'missingPage' & pageNumer is the stack page and change it if it is.*/
	if(missingPage >= KUSEGPTESIZE ) {
		missingPage = KUSEGPTESIZE -1;
	}
	
	if(pageNumber >=KUSEGPTESIZE){
		pageNumber = KUSEGPTESIZE-1;
	}
	
	/*Note: If we had a KUSeg3, we would have to worry if the page is still missing and release mutual exclusion
	 * if it wasn't but the page should always stay missing because we do not have a KUSeg3 to make things more complicated...*/
	
    /*RAMTOP minus 3 stacks and the framepick area that we want*/
    pageLocation = (((deviceData-> rambase) + (deviceData->ramsize)) - (PAGESIZE * 3)) - (framePick * PAGESIZE);
    
    /*get the asid of the proc being removed*/
    loserProc = swapTables[framePick].sw_asid;
    
	 /*If the frame is occupied,, mark the PTE entry as invalid*/
	 if(swapTables[framePick].sw_asid != -1) {

		//interrupts off 
		status = getSTATUS();
		setSTATUS(ALLOFF);                                          
		
		/*Mark the PTE entry*/
        swapTables[framePick].sw_pte -> pte_entryLO = ALLOFF | DIRTYON;
        swapTables[framePick].sw_asid = -1;
        swapTables[framePick].sw_pageNo = 0;
        swapTables[framePick].sw_segNo = 0;
        swapTables[framePick].sw_pte = NULL;
        TLBCLR();
		
		/* interrupts on*/
		setSTATUS(status);
		
		/*Write the frame out to the backing store device*/	
		SYSCALL(PASSEREN, (int)&diskSem,0,0); /*Mutual exclusion for the disk write*/
		
		command = (pageNumber << 8) | SEEKCYL; /*Get cylinder*/
		
		status = getSTATUS(); /*Interrupts off*/
		setSTATUS(ALLOFF);
		
		disk->d_command = command;
		
		int waitStatus = SYSCALL(WAITIO, DISKINT, 0,0); /*wait*/
		
		setSTATUS(status); /*Interrupts on*/
		
		if(waitStatus !=READY)
		{
			SYSCALL(TERMINATE,0,0,0);
		}
		
		disk->d_data0 = pageLocation; /*Write it */
		command = command = (((HEADLOC << 8) | loserProc-1) << 8) | WRITEBLK;
		
		status = getSTATUS(); /*Interrupts off...again?*/
		setSTATUS(ALLOFF);
		
		disk->d_command = command;
		
		waitStatus = SYSCALL(WAITIO, DISKINT,0,0);
		
		setSTATUS(status);
		
		if(waitStatus !=READY)
		{
			SYSCALL(TERMINATE,0,0,0);
		}
		
		SYSCALL(VERHOGEN, (int)&diskSem,0,0); /*release the disk sema4*/
		
	}
	
	/*Read in the missing page from the backing store device*/
	SYSCALL(PASSEREN, (int)&diskSem,0,0); /*Mutual exclusion for the disk read*/
	
	command = (missingPage << 8) | SEEKCYL; /*Get cylinder*/
	
	status = getSTATUS(); /*Interrupts off*/
	setSTATUS(ALLOFF);
	
	disk->d_command = command;
	
	int waitStatus = SYSCALL(WAITIO, DISKINT, 0,0); /*wait*/
	
	setSTATUS(status); /*Interrupts on*/
	
	if(waitStatus !=READY)
	{
		SYSCALL(TERMINATE,0,0,0);
	}
	
	disk->d_data0 = pageLocation; /*Write it */
	command = command = (((HEADLOC << 8) | asid-1) << 8) | READBLK;
	
	status = getSTATUS(); /*Interrupts off...again?*/
	setSTATUS(ALLOFF);
	
	disk->d_command = command;
	
	waitStatus = SYSCALL(WAITIO, DISKINT,0,0);
	
	setSTATUS(status);
	
	if(waitStatus !=READY)
	{
		SYSCALL(TERMINATE,0,0,0);
	}
	
	SYSCALL(VERHOGEN, (int)&diskSem,0,0); /*release the disk sema4*/
	
	
	/*Update the PTE to reflect that the page is now sitting in RAM*/
    swapTables[framePick].sw_asid = asid;
    swapTables[framePick].sw_pageNo = missingPage;
    swapTables[framePick].sw_segNo = segment;
    swapTables[framePick].sw_pte = &(procs[asid-1].Tp_pte.pteTable[missingPage]);
    swapTables[framePick].sw_pte->pte_entryLO = (pageLocation & 0xFFFFF000) | VALIDON | DIRTYON;
    
    TLBCLR();
    
    /*V the swap sema4 */
    SYSCALL(VERHOGEN, (int)&swapSem, 0, 0);
    
    /*Load the uproc */
    LDST(uProc); 
}

/*******************Helper Functions********************/

/*Select a frame to be used for this page fault. The oldest page first frame selection algorithm is used*/
int pickAVictim()
{
	static int victim = 0;
	
	victim = victim + 1;
	
	return victim;
	
}
