/*****************************  Scheduler.c  *************************************** 
 * This module implements a simple round-robin process scheduler with a time slice of
 * 5 miliseconds. It also performs deadlock detection.
 * 
 * Helper Functions:
 * 		loadState: Performs LDST command on passed in state_t ptr.
 * 
 ****************************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../e/initial.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/*******************Helper Functions********************/

/*Load the previous state the was before a context switch*/
void loadState(state_t *state){
	LDST(state);
	
}

void scheduler(){
	

	/*If we don't have a current process, get one going!*/
	if(currentProcess == NULL){
		
		/*If we want a new currentProcess, but the readyQ doesn't have anything, handle it one of three ways:*/
		if(emptyProcQ(readyQueue)){
			
			/* if the process count is zero invoke the HALT ROM  service*/
			if(processCount == 0) {
				
				HALT();
				
			}
			/* Oh no - Deadlock is defined when the process count > 0 and the soft block count is zero*/ 
			else if (processCount > 0 && softBlockCount == 0) {
	
				PANIC();
				
			}
			/* else processCount > 0 and softblock count > 0 so we WAIT */
			else{
				setTIMER(INTERVALTIME);
				setSTATUS((getSTATUS() | IEc | IM));
				WAIT();
				
			}
		}
		
		/* if readyQueue is not empty, take a process*/
		currentProcess = removeProcQ(&readyQueue);

		/* context switch*/
		loadState(&(currentProcess->p_s));
	}
}




