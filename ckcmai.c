char *versio = "C-Kermit, 4F(095) 31 Aug 89";

/*  C K C M A I  --  C-Kermit Main program  */

/*
 Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 Columbia University Center for Computing Activities.
 First released January 1985.
 Copyright (C) 1985, 1989, Trustees of Columbia University in the City of New 
 York.  Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
*/
/*
 The Kermit file transfer protocol was developed at the Columbia University
 Center for Computing Activities (CUCCA).  It is named after Kermit the Frog,
 star of the television series THE MUPPET SHOW; the name is used by permission
 of Henson Associates, Inc.  "Kermit" is also Celtic for "free".
*/
/*
 Thanks to Herm Fischer of Encino CA for extensive contributions to version
 4.0, and to the following people for their contributions over the years:

   Chris Adie, Edinburgh U, Scotland (OS/2 support)
   Larry Afrin, Clemson U
   Barry Archer, U of Missouri
   Robert Andersson, Oslo, Norway
   Fuat Baran, CUCCA
   Stan Barber, Rice U
   Charles Brooks, EDN
   Mike Brown, Purdue U
   Mark Buda, DEC
   Bill Catchings, formerly of CUCCA
   Bob Cattani, Columbia U CS Dept
   Howard Chu, U of Michigan
   Bill Coalson, McDonnell Douglas
   Alan Crosswell, CUCCA
   Jeff Damens, formerly of CUCCA
   Mark Davies, Bath U, UK
   Joe R. Doupnik, Utah State U
   John R. Evans, IRS, Kansas City
   Glenn Everhart, RCA Labs
   Carl Fongheiser, CWRU
   Christine Gianone, CUCCA
   John Gilmore, UC Berkeley
   Yekta Gursel, MIT
   Jim Guyton, Rand Corp
   Marion Kananson, hakanson@cs.orst.edu
   Stan Hanks, Rice U.
   Michael Haertel
   Ken Harrenstein, SRI
   Chuck Hedrick, Rutgers U
   Ron Heiby, Motorola Micromputer Division
   Steve Hemminger, Tektronix
   Randy Huntziger, National Library of Medicine
   Phil Julian, SAS Institute
   Howie Kaye, CUCCA
   Jim Knutson, U of Texas at Austin
   Bo Kullmar, Kista, Sweden
   John Kunze, UC Berkeley
   Bob Larson, USC (OS-9 support)
   David Lawyer, UC Irvine
   S.O. Lidie, Lehigh U
   David MacKenzie, Environmental Defense Fund, Rockefeller U
   Martin Maclaren, Bath U, UK
   Chris Maio, Columbia U CS Dept
   Peter Mauzey, AT&T
   Leslie Mikesall, American Farm Bureau
   Martin Minow, DEC
   Ray Moody, Purdue U
   Tony Movshon, NYU
   Dan Murphy, ???
   Jim Noble, Planning Research Corporation
   Ian O'Brien, Bath U, UK
   Paul Placeway, Ohio State U
   Ken Poulton, HP Labs
   Frank Prindle, NADC
   Anton Rang, ???
   Scott Ribe, ???
   Jack Rouse, SAS Institute
   Stew Rubenstein, Harvard U
   Dan Schullman, DEC
   Gordon Scott, Micro Focus, Newbury UK
   David Sizeland, U of London Medical School
   Bradley Smith, UCLA
   Andy Tanenbaum, Vrije U, Amsterdam, Netherlands
   Markku Toijala, Helsinki U of Technology
   Dave Tweten, AMES-NAS
   Walter Underwood, Ford Aerospace
   Pieter Van Der Linden, Centre Mondial, Paris
   Ge van Geldorp, Netherlands
   Wayne Van Pelt, GE/CRD
   Mark Vasoll & Gregg Wonderly, Oklahoma State U
   Paul Vixie (DEC)
   Stephen Walton, Calif State U, Northridge
   Lauren Weinstein
   Joachim Wiesel, U of Karlsruhe
   Michael Williams, UCLA
   Patrick Wolfe, Kuck & Associates, Inc.
   Farrell Woods, Concurrent (formerly Masscomp)
   Dave Woolley, CAP Communication Systems, London
   John Zeeff, Ann Arbor, MI
*/

#include "ckcsym.h"			/* Macintosh once needed this */
#include "ckcker.h"			/* Kermit symbols */
#include "ckcdeb.h"			/* Debug & other symbols */

/* Text message definitions.. each should be 256 chars long, or less. */
#ifdef MAC
char *hlptxt = "\r\
MacKermit Server Commands:\r\
\r\
    BYE\r\
    FINISH\r\
    GET filespec\r\
    REMOTE CWD directory\r\
    REMOTE HELP\r\
    SEND filespec\r\
\r\0";
#else
#ifdef AMIGA
char *hlptxt = "C-Kermit Server Commands:\n\
\n\
GET filespec, SEND filespec, FINISH, BYE, REMOTE HELP\n\
\n\0";
#else
#ifdef OS2
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
\n\
GET files  REMOTE CWD [dir]    REMOTE DIRECTORY [files]\n\
SEND files REMOTE SPACE [dir]  REMOTE HOST command\n\
FINISH     REMOTE DELETE files REMOTE TYPE files\n\
BYE        REMOTE HELP\n\
\n\0";
#else
char *hlptxt = "C-Kermit Server REMOTE Commands:\n\
\n\
GET files  REMOTE CWD [dir]    REMOTE DIRECTORY [files]\n\
SEND files REMOTE SPACE [dir]  REMOTE HOST command\n\
MAIL files REMOTE DELETE files REMOTE WHO [user]\n\
BYE        REMOTE PRINT files  REMOTE TYPE files\n\
FINISH     REMOTE HELP\n\
\n\0";
#endif
#endif
#endif

#ifdef OSK
char *srvtxt = "\r\l\
C-Kermit server starting.  Return to your local machine by typing\r\l\
its escape sequence for closing the connection, and issue further\r\l\
commands from there.  To shut down the C-Kermit server, issue the\r\l\
FINISH or BYE command and then reconnect.\r\l\
\l\0";
#else
char *srvtxt = "\r\n\
C-Kermit server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the C-Kermit server, issue the\r\n\
FINISH or BYE command and then reconnect.\n\
\r\n\0";
#endif

/* Declarations for Send-Init Parameters */

int spsiz = DSPSIZ,                     /* curent packet size to send */
    spmax = DSPSIZ,		/* (PWP) Biggest packet size we can send */
                                /* (see rcalcpsz()) */
    spsizf = 0,                         /* Flag to override what you ask for */
    rpsiz = DRPSIZ,                     /* Biggest we want to receive */
    urpsiz = DRPSIZ,			/* User-requested rpsiz */
    maxrps = MAXRP,			/* Maximum incoming long packet size */
    maxsps = MAXSP,			/* Maximum outbound l.p. size */
    maxtry = MAXTRY,			/* Maximum retries per packet */
    wslots = 1,				/* Window size */
    timint = DMYTIM,                    /* Timeout interval I use */
    srvtim = DSRVTIM,			/* Server command wait timeout */
    rtimo = URTIME,                     /* Timeout I want you to use */
    timef = 0,                          /* Flag to override what you ask */
    npad = MYPADN,                      /* How much padding to send */
    mypadn = MYPADN,                    /* How much padding to ask for */
    bctr = 1,                           /* Block check type requested */
    bctu = 1,                           /* Block check type used */
    ebq =  MYEBQ,                       /* 8th bit prefix */
    ebqflg = 0,                         /* 8th-bit quoting flag */
    rqf = -1,				/* Flag used in 8bq negotiation */
    rq = 0,				/* Received 8bq bid */
    sq = 'Y',				/* Sent 8bq bid */
    rpt = 0,                            /* Repeat count */
    rptq = MYRPTQ,                      /* Repeat prefix */
    rptflg = 0;                         /* Repeat processing flag */

int capas = 10,				/* Position of Capabilities */
    atcapb = 8,				/* Attribute capability */
    atcapr = 1,				/*  requested */
    atcapu = 0,				/*  used */
    swcapb = 4,				/* Sliding Window capability */
    swcapr = 0,				/*  requested */
    swcapu = 0,				/*  used */
    lpcapb = 2,				/* Long Packet capability */
    lpcapr = 1,				/*  requested */
    lpcapu = 0;				/*  used */

CHAR padch = MYPADC,                    /* Padding character to send */
    mypadc = MYPADC,                    /* Padding character to ask for */
    seol = MYEOL,                       /* End-Of-Line character to send */
    eol = MYEOL,                        /* End-Of-Line character to look for */
    ctlq = CTLQ,                        /* Control prefix in incoming data */
    myctlq = CTLQ;                      /* Outbound control character prefix */

struct zattr iattr;			/* Incoming file attributes */

/* Packet-related variables */

int pktnum = 0,                         /* Current packet number */
    prvpkt = -1,                        /* Previous packet number */
    sndtyp,                             /* Type of packet just sent */
    rsn,				/* Received packet sequence number */
    rln,				/* Received packet length */
    size,                               /* Current size of output pkt data */
    osize,                              /* Previous output packet data size */
    maxsize,                            /* Max size for building data field */
    spktl = 0;				/* Length packet being sent */

CHAR sndpkt[MAXSP+100],                 /* Entire packet being sent */
    recpkt[MAXRP+200],                  /* Packet most recently received */
    *rdatap,				/* Pointer to received packet data */
    data[MAXSP+4],			/* Packet data buffer */
    srvcmd[MAXRP+4],                    /* Where to decode server command */
    *srvptr,                            /* Pointer to above */
    mystch = SOH,                       /* Outbound packet-start character */
    stchr = SOH;                        /* Incoming packet-start character */

/* File-related variables */

#ifdef datageneral
CHAR filnam[256];                       /* Name of current file. */
#else
CHAR filnam[50];                        /* Name of current file. */
#endif

int nfils;                              /* Number of files in file group */
long fsize;                             /* Size of current file */

/* Communication line variables */

CHAR ttname[50];                        /* Name of communication line. */

int parity,                             /* Parity specified, 0,'e','o',etc */
    flow,                               /* Flow control, 1 = xon/xoff */
    speed = -1,                         /* Line speed */
    turn = 0,                           /* Line turnaround handshake flag */
    turnch = XON,                       /* Line turnaround character */
    duplex = 0,                         /* Duplex, full by default */
#ifdef OS2
    escape = 035,                       /* Escape character for connect */
#else
    escape = 034,                       /* Escape character for connect */
#endif
    delay = DDELAY,                     /* Initial delay before sending */
    mdmtyp = 0;                         /* Modem type (initially none)  */

    int tlevel = -1;			/* Take-file command level */

/* Statistics variables */

long filcnt,                    /* Number of files in transaction */
    flci,                       /* Characters from line, current file */
    flco,                       /* Chars to line, current file  */
    tlci,                       /* Chars from line in transaction */
    tlco,                       /* Chars to line in transaction */
    ffc,                        /* Chars to/from current file */
    tfc;                        /* Chars to/from files in transaction */

int tsecs;                      /* Seconds for transaction */

/* Flags */

int deblog = 0,                         /* Flag for debug logging */
    pktlog = 0,                         /* Flag for packet logging */
    seslog = 0,                         /* Session logging */
    tralog = 0,                         /* Transaction logging */
    displa = 0,                         /* File transfer display on/off */
    stdouf = 0,                         /* Flag for output to stdout */
    xflg   = 0,                         /* Flag for X instead of F packet */
    hcflg  = 0,                         /* Doing Host command */
    fncnv  = 1,                         /* Flag for file name conversion */
    binary = 0,                         /* Flag for binary file */
    savmod = 0,                         /* Saved file mode (whole session) */
    bsave  = 0,				/* Saved file mode (per file) */
    bsavef = 0,				/* Flag if bsave was used. */
    cmask  = 0177,			/* Connect byte mask */
    fmask  = 0377,			/* File byte mask */
    warn   = 0,                         /* Flag for file warning */
    quiet  = 0,                         /* Be quiet during file transfer */
    local  = 0,                         /* Flag for external tty vs stdout */
    server = 0,                         /* Flag for being a server */
    cnflg  = 0,                         /* Connect after transaction */
    cxseen = 0,                         /* Flag for cancelling a file */
    czseen = 0,                         /* Flag for cancelling file group */
    keep = 0,                           /* Keep incomplete files */
    nakstate = 0;			/* In a state where we can send NAKs */

/* Variables passed from command parser to protocol module */

char parser();                          /* The parser itself */
char sstate  = 0;                       /* Starting state for automaton */
char *cmarg  = "";                      /* Pointer to command data */
char *cmarg2 = "";                      /* Pointer to 2nd command data */
char **cmlist;                          /* Pointer to file list in argv */

/* Miscellaneous */

char **xargv;                           /* Global copies of argv */
int  xargc;                             /* and argc  */

extern char *dftty;                     /* Default tty name from ckx???.c */
extern int dfloc;                       /* Default location: remote/local */
extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

/* (PWP) buffered input and output buffers; see ckcfns.c getpkt(). */
CHAR zinbuffer[INBUFSIZE], zoutbuffer[INBUFSIZE];
CHAR *zinptr, *zoutptr;
int zincnt, zoutcnt;


/*  M A I N  --  C-Kermit main program  */

#ifdef aegis
/* On the Apollo, intercept main to insert a cleanup handler */
ckcmai(argc,argv) int argc; char **argv; {
#else
main(argc,argv) int argc; char **argv; {
#endif

    char *strcpy();

/* Do some initialization */

    xargc = argc;                       /* Make global copies of argc */
    xargv = argv;                       /* ...and argv. */
    sstate = 0;                         /* No default start state. */
    if (sysinit() < 0) doexit(BAD_EXIT); /* System-dependent initialization. */
    strcpy(ttname,dftty);               /* Set up default tty name. */
    local = dfloc;                      /* And whether it's local or remote. */
    parity = dfprty;                    /* Set initial parity, */
    flow = dfflow;                      /* and flow control. */

/*** attempt to take ini file before doing command line ***/

    cmdini();				/* Sets tlevel */
    while (tlevel > -1) {		/* Execute init file. */
	sstate = parser();		/* Loop getting commands. */
        if (sstate) proto();            /* Enter protocol if requested. */
    }

/* Look for a UNIX-style command line... */

    if (argc > 1) {                     /* Command line arguments? */
        sstate = cmdlin();              /* Yes, parse. */
        if (sstate) {
            proto();                    /* Take any requested action, then */
            if (!quiet) conoll("");     /* put cursor back at left margin, */
            if (cnflg) conect();        /* connect if requested, */
            doexit(GOOD_EXIT);          /* and then exit with status 0. */
        }
    }

#ifdef OS2
    if (speed==-1) speed = ttspeed();	/* Unless explicitly changed,
    					   use the current line speed */
#endif

/* If no action requested on command line, enter interactive parser */

    herald();				/* Display program herald. */
    while(1) {				/* Loop getting commands. */
	sstate = parser();
        if (sstate) proto();            /* Enter protocol if requested. */
    }
}
