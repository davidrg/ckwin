/* DEC/CMS REPLACEMENT HISTORY, Element CKVTIO.C */
/* *6    29-AUG-1989 00:32:11 BUDA "Fix exquota problem" */
/* *5     1-MAY-1989 23:05:45 BUDA "Add debugging code/general cleanup" */
/* *4    24-APR-1989 22:54:08 BUDA "Work on parity" */
/* *3    16-APR-1989 17:57:47 BUDA "" */
/* *2    12-APR-1989 00:58:59 BUDA "Add some of the new support" */
/*  1U1  12-APR-1989 00:31:27 BUDA "Lots of neat stuff added" */
/* *1    12-APR-1989 00:30:04 BUDA "Initial Creation" */
/* DEC/CMS REPLACEMENT HISTORY, Element CKVTIO.C */
char *ckxv = "TTY I/O, 1.0(021), 08 Jul 89";

/*  C K V T I O  --  Terminal and Interrupt Functions for VAX/VMS  */

/* Edit History
 * 021 08-Jul-89 mab	Add ^C/^Y abort server mode code.
 * 020 13-Jun-89 mab	Fix on exquota problem on qiow(readvblk)
 * 019 25-Apr-89 mab	Change baud to 110 for V4.x break routine as
 * 			50 baud is nto supported on most MUX's by VMS.
 * 018 23-Apr-89 mab	Add some of Valerie Mates parity changes.
 * 017 04-Apr-89 mab	Fix some minor bugs to lcl/remote code
 * 016 23-Mar-89 mab	Add IO$M_BREAKTHRU to IO$_WRITEVBLK.
 *			Add zchkspd() function to check for valid speed.
 * 015 26 Feb 89 mab	Add dcl exit handler and minor cleanup
 * 014 14 Feb 89 mab	Make break REALLY work.  Add IOSB's to all QIO's.
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

char *ckxsys = " VAX/VMS";



/*
 Variables available to outside world:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console(remote), 1 if external line(local).
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
   tt_cancel()		   -- Cancel any asynch I/O to tty
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
#include "ckvvms.h"
#include <stdio.h>			/* Unix Standard i/o */
#include <signal.h>			/* Interrupts */
#include <setjmp.h>			/* Longjumps */
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <ssdef.h>
#include <descrip.h>
#include <dvidef.h>
#include <dcdef.h>
#include <devdef.h>
#include "ckcdeb.h"			/* Formats for debug() */



/* Declarations */

    long time();			/* Get current time in secs */
    void dcl_exit_h(unsigned long int *); /* Exit handler */
    unsigned long int vms_assign_channel(char *);

/* dftty is the device name of the default device for file transfer */
/* dfloc is 1 if dftty is the user's console terminal, 0 if an external line */

    char *dftty = CTTNAM;
    int dfloc = 0;			/* Default location is local */
    int dfprty = 0;			/* Parity (0 = none) */
    int ttprty = 0;			/* Parity in use */
    int ttmdm = 0;                      /* Modem in use. */
    int dfflow = 1;			/* Xon/Xoff flow control */
    int batch = 0;			/* Assume interactive */

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

unsigned int vms_status;		/* Used for system service return status */

/* Structures used within this module */


static struct {
    unsigned char dec;
    unsigned short int line;
    } ttspeeds[] = {
	{TT$C_BAUD_50,       50},
	{TT$C_BAUD_75,       75},
	{TT$C_BAUD_110,     110},
	{TT$C_BAUD_134,     134},
        {TT$C_BAUD_150,     150},
	{TT$C_BAUD_300,     300},
	{TT$C_BAUD_600,     600},
	{TT$C_BAUD_1200,   1200}, 
	{TT$C_BAUD_1800,   1800}, 
	{TT$C_BAUD_2000,   2000},
	{TT$C_BAUD_2400,   2400},
	{TT$C_BAUD_3600,   3600}, 
	{TT$C_BAUD_4800,   4800},
	{TT$C_BAUD_7200,   7200},
	{TT$C_BAUD_9600,   9600}, 
	{TT$C_BAUD_19200, 19200},
	{TT$C_BAUD_38400, 38400},
	{0,                   0} };


/* Declarations of variables global within this module */

static long tcount;			/* For timing statistics */

static char *brnuls = "\0\0\0\0\0\0\0"; /* A string of nulls */

static int conif = 0,			/* Console interrupts on/off flag */
    conclass = 0,			/* Console device type */
    cgmf = 0,				/* Flag that console modes saved */
    xlocal = 0,                         /* Flag for tty local or remote */
    ttychn = 0,				/* TTY i/o channe; */
    conchn = 0,				/* Console i/o channel */
    con_queued = 0,			/* console i/o queued in contti() */
    tt_queued = 0,			/* tty i/o queued in contti() */
    conch,				/* console input character buffer  */
    ttch;				/* tty input character buffer */
static unsigned char escchr;		/* Escape or attn character */
static char ttnmsv[65];          	/* copy of open path for tthang */
static char lclnam[65];			/* Local device name */

static long int qio_maxbuf_size;	/* Maximum size of QIO to succeed */
static unsigned long dclexh_status;	/* Exit status for DCL exit handler */
static struct iosb_struct coniosb, ttiosb, wrk_iosb;
static struct tt_mode
    ttold, ttraw, tttvt,		/* for communication line */
    ccold, ccraw, cccbrk,		/* and for console */
    cctmp;

#if 0
/* These aren't used */
static int inbufc = 0;			/* stuff for efficient raw line */
static int ungotn = -1;			/* pushback to unread character */
#endif



/*  P R I N T _ M S G  --  Log an error message from VMS  */

print_msg(s) char *s; {
    long int blen = 0;
    char buf[PMSG_BUF_SIZE], msg[PMSG_MSG_SIZE];
    struct dsc$descriptor_s b = {PMSG_BUF_SIZE-1,DSC$K_DTYPE_T,DSC$K_CLASS_S,&buf};

    if (!((vms_status = SYS$GETMSG(vms_status, &blen, &b, 0, 0)) & 1)) {
	fprintf(stderr,"print_msg; SYS$GETMSG\n");
	return(-1);
    }

    buf[blen] = '\0';
    sprintf(msg, "%s: %s\n", s, buf);
    ermsg(msg);
}


/*  S Y S I N I T  --  System-dependent program initialization.  */

sysinit() {
extern int speed;
extern char ttname[];
struct itmlst dviitm[] = {{64,DVI$_FULLDEVNAM,&lclnam,0},
			{sizeof(conclass),DVI$_DEVCLASS,&conclass,0},
			{0,0,0,0}};

static struct desblk {
    long int *fl;		/* Forward link.  Used by VMS only */
    void (*fncpnt)();		/* Function to call */
    unsigned char argcnt;	/* Only one arg allowed */
    unsigned char filler[3];	/* Filler.  Must be zero */
    long int *sts;		/* Address of sts (written by VMS) */
    } dclexh_ = {0,dcl_exit_h,1,{0,0,0},&dclexh_status};

/*
 * Set up DCL Exit handler.  This allows us to reset terminal
 * and any other modifications we have done.
 */
    if (!CHECK_ERR("sysinit: SYS$DCLEXH",
	SYS$DCLEXH(&dclexh_))) return(0);

    if (ttychn)                     	/* if comms line already opened */
          return(0);
    
    if (!conchn)
	   conchn = vms_assign_channel(dftty);
/*
 * Parse console terminal device name.
 */
    CHECK_ERR("sysinit: SYS$GETDVIW",
	SYS$GETDVIW(0, conchn, 0, &dviitm, &wrk_iosb, 0, 0, 0));
    debug(F111,"sysinit","lclnam",lclnam);

    if (!CHECK_ERR("congm: SYS$QIOW",
    	SYS$QIOW(QIOW_EFN, conchn, IO$_SENSEMODE, &wrk_iosb, 0, 0,
    	         &ccold, sizeof(ccold), 0, 0, 0, 0))) return(-1);

    speed = ttispd((unsigned char) wrk_iosb.size);
    debug(F111,"sysinit speed",lclnam,speed);
    strncpy(ttname,lclnam,DEVNAMLEN);		/* Max because of ckcmai */

    return(0);
}

/*
 * DCL Exit handler.  This is the cleanup handler for program.
 * Any final cleanup (closing channels etc) should be done at this
 * point.
 */
void dcl_exit_h(unsigned long int *sts)
{
    syscleanup();
    return;
}


/*  S Y S C L E A NU P -- System-dependent program epilog.  */

syscleanup() {
    extern zclosf();

    ttclos();			/* Do the cleanup no matter what... */
    conres();			/* for the console also... */
    zclosf();			/* Close various files and kill child proc */
    printf("\r");
    return(0);
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
    extern int speed;
    int s;
    unsigned long int devchar, devclass, devsts;
    char dvibuf[65];
    struct dsc$descriptor_s devnam = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
    struct itmlst dviitm[] = {{64,DVI$_FULLDEVNAM,&dvibuf,0},
			{sizeof(devchar),DVI$_DEVCHAR,&devchar,0},
			{sizeof(devclass),DVI$_DEVCLASS,&devclass,0},
			{sizeof(devsts),DVI$_STS,&devsts,0},
			{0,0,0,0}};

    devnam.dsc$w_length  = strlen(ttname);
    devnam.dsc$a_pointer = ttname;
    CHECK_ERR("ttopen: SYS$GETDVIW",
	SYS$GETDVIW(0, 0, &devnam, &dviitm, &wrk_iosb, 0, 0, 0));

    if ((devclass != DC$_TERM) ||
    	!(devchar & DEV$M_AVL) ||
	!(devsts & UCB$M_ONLINE)) {
	fprintf(stderr,"%%CKERMIT-W-NOTTERM, Device '%s' is not a terminal\n",dvibuf);
	return(-1);
    }
    dvibuf[65] = '\0';
    if (ttychn) return(0);		/* Close channel if open */

    ttmdm = modem;			/* Make this available to other fns */
    xlocal = *lcl;			/* Make this available to other fns */
/*
 * The following is 'kludgy', but it is the only way to make this work
 * without modifying CKUUS3.  I copy the returned device name into
 * the command line!  If another parameter were to follow this one (which
 * it does not at this time), it would be overwritten, possibly.
 */

    strncpy(ttname,dvibuf,50);

    ttychn = vms_assign_channel(ttname);

    debug(F111,"ttopen","modem",modem);
    debug(F101," ttychn","",ttychn);

    if (!ttychn) return(-1);		/* If couldn't open, fail. */

/*
 * Check for maximum size of QIO, so as to not get the dreaded exceeded quota
 * status returned.  When doing a QIO that has a larger buffer than
 * MAXBUF, exceeded quota wil be returned.  There is also another limit, lower
 * than MAXBUF, as to when extra features are used in the QIO, that more
 * space can be used.
 * 
 * Example: MAXBUF = 2048, QIO = 1936, overhead is 112 will succeed.
 *	    QIO of 1937 will fail.
 *
 * This can change for different versions of VMS.
 */

    qio_maxbuf_size = get_qio_maxbuf_size(ttychn);

    strcpy(ttnmsv,ttname);		/* Open, keep copy of name locally. */

/* Caller wants us to figure out if line is controlling tty */

    debug(F111,"ttopen ok",ttname,*lcl);
    if (*lcl < 0) {
	if (conclass == DC$_TERM)
	    xlocal = (strncmp(ttname,lclnam,DEVNAMLEN) == 0) ? 0 : 1;
	else
	    xlocal = 1;			/* If not a term, then we must be local*/
/*
 * xlocal = 1 = local
 *          0 = remote
 */
	debug(F111,"ttyname",lclnam,xlocal);
    }

    if (!CHECK_ERR("ttopen: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SENSEMODE, &wrk_iosb, 0, 0,
		 &ttold, sizeof(ttold), 0, 0, 0, 0))) return(-1);

    speed = ttispd((unsigned char) wrk_iosb.size);

/* Got the line, now set the desired value for local. */

    if (*lcl) *lcl = xlocal;

    tttvt = ttold;
    ttraw = ttold;
    debug(F101," lcl","",*lcl);
    return(0);
}


unsigned long int vms_assign_channel(ttname) char *ttname;  {
    unsigned int channel = 0;
    struct dsc$descriptor_s d = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};

    d.dsc$w_length  = strlen(ttname);
    d.dsc$a_pointer = ttname;
    if (!CHECK_ERR("vms_assign_channel: SYS$ASSIGN",
	SYS$ASSIGN(&d, &channel, 0, 0))) return(0);
    return(channel);
}



/*  T T C L O S  --  Close the TTY, releasing any lock.  */

ttclos() {
    if (!ttychn) return(0);		/* Wasn't open. */

    ttres();				/* Reset modes. */
    if (!CHECK_ERR("ttclos: SYS$DASSGN",
	SYS$DASSGN(ttychn))) return(-1);
    ttychn = 0;				/* Mark it as closed. */
    return(0);
}


/*  T T R E S  --  Restore terminal to "normal" mode.  */

ttres() {				/* Restore the tty to normal. */
    if (!ttychn) return(-1);		/* Not open. */

    tt_cancel();			/* Cancel outstanding I/O */
    msleep(250);			/* Wait for pending i/o to finish. */
    debug(F101,"ttres, ttychn","",ttychn);
    if (!CHECK_ERR("ttres: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &wrk_iosb, 0, 0,
    		 &ttold, sizeof(ttold), 0, 0, 0, 0))) return(-1);
    return(0);
}



/*  T T P K T  --  Condition the communication line for packets. */
/*		or for modem dialing */

#define DIALING	4		/* flags (via flow) for modem handling */
#define CONNECT 5

/*  If called with speed > -1, also set the speed.  */

/*  Returns 0 on success, -1 on failure.  */

ttpkt(speed,flow,parity) int speed, flow, parity; {
    extern char ttname[];
    int s;

    if (!ttychn) return(-1);		/* Not open. */
    ttprty = parity;
    debug(F101,"ttpkt setting ttprty","",ttprty);

    if ((s = ttsspd(speed)) < 0) s = 0;
    if (flow == 1) ttraw.basic |=  (TT$M_HOSTSYNC|TT$M_TTSYNC);
    if (flow == 0) ttraw.basic &= ~(TT$M_HOSTSYNC|TT$M_TTSYNC);
#ifdef TT2$M_PASTHRU
    ttraw.extended |= TT2$M_PASTHRU;
#else
    ttraw.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    ttraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ttraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    ttraw.basic &= ~TT$M_WRAP;
    if (!CHECK_ERR("ttpkt: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &wrk_iosb, 0, 0,
    	    	 &ttraw, sizeof(ttraw), s, 0, 0, 0))) return(-1);
    ttflui();				/* Flush any pending input */
    return(0);
}



/*  T T V T -- Condition communication line for use as virtual terminal  */

ttvt(speed,flow) int speed, flow; {
    extern char ttname[];
    int s;

    if (!ttychn) return(-1);		/* Not open. */

    if ((s = ttsspd(speed)) < 0) s = 0;

    if (flow == 1) ttraw.basic |=  (TT$M_HOSTSYNC|TT$M_TTSYNC);
    if (flow == 0) ttraw.basic &= ~(TT$M_HOSTSYNC|TT$M_TTSYNC);
#ifdef TT2$M_PASTHRU
    ttraw.extended |= TT2$M_PASTHRU;
#else
    ttraw.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    ttraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ttraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    ttraw.basic &= ~TT$M_WRAP;
    if (!CHECK_ERR("ttvt: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &wrk_iosb, 0, 0,
			  &ttraw, sizeof(ttraw), s, 0, 0, 0))) return(-1);
    return(0);
}


/* T T I S P D  -- Return binary baud rate for internal coded speed */

int
ttispd(ispeed) unsigned char ispeed; {
    int s;

/* When the line is set, grab the line speed  and save it */

    for (s = 0;  ttspeeds[s].dec && 
	(ttspeeds[s].dec != ispeed);  s++)
		;

/* If speed is zero, then no match.  Set speed to -1 so it is undefined */

    return(ttspeeds[s].line ? (int) ttspeeds[s].line : -1);

}


/*  T T S S P D  --  Return the internal baud rate code for 'speed'.  */

int
ttsspd(speed) int speed; {
    int s;
    char msg[50];

    if (speed < 0)			/* 006 Unknown speed fails	*/
	return (-1);
    for (s = 0;  ttspeeds[s].line && (ttspeeds[s].line != speed);  s++) ;
    if (ttspeeds[s].line)
	return(ttspeeds[s].dec);
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

    if (!ttychn) return(-1);		/* Not open. */

    if (!CHECK_ERR("ttflui: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_READVBLK|IO$M_TIMED|IO$M_PURGE,
		 &wrk_iosb, 0, 0, &n, 0, 0, 0, 0, 0))) perror("flush failed");

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
    static struct {
	unsigned short count; 
	unsigned char first; 
	unsigned char reserved1; 
	long reserved2; } ttchk_struct;

    /* inbufc + (ungotn >= 0) +		-- 006 Note: unused	*/
    CHECK_ERR("ttchk: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SENSEMODE|IO$M_TYPEAHDCNT, &wrk_iosb, 0, 0,
	  &ttchk_struct, sizeof(ttchk_struct), 0, 0, 0, 0));
    debug(F101,"ttchk","",(int)ttchk_struct.count);
    return(vms_status & 1 ? ttchk_struct.count : 0);
}


/*  T T X I N  --  Get n characters from tty input buffer  */

ttxin(n,buf) int n; char *buf; {
    int i;
    unsigned char *cp;
    static int trmmsk[2] = {0,0};

    if (!CHECK_ERR("ttxin: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_READVBLK, &wrk_iosb, 0, 0,
			  buf, n, 0, &trmmsk, 0, 0))
	|| !CHECK_ERR("ttxin: wrk_iosb.status",wrk_iosb.status)) return(-1);
    if (ttprty)
	for (i = wrk_iosb.size, cp = buf; (i) ;i--)
		*cp++ &= 0177;
    buf[n] = '\0';
    return(wrk_iosb.size);
}



/*  T T O L  --  Similar to "ttinl", but for writing.  */
/*
 * This probably should be buffered with a flush command added.
 */

ttol(s,n) int n; char *s; {
    int x;

    if (!ttychn) return(-1);		/* Not open. */
    if (CHECK_ERR("ttol: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &wrk_iosb, 0, 0,
    	s, n, 0, 0, 0, 0))) x = 0;
    else x = -1;
    debug(F111,"ttol",s,n);
    if (x < 0) debug(F101,"ttol failed","",x);
    return(x);
}


/*  T T O C  --  Output a character to the communication line  */

ttoc(c) char c; {
    if (!ttychn) return(-1);		/* Not open. */

    if (CHECK_ERR("ttoc: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &wrk_iosb, 0, 0,
    	&c, 1, 0, 0, 0, 0))) return(0);
    return(-1);
}



/*  T T I N L  --  Read a record (up to break character) from comm line.  */
/*
  If no break character encountered within "max", return "max" characters,
  with disposition of any remaining characters undefined.  Otherwise, return
  the characters that were read, including the break character, in "dest" and
  the number of characters read as the value of function, or 0 upon end of
  file, or -1 if an error occurred or, -2 if two ^C's were typed.  Times
  out & returns error if not completed within "timo" seconds.
*/

ttinl(dest,max,timo,eol) int max,timo; char *dest, eol; {
    int x, y, c, i;
    static int trmmsk[2] = {0,0};
    static int trmlong[8] = {0,0,0,0,0,0,0,0};
    int ccnt = 0;		/* Control C counter */
    int func;
    unsigned char *cp;

    if (!ttychn) return(-1);		/* Not open. */

    if (ttprty) {
	trmmsk[0] = sizeof(trmlong);
	trmmsk[1] = &trmlong;
	trmlong[0] = (1 << eol) | CKV_M_CTRLC | CKV_M_CTRLY;
	trmlong[4] = (1 << eol) | CKV_M_CTRLC | CKV_M_CTRLY;
    } else {
	trmmsk[0] = 0;
	trmmsk[1] = (1 << eol) | CKV_M_CTRLC | CKV_M_CTRLY;
    }

    func = IO$_READVBLK;
    if (timo > 0) func |= IO$M_TIMED;

/*
 * Limit size of QIO so we do not get exquota error.
 */
    if (max > qio_maxbuf_size) max = qio_maxbuf_size;

    do {
	vms_status = SYS$QIOW(QIOW_EFN, ttychn, func, &wrk_iosb, 0, 0,
			  dest, max, timo, &trmmsk, 0, 0);

/*
 * Did any type of error occur on submission of QIO or execution of QIO?
 */
	if ((!(vms_status & 1)) ||
	    (!(wrk_iosb.status & 1))) {
/*
 * If we got a time out, then fine, return with a failure, but no messages
 */
	    if (wrk_iosb.status == SS$_TIMEOUT)
		return(-1);
/*
 * If the submission of QIO was fine, but the execution failed
 * then save the status so when the error is printed out, we know
 */
	
	    if (!(vms_status & 1))
		print_msg("ttinl: SYS$QIOW");
	    if (!(wrk_iosb.status & 1)) {
		vms_status = wrk_iosb.status;
		print_msg("ttinl: SYS$QIOW(iosb)");
		}
	    return(-1);
	}
/*
 * Check for a control C being typed
 */
	if ((((unsigned char) wrk_iosb.terminator & 0177) == CKV_K_CTRLC) ||
	     (((unsigned char) wrk_iosb.terminator & 0177) == CKV_K_CTRLY)) {
	    if (++ccnt > 1) {
		fprintf(stderr,"^C...");
		ttres();
		fprintf(stderr,"\n");
		return(-2);
		}
	    }
	else
	    ccnt = 0;
    } while (ccnt);

    if (ttprty)
	for (i=wrk_iosb.size, cp=dest; (i); i--) {
	    *cp++ &= 0177;
	    }
    return(wrk_iosb.size);
}



/*  T T I N C --  Read a character from the communication line  */

ttinc(timo) int timo; {
    unsigned char ch[1];
    static int trmmsk[2] = {0,0};
    int func;

    if (!ttychn) return(-1);		/* Not open. */
    func = IO$_READVBLK;
    if (timo > 0) func |= IO$M_TIMED;
    if (!CHECK_ERR("ttinc: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, func, &wrk_iosb, 0, 0,
			  &ch, 1, timo, &trmmsk, 0, 0))
	|| wrk_iosb.status == SS$_TIMEOUT	/* Check separately so no err msg */
	|| !CHECK_ERR("ttinc: wrk_iosb.status",wrk_iosb.status)) return(-1);
    return(ttprty ? ch[0] & 0177 : ch[0]);
}



/*  T T _ C A N C E L  --  Cancel i/o on tty channel if not complete  */

tt_cancel() {
    int mask;

    CHECK_ERR("tt_cancel: SYS$CANCEL",SYS$CANCEL(ttychn));
    tt_queued = 0;
}

/*  C O N _ C A N C E L  --  Cancel i/o on console channel if not complete  */

con_cancel() {
    int mask;

    CHECK_ERR("con_cancel: SYS$CANCEL",SYS$CANCEL(conchn));
    con_queued = 0;
}

/*  T T S N D B  --  Send a BREAK signal  */

ttsndb() {
    int long x = 0;
    struct iosb_struct  tmp_ttiosb;
    struct tt_mode ttchr;

    if (!ttychn) return(-1);		/* Not open. */

    tt_cancel();
#ifndef TT$M_BREAK
    if (!CHECK_ERR("ttsndb: SENSEMODE",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SENSEMODE, &wrk_iosb, 0, 0,
		&ttchr, sizeof(ttchr), 0, 0, 0, 0))) return(-1);
    if (!CHECK_ERR("ttsndb: SETMODE(1)",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &tmp_ttiosb, 0, 0,
		&ttchr, sizeof(ttchr), TT$C_BAUD_110, 0, 0, 0))) return(-1);
    if (!CHECK_ERR("ttsndb: writing nulls",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &tmp_ttiosb, 0, 0,
		brnuls, 6, 0, 0, 0, 0))) return(-1);
    if (!CHECK_ERR("ttsndb: SETMODE(2)",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &tmp_ttiosb, 0, 0,
		&ttchr, sizeof(ttchr), wrk_iosb.size, 0, 0, 0))) return(-1);
#else
    if (!CHECK_ERR("ttsndb: SENSEMODE",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SENSEMODE, &wrk_iosb, 0, 0,
		&ttchr, sizeof(ttchr), 0, 0, 0, 0))) return(-1);

/* Break signal on */
    x = TT$M_BREAK;    
    if (!CHECK_ERR("ttsndb: SETMODE(1)",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &wrk_iosb, 0, 0,
		&ttchr, sizeof(ttchr), 0, 0, x, 0))) return(-1);
    msleep(275);

/* Break signal off */
    x = 0;
    if (!CHECK_ERR("ttsndb: SETMODE(2)",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE, &wrk_iosb, 0, 0,
		&ttchr, sizeof(ttchr), 0, 0, x, 0))) return(-1);

#endif
    return(0);
}



/*  T T H A N G  --  Hang up the communications line  */

tthang() {
    if (!ttychn) return(0);			/* Not open. */

    tt_cancel();
    if (!CHECK_ERR("tthang: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, ttychn, IO$_SETMODE|IO$M_HANGUP, &wrk_iosb, 0, 0,
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
    tcount = time( (long *) 0);
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
    static char time_string[24];
    struct dsc$descriptor_s t =
	{sizeof(time_string)-1,DSC$K_DTYPE_T,DSC$K_CLASS_S,&time_string};

    if (!CHECK_ERR("ztime: SYS$ASCTIM",
	SYS$ASCTIM(0, &t, 0, 0))) return(-1);
    time_string[t.dsc$w_length] = '\0';
    *s = &time_string;
}



/*  C O N G M  --  Get console terminal modes.  */

/*
 Saves current console mode, and establishes variables for switching between 
 current (presumably normal) mode and other modes.
*/

congm() {
    char s[] = "SYS$INPUT:";
    struct itmlst dviitm[] = { {4,DVI$_DEVCLASS,&dviitm[0].adr,0},
			{0,0,0,0}};
    struct dsc$descriptor_s
	r = {sizeof(s),DSC$K_DTYPE_T,DSC$K_CLASS_S,&s};

    if (cgmf) return(-1);		/* If called already, then nop */

    if (!CHECK_ERR("congm: SYS$GETDVIW",
	SYS$GETDVIW(0, 0, &r, &dviitm, &wrk_iosb, 0, 0, 0))) return(-1);
    debug(F101, "congm: devclass", "", (unsigned long int) dviitm[0].adr);
    if ((unsigned long int) dviitm[0].adr != DC$_TERM)
	batch = 1;
    else {
	if (conchn == 0 && (conchn = vms_assign_channel("SYS$INPUT:")) == 0)
	    return(-1);
        debug(F101, "congm: conchn", "", conchn);
        if (!CHECK_ERR("congm: SYS$QIOW",
    	SYS$QIOW(QIOW_EFN, conchn, IO$_SENSEMODE, &wrk_iosb, 0, 0,
    	         &ccold, sizeof(ccold), 0, 0, 0, 0))) return(-1);
        ccraw = cccbrk = ccold;
    }
    cgmf = 1;				/* Flag that we got them. */
    return(0);
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

concb(esc) char esc; {
    int x;

    if (batch) return(0);
    if (!cgmf) congm();			/* Get modes if necessary. */
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifdef TT2$M_PASTHRU
    cccbrk.extended |= TT2$M_PASTHRU | TT2$M_DMA;
#else
    cccbrk.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    cccbrk.extended |= TT2$M_ALTYPEAHD;
#endif
    cccbrk.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    cccbrk.basic &= ~(TT$M_WRAP);
/*    cccbrk.basic &= ~(TT$M_WRAP | TT$M_HOSTSYNC | TT$M_TTSYNC); */
    if (!CHECK_ERR("concb: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_SETMODE, &wrk_iosb, 0, 0,
    	    	 &cccbrk, sizeof(cccbrk), 0, 0, 0, 0))) return(-1);
    return(0);
}



/*  C O N B I N  --  Put console in binary mode  */


/*  Returns 0 if ok, -1 if not  */

conbin(esc) char esc; {
    if (batch) return(0);
    if (!cgmf) congm();		/* Get modes if necessary. */

    debug(F100,"conbin","",0);
    escchr = esc;			/* Make this available to other fns */
    ckxech = 1;				/* Program can echo characters */
#ifdef TT2$M_PASTHRU
    ccraw.extended |= TT2$M_PASTHRU | TT2$M_DMA;
#else
    ccraw.basic |= TT$M_PASSALL;
#endif
#ifdef TT2$M_ALTYPEAHD
    ccraw.extended |= TT2$M_ALTYPEAHD;
#endif
    ccraw.basic |= TT$M_NOECHO | TT$M_EIGHTBIT;
    ccraw.basic &= ~(TT$M_WRAP | TT$M_HOSTSYNC | TT$M_TTSYNC);
    if (!CHECK_ERR("conbin: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_SETMODE, &wrk_iosb, 0, 0,
    	    	 &ccraw, sizeof(ccraw), 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  C O N R E S  --  Restore the console terminal  */

conres() {
    debug(F100," conres","",0);
    if (!cgmf) return(0);		/* Don't do anything if modes */
    if (batch) return(0);

#ifdef mab
    msleep(250);			/*  not known! */
#endif
    ckxech = 0;				/* System should echo chars */
    if (!CHECK_ERR("conres: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_SETMODE, &wrk_iosb, 0, 0,
	&ccold, sizeof(ccold), 0, 0, 0, 0))) return(-1);
    return(0);
}


/*  C O N R E S N E --  Restore the console terminal with No Echo */

conresne() {
    if (!cgmf) return(0);		/* Don't do anything if modes */
    if (batch) return(0);

    msleep(250);			/*  not known! */
    ckxech = 1;				/* Program should echo chars */

    cctmp = ccold;
    cctmp.basic |= TT$M_NOECHO;
    if (!CHECK_ERR("conres: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_SETMODE, &wrk_iosb, 0, 0,
	&cctmp, sizeof(cctmp), 0, 0, 0, 0))) return(-1);
    return(0);
}




/*  C O N O C  --  Output a character to the console terminal  */

conoc(c) char c; {
    if (batch) putchar(c);
    else
	CHECK_ERR("conoc: SYS$QIOW",
	    SYS$QIOW(QIOW_EFN, conchn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &wrk_iosb, 0, 0, &c,
		1, 0, 0, 0, 0));
}

/*  C O N X O  --  Write x characters to the console terminal  */

conxo(x,s) char *s; int x; {
    if (batch) fprintf(stdout, "%.*s", x, s);
    else CHECK_ERR("conxo: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &wrk_iosb, 0, 0, s, x, 0, 0, 0, 0));
}

/*  C O N O L  --  Write a line to the console terminal  */

conol(s) char *s; {
    int len;

    if (batch) fputs(s, stdout);
    else {
	len = strlen(s);
	CHECK_ERR("conol: SYS$QIOW",
	    SYS$QIOW(QIOW_EFN, conchn, IO$_WRITEVBLK|IO$M_BREAKTHRU, &wrk_iosb, 0, 0,
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
    struct {
	unsigned short count; 
	unsigned char first; 
	unsigned char reserved1; 
	long reserved2;
	} t;

    if (batch) return(0);
    return(CHECK_ERR("conchk: SYS$QIOW",
	SYS$QIOW(QIOW_EFN, conchn, IO$_SENSEMODE|IO$M_TYPEAHDCNT, &wrk_iosb, 0, 0,
	&t, sizeof(t), 0, 0, 0, 0)) ? t.count : 0);
}



/*  C O N I N C  --  Get a character from the console  */

coninc(timo) int timo; {
    int n = 0;
    unsigned char ch;
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
	    ch = (unsigned char) conch;
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
	    SYS$QIOW(QIOW_EFN, conchn, func, &wrk_iosb, 0, 0, &ch, 1, timo, 0, 0, 0));
    }
    if (ch == '\r') ch = '\n';
    if (vms_status & 1) return(ch);
    return(-1);
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
	register unsigned int	ch;
	static int	ateof = FALSE;

    if (ateof)
	return (EOF);
    ch = coninc(0);
    switch (ch) {
	case ('Y' - 64):
	case ('C' - 64):
	    ttclos();			/* Close down other terminal	*/
	    conres();			/* And cleanup console modes	*/
	    exit(SS$_ABORT);		/* Fatal exit.			*/

	case ('Z' - 64):
	    ateof = TRUE;
	    return (EOF);

	default:
	    return (ch);
    }
}



/*  C O N T T I  --  Get character from console then from tty  */
/*	This is used in conect() when NO_FORK is defined.  */
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
    } else {				/* If interactive */
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
        if (!(*src = ((mask & 1<<CON_EFN) ? 0 : 1))) {
            *c = conch;
            CHECK_ERR("contti: coniosb.status", coniosb.status);
            con_queued = 0;
        } else {
            *c = (ttprty ? ttch & 0177 : ttch);
            CHECK_ERR("contti: ttiosb.status", ttiosb.status);
            tt_queued = 0;
        }
        if (!(vms_status & 1)) *src = -1;
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
        CHECK_ERR("cancio: tty SYS$CANCEL",
            SYS$CANCEL(ttychn));
        con_queued = 0;
        tt_queued = 0;
    }
}

/* get_qio_maxbuf_size()
 *
 * Get maximum size of QIO that can occur without getting the dreaded
 * exceeded quota status.
 */

int get_qio_maxbuf_size(ttychn)
unsigned long int ttychn;
{
    int long max=2048;
    char tmpbuf[2049];

    if (!ttychn) return(-1);

    for (; max > 0; max -= 16) {
	if (!test_qio(ttychn,max,&tmpbuf)) return(max);
    }

    printf("\n%%CKERMIT-F-get_qio_maxbuf_size, Could not get maxbuf size\n");
    exit(SS$_ABORT);		/* Fatal exit */

}

int test_qio(ttychn,max,dest)
unsigned long int ttychn;
long int max;
unsigned char *dest;
{
    static int trmmsk[2] = {0,0};

/*    trmmsk[1] = 1 << eol; */

    vms_status = SYS$QIOW(QIOW_EFN, ttychn, IO$_READVBLK|IO$M_TIMED, &wrk_iosb, 0, 0,
			  dest, max, 0, &trmmsk, 0, 0);
    return( !(vms_status & 1) ||
	(!(wrk_iosb.status & 1)) && wrk_iosb.status != SS$_TIMEOUT);
}
