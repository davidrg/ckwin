char *ckxv = "OS-9 tty I/O, 07 Apr 89";
 
/*  c k 9 T I O  */
 
/* C-Kermit interrupt, terminal control & i/o functions for os9/68k systems */
 
/*
 Author: Peter Scholz
 Ruhr University Bochum, Department of analytical chemistry
 Federal Republic of Germany, February 1987
 
 Bob Larson (Blarson@ecla.usc.edu)
 Cleanup, fix timeouts, fix connect escape
 Rewrite ttinl for less overhead

 adapted from unix C-Kermit
 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/
 
 
#include <dir.h>   /* Directory */
#include <ctype.h>   /* Character types */
#ifdef NULL
#undef NULL
#endif NULL
#ifndef GREGORIAN
#define GREGORIAN 0
#define JULIAN 1
#endif
#define MIDNIGHT 86400
#include <stdio.h>	/* Standard i/o */
#include <signal.h>	/* Interrupts */
#include <errno.h>
#include <sgstat.h>	/* Set/Get tty modes */
#include <sg_codes.h>	/* setstat/getstat function codes */
#include "ckcdeb.h"	/* Typedefs, formats for debug() */
 
/* Maximum length for the name of a tty device */
 
#ifndef DEVNAMLEN
#define DEVNAMLEN 34		/* leave room for / and terminating \0 */
#endif

typedef struct
 { int date,time;} syst;
 
char *ckxsys = " OS-9/68000";

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
 
   inbufc -- number of tty line rawmode unread characters

   ticks -- ticks left to sleep, used with SIGARB

 Functions for assigned communication line (either external or console tty):
 
   syscleanup()		   -- System dependent shutdown
   sysinit()               -- System dependent program initialization
   ttopen(ttname,local,mdmtyp) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty.
   ttpkt(speed,flow)       -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
    or in DIALING or CONNECTED modem control state.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
   myread()     -- System 3 raw mode bulk buffer read, gives
      --   subsequent chars one at a time and simulates
      --   FIONREAD!
   myunrd(c)     -- Places c back in buffer to be read (one only)
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.
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
   ztime(s) -- Return date/time string
   rtimer() --  Reset timer
   gtimer()  -- Get elapsed time since last call to rtimer()
*/
 
#define SLEEPPERSEC 5			/* sleep intervals per second	*/
#define MINSLEEP (256/SLEEPPERSEC+1)	/* minimum sleep interval, 2 ticks min*/
#define SLEEPINT (0x80000000|MINSLEEP)	/* sleep interval */
 
/* Declarations */
 
extern int errno;   /* System call error return */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */
 
    char *dftty;	/* Remote by default, use normal */
    int dfloc = 0;	/* controlling terminal name. */
 
    int dfprty = 0;	/* Parity (0 = none) */
    int ttprty = 0;	/* current parity */
    int dfflow = 0;	/* Xon/Xoff flow control */
    int backgrd = 0;	/* Assume in foreground (no '&' ) */
 
    int ckxech = 0;	/* 0 if system normally echoes console characters, else 1 */
 
    int ticks;

int ttyfd = -1;    /* TTY file descriptor, also used in ck9con.c */

char myttystr[DEVNAMLEN];

/* Declarations of variables global within this module */
 
static char dfttystr[DEVNAMLEN];
static int
    conif = 0,    /* Console interrupts on/off flag */
    cgmf = 0,    /* Flag that console modes saved */
    xlocal = 0;    /* Flag for tty local or remote */
 
static int kerld = 0;  /* Not selected, no special l.d. */
 
static struct sgbuf    /* sgtty info... */
  ttold, ttraw, tttvt, ttbuf,  /* for communication line */
  ccold, ccraw, cccbrk;  /* and for console */
 
static int ungotn = -1;   /* pushback to unread character */
static int conesc = 0;   /* set to 1 if esc char (^\) typed */
 
static char ttnmsv[DEVNAMLEN];  /* copy of open path for tthang */
static syst tcount;   /* Elapsed time counter */
static char *wday = { "SunMonTueWedThuFriSat" };
static char *month ={ "JanFebMarAprMayJunJulAugSepOctNovDec" };

static int when = MIDNIGHT;		/* used in alarm faking */

#define B50	0
#define B75	1
#define B110	2
#define B134	3
#define B150	4
#define B300	5
#define B600	6
#define B1200	7
#define B1800	8
#define B2000	9
#define B2400  10
#define B3600  11
#define B4800  12
#define B7200  13
#define B9600  14
#define B19200 15

static unsigned short bdrts[] = {
	50, 75, 110, 134, 150, 300, 600, 1200, 
	1800, 2000, 2400, 3600, 4800, 7200, 9600, 19200
};

/* for unix signal emulation */

#define MAXSIG 4
typedef int (*fptr)();
fptr sigtbl[MAXSIG+1];
fptr signal();

syscleanup() {
    return 0;			/* none needed */
}

/*  S Y S I N I T  --  System-dependent program initialization.  */
 
sysinit() {
/* initialize unix signal emulation */
    register int i;
    register char *cp;
    int catch();
    char *getenv();

    for(i=0;i<=MAXSIG;i++)
	sigtbl[i] = SIG_DFL;
    intercept(catch);
    if(isatty(0)) {		/* must disable buffering before io */
	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
    }
    /* get terminal name from enviornment variable PORT,
     * use device of stderr as a backup
     */
    myttystr[0] = '/';
    if((cp = getenv("PORT")) == NULL) {
	if(_gs_devn(2, myttystr+1) < 0) return -1;
    } else strcpy(myttystr+1, cp);
    strcpy(dfttystr, myttystr);
    dftty = &dfttystr[0];
    return 0;
}
 
/*  T T O P E N  --  Open a tty for exclusive access.  */
 
/*  Returns 0 on success, -1 on failure.  */
/*
  If called with lcl < 0, sets value of lcl as follows:
  0: the terminal named by ttname is the job's controlling terminal.
  1: the terminal named by ttname is not the job's controlling terminal.
  But watch out: if a line is already open, or if requested line can't
  be opened, then lcl remains (and is returned as) -1.
*/
ttopen(ttname,lcl,modem) char *ttname; int *lcl, modem; {
 
    char x[DEVNAMLEN];
    char cname[DEVNAMLEN+4];
    extern int speed;
 
    if (ttyfd > -1) return(0);  /* If already open, ignore this call */
    xlocal = *lcl;   /* Make this available to other fns */
    strncpy(ttnmsv,ttname,DEVNAMLEN); /* Open, keep copy of name locally. */
    x[0] = '/';
    _gs_devn(0,&x[1]);
    if (*ttname=='\0' || strcmp(ttname,x)==0) {
	ttyfd = dup(0);
    } else ttyfd = open(ttnmsv,0x43);  /* Try to open for read/write nonshare*/
 
    if (ttyfd < 0) {   /* If couldn't open, fail. */
	perror(ttname);
	return(-1);
    }
 
/* Caller wants us to figure out if line is controlling tty */
 
    debug(F111,"ttopen ok",ttname,*lcl);
    if (*lcl == -1) {
	if (isatty(0)) {		/* if stdin not redirected */
	    strncpy(cname,x,DEVNAMLEN); /* (copy from internal static buf) */
	    debug(F110," ttyname(0)",x,0);
	    _gs_devn(ttyfd,&x[1]);	/* ...with real name of ttname. */
	    xlocal = (strncmp(x,cname,DEVNAMLEN) != 0);
	    debug(F111," ttyname",x,xlocal);
	} else {   /* Else, if stdin redirected... */
/* Just assume local, so "set speed" and similar commands will work */
/* If not really local, how could it work anyway?... */
	    xlocal = 1;
	    debug(F101," redirected stdin","",xlocal);
        }
    }

/* line locking not needed -- os9 has exclusive access flag on open */

/* Got the line, now set the desired value for local. */
 
    if (*lcl < 0) *lcl = xlocal;
 
/* Get tty device settings */
 
    _gs_opt(ttyfd,&ttold);   /* Get sgtty info */
    _gs_opt(ttyfd,&ttraw);   /* And a copy of it for packets*/
    _gs_opt(ttyfd,&tttvt);   /* And one for virtual tty service */
    speed = bdrts[ttold.sg_baud];
    debug(F101,"ttopen, ttyfd","",ttyfd);
    debug(F101," lcl","",*lcl);
    return(0);
}
 
/*  T T C L O S  --  Close the TTY */
 
ttclos() {
    if (ttyfd < 0) return(0);  /* Wasn't open. */
    if (xlocal) {
	if (tthang())   /* Hang up phone line */
	    fprintf(stderr,"Warning, problem hanging up the phone\n");
    }
    ttres();    /* Reset modes. */
/* Relinquish exclusive access if we might have had it... */
    close(ttyfd);   /* Close it. */
    debug (F101,"closed connection, ttyfd","",ttyfd);
    ttyfd = -1;    /* Mark it as closed. */
    return(0);
}
 
/*  T T H A N G -- Hangup phone line */
 
tthang() {
/*    if (ttyfd < 0) return(0);	/* Not open. */
    return 0;			/* not implemented */
}
 
 
/*  T T R E S  --  Restore terminal to "normal" mode.  */
 
ttres() {    /* Restore the tty to normal. */
    if (ttyfd < 0) return(-1);  /* Not open. */
    tsleep(2);    /* Wait for pending i/o to finish. */
    if (_ss_opt(ttyfd,&ttold) < 0) return(-1); /* Restore sgtty stuff */
    return(0);
}
 
/*  T T P K T  --  Condition the communication line for packets. */
/*  or for modem dialing */
 
/*  If called with speed > -1, also set the speed.  */
 
/*  Returns 0 on success, -1 on failure.  */
 
ttpkt(speed,flow,parity)
int speed, flow, parity;
{
    int s;
    if (ttyfd < 0) return(-1);  /* Not open. */
 
    s = ttsspd(speed,ttnmsv);   /* Check the speed */
    ttprty = parity;

    if (xlocal) {
	close(ttyfd);
	ttyfd= open(ttnmsv,0x43); /* magic to force mode change!!! */
	debug (F101,"ttpkt: reopen of line, ttyfd","",ttyfd);
    }
    ttraw.sg_case =
    ttraw.sg_backsp =
    ttraw.sg_delete =
    ttraw.sg_echo = 
    ttraw.sg_alf =
    ttraw.sg_nulls =
    ttraw.sg_pause =
    ttraw.sg_bspch =
    ttraw.sg_dlnch =
    ttraw.sg_eorch =
    ttraw.sg_eofch =
    ttraw.sg_rlnch =
    ttraw.sg_dulnch =
    ttraw.sg_psch =
    ttraw.sg_kbich =
    ttraw.sg_kbach =
    ttraw.sg_bsech = 
    ttraw.sg_tabcr = 0;
    if(s>-1) ttraw.sg_baud = (char)s;
    if (flow==1) {
	ttraw.sg_xon  = 0x11;
	ttraw.sg_xoff = 0x13;
    } else ttraw.sg_xon = ttraw.sg_xoff = 0;
    _ss_opt(ttyfd,&ttraw);   /* set new modes . */
    ttflui();    /* Flush any pending input */
    return(0);
}
 
/*  T T V T -- Condition communication line for use as virtual terminal  */
 
ttvt(speed,flow) int speed, flow; {
    int s;
    if (ttyfd < 0) return(-1);  /* Not open. */
 
    s = ttsspd(speed,ttnmsv);   /* Check the speed */
 
    close(ttyfd);
    ttyfd= open(ttnmsv,0x43); /* magic to force mode change!!! */
    debug(F101,"ttvt: reopen of tty","",ttyfd);
    tttvt.sg_case =
    tttvt.sg_backsp =
    tttvt.sg_delete =
    tttvt.sg_echo = 
    tttvt.sg_alf =
    tttvt.sg_nulls =
    tttvt.sg_pause =
    tttvt.sg_bspch =
    tttvt.sg_dlnch =
    tttvt.sg_eorch =
    tttvt.sg_eofch =
    tttvt.sg_rlnch =
    tttvt.sg_dulnch =
    tttvt.sg_psch =
    tttvt.sg_kbich =
    tttvt.sg_kbach =
    tttvt.sg_bsech =
    tttvt.sg_tabcr = 0;
    if(s>-1)tttvt.sg_baud = (char)s;
    if (flow==1) {
	tttvt.sg_xon  = 0x11;
	tttvt.sg_xoff = 0x13;
    } else tttvt.sg_xon = tttvt.sg_xoff = 0;
    s = _ss_opt(ttyfd,&tttvt);   /* set new modes . */
    debug (F101,"ss_opt on tty was :","",s);
    return(0);
}
 
/*  T T S S P D  --  Return the internal baud rate code for 'speed'.  */
 
ttsspd(speed,name) {
    int s;
    char command[80];
 
    switch (speed) {
	case 50:	s = B50;	break;
	case 75:	s = B75;	break;
	case 110:	s = B110;	break;
	case 134:	s = B134;	break;
	case 150:	s = B150;	break;
	case 300:	s = B300;	break;
	case 600:	s = B600;	break;
	case 1200:	s = B1200;	break;
	case 1800:	s = B1800;	break;
	case 2000:	s = B2000;	break;
	case 2400:	s = B2400;	break;
	case 3600:	s = B3600;	break;
	case 4800:	s = B4800;	break;
	case 7200:	s = B7200;	break;
	case 9600:	s = B9600;	break;
	case 19200:	s = B19200;	break;
	default:
	    if (speed >= 0) {
		fprintf(stderr,"Unsupported line speed - %d\n",speed);
		fprintf(stderr,"Current speed not changed\n");
	    }
	    return -1;
    }
    sprintf(command,"xmode %s baud=%d",name,bdrts[s]);
    debug (F000,"ttsspd: command = ",command,'\0');
    debug (F101,"ttspd: baud","\0",s);
    system(command);
    return s;
}
 
 
 
/*  T T F L U I  --  Flush tty input buffer */
 
ttflui() {
    long n;
    char flushbuf[256];
    if (ttyfd < 0) return(-1);  /* Not open. */
    ungotn = -1;   /* Initialize myread() stuff */
    while((n=_gs_rdy(ttyfd))>0)
    	read(ttyfd, flushbuf, n>256 ? 256 : n);
    return 0;
}
 
/* Interrupt Functions */
 
/* Set up terminal interrupts on console terminal */
 
esctrp() {    /* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);  /* ignore until trapped */
}
 
/*  C O N I N T  --  Console Interrupt setter  */
 
conint(f) int (*f)(); {   /* Set an interrupt trap. */
    if (!isatty(0)) return(0);  /* only for real ttys */
    if (backgrd) return;  /* must ignore signals in bkgrd */
 
/*
 Except for special cases below, ignore keyboard quit signal.
 ^\ too easily confused with connect escape, and besides, we don't want
 to leave lock files around.  (Frank Prindle)
*/
    signal(SIGQUIT,esctrp);  /* console escape in pkt modes */
    if (conesc) {   /* clear out pending escapes */
	conesc = 0;
    }
 
    if (conif) return;   /* Nothing to do if already on. */
 
/* check if invoked in background -- if so signals set to be ignored */
 
    if (signal(SIGINT,SIG_IGN) == SIG_IGN) {
	backgrd = 1;   /*   means running in background */
	signal(SIGQUIT,SIG_IGN); /*   must leave signals ignored */
	return;
    }
    signal(SIGINT,f);   /* Function to trap to on interrupt. */
    conif = 1;    /* Flag console interrupts on. */
}
 
 
/*  C O N N O I  --  Reset console terminal interrupts */
 
connoi() {    /* Console-no-interrupts */
    if (!isatty(0)) return(0);  /* only for real ttys */
    if (backgrd) return;  /* Ignore signals in background */
 
    signal(SIGINT,SIG_DFL);
    signal(SIGQUIT,SIG_DFL);
    conif = 0;    /* Flag interrupt trapping off */
}
 
/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */
 
ttchk() {
    long n;
    n = _gs_rdy(ttyfd);
    return ( (n < 0) ? 0 : n);
}
 
 
/*  T T X I N  --  Get n characters from tty input buffer  */
 
/*  Returns number of characters actually gotten, or -1 on failure  */
 
/*  Intended for use only when it is known that n characters are actually */
/*  Available in the input buffer.  */
 
ttxin(n,buf) int n; char *buf; {
    int x;
    CHAR c;
 
    if((x = read(ttyfd,buf,n)) < 0) return -1;
    buf[x] = '\0';
    return(x);
}
 
/*  T T O L  --  Similar to "ttinl", but for writing.  */
 
ttol(s,n) int n; char *s; {
    int x;
    debug(F101,"ttol: ttyfd","",ttyfd);
    if (ttyfd < 0) return(-1);  /* Not open. */
    x = write(ttyfd,s,n);
    debug(F111,"ttol",s,n);
    if (x < 0) debug(F101,"ttol failed","",x);
    return(x);
}
 
 
/*  T T O C  --  Output a character to the communication line  */
 
ttoc(c) char c; {
    if (ttyfd < 0) return(-1);  /* Not open. */
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
 
ttinl(dest,max,timo,eol)
int max,timo;
register char *dest;
char eol;
{
    register int  i,j,n,m;
    int time, date, t;
    short day;
 
    m = ttprty ? 0177 : 0377;
    if (ttyfd < 0) return(-1);  /* Not open. */
    if (timo <= 0) {   /* Untimed read... */
 	if(when == MIDNIGHT) { /* unalarmed read */
	    i = 0;
	    j = 0;
	    for(;;) {
	    	if ((n = _gs_rdy(ttyfd)) > 0) {		/* see how many chars ready */
		    if (n > (max - i)) n = max - i;
		    if ((n = read(ttyfd, dest, n)) < 0) { /* get them all */
		        return -1;
		    }
		} else {
		    if((n = read(ttyfd, dest, 1)) < 0) {
		        return -1;
		    }
		}
		j += n;
		if (j >= max) {
		    debug(F101, "ttinl buffer overflow", "", j);
		    return -1;
		}
		for (; i < j; i++,dest++) {
		    *dest &= m;
		    if (*dest == eol) {
		        *++dest = '\0';
			return i;
		    }
		}
	    }
	}
    } else {	/* timed read */
	_sysdate(1, &when, &date, &day, &t);
	when += timo;
	if(when >= MIDNIGHT) when -= MIDNIGHT;
    }
    i = 0;
    j = 0;
    for (;;) {
	while(((n=_gs_rdy(ttyfd))<0)&&(errno==E_NOTRDY)) {
	    ticks = SLEEPINT;
	    _ss_ssig(ttyfd, SIGARB);
	    _sysdate(1, &time, &date, &day, &t);
	    if(time >= when && (when > MIDNIGHT/3 || time < MIDNIGHT*2/3)) {
	    	_ss_rel(ttyfd);
		when = MIDNIGHT;
		if(timo <= 0) catch(SIGALRM);
	    	return -1;
	    }
	    tsleep(ticks);
	}
	if (n > max - i) n = max - i;
	if (n > 0) n = read(ttyfd, dest, n);
	if (n < 0) {
	    if(timo>0) when = MIDNIGHT;
	    return n;  /* Return the error indication. */
	}
	j += n;
	if (j >= max) {
	    debug(F101, "ttinl buffer overflow", "", j);
	    if(timo>0) when = MIDNIGHT;
	    return -1;
	}
	for(; i < j; i++, dest++) {
	    *dest &= m;
	    if (*dest == eol) {
	        *++dest = '\0';
		if(timo>0) when = MIDNIGHT;
		return i;
	    }
	}
    }
}
 
/*  T T I N C --  Read a character from the communication line  */
 
ttinc(timo) int timo; {
    int m;
    int n = 0;
    CHAR ch = 0;
    int time, date, t;
    short day;
 
    m = ttprty ? 0177 : 0377;
    if (ttyfd < 0) return(-1);  /* Not open. */
    if (timo <= 0) {   /* Untimed. */
	if(when == MIDNIGHT) {
	    while ((n = read(ttyfd,&ch,1)) == 0) ; /* Wait for a character. */
	    return( (n > 0) ? (ch & m) : n );
	}
    } else {
    	_sysdate(1, &when, &date, &day, &t);
	when += timo;
	if(when >= MIDNIGHT) when -= MIDNIGHT;
    }
    while(((n=_gs_rdy(ttyfd))<0)&&(errno==E_NOTRDY)) {
    	ticks = SLEEPINT;
	_ss_ssig(ttyfd, SIGARB);
	_sysdate(1, &time, &date, &day, &t);
	if(time >= when && (when > MIDNIGHT/3 || time < MIDNIGHT*2/3)) {
	    _ss_rel(ttyfd);
	    when = MIDNIGHT;
	    if(timo <= 0) catch(SIGALRM);
	    return -1;
	}
	tsleep(ticks);
    }
    if(timo > 0) when = MIDNIGHT;
    if (n>0) n = read(ttyfd, &ch, 1);
    return ((n>0) ? ch&m : -1);
}
 
/*  T T S N D B  --  Send a BREAK signal  */
 
ttsndb()
{
    if (ttyfd < 0) return(-1);  /* Not open. */
    return setstat(SS_Break, ttyfd);
}
 
/*  R T I M E R --  Reset elapsed time counter  */
 
rtimer() 
{
    int temp;
    _sysdate(JULIAN,&tcount.time,&tcount.date,&temp,&temp);
}
 
 
/*  G T I M E R --  Get current value of elapsed time counter in seconds  */
 
gtimer() {
	
    int x;
    syst act;
    _sysdate(JULIAN,&act.time,&act.date,&x,&x);
    x = (int) (act.date-tcount.date)*86400 + act.time - tcount.time;
    rtimer();
    return( (x < 0) ? 0 : x );
}
 
 
/*  Z T I M E  --  Return date/time string  */
 
ztime(s) char **s;
{
    int time,date,tick;
    static char timb[30];
    short day;
    _sysdate(GREGORIAN,&time,&date,&day,&tick);
    sprintf(timb,"%.3s %.3s %2d %02d:%02d:%02d     %4d\n",
	&wday[day*3],&month[(((date&0x0ff00)>>8)-1)*3],date&0x0ff,
 	(time&0x0ff0000)>>16,(time&0x0ff00)>>8,time&0x0ff,
 	(date>>16)&0x0ffff);
    *s = timb;
}
 
/*  C O N G M  --  Get console terminal modes.  */
 
/*
 Saves current console mode, and establishes variables for switching between
 current (presumably normal) mode and other modes.
*/
 
congm() {
    if (!isatty(0)) return(0);  /* only for real ttys */
    _gs_opt(0,&ccold);   /* Structure for restoring */
    _gs_opt(0,&cccbrk);  /* For setting CBREAK mode */
    _gs_opt(0,&ccraw);   /* For setting RAW mode */
    cgmf = 1;    /* Flag that we got them. */
}
 
 
/*  C O N C B --  Put console in cbreak mode.  */
 
/*  Returns 0 if ok, -1 if not  */
 
concb(esc) char esc; {
    int x;
    if (!isatty(0)) return(0);	/* only for real ttys */
    if (cgmf == 0) congm();	/* Get modes if necessary. */
    ckxech = 1;			/* Program can echo characters */
    cccbrk.sg_echo  = 0;
    cccbrk.sg_pause = 0;
    cccbrk.sg_eofch = 0;
    x = _ss_opt(0,&cccbrk) | _ss_opt(1,&cccbrk);
    debug(F100,"console set to cbreak mode");
    return(x);
}
 
/*  C O N B I N  --  Put console in binary mode  */
 
/*  Returns 0 if ok, -1 if not  */
 
conbin(esc) char esc; {
    if (!isatty(0)) return(0);  /* only for real ttys */
    if (cgmf == 0) congm();  /* Get modes if necessary. */
    ckxech = 1;			/* Program can echo characters */
    ccraw.sg_case =
    ccraw.sg_backsp =
    ccraw.sg_delete =
    ccraw.sg_echo =
    ccraw.sg_alf =
    ccraw.sg_nulls =
    ccraw.sg_pause =
    ccraw.sg_bspch =
    ccraw.sg_dlnch =
    ccraw.sg_eorch =
    ccraw.sg_eofch =
    ccraw.sg_rlnch =
    ccraw.sg_dulnch =
    ccraw.sg_psch =
    ccraw.sg_kbich =
    ccraw.sg_kbach = 
    ccraw.sg_bsech =
    ccraw.sg_tabcr =
    ccraw.sg_xon =
    ccraw.sg_xoff = 0;
    _ss_opt(0,&ccraw);
    _ss_opt(1,&ccraw);		/* set new modes . */
    debug(F100,"console switched to raw mode");
    if (!conesc)signal(SIGQUIT,esctrp);
    return(0);   
}
 
 
/*  C O N R E S  --  Restore the console terminal  */
 
conres() {
    if(cgmf==0 || !isatty(0)) return 0; /* only for real ttys with known modes*/
    tsleep(2);
    ckxech = 0;    /* System should echo chars */
    return(_ss_opt(0,&ccold)| _ss_opt(1,&ccold));  /* Restore controlling tty */
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
    write(1,"\r\l",2);
}
 
/*  C O N C H K  --  Return how many characters available at console  */
 
conchk() 
{
    int x;
    x=_gs_rdy(0);
    if(x<0 && errno==E_NOTRDY) x=0;
    debug (F101,"conchk","",x);
    return x;
}
 
/*  C O N I N C  --  Get a character from the console  */
 
coninc(timo)
int timo;
{
    int n = 0;
    char ch;
    int time, date, t;
    short day;

    if (timo <= 0) {			/* untimed */
	if(when == MIDNIGHT) {
	    n = read(0, &ch, 1);	/* Read a character. */
	    ch &= 0377;
	    if (n > 0) return(ch);	/* Return the char if read */
	    else return(-1);		/* Return the char, or -1. */
	}
    } else {
	_sysdate(1, &when, &date, &day, &t);
	when += timo;
	if(when >= MIDNIGHT) when -= MIDNIGHT;
    }
    while(((n=_gs_rdy(0))<0)&&(errno==E_NOTRDY)) {
    	ticks = SLEEPINT;
	_ss_ssig(0, SIGARB);
	_sysdate(1, &time, &date, &day, &t);
	if(time >= when && (when > MIDNIGHT/3 || time < MIDNIGHT*2/3)) {
	    _ss_rel(0);
	    when = MIDNIGHT;
	    if(timo <= 0) catch(SIGALRM);
	    return -1;
	}
	tsleep(ticks);
    }
    if(timo>0) when = MIDNIGHT;
    if (n>0) {
	n = read(0, &ch, 1);
	ch &= 0377;
    }
    if (n>0) return ch;
    return -1;
}

/* unix alarm faking */

alarm(timo)
int timo;
{
    int date, t;
    short day;

    if(timo==0) {
    	when = MIDNIGHT;
	return;
    }
    _sysdate(1, &when, &date, &day, &t);
    when += timo;
    if(when >= MIDNIGHT) when -= MIDNIGHT;
}

/* emulate unix signal functions */

fptr signal(sig,func)
int sig;
fptr func;
{
    fptr temp;
    if (sig < MAXSIG) {
	temp = sigtbl[sig];
	sigtbl[sig] = func;
	return temp;
    }
    if (sig == SIGALRM) {
    	temp = sigtbl[MAXSIG];
	sigtbl[MAXSIG] = func;
	return temp;
    }
    return (fptr)-1;
}
 
int catch(sig) {
    register fptr temp;

    if(sig==SIGARB) {
	ticks = 1;
	return;
    }
    if(sig < MAXSIG) {
	if ((temp=sigtbl[sig])==SIG_DFL) exit(1);
	if(temp!=SIG_IGN) (*temp)();
	return;
    }
    if(sig == SIGALRM) {
    	if ((temp=sigtbl[MAXSIG])==SIG_DFL) return;
	if (temp!=SIG_IGN) (*temp)();
	return;
    }
    exit(1);
}

int isatty(path)
int path;
{
    int x;
    struct sgbuf buffer;
    return(((x = _gs_opt(path,&buffer))<0) ? 0 : buffer.sg_class == 0);
}

msleep(interval)
register int interval;
{
    interval <<= 8;		/* convert from miliseconds to 256ths */
    interval /= 1000;
    if(interval > MINSLEEP) tsleep(interval|0x80000000);
    else tsleep(2);
}
