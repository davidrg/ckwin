#include "ckcsym.h"
#define DOCHKVAR

char *cmdv = "Command package 7.0.121, 24 Dec 1999";

/*  C K U C M D  --  Interactive command package for Unix  */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#ifdef OS2                    /* Command-terminal-to-C-Kermit character mask */
int cmdmsk = 255;
#else
int cmdmsk = 127;
#endif /* OS2 */

#ifdef BS_DIRSEP			/* Directory separator is backslash */
#undef BS_DIRSEP
#endif /* BS_DIRSEP */

#ifdef OS2
#define BS_DIRSEP
#endif /* BS_DIRSEP */

#define CKUCMD_C

#include "ckcdeb.h"                     /* Formats for debug(), etc. */
#include "ckcker.h"			/* Needed for BIGBUFOK definition */
#include "ckcnet.h"			/* Needed for server-side Telnet */
#include "ckucmd.h"			/* Needed for xx_strp prototype */
#include "ckuusr.h"                     /* Needed for prompt length */

#undef CKUCMD_C

_PROTOTYP( int unhex, (char) );
_PROTOTYP( static VOID cmdclrscn, (void) );

struct keytab cmonths[] = {
  { "april",     4, 0 },
  { "august",    8, 0 },
  { "december", 12, 0 },
  { "february",  2, 0 },
  { "january",   1, 0 },
  { "july",      7, 0 },
  { "june",      6, 0 },
  { "march",     3, 0 },
  { "may",       5, 0 },
  { "november", 11, 0 },
  { "october",  10, 0 },
  { "september", 9, 0 }
};

#ifndef NOICP     /* The rest only if interactive command parsing selected */

#ifndef NOSPL
_PROTOTYP( int chkvar, (char *) );
extern int askflag;
#endif /* NOSPL */

int cmfldflgs = 0;			/* Flags for cmfld() */
static int cmkwflgs = 0;		/* Flags from last keyword parse */
static int blocklvl = 0;		/* Block nesting level */
static int linebegin = 0;		/* Flag for at start of a line */
static int quoting = 1;			/* Quoting is allowed */
static int swarg = 0;			/* Parsing a switch argument */
static int xcmfdb = 0;			/* Flag for parsing chained fdbs... */
static int chsrc = 0;			/* Source of character, 1 = tty */

#ifdef BS_DIRSEP
static int dirnamflg = 0;
#endif /* BS_DIRSEP */

/*
Modeled after the DECSYSTEM-20 command parser (the COMND JSYS), RIP. Features:

. parses and verifies keywords, filenames, text strings, numbers, other data
. displays appropriate menu or help message when user types "?"
. does keyword and filename completion when user types ESC or TAB
. does partial filename completion
. accepts any unique abbreviation for a keyword
. allows keywords to have attributes, like "invisible" and "abbreviation"
. can supply defaults for fields omitted by user
. provides command retry and recall
. provides command line editing (character, word, and line deletion)
. accepts input from keyboard, command files, macros, or redirected stdin
. allows for full or half duplex operation, character or line input
. allows \-escapes for hard-to-type characters
. allows specification of a user exit to expand variables, etc.
. settable prompt, protected from deletion, dynamically re-evaluated each time.
. allows chained parse functions.

Functions:
 cmsetp - Set prompt (cmprom is prompt string)
 cmsavp - Save current prompt
 prompt - Issue prompt
 cmini  - Clear the command buffer (before parsing a new command)
 cmres  - Reset command buffer pointers (before reparsing)
 cmkey  - Parse a keyword or token (also cmkey2)
 cmswi  - Parse a switch
 cmnum  - Parse a number
 cmifi  - Parse an input file name
 cmofi  - Parse an output file name (also cmifip, cmifi2, ...)
 cmdir  - Parse a directory name (also cmdirp)
 cmfld  - Parse an arbitrary field
 cmtxt  - Parse a text string
 cmdate - Parse a date-time string
 cmcfm  - Parse command confirmation (end of line)
 cmfdb  - Parse any of a list of the foregoing (chained parse functions)

Return codes:
 -9: like -2 except this module already printed the error message
 -3: no input provided when required
 -2: input was invalid (e.g. not a number when a number was required)
 -1: reparse required (user deleted into a preceding field)
  0 or greater: success
See individual functions for greater detail.

Before using these routines, the caller should #include ckucmd.h, and set the
program's prompt by calling cmsetp().  If the file parsing functions cmifi,
cmofi, or cmdir are to be used, this module must be linked with a ck?fio file
system support module for the appropriate system, e.g. ckufio for Unix.  If
the caller puts the terminal in character wakeup ("cbreak") mode with no echo,
then these functions will provide line editing -- character, word, and line
deletion, as well as keyword and filename completion upon ESC and help
strings, keyword, or file menus upon '?'.  If the caller puts the terminal
into character wakeup/noecho mode, care should be taken to restore it before
exit from or interruption of the program.  If the character wakeup mode is not
set, the system's own line editor may be used.

NOTE: Contrary to expectations, many #ifdef's have been added to this module.
Any operation requiring an #ifdef (like clear screen, get character from
keyboard, erase character from screen, etc) should eventually be turned into a
call to a function that is defined in ck?tio.c, but then all the ck?tio.c
modules would have to be changed...
*/

/* Includes */

#include "ckcker.h"
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckucmd.h"                     /* Command parsing definitions */

#ifdef OSF13
#ifdef CK_ANSIC
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif /* _NO_PROTO */
#endif /* CK_ANSIC */
#endif /* OSF13 */

#include <errno.h>			/* Error number symbols */

#ifdef OS2
#ifndef NT
#define INCL_NOPM
#define INCL_VIO			/* Needed for ckocon.h */
#include <os2.h>
#undef COMMENT
#else
#define APIRET ULONG
#include <windows.h>
#endif /* NT */
#include "ckocon.h"
#include <io.h>
#endif /* OS2 */

#ifdef NT
#define stricmp _stricmp
#endif /* NT */

#ifdef OSK
#define cc ccount			/* OS-9/68K compiler bug */
#endif /* OSK */

#ifdef GEMDOS				/* Atari ST */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
#endif /* GEMDOS */

#ifdef CK_AUTODL
extern int cmdadl, justone;
#endif /* CK_AUTODL */

extern int timelimit, nzxopts;

#ifdef CKSYSLOG
#ifdef UNIX
#ifdef CKXPRINTF			/* Our printf macro conflicts with */
#undef printf				/* use of "printf" in syslog.h */
#endif /* CKXPRINTF */
#include <syslog.h>
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#endif /* UNIX */
#endif /* CKSYSLOG */

/* Local variables */

static
int psetf = 0,                          /* Flag that prompt has been set */
    cc = 0,                             /* Character count */
    dpx = 0,                            /* Duplex (0 = full) */
    inword = 0;				/* In the middle of getting a word */

#ifdef OLDHELP
static
int hw = HLPLW,                         /* Help line width */
    hc = HLPCW,                         /* Help line column width */
    hh,                                 /* Current help column number */
    hx;                                 /* Current help line position */
#endif /* OLDHELP */
char *dfprom = "Command? ";             /* Default prompt */

int cmflgs;                             /* Command flags */
int cmfsav;				/* A saved version of them */

static char pushc = NUL;
static char brkchar = NUL;

#define CMDEFAULT 1023
static char cmdefault[CMDEFAULT+1];

#ifdef DCMDBUF
char *cmdbuf = NULL;			/* Command buffer */
char *savbuf = NULL;			/* Buffer to save copy of command */
char *atmbuf = NULL;			/* Atom buffer - for current field */
char *atxbuf = NULL;			/* For expanding the atom buffer */
#ifdef OLDHELP
static char *hlpbuf;			/* Help string buffer */
#endif /* OLDHELP */
static char *atybuf = NULL;		/* For copying atom buffer */
static char *filbuf = NULL;		/* File name buffer */
static char *cmprom = NULL;		/* Program's prompt */
static char *cmprxx = NULL;		/* Program's prompt, unevaluated */

#ifdef CK_RECALL
/*
  Command recall is available only if we can make profligate use of malloc().
*/
#define R_MAX 10			/* How many commands to save */
int cm_recall = R_MAX;			/* Size of command recall buffer */
int on_recall = 1;			/* Recall feature is ON */
static int no_recall = 0;		/* Recall OFF for this cmd only */
static int force_add = 0;		/* Force cmd into recall buffer */
int in_recall = 0;			/* Recall buffers are init'd */
static int
  current = -1,				/* Pointer to current command */
  rlast = -1;				/* Index of last command in buffer */
static char **recall = NULL;		/* Array of recall buffer pointers */
#endif /* CK_RECALL */
#else
char cmdbuf[CMDBL+4];                   /* Command buffer */
char savbuf[CMDBL+4];                   /* Buffer to save copy of command */
char atmbuf[ATMBL+4];                   /* Atom buffer */
char atxbuf[CMDBL+4];                   /* For expanding the atom buffer */
#ifdef OLDHELP
static char hlpbuf[HLPBL+4];		/* Help string buffer */
#endif /* OLDHELP */
static char atybuf[ATMBL+4];		/* For copying atom buffer */
static char filbuf[ATMBL+4];		/* File name buffer */
static char cmprom[PROMPTL+1];		/* Program's prompt */
static char cmprxx[PROMPTL+1];		/* Program's prompt, unevaluated */
#endif /* DCMDBUF */

/* Command buffer pointers */

#define PPVLEN 24
char ppvnambuf[PPVLEN+1] = { NUL, NUL };

char * cmbptr = NULL;			/* Current position (for export) */

static char *bp,                        /* Current command buffer position */
    *pp,                                /* Start of current field */
    *np;                                /* Start of next field */

static int ungw,			/* For ungetting words */
    atxn;				/* Expansion buffer (atxbuf) length */

#ifdef OS2
extern int wideresult;
#endif /* OS2 */

extern int cmd_cols, cmd_rows, local, quiet;

#ifdef TNCODE
#ifdef IAC
#undef IAC
#endif /* IAC */
#define IAC 255
#endif /* TNCODE */

#ifdef OLDHELP
_PROTOTYP( static VOID addhlp, (char *) );
_PROTOTYP( static VOID clrhlp, (void) );
_PROTOTYP( static VOID dmphlp, (void) );
#endif /* OLDHELP */
_PROTOTYP( static int gtword, (int) );
_PROTOTYP( static int addbuf, (char *) );
_PROTOTYP( static int setatm, (char *, int) );
_PROTOTYP( static VOID cmdnewl, (char) );
_PROTOTYP( static VOID cmdchardel, (void) );
_PROTOTYP( static VOID cmdecho, (char, int) );
_PROTOTYP( static int test, (int, int) );
#ifdef GEMDOS
_PROTOTYP( extern char *strchr, (char *, int) );
#endif /* GEMDOS */

extern char * dftty;

/* The following are for use with chained FDB's */

static int crflag = 0;			/* Carriage return was typed */
static int qmflag = 0;			/* Question mark was typed */
static int esflag = 0;			/* Escape was typed */

/* Directory separator */

#ifdef GEMDOS
static char dirsep = '\\';
#else
#ifdef datageneral
static char dirsep = ':';
#else
#ifdef MAC
static char dirsep = ':';
#else
#ifdef VMS
static char dirsep = '.';
#else
#ifdef STRATUS
static char dirsep = '>';
#else
static char dirsep = '/';		/* UNIX, OS/2, OS-9, Amiga, etc. */
#endif /* STRATUS */
#endif /* VMS */
#endif /* MAC */
#endif /* datageneral */
#endif /* GEMDOS */

/*  C K S P R E A D  --  Print string double-spaced  */

static char * sprptr = NULL;

static char *
ckspread(s) char * s; {
    int n = 0;
    char * p;
    n = strlen(s);
    if (sprptr)
      free(sprptr);
    sprptr = malloc(n + n + 3);
    if (sprptr) {
	p = sprptr;
	while (*s) {
	    *p++ = *s++;
	    *p++ = SP;
	}
	*p = NUL;
    }
    return(sprptr ? sprptr : "");
}

/*  T E S T  --  Bit test  */

static int
test(x,m) int x, m; { /*  Returns 1 if any bits from m are on in x, else 0  */
    return((x & m) ? 1 : 0);
}

/*  K W D H E L P  --  Given a keyword table, print keywords in columns.  */
/*
  Call with:
    s     - keyword table
    n     - number of entries
    pat   - pattern (left substring) that must match for each keyword
    pre   - prefix to add to each keyword
    post  - suffix to add to each keyword
    off   - offset on first screenful, allowing room for introductory text
    xhlp  - 1 to print any CM_INV keywords that are not also abbreviations.
            2 to print CM_INV keywords if CM_HLP also set
            4 if it's a switch table (to show ':' if CM_ARG)

  Arranges keywords in columns with width based on longest keyword.
  Does "more?" prompting at end of screen.
  Uses global cmd_rows and cmd_cols for screen size.
*/
VOID
kwdhelp(s,n,pat,pre,post,off,xhlp)
    struct keytab s[]; int n, off, xhlp; char *pat, *pre, *post;
/* kwdhelp */ {

    int width = 0;
    int cc;
    int cols, height, i, j, k, lc, n2 = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;
    char *tmpbuf = NULL;

    cc = strlen(pat);

    if (!s) return;			/* Nothing to do */
    if (n < 1) return;			/* Ditto */
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (xhlp & 4)			/* For switches */
      tmpbuf = (char *)malloc(TMPBUFSIZ+1);

    if (s2 = (char **) malloc(n * sizeof(char *))) {
	for (i = 0; i < n; i++) {	/* Find longest keyword */
	    s2[i] = NULL;
	    if (ckstrcmp(s[i].kwd,pat,cc,0))
	      continue;
	    if (s[i].flgs & CM_INV) {
#ifdef COMMENT
/* This code does not show invisible keywords at all except for "help ?" */
/* and then only help topics (CM_HLP) in the top-level keyword list. */

		if ((xhlp & 2) == 0)
		  continue;
		else if ((s[i].flgs & CM_HLP) == 0)
		  continue;
#else
/* This code shows invisible keywords that are not also abbreviations when */
/* ? was typed AFTER the beginning of the field so the user can find out */
/* what they are and (for example) why completion doesn't work at this point */

		if (s[i].flgs & CM_ABR)
		  continue;
		else if ((xhlp & 3) == 0)
		  continue;
		else if ((xhlp & 2) && ((s[i].flgs & CM_HLP) == 0))
		  continue;
#endif /* COMMENT */
	    }
	    j = strlen(s[i].kwd);
	    if (!(xhlp & 4) || !tmpbuf) { /* Regular keyword table */
		s2[n2++] = s[i].kwd;	/* Copy pointers to visible ones */
	    } else {			/* Switches */
		sprintf(tmpbuf,		/* Make a copy that shows ":" if */
			"%s%s",		/* the switch takes an argument. */
			s[i].kwd,
			(s[i].flgs & CM_ARG) ? ":" : ""
			);
		makestr(&(s2[n2]),tmpbuf);
		if (s[i].flgs & CM_ARG) j++;
		n2++;
	    }
	    if (j > width)
	      width = j;
	}
	/* Column width */
	n = n2;
    }
    if (s2 && (b = (char *) malloc(cmd_cols + 1))) { /* Make a line buffer   */
	char * bx;
	bx = b + cmd_cols;
	width += (int)strlen(pre) + (int)strlen(post) + 2;
	cols = cmd_cols / width;	/* How many columns? */
	if (cols < 1) cols = 1;
	height = n / cols;		/* How long is each column? */
	if (n % cols) height++;		/* Add one for remainder, if any */

	for (i = 0; i < height; i++) {	    /* Loop for each row */
	    for (j = 0; j < cmd_cols; j++)  /* First fill row with blanks */
	      b[j] = SP;
	    for (j = 0; j < cols; j++) {    /* Loop for each column in row */
		k = i + (j * height);       /* Index of next keyword */
		if (k < n) {		    /* In range? */
		    pa = pre;
		    px = post;
		    p = s2[k];		    /* Point to verb name */
		    q = b + (j * width) + 1; /* Where to copy it to */
		    while ((q < bx) && (*q++ = *pa++)) ; /* Copy prefix */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *p++)) ;	 /* Copy filename */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *px++)) ; /* Copy suffix */
		    if (j < cols - 1) {
			q--;
			*q = SP;	/* Replace the space */
		    }
		}
	    }
	    p = b + cmd_cols - 1;	/* Last char in line */
	    while (*p-- == SP) ;	/* Trim */
	    *(p+2) = NUL;
	    printf("%s\n",b);		/* Print the line */
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore())		/* Do more-prompting... */
		  goto xkwdhelp;
		else
		  lc = 0;
	    }
	}
	/* printf("\n"); */		/* Blank line at end of report */
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    if (s[i].flgs & CM_INV)	/* Use original keyword table */
	      continue;			/* skipping invisible entries */
	    printf("%s%s%s\n",pre,s[i].kwd,post);
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore())		/* Do more-prompting... */
		  goto xkwdhelp;
		else
		  lc = 0;
	    }
	}
    }
  xkwdhelp:
    if (xhlp & 4) {
	if (tmpbuf) free(tmpbuf);
	for (i = 0; i < n; i++)
	  if (s2[i]) free(s2[i]);
    }
    if (s2) free(s2);			/* Free array copy */
    if (b) free(b);			/* Free line buffer */
    return;
}

/*  F I L H E L P  --  Given a file list, print names in columns.  */
/*
  Call with:
    n     - number of entries
    pre   - prefix to add to each filename
    post  - suffix to add to each filename
    off   - offset on first screenful, allowing room for introductory text
    cmdirflg - 1 if only directory names should be listed, 0 to list all files

  Arranges filenames in columns with width based on longest filename.
  Does "more?" prompting at end of screen.
  Uses global cmd_rows and cmd_cols for screen size.
*/

int
filhelp(n,pre,post,off,cmdirflg) int n, off; char *pre, *post; int cmdirflg; {
    char filbuf[CKMAXPATH + 1];		/* Temp buffer for one filename */
    int width = 0;
    int cols, height, i, j, k, lc, n2 = 0, rc = 0, itsadir = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;
#ifdef VMS
    char * cdp = zgtdir();
#endif /* VMS */

    if (n < 1) return(0);
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (s2 = (char **) malloc(n * sizeof(char *))) {
	for (i = 0; i < n; i++) {	/* Loop through filenames */
	    itsadir = 0;
	    s2[i] = NULL;		/* Initialize each pointer to NULL */
	    znext(filbuf);		/* Get next filename */
	    if (!filbuf[0])		/* Shouldn't happen */
	      break;
#ifdef COMMENT
	    itsadir = isdir(filbuf);	/* Is it a directory? */
	    if (cmdirflg && !itsadir)	/* No, listing directories only? */
	      continue;			/* So skip this one. */
#endif /* COMMENT */
#ifdef VMS
	    ckstrncpy(filbuf,zrelname(filbuf,cdp),CKMAXPATH);
#endif /* VMS */
	    j = strlen(filbuf);
#ifndef VMS
	    if (itsadir && j < CKMAXPATH - 1 && j > 0) {
		if (filbuf[j-1] != dirsep) {
		    filbuf[j++] = dirsep;
		    filbuf[j] = NUL;
		}
	    }
#endif /* VMS */
	    if (!(s2[n2] = malloc(j+1))) {
		printf("?Memory allocation failure\n");
		rc = -9;
		goto xfilhelp;
	    }
	    if (j <= CKMAXPATH) {
		strcpy(s2[n2],filbuf);
		n2++;
	    } else {
		printf("?Name too long - %s\n", filbuf);
		rc = -9;
		goto xfilhelp;
	    }
	    if (j > width)		/* Get width of widest one */
	      width = j;
	}
	n = n2;				/* How many we actually got */
    }
    sh_sort(s2,NULL,n,0,0,filecase);	/* Alphabetize the list */

    rc = 1;
    if (s2 && (b = (char *) malloc(cmd_cols + 1))) { /* Make a line buffer */
	char * bx;
	bx = b + cmd_cols;
	width += (int)strlen(pre) + (int)strlen(post) + 2;
	cols = cmd_cols / width;	/* How many columns? */
	if (cols < 1) cols = 1;
	height = n / cols;		/* How long is each column? */
	if (n % cols) height++;		/* Add one for remainder, if any */

	for (i = 0; i < height; i++) {	    /* Loop for each row */
	    for (j = 0; j < cmd_cols; j++)  /* First fill row with blanks */
	      b[j] = SP;
	    for (j = 0; j < cols; j++) {    /* Loop for each column in row */
		k = i + (j * height);       /* Index of next filename */
		if (k < n) {		    /* In range? */
		    pa = pre;
		    px = post;
		    p = s2[k];		               /* Point to filename */
		    q = b + (j * width) + 1;             /* and destination */
		    while ((q < bx) && (*q++ = *pa++)) ; /* Copy prefix */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *p++)) ;	 /* Copy filename */
		    q--;		                 /* Back up over NUL */
		    while ((q < bx) && (*q++ = *px++)) ; /* Copy suffix */
		    if (j < cols - 1) {
			q--;
			*q = SP;	/* Replace the space */
		    }
		}
	    }
	    p = b + cmd_cols - 1;	/* Last char in line */
	    while (*p-- == SP) ;	/* Trim */
	    *(p+2) = NUL;
	    printf("%s\n",b);		/* Print the line */
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore()) {	/* Do more-prompting... */
		    rc = 0;
		    goto xfilhelp;
		} else
		  lc = 0;
	    }
	}
	printf("\n");			/* Blank line at end of report */
	goto xfilhelp;
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    znext(filbuf);
	    if (!filbuf[0]) break;
	    printf("%s%s%s\n",pre,filbuf,post);
	    if (++lc > (cmd_rows - 2)) { /* Screen full? */
		if (!askmore()) {	 /* Do more-prompting... */
		    rc = 0;
		    goto xfilhelp;
		} else lc = 0;
	    }
	}
xfilhelp:
	if (b) free(b);
	for (i = 0; i < n2; i++)
	  if (s2[i]) free(s2[i]);
	if (s2) free(s2);
	return(rc);
    }
}

/*  C M S E T U P  --  Set up command buffers  */

#ifdef DCMDBUF
int
cmsetup() {
    if (!(cmdbuf = malloc(CMDBL + 4))) return(-1);
    if (!(savbuf = malloc(CMDBL + 4))) return(-1);
    savbuf[0] = '\0';
#ifdef OLDHELP
    if (!(hlpbuf = malloc(HLPBL + 4))) return(-1);
#endif /* OLDHELP */
    if (!(atmbuf = malloc(ATMBL + 4))) return(-1);
    if (!(atxbuf = malloc(CMDBL + 4))) return(-1);
    if (!(atybuf = malloc(ATMBL + 4))) return(-1);
    if (!(filbuf = malloc(ATMBL + 4))) return(-1);
    if (!(cmprom = malloc(PROMPTL + 4))) return(-1);
    if (!(cmprxx = malloc(PROMPTL + 4))) return(-1);
#ifdef CK_RECALL
    cmrini(cm_recall);
#endif /* CK_RECALL */
    return(0);
}
#endif /* DCMDBUF */

/*  C M S E T P  --  Set the program prompt.  */

VOID
cmsetp(s) char *s; {
    if (!s) s = "";
    ckstrncpy(cmprxx,s,PROMPTL);
    psetf = 1;                          /* Flag that prompt has been set. */
}

/*  C M S A V P  --  Save a copy of the current prompt.  */

VOID
#ifdef CK_ANSIC
cmsavp(char s[], int n)
#else
cmsavp(s,n) char s[]; int n;
#endif /* CK_ANSIC */
/* cmsavp */ {
    if (psetf)				/* But not if no prompt is set. */
      ckstrncpy(s,cmprxx,n);
}

int
cmgbrk() {
    return(brkchar);
}

int
cmgkwflgs() {
    return(cmkwflgs);
}

/*  P R O M P T  --  Issue the program prompt.  */

VOID
prompt(f) xx_strp f; {
    char *sx, *sy; int n;
#ifdef CK_SSL
    extern int ssl_active_flag, tls_active_flag;
#endif /* CK_SSL */
#ifdef OS2
    extern int display_demo;

    /* If there is a demo screen to be displayed, display it */
    if (display_demo && cmdsrc() == 0) {
        demoscrn();
        display_demo = 0;
    }
#endif /* OS2 */

    if (psetf == 0)			/* If no prompt set, set default. */
      cmsetp(dfprom);

    sx = cmprxx;			/* Unevaluated copy */
    if (f) {				/* If conversion function given */
	sy = cmprom;			/* Evaluate it */
	debug(F101,"prompt sx","",sx);
	debug(F101,"prompt sy","",sy);
	n = PROMPTL;
	if ((*f)(sx,&sy,&n) < 0)	/* If evaluation failed */
	  sx = cmprxx;			/* revert to unevaluated copy */
	else if (!*cmprom)		/* ditto if it came up empty */
	  sx = cmprxx;
	else
	  sx = cmprom;
    } else
      ckstrncpy(cmprom,sx,PROMPTL);
    cmprom[PROMPTL-1] = NUL;
    if (!*sx)				/* Don't print if empty */
      return;

#ifdef OSK
    fputs(sx, stdout);
#else
#ifdef MAC
    printf("%s", sx);
#else
    printf("\r%s",sx);			/* Print the prompt. */
#ifdef CK_SSL
    if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
      fflush(stdout);			/* Now! */
#endif /* MAC */
#endif /* OSK */
}

#ifndef NOSPL
VOID
pushcmd(s) char * s; {			/* For use with IF command. */
    if (!s) s = np;
    ckstrncpy(savbuf,s,CMDBL);		/* Save the dependent clause,  */
    cmres();				/* and clear the command buffer. */
    debug(F110, "pushcmd savbuf", savbuf, 0);
}

VOID
pushqcmd(s) char * s; {			/* For use with ELSE command. */
    char c, * p = savbuf;		/* Dest */
    if (!s) s = np;			/* Source */
    while (*s) {			/* Get first nonwhitespace char */
	if (*s != SP)
	  break;
	else
	  s++;
    }
    if (*s != '{') {			/* If it's not "{" */
	pushcmd(s);			/* do regular pushcmd */
	return;
    }
    while (c = *s++) {			/* Otherwise insert quotes */
	if (c == CMDQ)
	  *p++ = CMDQ;
	*p++ = c;
    }
    cmres();				/* and clear the command buffer. */
    debug(F110, "pushqcmd savbuf", savbuf, 0);
}
#endif /* NOSPL */

#ifdef COMMENT
/* no longer used... */
VOID
popcmd() {
    ckstrncpy(cmdbuf,savbuf,CMDBL);	/* Put back the saved material */
    *savbuf = '\0';			/* and clear the save buffer */
    cmres();
}
#endif /* COMMENT */

/*  C M R E S  --  Reset pointers to beginning of command buffer.  */

VOID
cmres() {
    inword = 0;				/* We're not in a word */
    cc = 0;				/* Character count is zero */

/* Initialize pointers */

    pp = cmdbuf;			/* Beginning of current field */
    bp = cmdbuf;			/* Current position within buffer */
    np = cmdbuf;			/* Where to start next field */

    cmfldflgs = 0;
    cmflgs = -5;                        /* Parse not yet started. */
    ungw = 0;				/* Don't need to unget a word. */
}

/*  C M I N I  --  Clear the command and atom buffers, reset pointers.  */

/*
The argument specifies who is to echo the user's typein --
  1 means the cmd package echoes
  0 somebody else (system, front end, terminal) echoes
*/
VOID
cmini(d) int d; {
#ifdef DCMDBUF
    if (!atmbuf)
      if (cmsetup()<0)
	fatal("fatal error: unable to allocate command buffers");
#endif /* DCMDBUF */
#ifdef USE_MEMCPY
    memset(cmdbuf,0,CMDBL);
    memset(atmbuf,0,ATMBL);
#else
    for (bp = cmdbuf; bp < cmdbuf+CMDBL; bp++) *bp = NUL;
    for (bp = atmbuf; bp < atmbuf+ATMBL; bp++) *bp = NUL;
#endif /* USE_MEMCPY */

    *atmbuf = *savbuf = *atxbuf = *atybuf = *filbuf = NUL;
#ifdef OLDHELP
    *hlpbuf = NUL;
#endif /* OLDHELP */
    blocklvl = 0;		/* Block level is 0 */
    linebegin = 1;		/* And we're at the beginning of a line */
    dpx = d;			/* Make a global copy of the echo flag */
    debug(F101,"cmini dpx","",dpx);
    crflag = 0;
    qmflag = 0;
    esflag = 0;
#ifdef CK_RECALL
    no_recall = 0;
#endif /* CK_RECALL */
    cmres();
}

#ifndef NOSPL
/* The following bits are to allow the command package to call itself */
/* in the middle of a parse.  To do this, begin by calling cmpush, and */
/* end by calling cmpop. */

#ifdef DCMDBUF
struct cmp {
    int i[5];				/* stack for integers */
    char *c[3];				/* stack for pointers */
    char *b[8];				/* stack for buffer contents */
};
struct cmp *cmp = 0;
#else
int cmp_i[CMDDEP+1][5];			/* Stack for integers */
char *cmp_c[CMDDEP+1][5];		/* for misc pointers */
char *cmp_b[CMDDEP+1][7];		/* for buffer contents pointers */
#endif /* DCMDBUF */

int cmddep = -1;			/* Current stack depth */

int
cmpush() {				/* Save the command environment */
    char *cp;				/* Character pointer */

    if (cmddep >= CMDDEP)		/* Enter a new command depth */
      return(-1);
    cmddep++;
    debug(F101,"&cmpush","",cmddep);

#ifdef DCMDBUF
    /* allocate memory for cmp if not already done */
    if (!cmp && !(cmp = (struct cmp *) malloc(sizeof(struct cmp)*(CMDDEP+1))))
      fatal("cmpush: no memory for cmp");
    cmp[cmddep].i[0] = cmflgs;		/* First do the global ints */
    cmp[cmddep].i[1] = cmfsav;
    cmp[cmddep].i[2] = atxn;
    cmp[cmddep].i[3] = ungw;

    cmp[cmddep].c[0] = bp;		/* Then the global pointers */
    cmp[cmddep].c[1] = pp;
    cmp[cmddep].c[2] = np;
#else
    cmp_i[cmddep][0] = cmflgs;		/* First do the global ints */
    cmp_i[cmddep][1] = cmfsav;
    cmp_i[cmddep][2] = atxn;
    cmp_i[cmddep][3] = ungw;

    cmp_c[cmddep][0] = bp;		/* Then the global pointers */
    cmp_c[cmddep][1] = pp;
    cmp_c[cmddep][2] = np;
#endif /* DCMDBUF */

    /* Now the buffers themselves.  A lot of repititious code... */

#ifdef DCMDBUF
    cp = malloc((int)strlen(cmdbuf)+1);	/* 0: Command buffer */
    if (cp) strcpy(cp,cmdbuf);
    cmp[cmddep].b[0] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(savbuf)+1);	/* 1: Save buffer */
    if (cp) strcpy(cp,savbuf);
    cmp[cmddep].b[1] = cp;
    if (cp == NULL) return(-1);

#ifdef OLDHELP
    cp = malloc((int)strlen(hlpbuf)+1);	/* 2: Help string buffer */
    if (cp) strcpy(cp,hlpbuf);
    cmp[cmddep].b[2] = cp;
    if (cp == NULL) return(-1);
#else
    cmp[cmddep].b[2] = NULL;
#endif /* OLDHELP */

    cp = malloc((int)strlen(atmbuf)+1);	/* 3: Atom buffer */
    if (cp) strcpy(cp,atmbuf);
    cmp[cmddep].b[3] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atxbuf)+1);	/* 4: Expansion buffer */
    if (cp) strcpy(cp,atxbuf);
    cmp[cmddep].b[4] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atybuf)+1);	/* 5: Atom buffer copy */
    if (cp) strcpy(cp,atybuf);
    cmp[cmddep].b[5] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(filbuf)+1);	/* 6: File name buffer */
    if (cp) strcpy(cp,filbuf);
    cmp[cmddep].b[6] = cp;
    if (cp == NULL) return(-1);
#else
    cp = malloc((int)strlen(cmdbuf)+1);	/* 0: Command buffer */
    if (cp) strcpy(cp,cmdbuf);
    cmp_b[cmddep][0] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(savbuf)+1);	/* 1: Save buffer */
    if (cp) strcpy(cp,savbuf);
    cmp_b[cmddep][1] = cp;
    if (cp == NULL) return(-1);

#ifdef OLDHELP
    cp = malloc((int)strlen(hlpbuf)+1);	/* 2: Help string buffer */
    if (cp) strcpy(cp,hlpbuf);
    cmp_b[cmddep][2] = cp;
    if (cp == NULL) return(-1);
#else
    cmp_b[cmddep][2] = NULL;
#endif /* OLDHELP */

    cp = malloc((int)strlen(atmbuf)+1);	/* 3: Atom buffer */
    if (cp) strcpy(cp,atmbuf);
    cmp_b[cmddep][3] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atxbuf)+1);	/* 4: Expansion buffer */
    if (cp) strcpy(cp,atxbuf);
    cmp_b[cmddep][4] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(atybuf)+1);	/* 5: Atom buffer copy */
    if (cp) strcpy(cp,atybuf);
    cmp_b[cmddep][5] = cp;
    if (cp == NULL) return(-1);

    cp = malloc((int)strlen(filbuf)+1);	/* 6: File name buffer */
    if (cp) strcpy(cp,filbuf);
    cmp_b[cmddep][6] = cp;
    if (cp == NULL) return(-1);
#endif /* DCMDBUF */

    cmini(dpx);				/* Initize the command parser */
    return(0);
}

int
cmpop() {				/* Restore the command environment */
    debug(F101,"&cmpop","",cmddep);
    if (cmddep < 0) return(-1);		/* Don't pop too much! */

#ifdef DCMDBUF
    cmflgs = cmp[cmddep].i[0];		/* First do the global ints */
    cmfsav = cmp[cmddep].i[1];
    atxn = cmp[cmddep].i[2];
    ungw = cmp[cmddep].i[3];

    bp = cmp[cmddep].c[0];		/* Then the global pointers */
    pp = cmp[cmddep].c[1];
    np = cmp[cmddep].c[2];
#else
    cmflgs = cmp_i[cmddep][0];		/* First do the global ints */
    cmfsav = cmp_i[cmddep][1];
    atxn = cmp_i[cmddep][2];
    ungw = cmp_i[cmddep][3];

    bp = cmp_c[cmddep][0];		/* Then the global pointers */
    pp = cmp_c[cmddep][1];
    np = cmp_c[cmddep][2];
#endif /* DCMDBUF */

    /* Now the buffers themselves. */
    /* Note: strncpy(), not ckstrncpy() -- Here we WANT the NUL padding... */

#ifdef DCMDBUF
    if (cmp[cmddep].b[0]) {

	strncpy(cmdbuf,cmp[cmddep].b[0],CMDBL); /* 0: Command buffer */
	free(cmp[cmddep].b[0]);
	cmp[cmddep].b[0] = NULL;
    }
    if (cmp[cmddep].b[1]) {
	strncpy(savbuf,cmp[cmddep].b[1],CMDBL); /* 1: Save buffer */
	free(cmp[cmddep].b[1]);
	cmp[cmddep].b[1] = NULL;
    }
#ifdef OLDHELP
    if (cmp[cmddep].b[2]) {
	strncpy(hlpbuf,cmp[cmddep].b[2],HLPBL); /* 2: Help buffer */
	free(cmp[cmddep].b[2]);
	cmp[cmddep].b[2] = NULL;
    }
#endif /* OLDHELP */
    if (cmp[cmddep].b[3]) {
	strncpy(atmbuf,cmp[cmddep].b[3],ATMBL); /* 3: Atomic buffer! */
	free(cmp[cmddep].b[3]);
	cmp[cmddep].b[3] = NULL;
    }
    if (cmp[cmddep].b[4]) {
	strncpy(atxbuf,cmp[cmddep].b[4],ATMBL); /* 4: eXpansion buffer */
	free(cmp[cmddep].b[4]);
	cmp[cmddep].b[4] = NULL;
    }
    if (cmp[cmddep].b[5]) {
	strncpy(atybuf,cmp[cmddep].b[5],ATMBL); /* 5: Atom buffer copY */
	free(cmp[cmddep].b[5]);
	cmp[cmddep].b[5] = NULL;
    }
    if (cmp[cmddep].b[6]) {
	strncpy(filbuf,cmp[cmddep].b[6],ATMBL); /* 6: Filename buffer */
	free(cmp[cmddep].b[6]);
	cmp[cmddep].b[6] = NULL;
    }
#else
    if (cmp_b[cmddep][0]) {
	strncpy(cmdbuf,cmp_b[cmddep][0],CMDBL); /* 0: Command buffer */
	free(cmp_b[cmddep][0]);
	cmp_b[cmddep][0] = NULL;
    }
    if (cmp_b[cmddep][1]) {
	strncpy(savbuf,cmp_b[cmddep][1],CMDBL); /* 1: Save buffer */
	free(cmp_b[cmddep][1]);
	cmp_b[cmddep][1] = NULL;
    }
#ifdef OLDHELP
    if (cmp_b[cmddep][2]) {
	strncpy(hlpbuf,cmp_b[cmddep][2],HLPBL); /* 2: Help buffer */
	free(cmp_b[cmddep][2]);
	cmp_b[cmddep][2] = NULL;
    }
#endif /* OLDHELP */
    if (cmp_b[cmddep][3]) {
	strncpy(atmbuf,cmp_b[cmddep][3],ATMBL); /* 3: Atomic buffer! */
	free(cmp_b[cmddep][3]);
	cmp_b[cmddep][3] = NULL;
    }
    if (cmp_b[cmddep][4]) {
	strncpy(atxbuf,cmp_b[cmddep][4],ATMBL); /* 4: eXpansion buffer */
	free(cmp_b[cmddep][4]);
	cmp_b[cmddep][4] = NULL;
    }
    if (cmp_b[cmddep][5]) {
	strncpy(atybuf,cmp_b[cmddep][5],ATMBL); /* 5: Atom buffer copY */
	free(cmp_b[cmddep][5]);
	cmp_b[cmddep][5] = NULL;
    }
    if (cmp_b[cmddep][6]) {
	strncpy(filbuf,cmp_b[cmddep][6],ATMBL); /* 6: Filename buffer */
	free(cmp_b[cmddep][6]);
	cmp_b[cmddep][6] = NULL;
    }
#endif /* DCMDBUF */

    cmddep--;				/* Rise, rise */
    debug(F101,"&cmpop","",cmddep);
    return(cmddep);
}
#endif /* NOSPL */

#ifdef COMMENT
VOID
stripq(s) char *s; {                    /* Function to strip '\' quotes */
    char *t;
    while (*s) {
        if (*s == CMDQ) {
            for (t = s; *t != '\0'; t++) *t = *(t+1);
        }
        s++;
    }
}
#endif /* COMMENT */

/* Convert tabs to spaces, one for one */
VOID
untab(s) char *s; {
    while (*s) {
	if (*s == HT) *s = SP;
	s++;
    }
}

/*  C M N U M  --  Parse a number in the indicated radix  */

/*
 The only radix allowed in unquoted numbers is 10.
 Parses unquoted numeric strings in base 10.
 Parses backslash-quoted numbers in the radix indicated by the quote:
   \nnn = \dnnn = decimal, \onnn = octal, \xnn = Hexadecimal.
 If these fail, then if a preprocessing function is supplied, that is applied
 and then a second attempt is made to parse an unquoted decimal string.
 And if that fails, the preprocessed string is passed to an arithmetic
 expression evaluator.

 Returns:
   -3 if no input present when required,
   -2 if user typed an illegal number,
   -1 if reparse needed,
    0 otherwise, with argument n set to the number that was parsed
*/
int
cmnum(xhlp,xdef,radix,n,f) char *xhlp, *xdef; int radix, *n; xx_strp f; {
    int x; char *s, *zp, *zq;
    char lbrace, rbrace;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    if (cmfldflgs & 1) {
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
    if (radix != 10) {                  /* Just do base 10 */
        printf("cmnum: illegal radix - %d\n",radix);
        return(-1);
    } /* Easy to add others but there has never been a need for it. */
    x = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F101,"cmnum: cmfld","",x);
    if (x < 0) return(x);		/* Parse a field */
    zp = atmbuf;
/*
  Edit 192 - Allow any number field to be braced.  This lets us include
  spaces in expressions, but perhaps more important lets us have user-defined
  functions in numeric fields.
*/
    if (*zp == lbrace) {		/* Braced field, strip braces */
	x = (int) strlen(atmbuf);
	if (x > 0) {			/* The "if" is to shut up optimizers */
	    if (*(atmbuf+x-1) == rbrace) {
		*(atmbuf+x-1) = NUL;	/* that complain about a possible */
		zp++;			/* reference to atbmbuf[-1] even */
	    }
	}				/* though we know that x > 0. */
    }
    if (chknum(zp)) {			/* Check for decimal number */
        *n = atoi(zp);			/* Got one, we're done. */
	debug(F101,"cmnum 1st chknum ok","",*n);
        return(0);
    } else if ((x = xxesc(&zp)) > -1) {	/* Check for backslash escape */

#ifndef OS2
	*n = x;
#else
	*n = wideresult;
#endif /* OS2 */

	debug(F101,"cmnum xxesc ok","",*n);
	return(*zp ? -2 : 0);
    } else if (f) {			/* If conversion function given */
	zq = atxbuf;			/* Try that */
	atxn = CMDBL;
	if ((*f)(zp,&zq,&atxn) < 0)	/* Convert */
	  return(-2);
	zp = atxbuf;
    }
    debug(F110,"cmnum zp 1",zp,0);
    if (!*zp) zp = xdef;		/* Result empty, substitute default */
    debug(F110,"cmnum zp 2",zp,0);
    if (chknum(zp)) {			/* Check again for decimal number */
        *n = atoi(zp);			/* Got one, we're done. */
	debug(F101,"cmnum 2nd chknum ok","",*n);
        return(0);
#ifndef NOSPL
    }  else if ((x = xxesc(&zp)) > -1) { /* Check for backslash escape */
#ifndef OS2
	*n = x;
#else
	*n = wideresult;
#endif /* OS2 */
	debug(F101,"cmnum xxesc 2 ok","",*n);
	return(*zp ? -2 : 0);
    } else if (f) {			/* Not numeric, maybe an expression */
	char * p;
	p = evala(zp);
	if (chknum(p)) {
	    *n = atoi(p);
	    debug(F101,"cmnum exp eval ok","",*n);
	    return(0);
	} else return(-2);
#endif /* NOSPL */
    } else {				/* Not numeric */
	return(-2);
    }
}

#ifdef CKCHANNELIO
extern int z_error;
#endif /* CKCHANNELIO */

/*  C M O F I  --  Parse the name of an output file  */

/*
 Depends on the external function zchko(); if zchko() not available, use
 cmfld() to parse output file names.

 Returns:
   -9 like -2, except message already printed,
   -3 if no input present when required,
   -2 if permission would be denied to create the file,
   -1 if reparse needed,
    0 or 1 if file can be created, with xp pointing to name.
    2 if given the name of an existing directory.
*/
int
cmofi(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int x; char *s, *zq;
#ifdef DOCHKVAR
    int tries;
#endif /* DOCHKVAR */
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */

    cmfldflgs = 0;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    if (*xhlp == NUL) xhlp = "Output file";
    *xp = "";

    x = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F111,"cmofi cmfld returns",s,x);
    if (x < 0)
      return(x);

    if (*s == '{') {			/* Strip enclosing braces */
	int n;
	n = strlen(s);
	if (s[n-1] == '}') {
	    s[n-1] = NUL;
	    s++;
	}
    }
    debug(F110,"cmofi 1.5",s,0);

#ifdef DOCHKVAR
    tries = 0;
    {
	char *p = s;
    /*
      This is really ugly.  If we skip conversion the first time through,
      then variable names like \%a will be used as filenames (e.g. creating
      a file called %A in the root directory).  If we DON'T skip conversion
      the first time through, then single backslashes used as directory
      separators in filenames will be misinterpreted as variable lead-ins.
      So we prescan to see if it has any variable references.  But this
      module is not supposed to know anything about variables, functions,
      etc, so this code does not really belong here, but rather it should
      be at the same level as zzstring().
    */
/*
  Hmmm, this looks a lot like chkvar() except it that includes \nnn number
  escapes.  But why?  This makes commands like "mkdir c:\123" impossible.
  And in fact, "mkdir c:\123" creates a directory called "c:{".  What's worse,
  rmdir(), which *does* call chkvar(), won't let us remove it.  So let's at
  least try making cmofi() symmetrical with cmifi()...
*/
#ifdef COMMENT
	char * q;
	while ( (tries == 0) && (p = strchr(p,CMDQ)) ) {
	    q = *(p+1);			/* Char after backslash */
	    if (!q)			/* None, quit */
	      break;
	    if (isupper(q))		/* If letter, convert to lowercase */
	      q = tolower(q);
	    if (isdigit(q)) {		/* If it's a digit, */
		tries = 1;		/* assume it's a backslash code  */
		break;
	    }
	    switch (q) {
	      case CMDQ:		/* Double backslash */
		tries = 1;		/* so call the conversion function */
		break;
	      case '%':			/* Variable or array reference */
	      case '&':			/* must be followed by letter */
		if (isalpha(*(p+2)) || (*(p+2) >= '0' && *(p+2) <= '9'))
		  tries = 1;
		break;
	      case 'm': case 'v': case '$': /* \m(), \v(), \$() */
		if (*(p+2) == '(')
		  if (strchr(p+2,')'))
		    tries = 1;
		break;
	      case 'f':			/* \Fname() */
		if (strchr(p+2,'('))
		  if (strchr(p+2,')'))
		      tries = 1;
		break;
	      case '{':			/* \{...} */
		if (strchr(p+2,'}'))
		  tries = 1;
		break;
	      case 'd': case 'o':	/* Decimal or Octal number */
	        if (isdigit(*(p+2)))
		  tries = 1;
		break;
	      case 'x':			/* Hex number */
		if (isdigit(*(p+2)) ||
		    ((*(p+2) >= 'a' && *(p+2) <= 'f') ||
		     ((*(p+2) >= 'A' && *(p+2) <= 'F'))))
		  tries = 1;
	      default:
		break;
	    }
	    p++;
	}
#else
#ifndef NOSPL
	if (f) {			/* If a conversion function is given */
	    char *s = p;		/* See if there are any variables in */
	    while (*s) {		/* the string and if so, expand them */
		if (chkvar(s)) {
		    tries = 1;
		    break;
		}
		s++;
	    }
	}
#endif /* NOSPL */
#endif /* COMMENT */
    }
#ifdef OS2
o_again:
#endif /* OS2 */
    if (tries == 1)
#endif /* DOCHKVAR */
    if (f) {				/* If a conversion function is given */
	zq = atxbuf;			/* do the conversion. */
	atxn = CMDBL;
	if ((x = (*f)(s,&zq,&atxn)) < 0)
	  return(-2);
	s = atxbuf;
	if (!*s)			/* Result empty, substitute default */
	  s = xdef;
    }
    debug(F111,"cmofi 2",s,x);

#ifdef DTILDE
    dirp = tilde_expand(s);		/* Expand tilde, if any, */
    if (*dirp != '\0') {		/* right in the atom buffer. */
	if (setatm(dirp,1) < 0) {
	    printf("?Name too long\n");
	    return(-9);
	}
    }
    s = atmbuf;
    debug(F110,"cmofi 3",s,0);
#endif /* DTILDE */

    if (iswild(s)) {
        printf("?Wildcards not allowed - %s\n",s);
        return(-2);
    }
    debug(F110,"cmofi 4",s,0);

#ifdef CK_TMPDIR
    /* isdir() function required for this! */
    if (isdir(s)) {
	debug(F110,"cmofi 5: is directory",s,0);
        *xp = s;
	return(2);
    }
#endif /* CK_TMPDIR */

    if (strcmp(s,CTTNAM) && (zchko(s) < 0)) { /* OK to write to console */
#ifdef COMMENT
#ifdef OS2
/*
  We don't try again because we already prescanned the string to see if
  if it contained anything that could be used by zzstring().
*/
	if (tries++ < 1)
	  goto o_again;
#endif /* OS2 */
#endif /* COMMENT */
/*
  Note: there are certain circumstances where zchko() can give a false
  positive, so don't rely on it to catch every conceivable situation in
  which the given output file can't be created.  In other words, we print
  a message and fail here if we KNOW the file can't be created.  If we
  succeed but the file can't be opened, the code that tries to open the file
  has to print a message.
*/
	debug(F110,"cmofi 6: failure",s,0);
        printf("?Write permission denied - %s\n",s);
#ifdef CKCHANNELIO
	z_error = FX_ACC;
#endif /* CKCHANNELIO */
        return(-9);
    } else {
	debug(F110,"cmofi 7: ok",s,0);
        *xp = s;
        return(x);
    }
}

/*  C M I F I  --  Parse the name of an existing file  */

/*
 This function depends on the external functions:
   zchki()  - Check if input file exists and is readable.
   zxpand() - Expand a wild file specification into a list.
   znext()  - Return next file name from list.
 If these functions aren't available, then use cmfld() to parse filenames.
*/
/*
 Returns
   -4 EOF
   -3 if no input present when required,
   -2 if file does not exist or is not readable,
   -1 if reparse needed,
    0 or 1 otherwise, with:
        xp pointing to name,
        wild = 1 if name contains '*' or '?', 0 otherwise.
*/

/*
   C M I O F I  --  Parse an input file OR the name of a nonexistent file.

   Use this when an existing file is wanted (so we get help, completion, etc),
   but if a file of the given name does not exist, the name of a new file is
   accepted.  For example, with the EDIT command (edit an existing file, or
   create a new file).  Returns -9 if file does not exist.  It is up to the
   caller to check creatability.
*/
static int nomsg = 0;
int
cmiofi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    int msgsave, x;
    msgsave = nomsg;
    nomsg = 1;
    x = cmifi2(xhlp,xdef,xp,wild,0,NULL,f,0);
    nomsg = msgsave;
    return(x);
}

int
cmifi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    return(cmifi2(xhlp,xdef,xp,wild,0,NULL,f,0));
}
/*
  cmifip() is called when we want to supply a path or path list to search
  in case the filename that the user gives is (a) not absolute, and (b) can't
  be found as given.  The path string can be the name of a single directory,
  or a list of directories separated by the PATHSEP character, defined in
  ckucmd.h.  Look in ckuusr.c and ckuus3.c for examples of usage.
*/
int
cmifip(xhlp,xdef,xp,wild,d,path,f)
    char *xhlp,*xdef,**xp; int *wild, d; char * path; xx_strp f; {
    return(cmifi2(xhlp,xdef,xp,wild,0,path,f,0));
}

/*  C M D I R  --  Parse a directory name  */

/*
 This function depends on the external functions:
   isdir(s)  - Check if string s is the name of a directory
   zchki(s)  - Check if input file s exists and what type it is.
 If these functions aren't available, then use cmfld() to parse dir names.

 Returns
   -9 For all sorts of reasons, after printing appropriate error message.
   -4 EOF
   -3 if no input present when required,
   -2 if out of space or other internal error,
   -1 if reparse needed,
    0 or 1, with xp pointing to name, if directory specified,
*/
int
cmdir(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int wild;
    return(cmifi2(xhlp,xdef,xp,&wild,0,NULL,f,1));
}

/* Like CMDIR but includes PATH search */

int
cmdirp(xhlp,xdef,xp,path,f) char *xhlp, *xdef, **xp; char * path; xx_strp f; {
    int wild;
    return(cmifi2(xhlp,xdef,xp,&wild,0,path,f,1));
}

/*
  cmifi2() is the base filename parser called by cmifi, cmifip, cmdir, etc.
  Use it directly when you also want to parse a directory or device
  name as an input file, as in the DIRECTORY command.  Call with:
    xhlp  -- help message on ?
    xdef  -- default response
    xp    -- pointer to result (in our space, must be copied from here)
    wild  -- flag set upon return to indicate if filespec was wild
    d     -- 0 to parse files, 1 to parse files or directories
    path  -- search path for files
    f     -- pointer to string processing function (e.g. to evaluate variables)
    dirflg -- 1 to parse *only* directories, 0 otherwise
*/
int
cmifi2(xhlp,xdef,xp,wild,d,path,f,dirflg)
    char *xhlp,*xdef,**xp; int *wild, d; char * path; xx_strp f; int dirflg; {
    extern int recursive, diractive;
    int i, x, itsadir, xc, expanded = 0, nfiles = 0;
    long y;
    char *sp = NULL, *zq, *np = NULL;
    char *sv = NULL, *p = NULL;
#ifdef DTILDE
    char *dirp;
#endif /* DTILDE */

#ifndef NOPARTIAL
#ifndef OS2
#ifdef OSK
    /* This large array is dynamic for OS-9 -- should do for others too... */
    extern char **mtchs;
#else
#ifdef UNIX
    /* OK, for UNIX too */
    extern char **mtchs;
#else
#ifdef VMS
    extern char **mtchs;
#else
    extern char *mtchs[];
#endif /* VMS */
#endif /* UNIX */
#endif /* OSK */
#endif /* OS2 */
#endif /* NOPARTIAL */

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    cmfldflgs = 0;
    if (path)
      if (!*path)
	path = NULL;
    if (path) {				/* Make a copy we can poke */
	x = strlen(path);
	np = (char *) malloc(x + 1);
	if (np) {
	    strcpy(np, path);
	    path = sp = np;
	}
    }
    debug(F110,"cmifi2 path",path,0);

    ckstrncpy(cmdefault,xdef,CMDEFAULT);	/* Copy default */
    xdef = cmdefault;

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";				/* Pointer to result string */
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
#ifdef BS_DIRSEP
	dirnamflg = 1;
        x = gtword(0);			/* No, get a word */
	dirnamflg = 0;
#else
        x = gtword(0);                  /* No, get a word */
#endif /* BS_DIRSEP */
    } else {				/* If so, use default, if any. */
        if (setatm(xdef,1) < 0) {
	    printf("?Default name too long\n");
	    if (np) free(np);
	    return(-9);
	}
    }
  i_path:
    *xp = atmbuf;                       /* Point to result. */

    while (1) {
        xc += cc;                       /* Count this character. */
        debug(F111,"cmifi gtword",atmbuf,xc);
	debug(F101,"cmifi switch x","",x);
        switch (x) {			/* x = gtword() return code */
	  case -10:
	    if (gtimer() > timelimit) {
#ifdef IKSD
                extern int inserver;
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		if (!quiet) printf("?Timed out\n");
		return(-10);
	    } else {
		x = gtword(0);
		continue;
	    }
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -2:			/* Out of space. */
	  case -1:			/* Reparse needed */
	    if (np) free(np);
	    return(x);
	  case 0:			/* SP or NL */
	  case 1:
	    if (xc == 0)		/* If no input... */
	      *xp = xdef;		/* substitute the default */
	    if (**xp == NUL) {		/* If field still empty return -3. */
		if (np) free(np);
		return(-3);
	    }
	    *xp = brstrip(*xp);		/* Strip braces */
	    debug(F110,"cmifi brstrip",*xp,0);
#ifndef NOSPL
	    if (f) {			/* If a conversion function is given */
#ifdef DOCHKVAR
		char *s = *xp;		/* See if there are any variables in */
		int x;
		while (*s) {		/* the string and if so, expand them */
		    if (chkvar(s)) {
#endif /* DOCHKVAR */
			zq = atxbuf;
			atxn = CMDBL;
			if ((*f)(*xp,&zq,&atxn) < 0) {
			    if (np) free(np);
			    return(-2);
			}
			*xp = atxbuf;
			if (!atxbuf[0])
			  *xp = xdef;
#ifdef DOCHKVAR
			break;
		    }
		    s++;
		}
#endif /* DOCHKVAR */
	    }
#endif /* NOSPL */

#ifdef DTILDE
	    dirp = tilde_expand(*xp);	/* Expand tilde, if any, */
	    if (*dirp != '\0') {	/* in the atom buffer. */
		if (setatm(dirp,1) < 0) {
		    printf("Expanded name too long\n");
		    if (np) free(np);
		    return(-9);
		}
	    }
	    *xp = atmbuf;
#endif /* DTILDE */
	    debug(F110,"cmifi tilde_expand",*xp,0);
	    if (!sv) {			/* Only do this once */
		sv = malloc((int)strlen(*xp)+1); /* Make a safe copy */
		if (!sv) {
		    printf("?cmifi: malloc error\n");
		    if (np) free(np);
		    return(-9);
		}
		strcpy(sv,*xp);
		debug(F110,"cmifi sv",sv,0);
	    }

/* This is to get around "cd /" failing because "too many directories match" */

	    expanded = 0;		/* Didn't call zxpand */
#ifdef datageneral
	    debug(F110,"cmifi isdir 1",*xp,0);
	    {
		int y; char *s;
		s = *xp;
		y = strlen(s);
		if (y > 1 &&
		    (s[y-1] == ':' ||
		     s[y-1] == '^' ||
		     s[y-1] == '=')
		    )
		  s[y-1] = NUL;
	    }
	    debug(F110,"cmifi isdir 2",*xp,0);
#endif /*  datageneral */

#ifdef VMS
	    if (dirflg) {
		if (!strcmp(*xp,"..")) { /* For UNIXers... */
		    setatm("-",0);
		    *xp = atmbuf;
		} else if (!strcmp(*xp,".")) {
		    setatm("[]",0);
		    *xp = atmbuf;
		}
	    }
#endif /* VMS */
	    itsadir = isdir(*xp);	/* Is it a directory? */
	    debug(F111,"cmifi itsadir",*xp,itsadir);
#ifdef VMS
	    /* If they said "blah" where "blah.dir" is a directory... */
	    /* change it to [.blah]. */
	    if (!itsadir) {
		char tmpbuf[600];
		int flag = 0; char c, * p;
		p = *xp;
		while ((c = *p++) && !flag)
		  if (ckstrchr(".[]:*?<>",c))
		    flag = 1;
		debug(F111,"cmifi VMS dirname flag",*xp,flag);
		if (!flag) {
		    sprintf(tmpbuf,"[.%s]",*xp);
		    itsadir = isdir(tmpbuf);
		    if (itsadir) {
			setatm(tmpbuf,0);
			*xp = atmbuf;
		    }
		    debug(F111,"cmifi VMS dirname flag itsadir",*xp,itsadir);
		}
	    } else if (itsadir == 1 && *(xp[0]) == '.' && *(xp[1])) {
		char *p;
		if (p = malloc(cc + 4)) {
		    sprintf(p,"[%s]",*xp);
		    setatm(p,0);
		    *xp = atmbuf;
		    debug(F110,"cmdir .foo",*xp,0);
		    free(p);
		}
	    } else if (itsadir == 2 && !diractive) {
		int x;			/* [FOO]BAR.DIR instead of [FOO.BAR] */
		char *p;
		p = malloc(cc + 4);
		if (p) {
		    x = cvtdir(*xp,p);	/* Convert to [FOO.BAR] */
		    if (x > 0) {
			setatm(p,0);
			*xp = atmbuf;
			debug(F110,"cmdir cvtdir",*xp,0);
		    }
		    free(p);
		}
	    }
#endif /* VMS */

	    if (dirflg) {		/* Parsing a directory name? */
		/* Yes, does it contain wildcards? */
		if (iswild(*xp) ||
		    diractive && (!strcmp(*xp,".")  || !strcmp(*xp,".."))
		    ) {
		    nzxopts = ZX_DIRONLY; /* Match only directory names */
		    if (matchdot)  nzxopts |= ZX_MATCHDOT;
		    if (recursive) nzxopts |= ZX_RECURSE;
		    y = nzxpand(*xp,nzxopts);
		    nfiles = y;
		    expanded = 1;
		} else {
#ifdef VMS
/*
  This is to allow (e.g.) "cd foo", where FOO.DIR;1 is in the
  current directory.
*/
		    debug(F111,"cmdir itsadir",*xp,itsadir);
		    if (!itsadir) {
			char *s;
			int n;
			s = *xp;
			n = strlen(s);
			if (n > 0 &&
#ifdef COMMENT
			    *s != '[' && s[n-1] != ']' &&
			    *s != '<' && s[n-1] != '>' &&
#else
			    ckindex("[",s,0,0,1) == 0 &&
			    ckindex("<",s,0,0,1) == 0 &&
#endif /* COMMENT */
			    s[n-1] != ':') {
			    char * dirbuf = NULL;
			    dirbuf = (char *)malloc(n+4);
			    if (dirbuf) {
				if (*s == '.')
				  sprintf(dirbuf,"[%s]",s);
				else
				  sprintf(dirbuf,"[.%s]",s);
				itsadir = isdir(dirbuf);
				debug(F111,"cmdir dirbuf",dirbuf,itsadir);
				if (itsadir) {
				    setatm(dirbuf,0);
				    *xp = atmbuf;
				    debug(F110,"cmdir new *xp",*xp,0);
				}
				free(dirbuf);
			    }

/* This is to allow CDPATH to work in VMS... */

			} else if (n > 0) {
			    char * p; int i, j, k, d;
			    char rb[2] = "]";
			    if (p = malloc(x + 8)) {
				strcpy(p,*xp);
				i = ckindex(".",p,-1,1,1);
				d = ckindex(".dir",p,0,0,0);
				j = ckindex("]",p,-1,1,1);
				if (j == 0) {
				    j = ckindex(">",p,-1,1,1);
				    rb[0] = '>';
				}
				k = ckindex(":",p,-1,1,1);
				if (i < j || i < k) i = 0;
				if (d < j || d < k) d = 0;
				/* Change [FOO]BAR or [FOO]BAR.DIR */
				/* to [FOO.BAR] */
				if (j > 0 && j < n) {
				    p[j-1] = '.';
				    if (d > 0) p[d-1] = NUL;
				    strcat(p,rb);
				    debug(F110,"cmdir xxx",p,0);
				}
				itsadir = isdir(p);
				debug(F111,"cmdir p",p,itsadir);
				if (itsadir) {
				    setatm(p,0);
				    *xp = atmbuf;
				    debug(F110,"cmdir new *xp",*xp,0);
				}
				free(p);
			    }
			}
		    }
#endif /* VMS */
		    y = (!itsadir) ? 0 : 1;
		    debug(F111,"cmifi y itsadir",*xp,y);
		}
	    } else {			/* Parsing a filename. */
		debug(F110,"cmifi *xp pre-zxpand",*xp,0);
#ifndef COMMENT
		nzxopts = (d == 0) ? ZX_FILONLY : 0; /* So always expand. */
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		y = nzxpand(*xp,nzxopts);
#else
/* Here we're trying to fix a problem in which a directory name is accepted */
/* as a filename, but this breaks too many other things. */
		nzxopts = 0;
		if (!d) {
		    if (itsadir & !iswild(*xp)) {
			debug(F100,"cmifi dir when filonly","",0);
			printf("?Not a regular file: \"%s\"\n",*xp);
			if (sv) free(sv);
			if (np) free(np);
			return(-9);
		    } else {
			nzxopts = ZX_FILONLY;
			if (matchdot)  nzxopts |= ZX_MATCHDOT;
			if (recursive) nzxopts |= ZX_RECURSE;
			y = nzxpand(*xp,nzxopts);
		    }
		}
#endif /* COMMENT */
		nfiles = y;
		debug(F111,"cmifi y nzxpand",*xp,y);
		debug(F111,"cmifi y atmbuf",atmbuf,itsadir);
		expanded = 1;
	    }
	    /* domydir() calls zxrewind() so we MUST call nzxpand() here */
	    if (!expanded && diractive) {
		debug(F110,"cmifi diractive catch-all zxpand",*xp,0);
		nzxopts = (d == 0) ? ZX_FILONLY : (dirflg ? ZX_DIRONLY : 0);
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		y = nzxpand(*xp,nzxopts);
		nfiles = y;
		expanded = 1;
	    }
	    *wild = (iswild(sv) || (y > 1)) && (itsadir == 0);

#ifdef RECURSIVE
	    if (!*wild) *wild = recursive;
#endif /* RECURSIVE */

	    debug(F111,"cmifi sv wild",sv,*wild);
	    if (dirflg && *wild && !diractive) {
		printf("?Wildcard matches more than one directory\n");
		if (sv) free(sv);
		if (np) free(np);
		return(-9);
	    }
	    if (itsadir && d && !dirflg) { /* It's a directory and not wild */
		if (sv) free(sv);	/* and it's ok to parse directories */
		if (np) free(np);
		return(x);
	    }
	    if (y == 0) {
		if (path && !isabsolute(sv)) {
		    char * ptr = path;
		    char c;
		    while (1) {
			c = *ptr;
			if (c == PATHSEP || c == NUL) {
			    if (!*path) {
				path = NULL;
				break;
			    }
			    *ptr = NUL;
#ifdef UNIX
/* By definition of CDPATH, an empty member denotes the current directory */
			    if (!*path)
			      strcpy(atmbuf,".");
			    else
#endif /* UNIX */
			      strncpy(atmbuf,path,ATMBL);
#ifdef VMS
			    atmbuf[ATMBL] = NUL;
/* If we have a logical name, evaluate it recursively */
			    if (*(ptr-1) == ':') { /* Logical name ends in : */
				char *p; int n;
				while (((n = strlen(atmbuf))  > 0) &&
				       atmbuf[n-1] == ':') {
				    atmbuf[n-1] = NUL;
				    for (p = atmbuf; *p; p++)
				      if (islower(*p)) *p = toupper(*p);
				    debug(F111,"cmdir CDPATH LN 1",atmbuf,n);
				    p = getenv(atmbuf);
				    debug(F110,"cmdir CDPATH LN 2",p,0);
				    if (!p)
				      break;
				    strncpy(atmbuf,p,ATMBL);
				    atmbuf[ATMBL] = NUL;
				}
			    }
#else
#ifdef OS2
			    if (*(ptr-1) != '\\' && *(ptr-1) != '/')
			      strcat(atmbuf,"\\");
#else
#ifdef UNIX
			    if (*(ptr-1) != '/')
			      strcat(atmbuf,"/");
#else
#ifdef datageneral
			    if (*(ptr-1) != ':')
			      strcat(atmbuf,":");
#endif /* datageneral */
#endif /* UNIX */
#endif /* OS2 */
#endif /* VMS */
			    strcat(atmbuf,sv);
			    debug(F110,"cmifip add path",atmbuf,0);
			    if (c == PATHSEP) ptr++;
			    path = ptr;
			    break;
			}
			ptr++;
		    }
		    x = 1;
		    inword = 0;
		    cc = 0;
		    xc = (int) strlen(atmbuf);
		    *xp = "";
		    goto i_path;
		}
		if (d) {
		    if (sv) free(sv);
		    if (np) free(np);
		    return(-2);
		} else {
		    if (!nomsg)
		      printf("?No %s match - %s\n",
			     dirflg ? "directories" : "files", sv);
		    if (sv) free(sv);
		    if (np) free(np);
		    return(-9);
		}
	    } else if (y < 0) {
		printf("?Too many %s match - %s\n",
		       dirflg ? "directories" : "files", sv);
		if (sv) free(sv);
		if (np) free(np);
		return(-9);
	    } else if (*wild || y > 1) {
		if (sv) free(sv);
		if (np) free(np);
		return(x);
	    }

	    /* If not wild, see if it exists and is readable. */

	    debug(F111,"cmifi sv not wild",sv,*wild);

	    if (expanded)
	      znext(*xp);		/* Get first (only?) matching file */
	    if (dirflg)			/* Maybe wild and expanded */
	      itsadir = isdir(*xp);	/* so do this again. */
	    y = dirflg ? itsadir : zchki(*xp); /* Now check accessibility */
	    if (expanded) {
#ifdef ZXREWIND
		nfiles = zxrewind();	/* Rewind so next znext() gets 1st */
#else

		nzxopts = dirflg ? ZX_DIRONLY : 0;
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		nfiles = nzxpand(*xp,nzxopts);
#endif /* ZXREWIND */
	    }
	    debug(F111,"cmifi nfiles",*xp,nfiles);
	    free(sv);			/* done with this */
	    sv = NULL;
	    if (dirflg && y == 0) {
		printf("?Not a directory - %s\n",*xp);
#ifdef CKCHANNELIO
		z_error = FX_ACC;
#endif /* CKCHANNELIO */
		return(-9);
	    } else if (y == -3) {
		if (!xcmfdb) {
		    if (diractive)
		      /* Don't show filename if we're not allowed to see it */
		      printf("?Read permission denied\n");
		    else
		      printf("?Read permission denied - %s\n",*xp);
		}
		if (np) free(np);
#ifdef CKCHANNELIO
		z_error = FX_ACC;
#endif /* CKCHANNELIO */
		return(xcmfdb ? -6 : -9);
	    } else if (y == -2) {
		if (!recursive) {
		    if (np) free(np);
		    if (d) return(0);
		    if (!xcmfdb)
		      printf("?File not readable - %s\n",*xp);
#ifdef CKCHANNELIO
		    z_error = FX_ACC;
#endif /* CKCHANNELIO */
		    return(xcmfdb ? -6 : -9);
		}
	    } else if (y < 0) {
		if (np) free(np);
		if (!nomsg && !xcmfdb)
		  printf("?File not found - %s\n",*xp);
#ifdef CKCHANNELIO
		z_error = FX_FNF;
#endif /* CKCHANNELIO */
		return(xcmfdb ? -6 : -9);
	    }
	    if (np) free(np);
	    return(x);

#ifndef MAC
	  case 2:			/* ESC */
	    debug(F101,"cmifi esc, xc","",xc);
	    if (xc == 0) {
		if (*xdef) {
		    printf("%s ",xdef); /* If at beginning of field */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    inword = cmflgs = 0;
		    addbuf(xdef);	/* Supply default. */
		    if (setatm(xdef,0) < 0) {
			printf("Default name too long\n");
			if (np) free(np);
			return(-9);
		    }
		} else {		/* No default */
		    bleep(BP_WARN);
		}
		break;
	    }
#ifndef NOSPL
	    if (f) {			/* If a conversion function is given */
#ifdef DOCHKVAR
		char *s = *xp;		/* See if there are any variables in */
		while (*s) {		/* the string and if so, expand it.  */
		    if (chkvar(s)) {
#endif /* DOCHKVAR */
			zq = atxbuf;
			atxn = CMDBL;
			if ((x = (*f)(*xp,&zq,&atxn)) < 0) {
			    if (np) free(np);
			    return(-2);
			}
#ifdef DOCHKVAR
		    /* reduce cc by number of \\ consumed by conversion */
		    /* function (needed for OS/2, where \ is path separator) */
			cc -= (strlen(*xp) - strlen(atxbuf));
#endif /* DOCHKVAR */
			*xp = atxbuf;
			if (!atxbuf[0]) { /* Result empty, use default */
			    *xp = xdef;
			    cc = strlen(xdef);
			}
#ifdef DOCHKVAR
			break;
		    }
		    s++;
		}
#endif /* DOCHKVAR */
	    }
#endif /* NOSPL */
#ifdef DTILDE
	    dirp = tilde_expand(*xp);	/* Expand tilde, if any, */
	    if (*dirp != '\0') {	/* in the atom buffer. */
		if (setatm(dirp,0) < 0) {
		    printf("Expanded name too long\n");
		    if (np) free(np);
		    return(-9);
		}
	    }
	    *xp = atmbuf;
#endif /* DTILDE */
	    sp = *xp + cc;
#ifdef UNIXOROSK
	    if (!strcmp(atmbuf,"..")) {
		printf(" ");
		strcat(cmdbuf," ");
		cc++;
		bp++;
		*wild = 0;
		*xp = atmbuf;
		break;
	    } else if (!strcmp(atmbuf,".")) {
		bleep(BP_WARN);
		if (np) free(np);
		return(-1);
	    } else {
		/* This patches a glitch when user types "./foo<ESC>" */
		/* in which the next two chars are omitted from the */
		/* expansion.  There should be a better fix, however, */
		/* since there is no problem with "../foo<ESC>". */
		char *p = *xp;
		if (*p == '.' && *(p+1) == '/')
		  cc -= 2;
	    }
#endif /* UNIXOROSK */

#ifdef datageneral
	    *sp++ = '+';		/* Data General AOS wildcard */
#else
	    *sp++ = '*';		/* Others */
#endif /* datageneral */
	    *sp-- = '\0';
#ifdef GEMDOS
	    if (!strchr(*xp, '.'))	/* abde.e -> abcde.e* */
	      strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
	    /* Add wildcard and expand list. */
#ifdef COMMENT
	    /* This kills partial completion when ESC given in path segment */
	    nzxopts = dirflg ? ZX_DIRONLY : (d ? 0 : ZX_FILONLY);
#else
	    nzxopts = 0;
#endif /* COMMENT */
	    if (matchdot)  nzxopts |= ZX_MATCHDOT;
	    if (recursive) nzxopts |= ZX_RECURSE;
	    y = nzxpand(*xp,nzxopts);
	    nfiles = y;
	    debug(F111,"cmifi nzxpand",*xp,y);
	    if (y > 0) {
#ifdef OS2
                znext(filbuf);		/* Get first */
#ifdef ZXREWIND
		zxrewind();		/* Must "rewind" */
#else
		nzxpand(*xp,nxzopts);
#endif /* ZXREWIND */
#else  /* Not OS2 */
                ckstrncpy(filbuf,mtchs[0],CKMAXPATH);
#endif /* OS2 */
	    } else
	      *filbuf = '\0';
	    filbuf[CKMAXPATH] = NUL;
	    debug(F111,"cmifi filbuf",filbuf,y);
	    *sp = '\0';			/* Remove wildcard. */
	    *wild = (y > 1);
	    if (y == 0) {
		if (!nomsg) {
		    printf("?No %s match - %s\n",
			   dirflg ? "directories" : "files", atmbuf);
		    if (np) free(np);
		    return(-9);
		} else {
		    bleep(BP_WARN);
		    if (np) free(np);
		    return(-1);
		}
	    } else if (y < 0) {
		printf("?Too many %s match - %s\n",
		       dirflg ? "directories" : "files", atmbuf);
		if (np) free(np);
		return(-9);
	    } else if (y > 1) {     /* Not unique. */
#ifndef NOPARTIAL
/* Partial filename completion */
		int j, k; char c;
		k = 0;
		debug(F111,"cmifi partial",filbuf,cc);
#ifdef OS2
		{
		    int cur = 0,
		    len = 0,
		    len2 = 0,
		    min = strlen(filbuf);
		    char localfn[CKMAXPATH+1];

		    len = min;
		    for (j = 1; j <= y; j++) {
			znext(localfn);
			if (dirflg && !isdir(localfn))
			  continue;
			len2 = strlen(localfn);
			for (cur = cc;
			     cur < len && cur < len2 && cur <= min;
			     cur++
			     ) {
                            /* OS/2 or Windows, case doesn't matter */
			    if (tolower(filbuf[cur]) != tolower(localfn[cur]))
			      break;
			}
			if (cur < min)
			  min = cur;
		    }
		    filbuf[min] = NUL;
		    if (min > cc)
		      k++;
		}
#else /* OS2 */
		for (i = cc; (c = filbuf[i]); i++) {
		    for (j = 1; j < y; j++)
		      if (mtchs[j][i] != c) break;
		    if (j == y) k++;
		    else filbuf[i] = filbuf[i+1] = NUL;
		}
#endif /* OS2 */
		debug(F111,"cmifi partial k",filbuf,k);
		if (k > 0) {		/* Got more characters */
		    sp = filbuf + cc;	/* Point to new ones */
#ifdef VMS
		    for (i = 0; i < cc; i++) {
			cmdchardel();	/* Back up over old partial spec */
			bp--;
		    }
		    sp = filbuf;	/* Point to new word start */
		    debug(F100,"cmifi vms erase ok","",0);
#endif /* VMS */
		    cc = k;		/* How many new ones we just got */
		    printf("%s",sp);	/* Print them */
		    while (*bp++ = *sp++) ;	/* Copy to command buffer */
		    bp--;	    	        /* Back up over NUL */
		    debug(F110,"cmifi partial cmdbuf",cmdbuf,0);
		    if (setatm(filbuf,0) < 0) {
			printf("?Partial name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    debug(F111,"cmifi partial atmbuf",atmbuf,cc);
		    *xp = atmbuf;
		}
#endif /* NOPARTIAL */
		bleep(BP_WARN);
	    } else {			/* Unique, complete it.  */
#ifndef VMS
#ifdef CK_TMPDIR
		/* isdir() function required for this! */
		debug(F111,"cmifi unique",filbuf,cc);
		if (isdir(filbuf) && !dirflg) {
		    int len;
		    len = strlen(filbuf);
		    if (len > 0 && len < ATMBL - 1) {
			if (filbuf[len-1] != dirsep) {
			    filbuf[len] = dirsep;
			    filbuf[len+1] = NUL;
			}
		    }
		    sp = filbuf + cc;
		    bleep(BP_WARN);
		    printf("%s",sp);
		    cc++;
		    while (*bp++ = *sp++) ;
		    bp--;
		    if (setatm(filbuf,0) < 0) {
			printf("?Directory name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    debug(F111,"cmifi directory atmbuf",atmbuf,cc);
		    *xp = atmbuf;
		} else {		/* Not a directory or dirflg */
#endif /* CK_TMPDIR */
#endif /* VMS */
#ifndef VMS				/* VMS dir names are special */
#ifndef datageneral			/* VS dirnames must not end in ":" */
		    if (dirflg) {
			int len;
			len = strlen(filbuf);
			if (len > 0 && len < ATMBL - 1) {
			    if (filbuf[len-1] != dirsep) {
				filbuf[len] = dirsep;
				filbuf[len+1] = NUL;
			    }
			}
		    }
#endif /* datageneral */
#endif /* VMS */
		    sp = filbuf + cc;	/* Point past what user typed. */
#ifdef VMS
		    debug(F111,"cmifi VMS erasing",filbuf,cc);
		    for (i = 0; i < cc; i++) {
			cmdchardel();	/* Back up over old partial spec */
			bp--;
		    }
		    sp = filbuf;	/* Point to new word start */
		    debug(F111,"cmifi after VMS erase sp=",sp,cc);
#endif /* VMS */
		    /* Complete the name. */
#ifdef COMMENT
		    printf("%s%s",dirflg ? "" : " ",sp);
#else
		    printf("%s ",sp); /* Complete the name. */
#endif /* COMMENT */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(sp);		/* Add the characters to cmdbuf. */
		    if (setatm(filbuf,0) < 0) { /* And to atmbuf. */
			printf("?Completed name too long\n");
			if (np) free(np);
			return(-9);
		    }
		    if (dirflg && !isdir(filbuf)) {
			printf("?Not a directory - %s\n", filbuf);
			if (np) free(np);
			return(-9);
		    }
		    inword = cmflgs = 0;
		    *xp = atmbuf;	/* Return pointer to atmbuf. */
		    if (np) free(np);
		    return(0);
#ifndef VMS
#ifdef CK_TMPDIR
		}
#endif /* CK_TMPDIR */
#endif /* VMS */
	    }
	    break;

	  case 3:			/* Question mark - file menu wanted */
	    if (*xhlp == NUL)
	      printf(dirflg ? " Directory name" : " Input file specification");
	    else
	      printf(" %s",xhlp);
#ifdef GEMDOS
	    fflush(stdout);
#endif /* GEMDOS */
#ifdef OLDHELP
	    if (xc > 0) {
#endif /* OLDHELP */
#ifndef NOSPL
		if (f) {		/* If a conversion function is given */
#ifdef DOCHKVAR
		    char *s = *xp;	/* See if there are any variables in */
		    while (*s) {	/* the string and if so, expand them */
			if (chkvar(s)) {
#endif /* DOCHKVAR */
			    zq = atxbuf;
			    atxn = CMDBL;
			    if ((x = (*f)(*xp,&zq,&atxn)) < 0) {
				if (np) free(np);
				return(-2);
			    }
#ifdef DOCHKVAR
                    /* reduce cc by number of \\ consumed by conversion */
                    /* function (needed for OS/2, where \ is path separator) */
			    cc -= (strlen(*xp) - strlen(atxbuf));
#endif /* DOCHKVAR */
			    *xp = atxbuf;
#ifdef DOCHKVAR
			    break;
			}
			s++;
		    }
#endif /* DOCHKVAR */
		}
#endif /* NOSPL */
#ifdef DTILDE
		dirp = tilde_expand(*xp); /* Expand tilde, if any */
		if (*dirp != '\0') {
		    if (setatm(dirp,0) < 0) {
			printf("?Expanded name too long\n");
			if (np) free(np);
			return(-9);
		    }
		}
		*xp = atmbuf;
#endif /* DTILDE */
		debug(F111,"cmifi ? *xp, cc",*xp,cc);
		sp = *xp + cc;		/* Insert "*" at end */
#ifdef datageneral
		*sp++ = '+';		/* Insert +, the DG wild card */
#else
		*sp++ = '*';
#endif /* datageneral */
		*sp-- = '\0';
#ifdef GEMDOS
		if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
		  strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
		debug(F110,"cmifi ? wild",*xp,0);

#ifdef COMMENT
		/* This kills file lists when we're still the path part */
		nzxopts = dirflg ? ZX_DIRONLY : (d ? 0 : ZX_FILONLY);
#else
#ifdef COMMENT
		/* But this makes "cd ?" list regular files */
		nzxopts = 0;
#else
		nzxopts = dirflg ? ZX_DIRONLY : 0;
#endif /* COMMENT */
#endif /* COMMENT */
		debug(F101,"cmifi matchdot","",matchdot);
		if (matchdot)  nzxopts |= ZX_MATCHDOT;
		if (recursive) nzxopts |= ZX_RECURSE;
		y = nzxpand(*xp,nzxopts);
		nfiles = y;
		*sp = '\0';
		if (y == 0) {
		    if (nomsg) {
			printf(": %s\n",atmbuf);
			printf("%s%s",cmprom,cmdbuf);
			fflush(stdout);
			if (np) free(np);
			return(-1);
		    } else {
			printf("?No %s match - %s\n",
			       dirflg ? "directories" : "files", atmbuf);
			if (np) free(np);
			return(-9);
		    }
		} else if (y < 0) {
		    printf("?Too many %s match - %s\n",
			   dirflg ? "directories" : "files", atmbuf);
		    if (np) free(np);
		    return(-9);
		} else {
		    printf(", one of the following:\n");
#ifdef OLDHELP
		    clrhlp();
		    for (i = 0; i < y; i++) {
			znext(filbuf);
			if (!dirflg || isdir(filbuf)) {
#ifdef VMS
			    printf(" %s\n",filbuf); /* VMS names can be long */
#else
			    addhlp(filbuf);
#endif /* VMS */
			}
		    }
		    dmphlp();
#else  /* New way... */
		    if (filhelp(y,"","",1,dirflg) < 0) {
			if (np) free(np);
			return(-9);
		    }
#endif /* OLDHELP */
		}
#ifdef OLDHELP
	    } else
	      printf("\n");
#endif /* OLDHELP */
	    printf("%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
#endif /* MAC */
        }
#ifdef BS_DIRSEP
        dirnamflg = 1;
        x = gtword(0);                  /* No, get a word */
	dirnamflg = 0;
#else
        x = gtword(0);                  /* No, get a word */
#endif /* BS_DIRSEP */
    *xp = atmbuf;
    }
}

/*  C M F L D  --  Parse an arbitrary field  */
/*
 Returns
   -3 if no input present when required,
   -2 if field too big for buffer,
   -1 if reparse needed,
    0 otherwise, xp pointing to string result.
*/
int
cmfld(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int x, xc;
    char *zq;

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";

    debug(F110,"cmfld xdef 1",xdef,0);

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";
    ckstrncpy(cmdefault,xdef,CMDEFAULT); /* Copy default */
    xdef = cmdefault;

    debug(F111,"cmfld xdef 2",xdef,cmflgs);

    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword(0);                  /* No, get a word */
    } else {
	if (setatm(xdef,0) < 0) {	/* If so, use default, if any. */
	    printf("?Default too long\n");
	    return(-9);
	}
    }
    *xp = atmbuf;                       /* Point to result. */
    debug(F111,"cmfld atmbuf 1",atmbuf,cmflgs);

    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmfld: gtword",atmbuf,xc);
        debug(F101,"cmfld x","",x);
        switch (x) {
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -3:			/* Empty. */
	  case -2:			/* Out of space. */
	  case -1:			/* Reparse needed */
	    return(x);
	  case 0:			/* SP or NL */
	  case 1:
	    debug(F111,"cmfld 1",atmbuf,xc);
	    if (xc == 0) {		/* If no input, return default. */
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
	    }
	    *xp = atmbuf;		/* Point to what we got. */
	    debug(F111,"cmfld 2",atmbuf,(f) ? 1 : 0);
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* employ it now. */
		atxn = CMDBL;
		debug(F111,"cmfld zzstring",atxbuf,x);
		if ((*f)(*xp,&zq,&atxn) < 0)
		  return(-2);
		if (setatm(atxbuf,1) < 0) { /* Replace by new value */
		    printf("Value too long\n");
		    return(-9);
		}
		*xp = atmbuf;
	    }
	    debug(F111,"cmfld 3",atmbuf,xc);
	    if (**xp == NUL) {		/* If variable evaluates to null */
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
		if (**xp == NUL) x = -3; /* If still empty, return -3. */
	    }
	    debug(F111,"cmfld returns",*xp,x);
	    return(x);
	  case 2:			/* ESC */
	    if (xc == 0 && *xdef) {
		printf("%s ",xdef); /* If at beginning of field, */
#ifdef GEMDOS
		fflush(stdout);
#endif /* GEMDOS */
		addbuf(xdef);		/* Supply default. */
		inword = cmflgs = 0;
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		} else			/* Return as if whole field */
		  return(0);		/* typed, followed by space. */
	    } else {
		bleep(BP_WARN);
	    }
	    break;
	  case 3:			/* Question mark */
	    if (*xhlp == NUL)
	      printf(" Please complete this field");
	    else
	      printf(" %s",xhlp);
	    printf("\n%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
        }
	x = gtword(0);
	/* *xp = atmbuf; */
    }
}


/*  C M T X T  --  Get a text string, including confirmation  */

/*
  Print help message 'xhlp' if ? typed, supply default 'xdef' if null
  string typed.  Returns:

   -1 if reparse needed or buffer overflows.
    1 otherwise.

  with cmflgs set to return code, and xp pointing to result string.
*/
int
cmtxt(xhlp,xdef,xp,f) char *xhlp; char *xdef; char **xp; xx_strp f; {

    int x, i;
    char *xx, *zq;
    static int xc;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    cmfldflgs = 0;
    ckstrncpy(cmdefault,xdef,CMDEFAULT); /* Copy default */
    xdef = cmdefault;

    debug(F101,"cmtxt, cmflgs","",cmflgs);
    inword = 0;				/* Start atmbuf counter off at 0 */
    cc = 0;
    if (cmflgs == -1) {                 /* If reparsing, */
	*xp = pp;
        xc = (int)strlen(*xp);		/* get back the total text length, */
	bp = *xp;			/* and back up the pointers. */
	np = *xp;
	pp = *xp;
    } else {                            /* otherwise, */
	debug(F100,"cmtxt: fresh start","",0);
        *xp = "";                       /* start fresh. */
        xc = 0;
    }
    *atmbuf = NUL;                      /* And empty the atom buffer. */
    rtimer();				/* Reset timer */
    if ((x = cmflgs) != 1) {
	int done = 0;
	while (!done) {
	    x = gtword(0);		/* Get first word. */
	    *xp = pp;			/* Save pointer to it. */
	    debug(F111,"cmtxt:",*xp,cc);
	    if (x == -10) {
		if (gtimer() > timelimit) {
		    if (!quiet) printf("?Timed out\n");
		    return(x);
		}
	    } else
	      done = 1;
	}
    }
    while (1) {				/* Loop for each word in text. */
        xc += cc;                       /* Char count for all words. */
        debug(F111,"cmtxt gtword",atmbuf,xc);
        debug(F101,"cmtxt x","",x);
        switch (x) {
	  case -10:
	    if (gtimer() > timelimit) {
#ifdef IKSD
                extern int inserver;
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		if (!quiet) printf("?Timed out\n");
		return(-10);
	    } else {
		x = gtword(0);
		continue;
	    }
	  case -9:			/* Buffer overflow */
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
#ifdef MAC
	  case -3:			/* Quit/Timeout */
#endif /* MAC */
	  case -2:			/* Overflow */
	  case -1:			/* Deletion */
	    return(x);
	  case 0:			/* Space */
	    xc++;			/* Just count it */
	    break;
	  case 1:			/* CR or LF */
	    if (xc == 0) *xp = xdef;
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* Point to the expansion buffer */
		atxn = CMDBL;		/* specify its length */
		debug(F110,"cmtxt calling (*f)",*xp,0);
		if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
		cc = (int)strlen(atxbuf);
		if (cc < 1) {
		    *xp = xdef;
		    cc = strlen(xdef);
		} else {
		    *xp = atxbuf;	/* and return pointer to it. */
		}
		debug(F111,"cmtxt (*f) returns",*xp,cc);
	    }
	    xx = *xp;
	    for (i = (int)strlen(xx) - 1; i > 0; i--)
	      if (xx[i] != SP)		/* Trim trailing blanks */
		break;
	      else
		xx[i] = NUL;
#ifdef CK_RECALL
	    addcmd(cmdbuf);
#endif /* CK_RECALL */
	    return(x);
	  case 2:			/* ESC */
	    if (xc == 0) {		/* Nothing typed yet */
		if (*xdef) {		/* Have a default for this field? */
		    printf("%s ",xdef);	/* Yes, supply it */
		    inword = cmflgs = 0;
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    cc = addbuf(xdef);
		} else bleep(BP_WARN);	/* No default */
	    } else {			/* Already in field */
		int x; char *p;
		x = strlen(atmbuf);
		if (ckstrcmp(atmbuf,xdef,x,0)) /* Matches default? */
		  bleep(BP_WARN);	/* No */
		else {			/* Yes */
		    p = xdef + x;
		    printf("%s ", p);
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(p);
		    inword = cmflgs = 0;
		    debug(F110,"cmtxt: addbuf",cmdbuf,0);
		}
	    }
	    break;
	  case 3:			/* Question Mark */
	    if (*xhlp == NUL)
	      printf(" Text string");
	    else
	      printf(" %s",xhlp);
	    printf("\n%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    break;
	  default:
	    printf("?Unexpected return code from gtword() - %d\n",x);
	    return(-2);
        }
        x = gtword(0);
    }
}

/*  C M K E Y  --  Parse a keyword  */

/*
 Call with:
   table    --  keyword table, in 'struct keytab' format;
   n        --  number of entries in table;
   xhlp     --  pointer to help string;
   xdef     --  pointer to default keyword;
   f        --  processing function (e.g. to evaluate variables)
   pmsg     --  0 = don't print error messages
                1 = print error messages
                2 = include CM_HLP keywords even if invisible
                3 = 1+2
                4 = parse a switch (keyword possibly ending in : or =)
 Returns:
   -3       --  no input supplied and no default available
   -2       --  input doesn't uniquely match a keyword in the table
   -1       --  user deleted too much, command reparse required
    n >= 0  --  value associated with keyword
*/
int
cmkey(table,n,xhlp,xdef,f)
/* cmkey */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,1));
}
int
cmkeyx(table,n,xhlp,xdef,f)
/* cmkeyx */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,0));
}
int
cmswi(table,n,xhlp,xdef,f)
/* cmswi */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,4));
}

int
cmkey2(table,n,xhlp,xdef,tok,f,pmsg)
    struct keytab table[];
    int n;
    char *xhlp,
    *xdef;
    char *tok;
    xx_strp f;
    int pmsg;
{ /* cmkey2 */
    int i, tl, y, z = 0, zz, xc, wordlen = 0, cmswitch;
    char *xp, *zq;

    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";

    cmfldflgs = 0;
    if (!table) {
	printf("?Keyword table missing\n");
	return(-9);
    }
    tl = (int)strlen(tok);
    inword = xc = cc = 0;		/* Clear character counters. */
    cmswitch = pmsg & 4;		/* Flag for parsing a switch */

    debug(F101,"cmkey: pmsg","",pmsg);
    debug(F101,"cmkey: cmflgs","",cmflgs);
    /* debug(F101,"cmkey: cmdbuf","",cmdbuf);*/

    ppvnambuf[0] = NUL;

    if ((zz = cmflgs) == 1) {		/* Command already entered? */
	if (setatm(xdef,0) < 0) {	/* Yes, copy default into atom buf */
	    printf("?Default too long\n");
	    return(-9);
	}
    } else
      zz = gtword((pmsg == 4) ? 1 : 0);	/* Otherwise get a command word */

    debug(F101,"cmkey table length","",n);
    debug(F101,"cmkey cmflgs","",cmflgs);
    debug(F101,"cmkey zz","",zz);
    debug(F101,"cmkey cc","",cc);
    rtimer();				/* Reset timer */

    while (1) {
	xc += cc;
	debug(F111,"cmkey gtword xc",atmbuf,xc);

	switch (zz) {
	  case -10:			/* Timeout */
	    if (gtimer() < timelimit) {
		zz = gtword((pmsg == 4) ? 1 : 0);
		continue;
	    } else {
#ifdef IKSD
                extern int inserver;
                if (inserver) {
                    printf("\r\nIKSD IDLE TIMEOUT: %d sec\r\n", timelimit);
                    doexit(GOOD_EXIT,0);
                }
#endif /* IKSD */
		return(-10);
            }
	  case -5:
	    return(cmflgs = 0);
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -3:			/* Null Command/Quit/Timeout */
	  case -2:			/* Buffer overflow */
	  case -1:			/* Or user did some deleting. */
	    return(cmflgs = zz);

	  case 0:			/* User terminated word with space */
	  case 1:			/* or newline */
	  case 4:			/* or switch ending in : or = */
	    wordlen = cc;		/* Length if no conversion */
	    if (cc == 0) {		/* Supply default if we got nothing */
		if ((wordlen = setatm(xdef,(zz == 4) ? 2 : 0)) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
	    }
	    if (zz == 1 && cc == 0)	/* Required field missing */
	      return(-3);

	    if (f) {			/* If a conversion function is given */
		char * pp;
		zq = atxbuf;		/* apply it */
		pp = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		debug(F110,"cmkey atxbuf after *f",atxbuf,0);
		if (!*pp)		/* Supply default if we got nothing */
		  pp = xdef;
		ckstrncpy(ppvnambuf,atmbuf,PPVLEN);
		if ((wordlen = setatm(pp,(zz == 4) ? 2 : 0)) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    if (cmswitch && *atmbuf != '/') {
		if (pmsg & 1) {
		    bleep(BP_FAIL);
                    printf("?Not a switch - %s\n",atmbuf);
		}
		cmflgs = -2;
		return(-6);
	    }
	    if (cmswitch) {
		int i;
		for (i = 0; i < wordlen; i++) {
		    if (atmbuf[i] == ':' || atmbuf[i] == '=') {
			atmbuf[i] = NUL;
			break;
		    }
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Look up the word in the table */
	    switch (y) {
	      case -3:			/* Nothing to look up */
		break;
	      case -2:			/* Ambiguous */
		cmflgs = -2;
		if (pmsg & 1) {
		    bleep(BP_FAIL);
                    printf("?Ambiguous - %s\n",atmbuf);
		    return(-9);
		}
		return(-2);
	      case -1:			/* Not found at all */
		if (tl) {
		    for (i = 0; i < tl; i++) /* Check for token */
		      if (tok[i] == *atmbuf) { /* Got one */
			  debug(F000,"cmkey token:",atmbuf,*atmbuf);
			  ungword();  /* Put back the following word */
			  return(-5); /* Special return code for token */
		      }
		}
		if (tl == 0) {		/* No tokens were included */
#ifdef OS2
		    /* In OS/2 and Windows, allow for a disk letter like DOS */
		    if (isalpha(*atmbuf) && *(atmbuf+1) == ':')
		      return(-7);
#endif /* OS2 */
		    if ((pmsg & 1) && !quiet) {
			bleep(BP_FAIL);
			printf("?No keywords match - %s\n",atmbuf); /* cmkey */
		    }
		    return(cmflgs = -9);
		} else {
		    if (cmflgs == 1 || cmswitch) /* cmkey2 or cmswi */
		      return(cmflgs = -6);
		    else
		      return(cmflgs = -2);
		    /* The -6 code is to let caller try another table */
		}
		break;
	      default:
#ifdef CK_RECALL
		if (test(table[z].flgs,CM_NOR)) no_recall = 1;
#endif /* CK_RECALL */
		if (zz == 4)
		  swarg = 1;
		cmkwflgs = table[z].flgs;
		break;
	    }
	    return(y);

	  case 2:			/* User terminated word with ESC */
	    debug(F101,"cmkey Esc cc","",cc);
            if (cc == 0) {
		if (*xdef != NUL) {     /* Nothing in atmbuf */
		    printf("%s ",xdef); /* Supply default if any */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
		    addbuf(xdef);
		    if (setatm(xdef,0) < 0) {
			printf("?Default too long\n");
			return(-9);
		    }
		    inword = cmflgs = 0;
		    debug(F111,"cmkey: default",atmbuf,cc);
		} else {
		    debug(F101,"cmkey Esc pmsg","",0);
#ifdef COMMENT
/*
  Chained FDBs...  The idea is that this function might not have a default,
  but the next one might.  But if it doesn't, there is no way to come back to
  this one.  To be revisited later...
*/
		    if (xcmfdb)		/* Chained fdb -- try next one */
		      return(-3);
#endif /* COMMENT */
		    if (pmsg & (1|4)) {	/* So for now just beep */
			bleep(BP_WARN);
		    }
		    break;
		}
            }
	    if (f) {			/* If a conversion function is given */
		char * pp;
		zq = atxbuf;		/* apply it */
		pp = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0)
		  return(-2);
		if (!*pp)
		  pp = xdef;
		if (setatm(pp,0) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Something in atmbuf */
	    debug(F111,"cmkey lookup y",atmbuf,y);
	    debug(F111,"cmkey lookup z",atmbuf,z);
	    if (y == -2 && z >= 0 && z < n) { /* Ambiguous */
#ifndef NOPARTIAL
		int j, k, len = 9999;	/* Do partial completion */
		/* Skip past any abbreviations in the table */
		for ( ; z < n; z++) {
		    if (table[z].flgs & CM_ABR == 0)
		      break;
		    if (!(table[z].flgs & CM_HLP) || (pmsg & 2))
		      break;
		}
		debug(F111,"cmkey partial z",atmbuf,z);
		debug(F111,"cmkey partial n",atmbuf,n);
		for (j = z+1; j < n; j++) {
		    debug(F111,"cmkey partial j",table[j].kwd,j);
		    if (ckstrcmp(atmbuf,table[j].kwd,cc,0))
		      break;
		    if (table[j].flgs & CM_ABR)
		      continue;
		    if ((table[j].flgs & CM_HLP) && !(pmsg & 2))
		      continue;
		    k = ckstrpre(table[z].kwd,table[j].kwd);
		    debug(F111,"cmkey partial k",table[z].kwd,k);
		    if (k < len)
		      len = k; /* Length of longest common prefix */
		}
		debug(F111,"cmkey partial len",table[z].kwd,len);
		if (len != 9999 && len > cc) {
		    strcat(atmbuf,table[z].kwd+cc);
		    atmbuf[len] = NUL;
		    printf("%s",atmbuf+cc);
		    strcat(cmdbuf,atmbuf+cc);
		    xc += (len - cc);
		    cc = len;
		}
#endif /* NOPARTIAL */
		bleep(BP_WARN);
		break;
	    } else if (y == -3) {
		bleep(BP_WARN);
		break;
	    } else if (y == -1) {	/* Not found */
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf("?No keywords match - \"%s\"\n",atmbuf);
		}
		cmflgs = -2;
		return(-9);
	    }
/*
  If we found it, but it's a help-only keyword and the "help" bit is not
  set in pmsg, then not found.
*/
	    debug(F101,"cmkey flgs","",table[z].flgs);
	    if (test(table[z].flgs,CM_HLP) && ((pmsg & 2) == 0)) {
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf("?No keywords match - %s\n",atmbuf);
		}
		cmflgs = -2;
		return(-9);
	    }
/*
  See if the keyword just found has the CM_ABR bit set in its flgs field, and
  if so, search forwards in the table for a keyword that has the same kwval
  but does not have CM_ABR (or CM_INV?) set, and then expand using the full
  keyword.  WARNING: This assumes that (a) keywords are in alphabetical order,
  and (b) the CM_ABR bit is set only if the the abbreviated keyword is a true
  abbreviation (left substring) of the full keyword.
*/
	    if (test(table[z].flgs,CM_ABR)) {
		int zz;
		for (zz = z+1; zz < n; zz++)
		  if ((table[zz].kwval == table[z].kwval) &&
		      (!test(table[zz].flgs,CM_ABR)) &&
		      (!test(table[zz].flgs,CM_INV))) {
		      z = zz;
		      break;
		  }
	    }
	    xp = table[z].kwd + cc;
	    if (cmswitch && test(table[z].flgs,CM_ARG)) {
#ifdef VMS
		printf("%s=",xp);
		brkchar = '=';
#else
		printf("%s:",xp);
		brkchar = ':';
#endif /* VMS */
	    } else {
		printf("%s ",xp);
		brkchar = SP;
	    }
#ifdef CK_RECALL
	    if (test(table[z].flgs,CM_NOR)) no_recall = 1;
#endif /* CK_RECALL */
	    cmkwflgs = table[z].flgs;
#ifdef GEMDOS
	    fflush(stdout);
#endif /* GEMDOS */
	    addbuf(xp);
	    if (cmswitch && test(table[z].flgs,CM_ARG)) {
		bp--;			/* Replace trailing space with : */
#ifdef VMS
		*bp++ = '=';
#else
		*bp++ = ':';
#endif /* VMS */
		*bp = NUL;
		np = bp;
		swarg = 1;
	    }
	    inword = 0;
	    cmflgs = 0;
	    debug(F110,"cmkey: addbuf",cmdbuf,0);
	    return(y);

	  case 3:			/* User typed "?" */
	    if (f) {			/* If a conversion function is given */
		char * pp;
		zq = atxbuf;		/* do the conversion now. */
		pp = atxbuf;
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		if (setatm(pp,0) < 0) {
		    printf("?Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Look up what we have so far. */
	    if (y == -1) {
		cmflgs = -2;
		if ((pmsg & 1) && !quiet) {
		    bleep(BP_FAIL);
		    printf(" No keywords match\n");
		    return(-9);
		}
		return(-2);
	    }
	    if (*xhlp == NUL)
	      printf(" One of the following:\n");
	    else
	      printf(" %s, one of the following:\n",xhlp);

#ifdef OLDHELP
	    if ((y > -1) &&
		!test(table[z].flgs,CM_ABR) &&
		((z >= n-1) || ckstrcmp(table[z].kwd,table[z+1].kwd,cc,0))
		) {
		printf(" %s\n",table[z].kwd);
	    } else {
		clrhlp();
		for (i = 0; i < n; i++) {
		    if (!ckstrcmp(table[i].kwd,atmbuf,cc,0)
			&& !test(table[i].flgs,CM_INV)
			)
		      addhlp(table[i].kwd);
		}
		dmphlp();
	    }
#else  /* New way ... */
	    {
		int x;
		x = pmsg & (2|4);	/* See kwdhelp() comments */
		if (atmbuf[0])		/* If not at beginning of field */
		  x |= 1;		/* also show invisibles */
		kwdhelp(table,n,atmbuf,"","",1,x);
	    }
#endif /* OLDHELP */
#ifndef NOSPL
	    {
		extern int topcmd;
		if (tl > 0 && topcmd != XXHLP) /* This is bad... */
		  printf("or the name of a macro (\"do ?\" for a list)\n");
	    }
#endif /* NOSPL */
	    if (*atmbuf == NUL) {
		if (tl == 1)
		  printf("or the token %c\n",*tok);
		else if (tl > 1)
		  printf("or one of the tokens: %s\n",ckspread(tok));
	    }
	    printf("%s%s", cmprom, cmdbuf);
	    fflush(stdout);
	    break;

	  default:
	    printf("\n%d - Unexpected return code from gtword\n",zz);
	    return(cmflgs = -2);
	}
	zz = gtword(0);
	debug(F111,"cmkey gtword zz",atmbuf,zz);
    }
}

int
chktok(tlist) char *tlist; {
    char *p;
    p = tlist;
    while (*p != NUL && *p != *atmbuf) p++;
    return((*p) ? (int) *p : 0);
}

/* Routines for parsing and converting dates and times */

#define isdatesep(c) (c==SP||c=='-'||c=='/'||c=='.'||c=='_')
#define istimesep(c) (c==':' || c=='P'||c=='p'||c=='A'||c=='a')

char cmdatebuf[18] = { NUL, NUL };

#define TU_DAYS   0
#define TU_WEEKS  1
#define TU_MONTHS 2
#define TU_YEARS  3

static struct keytab timeunits[] = {
  { "days",   TU_DAYS,   0 },
  { "months", TU_MONTHS, 0 },
  { "weeks",  TU_WEEKS,  0 },
  { "wks",    TU_WEEKS,  0 },
  { "years",  TU_YEARS,  0 },
  { "yrs",    TU_YEARS,  0 }
};
static int nunits = (sizeof(timeunits) / sizeof(struct keytab));

/*  C M C V T D A T E  --  Converts free-form date to standard form.  */

/*
   Call with
     s = pointer to free-format date-time
     t = 0: return time only if time was given in s
     t = 1: always return time (00:00:00 if no time given in s)
     t = 2: allow time to be > 24:00:00
   Returns:
     -1 on failure, 0 or greater on success with result in cmdate[].
*/
int
cmcvtdate(s,t) char * s; int t; {
    int rc = 0, x, i, hh, mm, ss, pmflag = 0, nodate = 0, len;
    int units;
    char * fld[3], * p;
    char * year, * month = NULL, * day;
    char * hour = "00", * min = "00", * sec = "00";
    char tmpbuf[8];
    char xbuf[32];
    char ybuf[32];
    char dbuf[26];
    char daybuf[3];
    char monbuf[3];
    char yearbuf[5];

    debug(F110,"cmcvtdate",s,0);
    if (!s) s = "";

    tmpbuf[0] = NUL;

    while (*s == SP) s++;		/* Gobble any leading blanks */

    len = strlen(s);
    if (len == 0) {
	s = ckdate();
	len = 17;
	debug(F110,"cmcvtdate now",s,0);
    }
    if (len > 30) {			/* Check length of arg */
	debug(F101,"cmcvtdate date too long","",-1);
	return(-1);
    }
    ckstrncpy(xbuf,s,32);		/* Make a local copy we can poke */
    s = xbuf;				/* Point to it */
    s[len] = NUL;

/* First we handle "today", "yesterday", "tomorrow", etc */

    if (ckstrchr("+-TtYy",s[0])) {
	int i, k, n, minus = 0;
	long jd;
	jd = mjd(ckdate());
	debug(F111,"cmcvtdate NEW",s,jd);
	if (s[0] == '+' || s[0] == '-') { /* {+,-} <number> <timeunits> */
	    char * kp, * np;
	    char tmpyear[5];
	    char * dp;
	    if (s[0] == '-')		/* Sign */
	      minus = 1;
	    kp = s+1;			/* Skip intervening spaces */
	    while (*kp) {
		if (*kp == SP) kp++;
		else break;
	    }
	    if (!*kp)
	      return(-1);
	    np = kp;			/* Number */
	    while (*kp) {
		if (isdigit(*kp)) kp++;
		else break;
	    }
	    if (!*kp)
	      return(-1);
	    n = atoi(np);
	    if (minus) n = 0 - n;
	    debug(F101,"cmcvtdate offset n","",n);
	    while (*kp) {		/* Find end of number */
		if (isdigit(*kp)) kp++;
		else break;
	    }
	    while (*kp) {		/* Skip spaces again */
		if (*kp == SP) kp++;
		else break;
	    }
	    debug(F110,"cmcvtdate unit start",kp,0);
	    p = kp;			/* Units */
	    while (*p) {
		if (isalpha(*p)) {
		    p++;
		} else if (isdatesep(*p)) { /* Have a date separator */
		    *p++ = NUL;
		    break;
		} else
		  return(-1);
	    }
	    while (*p) {		/* Skip any spaces */
		if (*p == SP) p++;
		else break;
	    }
	    debug(F110,"cmcvtdate time pointer",p,0);
	    debug(F110,"cmcvtdate unit",kp,0);
	    x = lookup(timeunits,kp,nunits,&k);	/* Look up units */
	    if (x < 0) {
		debug(F111,"cmcvtdate lookup fails",kp,x);
		return(-1);
	    }
	    units = x;
	    debug(F111,"cmcvtdate offset units",timeunits[k].kwd,units);

	    switch (units) {		/* Handle each unit */
	      case TU_DAYS:		/* Days */
		jd += n;
		strcpy(ybuf,mjd2date(jd));
		if (*p) {
		    ybuf[8] = ' ';
		    strcpy(ybuf+9,p);
		} else
		  strcpy(ybuf+8," 00:00:00");
		s = ybuf;
		len = strlen(ybuf);
		debug(F111,"cmcvtdate days",s,len);
		goto normal;

	      case TU_WEEKS:		/* Weeks */
		jd += (7 * n);
		strcpy(ybuf,mjd2date(jd));
		if (*p) {
		    ybuf[8] = ' ';
		    strcpy(ybuf+9,p);
		} else
		  strcpy(ybuf+8," 00:00:00");
		s = ybuf;
		len = strlen(ybuf);
		debug(F111,"cmcvtdate weeks",s,len);
		goto normal;

	      case TU_MONTHS: {		/* Months */
		  char tmpmonth[3];
		  int xyear, xmonth;
		  dp = ckdate();
		  tmpyear[0] = dp[0];
		  tmpyear[1] = dp[1];
		  tmpyear[2] = dp[2];
		  tmpyear[3] = dp[3];
		  tmpyear[4] = NUL;
		  tmpmonth[0] = dp[4];
		  tmpmonth[1] = dp[5];
		  tmpmonth[2] = NUL;
		  xyear = atoi(tmpyear);
		  xmonth = atoi(tmpmonth);
		  xmonth += n;
		  xyear += (xmonth / 12);
		  xmonth = (xmonth % 12);
		  if (xmonth <= 0) {
		      xmonth += 12;
		      xyear--;
		  }
		  sprintf(ybuf,"%04d%02d%s",xyear,xmonth,dp+6);
		  if (*p) {
		      ybuf[8] = ' ';
		      strcpy(ybuf+9,p);
		  } else
		    strcpy(ybuf+8," 00:00:00");
		  s = ybuf;
		  len = strlen(ybuf);
		  debug(F111,"cmcvtdate months",s,len);
		  goto normal;
	      }
	      case TU_YEARS: {		/* Years */
		  dp = ckdate();
		  tmpyear[0] = dp[0];
		  tmpyear[1] = dp[1];
		  tmpyear[2] = dp[2];
		  tmpyear[3] = dp[3];
		  tmpyear[4] = NUL;
		  sprintf(ybuf,"%04d%s",(atoi(tmpyear)+n),dp+4);
		  if (*p) {
		      ybuf[8] = ' ';
		      strcpy(ybuf+9,p);
		  } else
		    strcpy(ybuf+8," 00:00:00");
		  s = ybuf;
		  len = strlen(ybuf);
		  debug(F111,"cmcvtdate years",s,len);
		  goto normal;
	      }
	    }
	    return(-1);
	}
	i = ckstrpre(s,"today");	/* TODAY */
	if (i > 2 && (s[i] == NUL || isdatesep(s[i]))) {
	    strncpy(ybuf,ckdate(),8);
	    strcpy(ybuf+8," 00:00:00");
	    if (s[i])
	      strcpy(ybuf+8,s+i);
	    s = ybuf;
	    len = strlen(s);
	    debug(F111,"cmcvtdate today",s,len);
	    goto normal;
	}
	i = ckstrpre(s,"tomorrow");	/* TOMORROW */
	if (i > 2 && (s[i] == NUL || isdatesep(s[i]))) {
	    jd++;
	    strncpy(ybuf,mjd2date(jd),8);
	    strcpy(ybuf+8," 00:00:00");
	    if (s[i]) strcpy(ybuf+8,s+i);
	    s = ybuf;
	    len = strlen(s);
	    debug(F111,"cmcvtdate tomorrow",s,len);
	    goto normal;

	}
	i = ckstrpre(s,"yesterday");	/* YESTERDAY */
	if (i > 0 && (s[i] == NUL || isdatesep(s[i]))) {
	    jd--;
	    strncpy(ybuf,mjd2date(jd),8);
	    strcpy(ybuf+8," 00:00:00");
	    if (s[i]) strcpy(ybuf+8,s+i);
	    s = ybuf;
	    len = strlen(s);
	    debug(F111,"cmcvtdate yesterday",s,len);
	    goto normal;
	}
    }

  normal:

    if (len >= 8) {			/* Already in right format? */
	if (isdigit(s[0])  && isdigit(s[1])  &&
	    isdigit(s[2])  && isdigit(s[3])  &&
	    isdigit(s[4])  && isdigit(s[5])  &&
	    isdigit(s[6])  && isdigit(s[7]))
	  if (!s[8]) {
	      strcat(s," 00:00:00");
	      ckstrncpy(cmdatebuf,s,18);
	      debug(F111,"cmcvtdate yyyymmdd",s,rc);
	      return(rc);
	  } else if (len == 17 &&
		     isdigit(s[9])  && isdigit(s[10]) &&
		     isdigit(s[12]) && isdigit(s[13]) &&
		     isdigit(s[15]) && isdigit(s[16]) &&
		     s[11] == ':'   && s[14] == ':'   &&
		     (s[8] == SP || s[8] == '-' || s[8] == '_')) {
	      ckstrncpy(cmdatebuf,s,18);
	      debug(F111,"cmcvtdate yyyymmdd hh:mm:ss",s,rc);
	      return(rc);
	} else {			/* We have a numeric date */
	    debug(F111,"cmcvtdate yyyymmdd xxx",s,rc);
	    p = s+9;			/* Start of time field */

	    yearbuf[0] = s[0]; yearbuf[1] = s[1];
	    yearbuf[2] = s[2]; yearbuf[3] = s[3];
	    yearbuf[4] = NUL;  year = yearbuf;

	    monbuf[0] = s[4];  monbuf[1] = s[5];
	    monbuf[2] = NUL;   month = monbuf;

	    daybuf[0] = s[6];  daybuf[1] = s[7];
	    daybuf[2] = NUL;   day = daybuf;

	    goto dotime;
	}
    }
    fld[i = 0] = (p = s);		/* First field */
    while (*p) {			/* Get next two fields */
	if (isdatesep(*p)) {		/* Have a date separator */
	    *p++ = NUL;			/* Replace by NUL */
	    if (*p) {			/* Now we're at the next field */
		while (*p == SP) p++;	/* Skip leading spaces */
		if (!*p) break;		/* Make sure we still have something */
		if (i == 2)		/* Last one? */
		  break;
		fld[++i] = p;		/* No, record pointer to this one */
	    } else
	      break;
	} else if (istimesep(*p)) {	/* Have a time separator */
	    if (isalpha(*p) && !isdigit(*(p-1))) { /* Might be letter */
		p++;			/* in month name... */
		continue;
	    }
	    if (i != 0) {		/* After a date */
		debug(F111,"cmcvtdate date bad timesep",p,-1);
		return(-1);
	    }
	    nodate = 1;			/* Or without a date */
	    break;
	}
	p++;
    }
    if (p > s && i == 0)		/* Make sure we have a date */
      nodate = 1;

    if (nodate) {			/* No date */
	char *tmp;			/* Substitute today's date */
	ztime(&tmp);
	if (!tmp) {
	    debug(F101,"cmcvtdate null ztime","",-1);
	    return(-1);
	}
	if (!*tmp) {
	    debug(F101,"cmcvtdate emtpy ztime","",-1);
	    return(-1);
	}
	ckstrncpy(dbuf,tmp,26);		/* Reformat */
	if (dbuf[8] == SP) dbuf[8] = '0';
	fld[0] = dbuf+8;
	dbuf[10] = NUL;
	fld[1] = dbuf+4;
	dbuf[7] = NUL;
	fld[2] = dbuf+20;
	dbuf[24] = NUL;
	p = s;				/* Back up source pointer to reparse */
    } else if (i != 2) {
	debug(F101,"cmcvtdate fail A","",-1);
	return(-1);
    }
    if (!rdigits(fld[0])) {		/* Now parse the date */
	debug(F101,"cmcvtdate fail B","",-1);
	return(-1);
    }
    if (!rdigits(fld[1])) {
	if ((x = lookup(cmonths,fld[1],12,NULL)) < 0) {
	    debug(F101,"cmcvtdate fail C","",-1);
	    return(-1);
	}
	sprintf(tmpbuf,"%02d",x);
	month = tmpbuf;
    }
    if (((int)strlen(fld[0]) == 4)) {	/* yyyy-xx-dd */
	year = fld[0];
	day = fld[2];
	if (!month)
	  month = fld[1];		/* yyyy-mm-dd */
    } else if (((int)strlen(fld[2]) == 4)) { /* xx-xx-yyyy */
	year = fld[2];
	if (month) {			/* dd-name-yyyy */
	    day = fld[0];
	} else {			/* xx-xx-yyyy */
	    int f0, f1;
	    f0 = atoi(fld[0]);
	    f1 = atoi(fld[1]);
	    if ((f0 > 12) && (f1 <= 12)) {
		day = fld[0];		/* mm-dd-yyyy */
		month = fld[1];
	    } else if ((f0 <= 12) && (f1 > 12)) {
		if (!rdigits(fld[1]))
		  return(-1);
		else
		  day = fld[1];		/* dd-mm-yyyy */
		month = fld[0];
#ifdef COMMENT
	    } else if ((f0 <= 12) && (f1 <= 12)) {
		if (!quiet)
		  printf("?Day and month are ambiguous - \"%s\"\n", o);
		return(-9);
#endif /* COMMENT */
	    } else {
		debug(F101,"cmcvtdate fail D","",-1);
		return(-1);
	    }
	}
    } else {
	debug(F101,"cmcvtdate fail E","",-1);
	return(-1);
    }
    x = atoi(month);
    sprintf(tmpbuf,"%02d",x);		/* 2-digit numeric month */

  dotime:
    debug(F110,"cmcvtdate dotime s",s,0);
    debug(F110,"cmcvtdate dotime p",p,0);
    if ((x  = atoi(day)) > 31) {
	debug(F101,"cmcvtdate fail K","",-1);
	return(-1);
    }
    if (!*p && t == 0) {
	sprintf(cmdatebuf,"%s%s%02d",year,month,x);
	return(0);
    }
    fld[i = 0] = *p ? p : "00";		/* First time field */
    fld[1] = "00";
    fld[2] = "00";
    while (*p) {			/* Get the rest, if any */
	if (istimesep(*p)) {
	    debug(F000,"cmcvtdate timesep:",p,*p);
	    if (*p == 'P' || *p == 'p') {
		if (*(p+1) != 'M' && *(p+1) != 'm') {
		    debug(F101,"cmcvtdate fail F","",-1);
		    return(-1);
		} else if (!*(p+2)) {
		    pmflag = 1;
		    *p = NUL;
		    break;
		} else {
		    debug(F101,"cmcvtdate fail F","",-1);
		    return(-1);
		}
	    } else if (*p == 'A' || *p == 'a') {
		if (*(p+1) != 'M' && *(p+1) != 'm') {
		    debug(F101,"cmcvtdate fail F","",-1);
		    return(-1);
		}
		if (*(p+2)) {
		    debug(F101,"cmcvtdate fail F","",-1);
		    return(-1);
		}
		*p = NUL;
		break;
	    }
	    *p++ = NUL;
	    if (*p) {
		while (*p == SP) p++;
		if (!*p) break;
		if (i == 2)
		  break;
		fld[++i] = p;
	    } else
	      break;
	}
	p++;
    }
    debug(F101,"cmcvtdate time i","",i);
    debug(F110,"cmcvtdate time fld[0]",fld[0],0);
    debug(F110,"cmcvtdate time fld[1]",fld[1],0);
    debug(F110,"cmcvtdate time fld[2]",fld[2],0);

    if (!rdigits(fld[0]))
      return(-1);

    hour = fld[0];
    if (i >= 1) {
	if (!rdigits(fld[1]))
	  return(-1);
	else
	  min = fld[1];
    }
    if (i == 2) {
	if (!rdigits(fld[2]))
	  return(-1);
	else
	  sec = fld[2];
    }
    hh = atoi(hour);
    if (pmflag && hh <= 11)
      hh += 12;
    if ((t != 2 && hh > 24) || hh < 0) {
	debug(F101,"cmcvtdate fail G","",-1);
	return(-1);
    }
    if ((mm = atoi(min)) > 59) {
	debug(F101,"cmcvtdate fail H","",-1);
	return(-1);
    }
    if ((ss = atoi(sec)) > 59) {
	debug(F101,"cmcvtdate fail I","",-1);
	return(-1);
    }
    if (mm < 0 || ss < 0) return(-1);
    if (t != 2 && (ss > 0 || mm > 0) && hh > 23) {
	debug(F101,"cmcvtdate fail J","",-1);
	return(-1);
    }
    debug(F110,"cmcvtdate year",year,0);
    debug(F110,"cmcvtdate month",month,0);
    debug(F101,"cmcvtdate x","",x);
    debug(F101,"cmcvtdate hh","",hh);
    debug(F101,"cmcvtdate mm","",mm);
    debug(F101,"cmcvtdate ss","",ss);

    sprintf(cmdatebuf,"%s%s%02d %02d:%02d:%02d",year,month,x,hh,mm,ss);

#ifdef DEBUG
    if (deblog) {
	debug(F101,"cmcvtdate hour","",hh);
	debug(F101,"cmcvtdate minute","",mm);
	debug(F101,"cmcvtdate second","",ss);
	debug(F111,"cmcvtdate result",cmdatebuf,rc);
    }
#endif /* DEBLOG */
    return(0);
}


/*  C K C V T D A T E  --  Like cmcvtdate(), but returns string.  */
/*  For use by date-related functions */
/*  See calling conventions for cmcvtdate() above. */

char *
ckcvtdate(p,t) char * p; int t; {
    if (cmcvtdate(p,t) < 0)
      return("<BAD_DATE_OR_TIME>");	/* \fblah() error message */
    else
      return((char *) cmdatebuf);
}


/*  C M D A T E  --  Parse a date and/or time  */

/*
  Accepts date in various formats.  If the date is recognized,
  this routine returns 0 or greater with the result string pointer
  pointing to a buffer containing the date as "yyyymmdd hh:mm:ss".
*/
int
cmdate(xhlp,xdef,xp,quiet,f) char *xhlp, *xdef, **xp; int quiet; xx_strp f; {
    int /* i, */ x, rc;
    char *o, *s, *zq;

    cmfldflgs = 0;
    if (!xhlp) xhlp = "";
    if (!xdef) xdef = "";
    if (!*xhlp) xhlp = "Date and/or time";
    *xp = "";

    rc = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F101,"cmdate cmfld rc","",rc);
    if (rc < 0)
      return(rc);
    debug(F110,"cmdate 1",s,0);
    o = s;				/* Remember what they typed. */
    s = brstrip(s);
    debug(F110,"cmdate 2",s,0);

    x = 0;
    if (f) {				/* If a conversion function is given */
	char * pp;
	zq = atxbuf;			/* do the conversion. */
	pp = atxbuf;
	atxn = CMDBL;
	if ((x = (*f)(s,&zq,&atxn)) < 0) return(-2);
	if (!*pp)
	  pp = xdef;
	if (setatm(pp,0) < 0) {
	    if (!quiet) printf("?Evaluated date too long\n");
	    return(-9);
	}
	s = atxbuf;
    }
    rc = cmcvtdate(s,1);
    if (rc < 0) {
	if (!quiet) printf("Invalid date or time - \"%s\"\n", o);
	return(-9);
    }
    *xp = cmdatebuf;
    return(rc);
}

#ifdef CK_RECALL			/* Command-recall functions */

/*  C M R I N I  --  Initialize or change size of command recall buffer */

int
cmrini(n) int n; {
    int i;
    if (recall && in_recall) {		/* Free old storage, if any */
	for (i = 0; i < cm_recall; i++) {
	    if (recall[i]) {
		free(recall[i]);
		recall[i] = NULL;
	    }
	}
	free(recall);
	recall = NULL;
    }
    cm_recall = n;			/* Set new size */
    rlast = current = -1;		/* Initialize pointers */
    if (n > 0) {
	recall = (char **)malloc((cm_recall + 1) * sizeof(char *));
	if (!recall)
	  return(1);
	for (i = 0; i < cm_recall; i++) {
	    recall[i] = NULL;
	}
	in_recall = 1;			/* Recall buffers init'd */
    }
    return(0);
}

/*  C M A D D N E X T  --  Force addition of next command */

VOID
cmaddnext() {
    if (on_recall && in_recall) {	/* Even if it doesn't come */
	force_add = 1;			/* from the keyboard */
	no_recall = 0;
    }
}

/*  C M G E T C M D  --  Find most recent matching command  */

char *
cmgetcmd(s) char * s; {
    int i;
    for (i = current; i >= 0; i--) {	/* Search backward thru history list */
	if (!recall[i]) continue;	/* This one's null, skip it */
	if (ckmatch(s,recall[i],0,1))	/* Match? */
	  return(recall[i]);		/* Yes, return pointer */
    }
    return(NULL);			/* No match, return NULL pointer */
}

/*  A D D C M D  --  Add a command to the recall buffer  */

VOID
addcmd(s) char * s; {
    int len;

    if (!s) s = cmdbuf;
    len = strlen(s);

#ifdef CKSYSLOG
    /* Log all interactive commands */
    /* Logging macros & TAKE files is way too much */
    if (ckxlogging) {
	if (ckxsyslog >= SYSLG_CX || ckxsyslog >= SYSLG_CM && !cmdsrc())
	  cksyslog(SYSLG_CX, 1, "command", s, NULL);
    }
#endif /* CKSYSLOG */

    if ((!cmdsrc() || force_add) &&	/* Reading commands from keyboard? */
	(on_recall) &&			/* Recall is turned on? */
	(cm_recall > 0) &&		/* Saving commands? */
	!no_recall &&			/* Not not saving this command? */
	len > 0) {			/* Non-null command? */

	force_add = 0;

        if (rlast >= cm_recall - 1) {	/* Yes, buffer full? */
	    int i;			/* Yes. */
	    if (recall[0]) {		/* Discard oldest command */
		free(recall[0]);
		recall[0] = NULL;
	    }
	    for (i = 0; i < rlast; i++) { /* The rest */
		recall[i] = recall[i+1]; /* move back */
	    }
	    rlast--;			/* Now we have one less */
	}
        rlast++;			/* Index of last command in buffer */
	current = rlast;		/* Also now the current command */
	if (current >= cm_recall) {
	    printf("Oops, command recall error\n");
	} else {
	    recall[current] = malloc(len+1);
	    if (recall[current])
	      strcpy(recall[current],s);
	}
    }
}
#endif /* CK_RECALL */

int
cmgetlc(s) char * s; {			/* Get leading char */
    char c;
    while ((c = *s++) <= SP) ;
    return(c);
}


/*  C M C F M  --  Parse command confirmation (end of line)  */

/*
 Returns
   -2: User typed anything but whitespace or newline
   -1: Reparse needed
    0: Confirmation was received
*/
int
cmcfm() {
    int x, xc;
    debug(F101,"cmcfm: cmflgs","",cmflgs);
    debug(F110,"cmcfm: atmbuf",atmbuf,0);
    inword = xc = cc = 0;

    setatm("",0);			/* (Probably unnecessary) */

    while (cmflgs != 1) {
        x = gtword(0);
        xc += cc;

        switch (x) {
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -2:
	  case -1:
	    return(x);
	  case 1:			/* End of line */
	    if (xc > 0) {
		if (xcmfdb) {
		    return(-6);
		} else {
		    printf("?Not confirmed - %s\n",atmbuf);
		    return(-9);
		}
	    } else break;		/* Finish up below */
	  case 2:			/* ESC */
	    if (xc == 0) {
		bleep(BP_WARN);
		continue;		/* or fall thru. */
	    }
	  case 0:			/* Space */
	    if (xc == 0)		/* If no chars typed, continue, */
	      continue;			/* else fall thru. */
	    /* else fall thru... */

	  case 3:			/* Question mark */
	    if (xc > 0) {
		if (xcmfdb) {
		    return(-6);
		} else {
		    printf("?Not confirmed - %s\n",atmbuf);
		    return(-9);
		}
	    }
	    printf(
	       "\n Press the Return or Enter key to confirm the command\n");
	    printf("%s%s",cmprom,cmdbuf);
	    fflush(stdout);
	    continue;
	}
    }
#ifdef CK_RECALL
    addcmd(cmdbuf);
#endif /* CK_RECALL */
    return(0);
}


/* The following material supports chained parsing functions. */
/* See ckucmd.h for FDB and OFDB definitions. */

struct OFDB cmresult = {		/* Universal cmfdb result holder */
    NULL,
    0,
    NULL,
    0
};

VOID
cmfdbi(p,fc,s1,s2,s3,n1,n2,f,k,nxt)	/* Initialize an FDB */
    struct FDB * p;
    int fc;
    char * s1, * s2, * s3;
    int n1, n2;
    xx_strp f;
    struct keytab * k;
    struct FDB * nxt; {

    p->fcode = fc;
    p->hlpmsg = s1;
    p->dflt = s2;
    p->sdata = s3;
    p->ndata1 = n1;
    p->ndata2 = n2;
    p->spf = f;
    p->kwdtbl = k;
    p->nxtfdb = nxt;
}

/*  C M F D B  --  Parse a field with several possible functions  */

int
cmfdb(fdbin) struct FDB * fdbin; {
#ifndef NOSPL
    extern int x_ifnum;                 /* IF NUMERIC - disables warnings */
#endif /* NOSPL */
    struct FDB * in = fdbin;
    struct OFDB * out = &cmresult;
    int x = 0, n;
    char *s, *xp, *m = NULL;
    int errbits = 0;

    xp = bp;

    out->fcode = -1;			/* Initialize output struct */
    out->fdbaddr = NULL;
    out->sresult = NULL;
    out->nresult = 0;
/*
  Currently we make one trip through the FDBs.  So if the user types Esc or
  Tab at the beginning of a field, only the first FDB is examined for a
  default.  If the user types ?, help is given only for one FDB.  We should
  search through the FDBs for all matching possibilities -- and in particular
  display the pertinent context-sensitive help for each function, rather than
  the only the first one that works, and then rewind the FDB pointer so we
  are not locked out of the earlier ones.
*/
    cmfldflgs = 0;
    while (1) {				/* Loop through the chain of FDBs */
	nomsg = 1;
	xcmfdb = 1;
	s = NULL;
	n = 0;
	debug(F101,"cmfdb in->fcode","",in->fcode);
	switch (in->fcode) {		/* Current parsing function code */
	  case _CMNUM:
#ifndef NOSPL
            x_ifnum = 1;                /* Disables warning messages */
#endif /* NOSPL */
	    x = cmnum(in->hlpmsg,in->dflt,10,&n,in->spf);
#ifndef NOSPL
            x_ifnum = 0;
#endif /* NOSPL */
	    debug(F101,"cmfdb cmnum","",x);
	    if (x < 0) errbits |= 1;
	    break;
	  case _CMOFI:
	    x = cmofi(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmofi","",x);
	    if (x < 0) errbits |= 2;
	    break;
	  case _CMIFI:
	    x = cmifi2(in->hlpmsg,
		       in->dflt,
		       &s,
		       &n,
		       in->ndata1,
		       in->sdata,
		       in->spf,
		       in->ndata2
		       );
	    debug(F101,"cmfdb cmifi2 x","",x);
	    debug(F101,"cmfdb cmifi2 n","",n);
	    if (x < 0) errbits |= 4;
	    break;
	  case _CMFLD:
	    cmfldflgs = in->ndata1;
	    x = cmfld(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmfld","",x);
	    if (x < 0) errbits |= 8;
	    break;
	  case _CMTXT:
	    x = cmtxt(in->hlpmsg,in->dflt,&s,in->spf);
	    debug(F101,"cmfdb cmtxt","",x);
	    if (x < 0) errbits |= 16;
	    break;
	  case _CMKEY:
	    x = cmkey2(in->kwdtbl,
		       in->ndata1,
		       in->hlpmsg,in->dflt,in->sdata,in->spf,in->ndata2);
	    debug(F101,"cmfdb cmkey","",x);
	    if (x < 0) errbits |= ((in->ndata2 & 4) ? 32 : 64);
	    break;
	  case _CMCFM:
	    x = cmcfm();
	    debug(F101,"cmfdb cmcfm","",x);
	    if (x < 0) errbits |= 128;
	    break;
	  default:
	    debug(F101,"cmfdb - unexpected function code","",in->fcode);
	    printf("?cmfdb - unexpected function code: %d\n",in->fcode);
	}
	debug(F101,"cmfdb x","",x);
	debug(F101,"cmfdb cmflgs","",cmflgs);
	debug(F101,"cmfdb crflag","",crflag);
	debug(F101,"cmfdb qmflag","",qmflag);
	debug(F101,"cmfdb esflag","",esflag);

	if (x > -1) {			/* Success */
	    out->fcode = in->fcode;	/* Fill in output struct */
	    out->fdbaddr = in;
	    out->sresult = s;
	    out->nresult = (in->fcode == _CMKEY) ? x : n;
	    out->kflags = (in->fcode == _CMKEY) ? cmkwflgs : 0;
	    debug(F101,"cmfdb out->nresult","",out->nresult);
	    nomsg = 0;
	    xcmfdb = 0;
	    debug(F111,"cmfdb cmdbuf & crflag",cmdbuf,crflag);
	    if (crflag) {
		cmflgs = 1;
#ifdef CK_RECALL
		debug(F101,"cmfdb code","",in->fcode);
		if (in->fcode != _CMCFM)
		  addcmd(cmdbuf);
#endif /* CK_RECALL */
	    }
	    return(x);			/* and return */
	}
	in = in->nxtfdb;		/* Failed, get next parsing function */
	nomsg = 0;
	xcmfdb = 0;
	if (!in) {			/* No more */
	    debug(F101,"cmfdb failure x","",x);
	    debug(F101,"cmfdb failure errbits","",errbits);
	    if (x == -6)
	      x = -9;
	    if (x == -9) {
		/* Make informative messages for a few common cases */
		switch (errbits) {
		  case 4+32: m = "Does not match filename or switch"; break;
		  case 4+64: m = "Does not match filename or keyword"; break;
		  case 1+32: m = "Not a number or valid keyword"; break;
		  case 1+64: m = "Not a number or valid switch"; break;
		  default: m = "Not valid in this position";
		}
		printf("?%s: \"%s\"\n",m, atmbuf);
	    }
	    return(x);
	}
	if (x != -2 && x != -6 && x != -9 && x != -3) /* Editing or somesuch */
	  return(x);			/* Go back and reparse */
	pp = np = bp = xp;		/* Back up pointers */
	cmflgs = -1;			/* Force a reparse */


#ifndef NOSPL
	if (!askflag) {			/* If not executing ASK-class cmd... */
#endif /* NOSPL */
	    if (crflag) {		/* If CR was typed, put it back */
		pushc = LF;		/* But as a linefeed */
	    } else if (qmflag) {	/* Ditto for Question mark */
		pushc = '?';
	    } else if (esflag) {	/* and Escape or Tab */
		pushc = ESC;
	    }
#ifndef NOSPL
	}
#endif /* NOSPL */
    }
}

#ifdef OLDHELP
/* Keyword help routines */

/*  C L R H L P -- Initialize/Clear the help line buffer  */

static VOID
clrhlp() {                              /* Clear the help buffer */
    hlpbuf[0] = NUL;
    hh = hx = 0;
}


/*  A D D H L P  --  Add a string to the help line buffer  */

static VOID
addhlp(s) char *s; {                    /* Add a word to the help buffer */
    int j;

    hh++;                               /* Count this column */

    for (j = 0; (j < hc) && (*s != NUL); j++) { /* Fill the column */
        hlpbuf[hx++] = *s++;
    }
    if (*s != NUL)                      /* Still some chars left in string? */
        hlpbuf[hx-1] = '+';             /* Mark as too long for column. */

    if (hh < (hw / hc)) {               /* Pad col with spaces if necessary */
        for (; j < hc; j++) {
            hlpbuf[hx++] = SP;
        }
    } else {                            /* If last column, */
        hlpbuf[hx++] = NUL;             /* no spaces. */
        dmphlp();                       /* Print it. */
        return;
    }
}

/*  D M P H L P  --  Dump the help line buffer  */

static VOID
dmphlp() {                              /* Print the help buffer */
    hlpbuf[hx++] = NUL;
    if ( hlpbuf[0] )
       printf(" %s\n",hlpbuf);
    clrhlp();
}
#endif /* OLDHELP */

/*  G T W O R D  --  Gets a "word" from the command input stream  */

/*
Usage: retcode = gtword(brk);
  brk = 0 for normal word breaks (space, CR, Esc, ?)
  brk = 1 to add / : = (for parsing switches)

Returns:
-10 Timelimit set and timed out
 -9 if input was too long
 -4 if end of file (e.g. pipe broken)
 -3 if null field
 -2 if command buffer overflows
 -1 if user did some deleting
  0 if word terminates with SP or tab
  1 if ... CR
  2 if ... ESC
  3 if ... ? (question mark)
  4 if ... : or = and called with brk != 0

With:
  pp pointing to beginning of word in buffer
  bp pointing to after current position
  atmbuf containing a copy of the word
  cc containing the number of characters in the word copied to atmbuf
*/

int
ungword() {				/* Unget a word */
    debug(F101,"ungword cmflgs","",cmflgs);
    if (ungw) return(0);
    cmfsav = cmflgs;
    ungw = 1;
    cmflgs = 0;
    return(0);
}

/* Un-un-get word.  Undo ungword() if it has been done. */

VOID
unungw() {
    debug(F110,"unungw atmbuf",atmbuf,cmflgs);
    if (ungw) {
	ungw = 0;
	cmflgs = cmfsav;
	atmbuf[0] = NUL;
    }
}

static int
gtword(brk) int brk; {
    int c;                              /* Current char */
    int quote = 0;                      /* Flag for quote character */
    int echof = 0;                      /* Flag for whether to echo */
    int comment = 0;			/* Flag for in comment */
    char *cp = NULL;			/* Comment pointer */
    int eintr = 0;
    int bracelvl = 0;			/* nested brace counter [jrs] */
    int iscontd = 0;
    char lastchar = NUL;
    char prevchar = NUL;
    char lbrace, rbrace;

#ifdef RTU
    extern int rtu_bug;
#endif /* RTU */

#ifdef IKSD
    extern int inserver;
#endif /* IKSD */
#ifdef CK_LOGIN
    extern int x_logged;
#endif /* CK_LOGIN */

    extern int kstartactive;

#ifdef datageneral
    extern int termtype;                /* DG terminal type flag */
    extern int con_reads_mt;            /* Console read asynch is active */
    if (con_reads_mt) connoi_mt();      /* Task would interfere w/cons read */
#endif /* datageneral */

    if (cmfldflgs & 1) {
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
    crflag = 0;
    qmflag = 0;
    esflag = 0;

    if (swarg) {			/* No leading space for switch args */
	inword = 1;
	swarg = 0;
    }
    debug(F000,"gtword brkchar","",brkchar);
    debug(F101,"gtword brk","",brk);
    if (ungw) {				/* Have a word saved? */
	int x;
	debug(F110,"gtword ungetting from pp",pp,0);
	while (*pp++ == SP) ;
	if (setatm(pp,2) < 0) {
	    printf("?Saved word too long\n");
	    return(-9);
	}
	strncpy(atmbuf,pp,ATMBL);
	atmbuf[ATMBL] = NUL;
	x = strlen(atmbuf);
	while (x > 0 && atmbuf[x-1] == SP) /* Trim trailing spaces */
	  atmbuf[--x] = NUL;
	ungw = 0;
	cmflgs = cmfsav;
	debug(F111,"gtword returning atmbuf",atmbuf,cmflgs);
	return(cmflgs);
    }
    pp = np;                            /* Start of current field */

    debug(F110,"gtword cmdbuf",cmdbuf,0);
    debug(F110,"gtword bp",bp,0);
    debug(F110,"gtword pp",pp,0);

    while (bp < cmdbuf+CMDBL) {         /* Big get-a-character loop */
	echof = 0;			/* Assume we don't echo because */
	chsrc = 0;			/* character came from reparse buf. */
#ifdef BS_DIRSEP
CMDIRPARSE:
#endif /* BS_DIRSEP */
	c = *bp;
        if (!c) {			/* If no char waiting in reparse buf */
	    if (dpx && (!pushc
#ifndef NOSPL
			|| askflag
#endif /* NOSPL */
			)) /* get from tty, set echo flag */
	      echof = 1;
	    debug(F101,"gtword timelimit","",timelimit);
	    c = cmdgetc(timelimit);	/* Read a command character. */
	    debug(F101,"gtword c","",c);
	    debug(F111,"gtword dpx,echof",ckitoa(dpx),echof);
	    if (timelimit && c < -1) {	/* Timed out */
		return(-10);
	    }

#ifndef NOXFER
/*
  The following allows packet recognition in the command parser.
  Presently it works only for Kermit packets, and if our current protocol
  happens to be anything besides Kermit, we simply force it to Kermit.
  We don't use the APC mechanism here for mechanical reasons, and also
  because this way, it works even with minimally configured interactive
  versions.  Add Zmodem later...
*/
#ifdef CK_AUTODL
	    if (!local && cmdadl	/* Autodownload enabled? */
#ifdef IKS_OPTION
		|| TELOPT_SB(TELOPT_KERMIT).kermit.me_start
#endif /* IKS_OPTION */
		) {
		int k;
		k = kstart((CHAR)c);	/* Kermit S or I packet? */
		if (k) {
		    int ksign = 0;
		    if (k < 0) {	/* Minus-Protocol? */
#ifdef NOSERVER
			goto noserver;	/* Need server mode for this */
#else
			ksign = 1;	/* Remember */
			k = 0 - k;	/* Convert to actual protocol */
			justone = 1;	/* Flag for protocol module */
#endif /* NOSERVER */
		    } else
		      justone = 0;
		    k--;		/* Adjust kstart's return value */
		    if (k == PROTO_K) {
			extern int protocol, g_proto;
			extern CHAR sstate;
			g_proto = protocol;
			protocol = PROTO_K; /* Crude... */
			sstate = ksign ? 'x' : 'v';
			cmdbuf[0] = NUL;
			return(-3);
		    }
		}
	    }
#ifdef NOSERVER
	  noserver:
#endif /* NOSERVER */
#endif /* CK_AUTODL */
#endif /* NOXFER */

	    chsrc = 1;			/* Remember character source is tty. */
	    brkchar = c;

#ifdef IKSD
            if (inserver && c < 0) {    /* End of session? */
                debug(F111,"gtword c < 0","exiting",c);
                return(-4);             /* Cleanup and terminate */
            }
#endif /* IKSD */

#ifdef OS2
           if (c < 0) {			/* Error */
	       if (c == -3) {		/* Empty word? */
		   if (blocklvl > 0)	/* In a block */
		     continue;		/* so keep looking for block end */
		   else
		     return(-3);	/* Otherwise say we got nothing */
	       } else {			/* Not empty word */
		   return(-4);		/* So some kind of i/o error */
	       }
           }
#else
#ifdef MAC
	   if (c == -3)			/* Empty word... */
	     if (blocklvl > 0)
	       continue;
	     else
	       return(-3);
#endif /* MAC */
#endif /* OS2 */
	   if (c == EOF) {		/* This can happen if stdin not tty. */
#ifdef EINTR
/*
  Some operating and/or C runtime systems return EINTR for no good reason,
  when the end of the standard input "file" is encountered.  In cases like
  this, we get into an infinite loop; hence the eintr counter, which is reset
  to 0 upon each call to this routine.
*/
		debug(F101,"gtword EOF","",errno);
		if (errno == EINTR && ++eintr < 4) /* When bg'd process is */
		  continue;		/* fg'd again. */
#endif /* EINTR */
		return(-4);
	    }
	    c &= cmdmsk;		/* Strip any parity bit */
	}				/* if desired. */

/* Now we have the next character */

	debug(F000,"gtword char","",c);

	if (quote && (c == CR || c == LF)) { /* Enter key following quote */
	    *bp++ = CMDQ;		/* Double it */
	    *bp = NUL;
	    quote = 0;
	}
        if (quote == 0) {		/* If this is not a quoted character */
            if (c == CMDQ) {		/* Got the quote character itself */
		if (!comment && quoting)
		  quote = 1;		/* Flag it if not in a comment */
            }
	    if (c == FF) {		/* Formfeed. */
                c = NL;                 /* Replace with newline */
		cmdclrscn();		/* Clear the screen */
            }
	    if (c == HT) {		/* Tab */
		if (comment)		/* If in comment, */
		  c = SP;		/* substitute space */
		else			/* otherwise */
		  c = ESC;		/* substitute ESC (for completion) */
	    }
	    if (c == ';' || c == '#') { /* Trailing comment */
		if (inword == 0 && quoting) { /* If not in a word */
		    comment = 1;	/* start a comment. */
		    cp = bp;		/* remember where it starts. */
		}
	    }
	    if (!kstartactive &&	/* Not in possible Kermit packet */
		!comment && c == SP) {	/* Space not in comment */
                *bp++ = (char) c;	/* deposit in buffer if not already */
		debug(F101,"gtword echof 2","",echof);
#ifdef BEBOX
                if (echof) {
                    putchar(c);		/* echo it. */
                    fflush(stdout);
                    fflush(stderr);
                }
#else
                if (echof) {		/* echo it. */
		    putchar((CHAR)c);
		    if (timelimit)
		      fflush(stdout);
		}
#endif /* BEBOX */
                if (inword == 0) {      /* If leading, gobble it. */
                    pp++;
                    continue;
                } else {                /* If terminating, return. */
		    if ((*pp != lbrace) || (bracelvl == 0)) {
			np = bp;
			cmbptr = np;
			if (setatm(pp,0) < 0) {
			    printf("?Field too long error 1\n");
			    debug(F111,"gtword too long #1",pp,strlen(pp));
			    return(-9);
			}
			brkchar = c;
			inword = cmflgs = 0;
			return(0);
		    }
                    continue;
                }
            }
	    if (!kstartactive && !comment && brk && (c == '=' || c == ':')) {
                *bp++ = (char) c;
#ifdef BEBOX
                if (echof) {
                    putchar(c);		/* echo it. */
                    fflush(stdout);
                    fflush(stderr);
                }
#else
		if (echof) {
		    putchar((CHAR)c);
		    if (timelimit)
		      fflush(stdout);
		}
#endif /* BEBOX */
		if ((*pp != lbrace) || (bracelvl == 0)) {
		    np = bp;
		    cmbptr = np;
		    if (setatm(pp,0) < 0) {
			printf("?Field too long error 1\n");
			debug(F111,"gtword too long #1",pp,strlen(pp));
			return(-9);
		    }
		    inword = cmflgs = 0;
		    brkchar = c;
		    return(4);
		}
            }
            if (c == lbrace)
              bracelvl++;
            if (c == rbrace) {
                bracelvl--;
                if (linebegin)
		  blocklvl--;
            }
            if (c == LF || c == CR) {	/* CR or LF. */
		if (echof) {
                    cmdnewl((char)c);	/* echo it. */
#ifdef BEBOX
                    fflush(stdout);
                    fflush(stderr);
#endif /* BEBOX */
                }
		{
		    /* Trim trailing comment and whitespace */
		    char *qq;
		    if (comment) {	/* Erase comment */
			while (bp >= cp) /* Back to comment pointer */
			  *bp-- = NUL;
			bp++;
			pp = bp;	/* Adjust other pointers */
			inword = 0;	/* and flags */
			comment = 0;
			cp = NULL;
		    }
		    qq = inword ? pp : (char *)cmdbuf;
		    /* Erase trailing whitespace */
		    while (bp > qq && (*(bp-1) == SP || *(bp-1) == HT)) {
			bp--;
			debug(F000,"erasing","",*bp);
			*bp = NUL;
		    }
		    lastchar = (bp > qq) ? *(bp-1) : NUL;
		    prevchar = (bp > qq+1) ? *(bp-2) : NUL;
		}
		if (linebegin && blocklvl > 0) /* Blank line in {...} block */
		  continue;

		linebegin = 1;		/* At beginning of next line */
		iscontd = prevchar != CMDQ &&
		  (lastchar == '-' || lastchar == lbrace);
		debug(F101,"gtword iscontd","",iscontd);

                if (iscontd) {		/* If line is continued... */
                    if (chsrc) {	/* If reading from tty, */
                        if (*(bp-1) == lbrace) { /* Check for "begin block" */
                            *bp++ = SP;	/* Insert a space for neatness */
                            blocklvl++;	/* Count block nesting level */
                        } else {	/* Or hyphen */
			    bp--;	/* Overwrite the hyphen */
                        }
                        *bp = NUL;	/* erase the dash, */
                        continue;	/* and go back for next char now. */
                    }
		} else if (blocklvl > 0) { /* No continuation character */
		    if (chsrc) {	/* But we're in a "block" */
			*bp++ = ',';	/* Add comma */
			*bp = NUL;
			continue;
		    }
		} else {		/* No continuation, end of command. */
		    *bp = NUL;		/* Terminate the command string. */
		    if (comment) {	/* If we're in a comment, */
			comment = 0;	/* Say we're not any more, */
			*cp = NUL;	/* cut it off. */
		    }
		    np = bp;		/* Where to start next field. */
		    cmbptr = np;
		    if (setatm(pp,0) < 0) { /* Copy field to atom buffer */
			debug(F111,"gtword too long #2",pp,strlen(pp));
			printf("?Field too long error 2\n");
			return(-9);
		    }
		    inword = 0;		/* Not in a word any more. */
		    crflag = 1;
                    debug(F110,"gtword","crflag is set",0);
		    return(cmflgs = 1);
		}
            }

	    /* Question mark */

            if ((c == '?')
		&& quoting
		&& !kstartactive
		&& !comment
		&& cmdsrc() == 0	/* Commands coming from terminal */
		&& chsrc != 0		/* and NOT from reparse buffer! */
		) {
                putchar((CHAR)c);
                *bp = NUL;
                if (setatm(pp,0) < 0) {
		    debug(F111,"gtword too long #3",pp,strlen(pp));
		    printf("?Too long #3\n");
		    return(-9);
		}
		qmflag = 1;
                return(cmflgs = 3);
            }
            if (c == ESC) {		/* ESC */
		if (!comment) {
		    *bp = NUL;
		    if (setatm(pp,0) < 0) {
			debug(F111,"gtword too long #4",pp,strlen(pp));
			printf("?Too long #4\n");
			return(-9);
		    }
		    esflag = 1;
		    return(cmflgs = 2);
		} else {
		    bleep(BP_WARN);
		    continue;
		}
            }
            if (c == BS || c == RUB) {  /* Character deletion */
                if (bp > cmdbuf) {      /* If still in buffer... */
		    cmdchardel();	/* erase it. */
                    bp--;               /* point behind it, */
#ifdef COMMENT
                    if (*bp == SP) inword = 0; /* Flag if current field gone */
#else
/* fixed by Ulli Schlueter */
                    if (*bp == lbrace) bracelvl--; /* Adjust brace count */
                    if (*bp == rbrace) bracelvl++;
                    if ((*bp == SP) &&       /* Flag if current field gone */
			(*pp != lbrace || bracelvl == 0))
		      inword = 0;
#endif /* COMMENT */
                    *bp = NUL;          /* Erase character from buffer. */
                } else {                /* Otherwise, */
		    bleep(BP_WARN);
                    cmres();            /* and start parsing a new command. */
		    *bp = *atmbuf = NUL;
                }
                if (pp < bp) continue;
                else return(cmflgs = -1);
            }
            if (c == LDEL) {            /* ^U, line deletion */
                while ((bp--) > cmdbuf) {
                    cmdchardel();
                    *bp = NUL;
                }
                cmres();                /* Restart the command. */
		*bp = *atmbuf = NUL;
                inword = 0;
                return(cmflgs = -1);
            }
            if (c == WDEL) {            /* ^W, word deletion */
                if (bp <= cmdbuf) {     /* Beep if nothing to delete */
		    bleep(BP_WARN);
                    cmres();
		    *bp = *atmbuf = NUL;
                    return(cmflgs = -1);
                }
                bp--;
		/* Back up over any trailing nonalphanums */
		/* This is dependent on ASCII collating sequence */
		/* but isalphanum() is not available everywhere. */
                for ( ; (bp >= cmdbuf) &&
		     (*bp < '0') ||
		     (*bp > '9' && *bp < '@') ||
		     (*bp > 'Z' && *bp < 'a') ||
		     (*bp > 'z') ; bp--) {
                    cmdchardel();
                    *bp = NUL;
                }
		/* Now delete back to rightmost remaining nonalphanum */
                for ( ; (bp >= cmdbuf) && (*bp) ; bp--) {
		    if ((*bp < '0') ||
			(*bp > '9' && *bp < '@') ||
			(*bp > 'Z' && *bp < 'a') ||
			(*bp > 'z'))
		      break;
                    cmdchardel();
                    *bp = NUL;
                }
                bp++;
                inword = 0;
                return(cmflgs = -1);
            }
            if (c == RDIS) {            /* ^R, redisplay */
#ifdef COMMENT
                *bp = NUL;
                printf("\n%s%s",cmprom,cmdbuf);
#else
		char *cpx; char cx;
                *bp = NUL;
                printf("\n%s",cmprom);
		cpx = cmdbuf;
		while (cx = *cpx++) {
#ifdef isprint
		    putchar((CHAR) (isprint(cx) ? cx : '^'));
#else
		    putchar((CHAR) ((cx >= SP && cx < DEL) ? cx : '^'));
#endif /* isprint */
		}
#endif /* COMMENT */
		fflush(stdout);
                continue;
            }
#ifdef CK_RECALL
	    if (chsrc && on_recall &&	/* Reading commands from keyboard? */
		(cm_recall > 0) &&	/* Saving commands? */
		(c == C_UP || c == C_UP2)) { /* Go up one */
		if (current < 0) {	/* Nowhere to go, */
		    bleep(BP_WARN);
		    continue;
		}
		if (recall[current]) {
		    if (!strcmp(recall[current],cmdbuf)) {
			if (current > 0) {
			    current--;
			} else {
			    bleep(BP_WARN);
			    continue;
			}
		    }
		}
		if (recall[current]) { /* We have a previous command */
		    while ((bp--) > cmdbuf) { /* Erase current line */
			cmdchardel();
			*bp = NUL;
		    }
		    ckstrncpy(cmdbuf,recall[current],CMDBL);
#ifdef OSK
		    fflush(stdout);
		    write(fileno(stdout), "\r", 1);
		    printf("%s%s",cmprom,cmdbuf);
#else
		    printf("\r%s%s",cmprom,cmdbuf);
#endif /* OSK */
		    current--;
		}
		return(cmflgs = -1);	/* Force a reparse */
	    }
	    if (chsrc && on_recall &&	/* Reading commands from keyboard? */
		(cm_recall > 0) &&	/* Saving commands? */
		(c == C_DN)) {		/* Down one */
		if (current + 1 > rlast) { /* Already at bottom, just beep */
		    bleep(BP_WARN);
		    continue;
		}
		current++;		/* OK to go down */
		if (recall[current]) {
		    if (!strcmp(recall[current],cmdbuf)) {
			if (current + 1 > rlast) { /* At bottom, beep */
			    bleep(BP_WARN);
			    continue;
			} else
			  current++;
		    }
		}
		if (recall[current]) {
		    while ((bp--) > cmdbuf) { /* Erase current line */
			cmdchardel();
			*bp = NUL;
		    }
		    ckstrncpy(cmdbuf,recall[current],CMDBL);
#ifdef OSK
		    fflush(stdout);
		    write(fileno(stdout), "\r", 1);
		    printf("%s%s",cmprom,cmdbuf);
#else
		    printf("\r%s%s",cmprom,cmdbuf);
#endif /* OSK */
		    return(cmflgs = -1); /* Force reparse */
		}
	    }
#endif /* CK_RECALL */

	    if (c < SP && quote == 0) { /* Any other unquoted control char */
		if (!chsrc) {		/* If cmd file, point past it */
		    bp++;
		} else {
		    bleep(BP_WARN);
		}
		continue;		/* continue, don't put in buffer */
	    }
	    linebegin = 0;		/* Not at beginning of line */
#ifdef BEBOX
	    if (echof) {
                cmdecho((char) c, 0);	/* Echo what was typed. */
                fflush (stdout);
                fflush(stderr);
            }
#else
            if (echof) cmdecho((char) c, 0); /* Echo what was typed. */
#endif /* BEBOX */
        } else {			/* This character was quoted. */
	    int qf = 1;
	    quote = 0;			/* Unset the quote flag. */
	    debug(F000,"gtword quote 0","",c);
	    /* Quote character at this level is only for SP, ?, and controls */
            /* If anything else was quoted, leave quote in, and let */
	    /* the command-specific parsing routines handle it, e.g. \007 */
	    if (c > 32 && c != '?' && c != RUB && chsrc != 0) {
		debug(F000,"gtword quote 1","",c);
		*bp++ = CMDQ;		/* Deposit \ if it came from tty */
		qf = 0;			/* and don't erase it from screen */
		linebegin = 0;		/* Not at beginning of line */
#ifdef BS_DIRSEP
/*
  This is a hack to handle "cd \" or "cd foo\" on OS/2 and similar systems.
  If we were called from cmdir() and the previous character was the quote
  character, i.e. backslash, and this character is the command terminator,
  then we stuff an extra backslash into the buffer without echoing, then
  we stuff the carriage return back in again, and go back and process it,
  this time with the quote flag off.
*/
	    } else if (dirnamflg && (c == CR || c == LF || c == SP)) {
		debug(F000,"gtword quote 2","",c);
		*bp++ = CMDQ;
		linebegin = 0;		/* Not at beginning of line */
		*bp = (c == SP ? SP : CR);
        goto CMDIRPARSE ;
#endif /* BS_DIRSEP */
	    } else {
		debug(F000,"gtword quote 3","",c);
	    }
#ifdef BEBOX
	    if (echof) {
                cmdecho((char) c, qf);	/* Echo what was typed. */
                fflush (stdout);
                fflush(stderr);
            }
#else
	    if (echof) cmdecho((char) c, qf); /* Now echo quoted character */
#endif /* BEBOX */
	    debug(F111,"gtword quote",cmdbuf,c);
	}
#ifdef COMMENT
        if (echof) cmdecho((char) c,quote); /* Echo what was typed. */
#endif /* COMMENT */
        if (!comment) inword = 1;	/* Flag we're in a word. */
	if (quote) continue;		/* Don't deposit quote character. */
        if (c != NL) {			/* Deposit command character. */
	    *bp++ = (char) c;		/* and make sure there is a NUL */
#ifdef COMMENT
	    *bp = NUL;			/* after it */
#endif /* COMMENT */
	}
    }                                   /* End of big while */
    bleep(BP_WARN);
    printf("?Command too long, maximum length: %d.\n",CMDBL);
    cmflgs = -2;
    return(-9);
}

/* Utility functions */

/* A D D B U F  -- Add the string pointed to by cp to the command buffer  */

static int
addbuf(cp) char *cp; {
    int len = 0;
    while ((*cp != NUL) && (bp < cmdbuf+CMDBL)) {
        *bp++ = *cp++;                  /* Copy and */
        len++;                          /* count the characters. */
    }
    *bp++ = SP;                         /* Put a space at the end */
    *bp = NUL;                          /* Terminate with a null */
    np = bp;                            /* Update the next-field pointer */
    cmbptr = np;
    return(len);                        /* Return the length */
}

/*  S E T A T M  --  Deposit a token in the atom buffer.  */
/*
  Break on space, newline, carriage return, or NUL.
  Call with:
    cp = Pointer to string to copy to atom buffer.
    fcode = 0 means break on whitespace or EOL.
    fcode = 1 means don't break on space.
    fcode = 2 means break on space, ':', or '='.
  Null-terminate the result.
  If the source pointer is the atom buffer itself, do nothing.
  (no longer true.  now we copy the buffer, edit it, and copy it back.)
  Return length of token, and also set global "cc" to this length.
  Return -1 if token was too long.
*/
static int
setatm(cp,fcode) char *cp; int fcode; {
    char *ap, *xp;
    int  bracelvl, n;
    char lbrace, rbrace;

    if (cmfldflgs & 1) {
	lbrace = '(';
	rbrace = ')';
    } else {
	lbrace = '{';
	rbrace = '}';
    }
    cc = 0;				/* Character counter */
    ap = atmbuf;			/* Address of atom buffer */

    if ((int) strlen(cp) > ATMBL)
      return(-1);

    if (cp == ap) {			/* In case source is atom buffer */
	xp = atybuf;			/* make a copy */
	strncpy(xp,ap,ATMBL);		/* so we can copy it back, edited. */
	cp = xp;
    }
    *ap = NUL;				/* Zero the atom buffer */
    if (fcode == 1) {			/* Trim trailing blanks */
	n = strlen(cp);
	while (--n >= 0)
	  if (cp[n] != SP) break;
	cp[n+1] = NUL;
    }
    while (*cp == SP) cp++;		/* Trim leading spaces */
    bracelvl = 0;
    while (*cp) {
        if (*cp == lbrace) bracelvl++;
        if (*cp == rbrace) bracelvl--;
	if (bracelvl < 0) bracelvl = 0;
	if (bracelvl == 0) {
	    if ((*cp == SP || *cp == HT) && (fcode != 1)) break;
	    if ((fcode == 2) && (*cp == '=' || *cp == ':')) break;
	    if (*cp == LF || *cp == CR) break;
	}
        *ap++ = *cp++;
        cc++;
    }
    *ap = NUL;				/* Terminate the string. */
    return(cc);                         /* Return length. */
}

/*
  These functions attempt to hide system dependencies from the mainline
  code in gtword().  Dummy arg for cmdgetc() needed for compatibility with
  coninc(), ttinc(), etc, since a pointer to this routine can be passed in
  place of those to tn_doop().

  No longer static.  Used by askmore().  Fri Aug 20 15:03:34 1999.
*/
int
cmdgetc(timelimit) int timelimit; {	/* Get a character from the tty. */
    int c;
#ifdef IKSD
    extern int inserver;
#endif /* IKSD */
#ifdef CK_LOGIN
    extern int x_logged;
#endif /* CK_LOGIN */
#ifdef TNCODE
    static int got_cr = 0;
    extern int ckxech;
    int tx = 0, is_tn = 0;
#endif /* TNCODE */

    if (pushc
#ifndef NOSPL
	&& !askflag
#endif /* NOSPL */
	) {
        debug(F111,"cmdgetc()","pushc",pushc);
	c = pushc;
	pushc = NUL;
	if (xcmfdb && c == '?')		/* Don't echo ? twice if chaining. */
	  cmdchardel();
	return(c);
    }
#ifdef datageneral
    {
	char ch;
	c = dgncinb(0,&ch,1);		/* -1 is EOF, -2 TO,
                                         * -c is AOS/VS error */
	if (c == -2) {			/* timeout was enabled? */
	    resto(channel(0));		/* reset timeouts */
	    c = dgncinb(0,&ch,1);	/* retry this now! */
	}
	if (c < 0) return(-4);		/* EOF or some error */
	else c = (int) ch & 0177;	/* Get char without parity */
/*	echof = 1; */
    }
#else /* Not datageneral */
#ifndef MINIX2
    if (
#ifdef IKSD
	(!local && inserver) ||
#endif /* IKSD */
	timelimit > 0) {
#ifdef TNCODE
          GETNEXTCH:
            is_tn = !pushc && !local && sstelnet;
#endif /* TNCODE */
#ifdef COMMENT
	    c = coninc(timelimit > 0 ? 1 : 0);
#else /* COMMENT */
	    /* This is likely to break the asktimeout... */
	    c = coninc(timelimit);
#endif /* COMMENT */
	    /* debug(F101,"cmdgetc coninc","",c); */
#ifdef TNCODE
            if (c >= 0 && is_tn) {	/* Server-side Telnet */
                switch (c) {
		  case IAC:
                    debug(F111,"gtword IAC","c",c);
                    got_cr = 0;
                    if ((tx = tn_doop((CHAR)(c & 0xff),ckxech,coninc)) == 0) {
                        goto GETNEXTCH;
                    } else if (tx <= -1) { /* I/O error */
                        /* If there was a fatal I/O error then ttclos()    */
                        /* has been called and the next GETNEXTCH attempt  */
                        /* will be !is_tn since ttclos() sets sstelnet = 0 */
                        doexit(BAD_EXIT,-1); /* (or return(-4)? */
                    } else if (tx == 1) { /* ECHO change */
                        ckxech = dpx = 1; /* Get next char */
                        goto GETNEXTCH;
                    } else if (tx == 2) { /* ECHO change */
                        ckxech = dpx = 0; /* Get next char */
                        goto GETNEXTCH;
                    } else if (tx == 3) { /* Quoted IAC */
                        c = 255;	/* proceeed with it. */
                    }
#ifdef IKS_OPTION
                    else if (tx == 4) {	/* IKS State Change */
                        goto GETNEXTCH;
                    }
#endif /* IKS_OPTION */
                    else if (tx == 6) {	/* Remote Logout */
			doexit(GOOD_EXIT,0);
                    } else {
			goto GETNEXTCH;	/* Unknown, get next char */
		    }
                    break;
#ifdef COMMENT
                  case CR:
                    if (!TELOPT_U(TELOPT_BINARY)) {
			if ( got_cr ) {
			    /* This means the sender is violating Telnet   */
			    /* protocol because we received two CRs in a   */
			    /* row without getting either LF or NUL.       */
			    /* This will not solve the problem but it      */
			    /* will at least allow two CRs to do something */
			    /* whereas before the user would have to guess */
			    /* to send LF or NUL after the CR.             */
			    debug(F100,"gtword CR telnet error","",0);
			    c = LF;
			} else {
			    debug(F100,"gtword skipping CR","",0);
			    got_cr = 1;	/* Remember a CR was received */
			    goto GETNEXTCH;
			}
                    } else {
			debug(F100,"gtword CR to LF","",0);
			c = LF;
                    }
                    break;
                  case LF:
                    if (!TELOPT_U(TELOPT_BINARY)) {
			got_cr = 0;
			debug(F100,"gtword LF","",0);
                    } else {
			if (got_cr) {
			    got_cr = 0;
			    debug(F100,"gtword skipping LF","",0);
			    goto GETNEXTCH;
			}
                    }
                    break;
                  case NUL:
                    if (!TELOPT_U(TELOPT_BINARY) && got_cr) {
			c = LF;
			debug(F100,"gtword NUL to LF","",0);
                    } else {
			debug(F100,"gtword NUL","",0);
                    }
                    got_cr = 0;
                    break;
#else /* COMMENT */
                  case CR:
                    if ( !TELOPT_U(TELOPT_BINARY) && got_cr ) {
                        /* This means the sender is violating Telnet   */
                        /* protocol because we received two CRs in a   */
                        /* row without getting either LF or NUL.       */
                        /* This will not solve the problem but it      */
                        /* will at least allow two CRs to do something */
                        /* whereas before the user would have to guess */
                        /* to send LF or NUL after the CR.             */
                        debug(F100,"gtword CR telnet error","",0);
                    } else {
                        got_cr = 1;	/* Remember a CR was received */
                    }
                    debug(F100,"gtword CR to LF","",0);
                    c = LF;
		    break;
                  case LF:
                    if (got_cr) {
                        got_cr = 0;
                        debug(F100,"gtword skipping LF","",0);
                        goto GETNEXTCH;
                    }
		    break;
                  case NUL:
                    if (got_cr) {
                        got_cr = 0;
                        debug(F100,"gtword skipping NUL","",0);
                        goto GETNEXTCH;
                    } else {
                      debug(F100,"gtword NUL","",0);
                    }
                    break;
#endif /* COMMENT */
#ifdef IKSD
		  case ETX:		/* Ctrl-C... */
                  case EOT:		/* EOT = EOF */
                      debug(F000,"gtword","",c);
                      if (inserver
#ifdef CK_LOGIN
			  && !x_logged
#endif /* CK_LOGIN */
			  )
                          return(-4);
		    break;
#endif /* IKSD */
		  default:
                      got_cr = 0;
                }
            }
#endif /* TNCODE */
    } else {
	/* debug(F100,"cmdgetc getchar 1","",0); */
#ifdef OS2
	c = coninc(0);
#else /* OS2 */
	c = getchar();
	/* debug(F101,"cmdgetc getchar 2","",c); */
#endif /* OS2 */
    }
#else  /* MINIX2 */
#undef getc
    c = getc(stdin);
#endif /* MINIX2 */
#ifdef RTU
    if (rtu_bug) {
	c = getchar();			/* RTU doesn't discard the ^Z */
	rtu_bug = 0;
    }
#endif /* RTU */
#endif /* datageneral */
    return(c);				/* Return what we got */
}

static VOID
cmdclrscn() {				/* Clear the screen */
    ck_cls();
}

static VOID				/* What to echo at end of command */
#ifdef CK_ANSIC
cmdnewl(char c)
#else
cmdnewl(c) char c;
#endif /* CK_ANSIC */
/* cmdnewl */ {
#ifdef OS2
#ifdef IKSD
    extern int inserver;
    if (inserver && c == LF)
      putchar(CR);
#endif /* IKSD */
#endif /* OS2 */

    putchar(c);				/* c is the terminating character */

#ifdef WINTCP				/* what is this doing here? */
    if (c == CR) putchar(NL);
#endif /* WINTCP */

/*
  A.A. Chernov, who sent in changes for FreeBSD, said we also needed this
  for SVORPOSIX because "setup terminal by termios and curses does
  not convert \r to \n, so additional \n needed in newline function."  But
  it is also very likely to result in unwanted blank lines.
*/
#ifdef BSD44
    if (c == CR) putchar(NL);
#endif /* BSD44 */

#ifdef COMMENT
    /* OS2 no longer needs this as all CR are converted to NL in coninc() */
    /* This eliminates the ugly extra blank lines discussed above.        */
#ifdef OS2
    if (c == CR) putchar(NL);
#endif /* OS2 */
#endif /* COMMENT */
#ifdef aegis
    if (c == CR) putchar(NL);
#endif /* aegis */
#ifdef AMIGA
    if (c == CR) putchar(NL);
#endif /* AMIGA */
#ifdef datageneral
    if (c == CR) putchar(NL);
#endif /* datageneral */
#ifdef GEMDOS
    if (c == CR) putchar(NL);
#endif /* GEMDOS */
#ifdef STRATUS
    if (c == CR) putchar(NL);
#endif /* STRATUS */
}

static VOID
cmdchardel() {				/* Erase a character from the screen */
    if (!dpx) return;
#ifdef datageneral
    /* DG '\b' is EM (^y or \031) */
    if (termtype == 1)
      /* Erase a character from non-DG screen, */
      dgncoub(1,"\010 \010",3);
    else
#endif /* datageneral */
      printf("\b \b");
#ifdef GEMDOS
    fflush(stdout);
#else
#ifdef BEBOX
    fflush(stdout);
#endif /* BEBOX */
#endif /* GEMDOS */
}

static VOID
#ifdef CK_ANSIC
cmdecho(char c, int quote)
#else
cmdecho(c,quote) char c; int quote;
#endif /* CK_ANSIC */
{ /* cmdecho */
    if (!dpx) return;
    /* Echo tty input character c */
    if (quote) {
	putchar(BS);
	putchar(SP);
	putchar(BS);
#ifdef isprint
	putchar((CHAR) (isprint(c) ? c : '^' ));
#else
	putchar((CHAR) ((c >= SP && c < DEL) ? c : '^'));
#endif /* isprint */
    } else putchar(c);
#ifdef OS2
    if (quote==1 && c==CR) putchar((CHAR) NL);
#endif /* OS2 */
    if (timelimit)
      fflush(stdout);
}

/* Return pointer to current position in command buffer. */

char *
cmpeek() {
    return(np);
}
#endif /* NOICP */


#ifdef NOICP
#include "ckcdeb.h"
#include "ckucmd.h"
#include "ckcasc.h"
#endif /* NOICP */

/*  X X E S C  --  Interprets backslash codes  */
/*  Returns the int value of the backslash code if it is > -1 and < 256 */
/*  and updates the string pointer to first character after backslash code. */
/*  If the argument is invalid, leaves pointer unchanged and returns -1. */

int
xxesc(s) char **s; {			/* Expand backslash escapes */
    int x, y, brace, radix;		/* Returns the int value */
    char hd = '9';			/* Highest digit in radix */
    char *p;

    p = *s;				/* pointer to beginning */
    if (!p) return(-1);			/* watch out for null pointer */
    x = *p++;				/* character at beginning */
    if (x != CMDQ) return(-1);		/* make sure it's a backslash code */

    x = *p;				/* it is, get the next character */
    if (x == '{') {			/* bracketed quantity? */
	p++;				/* begin past bracket */
	x = *p;
	brace = 1;
    } else brace = 0;
    switch (x) {			/* Start interpreting */
      case 'd':				/* Decimal radix indicator */
      case 'D':
	p++;				/* Just point past it and fall thru */
      case '0':				/* Starts with digit */
      case '1':
      case '2':  case '3':  case '4':  case '5':
      case '6':  case '7':  case '8':  case '9':
	radix = 10;			/* Decimal */
	hd = '9';			/* highest valid digit */
	break;
      case 'o':				/* Starts with o or O */
      case 'O':
	radix = 8;			/* Octal */
	hd = '7';			/* highest valid digit */
	p++;				/* point past radix indicator */
	break;
      case 'x':				/* Starts with x or X */
      case 'X':
	radix = 16;			/* Hexadecimal */
	p++;				/* point past radix indicator */
	break;
      default:				/* All others */
#ifdef COMMENT
	*s = p+1;			/* Treat as quote of next char */
	return(*p);
#else
	return(-1);
#endif /* COMMENT */
    }
    /* For OS/2, there are "wide" characters required for the keyboard
     * binding, i.e \644 and similar codes larger than 255 (byte).
     * For this purpose, give up checking for < 256. If someone means
     * \266 should result in \26 followed by a "6" character, he should
     * always write \{26}6 anyway.  Now, return only the lower byte of
     * the result, i.e. 10, but eat up the whole \266 sequence and
     * put the wide result 266 into a global variable.  Yes, that's not
     * the most beautiful programming style but requires the least
     * amount of changes to other routines.
     */
    if (radix <= 10) {			/* Number in radix 8 or 10 */
	for ( x = y = 0;
 	      (*p) && (*p >= '0') && (*p <= hd)
#ifdef OS2
                   && (y < 5) && (x*radix < KMSIZE);
              /* the maximum needed value \8196 is 4 digits long */
              /* while as octal it requires \1377, i.e. 5 digits */
#else
                   && (y < 3) && (x*radix < 256);
#endif /* OS2 */
	      p++,y++) {
	    x = x * radix + (int) *p - 48;
	}
#ifdef OS2
        wideresult = x;			/* Remember wide result */
        x &= 255;
#endif /* OS2 */
	if (y == 0 || x > 255) {	/* No valid digits? */
	    *s = p;			/* point after it */
	    return(-1);			/* return failure. */
	}
    } else if (radix == 16) {		/* Special case for hex */
	if ((x = unhex(*p++)) < 0) { *s = p - 1; return(-1); }
	if ((y = unhex(*p++)) < 0) { *s = p - 2; return(-1); }
	x = ((x << 4) & 0xF0) | (y & 0x0F);
#ifdef OS2
        wideresult = x;
        if ((y = unhex(*p)) >= 0) {
           p++;
	   wideresult = ((x << 4) & 0xFF0) | (y & 0x0F);
           x = wideresult & 255;
        }
#endif /* OS2 */
    } else x = -1;
    if (brace && *p == '}' && x > -1)	/* Point past closing brace, if any */
      p++;
    *s = p;				/* Point to next char after sequence */
    return(x);				/* Return value of sequence */
}

int					/* Convert hex string to int */
#ifdef CK_ANSIC
unhex(char x)
#else
unhex(x) char x;
#endif /* CK_ANSIC */
/* unhex */ {

    if (x >= '0' && x <= '9')		/* 0-9 is offset by hex 30 */
      return(x - 0x30);
    else if (x >= 'A' && x <= 'F')	/* A-F offset by hex 37 */
      return(x - 0x37);
    else if (x >= 'a' && x <= 'f')	/* a-f offset by hex 57 */
      return(x - 0x57);			/* (obviously ASCII dependent) */
    else return(-1);
}

/*  L O O K U P  --  Lookup the string in the given array of strings  */

/*
 Call this way:  v = lookup(table,word,n,&x);

   table - a 'struct keytab' table.
   word  - the target string to look up in the table.
   n     - the number of elements in the table.
   x     - address of an integer for returning the table array index,
           or NULL if you don't need a table index.

 The keyword table must be arranged in ascending alphabetical order.
 Alphabetic case doesn't matter.

 Returns the keyword's associated value (zero or greater) if found,
 with the variable x set to the keyword-table index, or:

  -3 if nothing to look up (target was null),
  -2 if ambiguous,
  -1 if not found.

 A match is successful if the target matches a keyword exactly, or if
 the target is a prefix of exactly one keyword.  It is ambiguous if the
 target matches two or more keywords from the table.
*/
int
lookup(table,cmd,n,x) char *cmd; struct keytab table[]; int n, *x; {

    int i, v, len, cmdlen;

/* Get length of search object, if it's null return code -3. */

    if (!cmd)
      return(-3);
    if (((cmdlen = (int) strlen(cmd)) == 0) || (n < 1))
      return(-3);

/* Not null, look it up */

    for (i = 0; i < n-1; i++) {
	len = strlen(table[i].kwd);
        if ((len == cmdlen && !ckstrcmp(table[i].kwd,cmd,len,0)) ||
	    ((v = !ckstrcmp(table[i].kwd,cmd,cmdlen,0)) &&
	     ckstrcmp(table[i+1].kwd,cmd,cmdlen,0))) {
	    if (x) *x = i;
	    return(table[i].kwval);
	}
        if (v) {			/* Ambiguous */
	    if (x) *x = i;		/* Set index of first match */
	    return(-2);
	}
    }

/* Last (or only) element */

    if (!ckstrcmp(table[n-1].kwd,cmd,cmdlen,0)) {
        if (x) *x = n-1;
        return(table[n-1].kwval);
    } else return(-1);
}

/* Like lookup, but requires a full (but case-independent) match */

int
xlookup(table,cmd,n,x) struct keytab table[]; char *cmd; int n, *x; {
    int i, cmdlen;

    if (!cmd)
      return(-3);
    if (((cmdlen = (int) strlen(cmd)) == 0) || (n < 1))
      return(-3);

    for (i = 0; i < n; i++) {
	if (((int)strlen(table[i].kwd) == cmdlen) &&
	    (!ckstrcmp(table[i].kwd,cmd,cmdlen,0))) {
	    if (x) *x = i;
	    return(table[i].kwval);
	}
    }
    return(-1);
}

#ifndef NOICP
int
cmdsquo(x) int x; {
    quoting = x;
    return(1);
}

int
cmdgquo() {
    return(quoting);
}
#endif /* NOICP */
