#include "ckcsym.h"
#ifndef NOCMDL

/*  C K U U S Y --  "User Interface" for Unix Kermit, part Y  */

/*  Command-Line Argument Parser */
 
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
*/

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"

#ifdef NETCONN
#ifdef ANYX25
extern int revcall, closgr, cudata;
extern char udata[MAXCUDATA];
extern int x25fd;
#endif /* ANYX25 */
#ifndef VMS
#ifndef OS2
extern
#endif /* OS2 */
#endif /* VMS */
 int telnetfd;
#endif /* NETCONN */

extern char *ckxsys, *ckzsys, *cmarg, *cmarg2, **xargv, **cmlist, *clcmds;
extern int action, cflg, xargc, stdouf, stdinf, displa, cnflg, nfils,
  local, quiet, escape, network, mdmtyp, maxrps, rpsiz, bgset, xargs,
  urpsiz, wslotr, swcapr, binary, warn, parity, turn, turnch, duplex, flow,
  fncact, clfils, noinit, stayflg, nettype, cfilef, delay, noherald;
extern long speed, ttgspd(), zchki();
extern char ttname[];
#ifdef OS2PM
extern int os2pm;
#endif /* OS2PM */

#ifdef CK_NETBIOS
extern unsigned char NetBiosAdapter ;
#endif /* CK_NETBIOS */

#ifndef NODIAL
extern int nmdm;
extern struct keytab mdmtab[];
#endif /* NODIAL */

/*  C M D L I N  --  Get arguments from command line  */
/*
 Simple Unix-style command line parser, conforming with 'A Proposed Command
 Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World, Vol.1,
 No.3, 1984.
*/
int
cmdlin() {
    char x;				/* Local general-purpose int */
    cmarg = "";				/* Initialize globals */
    cmarg2 = "";
    action = cflg = 0;
 
    debug(F111,"cmdlin",*xargv,xargc);

/* If we were started directly from a Kermit application file, its name is */
/* in argv[1], so skip past it. */

    if (xargc > 1) {
	if (*xargv[0] != '-' && *xargv[1] != '-') {
	    if (
		/* some shells don't put full pathname... */
		/* zchki(xargv[0]) > 0 && */ /* ...so skip this test */
		cfilef ||		/* If it's our command file */
		zchki(xargv[1]) > 0	/* or if it's an existing file */
		) {			
		xargc -= 1;		/* skip past it */
		xargv += 1;		/* ... */
	    }
	}
    }

    while (--xargc > 0) {		/* Go through command line words */
	xargv++;
	debug(F111,"xargv",*xargv,xargc);
	if (**xargv == '=') return(0);
#ifdef VMS
	else if (**xargv == '/') continue;
#endif /* VMS */
    	else if (**xargv == '-') {	/* Got an option (begins with dash) */
	    x = *(*xargv+1);		/* Get the option letter */
	    if (doarg(x) < 0) doexit(BAD_EXIT,1); /* Go handle option */
    	} else {			/* No dash where expected */
	    fatal2(*xargv,
		   "invalid command-line option, type \"kermit -h\" for help");
	}
    }
    debug(F101,"action","",action);
#ifndef NOLOCAL
    if (!local) {
	if ((action == 'c') || (cflg != 0))
	    fatal("-l or -j or -X required");
    }
#endif /* NOLOCAL */
    if (*cmarg2 != 0) {
	if ((action != 's') && (action != 'r') &&
	    (action != 'v'))
	    fatal("-a without -s, -r, or -g");
	if (action == 'r' || action == 'v') {
	    if (zchko(cmarg2) < 0)
	      fatal("write access to -a file denied");
#ifdef CK_TMPDIR
	    if (isdir(cmarg2))		/* -a is a directory */
	      if (!zchdir(cmarg2))	/* try to change to it */
		fatal("can't change to -a directory");
	      else cmarg2 = "";
#endif /* CK_TMPDIR */
	}
    }
    if ((action == 'v') && (stdouf) && (!local)) {
    	if (isatty(1))
	    fatal("unredirected -k can only be used in local mode");
    }
    if ((action == 's') || (action == 'v') ||
    	(action == 'r') || (action == 'x')) {
	if (local) displa = 1;
	if (stdouf) { displa = 0; quiet = 1; }
    }
    if (quiet) displa = 0;		/* No display if quiet requested */
    return(action);			/* Then do any requested protocol */
}

/*  D O A R G  --  Do a command-line argument.  */
 
int
#ifdef CK_ANSIC
doarg(char x)
#else
doarg(x) char x; 
#endif /* CK_ANSIC */
/* doarg */ {
    int i, y, z; long zz; char *xp;
 
    xp = *xargv+1;			/* Pointer for bundled args */
    debug(F111,"doarg entry",xp,xargc);
    while (x) {
	debug(F000,"doarg arg","",x);
	switch (x) {

#ifndef NOSPL
case 'C':				/* Commands for parser */
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
      fatal("No commands given for -C");
    clcmds = *xargv;			/* Get the argument (must be quoted) */
    break;
#endif /* NOSPL */

case 'D':				/* Delay */
    if (*(xp+1)) fatal("invalid argument bundling");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
      fatal("missing delay value");
    z = atoi(*xargv);			/* Convert to number */
    if (z > -1)				/* If in range */
      delay = z;			/* set it */
    else
      fatal("bad delay value");
    break;


#ifndef NOICP
case 'S':				/* "Stay" - enter interactive */
    stayflg = 1;			/* command parser after executing */
    break;				/* command-line actions. */
#endif /* NOICP */

case 'R':				/* Remote-Only */
    break;				/* This is handled in prescan(). */

#ifndef NOSERVER
case 'x':				/* server */
    if (action) fatal("conflicting actions");
    action = 'x';
    break;
#endif /* NOSERVER */
 
case 'f':				/* finish */
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
    nfils = z = 0;			/* Initialize file counter, flag */
    cmlist = xargv+1;			/* Remember this pointer */
    while (--xargc > 0) {		/* Traverse the list */	
	xargv++;
	if (**xargv == '-') {		/* Check for sending stdin */
	    if (strcmp(*xargv,"-") != 0) /* Watch out for next option. */
	      break;
	    z++;			/* "-" alone means send from stdin. */
        } else if (zchki(*xargv) > -1	/* Check if file exists */
#ifndef UNIX
		   /* or contains wildcard characters matching real files */
		   || (iswild(*xargv) && zxpand(*xargv) > 0)
#endif /* UNIX */
		   ) {
	    nfils++;			/* Bump file counter */
	}
    }
    xargc++, xargv--;			/* Adjust argv/argc */
    if (nfils < 1 && z == 0)
#ifdef VMS
      fatal("%CKERMIT-E-SEARCHFAIL, no files for -s");
#else
      fatal("No files for -s");
#endif /* VMS */
    if (z > 1) fatal("-s: too many -'s");
    if (z == 1 && nfils > 0)
      fatal("invalid mixture of filenames and '-' in -s");
    if (nfils == 0) {
	if (isatty(0)) fatal("sending from terminal not allowed");
	else stdinf = 1;
    }

#ifdef COMMENT
    /* If only one filespec was given, indicate "internal list" rather than */
    /* "expanded list", so in case it includes wildcards, C-Kermit can */
    /* expand them itself. */
    if (nfils == 1) {
	cmarg = *cmlist;
	nfils = -1;
    }
#endif /* COMMENT */

    debug(F101,*xargv,"",nfils);
    action = 's';
#ifdef UNIX
/* When set, this flag tells Kermit not to expand wildcard characters. */
/* In UNIX, the shell has already expanded them.  In VMS, OS/2, etc, */
/* Kermit must expand them.  Kermit must not expand them in UNIX because */
/* a filename might itself contain metacharacters.  Imagine, for example, */
/* what would happen if a directory contained a file named "*". */
    clfils = 1;				/* Flag for command-line files */
#endif /* UNIX */
    break;
 
case 'g':				/* get */
    if (action) fatal("conflicting actions");
    if (*(xp+1)) fatal("invalid argument bundling after -g");
    xargv++, xargc--;
    if ((xargc == 0) || (**xargv == '-'))
    	fatal("missing filename for -g");
    cmarg = *xargv;
    action = 'r';
    break;
 
#ifndef NOLOCAL
case 'c':				/* connect before */
    cflg = 1;
    break;
 
case 'n':				/* connect after */
    cnflg = 1;
    break;
#endif /* NOLOCAL */
 
case 'h':				/* help */
    usage();
#ifndef NOICP
    if (stayflg)	
      break;
    else
#endif /* NOICP */
      doexit(GOOD_EXIT,-1);

case 'a':				/* "as" */
    if (*(xp+1)) fatal("invalid argument bundling after -a");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing name in -a");
    cmarg2 = *xargv;
    break;
 
#ifndef NOICP
case 'Y':				/* No initialization file */
    noinit = 1;
    break;

case 'y':				/* Alternate init-file name */
    if (*(xp+1)) fatal("invalid argument bundling after -y");
    xargv++, xargc--;
    if (xargc < 1) fatal("missing filename in -y");
    /* strcpy(kermrc,*xargv); ...this was already done in prescan()... */
    break;
#endif /* NOICP */

#ifndef NOLOCAL
case 'l':				/* SET LINE */
#ifdef NETCONN
#ifdef ANYX25
case 'X':				/* SET HOST to X.25 address */
#ifdef SUNX25
case 'Z':				/* SET HOST to X.25 file descriptor */
#endif /* SUNX25 */
#endif /* ANYX25 */
#ifdef TCPSOCKET
case 'j':				/* SET HOST (TCP/IP socket) */
#endif /* TCPSOCKET */
#endif /* NETCONN */
    network = 0;
    if (*(xp+1)) fatal("invalid argument bundling after -l or -j");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("communication line device name missing");
    strcpy(ttname,*xargv);
    local = (strcmp(ttname,CTTNAM) != 0);
/*
  NOTE: We really do not need to call ttopen here, since it should be called
  again later, automatically, when we first try to condition the device via
  ttpkt or ttvt.  Calling ttopen here has the bad side effect of making the
  order of the -b and -l options significant, but order of command-line options
  should not matter.  However, the network cases immediately below complicate
  matters a bit, so we'll settle this in a future edit.
*/
    if (x == 'l') {
	if (ttopen(ttname,&local,mdmtyp,0) < 0)
	  fatal("can't open device");
	debug(F101,"cmdlin speed","",speed);
#ifdef COMMENT
/* What can it hurt? */
	if (speed < 0L)			/* If speed hasn't been set yet, */
#endif /* COMMENT */
	  speed = ttgspd();		/* get it. */
#ifdef NETCONN
    } else {
	if (x == 'j') {			/* IP network host name */
	    nettype = NET_TCPB;
	    mdmtyp = -nettype;          /* perhaps already set in init file */
	    telnetfd = 1;		/* Or maybe an open file descriptor */
#ifdef SUNX25
	} else if (x == 'X') {		/* X.25 address */
	    nettype = NET_SX25;
	    mdmtyp = -nettype;
	} else if (x == 'Z') {		/* Open X.25 file descriptor */
	    nettype = NET_SX25;
	    mdmtyp = -nettype;
	    x25fd = 1;
#endif /* SUNX25 */
#ifdef STRATUSX25
	} else if (x == 'X') {		/* X.25 address */
	    nettype = NET_VX25;
	    mdmtyp = -nettype;
#endif /* STRATUSX25 */
	}
	if (ttopen(ttname,&local,mdmtyp,0) < 0)
	  fatal("can't open host connection");
	network = 1;
#endif /* NETCONN */
    }
    /* add more here later - decnet, etc... */
    break;
 
#ifdef ANYX25
case 'U':                               /* X.25 call user data */
    if (*(xp+1)) fatal("invalid argument bundling");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
        fatal("missing call user data string");
    strcpy(udata,*xargv);
    if ((int)strlen(udata) <= MAXCUDATA) cudata = 1;
    else fatal("Invalid call user data");
    break;

case 'o':                               /* X.25 closed user group */
    if (*(xp+1)) fatal("invalid argument bundling");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing closed user group index");
    z = atoi(*xargv);			/* Convert to number */
    if (z >= 0 && z <= 99) closgr = z;
    else fatal("Invalid closed user group index");
    break;

case 'u':                               /* X.25 reverse charge call */
    revcall = 1;
    break;
#endif /* ANYX25 */
#endif /* NOLOCAL */

case 'b':   	    			/* set bits-per-second for serial */
    if (*(xp+1)) fatal("invalid argument bundling"); /* communication device */
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing bps");
    zz = atol(*xargv);			/* Convert to long int */
    i = zz / 10L;
#ifndef NOLOCAL
    if (ttsspd(i) > -1)			/* Check and set it */
#endif /* NOLOCAL */
      speed = ttgspd();			/* and read it back. */
#ifndef NOLOCAL
    else
      fatal("unsupported transmission rate");
#endif /* NOLOCAL */
    break;
 
#ifndef NODIAL
case 'm':				/* Modem type */
    if (*(xp+1)) fatal("invalid argument bundling after -m");    
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("modem type missing");
    y = lookup(mdmtab,*xargv,nmdm,&z);
    if (y < 0)
      fatal("unknown modem type");
    mdmtyp = y;
    break;
#endif /* NODIAL */

case 'e':				/* Extended packet length */
    if (*(xp+1)) fatal("invalid argument bundling after -e");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing length");
    z = atoi(*xargv);			/* Convert to number */
    if (z > 10 && z <= maxrps) {
        rpsiz = urpsiz = z;
	if (z > 94) rpsiz = 94;		/* Fallback if other Kermit can't */
    } else fatal("Unsupported packet length");
    break;

case 'v':				/* Vindow size */
    if (*(xp+1)) fatal("invalid argument bundling");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
    	fatal("missing or bad window size");
    z = atoi(*xargv);			/* Convert to number */
    if (z < 32) {			/* If in range */
	wslotr = z;			/* set it */
	if (z > 1) swcapr = 1;		/* Set capas bit if windowing */
    } else fatal("Unsupported packet length");
    break;

case 'i':				/* Treat files as binary */
    binary = XYFT_B;
    break;
 
case 'w':				/* Writeover */
    warn = 0;
    fncact = XYFX_X;
    break;
 
case 'q':				/* Quiet */
    quiet = 1;
    break;
 
#ifdef DEBUG
case 'd':				/* DEBUG */
    break;				/* Handled in prescan() */
#endif /* DEBUG */ 

case 'p':				/* SET PARITY */
    if (*(xp+1)) fatal("invalid argument bundling");
    xargv++, xargc--;
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
 
case 'z':				/* Not background */
    bgset = 0;
    break;

#ifdef CK_NETBIOS
case 'N':               /* NetBios Adapter Number follows */
    if (*(xp+1)) fatal("invalid argument bundling after -N");
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-'))
      fatal("missing NetBios Adapter number");
    if ( (strlen(*xargv) != 1) ||
	(atoi(*xargv) < 0) ||
	(atoi(*xargv) > 3) )
      fatal("invalid NetBios Adapter specified\nadapters 0 to 3 are valid");
    break;
#endif /* CK_NETBIOS */

#ifdef OS2PM
case 'P':				/* OS/2 Presentation Manager */
    os2pm = 1;
    break;
#endif /* OS2PM */

#ifndef NOICP
  case 'H':
    noherald = 1;
    break;
#endif /* NOICP */

default:
    fatal2(*xargv,
	   "invalid command-line option, type \"kermit -h\" for help");
        }
 
    x = *++xp;				/* See if options are bundled */
    }
    return(0);
}
#else /* No command-line interface... */

extern int xargc;
int
cmdlin() {
    if (xargc > 1) fatal("Sorry, command-line options disabled.");
}
#endif /* NOCMDL */
