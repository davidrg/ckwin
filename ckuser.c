char *userv = "User Interface V4.0(034), 5 Feb 85";

/*  C K U S E R --  "User Interface" for Unix Kermit  */

/*  Frank da Cruz, Columbia University Center for Computing Activities, 1985 */
/*
 This module contains the terminal input and output functions for Unix
 Kermit.  It includes a simple Unix-style command line parser as well as
 an interactive prompting keyword command parser.  It depends on the existence
 of Unix facilities like fopen, fgets, feof, (f)printf, argv/argc, etc.  Other
 functions that are likely to vary among Unix implementations -- like setting
 terminal modes or interrupts -- are invoked via calls to functions that are
 defined in the system-dependent modules, ckx??? and ckz???.

 The command line parser processes any arguments found on the command line,
 as passed to main() via argv/argc.  The interactive parser uses the facilities
 of the cmd package (developed for this program, but usable by any program).

 Any command parser may be substituted for this one.  The only requirements
 for the Kermit command parser are these:

 1. Set parameters via global variables like duplex, speed, ttname, etc.
    See ckmain.c for the declarations and descriptions of these variables.

 2. If a command can be executed without the use of Kermit protocol, then
    execute the command directly and set the variable sstate to 0. Examples
    include 'set' commands, local directory listings, the 'connect' command.

 3. If a command requires the Kermit protocol, set the following variables:

    sstate                             string data
      'x' (enter server mode)            (none)
      'r' (send a 'get' command)         cmarg, cmarg2
      'v' (enter receive mode)           cmarg2
      'g' (send a generic command)       cmarg
      's' (send files)                   nfils, cmarg & cmarg2 OR cmlist
      'c' (send a remote host command)   cmarg

    cmlist is an array of pointers to strings.
    cmarg, cmarg2 are pointers to strings.
    nfils is an integer.    

    cmarg can be a filename string (possibly wild), or
       a pointer to a prefabricated generic command string, or
       a pointer to a host command string.
    cmarg2 is the name to send a single file under, or
       the name under which to store an incoming file; must not be wild.
    cmlist is a list of nonwild filenames, such as passed via argv.
    nfils is an integer, interpreted as follows:
      -1: argument string is in cmarg, and should be expanded internally.
       0: stdin.
      >0: number of files to send, from cmlist.

 The screen() function is used to update the screen during file transfer.
 The tlog() function maintains a transaction log.
 The debug() function maintains a debugging log.
 The intmsg() and chkint() functions provide the user i/o for interrupting
   file transfers.
*/


/* Includes */

#include "ckermi.h"
#include "ckcmd.h"

/* External Kermit Variables, see ckmain.c for description. */

extern int size, rpsiz, spsiz, npad, timint, speed, local, server, image, flow,
  displa, binary, fncnv, delay, parity, deblog, escape, xargc,
  turn, duplex, cxseen, czseen, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  filcnt, tfc, tlci, tlco, ffc, turnch, chklen, bctr, bctu, fsize, dfloc,
  rptflg, ebqflg, warn, quiet, cnflg, timef, mypadn;

extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *cmarg, *cmarg2, **xargv, **cmlist;
extern char mystch, sstate, mypadc, padch, eol, ctlq, filnam[], ttname[];
char *strcpy();

/* Declarations from cmd package */

extern char cmdbuf[];			/* Command buffer */

/* Declarations from ckz??? module */

extern char *SPACMD, *zhome();		/* Space command, home directory */

/* Variables and symbols local to this module */

char line[100], *lp;			/* Character buffer for anything */
char debfil[50];			/* Debugging log file name */
char pktfil[50];			/* Packet log file name */
char sesfil[50];			/* Session log file name */
char trafil[50];			/* Transaction log file name */

int n,					/* General purpose int */
    cflg,				/* Command-line connect cmd given */
    action,				/* Action selected on command line*/
    ncmd,				/* Number of commands */
    nprm,	    	    	    	/* Number of parameters */
    nrmt,				/* Number of remote commands */
    npar,				/* Number of kinds of parity */
    nlog,				/* Number of kinds of log files */
    nflo,				/* Kinds of flow control */
    nhsh,				/* Kinds of handshake */
    nfilp,				/* Number of file parameters */
    repars,				/* Reparse needed */
    tlevel,				/* Take command level */
    cwdf = 0;				/* CWD has been done */

#define MAXTAKE 20			/* Maximum nesting of TAKE files */
FILE *tfile[MAXTAKE];			/* File pointers for TAKE command */

char *homdir;				/* Pointer to home directory string */


/*
 Simple Unix-style command line parser, conforming with 'A Proposed Command
 Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World, Vol.1,
 No.3, 1984.
*/

char cmdstr[100];

char *hlp1 = "\
  Usage: kermit [-x arg [-x arg]...[-yyy]..]]\n\
   x is an option that requires an argument, y an option with no argument:\n\
     actions (* options also require -l and -b) --\n\
       -s file(s)   send (use '-s -' to send from stdin)\n\
       -r           receive\n\
       -k           receive to stdout\n\
     * -g file(s)   get remote file(s) from server (quote wildcards)\n\
       -a name      alternate name, used with -s, -r, -g\n\
       -x           enter server mode\n\
     * -f           finish remote server\n\
     * -c           connect before transaction\n\
     * -n           connect after transaction\n\
       -h           help - print this message\n";

char *hlp2 = "\
     settings --\n\
       -l line      communication line device\n\
       -b baud      line speed, e.g. 1200\n\
       -i           binary file or Unix-to-Unix\n\
       -p x         parity, x is one of e,o,m,s,n\n\
       -t           line turnaround handshake = xon, half duplex\n\
       -w           don't write over preexisting files\n\
       -q   	    be quiet during file transfer\n\
       -d   	    log debugging info to debug.log\n\
 If no action command is included, enter interactive dialog.\n";


/*  U S A G E  */

usage() {
    conol(hlp1);
    conol(hlp2);
}


/*  C M D L I N  --  Get arguments from command line  */

cmdlin() {
    char x;

    cmarg = "";				/* Initialize. */
    cmarg2 = "";
    action = cflg = 0;

    while (--xargc > 0) {		/* Go through command line words */
	*xargv++;
	debug(F111,"xargv",*xargv,xargc);
    	if (**xargv == '-') {		/* Got an option (begins with dash) */
	    x = *(*xargv+1);		/* Get the option letter */
	    x = doarg(x);		/* Go handle the option */
	    if (x < 0) exit(0);
    	} else {			/* No dash where expected */
	    usage();
	    exit(1);
	}
    }
    debug(F101,"action","",action);
    if (!local) {
	if ((action == 'g') || (action == 'r') ||
	    (action == 'c') || (cflg != 0))
	    fatal("-l and -b required");
    }
    if (*cmarg2 != 0) {
	if ((action != 's') && (action != 'r') &&
	    (action != 'v'))
	    fatal("-a without -s, -r, or -g");
    }
    if ((action == 'v') && (stdouf) && (!local)) {
    	if (isatty(1))
	    fatal("unredirected -k can only be used in local mode");
    }
    if ((action == 's') || (action == 'v') ||
    	(action == 'r') || (action == 'x')) {
	if (local) displa = 1;
	if (stdouf) displa = 0;
    }

    if (quiet) displa = 0;		/* No display if quiet requested */

    if (cflg) {
	conect();			/* Connect if requested */
	if (action == 0) {
	    if (cnflg) conect();	/* And again if requested */
	    doexit();
	}
    }
    if (displa) concb();		/* (for console "interrupts") */
    return(action);			/* Then do any requested protocol */
}


/*  D O A R G  --  Do a command-line argument.  */

doarg(x) char x; {
    int z; char *xp;

    xp = *xargv+1;			/* Pointer for bundled args */
    while (x) {
	switch (x) {

case 'x':				/* server */
    if (action) fatal("conflicting actions");
    action = 'x';
    break;

case 'f':
    if (action) fatal("conflicting actions");
    action = setgen('F',"","","");
    break;

case 'r':				/* receive */
    if (action) fatal("conflicting actions");
    action = 'v';
    break;

case 'k':				/* receive to stdout */
    if (action) fatal("conflicting actions");
    stdouf = 1;
    action = 'v';
    break;

case 's': 				/* send */
    if (action) fatal("conflicting actions");
    if (*(xp+1)) fatal("invalid argument bundling after -s");
    z = nfils = 0;			/* Initialize file counter, flag */
    cmlist = xargv+1;			/* Remember this pointer */
    while (--xargc > 0) {		/* Traverse the list */	
	*xargv++;
	if (**xargv == '-') {		/* Check for sending stdin */
	    if (strcmp(*xargv,"-") != 0) break;
	    z++;
        }
	nfils++;			/* Bump file counter */
    }
    xargc++, *xargv--;			/* Adjust argv/argc */
    if (nfils < 1) fatal("missing filename for -s");
    if (z > 1) fatal("-s: too many -'s");
    if (z == 1) {
	if (nfils == 1) nfils = 0;
	else fatal("invalid mixture of filenames and '-' in -s");
    }
    if (nfils == 0) {
	if (isatty(0)) fatal("sending from terminal not allowed");
    }
    debug(F101,*xargv,"",nfils);
    action = 's';
    break;

/* cont'd... */


/* ...doarg(), cont'd */

case 'g':				/* get */
    if (action) fatal("conflicting actions");
    if (*(xp+1)) fatal("invalid argument bundling after -g");
    *xargv++, xargc--;
    if ((xargc == 0) || (**xargv == '-'))
    	fatal("missing filename for -g");
    cmarg = *xargv;
    action = 'r';
    break;

case 'c':				/* connect before */
    cflg = 1;
    break;

case 'n':				/* connect after */
    cnflg = 1;
    break;

case 'h':				/* help */
    usage();
    return(-1);

case 'a':				/* "as" */
    if (*(xp+1)) fatal("invalid argument bundling after -a");
    *xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing name in -a");
    cmarg2 = *xargv;
    break;

case 'l':				/* set line */
    if (*(xp+1)) fatal("invalid argument bundling after -l");
    *xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("communication line device name missing");
    strcpy(ttname,*xargv);
    if (strcmp(ttname,dftty) == 0) local = dfloc; else local = 1;
    break;

case 'b':   	    			/* set baud */
    if (*(xp+1)) fatal("invalid argument bundling");
    *xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing baud");
    z = atoi(*xargv);			/* Convert to number */
    if (chkspd(z) > -1) speed = z;	/* Check it */
    	else fatal("unsupported baud rate");
    break;

case 'i':				/* Treat files as binary */
    binary = 1;
    break;

/* cont'd... */


/* ...doarg(), cont'd */


case 'w':				/* File warning */
    warn = 1;
    break;

case 'q':				/* Quiet */
    quiet = 1;
    break;

case 'd':				/* debug */
    debopn("debug.log");
    break;

case 'p':				/* set parity */
    if (*(xp+1)) fatal("invalid argument bundling");
    *xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing parity");
    switch(x = **xargv) {
	case 'e':
	case 'o':
	case 'm':
	case 's': parity = x; break;
	case 'n': parity = 0; break;
	default:  fatal("invalid parity");
        }
    break;

case 't':
    turn = 1;				/* Line turnaround handshake */
    turnch = XON;			/* XON is turnaround character */
    duplex = 1;				/* Half duplex */
    flow = 0;				/* No flow control */
    break;

default:
    fatal("invalid argument, type 'kermit -h' for help");
        }

    x = *++xp;				/* See if options are bundled */
    }
    return(0);
}


/* Misc */

fatal(msg) char *msg; {			/* Fatal error message */
    fprintf(stderr,"\r\nFatal: %s\n",msg);
    tlog(F110,"Fatal:",msg,0);
    doexit();
}


ermsg(msg) char *msg; {			/* Print error message */
    if (!quiet) fprintf(stderr,"\r\nError - %s\n",msg);
    tlog(F110,"Error -",msg,0);
}


/* Interactive command parser */ 


/* Values associated with top-level commands, must be 0 or greater. */

#define XXBYE   0	/* BYE */
#define XXCLE   1	/* CLEAR */
#define XXCLO   2	/* CLOSE */
#define XXCON   3	/* CONNECT */
#define XXCPY   4	/* COPY */
#define XXCWD   5	/* CWD (Change Working Directory) */
#define XXDEF	6	/* DEFINE (a command macro) */
#define XXDEL   7	/* (Local) DELETE */
#define XXDIR   8	/* (Local) DIRECTORY */
#define XXDIS   9	/* DISCONNECT */
#define XXECH  10	/* ECHO */
#define XXEXI  11	/* EXIT */
#define XXFIN  12	/* FINISH */
#define XXGET  13	/* GET */
#define XXHLP  14	/* HELP */
#define XXINP  15	/* INPUT */
#define XXLOC  16	/* LOCAL */
#define XXLOG  17	/* LOG */
#define XXMAI  18	/* MAIL */
#define XXMOU  19	/* (Local) MOUNT */
#define XXMSG  20	/* (Local) MESSAGE */
#define XXOUT  21	/* OUTPUT */
#define XXPAU  22	/* PAUSE */
#define XXPRI  23	/* (Local) PRINT */
#define XXQUI  24	/* QUIT */
#define XXREC  25	/* RECEIVE */
#define XXREM  26	/* REMOTE */
#define XXREN  27	/* (Local) RENAME */
#define XXSEN  28	/* SEND */
#define XXSER  29   	/* SERVER */
#define XXSET  30	/* SET */
#define XXSHE  31	/* Command for SHELL */
#define XXSHO  32	/* SHOW */
#define XXSPA  33	/* (Local) SPACE */
#define XXSTA  34	/* STATISTICS */
#define XXSUB  35	/* (Local) SUBMIT */
#define XXTAK  36	/* TAKE */
#define XXTRA  37	/* TRANSMIT */
#define XXTYP  38	/* (Local) TYPE */
#define XXWHO  39	/* (Local) WHO */


/* Top-Level Keyword Table */

struct keytab cmdtab[] = {
    "!",	   XXSHE, 0,
    "bye",         XXBYE, 0,
    "c",           XXCON, CM_INV,
    "close",	   XXCLO, 0,
    "connect",     XXCON, 0,
    "cwd",	   XXCWD, 0,
    "directory",   XXDIR, 0,
    "echo",        XXECH, 0,
    "exit",	   XXEXI, 0,
    "finish",	   XXFIN, 0,
    "get",	   XXGET, 0,
    "help",	   XXHLP, 0,
    "log",  	   XXLOG, 0,
    "quit",	   XXQUI, 0,
    "r",           XXREC, CM_INV,
    "receive",	   XXREC, 0,
    "remote",	   XXREM, 0,
    "s",           XXSEN, CM_INV,
    "send",	   XXSEN, 0,
    "server",	   XXSER, 0,
    "set",	   XXSET, 0,
    "show", 	   XXSHO, 0,
    "space",       XXSPA, 0,
    "statistics",  XXSTA, 0,
    "take",	   XXTAK, 0
};

/* Number of Top-Level Keywords */

#define NCMD (sizeof(cmdtab) / sizeof(struct keytab))


/* SET parameters */

#define XYBREA  0	/* BREAK simulation */
#define XYCHKT  1	/* Block check type */
#define XYDEBU  2	/* Debugging */
#define XYDELA  3	/* Delay */
#define XYDUPL  4	/* Duplex */
#define XYEOL   5	/* End-Of-Line (packet terminator) */
#define XYESC   6	/* Escape character */
#define XYFILE  7	/* File Parameters */

#define XYFLOW  9	/* Flow Control */
#define XYHAND 10	/* Handshake */
#define XYIFD  11	/* Incomplete File Disposition */
#define XYIMAG 12	/* "Image Mode" */
#define XYINPU 13	/* INPUT command parameters */
#define XYLEN  14	/* Maximum packet length to send */
#define XYLINE 15	/* Communication line to use */
#define XYLOG  16	/* Log file */
#define XYMARK 17	/* Start of Packet mark */
#define XYNPAD 18	/* Amount of padding */
#define XYPADC 19	/* Pad character */
#define XYPARI 20	/* Parity */
#define XYPAUS 21	/* Interpacket pause */
#define XYPROM 22	/* Program prompt string */
#define XYQBIN 23	/* 8th-bit prefix */
#define XYQCTL 24	/* Control character prefix */
#define XYREPT 25	/* Repeat count prefix */
#define XYRETR 26	/* Retry limit */
#define XYSPEE 27	/* Line speed (baud rate) */
#define XYTACH 28	/* Character to be doubled */
#define XYTIMO 29	/* Timeout interval */


/* Parameter keyword table */

struct keytab prmtab[] = {
    "baud",	        XYSPEE,  CM_INV,
    "block-check",  	XYCHKT,  0,
    "delay",	    	XYDELA,  0,
    "duplex",	    	XYDUPL,  0,
    "end-of-packet",    XYEOL,   0,
    "escape-character", XYESC,   0,
    "file", 	  	XYFILE,  0,
    "flow-control", 	XYFLOW,  0,
    "handshake",    	XYHAND,  0,
    "line",             XYLINE,  0,
    "packet-length",    XYLEN,   0,
    "pad-character",    XYPADC,  0,
    "padding",          XYNPAD,  0,
    "parity",	    	XYPARI,  0,
    "prompt",	    	XYPROM,  0,
    "speed",	        XYSPEE,  0,
    "start-of-packet",  XYMARK,  0,
    "timeout",	        XYTIMO,  0
};
#define NPRM (sizeof(prmtab) / sizeof(struct keytab)) /* How many parameters */


/* REMOTE command symbols */

#define XZCPY  0	/* Copy */
#define XZCWD  1	/* Change Working Directory */
#define XZDEL  2	/* Delete */
#define XZDIR  3	/* Directory */
#define XZHLP  4	/* Help */
#define XZHOS  5	/* Host */
#define XZKER  6	/* Kermit */
#define XZLGI  7	/* Login */
#define XZLGO  8	/* Logout */
#define XZMAI  9	/* Mail */
#define XZMOU 10	/* Mount */
#define XZMSG 11	/* Message */
#define XZPRI 12	/* Print */
#define XZREN 13	/* Rename */
#define XZSET 14	/* Set */
#define XZSPA 15	/* Space */
#define XZSUB 16	/* Submit */
#define XZTYP 17	/* Type */
#define XZWHO 18	/* Who */

/* Remote Command Table */

struct keytab remcmd[] = {
    "cwd",       XZCWD, 0,
    "delete",    XZDEL, 0,
    "directory", XZDIR, 0,
    "help",      XZHLP, 0,
    "host",      XZHOS, 0,
    "space",	 XZSPA, 0,
    "type", 	 XZTYP, 0,
    "who",  	 XZWHO, 0
};

/* Number of Elements */

#define NRMT (sizeof(remcmd) / sizeof(struct keytab))


/* Miscellaneous keyword tables */


/* On/Off table */

struct keytab onoff[] = {
    "off",       0, 0,
    "on",        1, 0
};


/* Duplex keyword table */

struct keytab dpxtab[] = {
    "full", 	 0, 0,
    "half",      1, 0
};


/* Parity keyword table */

struct keytab partab[] = {
    "even",    'e', 0,
    "mark",    'm', 0,
    "none",      0, 0,
    "odd",     'o', 0,
    "space",   's', 0
};


/* Show table */

#define SHPAR 0
#define SHVER 1

struct keytab shotab[] = {
    "parameters", SHPAR, 0,
    "versions",   SHVER, 0
};


/* Logging keyword table */

#define LOGD 0
#define LOGP 1
#define LOGS 2
#define LOGT 3

struct keytab logtab[] = {
    "debugging",    LOGD, 0,
    "packets",	    LOGP, 0,
    "session",      LOGS, 0,
    "transactions", LOGT, 0
};


/*  More keyword tables  */


/* Flow Control */

struct keytab flotab[] = {
    "none",     0, 0,
    "xon/xoff", 1, 0
};
#define NFLO (sizeof(flotab) / sizeof(struct keytab))


/*  Handshake characters  */

struct keytab hshtab[] = {
    "bell", 007, 0,
    "cr",   015, 0,
    "esc",  033, 0,
    "lf",   012, 0,
    "none", 999, 0,  /* (can't use negative numbers) */
    "xoff", 023, 0,
    "xon",  021, 0
};
#define NHSH (sizeof(hshtab) / sizeof(struct keytab))

/* File parameters */

#define XYFILN 0
#define XYFILT 1
#define XYFILW 2
#define XYFILD 3

struct keytab filtab[] = {
    "display", XYFILD, 0,
    "names",   XYFILN, 0,
    "type",    XYFILT, 0,
    "warning", XYFILW, 0
};
#define NFILP (sizeof(filtab) / sizeof(struct keytab))

struct keytab fntab[] = {   		/* File naming */
    "converted", 1, 0,
    "literal",   0, 0
};

struct keytab fttab[] = {		/* File types */
    "binary",    1, 0,
    "text",      0, 0
};


/* Block checks */

struct keytab blktab[] = {
    "1", 1, 0,
    "2", 2, 0,
    "3", 3, 0
};


/*  C M D I N I  --  Initialize the interactive command parser  */

cmdini() {

    printf("%s\nType ? for help\n",versio);
    cmsetp("C-Kermit>");		/* Set default prompt. */

    ncmd = NCMD;			/* Perform these calculations */
    nprm = NPRM;			/* only once... */
    nrmt = NRMT;
    nhsh = NHSH;
    nflo = NFLO;
    nfilp = NFILP;			/* Number of file parameters */
    npar = 5;				/* Five kinds of parity */
    nlog = 4;				/* Four kinds of logs */
    tlevel = -1;			/* Take file level */

/* Look for init file ".kermrc" in home or current directory. */

    homdir = zhome();
    lp = line;
    if (homdir)
	sprintf(lp,"%s/.kermrc",homdir);
    else
    	sprintf(lp,".kermrc");
    if ((tfile[0] = fopen(line,"r")) != NULL) {
	tlevel = 0;
	debug(F110,"init file",line,0);
    }
    if (homdir && (tlevel < 0)) {
    	sprintf(lp,".kermrc");
	if ((tfile[0] = fopen(line,"r")) != NULL) {
	    tlevel = 0;
	    debug(F110,"init file",line,0);
	} else {
	    debug(F100,"no init file","",0);
        }
    }

/* Set up tty for interactive command parsing */

     congm();				/* Get console modes */
     concb();				/* Put in cbreak/noecho mode */
}


/*  T R A P  --  Terminal interrupt handler */

trap() {
    debug(F100,"terminal interrupt...","",0);
    doexit();
}


/* P A R S E R  */

parser() {
    int xx;

    conint(trap);		/* Turn on console terminal interrupts. */

/*
 sstate becomes nonzero when a command has been parsed that requires
 some action from the protocol module.  Any non-protocol actions, such as
 local directory listing or terminal emulation, are invoked directly from
 below.
*/
    if (local) printf("\n");		/*** Temporary kludge **/
     sstate = 0;			/* Start with no start state. */
     while (sstate == 0) {		/* Parse cmds until action requested */

	while (feof(tfile[tlevel]) && (tlevel > -1)) { /* If end of take */
		fclose(tfile[tlevel]);	/* file, close it */
		tlevel--;		/* and forget about it. */
		cmini(ckxech);		/* and clear the cmd buffer. */
 	}
	if (tlevel > -1) {
	    if (fgets(cmdbuf,CMDBL,tfile[tlevel]) == NULL) continue;
	} else {			/* Otherwise. */
	    prompt();			/* Issue interactive prompt. */
	    cmini(ckxech);
    	}
	repars = 1;
	displa = 0;

	while (repars) {
	    cmres();			/* Reset buffer pointers. */
	    xx = cmkey(cmdtab,ncmd,"Command","");
	    debug(F101,"top-level cmkey","",xx);
	    switch (docmd(xx)) {
		case -4:		/* EOF */
		    doexit();
	        case -1:		/* Reparse needed */
		    repars = 1;
		    continue;
	    	case -2:		/* Invalid command given */
		    cmini(ckxech);	/* (fall thru) */
 	    	case -3:		/* Empty command OK at top level */
		default:		/* Anything else (fall thru) */
		    repars = 0;		/* No reparse, get new command. */
		    continue;
            }
        }
    }

/* Got an action command; disable terminal interrupts and return start state */

    if (!local) connoi();		/* Interrupts off only if remote */
    return(sstate);
}


/*  D O E X I T  --  Exit from the program.  */

doexit() {
    if (local) {
	ttclos();			/* Close external line */
	strcpy(ttname,dftty);		/* Restore default tty */
	local = dfloc;			/* And default remote/local status */
	}
    if (!quiet) conres();		/* Restore console terminal. */
    if (!quiet) connoi();		/* Turn off console interrupt traps. */

    if (deblog) {			/* Close any open logs. */
	debug(F100,"Debug Log Closed","",0);
	*debfil = '\0';
	deblog = 0;
	zclose(ZDFILE);
    }
    if (pktlog) {
	*pktfil = '\0';
	pktlog = 0;
	zclose(ZPFILE);
    }
    if (seslog) {
    	*sesfil = '\0';
	seslog = 0;
	zclose(ZSFILE);
    }
    if (tralog) {
	tlog(F100,"Transaction Log Closed","",0);
	*trafil = '\0';
	tralog = 0;
	zclose(ZTFILE);
    }
    exit(0);				/* Exit from the program. */
}


/*  B L D L E N  --  Make length-encoded copy of string  */

char *
bldlen(str,dest) char *str, *dest; {
    int len;
    len = strlen(str);
    *dest = tochar(len);
    strcpy(dest+1,str);
    return(dest+len+1);
}


/*  S E T G E N  --  Construct a generic command  */

setgen(type,arg1,arg2,arg3) char type, *arg1, *arg2, *arg3; {
    char *upstr, *cp;

    cp = cmdstr;
    *cp++ = type;
    *cp = NUL;
    if (*arg1 != NUL) {
	upstr = bldlen(arg1,cp);
	if (*arg2 != NUL) {
	    upstr = bldlen(arg2,upstr);
	    if (*arg3 != NUL) bldlen(arg3,upstr);
	}
    }
    cmarg = cmdstr;
    debug(F110,"setgen",cmarg,0);

    return('g');
}


/*  D O C M D  --  Do a command  */

/*
 Returns:
   -2: user typed an illegal command
   -1: reparse needed
    0: parse was successful (even tho command may have failed).
*/ 

docmd(cx) int cx; {
    int x, y;
    char *s;

    switch (cx) {

case -4:				/* EOF */
    if (!quiet) printf("\r\n");
    doexit();
case -3:				/* Null command */
    return(0);
case -2:				/*
 Error */
case -1:				/* Reparse needed */
    return(cx);

case XXBYE:				/* bye */
    if ((x = cmcfm()) < 0) return(x);
    if (!local) {
	printf("You have to 'set line' first\n");
	return(0);
    }
    sstate = setgen('L',"","","");
    return(0);

case XXCON:                     	/* connect */
    if ((x = cmcfm()) < 0) return(x);
    conres();				/* restore tty to normal mode */
    conect();
    concb();				/* tty back in character mode */
    return(0);

case XXCWD:
    if (cmtxt("Name of local directory, or carriage return",homdir,&s) < 0)
    	return(-1);    
    if (chdir(s)) perror(s);
    cwdf = 1;
    system("pwd");
    return(0);


case XXCLO:
    x = cmkey(logtab,nlog,"Which log to close","");
    if (x == -3) {
	printf("?You must tell which log\n");
	return(-2);
    }
    if (x < 0) return(x);
    if ((y = cmcfm()) < 0) return(y);
    switch (x) {

	case LOGD:
	    if (deblog == 0) {
		printf("?Debugging log wasn't open\n");
		return(0);
	    }
	    *debfil = '\0';
	    deblog = 0;
	    return(zclose(ZDFILE));

	case LOGP:
	    if (pktlog == 0) {
		printf("?Packet log wasn't open\n");
		return(0);
	    }
	    *pktfil = '\0';
	    pktlog = 0;
	    return(zclose(ZPFILE));

	case LOGS:
	    if (seslog == 0) {
		printf("?Session log wasn't open\n");
		return(0);
	    }
	    *sesfil = '\0';
	    seslog = 0;
	    return(zclose(ZSFILE));

    	case LOGT:
	    if (tralog == 0) {
		printf("?Transaction log wasn't open\n");
		return(0);
	    }
	    *trafil = '\0';
	    tralog = 0;
	    return(zclose(ZTFILE));

	default:
	    printf("\n?Unexpected log designator - %d\n", x);
	    return(0);
    }


case XXDIR:				/* directory */
    if ((x = cmifi("Directory/file specification","*",&s,&y)) < 0) return(x);
    lp = line;
    sprintf(lp,"ls -l %s",s);
    system(line);
    return(0);

case XXECH:				/* echo */
    x = cmtxt("Material to be echoed","",&s);
    if (x < 0) return(x);
    printf("%s\n",s);
    return(0);

case XXQUI:				/* quit, exit */
case XXEXI:
    if ((x = cmcfm()) > -1) doexit();
    else return(x);

case XXFIN:				/* finish */
    if ((x = cmcfm()) < 0) return(x);
    if (!local) {
	printf("You have to 'set line' first\n");
	return(0);
    }
    sstate = setgen('F',"","","");
    return(0);


case XXGET:				/* Get */
    if (!local) {
	printf("\nYou have to 'set line' first\n");
	return(0);
    }
    x = cmtxt("Name of remote file(s), or carriage return","",&cmarg);
    if ((x == -2) || (x == -1)) return(x);

/* If foreign file name omitted, get foreign and local names separately */

    if (*cmarg == NUL) {

	if (tlevel > -1) {		/* Input is from take file */

	    if (fgets(line,100,tfile[tlevel]) == NULL)
	    	fatal("take file ends prematurely in 'get'");
	    cmarg = line;
	    if (fgets(cmdbuf,CMDBL,tfile[tlevel]) == NULL)
	    	fatal("take file ends prematurely in 'get'");
	    if (*cmdbuf == NUL) cmarg2 = line; else cmarg2 = cmdbuf;

        } else {			/* Input is from terminal */

	    char psave[40];		/* Save old prompt */
	    cmsavp(psave,40);
	    cmsetp(" Remote file specification: "); /* Make new one */
	    cmini(ckxech);
	    x = -1;
	    while (x < 0) {		/* Prompt till they answer */
	    	prompt();
	    	x = cmtxt("Name of remote file(s)","",&cmarg);
	    	if (*cmarg == NUL) x = -1;
	    }
	    strcpy(line,cmarg);		/* Make a safe copy */
	    cmarg = line;
	    cmsetp(" Local name to store it under: ");	/* New prompt */
	    cmini(ckxech);
	    x = -1;
	    while (x < 0) {		/* Again, prompt till answered */
	    	prompt();
	    	x = cmofi("Local file name",cmarg,&cmarg2);
            }
	    cmsetp(psave);		/* Restore old prompt. */
	    if ((x == cmcfm()) < 0) return(-2);
        }
    }
    sstate = 'r';			/* All ok, set start state. */
    if (local) displa = 1;
    return(0);


case XXHLP:				/* Help */
    x = cmkey(cmdtab,ncmd,"C-Kermit command","help");
    return(dohlp(x));

case XXLOG:				/* Log */
    x = cmkey(logtab,nlog,"What to log","");
    if (x == -3) {
	printf("?You must specify what is to be logged\n");
	return(-2);
    }
    if (x < 0) return(x);
    return(dolog(x));

case XXREC:				/* Receive */
    cmarg2 = "";
    x = cmofi("Name under which to store the file, or CR","",&cmarg2);
    if ((x == -1) || (x == -2)) return(x);
    debug(F111,"cmofi cmarg2",cmarg2,x);
    if ((x = cmcfm()) < 0) return(x);
    sstate = 'v';
    if (local) displa = 1;
    return(0);

case XXREM:				/* Remote */
    if (!local) {
	printf("\nYou have to 'set line' first\n");
	return(-2);
    }
    x = cmkey(remcmd,nrmt,"Remote Kermit server command","");
    if (x == -3) {
	printf("?You must specify a command for the remote server\n");
	return(-2);
    }
    return(dormt(x));

case XXSEN:				/* Send */
    cmarg = cmarg2 = "";
    if ((x = cmifi("File(s) to send","",&s,&y)) < 0) {
	if (x == -3) {
	    printf("?A file specification is required\n");
	    return(-2);
	}
	return(x);
    }
    nfils = -1;				/* Files come from internal list. */
    strcpy(line,s);			/* Save copy of string just parsed. */
    debug(F101,"Send: wild","",y);
    *cmarg2 = '\0';			/* Initialize send-as name */
    if (y == 0) {
	if ((x = cmfld("Name to send it with",line,&cmarg2)) < 0) return(x);
    }
    if ((x = cmcfm()) < 0) return(x);
    cmarg = line;			/* File to send */
    debug(F110,"Sending:",cmarg,0);
    debug(F110," as:",cmarg2,0);
    sstate = 's';			/* Set start state */
    if (local) displa = 1;
    return(0);


case XXSER:				/* Server */
    if (x = (cmcfm()) < 0) return(x);
    sstate = 'x';
    if (local) displa = 1;
    return(0);

case XXSET:				/* Set */
    x = cmkey(prmtab,nprm,"Parameter","");
    if (x == -3) {
	printf("?You must specify a parameter to set\n");
	return(-2);
    }
    if (x < 0) return(x);
    return(doprm(x));
    
case XXSHE:				/* Local shell command */
    if (cmtxt("Unix shell command to execute","",&s) < 0) return(-1);
    conres();				/* Make console normal  */
    system(s);				/* Execute the command */
    concb();				/* Console back in cbreak mode */
    return(0);


case XXSHO:				/* Show */
    x = cmkey(shotab,2,"","parameters");
    if (x < 0) return(x);
    if (y = (cmcfm()) < 0) return(y);
    switch (x) {
	case SHPAR:
	    printf("\nLine: %s, speed: %d, mode: ",ttname,speed);
	    if (local) printf("local"); else printf("remote");
	    printf("\n Parity: ");
	    switch (parity) {
		case 'e': printf("even");  break;
		case 'o': printf("odd");   break;
		case 'm': printf("mark");  break;
		case 's': printf("space"); break;
		case 0:   printf("none");  break;
		default:  printf("invalid - %d",parity); break;
    	    }		
	    printf(", duplex: ");
	    if (duplex) printf("half, "); else printf("full, ");
	    printf("flow: ");
	    if (flow == 1) printf("xon/xoff");
	    	else if (flow == 0) printf("none");
		else printf("%d",flow);
	    printf(", handshake: ");
	    if (turn) printf("%d\n",turnch); else printf("none\n");
	    printf(" Timeout: %d, delay: %d\n",timint,delay);
	    printf(" Padding: %d, pad character: %d\n",mypadn,mypadc);
            printf(" Packet start: %d, end: %d, length: %d",mystch,eol,spsiz);
	    printf(", block check: %d\n",bctr);
	    printf("\nFile parameters:\n");
	    printf(" Names:        ");
	    if (fncnv) printf("converted\n"); else printf("literal\n");
	    printf(" Type:         ");
	    if (binary) printf("binary\n"); else printf("text\n");
	    printf(" Warning:      ");
	    if (warn) printf("on\n"); else printf("off\n");
	    printf(" Display:      ");
	    if (quiet) printf("off\n"); else printf("on\n");
	    printf("\nLogs:");
	    printf("\n Debugging:    ");
	    if (deblog) printf("%s",debfil); else printf("none");
	    printf("\n Packets:      ");
	    if (pktlog) printf("%s",pktfil); else printf("none");
	    printf("\n Session:      ");
	    if (seslog) printf("%s",sesfil); else printf("none");
	    printf("\n Transactions: ");
	    if (tralog) printf("%s",trafil); else printf("none");
	    printf("\n\n");
	    break;
	case SHVER:
	    printf("\nVersions:\n %s\n %s\n %s\n",versio,protv,fnsv);
	    printf(" %s\n %s\n %s\n %s\n %s\n\n",cmdv,userv,ckxv,ckzv,connv);
	    break;
	default:
	    printf("\nNothing to show...\n");
	    break;
    }	    
    return(0);


case XXSPA:				/* space */
    if (x = (cmcfm()) < 0) return(x);
    system(SPACMD);
    return(0);

case XXSTA:				/* statistics */
    if (x = (cmcfm()) < 0) return(x);
    printf("\nMost recent transaction --\n");
    printf(" files: %d\n",filcnt);
    printf(" total file characters  : %d\n",tfc);
    printf(" communication line in  : %d\n",tlci);
    printf(" communication line out : %d\n\n",tlco);
    printf(" block check type used  : %d\n",bctu);
    printf(" compression            : ");
    if (rptflg) printf("yes\n"); else printf("no\n");
    printf(" 8th bit prefixing      : ");
    if (ebqflg) printf("yes\n"); else printf("no\n\n");
    return(0);

case XXTAK:				/* take */
    if (tlevel > MAXTAKE-1) {
	printf("?Take files nested too deeply\n");
	return(-2);
    }
    if ((y = cmifi("C-Kermit command file","",&s,&x)) < 0) { 
	if (y == -3) {
	    printf("?A file specification is required\n");
	    return(-2);
	} else return(y);
    }
    if (x != 0) {
	printf("?Wildcards not allowed in command file name\n");
	return(-2);
    }
    strcpy(line,s);			/* Make a safe copy of the string */
    if ((y = cmcfm()) < 0) return(y);
    if ((tfile[++tlevel] = fopen(line,"r")) == NULL) {
	perror("take");
	printf("Can't open command file - %s\n",line);
	debug(F110,"Failure to open",line,0);
	tlevel--;
	return(0);
    }
    return(0);

default:
    printf("Not available yet - %s\n",cmdbuf);
    return(-2);
    }
}


/*  D O L O G  --  */

dolog(x) int x; {
    int y; char *s;

    switch (x) {

	case LOGD:
	    y = cmofi("Name of debugging log file","debug.log",&s);
	    break;

	case LOGP:
	    y = cmofi("Name of packet log file","packet.log",&s);
	    break;

	case LOGS:
	    y = cmofi("Name of session log file","session.log",&s);
	    break;

	case LOGT:
	    y = cmofi("Name of transaction log file","transaction.log",&s);
	    break;

	default:
	    printf("\n?Unexpected log designator - %d\n",x);
	    return(-2);
    }
    if (y < 0) return(y);

    strcpy(line,s);
    s = line;
    if ((y = cmcfm()) < 0) return(y);

/* cont'd... */


/* ...dolog, cont'd */


    switch (x) {

	case LOGD:
	    return(deblog = debopn(s));

	case LOGP:
	    zclose(ZPFILE);
	    y = zopeno(ZPFILE,s);
	    if (y > 0) strcpy(pktfil,s); else *pktfil = '\0';
	    return(pktlog = y);

	case LOGS:
	    zclose(ZSFILE);
	    y = zopeno(ZSFILE,s);
	    if (y > 0) strcpy(sesfil,s); else *sesfil = '\0';
	    return(seslog = y);

	case LOGT:
	    zclose(ZTFILE);
	    tralog = zopeno(ZTFILE,s);
	    if (tralog > 0) {
		strcpy(trafil,s);
		tlog(F110,"Transaction Log:",versio,0);
		ztime(&s);
		tlog(F100,s,"",0);
    	    }
	    else *trafil = '\0';
	    return(tralog);

	default:
	    return(-2);
    }
}


/*  D E B O P N  --  Open a debugging file  */

debopn(s) char *s; {
    char *tp;
    zclose(ZDFILE);
    deblog = zopeno(ZDFILE,s);
    if (deblog > 0) {
	strcpy(debfil,s);
	debug(F110,"Debug Log ",versio,0);
	ztime(&tp);
	debug(F100,tp,"",0);
    } else *debfil = '\0';
    return(deblog);
}


/*  D O P R M  --  Set a parameter.  */
/*
 Returns:
  -2: illegal input
  -1: reparse needed
   0: success
*/
doprm(xx) int xx; {
    int x, y, z;
    char *s;

switch (xx) {

case XYLINE:
    if ((x = cmtxt("Device name",dftty,&s)) < 0) return(x);
    strcpy(ttname,s);
    if (strcmp(ttname,dftty) == 0) local = dfloc; else local = 1;
    return(0);

case XYCHKT:
    if ((y = cmkey(blktab,3,"","1")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    bctr = y;
    return(0);

case XYDEBU:
    return(seton(&deblog));

case XYDELA:
    y = cmnum("Number of seconds before starting to send","5",10,&x);
    debug(F101,"XYDELA: y","",y);
    return(setnum(&delay,x,y));

case XYDUPL:
    if ((y = cmkey(dpxtab,2,"","full")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    duplex = y;
    return(0);

case XYEOL:
    y = cmnum("Decimal ASCII code for packet terminator","0",10,&x);
    y = setcc(&z,x,y);
    eol = z;
    return(y);

case XYESC:
    y = cmnum("Decimal ASCII code for escape character","",10,&x);
    return(setcc(&escape,x,y));


case XYFILE:
    if ((y = cmkey(filtab,nfilp,"File parameter","")) < 0) return(y);
    switch (y) {
	int z;
	case XYFILD:			/* Display */
	    y = seton(&z);
	    if (y < 0) return(y);
	    quiet = !z;
	    return(0);

	case XYFILN:			/* Names */
	    if ((x = cmkey(fntab,2,"how to handle filenames","converted")) < 0)
	    	return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    fncnv = x;
	    return(0);

	case XYFILT:			/* Type */
	    if ((x = cmkey(fttab,2,"type of file","text")) < 0)
	    	return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    binary = x;
	    return(0);

	case XYFILW:			/* Warning/Write-Protect */
	    return(seton(&warn));
    }

case XYFLOW:				/* Flow control */
    if ((y = cmkey(flotab,nflo,"","xon/xoff")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    flow = y;
    return(0);

case XYHAND:				/* Handshake */
    if ((y = cmkey(hshtab,nhsh,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    turn = (y > 0127) ? 0 : 1 ;
    turnch == y;
    return(0);

case XYLEN:
    y = cmnum("Maximum number of characters in a packet","80",10,&x);
    return(setnum(&spsiz,x,y));

case XYMARK:
    y = cmnum("Decimal ASCII code for packet-start character","1",10,&x);
    y = setcc(&z,x,y);
    mystch = z;
    return(y);

case XYNPAD:
    y = cmnum("How many padding characters for inbound packets","0",10,&x);
    return(setnum(&mypadn,x,y));


case XYPADC:
    y = cmnum("Decimal ASCII code for inbound pad character","0",10,&x);
    y = setcc(&z,x,y);
    mypadc = z;
    return(y);

case XYPARI:
    if ((y = cmkey(partab,npar,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    parity = y;
    ebqflg = 1;				/* Flag we want 8th-bit prefixing */
    return(0);

case XYPROM:
    if ((x = cmtxt("Program's command prompt","C-Kermit>",&s)) < 0) return(x);
    cmsetp(s);
    return(0);

case XYSPEE:
    if (!local) {
	printf("\nSpeed setting can only be done on an external line\n");
	printf("You must 'set line' before issuing this command\n");
	return(0);
    }	
    lp = line;
    sprintf(lp,"Baud rate for %s",ttname);
    if ((y = cmnum(line,"",10,&x)) < 0) return(y);
    if (y = (cmcfm()) < 0) return(y);
    y = chkspd(x);
    if (y < 0) 
    	printf("?Unsupported line speed - %d\n",x);
    else {
    	speed = y;
	printf("%s: %d baud\n",ttname,speed);
    }
    return(0);

case XYTIMO:
    y = cmnum("Interpacket timeout interval","5",10,&x);
    y = setnum(&timint,x,y);
    if (y > -1) timef = 1;
    return(y);

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("Not working yet - %s\n",cmdbuf);
    return(0);
    }
}


/*  C H K S P D  --  Check if argument is a valid baud rate  */

chkspd(x) int x; {
    switch (x) {
	case 0:
	case 110:
	case 150:
	case 300:
	case 600:
	case 1200:
	case 1800:
	case 2400:
	case 4800:
	case 9600:
	    return(x);
	default: 
	    return(-1);
      }
}

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */

seton(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoff,2,"","on")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(0);
}

/*  S E T N U M  --  Set parameter to result of cmnum() parse.  */
/*
 Call with x - number from cnum parse, y - return code from cmnum
*/
setnum(prm,x,y) int x, y, *prm; {
    debug(F101,"setnum",y);
    if (y < 0) return(y);
    if (x > 94) {
	printf("\n?Sorry, 94 is the maximum\n");
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}

/*  S E T C C  --  Set parameter to an ASCII control character value.  */

setcc(prm,x,y) int x, y, *prm; {
    if (y < 0) return(y);
    if ((x > 037) && (x != 0177)) {
	printf("\n?Not in ASCII control range - %d\n",x);
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}


/*  D O H L P  --  Give a help message  */

dohlp(xx) int xx; {
    int x,y;

    char *tophlp = "\n\
Type ? for a list of commands, type 'help x' for any command x.\n\
While typing commands, use the following special characters:\n\n\
 DEL, RUBOUT, BACKSPACE, CTRL-H: Delete the most recent character typed.\n\
 CTRL-W: Delete the most recent word typed.\n\
 CTRL-U: Delete the current line.\n\
 CTRL-R: Redisplay the current line.\n\
 ?       (question mark) display help on the current command or field.\n\
 ESC     (Escape or Altmode) Attempt to complete the current field.\n\
 \\       (backslash) include the following character literally.\n\n\
From Unix command level, type 'kermit -h' to get help about command line args.\
\n";

    if (xx < 0) return(xx);
    switch (xx) {

case XXBYE:
    return(hmsg("Shut down and log out a remote Kermit server"));

case XXCLO:
    return(hmsg("Close one of the following logs:\n\
 session, transaction, packet, debugging -- 'help log' for further info."));

case XXCON:
    return(hmsg("Connect to a remote system via the tty device given in the\n\
most recent 'set line' command"));

case XXCWD:
    return(hmsg("Change Working Directory, equivalent to Unix 'cd' command"));

case XXDEL:
    return(hmsg("Delete a local file or files"));

case XXDIR:
    return(hmsg("Display a directory of local files"));

case XXECH:
    return(hmsg("Display the rest of the command on the terminal,\n\
useful in command files."));

case XXEXI:
case XXQUI:
    return(hmsg("Exit from the Kermit program, closing any open logs."));

case XXFIN:
    return(hmsg("\
Tell the remote Kermit server to shut down without logging out."));


case XXGET:
    return(hmsg("\
Format: 'get filespec'.  Tell the remote Kermit server to send the named\n\
files.  If filespec is omitted, then you are prompted for the remote and\n\
local filenames separately."));

case XXHLP:
    return(hmsg(tophlp));

case XXLOG:
    return(hmsg("Record information in a log file:\n\n\
 debugging             Debugging information, to help track down\n\
  (default debug.log)  bugs in the C-Kermit program.\n\n\
 packets               Kermit packets, to help track down protocol problems.\n\
  (packet.log)\n\n\
 session               Terminal session, during CONNECT command.\n\
  (session.log)\n\n\
 transactions          Names and statistics about files transferred.\n\
  (transaction.log)\n"));

case XXREC:
    return(hmsg("\
Format: 'receive [filespec]'.  Wait for a file to arrive from the other\n\
Kermit, which must be given a 'send' command.  If the optional filespec is\n\
given, the (first) incoming file will be stored under that name, otherwise\n\
it will be stored under the name it arrives with."));

case XXREM:
    if ((y = cmkey(remcmd,nrmt,"Remote command","")) == -2) return(y);
    if (y == -1) return(y);
    if (x = (cmcfm()) < 0) return(x);
    return(dohrmt(y));

case XXSEN:
    return(hmsg("\
Format: 'send file1 [file2]'.  File1 may contain wildcard characters '*' or\n\
'?'.  If no wildcards, then file2 may be used to specify the name file1 is\n\
sent under; if file2 is omitted, file1 is sent under its own name."));

case XXSER:
    return(hmsg("\
Enter server mode on the currently selected line.  All further commands\n\
will be taken in packet form from the other Kermit program."));

case XXSET:
    if ((y = cmkey(prmtab,nprm,"Parameter","")) == -2) return(y);
    if (y == -2) return(y);
    if (x = (cmcfm()) < 0) return(x);
    return(dohset(y));

case XXSHE:
    return(hmsg("\
Issue a command to the Unix shell (space required after '!')"));


case XXSHO:
    return(hmsg("\
Display current values of 'set' parameters; 'show version' will display\n\
program version information for each of the C-Kermit modules."));

case XXSPA:
    return(hmsg("Display disk usage in current device, directory"));

case XXSTA:
    return(hmsg("Display statistics about most recent file transfer"));

case XXTAK:
    return(hmsg("\
Take Kermit commands from the named file.  Kermit command files may\n\
themselves contain 'take' commands, up to a reasonable depth of nesting."));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("Not available yet - %s\n",cmdbuf);
    break;
    }
    return(0);
}



/*  H M S G  --  Get confirmation, then print the given message  */

hmsg(s) char *s; {
    int x;
    if (x = (cmcfm()) < 0) return(x);
    printf("%s\n",s);
    return(0);
}


/*  D O H S E T  --  Give help for SET command  */

dohset(xx) int xx; {
    
    if (xx == -3) {
	printf("\
The 'set' command is used to establish various communication or file\n\
parameters.  The 'show' command can be used to display the values of\n\
'set' parameters.  Help is available for each individual parameter;\n\
type 'help set ?' to see what's available.\n");
    	return(0);
    }
    if (xx < 0) return(xx);
    switch (xx) {

case XYCHKT:
    printf("\
Type of packet block check to be used for error detection, 1, 2, or 3.\n\
Type 1 is standard, and catches most errors.  Types 2 and 3 specify more\n\
rigorous checking, at the cost of higher overhead.  Not all Kermit programs\n\
support types 2 and 3.\n");
    return(0);

case XYDELA: 
    printf("\
Number of seconds to wait before sending first packet after 'send' command\n");
    return(0);

case XYDUPL:
    printf("\
During 'connect': 'full' means remote host echoes, 'half' means this program\n\
does its own echoing.\n");
    return(0);

case XYEOL:
    printf("\
Decimal ASCII value for character to terminate outbound packets, normally\n\
13 (CR, carriage return).  Inbound packets are assumed to end with CR.\n");
    return(0);

case XYESC:
    printf("\
Decimal ASCII value for escape character during 'connect', normally 28\n\
(Control-\\)\n");
    return(0);


case XYFILE:
    printf("\
set file: names, type, warning, display.\n\n\
'names' are normally 'converted', which means file names are converted\n\
to 'common form' during transmission; 'literal' means use filenames\n\
literally (useful between like Unix systems).\n");
    printf("\n\
'type' is normally 'text', in which conversion is done between Unix newlines\n\
and CRLF line delimiters; 'binary' means to do no conversion.  Use 'binary'\n\
for executable programs or binary data.\n");
    printf("\n\
'warning' is 'on' or 'off', normally off.  When off, incoming files will\n\
overwrite existing files of the same name.  When on, new names will be\n\
given to incoming files whose names are the same as existing files.\n");
    printf("\n\
'display' is normally 'on', causing file transfer progress to be displayed\n\
on your screen when in local mode.  'set display off' is useful for allowing\n\
file transfers to proceed in the background.\n\n");
    return(0);

case XYFLOW:
    printf("\
Type of flow control to be used.  Choices are 'xon/xoff' and 'none'.\n\
normally xon/xoff.\n");
    return(0);

case XYHAND:
    printf("\
Decimal ASCII value for character to use for half duplex line turnaround\n\
handshake.  Normally, handshaking is not done.\n");
    return(0);

case XYLEN:
    printf("\
Packet length to use.  90 by default.  94 maximum.\n");
    return(0);

case XYLINE:
    printf("Device name of communication line to use.  Normally %s.\n",dftty);
    if (!dfloc) {
	printf("If you set the line to other than %s, then Kermit\n",dftty);
	printf("\
will be in 'local' mode; 'set line' will reset Kermit to remote mode.\n");
    }
    return(0);

case XYMARK:
    printf("\
Decimal ASCII value of character that marks the beginning of packets sent by\n\
this program (normally 1 = Control-A)\n");
    return(0);


case XYNPAD:
    printf("\
Number of padding characters to request for inbound packets, normally 0.\n");
    return(0);

case XYPADC:
    printf("Decimal ASCII value of inbound padding character, normally 0.\n");
    return(0);

case XYPARI:
    printf("\
Parity to use during terminal connection and file transfer:\n\
even, odd, mark, space, or none.  Normally none.\n");
    return(0);

case XYPROM:
    printf("Prompt string for this program, normally 'C-Kermit>'.\n");
    return(0);

case XYSPEE:
    printf("\
Communication line speed for external tty line specified in most recent\n\
'set line'.  Any of the common baud rates: 0, 110, 150, 300, 600, 1200,\n\
1800, 2400, 4800, 9600.\n");
    return(0);

case XYTIMO:
    printf("\
Timeout interval for this program to use during file transfer, seconds.\n");
    return(0);

default:
    printf("Not available yet - %s\n",cmdbuf);
    return(0);
    }
}


/*  D O R M T  --  Do a remote command  */

dormt(xx) int xx; {
    int x;
    char *s, sbuf[50], *s2;

    if (xx < 0) return(xx);
    switch (xx) {

case XZCWD:				/* CWD */
    if ((x = cmtxt("Remote directory name","",&s)) < 0) return(x);
    debug(F111,"XZCWD: ",s,x);
    *sbuf = NUL;
    s2 = sbuf;
    if (*s != NUL) {			/* If directory name given, */
					/* get password on separate line. */
        if (tlevel > -1) {		/* From take file... */

	    *line = NUL;
	    if (fgets(sbuf,50,tfile[tlevel]) == NULL)
	    	ermsg("take file ends prematurely in 'remote cwd'");
	    debug(F110," pswd from take file",s2,0);

        } else {			/* From terminal... */

	    printf(" Password: "); 		/* get a password */
	    while ((x = getchar()) != '\n') {   /* without echo. */
	    	if ((x &= 0177) == '?') {
	    	    printf("? Password of remote directory\n Password: ");
		    s2 = sbuf;
		    *sbuf = NUL;
	    	}
	    	else if (x == ESC)	/* Mini command line editor... */
	    	    putchar(BEL);
		else if (x == BS || x == 0177)
		    *s2--;
		else if (x == 025) {
		    s2 = sbuf;
		    *sbuf = NUL;
		}
	    	else
		    *s2++ = x;
            }
	    *s2 = NUL;
	    putchar('\n');
        }
        s2 = sbuf;
    } else s2 = "";
    debug(F110," password",s2,0);
    sstate = setgen('C',s,s2,"");
    return(0);


case XZDEL:				/* Delete */
    if ((x = cmtxt("Name of remote file(s) to delete","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'E'));

case XZDIR:				/* Directory */
    if ((x = cmtxt("Remote directory or file specification","",&s)) < 0)
    	return(x);
    return(sstate = rfilop(s,'D'));

case XZHLP:				/* Help */
    if (x = (cmcfm()) < 0) return(x);
    sstate = setgen('H',"","","");
    return(0);

case XZHOS:				/* Host */
    if ((x = cmtxt("Command for remote system","",&cmarg)) < 0) return(x);
    return(sstate = 'c');

case XZPRI:				/* Print */
    if ((x = cmtxt("Remote file(s) to print on remote printer","",&s)) < 0)
    	return(x);
    return(sstate = rfilop(s,'S'));

case XZSPA:				/* Space */
    if ((x = cmtxt("Confirm, or remote directory name","",&s)) < 0) return(x);
    return(sstate = setgen('U',s,"",""));
    
case XZTYP:				/* Type */
    if ((x = cmtxt("Remote file specification","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'T'));

case XZWHO:
    if ((x = cmtxt("Remote user name, or carriage return","",&s)) < 0)
    	return(x);
    return(sstate = setgen('W',s,"",""));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}



/*  R F I L O P  --  Remote File Operation  */

rfilop(s,t) char *s, t; {
    if (*s == NUL) {
	printf("?File specification required\n");
	return(-2);
    }
    debug(F111,"rfilop",s,t);
    return(setgen(t,s,"",""));
}


/*  D O H R M T  --  Give help about REMOTE command  */

dohrmt(xx) int xx; {
    int x;
    if (xx == -3) {
	printf("\
The 'remote' command is used to send file management instructions to a\n\
remote Kermit server.  There should already be a Kermit running in server\n\
mode on the other end of the currently selected line.  Type 'remote ?' to\n\
see a list of available remote commands.  Type 'help remote x' to get\n\
further information about a particular remote command 'x'.\n");
    	return(0);
    }
    if (xx < 0) return(xx);
    switch (xx) {

case XZCWD:
    return(hmsg("\
Ask remote Kermit server to change its working directory."));

case XZDEL:
    return(hmsg("\
Ask remote Kermit server to delete the named file(s)."));

case XZDIR:
    return(hmsg("\
Ask remote Kermit server to provide directory listing of the named file(s)."));

case XZHLP:
    return(hmsg("\
Ask remote Kermit server to tell you what services it provides."));

case XZHOS:
    return(hmsg("\
Send a command to the remote system in its own command language\n\
through the remote Kermit server."));

case XZSPA:
    return(hmsg("\
Ask the remote Kermit server to tell you about its disk space."));

case XZTYP:
    return(hmsg("\
Ask the remote Kermit server to type the named file(s) on your screen."));

case XZWHO:
    return(hmsg("\
Ask the remote Kermit server to list who's logged in, or to give information\n\
about the specified user."));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}


/*  S C R E E N  --  Screen display function  */
/*  
   c - a character or small integer
   n - an integet
   s - a string.
 Fill in this routine with the appropriate display update for the system.
 This version is for a dumb tty, and uses the arguments like this:
   c:     0 - print s on a new line, followed by a space.
          1 - print s at current screen position.
          2 - print s at current position, followed by newline.
	  3 - print "s: n" at current position, followed by newline.
      other - print c as a character.
   n: if c is 'other', used to decide whether or how to print c.
   s: as above.
 Horizontal screen position is kept current if screen is only updated by 
 calling this function.  Wraparound is done at column 78.
*/
screen(c,n,s) char c; int n; char *s; {
    static int p = 0;			/* Screen position */
    int len;  char buf[80];
    len = strlen(s);
    if (!displa || quiet) return;	/* No update if display flag off */
    switch (c) {
	case 0:				/* Print s on newline */
	    conoll("");			/* Start new line, */
	    conol(s);			/* Print string. */
	    conoc(SP);			/* Leave a space. */
	    p = len + 1;		/* Set position counter. */
	    return;
	case 1:
	    if (p + len > 78) { conoll(""); p = 0; }
	    conol(s);  if ((p += len) > 78) conoll("");
	    return;
	case 2:				/* Print s, then newline */
	    if (p + len > 78) conoll("");
	    conoll(s);  p = 0;
	    return;
	case 3:
	    sprintf(buf,"%s: %d",s,n);  conoll(buf);  p = 0;
	    return;
	case BS:			/* Backspace */
	    if (p > 0) p--;
	case BEL:			/* Beep */
	    conoc(c);
	    return;
	default:			/* Packet type display */
	    if (c == 'Y') return;	/* Don't bother with ACKs */
	    if (c == 'D') {		/* Only show every 4th data packet */
		c = '.';
		if (n % 4) return;
	    }
	    if (p++ > 78) {		/* If near left margin, */
		conoll("");		/* Start new line */
		p = 0;			/* and reset counter. */
	    }
	    conoc(c);			/* Display the character. */
	    return;
    }
}


/*  I N T M S G  --  Issue message about terminal interrupts  */

intmsg(n) int n; {
    if ((!displa) || (quiet)) return;
    if (n == 1) {
	screen(2,0,"");
	screen(2,0,"CTRL-F to cancel file,  CTRL-R to resend current packet");
	screen(2,0,"CTRL-B to cancel batch, CTRL-A for status report...");
    }
    else screen(1,0," ");
}


/*  C H K I N T  --  Check for console interrupts  */

/*** should rework not to destroy typeahead ***/

chkint() {
    int ch, cn;

    if ((!local) || (quiet)) return(0);	/* Only do this if local & not quiet */
    cn = conchk();			/* Any input waiting? */
    debug(F101,"conchk","",cn);

    while (cn > 0) {			/* Yes, read it. */
	cn--;
	if ((ch = coninc()) < 0) return(0);
	switch (ch & 0177) {
	    case 0001:			/* CTRL-A */
		screen(2,0,"^A  Status report:");
		screen(1,0,     " file type: ");
		if (binary) screen(2,0,"binary"); else screen(2,0,"text");
		screen(3,filcnt," file number");
		screen(3,ffc,   " characters ");
		screen(3,bctu,  " block check");
		screen(3,rptflg," compression");
		screen(3,ebqflg," 8th-bit prefixing");
		continue;
	    case 0002:			/* CTRL-B */
	    	screen(1,0,"^B - Cancelling Batch ");
	    	czseen = 1;
		continue;
	    case 0006:			/* CTRL-F */
	    	screen(1,0,"^F - Cancelling File ");
	    	cxseen = 1;
		continue;
	    case 0022:	    	    	/* CTRL-R */
	    	screen(1,0,"^R - Resending ");
	    	resend();
		return(1);
	    default:			/* Anything else, just beep */
	    	screen(BEL,0,"");
		continue;
    	}
    }
    return(0);
}


/*  D E B U G  --  Enter a record in the debugging log  */

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
        If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.
*/
debug(f,s1,s2,n) int f, n; char *s1, *s2; {
    static char s[200];
    char *sp = s;

    if (!deblog) return;		/* If no debug log, don't */
    switch (f) {
    	case F000:			/* 0, print both strings, */
	    sprintf(sp,"%s%s%c\n",s1,s2,n); /*  and interpret n as a char */
	    zsout(ZDFILE,s);
	    break;
    	case F001:			/* 1, "=n" */
	    sprintf(sp,"=%d\n",n);
	    zsout(ZDFILE,s);
	    break;
    	case F010:			/* 2, "[s2]" */
	    sprintf(sp,"[%s]\n",s2);
	    zsout(ZDFILE,"");
	    break;
    	case F011:			/* 3, "[s2]=n" */
	    sprintf(sp,"[%s]=%d\n",s2,n);
	    zsout(ZDFILE,s);
	    break;
    	case F100:			/* 4, "s1" */
	    zsoutl(ZDFILE,s1);
	    break;
    	case F101:			/* 5, "s1=n" */
	    sprintf(sp,"%s=%d\n",s1,n);
	    zsout(ZDFILE,s);
	    break;
    	case F110:			/* 6, "s1[s2]" */
	    sprintf(sp,"%s[%s]\n",s1,s2);
	    zsout(ZDFILE,s);
	    break;
    	case F111:			/* 7, "s1[s2]=n" */
	    sprintf(sp,"%s[%s]=%d\n",s1,s2,n);
	    zsout(ZDFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for debug() - %d\n",n);
	    zsout(ZDFILE,s);
    }
}


/*  T L O G  --  Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1 - String, argument number 1.
   s2 - String, argument number 2.
   n  - Int, argument 3.
*/
tlog(f,s1,s2,n) int f, n; char *s1, *s2; {
    static char s[200];
    char *sp = s; int x;
    
    if (!tralog) return;		/* If no transaction log, don't */
    switch (f) {
    	case F000:			/* 0 (special) "s1 n s2"  */
	    sprintf(sp,"%s %d %s\n",s1,n,s2);
	    zsout(ZTFILE,s);
	    break;
    	case F001:			/* 1, " n" */
	    sprintf(sp," %d\n",n);
	    zsout(ZTFILE,s);
	    break;
    	case F010:			/* 2, "[s2]" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"[%s]\n",s2);
	    zsout(ZTFILE,"");
	    break;
    	case F011:			/* 3, "[s2] n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"[%s] %d\n",s2,n);
	    zsout(ZTFILE,s);
	    break;
    	case F100:			/* 4, "s1" */
	    zsoutl(ZTFILE,s1);
	    break;
    	case F101:			/* 5, "s1: n" */
	    sprintf(sp,"%s: %d\n",s1,n);
	    zsout(ZTFILE,s);
	    break;
    	case F110:			/* 6, "s1 s2" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"%s %s\n",s1,s2);
	    zsout(ZTFILE,s);
	    break;
    	case F111:			/* 7, "s1 s2: n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"%s %s: %d\n",s1,s2,n);
	    zsout(ZTFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for tlog() - %d\n",n);
	    zsout(ZTFILE,s);
    }
}
