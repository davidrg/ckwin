#include "ckcsym.h"

char *cmdv = "Command package 6.0.088, 6 Sep 96";

#ifdef OS2
int cmdmsk = 255; /* Command-terminal-to-C-Kermit character mask */
#else
int cmdmsk = 127; /* Command-terminal-to-C-Kermit character mask */
#endif /* OS2 */

#ifdef BS_DIRSEP			/* Directory separator is backslash */
#undef BS_DIRSEP
#endif /* BS_DIRSEP */

#ifdef OS2
#define BS_DIRSEP
#endif /* BS_DIRSEP */

#include "ckcdeb.h"                     /* Formats for debug(), etc. */
#include "ckcker.h"			/* Needed for BIGBUFOK definition */
#include "ckucmd.h"			/* Needed for xx_strp prototype */
#include "ckuusr.h"                     /* Needed for prompt length */

_PROTOTYP( int unhex, (char) );
_PROTOTYP( static int filhelp, (int, char *, char *, int) );
_PROTOTYP( VOID sh_sort, (char **, int) );
_PROTOTYP( static VOID cmdclrscn, (void) );

#ifndef NOICP     /* The rest only if interactive command parsing selected */

#ifndef NOSPL
_PROTOTYP( int chkvar, (char *) );
#endif /* NOSPL */

static int blocklvl = 0;              /* "Block" level */
static int linebegin = 0;             /* Flag for at start of a line */
static int quoting = 1;

#ifdef BS_DIRSEP
static int cmdirflg = 0;
#endif /* BS_DIRSEP */

/*  C K U C M D  --  Interactive command package for Unix  */

/*
  Author: Frank da Cruz (fdc@columbia.edu),
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

Functions:
 cmsetp - Set prompt (cmprom is prompt string)
 cmsavp - Save current prompt
 prompt - Issue prompt
 cmini - Clear the command buffer (before parsing a new command)
 cmres - Reset command buffer pointers (before reparsing)
 cmkey - Parse a keyword or token
 cmnum - Parse a number
 cmifi - Parse an input file name
 cmofi - Parse an output file name
 cmdir - Parse a directory name
 cmfld - Parse an arbitrary field
 cmtxt - Parse a text string
 cmcfm - Parse command confirmation (end of line)

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

/* Local variables */

static
int psetf = 0,                          /* Flag that prompt has been set */
    cc = 0,                             /* Character count */
    dpx = 0,                            /* Duplex (0 = full) */
    inword = 0;				/* In the middle of getting a word */

static
int hw = HLPLW,                         /* Help line width */
    hc = HLPCW,                         /* Help line column width */
    hh,                                 /* Current help column number */
    hx;                                 /* Current help line position */

char *dfprom = "Command? ";             /* Default prompt */

int cmflgs;                             /* Command flags */
int cmfsav;				/* A saved version of them */

#ifdef DCMDBUF
char *cmdbuf;				/* Command buffer */
char *savbuf;				/* Buffer to save copy of command */
char *atmbuf;				/* Atom buffer - for current field */
char *atxbuf;				/* For expanding the atom buffer */
#ifdef OLDHELP
static char *hlpbuf;			/* Help string buffer */
#endif /* OLDHELP */
static char *atybuf;			/* For copying atom buffer */
static char *filbuf;			/* File name buffer */
static char *cmprom;			/* Program's prompt */
static char *cmprxx;			/* Program's prompt, unevaluated */

#ifdef CK_RECALL
/*
  Command recall is available only if we can make profligate use of malloc().
*/
#define R_MAX 10			/* Max number of commands to save */
int cm_recall = R_MAX;			/* Size of command recall buffer */
int on_recall = 1;			/* Recall feature is ON */
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

static char *bp,                        /* Current command buffer position */
    *pp,                                /* Start of current field */
    *np;                                /* Start of next field */

static int ungw,			/* For ungetting words */
    atxn;				/* Expansion buffer (atxbuf) length */

#ifdef OS2
extern int wideresult;
#endif /* OS2 */

extern int cmd_cols, cmd_rows;

#ifdef OLDHELP
_PROTOTYP( static VOID addhlp, (char *) );
_PROTOTYP( static VOID clrhlp, (void) );
_PROTOTYP( static VOID dmphlp, (void) );
#endif /* OLDHELP */
_PROTOTYP( static int gtword, (void) );
_PROTOTYP( static int addbuf, (char *) );
_PROTOTYP( static int setatm, (char *, int) );
_PROTOTYP( static int cmdgetc, (void) );
_PROTOTYP( static VOID cmdnewl, (char) );
_PROTOTYP( static VOID cmdchardel, (void) );
_PROTOTYP( static VOID cmdecho, (char, int) );
_PROTOTYP( static int test, (int, int) );
#ifdef GEMDOS
_PROTOTYP( extern char *strchr, (char *, int) );
#endif /* GEMDOS */

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

  Arranges keywords in columns with width based on longest keyword.
  Does "more?" prompting at end of screen.  
  Uses global cmd_rows and cmd_cols for screen size.
*/
VOID
kwdhelp(s,n,pat,pre,post,off)
    struct keytab s[]; int n, off; char *pat, *pre, *post;
/* kwdhelp */ {

    int width = 0;
    int cc;
    int cols, height, i, j, k, lc, n2 = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;
    
    cc = strlen(pat);

    if (!s) return;			/* Nothing to do */
    if (n < 1) return;			/* Ditto */
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (s2 = (char **) malloc(n * sizeof(char *))) {
	for (i = 0; i < n; i++) {	/* Find longest keyword */
	    s2[i] = NULL;
	    if (/* (cc > 0) && */
		(xxstrcmp(s[i].kwd,pat,cc)) || (s[i].flgs & CM_INV)
		)
	      continue;
	    s2[n2++] = s[i].kwd;	/* Copy pointers to visible ones */
	    j = strlen(s[i].kwd);
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
    if (s2) free(s2);			/* Free array copy */
    if (b) free(b);			/* Free line buffer */
    return;
}

VOID
sh_sort(s,n) char **s; int n; {		/* Shell sort */
    int m, i, j;			/* Much faster than bubble sort */
    char *t;				/* Not as fast as Quicksort */
    if (n < 2)				/* but less code, and fast enough */
      return;
    m = n;				/* Initial group size is whole array */
    while (1) {
	m = m / 2;			/* Divide group size in half */
	if (m < 1)			/* Small as can be, so done */
	  break;
	for (j = 0; j < n-m; j++) {	/* Sort each group */
	    t = s[j+m];
	    for (i = j; i >= 0; i -= m) {
		if (strcmp(s[i],t) < 0)
		  break;
		s[i+m] = s[i];
	    }
	    s[i+m] = t;
	}
    }
}

/*  F I L H E L P  --  Given a file list, print names in columns.  */
/*
  Call with:
    s     - file list
    n     - number of entries
    pat   - pattern (left substring) that must match for each filename
    pre   - prefix to add to each filename
    post  - suffix to add to each filename
    off   - offset on first screenful, allowing room for introductory text

  Arranges filenames in columns with width based on longest filename.
  Does "more?" prompting at end of screen.  
  Uses global cmd_rows and cmd_cols for screen size.
*/
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /* MAXPATHLEN */

static int
filhelp(n,pre,post,off) int n, off; char *pre, *post; {
    char filbuf[MAXPATHLEN + 1];	/* Temp buffer for one filename */
    int width = 0;
    int cols, height, i, j, k, lc, n2 = 0, rc = 0;
    char *b = NULL, *p, *q;
    char *pa, *px;
    char **s2 = NULL;

    if (n < 1) return(0);
    if (off < 0) off = 0;		/* Offset for first page */
    if (!pre) pre = "";			/* Handle null string pointers */
    if (!post) post = "";
    lc = off;				/* Screen-line counter */

    if (s2 = (char **) malloc(n * sizeof(char *))) {
	for (i = 0; i < n; i++) {	/* Loop through filenames */
	    s2[i] = NULL;		/* Initialize each pointer to NULL */
	    znext(filbuf);		/* Get next filename */
	    if (!filbuf[0])		/* Shouldn't happen */
	      break;
	    if (!(s2[n2] = malloc((j = (int) strlen(filbuf)) + 1))) {
		printf("?Memory allocation failure\n");
		rc = -9;
		goto xfilhelp;
	    }
	    if (j <= MAXPATHLEN) {
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
    sh_sort(s2,n);			/* Alphabetize the list */

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
	rc = 0;
	goto xfilhelp;
    } else {				/* Malloc failure, no columns */
	for (i = 0; i < n; i++) {
	    znext(filbuf);
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

#ifdef CK_RECALL
/* Initialize or change size of command recall buffer */
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
#endif /* CK_RECALL */
#endif /* DCMDBUF */

/*  C M S E T P  --  Set the program prompt.  */

VOID
cmsetp(s) char *s; {
    strncpy(cmprxx,s,PROMPTL - 1);
    cmprxx[PROMPTL] = NUL;
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
    strncpy(s,cmprxx,n-1);
    s[n-1] = NUL;
}

/*  P R O M P T  --  Issue the program prompt.  */

VOID
prompt(f) xx_strp f; {
    char *sx, *sy; int n;

    if (psetf == 0) cmsetp(dfprom);     /* If no prompt set, set default. */

    sx = cmprxx;			/* Unevaluated copy */
    if (f) {				/* If conversion function given */
	sy = cmprom;			/* Evaluate it */
	debug(F101,"PROMPT sx","",sx);
	debug(F101,"PROMPT sy","",sy);
	n = PROMPTL;
	if ((*f)(sx,&sy,&n) < 0)	/* If evaluation failed */
	  sx = cmprxx;			/* revert to unevaluated copy */
	else
	  sx = cmprom;
    } else strcpy(cmprom,sx);
#ifdef OSK
    fputs(sx, stdout);
#else
#ifdef MAC
    printf("%s", sx);
#else
    printf("\r%s",sx);			/* Print the prompt. */
    fflush(stdout);			/* Now! */
#endif /* MAC */
#endif /* OSK */
}

#ifndef NOSPL
VOID
pushcmd() {				/* For use with IF command. */
    strncpy(savbuf,np,CMDBL);		/* Save the dependent clause,  */
    cmres();				/* and clear the command buffer. */
    debug(F110, "pushcmd: savbuf:", savbuf, 0);
}
#endif /* NOSPL */

#ifdef COMMENT
/* no longer used... */
VOID
popcmd() {
    strncpy(cmdbuf,savbuf,CMDBL);	/* Put back the saved material */
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
    for (bp = cmdbuf; bp < cmdbuf+CMDBL; bp++) *bp = NUL;
    *atmbuf = *savbuf = *atxbuf = *atybuf = *filbuf = NUL;
#ifdef OLDHELP
    *hlpbuf = NUL;
#endif /* OLDHELP */
    blocklvl = 0;		/* Block level is 0 */
    linebegin = 1;		/* And we're at the beginning of a line */
    dpx = d;			/* Make a global copy of the echo flag */
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

    if (radix != 10) {                  /* Just do base 10 */
        printf("cmnum: illegal radix - %d\n",radix);
        return(-1);
    } /* Easy to add others but there has never been a need for it. */
    x = cmfld(xhlp,xdef,&s,(xx_strp)0);
    debug(F101,"cmnum: cmfld","",x);
    if (x < 0) return(x);		/* Parse a field */
    zp = atmbuf;
/*    
  Edit 192 - Allow any number field to be braced.  This lets us included
  spaces in expressions, but perhaps more important lets us have user-defined
  functions in numeric fields, since these always have spaces in them.
*/
    if (*zp == '{') {			/* Braced field, strip braces */
	x = (int) strlen(atmbuf);
	if (x > 0) {			/* The "if" is to shut up optimizers */
	    *(atmbuf+x-1) = NUL;	/* that complain about a possible */
	    zp++;			/* reference to atbmbuf[-1] even */
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
	(*f)(zp,&zq,&atxn);		/* Convert */
	zp = atxbuf;
    }
    debug(F110,"cmnum zp",zp,0);
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
	char *p; _PROTOTYP(char * evala, (char *));
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
#ifdef OS2
    int tries;
#endif /* OS2 */
#ifdef DTILDE
    _PROTOTYP( char * tilde_expand, (char *) );
    char *dirp;
#endif /* DTILDE */

    if (*xhlp == NUL) xhlp = "Output file";
    *xp = "";

    if ((x = cmfld(xhlp,xdef,&s,(xx_strp)0)) < 0) return(x);
    debug(F111,"cmofi 1",s,x);

    if (*s == '{') {			/* Strip enclosing braces */
	int n;
	n = strlen(s);
	if (s[n-1] == '}') {
	    s[n-1] = NUL;
	    s++;
	}
    }
    debug(F110,"cmofi 1.5",s,0);

#ifdef OS2
    tries = 0;
    {
	char *p = s, q;
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
    }
o_again:
    if (tries == 1)
#endif /* OS2 */
    if (f) {				/* If a conversion function is given */
	zq = atxbuf;			/* do the conversion. */
	atxn = CMDBL;
	if ((x = (*f)(s,&zq,&atxn)) < 0)
	  return(-2);
	s = atxbuf;
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
  if contained anything that could be used by zzstring().
*/
	if (tries++ < 1)
	  goto o_again;
#endif /* OS2 */
#endif /* COMMENT */
	debug(F110,"cmofi 6: failure",s,0);
        printf("?Write permission denied - %s\n",s);
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
int
cmifi(xhlp,xdef,xp,wild,f) char *xhlp, *xdef, **xp; int *wild; xx_strp f; {
    return(cmifi2(xhlp,xdef,xp,wild,0,NULL,f));
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
    return(cmifi2(xhlp,xdef,xp,wild,0,path,f));
}

/*
  cmifi2() is for use when you also want to parse a directory or device
  name as an input file, as in the DIRECTORY command.
*/
int
cmifi2(xhlp,xdef,xp,wild,d,path,f)
    char *xhlp,*xdef,**xp; int *wild, d; char * path; xx_strp f; {
#define NEWCMIFI
#ifdef NEWCMIFI
    /* New version... */
    int i, x, xc; long y; char *sp, *zq;
    char *sv = NULL;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
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
    extern char *mtchs[];
#endif /* UNIX */
#endif /* OSK */
#endif /* OS2 */
#endif /* NOPARTIAL */

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
#ifdef BS_DIRSEP
	cmdirflg = 1;
        x = gtword();                   /* No, get a word */
	cmdirflg = 0;
#else
        x = gtword();                   /* No, get a word */
#endif /* BS_DIRSEP */
    } else {				/* If so, use default, if any. */
        if (setatm(xdef,0) < 0) {
	    printf("?Default input filename too long\n");
	    return(-9);
	}
    }
  i_path:
    *xp = atmbuf;                       /* Point to result. */

    while (1) {
        xc += cc;                       /* Count this character. */
        debug(F111,"cmifi gtword",atmbuf,xc);
	debug(F101,"cmifi switch x","",x);
        switch (x) {
	    case -9:
	       printf("Command or field too long\n");
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) *xp = xdef;     /* If no input, return default. */
                if (**xp == NUL) return(-3); /* If field empty, return -3. */

		if (**xp == '{') {	/* Strip enclosing braces first  */
		    char *s = *xp;	/* which might have been used if */
		    int n;		/* a filespec included spaces... */
		    n = strlen(s);
		    if (s[n-1] == '}') {
			s[n-1] = NUL;
			s++;
			*xp = s;
		    }
		}
#ifdef OS2
/* In OS/2 and Windows, if we're parsing directory name, allow a disk name */

		if (d && ((int)strlen(*xp) == 2))
		  if (isalpha(**xp) && *(*xp + 1) == ':')
		    return(x);
#endif /* OS2 */
#ifndef NOSPL
		if (f) {		/* If a conversion function is given */
		    char *s = *xp;	/* See if there are any variables in */
		    while (*s) {	/* the string and if so, expand them */
			if (chkvar(s)) {
			    zq = atxbuf;
			    atxn = CMDBL;
			    if ((y = (*f)(*xp,&zq,&atxn)) < 0)
			      return(-2);
			    if ((int) strlen(atxbuf) > 0) {
				*xp = atxbuf;
				break;
			    }
			}
			s++;
		    }
		}
#endif /* NOSPL */
		debug(F110,"cmifi atxbuf",atxbuf,0);
		if (!sv) {			     /* Only do this once */
		    sv = malloc((int)strlen(*xp)+1); /* Make a safe copy */
		    if (!sv) {
			printf("?malloc error 73, cmifi\n");
			return(-9);
		    }
		    strcpy(sv,*xp);
		    debug(F110,"cmifi sv",sv,0);
		}
		y = zxpand(*xp);
		*wild = (y > 1);
		debug(F111,"cmifi sv wild",sv,*wild);
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
				strcpy(atmbuf,path);
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
                      return(-2);
		    } else {
                       printf("?No files match - %s\n",sv);
                       if (sv) free(sv);
                       return(-9);
		    }
		} else if (y < 0) {
		    printf("?Too many files match - %s\n",sv);
		    if (sv) free(sv);
		    return(-9);
		} else if (y > 1) {
		    if (sv) free(sv);
		    return(x);
		}

                /* If not wild, see if it exists and is readable. */
 
		debug(F111,"cmifi sv not wild",sv,*wild);

		znext(*xp);		/* Get first (only?) matching file */
                y = zchki(*xp);		/* Check its accessibility */
		zxpand(sv);		/* Rewind so next znext() gets 1st */
		free(sv);		/* done with this */
		sv = NULL;
                if (y == -3) {
                    printf("?Read permission denied - %s\n",*xp);
                    return(-9);
                } else if (y == -2) {
		    if (d) return(0);
                    printf("?File not readable - %s\n",*xp);
                    return(-9);
                } else if (y < 0) {
                    printf("?File not found - %s\n",*xp);
                    return(-9);
                }
                return(x);

#ifndef MAC
            case 2:                     /* ESC */
		debug(F101,"cmifi esc, xc","",xc);
                if (xc == 0) {
                    if (*xdef != '\0') {
                        printf("%s ",xdef); /* If at beginning of field, */
#ifdef GEMDOS
			fflush(stdout);
#endif /* GEMDOS */
			inword = cmflgs = 0;
                        addbuf(xdef);   /* Supply default. */
                        if (setatm(xdef,0) < 0) {
			    printf("Default input filename too long\n");
			    return(-9);
			}
                    } else {            /* No default */
                        bleep(BP_WARN);
                    }
                    break;
                }
#ifndef NOSPL
		if (f) {		/* If a conversion function is given */
		    char *s = *xp;	/* See if there are any variables in */
		    while (*s) {	/* the string and if so, expand it.  */
			if (chkvar(s)) {
			    zq = atxbuf;
			    atxn = CMDBL;
			    if ((x = (*f)(*xp,&zq,&atxn)) < 0)
			      return(-2);
                    /* reduce cc by number of \\ consumed by conversion */
		    /* function (needed for OS/2, where \ is path separator) */
			    if ((int) strlen(atxbuf) > 0) {
				cc -= (strlen(*xp) - strlen(atxbuf));
				*xp = atxbuf;
				break;
			    }
			}
			s++;
		    }
		}
#endif /* NOSPL */
#ifdef DTILDE
		dirp = tilde_expand(*xp); /* Expand tilde, if any, */
		if (*dirp != '\0') {	/* in the atom buffer. */
		    if (setatm(dirp,0) < 0) {
			printf("Expanded input filename too long\n");
			return(-9);
		    }
		}
                *xp = atmbuf;
#endif /* DTILDE */
                sp = *xp + cc;
#ifdef datageneral
                *sp++ = '+';		/* Data General AOS wildcard */
#else
                *sp++ = '*';		/* Others */
#endif /* datageneral */
                *sp-- = '\0';
#ifdef GEMDOS
		if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
		  strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
                y = zxpand(*xp);	/* Add wildcard and expand list. */
		if (y > 0) {
#ifdef OS2
		    znext(filbuf);
		    zxpand(*xp);	/* Must "rewind" */
#else
		    strcpy(filbuf,mtchs[0]);
#endif /* OS2 */
		} else
		  *filbuf = '\0';
                *sp = '\0';             /* Remove wildcard. */
		*wild = (y > 1);
                if (y == 0) {
		    printf("?No files match - %s\n",atmbuf);
                    return(-9);
                } else if (y < 0) {
                    printf("?Too many files match - %s\n",atmbuf);
                    return(-9);
                } else if (y > 1) {     /* Not unique. */
#ifndef NOPARTIAL
/* Partial filename completion */
		    int i, j, k; char c;
		    k = 0;
		    debug(F111,"cmifi partial",filbuf,cc);
#ifdef OS2
                    {
                        int cur = 0,
                          len = 0,
                          len2 = 0,
                          min = strlen(filbuf);
                        char localfn[257];

                        len = min;
                        for (j = 1; j < y; j++) {
                            znext(localfn);
                            len2 = strlen(localfn);
                            for (cur = cc;
				 cur < len && cur < len2 && cur <= min; 
				 cur++
				 ) {
                                if (tolower(filbuf[cur]) != 
                                    tolower(localfn[cur])
				    )
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
		    if (k > 0) {	/* Got more characters */
			sp = filbuf + cc; /* Point to new ones */
#ifdef VMS
			for (i = 0; i < cc; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
			debug(F100,"cmifi vms erase ok","",0);
#endif /* VMS */
			cc = k;		/* How many new ones we just got */
			printf("%s",sp);        /* Print them */
			while (*bp++ = *sp++) ;	/* Copy to command buffer */
			bp--;	    	        /* Back up over NUL */
			debug(F110,"cmifi partial cmdbuf",cmdbuf,0);
			if (setatm(filbuf,0) < 0) {
			    printf("?Partial filename too long\n");
			    return(-9);
			}
			debug(F111,"cmifi partial atmbuf",atmbuf,cc);
			*xp = atmbuf;
		    }
#endif /* NOPARTIAL */
		    bleep(BP_WARN); 
                } else {                /* Unique, complete it.  */
#ifndef VMS
#ifdef CK_TMPDIR    
		    /* isdir() function required for this! */
		    if (isdir(filbuf)) {
#ifdef UNIX
			strcat(filbuf,"/");
#endif
#ifdef OS2
			strcat(filbuf,"/");
#endif
#ifdef AMIGA
			strcat(filbuf,"/");
#endif
#ifdef OSK
			strcat(filbuf,"/");
#endif
#ifdef datageneral
			strcat(filbuf,":");
#endif
#ifdef MAC
			strcat(filbuf,":");
#endif
#ifdef STRATUS
			strcat(filbuf,">");
#endif
#ifdef GEMDOS
			strcat(filbuf,"\\");
#endif
			sp = filbuf + cc;
			bleep(BP_WARN);
			printf("%s",sp);
			cc++;
			while (*bp++ = *sp++) ;
			bp--;
			if (setatm(filbuf,0) < 0) {
			    printf("?Directory name too long\n");
			    return(-9);
			}
			debug(F111,"cmifi directory atmbuf",atmbuf,cc);
			*xp = atmbuf;
		    } else { /* Not a directory... */
#endif /* CK_TMPDIR */
#endif /* VMS */
			sp = filbuf + cc; /* Point past what user typed. */
#ifdef VMS
			for (i = 0; i < cc; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
#endif /* VMS */
			printf("%s ",sp); /* Complete the name. */
#ifdef GEMDOS
			fflush(stdout);
#endif /* GEMDOS */
			addbuf(sp);	/* Add the characters to cmdbuf. */
			if (setatm(filbuf,0) < 0) { /* And to atmbuf. */
			    printf("?Completed filename too long\n");
			    return(-9);
			}
			inword = cmflgs = 0;
			*xp = atmbuf;	/* Return pointer to atmbuf. */
			return(0);
#ifndef VMS
#ifdef CK_TMPDIR
		    }
#endif /* CK_TMPDIR */
#endif /* VMS */
                }
                break;
 
            case 3:                     /* Question mark - file menu wanted */
                if (*xhlp == NUL)
		  printf(" Input file specification");
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
			char *s = *xp;	/* See if there are any variables in */
			while (*s) {	/* the string and if so, expand them */
			    if (chkvar(s)) {
				zq = atxbuf;
				atxn = CMDBL;
				if ((x = (*f)(*xp,&zq,&atxn)) < 0)
				  return(-2);
				if ((int) strlen(atxbuf) > 0) {
				    *xp = atxbuf;
				    break;
				}
			    }
			    s++;
			}
		    }
#endif /* NOSPL */
#ifdef DTILDE
		    dirp = tilde_expand(*xp);    /* Expand tilde, if any */
		    if (*dirp != '\0') {
			if (setatm(dirp,0) < 0) {
			    printf("?Expanded filename too long\n");
			    return(-9);
			}
		    }
		    *xp = atmbuf;
#endif /* DTILDE */
		    debug(F111,"cmifi ? *xp, cc",*xp,cc);
                    sp = *xp + cc;	/* Insert "*" at end */
#ifdef datageneral
                    *sp++ = '+';        /* Insert +, the DG wild card */
#else
                    *sp++ = '*';
#endif /* datageneral */
                    *sp-- = '\0';
#ifdef GEMDOS
		    if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
		      strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
		    debug(F110,"cmifi ? wild",*xp,0);
                    y = zxpand(*xp);
                    *sp = '\0';
                    if (y == 0) {
			printf("?No files match - %s\n",atmbuf);
                        return(-9);
                    } else if (y < 0) {
                        printf("?Too many files match - %s\n",atmbuf);
                        return(-9);
                    } else {
                        printf(", one of the following:\n");
#ifdef OLDHELP
                        clrhlp();
                        for (i = 0; i < y; i++) {
                            znext(filbuf);
#ifdef VMS
			    printf(" %s\n",filbuf); /* VMS names can be long */
#else
                            addhlp(filbuf);
#endif /* VMS */
                        }
                        dmphlp();
#else  /* New way... */
			if (filhelp(y,"","",1) < 0)
			  return(-9);
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
	cmdirflg = 1;
        x = gtword();                   /* No, get a word */
	cmdirflg = 0;
#else
        x = gtword();                   /* No, get a word */
#endif /* BS_DIRSEP */
    *xp = atmbuf;
    }

#else /* Not NEWCMIFI ... */

    int i, x, xc; long y; char *sp, *zq;
    char *sv = NULL;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
#endif /* DTILDE */
#ifdef OS2
    int tries;
#endif /* OS2 */

#ifndef NOPARTIAL
#ifndef OS2
#ifdef OSK
    extern char **mtchs; /* This large array is dynamic for OS-9 */
#else
    extern char *mtchs[];
#endif /* OSK */
#endif /* OS2 */
#endif /* NOPARTIAL */

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;	
    *xp = "";
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
#ifdef BS_DIRSEP
	cmdirflg = 1;
        x = gtword();                   /* No, get a word */
	cmdirflg = 0;
#else
        x = gtword();                   /* No, get a word */
#endif /* BS_DIRSEP */
    } else {				/* If so, use default, if any. */
        if (setatm(xdef,0) < 0) {
	    printf("?Default input filename too long\n");
	    return(-9);
	}
    }
  i_path:
    *xp = atmbuf;                       /* Point to result. */

    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmifi gtword",atmbuf,xc);
	debug(F101,"cmifi switch x","",x);
        switch (x) {
	    case -9:
	       printf("Command or field too long\n");
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) *xp = xdef;     /* If no input, return default. */
                if (**xp == NUL) return(-3); /* If field empty, return -3. */

		if (**xp == '{') {	/* Strip enclosing braces first */
		    char *s = *xp;
		    int n;
		    n = strlen(s);
		    if (s[n-1] == '}') {
			s[n-1] = NUL;
			s++;
			*xp = s;
		    }
		}
#ifdef OS2
		if (d && ((int)strlen(*xp) == 2))
		  if (isalpha(**xp) && *(*xp + 1) == ':')
		    return(x);
		tries = 0;
i_again:
		if (tries > 0)
#endif /* OS2 */
		if (f) {		/* If a conversion function is given */
		    zq = atxbuf;	/* ... */
		    atxn = CMDBL;
		    if ((y = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
		    *xp = atxbuf;
		}
		debug(F110,"cmifi atxbuf",atxbuf,0);
		if (!sv) {			     /* Only do this once */
		    sv = malloc((int)strlen(*xp)+1); /* Make a safe copy */
		    if (!sv) {
			printf("?malloc error 73, cmifi\n");
			return(-9);
		    }
		    strcpy(sv,*xp);
		    debug(F110,"cmifi sv",sv,0);
		}
		y = zxpand(*xp);
		*wild = (y > 1);
		debug(F111,"cmifi sv wild",sv,*wild);
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
				strcpy(atmbuf,path);
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
#ifdef OS2
		    if (tries++ < 1)
		      goto i_again;
#endif /* OS2 */
		    if (sv) free(sv);
		    if (d) {
			return(-2);
		    } else {
			printf("?No files match - %s\n",*xp);
			return(-9);
		    }
		} else if (y < 0) {
		    printf("?Too many files match - %s\n",*xp);
		    if (sv) free(sv);
		    return(-9);
		} else if (y > 1) {
		    if (sv) free(sv);
		    return(x);
		}

                /* If not wild, see if it exists and is readable. */
 
		debug(F111,"cmifi sv not wild",sv,*wild);

		znext(*xp);		/* Get first (only?) matching file */
                y = zchki(*xp);		/* Check its accessibility */
		zxpand(sv);		/* Rewind so next znext() gets 1st */
		free(sv);		/* done with this */
		sv = NULL;
                if (y == -3) {
                    printf("?Read permission denied - %s\n",*xp);
                    return(-9);
                } else if (y == -2) {
		    if (d) return(0);
                    printf("?File not readable - %s\n",*xp);
                    return(-9);
                } else if (y < 0) {
                    printf("?File not found - %s\n",*xp);
                    return(-9);
                }
                return(x);

#ifndef MAC
            case 2:                     /* ESC */
		debug(F101,"cmifi esc, xc","",xc);
#ifdef OS2
		tries = 0;
#endif /* OS2 */
                if (xc == 0) {
                    if (*xdef != '\0') {
                        printf("%s ",xdef); /* If at beginning of field, */
#ifdef GEMDOS
			fflush(stdout);
#endif /* GEMDOS */
			inword = cmflgs = 0;
                        addbuf(xdef);   /* Supply default. */
                        if (setatm(xdef,0) < 0) {
			    printf("Default input filename too long\n");
			    return(-9);
			}
                    } else {            /* No default */
                        bleep(BP_WARN);
                    }
                    break;
                }
#ifdef OS2
e_again:
		if (tries > 0)
#endif /* OS2 */
		if (f) {		/* If a conversion function is given */
		    zq = atxbuf;	/* ... */
		    atxn = CMDBL;
		    if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
                    /* reduce cc by number of \\ consumed by conversion */
		    /* function (needed for OS/2, where \ is path separator) */
                    cc -= (strlen(*xp) - strlen(atxbuf));
		    *xp = atxbuf;
		}
#ifdef DTILDE
		dirp = tilde_expand(*xp); /* Expand tilde, if any, */
		if (*dirp != '\0') {	/* in the atom buffer. */
		    if (setatm(dirp,0) < 0) {
			printf("Expanded input filename too long\n");
			return(-9);
		    }
		}
                *xp = atmbuf;
#endif /* DTILDE */
                sp = *xp + cc;
#ifdef datageneral
                *sp++ = '+';		/* Data General AOS wildcard */
#else
                *sp++ = '*';		/* Others */
#endif /* datageneral */
                *sp-- = '\0';
#ifdef GEMDOS
		if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
		  strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
                y = zxpand(*xp);	/* Add wildcard and expand list. */
		if (y > 0)
#ifdef OS2
		  znext(filbuf);
else
		  strcpy(filbuf,mtchs[0]);
#endif /* OS2 */
		else
		  *filbuf = NUL;
                *sp = '\0';             /* Remove wildcard. */
		*wild = (y > 1);
                if (y == 0) {
#ifdef OS2
		    if (tries++ < 1)
		      goto e_again;
		    else
#endif /* OS2 */
		      printf("?No files match - %s\n",atmbuf);
                    return(-9);
                } else if (y < 0) {
                    printf("?Too many files match - %s\n",atmbuf);
                    return(-9);
                } else if (y > 1) {     /* Not unique. */
#ifndef NOPARTIAL
/* Partial filename completion */
		    int i, j, k; char c;
		    k = 0;
		    debug(F111,"cmifi partial",filbuf,cc);
#ifdef OS2
                    {
                        int cur = 0,
			len = 0,
			len2 = 0,
			min = strlen(filbuf);
                        char localfn[257];

                        len = min;
                        for (j = 1; j < y; j++) {
                            znext(localfn);
                            len2 = strlen(localfn);
                            for (cur=cc; 
                                 cur < len && cur < len2 && cur <= min; 
                                 cur++
				 ) {
                                if (tolower(filbuf[cur]) != 
                                    tolower(localfn[cur])
				    )
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
		    if (k > 0) {	/* Got more characters */
			sp = filbuf + cc; /* Point to new ones */
#ifdef VMS
			for (i = 0; i < cc; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
			debug(F100,"cmifi vms erase ok","",0);
#endif /* VMS */
			cc = k;		/* How many new ones we just got */
			printf("%s",sp);        /* Print them */
			while (*bp++ = *sp++) ;	/* Copy to command buffer */
			bp--;	    	        /* Back up over NUL */
			debug(F110,"cmifi partial cmdbuf",cmdbuf,0);
			if (setatm(filbuf,0) < 0) {
			    printf("?Partial filename too long\n");
			    return(-9);
			}
			debug(F111,"cmifi partial atmbuf",atmbuf,cc);
			*xp = atmbuf;
		    }
#endif /* NOPARTIAL */
		    bleep(BP_WARN); 
                } else {                /* Unique, complete it.  */
#ifndef VMS
#ifdef CK_TMPDIR    
		    /* isdir() function required for this! */
		    if (isdir(filbuf)) {
#ifdef UNIX
			strcat(filbuf,"/");
#endif
#ifdef OS2
			strcat(filbuf,"/");
#endif
#ifdef AMIGA
			strcat(filbuf,"/");
#endif
#ifdef OSK
			strcat(filbuf,"/");
#endif
#ifdef datageneral
			strcat(filbuf,":");
#endif
#ifdef MAC
			strcat(filbuf,":");
#endif
#ifdef STRATUS
			strcat(filbuf,">");
#endif
#ifdef GEMDOS
			strcat(filbuf,"\\");
#endif
			sp = filbuf + cc;
			bleep(BP_WARN);
			printf("%s",sp);
			cc++;
			while (*bp++ = *sp++) ;
			bp--;
			if (setatm(filbuf,0) < 0) {
			    printf("?Directory name too long\n");
			    return(-9);
			}
			debug(F111,"cmifi directory atmbuf",atmbuf,cc);
			*xp = atmbuf;
		    } else { /* Not a directory... */
#endif /* CK_TMPDIR */
#endif /* VMS */
			sp = filbuf + cc; /* Point past what user typed. */
#ifdef VMS
			for (i = 0; i < cc; i++) {
			    cmdchardel(); /* Back up over old partial spec */
			    bp--;
			}
			sp = filbuf;	/* Point to new word start */
#endif /* VMS */
			printf("%s ",sp); /* Complete the name. */
#ifdef GEMDOS
			fflush(stdout);
#endif /* GEMDOS */
			addbuf(sp);	/* Add the characters to cmdbuf. */
			if (setatm(filbuf,0) < 0) { /* And to atmbuf. */
			    printf("?Completed filename too long\n");
			    return(-9);
			}
			inword = cmflgs = 0;
			*xp = atmbuf;	/* Return pointer to atmbuf. */
			return(0);
#ifndef VMS
#ifdef CK_TMPDIR
		    }
#endif /* CK_TMPDIR */
#endif /* VMS */
                }
                break;
 
            case 3:                     /* Question mark */
#ifdef OS2
		tries = 0;
#endif /* OS2 */
                if (*xhlp == NUL)
		  printf(" Input file specification");
                else
		  printf(" %s",xhlp);
#ifdef GEMDOS
		fflush(stdout);
#endif /* GEMDOS */
                if (xc > 0) {
#ifdef OS2
q_again:
		    if (tries > 0)
#endif /* OS2 */
		    if (f) {		/* If a conversion function is given */
			zq = atxbuf;	/* ... */
			atxn = CMDBL;
			if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
			*xp = atxbuf;
		    }
#ifdef DTILDE
		    dirp = tilde_expand(*xp);    /* Expand tilde, if any */
		    if (*dirp != '\0') {
			if (setatm(dirp,0) < 0) {
			    printf("?Expanded filename too long\n");
			    return(-9);
			}
		    }
		    *xp = atmbuf;
#endif /* DTILDE */
		    debug(F111,"cmifi ? *xp, cc",*xp,cc);
                    sp = *xp + cc;	/* Insert "*" at end */
#ifdef datageneral
                    *sp++ = '+';        /* Insert +, the DG wild card */
#else
                    *sp++ = '*';
#endif /* datageneral */
                    *sp-- = '\0';
#ifdef GEMDOS
		    if (! strchr(*xp, '.'))	/* abde.e -> abcde.e* */
		      strcat(*xp, ".*");	/* abc -> abc*.* */
#endif /* GEMDOS */
		    debug(F110,"cmifi ? wild",*xp,0);
                    y = zxpand(*xp);
                    *sp = '\0';
                    if (y == 0) {
#ifdef OS2
			if (tries++ < 1)
			  goto q_again;
			else
#endif /* OS2 */
			  printf("?No files match - %s\n",atmbuf);
                        return(-9);
                    } else if (y < 0) {
                        printf("?Too many files match - %s\n",atmbuf);
                        return(-9);
                    } else {
                        printf(", one of the following:\n");
                        clrhlp();
                        for (i = 0; i < y; i++) {
                            znext(filbuf);
#ifdef VMS
			    printf(" %s\n",filbuf); /* VMS names can be long */
#else
                            addhlp(filbuf);
#endif /* VMS */
                        }
                        dmphlp();
                    }
                } else printf("\n");
                printf("%s%s",cmprom,cmdbuf);
		fflush(stdout);
                break;
#endif /* MAC */
        }
#ifdef BS_DIRSEP
	cmdirflg = 1;
        x = gtword();                   /* No, get a word */
	cmdirflg = 0;
#else
        x = gtword();                   /* No, get a word */
#endif /* BS_DIRSEP */
    *xp = atmbuf;
    }
#endif /* NEWCMIFI */
}

/*  C M D I R  --  Parse a directory specification  */

/*
 This function depends on the external functions:
   zchki()  - Check if input file exists and is readable.
 If these functions aren't available, then use cmfld() to parse dir names.
 Note: this function quickly cobbled together, mainly by deleting lots of
 lines from cmifi().  It seems to work, but various services are missing,
 like completion, lists of matching directories on "?", etc.
*/
/*
 Returns
   -4 EOF
   -3 if no input present when required,
   -2 if out of space or other internal error,
   -1 if reparse needed,
    0 or 1, with xp pointing to name, if directory specified,
    2 if a wildcard was included.
*/
int
cmdir(xhlp,xdef,xp,f) char *xhlp, *xdef, **xp; xx_strp f; {
    int x, xc; char *zq;
#ifdef DTILDE
    char *tilde_expand(), *dirp;
#endif /* DTILDE */

    inword = 0;				/* Initialize counts & pointers */
    cc = 0;
    xc = 0;
    *xp = "";
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
#ifdef BS_DIRSEP
	cmdirflg = 1;
        x = gtword();                   /* No, get a word */
	cmdirflg = 0;
#else
        x = gtword();                   /* No, get a word */
#endif /* BS_DIRSEP */
    } else {
        if (setatm(xdef,0) < 0) {	/* If so, use default, if any. */
	    printf("?Default directory name too long\n");
	    return(-9);
	}
    }
    *xp = atmbuf;                       /* Point to result. */
    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmdir gtword",atmbuf,xc);
        switch (x) {
	    case -9:
	       printf("Command or field too long\n");
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
	    case 1:
                if (xc == 0) *xp = xdef;     /* If no input, return default. */
		else *xp = atmbuf;
                if (**xp == NUL) return(-3); /* If field empty, return -3. */

		if (**xp == '{') {	/* Strip enclosing braces first */
		    char *s = *xp;
		    int n;
		    n = strlen(s);
		    if (s[n-1] == '}') {
			s[n-1] = NUL;
			s++;
			*xp = s;
		    }
		}
#ifndef OS2
		if (f) {		/* If a conversion function is given */
		    zq = atxbuf;	/* ... */
		    atxn = CMDBL;
		    if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
		    *xp = atxbuf;
		    cc = (int)strlen(atxbuf);
		}
	        if (cc == 0) {
		    xc = 0;
		    continue;
		}
#ifdef DTILDE
		dirp = tilde_expand(*xp); /* Expand tilde, if any, */
		if (*dirp == '~') {	/* Still starts with tilde? */
		    char *tp;		/* Yes, convert to lowercase */
		    tp = *xp;		/* and try again. */
		    while (*tp) {
			if (isupper(*tp)) *tp = (char) tolower(*tp);
			tp++;
		    }
		}
		dirp = tilde_expand(*xp); /* Expand tilde, if any, */
#ifdef COMMENT
		if (*dirp != '\0') {	/* in the atom buffer. */
		    if (setatm(dirp,0) < 0) {
			printf("Expanded name too long\n");
			return(-9);
		    }
		}
#else
		/* This allows for directory names that contain spaces. */
		if (*dirp != '\0')
		  strcpy(atmbuf,dirp);
		*xp = atmbuf;
#endif /* COMMENT */
#endif /* DTILDE */
#else  /* OS2 */
		if (isdir(*xp)) {	/* OS/2 version has this function */
		    return(x);
		} else {
		    if (f) {		/* If a conversion function is given */
			zq = atxbuf;	/* ... */
			atxn = CMDBL;
			if ((x = (*f)(*xp,&zq,&atxn)) < 0) return(-2);
			*xp = atxbuf;
			cc = (int)strlen(atxbuf);
		    }
		}
	        if (cc == 0) {
		    xc = 0;
		    continue;
		}
#endif /* OS2 */
		if (iswild(*xp)) return(2);
		else return(x);

            case 2:                     /* ESC */
                bleep(BP_WARN);
		break;

            case 3:                     /* Question mark */
                if (*xhlp == NUL)
		  printf(" Directory name");
                else
		  printf(" %s",xhlp);
                printf("\n%s%s",cmprom,cmdbuf);
		fflush(stdout);
                break;
        }
#ifdef BS_DIRSEP
	cmdirflg = 1;
	x = gtword();
	cmdirflg = 0;
#else
	x = gtword();
#endif /* BS_DIRSEP */

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
    if ((x = cmflgs) != 1) {            /* Already confirmed? */
        x = gtword();                   /* No, get a word */
    } else {
	if (setatm(xdef,0) < 0) {	/* If so, use default, if any. */
	    printf("?Default too long\n");
	    return(-9);
	}
    }
    *xp = atmbuf;                       /* Point to result. */

    while (1) {
        xc += cc;                       /* Count the characters. */
        debug(F111,"cmfld: gtword",atmbuf,xc);
        debug(F101,"cmfld x","",x);
        switch (x) {
	    case -9:
	       printf("Command or field too long\n");
            case -4:                    /* EOF */
            case -2:                    /* Out of space. */
            case -1:                    /* Reparse needed */
                return(x);
            case 0:                     /* SP or NL */
            case 1:
                if (xc == 0) { 		/* If no input, return default. */
		    if (setatm(xdef,0) < 0) {
			printf("?Default too long\n");
			return(-9);
		    }
		}
		*xp = atmbuf;
		if (f) {		/* If a conversion function is given */
		    zq = atxbuf;	/* employ it now. */
		    atxn = CMDBL;
		    if ((*f)(*xp,&zq,&atxn) < 0)
		      return(-2);
		    if (setatm(atxbuf,0) < 0) {
			printf("Value too long\n");
			return(-9);
		    }
		    *xp = atmbuf;
		}
                if (**xp == NUL) {	/* If variable evaluates to null */
		    if (setatm(xdef,0) < 0) {
			printf("?Default too long\n");
			return(-9);
		    }
		    if (**xp == NUL) x = -3; /* If still empty, return -3. */
		}
#ifdef COMMENT
/* The following is apparently not necessary. */
/* Remove it if nothing is broken, esp. TAKE file with trailing comments */
		xx = *xp;
		debug(F111,"cmfld before trim",*xp,x);
		for (i = (int)strlen(xx) - 1; i > 0; i--)
		  if (xx[i] != SP)	/* Trim trailing blanks */
		    break;
		  else
		    xx[i] = NUL;
#endif /* COMMENT */
		debug(F111,"cmfld returns",*xp,x);
                return(x);
            case 2:                     /* ESC */
                if (xc == 0 && *xdef != NUL) {
                    printf("%s ",xdef); /* If at beginning of field, */
#ifdef GEMDOS
		    fflush(stdout);
#endif /* GEMDOS */
                    addbuf(xdef);       /* supply default. */
		    inword = cmflgs = 0;
		    if (setatm(xdef,0) < 0) {
			printf("?Default too long\n");
			return(-9);
		    } else		/* Return as if whole field */
		      return(0);	/* typed, followed by space. */
                } else {
		    bleep(BP_WARN);
                }
                break;
            case 3:                     /* Question mark */
                if (*xhlp == NUL)
                    printf(" Please complete this field");
                else
                    printf(" %s",xhlp);
                printf("\n%s%s",cmprom,cmdbuf);
		fflush(stdout);
                break;
        }
    x = gtword();
/*  *xp = atmbuf; */
    }
}


/*  C M T X T  --  Get a text string, including confirmation  */

/*
  Print help message 'xhlp' if ? typed, supply default 'xdef' if null
  string typed.  Returns

   -1 if reparse needed or buffer overflows.
    1 otherwise.

  with cmflgs set to return code, and xp pointing to result string.
*/
int
cmtxt(xhlp,xdef,xp,f) char *xhlp; char *xdef; char **xp; xx_strp f; {

    int x, i;
    char *xx, *zq;
    static int xc;

    debug(F101,"cmtxt, cmflgs","",cmflgs);
    inword = 0;				/* Start atmbuf counter off at 0 */
    cc = 0;		
    if (cmflgs == -1) {                 /* If reparsing, */
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
    if ((x = cmflgs) != 1) {
        x = gtword();                   /* Get first word. */
        *xp = pp;                       /* Save pointer to it. */
	debug(F111,"cmtxt:",*xp,cc);
    }
    debug(F101,"cmtxt (*f)","", f);
    while (1) {				/* Loop for each word in text. */
        xc += cc;                       /* Char count for all words. */
        debug(F111,"cmtxt: gtword",atmbuf,xc);
        debug(F101," x","",x);
        switch (x) {
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
		*xp = atxbuf;		/* and return pointer to it. */
		debug(F111,"cmtxt (*f) returns",*xp,cc);
	    }
	    xx = *xp;
	    for (i = (int)strlen(xx) - 1; i > 0; i--)
	      if (xx[i] != SP)		/* Trim trailing blanks */
		break;
	      else
		xx[i] = NUL;
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
		if (xxstrcmp(atmbuf,xdef,x)) /* Matches default? */
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
        x = gtword();
    }
}

/*  C M K E Y  --  Parse a keyword  */

/*
 Call with:
   table    --  keyword table, in 'struct keytab' format;
   n        --  number of entries in table;
   xhlp     --  pointer to help string;
   xdef     --  pointer to default keyword;

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
/* cmkey */  struct keytab table[]; int n; char *xhlp, *xdef; xx_strp f; {
    return(cmkey2(table,n,xhlp,xdef,"",f,0));
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
    int i, tl, y, z, zz, xc;
    char *xp, *zq;

    tl = (int)strlen(tok);
    inword = xc = cc = 0;		/* Clear character counters. */

    debug(F101,"cmkey: cmflgs","",cmflgs);
    debug(F101,"cmkey: cmdbuf","",cmdbuf);

    if ((zz = cmflgs) == 1) {		/* Command already entered? */
	if (setatm(xdef,0) < 0) {	/* Yes, copy default into atom buf */
	    printf("?Default too long\n");
	    return(-9);
	}
    } else
      zz = gtword();			/* Otherwise get a command word */
 
    debug(F101,"cmkey: table length","",n);
    debug(F101," cmflgs","",cmflgs);
    debug(F101," zz","",zz);
    while (1) {
	xc += cc;
	debug(F111,"cmkey: gtword",atmbuf,xc);

	switch(zz) {
	  case -9:
	    printf("Command or field too long\n");
	  case -4:			/* EOF */
	  case -3:			/* Null Command/Quit/Timeout */
	  case -2:			/* Buffer overflow */
	  case -1:			/* Or user did some deleting. */
	    return(cmflgs = zz);

	  case 0:			/* User terminated word with space */
	  case 1:			/* or newline */
	    if (cc == 0) {		/* Supply default if we got nothing */
		if (setatm(xdef,0) < 0) {
		    printf("?Default too long\n");
		    return(-9);
		}
	    }
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* apply it */
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		debug(F110,"cmkey atxbuf after *f",atxbuf,0);
		if (setatm(atxbuf,0) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Look up the word in the table */
	    switch (y) {
	      case -2:			/* Ambiguous */
		if (pmsg) {
		    bleep(BP_FAIL);
                    printf("?Ambiguous - %s\n",atmbuf);
		}
		cmflgs = -2;
		return(-9);
	      case -1:			/* Not found at all */
		if (tl) {
		    for (i = 0; i < tl; i++) /* Check for token */
		      if (tok[i] == *atmbuf) { /* Got one */
			  ungword();  /* Put back the following word */
			  return(-5); /* Special return code for token */
		      }
		}
		/* Kludge alert... only print error if */
		/* we were called as cmkey2, but not cmkey... */
		/* This doesn't seem to always work. */
		if (tl == 0) {
#ifdef OS2
		    if (isalpha(*atmbuf) && *(atmbuf+1) == ':')
		      return(-7);
#endif /* OS2 */
		    if (pmsg) {
			bleep(BP_FAIL);
			printf("?No keywords match - %s\n",atmbuf); /* cmkey */
		    }
		    return(cmflgs = -9);
		} else {
		    if (cmflgs == 1)	/* cmkey2 */
		      return(cmflgs = -6);
		    else
		      return(cmflgs = -2);
		    /* The -6 code is to let caller try another table */
		}
	      default:
		break;
	    }
	    return(y);

	  case 2:			/* User terminated word with ESC */
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
		    if (pmsg) bleep(BP_WARN);
		    break;
		}
            }
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* apply it */
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0)
		  return(-2);
		if (setatm(atxbuf,0) < 0) {
		    printf("Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Something in atmbuf */
	    debug(F111,"cmkey: esc",atmbuf,y);
	    if (y == -2 || y == -3) {	/* Ambiguous */
		bleep(BP_WARN);
		break;
	    }
	    if (y == -1) {		/* Not found */
		if (pmsg) {
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
		      (!test(table[zz].flgs,CM_ABR))) {
		      z = zz;
		      break;
		  }
	    }
	    xp = table[z].kwd + cc;
	    printf("%s ",xp);
#ifdef GEMDOS
	    fflush(stdout);
#endif /* GEMDOS */
	    addbuf(xp);
	    inword = cmflgs = 0;
	    debug(F110,"cmkey: addbuf",cmdbuf,0);
	    return(y);

	  case 3:			/* User typed "?" */
	    if (f) {			/* If a conversion function is given */
		zq = atxbuf;		/* do the conversion now. */
		atxn = CMDBL;
		if ((*f)(atmbuf,&zq,&atxn) < 0) return(-2);
		if (setatm(atxbuf,0) < 0) {
		    printf("?Evaluated keyword too long\n");
		    return(-9);
		}
	    }
	    y = lookup(table,atmbuf,n,&z); /* Look up what we have so far. */

	    if (y == -1) {
		if (pmsg) {
		    bleep( BP_FAIL ) ;
		    printf(" No keywords match\n");
		}
		cmflgs = -2;
		return(-9);
	    }
	    if (*xhlp == NUL)
	      printf(" One of the following:\n");
	    else
	      printf(" %s, one of the following:\n",xhlp);
    	  
#ifdef OLDHELP
	    if ((y > -1) &&
		!test(table[z].flgs,CM_ABR) &&
		((z >= n-1) || xxstrcmp(table[z].kwd,table[z+1].kwd,cc))
		) {
		printf(" %s\n",table[z].kwd);
	    } else {
		clrhlp();
		for (i = 0; i < n; i++) {
		    if (!xxstrcmp(table[i].kwd,atmbuf,cc)
			&& !test(table[i].flgs,CM_INV)
			)
		      addhlp(table[i].kwd);
		}       
		dmphlp();
	    }
#else  /* New way ... */
	    kwdhelp(table,n,atmbuf,"","",1);
#endif /* OLDHELP */
	    if (*atmbuf == NUL) {
		if (tl == 1)
		  printf("or the token %c\n",*tok);
		else if (tl > 1)
		  printf("or one of the tokens: %s\n",tok);
	    }
	    printf("%s%s", cmprom, cmdbuf);
	    fflush(stdout);
	    break;

	  default:
	    printf("\n%d - Unexpected return code from gtword\n",zz);
	    return(cmflgs = -2);
	}
	zz = gtword();
    }
}

int
chktok(tlist) char *tlist; {
    char *p;
    p = tlist;
    while (*p != NUL && *p != *atmbuf) p++;
    return((*p) ? (int) *p : 0);
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
    if (cmflgs == 1) return(0);

    setatm("",0);			/* (Probably unnecessary) */

    while (1) {
        x = gtword();
        xc += cc;
        switch (x) {
	    case -9:
	      printf("Command or field too long\n");
            case -4:                    /* EOF */
            case -2:
            case -1:
                return(x);

            case 1:                     /* End of line */
                if (xc > 0) {
                    printf("?Not confirmed - %s\n",atmbuf);
                    return(-9);
                } else return(0);
            case 2:			/* ESC */
		if (xc == 0) {
                bleep(BP_WARN);
		    continue;		/* or fall thru. */
		}
            case 0:                     /* Space */
		if (xc == 0)		/* If no chars typed, continue, */
		  continue;		/* else fall thru. */
            case 3:			/* Question mark */
                if (xc > 0) {
                    printf("?Not confirmed - %s\n",atmbuf);
                    return(-9);
                }
                printf("\n Type a carriage return to confirm the command\n");
                printf("%s%s",cmprom,cmdbuf);
		fflush(stdout);
                continue;
        }
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
Usage: retcode = gtword();

Returns:
 -9 if input was too long
 -4 if end of file (e.g. pipe broken)
 -3 if null command
 -2 if command buffer overflows
 -1 if user did some deleting
  0 if word terminates with SP or tab
  1 if ... CR
  2 if ... ESC
  3 if ... ? (question mark)

With:
  pp pointing to beginning of word in buffer
  bp pointing to after current position
  atmbuf containing a copy of the word
  cc containing the number of characters in the word copied to atmbuf
*/

int
ungword() {				/* Unget a word */
    if (ungw) return(0);
    cmfsav = cmflgs;
    debug(F101,"ungword cmflgs","",cmflgs);
    ungw = 1;
    cmflgs = 0;
    return(0);
}

static int
gtword() {
    int c;                              /* Current char */
    int quote = 0;                      /* Flag for quote character */
    int echof = 0;                      /* Flag for whether to echo */
    int chsrc = 0;			/* Source of character, 1 = tty */
    int comment = 0;			/* Flag for in comment */
    char *cp = NULL;			/* Comment pointer */
    int eintr = 0;
    int bracelvl = 0;			/* nested brace counter [jrs]	    */

#ifdef RTU
    extern int rtu_bug;
#endif /* RTU */

#ifdef datageneral
    extern int termtype;                /* DG terminal type flag */
    extern int con_reads_mt;            /* Console read asynch is active */
    if (con_reads_mt) connoi_mt();      /* Task would interfere w/cons read */
#endif /* datageneral */

    if (ungw) {				/* Have a word saved? */
	debug(F110,"gtword ungetting from pp",pp,0);
	while (*pp++ == SP) ;
	if (setatm(pp,0) < 0) {
	    printf("?Saved word too long\n");
	    return(-9);
	}
	strncpy(atmbuf,pp,ATMBL);
	ungw = 0;
	cmflgs = cmfsav;
	debug(F111,"gtword returning atmbuf",atmbuf,cmflgs);
	return(cmflgs);
    }
    pp = np;                            /* Start of current field */

    debug(F111,"gtword: cmdbuf",cmdbuf,cmdbuf);
    debug(F111," bp",bp,bp);
    debug(F111," pp",pp,pp);

    while (bp < cmdbuf+CMDBL) {         /* Big get-a-character loop */
	echof = 0;			/* Assume we don't echo because */
	chsrc = 0;			/* character came from reparse buf. */
#ifdef BS_DIRSEP
CMDIRPARSE:
#endif /* BS_DIRSEP */
        if ((c = *bp) == NUL) {         /* If no char waiting in reparse buf */
           if (dpx) echof = 1;		/* must get from tty, set echo flag. */
           c = cmdgetc();		/* Read a character from the tty. */
           chsrc = 1;			/* Remember character source is tty. */
#ifdef OS2
           if ( c < 0 ) {
	       if ( c == -3 ) {
		   if (blocklvl > 0)
		     continue;
		   else
		     return(-3);	/* Empty command */
	       } else {
		   return -4;		/* Something went wrong */
	       }
           }
#else
#ifdef MAC
	   if (c == -3)			/* If null command... */
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

	if (quote && (c == CR || c == NL)) { /* Enter following quote */
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
#ifdef OS2
		cmdclrscn();		/* Clear the screen */
#endif /* OS2 */
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
	    if (!comment && c == SP) {	/* Space */
                *bp++ = (char) c;	/* deposit in buffer if not already */
#ifdef BEBOX
                if (echof) {
                    putchar(c);		/* echo it. */
                    fflush (stdout);
                    fflush(stderr);
                }
#else
                if (echof) putchar(c);  /* echo it. */
#endif /* BEBOX */
                if (inword == 0) {      /* If leading, gobble it. */
                    pp++;
                    continue;
                } else {                /* If terminating, return. */
#ifdef COMMENT
                    np = bp;
                    setatm(pp,0);
                    inword = cmflgs = 0;
		    return(0);
#else
/* This allows { ... } grouping */
		    if ((*pp != '{') || (bracelvl == 0)) {
			np = bp;
			if (setatm(pp,0) < 0) {
			    printf("?Field too long error 1\n");
			    debug(F111,"gtword too long #1",pp,strlen(pp));
			    return(-9);
			}
			inword = cmflgs = 0;
			return(0);
		    }
                    continue;
#endif /* COMMENT */
                }
            }
            if (c == '{')
              bracelvl++;
            if (c == '}') {
                bracelvl--;
                if (linebegin)
		  blocklvl--;
            }
            if (c == NL || c == CR) {	/* CR or LF. */
#ifdef BEBOX
		if (echof) {
                    cmdnewl((char)c);	/* echo it. */
                    fflush (stdout);
                    fflush(stderr);
                }
#else
                if (echof) cmdnewl((char)c); /* echo it. */
#endif /* BEBOX */
		while (bp > pp && (*(bp-1) == SP || *(bp-1) == HT)) /* Trim */
		  bp--;			/* trailing */
		*bp = NUL;		/* whitespace. */

		if (linebegin && blocklvl > 0) /* Blank line in {...} block */
		  continue;

		linebegin = 1;		/* At beginning of next line */

                if ((bp > pp) &&
		    (*(bp-1) == '-' || *(bp-1) == '{')) { /* Line continued? */
                    if (chsrc) {	/* If reading from tty, */
                        if (*(bp-1) == '{') { /* Check for "begin block" */
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
                     if (setatm(pp,0) < 0) { /* Copy field to atom buffer */
                         debug(F111,"gtword too long #2",pp,strlen(pp));
			printf("?Field too long error 2\n");
			return(-9);
		    }
		    inword = 0;		/* Not in a word any more. */
#ifdef CK_RECALL
		    if (chsrc &&	/* Reading commands from keyboard? */
			(on_recall) &&	       /* Recall is turned on? */
			(cm_recall > 0) &&     /* Saving commands? */
			(int)strlen(cmdbuf)) { /* Non-null command? */
			if (rlast >= cm_recall - 1) { /* Yes, buffer full? */
			    int i;	   /* Yes. */
			    if (recall[0]) { /* Discard oldest command */
				free(recall[0]);
				recall[0] = NULL;
			    }
			    for (i = 0; i < rlast; i++) { /* The rest */
				recall[i] = recall[i+1]; /* move back */
			    }
			    rlast--;	 /* Now we have one less */
			}
			rlast++;	 /* Index of last command in buffer */
			current = rlast; /* Also now the current command */
			if (current >= cm_recall) {
			    printf("Oops, command recall error\n");
			} else {
			    recall[current] = malloc((int)strlen(cmdbuf)+1);
			    if (recall[current])
			      strcpy(recall[current],cmdbuf);
			}
		    }
#endif /* CK_RECALL */
		    return(cmflgs = 1);
		}
            }

	    /* Question mark */

            if (!comment && quoting && echof && (c == '?')) {
                putchar(c);
                *bp = NUL;
                if (setatm(pp,0) < 0) {
		    debug(F111,"gtword too long #3",pp,strlen(pp));
		    printf("?Too long #3\n");
		    return(-9);
		}
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
                    if (*bp == '{') bracelvl--; /* Adjust brace count */
                    if (*bp == '}') bracelvl++;
                    if ((*bp == SP) &&       /* Flag if current field gone */
			(*pp != '{' || bracelvl == 0))
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
                for ( ; (bp >= cmdbuf) && (*bp == SP) ; bp--) {
                    cmdchardel();
                    *bp = NUL;
                }
                for ( ; (bp >= cmdbuf) && (*bp != SP) ; bp--) {
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
		    putchar(isprint(cx) ? cx : '^');
#else
		    putchar((cx >= SP && cx < DEL) ? cx : '^');
#endif /* isprint */
		}
#endif /* COMMENT */
		fflush(stdout);
                continue;
            }
#ifdef CK_RECALL
	    if (chsrc &&		/* Reading commands from keyboard? */
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
		    strcpy(cmdbuf,recall[current]);
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
	    if (chsrc &&		/* Reading commands from keyboard? */
		(cm_recall > 0) &&	/* Saving commands? */
		(c == C_DN)) {		/* Down one */
		if (current + 1 > rlast) { /* Already at bottom, just beep */
		    bleep(BP_WARN);
		    continue;
		}
		current++;		/* OK to go down */
#ifdef COMMENT
		if (recall[current])	/* It's the same as this one? */
		  if (!strcmp(recall[current],cmdbuf))
		    current++;
#else
/* Fix by Ulli Schlueter */
		if (recall[current]) {
		    if (!strcmp(recall[current],cmdbuf)) {
			if (current + 1 > rlast) { /* At bottom, beep */
			    bleep(BP_WARN);
			    continue;
			} else
			  current++;
		    }
		}
#endif /* COMMENT */
		if (recall[current]) {
		    while ((bp--) > cmdbuf) { /* Erase current line */
			cmdchardel();
			*bp = NUL;
		    }
		    strcpy(cmdbuf,recall[current]);
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
		if (!chsrc)		/* If cmd file, point past it */
		  bp++;
		else
		  bleep(BP_WARN);
		continue;		/* continue, don't put in buffer */
	    }
	    linebegin = 0;		/* Not at beginning of line */
#ifdef BEBOX
	    if (echof) {
                cmdecho((char) c, 0); /* Echo what was typed. */
                fflush (stdout);
                fflush(stderr);
            }
#else
            if (echof) cmdecho((char) c, 0); /* Echo what was typed. */
#endif /* BEBOX */
        } else {			/* This character was quoted. */
	    int qf = 1;
	    quote = 0;			/* Unset the quote flag. */

	    /* Quote character at this level is only for SP, ?, and controls */
            /* If anything else was quoted, leave quote in, and let */
	    /* the command-specific parsing routines handle it, e.g. \007 */
	    if (c > 32 && c != '?' && c != RUB && chsrc != 0) {
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
	    } else if (cmdirflg && (c == CR || c == LF || c == SP)) {
		*bp++ = CMDQ;
		linebegin = 0;		/* Not at beginning of line */
		*bp = (c == SP ? SP : CR);
        goto CMDIRPARSE ;
#endif /* BS_DIRSEP */
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
        if (c != NL) *bp++ = (char) c;	/* Deposit command character. */
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
    return(len);                        /* Return the length */
}

/*  S E T A T M  --  Deposit a token in the atom buffer.  */
/*
  Break on space, newline, carriage return, or NUL.
  Except flag != 0 means to allow imbedded spaces in selected fields.
  Null-terminate the result.
  If the source pointer is the atom buffer itself, do nothing.
  Return length of token, and also set global "cc" to this length.
  Return -1 if token was too long.
*/
static int
setatm(cp,flag) char *cp; int flag; {
    char *ap, *xp;
    int  bracelvl, n;

    cc = 0;				/* Character counter */
    ap = atmbuf;			/* Address of atom buffer */

    if ((int) strlen(cp) > ATMBL)
      return(-1);

    if (cp == ap) {			/* In case source is atom buffer */
	xp = atybuf;			/* make a copy */
	strcpy(xp,ap);			/* so we can copy it back, edited. */
	cp = xp;
    }
    *ap = NUL;				/* Zero the atom buffer */
    if (flag) {				/* Trim trailing blanks */
	n = strlen(cp);
	while (--n >= 0)
	  if (cp[n] != SP) break;
	cp[n+1] = NUL;
    }
    while (*cp == SP) cp++;		/* Trim leading spaces */
#ifdef COMMENT
/* This one doesn't work for items like "input 20 {\13\10$ }" */
    bracelvl = (*cp == '{');		/* jrs */
    while ( /* (*cp != SP) && */ (*cp != NL) && (*cp != NUL) && (*cp != CR)) {
        if ((*cp == SP) && (flag == 0) && (bracelvl == 0)) break; /* jrs */
        *ap++ = *cp++;			/* Copy up to SP, NL, CR, or end */
        if (*cp == '{') bracelvl++;	/* jrs */
        if (*cp == '}') bracelvl--;	/* jrs */
        cc++;				/* and count */
    }
#else
    bracelvl = 0;
    while (*cp) {
        if (*cp == '{') bracelvl++;
        if (*cp == '}') bracelvl--;
	if (bracelvl < 0) bracelvl = 0;
	if (bracelvl == 0) {
	    if ((*cp == SP || *cp == HT) && (flag == 0)) break;
	    if (*cp == LF || *cp == CR) break;
	}
        *ap++ = *cp++;
        cc++;
    }
#endif /* COMMENT */
    *ap = NUL;				/* Terminate the string. */
    return(cc);                         /* Return length. */
}

/*  R D I G I T S  -- Verify that all the characters in line ARE DIGITS  */

int
rdigits(s) char *s; {
    while (*s) {
        if (!isdigit(*s)) return(0);
        s++;
    }
    return(1);
}

/* These functions attempt to hide system dependencies from the mainline */
/* code in gtword().  Ultimately they should be moved to ck?tio.c, where */
/* ? = each and every system supported by C-Kermit. */

static int
cmdgetc() {				/* Get a character from the tty. */
    int c;

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
#ifdef OS2
    c = is_a_tty(0) ? coninc(0) : getchar();
    if (c < 0) return(c); /* was (-4); */
#else /* Not OS2 */
    c = getchar();			/* or from tty. */
#ifdef RTU
    if (rtu_bug) {
	c = getchar();			/* RTU doesn't discard the ^Z */
	rtu_bug = 0;
    }
#endif /* RTU */
#endif /* OS2 */
#endif /* datageneral */
    return(c);				/* Return what we got */
}


/*
  No more screen clearing.  If you wanna clear the screen, define a macro
  to do it, like "define cls write screen \27[;H\27[2J".
*/
static VOID
cmdclrscn() {				/* Clear the screen */

#ifdef OS2
    clear();
#else /* OS2 */
#ifdef COMMENT
#ifdef aegis
    putchar(FF);
#else
#ifdef AMIGA
    putchar(FF);
#else
#ifdef OSK
    putchar(FF);
#else
#ifdef datageneral
    putchar(FF);
#endif /* datageneral */
#endif /* OSK */
#endif /* AMIGA */
#endif /* aegis */
#endif /* COMMENT */
#endif /* OS2 */
}

static VOID				/* What to echo at end of command */
#ifdef CK_ANSIC
cmdnewl(char c)
#else
cmdnewl(c) char c;
#endif /* CK_ANSIC */
/* cmdnewl */ {
    putchar(c);				/* c is the terminating character */

#ifdef WINTCP				/* what is this doing here? */
    if (c == CR) putchar(NL);
#endif /* WINTCP */

/*
  A.A. Chernov, who sent in changes for FreeBSD, said we also needed this
  for SVORPOSIX because "setup terminal by termios and curses does
  not convert \r to \n, so additional \n needed in newline function.  But
  it is also very likely to result in unwanted blank lines.
*/

#ifdef OS2
    if (c == CR) putchar(NL);
#endif /* OS2 */
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
#endif
      printf("\b \b");
#ifdef GEMDOS
    fflush(stdout);
#endif /* GEMDOS */
#ifdef BEBOX
    fflush(stdout);
#endif /* BEBOX */
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
	putchar(BS); putchar(SP); putchar(BS);
#ifdef isprint
	putchar( isprint(c) ? c : '^' );
#else
	putchar((c >= SP && c < DEL) ? c : '^');
#endif /* isprint */
    } else putchar(c);
#ifdef OS2
    if (quote==1 && c==CR) putchar(NL);
#endif /* OS2 */
}

#endif /* NOICP */

#ifdef NOICP
#include "ckcdeb.h"
#include "ckucmd.h"
#include "ckcasc.h"
/*** #include <ctype.h> (ckcdeb.h already includes this) ***/
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

/* See if argument string is numeric */
/* Returns 1 if OK, zero if not OK */
/* If OK, string should be acceptable to atoi() */
/* Allows leading space, sign */

int
chknum(s) char *s; {			/* Check Numeric String */
    int x = 0;				/* Flag for past leading space */
    int y = 0;				/* Flag for digit seen */
    char c;
    debug(F110,"chknum",s,0);
    while (c = *s++) {			/* For each character in the string */
	switch (c) {
	  case SP:			/* Allow leading spaces */
	  case HT:
	    if (x == 0) continue;
	    else return(0);
	  case '+':			/* Allow leading sign */
	  case '-':
	    if (x == 0) x = 1;
	    else return(0);
	    break;
	  default:			/* After that, only decimal digits */
	    if (c >= '0' && c <= '9') {
		x = y = 1;
		continue;
	    } else return(0);
	}
    }
    return(y);
}

/*  L O W E R  --  Lowercase a string  */

int
lower(s) char *s; {
    int n = 0;
    while (*s) {
        if (isupper(*s)) *s = (char) tolower(*s);
        s++, n++;
    }
    return(n);
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
        if ((len == cmdlen && !xxstrcmp(table[i].kwd,cmd,len)) ||
	    ((v = !xxstrcmp(table[i].kwd,cmd,cmdlen)) &&
	     xxstrcmp(table[i+1].kwd,cmd,cmdlen))) {
	    if (x) *x = i;
	    return(table[i].kwval);
	}
        if (v) return(-2);
    }

/* Last (or only) element */

    if (!xxstrcmp(table[n-1].kwd,cmd,cmdlen)) {
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
	    (!xxstrcmp(table[i].kwd,cmd,cmdlen))) {
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
