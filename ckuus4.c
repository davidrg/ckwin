#include "ckcsym.h"

/*  C K U U S 4 --  "User Interface" for C-Kermit, part 4  */

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

/*
  File ckuus4.c -- Functions moved from other ckuus*.c modules to even
  out their sizes.
*/
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckuver.h"
#include "ckcnet.h"			/* Network symbols */

#ifdef VMS
#include <errno.h>			/* For \v(errno) */
#ifndef OLD_VMS
#include <lib$routines.h>		/* Not for VAX C 2.4 */
#else
#include <libdef.h>
#endif /* OLD_VMS */
_PROTOTYP(int vmsttyfd, (void) );
#endif /* VMS */

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h> 
#else 
#include <windows.h>
#define APIRET ULONG
#endif /* NT */
#include "ckocon.h"
#include "ckoetc.h"
int StartedFromDialer = 0;
HWND hwndDialer = 0;
LONG KermitDialerID = 0;
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#endif /* OS2 */

extern xx_strp xxstring;

#ifdef DEC_TCPIP
#include <descrip>
#include <dvidef>
#include <dcdef>
#endif /* DEC_TCPIP */

#include "ckcxla.h"			/* Character sets */
#ifdef CKOUNI
#include "ckouni.h"
#endif /* CKOUNI */

extern int quiet, network, xitsta, escape;
#ifndef MAC
#ifndef AMIGA
extern int ttyfd;
#endif /* MAC */
#endif /* AMIGA */

#ifdef NETCONN
extern int tn_exit;
#endif /* NETCONN */

#ifndef NOICP				/* Most of this file... */

#ifndef AMIGA
#ifndef MAC
#include <signal.h>
#endif /* MAC */
#endif /* AMIGA */

#ifdef STRATUS				/* Stratus Computer, Inc.  VOS */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#ifdef getchar
#undef getchar
#endif /* getchar */
#define getchar(x) coninc(0)
#endif /* STRATUS */

#ifdef ANYX25
extern int revcall, closgr, cudata, npadx3;
int x25ver;
extern char udata[];
extern CHAR padparms[];
extern struct keytab padx3tab[];
#endif /* ANYX25 */

#ifdef NETCONN
#ifndef NODIAL
extern int nnetdir;
extern char *netdir[];
#endif /* NODIAL */
extern char ipaddr[];
#ifdef TNCODE
_PROTOTYP (static VOID shotel, (void) );
extern int tn_duplex, tn_nlm, tn_binary, tn_b_nlm, u_binary, me_binary;
extern int tn_b_meu, tn_b_ume;
extern char *tn_term;
#endif /* TNCODE */

#ifdef CK_NETBIOS
extern unsigned short netbiosAvail;
extern unsigned long NetbeuiAPI;
extern unsigned char NetBiosName[];
extern unsigned char NetBiosAdapter;
extern unsigned char NetBiosLSN;
#endif /* CK_NETBIOS */

#ifdef TCPSOCKET
extern char myipaddr[];
#ifdef SOL_SOCKET
#ifdef SO_LINGER
extern int tcp_linger;
extern int tcp_linger_tmo;
#endif /* SO_LINGER */
#ifdef TCP_NODELAY
extern int tcp_nodelay;
#endif /* TCP_NODELAY */
#ifdef SO_SNDBUF
extern int tcp_sendbuf;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
extern int tcp_recvbuf;
#endif /* SO_RCVBUF */
#ifdef SO_KEEPALIVE
extern int tcp_keepalive;
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */
#endif /* TCPSOCKET */
#endif /* NETCONN */

extern int cfilef;
extern char cmdfil[];

#ifndef NOSPL
#ifdef CK_APC
extern int apcactive;			/* Nonzero = APC command was rec'd */
extern int apcstatus;			/* Are APC commands being processed? */
#ifdef DCMDBUF
extern char *apcbuf;			/* APC command buffer */
#else
extern char apcbuf[];
#endif /* DCMDBUF */
#endif /* CK_APC */

extern char evalbuf[];			/* EVALUATE result */
extern char uidbuf[], pwbuf[], prmbuf[];
_PROTOTYP( static char * fneval, (char *, char * [], int, char * ) );
_PROTOTYP( static VOID myflsh, (void) );
_PROTOTYP( static char * getip, (char *) );

static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};
extern char * tempdir;

#ifdef CK_REXX
extern char rexxbuf[];
#endif /* CK_REXX */

extern int tfline[];

/* These need to be internationalized... */

static
char *wkdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
#endif /* NOSPL */

static
char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#ifdef OS2
_PROTOTYP (int os2getcp, (void) );
#ifdef TCPSOCKET
extern char tcpname[];
#endif /* TCPSOCKET */
extern char startupdir[],exedir[];
extern int tcp_avail;
#ifdef DECNET
extern int dnet_avail;
#endif /* DECNET */
#ifdef SUPERLAT
extern int slat_avail;
#endif /* SUPERLAT */

extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */

extern struct keytab colxtab[];

extern CHAR
  eol, feol, mypadc, mystch, padch, seol, stchr;

extern char ttname[], *ckxsys, *versio, **xargv, *zinptr;
extern char inidir[], *cksysid;

extern int activecmd, remonly, cmd_rows;

extern struct ck_p ptab[];
extern int protocol, prefixing;

extern int
  atcapr, autopar, bctr, bctu, bgset, bigrbsiz, bigsbsiz, binary, carrier,
  cdtimo, cmask, crunched, delay, duplex, ebq, ebqflg, flow, fmask,
  fncact, fncnv, inecho, keep, local, lscapr, lscapu, xfermode,
  maxrps, maxsps, maxtry, mypadn, ncolx, dest, slostart,
  nettype, nmac, noinit, npad, parity, pktlog, pkttim, rcflag,
  retrans, rpackets, rptflg, rptq, rtimo, seslog, sessft, sosi, spackets,
  spsiz, spsizf, spsizr, srvtim, stayflg, success, timeouts, tralog,
  tsecs, ttnproto, turn, turnch, urpsiz, wmax, wslotn, wslotr, xargc, xargs,
  zincnt, fdispla, tlevel, spmax, insilence, cmdmsk, timint, timef,
  fnrpath, fnspath, inbufsize;

#ifdef VMS
  extern int frecl;
#endif /* VMS */

extern long
  ffc, filcnt, rptn, speed, tfc, tlci, tlco, ccu, ccp, vernum, xvernum, crc16;

#ifndef NOSPL
extern char fspec[], myhost[];
#endif /* NOSPL */

extern char *tfnam[];			/* Command file names */
#ifdef CK_TMPDIR
extern char *dldir;
#endif /* CK_TMPDIR */

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;
extern char *line, *tmpbuf, *kermrc;
#else
extern struct cmdptr cmdstk[];
extern char line[], tmpbuf[], kermrcb[], *kermrc;
#endif /* DCMDBUF */

extern char pktfil[],			/* Packet log file name */
#ifdef DEBUG
  debfil[],				/* Debug log file name */
#endif /* DEBUG */
#ifdef TLOG
  trafil[],				/* Transaction log file name */
#endif /* TLOG */
  sesfil[];				/* Session log file name */

#ifndef NOXMIT				/* TRANSMIT command variables */
extern char xmitbuf[];
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw;
#endif /* NOXMIT */

#ifndef NOSPL
/* Script programming language items */
extern char **a_ptr[];			/* Arrays */
extern int a_dim[];
extern char * inpbuf, inchar[];		/* Buffers for INPUT and REINPUT */
extern char *inpbp;			/* And pointer to same */
#ifdef COMMENT
static char *inpbps = inpbuf;		/* And another */
#endif /* COMMENT */
static char *r3 = (char *)0;
extern int incount;			/* INPUT character count */
extern int m_found;			/* MINPUT result */
extern int maclvl;			/* Macro invocation level */
extern struct mtab *mactab;		/* Macro table */
extern char *mrval[];
extern int macargc[], cmdlvl;
extern char *m_arg[MACLEVEL][10]; /* You have to put in the dimensions */
extern char *g_var[GVARS];	  /* for external 2-dimensional arrays. */
#ifdef DCMDBUF
extern int *count, *inpcas;
#else
extern int count[], inpcas[];
#endif /* DCMDBUF */
#endif /* NOSPL */

#ifdef UNIX
extern int haslock;			/* For UUCP locks */
extern char flfnam[];
#endif /* UNIX */

#ifdef OS2ORUNIX
extern int maxnam, maxpath;		/* Longest name, path length */
#endif /* OS2ORUNIX */

extern int mdmtyp, mdmsav; 

#ifndef NODIAL
/* DIAL-related variables */
extern char modemmsg[];
extern MDMINF *modemp[];		/* Pointers to modem info structs */
extern int nmdm, dialhng, dialtmo, dialksp, dialdpy, dialsrt, dialmhu, dialsta;
extern int dialrtr, dialint, dialrstr, dialcon, dialcq;
extern int mdmspd, dialec, dialdc, dialfc, dialmth, dialesc;
extern char *dialnum,   *dialini,  *dialdir[], *dialcmd,  *dialnpr,
 *dialdcon, *dialdcoff, *dialecon, *dialecoff, *dialhcmd, *diallac,
 *dialhwfc, *dialswfc,  *dialnofc, *dialpulse, *dialtone,
 *dialaaon, *dialaaoff;
extern char *diallcc,   *dialixp,  *dialixs,   *dialldp,  *diallds,
 *dialpxx,  *dialpxi,   *dialpxo,  *dialsfx,   *dialtfp;
extern int ntollfree;
extern char *dialtfc[];
extern int ndialdir, dialcnf, dialcvt;
extern long dialmax, dialcapas;

extern struct keytab mdmtab[];

#ifdef BIGBUFOK
extern char * dialmsg[];
#endif /* BIGBUFOK */

#endif /* NODIAL */

#ifndef NOCSETS
/* Translation stuff */
extern int fcharset, tcharset, tslevel, language, nlng, tcsr, tcsl;
extern struct keytab lngtab[];
extern struct csinfo fcsinfo[], tcsinfo[];
extern struct langinfo langs[];
#ifdef CK_ANSIC
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
#else
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();	/* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();	/* translation functions. */
#endif /* CK_ANSIC */
#endif /* NOCSETS */

#ifndef NOSPL
/* Built-in variable names, maximum length VNAML (20 characters) */

struct keytab vartab[] = {
    "_line",     VN_TFLN,  CM_INV,	/* 192 */
    "apcactive", VN_APC,   CM_INV,	/* 192 */
    "argc",      VN_ARGC,  0,
    "args",      VN_ARGS,  0,
#ifndef NOCSETS
    "charset",   VN_CSET,  0,		/* 192 */
#endif /* NOCSETS */
    "cmdfile",   VN_CMDF,  0,
    "cmdlevel",  VN_CMDL,  0,
    "cmdsource", VN_CMDS,  0,
    "cols",      VN_COLS,  0,		/* 190 */
    "connection",VN_CONN,  0,		/* 190 */
    "count",     VN_COUN,  0,
    "cps",       VN_CPS,   0,		/* 190 */
    "cpu",	 VN_CPU,   0,
    "crc16",     VN_CRC16, 0,		/* 192 */
#ifndef NODIAL
    "d$ac",      VN_D_AC,  0,		/* 192 */
    "d$cc",      VN_D_CC,  0,		/* 192 */
    "d$ip",      VN_D_IP,  0,		/* 192 */
    "d$lp",      VN_D_LP,  0,		/* 192 */
#endif /* NODIAL */
    "date",      VN_DATE,  0,
    "day",       VN_DAY,   0,
    "dialnumber",VN_DNUM,  0,		/* 192 */
    "dialresult",VN_MDMSG, 0,		/* 192 */
    "dialstatus",VN_DIAL,  0,		/* 190 */
    "directory", VN_DIRE,  0,
    "download",  VN_DLDIR, 0,		/* 192 */
    "errno",     VN_ERRNO, 0,		/* 192 */
    "errstring", VN_ERSTR, 0,		/* 192 */
    "evaluate",  VN_EVAL,  0,		/* 190 */
#ifdef OS2
     "exedir",   VN_EXEDIR,0,		/* 192 */
#endif /* OS2 */
    "exitstatus",VN_EXIT,  0,
    "filespec",  VN_FILE,  0,
    "fsize",     VN_FFC,   0,		/* 190 */
    "ftype",     VN_MODE,  0,		/* 190 */
    "home",      VN_HOME,  0,
    "host",      VN_HOST,  0,
    "input",     VN_IBUF,  0,
    "inchar",    VN_ICHR,  0,
    "incount",   VN_ICNT,  0,
    "inidir",    VN_INI,   0,		/* 192 */
    "instatus",  VN_ISTAT, 0,		/* 192 */
    "ipaddress", VN_IPADDR,0,		/* 192 */
#ifdef OS2
    "keyboard",  VN_KEYB,  0,
#endif /* OS2 */
    "line",      VN_LINE,  0,
    "local",     VN_LCL,   0,
    "macro",     VN_MAC,   0,
    "minput",    VN_MINP,  0,		/* 192 */
    "modem",     VN_MDM,   0,
#ifndef NODIAL
    "m_aa_off",  VN_M_ECX, 0,		/* all 192... */
    "m_aa_on",   VN_M_AAO, 0,
    "m_dc_off",  VN_M_DCX, 0,
    "m_dc_on",   VN_M_DCO, 0,
    "m_dial",    VN_M_DCM, 0,
    "m_ec_off",  VN_M_ECX, 0,
    "m_ec_on",   VN_M_ECO, 0,
    "m_fc_hw",   VN_M_HWF, 0,
    "m_fc_no",   VN_M_NFC, 0,
    "m_fc_sw",   VN_M_SWF, 0,
    "m_hup",     VN_M_HUP, 0,
    "m_init",    VN_M_INI, 0,
    "m_pulse",   VN_M_PDM, 0,
    "m_tone",    VN_M_TDM, 0,
#endif /* NODIAL */
    "ndate",     VN_NDAT,  0,
    "nday",      VN_NDAY,  0,
    "newline",   VN_NEWL,  0,
    "ntime",     VN_NTIM,  0,
    "packetlen", VN_RPSIZ, 0,		/* 192 */
    "parity",    VN_PRTY,  0,		/* 190 */
    "password",  VN_PWD,   CM_INV,	/* 192 */
    "platform",  VN_SYSV,  0,
    "program",   VN_PROG,  0,
    "query",     VN_QUE,   0,		/* 190 */
    "prompt",    VN_PRM,   CM_INV,	/* 192 */
    "protocol",  VN_PROTO, 0,		/* 192 */
    "return",    VN_RET,   0,
#ifdef CK_REXX
    "rexx",      VN_REXX,  0,		/* 190 */
#endif /* CK_REXX */
    "rows",      VN_ROWS,  0,		/* 190 */
#ifdef OS2
    "select",    VN_SELCT, 0,		/* 192 */
#endif /* OS2 */
    "speed",     VN_SPEE,  0,
#ifdef OS2
    "space",     VN_SPA,   0,
    "startup",   VN_STAR,  0,		/* 190 */
#endif /* OS2 */
    "status",    VN_SUCC,  0,
    "sysid",     VN_SYSI,  0,
    "system",    VN_SYST,  0,
    "terminal",  VN_TTYP,  0,
#ifdef OS2
    "termkey",   VN_TRMK,  CM_INV,      /* 192 */
#endif /* OS2 */
    "tfsize",    VN_TFC,   0,
    "time",      VN_TIME,  0,
    "tmpdir",    VN_TEMP,  0,		/* 192 */
#ifdef CK_TTYFD
    "ttyfd",     VN_TTYF,  0,
#endif /* CK_TTYFD */
    "userid",    VN_UID,   0,		/* 192 */
    "version",   VN_VERS,  0,
    "window",    VN_WINDO, 0,		/* 192 */
    "xversion",  VN_XVNUM, 0		/* 192 */
};
int nvars = (sizeof(vartab) / sizeof(struct keytab));
#endif /* NOSPL */

#ifndef NOSPL
struct keytab fnctab[] = {		/* Function names */
#ifdef OS2
    ".oox",       FN_OOX, CM_INV,	/* ... */
#endif /* OS2 */
    "basename",   FN_BSN, 0,		/* Basename */
    "break",      FN_BRK, 0,		/* Break (as in Snobol) */
    "capitalize", FN_CAP, 0,		/* First Letter -> uppercase */
    "caps",       FN_CAP, CM_INV,	/* ditto */
    "character",  FN_CHR, 0,		/* Character from code */
    "checksum",   FN_CHK, 0,		/* Checksum */
    "code",       FN_COD, 0,		/* Code from character */
    "contents",   FN_CON, 0,		/* Definition (contents) of variable */
    "crc16",      FN_CRC, 0,		/* CRC-16 */
#ifdef OS2
    "crypt",      FN_CRY, CM_INV,
#endif /* OS2 */
#ifdef ZFCDAT
    "date",       FN_FD,  0,		/* File modification/creation date */
#endif /* ZFCDAT */
    "definition", FN_DEF, 0,		/* Return definition of given macro */
    "evaluate",   FN_EVA, 0,		/* Evaluate given arith expression */
    "execute",    FN_EXE, 0,		/* Execute given macro */
    "files",      FN_FC,  0,		/* File count */
    "hexify",     FN_HEX, 0,		/* Hexify */
    "index",      FN_IND, 0,		/* Index (string search) */
    "ipaddress",  FN_IPA, 0,		/* Find and return IP address */
    "length",     FN_LEN, 0,		/* Return length of argument */
    "literal",    FN_LIT, 0,		/* Return argument literally */
    "lower",      FN_LOW, 0,		/* Return lowercased argument */
    "lpad",       FN_LPA, 0,		/* Return left-padded argument */
    "ltrim",      FN_LTR, 0,		/* Left-Trim */
    "maximum",    FN_MAX, 0,		/* Return maximum of two arguments */
    "minimim",    FN_MIN, 0,		/* Return minimum of two arguments */
    "modulus",    FN_MOD, CM_INV,	/* Return modulus of two arguments */
    "nextfile",   FN_FIL, 0,		/* Next file in list */
    "pathname",   FN_FFN, 0,		/* Full file name */
    "rep",        FN_REP, CM_INV|CM_ABR,
    "repeat",     FN_REP, 0,		/* Repeat argument given # of times */
    "replace",    FN_RPL, 0,		/* Replace characters in string */
    "reverse",    FN_REV, 0,		/* Reverse the argument string */
    "right",      FN_RIG, 0,		/* Rightmost n characters of string */
    "rindex",     FN_RIX, 0,		/* Right index */
    "rpad",       FN_RPA, 0,		/* Right-pad the argument */
    "size",       FN_FS,  0,		/* File size */
    "span",       FN_SPN, 0,		/* Span - like Snobol */
    "substring",  FN_SUB, 0,		/* Extract substring from argument */
    "tod2secs",   FN_TOD, 0,		/* Time-of-day-to-secs-since-midnite */
    "trim",       FN_TRM, 0,		/* Trim */
    "unhexify",   FN_UNH, 0,		/* Unhexify */
    "upper",      FN_UPP, 0,		/* Return uppercased argument */
    "verify",	  FN_VER, 0		/* Verify */
};
int nfuncs = (sizeof(fnctab) / sizeof(struct keytab));
#endif /* NOSPL */

#ifndef NOSPL				/* Buffer for expansion of */
#define VVBUFL 256			/* built-in variables. */
char vvbuf[VVBUFL+1];
#endif /* NOSPL */

struct keytab disptb[] = {		/* Log file disposition */
    "append",    1,  0,
    "new",       0,  0
};

/* 
  P R E S C A N -- A quick look through the command-line options for 
  items that must be handled before the initialization file is executed.
*/
#ifdef NT
extern int StartedFromDialer;
#endif /* NT */

VOID
prescan(y) int y; {
    int yargc; char **yargv;
    char x;
    char *yp;

    yargc = xargc;
    yargv = xargv;

#ifdef DCMDBUF
    if (!kermrc)
      if (!(kermrc = (char *) malloc(KERMRCL+1)))
	fatal("prescan: no memory for kermrc");
#endif /* DCMDBUF */

    strcpy(kermrc,KERMRC);		/* Default init file name */

#ifndef NOCMDL
#ifndef NOICP
    if (yargc > 1 && *yargv[1] != '-') { /* Filename as 1st argument */
#ifdef OS2
	extern char startupdir[], exedir[], inidir[];
	char * scriptenv, * keymapenv;
#endif /* OS2 */
#ifdef DCMDBUF
	extern char * cmdbuf;
#else
	extern char cmdbuf[];
#endif /* DCMDBUF */
	char takepath[1024];
	char *s;
	int x, y;

	if (!isabsolute(yargv[1])) {	/* If not absolute */
	    /* Set up search path... */
#ifdef OS2    
#ifdef NT
	    scriptenv = getenv("K95SCRIPTS");
	    keymapenv = getenv("K95KEYMAPS");
#else /* NT */
	    scriptenv = getenv("K2SCRIPTS");
	    keymapenv = getenv("K2KEYMAPS");
#endif /* NT */
	    if (!scriptenv)
	      scriptenv = getenv("CK_SCRIPTS");
	    if (!keymapenv)
	      keymapenv = getenv("CK_KEYMAPS");

	    sprintf(takepath,
		    /* semicolon-separated path list */
		    "%s%s%s%s%s;%s%s;%s%s;%s;%s%s;%s%s;%s;%s%s;%s%s", 
		    scriptenv?scriptenv:"",
		    (scriptenv && scriptenv[strlen(scriptenv)-1]==';')?"":";",
		    keymapenv?keymapenv:"",
		    (keymapenv && keymapenv[strlen(keymapenv)-1]==';')?"":";",
		    startupdir,
		    startupdir, "SCRIPTS/",    
		    startupdir, "KEYMAPS/",
		    inidir,
		    inidir, "SCRIPTS/",
		    inidir, "KEYMAPS/",
		    exedir,
		    exedir, "SCRIPTS/",
		    exedir, "KEYMAPS/"
                );
#else /* not OS2 */
#ifndef NOSPL
	    y = 1024;			/* Look in home directory */
	    s = takepath;
	    zzstring("\\v(home)",&s,&y);
#else
	    takepath[0] = '\0';
#endif /* NOSPL */
#endif /* OS2 */
/*
  All the logic for searching the take path is in the command parser.
  So even though we aren't parsing commands, we initialize and call the
  parser from here, with the purported filename stuffed into the command
  buffer, followed by some carriage returns to make the parser return.
  If the file is not found, or otherwise not accessible, the parser prints
  an appropriate message, and then we just exit.
*/
	    cmdini();			/* Allocate command buffers etc */
	    cmini(0);			/* Initialize them */
	    strcpy(cmdbuf,yargv[1]);	/* Stuff filename into command buf */
	    strcat(cmdbuf,"\r\r");	/* And some carriage returns */
	    if ((y = cmifip("","",&s,&x,0,takepath,xxstring)) < 0)
	      doexit(BAD_EXIT,xitsta);
	    cmres();
	} else
	  s = yargv[1];
	/* cfilef = 1; */	/* Command file */
#ifdef ZFNQFP
	zfnqfp(s,CKMAXPATH,cmdfil); /* In case of CD in file */
#else
	strncpy(cmdfil,CKMAXPATH,s);
#endif /* ZFNQFP */
	yargc -= 1;			/* Skip past the filename */
	yargv += 1;			/* Otherwise we'll get an error */
    }
#endif /* NOICP */
    while (--yargc > 0) {		/* Go through command-line args */
	yargv++;
	yp = *yargv+1;			/* Pointer for bundled args */
	if (**yargv == '=') return;	/* Same rules as cmdlin()... */
#ifdef VMS
	else if (**yargv == '/')
	  continue;
#endif /* VMS */
    	else if (**yargv == '-') {	/* Got an option (begins with dash) */
	    x = *(*yargv+1);		/* Get option letter */
	    while (x) {			/* Allow for bundled options */
		debug(F000,"prescan arg","",x);
		switch (x) {
#ifdef OS2
		  case 'W':
		    if (*(yp+1))
		      fatal("invalid argument bundling after -W"); 
		    yargv++, yargc--;
		    if (yargc < 1)
		      fatal("Window handle missing");
		    if (y) {
			yargv++, yargc--;
			break; 
		    } else {	
			hwndDialer = (HWND) atol(*yargv);
			StartedFromDialer = 1;
			yargv++, yargc--;
			KermitDialerID = atol(*yargv) ;
		    }
		    break;
#endif /* OS2 */

#ifndef NOSPL
		case 'M':				/* My User Name */
		      if (*(yp+1)) {
			  fatal("invalid argument bundling");
		      }
		      yargv++, yargc--;
		      if ((yargc < 1) || (**yargv == '-')) {
			  fatal("missing username");
		      }
		      if ((int)strlen(*yargv) > 63) {
			  fatal("username too long");
		      }
		      strcpy(uidbuf,*yargv);
		      break;
#endif /* NOSPL */
		  case 'R':		/* Remote-only advisory */
#ifdef CK_IFRO
		    remonly = 1;
#endif /* CK_IFRO */
		    break;
		  case 'S':		/* STAY */
		    stayflg = 1;
		    break;
		  case 'h':
		  case 'Y':		/* No init file */
		    noinit = 1;
		    break;
		  case 'd':		/* = SET DEBUG ON */
#ifdef DEBUG
		    if (!deblog)
		      deblog = debopn("debug.log",0);
#endif /* DEBUG */
		    break;
		  case 'y':		/* Alternative init file */
		    yargv++, yargc--;
		    if (yargc < 1) fatal("missing name in -y");
		    if (!y)
		      break;
		    strcpy(kermrc,*yargv); /* Replace init file name */
		    rcflag = 1;		/* Flag that this has been done */
		    break;
		  case 'z':		/* = SET BACKGROUND OFF */
		    bgset = 0;
		    break;
#ifdef CK_NETBIOS
		  case 'N':
		    {
			int n ;
			yargv++, yargc--;
			if (y)
			  break;
			if (strlen(*yargv) != 1 || (*yargv)[0] == 'X') {
			    NetBiosAdapter = -1;
			} else {
			    n = atoi(*yargv);
			    if (n >= 0 && n <= 9)
			      NetBiosAdapter = n;
			    else
			      NetBiosAdapter = -1;
			} 
		    } 
		    break;
#endif /* CK_NETBIOS */
		  default:
		    break;
		}
		x = *++yp;		/* See if options are bundled */
	    }
	}
    }
#endif /* NOCMDL */
}

static int tr_int;			/* Flag if TRANSMIT interrupted */

#ifndef MAC
SIGTYP
#ifdef CK_ANSIC
trtrap(int foo)				/* TRANSMIT interrupt trap */
#else 
trtrap(foo) int foo;			/* TRANSMIT interrupt trap */
#endif /* CK_ANSIC */
/* trtrap */ {
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
    tr_int = 1;				/* (Need arg for ANSI C) */
    SIGRETURN;
}
#endif /* MAC */
#endif /* NOICP */

/*  G E T T C S  --  Get Transfer (Intermediate) Character Set  */

/*
  Given two file character sets, this routine picks out the appropriate
  "transfer" character set to use for translating between them.
  The transfer character set number is returned.

  Translation between two file character sets is done, for example,
  by the CONNECT, TRANSMIT, and TRANSLATE commands.

  Translation between Kanji character sets is not yet supported.
*/
int
gettcs(cs1,cs2) int cs1, cs2; {
#ifdef NOCSETS				/* No character-set support */
    return(0);				/* so no translation */
#else
    int tcs = TC_TRANSP;
#ifdef KANJI
/* Kanji not supported yet */
    if (fcsinfo[cs1].alphabet == AL_JAPAN ||
	fcsinfo[cs2].alphabet == AL_JAPAN )
      tcs = TC_TRANSP;
    else
#endif /* KANJI */
#ifdef CYRILLIC
/*
  I can't remember why we don't test both sets here, but I think there
  must have been a reason...
*/
      if (fcsinfo[cs2].alphabet == AL_CYRIL)
	tcs = TC_CYRILL;
      else
#endif /* CYRILLIC */
#ifdef LATIN2
	if (cs1 == FC_2LATIN || cs2 == FC_2LATIN ||
	    cs1 == FC_CP852  || cs2 == FC_CP852 )
	  tcs = TC_2LATIN;
	else
#endif /* LATIN2 */
#ifdef HEBREW
	  if (fcsinfo[cs1].alphabet == AL_HEBREW ||
	      fcsinfo[cs2].alphabet == AL_HEBREW )
	    tcs = TC_HEBREW;
	  else
#endif /* HEBREW */
	    tcs = TC_1LATIN;
    return(tcs);
#endif /* NOCSETS */
}

#ifndef NOLOCAL
/*  D O C O N E C T  --  Do the connect command  */
/*
  q = 0 means issue normal informational message about how to get back, etc.
  q != 0 means to skip the message.
*/

int
doconect(q) int q; {
    int x;				/* Return code */
    extern int what;
#ifndef NOKVERBS			/* Keyboard macro material */
    extern int keymac, keymacx;
#endif /* NOKVERBS */
    extern int justone;
    int qsave;				/* For remembering "quiet" value */
/*
  Saving, changing, and restoring the global "quiet" variable around calls
  to conect() to control whether the verbose CONNECT message is printed is
  obviously less elegant than passing a parameter to conect(), but we do it
  this way to avoid the need to change all of the ck?con.c modules.  NOTE:
  it is important to restore the value immediately upon return in case there
  is an autodownload or APC.
*/
    qsave = quiet;			/* Save it */
    if (!quiet && q > -1)
      quiet = q;			/* Use argument temporarily */
    conres();				/* Put console back to normal */
    debug(F101,"doconect justone 1","",justone);
    x = conect();			/* Connect the first time */
    quiet = qsave;			/* Restore "quiet" value */

    debug(F101,"doconect justone 2","",justone);
#ifdef NETCONN
    if (network && tn_exit && ttyfd == -1)
      doexit(GOOD_EXIT,xitsta);		/* Exit with good status */      
#endif /* NETCONN */

    concb((char)escape);		/* Restore console for commands */

#ifdef CK_APC
/*
  If an APC command was received during CONNECT mode, we define it now
  as a macro, execute the macro, and then return to CONNECT mode.
  We do this in a WHILE loop in case additional APCs come during subsequent
  CONNECT sessions.
*/
    while (apcactive == APC_LOCAL || 
	   apcactive == APC_REMOTE && apcstatus != APC_OFF) {
	debug(F101,"doconect justone 3","",justone);
	domac("apc_commands",apcbuf,cmdstk[cmdlvl].ccflgs|CF_APC);
	if (!apcactive)			/* In case CLEAR APC was in APC */
	  break;
#ifdef OS2
	msleep(250);
#endif /* OS2 */
	debug(F101,"doconect justone 4","",justone);
	qsave = quiet;			/* Do this again... */
	if (!quiet && q > -1)
	  quiet = q;
	x = conect();			/* Re-CONNECT. */
	quiet = qsave;
	debug(F101,"doconect justone 5","",justone);
#ifdef NETCONN
	if (network && tn_exit && ttyfd == -1)
	  doexit(GOOD_EXIT,xitsta);	/* Exit with good status */      
#endif /* NETCONN */
	concb((char)escape);		/* Restore console. */
	if (ttyfd == -1)
	  break;
    }					/* Loop back for more. */
#endif /* CK_APC */

#ifndef NOKVERBS
    if ((keymac > 0) && (keymacx > -1)) { /* Executing a keyboard macro? */
	/* Set up the macro and return */
	/* Do not clear the keymac flag */
	return(dodo(keymacx,NULL,CF_KMAC|cmdstk[cmdlvl].ccflgs));
    }
#endif /* NOKVERBS */
    what = W_COMMAND;			/* Back in command mode. */
    return(x);				/* Done. */
}
#endif /* NOLOCAL */

#ifndef NOICP 
#ifdef COMMENT
/*
  It seemed that this was needed for OS/2, in which \v(cmdfile) and other
  file-oriented variables or functions can return filenames containing
  backslashes, which are subsequently interpreted as quotes rather than
  directory separators (e.g. see commented section for VN_CMDF below).
  But the problem can't be cured at this level.  Example:

    type \v(cmdfile)

  Without doubling, the filename is parsed correctly, but then when passed
  to UNIX 'cat' through the shell, the backslash is removed, and then cat
  can't open the file.  With doubling, the filename is not parsed correctly
  and the TYPE command fails immediately with a "file not found" error.
*/
/*
  Utility routine to double all backslashes in a string.
  s1 is pointer to source string, s2 is pointer to destination string,
  n is length of destination string, both NUL-terminated.
  Returns 0 if OK, -1 if not OK (destination string too short).
*/
int
dblbs(s1,s2,n) char *s1, *s2; int n; {
    int i = 0;
    while (*s1) {
	if (*s1 == '\\') {
	    if (++i > n) return(-1);
	    *s2++ = '\\';
	}
	if (++i > n) return(-1);
	*s2++ = *s1++;
    }
    *s2 = NUL;
    return(0);
}
#endif /* COMMENT */

char * 
gmdmtyp() {				/* Get modem type */
#ifndef NODIAL
    int i, x;
    x = mdmtyp;
    if (x < 0)				/* In case of network dialing */
      x = mdmsav;
    if (x < 1)    
      return("none");
    else
      for (i = 0; i < nmdm; i++)
	if ((mdmtab[i].kwval == x) && (mdmtab[i].flgs == 0))
	  return(mdmtab[i].kwd);
#endif /* NODIAL */
    return("none");
}

#ifndef NOXMIT
#ifndef NOLOCAL
/*  T R A N S M I T  --  Raw upload  */

/*  Obey current line, duplex, parity, flow, text/binary settings. */
/*  Returns 0 upon apparent success, 1 on obvious failure.  */

/***
 Things to add:
 . Make both text and binary mode obey set file bytesize.
 . Maybe allow user to specify terminators other than CR?
 . Maybe allow user to specify prompts other than single characters?
***/

/*  T R A N S M I T  --  Raw upload  */

/*  s is the filename, t is the turnaround (prompt) character  */

/*
  Maximum number of characters to buffer.
  Must be less than LINBUFSIZ
*/
#define XMBUFS 120

#ifdef NETCONN
#ifndef IAC
#define IAC 255
#endif /* IAC */
#endif /* NETCONN */

int
#ifdef CK_ANSIC
transmit(char * s, char t)
#else
transmit(s,t) char *s; char t;
#endif /* CK_ANSIC */
/* transmit */ {
#ifdef MAC
    extern char sstate;
    int count = 100;
#else
#ifdef OS2
#ifdef NT
SIGTYP (* oldsig)(int);			/* For saving old interrupt trap. */
#else /* NT */
SIGTYP (* volatile oldsig)(int);	/* For saving old interrupt trap. */
#endif /* NT */

#else /* OS2 */
    SIGTYP (* oldsig)();
#endif /* OS2 */
#endif /* MAC */
    long zz;
    int z = 1;				/* Return code. 0=fail, 1=succeed. */
    int x, c, i;			/* Workers... */
    int myflow;
    int mybinary;
#ifdef COMMENT
    CHAR csave;
#endif /* COMMENT */
    char *p;

#ifndef NOCSETS
    int tcs = TC_TRANSP;		/* Intermediate (xfer) char set */
    int langsv = L_USASCII;		/* Save current language */

#ifdef CKOUNI
    _PROTOTYP ( USHORT (*sxo), (CHAR) ) = NULL; /* Translation functions */
    _PROTOTYP ( int (*rxo), (USHORT) ) = NULL;
    _PROTOTYP ( USHORT (*sxi), (CHAR) ) = NULL;
    _PROTOTYP ( int (*rxi), (USHORT) ) = NULL;
#else /* CKOUNI */
    _PROTOTYP ( CHAR (*sxo), (CHAR) ) = NULL; /* Translation functions */
    _PROTOTYP ( CHAR (*rxo), (CHAR) ) = NULL;
    _PROTOTYP ( CHAR (*sxi), (CHAR) ) = NULL;
    _PROTOTYP ( CHAR (*rxi), (CHAR) ) = NULL;
#endif /* CKOUNI */
#endif /* NOCSETS */

/*
   If a system-specific binary mode is set (MacBinary, Image, Labeled, etc),
   revert to "normal" binary mode for duration of TRANSMIT command.
*/
    mybinary = binary;
    if (binary) binary = XYFT_B;
    if (zopeni(ZIFILE,s) == 0) {	/* Open the file to be transmitted */
	printf("?Can't open file %s\n",s);
	binary = mybinary;
	return(0);
    }
    x = -1;				/* Open the communication line */
    if (ttopen(ttname,&x,mdmtyp,cdtimo) < 0) {	/* (no harm if already open) */
	printf("Can't open device %s\n",ttname);
	binary = mybinary;
	return(0);
    }
    zz = x ? speed : -1L;
    if (binary) {			/* Binary file transmission */
	myflow = (flow == FLO_XONX) ? FLO_NONE : flow;
	if (ttvt(zz,myflow) < 0) {	/* So no Xon/Xoff! */
	    printf("Can't condition line\n");
	    binary = mybinary;
	    return(0);
	}
    } else {
	if (ttpkt(zz,flow,parity) < 0) { /* Put the line in "packet mode" */
	    printf("Can't condition line\n"); /* so Xon/Xoff will work, etc. */
	    binary = mybinary;
	    return(0);
	}
    }

#ifndef NOCSETS
#ifdef CKOUNI
/* Set up character set translations */
    if (binary == 0) {
	if (tcsr == tcsl || binary) {	/* Remote and local sets the same? */
	    sxo = NULL;			/* Or file type is not text? */
	    rxo = NULL;		
	    sxi = NULL;
	    rxi = NULL;
	} else {  
   	    sxo = xl_u[tcsl];
	    rxo = xl_tx[tcsr];
	    rxi = xl_tx[tcsl];
	    sxi = xl_u[tcsr];
	}
/*
   This is to prevent use of zmstuff() and zdstuff() by translation functions.
   They only work with disk i/o, not with communication i/o.  Luckily Russian
   translation functions don't do any stuffing...
*/
	langsv = language;
	language = L_USASCII;
    }
#else /* CKOUNI */
    tcs = gettcs(tcsr,tcsl);		/* Get intermediate set. */

/* Set up character set translations */
    if (binary == 0) {

	if (tcsr == tcsl || binary) {	/* Remote and local sets the same? */
	    sxo = rxo = NULL;		/* Or file type is not text? */
	    sxi = rxi = NULL;
	} else {			/* Otherwise, set up */
	    sxo = xls[tcs][tcsl];	/* translation function */
	    rxo = xlr[tcs][tcsr];	/* pointers for output functions */
	    sxi = xls[tcs][tcsr];	/* and for input functions. */
	    rxi = xlr[tcs][tcsl];
	}
/*
  This is to prevent use of zmstuff() and zdstuff() by translation functions.
  They only work with disk i/o, not with communication i/o.  Luckily Russian
  translation functions don't do any stuffing...
*/
	langsv = language;
	language = L_USASCII;
    }
#endif /* CKOUNI */
#endif /* NOCSETS */

    i = 0;				/* Beginning of buffer. */
#ifndef MAC
#ifndef AMIGA
    oldsig = signal(SIGINT, trtrap);	/* Save current interrupt trap. */
#endif /* AMIGA */
#endif /* MAC */
    tr_int = 0;				/* Have not been interrupted (yet). */
    z = 1;				/* Return code presumed good. */
#ifdef VMS
    conres();
#endif /* VMS */

    c = 0;				/* Initial condition */
    while (c > -1) {			/* Loop for all characters in file */
#ifdef MAC
	/*
	 * It is expensive to run the miniparser so don't do it for
	 * every character.
	 */
	if (--count < 0) {
	    count = 100;
	    miniparser(1);
	    if (sstate == 'a') {
		sstate = '\0';
		z = 0;
		break;
	    }
	}
#else /* Not MAC */
	if (tr_int) {			/* Interrupted? */
	    printf("^C...\n");		/* Print message */
	    z = 0;
	    break;
	}
#endif /* MAC */
	c = zminchar();			/* Get a file character */
	debug(F101,"transmit char","",c);
	if (c == -1)			/* Test for end-of-file */
	  break;
	if (c < 0) {
	    z = 0;
	    goto xmitexit;	    
	}
	c &= fmask;			/* Apply SET FILE BYTESIZE mask */

	if (binary) {			/* If binary file, */
	    if (ttoc(dopar((char) c)) < 0) { /* else just send the char */
		printf("?Can't transmit character\n");
		z = 0;
		goto xmitexit;
	    }
#ifdef TNCODE
	    if (c == IAC && network && ttnproto == NP_TELNET)
	      ttoc((char)IAC);
#endif /* TNCODE */
	    if (xmitw) msleep(xmitw);	/* Pause if requested */
	    if (xmitx) {		/* SET TRANSMIT ECHO ON? */
		if (duplex) {		/* Yes, for half duplex */
		    if (conoc((char)(c & cmdmsk)) < 0) { /* echo locally. */
			z = 0;
			goto xmitexit;
		    }
		} else {		/* For full duplex, */
		    int i, n;		/* display whatever is there. */
		    n = ttchk();	/* See how many chars are waiting */
		    if (n < 0) {	/* Connection dropped? */
			z = 0;
			goto xmitexit;
		    }
		    for (i = 0; i < n; i++) { /* Read and echo that many. */
			x = ttinc(1);	/* Timed read just in case. */
			if (x > -1) {	/* If no timeout */
			    if (parity) x &= 0x7f; /* display the char, */
			    if (conoc((char)(x & cmdmsk)) < 0) {
				z = 0;
				goto xmitexit;
			    }
			} else break;	/* otherwise stop reading. */
		    }
		}
	    } else ttflui();		/* Not echoing, just flush input. */

	} else {			/* Text mode, line at a time. */

	    if (c == '\n') {		/* Got a line */
		int stuff = -1;
		if (i == 0) {		/* Blank line? */
		    if (xmitf)		/* Yes, insert fill if asked. */
		      line[i++] = dopar((char) xmitf);
		}
		if (i == 0 || ((char) line[i-1]) != ((char) dopar(CR)))
		  line[i++] = dopar(CR); /* Terminate it with CR */
		if (xmitl) {
		    stuff = LF;
#ifdef TNCODE
		} else if (network &&	/* TELNET NEWLINE ON/OFF/RAW */
			   (ttnproto == NP_TELNET) &&
			   (tn_nlm != TNL_CR)) {
		    stuff = (tn_nlm == TNL_CRLF) ? LF : NUL;
#endif /* TNCODE */
		}
		if (stuff > -1)
		  line[i++] = dopar((char)stuff);

	    } else if (c != -1) {	/* Not a newline, regular character */
#ifdef COMMENT
		csave = c;		/* Remember untranslated version */
#endif /* COMMENT */
#ifndef NOCSETS
		/* Translate character sets */
#ifdef CKOUNI
		if (cs_is_nrc(tcsl) || c > 127) 
		  if (sxo) c = (*sxo)(c); /* From local to intermediate */
		if (c >= 32)
		  if (rxo) c = (*rxo)(c); /* From intermediate to remote */
#else /* CKOUNI */
		if (sxo) c = (*sxo)((CHAR)c); /* From local to intermediate */
		if (rxo) c = (*rxo)((CHAR)c); /* From intermediate to remote */
#endif /* CKOUNI */
#endif /* NOCSETS */

		if (xmits && parity && (c & 0200)) { /* If shifting */
		    line[i++] = dopar(SO);          /* needs to be done, */
		    line[i++] = dopar((char)c);	    /* do it here, */
		    line[i++] = dopar(SI);          /* crudely. */
		} else {
		    line[i++] = dopar((char)c); /* else, just char itself */
#ifdef TNCODE
		    if (c == IAC && network && ttnproto == NP_TELNET)
		      line[i++] = IAC;
#endif /* TNCODE */
		}
	    }

/* Send characters if buffer full, or at end of line, or at end of file */

	    if (i >= XMBUFS || c == '\n' || c == -1) {
		p = line;
		line[i] = '\0';
		debug(F111,"transmit buf",p,i);
		if (ttol((CHAR *)p,i) < 0) { /* try to send it. */
		    printf("Can't send buffer\n");
		    z = 0;
		    break;
		}
		i = 0;			/* Reset buffer pointer. */

/* Worry about echoing here. "xmitx" is SET TRANSMIT ECHO flag. */

		if (duplex && xmitx) {	/* If local echo, echo it */
		    if (parity || cmdmsk == 0x7f) { /* Strip off high bits */
			char *s = p;		    /* if necessary */
			while (*s) {
			    *s &= 0x7f;
			    s++;
			}
			if (conoll(p) < 0) {
			    z = 0;
			    goto xmitexit;
			}
		    }
		}
		if (xmitw)		/* Give receiver time to digest. */
		  msleep(xmitw);
		if (t != 0 && c == '\n') { /* Want a turnaround character */
		    x = 0;		   /* Wait for it */
		    while (x != t) {
			if ((x = ttinc(1)) < 0) { z = 0; goto xmitexit; }
			if (xmitx && !duplex) {	/* Echo any echoes */
			    if (parity) x &= 0x7f;
#ifndef NOCSETS
#ifdef CKOUNI
			    if (cs_is_nrc(tcsr) || x > 127) 
			      if (sxi) x = (*sxi)(x);
			    if (x >= 32)
			      if (rxi) x = (*rxi)(x);
#else /* CKOUNI */
			    if (sxi) x = (*sxi)((CHAR)x); /* But translate */
			    if (rxi) x = (*rxi)((CHAR)x); /* them first... */
#endif /* CKOUNI */
#endif /* NOCSETS */
			    if (conoc((char) x) < 0) { z = 0; goto xmitexit; }
			}
		    }
		} else if (xmitx && !duplex) { /* Otherwise, */
		    int n;
		    while ((n = ttchk()) > 0) {	/* echo for as long as */
			if ((x = ttinc(0)) < 0) break; /* anything is there. */
			if (parity) x &= 0x7f;
#ifndef NOCSETS
#ifdef CKOUNI
			if (cs_is_nrc(tcsr) || x > 127) 
			  if (sxi) x = (*sxi)(x);
			if (x >= 32)
			  if (rxi) x = (*rxi)(x);
#else /* CKOUNI */
			if (sxi) x = (*sxi)((CHAR)x); /* Translate first */
			if (rxi) x = (*rxi)((CHAR)x);
#endif /* CKOUNI */
#endif /* NOCSETS */
			if (conoc((char)x) < 0) { z = 0; goto xmitexit; }
		    }
		    if (n < 0) {	/* Connection dropped? */
			z = 0;
			goto xmitexit;
		    }
		} else ttflui();	/* Otherwise just flush input buffer */
	    }				/* End of buffer-dumping block */
	}				/* End of text mode */
    }					/* End of character-reading loop */

xmitexit:

    if (z > 0) {
	if (*xmitbuf) {			/* Anything to send at EOF? */
	    p = xmitbuf;		/* Yes, point to string. */
	    while (*p)			/* Send it. */
	      ttoc(dopar(*p++));	/* Don't worry about echo here. */
	}
    }

#ifndef AMIGA
#ifndef MAC
    signal(SIGINT,oldsig);		/* Put old signal action back. */
#endif /* MAC */
#endif /* AMIGA */
#ifdef VMS
    concb(escape);			/* Put terminal back, */
#endif /* VMS */
    zclose(ZIFILE);			/* Close file, */
#ifndef NOCSETS
    language = langsv;			/* restore language, */
#endif /* NOCSETS */
    binary = mybinary;			/* restore transfer mode, */
    ttres();				/* and terminal modes, */
    return(z);				/* and return successfully. */
}
#endif /* NOLOCAL */
#endif /* NOXMIT */

#ifndef NOCSETS

_PROTOTYP( CHAR (*sxx), (CHAR) );       /* Local translation function */
_PROTOTYP( CHAR (*rxx), (CHAR) );       /* Local translation function */
_PROTOTYP( CHAR zl1as, (CHAR) );	/* Latin-1 to ascii */
_PROTOTYP( CHAR xl1as, (CHAR) );	/* ditto */

/*  X L A T E  --  Translate a local file from one character set to another */

/*
  Translates input file (fin) from character set csin to character set csout
  and puts the result in the output file (fout).  The two character sets are
  file character sets from fcstab.
*/

int
xlate(fin, fout, csin, csout) char *fin, *fout; int csin, csout; {

#ifndef MAC
#ifdef OS2
#ifdef NT
    SIGTYP (* oldsig)(int);		/* For saving old interrupt trap. */
#else /* NT */
    SIGTYP (* volatile oldsig)(int);	/* For saving old interrupt trap. */
#endif /* NT */
#else /* OS2 */    
    SIGTYP (* oldsig)();		/* For saving old interrupt trap. */
#endif /* OS2 */
#endif /* MAC */
    int filecode;			/* Code for output file */

    int z = 1;				/* Return code. */
    int c, tcs;				/* Workers */

    if (zopeni(ZIFILE,fin) == 0) {	/* Open the file to be translated */
	printf("?Can't open input file %s\n",fin);
	return(0);
    }
#ifdef MAC
/*
  If user specified no output file, it goes to the screen.  For the Mac,
  this must be done a special way (result goes to a new window); the Mac
  doesn't have a "controlling terminal" device name.
*/
    filecode = !strcmp(fout,CTTNAM) ? ZCTERM : ZOFILE;
#else
#ifdef VMS
    filecode = !strcmp(fout,CTTNAM) ? ZCTERM : ZMFILE;
#else
    filecode = ZOFILE;
#endif /* VMS */
#endif /* MAC */

    if (zopeno(filecode,fout,NULL,NULL) == 0) { /* And the output file */
	printf("?Can't open output file %s\n",fout);
	return(0);
    }
#ifndef AMIGA
#ifndef MAC
    oldsig = signal(SIGINT, trtrap);	/* Save current interrupt trap. */
#endif /* MAC */
#endif /* AMIGA */

    tr_int = 0;				/* Have not been interrupted (yet). */
    z = 1;				/* Return code presumed good. */

    tcs = gettcs(csin,csout);		/* Get intermediate set. */

    printf("%s (%s) => %s (%s)\n",	/* Say what we're doing. */
	   fin, fcsinfo[csin].name,
	   fout,fcsinfo[csout].name
    );
    printf("via %s", tcsinfo[tcs].name);
    if (language)
      printf(", language: %s\n",langs[language].description);
    printf("\n\n");

    if (csin == csout) {		/* Input and output sets the same? */
	sxx = rxx = NULL;		/* If so, no translation. */
    } else {				/* Otherwise, set up */
	sxx = xls[tcs][csin];		/* translation function */
	rxx = xlr[tcs][csout];		/* pointers. */
	if (rxx == zl1as) rxx = xl1as;
    }
    while ((c = zminchar()) != -1) {	/* Loop for all characters in file */
	if (tr_int) {			/* Interrupted? */
	    printf("^C...\n");		/* Print message */
	    z = 0;
	    break;
	}
	if (sxx) c = (*sxx)((CHAR)c);	/* From fcs1 to tcs */
	if (rxx) c = (*rxx)((CHAR)c);	/* from tcs to fcs2 */

	if (zchout(filecode,(char)c) < 0) { /* Output the xlated character */
	    printf("File output error\n");
	    z = 0;
	    break;
	}
    }
#ifndef AMIGA
#ifndef MAC
    signal(SIGINT,oldsig);		/* put old signal action back. */
#endif /* MAC */
#endif /* AMIGA */

    tr_int = 0;
    zclose(ZIFILE);			/* close files, */
    zclose(filecode);
    return(z);				/* and return successfully. */
}
#endif /* NOCSETS */

/*  D O L O G  --  Do the log command  */

int
dolog(x) int x; {
    int y, disp; char *s = NULL;
#ifdef ZFNQFP
    struct zfnfp * fnp;
#endif /* ZFNQFP */

    switch (x) {			/* Which log... */

#ifdef DEBUG
      case LOGD:
	y = cmofi("Name of debugging log file","debug.log",&s,xxstring);
	break;
#endif /* DEBUG */

      case LOGP:
	y = cmofi("Name of packet log file","packet.log",&s,xxstring);
	break;

#ifndef NOLOCAL
      case LOGS:
	y = cmofi("Name of session log file","session.log",&s,xxstring);
	break;
#endif /* NOLOCAL */

#ifdef TLOG
      case LOGT:
	y = cmofi("Name of transaction log file","transact.log",&s,
		  xxstring);
	break;
#endif /* TLOG */

      default:
	printf("\n?Unknown log designator - %d\n",x);
	return(-2);
    }
    if (y < 0) return(y);
    if (y == 2) {
	printf("?Sorry, %s is a directory name\n",s);
	return(-9);
    }
#ifdef ZFNQFP
    if (fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf)) {
	if (fnp->fpath)
	  if ((int) strlen(fnp->fpath) > 0)
	    s = fnp->fpath;
    }
#endif /* ZFNQFP */

    strcpy(line,s);
    s = line;
#ifdef MAC
    y = 0;
#else
    if ((y = cmkey(disptb,2,"Disposition","new",xxstring)) < 0)
      return(y);
#endif /* MAC */
    disp = y;    
    if ((y = cmcfm()) < 0) return(y);

    switch (x) {

#ifdef DEBUG
      case LOGD:
	return(deblog = debopn(s,disp));
#endif /* DEBUG */

      case LOGP:
	return(pktlog = pktopn(s,disp));

#ifndef NOLOCAL
      case LOGS:
	return(seslog = sesopn(s,disp));
#endif /* NOLOCAL */

#ifdef TLOG
      case LOGT:
	return(tralog = traopn(s,disp));
#endif /* TLOG */

      default:
	return(-2);
    }
}

int
pktopn(s,disp) char *s; int disp; {
    extern char pktfil[];
    static struct filinfo xx;
    int y;

    zclose(ZPFILE);
    if(s[0] == '\0') return(0);
    if (disp) {
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
	xx.lblopts = 0;
	pktlog = zopeno(ZPFILE,s,NULL,&xx);
    } else pktlog = zopeno(ZPFILE,s,NULL,NULL);
    if (pktlog > 0)
      strcpy(pktfil,s);
    else
      *pktfil = '\0';
    return(pktlog);
}

int
traopn(s,disp) char *s; int disp; {
#ifdef TLOG
    extern char trafil[];
    static struct filinfo xx;
    int y;

    zclose(ZTFILE);
    if(s[0] == '\0') return(0);
    if (disp) {
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
	xx.lblopts = 0;
	tralog = zopeno(ZTFILE,s,NULL,&xx);
    } else tralog = zopeno(ZTFILE,s,NULL,NULL);
    if (tralog > 0) {
	strcpy(trafil,s);
	tlog(F110,"Transaction Log:",versio,0L);
#ifndef MAC
	tlog(F100,ckxsys,"",0L);
#endif /* MAC */
	ztime(&s);
	tlog(F100,s,"",0L);
    } else *trafil = '\0';
    return(tralog);
#else
    return(0);
#endif /* TLOG */
}

#ifndef NOLOCAL
int
sesopn(s,disp) char * s; int disp; {
    extern char sesfil[];
    static struct filinfo xx;
    int y;

    zclose(ZSFILE);
    if(s[0] == '\0') return(0);
    if (disp) {
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
	xx.lblopts = 0;
	seslog = zopeno(ZSFILE,s,NULL,&xx);
    } else seslog = zopeno(ZSFILE,s,NULL,NULL);
    if (seslog > 0)
      strcpy(sesfil,s);
    else
      *sesfil = '\0';
    return(seslog);
}
#endif /* NOLOCAL */

int
debopn(s,disp) char *s; int disp; {
#ifdef DEBUG
    char *tp;
    static struct filinfo xx;

    zclose(ZDFILE);

    if (disp) {
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
	xx.lblopts = 0;
	deblog = zopeno(ZDFILE,s,NULL,&xx);
    } else deblog = zopeno(ZDFILE,s,NULL,NULL);
    if (deblog > 0) {
	strcpy(debfil,s);
	debug(F110,"Debug Log ",versio,0);
#ifndef MAC
	debug(F100,ckxsys,"",0);
#endif /* MAC */
	ztime(&tp);
	debug(F100,tp,"",0);
    } else *debfil = '\0';
    return(deblog);
#else
    return(0);
#endif /* MAC */
}

/*  G F M O D E  --  Get File (transfer) Mode  */

char *
gfmode(binary) int binary; {
    char * s;
    switch (binary) {
      case XYFT_T: s = "text";	       break;
#ifdef VMS
      case XYFT_B: s = "binary fixed"; break;
      case XYFT_I: s = "image";        break;
      case XYFT_L: s = "labeled";      break;
      case XYFT_U: s = "binary undef"; break;
#else
#ifdef MAC
      case XYFT_B: s = "binary";       break;
      case XYFT_M: s = "macbinary";    break;
#else
      case XYFT_B: s = "binary";       break;
#ifdef CK_LABELED
      case XYFT_L: s = "labeled";      break;
#endif /* CK_LABELED */
#endif /* MAC */
#endif /* VMS */
      default: s = ""; break;
    }
    return(s);
}

#ifndef NOSHOW

/*  S H O P A R  --  Show Parameters  */

#ifdef ANYX25
VOID
shox25() {
    if (nettype == NET_SX25) {
	printf("SunLink X.25 V%d.%d",x25ver / 10,x25ver % 10);
	if (ttnproto == NP_X3) printf(", PAD X.3, X.28, X.29 protocol,");
	printf("\n Reverse charge call %s",
	       revcall ? "selected" : "not selected");
	printf (", Closed user group ");
	if (closgr > -1)
	  printf ("%d",closgr);
	else
	  printf ("not selected");
	printf (",");
	printf("\n Call user data %s.\n", cudata ? udata : "not selected");
    } else if (nettype == NET_VX25) {
	if (ttnproto == NP_X3) printf(", PAD X.3, X.28, X.29 protocol,");
	printf("\n Reverse charge call %s",
	       revcall ? "selected" : "not selected");
	printf (", Closed user group [unsupported]");
	if (closgr > -1)
	  printf ("%d",closgr);
	else
	  printf ("not selected");
	printf (",");
	printf("\n Call user data %s.\n", cudata ? udata : "not selected");
    }
}
#endif /* ANYX25 */

VOID
shoparc() {
    int i; char *s;
    long zz;

    printf("Communications Parameters:\n");

    if (network) {
	printf(" Host: %s",ttname);
    } else {
#ifdef OS2
	printf(" Port: %s, speed: ",ttname);
#else
	printf(" Line: %s, speed: ",ttname);
#endif /* OS2 */
	if ((zz = ttgspd()) < 0) {
	    printf("unknown");
        } else {
	    if (speed == 8880) printf("75/1200"); else printf("%ld",zz);
	}
    }
    printf(", mode: ");
    if (local) printf("local"); else printf("remote");
    if (network == 0) {
	printf(", modem: %s",gmdmtyp());
    } else {
       if (nettype == NET_TCPA) printf(", TCP/IP");
       if (nettype == NET_TCPB) printf(", TCP/IP");
       if (nettype == NET_DEC) {
          if ( ttnproto == NP_LAT ) printf(", DECnet LAT");
          else if ( ttnproto == NP_CTERM ) printf(", DECnet CTERM");
          else printf(", DECnet");
        }
       if ( nettype == NET_SLAT ) printf(", Meridian Technologies' SuperLAT") ;
#ifdef NETFILE
       if ( nettype == NET_FILE ) printf(", local file") ;
#endif /* NETFILE */
       if (nettype == NET_PIPE) printf(", Named Pipes");
#ifdef ANYX25
	shox25();
#endif /* ANYX25 */
	if (ttnproto == NP_TELNET) printf(", telnet protocol");
    }
    if (local) {
	i = parity ? 7 : 8;
	if (i == 8) i = (cmask == 0177) ? 7 : 8;
	printf("\n Terminal bits: %d, p",i);
    } else printf("\n P");
    printf("arity: %s",parnam((char)parity));
    printf(", duplex: ");
    if (duplex) printf("half, "); else printf("full, ");
    printf("flow: ");
    if (flow == FLO_KEEP) printf("keep");
        else if (flow == FLO_AUTO) printf("auto");
        else if (flow == FLO_XONX) printf("xon/xoff");
	else if (flow == FLO_NONE) printf("none");
	else if (flow == FLO_RTSC) printf(network ? "none" : "rts/cts");
	else if (flow == FLO_DTRT) printf(network ? "none" : "dtr/cts");
        else if (flow == FLO_DTRC) printf(network ? "none" : "dtr/cd");
	else printf("%d",flow);
    printf(", handshake: ");
    if (turn) printf("%d\n",turnch); else printf("none\n");
    if (local && !network) {		/* Lockfile & carrier stuff */
	if (carrier == CAR_OFF) s = "off";
	else if (carrier == CAR_ON) s = "on";
	else if (carrier == CAR_AUT) s = "auto";
	else s = "unknown";
	printf(" Carrier: %s", s);
	if (carrier == CAR_ON) {
	    if (cdtimo) printf(", timeout: %d sec", cdtimo);
	    else printf(", timeout: none");
	}
#ifdef UNIX
	if (haslock && *flfnam) {	/* Lockfiles only apply to UNIX... */
	    printf(", lockfile: %s",flfnam);
	}
#endif /* UNIX */
	printf("\n Escape character: %d (^%c)\n",escape,ctl(escape));
    }
}

#ifdef TNCODE
static VOID
shotel() {
    printf("SET TELNET parameters:\n echo: %s\n NVT newline-mode: ",
	   tn_duplex ? "local" : "remote");
    switch (tn_nlm) {
      case TNL_CRNUL: printf("%s\n","off (cr-nul)"); break;
      case TNL_CRLF:  printf("%s\n","on (cr-lf)"); break;
      case TNL_CR:    printf("%s\n","raw (cr)"); break;
      case TNL_LF:    printf("%s\n","(lf)"); break;
    }
    printf(" BINARY newline-mode: ");
    switch (tn_b_nlm) {
      case TNL_CRNUL: printf("%s\n","off (cr-nul)"); break;
      case TNL_CRLF:  printf("%s\n","on (cr-lf)"); break;
      case TNL_CR:    printf("%s\n","raw (cr)"); break;
      case TNL_LF:    printf("%s\n","(lf)"); break;
    }
    printf(" binary-mode: ");
    switch ( tn_binary ) {
        case TN_BM_AC: printf( "accepted, " ); break ;
        case TN_BM_RF: printf( "refused, " ); break;
        case TN_BM_RQ: printf( "requested, "); break;
        };
    printf("host=%s, c-kermit=%s\n", u_binary ? "BINARY" : "NVT",
        me_binary ? "BINARY" : "NVT" ) ;
    printf(" bug binary-me-means-u-too: %s\n",showoff(tn_b_meu));
    printf(" bug binary-u-means-me-too: %s\n",showoff(tn_b_ume));
    printf(" terminal-type: ");
    if (tn_term) {
	printf("%s\n",tn_term);
    } else {
	char *p;
#ifdef OS2
	p = (tt_type >= 0 && tt_type <= max_tt) ?
	  tt_info[tt_type].x_name :
	    "UNKNOWN";
#else
	p = getenv("TERM");
#endif /* OS2 */
	if (p)
	  printf("none (%s will be used)\n",p);
	else printf("none\n");
    }
}
#endif /* TNCODE */

#ifdef CK_NETBIOS 
static VOID
shonb() {
   printf("NETBIOS parameters:\n");
   printf(" API       : %s\n",
      NetbeuiAPI ?
	  "NETAPI.DLL - IBM Extended Services or Novell Netware Requester"
	  : "ACSNETB.DLL - IBM Network Transport Services/2" ) ;
   printf(" Local Name: [%s]\n", NetBiosName ) ;
   printf(" Adapter   : %d\n", NetBiosAdapter ) ;
   if ( NetBiosLSN > 0xFF )
     printf(" Session   : %d\n", NetBiosLSN ) ;
   else
     printf(" Session   : none active\n") ;
}
#endif /* CK_NETBIOS */

VOID
shonet() {
#ifndef NETCONN
    printf("\nNo networks are supported in this version of C-Kermit\n");

#else /* rest of this routine */

    int i;

#ifndef NODIAL
    if (nnetdir <= 1) {
	printf("\nNetwork directory: %s\n",netdir[0] ? netdir[0] : "(none)");
    } else {
	int i;
	printf("\nNetwork directories:\n");
	for (i = 0; i < nnetdir; i++)
	  printf("%2d. %s\n",i,netdir[i]);
    }
#endif /* NODIAL */

#ifdef OS2
    printf("\nNetwork availability:\n");
#else
    printf("\nSupported networks:\n");
#endif /* OS2 */

#ifdef VMS

#ifdef MULTINET
    printf(" TGV MultiNet TCP/IP");
#else
#ifdef WINTCP
    printf(" WOLLONGONG WIN/TCP");
#else
#ifdef DEC_TCPIP
    {
	static $DESCRIPTOR(tcp_desc,"_TCP0:");
        int status;
	long devclass;
	static int itmcod = DVI$_DEVCLASS;

#ifdef COMMENT
	status = LIB$GETDVI(&itmcod, 0, &tcp_desc, &devclass);
#else
	/* Martin Zinser 9/96 */
	status = lib$getdvi(&itmcod, 0, &tcp_desc, &devclass);
#endif /* COMMENT */
	if ((status & 1) && (devclass == DC$_SCOM))
	  printf(" Process Software Corporation TCPware for OpenVMS");
	else
	  printf(" DEC TCP/IP Services for (Open)VMS");
    }
#else
#ifdef CMU_TCPIP
    printf(" CMU-OpenVMS/IP");
#else
    printf(" None");
#endif /* CMU_TCPIP */
#endif /* DEC_TCPIP */
#endif /* WINTCP */
#endif /* MULTINET */
#ifdef TNCODE
    printf(", TELNET protocol\n\n");
    shotel();
#endif /* TNCODE */
    printf("\n");

#else /* Not VMS */

#ifdef SUNX25
    printf(" SunLink X.25\n");
#endif /* SUNX25 */

#ifdef STRATUSX25
    printf(" Stratus VOS X.25\n");
#endif /* STRATUSX25 */

#ifdef DECNET
#ifdef OS2
    if (dnet_avail)
      printf(" DECnet, LAT protocol\n");
    else
      printf(" DECnet, LAT protocol - not available\n");
#else
    printf(" DECnet\n");
#endif /* OS2 */
#endif /* DECNET */

#ifdef NPIPE
    printf(" Named Pipes\n");
#endif /* NPIPE */

#ifdef CK_NETBIOS
    if (netbiosAvail)
      printf(" NETBIOS\n");
    else
      printf(" NETBIOS - not available\n");
#endif /* CK_NETBIOS */

#ifdef SUPERLAT
    if (slat_avail)
      printf(" SuperLAT\n");
    else
      printf(" SuperLAT - not available\n") ;
#endif /* SUPERLAT */

#ifdef TCPSOCKET
    if (
#ifdef OS2
	tcp_avail
#else
	1
#endif /* OS2 */
	) {
	if ( myipaddr[0] )
#ifdef OS2ONLY
	  printf(" TCP/IP [%s] via %s\n", myipaddr, tcpname);
#else
	  printf(" TCP/IP [%s]\n",myipaddr);
#endif /* OS2ONLY */
	else
#ifdef OS2ONLY
	  printf(" TCP/IP via %s\n", tcpname);
#else
	  printf(" TCP/IP\n");
#endif /* OS2ONLY */

#ifdef TNCODE
	if (nettype == NET_TCPB) {
	    printf("\n");
	    shotel();
	}
#endif /* TNCODE */
#ifdef OS2
    } else {
        printf(" TCP/IP - not available%s\n",tcpname[0] ? tcpname : "" );
#endif /* OS2 */
    }
#endif /* TCPSOCKET */

#ifdef CK_NETBIOS 
    if (netbiosAvail && nettype == NET_BIOS) {
       printf("\n") ;
       shonb();
    } 
#endif /* CK_NETBIOS */

#endif /* VMS */

    printf("\nActive network connection:\n");

    if (network) {
	printf(" Host: %s",ttname);
	if ((nettype == NET_TCPA || nettype == NET_TCPB) && *ipaddr)
	  printf(" [%s]",ipaddr);
    } else
      printf(" Host: none");
    printf(" via: ");
    if (nettype == NET_TCPA || nettype == NET_TCPB)
      printf("tcp/ip\n");
    else if (nettype == NET_SX25)
      printf("SunLink X.25\n");
    else if (nettype == NET_VX25)
      printf("Stratus VOS X.25\n");
    else if (nettype == NET_DEC) {
	if ( ttnproto == NP_LAT )
	  printf("DECnet LAT\n");
	else if ( ttnproto == NP_CTERM )
	  printf("DECnet CTERM\n");
	else
	  printf("DECnet\n");
    } else if (nettype == NET_PIPE)
      printf("Named Pipes\n");
    else if (nettype == NET_BIOS)
      printf("NetBIOS\n");
    else if (nettype == NET_SLAT)
      printf("SuperLAT\n");

#ifdef NETFILE
    else if ( nettype == NET_FILE )
      printf("local file\n");
#endif /* NETFILE */

#ifdef ANYX25
    if (nettype == NET_SX25 || nettype == NET_VX25)
      shox25();
#endif /* ANYX25 */

#ifdef TCPSOCKET
    if (nettype == NET_TCPA || nettype == NET_TCPB) {
#ifdef SOL_SOCKET
#ifdef SO_KEEPALIVE
	printf(" Keepalive is %s\n", tcp_keepalive ? "on" : "off" );
#endif /* SO_KEEPALIVE */

#ifdef SO_LINGER
	printf(" Linger is %s", tcp_linger ? "on, " : "off\n" );
	if (tcp_linger) {
	    if (tcp_linger_tmo)
	      printf("%d x 10 milliseconds\n",tcp_linger_tmo);
	    else
	      printf("no timeout\n");
	}
#endif /* SO_LINGER */

#ifdef TCP_NODELAY
	printf(" Nodelay is %s\n", tcp_nodelay ? "on" : "off");
#endif /* TCP_NODELAY */

#ifdef SO_SNDBUF
	if (tcp_sendbuf <= 0)
	  printf(" Send buffer is default size\n");
	else
	  printf(" Send buffer is %d bytes\n", tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF 
	if (tcp_recvbuf <= 0)
	  printf(" Receive buffer is default size\n");
	else
	  printf(" Receive buffer is %d bytes\n", tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
    }

#ifdef RLOGCODE
    if (ttnproto == NP_RLOGIN)
      printf(" LOGIN protocol\n");
#endif /* RLOGCODE */

#ifdef TNCODE
    if (ttnproto == NP_TELNET) {
	printf(" TELNET protocol\n");
	printf(" Echoing is currently %s\n",duplex ? "local" : "remote");
    }
#endif /* TNCODE */
#endif /* TCPSOCKET */

    printf("\n");
#endif /* NETCONN */
}

#ifndef NODIAL

VOID
shodial() {
    if (mdmtyp >= 0 || local != 0) doshodial();
}

VOID
shods(s) char *s; {			/* Show a dial-related string */
    char c;
    if (s == NULL || !(*s)) {		/* Empty? */
	printf("(none)\n");
    } else {				/* Not empty. */
	while (c = *s++)		/* Can contain controls */
	  if (c > 31 && c < 127) { putchar(c); } /* so display them */
	  else printf("\\{%d}",c);	/* in backslash notation */
	printf("\n");
    }
}

int
doshodial() {

    int i, n;

    printf(" Dial status:  %d", dialsta);

#ifdef BIGBUFOK
    if (dialsta > 90)
      printf(" = Unknown error");
    else if (dialsta < 0)
      printf(" = (none)");
    else if (dialsta < 30 && dialmsg[dialsta])
      printf(" = %s", dialmsg[dialsta]);
#endif /* BIGBUFOK */

    if (ndialdir <= 1) {
	printf("\n Dial directory: %s\n",dialdir[0] ? dialdir[0] : "(none)");
    } else {
	int i;
	printf("\n Dial directories:\n");
	for (i = 0; i < ndialdir; i++)
	  printf("%2d. %s\n",i+1,dialdir[i]);
    }
    printf(" Dial method:  ");
    if      (dialmth == XYDM_D) printf("default");
    else if (dialmth == XYDM_P) printf("pulse  ");
    else if (dialmth == XYDM_T) printf("tone   ");
    printf("         Dial sort: %s\n",dialsrt ? "on" : "off") ;
    printf(" Dial hangup:  %s             Dial display: %s\n",
	   dialhng ? "on " : "off", dialdpy ? "on" : "off") ;
    printf(" Dial retries: %-6d          Dial interval: %d\n",
	   dialrtr, dialint);
    printf(" Dial timeout: ");
    if (dialtmo > 0)
      printf("%4d sec", dialtmo);
    else
      printf("0 (auto)");
    printf("        Redial number: %s\n",dialnum ? dialnum : "(none)");
    printf(" Dial confirmation: %s        Dial convert-directory: %s\n",
	   dialcnf ? "on " : "off",
	   dialcvt ? ((dialcvt == 1) ? "on" : "ask") : "off");
    printf(
" Dial prefix:                  %s\n", dialnpr ? dialnpr : "(none)");
    printf(
" Dial suffix:                  %s\n", dialsfx ? dialsfx : "(none)");
    printf(
" Dial country-code:            %-12s", diallcc ? diallcc : "(none)");
    printf("Dial connect:  %s", dialcon ? ((dialcon == 1) ? "on" : "auto")
	   : "off");
    if (dialcon != CAR_OFF)
      printf(" %s", dialcq ? "quiet" : "verbose");
    printf(
"\n Dial area-code:               %-12s", diallac ? diallac : "(none)");
    printf("Dial restrict: ");
    if (dialrstr == 5) printf("international\n");
    else if (dialrstr == 4) printf("long-distance\n");
    else if (dialrstr == 2) printf("local\n");
    else if (dialrstr == 6) printf("none\n");
    else printf("?\n");
    printf(
" Dial ld-prefix:               %s\n", dialldp ? dialldp : "(none)");
    printf(
" Dial ld-suffix:               %s\n", diallds ? diallds : "(none)");
    printf(
" Dial intl-prefix:             %s\n", dialixp ? dialixp : "(none)");
    printf(
" Dial intl-suffix:             %s\n", dialixs ? dialixs : "(none)");
    printf(
" Dial toll-free-area-code:     ");
    if (ntollfree == 0)
      printf("(none)");
    else
      for (i = 0; i < ntollfree; i++)
	printf("%s ", dialtfc[i]);
    printf(
"\n Dial toll-free-prefix:        %s\n",
	   dialtfp ? dialtfp :
	  (dialldp ? dialldp : "(none)")
	  );
    printf(
" Dial pbx-exchange:            %s\n", dialpxx ? dialpxx : "(none)");
    printf(
" Dial pbx-internal-prefix:     %s\n", dialpxi ? dialpxi : "(none)");
    printf(
" Dial pbx-outside-prefix:      %s\n", dialpxo ? dialpxo : "(none)");
    return(0);
}
#endif /* NODIAL */

#ifdef ANYX25
VOID
shopad() {
    int i;
    printf("\nX.3 PAD Parameters:\n");
    for (i = 0; i < npadx3; i++)
      printf(" [%d] %s %d\n",padx3tab[i].kwval,padx3tab[i].kwd,
	     padparms[padx3tab[i].kwval]);
}
#endif /* ANYX25 */

/*  Show File Parameters */

VOID
shoparf() {
    char *s; int i;
#ifdef COMMENT
    printf("\nFile parameters:       ");
#ifdef COMMENT
    printf("Blocksize:     %5d      ",fblksiz);
#endif /* COMMENT */
    printf(" Attributes:       ");
    if (atcapr) printf("on"); else printf("off");
#ifdef VMS
    printf("  Record-Length: %5d",frecl);
#endif /* VMS */
    printf("\n Names:   ");
    if (fncnv == XYFN_L)
      s = "literal";
    else if (fncnv == XYFN_C)
      s = "converted";
    else s = "(unknown)"
    printf("%-12s",s);
#ifdef DEBUG
#ifndef MAC
    printf("  Debugging Log:    ");
    if (deblog) printf("%s",debfil); else printf("none");
#endif /* MAC */
#endif /* DEBUG */

    printf("\n Type:    ");
    s = gfmode(binary);
    printf("%-12s",s);
#ifdef COMMENT
    printf(" Organization:  ");
    switch (forg) {
      case XYFO_I: printf("%-10s","indexed"); break;
      case XYFO_R: printf("%-10s","relative"); break;
      case XYFO_S: printf("%-10s","sequential"); break;
    }
#endif /* COMMENT */
#ifndef MAC
    printf("  Packet Log:       ");
    if (pktlog) printf(pktfil); else printf("none");
#endif /* MAC */
#ifdef UNIX
    printf("  Longest filename: %d",maxnam);
#endif /* UNIX */
    printf("\n Collide: ");
    for (i = 0; i < ncolx; i++)
      if (colxtab[i].kwval == fncact) break;
    printf("%-12s", (i == ncolx) ? "unknown" : colxtab[i].kwd);

#ifdef COMMENT
    printf(" Format:        ");
    switch (frecfm) {
      case XYFF_F:  printf("%-10s","fixed"); break;
      case XYFF_VB: printf("%-10s","rcw"); break;
      case XYFF_S:  printf("%-10s","stream"); break;
      case XYFF_U:  printf("%-10s","undefined"); break;
      case XYFF_V:  printf("%-10s","variable"); break;
    }
#endif /* COMMENT */
#ifndef MAC
    printf("  Session Log:      ");
    if (seslog) printf(sesfil); else printf("none");
#endif /* MAC */
#ifdef UNIX
    printf("  Longest pathname: %d",maxpath);
#endif /* UNIX */
    printf("\n Send Pathnames: %s", fnspath ? "off" : " on");
    printf("    Receive Pathnames: %s\n", fnrpath ? "off" : "on");
    printf(" Display: ");
    switch (fdispla) {
      case XYFD_N: printf("%-12s","none"); break;
      case XYFD_R: printf("%-12s","serial"); break;
      case XYFD_C: printf("%-12s","fullscreen"); break;
      case XYFD_S: printf("%-12s","crt"); break;
    }
#ifdef COMMENT
    printf("Carriage-Control: ");
    switch (fcctrl) {
      case XYFP_F: printf("%-10s","fortran"); break;
      case XYFP_N: printf("%-10s","newline"); break;
      case XYFP_P: printf("%-10s","machine"); break;
      case XYFP_X: printf("%-10s","none"); break;
    }
#endif /* COMMENT */
#ifdef TLOG
#ifndef MAC
    printf("  Transaction Log:  ");
    if (tralog) printf(trafil); else printf("none");
#endif /* MAC */
#endif /* TLOG */
#ifndef NOCSETS
    if (binary == XYFT_T) {
	shocharset();
    } else
#endif /* NOCSETS */
      printf("\n");
    printf("\nByte Size: %d",(fmask == 0177) ? 7 : 8);
    printf(", Incomplete: ");
    if (keep) printf("keep"); else printf("discard");
#ifdef KERMRC
    printf(", Init file: %s",
#ifdef CK_SYSINI
	   CK_SYSINI
#else
	   kermrc
#endif /* CK_SYSINI */
	   );
#endif /* KERMRC */
    printf("\n");

#else /* not COMMENT -- new format */

#ifdef VMS
    printf(" File record-Length: %5d\n",frecl);
#endif /* VMS */

    printf(" Transfer mode:      %s\n",
	   xfermode == XMODE_A ?
	   "automatic" :
	   "manual"
	   );

    printf(" File type:          ");
    switch (binary) {
      case XYFT_T: s = "text";	       break;
#ifdef VMS
      case XYFT_B: s = "binary fixed"; break;
      case XYFT_I: s = "image";        break;
      case XYFT_L: s = "labeled";      break;
      case XYFT_U: s = "binary undef"; break;
#else
#ifdef MAC
      case XYFT_B: s = "binary";       break;
      case XYFT_M: s = "macbinary";    break;
#else
      case XYFT_B: s = "binary";       break;
#ifdef CK_LABELED
      case XYFT_L: s = "labeled";      break;
#endif /* CK_LABELED */
#endif /* MAC */
#endif /* VMS */
      default: s = "?"; break;
    }
    printf("%s\n",s);
    if (fncnv == XYFN_L)
      s = "literal";
    else if (fncnv == XYFN_C)
      s = "converted";
    else
      s = "(unknown)";
    printf(" File names:         %s\n",s);
    printf(" Send pathnames:     %s\n", fnspath ? "off" : "on");
    printf(" Receive pathnames:  %s\n", fnrpath ? "off" : "on");

    printf(" File collision:     ");
    for (i = 0; i < ncolx; i++)
      if (colxtab[i].kwval == fncact) break;
    printf("%s\n", (i == ncolx) ? "unknown" : colxtab[i].kwd);
    printf(" File destination:   %s\n",
	   (dest == DEST_D) ? "disk" :
	   ((dest == DEST_S) ? "screen" : "printer")
	   );
    printf(" File incomplete:    %s\n", keep ? "keep" : "discard");
    printf(" File bytesize:      %d\n",(fmask == 0177) ? 7 : 8);
#ifndef NOCSETS
    printf(" File character-set: %s\n",fcsinfo[fcharset].keyword);
#endif /* NOCSETS */

    printf(" File end-of-line:   ");
    switch (feol) {
      case XYFA_C: printf("%s\n","cr"); break;
      case XYFA_L: printf("%s\n","lf"); break;
      case XYFA_2: printf("%s\n","crlf"); break;
    }
    printf(" File display:       ");
    switch (fdispla) {
      case XYFD_N: printf("%s\n","none"); break;
      case XYFD_R: printf("%s\n","serial"); break;
      case XYFD_C: printf("%s\n","fullscreen"); break;
      case XYFD_S: printf("%s\n","crt"); break;
    }
#ifndef MAC
#ifdef DEBUG
    printf(" Debug log:          %s\n", deblog ? debfil : "(none)");
#endif /* DEBUG */
    printf(" Packet log:         %s\n", pktlog ? pktfil : "(none)");
    printf(" Session log:        %s\n", seslog ? sesfil : "(none)");
#ifdef TLOG
    printf(" Transaction log:    %s\n", tralog ? trafil : "(none)");
#endif /* TLOG */
#endif /* MAC */

#ifdef KERMRC
    printf("\n Initialization file:     %s\n", noinit ? "(none)" :
#ifdef CK_SYSINI
	   CK_SYSINI
#else
	   kermrc
#endif /* CK_SYSINI */
	   );
#endif /* KERMRC */
#ifdef CK_TMPDIR
    printf(" File download-directory: %s\n", dldir ? dldir : "(none)");
    i = 256;
    s = line;
    zzstring("\\v(tmpdir)",&s,&i);
    printf(" Temporary directory:     %s\n", line);
#endif /* CK_TMPDIR */

#ifdef OS2ORUNIX
    printf(" Longest filename:        %d\n", maxnam);
    printf(" Longest pathname:        %d\n", maxpath);
#endif /* OS2ORUNIX */

#endif /* COMMENT */
}

VOID
shoparp() {
    char *s;

#ifdef CK_TIMERS
    extern int rttflg;
#endif /* CK_TIMERS */

    printf("\nProtocol: %s\n",ptab[protocol].p_name);

    if (protocol == PROTO_K) {
	printf("\nProtocol Parameters:   Send    Receive");
	if (timef)
	  printf("\n Timeout (used=%2d):%7d*%8d ", timint, rtimo, pkttim);
	else
	  printf("\n Timeout (used=%2d):%7d%9d ",  timint, rtimo, pkttim);
#ifndef NOSERVER
	printf("       Server Timeout:%4d",srvtim);
#endif /* NOSERVER */
	printf("\n Padding:      %11d%9d", npad,   mypadn);
	if (bctr == 4)
	  printf("        Block Check: blank-free-2\n");
	else
	  printf("        Block Check: %6d\n",bctr);
	printf(  " Pad Character:%11d%9d", padch,  mypadc);
	printf("        Delay:       %6d\n",delay);
	printf(  " Packet Start: %11d%9d", mystch, stchr);
	printf("        Max Retries: %6d\n",maxtry);
	printf(  " Packet End:   %11d%9d", seol,   eol);
	if (ebqflg)
	  printf("        8th-Bit Prefix: '%c'",ebq);
	printf(  "\n Packet Length:%11d ", spmax);
	printf("%8d     ",  urpsiz);
	if (rptflg)
	  printf("   Repeat Prefix:  '%c'",rptq);
	printf(  "\n Maximum Length: %9d%9d", maxsps, maxrps);
	printf("        Window Size:%7d set, %d used\n",wslotr,wmax);
	printf(    " Buffer Size:  %11d%9d", bigsbsiz, bigrbsiz);
	printf("        Locking-Shift:    ");
	if (lscapu == 2) {
	    printf("forced");
	} else {
	    printf("%s", (lscapr ? "enabled" : "disabled"));
	    if (lscapr) printf(",%s%s", (lscapu ? " " : " not "), "used");
	}
	printf("\n\n");
#ifdef CK_TIMERS
	if (rttflg) {
	    extern int mintime, maxtime;
	    printf(" Packet timeouts: dynamic %d:%d\n", mintime, maxtime);
	} else {
	    printf(" Packet timeouts: fixed\n");
	}
#endif /* CK_TIMERS */
        if (!(s = ptab[protocol].h_b_init))
	  s = "";
	printf(" Auto-upload command (binary):  %s\n",
	       *s ? s : "(none)");
        if (!(s = ptab[protocol].h_t_init))
	  s = "";
	printf(" Auto-upload command (text):    %s\n",
	       *s ? s : "(none)");

#ifndef NOCSETS
	printf(" Transfer character-set: ");
	if (tcharset == TC_TRANSP)
	  printf("transparent");
	else
	  printf("%s\n", tcsinfo[tcharset].keyword );
#endif /* NOCSETS */
	printf("\n Transfer mode: %s\n", xfermode == XMODE_A ?
	       "automatic" :
	       "manual"
	       );
	printf(" Transfer slow-start: %s\n",showoff(slostart));
	printf(" Attributes: %s\n",showoff(atcapr));
    }

#ifdef CK_XYZ

#ifdef XYZ_INTERNAL
    
    if (protocol != PROTO_K) {
	int i;
	int x;
	printf(" Transfer mode: %s\n", binary ? "binary" : "text");
        if (protocol == PROTO_Z) {		/* Zmodem */
            printf(" Window size:   ");
            if (ptab[protocol].winsize < 1)
              printf("none\n");
            else
              printf("%d\n",wslotr);
#ifdef COMMENT
            printf(" Packet (frame) length: ");
            if (ptab[protocol].spktlen < 0)
              printf("none\n");
            else
              printf("%d\n",spmax);
#endif /* COMMENT */
        } else {
            if (ptab[protocol].spktlen >= 1000)
              printf(" 1K packets\n");
            else
              printf(" 128-byte packets\n");
        }
	printf(" Pathname stripping when sending:   %s\n",
               showoff(ptab[protocol].fnsp)
               );
	printf(" Pathname stripping when receiving: %s\n",
               showoff(ptab[protocol].fnrp)
               );
	printf(" Filename collision action:         ");
	for (i = 0; i < ncolx; i++)
          if (colxtab[i].kwval == fncact) break;
	printf("%-12s", (i == ncolx) ? "unknown" : colxtab[i].kwd);

	printf("\n Escape control characters:          ");
	x = ptab[protocol].prefix;
	if (x == PX_ALL)
	  printf("all\n");
	else if (x == PX_CAU || x==PX_WIL)
	  printf("minimal\n");
	else
	  printf("none\n");
        if (!(s = ptab[protocol].h_b_init))
	  s = "";
	printf(" Autoreceive command (binary): %s\n", *s ? s : "(none)");
        if (!(s = ptab[protocol].h_t_init))
	  s = "";
	printf(" Autoreceive command (text):   %s\n", *s ? s : "(none)");
    }
#else
    if (protocol != PROTO_K) {
	printf("\nExecuted by external commands:\n\n");
	s = ptab[protocol].p_b_scmd; 
	if (!s) s = "";
	printf(" SEND command (binary):        %s\n", *s ? s : "(none)");
	s = ptab[protocol].p_t_scmd; 
	if (!s) s = "";
	printf(" SEND command (text):          %s\n", *s ? s : "(none)");
	s = ptab[protocol].p_b_rcmd; 
	if (!s) s = "";
	printf(" RECEIVE command (binary):     %s\n", *s ? s : "(none)");
	s = ptab[protocol].p_t_rcmd; 
	if (!s) s = "";
	printf(" RECEIVE command (text):       %s\n", *s ? s : "(none)");
	s = ptab[protocol].h_b_init; 
	if (!s) s = "";
	printf(" Autoreceive command (binary): %s\n", *s ? s : "(none)");
	s = ptab[protocol].h_t_init; 
	if (!s) s = "";
	printf(" Autoreceive command (text):   %s\n", *s ? s : "(none)");
    }
#endif /* XYZ_INTERNAL */
#endif /* CK_XYZ */
}


#ifndef NOCSETS
VOID
shoparl() {
#ifdef COMMENT
    int i;
/* Misleading... */
    printf("\nAvailable Languages:\n");
    for (i = 0; i < MAXLANG; i++) {
	printf(" %s\n",langs[i].description);
    }
#else
    printf("\nLanguage-specific translation rules: %s\n",
	   language == L_USASCII ? "none" : langs[language].description);
    shocharset();
    printf("\n\n");
#endif /* COMMENT */
}

VOID
shocharset() {
    int x;
    printf("\n File Character-Set: %s (%s), ",
	   fcsinfo[fcharset].keyword,
	   fcsinfo[fcharset].name
	   );
    if ((x = fcsinfo[fcharset].size) == 128) printf("7-bit");
    else if (x == 256) printf("8-bit");
    else printf("multibyte");
    printf("\n Transfer Character-Set");
#ifdef COMMENT
    if (tslevel == TS_L2)
      printf(": (international)");
    else
#endif /* COMMENT */
    if (tcharset == TC_TRANSP)
      printf(": Transparent");
    else
      printf(": %s (%s)",tcsinfo[tcharset].keyword, tcsinfo[tcharset].name);
}
#endif /* NOCSETS */

VOID
shopar() {
    printf("SHOW what?\n");
}
#endif /* NOSHOW */

/*  D O S T A T  --  Display file transfer statistics.  */

int
dostat() {
    extern long filrej;
    extern char whoareu[];
    printf("\nMost recent transaction --\n");
    if (whoareu[0])
      printf(" remote system type     : %s\n", getsysid((char *)whoareu));
    printf(" files transferred      : %ld\n",filcnt - filrej);
    printf(" files not transferred  : %ld\n",filrej);
    printf(" characters last file   : %ld\n",ffc);
    printf(" total file characters  : %ld\n",tfc);
    printf(" communication line in  : %ld\n",tlci);
    printf(" communication line out : %ld\n",tlco);
    printf(" packets sent           : %d\n", spackets);
    printf(" packets received       : %d\n", rpackets);
    printf(" damaged packets rec'd  : %d\n", crunched);
    printf(" timeouts               : %d\n", timeouts);
    printf(" retransmissions        : %d\n", retrans);
    if (filcnt > 0) {
	printf(" parity                 : %s",parnam((char)parity));
	if (autopar) printf(" (detected automatically)");
	printf("\n control characters     : %ld prefixed, %ld unprefixed\n",
	       ccp, ccu);
	printf(" 8th bit prefixing      : ");
	if (ebqflg) printf("yes [%c]\n",ebq); else printf("no\n");
	printf(" locking shifts         : %s\n", lscapu ? "yes" : "no");
	printf(" window slots used      : %d of %d\n", wmax, wslotr);
	printf(" packet length          : %d (send), %d (receive)\n",
	       spmax, urpsiz);
	printf(" compression            : ");
	if (rptflg)
	  printf("yes [%c] (%ld)\n",(char) rptq,rptn);
	else
	  printf("no\n");
	if (bctu == 4)
	  printf(" block check type used  : blank-free-2\n");
	else
	  printf(" block check type used  : %d\n",bctu);
	printf(" elapsed time           : %d sec\n",tsecs);
	if (speed <= 0L) speed = ttgspd();
	if (speed > 0L) {
	    if (speed == 8880)
	      printf(" transmission rate      : 75/1200 bps\n");
	    else
	      printf(" transmission rate      : %ld bps\n",speed);
	}
	if (tsecs > 0) {		/* No dividing by zero...   */
	    long ecps;			/* Effective data rate, cps */
	    int eff;			/* Percent efficiency */
	    ecps = tfc / (long) tsecs;
	    if (local && !network &&	/* Only makes sense for */
		mdmtyp == 0 &&		/* direct serial connections */
		speed > 99L &&		/* when we really know the speed */
		speed != 8880L
		) {
		eff = (((ecps * 100L) / (speed / 100L)) + 5L) / 10L;
		printf(" effective data rate    : %ld cps (%d%%)\n",ecps,eff);
	    } else
		printf(" effective data rate    : %ld cps\n", ecps);
	}
    }
    return(1);
}

#ifndef NOSPL

/* The INPUT command */

/* Output buffering for "doinput" */

#ifdef pdp11
#define MAXBURST 16		/* Maximum size of input burst */
#else
#define	MAXBURST 1024
#endif /* pdp11 */
#ifdef OSK
static CHAR *conbuf;		/* Buffer to hold output for console */
#else
static CHAR conbuf[MAXBURST];	/* Buffer to hold output for console */
#endif /* OSK */
static int concnt = 0;		/* Number of characters buffered */
#ifdef OSK
static CHAR *sesbuf;		/* Buffer to hold output for session log */
#else
static CHAR sesbuf[MAXBURST];	/* Buffer to hold output for session log */
#endif /* OSK */
static int sescnt = 0;		/* Number of characters buffered */

static VOID				/* Flush INPUT echoing */
myflsh() {				/* and session log output. */
    if (concnt > 0) {
	conxo(concnt, (char *) conbuf);
	concnt = 0;
    }
    if (sescnt > 0) {
	if (zsoutx(ZSFILE, (char *) sesbuf, sescnt) < 0)
	  seslog = 0;
	sescnt = 0;
    }
}

/* Execute the INPUT and MINPUT commands */

int instatus = -1;

int
doinput(timo,ms) int timo; char *ms[]; {
    int x, y, i, t, rt, icn, anychar, mi[MINPMAX];
    int lastchar = 0;
    char *xp, *s;
    CHAR c;
#define CK_BURST
/*
  This enables the INPUT speedup code, which depends on ttchk() returning
  accurate information.  If INPUT fails with this code enabled, change the
  above "#define" to "#undef".
*/
#ifdef CK_BURST
    int burst = 0;			/* Chars remaining in input burst */
#endif /* CK_BURST */

    instatus = INP_IE;			/* 3 = internal error */

#ifdef OSK
    if (conbuf == NULL) {
	if ((conbuf = (CHAR *)malloc(MAXBURST*2)) == NULL) {
	    return(0);
	}
	sesbuf = conbuf + MAXBURST;
    }
#endif /* OSK */

#ifndef NOLOCAL
    if (local) {			/* Put line in "ttvt" mode */
	y = ttvt(speed,flow);		/* if not already. */
	if (y < 0) {
	    printf("?Can't condition line for INPUT\n");
	    return(0);			/* Watch out for failure. */
	}
    }
#endif /* NOLOCAL */

    if (!ms[0]) {			/* If we were passed a NULL pointer */
	anychar = 1;			/*  ... */
    } else {
        y = (int)strlen(ms[0]);		/* Or if search string is empty */
        anychar = (y < 1);		/* any input character will do. */
    }
#ifndef NODEBUG
    debug(F101,"doinput","",anychar);
    y = -1;
    while (ms[++y]) debug(F111,"  ",ms[y],strlen(ms[y]));
    debug(F101,"doinput timo","",timo);
    debug(F101,"doinput echo","",inecho);
#endif /* NODEBUG */

    x = 0;				/* Return code, assume failure */
    instatus = INP_TO;			/* Status, assume timeout */

    for (y = 0; y < MINPMAX; y++)
      mi[y] = 0;			/* String pattern match position */

    if (!inpcas[cmdlvl]) {		/* INPUT CASE = IGNORE?  */
	y = -1;

	while(xp = ms[++y]) {
	    while (*xp) {               /* Convert to lowercase */
		if (isupper(*xp)) *xp = (char) tolower(*xp);
		xp++;
	    }
        }
    }
#ifdef COMMENT
    inpbps = inpbp;			/* Save current pointer. */
#endif /* COMMENT */
    rtimer();				/* Reset timer. */
    t = 0;				/* Time now is 0. */
    m_found = 0;			/* Default to timed-out */
    incount = 0;			/* Character counter */

    rt = 1;				/* One-second intervals */
    while (1) {				/* Character-getting loop */
#ifdef SCRIPTTERM 
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
	    (apcactive == APC_REMOTE && apcstatus != APC_OFF)) {
	    domac("apc_commands",apcbuf,CF_APC);
	    apcactive = APC_INACTIVE ;
	}
#endif /* CK_APC */
#endif /* SCRIPTTERM */
	if (local) {			/* One case for local */
	    y = ttinc(rt);		/* Get character from comm device */
	    debug(F101,"input ttinc(rt) returns","",y);
	    if (y < -1) {		/* Connection failed. */
		instatus = INP_IO;	/* Status = i/o error */
		return(0);
	    }		
	    if (icn = conchk()) {	/* Interrupted from keyboard? */
		debug(F101,"input interrupted from keyboard","",icn);
		while (icn--) coninc(0); /* Yes, absorb what was typed. */
		instatus = INP_UI;	/* Fail and remember why. */
		break;
	    }
	} else {			/* Another for remote */
	    y = coninc(rt);
	    debug(F101,"input coninc(rt) returns","",y);
	}
	if (y > -1) {			/* A character arrived */
#ifndef SCRIPTTERM
#ifdef TNCODE
/* Check for telnet protocol negotiation */
	    if (network && (ttnproto == NP_TELNET) && ((y & 0xff) == IAC)) {
		myflsh();	/* Break from input burst for tn_doop() */
#ifdef CK_BURST
		burst = 0;
#endif /* CK_BURST */
		switch (tn_doop((CHAR)(y & 0xff),duplex,ttinc)) {
		  case 2: duplex = 0; continue;
		  case 1: duplex = 1;
		  default: continue;
		}
	    }
#endif /* TNCODE */
#else  /* SCRIPTTERM */
	    scriptwrtbuf(y);		/* Handles Telnet negotiations */
#endif /* SCRIPTTERM */

	    /* Real input character to be checked */

#ifdef CK_BURST
	    burst--;			/* One less character waiting */
#endif /* CK_BURST */
	    c = (CHAR) (cmask & (CHAR) y); /* Mask off parity */

	    inchar[0] = c;		/* Remember character for \v(inchar) */
#ifdef CK_BURST
	    /* Update "lastchar" time only once during input burst */
	    if (burst <= 0)
#endif /* CK_BURST */
	      lastchar = gtimer();	/* Remember when it came */

	    if (c == '\0') {		/* NUL, we can't use it */
		if (anychar) {		/* Except if any character will do? */
		    x = 1;		/* Yes, done. */
		    incount = 1;	/* This must be the first and only. */
		    break;
		} else continue;	/* Otherwise continue INPUTting */
	    }
	    *inpbp++ = c;		/* Store char in circular buffer */
	    incount++;			/* Count it for \v(incount) */

	    if (inpbp >= inpbuf + inbufsize) { /* Time to wrap around? */
		*inpbp = NUL ;		/* Make it null-terminated */
		inpbp = inpbuf;		/* Yes. */
	    }
#ifdef MAC
	    {
		extern char *ttermw;	/* fake pointer cast */
		if (inecho) {
		    outchar(ttermw, c);	/* echo to terminal window */
		    /* this might be too much overhead to do here ? */
		    updatecommand(ttermw);
		}
	    }
#else /* Not MAC */
	    if (inecho) conbuf[concnt++] = c; /* Buffer console output */
#endif /* MAC */
#ifndef SCRIPTTERM
        if (seslog) {
#ifdef UNIX
		if (sessft != 0 || c != '\r')
#else
#ifdef OSK
		if (sessft != 0 || c != '\012')
#endif /* OSK */
#endif /* UNIX */
		  sesbuf[sescnt++] = c;	/* Buffer session log output */
	    }
#endif /* SCRIPTTERM */
	    if (anychar) {		/* Any character will do? */
		x = 1;
		break;
	    }
	    if (!inpcas[cmdlvl]) {	/* Ignore alphabetic case? */
		if (isupper(c))		/* Yes, convert input char to lower */
		  c = (CHAR) tolower(c);
	    }
	    debug(F000,"doinput char","",c);
	    y = -1;			/* Loop thru search strings */
	    while (s = ms[++y]) {	/* ...as many as we have. */
		i = mi[y];		/* Match-position in this one. */
		debug(F000,"compare char","",(CHAR)s[i]);
		if (c == (CHAR) s[i]) { /* Check for match */
		    i++;		/* Got one, go to next character */
		} else {		/* Don't have a match */
		    int j;
		    for (j = i; i > 0; ) { /* Back up in search string */
			i--; /* (Do this here to prevent compiler foulup) */
			/* j is the length of the substring that matched */
			if (c == (CHAR) s[i]) {
			    if (!strncmp(s,&s[j-i],i)) {
				i++;          /* c actually matches -- cfk */
				break;
			    }
			}
		    }
		}
		if ((CHAR) s[i] == (CHAR) '\0') { /* Matched to end? */
		    x = 1;		/* Yes, */
		    break;		/* done. */
		}
		mi[y] = i;		/* No, remember match-position */
            }
	    if (x == 1) {		/* Set \v(minput) result */
		m_found = y + 1;
		break;
	    }
        }
#ifdef CK_BURST
	if (burst <= 0) {
	    myflsh();			/* Flush buffered output */
	    if (local) {		/* Get size of next input burst */
		burst = ttchk();
		if (icn = conchk()) {	/* Interrupted from keyboard? */
		    debug(F101,"input interrupted from keyboard","",icn);
		    while (icn--) coninc(0); /* Yes, absorb what was typed. */
		    break;		/* And fail. */
		}
	    } else {
		burst = conchk();
	    }
	    /* Prevent overflow of "conbuf" and "sesbuf" */
	    if (burst > MAXBURST)
	      burst = MAXBURST;

	    /* Did not match, timer exceeded? */
	    if (((t = gtimer()) >= timo) && (timo > -1))
	      break;
	    else if (insilence > 0 && (t - lastchar) > insilence)
	      break;
	}
#else
	myflsh();			/* Flush buffered output */
	/* Did not match, timer exceeded? */
	if (((t = gtimer()) >= timo) && (timo > -1))
	  break;
	else if (insilence > 0 && (t - lastchar) > insilence)
	  break;
#endif /* CK_BURST */
    }					/* Still have time left, continue. */
    myflsh();				/* Flush buffered output. */
    if (x > 0)
      instatus = 0;
    return(x);				/* Return the return code. */
}
#endif /* NOSPL */

#ifndef NOSPL
/* REINPUT Command */

/*
  Note, the timeout parameter is required, but ignored.  Syntax is compatible
  with MS-DOS Kermit except timeout can't be omitted.  This function only
  looks at the characters already received and does not read any new
  characters from the connection.
*/
int
doreinp(timo,s) int timo; char *s; {
    int x, y, i;
    char *xx, *xp, *xq = (char *)0;
    CHAR c;

    y = (int)strlen(s);
    debug(F111,"doreinput",s,y);

    if (y > inbufsize)			/* If search string longer than */
      return(0);			/* input buffer, fail. */

    x = 0;				/* Return code, assume failure */
    i = 0;				/* String pattern match position */

    if (!inpcas[cmdlvl]) {		/* INPUT CASE = IGNORE?  */
	xp = malloc(y+2);		/* Make a separate copy of the */
	if (!xp) {			/* search string. */
	    printf("?malloc error 6\n");
	    return(x);
	} else xq = xp;			/* Keep pointer to beginning. */
	while (*s) {			/* Yes, convert to lowercase */
	    *xp = *s;
	    if (isupper(*xp)) *xp = (char) tolower(*xp);
	    xp++; s++;
	}
	*xp = NUL;			/* Terminate it! */
	s = xq;				/* Move search pointer to it. */
    }
    xx = inpbp;				/* Current INPUT buffer pointer */
    do {
	c = *xx++;			/* Get next character */
	if (xx >= inpbuf + inbufsize)	/* Wrap around if necessary */
	  xx = inpbuf;
	if (!inpcas[cmdlvl]) {		/* Ignore alphabetic case? */
	    if (isupper(c)) c = (CHAR) tolower(c); /* Yes */
	}
	debug(F000,"doreinp char","",c);
	debug(F000,"compare char","",(CHAR) s[i]);
	if (((char) c) == ((char) s[i])) { /* Check for match */
	    i++;			/* Got one, go to next character */
	} else {			/* Don't have a match */
   	    int j;
   	    for (j = i; i > 0; ) {	/* [jrs] search backwards for it  */
		i--;
   		if (((char) c) == ((char) s[i])) {
		    if (!strncmp(s,&s[j-i],i)) {
			i++;
			break;
		    }
		}
   	    }
   	}				/* [jrs] or return to zero from -1 */
	if (s[i] == '\0') {		/* Matched all the way to end? */
	    x = 1;			/* Yes, */
	    break;			/* done. */
	}
    } while (xx != inpbp);		/* Until back where we started. */

    if (!inpcas[cmdlvl]) if (xq) free(xq); /* Free this if it was malloc'd. */
    return(x);				/* Return search result. */
}
#ifndef NOSPL

#endif /* NOSPL */
/*  X X S T R I N G  --  Interpret strings containing backslash escapes  */
/*  Z Z S T R I N G  --  (new name...)  */
/*
 Copies result to new string.
  strips enclosing braces or doublequotes.
  interprets backslash escapes.
  returns 0 on success, nonzero on failure.
  tries to be compatible with MS-DOS Kermit.

 Syntax of input string:
  string = chars | "chars" | {chars}
  chars = (c*e*)*
  where c = any printable character, ascii 32-126
  and e = a backslash escape
  and * means 0 or more repetitions of preceding quantity
  backslash escape = \operand
  operand = {number} | number | fname(operand) | v(name) | $(name) | m(name)
  number = [r]n[n[n]]], i.e. an optional radix code followed by 1-3 digits
  radix code is oO (octal), xX (hex), dD or none (decimal) (see xxesc()).
*/

#ifndef NOFRILLS
int
yystring(s,s2) char *s; char **s2; {	/* Reverse a string */
    int x;
    static char *new;
    new = *s2;
    if (!s || !new) return(-1);		/* Watch out for null pointers. */
    if ((x = (int)strlen(s)) == 0) {	/* Recursion done. */
	*new = '\0';
	return(0);
    }
    x--;				/* Otherwise, call self */
    *new++ = s[x];			/* to reverse rest of string. */
    s[x] = 0;
    return(yystring(s,&new));
}
#endif /* NOFRILLS */

#define FNVALL 1024
char fnval[FNVALL+2];			/* Return value */

static char ipabuf[16] = { NUL };	/* IP address buffer */

static char *
getip(s) char *s; {
    char c=NUL;				/* Workers... */
    int i=0, p=0, d=0;
    int state = 0;			/* State of 2-state FSA */

    while (c = *s++) {
	switch(state) {
	  case 0:			/* Find first digit */
	    i = 0;			/* Output buffer index */
	    ipabuf[i] = NUL;		/* Initialize output buffer */
	    p = 0;			/* Period counter */
	    d = 0;			/* Digit counter */
	    if (isdigit(c)) {		/* Have first digit */
		d = 1;			/* Count it */
		ipabuf[i++] = c;	/* Copy it */
		state = 1;		/* Change state */
	    }
	    break;

	  case 1:			/* In numeric field */
	    if (isdigit(c)) {		/* Have digit */
		if (++d > 3)		/* Too many */
		  state = 0;		/* Start over */
		else			/* Not too many */
		  ipabuf[i++] = c;	/* Keep it */
	    } else if (c == '.' && p < 3) { /* Have a period */
		p++;			/* Count it */
		if (d == 0)		/* Not preceded by a digit */
		  state = 0;		/* Start over */
		else			/* OK */
		  ipabuf[i++] = c;	/* Keep it */
		d = 0;			/* Reset digit counter */
	    } else if (p == 3 && d > 0) { /* Not part of address */
		ipabuf[i] = NUL;	/* If we have full IP address */
		return((char *)ipabuf);	/* Return it */
	    } else {			/* Otherwise */
		state = 0;		/* Start over */
		ipabuf[0] = NUL;	/* (in case no more chars left) */
	    }
	}
    }					/* Fall thru at end of string */
    ipabuf[i] = NUL;			/* Maybe we have one */
    return((p == 3 && d > 0) ? (char *)ipabuf : "");
}

static char *				/* Evaluate builtin function */
fneval(fn,argp,argn,xp) char *fn, *argp[]; int argn; char * xp; {
    int i, j, k, len1, len2, len3, n, x, y;
    char *bp[FNARGS];			/* Pointers to malloc'd strings */
    char c;
    char *p, *s;
    char *val1, *val2;			/* Pointers to numeric string values */

    if (!fn) fn = "";			/* Protect against null pointers */
    debug(F111,"fneval",fn,argn);
    debug(F110,"fneval",argp[0],0);
    y = lookup(fnctab,fn,nfuncs,&x);
    if (y < 0)				/* bad function name */
      return("");			/* so value is null */

#ifdef DEBUG
    if (deblog) {
	int j;
	for (j = 0; j < argn; j++)
	  debug(F111,"fneval function arg",argp[j],j);
    }
#endif /* DEBUG */

    if (y == FN_LIT) {			/* literal(arg1) */
	debug(F110,"flit",xp,0);
	return(xp ? xp : "");		/* return a pointer to arg itself */
    }
    if (y == FN_CON) {			/* Contents of variable, unexpanded. */
	char c;
	if (!(p = argp[0]) || !*p) return("");
	p = brstrip(p);
	if (*p == CMDQ) p++;
	if ((c = *p) == '%') {		/* Scalar variable. */
	    c = *++p;			/* Get ID character. */
	    p = "";			/* Assume definition is empty */
	    if (!c) return(p);		/* Double paranoia */
	    if (c >= '0' && c <= '9') { /* Digit for macro arg */
		if (maclvl < 0)		/* Digit variables are global */
		  p = g_var[c];		/* if no macro is active */
		else			/* otherwise */
		  p = m_arg[maclvl][c - '0']; /* they're on the stack */
	    } else {
		if (isupper(c)) c -= ('a'-'A');
		p = g_var[c];		/* Letter for global variable */
	    }
	    return(p ? p : "");
	}
	if (c == '&') {			/* Array reference. */
	    int vbi, d;
	    if (arraynam(p,&vbi,&d) < 0) /* Get name and subscript */
	      return("");
	    if (chkarray(vbi,d) > 0) {	/* Array is declared? */
		vbi -= ARRAYBASE;	/* Convert name to index */
		if (a_dim[vbi] >= d) {	/* If subscript in range */
		    char **ap;
		    ap = a_ptr[vbi];	/* get data pointer */
		    if (ap) {		/* and if there is one */
			return(ap[d]);	/* return what it points to */
		    }
		}
	    }
	    else return("");		/* Otherwise its enexpanded value. */
	}
    }

    for (i = 0; i < argn; i++) {	/* Not literal, expand the args */
	n = 1024;			/* allow 1K per expanded arg, yow! */
	bp[i] = s = malloc(n);		/* get the new space */
	if (bp[i] == NULL) {		/* handle failure to get space */
	    for (k = 0; k < i; k++) if (bp[k]) free(bp[k]);
	    debug(F101,"fneval malloc failure, arg","",i);
	    return("");
	}
	p = argp[i] ? argp[i] : "";	/* Point to this argument */

/*
  Trim leading and trailing spaces from the original argument, before
  evaluation.  This code new to edit 184.
*/

#ifdef COMMENT
/* Don't trim 1st REPEAT argument or second TRIM or LTRIM argument */
	if (!((y == FN_REP && i == 0) ||
	      ((y == FN_TRM || y == FN_LTR) && i == 1))
	    )
/* In edit 192 we can use braces to include spaces, commas, etc in/as args */
#endif /* COMMENT */
	{
	    int x, j;			/* All others... */
	    x = strlen(p);
	    if (*p == '{' && *(p+x-1) == '}') {
		p[x-1] = NUL;
		p++;
		x -= 2;
	    } else {
		j = x - 1;		/* Trim trailing whitespace */
		while (j > 0 && (*(p + j) == SP || *(p + j) == HT))
		  *(p + j--) = NUL;
		while (*p == SP || *p == HT) /* Strip leading whitespace */
		  p++;
	    }
	}

/* Now evaluate the argument */

	if (zzstring(p,&s,&n) < 0) {	/* Expand arg into new space */
	    debug(F101,"fneval xxstring fails, arg","",i);
	    for (k = 0; k <= i; k++)	/* Free up previous space on error */
	      if (bp[k]) free(bp[k]);
	    return("");			/* and return null string. */
	}
	debug(F111,"fneval arg",bp[i],i);
    }

#ifdef DEBUG
    if (deblog) {
	int j;
	for (j = 0; j < argn; j++) {
	    debug(F111,"fneval arg post eval",argp[j],j);
	    debug(F111,"fneval evaluated arg",bp[j],j);
	}
    }
#endif /* DEBUG */

    switch (y) {			/* Do function on expanded args */

      case FN_DEF:
	k = mlook(mactab,bp[0],nmac);	/* def(arg1) - Return a macro def */
	p = (k > -1) ? mactab[k].mval : "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_EVA:			/* eval(arg1) */
	p = evala(bp[0]);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_EXE:			/* execute(arg1) */
	j = (int)strlen(s = bp[0]);	/* Length of macro invocation */
	p = "";				/* Initialize return value to null */
	if (j) {			/* If there is a macro to execute */
	    while (*s == SP) s++,j--;	/* strip leading spaces */
	    p = s;			/* remember beginning of macro name */
	    for (i = 0; i < j; i++) {	/* find end of macro name */
		if (*s == SP)
		  break;
		s++;
	    }
	    if (*s == SP) 	{	/* if there was a space after */
		*s++ = NUL;		/* terminate the macro name */
		while (*s == SP) s++;	/* skip past any extra spaces */
	    } else s = "";		/* maybe there are no arguments */
	    if (p && *p)
	      k = mlook(mactab,p,nmac);	/* Look up the macro name */
	    else k = -1;
/*
  This is just a WEE bit dangerous because we are copying up to 9 arguments
  into the space reserved for one.  It won't overrun the buffer or anything
  like that, but if there are lots of long arguments we might lose some.
  The other problem is that if the macro has more than 3 arguments, the 4th
  through last are all concatenated onto the third.  (The workaround is to
  use spaces rather than commas to separate them.)
  Leaving it like this for now to avoid having to allocate tons more buffers.
*/
	    if (argn > 1) {		/* Commas used instead of spaces */
		int i;
		char *p = bp[0];	/* Reuse this space */
		*p = NUL;		/* Make into dodo() arg list */
		for (i = 1; i < argn; i++) {
		    strncat(p,bp[i],1023);
		    strncat(p," ",1023);
		}		    
		s = bp[0];		/* Point to new list */
	    }
	    p = "";			/* Initialize return value */
	    if (k >= 0) {		/* If macro found in table */
		/* Go set it up (like DO cmd) */
		if ((j = dodo(k,s,cmdstk[cmdlvl].ccflgs)) > 0) {
		    if (cmpush() > -1) { /* Push command parser state */
			extern int ifc;
			int ifcsav = ifc; /* Push IF condition on stack */
			k = parser(1);	/* Call parser to execute the macro */
			cmpop();	/* Pop command parser */
			ifc = ifcsav;	/* Restore IF condition */
			if (k == 0) {	/* No errors, ignore action cmds. */
			    p = mrval[maclvl+1]; /* If OK, set return value. */
			    if (p == NULL) p = "";
			}
		    } else {		/* Can't push any more */
			debug(F100,"fexec pushed too deep","",0);
                        printf("\n?\\fexec() too deeply nested\n");
			while (cmpop() > -1) ;
			p = "";
		    }
		}
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_FC:			/* File count. */
	p = fnval;
	*p = NUL;
	if (argn > 0) {
	    k = zxpand(bp[0]);
	    sprintf(fnval,"%d",k);
	    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	}
	return(p);

      case FN_FIL:			/* Next file in list. */
	p = fnval;			/* (no args) */
	*p = NUL;
	znext(p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_IND:			/* index(arg1,arg2,arg3) */
      case FN_RIX:			/* rindex(arg1,arg2,arg3) */
	if (argn > 1) {			/* Only works if we have 2 or 3 args */
	    int start;
	    len1 = (int)strlen(bp[0]);	/* length of string to look for */
	    len2 = (int)strlen(s = bp[1]); /* length of string to look in */
	    if (len1 < 0) return("");	/* paranoia */
	    if (len2 < 0) return("");
	    j = len2 - len1;		/* length difference */
	    start = (y == FN_IND) ? 0 : j; /* Starting position */
	    if (argn > 2) {
		val1 = evala(bp[2]);
		if (chknum(val1)) {
		    int t;
		    t = atoi(val1) - 1;
		    if (t < 0) t = 0;
		    start = (y == FN_IND) ? t : start - t - 1;
		    if (start < 0) start = 0;
		}
	    }
	    start = ckindex(bp[0],bp[1],start,(y==FN_IND)?0:1,inpcas[cmdlvl]);
	    sprintf(fnval,"%d",start);
	    p = fnval;
	} else p = "0";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_RPL:			/* replace(s1,s2,s3) */
      /*
	s = bp[0] = source string
	    bp[1] = match string
	    bp[2] = replacement string
	p = fnval = destination (result) string
      */
	p = fnval;
	if (argn < 2) {			/* Only works if we have 2 or 3 args */
	    strcpy(p,bp[0]);
	} else  {			
	    len1 = (int)strlen(bp[0]);	/* length of string to look in */
	    len2 = (int)strlen(bp[1]);	/* length of string to look for */
	    len3 = (argn < 3) ? 0 : (int)strlen(bp[2]); /* Len of replacemnt */
	    j = len1 - len2 + 1;
	    if (j < 1 || len1 == 0 || len2 == 0) { /* Args out of whack */
		strcpy(p,bp[0]);	/* so just return original string */
	    } else {
		s = bp[0];		/* Point to beginning of string */
		while (j--) {		/* For each character */
		    if (inpcas[cmdlvl] ?
			!strncmp(bp[1],s,len2) :
			!xxstrcmp(bp[1],s,len2) ) { /* To be replaced? */
			if (len3) {		    /* Yes, */
			    strncpy(p,bp[2],len3);  /* replace it */
			    p += len3;
			}
			s += len2;	            /* and skip past it. */
		    } else {		/* No, */
			*p++ = *s++;	/* just copy this character */
		    }
		}
		*p = NUL;
		while (*p++ = *s++);
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p = fnval);

      case FN_CHR:			/* character(arg1) */
	val1 = evala(bp[0]);
	if (chknum(val1)) {		/* Must be numeric */
	    i = atoi(val1);
	    if (i >= 0 && i < 256) {	/* Must be an 8-bit value */
		p = fnval;
		*p++ = (char) i;
		*p = NUL;
		p = fnval;
	    } else p = "";		/* Otherwise return null */
	} else p = "";			/* Otherwise return null */
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_COD:			/* code(char) */
	p = "";
	if ((int)strlen(bp[0]) > 0) {
	    p = fnval;
	    i = *bp[0];
	    sprintf(p,"%d",(i & 0xff));
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_LEN:			/* length(arg1) */
	p = fnval;
	sprintf(p,"%d",(int)strlen(bp[0]));
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_LOW:			/* lower(arg1) */
	s = bp[0];
	p = fnval;

	while (*s) {
	    if (isupper(*s))
	      *p = (char) tolower(*s);
	    else
	      *p = *s;
	    p++; s++;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_MAX:			/* max(arg1,arg2) */
      case FN_MIN:			/* min(arg1,arg2) */
      case FN_MOD:			/* mod(arg1,arg2) */
	val1 = evala(bp[0]);
	if (bp[0]) {			/* Have to copy this */
	    free(bp[0]);		/* because evala() returns */
	    bp[0] = malloc((int)strlen(val1)+1); /* pointer to same */
	    strcpy(bp[0],val1);
	    val1 = bp[0];		/* buffer next time. */
	}
	val2 = evala(bp[1]);
	if (chknum(val1) && chknum(val2)) {
	    i = atoi(val1);
	    j = atoi(val2);
	    switch (y) {
	      case FN_MAX:
		if (j < i) j = i;
		break;
	      case FN_MIN:
		if (j > i) j = i;
		break;
	      case FN_MOD:
		j = i % j;
		break;
	    }
	    p = fnval;
	    sprintf(p,"%d",j);
	} else p = "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_SUB:			/* substr(arg1,arg2,arg3) */
      case FN_RIG:			/* right(arg1,arg2) */
	val1 = argn > 1 ? evala(bp[1]) : "";
	if (bp[1]) {			/* Have to copy this */
	    free(bp[1]);
	    bp[1] = malloc((int)strlen(val1)+1);
	    strcpy(bp[1],val1);
	    val1 = bp[1];
	}
	val2 = argn > 2 ? evala(bp[2]) : "";
	if (
	    ((argn > 1) && (int)strlen(val1) && !rdigits(val1)) ||
	    ((y == FN_SUB) &&
	      ((argn > 2) && (int)strlen(val2) && !rdigits(val2)))
	    ) {
	    p = "";			/* if either, return null */
	} else {
	    int lx;
	    p = fnval;			/* pointer to result */
	    lx = strlen(bp[0]);		/* length of arg1 */
	    if (y == FN_SUB) {		/* substring */
		k = (argn > 2) ? atoi(val2) : 1023; /* length */
		j = (argn > 1) ? atoi(val1) : 1; /* start pos for substr */
	    } else {				 /* right */
		k = (argn > 1) ? atoi(val1) : lx; /* length */
		j = lx - k + 1;			 /* start pos for right */
		if (j < 1) j = 1;
	    }
	    if (k > 0 && j <= lx) { 		 /* if start pos in range */
		s = bp[0]+j-1;    		 /* point to source string */
		for (i = 0; (i < k) && (*p++ = *s++); i++) ;  /* copy */
	    }
	    *p = NUL;			/* terminate the result */
	    p = fnval;			/* and point to it. */
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]); /* Free temp mem */
	return(p);

      case FN_UPP:			/* upper(arg1) */
	s = bp[0];
	p = fnval;
	while (*s) {
	    if (islower(*s))
	      *p = (char) toupper(*s);
	    else
	      *p = *s;
	    p++; s++;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_REP:			/* Repeat */
	p = "";				/* Return value */
	val1 = evala(bp[1]);
	if (chknum(val1)) {		/* Repeat count */
	    n = atoi(val1);
	    if (n > 0) {		/* Make n copies */
		p = fnval;
		*p = '\0';
		k = (int)strlen(bp[0]);	/* Make sure string has some length */
		if (k > 0) {
		    for (i = 0; i < n; i++) {
			s = bp[0];
			for (j = 0; j < k; j++) {
			    if ((p - fnval) >= FNVALL) { /* Protect against */
				p = "";	             /* core dumps... */
				break;
			    } else *p++ = *s++;
			}
		    }
		    *p = NUL;
		}
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

#ifndef NOFRILLS
      case FN_REV:
	p = fnval;
	yystring(bp[0],&p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);
#endif /* NOFRILLS */

      case FN_RPA:			/* RPAD and LPAD */
      case FN_LPA:
	*fnval = NUL;			/* Return value */
	val1 = evala(bp[1]);
	if (argn == 1) {		/* If a number wasn't given */
	    p = fnval;			/* just return the original string */
	    strncpy(p,bp[0],FNVALL);
	} else if (chknum(val1)) {	/* Repeat count */
	    char pc;
	    n = atoi(val1);
	    if (n >= 0) {		/* Pad it out */
		p = fnval;
		k = (int)strlen(bp[0]);	/* Length of string to be padded */
		pc = (char) ((argn < 3) ? SP : *bp[2]); /* Padding character */
		if (n > FNVALL) n = FNVALL-1; /* protect against overruns */
		if (k > FNVALL) k = FNVALL-1; /* and silly args. */
                if (k > n) k = n;
		if (y == FN_RPA) {	/* RPAD */
		    strncpy(p,bp[0],k);
		    p += k;
		    for (i = k; i < n; i++)
		      *p++ = pc;
		} else {		/* LPAD */
		    n -= k;
		    for (i = 0; i < n; i++)
		      *p++ = pc;
		    strncpy(p,bp[0],k);
		    p += k;
		}
		*p = NUL;
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

#ifdef ZFCDAT
      case FN_FD:			/* \fdate(filename) */
	p = fnval;
	*p = NUL;
	if (argn > 0) {
	    sprintf(fnval,"%s",zfcdat(bp[0]));
	    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	}
	return(p);
#endif /* ZFCDAT */

      case FN_FS:			/* \fsize(filename) */
	p = fnval;
	*p = NUL;
	if (argn > 0) {
	    sprintf(fnval,"%ld",zchki(bp[0]));
	    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	}
	return(p);

      case FN_VER:			/* VERIFY */
	if (argn > 1) {			/* Only works if we have 2 or 3 args */
	    int start;
	    char *s2;
	    start = 0;
	    if (argn > 2) {		/* Starting position specified */
		val1 = evala(bp[1]);
		if (chknum(val1)) {
		    start = atoi(val1) - 1;
		    if (start < 0) start = 0;
		}
	    }
	    i = start;
	    p = "0";
	    for (s = bp[1] + start; *s; s++,i++) {
		j = 0;
		for (s2 = bp[0]; *s2; s2++) {
		    if (*s2 == *s) {
			j = 1;
			break;
		    }
		}
		if (j == 0) {
		    sprintf(fnval,"%d",i+1);
		    p = fnval;
		    break;
		}
	    }
	    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	    return(p);
	}

      case FN_IPA:			/* Find and return IP address */
	if (argn > 0) {			/* in argument string. */
	    int start;
	    char *s2;
	    start = 0;
	    if (argn > 1) {		/* Starting position specified */
		if (chknum(bp[1])) {
		    start = atoi(bp[1]) - 1;
		    if (start < 0) start = 0;
		}
	    }
	    p = getip(bp[0]+start);
	    for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	    return(p);
	}

#ifdef OS2
      case FN_CRY:
	p = "";
	if (argn > 0) {
	    p = fnval;
	    strcpy(p,bp[0]);
            ck_encrypt(p);
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_OOX:
	p = "";
	if (argn > 0)
	  p = (char *) ck_oox(bp[0], (argn > 1) ? bp[1] : "");
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);
#endif /* OS2 */

      case FN_HEX:
	p = "";
	if ((int)strlen(bp[0]) < (FNVALL / 2)) {
	    s = bp[0];
	    p = fnval;
	    while (*s) {
		x = (*s >> 4) & 0x0f;
		*p++ = hexdigits[x];
		x = *s++ & 0x0f;
		*p++ = hexdigits[x];	    
	    }
	    *p = NUL;
	    p = fnval;
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_UNH: {
	  int c[2], i;
	  p = "";
	  if ((int)strlen(bp[0]) < (FNVALL * 2)) {
	      s = bp[0];
	      p = fnval;
	      while (*s) {
		  for (i = 0; i < 2; i++) {
		      c[i] = *s++;
		      if (!c[i]) { p = ""; goto unhexfin; }
		      if (islower(c[i])) c[i] = toupper(c[i]);
		      if (c[i] >= '0' && c[i] <= '9')
			c[i] -= 0x30;
		      else if (c[i] >= 'A' && c[i] <= 'F')
			c[i] -= 0x37;
		      else { p = ""; goto unhexfin; }
		  }
		  *p++ = ((c[0] << 4) & 0xf0) | (c[1] & 0x0f);
	      }
	      *p = NUL;
	      p = fnval;
	  }
  unhexfin:
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_BRK: {			/* Break */
	  char * c;			/* Characters to break on */
	  int done = 0;
	  s = bp[0];			/* Source pointer */
	  p = fnval;			/* Desination pointer */
	  while (*s && !done) {
	      c = bp[1];		/* Character to break on */
	      while (*c) {
		  if (*s == *c++) {
		      done = 1;
		      break;
		  }
	      }
	      if (done) break;
	      *p++ = *s++;
	  }
	  *p = NUL;			/* terminate the result */
	  p = fnval;			/* and point to it. */
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_SPN: {			/* Span */
	  char *q;
	  char c1, c2;
	  s = bp[0];			/* Source string */
	  p = fnval;			/* Result pointer */
	  if ((int)strlen(bp[1]) > 0) {	/* If span string is not empty */
	      while (*s) {		/* Loop thru source string */
		  q = bp[1];		/* Span string */
		  c1 = *s;
		  if (!inpcas[cmdlvl])
		    if (islower(c1)) c1 = toupper(c1);
		  x = 0;
		  while (c2 = *q++) {
		      if (!inpcas[cmdlvl])
			if (islower(c2)) c2 = toupper(c2);
		      if (c1 == c2) { x = 1; break; }
		  }
		  if (!x) break;
		  *p++ = *s++;
	      }
	  }
	  *p = NUL;			/* Terminate and return the result */
	  p = fnval;
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_TRM:			/* Trim(s1[,s2]) */
      case FN_LTR:			/* Left-Trim(s1[,s2]) */

	if (argn > 0 && (len1 = (int)strlen(bp[0])) > 0) {
	    s = " \t";
	    if (argn > 1)		/* Trim list given */
	      s = bp[1];
	    len2 = (int)strlen(s);

	    if (len2 < 1) {		/* or not... */
		s = " \t";		/* Default is to trim whitespace */
		len2 = 2;
	    }
	    if (y == FN_TRM) {		/* Trim from right */
		char * q;
		strncpy(fnval,bp[0],FNVALL); /* Copy string to output */
		p = fnval + len1 - 1;	/* Point to last character */
		while (p >= (char *)fnval) { /* Go backwards */
		    q = s;		/* Point to trim list */
		    while (*q) {	/* Is this char in trim list? */
			if (*q == *p) {	/* Yes, null it out */
			    *p = NUL;
			    break;
			}
			q++;
		    }
		    if (!*q)		/* Trim list exhausted */
		      break;		/* So we're done. */
		    p--;		/* Else keep trimming */
		}
	    } else {			/* Trim from left */
		char * q;
		p = bp[0];		/* Source */
		while (*p) {
		    q = s;
		    while (*q) {	/* Is this char in trim list? */
			if (*q == *p) {	/* Yes, point past it */
			    p++;	/* and try next source character */
			    break;
			}
			q++;		/* No, try next trim character */
		    }
		    if (!*q)		/* Trim list exhausted */
		      break;		/* So we're done. */
		}
		strncpy(fnval,p,FNVALL);
	    }
	    p = fnval;
	} else p = "0";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_CAP:			/* Capitalize */
	s = bp[0];
	p = fnval;
	x = 0;
	while (c = *s++) {
	    if (isalpha(c)) {
		if (x == 0) {
		    x = 1;
		    if (islower(c))
		      c = toupper(c);
		} else if (isupper(c))
		  c = tolower(c);
	    }
	    *p++ = c;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_TOD:
	p = fnval;
	sprintf(p,"%ld",tod2sec(bp[0]));
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_FFN:
	p = fnval;
	*p = NUL;
	if (bp[0])
#ifdef ZFNQFP
	  zfnqfp(bp[0],FNVALL,p);
#else
	  strcpy(p,bp[0]);
#endif /* ZFNQFP */
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_CHK: {
	  long chk = 0;
	  p = bp[0] ? bp[0] : "";
	  while (*p) chk += *p++;
	  sprintf(fnval,"%lu",chk);
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return((char *)fnval);
      }
      case FN_CRC:
	*fnval = NUL;
	if (bp[0])
	  sprintf(fnval,"%u",chk3((CHAR *)bp[0],0));
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return((char *)fnval);

      case FN_BSN:
	if (bp[0])
	  zstrip(bp[0],&p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      default:
	return("");
    }
}
#endif /* NOSPL */

static char ndatbuf[10];

char *
zzndate() {
    char * p;
    int x;

    ztime(&p);				/* Get "asctime" string */
    if (p == NULL || *p == NUL) return("");
    for (x = 20; x < 24; x++)		/* yyyy */
      ndatbuf[x - 20] = p[x];
    ndatbuf[6] = (char) ((p[8] == ' ') ? '0' : p[8]);
    ndatbuf[7] = p[9];			/* dd */
    for (x = 0; x < 12; x++)		/* mm */
      if (!strncmp(p+4,months[x],3)) break;
    if (x == 12) {
	ndatbuf[4] = ndatbuf[5] = '?';
    } else {
	x++;
	ndatbuf[4] = (char) ((x < 10) ? '0' : '1');
	ndatbuf[5] = (char) ((x % 10) + 48);
    }
    ndatbuf[8] = NUL;
    return((char *)ndatbuf);
}

#ifndef NOSPL
char *					/* Evaluate builtin variable */
nvlook(s) char *s; {
    int x, y;
    long z;
    char *p;
#ifndef NODIAL
    MDMINF * m;
#endif /* NODIAL */
#ifndef NOKVERBS			/* Keyboard macro material */
    extern int keymac, keymacx;
#endif /* NOKVERBS */
    x = 30;
    p = vvbuf;
    if (zzstring(s,&p,&x) < 0) {
	y = -1;
    } else {
	s = vvbuf;
	if ((y = lookup(vartab,s,nvars,&x)) < 0) return(NULL);
    }
#ifndef NODIAL
    m = (mdmtyp > 0) ? modemp[mdmtyp - 1] : NULL;
#endif /* NODIAL */
    switch (y) {
      case VN_ARGC:			/* ARGC */
	sprintf(vvbuf,"%d",macargc[maclvl]);
	return(vvbuf);

      case VN_ARGS:			/* ARGS */
	sprintf(vvbuf,"%d",xargs);
	return(vvbuf);

      case VN_COUN:			/* COUNT */
	sprintf(vvbuf,"%d",count[cmdlvl]);
	return(vvbuf);

      case VN_DATE:			/* DATE */
	ztime(&p);			/* Get "asctime" string */
	if (p == NULL || *p == NUL) return(NULL);
	vvbuf[0] = p[8];		/* dd */
	vvbuf[1] = p[9];
	vvbuf[2] = SP;
	vvbuf[3] = p[4];		/* mmm */
	vvbuf[4] = p[5];
	vvbuf[5] = p[6];
	vvbuf[6] = SP;
	for (x = 20; x < 24; x++)	/* yyyy */
	  vvbuf[x - 13] = p[x];
	vvbuf[11] = NUL;
	return(vvbuf);

      case VN_NDAT:			/* Numeric date */
	strcpy(vvbuf,zzndate());
        return(vvbuf);

      case VN_DIRE:			/* DIRECTORY */
	return(zgtdir());

      case VN_FILE:			/* filespec */
	return(fspec);

      case VN_HOST:			/* host name */
	if (*myhost) {			/* If known */
	    return(myhost);		/* return it. */
	} else {			/* Otherwise */
	    strcpy(vvbuf,"unknown");	/* just say "unknown" */
	    return(vvbuf);
	}

      case VN_SYST:			/* System type */
#ifdef UNIX
	strcpy(vvbuf,"UNIX");
#else
#ifdef VMS
	strcpy(vvbuf,"VMS");
#else
#ifdef OSK
	strcpy(vvbuf,"OS9/68K");
#else
#ifdef AMIGA
	strcpy(vvbuf,"Amiga");
#else
#ifdef MAC
	strcpy(vvbuf,"Macintosh");
#else
#ifdef OS2
#ifdef NT
	strcpy(vvbuf,"WIN32") ;
#else /* NT */
	strcpy(vvbuf,"OS/2");
#endif /* NT */
#else
#ifdef datageneral
	strcpy(vvbuf,"AOS/VS");
#else
#ifdef GEMDOS
	strcpy(vvbuf,"Atari_ST");
#else
#ifdef STRATUS
	strcpy(vvbuf,"Stratus_VOS");
#else
	strcpy(vvbuf,"unknown");
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* MAC */
#endif /* AMIGA */
#endif /* OSK */
#endif /* VMS */
#endif /* UNIX */
	return(vvbuf);

      case VN_SYSV:			/* System herald */
	for (x = y = 0; x < VVBUFL; x++) {
	    if (ckxsys[x] == SP && y == 0) continue;
	    vvbuf[y++] = (char) ((ckxsys[x] == SP) ? '_' : ckxsys[x]);
	}
	vvbuf[y] = NUL;
	return(vvbuf);

      case VN_TIME:			/* TIME. Assumes that ztime returns */
	ztime(&p);			/* "Thu Feb  8 12:00:00 1990" */
	if (p == NULL || *p == NUL)	/* like asctime()! */
	  return(NULL);
	for (x = 11; x < 19; x++)	/* copy hh:mm:ss */
	  vvbuf[x - 11] = p[x];		/* to vvbuf */
	vvbuf[8] = NUL;			/* terminate */
	return(vvbuf);			/* and return it */

      case VN_NTIM:			/* Numeric time */
	ztime(&p);			/* "Thu Feb  8 12:00:00 1990" */
	if (p == NULL || *p == NUL)	/* like asctime()! */
	  return(NULL);
	z = atol(p+11) * 3600L + atol(p+14) * 60L + atol(p+17);
	sprintf(vvbuf,"%ld",z);
	return(vvbuf);

#ifdef CK_TTYFD
      case VN_TTYF:			/* TTY file descriptor */
	sprintf(vvbuf,"%d",
#ifdef VMS
		vmsttyfd()
#else
		ttyfd
#endif /* VMS */
		);
	return(vvbuf);
#endif /* CK_TTYFD */

      case VN_VERS:			/* Numeric Kermit version number */
	sprintf(vvbuf,"%ld",vernum);
	return(vvbuf);

      case VN_XVNUM:			/* Product-specific version number */
	sprintf(vvbuf,"%ld",xvernum);
	return(vvbuf);

      case VN_HOME:			/* Home directory */
#ifdef UNIX
        sprintf(vvbuf,"%s/",zhome());
	return(vvbuf);
#else
#ifdef OSK
        sprintf(vvbuf,"%s/",zhome());
	return(vvbuf);
#else
#ifdef STRATUS
	sprintf(vvbuf,"%s>",zhome());
	return(vvbuf);
#else
	return(zhome());
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */

      case VN_IBUF:			/* INPUT buffer */
	return((char *)inpbuf);

      case VN_ICHR:			/* INPUT character */
	inchar[1] = NUL;
	return((char *)inchar);

      case VN_ICNT:			/* INPUT character count */
        sprintf(vvbuf,"%d",incount);
	return(vvbuf);

      case VN_SPEE: {			/* Transmission SPEED */
	  long t;
	  t = ttgspd();
	  if (t < 0L)
	    sprintf(vvbuf,"unknown");
	  else
	    sprintf(vvbuf,"%ld",t);
	  return(vvbuf);
      }
      case VN_SUCC:			/* SUCCESS flag */
	sprintf(vvbuf,"%d",(success == 0) ? 1 : 0);
	return(vvbuf);

      case VN_LINE:			/* LINE */
	p = (char *) ttname;
        return(p);

      case VN_PROG:			/* Program name */
	return("C-Kermit");

      case VN_RET:			/* Value of most recent RETURN */
	p = mrval[maclvl+1];
	if (p == NULL) p = "";
	return(p);

      case VN_FFC:			/* Size of most recent file */
	sprintf(vvbuf, "%ld", ffc);
	return(vvbuf);

      case VN_TFC:			/* Size of most recent file group */
	sprintf(vvbuf, "%ld", tfc);
	return(vvbuf);

    case VN_CPU:			/* CPU type */
#ifdef OS2
         {
            char * getcpu(void) ;
            return getcpu();
         }
#else /* OS2 */
#ifdef CKCPU
	return(CKCPU);
#else
	return("unknown");
#endif /* CKCPU */
#endif /* OS2 */

      case VN_CMDL:			/* Command level */
	sprintf(vvbuf, "%d", cmdlvl);
	return(vvbuf);

      case VN_DAY:			/* Day of week */
      case VN_NDAY:
/*
  Depends on ztime() returning ENGLISH asctime()-format string!
  asctime() format is: "Thu Feb  8 12:00:00 1990".
  Needs updating to accommodate non-English asctime() strings.
*/
	ztime(&p);
	if (p != NULL && *p != NUL) {	/* ztime() succeeded. */
	    if (y == VN_DAY) {		/* String day. */
		strncpy(vvbuf,p,3);
	    } else {			/* Numeric day. */
		for (x = 0; x < 7; x++)	  /* Look up day string in table */
		  if (!strncmp(p,wkdays[x],3))
		    break;
		if (x > 6) x = -1;	/* Not found */
		sprintf(vvbuf,"%d",x);	/* Return the number */
	    }
	} else vvbuf[0] = NUL;		/* ztime() failed. */
	return(vvbuf);			/* Return what we got. */

      case VN_LCL:			/* Local (vs remote) mode */
	strcpy(vvbuf, local ? "1" : "0");
	return(vvbuf);

      case VN_CMDS:			/* Command source */
	if (cmdstk[cmdlvl].src == CMD_KB)
	  strcpy(vvbuf,"prompt");
	else if (cmdstk[cmdlvl].src == CMD_MD)
	  strcpy(vvbuf,"macro");
	else if (cmdstk[cmdlvl].src == CMD_TF)
	  strcpy(vvbuf,"file");
	else strcpy(vvbuf,"unknown");
	return(vvbuf);

      case VN_CMDF:			/* Current command file name */
#ifdef COMMENT				/* (see comments above) */
	if (tfnam[tlevel]) {		/* (near dblbs declaration) */
	    dblbs(tfnam[tlevel],vvbuf,VVBUFL);
	    return(vvbuf);
	} else return("");
#else
	if (tlevel < 0)
	  return("");
	else
	  return(tfnam[tlevel] ? tfnam[tlevel] : "");
#endif /* COMMENT */

      case VN_MAC:			/* Current macro name */
	return((maclvl > -1) ? m_arg[maclvl][0] : "");

      case VN_EXIT:
	sprintf(vvbuf,"%d",xitsta);
	return(vvbuf);

      case VN_PRTY: {			/* Parity */
	  char *s;
	  switch (parity) {
	    case 0:   s = "none";  break;
	    case 'e': s = "even";  break;
	    case 'm': s = "mark";  break;
	    case 'o': s = "odd";   break;
	    case 's': s = "space"; break;
	    default:  s = "unknown"; break;
	  }
	  strcpy(vvbuf,s);
	  return(vvbuf);
      }

      case VN_DIAL:
	sprintf(vvbuf,"%d",
#ifndef NODIAL
		dialsta
#else
		-1
#endif /* NODIAL */
		);
	return(vvbuf);

#ifdef OS2
      case VN_KEYB:
	strncpy(vvbuf,conkbg(),VVBUFL);
	return(vvbuf);

      case VN_SELCT: {
          extern char * selection ;
          return( selection ? selection : "" ) ;
      }
#endif /* OS2 */

      case VN_CPS:
	if (tsecs > 0)
	  sprintf(vvbuf, "%ld", tfc / (long) tsecs);
	else strcpy(vvbuf,"0");
	return(vvbuf);

      case VN_MODE:			/* File transfer mode */
	switch (binary) {
	  case XYFT_T: strcpy(vvbuf,"text"); break;
	  case XYFT_B:
	  case XYFT_U: strcpy(vvbuf,"binary"); break;
	  case XYFT_I: strcpy(vvbuf,"image"); break;
	  case XYFT_L: strcpy(vvbuf,"labeled"); break;
	  case XYFT_M: strcpy(vvbuf,"macbinary"); break;
	  default:     strcpy(vvbuf,"unknown");
	}
	return(vvbuf);

#ifdef CK_REXX
      case VN_REXX:
	return(rexxbuf);
#endif /* CK_REXX */

      case VN_NEWL:			/* System newline char or sequence */
#ifdef UNIX
	strcpy(vvbuf,"\n");
#else
#ifdef datageneral
	strcpy(vvbuf,"\n");
#else
#ifdef OSK
	strcpy(vvbuf,"\15");		/* Remember, these are octal... */
#else
#ifdef MAC
	strcpy(vvbuf,"\15");
#else
#ifdef OS2
	strcpy(vvbuf,"\15\12");
#else
#ifdef STRATUS
	strcpy(vvbuf,"\n");
#else
#ifdef VMS
	strcpy(vvbuf,"\15\12");
#else
#ifdef AMIGA
	strcpy(vvbuf,"\n");
#else
#ifdef GEMDOS
	strcpy(vvbuf,"\n");
#else
	strcpy(vvbuf,"\n");
#endif /* GEMDOS */
#endif /* AMIGA */
#endif /* VMS */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* MAC */
#endif /* OSK */
#endif /* datageneral */
#endif /* UNIX */
	return(vvbuf);

      case VN_ROWS:			/* ROWS */
      case VN_COLS:			/* COLS */
        strcpy(vvbuf,(y == VN_ROWS) ? "24" : "80"); /* Default */
#ifdef CK_TTGWSIZ
#ifdef OS2 
        if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
	  ttgwsiz();
        sprintf(vvbuf,"%d",(y == VN_ROWS) ? tt_rows[VTERM] : tt_cols[VTERM]);
#else /* OS2 */
        if (ttgwsiz() > 0)		/* Get window size */
	  if (tt_cols > 0 && tt_rows > 0) /* sets tt_rows, tt_cols */
	    sprintf(vvbuf,"%d",(y == VN_ROWS) ? tt_rows : tt_cols);
#endif /* OS2 */
#endif /* CK_TTGWSIZ */
	return(vvbuf);

      case VN_TTYP:
#ifdef OS2
	sprintf(vvbuf, "%s",
		(tt_type >= 0 && tt_type <= max_tt) ?
		tt_info[tt_type].x_name :
		"unknown"
		);
#else
#ifdef MAC
	strcpy(vvbuf,"vt320");
#else
	p = getenv("TERM");
	sprintf(vvbuf,"%s", p ? p : "unknown");
#endif /* MAC */
#endif /* OS2 */
	return(vvbuf);

      case VN_MINP:			/* MINPUT */
	sprintf(vvbuf, "%d", m_found);
	return(vvbuf);

      case VN_CONN:			/* CONNECTION */
	if (!local) {
	  strcpy(vvbuf,"remote");
	} else {
	    if (!network)
	      strcpy(vvbuf,"serial");
#ifdef TCPSOCKET
	    else if (nettype == NET_TCPB || nettype == NET_TCPA) {
		if (ttnproto == NP_TELNET)
		  strcpy(vvbuf,"tcp/ip_telnet");
		else
		  strcpy(vvbuf,"tcp/ip");
	    }
#endif /* TCPSOCKET */
#ifdef ANYX25
	    else if (nettype == NET_SX25 || nettype == NET_VX25)
	      strcpy(vvbuf,"x.25");
#endif /* ANYX25 */
#ifdef DECNET
	    else if (nettype == NET_DEC) {
		if ( ttnproto == NP_LAT ) strcpy(vvbuf,"decnet_lat");
		else if ( ttnproto == NP_CTERM ) sprintf(vvbuf,"decnet_cterm");
		else strcpy(vvbuf,"decnet");
	    }
#endif /* DECNET */
#ifdef SUPERLAT
        else if ( nettype == NET_SLAT ) 
           strcpy(vvbuf,"superlat");
#endif /* SUPERLAT */
#ifdef NETFILE
        else if ( nettype == NET_FILE )
           strcpy(vvbuf,"local file");
#endif /* NETFILE */

#ifdef NPIPE
	    else if (nettype == NET_PIPE)
	      strcpy(vvbuf,"named_pipe");
#endif /* NPIPE */
#ifdef CK_NETBIOS
	    else if (nettype == NET_BIOS)
	      strcpy(vvbuf,"netbios");
#endif /* CK_NETBIOS */
	    else
	      strcpy(vvbuf,"unknown");
	}
	return(vvbuf);

      case VN_SYSI:			/* System ID, Kermit code */
	return((char *)cksysid);

#ifdef OS2
      case VN_SPA:
	sprintf(vvbuf,"%ld",zdskspace(0));
	return(vvbuf);
#endif /* OS2 */

      case VN_QUE: {
	  extern char querybuf[];
	  return(querybuf);
      }
#ifndef NOCSETS
      case VN_CSET:
#ifdef OS2
	sprintf(vvbuf,"cp%d",os2getcp());
#else
	sprintf(vvbuf,"%s",fcsinfo[fcharset].keyword);
#endif /* OS2 */
	return(vvbuf);
#endif /* NOCSETS */

#ifdef OS2
      case VN_STAR:
	return(startupdir);

      case VN_EXEDIR:
	return(exedir);    
#endif /* OS2 */

      case VN_INI:
	return(inidir) ;

      case VN_MDM:
	return(gmdmtyp());

      case VN_EVAL:
	return(evalbuf);

#ifndef NODIAL
      case VN_D_CC:			/* DIAL COUNTRY-CODE */
	return(diallcc ? diallcc : "");
	
      case VN_D_AC:			/* DIAL AREA-CODE */
	return(diallac ? diallac : "");

      case VN_D_IP:			/* DIAL INTERNATIONAL-PREFIX */
	return(dialixp ? dialixp : "");

      case VN_D_LP:			/* DIAL LD-PREFIX */
	return(dialldp ? dialldp : "");
#else
      case VN_D_CC:			/* DIAL COUNTRY-CODE */
      case VN_D_AC:			/* DIAL AREA-CODE */
      case VN_D_IP:			/* DIAL INTERNATIONAL-PREFIX */
      case VN_D_LP:			/* DIAL LD-PREFIX */
	return("");
#endif /* NODIAL */
      case VN_UID:
	return((char *)uidbuf);

      case VN_PWD:
#ifdef OS2
	if (activecmd == XXOUT) {
	    strncpy(vvbuf,pwbuf,VVBUFL);
	    ck_encrypt((char *)vvbuf);
	    return((char *)vvbuf);
	} else
#endif /* OS2 */
	  return((char *)pwbuf);

      case VN_PRM:
	return((char *)prmbuf);

      case VN_PROTO:
#ifdef CK_XYZ
	return(ptab[protocol].p_name);
#else
	return("kermit");
#endif /* CK_XYZ */

#ifdef CK_TMPDIR
      case VN_DLDIR:
	return(dldir ? dldir : "");
#endif /* CK_TMPDIR */

#ifndef NODIAL
      case VN_M_INI:			/* Modem init string */
	return(dialini ? dialini : (m ? m->wake_str : ""));

      case VN_M_DCM:			/* Modem dial command */
	return(dialcmd ? dialcmd : (m ? m->dial_str : ""));

      case VN_M_DCO:			/* Modem data compression on */
	return(dialdcon ? dialdcon : (m ? m->dc_on_str : ""));

      case VN_M_DCX:			/* Modem data compression off */
	return(dialdcoff ? dialdcoff : (m ? m->dc_off_str : ""));

      case VN_M_ECO:			/* Modem error correction on */
	return(dialecon ? dialecon : (m ? m->ec_on_str : ""));

      case VN_M_ECX:			/* Modem error correction off */
	return(dialecoff ? dialecoff : (m ? m->ec_off_str : ""));

      case VN_M_AAO:			/* Modem autoanswer on */
	return(dialaaon ? dialaaon : (m ? m->aa_on_str : ""));

      case VN_M_AAX:			/* Modem autoanswer off */
	return(dialaaoff ? dialaaoff : (m ? m->aa_off_str : ""));

      case VN_M_HUP:			/* Modem hangup command */
	return(dialhcmd ? dialhcmd : (m ? m->hup_str : ""));

      case VN_M_HWF:			/* Modem hardware flow command */
	return(dialhwfc ? dialhwfc : (m ? m->hwfc_str : ""));

      case VN_M_SWF:			/* Modem software flow command */
	return(dialswfc ? dialswfc : (m ? m->swfc_str : ""));

      case VN_M_NFC:			/* Modem no flow-control command */
	return(dialnofc ? dialnofc : (m ? m->nofc_str : ""));

      case VN_M_PDM:			/* Modem pulse dialing mode */
	return(dialpulse ? dialpulse : (m ? m->pulse : ""));

      case VN_M_TDM:			/* Modem tone dialing mode */
	return(dialtone ? dialtone : (m ? m->tone : ""));
#else
      case VN_M_INI:			/* Modem init string */
      case VN_M_DCM:			/* Modem dial command */
      case VN_M_DCO:			/* Modem data compression on */
      case VN_M_DCX:			/* Modem data compression off */
      case VN_M_ECO:			/* Modem error correction on */
      case VN_M_ECX:			/* Modem error correction off */
      case VN_M_AAO:			/* Modem autoanswer on */
      case VN_M_AAX:			/* Modem autoanswer off */
      case VN_M_HUP:			/* Modem hangup command */
      case VN_M_HWF:			/* Modem hardware flow command */
      case VN_M_SWF:			/* Modem software flow command */
      case VN_M_NFC:			/* Modem no flow-control command */
      case VN_M_PDM:			/* Modem pulse dialing mode */
      case VN_M_TDM:			/* Modem tone dialing mode */
	return("");
#endif /* NODIAL */

      case VN_ISTAT:			/* INPUT status */
	sprintf(vvbuf, "%d", instatus);
	return(vvbuf);

      case VN_TEMP:			/* Temporary directory */
	if (tempdir) {
	    p = tempdir;
	} else {
#ifdef OS2
#ifdef NT
	    p = getenv("K95TMP");
#else
	    p = getenv("K2TMP");
#endif /* NT */
	    if ( !p )
		p = getenv("CK_TMP");
	    if (!p)
#endif /* OS2 */
	    p = getenv("TEMP");
	    if (!p) p = getenv("TMP");
	    if (!p)
#ifdef UNIX				/* Systems that have a standard */
	      p = "/tmp/";		/* temporary directory... */
#else
#ifdef datageneral
	      p = ":TMP:";
#else
	      p = "";
#endif /* datageneral */
#endif /* UNIX */
	}
	strncpy(vvbuf,p,VVBUFL - 1);
	p = vvbuf;

/* This needs generalizing for VOS, AOS/VS, etc... */

	while (*p) {
#ifdef OS2
	    if (*p == '\\') *p = '/';
#endif /* OS2 */
	    p++;
	}
	if (p > vvbuf) {
	    char c =			/* Directory termination character */
#ifdef MAC
	      ':'
#else
#ifdef datageneral
	      ':'
#else
#ifdef STRATUS
	      '>'
#else
	      '/'
#endif /* STRATUS */
#endif /* datageneral */
#endif /* MAC */
		;

	    if (*(p-1) != c) {
		*p++ = c;
		*p = NUL;
	    }
	}
	return(vvbuf);

      case VN_ERRNO:			/* Error number */
	sprintf(vvbuf, "%d", errno);
	return(vvbuf);

      case VN_ERSTR:			/* Error string */
	strncpy(vvbuf,ck_errstr(),VVBUFL - 1);
	return(vvbuf);

      case VN_RPSIZ:			/* RECEIVE packet-length */
	sprintf(vvbuf,"%d",urpsiz);
	return(vvbuf);

      case VN_WINDO:
	sprintf(vvbuf,"%d",wslotr);
	return(vvbuf);

      case VN_TFLN:			/* TAKE-file line number */
	if (tlevel > -1) {
	    sprintf(vvbuf, "%d", tfline[tlevel]);
	    return(vvbuf);
	} else
	  return("0");

      case VN_MDMSG:			/* DIALRESULT */
#ifndef NODIAL
	return((char *)modemmsg);
#else
	return("");
#endif /* NODIAL */

      case VN_DNUM:			/* DIALNUMBER */
#ifndef NODIAL
	return(dialnum ? (char *) dialnum : "");
#else
	return("");
#endif /* NODIAL */

      case VN_APC:
	sprintf(vvbuf, "%d",
#ifdef CK_APC
		apcactive
#else
		0
#endif /* CK_APC */
		);
	return((char *)vvbuf);

      case VN_TRMK:
	  sprintf(vvbuf, "%d",
#ifdef OS2
		keymac
#else
		0
#endif /* OS2 */
		);
	return((char *)vvbuf);

      case VN_IPADDR:
	sprintf(vvbuf, "%s",
#ifdef TCPSOCKET
		(char *) myipaddr
#else
		""
#endif /* TCPSOCKET */
		);
	return((char *)vvbuf);

      case VN_CRC16:			/* CRC-16 of most recent transfer */
	sprintf(vvbuf,"%ld",crc16);
	return(vvbuf);

      default:
	return(NULL);
    }
}
#endif /* NOSPL */


/*
  X X S T R I N G  --  Expand variables and backslash codes.

    int xxtstring(s,&s2,&n);

  Expands \ escapes via recursive descent.
  Argument s is a pointer to string to expand (source).
  Argument s2 is the address of where to put result (destination).
  Argument n is the length of the destination string (to prevent overruns).
  Returns -1 on failure, 0 on success,
    with destination string null-terminated and s2 pointing to the
    terminating null, so that subsequent characters can be added.
*/

#define XXDEPLIM 100			/* Recursion depth limit */

int
zzstring(s,s2,n) char *s; char **s2; int *n; {
    int x,				/* Current character */
        y,				/* Worker */
        pp,				/* Paren level */
	kp,				/* Brace level */
        argn,				/* Function argument counter */
        n2,				/* Local copy of n */
        d,				/* Array dimension */
        vbi,				/* Variable id (integer form) */
        argl;				/* String argument length */

    char vb,				/* Variable id (char form) */
        *vp,				/* Pointer to variable definition */
        *new,				/* Local pointer to target string */
        *p,				/* Worker */
        *q;				/* Worker */
    char *r  = (char *)0;		/* For holding function args */
    char *r2 = (char *)0;
    char *r3p;

#ifndef NOSPL
    char vnambuf[VNAML];		/* Buffer for variable/function name */
    char *argp[FNARGS];			/* Pointers to function args */
#endif /* NOSPL */

    static int depth = 0;		/* Call depth, avoid overflow */

    n2 = *n;				/* Make local copies of args */
    new = *s2;				/* for one less level of indirection */

    depth++;				/* Sink to a new depth */
    if (depth > XXDEPLIM) {		/* Too deep? */
	printf("?definition is circular or too deep\n");
	depth = 0;
	*new = NUL;
	return(-1);
    }
    if (!s || !new) {			/* Watch out for null pointers */
	depth = 0;
	*new = NUL;
	return(-1);
    }
    argl = (int)strlen(s);		/* Get length of source string */
    debug(F111,"xxstring",s,argl);
    if (argl < 0) {			/* Watch out for garbage */
	depth = 0;
	*new = NUL;
	return(-1);
    }
    while ( x = *s ) {			/* Loop for all characters */
        if (x != CMDQ) {		/* Is it the command-quote char? */
	    *new++ = *s++;		/* No, normal char, just copy */
	    if (n2-- < 0) {		/* and count it, careful of overflow */
		return(-1);
	    }
	    continue;
	}

/* We have the command-quote character. */

	x = *(s+1);			/* Get the following character. */

	switch (x) {			/* Act according to variable type */
#ifndef NOSPL
	  case 0:			/* It's a lone backslash */
	    *new++ = *s++;
	    if (n2-- < 0)
	      return(-1);
	    break;
	  case '%':			/* Variable */
	    s += 2;			/* Get the letter or digit */
	    vb = *s++;			/* and move source pointer past it */
	    vp = NULL;			/* Assume definition is empty */
	    if (vb >= '0' && vb <= '9') { /* Digit for macro arg */
		if (maclvl < 0) 	/* Digit variables are global */
		  vp = g_var[vb];	/* if no macro is active */
		else			/* otherwise */
		  vp = m_arg[maclvl][vb - '0']; /* they're on the stack */
	    } else {
		if (isupper(vb)) vb += ('a'-'A');
		vp = g_var[vb];		/* Letter for global variable */
	    }
	    if (vp) {			/* If definition not empty */
		if (zzstring(vp,&new,&n2) < 0) { /* call self to evaluate it */
		    return(-1);		/* Pass along failure */
		}
	    }
	    break;
	  case '&':			/* An array reference */
	    if (arraynam(s,&vbi,&d) < 0) { /* Get name and subscript */
		return(-1);
	    }
	    pp = 0;			/* Bracket counter */
	    while (*s) {		/* Advance source pointer */
		if (*s == '[') pp++;
		if (*s == ']' && --pp == 0) break;
		s++;
	    }
	    if (*s == ']') s++;		/* past the closing bracket. */
	    if (chkarray(vbi,d) > 0) {	/* Array is declared? */
		vbi -= ARRAYBASE;	/* Convert name to index */
		if (a_dim[vbi] >= d) {	/* If subscript in range */
		    char **ap;
		    ap = a_ptr[vbi];	/* get data pointer */
		    if (ap) {		/* and if there is one */
			if (ap[d]) {	/* If definition not empty */
			    if (zzstring(ap[d],&new,&n2) < 0) { /* evaluate */
				return(-1); /* Pass along failure */
			    }
			}
		    }
		}
	    }
	    break;

	  case 'F':			/* A builtin function */
	  case 'f':
	    q = vnambuf;		/* Copy the name */
	    y = 0;			/* into a separate buffer */
	    s += 2;			/* point past 'F' */
	    while (y++ < VNAML) {
		if (*s == '(') { s++; break; } /* Look for open paren */
		if ((*q = *s) == NUL) break;   /* or end of string */
		s++; q++;
	    }
	    *q = NUL;			/* Terminate function name */
	    if (y >= VNAML) {		/* Handle pathological case */
		while (*s && (*s != '(')) /* of very long string entered */
		  s++;			  /* as function name. */
		if (*s == ')') s++;	  /* Skip past it. */
	    }
	    r = r2 = malloc(argl+2);	/* And make a place to copy args */
	    debug(F101,"xxstring r2","",r2);
	    if (!r2) {			/* Watch out for malloc failure */
		depth = 0;
		*new = NUL;
		return(-1);
	    }
	    if (r3) free(r3); /* And another to copy literal arg string */
	    r3 = malloc(argl+2);
	    debug(F101,"xxstring r3","",r3);
	    if (!r3) {
		depth = 0;
		*new = NUL;
		return(-1);
	    } else
	      r3p = r3;
	    argn = 0;			/* Argument counter */
	    argp[argn++] = r;		/* Point to first argument */
	    y = 0;			/* Completion flag */
	    pp = 1;			/* Paren level (already have one). */
	    kp = 0;
	    while (1) {			/* Copy each argument, char by char. */
		*r3p++ = *s;		/* This is a literal copy for \flit */
		if (!*s) break;

		if (*s == '{') {	/* Left brace */
#ifdef COMMENT
		    if (kp++ == 0) {	/* Skip it if it's the outer one */
			s++;
			*r3p++ = *s;
		    }
#else
		    kp++;
#endif /* COMMENT */
		}
		if (*s == '}') {	/* Right brace */
#ifdef COMMENT
		    if (--kp == 0) {	/* Skip it if it's the outer one */
			s++;
			*r3p++ = *s;
		    }
#else
		    kp--;
#endif /* COMMENT */
		}
		if (*s == '(' && kp <= 0) { /* Open paren not in brace */
		    pp++;		/* Count it */
		}
		*r = *s;		/* Now copy resulting byte */
		if (!*r)		/* If NUL, done. */
		  break;
		if (*r == ')' && kp <= 0) { /* Closing paren, count it. */
		    if (--pp == 0) {	/* Final one? */
			*r = NUL;	/* Make it a terminating null */
			*(r3p - 1) = NUL;
			s++;		/* Point past it in source string */
			y = 1;		/* Flag we've got all the args */
			break;		/* Done with while loop */
		    }
		}
		if (*r == ',' && kp <= 0) { /* Comma */
		    if (pp == 1) {	    /* If not within ()'s, */
			if (argn >= FNARGS) { /* Too many args */
			    s++; r++;	/* Keep collecting flit() string */
			    continue;
			}
			*r = NUL;	    /* New arg, skip past comma */
			argp[argn++] = r+1; /* In range, point to new arg */
		    }			/* Otherwise just skip past  */
		}
		s++; r++;		/* Advance pointers */
	    }
	    debug(F111,"xxstring function name",vnambuf,y);
	    debug(F110,"xxstring function r3",r3,0);
	    if (!y) {			/* If we didn't find closing paren */
		debug(F101,"xxstring r2 before free","",r2);
		if (r2) {
		    free(r2);		/* free the temporary storage */
		    r2 = NULL;
		}
		return(-1);		/* and return failure. */
	    }
#ifdef DEBUG
	    if (deblog)
	      for (y = 0; y < argn; y++)
		debug(F111,"xxstring function arg",argp[y],y);
#endif /* DEBUG */
	    vp = fneval(vnambuf,argp,argn,r3); /* Evaluate the function. */
	    if (vp) {			/* If definition not empty */
		while (*new++ = *vp++)	/* copy it to output string */
		  if (n2-- < 0) return(-1); /* mindful of overflow */
		new--;			/* Back up over terminating null */
		n2++;			/* to allow for further deposits. */
	    }
	    if (r2) {
		debug(F101,"xxstring freeing r2","",r2);
		free(r2);		/* Now free the temporary storage */
		r2 = NULL;
	    }
	    break;
	  case '$':			/* An environment variable */
	  case 'V':			/* Or a named builtin variable. */
	  case 'v':
	  case 'M':			/* Or a macro = long variable */
	  case 'm':
	    p = s+2;			/* $/V/M must be followed by (name) */
	    if (*p != '(') {		/* as in \$(HOME) or \V(count) */
		*new++ = *s++;		/* If not, just copy it */
		if (n2-- < 0) {
		    return(-1);
		}
		break;
	    }
	    p++;			/* Point to 1st char of name */
	    q = vnambuf;		/* Copy the name */
	    y = 0;			/* into a separate buffer */
	    while (y++ < VNAML) {	/* Watch out for name too long */
		if (*p == ')') {	/* Name properly terminated with ')' */
		    p++;		/* Move source pointer past ')' */
		    break;
		}
		if ((*q = *p) == NUL)	/* String ends before ')' */
		  break;
 		p++; q++;		/* Advance pointers */
	    }
	    *q = NUL;			/* Terminate the variable name */
	    if (y >= VNAML) {		/* Handle pathological case */
		while (*p && (*p != ')')) /* of very long string entered */
		  p++;			  /* as variable name. */
		if (*p == ')') p++;	  /* Skip ahead to the end of it. */
	    }
	    s = p;			/* Adjust global source pointer */
	    p = malloc((int)strlen(vnambuf) + 1); /* Make temporary space */
	    if (p) {			/* If we got the space */
		vp = vnambuf;		/* Point to original */
		strcpy(p,vp);		/* Make a copy of it */
		y = VNAML;		/* Length of name buffer */
		zzstring(p,&vp,&y);	/* Evaluate the copy */
		free(p);		/* Free the temporary space */
		p = NULL;
	    }
	    debug(F110,"xxstring vname",vnambuf,0);
	    q = NULL;
	    if (x == '$') {		/* Look up its value */
		vp = getenv(vnambuf);	/* This way for environment variable */
	    } else if (x == 'm' || x == 'M') { /* or this way for macro */
		y = mlook(mactab,vnambuf,nmac);	/* get definition */
		if (y > -1) {		/* Got definition */
		    vp = mactab[y].mval;
#ifdef COMMENT
/*
  This works, but it breaks too many things, like CKERMIT.INI!
  We need a new \_() function for this, which fully evaluates the name
  of the macro before retrieving its definition.
*/
		    q = p = malloc(1024); /* Now evaluate it */
		    if (p) {
			y = 1023;
			zzstring(vp,&p,&y);
		    }
		    vp = q;		/* Point to evaluated definition */
#endif /* COMMENT */
		} else vp = NULL;
	    } else { 			/*  or */
	        vp = nvlook(vnambuf);	/* this way for builtin variable */
	    }
	    if (vp) {			/* If definition not empty */
		while (*new++ = *vp++)	/* copy it to output string. */
		  if (n2-- < 0) {
		      if (q) free(q);
		      return(-1);
		  }
		new--;			/* Back up over terminating null */
		n2++;			/* to allow for further deposits. */
	    }
	    if (q) {
		free(q);
		q = NULL;
	    }
	    break;
#endif /* NOSPL	*/			/* Handle \nnn even if NOSPL. */

#ifndef NOKVERBS
	case 'K':
	case 'k': {
	    extern struct keytab kverbs[];
	    extern int nkverbs;
#define K_BUFLEN 30
	    char kbuf[K_BUFLEN + 1];	/* Key verb name buffer */
	    int x, y, z, brace = 0;
	    s += 2;
/*
  We assume that the verb name is {braced}, or it extends to the end of the
  string, s, or it ends with a space, control character, or backslash.
*/
	    p = kbuf;			/* Copy verb name into local buffer */
	    x = 0;
	    if (*s == '{')  {
		s++;
		brace++;
	    }
	    while ((x++ < K_BUFLEN) && (*s > SP) && (*s != CMDQ)) {
		if (brace && *s == '}') {
		    s++;
		    break;
		}	
		*p++ = *s++;
	    }
	    brace = 0;
	    *p = NUL;			/* Terminate. */
	    p = kbuf;			/* Point back to beginning */
	    debug(F110,"zzstring kverb",p,0);
	    y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
	    debug(F101,"zzstring lookup",0,y);
	    if (y > -1) {
		dokverb(VTERM,y);
#ifndef NOSPL
	    } else {			/* Is it a macro? */
		y = mxlook(mactab,p,nmac);
		if (y > -1) {
		    debug(F111,"zzstring mxlook",s,y);
		    if ((z = dodo(y,NULL,cmdstk[cmdlvl].ccflgs)) > 0) {
			if (cmpush() > -1) {  /* Push command parser state */
			    extern int ifc;
			    int ifcsav = ifc; /* Push IF condition on stack */
			    y = parser(1);    /* New parser to execute macro */
			    cmpop();	      /* Pop command parser */
			    ifc = ifcsav;     /* Restore IF condition */
			    if (y == 0) {     /* No errors, ignore actions */
				p = mrval[maclvl+1]; /* If OK set return val */
				if (p == NULL) p = "";
			    }
			} else {		/* Can't push any more */
			    debug(F100,"zzstring pushed too deep","",0);
			    printf("\n?zzstring too deeply nested\n");
			    while (cmpop() > -1);
			    p = "";
			}
		    }
		}
#endif /* NOSPL */
	    }
	    break;
	}
#endif /* NOKVERBS */

	default:			/* Maybe it's a backslash code */
	  y = xxesc(&s);		/* Go interpret it */
	  if (y < 0) {			/* Upon failure */
	      *new++ = (char) x;	/* Just quote the next character */
	      s += 2;			/* Move past the pair */
	      n2 -= 2;
	      if (n2 < 0) {
		  return(-1);
	      }
	      continue;			/* and go back for more */
	  } else {
	      *new++ = (char) y;	/* else deposit interpreted value */
	      if (n2-- < 0) {
		  return(-1);
	      }
	  }
	}
    }
    *new = NUL;				/* Terminate the new string */
    depth--;				/* Adjust stack depth gauge */
    *s2 = new;				/* Copy results back into */
    *n = n2;				/* the argument addresses */
    return(0);				/* and return. */
}
#endif /* NOICP */
