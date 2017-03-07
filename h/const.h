#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* maximum process ///*/
#define MAXPROC 20

/* Hardware & software constants */
#define PAGESIZE		4096	/* page size in bytes */
#define WORDLEN			4		/* word size in bytes */
#define PTEMAGICNO		0x2A


#define ROMPAGESTART	0x20000000	 /* ROM Reserved Page */

/*Clock values*/
#define INTERVALTIME 	100000000 /*time put into PsuedoClock (interval)*/
#define CPUADDTIME 		100000 /*time for scheduler (CPU specific)*/
#define TIMESLICE 		5000 	 /*time set for timers*/

/* SYSCALL values*/
#define CREATEPROCESS 		1
#define TERMINATEPROCESS 	2
#define VERHOGEN 			3
#define PASSEREN 			4
#define SPECTRAPVEC 		5
#define GETCPUTIME 			6
#define WAITCLOCK 			7 
#define WAITIO 				8

#define WRITETERMINAL 		10
#define WRITEPRINTER 		16
#define GETTOD 				17
#define TERMINATE 			18


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define TODLOADDR		0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024

/* reserved areas*/
#define ADDRESS1 		0x10000000
#define NEWSYSCALL 		0x200003D4
#define NEWTRAP 		0x200002BC
#define NEWTLB 			0x200001A4
#define NEWINTERRUPT 	0x2000008C
#define OLDSYSCALL 		0x20000348
#define OLDTRAP 		0x20000230
#define OLDTLB 			0x200001A4
#define OLDINTERRUPT 	0x20000000

/*semaphore number for devices*/
#define DEVICELISTNUM 	6
#define DEVICENUM 		8


/* utility constants */
#define	TRUE		1
#define	FALSE		0
#define ON          1
#define OFF         0
#define HIDDEN		static
#define EOS			'\0'

#define NULL ((void *)0xFFFFFFFF)


/* vectors number and type */
#define VECTSNUM	4

#define TLBTRAP		0
#define PROGTRAP	1
#define SYSTRAP		2
#define TRAPTYPES	3

/* device interrupts */
#define DISKINT		3
#define TAPEINT 	4
#define NETWINT 	5
#define PRNTINT 	6
#define TERMINT		7

#define DEVREGLEN	4	/* device register field length in bytes & regs per dev */
#define DEVREGSIZE	16 	/* device register size in bytes */

/* device register field number for non-terminal devices */
#define STATUS		0
#define COMMAND		1
#define DATA0		2
#define DATA1		3

/* device register field number for terminal devices */
#define RECVSTATUS      0
#define RECVCOMMAND     1
#define TRANSTATUS      2
#define TRANCOMMAND     3


/* device common STATUS codes */
#define UNINSTALLED	0
#define READY		1
#define BUSY		3

/* device common COMMAND codes */
#define RESET			0
#define ACK				1
#define READBLK 		3
#define CHARTRANS		5
#define PRINTCHR 		2
#define TRANSMITCHAR 	2
#define SEEKCYL			2
#define WRITEBLK 		4

/* operations */
#define	MIN(A,B)	((A) < (B) ? A : B)
#define MAX(A,B)	((A) < (B) ? B : A)
#define	ALIGNED(A)	(((unsigned)A & 0x3) == 0)

/* Useful operations */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* hexadecimal values for IE, KU, BEV, and VM (for Status)*/
#define ALLOFF 	0x00000000		/* everything is off bit */
#define IEc 	0x00000001		/* global interrupt enable bit */
#define KUc 	0x00000002		/* The “current” kernel-mode user-mode control bit.*/

#define IEp 	0x00000004		/* the “previous” settings of the Status.IEc */
#define KUp 	0x00000008		/* the “previous” settings of the Status.KUc*/
#define IEo 	0x00000010		/* the “previous” settings of the Status.IEp*/
#define KUo 	0x00000020		/* the “previous” settings of the Status.KUp*/

#define IM  	0x0000ff00		/* Interrupt Mask. An 8-bit mask that enables/disables external interrupts.*/

#define BEV 	0x00400000		/* The Bootstrap Exception Vector. This bit determines the starting address for the exception vectors.*/

#define VMc 	0x01000000		/* The “current” VM on/off flag bit. Status.VMc=0 indicates that virtual memory translation is currently off.*/
#define VMp 	0x02000000		/* the “previous” setting of the Status.VMc bit*/
#define VMo 	0x04000000		/* the “previous” setting of the Status.VMp bit*/

#define TE  	0x08000000		/* the processor Local Timer enable bit. A 1-bit mask that enables disables the processor’s Local Timer.*/

/*hex values for line addresses for cause bits*/
#define LINEADDR_0 0x00000100
#define LINEADDR_1 0x00000200
#define LINEADDR_2 0x00000400
#define LINEADDR_3 0x00000800
#define LINEADDR_4 0x00001000
#define LINEADDR_5 0x00002000
#define LINEADDR_6 0x00004000
#define LINEADDR_7 0x00008000

/*hex values for device addresses on the above line addrs*/
#define DEVICEADDR_0 0x1
#define DEVICEADDR_1 0x2
#define DEVICEADDR_2 0x4
#define DEVICEADDR_3 0x8
#define DEVICEADDR_4 0x10
#define DEVICEADDR_5 0x20
#define DEVICEADDR_6 0x40
#define DEVICEADDR_7 0x80

/*hex values for word based on the lineNumber*/
#define WORD_0 0x1000003C
#define WORD_1 0x10000040
#define WORD_2 0x10000044
#define WORD_3 0x10000048
#define WORD_4 0x1000004C

/*Phase 3 constants*/
#define MAXPAGEPROCS 	8
#define SWAPPAGES 		24
#define SEGWIDTH 		12
#define KUSEGPTESIZE 	32
#define KUSEGOSSIZE 	64
#define MAGICNUMBER 	0x2A /*42*/
#define DISKCOUNT 		8
#define TAPECOUNT 		8
#define HEADLOC   		0
#define TLBL 	  		2
#define TLBS	  		3

#define OSEND 			(PAGESIZE * KUSEGOSSIZE)
#define KSEGEND 		(ROMPAGESTART + OSEND)
#define BUFFERSTART 	(KSEGEND - (DISKCOUNT * PAGESIZE))
#define TAPEBUFFSTART 	(OSEND - ((TAPECOUNT * PAGESIZE) + (DISKCOUNT* PAGESIZE))
#define SYSSTACK 		(PAGESIZE * SYSTRAP)

#define TAPEDEV (((TAPEINT-3) * DEVREGSIZE * DEVPERINT) + DEVICEREGS)
#define DISKDEV (((DISKINT-3) * DEVREGSIZE * DEVPERINT) + DEVICEREGS)
#define TERMDEV (((TERMINT-3) * DEVREGSIZE * DEVPERINT) + DEVICEREGS)
#define PRNTDEV (((PRNTINT-3) * DEVREGSIZE * DEVPERINT) + DEVICEREGS)

/*Phase 3 Addresses*/
#define STARTADDR 	0x20000500
#define SEGOSADDR 	0x2000
#define SEGTWOADDR 	0x8000
#define LASTADDR 	0xBFFFF
#define PAGE52ADDR 	0x8000000B
#define LASTSEG2PG 	0xC0000000
#define DEVICEREGS 	0x10000050

/*Bits for Phase 3*/
#define DIRTYON 	0x00000400
#define GLOBALON 	0x00000100
#define VALIDON 	0x00000200
#define MASKBIT 	0x00000FC0
#define PAGEMASK 	0x3ffff00

/*YellowBook page 40 const*/
#define EOT 	0
#define EOF 	1
#define EOB 	2
#define TS 		3



/*-------------------------------------------------------------------*/
#endif
