char *ckxv = "Data General tty I/O, 4C(039), 11 May 87";

/*  C K U T I O  */

/* C-Kermit interrupt, terminal control & i/o functions for Unix systems */

/*
 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
  
 This module was adapted to the Data General computers by:

    Phil Julian, SAS Institute, Inc., Box 8000, Cary, NC 27512-8000
  
*/
/* Includes for all Unixes (conditional includes come later) */

#ifdef datageneral
/* Data General includes need to come here, esp. for items like DEVNAMLEN */
#define MAXINBUF 2048                  /* See ckdcon.c for same definition. */
#include <memory.h>
#include <string.h>
#include <dglib.h>
#include <bit.h>
#include <sysid.h>
#include <sys_calls.h>
#include <packets/normal_io.h>
#include <multitask.h>                                  
#define CONINT_TSK 20                   /* 16, 17 are taken IDs */
#define CONINT_PRI 0                    /* Same as calling task */ 
#define CONINT_STACK 0
int con_reads_mt = 0;                   /* Flag if task is active */
int conint_ch;                          /* MT character read */
int conint_avl;                         /* Flag that char available */
int termtype;
/* Delimiter table -- defaults to CR only */
short idel_tbl[7] = {0x4,0,0,0,0,0,0};
#include <stdio.h>			/* Unix Standard i/o */
#ifndef DEVNAMLEN
#define DEVNAMLEN (L_ctermid + 6)
#endif 

#else 
#include <sys/types.h>			/* Types */
#endif datageneral

#include <sys/dir.h>			/* Directory */
#include <ctype.h>			/* Character types */
/* Maximum length for the name of a tty device */

#ifndef DEVNAMLEN
#define DEVNAMLEN 25
#endif

#include <stdio.h>			/* Unix Standard i/o */
#include <signal.h>			/* Interrupts */
#include <setjmp.h>			/* Longjumps */
#include "ckcdeb.h"			/* Typedefs, formats for debug() */

/* 4.1 BSD support added by Charles E. Brooks, EDN-VAX */
/* Fortune 16:32 For:Pro 1.7 support mostly like 4.1, added by J-P Dumas */

#ifdef BSD4
#define ANYBSD
#ifdef MAXNAMLEN
#define BSD42
char *ckxsys = " 4.2 BSD";
#else
#ifdef FT17
#define BSD41
char *ckxsys = " For:Pro Fortune 1.7";
#else
#define BSD41
#ifndef C70
char *ckxsys = " 4.1 BSD";
#endif
#endif
#endif
#endif

/* 2.9bsd support contributed by Bradley Smith, UCLA */
#ifdef BSD29
#define ANYBSD

char *ckxsys = " 2.9 BSD";
#endif

/*
 Version 7 UNIX support contributed by Gregg Wonderly,
 Oklahoma State University:  gregg@okstate.csnet
*/
#ifdef	V7
char *ckxsys = " Version 7 UNIX (tm)";
#endif

/* BBN C70 support from Frank Wancho, WANCHO@SIMTEL20 */
#ifdef C70
char *ckxsys = " BBN C/70";
#endif


/* Amdahl UTS 2.4 (v7 derivative) for IBM 370 series compatible mainframes */
/* Contributed by Garard Gaye, Jean-Pierre Dumas, DUMAS@SUMEX-AIM. */
#ifdef UTS24
char *ckxsys = " Amdahl UTS 2.4";
#endif

/* Pro/Venix Version 1.x support from Columbia U. */
#ifdef PROVX1
char *ckxsys = " Pro-3xx Venix v1";
#endif

/* Tower support contributed by John Bray, Auburn, Alabama */
#ifdef TOWER1
char *ckxsys = " NCR Tower 1632, OS 1.02";
#endif

/* Sys III/V, Xenix, PC/IX support by Herm Fischer, Encino, CA */
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
/* Datageneral support contributed by Phil Julian, SAS Institute, Inc. */
#ifdef dgux
char *ckxsys = " Data General DG/UX";
#else dgux
#ifdef datageneral
char *ckxsys = " Data General AOS/VS";

#else
char *ckxsys = " AT&T System III/System V";

#endif
#endif
#endif
#endif
#endif
#endif

/* Features... */


/* Do own buffering, using unbuffered read() calls... */
#ifdef UXIII
#define MYREAD
#endif

#ifdef BSD42
#define MYREAD
#include <errno.h>
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

   sysinit()               -- System dependent program initialization
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
   rtimer() --  Reset timer
   gtimer()  -- Get elapsed time since last call to rtimer()
*/

/* Conditional Includes */

#ifdef FT17
#include <sys/file.h>	  		/* File information */
#endif

#ifndef PROVX1
#ifdef datageneral
#include <sys/stat.h>
#else
#include <sys/file.h>	  		/* File information */
#endif
#endif

/* System III, System V */

#ifdef UXIII
#ifdef datageneral
/* Console and terminal characteristics vectors */
#include <paru.h>
short *charcurr;
short charconin[$CLMAX];    /* Console input; channel 0 */
short charconout[$CLMAX];   /* Console output; channel 1 */
short chartty[$CLMAX];      /* generic tty channel */
short charttyin[$CLMAX];    /* tty input; channel ttyfd */
short charttyout[$CLMAX];   /* tty output; channel ttyfdout */

short ch_ttold[$CLMAX];
short ch_ttraw[$CLMAX];
short ch_ttvt[$CLMAX]; 
short ch_ccold[$CLMAX];
short ch_ccraw[$CLMAX];
short ch_ccbrk[$CLMAX]; 
#undef ECHO
#include <sys/termio.h>
/* Add synonymns for Unix type ioctl */
#define TCSETAF TCSETA
#define TCSETAW TCSETA

#else
#include <termio.h>
#include <sys/ioctl.h>
#endif datageneral

#include <fcntl.h>			/* directory reading for locking */
#include <errno.h>			/* error numbers for system returns */
#endif

/* Not Sys III/V */

#ifndef UXIII
#include <sgtty.h>			/* Set/Get tty modes */
#ifndef PROVX1
#ifndef V7
#ifndef BSD41
#include <sys/time.h>			/* Clock info (for break generation) */
#endif
#endif
#endif
#endif

#ifdef BSD41

#include <sys/timeb.h>			/* BSD 4.1 ... ceb */
#endif

#ifdef BSD29
#include <sys/timeb.h>			/* BSD 2.9 (Vic Abell, Purdue) */
#endif

#ifdef TOWER1
#include <sys/timeb.h>			/* Clock info for NCR Tower */
#endif

/* Declarations */

long time();				/* All Unixes should have this... */
extern int errno;			/* System call error return */

/* Special stuff for V7 input buffer peeking */

#ifdef	V7
int kmem[2] = { -1, -1};
char *initrawq(), *qaddr[2]={0,0};
#define CON 0
#define TTY 1
#endif

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

#ifdef PROVX1
    char *dftty = "/dev/com1.dout"; /* Only example so far of a system */
    int dfloc = 1;		    /* that goes in local mode by default */
#else
    char *dftty = CTTNAM;		/* Remote by default, use normal */
    int dfloc = 0;			/* controlling terminal name. */
#endif

    int dfprty = 0;			/* Parity (0 = none) */
    int dfflow = 1;			/* Xon/Xoff flow control */
    int backgrd = 0;			/* Assume in foreground (no '&' ) */

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

/* Declarations of variables global within this module */

static long tcount;			/* Elapsed time counter */

static char *brnuls = "\0\0\0\0\0\0\0"; /* A string of nulls */

static jmp_buf sjbuf, jjbuf;		/* Longjump buffer */
static int lkf = 0,			/* Line lock flag */
    conif = 0,				/* Console interrupts on/off flag */
    cgmf = 0,				/* Flag that console modes saved */
    xlocal = 0;				/* Flag for tty local or remote */
int ttyfd = -1;			/* TTY file descriptor */
static char escchr;			/* Escape or attn character */

/* Special line discipline, 4.2bsd only, and only with kernel mods... */
#ifdef KERLD
    static int kerld = 1;		/* Special Kermit line discipline... */
    struct tchars oldc, newc;		/* Special characters */
    int ld = NETLDISC;			/* Really a hack to "Berknet" l.d. */
    int oldld;				/* Old discipline */
#else
    static int kerld = 0;		/* Not selected, no special l.d. */
#endif

#ifdef BSD42
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#endif

#ifdef BSD29
    static long clock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif

#ifdef BSD41
    static long clock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif

#ifdef TOWER1
static long clock;			/* For getting time from sys/time.h */
static struct timeb ftp;		/* And from sys/timeb.h */
#endif

#ifdef V7
static long clock;
#endif

#ifdef UXIII
  static struct termio 			/* sgtty info... */
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */
#else
  static struct sgttyb 			/* sgtty info... */
    ttold, ttraw, tttvt, ttbuf,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */

#endif

static char flfnam[80];			/* uucp lock file path name */
static int hasLock = 0;			/* =1 if this kermit locked uucp */
static int inbufc = 0;			/* stuff for efficient SIII raw line */
static int ungotn = -1;			/* pushback to unread character */
static int conesc = 0;			/* set to 1 if esc char (^\) typed */

static int ttlock();			/* definition of ttlock subprocedure */
static int ttunlck();			/* and unlock subprocedure */
static char ttnmsv[DEVNAMLEN];		/* copy of open path for tthang */

#ifdef datageneral
FILE *ttfile,*ttfileout;                /* Files opened by ttopen */
  
struct p_nio_ex w_io_parms;             /* ?write system call structure */
struct p_nio_ex r_io_parms;             /* ?read system call structure */

struct p_nio_ex io_parms;               /* open structure in ttopen */
struct p_nio_ex x_io_parms;             /* ?read/?write structure for ttyfd */
struct p_nio_ex xout_parms;             /* ?write structure for ttyfdout */
struct p_screen x_io_scrn,
       r_io_scrn,io_screen;             /* Edit-read structures */

int ttyfdout = -1;                      /* ?write Unix number for remote out */
short timotty = 0;                      /* {Flag for timeouts (=1), value */
short timocon = 0;                      /* tty is ttyfd..., con is Unix 0/1 */
extern char *SPACMD;                    /* Need to supply ":udd:username" */
#endif

/*  S Y S I N I T  --  System-dependent program initialization.  */

int sysinit() {

/* for now, nothing... */
#ifdef datageneral
     /* Set up many things on the DG system, for initialization, etc. */
     
     short flags[3];
     char *username;
     int err;
     
     /* Get the terminal type, since DG terminals are different from others,
        especially in the way BS is handled ('\b' is EM).
        termtype = 0 means a regular DG style terminal, and 
        termtype = 1 means an ANSI style terminal with ^H for backspace.
        If Kermit is run in batch or has redirected input, then we do not
        want to test or set the console characteristics.
     */
     termtype = 0;
     if (isatty(0)) {
     	if (sys_gchr(channel(0),1<<31,&flags))
             { perror("sysinit: sys_gchr: "); return(1); }

        /* Only determine whether or not it is a $TTY */
        if ((flags[1] & $DTYPE) == $TTY) termtype = 1;
     }

     zero((char *) &w_io_parms, sizeof(w_io_parms));
     w_io_parms.ich  = channel(1); 		     /* stdout */
     w_io_parms.isti = $IBIN|$RTDS|$ICRF|$OFOT;
     w_io_parms.isti &= ~$IPST;
     w_io_parms.imrs = MAXINBUF-1;
     w_io_parms.ibad = -1;
     w_io_parms.ircl = -1;

     zero((char *) &r_io_parms, sizeof(r_io_parms));
     r_io_parms.ich  = channel(0); 		     /* stdin */
     r_io_parms.isti = $IBIN|$RTDS|$ICRF|$OFIN;
     r_io_parms.isti &= ~$IPST;
     r_io_parms.imrs = MAXINBUF-1;
     r_io_parms.ibad = -1;
     r_io_parms.ircl = -1;

     zero((char *) &r_io_scrn, sizeof(r_io_scrn));
     r_io_parms.etsp = 0;              /* Screen management extension */
     r_io_scrn.esfc &= ~$ESNE;	       /* Echo input data */
     r_io_scrn.esfc &= ~$ESSE;	       /* No screen edit */
     r_io_scrn.esfc |= $ESGT;	       /* Dump ring buffer */

     zero((char *) &x_io_parms, sizeof(x_io_parms));
     /* $RTDY must be used instead of $RTDS since this causes failure on
      * the Prime, which is probably getting data overruns on output or
      * missing data.  Also, $RTDY is more efficient and faster.
      */
     x_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFIO|$IPKL;
     x_io_parms.isti &= ~$IPST;
     x_io_parms.imrs = MAXINBUF-1;
     x_io_parms.ibad = -1;
     x_io_parms.ircl = -1;
     /* IDEL would default to NULL, FF, CR, NL as delimeters.
      * The only delimeters here are CR and NL, for data sensitive 
      * records.
      */
     idel_tbl[0] = 0x24;
     x_io_parms.idel = idel_tbl;

     zero((char *) &x_io_scrn, sizeof(x_io_scrn));
     x_io_parms.etsp = &x_io_scrn;     /* Screen management extension */
     x_io_parms.etsp |= BIT0;	       /* Turn on the high bit */
     x_io_scrn.esfc &= ~$ESNE;	       /* Echo input data */
     x_io_scrn.esfc &= ~$ESSE;	       /* No screen edit */
     x_io_scrn.esfc |= $ESGT;	       /* Dump ring buffer */

     memcpy((char *) &xout_parms,(char *) &x_io_parms,sizeof(x_io_parms));
     xout_parms.isti = $IBIN|$RTDY|$ICRF|$OFOT;
     xout_parms.etsp = 0;              /* Screen management extension */

     username = getenv("LOGNAME");
     memcpy(SPACMD+11, username, min(31,strlen(username)));

     /* Zero the console characteristics arrays */
     zero ((char *) charconin, sizeof(charconin));
     zero ((char *) charconout, sizeof(charconout));
     zero ((char *) chartty, sizeof(chartty));
     zero ((char *) charttyin, sizeof(charttyin));
     zero ((char *) charttyout, sizeof(charttyout));
      
     /* Initialize charconin & out, 
      * and set up the console for CHAR/BREAK=BMOB 
      */
     if (isatty(0)) {
          if (sys_gechr( channel(0), (1<<31)|$CLMAX, charconin)) 
               { perror("sysinit: gechr 0"); return(-1); }
          if (sys_gechr( channel(1), (1<<31)|$CLMAX, charconout)) 
               { perror("sysinit: gechr 1"); return(-1); }
          charconin[4] &= ~$CBKM;
          charconin[4] |= $CBBM;
          if (sys_sechr( channel(0), (1<<31)|$CLMAX, charconin)) 
               { perror("sysinit: sechr "); return(-1); }
     }
      
     return(0);
}

/* S E T T O -- set timeout on the channel to period.
                Save the old characteristics in char***in[]. 
*/

int setto(chan, period) int chan, period; 
{
     int isconsole = 0;
     int iscon[2];       /* Flags because i/o could be redirected */

     /* Setting is done external to here, since they can distinguish the 
        console from ttyfd device.  Both console channels must be set
        the same.
     */
     if ( (chan == channel(0)) || (chan == channel(1)) ) {
          isconsole = 1; zero((char *) iscon, sizeof(iscon));
          chan = channel(0);
          if (sys_gechr( channel(0), (1<<31)|$CLMAX, charconin)) 
               if (isatty(0)) {
                    iscon[0] = 1;
                    perror("setto: gechr 0: "); 
                    return(-1);
               }
          charcurr = charconin;
          if (sys_gechr( channel(1), (1<<31)|$CLMAX, charconout)) 
               if (isatty(1)) {
                    iscon[1] = 1;
                    perror("setto: gechr 1: "); 
                    return(-1);
               }
          timocon = period;
     }

     else {
          if (sys_gechr( chan, (1<<31)|$CLMAX, chartty))
               { perror("setto: gechr tty: "); return(-1); }
          charcurr = chartty;
          timotty = period;
     }

     charcurr[1] |= (0100000>>$CTO);
     if (sys_sechr( chan, (1<<31)|$CLMAX, charcurr))
          if ((isconsole == 0) || iscon[0]) {
               perror("setto: sechr: "); 
               return(-1); 
          }
     if (sys_stom( chan, 1<<31, period)) 
          if ((isconsole == 0) || iscon[0]) {
               perror("setto: stom: "); 
               return(-1);
          }

     if (isconsole == 0) return(0);
     
     chan = channel(1);
     charconout[1] |= (0100000>>$CTO);
     if (sys_sechr( chan, (1<<31)|$CLMAX, charconout)) 
          if (iscon[1]) {
               perror("setto: sechr 1: "); 
               return(-1);
          }
     if (sys_stom( chan, 1<<31, period)) 
          if (iscon[1]) {
               perror("setto: stom 1: "); 
               return(-1);
          }
     return(0);
}


/* R E S T O -- reset timeouts to the previous values, and restore char */

int resto(chan) int chan;
{
     int i, chan0, chan1, iscon[2];

     if ( (chan==channel(0)) || (chan==channel(1)) ) {

          chan0 = channel(0);      chan1 = channel(1); 
          iscon[0] = isatty(0);    iscon[1] = isatty(1);
          charconin[1] &= ~(0100000>>$CTO);
          if (sys_stom( chan0, 1<<31, -1)) 
               if (iscon[0]) {
                    perror("resto: stom 0: "); 
                    return(-1);
               }
          for (i = 0; i < $CLMAX; i++) if (charconin[i] != 0) {
               if (sys_sechr( chan0, (1<<31)|$CLMAX, charconin))
                    if (iscon[0]) {
                         perror("resto: sechr 0: "); 
                         return(-1); 
                    }
               break;
          }
          charconout[1] &= ~(0100000>>$CTO);
          if (sys_stom( chan1, 1<<31, -1)) 
               if (iscon[1]) {
                    perror("resto: stom 1: "); 
                    return(-1);
               }
          for (i = 0; i < $CLMAX; i++) if (charconout[i] != 0) {
               if (sys_sechr( chan1, (1<<31)|$CLMAX, charconout)) 
               if (iscon[1]) {
                    perror("resto: sechr 1: "); 
                    return(-1);
               }
               break;
          }

          timocon = 0; 
     }
     else {
          chartty[1] &= ~(0100000>>$CTO);
          if (sys_stom( chan, 1<<31, -1)) 
               { perror("setto: stom tty: "); return(-1); }
          for (i = 0; i < $CLMAX; i++) if (chartty[i] != 0) {
               if (sys_sechr( chan, (1<<31)|$CLMAX, chartty))
                    { perror("resto: sechr tty: "); return(-1); }
               break;
          }
          timotty = 0; 
     }
      
     return(0);
}


/* S E T F L O W -- Set flow control in the device. 
     
    The device is assumed to be an intelligent IAC, where a CHAR/ON/IFC/OFC
    could be done.

*/
int setflow(chan) int chan;
{
     short termparms[$CLMAX];

     if (sys_gechr( chan, (1<<31)|$CLMAX, termparms)) 
          { perror("setflow: gechr "); return(-1); }

     bitset(&termparms[4],$XOFC,1);
     bitset(&termparms[4],$XIFC,1);
     
     if (sys_sechr( chan, (1<<31)|$CLMAX, termparms)) 
          { perror("setflow: sechr "); return(-1); }

     return(0);
}


/* R E S F L O W -- Re-Set flow control in the device. 
     
    The device is assumed to be an intelligent IAC, where a CHAR/OFF/IFC/OFC
    could be done.

*/
int resflow(chan) int chan;
{
     int i;
     short termparms[$CLMAX];

     if (sys_gechr( chan, (1<<31)|$CLMAX, termparms)) 
          { perror("resflow: gechr "); return(-1); }

     bitset(&termparms[4],$XOFC,0);
     bitset(&termparms[4],$XIFC,0);
     
     if (sys_sechr( chan, (1<<31)|$CLMAX, termparms))
          { perror("resflow: sechr "); return(-1); }

     return(0);
}


/* S E T S P E E D -- Set the baud rate when not possible thru IOCTL.
     
    The device is assumed to be an intelligent IAC, where a CHAR/ON/BAUD=
    could be done.

*/
int setspeed(sp) int sp;
{
     short termparms[$CLMAX];

     /* ttyfd is the Unix file number for the tty device.  Setting the
      * characteristics will set it for both ttyfd and ttyfdout, since
      * it is the device itself which is affected, and not just the 
      * channel.
      */
     if (sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, termparms)) 
          { perror("setspeed: gechr "); return(-1); }

     termparms[3] &= ~$BRMSK;            /* Zero out the previous setting */
     termparms[3] |= sp;
     
     if (sys_sechr( channel(ttyfd), (1<<31)|$CLMAX, termparms)) 
          { perror("setspeed: sechr "); return(-1); }

     return(0);
}
#endif datageneral

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

#ifdef datageneral
    /* AOS/VS special stuff */
    FILE *dg_open();                    /* Custom file open */
    int retrycnt = 0;                   /* Retry if Modem disc in progress */
#endif
#ifdef UXIII
    char *ctermid();			/* Wish they all had this! */
#endif

    char *x; extern char* ttyname();
    char cname[DEVNAMLEN+4];

    if (ttyfd > -1) return(0);		/* If already open, ignore this call */
    xlocal = *lcl;			/* Make this available to other fns */
#ifdef NEWUUCP
    acucntrl("disable",ttname);		/* Open getty on line (4.3BSD) */
#endif
#ifndef datageneral
#ifdef UXIII
    /* if modem connection, don't wait for carrier */
    ttyfd = open(ttname,O_RDWR | (modem ? O_NDELAY : 0) );
#endif
#endif

#ifdef datageneral
    /* A custom open is done for the device, but a FILE handle is also needed
     * for doing other special operations on the device.  See sysinit() for
     * the initialization of the x_io_parms and xout_parms structures.
     *
     * First open up the output channel ($OFOT) 
     */
    idel_tbl[0] = 0x4;             /* CR only */
    xout_parms.idel = idel_tbl;
    xout_parms.ifnp = ttname;

retry:
    if ((ttfileout =
       dg_open(ttname, xout_parms.isti, xout_parms.isto, 
               xout_parms.imrs,xout_parms.ires,xout_parms.idel))
       == NULL) {
          /* If a modem disconnect, or one in progress, wait and retry. */
          if ((errno == ERDCT) || (errno == ERCDS))
               if (retrycnt == 0) {
                    retrycnt = 1; 
                    sleep(2); 
                    goto retry; 
               }
          ttyfdout = -1;
          perror("ttopen: output_line: dg_open");
          return(-1);
    }
    /* Successfull opens fall through here! */
    
    ttyfdout = fileno(ttfileout);
    xout_parms.ich = channel(ttyfdout);
    

    /* Next, open up the input channel ($OFIN) */

    x_io_parms.ifnp = ttname;
    /* IDEL would default to NULL, FF, CR, NL as delimeters.
     * The only delimeters here are CR and NL, for data sensitive 
     * records.
     */
    idel_tbl[0] = 0x24;
    x_io_parms.idel = idel_tbl;

    if ((ttfile =
       dg_open(ttname, x_io_parms.isti, x_io_parms.isto, 
               x_io_parms.imrs,x_io_parms.ires,x_io_parms.idel,
               (P_SCREEN *) x_io_parms.etsp))
       == NULL) {
          ttyfd = -1;
          perror("ttopen: input_line: dg_open");
          return(-1);
       }
    else {
          ttyfd = fileno(ttfile);
          x_io_parms.ich = channel(ttyfd);
    }

#else datageneral
    ttyfd = open(ttname,2);		/* Try to open for read/write */

    if (ttyfd < 0) {			/* If couldn't open, fail. */
	perror(ttname);
	return(-1);
    }
#endif

    strncpy(ttnmsv,ttname,DEVNAMLEN);	/* Open, keep copy of name locally. */

/* Caller wants us to figure out if line is controlling tty */

    debug(F111,"ttopen ok",ttname,*lcl);
    if (*lcl == -1) {
	if (strcmp(ttname,CTTNAM) == 0) {   /* "/dev/tty" always remote */
	    debug(F110," Same as CTTNAM",ttname,0);
	    xlocal = 0;
	} else if (isatty(0)) {		/* Else, if stdin not redirected */
	    x = ttyname(0);		/* then compare its device name */
	    strncpy(cname,x,DEVNAMLEN);	/* (copy from internal static buf) */
	    debug(F110," ttyname(0)",x,0);
	    x = ttyname(ttyfd); 	/* ...with real name of ttname. */
	    xlocal = (strncmp(x,cname,DEVNAMLEN) == 0) ? 0 : 1;
	    debug(F111," ttyname",x,xlocal);
	} else {			/* Else, if stdin redirected... */
#ifndef datageneral
#ifdef UXIII
/* Sys III/V provides nice ctermid() function to get name of controlling tty */
    	    ctermid(cname);		/* Get name of controlling terminal */
	    debug(F110," ctermid",cname,0);
	    x = ttyname(ttyfd); 	/* Compare with name of comm line. */
	    xlocal = (strncmp(x,cname,DEVNAMLEN) == 0) ? 0 : 1;
	    debug(F111," ttyname",x,xlocal);
/* Just assume local, so "set speed" and similar commands will work */
/* If not really local, how could it work anyway?... */

	    xlocal = 1;
	    debug(F101," redirected stdin","",xlocal);
#endif UXIII
#endif datageneral
        }
    }    

/* Now check if line is locked -- if so fail, else lock for ourselves */

    lkf = 0;				/* Check lock */
    if (xlocal > 0) {
	if (ttlock(ttname) < 0) {
	    fprintf(stderr,"Exclusive access to %s denied\n",ttname);
	    close(ttyfd); ttyfd = -1;
	    debug(F110," Access denied by lock",ttname,0);

	    return(-1);			/* Not if already locked */
    	} else lkf = 1;
    }

/* Got the line, now set the desired value for local. */

    if (*lcl < 0) *lcl = xlocal;

/* Some special stuff for v7... */

#ifdef	V7
	if (kmem[TTY] < 0) {	/*  If open, then skip this.  */
		qaddr[TTY] = initrawq(ttyfd);	/* Init the queue. */
		if ((kmem[TTY] = open("/dev/kmem", 0)) < 0) {
			fprintf(stderr, "Can't read /dev/kmem in ttopen.\n");
			perror("/dev/kmem");
			exit(1);
		}
	}
#endif	V7

/* Request exclusive access on systems that allow it. */

#ifndef XENIX
/* Xenix exclusive access prevents open(close(...)) from working... */
#ifdef TIOCEXCL
    	if (ioctl(ttyfd,TIOCEXCL, NULL) < 0)
	    fprintf(stderr,"Warning, problem getting exclusive access\n");
#endif
#endif

/* Get tty device settings */

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
#ifdef datageneral
    debug(F101,"ttopen, ttyfdout","",ttyfdout);

    /* For DG, also get the tty extended device characteristics. */
    sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttold);
    sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttraw);
    sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttvt);
#endif
    debug(F101," lcl","",*lcl);
    debug(F111," lock file",flfnam,lkf);
    return(0);
}

/*  T T C L O S  --  Close the TTY, releasing any lock.  */

ttclos() {
    if (ttyfd < 0) return(0);		/* Wasn't open. */
    if (xlocal) {
	if (tthang())			/* Hang up phone line */
	    fprintf(stderr,"Warning, problem hanging up the phone\n");
#ifndef datageneral
    	if (ttunlck())			/* Release uucp-style lock */
	    fprintf(stderr,"Warning, problem releasing lock\n");
#endif
    }
    ttres();				/* Reset modes. */
/* Relinquish exclusive access if we might have had it... */
#ifndef XENIX
#ifdef TIOCEXCL
#ifdef TIOCNXCL
    if (ioctl(ttyfd, TIOCNXCL, NULL) < 0)
    	fprintf(stderr,"Warning, problem relinquishing exclusive access\n");
#endif
#endif
#endif
#ifdef datageneral
    debug(F101,"ttclos, ttyfd","",ttyfd);
    debug(F101,"ttclos, ttyfdout","",ttyfdout);
    fclose(ttfile);			/* Close it. */
    ttyfd = -1;				/* Mark it as closed. */

    /* Close the output tty device */
    fclose(ttfileout);                  /* Close it. */
    ttyfdout = -1;			/* Mark it as closed. */
#else
    close(ttyfd);			/* Close it. */
#ifdef NEWUUCP
    acucntrl("enable",flfnam);		/* Close getty on line. */
#endif
    ttyfd = -1;				/* Mark it as closed. */

#endif
    return(0);
}

/*  T T H A N G -- Hangup phone line */

tthang() {
#ifdef UXIII
    unsigned short ttc_save;
#endif

    if (ttyfd < 0) return(0);		/* Not open. */
#ifdef ANYBSD
    ioctl(ttyfd,TIOCCDTR,0);		/* Clear DTR */
    msleep(500);			/* Let things settle */
    ioctl(ttyfd,TIOCSDTR,0);		/* Restore DTR */
#endif
#ifdef UXIII
    ttc_save = ttraw.c_cflag;
     ttraw.c_cflag &= ~CBAUD;		/* swa: set baud rate to 0 to hangup */
    if (ioctl(ttyfd,TCSETAF,&ttraw) < 0) return(-1); /* do it */
    msleep(100);			/* let things settle */
    ttraw.c_cflag = ttc_save;
#ifndef XENIX		/* xenix cannot do close/open when carrier drops */
				/* following corrects a PC/IX defficiency */
#ifndef aosvs
    ttc_save = fcntl(ttyfd,F_GETFL,0);
#endif
#ifdef datageneral
    if (sys_sechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttraw)) 
         { perror("tthang: sechr "); return(-1); }
#else
    close(ttyfd);		/* close/reopen file descriptor */
    if ((ttyfd = open(ttnmsv, ttc_save)) < 0) return(-1);
#endif 
#endif
    if (ioctl(ttyfd,TCSETAF,&ttraw) < 0) return(-1); /* un-do it */
#endif
    return (0);
}


/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {				/* Restore the tty to normal. */
    if (ttyfd < 0) return(-1);		/* Not open. */
#ifndef UXIII				/* except for sIII, */
    sleep(1);				/* Wait for pending i/o to finish. */
#endif					/*   (sIII does wait in ioctls) */
#ifdef KERLD
    if (kerld) ioctl(ttyfd,TIOCSETD,&oldld); /* Restore old line discipline. */
#endif
#ifdef UXIII
    if (ioctl(ttyfd,TCSETAW,&ttold) < 0) return(-1); /* restore termio stuff */
#else
    if (stty(ttyfd,&ttold) < 0) return(-1); /* Restore sgtty stuff */
#endif
#ifdef KERLD
    if (kerld) ioctl(ttyfd,TIOCSETC,&oldc); /* Restore old special chars. */
#endif
#ifdef datageneral
    /* Un-set all the timeouts */
    if (timotty) {
         resto(channel(ttyfd));
         resto(channel(ttyfdout));
    }
    if (sys_sechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttold)) 
         { perror("ttres: sechr "); return(-1); }
#endif

    return(0);
}

/* Exclusive uucp file locking control */
/*
 by H. Fischer, creative non-Bell coding !
 copyright rights for lock modules assigned to Columbia University
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
    char lockfil[50];			/* Max length for lock file name */

#ifdef ISIII
    char *lockdir = "/etc/locks";
#else
#ifdef ATT3BX
    char *lockdir = "/usr/spool/locks";
#else
    char *lockdir = "/usr/spool/uucp";
#endif
#endif

    device = ( (devname=xxlast(ttname,'/')) != NULL ? devname+1 : ttname);

#ifdef ISIII
    (void) strcpy( lockfil, device );
#else
    strcat( strcpy( lockfil, "LCK.." ), device );
#endif


    if (access( lockdir, 04 ) < 0) {	/* read access denied on lock dir */
	fprintf(stderr,"Warning, read access to lock directory denied\n");
	return( 1 );			/* cannot check or set lock file */
    }
	
    strcat(strcat(strcpy(flfnam,lockdir),"/"), lockfil);
    debug(F110,"look4lk",flfnam,0);

    if ( ! access( flfnam, 00 ) ) {	/* print out lock file entry */
	char lckcmd[40] ;
	strcat( strcpy(lckcmd, "ls -l ") , flfnam);
	system(lckcmd);
	if (access(flfnam,02) == 0)
	    printf("(You may type \"! rm %s\" to remove this file)\n",flfnam);
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
#ifdef ATT3BX
    FILE *lck_fild;
#endif
#ifdef datageneral
    /* The Data General locks a device when that device is opened.  So
     * return that the device is already locked.
     * Besides, because of internal multi-tasking, locking the device by
     * the ?ASSIGN call or an $IEXO flag would cause problem for the task
     * id that did not open the device first.
     */
#else datageneral
    int lck_fil, l4l;
    int pid_buf = getpid();		/* pid to save in lock file */
	
    hasLock = 0;			/* not locked yet */
    l4l = look4lk(ttyfd);
    if (l4l < 0) return (-1);		/* already locked */
    if (l4l == 1) return (0);		/* can't read/write lock directory */
    lck_fil = creat(flfnam, 0444);	/* create lock file ... */
    if (lck_fil < 0) return (-1);	/* create of lockfile failed */
		/* creat leaves file handle open for writing -- hf */
#ifdef ATT3BX
    fprintf((lck_fild = fdopen(lck_fil, "w")), "%10d\n", pid_buf);
    fflush(lck_fild);
#else
    write (lck_fil, &pid_buf, sizeof(pid_buf) ); /* uucp expects int in file */
#endif
    close (lck_fil);
#endif datageneral
    hasLock = 1;			/* now is locked */
    return(0);
}

/*  T T U N L O C K  */

static
ttunlck() {				/* kill uucp lock if possible */
#ifdef datageneral
    /* The DG automatically unlocks the device when it is closed, or when
     * the process is terminated for any reason.  Unlocking is not needed.
     * Also, see the note for ttlock().
     */
    return(0);
#else
    if (hasLock) return( unlink( flfnam ) ); 
#endif
}

/*  T T P K T  --  Condition the communication line for packets. */
/*		or for modem dialing */

#define DIALING	4		/* flags (via flow) for modem handling */
#define CONNECT 5

/*  If called with speed > -1, also set the speed.  */

/*  Returns 0 on success, -1 on failure.  */

ttpkt(speed,flow) int speed, flow; {
    int s;
    if (ttyfd < 0) return(-1);		/* Not open. */


#ifdef KERLD
/* Note, KERLD ignores the TANDEM, ECHO, and CRMOD bits */
    if (kerld) {
	ioctl(ttyfd,TIOCGETD,&oldld);	/* Get line discipline */
	ioctl(ttyfd,TIOCGETC,&oldc);	/* Get special chars */
	newc = oldc;			/* Copy special chars */
	newc.t_brkc = '\r';		/* Set CR to be break character */
	if(ioctl(ttyfd,TIOCSETC,&newc) < 0) return(-1);
    }
#endif

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


#ifdef MYREAD
#ifdef BSD4
/* Try to make reads nonblocking */
    if (kerld == 0) {
	if (fcntl(ttyfd,F_SETFL,fcntl(ttyfd,F_GETFL,0) & FNDELAY) == -1)
    	    return(-1);
	else return(0);
    }
#endif
#endif
#endif

#ifdef UXIII
    if (flow == 1) ttraw.c_iflag |= (IXON|IXOFF);
    if (flow == 0) ttraw.c_iflag &= ~(IXON|IXOFF);

    if (flow == DIALING)  ttraw.c_cflag |= CLOCAL|HUPCL;
    if (flow == CONNECT)  ttraw.c_cflag &= ~CLOCAL;

    ttraw.c_lflag &= ~(ICANON|ECHO);
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|INPCK|ISTRIP|IXANY);
    ttraw.c_oflag &= ~OPOST;
#ifdef datageneral
    ttraw.c_lflag &= ~CSETPM;      /* No DG page mode */
#endif
    ttraw.c_cc[4] = 1;
    ttraw.c_lflag |= ISIG;		/* do check for interrupt */
    ttraw.c_iflag |= (BRKINT|IGNPAR);
    ttraw.c_cflag &= ~(CSIZE|PARENB);
    ttraw.c_cflag |= (CS8|CREAD);
    ttraw.c_cc[5] = 0;

    if (s > -1) ttraw.c_cflag &= ~CBAUD, ttraw.c_cflag |= s; /* set speed */

    if (ioctl(ttyfd,TCSETAW,&ttraw) < 0) return(-1);  /* set new modes . */
#ifdef datageneral
    if (flow == 1) {
         setflow(channel(ttyfdout));
         setflow(channel(ttyfd));
    }
    if (flow == 0) {
         resflow(channel(ttyfdout));
         resflow(channel(ttyfd));
    }
    if (sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttraw)) 
         { perror("ttpkt: gechr: "); return(-1); }
#endif

    if (flow == DIALING) {
#ifdef datageneral
#ifndef aosvs
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0)) < 0 )
		return(-1);
#endif
#else
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0 )
		return(-1);
	close( open(ttnmsv,2) );	/* magic to force mode change!!! */
#endif
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
    if (stty(ttyfd,&tttvt) < 0) return(-1);
#ifdef MYREAD
#ifdef BSD4
/* Make reads nonblocking */
    if (kerld == 0) {
	if (fcntl(ttyfd,F_SETFL,fcntl(ttyfd,F_GETFL,0) & FNDELAY) == -1)
	    return(-1);
	else return(0);
    }
#endif
#endif

    if (flow == 1) tttvt.c_iflag |= (IXON|IXOFF);
    if (flow == 0) tttvt.c_iflag &= ~(IXON|IXOFF);

    if (flow == DIALING)  tttvt.c_cflag |= CLOCAL|HUPCL;
    if (flow == CONNECT)  tttvt.c_cflag &= ~CLOCAL;

    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
    tttvt.c_iflag |= (IGNBRK|IGNPAR);
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|BRKINT|INPCK|ISTRIP|IXANY);
    tttvt.c_oflag &= ~OPOST;
    tttvt.c_cflag &= ~(CSIZE|PARENB);
    tttvt.c_cflag |= (CS8|CREAD);
    tttvt.c_cc[4] = 1;
    tttvt.c_cc[5] = 0;

    if (s > -1) tttvt.c_cflag &= ~CBAUD, tttvt.c_cflag |= s; /* set speed */

    if (ioctl(ttyfd,TCSETAW,&tttvt) < 0) return(-1);  /* set new modes . */

#ifdef datageneral
    if (flow == 1) {
         setflow(channel(ttyfdout));
         setflow(channel(ttyfd));
    }
    if (flow == 0) {
         resflow(channel(ttyfdout));
         resflow(channel(ttyfd));
    }
    if (sys_gechr( channel(ttyfd), (1<<31)|$CLMAX, ch_ttvt)) 
         { perror("ttvt: gechr: "); return(-1); }
#endif

    if (flow == DIALING) {
#ifdef datageneral
#ifndef aosvs
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0)) < 0 )
		return(-1);
#endif
#else
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0 )
		return(-1);
	close( open(ttnmsv,2) );	/* magic to force mode change!!! */
#endif
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
#ifdef PLEXUS
	    case 19200: s = EXTA; break;
#else
#ifdef datageneral
            /* ioctl() allows other speeds */
            case 50:     s = B50; break;
            case 75:     s = B75; break;
            case 134:    s = B134; break;
            case 3600:   s = B3600; break;
            case 7200:   s = B7200; break;
            case 19200:  s = B19000; break;

            /* Some speeds are supported by the hardware thru PMGR.
               We don't bother about 45.5 but it could be added.
            */
            case 38400:  if (setspeed($CR38K)) spdok = 0; s = 0; break;
#endif
#endif
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

#ifndef UXIII
    long n;
#endif
    if (ttyfd < 0) return(-1);		/* Not open. */

    ungotn = -1;			/* Initialize myread() stuff */
    inbufc = 0;

#ifdef UXIII
#ifdef datageneral
    /* Since $ESGT is turned on in the sys_read packets, there is never a need
     * to flush the input buffer -- the system automatically does it each time
     * a read is issued.
     */
#else
    if (ioctl(ttyfd,TCFLSH,0) < 0) perror("flush failed");
#endif
#else
#ifdef TIOCFLUSH
#ifdef ANYBSD
    n = FREAD;				/* Specify read queue */
    if (ioctl(ttyfd,TIOCFLUSH,&n) < 0) perror("flush failed");
#else
    if (ioctl(ttyfd,TIOCFLUSH,0) < 0) perror("flush failed");
#endif
#endif
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

#ifdef V7
esctrp() {				/* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);		/* ignore until trapped */
}
#endif


#ifdef C70
esctrp() {				/* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);		/* ignore until trapped */
}
#endif

/*  C O N I N T  --  Console Interrupt setter  */

conint(f) int (*f)(); {			/* Set an interrupt trap. */

    if (!isatty(0)) return(0);		/* only for real ttys */
    if (backgrd) return;		/* must ignore signals in bkgrd */

/*
 Except for special cases below, ignore keyboard quit signal.
 ^\ too easily confused with connect escape, and besides, we don't want
 to leave lock files around.  (Frank Prindle)
*/
#ifdef datageneral
    /* Set up the asynchronous task to read the console. 
     * But, I don't want them set up all the time, so I will kludge up the
     * call in  chkint() (ckuus3.c) to see if this asynch task is running:
     * conint_mt()
     */
#endif
    signal(SIGQUIT,SIG_IGN);

#ifdef UXIII
    signal(SIGQUIT,esctrp);		/* console escape in pkt modes */
    if (conesc) {			/* clear out pending escapes */
	conesc = 0;
    }
#endif

#ifdef V7
    signal(SIGQUIT,esctrp);		/* console escape in pkt modes */
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
#ifdef V7
	signal(SIGQUIT,SIG_IGN);	/*   must leave signals ignored */
#endif
	return;
    }
    signal(SIGINT,f);			/* Function to trap to on interrupt. */
    signal(SIGHUP,f);			/* Or hangup, so lock file cleared. */
    conif = 1;				/* Flag console interrupts on. */
}


/*  C O N N O I  --  Reset console terminal interrupts */

connoi() {				/* Console-no-interrupts */

    if (!isatty(0)) return(0);		/* only for real ttys */
    if (backgrd) return;		/* Ignore signals in background */

#ifdef datageneral
    /* Terminate the asynchronous task that reads the console for
     * interrupt characters. 
     * But, I don't want to kill this task all the time, so I will kludge 
     * up the call in input() (ckcfn2.c) to kill the asynch task if it
     * is running: connoi_mt()
     */
#endif
    signal(SIGINT,SIG_DFL);
    signal(SIGHUP,SIG_DFL);
    signal(SIGQUIT,SIG_DFL);
    conif = 0;				/* Flag interrupt trapping off */
}

#ifdef datageneral
/* C O N I N T _ M T  -- Asynch read console task */

void conint_mt() {
    /* Any input waiting? */
    con_reads_mt = 1;                   /* Flag that task is active */
    conint_avl = 0;                     /* Flag that interrupt gotten */
    while (con_reads_mt)
        /* Get a char; 
         * then, wait 2 seconds for the system to respond to it. 
         */
        if ((conint_ch = coninc(2)) <= 0) {
            conint_avl = 1;
            sleep(5);
        }
    conint_avl = 0;
}


/* C O N N O I _ M T  -- Kill asynch read console task */

connoi_mt() {
    con_reads_mt = 0;
    conint_avl = 0;
    midkill(CONINT_TSK);
}


/* C O N S T A _ M T  --  Start the console asynch read task */

consta_mt() {
    if ((con_reads_mt == 0)      /* Not already running */
     && (mtask(conint_mt, CONINT_STACK, CONINT_TSK, CONINT_PRI) != 0))
        perror("consta_mt: Can't initiate task:");
}
#endif datageneral

/*  myread() -- For use by systems that can do nonblocking read() calls  */
/*
 Returns:
  -1  if no characters available,
  -2  upon error (such as disconnect),
  otherwise value of character (0 or greater)

*/
myread() {
    static int inbuf_item;
#ifdef datageneral
    static CHAR inbuf[MAXINBUF];
#else
    static CHAR inbuf[257];
#endif
    CHAR readit;
    
    if (ungotn >= 0) {
	readit = ungotn;
    } else {
        if (inbufc > 0) {
	    readit = inbuf[++inbuf_item];
        } else {
#ifdef datageneral
	    if ((inbufc = dgncinb(ttyfd,inbuf,MAXINBUF-1)) == 0) {  
	        /* end of file */
		return(-2);		/* end of file has no errno */
		}
	    if (inbufc < 0) {		/* Real error */
     		/* No data available if == -1.  Unix can never return -1 ?? */
     		if (inbufc == -1) return(-1);
                if ((inbufc == -ERDCT) || (inbufc == -ERCDS))
                     /* Modem disconnected, or in progress of same */
     		    errno = 9999;	/* magic number for no carrier */
#else
	    if ((inbufc = read(ttyfd,inbuf,256)) == 0) {  /* end of file */
			/* means carrier dropped on modem connection */
		errno = 9999;		/* magic number for no carrier */
		return(-2);		/* end of file has no errno */
		}
	    if (inbufc < 0) {		/* Real error */
#endif
    	    	return(-2);
    	    }
	    readit = inbuf[inbuf_item = 0];
	}
        inbufc--;	
    }
    ungotn = -1;
    return(readit);
}

myunrd(ch) CHAR ch; {			/* push back up to one character */
    ungotn = ch;
}

/*  I N I T R A W Q  --  Set up to read /DEV/KMEM for character count.  */

#ifdef	V7
/*
 Used in Version 7 to simulate Berkeley's FIONREAD ioctl call.  This
 eliminates blocking on a read, because we can read /dev/kmem to get the
 number of characters available for raw input.  If your system can't
 or you won't let it read /dev/kmem (the world that is) then you must
 figure out a different way to do the counting of characters available,
 or else replace this by a dummy function that always returns 0.
*/
/*
 * Call this routine as: initrawq(tty)
 * where tty is the file descriptor of a terminal.  It will return
 * (as a char *) the kernel-mode memory address of the rawq character
 * count, which may then be read.  It has the side-effect of flushing
 * input on the terminal.
 */
/*
 * John Mackin, Physiology Dept., University of Sydney (Australia)
 * ...!decvax!mulga!physiol.su.oz!john
 *

 * Permission is hereby granted to do anything with this code, as
 * long as this comment is retained unmodified and no commercial
 * advantage is gained.
 */
#include <a.out.h>
#include <sys/proc.h>

char *initrawq(tty) int tty; {
#ifdef UTS24
    return(0);
#else
#ifdef BSD29
    return(0);
#else
    long lseek();
    static struct nlist nl[] = {
	{PROCNAME},
	{NPROCNAME},
	{""}

    };
    static struct proc *pp;
    char *malloc(), *qaddr, *p, c;
    int m, pid, me;
    NPTYPE xproc;			/* Its type is defined in makefile. */
    int catch();

    me = getpid();
    if ((m = open("/dev/kmem", 0)) < 0) err("kmem");

    nlist(BOOTNAME, nl);
    if (nl[0].n_type == 0) err("proc array");
 
    if (nl[1].n_type == 0) err("nproc");

    lseek(m, (long)(nl[1].n_value), 0);
    read (m, &xproc, sizeof(xproc));
    signal(SIGALRM, catch);
    if ((pid = fork()) == 0) {
	while(1)
	    read(tty, &c, 1);
    }
    alarm(2);
 
    if(setjmp(jjbuf) == 0) {
	while(1)
	    read(tty, &c, 1);
    }
    signal(SIGALRM, SIG_DFL);

#ifdef DIRECT
    pp = (struct proc *) nl[0].n_value;
#else 
    if (lseek(m, (long)(nl[0].n_value), 0) < 0L) err("seek");
    if (read(m, &pp, sizeof(pp)) != sizeof(pp))  err("no read of proc ptr");
#endif
    lseek(m, (long)(nl[1].n_value), 0);
    read(m, &xproc, sizeof(xproc));

    if (lseek(m, (long)pp, 0) < 0L) err("Can't seek to proc");
    if ((p = malloc(xproc * sizeof(struct proc))) == NULL) err("malloc");
    if (read(m,p,xproc * sizeof(struct proc)) != xproc*sizeof(struct proc))
    	err("read proc table");
    for (pp = (struct proc *)p; xproc > 0; --xproc, ++pp) {
	if (pp -> p_pid == (short) pid) goto iout;
    }
    err("no such proc");
 
iout:
    close(m);
    qaddr = (char *)(pp -> p_wchan);

    free (p);
    kill(pid, SIGKILL);
    wait((int *)0);		/* Destroy the ZOMBIEs! */
    return (qaddr);
#endif
#endif
}

/*  More V7-support functions...  */

static
err(s) char *s; {
    char buf[200];

    sprintf(buf, "fatal error in initrawq: %s", s);
    perror(buf);
    doexit(1);
}

static
catch() {
    longjmp(jjbuf, -1);
}


/*  G E N B R K  --  Simulate a modem break.  */

#define	BSPEED	B150

genbrk(fn) int fn; {
    struct sgttyb ttbuf;
    int ret, sospeed;

    ret = ioctl(fn, TIOCGETP, &ttbuf);
    sospeed = ttbuf.sg_ospeed;
    ttbuf.sg_ospeed = BSPEED;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    ret = write(fn, "\0\0\0\0\0\0\0\0\0\0\0\0", 8);
    ttbuf.sg_ospeed = sospeed;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    ret = write(fn, "@", 1);

    return;
}
#endif	V7

/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

ttchk() {
    int x; long n;
#ifdef datageneral
    /* FIONREAD is defined, so exit here */
    return(inbufc + (ungotn >= 0) );	
#endif
#ifdef FIONREAD
    x = ioctl(ttyfd, FIONREAD, &n);	/* Berkeley and maybe some others */
    debug(F101,"ttchk","",n);
    return((x < 0) ? 0 : n);
#else
#ifdef	V7
    lseek(kmem[TTY], (long) qaddr[TTY], 0);
    x = read(kmem[TTY], &n, sizeof(int));
    return((x == sizeof(int))? n: 0);
#else	V7
#ifdef UXIII
    return(inbufc + (ungotn >= 0) );	
#else
#ifdef C70
    return(inbufc + (ungotn >= 0) );
#else
#ifdef PROVX1
    x = ioctl(ttyfd, TIOCQCNT, &ttbuf);
    n = ttbuf.sg_ispeed & 0377;
    return((x < 0) ? 0 : n);
#else
    return(0);
#endif
#endif

#endif
#endif
#endif
}


/*  T T X I N  --  Get n characters from tty input buffer  */

/*  Returns number of characters actually gotten, or -1 on failure  */

/*  Intended for use only when it is known that n characters are actually */
/*  Available in the input buffer.  */

ttxin(n,buf) int n; char *buf; {
    int x;
    CHAR c;

#ifdef MYREAD
    for( x = 0; (x > -1) && (x < n); buf[x++] = myread() );
#else
    debug(F101,"ttxin: n","",n);
    x = read(ttyfd,buf,n);
    debug(F101," x","",x);
#endif
    if (x > 0) buf[x] = '\0';
    if (x < 0) x = -1;
    return(x);
}

/*  T T O L  --  Similar to "ttinl", but for writing.  */

ttol(s,n) int n; char *s; {
    int x;
#ifdef datageneral
    if (ttyfdout < 0) return(-1);	/* Not open. */
    x = dgncoub(ttyfdout,s,n); 
#else
    if (ttyfd < 0) return(-1);		/* Not open. */
    x = write(ttyfd,s,n);
#endif
    debug(F111,"ttol",s,n);
    if (x < 0) debug(F101,"ttol failed","",x);
    return(x);
}


/*  T T O C  --  Output a character to the communication line  */

ttoc(c) char c; {
#ifdef datageneral
    if (ttyfdout < 0) return(-1);	/* Not open. */
    return(dgncoub(ttyfdout,&c,1)); 
#else
    if (ttyfd < 0) return(-1);		/* Not open. */
    return(write(ttyfd,&c,1));
#endif
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

ttinl(dest,max,timo,eol) int max,timo; char *dest; char eol; {
    int x, y;
    CHAR c;

    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed read... */

#ifdef MYREAD
	for (x = c = 0; (x < max) && (c != eol); x++) {
	     while ((y = myread()) == -1) ;
	     if (y == -2) return(-1);
	     dest[x] = c = y & 0377;
	}
#else
	x = read(ttyfd,dest,max);	/* Try to read. */
#endif
	return(x);			/* Return the count. */
    }

/* Timed read... */

#ifdef datageneral
     /* Peform timeout setting only once per device */
     if (timotty != timo) { 
          resto(channel(ttyfd));        /* Save previous settings */
          setto(channel(ttyfd),timo);   /* Set the new timeout values */
     }
     {
#else
    signal(SIGALRM,timerh);		/* Set up timeout action. */
    alarm(timo);			/* Set the timer. */
    if (setjmp(sjbuf))			/* Do this if timer went off. */
    	x = -1;
    else if (kerld) {			/* Efficient Kermit line discipline */
	x = read(ttyfd,dest,max);	/* for 4.2bsd only... */
    } else {				/* Normal case... */
#endif    	
	for (x = c = y = 0; (x < max) && (c != eol); x++) {
#ifdef MYREAD
    	    while ((y = myread()) == -1) /* Use own buffering if we can */
	    	;
	    if (y == -2) y++;
	    c = y & 0377;
#else
	    while ((y = read(ttyfd,&c,1)) == 0) /* Else call system */
	    	;			/* ...for each character. */
#endif MYREAD
	    if (y < 0) {
#ifdef datageneral
               resto(channel(ttyfd));
#else
	       alarm(0);		/* Error, turn off timer, */
   	       signal(SIGALRM,SIG_DFL); /* and associated interrupt. */
#endif
	       return(y);		/* Return the error indication. */
	    }
    	    dest[x] = c;
	}
	x++;
    }

#ifndef datageneral
    alarm(0);				/* Success, turn off timer, */
    signal(SIGALRM,SIG_DFL);		/* and associated interrupt. */
#endif
    return(x);				/* Return the count. */
}

/*  T T I N C --  Read a character from the communication line  */

ttinc(timo) int timo; {
    int n = 0;
    CHAR ch = 0;

    if (ttyfd < 0) return(-1);		/* Not open. */
    if (timo <= 0) {			/* Untimed. */
#ifdef MYREAD
    	/* comm line failure returns -1 thru myread, so no &= 0377 */
    	while ((n = myread()) == -1) ;	/* Wait for a character... */
	if (n == -2) n++;
	return( n );
#else
	while ((n = read(ttyfd,&ch,1)) == 0) ; /* Wait for a character. */
	return( (n > 0) ? (ch & 0377) : n );
#endif
    }

#ifdef datageneral
    /* Peform timeout setting only once per device */
    if (timotty != timo) { 
         resto(channel(ttyfd));         /* Save previous settings */
         setto(channel(ttyfd),timo);    /* Set the new timeout value */
    }
    {
#else
    signal(SIGALRM,timerh);		/* Timed, set up timer. */
    alarm(timo);
    if (setjmp(sjbuf))
	n = -1;
    else {
#endif
#ifdef MYREAD
    	while ((n = myread()) == -1) ;	/* If managing own buffer... */
	if (n == -2) {
	    n++;
	} else {
	    ch = n;
	    n = 1;	
	}
#else
    	n = read(ttyfd,&ch,1);		/* Otherwise call the system. */
#endif
    }
#ifndef datageneral
    alarm(0);				/* Turn off timer, */
    signal(SIGALRM,SIG_DFL);		/* and interrupt. */
#endif
    return( (n > 0) ? (ch & 0377) : n ); /* Return char or -1. */
}

/*  T T S N D B  --  Send a BREAK signal  */

ttsndb() {
    int x; long n; char spd;

    if (ttyfd < 0) return(-1);		/* Not open. */

#ifdef PROVX1
    gtty(ttyfd,&ttbuf);			/* Get current tty flags */
    spd = ttbuf.sg_ospeed;		/* Save speed */
    ttbuf.sg_ospeed = B50;		/* Change to 50 baud */
    stty(ttyfd,&ttbuf);			/*  ... */
    write(ttyfd,brnuls,3);		/* Send 3 nulls */
    ttbuf.sg_ospeed = spd;		/* Restore speed */
    stty(ttyfd,&ttbuf);			/*  ... */
    return(0);
#else
#ifdef UXIII
#ifdef datageneral
    if (sys_clrdv(channel(ttyfdout),1<<31,$CDSBRK)) {   /* Send a BREAK */
#else
    if (ioctl(ttyfd,TCSBRK,(char *)0) < 0) {	        /* Send a BREAK */
#endif
    	perror("Can't send BREAK");
	return(-1);
    }
    return(0);
#else
#ifdef ANYBSD
    n = FWRITE;				/* Flush output queue. */
    ioctl(ttyfd,TIOCFLUSH,&n); 		/* Ignore any errors.. */
    if (ioctl(ttyfd,TIOCSBRK,(char *)0) < 0) {	/* Turn on BREAK */
    	perror("Can't send BREAK");
	return(-1);
    }
    x = msleep(275);			/* Sleep for so many milliseconds */
    if (ioctl(ttyfd,TIOCCBRK,(char *)0) < 0) {	/* Turn off BREAK */
	perror("BREAK stuck!!!");
	doexit(1);			/* Get out, closing the line. */
					/*   with exit status = 1 */
    }
    return(x);
#else
#ifdef	V7
    genbrk(ttyfd);			/* Simulate a BREAK */
    return(x);
#endif
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
    if (m <= 0) return(0);
    sleep(-((m * 60 + 500) / 1000));

    return(0);
#endif

#ifdef ANYBSD
    int t1, t3, t4;
    if (m <= 0) return(0);
#ifndef BSD42
/* 2.9 and 4.1 BSD do it this way */
    if (ftime(&ftp) < 0) return(-1);	/* Get current time. */
    t1 = ((ftp.time & 0xff) * 1000) + ftp.millitm;
    while (1) {
	ftime(&ftp);			/* new time */
	t3 = (((ftp.time & 0xff) * 1000) + ftp.millitm) - t1;
	if (t3 > m) return (t3);
    }
#else
/* 4.2 & above can do it with select()... */
    if (gettimeofday(&tv, &tz) < 0) return(-1); /* Get current time. */
    t1 = tv.tv_sec;			/* Seconds */

    tv.tv_sec = 0;			/* Use select() */
    tv.tv_usec = m * 1000;
    return(select( 0, (int *)0, (int *)0, (int *)0, &tv) );
#endif
#endif

#ifdef UXIII
#ifdef XENIX
#define CLOCK_TICK 50			/* millisecs per clock tick */
#else
#define CLOCK_TICK 17			/* 1/60 sec */
#endif
    extern long times();
    long t1, t2, tarray[4];
    int t3;

    if (m <= 0) return(0);
    if ((t1 = times(tarray)) < 0) return(-1);
    while (1) {
	if ((t2 = times(tarray)) < 0) return(-1);
	t3 = ((int)(t2 - t1)) * CLOCK_TICK;
	if (t3 > m) return(t3);
    }
#endif


#ifdef TOWER1
    int t1, t3;
    if (m <= 0) return(0);
    if (ftime(&ftp) < 0) return(-1);		/* Get current time. */
    t1 = ((ftp.time & 0xff) * 1000) + ftp.millitm;
    while (1) {
	ftime(&ftp);				/* new time */
	t3 = (((ftp.time & 0xff) * 1000) + ftp.millitm) - t1;
	if (t3 > m) return (t3);
    }
#endif
}

/*  R T I M E R --  Reset elapsed time counter  */

rtimer() {
    tcount = time( (long *) 0 );
}


/*  G T I M E R --  Get current value of elapsed time counter in seconds  */

gtimer() {
    int x;
    x = (int) (time( (long *) 0 ) - tcount);
    rtimer();
    return( (x < 0) ? 0 : x );
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

#ifdef ANYBSD
    char *asctime();			/* Berkeley way */
    struct tm *localtime();
    struct tm *tp;
#ifdef BSD42
    gettimeofday(&tv, &tz);		/* BSD 4.2 */
    time(&tv.tv_sec);
    tp = localtime(&tv.tv_sec);
#else
    time(&clock);			/* BSD 4.1, 2.9 ... ceb */
    tp = localtime(&clock);
#endif
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
#ifdef V7
    char *asctime();			/* V7 way */
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
    if (!isatty(0)) return(0);		/* only for real ttys */
#ifndef UXIII
     gtty(0,&ccold);			/* Structure for restoring */
     gtty(0,&cccbrk);			/* For setting CBREAK mode */
     gtty(0,&ccraw);			/* For setting RAW mode */
#else
     ioctl(0,TCGETA,&ccold);
     ioctl(0,TCGETA,&cccbrk);
     ioctl(0,TCGETA,&ccraw);
#ifdef datageneral
     /* For DG, also get the tty extended device characteristics. */
     sys_gechr( channel(0), (1<<31)|$CLMAX, ch_ccold);
     sys_gechr( channel(0), (1<<31)|$CLMAX, ch_ccraw);
     sys_gechr( channel(0), (1<<31)|$CLMAX, ch_ccbrk);
#endif
#endif
     cgmf = 1;				/* Flag that we got them. */
}



/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

concb(esc) char esc; {
    int x;
    if (!isatty(0)) return(0);		/* only for real ttys */
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifndef UXIII
    cccbrk.sg_flags |= CBREAK;		/* Set to character wakeup, */
    cccbrk.sg_flags &= ~ECHO;		/* no echo. */
    x = stty(0,&cccbrk);
#else
#ifdef datageneral
    cccbrk.c_lflag |= CBREAK;
#endif
    cccbrk.c_lflag &= ~(ICANON|ECHO);
    cccbrk.c_cc[0] = 003;		/* interrupt char is control-c */
    cccbrk.c_cc[1] = escchr;		/* escape during packet modes */
    cccbrk.c_cc[4] = 1;
    cccbrk.c_cc[5] = 1;
    x = ioctl(0,TCSETAW,&cccbrk);  	/* set new modes . */
#ifdef datageneral
    if (sys_gechr( channel(0), (1<<31)|$CLMAX, ch_ccbrk)) 
         { perror("concb: gechr: "); return(-1); }
#endif
#endif
    if (x > -1) setbuf(stdout,NULL);	/* Make console unbuffered. */
#ifdef	V7
    if (kmem[CON] < 0) {
	qaddr[CON] = initrawq(0);
	if((kmem[CON] = open("/dev/kmem", 0)) < 0) {
	    fprintf(stderr, "Can't read /dev/kmem in concb.\n");
	    perror("/dev/kmem");
	    exit(1);
	}
    }
#endif	V7
    return(x);
}

/*  C O N B I N  --  Put console in binary mode  */

/*  Returns 0 if ok, -1 if not  */

conbin(esc) char esc; {
int filedes,filedes2;

    if (!isatty(0)) return(0);		/* only for real ttys */
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
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF
			|INPCK|ISTRIP);
    ccraw.c_oflag &= ~OPOST;

/*** Kermit used to put the console in 8-bit raw mode, but some users have
 *** pointed out that this should not be done, since some sites actually
 *** use terminals with parity settings on their Unix systems, and if we
 *** override the current settings and stop doing parity, then their terminals
 *** will display blotches for characters whose parity is wrong.  Therefore,
 *** the following two lines are commented out (Larry Afrin, Clemson U):
 ***
 ***   ccraw.c_cflag &= ~(PARENB|CSIZE);
 ***   ccraw.c_cflag |= (CS8|CREAD);
 ***
 *** Sys III/V sites that have trouble with this can restore these lines.
 ***/
    ccraw.c_cc[4] = 1;
    ccraw.c_cc[5] = 1;
#ifdef datageneral
    if (ioctl(0,TCSETAW,&ccraw)) return(-1);  	/* set new modes . */
    if (sys_gechr( channel(0), (1<<31)|$CLMAX, ch_ccraw))
         { perror("conbin: gechr: "); return(-1); }
    return(0);
#else
    return(ioctl(0,TCSETAW,&ccraw) );  	/* set new modes . */
#endif
#endif
}


/*  C O N R E S  --  Restore the console terminal  */

conres() {
    if (cgmf == 0) return(0);		/* Don't do anything if modes */
    if (!isatty(0)) return(0);		/* only for real ttys */
#ifdef datageneral
    if (timocon) 
         /* Timeouts are set, so un-set them. */
         resto(channel(0));
#endif
#ifndef UXIII				/* except for sIII, */
    sleep(1);				/*  not known! */
#endif					/*   (sIII does wait in ioctls) */
    ckxech = 0;				/* System should echo chars */
#ifndef UXIII
    return(stty(0,&ccold));		/* Restore controlling tty */
#else
#ifdef datageneral
    if (ioctl(0,TCSETAW,&ccold)) return(-1);  	/* set new modes . */
    if (sys_sechr( channel(0), (1<<31)|$CLMAX, ch_ccold))
         { perror("conres: sechr "); return(-1); }
    return(0);
#else
    return(ioctl(0,TCSETAW,&ccold));
#endif
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


/*  C O N C H K  --  Return how many characters available at console  */

conchk() {
    int x; long n;

#ifdef PROVX1 
    x = ioctl(0, TIOCQCNT, &ttbuf);
    n = ttbuf.sg_ispeed & 0377;
    return((x < 0) ? 0 : n);
#else
#ifdef V7
    lseek(kmem[CON], (long) qaddr[CON], 0);
    x = read(kmem[CON], &n, sizeof(int));
    return((x == sizeof(int))? n: 0);
#else
#ifdef UXIII
    if (conesc) {			/* Escape typed */
	conesc = 0;
	signal(SIGQUIT,esctrp);		/* Restore escape */
	return(1);
    }
    return(0);
#else
#ifdef C70
    if (conesc) {			/* Escape typed */
	conesc = 0;
	signal(SIGQUIT,esctrp);		/* Restore escape */
	return(1);
    }
    return(0);
#else
#ifdef FIONREAD
    x = ioctl(0, FIONREAD, &n);		/* BSD and maybe some others */
    return((x < 0) ? 0 : n);
#else
    return(0);				/* Others can't do. */
#endif
#endif
#endif
#endif
#endif
}

#ifdef datageneral

/* D G N C O U B -- Output len characters to the file number filenum */

dgncoub(filenum,chs,len) int filenum, len; char *chs; 
{
     int ac2,err;

     if (len == 0) return(0);

     if (filenum == ttyfdout) {
          xout_parms.ibad = chs;
          xout_parms.ircl = len;
          ac2 = &xout_parms;
     } else {
	  w_io_parms.ich = channel(filenum);
          w_io_parms.ibad = chs;
          w_io_parms.ircl = len;
          ac2 = &w_io_parms;
     }
     
     if ((err = sys_write(ac2)) == 0) return(0);
     if ( err != ERLTL && err != EREOF ) {
    	  perror("dgncoub: sys_write ");
          return(err);
     }
}


/* D G N C I N B -- System level read of len characters from file number 
   fileno.  The global DG i/o structures are used for this.

   Return codes:  irlr if data read ok, where irlr=length of data
                  -1 if EOF
                  -2 if timeout occured
                  -err if an AOS/VS error occured, where err is errno
*/                  

dgncinb(filenum,chs,len) int filenum, len; char *chs; 
{
     int ac2,                       /* I/O parameter address block */
         err,                       /* Error from sys_read */
         irlr;                      /* Number of bytes read */
     int rbe;                       /* Ring buffer empty on ESGT read */

     if (filenum == ttyfd) {
          x_io_parms.ibad = chs;
          x_io_parms.ircl = len;
          ac2 = &x_io_parms;
          err = sys_read(ac2);
          irlr = x_io_parms.irlr;
          if (err == 0) return(irlr);
          rbe = (x_io_parms.etsp == 0) ? 0 : (x_io_scrn.esfc & $ESBE);
     } else {
          r_io_parms.ich  = channel(filenum); 
          r_io_parms.ibad = chs;
          r_io_parms.ircl = len;
          ac2 = &r_io_parms;
          err = sys_read(ac2);
          irlr = r_io_parms.irlr;
          if (err == 0) return(irlr);
          rbe = (r_io_parms.etsp == 0) ? 0 : (r_io_scrn.esfc & $ESBE);
     }

     if ((err != ERLTL) && (err != ERDTO) && (err != EREOF)) {
          /* NOT line-too-long, timeout, or EOF errors.
           * Don't print error when it is a data overrun.
           */
    	  if (err != EROVR) perror("dgncinb: sys_read ");
          return(-err);
     } else {
          /* If the system read buffer was empty when a screen management
          * packet was supplied, then there is a virtual EOF condition.  
          * So we return an EOF flag in that case (rbe != 0).
          */
          if ((err == EREOF) || rbe) return(-1);            /* EOF */
          /* We got some data, even though we may have timed out getting it. */
          if (irlr > 0) return(irlr);                       /* Got data */
          if (err == ERDTO) return(-2);                     /* Timeout */
     }
}
#endif datageneral

/*  C O N I N C  --  Get a character from the console  */

coninc(timo) int timo; {
    int n = 0; char ch;
    if (timo <= 0 ) {			/* untimed */
#ifdef datageneral
        /* Binary input */
        n = dgncinb(0,&ch,1);
#else
	n = read(0, &ch, 1);		/* Read a character. */
#endif
	ch &= 0377;
	if (n > 0) return(ch); 		/* Return the char if read */
	else 
#ifdef UXIII
#ifdef datageneral
	    if (n != -2 && n != 0 && errno == EINTR)
	         { perror("coninc: timo=0: "); return(escchr); }
#else
	    if (n < 0 && errno == EINTR) /* if read was interrupted by QUIT */
		return(escchr);		 /* user entered escape character */
#endif datageneral
	    else		    /* couldnt be ^c, sigint never returns */
#endif UXIII
		return(-1);  		/* Return the char, or -1. */
	}
#ifdef datageneral
    /* Peform timeout setting only once per device */
    if (timocon != timo) { 
         resto(channel(0));             /* Save the previous values */
         setto(channel(0),timo);        /* Set the new timeout value */
    }
    n = dgncinb(0,&ch,1);
    ch &= 0377;
#else
    signal(SIGALRM,timerh);		/* Timed read, so set up timer */
    alarm(timo);
    if (setjmp(sjbuf)) n = -2;
    else {
	n = read(0, &ch, 1);
	ch &= 0377;
    }
    alarm(0);				/* Stop timing, we got our character */
    signal(SIGALRM,SIG_DFL);
#endif
    if (n > 0) return(ch);  
    else
#ifdef UXIII
#ifdef datageneral
	if (n != -2 && n != 0 && errno == EINTR)
	     { perror("coninc: timo>0: "); return(escchr); }
#else
        if (n == -1 && errno == EINTR)  /* If read interrupted by QUIT, */
	    return(escchr);		/* user entered escape character, */
#endif datageneral
        else		    		/* can't be ^c, sigint never returns */
#endif UXIII
	return(-1);
}
