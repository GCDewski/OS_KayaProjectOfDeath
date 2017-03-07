#ifndef INITIAL
#define INITIAL

/************************** INITIAL.E ******************************
*
*  Description here
*
*  Written by Mbusi & Gracie
*/

#include "../h/types.h"

extern int main();

extern int processCount;
extern int softBlockCount;
extern pcb_t * currentProcess;
extern pcb_t *readyQueue;
extern int deviceList[DEVICELISTNUM][DEVICENUM];
extern unsigned int deviceStatusList[DEVICELISTNUM][DEVICENUM];
extern int clockTimer;
extern cpu_t startTOD;
extern cpu_t endTOD;

extern void moveState(state_t *previous, state_t *current );


/***************************************************************/

#endif
