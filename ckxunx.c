char *ckxv = "Unix tty I/O, 4.2(016), 5 Mar 85";

/* C-Kermit interrupt, terminal control & i/o functions for Unix systems */

/* F. da Cruz, Columbia University Center for Computing Activities */

#ifdef BSD4
char *ckxsys = " 4.2 BSD";
#endif

#ifdef PROVX1
char *ckxsys = " Pro-3xx Venix 1.0";
#endif

/* Tower support contributed by John Bray, Auburn University */
#ifdef TOWER1
char *ckxsys = " NCR Tower 1632, OS 1.02";
#endif

/* Sys III/V, Xenix, PC/IX support by Herm Fischer, Litton Data Systems */
#ifdef UXIII
#ifdef XENIX
char *ckxsys = " Xenix/286";
#else
#ifdef PCIX
char *ckxsys = " PC/IX";
#else
#ifdef ISIII
char *ckxsys = " Interactive Systems Corp System III";
#else
char *ckxsys = " AT&T System III/System V";
#endif
#endif
#endif
#endif

/*
 Note - KERLD is the Berkeley Unix Berknet line driver, modified to pass
 through all 8  bits, and to allow an arbitrary break character to be set.
 Don't define this symbol unless you have made this modification to your
 4.2BSD kernel!
*/
#ifdef BSD4
/* #define KERLD */  /* <-- note, commented out */
#endif

/*
 Variables available to outside world:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console, 1 if external line.
   dfprty -- Default parity
   dfflow -- Default flow control
   ckxech -- Flag for who echoes console typein:
     1 - The program (system echo is turned off)
     0 - The system (or front end, or terminal).
   functions that want to do their own echoing should check this flag
   before doing so.

   flfnam -- Name of lock file, including its path, e.g.,
		"/usr/spool/uucp/LCK..cul0" or "/etc/locks/tty77"
   hasLock -- Flag set if this kermit established a uucp lock.
   inbufc -- number of tty line rawmode unread characters 
		(system III/V unixes)
   backgrd -- Flag indicating program executing in background ( & on 
		end of shell command). Used to ignore INT and QUIT signals.

 Functions for assigned communication line (either external or console tty):

   ttopen(ttname,local,mdmtyp) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttpkt(speed,flow)       -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
				or in DIALING or CONNECTED modem control state.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
   myread()		   -- System 3 raw mode bulk buffer read, gives
			   --   subsequent chars one at a time and simulates
			   --   FIONREAD!
   myunrd(c)		   -- Places c back in buffer to be read (one only)
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.

   ttlock(ttname)	   -- Lock against uucp collisions (Sys III)
   ttunlck()		   -- Unlock "       "     "
   look4lk(ttname)	   -- Check if a lock file exists
*/

/*
Functions for console terminal:

   congm()   -- Get console terminal modes.
   concb(esc) -- Put the console in single-character wakeup mode with no echo.
   conbin(esc) -- Put the console in binary (raw) mode.
   conres()  -- Restore the console to mode obtained by congm().
   conoc(c)  -- Unbuffered output, one character to console.
   conol(s)  -- Unbuffered output, null-terminated string to the console.
   conola(s) -- Unbuffered output, array of strings to the console.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console (bsd 4.2).
		Check if escape char (^\) typed at console (System III/V).
   coninc(timo)  -- Timed get a character from the console.
   conint()  -- Enable terminal interrupts on the console if not background.
   connoi()  -- Disable terminal interrupts on the console if not background.

Time functions

   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
*/

/* Includes */

#include <stdio.h>			/* Unix Standard i/o */
#include <signal.h>			/* Interrupts */
#include <setjmp.h>			/* Longjumps */

#ifdef UXIII
#include <sys/types.h>
#endif

#ifndef PROVX1
#include <sys/file.h>			/* File information */
#endif

#ifndef DIRSIZ
#ifdef MAXNAMLEN
#define DIRSIZ MAXNAMLEN
#else
#define DIRSIZ 14
#endif
#endif

#ifdef UXIII
#include <termio.h>
#include <sys/ioctl.h>
#include <fcntl.h>			/* directory reading for locking */
#include <sys/dir.h>
#include <errno.h>			/* error numbers for system returns */
extern int errno;			/* system call error return */
#endif

#ifndef UXIII
#include <sgtty.h>			/* Set/Get tty modes */
#ifndef PROVX1
#include <sys/time.h>			/* Clock info (for break generation) */
#endif
#endif

#ifdef TOWER1
#include <sys/timeb.h>			/* Clock info for NCR Tower */
#endif

#include "ckdebu.h"			/* Formats for debug() */

/* Declarations */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

#ifdef PROVX1
    char *dftty = "/dev/com1";
    int dfloc = 1;
#else
    char *dftty = "/dev/tty";
    int dfloc = 0;
#endif

    int dfprty = 0;			/* Parity (0 = none) */
    int dfflow = 1;			/* Xon/Xoff flow control */
    int backgrd = 0;			/* Assume in foreground (no '&' ) */

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

/* Declarations of variables global within this module */

static jmp_buf sjbuf;			/* Longjump buffer */
static int lkf = 0,			/* Line lock flag */
    conif = 0,				/* Console interrupts on/off flag */
    cgmf = 0,				/* Flag that console modes saved */
    xlocal = 0,				/* Flag for tty local or remote */
    ttyfd = -1;				/* TTY file descriptor */
static char escchr;			/* Escape or attn character */

#ifdef KERLD
    static int kerld = 1;		/* Special Kermit line discipline... */
    struct tchars oldc, newc;		/* Special characters */
    int ld = NETLDISC;			/* Special Kermit line discipline */
    int oldld;				/* Old discipline */
#else
    static int kerld = 0;		/* for 4.2BSD only, */
#endif

#ifdef BSD4
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#endif

#ifdef TOWER1
static long clock;			/* For getting time from sys/time.h */
static struct timeb ftp;		/* And from sys/timeb.h */
#endif

#ifdef UXIII
  static struct termio 			/* sgtty info... */
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */
#else
  static struct sgttyb 			/* sgtty info... */
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */
#endif

static char flfnam[80];			/* uucp lock file path name */
static int hasLock = 0;			/* =1 if this kermit locked uucp */
static int inbufc = 0;			/* stuff for efficient SIII raw line */
static int ungotn = -1;			/* pushback to unread character */
static int conesc = 0;			/* set to 1 if esc char (^\) typed */

static int ttlock();			/* definition of ttlock subprocedure */
static int ttunlck();			/*   unlock subprocedure */

/*  T T O P E N  --  Open a tty for exclusive access.  */

/*  Returns 0 on success, -1 on failure.  */

ttopen(ttname,lcl,modem) char *ttname; int lcl, modem; {

    if (ttyfd > -1) return(0);		/* If already open, ignore this call */
    xlocal = lcl;			/* Make available to other functions */
#ifndef UXIII
    ttyfd = open(ttname,2);		/* Open a tty for read/write */
#else
    /* if modem connection, don't wait for carrier */
    ttyfd = open(ttname,O_RDWR | (modem ? O_NDELAY : 0) );
#endif
    if (ttyfd < 0) return(-1);
    lkf = 0;

#ifndef PROVX1
    if (xlocal) {
#ifdef BSD4
    	if ((flock(ttyfd,(LOCK_EX|LOCK_NB)) < 0) || (ttlock(ttname) < 0 )) {
#else
#ifdef TOWER1
    	if (ioctl(ttyfd,TIOCEXCL, NULL) < 0) {
#else
    	if (ttlock(ttname) < 0) {
#endif
#endif
    	fprintf(stderr,"Sorry - Exclusive access to %s was denied\n",ttname);
	return(-1);			/* Can't open if already locked */
    	} else lkf = 1;
    }
#endif

#ifndef UXIII
    gtty(ttyfd,&ttold);			/* Get sgtty info */
    gtty(ttyfd,&ttraw);			/* And a copy of it for packets*/
    gtty(ttyfd,&tttvt);			/* And one for virtual tty service */
#else
    ioctl(ttyfd,TCGETA,&ttold);		/* Same deal for Sys III, Sys V */
    ioctl(ttyfd,TCGETA,&ttraw);
    ioctl(ttyfd,TCGETA,&tttvt);
#endif
    debug(F101,"ttopen, ttyfd","",ttyfd);
    debug(F111," lock file",flfnam,lkf);
    return(0);
}

/*  T T C L O S  --  Close the TTY, releasing any lock.  */

ttclos() {
    if (ttyfd < 0) return(0);		/* Wasn't open. */
#ifndef PROVX1
#ifdef BSD4
    if (lkf) flock(ttyfd,LOCK_UN);	/* Unlock it first. */
#endif
    if (xlocal) ttunlck();
#endif
    ttres();				/* Reset modes. */
    close(ttyfd);			/* Close it. */
    ttyfd = -1;				/* Mark it as closed. */
    return(0);
}


/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {				/* Restore the tty to normal. */
    if (ttyfd < 0) return(-1);		/* Not open. */
    sleep(1);				/* Wait for pending i/o to finish. */
#ifdef KERLD
    if (kerld) ioctl(ttyfd,TIOCSETD,&oldld); /* Restore old line discipline. */
#endif
#ifdef UXIII
    if (ioctl(ttyfd,TCSETA,&ttold) < 0) return(-1); /* restore termio stuff */
#else
    debug(F101,"ttres, ttyfd","",ttyfd);
    if (stty(ttyfd,&ttold) < 0) return(-1); /* Restore sgtty stuff */
#endif
#ifdef KERLD
    if (kerld) ioctl(ttyfd,TIOCSETC,&oldc); /* Restore old special chars. */
#endif

    return(0);
}

#ifndef PROVX1

/* Exclusive uucp file locking control */
/*
 by H. Fischer, creative non-Bell coding !
*/
static char *
xxlast(s,c) char *s; char c; {		/* Equivalent to strrchr() */
    int i;
    for (i = strlen(s); i > 0; i--)
    	if ( s[i-1] == c ) return( s + (i - 1) );
    return(NULL);	    
}
static
look4lk(ttname) char *ttname; {
    extern char *strcat(), *strcpy();
    char *device, *devname;
    char lockfil[DIRSIZ+1];

#ifdef ISIII
    char *lockdir = "/etc/locks";
#else
    char *lockdir = "/usr/spool/uucp";
#endif

    device = ( (devname=xxlast(ttname,'/')) != NULL ? devname+1 : ttname);

#ifdef ISIII
    (void) strcpy( lockfil, device );
#else
    (void) strcat( strcpy( lockfil, "LCK.." ), device );
#endif

    if (access( lockdir, 04 ) < 0) {	/* read access denied on lock dir */
	fprintf(stderr,"Warning, read access to lock directory denied\n");
	return( 1 );			/* cannot check or set lock file */
	}
	
    (void) strcat(strcat(strcpy(flfnam,lockdir),"/"), lockfil);
    debug(F110,"look4lk",flfnam,0);

    if ( ! access( flfnam, 00 ) ) {	/* print out lock file entry */
	char lckcmd[40] ;
	(void) strcat( strcpy(lckcmd, "ls -l ") , flfnam);
	system(lckcmd);
	return( -1 );
	}
    if ( access( lockdir, 02 ) < 0 ) {	/* lock file cannot be written */
	fprintf(stderr,"Warning, write access to lock directory denied\n");
	return( 1 );
	}
    return( 0 );			/* okay to go ahead and lock */
}

/*  T T L O C K  */

static
ttlock(ttyfd) char *ttyfd; {		/* lock uucp if possible */
    int lck_fil, l4l;
	
    hasLock = 0;			/* not locked yet */
    if ((l4l=look4lk(ttyfd)) < 0) return(-1); /* already locked */
    if (l4l == 1) return (0);		/* can't read/write lock directory */
    if ((lck_fil=open (flfnam, O_CREAT | O_EXCL)) < 1 ) 
    	return(-1);			/* failed to create */
    close (lck_fil);
    hasLock = 1;			/* now is locked */
    return(0);
}

/*  T T U N L O C K  */

static
ttunlck() {				/* kill uucp lock if possible */
    if (hasLock) unlink( flfnam );
}
#endif



/*  T T P K T  --  Condition the communication line for packets. */
/*		or for modem dialing */

#define DIALING	4		/* flags (via flow) for modem handling */
#define CONNECT 5

/*  If called with speed > -1, also set the speed.  */

/*  Returns 0 on success, -1 on failure.  */

ttpkt(speed,flow) int speed, flow; {
    extern char ttname[];
    int s;
    if (ttyfd < 0) return(-1);		/* Not open. */

#ifdef KERLD
    if (kerld) {
	ioctl(ttyfd,TIOCGETD,&oldld);	/* Get line discipline */
	ioctl(ttyfd,TIOCGETC,&oldc);	/* Get special chars */
	newc = oldc;			/* Copy special chars */
	newc.t_brkc = '\r';		/* Set CR to be break character */
	if(ioctl(ttyfd,TIOCSETC,&newc) < 0) return(-1);
    }
#endif

/* cont'd... */

/* ...ttpkt(), cont'd */


/* Note, KERLD ignores the TANDEM, ECHO, and CRMOD bits */

    s = ttsspd(speed);			/* Check the speed */

#ifndef UXIII
    if (flow == 1) ttraw.sg_flags |= TANDEM; /* Use XON/XOFF if selected */
    if (flow == 0) ttraw.sg_flags &= ~TANDEM;
    ttraw.sg_flags |= RAW;		/* Go into raw mode */
    ttraw.sg_flags &= ~(ECHO|CRMOD);	/* Use CR for break character */
#ifdef TOWER1
    ttraw.sg_flags &= ~ANYP; 		/* Must tell Tower no parityr */
#endif
    if (s > -1) ttraw.sg_ispeed = ttraw.sg_ospeed = s; /* Do the speed */
    if (stty(ttyfd,&ttraw) < 0) return(-1);	/* Set the new modes. */
#endif

#ifdef UXIII
    if (flow == 1) ttraw.c_iflag |= (IXON|IXOFF);
    if (flow == 0) ttraw.c_iflag &= ~(IXON|IXOFF);

    if (flow == DIALING)  ttraw.c_cflag |= CLOCAL|HUPCL;
    if (flow == CONNECT)  ttraw.c_cflag &= ~CLOCAL;

    ttraw.c_lflag &= ~(ICANON|ECHO);
    ttraw.c_lflag |= ISIG;		/* do check for interrupt */
    ttraw.c_iflag |= (BRKINT|IGNPAR);
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC);
    ttraw.c_oflag &= ~(ONLCR|OCRNL|ONLRET);
    ttraw.c_cc[4] = 1;
    ttraw.c_cc[5] = 0;

    if (s > -1) ttraw.c_cflag &= ~CBAUD, ttraw.c_cflag |= s; /* set speed */

    if (ioctl(ttyfd,TCSETA,&ttraw) < 0) return(-1);  /* set new modes . */
    if (flow == DIALING) {
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0 )
		return(-1);
	close( open(ttname,2) );	/* magic to force file open!!! */
	}
#endif

#ifdef KERLD
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
    extern char ttname[];
    int s;
    if (ttyfd < 0) return(-1);		/* Not open. */

    s = ttsspd(speed);			/* Check the speed */

#ifndef UXIII
    if (flow == 1) tttvt.sg_flags |= TANDEM; /* XON/XOFF if selected */
    if (flow == 0) tttvt.sg_flags &= ~TANDEM;
    tttvt.sg_flags |= RAW;		/* Raw mode */
#ifdef TOWER1
    tttvt.sg_flags &= ~(ECHO|ANYP);	/* No echo or system III ??? parity */
#else
    tttvt.sg_flags &= ~ECHO;		/* No echo */
#endif    
    if (s > -1) tttvt.sg_ispeed = tttvt.sg_ospeed = s; /* Do the speed */

/* NOTE-- bsd code needs clocal and o_Ndelay stuff here */
    return(stty(ttyfd,&tttvt));
#else
    if (flow == 1) tttvt.c_iflag |= (IXON|IXOFF);
    if (flow == 0) tttvt.c_iflag &= ~(IXON|IXOFF);

    if (flow == DIALING)  tttvt.c_cflag |= CLOCAL|HUPCL;
    if (flow == CONNECT)  tttvt.c_cflag &= ~CLOCAL;

    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
    tttvt.c_iflag |= (IGNBRK|BRKINT|IGNPAR);
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC);
    tttvt.c_oflag &= ~(ONLCR|OCRNL|ONLRET);
    tttvt.c_cc[4] = 1;
    tttvt.c_cc[5] = 0;

    if (s > -1) tttvt.c_cflag &= ~CBAUD, tttvt.c_cflag |= s; /* set speed */

    if (ioctl(ttyfd,TCSETA,&tttvt) < 0) return(-1);  /* set new modes . */
    if (flow == DIALING) {
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0 )
		return(-1);
	close( open(ttname,2) );	/* magic to force file open!!! */
	}
    return(0);
#endif
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

#ifndef PROVX1
    long n;
#endif
    if (ttyfd < 0) return(-1);		/* Not open. */

#ifdef BSD4
#ifdef TOWER1
    n = FREAD;				/* Specify read queue */
    if (ioctl(ttyfd,TIOCFLUSH,&n) < 0) perror("flush failed");
#endif
#endif

#ifdef UXIII
    inbufc = 0;
    ungotn = -1;
    if (ioctl(ttyfd,TCFLSH,0) < 0) perror("flush failed");
#endif

    return(0);
}

/* Interrupt Functions */


/* Timeout handler for communication line input functions */

timerh() {
    longjmp(sjbuf,1);
}

 
/* Set up terminal interrupts on console terminal */

#ifdef UXIII
esctrp() {				/* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);		/* ignore until trapped */
}
#endif


/*  C O N I N T  --  Console Interrupt setter  */

conint(f) int (*f)(); {			/* Set an interrupt trap. */

    if (backgrd) return;		/* must ignore signals in bkgrd */

#ifdef UXIII
    signal(SIGQUIT,esctrp);	/* console escape in pkt modes */
    if (conesc) {			/* clear out pending escapes */
	conesc = 0;
    }
#endif

    if (conif) return;			/* Nothing to do if already on. */

/* check if invoked in background -- if so signals set to be ignored */

    if (signal(SIGINT,SIG_IGN) == SIG_IGN) {
	backgrd = 1;			/*   means running in background */
#ifdef UXIII
	signal(SIGQUIT,SIG_IGN);	/*   must leave signals ignored */
#endif
	return;
    }
    signal(SIGINT,f);			/* Function to trap to. */
    conif = 1;				/* Flag console interrupts on. */
}


/*  C O N N O I  --  Reset console terminal interrupts */

connoi() {				/* Console-no-interrupts */

    if (backgrd) return;		/* must ignore signals in bkgrd */

    signal(SIGINT,SIG_DFL);
    conif = 0;
}

/*  myread() -- System III raw read buffer to block input up */

myread() {			/* return character or -1 if disconnected */

    static int inbuf_item;
    static CHAR inbuf[257];
    CHAR readit;
    
    if (ungotn >= 0) readit = ungotn;
    else {
        if (inbufc > 0)
	    readit = inbuf[++inbuf_item];
    	        else {
	    if ((inbufc=read(ttyfd,inbuf,256)) == 0 ) return(-1);
	    readit = inbuf[inbuf_item=0];
	    }
        inbufc--;	
        }
    ungotn = -1;
    return(readit );
    }

myunrd(ch) CHAR ch; {			/* push back up to one character */
    ungotn = ch;
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

ttchk() {
#ifndef PROVX1
    int n, x;
#ifdef BSD4
    x = ioctl(ttyfd, FIONREAD, &n);
    return((x < 0) ? 0 : n);
#else
    return(inbufc + (ungotn >= 0) );	
#endif
#else
    return(0);
#endif
}


/*  T T X I N  --  Get n characters from tty input buffer  */

ttxin(n,buf) int n; char *buf; {
    int x;
#ifndef UXIII
    x = read(ttyfd,buf,n);
#else
    for( x=0; x<n; buf[x++]=myread() );
#endif
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
    int x, y, c;
    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed. */
#ifndef UXIII
	x = read(ttyfd,dest,max);	/* Try to read. */
#else
	for (x = c = 0; (x < max) && (c != eol); x++) {
	     c = myread(); 
	     dest[x] = c;
	}
#endif
	return(x);			/* Return the count. */
    }
    signal(SIGALRM,timerh);		/* Timed, set up timeout action. */
    alarm(timo);			/* Set the timer. */
    if (setjmp(sjbuf)) x = -1;		/* Do this if timer went off. */
    else if (kerld) {
	x = read(ttyfd,dest,max);	/* Try to read. */
    } else {
	for (x = c = y = 0; (x < max) && (c != eol); x++) {
#ifndef UXIII
	     while ( !(y = read(ttyfd,&c,1))) ; /* skip null reads */
	     if (y < 0) return(y);
#else
	     c = myread(); 
#endif
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
    int n;
    CHAR ch;

    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed. */
#ifndef UXIII
	while ( !(n = read(ttyfd,&ch,1)) ) ; /* Wait for a character. */
	return( (n > 0) ? (ch & 0377) : n );
#else
	/* comm line failure returns -1 thru myread, so don't &= 0377 */
	return( myread() );
#endif
    }

    signal(SIGALRM,timerh);		/* Timed, set up timer. */
    alarm(timo);
    if (setjmp(sjbuf)) n = -1;
    else {
#ifndef UXIII
    n = read(ttyfd,&ch,1);		/* Read a character. */
#else
    ch = myread();
    n = 1;
#endif
    }
    alarm(0);				/* Turn off timer, */
    signal(SIGALRM,SIG_DFL);		/* and interrupt. */
    if (n > 0) return(ch & 0377); else return(n);  /* Return char or -1. */
}

/*  T T S N D B  --  Send a BREAK signal  */

ttsndb() {
    int x;

    if (ttyfd < 0) return(-1);		/* Not open. */

#ifdef PROVX1
/*** insert code to set speed to 50 baud and send 2-3 nulls ***/
    return(0);
#else
#ifdef UXIII
    if (ioctl(ttyfd,TCSBRK,(char *)0) < 0) {	/* Turn on BREAK */
    	conol("Can't send BREAK");
	return(-1);
    }
    return(0);
#else
#ifdef BSD4
    if (ioctl(ttyfd,TIOCSBRK,(char *)0) < 0) {	/* Turn on BREAK */
    	conol("Can't send BREAK");
	return(-1);
    }
    x = msleep(275);			/* Sleep for so many milliseconds */
    if (ioctl(ttyfd,TIOCCBRK,(char *)0) < 0) {	/* Turn off BREAK */
	conol("BREAK stuck!!!");
	doexit(1);			/* Get out, closing the line. */
					/*   with exit status = 1 */
    }
    return(x);
#endif
#endif
#endif
}

/*  M S L E E P  --  Millisecond version of sleep().  */

/*
 Intended only for small intervals.  For big ones, just use sleep().
*/

msleep(m) int m; {

#ifdef PROVX1
    sleep(1+m/1000.0);
    return(0);
#endif

#ifdef BSD4
    int t1, t3, t4;
    if (gettimeofday(&tv, &tz) < 0) return(-1); /* Get current time. */
    t1 = tv.tv_sec;			/* Seconds */

if (0) {				/* Old way (works) */
    while (1) {
	gettimeofday(&tv, &tz);
	t3 = tv.tv_sec - t1;
	t4 = (tv.tv_usec + 1000000 * t3) / 1000;
	if (t4 > m) return(t4);
    }
} else {				/* New way (also works) */
    tv.tv_sec = 0;
    tv.tv_usec = m * 1000;
    return(select( 0, (int *)0, (int *)0, (int *)0, &tv) );
}
#endif

#ifdef TOWER1
    if (ftime(&ftp) < 0) return(-1);		/* Get current time. */
    t1 = ((ftp.time & 0xff) * 1000) + ftp.millitm;
    while (1) {
	ftime(&ftp);				/* new time */
	t3 = (((ftp.time & 0xff) * 1000) + ftp.millitm) - t1;
	if (t3 > m) return (t3);
    }
#endif
}

/*  Z T I M E  --  Return date/time string  */

ztime(s) char **s; {

#ifdef UXIII
    extern long time();			/* Sys III/V way to do it */
    char *ctime();
    long clock_storage;

    clock_storage = time( (long *) 0 );
    *s = ctime( &clock_storage );
#endif

#ifdef PROVX1
    int utime[2];			/* Venix way */
    time(utime);
    *s = ctime(utime);
#endif

#ifdef BSD4
    char *asctime();			/* Berkeley way */
    struct tm *localtime();
    struct tm *tp;

    gettimeofday(&tv, &tz);
    time(&tv.tv_sec);
    tp = localtime(&tv.tv_sec);
    *s = asctime(tp);
#endif

#ifdef TOWER1
    char *asctime();			/* Tower way */
    struct tm *localtime();
    struct tm *tp;

    time(&clock);
    tp = localtime(&clock);
    *s = asctime(tp);
#endif
}

/*  C O N G M  --  Get console terminal modes.  */

/*
 Saves current console mode, and establishes variables for switching between 
 current (presumably normal) mode and other modes.
*/

congm() {
#ifndef UXIII
     gtty(0,&ccold);			/* Structure for restoring */
     gtty(0,&cccbrk);			/* For setting CBREAK mode */
     gtty(0,&ccraw);			/* For setting RAW mode */
#else
     ioctl(0,TCGETA,&ccold);
     ioctl(0,TCGETA,&cccbrk);
     ioctl(0,TCGETA,&ccraw);
#endif
     cgmf = 1;				/* Flag that we got them. */
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

concb(esc) char esc; {
    int x;
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifndef UXIII
    cccbrk.sg_flags |= CBREAK;		/* Set to character wakeup, */
    cccbrk.sg_flags &= ~ECHO;		/* no echo. */
    x = stty(0,&cccbrk);
#else
    cccbrk.c_lflag &= ~(ICANON|ECHO);
    cccbrk.c_cc[0] = 003;		/* interrupt char is control-c */
    cccbrk.c_cc[1] = escchr;		/* escape during packet modes */
    cccbrk.c_cc[4] = 1;
    cccbrk.c_cc[5] = 1;
    x = ioctl(0,TCSETA,&cccbrk);  	/* set new modes . */
#endif
    if (x > -1) setbuf(stdout,NULL);	/* Make console unbuffered. */
    return(x);
}

/*  C O N B I N  --  Put console in binary mode  */


/*  Returns 0 if ok, -1 if not  */

conbin(esc) char esc; {
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifndef UXIII
    ccraw.sg_flags |= (RAW|TANDEM);   	/* Set rawmode, XON/XOFF */
    ccraw.sg_flags &= ~(ECHO|CRMOD);  	/* Set char wakeup, no echo */
    return(stty(0,&ccraw));
#else
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
    ccraw.c_iflag |= (BRKINT|IGNPAR);
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF);
    ccraw.c_oflag &= ~(ONLCR|OCRNL|ONLRET);
    ccraw.c_cc[4] = 1;
    ccraw.c_cc[5] = 1;
    return(ioctl(0,TCSETA,&ccraw) );  	/* set new modes . */
#endif
}


/*  C O N R E S  --  Restore the console terminal  */

conres() {
    if (cgmf == 0) return(0);		/* Don't do anything if modes */
    sleep(1);				/*  not known! */
    ckxech = 0;				/* System should echo chars */
#ifndef UXIII
    return(stty(0,&ccold));		/* Restore controlling tty */
#else
    return(ioctl(0,TCSETA,&ccold));
#endif
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

/*  C O N O L A  --  Write an array of lines to the console terminal */

conola(s) char *s[]; {
    int i;
    for (i=0 ; *s[i] ; i++) conol(s[i]);
}

/*  C O N O L L  --  Output a string followed by CRLF  */

conoll(s) char *s; {
    conol(s);
    write(1,"\r\n",2);
}


/*  C O N C H K  --  Check if characters available at console  */

conchk() {
#ifdef PROVX1
    return(0);
#else
    int n, x;
#ifndef UXIII
    x = ioctl(0, FIONREAD, &n);
    return((x < 0) ? 0 : n);
#else
    if (conesc) {			/* Escape typed */
	conesc = 0;
	signal(SIGQUIT,esctrp);		/* restore escape */
	return(1);
    }
    return(0);
#endif
#endif
}

/*  C O N I N C  --  Get a character from the console  */

coninc(timo) int timo; {
    int n = 0; char ch;
    if (timo <= 0 ) {			/* untimed */
	n = read(0, &ch, 1);		/* Read a character. */
	ch &= 0377;
	if (n > 0) return(ch); 		/* Return the char if read */
	else 
#ifdef UXIII
	    if (n < 0 && errno == EINTR) /* if read was interrupted by QUIT */
		return(escchr);		 /* user entered escape character */
	    else		    /* couldnt be ^c, sigint never returns */
#endif
		return(-1);  		/* Return the char, or -1. */
	}
    signal(SIGALRM,timerh);		/* timed, set up timer */
    alarm(timo);
    if (setjmp(sjbuf)) n = -2;
    else {
	n = read(0, &ch, 1);
	alarm(0);
	signal(SIGALRM,SIG_DFL);	/* stop timing, we got our character */
	ch &= 0377;
    }
    if (n > 0) return(ch);  
    else
#ifdef UXIII
        if (n == -1 && errno == EINTR)  /* If read interrupted by QUIT, */
	    return(escchr);		/* user entered escape character, */
        else		    		/* can't be ^c, sigint never returns */
#endif
	return(-1);
}
