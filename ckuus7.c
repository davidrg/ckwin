#include "ckcsym.h"
#ifndef NOICP

/*  C K U U S 7 --  "User Interface" for C-Kermit, part 7  */
 
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
  This file created from parts of ckuus3.c, which became to big for
  Mark Williams Coherent compiler to handle.
*/

/*
  Definitions here supersede those from system include files.
*/
#include "ckcdeb.h"			/* Debugging & compiler things */
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckcker.h"			/* Kermit application definitions */
#include "ckcxla.h"			/* Character set translation */
#include "ckcnet.h"			/* Network symbols */
#include "ckuusr.h"			/* User interface symbols */
#include "ckucmd.h"
#ifdef CKOUNI
#include "ckouni.h"
#endif /* CKOUNI */

#ifdef OS2
#ifndef NT
#define INCL_NOPM 
#define INCL_VIO            /* needed for ckocon.h */
#define INCL_DOSMODULEMGR
#include <os2.h>
#undef COMMENT
#else /* NT */
#define APIRET ULONG
#include "cknwin.h"
#include "ckntap.h"
#endif /* NT */
#include "ckowin.h"
#include "ckocon.h"
#include "ckodir.h"
#ifdef OS2MOUSE
#include "ckokey.h"
#endif /* OS2MOUSE */
#ifdef KUI
#include "ikui.h"
#endif /* KUI */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
extern int mskkeys;
#endif /* OS2 */    

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

static int x, y = 0, z;
static char *s;

#ifdef CK_SPEED
extern short ctlp[];			/* Control-char prefixing table */
#endif /* CK_SPEED */

#ifdef NETCONN
extern struct keytab netcmd[];
#ifndef NODIAL
extern int dirline;
extern int nnets, nnetdir;		/* Network services directory */
extern char *netdir[];
_PROTOTYP( VOID ndinit, (void) );
_PROTOTYP( VOID ndreset, (void) );
char *nh_p[MAXDNUMS + 1];		/* Network directory entry pointers */
char *nh_p2[MAXDNUMS + 1];		/* Network directory entry nettype */
char *nh_px[4][MAXDNUMS + 1];		/* Network-specific stuff... */
#endif /* NODIAL */
int nhcount = 0;
int ndinited = 0;
char * n_name = NULL;			/* Network name pointer */
static int oldplex = -1;		/* Duplex holder around network */
#endif /* NETCONN */

_PROTOTYP(static int remtxt, (char **) );
_PROTOTYP(VOID rmsg, (void) );
_PROTOTYP(static int remcfm, (void) );

#ifndef NOPUSH
extern int nopush;
#endif /* NOPUSH */

int mdmsav = -1;			/* Save modem type around network */

extern xx_strp xxstring;

extern int remfile, rempipe, remappd; /* REMOTE output redirection */
extern char * remdest;

#ifdef CK_TMPDIR
char * dldir = NULL;
#endif /* CK_TMPDIR */

extern struct ck_p ptab[];
extern int protocol;

extern int success, nfilp, fmask, fncnv, frecl, binary, warn, msgflg, quiet;
extern int cmask, maxrps, wslotr, bigsbsiz, bigrbsiz, urpsiz, rpsiz, spsiz;
extern int spsizr, spsizf, maxsps, spmax, pflag, bctr, npad, timef, timint;
extern int pkttim, rtimo, local, nfils, displa, atcapr, nettype, escape;
extern int mdmtyp, duplex, dfloc, network, cdtimo, fncact, mypadn, autoflow;
extern int tnlm, sosi, tlevel, lf_opts, backgrd, flow, fdispla, b_save, f_save;
extern int fnrpath, fnspath, debses, parity, pktpaus, ttnproto, ckxech;
extern int x_ifnum;
extern int
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso;
extern char psave[];			/* For saving & restoring prompt */

#ifdef OS2
#ifdef NT
#define stricmp _stricmp
extern int tt_attr_bug;
#endif /* NT */
extern int tt_rows[], tt_cols[];
extern int tt_cols_usr;
extern int tt_szchng[VNUM];
int tt_modechg = 1;
extern struct _vtG G[4], *GL, *GR, *GNOW, *SSGL;
struct _vtG savedG[4];
extern int priority;
extern bool send_c1;
int send_c1_usr = FALSE;
extern int sgrcolors;
#else
extern int tt_rows, tt_cols;
#endif /*  OS2 */

#ifdef STRATUS
extern int atfrmi, atfrmo, atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */

extern int tt_escape;
extern long speed;

extern CHAR 
  feol, sstate, eol, seol, stchr, mystch, mypadc, padch, ctlq, myctlq;

extern char *cmarg, *cmarg2, *dftty;

extern char *tp, *lp;			/* Temporary buffer & pointers */
#ifndef NOFRILLS
extern char optbuf[];			/* Buffer for MAIL or PRINT options */
extern int rprintf;			/* REMOTE PRINT flag */
#endif /* NOFRILLS */
extern char ttname[];

int tttapi = 0;				/* is Line TAPI? */
struct keytab * tapilinetab = NULL;
int ntapiline = 0;

#ifdef NETCONN				/* Network items */

#ifdef ANYX25
extern int revcall, closgr, cudata, nx25, npadx3;
extern char udata[];
extern CHAR padparms[];
extern struct keytab x25tab[], padx3tab[];
#endif /* ANYX25 */

#ifdef OS2
#ifndef NT
extern bool ttslip,ttppp;
#endif /* NT */
#endif /* OS2 */
#ifdef NPIPE
extern char pipename[];
#endif /* NPIPE */

#ifdef TCPSOCKET
#ifdef TNCODE
extern int tn_init;
#endif /* TNCODE */
_PROTOTYP( int tn_snaws, (void) );
#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );
#endif /* RLOGCODE */
extern int nawsflg;

extern int me_binary, u_binary;
#endif /* TCPSOCKET */

#ifdef SUPERLAT
extern char slat_pwd[18];
#endif /* SUPERLAT */

#endif /* NETCONN */

#ifdef COMMENT
#ifndef NOSETKEY
extern KEY *keymap;
#ifndef OS2
#define mapkey(x) keymap[x]
#endif /* OS2 */
extern MACRO *macrotab;
_PROTOTYP( VOID shostrdef, (CHAR *) );
#ifndef NOKVERBS
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#else
#ifndef NOSETKEY
extern KEY *keymap;
extern MACRO *macrotab;
_PROTOTYP( VOID shostrdef, (CHAR *) );
#ifndef NOKVERBS
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#endif /* COMMENT */

/* Keyword tables ... */

extern struct keytab onoff[], filtab[], rltab[];
extern int nrlt;
 
struct keytab fttab[] = {		/* File types for SET FILE TYPE */
    "ascii",     XYFT_B, CM_INV,
#ifdef VMS
    "b",         XYFT_B, CM_INV|CM_ABR,
#endif /* VMS */
    "binary",    XYFT_B, 0,
#ifdef VMS
    "block",     XYFT_I, CM_INV,
    "image",     XYFT_I, 0,
#endif /* VMS */
#ifdef CK_LABELED
    "labeled",   XYFT_L, 0,
#endif /* CK_LABELED */
#ifdef MAC
    "macbinary", XYFT_M, 0,
#endif /* MAC */
    "text",      XYFT_T, 0
};
int nfttyp = (sizeof(fttab) / sizeof(struct keytab));

static struct keytab rfttab[] = {	/* File types for REMOTE SET FILE */
    "ascii",     XYFT_B, CM_INV,
    "binary",    XYFT_B, 0,
#ifdef VMS
    "labeled",   XYFT_L, 0,
#else
#ifdef OS2
    "labeled",   XYFT_L, 0,
#endif /* OS2 */
#endif /* VMS */
    "text",      XYFT_T, 0
};
static int nrfttyp = (sizeof(rfttab) / sizeof(struct keytab));

extern char uidbuf[];
static int uidflag = 0;

#ifndef NOSPL

int query = 0;				/* Global flag for QUERY active */

static struct keytab vartyp[] = {	/* Variable types for REMOTE QUERY */
    "global", (int) 'G', CM_INV,
    "kermit", (int) 'K', 0,
    "system", (int) 'S', 0,
    "user",   (int) 'G', 0
};
static int nvartyp = (sizeof(vartyp) / sizeof(struct keytab));
#endif /* NOSPL */

#ifdef CK_TIMERS
static struct keytab timotab[] = {	/* Timer types */
    "dynamic", 1, 0,
    "fixed",   0, 0
};
#endif /* CK_TIMERS */

#ifdef DCMDBUF
extern char *atxbuf, *atmbuf;			/* Atom buffer */
extern char *cmdbuf;			/* Command buffer */
extern char *line, *tmpbuf;		/* Character buffers for anything */
extern int *intime;			/* INPUT TIMEOUT */

#else  /* Not DCMDBUF ... */

extern char atxbuf[], atmbuf[];		/* Atom buffer */
extern char cmdbuf[];			/* Command buffer */
extern char line[], tmpbuf[];		/* Character buffer for anything */
extern int intime[];

#endif /* DCMDBUF */

#ifndef NOCSETS
extern struct keytab fcstab[];		/* For SET FILE CHARACTER-SET */
extern struct keytab ttcstab[];
extern int nfilc, fcharset, tcharset, ntermc, tcsr, tcsl;
#ifdef OS2
_PROTOTYP( int os2setcp, (int) );
_PROTOTYP( int os2getcp, (void) );
_PROTOTYP( void os2debugoff, (void) );
#endif /* OS2 */
#endif /* NOCSETS */

#ifndef NOSPL
extern int cmdlvl;			/* Overall command level */
#ifdef DCMDBUF
extern int *inpcas;			/* INPUT CASE setting on cmd stack */
#else
extern int inpcas[];
#endif /* DCMDBUF */
#endif /* NOSPL */

#ifdef CK_CURSES
#ifndef VMS
#ifndef COHERENT
_PROTOTYP(int tgetent,(char *, char *));
#endif /* COHERENT */
#else
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */
#endif /* CK_CURSES */

#ifndef NOXMIT
#define XMITF 0				/* SET TRANSMIT values */
#define XMITL 1				/* (Local to this module) */
#define XMITP 2
#define XMITE 3
#define XMITX 4
#define XMITS 5
#define XMITW 6

#define XMBUFL 50
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw;
char xmitbuf[XMBUFL+1] = { NUL };	/* TRANSMIT eof string */

struct keytab xmitab[] = {		/* SET TRANSMIT */
    "echo",     XMITX, 0,
    "eof",      XMITE, 0,
    "fill",     XMITF, 0,
    "linefeed", XMITL, 0,
    "locking-shift", XMITS, 0,
    "pause",    XMITW, 0,
    "prompt",   XMITP, 0
};
int nxmit = (sizeof(xmitab) / sizeof(struct keytab));
#endif /* NOXMIT */

/* For SET FILE COLLISION */
/* Some of the following may be possible for some C-Kermit implementations */
/* but not others.  Those that are not possible for your implementation */
/* should be ifdef'd out. */

struct keytab colxtab[] = { /* SET FILE COLLISION options */
#ifndef MAC
    "append",    XYFX_A, 0,  /* append to old file */
#endif /* MAC */
#ifdef COMMENT
    "ask",       XYFX_Q, 0,  /* ask what to do (not implemented) */
#endif
    "backup",    XYFX_B, 0,  /* rename old file */
#ifndef MAC
    /* This crashes Mac Kermit. */
    "discard",   XYFX_D, 0,  /* don't accept new file */
    "no-supersede", XYFX_D, CM_INV, /* ditto (MSK compatibility) */
#endif /* MAC */
    "overwrite", XYFX_X, 0,  /* overwrite the old file == file warning off */
    "rename",    XYFX_R, 0   /* rename the incoming file == file warning on */
#ifndef MAC
    /* This crashes Mac Kermit. */
,   "update",    XYFX_U, 0  /* replace if newer */
#endif /* MAC */
};
int ncolx = (sizeof(colxtab) / sizeof(struct keytab));

static struct keytab rfiltab[] = {	/* for REMOTE SET FILE */
    "collision",     XYFILX, 0,
    "names",         XYFILN, 0,
    "record-length", XYFILR, 0,
    "type",          XYFILT, 0
};
int nrfilp = (sizeof(rfiltab) / sizeof(struct keytab));

struct keytab eoftab[] = {   		/* File eof delimiters */
    "cr",        XYFA_C, 0,
    "crlf",      XYFA_2, 0,
    "lf",        XYFA_L, 0
};
static int neoftab = (sizeof(eoftab) / sizeof(struct keytab));

struct keytab fntab[] = {   		/* File naming */
    "converted", XYFN_C, 0,
    "literal",   XYFN_L, 0
};
int nfntab = (sizeof(fntab) / sizeof(struct keytab));

#ifndef NOLOCAL
/* Terminal parameters table */
static struct keytab trmtab[] = {
#ifdef OS2
    "answerback",    XYTANS, 0,
#endif /* OS2 */
#ifdef CK_APC
    "apc",           XYTAPC, 0,
#endif /* CK_APC */
#ifdef OS2
    "arrow-keys",    XYTARR, 0,
#endif /* OS2 */
#ifdef NT
    "attr",	     XYTATTR, CM_INV|CM_ABR,
    "attr-bug",      XYTATTBUG, CM_INV,
#endif /* NT */
#ifdef OS2
    "attribute",     XYTATTR, 0,
#endif /* OS2 */
#ifdef CK_APC
#ifdef CK_AUTODL
   "autodownload",   XYTAUTODL, 0,
#endif /* CK_AUTODL */
#endif /* CK_APC */
#ifdef OS2
    "bell",          XYTBEL, CM_INV,
#endif /* OS2 */
    "bytesize",      XYTBYT, 0,
#ifndef NOCSETS
#ifndef KUI
#ifdef OS2
    "character-set", XYTCS,  CM_INV,
#else /* OS2 */
    "character-set", XYTCS,  0,
#endif /* OS2 */
#endif /* KUI */
#endif /* NOCSETS */
#ifdef OS2
    "code-page",     XYTCPG, 0,
    "color",         XYTCOL, 0,
    "controls",      XYTCTRL, 0,
#endif /* OS2 */
    "cr-display",    XYTCRD, 0,
#ifdef OS2
    "cursor",        XYTCUR, 0,
#endif /* OS2 */
    "debug",         XYTDEB, 0,
    "echo",          XYTEC,  0,
    "escape-character", XYTESC, 0,
#ifdef OS2
#ifdef PCFONTS
    "font",          XYTFON, 0,
#endif /* PCFONTS */
#endif /* OS2 */
    "height",        XYTHIG, 0,
#ifdef OS2
#ifdef COMMENT
    /* not needed anymore -- 5a(191) new screen handler mechanism */
    "hide-cursor",   XYTHCU, 0,
#endif /* COMMENT */
    "key",           XYTKEY, 0,
    "keypad-mode",   XYTKPD, 0,
#endif /* OS2 */
#ifndef NOCSETS
#ifdef OS2
    "local-character-set", XYTLCS,  0,
#else
    "local-character-set", XYTLCS,  CM_INV,
#endif /* OS2 */
#endif /* NOCSETS */
    "locking-shift", XYTSO,  0,
#ifdef OS2MOUSE
    "mouse",         XYTMOU, CM_INV,
#endif /* OS2MOUSE */
    "newline-mode",  XYTNL,  0,
#ifdef OS2
    "output-pacing", XYTPAC, 0,
#ifndef NOCSETS
#ifdef OS2
    "remote-character-set", XYTRCS,  0,
#else
    "remote-character-set", XYTRCS,  CM_INV,
#endif /* OS2 */
#endif /* NOCSETS */
    "roll-mode",     XYTROL, 0,
    "screen-update", XYTUPD, 0,
    "scrollback",    XYSCRS, 0,
    "send-data",     XYTSEND, 0,
    "send-end-of-block", XYTSEOB, 0,
    "sgr-colors",    XYTSGRC, 0,
    "statusline",    XYTSTAT, 0,
    "transmit-timeout", XYTCTS, 0,
    "type",          XYTTYP, 0,
#else
    "type",          XYTTYP, CM_INV,
#endif /* OS2 */

#ifdef OS2
#ifndef NOCSETS
#ifdef CKOUNI
    "unicode",       XYTUNI, CM_INV,
#endif /* CKOUNI */
#endif /* NOCSETS */
#ifdef NT
    "video-change",  XYTVCH, 0,
#endif /* NT */
#endif /* OS2 */
    "width",         XYTWID, 0,
#ifdef OS2
    "wrap",          XYTWRP, 0,
#endif /* OS2 */
    "", 0, 0
};
int ntrm = (sizeof(trmtab) / sizeof(struct keytab)) - 1;

#ifdef OS2
struct keytab termctrl[] = { 	/* SET TERM CONTROLS */
    "7", 	7, 0,
    "8", 	8, 0
};
int ntermctrl = (sizeof(termctrl) / sizeof(struct keytab));

struct keytab rolltab[] = {   /* Set TERM Roll Options */
    "insert",    TTR_INSERT, 0,
    "off",       TTR_OVER,   CM_INV,
    "on",        TTR_INSERT, CM_INV,
    "overwrite", TTR_OVER,   0
};
int nroll = (sizeof(rolltab) / sizeof(struct keytab));

#define TT_GR_ALL 4
#define TT_GR_G0  0
#define TT_GR_G1  1
#define TT_GR_G2  2
#define TT_GR_G3  3
struct keytab graphsettab[] = {  /* DEC VT Graphic Sets */
    "all",   TT_GR_ALL, 0,
    "g0",    TT_GR_G0,  0,
    "g1",    TT_GR_G1,  0,
    "g2",    TT_GR_G2,  0,
    "g3",    TT_GR_G3,  0
};
int ngraphset = (sizeof(graphsettab) / sizeof(struct keytab));

#endif /* OS2 */

struct keytab crdtab[] = {		/* Carriage-return display */
    "crlf",        1, 0,
    "normal",      0, 0
};
extern int tt_crd;			/* Carriage-return display variable */

#ifdef CK_APC
extern int apcstatus, apcactive;
static struct keytab apctab[] = {	/* Terminal APC parameters */
    "off",	 APC_OFF,  0,
    "on",	 APC_ON,   0,
    "unchecked", APC_UNCH, 0
};
#endif /* CK_APC */
#endif /* NOLOCAL */

extern int autodl;

#ifdef OS2
/*
  OS/2 serial communication devices.
*/
struct keytab os2devtab[] = {
    "1",    1, CM_INV,			/* Invisible synonyms, like */
    "2",    2, CM_INV,			/* "set port 1" */
    "3",    3, CM_INV,
    "4",    4, CM_INV,
    "5",    5, CM_INV,
    "6",    6, CM_INV,
    "7",    7, CM_INV,
    "8",    8, CM_INV,
    "com1", 1, 0,			/* Real device names */
    "com2", 2, 0,
    "com3", 3, 0,
    "com4", 4, 0,
    "com5", 5, 0,
    "com6", 6, 0,
    "com7", 7, 0,
    "com8", 8, 0
#ifdef OS2ONLY
   ,"slipcom1", 1, 0,          /* For use with SLIP driver */
    "slipcom2", 2, 0,
    "slipcom3", 3, 0,          /* shared access            */
    "slipcom4", 4, 0,
    "slipcom5", 5, 0,
    "slipcom6", 6, 0,
    "slipcom7", 7, 0,
    "slipcom8", 8, 0,
    "pppcom1", 1, 0,          /* For use with PPP driver */
    "pppcom2", 2, 0,
    "pppcom3", 3, 0,          /* shared access            */
    "pppcom4", 4, 0,
    "pppcom5", 5, 0,
    "pppcom6", 6, 0,
    "pppcom7", 7, 0,
    "pppcom8", 8, 0
#endif /* OS2ONLY */
};
int nos2dev = (sizeof(os2devtab) / sizeof(struct keytab));

#ifdef OS2ONLY
struct keytab os2ppptab[] = {
    "0",    0, CM_INV,
    "1",    1, CM_INV,			/* Invisible synonyms, like */
    "2",    2, CM_INV,			/* "set port 1" */
    "3",    3, CM_INV,
    "4",    4, CM_INV,
    "5",    5, CM_INV,
    "6",    6, CM_INV,
    "7",    7, CM_INV,
    "8",    8, CM_INV,
    "9",    9, CM_INV,
    "ppp0", 0, 0,
    "ppp1", 1, 0,          /* For use with PPP driver */
    "ppp2", 2, 0,
    "ppp3", 3, 0,          /* shared access            */
    "ppp4", 4, 0,
    "ppp5", 5, 0,
    "ppp6", 6, 0,
    "ppp7", 7, 0,
    "ppp8", 8, 0,
    "ppp9", 9, 0
};
int nos2ppp = (sizeof(os2ppptab) / sizeof(struct keytab));
#endif /* OS2ONLY */

/*
  Terminal parameters that can be set by SET commands.
  Used by the ck?con.c terminal emulator code.  
  For now, only used for #ifdef OS2.  Should add these for Macintosh.
*/
int tt_arrow = TTK_NORM;		/* Arrow key mode: normal (cursor) */
int tt_keypad = TTK_NORM;		/* Keypad mode: normal (numeric) */
int tt_shift_keypad = 0; 	        /* Keypad Shift mode: Off */
int tt_wrap = 1;			/* Terminal wrap, 1 = On */
int tt_type = TT_VT320;			/* Terminal type, initially VT320 */
int tt_type_mode = TT_VT320;    	/* Terminal type set by host command */
int tt_cursor = 0;			/* Terminal cursor, 0 = Underline */
int tt_cursor_usr = 0;			/* Users Terminal cursor type */
int tt_answer = 0;			/* Terminal answerback (disabled) */
int tt_scrsize[VNUM] = {512,512,512,1};	/* Terminal scrollback buffer size */
int tt_bell = XYB_AUD | XYB_SYS;	/* Bell (system sounds) */
int tt_roll[VNUM] = {1,1,1,1};		/* Terminal roll (on) */
int tt_pacing = 0;			/* Terminal output-pacing (none) */
int tt_inpacing = 0;			/* Terminal input-pacing (none) */
#ifdef COMMENT 
int tt_hide = 0;			/* Terminal hide-cursor (off) */
#endif /* COMMENT */
int tt_ctstmo = 15;			/* Terminal transmit-timeout */
int tt_codepage = -1;			/* Terminal code-page */
int tt_update = 100;			/* Terminal screen-update interval */
int tt_updmode = TTU_FAST;		/* Terminal screen-update mode FAST */
extern int updmode;
int tt_font = TTF_ROM;			/* Terminal screen font */
#ifndef KUI
int tt_status = 1;			/* Terminal status line displayed */
int tt_status_usr = 1;
#else  /* KUI */
int tt_status = 0;			/* Terminal status line displayed */
int tt_status_usr = 0;
#endif /* KUI */
#ifdef CKOUNI
int tt_unicode = 1;			/* Use Unicode if possible */
#endif /* CKOUNI */
int tt_senddata = 0;			/* Let host read terminal data */
extern int wy_blockend;			/* Terminal Send Data EOB type */

extern unsigned char colornormal, colorselect,
colorunderline, colorstatus, colorhelp, colorborder,
colorgraphic, colordebug, colorreverse;

extern int trueblink, trueunderline, truereverse;

extern int bgi, fgi;
extern int scrninitialized[];

struct keytab beltab[] = {		/* Terminal bell mode */
    "audible", XYB_AUD, 0,
    "none",    XYB_NONE, 0,
    "visible", XYB_VIS, 0
};
int nbeltab = sizeof(beltab)/sizeof(struct keytab);

struct keytab audibletab[] = {		/* Terminal Bell Audible mode */
    "beep",          XYB_BEEP, 0,	/* Values ORd with bell mode */
    "system-sounds", XYB_SYS, 0   
};
int naudibletab = sizeof(audibletab)/sizeof(struct keytab);

struct keytab akmtab[] = {		/* Arrow key mode */
    "application", TTK_APPL, 0,
    "cursor",      TTK_NORM, 0
};
struct keytab kpmtab[] = {		/* Keypad mode */
    "application", TTK_APPL, 0,
    "numeric",     TTK_NORM, 0
};

struct keytab ttcolmodetab[] = {
    "current-color", 0, 0,
    "default-color",  1, 0
};
int ncolmode = sizeof(ttcolmodetab)/sizeof(struct keytab);

#define TTCOLNOR  0
#define TTCOLREV  1
#define TTCOLUND  2
#define TTCOLSTA  3
#define TTCOLHLP  4
#define TTCOLBOR  5
#define TTCOLSEL  6
#define TTCOLDEB  7
#define TTCOLGRP  8

#define TTCOLRES  10
#define TTCOLERA  11

struct keytab ttycoltab[] = {			/* Terminal Screen coloring */
    "border",             TTCOLBOR, 0,		/* Screen border color */
    "debug-terminal",     TTCOLDEB, 0,		/* Debug color */
    "erase",              TTCOLERA, 0,          /* Erase mode */
    "graphic",            TTCOLGRP, 0,          /* Graphic Color */
    "help-text",          TTCOLHLP, 0,		/* Help screens */
    "normal",             TTCOLNOR, CM_INV,	/* Normal screen text */
    "reset-on-esc[0m",    TTCOLRES, 0,          /* Reset on ESC [ 0 m */
    "reverse-video",      TTCOLREV, 0,          /* Reverse video */
    "status-line",        TTCOLSTA, 0,		/* Status line */
    "selection",          TTCOLSEL, 0,		/* Selection color */
    "terminal-screen",    TTCOLNOR, 0,		/* Better name than "normal" */
    "underlined-text",    TTCOLUND, 0    	/* Underlined text */
};
int ncolors = (sizeof(ttycoltab) / sizeof(struct keytab));

#define TTATTBLI 0
#define TTATTREV 1
#define TTATTUND 2

struct keytab ttyattrtab[] = {
    "blink", 	TTATTBLI, 0,
    "reverse",	TTATTREV, 0,
    "underline",TTATTUND, 0
};
int nattrib = (sizeof(ttyattrtab) / sizeof(struct keytab));

struct keytab ttyseobtab[] = {
    "crlf_etx",	 1, 0,
    "us_cr",	 0, 0
};

struct keytab ttyclrtab[] = {		/* Colors */
    "black",       0, 0,
    "blue",        1, 0,
    "brown",       6, 0,
    "cyan",        3, 0,
    "darkgray",    8, CM_INV,
    "dgray",       8, 0,
    "green",       2, 0,
    "lblue",       9, CM_INV,
    "lcyan",      11, CM_INV,
    "lgray",       7, CM_INV,
    "lgreen",     10, CM_INV,
    "lightblue",   9, 0,
    "lightcyan",  11, 0,
    "lightgray",   7, 0,
    "lightgreen", 10, 0,
    "lightmagenta",13,0,
    "lightred",   12, 0,
    "lmagenta",   13, CM_INV,
    "lred",       12, CM_INV,
    "magenta",     5, 0,
    "red",         4, 0,
    "white",      15, 0,
    "yellow",     14, 0
};
int nclrs = (sizeof (ttyclrtab) / sizeof (struct keytab));

struct keytab ttycurtab[] = {
    "full",        TTC_BLOCK, 0,
    "half",        TTC_HALF,  0,
    "underline",   TTC_ULINE, 0
};
int ncursors = 3;

struct keytab ttyptab[] = {
    "ansi-bbs", TT_ANSI,    0,		/* ANSI.SYS (BBS) */
    "at386",    TT_AT386,   0,		/* Unixware ANSI */
    "avatar/0+", TT_ANSI,   0,          /* AVATAR/0+ */
    "dg200",    TT_DG200,   0,		/* Data General DASHER 200 */
    "dg210",    TT_DG200,   0,          /* Data General DASHER 210 */
    "h19",      TT_H19,     CM_INV,	/* Heath-19 */
    "heath19",  TT_H19,     0,		/* Heath-19 */
    "hp2621",   TT_HP2621,  0,		/* HP 2621A */
    "hz1500",   TT_HZL1500, 0,    	/* Hazeltine 1500 */
#ifdef COMMENT
    "ibm",      TT_IBM,     CM_INV,     /* IBM 3101-xx,3161 */
#endif /* COMMENT */
    "scoansi",  TT_SCOANSI, 0,		/* SCO ANSI */
/*
  The idea of NONE is to let the console driver handle the escape sequences,
  which, in theory at least, would give not only ANSI emulation, but also any
  other kind of emulation that might be provided by alternative console
  drivers, if any existed.

  For this to work, ckocon.c would need to be modified to make higher-level
  calls, like VioWrtTTY(), DosWrite(), or (simply) write(), rather than
  VioWrt*Cell() and similar, and it would also have to give up its rollback
  feature, and its status line and help screens would also have to be
  forgotten or else done in an ANSI way.

  As matters stand, we already have perfectly good ANSI emulation built in,
  and there are no alternative console drivers available, so there is no point
  in having a terminal type of NONE, so it is commented out.  However, should
  you uncomment it, it will work like a "glass tty" -- no escape sequence
  interpretation at all; somewhat similar to debug mode, except without the
  debugging (no highlighting of control chars or escape sequences); help
  screens, status line, and rollback will still work.
*/
#ifdef OS2PM
#ifdef COMMENT
    "tek4014", TT_TEK40, 0,
#endif /* COMMENT */
#endif /* OS2PM */
    "tty",     TT_NONE,  0,
    "tvi910+", TT_TVI910, 0,
    "tvi925",  TT_TVI925, 0,
    "tvi950",  TT_TVI950, 0,
    "vc404",   TT_VC4404, 0,
    "vc4404",  TT_VC4404, CM_INV,
    "vt100",   TT_VT100, 0,
    "vt102",   TT_VT102, 0,
    "vt220",   TT_VT220, 0,
    "vt320",   TT_VT320, 0,
    "vt52",    TT_VT52,  0,
    "wy30",    TT_WY30,  0,
    "wy370",   TT_WY370, 0,
    "wy50",    TT_WY50,  0,
    "wy60",    TT_WY60,  0,
    "wyse30",  TT_WY30,  CM_INV,
    "wyse370", TT_WY370, CM_INV,
    "wyse50",  TT_WY50,  CM_INV,
    "wyse60",  TT_WY60,  CM_INV
};
int nttyp = (sizeof(ttyptab) / sizeof(struct keytab));

struct keytab ttkeytab[] = {
    "ansi-bbs",  TT_ANSI,    0,		/* ANSI.SYS (BBS) */
    "at386",     TT_AT386,   0,		/* Unixware ANSI */
    "avatar/0+", TT_ANSI,   0,          /* AVATAR/0+ */
    "dg200",     TT_DG200,   0,		/* Data General DASHER 200 */
    "dg210",     TT_DG200,   0,         /* Data General DASHER 210 */
    "emacs",     TT_MAX+1,   0,         /* Emacs mode */ 
    "h19",       TT_H19,     CM_INV,	/* Heath-19 */
    "heath19",   TT_H19,     0,		/* Heath-19 */
    "hebrew",    TT_MAX+2,   0,         /* Hebrew mode */
    "hp2621",    TT_HP2621,  0,		/* HP 2621A */
    "hz1500",    TT_HZL1500, 0,    	/* Hazeltine 1500 */
#ifdef COMMENT
    "ibm",       TT_IBM,     CM_INV,    /* IBM 3101-xx,3161 */
#endif /* COMMENT */
    "russian",   TT_MAX+3,   0,         /* Russian mode */
    "scoansi",   TT_SCOANSI, 0,		/* SCO ANSI */
#ifdef OS2PM
#ifdef COMMENT
    "tek4014", TT_TEK40, 0,
#endif /* COMMENT */
#endif /* OS2PM */
    "tty",     TT_NONE,  0,
    "tvi910+", TT_TVI910, 0,
    "tvi925",  TT_TVI925, 0,
    "tvi950",  TT_TVI950, 0,
    "vc404",   TT_VC4404, 0,
    "vc4404",  TT_VC4404, CM_INV,
    "vt100",   TT_VT100, 0,
    "vt102",   TT_VT102, 0,
    "vt220",   TT_VT220, 0,
    "vt320",   TT_VT320, 0,
    "vt52",    TT_VT52,  0,
    "wy30",    TT_WY30,  0,
    "wy370",   TT_WY370, 0,
    "wy50",    TT_WY50,  0,
    "wy60",    TT_WY60,  0,
    "wyse30",  TT_WY30,  CM_INV,
    "wyse370", TT_WY370, CM_INV,
    "wyse50",  TT_WY50,  CM_INV,
    "wyse60",  TT_WY60,  CM_INV
};
int nttkey = (sizeof(ttkeytab) / sizeof(struct keytab));

struct keytab prtytab[] = { /* OS/2 Priority Levels */
    "foreground-server", XYP_SRV, 0,
    "idle",              XYP_IDLE, CM_INV,
    "regular",           XYP_REG, 0,
    "time-critical",     XYP_RTP, 0
};
int nprty = (sizeof(prtytab) / sizeof(struct keytab));
#endif /* OS2 */

#ifdef NT
struct keytab win95tab[] = { /* Win95 work-arounds */
    "alt-gr", 		     XYWAGR, 0,
    "keyboard-translation",  XYWKEY, 0,
    "overlapped-io",         XYWOIO, 0
};
int nwin95 = (sizeof(win95tab) / sizeof(struct keytab));
#endif /* NT */

#ifdef OS2MOUSE
extern int wideresult;
int tt_mouse = 1;			/* Terminal mouse on/off */

struct keytab mousetab[] = {		/* Mouse items */
    "activate", XYM_ON,    0,
    "button",   XYM_BUTTON, 0,
    "clear",    XYM_CLEAR, 0
};
int nmtab = (sizeof(mousetab)/sizeof(struct keytab));

struct keytab mousebuttontab[] = {	/* event button */
    "one",           XYM_B1, CM_INV,
    "three",         XYM_B3, CM_INV,
    "two",           XYM_B2, CM_INV,
    "1",             XYM_B1, 0,
    "2",             XYM_B2, 0,
    "3",             XYM_B3, 0
};
int nmbtab = (sizeof(mousebuttontab) / sizeof(struct keytab));

struct keytab mousemodtab[] = {		/* event button key modifier */
    "alt",		XYM_ALT,   0,
    "alt-shift",	XYM_SHIFT|XYM_ALT, 0,
    "ctrl",		XYM_CTRL,  0,
    "ctrl-alt",		XYM_CTRL|XYM_ALT, 0,
    "ctrl-alt-shift",	XYM_CTRL|XYM_SHIFT|XYM_ALT, 0,
    "ctrl-shift",	XYM_CTRL|XYM_SHIFT, 0,
    "none",  		0, 0,
    "shift",		XYM_SHIFT, 0
};
int nmmtab = (sizeof(mousemodtab) / sizeof(struct keytab));

struct keytab mclicktab[] = {		/* event button click modifier */
    "click",        XYM_C1,  0,
    "drag",         XYM_DRAG, 0,
    "double-click", XYM_C2,  0
};
int nmctab = (sizeof(mclicktab) / sizeof(struct keytab));

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */
#endif /* OS2MOUSE */

/* #ifdef VMS */
struct keytab fbtab[] = {		/* Binary record types for VMS */
    "fixed",     XYFT_B, 0,		/* Fixed is normal for binary */
    "undefined", XYFT_U, 0		/* Undefined if they ask for it */
};
int nfbtyp = (sizeof(fbtab) / sizeof(struct keytab));
/* #endif */

#ifdef VMS
struct keytab lbltab[] = {		/* Labeled File info */
    "acl",         LBL_ACL, 0,
    "backup-date", LBL_BCK, 0,
    "name",        LBL_NAM, 0,
    "owner",       LBL_OWN, 0,
    "path",        LBL_PTH, 0
};
int nlblp = (sizeof(lbltab) / sizeof(struct keytab));
#else
#ifdef OS2
struct keytab lbltab[] = {		/* Labeled File info */
    "archive",   LBL_ARC, 0,
    "extended",  LBL_EXT, 0,
    "hidden",    LBL_HID, 0,
    "read-only", LBL_RO,  0,
    "system",    LBL_SYS, 0
};
int nlblp = (sizeof(lbltab) / sizeof(struct keytab));
#endif /* OS2 */
#endif /* VMS */

#ifdef CK_CURSES
#ifdef CK_PCT_BAR
static struct keytab fdftab[] = {	/* SET FILE DISPLAY FULL options */
    "thermometer", 1, 0,
    "no-thermometer", 0, 0
};
extern int thermometer;
#endif /* CK_PCT_BAR */
#endif /* CK_CURSES */

static struct keytab fdtab[] = {	/* SET FILE DISPLAY options */
#ifdef MAC				/* Macintosh */
    "fullscreen", XYFD_R, 0,		/* Full-screen but not curses */
    "none",   XYFD_N, 0,
    "off",    XYFD_N, CM_INV,
    "on",     XYFD_R, CM_INV,
    "quiet",  XYFD_N, CM_INV

#else					/* Not Mac */
    "crt", XYFD_S, 0,			/* CRT display */
#ifdef CK_CURSES
#ifdef COMMENT
    "curses",     XYFD_C, CM_INV,	/* Full-screen, curses */
#endif /* COMMENT */
    "fullscreen", XYFD_C, 0,		/* Full-screen, whatever the method */
#endif /* CK_CURSES */
    "none",   XYFD_N, 0,		/* No display */
    "off",    XYFD_N, CM_INV,		/* Ditto */
    "on",     XYFD_R, CM_INV,		/* On = Serial */
    "quiet",  XYFD_N, CM_INV,		/* No display */
    "serial", XYFD_R, 0			/* Serial */
#endif /* MAC */
};
int nfdtab = (sizeof(fdtab) / sizeof(struct keytab));

struct keytab rsrtab[] = {		/* For REMOTE SET RECEIVE */
    "packet-length", XYLEN, 0,
    "timeout", XYTIMO, 0
};
int nrsrtab = (sizeof(rsrtab) / sizeof(struct keytab));

/* Send/Receive Parameters */
 
struct keytab srtab[] = {
    "control-prefix", XYQCTL, 0,
    "end-of-packet", XYEOL, 0,
    "packet-length", XYLEN, 0,
    "pad-character", XYPADC, 0,
    "padding", XYNPAD, 0,
    "pathnames", XYFPATH, 0,
    "pause", XYPAUS, 0,
    "quote", XYQCTL, CM_INV,		/* = CONTROL-PREFIX */
    "start-of-packet", XYMARK, 0,
    "timeout", XYTIMO, 0
};
int nsrtab = (sizeof(srtab) / sizeof(struct keytab));

/* REMOTE SET */

struct keytab rmstab[] = {
    "attributes",  XYATTR, 0,
    "block-check", XYCHKT, 0,
    "file",        XYFILE, 0,
    "incomplete",  XYIFD,  0,
    "receive",     XYRECV, 0,
    "retry",       XYRETR, 0,
    "server",      XYSERV, 0,
    "transfer",    XYXFER, 0,
    "window",      XYWIND, 0
};
int nrms = (sizeof(rmstab) / sizeof(struct keytab));

struct keytab attrtab[] = {
#ifdef STRATUS
    "account",	     AT_ACCT, 0,
#endif /* STRATUS */
    "all",           AT_XALL, 0,
#ifdef COMMENT
    "blocksize",     AT_BLKS, 0,	/* not used */
#endif /* COMMENT */
#ifndef NOCSETS
    "character-set", AT_ENCO, 0,
#endif /* NOCSETS */
#ifdef STRATUS
    "creator",	     AT_CREA, 0,
#endif /* STRATUS */
    "date",          AT_DATE, 0,
    "disposition",   AT_DISP, 0,
    "encoding",      AT_ENCO, CM_INV,
#ifdef STRATUS
    "format",	     AT_RECF, 0,
#endif /* STRATUS */
    "length",        AT_LENK, 0,
    "off",           AT_ALLN, 0,
    "on",            AT_ALLY, 0,
#ifdef COMMENT
    "os-specific",   AT_SYSP, 0,	/* not used by UNIX or VMS */
#endif /* COMMENT */
    "system-id",     AT_SYSI, 0,
    "type",          AT_FTYP, 0
};
int natr = (sizeof(attrtab) / sizeof(struct keytab)); /* how many attributes */

#ifndef NOSPL
extern int indef, inecho, insilence, inbufsize;
extern char * inpbuf, * inpbp;
struct keytab inptab[] = {		/* SET INPUT parameters */
    "buffer-length",   IN_BUF, 0,
    "case",            IN_CAS, 0,
    "default-timeout", IN_DEF, CM_INV,	/* There is no default timeout */
    "echo",            IN_ECH, 0,
#ifdef OS2
    "pacing",          IN_PAC, CM_INV,
#endif /* OS2 */
    "silence",         IN_SIL, 0,
    "timeout-action",  IN_TIM, 0
};
int ninp = (sizeof(inptab) / sizeof(struct keytab));

struct keytab intimt[] = {		/* SET INPUT TIMEOUT parameters */
    "proceed", 0, 0,			/* 0 = proceed */
    "quit",    1, 0			/* 1 = quit */
};

struct keytab incast[] = {		/* SET INPUT CASE parameters */
    "ignore",  0, 0,			/* 0 = ignore */
    "observe", 1, 0			/* 1 = observe */
};
#endif /* NOSPL */

struct keytab nabltab[] = {		/* For any command that needs */
    "disabled", 0, 0,			/* these keywords... */
    "enabled",  1, 0
};

#ifdef OS2 
struct keytab msktab[] = { /* SET MS-DOS KERMIT compatibilities */
#ifdef COMMENT
    "color", MSK_COLOR,  0,
#endif /* COMMENT */
    "keycodes", MSK_KEYS, 0
};
int nmsk = (sizeof(msktab) / sizeof(struct keytab));

struct keytab scrnupd[] = { /* SET TERMINAL SCREEN-UPDATE */
    "fast", TTU_FAST,  0,          
    "smooth", TTU_SMOOTH, 0        
};
int nscrnupd = (sizeof(scrnupd) / sizeof(struct keytab));

struct keytab termfont[] = { /* SET TERMINAL FONT */
#ifdef COMMENT
    "cp111", TTF_111, 0,
    "cp112", TTF_112, 0,
    "cp113", TTF_113, 0,
#endif /* COMMENT */
    "cp437", TTF_437, 0,
    "cp850", TTF_850, 0,
#ifdef COMMENT
    "cp851", TTF_851, 0,
#endif /* COMMENT */
    "cp852", TTF_852, 0,
#ifdef COMMENT
    "cp853", TTF_853, 0,
    "cp860", TTF_860, 0,
    "cp861", TTF_861, 0,
#endif /* COMMENT */
    "cp862", TTF_862, 0,
#ifdef COMMENT
    "cp863", TTF_863, 0,
    "cp864", TTF_864, 0,
    "cp865", TTF_865, 0,
#endif /* COMMENT */
    "cp866", TTF_866, 0,
#ifdef COMMENT
    "cp880", TTF_880, 0,
    "cp881", TTF_881, 0,
    "cp882", TTF_882, 0,
    "cp883", TTF_883, 0,
    "cp884", TTF_884, 0,
    "cp885", TTF_885, 0,
#endif /* COMMENT */
    "default", TTF_ROM, 0
};
int ntermfont = (sizeof(termfont) / sizeof(struct keytab));

struct keytab anbktab[] = {		/* For any command that needs */
    "message", 2, 0,			/* these keywords... */
    "off",     0, 0,			
    "on",      1, 0,
    "unsafe-messag0", 99, CM_INV,
    "unsafe-message", 3,  CM_INV
};
int nansbk = (sizeof(anbktab) / sizeof(struct keytab));

#endif /* OS2 */


/* The following routines broken out of doprm() to give compilers a break. */

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */
 
int
seton(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(1);
}
 
/*  S E T N U M  --  Set parameter to result of cmnum() parse.  */
/*
 Call with pointer to integer variable to be set,
   x = number from cnum parse, y = return code from cmnum,
   max = maximum value to accept, -1 if no maximum.
 Returns -9 on failure, after printing a message, or 1 on success.
*/
int
setnum(prm,x,y,max) int x, y, *prm, max; {
    extern int cmflgs;
    debug(F101,"setnum","",y);
    if (y == -3) {
	printf("\n?Value required\n");
	return(-9);
    }
    if (y == -2) {
	printf("%s?Not a number: %s\n",cmflgs == 1 ? "" : "\n", atxbuf);
	return(-9);
    }
    if (y < 0) return(y);
    if (max > -1 && x > max) {
	printf("?Sorry, %d is the maximum\n",max);
	return(-9);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(1);
}
 
/*  S E T C C  --  Set parameter var to an ASCII control character value.  */
/*
  Parses a number, or a literal control character, or a caret (^) followed
  by an ASCII character whose value is 63-95 or 97-122, then gets confirmation,
  then sets the parameter to the code value of the character given.  If there
  are any parse errors, they are returned, otherwise on success 1 is returned.
*/
int
setcc(dflt,var) char *dflt; int *var; {
    int x, y;
    unsigned int c;
    char *hlpmsg = "Control character,\n\
 numeric ASCII value,\n\
 or in ^X notation,\n\
 or preceded by a backslash and entered literally";
    
    /* This is a hack to turn off complaints from expression evaluator. */
    x_ifnum = 1;
    y = cmnum(hlpmsg, dflt, 10, &x, xxstring); /* Parse a number */
    x_ifnum = 0;			       /* Allow complaints again */
    if (y < 0) {			/* Parse failed */
	if (y != -2)			/* Reparse needed or somesuch */
	  return(y);			/* Pass failure back up the chain */
    }
    /* Did they type a real control character? */

    for (c = strlen(atmbuf) - 1; c > 0; c--) /* Trim */
      if (atmbuf[c] == SP) atmbuf[c] = NUL;

    if (y < 0) {			/* It was not a number */
	if ((c = atmbuf[0]) && !atmbuf[1]) { /* Was it a literal Ctrl char? */
	    if ((c > 037) && (c != 0177)) {
		printf("\n?Not a control character - %d\n",c);
		return(-9);
	    } else {
		if ((y = cmcfm()) < 0)	/* Confirm */
		  return(y);
		*var = c;		/* Set the variable */
		return(1);
	    }
	} else if (atmbuf[0] == '^' && !atmbuf[2]) { /* Or ^X notation? */
	    c = atmbuf[1];
	    if (islower((char) c))	/* Uppercase lowercase letters */
	      c = toupper(c);
	    if (c > 62 && c < 96) {	/* Check range */
		if ((y = cmcfm()) < 0)
		  return(y);
		*var = ctl(c);		/* OK */
		return(1);
	    } else {
		printf("?Not a control character - %s\n", atmbuf);
		return(-9);
	    }
	} else {			/* Something illegal was typed */
	    printf("?Invalid - %s\n", atmbuf);
	    return(-9);
	}
    }
    if ((x > 037) && (x != 0177)) {	/* They typed a number */
	printf("\n?Not in ASCII control range - %d\n",x);
	return(-9);
    }
    if ((y = cmcfm()) < 0)		/* In range, confirm */
      return(y);
    *var = x;				/* Set variable */
    return(1);
}

int
doxdis() {
    int x, y, z;
    char *s;

    if ((x = cmkey(fdtab,nfdtab,"file transfer display style","",
		   xxstring)) < 0)
      return(x);
#ifdef CK_PCT_BAR
    if ((y = cmkey(fdftab,2,"","thermometer",xxstring)) < 0)
      return(y);
    thermometer = y;
#endif /* CK_PCT_BAR */
    if ((z = cmcfm()) < 0) return(z);
#ifdef CK_CURSES
    if (x == XYFD_C) {			/* FULLSCREEN */
#ifndef MYCURSES
	extern char * trmbuf;		/* Real curses */
	int z;
	s = getenv("TERM");
	if (!s) s = "";
#ifndef COHERENT
	if (*s)
	  z = tgetent(trmbuf,s);
	else
	  z = 0;
	debug(F111,"SET tgetent",s,z);
	if (z < 1) {
#ifdef VMS
	    printf("Sorry, terminal type not supported: \"%s\"\n",s);
#else
	    printf("Sorry, terminal type unknown: \"%s\"\n",s);
#endif /* VMS */
	    return(success = 0);
	}
#endif /* COHERENT */
#endif /* MYCURSES */
	line[0] = '\0';			/* (What's this for?) */
    }
#endif /* CK_CURSES */
    fdispla = x;			/* It's OK. */
    return(success = 1);
}

int
setfil(rmsflg) int rmsflg; {
    if (rmsflg) {
	if ((y = cmkey(rfiltab,nrfilp,"Remote file parameter","",
		       xxstring)) < 0) {
	    if (y == -3) {
		printf("?Remote file parameter required\n");
		return(-9);
	    } else return(y);
	}
    } else {
	if ((y = cmkey(filtab,nfilp,"File parameter","",xxstring)) < 0)
	  return(y);
    }
    switch (y) {
#ifdef COMMENT				/* Not needed */
      case XYFILB:			/* Blocksize */
	sprintf(tmpbuf,"%d",DBLKSIZ);
	if ((y = cmnum("file block size",tmpbuf,10,&z,xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",z);
	    sstate = setgen('S', "311", tmpbuf, "");
	    return((int) sstate);
	} else {
	    fblksiz = z;
	    return(success = 1);
	}
#endif /* COMMENT */

      case XYFILS:			/* Byte size */
	if ((y = cmnum("file byte size (7 or 8)","8",10,&z,xxstring)) < 0)
	  return(y);
	if (z != 7 && z != 8) {
	    printf("\n?The choices are 7 and 8\n");
	    return(0);
	}
	if ((y = cmcfm()) < 0) return(y);
	if (z == 7) fmask = 0177;
	else if (z == 8) fmask = 0377;
	return(success = 1);

#ifndef NOCSETS
      case XYFILC:			/* Character set */
	if ((x = cmkey(fcstab,nfilc,"local file code","ascii", xxstring)) < 0)
	  return(x);
	if ((z = cmcfm()) < 0) return(z);
	fcharset = x;
#ifndef MAC
	if (tcharset == TC_TRANSP) {	/* Automatically pick XFER CHAR */
	    if (fcharset == FC_USASCII ||
		fcharset == FC_UKASCII ||
		fcharset == FC_DUASCII ||
		fcharset == FC_FIASCII ||
		fcharset == FC_FRASCII ||
		fcharset == FC_FCASCII ||
		fcharset == FC_GEASCII ||
		fcharset == FC_ITASCII ||
		fcharset == FC_NOASCII ||
		fcharset == FC_POASCII ||
		fcharset == FC_SPASCII ||
		fcharset == FC_SWASCII ||
		fcharset == FC_CHASCII
		)
	      tcharset = TC_1LATIN;
	    else if (fcharset == FC_1LATIN ||
		     fcharset == FC_DECMCS ||
		     fcharset == FC_NEXT   ||
		     fcharset == FC_CP437  ||
		     fcharset == FC_CP850  ||
		     fcharset == FC_APPQD  ||
		     fcharset == FC_DGMCS  ||
		     fcharset == FC_HPR8
		     )
	      tcharset = TC_1LATIN;
#ifndef NOLATIN2
	    else if (fcharset == FC_HUASCII ||
		     fcharset == FC_2LATIN  ||
		     fcharset == FC_CP852
		     )
	      tcharset = TC_2LATIN;
#endif /* NOLATIN2 */
#ifndef NOCYRIL
	    else if (fcharset == FC_CYRILL ||
		     fcharset == FC_CP866  ||
		     fcharset == FC_KOI7   ||
		     fcharset == FC_KOI8
		     )
	      tcharset = TC_CYRILL;
#endif /* NOCYRIL */
#ifndef NOKANJI
	    else if (fcharset == FC_JIS7  ||
		     fcharset == FC_SHJIS ||
		     fcharset == FC_JEUC  ||
		     fcharset == FC_JDEC
		     )
	      tcharset = TC_JEUC;
#endif /* NOKANJI */
#ifndef NOHEBREW
	    else if (fcharset == FC_HE7 ||
		     fcharset == FC_HEBREW ||
		     fcharset == FC_CP862
		     )
	      tcharset = TC_HEBREW;
#endif /* NOKANJI */
	}
#endif /* MAC */
	return(success = 1);
#endif /* NOCSETS */

      case XYFILD:			/* Display */
	return(doxdis());

      case XYFILA:			/* End-of-line */
#ifdef NLCHAR
	s = "";
	if (NLCHAR == 015)
	  s = "cr";
	else if (NLCHAR == 012)
	  s = "lf";
	if ((x = cmkey(eoftab, neoftab,
		       "local text-file line terminator",s,xxstring)) < 0)
	  return(x);
#else
	if ((x = cmkey(eoftab, neoftab,
		       "local text-file line terminator","crlf",xxstring)) < 0)
	  return(x);
#endif /* NLCHAR */
	if ((z = cmcfm()) < 0) return(z);
	feol = (CHAR) x;
	return(success = 1);

      case XYFILN:			/* Names */
	if ((x = cmkey(fntab,nfntab,"how to handle filenames","converted",
		       xxstring)) < 0)
	  return(x);
	if ((z = cmcfm()) < 0) return(z);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",1 - x);
	    sstate = setgen('S', "301", tmpbuf, "");
	    return((int) sstate);
	} else {
	    ptab[protocol].fncn = x;	/* Set structure */
	    fncnv = x;			/* Set variable */
	    f_save = x;			/* Set and set "permanent" variable */
	    return(success = 1);
	}

      case XYFILR:			/* Record length */
	sprintf(tmpbuf,"%d",DLRECL);
	if ((y = cmnum("file record length",tmpbuf,10,&z,xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",z);
	    sstate = setgen('S', "312", tmpbuf, "");
	    return((int) sstate);
	} else {
	    frecl = z;
	    return(success = 1);
	}

#ifdef COMMENT
      case XYFILO:			/* Organization */
	if ((x = cmkey(forgtab,nforg,"file organization","sequential",
		       xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",x);
	    sstate = setgen('S', "314", tmpbuf, "");
	    return((int) sstate);
	} else {
	    forg = x;
	    return(success = 1);
	}	
#endif /* COMMENT */

#ifdef COMMENT				/* Not needed */
      case XYFILF:			/* Format */
	if ((x = cmkey(frectab,nfrec,"file record format","stream",
		       xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",x);
	    sstate = setgen('S', "313", tmpbuf, "");
	    return((int) sstate);
	} else {
	    frecfm = x;
	    return(success = 1);
	}
#endif /* COMMENT */

#ifdef COMMENT
      case XYFILP:			/* Printer carriage control */
	if ((x = cmkey(fcctab,nfcc,"file carriage control","newline",
		       xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",x);
	    sstate = setgen('S', "315", tmpbuf, "");
	    return((int) sstate);
	} else {
	    fcctrl = x;
	    return(success = 1);
	}	
#endif /* COMMENT */

      case XYFILT:			/* Type */
	if ((x = cmkey(rmsflg ? rfttab  : fttab,
		       rmsflg ? nrfttyp : nfttyp,
		       "type of file transfer","text",xxstring)) < 0)
	  return(x);

#ifdef VMS
        /* Allow VMS users to choose record format for binary files */
        if ((x == XYFT_B) && (rmsflg == 0)) {
	    if ((x = cmkey(fbtab,nfbtyp,"VMS record format","fixed",
			   xxstring)) < 0)
	      return(x);
	}
#endif /* VMS */
	if ((y = cmcfm()) < 0) return(y);
	binary = x;
	b_save = x;
#ifdef MAC
	(void) mac_setfildflg(binary);
#endif /* MAC */
	if (rmsflg) {
	    char buf[4];		/* Allow for LABELED in VMS & OS/2 */
	    sprintf(buf,"%d",x);
	    sstate = setgen('S', "300", buf, "");
	    return((int) sstate);
	} else {
	    return(success = 1);
	}

      case XYFILX:			/* Collision Action */
	if ((x = cmkey(colxtab,ncolx,"Filename collision action","backup",
		       xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	fncact = x;
	ptab[protocol].fnca = x;
	if (rmsflg) {
	    sprintf(tmpbuf,"%d",fncact);
	    sstate = setgen('S', "302", tmpbuf, "");
	    return((int) sstate);
	} else {
	    if (fncact == XYFX_R) warn = 1; /* SET FILE WARNING implications */
	    if (fncact == XYFX_X) warn = 0; /* ... */
	    return(success = 1);
	}

      case XYFILW:			/* Warning/Write-Protect */
	if ((x = seton(&warn)) < 0) return(x);
	if (warn)
	  fncact = XYFX_R;
	else
	  fncact = XYFX_X;
	return(success = 1);

#ifdef CK_LABELED
      case XYFILL:			/* LABELED FILE parameters */
	if ((x = cmkey(lbltab,nlblp,"Labeled file feature","",
		       xxstring)) < 0)
	  return(x);
	if ((success = seton(&y)) < 0)
	  return(success);
	if (y)				/* Set or reset the selected bit */
	  lf_opts |= x;			/* in the options bitmask. */
	else
	  lf_opts &= ~x;
	return(success);
#endif /* CK_LABELED */

      case XYFILI:			/* INCOMPLETE */
	return(doprm(XYIFD,rmsflg));

#ifdef CK_TMPDIR
      case XYFILG: {			/* Download directory */
	  int x;
	  char *s;
#ifdef ZFNQFP
	  struct zfnfp * fnp;
#endif /* ZFNQFP */
#ifdef MAC
	  char temp[34];
#endif /* MAC */

#ifdef GEMDOS
	  if ((x = cmdir("Name of local directory, or carriage return",
			 "",&s,
			 NULL)) < 0 ) {
	      if (x != -3)
		return(x);
	  }
#else
#ifdef OS2
	  if ((x = cmdir("Name of PC disk and/or directory,\n\
       or press the Enter key to use current directory",
			 "",&s,xxstring)) < 0 ) {
	      if (x != -3)
		return(x);
	  }
#else
#ifdef MAC
	  strncpy(temp,homdir,32);
	  x = strlen(temp);
	  if (x > 0) if (temp[x-1] != ':') { temp[x] = ':'; temp[x+1] = NUL; }
	  if ((x = cmtxt("Name of Macintosh volume and/or folder,\n\
 or press the Return key for the desktop on the boot disk",
			 temp,&s, xxstring)) < 0 )
	    return(x);
#else
	  if ((x = cmdir("Name of local directory, or carriage return",
			 "", &s, xxstring)) < 0 ) {
	      if (x != -3)
		return(x);
	  }
#endif /* MAC */
#endif /* OS2 */
#endif /* GEMDOS */
	  debug(F110,"download dir",s,0);

#ifndef MAC
	  if (x == 2) {
	      printf("?Wildcards not allowed in directory name\n");
	      return(-9);
	  }
#endif /* MAC */

#ifdef ZFNQFP
	  if (fnp = zfnqfp(s,TMPBUFSIZ - 1,tmpbuf)) {
	      if (fnp->fpath)
		if ((int) strlen(fnp->fpath) > 0)
		  s = fnp->fpath;
	  }
	  debug(F110,"download zfnqfp",s,0);
#endif /* ZFNQFP */

	  strcpy(line,s);		/* Make a safe copy */
	  s = line;
#ifndef MAC
	  if ((x = cmcfm()) < 0)	/* Get confirmation */
	    return(x);
#endif /* MAC */

	  x = strlen(line);

#ifdef datageneral
	  if (line[x-1] == ':')		/* homdir ends in colon, */
	    line[x-1] = NUL;		/* and "dir" doesn't like that... */
#endif /* datageneral */
	 
	  if (dldir)
	    free(dldir);
	  dldir = NULL;

	  if (x && (dldir = malloc(x + 1)))
	    strcpy(dldir, line);

	  return(success = 1);
      }
#endif /* CK_TMPDIR */
      case XYFILY:
	return(setdest());

      default:
	printf("?unexpected file parameter\n");
	return(-2);
    }
}

#ifdef OS2
/* MS-DOS KERMIT compatibility modes */
int
setmsk() {
    if ((y = cmkey(msktab,nmsk,"MS-DOS Kermit compatibility mode", 
                    "keycodes",xxstring)) < 0) return(y);

    switch ( y ) {
#ifdef COMMENT
      case MSK_COLOR:
        return(seton(&mskcolors));
#endif /* COMMENT */
      case MSK_KEYS:
	return(seton(&mskkeys));
      default:				/* Shouldn't get here. */
        return(-2);
    }
}
#endif

#ifndef NOLOCAL
int
settrm() {
    int i = 0;
#ifdef OS2
    extern int colorreset, erasemode;
#endif /* OS2 */

    if ((y = cmkey(trmtab,ntrm,"", "",xxstring)) < 0) return(y);
#ifdef MAC
    printf("\n?Sorry, not implemented yet.  Please use the Settings menu.\n");
    return(-9);
#else
    switch (y) {
      case XYTBYT:			/* SET TERMINAL BYTESIZE */
	if ((y = cmnum("bytesize for terminal connection","8",10,&x,
		       xxstring)) < 0)
	  return(y);
	if (x != 7 && x != 8) {
	    printf("\n?The choices are 7 and 8\n");
	    return(success = 0);
	}
	if ((y = cmcfm()) < 0) return(y);
	if (x == 7) cmask = 0177;
	else if (x == 8) {
	    cmask = 0377;
	    parity = 0;
	}
        return(success = 1);

      case XYTSO:			/* SET TERMINAL LOCKING-SHIFT */
	return(seton(&sosi));

      case XYTNL:			/* SET TERMINAL NEWLINE-MODE */
	return(seton(&tnlm)); 

#ifdef OS2
      case XYTCOL:
	if ((x = cmkey(ttycoltab,ncolors,"","terminal",xxstring)) < 0) 
	  return(x);
	else if (x == TTCOLRES) {
	    if ((y = cmkey(ttcolmodetab,ncolmode,
			   "","default-color",xxstring)) < 0)
	      return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    colorreset = y;
	    return(success = 1);
	} else if (x == TTCOLERA) {
	    if ((y = cmkey(ttcolmodetab,ncolmode,"",
			   "current-color",xxstring)) < 0)
	      return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    erasemode = y;
	    return(success=1);
	} else {			/* No parse error */
	    int fg = 0, bg = 0;
	    fg = cmkey(ttyclrtab, nclrs,
		       (x == TTCOLBOR ?
			"color for screen border" :
			"foreground color and then background color"),
		       "lgray", xxstring);
	    if (fg < 0)
	      return(fg);
	    if (x != TTCOLBOR) {
		if ((bg = cmkey(ttyclrtab,nclrs,
				"background color","blue",xxstring)) < 0)
		  return(bg);
	    }
	    if ((y = cmcfm()) < 0)
	      return(y);
	    switch (x) {
	      case TTCOLNOR:
		colornormal = fg | bg << 4;
		fgi = fg & 0x08;
		bgi = bg & 0x08;
		break;
	      case TTCOLREV:
		colorreverse = fg | bg << 4;
		break;
	      case TTCOLUND:   
		colorunderline = fg | bg << 4;
		break;
	      case TTCOLGRP:
		colorgraphic = fg | bg << 4;
		break;
	      case TTCOLDEB:
		colordebug = fg | bg << 4;
		break;
	      case TTCOLSTA:
		colorstatus = fg | bg << 4;
		break;
	      case TTCOLHLP:
		colorhelp = fg | bg << 4;
		break;
	      case TTCOLBOR:
		colorborder = fg;
		break;
	      case TTCOLSEL:
		colorselect = fg | bg << 4;
		break;
	      default:
		printf("%s - invalid\n",cmdbuf);
		return(-9);
		break;
	    }
	    scrninitialized[VTERM] = 0;
	    VscrnInit(VTERM);
	}
	return(success = 1);

      case XYTCUR:			/* SET TERMINAL CURSOR */
	if ((x = cmkey(ttycurtab,ncursors,"","underline",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
        tt_cursor = tt_cursor_usr = x;
	return(success = 1);
#endif /* OS2 */

      case XYTTYP:			/* SET TERMINAL TYPE */
#ifdef OS2
	if ((x = cmkey(ttyptab,nttyp,"","vt320",xxstring)) < 0) 
	  return(x);
	if ((y = cmcfm()) < 0) 
	  return(y);
	settermtype(x,1);
	return(success=1);
#else  /* Not OS2 */
	printf(
"\n Sorry, this version of C-Kermit does not support the SET TERMINAL TYPE\n");
	printf(
" command.  Type \"help set terminal\" for further information.\n");
#endif /* OS2 */
	return(success = 0);

#ifdef OS2
      case XYTARR:			/* SET TERMINAL ARROW-KEYS */
	if ((x = cmkey(akmtab,2,"","",xxstring)) < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
	tt_arrow = x;			/* TTK_NORM / TTK_APPL; see ckuusr.h */
	return(success = 1);

      case XYTKPD:			/* SET TERMINAL KEYPAD-MODE */
	if ((x = cmkey(kpmtab,2,"","",xxstring)) < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
	tt_keypad = x;			/* TTK_NORM / TTK_APPL; see ckuusr.h */
	return(success = 1);

      case XYTWRP:			/* SET TERMINAL WRAP */
	return(seton(&tt_wrap)); 

      case XYSCRS:
	if ((y = cmnum("CONNECT scrollback buffer size, lines","2000",10,&x,
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
#ifndef VSCRNINIT
	if ( (ULONG) x < VscrnGetBufferSize(VTERM) ) {
	    printf("\nWarning: the scrollback buffer will be emptied on the");
	    printf(" next CONNECT,\n");
	    printf("unless the buffer is restored to %d lines.\n",
		   VscrnGetBufferSize(VTERM));
        }
#endif /* VSCRNINIT */
	tt_scrsize[VTERM] = x;
#ifdef VSCRNINIT
	VscrnInit(VTERM);
#endif /* VSCRNINIT */
	return(success = 1);
#endif /* OS2 */

#ifndef NOCSETS
#ifndef KUI
      case XYTCS:			/* SET TERMINAL CHARACTER-SET */
	  /* set terminal character-set <remote> <local> */
	if ((x = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       ttcstab,ntermc,
#endif /* CKOUNI */
		       "remote terminal character-set","",xxstring)) < 0) 
	  return(x);
	if (x == FC_TRANSP) {		/* TRANSPARENT? */
	    if ((x = cmcfm()) < 0) return(x); /* Confirm the command */
#ifdef CKOUNI
	    tcsr = tcsl = TX_ASCII;	/* Make them both the same */
#else /* CKOUNI */
	    tcsr = tcsl = FC_USASCII;
#endif /* CKOUNI */
#ifdef OS2
	    y = os2getcp();		/* Default is current code page */
	    switch (y) {
#ifdef CKOUNI
	    case 437: tcsr = tcsl = TX_CP437; break;
	    case 850: tcsr = tcsl = TX_CP850; break;
	    case 852: tcsr = tcsl = TX_CP852; break;
	    case 857: tcsr = tcsl = TX_CP857; break;
	    case 862: tcsr = tcsl = TX_CP862; break;
	    case 866: tcsr = tcsl = TX_CP866; break;
	    case 869: tcsr = tcsl = TX_CP869; break;
#else /* CKOUNI */
	    case 437: tcsr = tcsl = FC_CP437; break;
	    case 850: tcsr = tcsl = FC_CP850; break;
	    case 852: tcsr = tcsl = FC_CP852; break;
	    case 862: tcsr = tcsl = FC_CP862; break;
	    case 866: tcsr = tcsl = FC_CP866; break;
#endif /* CKOUNI */
	    }
	    for (i = 0; i < 4; i++) {
#ifdef CKOUNI
		G[i].def_designation = G[i].designation = TX_TRANSP;
#else /* CKOUNI */
		G[i].def_designation = G[i].designation = FC_TRANSP;
#endif /* CKOUNI */
		G[i].init = FALSE;
		G[i].size = G[i].def_size = cs96;
		G[i].c1 = G[i].def_c1 = FALSE;
		G[i].national = FALSE;
		G[i].rtoi = NULL;
		G[i].itol = NULL;
		G[i].ltoi = NULL;
		G[i].itor = NULL;
            }
#endif /* OS2 */
	    return(success = 1);
	}

/* Not transparent, so get local set to translate it into */

	s = "";
#ifdef OS2
 	y = os2getcp();			/* Default is current code page */
	switch (y) {
	   case 437: s = "cp437"; break;
	   case 850: s = "cp850"; break;
	   case 852: s = "cp852"; break;
	   case 862: s = "cp862"; break;
	   case 866: s = "cp866"; break;
	 }
#ifdef OS2ONLY
/*
   If the user has loaded a font with SET TERMINAL FONT then we want
   to change the default code page to the font that was loaded.
*/
	if (tt_font != TTF_ROM) {
	    for (y = 0; y < ntermfont; y++ ) {
		if (termfont[y].kwval == tt_font) {
		    s = termfont[y].kwd;
		    break;
		}
	    }
	}
#endif /* OS2ONLY */
#else
					/* Make current file char set */
	for (y = 0; y <= nfilc; y++)	/* be the default... */
	  if (fcstab[y].kwval == fcharset) {
	      s = fcstab[y].kwd;
	      break;
	  }
#endif /* OS2 */

	if ((y = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       fcstab,nfilc,
#endif /* CKOUNI */
		       "local character-set",s,xxstring)) < 0)
	  return(y);

#ifdef OS2
	if ((z = cmkey(graphsettab,ngraphset,
		       "DEC VT intermediate graphic set","all",xxstring)) < 0)
	  return(z);
	{
	    int eol;
	    if ((eol = cmcfm()) < 0)
	      return(eol); /* Confirm the command */
	}
	tcsr = x;			/* Remote character set */
	tcsl = y;			/* Local character set */
	if (z == TT_GR_ALL) {
	    int i;
	    for (i = 0; i < 4; i++) {
		G[i].def_designation = G[i].designation = x;
		G[i].init = TRUE;
		switch (cs_size(x)) {	/* 94, 96, or 128 */
		  case 128:
		  case 96:
		    G[i].size = G[i].def_size = cs96;
		    break;
		  case 94:
		    G[i].size = G[i].def_size = cs94;
		    break;
		  default:
		    G[i].size = G[i].def_size = csmb;
		    break;
		}
		G[i].c1 = G[i].def_c1 = x != tcsl && cs_is_std(x);
		G[i].national = cs_is_nrc(x);
            }
	    if (!cs_is_nrc(x)) {
		G[0].designation = G[0].def_designation = FC_USASCII;
		G[0].size = G[0].def_size = cs94;
            }
        } else {			/* Specific Gn */
	    G[z].def_designation = G[z].designation = x;
	    G[z].init = TRUE;
	    switch (cs_size(x)) {	/* 94, 96, or 128 */
	      case 128:
	      case 96:
		G[i].size = G[i].def_size = cs96;
		break;
	      case 94:
		G[i].size = G[i].def_size = cs94;
		break;
	      default:
		G[i].size = G[i].def_size = csmb;
		break;
	    }
	    G[i].c1 = G[i].def_c1 = x != tcsl && cs_is_std(x);
	    G[i].national = cs_is_nrc(x);
        }
#else /* not OS2 */
	if ((z = cmcfm()) < 0) return(z); /* Confirm the command */
	tcsr = x;			/* Remote character set */
	tcsl = y;			/* Local character set */
#endif /* OS2 */
	return(success = 1);
#endif /* CKOUNI */
#endif /* NOCSETS */

#ifndef NOCSETS
      case XYTLCS:			/* SET TERMINAL LOCAL-CHARACTER-SET */
	/* set terminal character-set <local> */
	s = "";
#ifdef OS2
 	y = os2getcp();			/* Default is current code page */
	switch (y) {
	   case 437: s = "cp437"; break;
	   case 850: s = "cp850"; break;
	   case 852: s = "cp852"; break;
	   case 862: s = "cp862"; break;
	   case 866: s = "cp866"; break;
	 }
#ifdef OS2ONLY
/*
   If the user has loaded a font with SET TERMINAL FONT then we want
   to change the default code page to the font that was loaded.
*/
	if (tt_font != TTF_ROM) {
	    for (y = 0; y < ntermfont; y++ ) {
		if (termfont[y].kwval == tt_font) {
		    s = termfont[y].kwd;
		    break;
		}
	    }
	}
#endif /* OS2ONLY */
#else /* OS2 */
					/* Make current file char set */
	for (y = 0; y <= nfilc; y++)	/* be the default... */
	  if (fcstab[y].kwval == fcharset) {
	      s = fcstab[y].kwd;
	      break;
	  }
#endif /* OS2 */
	if ((y = cmkey(
#ifdef CKOUNI
                       txrtab,ntxrtab,
#else /* CKOUNI */
                       fcstab,nfilc,
#endif /* CKOUNI */
		       "local character-set",s,xxstring)) < 0)
	  return(y);

#ifdef OS2
        if ((z = cmcfm()) < 0) return(z); /* Confirm the command */
	tcsl = y;			/* Local character set */
	{
	    int i;
	    for (i = 0; i < 4; i++) {
		G[i].init = TRUE;
		x = G[i].designation;
		G[i].c1 = (x != tcsl) && cs_is_std(x);
		x = G[i].def_designation;
		G[i].def_c1 = (x != tcsl) && cs_is_std(x);
            }
        }
#else /* not OS2 */
	if ((z = cmcfm()) < 0) return(z); /* Confirm the command */
	tcsl = y;			/* Local character set */
#endif /* OS2 */
	return(success = 1);
#endif /* NOCSETS */

#ifndef NOCSETS
#ifdef CKOUNI
      case XYTUNI: /* SET TERMINAL UNICODE */
	return(success = seton(&tt_unicode));
#endif /* CKOUNI */
      case XYTRCS:			/* SET TERMINAL REMOTE-CHARACTER-SET */
	/* set terminal character-set <remote> <Graphic-set> */
	if ((x = cmkey(
#ifdef CKOUNI
                txrtab, ntxrtab,
#else /* CKOUNI */
                ttcstab,ntermc,
#endif /* CKOUNI */
		       "remote terminal character-set","",xxstring)) < 0) 
	  return(x);
#ifndef KUI
	/* KUI can't have a Transparent Character Set */
#ifdef CKOUNI
	if (x == TX_TRANSP)
#else /* CKOUNI */
	if (x == FC_TRANSP)
#endif /* CKOUNI */
	  {				/* TRANSPARENT? */
	      if ((x = cmcfm()) < 0)	/* Confirm the command */
		return(x);
	      tcsr = tcsl;		/* Make both sets the same */
#ifdef OS2
#ifdef CKOUNI
	      if (!cs_is_nrc(tcsl)) {
		  G[0].def_designation = G[i].designation = TX_ASCII;
		  G[0].init = TRUE;
		  G[0].def_c1 = G[i].c1 = FALSE;
		  G[0].size = cs94;
		  G[0].national = FALSE;
	      }
	      for (i = cs_is_nrc(tcsl) ? 0 : 1; i < 4; i++) {
		  G[i].def_designation = G[i].designation = tcsl;
		  G[i].init = TRUE;
		  G[i].def_c1 = G[i].c1 = FALSE;
		  switch (cs_size(G[i].designation)) { /* 94, 96, or 128 */
		    case 128:
		    case 96:
		      G[i].size = G[i].def_size = cs96;
		      break;
		    case 94:
		      G[i].size = G[i].def_size = cs94;
		      break;
		    default:
		      G[i].size = G[i].def_size = csmb;
		      break;
		  }
		  G[i].national = cs_is_nrc(x);
	      }
#else /* CKOUNI */ 
	      for (i = 0; i < 4; i++) {
		  G[i].def_designation = G[i].designation = FC_TRANSP;
		  G[i].init = FALSE;
		  G[i].size = G[i].def_size = cs96;
		  G[i].c1 = G[i].def_c1 = FALSE;
		  G[i].rtoi = NULL;
		  G[i].itol = NULL;
		  G[i].ltoi = NULL;
		  G[i].itor = NULL;
		  G[i].national = FALSE;
	      }
#endif /* CKOUNI */
#endif /* OS2 */
	      return(success = 1);
	  }
#endif /* KUI */
#ifdef OS2
	if ((z = cmkey(graphsettab,ngraphset,
		       "DEC VT intermediate graphic set","all",xxstring)) < 0)
	  return(z);
	{
	    int eol;
	    if ((eol = cmcfm()) < 0)	/* Confirm the command */
	      return(eol);
	}
	tcsr = x;			/* Remote character set */
	if (z == TT_GR_ALL) {
	    int i;
	    for (i = 0; i < 4; i++) {
		G[i].def_designation = G[i].designation = x;
		G[i].init = TRUE;
		switch (cs_size(x)) {	/* 94, 96, or 128 */
		  case 128:
		  case 96:
		    G[i].size = G[i].def_size = cs96;
		    break;
		  case 94:
		    G[i].size = G[i].def_size = cs94;
		    break;
		  default:
		    G[i].size = G[i].def_size = csmb;
		    break;
		}
		G[i].c1 = G[i].def_c1 = x != tcsl && cs_is_std(x);
		G[i].national = cs_is_nrc(x);
            }
	    if (!cs_is_nrc(x)) {
		G[0].designation = G[0].def_designation = FC_USASCII;
		G[0].size = G[0].def_size = cs94;
            }
        } else {			/* Specific Gn */
	    G[z].def_designation = G[z].designation = x;
	    G[z].init = TRUE;
	    switch (cs_size(x)) {	/* 94, 96, or 128 */
	      case 128:
	      case 96:
		G[i].size = G[i].def_size = cs96;
		break;
	      case 94:
		G[i].size = G[i].def_size = cs94;
		break;
	      default:
		G[i].size = G[i].def_size = csmb;
		break;
	    }
	    G[z].c1 = G[z].def_c1 = x != tcsl && cs_is_std(x);
	    G[z].national = cs_is_nrc(x);
        }
#else /* not OS2 */
	if ((z = cmcfm()) < 0) return(z); /* Confirm the command */
	tcsr = x;			/* Remote character set */
#endif /* OS2 */
	return(success = 1);
#endif /* NOCSETS */

      case XYTEC:			/* SET TERMINAL ECHO */
	if ((x = cmkey(rltab,nrlt,"which side echos during CONNECT",
		       "remote", xxstring)) < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
	duplex = x;
	return(success = 1); 

      case XYTESC:			/* SET TERM ESC */
	if ((x = cmkey(nabltab,2,"","enabled",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	tt_escape = x;
	return(1);

      case XYTCRD:			/* SET TERMINAL CR-DISPLAY */
	if ((x = cmkey(crdtab,2,"", "normal", xxstring)) < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
	tt_crd = x;
	return(success = 1); 

#ifdef OS2
      case XYTANS: {			/* SET TERMINAL ANSWERBACK */
/*
  NOTE: We let them enable and disable the answerback sequence, but we
  do NOT let them change it, and we definitely do not let the host set it.
  This is a security feature.

  As of 1.1.8 we allow the SET TERM ANSWERBACK MESSAGE <string> to be 
  used just as MS-DOS Kermit does.  C0 and C1 controls as well as DEL
  are not allowed to be used as characters.  They are translated to
  underscore.  This may not be set by APC.
*/
	  if ((x = cmkey(anbktab,nansbk,"", "off", xxstring)) < 0) 
	    return(x);
	  if (x < 2) {
	      if ((y = cmcfm()) < 0) 
		return(y);
	      tt_answer = x;
	      return(success = 1); 
	  } else if ( x == 2 || x == 3) {
	      int len = 0;
	      extern int safeanswerbk;
	      extern char useranswerbk[];
	      if ((y = cmtxt("Answerback extension","",&s,xxstring)) < 0)
		return(y);
	      if (apcactive == APC_LOCAL ||
		  (apcactive == APC_REMOTE && apcstatus != APC_UNCH))
		return(success = 0);
	      len = strlen(s);
	      if (x == 2) {
		  /* Safe Answerback's don't have C0/C1 chars */
		  for (z = 0; z < len; z++) {
		      if ((s[z] & 0x7F) <= SP || (s[z] & 0x7F) == DEL)
			useranswerbk[z] = '_';
		      else
			useranswerbk[z] = s[z];
		  }
		  useranswerbk[z] = '\0';
		  safeanswerbk = 1 ;	/* TRUE */
	      } else {
		  strncpy( useranswerbk, s, 65 );
		  useranswerbk[65] = '\0';
		  safeanswerbk = 0;	/* FALSE */
	      }
	      updanswerbk();
	      return(success = 1);
	  } else
	    return(success = 0); 
      }
#endif /* OS2 */

#ifdef CK_APC
      case XYTAPC:
	if ((y = cmkey(apctab,3,"application program command execution","",
		       xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0)
	  return(x);
	if (apcactive == APC_LOCAL ||
	    (apcactive == APC_REMOTE && apcstatus != APC_UNCH))
	  return(success = 0);
	apcstatus = y;	
	return(success = 1);

#ifdef CK_AUTODL
  case XYTAUTODL:
    return(success = seton(&autodl));	/* AUTODOWNLOAD ON, OFF */    
#endif /* CK_AUTODL */
#endif /* CK_APC */

#ifdef OS2
      case XYTBEL:
        return(success = setbell());
#endif /* OS2 */

      case XYTDEB:			/* TERMINAL DEBUG */
	x = debses;			/* What it was before */
	y = seton(&debses);		/* Go parse ON or OFF */
#ifdef OS2
	if (y > 0)			/* Command succeeded? */
	  if ((x != 0) && (debses == 0)) /* It was on and we turned it off? */
	    os2debugoff();		/* Fix OS/2 coloration */
#endif /* OS2 */
	return(y);

#ifdef OS2
      case XYTROL:			/* SET TERMINAL ROLL */
        if ((y = cmkey(rolltab,nroll,"scrollback mode","insert",xxstring))<0)
            return(y);
        if ((x = cmcfm()) < 0) return(x);
        tt_roll[VTERM] = y;
	return(success = 1);

      case XYTCTS:			/* SET TERMINAL TRANSMIT-TIMEOUT */
	y = cmnum("Maximum seconds to allow CTS off during CONNECT",
		  "5",10,&x,xxstring);
	return(setnum(&tt_ctstmo,x,y,10000));

      case XYTCPG: {			/* SET TERMINAL CODE-PAGE */
        int i;
	int cp = -1;
	y = cmnum("PC code page to use during terminal emulation",
		  "850",10,&x,xxstring);
	if ((x = setnum(&cp,x,y,2000)) < 0) return(x);
	if (os2setcp(cp) != 1) {
#ifdef NT
	    if (isWin95())
	      printf(
		 "Sorry, Windows 95 does not support code page switching\n");
	    else
#endif /* NT */
	      printf(
		 "Sorry, %d is not a valid code page for this system.\n",cp);
	    return(-9);
	}
    /* Force the terminal character-sets conversions to be updated */
	for ( i = 0; i < 4; i++ )
	  G[i].init = TRUE;
	return(1);
    }

#ifdef  COMMENT
      case XYTHCU:			/* SET TERMINAL HIDE-CURSOR */
	return(seton(&tt_hide));
#endif /* COMMENT */

      case XYTPAC:			/* SET TERMINAL OUTPUT-PACING */
	y = cmnum(
	   "Pause between sending each character during CONNECT, milliseconds",
		  "-1",10,&x,xxstring);
	return(setnum(&tt_pacing,x,y,10000));

#ifdef OS2MOUSE
      case XYTMOU: {			/* SET TERMINAL MOUSE */
	  int old_mou = tt_mouse;
	  seton(&tt_mouse);
	  if ( tt_mouse != old_mou )
	    if ( tt_mouse )
	      os2_mouseon();
	    else
	      os2_mouseoff();
	  return(1);
      }
#endif /* OS2MOUSE */
#endif /* OS2 */

      case XYTWID: {
	  if ((y = cmnum(
#ifdef OS2
			 "number of columns in display window during CONNECT",
#else
			 "number of columns on your screen",
#endif /* OS2 */
			 "80",10,&x,xxstring)) < 0)
            return(y);
          if ((y = cmcfm()) < 0) return(y);
#ifdef OS2
	  if (x % 2) {
	      printf("\n?The width must be an even value\n.");
	      return(success = 0);
	  }

	  if ( IsOS2FullScreen() ) {
	      if ( x != 40 && x != 80 && x != 132 ) {
		  printf("\n?The width must be 40, 80,");
#ifdef NT
		  printf(" or 132 under Windows 95.\n.");
#else /* NT */
		  printf(" or 132 in a Full Screen session.\n.");
#endif /* NT */
		  return(success = 0);
	      }
	  } else {
	      if ( !IsWARPed() && x != 80 ) {
		  printf("\n?OS/2 version is pre-WARP: the width must equal ");
		  printf("80 in a Windowed Session\n.");
		  return(success = 0);
                }
	      if (x < 20 || x > MAXTERMCOL ) {
		  printf(
		    "\n?The width must be between 20 and %d\n.",MAXTERMCOL);
		  return(success = 0);
	      }
	  }
	  if (x > 8192/(tt_rows[VTERM]+1)) {
	      printf(
"\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
		     tt_rows[VTERM]+1,x,x*(tt_rows[VTERM]+1));
	      return(success = 0);
	  }
	  tt_cols[VTERM] = x;
	  tt_cols_usr = x;
#ifdef VSCRNINIT
	  VscrnSetWidth( VTERM, x);
#ifdef TCPSOCKET
	  if (nawsflg) {
	      tn_snaws();
#ifdef RLOGCODE
	      rlog_naws();
#endif /* RLOGCODE */
	  }
#endif /* TCPSOCKET */
#else /* VSCRNINIT */
	  VscrnSetWidth( VTERM, -1);
#endif /* VSCRNINIT */
/*
   We do not set tt_szchng here because that would result in the screen buffer
   being reallocated and the screen cleared.  But that is not necessary when 
   only the screen width is being changed since the buffer allocates the full
   width
*/ 
#else  /* Not OS/2 */
	  tt_cols = x;
#endif /* OS2 */
	  return(success = 1);
      }

      case XYTHIG: 
   	if ((y = cmnum(
#ifdef OS2
"number of rows in display window during CONNECT, not including status line",
#else
"number of rows on your screen",
#endif /* OS2 */
		       "24",10,&x,xxstring)) < 0)
	  return(y);
	if ((y = cmcfm()) < 0) return(y);

#ifdef OS2
        if (IsOS2FullScreen()) {
            if (tt_status && x != 24 && x != 42 && x != 49 && x != 59) {
                printf("\n?The height must be 24, 42, 49");
#ifdef NT
                printf(" or 59 under Windows 95.\n.");
#else /* NT */
                printf(" or 59 in a Full Screen session.\n.");
#endif /* NT */
                return(success = 0);
	    } else if (!tt_status &&
		       x != 25 &&
		       x != 43 &&
		       x != 50 &&
		       x != 60) {
                printf("\n?The height must be 25, 43, 50");
#ifdef NT
                printf(" or 60 under Windows 95.\n.");
#else /* NT */
                printf(" or 60 in a Full Screen session.\n.");
#endif /* NT */
                return(success = 0);
	    }
	} else {
            if (x < 8 || x > MAXTERMROW ) {
                printf("\n?The height must be between 8 and %d\n.",MAXTERMROW);
                return(success = 0);
	    }
	}
        if (x > 8192/tt_cols[VTERM]) {
            printf(
"\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
		   x,tt_cols[VTERM],x*tt_cols[VTERM]);
            return(success = 0);
	}
#ifdef VSCRNINIT 
        tt_szchng[VTERM] = 1;
        tt_rows[VTERM] = x;
        VscrnInit( VTERM );		/* Height set here */
#ifdef TCPSOCKET
        if (nawsflg){
            tn_snaws();
#ifdef RLOGCODE
            rlog_naws();
#endif /* RLOGCODE */
        }
#endif /* TCPSOCKET */
#else /* VSCRNINIT */
        tt_rows[VTERM] = x;
        VscrnSetHeight( VTERM, -1 );
        tt_szchng[VTERM] = 1;
#endif /* VSCRNINIT */
#else  /* Not OS/2 */
	tt_rows = x;
#endif /* OS2 */
        return(success = 1);

#ifdef OS2
    case XYTUPD: { 
	int mode, delay;
	if ((mode = cmkey(scrnupd,nscrnupd,"","fast",xxstring)) < 0) {
	    return(mode);
        } else {
	    y = cmnum(
	    "Pause between FAST screen updates in CONNECT mode, milliseconds",
		      "100",10,&x,xxstring
		      );
	    if (x < 0 || x > 1000 ) {
		printf(
	    "\n?The update rate must be between 0 and 1000 milliseconds.\n"
		       );
		return(success = 0);
            }
            if ((y = cmcfm()) < 0) return(y);

	    updmode = tt_updmode = mode;
	    return(setnum(&tt_update,x,y,10000));
	}
    }
    case XYTCTRL:
	  if ((x = cmkey(termctrl,ntermctrl,"","7",xxstring)) < 0) {
	      return(x);
	  } else {
	      if ((y = cmcfm()) < 0) 
		  return(y);
	      switch ( x ) {
	      case 8:
		  send_c1 = send_c1_usr = TRUE;
		  break;
	      case 7:
	      default:
		  send_c1 = send_c1_usr = FALSE;
		  break;
	      }
	  }	   
	  return(success = TRUE);
	  break;

#ifdef PCFONTS
      case XYTFON: 
	if ( !IsOS2FullScreen() ) {
	    printf(
        "\n?SET TERMINAL FONT is only supported in Full Screen sessions.\n");
	    return(success = FALSE);
        }

	if ((x = cmkey(termfont,ntermfont,"","default",xxstring)) < 0) {
            return(x);
        } else {
	    if ((y = cmcfm()) < 0) return(y);
            if ( !os2LoadPCFonts() ) {
                tt_font = x;
		return(success = TRUE);
	    } else {
		printf(
      "\n?PCFONTS.DLL is not available in CKERMIT executable directory.\n");
                return(success = FALSE);
	    }
	}
	break;
#endif /* PCFONTS */
      case XYTVCH:
	if ((x = cmkey(nabltab,2,"","enabled",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	tt_modechg = x;
	return(1);

      case XYTSTAT: {
          extern int marginbot;
          if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
          if ((x = cmcfm()) < 0) return(x);
          if (y != tt_status || y != tt_status_usr) {
              /* Might need to fixup the margins */
              if ( marginbot == VscrnGetHeight(VTERM)-(tt_status?1:0) )
                if (y) {
                    marginbot--;
                } else {
                    marginbot++;
                }
              tt_status_usr = tt_status = y;
              if (y) {
#ifdef VSCRNINIT 
                    tt_szchng[VTERM] = 2;
                    tt_rows[VTERM]--;
                    VscrnInit(VTERM);  /* Height set here */
#ifdef TCPSOCKET
                    if (nawsflg) {
                        tn_snaws();
#ifdef RLOGCODE
                        rlog_naws();
#endif /* RLOGCODE */
                    }
#endif /* TCPSOCKET */
#else /* VSCRNINIT */
                    tt_rows[VTERM]--;
                    VscrnSetHeight(VTERM, -1);
                    tt_szchng[VTERM] = 1;
#endif /* VSCRNINIT */
              } else {
#ifdef VSCRNINIT 
                    tt_szchng[VTERM] = 1;
                    tt_rows[VTERM]++;
                    VscrnInit(VTERM);  /* Height set here */
#ifdef TCPSOCKET
                    if (nawsflg){
                        tn_snaws();
#ifdef RLOGCODE
                        rlog_naws();
#endif /* RLOGCODE */
                    }
#endif /* TCPSOCKET */
#else /* VSCRNINIT */
                    tt_rows[VTERM]++;
                    VscrnSetHeight(VTERM, -1);
                    tt_szchng[VTERM] = 1;
#endif /* VSCRNINIT */
              }
          }
          return(1);
      }
#endif /* OS2 */

#ifdef NT	
      case XYTATTBUG:
	if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	tt_attr_bug = y;
	return(1);
#endif /* NT */

#ifdef OS2
      case XYTSGRC:
	if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	sgrcolors = y;
	return(1);

      case XYTSEND:
	  if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	  if ((x = cmcfm()) < 0) return(x);
	  tt_senddata = y;
	  return(1);

      case XYTSEOB:
	  if ((y = cmkey(ttyseobtab,2,"","us_cr",xxstring)) < 0) return(y);
	  if ((x = cmcfm()) < 0) return(x);
	  wy_blockend = y;
	  return(1);

      case XYTATTR:
	if ((x = cmkey(ttyattrtab,nattrib,"","underline",xxstring)) < 0) 
	  return(x);
	switch ( x ) {
	  case TTATTBLI:
	    if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	    if ((x = cmcfm()) < 0) return(x);
	    trueblink = y;
#ifndef KUI
	    if ( !trueblink && trueunderline ) {
		trueunderline = 0;
		printf("\nWarning: Underline being simulated by color.");
	    }

#endif /* KUI */
	    break;
	  case TTATTREV:
	    if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	    if ((x = cmcfm()) < 0) return(x);
	    truereverse = y;
	    break;
	  case TTATTUND:
	    if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	    if ((x = cmcfm()) < 0) return(x);
	    trueunderline = y;
#ifndef KUI
	    if ( !trueblink && trueunderline ) {
		trueblink = 1;
		printf("\nWarning: True blink mode is active.");
	    }
#endif /* KUI */
	    break;
	}
	return(1);

      case XYTKEY: {			/* SET TERMINAL KEY */
	  int t, x, y;
	  int clear = 0, deflt = 0;
	  int flag = 0;
	  int kc;			/* Key code */
	  char *s = NULL;		/* Key binding */
#ifndef NOKVERBS
	  char *p = NULL;		/* Worker */
#endif /* NOKVERBS */
	  con_event defevt;
	  extern int os2gks;
	  extern int mskkeys;
	  extern int initvik;

	  if ((t = cmkey(ttkeytab,nttkey,"","",xxstring)) < 0) 
	    return(t);
	  x_ifnum = 1;
	  y = cmnum("numeric key code, or the words CLEAR or DEFAULT,",
		   "",10,&kc,xxstring);
	  x_ifnum = 0;
	  if (y < 0) {
	      debug(F111,"SET KEY",atmbuf,y);
	      if (y == -2) {		/* Not a valid number */
		  /* Check for SET TERM KEY <term> CLEAR */
		  if ((y = strlen(atmbuf)) < 0)
		    return(-2);
		  if (!xxstrcmp(atmbuf,"clear",y))
		    clear = 1;
		  else if (!xxstrcmp(atmbuf,"default",y))
		    deflt = 1;
		  else
		    return(-2);
		  if ((x = cmcfm()) < 0)
		    return(x);
		  if (clear)
		    clearkeymap(t);
		  if (deflt)
		    defaultkeymap(t);
		  initvik = 1;		/* Update the VIK table */
		  return(1);
	      } else if (y == -3) {	/* SET TERM KEY <terminal> <Return> */
		  /* Prompt for a keystroke */
		  printf(" Press key to be defined: ");
		  conbin((char)escape);	/* Put terminal in binary mode */
		  os2gks = 0;		/* Turn off Kverb preprocessing */
		  kc = congks(0);	/* Get character or scan code */
		  os2gks = 1;		/* Turn on Kverb preprocessing */
		  concb((char)escape);	/* Restore terminal to cbreak mode */
		  if (kc < 0) {		/* Check for error */
		      printf("?Error reading key\n");
		      return(0);
		  }
		  shokeycode(kc);	/* Show current definition */
		  flag = 1;		/* Remember it's a multiline command */
	    } else			/* Error */
	      return(y);
	  }

    /* Normal SET TERMINAL KEY <terminal> <scancode> <value> command... */

	  if (mskkeys)
	    kc = msktock(kc);

	  if (kc < 0 || kc >= KMSIZE) {
	      printf("?key code must be between 0 and %d\n", KMSIZE - 1);
	      return(-9);
	  }
	  if (kc == escape) {
	      printf("Sorry, %d is the CONNECT-mode escape character\n",kc);
	      return(-9);
	  }
	  wideresult = -1;
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
	  p = s;			/* Save this place */
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
		  ) {			/* Have K */
  
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
	  line[y++] = NUL;		/* Terminate */
	  s = line + y + 1;		/* Point to after it */
	  x = LINBUFSIZ - (int) strlen(line) - 1; /* Get remaining space */
	  if ((x < (LINBUFSIZ / 2)) ||
	      (zzstring(line, &s, &x) < 0)) { /* Expand variables, etc. */
	      printf("?Key definition too long\n");
	      if (flag) cmsetp(psave);
	      return(-9);
	  }
	  s = line + y + 1;		/* Point to result. */

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

	  switch (strlen(s)) {		/* Action depends on length */
	    case 0:			/* Clear individual key def */
	      deletekeymap(t,kc);
	      break;
	    case 1: 
	      defevt.type = key;	/* Single character */
	      defevt.key.scancode = *s;
	      break;
	    default:			/* Character string */
#ifndef NOKVERBS
	      if (p) {
		  y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
		  /* Need exact match */
		  debug(F101,"set key kverb lookup",0,y);
		  if (y > -1) {
		      defevt.type = kverb;
		      defevt.kverb.id = y;
		      break;
		  }
	      }
#endif /* NOKVERBS */
	      defevt.type = macro;
	      defevt.macro.string = (char *) malloc(strlen(s)+1);
	      if (defevt.macro.string)
		strcpy(defevt.macro.string, s);
	      break;
	  }
	  insertkeymap(t, kc, defevt);
	  if (flag) 
	    cmsetp(psave);
	  initvik = 1;			/* Update VIK table */
	  return(1);
      }
#endif /* OS2 */

      default:				/* Shouldn't get here. */
	return(-2);
    } 
#endif /* MAC */
#ifdef COMMENT
    /*
      This was supposed to shut up picky compilers but instead it makes
      most compilers complain about "statement not reached".
    */
    return(-2);
#endif /* COMMENT */
#ifdef OS2
return(-2);
#endif /* OS2 */
}

#ifdef OS2
int
settitle(void) {
    extern char usertitle[];
    if ((y = cmtxt("title text","",&s,xxstring)) < 0)
      return(y);
    strncpy(usertitle,s,63);
    return(1);
}

static struct keytab dialertab[] = {	/* K95 Dialer types */
    "backspace", 	0, 0,
    "enter",   		1, 0
};
static int ndialer = 2;

int
setdialer(void) {
    int t, x, y;
    int clear = 0, deflt = 0;
    int kc;				/* Key code */
    char *s = NULL;			/* Key binding */
#ifndef NOKVERBS
    char *p = NULL;			/* Worker */
#endif /* NOKVERBS */
    con_event defevt;
    extern int os2gks;
    extern int mskkeys;
    extern int initvik;

    if (( x = cmkey(dialertab, ndialer,
		    "Kermit-95 dialer work-arounds",
		    "", xxstring)) < 0 )
      return(x);
    switch (x) {
      case 0:				/* Backspace */
	kc = 264;
	break;
      case 1:				/* Enter */
	kc = 269;
	break;
      default:
	printf("Illegal value in setdialer()\n");
	return(-9);
    }
    if ((y = cmtxt("Key definition","",&s,xxstring)) < 0)
      return(y);

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

    /* Now reprogram the default value for all terminal types */
    /* remember to treat Wyse and Televideo terminals special */
    /* because of their use of Kverbs for Backspace and Enter */
    for (t = 0; t <= TT_MAX; t++) {
	if (ISWY50(t) || ISTVI(t)) {
	    extern char * udkfkeys[] ;
	    if (kc == 264) {		/* \Kwybs or \Ktvibs */
		if (udkfkeys[32])
		  free(udkfkeys[32]);
		udkfkeys[32] = strdup(s);
	    }	
	    if (kc == 269) {		/* \Kwyenter and \Kwyreturn */
		if (udkfkeys[39])	/* \Ktvienter and \Ktvireturn */
		  free(udkfkeys[39]);
		udkfkeys[39] = strdup(s);
		if (udkfkeys[49])
		  free(udkfkeys[49]);
		udkfkeys[49] = strdup(s);
	    }
	} else {
	    switch (strlen(s)) {	/* Action depends on length */
	      case 0:			/* Clear individual key def */
		deletekeymap(t,kc);
		break;
	      case 1: 
		defevt.type = key;	/* Single character */
		defevt.key.scancode = *s;
		break;
	      default:			/* Character string */
#ifndef NOKVERBS
		if (p) {
		    y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
		    /* Exact match req'd */
		    debug(F101,"set key kverb lookup",0,y);
		    if (y > -1) {
			defevt.type = kverb;
			defevt.kverb.id = y;
			break;
		    }
		}
#endif /* NOKVERBS */
		defevt.type = macro;
		defevt.macro.string = (char *) malloc(strlen(s)+1);
		if (defevt.macro.string)
		  strcpy(defevt.macro.string, s);
		break;
	    }
	    insertkeymap( t, kc, defevt ) ;
	    initvik = 1;		/* Update VIK table */
	}
    }
    return(1);
}
#endif /* OS2 */

#ifdef NT
CHAR (*win95kcs)(CHAR) = NULL;
int win95altgr = 0;
int win95kl2 = 0;

extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);
extern struct keytab tcstab[];
extern int ntcs;
extern int maxow, owwait;		/* Overlapped I/O variables */

int
setwin95( void ) {
    int x, y, z;

    if (( y = cmkey(win95tab, nwin95,
		    "Windows 95 specific work-arounds",
		    "keyboard-translation",
		    xxstring)) < 0 )
	return (y);
    switch ( y ) {
      case XYWAGR:
	if ((y = cmkey(onoff,2,"Right-Alt is Alt-Gr","off",xxstring)) < 0)
	  return(y);
	if ((x = cmcfm()) < 0) return(x);
	win95altgr = y;
	return(1);

      case XYWOIO:
	if ((y = cmkey(onoff,2,"Use Overlapped I/O","off",xxstring)) < 0)
	  return(y);
	if ( y ) {
	    if ((x = cmnum("Num of max I/O requests","1",10,&z,xxstring)) < 0)
	      return(x);
	    if ( z < 1 || z > 5 ) {
		printf("?max I/O requests must be between 1 and 5.\n");
		return(-9);
	    }
	} else
	  z = 1;
	if ((x = cmcfm()) < 0) return(x);
	owwait = !y;
	maxow = z;
	return(1);

      case XYWKEY:
	if (( z = cmkey(tcstab, ntcs,
			 "Keyboard Character Set",
			 "latin1-iso",
			 xxstring)) < 0 )
	    return (z);
	if ((x = cmcfm()) < 0)
	  return(x);

	win95kl2 = (z == TC_2LATIN);

	if ( z == TC_TRANSP )
	  win95kcs = NULL;
	else 
	  win95kcs = xlr[z][tcsl];
	return(1);

      default:
	printf("Illegal value in setwin95()\n");
	return(-9);
    }
}
#endif /* NT */

#ifdef OS2
int
setprty (
#ifdef CK_ANSIC
    void
#endif /* CK_ANSIC */
/* setprty */ ) {
    int x, y, z;

    if (( y = cmkey(prtytab, nprty,
		    "priority level of terminal and communication threads",
		    "foreground-server",
		    xxstring)) < 0 )
      return (y);

    if ((x = cmcfm()) < 0)
      return (x);
    priority = y;
    return(TRUE); 
}

int
setbell(
#ifdef CK_ANSIC
    void
#endif /* CK_ANSIC */
 /* setbell */ ) {
    int z, y, x;

    if ((y = cmkey(beltab,nbeltab,
        "how console and terminal bells should\nbe generated",
        "audible",xxstring)) < 0)
	  return(y);

    switch ( y ) {
        case XYB_NONE:
        case XYB_VIS:
    	    if ((x = cmcfm()) < 0)
                return(x);
            tt_bell = y;
            break;

        case XYB_AUD:
            if ((x = cmkey(audibletab, naudibletab,
             "how audible console and terminal\nbells should be generated",
                "beep",xxstring))<0)
                return(x);
            if ((z = cmcfm()) < 0)
                return(z);
            tt_bell = y | x;
            break;
        }
	return(1);
    }
#endif /* OS2 */

#ifdef OS2MOUSE
int
setmou(
#ifdef CK_ANSIC
       void
#endif /* CK_ANSIC */
 /* setmou */ ) {
    extern int initvik;
    int button = 0, event = 0;
    char * p;

    if ((y = cmkey(mousetab,nmtab,"","",xxstring)) < 0)
      return(y);

    if (y == XYM_ON) {			/* MOUSE ACTIVATION */
        int old_mou = tt_mouse;
        seton(&tt_mouse);
        if ( tt_mouse != old_mou )
          if ( tt_mouse )
            os2_mouseon();
          else
            os2_mouseoff();
        return(1);
    }

    if (y == XYM_CLEAR) {		/* Reset Mouse Defaults */
	mousemapinit(-1,-1);
	initvik = 1;			/* Update VIK Table */
	return 1;
    }
    if (y != XYM_BUTTON) {		/* Shouldn't happen. */
	printf("Internal parsing error\n");
	return(-9);
    }

    /* MOUSE EVENT ... */

    if ((button = cmkey(mousebuttontab,nmbtab,
			"Button number, one of the following","1",
			xxstring)) < 0)
      return(button);

    if ((y =  cmkey(mousemodtab,nmmtab,
		    "Keyboard modifier, one of the following",
		    "none",xxstring)) < 0)
      return(y);

    event |= y;				/* OR in the bits */

    if ((y =  cmkey(mclicktab,nmctab,"","click",xxstring)) < 0)
      return(y);

    /* Two bits are assigned, if neither are set then it is button one */

    event |= y;			/* OR in the bit */

    wideresult = -1;

    if ((y = cmtxt("definition,\n\
or Ctrl-C to cancel this command,\n\
or Enter to restore default definition",
		   "",&s,NULL)) < 0) {
	return(y);
    }
    s = brstrip(s);
    p = s;				/* Save this place */
/*
  If the definition included any \Kverbs, quote the backslash so the \Kverb
  will still be in the definition when the key is pressed.  We don't do this
  in zzstring(), because \Kverbs are valid only in this context and nowhere
  else.  This code copied from SET KEY, q.v. for addt'l commentary.
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
#else
    p = NULL;
#endif /* NOKVERBS */

    /* free the old definition if necessary */
    if (mousemap[button][event].type == macro) {
        free( mousemap[button][event].macro.string);
	mousemap[button][event].macro.string = NULL;
    }
    switch (strlen(s)) {		/* Action depends on length */
      case 0:				/* Reset to default binding */
        mousemapinit( button, event );
    	break;
      case 1:				/* Single character */
  	    mousemap[button][event].type = key;
        mousemap[button][event].key.scancode = *s;
      	break;
      default:				/* Character string */
#ifndef NOKVERBS
	if (p) {
	    y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
	    debug(F101,"set mouse kverb lookup",0,y); /* need exact match */
	    if (y > -1) {
            /* Assign the kverb to the event */
            mousemap[button][event].type = kverb;
            mousemap[button][event].kverb.id = F_KVERB | y; 
            break;
	    }
	}
#endif /* NOKVERBS */

       /* Otherwise, it's a macro, so assign the macro to the event */
       mousemap[button][event].type = macro;
       mousemap[button][event].macro.string = (MACRO) malloc(strlen(s)+1);
       if (mousemap[button][event].macro.string)
         strcpy((char *) mousemap[button][event].macro.string, s);
        break;
    }
    initvik = 1;			/* Update VIK Table */
    return(1);
}
#endif /* OS2MOUSE */
#endif /* NOLOCAL */

int					/* SET SEND/RECEIVE */
setsr(xx, rmsflg) int xx; int rmsflg; {
    if (xx == XYRECV)
      strcpy(line,"Parameter for inbound packets");
    else
      strcpy(line,"Parameter for outbound packets");
 
    if (rmsflg) {
	if ((y = cmkey(rsrtab,nrsrtab,line,"",xxstring)) < 0) {
	    if (y == -3) {
		printf("?Remote receive parameter required\n");
		return(-9);
	    } else return(y);
	}
    } else {
	if ((y = cmkey(srtab,nsrtab,line,"",xxstring)) < 0) return(y);
    }
    switch (y) {
      case XYQCTL:			/* CONTROL-PREFIX */
	if ((x = cmnum("ASCII value of control prefix","",10,&y,xxstring)) < 0)
	  return(x);
	if ((x = cmcfm()) < 0) return(x);
	if ((y > 32 && y < 63) || (y > 95 && y < 127)) {
	    if (xx == XYRECV)
	      ctlq = (CHAR) y;		/* RECEIVE prefix, use with caution! */
	    else
	      myctlq = (CHAR) y;	/* SEND prefix, OK to change */
	    return(success = 1); 
	} else {
	    printf("?Illegal value for prefix character\n");
	    return(-9);
	}

      case XYEOL:
	if ((y = setcc("13",&z)) < 0)
	    return(y);
	if (z > 31) {
	    printf("Sorry, the legal values are 0-31\n");
	    return(-9);
	}
	if (xx == XYRECV)
	  eol = (CHAR) z;
	else
	  seol = (CHAR) z;
	return(success = y);
 
      case XYLEN:
	y = cmnum("Maximum number of characters in a packet","90",10,&x,
		  xxstring);
	if (xx == XYRECV) {		/* Receive... */
	    if ((y = setnum(&z,x,y,maxrps)) < 0)
	      return(y);
	    if (z < 10) {
		printf("Sorry, 10 is the minimum\n");
		return(-9);
	    }
	    if (rmsflg) {
		tp = tmpbuf;
		sprintf(tp,"%d",z);
		sstate = setgen('S', "401", tp, "");
		return((int) sstate);
	    } else {
		if (protocol == PROTO_K) {
		    if (z > MAXRP) z = MAXRP;
		    y = adjpkl(z,wslotr,bigrbsiz);
		    if (y != z) {
			urpsiz = y;
			if (
#ifndef NOSPL
			    cmdlvl == 0
#else
			    tlevel < 0
#endif /* NOSPL */
			    )
			  if (msgflg) printf(
" Adjusting receive packet-length to %d for %d window slots\n",
					     y, wslotr);
		    }
		    urpsiz = y;
		    ptab[protocol].rpktlen = urpsiz;
		    rpsiz =  (y > 94) ? 94 : y;
		} else {
#ifdef CK_XYZ
		    if (protocol == PROTO_X && z != 128 && z != 1024) {
			printf("Sorry, bad packet length for XMODEM.\n");
			printf("Please use 128 or 1024.\n");
			return(-9);
		    }
#endif /* CK_XYZ */
		    urpsiz = rpsiz = z;
		}
	    }
	} else {			/* Send... */
	    if ((y = setnum(&z,x,y,maxsps)) < 0)
	      return(y);
	    if (z < 10) {
		printf("Sorry, 10 is the minimum\n");
		return(-9);
	    }
	    if (protocol == PROTO_K) {
		if (z > MAXSP) z = MAXSP;
		spsiz = z;		/* Set it */
		y = adjpkl(spsiz,wslotr,bigsbsiz);
		if (y != spsiz &&
#ifndef NOSPL
		    cmdlvl == 0
#else
		    tlevel < 0
#endif /* NOSPL */
		    )
		  if (msgflg)
		    printf("Adjusting packet size to %d for %d window slots\n",
			   y,wslotr);
	    } else
	      y = z;
#ifdef CK_XYZ
	    if (protocol == PROTO_X && z != 128 && z != 1024) {
		printf("Sorry, bad packet length for XMODEM.\n");
		printf("Please use 128 or 1024.\n");
		return(-9);
	    }
#endif /* CK_XYZ */
	    spsiz = spmax = spsizr = y;	/* Set it and flag that it was set */
	    spsizf = 1;			/* to allow overriding Send-Init. */
	    ptab[protocol].spktflg = spsizf;
	    ptab[protocol].spktlen = spsiz;
	}
	if (pflag && protocol == PROTO_K &&
#ifndef NOSPL
	    cmdlvl == 0
#else
	    tlevel < 0
#endif /* NOSPL */
	    ) {
	    if (z > 94 && msgflg) {
		/* printf("Extended-length packets requested.\n"); */
		if (bctr < 2 && z > 200) printf("\
Remember to SET BLOCK 2 or 3 for long packets.\n");
	    }
	    if (speed <= 0L) speed = ttgspd();
#ifdef COMMENT
/*
  Kermit does this now itself.
*/
	    if (speed <= 0L && z > 200 && msgflg) {
		printf("\
Make sure your timeout interval is long enough for %d-byte packets.\n",z);
	    }
#endif /* COMMENT */
	}
	return(success = y);

      case XYMARK:
#ifdef DOOMSDAY
/*
  Printable start-of-packet works for UNIX and VMS only!
*/
	x_ifnum = 1;
	y = cmnum("Code for packet-start character","1",10,&x,xxstring);
	x_ifnum = 0;
	if ((y = setnum(&z,x,y,126)) < 0) return(y);
#else
	if ((y = setcc("1",&z)) < 0)
	    return(y);
#endif /* DOOMSDAY */
	if (xx == XYRECV)
	  stchr = (CHAR) z;
	else
	  mystch = (CHAR) z;
	return(success = y);

      case XYNPAD:			/* PADDING */
	y = cmnum("How many padding characters for inbound packets","0",10,&x,
		  xxstring);
	if ((y = setnum(&z,x,y,94)) < 0) return(y);
	if (xx == XYRECV)
	  mypadn = (CHAR) z;
	else
	  npad = (CHAR) z;
	return(success = y);
 
      case XYPADC:			/* PAD-CHARACTER */
	if ((y = setcc("0",&z)) < 0) return(y);
	if (xx == XYRECV) mypadc = z; else padch = z;
	return(success = y);
 
      case XYTIMO:			/* TIMEOUT */
	if (xx == XYRECV) {
	    char buf[16];		/* Construct default */
	    sprintf(buf,"%d",URTIME);
	    y = cmnum("Packet timeout interval",buf,10,&x,xxstring);
	    if ((y = setnum(&z,x,y,94)) < 0) return(y);

	    if (rmsflg) {		/* REMOTE SET RECEIVE TIMEOUT */
		tp = tmpbuf;		/*   Tell Kermit server what */
		sprintf(tp,"%d",z);	/*   timeout to ask me to use. */
		sstate = setgen('S', "402", tp, "");
		return((int) sstate);
	    } else {			/* SET RECEIVE TIMEOUT */
		pkttim = z;		/*   Value to put in my negotiation */
	    }				/*   packet for other Kermit to use */

	} else {			/* SET SEND TIMEOUT */
#ifdef CK_TIMERS
	    extern int rttflg, mintime, maxtime;
	    int tmin, tmax;
#endif /* CK_TIMERS */
	    y = cmnum("Packet timeout interval","",10,&x,xxstring);
	    if (y == -3) {		/* They cancelled a previous */
		x = DMYTIM;		/* SET SEND command, so restore */
		y = 0;			/* the default */
		timef = 0;		/* and turn off the override flag */
	    }
#ifdef CK_TIMERS	    
	    if (y < 0) return(y);
	    if (x < 0) {
		printf("?Out of range - %d\n",x);
		return(-9);
	    }
	    if ((z = cmkey(timotab,2,"","dynamic",xxstring)) < 0) return(z);
	    if (z) {
		if ((y = cmnum("Minimum timeout to allow",
			       "1",10,&tmin,xxstring)) < 0)
		  return(y);
		if (tmin < 1) {
		    printf("?Out of range - %d\n",x);
		    return(-9);
		}
		if ((y = cmnum("Maximum timeout to allow",
			       "0",10,&tmax,xxstring)) < 0)
		  return(y);
		/* 0 means let Kermit choose, < 0 means no maximum */
	    }
	    if ((y = cmcfm()) < 0)
	      return(y);
	    rttflg = z;			/* Round-trip timer flag */
	    z = x;
#else
	    if ((y = setnum(&z,x,y,94)) < 0)
	      return(y);
#endif /* CK_TIMERS */
	    timef = 1;			/* Turn on the override flag */
	    timint = rtimo = z;		/* Override value for me to use */
	    if (rttflg) {		/* Lower and upper bounds */
		mintime = tmin;
		maxtime = tmax;
	    }
	}
	return(success = 1);

      case XYFPATH:			/* PATHNAMES */
	if ((y = cmkey(onoff,2,"","on",xxstring)) < 0) return(y);
	if ((x = cmcfm()) < 0) return(x);
	if (xx == XYRECV) {		/* SET RECEIVE PATHNAMES */
	    fnrpath = 1 - y;		/* OFF (with their heads!), ON */
	    ptab[protocol].fnrp = fnrpath;
	} else {			/* SET SEND PATHNAMES */
	    fnspath = 1 - y;		/* OFF, ON */
	    ptab[protocol].fnsp = fnspath;
	}
	return(success = 1);		/* Note: 0 = ON, 1 = OFF */
	/* In other words, ON = leave pathnames ON, OFF = take them off. */

      case XYPAUS:			/* SET SEND/RECEIVE PAUSE */
	y = cmnum("Milliseconds to pause between packets","0",10,&x,xxstring);
	if ((y = setnum(&z,x,y,15000)) < 0)
	  return(y);
	pktpaus = z;
	return(success = 1);

      default:
	return(-2);
    }					/* End of SET SEND/RECEIVE... */
}

#ifndef NOXMIT
int
setxmit() {
    if ((y = cmkey(xmitab,nxmit,"","",xxstring)) < 0) return(y);
    switch (y) {
      case XMITE:			/* EOF */
	y = cmtxt("Characters to send at end of file,\n\
 Use backslash codes for control characters","",&s,xxstring);
	if (y < 0) return(y);
	if ((int)strlen(s) > XMBUFL) {
	    printf("?Too many characters, %d maximum\n",XMBUFL);
	    return(-2);
	}
	strcpy(xmitbuf,s);
	return(success = 1);

      case XMITF:			/* Fill */
	y = cmnum("Numeric code for blank-line fill character","0",10,&x,
		  xxstring);
	if ((y = setnum(&z,x,y,127)) < 0) return(y);
	xmitf = z;
	return(success = 1);
      case XMITL:			/* Linefeed */
        return(seton(&xmitl));
      case XMITS:			/* Locking-Shift */
        return(seton(&xmits));
      case XMITP:			/* Prompt */
	y = cmnum("Numeric code for host's prompt character, 0 for none",
		  "10",10,&x,xxstring);
	if ((y = setnum(&z,x,y,127)) < 0) return(y);
	xmitp = z;
	return(success = 1);
      case XMITX:			/* Echo */
        return(seton(&xmitx));
      case XMITW:			/* Pause */
	y = cmnum("Number of milliseconds to pause between binary characters\n\
or text lines during transmission","0",10,&x,xxstring);
	if ((y = setnum(&z,x,y,1000)) < 0) return(y);
	xmitw = z;
	return(success = 1);
      default:
	return(-2);
    }
}
#endif /* NOXMIT */

/*  D O R M T  --  Do a remote command  */
 
VOID
rmsg() {
    if (pflag)
      printf(
#ifdef CK_NEED_SIG
       " Type your escape character, %s, followed by X or E to cancel.\n",
       dbchr(escape)
#else
       " Press the X or E key to cancel.\n"
#endif /* CK_NEED_SIG */
      );
}

/*  R E M C F M  --  Confirm a REMOTE command  */
/*
  Like cmcfm(), but allows for a redirection indicator on the end,
  like "> filename" or "| command".  Returns what cmcfm() would have
  returned: -1 if reparse needed, etc etc blah blah.  On success,
  returns 1 with:

    char * remdest containing the name of the file or command.
    int remfile set to 1 if there is to be any redirection.
    int rempipe set to 1 if remdest is a command, 0 if it is a file.
*/
static int
remcfm() {
    int x;
    char *s;
    char c;

    remfile = 0;
    rempipe = 0;
    if ((x = cmtxt(
	     "> filename, | command,\n\
or type carriage return to confirm the command",
		   "",&s,xxstring)) < 0)
      return(x);
    if (remdest) {
	free(remdest);
	remdest = NULL;
    }
    if (!*s)				/* No redirection indicator */
      return(1);
    c = *s;				/* We have something */
    if (c != '>' && c != '|') {		/* Is it > or | ? */
	printf("?Not confirmed\n");	/* No */
	return(-9);
    }
    s++;				/* See what follows */
    if (c == '>' && *s == '>') {	/* Allow for ">>" too */
	s++;
	remappd = 1;			/* Append to output file */
    }
    while (*s == SP || *s == HT) s++;	/* Strip intervening whitespace */
    if (!*s) {
	printf("?%s missing\n", c == '>' ? "Filename" : "Command");
	return(-9);
    }
    if (c == '>' && zchko(s) < 0) {	/* Check accessibility */
	printf("?Access denied - %s\n", s);
	return(-9);
    }
    remfile = 1;			/* Set global results */
    rempipe = (c == '|');
    makestr(&remdest,s);
#ifndef NODEBUG
    if (deblog) {
	debug(F101,"remcfm remfile","",remfile);
	debug(F101,"remcfm remappd","",remappd);
	debug(F101,"remcfm rempipe","",rempipe);
	debug(F110,"remcfm remdest",remdest, 0);
    }
#endif /* NODEBUG */
    return(1);
}

/*  R E M T X T  --  Like remcfm()...  */
/*
   ... but for REMOTE commands that end with cmtxt().
   Here we must decipher braces to discover whether the trailing
   redirection indicator is intended for local use, or to be sent out
   to the server, as in:

     remote host blah blah > file                 This end
     remote host { blah blah } > file             This end
     remote host { blah blah > file }             That end
     remote host { blah blah > file } > file      Both ends

   Pipes too:

     remote host blah blah | cmd                  This end
     remote host { blah blah } | cmd              This end
     remote host { blah blah | cmd }              That end
     remote host { blah blah | cmd } | cmd        Both ends

   Or both:

     remote host blah blah | cmd > file           This end, etc etc...

   Note: this really only makes sense for REMOTE HOST, but why be picky?
   Call after calling cmtxt(), with pointer to string that cmtxt() parsed,
   as in "remtxt(&s);".

   Returns:
    1 on success with braces & redirection things removed & pointer updated,
   -9 on failure (bad indirection), after printing error message.
*/
static int
remtxt(p) char ** p; {
    int i, x, bpos, ppos;
    char c, *s, *q;

    remfile = 0;			/* Initialize global results */
    rempipe = 0;
    remappd = 0;
    if (remdest) {
	free(remdest);
	remdest = NULL;
    }
    s = *p;
    if (!*s)				/* No redirection indicator */
      return(1);			/* Done */

    bpos = -1;				/* Position of > (bracket) */
    ppos = -1;				/* Position of | (pipe) */
    x = strlen(s);			/* Length of cmtxt() string */

    for (i = x-1; i >= 0; i--) {	/* Search right to left. */
	c = s[i];
	if (c == '}')			/* Break on first right brace */
	  break;			/* Don't look at contents of braces */
	else if (c == '>')		/* Record position of > */
	  bpos = i;
	else if (c == '|')		/* and of | */
	  ppos = i;
    }
    if (bpos < 0 && ppos < 0) {		/* No redirectors. */
	s = brstrip(s);			/* Remove outer braces if any. */
	*p = s;				/* Point to result */
	return(1);			/* and return. */
    }    
    remfile = 1;			/* It's | or > */
    i = -1;				/* Get leftmost symbol */
    if (bpos > -1)			/* Bracket */
      i = bpos;
    if (ppos > -1 && ppos < bpos) {	/* or pipe */
	i = ppos;
	rempipe = 1;
    }
    c = s[i];				/* Copy of symbol */

    if (c == '>' && s[i+1] == '>')	/* ">>" for append? */
      remappd = 1;		       /* It's not just a flag it's a number */

    q = s + i + 1 + remappd;		/* Point past symbol in string */
    while (*q == SP || *q == HT) q++;	/* and any intervening whitespace */
    if (!*q) {
	printf("?%s missing\n", c == '>' ? "Filename" : "Command");
	return(-9);
    }
    if (c == '>' && zchko(q) < 0) {	/* (Doesn't work for | cmd > file) */
	printf("?Access denied - %s\n", q);
	return(-9);
    }
    makestr(&remdest,q);		/* Create the destination string */
    q = s + i - 1;			/* Point before symbol */
    while (q > s && (*q == SP || *q == HT)) /* Strip trailing whitespace */
      q--;
    *(q+1) = NUL;			/* Terminate the string. */
    s = brstrip(s);			/* Remove any braces */
    *p = s;				/* Set return value */

#ifndef NODEBUG
    if (deblog) {
	debug(F101,"remtxt remfile","",remfile);
	debug(F101,"remtxt remappd","",remappd);
	debug(F101,"remtxt rempipe","",rempipe);
	debug(F110,"remtxt remdest",remdest, 0);
	debug(F110,"remtxt command",s,0);
    }
#endif /* NODEBUG */

    return(1);
}

int
dormt(xx) int xx; {			/* REMOTE commands */
    int x, y, retcode;
    char *s, sbuf[50], *s2;
 
    remfile = 0;			/* Clear these */
    rempipe = 0;
    remappd = 0;

    if (xx < 0) return(xx);		/* REMOTE what? */

    if (xx == XZSET) {			/* REMOTE SET */
	if ((y = cmkey(rmstab,nrms,"","",xxstring)) < 0) {
	    if (y == -3) {
		printf("?Parameter name required\n");
		return(-9);
	    } else return(y);
	}
	return(doprm(y,1));
    }

    switch (xx) {			/* Others... */
 
      case XZCWD:			/* CWD (CD) */
	if ((x = cmtxt("Remote directory name","",&s,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&s)) < 0)
	  return(x);
	debug(F111,"XZCWD: ",s,x);
	*sbuf = NUL;
	s2 = sbuf;
/*
  The following is commented out because since the disappearance of the
  DECSYSTEM-20 from the planet, no known computer requires a password for
  changing directory.
*/
#ifdef DIRPWDPR
	if (*s != NUL) {		/* If directory name given, */
					/* get password on separate line. */
	    if (tlevel > -1) {		/* From take file... */
 
		if (fgets(sbuf,50,tfile[tlevel]) == NULL)
		  fatal("take file ends prematurely in 'remote cwd'");
		debug(F110," pswd from take file",s2,0);
		for (x = (int)strlen(sbuf);
		     x > 0 && (sbuf[x-1] == NL || sbuf[x-1] == CR);
		     x--)
		  sbuf[x-1] = '\0';
 
	    } else {			/* From terminal... */
 
		printf(" Password: ");	/* get a password */
		while (
#ifdef OS2
		       ((x = is_a_tty(0) ? coninc(0) : /* with no echo ... */
			 getchar()) != NL) && (x != CR)
#else
		       ((x = getchar()) != NL) && (x != CR)
#endif /* OS2 */
		       ) {
		    if ((x &= 0177) == '?') {
			printf("? Password of remote directory\n Password: ");
			s2 = sbuf;
			*sbuf = NUL;
		    } else if (x == ESC) /* Mini command line editor... */
		      bleep(BP_WARN);
		    else if (x == BS || x == 0177)
		      s2--;
		    else if (x == 025) {	/* Ctrl-U */
			s2 = sbuf;
			*sbuf = NUL;
		    } else
		      *s2++ = x;
		}
		*s2 = NUL;
		putchar('\n');
	    }
	    s2 = sbuf;
	} else s2 = "";
#endif /* DIRPWDPR */

	debug(F110," password",s2,0);
	sstate = setgen('C',s,s2,"");
	retcode = 0;
	break;

      case XZDEL:				/* Delete */
	if ((x = cmtxt("Name of remote file(s) to delete",
		       "",&s,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of remote file(s) required\n");
		return(-9);
	    } else return(x);
	}
	if ((x = remtxt(&s)) < 0)
	  return(x);
	if (local) ttflui();		/* If local, flush tty input buffer */
	retcode = sstate = rfilop(s,'E');
	break;
 
      case XZDIR:			/* Directory */
	if ((x = cmtxt("Remote directory or file specification","",&s,
		       xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&s)) < 0)
	  return(x);
	if (local) ttflui();		/* If local, flush tty input buffer */
	rmsg();
	retcode = sstate = setgen('D',s,"","");
	break;
 
      case XZHLP:			/* Help */
	if ((x = remcfm()) < 0) return(x);
	sstate = setgen('H',"","","");
	retcode = 0;
	break; 

#ifndef NOPUSH
/* Why is this ifdef'd? */

      case XZHOS:			/* Host */
	if (nopush) {
	    if ((x = remcfm()) < 0) return(x);
	    printf("?Not available - %s\n",cmdbuf);
	    return(-2);
	}
	if ((x = cmtxt("Command for remote system","",&cmarg,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&cmarg)) < 0)
	  return(x);
	if ((y = (int)strlen(cmarg)) < 1)
	  return(x);
	rmsg();
	retcode = sstate = 'c';
	break; 
#endif /* NOPUSH */

#ifndef NOFRILLS
      case XZKER:
	if ((x = cmtxt("Command for remote Kermit","",&cmarg,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&cmarg)) < 0)
	  return(x);
	if ((int)strlen(cmarg) < 1)  {
	    if (x == -3) {
		printf("?Remote Kermit command required\n");
		return(-9);
	    } else return(x);
	}
	retcode = sstate = 'k';
	rmsg();
	break; 

      case XZLGI: {			/* Login */
	  char *p1, *p2, *p3;
	  if ((x = cmfld("User ID","",&s,xxstring)) < 0)
	    return(x);
	  if ((p1 = malloc((int)strlen(s) + 1)) == NULL) {
	      printf("Internal error: malloc\n");
	      return(-2);
	  } else
	    strcpy(p1,s);
	  if ((x = cmfld("Password","",&s,xxstring)) < 0)
	    return(x);
	  if ((p2 = malloc((int)strlen(s) + 1)) == NULL) {
	      printf("Internal error: malloc\n");
	      return(-2);
	  } else
	    strcpy(p2,s);
	  if ((x = cmtxt("Account or carriage return","",
			 &s,xxstring)) < 0 && x != -3)
	    return(x);
	  if ((x = remtxt(&s)) < 0)
	    return(x);
	  if ((p3 = malloc((int)strlen(s) + 1)) == NULL) {
	      printf("Internal error: malloc\n");
	      return(-2);
	  } else
	    strcpy(p3,s);
	  sstate = setgen('I',p1,p2,p3);
	  if (p3) { free(p3); p3 = NULL; }
	  if (p2) { free(p2); p2 = NULL; }
	  if (p1) { free(p1); p1 = NULL; }
	  retcode = 0;
	  break; 
      }

      case XZLGO:			/* Logout */
	if ((x = remcfm()) < 0) return(x);
	sstate = setgen('I',"","","");
	retcode = 0;
	break; 

      case XZPRI:			/* Print */
	if (!atdiso || !atcapr) {	/* Disposition attribute off? */
	    printf("?Disposition Attribute is Off\n");
	    return(-2);
	}
	cmarg = "";
	cmarg2 = "";
	if ((x = cmifi("Local file(s) to print on remote printer","",&s,&y,
		       xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of local file(s) required\n");
		return(-9);
	    }
	    return(x);
	}
	strcpy(line,s);			/* Make a safe copy of filename */
	*optbuf = NUL;			/* Wipe out any old options */
	if ((x = cmtxt("Options for remote print command","",&s,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&s)) < 0)
	  return(x);
	strcpy(optbuf,s);		/* Make a safe copy of options */
	if ((int)strlen(optbuf) > 94) {	/* Make sure this is legal */
	    printf("?Option string too long\n");
	    return(-9);
	}
	nfils = -1;			/* Expand file list internally */
	cmarg = line;			/* Point to file list. */
	rprintf = 1;			/* REMOTE PRINT modifier for SEND */
	sstate = 's';			/* Set start state to SEND */
	if (local) displa = 1;
	retcode = 0;
	break;
#endif /* NOFRILLS */
	
      case XZSPA:			/* Space */
	if ((x = cmtxt("Confirm, or remote directory name",
		       "",&s,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&s)) < 0)
	  return(x);
	retcode = sstate = setgen('U',s,"","");
	break;
    
#ifndef NOFRILLS
      case XZTYP:			/* Type */
	if ((x = cmtxt("Remote file specification","",&s,xxstring)) < 0)
	  return(x);
	if ((int)strlen(s) < 1) {
	    printf("?Remote filename required\n");
	    return(-9);	
	}
	if ((x = remtxt(&s)) < 0)
	  return(x);
	rmsg();
	retcode = sstate = rfilop(s,'T');
	break;
#endif /* NOFRILLS */
 
#ifndef NOFRILLS
      case XZWHO:
	if ((x = cmtxt("Remote user name, or carriage return",
		       "",&s,xxstring)) < 0)
	  return(x);
	if ((x = remtxt(&s)) < 0)
	  return(x);
	retcode = sstate = setgen('W',s,"","");
	break;
#endif /* NOFRILLS */
 
      case XZPWD:			/* PWD */
	if ((x = remcfm()) < 0) return(x);
	sstate = setgen('A',"","","");
	retcode = 0;
	break; 
    
#ifndef NOSPL
      case XZQUE: {			/* Query */
	  char buf[2];
	  extern char querybuf[], * qbufp;
	  extern int qbufn;
	  if ((y = cmkey(vartyp,nvartyp,"","",xxstring)) < 0)
	    return(y);
	  if ((x = cmtxt("Remote variable name","",&s,NULL)) < 0) /* No eval */
	    return(x);
	  if ((x = remtxt(&s)) < 0)
	    return(x);
	  query = 1;			/* QUERY is active */
	  qbufp = querybuf;		/* Initialize query response buffer */
	  qbufn = 0;
	  querybuf[0] = NUL;
	  buf[0] = (char) (y & 127);
	  buf[1] = NUL;
	  retcode = sstate = setgen('V',"Q",(char *)buf,s);
	  break;
      }

      case XZASG: {			/* Assign */
	  char buf[VNAML];
	  if ((y = cmfld("Remote variable name","",&s,NULL)) < 0) /* No eval */
	    return(y);
	  strcpy(buf,s);
	  if ((x = cmtxt("Assignment for remote variable",
		   "",&s,xxstring)) < 0) /* Evaluate this one */
	    return(x);
	  if ((x = remtxt(&s)) < 0)
	    return(x);
#ifdef COMMENT
/*
  Server commands can't be long packets.  In principle there's no reason
  why they shouldn't be, except that we don't know at this point if the
  server is capable of accepting long packets because we haven't started
  the protocol yet.  In practice, allowing a long packet here breaks a lot
  of assumptions, causes buffer overruns and crashes, etc.  To be fixed
  later.  (But since this is commented out, evidently I fixed it later...)
*/
	  if ((int)strlen(s) > 85) {	/* Allow for encoding expansion */
	      printf("?Sorry, value is too long - 85 characters max\n");
	      return(-9);
	  }
#endif /* COMMENT */
	  retcode = sstate = setgen('V',"S",(char *)buf,s);
	  break;
      }
#endif /* NOSPL */

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif /* MAXPATHLEN */

      case XZCPY: {			/* Copy */
	  char buf[MAXPATHLEN+1];
	  buf[MAXPATHLEN] = '\0';
	  if ((x = cmfld("Name of remote file to copy","",&s,xxstring)) < 0) {
	      if (x == -3) {
		  printf("?Name of remote file required\n");
		  return(-9);
	      }
	      else
		return(x);
	  }
	  strncpy(buf,s,MAXPATHLEN);
	  if ((x = cmfld("Name of remote destination file or directory",
			 "",&s, xxstring)) < 0) {
	      if (x == -3) {
		  printf("?Name of remote file or directory required\n");
		  return(-9);
	      } else return(x);
	  }
	  if ((x = remcfm()) < 0)
	    return(x);
	  if (local) ttflui();		/* If local, flush tty input buffer */
	  retcode = sstate = setgen('K',buf,s,"");
	  break;
      }

      case XZREN: {			/* Rename */
	  char buf[MAXPATHLEN+1];
	  buf[MAXPATHLEN] = '\0';
	  if ((x = cmfld("Name of remote file to rename",
			 "",&s,xxstring)) < 0) {
	      if (x == -3) {
		  printf("?Name of remote file required\n");
		  return(-9);
	      } else return(x);
	  }
	  strncpy(buf,s,MAXPATHLEN);
	  if ((x = cmfld("New name of remote file","",&s, xxstring)) < 0) {
	      if (x == -3) {
		  printf("?Name of remote file required\n");
		  return(-9);
	      } else return(x);
	  }
	  if ((x = remcfm()) < 0)
	    return(x);
	  if (local) ttflui();		/* If local, flush tty input buffer */
	  retcode = sstate = setgen('R',buf,s,"");
	  break;
      }

      default:
        if ((x = remcfm()) < 0) return(x);
        printf("?Not implemented - %s\n",cmdbuf);
        return(-2);
    }
    if (local) ttflui();		/* If local, flush tty input buffer */
    return(retcode);
}
 
 
/*  R F I L O P  --  Remote File Operation  */
 
CHAR
#ifdef CK_ANSIC
rfilop(char * s, char t)
#else
rfilop(s,t) char *s, t; 
#endif /* CK_ANSIC */
/* rfilop */ {
    if (*s == NUL) {
	printf("?File specification required\n");
	return((CHAR) 0);
    }
    debug(F111,"rfilop",s,t);
    return(setgen(t,s,"",""));
}

#ifdef ANYX25
int
setx25() {
    if ((y = cmkey(x25tab,nx25,"X.25 call options","",xxstring)) < 0)
      return(y);
    switch (y) {
      case XYUDAT:
	if ((z = cmkey(onoff,2,"X.25 call user data","",xxstring))
	    < 0) return(z);
	if (z == 0) {
	    if ((z = cmcfm()) < 0) return(z);
	    cudata = 0;             /* disable call user data */
	    return (success = 1);
	}
	if ((x = cmtxt("X.25 call user data string","",&s,xxstring)) < 0)
	  return(x);
	if ((int)strlen(s) == 0) {
	    return (-3);
	} else if ((int)strlen(s) > MAXCUDATA) {
	    printf("?The length must be > 0 and <= %d\n",MAXCUDATA);
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	strcpy(udata,s);
	cudata = 1;			/* X.25 call user data specified */
	return (success = 1);
      case XYCLOS:
	if ((z = cmkey(onoff,2,"X.25 closed user group call","",xxstring))
	    < 0) return(z);
	if (z == 0) {
	    if ((z = cmcfm()) < 0) return(z);
	    closgr = -1;		/* disable closed user group */
	    return (success = 1);
	}
	if ((y = cmnum("0 <= cug index >= 99","",10,&x,xxstring)) < 0)
	  return(y);
	if (x < 0 || x > 99) {
	    printf("?The choices are 0 <= cug index >= 99\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	closgr = x;			/* closed user group selected */
	return (success = 1);

      case XYREVC:
	if((z = cmkey(onoff,2,"X.25 reverse charge call","",xxstring)) < 0)
	  return(z);
	if ((x = cmcfm()) < 0) return(x);
	revcall = z;
	return (success = 1);
    }
}

int
setpadp() {
    if ((y = cmkey(padx3tab,npadx3,"PAD X.3 parameter name","",xxstring)) < 0)
      return(y);
    x = y;
    switch (x) {
      case PAD_BREAK_CHARACTER:
	if ((y = cmnum("PAD break character value","",10,&z,xxstring)) < 0)
	  return(y);
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_ESCAPE:
	if ((y = cmnum("PAD escape","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_ECHO:
 	if ((y = cmnum("PAD echo","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_DATA_FORWARD_CHAR:
 	if ((y = cmnum("PAD data forward char","",10,&z,xxstring)) < 0)
	  return(y);
	if (z != 0 && z != 2) {
	    printf("?The choices are 0 or 2\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_DATA_FORWARD_TIMEOUT:
 	if ((y = cmnum("PAD data forward timeout","",10,&z,xxstring)) < 0)
	    return(y);
	if (z < 0 || z > 255) {
	    printf("?The choices are 0 or 1 <= timeout <= 255\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_FLOW_CONTROL_BY_PAD:
 	if ((y = cmnum("PAD pad flow control","",10,&z,xxstring)) < 0)
	  return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
      case PAD_SUPPRESSION_OF_SIGNALS:
 	if ((y = cmnum("PAD service","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_BREAK_ACTION:
 	if ((y = cmnum("PAD break action","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1 && z != 2 && z != 5 && z != 8 && z != 21) {
	    printf("?The choices are 0, 1, 2, 5, 8 or 21\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_SUPPRESSION_OF_DATA:
 	if ((y = cmnum("PAD data delivery","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_PADDING_AFTER_CR:
 	if ((y = cmnum("PAD crpad","",10,&z,xxstring)) < 0) return(y);
	if (z < 0 || z > 7) {
	    printf("?The choices are 0 or 1 <= crpad <= 7\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_LINE_FOLDING:
 	if ((y = cmnum("PAD linefold","",10,&z,xxstring)) < 0) return(y);
	if (z < 0 || z > 255) {
	    printf("?The choices are 0 or 1 <= linefold <= 255\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_LINE_SPEED:
 	if ((y = cmnum("PAD baudrate","",10,&z,xxstring)) < 0) return(y);
	if (z < 0 || z > 18) {
	    printf("?The choices are 0 <= baudrate <= 18\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_FLOW_CONTROL_BY_USER:
 	if ((y = cmnum("PAD terminal flow control","",10,&z,xxstring)) < 0)
	    return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_LF_AFTER_CR:
 	if ((y = cmnum("PAD crpad","",10,&z,xxstring)) < 0) return(y);
	if (z < 0 || z == 3 || z > 7) {
	    printf("?The choices are 0, 1, 2, 4, 5, 6 or 7\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_PADDING_AFTER_LF:
 	if ((y = cmnum("PAD lfpad","",10,&z,xxstring)) < 0) return(y);
	if (z < 0 || z > 7) {
	    printf("?The choices are 0 or 1 <= lfpad <= 7\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_EDITING:
 	if ((y = cmnum("PAD edit control","",10,&z,xxstring)) < 0) return(y);
	if (z != 0 && z != 1) {
	    printf("?The choices are 0 or 1\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_CHAR_DELETE_CHAR:
 	if ((y = cmnum("PAD char delete char","",10,&z,xxstring)) < 0)
	    return(y);
	if (z < 0 || z > 127) {
	    printf("?The choices are 0 or 1 <= chardelete <= 127\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_BUFFER_DELETE_CHAR:
 	if ((y = cmnum("PAD buffer delete char","",10,&z,xxstring)) < 0)
	    return(y);
	if (z < 0 || z > 127) {
	    printf("?The choices are 0 or 1 <= bufferdelte <= 127\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;

      case PAD_BUFFER_DISPLAY_CHAR:
 	if ((y = cmnum("PAD display line char","",10,&z,xxstring)) < 0)
	    return(y);
	if (z < 0 || z > 127) {
	    printf("?The choices are 0 or 1 <= displayline <= 127\n");
	    return(-2);
	}
	if ((y = cmcfm()) < 0) return(y);
	break;
    }
    padparms[x] = z;
    return(success = 1);
}
#endif /* ANYX25 */ 

int
setat(rmsflg) int rmsflg; {
    int xx;
    if ((y = cmkey(attrtab,natr,"File Attribute packets","",xxstring)) < 0)
      return(y);    
    if (y == AT_XALL) {			/* ATTRIBUTES ALL ON or ALL OFF */
	if ((z = seton(&xx)) < 0) return(z);
	if (rmsflg) {
	    printf("Sorry, command not available\n");
	    return(-9);
	} else {
	    atenci = xx;		/* Encoding in */
	    atenco = xx;		/* Encoding out */
	    atdati = xx;		/* Date in */
	    atdato = xx;		/* Date out */
	    atdisi = xx;		/* Disposition in/out */
	    atdiso = xx;
	    atleni = xx;		/* Length in/out (both kinds) */
	    atleno = xx;
	    atblki = xx;		/* Blocksize in/out */
	    atblko = xx;
	    attypi = xx;		/* File type in/out */
	    attypo = xx;
	    atsidi = xx;		/* System ID in/out */
	    atsido = xx;
	    atsysi = xx;		/* System-dependent params in/out */
	    atsyso = xx;
#ifdef STRATUS
	    atfrmi = xx;		/* Format in/out */
	    atfrmo = xx;
	    atcrei = xx;		/* Creator id in/out */
	    atcreo = xx;
	    atacti = xx;		/* Account in/out */
	    atacto = xx;
#endif /* STRATUS */
	}
	return(z);
    } else if (y == AT_ALLY || y == AT_ALLN) { /* ATTRIBUTES ON or OFF */
	if ((x = cmcfm()) < 0) return(x);
	atcapr = (y == AT_ALLY) ? 1 : 0;
	if (rmsflg) {
	    sstate = setgen('S', "132", atcapr ? "1" : "0", "");
	    return((int) sstate);
	} else return(success = 1);
    }
    /* Otherwise, it's an individual attribute that wants turning off/on */

    if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
    if ((x = cmcfm()) < 0) return(x);

/* There are better ways to do this... */
/* The real problem is that we're not separating the in and out cases */
/* and so we have to arbitrarily pick the "in" case, i.e tell the remote */
/* server to ignore incoming attributes of the specified type, rather */
/* than telling it not to send them.  The protocol does not (yet) define */
/* codes for "in-and-out-at-the-same-time". */

    switch(y) {
      case AT_DISP:
	if (rmsflg) {
	    sstate = setgen('S', "142", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atdisi = atdiso = z; break;
      case AT_ENCO:
	if (rmsflg) {
	    sstate = setgen('S', "141", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atenci = atenco = z; break;
      case AT_DATE:
	if (rmsflg) {
	    sstate = setgen('S', "135", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atdati = atdato = z; break;
      case AT_LENB:
      case AT_LENK:
	if (rmsflg) {
	    sstate = setgen('S', "133", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atleni = atleno = z; break;
      case AT_BLKS:
	if (rmsflg) {
	    sstate = setgen('S', "139", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atblki = atblko = z; break;
      case AT_FTYP:
	if (rmsflg) {
	    sstate = setgen('S', "134", z ? "1" : "0", "");
	    return((int) sstate);
	}
	attypi = attypo = z; break;
#ifdef STRATUS
      case AT_CREA:
	if (rmsflg) {
	    sstate = setgen('S', "136", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atcrei = atcreo = z; break;
      case AT_ACCT:
	if (rmsflg) {
	    sstate = setgen('S', "137", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atacti = atacto = z; break;
#endif /* STRATUS */
      case AT_SYSI:
	if (rmsflg) {
	    sstate = setgen('S', "145", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atsidi = atsido = z; break;
#ifdef STRATUS
      case AT_RECF:
	if (rmsflg) {
	    sstate = setgen('S', "146", z ? "1" : "0", "");
	    return((int) sstate);
	}
        atfrmi = atfrmo = z; break;
#endif /* STRATUS */
      case AT_SYSP:
	if (rmsflg) {
	    sstate = setgen('S', "147", z ? "1" : "0", "");
	    return((int) sstate);
	}
	atsysi = atsyso = z; break;
      default:
	printf("?Not available\n");
	return(-2);
    }
    return(1);
}

#ifndef NOSPL
int
setinp() {
    if ((y = cmkey(inptab,ninp,"","",xxstring)) < 0) return(y);
    switch (y) {
#ifdef OS2
      case IN_PAC:			/* SET INPUT PACING */
	z = cmnum("milliseconds","0",10,&x,xxstring);
	return(setnum(&tt_inpacing,x,z,94));
#endif /* OS2 */
      case IN_DEF:			/* SET INPUT DEFAULT-TIMEOUT */
	z = cmnum("Positive number","",10,&x,xxstring);
	return(setnum(&indef,x,z,94));
      case IN_TIM:			/* SET INPUT TIMEOUT-ACTION */
	if ((z = cmkey(intimt,2,"","",xxstring)) < 0) return(z);
	if ((x = cmcfm()) < 0) return(x);
	intime[cmdlvl] = z;
	return(success = 1);
      case IN_CAS:			/* SET INPUT CASE */
	if ((z = cmkey(incast,2,"","",xxstring)) < 0) return(z);
	if ((x = cmcfm()) < 0) return(x);
	inpcas[cmdlvl] = z;
	return(success = 1);
      case IN_ECH:			/* SET INPUT ECHO */
	return(seton(&inecho));
      case IN_SIL:			/* SET INPUT SILENCE */
	z = cmnum("Seconds of inactivity before INPUT fails","",10,&x,
		  xxstring);
	return(setnum(&insilence,x,z,-1));
      case IN_BUF:			/* SET INPUT BUFFER-SIZE */
	sprintf(tmpbuf, "%d", INPBUFSIZ);
	if ((z = cmnum("Number of bytes in INPUT buffer",
		       tmpbuf,10,&x, xxstring)) < 0)
	  return(z);
	if ((y = cmcfm()) < 0) return(y);
	inbufsize = 0;
	if (inpbuf) {
	    free(inpbuf);
	    inpbuf = NULL;
	    inpbp = NULL;
	}
	if (!(s = (char *)malloc(x + 1)))
	  return(0);
	inpbuf = s;
	inpbp = s;
	inbufsize = x;
	for (x = 0; x <= inbufsize; x++)
	  inpbuf[x] = NUL;
    }
    return(0);
}
#endif /* NOSPL */

#ifdef NETCONN
VOID
ndreset() {
#ifndef NODIAL				/* This depends on DIAL... */
    int i=0, j=0;
    if (!ndinited)			/* Don't free garbage... */
      return;
    for (i = 0; i < nhcount; i++) {	/* Clean out previous list */
	if (nh_p[i]) 
	  free(nh_p[i]);
	nh_p[i] = NULL;
	if (nh_p2[i]) 
	  free(nh_p2[i]);
	nh_p2[i] = NULL;
	for (j = 0; j < 4; j++) {
	    if (nh_px[j][i]) 
	      free(nh_px[j][i]);
	    nh_px[j][i] = NULL;
	}
    }
#endif /* NODIAL */
}

VOID
ndinit() {				/* Net directory pointers */
#ifndef NODIAL				/* This depends on DIAL... */
    int i, j;
    if (ndinited++)			/* Don't do this more than once. */
      return;
    for (i = 0; i < MAXDDIR; i++) {	/* Init all pointers to NULL */
	netdir[i] = NULL;
    }
    for (i = 0; i < MAXDNUMS; i++) {
	nh_p[i] = NULL;
	nh_p2[i] = NULL;
	for (j = 0; j < 4; j++)
	  nh_px[j][i] = NULL;
    }
#endif /* NODIAL */
}

#ifndef NODIAL
#ifdef NETCONN
VOID					/* Get net defaults from environment */
getnetenv() {
    char *p = NULL;

    makestr(&p,getenv("K_NET_DIRECTORY")); /* Dialing directories */
    if (p) {
	int i;
	xwords(p,(MAXDDIR - 2),netdir,0);
	for (i = 0; i < (MAXDDIR - 1); i++) { /* Fill in any gaps... */
	    if (!netdir[i+1])
	      break;
	    else
	      netdir[i] = netdir[i+1];
	}
	nnetdir = i;
    }
}
#endif /* NETCONN */
#endif /* NODIAL */

int	
#ifdef CK_ANSIC
lunet(char *s)				/* s = name to look up   */
#else
lunet(s) char *s;
#endif /* CK_ANSIC */
/* lunet */ {
#ifndef NODIAL				/* This depends on DIAL... */
    int n, n1, n3, n4, t, i, j, k, dd = 0;
    int ambiguous = 0;
    FILE * f;
    char *line = NULL, *pp, *p2;
    extern int dialdpy;
    int netdpy = dialdpy;
    char *info[8];

    nhcount = 0;			/* Set this before returning */

    if (!s || nnetdir < 1)		/* Validate arguments */
      return(-1);

    if (isdigit(*s) || *s == '*' || *s == '.')
      return(0);

    if ((n1 = (int) strlen(s)) < 1)	/* Length of string to look up */
      return(-1);

    if (!(line = malloc(1024)))		/* Allocate input buffer */
      return(-1);

  lu_again:
    f = NULL;				/* Network directory file descriptor */
    t = nhcount = 0;			/* Match count */
    dd = 0;				/* Directory counter */

    dirline = 0;
    while (1) {				/* We make one pass */
	if (!f) {			/* Directory not open */
            if (dd >= nnetdir)		/* No directories left? */
	      break;			/* Done. */
	    if ((f = fopen(netdir[dd],"r")) == NULL) { /* Open it */
		perror(netdir[dd]);	/* Can't, print message saying why */
		dd++;
		continue;		/* But go on to next one. */
	    }
	    if (netdpy)
	      printf("Opening %s...\n",netdir[dd]);
            dd++;
	}
	line[0] = NUL;
	if (getnct(line,1023,f,1) < 0) { /* Read a line */
	    if (f) {			/* f can be clobbered! */
		fclose(f);		/* Close the file */
		f = NULL;		/* Indicate next one needs opening */
	    }
	    continue;
	}
	if (!line[0])			/* Empty line */
	  continue;

	xwords(line,7,info,0);		/* Parse it */
	
	if (!info[1] || !info[2] || !info[3]) /* Required fields */
	  continue;
	if (*info[1] == ';')		/* Full-line comment */
	  continue;
	if ((n = (int) strlen(info[1])) < 1) /* Length of name-tag */
	  continue;
	if (n < n1)			/* Search name is longer */
	  continue;			/* Can't possibly match */
	if (ambiguous && n != n1)
	  continue;
	if (xxstrcmp(s,info[1],n1))	/* Compare using length of */
	  continue;			/* search string s. */

	/* Have a match */

	makestr(&(nh_p[nhcount]), info[3]);    /* address */
	makestr(&(nh_p2[nhcount]),info[2]);    /* net type */
	makestr(&(nh_px[0][nhcount]),info[4]); /* net-specific stuff... */
	makestr(&(nh_px[1][nhcount]),info[5]);
	makestr(&(nh_px[2][nhcount]),info[6]);
	makestr(&(nh_px[3][nhcount]),info[7]);

	nhcount++;			/* Count this match */
	if (nhcount > MAXDNUMS) {	/* Watch out for too many */
	    printf("Warning: %d matches found, %d max\n",
		   nhcount,
		   MAXDNUMS
		   );
	    nhcount = MAXDNUMS;
	    break;
	}
	if (nhcount == 1) {		/* First one - save entry name */
	    if (n_name) {		/* Free the one from before if any */
		free(n_name);
		n_name = NULL;
	    }
	    if (!(n_name = (char *)malloc(n + 1))) { /* Allocate new storage */
		printf("?memory allocation error - lunet:3\n");
		if (line) {
		    free(line);
		    line = NULL;
		}
		nhcount = 0;
		return(-1);
	    }
	    t = n;			/* Remember its length */
	    strcpy(n_name,info[1]);
	} else {			/* Second or subsequent one */
	    if ((int) strlen(info[1]) == t) /* Lengths compare */
	      if (!xxstrcmp(n_name,info[1],t)) /* Caseless compare OK */
		continue;

	    /* Name given by user matches entries with different names */
	    
	    if (ambiguous)		/* Been here before */
	      break;

	    ambiguous = 1;		/* Now an exact match is required */
	    ndreset();			/* Clear out previous list */
	    goto lu_again;		/* Do it all over again. */
	}
    }
    if (line) {
	free(line);
	line = NULL;
    }
    if (nhcount == 0 && ambiguous)
      printf("Ambiguous - \"%s\" different names in network directory\n",s);
#else
    nhcount = 0;
#endif /* NODIAL */
    return(nhcount);
}
#endif /* NETCONN */

#ifndef NOLOCAL
/* S E T L I N -- parse name of and then open communication device. */
/*
  Call with:
    xx == XYLINE for a serial (tty) line, XYHOST for a network host,
    zz == 0 means if user doesn't give a device name, continue current
            active connection (if any);
    zz != 0 means if user doesn't give a device name, then close the
            current connection and restore the default communication device.
*/
int
setlin(xx, zz) int xx, zz; {
    int i;
    char * ss;

#ifdef OS2
#define SRVBUFSIZ PIPENAML
#else
#define SRVBUFSIZ 63
#endif /* OS2 */

    char srvbuf[SRVBUFSIZ+1];
    char * service;

#ifdef OS2
#ifdef NT
    int xxtapi = 0;
#else
    int xxslip = 0, xxppp = 0;
#endif /* NT */
#endif /* OS2 */

    service = srvbuf;
    *service = NUL;

    if (xx == XYHOST) {			/* SET HOST <hostname> */
#ifndef NETCONN
        printf("?Network connections not supported\n");
	return(-9);
#else
	if (
#ifdef DECNET
	    (nettype != NET_DEC) &&
#endif /* DECNET */
	    (nettype != NET_SX25) &&
	    (nettype != NET_VX25) &&
#ifdef NPIPE
	    (nettype != NET_PIPE) &&
#endif /* NPIPE */
#ifdef CK_NETBIOS
	    (nettype != NET_BIOS) &&
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
	    (nettype != NET_SLAT) &&
#endif /* SUPERLAT */
#ifdef NETFILE
	    (nettype != NET_FILE) &&
#endif /* NETFILE */
	    (nettype != NET_TCPB)) {
	    printf("?Network type not supported\n");
	    return(-9);
	}
#ifdef CK_NETBIOS
	if (nettype == NET_BIOS) {
	    if ((x = cmtxt( zz ? 
    "server name, *,\n or carriage return to close an open connection" :
    "server name, *,\n or carriage return to resume an open connection",
			   "",&s,xxstring)) < 0)
	      return(x);
	    strcpy(line,s);
	    s = line;
	} else
#endif /* CK_NETBIOS */
#ifdef NPIPE
	  if (nettype == NET_PIPE) {
	      if ((x = cmtxt( zz ? 
    "server name, *,\n or carriage return to close an open connection" :
    "server name, *,\n or carriage return to resume an open connection",
			    "",&s,xxstring)) < 0)
		return(x);
	      if (*s != '\0') {
		  if (strcmp(s,"*")) {	/* If remote, */
		      strcpy(line,"\\\\"); /* begin with server name */
		      strcat(line,s);
		  } else {
		      line[0]='\0';
		  }	
		  strcat(line,"\\pipe\\"); /* Make this a pipe name */
		  strcat(line,pipename);   /* Add the name of the pipe */
		  s = line;
	      }
	  } else	
#endif /* NPIPE */
#ifdef NETFILE
	    if (nettype == NET_FILE) {
		if ((x = cmtxt( zz ? 
    "file name,\n or carriage return to close an open connection" :
    "file name,\n or carriage return to resume an open connection",
			       "",&s,xxstring)) < 0)
		  return(x);
		if (*s != '\0')
		  strcpy(line,s);
		s = line;
	    } else
#endif /* NETFILE */
#ifdef SUPERLAT
	      if (nettype == NET_SLAT) {
		  slat_pwd[0] = NUL;	/* Erase any previous one */

		  /* Just get a text string */

		  if ((x = cmfld( zz ?
   "service, node/port,\n or carriage return to close an open connection" :
   "service, node/port,\n or carriage return to resume an open connection",
				 "",&s,xxstring)) < 0) {
		      if (x != -3)	/* Parse error */
			return(x);	/* return it */
		      else if (!network) {
			  printf("?Host name or address required\n");
			  return(-9);
		      } else if (!zz)	/* No hostname given */
			return(1);	/* and none required, */
		  } /* continue current connection. */
		  if (*s) {		/* If they gave a host name... */
		      strcpy(line,s);
		      if ((x = cmfld(
		  "password,\n or carriage return if no password required",
				     "",
				     &s,
				     xxstring
				     )) < 0 && x != -3)
			return(x);
		      strncpy(slat_pwd,s,17); /* Set the password, if any */
		  }
		  if ((x = cmcfm()) < 0) return(x); /* Confirm the command */
		  s = line;
	      } else
#endif /* SUPERLAT */
		if (nettype != NET_TCPB) { /* Not a TCP/IP connection */
		    /* Just get a text string */
		    if ((x = cmtxt(zz ?
    "Network host name,\n or carriage return to close an open connection" :
    "Network host name,\n or carriage return to resume an open connection",
				   "",&s,xxstring)) < 0)
		      return(x);
		    strcpy(line,s);
		    s = line;
		} else {		/* TCP/IP connection... */
		    /* Parse for host and service separately. */
#ifndef OS2	      
		    ss =  zz ?
 "IP host name or number,\n or carriage return to close an open connection" :
 "IP host name or number,\n or carriage return to resume an open connection";
#else /* OS2 */
		    if (ttnproto != NP_RLOGIN && ttnproto != NP_TELNET) {
			ss = zz ?
 "IP host name, number, *,\n or carriage return to close an open connection" :
 "IP host name, number, *,\n or carriage return to resume an open connection";
		    } else {
			ss = zz ?
 "IP host name or number,\n or carriage return to close an open connection" :
 "IP host name or number,\n or carriage return to resume an open connection";
		    }
#endif /* OS2 */
		    if ((x = cmfld(ss,"",&s,xxstring)) < 0) {
			if (x != -3)	/* Parse error */
			  return(x);	/* return it */
			else if (!network) {
			    printf("?Host name or address required\n");
			    return(-9);
			} else if (!zz)	/* No hostname given */
			  return(1);	/* and none required, */
		    } /* continue current connection. */
		    if (*s) {		/* If they gave a host name... */
			debug(F110,"setlin host s 1",s,0);
#ifdef NOLISTEN		  
			if (nettype == NET_TCPB && *s == '*') {
			    printf(
"?Sorry, incoming connections not supported in this version of Kermit.\n"
				   );
			    return(-9);
			}
#endif /* NOLISTEN */
			strcpy(line,s); /* Make a copy */
			s = line;
			debug(F110,"setlin host line[]",line,0);
#ifdef RLOGCODE	
			/* Allow a username if rlogin is requested */
			if (ttnproto == NP_RLOGIN) {
			    int y;
			    uidflag = 0;
#ifdef VMS	
			    strcpy(service,"513"); /* Set service to login */
#else /* VMS */
#ifdef OS2_LOGIN_IS_513
			    strcpy(service,"513"); 
#else /* OS2 */
			    strcpy(service,"login");
#endif /* OS2 */
#endif /* VMS */
			    y = cmfld("Userid on remote system",
				      uidbuf,&s,xxstring);
			    if (y < 0 && y != -3)
			      return(y);
			    if ((int)strlen(s) > 63) {
				printf("Sorry, too long\n");
				return(-9);
			    }
			    strcpy(uidbuf,s);
			    if (uidbuf[0])
			      uidflag = 1;
			} else {
#endif /* RLOGCODE */
			    s = line;	/* TELNET or SET HOST */
			    /* Check for "host:service" */
			    for ( ; (*s != '\0') && (*s != ':'); s++) ;
			    if (*s) {	/* Service, save it */
				*s++ = NUL;
				strncpy(service,s,SRVBUFSIZ);
				service[SRVBUFSIZ] = NUL;
			    } else {	/* No :service, let them type one. */
				if (*line != '*') {
				    if (ttnproto == NP_KERMIT) {
					if ((x = cmfld(
					       "TCP service name or number",
						       "",&s,xxstring)
					     ) < 0 && x != -3)
					  return(x);
				    } else if (ttnproto == NP_RLOGIN) {
					if ((x = cmfld(
       "TCP service name or number,\n or carriage return for rlogin (513)",
       "513",&s,xxstring)
					     ) < 0 && x != -3)
					  return(x);
				    } else {
					if ((x = cmfld(
					     "TCP service name or number",
						       "",&s,xxstring)
					     ) < 0 && x != -3)
					  return(x);
				    }
				} else { /* Not incoming */
				    if ((x = cmfld(
						"TCP service name or number",
 						   "",&s,xxstring)
					 ) < 0 && x != -3)
				      return(x);
				}
				if (*s) { /* If they gave a service, */
				    strncpy(srvbuf,s,SRVBUFSIZ); /* copy it */
				} 
			    }
#ifdef RLOGCODE
			}
#endif /* RLOGCODE */
			if ((x = cmcfm()) < 0) /* Confirm the command */
			  return(x);
		    } else {
			line[0] = NUL;
		    }
		}

	s = line;
	debug(F110,"setlin service 0",srvbuf,0);
	debug(F110,"setlin host s 2",s,0);

#ifndef NODIAL
	/* Look up in network directory */
	x = 0;
	debug(F101,"setlin nettype 1","",nettype);
	debug(F111,"setlin nnetdir",s,nnetdir);
	if (*s == '=') {		/* If number starts with = sign */
	    s++;			/* strip it */
	    while (*s == SP) s++;	/* and any leading spaces */
	    strcpy(line,s);		/* Do this again. */
	    s = line;
	    debug(F110,"setlin host s 3",s,0);
	    nhcount = 0;
	} else if (*s) {		/* We want to look it up. */
	    if (nnetdir > 0)		/* If there is a directory... */
	      x = lunet(line);		/* (sets nhcount) */
	    else			/* otherwise */
	      nhcount = 0;		/* we didn't find any there */
	    if (x < 0) {		/* Internal error? */
		printf("?Fatal network directory lookup error - %s\n",line);
		return(-9);
	    }
	    debug(F111,"setlin lunet nhcount",line,nhcount);
	}
#endif /* NODIAL */

	/* New connection wanted.  Make a copy of the host name/address... */

	strncpy(tmpbuf,s,TMPBUFSIZ);	/* Because we are reusing line[] */
	s = tmpbuf;			/* for net directory entries...  */
	debug(F110,"setlin host s 5",s,0);
	if (!hupok(1))
	  return(success = 0);

	ttflui();			/* Clear away buffered up junk */
#ifndef NODIAL
	mdmhup();
#endif /* NODIAL */
	ttclos(0);			/* Close old connection, if any */
	if (oldplex > -1)		/* Restore duplex setting. */
	  duplex = oldplex;
	if (*s) {			/* They gave a hostname */
	    x = 1;			/* Network connection always local */
	    if (mdmsav < 0)
	      mdmsav = mdmtyp;		/* Remember old modem type */
	    mdmtyp = -nettype;		/* Special code for network */
	    if (nettype == NET_TCPB) {	/* For TCP/IP telnet connections */
		oldplex = duplex;	/* Remember previous duplex */
		duplex = 0;		/* Set full duplex and let */
		                        /* negotiations change if necessary. */
	    }
	} else {			/* They just said "set host" */
	    if (network && msgflg)
	      printf(" Closing connection\n");
	    s = dftty;			/* So go back to normal */
	    x = dfloc;			/* default tty, location, */
	    network = 0;		/* No more network connection. */
	    if (oldplex > -1)
	      duplex = oldplex;		/* Restore old duplex setting. */
	    if (mdmtyp < 0) {		/* Switching from net to serial? */
		if (mdmsav > -1) {	/* Restore modem type from last */
		    mdmtyp = mdmsav;	/* SET MODEM command, if any. */
		    mdmsav = -1;
		} else
		  mdmtyp = 0;
	    }
	    if (zz) {
		if (autoflow)		/* Maybe change flow control */
		  setflow();
		return(success = 1);
	    }
	}
#endif /* NETCONN */
    }

/* Serial tty device, possibly modem, connection... */

    if (xx == XYLINE			/* SET LINE or SET PORT */
#ifdef CK_TAPI
	|| xx == XYTAPI_LIN		/* SET TAPI LINE */
#endif /* CK_TAPI */
	) {
#ifdef OS2			
/*
  User can type:
    COM1..COM8 = Regular COM port
    1..8       = Synonym for COM1..COM8, is translated to COM1..COM8
    _n         = (n is a number) = open file handle
    string     = any text string = name of some other kind of device,
                 taken literally, as given.
*/
	s = "Communication device name";
#ifdef CK_TAPI
	if (TAPIAvail)
          cktapiBuildLineTable(&tapilinetab, &ntapiline);
	if (tapilinetab && ntapiline > 0)
	  s = "Communication device name, or \"TAPI\"";
	else if ( xx == XYTAPI_LIN ) {
	    printf("\nNo TAPI Line Devices are configured for this system\n");
	    return(-9);
	}
	if (xx == XYTAPI_LIN)
          s = "tapi";
	else  /* Query the user */
#endif /* CK_TAPI */
	  if ((x = cmfld(s,dftty,&s,xxstring)) < 0)
	    return(x);
	debug(F110,"OS2 SET PORT s",s,0);
	y = lookup(os2devtab,s,nos2dev,&x); /* Look up in keyword table */
	debug(F101,"OS2 SET PORT x","",x);
	debug(F101,"OS2 SET PORT y","",y);
	if ((y > -1) && (x >= 0 && x < 8)) { /* User typed a digit 1..8 */
	    s = os2devtab[x+8].kwd;	/* Substitite its real name */
#ifdef NT
	    xxtapi = 0;
#else /* NT */
	    xxslip = xxppp = 0;
#endif /* NT */
	    debug(F110,"OS2 SET PORT subst s",s,"");
#ifndef NT
	} else if ((y >-1) && (x >= 16 && x < 24)) { /* SLIP access */
	    s = os2devtab[x-8].kwd;	/* Substitite its real name */
	    debug(F110,"OS2 SET PORT SLIP subst s",s,"");
	    xxslip = 1;
	    xxppp  = 0;
	} else if ((y >-1) && (x >= 24 && x < 32)) { /* PPP access */
	    s = os2devtab[x-16].kwd;	/* Substitite its real name */
	    debug(F110,"OS2 SET PORT PPP subst s",s,"");
	    xxppp = 1;
	    xxslip = 0;
	    if ((y = cmkey(os2ppptab,
			   nos2ppp,
			   "PPP driver interface",
			   "ppp0",
			   xxstring)
		 ) < 0)
		return(y);
	    debug(F101,"OS2 SET PORT PPP INTERFACE y","",y);
	    xxppp = (y % 10) + 1;
#endif /* NT */
	} else if (*s == '_') {		/* User used "_" prefix */
	    s++;			/* Remove it */
	    /* Rest must be numeric */
	    debug(F110,"OS2 SET PORT HANDLE _subst s",s,0);
	    if (!rdigits(s)) {
		printf("?Invalid format for file handle\n");
		return(-9);
	    }
#ifdef NT
	    xxtapi = 0;
#else /* NT */
	    xxslip = xxppp = 0;
#endif /* NT */
	} else {			/* A normal COMx port or a string */
	    s = brstrip(s);		/* Strip braces if any */
#ifdef NT
	    /* Windows TAPI support - Look up in keyword table */
	    if (tapilinetab && ntapiline > 0) {
		if (!xxstrcmp(s,"tapi",4)) {

		    /* Find out what the lowest numbered TAPI device is */
		    /* and use it as the default.                       */
		    int j = 9999, k = -1;
		    for (i = 0; i < ntapiline; i++ ) {
		    	if (tapilinetab[i].kwval < j) {
			    j = tapilinetab[i].kwval;
			    k = i;
			}
		    }		
		    if (k >= 0)
		      s = tapilinetab[k].kwd;
		    else s = "";

		    if ((y = cmkey(tapilinetab,ntapiline,
				   "TAPI device name",s,xxstring)) < 0)
		      return(y);

		    xxtapi = 1;
#ifdef COMMENT
		    y = lookup(tapilinetab,s,ntapiline,&x);
#endif /* COMMENT */
		    if (y > -1)
		      s = tapilinetab[y].kwd;
		} else
		  xxtapi = 0;
	    }
#else /* NT */
	    /* not OS/2 SLIP or PPP */
	    xxslip = xxppp = 0;
#endif /* NT */
        }
	if ((x = cmcfm()) < 0)
	  return(x);

#else /* !OS2 */

	if ((x = cmtxt("Communication device name",dftty,&s,xxstring)) < 0)
	  return(x);
#endif /* OS2 */

	strcpy(tmpbuf,s);
	s = tmpbuf;
	if (!hupok(1))
	  return(success = 0);

	if (local) ttflui();		/* Clear away buffered up junk */
#ifndef NODIAL
#ifdef OS2ONLY
/* Don't hangup a line that is shared with the SLIP or PPP driver */
	if (!ttslip && !ttppp)
#endif /* OS2ONLY */
	mdmhup();
#endif /* NODIAL */
	ttclos(0);			/* Close old line, if any was open */
	if (*s) {			/* They gave a device name */
	    x = -1;			/* Let ttopen decide about it */
	} else {			/* They just said "set line" */
	    s = dftty;			/* so go back to normal tty */
	    x = dfloc;			/* and mode. */
	}

#ifdef OS2				/* Must wait until after ttclos() */
#ifdef NT				/* to change these settings       */
	tttapi = xxtapi;
#else
	ttslip = xxslip;
	ttppp  = xxppp;
#endif /* NT */
	debug(F110,"OS2 SET PORT final s",s,"");
#endif /* OS2 */

#ifdef NETCONN
	if (mdmtyp < 0) {		/* Switching from net to async? */
	    if (mdmsav > -1)		/* Restore modem type from last */
	      mdmtyp = mdmsav;		/* SET MODEM command, if any. */
	    else
	      mdmtyp = 0;
	    mdmsav = -1;
	}
	if (oldplex > -1) {		/* Restore previous duplex setting. */
	    duplex = oldplex;
	    oldplex = -1;
	}
	if (network) {
#ifdef TNCODE
/* This should be unnecessary, since ttclos() did it already? */
/* But it can't hurt ... */
	    tn_init = 0;		/* TELNET not init'd any more. */
#endif /* TNCODE */
	    network = 0;		/* No more network. */
	}
#endif /* NETCONN */
    }
#ifdef COMMENT
/*
  The following is removed, not so much because it's a horrible hack, but
  because it only works if the SET FLOW command was given *before* the SET
  LINE command, whereas normally these commands can be given in any order.
*/
#ifdef NEXT
/*
  This is a horrible hack, but it's nice for users.  On the NeXT, you select
  RTS/CTS hardware flow control not by system calls, but by referring to the
  device with a different name.  If the user has asked for RTS/CTS flow
  control on a NeXT, but used the non-RTS/CTS device name in the SET LINE
  command, we make the appropriate substitute here.  I wonder how much bigger
  this section of code will grow as the years go by... 
*/
    if ((flow == FLO_RTSC) &&		/* RTS/CTS flow control selected */
	strcmp(s,dftty)) {		/*  ...on external port? */
	y = strlen(s);			/* Yes, insert "f" as next-to-last */
	if (s[y-2] != 'f') {		/* character in device name if not */
	    strcpy(line,s);		/* already there... */
	    line[y] = line[y-1];	/* So /dev/cua => /dev/cufa, etc. */
	    line[y-1] = 'f';
	    line[y+1] = '\0';
	    s = line;
	}
    }
#endif /* NEXT */
#endif /* COMMENT */

    success = 0;
    if (mdmtyp > -1) {			/* Serial connections... */
	if ((y = ttopen(s,&x,mdmtyp,cdtimo)) > -1 ) { /* Open the new line */
	    success = 1;
	} else {
#ifdef OS2ONLY
	    if (!strcmp(s,dftty))   /* Do not generate an error with dftty */
	      ;
	    else if (y == -6 && ttslip) {
		printf("Unable to access SLIP driver.\n");
	    } else if (y == -6 && ttppp) {
		printf("Unable to access PPP driver (wrong interface?)\n");
	    } else
#endif /* OS2ONLY */
	      if (y == -2) {
		  printf("?Timed out, no carrier.\n");
		  printf("Try SET CARRIER OFF and SET LINE again, or else\n");
		  printf("SET MODEM, SET LINE, and then DIAL.\n");
	      } else if (y == -3) {
		  printf("Sorry, access to lock denied: %s\n",s);
	      } else if (y == -4) {
		  printf("Sorry, access to device denied: %s\n",s);
	      } else if (y == -5) {
#ifdef VMS
		  printf(
		   "Sorry, device is in use or otherwise unavailable: %s\n",s);
#else
		  printf("Sorry, device is in use: %s\n",s);
#endif /* VMS */
	      } else {			/* Other error. */
#ifdef COMMENT
#ifndef VMS
#endif /* COMMENT */
		  if (errno) {
		      int x;		/* Find a safe, long buffer */
		      x = strlen(line) + 2; /* for the error message. */
		      if (LINBUFSIZ - x > 100) { /* Allow room for 100 chars */
			  tp = line + x;
			  sprintf(tp,"Sorry, can't open connection: %s",s);
			  perror(tp);
		      } else printf("Sorry, can't open connection: %s\n",s);
		  } else
#ifdef COMMENT
#endif /* VMS */
#endif /* COMMENT */
		    printf("Sorry, can't open connection: %s\n",s);
	      }
	}
    }
#ifdef NETCONN
    else {				/* Network connection */
	char *p;
	int i, n;
#ifndef NODIAL
	debug(F101,"setlin nettype 3","",nettype);
	debug(F101,"setlin nhcount","",nhcount);
	if ((nhcount > 1) && !quiet && !backgrd) {
	    int k;
	    printf("%d entr%s found for \"%s\"%s\n",
		   nhcount,
		   (nhcount == 1) ? "y" : "ies",
		   s,
		   (nhcount > 0) ? ":" : "."
		   );
	    for (i = 0; i < nhcount; i++) {
		    printf("%3d. %-12s => %-9s %s",
			   i+1,n_name,nh_p2[i],nh_p[i]);
		for (k = 0; k < 4; k++) { /* Also list net-specific items */
		    if (nh_px[k][i])	  /* free format... */
		      printf(" %s",nh_px[k][i]);
		    else
		      break;
		}
		printf("\n");
	    }
	}
	if (nhcount == 0)
	  n = 1;
	else
	  n = nhcount;
#else
	n = 1;
	nhcount = 0;
#endif /* NODIAL */
	for (i = 0; i < n; i++) {	/* Loop for each entry found */
#ifndef NODIAL
	    debug(F101,"setlin loop i","",i);
	    if (nhcount > 0) {		/* If we found at least one entry... */
		strcpy(line,nh_p[i]);	/* Copy the current entry to line[] */
		if (lookup(netcmd,nh_p2[i],nnets,&x) > -1) { /* Net type */
		    nettype = netcmd[x].kwval;
		    mdmtyp  = 0 - nettype;
		    debug(F101,"setlin nettype 4","",nettype);
		} else {
		    printf("Error - network type \"%s\" not supported\n",
			   nh_p2[i]
			   );
		    continue;
		}		    

		switch (nettype) {	/* Net-specific things */

		  case NET_TCPB: {	/* TCP/IP */
#ifdef TCPSOCKET
		      char *q;
		      int flag = 0;

		      /* Extract ":service", if any, from host string */
		      debug(F110,"setlin service 1",line,0);
		      for (q = line; (*q != '\0') && (*q != ':'); q++)
			;
		      if (*q == ':') { *q++ == NUL; flag = 1; }
		      debug(F111,"setlin service 2",line,flag);

		      /* Get service, if any, from directory entry */

		      if (!*service) {
			  if (nh_px[0][i]) {
			      strncpy(srvbuf,nh_px[0][i],SRVBUFSIZ);
			      debug(F110,"setlin service 3",service,0);
			  }			  
			  if (flag) {
			      strncpy(srvbuf,q,SRVBUFSIZ);
			      debug(F110,"setlin service 4",service,0);
			  }			  
		      }

		      /* If we have a service, append to host name/address */

		      if (*service) {
			  strcat(line, ":"); /* separated by a colon. */
			  strncat(line, service, LINBUFSIZ);
			  debug(F110,"setlin service 5",line,0);
		      }
#ifdef RLOGCODE
		      /* If no service given but command was RLOGIN */

		      else if (ttnproto == NP_RLOGIN) {	/* add this... */
			  strcat(line, ":login");
			  debug(F110,"setlin service 6",line,0);
		      }
#endif /* RLOGCODE */
		      else {		/* Otherwise, add ":telnet". */
			  strcat(line, ":telnet");
			  debug(F110,"setlin service 7",line,0);
		      }       

		      /* Fifth field, if any, is user ID (for rlogin) */

		      if (nh_px[1][i] && !uidflag)
			strncpy(uidbuf,nh_px[1][i],63);
#ifdef RLOGCODE
		      if (ttnproto == NP_RLOGIN && !uidbuf[0]) {
			  printf("Username required for rlogin\n");
			  return(-9);
		      }
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
		      break;
		  }


		  case NET_PIPE:
#ifdef NPIPE
		    if (!pipename[0]) {	/* User didn't give a pipename */
			if (nh_px[0][i]) { /* But directory entry has one */
			    if (strcmp(pipename,"\\pipe\\")) {
				strcpy(pipename,"\\pipe\\");
				strncat(srvbuf,nh_px[0][i],PIPENAML-6);
			    } else {
				strncpy(pipename,nh_px[0][i],PIPENAML);
			    }
			    debug(F110,"setlin pipeneme",pipename,0);
			}
		    }
#endif /* NPIPE */
		    break;

		  case NET_SLAT:
#ifdef SUPERLAT
		    if (!slat_pwd[0]) {	/* User didn't give a password */
			if (nh_px[0][i]) { /* But directory entry has one */
			    strncpy(slat_pwd,nh_px[0][i],17);
			    debug(F110,"setlin SuperLAT password",slat_pwd,0);
			}
		    }
#endif /* SUPERLAT */
		    break;

		  case NET_SX25:	/* X.25 keyword parameters */
		  case NET_VX25: {
#ifdef ANYX25		    
		      int k;		/* Cycle through the four fields */
		      for (k = 0; k < 4; k++) {
			  if (!nh_px[k][i]) /* Bail out if none left */
			    break;
			  if (!xxstrcmp(nh_px[k][i],"cug=",4)) {
			      closgr = atoi(nh_px[k][i]+4);
			      debug(F101,"X25 CUG","",closgr);
			  } else if (!xxstrcmp(nh_px[k][i],"cud=",4)) {
			      cudata = 1;
			      strncpy(udata,nh_px[k][i]+4,MAXCUDATA);
			      debug(F110,"X25 CUD",cudata,0);
			  } else if (!xxstrcmp(nh_px[k][i],"rev=",4)) {
			      revcall = !xxstrcmp(nh_px[k][i]+4,"=on",3);
			      debug(F101,"X25 REV","",revcall);
			  } else if (!xxstrcmp(nh_px[k][i],"pad=",4)) {
			      int x3par, x3val;
			      char *s1, *s2;
			      s1 = s2 = nh_px[k][i]+4; /* PAD parameters */
			      while (*s2) {            /* Pick them apart */
				  if (*s2 == ':') {
				      *s2 = NUL;
				      x3par = atoi(s1);
				      s1 = ++s2;
				      continue;
				  } else if (*s2 == ',') {
				      *s2 = NUL;
				      x3val = atoi(s1);
				      s1 = ++s2;
				      debug(F111,"X25 PAD",x3par,x3val);
				      if (x3par > -1 &&
					  x3par <= MAXPADPARMS)
					padparms[x3par] = x3val;
				      continue;
				  } else
				    s2++;
			      }
			  }
		      }
#endif /* ANYX25 */
		      break;
		  }

		  default:		/* Nothing special for other nets */
		    break;
		}
	    } else {			/* No directory entries found. */
		strcpy(line,tmpbuf);	/* Put this back... */
		if (nettype == NET_TCPB)
		  if (service)		/* If user gave a service */
		    if (*service) {	/* append it to host name/address */
			strcat(line, ":");
			strcat(line, service);
		    }
	    }
#endif /* NODIAL */
	    /*
	       Get here with host name/address and all net-specific
	       parameters set, ready to open the connection.
	    */
	    mdmtyp = -nettype;		/* This should have been done */
					/* already but just in case ... */

	    debug(F110,"setlin net line[] before ttopen",line,0);
	    debug(F101,"setlin net mdmtyp before ttopen",line,mdmtyp);

	    if ((y = ttopen(line, &x, mdmtyp, 0 )) < 0) { /* Try to open */
#ifndef VMS
		if (errno) {
		    int x;
		    debug(F111,"set host line, errno","",errno);
		    x = strlen(line) + 2;
		    if (LINBUFSIZ - x > 100) {
			tp = line + x;
			sprintf(tp,"Can't connect to %s",line);
			perror(tp);
		    } else printf("Can't connect to %s\n",line);
		} else
#endif /* VMS */
		  printf("Can't open connection to %s\n",line);
		continue;
	    } else {
		success = 1;
		break;
	    }
	} /* for-loop */
	s = line;
    } /* network connection */
#endif /* NETCONN */
/*
  NOTE:
  The following will fail if Kermit is running as a daemon with no
  controlling tty.  Needs research.
*/
    if (!success) {
	local = dfloc;			/* Go back to normal */
#ifndef MAC
	strcpy(ttname,dftty);		/* Restore default tty name */
#endif /* MAC */
	speed = ttgspd();
	network = 0;			/* No network connection active */
	return(0);			/* Return failure */
    }
    if (x > -1) local = x;		/* Opened ok, set local/remote. */
    network = (mdmtyp < 0);		/* Remember connection type. */
    strcpy(ttname,s);			/* Copy name into real place. */
    speed = ttgspd();			/* Get the current speed. */
    debug(F111,"set line ",ttname,local);
#ifdef NETCONN
    if (network)
#ifdef CK_SPEED
      /* Force prefixing of 255 on TCP/IP connections... */
      if (nettype == NET_TCPB) {
	  ctlp[255] = 1;
      } else
#endif /* CK_SPEED */
	if (nettype == NET_SX25 || nettype == NET_VX25)
	  duplex = 1;			/* Local echo for X.25 */
#endif /* NETCONN */
#ifdef OS2
    if (mdmtyp <= 0)			/* Network or Direct Connection */
      DialerSend(OPT_KERMIT_CONNECT, 0);
#endif /* OS2 */
    if (autoflow)			/* Maybe change flow control */
      setflow();
    return(success = 1);
}
#endif /* NOLOCAL */

#ifndef NOSETKEY
/* Save Key maps and in OS/2 Mouse maps */
int
savkeys(name,disp) char * name; int disp; {
    char *tp;
    static struct filinfo xx;
    int savfil, i, j, k;
    char buf[1024];

    zclose(ZMFILE);

    if (disp) {
	xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
	xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
	xx.lblopts = 0;
	savfil = zopeno(ZMFILE,name,NULL,&xx);
    } else savfil = zopeno(ZMFILE,name,NULL,NULL);

    if (savfil) {
	ztime(&tp);
	zsout(ZMFILE, "; C-Kermit SAVE KEYMAP file: ");
	zsoutl(ZMFILE,tp);
#ifdef OS2
	if (mskkeys) {
	    zsoutl(ZMFILE,
	 "if eq \"\\v(program)\" \"C-Kermit\" set mskermit keycodes on");
	} else {
	    zsoutl(ZMFILE,
         "if NOT eq \"\\v(program)\" \"C-Kermit\" stop 1 C-Kermit required."); 
	    zsoutl(ZMFILE,"set mskermit keycodes off");
	}
	zsoutl(ZMFILE,"");
#endif /* OS2 */

	zsoutl(ZMFILE,"; Clear previous keyboard mappings ");
	zsoutl(ZMFILE,"set key clear");
#ifdef OS2
	for (k = 0; k < nttkey; k++) {
	    if (!ttkeytab[k].flgs) {
		sprintf(buf, "set terminal key %s clear", ttkeytab[k].kwd);
		zsoutl(ZMFILE,buf);
	    }
	}
#endif /* OS2 */
	zsoutl(ZMFILE,"");

	for (i = 0; i < KMSIZE; i++) {
	    if (macrotab[i]) {
		int len = strlen((char *)macrotab[i]);
#ifdef OS2
		sprintf(buf,"set key \\%d ",mskkeys ? cktomsk(i) : i);
#else /* OS2 */
		sprintf(buf,"set key \\%d ",i);
#endif /* OS2 */
		zsout(ZMFILE,buf);

		for (j = 0; j < len; j++) {
		    char ch = macrotab[i][j];
		    if (ch <= SP || ch >= DEL || 
			 ch == '-' || ch == ',') {
			sprintf(buf, "\\{%d}", ch);
			zsout(ZMFILE,buf);
		    } else {
			sprintf(buf, "%c", ch);
			zsout(ZMFILE,buf);
		    }
		}
#ifdef OS2
		sprintf(buf, "\t; %s", keyname(i));
		zsoutl(ZMFILE,buf);
#else
		zsoutl(ZMFILE,"");
#endif /* OS2 */
	    } else if ( keymap[i] != i ) {
#ifndef NOKVERBS
		if (IS_KVERB(keymap[i])) {
		    for (j = 0; j < nkverbs; j++)
		      if (kverbs[j].kwval == (keymap[i] & ~F_KVERB))
			break;
		    if (j != nkverbs) {
#ifdef OS2
			sprintf(buf, "set key \\%d \\K%s\t; %s", 
				mskkeys ? cktomsk(i) : i, 
				kverbs[j].kwd, keyname(i)
				);
			zsoutl(ZMFILE,buf);
#else
			sprintf(buf, "set key \\%d \\K%s", i, kverbs[j].kwd);
			zsoutl(ZMFILE,buf);
#endif
		    }
		} else 
#endif /* NOKVERBS */
		  {
#ifdef OS2
		      sprintf(buf, "set key \\%d \\{%d}\t; %s", 
			      mskkeys ? cktomsk(i) : i, 
			      keymap[i],
			      keyname(i)
			      );
		      zsoutl(ZMFILE,buf);
#else
		      sprintf(buf, "set key \\%d \\{%d}", i, keymap[i]);
		      zsoutl(ZMFILE,buf);
#endif /* OS2 */
		  }
	    }
	}
#ifdef OS2
	/* OS/2 also has the SET TERMINAL KEY <termtype> defines */
	for (k = 0; k < nttkey; k++) {
	    extern struct keynode * ttkeymap[];
	    struct keynode * pnode = NULL;

	    if (ttkeytab[k].flgs)	/* Don't process CM_INV or CM_ABR */
	      continue;

	    zsoutl(ZMFILE,"");
	    sprintf(buf, "; SET TERMINAL KEY %s", ttkeytab[k].kwd );
	    zsoutl(ZMFILE,buf);

	    for (pnode = ttkeymap[ttkeytab[k].kwval]; 
		 pnode;
		 pnode = pnode->next
		 ) {
		switch (pnode->def.type) {
		  case key:
		    sprintf(buf, "set terminal key %s \\%d \\{%d}\t; %s", 
         		    ttkeytab[k].kwd,
			    mskkeys ? cktomsk(pnode->key) : pnode->key, 
			    pnode->def.key.scancode,
			    keyname(pnode->key)
			    );
		    zsoutl(ZMFILE,buf);
		    break;
		  case kverb:
		    for (j = 0; j < nkverbs; j++)
		      if (kverbs[j].kwval == (pnode->def.kverb.id & ~F_KVERB))
			break;
		    if (j != nkverbs) {
			sprintf(buf, "set terminal key %s \\%d \\K%s\t; %s", 
				ttkeytab[k].kwd,
				mskkeys ? cktomsk(pnode->key) : pnode->key, 
				kverbs[j].kwd, keyname(pnode->key)
				);
			zsoutl(ZMFILE,buf);
		    }
		    break;
		  case macro: {
		      int len = strlen((char *)pnode->def.macro.string);
		      sprintf(buf,"set terminal key %s \\%d ",
			      ttkeytab[k].kwd,
			      mskkeys ? cktomsk(pnode->key) : pnode->key);
		      zsout(ZMFILE,buf);

		      for (j = 0; j < len; j++) {
			  char ch = pnode->def.macro.string[j];
			  if (ch <= SP || ch >= DEL || 
			      ch == '-' || ch == ',') {
			      sprintf(buf, "\\{%d}", ch);
			      zsout(ZMFILE,buf);
			  } else {
			      sprintf(buf, "%c", ch);
			      zsout(ZMFILE,buf);
			  }
		      }
		      sprintf(buf, "\t; %s", keyname(pnode->key));
		      zsoutl(ZMFILE,buf);
		      break;
		  }
		  case esc:
		    sprintf(buf,"set terminal key %s \\%d \\{27}\\{%d}\t; %s", 
			    ttkeytab[k].kwd,
			    mskkeys ? cktomsk(pnode->key) : pnode->key, 
			    pnode->def.esc.key & ~F_ESC,
			    keyname(pnode->key)
			    );
		    zsoutl(ZMFILE,buf);
		    break;
		  case csi:
		    sprintf(buf,"set terminal key %s \\%d \\{27}[\\{%d}\t; %s",
			    ttkeytab[k].kwd,
			    mskkeys ? cktomsk(pnode->key) : pnode->key, 
			    pnode->def.csi.key & ~F_CSI,
			    keyname(pnode->key)
			    );
		    zsoutl(ZMFILE,buf);
		    break;
		  default:
		    continue;
		}
	    }
	}
#endif /* OS2 */

	zsoutl(ZMFILE,"");
	zsoutl(ZMFILE,"; End");
	zclose(ZMFILE);
	return(success = 1);
    } else {
	return(success = 0);
    }
}
#endif /* NOSETKEY */

int
dosave(xx) int xx; {
    int x, y, disp;
    char * s = NULL;
    extern struct keytab disptb[];
#ifdef ZFNQFP
    struct zfnfp * fnp;
#endif /* ZFNQFP */

    switch (xx) {
#ifndef NOSETKEY
      case XSKEY:
	y = cmofi("Name of INI file","keymap.ini",&s,xxstring);
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
	if ((x = cmcfm()) < 0) return(x);
	return (savkeys(s,disp));
#endif /* NOSETKEY */

      default:
	if ((x = cmcfm()) < 0) return(x);
	printf("Not implemented - %s\n",cmdbuf);
	return(success = 0);
    }
}
#endif /* NOICP */
