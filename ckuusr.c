#include "ckcsym.h"
char *userv = "User Interface 7.0.222, 1 Jan 2000";

/*  C K U U S R --  "User Interface" for C-Kermit (Part 1)  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  Originally the entire user interface was in one module, ckuusr.c.  Over
  the years it has been split into many modules: ckuus2.c, ckuus3.c, ...,
  ckuus7.c.  ckuus2.c contains the help command parser and help text strings
  ckuusy.c contains the UNIX-style command-line interface; ckuusx.c contains
  routines needed by both the command-line interface and the interactive
  command parser.
*/

/*
  The ckuus*.c modules depend on the existence of C library features like
  fopen, fgets, feof, (f)printf, argv/argc, etc.  Other functions that are
  likely to vary among different platforms -- like setting terminal modes or
  interrupts -- are invoked via calls to functions that are defined in the
  system- dependent modules, ck?[ft]io.c.  The command line parser processes
  any arguments found on the command line, as passed to main() via argv/argc.
  The interactive parser uses the facilities of the cmd package (developed for
  this program, but usable by any program).  Any command parser may be
  substituted for this one.  The only requirements for the Kermit command
  parser are these:

  . Set parameters via global variables like duplex, speed, ttname, etc.  See
    ckmain.c for the declarations and descriptions of these variables.

  . If a command can be executed without the use of Kermit protocol, then
    execute the command directly and set the variable sstate to 0. Examples
    include 'set' commands, local directory listings, the 'connect' command.

  . If a command requires the Kermit protocol, set the following variables:

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
     cmarg2 is an "as-name" - the name to send file(s) under, or
	the name under which to store incoming file(s); must not be wild.
	A null or empty value means to use the file's own name.
     cmlist is a list of filenames, such as passed via argv.
     nfils is an integer, interpreted as follows:
       -1: filespec (possibly wild) in cmarg, must be expanded internally.
	0: send from stdin (standard input).
       >0: number of files to send, from cmlist.

  The screen() function is used to update the screen during file transfer.
  The tlog() function writes to a transaction log.
  The debug() function writes to a debugging log.
  The intmsg() and chkint() functions provide the user i/o for interrupting
    file transfers.
*/
int g_fncact = -1;			/* Needed for NOICP builds */
int noinit = 0;				/* Flag for skipping init file */

#ifndef NOICP
/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckcnet.h"			/* Network symbols */
#include "ckuusr.h"
#include "ckcxla.h"

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else
#define APIRET ULONG
#include <windows.h>
#include <tapi.h>
#include "cknwin.h"
#include "ckntap.h"			/* CK_TAPI definition */
#endif /* NT */
#include "ckowin.h"
#include "ckocon.h"
extern int tcp_avail;
extern bool viewonly;
extern tt_status;
int display_demo = 1;
#endif /* OS2 */

int optlines = 0;
int didsetlin = 0;

#ifdef VMS
extern int batch;
#endif /* VMS */

#ifdef datageneral
#include <packets:common.h>
#define fgets(stringbuf,max,fd) dg_fgets(stringbuf,max,fd)
#endif /* datageneral */

extern int hints, cmflgs, whyclosed;

#ifndef NOCSETS
extern int nfilc;
extern struct keytab fcstab[];
extern int fcharset;
#endif /* NOCSETS */

char * g_pswd = NULL;
int g_pcpt = -1;
int g_pflg = -1;

extern int cmd_rows, cmd_cols;

#ifndef NOXFER
extern int atcapr, atdiso, nfils, moving, protocol, sendmode, epktflg, size,
  sndsrc, server, displa, inserver, fncnv, fnspath, fnrpath, xfermode, urpsiz,
  spsizf, spsiz, spsizr, spmax, wslotr, prefixing, fncact;

#ifdef CK_LOGIN
extern int isguest;
#endif /* CK_LOGIN */

extern long sendstart;

extern char *cmarg, *cmarg2, **cmlist, * dftty;

extern struct keytab fntab[]; extern int nfntab;
extern struct ck_p ptab[NPROTOS];

int sndcmd = 0;		/* Last command was a SEND-class command. */

int g_xfermode = -1;
int g_proto  = -1;
int g_urpsiz = -1;
int g_spsizf = -1;
int g_spsiz  = -1;
int g_spsizr = -1;
int g_spmax  = -1;
int g_wslotr = -1;
int g_prefixing = -1;
int g_fncnv  = -1;
int g_fnspath = -1;
int g_fnrpath = -1;
int g_fnact  = -1;
int g_displa = -1;
int g_spath  = -1;
int g_rpath  = -1;
char * g_sfilter = NULL;
char * g_rfilter = NULL;

#ifdef PATTERNS
extern int patterns;
int g_patterns = -1;
#endif /* PATTERNS */
int g_skipbup = -1;

#ifdef PIPESEND
extern int usepipes, pipesend;
extern char * sndfilter, * rcvfilter;
#endif /* PIPESEND */

#ifdef PATTERNS
extern char *txtpatterns[], *binpatterns[];
#endif /* PATTERNS */

#ifndef NOSPL
extern int sndxlo, sndxhi, sndxin;
#endif /* NOSPL */

extern char fspec[];			/* Most recent filespec */
extern int fspeclen;			/* Length of fspec[] buffer */

#ifndef NOFRILLS
extern int rmailf;			/* MAIL command items */
extern char optbuf[];
#endif /* NOFRILLS */

extern int
  en_cpy, en_cwd, en_del, en_dir, en_fin, en_get, en_bye, en_mai, en_pri,
  en_hos, en_ren, en_sen, en_spa, en_set, en_typ, en_who, en_ret, en_xit,
  en_mkd, en_rmd, en_asg;

#ifndef NOMSEND				/* Multiple SEND */
extern char *msfiles[];
int filesinlist = 0;			/* And ADD ... */
extern struct filelist * filehead;
extern struct filelist * filetail;
extern struct filelist * filenext;
extern int addlist;
#endif /* NOMSEND */

static struct keytab addtab[] = {
#ifdef PATTERNS
    "binary-patterns", ADD_BIN, 0,
#endif /* PATTERNS */
#ifndef NOMSEND
    "send-list", ADD_SND, 0,
#endif /* NOMSEND */
#ifdef PATTERNS
    "text-patterns", ADD_TXT, 0,
#endif /* PATTERNS */
    "", 0, 0
};
static int naddtab = sizeof(addtab)/sizeof(struct keytab) - 1;

#ifndef NOCSETS
struct keytab assoctab[] = {
    "file-character-set",     ASSOC_FC, 0,
    "transfer-character-set", ASSOC_TC, 0,
    "xfer-character-set",     ASSOC_TC, CM_INV
};
static int nassoc = sizeof(assoctab)/sizeof(struct keytab);
extern int afcset[MAXFCSETS+1];		/* Character-set associations */
extern int axcset[MAXTCSETS+1];
#endif /* NOCSETS */

#ifndef ADDCMD
#ifndef NOMSEND
#define ADDCMD
#endif /* NOMSEND */
#ifndef ADDCMD
#ifdef PATTERNS
#define ADDCMD
#endif /* PATTERNS */
#endif /* ADDCMD */
#endif /* ADDCMD */
#endif /* NOXFER */

/* External Kermit Variables, see ckmain.c for description. */

extern xx_strp xxstring;
extern long xvernum;

extern int local, xitsta, binary, msgflg, escape, duplex, quiet, tlevel,
  pflag, zincnt, ckxech, carrier, what, nopush, haveline, bye_active;
#ifdef TNCODE
extern int debses;
extern char tn_msg[];
#endif /* TNCODE */

int sleepcan = 1;
int g_binary = -1;
int g_recursive = -1;
int g_matchdot = -1;

extern long vernum;
extern char *versio, *copyright[];
extern char *ckxsys;
#ifndef NOHELP
extern char *introtxt[];
extern char *newstxt[];
#endif /* NOHELP */

#ifndef OS2
#ifndef UNIX
extern char *PWDCMD;
#endif /* UNIX */
extern char *WHOCMD;
#endif /* OS2 */

extern char ttname[];

extern CHAR sstate;

extern int network;			/* Have active network connection */
#ifdef NETCONN
extern int nettype,			/* Type of network */
  ttnproto;				/* Network Protocol */
#endif /* NETCONN */

#ifndef NODIAL
extern int dialsta, dialatmo, dialcon, dialcq; /* DIAL status, etc. */
#endif /* NODIAL */

#ifdef CK_APC
extern int apcactive, apcstatus;
#endif /* CK_APC */

#ifndef NOPUSH
#ifndef NOFRILLS
extern char editor[];
extern char editopts[];
extern char editfile[];
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef BROWSER
extern char browser[];			/* Web browser application */
extern char browsopts[];		/* Web browser options */
extern char browsurl[];			/* Most recent URL */
char ftpapp[CKMAXPATH+1] = { NUL, NUL }; /* ftp executable */
char ftpopts[128] = { NUL, NUL };	/* ftp command-line options */
#endif /* BROWSER */

extern struct keytab onoff[];		/* On/Off keyword table */

#ifdef CK_TMPDIR
int f_tmpdir = 0;			/* Directory changed temporarily */
char savdir[TMPDIRLEN];			/* For saving current directory */
extern char * dldir;
#endif /* CK_TMPDIR */

int activecmd = -1;			/* Keyword index of active command */
int doconx = -1;			/* CONNECT-class command active */
int ooflag = 0;				/* User-settable on/off flag */

int rcflag = 0;				/* Pointer to home directory string */
int repars,				/* Reparse needed */
    techo = 0;				/* Take echo */
int secho = 1;				/* SCRIPT echo */

int xitwarn =			/* Warn about open connection on exit */
#ifdef NOWARN
0
#else
1
#endif /* NOWARN */
;

struct keytab onoffsw[] = {
    "/off", 0, 0,
    "/on",  1, 0
};

#ifdef CKEXEC
struct keytab redirsw[] = {
    "/redirect", 1, 0
};
#endif /* CKEXEC */

#ifndef NOXMIT
/* Variables for TRANSMIT command */

int xmitx = 1;			/* Whether to echo during TRANSMIT */
int xmitf = 0;			/* Character to fill empty lines */
int xmitl = 0;			/* 0 = Don't send linefeed too */
int xmitp = LF;			/* Host line prompt */
int xmits = 0;			/* Use shift-in/shift-out, 0 = no */
int xmitw = 0;			/* Milliseconds to pause during TRANSMIT */
int xmitt = 1;			/* Seconds to wait for each char to echo */
int xmita = 1;			/* Action upon timeout */

#define XMI_BIN 1
#define XMI_TXT 2
#define XMI_CMD 3
#define XMI_TRA 4
#define XMI_VRB 5
#define XMI_QUI 6
#define XMI_NOW 7

static struct keytab xmitsw[] = {	/* TRANSMIT command options */
    "/binary",          XMI_BIN, 0,
#ifdef PIPESEND
    "/command",         XMI_CMD, CM_INV,
#endif /* PIPESEND */
    "/nowait",          XMI_NOW, 0,
#ifdef PIPESEND
    "/pipe",            XMI_CMD, 0,
#endif /* PIPESEND */
#ifdef COMMENT
    "/quiet",           XMI_QUI, 0,
#endif /* COMMENT */
    "/text",            XMI_TXT, 0,
    "/transparent",     XMI_TRA, 0,
#ifdef COMMENT
    "/verbose",         XMI_VRB, 0,
#endif /* COMMENT */
    "", 0, 0
};
#define NXMITSW sizeof(xmitsw)/sizeof(struct keytab) - 1
static int nxmitsw = NXMITSW;

#endif /* NOXMIT */

/* Declarations from ck?fio.c module */

extern char *SPACMD, *SPACM2;		/* SPACE commands */

/* Command-oriented items */

#ifdef DCMDBUF
extern char *cmdbuf;			/* Command buffers */
extern char *atmbuf;
extern char *line;			/* Character buffer for anything */
extern char *tmpbuf;			/* Short temporary string buffer */
extern int *ifcmd;
extern int *intime;
extern char *inpcas;
#else
extern char cmdbuf[];			/* Command buffers */
extern char atmbuf[];
extern char line[];			/* Character buffer for anything */
extern char tmpbuf[];			/* Temporary buffer */
extern int ifcmd[];
extern int intime[];
extern char inpcas[];
#endif /* DCMDBUF */

char *lp;				/* Pointer to line buffer */

#ifndef NOSPL
int oldeval = 0;
char evalbuf[33];			/* EVALUATE result */
extern char * inpbuf;			/* Buffer for INPUT and REINPUT */
char *inpbp;				/* And pointer to same */
extern char lblbuf[];			/* Buffer for labels */
int m_found;				/* MINPUT result */
int i_active = 0;			/* INPUT command is active */
char *ms[MINPMAX];			/* Pointers to MINPUT strings */
static int mp[MINPMAX];			/* and flags */
extern int fndiags, fnerror, fnsuccess;	/* Function diagnostics */
#endif /* NOSPL */

char psave[PROMPTL] = { NUL };		/* For saving & restoring prompt */

extern int success;			/* Command success/failure flag */
extern int cmdlvl;			/* Current position in command stack */

#ifndef NOSPL
int					/* SET INPUT parameters. */
/* Note, INPUT TIMEOUT, intime[], is on the command-level stack. */
  inbufsize = 0,			/* INPUT buffer size */
  indef = 1,				/* default timeout, seconds */
  inecho = 1,				/* 1 = echo on */
  inautodl = 0,				/* INPUT autodownload */
  inintr = 1,				/* INPUT interrupion allowed */
  insilence = 0;			/* 0 = no silence constraint */
#ifdef OS2
int interm = 1;				/* Terminal emulator displays input */
#endif /* OS2 */
int maclvl = -1;			/* Macro nesting level */
int mecho = 0;				/* Macro echo, 0 = don't */
char varnam[6];				/* For variable names */
extern int macargc[];			/* ARGC from macro invocation */

extern char *m_arg[MACLEVEL][NARGS];	/* Stack of macro arguments */
extern char *mrval[];

extern char **a_ptr[];			/* Array pointers */
extern int a_dim[];			/* Array dimensions */

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;		/* The command stack itself */
#else
extern struct cmdptr cmdstk[];		/* The command stack itself */
#endif /* DCMDBUF */

long ck_alarm = 0;			/* SET ALARM value */
char alrm_date[10] = { ' ',' ',' ',' ',' ',' ',' ',' ',' ' };
char alrm_time[ 8] = { ' ',' ',' ',' ',' ',' ',' ' };

#endif /* NOSPL */

static int x, y, z = 0;			/* Local workers */
static char *s;

#define xsystem(s) zsyscmd(s)

/* Top-Level Interactive Command Keyword Table */
/* Keywords must be in lowercase and in alphabetical order. */

struct keytab cmdtab[] = {
#ifndef NOPUSH
    "!",	   XXSHE, CM_INV,	/* Shell escape */
#else
    "!",	   XXNOTAV, CM_INV,
#endif /* NOPUSH */
    "#",    	   XXCOM, CM_INV,	/* Comment */
#ifndef NOSPL
    ".",           XXDEF, CM_INV,	/* Assignment */
    ":",           XXLBL, CM_INV,	/* Label */
#endif /* NOSPL */
#ifdef CK_REDIR
#ifndef NOPUSH
    "<",           XXFUN, CM_INV,	/* REDIRECT */
#else
    "<",           XXNOTAV, CM_INV,	/* REDIRECT */
#endif /* NOPUSH */
#endif /* CK_REDIR */
#ifndef NOPUSH
    "@",           XXSHE, CM_INV,	/* DCL escape */
#else
    "@",           XXNOTAV, CM_INV,	/* DCL escape */
#endif /* NOPUSH */
    "about",       XXVER, CM_INV,	/* Synonym for VERSION */
#ifndef NOSPL
#ifdef ADDCMD
    "add",         XXADD, 0,		/* ADD */
#endif /* ADDCMD */
#ifndef NODIAL
    "answer",      XXANSW, 0,		/* ANSWER the phone */
#else
    "answer",      XXNOTAV, CM_INV,	/* ANSWER the phone */
#endif /* NODIAL */
    "apc",         XXAPC, 0,		/* Application Program Command */
#ifndef NOSPL
    "array",       XXARRAY, 0,          /* Array operations */
#endif /* NOSPL */
    "ascii",       XXASC, CM_INV,
    "asg",         XXASS, CM_INV,       /* Invisible synonym for ASSIGN */
    "ask",         XXASK, 0,		/* ASK for text, assign to variable */
    "askq",        XXASKQ,0,            /* ASK quietly (no echo) */
#ifndef NOSPL
    "ass",         XXASS, CM_INV|CM_ABR,
    "assert",      XXASSER, 0,		/* ASSERT */
    "assign",      XXASS, 0,		/* ASSIGN */
#endif /* NOSPL */
#ifndef NOXFER
#ifndef NOCSETS
    "associate",   XXASSOC, 0,		/* ASSOCIATE */
#else
    "associate",   XXNOTAV, CM_INV,	/* ASSOCIATE */
#endif /* NOCSETS */
#endif /* NOXFER */
#ifdef CK_KERBEROS
    "authenticate",XXAUTH,		/* Authentication */
#ifdef CK_AUTHENTICATION
                          0,
#else
                          CM_INV,
#endif /* CK_AUTHENTICATION */
#endif /* CK_KERBEROS */
#endif /* NOSPL */
#ifndef NOFRILLS
    "back",        XXBACK,0,		/* BACK to previous directory */
#else
    "back",        XXNOTAV,CM_INV,
#endif /* NOFRILLS */
    "beep",        XXBEEP,CM_INV,	/* BEEP */
#ifndef NOXFER
    "binary",      XXBIN, CM_INV,	/* == SET FILE TYPE BINARY */
#endif /* NOXFER */
#ifndef NOFRILLS
    "bug",         XXBUG, CM_INV,	/* BUG report instructions */
#else
    "bug",         XXNOTAV, CM_INV,
#endif /* NOFRILLS */
#ifdef BROWSER
    "browse",      XXBROWS, 0,		/* BROWSE (start browser) */
#else
    "browse",      XXNOTAV, CM_INV,	/* BROWSE (start browser) */
#endif /* BROWSER */
#ifndef NOXFER
    "bye",         XXBYE, 0,		/* BYE to remote server */
#endif /* NOXFER */
#ifndef NOLOCAL
    "c",           XXCON, CM_INV|CM_ABR, /* invisible synonym for CONNECT */
#endif /* NOLOCAL */
#ifndef NOFRILLS
    "cat",         XXTYP, CM_INV,	/* Invisible synonym for TYPE */
#endif /* NOFRILLS */
#ifndef NOSPL
#ifdef COMMENT
#ifndef NOXFER
    "cautious",    XXCAU, CM_INV,
#endif /* NOXFER */
#endif /* COMMENT */
#endif /* NOSPL */
    "cd",          XXCWD, 0,		/* Change Directory */
#ifndef NOXFER
#ifdef PIPESEND
    "cget",        XXCGET, CM_INV,	/* CGET */
#else
    "cget",        XXNOTAV, CM_INV,	/* CGET */
#endif /* PIPESEND */
#endif /* NOXFER */
    "check",       XXCHK, 0,		/* CHECK for a feature */
    "ckermit",     XXKERMI, CM_INV,
#ifndef NOFRILLS
    "clear",       XXCLE, 0,		/* CLEAR input and/or device buffer */
#else
    "clear",       XXNOTAV, CM_INV,
#endif /* NOFRILLS */
    "close",	   XXCLO, 0,		/* CLOSE a log or other file */
    "cls",         XXCLS, CM_INV,	/* Clear Screen (CLS) */
    "comment",     XXCOM, CM_INV,	/* Introduce a comment */
#ifndef NOLOCAL
    "connect",     XXCON, 0,		/* Begin terminal connection */
#else
    "connect",     XXNOTAV, 0,
#endif /* NOLOCAL */
#ifndef NOFRILLS
#ifdef ZCOPY
    "co",          XXCPY, CM_INV|CM_ABR,
    "cop",         XXCPY, CM_INV|CM_ABR,
    "copy",        XXCPY, 0,		/* COPY a file */
#else
    "copy",        XXNOTAV, CM_INV,
#endif /* ZCOPY */
    "copyright",   XXCPR, CM_INV,	/* COPYRIGHT */
#ifdef ZCOPY
    "cp",          XXCPY, CM_INV,	/* COPY a file */
#endif /* ZCOPY */
#ifndef NOLOCAL
#ifndef OS2
    "cq",          XXCQ, CM_INV,	/* CQ (connect quietly) */
#endif /* OS2 */
#endif /* NOLOCAL */
#ifndef NOXFER
#ifdef PIPESEND
    "creceive",    XXCREC,CM_INV,	/* CRECEIVE (receive to command) */
    "csend",       XXCSEN,CM_INV,	/* CSEND (send from command) */
#else
    "creceive",    XXNOTAV,CM_INV,	/* CRECEIVE (receive to command) */
    "csend",       XXNOTAV,CM_INV,	/* CSEND (send from command) */
#endif /* PIPESEND */
#endif /* NOXFER */
#endif /* NOFRILLS */

    "cwd",	   XXCWD, CM_INV,	/* Invisible synonym for cd */

#ifndef NOSPL
    "date",        XXDATE,0,		/* DATE */
    "dcl",         XXDCL, CM_INV,	/* DECLARE an array (see ARRAY) */
    "declare",     XXDCL, CM_INV,	/* DECLARE an array (see ARRAY) */
    "decrement",   XXDEC, 0,		/* DECREMENT a numeric variable */
    "define",      XXDEF, 0,		/* DEFINE a macro or variable */
#else
    "date",        XXNOTAV, CM_INV,
    "dcl",         XXNOTAV, CM_INV,
    "declare",     XXNOTAV, CM_INV,
    "decrement",   XXNOTAV, CM_INV,
    "define",      XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOFRILLS
    "delete",      XXDEL, 0,		/* DELETE a file */
#else
    "delete",      XXNOTAV, CM_INV,
#endif /* NOFRILLS */

#ifndef NODIAL
    "dial",	   XXDIAL,0,		/* DIAL a phone number */
#else
    "dial",	   XXNOTAV, CM_INV,
#endif /* NODIAL */

    "directory",   XXDIR, 0,		/* DIRECTORY of files */

#ifndef NOFRILLS
#ifndef NOSERVER
    "disable",     XXDIS, 0,		/* DISABLE a server function */
#else
    "disable",     XXNOTAV, CM_INV,
#endif /* NOSERVER */
#endif /* NOFRILLS */

#ifndef NOSPL
    "do",          XXDO,  0,		/* DO (execute) a macro */
#else
    "do",          XXNOTAV, CM_INV,
#endif /* NOSPL */

    "e",           XXEXI, CM_INV|CM_ABR,

#ifndef NOFRILLS
#ifndef NOXFER
    "e-packet",    XXERR, CM_INV,	/* Send an Error-Packet */
#endif /* NOXFER */
#endif /* NOFRILLS */

    "echo",        XXECH, 0,		/* ECHO text */

#ifndef NOFRILLS
#ifndef NOPUSH
    "edit",        XXEDIT, 0,		/* EDIT */
#else
    "edit",        XXNOTAV, CM_INV,	/* EDIT */
#endif /* NOPUSH */
#endif /* NOFRILLS */

    "eightbit",    XXEIGHT, 0,		/* EIGHTBIT */

#ifndef NOSPL
    "else",        XXELS, CM_INV,	/* ELSE part of IF statement */
#else
    "else",        XXNOTAV, CM_INV,	/* ELSE part of IF statement */
#endif /* NOSPL */

#ifndef NOSERVER
#ifndef NOFRILLS
    "enable",      XXENA,  0,		/* ENABLE a server function */
#else
    "enable",      XXNOTAV, CM_INV,
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
    "end",         XXEND,  0,		/* END command file or macro */
    "evaluate",    XXEVAL, 0,		/* EVALUATE */
#else
    "end",         XXNOTAV, CM_INV,
    "evaluate",    XXNOTAV, CM_INV,
#endif /* NOSPL */

    "ex",          XXEXI, CM_INV|CM_ABR, /* Let "ex" still be EXIT */

#ifdef CKEXEC
    "exec",        XXEXEC, CM_INV,	/* exec() */
#else
    "exec",        XXNOTAV, CM_INV,
#endif /* CKEXEC */

    "exit",	   XXEXI, 0,		/* EXIT from C-Kermit */
    "extended-options", XXXOPTS,CM_INV|CM_HLP, /* Extended-Options */

#ifdef OS2
    "extproc",     XXCOM, CM_INV,	/* Dummy command for OS/2 */
#endif /* OS2 */

#ifndef NOXFER
    "f",           XXFIN, CM_INV|CM_ABR, /* Invisible abbreviation for FIN */
#endif /* NOXFER */

#ifndef NOSPL
    "fail",        XXFAIL,0,		/* FAIL */

#ifdef COMMENT
#ifndef NOXFER
    "fast",        XXFAST, CM_INV,
#endif /* NOXFER */
#endif /* COMMENT */

#ifdef CKCHANNELIO
    "fclose",      XXF_CL, CM_INV,	/* FCLOSE */
    "fcount",      XXF_CO, CM_INV,	/* FCOUNT */
    "fflush",      XXF_FL, CM_INV,	/* FFLUSH */
#endif /* CKCHANNELIO */

#ifndef NOXFER
    "fi",          XXFIN, CM_INV|CM_ABR, /* FINISH */
#endif /* NOXFER */

#ifdef CKCHANNELIO
    "file",        XXFILE, 0,		/* FILE */
#endif /* CKCHANNELIO */
#endif /* NOSPL */

#ifndef NOXFER
    "finish",      XXFIN, 0,		/* FINISH */
#endif /* NOXFER */

#ifdef CKCHANNELIO
    "flist",      XXF_LI, CM_INV,	/* FLIST */
    "fopen",      XXF_OP, CM_INV,	/* FOPEN */
#endif /* CKCHANNELIO */

#ifndef NOSPL
    "fo",          XXFOR, CM_INV|CM_ABR, /* Invisible abbreviation for... */
    "for",         XXFOR, 0,		/* FOR loop */
    "forward",     XXFWD, CM_INV,	/* FORWARD */
#endif /* NOSPL */
#ifndef NOFRILLS
    "fot",	   XXDIR, CM_INV,	/* "fot" = "dir" (for Chris) */
#endif /* NOFRILLS */

#ifdef CKCHANNELIO
    "fread",      XXF_RE, CM_INV,	/* FREAD */
    "frewind",    XXF_RW, CM_INV,	/* FREWIND */
    "fseek",      XXF_SE, CM_INV,	/* FSEEK */
    "fstatus",    XXF_ST, CM_INV,	/* FSTATUS */
#endif /* CKCHANNELIO */

#ifdef TCPSOCKET
#ifndef NOPUSH
    "ftp",	   XXFTP,   CM_INV,	/* FTP */
#else
    "ftp",	   XXNOTAV, CM_INV,
#endif /* NOPUSH */
#else
    "ftp",	   XXNOTAV, CM_INV,
#endif /* TCPSOCKET */

#ifndef NOSPL
    "function",    XXFUNC, CM_INV|CM_HLP, /* Function (for HELP FUNCTION) */
#endif /* NOSPL */

#ifdef CKCHANNELIO
    "fwrite",      XXF_WR, CM_INV,	/* FWRITE */
#endif /* CKCHANNELIO */

#ifndef NOXFER
    "g",           XXGET, CM_INV|CM_ABR, /* Invisible abbreviation for GET */
#ifndef NOSPL
    "ge",          XXGET, CM_INV|CM_ABR, /* Ditto */
#endif /* NOSPL */
    "get",         XXGET, 0,		/* GET */
#endif /* NOXFER */
#ifndef NOSPL
    "getc",        XXGETC, 0,		/* GETC */
#ifdef OS2
    "getkeycode",  XXGETK, 0,		/* GETKEYCODE */
#endif /* OS2 */
#ifndef NOFRILLS
    "getok",       XXGOK, 0,		/* GETOK (ask for Yes/No/OK) */
#endif /* NOFRILLS */
#endif /* NOSPL */
#ifndef NOSPL
    "goto",        XXGOTO,0,		 /* GOTO label in take file or macro */
#endif /* NOSPL */
    "h",           XXHLP, CM_INV|CM_ABR, /* Invisible synonym for HELP */
#ifndef NOLOCAL
    "hangup",      XXHAN, 0,		 /* HANGUP the connection */
#endif /* NOLOCAL */
    "help",	   XXHLP, 0,		 /* Display HELP text */
#ifndef NOHTTP
#ifdef TCPSOCKET
    "http",        XXHTTP,CM_INV,	 /* HTTP operations (not yet) */
#endif /* TCPSOCKET */
#endif /* NOHTTP */
#ifndef NOSPL
    "i",           XXINP, CM_INV|CM_ABR, /* Invisible synonym for INPUT */
    "if",          XXIF,  0,		 /* IF ( condition ) command */
#ifdef TCPSOCKET
    "iksd",        XXIKSD,0,            /* IKSD (TCP/IP only) */
#else
    "iksd",        XXNOTAV, CM_INV,
#endif /* TCPSOCKET */
    "in",          XXINP, CM_INV|CM_ABR, /* Invisible synonym for INPUT */
    "increment",   XXINC, 0,		 /* Increment a numeric variable */
    "input",       XXINP, 0,		 /* INPUT text from comm device */
#endif /* NOSPL */

#ifndef NOHELP
    "int",         XXINT, CM_INV|CM_ABR,
    "intr",        XXINT, CM_INV|CM_ABR,
    "intro",       XXINT, 0,
    "introduction",XXINT, CM_INV,	/* Print introductory text */
#else
    "intro",       XXNOTAV, CM_INV,
    "introduction",XXNOTAV, CM_INV,
#endif /* NOHELP */

#ifdef OS2
    "k95",         XXKERMI, CM_INV,	/* Hmmm what's this... */
    "kermit",      XXKERMI, CM_INV,
#else
    "kermit",      XXKERMI, CM_INV,
#endif /* OS2 */

#ifdef OS2
#ifndef NOKVERBS
    "kverb",       XXKVRB, CM_INV|CM_HLP, /* Keyboard verb */
#endif /* NOKVERBS */
#endif /* OS2 */

#ifndef NOFRILLS
    "l",           XXLOG, CM_INV|CM_ABR, /* Invisible synonym for log */
#endif /* NOFRILLS */

#ifndef NOSPL
    "lineout",     XXLNOUT, 0,		/* LINEOUT = OUTPUT + eol */
#endif /* NOSPL */

#ifndef NOFRILLS
    "lo",           XXLOG, CM_INV|CM_ABR, /* Invisible synonym for log */
#endif /* NOFRILLS */

#ifndef NOSPL
    "local",       XXLOCAL, 0,		/* LOCAL variable declaration */
#else
    "local",       XXNOTAV, CM_INV,
#endif /* NOSPL */

    "log",  	   XXLOG, 0,		/* Open a log file */

    "login",       XXLOGIN,  0,		/* LOGIN to server or IKSD */
    "logout",      XXLOGOUT, 0,         /* LOGOUT from server or IKSD */

#ifndef NOFRILLS
#ifndef NODIAL
    "lookup",      XXLOOK,0,		/* LOOKUP */
#else
    "lookup",      XXNOTAV, CM_INV,
#endif /* NODIAL */
#ifdef UNIXOROSK
    "ls",          XXLS,  0,		/* UNIX ls command */
#else
    "ls",          XXDIR, CM_INV,	/* Invisible synonym for DIR */
#endif /* UNIXOROSK */
#ifndef NOXFER
    "mail",        XXMAI, 0,		/* Send a file as e-mail */
#endif /* NOXFER */
#ifndef NOHELP
    "manual",      XXMAN, 0,		/* MAN(UAL) */
#else
    "manual",      XXNOTAV, CM_INV,
#endif /* NOHELP */
#endif /* NOFRILLS */
#ifdef CK_MKDIR
    "md",          XXMKDIR, CM_INV,	/* Synonym for MKDIR */
#endif /* CK_MKDIR */
#ifdef CK_MINPUT
    "minput",      XXMINP, 0,		/* MINPUT */
#else
    "minput",      XXNOTAV, CM_INV,
#endif /* CK_MINPUT */
#ifndef NOMSEND
    "mget",        XXMGET, 0,		/* MGET */
#else
    "mget",        XXNOTAV, CM_INV,
#endif /* NOMSEND */
#ifdef CK_MKDIR
    "mkdir",       XXMKDIR, 0,		/* MKDIR */
#else
    "mkdir",       XXNOTAV, CM_INV,
#endif /* CK_MKDIR */

#ifndef NOXFER
#ifndef NOMSEND
    "mmove",       XXMMOVE, 0,		/* MMOVE */
#else
    "mmove",       XXNOTAV, CM_INV,
#endif /* NOMSEND */
#endif /* NOXFER */

#ifndef NOFRILLS
    "more",        XXMORE, CM_INV,	/* MORE */
#endif /* NOFRILLS */

#ifndef NOXFER
    "move",        XXMOVE, 0,		/* MOVE  */
#endif /* NOXFER */

#ifndef NOSPL
    "mpause",      XXMSL, CM_INV,	/* Millisecond sleep */
#else
    "mpause",      XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOXFER
#ifndef NOMSEND
    "mput",        XXMSE, CM_INV,	/* MPUT = MSEND */
    "ms",          XXMSE, CM_INV|CM_ABR,
    "msend",       XXMSE, 0,		/* Multiple SEND */
#else
    "mput",        XXNOTAV, CM_INV,
    "msend",       XXNOTAV, CM_INV,
#endif /* NOMSEND */
#endif /* NOXFER */
#ifndef NOSPL
    "msleep",      XXMSL, 0,		/* Millisecond sleep */
#else
    "msleep",      XXNOTAV, CM_INV,
#endif /* NOSPL */
#ifndef NOFRILLS
    "mv",          XXREN, CM_INV,	/* Synonym for rename */
#endif /* NOFRILLS */
#ifndef NOHELP
    "news",        XXNEW, CM_INV,	/* Display NEWS of new features */
#else
    "news",        XXNOTAV, CM_INV,
#endif /* NOHELP */
    "nolocal",     XXNLCL, CM_INV,	/* Disable SET LINE / SET HOST */
    "nopush",      XXNPSH, CM_INV,	/* Disable PUSH command/features */
#ifndef NOSPL
    "o",           XXOUT, CM_INV|CM_ABR, /* Invisible synonym for OUTPUT */
    "open",        XXOPE, 0,		/* OPEN file for reading or writing */
#else
    "open",        XXNPSH, CM_INV,	/* Disable PUSH command/features */
#endif /* NOSPL */
#ifndef NOHELP
    "options",     XXOPTS,CM_INV|CM_HLP, /* Options */
#endif /* NOHELP */
#ifndef NOSPL
    "output",      XXOUT, 0,		/* OUTPUT text to comm device */
#else
    "output",      XXNOTAV, CM_INV,
#endif /* NOSPL */
#ifdef ANYX25
#ifndef IBMX25
    "pad",         XXPAD, 0,            /* X.3 PAD commands */
#endif /* IBMX25 */
#endif /* ANYX25 */
#ifndef NOSPL
    "pause",       XXPAU, 0,		/* Sleep for specified interval */
#else
    "pause",       XXNOTAV, CM_INV,
#endif /* NOSPL */
#ifndef NODIAL
    "pdial",       XXPDIA,0,		/* PDIAL (partial dial) */
#else
    "pdial",       XXNOTAV, CM_INV,
#endif /* NODIAL */
#ifdef TCPSOCKET
#ifndef NOPUSH
    "ping",        XXPNG, CM_INV,	/* PING */
#else
    "ping",        XXNOTAV, CM_INV,
#endif /* NOPUSH */
#endif /* TCPSOCKET */
#ifdef NETCMD
#ifndef NOPUSH
    "pipe",        XXPIPE, 0,		/* PIPE */
#else
    "pipe",        XXNOTAV, CM_INV,	/* PIPE */
#endif /* NOPUSH */
#endif /* NETCMD */

#ifndef NOSPL
    "pop",         XXEND, CM_INV,	/* Invisible synonym for END */
#endif /* NOSPL */
#ifndef NOFRILLS
    "print",       XXPRI, 0,		/* PRINT a file locally */
#endif /* NOFRILLS */
#ifndef NOXFER
#ifdef CK_RESEND
    "psend",       XXPSEN, CM_INV,	/* PSEND */
#else
    "psend",       XXNOTAV, CM_INV,
#endif /* CK_RESEND */
#endif /* NOXFER */

#ifdef NETPTY
    "pty",         XXPTY, 0,		/* PTY */
#else
    "pty",         XXNOTAV, CM_INV,
#endif /* NETPTY */

#ifndef NOPUSH
    "pu",          XXSHE, CM_INV|CM_ABR, /* PU = PUSH */
#endif /* NOPUSH */

#ifdef CKPURGE
    "purge",       XXPURGE, 0,		/* PURGE (real) */
#else
#ifdef VMS
    "purge",       XXPURGE, 0,		/* PURGE (fake) */
#else
    "purge",       XXNOTAV, CM_INV,
#endif /* VMS */
#endif /* CKPURGE */

#ifndef NOPUSH
    "push",        XXSHE, 0,		/* PUSH command (like RUN, !) */
#else
    "push",        XXNOTAV, CM_INV,
#endif /* NOPUSH */

#ifndef NOXFER
    "put",	   XXSEN, CM_INV,	/* PUT = SEND */
#endif /* NOXFER */

    "pwd",         XXPWD, 0,            /* Print Working Directory */
    "q",       	   XXQUI, CM_INV|CM_ABR, /* Invisible synonym for QUIT */

#ifndef NOXFER
    "query",       XXRQUE,0,
#endif /* NOXFER */

    "quit",	   XXQUI, 0,		/* QUIT from program = EXIT */

#ifndef NOXFER
    "r",           XXREC, CM_INV|CM_ABR, /* Invisible synonym for RECEIVE */
#endif /* NOXFER */

#ifndef NOXFER
    "rasg",        XXRASG, CM_INV,	/* REMOTE ASSIGN */
    "rassign",     XXRASG, CM_INV,	/* ditto */
    "rcd",         XXRCWD, CM_INV,	/* REMOTE CD */
    "rcopy",       XXRCPY, CM_INV,	/* REMOTE COPY */
    "rcwd",        XXRCWD, CM_INV,	/* REMOTE CWD */
    "rdelete",     XXRDEL, CM_INV,	/* REMOTE DELETE */
    "rdirectory",  XXRDIR, CM_INV,	/* REMODE DIRECTORY */
#endif /* NOXFER */

#ifndef NOSPL
    "read",        XXREA, 0,            /* READ a line from a file */
#else
    "read",        XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOXFER
    "receive",	   XXREC, 0,		/* RECEIVE files */
#endif /* NOXFER */

#ifndef NODIAL
    "red",         XXRED, CM_INV|CM_ABR, /* Invisible synonym for REDIAL */
    "redi",        XXRED, CM_INV|CM_ABR, /* Invisible synonym for REDIAL */
    "redial",      XXRED, 0,		/* REDIAL last DIAL number */
#else
    "red",         XXNOTAV, CM_INV,
    "redi",        XXNOTAV, CM_INV,
    "redial",      XXNOTAV, CM_INV,
#endif /* NODIAL */

#ifdef CK_REDIR
#ifdef OS2
#ifndef NOPUSH
    "redirect",    XXFUN, CM_INV,	/* REDIRECT local command to ttyfd */
#else
    "redirect",    XXNOTAV, CM_INV,
#endif /* NOPUSH */
#else /* OS2 */
#ifndef NOPUSH
    "redirect",    XXFUN, 0,		/* REDIRECT local command to ttyfd */
#else
    "redirect",    XXNOTAV, CM_INV,
#endif /* NOPUSH */
#endif /* OS2 */
#endif /* CK_REDIR */

#ifdef CK_RECALL
    "redo",        XXREDO,  CM_NOR,	/* REDO */
#else
    "redo",        XXNOTAV, CM_INV,
#endif /* CK_RECALL */

#ifndef NOXFER
#ifdef CK_RESEND
    "reget",       XXREGET, 0,		/* REGET */
#else
    "reget",       XXNOTAV, CM_INV,
#endif /* CK_RESEND */
#endif /* NOXFER */

#ifndef NOHELP
    "regular-expressions", XXWILD,CM_INV|CM_HLP,
#endif /* NOHELP */

#ifndef NOSPL
    "reinput",     XXREI, 0,            /* REINPUT (from INPUT buffer) */
#else
    "reinput",     XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOXFER
#ifdef ADDCMD
    "rem",         XXREM, CM_INV|CM_ABR,
    "remo",        XXREM, CM_INV|CM_ABR,
#endif /* ADDCMD */
    "remote",	   XXREM, 0,		/* Send REMOTE command to server */
#endif /* NOXFER */

#ifdef ADDCMD
    "remove",      XXREMV,0,		/* REMOVE (something from a list) */
#else
    "remove",      XXNOTAV, CM_INV,
#endif /* ADDCMD */

#ifndef NOFRILLS
#ifndef NORENAME
    "rename",      XXREN, 0,		/* RENAME a local file */
#else
    "rename",      XXNOTAV, CM_INV,
#endif /* NORENAME */
    "replay",      XXTYP, CM_INV,	/* REPLAY (for now, just type) */
#endif /* NOFRILLS */

#ifndef NOXFER
#ifdef CK_RESEND
    "res",         XXRSEN, CM_INV|CM_ABR, /* RESEND */
    "rese",        XXRSEN, CM_INV|CM_ABR, /* RESEND */
    "resend",      XXRSEN, 0,		  /* RESEND */
#else
    "res",         XXNOTAV, CM_INV,
    "rese",        XXNOTAV, CM_INV,
    "resend",      XXNOTAV, CM_INV,
#endif /* CK_RESEND */
#endif /* NOXFER */

    "reset",       XXRESET, CM_INV,	/* RESET */

#ifdef CK_RESEND
#ifndef NOSPL
    "ret",         XXRET, CM_INV|CM_ABR,
#endif /* NOSPL */
#endif /* CK_RESEND */

#ifndef NOXFER
    "retrieve",    XXRETR, CM_INV,	/* RETRIEVE */
#endif /* NOXFER */

#ifndef NOSPL
    "return",      XXRET, 0,		/* RETURN from a function */
#else
    "return",      XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOXFER
    "rexit",      XXRXIT, CM_INV,	/* REMOTE EXIT */
#endif /* NOXFER */

#ifdef CK_REXX
#ifndef NOPUSH
    "rexx",       XXREXX, 0,		/* Execute a Rexx command */
#else
    "rexx",       XXNOTAV, CM_INV,
#endif /* NOPUSH */
#endif /* CK_REXX */

#ifndef NOXFER
    "rhelp",      XXRHLP, CM_INV,	/* REMOTE HELP */
    "rhost",      XXRHOS, CM_INV,	/* REMOTE HOST */
    "rkermit",    XXRKER, CM_INV,	/* REMOTE KERMIT */
#endif /* NOXFER */

#ifdef TCPSOCKET
    "rlogin",    XXRLOG, 0,		/* Make an Rlogin connection */
#else
    "rlogin",    XXNOTAV, CM_INV,
#endif /* TCPSOCKET */

#ifndef NOFRILLS
    "rm",          XXDEL, CM_INV,	/* Invisible synonym for delete */
#endif /* NOFRILLS */

#ifdef CK_MKDIR
    "rmdir",       XXRMDIR, 0,          /* RMDIR */
#else
    "rmdir",       XXNOTAV, CM_INV,
#endif /* CK_MKDIR */

#ifndef NOXFER
    "rmkdir",      XXRMKD, CM_INV,	/* REMOTE MKDIR */
#ifndef NOSPL
    "robust",      XXROB,  CM_INV,
#else
    "robust",      XXNOTAV, CM_INV,
#endif /* NOSPL */
    "rpwd",        XXRPWD, CM_INV,	/* REMOTE PWD */
    "rquery",      XXRQUE, CM_INV,	/* REMOTE QUERY */
#endif /* NOXFER */

#ifdef CK_RECALL
    "rr",          XXREDO, CM_INV|CM_NOR,
#endif /* CK_RECALL */

#ifndef NOXFER
    "rrename",    XXRREN, CM_INV,	/* REMOTE RENAME */
    "rrmdir",     XXRRMD, CM_INV,	/* REMOTE REMDIR */
    "rset",       XXRSET, CM_INV,	/* REMOTE SET */
    "rspace",     XXRSPA, CM_INV,	/* REMOTE SPACE */
    "rtype",      XXRTYP, CM_INV,	/* REMOTE TYPE */
#endif /* NOXFER */

#ifndef NOPUSH
    "run",         XXSHE, 0,		/* RUN a program or command */
#else
    "run",         XXNOTAV, CM_INV,
#endif /* NOPUSH */

#ifndef NOXFER
    "rwho",        XXRWHO, CM_INV,	/* REMOTE WHO */
    "s",           XXSEN, CM_INV|CM_ABR, /* Invisible synonym for send */
#endif /* NOXFER */

#ifndef NOSETKEY
#ifdef OS2
    "save",	   XXSAVE, 0,		/* SAVE something */
#else
    "save",	   XXSAVE, CM_INV,
#endif /* OS2 */
#else
    "save",	   XXNOTAV, CM_INV,
#endif /* NOSETKEY */

#ifndef NOSCRIPT
    "sc", 	   XXLOGI, CM_INV|CM_ABR,
    "scr",	   XXLOGI, CM_INV|CM_ABR,
#endif /* NOSCRIPT */
    "screen",      XXSCRN, 0,		/* SCREEN actions */
#ifndef NOSCRIPT
    "script",	   XXLOGI,0,		/* Expect-Send-style script line */
#else
    "script",	   XXNOTAV, CM_INV,
#endif /* NOSCRIPT */

#ifndef NOXFER
    "send",	   XXSEN, 0,		/* Send (a) file(s) */
#ifndef NOSERVER
    "server",	   XXSER, 0,		/* Be a SERVER */
#else
    "server",	   XXNOTAV, CM_INV,
#endif /* NOSERVER */
#endif /* NOXFER */

    "set",	   XXSET, 0,		/* SET parameters */

#ifndef NOSPL
#ifndef NOSHOW
    "sh",          XXSHO, CM_INV|CM_ABR,/* SHOW parameters */
#endif /* NOSHOW */
    "shift",       XXSHIFT, 0,		/* SHIFT args */
#else
    "shift",       XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOSHOW
    "show", 	   XXSHO, 0,		/* SHOW parameters */
#else
    "show", 	   XXNOTAV, CM_INV,
#endif /* NOSHOW */

#ifndef NOSPL
#ifndef NOFRILLS
    "sleep",       XXPAU, CM_INV,	/* SLEEP for specified interval */
#endif /* NOFRILLS */
    "sort",        XXSORT, CM_INV,	/* (see ARRAY) */
#else
    "sort",        XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef MAC
#ifndef NOFRILLS
    "sp",          XXSPA, CM_INV|CM_ABR,
    "spa",         XXSPA, CM_INV|CM_ABR,
#endif /* NOFRILLS */
    "space",       XXSPA, 0,		/* Show available disk SPACE */
#endif /* MAC */

#ifndef NOFRILLS
#ifndef NOPUSH
    "spawn",       XXSHE, CM_INV,	/* Synonym for PUSH, RUN */
#else
    "spawn",       XXNOTAV, CM_INV,	/* Synonym for PUSH, RUN */
#endif /* NOPUSH */
#endif /* NOFRILLS */

#ifndef NOXFER
    "sta",         XXSTA, CM_INV|CM_ABR,
    "stat",        XXSTA, CM_INV|CM_ABR,
    "statistics",  XXSTA, 0,		/* Display file transfer stats */
#endif /* NOXFER */

    "status",      XXSTATUS, 0,		/* Show status of previous command */

#ifndef NOSPL
    "stop",        XXSTO, 0,		/* STOP all take files and macros */
    "succeed",     XXSUCC, 0,		/* SUCCEED */
#else
    "stop",        XXNOTAV, CM_INV,
    "succeed",     XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOFRILLS
    "support",     XXBUG, 0,		/* BUG report instructions */
#else
    "support",     XXNOTAV, CM_INV,
#endif /* NOFRILLS */

#ifndef NOJC
    "suspend",     XXSUS, 0,		/* SUSPEND C-Kermit (UNIX only) */
#else
    "suspend",     XXNOTAV, CM_INV,
#endif /* NOJC */

#ifndef NOSPL
    "switch",      XXSWIT, 0,		/* SWITCH */
#else
    "switch",      XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifdef CK_TAPI
    "ta",	   XXTAK, CM_INV|CM_ABR, /* (because of TAPI) */
#endif /* CK_TAPI */

    "take",	   XXTAK, 0,		/* TAKE commands from a file */

#ifdef CK_TAPI
    "tapi",	   XXTAPI, 0,		/* Microsoft TAPI commands */
#else
    "tapi",	   XXNOTAV, CM_INV,
#endif /* CK_TAPI */

#ifndef NOFRILLS
#ifdef TCPSOCKET
    "tel",         XXTEL, CM_INV|CM_ABR,
    "telnet",      XXTEL, 0,		/* TELNET (TCP/IP only) */
    "telopt",      XXTELOP, CM_INV,     /* TELOPT (ditto) */
#else
    "tel",         XXNOTAV, CM_INV,
    "telnet",      XXNOTAV, CM_INV,
    "telopt",      XXNOTAV, CM_INV,
#endif /* TCPSOCKET */
#ifdef OS2
    "terminal",    XXTERM, 0,		/* == SET TERMINAL TYPE */
#else
    "terminal",    XXTERM, CM_INV,
#endif /* OS2 */
#endif /* NOFRILLS */
#ifndef NOXFER
    "text",        XXASC, CM_INV,	/* == SET FILE TYPE TEXT */
#endif /* NOXFER */

#ifndef NOSPL
    "trace",       XXTRACE, 0,		/* TRACE */
#else
    "trace",       XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOCSETS
    "translate",   XXXLA, 0,		/* TRANSLATE local file char sets */
#else
    "translate",   XXNOTAV, CM_INV,
#endif /* NOCSETS */

#ifndef NOXMIT
    "transmit",    XXTRA, 0,		/* Send (upload) a file, no protocol */
#else
    "transmit",    XXNOTAV, CM_INV,
#endif /* NOXMIT */

#ifndef NOFRILLS
    "type",        XXTYP, 0,		/* Display a local file */
#endif /* NOFRILLS */

#ifndef NOSPL
    "undefine",    XXUNDEF, 0,		/* UNDEFINE a variable or macro */
#else
    "undefine",    XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifdef COMMENT
    "updates",     XXUPD, 0,		/* View UPDATES file */
#endif /* COMMENT */

    "version",     XXVER, 0,		/* VERSION-number display */

#ifdef OS2
    "viewonly",    XXVIEW, 0,		/* VIEWONLY Terminal Mode */
#endif /* OS2 */

#ifndef NOSPL
    "wait",        XXWAI, 0,		/* WAIT (like pause) */
#else
    "wait",        XXNOTAV, CM_INV,
#endif /* NOSPL */

    "wermit",      XXKERMI, CM_INV,

#ifndef NOXFER
    "where",       XXWHERE, 0,		/* WHERE (did my file go?) */
#endif /* NOXFER */

#ifndef NOSPL
    "while",       XXWHI, 0,		/* WHILE loop */
#else
    "while",       XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef OS2
#ifndef MAC
#ifndef NOFRILLS
    "who",         XXWHO, 0,		/* WHO's logged in? */
#endif /* NOFRILLS */
#endif /* MAC */
#endif /* OS2 */

#ifndef NOHELP
    "wildcards",   XXWILD,CM_INV|CM_HLP, /* Wildcard syntax */
#endif /* NOHELP */

#ifndef NOSPL
    "wr",          XXWRI, CM_INV|CM_ABR,
    "wri",         XXWRI, CM_INV|CM_ABR,
    "writ",        XXWRI, CM_INV|CM_ABR,
    "write",       XXWRI, 0,		/* WRITE characters to a file */
    "write-line",  XXWRL, CM_INV,	/* WRITE a line to a file */
    "writeln",     XXWRL, CM_INV,	/* Pascalisch synonym for write-line */
#else
    "wr",          XXNOTAV, CM_INV,
    "wri",         XXNOTAV, CM_INV,
    "writ",        XXNOTAV, CM_INV,
    "write",       XXNOTAV, CM_INV,
    "write-line",  XXNOTAV, CM_INV,
    "writeln",     XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOFRILLS
    "xecho",       XXXECH,0,		/* XECHO */
#endif /* NOFRILLS */

#ifndef NOSPL
    "xif",         XXIFX, CM_INV,	/* Extended IF command (obsolete) */
#else
    "xif",         XXNOTAV, CM_INV,
#endif /* NOSPL */

#ifndef NOCSETS
    "xlate",       XXXLA, CM_INV,	/* Synonym for TRANSLATE */
#else
    "xlate",       XXNOTAV, CM_INV,
#endif /* NOCSETS */

#ifndef NOXMIT
    "xmit",        XXTRA, CM_INV,	/* Synonym for TRANSMIT */
#else
    "xmit",        XXNOTAV, CM_INV,
#endif /* NOXMIT */

#ifndef OS2
#ifndef NOJC
    "z",           XXSUS, CM_INV,	/* Synonym for SUSPEND */
#else
    "z",           XXNOTAV, CM_INV,
#endif /* NOJC */
#endif /* OS2 */

#ifdef CK_RECALL
    "^",           XXREDO,CM_INV|CM_NOR, /* Synonym for REDO */
#endif /* CK_RECALL */
#ifndef NOSPL
    "_asg",        XXASX,   CM_INV,	/* Used internally by FOR, etc */
    "_assign",     XXASX,   CM_INV,	/* Used internally by FOR, etc */
    "_decrement",  XX_DECR, CM_INV,
    "_define",     XXDFX,   CM_INV,	/* Used internally by FOR, etc */
    "_evaluate",   XX_EVAL, CM_INV,
    "_forward",    XXXFWD,  CM_INV,	/* Used internally by SWITCH   */
    "_getargs",    XXGTA,   CM_INV,	/* Used internally by FOR, etc */
    "_increment",  XX_INCR, CM_INV,
    "_putargs",    XXPTA,   CM_INV,	/* Used internally by FOR, etc */
#endif /* NOSPL */
      "", 0, 0
};
int ncmd = (sizeof(cmdtab) / sizeof(struct keytab)) - 1;

char toktab[] = {
#ifndef NOPUSH
    '!',				/* Shell escape */
#endif /* NOPUSH */
    '#',				/* Comment */
#ifndef NOSPL
    '.',				/* Assignment */
#endif /* NOSPL */
    ';',				/* Comment */
#ifndef NOSPL
    ':',				/* Label */
#endif /* NOSPL */
#ifndef NOPUSH
#ifdef CK_REDIR
    '<',				/* REDIRECT */
#endif /* CK_REDIR */
    '@',				/* DCL escape */
#endif /* NOPUSH */
#ifdef CK_RECALL
    '^',				/* Command recall */
#endif /* CK_RECALL */
#ifndef NOSPL
    '{',				/* Immediate macro */
#endif /* NOSPL */
    '\0'				/* End of this string */
};
int xxdot = 0;				/* Used with "." token */

struct keytab yesno[] = {		/* Yes/No keyword table */
    "no",    0, 0,
    "ok",    1, 0,
    "yes",   1, 0
};
int nyesno = (sizeof(yesno) / sizeof(struct keytab));

/* Save keyword table */

struct keytab savtab[] = {
#ifdef OS2
    "command",  XSCMD, 0,
#endif /* OS2 */
#ifndef NOSETKEY
    "keymap",   XSKEY, 0,
#endif	/* NOSETKEY */
#ifdef OS2
    "terminal", XSTERM, 0,
#endif /* OS2 */
	"", 	0, 	0
};
int nsav = (sizeof(savtab) / sizeof(struct keytab)) - 1;

/* Parameter keyword table */

struct keytab prmtab[] = {
    "alarm",            XYALRM,  0,
    "ask-timer",        XYTIMER, 0,
#ifndef NOXFER
    "attributes",       XYATTR,  0,
#endif /* NOXFER */
#ifdef CK_AUTHENTICATION
    "authentication",   XYAUTH,  0,
#else /* CK_AUTHENTICATION */
#ifdef CK_SSL
    "authentication",   XYAUTH,  0,
#endif /* CK_SSL */
#endif /* CK_AUTHENTICATION */
    "b",		XYBACK,  CM_INV|CM_ABR,
    "ba",		XYBACK,  CM_INV|CM_ABR,
#ifdef VMS
    "background",       XYBACK,  CM_INV,
    "batch",            XYBACK,  0,
#else
    "background",       XYBACK,  0,
    "batch",            XYBACK,  CM_INV,
#endif /* VMS */
#ifndef NOLOCAL
    "baud",	        XYSPEE,  CM_INV,
#endif /* NOLOCAL */
    "bell",             XYBELL,  0,
#ifndef NOXFER
    "block-check",  	XYCHKT,  0,
#endif /* NOXFER */
#ifdef OS2
#ifdef BPRINT
    "bprinter",         XYBDCP,  CM_INV,
#endif /* BPRINT */
#endif /*  OS2 */
#ifdef BROWSER
    "browser",          XYBROWSE,0,
#endif /* BROWSER */
#ifndef NOXFER
#ifdef DYNAMIC
    "buffers",          XYBUF,   0,
#endif /* DYNAMIC */
#endif /* NOXFER */
#ifndef NOLOCAL
#ifndef MAC
    "carrier-watch",    XYCARR,  0,
#endif /* MAC */
#endif /* NOLOCAL */
#ifndef NOSPL
    "case",             XYCASE,  0,
#endif /* NOSPL */
    "cd",               XYCD,    0,
#ifndef NOXFER
    "cl",               XYCLEAR, CM_INV|CM_ABR,
    "cle",              XYCLEAR, CM_INV|CM_ABR,
    "clea",             XYCLEAR, CM_INV|CM_ABR,
    "clear",            XYCLEAR, CM_INV|CM_ABR,
    "clear-channel",    XYCLEAR, 0,
    "clearchannel",     XYCLEAR, CM_INV,
#endif /* NOXFER */
#ifndef NOLOCAL
    "close-on-disconnect", XYDISC, CM_INV,
#endif /* NOLOCAL */
    "cmd",              XYCMD,   CM_INV,
    "command",          XYCMD,   0,
#ifdef CK_SPEED
    "con",              XYQCTL,  CM_INV|CM_ABR,
#endif /* CK_SPEED */
    "console",          XYCMD,   CM_INV,
#ifdef CK_SPEED
    "control-character",XYQCTL,  0,
#endif /* CK_SPEED */
#ifndef NOSPL
    "count",            XYCOUN,  0,
#endif /* NOSPL */
#ifndef NOXFER
    "d",		XYDELA,  CM_INV|CM_ABR,
    "de",		XYDELA,  CM_INV|CM_ABR,
#endif /* NOXFER */
    "debug",            XYDEBU,  CM_INV,
#ifdef VMS
    "default",          XYDFLT,  0,
#else
#ifndef MAC
    "default",          XYDFLT,  CM_INV,
#endif /* MAC */
#endif /* VMS */
#ifndef NOXFER
    "delay",	    	XYDELA,  0,
    "destination",	XYDEST,  0,
#endif /* NOXFER */
#ifndef NODIAL
    "di",		XYDIAL,  CM_INV|CM_ABR,
    "dia",		XYDIAL,  CM_INV|CM_ABR,
    "dial",             XYDIAL,  0,
#endif /* NODIAL */
#ifdef OS2
    "dialer",		XYDLR,   CM_INV,
#endif /* OS2 */
#ifndef NOLOCAL
    "disconnect",       XYDISC,  0,
    "duplex",	    	XYDUPL,  0,
#endif /* NOLOCAL */
#ifndef NOPUSH
#ifndef NOFRILLS
    "editor",           XYEDIT,  0,
#endif /*  NOFRILLS */
#endif /* NOPUSH */
#ifdef CK_CTRLZ
    "eof",              XYEOF,   CM_INV,
#endif /* CK_CTRLZ */
#ifndef NOLOCAL
    "escape-character", XYESC,   0,
#endif /* NOLOCAL */
#ifndef NOSPL
    "evaluate",         XYEVAL,  CM_INV,
#endif /* NOSPL */
    "exit",		XYEXIT,  0,
#ifndef NOXFER
    "f-ack-bug",        XYFACKB, CM_INV,
    "f-ack-path",       XYFACKP, CM_INV,
#endif /* NOXFER */
    "file", 	  	XYFILE,  0,
    "fl",           	XYFLOW,  CM_INV|CM_ABR,
#ifndef NOSPL
    "flag",             XYFLAG,  0,
#endif /* NOSPL */
#ifdef BROWSER
    "ftp-client",       XYFTP,   0,
#endif /* BROWSER */
    "flow-control", 	XYFLOW,  0,
#ifndef NOSPL
    "function",         XYFUNC,  0,
#endif /* NOSPL */
    "handshake",    	XYHAND,  0,
    "hints",            XYHINTS, 0,
#ifdef NETCONN
    "host",             XYHOST,  0,
#endif /* NETCONN */
#ifndef NOSPL
    "i",		XYINPU,  CM_INV|CM_ABR,
    "in",		XYINPU,  CM_INV|CM_ABR,
#endif /* NOSPL */
#ifndef NOXFER
    "incomplete",   	XYIFD,   CM_INV,
#endif /* NOXFER */
#ifndef NOSPL
    "input",            XYINPU,  0,
#endif /* NOSPL */
#ifndef NOSETKEY
    "key",		XYKEY,   0,
#endif /* NOSETKEY */
    "l",                XYLINE,  CM_INV|CM_ABR,
#ifndef NOCSETS
    "language",         XYLANG,  0,
#endif /* NOCSETS */
#ifndef NOLOCAL
    "line",             XYLINE,  0,
    "local-echo",	XYLCLE,  CM_INV,
#endif /* NOLOCAL */
#ifndef NOSPL
    "login",		XYLOGIN, 0,
#endif /* NOSPL */
#ifndef NOSPL
    "macro",            XYMACR,  0,
#endif /* NOSPL */
#ifdef COMMENT
#ifdef VMS
    "messages",         XYMSGS,  0,
#endif /* VMS */
#endif /* COMMENT */
#ifndef NODIAL
    "modem",		XYMODM,	 0,
#endif /* NODIAL */
#ifndef NOLOCAL
#ifdef OS2MOUSE
    "mouse",		XYMOUSE, 0,
#endif /* OS2MOUSE */
#endif /* NOLOCAL */
#ifdef OS2
    "mskermit",         XYMSK,   0,
#endif /* OS2 */
#ifdef NETCONN
    "network",          XYNET,   0,
#endif /* NETCONN */
#ifndef NOSPL
    "output",           XYOUTP,  0,
#endif /* NOSPL */
    "options",          XYOPTS,  0,
    "pause",            XYSLEEP, CM_INV,
#ifdef ANYX25
#ifndef IBMX25
    "pad",              XYPAD,   0,
#endif /* IBMX25 */
#endif /* ANYX25 */
    "parity",	    	XYPARI,   0,
#ifndef NOLOCAL
#ifdef OS2
    "port",             XYLINE,   0,
#else
    "port",             XYLINE,   CM_INV,
#endif /* OS2 */
#endif /* NOLOCAL */
#ifndef NOFRILLS
    "pr",   	    	XYPROM,  CM_INV|CM_ABR,
    "printer",          XYPRTR,  0,
#endif /* NOFRILLS */
#ifdef OS2
    "priority",         XYPRTY,  0,
#endif /* OS2 */
#ifdef CK_SPEED
    "prefixing",        XYPREFIX, 0,
#endif /* CK_SPEED */
#ifndef NOFRILLS
    "prompt",	    	XYPROM,  0,
#endif /* NOFRILLS */
#ifndef NOXFER
    "protocol",		XYPROTO, 0,
#endif /* NOXFER */
    "q",		XYQUIE,  CM_INV|CM_ABR,
#ifndef NOXFER
    "q8flag",           XYQ8FLG, CM_INV,
#endif /* NOXFER */
#ifdef QNX
    "qnx-port-lock",    XYQNXPL, 0,
#else
    "qnx-port-lock",    XYQNXPL, CM_INV,
#endif /* QNX */
    "quiet",		XYQUIE,  0,
#ifndef NOXFER
    "rec",              XYRECV,  CM_INV|CM_ABR,
    "receive",          XYRECV,  0,
    "recv",             XYRECV,  CM_INV,
#endif /* NOXFER */
    "reliable",         XYRELY,  0,
#ifndef NOXFER
    "repeat",           XYREPT,  0,
    "retry-limit",      XYRETR,  0,
#endif /* NOXFER */
#ifndef NOSCRIPT
    "script",		XYSCRI,  0,
#endif /* NOSCRIPT */
#ifndef NOXFER
    "send",             XYSEND,  0,
#ifndef NOLOCAL
#ifndef NOSERVER
    "ser",              XYSERV,  CM_INV|CM_ABR,
#endif /* NOSERVER */
#endif /* NOXFER */
    "serial",           XYSERIAL,0,
#endif /* NOLOCAL */
#ifndef NOSERVER
    "server",           XYSERV,  0,
#endif /* NOSERVER */
#ifdef SESLIMIT
#ifndef NOLOCAL
    "session-l",        XYSESS,  CM_INV|CM_ABR,
#endif /* NOLOCAL */
    "session-limit",    XYLIMIT, CM_INV,        /* Session Limit */
#endif /* SESLIMIT */

#ifndef NOLOCAL
#ifdef OS2ORUNIX
    "session-log",      XYSESS,  0,
#else
#ifdef OSK
    "session-log",      XYSESS,  0,
#endif /* OSK */
#endif /* OS2ORUNIX */
#endif /* NOLOCAL */

    "sleep",            XYSLEEP, 0,

#ifndef NOLOCAL
    "speed",	        XYSPEE,  0,
#endif /* NOLOCAL */

#ifndef NOSPL
    "startup-file",     XYSTARTUP, CM_INV,
#endif /* NOSPL */

#ifndef NOLOCAL
#ifdef HWPARITY
    "stop-bits",        XYSTOP, 0,
#endif /* HWPARITY */
#endif /* NOLOCAL */

#ifndef NOXFER
#ifdef STREAMING
    "streaming",        XYSTREAM, 0,
#endif /* STREAMING */
#endif /* NOXFER */

#ifndef NOJC
    "suspend",          XYSUSP,  0,
#endif /* NOJC */
#ifdef CKSYSLOG
    "syslog",           XYSYSL,  CM_INV,
#endif /* CKSYSLOG */
    "take",             XYTAKE,  0,

#ifdef CK_TAPI
    "tapi",             XYTAPI,  0,
#endif /* CK_TAPI */

#ifndef NOTCPOPTS
#ifdef TCPSOCKET
    "tcp",              XYTCP, 0,
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */

#ifdef TNCODE
    "tel",              XYTEL,   CM_INV|CM_ABR,
    "telnet",           XYTEL,   0,
    "telopt",           XYTELOP, 0,
#endif /* TNCODE */

#ifndef NOSPL
    "temp-directory",   XYTMPDIR,0,
#endif /* NOSPL */

#ifndef NOLOCAL
    "terminal",         XYTERM,  0,
#endif /* NOLOCAL */

#ifdef OS2
    "title",		XYTITLE, 0,
#endif /* OS2 */
#ifdef TLOG
    "transaction-log",  XYTLOG,  0,
#endif /* TLOG */
#ifndef NOXFER
    "transfer",         XYXFER,  0,
#endif /* NOXFER */
#ifndef NOXMIT
    "transmit",         XYXMIT,  0,
#endif /* NOXMIT */
#ifndef NOXFER
#ifndef NOCSETS
    "unknown-char-set", XYUNCS,  0,
#endif /* NOCSETS */
#endif /* NOXFER */
    "wait",             XYSLEEP, CM_INV,
#ifndef NOPUSH
#ifdef UNIX
    "wildcard-expansion", XYWILD, 0,
#endif /* UNIX */
#endif /* NOPUSH */
#ifdef NT
    "w",                XYWIND,  CM_INV|CM_ABR,
    "wi",               XYWIND,  CM_INV|CM_ABR,
    "win",              XYWIND,  CM_INV|CM_ABR,
#endif /* NT */
    "window-size",      XYWIND,  0,
#ifdef NT
    "win95",            XYWIN95, 0,
#endif /* NT */
#ifdef ANYX25
    "x.25",             XYX25,   0,
    "x25",              XYX25,   CM_INV,
#endif /* ANYX25 */
    "xfer",             XYXFER,  CM_INV,
#ifndef NOXMIT
    "xmit",             XYXMIT,  CM_INV,
#endif /* NOXMIT */
    "", 0, 0
};
int nprm = (sizeof(prmtab) / sizeof(struct keytab)) - 1; /* How many */

struct keytab scntab[] = {
    "clear",   SCN_CLR, 0,
    "cleol",   SCN_CLE, 0,
    "move-to", SCN_MOV, 0
};
int nscntab = (sizeof(scntab) / sizeof(struct keytab)); /* How many */

/* Table of networks */
#ifdef NETCONN
struct keytab netkey[] = {
    "directory",     XYNET_D,  0,
    "type",          XYNET_T,  0
};
int nnetkey = (sizeof(netkey) / sizeof(struct keytab));

struct keytab netcmd[] = {
/*
  These are the network types.
*/
#ifdef NETCMD
    "command",       NET_CMD,  CM_INV,	/* Command */
#endif /* NETCMD */

#ifdef DECNET				/* DECnet / PATHWORKS */
    "decnet",        NET_DEC,  0,
#endif /* DECNET */

#ifdef NETDLL
    "dll",           NET_DLL,  CM_INV,  /* DLL to be loaded */
#endif /* NETDLL */

#ifdef NETFILE
    "file",           NET_FILE, CM_INV,  /* FILE (real crude) */
#endif /* NETFILE */

#ifdef NPIPE				/* Named Pipes */
    "named-pipe",     NET_PIPE,  0,
#endif /* NPIPE */

#ifdef CK_NETBIOS
    "netbios",        NET_BIOS,  0,	/* NETBIOS */
#endif /* CK_NETBIOS */

#ifdef DECNET				/* DECnet / PATHWORKS (alias) */
    "pathworks",     NET_DEC,  CM_INV,
#endif /* DECNET */

#ifdef NETCMD
    "pipe",          NET_CMD,  0,	/* Pipe */
#endif /* NETCMD */

#ifdef NETPTY
    "pseudoterminal", NET_PTY, 0,	/* Pseudoterminal */
#endif /* NETPTY */

#ifdef NETPTY
    "pty",           NET_PTY,  CM_INV,	/* Invisible synonym for pseudoterm */
#endif /* NETPTY */

#ifdef SSH
    "ssh",           NET_SSH,  CM_INV,  /* SSH */
#endif /* SSH */

#ifdef SUPERLAT
   "superlat",        NET_SLAT,  0,	/* Meridian Technologies' SuperLAT */
#endif /* SUPERLAT */

#ifdef TCPSOCKET			/* TCP/IP sockets library */
    "tcp/ip",       NET_TCPB,    0,
#endif /* TCPSOCKET */
#ifdef SUPERLAT
    "tes32",        NET_SLAT,   0,	/* Emulux TES32 */
#endif /* SUPERLAT */
#ifdef ANYX25				/* X.25 */
#ifdef SUNX25
    "x",            NET_SX25, CM_INV|CM_ABR,
    "x.25",         NET_SX25, 0,
    "x25",          NET_SX25, CM_INV,
#else
#ifdef STRATUSX25
    "x",            NET_VX25, CM_INV|CM_ABR,
    "x.25",         NET_VX25, 0,
    "x25",          NET_VX25, CM_INV,
#endif /* STRATUSX25 */
#endif /* SUNX25 */
#ifdef IBMX25
    "x",            NET_IX25, CM_INV|CM_ABR,
    "x.25",         NET_IX25, CM_INV,
    "x25",          NET_IX25, CM_INV,
#endif /* IBMX25 */
#ifdef HPX25
    "x",            NET_IX25, CM_INV|CM_ABR,
    "x.25",         NET_IX25, 0,
    "x25",          NET_IX25, CM_INV,
#endif /* HPX25 */
#endif /* ANYX25 */
    "", 0, 0
};
int nnets = (sizeof(netcmd) / sizeof(struct keytab));

#ifndef NOTCPOPTS
#ifdef TCPSOCKET

/* TCP options */

struct keytab tcpopt[] = {
   "address",   XYTCP_ADDRESS, 0,
#ifdef CK_DNS_SRV
   "dns-service-records", XYTCP_DNS_SRV, 0,
#endif /* CK_DNS_SRV */
#ifdef SO_DONTROUTE
    "dontroute",   XYTCP_DONTROUTE, 0,
#endif /* SO_DONTROUTE */
#ifdef SO_KEEPALIVE
   "keepalive", XYTCP_KEEPALIVE, 0,
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
   "linger", XYTCP_LINGER, 0,
#endif  /* SO_LINGER */
#ifdef TCP_NODELAY
   "nagle",  XYTCP_NAGLE,    CM_INV,
   "nodelay", XYTCP_NODELAY, 0,
#endif /* TCP_NODELAY */
   "reverse-dns-lookup", XYTCP_RDNS, 0,
#ifdef SO_RCVBUF
   "recvbuf", XYTCP_RECVBUF, 0,
#endif /* SO_RCVBUF */
#ifdef SO_SNDBUF
   "sendbuf", XYTCP_SENDBUF, 0,
#endif /* SO_SNDBUF */
#ifdef VMS
#ifdef DEC_TCPIP
   "ucx-port-bug", XYTCP_UCX, 0,
#endif /* DEC_TCPIP */
#endif /* VMS */
   "",0,0
};
int ntcpopt = (sizeof(tcpopt) / sizeof(struct keytab));

#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */

#endif /* NETCONN */

#ifdef OS2
/* K95 Manual Chapter Table -- Keep these two tables in sync! */

static char * linktbl[] = {		/* Internal links in k95.htm */
    "#top",				/* 00 */
    "#what",				/* 01 */
    "#install",				/* 02 */
    "#start",				/* 03 */
    "#dialer",				/* 04 */
    "#entries",				/* 05 */
    "#command",				/* 06 */
    "#terminal",			/* 07 */
    "#transfer",			/* 08 */
    "#hostmode"				/* 09 */
};

static struct keytab chaptbl[] = {
    "Command-Screen",     6, 0,
    "Contents",           0, 0,
    "Dialer-Entries",     5, 0,
    "File-Transfer",      8, 0,
    "Getting-Started",    3, 0,
    "Host-Mode",          9, 0,
    "Installation",       2, 0,
    "Terminal-Emulation", 7, 0,
    "Using-The-Dialer",   4, 0,
    "What-Is-K95",        1, 0,
    "",                   0, 0,
};
static int nchaptbl = (sizeof(chaptbl) / sizeof(struct keytab));
#endif /* OS2 */

#ifndef NOXFER
/* Remote Command Table */

struct keytab remcmd[] = {
#ifndef NOSPL
    "as",	 XZASG, CM_INV|CM_ABR,	/*  */
    "asg",	 XZASG, CM_INV,
    "assign",	 XZASG, 0,
#endif /* NOSPL */
    "cd",        XZCWD, 0,
    "copy",      XZCPY, 0,
    "cwd",       XZCWD, CM_INV,
    "delete",    XZDEL, 0,
    "directory", XZDIR, 0,
    "exit",      XZXIT, 0,
    "help",      XZHLP, 0,
#ifndef NOPUSH
    "host",      XZHOS, 0,
#endif /* NOPUSH */
#ifndef NOFRILLS
    "kermit",    XZKER, 0,
    "l",         XZLGI, CM_ABR|CM_INV,
    "lo",        XZLGI, CM_ABR|CM_INV,
    "log",       XZLGI, CM_ABR|CM_INV,
    "login",     XZLGI, 0,
    "logout",    XZLGO, 0,
    "mkdir",     XZMKD, 0,
    "print",     XZPRI, 0,
#endif /* NOFRILLS */
    "pwd",       XZPWD, 0,
#ifndef NOSPL
    "query",	 XZQUE, 0,
#endif /* NOSPL */
    "rename",    XZREN, 0,
    "rmdir",     XZRMD, 0,
    "set",       XZSET, 0,
    "space",	 XZSPA, 0
#ifndef NOFRILLS
,   "type", 	 XZTYP, 0,
    "who",  	 XZWHO, 0
#endif /* NOFRILLS */
};
int nrmt = (sizeof(remcmd) / sizeof(struct keytab));
#endif /* NOXFER */

struct keytab logtab[] = {
#ifdef CKLOGDIAL
    "connections",  LOGM, CM_INV,
    "cx",           LOGM, 0,
#endif /* CKLOGDIAL */
#ifdef DEBUG
    "debugging",    LOGD, 0,
#endif /* DEBUG */
    "packets",	    LOGP, 0,
#ifndef NOLOCAL
    "session",      LOGS, 0,
#endif /* NOLOCAL */
#ifdef TLOG
    "transactions", LOGT, 0,
#endif /* TLOG */
    "", 0, 0
};
int nlog = (sizeof(logtab) / sizeof(struct keytab)) - 1;

struct keytab writab[] = {
#ifndef NOSPL
    "append-file",     LOGW, CM_INV,
#endif /* NOSPL */
    "debug-log",       LOGD, 0,
    "error",           LOGE, 0,
#ifndef NOSPL
    "file",            LOGW, 0,
#endif /* NOSPL */
    "packet-log",      LOGP, 0,
    "screen",          LOGX, 0,
#ifndef NOLOCAL
    "session-log",     LOGS, 0,
#endif /* NOLOCAL */
    "sys$output",      LOGX, CM_INV,
    "t",               LOGT, CM_ABR|CM_INV, /* Because of a typo in */
    "tr",              LOGT, CM_ABR|CM_INV, /* the book... */
    "tra",             LOGT, CM_ABR|CM_INV,
    "tran",            LOGT, CM_ABR|CM_INV,
    "trans",           LOGT, CM_ABR|CM_INV,
    "transa",          LOGT, CM_ABR|CM_INV,
    "transac",         LOGT, CM_ABR|CM_INV,
    "transact",        LOGT, CM_ABR|CM_INV,
    "transacti",       LOGT, CM_ABR|CM_INV,
    "transactio",      LOGT, CM_ABR|CM_INV,
    "transaction",     LOGT, CM_ABR|CM_INV,
    "transaction-log", LOGT, 0,
    "transactions",    LOGT, CM_INV
};
int nwri = (sizeof(writab) / sizeof(struct keytab));

#ifdef COMMENT				/* INPUT switches not used yet... */
static struct keytab inswtab[] = {
#ifdef COMMENT
    "/assign",       IN_ASG, CM_ARG,
#endif /* COMMENT */
    "/autodownload", IN_ADL, CM_ARG,
    "/case",         IN_CAS, CM_ARG,
    "/echo",         IN_ECH, CM_ARG,
    "/interrupts",   IN_NOI, CM_ARG,
    "/silence",      IN_SIL, CM_ARG,
#ifdef COMMENT
    "/pattern",      IN_PAT, CM_ARG,
#endif /* COMMENT */
    "", 0, 0
};
static int ninswtab = (sizeof(inswtab) / sizeof(struct keytab)) - 1;
#endif /* COMMENT */

static struct keytab clrtab[] = {	/* Keywords for CLEAR command */
#ifndef NOSPL
    "alarm",            CLR_ALR,         0,
#ifdef CK_APC
    "apc",              CLR_APC,         0,
#endif /* CK_APC */
#ifdef PATTERNS
    "binary-patterns",  CLR_BIN,         0,
#endif /* PATTERNS */
    "both",             CLR_DEV|CLR_INP, CM_INV,
#endif /* NOSPL */
#ifdef OS2
    "command-screen",   CLR_CMD,         0,
#endif /* OS2 */
#ifndef NOSPL
    "device",           CLR_DEV,         CM_INV|CM_ABR,
    "device-and-input", CLR_DEV|CLR_INP, 0,
#endif /* NOSPL */
    "device-buffer",    CLR_DEV,         0,
#ifndef NODIAL
    "dial-status",      CLR_DIA,	 0,
#endif /* NODIAL */
#ifndef NOSPL
    "input-buffer",     CLR_INP,         0,
#endif /* NOSPL */
    "send-list",        CLR_SFL,         0,
#ifdef OS2
    "scr",              CLR_SCL,         CM_INV|CM_ABR,
#endif /* OS2 */
    "screen",           CLR_SCR,         0,
#ifdef OS2
    "scrollback",       CLR_SCL,         CM_INV,
    "terminal-screen",  CLR_TRM,         0,
#endif /* OS2 */
#ifdef PATTERNS
    "text-patterns",    CLR_TXT,         0,
#endif /* PATTERNS */
    "", 0, 0
};
int nclear = (sizeof(clrtab) / sizeof(struct keytab)) - 1;

struct keytab clstab[] = {		/* Keywords for CLOSE command */
#ifndef NOSPL
    "!read",           LOGR, CM_INV,
    "!write",          LOGW, CM_INV,
#ifndef NOPUSH
#endif /* NOPUSH */
#endif /* NOSPL */
#ifndef NOSPL
    "append-file",     LOGW, CM_INV,
#endif /* NOSPL */
#ifndef NOLOCAL
    "connection",      9999, 0,
#endif /* NOLOCAL */
#ifdef CKLOGDIAL
    "cx-log",          LOGM, 0,
#endif /* CKLOGDIAL */
#ifdef DEBUG
    "debug-log",       LOGD, 0,
#endif /* DEBUG */
    "host",            9999, CM_INV,	/* Synonym for CLOSE CONNECTION */
    "line",            9999, CM_INV,	/* Synonym for CLOSE CONNECTION */
    "packet-log",      LOGP, 0,
    "port",            9999, CM_INV,	/* Synonym for CLOSE CONNECTION */
#ifndef NOSPL
    "read-file",       LOGR, 0,
#endif /* NOSPL */
#ifndef NOLOCAL
    "session-log",     LOGS, 0,
#endif /* NOLOCAL */
#ifdef TLOG
    "t",               LOGT, CM_ABR|CM_INV, /* Because of a typo in */
    "tr",              LOGT, CM_ABR|CM_INV, /* the book... */
    "tra",             LOGT, CM_ABR|CM_INV,
    "tran",            LOGT, CM_ABR|CM_INV,
    "trans",           LOGT, CM_ABR|CM_INV,
    "transa",          LOGT, CM_ABR|CM_INV,
    "transac",         LOGT, CM_ABR|CM_INV,
    "transact",        LOGT, CM_ABR|CM_INV,
    "transacti",       LOGT, CM_ABR|CM_INV,
    "transactio",      LOGT, CM_ABR|CM_INV,
    "transaction",     LOGT, CM_ABR|CM_INV,
    "transaction-log", LOGT, 0,
    "transactions",    LOGT, CM_INV,
#endif /* TLOG */
#ifndef NOSPL
    "write-file",      LOGW, 0,
#endif /* NOSPL */
    "", 0, 0
};
int ncls = (sizeof(clstab) / sizeof(struct keytab)) - 1;

/* SHOW command arguments */

#ifndef NOSHOW
struct keytab shotab[] = {
#ifndef NOSPL
    "alarm", SHALRM, 0,
    "arg",  SHARG, CM_INV|CM_ABR,
    "arguments", SHARG, 0,
    "args", SHARG, CM_INV,
    "arrays", SHARR, 0,
#endif /* NOSPL */

#ifndef NOCSETS
    "associations", SHASSOC, 0,
#endif /* NOCSETS */

#ifndef NOXFER
    "attributes", SHATT, 0,
#endif /* NOXFER */

#ifdef CK_AUTHENTICATION
    "authentication", SHOAUTH, CM_INV,
#endif /* CK_AUTHENTICATION */

#ifndef NOPUSH
#ifdef BROWSER
    "browser", SHBROWSE, 0,
#endif /*  BROWSER */
#endif /* NOPUSH */
    "cd", SHCD, 0,
    "character-sets", SHCSE, 0,
    "cmd",  SHCMD, CM_INV,
#ifndef NOLOCAL
    "com",  SHCOM, CM_INV|CM_ABR,
    "comm", SHCOM, CM_INV|CM_ABR,
    "communications", SHCOM, 0,
#endif /* NOLOCAL */
    "command", SHCMD, 0,
#ifdef CK_SPEED
    "control-prefixing", SHCTL, 0,
#endif /* CK_SPEED */
#ifdef CKLOGDIAL
    "connection", SHCONNX, 0,
    "cx", SHCONNX, CM_INV,
#endif /* CKLOGDIAL */
#ifndef NOSPL
    "count", SHCOU, 0,
#endif /* NOSPL */
    "d",       SHDIA, CM_INV|CM_ABR,
#ifdef VMS
    "default", SHDFLT, 0,
#else
    "default", SHDFLT, CM_INV,
#endif /* VMS */
#ifndef NODIAL
    "dial", SHDIA, 0,
#endif /* NODIAL */
    "double/ignore", SHDBL, 0,
#ifndef NOPUSH
#ifndef NOFRILLS
    "editor",        SHEDIT, 0,
#endif /*  NOFRILLS */
#endif /* NOPUSH */
#ifndef NOLOCAL
    "escape", SHESC, 0,
#endif /* NOLOCAL */
    "exit", SHEXI, 0,
    "extended-options", SHXOPT, CM_INV,
    "features", SHFEA, 0,
    "file", SHFIL, 0,
#ifndef NOLOCAL
    "flow-control", SHOFLO, 0,
#endif /* NOLOCAL */
#ifdef BROWSER
    "ftp", SHOFTP, 0,
#endif /* BROWSER */
#ifndef NOSPL
    "functions", SHFUN, 0,
    "globals", SHVAR, 0,
#endif /* NOSPL */
    "ignore/double", SHDBL, CM_INV,
#ifndef NOSPL
    "input", SHINP, 0,
#endif /* NOSPL */
#ifndef NOSETKEY
    "k",   SHKEY, CM_INV|CM_ABR,
    "key", SHKEY, 0,
#ifndef NOKVERBS
    "kverbs", SHKVB, 0,
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#ifdef CK_LABELED
    "labeled-file-info", SHLBL, 0,
#endif /* CK_LABELED */
#ifndef NOCSETS
    "languages", SHLNG, 0,
#endif /* NOCSETS */
    "logs", SHLOG, 0,
#ifndef NOSPL
    "macros", SHMAC, 0,
#endif /* NOSPL */
#ifndef NODIAL
    "modem", SHMOD, 0,
#else
    "modem-signals", SHCOM, CM_INV,
#endif /* NODIAL */
#ifndef NOLOCAL
#ifdef OS2MOUSE
    "mouse", SHMOU, 0,
#endif /* OS2MOUSE */
#endif /* NOLOCAL */
#ifdef NETCONN
    "network", SHNET, 0,
#else
    "network", SHNET, CM_INV,
#endif /* NETCONN */
    "options", SHOPTS, 0,
#ifndef NOSPL
    "output", SHOUTP, CM_INV,
#endif /* NOSPL */
#ifdef ANYX25
#ifndef IBMX25
    "pad", SHPAD, 0,
#endif /* IBMX25 */
#endif /* ANYX25 */
    "parameters", SHPAR, CM_INV,
#ifdef PATTERNS
    "patterns", SHOPAT, 0,
#endif /* PATTERNS */
    "printer",  SHPRT, 0,
#ifdef CK_SPEED
    "prefixing", SHCTL, CM_INV,
#endif /* CK_SPEED */
#ifndef NOXFER
    "protocol", SHPRO, 0,
#endif /* NOXFER */
#ifndef NOSPL
    "scripts", SHSCR, 0,
#endif /* NOSPL */
    "send-list", SHSFL, 0,
#ifndef NOSERVER
    "server", SHSER, 0,
#endif /* NOSERVER */
    "stack", SHSTK, 0,
    "status", SHSTA, 0,
#ifdef STREAMING
    "streaming", SHOSTR, 0,
#endif /* STREAMING */
#ifndef NOLOCAL
#ifdef OS2
    "tabs",SHTAB, CM_INV,
#endif /* OS2 */
#ifdef CK_TAPI
    "tapi",          SHTAPI, 0,
    "tapi-comm",     SHTAPI_C, CM_INV,
    "tapi-location", SHTAPI_L, CM_INV,
    "tapi-modem",    SHTAPI_M, CM_INV,
#endif /* CK_TAPI */
#ifdef TNCODE
    "tel",      SHTEL,  CM_INV|CM_ABR,
    "telnet",   SHTEL,  0,
    "telopt",   SHTOPT, 0,
#endif /* TNCODE */
    "terminal", SHTER, 0,
#endif /* NOLOCAL */
#ifndef NOXMIT
    "tr",    SHXMI, CM_INV|CM_ABR,
    "tra",   SHXMI, CM_INV|CM_ABR,
    "tran",  SHXMI, CM_INV|CM_ABR,
    "trans", SHXMI, CM_INV|CM_ABR,
#endif /* NOXMIT */
#ifndef NOXFER
    "transfer", SHOXFER, 0,
#endif /* NOXFER */
#ifndef NOXMIT
    "transmit", SHXMI, 0,
#endif /* NOXMIT */
#ifdef CK_TRIGGER
    "trigger", SHTRIG, 0,
#endif /* CK_TRIGGER */
#ifndef NOSETKEY
#ifndef NOKVERBS
#ifdef OS2
     "udk", SHUDK, 0,
#endif /* OS2 */
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#ifndef NOSPL
    "variables", SHBUI, 0,
#endif /* NOSPL */
#ifndef NOFRILLS
    "versions", SHVER, 0,
#endif /* NOFRILLS */
#ifdef OS2
    "vscrn",    SHVSCRN, CM_INV,
#endif /* OS2 */
    "xfer", SHOXFER, CM_INV,
#ifndef NOXMIT
    "xmit", SHXMI, CM_INV,
#endif /* NOXMIT */
    "", 0, 0
};
int nsho = (sizeof(shotab) / sizeof(struct keytab)) - 1;
#endif /* NOSHOW */

#ifdef ANYX25
#ifndef IBMX25
struct keytab padtab[] = {              /* PAD commands */
    "clear",      XYPADL, 0,
    "interrupt",  XYPADI, 0,
    "reset",      XYPADR, 0,
    "status",     XYPADS, 0
};
int npadc = (sizeof(padtab) / sizeof(struct keytab));
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOSERVER
static struct keytab kmstab[] = {
    "both",    3, 0,
    "remote",  2, 0,
    "local",   1, 0
};

static struct keytab enatab[] = {	/* ENABLE commands */
    "all",        EN_ALL,  0,
#ifndef NOSPL
    "as",         EN_ASG,  CM_INV|CM_ABR,
    "asg",        EN_ASG,  CM_INV,
    "assign",     EN_ASG,  0,
#endif /* NOSPL */
#ifndef datageneral
    "bye",        EN_BYE,  0,
#endif /* datageneral */
    "cd",         EN_CWD,  0,
#ifdef ZCOPY
    "copy",       EN_CPY,  0,
#endif /* ZCOPY */
    "cwd",        EN_CWD,  CM_INV,
    "delete",     EN_DEL,  0,
    "directory",  EN_DIR,  0,
    "enable",     EN_ENA,  CM_INV,
    "exit",       EN_XIT,  0,
    "finish",     EN_FIN,  0,
    "get",        EN_GET,  0,
    "host",       EN_HOS,  0,
    "mail",       EN_MAI,  0,
    "mkdir",      EN_MKD,  0,
#ifndef NOSPL
    "query",      EN_QUE,  0,
#endif /* NOSPL */
    "print",      EN_PRI,  0,
    "rename",     EN_REN,  0,
    "retrieve",   EN_RET,  CM_INV,
    "rmdir",      EN_RMD,  0,
    "send",       EN_SEN,  0,
    "set",        EN_SET,  0,
    "space",      EN_SPA,  0,
    "type",       EN_TYP,  0,
    "who",        EN_WHO,  0
};
static int nena = (sizeof(enatab) / sizeof(struct keytab));
#endif /* NOSERVER */

#ifndef NOXFER
static struct keytab sndtab[] = {	/* SEND command options */
    "/after",           SND_AFT, CM_ARG,
#ifndef NOSPL
    "/array",           SND_ARR, CM_ARG,
#endif /* NOSPL */
    "/as-name",         SND_ASN, CM_ARG,
    "/b",               SND_BIN, CM_INV|CM_ABR,
    "/before",          SND_BEF, CM_ARG,
    "/binary",          SND_BIN, 0,
#ifdef CALIBRATE
    "/c",               SND_CMD, CM_INV|CM_ABR,
    "/calibrate",       SND_CAL, CM_INV|CM_ARG,
#endif /* CALIBRATE */
    "/command",         SND_CMD, 0,
    "/delete",          SND_DEL, 0,
#ifdef UNIXOROSK
    "/dotfiles",        SND_DOT, 0,
#endif /* UNIXOROSK */
    "/except",          SND_EXC, CM_ARG,
#ifdef PIPESEND
    "/filter",          SND_FLT, CM_ARG,
#endif /* PIPESEND */
    "/filenames",       SND_NAM, CM_ARG,
#ifdef VMS
    "/image",           SND_IMG, 0,
#else
    "/image",           SND_BIN, CM_INV,
#endif /* VMS */
#ifdef CK_LABELED
    "/labeled",         SND_LBL, 0,
#endif /* CK_LABELED */
    "/larger-than",     SND_LAR, CM_ARG,
    "/listfile",        SND_FIL, CM_ARG,
#ifndef NOFRILLS
    "/mail",            SND_MAI, CM_ARG,
#endif /* NOFRILLS */
#ifdef CK_TMPDIR
    "/move-to",         SND_MOV, CM_ARG,
#endif /* CK_TMPDIR */
    "/nobackup",        SND_NOB, 0,
#ifdef UNIXOROSK
    "/nodotfiles",      SND_NOD, 0,
#endif /* UNIXOROSK */
    "/not-after",       SND_NAF, CM_ARG,
    "/not-before",      SND_NBE, CM_ARG,
    "/pathnames",       SND_PTH, CM_ARG,
    "/print",           SND_PRI, CM_ARG,
#ifdef CK_XYZ
    "/protocol",        SND_PRO, CM_ARG,
#else
    "/protocol",        SND_PRO, CM_ARG|CM_INV,
#endif /* CK_XYZ */
    "/quiet",           SND_SHH, 0,
    "/recover",         SND_RES, 0,
#ifdef RECURSIVE
/* Systems where we do recursion */
    "/recursive",       SND_REC, 0,
#else
#ifdef VMS
/* Systems that do recursion themselves without our assistance */
/* if we give them the right kind of wildcard */
    "/recursive",       SND_REC, 0,
#else
#ifdef datageneral
    "/recursive",       SND_REC, 0,
#else
    "/recursive",       SND_REC, CM_INV,
#endif /* datageneral */
#endif /* VMS */
#endif /* RECURSIVE */
    "/rename-to",       SND_REN, CM_ARG,
    "/since",           SND_AFT, CM_INV|CM_ARG,
    "/smaller-than",    SND_SMA, CM_ARG,
    "/starting-at",     SND_STA, CM_ARG,
#ifndef NOFRILLS
    "/su",              SND_ASN, CM_ARG|CM_INV|CM_ABR,
    "/sub",             SND_ASN, CM_ARG|CM_INV|CM_ABR,
    "/subject",         SND_ASN, CM_ARG,
#endif /* NOFRILLS */
#ifdef RECURSIVE
    "/subdirectories",  SND_REC, CM_INV,
#endif /* RECURSIVE */
    "/text",            SND_TXT, 0
};
#define NSNDTAB sizeof(sndtab)/sizeof(struct keytab)
static int nsndtab = NSNDTAB;

#ifndef NOMSEND
static struct keytab msndtab[] = {	/* MSEND options */
    "/after",           SND_AFT, CM_ARG,
    "/before",          SND_BEF, CM_ARG,
    "/binary",          SND_BIN, 0,
    "/delete",          SND_DEL, 0,
    "/except",          SND_EXC, CM_ARG,
    "/filenames",       SND_NAM, CM_ARG,
#ifdef VMS
    "/image",           SND_IMG, 0,
#else
    "/image",           SND_BIN, CM_INV,
#endif /* VMS */
#ifdef CK_LABELED
    "/labeled",         SND_LBL, 0,
#endif /* CK_LABELED */
    "/larger-than",     SND_LAR, CM_ARG,
    "/list",            SND_FIL, CM_ARG,
#ifndef NOFRILLS
    "/mail",            SND_MAI, CM_ARG,
#endif /* NOFRILLS */
#ifdef CK_TMPDIR
    "/move-to",         SND_MOV, CM_ARG,
#endif /* CK_TMPDIR */
    "/not-after",       SND_NAF, CM_ARG,
    "/not-before",      SND_NBE, CM_ARG,
    "/pathnames",       SND_PTH, CM_ARG,
    "/print",           SND_PRI, CM_ARG,
#ifdef CK_XYZ
    "/protocol",        SND_PRO, CM_ARG,
#endif /* CK_XYZ */
    "/quiet",           SND_SHH, 0,
    "/recover",         SND_RES, 0,
    "/rename-to",       SND_REN, CM_ARG,
    "/since",           SND_AFT, CM_INV|CM_ARG,
    "/smaller-than",    SND_SMA, CM_ARG,
    "/starting-at",     SND_STA, CM_ARG,
#ifndef NOFRILLS
    "/subject",         SND_ASN, CM_ARG,
#endif /* NOFRILLS */
    "/text",            SND_TXT, 0
};
#define NMSNDTAB sizeof(msndtab)/sizeof(struct keytab)
static int nmsndtab = NMSNDTAB;
#endif /* NOMSEND */
#endif /* NOXFER */

/* CONNECT command switches */

#ifndef NOLOCAL
static struct keytab conntab[] = {
#ifdef XLIMITS
    "/idle-interval",   CONN_II, CM_ARG,
    "/idle-limit",      CONN_IL, CM_ARG,
    "/idle-string",     CONN_IS, CM_ARG,
    "/quietly",         CONN_NV, CM_INV
#else
    "/quietly",         CONN_NV, 0
#endif /* XLIMITS */
#ifdef XLIMITS
,   "/time-limit",      CONN_TL, CM_ARG
#endif /* XLIMITS */
#ifdef CK_TRIGGER
,   "/trigger",         CONN_TS, CM_ARG
#endif /* CK_TRIGGER */
};
#define NCONNTAB sizeof(conntab)/sizeof(struct keytab)
static int nconntab = NCONNTAB;
#endif /* NOLOCAL */

#ifndef NOXFER
static struct keytab stattab[] = {	/* STATISTICS command switches */
    "/brief", 1, 0,
    "/verbose", 0, 0
};
#endif /* NOXFER */

#ifndef NOSPL
#ifdef COMMENT
struct mtab mactab[MAC_MAX] = {		/* Preinitialized macro table */
    NULL, NULL, 0
};
#else
struct mtab *mactab;			/* Dynamically allocated macro table */
#endif /* COMMENT */
int nmac = 0;

struct keytab mackey[MAC_MAX];		/* Macro names as command keywords */
#endif /* NOSPL */

#ifndef NOSPL
#ifdef  OS2
struct keytab beeptab[] = {		/* Beep options */
    "error", BP_FAIL, 0,
    "information", BP_NOTE, 0,
    "warning", BP_WARN, 0
};
int nbeeptab = sizeof(beeptab)/sizeof(struct keytab);

/* CLEAR COMMMAND-SCREEN options */

#define CLR_C_ALL 0
#define CLR_C_BOL 1
#define CLR_C_BOS 2
#define CLR_C_EOL 3
#define CLR_C_EOS 4
#define CLR_C_LIN 5
#define CLR_C_SCR 6

struct keytab clrcmdtab[] = {
    "all",     CLR_C_ALL, 0,
    "bol",     CLR_C_BOL, 0,
    "bos",     CLR_C_BOS, 0,
    "eol",     CLR_C_EOL, 0,
    "eos",     CLR_C_EOS, 0,
    "line",    CLR_C_LIN, 0,
    "scrollback", CLR_C_SCR, 0
};
int nclrcmd = sizeof(clrcmdtab)/sizeof(struct keytab);
#endif /* OS2 */
#endif /* NOSPL */

#ifdef COMMENT
/* Not used at present */
static struct keytab pagetab[] = {
    "/more",   1, CM_INV,
    "/nopage", 0, 0,
    "/page",   1, 0
};
int npagetab = sizeof(pagetab)/sizeof(struct keytab);
#endif /* COMMENT */

#define TYP_NOP 0			/* /NOPAGE */
#define TYP_PAG 1			/* /PAGE */
#define TYP_HEA 2			/* /HEAD:n */
#define TYP_TAI 3			/* /TAIL:n */
#define TYP_PAT 4			/* /MATCH:pattern */
#define TYP_WID 5			/* /WIDTH:cols */
#define TYP_COU 6			/* /COUNT */
#define TYP_OUT 7			/* /OUTPUT:file */
#define TYP_PFX 8			/* /PREFIX:string */

static struct keytab typetab[] = {
    "/count",  TYP_COU, 0,
    "/head",   TYP_HEA, CM_ARG,
    "/match",  TYP_PAT, CM_ARG,
#ifdef CK_TTGWSIZ
    "/more",   TYP_PAG, CM_INV,
    "/nopage", TYP_NOP, 0,
    "/page",   TYP_PAG, 0,
#endif /* CK_TTGWSIZ */
    "/prefix", TYP_PFX, CM_ARG,
    "/tail",   TYP_TAI, CM_ARG,
    "/width",  TYP_WID, CM_ARG,
    "", 0, 0
};
int ntypetab = sizeof(typetab)/sizeof(struct keytab) - 1;

int typ_page = -1;			/* TYPE /[NO]PAGE default */
int typ_wid  = -1;

#ifndef NOSPL
#define TRA_ALL 999			/* TRACE command */
#define TRA_ASG 0
#define TRA_CMD 1

int tra_asg = 0;
int tra_cmd = 0;

static struct keytab tracetab[] = {	/* TRACE options */
    "all",            TRA_ALL, 0,
    "assignments",    TRA_ASG, 0,
    "command-level",  TRA_CMD, 0
};
static int ntracetab = sizeof(tracetab)/sizeof(struct keytab);
#endif /* NOSPL */

#ifndef NOSHOW
VOID
showtypopts() {
    printf(" TYPE ");
    if (typ_page > -1) {
	prtopt(&optlines,typ_page ? "/PAGE" : "/NOPAGE");
    } else
      prtopt(&optlines,"(no options set)");
    if (typ_wid > -1) {
	sprintf(tmpbuf,"/WIDTH:%d",typ_wid);
	prtopt(&optlines,tmpbuf);
    }
    prtopt(&optlines,"");
}
#endif /* NOSHOW */

int
settypopts() {				/* Set TYPE option defaults */
    int xp = -1;
    int c, getval;
    while (1) {
	if ((y = cmswi(typetab,ntypetab,"Switch","",xxstring)) < 0) {
	    if (y == -3)
	      break;
	    else
	      return(y);
	}
	c = cmgbrk();
	if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
	    printf("?This switch does not take an argument\n");
	    return(-9);
	}
	switch (y) {
	  case TYP_NOP: xp = 0; break;
	  case TYP_PAG: xp = 1; break;
	  case TYP_WID:
	    if (getval)
	      if ((x = cmnum("Column at which to truncate",
			     ckitoa(cmd_cols),10,&y,xxstring)) < 0)
		return(x);
	    typ_wid = y;
	    break;
          default:
	    printf("?Sorry, this option can not be set\n");
	    return(-9);
	}
    }
    if ((x = cmcfm()) < 0)		/* Get confirmation */
      return(x);
    if (xp > -1) typ_page = xp;		/* Confirmed, save defaults */
    return(success = 1);
}

/* Forward declarations of functions local to this module */

_PROTOTYP (int doask,   ( int  ) );
_PROTOTYP (int dodef,   ( int  ) );
_PROTOTYP (int dodel,   ( void ) );
_PROTOTYP (int dodial,  ( int  ) );
_PROTOTYP (int doelse,  ( void ) );
_PROTOTYP (int dofor,   ( void ) );
_PROTOTYP (int doincr,  ( int  ) );
_PROTOTYP (int dopaus,  ( int  ) );
#ifndef NOPUSH
_PROTOTYP (int doping,  ( void ) );
_PROTOTYP (int doftp,   ( void ) );
#endif /* NOPUSH */
#ifndef NORENAME
#ifndef NOFRILLS
_PROTOTYP (int dorenam, ( void ) );
#endif /* NOFRILLS */
#endif /* NORENAME */
#ifdef ZCOPY
_PROTOTYP (int docopy, ( void ) );
#endif /* ZCOPY */
#ifdef CK_REXX
_PROTOTYP (int dorexx,  ( void ) );
#endif /* CK_REXX */

#ifdef TCPSOCKET
static struct keytab telcmd[] = {
    "abort", TN_ABORT, CM_INV, /* Emotionally toned word - don't show */
    "ao",    TN_AO,    0,
    "ayt",   TN_AYT,   0,
    "break", BREAK,    0,
    "cancel",TN_ABORT, 0,
    "ec",    TN_EC,    0,
    "el",    TN_EL,    0,
    "eof",   TN_EOF,   0,
    "eor",   TN_EOR,   0,
    "ga",    TN_GA,    0,
    "ip",    TN_IP,    0,
    "dmark", TN_DM,    0,
    "do",    DO,       0,
    "dont",  DONT,     0,
    "nop",   TN_NOP,   0,
    "sak",   TN_SAK,   CM_INV,
    "sb",    SB,       0,
    "se",    SE,       0,
    "susp",  TN_SUSP,  0,
    "will",  WILL,     0,
    "wont",  WONT,     0
};
static int ntelcmd = (sizeof(telcmd) / sizeof(struct keytab));

static struct keytab tnopts[] = {
#ifdef CK_AUTHENTICATION
    "auth",   TELOPT_AUTHENTICATION,   CM_INV,
#endif /* CK_AUTHENTICATION */
    "binary", TELOPT_BINARY, 0,
    "com-port-control", TELOPT_COM_PORT, CM_INV,
    "echo", TELOPT_ECHO, 0,
#ifdef CK_ENCRYPTION
    "encrypt", TELOPT_ENCRYPTION, CM_INV,
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
    "kermit", TELOPT_KERMIT, 0,
#endif /* IKS_OPTION */
    "lflow",  TELOPT_LFLOW, CM_INV,
#ifdef CK_NAWS
    "naws", TELOPT_NAWS, 0,
#endif /* CK_NAWS */
#ifdef CK_ENVIRONMENT
    "new-environment", TELOPT_NEWENVIRON,  CM_INV,
#endif /* CK_ENVIRONMENT */
    "pragma-heartbeat",TELOPT_PRAGMA_HEARTBEAT,  CM_INV,
    "pragma-logon",    TELOPT_PRAGMA_LOGON,  CM_INV,
    "pragma-sspi",     TELOPT_SSPI_LOGON,  CM_INV,
    "sak",   TELOPT_IBM_SAK, CM_INV,
#ifdef CK_SNDLOC
    "send-location",   TELOPT_SNDLOC,  CM_INV,
#endif /* CK_SNDLOC */
    "sga", TELOPT_SGA, 0,
#ifdef CK_SSL
    "start-tls",       TELOPT_START_TLS,  CM_INV,
#endif /* CK_SSL */
    "ttype", TELOPT_TTYPE, 0,
#ifdef CK_ENVIRONMENT
    "xdisplay-location", TELOPT_XDISPLOC, CM_INV,
#endif /* CK_ENVIRONMENT */
    "", 0, 0
};
static int ntnopts = (sizeof(tnopts) / sizeof(struct keytab)) - 1;

static struct keytab tnsbopts[] = {
#ifdef CK_NAWS
    "naws", TELOPT_NAWS, 0,
#endif /* CK_NAWS */
    "", 0, 0
};
static int ntnsbopts = (sizeof(tnsbopts) / sizeof(struct keytab)) - 1;

#ifndef NOPUSH
int
doftp() {				/* FTP command */
    char *p;				/* for now just runs the ftp program */
    int x;

    if (network)			/* If we have a current connection */
      ckstrncpy(line,ttname,LINBUFSIZ);	/* get the host name */
    else *line = '\0';			/* as default host */
    for (p = line; *p; p++)		/* Remove ":service" from end. */
      if (*p == ':') { *p = '\0'; break; }
    if ((x = cmtxt("IP host name or number", line, &s, xxstring)) < 0)
      return(x);
    if (nopush) {
        printf("?Sorry, FTP command disabled\n");
        return(success = 0);
    }
/* Construct FTP command */
#ifdef VMS
#ifdef MULTINET				/* TGV MultiNet */
    sprintf(line,"multinet ftp %s",s);
#else
    sprintf(line,"ftp %s",s);		/* Other VMS TCP/IP's */
#endif /* MULTINET */
#else					/* Not VMS */
#ifdef OS2ORUNIX
    sprintf(line,"%s %s",ftpapp,s);
#ifdef OS2
    p = line + strlen(ftpapp);
    while (p != line) {
        if (*p == '/') *p = '\\';
        p--;
    }
#endif /* OS2 */
#else /* OS2ORUNIX */
    sprintf(line,"ftp %s",s);
#endif /* OS2ORUNIX */
#endif /* VMS */
    conres();				/* Make console normal  */
#ifdef DEC_TCPIP
    printf("\n");			/* Prevent prompt-stomping */
#endif /* DEC_TCPIP */
    x = zshcmd(line);
    concb((char)escape);
    return(success = x);
}

int
doping() {				/* PING command */
    char *p;				/* just runs ping program */
    int x;

    if (network)			/* If we have a current connection */
      ckstrncpy(line,ttname,LINBUFSIZ);	/* get the host name */
    else *line = '\0';			/* as default host to be pinged. */
    for (p = line; *p; p++)		/* Remove ":service" from end. */
      if (*p == ':') { *p = '\0'; break; }
    if ((x = cmtxt("IP host name or number", line, &s, xxstring)) < 0)
      return(x);
    if (nopush) {
        printf("?Sorry, PING command disabled\n");
        return(success = 0);
    }

    /* Construct PING command */
#ifdef VMS
#ifdef MULTINET				/* TGV MultiNet */
    sprintf(line,"multinet ping %s /num=1",s);
#else
    sprintf(line,"ping %s 56 1",s);	/* Other VMS TCP/IP's */
#endif /* MULTINET */
#else					/* Not VMS */
    sprintf(line,"ping %s",s);
#endif /* VMS */
    conres();				/* Make console normal  */
#ifdef DEC_TCPIP
    printf("\n");			/* Prevent prompt-stomping */
#endif /* DEC_TCPIP */
    x = zshcmd(line);
    concb((char)escape);
    return(success = x);
}
#endif /* NOPUSH */
#endif /* TCPSOCKET */


#ifndef NOXFER
static char * asnbuf = NULL;		/* As-name buffer pointer */

char sndxnam[] = { "_array_x_" };	/* (with replaceable x!) */

/*
  The new SEND command, replacing BSEND, CSEND, PSEND, etc etc.
  Call with cx = top-level keyword value.  Returns:
    < 0  On parse error.
    0    On other type of failure (e.g. requested operation not allowed).
    1    On success with sstate set to 's' so protocol will begin.
*/

/*  D O X S E N D  --  Parse SEND and related commands with switches  */

int
doxsend(cx) int cx; {
    int c, i, n, wild, confirmed = 0;	/* Workers */
    int x, y;				/* of the world... */
    int getval = 0;			/* Whether to get switch value */
    extern char * snd_move;		/* Directory to move sent files to */
    extern char * snd_rename;		/* What to rename sent files to */
    extern char * g_snd_move;
    extern char * g_snd_rename;
    extern char * filefile;		/* File containing filenames to send */
    extern struct keytab pathtab[];	/* PATHNAMES option keywords */
    extern int npathtab;		/* How many of them */
    extern int recursive;		/* Recursive directory traversal */
    extern int rprintf;			/* REMOTE PRINT flag */
    extern int fdispla;			/* TRANSFER DISPLAY setting */
    extern int skipbup;			/* Skip backup files when sending */
    struct stringint {			/* Temporary array for switch values */
	char * sval;
	int ival;
    } pv[SND_MAX+1];
    struct FDB sf, sw, fl, cm;		/* FDBs for each parse function */
    int mlist = 0;			/* Flag for MSEND or MMOVE */
    char * m;				/* For making help messages */
    extern struct keytab protos[];	/* File transfer protocols */
    extern int nprotos;
    extern char sndbefore[], sndafter[], *sndexcept[]; /* Selection criteria */
    extern char sndnbefore[], sndnafter[];
    extern long sndsmaller, sndlarger, calibrate;
#ifndef NOSPL
    int range[2];			/* Array range */
    char ** ap = NULL;			/* Array pointer */
    int arrayx = -1;			/* Array index */
#endif /* NOSPL */

    for (i = 0; i <= SND_MAX; i++) {	/* Initialize switch values */
	pv[i].sval = NULL;		/* to null pointers */
	pv[i].ival = -1;		/* and -1 int values */
    }
#ifndef NOSPL
    range[0] = -1;
    range[1] = -1;
    sndxin = -1;			/* Array index */
#endif /* NOSPL */
    sndarray = NULL;			/* Array pointer */

#ifdef UNIXOROSK
    g_matchdot = matchdot;		/* Match dot files */
#endif /* UNIXOROSK */
    g_recursive = recursive;		/* Recursive sending */
    recursive = 0;			/* Save global value, set local */
    debug(F101,"xsend entry fncnv","",fncnv);

    /* Preset switch values based on top-level command that called us */

    switch (cx) {
      case XXMSE:			/* MSEND */
	mlist = 1; break;
      case XXCSEN:			/* CSEND */
	pv[SND_CMD].ival = 1; break;
      case XXMMOVE:			/* MMOVE */
	mlist = 1;
      case XXMOVE:			/* MOVE */
	pv[SND_DEL].ival = 1; break;
      case XXRSEN:			/* RESEND */
	pv[SND_BIN].ival = 1;		/* Implies /BINARY */
	pv[SND_RES].ival = 1; break;
      case XXMAI:			/* MAIL */
	pv[SND_MAI].ival = 1; break;
    }

    /* Set up chained parse functions... */

    cmfdbi(&sw,				/* First FDB - command switches */
	   _CMKEY,			/* fcode */
	   "Filename, or switch",	/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
#ifdef NOMSEND
	   nsndtab,			/* addtl numeric data 1: tbl size */
#else
	   mlist ? nmsndtab : nsndtab,	/* addtl numeric data 1: tbl size */
#endif /* NOMSEND */
	   4,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
#ifdef NOMSEND
	   sndtab,			/* Keyword table */
#else
	   mlist ? msndtab : sndtab,
#endif /* NOMSEND */
	   &sf				/* Pointer to next FDB */
	   );
    cmfdbi(&sf,				/* 2nd FDB - file to send */
	   _CMIFI,			/* fcode */
	   "File(s) to send",		/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   xxstring,
	   NULL,
	   mlist ? &cm : &fl
	   );
    cmfdbi(&fl,				/* 3rd FDB - command to send from */
	   _CMFLD,			/* fcode */
	   "Command",			/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   xxstring,
	   NULL,
	   &cm
	   );
    cmfdbi(&cm,				/* 4th FDB - Confirmation */
	   _CMCFM,			/* fcode */
	   "",				/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   NULL,
	   NULL,
	   NULL
	   );

    while (1) {				/* Parse 0 or more switches */
	x = cmfdb(&sw);			/* Parse something */
	debug(F101,"xsend cmfdb","",x);
	if (x < 0)			/* Error */
	  goto xsendx;			/* or reparse needed */
	if (cmresult.fcode != _CMKEY)	/* Break out if not a switch */
	  break;
/*
  They gave a switch, but let's see how they terminated it.
  If they ended it with : or =, then we must parse a value.
  If they ended it with anything else, then we must NOT parse a value.
*/
	c = cmgbrk();			/* Get break character */
	getval = (c == ':' || c == '='); /* to see how they ended the switch */
	if (getval && !(cmresult.kflags & CM_ARG)) {
	    printf("?This switch does not take arguments\n");
	    x = -9;
	    goto xsendx;
	}
	if (!getval && (cmgkwflgs() & CM_ARG)) {
	    printf("?This switch requires an argument\n");
	    x = -9;
	    goto xsendx;
	}
	n = cmresult.nresult;		/* Numeric result = switch value */
	debug(F101,"xsend switch","",n);

	switch (n) {			/* Process the switch */
	  case SND_CMD:			/* These take no args */
	    if (nopush) {
		printf("?Sorry, system command access is disabled\n");
		x = -9;
		goto xsendx;
	    }
#ifdef PIPESEND
	    else if (sndfilter) {
		printf(
"?Sorry, no SEND /COMMAND or CSEND when SEND FILTER selected\n");
		x = -9;
		goto xsendx;
	    }
#endif /* PIPESEND */
	    sw.hlpmsg = "Command, or switch"; /* Change help message */
	    pv[n].ival = 1;		/* Just set the flag */
	    pv[SND_ARR].ival = 0;
	    break;

	  case SND_REC:			/* /RECURSIVE */
	    recursive = 2;		/* Set the real variable */
	    pv[SND_PTH].ival = PATH_REL; /* Give them relative pathnames */
	    pv[n].ival = 1;		/* Just set the flag */
	    break;

	  case SND_RES:			/* /RECOVER (resend) */
	    pv[SND_ARR].ival = 0;
	    pv[SND_BIN].ival = 1;	/* Implies /BINARY */
	  case SND_NOB:			/* /NOBACKUP */
	  case SND_DEL:			/* /DELETE */
	  case SND_SHH:			/* /QUIET */
	    pv[n].ival = 1;		/* Just set the flag */
	    break;

#ifdef UNIXOROSK
/* Like recursive, these are set immediately because they affect cmifi() */
	  case SND_DOT:			/* /DOTFILES */
	    matchdot = 1;
	    break;
	  case SND_NOD:			/* /NODOTFILES */
	    matchdot = 0;
	    break;
#endif /* UNIXOROSK */

	  /* File transfer modes - each undoes the others */

	  case SND_BIN:			/* Binary */
	  case SND_TXT:			/* Text */
	  case SND_IMG:			/* Image */
	  case SND_LBL:			/* Labeled */
	    pv[SND_BIN].ival = 0;
	    pv[SND_TXT].ival = 0;
	    pv[SND_IMG].ival = 0;
	    pv[SND_LBL].ival = 0;
	    pv[n].ival = 1;
	    break;

	  case SND_EXC:			/* Excludes */
	    if (!getval) break;
	    if ((x = cmfld("Pattern","",&s,xxstring)) < 0) {
		if (x == -3) {
		    printf("?Pattern required\n");
		    x = -9;
		}
		goto xsendx;
	    }
	    if (pv[n].sval) free(pv[n].sval);
	    y = strlen(s);
	    if (y > 256) {
		printf("?Pattern too long - 256 max\n");
		x = -9;
		goto xsendx;
	    }
	    pv[n].sval = malloc(y+1);
	    if (pv[n].sval) {
		strcpy(pv[n].sval,s);
		pv[n].ival = 1;
	    }
	    break;

	  case SND_MOV:			/* MOVE after */
	  case SND_REN:			/* RENAME after */
	    if (!getval) break;
	    if ((x = cmfld(n == SND_MOV ?
	   "device and/or directory for source file after sending" :
	   "new name for source file after sending",
			   "",
			   &s,
			   n == SND_MOV ? xxstring : NULL
			   )) < 0) {
		if (x == -3) {
		    printf("%s\n", n == SND_MOV ?
			   "?Destination required" :
			   "?New name required"
			   );
		    x = -9;
		}
		goto xsendx;
	    }
	    if (pv[n].sval) free(pv[n].sval);
	    s = brstrip(s);
	    y = strlen(s);
	    if (y > 0) {
		pv[n].sval = malloc(y+1);
		if (pv[n].sval) {
		    strcpy(pv[n].sval,s);
		    pv[n].ival = 1;
		}
	    }
	    break;

	  case SND_SMA:			/* Smaller / larger than */
	  case SND_LAR:
	    if (!getval) break;
	    if ((x = cmnum("Size in bytes","0",10,&y,xxstring)) < 0)
	      goto xsendx;
	    pv[n].ival = y;
	    break;

	  case SND_AFT:			/* Send /AFTER:date-time */
	  case SND_BEF:			/* Send /BEFORE:date-time */
	  case SND_NAF:			/* Send /NOT-AFTER:date-time */
	  case SND_NBE:			/* Send /NOT-BEFORE:date-time */
	    if (!getval) break;
	    if ((x = cmdate("File date-time","",&s,0,xxstring)) < 0) {
		if (x == -3) {
		    printf("?Date-time required\n");
		    x = -9;
		}
		goto xsendx;
	    }
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = malloc((int)strlen(s)+1);
	    if (pv[n].sval) {
		strcpy(pv[n].sval,s);
		pv[n].ival = 1;
	    }
	    break;

	  case SND_MAI:			/* Send as mail (= MAIL) */
#ifdef IKSD
	    if (inserver && !ENABLED(en_mai)) {
		printf("?Sorry, sending files as mail is disabled\n");
		return(-9);
	    }
#endif /* IKSD */
	    pv[n].ival = 1;
	    if (!getval) break;
	    if ((x = cmfld("e-mail address","",&s,xxstring)) < 0) {
		if (x == -3) {
		    printf("?address required\n");
		    x = -9;
		}
		goto xsendx;
	    }
	    s = brstrip(s);
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = malloc((int)strlen(s)+1);
	    if (pv[n].sval)
	      strcpy(pv[n].sval,s);
	    break;

	  case SND_PRI:			/* Send to be printed (REMOTE PRINT) */
#ifdef IKSD
	    if (inserver && !ENABLED(en_mai)) {
		printf("?Sorry, sending files for printing is disabled\n");
		return(-9);
	    }
#endif /* IKSD */
	    pv[n].ival = 1;
	    if (!getval) break;
	    if ((x = cmfld("Print options","",&s,xxstring)) < 0)
	      if (x != -3) goto xsendx;
	    s = brstrip(s);
	    if (pv[n].sval) free(pv[n].sval);
	    pv[n].sval = malloc((int)strlen(s)+1);
	    if (pv[n].sval)
	      strcpy(pv[n].sval,s);
	    break;

	  case SND_ASN:			/* As-name */
	    debug(F101,"xsend /as-name getval","",getval);
	    if (!getval) break;
	    if ((x = cmfld("Name to send under","",&s,NULL)) < 0) {
		if (x == -3) {
		    printf("?name required\n");
		    x = -9;
		}
		goto xsendx;
	    }
	    s = brstrip(s);
	    if ((y = strlen(s)) > 0) {
		if (pv[n].sval) free(pv[n].sval);
		pv[n].sval = malloc(y+1);
		if (pv[n].sval) {
		    strcpy(pv[n].sval,s);
		    pv[n].ival = 1;
		}
	    }
	    break;

	  case SND_STA:			/* Starting position (= PSEND) */
	    if (!getval) break;
	    if ((x = cmnum("0-based position","0",10,&y,xxstring)) < 0)
	      goto xsendx;
	    pv[n].ival = y;
	    break;

	  case SND_PRO:			/* Protocol to use */
	    if (!getval) break;
	    if ((x = cmkey(protos,nprotos,"File-transfer protocol","",
			   xxstring)) < 0) {
		if (x == -3) {
		    printf("?name of protocol required\n");
		    x = -9;
		}
		goto xsendx;
	    }
	    pv[n].ival = x;
	    break;

#ifdef PIPESEND
	  case SND_FLT:			/* Filter */
	    debug(F101,"xsend /filter getval","",getval);
	    if (!getval) break;
	    if ((x = cmfld("Filter program to send through","",&s,NULL)) < 0) {
		if (x == -3)
		  s = "";
		else
		  goto xsendx;
	    }
	    s = brstrip(s);
	    y = strlen(s);
	    for (x = 0; x < y; x++) {	/* Make sure they included "\v(...)" */
		if (s[x] != '\\') continue;
		if (s[x+1] == 'v') break;
	    }
	    if (x == y) {
		printf(
		"?Filter must contain a replacement variable for filename.\n"
		       );
		x = -9;
		goto xsendx;
	    }
	    pv[n].ival = 1;
	    if (pv[n].sval) {
		free(pv[n].sval);
		pv[n].sval = NULL;
	    }
	    if ((y = strlen(s)) > 0) {
		if (pv[n].sval = malloc(y+1))
		  strcpy(pv[n].sval,s);
	    }
	    break;
#endif /* PIPESEND */

	  case SND_PTH:			/* Pathnames */
	    if (!getval) {
		pv[n].ival = PATH_REL;
		break;
	    }
	    if ((x = cmkey(pathtab,npathtab,"","absolute",xxstring)) < 0)
	      goto xsendx;
	    pv[n].ival = x;
	    break;

	  case SND_NAM:			/* Filenames */
	    if (!getval) break;
	    if ((x = cmkey(fntab,nfntab,"","converted",xxstring)) < 0)
	      goto xsendx;
	    debug(F101,"xsend /filenames","",x);
	    pv[n].ival = x;
	    break;

#ifdef CALIBRATE
          case SND_CAL:			/* /CALIBRATE */
	    if (getval) {
		if ((x = cmnum("number of Kbytes to send",
			   "1024",10,&y,xxstring)) < 0)
		  goto xsendx;
	    } else
	      y = 1024;
	    pv[n].ival = y;
	    pv[SND_ARR].ival = 0;
	    break;
#endif /* CALIBRATE */

	  case SND_FIL:			/* Name of file containing filnames */
	    if (!getval) break;
	    if ((x = cmifi("Name of file containing list of filenames",
			       "",&s,&y,xxstring)) < 0) {
		if (x == -3) {
		    printf("?Filename required\n");
		    x = -9;
		}
		goto xsendx;
	    } else if (y) {
		printf("?Wildcards not allowed\n");
		x = -9;
		goto xsendx;
	    }
	    if (pv[n].sval)
	      free(pv[n].sval);
	    if (s) if (*s) {
		if (pv[n].sval = malloc((int)strlen(s)+1)) {
		    strcpy(pv[n].sval,s);
		    pv[n].ival = 1;
		    pv[SND_ARR].ival = 0;
		}
	    }
	    break;

#ifndef NOSPL
	  case SND_ARR:			/* SEND /ARRAY: */
	    if (!getval) break;
	    ap = NULL;
	    if ((x = cmfld("Array name (a single letter will do)",
			   "",
			   &s,
			   NULL
			   )) < 0) {
		if (x == -3)
		  break;
		else
		  return(x);
	    }
	    if ((x = arraybounds(s,&(range[0]),&(range[1]))) < 0) {
		printf("?Bad array: %s\n",s);
		return(-9);
	    }
	    if (!(ap = a_ptr[x])) {
		printf("?No such array: %s\n",s);
		return(-9);
	    }
	    pv[n].ival = 1;
	    pv[SND_CMD].ival = 0;	/* Undo any conflicting ones... */
	    pv[SND_RES].ival = 0;
	    pv[SND_CAL].ival = 0;
	    pv[SND_FIL].ival = 0;
	    arrayx = x;
	    break;
#endif /* NOSPL */

	  default:
	    printf("?Unexpected switch value - %d\n",cmresult.nresult);
	    x = -9;
	    goto xsendx;
	}
    }
    debug(F101,"xsend cmresult fcode","",cmresult.fcode);

#ifdef COMMENT
    /* List switch parsing results in debug log */
    for (i = 0; i <= SND_MAX; i++) {
	sprintf(line,"xsend switch %02d",i);
	debug(F111,line, pv[i].sval, pv[i].ival);
    }
#endif /* COMMENT */

/* Now we have all switches, plus maybe a filename or command, or nothing */

#ifdef PIPESEND
    if (protocol != PROTO_K && pv[SND_CMD].ival > 0) {
	printf("?Sorry, %s works only with Kermit protocol\n",
	       (cx == XXCSEN) ? "CSEND" : "SEND /COMMAND");
	x = -9;
	goto xsendx;
    }
    if (pv[SND_RES].ival > 0 ||	/* /RECOVER */
	pv[SND_STA].ival > 0) {	/* or /STARTING */
	if (sndfilter || pv[SND_FLT].ival > 0) {
	    printf("?Sorry, no /RECOVER or /START if SEND FILTER selected\n");
	    x = -9;
	    goto xsendx;
	}
    }
#endif /* PIPESEND */

    cmarg = "";
    cmarg2 = "";
    line[0] = NUL;
    s = line;
    wild = 0;

    switch (cmresult.fcode) {		/* How did we get out of switch loop */
      case _CMIFI:			/* Input filename */
	ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Name */
	if (pv[SND_ARR].ival > 0)
	  cmarg2 = line;
	else
	  wild = cmresult.nresult;	/* Wild flag */
	break;
      case _CMFLD:			/* Field */
	/* Only allowed with /COMMAND and /ARRAY */
	if (pv[SND_CMD].ival < 1 && pv[SND_ARR].ival < 1) {
	    printf("?%s - \"%s\"\n",
		   iswild(cmresult.sresult) ?
		   "No files match" : "File not found",
		   cmresult.sresult
		   );
	    x = -9;
	    goto xsendx;
	}
	ckstrncpy(line,cmresult.sresult,LINBUFSIZ);
	if (pv[SND_ARR].ival > 0)
	  cmarg2 = line;
	break;
      case _CMCFM:			/* Confirmation */
	/* s = ""; */
	confirmed = 1;
	break;
      default:
	printf("?Unexpected function code: %d\n",cmresult.fcode);
	x = -9;
	goto xsendx;
    }
    debug(F110,"xsend string",s,0);
    debug(F101,"xsend confirmed","",confirmed);

    /* Save and change protocol and transfer mode */
    /* Global values are restored in main parse loop */

    g_proto = protocol;			/* Save current global protocol */
    g_urpsiz = urpsiz;
    g_spsizf = spsizf;
    g_spsiz = spsiz;
    g_spsizr = spsizr;
    g_spmax = spmax;
    g_wslotr = wslotr;
    g_prefixing = prefixing;
    g_fncact = fncact;
    g_fncnv = fncnv;
    g_fnspath = fnspath;
    g_fnrpath = fnrpath;

    if (pv[SND_PRO].ival > -1) {	/* Change according to switch */
	protocol = pv[SND_PRO].ival;
        if (ptab[protocol].rpktlen > -1) /* copied from initproto() */
            urpsiz = ptab[protocol].rpktlen;
        if (ptab[protocol].spktflg > -1)
            spsizf = ptab[protocol].spktflg;
        if (ptab[protocol].spktlen > -1) {
            spsiz = ptab[protocol].spktlen;
            if (spsizf)
	      spsizr = spmax = spsiz;
        }
        if (ptab[protocol].winsize > -1)
            wslotr = ptab[protocol].winsize;
        if (ptab[protocol].prefix > -1)
            prefixing = ptab[protocol].prefix;
        if (ptab[protocol].fnca > -1)
            fncact  = ptab[protocol].fnca;
        if (ptab[protocol].fncn > -1)
            fncnv   = ptab[protocol].fncn;
        if (ptab[protocol].fnsp > -1)
            fnspath = ptab[protocol].fnsp;
        if (ptab[protocol].fnrp > -1)
            fnrpath = ptab[protocol].fnrp;
    }
    debug(F101,"xsend protocol","",protocol);

    if (pv[SND_NOB].ival > -1) {	/* /NOBACKUP (skip backup file) */
	g_skipbup = skipbup;
	skipbup = 1;
    }
    if (pv[SND_REC].ival > 0)		/* Recursive */
      recursive = 2;

    g_binary = binary;			/* Save global transfer mode */
#ifdef PATTERNS
    g_patterns = patterns;		/* Save FILE PATTERNS setting */
#endif /* PATTERNS */
    if (pv[SND_BIN].ival > 0) {		/* Change according to switch */
	/* If they said /BINARY they mean /BINARY */
#ifdef PATTERNS
	patterns = 0;			/* So no pattern-based switching */
#endif /* PATTERNS */
	g_xfermode = xfermode;		/* or automatic transfer mode */
	xfermode = XMODE_M;
	binary = XYFT_B;
	debug(F101,"doxsend /BINARY xfermode","",xfermode);
    } else if (pv[SND_TXT].ival > 0) {	/* Ditto for /TEXT */
#ifdef PATTERNS
	patterns = 0;
#endif /* PATTERNS */
	g_xfermode = xfermode;
	xfermode = XMODE_M;
	binary = XYFT_T;
	debug(F101,"doxsend /TEXT xfermode","",xfermode);
    } else if (pv[SND_IMG].ival > 0) {
#ifdef VMS
	binary = XYFT_I;
#else
	binary = XYFT_B;
#endif /* VMS */
    }
#ifdef CK_LABELED
    else if (pv[SND_LBL].ival > 0) {
	binary = XYFT_L;
    }
#endif /* CK_LABELED */
    debug(F101,"xsend binary","",binary);

    /* Check for legal combinations of switches, filenames, etc */

#ifdef PIPESEND
    if (pv[SND_CMD].ival > 0) {	/* COMMAND - strip any braces */
	debug(F110,"SEND /COMMAND before stripping",s,0);
	s = brstrip(s);
	debug(F110,"SEND /COMMAND after stripping",s,0);
	if (!*s) {
	    printf("?Sorry, a command to send from is required\n");
	    x = -9;
	    goto xsendx;
	}
    }
#endif /* PIPESEND */

/* Set up /MOVE and /RENAME */

    if (pv[SND_DEL].ival > 0 &&
	(pv[SND_MOV].ival > 0 || pv[SND_REN].ival > 0)) {
	printf("?Sorry, /DELETE conflicts with /MOVE or /RENAME\n");
	x = -9;
	goto xsendx;
    }
#ifdef CK_TMPDIR
    if (pv[SND_MOV].ival > 0) {
	int len;
	char * p = pv[SND_MOV].sval;
#ifdef CK_LOGIN
	if (isguest) {
	    printf("?Sorry, /MOVE-TO not available to guests\n");
	    x = -9;
	    goto xsendx;
	}
#endif /* CK_LOGIN */
	len = strlen(p);
	if (!isdir(p)) {		/* Check directory */
#ifdef CK_MKDIR
	    char * s = NULL;
	    s = (char *)malloc(len + 4);
	    if (s) {
		strcpy(s,p);
#ifdef datageneral
		if (s[len-1] != ':') { s[len++] = ':'; s[len] = NUL; }
#else
		if (s[len-1] != '/') { s[len++] = '/'; s[len] = NUL; }
#endif /* datageneral */
		s[len++] = 'X';
		s[len] = NUL;
		x = zmkdir(s);
		free(s);
		if (x < 0) {
		    printf("?Can't create \"%s\"\n",p);
		    x = -9;
		    goto xsendx;
		}
	    }
#else
	    printf("?Directory \"%s\" not found\n",p);
	    x = -9;
	    goto xsendx;
#endif /* CK_MKDIR */
	}
	makestr(&snd_move,p);
    }
#endif /* CK_TMPDIR */

    if (pv[SND_REN].ival > 0) {		/* /RENAME */
	char * p = pv[SND_REN].sval;
#ifdef CK_LOGIN
	if (isguest) {
	    printf("?Sorry, /RENAME-TO not available to guests\n");
	    x = -9;
	    goto xsendx;
	}
#endif /* CK_LOGIN */
	if (!p) p = "";
	if (!*p) {
	    printf("?New name required for /RENAME\n");
	    x = -9;
	    goto xsendx;
	}
	p = brstrip(p);
#ifndef NOSPL
    /* If name given is wild, rename string must contain variables */
	if (wild) {
	    char * s = tmpbuf;
	    x = TMPBUFSIZ;
	    zzstring(p,&s,&x);
	    if (!strcmp(tmpbuf,p)) {
		printf(
    "?/RENAME for file group must contain variables such as \\v(filename)\n"
		       );
		x = -9;
		goto xsendx;
	    }
	}
#endif /* NOSPL */
	makestr(&snd_rename,p);
    }

/* Handle /RECOVER and /START */

#ifdef CK_RESEND
    if (pv[SND_RES].ival > 0 && binary != XYFT_B
#ifdef PATTERNS
	&& !patterns
#else
#ifdef VMS
/* VMS sets text/binary automatically later when it opens the file */
	&& 0
#endif /* VMS */
#endif /* PATTERNS */
	) {
	printf("?Sorry, /BINARY required\n");
	x = -9;
	goto xsendx;
    }

    if (pv[SND_STA].ival > 0) {		/* /START */
	if (wild) {
	    printf("?Sorry, wildcards not permitted with /START\n");
	    x = -9;
	    goto xsendx;
	}
	if (sizeof(int) < 4) {
	    printf("?Sorry, this command needs 32-bit integers\n");
	    x = -9;
	    goto xsendx;
	}
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("?Sorry, SEND /START works only with Kermit protocol\n");
	    x = -9;
	    goto xsendx;
	}
#endif /* CK_XYZ */
    }
#ifdef CK_XYZ
    if (pv[SND_RES].ival > 0) {
	if (protocol != PROTO_K && protocol != PROTO_Z) {
	    printf(
    "Sorry, /RECOVER is possible only with Kermit or ZMODEM protocol\n"
		   );
	    x = -9;
	    goto xsendx;
	}
    }
#endif /* CK_XYZ */
#endif /* CK_RESEND */

    if (protocol == PROTO_K) {
	if ((pv[SND_MAI].ival > 0 ||	/* MAIL */
	     pv[SND_PRI].ival > 0 ||	/* PRINT */
	     pv[SND_RES].ival > 0	/* RESEND */
	     ) &&
	    (!atdiso || !atcapr)) {	/* Disposition attribute off? */
	    printf("?Sorry, ATTRIBUTE DISPOSITION must be ON\n");
	    x = -9;
	    goto xsendx;
	}
    }

#ifdef CK_XYZ
    if (wild && (protocol == PROTO_X || protocol == PROTO_XC)) {
	printf(
"Sorry, you can only send one file at a time with XMODEM protocol\n"
	       );
	x = -9;
	goto xsendx;
    }
#endif /* CK_XYZ */

    if (!confirmed) {			/* CR not typed yet, get more fields */
	char *m;
	if (mlist) {			/* MSEND or MMOVE */
	    nfils = 0;			/* We already have the first one */
#ifndef NOMSEND
	    msfiles[nfils++] = line;	/* Store pointer */
	    lp = line + (int)strlen(line) + 1; /* Point past it */
	    debug(F111,"xsend msend",msfiles[nfils-1],nfils-1);
	    while (1) {			/* Get more filenames */
		char *p;
		if ((x = cmifi("Names of files to send, separated by spaces",
			       "", &s,&y,xxstring)) < 0) {
		    if (x != -3)
		      goto xsendx;
		    if ((x = cmcfm()) < 0)
		      goto xsendx;
		    break;
		}
		msfiles[nfils++] = lp;	/* Got one, count it, point to it, */
		p = lp;			/* remember pointer, */
		while (*lp++ = *s++)	/* and copy it into buffer */
		  if (lp > (line + LINBUFSIZ)) { /* Avoid memory leak */
		      printf("?MSEND list too long\n");
		      line[0] = NUL;
		      x = -9;
		      goto xsendx;
		  }
		debug(F111,"xsend msend",msfiles[nfils-1],nfils-1);
		if (nfils == 1) fspec[0] = NUL; /* Take care of \v(filespec) */
#ifdef ZFNQFP
		zfnqfp(p,TMPBUFSIZ,tmpbuf);
		p = tmpbuf;
#endif /* ZFNQFP */
		if (((int)strlen(fspec) + (int)strlen(p) + 1) < fspeclen) {
		    strcat(fspec,p);
		    strcat(fspec," ");
		} else
#ifdef COMMENT
		  printf("WARNING - \\v(filespec) buffer overflow\n");
#else
		  debug(F101,"doxsend filespec buffer overflow","",0);
#endif /* COMMENT */
	    }
#endif /* NOMSEND */
	} else {			/* Regular SEND */
	    char *p; int y;
	    nfils = -1;
	    if (pv[SND_MAI].ival > 0)
	      m = (pv[SND_MAI].sval) ?
		"e-mail address (optional)" :
		  "e-mail address (required)";
	    else if (pv[SND_PRI].ival > 0)
	      m = "printer options (optional)";
	    else if (wild)
	      m =
"\nOptional as-name template containing replacement variables \
like \\v(filename)";
	    else
	      m = "Optional name to send it with";
	    if ((x = cmtxt(m,"",&p,NULL)) < 0)
	      goto xsendx;
	    if (!p) p = "";
	    if (*p) {			/* If some text was given... */
		p = brstrip(p);		/* Replace /AS-NAME: value if any */
		if ((y = strlen(p)) > 0) {
		    if (pv[SND_ASN].sval) free(pv[SND_ASN].sval);
		    pv[SND_ASN].sval = malloc(y+1);
		    if (pv[SND_ASN].sval) {
			strcpy(pv[SND_ASN].sval,p);
			pv[SND_ASN].ival = 1;
		    }
		}
	    }
	}
    }
    /* Set cmarg2 from as-name, however we got it. */

    if (pv[SND_ASN].ival > 0 && pv[SND_ASN].sval && !*cmarg2) {
	int x;
	x = strlen(line);
	ckstrncpy(line+x+2,pv[SND_ASN].sval,LINBUFSIZ-x-1);
	cmarg2 = line+x+2;
	debug(F110,"doxsend cmarg2",cmarg2,0);
    }

#ifndef NOFRILLS
    if ((pv[SND_MAI].ival > 0) && (pv[SND_PRI].ival > 0)) {
	printf("Sorry, /MAIL and /PRINT are conflicting options\n");
	x = -9;
	goto xsendx;
    }
    n = 0;				/* /MAIL or /PRINT? */
    if (pv[SND_MAI].ival > 0)
      n = SND_MAI;
    else if (pv[SND_PRI].ival > 0)
      n = SND_PRI;
    if (n) {				/* Yes... */
#ifdef DEBUG
	char * p;
	if (n == SND_MAI)
	  p = "/MAIL";
	else
	  p = "/PRINT";
	debug(F111,"xsend",p,n);
#endif /* DEBUG */
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, %s available only with Kermit protocol\n",
		   (n == SND_MAI) ? "/MAIL" : "/PRINT"
		   );
	    x = -9;
	    goto xsendx;
	}
#endif /* CK_XYZ */
	debug(F101,"xsend print/mail wild","",wild);
	*optbuf = NUL;			/* Wipe out any old options */
	s = pv[n].sval;			/* mail address or print switch val */
	if (!s) s = "";
	debug(F110,"doxsend mail address or printer options",s,0);
	if (n == SND_MAI && !*s) {
	    printf("?E-mail address required\n");
	    x = -9;
	    goto xsendx;
	} else if ((int)strlen(s) > 94) { /* Ensure legal size */
	    printf("?%s too long\n",
		   (n == SND_MAI) ?
		   "E-mail address" :
		   "Print option string"
		   );
	    x = -9;
	    goto xsendx;
	}
	strcpy(optbuf,s);		/* OK, copy to option buffer */
	cmarg = line;			/* File to send */
	if (n == SND_MAI) {
	    debug(F110,"xsend mailing",cmarg,0);
	    debug(F110,"xsend address:",optbuf,0);
	    rmailf = 1;
	} else {
	    debug(F110,"xsend printing",cmarg,0);
	    debug(F110,"xsend options",optbuf,0);
	    rprintf = 1;
	}
    }
#endif /* NOFRILLS */

#ifdef CALIBRATE
    if (pv[SND_CAL].ival > 0) {		/* Handle /CALIBRATE */
	if (confirmed) {
	    calibrate = pv[SND_CAL].ival * 1024L;
	    sndsrc = -9;
	    nfils = 1;
	    wild = 0;
#ifndef NOMSEND
	    addlist = 0;
#endif /* NOMSEND */
	    strcpy(line,"CALIBRATION");
	    s = cmarg = line;
	    if (!cmarg2) cmarg2 = "";
	    debug(F110,"doxsend cmarg2 calibrate",cmarg2,0);
	} else if (line[0]) {
	    calibrate = 0L;
	    pv[SND_CAL].ival = 0L;
	}
    }
#endif /* CALIBRATE */

    if (pv[SND_FIL].ival > 0) {
	if (confirmed && !calibrate) {
	    if (zopeni(ZMFILE,pv[SND_FIL].sval) < 1) {
		debug(F110,"xsend can't open",pv[SND_FIL].sval,0);
		printf("?Failure to open %s\n",filefile);
		x = -9;
		goto xsendx;
	    }
	    makestr(&filefile,pv[SND_FIL].sval); /* Open, remember name */
	    debug(F110,"xsend opened",filefile,0);
	    wild = 1;
	}
    }

    /* SEND alone... */

#ifndef NOSPL
    if (confirmed && pv[SND_ARR].ival > 0) {
	if (!*cmarg2) {
	    sndxnam[7] = (char)((arrayx == 1) ? 64 : arrayx + ARRAYBASE);
	    cmarg2 = sndxnam;
	}
	cmarg = "";
	goto sendend;
    }
#endif /* NOSPL */

    if (confirmed && !line[0] && !filefile && !calibrate) {
#ifndef NOMSEND
	if (filehead) {			/* OK if we have a SEND-LIST */
	    nfils = filesinlist;
	    sndsrc = nfils;		/* Like MSEND */
	    addlist = 1;		/* But using a different list... */
	    filenext = filehead;
	    if (pv[SND_DEL].ival > 0)	/* /DELETE given? */
	      moving = 1;
	    goto sendend;
	}
#endif /* NOMSEND */
	printf("?Filename required but not given\n");
	x = -9;
	goto xsendx;
    }

    /* Not send-list or array */

#ifndef NOMSEND
    addlist = 0;			/* Don't use SEND-LIST. */
    filenext = NULL;
#endif /* NOMSEND */

    if (mlist) {			/* MSEND or MMOVE */
#ifndef NOMSEND
	cmlist = msfiles;		/* List of files to send */
	sndsrc = nfils;
	cmarg2 = "";
	sendstart = 0L;
#endif /* NOMSEND */
#ifdef PIPESEND
	pipesend = 0;
#endif /* PIPESEND */
    } else if (filefile) {		/* File contains list of filenames */
	s = "";
	cmarg = "";
	cmarg2 = "";
	line[0] = NUL;
	nfils = 1;
	sndsrc = 1;

    } else if (!calibrate &&  pv[SND_ARR].ival < 1) {

	nfils = sndsrc = -1;	/* Not MSEND, MMOVE, /LIST, or /ARRAY */
	if (
#ifndef NOFRILLS
	    !rmailf && !rprintf		/* Not MAIL or PRINT */
#else
	    1
#endif /* NOFRILLS */
	    ) {
	    int y = 1;
	    if (!iswild(s))
	      y = zchki(s);
	    if (y < 0) {
		printf("?Read access denied - \"%s\"\n", s);
		x = -9;
		goto xsendx;
	    }
	    if (s != line)		/* We might already have done this. */
	      ckstrncpy(line,s,LINBUFSIZ); /* Copy of string just parsed. */
	    else
	      debug(F110,"doxsend line=s",line,0);
	    cmarg = line;		/* File to send */
	}
	zfnqfp(cmarg,fspeclen,fspec);
    }
    if (!mlist) {			/* For all but MSEND... */
#ifdef PIPESEND
	if (pv[SND_CMD].ival > 0)	/* /COMMAND sets pipesend flag */
	  pipesend = 1;
	debug(F101,"xsend /COMMAND pipesend","",pipesend);
	if (pipesend && filefile) {
	    printf("?Invalid switch combination\n");
	    x = -9;
	    goto xsendx;
	}
#endif /* PIPESEND */

#ifndef NOSPL
    /* If as-name given and filespec is wild, as-name must contain variables */
	debug(F111,"doxsend cmarg2 wild",cmarg2,wild);
	if (wild && *cmarg2) {
	    char * s = tmpbuf;
	    x = TMPBUFSIZ;
	    zzstring(cmarg2,&s,&x);
	    if (!strcmp(tmpbuf,cmarg2)) {
		printf(
    "?As-name for file group must contain variables such as \\v(filename)\n"
		       );
		x = -9;
		goto xsendx;
	    }
	}
#endif /* NOSPL */

    /* Strip braces from as-name */
	debug(F110,"xsend cmarg2 before stripping",cmarg2,0);
	cmarg2 = brstrip(cmarg2);
	debug(F110,"xsend filename",cmarg,0);
	debug(F110,"xsend as-name",cmarg2,0);

    /* Copy as-name to a safe place */

	if (asnbuf) {
	    free(asnbuf);
	    asnbuf = NULL;
	}
	if ((y = strlen(cmarg2)) > 0) {
	    asnbuf = (char *) malloc(y + 1);
	    if (asnbuf) {
		strcpy(asnbuf,cmarg2);
		cmarg2 = asnbuf;
	    } else cmarg2 = "";
	}

#ifdef CK_RESEND
	debug(F111,"xsend pv[SND_STA].ival","",pv[SND_STA].ival);
	if (pv[SND_STA].ival > -1) {	/* /START position */
	    if (wild) {
		printf("?/STARTING-AT may not be used with multiple files.\n");
		x = -9;
		goto xsendx;
	    } else
	      sendstart = pv[SND_STA].ival;
	} else
	  sendstart = 0L;
	debug(F110,"xsend /STARTING","",sendstart);
#endif /* CK_RESEND */
    }

sendend:				/* Common successful exit */
    moving = 0;
    if (pv[SND_SHH].ival > 0) {		/* SEND /QUIET... */
	g_displa = fdispla;
	fdispla = 0;
	debug(F101,"xsend display","",fdispla);
    }

#ifndef NOSPL				/* SEND /ARRAY... */
    if (pv[SND_ARR].ival > 0) {
	if (!ap) { x = -2; goto xsendx; } /* (shouldn't happen) */
	if (range[0] == -1)		/* If low end of range not specified */
	  range[0] = 1;			/* default to 1 */
	if (range[1] == -1)		/* If high not specified */
	  range[1] = a_dim[arrayx];	/* default to size of array */
	if ((range[0] < 0) ||		/* Check range */
	    (range[0] > a_dim[arrayx]) ||
	    (range[1] < range[0]) ||
	    (range[1] > a_dim[arrayx])) {
	    printf("?Bad array range - [%d:%d]\n",range[0],range[1]);
	    x = -9;
	    goto xsendx;
	}
	sndarray = ap;			/* Array pointer */
	sndxin = arrayx;		/* Array index */
	sndxlo = range[0];		/* Array range */
	sndxhi = range[1];
	sndxnam[7] = (char)((sndxin == 1) ? 64 : sndxin + ARRAYBASE);

#ifdef COMMENT
	printf("SENDING FROM ARRAY: &%c[]...\n", /* debugging */
	       (sndxin == 1) ? 64 : sndxin + ARRAYBASE);
	printf("Lo=%d\nHi=%d\n", sndxlo, sndxhi);
	printf("cmarg=[%s]\ncmarg2=[%s]\n", cmarg, cmarg2);
	while ((x = agnbyte()) > -1) {
	    putchar((char)x);
	}
	return(1);
#endif /* COMMENT */
    }
#endif /* NOSPL */

    if (pv[SND_ARR].ival < 1) {		/* File selection & disposition... */

	if (pv[SND_DEL].ival > 0)	/* /DELETE was specified */
	  moving = 1;
	debug(F110,"xsend /DELETE","",moving);
	if (pv[SND_AFT].ival > 0)	/* Copy SEND criteria */
	  ckstrncpy(sndafter,pv[SND_AFT].sval,19);
	if (pv[SND_BEF].ival > 0)
	  ckstrncpy(sndbefore,pv[SND_BEF].sval,19);
	if (pv[SND_NAF].ival > 0)
	  ckstrncpy(sndnafter,pv[SND_NAF].sval,19);
	if (pv[SND_NBE].ival > 0)
	  ckstrncpy(sndnbefore,pv[SND_NBE].sval,19);
	if (pv[SND_EXC].ival > 0)
	  makelist(pv[SND_EXC].sval,sndexcept,8);
	if (pv[SND_SMA].ival > -1)
	  sndsmaller = pv[SND_SMA].ival;
	if (pv[SND_LAR].ival > -1)
	  sndlarger = pv[SND_LAR].ival;
	if (pv[SND_NAM].ival > -1) {
	    g_fncnv = fncnv;		/* Save global value */
	    fncnv = pv[SND_NAM].ival;
	    debug(F101,"xsend fncnv","",fncnv);
	}
	if (pv[SND_PTH].ival > -1) {
	    g_spath = fnspath;		/* Save global values */
	    fnspath = pv[SND_PTH].ival;
#ifndef NZLTOR
	    if (fnspath != PATH_OFF) {
		g_fncnv = fncnv;	/* Bad bad... */
		fncnv = XYFN_C;
	    }
#endif /* NZLTOR */
	    debug(F101,"xsend fnspath","",fnspath);
	    debug(F101,"xsend fncnv","",fncnv);
	}
    }

#ifdef PIPESEND
    if (pv[SND_FLT].ival > 0) {
	g_sfilter = sndfilter;
	if (!pv[SND_FLT].sval) {
	    sndfilter = NULL;
	} else {
	    sndfilter = (char *) malloc((int) strlen(pv[SND_FLT].sval) + 1);
	    if (sndfilter) strcpy(sndfilter,pv[SND_FLT].sval);
	}
    }
#endif /* PIPESEND */

#ifdef CK_APC
/* MOVE not allowed in APCs */
    if (moving &&
	(apcactive == APC_LOCAL || apcactive == APC_REMOTE)
	&& apcstatus != APC_UNCH)
      return(success = 0);
#endif /* CK_APC */
#ifdef IKS_OPTION
    if (!iks_wait(KERMIT_REQ_START,1)) {
        printf("?A Kermit Server is unavailable to process this command.\n");
        printf("?Start a RECEIVE command to complement this command.\n");
    }
#endif /* IKS_OPTION */

#ifdef IKSD
#ifdef CK_LOGIN
    if (moving && inserver && isguest) {
        printf("?File deletion not allowed for guests.\n");
	return(-9);
    }
#endif /* CK_LOGIN */
#endif /* IKSD */

    sstate = 's';			/* Set start state to SEND */
    sndcmd = 1;
#ifdef CK_RESEND
    if (pv[SND_RES].ival > 0)		/* Send sendmode appropriately */
      sendmode = SM_RESEND;
    else if (pv[SND_STA].ival > 0)
      sendmode = SM_PSEND;
    else
#endif /* CK_RESEND */
    if (mlist)
      sendmode = SM_MSEND;
    else
      sendmode = SM_SEND;
#ifdef MAC
    what = W_SEND;
    scrcreate();
#endif /* MAC */
    if (local && pv[SND_SHH].ival != 0) { /* If in local mode, */
	displa = 1;			/* turn on file transfer display */
    }
    x = 0;

  xsendx:				/* Common exit, including failure */
    debug(F101,"doxsend sndsrc","",sndsrc);
    for (i = 0; i <= SND_MAX; i++) {	/* Free malloc'd memory */
	if (pv[i].sval)
	  free(pv[i].sval);
    }
    return(x);
}
#endif /* NOXFER */

#ifndef NOLOCAL
/*  D O X C O N N  --  CONNECT command parsing with switches */

#ifdef XLIMITS
#define XLIMORTRIGGER
#else
#ifdef CK_TRIGGER
#define XLIMORTRIGGER
#endif /* CK_TRIGGER */
#endif /*  XLIMITS */

#ifdef OS2				/* K95 only: */
extern int
  tt_idlesnd_tmo;			/*   Idle interval */
int tt_idlelimit = 0;			/*   Idle limit */
int tt_timelimit = 0;			/*   Time limit, 0 = none */
extern char *				/* Parse results - strings: */
  tt_idlesnd_str;			/*   Idle string */
#endif /* OS2 */

#ifdef CK_TRIGGER
extern char *tt_trigger[];
extern CHAR *tt_trmatch[];
extern char *triggerval;
#endif /* CK_TRIGGER */

int
doxconn(cx) int cx; {
    int c, i, n;			/* Workers */
    int x, y;
    int getval = 0;			/* Whether to get switch value */
    struct stringint {			/* Temporary array for switch values */
	char * sval;
	int ival;
    } pv[CONN_MAX+1];
    struct FDB sw, cm;			/* FDBs for each parse function */

#ifdef CK_TRIGGER
    char *g_tt_trigger[TRIGGERS];
#endif /* CK_TRIGGER */

#ifdef OS2
    int g_tt_idlesnd_tmo, g_tt_timelimit; /* For saving and restoring */
    int g_tt_idlelimit;
    char * g_tt_idlesnd_str;		/* global settings */

    g_tt_idlesnd_tmo = tt_idlesnd_tmo;	/* Save global settings */
    g_tt_timelimit   = tt_timelimit;
    g_tt_idlelimit   = tt_idlelimit;
    g_tt_idlesnd_str = tt_idlesnd_str;
#endif /* OS2 */

#ifdef CK_TRIGGER
    if (!tt_trigger[0]) {		/* First initialization */
	for (i = 1; i < TRIGGERS; i++)
	  tt_trigger[i] = NULL;
    }
    for (i = 0; i < TRIGGERS; i++)
      g_tt_trigger[i] = tt_trigger[i];
    if (triggerval) {
	free(triggerval);
	triggerval = NULL;
    }
#endif /* CK_TRIGGER */

    for (i = 0; i <= CONN_MAX; i++) {	/* Initialize switch values */
	pv[i].sval = NULL;		/* to null pointers */
	pv[i].ival = -1;		/* and -1 int values */
    }
    if (cx == XXCQ)			/* CQ == CONNECT /QUIETLY */
      pv[CONN_NV].ival = 1;

    /* Set up chained parse functions... */

    cmfdbi(&sw,				/* First FDB - command switches */
	   _CMKEY,			/* fcode */
	   "Switch",			/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   nconntab,			/* addtl numeric data 1: tbl size */
	   4,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
	   conntab,			/* Keyword table */
	   &cm				/* Pointer to next FDB */
	   );
    cmfdbi(&cm,				/* 2nd FDB - Confirmation */
	   _CMCFM,			/* fcode */
	   "",				/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   NULL,
	   NULL,
	   NULL
	   );

    while (1) {				/* Parse 0 or more switches */
	x = cmfdb(&sw);			/* Parse switch or confirmation */
	debug(F101,"doxconn cmfdb","",x);
	if (x < 0) {			/* Error */
	    if (x == -9 || x == -2)
	      printf("?No switches match - \"%s\"\n",atmbuf);
	    goto xconnx;		/* or reparse needed */
	}
	if (cmresult.fcode != _CMKEY)	/* Break out if not a switch */
	  break;
	c = cmgbrk();			/* Get break character */
	getval = (c == ':' || c == '='); /* to see how they ended the switch */
	if (getval && !(cmresult.kflags & CM_ARG)) {
	    printf("?This switch does not take arguments\n");
	    x = -9;
	    goto xconnx;
	}
	if (!getval && (cmgkwflgs() & CM_ARG)) {
	    printf("?This switch requires an argument\n");
	    return(-9);
	}
	n = cmresult.nresult;		/* Numeric result = switch value */
	debug(F101,"doxconn switch","",n);

	switch (n) {			/* Process the switch */
	  case CONN_NV:			/* Non-verbal */
	    pv[n].ival = 1;
	    break;
#ifdef XLIMITS
	  case CONN_II:			/* Idle-interval */
	  case CONN_IL:			/* Idle-limit */
	  case CONN_TL:			/* Time-limit */
	    if (!getval) break;
	    if ((x = cmnum("Seconds","0",10,&y,xxstring)) < 0)
	      goto xconnx;
	    pv[n].ival = y;
	    break;
	  case CONN_IS:			/* Idle-string */
#endif /* XLIMITS */
#ifdef CK_TRIGGER
	  case CONN_TS:			/* Trigger-string */
#endif /* CK_TRIGGER */
#ifdef XLIMORTRIGGER
	    if (!getval) break;
	    if ((x = cmfld("String (enclose in braces if it contains spaces)",
			   "",&s,xxstring)) < 0) {
		if (x == -3) {
		    printf("?String required\n");
		    x = -9;
		}
		goto xconnx;
	    }
	    if (n != CONN_TS)
	      s = brstrip(s);
	    if ((y = strlen(s)) > 0) {
		if (pv[n].sval) free(pv[n].sval);
		pv[n].sval = malloc(y+1);
		if (pv[n].sval) {
		    strcpy(pv[n].sval,s);
		    pv[n].ival = 1;
		}
	    }
	    break;
#endif /* XLIMORTRIGGER */
	  default:
	    printf("?Unexpected switch value - %d\n",cmresult.nresult);
	    x = -9;
	    goto xconnx;
	}
    }
    debug(F101,"doxconn cmresult.fcode","",cmresult.fcode);
    if (cmresult.fcode != _CMCFM) {
	printf("?Unexpected function code: %d\n",cmresult.fcode);
	x = -9;
	goto xconnx;
    }

#ifdef OS2				/* Make results available globally */
    if (pv[CONN_IL].ival > -1)		/* Idle limit */
      tt_idlelimit = pv[CONN_IL].ival;
    if (pv[CONN_II].ival > -1)		/* Idle limit */
      tt_idlesnd_tmo = pv[CONN_II].ival;
    if (pv[CONN_IS].sval)		/* Idle string */
      if (tt_idlesnd_str = (char *)malloc((int)strlen(pv[CONN_IS].sval)+1))
	strcpy(tt_idlesnd_str,pv[CONN_IS].sval);
    if (pv[CONN_TL].ival > -1)		/* Session limit */
      tt_timelimit = pv[CONN_TL].ival;
#endif /* OS2 */

#ifdef CK_TRIGGER
    if (pv[CONN_TS].sval)		/* Trigger strings */
      makelist(pv[CONN_TS].sval,tt_trigger,TRIGGERS);
    for (i = 0; i < TRIGGERS; i++)	/* Trigger match pointers */
      tt_trmatch[i] = NULL;
    if (triggerval) {			/* Reset trigger value */
	free(triggerval);
	triggerval = NULL;
    }
#endif /* CK_TRIGGER */

    x = doconect((pv[CONN_NV].ival > 0) ? 1 : 0);
#ifdef CKLOGDIAL
    {
	int xx;
	debug(F101,"doxconn doconect returns","",x);
	if ((xx = ttchk()) < 0) dologend();
	debug(F101,"doxconn ttchk returns","",xx);
    }
#endif /* CKLOGDIAL */

#ifdef CK_TRIGGER
    debug(F111,"doxconn doconect triggerval",triggerval,x);
#endif /* CK_TRIGGER */

    /* Back from CONNECT -- Restore global settings */

#ifdef OS2
    tt_idlelimit   = g_tt_idlelimit;
    tt_idlesnd_tmo = g_tt_idlesnd_tmo;
    tt_timelimit   = g_tt_timelimit;
    tt_idlesnd_str = g_tt_idlesnd_str;
#endif /* OS2 */

#ifdef CK_TRIGGER
    for (i = 0; i < TRIGGERS; i++)
      tt_trigger[i] = g_tt_trigger[i];
#endif /* CK_TRIGGER */

  xconnx:
    for (i = 0; i <= CONN_MAX; i++) {	/* Free malloc'd memory */
	if (pv[i].sval)
	  free(pv[i].sval);
    }
    success = (x > 0) ? 1 : 0;
    return(x);
}
#endif /* NOLOCAL */

#ifdef ADDCMD
/* cx == XXADD or XXREMV */
/* fc == ADD_BIN or ADD_TXT */
static int
doadd(cx,fc) int cx, fc; {
#ifdef PATTERNS
    char * tmp[FTPATTERNS];
    char **p = NULL;
    int i, n = 0, x = 0, last;
#endif /* PATTERNS */
    if (cx != XXADD && cx != XXREMV) {
	printf("?Unexpected function code: %d\n",cx);
	return(-9);
    }
#ifdef PATTERNS
    while (n < FTPATTERNS) {		/* Collect new patterns */
	tmp[n] = NULL;
	if ((x = cmfld("Pattern","",&s,xxstring)) < 0)
	  break;
	strcpy(line,s);
	s = brstrip(line);
	makestr(&(tmp[n++]),s);
    }
    if (x == -3)
      x = cmcfm();
    if (x < 0)
      goto xdoadd;
    p = (fc == ADD_BIN) ? binpatterns : txtpatterns; /* Which list */
    last = 0;
    for (i = 0; i < FTPATTERNS; i++) { /* Find last one in list */
	if (!p[i]) {
	    last = i;
	    break;
	}
    }
    if (cx == XXADD) {			/* Adding */
	if (last + n > FTPATTERNS) {	/* Check if too many */
	    printf("?Too many patterns - %d is the maximum\n", FTPATTERNS);
	    goto xdoadd;
	}
	for (i = 0; i < n; i++)		/* Copy in the new ones. */
	  makestr(&(p[i+last]),tmp[i]);
	makestr(&(p[i+last]),NULL);	/* Null-terminate the list */
	x = 1;
	goto xdoadd;			/* Done */
    } else if (cx == XXREMV) {		/* Remove something(s) */
	int j, k;
	if (last == 0)			        /* List is empty */
	  goto xdoadd;			        /* Nothing to remove */
	for (i = 0; i < n; i++) {	        /* i = Patterns they typed */
	    for (j = 0; j < last; j++) {        /* j = Patterns in list */
		/* Change this to ckstrcmp()... */
		if (filecase)
		  x = !strcmp(tmp[i],p[j]);     /* Case-sensitive match */
		else
		  x = ckstrcmp(tmp[i],p[j],-1,0); /* Case-independent match */
		if (x) {	    	        /* This one matches */
		    makestr(&(p[j]),NULL);      /* Free it */
		    for (k = j; k < last; k++)  /* Move the rest up */
		      p[k] = p[k+1];
		    p[k] = NULL;	        /* Erase last one */
		    if (!p[k])
		      break;
		}
	    }
	}
    }
  xdoadd:				/* Common exit */
    for (i = 0; i < n; i++)
      if (tmp[i])
	free(tmp[i]);
    return(x);
#endif /* PATTERNS */
}

/* ADD SEND-LIST */

static int
addsend(cx) int cx; {
#ifndef NOMSEND
    extern struct keytab fttab[];
    extern int nfttyp;
    struct filelist * flp;
    char * fmode = "";
    int xmode = 0;
    int xbinary = 0;
#endif /* NOMSEND */

#ifdef NOMSEND
    printf("?Sorry, ADD/REMOVE SEND-LIST not available.\n");
    return(-9);
#endif /* NOMSEND */
    if (cx == XXREMV) {
	printf("?Sorry, REMOVE SEND-LIST not implemented yet.\n");
	return(-9);
    }
#ifndef NOMSEND
#ifndef XYZ_INTERNAL
    if (protocol != PROTO_K) {
       printf("?Sorry, ADD SEND-LIST does not work with external protocols\n");
       return(-9);
    }
#endif /* XYZ_INTERNAL */

    x = cmifi("File specification to add","", &s,&y,xxstring);
    if (x < 0) {
	if (x == -3) {
	    printf("?A file specification is required\n");
	    return(-9);
	} else
	  return(x);
    }
    strcpy(tmpbuf,s);
    s = tmpbuf;
    if (filesinlist == 0)		/* Take care of \v(filespec) */
      fspec[0] = NUL;
    zfnqfp(s,LINBUFSIZ,line);
    s = line;
    if (((int)strlen(fspec) + (int)strlen(s) + 1) < fspeclen) {
	strcat(fspec,s);
	strcat(fspec," ");
    } else
      printf("WARNING - \\v(filespec) buffer overflow\n");


    xbinary = binary;
#ifdef PATTERNS
    if (patterns			/* PATTERNS are ON */
#ifdef CK_LABELED
	&& binary != XYFT_L		/* And not if FILE TYPE LABELED */
#endif /* CK_LABELED */
#ifdef VMS
	&& binary != XYFT_I		/* or FILE TYPE IMAGE */
#endif /* VMS */
	) {
	if (binary != XYFT_T && txtpatterns[0]) {
	    int i;
	    for (i = 0; i < FTPATTERNS && txtpatterns[i]; i++) {
		if (ckmatch(txtpatterns[i],line,filecase,1)) {
		    xbinary = XYFT_T;
		    break;
		}
	    }
	}
	if (binary != XYFT_B && binpatterns[0]) {
	    int i;
	    for (i = 0; i < FTPATTERNS && binpatterns[i]; i++) {
		if (ckmatch(binpatterns[i],line,filecase,1)) {
		    xbinary = XYFT_B;
		    break;
		}
	    }
	}
    }
#endif /* PATTERNS */

    fmode = gfmode(xbinary,0);
    if ((x = cmkey(fttab,nfttyp,
		   "type of file transfer", fmode, xxstring)) < 0)
      return(x);
    xmode = x;

    cmarg2 = "";
    if ((x = cmfld(y ?
  "\nAs-name template containing replacement variables such as \\v(filename)" :
  "Name to send it with", "",&s,NULL)) < 0)
      if (x != -3)
	return(x);
#ifndef NOSPL
    if (y && *s) {
	char * p = tmpbuf;
	x = TMPBUFSIZ;
	zzstring(s,&p,&x);
	if (!strcmp(tmpbuf,s)) {
	    printf(
  "?As-name for file group must contain variables such as \\v(filename)\n"
		   );
	    return(-9);
	}
    }
#endif /* NOSPL */
    strcpy(tmpbuf,s);
    cmarg2 = tmpbuf;

    if ((x = cmcfm()) < 0)
      return(x);
    flp = (struct filelist *) malloc(sizeof(struct filelist));
    if (flp) {
	if (filetail)
	  filetail->fl_next = flp;
	filetail = flp;
	if (!filehead)
	  filehead = flp;
	x = (int) strlen(line);	/* Length of filename */
	s = (char *) malloc(x + 1);
	if (s) {
	    strcpy(s,line);
	    flp->fl_name = s;
	    flp->fl_mode = xmode;
	    x = (int) strlen(cmarg2);	/* Length of as-name */
	    if (x < 1) {
		flp->fl_alias = NULL;
	    } else {
		s = (char *) malloc(x + 1);
		if (s) {
		    strcpy(s,cmarg2);
		    flp->fl_alias = s;
		} else {
		    printf("Sorry, can't allocate space for as-name");
		    return(-9);
		}
	    }
	    flp->fl_next = NULL;
	    filesinlist++;		/* Count this node */
	    return(success = 1);	/* Finished adding this node */
	} else {
	    printf("Sorry, can't allocate space for name");
	    return(-9);
	}
    } else {
	printf("Sorry, can't allocate file list node");
	return(-9);
    }
#endif /* NOMSEND */
}
#endif /* ADDCMD */

#ifndef NOHTTP				/* HTTP ops... */
#ifdef TCPSOCKET
#define HTTP_GET 0			/* GET */
#define HTTP_PUT 1			/* PUT */
#define HTTP_POS 2			/* POST */
#define HTTP_IDX 3			/* INDEX */
#define HTTP_HED 4                      /* HEAD */
#define HTTP_DEL 5                      /* DELETE */

static struct keytab httptab[] = {
    "delete", HTTP_DEL, 0,
    "get",    HTTP_GET, 0,
    "head",   HTTP_HED, 0,
    "index",  HTTP_IDX, 0,
    "put",    HTTP_PUT, 0,
    "post",   HTTP_POS, 0
};
static int nhttptab = sizeof(httptab)/sizeof(struct keytab);

/* HTTP switches */
#define HT_SW_AG 0			/* /AGENT */
#define HT_SW_HD 1			/* /HEADER */
#define HT_SW_US 2			/* /USER */
#define HT_SW_PW 3			/* /PASSWORD */
#define HT_SW_AR 4

static struct keytab httpswtab[] = {
    "/agent",    HT_SW_AG, CM_ARG,
#ifndef NOSPL
    "/array",    HT_SW_AR, CM_ARG,
#endif /* NOSPL */
    "/header",   HT_SW_HD, CM_ARG,
    "/password", HT_SW_PW, CM_ARG,
    "/user",     HT_SW_US, CM_ARG,
    "", 0, 0
};
static int nhttpswtab = sizeof(httpswtab)/sizeof(struct keytab) - 1;

/* HTTP PUT/POST switches */
#define HT_PP_MT 0			/* /MIME-TYPE */

static struct keytab httpptab[] = {
    "/mime-type", HT_PP_MT, CM_ARG,
    "", 0, 0
};
static int nhttpptab = sizeof(httpptab)/sizeof(struct keytab) - 1;

#define HTTP_MAXHDR 8

static int
dohttp(action, lfile, rf, agent, hdr, user, pass, mime, array)
    int action; char *lfile, *rf, *agent, *hdr, *user, *pass, *mime, array;
/* dohttp */ {
    int i, rc = 0;
    char * hdrlist[HTTP_MAXHDR];
    char rfile[CKMAXPATH+1];

    /* Check for a valid state to execute the command */
    if (inserver) {
        printf("?The HTTP command may not be used from the IKS\r\n");
    } else if (!local || ttchk() < 0) {
	printf("?No connection\n");
    } else if (!network) {
	printf("?The HTTP command is not for serial connections\n");
    } else if (nettype != NET_TCPB && nettype != NET_TCPA) {
	printf("?The HTTP command only for TCP/IP connections\n");
    } else {
	rc = 1;
    }

    /* If the command is not valid, exit with failure */
    if (rc == 0)
        return(success = 0);

    if (rf[0] != '/') {
        rfile[0] = '/';
        ckstrncpy(&rfile[1],rf,CKMAXPATH);
    } else {
        ckstrncpy(rfile,rf,CKMAXPATH);
    }
    for (i = 0; i < HTTP_MAXHDR; i++)	/* Initialize header list */
      hdrlist[i] = NULL;
    makelist(hdr,hdrlist,HTTP_MAXHDR);	/* Make header list */

#ifdef BETATEST
    for (i = 0; i < nhttptab; i++)	/* Find action keyword */
      if (httptab[i].kwval == action)
	break;
    if (i == nhttptab) {		/* Shouldn't happen... */
	printf("?Invalid action - %d\n",action);
	return(success = 0);
    }

    printf("HTTP action:  %s\n",httptab[i].kwd);
    printf(" Agent:       %s\n",agent ? agent : "(null)");

    if (hdrlist[1]) {
	printf(" Header list: 1. %s\n",hdrlist[0]);
	for (i = 1; i < HTTP_MAXHDR && hdrlist[i]; i++)
	  printf("%15d. %s\n",i+1,hdrlist[i]);
    } else
      printf(" Header:      %s\n",hdrlist[0] ? hdrlist[0] : "(null)");

    printf(" User:        %s\n",user ? user : "(null)");
    printf(" Password:    %s\n",pass ? pass : "(null)");

#ifndef NOSPL
    if (array)
      printf(" Array:       \\%%%c[]\n", array);
    else
      printf(" Array:       (none)\n");
#endif /* NOSPL */

    if (action == HTTP_PUT || action == HTTP_POS)
      printf(" Mime-type:   %s\n",mime ? mime : "(null)");

    printf(" Local file:  %s\n",lfile ? lfile : "(null)");
    printf(" Remote file: %s\n",rfile ? rfile : "(null)");
#endif /* BETATEST */
    switch (action) {
      case HTTP_DEL:
        rc = http_delete(agent,hdrlist,user,pass,array,rfile);
        break;
      case HTTP_GET:
        rc = http_get(agent,hdrlist,user,pass,array,lfile,rfile);
        break;
      case HTTP_HED:
        rc = http_head(agent,hdrlist,user,pass,array,lfile,rfile);
        break;
      case HTTP_PUT:
        rc = http_put(agent,hdrlist,mime,user,pass,array,lfile,rfile);
        break;
      case HTTP_POS:
        rc = http_post(agent,hdrlist,mime,user,pass,array,lfile,rfile);
        break;
      case HTTP_IDX:
        rc = http_index(agent,hdrlist,user,pass,array,lfile,rfile);
        break;
    }
    return(success = ((rc < 0) ? 0 : 1));
}
#endif /* TCPSOCKET */
#endif /* NOHTTP */

#ifndef NOSPL				/* ARRAY ops... */
static struct keytab arraytab[] = {
    "clear",    ARR_CLR, 0,
    "copy",     ARR_CPY, 0,
    "dcl",      ARR_DCL, CM_INV,
    "declare",  ARR_DCL, 0,
    "destroy",  ARR_DST, 0,
    "resize",   ARR_RSZ, 0,
    "set",      ARR_SET, 0,
#ifndef NOSHOW
    "show",     ARR_SHO, 0,
#endif /* NOSHOW */
    "sort",     ARR_SRT, 0,
    "", 0, 0
};
static int narraytab = sizeof(arraytab)/sizeof(struct keytab) - 1;

int
arrayitoa(x) int x; {			/* Array index to array letter */
    if (x == 1)
      return(64);
    else if (x < 0 || x > (122 - ARRAYBASE))
      return(-1);
    else
      return(x + ARRAYBASE);
}

int
arrayatoi(c) int c; {			/* Array letter to array index */
    if (c == 64)
      c = 96;
    if (c > 63 && c < 91)
      c += 32;
    if (c < ARRAYBASE || c > 122)
      return(-1);
    return(c - ARRAYBASE);
}

static int
dodcl() {
    int i, n, v, lo, hi, rc = 0;
    int isdynamic = 0;
    char ** p = NULL;
    char tmp[64];			/* Local temporary string buffer */
    if ((y = cmfld("Array name","",&s,NULL)) < 0) { /* Parse array name */
	if (y == -3) {
	    printf("?Array name required\n");
	    return(-9);
	} else return(y);
    }
#ifdef COMMENT
    if ((y = arraynam(s,&x,&n)) < 0) {	/* Check it */
	if (y == -2 && n == -17) {	/* Secret code for emtpy brackets */
	    isdynamic = 1;
	    n = CMDBL / 5;
	} else {
	    return(y);
	}
    }
    debug(F111,"DCL",s,n);
#else
    ckstrncpy(line,s,LINBUFSIZ);
    s = line;
    x = arraybounds(s,&lo,&hi);
    if (lo < 0 && hi < 0) {
	isdynamic = 1;
	n = CMDBL / 5;
    } else if (hi > -1) {
	printf("?Segment notation not allowed in array declarations\n");
	return(-9);
    } else
      n = lo;
    x = arrayitoa(x);
#endif /* COMMENT */
    p = (char **)malloc(sizeof(char **)*(n+1));
    if (!p) {
	printf("?Memory allocation error\n");
	return(-9);
    }
    v = 0;				/* Highest initialized member */
    p[0] = NULL;			/* Element 0 */
    while (n > 0 && v < n) {		/* Parse initializers */
	p[v+1] = NULL;
	sprintf(tmp,"Initial value for \\&%c[%d]",x,v+1); /* Help string */
	if ((rc = cmfld((char *)tmp,"",&s,xxstring)) < 0) { /* Get field */
	    if (rc == -3)		/* If answer is empty, we're done */
	      break;
	    else			/* Parse error, free temp pointers */
	      goto dclx;
	}
	rc = 1;
	if (v == 0 && !strcmp(s,"="))	/* Skip the = sign. */
	  continue;
	s = brstrip(s);			/* Strip any braces */
	makestr(&(p[++v]),s);
    }
    if ((y = cmtxt("Carriage return to confirm","",&s,NULL)) < 0)
      return(y);
    if (isdynamic)
      n = v;
    if (dclarray((char)x,n) < 0) {	/* Declare the array */
	printf("?Declare failed\n");
	goto dclx;
    }
    for (i = 1; i <= v; i++) {		/* Add any initial values */
	sprintf(tmp,"&%c[%d]",(char)x,i);
	if (addmac(tmp,p[i]) < 0) {
	    printf("Array initialization error: %s %s\n",tmp,p[i]);
	    rc = -9;
	    goto dclx;
	}
    }
  dclx:
    if (p) {
	for (i = 1; i <= v; i++)
	  if (p[i]) free(p[i]);
	free(p);
    }
    debug(F101,"DCL rc","",rc);
    return(success = rc);
}

static int
rszarray() {
    int i, x, y, n, lo, hi;
    char c, * s, ** ap = NULL;
    if ((x = cmfld("Array name","",&s,NULL)) < 0) { /* Parse array name */
	if (x == -3) {
	    printf("?Array name required\n");
	    return(-9);
	} else return(x);
    }
    ckstrncpy(line,s,LINBUFSIZ);	/* Make safe copy of name */
    s = line;
    x = arraybounds(s,&lo,&hi);
    if (lo < 0 && hi < 0) {
	y = cmnum("New size","",10,&lo,xxstring);
	if (y < 0) {
	    if (y == -3)
	      printf("?New size required\n");
	    return(y);
	}
    }
    if ((y = cmcfm()) < 0)
      return(y);
    if (x < 0) {			/* Parse the name, get index */
	printf("?Bad array reference - \"%s\"\n", s);
	return(-9);
    }
    if (!a_ptr[x]) {
	printf("?Array not declared - \"%s\"\n", s);
	return(-9);
    }
    if (lo < 0) {
	printf("?New size required\n");
	return(-9);
    }
    if (hi > -1) {
	printf("?Array segments not allowed for this operation\n");
	return(-9);
    }
    c = arrayitoa(x);			/* Get array letter */
    if (c == '@') {			/* Argument vector array off limits */
	printf("?Sorry, \\&@[] is read-only\n");
	return(-9);
    }
    if (lo == 0) {			/* If new size is 0... */
	dclarray(c,0);			/* Undeclare the array */
	return(success = 1);
    }
    n = a_dim[x];			/* Current size */
    ap = (char **) malloc((lo+1) * sizeof(char *)); /* New array */
    y = (n < lo) ? n : lo;
    for (i = 0; i <= y; i++)		/* Copy the part that fits */
      ap[i] = a_ptr[x][i];
    if (n < lo) {			/* If original array smaller */
	for (; i <= lo; i++)		/* initialize extra elements in */
	  ap[i] = NULL;			/* new array to NULL. */
    } else if (n > lo) {		/* If new array smaller */
	for (; i <= lo; i++)		/* deallocate leftover elements */
	  makestr(&(a_ptr[x][i]),NULL);	/* from original array. */
    }
    free(a_ptr[x]);			/* Free original array list */
    a_ptr[x] = ap;			/* Replace with new one */
    a_dim[x] = lo;			/* Record the new dimension */
    return(success = 1);
}

static int
copyarray() {
    int i, j, x1, lo1, hi1, x2, lo2, hi2, whole = 0;
    char c1, c2, * a1, * a2;
    if ((y = cmfld("Name of source array","",&s,NULL)) < 0)
      return(y);
    ckstrncpy(line,s,LINBUFSIZ);
    a1 = line;
    if ((x1 = arraybounds(a1,&lo1,&hi1)) < 0) {
	printf("?Bad array reference - \"%s\"\n", a1);
	return(-9);
    } else if (!a_ptr[x1]) {
	printf("?Array not declared - \"%s\"\n", a1);
	return(-9);
    }
    c1 = arrayitoa(x1);

    if ((y = cmfld("Name of destination array","",&s,NULL)) < 0)
      return(y);
    ckstrncpy(tmpbuf,s,TMPBUFSIZ);
    a2 = tmpbuf;
    if ((x2 = arraybounds(a2,&lo2,&hi2)) < 0) {
	printf("?Bad array reference - \"%s\"\n", a2);
	return(-9);
    }
    c2 = arrayitoa(x2);

    if ((x = cmcfm()) < 0)
      return(x);

    if (c2 == '@') {			/* Argument vector array off limits */
	printf("?Sorry, \\&@[] is read-only\n");
	return(-9);
    }
    if (lo1 < 0 && lo2 < 0 && hi1 < 0 && hi2 < 0) /* Special case for */
      whole = 1;			          /* whole array... */

    if (lo1 < 0) lo1 = whole ? 0 : 1;	/* Supply lower bound of source */
    if (hi1 < 0) hi1 = a_dim[x1];	/* Supply upper bound of source */
    if (lo2 < 0) lo2 = whole ? 0 : 1;	/* Lower bound of target */
    if (hi2 < 0) hi2 = lo2 + hi1 - lo1;	/* Upper bound of target */
    if (a_ptr[x2]) {			/* Target array is already declared? */
	if (hi2 > a_dim[x2])		/* If upper bound out of range */
	  hi2 = a_dim[x2];		/* shrink to fit */
    } else {				/* Otherwise... */
	x2 = dclarray(c2, hi2);		/* declare the target array */
    }
    for (i = lo1, j = lo2; i <= hi1 && j <= hi2; i++,j++) { /* Copy */
	makestr(&(a_ptr[x2][j]),a_ptr[x1][i]);
    }
    return(success = 1);
}

static int				/* Undeclare an array */
unarray() {
    int x, y, n, rc = 0;
    char c, * s;

    if ((y = cmfld("Array name","",&s,NULL)) < 0) { /* Parse array name */
	if (y == -3) {
	    printf("?Array name required\n");
	    return(-9);
	} else return(y);
    }
    ckstrncpy(line,s,LINBUFSIZ);	/* Make safe copy of name */
    s = line;
    if ((y = cmcfm()) < 0)
      return(y);
    if ((x = arraybounds(s,&y,&n)) < 0) { /* Parse the name, get index */
	printf("?Bad array reference - \"%s\"\n", s);
	return(-9);
    }
    if (y > 0 || n > 0) {
	printf("?Partial arrays can not be destroyed\n");
	return(-9);
    }
    c = arrayitoa(x);			/* Get array letter */
    if (a_ptr[x]) {			/* If array is declared */
	if (c == '@') {			/* Argument vector array off limits */
	    printf("?Sorry, \\&@[] is read-only\n");
	    return(-9);
	}
	rc = dclarray(c,0);		/* Undeclare the array */
    } else				/* It wasn't declared */
      rc = 1;
    if (rc > -1) {			/* Set return code and success */
	success = 1;
	rc = 1;
    } else {
	success = 0;
	printf("?Failed - destroy \"\\&%c[]\"\n", c);
	rc = -9;
    }
    return(rc);
}

static int
clrarray(cx) int cx; {
    int i, x, lo, hi;
    char c, * s, * val = NULL;

    if ((x = cmfld("Array name","",&s,NULL)) < 0) { /* Parse array name */
	if (x == -3) {
	    printf("?Array name required\n");
	    return(-9);
	} else return(x);
    }
    ckstrncpy(line,s,LINBUFSIZ);	/* Make safe copy of name */
    s = line;
    if (cx == ARR_SET) {		/* SET */
	if ((x = cmtxt("Value","",&val,xxstring)) < 0)
	  return(x);
	ckstrncpy(tmpbuf,val,TMPBUFSIZ); /* Value to set */
	val = tmpbuf;
	if (!*val) val = NULL;
    } else if ((x = cmcfm()) < 0)	/* CLEAR */
      return(x);

    if ((x = arraybounds(s,&lo,&hi)) < 0) { /* Parse the name */
	printf("?Bad array reference - \"%s\"\n", s);
	return(-9);
    }
    c = arrayitoa(x);			/* Get array letter */
    if (!a_ptr[x]) {			/* If array is declared */
	printf("?Array %s is not declared\n", s);
	return(-9);
    } else if (c == '@') {		/* Argument vector array off limits */
	printf("?Sorry, \\&@[] is read-only\n");
	return(-9);
    }
    if (lo < 0) lo = 0;
    if (hi < 0) hi = a_dim[x];
    for (i = lo; i <= hi; i++)		/* Clear/Set selected range */
      makestr(&(a_ptr[x][i]),val);

    return(success = 1);
}
#endif /* NOSPL */

/*  D O C M D  --  Do a command  */

/*
 Returns:
   -2: user typed an illegal command
   -1: reparse needed
    0: parse was successful (even tho command may have failed).
*/
int
docmd(cx) int cx; {

    debug(F101,"docmd entry, cx","",cx);
    activecmd = cx;
    doconx = ((activecmd == XXCON)  || (activecmd == XXTEL) ||
	      (activecmd == XXRLOG) || (activecmd == XXPIPE) ||
              (activecmd == XXIKSD) || (activecmd == XXPTY));
/*
  Massive switch() broken up into many smaller ones, for the benefit of
  compilers that run out of space when trying to handle large switch
  statements.
*/
    switch (cx) {
      case -4:			/* EOF */
#ifdef OSK
	if (msgflg)  printf("\n");
#else
	if (msgflg)  printf("\r\n");
#endif /* OSK */
	  doexit(GOOD_EXIT,xitsta);
      case -3:				/* Null command */
	return(0);
      case -9:				/* Like -2, but errmsg already done */
      case -1:				/* Reparse needed */
	return(cx);
      case -6:				/* Special */
      case -2:				/* Error, maybe */
#ifndef NOSPL
/*
  Maybe they typed a macro name.  Let's look it up and see.
*/
	if (cx == -6)			/* If they typed CR */
	  strcat(cmdbuf,"\015");	/*  add it back to command buffer. */
	if (ifcmd[cmdlvl] == 2)		/* Watch out for IF commands. */
	  ifcmd[cmdlvl]--;
	repars = 1;			/* Force reparse */
	cmres();
	cx = XXDO;			/* Try DO command */
#else
	return(cx);
#endif /* NOSPL */
      default:
	break;
    }

#ifndef NOSPL
/* Copy macro args from/to two levels up, used internally by _floop et al. */
    if (cx == XXGTA || cx == XXPTA) {	/* _GETARGS, _PUTARGS */
	int x;
	debug(F101,"docmd XXGTA","",XXGTA);
	debug(F101,"docmd cx","",cx);
	debug(F101,"docmd XXGTA maclvl","",maclvl);
	x = dogta(cx);
	debug(F101,"docmd dogta returns","",x);
	debug(F101,"docmd dogta maclvl","",maclvl);
	return(x);
    }
#endif /* NOSPL */

#ifndef NOSPL
#ifdef CKCHANNELIO
    if (cx == XXFILE)
      return(dofile(cx));
    else if (cx == XXF_RE || cx == XXF_WR || cx == XXF_OP ||
	     cx == XXF_CL || cx == XXF_SE || cx == XXF_RW ||
	     cx == XXF_FL || cx == XXF_LI || cx == XXF_ST || cx == XXF_CO)
      return(dofile(cx));
#endif /* CKCHANNELIO */

/* ASK, ASKQ, READ */
    if (cx == XXASK  || cx == XXASKQ || cx == XXREA ||
	cx == XXRDBL || cx == XXGETC || cx == XXGETK) {
	return(doask(cx));
    }
#endif /* NOSPL */

#ifndef NOFRILLS
    if (cx == XXBUG) {			/* BUG */
	if ((x = cmcfm()) < 0) return(x);
	return(dobug());
    }
#endif /* NOFRILLS */

#ifndef NOXFER
    if (cx == XXBYE) {			/* BYE */
	bye_active = 1;
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, BYE only works with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
	if ((x = cmcfm()) < 0) return(x);

#ifdef IKS_OPTION
        if (!iks_wait(KERMIT_REQ_START,1)) {
	    printf(
	     "?A Kermit Server is unavailable to process this command\n");
	    return(-9);			/* Correct the return code */
        }
#endif /* IKS_OPTION */

	sstate = setgen('L',"","","");
	if (local) ttflui();		/* If local, flush tty input buffer */
	return(0);
    }
#endif /* NOXFER */

    if (cx == XXBEEP) {			/* BEEP */
        int x;
#ifdef OS2
	int y;
        if ((y = cmkey(beeptab, nbeeptab, "which kind of beep", "information",
		       xxstring)) < 0 )
	  return (y);
    	if ((x = cmcfm()) < 0) return(x);
        bleep((short)y);		/* y is one of the BP_ values */
#else  /* OS2 */
    	if ((x = cmcfm()) < 0) return(x);
#ifndef NOSPL
        bleep(BP_NOTE);
#else
	putchar('\07');
#endif /* NOSPL */
#endif /* OS2 */
        return(0);
    }

#ifndef NOFRILLS
    if (cx == XXCLE) {			/* CLEAR */
	if ((x = cmkey(clrtab,nclear,"item to clear",
#ifdef NOSPL
		  "device-buffer"
#else
		  "device-and-input"
#endif /* NOSPL */
		  ,xxstring)) < 0) return(x);
#ifndef NOSPL
#ifdef OS2
        if (x == CLR_CMD || x == CLR_TRM) {
            if ((z = cmkey(clrcmdtab,nclrcmd,"how much screen to clear\n",
			   "all",xxstring)) < 0)
	      return(z);
        }
#endif /* OS2 */
#endif /* NOSPL */
	if ((y = cmcfm()) < 0) return(y);

	/* Clear device input buffer if requested */
	y = (x & CLR_DEV) ? ttflui() : 0;

	if (x & CLR_SCR)		/* CLEAR SCREEN */
	  y = ck_cls();			/* (= SCREEN CLEAR = CLS) */

#ifndef NOSPL
	/* Clear INPUT command buffer if requested */
	if (x & CLR_INP) {
	    for (z = 0; z < inbufsize; z++)
	      inpbuf[z] = NUL;
	    inpbp = inpbuf;
	    y = 0;
	}
#ifdef CK_APC
	if (x & CLR_APC) {
	    debug(F101,"Executing CLEAR APC","",apcactive);
	    apcactive = 0;
	    y = 0;
	}
#endif /* CK_APC */
	if (x & CLR_ALR) {
	    setalarm(0L);
	    y = 0;
	}
#endif /* NOSPL */

#ifdef PATTERNS
	if (x & CLR_TXT|CLR_BIN) {
	    int i;
	    for (i = 0; i < FTPATTERNS; i++) {
		if (x & CLR_TXT)
		  makestr(&txtpatterns[i],NULL);
		if (x & CLR_BIN)
		  makestr(&binpatterns[i],NULL);
	    }
	    y = 0;
	}
#endif /* PATTERNS */

#ifndef NODIAL
	if (x & CLR_DIA) {
	    dialsta = DIA_UNK;
	    y = 0;
	}
#endif /* NODIAL */

#ifndef NOMSEND
	if (x & CLR_SFL) {		/* CLEAR SEND-LIST */
	    if (filehead) {
		struct filelist * flp, * next;
		flp = filehead;
		while (flp) {
		    if (flp->fl_name)
		      free(flp->fl_name);
		    if (flp->fl_alias)
		      free(flp->fl_alias);
		    next = flp->fl_next;
		    free(flp);
		    flp = next;
		}
	    }
	    filesinlist = 0;
	    filehead = NULL;
	    filetail = NULL;
	    y = 0;
	}
#endif /* NOMSEND */

#ifdef OS2
	switch (x) {
	  case CLR_SCL:
	    clearscrollback(VTERM);
	    break;
	  case CLR_CMD:
	    switch ( z ) {
	      case CLR_C_ALL:
		clear();
		break;
	      case CLR_C_BOS:
		clrboscr_escape(VCMD,SP);
		break;
	      case CLR_C_BOL:
		clrbol_escape(VCMD,SP);
		break;
	      case CLR_C_EOL:
		clrtoeoln(VCMD,SP);
		break;
	      case CLR_C_EOS:
		clreoscr_escape(VCMD,SP);
		break;
	      case CLR_C_LIN:
		clrline_escape(VCMD,SP);
		break;
	      case CLR_C_SCR:
		clearscrollback(VCMD);
		break;
            default:
		printf("Not implemented yet, sorry.\n");
		break;
	    }
	    break;

	  case CLR_TRM:
             switch ( z ) {
	      case CLR_C_ALL:
                 if (VscrnGetBufferSize(VTERM) > 0 ) {
                     VscrnScroll(VTERM, UPWARD, 0,
				 VscrnGetHeight(VTERM)-(tt_status?2:1),
				 VscrnGetHeight(VTERM) -
				 (tt_status?1:0), TRUE, SP
				 );
                     cleartermscreen(VTERM);
                 }
                 break;
	      case CLR_C_BOS:
		clrboscr_escape(VTERM,SP);
		break;
	      case CLR_C_BOL:
		clrbol_escape(VTERM,SP);
		break;
	      case CLR_C_EOL:
		clrtoeoln(VTERM,SP);
		break;
	      case CLR_C_EOS:
		clreoscr_escape(VTERM,SP);
		break;
	      case CLR_C_LIN:
		clrline_escape(VTERM,SP);
		break;
             case CLR_C_SCR:
                 clearscrollback(VTERM);
                 break;
             default:
                 printf("Not implemented yet, sorry.\n");
                 break;
	    }
	    break;
	}
	y = 0;
#endif /* OS2 */
	return(success = (y == 0));
    }
#endif /* NOFRILLS */

    if (cx == XXCOM) {			/* COMMENT */
	if ((x = cmtxt("Text of comment line","",&s,NULL)) < 0)
	  return(x);
	/* Don't change SUCCESS flag for this one */
	return(0);
    }

#ifndef NOLOCAL
    if (cx == XXCON || cx == XXCQ)	/* CONNECT or CONNECT /QUIETLY */
      return(doxconn(cx));
#endif /* NOLOCAL */

#ifndef NOFRILLS
#ifdef ZCOPY
    if (cx == XXCPY) {			/* COPY a file */
#ifdef IKSD
	if (inserver && !ENABLED(en_cpy)) {
	    printf("?Sorry, COPY is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
	    (apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	    )
	  return(success = 0);
#endif /* CK_APC */
	return(docopy());
    }
#endif /* ZCOPY */
#endif /* NOFRILLS */

    if (cx == XXCWD || cx == XXBACK) {	/* CD or BACK */
#ifdef IKSD
	if (inserver && !ENABLED(en_cwd)) {
	    printf("?Sorry, changing directories is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	return(success = docd(cx));
    }

    if (cx == XXCHK)			/* CHECK */
      return(success = dochk());

    if (cx == XXCLO) {			/* CLOSE */
	x = cmkey(clstab,ncls,"\"CONNECTION\", or log or file to close",
		  "connection",xxstring);
	if (x == -3) {
	    printf("?You must say which file or log\n");
	    return(-9);
	}
	if (x < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
#ifndef NOLOCAL
	if (x == 9999) {		/* CLOSE CONNECTION */
	    x = clsconnx(0);
	    switch (x) {
	      case 0:
		if (msgflg) printf("?Connection was not open\n");
	      case -1:
		return(0);
	      case 1:
		whyclosed = WC_CLOS;
		return(1);
	    }
	    return(0);
	}
#endif /* NOLOCAL */
	y = doclslog(x);
	success = (y == 1);
	return(success);
    }

#ifndef NOSPL
    if (cx == XX_INCR || cx == XXINC  || /* _INCREMENT, INCREMENT */
	cx == XX_DECR || cx == XXDEC)	 /* _DECREMENT, DECREMENT */
      return(doincr(cx));

    if (cx == XXEVAL || cx == XX_EVAL) { /* _EVALUATE,  EVALUATE  */
	char *p;
	char vnambuf[VNAML], * vnp = NULL; /* These must be on the stack */
	if (!oldeval) {
	    if ((y = cmfld("Variable name","",&s,
			   ((cx == XX_EVAL) ? xxstring : NULL))) < 0) {
		if (y == -3) {
		    printf("?Variable name required\n");
		    return(-9);
		} else return(y);
	    }
	    ckstrncpy(vnambuf,s,VNAML);	/* Make a copy. */
	    vnp = vnambuf;
	    if (vnambuf[0] == CMDQ &&
		(vnambuf[1] == '%' || vnambuf[1] == '&'))
	      vnp++;
	    y = 0;
	    if (*vnp == '%' || *vnp == '&') {
		if ((y = parsevar(vnp,&x,&z)) < 0)
		  return(y);
	    }
	}
	if ((x = cmtxt("Integer arithmetic expression","",&s,xxstring)) < 0)
	  return(x);
	p = evala(s);
	if (!p) p = "";
	if (oldeval && *p)
	  printf("%s\n", p);
	ckstrncpy(evalbuf,p,32);
	if (!oldeval)
	  return(success = addmac(vnambuf,p));
	else
	  return(success = *p ? 1 : 0);
    }
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDEF || cx == XXASS ||
	cx == XXDFX || cx == XXASX || cx == XXUNDEF) {
#ifdef IKSD

	if (inserver && !ENABLED(en_asg)) {
	    printf("?Sorry, DEFINE/ASSIGN disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	if (atmbuf[0] == '.' && !atmbuf[1]) /* "." entered as keyword */
	  xxdot = 1;			/* i.e. with space after it... */
	return(dodef(cx));		/* DEFINE, ASSIGN, etc... */
    }
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDCL) {			/* DECLARE an array */
	return(dodcl());
    }
#endif /* NOSPL */

#ifndef NODIAL
    if (cx == XXRED  || cx == XXDIAL || cx == XXPDIA ||
	cx == XXANSW || cx == XXLOOK) { /* DIAL, REDIAL etc */
#ifdef VMS
	extern int batch;
#else
#ifdef UNIXOROSK
	extern int backgrd;
#endif /* UNIXOROSK */
#endif /* VMS */
	x = dodial(cx);
	debug(F101,"dodial returns","",x);
	if ((cx == XXDIAL || cx == XXRED || cx == XXANSW) &&
	    (x > 0) &&			/* If DIAL or REDIAL succeeded */
	    (dialsta != DIA_PART) &&	/* and it wasn't partial */
	    (dialcon > 0)) {
	    if ((dialcon == 1 ||	/* And DIAL CONNECT is ON, */
		(dialcon == 2) &&	/* or DIAL CONNECT is AUTO */
		 !cmdsrc()		/* and we're at top level... */
#ifdef VMS
		 && !batch		/* Not if running from batch */
#else
#ifdef UNIXOROSK
		 && !backgrd		/* Not if running in background */
#endif /* UNIXOROSK */
#endif /* VMS */
		 )) /* Or AUTO */
	      x = doconect(dialcq);	/* Then also CONNECT */
#ifdef CKLOGDIAL
	    if (ttchk() < 0)
	      dologend();
#endif /* CKLOGDIAL */
	}
	return(success = x);
    }
#endif /* NODIAL */

#ifndef NOPUSH
#ifdef CK_REXX
    if (cx == XXREXX) {			/* REXX */
        extern int nopush;
        if ( nopush )
          return(success=0);
        return(dorexx());
    }
#endif /* CK_REXX */
#endif /* NOPUSH */

#ifndef NOFRILLS
    if (cx == XXDEL) {			/* DELETE */
#ifdef IKSD
	if (inserver && (!ENABLED(en_del)
#ifdef CK_LOGIN
			 || isguest
#endif /* CK_LOGIN */
			 )) {
	    printf("?Sorry, DELETE is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	return(dodel());
    }
#endif /* NOFRILLS */

    if (cx == XXDIR || cx == XXLS) {	/* DIRECTORY or LS */
#ifdef IKSD
	if (inserver && !ENABLED(en_dir)) {
	    printf("?Sorry, DIRECTORY is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	return(dodir(cx));
    }

#ifndef NOSPL
    if (cx == XXELS)			/* ELSE */
      return(doelse());
#endif /* NOSPL */

#ifndef NOSERVER
#ifndef NOFRILLS
    if (cx == XXENA || cx == XXDIS) {	/* ENABLE, DISABLE */
	s = (cx == XXENA) ?
	  "Server function to enable" :
	    "Server function to disable";

	if ((x = cmkey(enatab,nena,s,"",xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of server function required\n");
		return(-9);
	    } else return(x);
	}
	if ((y = cmkey(kmstab,3,"mode","both",xxstring)) < 0) {
	    if (y == -3) {
		printf("?Please specify remote, local, or both\n");
		return(-9);
	    } else return(y);
	}
	if (cx == XXDIS)		/* Disabling, not enabling */
	  y = 3 - y;
	if ((z = cmcfm()) < 0) return(z);
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
#ifdef IKSD
        /* This may seem like it duplicates the work in doenable()  */
        /* but this code returns failure whereas doenable() returns */
        /* success.                                                 */
        if (inserver &&
            (x == EN_HOS || x == EN_PRI || x == EN_MAI || x == EN_WHO))
            return(success = 0);
#endif /* IKSD */
	return(doenable(y,x));
    }
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
    if (cx == XXRET) {			/* RETURN */
	if ((x = cmtxt("optional return value","",&s,NULL)) < 0)
	  return(x);
	s = brstrip(s);			/* Strip braces */
	if (cmdlvl == 0) {		/* At top level, nothing happens... */
	    return(success = 1);
	} else if (cmdstk[cmdlvl].src == CMD_TF) { /* In TAKE file, like POP */
	    popclvl();			/* pop command level */
	    return(success = 1);	/* always succeeds */
	} else if (cmdstk[cmdlvl].src == CMD_MD) { /* Within macro */
	    return(doreturn(s));	/* Trailing text is return value. */
	} else return(-2);
    }
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDO || cx == XXMACRO) {	/* DO (a macro) */
	char mnamebuf[16];
	struct FDB kw, fl;

	if (cx == XXDO) {
	    if (nmac == 0) {
		printf("\n?No macros defined\n");
		return(-9);
	    }
	    for (y = 0; y < nmac; y++) { /* copy the macro table into a */
		mackey[y].kwd = mactab[y].kwd; /* regular keyword table */
		mackey[y].kwval = y;	/* with value = pointer to macro tbl */
		mackey[y].flgs = mactab[y].flgs;
	    }
	    cmfdbi(&kw,			/* First FDB - command switches */
		   _CMKEY,		/* fcode */
		   "Macro",		/* hlpmsg */
		   "",			/* default */
		   "",			/* addtl string data */
		   nmac,		/* addtl numeric data 1: tbl size */
		   0,			/* addtl numeric data 2: 0 = cmkey */
		   xxstring,		/* Processing function */
		   mackey,		/* Keyword table */
		   &fl			/* Pointer to next FDB */
		   );
	    cmfdbi(&fl,			/* 2nd FDB - for "{" */
		   _CMFLD,		/* fcode */
		   "",			/* hlpmsg */
		   "",
		   "",			/* addtl string data */
		   0,			/* addtl numeric data 1 */
		   0,			/* addtl numeric data 2 */
		   xxstring,
		   NULL,
		   NULL
		   );
	    x = cmfdb(&kw);		/* Parse something */
	    if (x < 0) {		/* Error */
		if (x == -3) {
		    printf("?Macro name required\n");
		    return(-9);
		} else
		  return(x);
	    }
	    if (cmresult.fcode == _CMKEY) {
		if ((y = cmtxt("optional arguments","",&s,xxstring)) < 0)
		  return(y);		/* get args */
		return(dodo(x,s,cmdstk[cmdlvl].ccflgs) < 1 ?
		       (success = 0) : 1);
	    }
	    ckstrncpy(line,cmresult.sresult,LINBUFSIZ);	/* _CMFLD */
	    if (atmbuf[0] == '{') {
		if ((y = cmcfm()) < 0)
		  return(y);
	    }
	} else {			/* XXMACRO */
	    int k = 0;
	    line[k++] = '{';
	    line[k++] = SP;
	    if ((y = cmtxt("braced list of commands","",&s,xxstring)) < 0)
	      return(y);
	    ckstrncpy(line+k,s,LINBUFSIZ);
	}
	x = strlen(line);
	if (line[0] != '{' && line[x-1] != '}') {
            if (x > 0)
	      printf("?Not a command or macro name: \"%s\"\n",line);
            else
	      printf("?Not a command or macro name.\n");
	    return(-9);
	}
	s = brstrip(line);
	sprintf(mnamebuf," ..tmp:%03d",cmdlvl);
	x = addmac(mnamebuf,s);
	return(dodo(x,NULL,cmdstk[cmdlvl].ccflgs) < 1 ?
	       (success = 0) : 1);
    }
#endif /* NOSPL */

    if (cx == XXECH || cx == XXXECH
#ifndef NOSPL
	|| cx == XXAPC
#endif /* NOSPL */
	) {				/* ECHO or APC */
	if ((x = cmtxt((cx == XXECH || cx == XXXECH) ?
		       "Text to be echoed" :
		       "Application Program Command text",
		       "",&s,xxstring)) < 0)
	  return(x);
	s = brstrip(s);			/* Strip braces */
	if (cx == XXECH) {		/* ECHO */
#ifndef NOSPL
	    if (!fndiags || fnsuccess)
#endif /* NOSPL */
	      printf("%s\n",s);

	} else if (cx == XXXECH) {	/* XECHO */
	    printf("%s",s);
#ifdef UNIX
	    fflush(stdout);
#endif /* UNIX */
	} else {			/* APC */
#ifdef CK_APC
	    if (apcactive == APC_LOCAL ||
            apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	      return(success = 0);
#endif /* CK_APC */
	    if (!local) {
		printf("%c_%s%c\\",ESC,s,ESC);
#ifdef UNIX
		fflush(stdout);
#endif /* UNIX */

	    } else {
#ifndef NOSPL
		sprintf(tmpbuf,"%c_%s%c\\",ESC,s,ESC);
		return(success = dooutput(tmpbuf, XXOUT));
#else
		printf("%c_%s%c\\",ESC,s,ESC);
#endif /* NOSPL */
	    }
	}
	return(success = 1);		/* Always succeeds */
    }

#ifndef NOSPL
    if (cx == XXOPE)			/* OPEN */
      return(doopen());
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXOUT || cx == XXLNOUT) {	/* OUTPUT or LINEOUT */
	if ((x = cmtxt("Text to be output","",&s,NULL)) < 0)
	  return(x);
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	debug(F110,"OUTPUT 1",s,0);
	s = brstrip(s);			/* Strip enclosing braces, */
	debug(F110,"OUTPUT 2",s,0);
/*
  I don't think I could ever fully explain this in a million years...
  We have read the user's string without calling the variable-expander
  function.  Now, before we call it, we have to double backslashes that
  appear before \N, \B, \L, and \ itself, so the expander function will
  reduce them back to single backslashes, so when we call dooutput()...
  But it's more complicated than that.
*/
	if (cmdgquo()) {		/* Only if COMMAND QUOTING ON ... */
	    for (x = 0, y = 0; s[x]; x++, y++) {
		if (s[x] == CMDQ) {
		    char c = s[x+1];
		    if (c == 'n' || c == 'N' ||
			c == 'b' || c == 'B' ||
			c == 'l' || c == 'L' ||
			c == CMDQ)
		      line[y++] = CMDQ;
		}
		line[y] = s[x];
	    }
	    line[y++] = '\0';		/* Now expand variables, etc. */
	    debug(F110,"OUTPUT 3",line,0);
	    s = line+y+1;
	    x = LINBUFSIZ - (int) strlen(line) - 1;
	    debug(F101,"OUTPUT size","",x);
	    if (zzstring(line,&s,&x) < 0)
	      return(success = 0);
	    s = line+y+1;
	    debug(F110,"OUTPUT 4",s,0);
	}
	success = dooutput(s,cx);
	return(success);
    }
#endif /* NOSPL */

#ifdef ANYX25
#ifndef IBMX25
    if (cx == XXPAD) {			/* PAD commands */
	x = cmkey(padtab,npadc,"PAD command","",xxstring);
	if (x == -3) {
	    printf("?You must specify a PAD command to execute\n");
	    return(-9);
	}
	if (x < 0) return(x);

	switch (x) {
	  case XYPADL:
	    if (x25stat() < 0)
	      printf("Sorry, you must 'set network' & 'set host' first\r\n");
	    else {
		x25clear();
		initpad();
	    }
	    break;
	  case XYPADS:
	    if (x25stat() < 0)
	      printf("Not connected\r\n");
	    else {
		extern int linkid, lcn;
		conol("Connected thru ");
		conol(ttname);
		printf(", Link id %d, Logical channel number %d\r\n",
		       linkid,lcn);
	    }
	    break;
	  case XYPADR:
	    if (x25stat() < 0)
	      printf("Sorry, you must 'set network' & 'set host' first\r\n");
	    else
	      x25reset(0,0);
	    break;
	  case XYPADI:
	    if (x25stat() < 0)
	      printf("Sorry, you must 'set network' & 'set host' first\r\n");
	    else
	      x25intr(0);
	}
	return(0);
    }
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOSPL
    if (cx == XXPAU || cx == XXWAI || cx == XXMSL) /* PAUSE, WAIT, etc */
      return(dopaus(cx));
#endif /* NOSPL */

#ifndef NOFRILLS
    if (cx == XXPRI) {
#ifdef IKSD
#ifdef CK_LOGIN
	if (inserver && (isguest || !ENABLED(en_pri))) {
	    printf("?Sorry, printing is disabled\n");
	    return(-9);
	}
#endif /* CK_LOGIN */
#endif /* IKSD */
	if ((x = cmifi("File to print","",&s,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?A file specification is required\n");
		return(-9);
	    } else return(x);
	}
	if (y != 0) {
	    printf("?Wildcards not allowed\n");
	    return(-9);
	}
	ckstrncpy(line,s,LINBUFSIZ);
	s = "";
#ifndef NT
	if ((x = cmtxt("Local print command options, or carriage return","",&s,
		       xxstring)) < 0)
	  return(x);
#endif /* NT */
	if ((x = cmcfm()) < 0)
	  return(x);
	return(success = (zprint(s,line) == 0) ? 1 : 0);
    }
#endif /* NOFRILLS */

#ifndef NOPUSH
#ifdef TCPSOCKET
    if (cx == XXPNG) 			/* PING an IP host */
      return(doping());

    if (cx == XXFTP)			/* FTP an IP host */
      return(doftp());
#endif /* TCPSOCKET */
#endif /* NOPUSH */

    if (cx == XXPWD) {			/* PWD */
	char *pwp;
	if ((x = cmcfm()) < 0)
	  return(x);
#ifndef MAC
#ifndef OS2
#ifdef UNIX
	printf("%s\n",zgtdir());
#else
	xsystem(PWDCMD);
#endif /* UNIX */
	return(success = 1);		/* Blind faith */
#else  /* OS2 */
	if (pwp = zgtdir()) {
	    printf("%s\n",pwp);
	    return(success = ((int)strlen(pwp) > 0));
	} else return(success = 0);
#endif /* OS2 */
#else  /* MAC */
	if (pwp = zgtdir()) {
	    printf("%s\n",pwp);
	    return(success = ((int)strlen(pwp) > 0));
	} else return(success = 0);
#endif /* MAC */
    }

    if (cx == XXQUI || cx == XXEXI) {	/* EXIT, QUIT */
	if ((y = cmnum("exit status code","",10,&x,xxstring)) < 0) {
	    if (y == -3)
	      x = xitsta;
	    else return(y);
	}
	if ((y = cmtxt("Optional EXIT message","",&s,xxstring)) < 0)
	  return(y);
	s = brstrip(s);
	ckstrncpy(line,s,LINBUFSIZ);
	if ((y = cmcfm()) < 0) return(y);

	if (!hupok(0))			/* Check if connection still open */
	  return(success = 0);

	if (line[0])			/* Print EXIT message if given */
	  printf("%s\n",(char *)line);

#ifdef VMS
	doexit(GOOD_EXIT,x);
#else
#ifdef OSK
/* Returning any codes here makes the OS-9 shell print an error message. */
	doexit(GOOD_EXIT,-1);
#else
#ifdef datageneral
        doexit(GOOD_EXIT,x);
#else
	doexit(x,-1);
#endif /* datageneral */
#endif /* OSK */
#endif /* VMS */
    }

#ifndef NOXFER
#ifndef NOFRILLS
    if (cx == XXERR) {			/* ERROR */
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, E-PACKET only works with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
	if ((x = cmcfm()) < 0) return(x);
	ttflui();
	epktflg = 1;
	sstate = 'a';
	return(0);
    }
#endif /* NOFRILLS */

    if (cx == XXFIN) {			/* FINISH */
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, FINISH only works with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
	if ((x = cmcfm()) < 0) return(x);

#ifdef IKS_OPTION
        if (!iks_wait(KERMIT_REQ_START,1)) {
	    printf(
              "?A Kermit Server is unavailable to process this command\n");
	    return(-9);			/* Correct the return code */
        }
#endif /* IKS_OPTION */

	sstate = setgen('F',"","","");
	if (local) ttflui();		/* If local, flush tty input buffer */
	return(0);
    }
#endif /* NOXFER */

#ifndef NOSPL
    if (cx == XXFOR)			/* FOR loop */
      return(dofor());
#endif /* NOSPL */

#ifndef NOXFER
    /* GET MGET REGET RETRIEVE etc */
    if (cx == XXGET || cx == XXMGET || cx == XXREGET || cx == XXRETR) {
#ifdef IKSD
	if (inserver && !ENABLED(en_sen)) {
	    printf("?Sorry, reception of files is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	return(doxget(cx));
    }
#endif /* NOXFER */

#ifndef NOSPL
#ifndef NOFRILLS
    if (cx == XXGOK) {			/* GETOK */
	return(success = doask(cx));
    }
#endif /* NOFRILLS */
#endif /* NOSPL */

    if (cx == XXHLP) {			/* HELP */
#ifdef NOHELP
	return(dohlp(XXHLP));
#else
	x = cmkey2(cmdtab,ncmd,"C-Kermit command","help",toktab,xxstring,3);
	debug(F101,"HELP command x","",x);
	if (x == -5) {
	    y = chktok(toktab);
	    debug(F101,"top-level cmkey token","",y);
	    ungword();
	    switch (y) {
#ifndef NOPUSH
	      case '!': x = XXSHE; break;
#endif /* NOPUSH */
	      case '#': x = XXCOM; break;
	      case ';': x = XXCOM; break;
#ifndef NOSPL
              case '.': x = XXDEF; break;
	      case ':': x = XXLBL; break;
#endif /* NOSPL */
	      case '&': x = XXECH; break;
	      default:
		printf("\n?Invalid - %s\n",cmdbuf);
		x = -2;
	    }
	}
	return(dohlp(x));
#endif /* NOHELP */
    }

#ifndef NOHELP
    if (cx == XXINT)			/* INTRO */
      return(hmsga(introtxt));
    if (cx == XXNEW) {			/* NEWS */
	int x;
	extern char * k_info_dir;
	x = hmsga(newstxt);
	if (k_info_dir) {
	    printf("The ckermit2.txt file can be found in %s.\n",
		   k_info_dir
		   );
	}
#ifdef UNIX
	printf(
"Type \"man kermit\" at the shell prompt to view the Kermit man page.\n\n"
	       );
#else
#ifdef VMS
	printf(
"Type \"help kermit\" at the DCL prompt to view the Kermit help topic.\n\n"
	       );
#endif /* VMS */
#endif /* UNIX */
	return(x);
    }

#ifdef OS2ONLY
    if (cx == XXUPD) {			/* View UPDATE file */
        extern char exedir[];
        char * pTopic;
        char updstr[2048];
        if ((x = cmtxt("topic name","",&pTopic,xxstring)) < 0)
            return x;
	sprintf(updstr,
		"start view %s\\docs\\k2.inf+%s\\docs\\using_ck.inf+\
%s\\docs\\dialing.inf+%s\\docs\\modems.inf %s",
		exedir,exedir,exedir,exedir,pTopic
		);
        system(updstr);
        return(success = 1);
    }
#endif /* OS2ONLY */
#endif /* NOHELP */

#ifndef NOLOCAL
    if (cx == XXHAN) {			/* HANGUP */
	if ((x = cmcfm()) < 0) return(x);
#ifndef NODIAL
	if ((x = mdmhup()) < 1)
#endif /* NODIAL */
	  x = (tthang() > -1);
#ifndef NODIAL
	dialsta = DIA_UNK;
#endif /* NODIAL */
	whyclosed = WC_CLOS;
	ttchk();			/* In case of CLOSE-ON-DISCONNECT */
#ifdef CKLOGDIAL
	dologend();
#endif /* CKLOGDIAL */
#ifdef OS2
	if (x)
	  DialerSend(OPT_KERMIT_HANGUP, 0);
#endif /* OS2 */
	if (x) haveline = 0;
	return(success = x);
    }
#endif /* NOLOCAL */

#ifndef NOSPL
    if (cx == XXGOTO || cx == XXFWD || cx == XXXFWD) { /* GOTO or FORWARD */
/* Note, here we don't set SUCCESS/FAILURE flag */
	if ((y = cmfld("label","",&s,xxstring)) < 0) {
	    if (y == -3) {
		printf("?Label name required\n");
		return(-9);
	    } else return(y);
	}
	ckstrncpy(lblbuf,s,LBLSIZ);
	if ((x = cmcfm()) < 0) return(x);
	s = lblbuf;
	return(dogoto(s,cx));
    }
#endif /* NOSPL */

#ifndef NOSPL
/* IF, Extended IF, WHILE */
    if (cx == XXIF || cx == XXIFX || cx == XXWHI || cx == XXASSER) {
	return(doif(cx));
    }
    if (cx == XXSWIT) {
	return(doswitch());
    }
#endif /* NOSPL */

#ifndef NOSPL
    /* INPUT, REINPUT, and MINPUT */

    if (cx == XXINP || cx == XXREI
#ifdef CK_MINPUT
	|| cx == XXMINP
#endif /* CK_MINPUT */
	) {
	long zz;
	extern int x_ifnum, ispattern;
	zz = -1L;
	x_ifnum = 1;			/* Turn off internal complaints */
	y = cmnum("Seconds to wait for input,\n or time of day hh:mm:ss",
		  ckitoa(indef), 10, &x, xxstring
		  );
	x_ifnum = 0;
	if (y < 0) {
	    if (y == -2) {		/* Invalid number or expression */
		zz = tod2sec(atmbuf);	/* Convert to secs since midnight */
		if (zz < 0L) {
		    printf("?Number, expression, or time of day required\n");
		    return(-9);
		} else {
		    char now[32];	/* Current time */
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
	if (zz > -1L) {
	    x = zz;
	    if (zz != (long) x) {
		printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
		       );
		return(-9);
	    }
	}
#ifdef CK_MINPUT
	for (y = 0; y < MINPMAX; y++) {	/* Initialize strings */
	    mp[y] = 0;			/* Assume it's not a pattern */
	    if (ms[y]) {
		free(ms[y]);		/* Free old strings, if any */
		ms[y] = NULL;
	    }
	}
	if (cx == XXMINP) {		/* MINPUT */
	    int rc = 0, cfm = 1;
	    for (y = 0; y < MINPMAX; y++) { /* Parse up to MINPMAX strings */
                rc = cmfld("List of input strings","",&s,xxstring);
		debug(F111,"MINPUT cmfld returns",s,rc);
                if (rc < 0) return(rc);
                if (*s == '{') {
		    char *ss;
		    int n;
		    ss = s; n = 0;
		    while (*ss) {*ss = *(ss+1); ss++; n++; }
		    while (--n > 0) if (s[n] == '}') break;
		    if (n > 0) {
			ss = s + n;
			while (*ss) {*ss = *(ss+1); ss++; n++; }
		    }
                }
		if (!(ms[y] = malloc((int)strlen(s) + 1))) { /* Get memory */
		    printf("?Memory allocation failure\n");
		    return(-9);
		}
		strcpy(ms[y],s);	/* Got memory, copy. */
		mp[y] = ispattern ? 1 : 0; /* Mark whether it's a pattern */
                if (rc == 1) {		/* List terminated by CR */
		    cfm = cmcfm();	/* Always call cmcfm() */
		    debug(F101,"MINPUT cmcfm","",cfm);
		    break;
		}
            }
	    for (y++; y < MINPMAX; y++) { /* Clean up old strings */
		if (ms[y]) {
		    free(ms[y]);	/* Free old strings, if any */
		    ms[y] = NULL;
		}
            }
	    debug(F101,"MINPUT end parse x","",x);
	    if (cfm < 0)
	      return(cfm);
#ifdef DEBUG
	    if (deblog) {		/* Check the parsing */
		for (y = 0; y < MINPMAX; y++)
		  if (ms[y]) debug(F111,"MINPUT",ms[y],mp[y]);
	    }
#endif /* DEBUG */

	} else
#endif /* CK_MINPUT */
	{
	    if ((y = cmtxt("Material to be input","",&s,xxstring)) < 0)
	      return(y);
	    mp[0] = ispattern ? 1 : 0;
	    s = brstrip(s);
	    if (!(ms[0] = malloc((int)strlen(s) + 1))) { /* Get memory */
		printf("?Memory allocation failure\n");
		return(-9);
	    }
	    strcpy(ms[0],s);		/* Got memory, copy. */
	    ms[1] = NULL;
	}
	if (cx == XXINP			/* INPUT */
#ifdef CK_MINPUT
	    || cx == XXMINP		/* or MINPUT */
#endif /* CK_MINPUT */
	    ) {
	    i_active = 1;
	    success = doinput(x,ms,mp);	/* Go try to input the search string */
	    i_active = 0;
	} else {			/* REINPUT */
	    debug(F110,"xxrei line",s,0);
	    success = doreinp(x,s,ispattern);
	}
	if (intime[cmdlvl] && !success) { /* TIMEOUT-ACTION = QUIT? */
	    popclvl();			/* If so, pop command level. */
	    if (pflag && cmdlvl == 0) {
		if (cx == XXINP)  printf("?INPUT timed out\n");
		if (cx == XXMINP) printf("?MINPUT timed out\n");
		if (cx == XXREI)  printf("?REINPUT failed\n");
	    }
	}
	return(success);		/* Return do(re)input's return code */
    }

#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXLBL) {			/* LABEL */
	if ((x = cmfld("label","",&s,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Label name required\n");
		return(-9);
	    } else return(x);
	}
	if ((x = cmcfm()) < 0) return(x);
	return(0);
    }
#endif /* NOSPL */

    if (cx == XXLOG) {			/* LOG */
	x = cmkey(logtab,nlog,"What to log","",xxstring);
	if (x == -3) {
	    printf("?Type of log required\n");
	    return(-9);
	}
	if (x < 0) return(x);
	x = dolog(x);
	if (x < 0)
	  return(x);
	else
	  return(success = x);
    }

    if (cx == XXLOGIN) {		/* (REMOTE) LOGIN */
#ifdef IKSD
	if (inserver) {
	    printf("?Already logged in\n");
	    return(-9);
	} else
#endif /* IKSD */
	{
#ifdef NOXFER
	    return(-2);
#else
	    return(dormt(XZLGI));
#endif /* NOXFER */
	}
    }
    if (cx == XXLOGOUT) {		/* (REMOTE) LOGOUT */
#ifdef IKSD
        if (inserver) {
	    if ((x = cmcfm()) < 0)
	      return(x);
	    doexit(GOOD_EXIT,xitsta);
	} else
#endif /* IKSD */
	if (!local || (network && ttchk() < 0)) {
	    printf("?No connection.\n");
	    return(-9);
	} else {
#ifdef NOXFER
	    return(-2);
#else
	    return(dormt(XZLGO));
#endif /* NOXFER */
	}
    }

#ifndef NOSCRIPT
    if (cx == XXLOGI) {			/* UUCP-style script */
	if ((x = cmtxt("expect-send expect-send ...","",&s,xxstring)) < 0)
	  return(x);
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
#ifdef VMS
	conres();			/* For Ctrl-C to work... */
#endif /* VMS */
	return(success = dologin(s));	/* Return 1=completed, 0=failed */
    }
#endif /* NOSCRIPT */

#ifndef NOXFER
#ifdef PIPESEND
    if (cx == XXCREC) {			/* CRECEIVE */
	if (protocol != PROTO_K) {
	    printf("?Sorry, CRECEIVE works only with Kermit protocol\n");
	    return(-9);
	}
    }
    if (cx == XXCGET) {			/* CGET */
	return(doxget(cx));
    }
#endif /* PIPESEND */

    if (cx == XXREC)			/* RECEIVE */
      return(doxget(cx));
#endif /* NOXFER */

#ifndef NOXFER
    if (cx == XXREM) {			/* REMOTE */
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, REMOTE commands only work with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
	x = cmkey(remcmd,nrmt,"Remote Kermit server command","",xxstring);
	if (x == -3) {
	    printf("?You must specify a command for the remote server\n");
	    return(-9);
	}
	return(dormt(x));
    }
#endif /* NOXFER */

#ifndef NORENAME
#ifndef NOFRILLS
    if (cx == XXREN) {			/* RENAME */
#ifdef IKSD
	if (inserver && (!ENABLED(en_ren)
#ifdef CK_LOGIN
			 || isguest
#endif /* CK_LOGIN */
			 )) {
	    printf("?Sorry, renaming of files is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
	    apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	return(dorenam());
    }
#endif /* NOFRILLS */
#endif /* NORENAME */

    if (cx == XXEIGHT) {		/* EIGHTBIT */
	extern int parity, cmask, cmdmsk;
	if ((x = cmcfm()) < 0)
	  return(x);
	parity = 0;
	cmask = 0xff;
	cmdmsk = 0xff;
	return(success = 1);
    }

#ifndef NOXFER
/* SEND, CSEND, MOVE, MAIL, and RESEND use the new common code */

    if (cx == XXSEN			/* SEND */
#ifdef PIPESEND
	|| cx == XXCSEN			/* CSEND */
#endif /* PIPESEND */
	|| cx == XXMOVE			/* MOVE */
	|| cx == XXMAI			/* MAIL */
#ifdef CK_RESEND
	|| cx == XXRSEN			/* RESEND */
#endif /* CK_RESEND */
	) {
#ifdef IKSD
	if (inserver && !ENABLED(en_get)) {
	    printf("?Sorry, sending files is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	return(doxsend(cx));
    }

/* PSEND, ADD, and REMOVE use special parsing */

#ifdef ADDCMD
    /* ADD and REMOVE */
    if (cx == XXADD || cx == XXREMV) {
	char * m;
	m = (cx == XXADD) ? "Add to which list?" : "Remove from which list?";
	x = cmkey(addtab,naddtab,m,"",xxstring);
	if (x < 0)
	  return(x);
#ifndef NOMSEND
	if (x == ADD_SND)
	  return(addsend(cx));
	else
#endif /* NOMSEND */
	  return(doadd(cx,x));
    }
#endif /* ADDCMD */

#ifdef CK_RESEND
    if (cx == XXPSEN) {			/* PSEND */
	int seekto = 0;

	cmarg = cmarg2 = "";
	x = cmifi("File to partially send", "", &s, &y, xxstring);
	if (x < 0) {
	    if (x == -3) {
		printf("?A file specification is required\n");
		return(-9);
	    } else return(x);
	}
	nfils = -1;			/* Files come from internal list. */
#ifndef NOMSEND
        addlist = 0;			/* Don't use SEND-LIST. */
        filenext = NULL;
#endif /* NOMSEND */
	ckstrncpy(line,s,LINBUFSIZ);	/* Save copy of string just parsed. */
	debug(F110,"PSEND line",line,0);
	if (y != 0) {
	    printf("?Sorry, wildcards not permitted in this command\n");
	    return(-9);
	}
	if (sizeof(int) < 4) {
	    printf("?Sorry, this command needs 32-bit integers\n");
	    return(-9);
	}
	x = cmnum("starting position (byte number)",
		  "",10,&seekto,xxstring);
	if (x < 0)
	  return(x);
	zfnqfp(s,fspeclen,fspec);	/* Get full path */
	if ((x = cmtxt("Name to send it with","",&s,NULL)) < 0)
	  return(x);
	ckstrncpy(tmpbuf,s,TMPBUFSIZ);

#ifdef IKSD
	if (inserver && !ENABLED(en_get)) {
	    printf("?Sorry, sending files is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef PIPESEND
	if (sndfilter) {
	    printf("?Sorry, no PSEND while SEND FILTER selected\n");
	    return(-9);
	}
#endif /* PIPESEND */
#ifdef CK_XYZ
	if ((protocol == PROTO_X || protocol == PROTO_XC)) {
	    printf("Sorry, PSEND works only with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */

	cmarg2 = brstrip(tmpbuf);	/* Strip braces */
	cmarg = line;			/* File to send */
	debug(F110,"PSEND filename",cmarg,0);
	debug(F110,"PSEND as-name",cmarg2,0);
	sendstart = seekto;
	sstate = 's';			/* Set start state to SEND */
#ifndef NOMSEND
	addlist = 0;
	filenext = NULL;
#endif /* NOMSEND */
	sendmode = SM_PSEND;
#ifdef MAC
	what = W_SEND;
	scrcreate();
#endif /* MAC */
	if (local) {			/* If in local mode, */
	    displa = 1;			/* enable file transfer display */
	}
	return(0);
    }
#endif /* CK_RESEND */
#endif /* NOXFER */

#ifndef NOXFER
#ifndef NOMSEND
    if (cx == XXMSE || cx == XXMMOVE) {
#ifdef CK_XYZ
	if (protocol == PROTO_X || protocol == PROTO_XC) {
	    printf(
"Sorry, you can only send one file at a time with XMODEM protocol\n"
		   );
	    return(-9);
	}
#endif /* CK_XYZ */
        return(doxsend(cx));
    }

#ifdef COMMENT				/* (moved to doxsend) */
    if (cx == XXMSE || cx == XXMMOVE) {	/* MSEND and MMOVE commands */
	nfils = 0;			/* Like getting a list of */
	lp = line;			/* files on the command line */
	addlist = 0;			/* Do not use SEND-LIST */
	filenext = NULL;		/* Ditto ! */

	while (1) {
	    char *p;
	    if ((x = cmifi("Names of files to send, separated by spaces","",
			   &s,&y,xxstring)) < 0) {
		if (x == -3) {
		    if (nfils <= 0) {
			printf("?A file specification is required\n");
			return(-9);
		    } else break;
		}
		return(x);
	    }
	    msfiles[nfils++] = lp;	/* Got one, count it, point to it, */
	    p = lp;			/* remember pointer, */
	    while (*lp++ = *s++)	/* and copy it into buffer */
	      if (lp > (line + LINBUFSIZ)) { /* Avoid memory leak */
		  printf("?MSEND list too long\n");
		  line[0] = NUL;
		  return(-9);
	      }
	    debug(F111,"msfiles",msfiles[nfils-1],nfils-1);
	    if (nfils == 1) *fspec = NUL; /* Take care of \v(filespec) */
#ifdef ZFNQFP
	    zfnqfp(p,TMPBUFSIZ,tmpbuf);
	    p = tmpbuf;
#endif /* ZFNQFP */
	    if (((int)strlen(fspec) + (int)strlen(p) + 1) < fspeclen) {
		strcat(fspec,p);
		strcat(fspec," ");
	    } else printf("WARNING - \\v(filespec) buffer overflow\n");
	}
	cmlist = msfiles;		/* Point cmlist to pointer array */
	cmarg2 = "";			/* No internal expansion list (yet) */
	sndsrc = nfils;			/* Filenames come from cmlist */
	sendmode = SM_MSEND;		/* Remember this kind of SENDing */
	sstate = 's';			/* Set start state for SEND */
	if (cx == XXMMOVE)		/* If MMOVE'ing, */
	  moving = 1;			/*  set this flag. */
#ifdef MAC
	what = W_SEND;
	scrcreate();
#endif /* MAC */
	if (local) {			/* If in local mode, */
	    displa = 1;			/* turn on file transfer display */
	    ttflui();			/* and flush tty input buffer. */
	}
	return(0);
    }
#endif /* COMMENT */
#endif /* NOMSEND */
#endif /* NOXFER */

#ifndef NOSERVER
    if (cx == XXSER) {			/* SERVER */
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, SERVER only works with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
#ifdef COMMENT
/*
  Parse for time limit, but since we don't use it yet,
  the parsing is commented out.
*/
	x_ifnum = 1;			/* Turn off internal complaints */
	y = cmnum("optional time limit, seconds, or time of day as hh:mm:ss",
		  "0", 10, &x, xxstring
		  );
	x_ifnum = 0;
	if (y < 0) {
	    if (y == -2) {		/* Invalid number or expression */
		zz = tod2sec(atmbuf);	/* Convert to secs since midnight */
		if (zz < 0L) {
		    printf("?Number, expression, or time of day required\n");
		    return(-9);
		} else {
		    char now[32];	/* Current time */
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
	if (zz > -1L) {
	    x = zz;
	    if (zz != (long) x) {
		printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
		       );
		return(-9);
	    }
	}
	if (x < 0)
	  x = 0;
#endif /* COMMENT */

	if ((x = cmcfm()) < 0) return(x);
	sstate = 'x';
#ifdef MAC
	what = W_RECV;
	scrcreate();
#endif /* MAC */
	if (local) displa = 1;
#ifdef AMIGA
	reqoff();			/* No DOS requestors while server */
#endif /* AMIGA */
	return(0);
    }
#endif /* NOSERVER */

    if (cx == XXSAVE) {			/* SAVE command */
	x = cmkey(savtab,nsav,"option","keymap",xxstring);
	if (x == -3) {
	    printf("?You must specify an option to save\n");
	    return(-9);
	}
	if (x < 0) return(x);
	/* have to set success separately for each item in doprm()... */
	/* actually not really, could have just had doprm return 0 or 1 */
	/* and set success here... */
	y = dosave(x);
	if (y == -3) {
	    printf("?More fields required\n");
	    return(-9);
	} else return(y);
    }

    if (cx == XXSET) {			/* SET command */
	x = cmkey(prmtab,nprm,"Parameter","",xxstring);
	if (x == -3) {
	    printf("?You must specify a parameter to set\n");
	    return(-9);
	}
	if (x < 0) return(x);
	/* have to set success separately for each item in doprm()... */
	/* actually not really, could have just had doprm return 0 or 1 */
	/* and set success here... */
	y = doprm(x,0);
	if (y == -3) {
	    printf("?More fields required\n");
	    return(-9);
	} else return(y);
    }

#ifndef NOPUSH
    if (cx == XXSHE			/* SHELL (system) command */
	|| cx == XXEXEC			/* exec() */
	) {
	int rx = 0;
	char * p = NULL;
	int i,n;
#ifdef UNIXOROSK
	char * args[256];
#endif /* UNIXOROSK */

#ifdef IKSD
	if (inserver && (nopush || !ENABLED(en_hos))) {
	    printf("?Sorry, host command access is disabled\n");
	    return(-9);
	}
#endif /* IKSD */

#ifdef CKEXEC
	if (cx == XXEXEC) {		/* EXEC (overlay ourselves) */
	    struct FDB sw, fl;
	    cmfdbi(&sw,			/* First FDB - command switches */
		   _CMKEY,		/* fcode */
		   "Command to overlay C-Kermit\n or switch", /* hlpmsg */
		   "",			/* default */
		   "",			/* addtl string data */
		   1,			/* addtl numeric data 1: tbl size */
		   4,			/* addtl numeric data 2: 4 = cmswi */
		   xxstring,		/* Processing function */
		   redirsw,		/* Keyword table */
		   &fl			/* Pointer to next FDB */
		   );
	    cmfdbi(&fl,			/* 2nd FDB - command to exec */
		   _CMFLD,		/* fcode */
		   "Command to overlay C-Kermit", /* hlpmsg */
		   "",			/* default */
		   "",			/* addtl string data */
		   0,			/* addtl numeric data 1 */
		   0,			/* addtl numeric data 2 */
		   xxstring,
		   NULL,
		   NULL			/* No more after this */
		   );
	    while (1) {
		x = cmfdb(&sw);		/* Parse something */
		debug(F101,"exec cmfdb","",x);
		if (x < 0)
		  return(x);
		/* Generalize this if we add more switches */
		if (cmresult.fcode == _CMKEY) {
		    rx = 1;
		    continue;
		}
		if (cmresult.fcode == _CMFLD)
		  break;
		return(-2);
	    }
	    ckstrncpy(tmpbuf,cmresult.sresult,TMPBUFSIZ);
	    if (!tmpbuf[0]) {
		printf("?Command required\n");
		return(-9);
	    }
	    p = brstrip(tmpbuf);
	    args[0] = NULL;		/* Set argv[0] to it */
	    makestr(&args[0],p);
	    for (i = 1; i < 255; i++) {	/* Get arguments for command */
		if ((x = cmfld("Argument","",&s,xxstring)) < 0) {
		    if (x == -3) {
			if ((x = cmcfm()) < 0)
			  return(x);
			break;
		    } else
		      return(x);
		}
		args[i] = NULL;
		s = brstrip(s);
		makestr(&args[i],s);
	    }
	    args[i] = NULL;
	} else {
#endif /* CKEXEC */
	    if ((x = cmtxt("System command to execute","",&s,xxstring)) < 0)
	      return(x);
#ifdef CKEXEC
	}
#endif /* CKEXEC */
        if (nopush)
          return(success = 0);
#ifdef CK_APC
	if (apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	conres();			/* Make console normal  */
#ifdef OS2
	if (!(s && *s)) {
	    os2push();
	    x = 1;
	} else
#endif /* OS2 */
	  if (cx == XXSHE) {
	      x = zshcmd(s);
	      debug(F101,"RUN zshcmd code","",x);
	      concb((char)escape);
	      return(success = x);
#ifdef CKEXEC
	  } else {
#ifdef DEBUG
	      if (deblog) {
		  debug(F111,"EXEC cmd",p,0);
		  for (i = 0; i < 256 && args[i]; i++)
		    debug(F111,"EXEC arg",args[i],i);
	      }
#endif /* DEBUG */
	      if (p) {
		  z_exec(p,args,rx);	/* Overlay ourself */
		  debug(F100,"EXEC fails","",0);
		  concb((char)escape);	/* In case it returns */
	      }
	      return(success = 0);
#endif /* CKEXEC */
	  }
    }

#ifdef CK_REDIR
    if (cx == XXFUN) {			/* REDIRECT */
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	sprintf(tmpbuf,
		"Local command to run,\n\
with its standard input/output redirected to %s\n",
		ttname);
	if ((x = cmtxt(tmpbuf,"",&s,xxstring)) < 0)
	  return(x);
        if (nopush) {
            printf("?REDIRECT disabled\n");
            return(success=0);
        }
	if (!local) {
	    printf("?SET LINE or SET HOST required first\n");
	    return(-9);
	}
	return(success = ttruncmd(s));
    }
#endif /* CK_REDIR */
#endif /* NOPUSH */

#ifndef NOSHOW
    if (cx == XXSHO) {			/* SHOW */
	x = cmkey(shotab,nsho,"","parameters",xxstring);
	if (x < 0) return(x);
	return(doshow(x));
    }
#endif /* NOSHOW */

#ifndef MAC
    if (cx == XXSPA) {			/* SPACE */
#ifdef IKSD
	if (inserver && !ENABLED(en_spa)) {
	    printf("?Sorry, SPACE command disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef datageneral
	/* AOS/VS can take an argument after its "space" command. */
	if ((x = cmtxt("Confirm, or local directory name","",&s,xxstring)) < 0)
	  return(x);
	if (nopush) {
	    printf("?Sorry, SPACE command disabled\n");
	    return(-9);
	} else if (*s == NUL) {
	    xsystem(SPACMD);
	} else {
	    sprintf(line,"space %s",s);
	    xsystem(line);
	}
#else
#ifdef OS2
	if ((x = cmtxt("Press Enter for current disk,\n\
 or specify a disk letter like A:","",&s,xxstring)) < 0)
	  return(x);
	if (*s == NUL) {		/* Current disk */
	    printf(" Free space: %ldK\n", zdskspace(0)/1024L);
	} else {
	    int drive = toupper(*s);
	    printf(" Drive %c: %ldK free\n", drive,
		   zdskspace(drive - 'A' + 1) / 1024L);
	}
#else
#ifdef UNIXOROSK
	x = cmdir("Confirm for current disk,\n\
 or specify a disk device or directory","",&s,xxstring);
	if (x == -3)
	  s = "";
	else if (x < 0)
	  return(x);
        ckstrncpy(tmpbuf,s,TMPBUFSIZ);
        s = tmpbuf;
	if ((x = cmcfm()) < 0) return(x);
	if (nopush) {
	    printf("?Sorry, SPACE command disabled\n");
	    return(-9);
	}
	if (!*s) {			/* Current disk */
	    xsystem(SPACMD);
	} else {			/* Specified disk */
	    sprintf(line,"%s %s",SPACM2,s);
	    xsystem(line);
	}
#else
	if ((x = cmcfm()) < 0) return(x);
	if (nopush) {
	    printf("?Sorry, SPACE command disabled\n");
	    return(-9);
	}
	xsystem(SPACMD);
#endif /* UNIXOROSK */
#endif /* OS2 */
#endif /* datageneral */
	return(success = 1);		/* Pretend it worked */
    }
#endif /* MAC */

#ifndef NOXFER
    if (cx == XXSTA) {			/* STATISTICS */
	if ((x = cmkey(stattab,2,"Carriage return, or option",
		       "/brief",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0) return(y);
	return(success = dostat(x));
    }
#endif /* NOXFER */

    if (cx == XXSTO || cx == XXEND) {	/* STOP, END, or POP */
	if ((y = cmnum("exit status code","0",10,&x,xxstring)) < 0)
	  return(y);
	if ((y = cmtxt("Message to print","",&s,xxstring)) < 0)
	  return(y);
	s = brstrip(s);
	if (*s) printf("%s\n",s);
	if (cx == XXSTO) {
	    dostop();
	} else {
#ifndef NOSPL
	    /* Pop from all FOR/WHILE/XIF/SWITCH's */
	    debug(F101,"END maclvl 1","",maclvl);
	    while ((maclvl > 0) &&
		   (m_arg[maclvl-1][0]) &&
		   (cmdstk[cmdlvl].src == CMD_MD) &&
		     (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
		      !strncmp(m_arg[maclvl-1][0],"_for",4) ||
		      !strncmp(m_arg[maclvl-1][0],"_whi",4) ||
		      !strncmp(m_arg[maclvl-1][0],"_swi",4))) {
		debug(F110,"END popping",m_arg[maclvl-1][0],0);
		dogta(XXPTA);		/* Put args back */
		popclvl();		/* Pop up two levels */
		popclvl();
		debug(F101,"END maclvl 2","",maclvl);
	    }
            if (maclvl > -1) {
		if (mrval[maclvl])	/* Free previous retval if any */
		  free(mrval[maclvl]);
		mrval[maclvl] = malloc(16); /* Room for up to 15 digits */
		if (mrval[maclvl])
		  sprintf(mrval[maclvl],"%d",x); /* Record current retval */
	    }
#endif /* NOSPL */
	    popclvl();			/* Now pop out of macro or TAKE file */
#ifndef NOSPL
#ifdef DEBUG
	    if (deblog) {
		debug(F101,"END maclvl 3","",maclvl);
		debug(F111,"END mrval[maclvl]",mrval[maclvl],maclvl);
		debug(F111,"END mrval[maclvl+1]",mrval[maclvl+1],maclvl+1);
	    }
#endif /* DEBUG */
#endif /* NOSPL */
	}
	return(success = (x == 0));
    }
    if (cx == XXSUS) {			/* SUSPEND */
	if ((y = cmcfm()) < 0) return(y);
#ifdef NOJC
	printf("Sorry, this version of Kermit cannot be suspended\n");
#else
#ifdef IKSD
	if (inserver) {
	    printf("?Sorry, IKSD can not be suspended\n");
	    return(-9);
	} else
#endif /* IKSD */
	  if (nopush) {
	    printf("?Sorry, access to system is disabled\n");
	    return(-9);
	}
	stptrap(0);
#endif /* NOJC */
	return(0);
    }

    if (cx == XXTAK) {			/* TAKE */
#ifdef OS2
	extern char startupdir[],exedir[],inidir[];
	char * scriptenv, * keymapenv;
#endif /* OS2 */
	char takepath[1024];

	if (tlevel >= MAXTAKE-1) {
	    printf("?Take files nested too deeply\n");
	    return(-9);
	}
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
                zhome(),
                zhome(), "SCRIPTS/",
                zhome(), "KEYMAPS/",
                exedir,
                exedir, "SCRIPTS/",
                exedir, "KEYMAPS/"
                );
#else /* not OS2 */
	y = 1024;
	s = takepath;
	zzstring("\\v(home)",&s,&y);
#endif /* OS2 */

	if ((y = cmifip("C-Kermit command file",
			"",&s,&x,0,takepath,xxstring)) < 0) {
	    if (y == -3) {
		printf("?A file name is required\n");
		return(-9);
	    } else
	      return(y);
	}
	if (x != 0) {
	    printf("?Wildcards not allowed in command file name\n");
	    return(-9);
	}
	ckstrncpy(line,s,LINBUFSIZ);
	debug(F110,"take file",s,0);
	if (isdir(s)) {
	    printf("?Can't execute a directory - \"%s\"\n", s);
	    return(-9);
	}
#ifndef NOTAKEARGS
	{
	    char * p;
	    /* extern char **m_xarg[]; */
	    x = strlen(line);
	    p = line + x + 1;
	    if ((y = cmtxt("Optional arguments","",&s,xxstring)) < 0)
	      return(y);
	    if (*s) {			/* Args given? */
		ckstrncpy(p,s,LINBUFSIZ-x-1);
#ifdef ZFNQFP
		zfnqfp(line,TMPBUFSIZ,tmpbuf);
		s = tmpbuf;
#else
		s = line;
#endif /* ZFNQFP */
		addmac("%0",s);		/* Define %0 = name of file */
		varnam[0] = '%';
		varnam[2] = '\0';
		debug(F110,"take arg 0",s,0);
		debug(F110,"take args",p,0);
		for (y = 1; y < 10; y++) { /* Clear current args %1..%9 */
		    varnam[1] = (char) (y + '0');
		    delmac(varnam);
		}
		xwords(p,MAXARGLIST,NULL,0); /* Assign new args */
		debug(F110,"take args",p,0);
	    }
	}
#else
	if ((y = cmcfm()) < 0) return(y);
#endif /* NOTAKEARGS */
	return(success = dotake(line));
    }

#ifndef NOLOCAL
#ifdef OS2
    if (cx == XXVIEW) {			/* VIEW Only Terminal mode */
	viewonly = TRUE;
	success = doconect(0);
	viewonly = FALSE;
	return success;
    }
#endif /* OS2 */

#ifdef NETCONN
    if (cx == XXTEL || cx == XXIKSD) {	/* TELNET */
	int x,z;
#ifdef OS2
    if (!tcp_avail) {
        printf("?Sorry, either TCP/IP is not available on this system or\n\
necessary DLLs did not load.  Use SHOW NETWORK to check network status.\n");
        success = 0;
        return(-9);
    } else
#endif /* OS2 */
      {
	  x = nettype;			/* Save net type in case of failure */
	  z = ttnproto;			/* Save protocol in case of failure */
	  nettype = NET_TCPB;
	  ttnproto = (cx == XXTEL) ? NP_TELNET : NP_KERMIT;
	  if ((y = setlin(XYHOST,0,1)) < 0) {
              nettype = x;		/* Failed, restore net type. */
              ttnproto = z;		/* and protocol */
              success = 0;
	  }
	  didsetlin++;
        }
	return(y);
    }

#ifndef PTYORPIPE
#ifdef NETCMD
#define PTYORPIPE
#else
#ifdef NETPTY
#define PTYORPIPE
#endif /* NETPTY */
#endif /* NETCMD */
#endif /* PTYORPIPE */

#ifdef PTYORPIPE
    if (cx == XXPIPE || cx == XXPTY) {	/* PIPE or PTY */
	int x;
	extern int netsave;
	x = nettype;			/* Save net type in case of failure */
	nettype = (cx == XXPIPE) ? NET_CMD : NET_PTY;
	if ((y = setlin(XYHOST,0,1)) < 0) {
	    nettype = x;		/* Failed, restore net type. */
	    ttnproto = z;		/* and protocol */
	    success = 0;
	}
	didsetlin++;
	netsave = x;
	return(y);
    }
#endif /* PTYORPIPE */

    if (cx == XXRLOG) {			/* RLOGIN */
#ifdef RLOGCODE
	int x,z;
#ifdef OS2
	if (!tcp_avail) {
	    printf("?Sorry, either TCP/IP is not available on this system or\n\
necessary DLLs did not load.  Use SHOW NETWORK to check network status.\n"
		   );
	    success = 0;
	    return(-9);
	} else {
#endif /* OS2 */
	    x = nettype;		/* Save net type in case of failure */
	    z = ttnproto;		/* Save protocol in case of failure */
	    nettype = NET_TCPB;
	    ttnproto = NP_RLOGIN;
	    if ((y = setlin(XYHOST,0,1)) < 0) {
		nettype = x;		/* Failed, restore net type. */
		ttnproto = z;		/* and protocol */
		success = 0;
	    }
	    didsetlin++;
#ifdef OS2
	}
#endif /* OS2 */
	return(y);
#else
	printf("?Sorry, RLOGIN is not configured in this copy of C-Kermit.\n");
	return(-9);
#endif /* RLOGCODE */
    }
#endif /* NETCONN */
#endif /* NOLOCAL */

#ifndef NOXMIT
    if (cx == XXTRA) {			/* TRANSMIT */
	int i, n, xpipe = 0, xbinary = 0, xxlate = 1, xxnowait = 0, getval;
	char c;
	struct FDB sf, sw, tx;		/* FDBs for parse functions */
#ifndef NOCSETS
	extern int tcs_transp;		/* Term charset is transparent */
#else
	int tcs_transp = 1;
#endif /* NOCSETS */

#ifdef COMMENT
	xbinary = binary;		/* Default text/binary mode */
#else
	xbinary = 0;			/* Default is text */
#endif /* COMMENT */

	cmfdbi(&sw,			/* First FDB - command switches */
	       _CMKEY,			/* fcode */
	       "Filename, or switch",	/* hlpmsg */
	       "",			/* default */
	       "",			/* addtl string data */
	       nxmitsw,			/* addtl numeric data 1: tbl size */
	       4,			/* addtl numeric data 2: 4 = cmswi */
	       xxstring,		/* Processing function */
	       xmitsw,			/* Keyword table */
	       &sf			/* Pointer to next FDB */
	       );
	cmfdbi(&sf,			/* 2nd FDB - file to send */
	       _CMIFI,			/* fcode */
	       "File to transmit",	/* hlpmsg */
	       "",			/* default */
	       "",			/* addtl string data */
	       0,			/* addtl numeric data 1 */
	       0,			/* addtl numeric data 2 */
	       xxstring,
	       NULL,
#ifdef PIPESEND
	       &tx
#else
	       NULL
#endif /* PIPESEND */
	       );
#ifdef PIPESEND
	cmfdbi(&tx,
	     _CMTXT,			/* fcode */
	     "Command",			/* hlpmsg */
	       "",			/* default */
	       "",			/* addtl string data */
	       0,			/* addtl numeric data 1 */
	       0,			/* addtl numeric data 2 */
	       xxstring,
	       NULL,
	       NULL
	       );
#endif /* PIPESEND */

	while (1) {
	    x = cmfdb(&sw);
	    if (x < 0)
	      return(x);
	    if (cmresult.fcode != _CMKEY)
	      break;
	    c = cmgbrk();		/* Have switch, get break character */
	    if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
		printf("?This switch does not take an argument\n");
		return(-9);
	    }
	    if (!getval && (cmgkwflgs() & CM_ARG)) {
		printf("?This switch requires an argument\n");
		return(-9);
	    }
	    n = cmresult.nresult;	/* Numeric result = switch ID */
	    switch (n) {		/* Process the switch */
#ifdef PIPESEND
	      case XMI_CMD:		/* Transmit from a command */
		if (nopush) {
		    printf("?Sorry, system command access is disabled\n");
		    return(-9);
		}
		sw.hlpmsg = "Command, or switch"; /* Change help message */
		xpipe = 1;		/* (No way to undo this one) */
		break;
#endif /* PIPESEND */

	      case XMI_BIN:		/* Binary */
		xbinary = 1;
		xxlate = 0;		/* Don't translate charsets */
		break;

	      case XMI_TXT:		/* Text */
		xbinary = 0;
		xxlate = !tcs_transp;	/* Translate if TERM CHAR not TRANSP */
		break;

	      case XMI_TRA:		/* Transparent text */
		xbinary = 0;
		xxlate = 0;		/* But don't translate charsets */
		break;

#ifdef COMMENT
	      case XMI_VRB:		/* /VERBOSE */
	      case XMI_QUI:		/* /QUIET */
		break;			/* (not implemented yet) */
#endif /* COMMENT */

	      case XMI_NOW:		/* /NOWAIT */
		xxnowait = 1;
		break;

	      default:
		return(-2);
	    }

	}
	if (cmresult.fcode != _CMIFI && cmresult.fcode != _CMTXT)
	  return(-2);
	ckstrncpy(line,cmresult.sresult,LINBUFSIZ); /* Filename */
	s = line;
	if ((y = cmcfm()) < 0)		/* Confirm */
	  return(y);
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	if (cmresult.nresult != 0) {
	    printf("?Only a single file may be transmitted\n");
	    return(-9);
	}
#ifdef PIPESEND
	if (xpipe) {
	    s = brstrip(s);
	    if (!*s) {
		printf("?Sorry, a command to send from is required\n");
		return(-9);
	    }
	    pipesend = 1;
	}
#endif /* PIPESEND */
	success = transmit(s,
			   (xxnowait ? (char)0 : (char)xmitp),
			   xxlate,
			   xbinary
			   );
	return(success);
    }
#endif /* NOXMIT */

#ifndef NOFRILLS
    if (cx == XXTYP || cx == XXMORE) {	/* TYPE or MORE */
	int paging = 0, havename = 0, head = 0, width = 0, count = 0;
	char pfxbuf[64], * prefix = NULL;
	extern int xaskmore;
	struct FDB sf, sw;
	char * pat = NULL;

	if (cx == XXMORE)
	  paging = 1;
	else
	  paging = (typ_page < 0) ? xaskmore : typ_page;

#ifdef IKSD
	if (inserver && !ENABLED(en_typ)) {
	    printf("?Sorry, TYPE command disabled\n");
	    return(-9);
	}
#endif /* IKSD */

    cmfdbi(&sw,				/* 2nd FDB - optional /PAGE switch */
	   _CMKEY,			/* fcode */
	   "Filename or switch",	/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   ntypetab,			/* addtl numeric data 1: tbl size */
	   4,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
	   typetab,			/* Keyword table */
	   &sf				/* Pointer to next FDB */
	   );

    cmfdbi(&sf,				/* 1st FDB - file to type */
	   _CMIFI,			/* fcode */
	   "",				/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   0,				/* addtl numeric data 1 */
	   0,				/* addtl numeric data 2 */
	   xxstring,
	   NULL,
	   NULL
	   );

	while (!havename) {
	    x = cmfdb(&sw);		/* Parse something */
	    debug(F101,"dotype cmfdb","",x);
	    debug(F101,"dotype cmresult.fcode","",cmresult.fcode);
	    debug(F101,"dotype cmresult.nresult","",cmresult.nresult);
	    if (x < 0)			/* Error */
	      return(x);
	    else if (cmresult.fcode == _CMKEY) {
		char c; int getval;
		c = cmgbrk();
		getval = (c == ':' || c == '=');
		if (getval && !(cmgkwflgs() & CM_ARG)) {
		    printf("?This switch does not take an argument\n");
		    return(-9);
		}
#ifdef COMMENT
		if (!getval && (cmgkwflgs() & CM_ARG)) {
		    printf("?This switch requires an argument\n");
		    /* Not if it has a default! */
		    return(-9);
		}
#endif /* COMMENT */
		switch (cmresult.nresult) {
#ifdef CK_TTGWSIZ
		  case TYP_PAG:
		    paging = 1;
		    break;

		  case TYP_NOP:
		    paging = 0;
		    break;
#endif /* CK_TTGWSIZ */

		  case TYP_COU:
		    count = 1;
		    break;

		  case TYP_HEA:
		  case TYP_TAI:
		    y = 10;
		    if (getval)
		      if ((x = cmnum("Number of lines",
				     "10",10,&y,xxstring)) < 0)
			return(x);
		    head = (cmresult.nresult == TYP_TAI) ? -y : y;
		    break;

		  case TYP_WID:
		    y = typ_wid > -1 ? typ_wid : cmd_cols;
		    if (getval)
		      if ((x = cmnum("Column at which to truncate",
				     ckitoa(y),10,&y,xxstring)) < 0)
			return(x);
		    width = y;
		    break;

		  case TYP_PAT:
		    if (!getval && (cmgkwflgs() & CM_ARG)) {
			printf("?This switch requires an argument\n");
			return(-9);
		    }
		    if ((x = cmfld("pattern","",&s,xxstring)) < 0)
		      return(x);
		    ckstrncpy(tmpbuf,s,TMPBUFSIZ);
		    pat = tmpbuf;
		    break;

		  case TYP_PFX:
		    if (!getval && (cmgkwflgs() & CM_ARG)) {
			printf("?This switch requires an argument\n");
			return(-9);
		    }
		    if ((x = cmfld("prefix for each line","",&s,xxstring)) < 0)
		      return(x);
		    if ((int)strlen(s) > 63) {
			printf("?Too long - 63 max\n");
			return(-9);
		    }
		    ckstrncpy(pfxbuf,s,64);
		    prefix = brstrip(pfxbuf);
		    break;
		}
	    } else if (cmresult.fcode == _CMIFI)
	      havename = 1;
	    else
	      return(-2);
	}
	if (havename) {
	    ckstrncpy(line,cmresult.sresult,LINBUFSIZ);
	    y = cmresult.nresult;
	} else {
	    if ((x = cmifi("Filename","",&s,&y,xxstring)) < 0) {
		if (x == -3) {
		    printf("?Name of an existing file required\n");
		    return(-9);
		} else return(x);
	    }
	    ckstrncpy(line,s,LINBUFSIZ);
	}
	if (y != 0) {
	    printf("?A single file please\n");
	    return(-9);
	}
	if ((y = cmcfm()) < 0)		/* Confirm the command */
	  return(y);

	if (count) paging = -1;
	debug(F111,"dotype",line,paging);
	return(success = dotype(line,paging,0,head,pat,width,prefix));
    }
#endif /* NOFRILLS */

#ifndef NOCSETS
    if (cx == XXXLA)  {	   /* TRANSLATE <ifn> from-cs to-cs <ofn> */
#ifdef OS2ONLY
        extern int tt_font;
#endif /* OS2ONLY */
	int incs, outcs;
        char * tocs = "";
	if ((x = cmifi("File to translate","",&s,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of an existing file\n");
		return(-9);
	    } else return(x);
	}
	if (y != 0) {
	    printf("?A single file please\n");
	    return(-9);
	}
	ckstrncpy(line,s,LINBUFSIZ);	/* Save copy of string just parsed. */

	if ((incs = cmkey(fcstab,nfilc,"from character-set","",xxstring)) < 0)
	  return(incs);
#ifdef OS2
        if (isunicode()) {
	    tocs = "ucs2";
        } else {
            y = os2getcp();		/* Default is current code page */
            switch (y) {
	      case 437: tocs = "cp437"; break;
	      case 850: tocs = "cp850"; break;
	      case 852: tocs = "cp852"; break;
	      case 857: tocs = "cp857"; break;
	      case 858: tocs = "cp858"; break;
	      case 862: tocs = "cp862"; break;
	      case 866: tocs = "cp866"; break;
	      case 869: tocs = "cp869"; break;
	      case 1250: tocs = "cp1250"; break;
	      case 1251: tocs = "cp1251"; break;
	      case 1252: tocs = "cp1252"; break;
	      case 1253: tocs = "cp1253"; break;
	      case 1254: tocs = "cp1254"; break;
	      case 1255: tocs = "cp1255"; break;
	      case 1256: tocs = "cp1256"; break;
	      case 1257: tocs = "cp1257"; break;
	      case 1258: tocs = "cp1258"; break;
            }
        }
#ifdef OS2ONLY
/*
   If the user has loaded a font with SET TERMINAL FONT then we want
   to change the default code page to the font that was loaded.
*/
	if (tt_font != TTF_ROM) {
            extern struct keytab termfont[];
            extern int ntermfont;
	    for (y = 0; y < ntermfont; y++) {
		if (termfont[y].kwval == tt_font) {
		    tocs = termfont[y].kwd;
		    break;
		}
	    }
	}
#endif /* OS2ONLY */
#else /* OS2 */
					/* Make current file char set */
	for (y = 0; y <= nfilc; y++)	/* be the default... */
	  if (fcstab[y].kwval == fcharset) {
	      tocs = fcstab[y].kwd;
	      break;
	  }
#endif /* OS2 */

	if ((outcs = cmkey(fcstab,nfilc,"to character-set",tocs,xxstring)) < 0)
	  return(outcs);
	if ((x = cmofi("output file",CTTNAM,&s,xxstring)) < 0) return(x);
	if (x > 1) {
	    printf("?Directory name not allowed\n");
	    return(-9);
	}
	ckstrncpy(tmpbuf,s,TMPBUFSIZ);
	if ((y = cmcfm()) < 0) return(y); /* Confirm the command */
	return(success = xlate(line,tmpbuf,incs,outcs)); /* Execute it */
    }
#endif /* NOCSETS */

    if (cx == XXVER) {			/* VERSION */
	int n = 0;
	extern char * ck_patch, * ck_s_test;
	extern int hmtopline;
	if ((y = cmcfm()) < 0)
          return(y);

	printf("\n%s, for%s\n Numeric: %ld",versio,ckxsys,vernum);
	printf("\n");
	n = 3;
	if (*ck_s_test) {
	    printf("\n THIS IS A TEST VERSION, NOT FOR PRODUCTION USE.\n");
	    n += 2;
	}
	if (*ck_patch) {
	    printf(" Patches: %s\n", ck_patch);
	    n++;
	}
#ifdef OS2
	printf(" Type COPYRIGHT for copyright information.\n\n");
	shoreg();
#else
	hmtopline = n+1;
	hmsga(copyright);
	hmtopline = 0;
#endif /* OS2 */
	return(success = 1);
    }

    if (cx == XXCPR) {			/* COPYRIGHT */
	if ((y = cmcfm()) < 0)
          return(y);
	hmsga(copyright);
	return(success = 1);
    }

#ifndef MAC				/* Only for multiuser systems */
#ifndef OS2
#ifndef NOFRILLS
    if (cx == XXWHO) {			/* WHO */
	char *wc;
#ifdef IKSD
	if (inserver && !ENABLED(en_who)) {
	    printf("?Sorry, WHO command disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef datageneral
	if ((z = cmcfm()) < 0) return(z);
	if (nopush) {
	    printf("?Sorry, who not allowed\n");
	    return(success = 0);
	}
        xsystem(WHOCMD);
#else
	if ((y = cmtxt("user name","",&s,xxstring)) < 0) return(y);
        if (nopush) {
	    printf("?Sorry, WHO command disabled\n");
	    return(success = 0);
	}
	if (!(wc = getenv("CK_WHO"))) wc = WHOCMD;
	if (wc)
	  if ((int) strlen(wc) > 0) {
	      sprintf(line,"%s %s",wc,s);
	      xsystem(line);
	  }
#endif /* datageneral */
	return(success = 1);
    }
#endif /* NOFRILLS */
#endif /* OS2 */
#endif /* MAC */

#ifndef NOFRILLS
    if (cx == XXWRI || cx == XXWRL || cx == XXWRBL) { /* WRITE */
	if ((x = cmkey(writab,nwri,"to file or log","",xxstring)) < 0) {
	    if (x == -3) printf("?Write to what?\n");
	    return(x);
	}
	if ((y = cmtxt("text","",&s,xxstring)) < 0) return(y);
	s = brstrip(s);
	switch (x) {
	  case LOGD: y = ZDFILE; break;
	  case LOGP: y = ZPFILE; break;
#ifndef NOLOCAL
	  case LOGS: y = ZSFILE; break;
#endif /* NOLOCAL */
	  case LOGT: y = ZTFILE; break;
#ifndef NOSPL
	  case LOGW: y = ZWFILE; break;
#endif /* NOSPL */
	  case LOGX:			/* SCREEN (stdout) */
	  case LOGE:			/* ERROR  (stderr) */
	    if (x == LOGE) {
		debug(F110,
		      (cx == XXWRL) ? "WRITELN ERROR" : "WRITE ERROR", s,0);
		fprintf(stderr,"%s%s",s,(cx == XXWRL) ? "\n" : "");
	    } else {
		debug(F110,
		      (cx == XXWRL) ? "WRITELN SCREEN" : "WRITE SCREEN", s,0);
		printf("%s%s",s,(cx == XXWRL) ? "\n" : "");
	    }
	    return(success = 1);
	  default: return(-2);
	}
	if (chkfn(y) > 0) {
	    x = (cx == XXWRI) ? zsout(y,s) : zsoutl(y,s);
	    if (x < 0) printf("?Write error\n");
	} else {
	    x = -1;
	    printf("?File or log not open\n");
	}
	return(success = (x == 0) ? 1 : 0);
    }
#endif /* NOFRILLS */

#ifndef NOXFER
    if (cx == XXASC || cx == XXBIN) {
	if ((x = cmcfm()) < 0) return(x);
	binary = (cx == XXASC) ? XYFT_T : XYFT_B;
	return(success = 1);
    }
#endif /* NOXFER */

    if (cx == XXCLS) {
	if ((x = cmcfm()) < 0) return(x);
	y = ck_cls();
	return(success = (y > -1) ? 1 : 0);
    }

#ifdef CK_MKDIR
    if (cx == XXMKDIR) {
	char *p;
#ifdef IKSD
	if (inserver && !ENABLED(en_mkd)) {
	    printf("?Sorry, directory creation is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	if ((x = cmfld("Name for new directory","",&s,xxstring)) < 0) {
	    if (x != -3) {
		return(x);
	    } else {
		printf("?Directory name required\n");
		return(-9);
	    }
	}
	ckstrncpy(line,s,LINBUFSIZ);
	s = line;
	if ((x = cmcfm()) < 0) return(x);
	s = brstrip(s);
	x = ckmkdir(0,s,&p,msgflg,0);
#ifdef COMMENT
	if (msgflg && x == 0)
	  printf("?Directory already exists\n");
#endif /* COMMENT */
	return(success = (x < 0) ? 0 : 1);
    }
    if (cx == XXRMDIR) {		/* RMDIR */
	char *p;
#ifdef IKSD
	if (inserver && !ENABLED(en_rmd)) {
	    printf("?Sorry, directory removal is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
	if ((x = cmdir("Name of directory to be removed","",&s,xxstring)) < 0)
	  return(x);
	ckstrncpy(line,s,LINBUFSIZ);
	s = line;
	if ((x = cmcfm()) < 0) return(x);
	s = brstrip(s);
	x = ckmkdir(1,s,&p,msgflg,0);
	return(success = (x < 0) ? 0 : 1);
    }
#endif /* CK_MKDIR */

#ifdef TCPSOCKET
    if (cx == XXTELOP) {
        if ((x = cmkey(telcmd, ntelcmd, "TELNET command", "", xxstring)) < 0 )
	  return(x);
        switch (x) {
          case WILL:
          case WONT:
          case DO:
          case DONT:
            if ((y = cmkey(tnopts,ntnopts,"TELNET option","",xxstring)) < 0)
	      return(y);
            if ((z = cmcfm()) < 0) return(z);

            switch (x) {
	      case WILL:
		if (TELOPT_UNANSWERED_WILL(y))
		  return(success = 0);
		break;
	      case WONT:
		if (TELOPT_UNANSWERED_WONT(y))
		  return(success = 0);
		break;
	      case DO:
		if (TELOPT_UNANSWERED_DO(y))
		  return(success = 0);
		break;
	      case DONT:
		if (TELOPT_UNANSWERED_DONT(y))
		  return(success = 0);
		break;
            }
            if (local) {
		success = ((tn_sopt(x,y) > -1) ? 1 : 0);
            } else {
		printf("ff%02x%02x\n",x,y);
		success = 1;
            }
            if (success) {
		switch (x) {
		  case WILL:
		    TELOPT_UNANSWERED_WILL(y) = 1;
		    break;
		  case WONT:
		    TELOPT_UNANSWERED_WONT(y) = 1;
		    break;
		  case DO:
		    TELOPT_UNANSWERED_DO(y) = 1;
		    break;
		  case DONT:
		    TELOPT_UNANSWERED_DONT(y) = 1;
		    break;
		}
		if (tn_wait("XXTELOP") < 0) {
		    tn_push();
		    success = 0;
		}
            }
            return(success);
          case SB:
            if ((y=cmkey(tnsbopts,ntnsbopts,"TELNET option","",xxstring)) < 0)
	      return(y);
            switch (y) {
	      case TELOPT_NAWS:
		/* Some compilers require switch() to have at least 1 case */
#ifdef CK_NAWS
                TELOPT_SB(TELOPT_NAWS).naws.x = 0;
                TELOPT_SB(TELOPT_NAWS).naws.y = 0;
                if (local)
		  return(success = ((tn_snaws() > -1) ? 1 : 0));
                else
		  return(success = 0);
                break;
#else
		return(success = 0);
#endif /* CK_NAWS */
	      default:
                return(success = 0);
            }
            break;
	  default:
            if ((z = cmcfm()) < 0) return(z);
            if (local) {
                CHAR temp[3];
                if (network && ttnproto == NP_TELNET) { /* TELNET */
                    temp[0] = (CHAR) IAC;
                    temp[1] = x;
                    temp[2] = NUL;
                    success = (ttol((CHAR *)temp,2) > -1 ? 1 : 0);
                    if (tn_deb || debses || deblog) {
                        sprintf(tn_msg,"TELNET SENT %s",TELCMD(x));
                        debug(F101,tn_msg,"",x);
                        if (debses || tn_deb) tn_debug(tn_msg);
                    }
                    return(success);
                }
            } else {
                printf("ff%02x\n",x);
                return(success = 1);
            }
        }
    }
#endif /* TCPSOCKET */

#ifndef NOPUSH
    if (cx == XXNPSH) {
	if ((z = cmcfm()) < 0) return(z);
        nopush = 1;
#ifndef NOSERVER
        en_hos = 0;
#endif /* NOSERVER */
#ifdef PIPESEND
	usepipes = 0;
#endif /* PIPESEND */
        return(success = 1);
    }
#endif /* NOPUSH */

#ifndef NOSPL
    if (cx == XXLOCAL)			/* LOCAL variable declarations */
      return(success = dolocal());
#endif /* NOSPL */

    if (cx == XXKERMI) {		/* The KERMIT command */
	char * list[65];
	extern char **xargv;
	extern int xargc;
	int i;
	if ((y = cmtxt("kermit command-line arguments, -h for help",
		       "",&s,xxstring)) < 0)
	  return(y);
	strcpy(line,"kermit ");
	strncat(line,s,LINBUFSIZ-8);
	xwords(line,64,list,0);
	for (i = 1; i < 64; i++) {
	    if (!list[i])
	      break;
	}
	i--;
	xargc = i;
	xargv = list;
	xargv++;
	sstate = cmdlin();
	if (sstate) {
	    extern int justone;
	    debug(F000,"KERMIT sstate","",sstate);
	    justone = 1;		/* Force return to command mode */
	    proto();			/* after protocol */
	    return(success);
	} else {
	    debug(F101,"KERMIT sstate","",sstate);
	    return(success = 1);	/* Not exactly right, but... */
	}
    }
    if (cx == XXDATE) {			/* DATE command */
	if ((y = cmtxt("date and/or time, or carriage return for current",
		       "",&s,xxstring)) < 0)
	  return(y);
	s = ckcvtdate(s,1);
	printf("%s\n",s);
	return(success = ((int)strcmp(s,"<BAD_DATE>") ? 1 : 0));
    }
#ifndef NOPUSH
#ifndef NOFRILLS
    if (cx == XXEDIT) {
	char * p = NULL;
	if (!editor[0]) {
	    s = getenv("EDITOR");
	    if (s) ckstrncpy(editor,s,CKMAXPATH);
	    editor[CKMAXPATH] = NUL;
	    if (!editor[0]) {
		printf("?Editor not defined - use SET EDITOR to define\n");
		return(-9);
	    }
	}
	ckstrncpy(tmpbuf,editfile,TMPBUFSIZ);
/*
   cmiofi() lets us parse the name of an existing file, or the name of
   a nonexistent file to be created.
*/
	x = cmiofi("File to edit", (char *)tmpbuf, &s, &y, xxstring);
	if (x < 0) {
	    if (x == -9) {
		if (zchko(s) < 0) {
		    printf("Can't create \"%s\"\n",s);
		    return(x);
		}
	    } else if (x != -3)
	      return(x);
	}
	if (x == -3)
	  tmpbuf[0] = NUL;
	else {
	    ckstrncpy(tmpbuf,s,TMPBUFSIZ);
	    if (iswild((char *)tmpbuf)) {
		printf("?A single file please\n");
		return(-9);
	    }
	}
	if ((z = cmcfm()) < 0) return(z);
	if (nopush) {
	    printf("?Sorry, editing not allowed\n");
	    return(success = 0);
	}
	if (tmpbuf[0]) {
	/* Get full path in case we change directories between EDIT commands */
	    zfnqfp(tmpbuf, CKMAXPATH, editfile);
	    editfile[CKMAXPATH] = NUL;
#ifdef OS2
            p = editfile;		/* Flip the stupid slashes */
            while (*p) {
                if (*p == '/') *p = '\\';
                p++;
            }
#endif /* OS2 */
	} else
	  editfile[0] = NUL;
	s = line;
	x = 0;
	if (editopts[0]) {
#ifdef OS2
	    x = ckindex("%1",(char *)editopts,0,0,1);
	    if (x > 0)
	      editopts[x] = 's';
	    else
#endif /* OS2 */
	      x = ckindex("%s",(char *)editopts,0,0,1);
	}
	if (x)
	  sprintf(tmpbuf,editopts,editfile);
	else
	  sprintf(tmpbuf,"%s %s",editopts,editfile);
	sprintf(s,"%s %s",editor,tmpbuf);
#ifdef OS2
        p = s + strlen(editor);		/* And again with the slashes */
        while (p != s) {
            if (*p == '/') *p = '\\';
            p--;
        }
#endif /* OS2 */
	conres();
	x = zshcmd(s);
	concb((char)escape);
	return(x);
    }
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef BROWSER				/* Defined only ifndef NOPUSH */
    if (cx == XXBROWS) {
#ifdef OS2
        char * p = NULL;
#endif /* OS2 */
	if (nopush) {
	    printf("?Sorry, browsing not allowed\n");
	    return(success = 0);
	}
#ifndef NT
        /* Windows lets the Shell Execute the URL if no Browser is defined */
	if (!browser[0]) {
	    s = getenv("BROWSER");
	    if (s) ckstrncpy(browser,s,CKMAXPATH);
	    browser[CKMAXPATH] = NUL;
	    if (!browser[0]) {
		printf("?Browser not defined - use SET BROWSER to define\n");
		return(-9);
	    }
	}
#endif /* NT */
	ckstrncpy(tmpbuf,browsurl,TMPBUFSIZ);
	if ((x = cmtxt("URL",(char *)browsurl,&s,xxstring)) < 0)
	  return(x);
	ckstrncpy(browsurl,s,4096);
	s = line;

	x = 0;
	if (browsopts[0]) {
#ifdef OS2
	    x = ckindex("%1",(char *)browsopts,0,0,1);
	    if (x > 0)
	      browsopts[x] = 's';
	    else
#endif /* OS2 */
	      x = ckindex("%s",(char *)browsopts,0,0,1);
	}
	if (x)
	  sprintf(tmpbuf,browsopts,browsurl);
	else
	  sprintf(tmpbuf,"%s %s",browsopts,browsurl);
#ifdef NT
        if ( !browser[0] ) {
            return(success = Win32ShellExecute( browsurl ));
        }
#endif /* NT */
	sprintf(s,"%s %s",browser,tmpbuf);
#ifdef OS2
        p = line + strlen(browser);	/* Flip slashes */
        while (p != line) {
            if (*p == '/') *p = '\\';
            p--;
        }
#endif /* OS2 */
	conres();
	x = zshcmd(s);
	concb((char)escape);
	return(x);
    }
#endif /* BROWSER */

#ifdef CK_TAPI
    if (cx == XXTAPI) {			/* Microsoft TAPI */
	return (success = dotapi());
    }
#endif /* CK_TAPI */

#ifndef NOXFER
    if (cx == XXWHERE) {
	extern char * rfspec, * sfspec, * srfspec, * rrfspec;
	if ((x = cmcfm()) < 0) return(x);
	printf("\nFile most recently...\n\n");
	printf("  Sent:       %s\n",   sfspec ? sfspec : "(none)");
	if (sfspec && srfspec) {
	    printf("  Stored as:  %s\n",   srfspec);
	    printf("\n");
	}
	printf("  Received:   %s\n",   rrfspec ? rrfspec : "(none)");
	if (rfspec && rrfspec)
	printf("  Stored as:  %s\n",   rfspec);
	printf(
"\nIf the full path is not shown, then the file is probably in your current\n"
	       );
	printf(
"directory or your download directory (if any - SHOW FILE to find out).\n\n"
	       );
	return(success = 1);
    }
#endif /* NOXFER */

#ifdef CK_RECALL
    if (cx == XXREDO) {			/* Find a previous cmd and redo it */
	extern int on_recall, in_recall;
	int x;
	char * p;

	if ((x = cmtxt(
"pattern, or first few characters of a previous command",
		       "*",&s,xxstring)) < 0)
	  return(x);
	ckstrncpy(line,s,LINBUFSIZ);
	x = strlen(s);
	s = line;
	if (*s == '{') {		/* Braces disable adding * to end */
	    if (s[x-1] == '}') {
		s[x-1] = NUL;
		s++;
		x--;
	    }
	} else {			/* No braces, add * to end. */
	    s[x] = '*';
	    s[x+1] = NUL;
	}

	while (x > 0 && s[x] == '*' && s[x-1] == '*') s[x--] = NUL;

	if (!on_recall || !in_recall) {
	    printf("?Sorry, command recall can't be used now.\n");
	    return(-9);
	}
	if (p = cmgetcmd(s)) {		/* Look for it history buffer */
	    sprintf(cmdbuf,"%s%c",p,'\r'); /* Found - copy to command buffer */
	    cmaddnext();		/* Force re-add to history buffer */
	    return(cmflgs = -1);	/* Force reparse */
	} else {
	    printf("?Sorry - \"%s\" not found\n", s);
	    return(-9);
	}
    }
#endif /* CK_RECALL */

/* This is useless unless user is root.  Too bad, it's just what we need. */

#ifdef COMMENT
    if (cx == XXCHRT) {			/* Change root directory */
#ifdef UNIX
	if ((x = cmdir("Name of new root directory","",&s,xxstring)) < 0)
	  return(x);
	ckstrncpy(line,s,LINBUFSIZ);
	s = line;
	if ((x = cmcfm()) < 0) return(x);
	s = brstrip(s);
	priv_on();			/* This doesn't work if setuid root */
	x = chroot(s);			/* See priv_ini() in ckutio.c. */
	priv_off();
	if (x) {
	    printf("Error %d\n",errno);	/* 1 = EPERM (i.e. "not root") */
	    perror("chroot");
	    return(success = 0);
	}
	return(success = zchdir(s));
#else
	printf("?Sorry, not implemented\n");
	return(-9);
#endif /* UNIX */
    }
#endif /* COMMENT */

#ifdef CK_KERBEROS
    if (cx == XXAUTH) {			/* KERBEROS */
	x = cp_auth();			/* Parse it */
	if (x < 0)			/* Pass parse errors back */
	  return(x);
#ifdef IKSD
        if (inserver) {
            printf("?Command disabled in IKSD.\r\n");
            return(success = 0);
        }
#endif /* IKSD */
	return(success = doauth(cx));
    }
#endif /* CK_KERBEROS */

#ifndef NOLOCAL
    if (cx == XXTERM) {
	return(settrmtyp());
    }
#endif /* NOLOCAL */

    if (cx == XXSTATUS) {
	if ((x = cmcfm()) < 0) return(x);
	printf( " %s\n", success ? "SUCCESS" : "FAILURE" );
	return(0);			/* Don't change it */
    }

    if (cx == XXFAIL) {
	if ((x = cmcfm()) < 0) return(x);
	return(success = 0);
    }

    if (cx == XXSUCC) {
	if ((x = cmcfm()) < 0) return(x);
	return(success = 1);
    }

    if (cx == XXNLCL) {
	extern int nolocal;
	if ((x = cmcfm()) < 0) return(x);
	nolocal = 1;
	return(success = 1);
    }

#ifndef NOXFER
    if (cx == XXRASG)			/* Shortcuts for REMOTE commands */
      return(dormt(XZASG));
    if (cx == XXRCWD)
      return(dormt(XZCWD));
    if (cx == XXRCPY)
      return(dormt(XZCPY));
    if (cx == XXRDEL)
      return(dormt(XZDEL));
    if (cx == XXRDIR)
      return(dormt(XZDIR));
    if (cx == XXRXIT)
      return(dormt(XZXIT));
    if (cx == XXRHLP)
      return(dormt(XZHLP));
    if (cx == XXRHOS)
      return(dormt(XZHOS));
    if (cx == XXRKER)
      return(dormt(XZKER));
    if (cx == XXRPWD)
      return(dormt(XZPWD));
    if (cx == XXRQUE)
      return(dormt(XZQUE));
    if (cx == XXRREN)
      return(dormt(XZREN));
    if (cx == XXRMKD)
      return(dormt(XZMKD));
    if (cx == XXRRMD)
      return(dormt(XZRMD));
    if (cx == XXRSET)
      return(dormt(XZSET));
    if (cx == XXRSPA)
      return(dormt(XZSPA));
    if (cx == XXRTYP)
      return(dormt(XZTYP));
    if (cx == XXRWHO)
      return(dormt(XZWHO));
#endif /* NOXFER */

    if (cx == XXRESET) {		/* RESET */
	if ((x = cmcfm()) < 0)
	  return(x);
	doclean(0);			/* Close all files */
	return(success = 1);
    }

#ifndef NOXFER
#ifndef NOCSETS
    if (cx == XXASSOC) {		/* ASSOCIATE */
	extern struct keytab tcstab[];
	extern int ntcs;
        if ((x = cmkey(assoctab, nassoc, "", "", xxstring)) < 0 )
	  return(x);

	switch (x) {			/* Associate what? */

	  case ASSOC_TC:		/* Transfer character-set... */
	    if ((x = cmkey(tcstab, ntcs,
			   "transfer character-set name","",xxstring)) < 0)
	      return(x);
	    if ((y = cmkey(fcstab, nfilc,
			   "with file character-set","", xxstring)) < 0)
	      if (y != -3)
		return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    axcset[x] = y;
	    return(success = 1);

	  case ASSOC_FC:		/* File character-set... */
	    if ((x = cmkey(fcstab, nfilc,
			   "file character-set name","",xxstring)) < 0)
	      return(x);
	    if ((y = cmkey(tcstab, ntcs,
			   "with transfer character-set","", xxstring)) < 0)
	      if (y != -3)
		return(y);
	    if ((z = cmcfm()) < 0)
	      return(z);
	    afcset[x] = y;
	    return(success = 1);

	  default:
	    return(-2);
	}
    }
#endif /* NOCSETS */
#endif /* NOXFER */

#ifndef NOSPL
    if (cx == XXSHIFT) {		/* SHIFT */
	if ((y = cmnum("Number of arguments to shift","1",10,&x,xxstring)) < 0)
	  return(y);
	if ((z = cmcfm()) < 0)
	  return(z);
	return(success = doshift(x));
    }
#endif /* NOSPL */

#ifndef NOHELP
    if (cx == XXMAN) {
#ifdef OS2
	if ((x = cmcfm()) < 0)
	  return(x);
	if (nopush) {
	    printf("?Sorry, access to system commands is disabled.\n");
	    return(-9);
	}
        y = mxlook(mactab,"manual",nmac);
        if (y > -1) {
            z = maclvl;			/* save the current maclvl */
            dodo(y,NULL,cmdstk[cmdlvl].ccflgs);	/* Run the macro */
            while (maclvl > z) {
                debug(F101,"XXMAN loop maclvl 1","",maclvl);
                sstate = (CHAR) parser(1);
                debug(F101,"XXMAN loop maclvl 2","",maclvl);
                if (sstate) proto();
            }
            debug(F101,"XXMAN loop exit maclvl","",maclvl);
            return(success);
        }
        return(success = 0);
#else
	if ((x = cmtxt(
#ifdef UNIX
		    "Carriage return to confirm the command, or manual topic",
#else
		    "Carriage return to confirm the command, or help topic",
#endif /* UNIX */
		       "kermit",
		       &s,
		       xxstring
		       )
	     ) < 0)
	  return(x);
#endif /* OS2 */

#ifdef UNIX
	sprintf(tmpbuf,"man %s",s);
#else
	sprintf(tmpbuf,"help %s",s);
#endif /* UNIX */
	debug(F110,"MANUAL",tmpbuf,0);
	if (nopush) {
	    printf("?Sorry, access to system commands is disabled.\n");
	    return(-9);
	} else {
	    conres();			/* Restore the console */
	    success = zshcmd(tmpbuf);
	    concb((char)escape);	/* Restore CBREAK mode */
	    return(success);
	}
    }
#endif /* NOHELP */

#ifndef NOSPL
    if (cx == XXSORT)			/* SORT an array */
      return(dosort());
#endif /* NOSPL */

    if (cx == XXPURGE) {
#ifdef IKSD
	if (inserver && (!ENABLED(en_del)
#ifdef CK_LOGIN
                          || isguest
#endif /* CK_LOGIN */
			 )) {
	    printf("?Sorry, DELETE is disabled\n");
	    return(-9);
	}
#endif /* IKSD */
#ifdef CK_APC
	if (apcactive == APC_LOCAL ||
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
#ifdef CKPURGE
        return(dopurge());
#else
#ifdef VMS
	if ((x = cmtxt("optional switches followed by filespec",
		       "",&s,xxstring)) < 0)
	  return(x);
	if (nopush) {
	    printf("?Sorry, DCL access is disabled\n");
	    return(-9);
	}
	ckstrncpy(line,s,LINBUFSIZ);
	s = line;
	x = mlook(mactab,"purge",nmac);
	return(success = dodo(x,s,cmdstk[cmdlvl].ccflgs));
#else
	return(-2);
#endif /* VMS */
#endif /* CKPURGE */
    }

#ifndef NOSPL
    if (cx == XXFAST) {
	if ((x = cmcfm()) < 0) return(x);
	x = mlook(mactab,"fast",nmac);
	return(success = dodo(x,NULL,cmdstk[cmdlvl].ccflgs));
    }
    if (cx == XXCAU) {
	if ((x = cmcfm()) < 0) return(x);
	x = mlook(mactab,"cautious",nmac);
	return(success = dodo(x,NULL,cmdstk[cmdlvl].ccflgs));
    }
    if (cx == XXROB) {
	if ((x = cmcfm()) < 0) return(x);
	x = mlook(mactab,"robust",nmac);
	return(success = dodo(x,NULL,cmdstk[cmdlvl].ccflgs));
    }
#endif /* NOSPL */

    if (cx == XXSCRN) {			/* SCREEN */
	int row, col;
	if ((x = cmkey(scntab, nscntab,"screen action","", xxstring)) < 0)
	  return(x);
	switch (x) {			/* MOVE-TO (cursor position) */
	  case SCN_MOV:
	    if ((y = cmnum("Row (1-based)","",10,&z,xxstring)) < 0)
	      return(y);
	    row = z;
	    y = cmnum("Column (1-based)","1",10,&z,xxstring);
	    if (y < 0)
	      return(y);
	    col = z;
	    if ((y = cmcfm()) < 0)
	      return(y);
	    if (row < 0 || col < 0) {
		printf("?Row and Column must be 1 or greater\n");
		return(-9);
	    }
	    if (cmd_rows > 0 && row > cmd_rows)
	      row = cmd_rows;
	    if (cmd_cols > 0 && col > cmd_cols)
	      col = cmd_cols;
	    y = ck_curpos(row,col);
	    return(success = (y > -1) ? 1 : 0);

	  case SCN_CLR:			/* CLEAR */
	    if ((y = cmcfm()) < 0)
	      return(y);
	    debug(F100,"screen calling ck_cls()","",0);
	    y = ck_cls();
	    return(success = (y > -1) ? 1 : 0);

	  case SCN_CLE:			/* CLEOL */
	    if ((y = cmcfm()) < 0)
	      return(y);
	    y = ck_cleol();
	    return(success = (y > -1) ? 1 : 0);
	}
    }

#ifndef NOHTTP
#ifdef TCPSOCKET
    if (cx == XXHTTP) {			/* HTTP */
	struct FDB sw, kw, fi;
	int n, getval;
	char c, * p;

	char * http_agent = NULL;	/* Parse results */
	char * http_hdr   = NULL;
	char * http_user  = NULL;
	char * http_pass  = NULL;
	char * http_mime  = NULL;
	char * http_lfile = NULL;
	char * http_rfile = NULL;
	char http_array = NUL;
	int http_action = -1;

#ifdef OS2
	p = "Kermit 95";		/* Default user agent */
#else
	p = "C-Kermit";
#endif /* OS2 */
	makestr(&http_agent,p);
	makestr(&http_mime,"text/HTML"); /* MIME type default */

	cmfdbi(&sw,			/* 1st FDB - general switches */
	   _CMKEY,			/* fcode */
	   "GET, HEAD, PUT, INDEX, or POST,\n or switch", /* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   nhttpswtab,			/* addtl numeric data 1: tbl size */
	   4,				/* addtl numeric data 2: 4 = cmswi */
	   xxstring,			/* Processing function */
	   httpswtab,			/* Keyword table */
	   &kw				/* Pointer to next FDB */
	   );
	cmfdbi(&kw,			/* 2nd FDB - commands */
	   _CMKEY,			/* fcode */
	   "Command",			/* hlpmsg */
	   "",				/* default */
	   "",				/* addtl string data */
	   nhttptab,			/* addtl numeric data 1: tbl size */
	   0,				/* addtl numeric data 2: 0 = keyword */
	   xxstring,			/* Processing function */
	   httptab,			/* Keyword table */
	   NULL				/* Pointer to next FDB */
	   );

	while (1) {
	    x = cmfdb(&sw);		/* Parse something */
	    if (x < 0)			/* Error */
	      goto xhttp;
	    n = cmresult.nresult;
	    if (cmresult.fdbaddr == &kw) /* Command - exit this loop */
	      break;
	    c = cmgbrk();		/* Switch... */
	    getval = (c == ':' || c == '=');
	    if (getval && !(cmgkwflgs() & CM_ARG)) {
		printf("?This switch does not take an argument\n");
		x = -9;
		goto xhttp;
	    }
	    switch (cmresult.nresult) {	/* Handle each switch */
	      case HT_SW_AG:		/* /AGENT */
		if (getval) {
		    if ((x = cmfld("User agent",p,&s,xxstring)) < 0)
		      goto xhttp;
		} else
		  s = p;
		makestr(&http_agent,s);
		break;
	      case HT_SW_HD:		/* /HEADER */
		s = NULL;
		if (getval) {
		    if ((x = cmfld("Header line","",&s,xxstring)) < 0) {
			if (x == -3)
			  s = NULL;
			else
			  goto xhttp;
		    }
		}
		makestr(&http_hdr,s);
		break;
	      case HT_SW_US:		/* /USER */
		s = NULL;
		if (getval) {
		    if ((x = cmfld("User ID","",&s,xxstring)) < 0)
		      goto xhttp;
		}
		makestr(&http_user,s);
		break;
	      case HT_SW_PW:		/* /PASSWORD */
		s = NULL;
		if (getval) {
		    if ((x = cmfld("Password","",&s,xxstring)) < 0)
		      goto xhttp;
		}
		makestr(&http_pass,s);
		break;
#ifndef NOSPL
	      case HT_SW_AR: {		/* /ARRAY: */
		  char * s2, array = NUL;
		  if (!getval) {
		      printf("?This switch requires an argument\n");
		      x = -9;
		      goto xhttp;
		  }
		  if ((x = cmfld("Array name (a single letter will do)",
				 "",
				 &s,
				 NULL
				 )) < 0) {
		      if (x == -3) {
			  printf("?Array name required\n");
			  x = -9;
			  goto xhttp;
		      } else
			goto xhttp;
		  }
		  if (!*s) {
		      printf("?Array name required\n");
		      x = -9;
		      goto xhttp;
		  }
		  s2 = s;
		  if (*s == CMDQ) s++;
		  if (*s == '&') s++;
		  if (!isalpha(*s)) {
		      printf("?Bad array name - \"%s\"\n",s2);
		      x = -9;
		      goto xhttp;
		  }
		  array = *s++;
		  if (isupper(array))
		    array = tolower(array);
		  if (*s && (*s != '[' || *(s+1) != ']')) {
		      printf("?Bad array name - \"%s\"\n",s2);
		      http_array = NUL;
		      x = -9;
		      goto xhttp;
		  }
		  http_array = array;
		  break;
	      }
#endif /* NOSPL */
	      default:
		x = -2;
		goto xhttp;
	    }
	}
	http_action = n;		/* Save the action */
	if (http_action == HTTP_PUT || http_action == HTTP_POS) {
	    cmfdbi(&sw,			/* 1st FDB - switch */
		   _CMKEY,		/* fcode */
		   "Local filename\n Or switch", /* help */
		   "",			/* default */
		   "",			/* addtl string data */
		   nhttpptab,		/* keyword table size */
		   4,			/* addtl numeric data 2: 4 = cmswi */
		   xxstring,		/* Processing function */
		   httpptab,		/* Keyword table */
		   &fi			/* Pointer to next FDB */
		   );
	    cmfdbi(&fi,			/* 2nd FDB - filename */
		   _CMIFI,		/* fcode */
		   "Local filename",	/* hlpmsg */
		   "",			/* default */
		   "",			/* addtl string data */
		   0,			/* addtl numeric data 1 */
		   0,			/* addtl numeric data 2 */
		   xxstring,
		   NULL,
		   NULL
		   );
	    while (1) {
		x = cmfdb(&sw);
		if (x < 0)
		  goto xhttp;		/* Free any malloc'd temp strings */
		n = cmresult.nresult;
		if (cmresult.fcode != _CMKEY)
		  break;
		c = cmgbrk();		/* Switch... */
		getval = (c == ':' || c == '=');
		if (getval && !(cmgkwflgs() & CM_ARG)) {
		    printf("?This switch does not take an argument\n");
		    x = -9;
		    goto xhttp;
		}
		switch (n) {
		  case HT_PP_MT:
		    s = "text/HTML";
		    if (getval) {
			if ((x = cmfld("MIME type",
				       "text/HTML",&s,xxstring)) < 0)
			  goto xhttp;
		    }
		    makestr(&http_mime,s);
		    break;
		  default:
		    x = -2;
		    goto xhttp;
		}
	    }
	    makestr(&http_lfile,cmresult.sresult);
	    if ((x = cmtxt("Remote filename","",&s,xxstring)) < 0)
	      goto xhttp;
	    if (!*s) s = http_lfile;
	    makestr(&http_rfile,s);
	}
	switch (http_action) {
          case HTTP_DEL:                /* DELETE */
	    if ((x = cmfld("Remote source file","",&s,xxstring)) < 0)
	      goto xhttp;
	    makestr(&http_rfile,s);
            break;
          case HTTP_HED:                /* HEAD */
              if ((x = cmfld("Remote source file","",&s,xxstring)) < 0)
                goto xhttp;
              makestr(&http_rfile,s);

              n = ckindex("/",http_rfile,-1,1,0);
              if ( n )
                  p = &http_rfile[n];
              else
                  p = http_rfile;
              if ((x = cmofi("Local filename",p,&s,xxstring)) < 0)
                goto xhttp;
              makestr(&http_lfile,s);
              break;
          case HTTP_GET:		/* GET */
          case HTTP_IDX:                /* INDEX */
	    if ((x = cmfld("Remote source file","",&s,xxstring)) < 0)
	      goto xhttp;
	    makestr(&http_rfile,s);
	    if ((x = cmofi("Local filename","",&s,xxstring)) < 0)
              if (x != -3)
                goto xhttp;
	    makestr(&http_lfile,s);
	    break;
	}
	if ((x = cmcfm()) < 0)
	  goto xhttp;

	x = dohttp(http_action,
		   http_lfile,
		   http_rfile,
		   http_agent,
		   http_hdr,
		   http_user,
		   http_pass,
		   http_mime,
		   http_array
		   );
      xhttp:
	if (http_agent) free(http_agent);
	if (http_hdr)   free(http_hdr);
	if (http_user)  free(http_user);
	if (http_pass)  free(http_pass);
	if (http_mime)  free(http_mime);
	if (http_lfile) free(http_lfile);
	if (http_rfile) free(http_rfile);
	if (x > -1)
	  success = x;
	return(x);
    }
#endif /* TCPSOCKET */
#endif /* NOHTTP */

#ifndef NOSPL
    if (cx == XXARRAY) {		/* ARRAY */
#ifndef NOSHOW
	extern int showarray();
#endif /* NOSHOW */
	if ((x = cmkey(arraytab, narraytab,"Array operation","",xxstring)) < 0)
	  return(x);
	switch (x) {
	  case ARR_DCL:
	    return(dodcl());
	  case ARR_SRT:
	    return(dosort());
#ifndef NOSHOW
	  case ARR_SHO:
	    return(showarray());
#endif /* NOSHOW */
	  case ARR_CPY:
	    return(copyarray());
	  case ARR_SET:
	  case ARR_CLR:
	    return(clrarray(x));
	  case ARR_DST:
	    return(unarray());
	  case ARR_RSZ:
	    return(rszarray());
	  default:
	    printf("?Sorry, not implemented yet - \"%s\"\n",cmdbuf);
	    return(-9);
	}
    }
    if (cx == XXTRACE) {
	int on = 1;
	struct FDB sw, kw;
	cmfdbi(&sw,			/* 1st FDB - switch */
	       _CMKEY,			/* fcode */
	       "Trace object;\n Or switch", /* help */
	       "",			/* default */
	       "",			/* addtl string data */
	       2,			/* keyword table size */
	       4,			/* addtl numeric data 2: 4 = cmswi */
	       xxstring,		/* Processing function */
	       onoffsw,			/* Keyword table */
	       &kw			/* Pointer to next FDB */
	       );
	cmfdbi(&kw,			/* 2nd FDB - Trace object */
	       _CMKEY,			/* fcode */
	       "Trace object",		/* help */
	       "all",			/* default */
	       "",			/* addtl string data */
	       ntracetab,		/* keyword table size */
	       0,			/* addtl numeric data 2: 0 = keyword */
	       xxstring,		/* Processing function */
	       tracetab,		/* Keyword table */
	       NULL			/* Pointer to next FDB */
	       );
	if ((x = cmfdb(&sw)) < 0)
	  return(x);
	if (cmresult.fdbaddr == &sw) {
	    on = cmresult.nresult;
	    if ((x = cmkey(tracetab, ntracetab,"","all",xxstring)) < 0)
	      return(x);
	} else {
	    x = cmresult.nresult;
	}
	if ((y = cmcfm()) < 0)
	  return(y);

	switch (x) {
	  case TRA_ASG:
	    tra_asg = on;
	    break;
	  case TRA_CMD:
	    tra_cmd = on;
	    break;
	  case TRA_ALL:
	    tra_asg = on;
	    tra_cmd = on;
	    break;
	  default:
	    return(-2);
	}
	printf("TRACE %s\n", on ? "ON" : "OFF");
	return(success = 1);
    }

    if (cx == XXNOTAV) {		/* Command in table not available */
	ckstrncpy(tmpbuf,atmbuf,TMPBUFSIZ);
	if ((x = cmtxt("Rest of command","",&s,NULL)) < 0)
	  return(x);
	printf("Sorry, \"%s\" not configured in this version of Kermit.\n",
	       tmpbuf
	       );
	return(success = 0);
    }
#endif /* NOSPL */

    return(-2);				/* None of the above */

} /* end of docmd() */

#endif /* NOICP */
