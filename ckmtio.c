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

#include "ckmdef.h"		/* macintosh defs */

char *dftty = "";

/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

int dfloc = 1;

/* Other defaults */

int dfprty = 0;			/* Default parity (necessary for ckcmai.c
				 * only) */
int dfflow = 1;			/* Xon/Xoff flow control */

/* buffer and pointer for input processing */

#define TTBUFL 200		/* good size (it's RBUFL guys!) */
unsigned char ttbuf[TTBUFL + 1];
int ttbufi = 0;
long ttbufm = 0;		/* input pointer, max */



short dfltVol;			/* the volume to be used for Take- and
				 * Log-File */

/****************************************************************************/
/*  S Y S I N I T  --  System-dependent program initialization.  */
/****************************************************************************/
sysinit ()
{
    VolumeParam volinfo;

    mac_init ();		/* Set up the Mac */

    /* get the default volume reference number for Take- and Log-Files */
    volinfo.ioNamePtr = NIL;
    PBGetVol (&volinfo, FALSE);
    dfltVol = volinfo.ioVRefNum;

    findfinderfiles ();		/* see if file was selected */
    return (0);
}				/* sysinit */



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
ttpkt (speed, flow)
int speed;
int flow;
{
    return (0);
}				/* ttpkt */



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
    int err;
    long wrcnt;			/* pascal long */
    int ammount, nc, num;

    if (have_multifinder) {

	/*
	 * if we have MF, we want to actually run only about 1/2 of the time,
	 * so as to be nice to the things running in the background.
	 */

	nc = (speed * (int) mf_sleep_time) / 600;
	/* nc is really the number of CHARS we can send in */
	/* one mf_sleep_time */
	if (nc < 10)
	    nc = 10;		/* set a reasonable minimum */
	num = n;

	while (num > 0) {	/* while we have chars to write out */
	    ammount = (num < nc) ? num : nc;
	    wrcnt = (long) ammount;
	    err = FSWrite (outnum, &wrcnt, s);
	    if (err)		/* actually, we SHOULD get in fg first */
		printerr ("Bad Writeout:", err);
	    num -= ammount;	/* decrement the number chars to write */
	    s += ammount;	/* forward the index to data */
	    miniparser (TRUE);	/* allow other tasks to run */
	}

    } else {			/* else no MF.  Just output the whole
				 * bunch... */

	wrcnt = n;
	err = FSWrite (outnum, &wrcnt, s);
	if (err)
	    printerr ("Bad Writeout:", err);
    }
    return (n);
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
ttinl (dest, max, timo, eol)
int max;
int timo;
char *dest;
char eol;
{
    register int x, c;
    int m, nc;
    m = (parity) ? 0177 : 0377;	/* Parity stripping mask. */

    if (have_multifinder) {
	/* see the comment on this in ttol() */

	nc = (speed * (int) mf_sleep_time) / 600;
	/* nc is really the number of CHARS we can send in */
	/* one mf_sleep_time */
	if (nc < 10)
	    nc = 10;		/* set a reasonable minimum */
    }
    for (x = c = 0; (x < max) && ((c & m) != eol); x++) {
	c = ttinc (timo);
	if (c < 0)
	    return (-1);	/* timeout */
	dest[x] = c & m;
	if (have_multifinder && ((x % nc) == 0))
	    miniparser (TRUE);	/* allow other tasks to run */
    }
    return (++x);		/* else return the count. */
}				/* ttinl */



/* DELAYTICKS is the dismiss value in 60th of a second when the input */
/* buffer goes empty.  Setting this value too small causes more overhead */
/* since we'll get very few character on the read.  Setting this value */
/* too large causes a slowdown if characters are on their way... */
#define DELAYTICKS 15		/* 15/60th of second retry */

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
/****************************************************************************/
ttchk ()
{
    long avcnt;			/* pascal long */

    SerGetBuf (innum, &avcnt);	/* get available */
    return (avcnt + (ttbufm - ttbufi));	/* return avail plus our own */
}				/* ttchk */



/****************************************************************************/
/****************************************************************************/
ttres ()
{
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
    IUDateString (secs, shortDate, &dtime);
    IUTimeString (secs, FALSE, &timestr);
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
    return (0);
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
