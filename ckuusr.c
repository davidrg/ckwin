#include "ckcsym.h"
char *userv = "User Interface 6.0.177, 6 Sep 96";

/*  C K U U S R --  "User Interface" for C-Kermit (Part 1)  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
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
  NOTE: Because of the massive additions in functionality, and therefore
  the increase in the number of commands, much code was moved from here to
  the two new modules, ckuus4.c and ckuus5.c.  This module now contains only
  the top-level command keyword table, the SET command keyword table, and
  the top-level interactive command parser/dispatcher.  ckuus3.c contains the
  rest of the SET and REMOTE command parsers; ckuus2.c contains the help
  command parser and help text strings, and ckuus4.c and ckuus5.c contain
  miscellaneous pieces that logically belong in the ckuusr.c file but had to
  be moved because of size problems with some C compilers / linkers.
  Later...  as the other modules became too large, a ckuus6.c was created.
  Still later...  ckuus7.c.
  Also: ckuusy.c contains the UNIX-style command-line interface;
  ckuusx.c contains routines needed by both the command-line interface and
  the interactive command parser.
*/

/*
 The ckuus*.c modules depend on the existence of C library features like fopen,
 fgets, feof, (f)printf, argv/argc, etc.  Other functions that are likely to
 vary among Unix implementations -- like setting terminal modes or interrupts
 -- are invoked via calls to functions that are defined in the system-
 dependent modules, ck?[ft]io.c.  The command line parser processes any
 arguments found on the command line, as passed to main() via argv/argc.  The
 interactive parser uses the facilities of the cmd package (developed for this
 program, but usable by any program).  Any command parser may be substituted
 for this one.  The only requirements for the Kermit command parser are these:

1. Set parameters via global variables like duplex, speed, ttname, etc.  See
   ckmain.c for the declarations and descriptions of these variables.

2. If a command can be executed without the use of Kermit protocol, then
   execute the command directly and set the variable sstate to 0. Examples
   include 'set' commands, local directory listings, the 'connect' command.

3. If a command requires the Kermit protocol, set the following variables:

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
    cmarg2 is the name to send a single file under, or
       the name under which to store an incoming file; must not be wild.
       If it's the name for receiving, a null value means to store the
       file under the name it arrives with.
    cmlist is a list of nonwild filenames, such as passed via argv.
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

#ifndef NOICP
/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
#include "ckcnet.h"			/* Network symbols */
#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h> 
#undef COMMENT
#else 
#define APIRET ULONG
#include <windows.h>
#include "cknwin.h"
#include "ckntap.h"			/* CK_TAPI definition */
#endif /* NT */
#include "ckowin.h"
#include "ckocon.h"
extern int tcp_avail;
extern bool viewonly;
extern tt_status;
#endif /* OS2 */

#ifdef datageneral
#include <packets:common.h>
#define fgets(stringbuf,max,fd) dg_fgets(stringbuf,max,fd)
#endif /* datageneral */

/* External Kermit Variables, see ckmain.c for description. */

extern xx_strp xxstring;
extern long xvernum;
extern char *ck_ver;

extern int size, local, sndsrc, xitsta, server, displa, binary, msgflg,
  escape, duplex, nfils, quiet, tlevel, pflag, zincnt, atcapr, atdiso, verwho,
  ckxech, carrier, deblog, sendmode, epktflg, what, moving, protocol;
extern int bye_active;
extern long sendstart;
#ifdef CK_TTYFD
extern int ttyfd;
#endif /* CK_TTYFD */

extern long vernum;
extern char *versio, *copyright[];
extern char *ckxsys, *cmarg, *cmarg2, **cmlist;
#ifndef NOHELP
extern char *introtxt[];
extern char *newstxt[];
#endif /* NOHELP */
extern char *PWDCMD, *WHOCMD, *TYPCMD;
extern char ttname[];
#ifndef NOFRILLS
extern int rmailf;			/* MAIL command items */
extern char optbuf[];
#endif /* NOFRILLS */
extern CHAR sstate;

#ifdef NETCONN
extern int network,			/* Have active network connection */
  nettype,  				/* Type of network */
  ttnproto;				/* Network Protocol */
#endif /* NETCONN */

#ifndef NODIAL
extern int dialsta, dialatmo, dialcon, dialcq; /* DIAL status, etc. */
#endif /* NODIAL */

#ifdef CK_APC
extern int apcactive, apcstatus;
#endif /* CK_APC */

#ifndef NOMSEND				/* Multiple SEND */
extern char *msfiles[];
int filesinlist = 0;			/* And ADD ... */
extern struct filelist * filehead;
extern struct filelist * filetail;
extern struct filelist * filenext;
extern int addlist;
static struct keytab addtab[] = {
    "send-list", 0, 0
};
static int naddtab = sizeof(addtab)/sizeof(struct keytab);
#endif /* NOMSEND */

extern char fspec[];			/* Most recent filespec */

#ifndef NOCSETS
extern int nfilc;
extern struct keytab fcstab[];
#endif /* NOCSETS */

#ifdef CK_TMPDIR
int f_tmpdir = 0;			/* Directory changed temporarily */
char savdir[TMPDIRLEN];			/* For saving current directory */
extern char * dldir;
#endif /* CK_TMPDIR */

int activecmd = -1;

#ifdef COMMENT
#ifdef pdp11
/* Normally this is defined in ckcfns.c */
#define ENCBUFL 200
CHAR encbuf[ENCBUFL];
#endif /* pdp11 */
#endif /* COMMENT */

int rcflag = 0;				/* Pointer to home directory string */
int repars,				/* Reparse needed */
    techo = 0;				/* Take echo */
#ifndef NOSCRIPT
int secho = 1;
#endif /* NOSCRIPT */

#ifdef OS2
int xitwarn = 1;	        /* Warn about open connection on exit */
#else
int xitwarn = 1;
#endif /* OS2 */

#ifndef NOXMIT
/* Variables for TRANSMIT command */

int xmitx = 1;			/* Whether to echo during TRANSMIT */
int xmitf = 0;			/* Character to fill empty lines */
int xmitl = 0;			/* 0 = Don't send linefeed too */
int xmitp = LF;			/* Host line prompt */
int xmits = 0;			/* Use shift-in/shift-out, 0 = no */
int xmitw = 0;			/* Milliseconds to pause during TRANSMIT */
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
#else
extern char cmdbuf[];			/* Command buffers */
extern char atmbuf[];
extern char line[];			/* Character buffer for anything */
extern char tmpbuf[];			/* Temporary buffer */
extern int ifcmd[];
extern int intime[];
#endif /* DCMDBUF */

char *lp;				/* Pointer to line buffer */

#ifndef NOSPL
char evalbuf[33];			/* EVALUATE result */
extern char * inpbuf;			/* Buffer for INPUT and REINPUT */
char *inpbp;				/* And pointer to same */
extern char lblbuf[];			/* Buffer for labels */
int m_found;				/* MINPUT result */
int i_active = 0;			/* INPUT command is active */
char *ms[MINPMAX];			/* Pointers to MINPUT strings */
#endif /* NOSPL */

char psave[PROMPTL] = { NUL };		/* For saving & restoring prompt */

extern int success;			/* Command success/failure flag */

#ifndef NOSPL
int					/* SET INPUT parameters. */
/* Note, INPUT TIMEOUT, intime[], is on the command-level stack. */
  inbufsize = 0,			/* INPUT buffer size */
  indef = 1,				/* default timeout, seconds */
  inecho = 1,				/* 1 = echo on */
  insilence = 0;			/* 0 = no silence constraint */

int maclvl = -1;			/* Macro nesting level */
int mecho = 0;				/* Macro echo, 0 = don't */
char varnam[6];				/* For variable names */
extern int macargc[];			/* ARGC from macro invocation */

extern char *m_arg[MACLEVEL][NARGS];	/* Stack of macro arguments */

extern char **a_ptr[];			/* Array pointers */
extern int a_dim[];			/* Array dimensions */

#ifdef DCMDBUF
extern struct cmdptr *cmdstk;		/* The command stack itself */
#else
extern struct cmdptr cmdstk[];		/* The command stack itself */
#endif /* DCMDBUF */
extern int cmdlvl;			/* Current position in command stack */

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
#endif /* NOPUSH */
    "#",    	   XXCOM, CM_INV,	/* Comment */
#ifndef NOSPL
    ":",           XXLBL, CM_INV,	/* Label */
#endif /* NOSPL */
#ifndef NOPUSH
#ifdef CK_REDIR
    "<",           XXFUN, CM_INV,	/* REDIRECT */
#endif /* CK_REDIR */
    "@",           XXSHE, CM_INV,	/* DCL escape */
#endif /* NOPUSH */
    "about",       XXVER, CM_INV,	/* Synonym for VERSION */
#ifndef NOSPL
#ifndef NOMSEND
    "add",         XXADD, 0,		/* ADD */
#endif /* NOMSEND */
#ifndef NODIAL
    "answer",      XXANSW, 0,		/* ANSWER the phone */
#endif /* NODIAL */
    "apc",         XXAPC, 0,		/* Application Program Command */
    "ascii",       XXASC, CM_INV,
    "asg",         XXASS, CM_INV,       /* Invisible synonym for ASSIGN */
    "ask",         XXASK, 0,		/* ASK for text, assign to variable */
    "askq",        XXASKQ,0,            /* ASK quietly (no echo) */
    "assign",      XXASS, 0,            /* ASSIGN value to variable or macro */
#endif /* NOSPL */
#ifndef NOSPL
    "beep",        XXBEEP,CM_INV,	/* BEEP */
#endif /* NOSPL */
    "binary",      XXBIN, CM_INV,
#ifndef NOFRILLS
    "bug",         XXBUG, 0,		/* BUG report instructions */
#endif /* NOFRILLS */
    "bye",         XXBYE, 0,		/* BYE to remote server */
#ifndef NOLOCAL
    "c",           XXCON, CM_INV|CM_ABR, /* invisible synonym for CONNECT */
#endif /* NOLOCAL */
#ifndef NOFRILLS
    "cat",         XXTYP, CM_INV,	/* Invisible synonym for TYPE */
#endif /* NOFRILLS */
    "cd",          XXCWD, 0,		/* Change Directory */
    "check",       XXCHK, 0,		/* CHECK for a feature */
#ifndef NOFRILLS
    "clear",       XXCLE, 0,		/* CLEAR input and/or device buffer */
#endif /* NOFRILLS */
    "close",	   XXCLO, 0,		/* CLOSE a log or other file */
#ifdef OS2
    "cls",         XXCLS, CM_INV,	/* Clear Screen (CLS) */
#endif /* OS2 */
    "comment",     XXCOM, 0,		/* Introduce a comment */
#ifndef NOLOCAL
    "connect",     XXCON, 0,		/* Begin terminal connection */
#endif /* NOLOCAL */
#ifndef NOFRILLS
#ifdef ZCOPY
    "copy",        XXCPY, 0,		/* COPY a file */
#endif /* ZCOPY */
#endif /* NOFRILLS */
    "cwd",	   XXCWD, CM_INV,	/* Invisisble synonym for cd */
#ifndef NOSPL
    "date",        XXDATE,0,		/* DATE */
    "dcl",         XXDCL, CM_INV,	/* DECLARE an array */
    "declare",     XXDCL, 0,		/* DECLARE an array */
    "decrement",   XXDEC, 0,		/* DECREMENT a numeric variable */
    "define",      XXDEF, 0,		/* DEFINE a macro or variable */
#endif /* NOSPL */
#ifndef NOFRILLS
    "delete",      XXDEL, 0,		/* DELETE a file */
#endif /* NOFRILLS */
#ifndef NODIAL
    "dial",	   XXDIAL,0,		/* DIAL a phone number */
#endif /* NODIAL */
    "directory",   XXDIR, 0,		/* DIRECTORY of files */
#ifndef NOFRILLS
#ifndef NOSERVER
    "disable",     XXDIS, 0,		/* DISABLE a server function */
#endif /* NOSERVER */
#endif /* NOFRILLS */
#ifndef NOSPL
    "do",          XXDO,  0,		/* DO (execute) a macro */
#endif /* NOSPL */
    "e",           XXEXI, CM_INV|CM_ABR,
#ifndef NOFRILLS
    "e-packet",    XXERR, CM_INV,	/* Send an Error-Packet */
#endif /* NOFRILLS */
    "echo",        XXECH, 0,		/* ECHO text */
    "eightbit",    XXEIGHT, 0,		/* EIGHTBIT */
#ifndef NOSPL
    "else",        XXELS, CM_INV,	/* ELSE part of IF statement */
#endif /* NOSPL */
#ifndef NOSERVER
#ifndef NOFRILLS
    "enable",      XXENA, 0,		/* ENABLE a server function */
#endif /* NOFRILLS */
#endif /* NOSERVER */
#ifndef NOSPL
    "end",         XXEND, 0,		/* END command file or macro */
    "evaluate",    XXEVAL, 0,		/* EVALUATE */
#endif /* NOSPL */
    "ex",          XXEXI, CM_INV|CM_ABR, /* Let "ex" still be EXIT */
    "exit",	   XXEXI, 0,		 /* EXIT from C-Kermit */
#ifdef OS2
    "extproc",     XXCOM, CM_INV,        /* Dummy command for OS/2 */
#endif /* OS2 */
    "f",           XXFIN, CM_INV|CM_ABR, /* Invisible abbreviation for... */
    "finish",      XXFIN, 0,		 /* FINISH */
#ifndef NOSPL
    "fo",          XXFOR, CM_INV|CM_ABR, /* Invisible abbreviation for... */
    "for",         XXFOR, 0,		/* FOR loop */
    "forward",     XXFWD, CM_INV,	/* FORWARD (ugh) */
#endif /* NOSPL */
#ifndef NOFRILLS
    "fot",	   XXDIR, CM_INV,	/* "fot" = "dir" (for Chris) */
#endif /* NOFRILLS */
#ifdef TCPSOCKET
    "ftp",	   XXFTP, 0,		/* FTP (for TCP/IP) */
#endif /* TCPSOCKET */
    "g",           XXGET, CM_INV|CM_ABR, /* Invisible abbreviation for GET */
#ifndef NOSPL
    "ge",          XXGET, CM_INV|CM_ABR, /* Ditto */
#endif /* NOSPL */
    "get",         XXGET, 0,		/* GET */
#ifndef NOSPL
    "getc",        XXGETC, 0,		/* GETC */
#ifndef NOFRILLS
    "getok",       XXGOK, 0,		/* GETOK (ask for Yes/No) */
#endif /* NOFRILLS */
#endif /* NOSPL */
#ifndef NOSPL
    "goto",        XXGOTO,0,		/* GOTO label in take file or macro */
#endif /* NOSPL */
    "h",           XXHLP, CM_INV|CM_ABR, /* Invisible synonym for HELP */
    "hangup",      XXHAN, 0,		 /* HANGUP the connection */
    "help",	   XXHLP, 0,		 /* Display HELP text */
#ifndef NOSPL
    "i",           XXINP, CM_INV|CM_ABR, /* Invisible synonym for INPUT */
    "if",          XXIF,  0,		 /* IF (condition) command */
    "in",          XXINP, CM_INV|CM_ABR, /* Invisible synonym for INPUT */
    "increment",   XXINC, 0,		 /* Increment a numeric variable */
    "input",       XXINP, 0,		 /* INPUT text from comm device */
#endif /* NOSPL */
#ifndef NOHELP
     "introduction", XXINT, 0,		/* Print introductory text */
#endif /* NOHELP */
    "kermit",      XXKERMI, 0,		/* Hmmm what's this... */
#ifndef NOFRILLS
    "l",           XXLOG, CM_INV|CM_ABR, /* Invisible synonym for log */
#endif /* NOFRILLS */
#ifndef NOSPL
    "local",       XXLOCAL, 0,		/* LOCAL variable declaration */
#endif /* NOSPL */
    "log",  	   XXLOG, 0,		/* Open a log file */
#ifndef NOFRILLS
#ifndef NODIAL
    "lookup",      XXLOOK,0,		/* LOOKUP */
#endif /* NODIAL */
    "ls",          XXDIR, CM_INV,	/* Invisible synonym for DIRECTORY */
    "mail",        XXMAI, 0,		/* Send a file as e-mail */
    "man",         XXHLP, CM_INV,       /* Synonym for HELP */
#endif /* NOFRILLS */
#ifdef CK_MKDIR
    "md",          XXMKDIR, CM_INV,	/* Synonym for MKDIR */
#endif /* CK_MKDIR */
#ifdef CK_MINPUT
    "minput",      XXMINP, 0,		/* MINPUT */
#endif /* CK_MINPUT */
#ifndef NOMSEND
    "mget",        XXGET, CM_INV,	/* MGET = GET */
#endif /* NOMSEND */
#ifdef CK_MKDIR
    "mkdir",       XXMKDIR, 0,		/* MKDIR */
#endif /* CK_MKDIR */
#ifndef NOMSEND
    "mmove",       XXMMOVE, 0,		/* MMOVE */
#endif /* NOMSEND */
    "move",        XXMOVE, 0,		/* MOVE  */
#ifndef NOSPL
    "mpause",      XXMSL, CM_INV,	/* Millisecond sleep */
#endif /* NOSPL */
#ifndef NOMSEND
    "mput",        XXMSE, CM_INV,	/* MPUT = MSEND */
#endif /* NOMSEND */
#ifndef NOMSEND
    "ms",          XXMSE, CM_INV|CM_ABR,
    "msend",       XXMSE, 0,		/* Multiple SEND */
#endif /* NOMSEND */
#ifndef NOSPL
    "msleep",      XXMSL, 0,		/* Millisecond sleep */
#endif /* NOSPL */
#ifndef NOFRILLS
    "mv",          XXREN, CM_INV,	/* Synonym for rename */
#endif /* NOFRILLS */
    "news",        XXNEW, 0,		/* Display NEWS of new features */
    "nopush",      XXNPSH, CM_INV,	/* Disable PUSH command/features */
#ifndef NOSPL
    "o",           XXOUT, CM_INV|CM_ABR, /* Invisible synonym for OUTPUT */
    "open",        XXOPE, 0,		/* OPEN file for reading or writing */
    "output",      XXOUT, 0,		/* OUTPUT text to comm device */
#endif /* NOSPL */
#ifdef ANYX25
    "pad",         XXPAD, 0,            /* X.3 PAD commands */
#endif /* ANYX25 */
#ifndef NOSPL
    "pause",       XXPAU, 0,		/* Sleep for specified interval */
#endif /* NOSPL */
#ifndef NODIAL
    "pdial",       XXPDIA,0,		/* PDIAL (partial dial) */
#endif /* NODIAL */
#ifdef TCPSOCKET
    "ping",        XXPNG, 0,		/* PING (for TCP/IP) */
#endif /* TCPSOCKET */
#ifndef NOSPL
    "pop",         XXEND, CM_INV,	/* Invisible synonym for END */
#endif /* NOSPL */
#ifndef NOFRILLS
    "print",       XXPRI, 0,		/* PRINT a file locally */
#ifndef NOPUSH
#ifdef CK_RESEND
    "psend",       XXPSEN, 0,		/* PSEND */
#endif /* CK_RESEND */
    "pu",          XXSHE, CM_INV,	/* PU = PUSH */
    "push",        XXSHE, 0,		/* PUSH command (like RUN, !) */
#endif /* NOPUSH */
    "put",	   XXSEN, CM_INV,	/* PUT = SEND */
    "pwd",         XXPWD, 0,            /* Print Working Directory */
#endif /* NOFRILLS */
    "quit",	   XXQUI, 0,		/* QUIT from program = EXIT */
    "r",           XXREC, CM_INV|CM_ABR, /* Invisible synonym for RECEIVE */
#ifdef CK_MKDIR
    "rd",          XXRMDIR, CM_INV,     /* RMDIR */
#endif /* CK_MKDIR */
#ifndef NOSPL
    "read",        XXREA, 0,            /* READ a line from a file */
#ifdef BINREAD
    "readblock",   XXRDBL, 0,		/* READ a block */
#endif /* BINREAD */
#endif /* NOSPL */
    "receive",	   XXREC, 0,		/* RECEIVE files */
#ifndef NODIAL
#ifdef CK_REDIR
    "red",         XXRED, CM_INV|CM_ABR, /* Invisible synonym for REDIAL */
    "redi",        XXRED, CM_INV|CM_ABR, /* Invisible synonym for REDIAL */
#endif /* CK_REDIR */
    "redial",      XXRED, 0,		/* REDIAL last DIAL number */
#endif /* NODIAL */
#ifndef NOPUSH
#ifdef CK_REDIR
#ifdef OS2
    "redirect",    XXFUN, CM_INV,	/* REDIRECT local command to ttyfd */
#else /* OS2 */
    "redirect",    XXFUN, 0,		/* REDIRECT local command to ttyfd */
#endif /* OS2 */
#endif /* CK_REDIR */
#endif /* NOPUSH */
#ifdef CK_RESEND
    "reget",       XXREGET, 0,		/* REGET */
#endif /* CK_RESEND */
#ifndef NOSPL
    "reinput",     XXREI, 0,            /* REINPUT (from INPUT buffer) */
#endif /* NOSPL */
    "remote",	   XXREM, 0,		/* Send generic command to server */
#ifndef NOFRILLS
    "rename",      XXREN, 0,		/* RENAME a local file */
    "replay",      XXTYP, CM_INV,	/* REPLAY (for now, just type) */
#endif /* NOFRILLS */
#ifdef CK_RESEND
    "resend",      XXRSEN, 0,		/* RESEND */
#ifndef NOSPL
    "ret",         XXRET, CM_INV|CM_ABR,
#endif /* NOSPL */
#endif /* CK_RESEND */
    "retrieve",    XXRETR, 0,           /* RETRIEVE */
#ifndef NOSPL
    "return",      XXRET, 0,		/* RETURN from a function */
#endif /* NOSPL */
#ifndef NOPUSH
#ifdef CK_REXX
    "rexx",       XXREXX, 0,		/* Execute a Rexx command */
#endif /* CK_REXX */
#endif /* NOPUSH */
#ifdef TCPSOCKET
#ifdef RLOGCODE
    "rlogin",    XXRLOG, 0,		/* Rlogin to host */
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
#ifndef NOFRILLS
    "rm",          XXDEL, CM_INV,	/* Invisible synonym for delete */
#endif /* NOFRILLS */
#ifdef CK_MKDIR
    "rmdir",       XXRMDIR, 0,          /* RMDIR */
#endif /* CK_MKDIR */
#ifndef NOPUSH
    "run",         XXSHE, 0,		/* RUN a program or command */
#endif /* NOPUSH */
    "s",           XXSEN, CM_INV|CM_ABR, /* Invisible synonym for send */
    "save",	   XXSAVE, 0,		/* SAVE parameters */
#ifndef NOSCRIPT
    "script",	   XXLOGI,0,		/* Execute a UUCP-style script */
#endif /* NOSCRIPT */
    "send",	   XXSEN, 0,		/* Send (a) file(s) */
#ifndef NOSERVER
    "server",	   XXSER, 0,		/* Be a SERVER */
#endif /* NOSERVER */
    "set",	   XXSET, 0,		/* SET parameters */
#ifndef NOSHOW
    "show", 	   XXSHO, 0,		/* SHOW parameters */
#endif /* NOSHOW */
#ifndef NOSPL
#ifndef NOFRILLS
    "sleep",       XXPAU, CM_INV,	/* SLEEP for specified interval */
#endif /* NOFRILLS */
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
#endif /* NOPUSH */
#endif /* NOFRILLS */
    "statistics",  XXSTA, 0,		/* Display file transfer stats */
#ifndef NOSPL
    "stop",        XXSTO, 0,		/* STOP all take files and macros */
#endif /* NOSPL */
#ifndef NOJC
    "suspend",     XXSUS, 0,		/* SUSPEND C-Kermit (UNIX only) */
#endif /* NOJC */
#ifndef NOSPL
    "switch",      XXSWIT, 0,		/* SWITCH */
#endif /* NOSPL */
    "take",	   XXTAK, 0,		/* TAKE commands from a file */
#ifndef NOFRILLS
#ifdef TCPSOCKET
    "tel",         XXTEL, CM_INV|CM_ABR,
    "telnet",      XXTEL, 0,		/* TELNET (TCP/IP only) */
    "telopt",      XXTELOP, CM_INV,     /* TELOPT (ditto) */
#endif /* TCPSOCKET */
#ifdef DEBUG
    "test",        XXTES, CM_INV,	/* (for testing) */
#endif /* DEBUG */
#endif /* NOFRILLS */
    "text",        XXASC, CM_INV,
#ifndef NOCSETS
    "translate",   XXXLA, 0,		/* TRANSLATE local file char sets */
#endif
#ifndef NOXMIT
    "transmit",    XXTRA, 0,		/* Send (upload) a file, no protocol */
#endif /* NOXMIT */
#ifndef NOFRILLS
    "type",        XXTYP, 0,		/* Display a local file */
#endif /* NOFRILLS */
#ifndef NOSPL
    "undefine",    XXUNDEF, 0,		/* UNDEFINE a variable or macro */
#endif /* NOSPL */
#ifdef OS2ONLY
    "updates",     XXUPD, 0,		/* View UPDATES file */
#endif /* OS2ONLY */
    "version",     XXVER, 0,		/* VERSION-number display */
#ifdef OS2
    "viewonly",    XXVIEW, 0,		/* VIEWONLY Terminal Mode */
#endif /* OS2 */
#ifndef NOSPL
    "wait",        XXWAI, 0,		/* WAIT (like pause) */
    "while",       XXWHI, 0,		/* WHILE loop */
#endif /* NOSPL */
#ifndef OS2
#ifndef MAC
#ifndef NOFRILLS
    "who",         XXWHO, 0,		/* WHO's logged in? */
#endif /* NOFRILLS */
#endif /* MAC */
#endif /* OS2 */
#ifndef NOSPL
    "wr",          XXWRI, CM_INV|CM_ABR,
    "wri",         XXWRI, CM_INV|CM_ABR,
    "writ",        XXWRI, CM_INV|CM_ABR,
    "write",       XXWRI, 0,		/* WRITE characters to a file */
    "write-line",  XXWRL, 0,		/* WRITE a line to a file */
#ifdef BINREAD
    "writeblock",  XXWRBL, 0,		/* WRITE a block */
#endif /* BINREAD */
    "writeln",     XXWRL, CM_INV,	/* Pascalisch synonym for write-line */
#endif /* NOSPL */
    "xecho",       XXXECH,0,		/* XECHO */
#ifndef NOSPL
    "xif",         XXIFX, 0,		/* Extended IF command */
#endif /* NOSPL */
#ifndef NOCSETS
    "xlate",       XXXLA, CM_INV,	/* Synonym for TRANSLATE */
#endif /* NOCSETS */
#ifndef NOXMIT
    "xmit",        XXTRA, CM_INV,	/* Synonym for TRANSMIT */
#endif /* NOXMIT */
#ifndef OS2
    "z",           XXSUS, CM_INV,	/* Synonym for SUSPEND */
#endif /* OS2 */
#ifndef NOSPL
    "_asg",        XXASX, CM_INV,	/* Used internally by FOR, etc */
    "_assign",     XXASX, CM_INV,	/* Used internally by FOR, etc */
    "_define",     XXDFX, CM_INV,	/* Used internally by FOR, etc */
    "_forward",    XXXFWD,CM_INV,	/* Used internally by SWITCH   */
    "_getargs",    XXGTA, CM_INV,       /* Used internally by FOR, etc */
    "_putargs",    XXPTA, CM_INV,       /* Used internally by FOR, etc */
#endif /* NOSPL */
"", 0, 0
};
int ncmd = (sizeof(cmdtab) / sizeof(struct keytab)) - 1;

char toktab[] = {
#ifndef NOPUSH
    '!',				/* Shell escape */
#endif /* NOPUSH */
    '#',				/* Comment */
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
    '\0'				/* End of this string */
};

struct keytab yesno[] = {		/* Yes/No keyword table */
    "no",    0, 0,
    "ok",    1, 0,
    "yes",   1, 0
};
int nyesno = (sizeof(yesno) / sizeof(struct keytab));

/* Save keyword table */

struct keytab savtab[] = {
#ifndef NOSETKEY
    "keymap", XSKEY, 0
#else
	"", 	0, 	0
#endif	/* NOSETKEY */
};
int nsav = (sizeof(savtab) / sizeof(struct keytab));

/* Parameter keyword table */

struct keytab prmtab[] = {
    "alarm",            XYALRM,  0,
    "attributes",       XYATTR,  0,
    "b",		XYBACK,  CM_INV|CM_ABR,
    "ba",		XYBACK,  CM_INV|CM_ABR,
    "background",       XYBACK,  0,
#ifndef NOLOCAL
    "baud",	        XYSPEE,  CM_INV,
#endif /* NOLOCAL */
#ifdef OS2
    "bell",             XYBELL,  0,
#endif /* OS2 */
    "block-check",  	XYCHKT,  0,
#ifdef DYNAMIC
    "buffers",          XYBUF,   0,
#endif /* DYNAMIC */
#ifndef NOLOCAL
#ifndef MAC
    "carrier-watch",    XYCARR,  0,
#endif /* MAC */
#endif /* NOLOCAL */
#ifndef NOSPL
    "case",             XYCASE,  0,
#endif /* NOSPL */
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
    "d",		XYDELA,  CM_INV|CM_ABR,
    "de",		XYDELA,  CM_INV|CM_ABR,
    "debug",            XYDEBU,  CM_INV,
#ifdef VMS
    "default",          XYDFLT,  0,
#else
#ifndef MAC
    "default",          XYDFLT,  CM_INV,
#endif /* MAC */
#endif /* VMS */
    "delay",	    	XYDELA,  0,
    "destination",	XYDEST,  0,
#ifndef NODIAL
    "dial",             XYDIAL,  0,
#endif /* NODIAL */
#ifdef OS2
    "dialer",		XYDLR,   CM_INV,
#endif /* OS2 */
#ifndef NOLOCAL
    "duplex",	    	XYDUPL,  0,
    "escape-character", XYESC,   0,
#endif /* NOLOCAL */
    "exit",		XYEXIT,  0,
    "file", 	  	XYFILE,  0,
    "flow-control", 	XYFLOW,  0,
    "handshake",    	XYHAND,  0,
#ifdef NETCONN
    "host",             XYHOST,  0,
#endif /* NETCONN */
#ifndef NOSPL
    "i",		XYINPU,  CM_INV|CM_ABR,
    "in",		XYINPU,  CM_INV|CM_ABR,
#endif /* NOSPL */
    "incomplete",   	XYIFD,   CM_INV,
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
    "line",             XYLINE,  0,
#ifndef NOLOCAL
    "local-echo",	XYLCLE,  CM_INV,
#endif /* NOLOCAL */
    "login",		XYLOGIN, 0,
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
#ifdef ANYX25
    "pad",              XYPAD,   0,
#endif /* ANYX25 */
    "parity",	    	XYPARI,   0,
#ifdef OS2
    "port",             XYLINE,   0,
#else
    "port",             XYLINE,   CM_INV,
#endif /* OS2 */
    "pr",   	    	XYPROM,  CM_INV|CM_ABR,
    "printer",          XYPRTR,  0,
#ifdef OS2
    "priority",         XYPRTY,  0,
#endif /* OS2 */
#ifdef CK_SPEED
    "prefixing",        XYPREFIX, 0,
#endif /* CK_SPEED */
#ifndef NOFRILLS
    "prompt",	    	XYPROM,  0,
#endif /* NOFRILLS */
    "protocol",		XYPROTO, 0,
    "quiet",		XYQUIE,  0,
    "receive",          XYRECV,  0,
    "repeat",           XYREPT,  0,
    "retry-limit",      XYRETR,  0,
#ifndef NOSCRIPT
    "script",		XYSCRI,  0,
#endif /* NOSCRIPT */
    "send",             XYSEND,  0,
#ifndef NOSERVER
    "server",           XYSERV,  0,
#endif /* NOSERVER */

#ifndef NOLOCAL
#ifdef UNIX
    "session-log",      XYSESS,  0,
#else
#ifdef OSK
    "session-log",      XYSESS,  0,
#endif /* OSK */
#endif /* UNIX */
#endif /* NOLOCAL */

#ifndef NOLOCAL
    "speed",	        XYSPEE,  0,
#endif /* NOLOCAL */

#ifndef NOSPL
    "startup-file",     XYSTARTUP, CM_INV,
#endif /* NOSPL */

#ifndef NOJC
    "suspend",          XYSUSP,  0,
#endif /* NOJC */
    "take",             XYTAKE,  0,
#ifdef CK_TAPI
   "tapi",              XYTAPI,  0,
#endif /* CK_TAPI */
#ifndef NOTCPOPTS
#ifdef TCPSOCKET
#ifdef SOL_SOCKET
   "tcp",               XYTCP, 0,
#endif /* SOL_SOCKET */
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */
#ifdef TNCODE
    "telnet",           XYTEL,   0,
#endif /* TNCODE */
    "temp-directory",   XYTMPDIR,0,
#ifndef NOLOCAL
    "terminal",         XYTERM,  0,
#endif /* NOLOCAL */
#ifdef OS2
    "title",		XYTITLE, 0,
#endif /* OS2 */
    "transfer",         XYXFER,  0,
#ifndef NOXMIT
    "transmit",         XYXMIT,  0,
#endif /* NOXMIT */
#ifndef NOCSETS
    "unknown-char-set", XYUNCS,  0,
#endif /* NOCSETS */
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
    "win95",            XYWIN95, CM_INV,
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

/* Table of networks */
#ifdef NETCONN
struct keytab netcmd[] = {
/*
  This is for SET NETWORK { DIRECTORY, TYPE }.
  The old form, SET NETWORK <name-of-network> is retained for compatibility,
  but made invisible.  See netkey[], just below.  NOTE that the values for the
  keywords in both tables should be consistent, and that the network-thing
  keyword (DIRECTORY, TYPE) values must be distinct from the network-type
  keywords values.  (This is exactly the same situation we have with SET MODEM
  vs SET MODEM TYPE...)
*/
#ifdef DECNET				/* DECnet / PATHWORKS */
    "decnet",        NET_DEC,  CM_INV,
#endif /* DECNET */

    "directory",     XYNET_D,  0,	/* DIRECTORY (no more yuk) */

#ifdef NETFILE
    "file",           NET_FILE, CM_INV,  /* FILE (real crude) */
#endif /* NETFILE */

#ifdef NPIPE				/* Named Pipes */
    "named-pipe",    NET_PIPE, CM_INV,
#endif /* NPIPE */

#ifdef CK_NETBIOS
    "netbios",       NET_BIOS, CM_INV,	/* NETBIOS */
#endif /* CK_NETBIOS */

#ifdef SUPERLAT
   "superlat",       NET_SLAT, CM_INV,  /* Meridian Technologies' SuperLAT */
#endif /* SUPERLAT */

#ifdef TCPSOCKET			/* TCP/IP sockets library */
    "tcp/ip",       NET_TCPB, CM_INV,
#endif /* TCPSOCKET */
#ifdef SUPERLAT
    "tes32",        NET_SLAT, CM_INV,  /* Emulux TES32 */
#endif /* SUPERLAT */
    "type",         XYNET_T,  0,
#ifdef ANYX25				/* X.25 */
#ifdef SUNX25
    "x",            NET_SX25, CM_INV|CM_ABR,
    "x.25",         NET_SX25, CM_INV,
    "x25",          NET_SX25, CM_INV,
#else
#ifdef STRATUSX25
    "x",            NET_VX25, CM_INV|CM_ABR,
    "x.25",         NET_VX25, CM_INV,
    "x25",          NET_VX25, CM_INV,
#endif /* STRATUSX25 */
#endif /* SUNX25 */
#endif /* ANYX25 */
    "", 0, 0
};
int nnets = (sizeof(netcmd) / sizeof(struct keytab)) - 1;

struct keytab netkey[] = {
/*
  These are the network types.
*/
#ifdef DECNET				/* DECnet / PATHWORKS */
    "decnet",        NET_DEC,  0,
#endif /* DECNET */

#ifdef NETFILE
    "file",           NET_FILE, CM_INV,  /* FILE (real crude) */
#endif /* NETFILE */

#ifdef NPIPE				/* Named Pipes */
    "named-pipe",     NET_PIPE,  0,
#endif /* NPIPE */

#ifdef CK_NETBIOS
    "netbios",        NET_BIOS,  0,	/* NETBIOS */
#endif /* CK_NETBIOS */

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
#endif /* ANYX25 */
    "", 0, 0
};
int nnetkey = (sizeof(netkey) / sizeof(struct keytab)) - 1;

#ifndef NOTCPOPTS
#ifdef TCPSOCKET
#ifdef SOL_SOCKET

/* TCP options */

struct keytab tcpopt[] = {
#ifdef SO_KEEPALIVE
   "keepalive", XYTCP_KEEPALIVE, 0,
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
   "linger", XYTCP_LINGER, 0,
#endif  /* SO_LINGER */
#ifdef TCP_NODELAY
   "nagle",  XYTCP_NODELAY, CM_INV,
   "nodelay", XYTCP_NODELAY, 0,
#endif /* TCP_NODELAY */
#ifdef SO_RCVBUF
   "recvbuf", XYTCP_RECVBUF, 0,
#endif /* SO_RCVBUF */
#ifdef SO_SNDBUF
   "sendbuf", XYTCP_SENDBUF, 0,
#endif /* SO_SNDBUF */
   "",0,0
};
int ntcpopt = (sizeof(tcpopt) / sizeof(struct keytab));
#endif /* SOL_SOCKET */
#endif /* TCPSOCKET */
#endif /* NOTCPOPTS */

#endif /* NETCONN */

/* Remote Command Table */

struct keytab remcmd[] = {
#ifndef NOSPL
    "as",	 XZASG, CM_INV|CM_ABR,
    "asg",	 XZASG, CM_INV,
    "assign",	 XZASG, 0,
#endif /* NOSPL */
    "cd",        XZCWD, 0,
    "copy",      XZCPY, 0,
    "cwd",       XZCWD, CM_INV,
    "delete",    XZDEL, 0,
    "directory", XZDIR, 0,
    "help",      XZHLP, 0,
#ifndef NOPUSH
    "host",      XZHOS, 0,
#endif /* NOPUSH */
#ifndef NOFRILLS
    "kermit",    XZKER, 0,
    "login",     XZLGI, 0,
    "logout",    XZLGO, 0,
    "print",     XZPRI, 0,
#endif /* NOFRILLS */
    "pwd",       XZPWD, 0,
#ifndef NOSPL
    "query",	 XZQUE, 0,
#endif /* NOSPL */
    "rename",    XZREN, 0,
    "set",       XZSET, 0,
    "space",	 XZSPA, 0
#ifndef NOFRILLS
,   "type", 	 XZTYP, 0,
    "who",  	 XZWHO, 0
#endif /* NOFRILLS */
};
int nrmt = (sizeof(remcmd) / sizeof(struct keytab));

struct keytab logtab[] = {
#ifdef DEBUG
    "debugging",    LOGD, 0,
#endif /* DEBUG */
    "packets",	    LOGP, 0
#ifndef NOLOCAL
,   "session",      LOGS, 0
#endif /* NOLOCAL */
#ifdef TLOG
,   "transactions", LOGT, 0
#endif /* TLOG */
};
int nlog = (sizeof(logtab) / sizeof(struct keytab));

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
    "transaction-log", LOGT, 0
#ifdef COMMENT
,   "transactions",    LOGT, CM_INV
#endif /* COMMENT */
};
int nwri = (sizeof(writab) / sizeof(struct keytab));

static struct keytab clrtab[] = {	/* Keywords for CLEAR command */
#ifndef NOSPL
#ifdef CK_APC
    "apc",              CLR_APC,         0,
#endif /* CK_APC */
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
    "scrollback",       CLR_SCL,         CM_INV,
    "terminal-screen",  CLR_TRM,         0,
#endif /* OS2 */
    "", 0, 0
};
int nclear = (sizeof(clrtab) / sizeof(struct keytab)) - 1;

struct keytab clstab[] = {		/* Keywords for CLOSE command */
#ifndef NOSPL
    "append-file",     LOGW, CM_INV,
#endif /* NOSPL */
#ifdef DEBUG
    "debug-log",       LOGD, 0,
#endif /* DEBUG */
    "packet-log",      LOGP, 0
#ifndef NOSPL
,   "read-file",       LOGR, 0
#endif /* NOSPL */
#ifndef NOLOCAL
,   "session-log",     LOGS, 0
#endif /* NOLOCAL */
#ifdef TLOG
,   "transaction-log", LOGT, 0
#endif /* TLOG */
#ifndef NOSPL
,   "write-file",      LOGW, 0
#endif /* NOSPL */
};
int ncls = (sizeof(clstab) / sizeof(struct keytab));

/* SHOW command arguments */

struct keytab shotab[] = {
#ifndef NOSPL
    "alarm", SHALRM, 0,
    "arg",  SHARG, CM_INV|CM_ABR,
    "arguments", SHARG, 0,
    "args", SHARG, CM_INV,
    "arrays", SHARR, 0,
#endif /* NOSPL */
    "attributes", SHATT, 0,
    "character-sets", SHCSE, 0,
    "cmd",  SHCMD, CM_INV,
    "com",  SHCOM, CM_INV|CM_ABR,
    "comm", SHCOM, CM_INV|CM_ABR,
    "communications", SHCOM, 0,
    "command", SHCMD, 0,
#ifdef CK_SPEED
    "control-prefixing", SHCTL, 0,
#endif /* CK_SPEED */
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
#ifndef NOLOCAL
    "escape", SHESC, 0,
#endif /* NOLOCAL */
    "exit", SHEXI, 0,
    "features", SHFEA, 0,
    "file", SHFIL, 0,
#ifndef NOSPL
    "functions", SHFUN, 0,
    "globals", SHVAR, 0,
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
#endif /* NETCONN */
#ifdef ANYX25
    "pad", SHPAD, 0,
#endif /* ANYX25 */
    "parameters", SHPAR, CM_INV,
    "printer",  SHPRT, 0,
#ifdef CK_SPEED
    "prefixing", SHCTL, CM_INV,
#endif /* CK_SPEED */
    "protocol", SHPRO, 0,
#ifndef NOSPL
    "scripts", SHSCR, 0,
#endif /* NOSPL */
    "send-list", SHSFL, 0,
#ifndef NOSERVER
    "server", SHSER, 0,
#endif /* NOSERVER */
    "status", SHSTA, 0
#ifdef MAC
,   "stack", SHSTK, 0			/* debugging */
#endif /* MAC */
#ifndef NOLOCAL
#ifdef OS2
,   "tabs",SHTAB, CM_INV
#endif /* OS2 */
,   "terminal", SHTER, 0
#endif /* NOLOCAL */
#ifndef NOXMIT
,   "transmit", SHXMI, 0
#endif /* NOXMIT */
#ifndef NOSETKEY
#ifndef NOKVERBS
#ifdef OS2
,    "udk", SHUDK, 0
#endif /* OS2 */
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#ifndef NOSPL
,   "variables", SHBUI, 0
#endif /* NOSPL */
#ifndef NOFRILLS
,   "versions", SHVER, 0
#endif /* NOFRILLS */
#ifdef OS2
,   "vscrn",    SHVSCRN, CM_INV
#endif /* OS2 */
#ifndef NOXMIT
,   "xmit", SHXMI, CM_INV
#endif /* NOXMIT */
};
int nsho = (sizeof(shotab) / sizeof(struct keytab));

#ifdef ANYX25
struct keytab padtab[] = {              /* PAD commands */
    "clear",      XYPADL, 0,
    "interrupt",  XYPADI, 0,
    "reset",      XYPADR, 0,
    "status",     XYPADS, 0
};
int npadc = (sizeof(padtab) / sizeof(struct keytab));
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
    "finish",     EN_FIN,  0,
    "get",        EN_GET,  0,
    "host",       EN_HOS,  0,
    "mail",       EN_MAI,  0,
#ifndef NOSPL
    "query",      EN_QUE,  0,
#endif /* NOSPL */
    "print",      EN_PRI,  0,
    "rename",     EN_REN,  0,
    "retrieve",   EN_RET,  0,
    "send",       EN_SEN,  0,
    "set",        EN_SET,  0,
    "space",      EN_SPA,  0,
    "type",       EN_TYP,  0,
    "who",        EN_WHO,  0
};
static int nena = (sizeof(enatab) / sizeof(struct keytab));
#endif /* NOSERVER */

#ifndef NOLOCAL
static struct keytab conntab[] = {	/* CONNECT options */
    "/quietly", 1, 0
};
#endif /* NOLOCAL */

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
struct keytab beeptab[] = {      /* Beep options */
    "error", BP_FAIL, 0,
    "information", BP_NOTE, 0,
    "warning", BP_WARN, 0
};
int nbeeptab = sizeof(beeptab)/sizeof(struct keytab);

#define CLR_C_ALL 0
#define CLR_C_BOL 1
#define CLR_C_BOS 2
#define CLR_C_EOL 3
#define CLR_C_EOS 4
#define CLR_C_LIN 5
#define CLR_C_SCR 6

struct keytab clrcmdtab[] = {    /* CLEAR COMMMAND-SCREEN options */
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

#ifndef NODIAL
static struct keytab looktab[] = {
    "dial", 0, 0
};
#endif /* NODIAL */

/* Forward declarations of functions */

_PROTOTYP (int doask,   ( int  ) );
_PROTOTYP (int dodef,   ( int  ) );
_PROTOTYP (int dodel,   ( void ) );
_PROTOTYP (int dodial,  ( int  ) );
_PROTOTYP (int dodir,   ( void ) );
_PROTOTYP (int doelse,  ( void ) );
_PROTOTYP (int dofor,   ( void ) );
_PROTOTYP (int dogta,   ( int  ) );
_PROTOTYP (int doincr,  ( int  ) );
_PROTOTYP (int dopaus,  ( int  ) );
_PROTOTYP (int doping,  ( void ) );
_PROTOTYP (int doftp,   ( void ) );
#ifndef NOFRILLS
_PROTOTYP (int dorenam, ( void ) );
#endif /* NOFRILLS */
#ifdef ZCOPY
_PROTOTYP (int docopy, ( void ) );
#endif /* ZCOPY */
#ifdef CK_REXX
_PROTOTYP (int dorexx,  ( void ) );
#endif /* CK_REXX */
#ifdef CK_REDIR
_PROTOTYP (int ttruncmd, ( char * ) );
#endif /* CK_REDIR */
_PROTOTYP (int dotype,   ( char * ) );

#ifdef TCPSOCKET
static struct keytab telcmd[] = {
   "do",   DO,   0,
   "dont", DONT, 0,
   "will", WILL, 0,
   "wont", WONT, 0
};

static struct keytab tnopts[] = {
    "binary", TELOPT_BINARY, 0,
    "echo", TELOPT_ECHO, 0,
#ifdef CK_NAWS
    "naws", TELOPT_NAWS, 0,
#endif /* CK_NAWS */
    "sga", TELOPT_SGA, 0,
    "ttype", TELOPT_TTYPE, 0,
    "", 0, 0
};
static int ntnopts = (sizeof(tnopts) / sizeof(struct keytab)) - 1;

int 
doftp() {
    char *p;
    int x;

    if (network)			/* If we have a current connection */
      strcpy(line,ttname);		/* get the host name */
    else *line = '\0';			/* as default host */
    for (p = line; *p; p++)		/* Remove ":service" from end. */
      if (*p == ':') { *p = '\0'; break; }
    if ((x = cmtxt("IP host name or number", line, &s, xxstring)) < 0)
      return(x);
/* Construct FTP command */
#ifdef VMS
#ifdef MULTINET				/* TGV MultiNet */
    sprintf(line,"multinet ftp %s",s);
#else
    sprintf(line,"ftp %s",s);		/* Other VMS TCP/IP's */
#endif /* MULTINET */
#else					/* Not VMS */
    sprintf(line,"ftp %s",s);
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
doping() {
    char *p;
    int x;

    if (network)			/* If we have a current connection */
      strcpy(line,ttname);		/* get the host name */
    else *line = '\0';			/* as default host to be pinged. */
    for (p = line; *p; p++)		/* Remove ":service" from end. */
      if (*p == ':') { *p = '\0'; break; }
    if ((x = cmtxt("IP host name or number", line, &s, xxstring)) < 0)
      return(x);
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
#endif /* TCPSOCKET */

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
/* ASK, ASKQ, READ */
    if (cx == XXASK || cx == XXASKQ || cx == XXREA ||
	cx == XXRDBL || cx == XXGETC) {
	return(doask(cx));
    }
#endif /* NOSPL */

#ifndef NOFRILLS
    if (cx == XXBUG) {			/* BUG */
	if ((x = cmcfm()) < 0) return(x);
	return(dobug());
    }
#endif /* NOFRILLS */

    if (cx == XXBYE) {			/* BYE */
	bye_active = 1;
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, BYE only works with Kermit protocol\n");
	    return(-9);
	}
#endif /* CK_XYZ */
	if ((x = cmcfm()) < 0) return(x);
	sstate = setgen('L',"","","");
	if (local) ttflui();		/* If local, flush tty input buffer */
	return(0);
    }

#ifndef NOSPL
    if ( cx == XXBEEP ) {        /* BEEP */
        int x;
#ifdef OS2
	int y;
        if ((y = cmkey( beeptab, nbeeptab, "which kind of beep", "information",
		       xxstring)) < 0 )
	  return (y);
    	if ((x = cmcfm()) < 0) return(x);
        bleep(y);			/* y is one of the BP_ values */
#else  /* OS2 */
    	if ((x = cmcfm()) < 0) return(x);
        bleep(BP_NOTE);
#endif /* OS2 */
        return(0);
    }
#endif /* NOSPL */

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
        if ( x == CLR_CMD || x == CLR_TRM ) {
            if ((z = cmkey(clrcmdtab,nclrcmd,"how much screen to clear\n",
			   "all",xxstring)) < 0)
	      return(z);
        }
#endif /* OS2 */
#endif /* NOSPL */
	if ((y = cmcfm()) < 0) return(y);

	/* Clear device input buffer if requested */
	y = (x & CLR_DEV) ? ttflui() : 0;
#ifndef NOSPL
	/* Clear INPUT command buffer if requested */
	if (x & CLR_INP) {
	    for (x = 0; x < inbufsize; x++)
	      inpbuf[x] = NUL;
	    inpbp = inpbuf;
	    y = 0;
	}
#ifdef CK_APC
	if (x & CLR_APC) {
	    apcactive = 0;
	    y = 0;
	}
#endif /* CK_APC */
#endif /* NOSPL */

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
    if (cx == XXCON) {			/* CONNECT */
	if ((x = cmkey(conntab,1,"Carriage return to confirm, or option",
		       "",xxstring)) < 0) {
	    if (x == -3)
	      return(success = doconect(0));
	    else return(x);
	}
	if ((y = cmcfm()) < 0)
	  return(y);
	return(success = doconect(x));
    }
#endif /* NOLOCAL */

#ifndef NOFRILLS
#ifdef ZCOPY
    if (cx == XXCPY) {			/* COPY a file */
#ifdef CK_APC
	if (apcactive == APC_LOCAL || 
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	return(docopy());
    }
#endif /* ZCOPY */
#endif /* NOFRILLS */

    if (cx == XXCWD)			/* CWD */
      return(success = docd());

    if (cx == XXCHK)			/* CHECK */
      return(success = dochk());

    if (cx == XXCLO) {			/* CLOSE */
	x = cmkey(clstab,ncls,"Which log or file to close","",xxstring);
	if (x == -3) {
	    printf("?You must say which file or log\n");
	    return(-9);
	}
	if (x < 0) return(x);
	if ((y = cmcfm()) < 0) return(y);
	y = doclslog(x);
	success = (y == 1);
	return(success);
    }

#ifndef NOSPL
    if (cx == XXDEC || cx == XXINC)	/* DECREMENT, INCREMENT */
      return(doincr(cx));

    if (cx == XXEVAL) {
	char *p;
	if ((x = cmtxt("Integer arithmetic expression","",&s,xxstring)) < 0)
	  return(x);
	p = evala(s);
	if (!p) p = "";
	if (*p)
	  printf("%s\n", p);
	strncpy(evalbuf,p,32);
	return(success = *p ? 1 : 0);
    }
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDEF || cx == XXASS ||
	cx == XXDFX || cx == XXASX || cx == XXUNDEF)
      return(dodef(cx));		/* DEFINE, ASSIGN, etc... */
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDCL) {			/* DECLARE an array */
	if ((y = cmfld("Array name","",&s,NULL)) < 0) {
	    if (y == -3) {
		printf("?Array name required\n");
		return(-9);
	    } else return(y);
	}
	if ((y = arraynam(s,&x,&z)) < 0) return(y);
	if ((y = cmcfm()) < 0) return(y);
	if (dclarray((char)x,z) < 0) {
	    printf("?Declare failed\n");
	    return(success = 0);
	}
	return(success = 1);
    }
#endif /* NOSPL */

#ifndef NODIAL
    if (cx == XXRED  || cx == XXDIAL || cx == XXPDIA ||
	cx == XXANSW || cx == XXLOOK) { /* DIAL, REDIAL etc */
	x = dodial(cx);
	debug(F101,"dodial returns","",x);
	if ((cx == XXDIAL || cx == XXRED) &&
	    (x > 0) &&			/* If DIAL or REDIAL succeeded */
	    (dialcon > 0)) {
	    if ( dialcon == 1 ||	/* And DIAL CONNECT is ON, */
		(dialcon == 2 &&	/* or DIAL CONNECT is AUTO */
#ifndef NOSPL				/* and we're at top level... */
		 cmdlvl == 0
#else
		 tlevel == -1		 
#endif /* NOSPL */
		 )) /* Or AUTO */
	      x = doconect(dialcq);	/* Then also CONNECT */
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
#ifdef CK_APC
	if (apcactive == APC_LOCAL || 
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	return(dodel());
    }
#endif /* NOFRILLS */

    if (cx == XXDIR)			/* DIRECTORY */
      return(dodir());

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
	return(doenable(y,x));
    }
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
    if (cx == XXRET) {			/* RETURN */
	if (cmdlvl == 0) {		/* At top level, nothing happens... */
	    if ((x = cmcfm()) < 0)
	      return(x);
	    return(success = 1);
	} else if (cmdstk[cmdlvl].src == CMD_TF) { /* In TAKE file, like POP */
	    if ((x = cmtxt("optional return value","",&s,NULL)) < 0)
	      return(x);		/* Allow trailing text, but ignore. */
	    if ((x = cmcfm()) < 0)
	      return(x);
	    popclvl();			/* pop command level */
	    return(success = 1);	/* always succeeds */
	} else if (cmdstk[cmdlvl].src == CMD_MD) { /* Within macro */
	    if ((x = cmtxt("optional return value","",&s,NULL)) < 0)
	      return(x);
	    return(doreturn(s));	/* Trailing text is return value. */
	} else return(-2);
    }
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXDO) {			/* DO (a macro) */
	if (nmac == 0) {
	    printf("\n?No macros defined\n");
	    return(-2);
	}
	for (y = 0; y < nmac; y++) {	/* copy the macro table */
	    mackey[y].kwd = mactab[y].kwd; /* into a regular keyword table */
	    mackey[y].kwval = y;	/* with value = pointer to macro tbl */
	    mackey[y].flgs = mactab[y].flgs;
	}
	/* parse name as keyword */
	if ((x = cmkey(mackey,nmac,"macro","",xxstring)) < 0) {
	    if (x == -3) {
		printf("?Macro name required\n");
		return(-9);
	    } else return(x);
	}
	if ((y = cmtxt("optional arguments","",&s,xxstring)) < 0)
	  return(y);			/* get args */
	return(dodo(x,s,cmdstk[cmdlvl].ccflgs) < 1 ? (success = 0) : 1);
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
	    printf("%s\n",s);
	} else if (cx == XXXECH) {	/* XECHO */
	    printf("%s",s);
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
		return(success = dooutput(tmpbuf));
#else
		printf("%c_%s%c\\",ESC,s,ESC);
#endif /* NOSPL */
	    }
	}
	return(1);			/* Always succeeds */
    }

#ifndef NOSPL
    if (cx == XXOPE)			/* OPEN */
      return(doopen());
#endif /* NOSPL */

#ifndef NOSPL
    if (cx == XXOUT) {			/* OUTPUT */
	int i;

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
	success = dooutput(s);
	return(success);
    }
#endif /* NOSPL */

#ifdef ANYX25
    if (cx == XXPAD) {			/* PAD commands */
	x = cmkey(padtab,npadc,"PAD command","",xxstring);
	if (x == -3) {
	    printf("?You must specify a PAD command to execute\n");
	    return(-2);
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
#endif /* ANYX25 */

#ifndef NOSPL
    if (cx == XXPAU || cx == XXWAI || cx == XXMSL) /* PAUSE, WAIT, etc */
      return(dopaus(cx));
#endif /* NOSPL */

#ifndef NOFRILLS
    if (cx == XXPRI) {
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
	strcpy(line,s);
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

#ifdef TCPSOCKET
    if (cx == XXPNG) 			/* PING an IP host */
      return(doping());

   if ( cx == XXFTP )			/* FTP an IP host */
      return(doftp());
#endif /* TCPSOCKET */

    if (cx == XXPWD) {			/* PWD */
#ifdef MAC
	char *pwp;
#else
#ifdef OS2
	char *pwp;
#endif
#endif /* MAC */
	if ((x = cmcfm()) < 0)
	  return(x);
#ifndef MAC
#ifndef OS2
	xsystem(PWDCMD);
	return(success = 1);		/* blind faith */
#else
	if (pwp = zgtdir()) {
	    printf("%s\n",pwp);
	    return(success = ((int)strlen(pwp) > 0));
	} else return(success = 0);
#endif
#else
	if (pwp = zgtdir()) {
	    printf("%s\n",pwp);
	    return(success = ((int)strlen(pwp) > 0));
	} else return(success = 0);
#endif /* MAC */
    }

#endif /* NOFRILLS */

    if (cx == XXQUI || cx == XXEXI) {	/* EXIT, QUIT */
	if ((y = cmnum("exit status code","",10,&x,xxstring)) < 0) {
	    if (y == -3)
	      x = xitsta;
	    else return(y);
	}
	if ((y = cmcfm()) < 0) return(y);

	if (!hupok(0))			/* Check if connection still open */
	  return(success = 0);
#ifdef COMMENT
#ifndef NODIAL
	mdmhup();
#endif /* NODIAL */
#endif /* COMMENT */

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
	sstate = setgen('F',"","","");
	if (local) ttflui();		/* If local, flush tty input buffer */
	return(0);
    }

#ifndef NOSPL
    if (cx == XXFOR)			/* FOR loop */
      return(dofor());
#endif /* NOSPL */

    /* GET MGET REGET RETRIEVE etc */
    if (cx == XXGET || cx == XXREGET || cx == XXRETR) {
#ifdef CK_XYZ
	if (protocol != PROTO_K) {
	    printf("Sorry, \"%s\" only works with Kermit protocol\n", atmbuf);
	    return(-9);
	}
#endif /* CK_XYZ */
	x = cmtxt("Name of remote file(s), or carriage return","",&cmarg,
		  xxstring);
#ifndef NOFRILLS
	if ((x == -2) || (x == -1)) return(x);
#else
	if (x < 0) return(x);
#endif /* NOFRILLS */
	cmarg = brstrip(cmarg);		/* Strip braces */
	x = doget(cx);
#ifdef MAC
	what = W_RECV;
	if (sstate == 'r')
	    scrcreate();
#endif /* MAC */
	return(x);
    }

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
	x = cmkey2(cmdtab,ncmd,"C-Kermit command","help",toktab,xxstring,1);
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
    if (cx == XXNEW)			/* NEWS */
      return(hmsga(newstxt));
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

    if (cx == XXHAN) {			/* HANGUP */
	if ((x = cmcfm()) < 0) return(x);
#ifndef NODIAL
	if ((x = mdmhup()) < 1)
#endif /* NODIAL */
	  x = (tthang() > -1);
#ifdef OS2
	if (x)
	  DialerSend(OPT_KERMIT_HANGUP, 0);
#endif /* OS2 */
	return(success = x);
    }

#ifndef NOSPL
    if (cx == XXGOTO || cx == XXFWD || cx == XXXFWD) { /* GOTO or FORWARD */
/* Note, here we don't set SUCCESS/FAILURE flag */
	if ((y = cmfld("label","",&s,xxstring)) < 0) {
	    if (y == -3) {
		printf("?Label name required\n");
		return(-9);
	    } else return(y);
	}
	strncpy(lblbuf,s,LBLSIZ);
	if ((x = cmcfm()) < 0) return(x);
	s = lblbuf;
	return(dogoto(s,cx));
    }
#endif /* NOSPL */

#ifndef NOSPL
/* IF, Extended IF, WHILE */
    if (cx == XXIF || cx == XXIFX || cx == XXWHI) {
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
	extern int x_ifnum;

	sprintf(tmpbuf,"%d",indef);
	zz = -1L;
	x_ifnum = 1;			/* Turn off internal complaints */
	y = cmnum("seconds to wait for input, or time of day hh:mm:ss",
		  (char *)tmpbuf, 10, &x, xxstring
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
	for (y = 0; y < MINPMAX; y++) {	/* Initialize strings */
	    if (ms[y]) {
		free(ms[y]);		/* Free old strings, if any */
		ms[y] = NULL;
	    }
	}
#ifdef CK_MINPUT
	if (cx == XXMINP) {		/* MINPUT */
	    int res = 0;
	    for (y = 0; y < MINPMAX; y++) { /* Parse up to MINPMAX strings */
                res = cmfld("List of input strings","",&s,xxstring);
                if (res < 0) return(res);
		debug(F110,"MINPUT cmfld returns",s,0);
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
                if (res == 1) break;
            }
	    for (y++; y < MINPMAX; y++) { /* Clean up old strings */
		if (ms[y]) {
		    free(ms[y]);	/* Free old strings, if any */
		    ms[y] = NULL;
		}
            }
#ifdef DEBUG
	    if (deblog) {		/* Check the parsing */
		for (y = 0; y < MINPMAX; y++)
		  if (ms[y]) debug(F111,"MINPUT",ms[y],y);
	    }
#endif /* DEBUG */
	
	} else
#endif /* CK_MINPUT */
	{
	    if ((y = cmtxt("Material to be input","",&s,xxstring)) < 0)
	      return(y);

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
	    success = doinput(x,ms);	/* Go try to input the search string */
	    i_active = 0;
	} else {			/* REINPUT */
	    debug(F110,"xxrei line",s,0);
	    success = doreinp(x,s);
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

    if (cx == XXREC) {			/* RECEIVE */
	cmarg2 = "";

#ifdef COMMENT /* wrong place for this -- see protocol module */
#ifdef CK_TMPDIR
	if (dldir && !f_tmpdir) {	/* If they have a download directory */
	    if (s = zgtdir()) {		/* Get current directory */
		if (zchdir(dldir)) {	/* Change to download directory */
		    strncpy(savdir,s,TMPDIRLEN);
		    f_tmpdir = 1;	/* Remember that we did this */
		}
	    }
	}
#endif /* CK_TMPDIR */
#endif /* COMMENT */

#ifdef CK_XYZ
	if (protocol == PROTO_X) {
	    x = cmofi("Name for incoming file", "", &s, xxstring);
	} else {
#endif /* CK_XYZ */
	    x = cmofi(
#ifdef CK_TMPDIR
"\nName under which to store the (first) incoming file, or:\n\
 name of directory in which to store all the file(s), or:\n\
 confirm the command now to store the files in the current\n\
 directory under their own names.",
#else
"Name under which to store the file, or confirm to accept\n\
 the file with its own name.",
#endif /* CK_TMPDIR */
		      "", &s, xxstring
		      );
#ifdef CK_XYZ
	}
#endif /* CK_XYZ */
	if ((x == -1) || (x == -2) || (x == -9)) {
#ifdef CK_TMPDIR
	    if (f_tmpdir) {
		zchdir(savdir);
		f_tmpdir = 0;
	    }
#endif /* CK_TMPDIR */
	    return(x);
	}
#ifdef CK_XYZ
	if (protocol == PROTO_X && x == -3) {
	    printf(
"Sorry, you must specify a name when receiving a file with XMODEM protocol\n");
#ifdef CK_TMPDIR
	    if (f_tmpdir) {
		zchdir(savdir);
		f_tmpdir = 0;
	    }
#endif /* CK_TMPDIR */
	    return(-9);
	}
#endif /* CK_XYZ */
	strcpy(line,s);
	if ((x = cmcfm()) < 0) {
#ifdef CK_TMPDIR
	    if (f_tmpdir) {
		zchdir(savdir);
		f_tmpdir = 0;
	    }
#endif /* CK_TMPDIR */
	    return(x);
	}
#ifdef CK_TMPDIR
/*
   User can give a device &/or directory specification here,
   rather than an alternative filename.
*/
	x = strlen(line);
	if (
#ifdef OS2
	    (isalpha(line[0]) &&
	     line[1] == ':' &&
	     line[2] == '\0') ||
	    isdir(line)
#else
#ifdef UNIX
	    (x > 0 && line[x-1] == '/') || isdir(line)
#else
#ifdef VMS
	    (x > 0) && isdir(line)
#else
/*
  Others -- Maybe this will work; if not, add another #ifdef..#endif.
  CK_TMPDIR should not be defined without an isdir() function.
*/
	    (x > 0) && isdir(line)
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
	    ) {
	    debug(F110,"DOWNLOAD arg disk or dir",line,0);
	    if (!f_tmpdir) {	/* If not already cd'd to download directory */
		s = zgtdir();		/* Get current directory */
		if (s) {		/* Save it */
		    strncpy(savdir,s,TMPDIRLEN);
		    f_tmpdir = 1;	/* Remember that we did this */
		    cmarg2 = "";	/* and we don't have an as-name. */
		} else {
		    printf("?Can't get current directory\n");
		    return(-9);
		}
	    }
	    if (!zchdir(line)) {	/* Change to given disk/directory, */
		printf("?Can't access %s\n",line);
		return(-9);
	    }
	} else				/* It's an alternative filename */
#endif /* CK_TMPDIR */
	  cmarg2 = line;

	debug(F111,"cmofi cmarg2",cmarg2,x);
	sstate = 'v';
#ifdef MAC
	what = W_RECV;
	scrcreate();
#endif /* MAC */
	if (local) displa = 1;
	return(0);
    }

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

#ifndef NOFRILLS
    if (cx == XXREN) {			/* RENAME */
#ifdef CK_APC
	if (apcactive == APC_LOCAL || 
	    apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	return(dorenam());
    }
#endif /* NOFRILLS */

    if (cx == XXEIGHT) {		/* EIGHTBIT */
	extern int parity, cmask, cmdmsk;
	if ((x = cmcfm()) < 0)
	  return(x);
	parity = 0;
	cmask = 0xff;
	cmdmsk = 0xff;
	return(success = 1);
    }

    if (cx == XXSEN  || cx == XXMAI 	/* SEND, MAIL */
	|| cx == XXMOVE			/* MOVE */
	|| cx == XXADD			/* ADD */
#ifdef CK_RESEND
	|| cx == XXRSEN || cx == XXPSEN /* RESEND, PSEND */
#endif /* CK_RESEND */
	) {

#ifdef CK_RESEND
	int seekto = 0;
	int filemode = XYFT_B;
#endif /* CK_RESEND */

#ifndef NOMSEND
	char * fmode = "";
	int xmode = 0;

	if (cx == XXADD) {
	    if ((x = cmkey(addtab,
			   naddtab,"Name of list","send-list",xxstring)) < 0)
	      return(x);
#ifndef XYZ_INTERNAL
	    if (protocol != PROTO_K) {
		printf(
		"?Sorry, ADD SEND-LIST does not work with external protocols\n"
		       );
		return(-9);
	    }
#endif /* XYZ_INTERNAL */
	}
#endif /* NOMSEND */

#ifdef CK_RESEND
	if (cx == XXRSEN && binary != XYFT_B
#ifdef VMS
	    && binary != XYFT_I
#endif /* VMS */
	    ) {
	    printf(
#ifdef VMS
		   "?Sorry, FILE TYPE must be BINARY\n"
#else
		   "?Sorry, FILE TYPE must be BINARY or IMAGE\n"
#endif /* VMS */
		   );
	    return(-9);
	}
#ifdef CK_XYZ
	if (cx == XXRSEN) {
	    if (protocol != PROTO_K && protocol != PROTO_Z) {
		printf(
"Sorry, RESEND is only possible with Kermit or ZMODEM protocol\n"
		       );
		return(-9);
		
	    }
	}
#endif /* CK_XYZ */
#endif /* CK_RESEND */
	if ((cx == XXMAI		/* MAIL */
#ifdef CK_RESEND
	     || cx == XXRSEN		/* RESEND */
#endif /* CK_RESEND */
	     ) &&
	    (!atdiso || !atcapr)) {	/* Disposition attribute off? */
	    printf("?Sorry, ATTRIBUTE DISPOSITION must be ON\n");
	    return(-9);
	}
	cmarg = cmarg2 = "";
#ifdef CK_XYZ
	if (protocol == PROTO_X)
	  x = cmifi("File to send","",&s,&y,xxstring);
	else
#endif /* CK_XYZ */
	  x = cmifi("File(s) to send","",&s,&y,xxstring);
	if (x < 0) {
	    if (x == -3) {
#ifndef NOMSEND
		if (filehead) {	/* SEND by itself with ADD list? */
		    nfils = filesinlist;
		    sndsrc = nfils;	/* Like MSEND */
		    addlist = 1;	/* But using a different list... */
		    filenext = filehead;
		    sstate = 's';
		    goto sendend;
		} else {		/* Oops, no list. */
		    printf("?No send list - use ADD to make one.\n");
		    return(-9);
		}
#else
		printf("?A file specification is required\n");
		return(-9);
#endif /* NOMSEND */
	    } else return(x);
	}
#ifdef CK_XYZ
	if (y != 0 && protocol == PROTO_X) {
	    printf(
"Sorry, you can only send one file at a time with XMODEM protocol\n"
		   );
	    return(-9);
	}
#endif /* CK_XYZ */

	nfils = -1;			/* Files come from internal list. */
#ifndef NOMSEND
        addlist = 0;			/* Don't use SEND-LIST. */
        filenext = NULL;
#endif /* NOMSEND */
	strcpy(line,s);			/* Save copy of string just parsed. */
#ifndef NOMSEND
	if (cx == XXADD) {
	    if (filesinlist == 0)	/* Take care of \v(filespec) */
	      fspec[0] = NUL;
	    if (((int)strlen(fspec) + (int)strlen(s) + 1) < CKMAXPATH) {
		strcat(fspec,s);
		strcat(fspec," ");
	    } else printf("WARNING - \\v(filespec) buffer overflow\n");
	} else
#endif /* NOMSEND */
	  strncpy(fspec,s,CKMAXPATH);

#ifdef CK_RESEND
	if (cx == XXPSEN) {		/* PSEND */
	    if (y != 0) {
		printf("?Sorry, wildcards not permitted in this command\n");
		return(-9);
	    }
	    if (sizeof(int) < 4) {
		printf("?Sorry, this command needs 32-bit integers\n");
		return(-9);
	    }
#ifdef CK_XYZ
	    if (protocol != PROTO_K) {
		printf("?Sorry, PSEND works only with Kermit protocol\n");
		return(-9);
	    }
#endif /* CK_XYZ */
	    x = cmnum("starting position (byte number)",
		      "",10,&seekto,xxstring);
	    if (x < 0)
	      return(x);
	}
#endif /* CK_RESEND */
#ifndef NOMSEND
	if (cx == XXADD) {
	    extern struct keytab fttab[];
	    extern int nfttyp;

	    fmode = gfmode(binary);
	    if ((x = cmkey(fttab,nfttyp,
		       "type of file transfer", fmode, xxstring)) < 0)
	      return(x);
	    xmode = x;
	}
#endif /* NOMSEND */
	if (cx == XXSEN			/* SEND command */
	    || cx == XXMOVE		/* MOVE command */
	    || cx == XXADD		/* ADD command */
#ifdef CK_RESEND
	    || cx == XXRSEN || cx == XXPSEN /* RESEND or PSEND command */
#endif /* CK_RESEND */
	    ) {
	    debug(F101,"Send: wild","",y);
	    if (y == 0) {
		if ((x = cmtxt("Name to send it with","",&cmarg2,
			       xxstring)) < 0)
		  return(x);
	    } else {
		if ((x = cmcfm()) < 0) return(x);
	    }
	    cmarg = line;		/* File to send */
	    debug(F110,"Sending:",cmarg,0);
	    if (*cmarg2 != '\0') debug(F110," as:",cmarg2,0);
	} else {			/* MAIL */
#ifndef NOFRILLS
#ifdef CK_XYZ
	    if (protocol != PROTO_K) {
		printf("Sorry, MAIL can be sent using Kermit protocol\n");
		return(-9);
	    }
#endif /* CK_XYZ */
	    if (!atdiso || !atcapr) {	/* Disposition attribute off? */
		printf("?Disposition Attribute is Off\n");
		return(-9);
	    }
	    debug(F101,"Mail: wild","",y);
	    *optbuf = NUL;		/* Wipe out any old options */
	    if ((x = cmtxt("Address to mail to","",&s,xxstring)) < 0)
	      return(x);
	    if ((int)strlen(s) == 0) {
		printf("?Address required\n");
		return(-9);
	    }
	    strcpy(optbuf,s);
	    if ((int)strlen(optbuf) > 94) { /* Ensure legal size */
		printf("?Option string too long\n");
		return(-9);
	    }
	    cmarg = line;		/* File to send */
	    debug(F110,"Mailing:",cmarg,0);
	    debug(F110,"To:",optbuf,0);
	    rmailf = 1;			/* MAIL modifier flag for SEND */
#else
	    printf("?Sorry, MAIL feature not configured.\n");
	    return(-9);
#endif /* NOFRILLS */
	}
#ifdef CK_RESEND
	if (cx == XXPSEN)		/* Partial-send starting position */
	  sendstart = seekto;
	else
	  sendstart = 0L;
#endif /* CK_RESEND */
	if (cx == XXMOVE)
	  moving = 1;
#ifndef NOMSEND
	if (cx != XXADD) {		/* Not ADD, really sending... */
#endif /* NOMSEND */
	    sstate = 's';		/* Set start state to SEND */
#ifndef NOMSEND
	    addlist = 0;
	    filenext = NULL;
	} else {			/* Just ADDing... */
	    struct filelist * flp;
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
		    x = (int) strlen(cmarg2); /* Length of as-name */
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
		    filesinlist++;	/* Count this node */
		    return(0);		/* Finished adding this node */
		} else {
		    printf("Sorry, can't allocate space for name");
		    return(-9);
		}
	    } else {
		printf("Sorry, can't allocate file list node");
		return(-9);
	    }
	}
#endif /* NOMSEND */
sendend:

#ifdef CK_RESEND
	switch (cx) {
	  case XXRSEN: sendmode = SM_RESEND; break;
	  case XXPSEN: sendmode = SM_PSEND;  break;
	  case XXSEN:
	  default:     sendmode = SM_SEND;   break;
	}
#else
	sendmode = SM_SEND;
#endif /* CK_RESEND */
#ifdef MAC
	what = W_SEND;
	scrcreate();
#endif /* MAC */
	if (local) {			/* If in local mode, */
	    displa = 1;			/* turn on file transfer display */
	}
	return(0);
    }

#ifndef NOMSEND
    if (cx == XXMSE || cx == XXMMOVE) {	/* MSEND and MMOVE commands */
	nfils = 0;			/* Like getting a list of */
	lp = line;			/* files on the command line */
	addlist = 0;			/* Do not use SEND-LIST */
	filenext = NULL;		/* Ditto ! */
#ifdef CK_XYZ
	if (protocol == PROTO_X) {
	    printf(
"Sorry, you can only send one file at a time with XMODEM protocol\n"
		   );
	    return(-9);
	}
#endif /* CK_XYZ */

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
	    if (((int)strlen(fspec) + (int)strlen(p) + 1) < CKMAXPATH) {
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
#endif /* NOMSEND */

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
    if (cx == XXSHE) {			/* SHELL (system) command */
        extern int nopush;
	if (cmtxt("System command to execute","",&s,xxstring) < 0)
	  return(-1);
       
        if (nopush)
          return(success = 0);
#ifdef CK_APC
	if (apcactive == APC_REMOTE && apcstatus != APC_UNCH)
	  return(success = 0);
#endif /* CK_APC */
	conres();			/* Make console normal  */
	x = zshcmd(s);
	debug(F101,"RUN zshcmd code","",x);
	concb((char)escape);
	return(success = x);
    }
#ifdef CK_REDIR
    if (cx == XXFUN) {			/* REDIRECT */
        extern int nopush;

        if (nopush) {
            printf("?REDIRECT disabled\n");
            return(success=0);
        }
#ifdef CK_APC
	if (apcactive == APC_LOCAL || 
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	if (!local) {
	    printf("?SET LINE or SET HOST required first\n");
	    return(-9);
	}
#ifdef OS2ONLY
#ifdef NETCONN
	if (network) {
	    printf("?Sorry, REDIRECT doesn't work on network connections\n");
	    return(-9);
	}
#endif /* NETCONN */
#endif /* OS2ONLY */

	sprintf(tmpbuf,
		"Local command to run,\n\
with its standard input/output redirected to %s\n",
		ttname);
	if ((x = cmtxt(tmpbuf,"",&s,xxstring)) < 0)
	  return(x);
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
#ifdef datageneral
	/* AOS/VS can take an argument after its "space" command. */
	if ((x = cmtxt("Confirm, or local directory name","",&s,xxstring)) < 0)
	  return(x);
	if (*s == NUL) {
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
#ifdef OSK
#define UNIX
#endif /* OSK */
#ifdef UNIX
#ifdef COMMENT
	if ((x = cmtxt("Confirm for current disk,\n\
 or specify a disk device or directory","",&s,xxstring)) < 0)
	  return(x);
#else
	x = cmdir("Confirm for current disk,\n\
 or specify a disk device or directory","",&s,xxstring);
	if (x == -3)
	  s = "";
	else if (x < 0)
	  return(x);
	if ((x = cmcfm()) < 0) return(x);
#endif /* COMMENT */
	if (*s == NUL) {		/* Current disk */
	    xsystem(SPACMD);
	} else {			/* Specified disk */
	    sprintf(line,"%s %s",SPACM2,s);
	    xsystem(line);
	}
#ifdef OSK
#undef UNIX
#endif /* OSK */
#else
	if ((x = cmcfm()) < 0) return(x);
	xsystem(SPACMD);
#endif /* UNIX */
#endif /* OS2 */
#endif /* datageneral */
	return(success = 1);		/* Pretend it worked */
    }
#endif /* MAC */

    if (cx == XXSTA) {			/* STATISTICS */
	if ((x = cmcfm()) < 0) return(x);
	return(success = dostat());
    }

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
	    /* Pop from all FOR/WHILE/XIFs */
	    debug(F101,"END maclvl 1","",maclvl);
	    while ((maclvl > 0) &&
		   (m_arg[maclvl-1][0]) &&
		   (cmdstk[cmdlvl].src == CMD_MD) &&
		     (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
		      !strncmp(m_arg[maclvl-1][0],"_for",4) ||
		      !strncmp(m_arg[maclvl-1][0],"_whi",4))) {
		debug(F110,"END popping",m_arg[maclvl-1][0],0);
		dogta(XXPTA);		/* Put args back */
		popclvl();		/* Pop up two levels */
		popclvl();
		debug(F101,"END maclvl 2","",maclvl);
	    }
#endif /* NOSPL */
	    popclvl();			/* Now pop out of macro or TAKE file */
#ifndef NOSPL
	    debug(F101,"END maclvl 3","",maclvl);
#endif /* NOSPL */
	}
	return(success = (x == 0));
    }

    if (cx == XXSUS) {			/* SUSPEND */
	if ((y = cmcfm()) < 0) return(y);
#ifdef NOJC
	printf("Sorry, this version of Kermit cannot be suspended\n");
#else
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

	if (tlevel > MAXTAKE-1) {
	    printf("?Take files nested too deeply\n");
	    return(-2);
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
	strcpy(line,s);
	if ((y = cmcfm()) < 0) return(y);
	return(success = dotake(line));
    }

#ifdef OS2
    if (cx == XXVIEW) {			/* VIEW Only Terminal mode */
	viewonly = TRUE;
	success = doconect(0);
	viewonly = FALSE;
	return success;
    }
#endif /* OS2 */

#ifdef NETCONN
    if (cx == XXTEL) {			/* TELNET */
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
	  ttnproto = NP_TELNET;
	  if ((y = setlin(XYHOST,0)) < 0) {
              nettype = x;		/* Failed, restore net type. */
              ttnproto = z;		/* and protocol */
              success = 0;
              return(y);
           }
        }
	return (success = (y == 0) ? 0 : doconect(0));
    }


#ifdef RLOGCODE
    if (cx == XXRLOG) {			/* RLOGIN */
	int x,z;
#ifdef OS2
	if ( !tcp_avail ) {
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
	    if ((y = setlin(XYHOST,0)) < 0) {
		nettype = x;		/* Failed, restore net type. */
		ttnproto = z;		/* and protocol */
		success = 0;
		return(y);
	    }
#ifdef OS2	    
	}
#endif /* OS2 */
	return (success = (y == 0) ? 0 : doconect(0));
    }
#endif /* RLOGCODE */
#endif /* NETCONN */

#ifndef NOXMIT
    if (cx == XXTRA) {			/* TRANSMIT */
	if ((x = cmifi("File to transmit","",&s,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of an existing file\n");
		return(-9);
	    } else return(x);
	}
	if (y != 0) {
	    printf("?Only a single file may be transmitted\n");
	    return(-2);
	}
	strcpy(line,s);			/* Save copy of string just parsed. */
	if ((y = cmcfm()) < 0) return(y); /* Confirm the command */
#ifdef CK_APC
	if (apcactive == APC_LOCAL || 
        apcactive == APC_REMOTE && apcstatus != APC_UNCH) return(success = 0);
#endif /* CK_APC */
	debug(F111,"calling transmit",line,xmitp);
	return(success = transmit(line,(char)xmitp)); /* Do the command */
    }
#endif /* NOXMIT */

#ifndef NOFRILLS
    if (cx == XXTYP) {			/* TYPE */
	if ((x = cmifi("File to type","",&s,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of an existing file\n");
		return(-9);
	    } else return(x);
	}
	if (y != 0) {
	    printf("?A single file please\n");
	    return(-2);
	}
	strcpy(line,s);
	if ((y = cmcfm()) < 0)		/* Confirm the command */
	  return(y);
	debug(F110,"TYPE",line,0);
	return(success = dotype(line));
    }
#endif /* NOFRILLS */

#ifndef NOFRILLS
    if (cx == XXTES) {			/* TEST */
	/* Fill this in with whatever is being tested... */
	if ((y = cmcfm()) < 0) return(y); /* Confirm the command */

#define CK_DUMPLOCALS			/* Define one or more of these. */

#ifdef CK_DUMPARRAYS
#ifndef NOSPL
	{ int d, i, j;			/* Dump all arrays */
	  char c, **p;
	  for (i = 0; i < 27; i++) {
	      p = a_ptr[i];
	      d = a_dim[i];
	      c = (i == 0) ? 64 : i + 96;
	      if (d && p) {
		  fprintf(stderr,"&%c[%d]\n",c,d);
		  for (j = 0; j <= d; j++) {
		      if (p[j]) {
			  fprintf(stderr,"  &%c[%2d] = [%s]\n",c,j,p[j]);
		      }
		  }
	      }
	  }
      }
#endif /* NOSPL */
#endif /* CK_DUMPARRAYS */

#ifdef CK_DUMPSTACK
#ifndef NOSPL
	printf("cmdlvl = %d, tlevel = %d, maclvl = %d\n",
	       cmdlvl,tlevel,maclvl);
	if (maclvl < 0) {
	    printf("%s\n",
	     "Call me from inside a macro and I'll dump the argument stack");
	    return(0);
	}
	printf("Macro level: %d, ARGC = %d\n     ",maclvl,macargc[maclvl]);
	for (y = 0; y < 10; y++) printf("%7d",y);
	for (x = 0; x <= maclvl; x++) {
	    printf("\n%2d:  ",x);
	    for (y = 0; y < 10; y++) {
		s = m_arg[x][y];
		printf("%7s",s ? s : "(none)");
	    }
	}
	printf("\n");

#endif /* NOSPL */
#endif /* CK_DUMPSTACK */

#ifdef CK_DUMPLOCALS
#ifndef NOSPL
	{
	    extern struct localvar * localhead[];
	    struct localvar * v;
	    int i;
	    printf("Command Level: %d\n",cmdlvl);
	    for (i = 0; i <= cmdlvl; i++) {
		v = localhead[i];
		if (!v) continue;
		printf("Level %d\n",i);
		while (v) {
		    printf("%s [%s]\n",v->lv_name,v->lv_value);
		    v = v->lv_next;
		}
	    }
	}
#endif /* NOSPL */
#endif /* CK_DUMPLOCALS */

	return(0);
    }
#endif /* NOFRILLS */

#ifndef NOCSETS
    if (cx == XXXLA) {	   /* TRANSLATE <ifn> from-cs to-cs <ofn> */
	int incs, outcs;
	if ((x = cmifi("File to translate","",&s,&y,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Name of an existing file\n");
		return(-9);
	    } else return(x);
	}
	if (y != 0) {
	    printf("?A single file please\n");
	    return(-2);
	}
	strcpy(line,s);			/* Save copy of string just parsed. */

	if ((incs = cmkey(fcstab,nfilc,"from character-set","",xxstring)) < 0)
	  return(incs);
	if ((outcs = cmkey(fcstab,nfilc,"to character-set","",xxstring)) < 0)
	  return(outcs);
	if ((x = cmofi("output file",CTTNAM,&s,xxstring)) < 0) return(x);
	if (x > 1) {
	    printf("?Directory name not allowed\n");
	    return(-9);
	}
	strncpy(tmpbuf,s,TMPBUFSIZ);
	if ((y = cmcfm()) < 0) return(y); /* Confirm the command */
	return(success = xlate(line,tmpbuf,incs,outcs)); /* Execute it */
    }
#endif /* NOCSETS */

    if (cx == XXVER) {			/* VERSION */
       if ((y = cmcfm()) < 0) 
          return(y);

       printf("%s, for%s\n Numeric: %ld",versio,ckxsys,vernum);
       if (xvernum)
	 printf("C-Kermit %s\n",ck_ver);
       if (verwho) 
	 printf("-%d\n",verwho); 
       else printf("\n");
       hmsga(copyright);
#ifdef OS2
       shoreg();
#endif /* OS2 */

	return(success = 1);
    }

#ifndef MAC				/* Only for multiuser systems */
#ifndef NOFRILLS
    if (cx == XXWHO) {			/* WHO */
	char *wc;
#ifdef datageneral
        xsystem(WHOCMD);
#else
	if ((y = cmtxt("user name","",&s,xxstring)) < 0) return(y);
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
#endif /* MAC */

#ifndef NOFRILLS
    if (cx == XXWRI || cx == XXWRL || cx == XXWRBL) { /* WRITE */
#ifdef BINREAD
	if (cx == XXWRBL) {
	    extern CHAR * readbuf;
	    extern int readsize;
	    if ((y = cmcfm()) < 0) return(y);
	    y = zsoutx(ZWFILE, readbuf, readsize);
	    printf("WRITEBLOCK = %d\n",y);
	    return(y);
	}	    
#endif /* BINREAD */
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
	  case LOGX:
	  case LOGE:

#ifndef MAC
	    if (x == LOGE) fprintf(stderr,"%s",s);
	    else
#endif /* MAC */
	      printf("%s",s);
	    if (
#ifndef NOSPL
		cmdlvl == 0
#else
		tlevel == -1
#endif /* NOSPL */
		)
#ifndef MAC
	      if (x == LOGE) fprintf(stderr,"\n");
	      else
#endif /* MAC */
		printf("\n");
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

    if (cx == XXASC || cx == XXBIN) {
	if ((x = cmcfm()) < 0) return(x);
	binary = (cx == XXASC) ? XYFT_T : XYFT_B;
	return(success = 1);
    }
#ifdef OS2
    if (cx == XXCLS) {
	if ((x = cmcfm()) < 0) return(x);	
	clear();
	return(success = 1);
    }
#endif /* OS2 */

#ifdef CK_MKDIR
    if (cx == XXMKDIR) {
	if ((x = cmofi("Name for new directory", "", &s, xxstring)) < 0)
	  return(x);
	strcpy(line,s);
	if ((x = cmcfm()) < 0) return(x);
	/* Unfortunately, zmkdir() is not quite what we want here... */
	x =
#ifdef OS2
	  _mkdir(line)
#else
#ifdef UNIX
	  mkdir(line,0777)
#else
	 -1
#endif /* UNIX */
#endif /* OS2 */
	    ;
	return(success = (x < 0) ? 0 : 1);
    }
    if (cx == XXRMDIR) {
	if ((x = cmofi("Name of empty directory to remove",
		       "", &s, xxstring)) < 0)
	  return(x);
	strcpy(line,s);
	if ((x = cmcfm()) < 0) return(x);
	x =
#ifdef NT
	  _rmdir(line)
#else
#ifdef UNIX
	  rmdir(line)
#else
	 -1
#endif /* UNIX */
#endif /* NT */
	    ;
	return(success = (x < 0) ? 0 : 1);
    }
#ifdef TCPSOCKET
    if (cx == XXTELOP) {
        if ((x = cmkey( telcmd, 4, "TELNET command", "", xxstring)) < 0 )
	  return(x);
        if ((y = cmkey( tnopts, ntnopts, "TELNET option", "", xxstring)) < 0 )
	  return(y);	
	if ((z = cmcfm()) < 0) return(z);
	if (local) {
	    return((tn_sopt(x,y) > -1) ? 1 : 0);
	} else {
	    printf("ff%02x%02x\n",x,y);
	    return(0);
	}
    }
#endif /* TCPSOCKET */

#endif /* CK_MKDIR */
                                            
#ifndef NOPUSH
    if ( cx == XXNPSH ) {
        extern int nopush;
#ifndef NOSERVER
	extern int en_hos;
#endif /* NOSERVER */
	if ((z = cmcfm()) < 0) return(z);
        nopush = 1;
#ifndef NOSERVER
        en_hos = 0;
#endif /* NOSERVER */
        return(success = 1);
    }
#endif /* NOPUSH */

#ifndef NOSPL
    if (cx == XXLOCAL)			/* LOCAL variable declarations */
      return(success = dolocal());
#endif /* NOSPL */

    if (cx == XXKERMI) {
	char * list[64];
	extern char **xargv;
	extern int xargc;
	int i;
	if ((y = cmtxt("kermit command-line arguments, -h for help",
		       "",&s,xxstring)) < 0)
	  return(y);
	strcpy(line,"kermit ");
	strcat(line,s);
	xwords(line,64,list,0);
	for (i = 1; i < 64; i++) {
	    if (!list[i])
	      break;
	}
	i--;
	xargc = i;
	xargv = list;
	xargv++;
	if (sstate = cmdlin()) {
	    proto();
	    return(success);
	} else return(success = 1);
    }
    if (cx == XXDATE) {
	if ((z = cmcfm()) < 0) return(z);
	ztime(&s);
	printf("%s\n",s);
	return(success = 1);
    }

    debug(F101,"docmd unk arg","",cx);
    return(-2);				/* None of the above. */
} /* end of docmnd() */

#endif /* NOICP */
