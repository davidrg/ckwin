#include "ckcsym.h"
#ifdef NOLOCAL
char *connv = "";
#else
char *connv = "CONNECT Command for UNIX, 5A(073) 15 Sep 94";

/*  C K U C O N  --  Terminal connection to remote system, for UNIX  */
/*
  Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1994, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.

  Special thanks to Eduard Vopicka in Prague, Czech Republic, for valuable
  contributions to this module in July 1994.
*/
#include "ckcdeb.h"			/* Common things first */

#ifdef NEXT
#undef NSIG
#include <sys/wait.h>			/* For wait() */
#endif /* NEXT */

#include <signal.h>			/* Signals */
#include <errno.h>			/* Error numbers */

#ifdef ZILOG				/* Longjumps */
#include <setret.h>
#else
#include <setjmp.h>
#endif /* ZILOG */

/* Kermit-specific includes */

#include "ckcasc.h"			/* ASCII characters */
#include "ckcker.h"			/* Kermit things */
#include "ckucmd.h"			/* For xxesc() prototype */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOCSETS
#include "ckcxla.h"			/* Character set translation */
#endif /* NOCSETS */

/* Internal function prototypes */

_PROTOTYP( VOID ttflux, (void) );
_PROTOTYP( VOID doesc, (char) );
_PROTOTYP( static VOID logchar, (char) );
_PROTOTYP( int hconne, (void) );
_PROTOTYP( VOID shomdm, (void) );
_PROTOTYP( static int kbget, (void) );

/*
  For inter-fork signaling.  Normally we use SIGUSR1, except on SCO, where 
  we use SIGUSR2 because SIGUSR1 is used by the system.  You can define
  CK_FORK_SIG to be whatever other signal you might want to use at compile
  time.  We don't use SIGUSR2 everywhere because in some systems, like
  UnixWare, the default action for SIGUSR2 is to killthe process that gets it.
*/
#ifndef CK_FORK_SIG

#ifndef SIGUSR1				/* User-defined signals */
#define SIGUSR1 30
#endif /* SIGUSR1 */

#ifndef SIGUSR2
#define SIGUSR2 31
#endif /* SIGUSR2 */

#ifdef M_UNIX
#define CK_FORK_SIG SIGUSR2		/* SCO - use SIGUSR2 */
#else
#define CK_FORK_SIG SIGUSR1		/* Others - use SIGUSR1 */
#endif /* M_UNIX */

#endif /* CK_FORK_SIG */

/* External variables */

extern int local, escape, duplex, parity, flow, seslog, sessft, debses,
 mdmtyp, ttnproto, cmask, cmdmsk, network, nettype, deblog, sosi, tnlm,
 xitsta, what, ttyfd, quiet, backgrd, pflag, tt_crd, tn_nlm, ttfdflg;
extern long speed;
extern char ttname[], sesfil[], myhost[], *ccntab[];

#ifdef CK_APC
extern int apcactive;			/* Application Program Command (APC) */
extern int apcstatus;			/* items ... */
static int apclength = 0;          
#ifdef DCMDBUF
extern char *apcbuf;
#else
extern char apcbuf[];
#endif /* DCMDBUF */
static int apcbuflen = APCBUFLEN - 2;
#endif /* CK_APC */

#ifndef NOSETKEY			/* Keyboard mapping */
extern KEY *keymap;			/* Single-character key map */
extern MACRO *macrotab;			/* Key macro pointer table */
static MACRO kmptr = NULL;		/* Pointer to current key macro */
#endif /* NOSETKEY */

/* Global variables local to this module */

static int quitnow = 0,			/* <esc-char>Q was typed */
  dohangup = 0,				/* <esc-char>H was typed */
  sjval,				/* Setjump return value */
  goterr = 0,				/* Fork/pipe creation error flag */
#ifndef SUNX25
  active = 0,				/* Lower fork active flag */
#endif /* SUNX25 */
  inshift = 0,				/* SO/SI shift states */
  outshift = 0;

static PID_T parent_id = (PID_T)0;	/* Process id of keyboard fork */

static char ecbuf[10], *ecbp;		/* Escape char buffer & pointer */

#ifdef pdp11
#define IBUFL 1536			/* Input buffer length */
#else
#define IBUFL 4096
#endif /* pdp11 */

static int obc = 0;			/* Output buffer count */

#ifndef OXOS
#define OBUFL 1024			/* Output buffer length */
#else
#define OBUFL IBUFL
#endif /* OXOS */

#define TMPLEN 200			/* Temporary message buffer length */
#ifdef DYNAMIC
static char *ibuf = NULL, *obuf = NULL, *temp = NULL; /* Buffers */
#else
static char ibuf[IBUFL], obuf[OBUFL], temp[TMPLEN];
#endif /* DYNAMIC */

#ifdef DYNAMIC
static char *ibp;			/* Input buffer pointer */
#else
static char *ibp = ibuf;		/* Input buffer pointer */
#endif /*DYNAMIC */
static int ibc = 0;			/* Input buffer count */

#ifdef DYNAMIC
static char *obp;			/* Output buffer pointer */
#else
static char *obp = obuf;		/* Output buffer pointer */
#endif /* DYNAMIC */

/* SunLink X.25 items */

#ifdef SUNX25
static char *p;				/* General purpose pointer */
char x25ibuf[MAXIX25];			/* Input buffer */
char x25obuf[MAXOX25];			/* Output buffer */
int active = 0;				/* Lower fork active flag */
int ibufl;				/* Length of input buffer */
int obufl;				/* Length of output buffer */
unsigned char tosend = 0;
int linkid, lcn;
static int dox25clr = 0;
CHAR padparms[MAXPADPARMS+1];
#endif /* SUNX25 */

static int xpipe[2] = {-1, -1};	/* Pipe descriptor for child-parent messages */
static PID_T pid;		/* Process ID of child */

/* Character-set items */

#ifndef NOCSETS
#ifdef CK_ANSIC /* ANSI C prototypes... */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
static CHAR (*sxo)(CHAR);	/* Local translation functions */
static CHAR (*rxo)(CHAR);	/* for output (sending) terminal chars */
static CHAR (*sxi)(CHAR);	/* and for input (receiving) terminal chars. */
static CHAR (*rxi)(CHAR);
#else /* Not ANSI C... */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();	/* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();	/* translation functions. */
static CHAR (*sxo)();		/* Local translation functions */
static CHAR (*rxo)();		/* for output (sending) terminal chars */
static CHAR (*sxi)();		/* and for input (receiving) terminal chars. */
static CHAR (*rxi)();
#endif /* CK_ANSIC */
extern int language;		/* Current language. */
static int langsv;		/* For remembering language setting. */
extern struct csinfo fcsinfo[]; /* File character set info. */
extern int tcsr, tcsl;		/* Terminal character sets, remote & local. */
static int tcs;			/* Intermediate ("transfer") character set. */
#endif /* NOCSETS */

/*
  We do not need to parse and recognize escape sequences if we are being built
  without character-set support AND without APC support.
*/
#ifdef NOCSETS				/* No character sets */
#ifndef CK_APC				/* No APC */
#ifndef NOESCSEQ
#define NOESCSEQ			/* So no escape sequence recognizer */
#endif /* NOESCSEQ */
#endif /* CK_APC */
#endif /* NOCSETS */
/*
  Child process events / messages
*/
#define CEV_NO  0			/* No event */
#define CEV_HUP 1			/* Communications hangup */
#define CEV_PAD 2			/* X.25 - change PAD parameters */
#define CEV_DUP 3			/* Toggle duplex */
#define CEV_APC 4			/* Execute APC */

#ifdef NOESCSEQ
#define chkaes(x) 0
#else
/*
  As of edit 178, the CONNECT command will skip past ANSI escape sequences
  to avoid translating the characters within them.  This allows the CONNECT
  command to work correctly when connected to a remote host that uses a
  7-bit ISO 646 national character set, in which characters like '[' would
  normally be translated into accented characters, ruining the terminal's
  interpretation (and generation) of escape sequences.

  As of edit 190, the CONNECT command responds to APC escape sequences
  (ESC _ text ESC \) if the user SETs TERMINAL APC ON or UNCHECKED, and the
  program was built with CK_APC defined.

  Non-ANSI/ISO-compliant escape sequences are not handled.
*/

/*
  States for the escape-sequence recognizer.
*/
#define ES_NORMAL 0			/* Normal, not in an escape sequence */
#define ES_GOTESC 1			/* Current character is ESC */
#define ES_ESCSEQ 2			/* Inside an escape sequence */
#define ES_GOTCSI 3			/* Inside a control sequence */
#define ES_STRING 4			/* Inside DCS,OSC,PM, or APC string */
#define ES_TERMIN 5			/* 1st char of string terminator */

static int
  escseq = 0,				/* 1 = Recognizer is active */
  inesc = ES_NORMAL,			/* State of sequence recognizer */
  oldesc = -1;				/* Previous state of recognizer */
/*
  ANSI escape sequence handling.  Only the 7-bit form is treated, because
  translation is not a problem in the 8-bit environment, in which all GL
  characters are ASCII and no translation takes place.  So we don't check
  for the 8-bit single-character versions of CSI, DCS, OSC, APC, or ST.
  Here is the ANSI sequence recognizer state table, followed by the code
  that implements it.

  Definitions:
    CAN = Cancel                       01/08         Ctrl-X
    SUB = Substitute                   01/10         Ctrl-Z
    DCS = Device Control Sequence      01/11 05/00   ESC P
    CSI = Control Sequence Introducer  01/11 05/11   ESC [
    ST  = String Terminator            01/11 05/12   ESC \
    OSC = Operating System Command     01/11 05/13   ESC ]
    PM  = Privacy Message              01/11 05/14   ESC ^
    APC = Application Program Command  01/11 05/15   ESC _

  ANSI escape sequence recognizer:

    State    Input  New State  ; Commentary

    NORMAL   (start)           ; Start in NORMAL state

    (any)    CAN    NORMAL     ; ^X cancels
    (any)    SUB    NORMAL     ; ^Z cancels

    NORMAL   ESC    GOTESC     ; Begin escape sequence
    NORMAL   other             ; NORMAL control or graphic character

    GOTESC   ESC               ; Start again
    GOTESC   [      GOTCSI     ; CSI
    GOTESC   P      STRING     ; DCS introducer, consume through ST
    GOTESC   ]      STRING     ; OSC introducer, consume through ST
    GOTESC   ^      STRING     ; PM  introducer, consume through ST
    GOTESC   _      STRING     ; APC introducer, consume through ST
    GOTESC   0..~   NORMAL     ; 03/00 through 17/14 = Final character
    GOTESC   other  ESCSEQ     ; Intermediate or ignored control character

    ESCSEQ   ESC    GOTESC     ; Start again
    ESCSEQ   0..~   NORMAL     ; 03/00 through 17/14 = Final character
    ESCSEQ   other             ; Intermediate or ignored control character

    GOTCSI   ESC    GOTESC     ; Start again
    GOTCSI   @..~   NORMAL     ; 04/00 through 17/14 = Final character
    GOTCSI   other             ; Intermediate char or ignored control char

    STRING   ESC    TERMIN     ; Maybe have ST
    STRING   other             ; Consume all else

    TERMIN   \      NORMAL     ; End of string
    TERMIN   other  STRING     ; Still in string
*/
/*
  chkaes() -- Check ANSI Escape Sequence.
  Call with EACH character in input stream.
  Sets global inesc variable according to escape sequence state.
  Returns 0 normally, 1 if an APC sequence is to be executed.
*/
int
#ifdef CK_ANSIC
chkaes(char c)
#else
chkaes(c) char c;
#endif /* CK_ANSIC */
/* chkaes */ {

    oldesc = inesc;			/* Remember previous state */
    if (c == CAN || c == SUB)		/* CAN and SUB cancel any sequence */
      inesc = ES_NORMAL;
    else				/* Otherwise */
      switch (inesc) {			/* enter state switcher */

	case ES_NORMAL:			/* NORMAL state */
	  if (c == ESC)			/* Got an ESC */
	    inesc = ES_GOTESC;		/* Change state to GOTESC */
	  break;			/* Otherwise stay in NORMAL state */

	case ES_GOTESC:			/* GOTESC state */
	  if (c == '[')			/* Left bracket after ESC is CSI */
	    inesc = ES_GOTCSI;		/* Change to GOTCSI state */
	  else if (c == 'P' || (c > 0134 && c < 0140)) { /* P, [, ^, or _ */
	      inesc = ES_STRING;	/* Switch to STRING-absorption state */
#ifdef CK_APC
	      if (c == '_' && pid == 0 && /* APC handled in child only */
		  apcstatus != APC_OFF) { 
		  debug(F100,"APC begin","",0);
		  apcactive = 1;	/* Set APC-Active flag */
		  apclength = 0;	/* and reset APC buffer pointer */
	      }
#endif /* CK_APC */
	  } else if (c > 057 && c < 0177) /* Final character '0' thru '~' */
	    inesc = ES_NORMAL;		/* Back to normal */
	  else if (c != ESC)		/* ESC in an escape sequence... */
	    inesc = ES_ESCSEQ;		/* starts a new escape sequence */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_ESCSEQ:			/* ESCSEQ -- in an escape sequence */
	  if (c > 057 && c < 0177)	/* Final character '0' thru '~' */
	    inesc = ES_NORMAL;		/* Return to NORMAL state. */
	  else if (c == ESC)		/* ESC ... */
	    inesc = ES_GOTESC;		/* starts a new escape sequence */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_GOTCSI:			/* GOTCSI -- In a control sequence */
	  if (c > 077 && c < 0177)	/* Final character '@' thru '~' */
	    inesc = ES_NORMAL;		/* Return to NORMAL. */
	  else if (c == ESC)		/* ESC ... */
	    inesc = ES_GOTESC;		/* starts over. */
	  break;			/* Intermediate or ignored ctrl char */

	case ES_STRING:			/* Inside a string */
	  if (c == ESC)			/* ESC may be 1st char of terminator */
	    inesc = ES_TERMIN;		/* Go see. */
#ifdef CK_APC
	  else if (apcactive && (apclength < apcbuflen)) /* If in APC, */
	    apcbuf[apclength++] = c;	/* deposit this character. */
#endif /* CK_APC */
	  break;			/* Absorb all other characters. */

	case ES_TERMIN:			/* May have a string terminator */
	  if (c == '\\') {		/* which must be backslash */
	      inesc = ES_NORMAL;	/* If so, back to NORMAL */
#ifdef CK_APC
	      if (apcactive) {		/* If it was an APC string, */
		  debug(F101,"APC terminated","",c);
		  apcbuf[apclength] = NUL; /* terminate it and then ... */
		  return(1);
	      }
#endif /* CK_APC */
	  } else {			/* Otherwise */
	      inesc = ES_STRING;	/* Back to string absorption. */
#ifdef CK_APC
	      if (apcactive && (apclength+1 < apcbuflen)) { /* In APC string */
		  apcbuf[apclength++] = ESC; /* deposit the Esc character */
		  apcbuf[apclength++] = c;   /* and this character too */
	      }
#endif /* CK_APC */
	  }
      }
    return(0);
}
#endif /* NOESCSEQ */

/* Connect state parent/child communication signal handlers */

/* Routines used by the child process */

int
pipemsg(n) int n; {			/* Send message ID to parent */
    int code = n & 255;
    return(write(xpipe[1], &code, sizeof(code)));
}

/* Environment pointer for CK_FORK_SIG signal handling in child... */

#ifdef CK_POSIX_SIG
static sigjmp_buf sig_env;
#else
static jmp_buf sig_env;
#endif /* CK_POSIX_SIG */

static SIGTYP				/* CK_FORK_SIG handling in child ... */
forkint(foo) int foo; {
    /* It is important to disable CK_FORK_SIG before longjmp */
    signal(CK_FORK_SIG, SIG_IGN);	/* Set to ignore CK_FORK_SIG */
    debug(F100, "forkint - CK_FORK_SIG", "", 0);
    /* Force return from ck_sndmsg() */
#ifdef CK_POSIX_SIG
    siglongjmp(sig_env, 1);
#else
    longjmp(sig_env, 1);
#endif /* CK_POSIX_SIG */
    /* NOTREACHED */
}

static void
ck_sndmsg() {				/* Executed by child only ... */
    debug(F100, "ck_sndmsg - notifying parent", "", 0);
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sig_env,1)
#else
	setjmp(sig_env)
#endif /* CK_POSIX_SIG */
	== 0) {
        signal(CK_FORK_SIG, forkint);	/* Set up signal handler */
        kill(parent_id, CK_FORK_SIG);	/* Kick the parent */
        for (;;) pause();		/* Wait for CK_FORK_SIG or SIGKILL */
	/* NOTREACHED */
    }
    /* We come here from forkint() via [sig]longjmp(sig_env,1) */
    debug(F100, "ck_sndmsg - returning", "", 0);
}

/* Routines used by the parent process */

#ifdef CK_POSIX_SIG		 /* Environment pointer for CONNECT errors */
static sigjmp_buf con_env;
#else
static jmp_buf con_env;
#endif /* CK_POSIX_SIG */

/*
  pipeint() handles CK_FORK_SIG signals from the lower (port) fork.
  It reads a function code from the pipe that connects the two forks,
  then reads additional data from the pipe, then handles it.
*/
static SIGTYP
pipeint(arg) int arg; {			/* Dummy argument */
    int code;
    /* IMPORTANT: At this point, the child fork is waiting for CK_FORK_SIG
       (eventually for SIGKILL) inside of ck_sndmsg().  So we can't get any
       subsequent CK_FORK_SIG from child before we send it CK_FORK_SIG.
    */
    signal(CK_FORK_SIG, SIG_IGN);	/* Ignore CK_FORK_SIG now */
    debug(F101,"pipeint arg","",arg);

    read(xpipe[0], &code, sizeof(code)); /* Get function code from pipe */
    debug(F101,"pipeint code","",code);

    switch (code & 255) {
      case CEV_HUP:
        sjval = CEV_HUP;		/* Set global variable */
#ifdef CK_POSIX_SIG			/* Notify parent process to stop */
        siglongjmp(con_env,sjval);
#else
        longjmp(con_env,sjval);
#endif /* CK_POSIX_SIG */
        /* NOTREACHED */
      case CEV_DUP:			/* Child sends duplex change */
	read(xpipe[0], (char *)&duplex, sizeof(duplex));
	debug(F101,"pipeint duplex","",duplex);
	break;

#ifdef CK_APC
      case CEV_APC:			/* Application Program Command */
	read(xpipe[0], (char *)&apclength, sizeof(apclength));
	read(xpipe[0], apcbuf, apclength+1); /* Include trailing zero byte */
	debug(F111,"APC at parent",apcbuf,apclength);
	read(xpipe[0], (char *)&ibc, sizeof(ibc)); /* Copy child's */
	if (ibc > 0) {				   /* input buffer. */
	    read(xpipe[0], (char *)&ibp, sizeof(ibp));
	    read(xpipe[0], ibp, ibc);
	}
	obc = 0; obp = obuf; *obuf = NUL; /* Because port fork flushed */
	sjval = CEV_APC;
#ifdef CK_POSIX_SIG
	siglongjmp(con_env,sjval);
#else
	longjmp(con_env,sjval);
#endif /* CK_POSIX_SIG */
	/* NOTREACHED */
#endif /* CK_APC */

#ifdef SUNX25
      case CEV_PAD:			/* X.25 PAD parameter change */
	debug(F100,"pipeint PAD change","",0);
	read(xpipe[0],padparms,MAXPADPARMS);
	sjval = CEV_PAD;		/* Set global variable. */
#ifdef COMMENT				/* We might not need to do this... */
#ifdef CK_POSIX_SIG
	siglongjmp(con_env,sjval);
#else
	longjmp(con_env,sjval);
#endif /* CK_POSIX_SIG */
	/* NOTREACHED */
#else  /* COMMENT */
	break;
#endif /* COMMENT */
#endif /* SUNX25 */
    }
    signal(CK_FORK_SIG, pipeint);	/* Set up signal handler */
    kill(pid, CK_FORK_SIG);		/* Signal the port fork ... */
}

/*  C K C P U T C  --  C-Kermit CONNECT Put Character to Screen  */
/*
  Output is buffered to avoid slow screen writes on fast connections.
  NOTE: These could (easily?) become macros ...
*/
int
ckcputf() {				/* Dump the output buffer */
    int x = 0;
    if (obc > 0)			/* If we have any characters, */
      x = conxo(obc,obuf);		/* dump them, */
    obp = obuf;				/* reset the pointer */
    obc = 0;				/* and the counter. */
    return(x);				/* Return conxo's return code */
}

int
ckcputc(c) int c; {
    int x;

    *obp++ = c & 0xff;			/* Deposit the character */
    obc++;				/* Count it */
    if (ibc == 0 ||			/* If input buffer about empty */
	obc == OBUFL) {			/* or output buffer full */
	debug(F101,"CKCPUTC obc","",obc);
	x = conxo(obc,obuf);		/* dump the buffer, */
	obp = obuf;			/* reset the pointer */
	obc = 0;			/* and the counter. */
	return(x);			/* Return conxo's return code */
    } else return(0);
}

/*  C K C G E T C  --  C-Kermit CONNECT Get Character  */
/*
  Buffered read from communication device.
  Returns the next character, refilling the buffer if necessary.
  On error, returns ttinc's return code (see ttinc() description).
  Dummy argument for compatible calling conventions with ttinc().
  NOTE: We don't have a macro for this because we have to pass
  a pointer to this function as an argument to tn_doop().
*/
int
ckcgetc(dummy) int dummy; {
    int c, n;

    debug(F101,"CKCGETC 1 ibc","",ibc); /* Log */
    if (ibc < 1) {			/* Need to refill buffer? */
	ibc = 0;			/* Yes, reset count */
	ibp = ibuf;			/* and buffer pointer */
	debug(F100,"CKCGETC 1 calling ttinc(0)","",0); /* Log */
	c = ttinc(0);			/* Read one character, blocking */
	debug(F101,"CKCGETC 1 ttinc(0)","",c); /* Log */
	if (c < 0) {			/* If error, return error code */
	    return(c);
	} else {			/* Otherwise, got one character */
	    *ibp++ = c;			/* Advance buffer pointer */
	    ibc++;			/* and count. */
	}

	/* Now quickly read any more that might have arrived */

	if ((n = ttchk()) > 0) {	/* Any more waiting? */
	    if (n > (IBUFL - ibc))	/* Get them all at once. */
	      n = IBUFL - ibc;		/* Don't overflow buffer */
	    if ((n = ttxin(n,(CHAR *)ibp)) > 0)
	      ibc += n;			/* Advance counter */
	}
	debug(F101,"CKCGETC 2 ibc","",ibc); /* Log how many */
	ibp = ibuf;
    }
    c = *ibp++ & 0xff;			/* Get next character from buffer */
    ibc--;				/* Reduce buffer count */
    return(c);				/* Return the character */
}

/*
   Keyboard handling, buffered for speed, which is needed when C-Kermit is
   in CONNECT mode between two other computers that are transferring data.
*/
static char *kbp;			/* Keyboard input buffer pointer */
static int kbc;				/* Keyboard input buffer count */

#ifdef pdp11				/* Keyboard input buffer length */
#define KBUFL 32			/* Small for PDP-11 UNIX */
#else
#define KBUFL 257			/* Regular kernel size for others */
#endif /* pdp11 */

#ifdef DYNAMIC
static char *kbuf = NULL;
#else
static char kbuf[KBUFL];
#endif /* DYNAMIC */

/* Macro for reading keystrokes. */

#define CONGKS() (((--kbc)>=0) ? ((int)(*kbp++) & 0377) : kbget())

/*
  Note that we call read() directly here, normally a no-no, but in this case
  we know it's UNIX and we're only doing what coninc(0) would have done,
  except we're reading a block of characters rather than just one.  There is,
  at present, no conxin() analog to ttxin() for chunk reads, and instituting
  one would only add function-call overhead as it would only be a wrapper for
  a read() call anyway.
*/
/*
  Another note: We stick in this read() till the user types something.
  But the other (lower) fork is running too, and on TELNET connections,
  it will signal us to indicate echo-change negotiations, and this can
  interrupt the read().  Some UNIXes automatically restart the interrupted
  system call, others return from it with errno == EINTR.
*/
static int				/* Keyboard buffer filler */
kbget() {
#ifdef EINTR
    int tries = 10;			/* If read() is interrupted, */
    int ok = 0;
    while (tries-- > 0) {		/* try a few times... */
#endif /* EINTR */
	if ((kbc = conchk()) < 1)	/* How many chars waiting? */
	  kbc = 1;			/* If none or dunno, wait for one. */
	else if (kbc > KBUFL)		/* If too many, */
	  kbc = KBUFL;			/* only read this many. */
	if ((kbc = read(0, kbuf, kbc)) < 1) { /* Now read it/them. */
	    debug(F101,"kbget errno","",errno);	/* Got an error. */
#ifdef EINTR
	    if (errno == EINTR)		/* Interrupted system call. */
	      continue;			/* Try again, up to limit. */
	    else			/* Something else. */
#endif /* EINTR */
	      return(-1);		/* Pass along read() error. */
	}
#ifdef EINTR
	else { ok = 1; break; }
    }
    if (!ok) return(-1);
#endif /* EINTR */
    kbp = kbuf;				/* Adjust buffer pointer, */
    kbc--;				/* count, */
    return((int)(*kbp++) & 0377);	/* and return first character. */
}


/*  C O N E C T  --  Perform terminal connection  */

int
conect() {
    int	n;			/* General purpose counter */

    int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    int c2;			/* A copy of c */
    int csave;			/* Another copy of c */
    int tx;			/* tn_doop() return code */
#ifdef SUNX25
    int i;			/* Worker for X.25 code */
#endif /* SUNX25 */
#ifdef NETCONN

/* NOTE: SIGPIPE is now ignored the whole time Kermit is running. */
/* (as of 30 Aug 94) */
/* #ifdef SIGPIPE */
/*    SIGTYP (*sigpiph)() = NULL; */
/* #endif */ /* SIGPIPE */

#endif /* NETCONN */
#ifndef NOESCSEQ
    int apcrc;
#endif /* NOESCSEQ */

    int conret = 0;			/* Return value from conect() */

    debug(F101,"CONNECT fork signal","",CK_FORK_SIG);
    signal(CK_FORK_SIG, SIG_IGN);	/* Initial CK_FORK_SIG handling, */
/*
  The following ttimoff() call should not be necessary, but evidently there
  are cases where a timer is left active and then goes off, taking a longjmp
  to nowhere after the program's stack has changed.  In any case, this is
  safe because the CONNECT module uses no timer of any kind, and no other timer
  should be armed while Kermit is in CONNECT mode.
*/
    ttimoff();				/* Turn off any timer interrupts */

    if (!local) {
#ifdef NETCONN
	printf("Sorry, you must SET LINE or SET HOST first\n");
#else
	printf("Sorry, you must SET LINE first\n");
#endif /* NETCONN */
	goto conret0;
    }
    if (speed < 0L && network == 0 && ttfdflg == 0) {
	printf("Sorry, you must SET SPEED first\n");
	goto conret0;
    }
#ifdef TCPSOCKET
    if (network && (nettype != NET_TCPB)
#ifdef SUNX25
        && (nettype != NET_SX25)
#endif /* SUNX25 */
    ) {
	printf("Sorry, network type not supported\n");
	goto conret0;
    }
#endif /* TCPSOCKET */

#ifdef DYNAMIC
    if (!ibuf) {
	if (!(ibuf = malloc(IBUFL+1))) { /* Allocate input line buffer */
	    printf("Sorry, CONNECT input buffer can't be allocated\n");
	    goto conret0;
	} else {
	    ibp = ibuf;
	    ibc = 0;
	}
    }
    if (!obuf) {
	if (!(obuf = malloc(OBUFL+1))) {    /* Allocate output line buffer */
	    printf("Sorry, CONNECT output buffer can't be allocated\n");
	    goto conret0;
	} else {
	    obp = obuf;
	    obc = 0;
	}
    }
    if (!kbuf) {
	if (!(kbuf = malloc(KBUFL+1))) { /* Allocate keyboard input buffer */
	    printf("Sorry, CONNECT keyboard buffer can't be allocated\n");
	    goto conret0;
	}
    }
    if (!temp) {
	if (!(temp = malloc(TMPLEN+1))) { /* Allocate temporary buffer */
	    printf("Sorry, CONNECT temporary buffer can't be allocated\n");
	    goto conret0;
	}
    }
#else
#ifdef COMMENT
    ibp = ibuf;				
    ibc = 0;
#endif /* COMMENT */
    obp = obuf;
    obc = 0;
#endif /* DYNAMIC */

    kbp = kbuf;				/* Always clear these. */
    *kbp = NUL;				/* No need to preserve them between */
    kbc = 0;				/* CONNECT sessions. */

#ifdef DEBUG
    if (deblog) {
	debug(F101,"ckucon entry ttyfd","",ttyfd);
	debug(F101,"ckucon entry ibc","",ibc);
	debug(F101,"ckucon entry obc","",obc);
	debug(F101,"ckucon entry kbc","",kbc);
	if (ttyfd > -1) {
	    n = ttchk();
	    debug(F101,"ckucon entry ttchk","",n);
	}
    }    
#endif /* DEBUG */    

    if (ttyfd < 0) {			/* If communication device not open */
	debug(F111,"ckucon opening",ttname,0); /* Open it now */
	if (ttopen(ttname,
		   &local,
		   network ? -nettype : mdmtyp,
		   0
		   ) < 0) {
	    sprintf(temp,"Sorry, can't open %s",ttname);
	    perror(temp);
	    debug(F110,"ckucon open failure",ttname,0);
	    goto conret0;
	}
    }
    dohangup = 0;			/* Hangup not requested yet */
#ifdef SUNX25
    dox25clr = 0;			/* X.25 clear not requested yet */
#endif /* SUNX25 */

    if (!quiet
#ifdef CK_APC
	&& !apcactive
#endif /* CK_APC */
	) {
#ifdef NETCONN
	if (network) {
	    printf("Connecting to host %s",ttname);
#ifdef SUNX25
	    if (nettype == NET_SX25)
	      printf(", Link ID %d, LCN %d",linkid,lcn);
#endif /* SUNX25 */
	} else {
#endif /* NETCONN */
	    printf("Connecting to %s",ttname);
	    if (speed > -1L) printf(", speed %ld",speed);
#ifdef NETCONN
	}
#endif /* NETCONN */
	printf(".\r\nThe escape character is Ctrl-%c (ASCII %d, %s)\r\n",
	       ctl(escape), escape, (escape == 127 ? "DEL" : ccntab[escape]));
	printf("Type the escape character followed by C to get back,\r\n");
	printf("or followed by ? to see other options.\r\n");
	if (seslog) {
	    printf("(Session logged to %s, ",sesfil);
	    printf("%s)\r\n", sessft ? "binary" : "text");
	}
	if (debses) printf("Debugging Display...)\r\n");
	fflush(stdout);
    }

/* Condition console terminal and communication line */	    

    if (conbin((char)escape) < 0) {
	printf("Sorry, can't condition console terminal\n");
	goto conret0;
    }
    debug(F101,"CONNECT cmask","",cmask);
    debug(F101,"CONNECT cmdmsk","",cmdmsk);
    debug(F101,"CONNECT speed before ttvt","",speed);
    if ((n = ttvt(speed,flow)) < 0) {	/* Enter "virtual terminal" mode */
	debug(F101,"CONNECT ttvt","",n);
	tthang();			/* Hang up and close the device. */
	ttclos(0);
	if (ttopen(ttname,		/* Open it again... */
		   &local,
		   network ? -nettype : mdmtyp,
		   0
		   ) < 0) {
	    sprintf(temp,"Sorry, can't reopen %s",ttname);
	    perror(temp);
	    goto conret0;
	}
	if (ttvt(speed,flow) < 0) {	/* Try virtual terminal mode again. */
	    conres();			/* Failure this time is fatal. */
	    printf("Sorry, Can't condition communication line\n");
	    goto conret0;
	}
    }
    debug(F101,"CONNECT ttvt ok, escape","",escape);

#ifndef NOCSETS
/* Set up character set translations */

    tcs = gettcs(tcsr,tcsl);		/* Get intermediate set. */

    if (tcsr == tcsl) {			/* Remote and local sets the same? */
	sxo = rxo = NULL;		/* If so, no translation. */
	sxi = rxi = NULL;
    } else {				/* Otherwise, set up */
	sxo = xls[tcs][tcsl];		/* translation function */
	rxo = xlr[tcs][tcsr];		/* pointers for output functions */
	sxi = xls[tcs][tcsr];		/* and for input functions. */
	rxi = xlr[tcs][tcsl];
    }
/*
  This is to prevent use of zmstuff() and zdstuff() by translation functions.
  They only work with disk i/o, not with communication i/o.  Luckily Russian
  translation functions don't do any stuffing...
*/
    langsv = language;
#ifndef NOCYRIL
    if (language != L_RUSSIAN)
#endif /* NOCYRIL */
      language = L_USASCII;

#ifdef COMMENT
#ifdef DEBUG
    if (deblog) {
	debug(F101,"tcs","",tcs);
	debug(F101,"tcsl","",tcsl);
	debug(F101,"tcsr","",tcsr);
	debug(F101,"fcsinfo[tcsl].size","",fcsinfo[tcsl].size);
	debug(F101,"fcsinfo[tcsr].size","",fcsinfo[tcsr].size);
    }
#endif /* DEBUG */
#endif /* COMMENT */

#ifndef NOESCSEQ
/*
  We need to activate the escape-sequence recognition feature when:
   (a) translation is elected, AND
   (b) the local and/or remote set is a 7-bit set other than US ASCII.
 Or:
   SET TERMINAL APC is not OFF (handled in the next statement).
*/
    escseq = (tcs != TC_TRANSP) &&	/* Not transparent */
      (fcsinfo[tcsl].size == 128 || fcsinfo[tcsr].size == 128) && /* 7 bits */
	(fcsinfo[tcsl].code != FC_USASCII); /* But not ASCII */
#endif /* NOESCSEQ */
#endif /* NOCSETS */

#ifndef NOESCSEQ
#ifdef CK_APC
    escseq = escseq || (apcstatus != APC_OFF);
    apcactive = 0;			/* An APC command is not active */
    apclength = 0;			/* ... */
#endif /* CK_APC */
    inesc = ES_NORMAL;			/* Initial state of recognizer */
    debug(F101,"escseq","",escseq);
#endif /* NOESCSEQ */

/*
  This is a label we jump back to when the lower fork sensed the need
  to change modes.  As of 5A(178), this is used only by X.25 code
  (perhaps unnecessarily? -- The X.25 code needs a lot of testing and
  cleaning up...)  As of 5A(190), it should not be used at all.
*/
#ifdef COMMENT
newfork: /* Label no longer used */
#endif /* COMMENT */
    debug(F100,"CONNECT newfork","",0);
    parent_id = getpid();		/* Get parent's pid for signalling */

    if (xpipe[0] > -1)			/* If old pipe hanging around, close */
      close(xpipe[0]);
    xpipe[0] = -1;
    if (xpipe[1] > -1)
      close(xpipe[1]);
    xpipe[1] = -1;
    goterr = 0;				/* Error flag for pipe & fork */
    if (pipe(xpipe) != 0) {		/* Create new pipe to pass info */
	perror("Can't make pipe");	/* between forks. */
	goterr = 1;
    } else if ((pid = fork()) == (PID_T) -1) { /* Pipe OK, make port fork. */
	perror("Can't make port fork");
	goterr = 1;	
    }
    if (goterr) {			/* Failed to make pipe or fork */
	conres();			/* Reset the console. */
	if (!quiet) {
	    printf("\r\nCommunications disconnect (Back at %s)\r\n",
		   *myhost ?
		   myhost :
#ifdef UNIX
		   "local UNIX system"
#else
		   "local system"
#endif /* UNIX */
		   );
	}
	printf("\n");
	what = W_NOTHING;		/* So console modes are set right. */
#ifndef NOCSETS
	language = langsv;		/* Restore language */
#endif /* NOCSETS */
	parent_id = (PID_T) 0;		/* Clean up */
	goto conret1;
    }

/* Upper fork (KEYB fork) reads keystrokes and sends them out. */

    if (pid) {				/* pid != 0, so I am the upper fork. */
  
/*
  Before doing anything significant, the child fork must wait for a go-ahead
  character from xpipe[0].  Before starting to wait, we have enough time to
  clear buffers and set up the signal handler.  When done with this, we will
  allow the child to continue by satisfying its pending read.

  Remember the child and parent have separate address space.  The child has
  its own copy of input buffers, so we must clear the input buffers in the
  parent.  Otherwise strange effects may occur, like chunks of characters
  repeatedly echoed on terminal screen.  The child process is designed to
  empty its input buffers by reading all available characters and either
  echoing them on the terminal screen or saving them for future use in the
  parent.  The latter case happens during APC processing - see the code around
  CEV_APC occurrences to see how the child passes its ibuf etc to parent via
  xpipe, for preservation until the next entry to this module, to ensure that
  no characters are lost between CONNECT sessions.
*/

/*
  This one needs a bit of extra explanation...  In addition to the CONNECT
  module's own buffers, which are communicated and synchronized via xpipe,
  the low-level UNIX communication routines (ttinc, ttxin, etc) are also
  buffered, statically, in the ckutio.c module.  But when the two CONNECT
  forks split off, the lower fork is updating this buffer's pointers and
  counts, but the upper fork never finds out about it and still has the old
  ones.  The following UNIX-specific call to the ckutio.c module takes care
  of this...  Without it, we get dual echoing of incoming characters.
*/
	ttflux();
/*
  At this point, perhaps you are wondering why we use forks at all.  It is
  simply because there is no other method portable among all UNIX variations.
*/
	ibp = ibuf;			/* Clear ibuf[]. */
	ibc = 0;			/* Child now has its own copy */
	signal(CK_FORK_SIG, pipeint);	/* Handler for messages from child. */
	write(xpipe[1], ibuf, 1);	/* Allow child to proceed */
	close(xpipe[1]); xpipe[1] = -1; /* Parent - prevent future writes */

#ifdef COMMENT
#ifdef NETCONN
#ifdef SIGPIPE
	/* SIGPIPE is raised if a process sends on a broken stream */
	/* (do it, but only once!) */
	if (network && ! sigpiph)
	  sigpiph = signal(SIGPIPE, SIG_IGN);
#endif /* SIGPIPE */
#endif /* NETCONN */
#endif /* COMMENT */

	what = W_CONNECT;		/* Keep track of what we're doing */
	active = 1;
	debug(F101,"CONNECT keyboard fork duplex","",duplex);
/*
  Catch communication errors or mode changes in lower fork.

  Note: Some C compilers (e.g. Cray UNICOS) interpret the ANSI C standard
  about setjmp() in a way that disallows constructions like:

        if ((var = [sig]setjmp(env)) == 0) ...

  which prevents the value returned by longjmp() from being used at all.
  So the signal handlers set a global variable, sjval, instead.
*/
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(con_env,1)
#else
	    setjmp(con_env)
#endif /* CK_POSIX_SIG */
	    == 0) {	/* Normal entry... */
	    sjval = CEV_NO;		/* Initialize setjmp return code. */
#ifdef SUNX25
	    if (network && nettype == NET_SX25) {
		obufl = 0;
		bzero (x25obuf,sizeof(x25obuf)) ;
	    }
#endif /* SUNX25 */
/*
  Here is the big loop that gets characters from the keyboard and sends them
  out the communication device.  There are two components to the communication
  path: the connection from the keyboard to C-Kermit, and from C-Kermit to
  the remote computer.  The treatment of the 8th bit of keyboard characters 
  is governed by SET COMMAND BYTESIZE (cmdmsk).  The treatment of the 8th bit
  of characters sent to the remote is governed by SET TERMINAL BYTESIZE
  (cmask).   This distinction was introduced in edit 5A(164).
*/
	    while (active) {
#ifndef NOSETKEY
		if (kmptr) {		/* Have current macro? */
		    debug(F100,"kmptr non NULL","",0);
		    if ((c = (CHAR) *kmptr++) == NUL) { /* Get char from it */
			kmptr = NULL;	/* If no more chars,  */
			debug(F100,"macro empty, continuing","",0);
			continue;	/* reset pointer and continue */
		    }
		    debug(F000,"char from macro","",c);
		} else			/* No macro... */
#endif /* NOSETKEY */
		  c = CONGKS();		/* Read from keyboard */

		debug(F101,"** KEYB","",c);

                if (c == -1) {		/* If read() got an error... */
		    debug(F101,"CONNECT keyboard read errno","",errno);
#ifdef COMMENT
/*
 This seems to cause problems.  If read() returns -1, the signal has already
 been delivered, and nothing will wake up the pause().
*/
		    pause();		/* Wait for transmitter to finish. */
#else
#ifdef A986
/*
  On Altos machines with Xenix 3.0, pressing DEL in connect mode brings us
  here (reason unknown).  The console line discipline at this point has
  intr = ^C.  The communications tty has intr = DEL but we get here after
  pressing DEL on the keyboard, even when the remote system has been set not
  to echo.  With A986 defined, we stay in the read loop and beep only if the
  offending character is not DEL.
*/
		    if ((c & 127) != 127) conoc(BEL);
#else
#ifdef EINTR
/*
   This can be caused by the other fork signalling this one about 
   an echoing change during TELNET negotiations.
*/
		    if (errno == EINTR)
		      continue;
#endif /* EINTR */
		    conoc(BEL);		/* Otherwise, beep */
		    active = 0;		/* and terminate the read loop */
		    continue;
#endif /* A986 */
#endif /* COMMENT */
		}
		c &= cmdmsk;		/* Do any requested masking */
#ifndef NOSETKEY
/*
  Note: kmptr is NULL if we got character c from the keyboard, and it is
  not NULL if it came from a macro.  In the latter case, we must avoid
  expanding it again.
*/
		if (!kmptr && macrotab[c]) { /* Macro definition for c? */
		    kmptr = macrotab[c];     /* Yes, set up macro pointer */
		    continue;		     /* and restart the loop, */
		} else c = keymap[c];	     /* else use single-char keymap */
#endif /* NOSETKEY */
		if (
#ifndef NOSETKEY
		    !kmptr &&
#endif /* NOSETKEY */
		    ((c & 0x7f) == escape)) { /* Escape character? */
		    debug(F000,"CONNECT got escape","",c);
		    c = CONGKS() & 0177; /* Got esc, get its arg */
		    /* No key mapping here */
		    doesc((char) c);	/* Now process it */

		} else {		/* It's not the escape character */
		    csave = c;		/* Save it before translation */
					/* for local echoing. */
#ifndef NOCSETS
#ifdef NOESCSEQ
		    /* Translate character sets */
		    if (sxo) c = (*sxo)(c); /* From local to intermediate. */
		    if (rxo) c = (*rxo)(c); /* From intermediate to remote. */
#else
		    if (inesc == ES_NORMAL) { /* If not inside escape seq.. */
			/* Translate character sets */
			if (sxo) c = (*sxo)((char)c); /* Local-intermediate */
			if (rxo) c = (*rxo)((char)c); /* Intermediate-remote */
		    }
		    if (escseq) apcrc = chkaes((char)c);
#endif /* NOESCSEQ */
#endif /* NOCSETS */
/*
 If Shift-In/Shift-Out is selected and we have a 7-bit connection,
 handle shifting here.
*/
		    if (sosi) {		     /* Shift-In/Out selected? */
			if (cmask == 0177) { /* In 7-bit environment? */
			    if (c & 0200) {          /* 8-bit character? */
				if (outshift == 0) { /* If not shifted, */
				    ttoc(dopar(SO)); /* shift. */
				    outshift = 1;
				}
			    } else {
				if (outshift == 1) { /* 7-bit character */
				    ttoc(dopar(SI)); /* If shifted, */
				    outshift = 0;    /* unshift. */
				}
			    }
			}
			if (c == SO) outshift = 1;   /* User typed SO */
			if (c == SI) outshift = 0;   /* User typed SI */
		    }
		    c &= cmask;		/* Apply Kermit-to-host mask now. */
#ifdef SUNX25
                    if (network && nettype == NET_SX25) {
                        if (padparms[PAD_ECHO]) {
                            if (debses)
			      conol(dbchr(c)) ;
                            else
			      if ((c != padparms[PAD_CHAR_DELETE_CHAR])   &&
				  (c != padparms[PAD_BUFFER_DELETE_CHAR]) &&
				  (c != padparms[PAD_BUFFER_DISPLAY_CHAR]))
                                conoc(c) ;
                            if (seslog) logchar(c);
                        }
			if (c == CR && (padparms[PAD_LF_AFTER_CR] == 4 ||
					padparms[PAD_LF_AFTER_CR] == 5)) {
                            if (debses)
			      conol(dbchr(LF)) ;
                            else
			      conoc(LF) ;
                            if (seslog) logchar(LF);
                        }
                        if (c == padparms[PAD_BREAK_CHARACTER])
			  breakact();
                        else if (padparms[PAD_DATA_FORWARD_TIMEOUT]) {
                            tosend = 1;
                            x25obuf [obufl++] = c;
                        } else if (((c == padparms[PAD_CHAR_DELETE_CHAR])  ||
				    (c == padparms[PAD_BUFFER_DELETE_CHAR]) ||
				    (c == padparms[PAD_BUFFER_DISPLAY_CHAR])) 
				   && (padparms[PAD_EDITING]))
			  if (c == padparms[PAD_CHAR_DELETE_CHAR])
			    if (obufl > 0) {
				conol("\b \b"); obufl--;
			    } else {}
			  else if (c == padparms[PAD_BUFFER_DELETE_CHAR]) {
			      conol ("\r\nPAD Buffer Deleted\r\n");
			      obufl = 0;
			  }
			  else if (c == padparms[PAD_BUFFER_DISPLAY_CHAR]) {
			      conol("\r\n");
			      conol(x25obuf);
			      conol("\r\n");
			  } else {} 
                        else {
                            x25obuf [obufl++] = c;
                            if (obufl == MAXOX25) tosend = 1;
                            else if (c == CR) tosend = 1;
                        }
                        if (tosend) 
			  if (ttol(x25obuf,obufl) < 0) {
			      perror ("\r\nCan't send characters");
			      active = 0;
			  } else {
			      bzero (x25obuf,sizeof(x25obuf));
			      obufl = 0;
			      tosend = 0;
			  } else {};
                    } else {
#endif /* SUNX25 */ 
		    if (c == '\015') {		/* Carriage Return */
			int stuff = -1;
			if (tnlm) {		/* TERMINAL NEWLINE ON */
			    stuff = LF; 	/* Stuff LF */
#ifdef TNCODE
			} else if (network &&	/* TELNET NEWLINE ON/OFF/RAW */
				   (ttnproto == NP_TELNET) &&
				   (tn_nlm != TNL_CR)) {
			    stuff = (tn_nlm == TNL_CRLF) ? LF : NUL;
#endif /* TNCODE */
			}
			if (stuff > -1) {
			    ttoc(dopar('\015'));	/* Send CR */
			    if (duplex) conoc('\015');	/* Maybe echo CR */
			    c = stuff;			/* Char to stuff */
			    csave = c;
			}
		    }
#ifdef TNCODE
/* If user types the 0xff character (TELNET IAC), it must be doubled. */
		    else		/* Not CR */
		      if ((dopar((CHAR) c) == IAC) && /* IAC (0xff) */
			  network && (ttnproto == NP_TELNET)) {
					/* Send one copy now */
			ttoc((char)IAC); /* and the other one just below. */
		    }
#endif /* TNCODE */
		    /* Send the character */

		    if (ttoc((char)dopar((CHAR) c)) > -1) {
		    	if (duplex) {	/* If half duplex, must echo */
			    if (debses)
			      conol(dbchr(csave)); /* the original char */
			    else	           /* not the translated one */
			      conoc((char)csave);
			    if (seslog) { /* And maybe log it too */
				c2 = csave;
				if (sessft == 0 && csave == '\r')
				  c2 = '\n';
				logchar((char)c2);
			    }
			}
    	    	    } else {
			perror("\r\nCan't send character");
			active = 0;
		    }
#ifdef SUNX25
		} 
#endif /* SUNX25 */
		}
	    }
	    /* now active == 0 */
            signal(CK_FORK_SIG, SIG_IGN); /* Turn off CK_FORK_SIG */
	    sjval = CEV_NO;		/* Set to hangup */
	}				/* Come here on termination of child */

/* longjmp() executed in pipeint() (parent only!) comes here */

/*
  Now the child fork is gone or is waiting for CK_FORK_SIG in ck_sndmsg().
  So we can't get (in the parent) any subsequent CK_FORK_SIG signals until
  we signal the child with CK_FORK_SIG.
*/
	debug(F100,"CONNECT signaling port fork","",0);
	signal(CK_FORK_SIG, SIG_IGN);	/* Turn this off */
	debug(F100,"CONNECT killing port fork","",0);
	kill(pid,9);			/* Done, kill inferior fork. */
	debug(F100,"CONNECT killed port fork","",0);
	pid = -1;
	wait((WAIT_T *)0);		/* Wait till gone. */
	if (sjval == CEV_HUP) {		/* Read error on comm device */
	    dohangup = 1;		/* so we want to hang up our side */
#ifdef NETCONN
	    if (network) {		/* and/or close network connection */
		ttclos(0);
#ifdef SUNX25
		if (nettype == NET_SX25) /* If X.25, restore the PAD params */
		  initpad();
#endif /* SUNX25 */
	    }
#endif /* NETCONN */
	}
#ifdef SUNX25
#ifdef COMMENT
	/* Not used, should not be needed -- see pipeint()... */
	if (sjval == CEV_PAD)		/* If it was a mode change, go back */
	  goto newfork;			/* and coordinate with other fork. */
#endif /* COMMENT */
#endif /* SUNX25 */

#ifdef CK_APC
	if (sjval == CEV_APC) {		/* Application Program Command rec'd */
	    apcactive = 1;		/* Flag APC as active */
	    active = 0;			/* Flag CONNECT as inactive */
	}
#endif /* CK_APC */
	conres();			/* Reset the console. */
	if (dohangup > 0) {		/* If hangup requested, do that. */
#ifndef NODIAL
	    if (dohangup > 1)		/* User asked for it */
	      if (mdmhup() < 1)		/* Maybe hang up via modem */
#endif /* NODIAL */
		tthang();		/* And make sure we don't hang up */
	    dohangup = 0;		/* again unless requested again. */
	}

#ifdef COMMENT
#ifdef NETCONN
#ifdef SIGPIPE
	if (network && sigpiph)		/* Restore previous SIGPIPE handler */
	  (VOID) signal(SIGPIPE, sigpiph);
#endif /* SIGPIPE */
#endif /* NETCONN */
#endif /* COMMENT */

#ifdef SUNX25
	if (dox25clr) {			/* If X.25 Clear requested */
	    x25clear();			/* do that. */
	    initpad();
	    dox25clr = 0;		/* But only once. */
	}
#endif /* SUNX25 */

	if (quitnow) doexit(GOOD_EXIT,xitsta); /* Exit now if requested. */
  	if (!quiet
#ifdef CK_APC
	    && !apcactive
#endif /* CK_APC */
	    )
	  printf("(Back at %s)", *myhost ? myhost : "local UNIX system");
#ifdef CK_APC
        if (!apcactive)
#endif /* CK_APC */
	  printf("\n");
	what = W_NOTHING;		/* So console modes set right. */
#ifndef NOCSETS
	language = langsv;		/* Restore language */
#endif /* NOCSETS */
	parent_id = (PID_T) 0;
	goto conret1;

    } else {	/* *** */		/* Inferior reads, prints port input */

	if (priv_can()) {		/* Cancel all privs */
	    printf("?setuid error - fatal\n");
	    doexit(BAD_EXIT,-1);
	}
	signal(SIGINT, SIG_IGN);	/* In case these haven't been */
	signal(SIGQUIT, SIG_IGN);	/* inherited from above... */
	signal(CK_FORK_SIG, SIG_IGN);	/* CK_FORK_SIG not expected yet */

	inshift = outshift = 0;		/* Initial SO/SI shift state. */
	{				/* Wait for parent's setup */
	    int i;
	    while ((i = read(xpipe[0], &c, 1)) <= 0) {
		if (i < 0) {
		    pipemsg(CEV_HUP);	/* Read error - hangup */
		    ck_sndmsg();	/* Send and wait to be killed */
		    /* NOTREACHED */
		}			/* Restart interrupted read() */
	    }
	}
	close(xpipe[0]); xpipe[0] = -1;	/* Child - prevent future reads */
	debug(F100,"CONNECT starting port fork","",0);
	debug(F101,"CONNECT port fork ibc","",ibc);
	debug(F101,"CONNECT port fork obc","",obc);

	while (1) {			/* Fresh read, wait for a character. */
#ifdef SUNX25
	    if (network && (nettype == NET_SX25)) {
		bzero(x25ibuf,sizeof(x25ibuf)) ;
		if ((ibufl = ttxin(MAXIX25,x25ibuf)) < 0) {
		    if (ibufl == -2) {  /* PAD parms changes */
			pipemsg(CEV_PAD);
			write(xpipe[1],padparms,MAXPADPARMS);
			ck_sndmsg();
		    } else {
			if (!quiet)
			  printf("\r\nCommunications disconnect ");
		        pipemsg(CEV_HUP);
			ck_sndmsg();		/* Wait to be killed */
			/* NOTREACHED */
		    }
		    /* pause(); <--- SHOULD BE OBSOLETE NOW! */
		    /* BECAUSE pause() is done inside of ck_sndmsg() */
		}
		if (debses) {		/* Debugging output */
		    p = x25ibuf ;
                        while (ibufl--) { c = *p++; conol(dbchr(c)); }
		} else {
		    if (sosi
#ifndef NOCSETS
			|| tcsl != tcsr
#endif /* NOCSETS */
			) { /* Character at a time */
			for (i = 1; i < ibufl; i++) {
			    c = x25ibuf[i] & cmask;
			    if (sosi) { /* Handle SI/SO */
				if (c == SO) {
				    inshift = 1;
				    continue;
				} else if (c == SI) {
				    inshift = 0;
				    continue;
				}
				if (inshift)
				  c |= 0200;
			    }
#ifndef NOCSETS
#ifdef NOESCSEQ
			    /* Translate character sets */
			    /* From local to intermediate. */
			    if (sxo) c = (*sxo)(c);
			    /* From intermediate to remote. */
			    if (rxo) c = (*rxo)(c);
#else
			    /* If not inside escape sequence... */
			    if (inesc == ES_NORMAL) {
				/* Translate character sets */
				if (sxo) c = (*sxo)(c);
				if (rxo) c = (*rxo)(c);
			    }
			    if (escseq) apcrc = chkaes(c);
#endif /* NOESCSEQ */
#endif /* NOCSETS */
			    c &= cmdmsk; /* Apply command mask. */
			    conoc(c);    /* Output to screen */
			    logchar(c);  /* and session log */
			}
		    } else {		 /* All at once */
			for (i = 1; i < ibufl; i++)
			  x25ibuf[i] &= (cmask & cmdmsk);
			conxo(ibufl,x25ibuf);
			if (seslog) zsoutx(ZSFILE,x25ibuf,ibufl);
		    }
		}
		continue;

	    } else {			/* Not X.25... */
#endif /* SUNX25 */
/*
  Get the next communication line character from our internal buffer.
  If the buffer is empty, refill it.
*/
		c = ckcgetc(0);		/* Get next character */
		/* debug(F101,"CONNECT c","",c); */
		if (c < 0) {		/* Failed... */
		    debug(F101,"CONNECT disconnect ibc","",ibc);
		    debug(F101,"CONNECT disconnect obc","",obc);
		    ckcputf();		/* Flush CONNECT output buffer */
		    if (!quiet) {
			printf("\r\nCommunications disconnect ");
			if ( c == -3
#ifdef ultrix
/* This happens on Ultrix if there's no carrier */
			    && errno != EIO
#endif /* ultrix */
#ifdef UTEK
/* This happens on UTEK if there's no carrier */
			    && errno != EWOULDBLOCK
#endif /* UTEK */
			    )
			  perror("\r\nCan't read character");
		    }
#ifdef NOSETBUF
		    fflush(stdout);
#endif /* NOSETBUF */
		    tthang();		/* Hang up the connection */
		    pipemsg(CEV_HUP);
		    ck_sndmsg();	/* Wait to be killed */
		    /* NOTREACHED */
		}
		debug(F101,"** PORT","",c); /* Got character c OK. */
#ifdef TNCODE
		/* Handle TELNET negotiations... */

		if (c == IAC && network && ttnproto == NP_TELNET) {
		    debug(F100,"CONNECT got IAC","",0);
		    ckcputf();		/* Dump screen-output buffer */
		    if ((tx = tn_doop((CHAR)(c & 0xff),duplex,ckcgetc)) == 0) {
			continue;
		    } else if (tx == -1) { /* I/O error */
			if (!quiet)
			  printf("\r\nCommunications disconnect ");
#ifdef NOSETBUF
			fflush(stdout);
#endif /* NOSETBUF */
			pipemsg(CEV_HUP);
			ck_sndmsg();		/* Wait to be killed */
			/* NOTREACHED */
		    } else if ((tx == 1) && (!duplex)) { /* ECHO change */
			duplex = 1;	/* Turn on local echo */
			debug(F101,"CONNECT TELNET duplex change","",duplex);
			pipemsg(CEV_DUP); /* Tell parent */
			write(xpipe[1], &duplex, sizeof(duplex));
			ck_sndmsg();      /* Tell the parent fork */
			continue;
		    } else if ((tx == 2) && (duplex)) { /* ECHO change */
			duplex = 0;
			debug(F101,"CONNECT TELNET duplex change","",duplex);
			pipemsg(CEV_DUP);
			write(xpipe[1], &duplex, sizeof(duplex));
			ck_sndmsg();
			continue;
		    } else if (tx == 3) { /* Quoted IAC */
			c = 255;
		    } else continue;	/* Negotiation OK, get next char. */
		}
#endif /* TNCODE */
		if (debses) {		/* Output character to screen */
		    char *s;		/* Debugging display... */
		    s = dbchr(c);
		    while (*s)
		      ckcputc(*s++);
		} else {		/* Regular display ... */
		    c &= cmask;		/* Apply Kermit-to-remote mask */
		    if (sosi) {		/* Handle SI/SO */
			if (c == SO) {	/* Shift Out */
			    inshift = 1;
			    continue;
			} else if (c == SI) { /* Shift In */
			    inshift = 0;
			    continue;
			}
			if (inshift) c |= 0200; 
		    }
#ifndef NOCSETS
		    if (
#ifndef NOESCSEQ
			inesc == ES_NORMAL /* If not in an escape sequence */
#else
			1
#endif /* NOESCSEQ */
			) {		/* Translate character sets */
			if (sxi) c = (*sxi)((CHAR)c);
			if (rxi) c = (*rxi)((CHAR)c);
		    }
#endif /* NOCSETS */

#ifndef NOESCSEQ
		    if (escseq)		/* If handling escape sequences */
		      apcrc = chkaes((char)c); /* update our state */
#ifdef CK_APC
/*
  If we are handling APCs, we have several possibilities at this point:
   1. Ordinary character to be written to the screen.
   2. An Esc; we can't write it because it might be the beginning of an APC.
   3. The character following an Esc, in which case we write Esc, then char,
      but only if we have not just entered an APC sequence.
*/
		    if (escseq && apcstatus != APC_OFF) {
			if (inesc == ES_GOTESC)	/* Don't write ESC yet */
			  continue;
			else if (oldesc == ES_GOTESC && !apcactive) {
			    ckcputc(ESC);	/* Write saved ESC */
			    if (seslog) logchar((char)ESC);
			} else if (apcrc) {     /* We have an APC */
			    debug(F111,"APC complete",apcbuf,apclength);
			    ckcputf();		/* Force screen update */
			    pipemsg(CEV_APC);	/* Notify parent */
			    write(xpipe[1],
				  (char *)&apclength,
				  sizeof(apclength)
				  );
			    /* Write buffer including trailing NUL byte */

			    write(xpipe[1], apcbuf, apclength+1);

			    /* Copy our input buffer to the parent fork */

			    debug(F101,"APC complete ibc","",ibc);
			    debug(F101,"APC complete obc","",obc);
			    write(xpipe[1], (char *)&ibc, sizeof(ibc));
			    if (ibc > 0) {
				write(xpipe[1], (char *)&ibp, sizeof(ibp));
				write(xpipe[1], ibp, ibc);
			    }			
			    ck_sndmsg();		/* Wait to be killed */
			    /* NOTREACHED */
			}
		    }
#endif /* CK_APC */
#endif /* NOESCSEQ */

		    if (
#ifdef CK_APC
			!apcactive	/* Ignore APC sequences */
#else
			1
#endif /* CK_APC */
			) {
			c &= cmdmsk;	/* Apply command mask. */
			if (c == CR && tt_crd) { /* SET TERM CR-DISPLA CRLF? */
			    ckcputc(c);	     /* Yes, output CR */
			    if (seslog) logchar((char)c);
			    c = LF;	/* and insert a linefeed */
			}
			ckcputc(c);	/* Write character to screen */
		    }
		    if (seslog) logchar((char)c); /* Handle session log */
		}
#ifdef SUNX25
	    }   
#endif /* SUNX25 */	
	}
    }

conret1:
    conret = 1;
conret0:
    signal(CK_FORK_SIG, SIG_IGN);	/* In case this wasn't done already */
    debug(F101,"ckucon exit ibc","",ibc);
    debug(F101,"ckucon exit obc","",obc);
    close(xpipe[0]); xpipe[0] = -1;	/* Close the pipe */
    close(xpipe[1]); xpipe[1] = -1;
    return(conret);
}


/*  H C O N N E  --  Give help message for connect.  */

int
hconne() {
    int c;
    static char *hlpmsg[] = {
"\r\n  ? for this message",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
#ifdef CK_LBRK
"\r\n  L to send a Long BREAK",
#endif /* CK_LBRK */
#ifdef NETCONN
"\r\n  I to send a network interrupt packet",
#ifdef TCPSOCKET
"\r\n  A to send Are You There?",
#endif /* TCPSOCKET */
#ifdef SUNX25
"\r\n  R to reset X.25 virtual circuit",
#endif /* SUNX25 */
#endif /* NETCONN */
"\r\n  H to hangup and close the connection",
"\r\n  Q to hangup and quit Kermit",
"\r\n  S for status",
"\r\n  ! to push to local shell",
"\r\n  Z to suspend",
"\r\n  \\ backslash code:",
"\r\n    \\nnn  decimal character code",
"\r\n    \\Onnn octal character code",
"\r\n    \\Xhh  hexadecimal character code",
"\r\n    terminate with carriage return.",
"\r\n Type the escape character again to send the escape character, or",
"\r\n press the space-bar to resume the CONNECT command.\r\n\r\n",
"" };
    conol("\r\nPress C to return to ");
    conol(*myhost ? myhost : "the C-Kermit prompt");
    conol(", or:");
    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = CONGKS() & 0177;		/* Get character, strip any parity. */
    /* No key mapping or translation here */
    if (c != CMDQ)
      conoll(""); 
   return(c);				/* Return it. */
}


/*  D O E S C  --  Process an escape character argument  */

VOID
#ifdef CK_ANSIC
doesc(char c)
#else
doesc(c) char c;
#endif /* CK_ANSIC */
/* doesc */ {
    CHAR d;
  
    debug(F101,"doesc","",c);
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar((CHAR) c); ttoc((char) d); return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);

	switch(c) {

	case 'c':			/* Escape back to prompt */
	case '\03':
	    active = 0; conol("\r\n"); return;

	case 'b':			/* Send a BREAK signal */
	case '\02':
	    ttsndb(); return;

#ifdef NETCONN
	case 'i':			/* Send Interrupt */
	case '\011':
#ifdef TCPSOCKET
#ifndef IP
#define IP 244
#endif /* IP */
	    if (network && ttnproto == NP_TELNET) { /* TELNET */
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) IP;	/* Interrupt Process */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else 
#endif /* TCPSOCKET */
#ifdef SUNX25
            if (network && (nettype == NET_SX25)) { /* X.25 */
		(VOID) x25intr(0);	            /* X.25 interrupt packet */
		conol("\r\n");
	    } else
#endif /* SUNX25 */
	      conoc(BEL);
	    return;

#ifdef TCPSOCKET
	case 'a':			/* "Are You There?" */
	case '\01':
#ifndef AYT
#define AYT 246
#endif /* AYT */
	    if (network && ttnproto == NP_TELNET) {
		temp[0] = (CHAR) IAC;	/* I Am a Command */
		temp[1] = (CHAR) AYT;	/* Are You There? */
		temp[2] = NUL;
		ttol((CHAR *)temp,2);
	    } else conoc(BEL);
	    return;
#endif /* TCPSOCKET */
#endif /* NETCONN */

#ifdef CK_LBRK
	case 'l':			/* Send a Long BREAK signal */
	    ttsndlb(); return;
#endif /* CK_LBRK */

	case 'h':			/* Hangup */
     /*	case '\010': */			/* No, too dangerous */
#ifdef SUNX25
            if (network && (nettype == NET_SX25)) dox25clr = 1;
            else
#endif /* SUNX25 */
	    dohangup = 2; active = 0; conol("\r\nHanging up "); return;

#ifdef SUNX25
        case 'r':                       /* Reset the X.25 virtual circuit */
        case '\022':
            if (network && (nettype == NET_SX25)) (VOID) x25reset(0,0);
            conol("\r\n"); return;
#endif /* SUNX25 */
 
	case 'q':			/* Quit */
	    dohangup = 2; quitnow = 1; active = 0; conol("\r\n"); return;

	case 's':			/* Status */
	    sprintf(temp,
		    "\r\nConnected %s %s", network ? "to" : "through", ttname);
	    conol(temp);
#ifdef SUNX25
            if (network && (nettype == NET_SX25)) {
                sprintf(temp,", Link ID %d, LCN %d",linkid,lcn); conol(temp);
	    }
#endif /* SUNX25 */
	    if (speed >= 0L) {
		sprintf(temp,", speed %ld", speed);
		conoll(temp);
	    } else conoll("");
	    sprintf(temp,
		    "Terminal bytesize: %d, Command bytesize: %d, Parity: ",
		    (cmask  == 0177) ? 7 : 8,
		    (cmdmsk == 0177) ? 7 : 8 );
	    conol(temp);

	    switch (parity) {
	      case  0:  conoll("none");  break;
	      case 'e': conoll("even");  break;
	      case 'o': conoll("odd");   break;
	      case 's': conoll("space"); break;
	      case 'm': conoll("mark");  break;
	    }
	    sprintf(temp,"Terminal echo: %s", duplex ? "local" : "remote");
	    conoll(temp);
	    if (seslog) {
		conol("Logging to: "); conoll(sesfil);
            }
	    if (!network) shomdm();
	    return;

	case '?':			/* Help */
	    c = hconne(); continue;

	case '0':			/* Send a null */
	    c = '\0'; d = dopar((CHAR) c); ttoc((char) d); return;

#ifndef NOPUSH
	case 'z': case '\032':		/* Suspend */
	    stptrap(0); return;

	case '@':			/* Start inferior command processor */
	case '!':
	    conres();			/* Put console back to normal */
	    zshcmd("");			/* Fork a shell. */
	    if (conbin((char)escape) < 0) {
		printf("Error resuming CONNECT session\n");
		active = 0;
	    }
	    return;
#endif /* NOPUSH */

	case SP:			/* Space, ignore */
	    return;

	default:			/* Other */
	    if (c == CMDQ) {		/* Backslash escape */
		int x;
		ecbp = ecbuf;
		*ecbp++ = c;
		while (((c = (CONGKS() & cmdmsk)) != '\r') && (c != '\n'))
		  *ecbp++ = c;
		*ecbp = NUL; ecbp = ecbuf;
		x = xxesc(&ecbp);	/* Interpret it */
		if (x >= 0) {		/* No key mapping here */
		    c = dopar((CHAR) x);
		    ttoc((char) c);
		    return;
		} else {		/* Invalid backslash code. */
		    conoc(BEL);
		    return;
		}
	    }
	    conoc(BEL); return; 	/* Invalid esc arg, beep */
    	}
    }
}

static
VOID
#ifdef CK_ANSIC
logchar(char c)
#else
logchar(c) char c;
#endif /* CK_ANSIC */
/* logchar */ {			/* Log character c to session log */
    if (seslog) 
      if ((sessft != 0) ||
	  (c != '\r' &&
	   c != '\0' &&
	   c != XON &&
	   c != XOFF))
	if (zchout(ZSFILE,c) < 0) {
	    conoll("");
	    conoll("ERROR WRITING SESSION LOG, LOG CLOSED!");
	    seslog = 0;
	}
}
#endif /* NOLOCAL */
