#include "ckcsym.h"			/* Symbol definitions */
#ifndef NOICP

/*  C K U U S 3 --  "User Interface" for Unix Kermit, part 3  */
 
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/
 
/*  SET command (but much material has been split off into ckuus7.c). */
 
/*
  Kermit-specific includes.
  Definitions here supersede those from system include files.
*/
#include "ckcdeb.h"			/* Debugging & compiler things */
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckcker.h"			/* Kermit application definitions */
#include "ckcxla.h"			/* Character set translation */
#include "ckcnet.h"			/* Network symbols */
#include "ckuusr.h"			/* User interface symbols */
#ifdef OS2
#ifdef NT
#include "ckntap.h"			/* Microsoft TAPI */
#endif /* NT */
#ifdef CK_NETBIOS
#include <os2.h>
#ifdef COMMENT				/* Would you believe */
#undef COMMENT				/* <os2.h> defines this ? */
#endif /* COMMENT */
#include "ckonbi.h"
extern UCHAR NetBiosAdapter;
#endif /* CK_NETBIOS */
#include "ckocon.h"
#include "ckokey.h"
extern unsigned char colorcmd;	/* Command-screen colors */
extern struct keytab ttyclrtab[];
extern int nclrs;
extern int tt_cols[], tt_rows[], tt_szchng[];
_PROTOTYP(int setprty, (void));
extern char startupdir[], exedir[];
extern int srvidl;
#endif /* OS2 */

#ifdef CK_RECALL
extern int cm_retry;
#endif /* CK_RECALL */

/* Variables */

int cmd_quoting = 1;

extern char * ckprompt;			/* Default prompt */

extern xx_strp xxstring;

extern struct ck_p ptab[];
extern int protocol;

#ifndef NOSERVER
extern int ngetpath;
extern char * getpath[];
#endif /* NOSERVER */

extern int size, spsiz, spmax, urpsiz, srvtim, slostart,
  local, server, success, dest,
  flow, autoflow, binary, xfermode, delay, parity, escape, what, srvdis,
  turn, duplex, backgrd,
  turnch, bctr, mdmtyp, keep, maxtry, unkcs, network,
  ebqflg, quiet, swcapr, nettype,
  wslotr, lscapr, lscapu,
  carrier, debses,
  cdtimo, nlangs, bgset, pflag, msgflg, dblchar,
  cmdmsk, spsizr, wildxpand, suspend,
  techo, rptena, rptmin, docrc,
  xfrcan, xfrchr, xfrnum, pacing, xitwarn, xitsta,
  cmd_cols, cmd_rows, ckxech, xaskmore, xfrbel;

#ifdef TCPSOCKET
  extern int tn_exit;
#endif /* TCPSOCKET */
  extern int exitonclose;

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */

#ifdef TNCODE
extern int ttnproto;
#endif /* TNCODE */

extern char *ccntab[];			/* Names of control chars */

#ifdef CK_SPEED
extern short ctlp[];			/* Control-prefix table */
extern int prefixing;
static struct keytab pfxtab[] = {
    "all",         PX_ALL, 0,
    "cautious",    PX_CAU, 0,
    "minimal",     PX_WIL, 0,
    "none",        PX_NON, 0
};
#endif /* CK_SPEED */

#ifndef NOSCRIPT
extern int secho;			/* Whether SCRIPT cmd should echo */
#endif /* NOSCRIPT */

#ifdef DCMDBUF
extern char *atmbuf, *atxbuf;
#else
extern char atmbuf[], atxbuf[];
#endif /* DCMDBUF */

extern char psave[];
extern char uidbuf[];

#ifndef NOSPL
int DeleteStartupFile =
#ifdef NT
                        1
#else
                        0
#endif /* NT */
;
char pwbuf[64]  = { NUL, NUL };
char prmbuf[64] = { NUL, NUL };
#ifdef DCMDBUF
extern int *count, *takerr, *merror, *inpcas;
#else
extern int count[], takerr[], merror[], inpcas[];
#endif /* DCMDBUF */
extern int mecho;			/* Macro echo */
extern long ck_alarm;
extern char alrm_date[], alrm_time[];
#else
extern int takerr[];
#endif /* NOSPL */

extern int x_ifnum;
extern int bigsbsiz, bigrbsiz;		/* Packet buffers */

extern long speed;			/* Terminal speed */

extern CHAR sstate;			/* Protocol start state */
extern CHAR myctlq;			/* Control-character prefix */
extern CHAR myrptq;			/* Repeat-count prefix */
extern CHAR mystch, seol;		/* Packet start and end chars */
extern char ttname[];			/* Communication device name */
extern char myhost[] ;
extern char inidir[];			/* Ini File directory */

#ifndef NOSETKEY
extern KEY *keymap;			/* Character map for SET KEY (1:1)  */
extern MACRO *macrotab;			/* Macro map for SET KEY (1:string) */
#ifdef OS2
int wideresult;				/* For SET KEY, wide OS/2 scan codes */
extern int tt_scrsize[];		/* Scrollback buffer Sizes */
#endif /* OS2 */
#endif /* NOSETKEY */

extern char * printfile;		/* NULL if printer not redirected */
extern int printpipe;			/* For SET PRINTER */

#ifdef OS2
extern int tcp_avail;			/* Nonzero if TCP/IP is available */
#ifdef DECNET
extern int dnet_avail;			/* Ditto for DECnet */
#endif /* DECNET */
#ifdef SUPERLAT
extern int slat_avail;
#endif /* SUPERLAT */
#endif /* OS2 */

static struct keytab logintab[] = {
    "password", LOGI_PSW, CM_INV,
    "prompt",   LOGI_PRM, CM_INV,
    "userid",   LOGI_UID, 0
};

#ifndef NOCSETS
/* system-independent character sets, defined in ckcxla.[ch] */
extern struct csinfo tcsinfo[];
extern struct langinfo langs[];

/* Other character-set related variables */
extern int tcharset, tslevel, language;
#endif /* NOCSETS */ 

/* Declarations from cmd package */
 
#ifdef DCMDBUF
extern char *cmdbuf;			/* Command buffer */
extern char *line;
extern char *tmpbuf;
#else
extern char cmdbuf[];			/* Command buffer */
extern char line[];			/* Character buffer for anything */
extern char tmpbuf[];
#endif /* DCMDBUF */

/* From main ckuser module... */
 
extern char *tp, *lp;			/* Temporary buffer */

extern int tlevel;			/* Take Command file level */

#ifndef NOSPL
extern int cmdlvl;			/* Overall command level */
#endif /* NOSPL */

#ifndef NOLOCAL
#ifdef UNIX
extern int sessft;			/* Session-log file type */
#else
#ifdef OSK
extern int sessft;
#endif /* OSK */
#endif /* UNIX */
#endif /* NOLOCAL */

char * tempdir = NULL;

#ifdef VMS
int vms_msgs = 1;			/* SET MESSAGES */
#endif /* VMS */

/* Keyword tables for SET commands */
 
#ifdef CK_SPEED
struct keytab ctltab[] = {
    "prefixed",   1, 0,			/* Note, the values are important. */
    "unprefixed", 0, 0
};
#endif /* CK_SPEED */

#ifndef NOSPL
struct keytab outptab[] = {		/* SET OUTPUT parameters */
    "pacing", 0, 0			/* only one so far... */
};
#endif /* NOSPL */

struct keytab chktab[] = {		/* Block check types */
    "1", 1, 0,				/* 1 =  6-bit checksum */
    "2", 2, 0,				/* 2 = 12-bit checksum */
    "3", 3, 0,				/* 3 = 16-bit CRC */
    "blank-free-2", 4, 0		/* B = 12-bit checksum, no blanks */
};

struct keytab rpttab[] = {		/* SET REPEAT */
    "counts",    0, 0,			/* On or Off */
#ifdef COMMENT
    "minimum",   1, 0,			/* Threshhold */
#endif /* COMMENT */
    "prefix",    2, 0			/* Repeat-prefix character value */
};

#ifndef NOLOCAL
/* For SET [ MODEM ] CARRIER, and also for SET DIAL CONNECT */

struct keytab crrtab[] = {
    "auto", CAR_AUT, 0,			/* 2 */
    "off",  CAR_OFF, 0,			/* 0 */
    "on",   CAR_ON, 0			/* 1 */
};
int ncrr = 3;

#endif /* NOLOCAL */

struct keytab qvtab[] = {		/* Quiet/Verbose table */
    "quiet", 1, 0,
    "verbose", 0, 0
};
int nqvt = 2;

/* For SET DEBUG */

struct keytab dbgtab[] = {
    "off",     0,  0,
    "on",      1,  0,
    "session", 2,  0
};
int ndbg = 3;

#ifndef NOLOCAL
/* Transmission speeds */

/*
  Note, the values are encoded in cps rather than bps because 19200 and higher
  are too big for some ints.  All but 75bps are multiples of ten.  Result of
  lookup in this table must be multiplied by 10 to get actual speed in bps.
  If this number is 70, it must be changed to 75.  If it is 888, this means
  75/1200 split speed.

  The values are generic, rather than specific to UNIX.  We can't use B75,
  B1200, B9600, etc, because non-UNIX versions of C-Kermit will not
  necessarily have these symbols defined.  The BPS_xxx symbols are
  Kermit-specific, and are defined in ckcdeb.h or on the CC command line.

  Like all other keytabs, this one must be in "alphabetical" order,
  rather than numeric order.
*/
struct keytab spdtab[] = {
    "0",      0,  CM_INV,
    "110",   11,  0,
#ifdef BPS_115K
 "115200",11520,  0,
#endif /* BPS_115K */
  "1200",   120,  0,
#ifdef BPS_134
  "134.5",  134,  0,
#endif /* BPS_134 */
#ifdef BPS_14K
  "14400", 1440,  0,
#endif /* BPS_14K */
#ifdef BPS_150
  "150",     15,  0,
#endif /* BPS_150 */
#ifdef BPS_19K
  "19200", 1920,  0,
#endif /* BPS_19K */
#ifdef BPS_200
  "200",     20,  0,
#endif /* BPS_200 */
#ifdef BPS_230K
  "230400", 23040, 0,
#endif /* BPS_230K */
  "2400",   240,  0,
#ifdef BPS_28K
  "28800", 2880,  0,
#endif /* BPS_28K */
  "300",     30,  0,
#ifdef BPS_3600
  "3600",   360,  0,
#endif /* BPS_3600 */
#ifdef BPS_38K
  "38400", 3840,  0,
#endif /* BPS_38K */
  "4800",   480,  0,
#ifdef BPS_50
  "50",       5,  0,
#endif /* BPS_50 */
#ifdef BPS_57K
  "57600", 5760,  0,
#endif /* BPS_57K */
  "600",     60,  0,
#ifdef BPS_7200
  "7200",   720,  0,
#endif /* BPS_7200 */
#ifdef BPS_75
  "75",       7,  0,
#endif /* BPS_75 */
#ifdef BPS_7512
  "75/1200",888,  0,		/* Special code "888" for split speed */
#endif /* BPS_7512 */
#ifdef BPS_76K
  "76800", 7680,  0,
#endif /* BPS_76K */
  "9600",   960,  0
};
int nspd = (sizeof(spdtab) / sizeof(struct keytab)); /* How many speeds */
#endif /* NOLOCAL */

#ifndef NOCSETS
extern struct keytab lngtab[];		/* Languages for SET LANGUAGE */
extern int nlng;
#endif /* NOCSETS */

#ifndef NOLOCAL
/* Duplex keyword table */
 
struct keytab dpxtab[] = {
    "full", 	 0, 0,
    "half",      1, 0
};
#endif /* NOLOCAL */
 
/* SET FILE parameters */

struct keytab filtab[] = {
    "bytesize",         XYFILS, 0,
#ifndef NOCSETS
    "character-set",    XYFILC, 0,
#endif /* NOCSETS */
    "collision",        XYFILX, 0,
    "destination",      XYFILY, 0,
    "display",          XYFILD, CM_INV,
#ifdef CK_TMPDIR
    "download-directory", XYFILG, 0,
#endif /* CK_TMPDIR */
    "end-of-line",      XYFILA, 0,
    "eol",              XYFILA, CM_INV,
    "incomplete",       XYFILI, 0,
#ifdef CK_LABELED
    "label",            XYFILL, 0,
#endif /* CK_LABELED */
    "names",            XYFILN, 0,
#ifdef VMS
    "record-length",    XYFILR, 0,
#endif /* VMS */
    "type",             XYFILT, 0,
    "warning",          XYFILW, CM_INV
};
int nfilp = (sizeof(filtab) / sizeof(struct keytab));

/* Flow Control */

struct keytab flotab[] = {		/* SET FLOW-CONTROL keyword table */
    "automatic", FLO_AUTO, 0,
#ifdef CK_DTRCD
    "dtr/cd",    FLO_DTRC, 0,
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    "dtr/cts",   FLO_DTRT, 0,
#endif /* CK_DTRCTS */
    "keep",      FLO_KEEP, 0,
    "none",      FLO_NONE, 0,
#ifdef CK_RTSCTS
    "rts/cts",   FLO_RTSC, 0,
#endif /* CK_RTSCTS */
#ifndef Plan9
    "xon/xoff",  FLO_XONX, 0
#endif /* Plan9 */
};
int nflo = (sizeof(flotab) / sizeof(struct keytab));

/*  Handshake characters  */
 
struct keytab hshtab[] = {
    "bell", 007, 0,
    "code", 998, 0,
    "cr",   015, 0,
    "esc",  033, 0,
    "lf",   012, 0,
    "none", 999, 0,			/* (can't use negative numbers) */
    "xoff", 023, 0,
    "xon",  021, 0
};
int nhsh = (sizeof(hshtab) / sizeof(struct keytab));
 
#ifndef NOLOCAL
static struct keytab sfttab[] = {	/* File types for SET SESSION-LOG */
    "ascii",     XYFT_B, CM_INV,
    "binary",    XYFT_B, 0,
    "text",      XYFT_T, 0
};
static int nsfttab = (sizeof(sfttab) / sizeof(struct keytab));
#endif /* NOLOCAL */

#ifndef NODIAL

#ifdef NETCONN				/* Networks directory depends */
int nnetdir = 0;			/* on DIAL code -- fix later... */
char *netdir[MAXDDIR];
#endif /* NETCONN */

_PROTOTYP( static int setdial, (int) );
_PROTOTYP( static int setdcd, (void) );
_PROTOTYP( static int parsdir, (int) );
_PROTOTYP( static int cklogin, (void) );

#ifndef MINIDIAL
#ifdef OLDTBCODE
extern int tbmodel;			/* Telebit model ID */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */

extern MDMINF *modemp[];		/* Pointers to modem info structs */
extern struct keytab mdmtab[] ;		/* Modem types (in module ckudia.c) */
extern int nmdm;			/* Number of them */

_PROTOTYP(static int dialstr,(char **, char *));

extern int dialhng, dialtmo, dialksp, dialdpy, dialmhu, dialec, dialdc;
extern int dialrtr, dialint, dialudt, dialsrt, dialrstr, mdmwaitd;
extern int mdmspd, dialfc, dialmth, dialesc;

int dialcvt = 2;			/* DIAL CONVERT-DIRECTORY */
int dialcnf = 0;			/* DIAL CONFIRMATION */
int dialcon = 2;			/* DIAL CONNECT */
int dialcq  = 0;			/* DIAL CONNECT AUTO quiet/verbose */
extern long dialmax, dialcapas;
int usermdm = 0;
extern int ndialdir;
extern char *dialini,   *dialmstr, *dialmprmt, *dialdir[], *dialcmd,  *dialnpr,
 *dialdcon, *dialdcoff, *dialecon, *dialecoff, *dialhcmd,
 *dialhwfc, *dialswfc,  *dialnofc, *dialtone,  *dialpulse, *dialname, *diallac;
extern char *diallcc,   *dialixp,  *dialixs,   *dialldp,   *diallds,  *dialtfp,
 *dialpxx,  *dialpxi,   *dialpxo,  *dialsfx,   *dialaaon,  *dialaaoff;

#define MAXTOLLFREE 8
int ntollfree = 0;
char *dialtfc[MAXTOLLFREE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static struct keytab drstrtab[] = {
    "international", 5, 0,
    "local",         2, 0,
    "long-distance", 4, 0,
    "none",          6, 0
};

static struct keytab dcnvtab[] = {
    "ask",  2, 0,
    "off",  0, 0,
    "on",   1, 0
};

struct keytab setmdm[] = {
    "capabilities",	XYDCAP,  0,
    "carrier-watch",	XYDMCD,  0,
    "command",		XYDSTR,  0,
    "compression",	XYDDC,   0,
    "dial-command",     XYDDIA,  0,
    "error-correction",	XYDEC,   0,
    "escape-character",	XYDESC,  0,
    "flow-control",	XYDFC,   0,
    "hangup-method",	XYDMHU,  0,
    "kermit-spoof",	XYDKSP,  0,
    "maximum-speed",	XYDMAX,  0,
    "name",		XYDNAM,  0,
    "speed-matching",	XYDSPD,  0,
    "type",		XYDTYP,  0
};
int nsetmdm = (sizeof(setmdm) / sizeof(struct keytab));

struct keytab mdmcap[] = {
    "at-commands",	CKD_AT,  0,
    "compression",      CKD_DC,  0,
    "dc",               CKD_DC,  CM_INV,
    "ec",               CKD_EC,  CM_INV,
    "error-correction", CKD_EC,  0,
    "hardware-flow",    CKD_HW,  0,
    "hwfc",             CKD_HW,  CM_INV,
    "itu",              CKD_V25, CM_INV,
    "kermit-spoof",	CKD_KS,  0,
    "ks",               CKD_KS,  CM_INV,
    "sb",               CKD_SB,  CM_INV,
    "software-flow",    CKD_SW,  0,
    "speed-buffering",  CKD_SB,  0,
    "swfc",             CKD_SW,  CM_INV,
    "tb",               CKD_TB,  CM_INV,
    "telebit",          CKD_TB,  0,
    "v25bis-commands",	CKD_V25, 0
};
int nmdmcap = (sizeof(mdmcap) / sizeof(struct keytab));

struct keytab dialtab[] = {		/* SET DIAL table */
    "area-code",        XYDLAC, 0,	/* Also still includes items     */
    "compression",	XYDDC,  CM_INV,	/* that were moved to SET MODEM, */
    "confirmation",	XYDCNF, 0,	/* but they are CM_INVisible...  */
    "connect",          XYDCON, 0,
    "convert-directory",XYDCVT, 0,
    "country-code",     XYDLCC, 0,
    "dial-command",	XYDDIA, CM_INV,
    "directory",	XYDDIR, 0,
    "display",		XYDDPY, 0,
    "escape-character", XYDESC, CM_INV,
    "error-correction",	XYDEC,  CM_INV,
    "flow-control",	XYDFC,  CM_INV,
    "hangup",		XYDHUP, 0,
    "interval",		XYDINT, 0,
    "in",		XYDINI, CM_INV|CM_ABR,
    "init-string",	XYDINI, CM_INV,
    "intl-prefix",	XYDIXP, 0,
    "intl-suffix",	XYDIXS, 0,
    "kermit-spoof",	XYDKSP, CM_INV,
    "local-area-code",  XYDLAC, CM_INV,
    "ld-prefix",	XYDLDP, 0,
    "ld-suffix",	XYDLDS, 0,
    "m",                XYDMTH, CM_INV|CM_ABR,
#ifdef MDMHUP
    "me",               XYDMTH, CM_INV|CM_ABR,
#endif /* MDMHUP */
    "method",		XYDMTH, 0,
    "mnp-enable",	XYDMNP, CM_INV,	/* obsolete but still accepted */
#ifdef MDMHUP
    "modem-hangup",	XYDMHU, CM_INV,
#endif /* MDMHUP */
    "pbx-exchange",     XYDPXX, 0,
    "pbx-inside-prefix",XYDPXI, 0,
    "pbx-outside-prefix",XYDPXO, 0,
    "prefix",		XYDNPR,  0,
    "restrict",         XYDRSTR, 0,
    "retries",		XYDRTM,  0,
    "sort",             XYDSRT,  0,
    "speed-matching",	XYDSPD, CM_INV,
    "string",		XYDSTR, CM_INV,
    "suffix",		XYDSFX, 0,
    "timeout",		XYDTMO, 0,
    "tf-area-code",     XYDTFC, CM_INV,
    "tf-prefix",        XYDTFP, CM_INV,
    "toll-free-area-code",XYDTFC, 0,
    "toll-free-prefix", XYDTFP, 0
};
int ndial = (sizeof(dialtab) / sizeof(struct keytab));

#ifdef MDMHUP
struct keytab mdmhang[] = {
    "modem-command", 1, 0,
    "rs232-signal",  0, 0,
    "v24-signal",    0, CM_INV
};
#endif /* MDMHUP */

struct keytab dial_str[] = {
    "autoanswer",       XYDS_AN, 0,	/* autoanswer */
    "compression",	XYDS_DC, 0,	/* data compression */
    "dial-mode-prompt", XYDS_MP, 0, /* dial mode prompt */
    "dial-mode-string", XYDS_MS, 0, /* dial mode string */
    "error-correction",	XYDS_EC, 0,	/* error correction */
    "hangup-command",	XYDS_HU, 0,	/* hangup command */
    "hardware-flow",	XYDS_HW, 0,	/* hwfc */
    "init-string",	XYDS_IN, 0,	/* init string */
    "no-flow-control",	XYDS_NF, 0,	/* no flow control */
    "pulse",            XYDS_DP, 0,	/* pulse */
    "software-flow",	XYDS_SW, 0,	/* swfc */
    "tone",             XYDS_DT, 0	/* tone */
};
int ndstr = (sizeof(dial_str) / sizeof(struct keytab));

struct keytab dial_fc[] = {
    "auto",     FLO_AUTO, 0,
    "none",     FLO_NONE, 0,
    "rts/cts",  FLO_RTSC, 0,
    "xon/xoff", FLO_XONX, 0
};

struct keytab dial_m[] = {		/* DIAL METHOD */
    "default", XYDM_D, 0,
    "pulse",   XYDM_P, 0,
    "tone",    XYDM_T, 0
};
                           
#ifdef CK_TAPI
struct keytab tapitab[] = {		/* SET Microsoft TAPI */
   "configure-line",     XYTAPI_CFG,  CM_INV,
   "dialing-properties", XYTAPI_DIAL, CM_INV,
   "line",               XYTAPI_LIN, 0,
   "location",           XYTAPI_LOC, 0
};
int ntapitab = (sizeof(tapitab)/sizeof(struct keytab)) ;

struct keytab * tapiloctab = NULL;	/* Microsoft TAPI Locations */
int ntapiloc = 0;
extern struct keytab * tapilinetab;	/* Microsoft TAPI Line Devices */
extern int ntapiline;
#endif /* CK_TAPI */
#endif /* NODIAL */
 
#ifndef NOPUSH
extern int nopush;
#ifdef UNIX
struct keytab wildtab[] = {		/* SET WILDCARD-EXPANSION */
    "kermit",  0, 0,
    "shell",   1, 0
};
#endif /* UNIX */
#endif /* NOPUSH */

#ifdef NETCONN
extern struct keytab netcmd[], netkey[];
extern int nnets, nnetkey;
#ifdef TCPSOCKET
#ifdef SOL_SOCKET
extern struct keytab tcpopt[];
extern int ntcpopt;
#ifdef SO_LINGER
_PROTOTYP( int linger, (int,int) );
#endif /* SO_LINGER */
#ifdef TCP_NODELAY
_PROTOTYP( int no_delay, (int) );
#endif /* TCP_NODELAY */
#ifdef SO_SNDBUF
_PROTOTYP( int sendbuf, (int) );
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
_PROTOTYP( int recvbuf, (int) );
#endif /* SO_RCVBUF */
#ifdef SO_KEEPALIVE
_PROTOTYP( int keepalive, (int) );
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */
#endif /* TCPSOCKET */
#ifdef NPIPE
char pipename[PIPENAML+1] = { NUL, NUL };
#endif /* NPIPE */
#ifdef CK_NETBIOS
extern unsigned char NetBiosName[];
#endif /* CK_NETBIOS */
#endif /* NETCONN */

#ifdef ANYX25
struct keytab x25tab[] = {
    "call-user-data",    XYUDAT, 0,
    "closed-user-group", XYCLOS, 0,
    "reverse-charge",    XYREVC, 0  
};
int nx25 = (sizeof(x25tab) / sizeof(struct keytab));

struct keytab padx3tab[] = {
    "break-action",         PAD_BREAK_ACTION,           0,
    "break-character",      PAD_BREAK_CHARACTER,        0,
    "character-delete",     PAD_CHAR_DELETE_CHAR,       0,
    "cr-padding",           PAD_PADDING_AFTER_CR,       0,
    "discard-output",       PAD_SUPPRESSION_OF_DATA,    0,
    "echo",                 PAD_ECHO,                   0,
    "editing",              PAD_EDITING,                0,
    "escape",               PAD_ESCAPE,                 0,
    "forward",              PAD_DATA_FORWARD_CHAR,      0,
    "lf-padding",           PAD_PADDING_AFTER_LF,       0,
    "lf-insert",            PAD_LF_AFTER_CR,            0,
    "line-delete",          PAD_BUFFER_DELETE_CHAR,     0,
    "line-display",         PAD_BUFFER_DISPLAY_CHAR,    0,
    "line-fold",            PAD_LINE_FOLDING,           0,
    "pad-flow-control",     PAD_FLOW_CONTROL_BY_PAD,    0,
    "service-signals",      PAD_SUPPRESSION_OF_SIGNALS, 0,
    "timeout",              PAD_DATA_FORWARD_TIMEOUT,   0,
/* Speed is read-only */
    "transmission-rate",    PAD_LINE_SPEED,             0,
    "user-flow-control",    PAD_FLOW_CONTROL_BY_USER,   0
};
int npadx3 = (sizeof(padx3tab) / sizeof(struct keytab));
#endif /* ANYX25 */

/* Parity keyword table */
 
struct keytab partbl[] = {
    "even",    'e', 0,
    "mark",    'm', 0,
    "none",     0 , 0,
    "odd",     'o', 0,
    "space",   's', 0
};
int npar = (sizeof(partbl) / sizeof(struct keytab));
 
/* On/Off table */
 
struct keytab onoff[] = {
    "off",       0, 0,
    "on",        1, 0
};
 
static
struct keytab xittab[] = {		/* SET EXIT */
    "on-disconnect", 2, 0,		/* ...ON-DISCONNECT */
    "status",        0, 0,		/* ...STATUS */
    "warning",       1, 0		/* ...WARNING */
};
int nexit = (sizeof(xittab) / sizeof(struct keytab));

struct keytab xitwtab[] = {		/* SET EXIT WARNING */
    "always", 2, 0,                     /* even when not connected */
    "off",    0, 0,			/* no warning     */
    "on",     1, 0			/* when connected */
};
int nexitw = (sizeof(xitwtab) / sizeof(struct keytab));

struct keytab rltab[] = {
    "local",     1, 0,
    "off",       0, CM_INV,
    "on",        1, CM_INV,
    "remote",    0, 0
};
int nrlt = (sizeof(rltab) / sizeof(struct keytab));

/* Incomplete File Disposition table */
static
struct keytab ifdtab[] = {
    "discard",   0, 0,
    "keep",      1, 0
};

/* SET TAKE parameters table */
static
struct keytab taktab[] = {
    "echo",  0, 0,
    "error", 1, 0,
    "off",   2, CM_INV,			/* For compatibility */
    "on",    3, CM_INV			/* with MS-DOS Kermit... */
};

#ifndef NOSPL
static
struct keytab suftab[] = {		/* (what to do with) STARTUP-FILE */
    "delete", 1, 0,
    "keep",   0, 0
};

/* SET MACRO parameters table */
static
struct keytab smactab[] = {
    "echo",  0, 0,
    "error", 1, 0
};
#endif /* NOSPL */

#ifndef NOSCRIPT
static
struct keytab scrtab[] = {
    "echo",  0, 0
};
#endif /* NOSCRIPT */

/* SET COMMAND table */
struct keytab scmdtab[] = {
    "bytesize",  SCMD_BSZ, 0,
#ifdef OS2
    "color",     SCMD_COL, 0,
    "cursor-position", SCMD_CUR, 0,
#endif /* OS2 */
    "height",    SCMD_HIG, 0,
    "more-prompting",  SCMD_MOR, 0,
    "quoting",   SCMD_QUO, 0
#ifdef CK_RECALL
,   "recall-buffer-size", SCMD_RCL, 0
#endif /* CK_RECALL */
#ifdef CK_RECALL
,   "retry", SCMD_RTR, 0
#endif /* CK_RECALL */
#ifdef OS2
#ifdef ONETERMUPD
,  "scrollback",  SCMD_SCR, 0
#endif /* ONETERMUPD */
#endif /* OS2 */
,   "width",     SCMD_WID, 0
};
int nbytt = (sizeof(scmdtab) / sizeof(struct keytab));

#ifndef NOSERVER
/* Server parameters table */
struct keytab srvtab[] = {
    "display",  XYSERD, 0,
    "get-path", XYSERP, 0,
#ifdef OS2
    "idle-timeout", XYSERI, 0,
#endif /* OS2 */
    "login",    XYSERL, 0,
    "timeout",  XYSERT, 0
};
int nsrvt = (sizeof(srvtab) / sizeof(struct keytab));
#endif /* NOSERVER */

/* SET TRANSFER/XFER table */

struct keytab tstab[] = {
    "bell",          XYX_BEL,   0,
#ifdef XFRCAN
    "cancellation",  XYX_CAN,   0,
#endif /* XFRCAN */
#ifndef NOCSETS
    "character-set", XYX_CSE,   0,
#endif /* NOCSETS */
#ifndef NOSPL
    "crc-calculation", XYX_CRC, 0,
#endif /* NOSPL */
    "display",       XYX_DIS,   0,
    "locking-shift", XYX_LSH,   0,
    "mode",          XYX_MOD,   0,
#ifdef CK_XYZ
    "protocol",      XYX_PRO,   0,
#endif /* CK_XYZ */
    "slow-start",    XYX_SLO,   0,
    "", 0, 0
};
int nts = (sizeof(tstab) / sizeof(struct keytab)) - 1;

static struct keytab xfrmtab[] = {
    "automatic", XMODE_A, 0,
    "manual",    XMODE_M, 0
};

#ifndef NOCSETS
/* SET TRANSFER CHARACTER-SET table */

extern struct keytab tcstab[];
extern int ntcs;
#endif /* NOCSETS */

/* SET TRANSFER LOCKING-SHIFT table */
struct keytab lstab[] = {
    "forced", 2,   0,
    "off",    0,   0,
    "on",     1,   0
};
int nls = (sizeof(lstab) / sizeof(struct keytab));

/* SET TELNET tables */
#ifdef TNCODE
extern int tn_duplex, tn_nlm, tn_binary, tn_b_nlm, tn_b_meu, tn_b_ume;
extern char *tn_term;

static struct keytab tnbmtab[] = {	/* TELNET BINARY-MODE table */
    "accepted", TN_BM_AC, 0,
    "refused",  TN_BM_RF, 0,
    "requested", TN_BM_RQ, 0
};

static struct keytab tnbugtab[] = {	/* TELNET BUG table */
    "binary-me-means-u-too", 0, 0,
    "binary-u-means-me-too", 1, 0
};

#ifdef CK_ENVIRONMENT
static struct keytab tnenvtab[] = {	/* TELNET ENVIRONMENT table */
    "acct",	TN_ENV_ACCT,	0,
    "disp",	TN_ENV_DISP,	0,
    "job",	TN_ENV_JOB,	0,
    "printer",	TN_ENV_PRNT,	0,
    "systemtype",TN_ENV_SYS,	0,
    "user", 	TN_ENV_USR, 	0,
    "uservar",	TN_ENV_UVAR,	CM_INV
};
static int ntnenv = sizeof(tnenvtab)/sizeof(struct keytab) ;
#endif /* CK_ENVIRONMENT */

#define TN_NL_BIN 3
#define TN_NL_NVT 4
static struct keytab tn_nlmtab[] = {	/* TELNET NEWLINE-MODE table */
    "binary-mode", TN_NL_BIN, 0,              /* Binary mode */
    "nvt",    TN_NL_NVT, 0,                 /* NVT mode */
    "off",    TNL_CRNUL, CM_INV,		/* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  CM_INV,		/* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    CM_INV			/* CR only (out of spec) */
};
static int ntn_nlm = (sizeof(tn_nlmtab) / sizeof(struct keytab));

static struct keytab tnlmtab[] = {	/* TELNET NEWLINE-MODE table */
    "cr",     TNL_CR,    CM_INV,    /* CR only (out of spec) */
    "cr-lf",  TNL_CRLF,  CM_INV,    /* CR-LF (TELNET spec) */
    "cr-nul", TNL_CRNUL, CM_INV,    /* CR-NUL (TELNET spec) */
    "lf",     TNL_LF,    CM_INV,    /* LF instead of CR-LF */
    "off",    TNL_CRNUL, 0,		    /* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  0,		    /* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    0			/* CR only (out of spec) */
};
static int ntnlm = (sizeof(tnlmtab) / sizeof(struct keytab));

struct keytab tntab[] = {
    "binary-mode",     CK_TN_BM,   0,
    "bug",             CK_TN_BUG,  0,
    "echo",            CK_TN_EC,   0,
#ifdef CK_ENVIRONMENT
    "environment",     CK_TN_ENV,  0,
#endif /* CK_ENVIRONMENT */
    "newline-mode",    CK_TN_NL,   0,
    "terminal-type",   CK_TN_TT,   0
};
int ntn = (sizeof(tntab) / sizeof(struct keytab));
#endif /* TNCODE */

struct keytab ftrtab[] = {		/* Feature table */
#ifndef NOCSETS				/* 0 = we have it, 1 = we don't */
"character-sets",	0, 0,
#else
"character-sets",	1, 0,
#endif /* NOCSETS */
#ifndef NOCYRIL
"cyrillic",		0, 0,
#else
"cyrillic",		1, 0,
#endif /* NOCYRIL */

#ifndef NODEBUG
"debug",		0, 0,
#else
"debug",		1, 0,
#endif /* NODEBUG */

#ifndef NODIAL
"dial",			0, 0,
#else
"dial",			1, 0,
#endif /* NODIAL */

#ifdef DYNAMIC
"dynamic-memory",       0, 0,
#else
"dynamic-memory",       1, 0,
#endif /* DYNAMIC */

#ifdef XXFWD
"forward",              0, 0,
#else
"forward",              1, 0,
#endif /* XXFWD */

#ifdef CK_CURSES
"fullscreen-display",	0, 0,
#else
"fullscreen-display",	1, 0,
#endif /* CK_CURSES */
#ifdef HEBREW
"hebrew",               0, 0,
#else
"hebrew",               1, 0,
#endif /* HEBREW */
#ifndef NOHELP
"help",			0, 0,
#else
"help",			1, 0,
#endif /* NOHELP */
#ifndef NOSPL
"if-command",		0, 0,
#else
"if-command",		1, 0,
#endif /* NOSPL */
#ifndef NOJC
#ifdef UNIX
"job-control",		0, 0,
#else
"job-control",		1, 0,
#endif /* UNIX */
#else
"job-control",		1, 0,
#endif /* NOJC */
#ifdef KANJI
"kanji",		0, 0,
#else
"kanji",		1, 0,
#endif /* KANJI */
#ifndef NOCSETS
"latin1",		0, 0,
#else
"latin1",		1, 0,
#endif /* NOCSETS */
#ifdef LATIN2
"latin2",		0, 0,
#else
"latin2",		1, 0,
#endif /* LATIN2 */
#ifdef NETCONN
"network",		0, 0,
#else
"network",		1, 0,
#endif /* NETCONN */
#ifndef NOPUSH
"push",			0, 0,
#else
"push",			1, 0,
#endif /* PUSH */
#ifdef CK_RTSCTS
"rts/cts",		0, 0,
#else
"rts/cts",		1, 0,
#endif /* RTS/CTS */

#ifdef CK_REDIR
"redirect",             0, 0,
#else
"redirect",             1, 0,
#endif /* CK_REDIR */

#ifndef NOSCRIPT
"script-command",	0, 0,
#else
"script-command",	1, 0,
#endif /* NOSCRIPT */
#ifndef NOSERVER
"server-mode",		0, 0,
#else
"server-mode",		1, 0,
#endif /* NOSERVER */
#ifndef NOSHOW
"show-command",		0, 0,
#else
"show-command",		1, 0,
#endif /* NOSHOW */

#ifndef NOXMIT
"transmit",		0, 0,
#else
"transmit",		1, 0,
#endif /* NOXMIT */

#ifdef CK_XYZ
"xyzmodem",		0, 0,
#else
"xyzmodem",		1, 0,
#endif /* NOXMIT */

"", 0, 0
};
int nftr = (sizeof(ftrtab) / sizeof(struct keytab)) - 1;

struct keytab desttab[] = {		/* SET DESTINATION */
    "disk",    DEST_D, 0,
    "printer", DEST_P, 0,
    "screen",  DEST_S, 0
};
int ndests =  (sizeof(desttab) / sizeof(struct keytab));


#ifndef NOICP /* Used only with script programming items... */

#ifndef NOSERVER			/* This is just to avoid some */
#define CK_PARSDIR			/* "statement not reached" */
#else					/* complaints... */
#ifndef NODIAL
#define CK_PARSDIR
#endif /* NODIAL */
#endif /* NOSERVER */

/*
  cx == 0 means dial directory
  cx == 1 means network directory
  cx == 2 means a directory path list
*/
static int
parsdir(cx) int cx; {
    int i, x, y, z, dd;			/* Workers */
    int nxdir;
    char *s;
    char ** xdir;
    char *pp[MAXGETPATH];		/* Temporary name pointers */
#ifdef ZFNQFP
    char dirpath[1024];			/* For fully qualified filenames */
    struct zfnfp * fnp;
#ifdef OS2
    char * env;
#endif /* OS2 */
#endif /* ZFNQFP */

    int max = 0;			/* Maximum number of things to parse */
    char c;

#ifndef NODIAL
    if (cx == 0) {			/* Dialing */
	nxdir = ndialdir;
	xdir = dialdir;
	max = MAXDDIR;
    } else 
#ifdef NETCONN
    if (cx == 1) {			/* Network */
	nxdir = nnetdir;
	xdir = netdir;
	max = MAXDDIR;
    } else 
#endif /* NETCONN */
#endif /* NODIAL */
#ifndef NOSERVER
    if (cx == 2) {			/* GET path */
	nxdir = ngetpath;
	xdir = getpath;
	max = MAXGETPATH;
    } else				/* Called with invalid function code */
#endif /* NOSERVER */
      return(-2);

#ifdef CK_PARSDIR
    dd = 0;				/* Temporary name counter */
    while (1) {
	if (cx != 2) {			/* Dialing or Network Directory */
#ifdef OS2    
#ifdef NT
	    env = getenv("K95PHONES");
#else /* NT */
	    env = getenv("K2PHONES");
#endif /* NT */
	    if (!env)
	      env = getenv("K95PHONES");
	    sprintf(dirpath,"%s%s%s;%s%s;%s;%s%s;%s;%s%s", 
		    /* semicolon-separated path list */
		    env?env:"",
		    (env && env[strlen(env)-1]==';')?"":";",
		    startupdir,
		    startupdir, "PHONES/",
		    inidir,
		    inidir, "PHONES/",
		    exedir,
		    exedir, "PHONES/"
		    );
#else
#ifdef UNIX
	    y = 1024;
	    s = dirpath;
	    zzstring("\\v(home)",&s,&y);
#endif /* UNIX */
#endif /* OS2 */
	    y = cmifip(
		  "Names of one or more directory files, separated by spaces", 
		       "",&s,&x,0,
#ifdef OS2ORUNIX
		       dirpath,
#else
		       NULL,
#endif /* OS2ORUNIX */
		       xxstring
		       );
	} else {
	    x = 0;
	    y = cmdir("Directory name","",&s,xxstring);
	}
	if (y < 0) {
	    if (y == -3) {		/* EOL or user typed <CR> */
		for (i = 0; i < max; i++) { /* Clear these */
		    if (i < nxdir && xdir[i]) {
			free(xdir[i]);
		    }
		    xdir[i] = (i < dd) ? pp[i] : NULL;
		}
#ifndef NODIAL
		if (cx == 0)
		  ndialdir = dd;
#ifdef NETCONN
		if (cx == 1)
		  nnetdir = dd;
#endif /* NETCONN */
#endif /* NODIAL */
#ifndef NOSERVER
		if (cx == 2)
		  ngetpath = dd;
#endif /* NOSERVER */
		return(success = 1);

	    } else {			/* Parse error */
		for (i = 0; i < dd; i++) {  /* Free temp storage */
		    if (pp[i]) free(pp[i]); /* but don't change */
		    pp[i] = NULL;           /* anything else */
		}
		return(y);
	    }
	}
	if (x) {
	    printf("?Wildcards not allowed\n");
	    return(-9);
	}
#ifdef CK_TMPDIR
	if (cx == 2 && !isdir(s)) {
	    printf("?Not a directory - %s\n", s);
	    return(-9);
	}
#endif /* CK_TMPDIR */

#ifdef ZFNQFP
	/* Get fully qualified pathname */
	if (fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf)) {
	    if (fnp->fpath)
	      if ((int) strlen(fnp->fpath) > 0)
		s = fnp->fpath;
	}
#endif /* ZFNQFP */
	c = NUL;
	x = strlen(s);
	if (x > 0)			/* Get last char */
	  c = s[x-1];
	debug(F000,"parsdir s",s,c);
	if ((pp[dd] = malloc(strlen(s)+2)) == NULL) {
	    printf("?Internal error - malloc\n");
	    for (i = 0; i < dd; i++) {  /* Free temp storage */
		if (pp[i]) free(pp[i]);
		pp[i] = NULL;
	    }
	    return(-9);
	} else {			/* Have storage for name */
	    strcpy(pp[dd],s);		/* Copy string into new storage */
	    debug(F111,"parsdir pp[dd] 1",pp[dd],dd);
	    if (cx == 2) {		/* If we are parsing directories */
		char dirsep[2];
		extern int myindex;	/* Append directory separator if */
		extern struct sysdata sysidlist[]; /* it is missing...   */
		debug(F101,"parsdir myindex","",myindex);
		if (myindex > -1)
		  if (sysidlist[myindex].sid_unixlike)
		    if (c != sysidlist[myindex].sid_dirsep) {
			dirsep[0] = sysidlist[myindex].sid_dirsep;
			dirsep[1] = NUL;
			strcat(pp[dd], (char *) dirsep);
		    }
	    }
	    debug(F111,"parsdir pp[dd] 2",pp[dd],dd);
	    if (++dd > max) {
		printf("?Too many directories - %d max\n", max);
		for (i = 0; i < dd; i++) {  /* Free temp storage */
		    if (pp[i]) free(pp[i]);
		    pp[i] = NULL;
		}
	    }
	}
    }
#endif /* CK_PARSDIR */
}
#endif /* NOICP */

#ifndef NOSERVER
static int
cklogin() {
    int x;
    char * s;
    char username[LOGINLEN+1];
    char password[LOGINLEN+1];
    char account[LOGINLEN+1];
    extern char * x_user, * x_passwd, * x_acct;
    extern int x_login, x_logged;

    username[0] = NUL;
    password[0] = NUL;
    account[0]  = NUL;

    x = cmfld("username", "", &s, xxstring);
    if (x != -3) {
	if (x < 0)
	  return(x);
	if ((int)strlen(s) > LOGINLEN) {
	    printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
	    return(-9);
	}
	strcpy(username,s);
	x = cmfld("password", "", &s, xxstring);
	if (x != -3) {
	    if (x < 0)
	      return(x);
	    if ((int)strlen(s) > LOGINLEN) {
		printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
		return(-9);
	    }
	    strcpy(password,s);
	    x = cmfld("account", "", &s, xxstring);
	    if (x != -3) {
		if (x < 0)
		  return(x);
		if ((int)strlen(s) > LOGINLEN) {
		    printf("\"%s\" - too long, %d max\n", s, LOGINLEN);
		    return(-9);
		}
		strcpy(account,s);
		if ((x = cmcfm()) < 0)
		  return(x);
	    }
	}
    }
    makestr(&x_user,username);
    makestr(&x_passwd,password);
    makestr(&x_acct,account);
    x_login = (x_user) ? 1 : 0;
    x_logged = 0;
    return(1);
}
#endif /* NOSERVER */

#ifndef NOLOCAL
VOID
setflow() {
#ifndef NODIAL
    MDMINF * p = NULL;
    long bits = 0;
#endif /* NODIAL */

    if (!autoflow)			/* Only if FLOW is AUTO */
      return;
#ifdef VMS
    flow = FLO_XONX;			/* Special for VMS */
    return;
#endif /* VMS */

    if (network)			/* Network connection */
      flow = FLO_NONE;			/* None for all non-VMS */

    if (mdmtyp < 1) {			/* Direct connection */
#ifdef CK_RTSCTS			/* If we can do RTS/CTS flow control */
	flow = FLO_RTSC;		/* then do it */
#else					/* Otherwise */
	flow = FLO_XONX;		/* Use Xon/Xoff. */
#endif /* CK_RTSCTS */
	return;
    }
#ifndef NODIAL
    bits = dialcapas;			/* Capability bits */
    if (!bits) {			/* No bits? */
	p = modemp[mdmtyp - 1];		/* Look in modem info structure */
	if (p)
	  bits = p->capas;
    }
    if (dialfc == FLO_AUTO) {		/* If DIAL flow is AUTO */
#ifdef CK_RTSCTS			/* If we can do RTS/CTS flow control */
	if (bits & CKD_HW)		/* and modem can do it too */
	  flow = FLO_RTSC;		/* then switch to RTS/CTS */
	else				/* otherwise */
	  flow = FLO_XONX;		/* use Xon/Xoff. */
#else
	flow = FLO_XONX;		/* No RTS/CTS so use Xon/Xoff. */
#endif /* CK_RTSCTS */
    }
#endif /* NODIAL */
    return;
}

static int
setdcd() {
    int x, y, z;
    if ((y = cmkey(crrtab,ncrr,"","auto",xxstring)) < 0) return(y);
    if (y == CAR_ON) {
	x = cmnum("Carrier wait timeout, seconds","0",10,&z,xxstring);
	if (x < 0) return(x);
    }
    if ((x = cmcfm()) < 0) return(x);
    carrier = ttscarr(y);
    cdtimo = z;
    return(1);
}
#endif /* NOLOCAL */

extern struct keytab yesno[];
extern int nyesno;

int
getyesno(msg) char * msg; {		/* Ask question, get yes/no answer */

    int x, y, z;

#ifdef VMS
/*
  In VMS, whenever a TAKE file or macro is active, we restore the 
  original console modes so Ctrl-C/Ctrl-Y can work.  But here we
  go interactive again, so we have to temporarily put them back.
*/
    if (cmdlvl > 0)
      concb((char)escape);
#endif /* VMS */
      
    cmsavp(psave,PROMPTL);		/* Save old prompt */
    cmsetp(msg);			/* Make new prompt */
    z = 0;				/* Initialize answer to No. */
    cmini(ckxech);			/* Initialize parser. */
    do {
	prompt(NULL);			/* Issue prompt. */
	y = cmkey(yesno,nyesno,"","",NULL); /* Get Yes or No */
	if (y < 0) {
	    if (y == -4) {		/* EOF */
		z = y;
		break;
	    } else if (y == -3)		/* No answer? */
	      printf(" Please respond Yes or No\n");
	    cmini(ckxech);
	} else {
	    z = y;			/* Save answer */
	    y = cmcfm();		/* Get confirmation */
	}
    } while (y < 0);			/* Continue till done */
    cmsetp(psave);			/* Restore real prompt */
#ifdef VMS
    if (cmdlvl > 0)			/* In VMS and not at top level, */
      conres();				/*  restore console again. */
#endif /* VMS */
    return(z);
}

int					/* CHECK command */
dochk() {
    int x, y;
    if ((y = cmkey(ftrtab,nftr,"","",xxstring)) < 0)
      return(y);
    strcpy(line,atmbuf);
    if ((y = cmcfm()) < 0)
      return(y);
#ifndef NOPUSH
    if (!xxstrcmp(atmbuf,"push",(int)strlen(atmbuf))) {
	if (msgflg)			/* If at top level... */
	  printf(" push%s available\n", nopush ? " not" : "");
	else if (nopush && !backgrd)
	  printf(" CHECK: push not available\n");
	return(success = 1 - nopush);
    }
#endif /* NOPUSH */
    y = lookup(ftrtab,line,nftr,&x);	/* Look it up */
    if (msgflg)				/* If at top level... */
      printf(" %s%s available\n", ftrtab[x].kwd, y ? " not" : "");
    else if (y && !backgrd)
      printf(" CHECK: %s not available\n", ftrtab[x].kwd);
    return(success = 1 - y);
}

#ifndef NODIAL
/*
  Parse a DIAL-related string, stripping enclosing braces, if any.
*/
static int
dialstr(p,msg) char **p; char *msg; {
    int x;
    char *s;

    if ((x = cmtxt(msg, "", &s, xxstring)) < 0)
      return(x);
    s = brstrip(s);			/* Strip braces around. */
    makestr(p,s);
    return(success = 1);
}

VOID
initmdm(x) int x; {
    MDMINF * p, * u;
    int m;

    mdmtyp = x;				/* Set global modem type */
    debug(F101,"initmdm mdmtyp","",mdmtyp);
    debug(F101,"initmdm usermdm","",usermdm);
    if (x < 1) return;

    m = usermdm ? usermdm : mdmtyp;

    p = modemp[m - 1];			/* Point to modem info struct, and */
    debug(F101,"initmdm p","",p);
    if (p) {
	dialec = p->capas & CKD_EC;	/* set DIAL ERROR-CORRECTION, */
	dialdc = p->capas & CKD_DC;	/* DIAL DATA-COMPRESSION, and */
	mdmspd = p->capas & CKD_SB ? 0 : 1; /* DIAL SPEED-MATCHING from it. */
	dialfc = FLO_AUTO;		    /* Modem's local flow control.. */
    } else if (mdmtyp > 0) {
	printf("WARNING: modem info for \"%s\" not filled in yet\n",
	       gmdmtyp()
	       );
    }

/* Reset or set the SET DIAL STRING items ... */

    if (usermdm && p) {	/* USER-DEFINED: copy info from specified template */

	makestr(&dialini,p->wake_str);
	makestr(&dialmstr,p->dmode_str);
	makestr(&dialmprmt,p->dmode_prompt);
	makestr(&dialcmd,p->dial_str);
	makestr(&dialdcon,p->dc_on_str);
	makestr(&dialdcoff,p->dc_off_str);
	makestr(&dialecon,p->ec_on_str);
	makestr(&dialecoff,p->ec_off_str);
	makestr(&dialhcmd,p->hup_str);
	makestr(&dialhwfc,p->hwfc_str);
	makestr(&dialswfc,p->swfc_str);
	makestr(&dialnofc,p->nofc_str);
	makestr(&dialtone,p->tone);
	makestr(&dialpulse,p->pulse);
	makestr(&dialname,"This space available (use SET MODEM NAME)");
	dialmax   = p->max_speed;
	dialcapas = p->capas;
	dialesc   = p->esc_char;

    } else {			/* Not user-defined, so wipe out overrides */
	
	if (dialini)   free(dialini);   dialini =   NULL; /* Init-string */
	if (dialmstr)  free(dialmstr);  dialmstr =  NULL; /* Dial-mode-str */
	if (dialmprmt) free(dialmprmt); dialmprmt = NULL; /* Dial-mode-pro */
	if (dialcmd)   free(dialcmd);   dialcmd =   NULL; /* Dial-command  */
	if (dialdcon)  free(dialdcon);  dialdcon =  NULL; /* DC ON command */
	if (dialdcoff) free(dialdcoff); dialdcoff = NULL; /* DC OFF command */
	if (dialecon)  free(dialecon);  dialecon =  NULL; /* EC ON command */
	if (dialecoff) free(dialecoff); dialecoff = NULL; /* EC OFF command */
	if (dialhcmd)  free(dialhcmd);  dialhcmd =  NULL; /* Hangup command */
	if (dialhwfc)  free(dialhwfc);  dialhwfc =  NULL; /* Flow control... */
	if (dialswfc)  free(dialswfc);  dialswfc =  NULL;
	if (dialnofc)  free(dialnofc);  dialnofc =  NULL;
	if (dialtone)  free(dialtone);  dialtone =  NULL; /* Dialing method */
	if (dialpulse) free(dialpulse); dialpulse = NULL;
	if (dialname)  free(dialname);  dialname =  NULL; /* Modem name */
    }
    if (autoflow)			/* Maybe change flow control */
      setflow();

#ifndef MINIDIAL
#ifdef OLDTBCODE
    tbmodel = 0;           /* If it's a Telebit, we don't know the model yet */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
}

int
setmodem() {				/* SET MODEM */

    int x, y, z;
    long zz;
    char *s;

    if ((x = cmkeyx(setmdm,nsetmdm,"modem parameter","", xxstring)) < 0) {
	debug(F111,"setmodem cmkeyx","atmbuf",x);
	if (x == -9) {
	    extern int cmflgs;
	    debug(F101,"setmodem cmflgs","",cmflgs);
	    if (!atmbuf[0])
	      return(-3);
	    y = lookup(mdmtab,atmbuf,nmdm,&x); /* Maybe old SET MODEM <type> */
	    if (y == -2) {		       /* Look up in modem table.... */
		printf("?Ambiguous modem type - %s\n",atmbuf);
		return(-9);
	    } else if (y < 0) {
		printf("?\"%s\" does not match a keyword or modem type\n",
		       atmbuf);
		return(-9);
	    }
	    if (!cmflgs)
	      if ((x = cmcfm()) < 0)	/* Confirm */
		return(x);
	    usermdm = 0;
	    initmdm(y);			/* Set it. */
	    return(success = 1);	/* Done */
	} else return(x);
    }
    switch (x) {
#ifdef MDMHUP
      case XYDMHU:			/* DIAL MODEM-HANGUP */
	if ((y = cmkey(mdmhang,3,"how to hang up modem",
		       "modem-command", xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0)
	  return(x);
	dialmhu = y;
	return(success = 1);
#endif /* MDMHUP */

      case XYDCAP:
	zz = 0L;
	y = 0;
	while (y != -3) {
	    if ((y = cmkey(mdmcap,nmdmcap,
			   "capability of modem", "", xxstring)) < 0) {
		if (y == -3)
		  break;
		else
		  return(y);
	    }
	    zz |= y;
	}
	dialcapas = zz;
	if (autoflow)			/* Maybe change flow control */
	  setflow();
	return(success = 1);

      case XYDMAX:
	if ((x = cmkey(spdtab,nspd,line,"",xxstring)) < 0) {
	    if (x == -3) printf("?value required\n");
	    return(x);
	}
	if ((y = cmcfm()) < 0) return(y);
	dialmax = (long) x * 10L;
	if (dialmax == 70) dialmax = 75;
	return(success = 1);

      case XYDSTR:			/* These moved from SET DIAL */
      case XYDDC:
      case XYDEC:
      case XYDESC:
      case XYDFC:
      case XYDKSP:
      case XYDSPD:
      case XYDDIA:
	return(setdial(x));

      case XYDTYP:
	if ((y = cmkey(mdmtab,nmdm,"modem type","none", xxstring)) < 0)
	  return(y);
	if (y == dialudt) {		/* User-defined modem type */
	    if ((x = cmkey(mdmtab,nmdm,"based on existing modem type",
			   "unknown", xxstring)) < 0)
	      return(x);
	}
	if ((z = cmcfm()) < 0)
	  return(z);
	usermdm = 0;
	usermdm = (y == dialudt) ? x : 0;
	initmdm(y);
	return(success = 1);

      case XYDNAM:
	return(dialstr(&dialname,"Descriptive name for modem"));

      case XYDMCD:			/* SET MODEM CARRIER */
	return(success = setdcd());

      default:
	printf("Unexpected SET MODEM parameter\n");
	return(-9);
    }
}

static int				/* Set DIAL command options */
setdial(y) int y; {
    int x, z;
    char *s;

    if (y < 0)
      if ((y = cmkey(dialtab,ndial,"","",xxstring)) < 0)
	return(y);
    switch (y) {
      case XYDHUP:			/* DIAL HANGUP */
	return(seton(&dialhng));
      case XYDINI:			/* DIAL INIT-STRING */
	return(dialstr(&dialini,"Modem initialization string"));
      case XYDNPR:			/* DIAL PREFIX */
	return(dialstr(&dialnpr,"Telephone number prefix"));
      case XYDDIA:			/* DIAL DIAL-COMMAND */
	x = cmtxt("Dialing command for modem,\n\
 include \"%s\" to stand for phone number,\n\
 for example, \"set dial dial-command ATDT%s\\13\"",
		  "",
		  &s,
		  xxstring);
	if (x < 0 && x != -3)		/* Handle parse errors */
	  return(x);
	y = x = strlen(s);		/* Get length of text */
	if (x > 0 && *s == '{') {	/* Strip enclosing braces, */
	    if (s[x-1] == '}') {	/* if any. */
		s[x-1] = NUL;
		s++;
		y -= 2;
	    }
	}
	if (y > 0) {			/* If there is any text (left), */
	    for (x = 0; x < y; x++) {	/* make sure they included "%s" */
		if (s[x] != '%') continue;
		if (s[x+1] == 's') break;
	    }
	    if (x == y) {
		printf(
"?Dial-command must contain \"%cs\" for phone number.\n",'%');
		return(-9);
	    }
	}
	if (dialcmd) {			/* Free any previous string. */
	    free(dialcmd);
	    dialcmd = (char *) 0;
	}	
	if (y > 0) {
	    dialcmd = malloc(y + 1);	/* Allocate space for it */
	    strcpy(dialcmd,s);		/* and make a safe copy. */
	}
	return(success = 1);
      case XYDKSP:			/* DIAL KERMIT-SPOOF */
	return(seton(&dialksp));
      case XYDTMO:			/* DIAL TIMEOUT */
	y = cmnum("Seconds to wait for call completion","0",10,&x,xxstring);
	if (y < 0) return(y);
	y = cmnum("Kermit/modem timeout differential","10",10,&z,xxstring);
	if (y < 0) return(y);	
	if ((y = cmcfm()) < 0)
	  return(y);
	dialtmo = x;
	mdmwaitd = z;
      case XYDESC:			/* DIAL ESCAPE-CHARACTER */
	y = cmnum("ASCII value of character to escape back to modem",
		  "43",10,&x,xxstring);
	return(setnum(&dialesc,x,y,128));
      case XYDDPY:			/* DIAL DISPLAY */
	return(seton(&dialdpy));
      case XYDSPD:			/* DIAL SPEED-MATCHING */
					/* used to be speed-changing */
	if ((y = seton(&mdmspd)) < 0) return(y);
#ifdef COMMENT
	mdmspd = 1 - mdmspd;		/* so here we reverse the meaning */
#endif /* COMMENT */
	return(success = 1);
      case XYDMNP:			/* DIAL MNP-ENABLE */
      case XYDEC:			/* DIAL ERROR-CORRECTION */
	x = seton(&dialec);
	if (x > 0)
	  if (!dialec) dialdc = 0;	/* OFF also turns off compression */
	return(x);

      case XYDDC:			/* DIAL COMPRESSION */
	x = seton(&dialdc);
	if (x > 0)
	  if (dialdc) dialec = 1;	/* ON also turns on error correction */
	return(x);

#ifdef MDMHUP
      case XYDMHU:			/* DIAL MODEM-HANGUP */
	return(seton(&dialmhu));
#endif /* MDMHUP */

      case XYDDIR:			/* DIAL DIRECTORY (zero or more) */
	return(parsdir(0));		/* 0 means DIAL */

      case XYDSTR:			/* DIAL STRING */
	if ((y = cmkey(dial_str,ndstr,"","",xxstring)) < 0) return(y);
	switch (y) {
	  case XYDS_AN:			/* Autoanswer ON/OFF */
	  case XYDS_DC:			/* Data compression ON/OFF */
	  case XYDS_EC:			/* Error correction ON/OFF */
	    if ((x = cmkey(onoff,2,"","on",xxstring)) < 0) return(x);
	    sprintf(tmpbuf,"Modem's command to %sable %s",
		    x ? "en" : "dis",
		    (y == XYDS_DC) ? "compression" :
		    ((y == XYDS_EC) ? "error-correction" :
		    "autoanswer")
		    );
	    if (x) {
		if (y == XYDS_DC)
		  return(dialstr(&dialdcon,tmpbuf));
		else if (y == XYDS_EC)
		  return(dialstr(&dialecon,tmpbuf));
		else
		  return(dialstr(&dialaaon,tmpbuf));
	    } else {
		if (y == XYDS_DC)
		  return(dialstr(&dialdcoff,tmpbuf));
		else if (y == XYDS_EC)
		  return(dialstr(&dialecoff,tmpbuf));
		else
		  return(dialstr(&dialaaoff,tmpbuf));
	    }
	  case XYDS_HU:			/*    hangup command */
	    return(dialstr(&dialhcmd,"Modem's hangup command"));
	  case XYDS_HW:			/*    hwfc */
	    return(dialstr(&dialhwfc,
			   "Modem's command to enable hardware flow control"));
	  case XYDS_IN:			/*    init */
	    return(dialstr(&dialini,"Modem's initialization string"));
	  case XYDS_NF:			/*    no flow control */
	    return(dialstr(&dialnofc,
			   "Modem's command to disable local flow control"));
	  case XYDS_PX:			/*    prefix */
	    return(dialstr(&dialnpr,"Telephone number prefix for dialing"));
	  case XYDS_SW:			/*    swfc */
	    return(dialstr(&dialswfc,
		   "Modem's command to enable local software flow control"));
	  case XYDS_DT:			/*    tone dialing */
	    return(dialstr(&dialtone,
		   "Command to configure modem for tone dialing"));
	  case XYDS_DP:			/*    pulse dialing */
	    return(dialstr(&dialpulse,
		   "Command to configure modem for pulse dialing"));
    case XYDS_MS:           /*    dial mode string */
         return(dialstr(&dialmstr,
                         "Command to enter dial mode"));
    case XYDS_MP:           /*    dial mode prompt */
         return(dialstr(&dialmprmt,
                         "Modem response upon entering dial mode"));
	  default:
	    printf("?Unexpected SET DIAL STRING parameter\n");
	}

      case XYDFC:			/* DIAL FLOW-CONTROL */
	if ((y = cmkey(dial_fc,4,"","auto",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	dialfc = y;
	return(success = 1);

      case XYDMTH:			/* DIAL METHOD */
	if ((y = cmkey(dial_m,3,"","default",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0)		/* DEFAULT means don't force */
	  return(x);			/* any particular method, use */
	dialmth = y;			/* modem's default method. */
	return(success = 1);

      case XYDRTM:
	y = cmnum("Number of times to try dialing a number",
		  "1",10,&x,xxstring);
	return(setnum(&dialrtr,x,y,16383));
	
      case XYDINT:
	y = cmnum("Seconds to wait between redial attempts",
		  "30",10,&x,xxstring);
	return(setnum(&dialint,x,y,128));

      case XYDLAC:			/* DIAL AREA-CODE */
	if ((x = dialstr(&diallac,"Area code you are calling from")) < 0)
	  return(x);
	if (diallac) {
	    if (!rdigits(diallac)) {
		printf("?Sorry, area code must be numeric\n");
		if (*diallac == '(')
		  printf("(please omit the parentheses)\n");
		if (*diallac == '/')
		  printf("(no slashes, please)\n");
		if (diallac) free(diallac);
		diallac = NULL;
		return(-9);
	    }
	}
	return(x);

      case XYDCNF:			/* CONFIRMATION */
	return(success = seton(&dialcnf));

      case XYDCVT:			/* CONVERT-DIRECTORY */
	if ((y = cmkey(dcnvtab,3,"","ask",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0)
	  return(x);
	dialcvt = y;
	return(success = 1);

      case XYDLCC:			/* DIAL COUNTRY-CODE */
	x = dialstr(&diallcc,"Country code you are calling from");
	if (x < 1) return(x);
	if (diallcc) {
	    if (!rdigits(diallcc)) {
		printf("?Sorry, country code must be numeric\n");
		if (*diallcc == '+')
		  printf("(please omit the plus sign)\n");
		if (diallcc) free(diallcc);
		diallcc = NULL;
		return(-9);
	    }
	    if (!strcmp(diallcc,"1")) {	/* Set defaults for USA and Canada */
		if (!dialldp)		/* Long-distance prefix */
		  makestr(&dialldp,"1");
		if (!dialixp) 		/* International dialing prefix */
		  makestr(&dialixp,"011");
		if (ntollfree == 0) {	/* Toll-free area codes */
		    if (dialtfc[0] = malloc(4)) {
			strcpy(dialtfc[0],"800");
			ntollfree++;
		    }
		    if (dialtfc[1] = malloc(4)) {
			strcpy(dialtfc[1],"888");
			ntollfree++;
		    }
		}
		if (!dialtfp) 		/* Toll-free dialing prefix */
		  makestr(&dialtfp,"1");
	    } else if (!strcmp(diallcc,"358") &&
		       ((int) strcmp(zzndate(),"19961011") > 0)
		       ) {		/* Finland */
		if (!dialldp)		/* Long-distance prefix */
		  makestr(&dialldp,"9");
		if (!dialixp) 		/* International dialing prefix */
		  makestr(&dialixp,"990");
	    } else {			/* Everywhere else ... */
		if (!dialldp) {
		    if (dialldp = malloc(4))
		      strcpy(dialldp,"0");
		}
		if (!dialixp) {
		    if (dialixp = malloc(4))
		      strcpy(dialixp,"00");
		}
	    }
	}
	return(success = 1);

      case XYDIXP:			/* DIAL INTL-PREFIX */
	return(dialstr(&dialixp,"International dialing prefix"));

      case XYDIXS:			/* DIAL INTL-SUFFIX */
	return(dialstr(&dialixs,"International dialing suffix"));

      case XYDLDP:			/* DIAL INTL-PREFIX */
	return(dialstr(&dialldp,"Long-distance dialing prefix"));

      case XYDLDS:			/* DIAL INTL-SUFFIX */
	return(dialstr(&diallds,"Long-distance dialing suffix"));

      case XYDPXX:			/* DIAL PBX-EXCHANGE */
	return(dialstr(&dialpxx,"Exchange of PBX you are calling from"));

      case XYDPXI:			/* DIAL PBX-INTERNAL-PREFIX */
	return(dialstr(&dialpxi,
		       "Internal-call prefix of PBX you are calling from"));

      case XYDPXO:			/* DIAL PBX-OUTSIDE-PREFIX */
	return(dialstr(&dialpxo,
		       "Outside-line prefix of PBX you are calling from"));

      case XYDSFX:			/* DIAL INTL-SUFFIX */
	return(dialstr(&dialsfx," Telephone number suffix for dialing"));

      case XYDSRT:			/* DIAL SORT */
	return(success = seton(&dialsrt));

      case XYDTFC: {			/* DIAL TOLL-FREE-AREA-CODE  */
	  int n, i;			/* (zero or more of them...) */
	  char * p[MAXTOLLFREE];	/* Temporary pointers */
	  for (n = 0; n < MAXTOLLFREE; n++) {
	      if ((x = cmfld(
		    "Toll-free area code in the country you are calling from",
		       "",&s,xxstring)) < 0)
		break;
	      if (s) {
		  int k;
		  k = (int) strlen(s);
		  if (k > 0) {
		      if (p[n] = malloc(k + 1))
			strcpy(p[n], s);
		  } else break;
	      } else break;
	  }
	  if (x == -3) {		/* Command was successful */
	      for (i = 0; i < ntollfree; i++) /* Remove old list, if any */
		if (dialtfc[i]) {
		    free(dialtfc[i]);
		    dialtfc[i] = NULL;
		}
	      ntollfree = n;		/* New count */
	      for (i = 0; i < ntollfree; i++) /* New list */
		dialtfc[i] = p[i];
	      return(success = 1);
	  } else {			/* Parse error, undo everything */
	      for (i = 0; i < n; i++)
		if (p[i]) free(p[i]);
	      return(x);
	  }
      }

      case XYDTFP:			/* TOLL-FREE-PREFIX */
	return(dialstr(&dialtfp,
		       " Long-distance prefix for toll-free dialing"));

      case XYDCON:			/* CONNECT */
	z = -1;
	if ((y = cmkey(crrtab,ncrr,"","auto",xxstring)) < 0) return(y);
	if (y != CAR_OFF)		/* AUTO or ON? */
	  if ((z = cmkey(qvtab,nqvt,"","verbose",xxstring)) < 0) return(z);
	if ((x = cmcfm()) < 0) return(x);
	if (z > -1)
	  dialcq = z;
	dialcon = y;
	return(success = 1);

      case XYDRSTR:			/* RESTRICT */
	if ((y = cmkey(drstrtab,4,"","none",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	dialrstr = y;
	return(success = 1);

      default:
	printf("?Unexpected SET DIAL parameter\n");
	return(-9);
    }
}

#ifdef CK_TAPI
static int				/* Set DIAL command options */
settapi() {
    int x,y;
    char *s;

    if (!TAPIAvail) {
	printf("\nTAPI is unavailable on this system.\n");
	return(-9);
    }
    if ((y = cmkey(tapitab,ntapitab,"MS TAPI option","line",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYTAPI_LIN:			/* TAPI LINE */
	return setlin(XYTAPI_LIN,1);
	break;
      case XYTAPI_LOC: {		/* TAPI LOCATION */
	  extern char tapiloc[] ;
	  extern int tapilocid ;
	  int i=0, j = 9999, k = -1 ;

	  cktapiBuildLocationTable(&tapiloctab, &ntapiloc);
	  if (!tapiloctab || !ntapiloc) {
	      printf("\nNo TAPI Locations are configured for this system\n");
	      return(-9) ;
	  }
	  /* Find out what the lowest numbered TAPI location is */
	  /* and use it as the default. */
	  for (i = 0; i < ntapiloc; i++) {
	      if (tapiloctab[i].kwval < j) {
		  j = tapiloctab[i].kwval;
		  k = i;
	      }
	  }		
	  if (k >= 0)
	    s = tapiloctab[k].kwd;
	  else
	    s = "";

	  if ((y = cmkey(tapiloctab,ntapiloc,
			 "TAPI location",s,xxstring)) < 0)
	    return(y);
	  y = lookup(tapiloctab,s,ntapiloc,&x);
	  if (y > -1) {
	      strcpy(tapiloc,tapilinetab[x].kwd);
	      tapilocid = y;
	  }
	  if ((x = cmcfm()) < 0) 
	    return(x);
        }
	break;
      case XYTAPI_CFG:			/* TAPI CONFIGURE-LINE */
	if ((x = cmcfm()) < 0) return(x);
	break;
      case XYTAPI_DIAL:			/* TAPI DIALING-PROPERTIES */
	if ((x = cmcfm()) < 0) return(x);
	break;
    }
    return(1);
}
#endif /* CK_TAPI */

#ifndef NOSHOW
int					/* SHOW MODEM */
shomodem() {
    extern MDMINF * modemp[];
    MDMINF * p;
    int x, n;
    char c;
    long zz;

    shmdmlin();
    printf("\n");

    p = (mdmtyp > 0) ? modemp[mdmtyp - 1] : NULL;
    if (p) {
	printf(" %s\n\n", dialname ? dialname : p->name);
	printf(" Modem carrier-watch:    ");
	if (carrier == CAR_OFF) printf("off\n");
	else if (carrier == CAR_ON) printf("on\n");
	else if (carrier == CAR_AUT) printf("auto\n");
	else printf("unknown\n");

	printf(" Modem capabilities:    ");
	zz = dialcapas ? dialcapas : p->capas;
	if (!zz) {
	    printf(" (none)");
	} else {
	    if (zz & CKD_AT) printf(" AT");
	    if (zz & CKD_V25) printf(" ITU");
	    if (zz & CKD_SB) printf(" SB");
	    if (zz & CKD_EC) printf(" EC");
	    if (zz & CKD_DC) printf(" DC");
	    if (zz & CKD_HW) printf(" HWFC");
	    if (zz & CKD_SW) printf(" SWFC");
	    if (zz & CKD_KS) printf(" KS");
	    if (zz & CKD_TB) printf(" TB");
	}
	printf("\n Modem maximum-speed:    ");
	zz = (dialmax > 0L) ? dialmax : p->max_speed;
	if (zz > 0)
	  printf("%ld bps\n", zz);
	else
	  printf("(unknown)\n");
	printf(" Modem error-correction: %s\n", dialec ? "on" : "off");
	printf(" Modem compression:      %s\n", dialdc ? "on" : "off");
	printf(" Modem speed-matching:   %s",   mdmspd ? "on" : "off");
	printf(" (interface speed %s)\n", mdmspd ? "changes" : "is locked");
	printf(" Modem flow-control:     ");
	if (dialfc == FLO_NONE) printf("none\n");
        else if (dialfc == FLO_XONX) printf("xon/xoff\n");
	else if (dialfc == FLO_RTSC) printf("rts/cts\n");
	else if (dialfc == FLO_AUTO) printf("auto\n");
	printf(" Modem kermit-spoof:     %s\n", dialksp ? "on" : "off");
	c = (char) (x = (dialesc ? dialesc : p->esc_char));
	printf(" Modem escape-character: %d", x);
	if (isprint(c))
	  printf(" (= \"%c\")",c);
	printf(
"\n\nMODEM COMMANDs (* = set automatically by SET MODEM TYPE):\n\n");
	printf(" %c Init-string:          ", dialini ? ' ' : '*' );
	shods(dialini ? dialini : p->wake_str);
    printf(" %c Dial-mode-string:     ", dialmstr ? ' ' : '*' );
    shods(dialmstr ? dialmstr : p->dmode_str);
    printf(" %c Dial-mode-prompt:     ", dialmprmt ? ' ' : '*' );
    shods(dialmprmt ? dialmprmt : p->dmode_prompt);
    printf(" %c Dial-command:         ", dialcmd ? ' ' : '*' );
	shods(dialcmd ? dialcmd : p->dial_str);
	printf(" %c Compression on:       ", dialdcon ? ' ' : '*' );
	shods(dialdcon ? dialdcon : p->dc_on_str);
	printf(" %c Compression off:      ", dialdcoff ? ' ' : '*' );
	shods(dialdcoff ? dialdcoff : p->dc_off_str);
	printf(" %c Error-correction on:  ", dialecon ? ' ' : '*' );
	shods(dialecon ? dialecon : p->ec_on_str);
	n = local ? 19 : 20;
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Error-correction off: ", dialecoff ? ' ' : '*' );
	shods(dialecoff ? dialecoff : p->ec_off_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Autoanswer on:        ", dialaaoff ? ' ' : '*' );
	shods(dialaaon ? dialaaon : p->aa_on_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Autoanswer off:       ", dialaaoff ? ' ' : '*' );
	shods(dialaaoff ? dialaaoff : p->aa_off_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Hangup-command:       ", dialhcmd ? ' ' : '*' );
	shods(dialhcmd ? dialhcmd : p->hup_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Hardware-flow:        ", dialhwfc ? ' ' : '*' );
	shods(dialhwfc ? dialhwfc : p->hwfc_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Software-flow:        ", dialswfc ? ' ' : '*' );
	shods(dialswfc ? dialswfc : p->swfc_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c No-flow-control:      ", dialnofc ? ' ' : '*' );
	shods(dialnofc ? dialnofc : p->nofc_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Pulse:                ", dialpulse ? ' ' : '*');
	shods(dialpulse ? dialpulse : p->pulse);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Tone:                 ", dialtone ? ' ' : '*');
	shods(dialtone ? dialtone : p->tone);
	if (++n > cmd_rows - 4) if (!askmore()) return(0); else n = 0;
	printf("\n For more info: SHOW DIAL and SHOW COMMUNICATIONS\n");

    } else if (mdmtyp > 0) {
	printf("Modem info for \"%s\" not filled in yet\n", gmdmtyp());
    } else printf(
" No modem selected, so DIAL and most SET MODEM commands have no effect.\n\
 Use SET MODEM TYPE to select a modem.\n");
    return(1);
}
#endif /* NOSHOW */
#endif /* NODIAL */

#ifndef NOSPL

static int mdays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

_PROTOTYP(static int setalarm,(long));

#ifdef CK_ANSIC				/* SET ALARM */
static int
setalarm(long xx)
#else
static int
setalarm(xx) long xx;
#endif /* CK_ANSIC */
/* setalarm */ {
    int yyyy, mm, dd, x;
    char *s;
    long zz;

    debug(F101,"setalarm xx","",xx);
    ck_alarm = 0L;			/* 0 = no alarm (in case of error) */
    if (xx < 0L) {
	printf("%ld - illegal value, must be 0 or positive\n", xx);
	return(-9);
    }
    x = 8;				/* Get current date */
    s = alrm_date;
    if (zzstring("\\v(ndate)",&s,&x) < 0) {
	printf("Internal date error, sorry.\n");
	alrm_date[0] = SP;
	return(-9);
    }
    x = 5;				/* Get current time */
    s = alrm_time;
    if (zzstring("\\v(ntime)",&s,&x) < 0) {
	printf("Internal time error, sorry.\n");
	alrm_time[0] = SP;
	return(-9);
    }
    debug(F110,"SET ALARM date (1)",alrm_date,0);
    debug(F110,"SET ALARM time (1)",alrm_time,0);

    if ((zz = atol(alrm_time) + xx) < 0L) {
	printf("Internal time conversion error, sorry.\n");
	return(-9);
    }
    if (zz > 86400L) {			/* Alarm crosses midnight */
	char d[10];			/* Local date buffer */
	int lastday;			/* Last day of this month */

	strncpy(d,alrm_date,8);		/* We'll have to change the date */

	x = (zz / 86400L);		/* How many days after today */

	dd = atoi((char *)(d+6));	/* Parse yyyymmdd */
	d[6] = NUL;			/* into yyyy, mm, dd ... */
	mm = atoi((char *)(d+4));
	d[4] = NUL;
	yyyy = atoi((char *)d);

	/* How many days in this month */

	lastday = mdays[mm];
	if (mm == 2 && yyyy % 4 == 0)	/* Works thru 2099 AD... */
	  lastday++;

	if (dd + x > lastday) {		/* Dumb loop */
	    int y;
	    
	    x -= (mdays[mm] - dd);	/* Deduct rest of this month's days */

	    /* There's a more elegant way to do this... */

	    while (1) {
		mm++;			/* Next month */
		if (mm > 12) {		/* Wrap around */
		    mm = 1;		/* Jan, next year */
		    yyyy++;
		}
		y = mdays[mm];		/* Days in new month */
		if (mm == 2 && yyyy % 4 == 0) /* Feb in leap year */
		  y++;			/* Works until 2100 AD */
		if (x - y < 1)
		  break;
		x -= y;
	    }
	    dd = x;			/* Day of alarm month */
	} else dd += x;

	sprintf(alrm_date,"%04d%02d%02d",yyyy,mm,dd);
	zz = zz % 86400L;
    }
    sprintf(alrm_time,"%ld",zz);
    debug(F110,"SET ALARM date (2)",alrm_date,0);
    debug(F110,"SET ALARM time (2)",alrm_time,0);
    ck_alarm = xx;
    return(1);
}
#endif /* NOSPL */

#ifndef NOSETKEY
int
set_key() {				/* SET KEY */
    int x, y;
    int flag = 0;
    int kc;				/* Key code */
    char *s;				/* Key binding */
#ifndef NOKVERBS
    char *p;				/* Worker */
#endif /* NOKVERBS */
#ifdef OS2
    extern int os2gks;
    extern int mskkeys;
    extern int initvik;
#endif /* OS2 */

    x_ifnum = 1;
    y = cmnum("numeric key code, or the word CLEAR,","",10,&kc,xxstring);
    x_ifnum = 0;
    if (y < 0) {
	debug(F111,"SET KEY",atmbuf,y);
	if (y == -2) {			/* Not a valid number */
	    if ((y = strlen(atmbuf)) < 0) /* Check for SET KEY CLEAR */
	      return(-2);
	    if (xxstrcmp(atmbuf,"clear",y))
	      return(-2);
	    if ((x = cmcfm()) < 0)
	      return(x);
	    for (y = 0; y < KMSIZE; y++) {
		keymap[y] = (KEY) y;
		macrotab[y] = NULL;
	    }
#ifdef OS2
	    keymapinit();		/* Special OS/2 initializations */
	    initvik = 1;		/* Update the VIK table */
#endif /* OS2 */
	    return(1);
	} else if (y == -3) {		/* SET KEY <Return> */
	    printf(" Press key to be defined: "); /* Prompt for a keystroke */
#ifdef UNIX
#ifdef NOSETBUF
	    fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
	    conbin((char)escape);	/* Put terminal in binary mode */
#ifdef OS2
	    os2gks = 0;			/* Turn off Kverb preprocessing */
#endif /* OS2 */
	    kc = congks(0);		/* Get character or scan code */
#ifdef OS2
	    os2gks = 1;			/* Turn on Kverb preprocessing */
#endif /* OS2 */
	    concb((char)escape);	/* Restore terminal to cbreak mode */
	    if (kc < 0) {		/* Check for error */
		printf("?Error reading key\n");
		return(0);
	    }
	    shokeycode(kc);		/* Show current definition */
	    flag = 1;			/* Remember it's a multiline command */
	} else				/* Error */
	  return(y);
    }

    /* Normal SET KEY <scancode> <value> command... */

#ifdef OS2
    if ( mskkeys )
      kc = msktock(kc);
#endif /* OS2 */

    if (kc < 0 || kc >= KMSIZE) {
	printf("?key code must be between 0 and %d\n", KMSIZE - 1);
	return(-9);
    }
    if (kc == escape) {
	printf("Sorry, %d is the CONNECT-mode escape character\n",kc);
	return(-9);
    }
#ifdef OS2
    wideresult = -1;
#endif /* OS2 */
    if (flag) {
	cmsavp(psave,PROMPTL);
	cmsetp(" Enter new definition: ");
	cmini(ckxech);
    }
  def_again:
    if (flag) prompt(NULL);
    if ((y = cmtxt("key definition,\n\
or Ctrl-C to cancel this command,\n\
or Enter to restore default definition",
		   "",&s,NULL)) < 0) {
	if (flag)			/* Handle parse errors */
	  goto def_again;
	else
	  return(y);
    }
    s = brstrip(s);
#ifndef NOKVERBS
    p = s;				/* Save this place */
#endif /* NOKVERBS */
/*
  If the definition included any \Kverbs, quote the backslash so the \Kverb
  will still be in the definition when the key is pressed.  We don't do this
  in zzstring(), because \Kverbs are valid only in this context and nowhere
  else.

  We use this code active for all versions that support SET KEY, even if they
  don't support \Kverbs, because otherwise \K would behave differently for
  different versions.
*/
    for (x = 0, y = 0; s[x]; x++, y++) { /* Convert \K to \\K */
	if ((x > 0) &&
	    (s[x] == 'K' || s[x] == 'k')
	    ) {				/* Have K */
  
	    if ((x == 1 && s[x-1] == CMDQ) ||
		(x > 1 && s[x-1] == CMDQ && s[x-2] != CMDQ)) {
		line[y++] = CMDQ;	/* Make it \\K */
	    }
	    if (x > 1 && s[x-1] == '{' && s[x-2] == CMDQ) {
  		line[y-1] = CMDQ;	/* Have \{K */
  		line[y++] = '{';	/* Make it \\{K */
	    }
	}
	line[y] = s[x];
    }
    line[y++] = NUL;			/* Terminate */
    s = line + y + 1;			/* Point to after it */
    x = LINBUFSIZ - (int) strlen(line) - 1; /* Calculate remaining space */
    if ((x < (LINBUFSIZ / 2)) ||
	(zzstring(line, &s, &x) < 0)) { /* Expand variables, etc. */
	printf("?Key definition too long\n");
	if (flag) cmsetp(psave);
	return(-9);
    }
    s = line + y + 1;			/* Point to result. */

#ifndef NOKVERBS
/*
  Special case: see if the definition starts with a \Kverb.
  If it does, point to it with p, otherwise set p to NULL.
*/
    p = s;
    if (*p++ == CMDQ) {
	if (*p == '{') p++;
	p = (*p == 'k' || *p == 'K') ? p + 1 : NULL; 
    }
#endif /* NOKVERBS */

    if (macrotab[kc]) {			/* Possibly free old macro from key. */
	free(macrotab[kc]);
	macrotab[kc] = NULL;
    }
    switch (strlen(s)) {		/* Action depends on length */
      case 0:				/* Reset to default binding */
	keymap[kc] = (KEY) kc;
	break;
      case 1:				/* Single character */
  	keymap[kc] = (CHAR) *s;
  	break;
      default:				/* Character string */
#ifndef NOKVERBS
	if (p) {
	    y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
	    debug(F101,"set key kverb lookup",0,y); /* exact match required */
	    if (y > -1) {
		keymap[kc] = F_KVERB | y;
		break;
	    }
	}
#endif /* NOKVERBS */
	keymap[kc] = (KEY) kc;
	macrotab[kc] = (MACRO) malloc(strlen(s)+1);
	if (macrotab[kc])
	  strcpy((char *) macrotab[kc], s);
	break;
    }
    if (flag) cmsetp(psave);
#ifdef OS2
    initvik = 1;			/* Update VIK table */
#endif /* OS2 */
    return(1);
}
#endif /* NOSETKEY */

/*
  PROTOCOL SELECTION.  Kermit is always available.  If CK_XYZ is defined at
  compile time, then the others become selections also.  In OS/2 and
  Windows, they are integrated and the various SET commands (e.g. "set file
  type") affect them as they would Kermit.  In other OS's (UNIX, VMS, etc),
  they are external protocols which are run via Kermit's REDIRECT mechanism.
  All we do is collect and verify the filenames and pass them along to the
  external protocol.
*/
extern int protocol;
struct keytab protos[] = {
#ifdef CK_XYZ
    "g",        PROTO_G,  CM_INV,
#endif /* CK_XYZ */
    "kermit",   PROTO_K,  0,
#ifdef CK_XYZ
    "other",    PROTO_O,  0,
    "xmodem",   PROTO_X,  0,
    "y",        PROTO_Y,  CM_INV|CM_ABR,
    "ymodem",   PROTO_Y,  0,
    "ymodem-g", PROTO_G,  0,
    "zmodem",   PROTO_Z,  0
#endif /* CK_XYZ */
};
int nprotos =  (sizeof(protos) / sizeof(struct keytab));

#define XPCMDLEN 71

_PROTOTYP(static int protofield, (char *, char *, char *));
_PROTOTYP(static int setproto, (void));

static int
protofield(current, help, px) char * current, * help, *px; {

    char *s, tmpbuf[XPCMDLEN+1];
    int x;

    if (current)			/* Put braces around default */
      sprintf(tmpbuf,"{%s}",current);
    else
      tmpbuf[0] = NUL;

    if ((x = cmfld(help, (char *)tmpbuf, &s, xxstring)) < 0)
      return(x);
    if ((int)strlen(s) > XPCMDLEN) {
	printf("?Sorry - maximum length is %d\n", XPCMDLEN);
	return(-9);
    } else if (*s) {
	strcpy(px,s);
    } else {
	px = NULL;
    }
    return(x);
}

static int
setproto() {				/* Select a file transfer protocol */
    char * s = NULL;
    int x = 0, y;
    char s1[XPCMDLEN+1], s2[XPCMDLEN+1], s3[XPCMDLEN+1];
    char s4[XPCMDLEN+1], s5[XPCMDLEN+1], s6[XPCMDLEN+1];
    char * p1 = s1, * p2 = s2, *p3 = s3;
    char * p4 = s4, * p5 = s5, *p6 = s6;

#ifdef XYZ_INTERNAL
    extern int p_avail;
#else
#ifndef CK_REDIR
    x = 1;
#endif /* CK_REDIR */
#endif /* XYZ_INTERNAL */
    s1[0] = NUL;
    s2[0] = NUL;
    s3[0] = NUL;
    s4[0] = NUL;
    s5[0] = NUL;
    s6[0] = NUL;

    if ((y = cmkey(protos,nprotos,"","kermit",xxstring)) < 0) 
      return(y);

    if (x && y != PROTO_K) {
	printf(
	   "?Sorry, REDIRECT capability required for external protocols.\n");
	return(-9);
    }
    if ((x = protofield(ptab[y].h_b_init,
     "Optional command to send to host prior to uploading in binary mode", 
	       p1)) < 0) {
	if (x == -3) {
	    protocol = y;		/* Set protocol but don't change */
	    return(1);			/* anything else */
	} else
	  return(x);
    }
    if ((x = protofield(ptab[y].h_t_init,
     "Optional command to send to host prior to uploading in text mode",
	       p2)) < 0) {
	if (x == -3)
	  goto protoexit;
	else
	  return(x);
    }

#ifndef XYZ_INTERNAL			/* If XYZMODEM are external... */

    if ((x = protofield(ptab[y].p_b_scmd,
     "External command to SEND in BINARY mode with this protocol",
	       p3)) < 0) {
	if (x == -3)
	  goto protoexit;
	else
	  return(x);
    }
    if ((x = protofield(ptab[y].p_t_scmd,
     "External command to SEND in TEXT mode with this protocol",
	       p4)) < 0) {
	if (x == -3)
	  goto protoexit;
	else
	  return(x);
    }
    if ((x = protofield(ptab[y].p_b_rcmd,
     "External command to RECEIVE in BINARY mode with this protocol",
	       p5)) < 0) {
	if (x == -3)
	  goto protoexit;
	else
	  return(x);
    }
    if ((x = protofield(ptab[y].p_t_rcmd,
     "External command to RECEIVE in TEXT mode with this protocol",
	       p6)) < 0) {
	if (x == -3)
	  goto protoexit;
	else
	  return(x);
    }
#endif /* XYZ_INTERNAL */

    if ((x = cmcfm()) < 0)		/* Confirm the command */
      return(x);

protoexit:				/* Common exit from this routine */

#ifdef XYZ_INTERNAL
    if (!p_avail) {
	bleep(BP_WARN);
	printf("\n?X,Y, and Zmodem are unavailable\n");
	return(success = 0);
    }
#endif /* XYZ_INTERNAL */

    if (p1) p1 = brstrip(p1);
    if (p2) p2 = brstrip(p2);
    if (p3) p3 = brstrip(p3);
    if (p4) p4 = brstrip(p4);
    if (p5) p5 = brstrip(p5);
    if (p6) p6 = brstrip(p6);

    initproto(y,p1,p2,p3,p4,p5,p6);
    return(success = 1);
}

int
setdest() {
    int x, y;
    if ((y = cmkey(desttab,ndests,"","disk",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    dest = y;
    return(1);
}

/*  D O P R M  --  Set a parameter.  */
/*
 Returns:
  -2: illegal input
  -1: reparse needed
   0: success
*/
int
doprm(xx,rmsflg) int xx, rmsflg; {
    int i, x, y = 0, z;
    long zz;
    char *s=NULL, *p=NULL;
 
#ifdef OS2
    if (xx == XYMSK)
      return(setmsk());
#endif /* OS2 */

    if (xx == XYPRTR) {			/* SET PRINTER */
	if ((x = cmofi("printer file",
#ifdef OS2
		       "PRN"
#else
#ifdef VMS
		       "LPT:"
#else
#ifdef UNIX
		       "|lpr"

#else
		       ""
#endif /* UNIX */
#endif /* VMS */
#endif /* OS2 */
		       ,&s,xxstring)) < 0)
	  return(x);
	if (x > 1) {
	    printf("?Directory names not allowed\n");
	    return(-9);
	}
	while (*s == SP || *s == HT) s++; /* Trim leading whitespace */
	strcpy(line,s);			/* Make a temporary safe copy */
	if ((x = cmcfm()) < 0)		/* Confirm the command */
	  return(x);
	s = line;
#ifdef OS2ORUNIX
	if (printpipe = (*s == '|')) {	/* Have pipe? */
	    s++;			/* Point past pipe sign */
	    while (*s == SP | *s == HT) s++; /* Gobble whitespace */
	}	    
#ifdef UNIX
	if (!printpipe) {
	    if (zchko(s) < 0) {
		printf("?Access denied - %s\n",s);
		return(-9);
	    }
	}
#endif /* UNIX */
#endif /* OS2ORUNIX */

	if (printfile) {		/* Had a print file before? */
	    free(printfile);		/* Remove its name */
	    printfile = NULL;
	}
	x = strlen(s);			/* Length of name of new print file */
	if (x > 0) {
#ifdef OS2
	    if ((x != 3) || (xxstrcmp(s,"PRN",3) != 0)) {
#endif /* OS2 */
		printfile = (char *) malloc(x + 1); /* Allocate space for it */
		if (!printfile) {
		    printf("?Memory allocation failure\n");
		    return(-9);
		}
		strcpy(printfile,s);	/* Copy new name to new space */
		debug(F110,"printfile name",printfile,0);
#ifdef OS2
	    } else
#endif /* OS2 */
	      debug(F101,"printfile is NULL","",printfile);
	}
	/* Return with printfile pointing to a file or device name */
	/* or NULL to indicate the default printer. */
	return(success = 1);
    }

switch (xx) {
 
#ifdef ANYX25				/* SET X25 ... */
case XYX25:
    return(setx25());

case XYPAD:				/* SET PAD ... */
    return(setpadp());
#endif /* ANYX25 */

case XYEOL:	/* These have all been moved to set send/receive... */
case XYLEN: 	/* Let the user know what to do. */
case XYMARK:
case XYNPAD:
case XYPADC:
case XYTIMO:
    printf("...Use SET SEND or SET RECEIVE instead.\n");
    printf("Type HELP SET SEND or HELP SET RECEIVE for more info.\n");
    return(success = 0);

case XYATTR:				/* File Attribute packets */
    return(setat(rmsflg));

case XYIFD:				/* Incomplete file disposition */
    if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    if (rmsflg) {
	sstate = setgen('S', "310", y ? "1" : "0", "");
	return((int) sstate);
    } else {
	keep = y;
	return(success = 1);
    }
 
#ifndef NOSPL
case XYINPU:				/* SET INPUT */
    return(setinp());
#endif /* NOSPL */

#ifdef NETCONN
case XYNET:				/* SET NETWORK */

#ifdef OS2     /* Hide network-type keywords for networks not installed */
    for (z = 0; z < nnets; z++) {
	if (netcmd[z].kwval == NET_TCPB && tcp_avail == 0)
	  netcmd[z].flgs =  CM_INV;
#ifdef DECNET
	else if (netcmd[z].kwval == NET_DEC  && dnet_avail == 0)
	  netcmd[z].flgs =  CM_INV;
#endif /* DECNET */
#ifdef CK_NETBIOS
	else if (netcmd[z].kwval == NET_BIOS && netbiosAvail == 0)
	  netcmd[z].flgs =  CM_INV;
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
	else if (netcmd[z].kwval == NET_SLAT  && slat_avail == 0)
	  netcmd[z].flgs =  CM_INV;
#endif /* SUPERLAT */
    }
    if (tcp_avail)			/* Default network type */
      strcpy(tmpbuf,"tcp/ip");
#ifdef DECNET
    else if (dnet_avail)
      strcpy(tmpbuf,"decnet");
#endif /* DECNET */
#ifdef SUPERLAT
    else if (slat_avail)
      strcpy(tmpbuf,"superlat");
#endif /* SUPERLAT */
#ifdef CK_NETBIOS
    else if (netbiosAvail)
      strcpy(tmpbuf,"netbios");
#endif /* CK_NETBIOS */
    else strcpy(tmpbuf,"named-pipe");
#else
#ifdef TCPSOCKET
    strcpy(tmpbuf,"tcp/ip");
#else
#ifdef ANYX25
    strcpy(tmpbuf,"x.25");
#else
    strcpy(tmpbuf,"");    
#endif /* ANYX25 */
#endif /* TCPSOCKET */
#endif /* OS2 */

    if ((z = cmkey(netcmd,nnets,"",tmpbuf,xxstring)) < 0)
      return(z);

#ifndef NODIAL
    if (z == XYNET_D)			/* DIRECTORY */
      return(parsdir(1));
#endif /* NODIAL */

    if (z == XYNET_T) {			/* TYPE */
	if ((z = cmkey(netkey,nnetkey,"",tmpbuf,xxstring)) < 0)
	  return(z);
    }

#ifdef OS2
    if (z == NET_TCPB && tcp_avail == 0) {
        printf("\n?Sorry, either TCP/IP is not available on this system or\n\
necessary DLLs did not load.  Use SHOW NETWORK to check network status.\n");
	return(-9);
#ifdef CK_NETBIOS
    } else if (z == NET_BIOS && netbiosAvail == 0) {
	printf("\n?Sorry, NETBIOS is not available on this system.\n") ;
	return(-9);
#endif /* CK_NETBIOS */
#ifdef DECNET
    } else if (z == NET_DEC && dnet_avail == 0) {
	printf("\n?Sorry, DECnet is not available on this system.\n") ;
	return(-9);
#endif /* DECNET */
#ifdef SUPERLAT
    } else if (z == NET_SLAT && slat_avail == 0) {
	printf("\n?Sorry, SuperLAT is not available on this system.\n") ;
	return(-9);
#endif /* SUPERLAT */
    }
#endif /* OS2 */

#ifdef NPIPEORBIOS
    if (z == NET_PIPE || 		/* Named pipe -- also get pipename */
	z == NET_BIOS) {		/* NETBIOS -- also get local name */
	char *defnam;
#ifdef CK_NETBIOS
	char tmpnbnam[NETBIOS_NAME_LEN+1];
#endif /* CK_NETBIOS */
	/* Construct default name  */
	if (z == NET_PIPE) {		/* Named pipe */
	    defnam = "kermit";		/* Default name is always "kermit" */
	} else {			/* NetBIOS */
	    if (NetBiosName[0] != SP) {	/* If there is already a name, */
		char *p = NULL; 
                int n;		/* use it as the default. */
		strcpy(tmpnbnam,NetBiosName);
		p = tmpnbnam + NETBIOS_NAME_LEN - 1;
		while (*p == SP) {
		    *p = NUL;
		    p--;
		}
		defnam = tmpnbnam;
	    } else if (*myhost)		/* Otherwise use this PC's host name */
	      defnam = (char *) myhost;
	    else			/* Otherwise use "kermit" */
	      defnam = "kermit";
	}
	if ((y = cmtxt((z == NET_PIPE) ? "pipe name" : "local NETBIOS name",
		       defnam, &s, xxstring)) < 0)
	  return(y);
#ifdef NPIPE
	pipename[0] = NUL;
#endif /* NPIPE */
	if ((y = (int) strlen(s)) < 1) {
	    printf("?You must also specify a %s name\n",
		   (z == NET_PIPE) ? "pipe" : "local NETBIOS" );
 	    return(-9);
	}
#ifdef CK_NETBIOS
	if (z == NET_BIOS) {
	    if ( !netbiosAvail ) {
		printf("?NETBIOS support is not available on this system.\n") ;
		return(-9) ;
	    }
	    if ( y - NETBIOS_NAME_LEN > 0) {
		printf("?NETBIOS name too long, %ld maximum\n",
		       NETBIOS_NAME_LEN);
		return(-9);
	    } else if ( !strcmp(s,tmpnbnam) ) {
      		nettype = z;		/* Returning to old connection... */
		return(success = 1);	/* Done */
	    } else if (strcmp("                ",NetBiosName)) {
		   printf("?Local NETBIOS name already assigned to \"%s\"\n",
		       NetBiosName);
		   return(-9) ;
	   } else {
		NCB ncb ;
		APIRET rc ;
		strcpy(NetBiosName,s);
		for (x = y; x < NETBIOS_NAME_LEN; x++)
		  NetBiosName[x] = SP;
		NetBiosName[NETBIOS_NAME_LEN] = NUL;
		printf("Verifying \"%s\" is a unique NetBIOS node name ...\n",
		       NetBiosName) ;
		rc = NCBAddName( NetbeuiAPI,
				&ncb, NetBiosAdapter, NetBiosName ) ;
		if ( rc ) {
		    printf(
		"?Sorry, \"%s\" is already in use by another NetBIOS node.\n",
			   NetBiosName);
		    for ( x=0; x < NETBIOS_NAME_LEN; x++)
		      NetBiosName[x] = SP ;
		    return(-9) ;
		} 
	    }
	}
#endif /* CK_NETBIOS */
#ifdef NET_PIPE
	if (z == NET_PIPE)
	  strncpy(pipename,s,PIPENAML);
#endif /* NET_PIPE */
    } else
#endif /* NPIPEORBIOS */
      if ((x = cmcfm()) < 0) return(x);
    nettype = z;
    if (
#ifdef DECNET
	(nettype != NET_DEC)  &&
#endif /* DECNET */
#ifdef NPIPE
	(nettype != NET_PIPE) &&
#endif /* NPIPE */
#ifdef CK_NETBIOS
	(nettype != NET_BIOS) &&
#endif /* CK_NETBIOS */
#ifdef NETFILE
	(nettype != NET_FILE) &&
#endif /* NETFILE */
#ifdef SUPERLAT
	(nettype != NET_SLAT) &&
#endif /* SUPERLAT */
	(nettype != NET_SX25) &&
	(nettype != NET_VX25) &&
        (nettype != NET_TCPB)) {
	printf("?Network type not supported\n");
	return(success = 0);
    } else {
	return(success = 1);
    }

#ifndef NOTCPOPTS
#ifdef TCPSOCKET
#ifdef SOL_SOCKET
case XYTCP:
if ((z = cmkey(tcpopt,ntcpopt,"TCP option","nodelay",xxstring)) < 0)
   return(z);
   
   switch (z) {
#ifdef SO_KEEPALIVE 
   case XYTCP_KEEPALIVE:
      if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = keepalive(z) ;
      return(success);
#endif /* SO_KEEPALIVE */
#ifdef TCP_NODELAY
   case XYTCP_NODELAY:
      if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = no_delay(z) ;
      return(success);
#endif /* TCP_NODELAY */
#ifdef SO_LINGER 
    case XYTCP_LINGER:
      if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) 
	return(z);
      if (z) {				/* if on, we need a timeout value */
	  if ((x = cmnum("Linger timeout in 10th of a millisecond",
			 "0",10,&y,xxstring)) < 0)
	    return(x);
      } else 
	y = 0;
      if ((x = cmcfm()) < 0) 
	return(x);
      success = ck_linger(z,y);
      return(success);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF 
   case XYTCP_SENDBUF:
      x = cmnum("Send buffer size, bytes","8192",10,&z,xxstring);
      if (x < 0) return(x);
      if ((x = cmcfm()) < 0) return(x);    
      success = sendbuf(z);
      return(success);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF 
   case XYTCP_RECVBUF:   
      x = cmnum("Receive buffer size, bytes","8192",10,&z,xxstring);
      if (x < 0) return(x);
      if ((x = cmcfm()) < 0) return(x);    
      success = recvbuf(z);
      return(success);
#endif /* SO_RCVBUF */
   default:
      return(0);
   }
#endif /* SOL_SOCKET */
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */
#endif /* NETCONN */
}

switch (xx) {

#ifndef NOLOCAL
case XYHOST:				/* SET HOST or SET LINE */
case XYLINE:
    return(setlin(xx,1));
#endif /* NOLOCAL */
 
#ifndef NOSETKEY
case XYKEY:				/* SET KEY */
    return(set_key());
#endif /* NOSETKEY */

#ifndef NOCSETS
case XYLANG: 				/* Language */
    if ((y = cmkey(lngtab,nlng,"","none",xxstring)) < 0) /* language code */
      return(y);
    if ((x = cmcfm()) < 0) return(x);	/* And confirmation of command */

    /* Look up language and get associated character sets */
    for (i = 0; (i < nlangs) && (langs[i].id != y); i++) ;
    if (i >= nlangs) {
	printf("?internal error, sorry\n");
	return(success = 0);
    }
    language = i;			/* All good, set the language, */
    return(success = 1);
#endif /* NOCSETS */

#ifndef MAC
case XYBACK:				/* BACKGROUND */
    if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
    if ((y = cmcfm()) < 0) return(y);
    bgset = z;
    success = 1;
    bgchk();
    return(success);
#endif /* MAC */

case XYQUIE:				/* QUIET */
    return(success = seton(&quiet));

case XYBUF: {				/* BUFFERS */
#ifdef DYNAMIC
    int sb, rb;
    char sbs[10];
    if ((y = cmnum("Send buffer size","",10,&sb,xxstring)) < 0) {
	if (y == -3) printf("?Buffer size required\n");
	return(y);
    }
    if (sb < 0) {
	if (*atmbuf == '-') printf("?Negative numbers can't be used here\n");
	else printf("?Integer overflow, use a smaller number please\n");
	return(-9);
    } else if (sb < 80) {
	printf("?Too small\n");
	return(-9);
    }
    sprintf(sbs,"%d",sb);		/* Default second size == first */
    if ((y = cmnum("Receive buffer size",sbs,10,&rb,xxstring)) < 0)
      return(y);
    if (rb < 0) {
	if (*atmbuf == '-') printf("?Negative numbers can't be used here\n");
	else printf("?Integer overflow, use a smaller number please\n");
	return(-9);
    } else if (rb < 80) {
	printf("?Too small\n");
	return(-9);
    }
    if ((y = cmcfm()) < 0) return(y);
    if ((y = inibufs(sb,rb)) < 0) return(y);
    y = adjpkl(urpsiz,wslotr,bigrbsiz); /* Maybe adjust packet sizes */
    if (y != urpsiz) urpsiz = y;
    y = adjpkl(spsiz,wslotr,bigsbsiz);
    if (y != spsiz) spsiz = spmax = spsizr = y;
    return(success = 1);
#else
    printf("?Sorry, not available\n");
    return(success = 0);
#endif /* DYNAMIC */
}

case XYCHKT:				/* BLOCK-CHECK */
    if ((x = cmkey(chktab,4,"","3",xxstring)) < 0) return(x);
    if ((y = cmcfm()) < 0) return(y);
    bctr = x;			     /* Set locally too, even if REMOTE SET */
    if (rmsflg) {
	if (x == 4) {
	    tmpbuf[0] = 'B';
	    tmpbuf[1] = '\0';
	} else sprintf(tmpbuf,"%d",x);
	sstate = setgen('S', "400", tmpbuf, "");
	return((int) sstate);
    } else {
	return(success = 1);
    }
 
#ifndef NOLOCAL
#ifndef MAC
/*
  The Mac has no carrier...
*/
case XYCARR:				/* CARRIER */
    return(success = setdcd());
#endif /* MAC */
#endif /* NOLOCAL */
}

#ifdef TNCODE
switch (xx) {				/* Avoid long switch statements... */
case XYTEL:				/* TELNET */
    if ((z = cmkey(tntab,ntn,"parameter for TELNET negotiations", "",
		   xxstring)) < 0)
      return(z);
    switch (z) {
      case CK_TN_EC:			/* ECHO */
	if ((x = cmkey(rltab,nrlt,
		       "initial TELNET echoing state","local",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	tn_duplex = x;
	return(success = 1);

      case CK_TN_TT:			/* TERMINAL TYPE */
	if ((y = cmtxt("terminal type for TELNET connections","",
		       &s,xxstring)) < 0)
	  return(y);
	if (tn_term) {
	    free(tn_term);		/* Free any previous storage */
	    tn_term = NULL;
	}
	if (s == NULL || *s == NUL) {	/* If none given */
	    tn_term = NULL;		/* remove the override string */
	    return(success = 1);
	} else if (tn_term = malloc(strlen(s)+1)) { /* Make storage for new */
	    strcpy(tn_term,s);		/* Copy string into new storage */
	    return(success = 1);
	} else return(success = 0);

      case CK_TN_NL:			/* TELNET NEWLINE-MODE */
        if ((x = cmkey(tn_nlmtab,ntn_nlm,"","nvt",xxstring)) < 0)
	  return(x);
	if (x == TN_NL_BIN) {
	    if ((x = cmkey(tnlmtab,ntnlm,"","raw",xxstring)) < 0)
	      return(x);
	    if ((y = cmcfm()) < 0)
	      return(y);
	    tn_b_nlm = x;
	    return(success = 1);
	} else if (x == TN_NL_NVT) {
	    if ((x = cmkey(tnlmtab,ntnlm,"","on",xxstring)) < 0)
	      return(x);
	    if ((y = cmcfm()) < 0)
	      return(y);
	    tn_nlm = x;
	    return(success = 1);
	} else {
	    if ((y = cmcfm()) < 0)
	      return(y);
	    tn_nlm = x;
	    return(success = 1);
	}

      case CK_TN_BM:			/* BINARY-MODE */
        if ((x = cmkey(tnbmtab,3,"","refused",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	tn_binary = x;
	return(success = 1);

      case CK_TN_BUG:			/* BUG */
	if ((x = cmkey(tnbugtab,2,"","binary-me-means-u-too",xxstring)) < 0)
	  return(x);
	if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
	if ((y = cmcfm()) < 0) return(y);
	switch (x) {
	  case 0:
	    tn_b_meu = z;
	    break;
	  case 1:
	    tn_b_ume = z;
	    break;
	}
	return(success = 1);

#ifdef CK_ENVIRONMENT
    case CK_TN_ENV: {
	char * msg = "value of telnet environment variable";
	extern char tn_env_acct[], tn_env_disp[], tn_env_job[], 
	            tn_env_prnt[], tn_env_sys[];  
	if ((x = cmkey(tnenvtab,ntnenv,"","",xxstring)) < 0)
	  return(x);
	if (x == TN_ENV_UVAR) {
	    /* Get the user variable name */
	}	

	/* Get the value */
	if ((y = cmtxt(msg, x == TN_ENV_USR ? uidbuf : "", &s, xxstring)) < 0)
	  return(y);
	if ((y = cmcfm()) < 0) return(y);
	switch (x) {
	  case TN_ENV_USR:
	    strncpy(uidbuf,s,63);
	    break;
	  case TN_ENV_ACCT:
	    strncpy(tn_env_acct,s,63);
	    break;
	  case TN_ENV_DISP:
	    strncpy(tn_env_disp,s,63);
	    break;
	  case TN_ENV_JOB:
	    strncpy(tn_env_job,s,63);
	    break;
	  case TN_ENV_PRNT:
	    strncpy(tn_env_prnt,s,63);
	    break;
	  case TN_ENV_SYS:
	    strncpy(tn_env_sys,s,63);
	    break;
	  case TN_ENV_UVAR:
	    printf("\n?Not yet implemented\n");
	    break;
	}
	return(success = 1);
	break;
    }
#endif /* CK_ENVIRONMENT */

      default:
	return(-2);
    }
}
#endif /* TNCODE */

switch (xx) {
#ifndef NOSPL
case XYCOUN:				/* SET COUNT */
    x = cmnum("Positive number","0",10,&z,xxstring);
    if (x < 0) return(x);
    if ((x = cmcfm()) < 0) return(x);
    if (z < 0) {
	printf("?A positive number, please\n");
	return(0);
    }
    debug(F101,"XYCOUN: z","",z);
    return(success = setnum(&count[cmdlvl],z,0,10000));
#endif /* NOSPL */

#ifndef NOSPL
case XYCASE:
    return(success = seton(&inpcas[cmdlvl]));
#endif /* NOSPL */

case XYCMD:				/* COMMAND ... */
    if ((y = cmkey(scmdtab,nbytt,"","",xxstring)) < 0) return(y);
    switch(y) {
      case SCMD_BSZ:
	if ((y = cmnum("bytesize for command characters, 7 or 8","7",10,&x,
		       xxstring)) < 0)
	  return(y);
	if (x != 7 && x != 8) {
	    printf("\n?The choices are 7 and 8\n");
	    return(success = 0);
	}
	if ((y = cmcfm()) < 0) return(y);
	if (x == 7) cmdmsk = 0177;
	else if (x == 8) cmdmsk = 0377;
	return(success = 1);
#ifdef CK_RECALL
      case SCMD_RCL:
	if ((y = cmnum("maximum number of commands in recall buffer","10",
		       10,&x,xxstring)) < 0)
	  return(y);
	if ((y = cmcfm()) < 0) return(y);
	return(success = cmrini(x));
#endif /* CK_RECALL */
#ifdef CK_RECALL
      case SCMD_RTR:
	return(success = seton(&cm_retry));
#endif /* CK_RECALL */
      case SCMD_MOR:			/* More-prompting */
	return(success = seton(&xaskmore));
      case SCMD_QUO:
	if ((x = seton(&y)) < 0) return(x);
	cmdsquo(y);			/* Do it the right way */
	cmd_quoting = y;		/* Also keep a global copy */
	/* Set string-processing function */
#ifdef datageneral
	xxstring = y ? zzstring : (xx_strp) NULL;
#else
#ifdef CK_ANSIC
	xxstring = y ? zzstring : (xx_strp) NULL;
#else
	xxstring = y ? zzstring : NULL;
#endif /* CK_ANSIC */
#endif /* datageneral */
	return(success = 1);

#ifdef OS2
      case SCMD_COL: {			/* Command-screen colors */
         int fg, bg;
         fg = cmkey(ttyclrtab, nclrs,
                     "foreground color and then background color",
                     "white",
                     xxstring);
         if (fg < 0)   
            return(fg);
         if ((bg = cmkey(ttyclrtab,nclrs,
                          "background color","black",xxstring)) < 0)
            return(bg);
         if ((y = cmcfm()) < 0)  
            return(y);
         colorcmd = fg | bg << 4;   
         return(success = 1);
      }

    case SCMD_SCR:       /* Command Scrollback size */
          if ((y = cmnum("COMMAND scrollback buffer size, lines","512",10,&x,
                          xxstring)) < 0)
            return(y);
#ifdef __32BIT__
	/* The max number of lines is the RAM  */
	/* we can actually dedicate to a       */
	/* scrollback buffer given the maximum */
	/* process memory space of 512MB       */
	if (x < 256 || x > 2000000L) {
	    printf("\n?The size must be between 256 and 2,000,000.\n"); 
	    return(success = 0);
 	} 
#else
	if (x < 64 || x > 240) {
	    printf("\n?The size must be between 64 and 240.\n");
	    return(success = 0);
	}
#endif /* __32BIT__ */
	if ((y = cmcfm()) < 0) return(y);
#ifndef VSCRNINIT
	if ( (ULONG) x < VscrnGetBufferSize(VCMD) ) {
	    printf("\nWarning: the scrollback buffer will be emptied on the");
	    printf(" next CONNECT,\n");
	    printf("unless the buffer is restored to %d lines.\n",
		   VscrnGetBufferSize(VCMD) ) ;
        }
#endif /* VSCRNINIT */
          tt_scrsize[VCMD] = x;
#ifdef VSCRNINIT
          VscrnInit( VCMD ) ;
#endif /* VSCRNINIT */
	return(success = 1);

          break;

      case SCMD_WID: {
	  if ((y = cmnum("number of columns in display window during CONNECT",
			 "80",10,&x,xxstring)) < 0)
	    return(y);
	  if ((y = cmcfm()) < 0) return(y);

	  if (IsOS2FullScreen()) {
	      if (x != 40 && x != 80 && x != 132) {
		  printf("\n?The width must be 40, 80,");
#ifdef NT
		  printf(" or 132 under Windows 95.\n.");
#else /* NT */
		  printf(" or 132 in a Full Screen session.\n.");
#endif /* NT */
		  return(success = 0);
	      }
	  } else {
	      if (!IsWARPed() && x != 80) {
		  printf("\n?OS/2 version is pre-WARP: the width must equal ");
		  printf("80 in a Windowed Session\n.");
		  return(success = 0);
	      }
	      if (x < 20 || x > MAXTERMCOL) {
		  printf(
		      "\n?The width must be between 20 and %d\n.",MAXTERMCOL);
		  return(success = 0);
	      }
	  }
	  if (x > 8192/(tt_rows[VCMD]+1)) {
	      printf(
"\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
		     tt_rows[VCMD]+1,x,x*(tt_rows[VCMD]+1));
	      return(success = 0);
	  }
	  tt_cols[VCMD] = x;
#ifdef VSCRNINIT
	  VscrnSetWidth(VCMD, x); 
#else /* VSCRNINIT */
	  VscrnSetWidth(VCMD, -1); 
#endif /* VSCRNINIT */
	  cmd_cols = x;
	  SetCols(VCMD);
	  return(success = 1);
      }
      case SCMD_HIG: 
   	if ((y = cmnum(
"number of rows in display window during CONNECT, not incl status line",
		       "24",10,&x,xxstring)) < 0)
	  return(y);
          if ((y = cmcfm()) < 0) return(y);

        if ( IsOS2FullScreen() ) {
            if ( x != 24 && x != 42 && x != 49 && x != 59 ) {
                printf("\n?The height must be 24, 42, 49");
#ifdef NT
                printf(" or 59 under Windows 95.\n.");
#else /* NT */
                printf(" or 59 in a Full Screen session.\n.");
#endif /* NT */
                return(success = 0);
	    }
	} else {
            if (x < 8 || x > MAXTERMROW ) {
                printf("\n?The height must be between 8 and %d\n.",MAXTERMROW);
                return(success = 0);
	    }
	}
        if (x > 8192/tt_cols[VCMD]) {
            printf(
"\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
		   x,tt_cols[VCMD],x*tt_cols[VCMD]);
            return(success = 0);
	}
#ifdef VSCRNINIT 
     
        tt_szchng[VCMD] = 1;
        tt_rows[VCMD] = x;
        VscrnInit(VCMD);
#else /* VSCRNINIT */
        tt_rows[VCMD] = x;
        VscrnSetHeight(VCMD, -1);
        tt_szchng[VCMD] = 1;
#endif /* VSCRNINIT */
	SetCols(VCMD);
	cmd_rows = x;
        return(success = 1);

    case SCMD_CUR: {
        int row, col;
        position * ppos;

        ppos = VscrnGetCurPos(VCMD);
#ifdef NT
#define itoa _itoa
#endif
        itoa(ppos->y+1, tmpbuf, 10);
        if ((y = cmnum("row (1-based)",tmpbuf,10,&row,xxstring)) < 0)
	  return(y);

        itoa(ppos->x+1, tmpbuf, 10);
        if ((y = cmnum("col (1-based)",tmpbuf,10,&col,xxstring)) < 0)
	  return(y);
        if ((x = cmcfm()) < 0) return(x);

        VscrnSetCurPos( VCMD, col-1, row-1 ) ;
        VscrnIsDirty( VCMD );
        return(success=1);
    }
#else
      case SCMD_WID:
	y = cmnum("Command screen width, characters","80",10,&x,xxstring);
	return(setnum(&cmd_cols,x,y,1024));

      case SCMD_HIG:
	y = cmnum("Command screen height, rows","24",10,&x,xxstring);
	return(setnum(&cmd_rows,x,y,1024));
#endif /* OS2 */

      default: 
	return(-2);
    }
}

switch (xx) {
    
case XYDFLT:				/* SET DEFAULT = CD */
    return(success = docd());

case XYDEBU:				/* SET DEBUG { on, off, session } */
    if ((y = cmkey(dbgtab,ndbg,"","",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    switch (y) {
      case 0:				/* 0 = all debugging off. */
	debses = 0;
#ifdef DEBUG
	if (deblog) doclslog(LOGD);
#endif /* DEBUG */
        return(success = 1);

      case 1:				/* 1 = log debugging to debug.log */
#ifdef DEBUG
	deblog = debopn("debug.log", 0);
	return(success = deblog ? 1 : 0);
#else
	printf("?Sorry, debug log feature not enabled\n");
	return(success = 0);
#endif /* DEBUG */

      case 2:				/* 2 = session. */
	return(success = debses = 1);
    }

case XYDELA:				/* SET DELAY */
    y = cmnum("Number of seconds before starting to send","5",10,&x,xxstring);
    if (x < 0) x = 0;
    return(success = setnum(&delay,x,y,999));

default:
    break;
}

switch (xx) {
#ifndef NODIAL
#ifdef CK_TAPI
case XYTAPI:
    return(settapi());
#endif /* CK_TAPI */
case XYDIAL:				/* SET MODEM or SET DIAL */
    return(setdial(-1));
case XYMODM:
    return(setmodem());
#endif /* NODIAL */

#ifdef COMMENT				/* Unused at present */
case XYDOUB:
    if ((x = cmfld("Character to double","none",&s,xxstring)) < 0) {
	if (x == -3) {
	    dblchar = -1;
	    if (msgflg) printf("Doubling Off\n");
	    return(success = 1);
	} else return(x);
    }
    strcpy(line,s);
    lp = line;
    if ((x = cmcfm()) < 0) return(x);
    if (!xxstrcmp(lp,"none",4)) {
	dblchar = -1;
	if (msgflg) printf("Doubling Off\n");
	return(success = 1);
    }
    if ((int)strlen(lp) != 1) return(-2);
    dblchar = *lp & 0xFF;
    if (msgflg) printf("Doubled: %d\n",dblchar);
    return(success = 1);
#endif /* COMMENT */

#ifndef NOLOCAL
case XYDUPL:				/* SET DUPLEX */
    if ((y = cmkey(dpxtab,2,"","full",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    duplex = y;
    return(success = 1);
 
case XYLCLE:				/* LOCAL-ECHO (= DUPLEX) */
    return(success = seton(&duplex));

case XYESC:				/* SET ESCAPE */
    sprintf(tmpbuf,"%d",DFESC);
    return(success = setcc(tmpbuf,&escape));
#endif /* NOLOCAL */

case XYEXIT:				/* SET EXIT */
    if ((z = cmkey(xittab,nexit,"","",xxstring)) < 0)
      return(z);
    switch (z) {
      case 0:				/* STATUS */
	y = cmnum("EXIT status code","",10,&x,xxstring);
	return(success = setnum(&xitsta,x,y,-1));
      case 1:				/* WARNING */
	if ((z = cmkey(xitwtab,nexitw,"","",xxstring)) < 0) return(z);
	if ((y = cmcfm()) < 0) return(y);
	xitwarn = z;
	return(success = 1);
      case 2:
	success = seton(&exitonclose);
#ifdef TCPSOCKET
	if (success) tn_exit = exitonclose;
#endif /* TCPSOCKET */
	return(success);
      default:
	return(-2);
    } /* End of SET EXIT switch() */

default:
    break;
}

switch (xx) {
case XYFILE:				/* SET FILE */
    return(setfil(rmsflg));

case XYFLOW:				/* FLOW-CONTROL */
/*
  Note: flotab[] keyword table (defined above) only includes the legal 
  flow-control options for each implementation, controlled by symbols
  defined in ckcdeb.h.
*/
    if ((y = cmkey(flotab,nflo,"","xon/xoff",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    flow = y;
#ifdef CK_SPEED
    if (flow == FLO_XONX)		/* Xon/Xoff forces prefixing */
      ctlp[XON] = ctlp[XOFF] = ctlp[XON+128] = ctlp[XOFF+128] = 1;
#endif /* CK_SPEED */
    autoflow = (flow == FLO_AUTO);
    debug(F101,"set flow","",flow);
    return(success = 1);
 
case XYHAND:				/* HANDSHAKE */
    if ((y = cmkey(hshtab,nhsh,"","none",xxstring)) < 0) return(y);
    if (y == 998) {
	if ((x = cmnum("ASCII value","",10,&y,xxstring)) < 0)
	  return(x);
	if ((y < 1) || ((y > 31) && (y != 127))) {
	    printf("?Character must be in ASCII control range\n");
	    return(-9);
	}
    }
    if ((x = cmcfm()) < 0) return(x);
    turn = (y > 0127) ? 0 : 1 ;
    turnch = y;
    return(success = 1);
 
#ifndef NOSPL
case XYMACR:				/* SET MACRO */
    if ((y = cmkey(smactab,2,"","",xxstring)) < 0) return(y);
    switch (y) {
      case 0: return(success = seton(&mecho));
      case 1: return(success = seton(&merror[cmdlvl]));
      default: return(-2);
    }
#endif /* NOSPL */

  case XYMSGS:
#ifdef VMS
    if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
    if ((y = cmcfm()) < 0) return(y);
    vms_msgs = z;
    printf("Sorry, SET MESSAGES not implemented yet\n");
    return(success = 0);
#endif /* VMS */
default:
    break;
}

switch (xx) {
	
case XYPARI:				/* PARITY */
    if ((y = cmkey(partbl,npar,"","none",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
 
/* If parity not none, then we also want 8th-bit prefixing */
 
    if (parity = y) ebqflg = 1; else ebqflg = 0;
    return(success = 1);
 
#ifndef NOFRILLS
case XYPROM:				/* SET PROMPT */
/*
  Note: xxstring not invoked here.  Instead, it is invoked every time the
  prompt is issued.  This allows the prompt string to contain variables
  that can change, like \v(dir), \v(time), etc.
*/
    if ((x = cmtxt("Program's command prompt",ckprompt,&s,NULL)) < 0)
      return(x);
    s = brstrip(s);			/* Remove enclosing braces, if any */
#ifdef COMMENT
/*
  Let's not do this any more -- we don't do it anywhere else.
*/
    else if (*s == '"') {		/* For compatibility with pre-5A */
	x = (int)strlen(s);
	if (s[x-1] == '"') {
	    s[x-1] = NUL;
	    s++;
	}
    }
#endif /* COMMENT */
    cmsetp(s);				/* Set the prompt */
    return(success = 1);
#endif /* NOFRILLS */
 
case XYRETR:				/* RETRY: per-packet retry limit */
    y = cmnum("Maximum retries per packet","10",10,&x,xxstring);
    if (x < 0) x = 0;
    if ((x = setnum(&maxtry,x,y,999)) < 0) return(x);
    if (maxtry <= wslotr) {
	printf("?Retry limit must be greater than window size\n");
	return(success = 0);
    }
    sprintf(tmpbuf,"%d",maxtry);
    if (rmsflg) {
	sstate = setgen('S', "403", tmpbuf, "");
	return((int) sstate);
    } else return(success = x);
 
#ifndef NOSERVER
case XYSERV:				/* SET SERVER items */
    if ((y = cmkey(srvtab,nsrvt,"","",xxstring)) < 0) return(y);
    switch (y) {
#ifdef OS2
      case XYSERI:
	if ((y = cmnum("interval for server idle timeout, 0 = none",
                       "0",10,&x,xxstring)) < 0)
	  return(y);
	if (x < 0) {
	    printf("\n?Specify a positive number, or 0 for no server NAKs\n");
          return(0);
	}
	if ((y = cmcfm()) < 0) return(y);
	srvidl = x;	        /* Set the server idle timeout variable */
	return(success = 1);
#endif /* OS2 */
      case XYSERT:
	tp = tmpbuf;
	sprintf(tp,"%d",DSRVTIM);
	if ((y = cmnum("interval for server NAKs, 0 = none",tp,10,&x,
		       xxstring)) < 0)
	  return(y);
	if (x < 0) {
	    printf("\n?Specify a positive number, or 0 for no server NAKs\n");
	    return(0);
	}
	if ((y = cmcfm()) < 0) return(y);
	sprintf(tp,"%d",x);
	if (rmsflg) {
	    sstate = setgen('S', "404", tp, "");
	    return((int) sstate);
	} else {
	    srvtim = x;			/* Set the server timeout variable */
	    return(success = 1);
	}
      case XYSERD:			/* SERVER DISPLAY */
	return(success = seton(&srvdis)); /* ON or OFF... */

      case XYSERP:			/* SERVER GET-PATH */
	return(parsdir(2));

      case XYSERL:			/* SERVER LOGIN */
	return(cklogin());

      default:
	return(-2);
    }
#endif /* NOSERVER */
}

switch (xx) {
#ifdef UNIX
#ifndef NOJC
case XYSUSP:				/* SET SUSPEND */
    seton(&suspend);			/* on or off... */
    return(success = 1);
#endif /* NOJC */
#endif /* UNIX */

case XYTAKE:				/* SET TAKE */
    if ((y = cmkey(taktab,4,"","",xxstring)) < 0) return(y);
    switch (y) {
      case 0: return(success = seton(&techo));
#ifndef NOSPL
      case 1: return(success = seton(&takerr[cmdlvl]));
#else
      case 1: return(success = seton(&takerr[tlevel]));
#endif /* NOSPL */
      case 2: techo = 0; return(success = 1); /* For compatibility with */
      case 3: techo = 1; return(success = 1); /* MS-DOS Kermit */
      default: return(-2);
    }

#ifndef NOSCRIPT
case XYSCRI:				/* SET SCRIPT */
    if ((y = cmkey(scrtab,1,"","echo",xxstring)) < 0) return(y);
    switch (y) {
      case 0: return(success = seton(&secho));
      default: return(-2);
    }
#endif /* NOSCRIPT */

default:
    break;
}

#ifndef NOLOCAL
switch (xx) {
case XYTERM:				/* SET TERMINAL */
    x = settrm();
    success = (x > 0) ? 1 : 0; 
    return(x);

#ifdef NT
case XYWIN95:				/* SET WIN95 workarounds */
    x = setwin95();
    success = (x > 0 ? 1 : 0);
    return(x);
#endif /* NT */

#ifdef OS2	
case XYDLR:				/* SET DIALER workarounds */
    x = setdialer();
    success = (x > 0 ? 1 : 0);
    return(x);

case XYTITLE:				/* SET TITLE of window */
    x = settitle();
    success = (x > 0 ? 1 : 0);
    return(x);
#endif /* OS2 */

#ifdef OS2MOUSE
case XYMOUSE:				/* SET MOUSE */
    return(success = setmou());
#endif /* OS2MOUSE */

#ifdef OS2
case XYBELL:				/* SET BELL */
    return( success = setbell() );

case XYPRTY:
    return( success = setprty() );
#endif /* OS2 */

default:
    break;
}
#endif /* NOLOCAL */

switch (xx) {

/* SET SEND/RECEIVE protocol parameters. */
 
case XYRECV:
case XYSEND:
    return(setsr(xx,rmsflg));
 
#ifndef NOLOCAL				/* Session log text/binary selection */
#ifdef UNIX				/* UNIX needs it */
#define _XYSESS
#endif /* UNIX */
#ifdef OSK				/* OS-9 too */
#define _XYSESS
#endif /* OSK */

#ifdef _XYSESS
case XYSESS:				/* SESSION-LOG */
    if ((x = cmkey(sfttab,nsfttab,"type of file","text",xxstring)) < 0)
      return(x);
    if ((y = cmcfm()) < 0) return(y);
    sessft = x;
    return(success = 1);
#undef _XYSESS
#endif /* _XYSESS */

case XYSPEE:				/* SET SPEED */
    if (network) {
	printf("\n?Speed cannot be set for network connections\n");
	return(success = 0);
    }
    lp = line;
    sprintf(lp,"Transmission rate for %s in bits per second",ttname);

    if ((x = cmkey(spdtab,nspd,line,"",xxstring)) < 0) {
	if (x == -3) printf("?value required\n");
	return(x);
    }
    if ((y = cmcfm()) < 0) return(y);
    if (!local) {
	printf("?Sorry, you must SET LINE first\n");
	return(success = 0);
    }
    zz = (long) x * 10L;
    if (zz == 70) zz = 75;		/* (see spdtab[] definition) */
    if (ttsspd(x) < 0)  {		/* Call ttsspd with cps, not bps! */
	printf("?Unsupported line speed - %ld\n",zz);
	return(success = 0);
    } else {
	speed = ttgspd();		/* Read it back */
	if (speed != zz)  {		/* Call ttsspd with cps, not bps! */
	    printf("?SET SPEED fails, speed is %ld\n",speed);
	    return(success = 0);
	}
	if (pflag &&
#ifndef NOSPL
	    cmdlvl == 0
#else
	    tlevel < 0
#endif /* NOSPL */
	    ) {
	    if (speed == 8880)
	      printf("%s, 75/1200 bps\n",ttname);
	    else
	      printf("%s, %ld bps\n",ttname,speed);
	}
	return(success = 1);
    }
#endif /* NOLOCAL */
 
  case XYXFER:				/* SET TRANSFER */
    if ((y = cmkey(tstab,nts,"","character-set",xxstring)) < 0) return(y);
    switch (y) {
#ifdef XFRCAN
      case XYX_CAN:			/* CANCELLATION */
	if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
	if (z == 0) {			/* OFF */
	    if ((y = cmcfm()) < 0) return(y);
	    xfrcan = 0;
	} else {
	    if ((y = cmnum("ASCII code for cancellation character","3",10,&x,
			   xxstring)) < 0)
	      return(y);
	    if (x > 31 && x != 127) {
		printf("Cancel character must be in ASCII control range\n");
		return(-9);
	    }
	    if ((y = cmnum("How many required to cause cancellation",
			   "2",10,&z, xxstring)) < 0)
	      return(y);
	    if (z < 2) {
		printf("Number must be 2 or greater\n");
		return(-9);
	    }
	    if ((y = cmcfm()) < 0) return(y);
	    xfrcan = 1;			/* CANCELLATION ON */
	    xfrchr = x;			/* Using this character */
	    xfrnum = z;			/* Needing this many of them */
	}
	return(success = 1);
#endif /* XFRCAN */
#ifndef NOCSETS
      case XYX_CSE:			/* CHARACTER-SET */
	if ((y = cmkey(tcstab,ntcs,"","transparent",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	if (rmsflg) {
	    sstate = setgen('S', "405", tcsinfo[y].designator, "");
	    return((int) sstate);
	} else {
	    tslevel = (y == TC_TRANSP) ? 0 : 1; /* transfer syntax level */
	    tcharset = y;		/* transfer character set */
	    return(success = 1);
	}
#endif /* NOCSETS */
      case XYX_LSH:			/* LOCKING-SHIFT */
	  if ((y = cmkey(lstab,nls,"","on",xxstring)) < 0)
	    return(y);
	  if ((x = cmcfm()) < 0) return(x);
	  lscapr = (y == 1) ? 1 : 0;	/* ON: requested = 1 */
	  lscapu = (y == 2) ? 2 : 0;	/* FORCED:  used = 1 */
	  return(success = 1);
#ifdef CK_XYZ	
      case XYX_PRO:			/* Protocol */
	return(setproto());
#endif /* CK_XYZ */

      case XYX_MOD:			/* Mode */
	if ((y = cmkey(xfrmtab,2,"","automatic",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);	
	xfermode = y;
	return(success = 1);

      case XYX_DIS:			/* Display */
	return(doxdis());

      case XYX_SLO:			/* Slow-start */
        return(seton(&slostart));

#ifndef NOSPL
      case XYX_CRC:			/* CRC */
        return(seton(&docrc));
#endif /* NOSPL */

      case XYX_BEL:			/* Bell */
        return(seton(&xfrbel));

      default:
	return(-2);
    }
}

switch (xx) {

#ifndef NOXMIT
  case XYXMIT:				/* SET TRANSMIT */
    return(setxmit());
#endif /* NOXMIT */

#ifndef NOCSETS
  case XYUNCS:				/* UNKNOWN-CHARACTER-SET */
    if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    unkcs = y;
    return(success = 1);
#endif /* NOCSETS */

#ifndef NOPUSH
#ifdef UNIX
case XYWILD:				/* WILDCARD-EXPANSION */
    if (nopush) {
        if ((x = cmcfm()) < 0) return(x);
        printf("Wildcard expansion is disabled\n");
        return(success = 0);
    }
    if ((y = cmkey(wildtab,2,"who expands wildcards","kermit",xxstring)) < 0)
      return(y);
    if ((x = cmcfm()) < 0) return(x);
    wildxpand = y;
    return(success = 1);
#endif /* UNIX */
#endif /* NOPUSH */

  case XYWIND:				/* WINDOW-SLOTS */
    if (protocol == PROTO_K) {
	y = cmnum("Number of sliding-window slots, 1 to 32",
		  "1", 10, &x, xxstring);
	y = setnum(&z,x,y,MAXWS);
    } else {
	y = cmnum("Window size for current protocol", "" ,10, &x, xxstring);
	y = setnum(&z,x,y,65472);
    }
    if (y < 0) return(y);
    if (protocol == PROTO_K)
      if (z < 1)
	z = 1;
#ifdef CK_XYZ
    if (protocol == PROTO_Z)		/* Must be a multiple of 64 */
      z = (z >> 6) << 6;
#endif /* CK_XYZ */

#ifdef COMMENT
    /* This is taken care of automatically now in protocol negotiation */
    if (maxtry < z) {
	printf("?Window slots must be less than retry limit\n");
	return(success = 0);
    }
#endif /* COMMENT */
    if (protocol == PROTO_K && rmsflg) { /* Set remote window size */
	wslotr = z;			/* Set local window size too */
	ptab[protocol].winsize = wslotr;
	tp = tmpbuf;
	sprintf(tp,"%d",z);
	sstate = setgen('S', "406", tp, "");
	return((int) sstate);
    }
    wslotr = z;				/* Set local window size */
    ptab[protocol].winsize = wslotr;
    swcapr = (wslotr > 1) ? 1 : 0;	/* Set window bit in capas word? */
    if (wslotr > 1) {			/* Window size > 1? */
	y = adjpkl(urpsiz,wslotr,bigrbsiz); /* Maybe adjust packet size */
	if (y != urpsiz) {		/* Did it change? */
	    urpsiz = y;
	    if (msgflg)
	    printf(
" Adjusting receive packet-length to %d for %d window slots\n",
		   urpsiz, wslotr);
	}
    }
    return(success = 1);
}

switch (xx) {

#ifndef NOSPL
  case XYOUTP:				/* OUTPUT command parameters */
    if ((y = cmkey(outptab,1,"OUTPUT command parameter","pacing",
		   xxstring)) < 0)
      return(y);
    switch(y) {				/* Which parameter */
      case 0:				/* PACING */
	y = cmnum("Milliseconds to pause between each OUTPUT character","100",
		  10,&x,xxstring);
	y = setnum(&z,x,y,16383);	/* Verify and get confirmation */
	if (y < 0) return(y);
	if (z < 0) z = 0;		/* (save some space) */
	pacing = z;
	return(success = 1);    
      default:				/* (shouldn't happen) */
	return(-2);
    }
#endif /* NOSPL */

#ifdef CK_SPEED
  case XYQCTL: {
    short *p;
    int zz;
    if ((z = cmkey(ctltab,2, "control-character prefixing option",""
		   ,xxstring)) < 0)
      return(z);
    /* Make space for a temporary copy of the prefixing table */

    p = (short *)malloc(256 * sizeof(short));
    if (!p) {
	printf("?Internal error - malloc failure\n");
	return(-9);
    }
    for (i = 0; i < 256; i++) p[i] = ctlp[i]; /* Copy current table */

    switch(z) {
      case 0:				/* UNPREFIXED control character */
      case 1:				/* PREFIXED control character */
	while (1) {			/* Collect a list of numbers */
#ifndef NOSPL
	    x_ifnum = 1;		/* Turn off complaints from eval() */
#endif /* NOSPL */
	    if ((x = cmnum((z == 0) ?
"\n Numeric ASCII value of control character that needs NO prefix,\n\
 or the word \"all\", or carriage return to complete the list" :
"\n Numeric ASCII value of control character that MUST BE prefixed,\n\
 or the word \"all\", or carriage return to complete the list",
			   "",10,&y,xxstring
			   )) < 0) {
#ifndef NOSPL
		x_ifnum = 0;
#endif /* NOSPL */
		if (x == -3)
		  break;
		if (x == -2) {
		    if (p) { free(p); p = NULL; }
		    debug(F110,"SET CONTROL atmbuf",atmbuf,0);
		    if (!xxstrcmp(atmbuf,"all",3) ||
			!xxstrcmp(atmbuf,"al",2) ||
			!xxstrcmp(atmbuf,"a",1)) {
			if ((x = cmcfm()) < 0) /* Get confirmation */
			  return(x);
			/* Set all values, but don't touch 0 */
			for (y = 1; y < 32; y++) ctlp[y] = (short) z;
			for (y = 127; y < 160; y++) ctlp[y] = (short) z;
			ctlp[255] = (short) z;
			/* Watch out for XON and XOFF */
			if (flow == FLO_XONX && z == 0) {
			    if (msgflg) {
				printf(
" XON/XOFF characters 17, 19, 145, 147 not affected.\n");
				printf(
#ifdef CK_RTSCTS
" SET FLOW NONE or RTS/CTS to transmit these characters unprefixed.\n"
#else
" SET FLOW NONE to transmit these characters unprefixed.\n"
#endif /* CK_RTSCTS */
				       );
			    }
			    ctlp[XON] =
			      ctlp[XOFF] =
				ctlp[XON+128] =
				  ctlp[XOFF+128] = 1;
			}
#ifdef TNCODE
			/* Watch out for TELNET IAC */
			if (network && (ttnproto == NP_TELNET) && z == 0) {
			    ctlp[255] = 1;
			    if (parity == 'e' || parity == 'm') ctlp[127] = 1;
			    ctlp[13] = 1;
			    if (msgflg)
			      printf(
			       " TELNET IAC = 255, CR = 13, not affected.\n");
			}
#endif /* TNCODE */
			return(success = 1);
		    } else {
			printf("?Please specify a number or the word ALL\n");
			return(-9);
		    }
		} else {
		    if (p) free(p);
		    return(x);
		}
	    }
#ifndef NOSPL
	    x_ifnum = 0;
#endif /* NOSPL */
	    zz = 1 - z;
	    if ((y >  31 && y < 127) ||	/* A specific numeric value */
		(y > 159 && y < 255) ||	/* Check that it is a valid */
		(y < zz) ||		/* control code. */
		(y > 255)) {
		printf("?Values allowed are: %d-31, 127-159, 255\n",zz);
		if (p) free(p);
		return(-9);
	    }
	    x = y & 127;		/* Get 7-bit value */
	    if ((z == 0) &&		/* If they are saying it is safe... */
		(y == 0	||		/* NUL = string terminator isn't */
		 ((flow == FLO_XONX) &&	/* If flow control is Xon/Xoff */
		  (x == XON || x == XOFF)) /* XON & XOFF chars not safe. */
		 )) {
		if (msgflg)
		  printf("Sorry, not while Xon/Xoff is in effect.\n");
		if (p) free(p);
		return(-9);
	    }
	    p[y] = (char) z;		/* All OK, set flag */
	}				/* End of while loop */
/*
  Get here only if they have made no mistakes.  Copy temporary table back to
  permanent one, then free temporary table and return successfully.
*/
	for (i = 0; i < 256; i++) ctlp[i] = p[i];
	if (p) free(p);
	return(success = 1);
      default:
	return(-2);
    }
  }
#endif /* CK_SPEED */
}

switch (xx) {

  case XYREPT:
    if ((y = cmkey(rpttab,2,"repeat-count compression parameter","",xxstring))
	< 0)
      return(y);
    switch(y) {
      case 0:
	return(success = seton(&rptena)); /* REPEAT COUNTS = ON, OFF */
      case 1:				/* REPEAT MININUM number */
	printf("(not implemented yet, nothing happens)\n");
	return(-9);
      case 2:				/* REPEAT PREFIX char */
	if ((x = cmnum("ASCII value","",10,&z,xxstring)) < 0)
	  return(x);
	if ((x = cmcfm()) < 0) return(x);
	if ((z > 32 && z < 63) || (z > 95 && z < 127)) {
	    if (y == 1) rptmin = (CHAR) z; else myrptq = (CHAR) z;
	    return(success = 1); 
	} else {
	    printf("?Illegal value for prefix character\n");
	    return(-9);
	}
    }

#ifndef NOSPL
  case XYALRM: {
#ifndef COMMENT
      long zz;
      zz = -1L;
      x_ifnum = 1;			/* Turn off internal complaints */
      y = cmnum("Seconds from now, or time of day as hh:mm:ss",
		"0" ,10, &x, xxstring);
      if (y < 0) {
	  if (y == -2) {		/* Invalid number or expression */
	      zz = tod2sec(atmbuf);	/* Convert to secs since midnight */
	      if (zz < 0L) {
		  printf("?Number, expression, or time of day required\n");
		  return(-9);
	      } else {
		  char now[32];		/* Current time */
		  char *p;
		  long tnow;
		  p = now;
		  ztime(&p);
		  tnow = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
		  if (zz < tnow)	/* User's time before now */
		    zz += 86400L;	/* So make it tomorrow */
		  zz -= tnow;		/* Seconds from now. */
	      }
	  } else
	    return(y);
      }
      if (x < 0) x = 0;
      if ((y = cmcfm()) < 0) return(y);
      if (zz > -1L) {			/* Time of day given? */
	  x = zz;
	  if (zz != (long) x) {
	      printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
		     );
	      return(-9);
	  }
      }
      return(setalarm((long)x));
  }
#else
/*
  This is to allow long values where int and long are not the same, e.g.
  on 16-bit systems.  But something is wrong with it.
*/
    if ((y = cmtxt("seconds from now", "0", &s, xxstring)) < 0)
      return(y);
    if (rdigits(s)) {
	return(setalarm(atol(s)));
    } else {
	printf("%s - not a number\n",s);
	return(-9);
    }
#endif /* COMMENT */
#endif /* NOSPL */

case XYPROTO:
    return(setproto());

#ifdef CK_SPEED
case XYPREFIX:    
    if ((z = cmkey(pfxtab, 4, "control-character prefixing option",
		   "", xxstring)) < 0)  
      return(z);
    if ((x = cmcfm()) < 0) return(x);
    setprefix(z);
    return(success = 1);
#endif /* CK_SPEED */

#ifndef NOSPL
case XYLOGIN:
    if ((z = cmkey(logintab, 3, "value for login script","userid",
		   xxstring)) < 0)  
      return(z);
    x = cmdgquo();
    cmdsquo(0);
    if ((y = cmtxt("text", /* z == LOGI_UID ? uidbuf : */ "", &s, NULL)) < 0) {
	cmdsquo(x);
	return(y);
    }
    cmdsquo(x);
    if ((int)strlen(s) > 63) {
	printf("Sorry, too long\n");
	return(-9);
    }
    switch(z) {
      case LOGI_UID:
	strcpy(uidbuf,s);
	break;
      case LOGI_PSW:
	strcpy(pwbuf,s);
	break;
      case LOGI_PRM:
	strcpy(prmbuf,s);
    }
    return(success = 1);
#endif /* NOSPL */
}

switch (xx) {

#ifndef NOSPL
  case XYSTARTUP:
    if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    DeleteStartupFile = (y != 0) ? 0 : 1;
    return(success = 1);
#endif /* NOSPL */

case XYTMPDIR:
    x = cmdir("Name of temporary directory","",&s,xxstring);
    if (x == -3)
      s = "";
    else if (x < 0)
      return(x);
    if ((x = cmcfm()) < 0) return(x);
    makestr(&tempdir,s);
    return(tempdir ? 1 : 0);

case XYDEST:				/* DESTINATION */
    return(setdest());

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("Not implemented - %s\n",cmdbuf);
    return(success = 0);
    }
}

#ifdef CK_TTYFD
extern int ttyfd;
#endif /* CK_TTYFD */ 

/*
  H U P O K  --  Is Hangup OK?

  Issues a warning and gets OK from user depending on whether a connection
  seems to be open and what the SET EXIT WARNING setting is.  Returns:
    0 if not OK to hang up or exit (i.e. user said No);
    nonzero if OK.  
  Argument x is used to differentiate the EXIT command from SET LINE / HOST.
*/
int
hupok(x) int x; {			/* Returns 1 if OK, 0 if not OK */
    int y, z = 1;

#ifdef VMS
    extern int batch;
    if (batch) return(1);
#endif /* VMS */

    debug(F101,"hupok x","",x);
    debug(F101,"hupok xitwarn","",xitwarn);
    debug(F101,"hupok network","",network);
#ifdef CK_TTYFD
    debug(F101,"hupok ttyfd","",ttyfd);
#endif /* CK_TTYFD */

    if ((local && xitwarn) ||		/* Is a connection open? */
	(!x && xitwarn == 2)) {		/* Or Always give warning on EXIT */
	int needwarn = 0;

	if (
#ifdef NETCONN
	    network
#else
	    0
#endif /* NETCONN */
	    ) {		/* Network? */
	    if (
#ifdef CK_TTYFD
		ttyfd > -1 &&
#endif /* CK_TTYFD */
		ttchk() >= 0
		)
	      needwarn = 1;
	    if (needwarn) {
		if (strcmp(ttname,"*"))
		  printf(
" A network connection to %s might still be active.\n",
			 ttname
			 );
		else
		  printf(
		   " An incoming network connection might still be active.\n"
			 );
	    }
	} else {			/* Serial connection */
	    if (carrier == CAR_OFF)	/* SET CARRIER OFF */
	      needwarn = 0;		/* so we don't care about carrier. */
	    else if ((y = ttgmdm()) >= 0) /* else, get modem signals */
	      needwarn = (y & BM_DCD);	/* Check for carrier */
	    else			/* If we can't get modem signals... */
	      needwarn =
#ifdef CK_TTYFD 
		(ttyfd > -1)		/* check tty file descriptor */
#else
		  1			/* or can't check ttyfd, then warn */
#endif /* CK_TTYFD */ 
		    ;
	    if (needwarn)
	      printf(
		     " A serial connection might still be active on %s.\n",
		     ttname
		     );
	}

/* If a warning was issued, get user's permission to EXIT. */

	if (needwarn || !x && xitwarn == 2 && local) {
	    z = getyesno(x ? "OK to close? " : "OK to exit? ");
	    if (z < -3) z = 0;
	}
    }
    return(z);
}

VOID
shoctl() {				/* SHOW CONTROL-PREFIXING */
#ifdef CK_SPEED
    int i;
    printf(
"\ncontrol quote = %d, applied to (0 = unprefixed, 1 = prefixed):\n\n",
	   myctlq);
    for (i = 0; i < 16; i++) {
	printf("  %3d: %d   %3d: %d ",i,ctlp[i], i+16, ctlp[i+16]);
	if (i == 15)
	  printf("  127: %d",ctlp[127]);
	else
	  printf("        ");
	printf("  %3d: %d   %3d: %d ",i+128,ctlp[i+128], i+144, ctlp[i+144]);
	if (i == 15)  printf("  255: %d",ctlp[255]);
	printf("\n");
    }
    printf("\n");
#endif /* CK_SPEED */
}

#ifndef NOPUSH
#ifdef CK_REXX
/*
  Rexx command.  Note, this is not OS/2-specific, because Rexx also runs
  on other systems where C-Kermit also runs, like the Amiga.
*/
#define REXBUFL 100			/* Change this if neccessary */
char rexxbuf[REXBUFL] = { '\0' };	/* Rexx's return value (string) */

int
dorexx() {
    int x, y;
    char *rexxcmd;

	if ((x = cmtxt("Rexx command","",&rexxcmd,xxstring)) < 0)    
	  return(x);
	strcpy(line,rexxcmd);
	rexxcmd = line;
#ifdef OS2
	return(os2rexx(rexxcmd,rexxbuf,REXBUFL));
#else /* !OS2 */
	printf("Sorry, nothing happens.\n");
	return(success = 0);
#endif /* OS2 */
}
#endif /* CK_REXX */
#endif /* NOPUSH */
#endif /* NOICP */
