#include "ckcsym.h"

/*  C K U U S X --  "User Interface" common functions. */

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

/*
  This module contains user interface functions needed by both the interactive
  user interface and the command-line-only user interface.
*/

/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"

extern xx_strp xxstring;

#ifndef NETCONN
/*
  We should just pull in ckcnet.h here, but it causes a conflict with curses.h.
*/
#ifdef TCPSOCKET
#define NETCONN
#else
#ifdef SUNX25
#define NETCONN
#else
#ifdef STRATUSX25
#define NETCONN
#else
#ifdef DECNET
#define NETCONN
#else
#ifdef NPIPE
#define NETCONN
#else
#ifdef CK_NETBIOS
#define NETCONN
#endif /* TCPSOCKET */
#endif /* SUNX25 */
#endif /* STRATUSX25 */
#endif /* DECNET */
#endif /* NPIPE */
#endif /* CK_NETBIOS */
#endif /* NETCONN */

#ifdef OS2
#ifdef CK_NETBIOS
#include "os2.h"
#ifdef COMMENT				/* Would you believe */
#undef COMMENT				/* <os2.h> defines this ? */
#endif /* COMMENT */
#include "ckonbi.h"
extern PNCB pRecvNCB, pSendNCB[MAXWS], pWorkNCB ;
extern UCHAR NetBiosAdapter ;
extern BOOL NetbeuiAPI ;
#endif /* CK_NETBIOS */

typedef struct ascreen_rec {		/* Structure for saving screen info */
    unsigned char   ox;
    unsigned char   oy;
    unsigned char   att;
    char            *scrncpy;
} ascreen;
extern ascreen commandscreen;

/* Functions from ckocon.c */
void savescreen(ascreen *, int, int);
int  restorescreen(ascreen *);
#endif /* OS2 */

#ifndef WINTCP
#include <signal.h>
#endif /* WINTCP */

#ifdef VMS
#ifdef WINTCP
#include <descrip.h>
#include <ssdef.h>
#include <stsdef.h>
#include <signal.h>
#else
#include <descrip.h>
#include <ssdef.h>
#include <stsdef.h>
#endif /* WINTCP */
#endif /* VMS */

/* Used internally */

_PROTOTYP( VOID screenc, (int, char, long, char *) );
#ifdef CK_CURSES
_PROTOTYP( static long shocps, (int) );
_PROTOTYP( static long shoetl, (long, long, long, long) );
#endif /* CK_CURSES */

static int ft_win = 0;  /* Fullscreen file transfer display window is active */

/* Variables declared here */

int fdispla = XYFD_R;			/* File transfer display type */
int tt_crd = 0;				/* Carriage return display */

#ifdef DEBUG
char debfil[50];			/* Debugging log file name */
#endif /* DEBUG */

#ifdef TLOG
char trafil[50];			/* Transaction log file name */
#endif /* TLOG */

char pktfil[50];			/* Packet log file name */
char sesfil[50];			/* Session log file name */

#ifndef NOFRILLS
char optbuf[100];			/* Options for MAIL or REMOTE PRINT */
#endif /* NOFRILLS */

#ifdef DYNAMIC
static char *cmdstr = NULL;		/* Place to build generic command */
#else
#ifdef pdp11
static char cmdstr[256];
#else
static char cmdstr[4096];
#endif /* pdp11 */
#endif /* DYNAMIC */

char fspec[FSPECL];			/* Filename string for \v(filespec) */

/*  C C N T A B  --  Names of ASCII control characters 0-31 */

char *ccntab[] = { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
 "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
 "DLE", "DC1/XON", "DC2", "DC3/XOFF", "DC4", "NAK", "SYN", "ETB", "CAN",
 "EM", "SUB", "ESC", "FS", "GS", "RS", "US"
};

int success = 1,			/* Command success/failure flag */

#ifndef NOSPL
    cmdlvl = 0,				/* Command level */
#endif /* NOSPL */
    action,				/* Action selected on command line */
    sessft = 0,				/* Session log file type, 0 = text */
    pflag = 1,				/* Print prompt */
    msgflg = 1;				/* Print informational messages */

#ifndef NOMSEND				/* Multiple SEND */
char *msfiles[MSENDMAX];
#endif /* NOMSEND */

#ifdef CK_RESEND
extern int sendmode;
extern long sendstart, rs_len;
#endif /* CK_RESEND */

#ifdef CK_APC
extern int apcactive;
#endif /* CK_APC */
/* External variables */

#ifndef NODIAL
extern FILE * dialfd;			/* Dialing directory */
#endif /* NODIAL */

#ifdef CK_PCT_BAR			/* File transfer thermometer */
int thermometer = 1;			/* ON by default */
#endif /* CK_PCT_BAR */

extern int local, quiet, binary, bctu, rptflg, ebqflg, network, server,
  what, spsiz, urpsiz, wmax, czseen, cxseen, winlo, displa, timint, parity,
  npad, ebq, ebqflg, bctr, rptq, atcapu, lpcapu, swcapu, wslotn, wslotr, rtimo,
  mypadn, sq, capas, rpsiz, tsecs, dfloc, tralog, pktlog, seslog, lscapu,
  xitsta, escape, tlevel, bgset, backgrd, wslots, suspend, srvdis, nettype,
  spackets, spktl, rpktl, retrans, wcur, numerrs, fsecs, sendmode, whatru;

#ifdef datageneral			/* 2/12/92 ENH */
#include <sysid.h>
extern int con_reads_mt, conint_ch, conint_avl;
#endif /* datageneral */

extern long speed, filcnt, ffc, tfc, rptn, fsize, sendstart, rs_len;
long oldcps = 0L, cps = 0L;

extern CHAR *rdatap, padch, seol, ctlq, mypadc, eol;

extern char ttname[], *dftty, *cmarg, **cmlist, *versio, myhost[];
#ifndef NOICP
#ifdef DCMDBUF
extern char *cmdbuf;			/* Command buffer */
#else
extern char cmdbuf[];			/* Command buffer */
#endif /* DCMDBUF */
#endif /* NOICP */

#ifndef NOCCTRAP
#include <setjmp.h>			/* Control-C trap */
#ifdef CK_POSIX_SIG			/* POSIX signal handling */
sigjmp_buf cmjbuf;
#else
jmp_buf cmjbuf;
#endif /* CK_POSIX_SIG */
#endif /* NOCCTRAP */

#ifndef NOCSETS
#include "ckcxla.h"
extern int fcharset, tcharset;
extern struct csinfo fcsinfo[], tcsinfo[];
#endif /* NOCSETS */

/*  P A R N A M  --  Return parity name */

char *
#ifdef CK_ANSIC
parnam(char c)
#else
parnam(c) char c;
#endif /* CK_ANSIC */
/* parnam */ {
    switch (c) {
	case 'e': return("even");
	case 'o': return("odd");
	case 'm': return("mark");
	case 's': return("space");
	case 0:   return("none");
	default:  return("invalid");
    }
}

/*  S H O M D M  --  Show modem signals  */

VOID
shomdm() {
/*
  Note use of "\r\n" to make sure this report prints right, even when
  called during CONNECT mode.
*/
    int y;
    y = ttgmdm();
    switch (y) {
      case -3: printf(
	         "Modem signals unavailable in this version of Kermit\r\n");
	       break;
      case -2: printf("No modem control for this device\r\n"); break;
      case -1: printf("Modem signals unavailable\r\n"); break;
      default:
#ifndef MAC
        printf(
	  " Carrier Detect      (CD):  %s\r\n",(y & BM_DCD) ? "On": "Off");
	printf(
          " Dataset Ready       (DSR): %s\r\n",(y & BM_DSR) ? "On": "Off");
#endif /* MAC */
	printf(
          " Clear To Send       (CTS): %s\r\n",(y & BM_CTS) ? "On": "Off");
#ifndef STRATUS
#ifndef MAC
        printf(
          " Ring Indicator      (RI):  %s\r\n",(y & BM_RNG) ? "On": "Off");
#endif /* MAC */
        printf(
          " Data Terminal Ready (DTR): %s\r\n",(y & BM_DTR) ? "On": "Off");
#ifndef MAC
        printf(
          " Request to Send     (RTS): %s\r\n",(y & BM_RTS) ? "On": "Off");
#endif /* MAC */
#endif /* STRATUS */
    }
}

/*  S D E B U  -- Record spar results in debugging log  */

VOID
sdebu(len) int len; {
    debug(F111,"spar: data",(char *) rdatap,len);
    debug(F101," spsiz ","", spsiz);
    debug(F101," timint","",timint);
    debug(F101," npad  ","",  npad);
    debug(F101," padch ","", padch);
    debug(F101," seol  ","",  seol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  rptq);
    debug(F101," rptflg","",rptflg);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotn","", wslotn);
    debug(F101," whatru","", whatru);
}
/*  R D E B U -- Debugging display of rpar() values  */

VOID
rdebu(d,len) CHAR *d; int len; {
    debug(F111,"rpar: data",d,len);
    debug(F101," rpsiz ","", xunchar(d[0]));
    debug(F101," rtimo ","", rtimo);
    debug(F101," mypadn","",mypadn);
    debug(F101," mypadc","",mypadc);
    debug(F101," eol   ","",   eol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," sq    ","",    sq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  d[8]);
    debug(F101," rptflg","",rptflg);
    debug(F101," capas ","", capas);
    debug(F101," bits  ","",d[capas]);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotr","", wslotr);
    debug(F101," rpsiz(extended)","",rpsiz);
}

#ifdef COMMENT
/*  C H K E R R  --  Decide whether to exit upon a protocol error  */

VOID
chkerr() {
    if (backgrd && !server) fatal("Protocol error");
}
#endif /* COMMENT */

/*  F A T A L  --  Fatal error message */

VOID
fatal2(msg1,msg2) char *msg1, *msg2; {
    if (!msg1) msg1 = "";
    if (!msg2) msg2 = "";
    sprintf(fspec,"\"%s\" - %s",msg1,msg2);
    fatal(fspec);
}

VOID
fatal(msg) char *msg; {
    if (!msg) msg = "";
#ifdef VMS
    if (strncmp(msg,"%CKERMIT",8))
      conol("%CKERMIT-E-FATAL, ");
    conoll(msg);
#else /* !VMS */
    screen(SCR_EM,0,0L,msg);
#endif /* VMS */
    debug(F110,"fatal",msg,0);
    tlog(F110,"Fatal:",msg,0L);
    doexit(BAD_EXIT,xitsta | 1);	/* Exit indicating failure */
}

/*  B L D L E N  --  Make length-encoded copy of string  */

char *
bldlen(str,dest) char *str, *dest; {
    int len;
    len = (int)strlen(str);
    if (len > 94)
      *dest = SP;
    else
      *dest = tochar(len);
    strcpy(dest+1,str);
    return(dest+len+1);
}


/*  S E T G E N  --  Construct a generic command  */
/*
  Call with Generic command character followed by three string arguments.  
  Trailing strings are allowed to be empty ("").  Each string except the last
  non-empty string must be less than 95 characters long.  The final nonempty
  string is allowed to be longer.
*/
CHAR
#ifdef CK_ANSIC
setgen(char type, char * arg1, char * arg2, char * arg3)
#else
setgen(type,arg1,arg2,arg3) char type, *arg1, *arg2, *arg3;
#endif /* CK_ANSIC */
/* setgen */ {
    char *upstr, *cp;
#ifdef DYNAMIC
    if (!cmdstr)
      if (!(cmdstr = malloc(MAXSP + 1)))
	fatal("setgen: can't allocate memory");
#endif /* DYNAMIC */

    cp = cmdstr;
    *cp++ = type;
    *cp = NUL;
    if (!arg1) arg1 = "";
    if (!arg2) arg2 = "";
    if (!arg3) arg3 = "";
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

#ifndef NOMSEND
static char *mgbufp = NULL;

/*  F N P A R S E  --  */

/*
  Argument is a character string containing one or more filespecs.
  This function breaks the string apart into an array of pointers, one
  to each filespec, and returns the number of filespecs.  Used by server
  when it receives a GET command to allow it to process multiple file
  specifications in one transaction.  Sets cmlist to point to a list of
  file pointers, exactly as if they were command line arguments.

  This version of fnparse treats spaces as filename separators.  If your
  operating system allows spaces in filenames, you'll need a different
  separator.

  This version of fnparse mallocs a string buffer to contain the names.  It
  cannot assume that the string that is pointed to by the argument is safe.
*/
int
fnparse(string) char *string; {
    char *p, *s, *q;
    int r = 0, x;			/* Return code */

    if (mgbufp) free(mgbufp);		/* Free this from last time. */
    mgbufp = malloc((int)strlen(string)+2);
    if (!mgbufp) {
	debug(F100,"fnparse malloc error","",0);
	return(0);
    }	
#ifndef NOICP
#ifndef NOSPL
    strncpy(fspec,string,FSPECL);	/* Make copy for \v(filespec) */
#endif /* NOSPL */
#endif /* NOICP */
    s = string;				/* Input string */
    p = q = mgbufp;			/* Point to the copy */
    r = 0;				/* Initialize our return code */
    while (*s == SP || *s == HT)	/* Skip leading spaces and tabs */
      s++;
    for (x = strlen(s);			/* Strip trailing spaces */
	 (x > 1) && (s[x-1] == SP || s[x-1] == HT);
	 x--)
      s[x-1] = NUL;
    while (1) {				/* Loop through rest of string */
	if (*s == CMDQ) {		/* Backslash (quote character)? */
	    if ((x = xxesc(&s)) > -1) {	/* Go interpret it. */
		*q++ = (char) x;	/* Numeric backslash code, ok */
	    } else {			/* Just let it quote next char */
		s++;			/* get past the backslash */
		*q++ = *s++;		/* deposit next char */
	    }
	    continue;
	} else if (*s == SP || *s == NUL) { /* Unquoted space or NUL? */
	    *q++ = NUL;			/* End of output filename. */
	    msfiles[r] = p;		/* Add this filename to the list */
	    debug(F111,"fnparse",msfiles[r],r);
	    r++;			/* Count it */
	    if (*s == NUL) break;	/* End of string? */
	    while (*s == SP) s++;	/* Skip repeated spaces */
	    p = q;			/* Start of next name */
	    continue;
	} else *q++ = *s;		/* Otherwise copy the character */
	s++;				/* Next input character */
    }
    debug(F101,"fnparse r","",r);
    msfiles[r] = "";			/* Put empty string at end of list */
    cmlist = msfiles;
    return(r);
}
#endif /* NOMSEND */

char *					/* dbchr() for DEBUG SESSION */
dbchr(c) int c; {
    static char s[8];
    char *cp = s;

    c &= 0xff;
    if (c & 0x80) {			/* 8th bit on */
	*cp++ = '~';
	c &= 0x7f;
    }
    if (c < SP) {			/* Control character */
	*cp++ = '^';
	*cp++ = ctl(c);
    } else if (c == DEL) {
	*cp++ = '^';
	*cp++ = '?';
    } else {				/* Printing character */
	*cp++ = c;
    }
    *cp = '\0';				/* Terminate string */
    cp = s;				/* Return pointer to it */
    return(cp);
}

/*  C K H O S T  --  Get name of local host (where C-Kermit is running)  */

/*
  Call with pointer to buffer to put hostname in, and length of buffer.
  Copies hostname into buffer on success, puts null string in buffer on
  failure.
*/
#ifdef BSD44
#define BSD4
#undef ATTSV
#endif /* BSD44 */

#ifdef SVORPOSIX
#ifndef BSD44
#ifndef apollo
#include <sys/utsname.h>
#endif /* apollo */
#endif /* BSD44 */
#endif /* SVORPOSIX*/

VOID
ckhost(vvbuf,vvlen) char * vvbuf; int vvlen; {
#ifdef pdp11
    *vvbuf = NUL;
#else
    char *g;
#ifdef VMS
    int x;
#endif /* VMS */
#ifdef SVORPOSIX
#ifndef BSD44
#ifndef _386BSD
    struct utsname hname;
#endif /* _386BSD */
#endif /* BSD44 */
#endif /* SVORPOSIX */
#ifdef datageneral
    int ac0 = (char *) vvbuf, ac1 = -1, ac2 = 0;
#endif /* datageneral */

    *vvbuf = NUL;
#ifndef OXOS
#ifdef SVORPOSIX
#ifdef BSD44
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
#ifdef _386BSD
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
#ifdef QNX
#ifdef TCPSOCKET
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
    if (uname(&hname) > -1) strncpy(vvbuf,hname.nodename,vvlen);
#endif /* TCPSOCKET */
#else /* SVORPOSIX but not _386BSD or BSD44 */
    if (uname(&hname) > -1) strncpy(vvbuf,hname.nodename,vvlen);
#endif /* QNX */
#endif /* _386BSD */
#endif /* BSD44 */
#else /* !SVORPOSIX */
#ifdef BSD4
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else /* !BSD4 */
#ifdef VMS
    g = getenv("SYS$NODE");
    if (g) strncpy(vvbuf,g,vvlen);
    x = (int)strlen(vvbuf);
    if (x > 1 && vvbuf[x-1] == ':' && vvbuf[x-2] == ':') vvbuf[x-2] = NUL;
#else
#ifdef datageneral
    if (sys($HNAME,&ac0,&ac1,&ac2) == 0) /* successful */
        vvlen = ac2 + 1;		/* enh - have to add one */
#else
#ifdef OS2				/* OS/2 */
    g = getenv("HOSTNAME");		/* (Created by TCP/IP install) */
    if (!g) g = getenv("SYSTEMNAME");	/* (Created by PATHWORKS install?) */
    if (g) strncpy(vvbuf,g,vvlen);
#endif /* OS2 */
#endif /* datageneral */
#endif /* VMS */
#endif /* BSD4 */
#endif /* SVORPOSIX */
#else /* OXOS */
    /* If TCP/IP is not installed, gethostname() fails, use uname() */
    if (gethostname(vvbuf,vvlen) < 0) {
	if (uname(&hname) > -1)
	    strncpy(vvbuf,hname.nodename,vvlen);
	else
	    *vvbuf = NUL;
    }
#endif /* OXOS */
    if (*vvbuf == NUL) {		/* If it's still empty */
        g = getenv("HOST");		/* try this */
        if (g) strncpy(vvbuf,g,vvlen);
    }
    vvbuf[vvlen-1] = NUL;		/* Make sure result is terminated. */
#endif /* pdp11 */
}
#ifdef BSD44
#undef BSD4
#define ATTSV
#endif /* BSD44 */

#ifndef NOSPL
#define ASKMORE
#endif /* NOSPL */
#ifndef NOHELP
#ifndef ASKMORE
#define ASKMORE
#endif /* ASKMORE */
#endif /* NOHELP */

#ifdef ASKMORE
/*
  A S K M O R E  --  Poor person's "more".
  Returns 0 if no more, 1 if more wanted.
  Presently used by SHO MAC, SHO GLOB, SHO VAR, and HELP, so compiled out if
  those options are also compiled out.
*/
int
askmore() {
    char c; int rv;

    rv = -1;
    while (rv < 0) {
#ifndef OS2
	printf("more? ");
#ifdef UNIX
#ifdef NOSETBUF
	fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
#else
	printf("more? (Y or space-bar for yes, N for no) ");
	fflush(stdout);
#endif /* OS2 */
	c = coninc(0);
	switch (c) {
	  /* Yes */
	  case SP: case 'y': case 'Y': case 012:  case 015:
	    printf("\015      \015");
	    rv = 1;
	    break;
          /* No */
	  case 'n': case 'N': case 'q': case 'Q':
	    printf("\015\012");
	    rv = 0;
	    break;
	  /* Invalid answer */
	  default:
	    printf("Y or space-bar for yes, N for no\n");
	    continue;
	}
#ifdef OS2
	printf("\r                                         \r");
	fflush(stdout);
#endif /* OS2 */
    }
    return(rv);
}
#endif /* ASKMORE */

/*  T R A P  --  Terminal interrupt handler */

SIGTYP
trap(sig) int sig; {
#ifdef VMS
    int i; FILE *f;
#endif /* VMS */
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
#ifdef GEMDOS
/* GEM is not reentrant, no i/o from interrupt level */
    longjmp(cmjbuf,1);			/* Jump back to parser now! */
#endif /* GEMDOS */
    debug(F101,"^C trap() caught signal","",sig);
    zclose(ZIFILE);			/* If we were transferring a file, */
    zclose(ZOFILE);			/* close it. */
#ifdef CK_APC
    apcactive = 0;
#endif /* CK_APC */

#ifdef VMS
/*
  Fix terminal.
*/
    if (ft_win) {			/* If curses window open */
	debug(F100,"^C trap() curses","",0);
	screen(SCR_CW,0,0L,"");		/* Close it */
	conres();			/* Restore terminal */
	i = printf("^C...");		/* Echo ^C to standard output */
    } else {
	conres();
	i = printf("^C...\n");		/* Echo ^C to standard output */
    }
    if (i < 1 && ferror(stdout)) {	/* If there was an error */
	debug(F100,"^C trap() error","",0);	
	fclose(stdout);			/* close standard output */
	f = fopen(dftty, "w");		/* open the controlling terminal */
	if (f) stdout = f;		/* and make it standard output */
	printf("^C...\n");		/* and echo the ^C again. */
    }
#else					/* Not VMS */
#ifdef STRATUS
    conres();				/* Set console back to normal mode */
#endif /* STRATUS */
    if (ft_win) {			/* If curses window open, */
	debug(F100,"^C trap() curses","",0);
	screen(SCR_CW,0,0L,"");		/* close it. */
	printf("^C...");		/* Echo ^C to standard output */
    } else {
	printf("^C...\n");
    }
#endif /* VMS */
#ifdef datageneral
    connoi_mt(); 			/* Kill asynch task that listens to */
    ttimoff();
    conres();				/* the keyboard */
#endif /* datageneral */

#ifndef NOCCTRAP
/*  This is stupid -- every version should have ttimoff()...  */
#ifdef UNIX
    ttimoff();				/* Turn off any timer interrupts */
#else
#ifdef OSK
    ttimoff();				/* Turn off any timer interrupts */
#else
#ifdef STRATUS
    ttimoff();				/* Turn off any timer interrupts */
#else
#ifdef OS2
#ifdef CK_NETBIOS 
   if ( netbiosAvail ) {
       int i ;
       NCB CancelNCB ;
       /* Cancel all outstanding Netbios Send or Work requests */

       for ( i = 0 ; i < MAXWS ; i++) {
	   if ( pSendNCB[i]->basic_ncb.bncb.ncb_retcode ==
	       NB_COMMAND_IN_PROCESS ) {
	       NCBCancel( NetbeuiAPI,
			 &CancelNCB,
			 NetBiosAdapter,
			 pSendNCB[i] ) ;
	       Dos16SemWait( pSendNCB[i]->basic_ncb.ncb_semaphore,
			    SEM_INDEFINITE_WAIT ) ;
	   }
       }
       if ( pWorkNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS ) {
	   NCBCancel( NetbeuiAPI, &CancelNCB, NetBiosAdapter, pWorkNCB ) ;
	   Dos16SemWait( pWorkNCB->basic_ncb.ncb_semaphore,
                        SEM_INDEFINITE_WAIT ) ;
       }
   }
#endif /* CK_NETBIOS */
   ttimoff();				/* Turn off any timer interrupts */
#else
#ifdef VMS
    ttimoff();				/* Turn off any timer interrupts */
#endif /* VMS */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */

#ifdef OSK
    sigmask(-1);
/*
  We are in an intercept routine but do not perform a F$RTE (done implicitly
  but rts).  We have to decrement the sigmask as F$RTE does.  Warning:
  longjump only restores the cpu registers, NOT the fpu registers.  So don't
  use fpu at all or at least don't use common fpu (double or float) register
  variables.
*/
#endif /* OSK */
#ifdef CK_POSIX_SIG
    siglongjmp(cmjbuf,1);			/* Jump back to parser */
#else
    longjmp(cmjbuf,1);
#endif /* CK_POSIX_SIG */
#else
/* No Ctrl-C trap, just exit. */
#ifdef CK_CURSES			/* Curses support? */
    screen(SCR_CW,0,0L,"");		/* Close curses window */
#endif /* CK_CURSES */
    doexit(BAD_EXIT,what);		/* Exit poorly */
#endif /* NOCCTRAP */
    SIGRETURN;
}

/*  C K _ T I M E  -- Returns pointer to current time. */

char *
ck_time() {
    static char tbuf[10];
    char *p;
    int x;

    ztime(&p);				/* "Thu Feb  8 12:00:00 1990" */
    if (!p)				/* like asctime()! */
      return("");
    if (*p) {
	for (x = 11; x < 19; x++)	/* copy hh:mm:ss */
	  tbuf[x - 11] = p[x];		/* to tbuf */
	tbuf[8] = NUL;			/* terminate */
    }
    return(tbuf);			/* and return it */
}

/*  C C _ C L E A N  --  Cleanup after terminal interrupt handler */

#ifdef GEMDOS
int
cc_clean() {
    zclose(ZIFILE);			/* If we were transferring a file, */
    zclose(ZOFILE);			/* close it. */
    printf("^C...\n");			/* Not VMS, no problem... */
}
#endif /* GEMDOS */


/*  S T P T R A P -- Handle SIGTSTP (suspend) signals */

SIGTYP
stptrap(sig) int sig; {
#ifndef NOJC
    int x; extern int cmflgs;
    debug(F101,"stptrap() caught signal","",sig);
    if (!suspend) {
	printf("\r\nsuspend disabled\r\n");
#ifndef NOICP
	if (what == W_COMMAND) {	/* If we were parsing commands */
	    prompt(xxstring);		/* reissue the prompt and partial */
	    if (!cmflgs)		/* command (if any) */
	      printf("%s",cmdbuf);
	}
#endif /* NOICP */
    } else {
	conres();			/* Reset the console */
#ifndef OS2
	/* Flush pending output first, in case we are continued */
	/* in the background, which could make us block */
	fflush(stdout);

	x = psuspend(suspend);		/* Try to suspend. */
	if (x < 0)
#endif /* OS2 */
	  printf("Job control not supported\r\n");
	conint(trap,stptrap);		/* Rearm the trap. */
	debug(F100,"stptrap back from suspend","",0);
	switch (what) {
	  case W_CONNECT:		/* If suspended during CONNECT? */
	    conbin((char)escape);	/* put console back in binary mode */
	    debug(F100,"stptrap W_CONNECT","",0);
	    break;
#ifndef NOICP
	  case W_COMMAND:		/* Suspended in command mode */
	    debug(F101,"stptrap W_COMMAND pflag","",pflag);
	    concb((char)escape);	/* Put back CBREAK tty mode */
	    if (pflag) {		/* If command parsing was */
		prompt(xxstring);	/* reissue the prompt and partial */
		if (!cmflgs)		/* command (if any) */
		  printf("%s",cmdbuf);
	    }
	    break;
#endif /* NOICP */
	  default:			/* All other cases... */
	    debug(F100,"stptrap default","",0);
	    concb((char)escape);	/* Put it back in CBREAK mode */
	    break;
	}
    }
#endif /* NOJC */
    SIGRETURN;
}

#ifdef TLOG
#define TBUFL 300
/*  T L O G  --  Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1,s2 - String arguments 1 and 2.
   n  - Int, argument 3.
*/
VOID
#ifdef CK_ANSIC
tlog(int f, char *s1, char *s2, long n)
#else
tlog(f,s1,s2,n) int f; long n; char *s1, *s2;
#endif /* CK_ANSIC */
/* tlog */ {
    static char s[TBUFL];
    char *sp = s; int x;

    if (!tralog) return;		/* If no transaction log, don't */
    switch (f) {
    	case F000:			/* 0 (special) "s1 n s2"  */
	    if ((int)strlen(s1) + (int)strlen(s2) + 15 > TBUFL)
	      sprintf(sp,"?T-Log string too long");
	    else sprintf(sp,"%s %ld %s",s1,n,s2);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
    	case F001:			/* 1, " n" */
	    sprintf(sp," %ld",n);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
    	case F010:			/* 2, "[s2]" */
	    x = (int)strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (x + 6 > TBUFL)
	      sprintf(sp,"?String too long");
	    else sprintf(sp,"[%s]",s2);
	    if (zsoutl(ZTFILE,"") < 0) tralog = 0;
	    break;
    	case F011:			/* 3, "[s2] n" */
	    x = (int)strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if (x + 6 > TBUFL)
	      sprintf(sp,"?String too long");
	    else sprintf(sp,"[%s] %ld",s2,n);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
    	case F100:			/* 4, "s1" */
	    if (zsoutl(ZTFILE,s1) < 0) tralog = 0;
	    break;
    	case F101:			/* 5, "s1: n" */
	    if ((int)strlen(s1) + 15 > TBUFL)
	      sprintf(sp,"?String too long");
	    else sprintf(sp,"%s: %ld",s1,n);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
    	case F110:			/* 6, "s1 s2" */
	    x = (int)strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if ((int)strlen(s1) + x + 4 > TBUFL)
	      sprintf(sp,"?String too long");
	    else sprintf(sp,"%s %s",s1,s2);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
    	case F111:			/* 7, "s1 s2: n" */
	    x = (int)strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    if ((int)strlen(s1) + x + 15 > TBUFL)
	      sprintf(sp,"?String too long");
	    else sprintf(sp,"%s %s: %ld",s1,s2,n);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
	    break;
	default:
	    sprintf(sp,"?Invalid format for tlog() - %ld",n);
	    if (zsoutl(ZTFILE,s) < 0) tralog = 0;
    }
}
#endif /* TLOG */

#ifndef MAC
/*
  The rest of this file is for all implementations but the Macintosh.
*/

/*  C H K I N T  --  Check for console interrupts  */

#ifdef CK_CURSES
static int repaint = 0;			/* Transfer display needs repainting */
#endif /* CK_CURSES */

int
chkint() {
    int ch, cn, ofd; long zz;

    if ((!local) || (quiet)) return(0);	/* Only do this if local & not quiet */
#ifdef datageneral
    if (con_reads_mt)                   /* if conint_mt task is active */
        if (conint_avl) {               /* and there's an interrupt pending */
            cn = 1;                     /* process it */
            ch = conint_ch;
            conint_avl = 0;             /* turn off flag so conint_mt can */
        } else                          /* proceed */
            return(0);
    else                                /* if conint_mt not active */
        if ((ch = coninc(2)) < 0)       /* try to get char manually */
            return(0);                  /* I/O error, or no data */
        else                            /* if successful, set cn so we */
            cn = 1;                     /* know we got one */
    debug(F101,"chkint got keyboard character",ch,cn);
#else /* !datageneral */
    cn = conchk();			/* Any input waiting? */
    debug(F101,"conchk","",cn);
    if (cn < 1) return(0);
    if ((ch = coninc(5)) < 0) return(0);
#endif /* datageneral */

    switch (ch & 0177) {
      case 'A': case 'a': case 0001:		/* Status report */
#ifdef COMMENT
	if (fdispla != XYFD_R && fdispla != XYFD_S)
	  return(0);	                        /* Only for serial or simple */
#else
	if (fdispla != XYFD_R && fdispla != XYFD_S && fdispla != XYFD_N)
	  return(0);	                        /* Only for serial, simple */
						/* or none */
	ofd = fdispla; /* [MF] Save file display type */
	if (fdispla == XYFD_N)
	  fdispla = XYFD_R; /* [MF] Pretend serial if no file display */
#endif /* COMMENT */
	screen(SCR_TN,0,0l,"Status report:");
	screen(SCR_TN,0,0l," file type: ");
	if (binary) {
	    switch(binary) {
	      case XYFT_L: screen(SCR_TZ,0,0l,"labeled"); break;
	      case XYFT_I: screen(SCR_TZ,0,0l,"image"); break;
	      case XYFT_U: screen(SCR_TZ,0,0l,"binary undefined"); break;
	      default: 
	      case XYFT_B: screen(SCR_TZ,0,0l,"binary"); break;
	    }
	} else {
#ifdef NOCSETS
	    screen(SCR_TZ,0,0l,"text");
#else
	    screen(SCR_TU,0,0l,"text, ");
	    if (tcharset == TC_TRANSP) {
		screen(SCR_TZ,0,0l,"transparent");
	    } else {
		if (what == W_SEND) {
		    screen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
		    screen(SCR_TU,0,0l," => ");
		    screen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
		} else {
		    screen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
		    screen(SCR_TU,0,0l," => ");
		    screen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
		}
	    }
#endif /* NOCSETS */
	}
	screen(SCR_QE,0,filcnt," file number");
	if (fsize) screen(SCR_QE,0,fsize," size");
	screen(SCR_QE,0,ffc,   " characters so far");
	if (fsize > 0L) {
	    zz = ( (ffc + sendstart) * 100L ) / fsize;
	    screen(SCR_QE,0,zz,      " percent done");
	}
	if (bctu == 4) {		/* Block check */
	    screen(SCR_TU,0,0L," block check: ");
	    screen(SCR_TZ,0,0L,"blank-free-2");
	} else screen(SCR_QE,0,(long)bctu,  " block check");
	screen(SCR_QE,0,(long)rptflg," compression");
	screen(SCR_QE,0,(long)ebqflg," 8th-bit prefixing");
	screen(SCR_QE,0,(long)lscapu," locking shifts");
	if (!network)
	  screen(SCR_QE,0, speed, " speed");
	if (what == W_SEND)

	  screen(SCR_QE,0,(long)spsiz, " packet length");
	else if (what == W_RECV || what == W_REMO)
	  screen(SCR_QE,0,(long)urpsiz," packet length");
	screen(SCR_QE,0,(long)wslots,  " window slots");
	fdispla = ofd; /* [MF] Restore file display type */
	return(0);

      case 'B': case 'b': case 0002:	/* Cancel batch */
      case 'Z': case 'z': case 0032:
	screen(SCR_TN,0,0l,"Cancelling Batch ");
	czseen = 1;
	return(0);

      case 'F': case 'f': case 0006:	/* Cancel file */
      case 'X': case 'x': case 0030:
	screen(SCR_TN,0,0l,"Cancelling File ");
	cxseen = 1;
	return(0);

      case 'R': case 'r': case 0022:	/* Resend packet */
      case 0015: case 0012:
	screen(SCR_TN,0,0l,"Resending packet ");
	numerrs++;
	resend(winlo);
	return(1);

      case 'E': case 'e':		/* Send error packet */
      case 0005:
	return(-1);

#ifdef datageneral
      case '\03':                       /* We're not trapping ^C's with */
        trap(0);                        /* signals, so we check here    */
#endif /* datageneral */

#ifdef CK_CURSES
      case 0014:			/* Ctrl-L to refresh screen */
      case 'L': case 'l':		/* Also accept L (upper, lower) */
      case 0027:			/* Ctrl-W synonym for VMS & Ingres */
	repaint = 1;
	return(0);
#endif /* CK_CURSES */

      default:				/* Anything else, print message */
	intmsg(1L);
	return(0);
    }
}

/*  I N T M S G  --  Issue message about terminal interrupts  */

VOID
#ifdef CK_ANSIC
intmsg(long n)
#else
intmsg(n) long n;
#endif /* CK_ANSIC */
/* intmsg */ {
    char buf[80];

    if (!displa || quiet)		/* Not if we're being quiet */
      return;
    if (server && (!srvdis || n > -1L))	/* Special for server */
      return;
    buf[0] = NUL;			/* Keep compilers happy */
#ifndef OXOS
#ifdef SVORPOSIX
    conchk();				/* Clear out pending escape-signals */
#endif /* SVORPOSIX */
#endif /* ! OXOS */
#ifdef VMS
    conres();				/* So Ctrl-C will work */
#endif /* VMS */
    if ((!server && n == 1L) || (server && n < 0L)) {

#ifdef CK_NEED_SIG
	sprintf(buf,"Type escape character (%s) followed by:",dbchr(escape));
	screen(SCR_TN,0,0l,buf);
#endif /* CK_NEED_SIG */

 screen(SCR_TN,0,0l,"X to cancel file,  CR to resend current packet");
 screen(SCR_TN,0,0l,"Z to cancel group, A for status report");
 screen(SCR_TN,0,0l,"E to send Error packet, Ctrl-C to quit immediately: ");
/* if (server) */ screen(SCR_TN,0,0l,"");
    }
    else screen(SCR_TU,0,0l," ");
}

static int newdpy = 0;			/* New display flag */
static char fbuf[80];			/* Filename buffer */
static char abuf[80];			/* As-name buffer */
static long oldffc = 0L;
static long dots = 0L;
static int hpos = 0;

static VOID				/* Initialize Serial or CTR display */
dpyinit() {
    newdpy = 0;				/*  Don't do this again */
    oldffc = 0L;			/*  Reset this */
    dots = 0L;				/*  and this.. */
    oldcps = cps = 0L ;

    conoll("");						/* New line */
    if (what == W_SEND) conol("Sending: "); 		/* Action */
    else if (what == W_RECV) conol("Receiving: ");
    conol(fbuf);
    if (*abuf) conol(" => "); conoll(abuf); 		/* Names */
    *fbuf = NUL; *abuf = NUL;
    if (fsize > -1L) {					/* Size */
	sprintf(fbuf,"Size: %ld, Type: ",fsize);
	conol(fbuf); *fbuf = NUL;
    } else conol("Size: unknown, Type: ");
    if (binary) {					/* Type */
	switch(binary) {
	      case XYFT_L: conoll("labeled"); break;
	      case XYFT_I: conoll("image"); break;
	      case XYFT_U: conoll("binary undefined"); break;
	      default: 
	      case XYFT_B: conoll("binary"); break;
	}
    } else {
#ifdef NOCSETS
	conoll("text");
#else
	conol("text, ");
	if (tcharset == TC_TRANSP) {
	    conoll("transparent");
	} else {
	    if (what == W_SEND) {
		conol(fcsinfo[fcharset].keyword);
		conol(" => ");
		conoll(tcsinfo[tcharset].keyword);
	    } else {
		conol(tcsinfo[tcharset].keyword);
		conol(" => ");
		conoll(fcsinfo[fcharset].keyword);
	    }
	}
#endif /* NOCSETS */
    }
    if (fdispla == XYFD_S) {		/* CRT field headings */
/*
  Define CK_CPS to show current transfer rate.
  Leave it undefined to show estimated time remaining.
  Estimated-time-remaining code from Andy Fyfe, not tested on
  pathological cases.
*/
#define CK_CPS

#ifdef CK_CPS
	conoll("    File   Percent       Packet");
	conoll("    Bytes  Done     CPS  Length");
#else
	conoll("    File   Percent  Secs Packet");
	conoll("    Bytes  Done     Left Length");
#endif /* CK_CPS */
	newdpy = 0;
    }
    hpos = 0;
}

/*
  showpkt(c)
  c = completion code: 0 means transfer in progress, nonzero means it's done.
  Show the file transfer progress counter and perhaps verbose packet type.
  Original by: Kai Uwe Rommel.
*/
VOID
#ifdef CK_ANSIC
showpkt(char c)
#else
showpkt(c) char c;
#endif /* CK_ANSIC */
/* showpkt */ {

    long howfar;			/* How far into file */

    if (newdpy)				/* Put up filenames, etc, */
      dpyinit();			/* if they're not there already. */

    howfar = ffc;			/* How far */

    if (fdispla == XYFD_S) {		/* CRT display */
	char buffer[40];
	long et;			/* Elapsed time, entire batch  */
	long pd;			/* Percent done, this file     */
	long tp;			/* Transfer rate, entire batch */
	long ps;			/* Packet size, current packet */
	long mytfc;			/* Local copy of byte counter  */

	et = gtimer();			/* Elapsed time  */
	ps = (what == W_RECV) ? rpktl+1 : spktl+1; /* Packet length */
#ifdef CK_RESEND
	if (what == W_SEND)		/* And if we didn't start at */
	  howfar += sendstart;		/*  the beginning... */
	else if (what == W_RECV)
	  howfar += rs_len;
#endif /* CK_RESEND */
	pd = -1;			/* Percent done. */
	if (c == NUL) {			/* Still going, figure % done */
	    if (fsize == 0L) return;	/* Empty file, don't bother */
	    pd = (fsize > 99L) ? (howfar / (fsize / 100L)) : 0L;
	    if (pd > 100) pd = 100;	/* Expansion */
	} else pd = 100;		/* File complete, so 100%. */

#ifndef CK_CPS
/*
  fsecs = time (from gtimer) that this file started (set in sfile()).
  Rate so far is ffc / (et - fsecs),  estimated time for remaining bytes
  is (fsize - ffc) / ( ffc / (et - fsecs )).
*/
	tp = (howfar > 0L) ? (fsize - howfar) * (et - fsecs) / howfar : 0L;
#endif /* CK_CPS */

#ifdef CK_CPS
	mytfc = (pd < 100) ? tfc + howfar : tfc;
	tp = (et > 0) ? mytfc / et : 0; /* Transfer rate */
	if (c && (tp == 0))		/* Watch out for subsecond times */
	  tp = howfar;
#endif /* CK_CPS */
	if (pd > -1L)
	  sprintf(buffer, "%c%9ld%5ld%%%8ld%8ld ", CR, howfar, pd, tp, ps);
	else
	  sprintf(buffer, "%c%9ld      %8ld%8ld ", CR, howfar, tp, ps);
	conol(buffer);
	hpos = 31;
    } else {				/* SERIAL display */
	long i, k;
	if (howfar - oldffc < 1024)	/* Update display every 1K */
	  return;
	oldffc = howfar;		/* Time for new display */
	k = (howfar / 1024L) - dots;	/* How many K so far */
	for (i = 0L; i < k; i++) {
	    if (hpos++ > 77) {		/* Time to wrap? */
		conoll("");
		hpos = 0;
	    }
	    conoc('.');			/* Print a dot for this K */
	    dots++;			/* Count it */
	}
    }
}

/*  S C R E E N  --  Screen display function  */

/*
  screen(f,c,n,s)
    f - argument descriptor
    c - a character or small integer
    n - a long integer
    s - a string.
  Fill in this routine with the appropriate display update for the system.
    FILE DISPLAY SERIAL:     Default, works on any terminal, even hardcopy.
    FILE DISPLAY CRT:        Works on any CRT, writes over current line.
    FILE DISPLAY FULLSCREEN: Requires terminal-dependent screen control.
*/
VOID
#ifdef CK_ANSIC
screen(int f, char c,long n,char *s)
#else
screen(f,c,n,s) int f; char c; long n; char *s;
#endif /* CK_ANSIC */
/* screen */ {
    char buf[80];
    int len;				/* Length of string */
#ifdef UNIX
#ifndef NOJC
    int obg;
_PROTOTYP( VOID conbgt, (int) );

    if (local) {
	obg = backgrd;			/* Previous background status */
	conbgt(1);			/* See if running in background */
	if (!backgrd && obg) {		/* Just came into foreground? */
	    concb((char)escape);	/* Put console back in CBREAK mode */
	    conint(trap,stptrap);	/* Turn interrupts back on. */
	}
    }
#endif /* NOJC */
#endif /* UNIX */

    if ((f != SCR_WM) && (f != SCR_EM)) /* Always update warnings & errors */
      if (!displa || quiet || backgrd || fdispla == XYFD_N ||
	  (server && !srvdis))
	return;

#ifdef CK_CURSES
    if (fdispla == XYFD_C) {		/* If fullscreen display selected */
	screenc(f,c,n,s);		/* call the fullscreen version */
	return;
    }
#endif /* CK_CURSES */

    len = (int)strlen(s);		/* Length of string */

    switch (f) {			/* Handle our function code */

case SCR_FN:    			/* Filename */
#ifdef MAC
    conoll(""); conol(s); conoc(SP); hpos = len + 1;
#else
    strncpy(fbuf,s,80);
    newdpy = 1;				/* New file so refresh display */
#endif /* MAC */
    return;

case SCR_AN:    			/* As-name */
#ifdef MAC
    if (hpos + len > 75) { conoll(""); hpos = 0; }
    conol("=> "); conol(s);
    if ((hpos += (len + 3)) > 78) { conoll(""); hpos = 0; }
#else
    strncpy(abuf,s,80);
#endif /* MAC */
    return;

case SCR_FS: 				/* File-size */
#ifdef MAC
    sprintf(buf,", Size: %ld",n);  conoll(buf);  hpos = 0;
#endif /* MAC */
    return;

case SCR_XD:    			/* X-packet data */
#ifdef MAC
    conoll(""); conoll(s); hpos = 0;
#else
    strncpy(fbuf,s,80);
#endif /* MAC */
    return;

case SCR_ST:      			/* File status */
    switch (c) {
      case ST_OK:   	   		/* Transferred OK */
	showpkt('Z');			/* Update numbers one last time */
	if ((hpos += 5) > 78) conoll(""); /* Wrap screen line if necessary. */
	conoll(" [OK]"); hpos = 0;	/* Print OK message. */
	if (fdispla == XYFD_S) {	/* We didn't show Z packet when */
	    conoc('Z');			/* it came, so show it now. */
	    hpos = 1;
	}
	return;	      

      case ST_DISC: 			/*  Discarded */
	if ((hpos += 12) > 78) conoll("");
	conoll(" [discarded]"); hpos = 0;
	return;

      case ST_INT:       		/*  Interrupted */
	if ((hpos += 14) > 78) conoll("");
	conoll(" [interrupted]"); hpos = 0;
	return;

      case ST_SKIP: 			/*  Skipped */
	if ((hpos += 10) > 78) conoll("");
	conol(" [skipped]"); hpos = 0;
	return;

      case ST_ERR:			/* Error */
	conoll("");
	conol("Error: "); conoll(s); hpos = 0;
	return;

      case ST_REFU:			/* Refused */
	conoll("");
	conol("Refused: "); conoll(s); hpos = 0;
	return;

      case ST_INC:       		/* Incomplete */
	if ((hpos += 12) > 78) conoll("");
	conoll(" [incomplete]"); hpos = 0;
	return;

      default:
	conoll("*** screen() called with bad status ***");
	hpos = 0;
	return;
    }

#ifdef MAC
case SCR_PN:    			/* Packet number */
    sprintf(buf,"%s: %ld",s,n); conol(buf); hpos += (int)strlen(buf); return;
#endif /* MAC */

case SCR_PT:    			/* Packet type or pseudotype */
    if (c == 'Y') return;		/* Don't bother with ACKs */
    if (c == 'D') {			/* In data transfer phase, */
	showpkt(NUL);			/* show progress. */
	return;
    }
#ifndef AMIGA
    if (hpos++ > 77) {			/* If near right margin, */
	conoll("");			/* Start new line */
	hpos = 0;			/* and reset counter. */
    }
#endif /* AMIGA */
    if (c == 'Z' && fdispla == XYFD_S)
      return;
    else
      conoc(c);				/* Display the packet type. */
#ifdef AMIGA
    if (c == 'G') conoll("");           /* New line after G packets */
#endif /* AMIGA */
    return;

case SCR_TC:    			/* Transaction complete */
    conoc(BEL); conoll(""); return;

case SCR_EM:				/* Error message */
    conoll(""); conoc('?'); conoll(s); hpos = 0; return;

case SCR_WM:				/* Warning message */
    conoll(""); conoll(s); hpos = 0; return;

case SCR_TU:				/* Undelimited text */
    if ((hpos += len) > 77) { conoll(""); hpos = len; }
    conol(s); return;

case SCR_TN:				/* Text delimited at beginning */
    conoll(""); conol(s); hpos = len; return;

case SCR_TZ:				/* Text delimited at end */
    if ((hpos += len) > 77) { conoll(""); hpos = len; }
    conoll(s); return;

case SCR_QE:				/* Quantity equals */
    sprintf(buf,"%s: %ld",s,n);
    conoll(buf); hpos = 0; return;

case SCR_CW:				/* Close fullscreen window */
    return;				/* No window to close */

case SCR_CD:
    return;

default:
    conoll("*** screen() called with bad object ***");
    hpos = 0;
    return;
    }
}

/*  E R M S G  --  Nonfatal error message  */

/* Should be used only for printing the message text from an Error packet. */

VOID
ermsg(msg) char *msg; {			/* Print error message */
    debug(F110,"ermsg",msg,0);
    if (local)
      screen(SCR_EM,0,0L,msg);
    tlog(F110,"Protocol Error:",msg,0L);
}

VOID
doclean() {				/* General cleanup upon exit */
#ifndef NOICP
#ifndef NOSPL
    extern struct mtab *mactab;		/* For ON_EXIT macro. */
    extern int nmac;
#endif /* NOSPL */
#endif /* NOICP */

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
#ifdef TLOG
    if (tralog) {
	tlog(F100,"Transaction Log Closed","",0L);
	*trafil = '\0';
	tralog = 0;
	zclose(ZTFILE);
    }
#endif /* TLOG */

#ifndef NOICP
#ifndef NOSPL
    zclose(ZRFILE);			/* READ and WRITE files, if any. */
    zclose(ZWFILE);
#ifndef NODIAL
    if (dialfd) fclose(dialfd);		/* Dial directory, if any. */
#endif /* NODIAL */
/*
  If a macro named "on_exit" is defined, execute it.  Also remove it from the
  macro table, in case its definition includes an EXIT or QUIT command, which
  would cause much recursion and would prevent the program from ever actually
  EXITing.
*/
    if (nmac) {				/* Any macros defined? */
	int k;				/* Yes */
	k = mlook(mactab,"on_exit",nmac); /* Look up "on_exit" */
	if (k >= 0) {			/* If found, */
	    *(mactab[k].kwd) = NUL;	/* poke its name from the table, */
	    if (dodo(k,"") > -1)	/* set it up, */
	      parser(1);		/* and execute it */
        }
    }
#endif /* NOSPL */
#endif /* NOICP */

/*
  Put console terminal back to normal.  This is done here because the
  ON_EXIT macro calls the parser, which meddles with console terminal modes.
*/
    ttclos(0);				/* Close external line, if any */
    if (local) {
	strcpy(ttname,dftty);		/* Restore default tty */
	local = dfloc;			/* And default remote/local status */
    }
    conres();				/* Restore console terminal. */

#ifdef COMMENT
/* Should be no need for this, and maybe it's screwing things up? */
    connoi();				/* Turn off console interrupt traps */
#endif /* COMMENT */

    syscleanup();			/* System-dependent cleanup, last */
}

/*  D O E X I T  --  Exit from the program.  */

/*
  First arg is general, system-independent symbol: GOOD_EXIT or BAD_EXIT.
  If second arg is -1, take 1st arg literally.
  If second arg is not -1, work it into the exit code.
*/
VOID
doexit(exitstat,what) int exitstat, what; {
#ifdef VMS
    char envstr[64];
    static $DESCRIPTOR(symnam,"CKERMIT_STATUS");
    static struct dsc$descriptor_s symval;
    int i;
#endif /* VMS */

    debug(F101,"doexit exitstat","",exitstat);
    debug(F101,"doexit what","",what);

    doclean();				/* First, clean up most things */

#ifdef VMS
    if (what == -1)
      what = 0;				/* Since we set two different items */
    sprintf(envstr,"%d", exitstat | what);
    symval.dsc$w_length = (int)strlen(envstr);
    symval.dsc$a_pointer = envstr;
    symval.dsc$b_class = DSC$K_CLASS_S;
    symval.dsc$b_dtype = DSC$K_DTYPE_T;
    i = 2;				/* Store in global table */
    LIB$SET_SYMBOL(&symnam, &symval, &i);
    if (exitstat == BAD_EXIT)
      exitstat = SS$_ABORT | STS$M_INHIB_MSG;
    if (exitstat == GOOD_EXIT)
      exitstat = SS$_NORMAL | STS$M_INHIB_MSG;
#else /* Not VMS */
    if (what != -1)			/* Take 1st arg literally */
      exitstat |= what;
#endif /* VMS */

/* We have put this off till the very last moment... */

#ifdef DEBUG
    if (deblog) {			/* Close the debug log. */
	debug(F101,"C-Kermit EXIT status","",exitstat);
	*debfil = '\0';
	deblog = 0;
	zclose(ZDFILE);
    }
#endif /* DEBUG */

      exit(exitstat);			/* Exit from C-Kermit */
}

/* Set up interrupts */

VOID
setint() {
    conint(trap,stptrap);       /* Turn on console terminal interrupts. */
    bgchk();	    	        /* Check background status */
}

VOID
bgchk() {				/* Check background status */
    if (bgset < 0)
      pflag = !backgrd;			/* Set prompt flag */
    else				/* based on foreground/background */
      pflag = (bgset == 0 ? 1 : 0);

    /* Message flag on only if at top level, pflag is on, and QUIET is OFF */

    if (
#ifndef NOSPL
	cmdlvl == 0
#else
	tlevel < 0
#endif /* NOSPL */
	)
      msgflg = (pflag == 0) ? 0 : !quiet;
    else msgflg = 0;
}

#ifdef DEBUG
/*  D E B U G  --  Enter a record in the debugging log  */

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
        If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Long int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.
*/

/*
  WARNING: Don't change DEBUFL without changing sprintf() formats below,
  accordingly.
*/
#define DBUFL 2300
static char *dbptr = (char *)0;
static int dbufl = DBUFL;

int
#ifdef CK_ANSIC
dodebug(int f, char *s1, char *s2, long n)
#else
dodebug(f,s1,s2,n) int f; char *s1, *s2; long n;
#endif /* CK_ANSIC */
/* dodebug */ {
    char *sp;
    int x;

    if (!deblog) return(0);		/* If no debug log, don't. */
    if (!dbptr) {			/* Allocate memory buffer */
	dbptr = malloc(DBUFL+1);	/* This only happens once */
	dbufl = DBUFL;			/* Unless ... */
	if (!dbptr) {
	    deblog = 0;
	    return(0);
	}
    }
    if (!s1) s1="(NULL)";
    if (!s2) s2="(NULL)";

#ifdef COMMENT
/*
  This should work, but it doesn't.
  So instead we'll cope with overflow via sprintf formats.
  N.B.: UNFORTUNATELY, this means we have to put constants in the
  sprintf formats.
*/
    if (!f || (f & 6)) {		/* String argument(s) included? */
	x = (int) strlen(s1) + (int) strlen(s2) + 18;
	if (x > dbufl) {		/* Longer than buffer? */
	    if (dbptr)			/* Yes, free previous buffer */
	      free(dbptr);
	    dbptr = (char *) malloc(x + 2); /* Allocate a new one */
	    if (!dbptr) {
		zsoutl(ZDFILE,"DEBUG: Memory allocation failure");
		zclose(ZDFILE);
		deblog = 0;
		return(0);
	    } else {
		dbufl = x;
		sprintf(dbptr,"DEBUG: Buffer expanded to %d\n", x + 18);
		zsoutl(ZDFILE,dbptr);
	    }
	}
    }
#endif /* COMMENT */

#ifdef COMMENT
/* The aforementioned sprintf() formats were like this: */
	if (n > 31 && n < 127)
	  sprintf(sp,"%.100s%.2000s:%c\n",s1,s2,(CHAR) n);
	else if (n < 32 || n == 127)
	  sprintf(sp,"%.100s%.2000s:^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
	else if (n > 127 && n < 160)
	  sprintf(sp,"%.100s%.2000s:~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
	else if (n > 159 && n < 256)
	  sprintf(sp,"%.100s%.2000s:~%c\n",s1,s2,(CHAR) (n & 0x7F));
	else sprintf(sp,"%.100s%.2000s:%ld\n",s1,s2,n);
/*
  But, naturally, it turns out these are not portable either, so now
  we do the stupidest possible thing.
*/
#endif /* COMMENT */

    if ((x = (int) strlen(s1)) > 100) s1 = "(string too long)";
    if ((int) strlen(s2) + 101 >= DBUFL) s2 = "(string too long)";

    sp = dbptr;

    switch (f) {		/* Write log record according to format. */
      case F000:		/* 0 = print both strings, and n as a char. */
	if (n > 31 && n < 127)
	  sprintf(sp,"%s%s:%c\n",s1,s2,(CHAR) n);
	else if (n < 32 || n == 127)
	  sprintf(sp,"%s%s:^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
	else if (n > 127 && n < 160)
	  sprintf(sp,"%s%s:~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
	else if (n > 159 && n < 256)
	  sprintf(sp,"%s%s:~%c\n",s1,s2,(CHAR) (n & 0x7F));
	else sprintf(sp,"%s%s:%ld\n",s1,s2,n);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      case F001:			/* 1, "=n" */
	sprintf(sp,"=%ld\n",n);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      case F010:			/* 2, "[s2]" */
	sprintf(sp,"[%s]\n",s2);
	if (zsout(ZDFILE,"") < 0) deblog = 0;
	break;
      case F011:			/* 3, "[s2]=n" */
	sprintf(sp,"[%s]=%ld\n",s2,n);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      case F100:			/* 4, "s1" */
	if (zsoutl(ZDFILE,s1) < 0) deblog = 0;
	break;
      case F101:			/* 5, "s1=n" */
	sprintf(sp,"%s=%ld\n",s1,n);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      case F110:			/* 6, "s1[s2]" */
	sprintf(sp,"%s[%s]\n",s1,s2);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      case F111:			/* 7, "s1[s2]=n" */
	sprintf(sp,"%s[%s]=%ld\n",s1,s2,n);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
	break;
      default:
	sprintf(sp,"\n?Invalid format for debug() - %d\n",f);
	if (zsout(ZDFILE,dbptr) < 0) deblog = 0;
    }
    return(0);
}
#endif /* DEBUG */

#ifdef CK_CURSES

/*
  There are three different ways to do fullscreen on VMS.
  1. Use the real curses library, VAXCCURSE.
  2. Use do-it-yourself code.
  3. Use the Screen Manager, SMG$.

  Method 1 doesn't work quite right; you can't call endwin(), so once you've
  started curses mode, you can never leave.

  Method 2 doesn't optimize the screen, and so much more time is spent in
  screen writes.  This actually causes file transfers to fail because the
  tty device input buffer can be overrun while the screen is being updated,
  especially on a slow MicroVAX that has small typeahead buffers.

  In the following #ifdef block, #define one of them and #undef the other 2.

  So now let's try method 3...
*/
#ifdef VMS
#define CK_SMG				/* Screen Manager */
#undef MYCURSES				/* Do-it-yourself */
#undef VMSCURSE				/* VAXCCURSE library */
#endif /* VMS */

/*  S C R E E N C  --  Screen display function, uses curses  */

/* Idea for curses display contributed by Chris Pratt of APV Baker, UK */

/* Avoid conficts with curses.h */

#ifdef QNX
/* Same as ckcasc.h, but in a different radix... */
#ifdef ESC
#undef ESC
#endif /* ESC */
#endif /* QNX */

#ifndef MYCURSES
#undef VOID				/* This was defined in ckcdeb.h */
#endif /* MYCURSES */

#undef BS				/* These were defined in ckcasc.h */
#undef CR
#undef NL
#undef SO
#undef US

#ifdef VMS				/* VMS fullscreen display */
#ifdef MYCURSES				/* Do-it-yourself method */
extern int isvt52;			/* From CKVTIO.C */
#define printw printf
#else
#ifdef VMSCURSE				/* VMS curses library VAXCCURSE */
#include <curses.h> 
/* Note: Screen manager doesn't need a header file */
#endif /* VMSCURSE */
#endif /* MYCURSES */
#else					/* Not VMS */
#ifdef MYCURSES				/* Do-it-yourself method */
#define isvt52 0			/* Used by OS/2, VT-100/ANSI always */
#define printw printf
#else
#include <curses.h>			/* So use real curses */
#endif /* MYCURSES */
#endif /* VMS */

#ifdef CK_SMG
/*
  Long section for Screen Manager starts here...
  By William Bader.
*/
#include "ckvvms.h"
/* #include <smgdef.h> */
/* #include <smgmsg.h> */

extern unsigned int vms_status;	    /* Used for system service return status */

static long smg_pasteboard_id = -1;	/* pasteboard identifier */
static long smg_display_id = -1;	/* display identifier */
static int smg_open = 0;		/* flag if smg current open */

#define	clrtoeol()	SMG$ERASE_LINE(&smg_display_id, 0, 0)

#define clear()		SMG$ERASE_DISPLAY(&smg_display_id, 0, 0, 0, 0)

#define	touchwin(scr)	SMG$REPAINT_SCREEN(&smg_pasteboard_id)

#define clearok(curscr,ok)		/* Let wrefresh() do the work */

#define wrefresh(cursrc) touchwin(scr)

static void
move(row, col) int row, col; {
    /* Change from 0-based for curses to 1-based for SMG */
    ++row; ++col;
    CHECK_ERR("move: smg$set_cursor_abs",
	      SMG$SET_CURSOR_ABS(&smg_display_id, &row, &col));
}

static void
refresh() {
    CHECK_ERR("refresh: smg$end_pasteboard_update",
	      SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));
    CHECK_ERR("refresh: smg$begin_pasteboard_update",
	      SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
}

#ifdef VMS_V40
#define	OLD_VMS
#endif /* VMS_V40 */
#ifdef VMS_V42
#define	OLD_VMS
#endif /* VMS_V42 */
#ifdef VMS_V44
#define	OLD_VMS
#endif /* VMS_V44 */

static int
initscr() {
    int rows = 24, cols = 80;
    int row = 1, col = 1;

    if (smg_pasteboard_id == -1) { 	/* Open the screen */
#ifdef OLD_VMS
	CHECK_ERR("initscr: smg$create_pasteboard",
		  SMG$CREATE_PASTEBOARD(&smg_pasteboard_id, 0, 0, 0, 0));
#else
	/* For VMS V5, not tested */
	CHECK_ERR("initscr: smg$create_pasteboard",
		  SMG$CREATE_PASTEBOARD(&smg_pasteboard_id, 0, 0, 0, 0, 0));
#endif /* OLD_VMS */
    }

    if (smg_display_id == -1) {		/* Create a display window */

	CHECK_ERR("initscr: smg$create_virtual_display",
		  SMG$CREATE_VIRTUAL_DISPLAY(&rows, &cols, &smg_display_id,
					     0, 0, 0));

	/* Connect the display window to the screen */
	CHECK_ERR("initscr: smg$paste_virtual_display",
		  SMG$PASTE_VIRTUAL_DISPLAY(&smg_display_id,&smg_pasteboard_id,
					    &row,&col));
    }

    if (!smg_open) {			/* Start a batch update */
	smg_open = 1;
	CHECK_ERR("initscr: smg$begin_pasteboard_update",
		  SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
    }
    return(1);
}

static void
endwin() {
    if (!smg_open)
      return;

    smg_open = 0;

    CHECK_ERR("endwin: smg$end_pasteboard_update",
	      SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));

    move(22, 0);

#ifdef COMMENT
/*
  These calls clear the screen.
*/
    CHECK_ERR("endwin: smg$delete_virtual_display",
	      SMG$DELETE_VIRTUAL_DISPLAY(&smg_display_id));
    smg_display_id = -1;

    CHECK_ERR("endwin: smg$delete_pasteboard",
	      SMG$DELETE_PASTEBOARD(&smg_pasteboard_id, 0));
    smg_pasteboard_id = -1;
#endif /* COMMENT */
}

static void printw(str, a1, a2, a3, a4, a5, a6, a7, a8)
char *str;
long a1, a2, a3, a4, a5, a6, a7, a8;
/* printw */ {
    char buf[255];
    $DESCRIPTOR(text_dsc, buf);

    text_dsc.dsc$w_length = sprintf(buf, str, a1, a2, a3, a4, a5, a6, a7, a8);
    CHECK_ERR("printw: smg$put_chars",
	      SMG$PUT_CHARS(&smg_display_id, &text_dsc, 0, 0, 0, 0, 0));
}
#endif /* CK_SMG */

#ifdef MYCURSES
/*
  Do-it-yourself curses implementation for VMS, OS/2 and other ANSI/VT-100's.
  Supports only the VT52 and VT1xx (and later VT2xx/3xx/4xx) terminals.
  By Terry Kennedy, St Peters College.
 
  First, some stuff we can just ignore:
*/

int
touchwin(x) int x; {
}
int
initscr() {
}
int
refresh() {
}
int
endwin() {
}

/*
 * Now, some stuff we need to do:
 */

_PROTOTYP( int move, (int, int) );

int
move(row, col) int row, col; {
    if (isvt52)
      printf("\033Y%c%c", row + 037, col + 037);
    else
      printf("\033[%d;%dH", row + 1, col + 1);
}

int
clear() {
    move(1,1);
    if (isvt52)
      printf("\033J");
    else
      printf("\033[J");
}

int
clrtoeol() {
    if (isvt52)
      printf("\033K");
    else
      printf("\033[K");
}
#endif /* MYCURSES */

/* Screen line numbers */

#define CW_BAN  0			/* Curses Window Banner */
#define CW_DIR  2			/* Current directory */
#define CW_LIN  3			/* Communication device */
#define CW_SPD  4			/* Communication speed */
#define CW_PAR  5			/* Parity */
#define CW_NAM  7			/* Filename */
#define CW_TYP  8			/* File type */
#define CW_SIZ  9			/* File size */
#define CW_PCD 10			/* Percent done */

#ifndef CK_PCT_BAR
#define CW_TR  11			/* Time remaining */
#define CW_CP  12			/* Characters per second */
#define CW_WS  13			/* Window slots */
#define CW_PT  14			/* Packet type */
#define CW_PC  15			/* Packet count */
#define CW_PL  16			/* Packet length */
#define CW_PR  17			/* Packet retry */
#ifdef COMMENT
#define CW_PB  17			/* Packet block check */
#endif /* COMMENT */
#else /* CK_PCT_BAR */
#define CW_BAR 11       /* Percent Bar Scale */
#define CW_TR  12			/* Time remaining */
#define CW_CP  13			/* Chars per sec */
#define CW_WS  14			/* Window slots */
#define CW_PT  15			/* Packet type */
#define CW_PC  16			/* Packet count */
#define CW_PL  17			/* Packet length */
#define CW_PR  18			/* Packet retry */
#ifdef COMMENT
#define CW_PB  18			/* Packet block check */
#endif /* COMMENT */
#endif /* CK_PCT_BAR */

#define CW_ERR 19			/* Error message */
#define CW_MSG 20			/* Info message */
#define CW_INT 22			/* Instructions */

static int cinit = 0;			/* Flag for curses init'd */
static int cendw = 0;			/* endwin() was called */

static
#ifdef CK_ANSIC				/* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */

#ifdef CK_ANSIC				/* Update % transfered and % bar */
updpct( long old, long new)
#else /* CK_ANSIC */
updpct(old, new) long old, new;
#endif /* CK_ANSIC */
/* updpct */ {
#ifdef COMMENT
    int m, n;
    move(CW_PCD,22);
    printw("%ld", new);
#ifdef CK_PCT_BAR
    if (thermometer) {
#ifndef OS2
	if (old > new) old = 0;
	m = old/2;
	move(CW_PCD, 26 + m);
	n = new / 2 - m;
	while (n > 0) {
	    if (( m + 1) % 5 == 0)
	      printw("*");
	    else
	      printw("=");
	    m++;
	    n--;
	}
	if (new % 2 != 0) printw("-");
	/* move(CW_PCD, 22+53); */
#else /* OS2 */
	if (old > new) old = 0 ;
	m = old / 2;
	n = new/2-m;
	move(CW_PCD, 26+m);
	while (n > 0) {
	    printw("%c", '\333');
	    m++; n--;
	}
	if (new % 2 != 0)
	  printw("%c", '\261');
#endif /* OS2 */
    }
#endif /* CK_PCT_BAR */
    /* clrtoeol(); */
#else  /* !COMMENT */
#ifdef OS2
#define CHAR1	'\333'		/* OS2 */
#define CHAR2	'\261'
#else
#define CHAR1	'/'		/* Default */
#define CHAR2	'-'
#endif /* OS2 */
    move(CW_PCD,22);
    printw("%ld", new);
#ifdef CK_PCT_BAR
    if (thermometer) {
        int m, n;

	if (old > new) old = 0 ;
	if (new <= 100L) {
	    m = old / 2;
	    n = new / 2 - m;
	    move(CW_PCD, 26+m);
	    while (n-- > 0)
	      printw("%c", CHAR1);
	    if (new % 2 != 0)
	      printw("%c", CHAR2);
	}
    }
#endif /* CK_PCT_BAR */
#endif /* COMMENT */
}

static long gtv = -1L, oldgtv;

static long
shocps(pct) int pct; {
    static long oldffc = 0L;
#ifndef CPS_WEIGHTED
    long secs;
#endif /* CPS_WEIGHTED */

    /* gtv starts at -1 but we want to ignore it until it is at least 1 */ 

    oldgtv = (gtv >= 0) ? gtv : 0;   
    gtv = gtimer();

#ifdef CPS_WEIGHTED
    if (gtv != oldgtv) {		/* The first packet is ignored */
	if (ffc < oldffc)
	  oldffc = ffc;
	oldcps = cps;
	if (oldcps && oldgtv > 1) {	/* The first second is ignored */
/*
  This version of shocps() produces a weighted average that some
  people like, but most people find it disconcerting and bombard us
  with questions and complaints about why the CPS figure fluctuates so
  wildly.  So now you only get the weighted average if you build the
  program yourself with CPS_WEIGHTED defined.
*/
#ifndef CPS_VINCE
/* Here is Jeff's weighting scheme, current = 25%, history = 75%.. */
	    cps = ( (oldcps * 3) + (ffc - oldffc) / (gtv-oldgtv) ) / 4;
#else
/* And an alternate weighting scheme from Vincent Fatica... */
	    cps = (3 *
	     ((1+pct/300)*oldffc/oldgtv+(1-pct/100)*(ffc-oldffc)/(gtv-oldgtv)))
	      / 4;
#endif /* CPS_VINCE */
	} else {
	    /* No weighted average since there is nothing to weigh */
	    cps = gtv ? (ffc - oldffc) / (gtv - oldgtv) : (ffc - oldffc) ;
	}
	if (deblog) {
	    debug(F101,"SHOCPS: pct   ","",pct);
	    debug(F101,"SHOCPS: gtv   ","",gtv);
	    debug(F101,"SHOCPS: oldgtv","",oldgtv);
	    debug(F101,"SHOCPS: dgtv  ","",gtv-oldgtv);
	    debug(F101,"SHOCPS: ffc   ","",ffc);
	    debug(F101,"SHOCPS: oldffc","",oldffc);
	    debug(F101,"SHOCPS: dffc  ","",ffc-oldffc);
	    debug(F101,"SHOCPS: cps   ","",cps);
	}
	move(CW_CP,22);
	printw("%ld", cps);
	clrtoeol();
	oldffc = ffc;
    }
#else /* !CPS_WEIGHTED */
    if (gtv != oldgtv) {		/* The first packet is ignored */
	if ((secs = gtv - fsecs) > 0) {
	    cps = (secs < 1L) ? ffc : ffc / secs;
	    move(CW_CP,22);
	    printw("%ld", cps);
	    clrtoeol();
	}
    }
#endif /* CPS_WEIGHTED */
    return(cps);
}

static
long
#ifdef CK_ANSIC
shoetl(long old_tr, long cps, long fsiz, long howfar )
#else
shoetl(old_tr, cps, fsiz, howfar) long old_tr, cps, fsiz, howfar;
#endif /* CK_ANSIC */
/* shoetl */ {
    /* Show estimated time left in transfer */
    long tr ;

    tr = (fsiz > 0L && cps > 0L) ? ( ( fsiz - howfar ) / cps ) : -1L ;
    if (tr > -1L && tr != old_tr) {
	move(CW_TR,22);
	printw("%s",hhmmss(tr));
	clrtoeol();
    }
    return(tr);
}

static
#ifdef CK_ANSIC				/* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */
scrft() {				/* Display file type */
    move(CW_TYP,22);
    if (binary) {
	switch(binary) {
	      case XYFT_L: printw("LABELED"); break;
	      case XYFT_I: printw("IMAGE"); break;
	      case XYFT_U: printw("BINARY UNDEFINED"); break;
	      default: 
	      case XYFT_B: printw("BINARY"); break;
	}
#ifdef CK_RESEND
	if (what == W_SEND && sendstart > 0L) {
	    if (sendmode == SM_PSEND)
	      printw(" / partial");
	    else if (sendmode == SM_RESEND)
	      printw(" / resend");
	} else if (what == W_RECV && rs_len > 0L)
	  printw(" / resend");
#endif /* CK_RESEND */
    } else {
	printw("TEXT");
#ifndef NOCSETS
	if (tcharset == TC_TRANSP) {
	    printw(" (no translation)");
	} else {
	    if (what == W_SEND)
	      printw(" (%s => %s)",
		     fcsinfo[fcharset].keyword,
		     tcsinfo[tcharset].keyword);
	    else printw(" (%s => %s)",
		     tcsinfo[tcharset].keyword,
		     fcsinfo[fcharset].keyword);
	}
#endif /* NOCSETS */
    }
    clrtoeol();
    return;
}

char *					/* Convert seconds to hh:mm:ss */
#ifdef CK_ANSIC
hhmmss(long x)
#else
hhmmss(x) long x;
#endif /* CK_ANSIC */
/* hhmmss(x) */ {
    static char buf[10];
    long s, h, m;
    h = x / 3600L;			/* Hours */
    x = x % 3600L;
    m = x / 60L;			/* Minutes */
    s = x % 60L;			/* Seconds */
    if (x > -1L)
      sprintf(buf,"%02ld:%02ld:%02ld",h,m,s);
    else buf[0] = NUL;
    return((char *)buf);
}

#ifdef CK_NEWTERM
static FILE *ck_stdout = NULL;
static int ck_fd = -1;
#endif /* CK_NEWTERM */

static long pct = 100, oldpct = 0;	/* Percent done */
static int oldtyp = 0, oldwin = -1, oldtry = -1, oldlen = -1;

#ifdef NETCONN
static char *netname[] = {
    "none", "TCP/IP", "TCP/IP", "X.25", "DECnet", "VAX PSI", "Named Pipes",
    "X.25", "NetBIOS"
};
#endif /* NETCONN */

#ifdef CK_ANSIC
void
screenc(int f, char c,long n,char *s)
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
screenc(f,c,n,s)
int f;		/* argument descriptor */
char c;		/* a character or small integer */
long n;		/* a long integer */
char *s;	/* a string */
#endif /* CK_ANSIC */
/* screenc() */ {

    static int q = 0;
    static long fsiz = -1L;		/* Copy of file size */
    static long fcnt = 0L;		/* Number of files transferred */
    static long fbyt = 0L;    /* Total file bytes of all files transferred */
    static long old_tr = -1L; /* Time remaining */
    static long howfar = 0L;  /* How much of current file has been transfer */
    long cps = 0L;

    int len;				/* Length of string */
    int x;				/* Worker */

    debug(F101,"screenc cinit","",cinit);
    debug(F101,"screenc cendw","",cendw);

    if (cinit == 0 || cendw > 0) {	/* Handle borderline cases... */
	if (f == SCR_CW) {		/* Close window, but it's not open */
	    ft_win = 0;
	    return;
	}
	if (f == SCR_EM ||
	   (f == SCR_PT && c == 'E')) {	/* Fatal error before window open */
	    conoll(""); conoc('?'); conoll(s); return; /* Regular display */
	}
    }
    if (cinit == 0) {			/* Only call initscr() once */
	cendw = 1;			/* New window needs repainting */
#ifdef COMMENT
	if (!initscr()) {		/* Oops, can't initialize window? */
/*
  In fact, this doesn't happen.  "man curses" says initscr() halts the
  entire program if it fails, which is true on the systems where I've
  tested it.  It will fail if your terminal type is not known to it.
  That's why SET FILE DISPLAY FULLSCREEN calls tgetent() to make sure the
  terminal type is known before allowing a curses display.
*/
	    fprintf(stderr,"CURSES INITSCR ERROR\r\n");
	    fdispla = XYFD_R;		/* Go back to regular display */
	    return;
	} else {
	    cinit++;			/* Window initialized ok */
	    debug(F100,"CURSES INITSCR OK","",0);
	}
#else					/* Save some memory. */
#ifdef CK_NEWTERM
	/* (From Andy Fyfe <andy@vlsi.cs.caltech.edu>)
	   System V curses seems to reserve the right to alter the buffering
	   on the output FILE* without restoring it.  Fortunately System V
	   curses provides newterm(), an alternative to initscr(), that
	   allows us to specify explicitly the terminal type and input and
	   output FILE pointers.  Thus we duplicate stdout, and let curses
	   have the copy.  The original remains unaltered.  Unfortunately,
	   newterm() seems to be particular to System V.
	*/
	s = getenv("TERM");
	if (ck_fd < 0) {
	    ck_fd = dup(fileno(stdout));
	    ck_stdout = (ck_fd >= 0) ? fdopen(ck_fd, "w") : NULL;
	}
	debug(F100,"screenc newterm...","",0);
	if (ck_stdout == NULL || newterm(s, ck_stdout, stdin) == 0) {
	    fprintf(stderr,
	      "Fullscreen display not supported for terminal type: %s\r\n",s);
	    fdispla = XYFD_R;		/* Go back to regular display */
	    return;
	}
	debug(F100,"screenc newterm ok","",0);
#else
	debug(F100,"screen calling initscr","",0);
	initscr();			/* Initialize curses. */
	debug(F100,"screen initscr ok","",0);
#endif /* CK_NEWTERM */
	cinit++;			/* Remember curses was initialized. */
#endif /* COMMENT */
    }
    ft_win = 1;				/* Window is open */
    if (repaint) {
#ifdef CK_WREFRESH 
/*
  This totally repaints the screen, just what we want, but we can only
  do this with real curses, and then only if clearok() and wrefresh() are
  provided in the curses library.
*/
#ifdef OS2
 	restorescreen(&commandscreen);
#else
#ifdef QNX
	clearok(stdscr, 1);		/* QNX doesn't have curscr */
	wrefresh(stdscr);
#else
	wrefresh(curscr);
#endif /* QNX */
#endif /* OS2 */
#else  /* No CK_WREFRESH */
/*
  Kermit's do-it-yourself method, works with all types of fullscreen
  support, but does not repaint all the fields.  For example, the filename
  is lost, because it arrives at a certain time and never comes again, and
  Kermit presently does not save it anywhere.  Making this method work for
  all fields would be a rather major recoding task, and would add a lot of
  complexity and storage space.
*/
	cendw = 1;
#endif /* CK_WREFRESH */
	repaint = 0;
    }
    if (cendw) {			/* endwin() was called previously */
#ifdef VMS
	initscr();			/* (or should have been!) */
	clear();
	touchwin(stdscr);
	refresh();
#else
#ifdef QNX
/*
  In QNX, if we don't call initscr() here we core dump.
  I don't have any QNX curses documentation, but other curses manuals
  say that initscr() should be called only once per application, and
  experience shows that on other systems, calling initscr() here generally
  results in a core dump.
*/
	debug(F100,"screenc re-calling initscr QNX","",0);
	initscr();
	clear();
	refresh();
#ifdef COMMENT
/*
  But even so, second and subsequent curses displays are messed up.
  Calling touchwin, refresh, etc, doesn't make any difference.
*/
	debug(F100,"screenc calling touchwin QNX","",0);
	touchwin(stdscr);
	debug(F100,"screenc calling refresh QNX","",0);
	refresh();
#endif /* COMMENT */

#else /* All others... */
	debug(F100,"screenc calling clear","",0);
	clear();
	debug(F100,"screenc clear ok","",0);
#endif /* QNX */
#endif /* VMS */
	debug(F100,"screenc setup ok","",0);
	debug(F100,"screenc doing first move","",0);
	move(CW_BAN,0);			/* Display the banner */
	if (*myhost) debug(F111,"screenc myhost",myhost,myhost);
	else debug(F111,"screenc myhost","",myhost);
	/* if (!myhost) myhost = ""; */
	if (*myhost) printw("%s, %s",versio,(char *)myhost);
	else printw("%s",versio);
	move(CW_DIR,3);  printw("Current Directory: %s",zgtdir());
	if (network) {
	    move(CW_LIN,8); printw("Network Host: %s",ttname);
	} else {
	    move(CW_LIN,0);  printw("Communication Device: %s",ttname);
	}
	if (network) {
	    move(CW_SPD,8);
	    printw("Network Type: ");
	} else {
	    move(CW_SPD,1);
	    printw("Communication Speed: ");
	}
	move(CW_SPD,22);		/* Speed */
	if (network) {
#ifdef NETCONN
	    printw("%s",netname[nettype]);
#else
	    printw("(network)");
#endif /* NETCONN */
	} else {
	    if (speed < 0L) speed = ttgspd();
	    if (speed > 0L) {
		if (speed == 8880) printw("75/1200");
		else printw("%ld",speed);
	    } else printw("unknown");
	}
	move(CW_PAR,14); printw("Parity: %s",parnam((char)parity));
	move(CW_TYP,11); printw("File Type:");
	move(CW_SIZ,11); printw("File Size:");
	move(CW_PCD, 8); printw("Percent Done:");
#ifdef CK_PCT_BAR
	if (thermometer) {
	    oldpct = pct = 0 ;
	    move(CW_BAR,22);
	    printw("    ...10...20...30...40...50...60...70...80...90..100");
	    move(CW_BAR,22+56);
	}
#endif /* CK_PCT_BAR */
	move(CW_TR,  1); printw("Estimated Time Left:");
	move(CW_CP,  2); printw("Transfer Rate, CPS:");
	move(CW_WS,  8); printw("Window Slots:");
	move(CW_PT,  9); printw("Packet Type:");
	move(CW_PC,  8); printw("Packet Count:");
	move(CW_PL,  7); printw("Packet Length:");
	move(CW_PR,  2); printw("Packet Retry Count:");
#ifdef COMMENT
	move(CW_PB,  2); printw("Packet Block Check:");
#endif /* COMMENT */
	move(CW_ERR,10); printw("Last Error:");
	move(CW_MSG, 8); printw("Last Message:");

#ifdef CK_NEED_SIG
	move(CW_INT, 0);
	printw(
"<%s>X to cancel file, <%s>Z to cancel group, <%s><CR> to resend last packet",
	       dbchr(escape), dbchr(escape), dbchr(escape)
	       );
	move(CW_INT + 1, 0);
	printw(
"<%s>E to send Error packet, ^C to quit immediately, <%s>L to refresh screen.",
	       dbchr(escape), dbchr(escape)
	       );
#else /* !CK_NEED_SIG */
	move(CW_INT, 0);
#ifdef OS2
	printw(
"X to cancel file, Z to cancel group, <Enter> to resend last packet,"
	       );
#else /* !OS2 */
	printw(
"X to cancel file, Z to cancel group, <CR> to resend last packet,"
	       );
#endif /* OS2 */
	move(CW_INT + 1, 0);
	printw(
#ifdef VMS
"E to send Error packet, ^C to quit immediately, ^W to refresh screen."
#else
"E to send Error packet, ^C to quit immediately, ^L to refresh screen."
#endif /* VMS */
	       );
#endif /* CK_NEED_SIG */
	refresh();
	cendw = 0;
    }
    len = strlen(s);			/* Length of argument string */

    debug(F101,"SCREENC switch","",f);	/* Handle our function code */
    switch (f) {
      case SCR_FN:    			/* Filename */
	fsiz = -1L;			/* Invalidate previous file size */
	move(CW_PCD,22);		/* Erase percent done from last time */
	clrtoeol();
	move(CW_SIZ,22);		/* Erase file size from last time */
	clrtoeol();
	move(CW_ERR,22);		/* And last error message */
	clrtoeol();
	if (what == W_SEND) {		/* If we're sending... */
#ifdef CK_RESEND
	    switch (sendmode) {
	      case SM_RESEND:
		move(CW_NAM,11);
		printw("Resending:");
		break;
	      default:
		move(CW_NAM,13);
		printw("Sending:");
		break;
	    }
#else
	    move(CW_NAM,13);
	    printw("Sending:");
#endif /* CK_RESEND */

	} else if (what == W_RECV) {	/* If we're receiving... */
	    move(CW_NAM,11);
	    printw("Receiving:");
	} else {			/* If we don't know... */
	    move(CW_NAM,11);		/* (should never see this) */
	    printw("File Name:");
	}
	move(CW_NAM,22);		/* Display the filename */
	if (len > 57) {
	    printw("%.55s..",s);
	    len = 57;
	} else printw("%s",s);
	q = len;			/* Remember name length for later */
	clrtoeol();
	scrft();			/* Display file type (can change) */
	refresh();
#ifdef OS2
	savescreen(&commandscreen, 0, 0);
#endif /* OS2 */
	return;

      case SCR_AN:    			/* File as-name */
	if (q + len + 4 < 58) {		/* Will fit */
	    move(CW_NAM, 22 + q);
	    printw(" => %s",s);
	} else {			/* Too long */
	    move(CW_NAM, 22);		/* Overwrite previous name */
	    q = 0;
	    if (len + 4 > 57) {					/* wg15 */
		printw(" => %.51s..",s);			/* wg15 */
		len = 53;					/* wg15 */
	    } else printw(" => %s",s);				/* wg15 */
	}
	q += len + 4;			/* Remember horizontal position */
	clrtoeol();
	refresh();
#ifdef OS2
	savescreen(&commandscreen, 0, 0);
#endif /* OS2 */
	return;

      case SCR_FS: 			/* File size */
	fsiz = n;
	move(CW_SIZ,22);
	if (fsiz > -1L) printw("%ld",n);
	clrtoeol();
	scrft();			/* File type */
	refresh();
#ifdef OS2
	savescreen(&commandscreen, 0, 0);
#endif /* OS2 */
	return;

      case SCR_PT:    			/* Packet type or pseudotype */
	if (spackets < 5) {
	    /* Things that won't change after the 4th packet */
	    move(CW_PAR,22); printw("%s",parnam((char)parity)); clrtoeol();
	    clrtoeol();
#ifdef COMMENT
	    move(CW_PB, 22);		/* Block check on this packet */
	    if (bctu == 4) printw("B"); else printw("%d",bctu);
	    clrtoeol();
#endif /* COMMENT */
	}

	x = (what == W_RECV) ?		/* Packet length */
	  rpktl+1 :
	    spktl ;
	if (x != oldlen) {		/* But only if it changed. */
	    move(CW_PL, 22);
	    printw("%d",x);
	    clrtoeol();
	    oldlen = x;
	}
	move(CW_PC, 22);		/* Packet count (always). */
	printw("%d",spackets);		/* WARNING: this can slow us way */
	clrtoeol();			/* down with short packets. */

	if (wcur != oldwin) {		/* Window slots, if changed. */
	    move(CW_WS, 22);
	    printw("%d of %d",wcur,wslotr);
	    clrtoeol();
	    oldwin = wcur;
	}
	if (retrans != oldtry) {	/* Retry count, if changed */
	    move(CW_PR, 22);
	    printw("%d",retrans);
	    clrtoeol();
	    oldtry = retrans;
	}
	if (c != oldtyp && c != 'Y' && c != 'N') { /* Sender's packet type */
	    move(CW_PT,22);
	    printw("%c",c);
	    clrtoeol();
	    oldtyp = c;
	}
	switch (c) {			/* Now handle specific packet types */
	  case 'S':			/* Beginning of transfer */
	    fcnt = fbyt = 0L;		/* Clear counters */
	    gtv = -1L;			/* And old/new things... */
	    oldpct = pct = 0L;
	    break;
	  case 'D':			/* Data packet */
	    if (fsiz > 0L) {		/* Show percent done if known */
		long rpct;
		oldpct = pct;		/* Remember previous percent */
		howfar = ffc;
#ifdef CK_RESEND
		if (what == W_SEND)	/* Account for PSEND or RESEND */
		  howfar += sendstart;
		else if (what == W_RECV)
		  howfar += rs_len;
#endif /* CK_RESEND */
		/* Percent done, to be displayed... */
		pct = (fsiz > 99L) ? (howfar / (fsiz / 100L)) : 0L;
		if (pct > 100L ||	/* Allow for expansion and */
		   (oldpct == 99L && pct < 0L)) /* other boundary conditions */
		  pct = 100L;
		if (pct != oldpct)	/* Only do this 100 times per file */
		  updpct(oldpct, pct);
	    } /* Fall thru... */

	  case '%':			/* Timeouts, retransmissions */
	    cps = shocps((int) pct);
	    old_tr = shoetl( old_tr, cps, fsiz, howfar ) ;
	    break;

	  case 'E':			/* Error packet */
#ifdef COMMENT
	    move(CW_ERR,22);		/* Print its data field */
	    if (*s) printw("%s",s);
	    clrtoeol();
#endif /* COMMENT */
	    fcnt = fbyt = 0L;		/* So no bytes for this file */
	    break;
	  case 'Q':			/* Crunched packet */
	    cps = shocps((int) pct);
	    old_tr = shoetl( old_tr, cps, fsiz, howfar ) ;
	    move(CW_ERR,22);
	    printw("Damaged Packet");
	    clrtoeol();
	    break;
	  case 'T':			/* Timeout */
	    cps = shocps((int) pct);
	    old_tr = shoetl( old_tr, cps, fsiz, howfar ) ;
	    move(CW_ERR,22);
	    printw("Timeout");
	    clrtoeol();
	    break;
	  default:			/* Others, do nothing */
	    break;
	}
	refresh();
#ifdef OS2
	savescreen( &commandscreen, 0, 0 ) ;
#endif /* OS2 */
	return;

      case SCR_ST:			/* File transfer status */
#ifdef COMMENT
	move(CW_PCD,22);		/* Update percent done */
	if (c == ST_OK) {		/* OK, print 100 % */
	    updpct(oldpct,100);
	    pct = 100;
	    oldpct = 0;
	} else if (fsiz > 0L)		/* Not OK, update final percent */
/*
  The else part writes all over the screen -- howfar and/or fsiz have
  been reset as a consequence of the not-OKness of the transfer.
*/
	  updpct(oldpct, (howfar * 100L) / fsiz);

	clrtoeol();
#else
	if (c == ST_OK) {		/* OK, print 100 % */
	    move(CW_PCD,22);		/* Update percent done */
	    updpct(oldpct,100);
	    pct = 100;
	    oldpct = 0;
	    clrtoeol();
	}
#endif /* COMMENT */

	move(CW_MSG,22);		/* Remove any previous message */
	clrtoeol(); refresh();
	move(CW_TR, 22);
	clrtoeol(); refresh();

	switch (c) {			/* Print new status message */
	  case ST_OK:			/* Transfer OK */
	    fcnt++;			/* Count this file */
	    fbyt += ffc;		/* Count its bytes */
	    move(CW_MSG,22);
	    printw("Transfer OK");	/* Say Transfer was OK */
	    clrtoeol(); refresh();
	    return;

	  case ST_DISC:			/* Discarded */
	    move(CW_ERR,22); printw("File discarded");
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  case ST_INT:       		/* Interrupted */
	    move(CW_ERR,22); printw("Transfer interrupted");
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  case ST_SKIP:			/* Skipped */
	    move(CW_ERR,22); printw("File skipped");
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  case ST_ERR:			/* Error message */
	    move(CW_ERR,22); printw("%s",s);
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  case ST_REFU:			/* Refused */
	    move(CW_ERR,22);
	    if (*s)
	      printw("Refused, %s",s);
	    else printw("Refused");
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  case ST_INC:
	    move(CW_ERR,22); printw("Incomplete");
	    pct = oldpct = 0;
	    clrtoeol(); refresh();
	    return;

	  default:			/* Bad call */
	    move(CW_ERR,22); printw("*** screen() called with bad status ***");
	    clrtoeol(); refresh(); return;
	}

      case SCR_TC: {   			/* Transaction complete */	
	  long ecps;
	  int eff = -1;
	  ecps = (tsecs > 0) ? ((fbyt * 10L) / (long) tsecs) / 10L : fbyt;
	  move(CW_CP,22);			/* Overall transfer rate */
	  printw("%ld", ecps);
	  clrtoeol();
	  move(CW_MSG,22);		/* Print statistics in message line */
	  if (speed > 99L && speed != 8880L && network == 0)
	    eff = (((ecps * 100L) / (speed / 100L)) + 5L) / 10L;
	  printw("Files: %ld, Total Bytes: %ld",fcnt,fbyt);
	  if (eff > -1) printw(", Efficiency: %d%%", eff);
	  clrtoeol();
	  move(CW_TR, 1);
	  printw("       Elapsed Time: %s",hhmmss((long)tsecs));
	  clrtoeol();
	  move(23,0); clrtoeol();	/* Clear instructions lines */
	  move(22,0); clrtoeol();	/* to make room for prompt. */
	  refresh();
#ifndef VMSCURSE
	  endwin();
#ifdef SOLARIS
	  conres();
#endif /* SOLARIS */
#endif /* VMSCURSE */
	  pct = 100; oldpct = 0;	/* Reset these for next time. */
	  oldtyp = 0; oldwin = -1; oldtry = -1; oldlen = -1;
	  cendw = 1; conoc(BEL);	/* Close window, then beep. */
	  ft_win = 0;			/* Window closed. */
	  return;
      }
      case SCR_EM:			/* Error packet (fatal) */
	move (CW_ERR,22);
	printw("? %s",s);
	conoc(BEL);
	pct = oldpct = 0;
	clrtoeol(); refresh(); return;

      case SCR_QE:			/* Quantity equals */
      case SCR_TU:			/* Undelimited text */
      case SCR_TN:			/* Text delimited at start */
      case SCR_TZ:			/* Text delimited at end */
	return;				/* (ignored in fullscreen display) */

      case SCR_XD:    			/* X-packet data */
	move(CW_NAM,22);
	printw("%s",s);
	clrtoeol(); refresh(); return;

      case SCR_CW:			/* Close Window */
	clrtoeol(); move(23,0); clrtoeol(); move(22,0);	clrtoeol();
	refresh();
	pct = 100; oldpct = 0;		/* Reset these for next time. */
	oldtyp = 0; oldwin = -1; oldtry = -1; oldlen = -1;

#ifndef VMSCURSE
	endwin();
#endif /* VMSCURSE */
	ft_win = 0;			/* Flag that window is closed. */
	cendw = 1; return;

      case SCR_CD:			/* Display current directory */
	move(CW_DIR,22); printw(s);
	clrtoeol();
	refresh();
#ifdef OS2
	savescreen(&commandscreen, 0, 0);
#endif /* OS2 */
	return;	

      default:				/* Bad call */
	move (CW_ERR,22);
	printw("*** screen() called with bad function code ***");
	clrtoeol(); refresh(); return;
    }
}
#endif /* CK_CURSES */

#endif /* MAC */
