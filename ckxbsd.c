char *ckxv = "4.2BSD Terminal I/O, 4.0(011), 30 Jan 85";

/* Interrupt, terminal control & i/o functions for 4.2BSD */

/*
 Note - KERLD is the Berknet line driver, modified to pass through all 8
 bits, and to allow an arbitrary break character to be set.  Don't define
 this symbol unless you have made this modification to your 4.2BSD kernel!
*/ 
#define KERLD 0

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

   ttopen(ttname)          -- Open the named tty for exclusive access.
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


#include <stdio.h>			/* Unix Standard i/o */
#include <sgtty.h>			/* Set/Get tty modes */
#include <signal.h>			/* Interrupts */
#include <setjmp.h>			/* Longjumps */
#include <sys/file.h>			/* File information */
#include <sys/time.h>			/* Clock info */
#include "ckermi.h"			/* Formats for debug() */
#include "ckdebu.h"			/* Formats for debug() */


/* dftty is the device name of the default device for file transfer */

char *dftty = "/dev/tty";


/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

int dfloc = 0;

/* Other defaults */

int dfprty = 0;				/* No parity */
int dfflow = 1;				/* Xon/Xoff flow control */

/* ckxech is 0 if system normally echoes console characters, 1 otherwise */

int ckxech = 0;


/* Declarations of variables global to this module */

jmp_buf sjbuf;				/* Longjump buffer */

static int lkf = 0,			/* Line lock flag */
    conif = 0,				/* Console interrupts on/off flag */
    cgmf = 0,				/* Flag that console modes saved */
    kerld = KERLD,			/* Flag for using line discipline */
    ttyfd = -1;				/* TTY file descriptor */

static struct timeval tv;		/* For getting time, from sys/time.h */
static struct timezone tz;

static struct sgttyb 			/* sgtty info... */
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */

#if KERLD
  struct tchars oldc, newc;		/* Special characters */
  int ld = NETLDISC;			/* Special Kermit line discipline */
  int oldld;				/* Old discipline */
#endif


/*  T T O P E N  --  Open a tty for exclusive access.  */

/*  Returns 0 on success, -1 on failure.  */

/***
 Do something better about exclusive access later, like the uucp
 lock file hair...
***/

ttopen(ttname) char *ttname; {

    if (ttyfd > -1) ttclos();		/* If old one open, close it. */

    ttyfd = open(ttname,2);		/* Open a tty for read/write */
    if (ttyfd < 0) return(-1);
    lkf = 0;

    if (flock(ttyfd,(LOCK_EX|LOCK_NB)) < 0) {
    	fprintf(stderr,"Warning - Access to %s not exclusive\n",ttname);
    } else lkf = 1;

    gtty(ttyfd,&ttold);			/* Get sgtty info */
    gtty(ttyfd,&ttraw);			/* And a copy of it for packets*/
    gtty(ttyfd,&tttvt);			/* And one for virtual tty service */
    return(0);
}


/*  T T C L O S  --  Close the TTY, releasing any lock.  */

ttclos() {
    if (ttyfd < 0) return(0);		/* Wasn't open. */
    if (lkf) flock(ttyfd,LOCK_UN);	/* Unlock it first. */
    ttres();				/* Reset modes. */
    close(ttyfd);			/* Close it. */
    ttyfd = -1;				/* Mark it as closed. */
    return(0);
}


/*  T T P K T  --  Condition the communication line for packets. */

/*  If called with speed > -1, also set the speed.  */

/*  Returns 0 on success, -1 on failure.  */

ttpkt(speed,flow) int speed, flow; {
    int s;
    if (ttyfd < 0) return(-1);		/* Not open. */

#if KERLD
    if (kerld) {
	ioctl(ttyfd,TIOCGETD,&oldld);	/* Get line discipline */
	ioctl(ttyfd,TIOCGETC,&oldc);	/* Get special chars */
	newc = oldc;			/* Copy special chars */
	newc.t_brkc = '\r';		/* Set CR to be break character */
	if(ioctl(ttyfd,TIOCSETC,&newc) < 0) return(-1);
    }
#endif

/* Note, KERLD ignores the TANDEM, ECHO, and CRMOD bits */

    if (flow == 1) ttraw.sg_flags |= TANDEM; /* XON/XOFF if selected */
    if (flow == 0) ttraw.sg_flags &= ~TANDEM;
    ttraw.sg_flags |= RAW;		/* Go into raw mode */
    ttraw.sg_flags &= ~(ECHO|CRMOD);	/* Use CR for break character */
    
    s = ttsspd(speed);			/* Check the speed */
    if (s > -1) ttraw.sg_ispeed = ttraw.sg_ospeed = s; /* Do the speed */

    if (stty(ttyfd,&ttraw) < 0) return(-1);	/* Set the new modes. */

#if KERLD
    if (kerld) {
	if (ioctl(ttyfd,TIOCSETD,&ld) < 0)
	    return(-1); /* Set line discpline. */
    }
#endif

    ttflui();				/* Flush any pending input */
    return(0);
}


/*  T T V T -- Condition communication line for use as virtual terminal  */

ttvt(speed,flow) int speed, flow; {
    int s;
    if (ttyfd < 0) return(-1);		/* Not open. */

    if (flow == 1) tttvt.sg_flags |= TANDEM; /* XON/XOFF if selected */
    if (flow == 0) tttvt.sg_flags &= ~TANDEM;
    tttvt.sg_flags |= RAW;		/* Raw mode */
    tttvt.sg_flags &= ~ECHO;		/* No echo */
    
    s = ttsspd(speed);			/* Check the speed */
    if (s > -1) tttvt.sg_ispeed = tttvt.sg_ospeed = s; /* Do the speed */

    return(stty(ttyfd,&tttvt));
}


/*  T T S S P D  --  Return the internal baud rate code for 'speed'.  */

ttsspd(speed) {
    int s, spdok;

    if (speed < 0) return(-1);
	spdok = 1;			/* Assume arg ok */
	switch (speed) {
	    case 0:    s = B0;    break;	/* Just the common ones. */
	    case 110:  s = B110;  break;	/* The others from ttydev.h */
	    case 150:  s = B150;  break;	/* could also be included if */
	    case 300:  s = B300;  break;	/* necessary... */
	    case 600:  s = B600;  break;
	    case 1200: s = B1200; break;
	    case 1800: s = B1800; break;
	    case 2400: s = B2400; break;
	    case 4800: s = B4800; break;
	    case 9600: s = B9600; break;
	    default:
	    	spdok = 0;
		fprintf(stderr,"Unsupported line speed - %d\n",speed);
		fprintf(stderr,"Current speed not changed\n");
		break;
	}	    
	if (spdok) return(s); else return(-1);
 }


/*  T T F L U I  --  Flush tty input buffer */

ttflui() {
    long int n;
    if (ttyfd < 0) return(-1);		/* Not open. */
    n = FREAD;				/* What's this??? */
    if (ioctl(ttyfd,TIOCFLUSH,&n) < 0) perror("flush failed");
    return(0);
}



/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {				/* Restore the tty to normal. */
    if (ttyfd < 0) return(-1);		/* Not open. */
    sleep(1);				/* Wait for pending i/o to finish. */
#if KERLD
    if (kerld) ioctl(ttyfd,TIOCSETD,&oldld); /* Restore old line discipline. */
#endif
    if (stty(ttyfd,&ttold) < 0) return(-1); /* Restore sgtty stuff */
#if KERLD
    if (kerld) ioctl(ttyfd,TIOCSETC,&oldc); /* Restore old special chars. */
#endif
    return(0);
}


/* Interrupt Functions */


/* Timeout handler for communication line input functions */

timerh() {
    longjmp(sjbuf,1);
}

 
/* Set up terminal interrupts on console terminal */


conint(f) int (*f)(); {			/* Set an interrupt trap. */
    if (conif) return;			/* Nothing to do if already on. */
    signal(SIGINT,f);			/* Function to trap to. */
    conif = 1;
}


/* Reset console terminal interrupts */

connoi() {
    signal(SIGINT,SIG_DFL);
    conif = 0;
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

ttchk() {
    int n, x;
    x = ioctl(ttyfd, FIONREAD, &n);
    return((x < 0) ? 0 : n);
}


/*  T T X I N  --  Get n characters from tty input buffer  */

ttxin(n,buf) int n; char *buf; {
    int x;
    x = read(ttyfd,buf,&n);
    if (x > 0) buf[x] = '\0';
    return(x);
}


/*  T T O L  --  Similar to "ttinl", but for writing.  */

ttol(s,n) int n; char *s; {
    int x;
    if (ttyfd < 0) return(-1);		/* Not open. */
    x = write(ttyfd,s,n);
    debug(F111,"ttol",s,n);
    if (x < 0) debug(F101,"ttol failed","",x);
    return(x);
}


/*  T T O C  --  Output a character to the communication line  */

ttoc(c) char c; {
    if (ttyfd < 0) return(-1);		/* Not open. */
    return(write(ttyfd,&c,1));
}


/*  T T I N L  --  Read a record (up to break character) from comm line.  */
/*
  If no break character encountered within "max", return "max" characters,
  with disposition of any remaining characters undefined.  Otherwise, return
  the characters that were read, including the break character, in "dest" and
  the number of characters read as the value of function, or 0 upon end of
  file, or -1 if an error occurred.  Times out & returns error if not completed
  within "timo" seconds.
*/

ttinl(dest,max,timo,eol) int max,timo; char *dest; {
    int x, c;
    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed. */
	x = read(ttyfd,dest,max);	/* Try to read. */
	return(x);			/* Return the count. */
    }
    signal(SIGALRM,timerh);		/* Timed, set up timeout action. */
    alarm(timo);			/* Set the timer. */
    if (setjmp(sjbuf)) return(-1);	/* Do this if timer went off. */
    if (kerld) {
	x = read(ttyfd,dest,max);	/* Try to read. */
    } else {
	for (x = c = 0; (x < max) && (c != eol); x++) {
	     read(ttyfd,&c,1);
	     dest[x] = c;
	}
	x++;
    }
    alarm(0);				/* Success, turn off timer, */
    signal(SIGALRM,SIG_DFL);		/* and associated interrupt. */
    return(x);				/* Return the count. */
}


/*  T T I N C --  Read a character from the communication line  */

ttinc(timo) int timo; {
    int n, ch;

    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed. */
	n = read(ttyfd,&ch,1);		/* Wait for a character. */
	ch &= 0377;
	return( (n > 0) ? ch : n );
    }

    signal(SIGALRM,timerh);		/* Timed, set up timer. */
    alarm(timo);
    if (setjmp(sjbuf)) return(-1);
    n = read(ttyfd,&ch,1);		/* Read a character. */
    alarm(0);				/* Turn off timer, */
    signal(SIGALRM,SIG_DFL);		/* and interrupt. */
    ch &= 0377;
    if (n > 0) return(ch); else return(n);  /* Return the char, or -1. */
}


/*  T T S N D B  --  Send a BREAK signal  */

ttsndb() {
    int x;

    if (ttyfd < 0) return(-1);		/* Not open. */
    if (ioctl(ttyfd,TIOCSBRK,(char *)0) < 0) {	/* Turn on BREAK */
    	conol("Can't send BREAK");
	return(-1);
    }
    x = msleep(275);			/* Sleep for so many milliseconds */
    if (ioctl(ttyfd,TIOCCBRK,(char *)0) < 0) {	/* Turn off BREAK */
	conol("BREAK stuck!!!");
	doexit();			/* Get out, closing the line. */
    }
    return(x);
}


/*  M S L E E P  --  Millisecond version of sleep().  */

/*
 Intended only for small intervals.  For big ones, just use sleep().
 This is a very expensive way to do interval timing.  It just loops,
 looking at the system clock.  If anyone can figure out a better way,
 e.g. with setitimer(), then do it that way.
*/

msleep(m) int m; {
    int t1, t3, t4;

    if (gettimeofday(&tv, &tz) < 0) return(-1); /* Get current time. */
    t1 = tv.tv_sec;			/* Seconds */

    while (1) {
	gettimeofday(&tv, &tz);
	t3 = tv.tv_sec - t1;
	t4 = (tv.tv_usec + 1000000 * t3) / 1000;
	if (t4 > m) return(t4);
    }
}


/*  Z T I M E  --  Return date/time string  */

ztime(s) char **s; {
    char *asctime();
    struct tm *localtime();
    struct tm *tp;

    gettimeofday(&tv, &tz);
    time(&tv.tv_sec);
    tp = localtime(&tv.tv_sec);
    *s = asctime(tp);
}


/*  C O N G M  --  Get console terminal modes.  */

/*
 Saves current console mode, and establishes variables for switching between 
 current (presumably normal) mode and other modes.
*/

congm() {
     gtty(0,&ccold);			/* Structure for restoring */
     gtty(0,&cccbrk);			/* For setting CBREAK mode */
     gtty(0,&ccraw);			/* For setting RAW mode */
     cgmf = 1;				/* Flag that we got them. */
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

concb() {
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    cccbrk.sg_flags |= CBREAK;		/* Set to character wakeup, */
    cccbrk.sg_flags &= ~ECHO;		/* no echo. */
    ckxech = 1;				/* Program can echo characters */
    return(stty(0,&cccbrk));
}


/*  C O N B I N  --  Put console in binary mode  */

/*  Returns 0 if ok, -1 if not  */

conbin() {
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    ccraw.sg_flags |= (RAW|TANDEM);   	/* Set rawmode, XON/XOFF */
    ccraw.sg_flags &= ~(ECHO|CRMOD);  	/* Set char wakeup, no echo */
    ckxech = 1;				/* Program can echo characters */
    return(stty(0,&ccraw));
}


/*  C O N R E S  --  Restore the console terminal  */

conres() {
    if (cgmf == 0) return(0);		/* Don't do anything if modes */
    sleep(1);				/*  not known! */
    ckxech = 0;				/* System should echo chars */
    return(stty(0,&ccold));		/* Restore controlling tty */
}


/*  C O N O C  --  Output a character to the console terminal  */

conoc(c) char c; {
    write(1,&c,1);
}

/*  C O N X O  --  Write x characters to the console terminal  */

conxo(x,s) char *s; int x; {
    write(1,s,x);
}

/*  C O N O L  --  Write a line to the console terminal  */

conol(s) char *s; {
    int len;
    len = strlen(s);
    write(1,s,len);
}

/*  C O N O L L  --  Output a string followed by CRLF  */

conoll(s) char *s; {
    conol(s);
    write(1,"\r\n",2);
}


/*  C O N C H K  --  Check if characters available at console  */

conchk() {
    int n, x;
    x = ioctl(0, FIONREAD, &n);
    return((x < 0) ? 0 : n);
}


/*  C O N I N C  --  Get a character from the console  */

coninc() {
    int n; char ch;
    n = read(0, &ch, 1);		/* Read a character. */
    ch &= 0377;
    if (n > 0) return(ch); else return(-1);  /* Return the char, or -1. */
}
