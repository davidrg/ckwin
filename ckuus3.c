#include "ckcsym.h"			/* Symbol definitions */
#ifndef NOICP

/*  C K U U S 3 --  "User Interface" for C-Kermit, part 3  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
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
#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */
#include "ckuusr.h"			/* User interface symbols */
#ifdef OS2
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
extern int tt_modechg;
#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"			/* Microsoft TAPI */
#endif /* NT */
#endif /* OS2 */

#ifdef CK_RECALL
extern int cm_retry;
#endif /* CK_RECALL */

extern int cmdint;
extern int srvidl;

#ifndef NOPUSH
#ifndef NOFRILLS
#ifdef VMS
char editor[CKMAXPATH + 1] = "edit";
#else
char editor[CKMAXPATH + 1] = { NUL, NUL };
#endif /* VMS */
char editopts[128] = { NUL, NUL };
char editfile[CKMAXPATH + 1] = { NUL, NUL };
#ifdef BROWSER
char browser[CKMAXPATH + 1] = { NUL, NUL };
char browsopts[128] = { NUL, NUL };
char browsurl[4096] = { NUL, NUL };
#endif /* BROWSER */
#endif /*  NOFRILLS */
#endif /* NOPUSH */

/* Variables */

int cmd_quoting = 1;
extern int hints;

#ifdef CK_KERBEROS
char * k4pwprompt = NULL;		/* Kerberos 4 password prompt */
char * k4prprompt = NULL;		/* Kerberos 4 principal prompt */
char * k5pwprompt = NULL;		/* Kerberos 5 password prompt */
char * k5prprompt = NULL;		/* Kerberos 5 principal prompt */
#endif /* CK_KERBEROS */
#ifdef CK_SRP
char * srppwprompt = NULL;
#endif /* CK_SRP */

extern char * ckprompt, * ikprompt;	/* Default prompt */
extern xx_strp xxstring;

extern char * cdmsgfile[], * cdmsgstr;

extern int
  local, server, success, dest, sleepcan, inserver, flow, autoflow, binary,
  parity, escape, what, turn, duplex, backgrd, hwparity, stopbits, turnch,
  mdmtyp, network, quiet, nettype, carrier, debses, debtim, cdtimo, nlangs,
  bgset, pflag, msgflg, cmdmsk, suspend, techo, pacing, xitwarn, xitsta,
  outesc, cmd_cols, cmd_rows, ckxech, xaskmore, haveline, didsetlin, isguest;
#ifndef NOSERVER
  extern int en_pri;
#endif /* NOSERVER */

#ifdef IKSDCONF
extern int iksdcf;
#endif /* IKSDCONF */
#ifdef TCPSOCKET
  extern int tn_exit;
#endif /* TCPSOCKET */
#ifdef TNCODE
  char * tn_pr_uid = NULL;
#endif /* TNCODE */
  extern int exitonclose;

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */

extern int ttnproto;			/* Network protocol */

extern char *ccntab[];			/* Names of control chars */

#ifdef CK_APC
extern int apcactive, apcstatus;
#endif /* CK_APC */

#ifndef NOSCRIPT
extern int secho;			/* Whether SCRIPT cmd should echo */
#endif /* NOSCRIPT */

#ifdef DCMDBUF
extern char *atmbuf, *atxbuf;
#else
extern char atmbuf[], atxbuf[];
#endif /* DCMDBUF */
extern int cmflgs;

extern char psave[];
extern char uidbuf[];
extern int  sl_uid_saved;
int DeleteStartupFile = 0;

extern int cmdlvl;			/* Overall command level */

#ifndef NOSPL
_PROTOTYP( static int parsdir, (int) );
char pwbuf[PWBUFL+1]  = { NUL, NUL };
int pwflg = 0;
int pwcrypt = 0;
char prmbuf[PWBUFL+1] = { NUL, NUL };
int fndiags = 1;			/* Function diagnostics on/off */
int fnerror = 1;			/* Function error treatment */

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

/* Printer settings */

extern char * printername;		/* NULL if printer not redirected */
extern int printpipe;
extern int noprinter;
#ifdef PRINTSWI
int printtimo = 0;
char * printterm = NULL;
char * printsep = NULL;
int printertype = 0;
#ifdef BPRINT
int printbidi = 0;			/* SET BPRINTER (bidirectional) */
long pportspeed = 0L;			/* Bidirection printer port speed, */
int pportparity = 0;			/*  parity, */
int pportflow = FLO_KEEP;		/*  and flow control */
#endif /* BPRINT */
#ifdef OS2
extern int txt2ps;                      /* Text2PS conversion? */
extern int ps_width, ps_length;         /* Text2PS dimensions */
#endif /* OS2 */
#endif /* PRINTSWI */

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

/* File-transfer variable declarations */

#ifndef NOXFER
#ifdef CK_AUTODL
extern int cmdadl;
#endif /* CK_AUTODL */

#ifndef NOSERVER
extern int ngetpath;
extern char * getpath[];
#endif /* NOSERVER */

#ifdef PATTERNS
extern int patterns;
#endif /* PATTERNS */

extern struct ck_p ptab[];

extern CHAR sstate;			/* Protocol start state */
extern CHAR myctlq;			/* Control-character prefix */
extern CHAR myrptq;			/* Repeat-count prefix */

extern int protocol, size, spsiz, spmax, urpsiz, srvtim, srvcdmsg, slostart,
  srvdis, xfermode, ckdelay, keep, maxtry, unkcs, bctr, ebqflg, swcapr,
  wslotr, lscapr, lscapu, spsizr, rptena, rptmin, docrc, xfrcan, xfrchr,
  xfrnum, xfrbel, xfrint, srvping, g_xfermode;

#ifdef PIPESEND
extern int usepipes;
#endif /* PIPESEND */

#ifdef CKXXCHAR				/* DOUBLE / IGNORE char table */
extern int dblflag, ignflag, dblchar;
extern short dblt[];
#endif /* CKXXCHAR */

#ifdef CK_SPEED
extern short ctlp[];			/* Control-prefix table */
extern int prefixing;
static struct keytab pfxtab[] = {
    "all",         PX_ALL, 0,
    "cautious",    PX_CAU, 0,
    "minimal",     PX_WIL, 0,
    "none",        PX_NON, CM_INV
};
#endif /* CK_SPEED */
#endif /* NOXFER */

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

#ifndef NOLOCAL
#ifdef OS2ORUNIX
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
extern int batch;
#endif /* VMS */

/* Keyword tables for SET commands */

#ifdef CK_SPEED
struct keytab ctltab[] = {
    "prefixed",   1, 0,			/* Note, the values are important. */
    "unprefixed", 0, 0
};
#endif /* CK_SPEED */

static struct keytab oldnew[] = {
    "new", 0, 0,
    "old", 1, 0
};

#ifndef NOSPL
static struct keytab functab[] = {
    "diagnostics", FUNC_DI, 0,
    "error",       FUNC_ER, 0
};
static int nfunctab = (sizeof(functab) / sizeof(struct keytab));

struct keytab outptab[] = {		/* SET OUTPUT parameters */
    "pacing", 0, 0,			/* only one so far... */
    "special-escapes", 1, 0
};
int noutptab = (sizeof(outptab) / sizeof(struct keytab)); /* How many */
#endif /* NOSPL */

struct keytab chktab[] = {		/* Block check types */
    "1", 1, 0,				/* 1 =  6-bit checksum */
    "2", 2, 0,				/* 2 = 12-bit checksum */
    "3", 3, 0,				/* 3 = 16-bit CRC */
    "4", 4, CM_INV,			/* Same as B */
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
    "automatic", CAR_AUT, 0,		/* 2 */
    "off",       CAR_OFF, 0,		/* 0 */
    "on",        CAR_ON,  0		/* 1 */
};
int ncrr = 3;
#endif /* NOLOCAL */

struct keytab ooatab[] = {		/* On/Off/Auto table */
    "automatic", SET_AUTO, 0,		/* 2 */
    "off",       SET_OFF,  0,		/* 0 */
    "on",        SET_ON,   0		/* 1 */
};

struct keytab qvtab[] = {		/* Quiet/Verbose table */
    "quiet", 1, 0,
    "verbose", 0, 0
};
int nqvt = 2;

/* For SET DEBUG */

struct keytab dbgtab[] = {
    "off",        0,  0,
    "on",         1,  0,
    "session",    2,  0,
    "timestamps", 3,  0
};
int ndbg = 4;

#ifndef NOLOCAL
/* Transmission speeds */

#ifdef TTSPDLIST /* Speed table constructed at runtime . . . */

struct keytab * spdtab = NULL;
int nspd = 0;

#else
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
#ifdef BPS_1800
  "1800",     180,  0,
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
#ifdef BPS_460K
  "460800", 46080,  0,			/* Need 32 bits for this... */
#endif /* BPS_460K */
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
  "75/1200",888,  0,			/* Code "888" for split speed */
#endif /* BPS_7512 */
#ifdef BPS_76K
  "76800", 7680,  0,
#endif /* BPS_76K */
#ifdef BPS_921K
  "921600", 92160,0,			/* Need 32 bits for this... */
#endif /* BPS_921K */
  "9600",   960,  0
};
int nspd = (sizeof(spdtab) / sizeof(struct keytab)); /* How many speeds */
#endif /* TTSPDLIST */

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

/* Flow Control */

struct keytab cxtypesw[] = {
#ifdef DECNET
    "/decnet",         CXT_DECNET,  0,
#endif /* DECNET */
    "/direct-serial",  CXT_DIRECT,  0,
#ifdef DECNET
    "/lat",            CXT_LAT,     0,
#else
#ifdef SUPERLAT
    "/lat",            CXT_LAT,     0,
#endif /* SUPERLAT */
#endif /* DECNET */
    "/modem",          CXT_MODEM,   0,
#ifdef NPIPE
    "/named-pipe",     CXT_NPIPE,   0,
#endif /* NPIPE */
#ifdef NETBIOS
    "/netbios",        CXT_NETBIOS, 0,
#endif /* NETBIOS */
    "/remote",         CXT_REMOTE,  0,
#ifdef SSH
    "/ssh",            CXT_SSH,     0,
#endif /* SSH */
#ifdef TCPSOCKET
    "/tcpip",          CXT_TCPIP,   0,
#endif /* TCPSOCKET */
#ifdef ANYX25
    "/x.25",           CXT_X25,     0,
#endif /* ANYX25 */
    "", 0, 0
};
int ncxtypesw = (sizeof(cxtypesw) / sizeof(struct keytab));

struct keytab flotab[] = {		/* SET FLOW-CONTROL keyword table */
    "automatic", FLO_AUTO, CM_INV,	/* Not needed any more */
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
    "xon/xoff",  FLO_XONX, 0,
#endif /* Plan9 */
    "", 0, 0
};
int nflo = (sizeof(flotab) / sizeof(struct keytab)) - 1;

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
    "debug",     XYFT_D, 0,
    "text",      XYFT_T, 0
};
static int nsfttab = (sizeof(sfttab) / sizeof(struct keytab));
#endif /* NOLOCAL */

#ifndef NODIAL

#ifdef NETCONN				/* Networks directory depends */
int nnetdir = 0;			/* on DIAL code -- fix later... */
char *netdir[MAXDDIR+2];
#endif /* NETCONN */

_PROTOTYP( static int setdial, (int) );
_PROTOTYP( static int setdcd, (void) );
_PROTOTYP( static int cklogin, (void) );

#ifndef MINIDIAL
#ifdef OLDTBCODE
extern int tbmodel;			/* Telebit model ID */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */

extern MDMINF *modemp[];		/* Pointers to modem info structs */
extern struct keytab mdmtab[];		/* Modem types (in module ckudia.c) */
extern int nmdm;			/* Number of them */

_PROTOTYP(static int dialstr,(char **, char *));

extern int dialhng, dialtmo, dialksp, dialdpy, dialmhu, dialec, dialdc;
extern int dialrtr, dialint, dialudt, dialsrt, dialrstr, mdmwaitd;
extern int mdmspd, dialfc, dialmth, dialesc, dialfld, dialidt, dialpace;
extern int mdmspk, mdmvol, dialtest;

int dialcvt = 2;			/* DIAL CONVERT-DIRECTORY */
int dialcnf = 0;			/* DIAL CONFIRMATION */
int dialcon = 2;			/* DIAL CONNECT */
int dialcq  = 0;			/* DIAL CONNECT AUTO quiet/verbose */
extern long dialmax, dialcapas;
int usermdm = 0;
extern int ndialdir;
extern char *dialini,   *dialmstr, *dialmprmt, *dialdir[], *dialcmd,  *dialnpr,
 *dialdcon, *dialdcoff, *dialecon, *dialecoff, *dialhcmd,  *dialx3,
 *dialhwfc, *dialswfc,  *dialnofc, *dialtone,  *dialpulse, *dialname, *diallac;
extern char *diallcc,   *dialixp,  *dialixs,   *dialldp,   *diallds,  *dialtfp,
 *dialpxi,  *dialpxo,   *dialsfx,  *dialaaon,  *dialaaoff;
extern char *diallcp,   *diallcs,  *dialini2,  *dialmac;
extern char *dialspoff, *dialspon, *dialvol1,  *dialvol2,  *dialvol3;

char *dialtocc[MAXTPCC] = { NULL, NULL };
int ndialtocc = 0;
char *dialpucc[MAXTPCC] = { NULL, NULL };
int ndialpucc = 0;

char *dialtfc[MAXTOLLFREE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int ntollfree = 0;

char *dialpxx[MAXPBXEXCH] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int ndialpxx = 0;

char *diallcac[MAXLOCALAC] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
int nlocalac = 0;

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
    "compression",	XYDDC,   CM_INV,
    "data-compression", XYDDC,   0,
    "dial-command",     XYDDIA,  0,
    "error-correction",	XYDEC,   0,
    "escape-character",	XYDESC,  0,
    "flow-control",	XYDFC,   0,
    "hangup-method",	XYDMHU,  0,
#ifndef NOXFER
    "kermit-spoof",	XYDKSP,  0,
#endif /* NOXFER */
    "maximum-speed",	XYDMAX,  0,
    "name",		XYDNAM,  0,
    "speaker",          XYDSPK,  0,
    "speed-matching",	XYDSPD,  0,
    "type",		XYDTYP,  0,
    "volume",           XYDVOL,  0
};
int nsetmdm = (sizeof(setmdm) / sizeof(struct keytab));

struct keytab voltab[] = {
    "high",   3,  0,
    "low",    1,  0,
    "medium", 2,  0
};

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
    "force-long-distance", XYDFLD, 0,
    "hangup",		XYDHUP, 0,
    "ignore-dialtone",  XYDIDT, 0,
    "interval",		XYDINT, 0,
    "in",		XYDINI, CM_INV|CM_ABR,
    "init-string",	XYDINI, CM_INV,
    "intl-prefix",	XYDIXP, 0,
    "intl-suffix",	XYDIXS, 0,
#ifndef NOXFER
    "kermit-spoof",	XYDKSP, CM_INV,
#endif /* NOXFER */
    "lc-area-codes",    XYDLLAC, 0,
    "lc-prefix",        XYDLCP, 0,
    "lc-suffix",        XYDLCS, 0,
    "ld-prefix",	XYDLDP, 0,
    "ld-suffix",	XYDLDS, 0,
    "local-area-code",  XYDLAC, CM_INV,
    "local-prefix",     XYDLCP, CM_INV,
    "local-suffix",     XYDLCS, CM_INV,
    "m",                XYDMTH, CM_INV|CM_ABR,
#ifndef NOSPL
    "macro",            XYDMAC, 0,	/* 195 */
#endif /* NOSPL */
#ifdef MDMHUP
    "me",               XYDMTH, CM_INV|CM_ABR,
#endif /* MDMHUP */
    "method",		XYDMTH, 0,
    "mnp-enable",	XYDMNP, CM_INV,	/* obsolete but still accepted */
#ifdef MDMHUP
    "modem-hangup",	XYDMHU, CM_INV,
#endif /* MDMHUP */
    "pacing",           XYDPAC,  0,
    "pbx-exchange",     XYDPXX,  0,
    "pbx-inside-prefix",XYDPXI,  0,
    "pbx-outside-prefix",XYDPXO, 0,
    "prefix",		XYDNPR,  0,
    "pulse-countries",  XYDPUCC, 0,
    "restrict",         XYDRSTR, 0,
    "retries",		XYDRTM,  0,
    "sort",             XYDSRT,  0,
    "speed-matching",	XYDSPD,  CM_INV,
    "string",		XYDSTR,  CM_INV,
    "suffix",		XYDSFX,  0,
    "test",             XYDTEST, 0,
    "timeout",		XYDTMO,  0,
    "tf-area-code",     XYDTFC,  CM_INV,
    "tf-prefix",        XYDTFP,  CM_INV,
    "toll-free-area-code",XYDTFC,0,
    "toll-free-prefix", XYDTFP,  0,
    "tone-countries",   XYDTOCC, 0
};
int ndial = (sizeof(dialtab) / sizeof(struct keytab));

#ifdef MDMHUP
struct keytab mdmhang[] = {
    "modem-command", 1, 0,
    "rs232-signal",  0, 0,
    "v24-signal",    0, CM_INV
};
#endif /* MDMHUP */

static struct keytab mdmcmd[] = {
    "autoanswer",       XYDS_AN, 0,	/* autoanswer */
    "compression",	XYDS_DC, 0,	/* data compression */
    "dial-mode-prompt", XYDS_MP, 0,	/* dial mode prompt */
    "dial-mode-string", XYDS_MS, 0,	/* dial mode string */
    "error-correction",	XYDS_EC, 0,	/* error correction */
    "hangup-command",	XYDS_HU, 0,	/* hangup command */
    "hardware-flow",	XYDS_HW, 0,	/* hwfc */
    "ignore-dialtone",	XYDS_ID, 0,	/* ignore dialtone */
    "init-string",	XYDS_IN, 0,	/* init string */
    "no-flow-control",	XYDS_NF, 0,	/* no flow control */
    "predial-init",     XYDS_I2, 0,     /* last-minute setup commands */
    "pulse",            XYDS_DP, 0,	/* pulse */
    "software-flow",	XYDS_SW, 0,	/* swfc */
    "speaker",          XYDS_SP, 0,     /* Speaker */
    "tone",             XYDS_DT, 0,	/* tone */
    "volume",           XYDS_VO, 0      /* Volume */
};
static int nmdmcmd = (sizeof(mdmcmd) / sizeof(struct keytab));

struct keytab dial_fc[] = {
    "auto",     FLO_AUTO, 0,
    "none",     FLO_NONE, 0,
    "rts/cts",  FLO_RTSC, 0,
    "xon/xoff", FLO_XONX, 0
};

struct keytab dial_m[] = {		/* DIAL METHOD */
    "auto",    XYDM_A, 0,
    "default", XYDM_D, 0,
    "pulse",   XYDM_P, 0,
    "tone",    XYDM_T, 0
};
int ndial_m = (sizeof(dial_m)/sizeof(struct keytab));

#ifdef CK_TAPI
struct keytab tapitab[] = {		/* Top-Level Microsoft TAPI */
    "configure-line",     XYTAPI_CFG,  0,
    "dialing-properties", XYTAPI_DIAL, 0
};
int ntapitab = (sizeof(tapitab)/sizeof(struct keytab));

struct keytab settapitab[] = {		/* SET Microsoft TAPI */
    "inactivity-timeout", XYTAPI_INA,  0,
    "line",               XYTAPI_LIN,  0,
    "location",           XYTAPI_LOC,  0,
    "manual-dialing",     XYTAPI_MAN,  0,
    "modem-dialing",	  XYTAPI_PASS, 0,
    "modem-lights",       XYTAPI_LGHT, 0,
    "phone-number-conversions",	  XYTAPI_CON,  0,
    "port",               XYTAPI_LIN,  CM_INV,
    "post-dial-terminal", XYTAPI_PST,  0,
    "pre-dial-terminal",  XYTAPI_PRE,  0,
    "use-windows-configuration", XYTAPI_USE, 0,
    "wait-for-credit-card-tone", XYTAPI_BNG, 0
};
int nsettapitab = (sizeof(settapitab)/sizeof(struct keytab));

struct keytab * tapiloctab = NULL;	/* Microsoft TAPI Locations */
int ntapiloc = 0;
extern struct keytab * tapilinetab;	/* Microsoft TAPI Line Devices */
extern int ntapiline;
extern int tttapi;			/* TAPI in use */
extern int tapipass;			/* TAPI Passthrough mode */
extern int tapiconv;			/* TAPI Conversion mode */
extern int tapilights;
extern int tapipreterm;
extern int tapipostterm;
extern int tapimanual;
extern int tapiinactivity;
extern int tapibong;
extern int tapiusecfg;
#endif /* CK_TAPI */
#endif /* NODIAL */

#ifndef NOPUSH
extern int nopush;
#ifdef UNIX
struct keytab wildtab[] = {		/* SET WILDCARD-EXPANSION */
    "kermit",  0, 0,
    "shell",   1, 0
};
struct keytab wdottab[] = {		/* cont'd */
    "/match-dot-files",    1, 0,
    "/no-match-dot-files", 0, 0
};
extern int wildxpand;
#endif /* UNIX */
#endif /* NOPUSH */

#ifdef NETCONN
extern struct keytab netcmd[], netkey[];
extern int nnets, nnetkey;
#ifdef TCPSOCKET
extern struct keytab tcpopt[];
extern int ntcpopt;
#ifndef NOTCPOPTS
#ifdef SO_LINGER
_PROTOTYP( int linger, (int,int) );
#endif /* SO_LINGER */
#ifdef TCP_NODELAY
_PROTOTYP( int no_delay, (int) );
#endif /* TCP_NODELAY */
#endif /* NOTCPOPTS */
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

#ifndef IBMX25
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
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifdef TLOG
static struct keytab vbtab[] = {
    "brief",   0, 0,
#ifdef OS2ORUNIX
    "ftp",     2, 0,
#endif /* OS2ORUNIX */
    "verbose", 1, 0
};
int nvb = (sizeof(vbtab) / sizeof(struct keytab));
#endif /* TLOG */

#ifdef CKSYSLOG
static struct keytab syslogtab[] = {
    "all",         SYSLG_CX, 0,
    "commands",    SYSLG_CM, 0,
    "connection",  SYSLG_AC, 0,
    "debug",       SYSLG_DB, 0,
    "dial",        SYSLG_DI, 0,
    "file-access", SYSLG_FA, 0,
    "file-create", SYSLG_FC, 0,
    "login",       SYSLG_LI, 0,
    "none",        SYSLG_NO, 0,
    "protocol",    SYSLG_PR, 0
};
int nsyslog = (sizeof(syslogtab) / sizeof(struct keytab));
#endif /* CKSYSLOG */

/* Parity keyword table */

struct keytab partbl[] = {
    "even",    'e', 0,
#ifdef HWPARITY
    "hardware",'H', 0,
#endif /* HWPARITY */
    "mark",    'm', 0,
    "none",     0 , 0,
    "odd",     'o', 0,
    "space",   's', 0
};
int npar = (sizeof(partbl) / sizeof(struct keytab));

#ifdef HWPARITY
struct keytab hwpartbl[] = {
/* Add mark and space if needed and possible */
    "even",    'e', 0,
#ifdef OS2
    "mark",    'm', 0,
#endif /* OS2 */
    "odd",     'o', 0,
#ifdef OS2
    "space",   's', 0,
#endif /* OS2 */
    "", 0, 0
};
int nhwpar = (sizeof(hwpartbl) / sizeof(struct keytab)) - 1;
#endif /* HWPARITY */

/* On/Off table */

struct keytab onoff[] = {
    "off",       0, 0,
    "on",        1, 0
};

struct keytab cdtab[] = {
    "message",   XYCD_M, 0,
    "path",      XYCD_P, 0
};
int ncdtab = (sizeof(cdtab) / sizeof(struct keytab));

struct keytab cdmsg[] = {
    "file",      2, 0,
    "off",       0, 0,
    "on",        1, 0
};
int ncdmsg = (sizeof(cdmsg) / sizeof(struct keytab));

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
    "local",     1, 0,			/* ECHO values */
    "off",       0, CM_INV,
    "on",        1, CM_INV,
    "remote",    0, 0
};
int nrlt = (sizeof(rltab) / sizeof(struct keytab));

/* Incomplete File Disposition table */

struct keytab ifdtab[] = {
    "discard", SET_OFF, 0,
    "keep",    SET_ON,  0
};

struct keytab ifdatab[] = {
    "auto",    SET_AUTO, 0,
    "discard", SET_OFF,  0,
    "keep",    SET_ON,   0
};

char * ifdnam[] = { "discard", "keep", "auto" };

/* SET TAKE parameters table */
static
struct keytab taktab[] = {
    "echo",  0, 0,
    "error", 1, 0,
    "off",   2, CM_INV,			/* For compatibility */
    "on",    3, CM_INV			/* with MS-DOS Kermit... */
};

#ifndef NOSPL
#ifdef COMMENT
/* not used */
static
struct keytab suftab[] = {		/* (what to do with) STARTUP-FILE */
    "delete", 1, 0,
    "keep",   0, 0
};
#endif /* COMMENT */

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
static struct keytab scmdtab[] = {
#ifdef CK_AUTODL
    "autodownload", SCMD_ADL, 0,
#endif /* CK_AUTODL */
    "bytesize",  SCMD_BSZ, 0,
    "cbreak",    876, CM_INV,
#ifdef OS2
    "color",     SCMD_COL, 0,
    "cursor-position", SCMD_CUR, 0,
#endif /* OS2 */
    "height",    SCMD_HIG, 0,
    "interruption", SCMD_INT, 0,
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
static int nbytt = (sizeof(scmdtab) / sizeof(struct keytab));

#ifndef NOSERVER
/* Server parameters table */
static struct keytab srvtab[] = {
    "cd-message",   XYSERC, 0,
    "display",      XYSERD, 0,
    "get-path",     XYSERP, 0,
    "idle-timeout", XYSERI, 0,
    "keepalive",    XYSERK, 0,
    "login",        XYSERL, 0,
    "timeout",      XYSERT, 0
};
static int nsrvt = (sizeof(srvtab) / sizeof(struct keytab));
#endif /* NOSERVER */

static struct keytab sleeptab[] = {	/* SET SLEEP table */
    "cancellation",  0,   0
};

static struct keytab tstab[] = {	/* SET TRANSFER/XFER table */
    "bell",            XYX_BEL, 0,
#ifdef XFRCAN
    "cancellation",    XYX_CAN, 0,
#endif /* XFRCAN */
#ifndef NOCSETS
    "character-set",   XYX_CSE, 0,
#endif /* NOCSETS */
#ifndef NOSPL
    "crc-calculation", XYX_CRC, 0,
#endif /* NOSPL */
    "display",         XYX_DIS, 0,
    "interruption",    XYX_INT, 0,
    "locking-shift",   XYX_LSH, 0,
    "mode",            XYX_MOD, 0,
#ifdef PIPESEND
    "pipes",           XYX_PIP, 0,
#endif /* PIPESEND */
#ifdef CK_XYZ
    "protocol",        XYX_PRO, 0,
#endif /* CK_XYZ */
    "slow-start",      XYX_SLO, 0,
    "", 0, 0
};
static int nts = (sizeof(tstab) / sizeof(struct keytab)) - 1;

static struct keytab rtstab[] = {	/* REMOTE SET TRANSFER/XFER table */
#ifndef NOCSETS
    "character-set",   XYX_CSE, 0,
#endif /* NOCSETS */
    "mode",            XYX_MOD, 0
};
static int nrts = (sizeof(rtstab) / sizeof(struct keytab));

struct keytab xfrmtab[] = {		/* TRANSFER MODE table */
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
extern int tn_nlm, tn_b_nlm, tn_b_meu, tn_b_ume, tn_b_xfer, tn_sb_bug;
extern int tn_no_encrypt_xfer;
extern int tn_wait_flg, tn_duplex;
extern int sl_tn_saved;
extern int tn_infinite;
extern int tn_rem_echo;
extern int tn_deb;
extern int tn_auth_how;
extern int tn_auth_enc;
#ifdef CK_AUTHENTICATION
static struct keytab setauth[] = {
#ifdef CK_KERBEROS
    "k4",        AUTH_KRB4, CM_INV,
    "k5",        AUTH_KRB5, CM_INV,
    "kerberos4", AUTH_KRB4, 0,
    "kerberos5", AUTH_KRB5, 0,
    "kerberos_iv",AUTH_KRB4, CM_INV,
    "kerberos_v", AUTH_KRB5, CM_INV,
    "krb4",      AUTH_KRB4, CM_INV,
    "krb5",      AUTH_KRB5, CM_INV,
#endif /* CK_KERBEROS */
#ifdef CK_SRP
    "srp",       AUTH_SRP,  0,
#endif /* CK_SRP */
#ifdef CK_SSL
    "ssl",      AUTH_SSL,   0,
    "tls",      AUTH_TLS,   0,
#endif /* CK_SSL */
    "",         0,      0
};
static int nsetauth = sizeof(setauth)/sizeof(struct keytab) - 1;
#ifdef CK_KERBEROS
extern char * krb5_d_principal;		/* Default principal */
extern char * krb5_d_instance;
extern char * krb5_d_realm;		/* Default realm */
extern char * krb5_d_cc;		/* Default credentials cache */
extern char * krb5_d_srv;               /* Default service name */
extern int    krb5_d_lifetime;          /* Default lifetime */
extern int    krb5_d_forwardable;
extern int    krb5_d_proxiable;
extern int    krb5_d_renewable;
extern int    krb5_autoget;
extern int    krb5_autodel;
extern int    krb5_d_getk4;
extern int    krb5_checkaddrs;		/* Check TGT Addrs */

extern struct krb4_init_data krb4_init;
extern char * krb4_d_principal;		/* Default principal */
extern char * krb4_d_realm;		/* Default realm */
extern char * krb4_d_srv;               /* Default service name */
extern int    krb4_d_lifetime;          /* Default lifetime */
extern int    krb4_d_preauth;
extern char * krb4_d_instance;
extern int    krb4_autoget;
extern int    krb4_autodel;
extern int    krb4_checkaddrs;		/* Check TGT Addrs */
#ifdef KRB4
extern int    k4debug;
#endif /* KRB4 */
static struct keytab krbver[] = {
    "4",                 4, 0,
    "5",                 5, 0,
    "iv",                4, CM_INV,
    "v",                 5, CM_INV
};
static int nkrbver = sizeof(krbver)/sizeof(struct keytab);

static struct keytab kdestab[] = {
    "never",            KRB_DEL_NO, 0,
    "no",               KRB_DEL_NO, CM_INV,
    "on-close",         KRB_DEL_CL, 0,
    "on-exit",          KRB_DEL_EX, 0
};
static int nkdestab = sizeof(kdestab)/sizeof(struct keytab);

static struct keytab k4tab[] = {
    "autodel",           XYKRBDEL, CM_INV,
    "autodestroy",       XYKRBDEL, 0,
    "autoget",           XYKRBGET, 0,
    "check-address",     XYKRBADR, 0,
    "debug",             XYKRBDBG, CM_INV,
    "instance",          XYKRBINS, 0,
    "lifetime",          XYKRBLIF, 0,
    "preauth",           XYKRBPRE, 0,
    "principal",         XYKRBPR,  0,
    "prompt",            XYKRBPRM, 0,
    "realm",             XYKRBRL,  0,
    "service-name",      XYKRBSRV, 0
};
static int nk4tab = sizeof(k4tab)/sizeof(struct keytab);

static struct keytab k5tab[] = {
    "autodel",           XYKRBDEL, CM_INV,
    "autodestroy",       XYKRBDEL, 0,
    "autoget",           XYKRBGET, 0,
    "cc",                XYKRBCC,  CM_INV,
    "check-address",     XYKRBADR, 0,
    "credentials-cache", XYKRBCC,  0,
    "forwardable",       XYKRBFWD, 0,
    "get-k4-tgt",        XYKRBK5K4,0,
    "instance",          XYKRBINS, 0,
    "lifetime",          XYKRBLIF, 0,
    "principal",         XYKRBPR,  0,
    "prompt",            XYKRBPRM, 0,
    "proxiable",         XYKRBPRX, 0,
    "realm",             XYKRBRL,  0,
    "renewable",         XYKRBRNW, 0,
    "service-name",      XYKRBSRV, 0
};
static int nk5tab = sizeof(k5tab)/sizeof(struct keytab);

#define KRB_PW_PRM 1
#define KRB_PR_PRM 2

static struct keytab krbprmtab[] = {
    "password",  KRB_PW_PRM, 0,
    "principal", KRB_PR_PRM, 0
};

#endif /* CK_KERBEROS */
#ifdef CK_SRP
static struct keytab srptab[] = {
    "prompt",            XYSRPPRM, 0
};
static int nsrptab = sizeof(srptab)/sizeof(struct keytab);
#define SRP_PW_PRM 1

static struct keytab srpprmtab[] = {
    "password",  SRP_PW_PRM, 0
};
#endif /* CK_SRP */
#ifdef CK_SSL
static struct keytab ssltab[] = {
    "certs-ok",          XYSSLCOK,  CM_INV,
    "cipher-list",       XYSSLCL,   0,
    "crl-dir",           XYSSLCRLD, 0,
    "crl-file",          XYSSLCRL,  0,
    "debug",             XYSSLDBG,  0,
    "dh-key-file",       XYSSLDKFL, CM_INV,
    "dh-param-file",     XYSSLDPFL, 0,
    "dsa-cert-file",     XYSSLDCFL, 0,
    "dsa-key-file",      XYSSLDKFL, 0,
    "dummy",             XYSSLDUM,  CM_INV,
    "only",              XYSSLON,   CM_INV,
    "rsa-cert-file",     XYSSLRCFL, 0,
    "rsa-key-file",      XYSSLRKFL, 0,
    "verbose",           XYSSLVRB,  0,
    "verify",            XYSSLVRF,  0,
    "verify-dir",        XYSSLVRFD, 0,
    "verify-file",       XYSSLVRFF, 0
};
static int nssltab = sizeof(ssltab)/sizeof(struct keytab);
static struct keytab sslvertab[] = {
    "fail-if-no-peer-cert", SSL_VERIFY_PEER |
                            SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0,
    "no",               SSL_VERIFY_NONE, 0,
    "none",             SSL_VERIFY_NONE, CM_INV,
    "off",              SSL_VERIFY_NONE, CM_INV,
    "on",               SSL_VERIFY_PEER, CM_INV,
    "peer-cert",        SSL_VERIFY_PEER, 0
};
static int nsslvertab = sizeof(sslvertab)/sizeof(struct keytab);
#endif /* CK_SSL */
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
int cx_type = CX_AUTO;
extern int sl_cx_type;
#endif /* CK_ENCRYPTION */
extern char *tcp_address;

static struct keytab tnnegtab[] = {	/* TELNET NEGOTIATION table */
    "accepted",  TN_NG_AC, 0,
    "refused",   TN_NG_RF, 0,
    "req",       TN_NG_RQ, CM_INV|CM_ABR,
    "requ",      TN_NG_RQ, CM_INV|CM_ABR,
    "reque",     TN_NG_RQ, CM_INV|CM_ABR,
    "reques",    TN_NG_RQ, CM_INV|CM_ABR,
    "request",   TN_NG_RQ, CM_INV|CM_ABR,
    "requeste",  TN_NG_RQ, CM_INV|CM_ABR,
    "requested", TN_NG_RQ, 0,
    "required",  TN_NG_MU, 0
};
static int ntnnegtab = sizeof(tnnegtab)/sizeof(struct keytab);

#ifdef CK_ENCRYPTION
static struct keytab typkwd[] = {
    "/type", 0, CM_ARG
};

static struct keytab tnenctab[] = {	/* TELNET ENCRYPTION table */
    "accepted",   TN_NG_AC,    CM_INV,
    "refused",    TN_NG_RF,    CM_INV,
    "req",        TN_NG_RQ,    CM_INV|CM_ABR,
    "requ",       TN_NG_RQ,    CM_INV|CM_ABR,
    "reque",      TN_NG_RQ,    CM_INV|CM_ABR,
    "reques",     TN_NG_RQ,    CM_INV|CM_ABR,
    "request",    TN_NG_RQ,    CM_INV|CM_ABR,
    "requeste",   TN_NG_RQ,    CM_INV|CM_ABR,
    "requested",  TN_NG_RQ,    CM_INV,
    "required",   TN_NG_MU,    CM_INV,
    "start",      TN_EN_START, CM_INV,
    "stop",       TN_EN_STOP,  CM_INV,
    "type",       TN_EN_TYP,   0
};
static int ntnenc = sizeof(tnenctab)/sizeof(struct keytab) ;

#endif /* CK_ENCRYPTION */

static struct keytab tnbugtab[] = {	/* TELNET BUG table */
    "binary-me-means-u-too", 0, 0,
    "binary-u-means-me-too", 1, 0,
    "infinite-loop-check",   2, 0,
    "sb-implies-will-do",    3, 0
};

#ifdef CK_ENVIRONMENT
static struct keytab tnenvtab[] = {	/* TELNET ENVIRONMENT table */
    "acct",	TN_ENV_ACCT,	0,
    "display",	TN_ENV_DISP,	0,
    "job",	TN_ENV_JOB,	0,
    "off",      TN_ENV_OFF,     CM_INV,
    "on",       TN_ENV_ON,      CM_INV,
    "printer",	TN_ENV_PRNT,	0,
    "systemtype",TN_ENV_SYS,	0,
    "user", 	TN_ENV_USR, 	0,
#ifdef COMMENT
    "uservar",	TN_ENV_UVAR,	CM_INV
#endif /* COMMENT */
    "", 0, 0
};
static int ntnenv = sizeof(tnenvtab)/sizeof(struct keytab) - 1;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
static struct keytab tnauthtab[] = {	/* TELNET AUTHENTICATION table */
    "accepted",   TN_NG_AC,  CM_INV,
    "encrypt-flag", TN_AU_ENC, 0,
    "forwarding", TN_AU_FWD,   0,
    "how-flag",   TN_AU_HOW,   0,
    "refused",    TN_NG_RF,  CM_INV,
    "req",        TN_NG_RQ,  CM_INV|CM_ABR,
    "requ",       TN_NG_RQ,  CM_INV|CM_ABR,
    "reque",      TN_NG_RQ,  CM_INV|CM_ABR,
    "reques",     TN_NG_RQ,  CM_INV|CM_ABR,
    "request",    TN_NG_RQ,  CM_INV|CM_ABR,
    "requeste",   TN_NG_RQ,  CM_INV|CM_ABR,
    "requested",  TN_NG_RQ,  CM_INV,
    "required",   TN_NG_MU,  CM_INV,
    "type",       TN_AU_TYP, 0
};
static int ntnauth = sizeof(tnauthtab)/sizeof(struct keytab) ;

struct keytab autyptab[] = {	/* TELNET AUTHENTICATION TYPE table */
    "automatic",  AUTH_AUTO, 0,
#ifdef CK_KERBEROS
    "k4",         AUTH_KRB4, CM_INV,
    "k5",         AUTH_KRB5, CM_INV,
    "kerberos4",  AUTH_KRB4, 0,
    "kerberos5",  AUTH_KRB5, 0,
    "kerberos_iv",AUTH_KRB4, CM_INV,
    "kerberos_v", AUTH_KRB5, CM_INV,
    "krb4",       AUTH_KRB4, CM_INV,
    "krb5",       AUTH_KRB5, CM_INV,
#endif /* CK_KERBEROS */
    "none",       AUTH_NONE, 0,
#ifdef NT
    "ntlm",       AUTH_NTLM, 0,
#endif /* NT */
#ifdef CK_SRP
    "srp",        AUTH_SRP,  0,
#endif /* CK_SRP */
#ifdef CK_SSL
    "ssl",        AUTH_SSL,  0,
#endif /* CK_SSL */
    "", 0, 0
};
int nautyp = sizeof(autyptab)/sizeof(struct keytab) - 1;

struct keytab auhowtab[] = {	/* TELNET AUTHENTICATION HOW table */
    "any",     TN_AUTH_HOW_ANY,     0,
    "mutual",  TN_AUTH_HOW_MUTUAL,  0,
    "one-way", TN_AUTH_HOW_ONE_WAY, 0,
    "", 0, 0
};
int nauhow = sizeof(auhowtab)/sizeof(struct keytab) - 1;

struct keytab auenctab[] = {	/* TELNET AUTHENTICATION ENCRYPT table */
    "any",     TN_AUTH_ENC_ANY,     0,
    "none",    TN_AUTH_ENC_NONE,    0,
    "telopt",  TN_AUTH_ENC_TELOPT,  0,
    "", 0, 0
};
int nauenc = sizeof(auenctab)/sizeof(struct keytab) - 1;
#endif /* CK_AUTHENTICATION */

#define TN_NL_BIN 3
#define TN_NL_NVT 4
static struct keytab tn_nlmtab[] = {	/* TELNET NEWLINE-MODE table */
    "binary-mode", TN_NL_BIN, 0,	/* Binary mode */
    "nvt",    TN_NL_NVT, 0,		/* NVT mode */
    "off",    TNL_CRNUL, CM_INV,	/* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  CM_INV,	/* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    CM_INV		/* CR only (out of spec) */
};
static int ntn_nlm = (sizeof(tn_nlmtab) / sizeof(struct keytab));

static struct keytab tnlmtab[] = {	/* TELNET NEWLINE-MODE table */
    "cr",     TNL_CR,    CM_INV,	/* CR only (out of spec) */
    "cr-lf",  TNL_CRLF,  CM_INV,	/* CR-LF (TELNET spec) */
    "cr-nul", TNL_CRNUL, CM_INV,	/* CR-NUL (TELNET spec) */
    "lf",     TNL_LF,    CM_INV,	/* LF instead of CR-LF */
    "off",    TNL_CRNUL, 0,		/* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF,  0,		/* CR-LF (TELNET spec) */
    "raw",    TNL_CR,    0		/* CR only (out of spec) */
};
static int ntnlm = (sizeof(tnlmtab) / sizeof(struct keytab));

struct keytab tntab[] = {
#ifdef CK_AUTHENTICATION
    "authentication",       CK_TN_AU,  0,
#endif /* CK_AUTHENTICATION */
    "b",                    CK_TN_BM,  CM_INV|CM_ABR,
    "bi",                   CK_TN_BM,  CM_INV|CM_ABR,
    "bin",                  CK_TN_BM,  CM_INV|CM_ABR,
    "bina",                 CK_TN_BM,  CM_INV|CM_ABR,
    "binar",                CK_TN_BM,  CM_INV|CM_ABR,
    "binary",               CK_TN_BM,  CM_INV|CM_ABR,
    "binary-",              CK_TN_BM,  CM_INV|CM_ABR,
    "binary-mode",          CK_TN_BM,  CM_INV,
    "binary-transfer-mode", CK_TN_XF,  0,
    "binary-xfer-mode",     CK_TN_XF,  CM_INV,
    "bug",                  CK_TN_BUG, 0,
    "debug",                CK_TN_DB,  0,
    "echo",                 CK_TN_EC,  0,
#ifdef CK_ENCRYPTION
    "encryption",      CK_TN_ENC,  0,
#endif /* CK_ENCRYPTION */
#ifdef CK_ENVIRONMENT
    "environment",     CK_TN_ENV,  0,
#endif /* CK_ENVIRONMENT */
#ifdef IKS_OPTION
    "kermit",          CK_TN_IKS,  CM_INV,
#endif /* IKS_OPTION */
#ifdef CK_SNDLOC
    "location",        CK_TN_LOC,  0,
#endif /* CK_SNDLOC */
#ifdef CK_NAWS
    "naws",            CK_TN_NAWS, CM_INV,
#endif /* CK_NAWS */
    "newline-mode",    CK_TN_NL,   0,
    "no-encrypt-during-xfer", CK_TN_NE, CM_INV,
    "prompt-for-userid",CK_TN_PUID,0,
    "remote-echo",     CK_TN_RE,   0,
#ifdef CK_SSL
    "start-tls",       CK_TN_TLS,  CM_INV,
#endif /* CK_SSL */
    "terminal-type",   CK_TN_TT,   0,
    "wait-for-negotiations", CK_TN_WAIT, 0,
#ifdef CK_ENVIRONMENT
    "xdisplay-location",CK_TN_XD, CM_INV,
#endif /* CK_ENVIRONMENT */
    "", 0, 0
};
int ntn = (sizeof(tntab) / sizeof(struct keytab)) - 1;

struct keytab tnopttab[] = {
#ifdef CK_AUTHENTICATION
    "authentication",  CK_TN_AU,   0,
#else
    "authentication",  CK_TN_AU,   CM_INV,
#endif /* CK_AUTHENTICATION */
    "binary-mode",     CK_TN_BM,   0,
    "com-port-control",CK_TN_CPC,  CM_INV,
    "echo",            CK_TN_EC,   0,
#ifdef CK_ENCRYPTION
    "encryption",      CK_TN_ENC,  0,
#else
    "encryption",      CK_TN_ENC,  CM_INV,
#endif /* CK_ENCRYPTION */
#ifdef CK_FORWARD_X
    "forward_x",       CK_TN_FX,   0,
#else /* CK_FORWARD_X */
    "forward_x",       CK_TN_FX,   CM_INV,
#endif /* CK_FORWARD_X */
    "ibm-sak",         CK_TN_SAK,  CM_INV,
#ifdef IKS_OPTION
    "kermit",          CK_TN_IKS,  0,
#else
    "kermit",          CK_TN_IKS,  CM_INV,
#endif /* IKS_OPTION */
    "lflow",           CK_TN_FLW,  CM_INV,
#ifdef CK_NAWS
    "naws",            CK_TN_NAWS, 0,
#else
    "naws",            CK_TN_NAWS, CM_INV,
#endif /* CK_NAWS */
#ifdef CK_ENVIRONMENT
    "new-environment", CK_TN_ENV,  0,
#else
    "new-environment", CK_TN_ENV,  CM_INV,
#endif /* CK_ENVIRONMENT */
    "pragma-heartbeat",CK_TN_PHR,  CM_INV,
    "pragma-logon",    CK_TN_PLG,  CM_INV,
    "pragma-sspi",     CK_TN_PSP,  CM_INV,
    "sak",             CK_TN_SAK,  CM_INV,
#ifdef CK_SNDLOC
    "send-location",   CK_TN_LOC,  0,
#else
    "send-location",   CK_TN_LOC,  CM_INV,
#endif /* CK_SNDLOC */
    "sga",             CK_TN_SGA, CM_INV|CM_ABR,
#ifdef CK_SSL
    "start-tls",       CK_TN_TLS,  0,
#else
    "start-tls",       CK_TN_TLS,  CM_INV,
#endif /* CK_SSL */
    "suppress-go-aheads", CK_TN_SGA, 0,
    "terminal-type",   CK_TN_TT,   0,
    "ttype",           CK_TN_TT,   CM_INV|CM_ABR,
#ifdef CK_ENVIRONMENT
    "xdisplay-location", CK_TN_XD, 0,
#else
    "xdisplay-location", CK_TN_XD, CM_INV,
#endif /* CK_ENVIRONMENT */
    "", 0, 0
};
int ntnopt = (sizeof(tnopttab) / sizeof(struct keytab)) - 1;

struct keytab tnoptsw[] = {
    "/client",  CK_TN_CLIENT,   0,
    "/server",  CK_TN_SERVER,   0
};
int ntnoptsw = (sizeof(tnoptsw) / sizeof(struct keytab));
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

#ifndef NOLOGDIAL
"cx-log",               0, 0,
#else
"cx-log",               1, 0,
#endif /* NOLOGDIAL */

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

#ifndef NOXFER
"file-transfer",        0, 0,
#else
"file-transfer",        1, 0,
#endif /* NOXFER */

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
#ifdef GREEK
"greek",                0, 0,
#else
"greek",                1, 0,
#endif /* GREEK */
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

#ifndef NOIKSD
"iksd",                 0, 0,
#else
"iksd",                 1, 0,
#endif /* NOIKSD */

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

#ifndef NOXFER
"kermit",               0, 0,
#else
"kermit",               1, 0,
#endif /* NOXFER */

#ifdef CK_KERBEROS
"kerberos",		0, 0,
#else
"kerberos",		1, 0,
#endif /* CK_KERBEROS */

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

#ifndef NOLOCAL
"making-connections",   0, 0,
#else
"making-connections",   1, 0,
#endif /* NOLOCAL */

#ifdef NETCONN
"network",		0, 0,
#else
"network",		1, 0,
#endif /* NETCONN */

#ifdef NT
#ifdef CK_AUTHENTICATION
"ntlm",                 1, 0,
#else /* CK_AUTHENTICATION */
"ntlm",                 0, 0,
#endif /* CK_AUTHENTICATION */
#else /* NT */
"ntlm",                 0, 0,
#endif /* NT */

#ifdef PIPESEND
"pipes",                0, 0,
#else
#ifdef NETCMD
"pipes",                0, 0,
#endif /* NETCMD */
#endif /* PIPESEND */
#ifndef PIPESEND
#ifndef NETCMD
"pipes",                1, 0,
#endif /* PIPESEND */
#endif /* NETCMD */

#ifdef NETPTY
"pty",                  0, 0,
#else
"pty",                  1, 0,
#endif /* NETPTY */

#ifndef NOPUSH
"push",			0, 0,
#else
"push",			1, 0,
#endif /* PUSH */

#ifdef CK_REDIR
"redirect",             0, 0,
#else
"redirect",             1, 0,
#endif /* CK_REDIR */

#ifdef CK_RTSCTS
"rts/cts",		0, 0,
#else
"rts/cts",		1, 0,
#endif /* RTS/CTS */

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

#ifdef CK_SRP
"srp",		        0, 0,
#else
"srp",		        1, 0,
#endif /* CK_SRP */

#ifdef CK_SSL
"ssl/tls",		0, 0,
#else
"ssl/tls",		1, 0,
#endif /* CK_SSL */

#ifndef NOXMIT
"transmit",		0, 0,
#else
"transmit",		1, 0,
#endif /* NOXMIT */

#ifdef UNICODE
"unicode",              0, 0,
#else
"unicode",              1, 0,
#endif /* UNICODE */

#ifdef CK_XYZ
"xyzmodem",		0, 0,
#else
"xyzmodem",		1, 0,
#endif /* NOXMIT */

"", 0, 0
};
int nftr = (sizeof(ftrtab) / sizeof(struct keytab)) - 1;

struct keytab desttab[] = {		/* SET DESTINATION */
#ifdef CALIBRATE
    "calibrate", DEST_N, CM_INV,
#endif /* CALIBRATE */
    "disk",    DEST_D, 0,
#ifdef CALIBRATE
    "nowhere", DEST_N, 0,
#endif /* CALIBRATE */
    "printer", DEST_P, 0,
    "screen",  DEST_S, 0
};
int ndests =  (sizeof(desttab) / sizeof(struct keytab));

#ifndef NOSPL		/* Used only with script programming items... */

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
    int i, x, y, dd;			/* Workers */
    int nxdir;
    char *s;
    char ** xdir;
    char *pp[MAXGETPATH];		/* Temporary name pointers */
#ifdef ZFNQFP
    struct zfnfp * fnp;
#ifdef OS2
    char * env;
    char dirpath[4096];
#else /* OS2 */
    char dirpath[1024];			/* For fully qualified filenames */
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
	    int len;
#ifdef NT
	    env = getenv("K95PHONES");
#else /* NT */
	    env = getenv("K2PHONES");
#endif /* NT */
	    if (!env)
	      env = getenv("K95PHONES");
            if (!env)
	      env = "";

            dirpath[0] = '\0';
            len = strlen(env) + 2*strlen(startupdir) + 2*strlen(inidir)
                + 2*strlen(zhome()) + 2*strlen(exedir) + 4*strlen("PHONES/")
                + 12;
            if (len < 4096)
	      sprintf(dirpath,"%s%s%s;%s%s;%s;%s%s;%s;%s%s",
		    /* Semicolon-separated path list */
		    env,
		    (env[0] && env[strlen(env)-1] == ';') ? "" : ";",
		    startupdir,
		    startupdir, "PHONES/",
		    inidir,
		    inidir, "PHONES/",
                    zhome(),
                    zhome(), "PHONES/",
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
	} else {			/* List of directory names */
	    x = 0;
	    y = cmdir("Directory name","",&s,xxstring);
	}
	if (y < 0) {
	    if (y == -3) {		/* EOL or user typed <CR> */
		if ((y = cmcfm()) < 0) return(y);
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
#ifndef NOXFER
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
#endif /* NOXFER */
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
#endif /* NOSPL */

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
static int
setdcd() {
    int x, y, z = 0;
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

static struct keytab qyesno[] = {	/* Yes/No/Quit keyword table */
    "no",    0, 0,
    "ok",    1, 0,
    "quit",  2, 0,
    "yes",   1, 0
};
static int nqyesno = (sizeof(qyesno) / sizeof(struct keytab));

/* Ask question, get yes/no answer */

int
getyesno(msg, quit) char * msg; int quit; {
#ifdef CK_RECALL
    int sv_recall;			/* For turning off command recall */
    extern int on_recall;		/* around Password prompting */
#endif /* CK_RECALL */
    int y, z;

#ifdef OS2
    extern int vmode;
    int vmode_sav = vmode;

#ifdef COMMENT
    if (win95_popup
#ifdef IKSD
	&& !inserver
#endif /* IKSD */
	)
      return(popup_readyesno(vmode,prmpt,quit));
#endif /* COMMENT */
    if (vmode == VTERM) {
        vmode = VCMD;
        VscrnIsDirty(VTERM);
        VscrnIsDirty(VCMD);
    }
#endif /* OS2 */
#ifdef VMS
/*
  In VMS, whenever a TAKE file or macro is active, we restore the
  original console modes so Ctrl-C/Ctrl-Y can work.  But here we
  go interactive again, so we have to temporarily put them back.
*/
    if (!cmdsrc())
      concb((char)escape);
#endif /* VMS */

#ifdef CK_RECALL
    sv_recall = on_recall;		/* Save and turn off command recall */
    on_recall = 0;
#endif /* CK_RECALL */
    cmsavp(psave,PROMPTL);		/* Save old prompt */
    cmsetp(msg);			/* Make new prompt */
    z = 0;				/* Initialize answer to No. */
    cmini(ckxech);			/* Initialize parser. */
    do {
	prompt(NULL);			/* Issue prompt. */
	if (quit)
	  y = cmkey(qyesno,nqyesno,"","",NULL); /* Get Yes or No */
	else
	  y = cmkey(yesno,nyesno,"","",NULL); /* Get Yes or No */
	if (y < 0) {
	    if (y == -4) {		/* EOF */
		z = y;
		break;
	    } else if (y == -3)		/* No answer? */
	      printf(" Please respond Yes or No%s\n", quit ? " or Quit" : "");
	    cmini(ckxech);
	} else {
	    z = y;			/* Save answer */
	    y = cmcfm();		/* Get confirmation */
	}
    } while (y < 0);			/* Continue till done */
    cmsetp(psave);			/* Restore real prompt */
#ifdef CK_RECALL
    on_recall = sv_recall;		/* Restore command recall */
#endif /* CK_RECALL */
#ifdef VMS
    if (cmdlvl > 0)			/* In VMS and not at top level, */
      conres();				/*  restore console again. */
#endif /* VMS */
#ifdef OS2
    if (vmode != vmode_sav) {
        vmode = VTERM;
        VscrnIsDirty(VCMD);
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
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
    if (!ckstrcmp(line,"push",(int)strlen(line),0)) {
	if (msgflg)			/* If at top level... */
	  printf(" push%s available\n", nopush ? " not" : "");
	else if (nopush && !backgrd)
	  printf(" CHECK: push not available\n");
	return(success = 1 - nopush);
    }
#endif /* NOPUSH */
#ifdef PIPESEND
    if (!ckstrcmp(line,"pipes",(int)strlen(line),0)) {
	if (msgflg)			/* If at top level... */
	  printf(" pipes%s available\n",
		 (nopush || protocol != PROTO_K) ? " not" : "");
	else if ((nopush || protocol != PROTO_K) && !backgrd)
	  printf(" CHECK: pipes not available\n");
	return(success = 1 - nopush);
    }
#endif /* PIPESEND */
    y = lookup(ftrtab,line,nftr,&x);	/* Look it up */
    debug(F111,"dochk",ftrtab[x].kwd,y);
    if (msgflg)				/* If at top level... */
      printf(" %s%s available\n", ftrtab[x].kwd, y ? " not" : "");
    else if (y && !backgrd)
      printf(" CHECK: %s not available\n", ftrtab[x].kwd);
    return(success = 1 - y);
}

#ifndef NOLOCAL
#ifdef CKLOGDIAL

/* Connection log and elapsed-time reporting */

extern char cxlogbuf[];			/* Log record buffer */
extern char diafil[];			/* Log file name */
extern int dialog, cx_active;		/* Flags */
static int cx_prev = 0L;		/* Elapsed time of previous session */

VOID
dologend() {				/* Write record to connection log */
    long d1, d2, t1, t2;
    char buf[32], * p;

    debug(F101,"dologend dialog","",dialog);
    debug(F101,"dologend cxlogbuf[0]","",cxlogbuf[0]);
#ifdef CKSYSLOG
    debug(F101,"dologend ckxlogging","",ckxlogging);
#endif /* CKSYSLOG */

    if (!cx_active || !cxlogbuf[0])	/* No active record */
      return;

    cx_active = 0;			/* Record is not active */
    debug(F110,"dologend cxlogbuf 1",cxlogbuf,0);

    d1 = mjd((char *)cxlogbuf);		/* Get start date of this session */
    ckstrncpy(buf,ckdate(),31);		/* Get current date */
    d2 = mjd(buf);			/* Convert them to mjds */
    p = cxlogbuf;			/* Get start time */
    p[11] = NUL;
    p[14] = NUL;			/* Convert to seconds */
    t1 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    p[11] = ':';
    p[14] = ':';
    p = buf;				/* Get end time */
    p[11] = NUL;
    p[14] = NUL;
    t2 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    t2 = ((d2 - d1) * 86400L) + (t2 - t1); /* Compute elapsed time */
    debug(F101,"dologend t2","",t2);
    if (t2 > -1L) {
	cx_prev = t2;
	p = hhmmss(t2);
	debug(F110,"dologend hhmmss",p,0);
	strncat(cxlogbuf,"E=",CXLOGBUFL); /* Append to log record */
	strncat(cxlogbuf,p,CXLOGBUFL);
	debug(F110,"dologend cxlogbuf 2",cxlogbuf,0);
    } else
      cx_prev = 0L;
    debug(F101,"dologend cx_prev","",cx_prev);
    if (dialog) {			/* If logging */
	int x;
	x = diaopn(diafil,1,1);		/* Open log in append mode */
	debug(F101,"dologend diaopn","",x);
	x = zsoutl(ZDIFIL,cxlogbuf);	/* Write the record */
	debug(F101,"dologend zsoutl","",x);
	x = zclose(ZDIFIL);		/* Close the log */
	debug(F101,"dologend zclose","",x);
    }
#ifdef CKSYSLOG
    debug(F101,"dologend ckxlogging","",ckxlogging);
    if (ckxlogging) {
	int x;
	x = ckindex("T=DIAL",cxlogbuf,0,0,1);
	debug(F111,"dologend ckxsyslog",cxlogbuf,ckxsyslog);
	debug(F111,"dologend ckindex","T=DIAL",x);
        if (x > 0) {
	    if (ckxsyslog >= SYSLG_DI) {
		debug(F110,"dologend syslog",cxlogbuf+18,0);
		cksyslog(SYSLG_DI,1,"CONNECTION",(char *)(cxlogbuf+18),"");
	    } else if (ckxsyslog >= SYSLG_AC) {
		debug(F110,"dologend syslog",cxlogbuf+18,0);
		cksyslog(SYSLG_AC,1,"CONNECTION",(char *)(cxlogbuf+18),"");
	    }
        }
    }
#endif /* CKSYSLOG */
}

/*  D O L O G S H O W  --  Show session/connection info  */

/* Call with fc == 1 to show, fc == 0 to only calculate. */
/* Returns session elapsed time in seconds. */
/* If no session active, returns elapsed time of previous session, if any, */
/* otherwise 0 */

long
dologshow(fc) int fc; {			/* SHOW (current) CONNECTION */
    long d1, d2, t1, t2 = 0;
    char c, buf1[32], buf2[32], * info[32], * p, * s;
    char xbuf[CXLOGBUFL+1];
    int i, x = 0, z;

    if (!cxlogbuf[0]) {
	if (fc) {
	    if (didsetlin)
	      printf(" No record.\n");
	    else
	      printf(" No connection.\n");
	}
	return(cx_prev);
    }
    debug(F101,"dologshow local","",local);
    debug(F101,"dologshow cx_active","",cx_active);

    if (local)
      z = ttchk();			/* See if we have an open connection */
    else
      z = cx_active ? 1 : -2;
    if (z < 0L) {
	if (!fc)
	  return(cx_prev);
	else
	  t2 = cx_prev;
    }
    debug(F101,"dologshow ttchk","",z);
    debug(F101,"dologshow cx_prev","",cx_prev);

    ckstrncpy(buf1,cxlogbuf,17);	/* Copy of just the timestamp */
    buf1[17] = NUL;			/* Terminate it */
    ckstrncpy(xbuf,cxlogbuf+18,CXLOGBUFL); /* Copy that can be poked */
    xwords(xbuf,31,info,1);		/* Break up into fields */
    d1 = mjd(buf1);			/* Convert start time to MJD */
    ckstrncpy(buf2,ckdate(),31);	/* Current date */
    d2 = mjd(buf2);			/* Convert to MJD */
    p = buf1;				/* Point to start time */
    p[11] = NUL;
    p[14] = NUL;			/* Convert to seconds */
    t1 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    p[11] = ':';
    p[14] = ':';
    p = buf2;				/* Ditto for current time */
    p[11] = NUL;
    p[14] = NUL;
    if (z > -1L) {
	t2 = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
	t2 = ((d2 - d1) * 86400L) + (t2 - t1); /* Elapsed time so far */
    }
    if (fc) {
	p = NULL;
	if (t2 > -1L)			/* Convert seconds to hh:mm:ss */
	  p = hhmmss(t2);
	if (z > -1)
	  s = "Active";
	else if (z == -2)
	  s = "Closed";
	else
	  s = "Unknown";
	printf("\n");			/* Show results */
	printf(" Status:       %s\n",s);
	printf(" Opened:       %s\n",buf1);
	printf(" User:         %s\n",info[1] ? info[1] : "");
	printf(" PID:          %s\n",info[2] ? info[2] : "");
	for (i = 3; info[i]; i++) {
	    c = info[i][0];
	    s = (info[i]) ? info[i]+2 : "";
	    switch (c) {
	      case 'T': printf(" Type:         %s\n", s); break;
	      case 'N': printf(" To:           %s\n", s); break;
	      case 'H': printf(" From:         %s\n", s); break;
	      case 'D': printf(" Device:       %s\n", s); break;
	      case 'O': printf(" Origin:       %s\n", s); break;
	      case 'E': break;
	      default:  printf(" %s\n",info[i] ? info[i] : "");
	    }
	}
	if (z < 0L)
	  printf(" Elapsed time: %s\n", hhmmss(t2));
	else
	  printf(" Elapsed time: %s\n", p ? p : "(unknown)");
	x = 0;
#ifdef NETCONN
#ifdef CK_ENCRYPTION
	if (ck_tn_encrypting() && ck_tn_decrypting()) x++;
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
	if (tls_active_flag || ssl_active_flag) x++;
#endif /* CK_SSL */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
#ifdef CK_ENCRYPTION
	if (ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN) x++;
#endif /* CK_ENCRYPTION */
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
#endif /* NETCONN */
	printf(" Encrypted:    %s\n", x ? "Yes" : "No");
	printf(" Log:          %s\n", dialog ? diafil : "(none)");
	printf("\n");
    }
    return(t2 > -1L ? t2 : 0L);
}

VOID
dologline() {
    char * p;
    int n, m = 0;

    dologend();				/* Previous session not closed out? */
    cx_active = 1;			/* Record is active */
    cx_prev = 0L;
    p = ckdate();			/* Get timestamp */
    n = ckstrncpy(cxlogbuf,p,CXLOGBUFL-1); /* Start record off with it */
    m = strlen(uidbuf) + strlen(myhost) + strlen(ttname) + 32;
    if (n+m < CXLOGBUFL-1) {		/* Add serial device info */
	p = cxlogbuf+n;
	sprintf(p," %s %s T=SERIAL H=%s D=%s ",
		uidbuf,
		ckgetpid(),
		myhost,
		ttname
		);
    } else
      strcpy(cxlogbuf,"LOGLINE BUFFER OVERFLOW");
    debug(F110,"dologline",cxlogbuf,0);
}

#ifdef NETCONN
VOID
dolognet() {
    char * p, * s = "NET";
    int n, m;

    dologend();				/* Previous session not closed out? */
    cx_prev = 0L;
    cx_active = 1;			/* Record is active */
    p = ckdate();
    n = ckstrncpy(cxlogbuf,p,CXLOGBUFL);
#ifdef TCPSOCKET
    if (nettype == NET_TCPB || nettype == NET_TCPA)
      s = "TCP";
#endif /* TCPSOCKET */
#ifdef ANYX25
    if (nettype == NET_SX25 || nettype == NET_VX25 || nettype == NET_IX25)
      s = "X25";
#endif /* ANYX25 */
#ifdef DECNET
    if (nettype == NET_DEC)
      s = "DECNET";
#endif /* DECNET */
#ifdef SUPERLAT
    if (nettype == NET_SLAT)
      s = "SUPERLAT";
#endif /* SUPERLAT */
#ifdef CK_NETBIOS
    if (nettype == NET_BIOS)
      s = "NETBIOS";
#endif /* CK_NETBIOS */

    m = strlen(uidbuf) + strlen(myhost) + strlen(ttname) + strlen(s) + 32;
    if (n+m < CXLOGBUFL-1) {
	p = cxlogbuf+n;
	sprintf(p," %s %s T=%s N=%s H=%s ",
		uidbuf,
		ckgetpid(),
		s,
		ttname,
		myhost
		);
    } else
      strcpy(cxlogbuf,"LOGNET BUFFER OVERFLOW");
    debug(F110,"dolognet cxlogbuf",cxlogbuf,0);
}
#endif /* NETCONN */
#endif /* CKLOGDIAL */

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
    debug(F110,"dialstr",s,0);
    return(success = 1);
}

VOID
initmdm(x) int x; {
    MDMINF * p;
    int m;

    mdmtyp = x;				/* Set global modem type */
    debug(F101,"initmdm mdmtyp","",mdmtyp);
    debug(F101,"initmdm usermdm","",usermdm);
    if (x < 1) return;

    m = usermdm ? usermdm : mdmtyp;

    p = modemp[m];			/* Point to modem info struct, and */
    debug(F101,"initmdm p","",p);
    if (p) {
	dialec = p->capas & CKD_EC;	/* set DIAL ERROR-CORRECTION, */
	dialdc = p->capas & CKD_DC;	/* DIAL DATA-COMPRESSION, and */
	mdmspd = p->capas & CKD_SB ? 0 : 1; /* DIAL SPEED-MATCHING from it. */
	dialfc = FLO_AUTO;		    /* Modem's local flow control.. */
	dialmax   = p->max_speed;
	dialcapas = p->capas;
	dialesc   = p->esc_char;
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
    struct FDB k1, k2;

    cmfdbi(&k1,_CMKEY,
	   "Modem parameter","","",nsetmdm, 0, xxstring, setmdm, &k2);
    cmfdbi(&k2,_CMKEY,"","","",nmdm,0,xxstring,mdmtab,NULL);
    x = cmfdb(&k1);
    if (x < 0) {			/* Error */
	if (x == -2 || x == -9)
	  printf("?No keywords match: \"%s\"\n",atmbuf);
	return(x);
    }
    y = cmresult.nresult;		/* Keyword value */
    if (cmresult.fdbaddr == &k2) {	/* Modem-type keyword table */
	if ((x = cmcfm()) < 0)
	  return(x);
	usermdm = 0;
	initmdm(cmresult.nresult);	/* Set the modem type. */
	return(success = 1);		/* Done */
    }
    switch (cmresult.nresult) {		/* SET MODEM keyword table. */
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
	if ((x = cmcfm()) < 0)
	  return(x);
	dialcapas = zz;
	debug(F101,"setmodem autoflow","",autoflow);
	debug(F101,"setmodem flow 1","",flow);
	if (autoflow)			/* Maybe change flow control */
	  setflow();
	debug(F101,"setmodem flow 2","",flow);
	mdmspd = zz & CKD_SB ? 0 : 1;	/* Set MODEM SPEED-MATCHING from it. */
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

      case XYDMCD:			/* SET MODEM CARRIER-WATCH */
	return(setdcd());

      case XYDSPK:			/* SET MODEM SPEAKER */
	return(seton(&mdmspk));

      case XYDVOL:			/* SET MODEM VOLUME */
	if ((x = cmkey(voltab,3,"","medium",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	mdmvol = x;
	return(success = 1);

      default:
	printf("Unexpected SET MODEM parameter\n");
	return(-9);
    }
}

static int				/* Set DIAL command options */
setdial(y) int y; {
    int x = 0, z = 0;
    char *s = NULL;

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
#ifndef NOXFER
      case XYDKSP:			/* DIAL KERMIT-SPOOF */
	return(seton(&dialksp));
#endif /* NOXFER */
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
	y = setnum(&dialesc,x,y,128);
	if (y > -1 && dialesc < 0)	/* No escape character */
	  dialmhu = 0;			/* So no hangup by modem command */
	return(y);
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

#ifndef NOSPL
      case XYDDIR:			/* DIAL DIRECTORY (zero or more) */
	return(parsdir(0));		/* 0 means DIAL */
#endif /* NOSPL */

      case XYDSTR:			/* DIAL STRING */
	if ((y = cmkey(mdmcmd,nmdmcmd,"","",xxstring)) < 0) return(y);
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
	  case XYDS_MS:			/*    dial mode string */
	    return(dialstr(&dialmstr,
                         "Command to enter dial mode"));
	  case XYDS_MP:			/*    dial mode prompt */
	    return(dialstr(&dialmprmt,
			   "Modem response upon entering dial mode"));
	  case XYDS_SP:			/* SPEAKER OFF */
	    if ((x = cmkey(onoff,2,"","on",xxstring)) < 0) return(x);
	    if (x)
	      return(dialstr(&dialspon,"Command to turn modem speaker on"));
	    else
	      return(dialstr(&dialspoff,"Command to turn modem speaker off"));

	  case XYDS_VO:			/* VOLUME LOW */
	    if ((x = cmkey(voltab,3,"","medium",xxstring)) < 0) return(x);
	    switch (x) {
	      case 0:
	      case 1:
		return(dialstr(&dialvol1,
			       "Command for low modem speaker volume"));
	      case 2:
		return(dialstr(&dialvol2,
			   "Command for medium modem speaker volume"));

	      case 3:
		return(dialstr(&dialvol3,
			       "Command for high modem speaker volume"));
	      default:
		return(-2);
	    }

	  case XYDS_ID:			/* IGNORE-DIALTONE */
	    return(dialstr(&dialx3,
			   "Command to tell modem to ignore dialtone"));

	  case XYDS_I2:			/* PREDIAL-INIT */
	    return(dialstr(&dialini2,
			   "Command to send to modem just prior to dialing"));

	  default:
	    printf("?Unexpected SET DIAL STRING parameter\n");
	}

      case XYDFC:			/* DIAL FLOW-CONTROL */
	if ((y = cmkey(dial_fc,4,"","auto",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	dialfc = y;
	return(success = 1);

      case XYDMTH:			/* DIAL METHOD */
	if ((y = cmkey(dial_m,ndial_m,"","default",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0)
	  return(x);
	if (y == XYDM_A) {		/* AUTO */
	    extern int dialmauto;	/* Means choose based on */
	    dialmauto = 1;		/* local country code, if known. */
	    dialmth = XYDM_D;
	} else
	  dialmth = y;
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
			strcpy(dialtfc[0],"800"); /* 1970-something */
			ntollfree++;
			if (dialtfc[1] = malloc(4)) {
			    strcpy(dialtfc[1],"888"); /* 1996 */
			    ntollfree++;
			    if (dialtfc[2] = malloc(4)) {
				strcpy(dialtfc[2],"877"); /* 5 April 1998 */
				ntollfree++;
				if (dialtfc[3] = malloc(4)) {
				    strcpy(dialtfc[3],"866"); /* Soon */
				    ntollfree++;
				}
			    }
			}
		    }
		}
		if (!dialtfp) 		/* Toll-free dialing prefix */
		  makestr(&dialtfp,"1");
#ifdef COMMENT
/* The time for this is past */
	    } else if (!strcmp(diallcc,"358") &&
		       ((int) strcmp(zzndate(),"19961011") > 0)
		       ) {		/* Finland */
		if (!dialldp)		/* Long-distance prefix */
		  makestr(&dialldp,"9");
		if (!dialixp) 		/* International dialing prefix */
		  makestr(&dialixp,"990");
#endif /* COMMENT */
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
	    if (!strcmp(diallcc,"33"))	/* France */
	      dialfld = 1;		/* Long-distance dialing is forced */
	}
	return(success = 1);

      case XYDIXP:			/* DIAL INTL-PREFIX */
	return(dialstr(&dialixp,"International dialing prefix"));

      case XYDIXS:			/* DIAL INTL-SUFFIX */
	return(dialstr(&dialixs,"International dialing suffix"));

      case XYDLDP:			/* DIAL LD-PREFIX */
	return(dialstr(&dialldp,"Long-distance dialing prefix"));

      case XYDLDS:			/* DIAL LD-SUFFIX */
	return(dialstr(&diallds,"Long-distance dialing suffix"));

      case XYDLCP:			/* DIAL LC-PREFIX */
	return(dialstr(&diallcp,"Local dialing prefix"));

      case XYDLCS:			/* DIAL LC-SUFFIX */
	return(dialstr(&diallcs,"Local dialing suffix"));

#ifdef COMMENT
      case XYDPXX:			/* DIAL PBX-EXCHANGE */
	return(dialstr(&dialpxx,"Exchange of PBX you are calling from"));
#endif /* COMMENT */

      case XYDPXI: {			/* DIAL PBX-INTERNAL-PREFIX */
#ifdef COMMENT
	  return(dialstr(&dialpxi,
		       "Internal-call prefix of PBX you are calling from"));
#else
	  int x;
	  if ((x = cmtxt("Internal-call prefix of PBX you are calling from",
			 "",&s,NULL)) < 0) /* Don't evaluate */
	    return(x);
#ifndef NOSPL
	  if (*s) {
	      char c, * p = tmpbuf;
	      if (*s == '\\') {
		  c = *(s+1);
		  if (isupper(c)) c = tolower(c);
		  if (c != 'f' &&
		      ckstrcmp(s,"\\v(d$px)",8,0) &&
		      ckstrcmp(s,"\\v(d$pxx)",9,0) &&
		      ckstrcmp(s,"\\v(d$p)",7,0)) {
		      x = TMPBUFSIZ;
		      zzstring(s,&p,&x);
		      s = tmpbuf;
		  }
	      }
	  }
#endif /* NOSPL */
	  makestr(&dialpxi,s);
	  return(1);
      }
#endif /* COMMENT */

      case XYDPXO:			/* DIAL PBX-OUTSIDE-PREFIX */
	return(dialstr(&dialpxo,
		       "Outside-line prefix of PBX you are calling from"));

      case XYDSFX:			/* DIAL INTL-SUFFIX */
	return(dialstr(&dialsfx," Telephone number suffix for dialing"));

      case XYDSRT:			/* DIAL SORT */
	return(success = seton(&dialsrt));

      case XYDPXX:			/* DIAL PBX-EXCHANGE */
      case XYDTFC: {			/* DIAL TOLL-FREE-AREA-CODE  */
	  int n, i;			/* (zero or more of them...) */
	  char * p[MAXTOLLFREE];	/* Temporary pointers */
	  char * m;
	  for (n = 0; n < MAXTOLLFREE; n++) {
	      if (n == 0) {
		  m = (y == XYDTFC) ?
		  "Toll-free area code(s) in the country you are calling from"
		    : "Exchange(s) of PBX you are calling from";
	      } else {
		  m = (y == XYDTFC) ?
		    "Another toll-free area code"
		      : "Another PBX exchange";
	      }
	      if ((x = cmfld(m,"",&s,xxstring)) < 0)
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
	      int m;
	      m = (y == XYDTFC) ? ntollfree : ndialpxx;
	      if ((x = cmcfm()) < 0)
		return(x);
	      x = 1;
	      for (i = 0; i < m; i++) { /* Remove old list, if any */
		  if  (y == XYDTFC)
		    makestr(&(dialtfc[i]),NULL);
		  else
		    makestr(&(dialpxx[i]),NULL);
	      }
	      if  (y == XYDTFC)
		ntollfree = n;		/* New count */
	      else
		ndialpxx = n;
	      for (i = 0; i < n; i++) { /* New list */
		  if  (y == XYDTFC)
		    makestr(&(dialtfc[i]),p[i]);
		  else
		    makestr(&(dialpxx[i]),p[i]);
	      }
	      x = 1;
	  }
	  for (i = 0; i < n; i++)
	    if (p[i]) free(p[i]);
	  return(x);
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

      case XYDLLAC: {			/* Local area-code list  */
	  int n, i;			/* (zero or more of them...) */
	  char * p[MAXLOCALAC];	/* Temporary pointers */
	  for (n = 0; n < MAXLOCALAC; n++) {
	      if ((x = cmfld(
		    "Area code to which calls from your area are local",
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
	      if ((x = cmcfm()) < 0)
		return(x);
	      for (i = 0; i < nlocalac; i++) /* Remove old list, if any */
		if (diallcac[i]) {
		    free(diallcac[i]);
		    diallcac[i] = NULL;
		}
	      nlocalac = n;		/* New count */
	      for (i = 0; i < nlocalac; i++) /* New list */
		diallcac[i] = p[i];
	      return(success = 1);
	  } else {			/* Parse error, undo everything */
	      for (i = 0; i < n; i++)
		if (p[i]) free(p[i]);
	      return(x);
	  }
      }

      case XYDFLD:
	return(success = seton(&dialfld));

      case XYDIDT:			/* DIAL IGNORE-DIALTONE */
	return(seton(&dialidt));

      case XYDPAC:
	y = cmnum(
	      "Milliseconds to pause between each character sent to dialer",
		  "",10,&x,xxstring);
	return(setnum(&dialpace,x,y,9999));

#ifndef NOSPL
      case XYDMAC:
	if ((x = cmfld("Name of macro to execute just prior to dialing",
		       "",&s,xxstring)) < 0) {
	    if (x == -3)
	      s = NULL;
	    else
	      return(x);
	}
	if (s) {
	    if (!*s) {
		s = NULL;
	    } else {
		strcpy(line,s);
		s = line;
	    }
	}
	if ((x = cmcfm()) < 0)
	  return(x);
	makestr(&dialmac,s);
	return(success = 1);
#endif /* NOSPL */

      case XYDPUCC:			/* Pulse country codes */
      case XYDTOCC: {			/* Tone country codes */
	  int n, i;
	  char * p[MAXTPCC];
	  char * m;
	  for (n = 0; n < MAXTPCC; n++) {
	      if (n == 0) {
		  m = (y == XYDPUCC) ?
		  "Country code where Pulse dialing is required"
		    : "Country code where Tone dialing is available";
	      } else
		m = "Another country code";
	      if ((x = cmfld(m,"",&s,xxstring)) < 0)
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
	      int m;
	      m = (y == XYDPUCC) ? ndialpucc : ndialtocc;
	      if ((x = cmcfm()) < 0)
		return(x);
	      x = 1;
	      for (i = 0; i < m; i++) { /* Remove old list, if any */
		  if (y == XYDPUCC)
		    makestr(&(dialpucc[i]),NULL);
		  else
		    makestr(&(dialtocc[i]),NULL);
	      }
	      if (y == XYDPUCC) {
		  ndialpucc = n;		/* New count */
	      } else {
		  ndialtocc = n;
	      }
	      for (i = 0; i < n; i++) { /* New list */
		  if (y == XYDPUCC) {
		      makestr(&(dialpucc[i]),p[i]);
		  } else {
		      makestr(&(dialtocc[i]),p[i]);
		  }
	      }
	      x = 1;
	  }
	  for (i = 0; i < n; i++)
	    if (p[i]) free(p[i]);
	  return(x);
      }
      case XYDTEST:
	return(seton(&dialtest));

      default:
	printf("?Unexpected SET DIAL parameter\n");
	return(-9);
    }
}

#ifdef CK_TAPI
int						/* TAPI action commands */
dotapi() {
    int x,y;
    char *s;

    if (!TAPIAvail) {
	printf("\nTAPI is unavailable on this system.\n");
	return(-9);
    }
    if ((y = cmkey(tapitab,ntapitab,"MS TAPI command","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYTAPI_CFG: {			/* TAPI CONFIGURE-LINE */
	  extern struct keytab * tapilinetab;
	  extern struct keytab * _tapilinetab;
	  extern int ntapiline;
	  extern int LineDeviceId;
	  int lineID=LineDeviceId;
	  if (TAPIAvail)
	    cktapiBuildLineTable(&tapilinetab, &_tapilinetab, &ntapiline);
	  if (tapilinetab && _tapilinetab && ntapiline > 0) {
	      int i=0, j = 9999, k = -1;

	      if ( LineDeviceId == -1 ) {
		  /* Find out what the lowest numbered TAPI device is */
		  /* and use it as the default.                       */
		  for (i = 0; i < ntapiline; i++ ) {
		      if (tapilinetab[i].kwval < j) {
			  k = i;
		      }
		  }
	      } else {
		  /* Find the LineDeviceId in the table and use that entry */
		  for (i = 0; i < ntapiline; i++ ) {
		      if (tapilinetab[i].kwval == LineDeviceId) {
			  k = i;
			  break;
		      }
		  }
	      }
	      if (k >= 0)
		s = _tapilinetab[k].kwd;
	      else
		s = "";

	      if ((y = cmkey(_tapilinetab,ntapiline,
			      "TAPI device name",s,xxstring)) < 0)
		return(y);
	      lineID = y;
	  }
	  if ((x = cmcfm()) < 0) return(x);
#ifdef IKSD
          if (inserver) {
              printf("Sorry, command disabled\r\n");
              return(success = 0);
          }
#endif /* ISKD */
	  cktapiConfigureLine(lineID);
	  break;
      }
      case XYTAPI_DIAL:			/* TAPI DIALING-PROPERTIES */
	if ((x = cmcfm()) < 0)
	  return(x);
#ifdef IKSD
	if (inserver) {
	    printf("Sorry, command disabled\r\n");
	    return(success = 0);
	}
#endif /* ISKD */
	cktapiDialingProp();
	break;
    }
    return(success = 1);
}

static int				/* SET TAPI command options */
settapi() {
    int x, y;
    char *s;

    if (!TAPIAvail) {
	printf("\nTAPI is unavailable on this system.\n");
	return(-9);
    }
    if ((y = cmkey(settapitab,nsettapitab,"MS TAPI option","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYTAPI_USE:
	return (success = seton(&tapiusecfg));
      case XYTAPI_LGHT:
	return (success = seton(&tapilights));
      case XYTAPI_PRE:
	return (success = seton(&tapipreterm));
      case XYTAPI_PST:
	return (success = seton(&tapipostterm));
      case XYTAPI_INA:
	y = cmnum("seconds of inactivity before auto-disconnect",
		  "0",10,&x,xxstring);
	return(setnum(&tapiinactivity,x,y,65535));
      case XYTAPI_BNG:
	y = cmnum("seconds to wait for credit card tone",
		  "8",10,&x,xxstring);
	return(setnum(&tapibong,x,y,90));
      case XYTAPI_MAN:
	return (success = seton(&tapimanual));
      case XYTAPI_CON:			/* TAPI CONVERSIONS */
	return (success = setonaut(&tapiconv));
      case XYTAPI_LIN:			/* TAPI LINE */
	x = setlin(XYTAPI_LIN,1,0);
	if (x > -1) didsetlin++;
	return(x);
      case XYTAPI_PASS:	{		/* TAPI PASSTHROUGH */
        /* Passthrough became Modem-dialing which is an antonym */
        success = seton(&tapipass);
        tapipass = !tapipass;
	return (success);
      }
      case XYTAPI_LOC: {		/* TAPI LOCATION */
	  extern char tapiloc[];
	  extern int tapilocid;
	  int i = 0, j = 9999, k = -1;

	  cktapiBuildLocationTable(&tapiloctab, &ntapiloc);
	  if (!tapiloctab || !ntapiloc) {
	      printf("\nNo TAPI Locations are configured for this system\n");
	      return(-9);
	  }
	  if (tapilocid == -1)
	    tapilocid = cktapiGetCurrentLocationID();

	  /* Find the current tapiloc entry */
	  /* and use it as the default. */
	  for (k = 0; k < ntapiloc; k++) {
	      if (tapiloctab[k].kwval == tapilocid)
		break;
	  }
          if (k >= 0 && k < ntapiloc)
	    s = tapiloctab[k].kwd;
	  else
	    s = "";

	  if ((y = cmkey(tapiloctab,ntapiloc, "TAPI location",s,xxstring)) < 0)
	    return(y);

	  if ((x = cmcfm()) < 0)
	    return(x);
#ifdef IKSD
          if (inserver) {
              printf("Sorry, command disabled\r\n");
              return(success = 0);
          }
#endif /* ISKD */
	  cktapiFetchLocationInfoByID( y );
	  CopyTapiLocationInfoToKermitDialCmd();
        }
	break;
    }
    return(success=1);
}
#endif /* CK_TAPI */

#ifndef NOSHOW
int					/* SHOW MODEM */
shomodem() {
    MDMINF * p;
    int x, n;
    char c;
    long zz;

#ifdef IKSD
    if (inserver) {
        printf("Sorry, command disabled\r\n");
        return(success = 0);
    }
#endif /* ISKD */

    shmdmlin();
    printf("\n");

    p = (mdmtyp > 0) ? modemp[mdmtyp] : NULL;
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
	printf(" Modem hangup-method:    %s\n",
	       dialmhu ?
	       "modem-command" :
	       "rs232-signal"
	       );
	printf(" Modem speaker:          %s\n", showoff(mdmspk));
	printf(" Modem volume:           %s\n",
	       (mdmvol == 2) ? "medium" : ((mdmvol <= 1) ? "low" : "high"));
	printf(" Modem kermit-spoof:     %s\n", dialksp ? "on" : "off");
	c = (char) (x = (dialesc ? dialesc : p->esc_char));
	printf(" Modem escape-character: %d", x);
	if (isprint(c))
	  printf(" (= \"%c\")",c);
	printf(
"\n\nMODEM COMMANDs (* = set automatically by SET MODEM TYPE):\n\n");
	debug(F110,"show dialini",dialini,0);
	printf(" %c Init-string:          ", dialini ? ' ' : '*' );
	shods(dialini ? dialini : p->wake_str);
	printf(" %c Dial-mode-string:     ", dialmstr ? ' ' : '*' );
	shods(dialmstr ? dialmstr : p->dmode_str);
	n = local ? 19 : 20;
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Dial-mode-prompt:     ", dialmprmt ? ' ' : '*' );
	shods(dialmprmt ? dialmprmt : p->dmode_prompt);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Dial-command:         ", dialcmd ? ' ' : '*' );
	shods(dialcmd ? dialcmd : p->dial_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Compression on:       ", dialdcon ? ' ' : '*' );
	if (!dialdcon)
	  debug(F110,"dialdcon","(null)",0);
	else
	  debug(F110,"dialdcon",dialdcon,0);
	shods(dialdcon ? dialdcon : p->dc_on_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Compression off:      ", dialdcoff ? ' ' : '*' );
	shods(dialdcoff ? dialdcoff : p->dc_off_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Error-correction on:  ", dialecon ? ' ' : '*' );
	shods(dialecon ? dialecon : p->ec_on_str);
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

	printf(" %c Speaker on:           ", dialspon ? ' ' : '*' );
	shods(dialspon ? dialspon : p->sp_on_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Speaker off:          ", dialspoff ? ' ' : '*' );
	shods(dialspoff ? dialspoff : p->sp_off_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Volume low:           ", dialvol1 ? ' ' : '*' );
	shods(dialvol1 ? dialvol1 : p->vol1_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Volume medium:        ", dialvol2 ? ' ' : '*' );
	shods(dialvol2 ? dialvol2 : p->vol2_str);
	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Volume high:          ", dialvol3 ? ' ' : '*' );
	shods(dialvol3 ? dialvol3 : p->vol3_str);
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

	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Ignore-dialtone:      ", dialx3 ? ' ' : '*');
	shods(dialx3 ? dialx3 : p->ignoredt);

	if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
	printf(" %c Predial-init:         ", dialini2 ? ' ' : '*');
	shods(dialini2 ? dialini2 : p->ini2);

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
#endif /* NOLOCAL */

#ifndef NOSPL

#ifdef CK_ANSIC				/* SET ALARM */
int
setalarm(long xx)
#else
int
setalarm(xx) long xx;
#endif /* CK_ANSIC */
/* setalarm */ {
#ifdef COMMENT
    int yyyy, mm, dd, x;
    char *s;
    long zz;
    char buf[6];
#endif /* COMMENT */
    long sec, jd;
    char xbuf[20], * p;

    debug(F101,"setalarm xx","",xx);
    ck_alarm = 0L;			/* 0 = no alarm (in case of error) */
    if (xx < 0L) {
	printf("%ld - illegal value, must be 0 or positive\n", xx);
	return(-9);
    }
    if (xx == 0L) {			/* CLEAR ALARM */
	alrm_date[0] = NUL;
	alrm_time[0] = NUL;
	return(1);
    }
#ifdef COMMENT
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
    sprintf(buf,"%05ld",atol(alrm_time));
    strcpy(alrm_time,buf);
    debug(F110,"SET ALARM date (1)",alrm_date,0);
    debug(F110,"SET ALARM time (1)",alrm_time,0);

    if ((zz = atol(alrm_time) + xx) < 0L) {
	printf("Internal time conversion error, sorry.\n");
	return(-9);
    }
    if (zz >= 86400L) {			/* Alarm crosses midnight */
	char d[10];			/* Local date buffer */
	int lastday;			/* Last day of this month */

	ckstrncpy(d,alrm_date,8);	/* We'll have to change the date */

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
#else
    /* Jul 1998 */
    ckstrncpy(xbuf,ckcvtdate("",1),20);	/* Get current date and time */
    p = xbuf;
    ckstrncpy(alrm_date,xbuf,10);
    alrm_date[8] = NUL;
    sec = atol(p+9) * 3600L + atol(p+12) * 60L + atol(p+15);
    debug(F110,"SET ALARM date (1)",alrm_date,0);
    debug(F101,"SET ALARM time (1)","",sec);
    if ((sec += xx) < 0L) {
	printf("Internal time conversion error, sorry.\n");
	return(-9);
    }
    if (sec >= 86400L) {		/* Alarm crosses midnight */
	long days;
	days = sec / 86400L;
	jd = mjd(p) + days;		/* Get modified Julian date */
	ckstrncpy(alrm_date,mjd2date(jd),10);
	sec %= 86400L;
    }
    sprintf(alrm_time,"%05ld",sec);
    debug(F110,"SET ALARM date (2)",alrm_date,0);
    debug(F110,"SET ALARM time (2)",alrm_time,0);
    ck_alarm = 1;			/* Alarm is set */

#endif /* COMMENT */
    return(1);
}
#endif /* NOSPL */

#ifndef NOSETKEY
int
dosetkey() {				/* SET KEY */
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
	    if (ckstrcmp(atmbuf,"clear",y,0))
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
#ifdef OS2
	    shokeycode(kc,-1);		/* Show current definition */
#else
	    shokeycode(kc);		/* Show current definition */
#endif /* OS2 */
	    flag = 1;			/* Remember it's a multiline command */
	} else				/* Error */
	  return(y);
    }

    /* Normal SET KEY <scancode> <value> command... */

#ifdef OS2
    if (mskkeys)
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
	cmflgs = 0;
	prompt(NULL);
    }
  def_again:
    if (flag)
      cmres();
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

#ifdef HWPARITY
struct keytab stoptbl[] = {
    "1", 1, 0,
    "2", 2, 0
};
#endif /* HWPARITY */

static struct keytab sertbl[] = {
    "7E1", 0, 0,
#ifdef HWPARITY
    "7E2", 1, 0,
#endif /* HWPARITY */
    "7M1", 2, 0,
#ifdef HWPARITY
    "7M2", 3, 0,
#endif /* HWPARITY */
    "7O1", 4, 0,
#ifdef HWPARITY
    "7O2", 5, 0,
#endif /* HWPARITY */
    "7S1", 6, 0,
#ifdef HWPARITY
    "7S2", 7, 0,
    "8E1", 9, 0,
    "8E2", 10, 0,
#endif /* HWPARITY */
#ifdef HWPARITY
    "8N1", 8, 0,
#endif /* HWPARITY */
#ifdef HWPARITY
    "8N2", 11, 0,
    "8O1", 12, 0,
    "8O2", 13, 0,
#endif /* HWPARITY */
    "", 0, 0
};
static int nsertbl = (sizeof(sertbl) / sizeof(struct keytab)) - 1;

static char * sernam[] = {		/* Keep this in sync with sertbl[] */
  "7E1", "7E2", "7M1", "7M2", "7O1", "7O2", "7S1", "7S2",
  "8N1", "8E1", "8E2", "8N2", "8O1", "8O2"
};

static struct keytab optstab[] = {	/* SET OPTIONS table */
#ifndef NOFRILLS
    "delete",    XXDEL,   0,            /* DELETE */
#endif /* NOFRILLS */
    "directory", XXDIR,   0,		/* DIRECTORY */
#ifdef CKPURGE
    "purge",     XXPURGE, 0,		/* PURGE */
#endif /* CKPURGE */
    "type",      XXTYP,   0,		/* TYPE */
    "", 0, 0
};
static int noptstab =  (sizeof(optstab) / sizeof(struct keytab)) - 1;

#ifndef NOXFER
/*
  PROTOCOL SELECTION.  Kermit is always available.  If CK_XYZ is defined at
  compile time, then the others become selections also.  In OS/2 and
  Windows, they are integrated and the various SET commands (e.g. "set file
  type") affect them as they would Kermit.  In other OS's (UNIX, VMS, etc),
  they are external protocols which are run via Kermit's REDIRECT mechanism.
  All we do is collect and verify the filenames and pass them along to the
  external protocol.
*/
struct keytab protos[] = {
#ifdef CK_XYZ
    "g",          PROTO_G,  CM_INV,
#endif /* CK_XYZ */
    "kermit",     PROTO_K,  0,
#ifdef CK_XYZ
    "other",      PROTO_O,  0,
    "x",          PROTO_X,  CM_INV|CM_ABR,
    "xmodem",     PROTO_X,  0,
    "xmodem-crc", PROTO_XC, 0,
    "y",          PROTO_Y,  CM_INV|CM_ABR,
    "ymodem",     PROTO_Y,  0,
    "ymodem-g",   PROTO_G,  0,
    "zmodem",     PROTO_Z,  0
#endif /* CK_XYZ */
};
int nprotos =  (sizeof(protos) / sizeof(struct keytab));

#define XPCMDLEN 71

_PROTOTYP(static int protofield, (char *, char *, char *));
_PROTOTYP(static int setproto, (void));

static int
protofield(current, help, px) char * current, * help, * px; {

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
    /* char * s = NULL; */
    int x = 0, y;
    char s1[XPCMDLEN+1], s2[XPCMDLEN+1], s3[XPCMDLEN+1];
    char s4[XPCMDLEN+1], s5[XPCMDLEN+1], s6[XPCMDLEN+1], s7[XPCMDLEN+1];
    char * p1 = s1, * p2 = s2, *p3 = s3;
    char * p4 = s4, * p5 = s5, *p6 = s6, *p7 = s7;

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

    if (y == PROTO_K) {
	if ((x = protofield(ptab[y].h_x_init,
		    "Optional command to send to host to start Kermit server",
			    p3)) < 0) {
	    if (x == -3)
	      goto protoexit;
	    else
	      return(x);
	}
    }


#ifndef XYZ_INTERNAL			/* If XYZMODEM are external... */

    if (y != PROTO_K) {
	if ((x = protofield(ptab[y].p_b_scmd,
  	         "External command to SEND in BINARY mode with this protocol",
			    p4)) < 0) {
	    if (x == -3)
	      goto protoexit;
	    else
	      return(x);
	}
	if ((x = protofield(ptab[y].p_t_scmd,
		 "External command to SEND in TEXT mode with this protocol",
			    p5)) < 0) {
	    if (x == -3)
	      goto protoexit;
	    else
	      return(x);
	}
	if ((x = protofield(ptab[y].p_b_rcmd,
	       "External command to RECEIVE in BINARY mode with this protocol",
			    p6)) < 0) {
	    if (x == -3)
	      goto protoexit;
	    else
	      return(x);
	}
	if ((x = protofield(ptab[y].p_t_rcmd,
		 "External command to RECEIVE in TEXT mode with this protocol",
			    p7)) < 0) {
	    if (x == -3)
	      goto protoexit;
	    else
	      return(x);
	}
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

    p1 = brstrip(p1);
    p2 = brstrip(p2);
    p3 = brstrip(p3);
    p4 = brstrip(p4);
    p5 = brstrip(p5);
    p6 = brstrip(p6);
    p7 = brstrip(p7);
    initproto(y,p1,p2,p3,p4,p5,p6,p7);
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
#endif /* NOXFER */

#ifdef DECNET
struct keytab dnettab[] = {
#ifndef OS2ONLY
    "cterm", NP_CTERM, 0,
#endif /* OS2ONLY */
    "lat",   NP_LAT,   0
};
int ndnet =  (sizeof(dnettab) / sizeof(struct keytab));
#endif /* DECNET */

/*  S E T P R I N T E R  --  SET PRINTER command  */

#ifdef PRINTSWI
static struct keytab prntab[] = {	/* SET PRINTER switches */
    "/bidirectional",    PRN_BID, 0,
    "/command",          PRN_PIP, CM_ARG,
    "/dos-device",       PRN_DOS, CM_ARG,
    "/end-of-job-string",PRN_TRM, CM_ARG,
    "/file",             PRN_FIL, CM_ARG,
#ifdef BPRINT
    "/flow-control",     PRN_FLO, CM_ARG,
#endif /* BPRINT */
    "/job-header-file",  PRN_SEP, CM_ARG,
#ifdef OS2
    "/length",           PRN_LEN, CM_ARG,
#endif /* OS2 */
    "/none",             PRN_NON, 0,
#ifdef OS2
    "/nopostscript",     PRN_RAW, 0,
    "/nops",             PRN_RAW, CM_INV,
#endif /* OS2 */
    "/output-only",      PRN_OUT, 0,
#ifdef BPRINT
    "/parity",           PRN_PAR, CM_ARG,
#endif /* BPRINT */
    "/pipe",             PRN_PIP, CM_ARG|CM_INV,
#ifdef OS2
    "/postscript",       PRN_PS,  0,
    "/ps",               PRN_PS,  CM_INV,
#endif /* OS2 */
    "/separator",        PRN_SEP, CM_ARG|CM_INV,
#ifdef BPRINT
    "/speed",            PRN_SPD, CM_ARG,
#endif /* BPRINT */
    "/timeout",          PRN_TMO, CM_ARG,
    "/terminator",       PRN_TRM, CM_ARG|CM_INV,
#ifdef OS2
#ifdef NT
    "/w",                PRN_WIN, CM_ARG|CM_ABR|CM_INV,
    "/wi",               PRN_WIN, CM_ARG|CM_ABR|CM_INV,
#endif /* NT */
    "/width",            PRN_WID, CM_ARG,
#endif /* OS2 */
#ifdef NT
    "/windows-queue",    PRN_WIN, CM_ARG,
#endif /* NT */
    "",                 0,      0
};
int nprnswi =  (sizeof(prntab) / sizeof(struct keytab)) - 1;
#endif /* PRINTSWI */

static int
setprinter(xx) int xx; {
    int x, y;
    char * s;
    char * defname = NULL;

#ifdef BPRINT
    char portbuf[64];
    long portspeed = 0L;
    int portparity = 0;
    int portflow = 0;
#endif /* BPRINT */

#ifdef PRINTSWI
    int c, i, n, wild, confirmed = 0;	/* Workers */
    int getval = 0;			/* Whether to get switch value */
    struct stringint {			/* Temporary array for switch values */
	char * sval;
	int ival;
    } pv[PRN_MAX+1];
    struct FDB sw, of, cm;		/* FDBs for each parse function */
    int haveque = 0;
#endif /* PRINTSWI */

#ifdef NT
    struct keytab * printtab = NULL, * _printtab = NULL;
    int nprint = 0, printdef=0;
#endif /* NT */

#ifdef OS2
    defname = "PRN";			/* default */
#else
#ifdef VMS
    defname = "LPT:";
#else
#ifdef UNIX
    defname = "|lpr";
#endif /* UNIX */
#endif /* VMS */
#endif /* OS2 */

#ifdef PRINTSWI
#ifdef NT
    haveque = Win32EnumPrt(&printtab,&_printtab,&nprint,&printdef);
    haveque = haveque && nprint;
#endif /* NT */

    for (i = 0; i <= PRN_MAX; i++) {	/* Initialize switch values */
	pv[i].sval = NULL;		/* to null pointers */
	pv[i].ival = -1;		/* and -1 int values */
    }
    if (xx == XYBDCP) {			/* SET BPRINTER == /BIDIRECTIONAL */
	pv[PRN_BID].ival = 1;
	pv[PRN_OUT].ival = 0;
    }

    /* Initialize defaults based upon current printer settings */
    if (printername) {
        defname = printername;
        switch (printertype) {
	  case PRT_WIN: pv[PRN_WIN].ival = 1; break;
	  case PRT_DOS: pv[PRN_DOS].ival = 1; break;
	  case PRT_PIP: pv[PRN_PIP].ival = 1; break;
	  case PRT_FIL: pv[PRN_FIL].ival = 1; break;
	  case PRT_NON: pv[PRN_NON].ival = 1; break;
        }
    }
#ifdef BPRINT
    /* only set the BIDI flag if we are bidi */
    if (printbidi)
        pv[PRN_BID].ival = 1;

    /* serial port parameters may be set for non-bidi devices */
    pv[PRN_SPD].ival = pportspeed / 10L;
    pv[PRN_PAR].ival = pportparity;
    pv[PRN_FLO].ival = pportflow;
#endif /* BPRINT */
    if (printtimo)
        pv[PRN_TMO].ival = printtimo;
    if (printterm) {
        pv[PRN_TRM].ival = 1;
        makestr(&pv[PRN_TRM].sval,printterm);
    }
    if (printsep) {
        pv[PRN_SEP].ival = 1;
        makestr(&pv[PRN_SEP].sval,printsep);
    }
    if (txt2ps) {
        pv[PRN_PS].ival = 1;
        pv[PRN_WID].ival = ps_width;
        pv[PRN_LEN].ival = ps_length;
    } else {
        pv[PRN_RAW].ival = 1;
    }

    /* Set up chained parse functions... */

    cmfdbi(&sw,				/* First FDB - command switches */
	   _CMKEY,			/* fcode */
	   "Switch",			/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   nprnswi,			/* addtl numeric data 1: tbl size */
	   4,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
	   prntab,			/* Keyword table */
	   &cm				/* Pointer to next FDB */
	   );
    cmfdbi(&cm,				/* Second fdb for confirmation */
	   _CMCFM,			/* fcode */
	   "",				/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   NULL,
	   NULL,
	   &of
	   );
    cmfdbi(&of,				/* Third FDB for printer name */
	   _CMOFI,			/* fcode */
	   "Printer or file name",	/* hlpmsg */
	   defname,			/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1: tbl size */
	   0,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
	   NULL,			/* Nothing */
	   NULL
	   );

    while (1) {				/* Parse 0 or more switches */
	x = cmfdb(&sw);			/* Parse switch or other thing */
	debug(F101,"setprinter cmfdb","",x);
	if (x < 0)			/* Error */
	  goto xsetprn;			/* or reparse needed */
	if (cmresult.fcode != _CMKEY)	/* Break out if not a switch */
	  break;
	if (cmresult.fdbaddr != &sw)	/* Advanced usage :-) */
	  break;
	c = cmgbrk();			/* Get break character */
	getval = (c == ':' || c == '='); /* to see how they ended the switch */
	n = cmresult.nresult;		/* Numeric result = switch value */
	debug(F101,"setprinter switch","",n);

	switch (n) {			/* Process the switch */
	  case PRN_PS:                  /* Text to Postscript */
            pv[PRN_PS].ival = 1;
	    pv[PRN_BID].ival = 0;
	    pv[PRN_OUT].ival = 1;
            pv[PRN_RAW].ival = 0;
            break;

	  case PRN_RAW:			/* Non-Postscript */
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
            break;

	  case PRN_BID:			/* Bidirectional */
	    pv[PRN_BID].ival = 1;
	    pv[PRN_OUT].ival = 0;
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
	    break;

	  case PRN_OUT:			/* Output-only */
	    pv[PRN_OUT].ival = 1;
	    pv[PRN_BID].ival = 0;
            pv[PRN_PS].ival = 0;
            pv[PRN_RAW].ival = 1;
	    break;

	  case PRN_NON:			/* NONE */
	    pv[n].ival = 1;
	    pv[PRN_SPD].ival = 0;
	    pv[PRN_PAR].ival = 0;
	    pv[PRN_FLO].ival = FLO_KEEP;
	    break;

#ifdef UNIX
	  case PRN_WIN:
#endif /* UNIX */
	  case PRN_DOS:			/* DOS printer name */
	  case PRN_FIL:			/* Or filename */
	  case PRN_PIP:
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = NULL;
	    pv[PRN_NON].ival = 0;	/* Zero any previous selections */
	    pv[PRN_WIN].ival = 0;
	    pv[PRN_DOS].ival = 0;
	    pv[PRN_FIL].ival = 0;
	    pv[PRN_PIP].ival = 0;
	    pv[n].ival = 1;		/* Flag this one */
	    if (!getval) break;		/* No value wanted */

	    if (n == PRN_FIL) {		/* File, check accessibility */
		int wild = 0;
		if ((x = cmiofi("Filename","kermit.prn",&s,&wild,xxstring))< 0)
		  if (x == -9) {
		      if (zchko(s) < 0) {
			  printf("Can't create \"%s\"\n",s);
			  return(x);
		      }
		  } else goto xsetprn;
		if (iswild(s)) {
		    printf("?A single file please\n");
		    return(-9);
		}
                pv[PRN_SPD].ival = 0;
                pv[PRN_PAR].ival = 0;
                pv[PRN_FLO].ival = FLO_KEEP;
	    } else if ((x = cmfld(n == PRN_DOS ? /* Value wanted - parse it */
			   "DOS printer device name" : /* Help message */
			   (n == PRN_PIP ?
			   "Program name" :
			   "Filename"),
			   n == PRN_DOS ?
			   "PRN" :	/* Default */
			   "",
			   &s,
			   xxstring
			   )) < 0)
	      goto xsetprn;
	    s = brstrip(s);		/* Strip enclosing braces */
	    while (*s == SP)		/* Strip leading blanks */
	      s++;
	    if (n == PRN_PIP) {		/* If /PIPE: */
		if (*s == '|') {	/* strip any extraneous pipe sign */
		    s++;
		    while (*s == SP)
		      s++;
		}
                pv[PRN_SPD].ival = 0;
                pv[PRN_PAR].ival = 0;
                pv[PRN_FLO].ival = FLO_KEEP;
	    }
	    if ((y = strlen(s)) > 0)	/* Anything left? */
	      if (pv[n].sval = (char *) malloc(y+1)) /* Yes, keep it */
		strcpy(pv[n].sval,s);
	    break;
#ifdef NT
	  case PRN_WIN:			/* Windows queue name */
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = NULL;
	    pv[PRN_NON].ival = 0;
	    pv[PRN_DOS].ival = 0;
	    pv[PRN_FIL].ival = 0;
	    pv[n].ival = 1;
            pv[PRN_SPD].ival = 0;
            pv[PRN_PAR].ival = 0;
            pv[PRN_FLO].ival = FLO_KEEP;

	    if (!getval || !haveque)
	      break;
	    if ((x = cmkey(_printtab,nprint,"Print queue name",
			   _printtab[printdef].kwd,xxstring)) < 0) {
		if (x != -2)
		  goto xsetprn;

		if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
		s = atmbuf;
		if ((y = strlen(s)) > 0)
		  if (pv[n].sval = (char *)malloc(y+1))
		    strcpy(pv[n].sval,s);
	    } else {
		if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
		s = printtab[x].kwd;
		if ((y = strlen(s)) > 0)
		  if (pv[n].sval = (char *)malloc(y+1))
		    strcpy(pv[n].sval,s);
	    }
	    break;
#endif /* NT */

	  case PRN_SEP:			/* /JOB-HEADER (separator) */
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = NULL;
	    pv[n].ival = 1;
	    if (!getval) break;
	    if ((x = cmifi("Filename","",&s,&y,xxstring)) < 0)
	      goto xsetprn;
	    if (y) {
		printf("?Wildcards not allowed\n");
		x = -9;
		goto xsetprn;
	    }
	    if ((y = strlen(s)) > 0)
	      if (pv[n].sval = (char *) malloc(y+1))
		strcpy(pv[n].sval,s);
	    break;

	  case PRN_TMO:			/* /TIMEOUT:number */
	    pv[n].ival = 0;
	    if (!getval) break;
	    if ((x = cmnum("Seconds","0",10,&y,xxstring)) < 0)
	      goto xsetprn;
	    if (y > 999) {
		printf("?Sorry - 999 is the maximum\n");
		x = -9;
		goto xsetprn;
	    } else
	      pv[n].ival = y;
	    break;

	  case PRN_TRM:			/* /END-OF-JOB:string */
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = NULL;
	    pv[n].ival = 1;
	    if (!getval) break;
	    if ((x = cmfld("String (enclose in braces if it contains spaces)",
			   "",&s,xxstring)) < 0)
	      goto xsetprn;
	    s = brstrip(s);
	    if ((y = strlen(s)) > 0)
	      if (pv[n].sval = (char *) malloc(y+1))
		strcpy(pv[n].sval,s);
	    break;

#ifdef BPRINT
	  case PRN_FLO:
	    if (!getval) break;
	    if ((x = cmkey(flotab,nflo,
			      "Serial printer-port flow control",
			      "rts/cts",xxstring)) < 0)
	      goto xsetprn;
	    pv[n].ival = x;
	    break;

#ifndef NOLOCAL
	  case PRN_SPD:
	    if (!getval) break;
	    if ((x = cmkey(spdtab,	/* Speed (no default) */
			   nspd,
			   "Serial printer-port interface speed",
			   "9600",
			   xxstring)
		 ) < 0)
	      goto xsetprn;
	    pv[n].ival = x;
	    break;
#endif /* NOLOCAL */

	  case PRN_PAR:
	    pv[n].ival = 0;
	    if (!getval) break;
	    if ((x = cmkey(partbl,npar,"Serial printer-port parity",
			   "none",xxstring)) < 0)
	      goto xsetprn;
	    pv[n].ival = x;
	    break;
#endif /* BPRINT */

#ifdef OS2
	  case PRN_LEN:
	    if (!getval) break;
            if ((x = cmnum("PS page length", "66",10,&y,xxstring)) < 0)
	      goto xsetprn;
	    pv[n].ival = y;
	    break;

	  case PRN_WID:
	    if (!getval) break;
            if ((x = cmnum("PS page width", "80",10,&y,xxstring)) < 0)
	      goto xsetprn;
	    pv[n].ival = y;
            break;
#endif /* OS2 */

	  default:
	    printf("?Unexpected switch value - %d\n",cmresult.nresult);
	    x = -9;
	    goto xsetprn;
	}
    }
    line[0] = NUL;			/* Initialize printer name value */
    switch (cmresult.fcode) {		/* How did we get here? */
      case _CMOFI:			/* They typed a filename */
	strcpy(line,cmresult.sresult);	/* Name */
	wild = cmresult.nresult;	/* Wild flag */
	if ((x = cmcfm()) < 0)		/* Confirm the command */
	  goto xsetprn;
	break;
      case _CMCFM:			/* They entered the command */
	if (pv[PRN_DOS].ival > 0)
	  strcpy(line,pv[PRN_DOS].sval ? pv[PRN_DOS].sval : "");
	else if (pv[PRN_WIN].ival > 0)
	  strcpy(line,pv[PRN_WIN].sval ? pv[PRN_WIN].sval : "");
	else if (pv[PRN_FIL].ival > 0)
	  strcpy(line,pv[PRN_FIL].sval ? pv[PRN_FIL].sval : "");
	else if (pv[PRN_PIP].ival > 0)
	  strcpy(line,pv[PRN_PIP].sval ? pv[PRN_PIP].sval : "");
	break;
      default:				/* By mistake */
	printf("?Unexpected function code: %d\n",cmresult.fcode);
	x = -9;
	goto xsetprn;
    }

#else  /* No PRINTSWI */

    if ((x = cmofi("Printer or file name",defname,&s,xxstring)) < 0)
      return(x);
    if (x > 1) {
	printf("?Directory names not allowed\n");
	return(-9);
    }
    while (*s == SP || *s == HT) s++;	/* Trim leading whitespace */
    strcpy(line,s);			/* Make a temporary safe copy */
    if ((x = cmcfm()) < 0) return(x);	/* Confirm the command */
#endif /* PRINTSWI */

#ifdef IKSD
    if (inserver && (isguest
#ifndef NOSERVER
		     || !ENABLED(en_pri)
#endif /* NOSERVER */
		     )) {
        printf("Sorry, printing disabled\r\n");
        return(success = 0);
    }
#endif /* ISKD */

#ifdef PRINTSWI
#ifdef BPRINT
    if (printbidi) {			/* If bidi printing active */
#ifndef UNIX
	bprtstop();			/* Stop it before proceeding */
#endif /* UNIX */
	printbidi = 0;
    }
    if (pv[PRN_SPD].ival > 0) {
        portspeed = (long) pv[PRN_SPD].ival * 10L;
        if (portspeed == 70L) portspeed = 75L;
    }
    if (pv[PRN_PAR].ival > 0)
        portparity = pv[PRN_PAR].ival;
    if (pv[PRN_FLO].ival > 0)
        portflow = pv[PRN_FLO].ival;
#endif /* BPRINT */
#endif /* PRINTSWI */

    s = line;				/* Printer name, if given */

#ifdef OS2ORUNIX
#ifdef PRINTSWI
    if (pv[PRN_PIP].ival > 0) {		/* /PIPE was given? */
	printpipe = 1;
	noprinter = 0;
	if (*s ==  '|') {		/* It might still have a pipe sign */
	    s++;			/* if name give later */
	    while (*s == SP)		/* so remove it and spaces */
	      s++;
	}
    } else
#endif /* PRINTSWI */
      if (*s == '|') {			/* Or pipe implied by name? */
	s++;				/* Point past pipe sign */
	while (*s == SP)		/* Gobble whitespace */
	  s++;
	if (*s) {
	    printpipe = 1;
	    noprinter = 0;
	}
    }

#ifdef PRINTSWI
#ifdef BPRINT
    if (printpipe && pv[PRN_BID].ival > 0) {
	printf("?Sorry, pipes not allowed for bidirectional printer\n");
	return(-9);
    }
#endif /* BPRINT */
#endif /* PRINTSWI */
#endif /* OS2ORUNIX */

#ifdef OS2
    if ( pv[PRN_PS].ival > 0 ) {
        txt2ps = 1;
        ps_width = pv[PRN_WID].ival <= 0 ? 80 : pv[PRN_WID].ival;
        ps_length = pv[PRN_LEN].ival <= 0 ? 66 : pv[PRN_LEN].ival;
    }
#endif /* OS2 */

    y = strlen(s);			/* Length of name of new print file */
    if (y > 0
#ifdef OS2
	&& ((y != 3) || (ckstrcmp(s,"PRN",3,0) != 0))
#endif /* OS2 */
	) {
	if (printername) {		/* Had a print file before? */
	    free(printername);		/* Remove its name */
	    printername = NULL;
	}
	printername = (char *) malloc(y + 1); /* Allocate space for it */
	if (!printername) {
	    printf("?Memory allocation failure\n");
	    return(-9);
	}
	strcpy(printername,s);	/* Copy new name to new space */
	debug(F110,"printername",printername,0);
    }

#ifdef PRINTSWI
    /* Set printer type from switches that were given explicitly */

    if (pv[PRN_NON].ival > 0) {		/* No printer */
	printertype = PRT_NON;
	noprinter = 1;
	printpipe = 0;
    } else if (pv[PRN_FIL].ival > 0) {	/* File */
	printertype = PRT_FIL;
	noprinter = 0;
	printpipe = 0;
    } else if (pv[PRN_PIP].ival > 0) {	/* Pipe */
	printertype = PRT_PIP;
	noprinter = 0;
	printpipe = 1;
    } else if (pv[PRN_WIN].ival > 0) {	/* Windows print queue */
	printertype = PRT_WIN;
	noprinter = 0;
	printpipe = 0;
    } else if (pv[PRN_DOS].ival > 0) {	/* DOS device */
	printertype = PRT_DOS;
	noprinter = 0;
	printpipe = 0;
    } else if (line[0]) {		/* Name given without switches */
	noprinter = 0;
	printertype = printpipe ? PRT_PIP : PRT_DOS;
#ifdef NT
        if (!lookup(_printtab,line,nprint,&x)) {
            printertype = PRT_WIN;
            if (pv[PRN_WIN].sval) free(pv[PRN_WIN].sval);
            if (printername) {		/* Had a print file before? */
                free(printername);	/* Remove its name */
                printername = NULL;
            }
            pv[PRN_WIN].sval = NULL;
            pv[PRN_WIN].ival = 1;
#ifdef COMMENT
            pv[PRN_NON].ival = 0;	/* These aren't needed */
            pv[PRN_DOS].ival = 0;
            pv[PRN_FIL].ival = 0;
#endif /* COMMENT */
            s = printtab[x].kwd;	/* Get full new name */
            if ((y = strlen(s)) > 0) {
                makestr(&pv[PRN_WIN].sval,s);
                makestr(&printername,s);
                if (!printername) {
                    printf("?Memory allocation failure\n");
                    return(-9);
                }
                debug(F110,"printername",printername,0);
            }
        }
#endif /* NT */
    }

#ifdef BPRINT
    /* Port parameters may be set for non-bidi mode */

    pportspeed = portspeed;		/* Set parameters */
    pportparity = portparity;
    pportflow = portflow;

    if (pv[PRN_BID].ival > 0) {		/* Bidirectional */
#ifdef UNIX
	printbidi = 1;			/* (just to test parsing...) */
#else
	printbidi = bprtstart();	/* Start bidirectional printer */
#endif /* UNIX */
	return(success = printbidi);
    } else
      printbidi = 0;			/* Not BPRINTER, unset flag */
#endif /* BPRINT */

    if (pv[PRN_TMO].ival > -1) {	/* Take care of timeout */
	printtimo = pv[PRN_TMO].ival;
    }
    if (pv[PRN_TRM].ival > 0) {		/* Termination string */
	if (printterm) {
	    free(printterm);
	    printterm = NULL;
	}
	if (pv[PRN_TRM].sval)
	  makestr(&printterm,pv[PRN_TRM].sval);
    }
    if (pv[PRN_SEP].ival > 0) {		/* and separator file */
	if (printsep) {
	    free(printsep);
	    printsep = NULL;
	}
	if (pv[PRN_SEP].sval)
	  makestr(&printsep,pv[PRN_SEP].sval);
    }
#endif /* PRINTSWI */

#ifdef UNIXOROSK
    if (!printpipe
#ifdef PRINTSWI
	&& !noprinter
#endif /* PRINTSWI */
	) {				/* File - check access */
	if (zchko(s) < 0) {
	    printf("?Access denied - %s\n",s);
	    x = -9;
	    goto xsetprn;
	}
    }
#endif /* UNIXOROSK */

    x = 1;				/* Return code */

  xsetprn:				/* Common exit */
#ifdef PRINTSWI
    for (i = 0; i <= PRN_MAX; i++) {	/* Free malloc'd memory */
	if (pv[i].sval)
	  free(pv[i].sval);
    }
#endif /* PRINTSWI */
    success = (x > 0) ? 1 : 0;
    return(x);
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
    int i = 0, x = 0, y = 0, z = 0;
    long zz = 0L;
    char *s = NULL, *p = NULL;
#ifdef OS2
    char portbuf[64];
    long portspeed = 0L;
    int portparity = 0;
    int portflow = 0;
#endif /* OS2 */

#ifdef OS2
    if (xx == XYMSK)
      return(setmsk());
#endif /* OS2 */

    if (xx == XYFLAG) {			/* SET FLAG */
	extern int ooflag;
	return(success = seton(&ooflag));
    }
    if (xx == XYPRTR			/* SET PRINTER (or BPRINTER) */
#ifdef BPRINT
	|| xx == XYBDCP
#endif /* BPRINT */
	)
      return(setprinter(xx));

switch (xx) {

#ifdef ANYX25				/* SET X25 ... */
case XYX25:
    return(setx25());

#ifndef IBMX25
case XYPAD:				/* SET PAD ... */
    return(setpadp());
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOXFER
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
    if ((y = cmkey(ifdatab,3,"","auto",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    if (rmsflg) {
	sstate = setgen('S',
			"310",
			y == 0 ? "0" : (y == 1 ? "1" : "2"),
			 ""
			);
	return((int) sstate);
    } else {
	keep = y;
	return(success = 1);

    }
#endif /* NOXFER */

#ifndef NOSPL
case XYINPU:				/* SET INPUT */
    return(setinp());
#endif /* NOSPL */

#ifdef NETCONN
case XYNET: {				/* SET NETWORK */

    struct FDB k1, k2;

    cmfdbi(&k1,_CMKEY,"","","",nnetkey, 0, xxstring, netkey, &k2);
    cmfdbi(&k2,_CMKEY,"","","",nnets,   0, xxstring, netcmd, NULL);

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

    x = cmfdb(&k1);
    if (x < 0) {			/* Error */
	if (x == -2 || x == -9)
	  printf("?No keywords match: \"%s\"\n",atmbuf);
	return(x);
    }
    z = cmresult.nresult;		/* Keyword value */
    if (cmresult.fdbaddr == &k1) {	/* Which table? */
#ifndef NOSPL
#ifndef NODIAL
	if (z == XYNET_D)
	  return(parsdir(1));
#endif /* NODIAL */
#endif /* NOSPL */
	if ((z = cmkey(netcmd,nnets,"",tmpbuf,xxstring)) < 0)
	  return(z);
    }

#ifdef NETCMD
    if (z == NET_CMD && nopush) {
        printf("\n?Sorry, access to external commands is disabled\n");
	return(-9);
    }
#endif /* NETCMD */

#ifndef NOPUSH
#ifdef NETPTY
    if (z == NET_PTY && nopush) {
        printf("\n?Sorry, access to external commands is disabled\n");
	return(-9);
    }
#endif /* NETPTY */
#endif /* NOPUSH */

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
		ckstrncpy(tmpnbnam,NetBiosName,NETBIOS_NAME_LEN+1);
                /* convert trailing spaces to NULs */
		p = &tmpnbnam[NETBIOS_NAME_LEN-1];
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
#ifdef NPIPE
	if (z == NET_PIPE)
	  ckstrncpy(pipename,s,PIPENAML);
#endif /* NPIPE */
    } else
#endif /* NPIPEORBIOS */
#ifdef DECNET
      if (z == NET_DEC) {
          /* Determine if we are using LAT or CTERM */
          if ((y = cmkey(dnettab,ndnet,"DECNET protocol","lat",xxstring)) < 0)
              return(y);
          if ((x = cmcfm()) < 0) return(x);
          ttnproto = y;
      } else
#endif /* DECNET */
#ifdef NETDLL
    if (z == NET_DLL) {
        /* Find out which DLL they are using */
        char dllname[256]="";
        char * p=NULL;
	if ((x = cmifi("Dynamic load library","",&p,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of dynamic load library (dll) required\n");
		return(-9);
	    }
	    return(x);
	}
        ckstrncpy(dllname,p,256);
        if ((x = cmcfm()) < 0) return(x);

        /* Try to load the dll */
        if (netdll_load(dllname) < 0)
            return(success = 0);
        else {
            nettype = z;
            return(success = 1);
        }
    } else
#endif /* NETDLL */
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
#ifdef NETCMD
        (nettype != NET_CMD) &&
#endif /* NETCMD */
#ifdef NETPTY
        (nettype != NET_PTY) &&
#endif /* NETPTY */
#ifdef NETDLL
        (nettype != NET_DLL) &&
#endif /* NETDLL */
#ifdef SSH
        (nettype != NET_SSH) &&
#endif /* SSH */
#ifdef SUPERLAT
	(nettype != NET_SLAT) &&
#endif /* SUPERLAT */
	(nettype != NET_SX25) &&
	(nettype != NET_VX25) &&
#ifdef IBMX25
	(nettype != NET_IX25) &&
#endif /* IBMX25 */
        (nettype != NET_TCPB)) {
	printf("?Network type not supported\n");
	return(success = 0);
    } else {
	return(success = 1);
    }
}

#ifndef NOTCPOPTS
#ifdef TCPSOCKET
case XYTCP:
if ((z = cmkey(tcpopt,ntcpopt,"TCP option","nodelay",xxstring)) < 0)
   return(z);

   switch (z) {
     case XYTCP_ADDRESS:
       if ((y = cmtxt("preferred IP Address for TCP connections","",
		      &s,xxstring)) < 0)
	 return(y);
       if (tcp_address) {
	   free(tcp_address);		/* Free any previous storage */
	   tcp_address = NULL;
       }
       if (s == NULL || *s == NUL) {	/* If none given */
	   tcp_address = NULL;		/* remove the override string */
	   return(success = 1);
       } else if (tcp_address = malloc(strlen(s)+1)) { /* Make new storage */
	   strcpy(tcp_address,s);	/* Copy string to new storage */
	   return(success = 1);
       } else
	 return(success = 0);
#ifdef SO_KEEPALIVE
   case XYTCP_KEEPALIVE:
      if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = keepalive(z) ;
      return(success);
#endif /* SO_KEEPALIVE */
#ifdef SO_DONTROUTE
   case XYTCP_DONTROUTE:
      if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = dontroute(z) ;
      return(success);
#endif /* SO_DONTROUTE */
#ifdef TCP_NODELAY
   case XYTCP_NODELAY:
      if ((z = cmkey(onoff,2,"","off",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = no_delay(z) ;
      return(success);
   case XYTCP_NAGLE: /* The inverse of NODELAY */
      if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
      if ((y = cmcfm()) < 0) return(y);
      success = no_delay(!z) ;
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

/* Note: The following is not 16-bit safe */

#ifndef QNX16
      if (x > 52248) {
          printf("?Warning: receive buffers larger than 52248 bytes\n");
          printf(" may not be understood by all hosts.  Performance\n");
          printf(" may suffer.\n");
          return(-9);
      }
#endif /* QNX16 */
      success = recvbuf(z);
      return(success);
#endif /* SO_RCVBUF */

#ifdef VMS
#ifdef DEC_TCPIP
    case XYTCP_UCX: {			/* UCX 2.0 port swabbing bug */
	extern int ucx_port_bug;
	return(success = seton(&ucx_port_bug));
    }
#endif /* DEC_TCPIP */
#endif /* VMS */

    case XYTCP_RDNS: {
	extern int tcp_rdns;
	return(success = setonaut(&tcp_rdns));
    }

#ifdef CK_DNS_SRV
     case XYTCP_DNS_SRV: {
       extern int tcp_dns_srv;
       return(success = setonaut(&tcp_dns_srv));
     }
#endif /* CK_DNS_SRV */

     default:
       return(0);
   }
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */
#endif /* NETCONN */
}

switch (xx) {

#ifndef NOLOCAL
#ifdef NETCONN
case XYHOST:				/* SET HOST */
{
    z = ttnproto;			/* Save protocol in case of failure */
#ifdef DECNET
    if (nettype != NET_DEC)
#endif /* DECNET */
      ttnproto = NP_NONE;
    if ((y = setlin(XYHOST,1,0)) < 0) {
	debug(F101,"SET HOST fail mdmtyp","",mdmtyp);
        ttnproto = z;		        /* Failed, restore protocol */
        success = 0;
    }
    didsetlin++;
    debug(F101,"SET HOST OK mdmtyp","",mdmtyp);
    return(y);
}
#endif /* NETCONN */

case XYLINE:				/* SET LINE (= SET PORT) */
    debug(F101,"setlin flow 1","",flow);
    x = setlin(xx,1,0);
    if (x > -1) didsetlin++;
    debug(F101,"setlin returns","",x);
    debug(F101,"setlin flow 2","",flow);
    debug(F101,"setlin local","",local);
    return(x);
#endif /* NOLOCAL */

#ifndef NOSETKEY
case XYKEY:				/* SET KEY */
    return(dosetkey());
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
#ifdef VMS
    if (batch && bgset == 0)		/* To enable echoing of commands */
      ckxech = 1;			/* in VMS batch logs */
#endif /* VMS */
    success = 1;
    bgchk();
    return(success);
#endif /* MAC */

case XYQUIE: {				/* QUIET */
#ifdef DCMDBUF
      extern int * xquiet;
#else
      extern int xquiet[];
#endif /* DCMDBUF */
      x = seton(&quiet);
      if (x < 0) return(x);
      xquiet[cmdlvl] = quiet;
      return(success = x);
    }

#ifndef NOXFER
case XYBUF: {				/* BUFFERS */
#ifdef DYNAMIC
    int sb, rb;
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
    if ((y = cmnum("Receive buffer size",ckitoa(sb),10,&rb,xxstring)) < 0)
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
#endif /* NOXFER */

#ifndef NOLOCAL
#ifndef MAC				/* The Mac has no RS-232 */
case XYCARR:				/* CARRIER-WATCH */
    return(setdcd());
#endif /* MAC */
#endif /* NOLOCAL */

}

#ifdef TNCODE
switch (xx) {				/* Avoid long switch statements... */
    case XYTELOP: {
        int c, n;			/* Workers */
        int getval = 0;			/* Whether to get switch value */
        int tnserver = 0;               /* Client by default */
        int opt = -1;                   /* Telnet Option */
        struct FDB sw, op;		/* FDBs for each parse function */
#ifdef CK_AUTHENTICATION
        extern int sl_topt_a_s_saved;
        extern int sl_topt_a_c_saved;
        extern int sl_topt_e_s_saved;
        extern int sl_topt_e_c_saved;
#endif /* CK_AUTHENTICATION */
#ifdef IKSD
        if (inserver)			/* Server by default when IKSD */
	  tnserver = 1;
#endif /* IKSD */

        /* Set up chained parse functions... */

        cmfdbi(&op,			/* First fdb - telopts*/
	       _CMKEY,			/* fcode */
	       "/client, /server or",	/* hlpmsg */
	       "",			/* default */
	       "",			/* addtl string data */
	       ntnopt,			/* addtl numeric data 1 */
	       0,			/* addtl numeric data 2 */
	       xxstring,
	       tnopttab,
	       &sw
	       );
        cmfdbi(&sw,			/* Second FDB - command switches */
	       _CMKEY,			/* fcode */
	       "",     			/* hlpmsg */
	       "",			/* default */
	       "",			/* addtl string data */
	       ntnoptsw,		/* addtl numeric data 1: tbl size */
	       4,			/* addtl numeric data 2: 4 = cmswi */
	       xxstring,		/* Processing function */
	       tnoptsw,			/* Keyword table */
	       NULL			/* Pointer to next FDB */
	       );

        while (opt < 0) {		/* Parse 0 or more switches */
            x = cmfdb(&op);		/* Parse switch or other thing */
            debug(F101,"XYTELOP cmfdb","",x);
            if (x < 0)			/* Error */
	      return(x);		/* or reparse needed */
            if (cmresult.fcode != _CMKEY) /* Break out if not a switch */
	      break;
            c = cmgbrk();		/* Get break character */
            getval = (c == ':' || c == '='); /* see how switch ended */
            if (getval && !(cmresult.kflags & CM_ARG)) {
                printf("?This switch does not take arguments\n");
                return(-9);
            }
            z = cmresult.nresult;	/* Numeric result = switch value */
            debug(F101,"XYTELOP switch","",z);

            switch (z) {			/* Process the switch */
	      case CK_TN_CLIENT:
                tnserver = 0;
                break;
	      case CK_TN_SERVER:
                tnserver = 1;
                break;
	      case CK_TN_EC:
                opt = TELOPT_ECHO;
                break;
	      case CK_TN_TT:
                opt = TELOPT_TTYPE;
                break;
	      case CK_TN_BM:
                opt = TELOPT_BINARY;
                break;
	      case CK_TN_ENV:
                opt = TELOPT_NEWENVIRON;
                break;
	      case CK_TN_LOC:
                opt = TELOPT_SNDLOC;
                break;
	      case CK_TN_AU:
                opt = TELOPT_AUTHENTICATION;
                break;
              case CK_TN_FX:
                opt = TELOPT_FORWARD_X;
                break;
	      case CK_TN_ENC:
                opt = TELOPT_ENCRYPTION;
                break;
	      case CK_TN_IKS:
                opt = TELOPT_KERMIT;
                break;
	      case CK_TN_TLS:
                opt = TELOPT_START_TLS;
                break;
	      case CK_TN_XD:
                opt = TELOPT_XDISPLOC;
                break;
	      case CK_TN_NAWS:
                opt = TELOPT_NAWS;
                break;
	      case CK_TN_SGA:
                opt = TELOPT_SGA;
                break;
              case CK_TN_PHR:
                opt = TELOPT_PRAGMA_HEARTBEAT;
                break;
              case CK_TN_PSP:
                opt = TELOPT_SSPI_LOGON;
                break;
              case CK_TN_PLG:
                opt = TELOPT_PRAGMA_LOGON;
                break;
              case CK_TN_SAK:
                opt = TELOPT_IBM_SAK;
                break;
              case CK_TN_CPC:
                opt = TELOPT_COM_PORT;
                break;
              case CK_TN_FLW:
                opt = TELOPT_LFLOW;
                break;
	      default:
                printf("?Unexpected value - %d\n",z);
                return(-9);
            }
#ifdef COMMENT
            if (cmresult.fdbaddr == &op)
	      break;
#endif /* COMMENT */
        }
        switch (opt) {
	  case TELOPT_ECHO:		/* Options only the Server WILL */
          case TELOPT_FORWARD_X:
	  case TELOPT_SEND_URL:
          case TELOPT_IBM_SAK:
            if ((x = cmkey(tnnegtab,
			   ntnnegtab,
			   "desired server state",
   TELOPT_MODE(tnserver?TELOPT_DEF_S_ME_MODE(opt):TELOPT_DEF_C_U_MODE(opt)),
			   xxstring)
		 ) < 0)
	      return(x);
            if ((z = cmcfm()) < 0)
	      return(z);
            if (tnserver) {
                TELOPT_DEF_S_ME_MODE(opt) = x;
                TELOPT_ME_MODE(opt) = x;
            } else {
                TELOPT_DEF_C_U_MODE(opt) = x;
                TELOPT_U_MODE(opt) = x;
            }
            break;

	  case TELOPT_TTYPE:		/* Options only the Client WILL */
	  case TELOPT_NEWENVIRON:
	  case TELOPT_SNDLOC:
	  case TELOPT_AUTHENTICATION:
	  case TELOPT_START_TLS:
	  case TELOPT_XDISPLOC:
	  case TELOPT_NAWS:
          case TELOPT_LFLOW:
          case TELOPT_COM_PORT:
            if ((x = cmkey(tnnegtab,
			   ntnnegtab,
			   "desired client state",
    TELOPT_MODE(!tnserver?TELOPT_DEF_S_U_MODE(opt):TELOPT_DEF_C_ME_MODE(opt)),
                            xxstring)
		 ) < 0)
	      return(x);
            if ((z = cmcfm()) < 0)
	      return(z);
            if (tnserver) {
                TELOPT_DEF_S_U_MODE(opt) = x;
                TELOPT_U_MODE(opt) = x;
#ifdef CK_AUTHENTICATION
                if (opt == TELOPT_AUTHENTICATION)
		  sl_topt_a_s_saved = 0;
#endif /* CK_AUTHENTICATION */
            } else {
                TELOPT_DEF_C_ME_MODE(opt) = x;
                TELOPT_ME_MODE(opt) = x;
#ifdef CK_AUTHENTICATION
                if (opt == TELOPT_AUTHENTICATION)
		  sl_topt_a_c_saved = 0;
#endif /* CK_AUTHENTICATION */
            }
            break;

	  default:
            if ((x = cmkey(tnnegtab,
			   ntnnegtab,
			   tnserver ?
			   "desired server state" :
			   "desired client state",
    TELOPT_MODE(tnserver?TELOPT_DEF_S_ME_MODE(opt):TELOPT_DEF_C_ME_MODE(opt)),
			   xxstring
			   )
		 ) < 0)
	      return(x);
            if ((y = cmkey(tnnegtab,
			   ntnnegtab,
			   !tnserver ? "desired server state" :
			   "desired client state",
    TELOPT_MODE(!tnserver?TELOPT_DEF_S_U_MODE(opt):TELOPT_DEF_C_U_MODE(opt)),
			   xxstring
			   )
		 ) < 0)
	      return(y);
            if ((z = cmcfm()) < 0)
	      return(z);
            if (tnserver) {
                TELOPT_DEF_S_ME_MODE(opt) = x;
                TELOPT_ME_MODE(opt) = x;
                TELOPT_DEF_S_U_MODE(opt) = y;
                TELOPT_U_MODE(opt) = y;
#ifdef CK_ENCRYPTION
                if (opt == TELOPT_ENCRYPTION)
		  sl_topt_e_s_saved = 0;
#endif /* CK_ENCRYPTION */
            } else {
                TELOPT_DEF_C_ME_MODE(opt) = x;
                TELOPT_ME_MODE(opt) = x;
                TELOPT_DEF_C_U_MODE(opt) = y;
                TELOPT_U_MODE(opt) = y;
#ifdef CK_ENCRYPTION
                if (opt == TELOPT_ENCRYPTION)
		  sl_topt_e_c_saved = 0;
#endif /* CK_ENCRYPTION */
            }
        }
        return(success = 1);
    }

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

      case CK_TN_RE:                    /* REMOTE-ECHO */
        return(success = seton(&tn_rem_echo));

      case CK_TN_DB:                    /* DEBUG */
          return(success = seton(&tn_deb));

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

      case CK_TN_XF:			/* BINARY-TRANSFER-MODE */
          if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
          if ((y = cmcfm()) < 0) return(y);
          tn_b_xfer = z;
          return(success = 1);

      case CK_TN_NE:			/* NO-ENCRYPT-DURING-XFER */
          if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
          if ((y = cmcfm()) < 0) return(y);
#ifdef CK_APC
          /* Don't let this be set remotely */
          if (apcactive == APC_LOCAL ||
               apcactive == APC_REMOTE && apcstatus != APC_UNCH)
              return(success = 0);
#endif /* CK_APC */
          tn_no_encrypt_xfer = z;
          return(success = 1);

      case CK_TN_BM:			/* BINARY-MODE */
        if ((x = cmkey(tnnegtab,ntnnegtab,"","refused",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	TELOPT_DEF_S_ME_MODE(TELOPT_BINARY) = x;
	TELOPT_DEF_S_U_MODE(TELOPT_BINARY) = x;
	TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = x;
	TELOPT_DEF_C_U_MODE(TELOPT_BINARY) = x;
	return(success = 1);

#ifdef IKS_OPTION
      case CK_TN_IKS:			/* KERMIT */
	if ((x = cmkey(tnnegtab,ntnnegtab,"DO","accept",xxstring)) < 0)
	  return(x);
        if ((y = cmkey(tnnegtab,ntnnegtab,"WILL","accept",xxstring)) < 0)
            return(y);
        if ((z = cmcfm()) < 0)
          return(z);
	TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = y;
	TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = x;
	TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = y;
	TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = x;
	return(success = 1);
#endif /* IKS_OPTION */

#ifdef CK_SSL
      case CK_TN_TLS:			/* START_TLS */
	if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
	  return(x);
	if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
	  return(y);
	if ((z = cmcfm()) < 0)
	  return(z);
	TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = x;
	TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = y;
	TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = x;
	TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = y;
	return(success = 1);
#endif /* CK_SSL */

#ifdef CK_NAWS
      case CK_TN_NAWS:			/* NAWS */
	if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
	  return(x);
	if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
	  return(y);
	if ((z = cmcfm()) < 0)
	  return(z);
	TELOPT_DEF_S_ME_MODE(TELOPT_NAWS) = x;
	TELOPT_DEF_S_U_MODE(TELOPT_NAWS) = y;
	TELOPT_DEF_C_ME_MODE(TELOPT_NAWS) = x;
	TELOPT_DEF_C_U_MODE(TELOPT_NAWS) = y;
	return(success = 1);
#endif /* CK_NAWS */

#ifdef CK_AUTHENTICATION
      case CK_TN_AU:			/* AUTHENTICATION */
	if ((x = cmkey(tnauthtab,ntnauth,"","accept",xxstring)) < 0)
	  return(x);
	if (x == TN_AU_FWD) {
	    extern int forward_flag;
	    return(success = seton(&forward_flag));
	} else if (x == TN_AU_TYP) {
	    extern int auth_type_user[];
            extern int sl_auth_type_user[];
            extern int sl_auth_saved;
            int i, j, atypes[AUTHTYPLSTSZ];

            for (i = 0; i < AUTHTYPLSTSZ; i++) {
                if ((y = cmkey(autyptab,nautyp,"",
			       i == 0 ? "automatic" : "" ,
			       xxstring)) < 0) {
                    if (y == -3)
		      break;
                    return(y);
                }
                if (i > 0 && (y == AUTHTYPE_AUTO || y == AUTHTYPE_NULL)) {
		    printf(
		       "\r\n?Choice may only be used in first position.\r\n");
		    return(-9);
                }
                for (j=0 ; j< i; j++) {
                    if (atypes[j] == y) {
                        printf("\r\n?Choice has already been used.\r\n");
                        return(-9);
                    }
                }
                atypes[i] = y;
                if (y == AUTHTYPE_NULL || y == AUTHTYPE_AUTO) {
                    i++;
                    break;
                }
            }
            if (i < AUTHTYPLSTSZ)
	      atypes[i] = AUTHTYPE_NULL;
	    if ((z = cmcfm()) < 0)
	      return(z);
            sl_auth_saved = 0;
            for (i = 0; i < AUTHTYPLSTSZ; i++) {
                auth_type_user[i] = atypes[i];
                sl_auth_type_user[i] = 0;
            }
	} else if (x == TN_AU_HOW) {
	    if ((y = cmkey(auhowtab,nauhow,"","any",xxstring)) < 0)
	      return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    tn_auth_how = y;
	} else if (x == TN_AU_ENC) {
	    if ((y = cmkey(auenctab,nauenc,"","encrypt",xxstring)) < 0)
	      return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    tn_auth_enc = y;
	} else {
	    if ((y = cmcfm()) < 0)
	      return(y);
	    TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = x;
	    TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = x;
	}
	return(success = 1);
#endif /* CK_AUTHENTICATION */

#ifdef CK_ENCRYPTION
      case CK_TN_ENC: {			/* ENCRYPTION */
	  int c, tmp = -1;
	  int getval = 0;
	  static struct keytab * tnetbl = NULL;
	  static int ntnetbl = 0;

          if ((y = cmkey(tnenctab,ntnenc,"","accept",xxstring)) < 0)
	    return(y);
          switch (y) {
	    case TN_EN_TYP:
              x = ck_get_crypt_table(&tnetbl,&ntnetbl);
              debug(F101,"ck_get_crypt_table x","",x);
              debug(F101,"ck_get_crypt_table n","",ntnetbl);
              if (x < 1 || !tnetbl || ntnetbl < 1) /* Didn't get it */
		x = 0;
              if (!x) {
                  printf("?Oops, types not loaded\n");
                  return(-9);
              }
              if ((x = cmkey(tnetbl,ntnetbl,"type of encryption",
			     "automatic",xxstring)) < 0)
		return(x);
              if ((z = cmcfm()) < 0)
		return(z);
              cx_type = x;
              sl_cx_type = 0;
              break;
	    case TN_EN_START:
              if ((z = cmcfm()) < 0)
		return(z);
#ifdef CK_APC
	      /* Don't let this be set remotely */
	      if (apcactive == APC_LOCAL ||
	          apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	        return(success = 0);
#endif /* CK_APC */
              ck_tn_enc_start();
              break;
	    case TN_EN_STOP:
              if ((z = cmcfm()) < 0)
		return(z);
#ifdef CK_APC
	      /* Don't let this be set remotely */
	      if (apcactive == APC_LOCAL ||
	          apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	        return(success = 0);
#endif /* CK_APC */
              ck_tn_enc_stop();
              break;
	    default:
              if ((z = cmcfm()) < 0)
		return(z);
	      TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = y;
	      TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = y;
	      TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = y;
	      TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = y;
          }
          return(success = 1);
      }
#endif /* CK_ENCRYPTION */

      case CK_TN_BUG:			/* BUG */
	if ((x = cmkey(tnbugtab,4,"","binary-me-means-u-too",xxstring)) < 0)
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
          case 2:
	    tn_infinite = z;
            break;
          case 3:
            tn_sb_bug = z;
            break;
	}
	return(success = 1);

#ifdef CK_ENVIRONMENT
      case CK_TN_XD:			/* XDISPLOC */
	if ((x = cmkey(tnnegtab,ntnnegtab,"me","accept",xxstring)) < 0)
	  return(x);
	if ((y = cmkey(tnnegtab,ntnnegtab,"u","accept",xxstring)) < 0)
	  return(y);
	if ((z = cmcfm()) < 0)
	  return(z);
	TELOPT_DEF_S_ME_MODE(TELOPT_XDISPLOC) = x;
	TELOPT_DEF_S_U_MODE(TELOPT_XDISPLOC) = y;
	TELOPT_DEF_C_ME_MODE(TELOPT_XDISPLOC) = x;
	TELOPT_DEF_C_U_MODE(TELOPT_XDISPLOC) = y;
	return(success = 1);

      case CK_TN_ENV: {
	  char * msg = "value of telnet environment variable";
	  extern int tn_env_flg;
	  extern char tn_env_acct[], tn_env_disp[], tn_env_job[],
	  tn_env_prnt[], tn_env_sys[];
	  if ((x = cmkey(tnenvtab,ntnenv,"","",xxstring)) < 0)
	    return(x);
#ifdef COMMENT
          /* If we ever support USERVAR variables */
	  if (x == TN_ENV_UVAR) {
	      /* Get the user variable name */
	  }
#endif /* COMMENT */
	  if (x == TN_ENV_OFF || x == TN_ENV_ON) {
	      if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
              if (inserver) {
                  printf("?Sorry, command disabled.\r\n");
                  return(success = 0);
              }
#endif /* IKSD */
	      tn_env_flg = x == TN_ENV_OFF ? 0 : 1;
	      return(success = 1);
	  }

	  /* Not ON/OFF - Get the value */
	  z = cmdgquo();
	  cmdsquo(0);
	  if ((y = cmtxt(msg,
			 "", &s, xxstring)) < 0) {
	      cmdsquo(z);
	      return(y);
	  }
	  cmdsquo(z);
#ifdef IKSD
          if (inserver)
	    return(success = 0);
#endif /* IKSD */
	  if ((int)strlen(s) > 63) {
	      printf("Sorry, too long\n");
	      return(-9);
	  }
	  switch (x) {
	    case TN_ENV_USR:
	      ckstrncpy(uidbuf,s,UIDBUFLEN);
              sl_uid_saved = 0;
	      break;
	    case TN_ENV_ACCT:
	      ckstrncpy(tn_env_acct,s,64);
	      break;
	    case TN_ENV_DISP:
	      ckstrncpy(tn_env_disp,s,64);
	      break;
	    case TN_ENV_JOB:
	      ckstrncpy(tn_env_job,s,64);
	      break;
	    case TN_ENV_PRNT:
	      ckstrncpy(tn_env_prnt,s,64);
	      break;
	    case TN_ENV_SYS:
	      ckstrncpy(tn_env_sys,s,64);
	      break;
	    case TN_ENV_UVAR:
	      printf("\n?Not yet implemented\n");
	      break;
	  }
	  return(success = 1);
      }
#endif /* CK_ENVIRONMENT */

#ifdef CK_SNDLOC
      case CK_TN_LOC: {			/* LOCATION */
	  extern char * tn_loc;
	  if ((y = cmtxt("Location string","",&s,xxstring)) < 0)
	    return(y);
	  if (!*s) s = NULL;
	  makestr(&tn_loc,s);
	  return(success = 1);
      }
#endif /* CK_SNDLOC */

      case CK_TN_WAIT:			/* WAIT-FOR-NEGOTIATIONS */
	if ((z = cmkey(onoff,2,"","on",xxstring)) < 0) return(z);
	if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
	if (inserver &&
#ifdef IKSDCONF
	    iksdcf
#else
	    1
#endif /* IKSDCONF */
	    ) {
	    printf("?Sorry, command disabled.\r\n");
	    return(success = 0);
	}
#endif /* IKSD */
	tn_wait_flg = z;
        sl_tn_saved = 0;
	return(success = 1);

      case CK_TN_PUID: {                /* PROMPT-FOR-USERID */
	int i,len;
	if ((y = cmtxt("Prompt string","",&s,xxstring)) < 0)
	  return(y);
	if (s == "") s = NULL;
	if (s) {
	    s = brstrip(s);
	    if (s == "") s = NULL;
	}
        /* we must check to make sure there are no % fields */
        len = strlen(s);
        for (i = 0; i < len; i++) {
            if (s[i] == '%') {
                if (s[i+1] != '%') {
                    printf("%% fields are not used in this command.\n");
                    return(-9);
                }
                i++;
            }
        }
	makestr(&tn_pr_uid,s);
        return(success = 1);
      }
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
      case 876:
	if ((y = cmcfm()) < 0) return(y);
	concb((char)escape);
	return(success = 1);

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
	xxstring = y ? zzstring : (xx_strp) NULL;
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
	/* The max number of lines is the RAM  */
	/* we can actually dedicate to a       */
	/* scrollback buffer given the maximum */
	/* process memory space of 512MB       */
	if (x < 256 || x > 2000000L) {
	    printf("\n?The size must be between 256 and 2,000,000.\n");
	    return(success = 0);
 	}
	if ((y = cmcfm()) < 0) return(y);
          tt_scrsize[VCMD] = x;
          VscrnInit( VCMD ) ;
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
	  VscrnSetWidth(VCMD, x);
	  cmd_cols = x;
	  SetCols(VCMD);
	  return(success = 1);
      }
      case SCMD_HIG:
   	if ((y = cmnum(
"number of rows in display window during CONNECT",
		       "25",10,&x,xxstring)) < 0)
	  return(y);
	if ((y = cmcfm()) < 0) return(y);

	if (tt_modechg == TVC_DIS) {
	    printf("\n?SET TERMINAL VIDEO-CHANGE DISABLED\n");
	    return(success = 0);
	}
	if (IsOS2FullScreen()) {
	    if (x != 25 && x != 43 && x != 50 && x != 60) {
		printf("\n?The height must be 25, 43, 50");
#ifdef NT
		printf(" or 60 under Windows 95.\n.");
#else /* NT */
		printf(" or 60 in a Full Screen session.\n.");
#endif /* NT */
		return(success = 0);
	    }
	} else if (tt_modechg == TVC_W95) {
	    if (x != 25 && x != 43 && x != 50) {
		printf("\n?The height must be 25, 43, 50");
#ifdef NT
		printf(" under Windows 95.\n.");
#else /* NT */
		printf(" in a Full Screen session.\n.");
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
        tt_szchng[VCMD] = 1;
        tt_rows[VCMD] = cmd_rows = x;
        VscrnInit(VCMD);
	SetCols(VCMD);
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

        VscrnSetCurPos( VCMD, (short) (col-1), (short) (row-1) ) ;
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

      case SCMD_INT:
	return(seton(&cmdint));

#ifdef CK_AUTODL
      case SCMD_ADL:
	return(seton(&cmdadl));
#endif /* CK_AUTODL */

      default:
	return(-2);
    }
}

switch (xx) {

case XYDFLT:				/* SET DEFAULT = CD */
    return(success = docd(XXCWD));

case XYDEBU:				/* SET DEBUG { on, off, session } */
    if ((y = cmkey(dbgtab,ndbg,"","",xxstring)) < 0) {
	return(y);
    } else if (y == 3) {		/* 3 = timestamp */
	debug(F101,"set debug y","",y);
	x = seton(&debtim);
	debug(F101,"set debug x","",x);
	if (x < 0)
	  return(x);
	else
	  return(success = 1);
    } else if ((x = cmcfm()) < 0)
      return(x);
#ifdef IKSD
    if (inserver && isguest) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
    debug(F101,"set debug y","",y);
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

      case 2:				/* 2 = session */
	return(success = debses = 1);

      default:
        return(-2);
    }
    break;

#ifndef NOXFER
case XYDELA:				/* SET DELAY */
    y = cmnum("Number of seconds before starting to send","5",10,&x,xxstring);
    if (x < 0) x = 0;
    return(success = setnum(&ckdelay,x,y,999));
#endif /* NOXFER */

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

#ifndef NOLOCAL
case XYDUPL:				/* SET DUPLEX */
    if ((y = cmkey(dpxtab,2,"","full",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    duplex = y;
    return(success = 1);

case XYLCLE:				/* LOCAL-ECHO (= DUPLEX) */
    return(success = seton(&duplex));

case XYESC:				/* SET ESCAPE */
    return(success = setcc(ckitoa(DFESC),&escape));
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
  extern int cxflow[];

  case XYFILE:				/* SET FILE */
    return(setfil(rmsflg));

  case XYFLOW: {			/* FLOW-CONTROL */
    struct FDB k1, k2;
    cmfdbi(&k1,_CMKEY,
/* This is because chained FDB's don't give chained help yet */
#ifdef Plan9
#ifdef CK_RTSCTS
	   "Flow control type, one of the following:\n\
   keep   none    rts/cts\n\
 or connection type",
#else
	   "Flow control type, one of the following:\n\
   keep   none\n\
 or connection type",
#endif /* CK_RTSCTS */
#else
#ifdef CK_RTSCTS
#ifdef CK_DTRCD
	   "Flow control type, one of the following:\n\
   dtr/cd    ctr/cts   keep    none    rts/cts   xon/xoff\n\
 or connection type",
#else
	   "Flow control type, one of the following:\n\
   keep   none    rts/cts   xon/xoff\n\
 or connection type",
#endif /* CK_DTRCD */
#else
	   "Flow control type, one of the following:\n\
   keep   none    xon/xoff\n\
 or connection type",
#endif /* CK_RTSCTS */
#endif /* Plan9 */
	   "","",ncxtypesw, 4, xxstring, cxtypesw, &k2);
    cmfdbi(&k2,_CMKEY,"","","", nflo, 0, xxstring, flotab, NULL);
    x = cmfdb(&k1);
    if (x < 0) {			/* Error */
	if (x == -2 || x == -9)
	  printf("?No keywords or switches match: \"%s\"\n",atmbuf);
	return(x);
    }
    z = cmresult.nresult;		/* Keyword value */
    if (cmresult.fdbaddr == &k2) {	/* Flow-control type keyword table */
	if ((x = cmcfm()) < 0)		/* Set it immediately */
	  return(x);
	flow = z;
	debug(F101,"set flow","",flow);
#ifdef CK_SPEED
	if (flow == FLO_XONX)		/* Xon/Xoff forces prefixing */
	  ctlp[XON] = ctlp[XOFF] = ctlp[XON+128] = ctlp[XOFF+128] = 1;
#endif /* CK_SPEED */
	autoflow = (flow == FLO_AUTO);
	return(success = 1);		/* Done */
    }
    debug(F101,"set flow /blah 1","",z); /* SET FLOW /for-what */
    if ((y = cmkey(flotab,nflo,"Flow control type","none",xxstring)) < 0)
      return(y);
    if ((x = cmcfm()) < 0) return(x);
    debug(F101,"set flow /blah 2","",y);
    if (z != FLO_AUTO && z >= 0 && z <= CXT_MAX)
      cxflow[z] = y;
    debug(F101,"set flow","",flow);
    debug(F101,"set flow autoflow","",autoflow);
    return(success = 1);
  }

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
    if ((y = cmkey(partbl,npar,"","none",xxstring)) < 0)
      return(y);

/* If parity not none, then we also want 8th-bit prefixing */

#ifdef HWPARITY
    if (y == 'H') {			/* Hardware */
	if ((x = cmkey(hwpartbl,nhwpar,"","even",xxstring)) < 0)
	  return(x);
    }
#endif /* HWPARITY */

    if ((z = cmcfm()) < 0)
      return(z);

#ifdef HWPARITY
    if (y == 'H') {			/* 8 data bits plus hardware parity */
	parity = 0;
#ifndef NOXFER
	ebqflg = 0;
#endif /* NOXFER */
	hwparity = x;
    } else {				/* 7 data bits + software parity */
	hwparity = 0;
#endif /* HWPARITY */
	parity = y;
#ifndef NOXFER
	ebqflg = (parity) ? 1 : 0;
#endif /* NOXFER */
#ifdef HWPARITY
    }
#endif /* HWPARITY */

    return(success = 1);

#ifndef NOFRILLS
case XYPROM:				/* SET PROMPT */
/*
  Note: xxstring not invoked here.  Instead, it is invoked every time the
  prompt is issued.  This allows the prompt string to contain variables
  that can change, like \v(dir), \v(time), etc.
*/
    sprintf(line,
	    "{%s}",                /* Default might have a trailing space */
	    inserver ? ikprompt : ckprompt
	    );
    if ((x = cmtxt("Program's command prompt",line,&s,NULL)) < 0)
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

#ifndef NOXFER
case XYRETR:				/* RETRY: per-packet retry limit */
    y = cmnum("Maximum retries per packet","10",10,&x,xxstring);
    if (x < 0) x = 0;
    if ((x = setnum(&maxtry,x,y,999)) < 0) return(x);
#ifdef COMMENT
    if (maxtry <= wslotr) {
	printf("?Retry limit must be greater than window size\n");
	return(success = 0);
    }
#endif /* COMMENT */
    if (rmsflg) {
	sstate = setgen('S', "403", ckitoa(maxtry), "");
	return((int) sstate);
    } else return(success = x);
#endif /* NOXFER */

#ifndef NOSERVER
case XYSERV:				/* SET SERVER items */
    if ((y = cmkey(srvtab,nsrvt,"","",xxstring)) < 0) return(y);
    switch (y) {
      case XYSERI:
	if ((y = cmnum("Number of seconds, or 0 for no idle timeout",
                       "0",10,&x,xxstring)) < 0)
	  return(y);
	if (x < 0)
	  x = 0;
	if ((y = cmcfm()) < 0)
	  return(y);
#ifndef OS2
	srvtim = 0;
#endif /* OS2 */
	srvidl = x;
	return(success = 1);
      case XYSERT:
	if ((y = cmnum("Interval for server NAKs, 0 = none",ckitoa(DSRVTIM),
		       10,&x, xxstring)) < 0)
	  return(y);
	if (x < 0) {
	    printf("\n?Specify a positive number, or 0 for no server NAKs\n");
	    return(0);
	}
	if ((y = cmcfm()) < 0) return(y);
	if (rmsflg) {
	    sstate = setgen('S', "404", ckitoa(x), "");
	    return((int) sstate);
	} else {
#ifndef OS2
	    srvidl = 0;
#endif /* OS2 */
	    srvtim = x;			/* Set the server timeout variable */
	    return(success = 1);
	}
      case XYSERD:			/* SERVER DISPLAY */
	return(success = seton(&srvdis)); /* ON or OFF... */

#ifndef NOSPL
      case XYSERP:			/* SERVER GET-PATH */
	return(parsdir(2));
#endif /* NOSPL */

      case XYSERL:			/* SERVER LOGIN */
	return(cklogin());

      case XYSERC:			/* SERVER CD-MESSAGE */
	x = rmsflg ?
	  cmkey(onoff,2,"","",xxstring) :
	  cmkey(cdmsg,3,"","",xxstring);
	if (x < 0)
	  return(x);
	if (x == 2) {			/* CD-MESSAGE FILE */
	    if ((x = cmtxt("Name of file","",&s,NULL)) < 0)
	      return(x);
	    if (!*s) {
		s = NULL;
		srvcdmsg = 0;
	    }
	    makestr(&cdmsgstr,s);
	    makelist(cdmsgstr,cdmsgfile,8);
	    return(success = 1);
	}
	if ((y = cmcfm()) < 0)		/* CD-MESSAGE ON/OFF */
	  return(y);
	if (rmsflg) {
	    sstate = setgen('S', "420", x ? "1" : "0", "");
	    return((int) sstate);
	} else {
	    if (x > 0)
	      srvcdmsg |= 1;
	    else
	      srvcdmsg &= 2;
	    return(success = 1);
	}
      case XYSERK:			/* SERVER KEEPALIVE */
	return(success = seton(&srvping)); /* ON or OFF... */

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

case XYBELL:				/* SET BELL */
    return( success = setbell() );

#ifdef OS2
case XYPRTY:
    return( success = setprty() );
#endif /* OS2 */

default:
    break;
}
#endif /* NOLOCAL */

switch (xx) {

/* SET SEND/RECEIVE protocol parameters. */

#ifndef NOXFER
case XYRECV:
case XYSEND:
    return(setsr(xx,rmsflg));
#endif /* NOXFER */

#ifndef NOLOCAL				/* Session log text/binary selection */
#ifdef OS2ORUNIX			/* UNIX needs it */
#define _XYSESS
#endif /* OS2ORUNIX */
#ifdef OSK				/* OS-9 too */
#define _XYSESS
#endif /* OSK */

#ifdef _XYSESS
case XYSESS:				/* SESSION-LOG */
    if ((x = cmkey(sfttab,nsfttab,"type of file",
#ifdef OS2
		   "binary",
#else /* OS2 */
		   "text",
#endif /* OS2 */
		   xxstring
		   )
	 ) < 0)
      return(x);
    if ((y = cmcfm()) < 0) return(y);
    sessft = x;
    return(success = 1);
#undef _XYSESS
#endif /* _XYSESS */

case XYSPEE:				/* SET SPEED */
    lp = line;
    sprintf(lp,"Transmission rate for %s in bits per second",ttname);

    zz = -1L;
    if ((x = cmkey(spdtab,nspd,line,"",xxstring)) < 0) {
	if (x == -3) printf("?value required\n");
#ifdef USETCSETSPEED
	/* In this case, any number can be tried */
	/* There's a parse error message but the request still goes thru */
	if (rdigits(atmbuf))
	  zz = atol(atmbuf);
	else
#endif /* USETCSETSPEED */
	return(x);
    }
    if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
    if (!local) {
	printf("?Sorry, you must SET LINE first\n");
	return(success = 0);
    } else if (network) {
	printf("\n?Speed cannot be set for network connections\n");
	return(success = 0);
    }

/*
  Note: This way of handling speeds is not 16-bit safe for speeds greater
  than 230400.  The argument to ttsspd() should have been a long.
*/
#ifdef USETCSETSPEED
    if (zz > -1L)
      x = zz / 10L;
#endif /* USETCSETSPEED */
      zz = (long) x * 10L;
    if (zz == 130L) zz = 134L;
    if (zz == 70L) zz = 75L;		/* (see spdtab[] definition) */
    if (ttsspd(x) < 0)  {		/* Call ttsspd with cps, not bps! */
	printf("?Unsupported line speed - %ld\n",zz);
	return(success = 0);
    } else {
#ifdef CK_TAPI
	if (!tttapi || tapipass)
	  speed = ttgspd();		/* Read it back */
	else
	  speed = zz;
#else /* CK_TAPI */
	speed = ttgspd();		/* Read it back */
#endif /* CK_TAPI */
	if (speed != zz)  {		/* Call ttsspd with cps, not bps! */
	    printf("?SET SPEED fails, speed is %ld\n",speed);
	    return(success = 0);
	}
	if (pflag && !cmdsrc()) {
	    if (speed == 8880)
	      printf("%s, 75/1200 bps\n",ttname);
	    else if (speed == 134)
	      printf("%s, 134.5 bps\n",ttname);
	    else
	      printf("%s, %ld bps\n",ttname,speed);
	}
	return(success = 1);
    }
#endif /* NOLOCAL */

#ifndef NOXFER
  case XYXFER:				/* SET TRANSFER */
    if ((y = cmkey(rmsflg ? rtstab : tstab, /* (or REMOTE SET TRANSFER) */
		   rmsflg ? nrts : nts,
		   "","character-set",xxstring)) < 0) return(y);
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
	    extern int s_cset, fcharset, axcset[];
	    tslevel = (y == TC_TRANSP) ? 0 : 1; /* transfer syntax level */
	    tcharset = y;		/* transfer character set */
	    if (s_cset == XMODE_A)	/* If SEND CHARACTER-SET is AUTO */
	      if (y > -1 && y <= MAXTCSETS)
		if (axcset[y] > -1 && axcset[y] > MAXFCSETS)
		  fcharset = axcset[y]; /* Auto-pick file charset */
	    setxlatype(tcharset,fcharset); /* Translation type */
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
#ifndef OS2
	if (inserver) {
	    printf("?Sorry, only Kermit protocol is available\n");
	    return(-9);
	}
#endif /* OS2 */
	return(setproto());
#endif /* CK_XYZ */

      case XYX_MOD:			/* Mode */
	if ((y = cmkey(xfrmtab,2,"","automatic",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);
	if (rmsflg) {
	    sstate = setgen('S', "410", y == XMODE_A ? "0" : "1", "");
	    return((int)sstate);
	}
	g_xfermode = y;
	xfermode = y;
	return(success = 1);

#ifndef NOLOCAL
      case XYX_DIS:			/* Display */
	return(doxdis());
#endif /* NOLOCAL */

      case XYX_SLO:			/* Slow-start */
        return(seton(&slostart));

#ifndef NOSPL
      case XYX_CRC:			/* CRC */
        return(seton(&docrc));
#endif /* NOSPL */

      case XYX_BEL:			/* Bell */
        return(seton(&xfrbel));

#ifdef PIPESEND
      case XYX_PIP:			/* Pipes */
#ifndef NOPUSH
	if (nopush) {
#endif /* NOPUSH */
	    printf("Sorry, access to pipes is disabled\n");
	    return(-9);
#ifndef NOPUSH
	} else
#endif /* NOPUSH */
	  return(seton(&usepipes));
#endif /* PIPESEND */

      case XYX_INT:			/* Interruption */
        return(seton(&xfrint));

      default:
	return(-2);
    }
#endif /* NOXFER */
}

switch (xx) {

#ifndef NOXMIT
  case XYXMIT:				/* SET TRANSMIT */
    return(setxmit());
#endif /* NOXMIT */

#ifndef NOXFER
#ifndef NOCSETS
  case XYUNCS:				/* UNKNOWN-CHARACTER-SET */
    if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    unkcs = y;
    return(success = 1);
#endif /* NOCSETS */
#endif /* NOXFER */

#ifndef NOPUSH
#ifdef UNIX
case XYWILD:				/* WILDCARD-EXPANSION */
    if ((y = cmkey(wildtab,2,"who expands wildcards","kermit",xxstring)) < 0)
      return(y);
    if ((z = cmkey(wdottab,
		   2,
		   "whether to match filenames that start with \".\"",
		   "/no-match-dot-files",
		   xxstring)
	 ) < 0)
      return(z);
    if ((x = cmcfm()) < 0) return(x);
    if (nopush) {
	if (y > 0) {
	    printf("Shell expansion is disabled\n");
	    return(success = 0);
	}
    }
    wildxpand = y;
    matchdot = z;
    return(success = 1);
#endif /* UNIX */
#endif /* NOPUSH */

#ifndef NOXFER
  case XYWIND:				/* WINDOW-SLOTS */
    if (protocol == PROTO_K) {
	y = cmnum("Window size for Kermit protocol, 1 to 32",
		  "1", 10, &x, xxstring);
	y = setnum(&z,x,y,MAXWS);	/* == 32 */
    }
#ifdef CK_XYZ
    else if (protocol == PROTO_Z) {
	y = cmnum("Window size for ZMODEM protocol, 0 to 65535",
		  "0", 10, &x, xxstring);
	y = setnum(&z,x,y,65535);
    }
#endif /* CK_XYZ */
    else {
	y = cmnum("Window size for current protocol",
		  "", 10, &x, xxstring);
	y = setnum(&z,x,y,65472);	/* Doesn't matter - we won't use it */
    }
    if (y < 0) return(y);
    if (protocol == PROTO_K) {
	if (z < 1)
	  z = 1;
    }
#ifdef CK_XYZ
    else if (protocol == PROTO_Z) {
        /* Zmodem windowing is closer to Kermit packet length */
        /* than Kermit window size.  If Window size is zero   */
        /* an end of frame and CRC is sent only at the end of */
        /* the file (default).  Otherwise, an End of Frame    */
        /* and CRC are sent after Window Size number of bytes */
        if (z < 0)			/* Disable windowing  */
            z = 0;
    } else {
	printf("?SET WINDOW does not apply to %s protocol\n",
	       ptab[protocol].p_name
	       );
    }
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
	sstate = setgen('S', "406", ckitoa(z), "");
	return((int) sstate);
    }
    wslotr = z;				/* Set requested window size here */
    ptab[protocol].winsize = wslotr;	/* and in protocol-specific table */
    if (protocol == PROTO_K) {		/* And for Kermit only... */
	swcapr = (wslotr > 1) ? 1 : 0;	/* set window bit in capas word */
	if (wslotr > 1) {		/* Window size > 1? */
	    y = adjpkl(urpsiz,wslotr,bigrbsiz); /* Maybe adjust packet size */
	    if (y != urpsiz) {		/* Did it change? */
		urpsiz = y;
		if (msgflg)
		  printf(
" Adjusting receive packet-length to %d for %d window slots\n",
			 urpsiz,
			 wslotr
			 );
	    }
	}
    }
    return(success = 1);
#endif /* NOXFER */
}

switch (xx) {

#ifndef NOSPL
  case XYOUTP:				/* OUTPUT command parameters */
    if ((y = cmkey(outptab,noutptab,"OUTPUT command parameter","pacing",
		   xxstring)) < 0)
      return(y);
    switch(y) {				/* Which parameter */
      case OUT_PAC:			/* PACING */
	y = cmnum("Milliseconds to pause between each OUTPUT character","100",
		  10,&x,xxstring);
	y = setnum(&z,x,y,16383);	/* Verify and get confirmation */
	if (y < 0) return(y);
	if (z < 0) z = 0;		/* (save some space) */
	pacing = z;
	return(success = 1);
      case OUT_ESC:			/* Special-escapes */
	return(seton(&outesc));
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

    switch (z) {
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
		if (x == -3) {
		    if ((x = cmcfm()) < 0) return(x);
		    break;
		}
		if (x == -2) {
		    if (p) { free(p); p = NULL; }
		    debug(F110,"SET CONTROL atmbuf",atmbuf,0);
		    if (!ckstrcmp(atmbuf,"all",3,0) ||
			!ckstrcmp(atmbuf,"al",2,0) ||
			!ckstrcmp(atmbuf,"a",1,0)) {
			if ((x = cmcfm()) < 0) /* Get confirmation */
			  return(x);
			if (z)
			  prefixing = PX_ALL;
#ifndef UNPREFIXZERO
			/* Set all values, but don't touch 0 */
			for (y = 1; y < 32; y++) ctlp[y] = (short) z;
#else
			/* Zero too */
			for (y = 0; y < 32; y++) ctlp[y] = (short) z;
#endif /* UNPREFIXZERO */
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
#ifndef UNPREFIXZERO
#ifdef OS2
			if (z == 0 && protocol != PROTO_K)
			  ctlp[0] = 0;
#endif /* OS2 */
#endif /* UNPREFIXZERO */
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
#ifdef UNPREFIXZERO
	    zz = 0;
#else
#ifndef OS2
	    zz = 1 - z;
#else
	    zz = 0;			/* Allow 0 (but only for Zmodem) */
#endif /* OS2 */
#endif /* UNPREFIXZERO */

	    /* printf("x = %d, y = %d, z = %d, zz = %d\n", x,y,z,zz); */

	    if ((y >  31 && y < 127) ||	/* A specific numeric value */
		(y > 159 && y < 255) ||	/* Check that it is a valid */
		(y < zz) ||		/* control code. */
		(y > 255)) {
		printf("?Values allowed are: %d-31, 127-159, 255\n",zz);
		if (p) free(p);
		return(-9);
	    }
	    x = y & 127;		/* Get 7-bit value */
	    if (z == 0) {		/* If they are saying it is safe... */
		if (((flow == FLO_XONX) && /* If flow control is Xon/Xoff */
		  (x == XON || x == XOFF)) /* XON & XOFF chars not safe. */
		 ) {
		    if (msgflg)
		      printf("Sorry, not while Xon/Xoff is in effect.\n");
		    if (p) free(p);
		    return(-9);
		}
#ifdef TNCODE
		else if (network && (ttnproto == NP_TELNET)
			 && (y == CR || (unsigned) y == (unsigned) 255)) {
		    if (msgflg)
		      printf("Sorry, not on a TELNET connection.\n");
		    if (p) free(p);
		    return(-9);
		}
#endif /* TNCODE */
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

#ifndef NOXFER
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
#endif /* NOXFER */

#ifndef NOSPL
  case XYALRM: {
#ifndef COMMENT
      int yy;
      long zz;
      zz = -1L;
      yy = x_ifnum;
      x_ifnum = 1;			/* Turn off internal complaints */
      y = cmnum("Seconds from now, or time of day as hh:mm:ss",
		"0" ,10, &x, xxstring);
      x_ifnum = yy;
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
      if (x < 0) {
	  printf("?Alarm time is in the past.\n");
	  return(-9);
      }
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

#ifndef NOXFER
case XYPROTO:
    return(setproto());
#endif /* NOXFER */

#ifdef CK_SPEED
case XYPREFIX: {
#ifdef COMMENT
    extern int clearrq;
#endif /* COMMENT */
    if ((z = cmkey(pfxtab, 4, "control-character prefixing option",
		   "", xxstring)) < 0)
      return(z);
    if ((x = cmcfm()) < 0) return(x);
    setprefix(z);
#ifdef COMMENT
    if (hints && (z == PX_ALL || z == PX_CAU) && clearrq) {
	printf("Hint: use SET CLEAR-CHANNEL OFF to disable negotiation of\n");
	printf("      SET PREFIXING NONE during file transfers on reliable\n");
	printf("      connections.\n");
    }
#endif /* COMMENT */
    return(success = 1);
  }
#endif /* CK_SPEED */

#ifndef NOSPL
case XYLOGIN:
   if ((z = cmkey(logintab, 3, "value for login script","userid",
		  xxstring)) < 0)
      return(z);
    x = cmdgquo();
    if (z == LOGI_PSW)
      cmdsquo(0);
    if ((y = cmtxt("text","", &s, NULL)) < 0) {
	cmdsquo(x);
	return(y);
    }
    cmdsquo(x);
#ifdef IKSD
    if (inserver)
        return(success = 0);
#endif /* IKSD */
    if ((int)strlen(s) > 63) {
	printf("Sorry, too long\n");
	return(-9);
    }
    s = brstrip(s);
    switch(z) {
      case LOGI_UID:
	ckstrncpy(uidbuf,s,UIDBUFLEN);
        sl_uid_saved = 0;
	break;
      case LOGI_PSW:
	ckstrncpy(pwbuf,s,PWBUFL);
        pwflg = 1;
#ifdef OS2
        pwcrypt = 1;
#else /* OS2 */
        pwcrypt = 0;
#endif /* OS2 */
	break;
      case LOGI_PRM:
	ckstrncpy(prmbuf,s,PWBUFL);
    }
    return(success = 1);
#endif /* NOSPL */
}

switch (xx) {

  case XYSTARTUP:
    if ((y = cmkey(ifdtab,2,"","discard",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    DeleteStartupFile = (y != 0) ? 0 : 1;
    return(success = 1);

case XYTMPDIR:
    x = cmdir("Name of temporary directory","",&s,xxstring);
    if (x == -3)
      s = "";
    else if (x < 0)
      return(x);
    if ((x = cmcfm()) < 0) return(x);
    makestr(&tempdir,s);
    return(tempdir ? 1 : 0);

#ifndef NOXFER
case XYDEST:				/* DESTINATION */
    return(setdest());
#endif /* NOXFER */

#ifndef NOPUSH
#ifndef NOFRILLS

/* Editor, Browser, and FTP Client */

case XYEDIT:				/* EDITOR */
#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
#ifdef CK_APC
    /* Don't let this be set remotely */
    if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH)
      return(success = 0);
#endif /* CK_APC */

#ifdef OS2ORUNIX
    {
	char *p = getenv("PATH");
	char *e;
	e = editor[0] ? (char *) editor : getenv("EDITOR");
	if (!e) e = "";
	if (p)
	  x = cmifip("Name of preferred editor",e,&s,&y,0,p,xxstring);
	else
	  x = cmifi("Full path of preferred editor",e,&s,&y,xxstring);
	if (x < 0 && x != -3)
	  return(x);
    }
#else
#ifdef VMS
    if ((y = cmtxt("DCL command for editing", "edit", &s, NULL)) < 0) {
	if (x != -3)
	  return(x);
    }
#else
    if ((x = cmifi("Full path of preferred editor","",&s,&y,xxstring)) < 0) {
	if (x != -3)
	  return(x);
    }
#endif /* VMS */
#endif /* OS2ORUNIX */
#ifdef VMS
    ckstrncpy(editor,s,CKMAXPATH);
    editopts[0] = NUL;
#else
    if (y != 0) {
	printf("?A single file please\n");
	return(-2);
    }
    strcpy(line,s);
    if ((x = cmtxt("editor command-line options","",&s,NULL)) < 0)
      return(x);
    ckstrncpy(tmpbuf,s,TMPBUFSIZ);
    if ((z = cmcfm()) < 0) return(z);
    if (line[0]) {
	zfnqfp(line,CKMAXPATH,editor);
	ckstrncpy(editopts,tmpbuf,128);
    } else {
        editor[0] = NUL;
        editopts[0] = NUL;
    }
#endif /* VMS */
    return(success = 1);

#ifdef BROWSER
case XYFTP:				/* SET FTP-CLIENT */
case XYBROWSE:				/* SET BROWSER */
    {
	char *p = getenv("PATH");
	char *app = (char *) browser, *opts = (char *) browsopts;
	extern char ftpapp[], ftpopts[];
	if (xx == XYFTP) {
	    app = (char *)ftpapp;
	    opts = (char *)ftpopts;
	}
#ifdef IKSD
        if (inserver) {
            printf("?Sorry, command disabled.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
#ifdef CK_APC
	/* Don't let this be set remotely */
	if (apcactive == APC_LOCAL ||
	    apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
#ifdef OS2ORUNIX
	if (p)
	  x = cmifip(xx == XYBROWSE ?
		     "Name of preferred browser" :
		     "Name of preferred ftp client",
#ifdef OS2
		     xx == XYFTP ? "ftp.exe" : ""
#else
		     xx == XYFTP ? "ftp" : ""
#endif /* OS2 */
		     ,&s,&y,0,p,xxstring
		     );
	else
	  x = cmifi(xx == XYBROWSE ?
		    "Full path of preferred browser" :
		    "Full path of preferred ftp client",
		    "",&s,&y,xxstring
		    );
	if (x < 0 && x != -3)
	  return(x);
#else
#ifdef VMS
	if ((x = cmtxt("DCL command to start your preferred Web browser",
		       "", &s, NULL)) < 0) {
	    if (x != -3)
	      return(x);
	}
#else
	if ((x = cmifi("Full path of preferred browser","",&s,&y,xxstring)
	     ) < 0) {
	    if (x != -3)
	      return(x);
	}
#endif /* VMS */
#endif /* OS2ORUNIX */
#ifdef VMS
	ckstrncpy(app,s,CKMAXPATH);
	*opts = NUL;
#else
	if (y != 0) {
	    printf("?A single file please\n");
	    return(-2);
	}
	strcpy(line,s);
	if ((x = cmtxt(xx == XYBROWSE ?
		       "browser command-line options" :
		       "ftp client command-line options",
		       "",&s,NULL)
	     ) < 0)
	  return(x);
	ckstrncpy(tmpbuf,s,TMPBUFSIZ);
	if ((z = cmcfm()) < 0) return(z);
	if (line[0]) {
	    zfnqfp(line,CKMAXPATH,app);
	    ckstrncpy(opts, tmpbuf, 128);
	} else {
	    *app = NUL;
	    *opts = NUL;
	}
#endif /* VMS */
	return(success = 1);
    }
#endif /* BROWSER */
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef CK_CTRLZ
  case XYEOF: {				/* SET EOF */
      extern int eofmethod; extern struct keytab eoftab[];
      if ((x = cmkey(eoftab,3,"end-of-file detection method","",
		     xxstring)) < 0)
	return(x);
      if ((y = cmcfm()) < 0)
	return(y);
      eofmethod = x;
      return(success = 1);
  }
#endif /* CK_CTRLZ */

#ifdef SESLIMIT
  case XYLIMIT: {            /* Session-Limit (length of session in seconds) */
      extern int seslimit;
      y = cmnum("Maximum length of session, seconds","0",10,&x,xxstring);
#ifdef IKSD
      if (inserver &&
#ifdef IKSDCONF
	  iksdcf
#else
	  1
#endif /* IKSDCONF */
	  ) {
          if ((z = cmcfm()) < 0)
	    return(z);
          printf("?Sorry, command disabled.\r\n");
          return(success = 0);
      }
#endif /* IKSD */

      return(setnum(&seslimit,x,y,86400));
  }
#endif /* SESLIMIT */

  case XYRELY: {			/* SET RELIABLE */
      extern int reliable, setreliable;
      if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
	return(x);
      if ((y = cmcfm()) < 0) return(y);
      reliable = x;
      setreliable = (x != SET_AUTO);
      debug(F101,"set reliable","",reliable);
      return(success = 1);
  }

#ifdef STREAMING
  case XYSTREAM: {			/* SET STREAMING */
      extern int streamrq;
      if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
	return(x);
      if ((y = cmcfm()) < 0) return(y);
      streamrq = x;
      return(success = 1);
  }
#endif /* STREAMING */

#ifdef CKSYSLOG
 case XYSYSL: {
     if ((x = cmkey(syslogtab,nsyslog,"","",xxstring)) < 0)
       return(x);
     if ((y = cmcfm()) < 0) return(y);
#ifdef IKSD
     if (inserver &&
#ifdef IKSDCONF
	 iksdcf
#else
	 1
#endif /* IKSDCONF */
          ) {
	 printf("?Sorry, command disabled.\n");
	 return(success = 0);
     }
#endif /* IKSD */
#ifdef CK_APC
     /* Don't let this be set remotely */
     if (apcactive == APC_LOCAL ||
         apcactive == APC_REMOTE && apcstatus != APC_UNCH)
       return(success = 0);
#endif /* CK_APC */
     ckxsyslog = x;
     return(success = 1);
}
#endif /* CKSYSLOG */

#ifdef TLOG
  case XYTLOG: {			/* SET TRANSACTION-LOG */
      extern int tlogsep;
      if ((x = cmkey(vbtab,nvb,"","verbose",xxstring)) < 0)
	return(x);
      if (x == 0) {
	  if ((y = cmtxt("field separator",",",&s,NULL)) < 0) return(y);
	  s = brstrip(s);
	  if (*s) {
	      if (s[1]) {
		  printf("?A single character, please.\n");
		  return(-9);
	      } else if ((*s >= '0' && *s <= '9') ||
		  (*s >= 'A' && *s <= 'Z') ||
		  (*s >= 'a' && *s <= 'z')) {
		  printf("?A non-alphanumeric character, please.\n");
		  return(-9);
	      } else
		tlogsep = *s;
	  }
      } else {
	  if ((y = cmcfm()) < 0) return(y);
      }
#ifdef IKSD
     if (inserver && isguest) {
	 printf("?Sorry, command disabled.\n");
	 return(success = 0);
     }
#endif /* IKSD */
#ifdef CK_APC
     /* Don't let this be set remotely */
     if (apcactive == APC_LOCAL ||
         apcactive == APC_REMOTE && apcstatus != APC_UNCH)
       return(success = 0);
#endif /* CK_APC */
      tlogfmt = x;
      return(success = 1);
  }
#endif /* TLOG */

  case XYCLEAR: {			/* SET CLEARCHANNEL */
      extern int clearrq;
      if ((x = cmkey(ooatab,3,"","automatic",xxstring)) < 0)
	return(x);
      if ((y = cmcfm()) < 0) return(y);
      clearrq = x;
      return(success = 1);
  }

#ifdef CK_AUTHENTICATION
  case XYAUTH: {			/* SET AUTHENTICATION */
#ifdef CK_KERBEROS
      int kv = 0;
      extern struct krb_op_data krb_op;
#endif /* CK_KERBEROS */
      char * p = NULL;
      if ((x = cmkey(setauth,nsetauth,"authentication type","",xxstring)) < 0)
	return(x);
      switch (x) {
#ifdef CK_KERBEROS
	case AUTH_KRB4: kv = 4; break;	/* Don't assume values are the same */
	case AUTH_KRB5: kv = 5; break;
#endif /* CK_KERBEROS */
#ifdef CK_SRP
      case AUTH_SRP: break;
#endif /* CK_SRP */
#ifdef CK_SSL
	case AUTH_SSL:
	case AUTH_TLS:
	  break;
#endif /* CK_SSL */
	default:
	  printf("?Authorization type not supported yet - \"%s\"\n",atmbuf);
	  return(-9);
      }
#ifdef IKSD
      if (inserver &&
#ifdef IKSDCONF
	  iksdcf
#else
	  1
#endif /* IKSDCONF */
          ) {
	  if ((y = cmcfm()) < 0) return(y);
	  printf("?Sorry, command disabled.\n");
	  return(success = 0);
      }
#endif /* IKSD */
#ifdef CK_APC
      /* Don't let this be set remotely */
      if (apcactive == APC_LOCAL ||
          apcactive == APC_REMOTE && apcstatus != APC_UNCH) {
	  if ((y = cmcfm()) < 0) return(y);
	  return(success = 0);
      }
#endif /* CK_APC */

      switch(x) {
#ifdef CK_KERBEROS
	case AUTH_KRB4:
	case AUTH_KRB5: {
	    if ((x = cmkey(kv == 4 ? k4tab : k5tab,
			   kv == 4 ? nk4tab : nk5tab,
			   "Kerberos parameter","",xxstring)) < 0) {
		return(x);
	    }
	    s = "";
	    switch (x) {
#ifdef KRB4
	      case XYKRBDBG:
		if (kv == 4) {
		    if ((y = seton(&k4debug)) < 0)
		      return(y);
#ifdef NT
		    ck_krb4_debug(k4debug);
#endif /* NT */
		} else {
		    return(-9);
		}
		break;
#endif /* KRB4 */
	      case XYKRBLIF:
		if ((y = cmnum("TGT lifetime","600",10,&z,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBPRE:
		if (kv == 4) {
		    if ((y = seton(&krb4_d_preauth)) < 0)
		      return(y);
		} else {
		    return(-9);
		}
		break;
	      case XYKRBINS:
		if ((y = cmtxt("Instance name","",&s,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBFWD:
		if (kv == 5) {
		    if ((y = seton(&krb5_d_forwardable)) < 0)
		      return(y);
		} else {
		    return(-9);
		}
		break;
	      case XYKRBPRX:
		if (kv == 5) {
		    if ((y = seton(&krb5_d_proxiable)) < 0)
		      return(y);
		} else {
		    return(-9);
		}
		break;
	      case XYKRBRNW:
		if ((y = cmnum("TGT renewable lifetime",
			       "0",10,&z,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBADR:
		if (kv == 5) {
		    if ((y = seton(&krb5_checkaddrs)) < 0)
		      return(y);
		} else {
		    if ((y = seton(&krb4_checkaddrs)) < 0)
		      return(y);
		}
		break;
	      case XYKRBGET:
		if (kv == 5) {
		    if ((y = seton(&krb5_autoget)) < 0)
		      return(y);
		} else {
		    if ((y = seton(&krb4_autoget)) < 0)
		      return(y);
		}
		break;
	      case XYKRBDEL:
		if ((z = cmkey(kdestab,nkdestab,
			       "Auto Destroy Tickets","never",xxstring)) < 0)
                  return(z);
		break;
	      case XYKRBPR:
		if ((y = cmtxt("User ID",uidbuf,&s,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBRL:
		if ((y = cmtxt("Name of realm","",&s,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBCC:
		if ((y = cmofi("Filename","",&s,xxstring)) < 0)
		  return(y);
		break;
	      case XYKRBSRV:
		if ((y = cmtxt("Name of service to use in ticket",
			       (kv == 4 ? "rcmd" : "host"),
			       &s,
			       xxstring
			       )) < 0)
		  return(y);
		break;
	      case XYKRBK5K4:
		if (kv == 5) {
		    if ((y = seton(&krb5_d_getk4)) < 0)
		      return(y);
		} else {
		    return(-9);
		}
		break;
	      case XYKRBPRM:		/* Prompt */
		if ((z = cmkey(krbprmtab,2,"","",xxstring)) < 0)
		  return(z);
		if ((y = cmtxt((z == KRB_PW_PRM) ?
  "Text of prompt;\nmay contain \"%s\" to be replaced by principal name" :
  "Text of prompt",
			       "",
			       &s,
			       xxstring
			       )
		     ) < 0)
		  return(y);
		break;
	    }
	    strcpy(line,s);
	    s = line;
	    if ((y = cmcfm()) < 0)
	      return(y);
#ifdef IKSD
            if (inserver &&
#ifdef IKSDCONF
		iksdcf
#else /* IKSDCONF */
		1
#endif /* IKSDCONF */
		)
	      return(success = 0);
#endif /* IKSD */

	    switch (x) {		/* Copy value to right place */
	      case XYKRBLIF:		/* Lifetime */
		if (kv == 4)
		  krb4_d_lifetime = z;
		else
		  krb5_d_lifetime = z;
		break;
	      case XYKRBRNW:
		if (kv == 5)
		  krb5_d_renewable = z;
		break;
	      case XYKRBPR:		/* Principal */
		s = brstrip(s);	        /* Strip braces around. */
		if (kv == 4)
		  makestr(&krb4_d_principal,s);
		else
		  makestr(&krb5_d_principal,s);
		break;
	      case XYKRBINS:		/* Instance */
		if (kv == 4)
		  makestr(&krb4_d_instance,s);
                else
                  makestr(&krb5_d_instance,s);
		break;
	      case XYKRBRL:		/* Realm */
		if (kv == 4)
		  makestr(&krb4_d_realm,s);
		else
		  makestr(&krb5_d_realm,s);
		break;
	      case XYKRBCC:		/* Credentials cache */
		makestr(&krb5_d_cc,s);
		break;
	      case XYKRBSRV:		/* Service Name */
		if (kv == 4)
		  makestr(&krb4_d_srv,s);
		else
		  makestr(&krb5_d_srv,s);
		break;
	      case XYKRBDEL:
		if (kv == 5)
		  krb5_autodel = z;
		else
		  krb4_autodel = z;
		break;
	      case XYKRBPRM:		/* Prompt */
		if (s == "") s = NULL;
		if (s) {
		    s = brstrip(s);
		    if (s == "") s = NULL;
		}
		switch (z) {
		  case KRB_PW_PRM: {	/* Password */
		      /* Check that there are no more than */
		      /* two % fields and % must followed by 's'. */
		      int i,n,len;
		      len = strlen(s);
		      for (i = 0, n = 0; i < len; i++) {
			  if (s[i] == '%') {
			      if (s[i+1] != '%') {
				  if (s[i+1] != 's') {
				      printf(
				       "Only %%s fields are permitted.\n");
				      return(-9);
				  }
				  if (++n > 2) {
				      printf(
				       "Only two %%s fields are permitted.\n");
				      return(-9);
				  }
			      }
			      i++;
			  }
		      }
		      if (kv == 5)
			makestr(&k5pwprompt,s);
		      else
			makestr(&k4pwprompt,s);
		      break;
		  }
		  case KRB_PR_PRM: {	/* Principal */
		      /* Check to make sure there are no % fields */
		      int i,len;
		      len = strlen(s);
		      for (i = 0; i < len; i++) {
			  if (s[i] == '%') {
			      if (s[i+1] != '%') {
				  printf(
				  "%% fields are not used in this command.\n");
				  return(-9);
			      }
			      i++;
			  }
		      }
		      if (kv == 5)
			makestr(&k5prprompt,s);
		      else
			makestr(&k4prprompt,s);
		      break;
		  }
		}
	    }
	    break;
	}
#endif /* CK_KERBEROS */
#ifdef CK_SRP
	case AUTH_SRP: {
	    if ((x = cmkey(srptab, nsrptab,
			   "SRP parameter","",xxstring)) < 0) {
		return(x);
	    }
	    s = "";
	    switch (x) {
	      case XYSRPPRM:		/* Prompt */
		if ((z = cmkey(srpprmtab,1,"","",xxstring)) < 0)
		  return(z);
		if ((y = cmtxt(
  "Text of prompt;\nmay contain one \"%s\" to be replaced by the username",
			       "",
			       &s,
			       xxstring
			       )
		     ) < 0)
		  return(y);
		break;
	    }
	    strcpy(line,s);
	    s = line;
	    if ((y = cmcfm()) < 0)
	      return(y);
	    switch (x) {		/* Copy value to right place */
	      case XYSRPPRM:		/* Prompt */
		if (s == "") s = NULL;
		if (s) {
		    s = brstrip(s);
		    if (s == "") s = NULL;
		}
		switch (z) {
		  case SRP_PW_PRM: {	/* Password */
		      /* Check %s fields */
		      int i,n,len;
		      len = strlen(s);
		      for (i = 0, n = 0; i < len; i++) {
			  if (s[i] == '%') {
			      if (s[i+1] != '%') {
				  if (s[i+1] != 's') {
				      printf(
				       "Only %%s fields are permitted.\n");
				      return(-9);
				  }
				  if (++n > 1) {
				      printf(
				       "Only one %%s field is permitted.\n");
				      return(-9);
				  }
			      }
			      i++;
			  }
		      }
		      makestr(&srppwprompt,s);
		      break;
		  }
		}
	    }
	    break;
	}
#endif /* CK_SRP */
#ifdef CK_SSL
	case AUTH_SSL:
	case AUTH_TLS: {
	    if ((z = cmkey(ssltab, nssltab,
			   (x == AUTH_SSL ? "SSL parameter" : "TLS parameter"),
			   "",xxstring)) < 0)
	      return(z);
	    s = "";
	    switch (z) {
	      case XYSSLRCFL:		/* SSL/TLS RSA Certs file */
	      case XYSSLRKFL:		/* SSL/TLS RSA Key File */
	      case XYSSLDCFL:		/* SSL/TLS DSA Certs file */
              case XYSSLDKFL:		/* SSL/TLS DH Key File */
              case XYSSLDPFL:           /* SSL/TLS DH Param File */
              case XYSSLCRL:            /* SSL/TLS CRL File */
              case XYSSLVRFF:           /* SSL/TLS Verify File */
		if ((y = cmifi("Filename","",&s,&x,xxstring)) < 0)
		  return(y);
		if (x) {
		    printf("?Wildcards not allowed\n");
		    return(-9);
		}
		strcpy(line,s);
		s = line;
		if ((y = cmcfm()) < 0)
		  return(y);
		switch (z) {
		  case XYSSLRCFL:	/* SSL/TLS RSA Certs file */
		    if (!s[0] && ssl_rsa_cert_file) {
			free(ssl_rsa_cert_file);
			ssl_rsa_cert_file = NULL;
		    } else {
			makestr(&ssl_rsa_cert_file,s);
			if (!ssl_rsa_key_file)
			  makestr(&ssl_rsa_key_file,s);
		    }
		    break;
		  case XYSSLRKFL:	/* SSL/TLS RSA Key File */
		    if (!s[0] && ssl_rsa_key_file) {
			free(ssl_rsa_key_file);
			ssl_rsa_key_file = NULL;
		    } else {
			makestr(&ssl_rsa_key_file,s);
		    }
		    break;
		  case XYSSLDCFL:	/* SSL/TLS DSA Certs file */
                    if (!s[0] && ssl_dsa_cert_file) {
                        free(ssl_dsa_cert_file);
                        ssl_dsa_cert_file = NULL;
                    } else {
                        makestr(&ssl_dsa_cert_file,s);
                        if (!ssl_dh_key_file)
			  makestr(&ssl_dh_key_file,s);
                    }
                    break;
		  case XYSSLDKFL:	/* SSL/TLS DH Key File */
                    if (!s[0] && ssl_dh_key_file) {
                        free(ssl_dh_key_file);
                        ssl_dh_key_file = NULL;
                    } else {
                        makestr(&ssl_dh_key_file,s);
                    }
                    break;
                  case XYSSLDPFL:	/* SSL/TLS DH Param File */
		    if (!s[0] && ssl_dh_param_file) {
		      free(ssl_dh_param_file);
		      ssl_dh_param_file = NULL;
		    } else {
		      makestr(&ssl_dh_param_file,s);
		    }
		    break;
                  case XYSSLCRL:	/* SSL/TLS CRL File */
                      if (!s[0] && ssl_crl_file) {
                          free(ssl_crl_file);
                          ssl_crl_file = NULL;
                      } else {
                          makestr(&ssl_crl_file,s);
                      }
                      break;
                  case XYSSLVRFF:	/* SSL/TLS Verify File */
                      if (!s[0] && ssl_crl_file) {
                          free(ssl_verify_file);
                          ssl_verify_file = NULL;
                      } else {
                          makestr(&ssl_verify_file,s);
                      }
                      break;
		}
		break;

              case XYSSLCRLD:
              case XYSSLVRFD:
		if ((y = cmdir("Directory","",&s,xxstring)) < 0)
		  return(y);
		strcpy(line,s);
		s = line;
		if ((y = cmcfm()) < 0)
		  return(y);
                switch(z) {
		  case XYSSLCRLD:
		    if (!s[0] && ssl_crl_dir) {
			free(ssl_crl_dir);
			ssl_crl_dir = NULL;
		    } else {
			makestr(&ssl_crl_dir,s);
		    }
		    break;
		  case XYSSLVRFD:
		    if (!s[0] && ssl_verify_dir) {
			free(ssl_verify_dir);
			ssl_verify_dir = NULL;
		    } else {
			makestr(&ssl_verify_dir,s);
		    }
		    break;
                }
		break;
	      case XYSSLCOK:		/* SSL/TLS Certs-Ok flag */
		if ((y = seton(&ssl_certsok_flag)) < 0)
		  return(y);
		break;
	      case XYSSLDBG:		/* SSL/TLS Debug flag */
		if ((y = seton(&ssl_debug_flag)) < 0)
		  return(y);
		break;
	      case XYSSLON:		/* SSL/TLS Only flag */
		switch (x) {
		  case AUTH_SSL:
		    if ((y = seton(&ssl_only_flag)) < 0)
		      return(y);
		    break;
		  case AUTH_TLS:
		    if ((y = seton(&tls_only_flag)) < 0)
		      return(y);
		    break;
		}
		break;
	      case XYSSLVRB:		/* SSL/TLS Verbose flag */
		if ((y = seton(&ssl_verbose_flag)) < 0)
		  return(y);
		break;
	      case XYSSLVRF:		/* SSL/TLS Verify flag */
		if ((x = cmkey(sslvertab, nsslvertab,
			       "SSL/TLS verify mode",
			       "peer-cert",xxstring)) < 0)
		  return(x);
		if ((y = cmcfm()) < 0)
		  return(y);
		ssl_verify_flag = x;
		break;
	      case XYSSLDUM:
		if ((y = seton(&ssl_dummy_flag)) < 0)
		  return(y);
		break;
	      case XYSSLCL: {		/* SSL/TLS Cipher List */
#ifdef COMMENT
		  /* This code is used to generate a colon delimited */
		  /* list of the ciphers currently in use to be used */
		  /* as the default for cmtxt().  However, a better  */
		  /* default is simply the magic keyword "ALL".      */
		  CHAR def[1024] = "";
		  if (ssl_con != NULL) {
		      CHAR * p = NULL, *q = def;
		      int i, len;

		      for (i = 0; ; i++) {
			  p = (CHAR *) SSL_get_cipher_list(ssl_con,i);
			  if (p == NULL)
			    break;
			  len = strlen(p);
			  if (q+len+1 >= def+1024)
			    break;
			  if (i != 0)
			    *q++ = ':';
			  strcpy(q,p);
			  q += len;
		      }
		  }
#endif /* COMMENT */
                  char * p = getenv("SSL_CIPHER");
                  if (!p)
		    p = "ALL";
		  if ((y = cmtxt(
                    "Colon-delimited list of ciphers or ALL (case sensitive)",
				 p,
				 &s,
				 xxstring
				 )
		       ) < 0)
		    return(y);
		  makestr(&ssl_cipher_list,s);
		  if (ssl_con == NULL) {
		      SSL_library_init();
		      ssl_ctx = (SSL_CTX *)
			SSL_CTX_new((SSL_METHOD *)TLSv1_method());
		      if (ssl_ctx != NULL)
			ssl_con= (SSL *) SSL_new(ssl_ctx);
		  }
		  if (ssl_con) {
		      SSL_set_cipher_list(ssl_con,ssl_cipher_list);
		  }
		  break;
	      }
	    }
	    break;
	}
#endif /* CK_SSL */
	default:
          break;
      }
      return(success = 1);
  }
#endif /* CK_AUTHENTICATION */

#ifndef NOSPL
case XYFUNC:
     if ((x = cmkey(functab,nfunctab,"","diagnostics",xxstring)) < 0)
       return(x);
     switch (x) {
       case FUNC_DI: return(seton(&fndiags));
       case FUNC_ER: return(seton(&fnerror));
       default:      return(-2);
     }
#endif /* NOSPL */

case XYSLEEP:				/* SET SLEEP / PAUSE */
    if ((x = cmkey(sleeptab,1,"","cancellation",xxstring)) < 0)
      return(x);
    return(seton(&sleepcan));

case XYCD:				/* SET CD */
    if ((x = cmkey(cdtab,ncdtab,"","",xxstring)) < 0)
      return(x);
    switch (x) {
      case XYCD_M:			/* SET CD MESSAGE */
	if ((x = cmkey(cdmsg,ncdmsg,"","",xxstring)) < 0)
	  return(x);
	if (x == 2) {			/* CD MESSAGE FILE */
	    if ((x = cmtxt("Name of file","",&s,NULL)) < 0)
	      return(x);
	    if (!*s) {
		s = NULL;
#ifndef NOXFER
		srvcdmsg = 0;
#endif /* NOXFER */
	    }
	    makestr(&cdmsgstr,s);
	    makelist(cdmsgstr,cdmsgfile,8);
	    return(success = 1);
	}

	if ((y = cmcfm()) < 0) return(y); /* CD-MESSAGE ON/OFF */
#ifndef NOXFER
	if (x > 0)
	  srvcdmsg |= 2;
	else
	  srvcdmsg &= 1;
#endif /* NOXFER */
	return(success = 1);

      case XYCD_P: {			/* SET CD PATH */
	  extern char * ckcdpath;
	  if ((x = cmtxt("CD PATH string","",&s,xxstring)) < 0)
	    return(x);
	  makestr(&ckcdpath,s);
	  return(success = 1);
      }
    }

#ifndef NOLOCAL
#ifdef HWPARITY
    case XYSTOP:			/* STOP-BITS */
        if ((x = cmkey(stoptbl,2,"Stop bits for serial device","",
		       xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	if (x > 0 && x < 3) {
	    stopbits = x;
	    return(success = 1);
	} else
	  return(-2);
#endif /* HWPARITY */

      case XYDISC: {
	  extern int clsondisc;
	  return(seton(&clsondisc));
      }

      case XYSERIAL: {
	  /* char c; */
	  extern int cmask;
	  if ((x = cmkey(sertbl,nsertbl,
			 "Serial device character size, parity, and stop bits",
			 "8N1", xxstring)) < 0)
	    return(x);
	  strcpy(line,atmbuf);		/* Copy associated keyword string */
	  s = line;
	  if ((y = cmcfm()) < 0)
	    return(y);
	  strcpy(line,sernam[x]);
	  s = line;
	  if (s[0] != '8' && s[0] != '7') /* Char size */
	    return(-2);
	  else
	    z = s[0] - '0';
	  if (isupper(s[1]))		/* Parity */
	    s[1] = tolower(s[1]);
	  if (s[2] != '1' && s[2] != '2') /* Stop bits */
	    return(-2);
	  else
	    stopbits = s[2] - '0';
	  if (z == 8) {			/* 8 bits + parity (or not) */
	      parity = 0;		/* Set parity */
	      hwparity = (s[1] == 'n') ? 0 : s[1];
	      cmask = 0xff;		/* Also set TERM BYTESIZE to 8 */
	  } else {			/* 7 bits plus parity */
	      parity = (s[1] == 'n') ? 0 : s[1];
	      hwparity = 0;
	      cmask = 0x7f;		/* Also set TERM BYTESIZE to 7 */
	  }
	  return(success = 1);
      }

      case XYOPTS: {			/* SET OPTIONS */
	  extern int setdiropts();
	  extern int settypopts();
#ifdef CKPURGE
	  extern int setpurgopts();
#endif /* CKPURGE */
	  if ((x = cmkey(optstab,noptstab,"for command","", xxstring)) < 0)
	    return(x);
	  switch (x) {
#ifndef NOFRILLS
	    case XXDEL:
	      return(setdelopts());
#endif /* NOFRILLS */
	    case XXDIR:
	      return(setdiropts());
	    case XXTYP:
	      return(settypopts());
#ifdef CKPURGE
	    case XXPURGE:
	      return(setpurgopts());
#endif /* CKPURGE */
	    default:
	      return(-2);
	  }
      }
#endif /* NOLOCAL */
#ifndef NOXFER
      case XYQ8FLG: {
	  extern int q8flag;
	  return(seton(&q8flag));
      }
      case XYTIMER: {
	  extern int asktimer;
	  y = cmnum("Time limit for ASK command, seconds","0",10,&x,xxstring);
#ifdef QNX16
	  return(setnum(&asktimer,x,y,32767));
#else
	  return(setnum(&asktimer,x,y,86400));
#endif /* QNX16 */
      }
      case XYFACKB: {
	  extern int fackbug;
	  return(seton(&fackbug));
      }
#endif /* NOXFER */

      case XYHINTS:
        return(seton(&hints));

#ifndef NOSPL
      case XYEVAL: {
	  extern int oldeval;
	  if ((x = cmkey(oldnew,2,"","", xxstring)) < 0)
	    return(x);
	  if ((y = cmcfm()) < 0)
	    return(y);
	  oldeval = x;
	  return(success = 1);
      }
#endif /* NOSPL */

#ifndef NOXFER
      case XYFACKP: {
	  extern int fackpath;
	  return(seton(&fackpath));
      }
#endif /* NOXFER */

      case XYQNXPL: {
	  extern int qnxportlock;
	  return(seton(&qnxportlock));
      }

      default:
	 if ((x = cmcfm()) < 0) return(x);
	 printf("Not implemented - %s\n",cmdbuf);
	 return(success = 0);
    }
}

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

    if (batch)				/* No warnings in batch */
      return(1);
#else
#ifdef UNIX
    if (backgrd)			/* No warnings in background */
      return(1);
#endif /* UNIX */
#endif /* VMS */

    debug(F101,"hupok local","",local);

    if (!local)				/* No warnings in remote mode */
      return(1);

    debug(F101,"hupok x","",x);
    debug(F101,"hupok xitwarn","",xitwarn);
    debug(F101,"hupok network","",network);
    debug(F101,"hupok haveline","",haveline);

    if ((local && xitwarn) ||		/* Is a connection open? */
        (!x && xitwarn == 2)) {		/* Or Always give warning on EXIT */
	int needwarn = 0;

	if (network) {
	    if (ttchk() >= 0)
	      needwarn = 1;
	    /* A connection seems to be open but it can't possibly be */
	    if (!haveline)
	      needwarn = 0;
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
	      needwarn = (ttchk() >= 0);
	    /* A connection seems to be open but it can't possibly be */
	    if (!haveline)
	      needwarn = 0;
	    if (needwarn)
	      printf(
		     " A serial connection might still be active on %s.\n",
		     ttname
		     );
	}

/* If a warning was issued, get user's permission to EXIT. */

	if (needwarn || !x && xitwarn == 2 && local) {
	    z = getyesno(x ? "OK to close? " : "OK to exit? ",0);
	    debug(F101,"hupok getyesno","",z);
	    if (z < -3) z = 0;
	}
    }
    return(z);
}

#ifndef NOSHOW
VOID
shoctl() {				/* SHOW CONTROL-PREFIXING */
#ifdef CK_SPEED
    int i;
#ifdef OS2
    int zero;
#endif /* OS2 */
    printf(
"\ncontrol quote = %d, applied to (0 = unprefixed, 1 = prefixed):\n\n",
	   myctlq);
#ifdef OS2
#ifndef UNPREFIXZERO
    zero = ctlp[0];
    if (protocol == PROTO_K)		/* Zero can't be unprefixed */
      ctlp[0] = 1;			/* for Kermit */
#endif /* UNPREFIXZERO */
#endif /* OS2 */
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
#ifndef UNPREFIXZERO
#ifdef OS2
    ctlp[0] = zero;
#endif /* OS2 */
#endif /* UNPREFIXZERO */

#endif /* CK_SPEED */
}

#ifndef NOXFER
VOID
shodbl() {				/* SHOW DOUBLE/IGNORE */
#ifdef CKXXCHAR
    int i, n = 0;
    printf("\nSET SEND DOUBLE characters:\n");
    for (i = 0; i < 255; i++) {
	if (dblt[i] & 2) {
	    n++;
	    printf(" %d", i);
	}
    }
    if (n == 0)
      printf(" (none)");
    n = 0;
    printf("\nSET RECEIVE IGNORE characters:\n");
    for (i = 0; i < 255; i++) {
	if (dblt[i] & 1) {
	    n++;
	    printf(" %d", i);
	}
    }
    if (n == 0)
      printf(" (none)");
    printf("\n\n");
#endif /* CKXXCHAR */
}
#endif /* NOXFER */
#endif /* NOSHOW */

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

#ifdef IKSD
    if (inserver) {
        printf("?Sorry, command disabled.\r\n");
        return(success = 0);
    }
#endif /* IKSD */
#ifdef CK_APC
    /* Don't let this be set remotely */
    if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH)
      return(success = 0);
#endif /* CK_APC */

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
