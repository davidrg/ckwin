char *dialv = "Dial Command, V2.0(013) 29 Aug 89";

/*  C K U D I A	 --  Dialing program for connection to remote system */

/*
 Original author: Herm Fischer (HFISCHER@USC-ECLB).
 Contributed to Columbia University for inclusion in C-Kermit.
 Copyright (C) 1985, Herman Fischer, 16400 Ventura Blvd, Encino CA 91436.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.

 Os9/68k and Microcom modem support by Bob Larson (Blarson@ecla.usc.edu)

 ------

 This module should work under all versions of Unix.  It calls externally
 defined system-depended functions for i/o, but depends upon the existence
 of various modem control functions.

 This module, and the supporting routines in the ckutio.c module, assume
 that the computer and modem properly utilize the following data communi-
 cations signals (that means one should prepare the modem to use, not
 circumvent, these signals):

     Data Terminal Ready:  This signal is asserted by the computer
     when Kermit is about to ask the modem to dial a call, and is
     removed when Kermit wishes to have the modem hang up a call.
     The signal is asserted both while Kermit is asking the modem
     to dial a specific number, and after connection, while Kermit
     is in a data exchange mode.

     Carrier detect:  This signal must be asserted by the modem when
     a carrier is detected from a remote modem on a communications
     circuit.  It must be removed by the modem when the circuit
     disconnects or is hung up.	 (Carrier detect is ignored while
     Kermit is asking the modem to dial the call, because there is
     no consistant usage of this signal during the dialing phase
     among different modem manufacturers.)

*/

/*
 * Modifications:
 *
 *	21-Jul-85	Fixed failure returns hanging on no carrier signal
 *			Requires tthang change too (ckutio.c revision)
 *							-- Herm Fischer
 *
 *	28-Jun-85	Fixed bug with defaulting the modem-failure message
 *			in lbuf.
 *							-- Dan Schullman
 *
 *	27-Jun-85	Merged in code from Joe Orost at Berkeley for
 *			supporting the US Robotics modem, which included
 *			changing the single characters in MDMINF into
 *			multi-character strings and modifying waitFor.
 *							-- Dan Schullman
 *
 *	26-Jun-85	Allow interrupts to be used to abort dialing,
 *			and ring the bell when a connection is made.
 *			Reorganized some of the failure paths to use the
 *			same code, and now close the line on failures.
 *			Allow use of stored numbers with the DF100 and
 *			DF200 modems.  Handlers now declared after the
 *			call to setjmp.
 *							-- Dan Schullman
 *
 *	24-May-85	DF03, DF100-series, DF200-series, and "unknown" modem
 *			support added.	Also restructured the various data
 *			tables, fixed some bugs related to missing data and
 *			missing case labels, and modified the failure message
 *			to display the "reason" given by the modem.
 *							-- Dan Schullman
 *	16-Mar-87	Support for the ATT7300 UNIX PC internal modem was
 *			added.
 *							-- Richard E. Hill
 *
 *	21-Feb-88	Os9/68k and microcom modem support.
 *							-- Bob Larson
 *
 *	14-Mar-88	Rewrite code for ATT7300 (here and in ckutio.c)
 *			Avoids dial(3c) with it's LCK files, hangs up line
 *			correctly, enables user interrupts and timeouts,
 *			turns on/off the system getty() login procedure.
 *			Correct Hayes command sequence at little.
 *			Procedures: attdial, atthang, ongetty, offgetty.
 *			Parts adapted from work of Richard E. Hill and
 *			Kevin O'Gorman.
 *							-- Joe R. Doupnik
 *
 * 13-Jan-89 Add IBM/Siemens/Rolm CBX dialing support.  - F. da Cruz
 *
 *	29-Aug-89	Added support for AT&T 2212C, 2224B, 2224CEO, and
 *			2296A switched-network modems in AT&T mode, and
 *			for the AT&T Digital Terminal Data Module (DTDM).
 *							-- Eric F. Jones
 */

/*
 * To add support for another modem, do the following:
 *
 *	Define a modem number symbol (n_XXX) for it, keeping the list
 *	in alphabetical and numerical order, and renumbering the values
 *	as necessary.
 *
 *	Create a MDMINF structure for it, again keeping the list alphabetical
 *	for sanity's sake.
 *
 *	Add the address of the MDMINF structure to the ptrtab array, again
 *	in alphabetical and numerical order.
 *
 *	Add the "user visible" modem name and corresponding modem number to
 *	the mdmtab array, again in alphabetical order.
 *
 *	Read through the code and add modem-specific sections as necessary.
 */

/*
 * The intent of the "unknown" modem is hopefully to allow KERMIT to support
 * unknown modems by having the user type the entire autodial sequence
 * (possibly including control characters, etc.) as the "phone number".
 * The only reason that the CONNECT command cannot be used to do this is
 * that a remote line cannot normally be opened unless carrier is present.
 *
 * The protocol and other characteristics of this modem are unknown, with
 * some "reasonable" values being chosen for some of them.  The only way to
 * detect if a connection is made is to look for carrier present.
 *
 * SUPPORT IS CURRENTLY ONLY PARTIALLY SKETCHED OUT FOR THIS.  ALSO, IT
 * SHOULD PERHAPS BE HANDLED MUCH EARLIER, SIMPLY READING USER INPUT AND
 * SENDING IT TO THE MODEM AND ECHOING MODEM RESPONSES BACK TO THE USER,
 * ALL THE TIME LOOKING FOR CARRIER.  OF COURSE, THE PROBLEM THEN BECOMES
 * ONE OF ALLOWING THE USER TO CANCEL THE DIALING.  WE COULD CHOOSE SOME
 * PHRASE THAT WOULD PRESUMABLY NEVER BE A PART OF A VALID AUTODIAL SEQUENCE
 * (E.G., "QUIT" and "quit"). -- DS
 */
#include "ckcdeb.h"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "ckcker.h"
#include "ckucmd.h"

#ifndef ZILOG
#include <setjmp.h>			/* Longjumps */
#else
#include <setret.h>
#endif

#ifdef OS2
#define SIGALRM SIGUSR1
void alarm( unsigned );
#endif

extern int flow, local, mdmtyp, quiet, speed, parity, seslog, ttyfd;
extern char ttname[], sesfil[];

#define MDMINF	struct mdminf

MDMINF		/* structure for modem-specific information */
    {
    int		dial_time;	/* time modem allows for dialing (secs) */
    char	*pause_chars;	/* character(s) to tell modem to pause */
    int		pause_time;	/* time associated with pause chars (secs) */
    char	*wake_str;	/* string to wakeup modem & put in cmd mode */
    int		wake_rate;	/* delay between wake_str characters (msecs) */
    char	*wake_prompt;	/* string prompt after wake_str */
    char	*dmode_str;	/* string to put modem in dialing mode */
    char	*dmode_prompt;	/* string prompt for dialing mode */
    char	*dial_str;	/* dialing string, with "%s" for number */
    int		dial_rate;	/* delay between dialing characters (msecs) */
    };

/*
 * Define symbolic modem numbers.
 *
 * The numbers MUST correspond to the ordering of entries
 * within the ptrtab array, and start at one (1).
 *
 * It is assumed that there are relatively few of these
 * values, and that the high(er) bytes of the value may
 * be used for modem-specific mode information.
 *
 * REMEMBER that only the first eight characters of these
 * names are guaranteed to be unique.
 */

#define		n_ATTDTDM	 1
#define		n_ATTMODEM	 2
#define		n_CERMETEK	 3
#define		n_DF03		 4
#define		n_DF100		 5
#define		n_DF200		 6
#define		n_GDC		 7
#define		n_HAYES		 8
#define		n_PENRIL	 9
#define		n_RACAL		10
#define		n_UNKNOWN	11
#define		n_USROBOT	12
#define		n_VENTEL	13
#define		n_CONCORD	14
#define		n_ATTUPC	15	/* aka Unix PC and ATT7300 */
#define		n_ROLM          16      /* Rolm CBX */
#define		n_MICROCOM	17

/*
 * Declare modem "variant" numbers for any of the above for which it is
 * necessary to note various operational modes, using the second byte
 * of a modem number.
 *
 * It is assumed that such modem modes share the same modem-specific
 * information (see MDMINF structure) but may differ in some of the actions
 * that are performed.
 */
#define		n_HAYESNV	( n_HAYES + ( 1<<8 ) )

/*
 * Declare structures containing modem-specific information.
 *
 * REMEMBER that only the first SEVEN characters of these
 * names are guaranteed to be unique.
 */

static
MDMINF ATTMODEM =	/* information for AT&T switched-network modems */
			/* "Number" following "dial" can include: p's and
			 * t's to indicate pulse or tone (default) dialing,
			 * + for wait for dial tone, , for pause, r for
			 * last number dialed, and, except for 2224B, some
			 * comma-delimited options like o12=y, before number.

 * "Important" options for the modems:
 *
 *	All:		Except for 2224B, enable option 12 for "transparent
 *			data," o12=y.  If a computer port used for both
 *			incoming and outgoing calls is connected to the
 *			modem, disable "enter interactive mode on carriage
 *			return," EICR.  The Kermit "dial" command can
 *			function with EIA leads standard, EIAS.
 *
 *	2212C:		Internal hardware switches at their default
 *			positions (four rockers down away from numbers)
 *			unless EICR is not wanted (rocker down at the 4).
 *			For EIAS, rocker down at the 1.
 *
 *	2224B:		Front-panel switch position 1 must be up (at the 1,
 *			closed).  Disable EICR with position 2 down.
 *			For EIAS, position 4 down.
 *			All switches on the back panel down.
 *
 *	2224CEO:	All front-panel switches down except either 5 or 6.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *			Disable EICR with position 3 up.  For EIAS, 1 up.
 *			Reset the modem after changing switches.
 *
 *	2296A:		If option 00 (zeros) is present, use o00=0.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *			Enable modem-port flow control (if available) with
 * 			o42=y.  Enable asynchronous operation with o50=y.
 * 			Disable EICR with o69=n.  For EIAS, o66=n, using
 * 			front panel.
 */
    {
    20,			/* dial_time */
    ",",		/* pause_chars */
    2,			/* pause_time */
    "+",		/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    "",			/* dmode_prompt */
    "at%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF ATTDTDM =	/* information for AT&T Digital Terminal Data Module
 *			For dialing: KYBD switch down, others usually up. */
    {
    5,			/* dial_time */
    "",			/* pause_chars */
    0,			/* pause_time */
    "",			/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    "",			/* dmode_prompt */
    "%s\r",		/* dial_str */		/* not used */
    0			/* dial_rate */
    };

static
MDMINF CERMETEK =	/* information for "Cermetek Info-Mate 212 A" modem */
    {
    20,			/* dial_time */
    "BbPpTt",		/* pause_chars */
    0,			/* pause_time */	/** unknown -- DS **/
    "  XY\016R\r",	/* wake_str */
    200,		/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "\016D '%s'\r",	/* dial_str */
    200			/* dial_rate */
    };

static
MDMINF DF03 =		/* information for "DEC DF03-AC" modem */
    {
    27,			/* dial_time */
    "=",		/* pause_chars */	/* wait for second dial tone */
    15,			/* pause_time */
    "\001\002",		/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "%s",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF DF100 =		/* information for "DEC DF100-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    30,			/* dial_time */
    "=",		/* pause_chars */	/* wait for second dial tone */
    15,			/* pause_time */
    "\001",		/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "%s#",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF DF200 =		/* information for "DEC DF200-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    30,			/* dial_time */
    "=W",		/* pause_chars */	/* =: second tone; W: 5 secs */
    15,			/* pause_time */	/* worst case */
    "\002",		/* wake_str */		/* allow stored number usage */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "%s!",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF GDC =		/* information for "GeneralDataComm 212A/ED" modem */
    {
    32,			/* dial_time */
    "%",		/* pause_chars */
    3,			/* pause_time */
    "\r\r",		/* wake_str */
    500,		/* wake_rate */
    "$",		/* wake_prompt */
    "D\r",		/* dmode_str */
    ":",		/* dmode_prompt */
    "T%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF HAYES =		/* information for "Hayes" modem */
    {
    35,			/* dial_time */
    ",",		/* pause_chars */
    2,			/* pause_time */
    "AT\r",		/* wake_str */
/*** Note: Other wake_str's are possible here.  For Hayes 2400 that is to
/*** be used for both in and out calls, AT&F&D3 might be best.  For out calls
/*** only, maybe AT&F&D2.  See Hayes 2400 manual. ***/
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    "",			/* dmode_prompt */
    "ATD%s\r",		/* dial_str, note: user can supply D or T */
    0			/* dial_rate */
    };

static
MDMINF PENRIL =		/* information for "Penril" modem */
    {
    50,			/* dial_time */
    "",			/* pause_chars */	/** unknown -- HF **/
    0,			/* pause_time */
    "\r\r",		/* wake_str */
    300,		/* wake_rate */
    ">",		/* wake_prompt */
    "k\r",		/* dmode_str */
    ":",		/* dmode_prompt */
    "%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF RACAL =		/* information for "Racal Vadic" modem */
    {
    35,			/* dial_time */
    "Kk",		/* pause_chars */
    5,			/* pause_time */
    "\005\r",		/* wake_str */
    50,			/* wake_rate */
    "*",		/* wake_prompt */
    "D\r",		/* dmode_str */
    "?",		/* dmode_prompt */
    "%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF UNKNOWN =	/* information for "Unknown" modem */
    {
    30,			/* dial_time */
    "",			/* pause_chars */
    0,			/* pause_time */
    "",			/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF USROBOT =	/* information for "US Robotics 212A" modem */
    {
    30,			/* dial_time */
    ",",		/* pause_chars */
    2,			/* pause_time */
    "ATS2=01\r",	/* wake_str */
    0,			/* wake_rate */
    "OK\r",		/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "ATTD%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF VENTEL =		/* information for "Ventel" modem */
    {
    20,			/* dial_time */
    "%",		/* pause_chars */
    5,			/* pause_time */
    "\r\r\r",		/* wake_str */
    300,		/* wake_rate */
    "$",		/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "<K%s\r>",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF CONCORD =	/* Info for Condor CDS 220 2400b modem */
    {
    35,			/* dial_time */
    ",",		/* pause_chars */
    2,			/* pause_time */
    "\r\r",		/* wake_str */
    20,			/* wake_rate */
    "CDS >",		/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "<D M%s\r>",	/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF ATTUPC = /* dummy information for "ATT7300/Unix PC" internal modem */
    {
    20,			/* dial_time */
    "",			/* pause_chars */
    0,			/* pause_time */
    "",			/* wake_str */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "%s\r",		/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF ROLM =		/* IBM / Siemens / Rolm 8000, 9000, 9751 CBX */
    {
    60,			/* dial_time */
    "",			/* pause_chars */
    0,			/* pause_time */
    "\r\r",		/* wake_str */
    5,			/* wake_rate */
    "MODIFY?",	        /* wake_prompt */
    "",			/* dmode_str */
    "",			/* dmode_prompt */
    "CALL %s\r",	/* dial_str */
    0			/* dial_rate */
    };

static
MDMINF MICROCOM =	/* information for "Microcom" modems in native mode */
			/* (long answer only) */
    {
    35,			/* dial_time */
    ",!@",		/* pause_chars (! and @ aren't pure pauses) */
    3,			/* pause_time */
    "\r",		/* wake_str */
    100,		/* wake_rate */
    "!",		/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
    "d%s\r",		/* dial_str */
    0			/* dial_rate */
    };

/*
 * Declare table for converting modem numbers to information pointers.
 *
 * The entries MUST be in ascending order by modem number, without any
 * "gaps" in the numbers, and starting from one (1).
 *
 * This table should NOT include entries for the "variant" modem numbers,
 * since it is assumed that they share the same information as the normal
 * value.
 */
static
MDMINF *ptrtab[] =
    {
    &ATTDTDM,
    &ATTMODEM,
    &CERMETEK,
    &DF03,
    &DF100,
    &DF200,
    &GDC,
    &HAYES,
    &PENRIL,
    &RACAL,
    &UNKNOWN,
    &USROBOT,
    &VENTEL,
    &CONCORD,
    &ATTUPC,			/* ATT7300 internal modem, jrd*/
    &ROLM,			/* Rolm CBX, fdc */
    &MICROCOM,
    };

/*
 * Declare modem names and associated numbers for command parsing,
 * and also for doing number-to-name translation.
 *
 * The entries MUST be in alphabetical order by modem name.
 */
struct keytab mdmtab[] =
    {
    "attdtdm",		n_ATTDTDM,	0,
    "attmodem",		n_ATTMODEM,	0,
    "att7300",		n_ATTUPC,	0,
    "cermetek",		n_CERMETEK,	0,
    "concord",		n_CONCORD,	0,
    "df03-ac",		n_DF03,		0,
    "df100-series",	n_DF100,	0,
    "df200-series",	n_DF200,	0,
    "direct",		0,		0,
    "gendatacomm",	n_GDC,		0,
    "hayes",		n_HAYES,	0,
    "microcom",		n_MICROCOM,	0,
    "penril",		n_PENRIL,	0,
    "racalvadic",	n_RACAL,	0,
    "rolm",		n_ROLM,		0,
    "unknown",		n_UNKNOWN,	0,
    "usrobotics-212a",	n_USROBOT,	0,
    "ventel",		n_VENTEL,	0
    };

int nmdm = (sizeof(mdmtab) / sizeof(struct keytab));	/* number of modems */

#define DIALING 4		/* for ttpkt parameter */
#define CONNECT 5

#define CONNECTED 1		/* for completion status */
#define FAILED	  2

/*
 * Failure reasons for use with the 'longjmp' exit.
 */
#define F_time		1	/* timeout */
#define F_int		2	/* interrupt */
#define F_modem		3	/* modem-detected failure */
#define F_minit		4	/* cannot initialize modem */

static
char *F_reason[5] = {		/* failure reasons for message */
    "Unknown",	"Timeout", "Interrupt", "Modem", "Initialize" };

static int tries = 0;

#define LBUFL 100
static char lbuf[LBUFL];

static jmp_buf sjbuf;

static SIGTYP (*savAlrm)();	/* for saving alarm handler */
static SIGTYP (*savInt)();	/* for saving interrupt handler */

char *					/* Copy a string of the */
xcpy(to,from,len)			/* the given length. */
  register char *to, *from;
  register unsigned len; {
    while (len--) *to++ = *from++;
}

SIGTYP
dialtime() {			/* timer interrupt handler */
#ifdef OS2
    alarmack();
#endif
    longjmp( sjbuf, F_time );
}

SIGTYP
dialint() {				/* user-interrupt handler */
#ifdef OS2
    alarmack();
#endif
    longjmp( sjbuf, F_int );
    }

static
ttolSlow(s,millisec) char *s; int millisec; {  /* output s-l-o-w-l-y */
    for (; *s; s++) {
	ttoc(*s);
	msleep(millisec);
	}
    }

/*
 * Wait for a string of characters.
 *
 * The characters are waited for individually, and other characters may
 * be received "in between".  This merely guarantees that the characters
 * ARE received, and in the order specified.
 */
static
waitFor(s) char *s; {
    CHAR c;
    while ( c = *s++ )			/* while more characters remain... */
	while ( ( ttinc(0) & 0177 ) != c ) /* wait for the character */
	    ;
}

static
didWeGet(s,r) char *s, *r; {	/* Looks in string s for response r */
    int lr = strlen(r);		/*  0 means not found, 1 means found it */
    int i;
    debug(F110,"didWeGet",r,0);
    debug(F110," in",s,0);
    for (i = strlen(s)-lr; i >= 0; i--)
	if ( s[i] == r[0] ) if ( !strncmp(s+i,r,lr) ) return( 1 );
    return( 0 );
}


/* R E S E T -- Reset alarms, etc. on exit. */

static
reset ()
    {
    alarm(0);
    signal(SIGALRM,savAlrm);		/* restore alarm handler */
#ifndef OS2
    signal(SIGINT,savInt);		/* restore interrupt handler */
#endif
    }

/*  C K D I A L	 --  Dial up the remote system */

ckdial(telnbr) char *telnbr; {

    char c;
    char *i, *j;
    int waitct, status;
    char errmsg[50], *erp;
    MDMINF *pmdminf;	/* pointer to modem-specific info */
    int augmdmtyp;	/* "augmented" modem type, to handle modem modes */
    int mdmEcho = 0;	/* assume modem does not echo */
    int n, n1;
    char *pc;		/* pointer to a character */

	if (!mdmtyp) {
	    printf("Sorry, you must 'set modem' first\n");
	    return(-2);
	}
	if (!local) {
	    printf("Sorry, you must 'set line' first\n");
	    return(-2);
	}
	if (speed < 0) {
	    printf("Sorry, you must 'set speed' first\n");
	    return(-2);
	}
        debug(F110,"dial",telnbr,0);

	/* Carrier no-wait can be invalidated by ckutio fun and games, jrd */
	if (ttopen(ttname,&local,mdmtyp) < 0) { /* Open, no carrier wait */
	    erp = errmsg;
	    sprintf(erp,"Sorry, can't open %s",ttname);
	    perror(errmsg);
	    return(-2);
	}
	pmdminf = ptrtab[mdmtyp-1];	/* set pointer to modem info */
	augmdmtyp = mdmtyp;		/* initialize "augmented" modem type */

/* cont'd... */

				       /* interdigit waits for tone dial */
/* ...dial, cont'd */


	waitct = 1*strlen(telnbr) ;	/* compute time to dial worst case */
	waitct += pmdminf->dial_time;	/* dialtone + completion wait times */
	for (i=telnbr; *i; i++)		/* add in pause characters time */
	    for (j=pmdminf->pause_chars; *j; j++)
		if (*i == *j) {
		    waitct += pmdminf->pause_time;
		    break;
		    }

       printf("Dialing thru %s, speed %d, number %s.\n",ttname,speed,telnbr);
       printf("The timeout for completing the call is %d seconds.\n",waitct);
#ifndef OS2
       printf("Type the interrupt character (^C) to cancel the dialing.\n");
#endif
       debug(F101,ttname,"",speed);
       debug(F101,"timeout","",waitct);

/* Hang up the modem (in case it wasn't "on hook") */

	if ( tthang() < 0 ) {
	    printf("Sorry, Can't hang up tty line\n");
	    return(-2);
	}
        if (augmdmtyp == n_ROLM) sleep(1);

/* Condition console terminal and communication line */
				/* place line into "clocal" dialing state */
	if ( ttpkt(speed,DIALING,parity) < 0 )	{
	    printf("Sorry, Can't condition communication line\n");
	    return(-2);
	}
        if (augmdmtyp == n_ROLM) sleep(1);
/*
 * Establish jump vector, or handle "failure" jumps.
 */

    if ( n = setjmp(sjbuf) )		/* if a "failure jump" was taken... */
	{
	alarm ( 0 );			/* disable timeouts */
	if ( n1 = setjmp(sjbuf) )	/* failure while handling failure */
	    {
	    printf ( "%s failure while handling failure.\n", F_reason[n1] );
	    }
	else				/* first (i.e., non-nested) failure */
	    {
	    signal ( SIGALRM, dialtime );	/* be sure to catch signals */
#ifndef OS2
	    if ( signal ( SIGINT, SIG_IGN ) != SIG_IGN )
		signal ( SIGINT, dialint );
#endif
	    alarm ( 10 );		/* be sure to exit this section */
	    ttclos ();			/* hangup and close the line */
	    }
	switch ( n )			/* type of failure */
	    {
	    case F_time:		/* timed out */
		{
		printf ( "No connection made within the allotted time.\n" );
		debug(F110,"dial","timeout",0);
		break;
		}
	    case F_int:			/* dialing interrupted */
		{
		printf ( "Dialing interrupted.\n" );
		debug(F110,"dial","interrupted",0);
		break;
		}
	    case F_modem:		/* modem detected a failure */
		{
		printf ( "Failed (\"" );
		for ( pc=lbuf; *pc; pc++ )
		    if ( isprint(*pc) )
			putchar(*pc);	/* display printable reason */
		printf ( "\").\n" );
		debug(F110,"dial",lbuf,0);
		break;
		}
	    case F_minit:		/* cannot initialize modem */
		{
		printf ( "Cannot initialize modem.\n" );
		debug(F110,"dial","modem init",0);
		break;
		}
	    }
	reset();			/* reset alarms, etc. */
	return ( -2 );			/* exit with failure code */
	}

/*
 * Set timer and interrupt handlers.
 */

/* The following call to ttflui() commented out because ttpkt just did this */
/*  ttflui();	*/		/* flush input buffer if any */

    savAlrm = signal(SIGALRM,dialtime); /* set alarm handler */
#ifndef OS2
    if ( ( savInt = signal ( SIGINT, SIG_IGN ) ) != SIG_IGN )
	signal ( SIGINT, dialint );	/* set int handler if not ignored */
#endif
    debug(F100,"ckdial giving modem 10 secs to wake up","",0);
    alarm(10);			/* give modem 10 seconds to wake up */

/*
 * Put modem in command mode.
 */

#define OKAY 1			/* modem attention attempt status */
#define IGNORE 2
#define GOT_O -2
#define GOT_A -3

switch (augmdmtyp) {

#ifdef ATT7300
	case n_ATTUPC:
/*
  For ATT7300/Unix PC's with their special internal modem. Timeout
  and user interrupts are enabled during dialing.  attdial() is in
  file ckutio.c.  - jrd
*/
	{
	alarm(waitct);				/* do alarm properly */
	if (attdial(ttname,speed,telnbr))	/* dial internal modem */
		{
		reset();			/* reset alarms, etc. */
		if ( ! quiet ) printf("Call did not complete.");
		ttclos();			/* close line if open */
		return (-2);			/* return failure */
		}
	reset();				/* reset alarms, etc. */
	ttpkt(speed,CONNECT,parity);		/* cancel dialing ioctl */
	if ( ! quiet ) printf ("Call completed.\07\r\n");
	return(0);	 /* no conversation with modem to complete dialing */
	}
#endif /* att7300 */

    case n_HAYES:
    case n_HAYESNV:
	while(tries++ < 4) {
	    ttol( HAYES.wake_str, strlen(HAYES.wake_str) );	/* wakeup */
	    status = 0;
	    while ( status <= 0 ) {
		switch (ttinc(0) & 0177) {
		    case 'A':			/* echoing, ignore */
			status = GOT_A;
			break;
		    case 'T':
			if (status == GOT_A) {
			    mdmEcho = 1;	/* expect echoing later */
			    status = 0;
			    break;
			}
			status = IGNORE;
			break;
		    case LF:
		    case CR:
			status = 0;
			break;
		    case '0':			/* numeric result code */
			augmdmtyp = n_HAYESNV;	/* nonverbal result codes */
			status = OKAY;
			break;
		    case 'O':			/* maybe English result code*/
			status = GOT_O;
			break;
		    case 'K':
			if (status == GOT_O) {
			    augmdmtyp = n_HAYES;
			    status = OKAY;
			    break;
			}			/* else its default anyway */
		    default:
			status = IGNORE;
			break;
		    }
		}
	    if (status == OKAY) break;
	    if (status == IGNORE) ttflui();
	    sleep(1);		/* wait before retrying */
	}
	if (status != 0) break;
	longjmp( sjbuf, F_minit );	/* modem-initialization failure */

/* cont'd... */

					/* interdigit waits for tone dial */
    case n_MICROCOM:
        {
	    jmp_buf savejmp;
	    alarm(0);
	    xcpy((char *)savejmp, (char *)sjbuf, sizeof savejmp);
	    if(setjmp(sjbuf)) {
	    	/* try the autobaud sequence */
		xcpy((char *)sjbuf, (char *)savejmp, sizeof savejmp);
		alarm(5);
	        ttolSlow("44445", MICROCOM.wake_rate);
		waitFor(MICROCOM.wake_str);
	    } else {
		alarm(2);
		ttolSlow(MICROCOM.wake_str, MICROCOM.wake_rate);
	    	waitFor(MICROCOM.wake_str);
		alarm(0);
		xcpy((char *)sjbuf, (char *)savejmp, sizeof savejmp);
	    }
	}
	break;
    case n_ATTDTDM:		/* DTDM requires BREAK to wake up */
	ttsndb();		/* Send BREAK */
	break;			/* ttsndb() defined in ckutio.c */

    default:			/* place modem into command mode */
	debug(F111,"ckdial default, wake string",pmdminf->wake_str,
	      pmdminf->wake_rate);
	ttolSlow(pmdminf->wake_str, pmdminf->wake_rate);
	debug(F110,"ckdial default, waiting for wake_prompt",
	      pmdminf->wake_prompt,0);
	waitFor(pmdminf->wake_prompt);
	break;
    }
    debug(F100,"ckdial got wake prompt","",0);
    alarm(0);			/* turn off alarm */
    msleep(500);		/* give things settling time */
    alarm(10);			/* alarm on dialing prompts */


/* Dial the number */
				/* put modem into dialing mode */
    ttolSlow(pmdminf->dmode_str, pmdminf->dial_rate);
    if (pmdminf->dmode_prompt) {	/* wait for prompt, if any expected */
	waitFor(pmdminf->dmode_prompt);
	msleep(300);
    }

    alarm(0);			/* turn off alarm on dialing prompts */
    alarm(waitct);		/* time to allow for connecting */
    ttflui();			/* clear out stuff from waking modem up */

    if (augmdmtyp != n_ATTDTDM)
	sprintf(lbuf,pmdminf->dial_str, telnbr); /* form dialing string */
    else
	sprintf(lbuf,"%s\r",telnbr);

    sprintf(lbuf, pmdminf->dial_str, telnbr); /* form dialing string */
    debug(F110,"dialing",lbuf,0);
    ttolSlow(lbuf,pmdminf->dial_rate);	/* send dialing string */

    if (augmdmtyp == n_RACAL) { /* acknowledge printout of dialing string */
	sleep(3);
	ttflui();
	ttoc('\r');
    }

/* cont'd... */

					/* interdigit waits for tone dial */
/* ...dial, cont'd */


/* Check for connection */

/*
 * I believe we also need to look for carrier in order to determine if a
 * connection has been made.  In fact, for many we may only want to look for
 * the "failure" responses in order to short-circuit the timeout, and let
 * carrier be the determination of whether a connection has been made. -- DS
 */

    status = 0;
    strcpy(lbuf,"No Connection");	/* default failure reason */
    while (status == 0) {
      switch (augmdmtyp) {
	default:
	    for (n=0; n < LBUFL-1; n++) { /* accumulate response */
		lbuf[n] = (ttinc(0) & 0177);
		if ( lbuf[n] == CR || lbuf[n] == LF ) break;
		}
	    lbuf[n] = '\0';		/* terminate response from modem */
	    debug(F110,"dial modem response",lbuf,0);
	    if (n) {			/* if one or more characters present */
		switch (augmdmtyp) {
		  case n_ATTMODEM:
		      if (didWeGet(lbuf,"Answered")) status = CONNECTED;
		      if (didWeGet(lbuf,"Connected")) status = CONNECTED;
		      if (didWeGet(lbuf,"Not connected")) status = FAILED;
		      if (didWeGet(lbuf,"Not Connected")) status = FAILED;
		      if (didWeGet(lbuf,"Busy")) status = FAILED;
		      if (didWeGet(lbuf,"No dial tone")) status = FAILED;
		      if (didWeGet(lbuf,"No Dial Tone")) status = FAILED;
		      if (didWeGet(lbuf,"No answer")) status = FAILED;
		      if (didWeGet(lbuf,"No Answer")) status = FAILED;
		      break;
		  case n_ATTDTDM:
		      if (didWeGet(lbuf,"DENIED")) status = FAILED;
		      if (didWeGet(lbuf,"CHECK OPTIONS")) status = FAILED;
		      if (didWeGet(lbuf,"DISCONNECTED")) status = FAILED;
		      if (didWeGet(lbuf,"ANSWERED")) status = CONNECTED;
		      if (didWeGet(lbuf,"BUSY")) status = FAILED;
		      break;
		  case n_CERMETEK:
		    if (didWeGet(lbuf,"\016A")) {
			status = CONNECTED;
			ttolSlow("\016U 1\r",200);	/* make transparent*/
			}
		    break;
		  case n_DF100:	     /* DF100 won't generate some of these */
		  case n_DF200:
		    if (didWeGet(lbuf,"Attached")) status = CONNECTED;
		    /*
		     * The DF100 will respond with "Attached" even if DTR
		     * and/or carrier are not present.	Another reason to
		     * (also) wait for carrier?
		     */
		    if (didWeGet(lbuf,"Busy")) status = FAILED;
		    if (didWeGet(lbuf,"Disconnected")) status = FAILED;
		    if (didWeGet(lbuf,"Error")) status = FAILED;
		    if (didWeGet(lbuf,"No answer")) status = FAILED;
		    if (didWeGet(lbuf,"No dial tone")) status = FAILED;
		    if (didWeGet(lbuf,"Speed:")) status = FAILED;
		    /*
		     * It appears that the "Speed:..." response comes after an
		     * "Attached" response, so this is never seen.  HOWEVER,
		     * it would be very handy to detect this and temporarily
		     * reset the speed, since it's a nuisance otherwise.
		     * If we wait for some more input from the modem, how do
		     * we know if it's from the remote host or the modem?
		     * Carrier reportedly doesn't get set until after the
		     * "Speed:..." response (if any) is sent.  Another reason
		     * to (also) wait for carrier.
		     */
		    break;
		  case n_GDC:
		    if (didWeGet(lbuf,"ON LINE")) status = CONNECTED;
		    if (didWeGet(lbuf,"NO CONNECT")) status = FAILED;
		    break;
		  case n_HAYES:
		  case n_USROBOT:
		    if (didWeGet(lbuf,"CONNECT 1200")) {
			if (speed != 1200) {
			    if (ttpkt(1200,DIALING) < 0) {
				printf("Can't change speed to 1200\r\n");
			    } else {
				speed = 1200;
				status = CONNECTED;
				if ( !quiet )
				  printf("Speed changed to 1200\r\n");
			    }
			} /* Expand this to include more speeds */
		    }
		    if (didWeGet(lbuf,"CONNECT")) status = CONNECTED;
		    if (didWeGet(lbuf,"NO CARRIER")) status = FAILED;
  		    if (didWeGet(lbuf,"NO DIALTONE")) status = FAILED;
  		    if (didWeGet(lbuf,"BUSY")) status = FAILED;
  		    if (didWeGet(lbuf,"NO ANSWER")) status = FAILED;
  		    if (didWeGet(lbuf,"RING")) status = FAILED;
  		    if (didWeGet(lbuf,"ERROR")) status = FAILED;
		    break;
		  case n_PENRIL:
		    if (didWeGet(lbuf,"OK")) status = CONNECTED;
		    if (didWeGet(lbuf,"BUSY")) status = FAILED;
		    if (didWeGet(lbuf,"NO RING")) status = FAILED;
		    break;
		  case n_RACAL:
		    if (didWeGet(lbuf,"ON LINE")) status = CONNECTED;
		    if (didWeGet(lbuf,"FAILED CALL")) status = FAILED;
		    break;
		  case n_ROLM:
		    if (didWeGet(lbuf,"CALLING")) status = 0;
		    if (didWeGet(lbuf,"COMPLETE")) status = CONNECTED;
		    if (didWeGet(lbuf,"FAILED")) status = FAILED;
		    if (didWeGet(lbuf,"NOT AVAILABLE")) status = FAILED;
		    if (didWeGet(lbuf,"LACKS PERMISSION")) status = FAILED;
		    if (didWeGet(lbuf,"NOT A DATALINE")) status = FAILED;
		    /*
                      Early versions of the Rolm 9751
		      CBX software do not give a CALL COMPLETE indication
		      when dialing an outpool number, but it does seem to
		      return a long string of DELs at that point.
		      (this doesn't really work...)
		    if (didWeGet(lbuf,"\177\177\177")) status = CONNECTED;
		    */
		    break;
		  case n_VENTEL:
		    if (didWeGet(lbuf,"ONLINE!")) status = CONNECTED;
		    if (didWeGet(lbuf,"BUSY")) status = FAILED;
		    if (didWeGet(lbuf,"DEAD PHONE")) status = FAILED;
		    break;
		  case n_CONCORD:
		    if (didWeGet(lbuf,"INITIATING")) status = CONNECTED;
		    if (didWeGet(lbuf,"BUSY")) status = FAILED;
		    if (didWeGet(lbuf,"CALL FAILED")) status = FAILED;
		    break;
		  case n_MICROCOM:
		    /* "RINGBACK" means phone line ringing, continue */
		    if (didWeGet(lbuf,"NO CONNECT")) status = FAILED;
                    /* trailing speed ignored */
		    if (didWeGet(lbuf,"CONNECT")) status = CONNECTED;
		    if (didWeGet(lbuf,"BUSY")) status = FAILED;
		    if (didWeGet(lbuf,"NO DIALTONE")) status = FAILED;
		    if (didWeGet(lbuf,"COMMAND ERROR")) status = FAILED;
		    if (didWeGet(lbuf,"IN USE")) status = FAILED;
		    break;
		}
	    }
	    break;

	case n_DF03:			/* because response lacks CR or NL */
	    c = ttinc(0) & 0177;
	    if ( c == 'A' ) status = CONNECTED;
	    if ( c == 'B' ) status = FAILED;
	    break;

	case n_HAYESNV:
	    c = ttinc(0) & 0177;
	    if (mdmEcho) {		/* sponge up dialing string */
		mdmEcho = c!='\r';	/* until return is echoed */
		break;
		}
	    if (c == '1') status = CONNECTED;
	    if (c == '3') status = FAILED;
	    if (c == '5') status = CONNECTED;
	    break;

	case n_UNKNOWN:
	    /** SHOULD WAIT FOR CARRIER OR TIMEOUT -- DS **/
	    break;
	}				/* switch (augmdmtyp) */
    }					/* while status == 0 */
    alarm(0);				/* turn off alarm on connecting */
    if ( status != CONNECTED )		/* modem-detected failure */
	longjmp( sjbuf, F_modem );	/* exit (with reason in lbuf) */
    msleep(500);			/* allow some time...  */
    alarm(3);				/* precaution in case of trouble */
    debug(F110,"dial","succeeded",0);
    if (augmdmtyp != n_ROLM)		/* Rolm has wierd modem signaling */
      ttpkt(speed,CONNECT,parity);	/* cancel dialing state ioctl */
    reset ();				/* reset alarms, etc. */
    if ( ! quiet )
	printf ( "Call completed.\07\n" );
    return ( 0 );			/* return, and presumably connect */
}
