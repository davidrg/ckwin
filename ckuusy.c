#include "ckcsym.h"
#define XFATAL fatal

#ifndef NOCMDL

/*  C K U U S Y --  "User Interface" for Unix Kermit, part Y  */

/*  Command-Line Argument Parser */
 
/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of
  New York.  All rights reserved.
*/

#ifdef COMMENT
  Census of command-line options ( "grep ^case ckuusy.c | sort" :-)
  Update this if you add a new one:

case '8':				/* 8-bit clean */
case 'B':				/* Batch (no controlling terminal) */
case 'C':				/* Commands for parser */
case 'D':				/* Delay */
case 'E':				/* Exit on close */
case 'F':				/* Socket file descriptor */
case 'J':				/* Like j but exits on disconnect */
case 'M':				/* Username for Telnet, Rlogin, etc */
case 'N':               		/* NetBios Adapter Number follows */
case 'P':				/* Pipe */
case 'Q':				/* Use quick settings */
case 'R':				/* Remote-Only */
case 'S':				/* "Stay" - enter interactive */
case 'T':				/* Text file transfer mode */
case 'U':                               /* X.25 call user data */
case 'W':				/* Win32 Window Handle */
case 'X':				/* SET HOST to X.25 address */
case 'Y':				/* No initialization file */
case 'Z':				/* SET HOST to X.25 file descriptor */
case 'a':				/* "as" */
case 'b':   	    			/* set bits-per-second for serial */
case 'c':				/* connect before */
case 'd':				/* DEBUG */
case 'e':				/* Extended packet length */
case 'f':				/* finish */
case 'g':				/* get */
case 'h':				/* help */
case 'i':				/* Treat files as binary */
case 'j':				/* SET HOST (TCP/IP socket) */
case 'k':				/* receive to stdout */
case 'l':				/* SET LINE */
case 'm':				/* Modem type */
case 'n':				/* connect after */
case 'o':                               /* X.25 closed user group */
case 'p':				/* SET PARITY */
case 'q':				/* Quiet */
case 'r':				/* receive */
case 's': 				/* send */
case 't':				/* Line turnaround handshake */
case 'u':                               /* X.25 reverse charge call */
case 'v':				/* Vindow size */
case 'w':				/* Writeover */
case 'x':				/* server */
case 'y':				/* Alternate init-file name */
case 'z':				/* Not background */
#endif /* COMMENT */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"
#include "ckuusr.h"

#ifdef OS2
#include <io.h>
#endif /* OS2 */

#ifdef NETCONN
#ifdef ANYX25
extern int revcall, closgr, cudata;
extern char udata[];
extern int x25fd;
#endif /* ANYX25 */
#ifndef VMS
#ifndef OS2
#ifndef OSK
extern
#endif /* OSK */
#endif /* OS2 */
#endif /* VMS */

int telnetfd;
extern struct keytab netcmd[];
extern int tn_exit;
#ifndef NOICP
#ifndef NODIAL
extern int nnets, nnetdir;		/* Network services directory */
extern char *netdir[];
extern char *nh_p[];			/* Network directory entry pointers */
extern char *nh_p2[];			/* Network directory entry nettype */
#endif /* NODIAL */
extern int nhcount;
extern char * n_name;			/* Network name pointer */
#endif /* NOICP */
#endif /* NETCONN */

#ifndef NOSPL
extern char uidbuf[];
#endif /* NOSPL */

extern char *ckxsys, *ckzsys, *cmarg, *cmarg2, **xargv, **cmlist, *clcmds;
extern int action, cflg, xargc, stdouf, stdinf, displa, cnflg, nfils,
  local, quiet, escape, network, mdmtyp, maxrps, rpsiz, bgset, backgrd, xargs,
  urpsiz, wslotr, swcapr, binary, warn, parity, turn, turnch, duplex, flow,
  fncact, clfils, noinit, stayflg, nettype, cfilef, delay, noherald,
  cmask, cmdmsk, backgrd, exitonclose;
extern long speed;
extern char ttname[];
extern char * pipedata;

#ifdef OS2
extern struct keytab os2devtab[];
extern int nos2dev;
extern int ttslip;
#ifdef OS2PM
extern int os2pm;
#endif /* OS2PM */
#endif /* OS2 */

#ifdef CK_NETBIOS
extern unsigned char NetBiosAdapter;
#endif /* CK_NETBIOS */

#ifdef XFATAL
#undef XFATAL
#endif /* XFATAL */

#ifndef NOICP
#ifndef NODIAL
extern int nmdm, telephony;
extern struct keytab mdmtab[];
extern int usermdm, dialudt;
#endif /* NODIAL */
extern int what;
_PROTOTYP(static int pmsg, (char *) );
_PROTOTYP(static int fmsg, (char *) );
static int pmsg(s) char *s; { printf("%s\n", s); return(0); }
static int fmsg(s) char *s; { fatal(s); return(0); }
#define XFATAL(s) return(what==W_COMMAND?pmsg(s):fmsg(s))
#else
#define XFATAL fatal
#endif /* NOICP */

VOID
fatal2(msg1,msg2) char *msg1, *msg2; {
    char buf[256];
    if (!msg1) msg1 = "";
    if (!msg2) msg2 = "";
    sprintf(buf,"\"%s\" - %s",msg1,msg2);
#ifndef NOICP
    if (what == W_COMMAND)
      printf("%s\n",buf);
    else
#endif /* NOICP */
      fatal((char *)buf);
}

/*  C M D L I N  --  Get arguments from command line  */
/*
 Simple Unix-style command line parser, conforming with 'A Proposed Command
 Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World, Vol.1,
 No.3, 1984.
*/
int
cmdlin() {
    char x;				/* Local general-purpose char */

    cmarg = "";				/* Initialize globals */
    cmarg2 = "";
    action = 0;
    cflg = 0;
 
    debug(F111,"cmdlin",*xargv,xargc);

/* If we were started directly from a Kermit application file, its name is */
/* in argv[1], so skip past it. */

    debug(F101,"cmdlin cfilef","",cfilef);
    if (xargc > 1) {
	if (*xargv[1] != '-') {
	    if (cfilef) {		/* Command file found in prescan() */
		xargc -= 1;		/* Skip past it */
		xargv += 1;
		cfilef = 0;
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
	    if (doarg(x) < 0) {
#ifndef NOICP
		if (what == W_COMMAND)
		  return(0);
		else
#endif /* NOICP */
		  {
#ifdef OS2
		      sleep(1);		/* Give it a chance... */
#endif /* OS2 */
		      doexit(BAD_EXIT,1); /* Go handle option */
		  }
	    }
    	} else {			/* No dash where expected */
	    fatal2(*xargv,
#ifdef NT
		   "invalid command-line option, type \"k95 -h\" for help"
#else
#ifdef OS2
		   "invalid command-line option, type \"ckermit -h\" for help"
#else
		   "invalid command-line option, type \"kermit -h\" for help"
#endif /* OS2 */
#endif /* NT */
		   );
	}
    }
    debug(F101,"action","",action);
#ifndef NOLOCAL
    if (!local) {
	if ((action == 'c') || (cflg != 0)) {
	    XFATAL("-l or -j or -X required");
	}
    }
#endif /* NOLOCAL */
    if (*cmarg2 != 0) {
	if ((action != 's') && (action != 'r') && (action != 'v')) {
	    XFATAL("-a without -s, -r, or -g");
	}
	if (action == 'r' || action == 'v') {
#ifdef CK_TMPDIR
	    if (isdir(cmarg2)) {	/* -a is a directory */
		if (!zchdir(cmarg2)) {	/* try to change to it */
		    XFATAL("can't change to '-a' directory");
		} else cmarg2 = "";
	    } else
#endif /* CK_TMPDIR */
	      if (zchko(cmarg2) < 0) {
		  XFATAL("write access to -a file denied");
	      }
	}
    }
    if ((action == 'v') && (stdouf) && (!local)) {
	if (is_a_tty(1)) {
	    XFATAL("unredirected -k can only be used in local mode");
	}
    }
    if ((action == 's') || (action == 'v') ||
	(action == 'r') || (action == 'x')) {
	if (local)
	  displa = 1;
	if (stdouf) {
	    displa = 0;
	    quiet = 1;
	}
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
    int i, n, y, z, xx; long zz; char *xp;
 
#ifdef NETCONN
#define YYBUFLEN 256
    char tmpbuf[YYBUFLEN+1];		/* Local storage for network things */
    char line[YYBUFLEN+1];
#endif /* NETCONN */

    xp = *xargv+1;			/* Pointer for bundled args */
    debug(F111,"doarg entry",xp,xargc);
    while (x) {
	debug(F000,"doarg arg","",x);
	switch (x) {

#ifndef NOSPL
case 'C':				/* Commands for parser */
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("No commands given for -C");
    }
    clcmds = *xargv;			/* Get the argument (must be quoted) */
    break;
#endif /* NOSPL */

case 'D':				/* Delay */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("missing delay value");
    }
    z = atoi(*xargv);			/* Convert to number */
    if (z > -1)				/* If in range */
      delay = z;			/* set it */
    else {
	XFATAL("bad delay value");
    }
    break;

case 'E':				/* Exit on close */
#ifdef NETCONN
    tn_exit = 1;
#endif /* NETCONN */
    exitonclose = 1;
    break;

#ifndef NOICP
case 'S':				/* "Stay" - enter interactive */
    stayflg = 1;			/* command parser after executing */
    break;				/* command-line actions. */
#endif /* NOICP */

case 'T':				/* File transfer mode = text */
    binary = XYFT_T;
    break;

case 'Q':				/* Quick (i.e. FAST) */
    wslotr = 20;			/* 20 window slots */
    rpsiz = 94;				/* 4K packets */
    urpsiz = 4096;
    if (urpsiz > MAXSP)
      urpsiz = MAXSP;
#ifdef CK_SPEED
    setprefix(PX_CAU);			/* Cautious unprefixing */
#endif /* CK_SPEED */
    break;

case 'R':				/* Remote-Only */
    break;				/* This is handled in prescan(). */

#ifndef NOSERVER
case 'x':				/* server */
    if (action) {
	XFATAL("conflicting actions");
    }
    action = 'x';
    break;
#endif /* NOSERVER */
 
case 'f':				/* finish */
    if (action) {
	XFATAL("conflicting actions");
    }
    action = setgen('F',"","","");
    break;
 
case 'r': {				/* receive */
    if (action) {
	XFATAL("conflicting actions");
    }
    action = 'v';
    break;
  } 
case 'k':				/* receive to stdout */
    if (action) {
	XFATAL("conflicting actions");
    }
    stdouf = 1;
    action = 'v';
    break;
 
case 's': 				/* send */
    if (action) {
	XFATAL("conflicting actions");
    }
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -s");
    }
    nfils = 0;				/* Initialize file counter */
    z = 0;				/* Flag for stdin */
    cmlist = xargv + 1;			/* Remember this pointer */
    while (--xargc > 0) {		/* Traverse the list */	
	xargv++;
	if (**xargv == '-') {		/* Check for sending stdin */
	    if (strcmp(*xargv,"-") != 0) /* Watch out for next option. */
	      break;
	    z++;			/* "-" alone means send from stdin. */
        } else if (zchki(*xargv) > -1) { /* Check if file exists */
	    nfils++;			/* Bump file counter */
	} else if (iswild(*xargv) && zxpand(*xargv) > 0) {
	    /* or contains wildcard characters matching real files */
	    nfils++;
	}
    }
    xargc++, xargv--;			/* Adjust argv/argc */
    if (nfils < 1 && z == 0) {
#ifdef VMS
	XFATAL("%CKERMIT-E-SEARCHFAIL, no files for -s");
#else
	XFATAL("No files for -s");
#endif /* VMS */
    }
    if (z > 1) {
	XFATAL("-s: too many -'s");
    }
    if (z == 1 && nfils > 0) {
	XFATAL("invalid mixture of filenames and '-' in -s");
    }
    debug(F101,"doarg s nfils","",nfils);
    debug(F101,"doarg s z","",z);
    if (nfils == 0) {
	if (is_a_tty(0)) {		/* (used to be is_a_tty(1) - why?) */
	    XFATAL("sending from terminal not allowed");
	} else stdinf = 1;
    }
    debug(F101,"doarg s stdinf","",stdinf);
    debug(F101,*xargv,"",nfils);
    action = 's';
    break;
 
case 'g':				/* get */
    if (action) {
	XFATAL("conflicting actions");
    }
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -g");
    }
    xargv++, xargc--;
    if ((xargc == 0) || (**xargv == '-')) {
    	XFATAL("missing filename for -g");
    }
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
    if (stayflg || what == W_COMMAND)
      break;
    else
#endif /* NOICP */
      doexit(GOOD_EXIT,-1);

case 'a':				/* "as" */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -a");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing name in -a");
    }
    cmarg2 = *xargv;
    break;
 
#ifndef NOICP
case 'Y':				/* No initialization file */
    noinit = 1;
    break;

case 'y':				/* Alternate init-file name */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -y");
    }
    xargv++, xargc--;
    if (xargc < 1) {
	XFATAL("missing filename in -y");
    }
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
case 'J':
case 'j':				/* SET HOST (TCP/IP socket) */
#endif /* TCPSOCKET */
#endif /* NETCONN */
    network = 0;
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -l or -j");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("communication line device name missing");
    }
#ifdef NETCONN
    if (x == 'J') {
	cflg    = 1;			/* Connect */
	stayflg = 1;			/* Stay */
	tn_exit = 1;			/* Telnet-like exit condition */
    }
#endif /* NETCONN */
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
	if (ttopen(ttname,&local,mdmtyp,0) < 0) {
	    XFATAL("can't open device");
	}
	debug(F101,"cmdlin speed","",speed);
	speed = ttgspd();		/* get it. */
#ifdef NETCONN
    } else {
	if (x == 'j' || x == 'J') {	/* IP network host name */
	    char * s = line;
	    char * service = tmpbuf;
	    if (xargc > 0) {		/* Check if it's followed by */
		/* A service name or number */
		if (*(xargv+1) && *(*(xargv+1)) != '-') {
		    xargv++, xargc--;		    
		    strcat(ttname,":");
		    strcat(ttname,*xargv);
		}
	    }
	    nettype = NET_TCPB;
	    mdmtyp = -nettype;          /* Perhaps already set in init file */
	    telnetfd = 1;		/* Or maybe an open file descriptor */
	    
	    strncpy(line, ttname, YYBUFLEN); /* Working copy of the name */
	    for (s = line; *s != '\0' && *s != ':'; s++); /* and service */
	    if (*s) {
		*s++ = '\0';
		strncpy(service, s, YYBUFLEN);
	    } else *service = '\0';
	    s = line;
#ifndef NODIAL
#ifndef NOICP
	    /* Look up in network directory */
	    x = 0;
	    if (*s == '=') {		/* If number starts with = sign */
		s++;			/* strip it */
		while (*s == SP)	/* and also any leading spaces */
		  s++;
		strncpy(line,s,YYBUFLEN); /* Do this again. */
		nhcount = 0;
	    } else if (!isdigit(line[0])) {
/*
  nnetdir will be greater than 0 if the init file has been processed and it
  contained a SET NETWORK DIRECTORY command.
*/
		xx = 0;			/* Initialize this */
		if (nnetdir > 0)	/* If there is a directory... */
		  xx = lunet(line);	/* Look up the name */
		else			/* If no directory */
		  nhcount = 0;		/* we didn't find anything there */
		if (xx < 0) {		/* Lookup error: */
		    sprintf(tmpbuf,
			    "?Fatal network directory lookup error - %s\n",
			    line
			    );
		    XFATAL(tmpbuf);
		}
	    }
#endif /* NOICP */
#endif /* NODIAL */
	    /* Add service to line specification for ttopen() */
	    if (*service) {		/* There is a service specified */
		strcat(line, ":");
		strcat(line, service);
	    } else {
		strcat(line, ":telnet");
	    }       

#ifndef NOICP
#ifndef NODIAL
	    if ((nhcount > 1) && !quiet && !backgrd) {
		printf("%d entr%s found for \"%s\"%s\n",
                       nhcount,
                       (nhcount == 1) ? "y" : "ies",
                       s,
                       (nhcount > 0) ? ":" : "."
                       );
		for (i = 0; i < nhcount; i++)
		  printf("%3d. %s %-12s => %s\n",
			 i+1, n_name, nh_p2[i], nh_p[i]
			 );
	    }
	    if (nhcount == 0)
	      n = 1;
	    else
	      n = nhcount;
#else
	    n = 1;
	    nhcount = 0;
#endif /* NODIAL */
	    for (i = 0; i < n; i++) {
#ifndef NODIAL
		if (nhcount >= 1) {
		    strcpy(line,nh_p[i]); /* Copy the current entry to line */
		    /* Check to see if the network entry contains a service */
		    for ( s = line ; (*s != '\0') && (*s != ':'); s++)
		      ;
		    /* If directory does not have a service ... */
		    if (!*s && *service) { /* and the user specified one */
			strcat(line, ":");
			strcat(line, service);
		    }
		    if (lookup(netcmd,nh_p2[i],nnets,&z) > -1) {
			mdmtyp = 0 - netcmd[z].kwval;
		    } else {
			printf("Error - network type \"%s\" not supported\n",
                               nh_p2[i]
			       );
			continue;
		    }		    
		}
#endif /* NODIAL */
	    }
#endif /* NOICP */
	    strcpy(ttname, line);
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
	if (ttopen(ttname,&local,mdmtyp,0) < 0) {
	    XFATAL("can't open host connection");
	}
	network = 1;
#endif /* NETCONN */
    }
    /* add more here later - decnet, etc... */
    break;
 
#ifdef ANYX25
case 'U':                               /* X.25 call user data */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing call user data string");
    }
    strcpy(udata,*xargv);
    if ((int)strlen(udata) <= MAXCUDATA) {
	cudata = 1;
    } else {
	XFATAL("Invalid call user data");
    }
    break;

case 'o':                               /* X.25 closed user group */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing closed user group index");
    }
    z = atoi(*xargv);			/* Convert to number */
    if (z >= 0 && z <= 99) {
	closgr = z;
    } else {
	XFATAL("Invalid closed user group index");
    }
    break;

case 'u':                               /* X.25 reverse charge call */
    revcall = 1;
    break;
#endif /* ANYX25 */
#endif /* NOLOCAL */

case 'b':				/* Bits-per-second for serial device */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling"); 
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing bps");
    }
    zz = atol(*xargv);			/* Convert to long int */
    i = zz / 10L;
#ifndef NOLOCAL
    if (ttsspd(i) > -1)			/* Check and set it */
#endif /* NOLOCAL */
      speed = ttgspd();			/* and read it back. */
#ifndef NOLOCAL
    else {
	XFATAL("unsupported transmission rate");
    }
#endif /* NOLOCAL */
    break;
 
#ifndef NODIAL
#ifndef NOICP
case 'm':				/* Modem type */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -m");    
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("modem type missing");
    }
    y = lookup(mdmtab,*xargv,nmdm,&z);
    if (y < 0) {
	XFATAL("unknown modem type");
    }
    usermdm = 0;
    usermdm = (y == dialudt) ? x : 0;
    initmdm(y);
    break;
#endif /* NOICP */
#endif /* NODIAL */

case 'e':				/* Extended packet length */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -e");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing length");
    }
    z = atoi(*xargv);			/* Convert to number */
    if (z > 10 && z <= maxrps) {
        rpsiz = urpsiz = z;
	if (z > 94) rpsiz = 94;		/* Fallback if other Kermit can't */
    } else {
	XFATAL("Unsupported packet length");
    }
    break;

case 'v':				/* Vindow size */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing or bad window size");
    }
    z = atoi(*xargv);			/* Convert to number */
    if (z < 32) {			/* If in range */
	wslotr = z;			/* set it */
	if (z > 1) swcapr = 1;		/* Set capas bit if windowing */
    } else {
	XFATAL("Unsupported packet length");
    }
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

case '8':				/* 8-bit clean */
    parity = 0;
    cmdmsk = 0xff;
    cmask = 0xff;
    break;

case 'p':				/* SET PARITY */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("missing parity");
    }
    switch(x = **xargv) {
	case 'e':
	case 'o':
	case 'm':
	case 's': parity = x; break;
	case 'n': parity = 0; break;
	default:  { XFATAL("invalid parity"); }
        }
    break;
 
case 't':				/* Line turnaround handshake */
    turn = 1;				
    turnch = XON;			/* XON is turnaround character */
    duplex = 1;				/* Half duplex */
    flow = 0;				/* No flow control */
    break;
 
case 'B':
    bgset = 1;				/* Force background (batch) */
    backgrd = 1;
    break;

case 'z':				/* Force foreground */
    bgset = 0;
    backgrd = 0;
    break;

#ifndef NOSPL
case 'M':				/* My User Name */
    /* Already done in prescan() */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
    }
#ifdef COMMENT
    if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
    }
    strcpy(uidbuf,*xargv);
#endif /* COMMENT */
    break;
#endif /* NOSPL */

#ifdef CK_NETBIOS
case 'N':		               /* NetBios Adapter Number follows */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -N");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("missing NetBios Adapter number");
    }
    if ((strlen(*xargv) != 1) ||
	(*xargv)[0] != 'X' ||
	((atoi(*xargv) < 0) ||
	 (atoi(*xargv) > 9)) ) {
	XFATAL("Invalid NetBios Adapter - Adapters 0 to 9 are valid");
    }
    break;
#endif /* CK_NETBIOS */

#ifdef NETCONN
case 'F':
    network = 1;
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -F");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("network file descriptor missing");
    }
    strcpy(ttname,*xargv);
    nettype = NET_TCPB;
    mdmtyp = -nettype;
    telnetfd = 1;
    local = 1;
    break;
#endif /* NETCONN */

#ifdef OS2PM
case 'P':				/* OS/2 Presentation Manager */
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -P"); 
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
    	XFATAL("pipe data missing");
    }
    pipedata = *xargv;
    break;
#endif /* OS2PM */

#ifndef NOICP
case 'H':
    noherald = 1;
    break;
#endif /* NOICP */

#ifdef OS2
case 'W':
    if (*(xp+1)) {
	XFATAL("invalid argument bundling after -W"); 
    }
    xargv++, xargc--;
    if ((xargc < 1)) { /* could be negative */
	XFATAL("Window handle missing");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
	XFATAL("Kermit Instance missing");
    }
    /* Action done in prescan */
    break;
#endif /* OS2 */

default:
    fatal2(*xargv,
#ifdef NT
		   "invalid command-line option, type \"k95 -h\" for help"
#else
#ifdef OS2
		   "invalid command-line option, type \"ckermit -h\" for help"
#else
		   "invalid command-line option, type \"kermit -h\" for help"
#endif /* OS2 */
#endif /* NT */
	   );
        }
 
    x = *++xp;				/* See if options are bundled */
    }
    return(0);
}
#else /* No command-line interface... */

extern int xargc;
int
cmdlin() {
    if (xargc > 1) {
	XFATAL("Sorry, command-line options disabled.");
    }
}
#endif /* NOCMDL */
