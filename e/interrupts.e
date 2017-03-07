#ifndef INTERRUPTS
#define INTERRUPTS

/************************** INTERRUPTS.E ******************************
*
*  Description here
*
*  Written by Mbusi & Gracie
*/

#include "../h/types.h"

void handleClockLines(int lineNo);
int handleTerminalLine(int *semaddr);
int findDevLine(memaddr causeAddr);
int findDev(int lineNo);

extern void interruptHandler();

/***************************************************************/

#endif
