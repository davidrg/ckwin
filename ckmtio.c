/* edit 3/25/87 by Paul Placeway -- don't let the TextEdit buffer get too */
/*  big, because the Mac will crash if it does */
/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */

/*  C K M T I O  --  interrupt, console, and port functions for Mac Kermit  */

/*
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/

/*
 Variables:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console, 1 if external line.
   dfprty -- Default parity
   dfflow -- Default flow control
   ckxech -- Flag for who echoes console typein:
     1 - The program (system echo is turned off)
     0 - The system (or front end, or terminal).
   functions that want to do their own echoing should check this flag
   before doing so.

 Functions for assigned communication line (either external or console tty):

   ttopen(ttname,local,modem) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttpkt(speed,flow)       -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.

Functions for console terminal:

   congm()   -- Get console terminal modes.
   concb()   -- Put the console in single-character wakeup mode with no echo.
   conbin()  -- Put the console in binary (raw) mode.
   conres()  -- Restore the console to mode obtained by congm().
   conoc(c)  -- Unbuffered output, one character to console.
   conol(s)  -- Unbuffered output, null-terminated string to the console.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console.
   coninc()  -- Get a character from the console.
   conint()  -- Enable terminal interrupts on the console.
   connoi()  -- Disable terminal interrupts on the console.

Time functions

   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
*/

#include "ckcdeb.h"		/* Formats for debug() */

#define	__SEG__ ckmtio
#include <files.h>
#include <textedit.h>
#include <packages.h>
#include <events.h>
#include <controls.h>
#include <devices.h>
#include <serial.h>

#include "ckcker.h"		/* kermit defs */
#include "ckmdef.h"		/* macintosh defs */

char *dftty = ".AIn";

/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

int dfloc = 1;

/* Other defaults */

int dfprty = 0;			/* Default parity (necessary for ckcmai.c
				 * only) */
int dfflow = 1;			/* Xon/Xoff flow control */

int backgrd = 0;		/* a Mac is allways in foreground */

/* Local variables */

int drop_dtr = 0;		/* drop DTR on Quit */

/* buffer and pointer for input processing */
static char *my_input_buf;

#define TTBUFL 200		/* good size (it's RBUFL guys!) */
static unsigned char *ttbuf;
static int ttbufi = 0;
static long ttbufm = 0;		/* input pointer, max */

static char *tto_buf;	/* output buffer for ttol() */
static IOParam iopb;		/* paramater block for ttol() */
static long tto_startticks;	/* when we started the output */
#define XOFFTIMEO 480L		/* a timeout of 8 seconds (in ticks) */

short dfltVol;			/* the volume to be used for Take- and
				 * Log-File */

extern Boolean usingRAMdriver,	/* true if using RAM serial driver (usually TRUE) */
	       have_128roms;	/* true if we are a Plus or better */

/****************************************************************************/
/*  S Y S I N I T  --  System-dependent program initialization.  */
/****************************************************************************/
sysinit ()
{
    VolumeParam volinfo;
    extern char *NewPtr();
    
    mac_init ();		/* Set up the Mac */

    /* get the default volume reference number for Take- and Log-Files */
    volinfo.ioNamePtr = NIL;
    PBGetVol (&volinfo, FALSE);
    dfltVol = volinfo.ioVRefNum;

    findfinderfiles ();		/* see if file was selected */
    return (0);
}				/* sysinit */

/* init terminal I/O buffers */
inittiobufs()
{
    if ((ttbuf = (unsigned char *) NewPtr(TTBUFL + 1)) == NIL)
    	fatal("Can't allocate ttbuf", 0);
    if ((tto_buf = (char *) NewPtr(MAXSP)) == NIL)
    	fatal("Can't allocate tto_buf", 0);
    if ((my_input_buf = (char *) NewPtr(MYBUFSIZE)) == NIL)
    	fatal("Can't allocate my_input_buf", 0);
}

/****************************************************************************/
/* P O R T _ O P E N -- Open and init a serial port.  port is either -6 (for */
/*  the modem port) or -8 (for the printer port)			    */
/****************************************************************************/

port_open(port)
int port;
{
    int err;
        
    if ((port != -6) && (port != -8))	/* sanity check */
	port = -6;
	
try_again:
    /* Set up IO drivers */
    innum = port;
    outnum = port - 1;
    if (innum == -6) {
	if (((err = OpenDriver (".AIn", &innum)) != noErr) ||
	    ((err = OpenDriver (".AOut", &outnum)) != noErr))
	    fatal ("Could not open the Modem port: ", err);
    } else {
	if (((err = OpenDriver (".BIn", &innum)) != noErr) ||
	    ((err = OpenDriver (".BOut", &outnum)) != noErr)) {
	    printerr("Could not open the Printer port.  Try\
 turning off Appletalk.", 0);
	    port = -6;
	    goto try_again;
	}
    }
    
    /* try for the RAM driver */
    if (innum == -6)
	err = RAMSDOpen (sPortA);
    else
	err = RAMSDOpen (sPortB);
	
    if (err == noErr) {
	usingRAMdriver = TRUE;
    } else {
	usingRAMdriver = FALSE;
	printerr("Can't open RAM serial driver; using the ROM driver\
 (without flow control).",0);
    }

    err = SerSetBuf (innum, my_input_buf, MYBUFSIZE);
			/* Make driver use larger buff */
    if (err)
	printerr ("Trouble making IO buffer:", err);
}

/*****************************************************************************/
/* P O R T _ C L O S E -- Close down the serial port.			     */
/*****************************************************************************/

port_close()
{
    int err;
    CntrlParam cpb;
    
    err = KillIO(innum);	/* Kill off IO drivers */
    if (err != noErr)
    	printerr("trouble KillIO-ing serial input driver:",err);
    err = KillIO(outnum);	/* Kill off IO drivers */
    if (err != noErr)
    	printerr("trouble KillIO-ing serial output driver:",err);

    err = SerSetBuf (innum, NULL, 0);	/* Make driver default buffer */
    if (err != noErr)
    	printerr("trouble resetting serial IO buffer:",err);

    if (usingRAMdriver) {
	if (!drop_dtr) {
	    cpb.ioNamePtr = NULL;	/* see TN 174 */
	    /* tell the ram driver not to lower DTR on close */
	    cpb.csCode = 13;		/* misc serial control */
	    *((unsigned char *) cpb.csParam) = 0x80;	/* don't lower DTR */
	    cpb.ioCRefNum = innum;
	    err = PBControl(&cpb, FALSE);
	    if ((err != noErr) && (err != -1))
    		printerr("trouble telling RAM serial driver (in) not to lower DTR:",err);

	    cpb.ioNamePtr = NULL;	/* see TN 174 */
	    cpb.csCode = 13;		/* misc serial control */
	    *((unsigned char *) cpb.csParam) = 0x80;	/* don't lower DTR */
	    cpb.ioCRefNum = outnum;
	    err = PBControl(&cpb, FALSE);
	    if ((err != noErr) && (err != -1))
    		printerr("trouble telling RAM serial driver (out) not to lower DTR:",err);
	}
	
	err = RAMSDClose (sPortA);
	/* PWP: I don't know why this returns -1 on a Mac II */
	if ((err != noErr) && (err != -1))
    	    printerr("trouble closing RAM serial driver:",err);
    }
    
    err = CloseDriver (innum);
    if (err != noErr)
	printerr("trouble closing serial input driver:",err);
/*
 * For some reason or other, doing this close on a 64k ROM machine will cause
 * the mouse to freeze.  Since the input driver is the only one that really
 * matters, we just close it.
 */
    if (have_128roms) {
	err = CloseDriver (outnum);
	if (err != noErr)
	    printerr("trouble closing serial output driver:",err);
    }
}

/****************************************************************************/
/*  T T O P E N  --  Open a tty for exclusive access.  */
/*  Returns 0 on success, -1 on failure.  */
/****************************************************************************/
ttopen (ttname, lcl, modem)
char *ttname;
int *lcl;
int modem;
{
    ttbufi = ttbufm = 0;	/* init buffer */
    if (*lcl < 0)
	*lcl = 1;		/* always in local mode */

    if (ttbuf == NIL)
    	fatal("No ttbuf", 0);
    if (tto_buf == NIL)
    	fatal("No tto_buf", 0);
    if (my_input_buf == NIL)
    	fatal("No my_input_buf", 0);

    iopb.ioResult = 0;		/* no pending output */
    
    iopb.ioActCount = 0;	/* for error checking in ttol() */
    iopb.ioReqCount = 0;
    
    return (0);
}				/* ttopen */



/****************************************************************************/
/*  T T C L O S  --  Close the TTY, releasing any lock.  */
/****************************************************************************/
ttclos ()
{
    ttbufi = ttbufm = 0;	/* init buffer */
    return;
}				/* ttclos */

/****************************************************************************/
/*  T T P K T  --  Condition the communication line for packets. */
/*  If called with speed > -1, also set the speed.  */
/*  Returns 0 on success, -1 on failure.  */
/****************************************************************************/
ttpkt (speed, flow, parity)	/* we only care about flow here */
int speed;
int flow;
int parity;
{
    int err1, err2;
    SerShk controlparam;	/* To change serial driver paramaters */

    if (flow)
	controlparam.fXOn = TRUE;	/* obey flow control */
    else
	controlparam.fXOn = FALSE;	/* ignore flow control */
    controlparam.fCTS = FALSE;
    controlparam.xOn = 17;
    controlparam.xOff = 19;
    controlparam.errs = FALSE;
    controlparam.evts = FALSE;
    if (flow && usingRAMdriver)		/* old ROM driver can't do this */
	controlparam.fInX = TRUE;	/* send flow control when almost full */
    else
	controlparam.fInX = FALSE;

    err1 = SerHShake (outnum, &controlparam);
    if (err1)
	printerr ("ttpkt(): Trouble with output handshake: ", err1);
    err2 = SerHShake (innum, &controlparam);
    if (err2)
	printerr ("ttpkt(): Trouble with input handshake: ", err2);

    if (err1 || err2)
	return (-1);
    else
	return (0);
}				/* ttpkt */

/****************************************************************************/
/*  T T V T  --  Condition the communication line for a virtual terminal. */
/*  If called with speed > -1, also set the speed.  */
/*  Returns 0 on success, -1 on failure.  */
/****************************************************************************/
ttvt (speed, flow, parity)	/* all ignoreed */
int speed;
int flow;
int parity;
{
    (void) ttres();
    return (0);
}				/* ttvt */


/****************************************************************************/
/*  T T F L U I  --  Flush tty input buffer */
/****************************************************************************/
ttflui ()
{
    int err;

    err = KillIO (-6);
    if (err)
	printerr ("Bad input clear", err);
    ttbufi = ttbufm = 0;	/* init buffer */
    return (0);
}				/* ttflui */



/* Interrupt Functions */

/****************************************************************************/
/* Set up terminal interrupts on console terminal */
/* Set an interrupt trap. */
/****************************************************************************/
conint (f)
int (*f) ();
{
    return;
}				/* conint */



/****************************************************************************/
/* Reset console terminal interrupts */
/****************************************************************************/
connoi ()
{
    return;
}				/* connoi */

/****************************************************************************/
/* writeps - write a pascal string to the serial port.
 *
 */
/****************************************************************************/
writeps (s)
char *s;
{
    long wcnt, w2;
    int err;
    char *s2;

    w2 = wcnt = *s++;		/* get count */

    for (s2 = s; w2 > 0; w2--, s2++)	/* add parity */
	*s2 = dopar (*s2);
    
    (void) ttol(s, wcnt);	/* ttol will printerr() if it has a problem */

    return;
}				/* writeps */

extern Boolean have_multifinder;/* true if running under MF */
extern Boolean in_background;	/* becomes true if running MF and in bg */
extern long mf_sleep_time;	/* number of 60ths between task time */

/****************************************************************************/
/*  T T O L  --  Similar to "ttinl", but for writing.  */
/****************************************************************************/
ttol (s, n)
char *s;
int n;
{
    long finalticks;
    int err;
    long wrcnt;			/* pascal long */
    int ammount, nc, num;
    CntrlParam cpb;

    /* wait for previous call to finish */
    while (iopb.ioResult == 1) {	/* while the prev. request is still running */
	if (have_multifinder && protocmd != 0) {	/* if we're running protocol */
	    miniparser (TRUE);	/* keep mac running */
	    finalticks = TickCount ();
	} else {		/* else terminal emulation */
	    Delay ((long) 1, &finalticks);	/* wait for a bit */
	}
	/* (PWP) If we have waited too long, unblock the output (keep the
	   Mac from freezing up) */
	if ((usingRAMdriver) && (finalticks > (tto_startticks + XOFFTIMEO))) {
	    cpb.csCode = 22;		/* clear XOFF for my output */
	    cpb.ioCRefNum = outnum;
	    err = PBControl (&cpb, FALSE);
	    if (err != noErr)
		printerr ("ttol() trouble unblocking output port: ", err);
	    tto_startticks = TickCount ();	/* get starting time */
	}
    }
    
    /* check for errors in previous call */
    if (iopb.ioResult) {
	printerr ("Error in previous PBWrite:", iopb.ioResult);
	return (-1);
    }
    if (iopb.ioActCount != iopb.ioReqCount) {
	printerr ("PBWrite to serial didn't write enough: ", iopb.ioActCount);
	printerr ("(asked for:)", iopb.ioReqCount);
	return (-1);
    }

    /* the previous call is now done, so we can load in our new information */
    strncpy(tto_buf, s, MAXSP);	/* MAXSP == sizeof(tto_buf) */

    iopb.ioCompletion = NULL;
    iopb.ioNamePtr = NULL;
    iopb.ioVRefNum = 0;

    iopb.ioRefNum = outnum;
    iopb.ioVersNum = 0;
    iopb.ioPermssn = 0;
    iopb.ioMisc = NULL;
    iopb.ioBuffer = tto_buf;
    iopb.ioReqCount = (long) n;
    iopb.ioPosMode = 0;
    iopb.ioPosOffset = 0;
    
    tto_startticks = TickCount ();	/* get starting time */

    PBWrite (&iopb, TRUE);	/* request an async. write */
    if (protocmd != 0)
	miniparser (TRUE);	/* allow other tasks to run */
    return (n);			/* fake a good run */
}				/* ttol */



/****************************************************************************/
/*  T T I N L  --  Read a record (up to break character) from comm line.  */
/*
  If no break character encountered within "max", return "max" characters,
  with disposition of any remaining characters undefined.  Otherwise, return
  the characters that were read, including the break character, in "dest" and
  the number of characters read as the value of function, or 0 upon end of
  file, or -1 if an error occurred.  Times out & returns error if not completed
  within "timo" seconds.
*/
/****************************************************************************/


/* DELAYTICKS is the dismiss value in 60th of a second when the input */
/* buffer goes empty.  Setting this value too small causes more overhead */
/* since we'll get very few character on the read.  Setting this value */
/* too large causes a slowdown if characters are on their way... */
#define DELAYTICKS	6		/* 15/60th of second retry */


ttinl (dest, max, timo, eol)
int max;
int timo;
char *dest;
char eol;
{
    register int i, j;
    register int m;
    long n;		/* can't be register */
    long finaltics;	/* can't be register */
    long intim = TickCount ();	/* when we started */
    int err;

    m = (parity) ? 0177 : 0377;	/* Parity stripping mask. */

#ifdef COMMENT
    if (in_background) {
	/* see the comment on this in ttol() */

	nc = (speed * (int) mf_sleep_time) / 600;
	/* nc is really the number of CHARS we can send in */
	/* one mf_sleep_time */
	if (nc < 10)
	    nc = 10;		/* set a reasonable minimum */
    }
#endif
    
    /* Refill buffer with available bytes, if none left retry until timo */
    i = 0;
    j = 0;
    for (;;) {
	SerGetBuf (innum, &n);	/* Get available count */
	if (n > 0) {	/* Have something? */
	    if (n > (max - j)) n = max - j;
	    if (n > 0) {
		err = FSRead (innum, &n, dest + i);    /* Into our buffer */
		if (err != noErr) {
		    screen(SCR_EM,0,(long) err,"Serial input error:");
		    return (-1);
		}
	    } else {
		n = 0;		/* (PWP) just in case */
	    }

	    j = i + n;		/* Remember next buffer position. */
	    if (j >= max) {
		screen(SCR_EM,0,(long) j,"ttinl buffer overflow:");
		return (-1);
	    }
	    for (i; i < j; i++) {       /* Go thru all chars we just got */
		dest[i] &= m;           /* Strip any parity */
		if (dest[i] == eol) {   /* Got eol? */
		    dest[++i] = '\0';     /* Yes, tie off string, */
		    return(i);
		}
	    }
	    if (j >= max) {		/* Full buffer? */
		dest[++j] = '\0';	/* Yes, tie off string, */
		return(j);
	    }
	} else {	/* no chars availiable yet */
	    if (protocmd != 0) {	/* if we're running protocol */
		miniparser (TRUE);	/* keep mac running */
		if (sstate == 'a')	/* abort occured? */
		    return (-1);	/* ugh, look like timeout */
	    }
#ifdef OLD_WAY
	    if (have_multifinder) {
		miniparser (TRUE);	/* allow other tasks to run */
		finaltics = TickCount ();
	    } else {
		Delay ((long) DELAYTICKS, &finaltics);	/* Delay for a little */
	    }
#else
	    finaltics = TickCount ();
#endif
	    if (timo > 0) {		/* Want to do timeout? */
		if (intim + timo * 60 < finaltics)
		    return (-1);	/* Too long, give up */
	    }
	    continue;
	}
	/* not at end of packet yet, so let other tasks have a chance */
	if (in_background)
	    miniparser (TRUE);	/* allow other tasks to run */
    }				/* end for ever */
    printerr("ttinl BUG: we should not have gotten here", 0);
    return (-1);
}				/* ttinl */


/****************************************************************************/
/* ttinc(timo) - read a character with timeout.  Return -1 on timeout. */
/****************************************************************************/
ttinc (timo)
int timo;
{
    long avcnt;
    long finaltics;
    long intim = 0;
    int err;

    if (ttbufi < ttbufm)
	return (ttbuf[ttbufi++]);	/* Return char quickly */

    /* Refill buffer with available bytes, if none left retry until timo */
    for (;;) {
	SerGetBuf (innum, &avcnt);	/* Get available count */
	if (avcnt > 0) {	/* Have something? */
	    ttbufi = 0;		/* Init input buffer ptr */
	    ttbufm = (avcnt > TTBUFL) ? TTBUFL : avcnt;	/* Set max */
	    err = FSRead (innum, &ttbufm, ttbuf);	/* Into our buffer */
	    if (err != noErr)
		printerr ("Serial input error: ", err);
	    return (ttbuf[ttbufi++]);	/* And return first character */
	}
	if (protocmd != 0) {	/* if we're running protocol */
	    miniparser (TRUE);	/* keep mac running */
	    if (sstate == 'a')	/* abort occured? */
		return (-1);	/* ugh, look like timeout */
	}
	Delay ((long) DELAYTICKS, &finaltics);	/* Delay for a little */
	if (timo > 0) {		/* Want to do timeout? */
	    if (intim == 0)	/* Do we know time in? */
		intim = finaltics - DELAYTICKS;	/* no, initialize it */
	    if (intim + timo * 60 < finaltics)
		return (-1);	/* Too long, give up */
	}
    }				/* for */
}				/* ttinc */

/****************************************************************************/
/* PWP: input as many characters as we can read from the serial line right now */
/****************************************************************************/

ttinm(buf, max)
register char *buf;
register int max;
{
    long avil, num;
    int err;
    
    if (ttbufi < ttbufm) {		/* err is a counter here  */
    	for (err = 0; (ttbufi < ttbufm) && (err < max); err++)
	    *buf++ = ttbuf[ttbufi++];
	return (err);
    }
    SerGetBuf (innum, &avil);	/* Get available count */
    if (avil > 0) {	/* Have something? */
	ttbufi = 0;		/* Init input buffer ptr */
	num = (avil > max) ? max : avil;	/* Set max */
	err = FSRead (innum, &num, buf);	/* Into our buffer */
	if (err != noErr)
	    printerr ("Serial input error: ", err);
	return (num);		/* return how many */
    } else {
    	return (0);
    }
}

/****************************************************************************/
/****************************************************************************/
ttchk ()
{
    long avcnt;			/* pascal long */

    SerGetBuf (innum, &avcnt);	/* get available */
    return (avcnt + (ttbufm - ttbufi));	/* return avail plus our own */
}				/* ttchk */



/****************************************************************************/
/* T T R E S -- Reset the serial line after doing a protocol things	    */
/****************************************************************************/
ttres ()
{
    int err1, err2;
    SerShk controlparam;	/* To change serial driver paramaters */

    if (flow)
	controlparam.fXOn = TRUE;	/* obey flow control */
    else
	controlparam.fXOn = FALSE;	/* ignore flow control */
    controlparam.fCTS = FALSE;
    controlparam.xOn = 17;
    controlparam.xOff = 19;
    controlparam.errs = FALSE;
    controlparam.evts = FALSE;
    controlparam.fInX = FALSE;

    err1 = SerHShake (outnum, &controlparam);
    if (err1)
	printerr ("ttvt(): Trouble with output handshake: ", err1);
    err2 = SerHShake (innum, &controlparam);
    if (err2)
	printerr ("ttvt(): Trouble with input handshake: ", err2);

    if (err1 || err2)
	return (-1);
    else
	return (1);
}				/* ttres */



long starttime;
Boolean timerRunning = FALSE;

/****************************************************************************/
/*  R T I M E R --  Reset elapsed time counter  */
/****************************************************************************/
rtimer ()
{
    GetDateTime (&starttime);
    timerRunning = TRUE;
}				/* rtimer */



/****************************************************************************/
/*  G T I M E R --  Get current value of elapsed time counter in seconds  */
/****************************************************************************/
long
gtimer ()
{
    long secs;

    if (timerRunning) {
	GetDateTime (&secs);
	return (secs - starttime);
	timerRunning = FALSE;
    } else
	return (0);
}				/* gtimer */


char dtime[25];

/****************************************************************************/
/*  Z T I M E  --  Return date/time string  */
/****************************************************************************/
ztime (s)
char **s;
{
    long secs;
    char timestr[10];

    GetDateTime (&secs);
    IUDateString (secs, shortDate, dtime);
    IUTimeString (secs, FALSE, timestr);
    strcat (dtime, " ");
    strcat (dtime, timestr);
    *s = dtime;
}				/* ztime */



/* Console IO routines.  The console is implemented as a text edit structure.
 * These routines are supported:
 *
 * conoc(c)   -- output one character to TE record at insertion point
 * conol(s)   -- output null terminated string to TE record " "
 * conoll(s)  -- same but with CR on the end
 * conxo(n,s) -- n character to TE record " "
 *
 */

#define NILTE ((TEHandle ) NILPTR)
#define LF 012
#define CR 015

#define TE_TOOBIG	32000	/* should be less than 32k-1 */
#define TE_TRIM		512	/* should be some approiate size */
#define TE_MAX		32767	/* the maximum size of text in a TE buffer */

TEHandle consoleTE = NILTE;	/* storage for console TE ptr */

/****************************************************************************/
/****************************************************************************/
consette (t)
TEHandle t;
{
    if (consoleTE != NILTE)	/* already have TE record */
	printerr ("TE record present at consette! ", 0);
    consoleTE = t;
}				/* consette */

/****************************************************************************/
/* PWP: trimcon(length) should be called before TEInsert()ing <length>    */
/*      number of bytes.  This function checks to see if there is enough  */
/*	room for this many characters, and deletes a bit from the beginning  */
/*	if there isn't space.  */
/****************************************************************************/
static
trimcon(l)
register int l;
{
    if (consoleTE != NILTE) {
	if (((*consoleTE)->teLength + l) > TE_TOOBIG) {	    
	    TESetSelect(0, TE_TRIM, consoleTE);	/* select the beginning part */
	    TEDelete(consoleTE);		/* nuke it */
	    	/* and make insertion point at end again */
	    TESetSelect(TE_MAX, TE_MAX, consoleTE);
	}
    }
}

/****************************************************************************/
/*  C O N O C  --  Output a character to the console text edit record */
/****************************************************************************/
conoc (c)
char c;
{
    static long len = 1;

    if (c == LF)		/* we don't support this */
	return;

    if (consoleTE != NILTE) {	/* is it present? */
	trimcon(1);
	TEInsert (&c, len, consoleTE);	/* insert the char */

	if (c == CR)		/* if CR */
	    rcdwscroll ();	/* then possibly scroll it */
    }
}				/* conoc */



/****************************************************************************/
/****************************************************************************/
conopen ()
{
    if (consoleTE == NILTE)
	printerr ("Tried to open console before TE set", 0);
    return (1);
}				/* conopen */



/****************************************************************************/
/*  C O N O L  --  Write a line to the console text edit record  */
/****************************************************************************/
conol (s)
register char *s;
{
    long len = strlen (s);	/* fetch length */

    debug (F101, "conol here: ", s, 0);
    if (consoleTE != NILTE) {	/* is it present? */
	trimcon(len);
	TEInsert (s, len, consoleTE);	/* insert the string */
    }
    return (0);
}				/* conol */



/****************************************************************************/
/****************************************************************************/
conxo (s, len)
register char *s;
{
    debug (F101, "conxo here: ", s, len);
    if (consoleTE != NILTE) {	/* is it present? */
	trimcon(len);
	TEInsert (s, (long) len, consoleTE);	/* insert the string */
	rcdwscroll ();		/* possibly scroll it */
    }
    return (0);
}				/* conxo */



/****************************************************************************/
/*  C O N O L L  --  Output a string followed by CRLF  */
/****************************************************************************/
conoll (s)
char *s;
{
    conol (s);			/* first the string */
    conoc (CR);			/* now the return */
    return (0);
}				/* conoll */
