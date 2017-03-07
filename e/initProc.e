#ifndef INITPROC
#define INITPROC

/************************** INITPROC.E ******************************
*
*  Description here
*
*  Written by Mbusi & Gracie
*/

#include "../h/types.h"
#include "../h/const.h"

extern int masterSem;
extern int swapSem; 
extern int diskSem;
extern Tproc_t procs[MAXPAGEPROCS];
extern swap_t swapTables[SWAPPAGES];
extern int sema4array[DEVICELISTNUM * DEVICENUM];
extern pteOS_t KSegOS;

extern void test();
extern void midwife();
extern state_t* setStateAreas();


/***************************************************************/

#endif
