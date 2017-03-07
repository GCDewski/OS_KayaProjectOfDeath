/*****************************  Exceptions.c  *************************************** 

 * This module handles Syscall/Bp Exceptions (1-8), Pgm Trap Exceptions, and
 * TLB Exceptions. 
 * 
 * Helper functions: 
 * 		passUpOrDie: If the exception is a PgmTrap, TLB, or 9+ Sys, this function
 * 			determines if the process has called sys 5 and if it has it should
 * 			be passed up, otherwise, kill it with sys 2.
 * 

 ****************************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/asl.e"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/interrupts.e"
#include "/usr/local/include/umps2/umps/libumps.e"


/****************Module global variables****************/

cpu_t endTOD;	/*Used to keep track of TOD clock*/

state_t *oldSys = (state_t *) OLDSYSCALL;	/*Old Syscall state*/
state_t *oldProgram = (state_t *) OLDTRAP;	/*Old Trap state*/
state_t *oldTLB = (state_t *) OLDTLB;	/*Old TLB state*/

/*****************Helper Functions***********************/

/*Helper function that handles if the TLB, pgm trap, or syscall should be passed up or killed*/
void passUpOrDie(int type, state_t *oldState)
{	
	/*If the current process does not have a value for type (sys 5 has not been called), kill it*/	
	if(currentProcess->p_types[type].newState == NULL) { 

		terminateProcess(currentProcess); 

		currentProcess = NULL;	

		scheduler();	

	/*else "pass it up"*/	
	} else {

		moveState(oldState, currentProcess->p_types[type].oldState);

		moveState(currentProcess->p_types[type].newState, &(currentProcess->p_s)); 

		loadState(&(currentProcess->p_s));

	}
}

/*****************TLB Exception Handling*****************/

/* Occurs: When MPS2 fails to translate a virtual address into its corresponding physical address.
 * 
 * Executes one of two actions based on if current process has performed a SYS5 through helper function "passUpOrDie": 
 * 1) If process has not performed SYS5 - currentProcess and its progeny are "killed" (SYS2)
 * 2) If the process has performed SYS5 - "pass up" the processor state from TLB OLD AREA to the one
 * 		recorded in currentProcess and the processor new TLB state recorded in currentProcess becomes 
 * 		the current processor state.*/
 
void tlbHandler(){

	passUpOrDie(0, oldTLB);

}



/*****************Pgm Trap Exception Handling******************/

/* Occurs: When the executing process attempts to perform some illegal or undefined action
 * 
 * Executes one of two actions based on if current process has performed a SYS5 through helper function "passUpOrDie": 
 * 1) If process has not performed SYS5 - currentProcess and its progeny are "killed" (SYS2)
 * 2) If the process has performed SYS5 - "pass up" the processor state from PGM OLD AREA to the one
 * 		recorded in currentProcess and the processor new PGM state recorded in currentProcess becomes 
 * 		the current processor state.*/

void programTrapHandler(){
	
	passUpOrDie(1, oldProgram);
}



/*****************Syscall Exception Handling*****************/

/* Occurs: When a Syscall or Breakpoint assembler instuction is executed.
 * Executes some instruction based on the value of 1-8 found in a[0] */
 
void syscallHandler(){
	
	/*local vars*/
	int kernelMode; /*Hold boolean value of if in kernel/user mode*/


	moveState(oldSys, &(currentProcess->p_s));	/*context switch!*/	
	
	currentProcess->p_s.s_pc = currentProcess->p_s.s_pc+4; 	/*move on from interrupt (so groundhog day won't happen)*/
	
	kernelMode = (oldSys->s_status & KUp);		/*set kernelMode*/


	/* if the syscall was 1-8 but we are also in user mode*/ 
	if(kernelMode != 0){
		
		if ((oldSys->s_a0 > 0) && (oldSys->s_a0 <= 8)){
			
			/* set the cause register to be a privileged instruction*/
			oldSys->s_cause = oldSys->s_cause | (10 << 2);

			moveState(oldSys, oldProgram);

			programTrapHandler();
		}
	}
	/* check if we are in kernel mode and the syscall is from 1-8 */
	else{ 
		
		if((oldSys->s_a0 > 0) && (oldSys->s_a0 <= 8)){

			switch (oldSys->s_a0){
			

				case CREATEPROCESS: 		

					createProcess((state_t *) oldSys->s_a1);  

				break;	

				case TERMINATEPROCESS:

					terminateProcess(currentProcess);

					scheduler();	

				break;

				case VERHOGEN:
					verhogen((int *) oldSys->s_a1);

				break;	

				case PASSEREN:

					passeren((int *) oldSys->s_a1);		

				break;

				case SPECTRAPVEC:

					specTrapVec((int) oldSys->s_a1, (state_t *) oldSys->s_a2, (state_t *) oldSys->s_a3);	

				break;

				case GETCPUTIME:

					getCPUTime();	

				break;

				case WAITCLOCK:

					waitForClock();	

				break;

				case WAITIO:
					waitForIO((int) oldSys->s_a1, (int) oldSys->s_a2, (int) oldSys->s_a3);		

				break;

			}
		}

	}
	
	/*If syscall is 9 or greater, kill it or pass up*/
	passUpOrDie(2, oldSys);

}



/*Syscall 1 causes a new process (progeny) of the caller to be created. If the new process cannot be created
 *(no free PCBs for example), the error code -1 is placed within the caller's v0. Otherwise, 0 is placed in v0 */
 
void createProcess(state_t *statep) { 		

	/* create and allocate a new pcb */
	pcb_t *newProcess;
	newProcess = allocPcb();

	

	/*check if process cannot be created*/
	if (newProcess == NULL) {

		/* set the v0 register to -1 we had an error*/
		currentProcess->p_s.s_v0 = -1;
		
	}

	else{
		
		processCount = processCount+1;
				
		/* we were successful in creating a new proc so move to the new state */
		moveState(statep, &(newProcess->p_s));

		insertChild(currentProcess, newProcess);

		insertProcQ(&readyQueue, newProcess);

		
		/* set the v0 register to 0 it was successful*/
		currentProcess->p_s.s_v0 = 0;	

	}

	/* calls load state with the current process state*/
	loadState(&(currentProcess->p_s));

}



/*Syscall 2 causes the executing process to cease to exist (POOF). In addition, recursively, all progeny of this
 * process are terminated as well.*/

void terminateProcess(pcb_t *p)

{
	/*call SYS2 recursively in order to get rid of all children*/
	while(!emptyChild(p)){

		terminateProcess(removeChild(p));

	}

	

	/*check if current process has been annihilated*/
	if (p == currentProcess) {

		outChild(p);

		currentProcess = NULL;

	}

	/*Check if the currentProcess is blocked*/
	else if (p->p_semAdd != NULL)
	{
		outBlocked(p);

		/*if the semaAdd is not the clock or the last sema4, subtract a softBlk*/
		if((p->p_semAdd > &(deviceList[0][0])) && (p->p_semAdd < &(deviceList[DEVICELISTNUM][DEVICENUM]))){

			softBlockCount = softBlockCount -1;

		}

		/*The semAdd is the clock*/
		else{

			*(p->p_semAdd) = *(p->p_semAdd) + 1;

		}

	}

	/*else the currentProcess is on the readyQueue so call outProcQ*/
	else{

		outProcQ(&(readyQueue), p);	

	}

	freePcb(p); /*put it on the free PCB list beacuse it is now been officially killed*/

	processCount = processCount - 1; /*One less process to worry about. */

}



/*Syscall 3 (or "V Operation) causes the sema4 from the address given to be signaled and incremented*/
void verhogen(int *semaddr) {

	pcb_t *p;

	*(semaddr) = *(semaddr)+1; /* Increment the int pointed ay by currentProcess->p_s.s_a1 */

	p = removeBlocked(semaddr);



	/*If p exists based on the address given, put it on the readyQueue*/
	if (p != NULL) {
		
		p->p_semAdd = NULL;
		insertProcQ(&readyQueue, p);

	}

	loadState(&(currentProcess->p_s));			/*Non-blocking call*/

}



/*Syscall 4 (or "P Operation) causes the sema4 from the address given to be told to wait and decremented*/
void passeren(int *semaddr) {

	*(semaddr) = *(semaddr)-1; /* Decrement the int pointed ay by currentProcess->p_s.s_a1 */

	/*If the semaddr is less than 0, block the currentProcess & prep for context switch*/
	if (*(semaddr) < 0) {
		
		/*Block process*/
		insertBlocked(semaddr, currentProcess);
		currentProcess->p_semAdd = semaddr;	
		
		/*Store time and change the time it took to process*/
		STCK(endTOD);
		currentProcess->p_CPUTime = (currentProcess->p_CPUTime) + (endTOD-startTOD);
		
		currentProcess = NULL;

		scheduler();	

	}
	
	loadState(&(currentProcess->p_s));
	

}



/* Syscall 5 causes the nucleus to store the processor state at the time of the exception in that area pointed
 * to by the address in a2, and loads the new processor state from the area pointed to by the address given in a3.
 * This syscall can only be called at most once for each of the three excpetion types(TLB, PGM, and Syscall). 
 * Any request that that calls this more than that shall be executed (Sys2 style).*/
void specTrapVec(int type, state_t *oldP, state_t *newP) {

	/*Sys 5 has already been called before - kill it*/
	if(currentProcess->p_types[type].newState != NULL)
	{
		terminateProcess(currentProcess);
		
		scheduler();
		
	}
	/*Else set the old and new states*/
	else
	{
		currentProcess->p_types[type].oldState = oldP;
		
		currentProcess->p_types[type].newState = newP;
		
		loadState(&(currentProcess->p_s));
		
	}

}



/*Syscall 6 causes the procesor time (in microseconds) used by the requesting process to be placed/returned to the caller's v0. 
 * This means that the nucleaus must record in the PCB the amount of processor time used by each process*/
 
void getCPUTime(){

	/* place the processor time in microseconds in the v0 reg */
	currentProcess->p_s.s_v0 = currentProcess->p_CPUTime;

	/* load the state of the process to continue */ 
	loadState(&(currentProcess->p_s));
}



/*Syscall 7 performs a P operation on the pseudo-clock timer sema4. This sema4 is V'd every 100 milliseconds automatically by the nucleus*/
void waitForClock()
{
	/* Decrement the int pointed ay by currentProcess->p_s.s_a1 */
	clockTimer = clockTimer - 1;

	/*If the semaddr is less than 0, block the currentProcess & prep for context switch*/
	if(clockTimer < 0){

		insertBlocked (&(clockTimer) , currentProcess);
		/*Store time and change the time it took to process*/
		STCK(endTOD);

		currentProcess->p_CPUTime = currentProcess->p_CPUTime + (endTOD - startTOD);

		softBlockCount = softBlockCount + 1;

		currentProcess = NULL;

		scheduler();

	}

	loadState(&(currentProcess->p_s));
	
}
/*Syscall 8 performs a P operation on the I/O device sema4 indicated by the values in a1, a2, and optionally a3*/

void waitForIO(int intlNo, int dnum, int waitForTermRead){

	/*Check if the line number is a terminal based on the constant*/
	if(intlNo == TERMINT)
	{
		/*If we don't want to wait to read it as a ternimal, set the terminal line number.*/
		if(!waitForTermRead)
		{
			intlNo = intlNo - 2;
		}
		else
		{
			intlNo = intlNo - 3;
		}

	}

	else
	{
		intlNo = intlNo - 3;
	}

	

	/*Perform a P operation on the correct sema4*/
	deviceList[intlNo][dnum] = (deviceList[intlNo][dnum])-1;

	if(deviceList[intlNo][dnum] < 0)

	{	
		insertBlocked(&(deviceList[intlNo][dnum]), currentProcess);

		/*Store time and change the time it took to process*/
		STCK(endTOD);
		currentProcess->p_CPUTime = (currentProcess->p_CPUTime) + (endTOD-startTOD);

		currentProcess = NULL;

		softBlockCount = softBlockCount + 1;

		scheduler();	/*Blocking call*/

	}
	
	currentProcess->p_s.s_v0 = deviceStatusList[intlNo][dnum]; /*set the status word*/
	
	loadState(&(currentProcess->p_s));

}

