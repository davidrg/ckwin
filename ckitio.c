/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors.                                                    */
/* | o  | ||    Dave Baker     Ed Burnette  Stan Chow    Jay Denebeim        */
/* |  . |//     Gordon Keener  Jack Rouse   John Toebes  Doug Walker         */
/* ======          BBS:(919)-471-6436      VOICE:(919)-469-4210              */
/*                                                                           */
/* Contributed to Columbia University for inclusion in C-Kermit.             */
/* Permission is granted to any individual or institution to use, copy, or   */
/* redistribute this software so long as it is not sold for profit, provided */
/* this copyright notice is retained.                                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char *ckxv = "Amiga tty I/O, 4F(007), 7 Nov 89";
 
/*  C K I T I O  --  Serial and Console I/O support for the Amiga */

/*
 Author: Jack Rouse, The Software Distillery
 Based on the CKUTIO.C module for Unix

 Modified for Manx Aztec C and Version 1.2 and forward of Amiga's OS by
 Stephen Walton of California State University, Northridge,
 ecphssrw@afws.csun.edu.  Further mods documented in ckiker.upd.
*/

#include <stdio.h>		/* standard I/O stuff */
#undef NULL
#include "exec/types.h"
#include "exec/exec.h"
#include "devices/serial.h"
#include "devices/timer.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#define fh_Interact fh_Port
#define fh_Process fh_Type
#include "intuition/intuition.h"
#include "intuition/intuitionbase.h"
#define BREAKSIGS (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)
#ifdef AZTEC_C
#include "fcntl.h"
#include "signal.h"
#else
#ifdef LAT310
#include "fcntl.h"
#include "signal.h"
#include "ios1.h"		/* defines ufbs structure */
#else
#include "lattice/ios1.h"	/* defines ufbs structure */
#endif
#endif

/*
 * We can't #include "ckcdeb.h" as we'd like here, because its typedef
 * for LONG conflicts with the one in exec/types.h.
 */

#ifndef DEBUG
#define debug(a,b,c,d)
#endif

char *ckxsys = " Commodore Amiga";	/* system name */

/* external declarations */
extern int speed, mdmtyp, parity, flow;

/* external definitions */
char *dftty = SERIALNAME;		/* serial device name */
int dfloc = 1;				/* serial line is external */
int dfprty = 0;				/* default parity is none */
int ttprty = 0;				/* parity in use */
int dfflow = 1;				/* default flow control is on */
int backgrd = 0;			/* default to foreground */
int ckxech = 0;				/* echo in case redirected stdin */

struct Process *CurProc;		/* current process */
struct CommandLineInterface *CurCLI;	/* current CLI info */
struct IntuitionBase *IntuitionBase;	/* ptr to Intuition lib */
 
/* static definitions */
static struct MsgPort *serport;		/* message port for serial comm */
static struct MsgPort *conport;		/* console packet port */
static struct timerequest *TimerIOB;	/* timer request */
static struct IOExtSer *ReadIOB;	/* serial input request */
static struct IOExtSer *WriteIOB;	/* serial output request */
static struct DosPacket *conpkt;	/* console I/O packet */
static WORD serialopen;			/* true iff serial device open */
static WORD timeropen;			/* true iff timer device open */
static WORD pendwrite;			/* true iff WriteIOB in use */
static WORD pendread;			/* true iff ReadIOB in use */
static WORD pendconsole;		/* true when console read pending */
static int queuedser;			/* serial pushback char or -1 */
static UBYTE serbufc;			/* char buffer for read ahead I/O */
#define NTTOQ 64			/* connect output queue size */
static char ttoq[NTTOQ];		/* connect output queue */
static int nttoq;			/* number of chars in ttoq */
static int pttoq;			/* next char to output in ttoq */
static int queuedcon;			/* contti pushback char or -1 */
static LONG intsigs;			/* signals for aborting serial I/O */
static int (*inthdlr)();		/* function to signal break to */
static BPTR rawcon;			/* file handle for RAW: window */
static BPTR saverr;                     /* saved stderr file handle */
static APTR savewindow;			/* saved process WindowPtr */
static APTR pushwindow;			/* pushed process WindowPtr */
static struct DateStamp prevtime;	/* saved time value */

/* Exec routines */
APTR AllocMem();
LONG AllocSignal();
struct IORequest *CheckIO();
VOID CloseDevice(), CloseLibrary();
LONG DoIO();
struct MsgPort *FindPort();
struct Task *FindTask();
VOID FreeMem(), FreeSignal();
struct Message *GetMsg();
LONG OpenDevice();
struct Library *OpenLibrary();
VOID PutMsg(), ReplyMsg();
VOID SendIO();
LONG SetSignal();
VOID Signal();
LONG Wait();
LONG WaitIO();
struct Message *WaitPort();

/* Exec support */
struct IORequest *CreateExtIO();
VOID DeleteExtIO();
struct MsgPort *CreatePort();
VOID DeletePort();
struct Task *CreateTask();
VOID DeleteTask();

/* AmigaDOS routines */
VOID Delay();
BPTR Open();
VOID Close();
BPTR Input(), Output();
LONG Read();
LONG WaitForChar();
struct DateStamp *DateStamp();

/* AmigaDOS support (from ckiutl.c) */
struct DosPacket *CreatePacket();
VOID DeletePacket();

#ifdef AZTEC_C
/* translate Unix file handle (0, 1, or 2) to AmigaDOS file handle */
#define DOSFH(n) (_devtab[n].fd)
/* translate Unix file handle (0, 1, or 2) to Aztec file handle */
#define FILENO(n) (n)
extern int Enable_Abort;
#else
#ifdef LAT310
/* translate Unix file handle (0, 1, or 2) to AmigaDOS file handle */
#define DOSFH(n) fileno(&_iob[n])
/* translate Unix file handle (0, 1, or 2) to Lattice file handle */
#define FILENO(n) fileno(&_iob[n])
#else
/* Lattice runtime externals */
extern struct UFB _ufbs[];
extern int Enable_Abort;
#define DOSFH(n) (_ufbs[n].ufbfh)
#define FILENO(n) (n)
#endif
#endif

/*
 * make note of a serial error and quit
 */
static Fail(msg)
char *msg;
{
	syscleanup();
	fprintf(stderr, msg);
	fprintf(stderr, "\n");
	exit(2);
}
/*
 * default interrupt handler
 */
static int defhdlr()
{
	printf("*** BREAK ***\n");
	doexit(1);
}

/*
 *  sysinit -- Amiga specific initialization
 */
sysinit()
{
	register struct IOExtSer *iob;

	/* set current process info */
	CurProc = (struct Process *)FindTask((char *)NULL);
	CurCLI = (struct CommandLineInterface *)BADDR(CurProc->pr_CLI);
	backgrd = (CurCLI == NULL || CurCLI->cli_Background);
	savewindow = CurProc->pr_WindowPtr;

	/* default interrupts to exit handler */
	intsigs = BREAKSIGS;
	inthdlr = defhdlr;
#ifdef LAT310
	signal(SIGINT, SIG_IGN);
#else
	Enable_Abort = 0;
#endif

	/* allocate console ports and IO blocks */
	if ((conport = CreatePort((char *)NULL, 0L)) == NULL)
		Fail("no console MsgPort");
	if ((conpkt = CreatePacket()) == NULL)
		Fail("no console packet");

	/* allocate serial ports and IO blocks */
	if ((serport = CreatePort((char *)NULL, 0L)) == NULL)
		Fail("no serial MsgPort");
	iob = (struct IOExtSer *)CreateExtIO(serport,(LONG)sizeof(*iob));
	if ((WriteIOB = iob) == NULL) Fail("no WriteIOB");
	iob = (struct IOExtSer *)CreateExtIO(serport,(LONG)sizeof(*iob));
	if ((ReadIOB = iob) == NULL) Fail("no ReadIOB");

	/* open the timer device */
	TimerIOB = (struct timerequest *)
		   CreateExtIO(serport,(LONG)sizeof(*TimerIOB));
	if (TimerIOB == NULL) Fail("no TimerIOB");
	if (OpenDevice(TIMERNAME, (LONG)UNIT_VBLANK, TimerIOB, 0L) != 0)
		Fail("no timer device");
	timeropen = TRUE;

	/* open the Intuition library */
	if (!IntuitionBase &&
	    (IntuitionBase = (struct IntuitionBase *)
			     OpenLibrary("intuition.library", 0L) ) == NULL )
		Fail("can't open Intuition");

	/* open the serial device to get configuration */
	iob->io_SerFlags = SERF_SHARED;
	if (OpenDevice(SERIALNAME, 0L, iob, 0L) != 0)
		Fail("can't open serial.device");
	/* set parameters from system defaults */
	flow   = !(iob->io_SerFlags & SERF_XDISABLED);
	/*
	 * Set default (startup) parity from Preferences settings.
	 * This module is called AFTER, not BEFORE, ckcmai.c sets
	 * "parity" to "dfprty".  If it
	 * called this first, the code below could set dfprty, which
	 * it is supposed to modify;  "parity" should be private to
	 * the system-independent code.
 	 */
	if (iob->io_SerFlags & SERF_PARTY_ON) 	/* Parity is on */
		if (iob->io_ExtFlags & SEXTF_MSPON)	/* Space or mark */
			if (iob->io_ExtFlags & SEXTF_MARK)
				parity = 'm';		/* Mark parity */
			else
				parity = 's';		/* Space parity */
		else					/* Even or odd */
			if (iob->io_SerFlags & SERF_PARTY_ODD)
				parity = 'o';		/* Odd parity */
			else
				parity = 'e';		/* Even parity */
	else
		parity = 0;				/* No parity. */
	speed  = iob->io_Baud;
	mdmtyp = (iob->io_SerFlags & SERF_7WIRE) != 0;
	ttprty = parity;

	CloseDevice(iob);
	serialopen = FALSE;
	return(0);
}

/*
 * syscleanup -- Amiga specific cleanup
 */
syscleanup()
{
	/* close everything */
	if (serialopen) CloseDevice(ReadIOB);
	if (timeropen) CloseDevice(TimerIOB);
	if (TimerIOB) DeleteExtIO(TimerIOB, (LONG)sizeof(*TimerIOB));
	if (WriteIOB) DeleteExtIO(WriteIOB, (LONG)sizeof(*WriteIOB));
	if (ReadIOB) DeleteExtIO(ReadIOB, (LONG)sizeof(*ReadIOB));
	if (serport) DeletePort(serport);
	if (conpkt) DeletePacket(conpkt);
	if (conport) DeletePort(conport);
	reqres();
	if (IntuitionBase)
	{
		CloseLibrary(IntuitionBase);
		IntuitionBase = NULL;
	}

	/* reset standard I/O */
	if (rawcon > 0)
	{
		/* restore Lattice AmigaDOS file handles */
		DOSFH(0) = Input();
		DOSFH(1) = Output();
		DOSFH(2) = saverr;
#ifdef LAT310
		close(rawcon);
#else
		Close(rawcon);
#endif
	}
}

/*
 * reqoff -- turn requestors off
 *    When AmigaDOS encounters an error that user intervention can fix
 *    (like inserting the correct disk), it normally puts up a requestor.
 *    The following code disables requestors, causing an error to be
 *    returned instead.
 */
reqoff()
{
	pushwindow = CurProc->pr_WindowPtr;
	CurProc->pr_WindowPtr = (APTR)-1;
}
/*
 * reqpop -- restore requesters to action at last reqoff
 */
reqpop()
{
	CurProc->pr_WindowPtr = pushwindow;
}

/*
 * reqres -- restore requestors to startup action
 */
reqres()
{
	CurProc->pr_WindowPtr = savewindow;
}

/*
 * KillIO -- terminate an I/O request
 */
static int KillIO(iob)
struct IORequest *iob;
{
	AbortIO(iob);
	return(WaitIO(iob));
}
/*
 * DoIOQuick -- DoIO with quick IO
 * This should not be used where waiting is expected since
 *    it cannot be interrupted.
 */
static int DoIOQuick(iob)
register struct IORequest *iob;
{
	register int D7Save;	/* V1.1 bug. IO sometimes trashes D7 */

	/* do I/O with quick option, wait around if necessary */
	iob->io_Flags = IOF_QUICK;
	if (BeginIO(iob) == 0 && !(iob->io_Flags & IOF_QUICK))
		WaitIO(iob);

	/* return the error, if any */
	return((int)iob->io_Error);
}

/*
 * ttopen -- open the serial device
 *    If already open, returns 0 immediately.
 *    Otherwise, the ttname is compare to SERIALNAME and used to
 *    open the serial device, and, if the value of *lcl is < 0, it is
 *    reset to 1 indicating local mode.  Returns -1 on error.
 */
ttopen(ttname, lcl, modem)
char *ttname;
int *lcl;
int modem;
{
	register struct IOExtSer *iob = ReadIOB;

	if (serialopen) return(0);	/* ignore if already open */

	/* verify the serial name */
	if (strcmp(ttname, SERIALNAME) != 0) return(-1);

	/* set open modes */
	iob->io_SerFlags = (modem) ? (SERF_SHARED|SERF_7WIRE) : SERF_SHARED;

	/* open the serial device */
	if (OpenDevice(ttname, 0L, iob, 0L) != 0) return(-1);
	serialopen = TRUE;
	pendread = pendwrite = pendconsole = FALSE;
	queuedser = -1;

	/* fill in the fields of the other IO blocks */
	*WriteIOB = *iob;

	/* set local mode */
	if (*lcl == -1)	*lcl = 1; /* always local */
	return(0);
}

/*
 * StartTimer -- start a timeout
 */
static VOID StartTimer(secs, micro)
LONG secs, micro;
{
	TimerIOB->tr_node.io_Command = TR_ADDREQUEST;
	TimerIOB->tr_time.tv_secs  = secs;
	TimerIOB->tr_time.tv_micro = micro;
	SendIO(TimerIOB);
}

/*
 * SerialWait -- wait for serial I/O to terminate
 *    return I/O error
 */
static int SerialWait(iob, timeout)
register struct IOExtSer *iob;
int timeout;
{
	register LONG sigs;
	register struct timerequest *timer = TimerIOB;
	register LONG waitsigs;

	/* set up timeout if necessary */
	if (timeout > 0) StartTimer((LONG)timeout, 0L);

	/* wait for completion, timeout, or interrupt */
	sigs = 0;
	waitsigs = (1L << serport->mp_SigBit) | intsigs;
	for (;;)
	{
		if (sigs & intsigs)
		{	/* interrupted */
			if (timeout > 0) KillIO(timer);
			KillIO(iob);
			testint(sigs);
			return(-1);
		}
		if (CheckIO(iob))
		{
			if (timeout > 0) KillIO(timer);
			return(WaitIO(iob));
		}
		if (timeout > 0 && CheckIO(timer))
		{
			KillIO(iob);
			WaitIO(timer);
			/* restart if XOFF'ed */
			iob->IOSer.io_Command = CMD_START;
			DoIOQuick(iob);
			return(-1);
		}
		sigs = Wait(waitsigs);
	}
}

/*
 * TerminateRead -- wait for queued read to finish
 */
static int TerminateRead()
{
	if (!pendread) return(0);
	if (WaitIO(ReadIOB) == 0) queuedser = serbufc;
	pendread = FALSE;
	return((int)ReadIOB->IOSer.io_Error);
}

/*
 * TerminateWrite -- ensure WriteIOB is ready for reuse
 */
static int TerminateWrite(timeout)
int timeout;
{
	testint(0L);
	if (!pendwrite) return(0);
	pendwrite = FALSE;
	return(SerialWait(WriteIOB, timeout));
}

/*
 * SerialReset -- terminate pending serial and console I/O
 */ 
static VOID SerialReset()
{
	if (pendread)
	{
		AbortIO(ReadIOB); /* should work even if read finished */
		TerminateRead();
	}

	if (pendconsole)
	{	/* this does not happen normally */
		WaitPort(conport);
		GetMsg(conport);
		pendconsole = FALSE;
	}

	if (pendwrite)
		TerminateWrite(1);
}

/*
 * ttres -- reset serial device
 */
ttres()
{
	if (!serialopen) return(-1);

	/* reset everything */
	SerialReset();
	ReadIOB->IOSer.io_Command = CMD_RESET;
	return(DoIOQuick(ReadIOB) ? -1 : 0);
}

/*
 * ttclos -- close the serial device
 */
ttclos()
{
	if (!serialopen) return(0);
	if (ttres() < 0) return(-1);
	CloseDevice(ReadIOB);
	serialopen = FALSE;
	return(0);
}

/*
 * tthang -- hang up phone line
 *    Drops DTR by closing serial.device
 */
tthang()
{	return((serialopen) ? ttclos() : -1); }

/*
 * ttpkt -- set serial device up for packet transmission
 *    sets serial parameters
 */
ttpkt(baud, flow, newpar)
int baud, flow, newpar;
{
	extern UBYTE eol;
	register struct IOExtSer *iob = ReadIOB;
	int speed;

	if (!serialopen || pendread) return(-1);

	/* terminate any pending writes */
	TerminateWrite(1);

	/* fill in parameters */
	iob->io_CtlChar = 0x11130000;
	if (baud >= 0 && (speed = ttsspd(baud)) >= 0) iob->io_Baud = speed;
	/*
	 * Notice the dopar(eol) here to set the EOL character with the
	 * appropriate parity.  See also ttinl().
	 */
	setmem(&iob->io_TermArray, sizeof(struct IOTArray), dopar(eol));
	iob->io_ReadLen = iob->io_WriteLen = 8;
	iob->io_StopBits = 1;
	if (flow)
		iob->io_SerFlags &= ~SERF_XDISABLED;
	else
		iob->io_SerFlags |= SERF_XDISABLED;
	/* if no flow and high baud rate, RAD_BOOGIE is appropriate */
	if (!flow && iob->io_Baud > 19200)
		iob->io_SerFlags |= SERF_RAD_BOOGIE;
	else
		iob->io_SerFlags &= ~SERF_RAD_BOOGIE;

	/*
	 * Parity setting.  For packet send/receive, we turn off the
	 * Amiga's internal parity generation and checking, as this code
	 * does it itself (which makes it bigger and slower...).  We
	 * save the current parity for ttinl().
	 */

	ttprty = newpar;
	iob->io_SerFlags &= ~(SERF_EOFMODE|SERF_PARTY_ON|SERF_PARTY_ODD);
	iob->io_ExtFlags = 0;		/* MUST BE ZERO unless Mark or Space. */

	/* set the parameters */
	iob->IOSer.io_Command = SDCMD_SETPARAMS;
	if (DoIOQuick(iob) != 0) return(-1);
	return(ttflui());
}

/*
 * ttvt -- set up serial device for connect mode.  This is the same
 * as ttpkt() on the Amiga.
 */
ttvt(baud, flow, newparity)
int baud, flow, newparity;
{	return(ttpkt(baud, flow, newparity)); }

/*
 * ttsspd -- verify baud rate
 */
int ttsspd(speed)
{
    if (speed < 110 || speed > 292000) return(-1);
    return(speed < 112 ? 112 : speed);
}

/*
 * ttflui -- flush serial device input buffer
 */
ttflui()
{
	if (!serialopen || pendread) return(-1);
	queuedser = -1;
	ReadIOB->IOSer.io_Command = CMD_CLEAR;
	return(DoIOQuick(ReadIOB) ? -1 : 0);
}


static struct IntuiText BodyText = {
	-1, -1, 0, 4, 4, NULL, "Interrupt Requested", NULL
};

static struct IntuiText ContinueText = {
	-1, -1, 0, 4, 4, NULL, "Continue", NULL
};

static struct IntuiText AbortText = {
	-1, -1, 0, 4, 4, NULL, "Exit C-Kermit", NULL
};

/*
 * test for and catch interrupt
 */
testint(sigs)
LONG sigs;
{
	int (*catch)();

	/* test for and reset caught interrupt signals */
	if (((sigs | SetSignal(0L, (LONG)BREAKSIGS)) & intsigs) && inthdlr)
	{
		if (AutoRequest((struct Window *)NULL,
				&BodyText,
				&ContinueText,
				&AbortText,
				0L, 0L, 260L, 55L) )
			return;
		catch = inthdlr;
		inthdlr = NULL;
		intsigs = 0;
		(*catch)();
	}
}

/*
 * conint -- set console interrupt handler
 */
conint(newhdlr)
int (*newhdlr)();
{
	testint(0L);			/* handle any pending interrupts */
	inthdlr = newhdlr;		/* set the new handler */
	intsigs = BREAKSIGS;		/* note signal caught */
}

/*
 * connoi -- disable interrupt trapping
 */
connoi()
{
	inthdlr = NULL;			/* disable interrupts */
	intsigs = 0;			/* note signal ignored */
	testint(0L);			/* ignore pending interrupts */
}

/*
 * ttchk -- return number of chars immediately available from serial device
 */ 
ttchk()
{
	register struct IOExtSer *read = ReadIOB;

	if (!serialopen) return(-1);
	testint(0L);
	if (pendread && !CheckIO(read)) return(0);
	if (TerminateRead() != 0) return(-1);
	read->IOSer.io_Command = SDCMD_QUERY;
	return((DoIOQuick(read) == 0)
			? ((queuedser >= 0) + (int)read->IOSer.io_Actual)
			: -1);
}

/*
 * ttxin -- get n characters from serial device
 */
ttxin(n, buf)
int n;
char *buf;
{	return(ttinl(buf, n, 0, -1)); }

/*
 * ttinc -- read character from serial line
 */
ttinc(timeout)
int timeout;
{
	UBYTE ch;
 
	return((ttinl(&ch, 1, timeout, -1) > 0) ? (int)ch : -1);
}

/*
 * ttol -- write n chars to serial device.  For small writes, we have
 * a small local buffer which allows them to run asynchronously.  For
 * large writes, we do them synchronously.  This seems to be the best
 * compromise between speed and code simplicity and size.
 *
 * Stephen Walton, 23 October 1989
 */
ttol(buf, n)
char *buf;
int n;
{
	register struct IOExtSer *write = WriteIOB;
	static char outbuf[256];	/* safe place for output characters */

	if (!serialopen) return(-1);
	if (TerminateWrite(0) != 0) return(-1);
	pendwrite = TRUE;
	write->IOSer.io_Command = CMD_WRITE;
	if (n <= 256) {
		movmem(buf, outbuf, n);
		write->IOSer.io_Data    = (APTR)outbuf;
		write->IOSer.io_Length  = n;
		SendIO(write);
	} else {
		write->IOSer.io_Data	= (APTR) buf;
		write->IOSer.io_Length	= n;
		DoIOQuick(write);
	}
	return(n);
}

/*
 * ttoc -- output single character to serial device
 */
ttoc(c)
char c;
{	return(ttol(&c, 1)); }

/*
 * ttinl -- read from serial device, possibly with timeout and eol character
 *    reads up to n characters, returning the number of characters read
 *    if eol >= 0, reading the eol character will terminate read
 *    if timeout > 0, terminates read if timeout elapses
 *    returns -1 on error, such as timeout or interrupt
 */
ttinl(buf, n, timeout, eol)
register char *buf;
int n;
int timeout;				/* timeout in seconds or <= 0 */
int eol;				/* end of line character */
{
        unsigned  mask;
	register struct IOExtSer *read = ReadIOB;
	register int count;
	int nread, i, error;

	testint(0L);
 	if (!serialopen || pendread || n <= 0) return(-1);

	mask = (ttprty ? 0177 : 0377);	/* parity stripping mask */

	/* handle pushback */
	if (queuedser >= 0)
	{
		*buf = queuedser & mask;	/* Strip queued character. */
		queuedser = -1;
		if (*buf == eol || n == 1) return(1);
		++buf;
		--n;
		count = 1;
	}
	else
		count = 0;

	/* set up line terminator */
	if (eol >= 0)
	{
		/*
		 * For reasons which are obscure to me, this batch of
		 * code generally fails.  Normally, this doesn't matter,
		 * because io_TermArray is set in ttpkt() above, and so
		 * this code is only executed if eol changes as a result
		 * of the initial packet negotiation.  I found the bug
		 * by inadvertently not using dopar(eol) in the setting
		 * of io_TermArray in ttpkt(), which did cause this code
		 * to be called if parity was MARK or EVEN (since in that
		 * case dopar(eol) != eol).
		 */

		if (dopar(eol) != *(UBYTE *)&read->io_TermArray)
		{
			setmem(&read->io_TermArray,
			       sizeof(struct IOTArray), dopar(eol));
			read->IOSer.io_Command = SDCMD_SETPARAMS;
			if (DoIOQuick(read) != 0) {
				debug(7, "SETPARAMS fails in ttinl()",
				      "io_Error", (int) read->IOSer.io_Error);
				read->io_TermArray.TermArray0 =
					read->io_TermArray.TermArray1 = -1;
				return -1;
			}
		}
		read->io_SerFlags |= SERF_EOFMODE;
	}
	else
		read->io_SerFlags &= ~SERF_EOFMODE;

	/* set up the read */
	read->IOSer.io_Command = CMD_READ;
	read->IOSer.io_Data    = (APTR)buf;
	read->IOSer.io_Length  = n;

	/* perform read quickly if possible */
	read->IOSer.io_Flags = IOF_QUICK;
	BeginIO(read);
	if (read->IOSer.io_Flags & IOF_QUICK)
		read->IOSer.io_Flags = 0;
	else
		/* wait for read to complete if no QUICK. */
		if (SerialWait(read, timeout) != 0)
			return -1;
    
	if (read->IOSer.io_Error != 0)
		return -1;

	/* Strip parity bits if need be. */
	nread = (int) read->IOSer.io_Actual;
	if (ttprty)
		for (i = 0; i < nread; i++)
			buf[i] &= mask;
	return(count + nread);
}

/*
 * Sleeper -- perform an interruptible timeout
 */
static Sleeper(secs, micro)
LONG secs, micro;
{
	register LONG sigs;
	register LONG waitsigs;
	register struct timerequest *timer = TimerIOB;

	if (!timeropen) return(-1);
	StartTimer(secs, micro);
	sigs = 0;
	waitsigs = (1L << serport->mp_SigBit) | intsigs;
	for (;;)
	{
		if (CheckIO(timer))
		{
			WaitIO(timer);
			return(0);
		}
		if (sigs & intsigs)
		{
			KillIO(timer);
			testint(sigs);
			return(-1);
		}
		sigs = Wait(waitsigs);
	}
}

/*
 * sleep -- wait n seconds
 */
sleep(n)
int n;
{	return(Sleeper((LONG)n, 0L)); }

/*
 * msleep -- wait n milliseconds
 */
msleep(m)
int m;
{	return(Sleeper((LONG)(m / 1000), (m % 1000) * 1000L)); } 


/*
 * rtimer -- reset elapsed time
 */
rtimer()
{	DateStamp(&prevtime); }

/*
 * gtimer -- get currently elapsed time in seconds
 */
gtimer()
{
	int x;
	struct DateStamp curtime;

	DateStamp(&curtime);
	x = ((curtime.ds_Days   - prevtime.ds_Days  ) * 1440 +
	     (curtime.ds_Minute - prevtime.ds_Minute) ) * 60 +
	     (curtime.ds_Tick   - prevtime.ds_Tick  ) / 50;
	return((x < 0) ? 0 : x );
}

/*
 * ztime -- format current date and time into string
 */
ztime(s)
char **s;
{
   /*
    * The following date code taken from a USENET article by
    *    Tomas Rokicki(rokicki@Navajo.ARPA)
    */
   static char *months[] = { NULL,
      "January","February","March","April","May","June",
      "July","August","September","October","November","December"};
   static char buf[32];

   long n ;
   int m, d, y ;
   struct DateStamp datetime;

   DateStamp(&datetime);

   n = datetime.ds_Days - 2251 ;
   y = (4 * n + 3) / 1461 ;
   n -= 1461 * y / 4 ;
   y += 1984 ;
   m = (5 * n + 2) / 153 ;
   d = n - (153 * m + 2) / 5 + 1 ;
   m += 3 ;
   if (m > 12) {
      y++ ;
      m -= 12 ;
   }
   sprintf(buf, "%02d:%02d:%02d %s %d, %d",
           datetime.ds_Minute / 60, datetime.ds_Minute % 60,
	   datetime.ds_Tick / 50, months[m], d, y) ;
   *s = buf;
}

/*
 * congm -- save console modes
 */
congm()
{
	if (!saverr) saverr = DOSFH(2);
	return(0);
}

/*
 * CreateWindow -- create window and jam it into standard I/O
 */
int CreateWindow(esc)
{
	if (rawcon > 0) return(0);
	congm();

#ifdef LAT310
	if ((rawcon = open("RAW:0/0/640/200/Kermit", O_RDWR)) <= 0)
		return(-1);
#else
	if ((rawcon = Open("RAW:0/0/640/200/Kermit", (LONG)MODE_NEWFILE)) == 0)
		return(-1);
#endif
	DOSFH(0) = DOSFH(1) = DOSFH(2) = rawcon;

	/* if we create a window, don't abort on errors or echo */
	backgrd = FALSE;
	ckxech = 1;
	return(0);
}

/*
 * concb -- put console in single character wakeup mode
 */
concb(esc)
{
	if (rawcon) return(0);
	if (CurCLI && CurProc->pr_CIS != CurCLI->cli_StandardInput)
		return(0);
	return(CreateWindow(esc));
}

/*
 * conbin -- put console in raw mode
 */
conbin(esc)
{
	if (rawcon) return(0);
	if (CurCLI && CurProc->pr_CIS != CurCLI->cli_StandardInput)
		return(isatty(0) ? 0 : -1);
	return(CreateWindow(esc));
}
 
/*
 * conres -- restore console
 *    we actually restore in syscleanup()
 */
conres()
{	return(0); }

/*
 * conoc -- output character to console
 */
conoc(c)
{
	putchar(c);
	fflush(stdout);
	testint(0L);
}

/*
 * conxo -- output x chars to console
 */ 
conxo(n, buf)
char *buf;
int n;
{
	fflush(stdout);
	write(FILENO(1), buf, n);
	testint(0L);
}

/*
 * conol -- output line to console
 */
conol(l)
char *l;
{
	fputs(l, stdout);
	fflush(stdout);
	testint(0L);
}

/*
 * conola -- output line array to console
 */
conola(l)
char **l;
{
	for (; **l; ++l) conol(*l);
}

/*
 * conoll -- output line with CRLF
 */
conoll(l) char *l;
{
    conol(l);
    conxo(2, "\r\n");
}

/*
 * conchk -- returns nonzero if characters available from console
 */
conchk()
{
	fflush(stdout);
	testint(0L);
	return(WaitForChar(DOSFH(0), 0L) != 0);
}

/*
 * coninc -- get input character from console
 */
coninc(timeout)
int timeout;
{
	UBYTE ch;

	fflush(stdout);
	testint(0L);
	if (timeout > 0 && !WaitForChar(DOSFH(0), timeout * 1000000L))
		return(-1);
	if (read(FILENO(0), &ch, 1) < 1) return(-1);
	testint(0L);
	return((int)ch);
}

/*
 * ttsndb -- send a BREAK
 *    flushes queued and active output
 */
ttsndb()
{

	if (!serialopen) return(-1);
	/* flush queued output */
	TerminateWrite(1);
	nttoq = 0;
	pendwrite = TRUE;
	WriteIOB->IOSer.io_Command = SDCMD_BREAK;
	WriteIOB->io_SerFlags &= ~SERF_QUEUEDBRK;
	SendIO(WriteIOB);
	return(0);
}

/*
 * ttocq -- write char to serial device, queueing if necessary
 *    returns -2 on overrun, -1 on serial error
 *    use only in connect mode
 */
ttocq(c)
char c;
{
	register int i;

	if (!serialopen) return(-1);
	if (pendwrite && CheckIO(WriteIOB))
	{
		pendwrite = FALSE;
		if (WaitIO(WriteIOB) != 0) return(-1);
	}
	if (pendwrite)
	{
		if (nttoq >= NTTOQ) return(-2);		/* overrun */
		ttoq[(pttoq + nttoq++) % NTTOQ] = c;
	}
	else if (nttoq == 0)
		return(ttoc(c));
	else
	{
		i = ttoc(ttoq[pttoq]);
		ttoq[(pttoq + nttoq) % NTTOQ] = c;
		pttoq = (pttoq + 1) % NTTOQ;
		if (i < 0) return(-1);
	}
	return(1);
}

/*
 * ttonq -- returns number of characters in serial output queue
 */
int ttonq()
{	return(nttoq); }

/*
 * conttb -- prepare for contti() usage
 */
conttb()
{
	/* flush queued input and output */
	queuedcon = -1;
	pttoq = nttoq = 0;
}

/*
 * contte -- end contti() usage
 *    this can be called after a tthang, it which case ttres will already
 *    have done this cleanup
 */
contte()
{
	/* clear any pending ^C, ^D interrupts */
	testint(0L);

	/* terminate any pending I/O */
	if (serialopen) SerialReset();
}

/*
 * contti -- wait for console or tty input
 *    returns next console input or -1 when serial input available
 */
int contti()
{
	register int i;
	register LONG waitsigs;
	register struct DosPacket *pkt = conpkt;
	register struct IOExtSer *read = ReadIOB;
	static UBYTE conchar;
	BPTR dosfh = DOSFH(0);
	struct FileHandle *fh = (struct FileHandle *)BADDR(dosfh);

	if (queuedcon >= 0)
	{
		conchar = queuedcon;
		queuedcon = -1;
		return((int)conchar);
	}

	if (!pendconsole)
	{	/* start a console read */
		pkt->dp_Port = conport;
		pkt->dp_Type = ACTION_READ;
		pkt->dp_Arg1 = (LONG)dosfh;
		pkt->dp_Arg2 = (LONG)&conchar;
		pkt->dp_Arg3 = 1;
		PutMsg(fh->fh_Process, pkt->dp_Link);
		pendconsole = TRUE;
	}

	if (queuedser < 0 && !pendread)
	{	/* start a serial read */
		read->IOSer.io_Command = CMD_READ;
		read->IOSer.io_Data    = (APTR)&serbufc;
		read->IOSer.io_Length  = 1;
		SendIO(read);
		pendread = TRUE;
	}

	waitsigs = (1L << serport->mp_SigBit) | (1L << conport->mp_SigBit);
	for (;;)
	{
		if (pendwrite && CheckIO(WriteIOB))
		{
			pendwrite = FALSE;
			if (nttoq > 0)
			{
				i = ttoc(ttoq[pttoq]);
				pttoq = (pttoq + 1) % NTTOQ;
				--nttoq;
				if (i < 0) return(-1);
			}
		}

		/* give the console first chance */
		if (GetMsg(conport))
		{
			pendconsole = FALSE;
			if (pkt->dp_Res1 != 1) return(-1);
			/* translate CSI to ESC [ */
			if (conchar == 0x9B)
			{	conchar = 0x1B; queuedcon = '['; }
			return((int)conchar);
		}

		if (queuedser >= 0) return(-2);

		if (CheckIO(read))
			return((TerminateRead() == 0) ? -2 : -1);

		Wait(waitsigs);
	}
}
