char *ckxv = "VMS tty I/O, 1.0(013), 14 Sep 87";

/*  C K V T I O  --  Terminal and Interrupt Functions for VAX/VMS  */

/* Edit History
 * 013 14 Sep 87 FdC    Add parity strip to ttinl(), add syscleanup().
 * 012 11 Jul 85 FdC    Add gtimer(), rtimer() for timing statistics.
 * 011  5 Jul 85 DS     Treat hangup of closed line as success.
 * 010 25 Jun 85 MM     Added sysinit() to open console.
 * 009 18 Jun 85 FdC    Move def of CTTNAM to ckcdeb.h so it can be shared.
 * 008 11 Jun 85 MM     Fix definition of CTTNAM
 *
 * 007 16-May-85 FdC	Changed calling convention of ttopen(), make it
 *  	    	    	set value of its argument "lcl", to tell whether
 *                    	C-Kermit is in local or remote mode.
 *
 * 006  8-May-85 MM	Got rid of "typeahead buffer" code as it didn't
 *			solve the problem of data overruns at 4800 Baud.
 *			Added vms "read a char" routine that checks for
 *			CTRL/C, CTRL/Z, etc.
 */

/* C-Kermit interrupt, terminal control & i/o functions for VMS systems */

/*  S. Rubenstein, Harvard University Chemical Labs  */
/*  (c) 1985 President and Fellows of Harvard College  */

char *ckxsys = " Vax/VMS";


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

   backgrd -- Flag indicating program executing in background ( & on 
		end of shell command). Used to ignore INT and QUIT signals.
   vms_status -- status returned by most recent system service
		which may be used for error reporting.

 Functions for assigned communication line (either external or console tty):

   ttopen(ttname,local,mdmtyp) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttpkt(speed,flow)       -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
				or in DIALING or CONNECTED modem control state.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
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
   conola(s) -- Unbuffered output, array of lines to the console, CRLFs added.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console (bsd 4.2).
		Check if escape char (^\) typed at console (System III/V).
   coninc(timo)  -- Timed get a character from the console.
   conint()  -- Enable terminal interrupts on the console if not background.
   connoi()  -- Disable terminal interrupts on the console if not background.
   contti()  -- Get a character from either console or tty, whichever is first.

Time functions

   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
   rtimer()  -- Reset elapsed time counter
   gtimer()  -- Get elapsed time
*/


/* Includes */
#include "ckcker.h"
#include <stdio.h>			/* Unix Standard i/o */
#include <signal.h>			/* Interrupts */
#include <setjmp.h>			/* Longjumps */
#include <iodef.h>
#include <ttdef.h>
#include <ssdef.h>
#include <descrip.h>
#include <dvidef.h>
#include <dcdef.h>
#include "ckcdeb.h"			/* Formats for debug() */


/* Declarations */

    long time();			/* Get current time in secs */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

    char *dftty = "TT:";
    int dfloc = 0;			/* Default location is remote */
    int dfprty = 0;			/* Parity (0 = none) */
    int ttprty = 0;			/* Parity in use */
    int dfflow = 1;			/* Xon/Xoff flow control */
    int batch = 0;			/* Assume interactive */

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

int vms_status;		/* Used for system service return status */

/* Structures used within this module */

struct tt_mode {
    char class, type;
    short width;
    int basic : 24;
    char length;
    long extended;
};

struct iosb_struct {
    short status, size, terminator, termsize;
};

/* Declarations of variables global within this module */

static int conif = 0,			/* Console interrupts on/off flag */
    cgmf = 0,				/* Flag that console modes saved */
    ttychn = 0,				/* TTY i/o channe; */
    conchn = 0,				/* Console i/o channel */
    con_queued = 0,			/* console i/o queued in contti() */
    tt_queued = 0,			/* tty i/o queued in contti() */
    conch,				/* console input character buffer  */
    ttch;				/* tty input character buffer */
static struct iosb_struct coniosb, ttiosb;
static char escchr;			/* Escape or attn character */
static struct tt_mode
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */

static long tcount;			/* For timing statistics */

/*  Event flags used for I/O completion testing  */
#define CON_EFN 1
#define TTY_EFN 2
#define TIM_EFN 3
#define SUCCESS(x) (((vms_status = (x)) & 7) == 1)
#define CHECK_ERR(s,x) (SUCCESS(x) ? 1 : print_msg(s))

#if 0
/* These aren't used */
static int inbufc = 0;			/* stuff for efficient raw line */
static int ungotn = -1;			/* pushback to unread character */
#endif


/*  P R I N T _ M S G  --  Log an error message from VMS  */

print_msg(s) char *s; {
    struct dsc$descriptor_s b;
    short blen;
    char buf[80], msg[120];

    b.dsc$w_length  = sizeof buf;
    b.dsc$b_dtype   = DSC$K_DTYPE_T;
    b.dsc$b_class   = DSC$K_CLASS_S;
    b.dsc$a_pointer = buf;
    SYS$GETMSG(vms_status, &blen, &b, 0, 0);
    buf[blen] = '\0';
    sprintf(msg, "%s: %s\n", s, buf);
    ermsg(msg);
}

/*  S Y S I N I T  --  System-dependent program initialization.  */

sysinit() {
    if (conchn == 0)
	   conchn = vms_assign_channel("SYS$INPUT:");
    return(0);
}

/*  S Y S C L E A NU P -- System-dependent program epilog.  */

syscleanup() {
    return(0);
}



/*  T T O P E N  --  Open a tty for exclusive access.  */

/*  Returns 0 on success, -1 on failure.  */

ttopen(ttname,lcl,modem) char *ttname; int *lcl, modem; {

    if (ttychn != 0) return(0);		/* If already open, ignore this call */
    ttychn = vms_assign_channel(ttname);
    if (ttychn == 0) return(-1);
    if (!CHECK_ERR("ttopen: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SENSEMODE, 0, 0, 0,
		 &ttold, sizeof ttold, 0, 0, 0, 0))) return(-1);

/* Tell whether local or remote */

    *lcl = (strcmp(ttname,CTTNAM) == 0) ? 0 : 1;

    tttvt = ttold;
    ttraw = ttold;
    /*
     * Possibly add "purge I/O" call here.
     */
    return(0);
}


vms_assign_channel(ttname) char *ttname;  {
    int channel;
    struct dsc$descriptor_s d;

    d.dsc$w_length  = strlen(ttname);
    d.dsc$a_pointer = ttname;
    d.dsc$b_class   = DSC$K_CLASS_S;
    d.dsc$b_dtype   = DSC$K_DTYPE_T;
    if (!CHECK_ERR("vms_assign_channel: SYS$ASSIGN",
	SYS$ASSIGN(&d, &channel, 0, 0))) return(0);
    else return(channel & 0xFFFF);
}


/*  T T C L O S  --  Close the TTY, releasing any lock.  */

ttclos() {
    if (ttychn == 0) return(0);		/* Wasn't open. */
    ttres();				/* Reset modes. */
    if (!CHECK_ERR("ttclos: SYS$DASSGN",
	SYS$DASSGN(ttychn))) return(-1);
    ttychn = 0;				/* Mark it as closed. */
    return(0);
}


/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {				/* Restore the tty to normal. */
    if (ttychn == 0) return(-1);		/* Not open. */
    sleep(1);				/* Wait for pending i/o to finish. */
    debug(F101,"ttres, ttychn","",ttychn);
    if (!CHECK_ERR("ttres: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SETMODE, 0, 0, 0,
    		 &ttold, sizeof ttold, 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  T T P K T  --  Condition the communication line for packets. */
/*		or for modem dialing */

#define DIALING	4		/* flags (via flow) for modem handling */
#define CONNECT 5

/*  If called with speed > -1, also set the speed.  */

/*  Returns 0 on success, -1 on failure.  */

ttpkt(speed,flow) int speed, flow; {
    extern char ttname[];
    int s;
    if (ttychn == 0) return(-1);		/* Not open. */
    s = ttsspd(speed);			/* Check the speed */
#ifdef TT2$M_PASTHRU
    ttraw.extended |= TT2$M_PASTHRU;
#else
    ttraw.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    ttraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ttraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    ttraw.basic &= ~TT$M_WRAP & ~TT$M_HOSTSYNC & ~TT$M_TTSYNC;
    if (!CHECK_ERR("ttpkt: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SETMODE, 0, 0, 0,
    	    	 &ttraw, sizeof ttraw, s, 0, 0, 0))) return(-1);
    ttflui();				/* Flush any pending input */
    return(0);
}


/*  T T V T -- Condition communication line for use as virtual terminal  */

ttvt(speed,flow) int speed, flow; {
    extern char ttname[];
    int s;
    if (ttychn == 0) return(-1);		/* Not open. */

    s = ttsspd(speed);			/* Check the speed */
#ifdef TT2$M_PASTHRU
    ttraw.extended |= TT2$M_PASTHRU;
#else
    ttraw.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    ttraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ttraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT | TT$M_HOSTSYNC | TT$M_TTSYNC;
    ttraw.basic &= ~TT$M_WRAP;
    if (!CHECK_ERR("ttvt: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SETMODE, 0, 0, 0,
			  &ttraw, sizeof ttraw, s, 0, 0, 0))) return(-1);
}


/*  T T S S P D  --  Return the internal baud rate code for 'speed'.  */

int speeds[] = {
	110, 150, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 0 } ;

static int speedcodes[] = {
	TT$C_BAUD_110,  TT$C_BAUD_150,  TT$C_BAUD_300,  TT$C_BAUD_600,
	TT$C_BAUD_1200, TT$C_BAUD_1800, TT$C_BAUD_2400, TT$C_BAUD_4800,
	TT$C_BAUD_9600, TT$C_BAUD_19200 } ;

int
ttsspd(speed) int speed; {
    int s;
    char msg[50];

    if (speed < 0)			/* 006 Unknown speed fails	*/
	return (0);
    for (s = 0;  speeds[s] != 0 && speeds[s] != speed;  s++) ;
    if (speeds[s] != 0)
	return(speedcodes[s]);
    else {
	sprintf(msg,"Unsupported line speed - %d\n",speed);
	ermsg(msg);
	ermsg("Current speed not changed\n");
	return(-1);
    }
}



/*  T T F L U I  --  Flush tty input buffer */

ttflui() {

    long n;
    if (ttychn == 0) return(-1);		/* Not open. */

    if (!CHECK_ERR("ttflui: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_READVBLK|IO$M_TIMED|IO$M_PURGE,
		 0, 0, 0, &n, 0, 0, 0, 0, 0))) perror("flush failed");

#if 0
    /* Note: unused */
    inbufc = 0;
    ungotn = -1;
#endif
    return(0);
}


/* Interrupt Functions */


/*  C O N I N T  --  Console Interrupt setter  */

conint(f) int (*f)(); {			/* Set an interrupt trap. */

    if (batch) return;		/* must ignore signals in bkgrd */

    if (conif) return;			/* Nothing to do if already on. */

/* check if invoked in background -- if so signals set to be ignored */

    if (signal(SIGINT,SIG_IGN) == SIG_IGN) {
	batch = 1;			/*   means running in background */
	return;
    }
    signal(SIGINT,f);			/* Function to trap to. */
    conif = 1;				/* Flag console interrupts on. */
}


/*  C O N N O I  --  Reset console terminal interrupts */

connoi() {				/* Console-no-interrupts */

    if (batch) return;			/* must ignore signals in bkgrd */

    signal(SIGINT,SIG_DFL);
    conif = 0;
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

ttchk() {
    struct { short count;  char first;  char reserved1;  long reserved2; } t;
    return (/* inbufc + (ungotn >= 0) +		-- 006 Note: unused	*/
	(CHECK_ERR("ttchk: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SENSEMODE|IO$M_TYPEAHDCNT, 0, 0, 0,
			  &t, sizeof t, 0, 0, 0, 0)) ? t.count : 0));
}


/*  T T X I N  --  Get n characters from tty input buffer  */

ttxin(n,buf) int n; char *buf; {
    int x;
    long trmmsk[2];

    trmmsk[0] = 0;
    trmmsk[1] = 0;
    if (!CHECK_ERR("ttxin: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_READVBLK, &ttiosb, 0, 0,
			  buf, n, 0, trmmsk, 0, 0))
	|| !CHECK_ERR("ttxin: ttiosb.status",ttiosb.status)) return(-1);
    buf[n] = '\0';
    return(ttiosb.size);
}


/*  T T O L  --  Similar to "ttinl", but for writing.  */
/*
 * This probably should be buffered with a flush command added.
 */

ttol(s,n) int n; char *s; {
    int x;
    if (ttychn == 0) return(-1);		/* Not open. */
    if (CHECK_ERR("ttol: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_WRITEVBLK, 0, 0, 0,
    	s, n, 0, 0, 0, 0))) x = 0;
    else x = -1;
    debug(F111,"ttol",s,n);
    if (x < 0) debug(F101,"ttol failed","",x);
    return(x);
}


/*  T T O C  --  Output a character to the communication line  */

ttoc(c) char c; {
    if (ttychn == 0) return(-1);		/* Not open. */
    if (CHECK_ERR("ttoc: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_WRITEVBLK, 0, 0, 0,
    	&c, 1, 0, 0, 0, 0))) return(0);
    else return(-1);
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
/*** WARNING: THIS MIGHT NOT WORK ANY MORE, BECAUSE IT IS NOW THIS ***/
/*** FUNCTION'S JOB TO STRIP PARITY.  I PUT IN SOME CODE THAT LOOKS LIKE ***/
/*** IT SHOULD DO IT, BUT IT'S NOT TESTED.  - Frank, C-Kermit 4E ***/

ttinl(dest,max,timo,eol) int max,timo; char *dest; {
    int x, y, c, i;
    int trmmsk[2], func;

    if (ttychn == 0) return(-1);		/* Not open. */
    trmmsk[0] = 0;
    trmmsk[1] = 1 << eol;		/* Assumes eol is a control char */
    func = IO$_READVBLK;
    if (timo > 0) func |= IO$M_TIMED;
    if (!CHECK_ERR("ttinl: SYS$QIOW",
	SYS$QIOW(0, ttychn, func, &ttiosb, 0, 0,
			  dest, max, timo, trmmsk, 0, 0))
	|| ttiosb.status == SS$_TIMEOUT	/* Check separately so no err msg */
	|| !CHECK_ERR("ttinl: ttiosb.status",ttiosb.status)) return(-1);

    if (ttprty) {
        for (i = 0; i < ttiosb.size; i++) dest[i] &= 0177; /* strip parity */
    }
    return(ttiosb.size);
}


/*  T T I N C --  Read a character from the communication line  */

ttinc(timo) int timo; {
    int n;
    CHAR ch;

    int trmmsk[2], func;

    if (ttychn == 0) return(-1);		/* Not open. */
    trmmsk[0] = 0;
    trmmsk[1] = 0;
    func = IO$_READVBLK;
    if (timo > 0) func |= IO$M_TIMED;
    if (!CHECK_ERR("ttinc: SYS$QIOW",
	SYS$QIOW(0, ttychn, func, &ttiosb, 0, 0,
			  &ch, 1, timo, trmmsk, 0, 0))
	|| ttiosb.status == SS$_TIMEOUT	/* Check separately so no err msg */
	|| !CHECK_ERR("ttinc: ttiosb.status",ttiosb.status)) return(-1);
    return(ch & 0377);
}


/*  T T _ C A N C E L  --  Cancel i/o on tty channel if not complete  */

tt_cancel() {
    int mask;
    if (tt_queued) {
	if (!CHECK_ERR("tt_cancel: SYS$READEF",
	SYS$READEF(TTY_EFN, &mask))) return(-1);
	if ((mask & (1<<TTY_EFN)) == 0) {
	    SYS$CANCEL(ttychn);
	    tt_queued = 0;
	}
    }
}

/*  T T S N D B  --  Send a BREAK signal  */

ttsndb() {
    int x;
    struct tt_mode ttchr;

    if (ttychn == 0) return(-1);		/* Not open. */

    tt_cancel();
    if (!CHECK_ERR("ttsndb: SENSEMODE",
	SYS$QIOW(0, ttychn, IO$_SENSEMODE, &ttiosb, 0, 0,
		&ttchr, sizeof ttchr, 0, 0, 0, 0))) return(-1);
    if (!CHECK_ERR("ttsndb: SETMODE(1)",
	SYS$QIOW(0, ttychn, IO$_SETMODE, 0, 0, 0,
		&ttchr, sizeof ttchr, TT$C_BAUD_50, 0, 0, 0))) return(-1);
    x = 0;
    if (!CHECK_ERR("ttsndb: writing nulls",
	SYS$QIOW(0, ttychn, IO$_WRITEVBLK, 0, 0, 0,
		&x, 2, 0, 0, 0, 0))) return(-1);
    if (!CHECK_ERR("ttsndb: SETMODE(2)",
	SYS$QIOW(0, ttychn, IO$_SETMODE, 0, 0, 0,
		&ttchr, sizeof ttchr, ttiosb.size, 0, 0, 0))) return(-1);
    return(0);
}


/*  T T H A N G  --  Hang up the communications line  */

tthang() {
    if (ttychn == 0) return(0);			/* Not open. */

    tt_cancel();
    if (!CHECK_ERR("tthang: SYS$QIOW",
	SYS$QIOW(0, ttychn, IO$_SETMODE|IO$M_HANGUP, 0, 0, 0,
		0, 0, 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  M S L E E P  --  Millisecond version of sleep().  */

/*
 Handles intervals up to about 7 minutes (2**32 / 10**7 seconds)
*/

msleep(m) int m; {

    struct time_struct {
	long int hi, lo;
	} t;
    if (m <= 0) return(0);
    t.hi = -10000 * m;  /*  Time in 100-nanosecond units  */
    t.lo = -1;
    if (!CHECK_ERR("msleep: SYS$SCHDWK",
	SYS$SCHDWK(0, 0, &t, 0))) return(-1);
    SYS$HIBER();
    return(0);
}

/*  R T I M E R --  Reset elapsed time counter  */

rtimer() {
    tcount = time(0);
}


/*  G T I M E R --  Get current value of elapsed time counter in seconds  */

gtimer() {
    return( time( (long *) 0 ) - tcount );
}


/*  Z T I M E  --  Return date/time string  */

ztime(s) char **s; {
    struct dsc$descriptor_s t;
    static char time_string[24];
    t.dsc$w_length  = sizeof time_string - 1;  /*  Leave room for null  */
    t.dsc$a_pointer = time_string;
    t.dsc$b_class   = DSC$K_CLASS_S;
    t.dsc$b_dtype   = DSC$K_DTYPE_T;
    if (!CHECK_ERR("ztime: SYS$ASCTIM",
	SYS$ASCTIM(0, &t, 0, 0))) return(-1);
    time_string[t.dsc$w_length] = '\0';
    *s = time_string;
}


/*  C O N G M  --  Get console terminal modes.  */

/*
 Saves current console mode, and establishes variables for switching between 
 current (presumably normal) mode and other modes.
*/

congm() {
    struct { short len, code; char *buf, *retlen; } itmlst[2];
    int devclass, context;
    struct dsc$descriptor_s f, r;
    char buf[100], *strchr();

    r.dsc$w_length  = sizeof buf;
    r.dsc$b_dtype   = DSC$K_DTYPE_T;
    r.dsc$b_class   = DSC$K_CLASS_S;
    r.dsc$a_pointer = buf;
    f.dsc$w_length  = 10;
    f.dsc$b_dtype   = DSC$K_DTYPE_T;
    f.dsc$b_class   = DSC$K_CLASS_S;
    f.dsc$a_pointer = "SYS$INPUT:";
    context = 0;
    lib$find_file(&f, &r, &context);
    r.dsc$w_length = strchr(buf, ' ') - buf;
    itmlst[0].len    = 4;
    itmlst[0].code   = DVI$_DEVCLASS;
    itmlst[0].buf    = &devclass;
    itmlst[0].retlen = 0;
    itmlst[1].len    = 0;
    itmlst[1].code   = 0;
    if (!CHECK_ERR("congm: SYS$GETDVI",
	SYS$GETDVI(0, 0, &r, itmlst, 0, 0, 0, 0))) return(-1);
    SYS$WAITFR(0);
    debug(F101, "congm: devclass", "", devclass);
    if (devclass != DC$_TERM)
	batch = 1;
    else {
	if (conchn == 0 && (conchn = vms_assign_channel("SYS$INPUT:")) == 0)
	    return(-1);
        debug(F101, "congm: conchn", "", conchn);
        if (!CHECK_ERR("congm: SYS$QIOW",
    	SYS$QIOW(0, conchn, IO$_SENSEMODE, 0, 0, 0,
    	         &ccold, sizeof ccold, 0, 0, 0, 0))) return(-1);
        ccraw = cccbrk = ccold;
    }
    cgmf = 1;				/* Flag that we got them. */
    return(0);
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

concb(esc) char esc; {
    int x;
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    if (batch) return(0);
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifdef TT2$M_PASTHRU
    cccbrk.extended |= TT2$M_PASTHRU;
#else
#ifdef TT2$M_ALTYPEAHD
    cccbrk.extended |= TT2$M_ALTYPEAHD;
#endif
    cccbrk.basic |= TT$M_PASSALL;
#endif
    cccbrk.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    cccbrk.basic &= ~TT$M_WRAP & ~TT$M_HOSTSYNC & ~TT$M_TTSYNC;
    if (!CHECK_ERR("concb: SYS$QIOW",
	SYS$QIOW(0, conchn, IO$_SETMODE, 0, 0, 0,
    	    	 &cccbrk, sizeof cccbrk, 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  C O N B I N  --  Put console in binary mode  */


/*  Returns 0 if ok, -1 if not  */

conbin(esc) char esc; {
    if (cgmf == 0) congm();		/* Get modes if necessary. */
    if (batch) return(0);
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifdef TT2$M_PASTHRU
    ccraw.extended |= TT2$M_PASTHRU;
#else
#ifdef TT2$M_ALTYPEAHD
    ccraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ccraw.basic |= TT$M_PASSALL;
#endif
    ccraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    ccraw.basic &= ~TT$M_WRAP & ~TT$M_HOSTSYNC & ~TT$M_TTSYNC;
    if (!CHECK_ERR("conbin: SYS$QIOW",
	SYS$QIOW(0, conchn, IO$_SETMODE, 0, 0, 0,
    	    	 &ccraw, sizeof ccraw, 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  C O N R E S  --  Restore the console terminal  */

conres() {
    if (cgmf == 0) return(0);		/* Don't do anything if modes */
    if (batch) return(0);
    sleep(1);				/*  not known! */
    ckxech = 0;				/* System should echo chars */
    if (!CHECK_ERR("conres: SYS$QIOW",
	SYS$QIOW(0, conchn, IO$_SETMODE, 0, 0, 0,
	&ccold, sizeof ccold, 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  C O N O C  --  Output a character to the console terminal  */

conoc(c) char c; {
    if (batch) putchar(c);
    else
	CHECK_ERR("conoc: SYS$QIOW",
	    SYS$QIOW(0, conchn, IO$_WRITEVBLK, 0, 0, 0, &c, 1, 0, 0, 0, 0));
}

/*  C O N X O  --  Write x characters to the console terminal  */

conxo(x,s) char *s; int x; {
    if (batch) fprintf(stdout, "%.*s", x, s);
    else CHECK_ERR("conxo: SYS$QIOW",
	SYS$QIOW(0, conchn, IO$_WRITEVBLK, 0, 0, 0, s, x, 0, 0, 0, 0));
}

/*  C O N O L  --  Write a line to the console terminal  */

conol(s) char *s; {
    int len;
    if (batch) fputs(s, stdout);
    else {
	len = strlen(s);
	CHECK_ERR("conol: SYS$QIOW",
	    SYS$QIOW(0, conchn, IO$_WRITEVBLK, 0, 0, 0,
		     s, len, 0, 0, 0, 0));
    }
}

/*  C O N O L A  --  Write an array of lines to console, with CRLFs added */

conola(s) char *s[]; {
    int i;
    char t[100], *cp;
    for (i=0 ; *s[i] ; i++) {
	strncpy(t,s[i],100);
	for (cp = t + strlen(t); --cp >= t;) {
	    if (*cp != '\n' && *cp != '\r') {
		cp++;
		*cp++ = '\r'; *cp++ = '\n'; *cp++ = '\0';
		break;
	    }
	}
	conol(t);
    }  
}

/*  C O N O L L  --  Output a string followed by CRLF  */

conoll(s) char *s; {
    conol(s);
    conol("\r\n");
}


/*  C O N C H K  --  Check if characters available at console  */

conchk() {
    struct { short count;  char first;  char reserved1;  long reserved2; } t;
    if (batch) return(0);
    return(CHECK_ERR("conchk: SYS$QIOW",
	SYS$QIOW(0, conchn, IO$_SENSEMODE|IO$M_TYPEAHDCNT, 0, 0, 0,
	&t, sizeof t, 0, 0, 0, 0)) ? t.count : 0);
}


/*  C O N I N C  --  Get a character from the console  */

coninc(timo) int timo; {
    int n = 0; char ch;
    int func, mask;
    if (batch) return(getchar());
    mask = 1 << CON_EFN;
    if (con_queued) {
	if (timo > 0) {
	    struct { int hi, lo; } qtime;
	    qtime.hi = -10*1000*1000*timo;  /* Max about seven minutes */
	    qtime.lo = -1;
	    SYS$SETIMR(TIM_EFN, &qtime, 0, 0);
	    mask |= TIM_EFN;
	}
	SYS$WFLOR(CON_EFN, mask);
	SYS$READEF(CON_EFN, &mask);
	if (mask & (1 << CON_EFN)) {
	    ch = conch & 0377;
	    CHECK_ERR("coninc: coniosb.status", coniosb.status);
	    con_queued = 0;
	} else {
	    ch = -1;
	    vms_status = SS$_TIMEOUT;
	}
    } else {
	func = IO$_READVBLK;
	if (timo > 0) func |= IO$M_TIMED;
	CHECK_ERR("coninc: SYS$QIOW",
	    SYS$QIOW(0, conchn, func, 0, 0, 0, &ch, 1, timo, 0, 0, 0));
	ch &= 0377;
    }
    if (ch == '\r') ch = '\n';
    if ((vms_status & 7) == 1) return(ch);
    else return(-1);
}

/*  V M S _ G E T C H A R -- get a character from the console (no echo).
 *	Since we use raw reads, we must check for ctrl/c, ctrl/y and
 *	ctrl/z ourselves.  We probably should post a "mailbox" for
 *	ctrl/c and ctrl/y so the poor user can abort a runaway Kermit.
 *	Note: this routine intends for ctrl/z (eof) to be "permanent".
 *	Currently, no kermit routine calls "clearerror".  If this
 *	changes, the following code must be rewritten.
 */

int
vms_getchar()
{
	register int	ch;
	static int	ateof = FALSE;

	if (ateof)
	    return (EOF);
	ch = coninc(0);
	switch (ch) {
	case ('Y' & 0x1F):
	case ('C' & 0x1F):
	    ttclos();			/* Close down other terminal	*/
	    conres();			/* And cleanup console modes	*/
	    exit(SS$_ABORT);		/* Fatal exit.			*/

	case ('Z' & 0x1F):
	    ateof = TRUE;
	    return (EOF);

	default:
	    return (ch);
	}
}


/*  C O N T T I  --  Get character from console or tty, whichever comes  */
/*	first.  This is used in conect() when NO_FORK is defined.  */
/*	src is returned with 1 if the character came from the comm. line, */
/*	0 if it was from the console, and with -1 if there was any error.  */

contti(c, src)  int *c, *src;  {
    int mask = 1<<CON_EFN | 1<<TTY_EFN;

    *src = -1;
    if (batch) {
	if ((*c = getchar()) != EOF) {
	    *src = 0;
	} else {
	    *src = 1;
	    *c = ttinc(0);
	}
    } else {
        if (!con_queued)
            if (!CHECK_ERR("contti: console SYS$QIO",
            SYS$QIO(CON_EFN, conchn, IO$_READVBLK, &coniosb, 0, 0,
                    &conch, 1, 0, 0, 0, 0))) return(-1);
        con_queued = 1;
        if (!tt_queued)
            if (!CHECK_ERR("contti: tty SYS$QIO",
            SYS$QIO(TTY_EFN, ttychn, IO$_READVBLK, &ttiosb, 0, 0,
                    &ttch, 1, 0, 0, 0, 0))) return(-1);
        tt_queued = 1;
        if (!CHECK_ERR("contti: SYS$WFLOR",
            SYS$WFLOR(CON_EFN, mask))) return(-1);
        if (!CHECK_ERR("contti: SYS$READEF",
            SYS$READEF(CON_EFN, &mask))) return(-1);
        if (*src = (mask & (1<<TTY_EFN)) != 0) {
            *c = ttch;
            CHECK_ERR("contti: ttiosb.status", ttiosb.status);
            tt_queued = 0;
        } else {
            *c = conch;
            CHECK_ERR("contti: coniosb.status", coniosb.status);
            con_queued = 0;
        }
        if ((vms_status & 7) != 1) *src = -1;
    }
    return(0);
}

/*  C A N C I O  --  Cancel any pending i/o requests on the console or the
        comm. line.
*/

cancio()  {
    if (!batch) {
        CHECK_ERR("cancio: console SYS$CANCEL",
            SYS$CANCEL(conchn));
        con_queued = 0;
        CHECK_ERR("cancio: tty SYS$CANCEL",
            SYS$CANCEL(ttychn));
        tt_queued = 0;
    }
}
