/*****************************  SysSupport.c  ********************************* 
 * This module implements the VM-IO support level sys/BP and pgmTrap exception handlers.
 * Occurs: When a Syscall or Breakpoint assembler instuction is executed.
 * Executes some instruction based on the value of 9-18 found in a[0]	
 * 
 ****************************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/initProc.e"
#include "../e/initial.e"
#include "../e/sysSupport.e"
#include "../e/asl.e"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/interrupts.e"
#include "/usr/local/include/umps2/umps/libumps.e"


/****************Module global variables****************/
int asid;
state_t * uProc;


/*******************Main Function***********************/

/*Based on value in the calling uproc's a0, call the correct action*/
void handleSyscall(){
	
	/*set up global vars*/
	asid =  getENTRYHI();
	asid = (asid & MASKBIT) >> 6;
	
	uProc = &(procs[asid-1].Told_trap[SYSTRAP]);
	
	switch (uProc->s_a0){	

		case (WRITETERMINAL): 		

			writeTerminal(); 

		break;	

		case (WRITEPRINTER):

			writePrinter();

		break;
		
		case (GETTOD):
			
			getTOD();
		
		case (TERMINATE):
		
			terminate();
		
		break;
		
		default:
		
			terminate(); /*Kill the ones we don't have a syscall set up for*/
				
	}

}

/*Syscall 10 causes the uProc to be suspended until a line of output has been transmitted to the
 * terminal device associated with that uProc. If all characters transmit successfully, the number
 * of chars transmitted is put into v0, o.w the negative status value is put in v0*/
void writeTerminal(){
	
	/*local vars*/
	int i;
	int command;
	int status;
	int oldStatus;
	
	device_t* terminal;
	
	int len = (int) uProc->s_a2;
	char *str = (char *) uProc->s_a1;
	
	/*Get the device register for the terminal we will write to */
	terminal = (device_t*) TERMDEV + ((asid-1) * DEVREGSIZE);
	
	/*It is an error to write to a terminal device from an address in ksegOS, request a SYS10 with a length
	 * less than 0, or a length greater than 128. Any of these errors result in a terminate.*/
	if(len < 0 || len > 128 || str <= (char*) OSEND) {
		
		terminate();
		
	}

	/*For each character give the transmit command to the terminal*/
	for (i = 0; i < len; i++){
		
		command = (str[i] << 8) | TRANSMITCHAR; /*page 47 yellow*/
		
		oldStatus = getSTATUS(); /*turn off interrupts*/
		setSTATUS(ALLOFF);
		
		terminal->t_transm_command = command;
		status = SYSCALL (WAITIO, TERMINT, asid-1, 0);
		
		setSTATUS(oldStatus);
		
		/*if terminal status is not 5 (character transmitted), negate for v0*/
		if((status &  0x0F) != CHARTRANS)
		{
			i = i * -1; 
		}
	}
	
	uProc->s_v0 = i;
	loadState(uProc);
	
}

/*Syscall 16 causes the uProc to be suspended until a line of output has been transmitted to the
 * printer device associated with that uProc. If all characters transmit successfully, the number
 * of chars transmitted is put into v0, o.w the negative status value is put in v0*/
void writePrinter(){
	
	/*local vars*/
	int i;
	int status;
	
	device_t* printer;
	
	int len = (int) uProc->s_a2;
	char *str = (char *) uProc->s_a1;
	
	/*Get the device register for the printer we will write to */
	printer = (device_t*) PRNTDEV + ((asid-1) * DEVREGSIZE);
	
	/*It is an error to write to a printer device from an address in ksegOS, request a SYS10 with a length
	 * less than 0, or a length greater than 128. Any of these errors result in a terminate.*/
	if(len < 0 || len > 128 || str <= (char*) OSEND) {
		
		terminate();
		
	}

	/*For each character give the transmit command to the terminal*/
	for (i = 0; i < len; i++){
		
		/*Give command and load character into data0*/
		printer->d_data0 = str[i];
		printer->d_command = PRINTCHR;	
		
		status = SYSCALL (WAITIO, PRNTINT, asid-1, 0); /*wait*/
		
		/*if device status is not 1 (ready), negate for v0*/
		if(status != READY)
		{
			i = i * -1; 
		}
	}
	
	uProc->s_v0 = i;
	loadState(uProc);
	
}

/*Syscall 17 places the number of microseconds in uproc's v0 since the system was last rebooted.*/
void getTOD()
{
	cpu_t time;
	
	STCK(time);
	uProc->s_v0 = time;
	
	loadState(uProc);
}
/*Syscall 18 causes the executing U-proc to cease to exist*/
void terminate(){
	
	TLBCLR(); /*nuke page table*/
	
	SYSCALL(TERMINATEPROCESS,0,0,0); /*nuke the process*/
	
}
