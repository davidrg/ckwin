char *userv = "User Interface V4.2(038), 5 Mar 85";

/*  C K U S E R --  "User Interface" for Unix Kermit (Part 1)  */

/*  Frank da Cruz, Columbia University Center for Computing Activities, 1985 */
/*
 The ckuser module contains the terminal input and output functions for Unix
 Kermit.  It includes a simple Unix-style command line parser as well as
 an interactive prompting keyword command parser.  It depends on the existence
 of Unix facilities like fopen, fgets, feof, (f)printf, argv/argc, etc.  Other
 functions that are likely to vary among Unix implementations -- like setting
 terminal modes or interrupts -- are invoked via calls to functions that are
 defined in the system-dependent modules, ck[xz]*.c.

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
#include "ckuser.h"

/* External Kermit Variables, see ckmain.c for description. */

extern int size, spsiz, npad, timint, speed, local, server, image, flow,
  displa, binary, fncnv, delay, parity, deblog, escape, xargc,
  turn, duplex, cxseen, czseen, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  turnch, chklen, bctr, bctu, fsize, dfloc, mdmtyp,
  rptflg, ebqflg, warn, quiet, cnflg, timef, mypadn;

extern long filcnt, tlci, tlco, ffc, tfc;

extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *ckxsys, *ckzsys, *cmarg, *cmarg2, **xargv, **cmlist;
extern char mystch, sstate, mypadc, padch, eol, ctlq, filnam[], ttname[];
char *strcpy();

/* Declarations from cmd package */

extern char cmdbuf[];			/* Command buffer */

/* Declarations from ckz??? module */

extern char *SPACMD, *zhome();		/* Space command, home directory. */
extern int backgrd;			/* Kermit executing in background */

/* The background flag is set by ckxunx.c (via conint() ) to note whether */
/* this kermit is executing in background ('&' on shell command line).    */


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
    repars,				/* Reparse needed */
    tlevel,				/* Take command level */
    cwdf = 0;				/* CWD has been done */

#define MAXTAKE 20			/* Maximum nesting of TAKE files */
FILE *tfile[MAXTAKE];			/* File pointers for TAKE command */

char *homdir;				/* Pointer to home directory string */
char cmdstr[100];

/*  C M D L I N  --  Get arguments from command line  */
/*
 Simple Unix-style command line parser, conforming with 'A Proposed Command
 Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World, Vol.1,
 No.3, 1984.
*/
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
	    doexit(0);			/*  exit with status = 0 */
	}
    }
    if (displa) concb(escape);		/* (for console "interrupts") */
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
    tlog(F110,"Fatal:",msg,0l);
    doexit(1);				/*  exit with status = 1 */
}


ermsg(msg) char *msg; {			/* Print error message */
    if (!quiet) fprintf(stderr,"\r\nError - %s\n",msg);
    tlog(F110,"Error -",msg,0l);
}

/* Interactive command parser */ 


/* Top-Level Keyword Table */

struct keytab cmdtab[] = {
    "!",	   XXSHE, 0,
    "bye",         XXBYE, 0,
    "c",           XXCON, CM_INV,
    "close",	   XXCLO, 0,
    "connect",     XXCON, 0,
    "cwd",	   XXCWD, 0,
    "dial",	   XXDIAL, 0,
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
    "script",	   XXLOGI, 0,
    "send",	   XXSEN, 0,
    "server",	   XXSER, 0,
    "set",	   XXSET, 0,
    "show", 	   XXSHO, 0,
    "space",       XXSPA, 0,
    "statistics",  XXSTA, 0,
    "take",	   XXTAK, 0
};
int ncmd = (sizeof(cmdtab) / sizeof(struct keytab));

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
    "modem-dialer",	XYMODM,	 0,
    "packet-length",    XYLEN,   0,
    "pad-character",    XYPADC,  0,
    "padding",          XYNPAD,  0,
    "parity",	    	XYPARI,  0,
    "prompt",	    	XYPROM,  0,
    "speed",	        XYSPEE,  0,
    "start-of-packet",  XYMARK,  0,
    "timeout",	        XYTIMO,  0
};
int nprm = (sizeof(prmtab) / sizeof(struct keytab)); /* How many parameters */


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
int nrmt = (sizeof(remcmd) / sizeof(struct keytab));

struct keytab logtab[] = {
    "debugging",    LOGD, 0,
    "packets",	    LOGP, 0,
    "session",      LOGS, 0,
    "transactions", LOGT, 0
};
int nlog = (sizeof(logtab) / sizeof(struct keytab));

/* Show command arguments */

#define SHPAR 0				/* Parameters */
#define SHVER 1				/* Versions */

struct keytab shotab[] = {
    "parameters", SHPAR, 0,
    "versions",   SHVER, 0
};

/*  C M D I N I  --  Initialize the interactive command parser  */

cmdini() {

    printf("%s,%s\nType ? for help\n",versio,ckxsys);
    cmsetp("C-Kermit>");		/* Set default prompt. */

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

    congm();				/* Get console tty modes */
}


/*  T R A P  --  Terminal interrupt handler */

trap() {
    debug(F100,"terminal interrupt...","",0);
    doexit(0);				/* exit with status = 0 */
}

/*  P A R S E R  --  Top-level interactive command parser.  */

parser() {
    int xx;
    concb(escape);		/* Put console in cbreak mode. */
    conint(trap);		/* Turn on console terminal interrupts. */
/*
 sstate becomes nonzero when a command has been parsed that requires some
 action from the protocol module.  Any non-protocol actions, such as local
 directory listing or terminal emulation, are invoked directly from below.
*/
    if (local) printf("\n");		/*** Temporary kludge ***/
    sstate = 0;				/* Start with no start state. */
    while (sstate == 0) {		/* Parse cmds until action requested */
	while ((tlevel > -1) && feof(tfile[tlevel])) { /* If end of take */
		fclose(tfile[tlevel]);	/* file, close it */
		tlevel--;		/* and forget about it. */
		cmini(ckxech);		/* and clear the cmd buffer. */
 	}
	if (tlevel > -1) {
	    if (fgets(cmdbuf,CMDBL,tfile[tlevel]) == NULL) continue;
	    stripq(cmdbuf);		/* Strip any quotes. */
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
		    doexit(0);		/*  exit with status 0 */
	        case -1:		/* Reparse needed */
		    repars = 1;
		    continue;
	    	case -2:		/* Invalid command given */
		    if (backgrd) 	/* if in background, terminate */
			fatal("Kermit command error in background execution");
		    if (tlevel > -1) {	/* If in take file, quit */
			ermsg("Kermit command error: take file terminated.");
			fclose(tfile[tlevel]);
			tlevel--;
		    }
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

doexit(exitstat) int exitstat; {
    
    ttclos();				/* Close external line, if any */
    if (local) {
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
	tlog(F100,"Transaction Log Closed","",0l);
	*trafil = '\0';
	tralog = 0;
	zclose(ZTFILE);
    }
    exit(exitstat);				/* Exit from the program. */
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
    doexit(0);
case -3:				/* Null command */
    return(0);
case -2:				/* Error */
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
    x = conect();
    concb(escape);			/* tty back in character mode */
    return(x);

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
	    printf("\n?Unexpected log designator - %ld\n", x);
	    return(0);
    }

case XXDIAL:				/* dial number */
    if ((x = cmtxt("Number to be dialed","",&s)) < 0) return(x);
    return( dial(s) );			/* return success 0=connected -2=fail*/

case XXDIR:				/* directory */
    if ((x = cmtxt("Directory/file specification","*",&s)) < 0) return(x);
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
    if ((x = cmcfm()) > -1) doexit(0);	/*  exit with status 0 */
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
	    stripq(line);
	    cmarg = line;
	    if (fgets(cmdbuf,CMDBL,tfile[tlevel]) == NULL)
	    	fatal("take file ends prematurely in 'get'");
		stripq(cmdbuf);
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
		if (x == -2) return(x);
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

case XXLOGI:				/* login to remote system */
    if ((x = cmtxt("Text of login script","",&s)) < 0) return(x);
    return( login(s) );			/* return success 0=completed -2=fail*/

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
    concb(escape);			/* Console back in cbreak mode */
    return(0);

case XXSHO:				/* Show */
    x = cmkey(shotab,2,"","parameters");
    if (x < 0) return(x);
    if (y = (cmcfm()) < 0) return(y);
    switch (x) {

	case SHPAR:
	    shopar();
	    break;

	case SHVER:
	    printf("\nVersions:\n %s\n %s\n %s\n",versio,protv,fnsv);
	    printf(" %s\n %s\n %s for%s\n",cmdv,userv,ckxv,ckxsys);
            printf(" %s for%s\n %s\n\n",ckzv,ckzsys,connv);
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
    printf(" files: %ld\n",filcnt);
    printf(" total file characters  : %ld\n",tfc);
    printf(" communication line in  : %ld\n",tlci);
    printf(" communication line out : %ld\n\n",tlco);
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

/*  S H O P A R  --  Show Parameters  */

shopar() {

    printf("\nLine: %s, speed: %d, mode: ",ttname,speed);
    if (local) printf("local"); else printf("remote");
    printf(", modem-dialer: ");
    if (mdmtyp == 1) printf("hayes");
	else if (mdmtyp == 2) printf("ventel");
	else printf("direct");
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
		tlog(F110,"Transaction Log:",versio,0l);
		tlog(F100,ckxsys,"",0);
		ztime(&s);
		tlog(F100,s,"",0l);
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
	debug(F100,ckxsys,"",0);
	ztime(&tp);
	debug(F100,tp,"",0);
    } else *debfil = '\0';
    return(deblog);
}

