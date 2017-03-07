#ifndef EXCEPTIONS
#define EXCEPTIONS

/************************** EXCEPTIONS.E ******************************
*
*  Description here
*
*  Written by Mbusi & Gracie
*/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"
#include "../e/interrupts.e"
#include "../e/scheduler.e"
#include "/usr/local/include/umps2/umps/libumps.e"


extern void syscallHandler();
extern void tlbHandler();
extern void programTrapHandler();
extern void createProcess(state_t *statep);
extern void terminateProcess(pcb_t *p);
extern void verhogen(int *semaddr);
extern void passeren(int *semaddr);
extern void specTrapVec(int type, state_t *oldP, state_t *newP);
extern void getCPUTime();
extern void waitForClock();
extern void waitForIO(int intlNo, int dnum, int waitForTermRead);

/***************************************************************/

#endif
