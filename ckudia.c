#include "ckcsym.h"
#ifndef NOLOCAL
#ifndef NODIAL
#ifndef NOICP

char *dialv = "Dial Command, 6.0.091, 6 Sep 96";

#ifndef NOOLDMODEMS        /* Unless instructed otherwise, */
#define OLDMODEMS          /* keep support for old modems. */
#endif /* NOOLDMODEMS */

#ifndef M_OLD		   /* Hide old modem keywords in SET MODEM table. */
#define M_OLD 0            /* Define as to CM_INV to make them invisible. */
#endif /* M_OLD */ 

/*  C K U D I A	 --  Module for automatic modem dialing. */

/*
  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/

/*
  Authors:

  Original (version 1, 1985) author: Herm Fischer, Encino, CA.
  Contributed to Columbia University in 1985 for inclusion in C-Kermit 4.0.
  Author and maintainer since 1985: Frank da Cruz, Columbia University,
  fdc@columbia.edu.

  Contributions by many others throughout the years, including: Jeffrey
  Altman, Mark Berryman, Fernando Cabral, John Chmielewski, Joe Doupnik,
  Richard Hill, Larry Jacobs, Eric Jones, Tom Kloos, Bob Larson, Peter Mauzey,
  Joe Orost, Kevin O'Gorman, Kai Uwe Rommel, Dan Schullman, Warren Tucker, and
  others too numerous to list here (but see acknowledgements in ckcmai.c).
*/

/*
  Entry points:
    ckdial(char * number)   Dial a number or answer a call
    dialhup()               Hang up a dialed connection
    mdmhup()                Use modem commands to hang up

  All other routines are static.
  Don't call dialhup() or mdmhup() without first calling ckdial().
*/

/*
  This module calls externally defined system-dependent functions for
  communications i/o, as described in CKCPLM.DOC, the C-Kermit Program Logic
  Manual, and thus should be portable to all systems that implement those
  functions, and where alarm() and signal() work as they do in UNIX.

  TO ADD SUPPORT FOR ANOTHER MODEM, do the following, all in this module:

  1. Define a modem-type number symbol (n_XXX) for it, the next highest one.

  2. Adjust MAX_MDM to the new number of modem types.

  3. Create a MDMINF structure for it.  NOTE: The wake_str should include
     all invariant setup info, e.g. enable result codes, BREAK transparency,
     modulation negotiation, etc.  See ckcker.h for MDMINF struct definition.

  4. Add the address of the MDMINF structure to the modemp[] array,
     according to the numerical value of the modem-type number.

  5. Add the user-visible (SET MODEM) name and corresponding modem number
     to the mdmtab[] array, in alphabetical order by modem-name string.

  6. Read through the code and add any modem-specific sections as necessary.
     For most modern Hayes-compatible modems, no specific code will be
     needed.

  NOTE: The MINIDIAL symbol is used to build this module to include support
  for only a minimum number of standard and/or generally useful modem types,
  namely Hayes, CCITT V.25bis, "Unknown", and None.  When adding support for
  a new modem type, keep it outside of the MINIDIAL sections.
*/

#include "ckcdeb.h"
#ifndef MAC
#include <signal.h>
#endif /* MAC */
#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"
#include "ckuusr.h"

#ifdef OS2ONLY
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#include "ckocon.h"
#endif /* OS2ONLY */

#ifdef NT
#include "cknwin.h"
#endif /* NT */
#ifdef OS2
#include "ckowin.h"
#endif /* OS2 */

#ifndef ZILOG
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#else
#include <setret.h>
#endif /* ZILOG */

#include "ckcsig.h"        /* C-Kermit signal processing */

#ifdef MAC
#define signal msignal
#define SIGTYP long
#define alarm malarm
#define SIG_IGN 0
#define SIGALRM 1
#define SIGINT  2
SIGTYP (*msignal(int type, SIGTYP (*func)(int)))(int);
#endif /* MAC */

#ifdef AMIGA
#define signal asignal
#define alarm aalarm
#define SIGALRM (_NUMSIG+1)
#define SIGTYP void
SIGTYP (*asignal(int type, SIGTYP (*func)(int)))(int);
unsigned aalarm(unsigned);
#endif /* AMIGA */

#ifdef STRATUS
/*
  VOS doesn't have alarm(), but it does have some things we can work with.
  However, we have to catch all the signals in one place to do this, so
  we intercept the signal() routine and call it from our own replacement.
*/
#define signal vsignal
#define alarm valarm
SIGTYP (*vsignal(int type, SIGTYP (*func)(int)))(int);
int valarm(int interval);
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#ifdef getchar
#undef getchar
#endif /* getchar */
#define getchar(x) coninc(0)
#endif /* STRATUS */

#ifdef OS2
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#endif /* OS2 */

#ifdef BIGBUFOK /* Only for verions that are not tight on memory */

char * dialmsg[] = {			/* DIAL status strings */

    /* Keyed to numbers defined in ckcker.h -- keep in sync! */

    "DIAL succeeded",			    /*  0 */
    "Modem type not specified",		    /*  1 */
    "Communication device not specified",   /*  2 */
    "Communication device can't be opened", /*  3 */
    "Speed not specified",		    /*  4 */
    "Pre-DIAL hangup failed",		    /*  5 */
    "Internal error",			    /*  6 */
    "Device input/output error",	    /*  7 */
    "DIAL TIMEOUT expired",		    /*  8 */
    "Interrupted by user",		    /*  9 */
    "Modem not ready",			    /* 10 */
    "Partial dial OK",			    /* 11 */
    "Dial directory lookup error",	    /* 12 */
    NULL,				    /* 13 */
    NULL,				    /* 14 */
    NULL,				    /* 15 */
    NULL,				    /* 16 */
    NULL,				    /* 17 */
    NULL,				    /* 18 */
    NULL,				    /* 19 */
    "Modem command error",		    /* 20 */
    "Failure to initialize modem",	    /* 21 */
    "Phone busy",			    /* 22 */
    "No carrier",			    /* 23 */
    "No dialtone",			    /* 24 */
    "Incoming call",			    /* 25 */
    "No answer",			    /* 26 */
    "Disconnected",			    /* 27 */
    "Answered by voice",		    /* 28 */
    "Access denied / forbidden call",	    /* 29 */
    "Blacklisted",			    /* 30 */
    "Delayed",    			    /* 31 */
    "Fax connection",			    /* 32 */
    NULL				    /* 33 */
};
#endif /* BIGBUFOK */

#ifndef NOSPL
char modemmsg[80];			/* DIAL response from modem */
#endif /* NOSPL */

#ifdef NTSIG
extern int TlsIndex;
#endif /* NTSIG */

int					/* SET DIAL parameters */
  dialhng = 1,				/* DIAL HANGUP, default is ON */
  dialdpy = 0,				/* DIAL DISPLAY, default is OFF */
  mdmspd  = 0,				/* DIAL SPEED-MATCHING (0 = OFF) */
  dialtmo = 0,				/* DIAL TIMEOUT */
  dialatmo = -1,			/* ANSWER TIMEOUT */
  dialksp = 0,				/* DIAL KERMIT-SPOOF, 0 = OFF */
#ifdef NOMDMHUP
  dialmhu = 0;				/* DIAL MODEM-HANGUP, 0 = OFF */
#else
  dialmhu = 1;				/* DIAL MODEM-HANGUP */
#endif /* NOMDMHUP */

int
  dialec = 0,				/* DIAL ERROR-CORRECTION */
  dialdc = 0,				/* DIAL COMPRESSION  */
  dialfc = FLO_AUTO,			/* DIAL FLOW-CONTROL */
  dialmth = XYDM_D,			/* DIAL METHOD */
  dialesc = 0;				/* DIAL ESCAPE */

int telephony = 0;			/* Command-line '-T' option */

long dialmax = 0L,			/* Modem's max interface speed */
  dialcapas  = 0L;			/* Modem's capabilities */

int dialsta = DIA_UNK;			/* Detailed return code (ckuusr.h) */

int is_rockwell = 0;
int is_hayeshispd = 0;

char *dialdir[MAXDDIR];			/* DIAL DIRECTORY filename array */
int   ndialdir = 0;			/* How many dial directories */
char *dialini = NULL;			/* DIAL INIT-STRING, default none */
char *dialmstr = NULL;			/* DIAL MODE-STRING, default none */
char *dialmprmt = NULL;			/* DIAL MODE-PROMPT, default none */
char *dialcmd = NULL;			/* DIAL DIAL-COMMAND, default none */
char *dialnpr = NULL;			/* DIAL PREFIX, ditto */
char *diallac = NULL;			/* DIAL LOCAL-AREA-CODE, ditto */
char *diallcc = NULL;			/* DIAL LOCAL-COUNTRY-CODE, ditto */
char *dialixp = NULL;			/* DIAL INTL-PREFIX */
char *dialixs = NULL;			/* DIAL INTL-SUFFIX */
char *dialldp = NULL;			/* DIAL LD-PREFIX */
char *diallds = NULL;			/* DIAL LD-SUFFIX */
char *dialpxx = NULL;			/* DIAL PBX-EXCHANGE */
char *dialpxi = NULL;			/* DIAL INTERNAL-PREFIX */
char *dialpxo = NULL;			/* DIAL OUTSIDE-PREFIX */
char *dialsfx = NULL;			/* DIAL SUFFIX */
char *dialtfp = NULL;			/* DIAL TOLL-FREE-PREFIX */
extern char * d_name;
extern char * dialtfc[];
extern int ntollfree;
char *dialname  = NULL;			/* Descriptive name for modem */
char *dialdcon  = NULL;			/* DC ON command */
char *dialdcoff = NULL;			/* DC OFF command */
char *dialecon  = NULL;			/* EC ON command */
char *dialecoff = NULL;			/* EC OFF command */
char *dialaaon  = NULL;			/* Autoanswer ON command */
char *dialaaoff = NULL;			/* Autoanswer OFF command */
char *dialhcmd  = NULL;			/* Hangup command */
char *dialhwfc  = NULL;			/* Hardware flow control command */
char *dialswfc  = NULL;			/* (Local) software f.c. command */
char *dialnofc  = NULL;			/* No (Local) flow control command */
char *dialtone  = NULL;			/* Command to force tone dialing */
char *dialpulse = NULL;			/*  ..to force pulse dialing */
char *mdmname   = NULL;

#ifndef MINIDIAL
/*
  Telebit model codes:

  ATI  Model Numbers           Examples
  ---  -------------           --------
  123                          Telebit in "total Hayes-1200" emulation mode
  960                          Telebit in Conventional Command (Hayes) mode
  961  RA12C                   IBM PC internal original Trailblazer
  962  RA12E                   External original Trailblazer
  963  RM12C                   Rackmount original Trailblazer
  964  T18PC                   IBM PC internal Trailblazer-Plus (TB+)
  965  T18SA, T2SAA, T2SAS     External TB+, T1600, T2000, T3000, WB, and later
  966  T18RMM                  Rackmount TB+
  967  T2MC                    IBM PS/2 internal TB+
  968  T1000                   External T1000
  969  ?                       Qblazer
  970                          Qblazer Plus
  971  T2500                   External T2500
  972  T2500                   Rackmount T2500
*/

/* Telebit model codes */

#define TB_UNK  0			/* Unknown Telebit model */
#define TB_BLAZ 1			/* Original TrailBlazer */
#define TB_PLUS	2			/* TrailBlazer Plus */
#define TB_1000 3			/* T1000 */
#define TB_1500 4			/* T1500 */
#define TB_1600 5			/* T1600 */
#define TB_2000 6			/* T2000 */
#define TB_2500 7			/* T2500 */
#define TB_3000 8			/* T3000 */
#define TB_QBLA 9			/* Qblazer */
#define TB_WBLA 10			/* WorldBlazer */
#define TB__MAX 10			/* Highest number */

char *tb_name[] = {			/* Array of model names */
    "Unknown",				/* TB_UNK  */
    "TrailBlazer",			/* TB_BLAZ */
    "TrailBlazer-Plus",			/* TB_PLUS */
    "T1000",				/* TB_1000 */
    "T1500",				/* TB_1500 */
    "T1600",				/* TB_1600 */
    "T2000",				/* TB_2000 */
    "T2500",				/* TB_2500 */
    "T3000",				/* TB_3000 */
    "Qblazer",				/* TB_QBLA */
    "WorldBlazer",			/* TB_WBLA */
    ""
};
#endif /* MINIDIAL */

extern int flow, local, mdmtyp, quiet, backgrd, parity, seslog, network;
extern int carrier, duplex, mdmsav;
#ifdef NETCONN
extern int ttnproto;
#endif /* NETCONN */
extern CHAR stchr;
extern long speed;
extern char ttname[], sesfil[];

/*  Failure codes  */

#define F_TIME		1		/* timeout */
#define F_INT		2		/* interrupt */
#define F_MODEM		3		/* modem-detected failure */
#define F_MINIT		4		/* cannot initialize modem */

static
#ifdef OS2
 volatile
#endif /* OS2 */
 int fail_code =  0;			/* Default failure reason. */

static int func_code;			/* 0 = dialing, nonzero = answering */
static int partial;
static int mymdmtyp = 0;

#define DW_NOTHING      0		/* What we are doing */
#define DW_INIT         1
#define DW_DIAL         2

static int dial_what = DW_NOTHING;	/* Nothing at first. */
static int nonverbal = 0;		/* Hayes in numeric response mode */
static MDMINF * mp;
static CHAR escbuf[6];
static long mdmcapas;

_PROTOTYP (static VOID dreset, (void) );
_PROTOTYP (static int (*xx_ok), (int,int) );
_PROTOTYP (static int ddinc, (int) );
_PROTOTYP (int dialhup, (void) );
_PROTOTYP (static int getok, (int,int) );
_PROTOTYP (char * ck_time, (void) );
_PROTOTYP (static VOID ttslow, (char *, int) );
#ifdef COMMENT
_PROTOTYP (static VOID xcpy, (char *, char *, unsigned int) );
#endif /* COMMENT */
_PROTOTYP (static VOID waitfor, (char *) );
_PROTOTYP (static VOID dialoc, (char) );
_PROTOTYP (static int didweget, (char *, char *) );
_PROTOTYP (static VOID spdchg, (long) );
#ifndef MINIDIAL
#ifdef OLDTBCODE
_PROTOTYP (static VOID tbati3, (int) );
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
_PROTOTYP (static int dialfail, (int) );
_PROTOTYP (static VOID gethrw, (void) );
_PROTOTYP (static VOID gethrn, (void) );

/*
 * Define symbolic modem numbers.
 *
 * The numbers MUST correspond to the ordering of entries
 * within the modemp array, and start at one (1).
 *
 * It is assumed that there are relatively few of these
 * values, and that the high(er) bytes of the value may
 * be used for modem-specific mode information.
 *
 * REMEMBER that only the first eight characters of these
 * names are guaranteed to be unique.
 */

#ifdef MINIDIAL				/* Minimum dialer support */
					/* Only for CCITT, HAYES, and UNK */
#define		n_CCITT		 1	/* CCITT/ITU-T V.25bis */
#define		n_HAYES		 2	/* Hayes 2400 */
#define		n_UNKNOWN	 3	/* Unknown */
#define         n_UDEF           4	/* User-Defined */
#define		MAX_MDM		 4	/* Number of modem types */

#else					/* Full-blown dialer support */

#define		n_ATTDTDM	 1
#define         n_ATTISN         2
#define		n_ATTMODEM	 3
#define		n_CCITT		 4
#define		n_CERMETEK	 5
#define		n_DF03		 6
#define		n_DF100		 7
#define		n_DF200		 8
#define		n_GDC		 9
#define		n_HAYES		10
#define		n_PENRIL	11
#define		n_RACAL		12
#define		n_UNKNOWN       13
#define		n_VENTEL	14
#define		n_CONCORD	15
#define		n_ATTUPC	16	/* aka UNIX PC and ATT7300 */
#define		n_ROLM          17      /* Rolm CBX DCM */
#define		n_MICROCOM	18	/* Microcoms in SX command mode */
#define         n_USR           19	/* Modern USRs */
#define         n_TELEBIT       20      /* Telebits of all kinds */
#define         n_DIGITEL       21	/* Digitel DT-22 (CCITT variant) */

#define         n_H_1200        22	/* Hayes 1200 */
#define		n_H_ULTRA       23	/* Hayes Ultra and maybe Optima */
#define		n_H_ACCURA      24	/* Hayes Accura and maybe Optima */
#define         n_PPI           25	/* Practical Peripherals */
#define         n_DATAPORT      26	/* AT&T Dataport */
#define         n_BOCA          27	/* Boca */
#define		n_MOTOROLA      28	/* Motorola Fastalk or Lifestyle */
#define		n_DIGICOMM	29	/* Digicomm Connection */
#define		n_DYNALINK      30	/* Dynalink 1414VE */
#define		n_INTEL		31	/* Intel 14400 Faxmodem */
#define		n_UCOM_AT	32	/* Microcoms in AT mode */
#define		n_MULTI		33	/* Multitech MT1432 */
#define		n_SUPRA		34	/* SupraFAXmodem */
#define	        n_ZOLTRIX	35	/* Zoltrix */
#define		n_ZOOM		36	/* Zoom */
#define		n_ZYXEL		37	/* ZyXEL */
#define         n_TAPI          38	/* Microsoft Windows dialer */
#define         n_TBNEW         39	/* Newer Telebit models */
#define		n_MAXTECH       40	/* MaxTech XM288EA */
#define         n_UDEF          41	/* User-Defined */
#define         n_RWV32         42	/* Generic Rockwell V.32 */
#define         n_RWV32B        43	/* Generic Rockwell V.32bis */
#define         n_RWV34         44	/* Generic Rockwell V.34 */
#define		n_MWAVE		45	/* IBM Mwave Adapter */
#define         n_TELEPATH      46	/* Gateway Telepath */
#define         n_MICROLINK     47	/* MicroLink modems */
#define         n_CARDINAL      48	/* Cardinal modems */
#define		MAX_MDM		48	/* Number of modem types */

#endif /* MINIDIAL */

int dialudt = n_UDEF;			/* Number of user-defined type */

/* BEGIN MDMINF STRUCT DEFINITIONS */

/*
  Declare structures containing modem-specific information.
  REMEMBER that only the first SEVEN characters of these names are
  guaranteed to be unique.

  First declare the three types that are allowed for MINIDIAL versions.
*/
static
MDMINF CCITT =				/* CCITT / ITU-T V.25bis autodialer */
/*
  According to V.25bis:
  . Even parity is required for giving commands to the modem.
  . Commands might or might not echo.
  . Responses ("Indications") from the modem are terminated by CR and LF.
  . Call setup is accomplished by:
    - DTE raises DTR (V.24 circuit 108)              [ttopen() does this]
    - Modem raises CTS (V.24 circuit 106)            [C-Kermit ignores this]
    - DTE issues a call request command ("CRN")
    - Modem responds with "VAL" ("command accepted")
    - If the call is completed:
        modem responds with "CNX" ("call connected");
        modem turns CTS (106) OFF;
        modem turns DSR (107) ON;
      else:
        modem responds with "CFI <parameter>" ("call failure indication").
  . To clear a call, the DTE turns DTR (108) OFF.
  . There is no mention of the Carrier Detect circuit (109) in the standard.
  . There is no provision for "escaping back" to the modem's command mode.

  It is not known whether there exists in real life a pure V.25bis modem.
  If there is, this code has never been tested on it.  See the Digitel entry.
*/
    {
    "CCITT / ITU-T V.25bis autodialer",
    "",			/* pulse command */
    "",			/* tone command */
    40,			/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "",			/* wake_str (none) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\015",        /* dial_str */
    200,		/* dial_rate (msec) */
    0,			/* No esc_time */
    0,			/* No esc_char  */
    "",			/* No hup_str  */
    "",			/* hwfc_str */
    "",			/* swfc_str */
    "",			/* nofc_str */
    "",			/* ec_on_str */
    "",			/* ec_off_str */
    "",			/* dc_on_str */
    "",			/* dc_off_str */
    "",			/* aa_on_str */
    "",			/* aa_off_str */
    "",			/* sb_on_str */
    "",			/* sb_off_str */
    0L,			/* max_speed */
    CKD_V25,		/* capas */
    NULL		/* No ok_fn    */
};

static
MDMINF HAYES =				/* Hayes 2400 and compatible modems */
    {
    "Hayes Smartmodem 2400 and compatibles",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0&S0&C1&D2\015",		/* wake_str */
#else
    "ATQ0\015",				/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, user supplies D or T */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    2400L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

/*
  The intent of the "unknown" modem is to allow KERMIT to support
  unknown modems by having the user type the entire autodial sequence
  (possibly including control characters, etc.) as the "phone number".
  The protocol and other characteristics of this modem are unknown, with
  some "reasonable" values being chosen for some of them.  The only way to
  detect if a connection is made is to look for carrier.
*/
static
MDMINF UNKNOWN =			/* Information for "Unknown" modem */
    {
    "Unknown",				/* name */
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

#ifndef MINIDIAL
static
MDMINF ATTISN =				/* AT&T ISN Network */
    {
    "",					/* pulse command */
    "",					/* tone command */
    "AT&T ISN Network",
    30,					/* Dial time */
    "",					/* Pause characters */
    0,					/* Pause time */
    "\015\015\015\015",			/* Wake string */
    900,				/* Wake rate */
    "DIAL",				/* Wake prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTMODEM =	/* information for AT&T switched-network modems */
			/* "Number" following "dial" can include: p's and
			 * t's to indicate pulse or tone (default) dialing,
			 * + for wait for dial tone, , for pause, r for
			 * last number dialed, and, except for 2224B, some
			 * comma-delimited options like o12=y, before number.

 * "Important" options for the modems:
 *
 *	All:		Except for 2224B, enable option 12 for "transparent
 *			data," o12=y.  If a computer port used for both
 *			incoming and outgoing calls is connected to the
 *			modem, disable "enter interactive mode on carriage
 *			return," EICR.  The Kermit "dial" command can
 *			function with EIA leads standard, EIAS.
 *
 *	2212C:		Internal hardware switches at their default
 *			positions (four rockers down away from numbers)
 *			unless EICR is not wanted (rocker down at the 4).
 *			For EIAS, rocker down at the 1.
 *
 *	2224B:		Front-panel switch position 1 must be up (at the 1,
 *			closed).  Disable EICR with position 2 down.
 *			For EIAS, position 4 down.
 *			All switches on the back panel down.
 *
 *	2224CEO:	All front-panel switches down except either 5 or 6.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *			Disable EICR with position 3 up.  For EIAS, 1 up.
 *			Reset the modem after changing switches.
 *
 *	2296A:		If option 00 (zeros) is present, use o00=0.
 *			Enable interactive flow control with o16=y.
 *			Select normal asynchronous mode with o34=0 (zero).
 *                      (available in Microcom Networking version, but
 *                      not necessarily other models of the 2296A).
 *			Enable modem-port flow control (if available) with
 * 			o42=y.  Enable asynchronous operation with o50=y.
 * 			Disable EICR with o69=n.  For EIAS, o66=n, using
 * 			front panel.
 */
    {
   "AT&T switched-network modems",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "+",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "at%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    CKD_AT,				/* capas */
    NULL				/* ok_fn */
};

static
MDMINF ATTDTDM = /* AT&T Digital Terminal Data Module  */
		 /* For dialing: KYBD switch down, others usually up. */
    {
    "AT&T Digital Terminal Data Module",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "%s\015",				/* dial_str */ 
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DIGITEL =        /* Digitel DT-22 CCITT variant used in Brazil */
/*
  Attempts to adhere strictly to the V.25bis specification do not produce good
  results in real life.  The modem for which this code was developed: (a)
  ignores parity; (b) sometimes terminates responses with LF CR instead of CR
  LF; (c) has a Hayes-like escape sequence; (d) supports a hangup ("HUP")
  command.  Information from Fernando Cabral in Brasilia.
*/
    {
    "Digitel DT-22 CCITT dialer",
    "",				/* pulse command */
    "",				/* tone command */
    40,				/* dial_time -- programmable -- */
    ",:",		/* pause_chars -- "," waits for programmable time */
                        /* ":" waits for dial tone */
    10,			/* pause_time (seconds, just a guess) */
    "HUP\015",          /* wake_str (Not Standard CCITT) */
    200,		/* wake_rate (msec) */
    "VAL",		/* wake_prompt */
    "",			/* dmode_str (none) */
    "",			/* dmode_prompt (none) */
    "CRN%s\015",        /* dial_str */
    200,		/* dial_rate (msec) */
    1100,		/* esc_time (Not Standard CCITT) */
    43,			/* esc_char  (Not Standard CCITT) */
    "HUP\015",		/* hup_str  (Not Standard CCITT) */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    CKD_V25,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_1200 =		/* Hayes 1200 and compatible modems */
    {
    "Hayes Smartmodem 1200 and compatibles",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "ATQ0\015",				/* wake_str */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    1200L,				/* max_speed */
    CKD_AT,				/* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ULTRA =			/* Hayes high-speed */
    {
    "Hayes Ultra/Optima/Accura 96/144/288", /* U,O,A */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4N1Y0&S0&C1&D2S37=0S82=128\015", /* wake_str */
#else
    "ATQ0X4N1Y0S37=0S82=128\015",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */ 
    "AT&K3\015",			/* hwfc_str */   /* OK for U,O */
    "AT&K4\015",			/* swfc_str */   /* OK for U,O */
    "AT&K0\015",			/* nofc_str */   /* OK for U,O */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */  /* OK for U,O */
    "AT&Q0\015",			/* ec_off_str */ /* OK for U,O */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF H_ACCURA =			/* Hayes Accura */
    {					/* GUESSING IT'S LIKE ULTRA & OPTIMA */
    "Hayes Accura",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4N1Y0&S0&C1&D2S37=0S82=128\015", /* wake_str */
#else
    "ATQ0X4N1Y0S37=0S82=128\015",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */ 
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    115200L,				/* max_speed */  /* (varies) */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF PPI =				/* Practical Peripherals  */
    {
    "Practical Peripherals V.22bis or higher with V.42 and V.42bis",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef COMMENT
/* In newer models S82 (BREAK handling) was eliminated, causing an error. */
#ifdef OS2
    "ATQ0X4N1&S0&C1&D2S37=0S82=128\015", /* wake_str */
#else
    "ATQ0X4N1S37=0S82=128\015",		/* wake_str */
#endif /* OS2 */
#else /* So now we use Y0 instead */
#ifdef OS2
    "ATQ0X4N1&S0&C1&D2Y0S37=0\015",	/* wake_str */
#else
    "ATQ0X4N1Y0S37=0\015",		/* wake_str */
#endif /* OS2 */
#endif /* COMMENT */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0S36=0S48=128\015",		/* ec_off_str */
    "ATS46=2\015",			/* dc_on_str */
    "ATS46=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str  */
    "",					/* sb_off_str  */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF DATAPORT =			/* AT&T Dataport  */
    {
    "AT&T / Paradyne DataPort V.32 or higher",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    /*
       Note: S41=0 (use highest modulation) omitted, since it is not
       supported on the V.32 and lower models.  So let's not touch it.
    */
#ifdef OS2
    "ATQ0E1X6&Q0S78=0\015",		/* wake_str */
#else
    "ATQ0E1X6&S0&C1&D2&Q0S78=0\015",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N7\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF UCOM_AT =			/* Microcom DeskPorte FAST ES 28.8 */
    {
    "Microcom DeskPorte FAST 28.8",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4\\N0F0&S0&C1&D2\\K5\015",	/* wake_str */
#else
    "ATQ0X4F0\\K5\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\H0\\Q0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\015",				/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT-J0\015",			/* sb_on_str */
    "AT-J1\015",			/* sb_off_str */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOOM =				/* Zoom Telephonics V.32bis  */
    {
    "Zoom Telephonics V.32bis",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1N1W1X4&S0&C1&D2S82=128S95=47\015", /* wake_str */
#else
    "ATQ0E1N1W1X4S82=128S95=47\015",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\015",		/* ec_on_str */
    "AT&Q0\015",			/* ec_off_str */
    "ATS46=138\015",			/* dc_on_str */
    "ATS46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZYXEL =				/* ZyXEL U-Series */
    {
    "ZyXEL U-Series V.32bis or higher",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1&S0&C1&D2&N0X5&Y1\015",	/* wake_str */
#else
    "ATQ0E1&N0X5&Y1\015",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H3\015",			/* hwfc_str */
    "AT&H4\015",			/* swfc_str */
    "AT&H0\015",			/* nofc_str */
    "AT&K3\015",			/* ec_on_str */
    "AT&K0\015",			/* ec_off_str */
    "AT&K4\015",			/* dc_on_str */
    "AT&K3\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ZOLTRIX =			/* Zoltrix */
    {
    "Zoltrix V.32bis and V.34 modems with Rockwell ACI chipset",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
   "ATQ0E1F0W1X4Y0&S0&C1&D2\\K5S82=128S95=41\015", /* wake_str */
#else
   "ATQ0E1F0W1X4Y0\\K5S82=128S95=41\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "ATS46=138%C3\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\N0\015",			/* sb_on_str */
    "AT&Q0\015",			/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MOTOROLA = {			/* Motorola FasTalk II or Lifestyle */
/*
  "\E" and "\X" commands removed - Motorola Lifestyle doesn't have them.
     \E0 = Don't echo while online
     \X0 = Process Xon/Xoff but don't pass through
*/
    "Motorola FasTalk II or Lifestyle",	/* Name */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1X4&S0&C1&D2\\K5\\V1\015",	/* wake_str */
#else
    "ATQ0E1X4\\K5\\V1\015",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N6\015",			/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT\\J0\015",			/* sb_on_str */
    "AT\\J1\015",			/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF BOCA =				/* Boca */
    {
    "BOCA 14.4 Faxmodem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1F1N1W1&S0&C1&D2\\K5S37=11S82=128S95=47X4\015", /* wake_str */
#else
    "ATQ0E1F1N1W1\\K5S37=11S82=128S95=47X4\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT\\N3S36=7S48=7\015",		/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "ATS46=138\015",			/* dc_on_str */
    "ATS46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF INTEL =				/* Intel */
    {
    "Intel High-Speed Faxmodem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1Y0X4&S0&C1&D2\\K1\\V2S25=50\015", /* wake_str */
#else
    "ATQ0E1Y0X4\\K1\\V2S25=50\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "ATB1+FCLASS=0\015",		/* dmode_str */ 
    "OK\015",				/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\G1\\Q3\015",			/* hwfc_str */
    "AT\\G1\\Q1\\X0\015",		/* swfc_str */
    "AT\\G0\015",			/* nofc_str */
    "AT\\J0\\N3\"H3\015",		/* ec_on_str */
    "AT\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MULTITECH =			/* Multitech */
    {
    "Multitech MT1432 Series MultiModem II",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1X4&S0&C1&D2&E8&Q0%E1#P0\015", /* wake_str */
#else
    "ATQ0E1X4&E8&Q0%E1#P0\015",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&E4&E7&E8&E11&E13\015",		/* hwfc_str */
    "AT&E5&E6&E8&E11&E13\015",		/* swfc_str */
    "AT&E3&E7&E8&E10&E12\015",		/* nofc_str */
    "AT&E1\015",			/* ec_on_str */
    "AT#L0&E0\015",			/* ec_off_str */
    "AT&E15\015",			/* dc_on_str */
    "AT&E14\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "AT$BA0\015",			/* sb_on_str (= "baud adjust off") */
    "AT$BA1\015",			/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF SUPRA =				/* Supra */
    {
    "SupraFAXModem 144 or 288",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1N1W0X4Y0&S0&C1&D2\\K5S82=128\015", /* wake_str */
#else
    "ATQ0E1N1W0X4Y0\\K5S82=128\015",	/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4\015",			/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\\N3S48=7\015",		/* ec_on_str */
    "AT&Q0\\N1\015",			/* ec_off_str */
    "AT%C1S46=138\015",			/* dc_on_str */
    "AT%C0S46=136\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MAXTECH =			/* MaxTech */
    {
    "MaxTech XM288EA or GVC FAXModem",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0E1X4Y0&S0&C1&D2&L0&M0\\K5\015", /* wake_str */
#else
    "ATQ0E1X4Y0&L0&M0\\K5\015",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N6\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT\\N6%C1\015",			/* dc_on_str */
    "AT\\N6%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF ROLM =		/* IBM / Siemens / Rolm 8000, 9000, 9751 CBX DCM */
    {
    "IBM/Siemens/Rolm CBX Data Communications Module",
    "",					/* pulse command */
    "",					/* tone command */
    60,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\015\015",				/* wake_str */
    5,					/* wake_rate */
    "MODIFY?",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "CALL %s\015",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    19200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF USR =				/* USR Courier and Sportster modems */
    {
    "US Robotics Courier or Sportster",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4&A3&S0&C1&D2&N0&Y3\015",	/* wake_str */
#else
    "ATQ0X4&A3&N0&Y3\015",		/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H1&R2&I0\015",			/* hwfc_str */
    "AT&H2&R1&I2\015",			/* swfc_str */
    "AT&H0&R1&I0\015",			/* nofc_str */
    "AT&M4&B1\015",			/* ec_on_str */
    "AT&M0\015",			/* ec_off_str */
    "AT&K1\015",			/* dc_on_str */
    "AT&K0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

#ifdef OLDTBCODE
static
MDMINF TELEBIT =			/* All Telebits */
    {
    "Telebit - all models",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
/*
  NOTE: The wake_string MUST contain the I command (model query), and otherwise
  must contain commands that work on ALL Telebit models.  Here we ensure that
  result codes are returned (Q0), and ask for extended result codes (X1), and
  ensure that the escape sequence is +++ and it is enabled.  And also, make
  sure the final character is not a digit (whose echo might be mistaken for a
  result code).  The Ctrl-Q (\021) and multiple A's are recommended by Telebit.
*/
#ifdef OS2
    "\021AAAAATQ0X1&S0&C1&D2S12=50 S50=0 I\015", /* wake_str. */
#else
    "\021AAAAATQ0X1S12=50 S50=0 I\015", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "ATS58=2S68=2\015",			/* hwfc_str */
    "ATS58=3S68=3\015",			/* swfc_str */
    "ATS58=0S68=0\015",			/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB, /* capas */
    getok				/* ok_fn */
};

#else

static
MDMINF OLDTB =				/* Old Telebits */
    {
    "Telebit TrailBlazer, T1000, T1500, T2000, T2500",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\021AAAAATQ0X1&S0&C1&D2S12=50S50=0S54=3\015", /* wake_str. */
#else
    "\021AAAAATQ0X1S12=50S50=0S54=3\015", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "ATS58=2S68=2\015",			/* hwfc_str */
    "ATS58=3S68=3S69=0\015",		/* swfc_str */
    "ATS58=0S68=0\015",			/* nofc_str */
    "ATS66=1S95=2\015",			/* ec_on_str */
    "ATS95=0\015",			/* ec_off_str */
    "ATS110=1S96=1\015",		/* dc_on_str */
    "ATS110=0S96=0\015",		/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    19200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};

static
MDMINF NEWTB =				/* New Telebits */
    {
    "Telebit T1600, T3000, QBlazer, WorldBlazer, etc.",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    60,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "\021AAAAATQ0X2&S0&C1&D2S12=50S50=0S61=1S63=0\015", /* wake_str. */
#else
    "\021AAAAATQ0X2S12=50S50=0S61=1S63=0\015", /* wake_str. */
#endif /* OS2 */
    100,				/* wake_rate = 100 msec */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str, Note: no T or P */
    80,					/* dial_rate */
    1100,				/* esc_time (guard time) */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "ATS58=2S68=2\015",			/* hwfc_str */
    "ATS58=3S68=3\015",			/* swfc_str */
    "ATS58=0S68=0\015",			/* nofc_str */
    "ATS180=3\015",			/* ec_on_str */
    "ATS180=0\015",			/* ec_off_str */
    "ATS190=1\015",			/* dc_on_str */
    "ATS190=0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    38400L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_TB|CKD_KS, /* capas */
    getok				/* ok_fn */
};
#endif /* OLDTBCODE */

static
MDMINF DUMMY = /* dummy information for modems that are handled elsewhere */
    {
    "(dummy)",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "",					/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF RWV32 =				/* Generic Rockwell V.32 */
    {
    "Generic Rockwell V.32 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4W1Y0&S0&C1&D2%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5\\N0\015",			/* ec_on_str */
    "AT&Q0\\N1\015",			/* ec_off_str */
    "AT%C1\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV32B =				/* Generic Rockwell V.32bis */
    {
    "Generic Rockwell V.32bis modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4W1Y0&S0&C1&D2%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\K5+FCLASS=0N1S37=0\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\\N3\015",		/* ec_on_str */
    "AT&Q0S48=128\\N1\015",		/* ec_off_str */
    "ATS46=138%C1\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF RWV34 =				/* Generic Rockwell V.34 Data/Fax */
    {
    "Generic Rockwell V.34 modem",	/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4W1Y0%E2&S0&C1&D2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\\N3\015",		/* ec_on_str */
    "AT&Q0S48=128\\N1\015",		/* ec_off_str */
    "ATS46=138%C1\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF MWAVE =				/* IBM Mwave */
    {
    "IBM Mwave Adapter",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4Y0&S0&C1&D2&M0&Q0&N1\\K3\\T0%E2S28=0\015", /* wake_str */
#else
    "ATQ0X4Y0&M0&Q0&N1&S0\\K3\\T0%E2S28=0\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "",					/* swfc_str (it doesn't!) */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N7\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C1\"H3\015",			/* dc_on_str */
    "AT%C0\"H0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF TELEPATH =			/* Gateway 2000 Telepath */
    {
    "Gateway 2000 Telepath II 28.8",
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4&S0&C1&D2&N0&Y2#CLS=0S13=0S15=0S19=0\015", /* wake_str */
#else
    "ATQ0X4&N0&Y1#CLS=0S13=0S15=0S19=0\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&H1&R2\015",			/* hwfc_str */
    "AT&H2&I2S22=17S23=19\015",		/* swfc_str */
    "AT&H0&I0&R1\015",			/* nofc_str */
    "AT&M4&B1\015",			/* ec_on_str -- also fixes speed */
    "AT&M0\015",			/* ec_off_str */
    "AT&K1\015",			/* dc_on_str */
    "AT&K0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};

static
MDMINF CARDINAL =			/* Cardinal - based on Rockwell V.34 */
    {
    "Cardinal MVP288X Series",		/* ATI3, ATI4, and ATI6 for details */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4W1Y0%E2&S0&C1&D2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#else
    "ATQ0X4W1Y0%E2\\K5+FCLASS=0+MS=11,1\015", /* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H0\015",			/* hup_str */
    "AT&K3\015",			/* hwfc_str */
    "AT&K4S32=17S33=19\015",		/* swfc_str */
    "AT&K0\015",			/* nofc_str */
    "AT&Q5S36=7S48=7\\N3\015",		/* ec_on_str */
    "AT&Q0S48=128\\N1\015",		/* ec_off_str */
    "ATS46=138%C1\015",			/* dc_on_str */
    "ATS46=136%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    115200L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW, /* capas */
    getok				/* ok_fn */
};

/*
  Now the "old" modems, all grouped together, and also within 
  "if not defined MINIDIAL"...
*/
#ifdef OLDMODEMS

static
MDMINF CERMETEK =	/* Information for "Cermetek Info-Mate 212 A" modem */
    {
    "Cermetek Info-Mate 212 A",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "BbPpTt",				/* pause_chars */
    0,					/* pause_time */
    "  XY\016R\015",			/* wake_str */
    200,				/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "\016D '%s'\015",			/* dial_str */
    200,				/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF03 =		/* information for "DEC DF03-AC" modem */
    {
    "Digital DF03-AC",
    "",					/* pulse command */
    "",					/* tone command */
    27,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001\002",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF100 =		/* information for "DEC DF100-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-100",
    "",					/* pulse command */
    "",					/* tone command */
    30,					/* dial_time */
    "=",				/* pause_chars */
    15,					/* pause_time */
    "\001",				/* wake_str */
    0,					/* wake_rate */
    "",					/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "%s#",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF DF200 =		/* information for "DEC DF200-series" modem */
			/*
			 * The telephone "number" can include "P"s and/or "T"s
			 * within it to indicate that subsequent digits are
			 * to be dialed using pulse or tone dialing.  The
			 * modem defaults to pulse dialing.  You may modify
			 * the dial string below to explicitly default all
			 * dialing to pulse or tone, but doing so prevents
			 * the use of phone numbers that you may have stored
			 * in the modem's memory.
			 */
    {
    "Digital DF-200",
    "",			/* pulse command */
    "",			/* tone command */
    30,			/* dial_time */
    "=W",		/* pause_chars */	/* =: second tone; W: 5 secs */
    15,			/* pause_time */	/* worst case */
    "\002",		/* wake_str */		/* allow stored number usage */
    0,			/* wake_rate */
    "",			/* wake_prompt */
    "",			/* dmode_str */
    NULL,		/* dmode_prompt */
#ifdef COMMENT
    "%s!",		/* dial_str */
#else
    "   d %s\015",
#endif /* COMMENT */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF GDC =		/* information for "GeneralDataComm 212A/ED" modem */
    {
    "GeneralDataComm 212A/ED",
    "",					/* pulse command */
    "",					/* tone command */
    32,					/* dial_time */
    "%",				/* pause_chars */
    3,					/* pause_time */
    "\015\015",				/* wake_str */
    500,				/* wake_rate */
    "$",				/* wake_prompt */
    "D\015",				/* dmode_str */
    ":",				/* dmode_prompt */
    "T%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    1200L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF PENRIL =		/* information for "Penril" modem */
    {
    "Penril modem",
    "",					/* pulse command */
    "",					/* tone command */
    50,					/* dial_time */
    "",					/* pause_chars */
    0,					/* pause_time */
    "\015\015",				/* wake_str */
    300,				/* wake_rate */
    ">",				/* wake_prompt */
    "k\015",				/* dmode_str */
    ":",				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF RACAL =				/* Racal Vadic VA4492E */
    {
    "Racal Vadic VA4492E",
    "",					/* pulse command */
    "",					/* tone command */
    35,			/* dial_time (manual says modem is hardwired to 60) */
    "Kk",				/* pause_chars */
    5,					/* pause_time */
    "\005\015",				/* wake_str, ^E^M */
    50,					/* wake_rate */
    "*",				/* wake_prompt */
    "D\015",				/* dmode_str */
    "?",				/* dmode_prompt */
    "%s\015",				/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    5,					/* esc_char, ^E */
    "\003\004",				/* hup_str, ^C^D */
    0,					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF VENTEL =				/* Information for Ven-Tel modem */
    {
    "Ven-Tel",
    "",					/* pulse command */
    "",					/* tone command */
    20,					/* dial_time */
    "%",				/* pause_chars */
    5,					/* pause_time */
    "\015\015\015",			/* wake_str */
    300,				/* wake_rate */
    "$",				/* wake_prompt */
    "K\015",				/* dmode_str (was "") */
    "Number to call: ",			/* dmode_prompt (was NULL) */
    "%s\015",				/* dial_str (was "<K%s\r>") */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};

static
MDMINF CONCORD =	/* Info for Condor CDS 220 2400b modem */
    {
    "Concord Condor CDS 220 2400b",
    "",					/* pulse command */
    "",					/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
    "\015\015",				/* wake_str */
    20,					/* wake_rate */
    "CDS >",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "<D M%s\015>",			/* dial_str */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char */
    "",					/* hup_str */
    "",					/* hwfc_str */
    "",					/* swfc_str */
    "",					/* nofc_str */
    "",					/* ec_on_str */
    "",					/* ec_off_str */
    "",					/* dc_on_str */
    "",					/* dc_off_str */
    "",					/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    2400L,				/* max_speed */
    0,					/* capas */
    NULL				/* ok_fn */
};
#endif /* OLDMODEMS */

static
MDMINF MICROCOM =	/* Microcom modems in native SX mode */
			/* (long answer only) */
{
    "Microcom MNP modems in SX command mode",
    "DP\015",				/* pulse command */
    "DT\015",				/* tone command */
    35,					/* dial_time */
    ",!@",		/* pause_chars (! and @ aren't pure pauses) */
    3,					/* pause_time */
/*
  The following sets 8 bits, no parity, BREAK passthru, and SE0 disables the
  escape character, which is a single character with no guard time, totally
  unsafe, so we have no choice but to disable it.  Especially since, by
  default, it is Ctrl-A, which is Kermit's packet-start character.  We would
  change it to something else, which would enable "mdmhup()", but the user
  wouldn't know about it.  Very bad.  Note: SE1 sets it to Ctrl-A, SE2
  sets it to Ctrl-B, etc (1..31 allowed).  Also SE/Q sets it to "Q".
*/
    "SE0;S1P4;SBRK5\015",		/* wake_str */
    100,				/* wake_rate */
    "!",				/* wake_prompt */
    "",					/* dmode_str */
    NULL,				/* dmode_prompt */
    "D%s\015",				/* dial_str - number up to 39 chars */
    0,					/* dial_rate */
    0,					/* esc_time */
    0,					/* esc_char - we can't use this */
    "",					/* hup_str - it's "H" but can't use */
    "SF13\015",				/* hwfc_str */
    "SF11\015",				/* swfc_str */
    "SF10\015",				/* nofc_str */
    "BAOFF;SMAUT\015",			/* ec_on_str */
    "BAON;SMDIR\015",			/* ec_off_str */
    "COMP1\015",			/* dc_on_str */
    "COMP0\015",			/* dc_off_str */
    "AA",				/* aa_on_str */
    "",					/* aa_off_str */
    "",					/* sb_on_str */
    "",					/* sb_off_str */
    0L,					/* max_speed */
    CKD_SB|CKD_EC|CKD_DC|CKD_HW|CKD_SW|CKD_KS, /* capas */
    getok				/* ok_fn */
};


static
MDMINF MICROLINK =			/* MicroLink ... */
    {					/* 14.4TQ,TL,PC;28.8TQ,TQV;2440T/TR */
    "MicroLink 14.4 or 28.8",		/* ELSA GmbH, Aachen */
    "ATP\015",				/* pulse command */
    "ATT\015",				/* tone command */
    35,					/* dial_time */
    ",",				/* pause_chars */
    2,					/* pause_time */
#ifdef OS2
    "ATQ0X4&S0\\D0&C1&D2\\K5\015",	/* wake_str */
#else
    "ATQ0X4\\K5\015",			/* wake_str */
#endif /* OS2 */
    0,					/* wake_rate */
    "OK\015",				/* wake_prompt */
    "",					/* dmode_str */
    "",					/* dmode_prompt */
    "ATD%s\015",			/* dial_str */
    0,					/* dial_rate */
    1100,				/* esc_time */
    43,					/* esc_char */
    "ATQ0H\015",			/* hup_str */
    "AT\\Q3\015",			/* hwfc_str */
    "AT\\Q1\\X0\015",			/* swfc_str */
    "AT\\Q0\015",			/* nofc_str */
    "AT\\N3\015",			/* ec_on_str */
    "AT\\N0\015",			/* ec_off_str */
    "AT%C3\015",			/* dc_on_str */
    "AT%C0\015",			/* dc_off_str */
    "ATS0=1\015",			/* aa_on_str */
    "ATS0=0\015",			/* aa_off_str */
    "\\J0",				/* sb_on_str (?) */
    "",					/* sb_off_str */
    57600L,				/* max_speed */
    CKD_AT|CKD_SB|CKD_EC|CKD_DC|CKD_HW, /* capas */
    getok				/* ok_fn */
};


#endif /* MINIDIAL */

/* END MDMINF STRUCT DEFINITIONS */

/*
  Table to convert modem numbers to MDMINF struct pointers.
  The entries MUST be in ascending order by modem number, without any
  "gaps" in the numbers, and starting from one (1).
*/

MDMINF *modemp[] = {
#ifdef MINIDIAL
    &CCITT,				/*  1 */
    &HAYES,				/*  2 */
    &UNKNOWN				/*  3 */
#else
    &ATTDTDM,				/*  1 */
    &ATTISN,				/*  2 */
    &ATTMODEM,				/*  3 */
    &CCITT,				/*  4 */
#ifdef OLDMODEMS
    &CERMETEK,				/*  5 */
    &DF03,				/*  6 */
    &DF100,				/*  7 */
    &DF200,				/*  8 */
    &GDC,				/*  9 */
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &HAYES,				/* 10 */
#ifdef OLDMODEMS
    &PENRIL,				/* 11 */
    &RACAL,				/* 12 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &UNKNOWN,				/* 13 */
#ifdef OLDMODEMS
    &VENTEL,				/* 14 */
    &CONCORD,				/* 15 */
#else
    NULL,
    NULL,
#endif /* OLDMODEMS */
    &DUMMY,				/* 16 */
    &ROLM,				/* 17 */
#ifdef OLDMODEMS
    &MICROCOM,				/* 18 */
#else
    NULL,
#endif /* OLDMODEMS */
    &USR,				/* 19 USR Courier and Sportster */
#ifdef OLDTBCODE
    &TELEBIT,
#else
    &OLDTB,				/* 20 Old Telebits */
#endif /* OLDTBCODE */
    &DIGITEL,				/* 21 Digitel CCITT */
    &H_1200,				/* 22 Hayes 1200 */
    &H_ULTRA,				/* 23 Hayes Ultra */
    &H_ACCURA,				/* 24 Hayes Optima */
    &PPI,				/* 25 PPI */
    &DATAPORT,				/* 26 Dataport */
    &BOCA,				/* 27 Boca */
    &MOTOROLA,				/* 28 Motorola UDS MOTOROLA */
    NULL,				/* 29 Digicomm */
    NULL,				/* 30 Dynalink */
    &INTEL,				/* 31 Intel */
    &UCOM_AT,				/* 32 Microcom in AT mode */
    &MULTITECH,				/* 33 Multitech */
    &SUPRA,				/* 34 Supra */
    &ZOLTRIX,				/* 35 Zoltrix */
    &ZOOM,				/* 36 Zoom */
    &ZYXEL,				/* 37 ZyXEL */
    &DUMMY,				/* 38 TAPI */
#ifdef OLDTBCODE
    &TELEBIT,
#else
    &NEWTB,				/* 39 New-Telebit */
#endif /* OLDTBCODE */
    &MAXTECH,				/* 40 MaxTech */
    &DUMMY,				/* 41 User-defined */
    &RWV32,				/* 42 Rockwell V.32 */
    &RWV32B,				/* 43 Rockwell V.32bis */
    &RWV34,				/* 44 Rockwell V.34 */
    &MWAVE,				/* 45 IBM Mwave */
    &TELEPATH,				/* 46 Gateway 2000 Telepath II 28.8 */
    &MICROLINK,				/* 47 MicroLink modems */
    &CARDINAL				/* 48 Cardinal */
#endif /* MINIDIAL */
};
/*
 * Declare modem names and associated numbers for command parsing,
 * and also for doing number-to-name translation.
 *
 * The entries must be in alphabetical order by modem name.
 */
struct keytab mdmtab[] = {
#ifndef MINIDIAL
    "att-dataport",	n_DATAPORT,	0,
    "att-dtdm",		n_ATTDTDM,	0,
    "attdtdm",		n_ATTDTDM,	CM_INV,	/* old name */
    "att-isn",          n_ATTISN,       0,
    "attisn",           n_ATTISN,       CM_INV,	/* old name */
    "att-switched-net", n_ATTMODEM,	0,
    "attmodem",		n_ATTMODEM,	CM_INV,	/* old name */
    "att-7300",		n_ATTUPC,	0,
    "att7300",		n_ATTUPC,	CM_INV,	/* old name */
    "boca",		n_BOCA,		0,
#endif /* MINIDIAL */
    "ccitt-v25bis",	n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifndef MINIDIAL
    "cardinal",         n_CARDINAL,     0,
#ifdef OLDMODEMS
    "cermetek",		n_CERMETEK,	M_OLD,
    "concord",		n_CONCORD,	M_OLD,
#endif /* OLDMODEMS */
    "courier",          n_USR,          CM_INV,
    "dataport",		n_DATAPORT,	CM_INV,	/* == att-dataport */
#ifdef OLDMODEMS
    "df03-ac",		n_DF03,		M_OLD,
    "df100-series",	n_DF100,	M_OLD,
    "df200-series",	n_DF200,	M_OLD,
#endif /* OLDMODEMS */
    "digitel-dt22",	n_DIGITEL,	0,
#endif /* MINIDIAL */
    "direct",		0,		CM_INV,	/* Synonym for NONE */
#ifndef MINIDIAL
    "gateway-telepath", n_TELEPATH,     0,
#ifdef OLDMODEMS
    "gdc-212a/ed",	n_GDC,		M_OLD,
    "gendatacomm",	n_GDC,		CM_INV,	/* Synonym for GDC */
#endif /* OLDMODEMS */
#endif /* MINIDIAL */
    "h", 	   	n_HAYES,	CM_INV|CM_ABR,
    "ha", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hay",    		n_HAYES,	CM_INV|CM_ABR,
    "haye", 	   	n_HAYES,	CM_INV|CM_ABR,
    "hayes",    	n_HAYES,	CM_INV|CM_ABR, /* Hayes 2400 */
#ifndef MINIDIAL
    "hayes-1200",	n_H_1200,	0,
#endif /* MINIDIAL */
    "hayes-2400",	n_HAYES,	0,
#ifndef MINIDIAL
    "hayes-high-speed", n_H_ACCURA,     0,
    "hayes-accura",     n_H_ACCURA,     CM_INV,
    "hayes-optima",     n_H_ACCURA,     CM_INV,
    "hayes-ultra",	n_H_ULTRA,	CM_INV,
    "hst-courier",      n_USR,          CM_INV,	/* Synonym for COURIER */
    "intel",		n_INTEL,        0,
#endif /* MINIDIAL */
    "itu-t-v25bis",	n_CCITT,	0,	/* New name for CCITT */
#ifndef MINIDIAL
    "maxtech",		n_MAXTECH,	0,
    "mi",		n_MICROCOM,	CM_INV|CM_ABR,
    "mic",		n_MICROCOM,	CM_INV|CM_ABR,
    "micr",		n_MICROCOM,	CM_INV|CM_ABR,
    "micro",		n_MICROCOM,	CM_INV|CM_ABR,
    "microc",		n_MICROCOM,	CM_INV|CM_ABR,
    "microco",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom",		n_MICROCOM,	CM_INV|CM_ABR,
    "microcom-at-mode",	n_UCOM_AT,	0, /* Microcom DeskPorte, etc */
    "microcom-sx-mode",	n_MICROCOM,	0, /* Microcom AX,QX,SX, native mode */
    "microlink",        n_MICROLINK,    0,
    "motorola-fastalk", n_MOTOROLA,	0,
    "multitech",	n_MULTI,	0,
    "mwave",		n_MWAVE,	0,
#endif /* MINIDIAL */
    "none",             0,              0,
#ifndef MINIDIAL
#ifndef OLDTBCODE
    "old-telebit",      n_TELEBIT,      0,
#endif /* OLDTBCODE */
#ifdef OLDMODEMS
    "penril",		n_PENRIL,	M_OLD,
#endif /* OLDMODEMS */
    "ppi",              n_PPI,		0,
#ifdef OLDMODEMS
    "racalvadic",	n_RACAL,	M_OLD,
#endif /* OLDMODEMS */
    "rockwell-v32",	n_RWV32,	0,
    "rockwell-v32bis",	n_RWV32B,	0,
    "rockwell-v34",	n_RWV34,	0,
    "rolm-dcm",		n_ROLM,		0,
    "sportster",        n_USR,          CM_INV,
    "supra",		n_SUPRA,	0,
    "tapi",		n_TAPI,		CM_INV,
#ifndef OLDTBCODE
    "te",               n_TBNEW,        CM_INV|CM_ABR,
    "tel",              n_TBNEW,        CM_INV|CM_ABR,
    "telebit",          n_TBNEW,        0,
#else
    "te",               n_TELEBIT,      CM_INV|CM_ABR,
    "tel",              n_TELEBIT,      CM_INV|CM_ABR,
    "telebit",          n_TELEBIT,      0,
#endif /* OLDTBCODE */
    "telepath",         n_TELEPATH,     CM_INV,
#endif /* MINIDIAL */
    "unknown",		n_UNKNOWN,	0,
    "user-defined",     n_UDEF,		0,
#ifndef MINIDIAL
    "usr",              n_USR,          CM_INV|CM_ABR,
/* Keep the next one for backwards compatibility, but it's the same as H2400 */
    "usr-212a",		n_HAYES,	CM_INV,
    "usr-courier",      n_USR,          CM_INV,
    "usr-sportster",    n_USR,          CM_INV,
    "usrobotics",       n_USR,          0,
    "v25bis",		n_CCITT,	CM_INV, /* Name changed to ITU-T */
#ifdef OLDMODEMS
    "ventel",		n_VENTEL,	M_OLD,
#endif /* OLDMODEMS */
    "zoltrix",		n_ZOLTRIX,	0,
    "zoom",		n_ZOOM,		0,
    "zyxel",		n_ZYXEL,	0,
    "",                 0,              0
#endif /* MINIDIAL */
};
int nmdm = (sizeof(mdmtab) / sizeof(struct keytab)) - 1; /* Number of modems */

#define CONNECTED 1			/* For completion status */
#define D_FAILED  2
#define D_PARTIAL 3

static int tries = 0;
static int mdmecho = 0;	/* assume modem does not echo */

static char *p;		/* For command strings & messages */

#ifdef DYNAMIC
#define LBUFL 256
static char *lbuf = NULL;
#else
#define LBUFL 100
static char lbuf[LBUFL];
#endif /* DYNAMIC */

#ifdef DYNAMIC
#define RBUFL 256
static char *rbuf = NULL;
#else
#define RBUFL 63
static char rbuf[RBUFL+1];
#endif /* DYNAMIC */

#ifdef DYNAMIC
#define FULLNUML 256
char *fbuf = NULL;			/* For full (prefixed) phone number */
#else
#define FULLNUML 100
char fbuf[FULLNUML];
#endif /* DYNAMIC */

static ckjmpbuf sjbuf;

#ifdef CK_ANSIC
static SIGTYP (*savalrm)(int);	/* For saving alarm handler */
static SIGTYP (*savint)(int);	/* For saving interrupt handler */
#else 
static SIGTYP (*savalrm)();	/* For saving alarm handler */
static SIGTYP (*savint)();	/* For saving interrupt handler */
#endif /* CK_ANSIC */

#ifndef MINIDIAL
#ifdef OLDTBCODE
int tbmodel = 0;		/* Telebit modem model */

char *
gtbmodel() {			/* Function to return name of Telebit model */
    if (tbmodel < 0 || tbmodel > TB__MAX) tbmodel = TB_UNK;
    return(tb_name[tbmodel]);
}
#endif /* OLDTBCODE */

#ifdef COMMENT
static VOID
xcpy(to,from,len)		/* Copy the given number of bytes */
    register char *to, *from;
    register unsigned int len; {
	while (len--) *to++ = *from++;
}
#endif /* COMMENT */
#endif /* MINIDIAL */

static SIGTYP
#ifdef CK_ANSIC
dialtime(int foo)			/* Timer interrupt handler */
#else
dialtime(foo) int foo;			/* Timer interrupt handler */
#endif /* CK_ANSIC */
/* dialtime */ {

    fail_code = F_TIME;			/* Failure reason = timeout */
    debug(F100,"dialtime caught SIGALRM","",0);
#ifdef BEBOX
    alarm_expired();
#endif /* BEBOX */
#ifdef OS2 
    signal(SIGALRM, dialtime);
#endif /* OS2 */
#ifdef __EMX__
    signal(SIGALRM, SIG_ACK);		/* Needed for OS/2 */
#endif /* __EMX__ */

#ifdef OSK				/* OS-9 */
/*
  We are in an intercept routine but do not perform a F$RTE (done implicitly
  by RTS), so we have to decrement the sigmask as F$RTE does.  Warning:
  longjump only restores the CPU registers, NOT the FPU registers.  So, don't
  use FPU at all or at least don't use common FPU (double or float) register
  variables.
*/
    sigmask(-1);
#endif /* OSK */

#ifdef NTSIG
    if (foo == SIGALRM)
      PostAlarmSigSem();
    else 
      PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(sjbuf),1);
#else /* NT */
    cklongjmp(sjbuf,1);
#endif /* NT */
#endif /* NTSIG */
    /* NOTREACHED */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
dialint(int foo)			/* Keyboard interrupt handler */
#else
dialint(foo) int foo;			/* Keyboard interrupt handler */
#endif /* CK_ANSIC */
/* dialint */ {
    fail_code = F_INT;
    debug(F100,"dialint caught SIGINT","",0);
#ifdef OS2
    signal(SIGINT, dialint);
    debug(F100,"dialint() SIGINT caught -- dialint restored","",0) ;
#endif /* OS2 */
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);		/* Needed for OS/2 */
#endif /* __EMX__ */
#ifdef OSK				/* OS-9, see comment in dialtime() */
    sigmask(-1);
#endif /* OSK */
#ifdef NTSIG
    PostCtrlCSem() ;
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(sjbuf),1);
#else /* NT */
    cklongjmp(sjbuf,1);
#endif /* NT */
#endif /* NT */
    SIGRETURN;
}

/*
  Routine to read a character from communication device, handling TELNET
  protocol negotiations in case we're connected to the modem through a
  TCP/IP TELNET modem server.
*/
static int
ddinc(n) int n; {
    int c;

#ifdef TNCODE
    int done = 0;
    debug(F101,"ddinc entry n","",n);
    while (!done) {
	c = ttinc(n);
	debug(F000,"ddinc","",c);
	if (c < 0) return(c);
	if (c == IAC && network && ttnproto == NP_TELNET) {
	    switch (tn_doop((CHAR)(c & 0xff),duplex,ttinc)) {
	      case 2: duplex = 0; continue;
	      case 1: duplex = 1;
	      default: continue;
	    }
	} else done = 1;
    }
    return(c & 0xff);
#else
    debug(F101,"ddinc entry n","",n);
    return(ttinc(n));
#endif /* TNCODE */
}

static VOID
ttslow(s,millisec) char *s; int millisec; { /* Output s-l-o-w-l-y */
#ifdef TCPSOCKET
    extern int tn_nlm, tn_b_nlm, me_binary;
#endif /* TCPSOCKET */
    if (dialdpy && duplex)		/* Echo the command in case modem */
      printf("%s\n",s);			/* isn't echoing commands. */
    for (; *s; s++) {
	ttoc(*s);
#ifdef TCPSOCKET
	if (*s == CR && network && ttnproto == NP_TELNET) {
       if (!me_binary && tn_nlm != TNL_CR)
    	  ttoc((char)((tn_nlm == TNL_CRLF) ? LF : NUL));
        else if (me_binary &&
            (tn_b_nlm == TNL_CRLF || tn_b_nlm == TNL_CRNUL))
    	  ttoc((char)((tn_b_nlm == TNL_CRLF) ? LF : NUL));
        }
#endif /* TCPSOCKET */
	msleep(millisec);
    }
}

/*
 * Wait for a string of characters.
 *
 * The characters are waited for individually, and other characters may
 * be received "in between".  This merely guarantees that the characters
 * ARE received, and in the order specified.
 */
static VOID
waitfor(s) char *s; {
    CHAR c, x;
    while ( c = *s++ ) {		/* while more characters remain... */
	do {				/* wait for the character */
	    x = (CHAR) (ddinc(0) & 0177);
	    debug(F000,"dial waitfor got","",x);
	    if (dialdpy) {
		if (x != LF) conoc(x);
		if (x == CR) conoc(LF);
	    }
	} while ( x != c);
    }
}

static int
didweget(s,r) char *s, *r; {	/* Looks in string s for response r */
    int lr = (int)strlen(r);	/*  0 means not found, 1 means found it */
    int i;
    debug(F110,"didweget",r,0);
    debug(F110," in",s,0);
    for (i = (int)strlen(s)-lr; i >= 0; i--)
	if ( s[i] == r[0] ) if ( !strncmp(s+i,r,lr) ) return( 1 );
    return( 0 );
}


/* R E S E T -- Reset alarms, etc. on exit. */

static VOID
dreset() {
    debug(F100,"dreset resetting alarm and signal handlers","",0);
    alarm(0);
    signal(SIGALRM,savalrm);		/* restore alarm handler */
    signal(SIGINT,savint);		/* restore interrupt handler */
    debug(F100,"dreset alarm and signal handlers reset","",0);
}

/*
  Call this routine when the modem reports that it has connected at a certain
  speed, giving that speed as the argument.  If the connection speed is not
  the same as Kermit's current communication speed, AND the modem interface
  speed is not locked (i.e. DIAL SPEED-MATCHING is not ON), then change the
  device speed to the one given.
*/
static VOID
#ifdef CK_ANSIC
spdchg(long s)
#else
spdchg(s) long s;
#endif /* CK_ANSIC */
/* spdchg */ {
    int s2;
    if (!mdmspd)			/* If modem interface speed locked, */
      return;				/*  don't do this. */
    if (speed != s) {			/* Speeds differ? */
	s2 = s / 10L;			/* Convert to cps expressed as int */
	if (ttsspd(s2) < 0) {		/* Change speed. */
	    printf(" WARNING - speed change to %ld failed.\r\n",s);
	} else {
	    printf(" Speed changed to %ld.\r\n",s);
	    speed = s;			/* Update global speed variable */
	}
    }
}

/*
  Display all characters received from modem dialer through this routine,
  for consistent handling of carriage returns and linefeeds.
*/
static VOID
#ifdef CK_ANSIC
dialoc(char c)
#else
dialoc(c) char c;
#endif /* CK_ANSIC */
{ /* dialoc */				/* Dial Output Character */
    if (dialdpy) {
	if (c != LF) conoc(c);		/* Don't echo LF */
	if (c == CR) conoc(LF);		/* Echo CR as CRLF */
    }
}

#ifndef MINIDIAL
#ifdef OLDTBCODE
/*
  tbati3() -- Routine to find out Telebit model when ATI reports "965"
  or "971". This routine sends another query, ATI3, to get further info
  to narrow down the model number.  Argument is ATI response as integer.
  Result: sets tbmodel variable to Telebit model.
*/
static VOID
tbati3(n) int n; {
    int status;
    ttflui();				/* Flush input buffer */
    ttslow("ATI3\015",100);		/* Send ATI3<CR> */
    status = getok(5,0);		/* Get OK response, nonstrict */
    if (status < 1) {			/* ERROR or timeout */
	tbmodel = TB_UNK;
	debug(F111,"tbati3 fails",rbuf,status);
	return;
    }
    debug(F110,"tbati3 rbuf",rbuf,0);

/* Got a good response, check the model info */

    if (n == 965) {			/* "965" - various models. */
	if (didweget(rbuf,"T1600")) {
	    tbmodel = TB_1600;			/* T1600 */
	} else if (didweget(rbuf,"T3000")) {
	    tbmodel = TB_3000;			/* T3000 */
	} else if (didweget(rbuf,"World")) {
	    tbmodel = TB_WBLA;			/* WorldBlazer */
	} else if (didweget(rbuf,"Version B") || /* TrailBlazer-Plus models */
		   didweget(rbuf,"TBSA") ||
		   didweget(rbuf,"TBRM") ||
		   didweget(rbuf,"DC")) { 	/* Ven-Tel EC18K */
	    tbmodel = TB_PLUS;
	} else tbmodel = TB_UNK;		/* Others: Unknown */

    } else if (n == 971) {		/* "971" could be T1500 or T1600. */
	if (didweget(rbuf,"T1500"))
	  tbmodel = TB_1500;
	else tbmodel = TB_2500;
    }					/* Other, don't change tbmodel. */
}
#endif /* OLDTBCODE */
#endif /* MINIDIAL */

VOID				/* Get dialing defaults from environment */
getdialenv() {
    char *p = NULL;

    makestr(&p,getenv("K_DIAL_DIRECTORY")); /* Dialing directories */
    if (p) {
	int i;
	xwords(p,(MAXDDIR - 2),dialdir,0);
	for (i = 0; i < (MAXDDIR - 1); i++) {
	    if (!dialdir[i+1])
	      break;
	    else
	      dialdir[i] = dialdir[i+1];
	}
	ndialdir = i;
    }
    makestr(&diallcc,getenv("K_COUNTRYCODE")); /* My country code */
    makestr(&dialixp,getenv("K_LD_PREFIX"));   /* My long-distance prefix */
    makestr(&dialldp,getenv("K_INTL_PREFIX")); /* My international prefix */
    makestr(&dialldp,getenv("K_TF_PREFIX"));   /* Ny Toll-free prefix */

    p = NULL;
    makestr(&p,getenv("K_TF_AREACODE")); /* Toll-free areacodes */
    if (p) {
	int i;
	xwords(p,7,dialtfc,0);
	for (i = 0; i < 8; i++) {
	    if (!dialtfc[i+1])
	      break;
	    else
	      dialtfc[i] = dialtfc[i+1];
	}
	ntollfree = i;
    }
    if (diallcc) {			/* Have country code */
	if (!strcmp(diallcc,"1")) {	/* If it's 1 */
	    if (!dialldp)		/* Set these prefixes... */
	      makestr(&dialldp,"1");
	    if (!dialtfp)
	      makestr(&dialtfp,"1");
	    if (!dialixp)
	      makestr(&dialixp,"011");
	    if (ntollfree == 0) {	/* Toll-free area codes... */
		if (dialtfc[0] = malloc(4)) {
		    strcpy(dialtfc[0],"800");
		    ntollfree++;
		}
		if (dialtfc[1] = malloc(4)) {
		    strcpy(dialtfc[1],"888");
		    ntollfree++;
		}
	    }
	} else if (!strcmp(diallcc,"358") &&
		   ((int) strcmp(zzndate(),"19961011") > 0)
		   ) {			/* Finland */
	    if (!dialldp)		/* Long-distance prefix */
	      makestr(&dialldp,"9");
	    if (!dialixp) 		/* International dialing prefix */
	      makestr(&dialixp,"990");
	} else {			/* Not NANP or Finland */
	    if (!dialldp)
	      makestr(&dialldp,"0");
	    if (!dialixp)
	      makestr(&dialixp,"00");
	}
    }
    makestr(&diallac,getenv("K_AREACODE"));
    makestr(&dialpxo,getenv("K_PBX_XCH"));
    makestr(&dialpxi,getenv("K_PBX_ICP"));
    makestr(&dialpxx,getenv("K_PBX_OCP"));
}

static int
dialfail(x) int x; {
    char * s;

    fail_code = x;
    debug(F101,"ckudial dialfail","",x);
    dreset();				/* Reset alarm and signal handlers */

    printf("%s Failure: ", func_code == 0 ? "DIAL" : "ANSWER");
    if (dialdpy) {			/* If showing progress */
       debug(F100,"dial display is on","",0);
	p = ck_time();			/* get current time; */
	if (*p) printf("%s: ",p);
    }
    switch (fail_code) {		/* Type of failure */
      case F_TIME: 			/* Timeout */
	if (dial_what == DW_INIT)
	  printf ("Timed out while trying to initialize modem.\n");
	else if (dial_what == DW_DIAL)
	  printf ("%s interval expired.\n",
		  func_code == 0 ? "DIAL TIMEOUT" : "ANSWER timeout");
	else printf("Timeout.\n");
	if (mp->capas & CKD_AT)
	  ttoc('\015');		/* Send CR to interrupt dialing */
	/* Some Hayes modems don't fail with BUSY on busy lines */
	dialsta = DIA_TIMO;
	debug(F110,"dial","timeout",0);
	break;

      case F_INT:			/* Dialing interrupted */
	printf ("Interrupted.\n");
	debug(F110,"dial","interrupted",0);
	if (mp->capas & CKD_AT)
	  ttoc('\015');			/* Send CR to interrupt dialing */
	dialsta = DIA_INTR;
	break;

    case F_MODEM:			/* Modem detected a failure */
         debug(F111,"dialfail()","lbuf",lbuf);
         if (lbuf && *lbuf) {
            printf(" \"");
            for (s = lbuf; *s; s++)
               if (isprint(*s))
                  putchar(*s);		/* Display printable reason */
            printf ("\"");
         } else printf(func_code == 0 ?
                        " Call not completed." :
                        " Call did not come in."
                        );
	printf("\n");
	debug(F110,"dial",lbuf?lbuf:"",0);
	if (dialsta < 0) dialsta = DIA_UNSP;
	break;

      case F_MINIT:			/* Failure to initialize modem */
	printf ("Error initializing modem.\n");
	debug(F110,"dial","modem init",0);
	dialsta = DIA_NOIN;
	break;

    default:
	printf("unknown\n");
	debug(F110,"dial","unknown",0);
	if (mp->capas & CKD_AT)
	  ttoc('\015');			/* Send CR to interrupt dialing */
	dialsta = DIA_INTR;
    }

#ifdef DYNAMIC
    if (lbuf) free(lbuf); lbuf = NULL;
    if (rbuf) free(rbuf); rbuf = NULL;
    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */

    if (dialsta < 0) dialsta = DIA_UERR; /* Set failure code */
    return(0);				/* Return zero (important) */
}

/*  C K D I A L	 --  Dial up the remote system */

/* Returns 1 if call completed, 0 otherwise */

static int waitct, mdmwait, mdmstat = 0;
int mdmwaitd = 10 ;   /* difference between dialtmo and mdmwait */
static char c;
static char *telnbr;

static SIGTYP
#ifdef CK_ANSIC
_dodial(void * threadinfo)
#else /* CK_ANSIC */
_dodial(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* _dodial */ {
    char c2;
    char *s, *ws;
    int x = 0, n = F_TIME;

#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
    }
#endif /* NTSIG */

    /* Hang up the modem (in case it wasn't "on hook") */
    /* But only if SET DIAL HANGUP ON... */

    if (dialhup() < 0) {		/* Hangup first */
	debug(F100,"ckdial dialhup failed","",0);
#ifndef MINIDIAL
	if (mdmcapas & CKD_TB)		/* Telebits might need a BREAK */
	  ttsndb();			/*  first. */
#endif /* MINIDIAL */
	if (dialhng && dialsta != DIA_PART) { /* If hangup failed, */
	    ttclos(0);			/* close and reopen the device. */
	    if (ttopen(ttname,&local,mymdmtyp,0) < 0) {
		printf("Sorry, Can't hang up communication device.\n");
		printf("Try 'set line %s' again.\n",ttname);
		dialsta = DIA_HANG;
#ifdef DYNAMIC
		if (lbuf) free(lbuf); lbuf = NULL;
		if (rbuf) free(rbuf); rbuf = NULL;
		if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
		dreset();
#ifdef NTSIG
		ckThreadEnd(threadinfo);
#endif /* NTSIG */
		SIGRETURN;
	    }
	}
    }
#ifndef MINIDIAL
    /* Don't start talking to Rolm too soon */
    if (mymdmtyp == n_ROLM && dialsta != DIA_PART)
      msleep(500);
#endif /* MINIDIAL */

/* Send init-string */

    if (dialsta != DIA_PART
#ifndef MINIDIAL
	&& mymdmtyp != n_ATTUPC
#endif /* MINIDIAL */
	) {
	fail_code = F_MINIT;		/* Default failure code */
	dial_what = DW_INIT;		/* What I'm Doing Now   */
	if (dialdpy) {			/* If showing progress, */
	    p = ck_time();		/* display timestamp.   */
	    if (*p) printf(" Initializing: %s...\n",p);
	}
    }

#ifndef MINIDIAL
#ifdef ATT7300
    if (mymdmtyp == n_ATTUPC) {
/*
  For ATT7300/Unix PC's with their special internal modem.  Whole dialing
  process is handled right here, an exception to the normal structure.
  Timeout and user interrupts are enabled during dialing.  attdial() is in
  file ckutio.c.  - jrd
*/
        _PROTOTYP( int attdial, (char *, long, char *) );
	fail_code = F_MODEM;		/* Default failure code */
	dial_what = DW_DIAL;
	if (dialdpy) {			/* If showing progress */
	    p = ck_time();		/* get current time; */
	    if (*p) printf(" Dialing: %s...\n",p);
	}
	alarm(waitct);			/* Do alarm properly */
	if (attdial(ttname,speed,telnbr)) { /* dial internal modem */
	    dreset();			/* reset alarms, etc. */
	    printf(" Call failed.\r\n");
	    dialhup();	        	/* Hangup the call */
#ifdef DYNAMIC
	    if (lbuf) free(lbuf); lbuf = NULL;
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    dialsta = DIA_UERR;
#ifdef NTSIG
	    ckThreadEnd(threadinfo);
#endif /* NTSIG */
	    SIGRETURN;			/* return failure */
	}
	dreset();			/* reset alarms, etc. */
	ttpkt(speed,FLO_DIAX,parity);	/* cancel dialing ioctl */
	if (!quiet && !backgrd) {
	    if (dialdpy) printf("\n");
	    printf(" Call complete.\r\n");
	}
	dialsta = DIA_OK;
#ifdef DYNAMIC
	if (lbuf) free(lbuf); lbuf = NULL;
	if (rbuf) free(rbuf); rbuf = NULL;
	if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;	/* No conversation with modem to complete dialing */
    } else
#endif /* ATT7300 */
    if (mymdmtyp == n_TAPI) {		/* Windows dialer */
	printf("INSERT CALL TO WINDOWS TELEPHONY API HERE\n");
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;
    } else
#endif /* MINIDIAL */

/* Modems with AT command set... */

      if ((mdmcapas & CKD_AT) && dialsta != DIA_PART) {

	  if (dialini)			/* Get wakeup/init string */
	    ws = dialini;
	  else
	    ws = mp->wake_str;
	  if (!ws) ws = "\015";		/* If none, use CR */

	  for (tries = 4; tries > 0; tries--) {	/* Send it */
	      ttslow(ws,mp->wake_rate);
	      mdmstat = getok(4,1);	/* Get response */
	      if (mdmstat > 0) break;
	      if (dialdpy && tries > 1)
		printf(" No response from modem, retrying%s...\n",
		       (tries < 4) ? " again" : "");
	  }
	  debug(F101,"ckdial wake_str mdmstat","",mdmstat);

	  if (mdmstat < 1) { 		/* Initialized OK? */
	      dialfail(F_MINIT); /* No, fail. */
#ifdef NTSIG
	      ckThreadEnd(threadinfo);
#endif /* NTSIG */
	      SIGRETURN;
	  } else {			/* Yes. */
	      char hbuf[16];

	      if (mdmwait > 255)	/* If larger than maximum, */
		mdmwait = 255;		/* make it maximum. */

	      sprintf(hbuf,"ATS7=%d%c",mdmwait,13); /* S7 carrier wait time */
	      ttslow(hbuf,mp->wake_rate); /* Set it. */
	      mdmstat = getok(4,1);	/* Get response from modem */
	      /* If it gets an error, go ahead anyway */
	      debug(F101,"ckdial S7 mdmstat","",mdmstat);
	  }

#ifndef MINIDIAL
#ifdef OLDTBCODE
/*
  Telebit modems fall into two basic groups: old and new.  The functions and
  command sets are different between the two groups, and also vary by specific
  models within each group, and even by firmware ROM revision number.  Read
  ckcker.bwr for details.

  Commands used by C-Kermit include:

    Old       New            Meaning
    -------   --------       ----------------------------------------
    Q0        Q0             Enable result codes.
    X1        X1             Extended result codes.
    X1        X1             Extended result codes + BUSY, NO DIALTONE, etc.
    I         I              Model number inquiry.
    I3        I3             Additional model information inquiry.
    S12=50    S12=50         Escape sequence guard time (1 sec).
    S2=43     S2=43          Escape character is '+'.
    S7=xx     S7=xx          DIAL TIMEOUT, calculated or SET by user.
    S48=0     S48=0          7-bit data (Kermit's PARITY is not NONE).
    S48=1     S48=1          8-bit data (Kermit's PARITY is NONE).
    S50=0     S50=0          Automatic speed & protocol determination.
    S50=3     S50=3          2400/1200/300 bps.
    S50=6     S50=6          V.32 (9600 bps).
    S50=255   S50=255        PEP mode.
    S110=1    S190=1 S191=7  Allow compression in PEP mode.
    S51=?     S51=?          DTE interface speed (left alone by Kermit).
    S54=3     S61=0 S63=0    Pass BREAK signal (always).
    S58=2     S58=2          RTS/CTS flow control if Kermit's FLOW is RTS/CTS.
    S58=?     S58=?          S58 unchanged if Kermit's FLOW is not RTS/CTS.
    S68=255   S68=255        Use flow control specified by S58 (always).
    S95=0     S180=0         MNP disabled (SET DIAL MNP-ENABLE OFF)
    S95=2     S180=3         MNP, fallback to direct (also as V.42 fallback)
    S97=1     S180=2         Enable V.42 (LAPM) error correction
    S98=3                    Enable compression in both directions
    S106=1                   V.42bis compression enable

For Kermit Spoof (same commands for all models that support it):

    S111=0                   No Kermit spoofing
    S111=10                  Kermit with no parity
    S111=11                  Kermit with odd parity
    S111=12                  Kermit with even parity
    S111=13                  Kermit with mark parity
    S111=14                  Kermit with space parity
    S112=??                  Kermit's start-of-packet character (stchr).
*/
	  if (mdmcapas & CKD_TB) { /* Telebits... */

	      int S111;			/* Telebit Kermit spoof register */
	      char tbcmdbuf[64];	/* Telebit modem command buffer */
	      char *ecstr = "";		/* Pointer to EC-enable string */
	      char *dprstr = "";	/* Pointer to dial protocol string */
/*
  If user defined a DIAL INIT-STRING, send that now, otherwise send built-in
  Telebit string.  Try up to 4 times to get OK or 0 response from modem.
  NOTE: The default init string *must* be independent of Telebit model.
*/
	      ws = dialini ? dialini : TELEBIT.wake_str;
	      debug(F110,"ckdial telebit init string",ws,0);
	      for (tries = 4; tries > 0; tries--) {
		  ttsndb();		/* Begin by sending BREAK */
		  ttslow(ws,mp->wake_rate); /* Send wakeup string */
		  mdmstat = getok(5,0);	/* Get modem's response */
		  if (mdmstat) break;	/* If response OK, done */
		  if (dialdpy && tries > 1)
		    printf(" No response from modem, retrying%s...\n",
			   (tries < 4) ? " again" : "");
		  msleep(300);		/* Otherwise, sleep 1/3 second */
		  dialhup();		/* Hang up */
		  ttflui();		/* Flush input buffer and try again */
	      }
	      if (mdmstat < 1) {	/* If we didn't get a response, */
		  dialfail(F_MINIT); /* fail. */
#ifdef NTSIG
		  ckThreadEnd(threadinfo);
#endif /* NTSIG */
		  SIGRETURN;
	      }
	      if (!dialini) {		/* If using built-in init strings... */
/*
  Try to get the model number.  It should be in the getok() response buffer,
  rbuf[], because the Telebit init string asks for it with the "I" command.
  If the model number is 965, we have to make another query to narrow it down.
*/
		  if (didweget(rbuf,"962") || /* Check model number */
		      didweget(rbuf,"961") ||
		      didweget(rbuf,"963")) {
		      tbmodel = TB_BLAZ; /* Trailblazer */
		  } else if (didweget(rbuf,"972")) {
		      tbmodel = TB_2500; /* T2500 */
		  } else if (didweget(rbuf,"968")) {
		      tbmodel = TB_1000;	/* T1000 */
		  } else if (didweget(rbuf,"966") ||
			     didweget(rbuf,"967") ||
			     didweget(rbuf,"964")) {
		      tbmodel = TB_PLUS; /* Trailblazer-Plus */
		  } else if (didweget(rbuf,"969")) {
		      tbmodel = TB_QBLA; /* Qblazer */
		  } else if (didweget(rbuf,"970")) {
		      tbmodel = TB_QBLA; /* Qblazer Plus */
		  } else if (didweget(rbuf,"965")) { /* Most new models */
		      tbati3(965);	/* Go find out */
		  } else if (didweget(rbuf,"971")) { /* T1500 or T2500 */
		      tbati3(971);	/* Go find out */
		  } else if (didweget(rbuf,"123") || didweget(rbuf,"960")) {
		      tbmodel = TB_UNK;	/* Telebit in Hayes mode */
		  }
		  debug(F111,"Telebit model",tb_name[tbmodel],tbmodel);
		  if (dialdpy)
		    printf("Telebit model: %s\n",tb_name[tbmodel]);
		  ttflui();
/*
  Flow control.  If C-Kermit's FLOW-CONTROL is RTS/CTS, then we set this on
  the modem too.  Unfortunately, many versions of UNIX only allow RTS/CTS
  to be set outside of Kermit (e.g. by selecting a special device name).
  In that case, Kermit doesn't know that it should set RTS/CTS on the modem,
  in which case the user SET MODEM FLOW appropriately.
*/
		  if (flow == FLO_RTSC) { /* RTS/CTS active in Kermit */
		      sprintf(tbcmdbuf,
			      "ATS7=%d S48=%d S50=0 S58=2 S68=255\015",
			      mdmwait, parity ? 0 : 1);
		  } else
		    sprintf(tbcmdbuf,	/* Otherwise, don't touch modem's fc */
			    "ATS7=%d S48=%d S50=0 S68=255\015",
			    mdmwait, parity ? 0 : 1);
		  s = tbcmdbuf;
		  debug(F110,"ckdial Telebit init step 2",s,0);
		  for (tries = 4; tries > 0; tries--) {
		      ttslow(s,mp->wake_rate);
		      mdmstat = getok(5,1);
		      if (mdmstat) break;
		      if (dialdpy && tries > 1)
			printf(" No response from modem, retrying%s...\n",
			       (tries < 4) ? " again" : "");
		      msleep(500);
		      ttflui();
		  }
		  if (mdmstat < 1) {
		      dialfail(F_MINIT);
#ifdef NTSIG
		      ckThreadEnd(threadinfo);
#endif /* NTSIG */
		      SIGRETURN;
		  }
/*
  Model-dependent items, but constant per model.
*/
		  switch (tbmodel) {
		    case TB_BLAZ:
		    case TB_PLUS:	/* TrailBlazer-Plus */
		    case TB_1000:	/* T1000 */
		    case TB_2000:	/* T2000 */
		    case TB_2500:	/* T2500 */
#ifdef COMMENT
/* Code from edit 183 told modem to follow RS-232 wrt CD and DTR */
		      /* DTR, CD, follow RS-232, pass BREAK */
		      sprintf(tbcmdbuf,"ATS52=1 S53=4 S54=3\015");
#else
/* But everybody agreed we should not touch modem's CD and DTR settings. */
		      /* Just pass BREAK */
		      sprintf(tbcmdbuf,"ATS54=3\015");
#endif /* COMMENT */
		      break;
		    case TB_1600:	/* T1600 */
		    case TB_3000:	/* T3000 */
		    case TB_WBLA:	/* WorldBlazer */
		    case TB_QBLA:	/* Qblazer */
#ifdef COMMENT
/* Code from edit 183 */
		      /* Follow RS-232, No CONNECT suffix, pass BREAK */
		      sprintf(tbcmdbuf,"AT&C1&D2&Q0 S59=0 S61=0 S63=0\015");
#else
/*
  Everybody agrees we should not touch modem's CD and DTR settings.
  Also no more &Q0, no more S59=0 (doesn't matter, so don't touch).
  So this section now deals only with treatment of BREAK.
  Here we also raise the result code from X1 to X2, which allows
  the T1600, T3000, and WB to supply NO DIALTONE, BUSY, RRING, and DIALING.
  X2 means something else on the other models.
*/
		      /* Transmit BREAK in sequence, raise result code. */
		      sprintf(tbcmdbuf,"ATX2 S61=0 S63=0\015");
#endif /* COMMENT */
		      break;
		    default:		/* Others, do nothing */
		      tbcmdbuf[0] = NUL;
		      break;
		  }
		  s = tbcmdbuf;
		  if (*s) {
		      debug(F110,"ckdial Telebit init step 3",s,0);
		      for (tries = 4; tries > 0; tries--) {
			  ttslow(s,mp->wake_rate);
			  mdmstat = getok(5,1);
			  if (mdmstat) break;
			  if (dialdpy && tries > 1)
			    printf(" No response from modem, retrying%s...\n",
				   (tries < 4) ? " again" : "");
			  msleep(500);
			  ttflui();
		      }
		      if (mdmstat < 1)
			dialfail(F_MINIT);
		  } else debug(F100,"ckdial Telebit init step 3 skipped","",0);

/* Error correction, MNP or V.42 */

		  if (dialec) {		/* User wants error correction */
		      switch (tbmodel) { /* which implies fallback to MNP. */
			case TB_PLUS:	/* BC7.00 and up firmware */
			case TB_2000:	/* now really the same as TB+ ? */
			case TB_2500:	/* LAPM+compress->MNP->direct */
			  ecstr = "S50=0 S95=2 S97=1 S98=3 S106=1";
			  break;
			case TB_1600:
			case TB_3000:
			case TB_WBLA:
			case TB_QBLA:
#ifdef COMMENT
			  /* V.42, fallback = lock speed */
			  ecstr = "S180=2 S181=0";
#else
/* Better not to mess with S181, let it be used however user has it set. */
/* S180=2 allows fallback to MNP, S180=1 disallows fallback to MNP. */
			  ecstr = "S180=2";	/* V.42 */
#endif /* COMMENT */
			  break;
			default:
			  if (dialdpy)
			    printf(
"V.42 not supported by this Telebit model\n");
		      }
		  } else {		/* Handle DIAL ERROR-CORRE.. setting */
		      switch (tbmodel) {
			case TB_BLAZ:	/* TrailBlazer */
			case TB_PLUS:	/* TrailBlazer-Plus */
			case TB_1000:	/* T1000 */
			case TB_2000:	/* T2000 */
			case TB_2500:	/* T2500 */
			  ecstr = dialec ? "S95=2" : "S95=0"; /* ON, OFF */
			  break;
			case TB_1600:	/* T1600 */
			case TB_3000:	/* T3000 */
			case TB_WBLA:	/* WorldBlazer */
			case TB_QBLA:	/* Qblazer */
			  ecstr = dialec ? "S180=3" : "S180=0"; /* ON, OFF */
			  /* (Leave S181 fallback method alone) */
			  break;
			default:
			  ecstr = "";
		      }
		  }

/* Dialing protocol */

		  dprstr = "";	/* Initialize dialing protocol string */
		  p = "";		/* and message string */
		  switch (mymdmtyp) {
		    case n_TELEBIT:	/* Start at highest and work down */
		      p = "standard";
		      switch (tbmodel) { /* First group starts with PEP */
			case TB_BLAZ:	/* TrailBlazer */
			case TB_PLUS:	/* TrailBlazer-Plus */
			case TB_1000:	/* T1000 */
			case TB_2000:	/* T2000 */
			case TB_2500:	/* T2500 */
			  dprstr = "S50=0 S110=1"; /* PEP, compression. */
			  break;
			case TB_WBLA:	/* WorldBlazer has PEP */
			  dprstr = "S50=0 S190=1 S191=7"; /* PEP, */
			  break;	/* compression allowed. */
			case TB_1600:	/* T1600 doesn't have PEP */
			case TB_3000:	/* T3000 doesn't */
			case TB_QBLA:	/* Qblazer doesn't*/
			default:
			  dprstr = "S50=0"; /* No PEP available */
			  break;
		      }
		      break;

#ifdef COMMENT
/* Who needs it? */
		    case n_TBS:		/* Telebit up to 2400 Baud */
		      p = "300/1200/2400 Baud"; /* Leave S90 alone assuming */
		      dprstr = "S50=3";	/* already set for V.22 vs 212A */
		      break;
		    case n_TB3:		/* Telebit V.32 */
		      if (tbmodel == TB_3000 || tbmodel == TB_1600 ||
			  tbmodel == TB_2500 || tbmodel == TB_WBLA) {
			  p = "V.32";
		    /* Note: we don't touch S51 (interface speed) here. */
		    /* We're already talking to the modem, and the modem */
		    /* SHOULD be able to make a V.32 call no matter what */
		    /* its interface speed is.  (In practice, however, */
		    /* that is not always true.) */
			  dprstr = "S50=6";
		      } else if (dialdpy)
			printf("V.32 not supported by this Telebit model.\n");
		      break;

		    case n_TBPEP:	/* Force PEP Protocol */
		      /* Models that don't support PEP */
		      if (tbmodel != TB_1600 &&
			  tbmodel != TB_3000 &&
			  tbmodel != TB_QBLA) {
			  p = "PEP";
			  if (tbmodel == TB_WBLA) /* WorldBlazer */
			    dprstr = "S50=255 S190=1 S191=7";
			  else if (tbmodel != TB_1000)
			    dprstr = "S50=255 S110=1"; /* TrailBlazer, etc. */
			  else dprstr = "S50=255"; /* T1000, no compression */
		      } else if (dialdpy)
			printf("PEP not supported by this Telebit model.\n");
		      break;
#endif /* COMMENT */
		  }

		  /* Telebit Kermit Spoof */

		  if (dialksp) {
		      p = "Kermit Spoof";
		      switch (parity) {	/* S111 value depends on parity */
			case 'e': S111 = 12; break;
			case 'm': S111 = 13; break;
			case 'o': S111 = 11; break;
			case 's': S111 = 14; break;
			case 0:
			default:  S111 = 10; break;
		      }
		      if (tbmodel != TB_QBLA)
			sprintf(tbcmdbuf,"AT%s %s S111=%d S112=%d\015",
				ecstr,dprstr,S111,stchr);
		      else {		/* Qblazer has no Kermit spoof */
			  sprintf(tbcmdbuf,"AT%s %s\015", ecstr,dprstr);
			  p = "No Kermit Spoof";
			  if (dialdpy)
			    printf("Kermit Spoof not supported by Qblazer\n");
		      }
		  } else {		/* KERMIT-SPOOF OFF */
		      p = "No Kermit Spoof";
		      sprintf(tbcmdbuf,"AT%s %s %s\015",
			      ecstr, dprstr,
			      (tbmodel == TB_QBLA) ? "" : "S111=0");
		  }
		  s = tbcmdbuf;
		  debug(F111,"ckdial Telebit config",p,speed);
		  debug(F110,"ckdial Telebit init step 4",s,0);
		  if (*s) {
		      for (tries = 4; tries > 0; tries--) {
			  ttslow(s,mp->wake_rate);
			  mdmstat = getok(5,1);
			  if (mdmstat) break;
			  if (dialdpy && tries > 1)
			    printf(" No response from modem, retrying%s...\n",
				   (tries < 4) ? " again" : "");
			  msleep(500);
			  ttflui();
		      }
		      debug(F101,"ckdial telebit init mdmstat","",mdmstat);
		      if (mdmstat < 1)
			dialfail(F_MINIT);
		  }
	      }
	      /* Done with Telebit protocols, remove bits from modem type */
	      /* Except nonverbal bit */
	      debug(F101,"ckdial Telebit mymdmtyp","",mymdmtyp);
	  }
#endif /* OLDTBCODE */

    } else if (mymdmtyp == n_ATTDTDM && dialsta != DIA_PART) { /* AT&T ... */
	ttsndb();			/* Send BREAK */
#endif /* MINIDIAL */

    } else if ( dialsta != DIA_PART ) { /* All others */



	/* Place modem into command mode */

	ws = dialini ? dialini : mp->wake_str;
	if (ws && (int)strlen(ws) > 0) {
	    debug(F111,"ckdial default, wake string", ws, mp->wake_rate);
	    ttslow(ws, mp->wake_rate);
	} else debug(F100,"ckdial no wake_str","",0);
	if (mp->wake_prompt && (int)strlen(mp->wake_prompt) > 0) {
	    debug(F110,"ckdial default, waiting for wake_prompt",
		  mp->wake_prompt,0);
	    alarm(10);
	    waitfor(mp->wake_prompt);
	    alarm(0);
	} else debug(F100,"ckdial no wake_prompt","",0);
    }
    if (dialsta != DIA_PART) {
	alarm(0);			/* Turn off alarm */
	debug(F100,"ckdial got wake prompt","",0);
	msleep(500);			/* Allow settling time */
    }
/* Handle error correction, data compression, and flow control... */

    if ( dialsta != DIA_PART
#ifndef MINIDIAL
#ifdef OLDTBCODE
	&& !(mdmcapas & CKD_TB)		/* Telebits already done. */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
	) {

	/* Enable/disable error-correction */

	x = 0;
	if (dialec) {			/* DIAL ERROR-CORRECTION is ON */
	    if (dialecon) {		/* SET DIAL STRING ERROR-CORRECTION */
		x = 1;
		ttslow(dialecon, mp->wake_rate);		
	    } else if (mdmcapas & CKD_EC) {
		x = 1;
		ttslow(mp->ec_on_str, mp->wake_rate);
	    } else printf(
		  "WARNING - I don't know how to turn on EC for this modem\n"
		     );
	} else {
	    if (dialecoff) {		/* SET DIAL STRING */
		x = 1;
		ttslow(dialecoff, mp->wake_rate);		
	    } else if (mdmcapas & CKD_EC) { /* Or built-in one... */
		x = 1;
		ttslow(mp->ec_off_str, mp->wake_rate);
	    }
#ifdef COMMENT
	    else printf(
		  "WARNING - I don't know how to turn off EC for this modem\n"
		     );
#endif /* COMMENT */
	}
	debug(F101,"ckudia xx_ok","",xx_ok);
	if (x && xx_ok) {			/* Look for OK response */
	    debug(F100,"ckudia calling xx_ok for EC","",0);
	    x = (*xx_ok)(5,1);
	    debug(F101,"ckudia xx_ok","",x);
	    if (x < 0) {
		printf("WARNING - Trouble enabling error-correction.\n");
		printf(
" Likely cause: Your modem is an RPI model, which does not have built-in\n");
		printf(" error correction and data compression.");
	    }
	}

	/* Enable/disable data compression */

	if (x > 0) x = 0;
	if (dialdc) {
	    if (x < 0 || !dialec) {
		printf(
"WARNING - You can't have compression without error correction.\n");
	    } else if (dialdcon) {	/* SET DIAL STRING ... */
		x = 1;
		ttslow(dialdcon, mp->wake_rate);		
	    } else if (mdmcapas & CKD_DC) {
		x = 1;
		ttslow(mp->dc_on_str, mp->wake_rate);
	    } else printf(
		  "WARNING - I don't know how to turn on DC for this modem\n"
			  );
	} else {
	    if (dialdcoff) {		/* SET DIAL STRING */
		x = 1;
		ttslow(dialdcoff, mp->wake_rate);		
	    } else if (mdmcapas & CKD_DC) { /* Or built-in one... */
		x = 1;
		ttslow(mp->dc_off_str, mp->wake_rate);
	    }
#ifdef COMMENT
	    else printf(
"WARNING - I don't know how to turn off compression for this modem\n"
			  );
#endif /* COMMENT */
	}
	if (x && xx_ok) {			/* Look for OK response */
	    x = (*xx_ok)(5,1);
	    if (x < 0) printf("WARNING - Trouble enabling compression\n");
	}

/* Flow control */

	x = 0;				/* User said SET DIAL FLOW RTS/CTS */
	if ( dialfc == FLO_RTSC ||	/* Even if Kermit's FLOW isn't...  */
	    (dialfc == FLO_AUTO && flow == FLO_RTSC)) {	
	    if (dialhwfc) {		/* User-defined HWFC string */
		x = 1;
		ttslow(dialhwfc, mp->wake_rate);
	    } else if (mdmcapas & CKD_HW) { /* or built-in one */
		x = 1;
		ttslow(mp->hwfc_str, mp->wake_rate);
	    } else
	      printf("WARNING - I don't know how to enable modem's HWFC.\n");

	} else if ( dialfc == FLO_XONX || /* User said SET DIAL FLOW SOFT */
		   (dialfc == FLO_AUTO && flow == FLO_XONX)) {
	    if (dialswfc) {
		x = 1;
		ttslow(dialswfc, mp->wake_rate);
	    } else if (mdmcapas & CKD_SW) {
		x = 1;
		ttslow(mp->swfc_str, mp->wake_rate);
	    }

	} else if (dialfc == FLO_NONE) { /* User said SET DIAL FLOW NONE */
	    if (dialnofc) {
		x = 1;
		ttslow(dialnofc, mp->wake_rate);
	    } else if (mp->nofc_str && *(mp->nofc_str)) {
		x = 1;
		ttslow(mp->nofc_str, mp->wake_rate);
	    }
	}
	if (x && xx_ok) {		/* Get modem's response */
	    x = (*xx_ok)(5,1);
	    if (x < 0)
	     printf("WARNING - Trouble %sabling modem's local flow control\n",
		    (dialfc == FLO_NONE) ? "dis" : "en");
#ifdef CK_TTSETFLOW
#ifdef CK_RTSCTS
/*
  So far only ckutio.c has ttsetflow().
  We have just told the modem to turn on RTS/CTS flow control and the modem
  has said OK.  But we ourselves have not turned it on yet because of the
  disgusting ttpkt(...FLO_DIAL...) hack.  So now, if the computer does not
  happen to be asserting RTS, the modem will no longer send characters to it.
  So at EXACTLY THIS POINT, we must enable RTS/CTS in the device driver.
*/
	    if (dialfc == FLO_RTSC ||
		(dialfc == FLO_AUTO && flow == FLO_RTSC))
	      ttsetflow(FLO_RTSC);
#endif /* CK_RTSCTS */
#endif /* CK_TTSETFLOW */
	}
    }

#ifndef MINIDIAL
    if (mdmcapas & CKD_KS && dialsta != DIA_PART) { /* Kermit spoof */
	int r;				/* Register */
	char tbcmdbuf[20];		/* Command buffer */
	switch (mymdmtyp) {
  
	  case n_MICROCOM:		/* Microcoms in SX mode */
  	    if (dialksp)
	      sprintf(tbcmdbuf,"APM1;KMC%d\015",stchr);
	    else
	      sprintf(tbcmdbuf,"APM0\015");
  	    ttslow(tbcmdbuf, MICROCOM.wake_rate);
  	    alarm(3);
	    waitfor(mp->wake_prompt);
	    alarm(0);
	    break;

	  case n_TELEBIT:		/* Old and new Telebits */
	  case n_TBNEW:
	    if (!dialksp) {
		sprintf(tbcmdbuf,"ATS111=0\015");
	    } else {
		switch (parity) {	/* S111 value depends on parity */
		  case 'e': r = 12; break;
		  case 'm': r = 13; break;
		  case 'o': r = 11; break;
		  case 's': r = 14; break;
		  case 0:
		  default:  r = 10; break;
		}
		sprintf(tbcmdbuf,"ATS111=%d S112=%d\015",r,stchr);
	    }
	    ttslow(tbcmdbuf, mp->wake_rate);

/* Not all Telebit models have the Kermit spoof, so ignore response. */

	    if (xx_ok) {		/* Get modem's response */
		x = (*xx_ok)(5,1);
#ifdef COMMENT
		if (x < 0)
		  printf("WARNING - Trouble %sabling Kermit spoof\n",
			 (dialksp == FLO_NONE) ? "dis" : "en");
#endif /* COMMENT */
	    }
	}
    }
#endif /* MINIDIAL */

    if (dialmth && dialsta != DIA_PART) { /* If dialing method specified... */
	char *s = "";			/* Do it here... */

	if (dialmth == XYDM_T)		/* Tone */
	  s = dialtone ? dialtone : mp->tone;
	else if (dialmth == XYDM_P)	/* Pulse */
	  s = dialpulse ? dialpulse : mp->pulse;
	if (s) if (*s) {
	    ttslow(s, mp->dial_rate);
	    if (xx_ok)		/* Get modem's response */
	      (*xx_ok)(5,1);	/* (but ignore it...) */
	}
    }
    if (func_code == 1) {		/* ANSWER (not DIAL) */
	char *s;
	s = dialaaon ? dialaaon : mp->aa_on_str;
	if (!s) s = "";
	if (*s) {
	    ttslow(s, mp->dial_rate);
	    if (xx_ok)			/* Get modem's response */
	      (*xx_ok)(5,1);		/* (but ignore it...) */
	} else {
	    printf(
"WARNING - I don't know how to enable autoanswer for this modem.\n"
		   );
	} /* And skip all the phone-number & dialing stuff... */
	alarm(waitct);			/* This much time allowed. */
	debug(F101,"ckdial ANSWER waitct","",waitct);

    } else {				/* DIAL (not ANSWER) */

	if (dialsta != DIA_PART) {	/* Last dial was not partial */

	    char *s = "";
#ifdef COMMENT
	    s = dialaaoff ? dialaaoff : mp->aa_off_str;
#endif /* COMMENT */
	    if (s) if (*s) {
		ttslow(s, mp->dial_rate);
		if (xx_ok)		/* Get modem's response */
		  (*xx_ok)(5,1);	/* (but ignore it...) */
	    }

	    /* Put modem into dialing mode, if the modem requires it. */

	    if (mp->dmode_str && *(mp->dmode_str)) {
		ttslow(mp->dmode_str, mp->dial_rate);
		savalrm = signal(SIGALRM,dialtime);
		alarm(10);
		/* Wait for prompt, if any expected */
		if (mp->dmode_prompt && *(mp->dmode_prompt)) {
		    waitfor(mp->dmode_prompt);
		    msleep(300);
		}
		alarm(0);		/* Turn off alarm on dialing prompts */
		signal(SIGALRM,savalrm); /* Restore alarm */
		ttflui(); /* Clear out stuff from waking modem up */
	    }
	}

/* Allocate a buffer for the dialing string. */

#ifdef DYNAMIC
	if (!lbuf) {			/* If, for some reason, this is NULL */
	    if (!(lbuf = malloc(LBUFL+1))) { /* allocate it... */
		dialsta = DIA_IE;
#ifdef NTSIG
		ckThreadEnd(threadinfo);
#endif /* NTSIG */
		SIGRETURN;
	    }
	}
#endif /* DYNAMIC */

	if (mdmcapas & CKD_AT && dialsta != DIA_PART) {
	    sprintf(lbuf,"ATS2=%d\015",	/* Set the escape character */
		    dialesc ? dialesc : mp->esc_char);
	    ttslow(lbuf, mp->dial_rate);
	    if (xx_ok)			/* Get modem's response */
	      x = (*xx_ok)(5,1);
	    if (x < 0)
	      printf(
		     "WARNING - Problem setting modem's escape character\n"
		     );
	}
	s = dialcmd ? dialcmd : mp->dial_str;

	if ((int)strlen(s) + (int)strlen(telnbr) > LBUFL) {
	    printf("DIAL command + phone number too long!\n");
	    dreset();
#ifdef DYNAMIC
	    if (lbuf) free(lbuf); lbuf = NULL;
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
#ifdef NTSIG
	    ckThreadEnd(threadinfo);
#endif /* NTSIG */
	    SIGRETURN;	 /* No conversation with modem to complete dialing */
	}

	sprintf(lbuf, s, telnbr);

	debug(F110,"dialing",lbuf,0);
	ttslow(lbuf,mp->dial_rate);	/* Send the dialing string */

	fail_code = F_MODEM;		/* New default failure code changes */
	dial_what = DW_DIAL;		/* and our state, too. */
	if (dialdpy) {			/* If showing progress */
	    p = ck_time();		/* get current time; */
	    if (*p) printf(" Dialing: %s...\n",p);
	}
	alarm(waitct);			/* This much time allowed. */
	debug(F101,"ckdial waitct","",waitct);

#ifndef MINIDIAL
#ifdef OLDMODEMS
	switch (mymdmtyp) {
	  case n_RACAL:			/* Acknowledge dialing string */
	    sleep(3);
	    ttflui();
	    ttoc('\015');
	    break;
	  case n_VENTEL:
	    waitfor("\012\012");	/* Ignore the first two strings */
	    break;
	  default:
	    break;
	}
#endif /* OLDMODEMS */
#endif /* MINIDIAL */
    }

/* Check for connection */

    mdmstat = 0;			/* No status yet */
    strcpy(lbuf,"");			/* Default reason for failure */
    debug(F101,"dial awaiting response, mymdmtyp","",mymdmtyp);

#ifndef NOSPL
    modemmsg[0] = NUL;
#endif /* NOSPL */
    while (mdmstat == 0) {		/* Till we get a result or time out */

	if ((mdmcapas & CKD_AT) && nonverbal) { /* AT command set */
	    gethrn();			/* In digit result mode */
	    if (partial && dialsta == DIA_ERR) {
		/*
		   If we get an error here, the phone is still
		   off hook so we have to hang it up.
		*/
		dialhup();
		dialsta = DIA_ERR;	/* (because dialhup() changes it) */
	    }
	    continue;

	} else if (mymdmtyp == n_UNKNOWN) { /* Unknown modem type */
	    int x, y = waitct;
	    mdmstat = D_FAILED;		/* Assume failure. */
	    while (y-- > -1) {
		x = ttchk();
		if (x > 0) {
		    if (x > LBUFL) x = LBUFL;
		    x = ttxin(x,(CHAR *)lbuf);
		    if ((x > 0) && dialdpy) conol(lbuf);
		} else if (network && x < 0) { /* Connection dropped */
#ifdef NTSIG
		    ckThreadEnd(threadinfo);
#endif /* NTSIG */
		    dialsta = DIA_IO;	/* Call it an I/O error */
#ifdef DYNAMIC
		    if (lbuf) free(lbuf); lbuf = NULL;
		    if (rbuf) free(rbuf); rbuf = NULL;
		    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
		    SIGRETURN;
		}
		x = ttgmdm();		/* Try to read modem signals */
		if (x < 0) break;	/* Can't, fail. */
		if (x & BM_DCD) {	/* Got signals OK.  Carrier present? */
		    mdmstat = CONNECTED; /* Yes, done. */
		    break;
		}			/* No, keep waiting. */
		sleep(1);
	    }
	    continue;
	}

	for (n = -1; n < LBUFL-1; ) {	/* Accumulate modem response */
	    int xx;
	    c2 = (char) (xx = ddinc(0)); /* Read a character, blocking */
	    if (xx < 1)			/* Ignore NULs and errors */
	      continue;			/* (Timeout will handle errors) */
	    else			/* Real character, keep it */
	      lbuf[++n] = (char) (c2 & 0177);
	    dialoc(lbuf[n]);		/* Maybe echo it  */
	    if (mdmcapas & CKD_V25) {	/* V.25bis dialing... */
/*
  This assumes that V.25bis indications are all at least 3 characters long
  and are terminated by either CRLF or LFCR.
*/
		if (mymdmtyp == n_CCITT) {
		    if (n < 3) continue;
		    if ((lbuf[n] == CR) && (lbuf[n-1] == LF)) break;
		    if ((lbuf[n] == LF) && (lbuf[n-1] == CR)) break;
		}
#ifndef MINIDIAL
		else if (mymdmtyp == n_DIGITEL) {
		    if (((lbuf[n] == CR) && (lbuf[n-1] == LF)) ||
			((lbuf[n] == LF) && (lbuf[n-1] == CR)))
		      break;
		    else
		      continue;
		}
#endif /* MINIDIAL */
	    } else {			/* All others, break on CR or LF */
		if ( lbuf[n] == CR || lbuf[n] == LF ) break;
	    }
	}
	lbuf[++n] = '\0';		/* Terminate response from modem */
	debug(F111,"ckdial modem response",lbuf,n);
#ifndef NOSPL
	strncpy(modemmsg,lbuf,79);	/* Call result message */
	{
	    int x;			/* Strip junk from end */
	    x = (int)strlen(modemmsg) - 1;
	    while ((modemmsg[x] < (char) 33) && (x > -1))
	      modemmsg[x--] = NUL;
	}
#endif /* NOSPL */
	if (mdmcapas & CKD_AT) {	/* Hayes AT command set */
	    gethrw();			/* in word result mode */
	    if (partial && dialsta == DIA_ERR) {
		dialhup();
		dialsta = DIA_ERR;	/* (because dialhup() changes it) */
	    }
	    continue;
	} else if (mdmcapas & CKD_V25) { /* CCITT command set */
	    if (didweget(lbuf,"VAL")) { /* Dial command confirmation */
#ifndef MINIDIAL
		if (mymdmtyp == n_CCITT)
#endif /* MINIDIAL */
		  continue;		/* Go back and read more */
#ifndef MINIDIAL
/* Digitel doesn't give an explicit connect confirmation message */
		else {
		    int n;
		    for (n = -1; n < LBUFL-1; ) {
			lbuf[++n] = c2 = (char) (ddinc(0) & 0177);
			dialoc(lbuf[n]);
			if (((lbuf[n] == CR) && (lbuf[n-1] == LF)) ||
			    ((lbuf[n] == LF) && (lbuf[n-1] == CR)))
			  break;
		    }
		    mdmstat = CONNECTED; /* Assume we're connected */
		    if (dialdpy && carrier != CAR_OFF) {
			sleep(1); 	/* Wait a second */
			n = ttgmdm();	/* Try to read modem signals */
			if ((n > -1) && ((n & BM_DCD) == 0))
			  printf("WARNING - no carrier\n");
		    }
		}
#endif /* MINIDIAL */

		/* Standard V.25bis stuff */

	    } else if (didweget(lbuf,"CNX")) { /* Connected */
		mdmstat = CONNECTED;
	    } else if (didweget(lbuf, "INV")) {
		mdmstat = D_FAILED;	/* Command error */
		dialsta = DIA_ERR;
		strcpy(lbuf,"INV");

	    } else if (didweget(lbuf,"CFI")) { /* Call Failure */

		if (didweget(lbuf,"AB")) { /* Interpret reason code */
		    strcpy(lbuf,"AB: Timed out");
		    dialsta = DIA_TIMO;
		} else if (didweget(lbuf,"CB")) {
		    strcpy(lbuf,"CB: Local DCE Busy");
		    dialsta = DIA_NRDY;
		} else if (didweget(lbuf,"ET")) {
		    strcpy(lbuf,"ET: Busy");
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf, "NS")) {
		    strcpy(lbuf,"NS: Number not stored");
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"NT")) {
		    strcpy(lbuf,"NT: No answer");
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"RT")) {
		    strcpy(lbuf,"RT: Ring tone");
		    dialsta = DIA_RING;
		} else if (didweget(lbuf,"PV")) {
		    strcpy(lbuf,"PV: Parameter value error");
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"PS")) {
		    strcpy(lbuf,"PS: Parameter syntax error");
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"MS")) {
		    strcpy(lbuf,"MS: Message syntax error");
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"CU")) {
		    strcpy(lbuf,"CU: Command unknown");
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"FC")) {
		    strcpy(lbuf,"FC: Forbidden call");
		    dialsta = DIA_NOAC;
		}
		mdmstat = D_FAILED;
	    } else if (didweget(lbuf,"INC")) { /* Incoming Call */
		strcpy(lbuf,"INC: Incoming call");
		dialsta = DIA_RING;
		mdmstat = D_FAILED;
	    } else if (didweget(lbuf,"DLC")) { /* Delayed Call */
		strcpy(lbuf,"DLC: Delayed call");
		dialsta = DIA_NOAN;
		mdmstat = D_FAILED;
	    } else			/* Response was probably an echo. */
#ifndef MINIDIAL
	      if (mymdmtyp == n_CCITT)
#endif /* MINIDIAL */
		continue;
#ifndef MINIDIAL
	      else			/* Digitel: If no error, connect. */
		mdmstat = CONNECTED;
#endif /* MINIDIAL */
	    break;

	} else if (n) {			/* Non-Hayes-compatibles... */
	    switch (mymdmtyp) {
#ifndef MINIDIAL
	      case n_ATTMODEM:
		/* Careful - "Connected" / "Not Connected" */
		if (didweget(lbuf,"Busy")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"Not connected") ||
			   didweget(lbuf,"Not Connected")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"No dial tone") ||
			   didweget(lbuf,"No Dial Tone")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"No answer") ||
			   didweget(lbuf,"No Answer")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"Answered") ||
			   didweget(lbuf,"Connected")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		}
		break;

	      case n_ATTISN:
		if (didweget(lbuf,"ANSWERED")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		} else if (didweget(lbuf,"BUSY")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DISCONNECT")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"NO ANSWER")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"WRONG ADDRESS")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		}
		break;

	      case n_ATTDTDM:
		if (didweget(lbuf,"ANSWERED")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"CHECK OPTIONS")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"DISCONNECTED")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"DENIED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		}
#ifdef DEBUG
#ifdef ATT6300
		/* Horrible hack lost in history. */
		else if (deblog && didweget(lbuf,"~~"))
		  mdmstat = CONNECTED;
#endif /* ATT6300 */
#endif /* DEBUG */
		break;
		
#ifdef OLDMODEMS
	      case n_CERMETEK:
		if (didweget(lbuf,"\016A")) {
		    mdmstat = CONNECTED;
		    ttslow("\016U 1\015",200); /* Make transparent*/
		}
		break;

	      case n_DF03:
		/* Because response lacks CR or NL . . . */
		c = (char) (ddinc(0) & 0177);
		dialoc(c);
		debug(F000,"dial df03 got","",c);
		if ( c == 'A' ) mdmstat = CONNECTED;
		if ( c == 'B' ) mdmstat = D_FAILED;
		break;

	      case n_DF100:	     /* DF100 has short response codes */
		if (strcmp(lbuf,"A") == 0) {
		    mdmstat = CONNECTED; /* Attached */
		    dialsta = DIA_OK;
		} else if (strcmp(lbuf,"N") == 0) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN; /* No answer or no dialtone */
		} else if (strcmp(lbuf,"E") == 0 || /* Error */
			   strcmp(lbuf,"R") == 0) { /* "Ready" (?) */
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;	/* Command error */
		}
		/* otherwise fall thru... */

	      case n_DF200:
		if (didweget(lbuf,"Attached")) {
		    mdmstat = CONNECTED;
		    dialsta = DIA_OK;
		    /*
		     * The DF100 will respond with "Attached" even if DTR
		     * and/or carrier are not present.	Another reason to
		     * (also) wait for carrier?
		     */
		} else if (didweget(lbuf,"Busy")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"Disconnected")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		} else if (didweget(lbuf,"Error")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"No answer")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		} else if (didweget(lbuf,"No dial tone")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"Speed:)")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		}
		/*
		 * It appears that the "Speed:..." response comes after an
		 * "Attached" response, so this is never seen.  HOWEVER,
		 * it would be very handy to detect this and temporarily
		 * reset the speed, since it's a nuisance otherwise.
		 * If we wait for some more input from the modem, how do
		 * we know if it's from the remote host or the modem?
		 * Carrier reportedly doesn't get set until after the
		 * "Speed:..." response (if any) is sent.  Another reason
		 * to (also) wait for carrier.
		 */
		break;

	      case n_GDC:
		if (didweget(lbuf,"ON LINE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"NO CONNECT"))
		  mdmstat = D_FAILED;
		break;

	      case n_PENRIL:
		if (didweget(lbuf,"OK")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		    } else if (didweget(lbuf,"NO RING")) {
			mdmstat = D_FAILED;
			dialsta = DIA_NOCA;
		    }
		break;

	      case n_RACAL:
		if (didweget(lbuf,"ON LINE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"FAILED CALL"))
		  mdmstat = D_FAILED;
		break;
#endif /* OLDMODEMS */

	      case n_ROLM:
		if (didweget(lbuf,"CALLING"))
		  mdmstat = 0;
		else if (didweget(lbuf,"COMPLETE"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"FAILED") ||
			 didweget(lbuf,"ABANDONDED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"NOT AVAILABLE") ||
			   didweget(lbuf,"LACKS PERMISSION") ||
			   didweget(lbuf,"NOT A DATALINE") ||
			   didweget(lbuf,"INVALID DATA LINE NUMBER") ||
			   didweget(lbuf,"INVALID GROUP NAME")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		} else if (didweget(lbuf,"BUSY")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DOES NOT ANSWER")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAN;
		}
		break;

#ifdef OLDMODEMS
	      case n_VENTEL:
		if (didweget(lbuf,"ONLINE!") ||
		    didweget(lbuf,"Online!")) {
		    mdmstat = CONNECTED;
		} else if (didweget(lbuf,"BUSY") ||
			   didweget(lbuf,"Busy")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"DEAD PHONE")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_DISC;
		}
		break;

	      case n_CONCORD:
		if (didweget(lbuf,"INITIATING"))
		  mdmstat = CONNECTED;
		else if (didweget(lbuf,"BUSY")) { 
		    mdmstat = D_FAILED;			
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"CALL FAILED")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		}
		break;
#endif /* OLDMODEMS */

	      case n_MICROCOM:
		/* "RINGBACK" means phone line ringing, continue */
		if (didweget(lbuf,"NO CONNECT")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOCA;
		} else if (didweget(lbuf,"BUSY")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_BUSY;
		} else if (didweget(lbuf,"NO DIALTONE")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_NODT;
		} else if (didweget(lbuf,"COMMAND ERROR")) { 
		    mdmstat = D_FAILED;
		    dialsta = DIA_ERR;
		} else if (didweget(lbuf,"IN USE")) {
		    mdmstat = D_FAILED;
		    dialsta = DIA_NOAC;
		} else if (didweget(lbuf,"CONNECT")) {
		    mdmstat = CONNECTED;
		    /* trailing speed ignored */
		}
		break;

#endif /* MINIDIAL */
	      default:
		printf(
		    "PROGRAM ERROR - No response handler for modem type %d\n",
		       mymdmtyp);
		mdmstat = D_FAILED;
		dialsta = DIA_ERR;
	    }
	}
    } /* while (mdmstat == 0) */

    debug(F101,"ckdial alarm off","",x);
    alarm(0);
    if (mdmstat == D_FAILED )	{	/* Failure detected by modem  */
        dialfail(F_MODEM);
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
        SIGRETURN;
    } else if (mdmstat == D_PARTIAL )	{ /* Partial dial command OK */
	msleep(500);
	debug(F100,"dial partial","",0);
    } else {				/* Call was completed */
	msleep(1000);			/* In case DTR blinks  */
	debug(F100,"dial succeeded","",0);
	if (
#ifndef MINIDIAL
	    mymdmtyp != n_ROLM		/* Rolm has weird modem signaling */
#else
	    1
#endif /* MINIDIAL */
	    ) {
	    alarm(3);			/* In case ttpkt() gets stuck... */
	    ttpkt(speed,FLO_DIAX,parity); /* Cancel dialing state ioctl */
	}
    }
    dreset();				/* Reset alarms and signals. */
    if (!quiet && !backgrd) {
	if (dialdpy && (p = ck_time())) { /* If DIAL DISPLAY ON, */
	    printf(" %sall complete: %s.\n", /* include timestamp.  */
		   (mdmstat == D_PARTIAL) ?
		   "Partial c" :
		   "C",
		   p );
	} else {
	    printf (" %sall complete.\n",
		    (mdmstat == D_PARTIAL) ?
		    "Partial c" :
		    "C"
		    );
	}
    }

#ifdef DYNAMIC
    if (lbuf) free(lbuf); lbuf = NULL;
    if (rbuf) free(rbuf); rbuf = NULL;
    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
    dialsta = (mdmstat == D_PARTIAL) ? DIA_PART : DIA_OK;
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}


static SIGTYP
#ifdef CK_ANSIC
faildial(void * threadinfo)
#else /* Not CK_ANSIC */
faildial(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* faildial */ {
    debug(F100,"longjmp returns to dial routine","",0);
    dialfail(fail_code);
    SIGRETURN;
}

/*
  nbr = number to dial (string)
  x1  = Retry counter 
  x2  = Number counter
  fc  = Function code:
        0 == DIAL
        1 == ANSWER
        2 == INIT/CONFIG
        3 == PARTIAL DIAL
*/
int
#ifdef OLD_DIAL
ckdial(nbr) char *nbr;
#else
ckdial(nbr, x1, x2, fc) char *nbr; int x1, x2, fc;
#endif /* OLD_DIAL */
/* ckdial */ {
#define ERMSGL 50
    char errmsg[ERMSGL], *erp;		/* For error messages */
    int n = F_TIME;
    char *s;
    long spdmax;

    char *mmsg = "Sorry, DIAL memory buffer can't be allocated\n";

    partial = 0;
    if (fc == 3) {			/* Partial dial requested */
	partial = 1;			/* Set flag */
	fc = 0;				/* Treat like regular dialing */
    }
    func_code = fc;			/* Make global to this module */
    telnbr = nbr;
    mymdmtyp = mdmtyp;
    if (mymdmtyp < 0) {			/* Whoa, network dialing... */
	if (mdmsav > -1)
	  mymdmtyp = mdmsav;
    }
    if (mymdmtyp < 0) {
	printf("Invalid modem type %d - internal error.\n",mymdmtyp);
	dialsta = DIA_NOMO;
	return 0;
    }
    dial_what = DW_NOTHING;		/* Doing nothing at first. */
    nonverbal = 0;

/* These are ONLY for the purpose of interpreting numeric result codes. */

    is_rockwell =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_RWV32 || mymdmtyp == n_RWV32B || mymdmtyp == n_RWV34 ||
	mymdmtyp == n_BOCA || mymdmtyp == n_TELEPATH || mymdmtyp == n_CARDINAL
#endif /* MINIDIAL */
	;

    is_hayeshispd =
#ifdef MINIDIAL
      0
#else
      mymdmtyp == n_H_ULTRA || mymdmtyp == n_H_ACCURA || n_PPI
#endif /* MINIDIAL */
	;

#ifdef OLDTBCODE
#ifndef MINIDIAL
    tbmodel = TB_UNK;			/* Initialize Telebit model */
#endif /* MINIDIAL */
#endif /* OLDTBCODE */

    mp = modemp[mymdmtyp - 1];		/* Set pointer to modem info */
    if (!mp) {
	printf("Sorry, handler for this modem type not yet filled in.\n");
	dialsta = DIA_NOMO;
	return 0;
    }
    debug(F110,"dial number",telnbr,0);
#ifdef COMMENT
    debug(F110,"dial prefix",(dialnpr ? dialnpr : ""), 0);
#endif /* COMMENT */

#ifdef DYNAMIC
    if (!(lbuf = malloc(LBUFL+1))) {    /* Allocate input line buffer */
	printf("%s", mmsg);
	dialsta = DIA_IE;
	return 0;
    }
    *lbuf = NUL;
    debug(F101,"DIAL lbuf malloc ok","",LBUFL+1);

    if (!rbuf) {    /* This one might already have been allocated by getok() */
	if (!(rbuf = malloc(RBUFL+1))) {    /* Allocate input line buffer */
	    printf("%s", mmsg);
	    dialsta = DIA_IE;
	    if (lbuf) free(lbuf); lbuf = NULL;
	    return 0;
	} else
	  debug(F101,"DIAL rbuf malloc ok","",RBUFL+1);
    }
    if (!(fbuf = malloc(FULLNUML+1))) {    /* Allocate input line buffer */
	printf("%s", mmsg);
	dialsta = DIA_IE;
	if (lbuf) free(lbuf); lbuf = NULL;
	if (rbuf) free(rbuf); rbuf = NULL;
	return 0;
    }
    debug(F101,"DIAL fbuf malloc ok","",FULLNUML+1);
#endif /* DYNAMIC */

    /* NOTE: mdmtyp, not mymdmtyp */

    if (ttopen(ttname,&local,mdmtyp,0) < 0) { /* Open, no carrier wait */
	erp = errmsg;
	if ((int)strlen(ttname) < (ERMSGL - 18))
	  sprintf(erp,"Sorry, can't open %s",ttname);
	else
	  sprintf(erp,"Sorry, can't open device");
	perror(errmsg);
	dialsta = DIA_OPEN;
#ifdef DYNAMIC
	if (lbuf) free(lbuf); lbuf = NULL;
	if (rbuf) free(rbuf); rbuf = NULL;
	if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	return 0;
    }

/* Condition console terminal and communication line */

    /* Place line into "clocal" dialing state, */
    /* important mainly for System V UNIX.     */

    if (ttpkt(speed,FLO_DIAL,parity) < 0) {
	ttclos(0);			/* If ttpkt fails do all this... */
	if (ttopen(ttname,&local,mymdmtyp,0) < 0) {
	    erp = errmsg;
	    if ((int)strlen(ttname) < (ERMSGL - 18))
	      sprintf(erp,"Sorry, can't reopen %s",ttname);
	    else
	      sprintf(erp,"Sorry, can't reopen device");
	    perror(errmsg);
	    dialsta = DIA_OPEN;
#ifdef DYNAMIC
	    if (lbuf) free(lbuf); lbuf = NULL;
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    return 0;
	}				/* And try again. */
	if ((ttpkt(speed,FLO_DIAL,parity) < 0)
#ifdef UNIX
	&& (strcmp(ttname,"/dev/null"))
#else
#ifdef OSK
	&& (strcmp(ttname,"/nil"))
#endif /* OSK */
#endif /* UNIX */
	    ) {
	    printf("Sorry, Can't condition communication line\n");
	    printf("Try 'set line %s' again\n",ttname);
	    dialsta = DIA_OPEN;
#ifdef DYNAMIC
	    if (lbuf) free(lbuf); lbuf = NULL;
	    if (rbuf) free(rbuf); rbuf = NULL;
	    if (fbuf) free(fbuf); fbuf = NULL;
#endif /* DYNAMIC */
	    return 0;
	}
    }
    /* Modem's escape sequence... */

    c = (char) (dialesc ? dialesc : mp->esc_char);
    mdmcapas = dialcapas ? dialcapas : mp->capas;

    xx_ok = mp->ok_fn;			/* Pointer to response reader */

    if (mdmcapas & CKD_AT) {		/* Hayes compatible */
	escbuf[0] = c;
	escbuf[1] = c;
	escbuf[2] = c;
	escbuf[3] = NUL;
	/* In case this modem type is user-defined */
	if (!xx_ok) xx_ok = getok;
    } else {				/* Other */
	escbuf[0] = c;
	escbuf[1] = NUL;
	/* In case user-defined */
	if (mdmcapas & CKD_V25) if (!xx_ok) xx_ok = getok;
    }

    /* Partial dialing */

    if (mdmcapas & CKD_AT
#ifndef MINIDIAL
	|| mymdmtyp == n_MICROCOM
#endif /* MINIDIAL */
	) {
	int x;
	x = (int) strlen(telnbr);
	if (x > 0) {
	    if (telnbr[x-1] == ';') {
		partial = 1;
	    } else if (partial) {
		sprintf(fbuf,"%s;", telnbr); /* add one */
		telnbr = fbuf;
	    }
	}
    }
    msleep(500);

    /* Interdigit waits for tone dial */

    if (fc == 1) {			/* ANSWER */
	waitct = (dialatmo > -1) ? dialatmo : 0;
    } else {				/* DIAL */
	if (dialtmo < 1) {		/* Automatic computation. */
	    waitct = 1 * (int)strlen(telnbr) ; /* Worst case dial time */
	    waitct += mp->dial_time;	/* dialtone + completion wait times */
	    for (s = telnbr; *s; s++) {	/* add in pause characters time */
		for (p = mp->pause_chars; *p; p++)
		  if (*s == *p) {
		      waitct += mp->pause_time;
		      break;
		  }
	    }
#ifdef COMMENT
#ifndef MINIDIAL
#ifdef OLDTBCODE
	if (mymdmtyp == n_TBPEP)
	  waitct += 30;			/* Longer connect wait for PEP call */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
#endif /* COMMENT */
	} else waitct = dialtmo;	/* User-specified timeout */
    }

/*
  waitct is our alarm() timer.
  mdmwait is how long we tell the modem to wait for carrier.
  We set mdmwait to be 5 seconds less than waitct, to increase the
  chance that we get a response from the modem before timing out.
*/
    if (waitct < 0) waitct = 0;
    if (fc == 0) {			/* DIAL */

#ifdef XWAITCT
	/* Addtl wait slop can be defined at compile time */	
	waitct += XWAITCT;
#endif /* XWAITCT */
	if (waitct < 25) waitct = 25;
	mdmwait = waitct - mdmwaitd;
    } else {				/* ANSWER */
#ifdef COMMENT
/*
  This is wrong.  mdmwait is the value given to S7 in Hayeslike modems.
  When in autoanswer mode, this is the amount of time the modem waits for
  carrier once ringing starts.  Whereas waitct is the timeout given to the
  ANSWER command, which is an entirely different thing.  Since the default
  ANSWER timeout is 0 (meaning "wait forever"), the following statement sets
  S7 to 0, which, on some modems (like the USR Sportster) makes it hang up
  and report NO CARRIER the instant the phone rings.
*/	
	mdmwait = waitct;
#else
	mdmwait = 60;			/* Always wait 60 seconds. */
#endif /* COMMENT */

    }
    if (!quiet && !backgrd) {		/* Print information messages. */
	if (fc == 1)
	  printf(" Waiting for phone call...\n");
	else
	  printf(" %srying: %s...\n", x1 > 0 ? "Ret" : "T", telnbr);
	if (x1 == 0 && x2 == 0 && dialsta != DIA_PART) {
	    if (network) {
		printf(" Via modem server: %s, modem: %s\n",
		       ttname, gmdmtyp() );
	    } else {
		printf(" Device: %s, modem: %s",
		       ttname, gmdmtyp() );
		if (speed > -1L)
		  printf(", speed: %ld\n", speed);
		else
		  printf(", speed: (unknown)\n");
	    }
	    spdmax = dialmax > 0L ? dialmax : mp->max_speed;

	    if (!network &&  spdmax > 0L && speed > spdmax) {
		printf(
"\n  WARNING - interface speed %ld might be too high for this modem type.\n",
		       speed
		       );
		printf(
"  If dialing fails, SET SPEED to %ld or less and try again.\n\n",
		       spdmax
		       );
	    }
	    printf(" %s timeout: ", fc == 0 ? "Dial" : "Answer");
	    if (waitct > 0)
	      printf("%d seconds\n",waitct);
	    else
	      printf(" (none)\n");
	    printf(
#ifdef MAC
	       " Type Command-. to cancel.\n"
#else
#ifdef UNIX
	       " To cancel: type your interrupt character (normally Ctrl-C).\n"
#else
	       " To cancel: type Ctrl-C (hold down Ctrl, press C).\n"
#endif /* UNIX */
#endif /* MAC */
		   );
	}
    }
    debug(F111,"ckdial",ttname,(int) (speed / 10L));
    debug(F101,"ckdial timeout","",waitct);

/* Set timer and interrupt handlers. */
    savint = signal( SIGINT, dialint ) ; /* And terminal interrupt handler. */ 
    cc_alrm_execute(ckjaddr(sjbuf), 0, dialtime, _dodial, faildial);
    signal(SIGINT, savint);
#ifdef OS2
    if (dialsta == DIA_OK)		/* Dialing is completed */
      DialerSend(OPT_KERMIT_CONNECT, 0);
#endif /* OS2 */
    if (dialsta == DIA_PART || dialsta == DIA_OK)
      return(1);			/* Dial attempt succeeded */
    else
      return(0);			/* Dial attempt failed */
} /* ckdial */

/*
  getok() - wait up to n seconds for OK (0) or ERROR (4) response from modem.
  Use with Hayeslike or CCITT modems for reading the reply to a nondialing
  command.

  Second argument says whether to be strict about numeric result codes, i.e.
  to require they be preceded by CR or else be the first character in the
  response, e.g. to prevent the ATH0<CR> echo from looking like a valid
  response.  Strict == 0 is needed for ATI on Telebit, which can return the
  model number concatenated with the numeric response code, e.g. "9620"
  ("962" is the model number, "0" is the response code).  getok() Returns:

   0 if it timed out,
   1 if it succeeded,
  -1 on modem command, i/o, or other error.
*/
static ckjmpbuf okbuf;

static SIGTYP
#ifdef CK_ANSIC
oktimo(int foo)				/* Alarm handler for getok(). */
#else
oktimo(foo) int foo;			/* Alarm handler for getok(). */
#endif /* CK_ANSIC */
/* oktimo */ {

#ifdef OS2
    alarm(0);
    /* signal(SIGALRM,SIG_IGN); */
    debug(F100,"oktimo() SIGALRM caught -- SIG_IGN set","",0) ;
#endif /* OS2 */
    
#ifdef OSK				/* OS-9, see comment in dialtime(). */
    sigmask(-1);
#endif /* OSK */
#ifdef NTSIG
    if ( foo == SIGALRM )
      PostAlarmSigSem();
    else 
      PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(okbuf),1);
#else /* NT */
    cklongjmp(okbuf,1);
#endif /* NTSIG */
#endif /* NT */
    /* NOTREACHED */
    SIGRETURN;
}

static int okstatus, okn, okstrict;

static SIGTYP
#ifdef CK_ANSIC
dook(void * threadinfo)
#else /* CK_ANSIC */
dook(threadinfo) VOID * threadinfo ;
#endif /* CK_ANSIC */
/* dook */ {
    CHAR c;
    int i, x;

#ifdef NTSIG
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
    }
#endif /* NTSIG */

    if (mdmcapas & CKD_V25) {		/* CCITT, easy... */
        waitfor("VAL");
        okstatus = 1 ;
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;
#ifndef MINIDIAL
    } else if (mymdmtyp == n_MICROCOM) { /* Microcom in SX mode, also easy */
        waitfor(MICROCOM.wake_prompt);	/* (I think...) */
        okstatus = 1 ;
#ifdef NTSIG
	ckThreadEnd(threadinfo);
#endif /* NTSIG */
	SIGRETURN;
#endif /* MINIDIAL */
    } else {				/* Hayes & friends, start here... */
	okstatus = 0;			/* No status yet. */
	for (x = 0; x < RBUFL; x++)	/* Initialize response buffer */
	  rbuf[x] = SP;			/*  to all spaces */
	rbuf[RBUFL] = NUL;		/* and terminate with NUL. */
	debug(F100,"getok rbuf init ok","",0);
	while (okstatus == 0) {		/* While no status... */
	    x = ddinc(okn);		/* Read a character */
	    if (x < 0) {		/* I/O error */
		okstatus = -1 ;
#ifdef NTSIG
		ckThreadEnd(threadinfo);
#endif /* NTSIG */
		SIGRETURN;
	    }
	    debug(F101,"getok ddinc","",x); /* Got a character. */
	    c = (char) (x & 0x7f);	/* Get low order 7 bits */
	    if (!c)			/* Don't deposit NULs */
	      continue;			/* or else didweget() won't work */
	    if (dialdpy) conoc((char)c); /* Echo it if requested */
	    for (i = 0; i < RBUFL-1; i++) /* Rotate buffer */
	      rbuf[i] = rbuf[i+1];
	    rbuf[RBUFL-1] = c;		/* Deposit character at end */
	    debug(F000,"getok:",rbuf,(int) c); /* Log it */
	    switch (c) {		/* Interpret it. */
	      case CR:			/* Got a carriage return. */
		switch(rbuf[RBUFL-2]) {	/* Look at character before it. */
		  case '0':		/* 0 = OK numeric response */
		    if (!okstrict ||
			rbuf[RBUFL-3] == CR || rbuf[RBUFL-3] == SP) {
			nonverbal = 1;
			okstatus = 1;	/* Good response */
		    }
		    break;
		  case '4':		/* 4 = ERROR numeric response */
#ifndef MINIDIAL
		    /* Or Telebit model number 964! */
		    if (mymdmtyp == n_TELEBIT &&
			isdigit(rbuf[RBUFL-3]) &&
			isdigit(rbuf[RBUFL-4]))
		      break;
		    else
#endif /* MINIDIAL */
		      if (!okstrict ||
			rbuf[RBUFL-3] == CR || rbuf[RBUFL-3] == SP) {
			nonverbal = 1;
			okstatus = -1;	/* Bad command */
		    }
		    break;
		}
		if (dialdpy && nonverbal) /* If numeric results, */
		  conoc(LF);		  /* echo a linefeed too. */
		break;
	      case LF:			/* Got a linefeed. */
		/*
		  Note use of explicit octal codes in the string for
		  CR and LF.  We want real CR and LF here, not whatever
		  the compiler happens to replace \r and \n with...
		*/
		if (!strcmp(rbuf+RBUFL-4,"OK\015\012")) /* Good response */
		  okstatus = 1;
		else if (!strcmp(rbuf+RBUFL-7,"ERROR\015\012"))	/* Error */
		  okstatus = -1;
		break;
	      /* Check whether modem echoes its commands... */
	      case 't':			/* Got little t */
		if (!strcmp(rbuf+RBUFL-3,"\015at") || /* See if it's "at" */
		    !strcmp(rbuf+RBUFL-3," at"))
		    mdmecho = 1;
		debug(F111,"MDMECHO-t",rbuf+RBUFL-2,mdmecho);
		break;
	      case 'T':			/* Got Big T */
		if (!strcmp(rbuf+RBUFL-3,"\015AT") ||	/* See if it's "AT" */
		    !strcmp(rbuf+RBUFL-3," AT"))
		    mdmecho = 1;
		debug(F111,"MDMECHO-T",rbuf+RBUFL-3,mdmecho);
		break;
	      default:			/* Other characters, accumulate. */
		okstatus = 0;
		break;
	    }
	}
    }
    debug(F101,"getok returns","",okstatus); /* <-- It's a lie */
#ifdef NTSIG
    ckThreadEnd(threadinfo);
#endif /* NTSIG */
    SIGRETURN;
}

static SIGTYP
#ifdef CK_ANSIC
failok(void * threadinfo)
#else /* CK_ANSIC */
failok(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failok */ {
    debug(F100,"longjmp returned to getok()","",0);
    debug(F100,"getok timeout","",0);
    SIGRETURN;
}

static int
getok(n, strict) int n, strict; {
    debug(F101,"getok entry n","",n);
    okstatus = 0;
    okn = n;
    okstrict = strict;

#ifdef DYNAMIC
    if (!rbuf) {
	if (!(rbuf = malloc(RBUFL+1))) { /* Allocate input line buffer */
	    dialsta = DIA_IE;
	    return(-1);
	}
	debug(F101,"GETOK rbuf malloc ok","",RBUFL+1);
    }
#endif /* DYNAMIC */

    mdmecho = 0;			/* Assume no echoing of commands */

    debug(F100,"about to alrm_execute dook()","",0);
    alrm_execute( ckjaddr(okbuf), n, oktimo, dook, failok ) ;
    debug(F100,"returning from alrm_execute dook()","",0);

    ttflui();				/* Flush input buffer */
    return(okstatus);			/* Return status */
}

/*  G E T H R N  --  Get Hayes Result Numeric  */

static VOID
gethrn() {
    char c;
    int x;
/*
  Hayes numeric result codes (Hayes 1200 and higher):
     0 = OK
     1 = CONNECT at 300 bps (or 1200 bps on Hayes 1200 with basic code set)
     2 = RING
     3 = NO CARRIER
     4 = ERROR (in command line)
     5 = CONNECT 1200 (extended code set)
  Hayes 2400 and higher:
     6 = NO DIALTONE
     7 = BUSY
     8 = NO ANSWER
     9 = (there is no 9)
    10 = CONNECT 2400
  Reportedly, the codes for Hayes V.32 modems are:
    1x = CONNECT <suffix>
    5x = CONNECT 1200 <suffix>
    9x = CONNECT 2400 <suffix>
   11x = CONNECT 4800 <suffix>
   12x = CONNECT 9600 <suffix>
  Where:
    x:   suffix:
    R  = RELIABLE
    RC = RELIABLE COMPRESSED
    L  = LAPM
    LC = LAPM COMPRESSED
  And for Telebits, all the above, except no suffix in numeric mode, plus:
    11 = CONNECT 4800
    12 = CONNECT 9600
    13 = CONNECT 14400
    14 = CONNECT 19200
    15 = CONNECT 38400
    16 = CONNECT 57600
    20 = CONNECT 300/REL  (= MNP)
    22 = CONNECT 1200/REL (= MNP)
    23 = CONNECT 2400/REL (= MNP)
    46 = CONNECT 7512  (i.e. 75/1200)
    47 = CONNECT 1275  (i.e. 1200/75)
    48 = CONNECT 7200
    49 = CONNECT 12000
    50 = CONNECT FAST (not on T1600/3000)
    52 = RRING
    53 = DIALING
    54 = NO PROMPTTONE
    61 = CONNECT FAST/KERM (Kermit spoof)
    70 = CONNECT FAST/COMP (PEP + compression)
    71 = CONNECT FAST/KERM/COMP (PEP + compression + Kermit spoof)

  And for others, lots of special cases below...
*/
#define NBUFL 8
    char nbuf[NBUFL+1];			/* Response buffer */
    int i = 0, j = 0;			/* Buffer pointers */

    debug(F101,"RESPONSE mdmecho","",mdmecho);
    if (mdmecho) {			/* Sponge up dialing string echo. */
	while (1) {
	    c = (char) (ddinc(0) & 0x7f);
	    debug(F000,"SPONGE","",c);
	    dialoc(c);
	    if (c == CR) break;
	}
    }
    while (mdmstat == 0) {		/* Read response */
	for (i = 0; i < NBUFL; i++)	/* Clear the buffer */
	  nbuf[i] = '\0';
	i = 0;				/* Reset the buffer pointer. */
	c = (char) (ddinc(0) & 0177);	/* Get first digit of response. */
					/* using an untimed, blocking read. */
	debug(F000,"RESPONSE-A","",c);
	dialoc(c);			/* Echo it if requested. */
	if (!isdigit(c))		/* If not a digit, keep looking. */
	  continue;
	nbuf[i++] = c;			/* Got first digit, save it. */
	while (c != CR && i < 8) {	/* Read chars up to CR */
	    x = ddinc(0) & 0177;	/* Get a character. */
	    c = (char) x;		/* Got it OK. */
	    debug(F000,"RESPONSE-C","",c);
	    if (c != CR)		/* If it's not a carriage return, */
	      nbuf[i++] = c;		/*  save it. */
	    dialoc(c);			/* Echo it. */
	}
	nbuf[i] = '\0';			/* Done, terminate the buffer. */
	debug(F110,"dial hayesnv lbuf",lbuf,0);
	debug(F111,"dial hayesnv got",nbuf,i);
	/*
	   Separate any non-numeric suffix from the numeric 
	   result code with a null.
	*/
	for (j = i-1; (j > -1) && !isdigit(nbuf[j]); j--)
	  nbuf[j+1] = nbuf[j];
	j++;
	nbuf[j++] = '\0';
	debug(F110,"dial hayesnv numeric",nbuf,0);
	debug(F111,"dial hayesnv suffix ",nbuf+j,j);
	/* Probably phone number echoing. */
	if ((int)strlen(nbuf) > 3)
	  continue;

	/* Now read and interpret the results... */

	i = atoi(nbuf);	/* Convert to integer */
	switch (i) {
	  case 0:
	    mdmstat = D_PARTIAL;	/* OK response */
	    break;
	  case 1:			/* CONNECT */
	    mdmstat = CONNECTED;	/* Could be any speed */
	    break;
	  case 2:			/* RING */
	    if (dialdpy)
	      printf("\r\n Local phone is ringing!\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_RING;
	    break;
	  case 3:			/* NO CARRIER */
	    if (dialdpy) printf("\r\n No Carrier.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NOCA;
	    break;
	  case 4:			/* ERROR */
	    if (dialdpy)
	      printf("\r\n Modem Command Error.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_ERR;
	    break;
	  case 5:			/* CONNECT 1200 */
	    spdchg(1200L); /* Change speed if necessary. */
	    mdmstat = CONNECTED;
	    break;
	  case 6:			/* NO DIALTONE */
#ifndef MINIDIAL
	    if (mymdmtyp == n_MICROLINK && atoi(diallcc) == 49 && dialdpy)
	      printf("\r\n Dial Locked.\r\n"); /* Germany */
	    else
#endif /* MINIDIAL */
	      if (dialdpy)
		printf("\r\n No Dialtone.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NODT;
	    break;
	  case 7:			/* BUSY */
	    if (dialdpy) printf("\r\n Busy.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_BUSY;
	    break;
	  case 8:			/* NO ANSWER */
#ifndef MINIDIAL
	    if (mymdmtyp == n_MICROLINK && atoi(diallcc) == 41 && dialdpy)
	      printf("\r\n Dial Locked.\r\n"); /* Switzerland */
	    else
#endif /* MINIDIAL */
	      if (dialdpy)
		printf("\r\n No Answer.\r\n");
	    mdmstat = D_FAILED;
	    dialsta = DIA_NOAN;
	    break;
	  case 9:			/* CONNECT 2400 */
	  case 10:
	    spdchg(2400L); /* Change speed if necessary. */
	    mdmstat = CONNECTED;
	    break;

#ifndef MINIDIAL

/* Starting here, we get different meanings from different manufacturers */

	  case 11:
	    if (mymdmtyp == n_USR) {
		if (dialdpy) printf(" Ringing...\r\n");		
	    } else {
		spdchg(4800L);		/* CONNECT 4800 */
		mdmstat = CONNECTED;
	    }
	    break;
	  case 12:
	    if (mymdmtyp == n_USR) {
		if (dialdpy)
		  printf("\r\n Answered by voice.\r\n");
		mdmstat = D_FAILED;
		dialsta = DIA_VOIC;
	    } else {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 13:
	    if (mymdmtyp == n_USR)
	      spdchg(9600L); 
	    if (is_rockwell || mymdmtyp == n_ZOLTRIX)
	      spdchg(7200L); 
	    else if (mymdmtyp != n_MICROLINK) /* 12000 */
	      spdchg(14400L);
	    mdmstat = CONNECTED;
	    break;
	  case 14:
	    if (is_rockwell)
	      spdchg(12000L); 
	    else if (mymdmtyp == n_DATAPORT || mymdmtyp == n_MICROLINK)
	      spdchg(14400L);
	    else if (mymdmtyp != n_USR && mymdmtyp != n_ZOLTRIX)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
	  case 15:
	    if (is_rockwell || mymdmtyp == n_ZOLTRIX)
	      spdchg(14400L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    else
	      spdchg(38400L);
	    mdmstat = CONNECTED;
	    break;
	  case 16:
	    if (is_rockwell || mymdmtyp == n_ZOLTRIX)
	      spdchg(19200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(7200L);
	    else if (mymdmtyp != n_ZYXEL && mymdmtyp != n_INTEL) /* 12000 */
	      spdchg(57600L);
	    mdmstat = CONNECTED;
	    break;
	  case 17:
	    if (mymdmtyp != n_DATAPORT)	/* 16800 */
	      spdchg(38400L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(14400L);
	    mdmstat = CONNECTED;
	    break;
	  case 18:
	    if (is_rockwell || mymdmtyp == n_ZOLTRIX)
	      spdchg(57600L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(19200L);
	    else if (mymdmtyp == n_USR)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 19:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(300L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(38400L);
	    else
	      spdchg(115200L);
	    mdmstat = CONNECTED;
	    break;
	  case 20:
	    if (mymdmtyp == n_USR)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(2400L);
	    else if (mymdmtyp == n_ZYXEL || mymdmtyp == n_INTEL)
	      spdchg(57600L);
	    else
	      spdchg(300L);
	    mdmstat = CONNECTED;
	    break;
	  case 21:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 22:
	    if (is_rockwell)
	      spdchg(8880L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(9600L);
	    else if (!is_hayeshispd)
	      spdchg(1200L);
	    mdmstat = CONNECTED;
	    break;
	  case 23:
	    if (is_hayeshispd || mymdmtyp == n_MULTI)
	      spdchg(8880L);
	    else if (mymdmtyp != n_DATAPORT && !is_rockwell) /* 12000 */
	      spdchg(2400L);
	    mdmstat = CONNECTED;
	    break;
	  case 24:
	    if (is_rockwell) {
		mdmstat = D_FAILED;
		dialsta = DIA_DELA;	/* Delayed */
		break;
	    } else if (is_hayeshispd)
	      spdchg(7200L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(14400L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(1200L);
	    mdmstat = CONNECTED;
	    break;
	  case 25:
	    if (mymdmtyp == n_MOTOROLA)
	      spdchg(9600L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(2400L);
	    mdmstat = CONNECTED;
	    break;
	  case 26:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(4800L);
	    mdmstat = CONNECTED;
	    break;
	  case 27:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(38400L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(7200L);
	    mdmstat = CONNECTED;
	    break;
	  case 28:
	    if (mymdmtyp == n_DATAPORT)
	      spdchg(7200L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(9600L);
	    mdmstat = CONNECTED;
	    break;
	  case 29:
	    if (mymdmtyp == n_MOTOROLA)
	      spdchg(4800L);
	    else if (mymdmtyp == n_DATAPORT)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
	  case 30:
	    if (mymdmtyp == n_INTEL) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 31:
	    if (mymdmtyp == n_UCOM_AT || mymdmtyp == n_MICROLINK) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 32:
	    if (is_rockwell) {
		mdmstat = D_FAILED;
		dialsta = DIA_BLCK;	/* Blacklisted */
	    } else if (mymdmtyp == n_UCOM_AT || mymdmtyp == n_MICROLINK) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 33:			/* FAX connection */
	    if (is_rockwell || mymdmtyp == n_ZOLTRIX) {
		mdmstat = D_FAILED;
		dialsta = DIA_FAX;
	    } else if (mymdmtyp == n_UCOM_AT ||
		       mymdmtyp == n_MOTOROLA ||
		       mymdmtyp == n_MICROLINK
		       ) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 34:
	    if (mymdmtyp == n_INTEL) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 35:
	    if (is_rockwell) {
		spdchg(300L);
		dialsta = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_ZOLTRIX) /* "DATA" */
	      mdmstat = CONNECTED;
	    break;
	  case 36:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 37:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 38:
	    if (mymdmtyp == n_MOTOROLA) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 39:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(38400L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_MICROLINK) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 40:
	    if (mymdmtyp == n_UCOM_AT) {
		mdmstat = D_FAILED;
		dialsta = DIA_NOCA;
	    } else if (mymdmtyp == n_MOTOROLA || mymdmtyp == n_INTEL) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 41:
	    if (mymdmtyp == n_MOTOROLA) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 42:
	    if (mymdmtyp == n_MOTOROLA) {
		spdchg(38400L);
		mdmstat = CONNECTED;
	    } /* fall thru on purpose... */
	  case 43:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 44:
	    if (is_rockwell) {
		spdchg(8800L);
		dialsta = CONNECTED;
	    } else if (mymdmtyp == n_MOTOROLA) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 45:
	    if (mymdmtyp == n_MOTOROLA) {
		spdchg(57600L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 46:
	    if (is_rockwell)
	      spdchg(1200L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(4800L);
	    else
	      spdchg(8880L);		/* 75/1200 split speed */
	    mdmstat = CONNECTED;
	    break;
	  case 47:
	    if (is_rockwell)
	      spdchg(2400L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(7200L);
	    else
	      printf("CONNECT 1200/75 - Not supported by C-Kermit\r\n");
	    mdmstat = CONNECTED;
	    break;
	  case 48:
	    if (is_rockwell)
	      spdchg(4800L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(9600L);
	    else
	      spdchg(7200L);
	    mdmstat = CONNECTED;
	    break;
	  case 49:
	    if (is_rockwell)
	      spdchg(7200L);
	    mdmstat = CONNECTED;
	    break;
	  case 50:			/* CONNECT FAST */
	    if (is_rockwell)
	      spdchg(9600L);
	    else if (mymdmtyp == n_INTEL)
	      spdchg(14400L);
	    mdmstat = CONNECTED;
	    break;
	  case 51:
	    if (mymdmtyp == n_UCOM_AT) {
		mdmstat = D_FAILED;
		dialsta = DIA_NODT;
	    }
	    break;
	  case 52:			/* RRING */
	    if (mymdmtyp == n_TELEBIT)
	      if (dialdpy) printf(" Ringing...\r\n");
	    break;
	  case 53:			/* DIALING */
	    if (mymdmtyp == n_TELEBIT)
	      if (dialdpy) printf(" Dialing...\r\n");
	    break;
	  case 54:
	    if (is_rockwell) {
		spdchg(19200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_INTEL) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    } else if (mymdmtyp == n_TELEBIT) {
		if (dialdpy) printf("\r\n No Prompttone.\r\n");
		mdmstat = D_FAILED;
		dialsta = DIA_NODT;
	    }
	    break;
	  case 55:
	    if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 56:
	    if (mymdmtyp == n_INTEL) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 57:
	    if (mymdmtyp == n_INTEL) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 58:
	    if (mymdmtyp == n_INTEL) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 59:
	    if (mymdmtyp == n_INTEL)	/* 12000 */
	      mdmstat = CONNECTED;
	    break;
	  case 60:
	    if (mymdmtyp == n_INTEL) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 64:
	    if (mymdmtyp == n_INTEL) {
		spdchg(1200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 65:
	    if (mymdmtyp == n_INTEL) {
		spdchg(2400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 66:
	    if (mymdmtyp == n_INTEL) {
		spdchg(4800L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 67:
	    if (mymdmtyp == n_INTEL) {
		spdchg(7200L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 68:
	    if (mymdmtyp == n_INTEL) {
		spdchg(9600L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 69:
	    if (mymdmtyp == n_INTEL)	/* 12000 */
	      mdmstat = CONNECTED;
	    break;
	  case 70:
	    if (mymdmtyp == n_INTEL) {
		spdchg(14400L);
		mdmstat = CONNECTED;
	    }
	    break;
	  case 73:
	    if (mymdmtyp == n_UCOM_AT) {
		spdchg(115200L);
		mdmstat = CONNECTED;
		break;
	    } /* else fall thru */
	    if (mymdmtyp == n_TELEBIT)	/* Early models only */
	      mdmstat = CONNECTED;
	    break;
	  case 85:
	    if (mymdmtyp == n_USR)
	      spdchg(19200L);
	    mdmstat = CONNECTED;
	    break;
#endif /* MINIDIAL */
	  default:
#ifndef MINIDIAL
	    if (mymdmtyp == n_USR || is_hayeshispd || is_rockwell)
#endif /* MINIDIAL */
	      if (i > 12)		/* There are hundreds of them... */
		mdmstat = CONNECTED;
	    break;
	}
    }
    if (mdmstat == CONNECTED && nbuf[j] != '\0') {
	if (dialdpy) {
	    printf("\r\n");
	    if (nbuf[j] == 'R') printf(" RELIABLE");
	    if (nbuf[j] == 'L') printf(" LAPM");
	    if (nbuf[j+1] == 'C') printf(" COMPRESSED");
	    printf("\r\n");
	}
	strcpy(lbuf,nbuf);		/* (for messages...) */
    }
}

static VOID				/* Get Hayes Result in Word mode */
gethrw() {
    char *cptr, *s;
    long conspd;

    if (mdmspd && !network) {
	s = lbuf;
	while (*s != '\0' && *s != 'C') s++;
	cptr = (*s == 'C') ? s : NULL;
	conspd = 0L;
	if ((cptr != NULL) && !strncmp(cptr,"CONNECT ",8)) {
	    if ((int)strlen(cptr) < 9)   /* Just CONNECT, */
	      conspd = 300L;		 /* use 300 bps */
	    else if (isdigit(*(cptr+8))) /* not CONNECT FAST */
	      conspd = atol(cptr + 8);   /* CONNECT nnnn */
	    if (conspd != speed) {
		if ((conspd / 10L) > 0) {
		    if (ttsspd((int) (conspd / 10L)) < 0) {
			printf(" Can't change speed to %ld\r\n",
			       conspd);
		    } else {
			speed = conspd;
			mdmstat = CONNECTED;
			if ( !quiet && !backgrd )
			  printf(" Speed changed to %ld\r\n",
				 conspd);
		    }
		}
	    } /* Expanded to handle any conceivable speed */
	}
    }
#ifndef MINIDIAL
    if (mymdmtyp == n_TELEBIT) {
	if (didweget(lbuf,"CONNECT FAST/KERM")) {
	    mdmstat = CONNECTED;
	    if (dialdpy) printf("FAST/KERM ");
	    return;
	}
    }
#endif /* MINIDIAL */
    if (didweget(lbuf,"RRING") ||
	didweget(lbuf,"RINGING") ||
	didweget(lbuf,"DIALING")) {
	mdmstat = 0;
    } else if (didweget(lbuf,"CONNECT")) {
	mdmstat = CONNECTED;
    } else if (didweget(lbuf,"OK")) {
	mdmstat = D_PARTIAL;
    } else if (didweget(lbuf,"NO CARRIER")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOCA;
    } else if (didweget(lbuf,"NO DIALTONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"NO DIAL TONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"BUSY")) {
	mdmstat = D_FAILED;
	dialsta = DIA_BUSY;
    } else if (didweget(lbuf,"NO ANSWER")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOAN;
    } else if (didweget(lbuf,"VOICE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_VOIC;
    } else if (didweget(lbuf,"NO PROMPT TONE")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NODT;
    } else if (didweget(lbuf,"REMOTE ACCESS FAILED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_NOCA;
    } else if (didweget(lbuf,"FAX")) {
	mdmstat = D_FAILED;
	dialsta = DIA_FAX;
    } else if (didweget(lbuf,"DELAYED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_DELA;
    } else if (didweget(lbuf,"BLACKLISTED")) {
	mdmstat = D_FAILED;
	dialsta = DIA_BLCK;
    } else if (didweget(lbuf,"DIAL LOCKED")) { /* Germany, Austria, Schweiz */
	mdmstat = D_FAILED;
	dialsta = DIA_BLCK;
    } else if (didweget(lbuf,"RING")) {
	mdmstat = (func_code == 0) ? D_FAILED : 0;
	dialsta = DIA_RING;			
    } else if (didweget(lbuf,"ERROR")) {
	mdmstat = D_FAILED;
	dialsta = DIA_ERR;
    } else if (didweget(lbuf,"CARRIER")) { /* Boca */
	mdmstat = CONNECTED;
    } else if (didweget(lbuf,"DATA")) {	/* Boca */
	mdmstat = CONNECTED;
    }
}

/* Maybe hang up the phone, depending on various SET DIAL parameters. */

int
dialhup() {
    int x = 0;
    if (dialhng && dialsta != DIA_PART) { /* DIAL HANGUP ON? */
	x = mdmhup();			/* Try modem-specific method first */
	debug(F101,"dialhup mdmhup","",x);
	if (x > 0) {			/* If it worked, */
	    dialsta = DIA_HUP;
	    if (dialdpy)
	      printf(" Modem hangup OK\r\n"); /* fine. */
	} else if (network) {		/* If we're telnetted to */
	    dialsta = DIA_HANG;
	    if (dialdpy)		/* a modem server, just print a msg */
	      printf(" WARNING - modem hangup failed\r\n"); /* don't hangup! */
	    return(0);
	} else {			/* Otherwise */
	    x = tthang();		/* Tell the OS to turn off DTR. */
	    if (x > 0) {		/* Yes, tell results from tthang() */
		dialsta = DIA_HUP;
		if (dialdpy) printf(" Hangup OK\r\n");
	    } else if (x == 0) {
		if (dialdpy) printf(" Hangup skipped\r\n");
	    } else {
		dialsta = DIA_HANG;
		if (dialdpy) perror(" Hangup error");
	    }
	}
    } else if (dialdpy) printf(" Hangup skipped\r\n"); /* DIAL HANGUP OFF */
    return(x);
}

/*
  M D M H U P  --

  Sends escape sequence to modem, then sends its hangup command.  Returns:
   0: If modem type is 0 (direct serial connection),
      or if modem type is < 0 (network connection),
      or if no action taken because DIAL MODEM-HANGUP is OFF)
        or because no hangup string for current modem type,
      or C-Kermit is in remote mode,
      or if action taken but there was no positive response from modem;
   1: Success: modem is in command state and acknowledged the hangup command;
  -1: On modem command error.
*/
int
mdmhup() {
#ifdef MDMHUP
    int m, x = 0;
    int xparity;
    char *s, *p;
    MDMINF * mp = NULL;

    if (dialmhu == 0 || local == 0)	/* If DIAL MODEM-HANGUP is OFF, */
      return(0);			/*  or not in local mode, fail. */

#ifdef OS2
/*
  In OS/2, if CARRIER is OFF, and there is indeed no carrier signal, any
  attempt to do i/o at this point can hang the program.  This might be true
  for other operating systems too.
*/
    if (!network) {			/* Not a network connection */
	m = ttgmdm();			/* Get modem signals */
	if ((m > -1) && (m & BM_DCD == 0)) /* Check for carrier */
	  return(0);			/* No carrier, skip the rest */
    }
#endif /* OS2 */
    
    if (mymdmtyp < 0)
      return(0);
    if (mymdmtyp > 0) mp = modemp[mymdmtyp - 1];
    if (!mp) return(0);

    s = dialhcmd ? dialhcmd : mp->hup_str;
    if (!s) return(0);
    if (!*s) return(0);

    debug(F110,"mdmhup hup_str",s,0);
    xparity = parity;			/* Set PARITY to NONE temporarily */

    if (escbuf[0]) {			/* Have escape sequence? */
	debug(F110,"mdmhup escbuf",escbuf,0);
	debug(F101,"mdmhup esc_time",0,mp->esc_time);
	parity = 0;
	if (ttpkt(speed,FLO_DIAL,parity) < 0) { /* Condition line */
	    parity = xparity;
	    return(-1);			/*  for dialing. */
	}
	if (mp->esc_time)		/* If we have a guard time */
	  msleep(mp->esc_time);		/* Pause for guard time */
	debug(F100,"mdmhup pause 1 OK","",0);

#ifdef NETCONN				/* Send modem's escape sequence */
	if (network) {			/* Must catch errors here. */
	    if (ttol((CHAR *)escbuf,(int)strlen((char *)escbuf)) < 0) {
		parity = xparity;
		return(-1);
	    }
	} else {
#endif /* NETCONN */
	    ttslow((char *)escbuf,mp->wake_rate); /* Send escape sequence */
	    debug(F110,"mdmhup net ttslow ok",escbuf,0);
#ifdef NETCONN
	}
#endif /* NETCONN */

	if (mp->esc_time)		/* Pause for guard time again */
	  msleep(mp->esc_time);
	else
	  msleep(500);			/* Wait half a sec for echoes. */
	debug(F100,"mdmhup pause 1 OK","",0);
#ifdef COMMENT	
	ttflui();			/* Flush response or echo, if any */
	debug(F100,"mdmhup ttflui OK","",0);
#endif /* COMMENT */
	ttslow(s,mp->wake_rate);	/* Now Send hangup string */
	debug(F110,"mdmhup ttslow ok",s,0);
/*
  This is not exactly right, but it works.
  If we are online:
    the modem says OK when it gets the escape sequence,
    and it says NO CARRIER when it gets the hangup command.    
  If we are offline:
    the modem does NOT say OK (or anything else) when it gets the esc sequence,
    but it DOES say OK (and not NO CARRIER) when it gets the hangup command.
  So the following function should read the OK in both cases.
  Of course, this is somewhat Hayes-specific...
*/
	if (xx_ok) {			/* Look for OK response */
	    debug(F100,"mdmhup calling response function","",0);
	    x = (*xx_ok)(3,1);		/* Give it 3 seconds, be strict. */
	    debug(F101,"mdmhup hangup response","",x);
	    msleep(500);		/* Wait half a sec */
	    ttflui();			/* Get rid of NO CARRIER, if any */
	} else {			/* No OK function, */
	    x = 1;			/* so assume it worked */
	    debug(F101,"mdmhup no ok_fn","",x);
	}
    }
    parity = xparity;			/* Restore prevailing parity */
    return(x);				/* Return OK function's return code. */

#else  /* MDMHUP not defined. */

    return(0);				/* Always fail. */
#endif /* MDMHUP */
}

#else /* NODIAL */

char *dialv = "Dial Command Disabled";

int					/* To allow NODIAL versions to */
mdmhup() {				/* call mdmhup(), so calls to  */
    return(0);				/* mdmhup() need not be within */
}					/* #ifndef NODIAL conditionals */

#endif /* NOICP */
#endif /* NODIAL */
#endif /* NOLOCAL */
