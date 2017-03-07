/*****************************  Interrupts.c  *************************************** 
 * Occurs: When either a previously initiated I/O request completes or when either a processor Local
 * 		Timer/Interval Timer makes a 0x0000.0000 -> 0xFFFF.FFFF transition. 
 * 
 * Execute actions:
 * 1) Acknowledge the outstanding interrupt by writing acknowledge command code or a new command in the 
 * 		interrupting device's  device register.
 * 2) Perform V operation on the sema4 associated with the interrupting (sub)device and pseudo-clock sema4.
 * 3) If SYS8was requested prior to the handling of this interrupt, store the interrupting (sub)device's 
 * 		status word in the newly unblocked process'es v0. Alternatively if SYS8 has not been called, 
 * 		store off the word until SYS8 is eventually called.
 * 4) Note that time spent in I/O Handler is handled by storing off the TOD clock beofre interrupt
 * 		and then putting that time spent in I/O Handler back on
 * 
 * Helper Functions:
 * 		findDevLine: Finds what line interrupt is on.
 * 		findDev: Finds what device on the line the interrupt is on.	
 * 		handleTerminalLine: Handle the special case of the terminal line (7) where
 * 			the interrupt could be for recevie or transmit.
 * 		handleClockLines: Handles the special case of the two clock lines (1 & 2) 
 * 			depending on what type of clock it is. */

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../e/scheduler.e"
#include "../e/initial.e"
#include "../e/interrupts.e"
#include "../e/exceptions.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/****************Module global variables****************/

cpu_t endTOD; /*Used to restore time spent in I/O Handler*/

HIDDEN state_t *oldINT = (state_t *) OLDINTERRUPT; /*Old interrupt state area*/

HIDDEN int termReceive = 0; /* set boolean value for if a line is a terminal w/ status recieve*/
HIDDEN devregarea_t *devregarea = (devregarea_t *) ADDRESS1; /* declare and initialize device area*/

/*******************Helper Functions********************/

/*find the line that the interrupt is on by taking the old cause addr and 'AND'ing it w/ the line addreses (defined by cause bytes 8-15)
 * Lines:
 * 		0 - multi core
 * 		1 & 2 - clocks
 * 		3 - disk device
 * 		4 - tape device
 * 		5 - network device
 * 		6 - printer device
 * 		7 - terminal device
*/
int findDevLine(memaddr causeAddr){
	
	if((causeAddr & LINEADDR_1) !=0){
		return 1;
	}
	else if((causeAddr & LINEADDR_2) !=0){
		return 2;
	}
	else if((causeAddr & LINEADDR_3) !=0){
		return 3;
	}
	else if((causeAddr & LINEADDR_4) !=0){
		return 4;
	}
	else if((causeAddr & LINEADDR_5) !=0){
		return 5;
	}
	else if((causeAddr & LINEADDR_6) !=0){
		return 6;
	}
	else if((causeAddr & LINEADDR_7) !=0){
		return 7;
	}
	else{									
		return 0; 								
	}
}

/*find the device on a specific line that the interrupt is on by getting the word based on the line number and then'AND'ing it w/ the device addreses*/
int findDev(int lineNo){
	
	memaddr intrAddr;
		
	/*Get the interrupt addr*/
	if(lineNo == 3){
		intrAddr = WORD_0;
	}
	else if(lineNo == 4){
		intrAddr = WORD_1;
	}
	else if(lineNo == 5){
		intrAddr = WORD_2;
	}	
	else if(lineNo == 6){
		intrAddr = WORD_3;
	}
	else{
		intrAddr = WORD_4;
	}
	
	/*find the deviceNo*/
	if((intrAddr & DEVICEADDR_0) == 0){
		return 0;
	}
	else if((intrAddr & DEVICEADDR_1) == 0){
		return 1;
	}	
	else if((intrAddr & DEVICEADDR_2) == 0){
		return 2;
	}
	else if((intrAddr & DEVICEADDR_3) == 0){
		return 3;
	}
	else if((intrAddr & DEVICEADDR_4) == 0){
		return 4;
	}
	else if((intrAddr & DEVICEADDR_5) == 0){
		return 5;
	}	
	else if((intrAddr & DEVICEADDR_6) == 0){
		return 6;
	}
	else if((intrAddr & DEVICEADDR_7) == 0){
		return 7;
	}
	else{
		return 0;
	}
	
	
}

/*Line 7 is a terminal line and because the status is split different than the rest of the lines, it has to be treated differently.*/
int handleTerminalLine(int *semaddr)
{
	/*local vars */
	unsigned int status;
	
	status = devregarea->devreg[*(semaddr)].t_transm_status;
	
	/*if terminal transit is on, acknowledge. else, ack recieve*/
	if((status &  0x0F) != READY)
	{
		devregarea->devreg[*(semaddr)].t_transm_command = ACK; 
	}
	else
	{
		status = devregarea->devreg[(*semaddr)].t_recv_status;
		devregarea->devreg[*(semaddr)].t_recv_command = ACK;
		
		
		termReceive = 1;
	}
	
	return(status);
	
}

/*Loads timers based on the type of clock it is (determined by line)*/
void handleClockLines(int lineNo)
{
	/*Highest priority Local Timer goes right on through to the readyQueue*/
	if(lineNo == 1)
	{
		moveState(oldINT, &(currentProcess->p_s));
		
		insertProcQ(&readyQueue, currentProcess);
		
		currentProcess = NULL;
		
		setTIMER(TIMESLICE); /*Load Local Timer*/
		
	}
	/*Perform V operation on Interval Timer*/
	else
	{
		
		pcb_t *p;
		
		LDIT(CPUADDTIME); /*Load Interval Timer*/		
		
		p = removeBlocked(&(clockTimer));
		
		while(p != NULL)
		{
			insertProcQ(&readyQueue, p);
			
			clockTimer = clockTimer + 1;
			softBlockCount = softBlockCount - 1;
			
			p = removeBlocked(&(clockTimer));
			
			
		}
	
	}
}

/*******************Interrupt Handling***********************/

/*Interrupts are handled by lowest number = highest priority*/
void interruptHandler(){
	
	/*Local Vars*/

	device_t *deviceSema; 	/*The device sema4*/
	pcb_t *p;				/*a pcb that could be blocked, put on the readyQueue, etc.*/
	
	int cause; 				/*Holds the value of the old cause status*/ 	
	int line; 				/*Holds the line number variable*/
	int lineIndex; 			/*Index of the line (used for sema4 status array)*/
	int device;				/*Holds the instance (device) number*/
	int deviceSemaIndex; 	/*Used to find the deviceSema using algebra*/
	unsigned int intStatus; /*Holds the status of the interrupt*/
	
	/*Step 0: Store time and previous state*/
	cause = oldINT->s_cause;
		
	if(currentProcess != NULL)
	{
		moveState(oldINT, &(currentProcess->p_s));

		STCK(endTOD);

		currentProcess->p_CPUTime = (currentProcess->p_CPUTime) + (endTOD - startTOD);
	}
	
	/*Step 1: Determine the line #*/
	line = findDevLine(cause);
	
	/*Check if line number is equal to one of the timers (lines 1 and 2). Else the line is 3-7*/
	if(line == 1 || line == 2)
	{
		handleClockLines(line);
	}
	
	else
	{
		/*Step 2: Determine the instance # */
		device = findDev(line);
		
		/*Step 3: Calculate the sema4 for this device*/
		deviceSemaIndex = (((line - 3) * 8) + device);
		
		deviceSema = &(devregarea->devreg[deviceSemaIndex]);
		
		/*Step 4: Get/set the status for process v0 & acknowledge the interrupt*/
		if(line == TERMINT)
		{
			intStatus = handleTerminalLine(&deviceSemaIndex);
		}
		else
		{
			intStatus = deviceSema->d_status;
			
			deviceSema-> d_command = ACK;
		}
		
		/*Set the line index based on if it is a terminal or not*/
		lineIndex = line;
		
		if(line == TERMINT)
		{
			/*Check if the terminal status is a transmit and adjust index accordingly*/
			if(termReceive != 1)
			{
				lineIndex = lineIndex - 2;
			}
			else
			{
				lineIndex = lineIndex - 3;
			}
		}
		else
		{
			lineIndex = lineIndex - 3;
		}
		
		/*Step 5: V the sema4*/
		deviceList[lineIndex][device] = deviceList[lineIndex][device] + 1;
		
		p = removeBlocked(&(deviceList[lineIndex][device]));

		
		/*If p is equal to NULL, set the status of the sema4 is the sema4 status list (special case is if it it a transmit status terminal line)*/
		if(p == NULL)
		{
			if(line == TERMINT)
			{
				if(termReceive != 1)
				{
					deviceStatusList[line - 2][device] = intStatus;
				}
				else
				{
					deviceStatusList[line - 3][device] = intStatus;
				}
			}
			
			else
			{
			
				deviceStatusList[line - 3][device] = intStatus;
			}
			
		}
		else
		{
			
			insertProcQ(&readyQueue, p); 	/*insert process into readyQueue (job is no longer blocked)*/
			
			softBlockCount = softBlockCount - 1;
			
			(p->p_s).s_v0 = intStatus;	/*Store devices status into the pcb's v0*/
		
		}
	}

		
	termReceive = 0; /*reset for next interrupt (else everything would think it is a terminal..Which would be bad*/
		
	/*Step 7: Return control to process that was excecuting at the time of the interrupt & fix time (if in wait state, call scheduler)*/
	if(currentProcess != NULL)
	{
		STCK(startTOD);
		
		loadState(oldINT);
	}
	else
	{
		scheduler();
	}
	
}
