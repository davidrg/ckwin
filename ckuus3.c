#include "ckcsym.h"			/* Symbol definitions */
#ifndef NOICP

/*  C K U U S 3 --  "User Interface" for Unix Kermit, part 3  */
 
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
#ifdef CK_NETBIOS
#include <os2.h>
#ifdef COMMENT				/* Would you believe */
#undef COMMENT				/* <os2.h> defines this ? */
#endif /* COMMENT */
#include "ckonbi.h"
extern UCHAR NetBiosAdapter ;
#endif /* CK_NETBIOS */
#endif /* OS2 */

/* Variables */

extern xx_strp xxstring;

extern int size, spsiz, spmax, urpsiz, srvtim, 
  local, server, success,
  flow, binary, delay, parity, escape, what, srvdis,
  turn, duplex, backgrd,
  turnch, bctr, mdmtyp, keep, maxtry, unkcs, network,
  ebqflg, quiet, swcapr, nettype,
  wslotr, lscapr, lscapu,
  carrier, debses,
  cdtimo, nlangs, bgset, pflag, msgflg, dblchar,
  cmdmsk, spsizr, wildxpand, suspend,
  techo, rptena, rptmin,
  xfrcan, xfrchr, xfrnum, pacing, xitwarn, xitsta,
  cmd_cols, cmd_rows, ckxech;

#ifndef NOKVERBS
extern int nkverbs;
extern struct keytab kverbs[];
#endif /* NOKVERBS */

#ifdef CM_RETRY
extern int cm_retry;
#endif /* CM_RETRY */

#ifdef TNCODE
  extern int ttnproto;
#endif /* TNCODE */

extern char *ccntab[];			/* Names of control chars */
#ifdef CK_SPEED
extern short ctlp[];			/* Control-prefix table */
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

#ifndef NOSPL
#ifdef DCMDBUF
extern int *count, *takerr, *merror, *inpcas;
#else
extern int count[], takerr[], merror[], inpcas[];
#endif /* DCMDBUF */
extern int mecho;			/* Macro echo */
#else
extern int takerr[];
#endif /* NOSPL */

extern int bigsbsiz, bigrbsiz;		/* Packet buffers */

extern long speed;			/* Terminal speed */

extern CHAR sstate;			/* Protocol start state */
extern CHAR myctlq;			/* Control-character prefix */
extern CHAR myrptq;			/* Repeat-count prefix */
extern CHAR mystch, seol;		/* Packet start and end chars */
extern char ttname[];			/* Communication device name */
extern char myhost[] ;

#ifndef NOSETKEY
extern KEY *keymap;			/* Character map for SET KEY (1:1)  */
extern MACRO *macrotab;			/* Macro map for SET KEY (1:string) */
#ifdef OS2
int wideresult;              /* for SET KEY, wide OS/2 scan codes */
char * printfile = NULL;		/* NULL if printer not redirected */
#endif /* OS2 */
#endif /* NOSETKEY */

#ifdef OS2
extern int tcp_avail;			/* Nonzero if TCP/IP is available */
extern int dnet_avail;			/* Ditto for DECnet */
#endif /* OS2 */

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
#endif /* UNIX */
#endif /* NOLOCAL */

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
    "prefix",    2, 0			/* Repeate-prefix character value */
};

#ifndef NOLOCAL
/* For SET CARRIER */

struct keytab crrtab[] = {
    "auto", CAR_AUT, 0,
    "off",  CAR_OFF, 0,
    "on",   CAR_ON, 0
};
int ncrr = 3;
#endif /* NOLOCAL */

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
  "2400",   240,  0,
#ifdef BPS_28K
  "28800", 2880,  0,
#endif /* BPS_28K */
#ifdef BPS_230K
  "230400", 2304, 0,
#endif /* BPS_230K */
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
    "display",          XYFILD, 0,
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
#ifdef CK_DTRCD
    "dtr/cd",   FLO_DTRC, 0,
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    "dtr/cts",FLO_DTRT, 0,
#endif /* CK_DTRCTS */
    "keep",     FLO_KEEP, 0,
    "none",     FLO_NONE, 0,
#ifdef CK_RTSCTS
    "rts/cts",  FLO_RTSC, 0,
#endif /* CK_RTSCTS */
    "xon/xoff", FLO_XONX, 0
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
extern struct keytab mdmtab[] ;		/* Modem types (in module ckudia.c) */
extern int nmdm;			/* Number of them */
#ifndef MINIDIAL
extern int tbmodel;			/* Telebit model ID */
#endif /* MINIDIAL */
#endif /* NODIAL */
 
#ifndef NOPUSH
#ifdef UNIX
struct keytab wildtab[] = {		/* SET WILDCARD-EXPANSION */
    "kermit",  0, 0,
    "shell",   1, 0
};
#endif /* UNIX */
#endif /* NOPUSH */

#ifdef NETCONN
extern struct keytab netcmd[];
extern int nnets;
#ifdef NPIPE
char pipename[PIPENAML+1];
#endif /* NPIPE */
#ifdef CK_NETBIOS
extern unsigned char NetBiosName[] ;
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
    "status",    0, 0,			/* ...STATUS */
    "warning",   1, 0			/* ...WARNING */
};
int nexit = (sizeof(xittab) / sizeof(struct keytab));

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

#define SCMD_BSZ 0			/* SET COMMAND items */
#define SCMD_RCL 1
#define SCMD_RTR 2
#define SCMD_QUO 3

/* SET COMMAND table */
struct keytab scmdtab[] = {
    "bytesize",  SCMD_BSZ, 0,
    "quoting",   SCMD_QUO, 0
#ifdef CK_RECALL
,   "recall-buffer-size", SCMD_RCL, 0
#endif /* CK_RECALL */
#ifdef CM_RETRY
,   "retry", SCMD_RTR, 0
#endif /* CM_RETRY */
};
int nbytt = (sizeof(scmdtab) / sizeof(struct keytab));

#ifndef NOSERVER
/* Server parameters table */
struct keytab srvtab[] = {
    "display", XYSERD, 0,
    "timeout", XYSERT, 0
};
#endif /* NOSERVER */

/* SET TRANSFER/XFER table */

struct keytab tstab[] = {
#ifdef XFRCAN
/*
  This should be propogated to all versions...
*/
    "cancellation",   0,   0,
#endif /* XFRCAN */
#ifndef NOCSETS
    "character-set", 1,   0,
#endif /* NOCSETS */
    "locking-shift", 2,   0
};
int nts = (sizeof(tstab) / sizeof(struct keytab));

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
extern int tn_duplex, tn_nlm;
extern char *tn_term;

static struct keytab tnlmtab[] = {	/* NEWLINE-MODE table */
    "off",    TNL_CRNUL, 0,		/* CR-NUL (TELNET spec) */
    "on",     TNL_CRLF, 0,		/* CR-LF (TELNET spec) */
    "raw",    TNL_CR, 0			/* CR only (out of spec) */
};
static int ntnlm = (sizeof(tnlmtab) / sizeof(struct keytab));

struct keytab tntab[] = {
    "echo",          CK_TN_EC,   0,
    "newline-mode",  CK_TN_NL,   0,
    "terminal-type", CK_TN_TT,   0
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
"transmit",		1, 0
#endif /* NOXMIT */
};
int nftr = (sizeof(ftrtab) / sizeof(struct keytab));

#ifndef NOSPL /* Present this is used only with script programming items... */

/*  K W D H E L P  --  Given a keyword table, print keywords in columns.  */
/*
  Call with:
    s     - keyword table
    n     - number of entries
    pre   - prefix for each keyword
    post  - suffix for each keyword
    off   - offset on first screenful, allowing room for introductory text

  Arranges keywords in columns with width based on longest keyword.
  Does "more?" prompting at end of screen.  
  Uses global cmd_rows and cmd_cols for screen size.
*/
VOID
kwdhelp(s,n,pre,post,off) struct keytab s[]; int n, off; char *pre, *post; {
    int width = 0;
    int cols, height, i, j, k, lc, n2 = 0;
    char *b, *p, *q;
    char *pa, *px;
    char **s2;
    
    if (!s) return;			/* Nothing to do */
    if (n < 1) return;			/* Ditto */
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (s2 = (char **) malloc(n * sizeof(char *))) {
	for (i = 0; i < n; i++) {	/* Find longest keyword */
	    if (s[i].flgs & CM_INV)	/* Skip invisible ones */
	      continue;
	    s2[n2++] = s[i].kwd;	/* Copy pointers to visible ones */
	    j = strlen(s[i].kwd);
	    if (j > width) width = j;
	}
	/* Column width */
	n = n2;
    }
    if (s2 && (b = (char *) malloc(cmd_cols + 1))) { /* Make a line buffer   */
	width += (int)strlen(pre) + (int)strlen(post) + 2; 
	cols = cmd_cols / width;		/* How many columns? */
	height = n / cols;		/* How long is each column? */
	if (n % cols) height++;		/* Add one for remainder, if any */

	for (i = 0; i < height; i++) {	    /* Loop for each row */
	    for (j = 0; j < cmd_cols; j++)  /* First fill row with blanks */
	      b[j] = SP;
	    for (j = 0; j < cols; j++) {    /* Loop for each column in row */
		k = i + (j * height);       /* Index of next keyword */
		if (k < n) {		    /* In range? */
		    pa = pre; px = post;
		    p = s2[k];		    /* Point to verb name */
		    q = b + (j * width) + 1; /* Where to copy it to */
		    while (*q++ = *pa++) ;  /* Copy prefix */
		    q--;		    /* Back up over NUL */
		    while (*q++ = *p++) ;   /* Copy the keyword */
		    q--;		    /* Back up over NUL */
		    while (*q++ = *px++) ;  /* Copy suffix */
		    q--;
		    *q = SP;		/* Replace the space */
		}
	    }
	    p = b + cmd_cols - 1;
	    while (*p-- == SP) ;
	    *(p+2) = NUL;
	    printf("%s\n",b);		/* Print the line */
	    if (++lc > (cmd_rows - 3)) { /* Screen full? */
		if (!askmore()) {	/* Do more-prompting... */
		    free(b);
		    return;
		} else
		  lc = 0;
	    }
	} 
	printf("\n");			/* Blank line at end of report */
	free(s2);			/* Free array copy */
	free(b);			/* Free line buffer */
	return;
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    if (s[i].flgs & CM_INV)	/* Use original keyword table */
	      continue;			/* skipping invisible entries */
	    printf("%s%s%s\n",pre,s[i].kwd,post);
	    if (++lc > (cmd_rows - 3)) { /* Screen full? */
		if (!askmore()) {	 /* Do more-prompting... */
		    return;
		} else lc = 0;
	    }	    
	}
	if (s2) free(s2);		/* Free array copy, if any */
	return;
    }
}
#endif /* NOSPL */

int					/* CHECK command */
dochk() {
    int x, y;
    if ((y = cmkey(ftrtab,nftr,"","",xxstring)) < 0)
      return(y);
    strcpy(line,atmbuf);
    if ((y = cmcfm()) < 0)
      return(y);
    y = lookup(ftrtab,line,nftr,&x);	/* This will succeed. */
    if (msgflg)				/* If at top level... */
      printf(" %s%s available\n", ftrtab[x].kwd, y ? " not" : "");
    else if (y && !backgrd)
      printf(" CHECK: %s not available\n", ftrtab[x].kwd);
    return(success = 1 - y);
}

#ifndef NOSETKEY
int
set_key() {				/* SET KEY */
    int x, y;
    int flag = 0;
    int kc;				/* Key code */
    char *s;				/* Key binding */
    char *p;				/* Worker */

    if ((y = cmnum("numeric key code, or the word CLEAR,",
		   "",10,&kc,xxstring)) < 0) {
	debug(F111,"SET KEY",atmbuf,y);
	if (y == -2) {			/* Not a valid number */
	    if ((y = strlen(atmbuf)) < 0) /* Check for SET KEY CLEAR */
	      return(-2);
	    if (xxstrcmp(atmbuf,"clear",y))
	      return(-2);
	    for (y = 0; y < KMSIZE; y++) {
		keymap[y] = y;
		macrotab[y] = NULL;
	    }
#ifdef OS2
	    keymapinit();		/* Special OS/2 initializations */
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
	    kc = congks(0);		/* Get character or scan code */
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
    p = s;				/* Save this place */
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
#else
    p = NULL;
#endif /* NOKVERBS */

    if (macrotab[kc]) {			/* Possibly free old macro from key. */
	free(macrotab[kc]);
	macrotab[kc] = NULL;
    }
    switch (strlen(s)) {		/* Action depends on length */
      case 0:				/* Reset to default binding */
	keymap[kc] = kc;
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
	keymap[kc] = kc;
	macrotab[kc] = (MACRO) malloc(strlen(s)+1);
	if (macrotab[kc])
	  strcpy((char *) macrotab[kc], s);
	break;
    }
    if (flag) cmsetp(psave);
    return(1);
}
#endif /* NOSETKEY */

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
    char *s;
 
#ifdef OS2
    if (xx == XYPRTR) {			/* SET PRINTER */
	if ((x = cmofi("printer file","PRN",&s,xxstring)) < 0)
	  return(x);
	if (x > 1) {
	    printf("?Directory names not allowed\n");
	    return(-9);
	}
	strcpy(line,s);			/* Make a temporary safe copy */
	if ((x = cmcfm()) < 0)		/* Confirm the command */
	  return(x);
	if (printfile) {		/* Had a print file before? */
	    free(printfile);		/* Remove its name */
	    printfile = NULL;
	}
	x = strlen(line);		/* Length of name of new print file */
	if ((x != 3) || (xxstrcmp(line,"PRN",3) != 0)) {
	    printfile = (char *) malloc(x + 1);	/* Allocate space for it */
	    if (!printfile) {
 		printf("?Memory allocation failure\n");
		return(-9);
	    }
	    strcpy(printfile,line);	/* Copy new name to new space */
	    debug(F110,"printfile name",printfile,0);
	} else
	    debug(F101,"printfile is NULL","",printfile);
	/* Return with printfile pointing to a file or device name */
	/* or NULL to indicate the default printer. */
	return(success = 1);
    }
#endif /* OS2 */

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
    }
    if (tcp_avail)			/* Default network type */
      strcpy(tmpbuf,"tcp/ip");
#ifdef DECNET
    else if (dnet_avail)
      strcpy(tmpbuf,"decnet");
#endif /* DECNET */
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

#ifdef OS2
    if (z == NET_TCPB && tcp_avail == 0) {
	printf("?Sorry, TCP/IP is not available on this system.\n") ;
	return(-9);
#ifdef CK_NETBIOS
    } else if (z == NET_BIOS && netbiosAvail == 0) {
	printf("?Sorry, NETBIOS is not available on this system.\n") ;
	return(-9);
#endif /* CK_NETBIOS */
#ifdef DECNET
    } else if (z == NET_DEC && dnet_avail == 0) {
	printf("?Sorry, DECnet is not available on this system.\n") ;
	return(-9);
#endif /* DECNET */
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
		char *p; int n;		/* use it as the default. */
		strcpy(tmpnbnam,NetBiosName);
		p=tmpnbnam + NETBIOS_NAME_LEN - 1;
		while ( *p == SP ) {
		    *p = NUL ;
		    p-- ;
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
	(nettype != NET_DEC)  &&
#ifdef NPIPE
	(nettype != NET_PIPE) &&
#endif /* NPIPE */
#ifdef CK_NETBIOS
	(nettype != NET_BIOS) &&
#endif /* CK_NETBIOS */
	(nettype != NET_SX25) &&
	(nettype != NET_VX25) &&
        (nettype != NET_TCPB)) {
	printf("?Network type not supported\n");
	return(success = 0);
    } else {
	return(success = 1);
    }
#endif /* NETCONN */

case XYHOST:				/* SET HOST or SET LINE */
case XYLINE:
    return(setlin(xx,1));
 
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
    if ((x = cmkey(chktab,4,"","1",xxstring)) < 0) return(x);
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
    if ((y = cmkey(crrtab,ncrr,"","auto",xxstring)) < 0) return(y);
    if (y == CAR_ON) {
	x = cmnum("Carrier wait timeout, seconds","0",10,&z,xxstring);
	if (x < 0) return(z);
    }
    if ((x = cmcfm()) < 0) return(x);
    carrier = ttscarr(y);
    cdtimo = z;
    return(success = 1);
#endif /* MAC */
#endif /* NOLOCAL */
#ifdef TNCODE
case XYTEL:				/* TELNET */
    if ((z = cmkey(tntab,ntn,"parameter for TELNET negotiations", "",
		   xxstring)) < 0) return(z);
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
	if (tn_term) free(tn_term);	/* Free any previous storage */
	if (s == NULL || *s == NUL) {	/* If none given */
	    tn_term = NULL;		/* remove the override string */
	    return(success = 1);
	} else if (tn_term = malloc(strlen(s)+1)) { /* Make storage for new */
	    strcpy(tn_term,s);		/* Copy string into new storage */
	    return(success = 1);
	} else return(success = 0);

      case CK_TN_NL:			/* NEWLINE-MODE */
        if ((x = cmkey(tnlmtab,ntnlm,"","on",xxstring)) < 0)
	  return(x);
	if ((y = cmcfm()) < 0)
	  return(y);
	tn_nlm = x;
	return(success = 1);

      default:
	return(-2);
    }
#endif /* TNCODE */

default:
    break;
}

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
#ifdef CM_RETRY
      case SCMD_RTR:
	return(success = seton(&cm_retry));
#endif /* CM_RETRY */
      case SCMD_QUO:
	if ((x = seton(&y)) < 0) return(x);
	cmdsquo(y);
	/* Set string-processing function */
#ifdef datageneral
	xxstring = y ? zzstring : (xx_strp) NULL;
#else
	xxstring = y ? zzstring : NULL;
#endif /* datageneral */
	return(success = 1);
      default:
	return(-2);
    }
    
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
case XYDIAL:				/* SET DIAL */
    return(setdial());
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
    y = cmnum("Decimal ASCII code for CONNECT-mode escape character",
	      tmpbuf, 10,&x,xxstring);
    success = setcc(&escape,x,y);
#ifdef COMMENT
/* This is what SHOW ESCAPE is for. */
    if (success && msgflg)
      printf(" CONNECT-mode escape character: %d (Ctrl-%c, %s)\n",
	     escape,ctl(escape),(escape == 127 ? "DEL" : ccntab[escape]));
#endif /* COMMENT */
    return(success);
#endif /* NOLOCAL */

case XYEXIT:				/* SET EXIT */
    if ((z = cmkey(xittab,nexit,"","",xxstring)) < 0)
      return(z);
    switch (z) {
      case 0:				/* STATUS */
	y = cmnum("EXIT status code","",10,&x,xxstring);
	return(success = setnum(&xitsta,x,y,-1));
      case 1:				/* WARNING */
	if ((z = cmkey(onoff,2,"","",xxstring)) < 0) return(z);
	if ((y = cmcfm()) < 0) return(y);
	xitwarn = z;
	return(success = 1);
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

#ifndef NODIAL
case XYMODM:				/* SET MODEM */
    if ((x = cmkey(mdmtab,nmdm,"type of modem","none", xxstring)) < 0)
	return(x);
    if ((z = cmcfm()) < 0) return(z);
    mdmtyp = x;
#ifndef MINIDIAL
    tbmodel = 0;          /* If it's a Telebit, we don't know the model yet */
#endif /* MINIDIAL */
    return(success = 1);
#endif /* NODIAL */

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
#ifdef MAC
    if ((x = cmtxt("Program's command prompt","Mac-Kermit>",&s,NULL)) < 0)
#else
    if ((x = cmtxt("Program's command prompt","C-Kermit>",&s,NULL)) < 0)
#endif /* MAC */
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
    if ((y = cmkey(srvtab,2,"","",xxstring)) < 0) return(y);
    switch (y) {
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
      default:
	return(-2);
    }
#endif /* NOSERVER */

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

default:
    break;
}
#endif /* NOLOCAL */

switch (xx) {

/* SET SEND/RECEIVE protocol parameters. */
 
case XYRECV:
case XYSEND:
    return(setsr(xx,rmsflg));
 
#ifndef NOLOCAL
#ifdef UNIX
case XYSESS:				/* SESSION-LOG */
    if ((x = cmkey(sfttab,nsfttab,"type of file","text",xxstring)) < 0)
      return(x);
    if ((y = cmcfm()) < 0) return(y);
    sessft = x;
    return(success = 1);
#endif /* UNIX */

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
#ifdef XFRCAN
    if (y == 0) {			/* CANCELLATION */
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
    } else
#endif /* XFERCAN */
#ifndef NOCSETS
      if (y == 1) {			/* CHARACTER-SET */
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
    } else
#endif /* NOCSETS */
      if (y == 2) {			/* LOCKING-SHIFT */
	  if ((y = cmkey(lstab,nls,"","on",xxstring)) < 0)
	    return(y);
	  if ((x = cmcfm()) < 0) return(x);
	  lscapr = (y == 1) ? 1 : 0;	/* ON: requested = 1 */
	  lscapu = (y == 2) ? 2 : 0;	/* FORCED:  used = 1 */
	  return(success = 1);
      } else return(-2);

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
    if ((y = cmkey(wildtab,2,"who expands wildcards","kermit",xxstring)) < 0)
      return(y);
    if ((x = cmcfm()) < 0) return(x);
    wildxpand = y;
    return(success = 1);
#endif /* UNIX */
#endif /* NOPUSH */

  case XYWIND:				/* WINDOW-SLOTS */
    y = cmnum("Number of sliding-window slots, 1 to 32","1",10,&x,xxstring);
    y = setnum(&z,x,y,MAXWS);
    if (y < 0) return(y);
    if (z < 1) z = 1;
#ifdef COMMENT
    /* This is taken care of automatically now in protocol negotiation */
    if (maxtry < z) {
	printf("?Window slots must be less than retry limit\n");
	return(success = 0);
    }
#endif /* COMMENT */
    if (rmsflg) {			/* Set remote window size */
	wslotr = z;			/* Set local window size too */
	tp = tmpbuf;
	sprintf(tp,"%d",z);
	sstate = setgen('S', "406", tp, "");
	return((int) sstate);
    }
    wslotr = z;				/* Set local window size */
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
	while (1) {		/* Collect a list of numbers */
	    if ((x = cmnum((z == 0) ?
"\n Numeric ASCII value of control character that needs NO prefix,\n\
 or the word \"all\", or carriage return to complete the list" :
"\n Numeric ASCII value of control character that MUST BE prefixed,\n\
 or the word \"all\", or carriage return to complete the list",
			   "",10,&y,xxstring
			   )) < 0) {
		if (x == -3)
		  break;
		if (x == -2) {
		    if (p) free(p);
		    debug(F110,"SET CONTROL atmbuf",atmbuf,0);
		    if (!xxstrcmp(atmbuf,"all",3) ||
			!xxstrcmp(atmbuf,"al",2) ||
			!xxstrcmp(atmbuf,"a",1)) {
			if ((x = cmcfm()) < 0) /* Get confirmation */
			  return(x);
			/* Set all values, but don't touch 0 */
			for (y = 1; y < 32; y++) ctlp[y] = z;
			for (y = 127; y < 160; y++) ctlp[y] = z;
			ctlp[255] = z;
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
	    p[y] = z;			/* All OK, set flag */
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

default:
    if ((x = cmcfm()) < 0) return(x);
    printf("Not working yet - %s\n",cmdbuf);
    return(success = 0);
    }
}

#ifdef CK_TTYFD
extern int ttyfd;
#endif /* CK_TTYFD */ 

extern struct keytab yesno[];
extern int nyesno;

int
hupok(x) int x; {			/* Returns 1 if OK, 0 if not OK */
    int y, z = 1;
    if (local && xitwarn) {		/* Is a connection open? */
	int needwarn = 0;
	if (
#ifdef NETCONN
	    network
#else
	    0
#endif /* NETCONN */
	    ) {		/* Network? */
#ifdef CK_TTYFD
	    if (ttyfd > -1)
#endif /* CK_TTYFD */
	      needwarn = 1;
	    if (needwarn)
	      printf(
" A network connection to %s might still be active.\n",
		     ttname
		     );
	} else {			/* Serial connection */
	    if (carrier == CAR_OFF)	/* SET CARRIER OFF */
	      needwarn = 0;		/* so we don't care about carrier. */
	    else if ((y = ttgmdm()) > 0) /* Otherwise, get modem signals */
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

	if (needwarn) {
#ifdef VMS
/*
  In VMS, whenever a TAKE file or macro is active, we restore the 
  original console modes so Ctrl-C/Ctrl-Y can work.  But here we
  go interactive again, so we have to temporarily put them back.
*/
	    if (cmdlvl > 0)
	      concb((char)escape);
#endif /* VMS */
      
	    cmsavp(psave,PROMPTL);	/* Save old prompt */
	    cmsetp(x ? "OK to close? " : "OK to exit? "); /* Make new prompt */
	    z = 0;			/* Initialize answer to No. */
	    cmini(ckxech);		/* Initialize parser. */
	    do {
		prompt(NULL);		/* Issue prompt. */
		y = cmkey(yesno,nyesno,"","",NULL); /* Get Yes or No */
		if (y < 0) {
		    if (y == -3) 	/* No answer? */
		      printf(" Please respond Yes or No\n");
		    cmini(ckxech);
		} else {
		    z = y;		/* Save answer */
		    y = cmcfm();	/* Get confirmation */
		}
	    } while (y < 0);		/* Continue till done */
	    cmsetp(psave);		/* Restore real prompt */
#ifdef VMS
	    if (cmdlvl > 0)		/* In VMS and not at top level, */
	      conres();			/*  restore console again. */
#endif /* VMS */
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
