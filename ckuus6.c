#include "ckcsym.h"
#ifndef NOICP
 
/*  C K U U S 6 --  "User Interface" for Unix Kermit (Part 6)  */

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

/* Includes */
 
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
#include "ckcnet.h"			/* Network symbols */
 
#ifdef datageneral
#define fgets(stringbuf,max,fd) dg_fgets(stringbuf,max,fd)
#endif /* datageneral */

/* External Kermit Variables, see ckmain.c for description. */
 
extern xx_strp xxstring;

extern int size, rpsiz, urpsiz, local, stdinf, sndsrc, xitsta,
  displa, binary, parity, escape, flow,
  turn, duplex, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  turnch, dfloc, keep, maxrps, warn, cnflg, tlevel, pflag, msgflg,
  mdmtyp, zincnt, fblksiz, frecl, frecfm, atcapr, atdiso, verwho, quiet;
extern int repars, techo;
 
#ifdef CK_IFRO
extern int remonly;
#endif /* CK_IFRO */

extern long vernum, speed;
extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *dialv, *loginv, *for_def[], *whil_def[], *xif_def[];
extern char *ckxsys, *ckzsys, *cmarg, *cmarg2;
extern char *DIRCMD, *PWDCMD, *DELCMD, *WHOCMD, ttname[], filnam[];
extern CHAR sstate;
extern char *zinptr;

#ifndef NOMSEND				/* Multiple SEND */
extern char *msfiles[];
#endif /* NOMSEND */
extern char fspec[];			/* Most recent filespec */

#ifdef CK_TMPDIR
extern int f_tmpdir;			/* Directory changed temporarily */
extern char savdir[];			/* For saving current directory */
#endif /* CK_TMPDIR */

/* Declarations from cmd package */
 
#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;		/* Command buffers */
#else
extern char cmdbuf[], atmbuf[];		/* Command buffers */
#endif /* DCMDBUF */

#ifndef NOSPL
extern struct mtab *mactab;
extern int nmac;
#endif /* NOSPL */

/* Declarations from ck?fio.c module */
 
extern int backgrd, bgset;		/* Kermit executing in background */
 
#ifdef COMMENT
/*
  These must be on stack!
*/
#ifndef NOSPL
extern char vnambuf[];			/* Buffer for variable names */
extern char *vnp;			/* Pointer to same */
#endif /* NOSPL */
#endif /* COMMENT */

extern char psave[];			/* For saving & restoring prompt */
extern char *tp;			/* Temporary buffer */

/* Keyword tables specific to this module */

/* Modem signal table */

struct keytab mstab[] = {
#ifdef COMMENT
/* The forms preceded by backslash are for MS-DOS Kermit compatibility. */
/* But... \dsr doesn't work because \d = decimal constant introducer */
    "\\cd",  BM_DCD, CM_INV,		/* Carrier Detect */
    "\\cts", BM_CTS, CM_INV,		/* Clear To Send  */
    "\\dsr", BM_DSR, CM_INV,		/* Data Set Ready */
    "\\ri",  BM_RNG, CM_INV,		/* Ring Indicator */
#endif /* COMMENT */
    "cd",    BM_DCD, 0,			/* Carrier Detect */
    "cts",   BM_CTS, 0,			/* Clear To Send  */
    "dsr",   BM_DSR, 0,			/* Data Set Ready */
    "ri",    BM_RNG, 0			/* Ring Indicator */
};
int nms = (sizeof(mstab) / sizeof(struct keytab));

#ifndef NOSPL
struct keytab opntab[] = {
#ifndef NOPUSH
    "!read",  XYFZ_Y, 0,
    "!write", XYFZ_X, 0,
#endif /* NOPUSH */
    "append", XYFZ_A, 0,
    "read",   XYFZ_O, 0,
    "write",  XYFZ_N, 0
};
int nopn = (sizeof(opntab) / sizeof(struct keytab));

struct keytab iftab[] = {		/* IF commands */
    "<",          XXIFLT, 0,
    "=",          XXIFAE, 0,
    ">",          XXIFGT, 0,
    "background", XXIFBG, 0,
    "count",      XXIFCO, 0,
    "defined",    XXIFDE, 0,
#ifdef CK_TMPDIR
    "directory",  XXIFDI, 0,
#endif /* CK_TMPDIR */
#ifdef COMMENT
    "eof",        XXIFEO, 0,
#endif /* COMMENT */
    "equal",      XXIFEQ, 0,
    "error",      XXIFFA, CM_INV,
    "exist",      XXIFEX, 0,
    "failure",    XXIFFA, 0,
    "foreground", XXIFFG, 0,
    "llt",        XXIFLL, 0,
    "lgt",        XXIFLG, 0,
#ifdef ZFCDAT
    "newer",      XXIFNE, 0,
#endif /* ZFCDAT */
    "not",        XXIFNO, 0,
    "ok",         XXIFSU, CM_INV,
    "numeric",    XXIFNU, 0,
    "remote-only",XXIFRO, 0,
    "success",    XXIFSU, 0
};
int nif = (sizeof(iftab) / sizeof(struct keytab));
#endif /* NOSPL */

/* Variables and symbols local to this module */
 
#ifndef NODIAL 
char *dialnum = (char *)0;		/* Remember DIAL number for REDIAL */
extern char * dialdir;			/* Dial directory file name */
extern FILE * dialfd;			/* Dial directory file descriptor */
#endif /* NODIAL */

#ifndef NOSPL
int ifc,				/* IF case */
    not = 0,				/* Flag for IF NOT */
    ifargs;				/* Count of IF condition words */
char ifcond[100];			/* IF condition text */
char *ifcp;				/* Pointer to IF condition text */
#ifdef DCMDBUF
extern int *ifcmd, *count, *iftest, *intime, *inpcas, *takerr, *merror;
#else
extern int ifcmd[];			/* Last command was IF */
extern int iftest[];			/* Last IF was true */
extern int count[];			/* For IF COUNT, one for each cmdlvl */
extern int intime[];
extern int inpcas[];
extern int takerr[];
extern int merror[];
#endif /* DCMDBUF */
#else
extern int takerr[];
#endif /* NOSPL */

#ifdef DCMDBUF
extern char *line;			/* Character buffer for anything */
extern char *tmpbuf;
#else
extern char line[], tmpbuf[];
#endif /* DCMDBUF */
extern char *lp;			/* Pointer to line buffer */

int cwdf = 0;				/* CWD has been done */

#ifndef NOSERVER
extern int en_cwd, en_del, en_dir, en_fin, /* Flags for ENABLE/DISABLE */
   en_get, en_hos, en_sen, en_set, en_spa, en_typ, en_who, en_bye,
   en_asg, en_que;
#endif /* NOSERVER */

extern FILE *tfile[];			/* File pointers for TAKE command */
extern char *tfnam[];			/* Names of TAKE files */

extern int success;			/* Command success/failure flag */

#ifndef NOSPL
extern int maclvl;			/* Macro to execute */
extern char *macx[];			/* Index of current macro */
extern char *mrval[];			/* Macro return value */
extern char *macp[];			/* Pointer to macro */
extern int macargc[];			/* ARGC from macro invocation */

extern char *m_arg[MACLEVEL][NARGS];	/* Stack of macro arguments */
extern char *g_var[];			/* Global variables %a, %b, etc */
 
#ifdef DCMDBUF
extern struct cmdptr *cmdstk;		/* The command stack itself */
#else
extern struct cmdptr cmdstk[];		/* The command stack itself */
#endif /* DCMDBUF */
extern int cmdlvl;			/* Current position in command stack */
#endif /* NOSPL */

#define xsystem(s) zsyscmd(s)

static int x, y, z = 0;
static char *s, *p;

/*  X X S T R C M P  --  Caseless string comparison  */
/*
  Call with pointers to the two strings, s1 and s2, and a length, n.
  Compares up to n characters of the two strings and returns:
    1 if s1 > t1
    0 if s1 = s2
   -1 if s1 < t1
*/
int
xxstrcmp(s1,s2,n) char *s1, *s2; int n; { /* Caseless string comparison. */
    char t1, t2;			

    if (!s1) s1 = "";			/* Watch out for null pointers. */
    if (!s2) s2 = "";
    while (n--) {
	t1 = *s1++;			/* Get next character from each. */
	if (isupper(t1)) t1 = tolower(t1);
	t2 = *s2++;
	if (isupper(t2)) t2 = tolower(t2);
	if (t1 < t2) return(-1);	/* s1 < s2 */
	if (t1 > t2) return(1);		/* s1 > s2 */
    }
    return(0);				/* They're equal */
}

#ifndef NOSPL

/* Do the ASK, ASKQ, GETOK, and READ commands */

#ifndef NOFRILLS
   extern struct keytab yesno[];
   extern int nyesno;
#endif /* NOFRILLS */

int
doask(cx) int cx; {
#ifdef CK_RECALL
    int sv_recall;
    extern int on_recall;
#endif /* CK_RECALL */
    if (cx != XXGOK) {			/* Get variable name */
	if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
	    if (y == -3) {
		printf("?Variable name required\n");
		return(-9);
	    } else return(y);
	}
	strcpy(line,s);			/* Make a copy. */
	lp = line;
	if ((y = parsevar(s,&x,&z)) < 0)  /* Check to make sure it's a */
	  return(y);			  /* variable name. */
    }
    if (cx == XXREA) {			/* READ command */
	if ((y = cmcfm()) < 0)		/* Get confirmation */
	  return(y);
	if (chkfn(ZRFILE) < 1) {	/* File open? */
	    printf("?Read file not open\n");
	    return(0);
	}
	s = line+VNAML+1;		/* Where to read into. */
	y = zsinl(ZRFILE, s, LINBUFSIZ - VNAML - 1); /* Read a line. */
	debug(F111,"read zsinl",s,y);
	if (y < 0) {			/* On EOF or other error, */
	    zclose(ZRFILE);		/* close the file, */
	    delmac(lp);			/* delete the variable, */
	    return(success = 0);	/* and return failure. */
	} else {			/* Read was OK. */
	    success = (addmac(lp,s) < 0 ? 0 : 1); /* Define the variable */
            debug(F111,"read addmac",lp,success);
	    return(success);		/* Return success. */
	}
    }

    /* ASK, ASKQ, or GETOK */

    if ((y = cmtxt("Prompt, enclose in { braces } to preserve\n\
leading and trailing spaces, precede question mark with backslash (\\).",
		   cx == XXGOK ? "{ Yes or no? }" : "",
		   &p,xxstring)) < 0) {
	return(y);
    }
#ifdef VMS
/*
  In VMS, whenever a TAKE file or macro is active, we had to restore the 
  original console modes or else Ctrl-C/Ctrl-Y would not work.  But here we
  go interactive again, so we have to temporarily put them back.
*/
    if (cmdlvl > 0)
      concb((char)escape);
#endif /* VMS */
      
    cmsavp(psave,PROMPTL);		/* Save old prompt */
    cmsetp(brstrip(p));			/* Make new prompt */
    if (cx == XXASKQ) {			/* For ASKQ, */
	concb((char)escape);		/* put console in cbreak mode */
	cmini(0);			/* and no-echo mode. */
    } else {				/* For others, regular echoing. */
	cmini(ckxech);
    }
    x = -1;				/* This means to reparse. */
reprompt:
    if (pflag) prompt(xxstring);	/* Issue prompt. */
    if (cx == XXGOK) {
#ifndef NOFRILLS
	x = cmkey(yesno,nyesno,"","",xxstring);	/* GETOK uses keyword table */
	if (x < 0) {			/* Parse error */
	    if (x == -3) {		/* No answer? */
		printf("Please respond Yes or No\n"); /* Make them answer */
		cmini(ckxech);
	    }
	    goto reprompt;
	}
	if ((y = cmcfm()) < 0)		/* Get confirmation */
	  goto reprompt;
	cmsetp(psave);			/* Restore prompt */
#ifdef VMS
	if (cmdlvl > 0)			/* In VMS and not at top level, */
	  conres();			/*  restore console again. */
#endif /* VMS */
	return(x);			/* Return success or failure */
#else /* NOFRILLS */
	;
#endif /* NOFRILLS */
    } else if (cx == XXGETC) {		/* GETC */
	char tmp[2];
	x = coninc(0);			/* Just read one character */
	if (x > -1) {
	    printf("\r\n");
	    tmp[0] = (char) (x & 0xff);
	    tmp[1] = NUL;
	    y = addmac(lp,tmp);		/* Add it to the macro table. */
	    debug(F111,"getc addmac",lp,y);
	    cmsetp(psave);		/* Restore old prompt. */
	} else y = -1;
	return(success = y < 0 ? 0 : 1);
    } else {				/* ASK or ASKQ */
#ifdef CK_RECALL
	sv_recall = on_recall;
	on_recall = 0;
#endif /* CK_RECALL */
	y = cmdgquo();			/* Get current quoting */
	cmdsquo(0);			/* Turn off quoting */
	while (x == -1) {		/* Prompt till they answer */
	    x = cmtxt("Please respond.","",&s,NULL);
	    debug(F111," cmtxt",s,x);
	}
	cmdsquo(y);			/* Restore previous quoting */
#ifdef CK_RECALL
	on_recall = sv_recall;
#endif /* CK_RECALL */
	if (cx == XXASKQ)		/* ASKQ must echo CRLF here */
	  printf("\r\n");
	if (x < 0) {			/* If cmtxt parse error, */
	    cmsetp(psave);		/* restore original prompt */
#ifdef VMS
	    if (cmdlvl > 0)		/* In VMS and not at top level, */
	      conres();			/*  restore console again. */
#endif /* VMS */
	    return(x);			/* and return cmtxt's error code. */
	}
	if (*s == NUL) {		/* If user typed a bare CR, */
	    cmsetp(psave);		/* Restore old prompt, */
	    delmac(lp);			/* delete variable if it exists, */
#ifdef VMS
	    if (cmdlvl > 0)		/* In VMS and not at top level, */
	      conres();			/*  restore console again. */
#endif /* VMS */
	    return(success = 1);	/* and return. */
	}
	y = addmac(lp,s);		/* Add it to the macro table. */
	debug(F111,"ask addmac",lp,y);
	cmsetp(psave);			/* Restore old prompt. */
#ifdef VMS
	if (cmdlvl > 0)			/* In VMS and not at top level, */
	  conres();			/*  restore console again. */
#endif /* VMS */
	return(success = y < 0 ? 0 : 1);
    }
}
#endif /* NOSPL */

#ifndef NOSPL
int
doincr(cx) int cx; {			/* INCREMENT, DECREMENT */
    char vnambuf[VNAML];		/* Buffer for variable names */

    if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
	if (y == -3) {
	    printf("?Variable name required\n");
	    return(-9);
	} else return(y);
    }
    if (*s != CMDQ) {
        *vnambuf = CMDQ;
	strncpy(vnambuf+1,s,VNAML-1);
    } else strncpy(vnambuf,s,VNAML);

    if ((y = parsevar(vnambuf,&x,&z)) < 0)
      return(y);

    if ((y = cmnum("by amount","1",10,&x,xxstring)) < 0) return(y);
    if ((y = cmcfm()) < 0) return(y);

    z = (cx == XXINC ? 1 : 0);		/* Increment or decrement? */

    if (incvar(vnambuf,x,z,&y) < 0) {
	printf("?Variable %s not defined or not numeric\n",vnambuf);
	return(success = 0);
    }
    return(success = 1);
}
#endif /* NOSPL */


/* Do the (_)DEFINE and (_)ASSIGN commands */

#ifndef NOSPL
int
dodef(cx) int cx; {
    char vnambuf[VNAML];		/* Buffer for variable names */
    char *vnp;				/* Pointer to same */
    if (cx == XXDFX || cx == XXASX) 
      y = cmfld("Macro or variable name","",&s,xxstring); /* eval var name */
    else 
      y = cmfld("Macro or variable name","",&s,NULL);     /* don't evaluate */
    if (y < 0) {
	if (y == -3) {
	    printf("?Variable name required\n");
	    return(-9);
	} else return(y);
    }
    debug(F111,"dodef success",s,success);
    strcpy(vnambuf,s);
    vnp = vnambuf;
    if (vnambuf[0] == CMDQ && (vnambuf[1] == '%' || vnambuf[1] == '&')) vnp++;
    if (*vnp == '%' || *vnp == '&') {
	if ((y = parsevar(vnp,&x,&z)) < 0) return(y);
	debug(F101,"dodef","",x);
	if (y == 1) {			/* Simple variable */
	    if ((y = cmtxt("Definition of variable","",&s,NULL)) < 0)
	      return(y);
	    s = brstrip(s);
	    debug(F110,"xxdef var name",vnp,0);
	    debug(F110,"xxdef var def",s,0);
	} else if (y == 2) {		/* Array element */
	    if ((y = arraynam(s,&x,&z)) < 0) return(y);
	    if (x == 96) {
		printf("?Argument vector array is read-only\n");
		return(-9);
	    }
	    if (chkarray(x,z) < 0) return(-2);
	    if ((y = cmtxt("Definition of array element","",&s,NULL)) < 0)
	      return(y);
	    debug(F110,"xxdef array ref",vnp,0);
	    debug(F110,"xxdef array def",s,0);
	}
    } else {				/* Macro */
	if ((y = cmtxt("Definition of macro","",&s,NULL)) < 0) return(y);
	debug(F110,"xxdef macro name",vnp,0);
	debug(F110,"xxdef macro def",s,0);
	if (*s == '{') {		/* Allow macro def to be bracketed. */
	    s++;			/* If it is, remove the brackets. */
	    y = (int)strlen(s);		/* FOR command depends on this! */
	    if (y > 0 && s[y-1] == '}') s[y-1] = NUL;
	}
    }
    if (*s == NUL) {			/* No arg given, undefine */
	delmac(vnp);			/* silently... */
	return(success = 1);		/* even if it doesn't exist... */
    } 

    /* Defining a new macro or variable */

    if (cx == XXASS || cx == XXASX) {	/* ASSIGN rather than DEFINE? */
	int t;
	t = LINBUFSIZ-1;
	lp = line;			/* If so, expand its value now */
	zzstring(s,&lp,&t);
	s = line;
    }
    debug(F111,"calling addmac",s,(int)strlen(s));

    y = addmac(vnp,s);			/* Add it to the appropriate table. */
    if (y < 0) {
	printf("?%s failed\n",(cx == XXASS || cx == XXASX) ?
	       "ASSIGN" : "DEFINE");
	return(success = 0);
    } else if (cx == XXASX || cx == XXDFX) /* For _ASG or _DEF, */
      return(1);			   /* don't change success variable */
    else
      return(success = 1);
}
#endif /* NOSPL */


#ifndef NODIAL
extern struct keytab partbl[];
/*
   L U D I A L  --  Lookup up dialing directory entry.
  
   Call with string to look up and file descriptor of open dialing directory
   file.  On success, returns pointer to phone number and sets speed
   and parity according to the directory entry.  On failure, returns the NULL
   pointer.
*/
char *
ludial(s,f) char *s; FILE *f; {
    int n, n1, n2, i, t;		/* Workers */
    long xspeed = -1L, x2;		/* Speed to set from directory entry */
    int xparity = -1;			/* Parity to set ...*/
    char *info[5];			/* Array of words from entry */

    if (!s || !f) return(NULL);		/* Validate arguments */
    if ((n1 = strlen(s)) < 1)
      return(NULL);
/*
  We make one or two passes.  The first pass searches for an exact match.
  If it fails, the second pass searches for the first entry of which the
  search string is an abbreviation.  Of course this could be done in one
  pass, but it was safer to add a couple lines for the for-loop than to
  totally rearrange the code at the last minute.  (Edit 186)
*/
    for (i = 0; i < 2; i++) {		/* Do this twice */
	debug(F101,"ludial pass","",i+1);
	rewind(f);			/* Go to beginning of directory file */
	while (1) {
	    if (fgets(line,LINBUFSIZ,f) == NULL) { /* Read a line */
		if (i == 0)		/* EOF */
		  break;		/* If first pass, start second pass */
		if (!backgrd && !quiet)	/* If second pass, give message */
		  printf(" %s not in %s\n",s,dialdir);
		debug(F110,"ludial fails",s,0);
		return(NULL);
	    }
	    n = strlen(line);		/* Strip terminator(s) */
	    while ((n > 0) && (line[n-1] < '!'))
	      line[--n] = NUL;
	    debug(F111,"ludial",line,n);

	    info[0] = NULL; info[1] = NULL; info[2] = NULL;
	    info[3] = NULL; info[4] = NULL;

	    xwords(line,4,info);	/* Get the words. */
	    if (info[1]) {		/* First word. */
		if ((n2 = (int) strlen(info[1])) < 1) /* Its length */
		  continue;		/* If no first word, keep looking. */

		if (n2 < n1)		/* If directory entry name shorter */
		  continue;		/* than search name, then no match. */
		if (i == 0 && n2 != n1)	/* Require exact match on first pass */
		  continue;
		if (xxstrcmp(s,info[1],n1)) /* Caseless string comparison */
		  continue;		/* up to length of search name */
		if (!backgrd && !quiet)
		  printf(" From dialing directory %s: %s=%s\n",
		    dialdir,info[1],info[2] ? info[2] : "(number missing)");
		if (!info[2])
		  return("");
		if (info[3]) {		/* Third word = speed */
		    if (*info[3] != '=') { /* "=" means don't change it */
			xspeed = atol(info[3]);
			x2 = xspeed / 10L;
			if (x2 > 0) {
			    if (ttsspd((int) x2) < 0)
			      printf("\n Can't set speed to %s\n",info[3]);
			    else 
			      speed = ttgspd();
			}
		    }
		}
		if (info[4]) {		/* 4th word = parity */
		    if (*info[4] != '=') { /* "=" means don't change it */
			if ((xparity = lookup(partbl,info[4],5,&t)) > -1)
			  parity = xparity;
		    }
		}
		return(info[2]);	/* Return 2nd word = phone number */
	    }
	}
    }
    return(NULL);			/* Failure */
}

int
dodial(cx) int cx; {			/* DIAL or REDIAL */
    int sparity;			/* For saving global parity value */
    if (cx == XXRED) {			/* REDIAL or... */
	if ((y = cmcfm()) < 0) return(y);
	if (dialnum) {
	    s = dialnum;
	} else {
	    printf("?No DIAL command given yet\n");
	    return(-9);
	}
    } else if (cx == XXDIAL) {		/* DIAL command */
	char *s2;
	if (dialdir && *dialdir)
	  s2 = "Number to dial or entry from dial directory";
	else
	  s2 = "Number to dial";
	if ((x = cmtxt(s2,"",&s,xxstring)) < 0)
	  return(x);
	if (s == NULL || (int)strlen(s) == 0) {
	    printf("?You must specify a number to dial\n");
	    return(-9);
	}
	if (dialfd) {			/* Have dialing directory? */
	    s2 = ludial(s,dialfd);	/* Look up in dialing directory */
	    if (s2) {
		if ((int)strlen(s2) < 1) {
		    printf(
		     "?Dialing directory \"%s\" entry lacks phone number\n",s);
		    return(-9);
		} else
		  s = s2;		/* Make substitution if found */
	    }
	}
	if (dialnum) free(dialnum);	/* Make copy for REDIAL */
	dialnum = malloc((int)strlen(s) + 1);
	if (dialnum) strcpy(dialnum,s);
    } else return(-2);
#ifdef VMS
    conres();				/* So Ctrl-C/Y will work */
#endif /* VMS */
/*
  A rather radical change for edit 190...

  Some modems do not react well to parity.  Also, if we are dialing through a
  TCP/IP TELNET modem server, parity can be fatally misinterpreted as TELNET
  negotiations.  Note that at this point it is very likely that we picked up a
  parity setting from the dialing directory (in the call to ludial() above).
  So now we save the current parity, set it to NONE during the dialing
  process, and then restore it afterwards.  This should work even if the user
  interrupts the DIAL command, because the DIAL module has its own interrupt
  handler.  BUT... if, for some reason, a dialing device actually *requires*
  parity (e.g. CCITT V.25bis says that even parity should be used), this
  might prevent successful dialing.
*/
    sparity = parity;			/* Save current parity */
    parity = 0;				/* Set parity to NONE */
    success = ckdial(s);		/* Try to dial */
    parity = sparity;			/* Restore parity */
#ifdef OS2
    ttres();
#endif /* OS2 */
#ifdef VMS
    concb((char)escape);		/* Back to command parsing mode */
#endif /* VMS */
    return(success);
}
#endif /* NODIAL */

int					/* Do the DIRECTORY command */
dodir() {
#ifndef MAC
    char *dc;
#endif /* MAC */
#ifdef MAC
/*
  This is a crude, do-it-yourself directory command.  It shows all the
  files in the current directory: size and name of each file.  Only regular
  files are shown.  With a little more work, it could also show directories,
  and mark files as regular or directories, and it could also show dates.
  See sample code in zldir() routine in ckmfio.c.
*/
    char mac_name[65];
    long mac_len, nfiles, nbytes;
    extern long mac_znextlen;		/* See ckmfio.c for this. */

    if ((y = cmcfm()) < 0)
      return(y);

    nfiles = nbytes = 0L;
    printf("\nDirectory of %s\n\n",zgtdir());
    x = zxpand(":");
    while (x-- > 0) {
	if (!znext(mac_name))
	  break;
	mac_len = zchki(mac_name);
	if (mac_len > -1L) {
	    nfiles++;
	    nbytes += mac_znextlen;
	    printf("%10ld %s\n", mac_znextlen, mac_name);
	}
    }
    printf("\n%ld file%s, %ld byte%s\n\n",
	   nfiles,
	   (nfiles == 1) ? "" : "s",
	   nbytes,
	   (nbytes == 1) ? "" : "s"
	   );
    return(success = 1);
#else
#ifdef VMS
    if ((x = cmtxt("Directory/file specification","",&s,xxstring)) < 0)
      return(x);
    /* now do this the same as a shell command - helps with LAT  */
    conres();				/* Make console normal */
    lp = line;
    if (!(dc = getenv("CK_DIR"))) dc = DIRCMD;
    sprintf(lp,"%s %s",dc,s);
    debug(F110,"DIR string", line, 0);
    x = zshcmd(lp);
    debug(F101,"DIR return code", "", x);
    concb((char)escape);
    return(success = (x > 0) ? 1 : 0);
#else
#ifdef AMIGA
    if ((x = cmtxt("Directory/file specification","",&s,xxstring)) < 0)
      return(x);
#else
#ifdef datageneral
    if ((x = cmtxt("Directory/file specification","+",&s,xxstring)) < 0)
      return(x);
#else
#ifdef OS2
    tmpbuf[0] = NUL;
    if ((x = cmifi2(
"Device, directory, and/or file specification,\n\
 or switch(es), or '> file'","",
		    &s,&y,1,xxstring)) < 0) {
	debug(F101,"DIR cmifi2","",x);
	if (x == -3) {			/* Done. */
	    goto sw_skip;
	} else if (x == -2 && (*s == '/' || *s == '>')) {
	    strncpy(tmpbuf,s,TMPBUFSIZ); /* Switch or redirect */
	    if ((y = cmcfm()) < 0)
	      return(y);
	    else
	      goto sw_skip;
	} else if (x == -2 && !strchr(s,'.')) {	/* Maybe ".*" is missing */
	    goto fs_copy;
	} else if (x == -2) {
	    printf("%s not found\n",s);
	    return(-9);
	} else
	  return(x);
    }
#else /* General Case */
    if ((x = cmdir("Directory/file specification","",&s,xxstring)) < 0)
      if (x != -3) return(x);
#endif /* OS2 */
#endif /* datageneral */
#endif /* AMIGA */

#ifdef OS2
fs_copy:
#endif /* OS2 */
    debug(F110,"DIR fs_copy",s,0);
    strncpy(tmpbuf,s,TMPBUFSIZ);	/* Copy the filespec */

#ifdef OS2
    {   /* Lower level functions change / to \, not good for CMD.EXE. */
	/* Only do this to filenames, not switches! */
	char *p = tmpbuf;
	while (*p) {			/* Change them back to \ */
	    if (*p == '/') *p = '\\';
	    p++;
	}
    }
    debug(F110,"DIR tmpbuf 1",tmpbuf,0);
    /* Now parse trailing switches like /P/O-D... */
    if ((x = cmtxt("Optional switches and/or redirect for OS/2 DIR command",
		   "",&s,xxstring)) < 0)
      return(x);
    strcat(tmpbuf,s);			/* Append them to the filespec */
    debug(F110,"DIR tmpbuf 2",tmpbuf,0);
sw_skip:
#else
    if ((y = cmcfm()) < 0) return(y);
#endif /* OS2 */
    s = tmpbuf;
    lp = line;
    if (!(dc = getenv("CK_DIR"))) dc = DIRCMD;
    sprintf(lp,"%s %s",dc,s);
    debug(F110,"DIR",line,0);
    xsystem(line);
    return(success = 1);		/* who cares... */
#endif /* VMS */
#endif /* MAC */
}

#ifndef NOSERVER
#ifndef NOFRILLS
/* Do the ENABLE and DISABLE commands */

int
doenable(cx,x) int cx, x; {
    y = ((cx == XXENA) ? 1 : 0);
    switch (x) {
      case EN_ALL:
	en_cwd = en_del = en_dir = en_fin = en_get = y;
	en_sen = en_set = en_spa = en_typ = en_who = y;
#ifndef datageneral
        en_bye = y;
#endif /* datageneral */
#ifndef NOPUSH
	en_hos = y;
#endif /* NOPUSH */
#ifndef NOSPL
	en_asg = en_que = y;
#endif /* NOSPL */
	break;
      case EN_BYE:
#ifndef datageneral
/*
  In Data General AOS/VS Kermit can't log out its superior process.
*/
        en_bye = y;
#endif /* datageneral */
	break;
      case EN_CWD:
	en_cwd = y;
	break;
      case EN_DEL:
	en_del = y;
	break;
      case EN_DIR:
	en_dir = y;
	break;
      case EN_FIN:
	en_fin = y;
	break;
      case EN_GET:
	en_get = y;
	break;
#ifndef NOPUSH
      case EN_HOS:
	en_hos = y;
	break;
#endif /* NOPUSH */
      case EN_SEN:
	en_sen = y;
	break;
      case EN_SET:
	en_set = y;
	break;
      case EN_SPA:
	en_spa = y;
	break;
      case EN_TYP:
	en_typ = y;
	break;
      case EN_WHO:
	en_who = y;
	break;
#ifndef NOSPL
      case EN_ASG:
	en_asg = y;
	break;
      case EN_QUE:
	en_que = y;
	break;
#endif /* NOSPL */
      default:
	return(-2);
    }
    return(1);
}
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOFRILLS
int
dodel() {				/* DELETE */
#ifndef MAC
    long zl;
#endif /* MAC */
    if ((x = cmifi("File(s) to delete","",&s,&y,xxstring)) < 0) {
	if (x == -3) {
	    printf("?A file specification is required\n");
	    return(-9);
	} else return(x);
    }
#ifdef MAC
    strcpy(line,s);
#else
    strncpy(tmpbuf,s,TMPBUFSIZ);	/* Make a safe copy of the name. */
#ifdef OS2
    {   /* Lower level functions change / to \, not good for CMD.EXE. */
	char *p = tmpbuf;
	while (*p) {			/* Change them back to \ */
	    if (*p == '/') *p = '\\';
	    p++;
	}
    }
#endif /* OS2 */
    debug(F110,"xxdel tmpbuf",tmpbuf,0);
    sprintf(line,"%s %s",DELCMD,tmpbuf); /* Construct the system command. */
#endif /* MAC */
    debug(F110,"xxdel line",line,0);
    if ((y = cmcfm()) < 0) return(y);	/* Confirm the user's command. */
#ifdef VMS
    conres();
#endif /* VMS */
#ifdef MAC
    s = line;
    success = (zdelet(line) == 0);
#else
    s = tmpbuf;
    xsystem(line);			/* Let the system do it. */
    zl = zchki(tmpbuf);
    success = (zl == -1L);
#endif /* MAC */
    if (msgflg)
      printf("%s - %sdeleted\n",s, success ? "" : "not ");
#ifdef VMS
    concb((char)escape);
#endif /* VMS */
    return(success);
}
#endif /* NOFRILLS */

#ifndef NOSPL				/* The ELSE command */
int
doelse() {
    if (!ifcmd[cmdlvl]) {
	printf("?ELSE doesn't follow IF\n");
	return(-2);
    }
#ifdef COMMENT
/*
  Wrong.  This prevents IF..ELSE IF...ELSE IF...ELSE IF...ELSE...
  from working.
*/
    ifcmd[cmdlvl] = 0;
#endif /* COMMENT */
    if (!iftest[cmdlvl]) {		/* If IF was false do ELSE part */
	if (maclvl > -1) {		/* In macro, */
	    pushcmd();			/* save rest of command. */
	} else if (tlevel > -1) {	/* In take file, */
	    pushcmd();			/* save rest of command. */
	} else {			/* If interactive, */
	    cmini(ckxech);		/* just start a new command */
	    printf("\n");		/* (like in MS-DOS Kermit) */
	    if (pflag) prompt(xxstring);
	}
    } else {				/* Condition is false */
	if ((y = cmtxt("command to be ignored","",&s,NULL)) < 0)
	  return(y);			/* Gobble up rest of line */
    }
    return(0);
}
#endif /* NOSPL */

#ifndef NOSPL
int
dofor() {				/* The FOR command. */
    int fx, fy, fz;			/* loop variables */
    char *ap;				/* macro argument pointer */

    if ((y = cmfld("Variable name","",&s,NULL)) < 0) { /* Get variable name */
	if (y == -3) {
	    printf("?Variable name required\n");
	    return(-9);
	} else return(y);
    }
    if ((y = parsevar(s,&x,&z)) < 0)	/* Check it. */
      return(y);

    lp = line;				/* Build a copy of the command */
    strcpy(lp,"_forx ");
    lp += (int)strlen(line);		/* "_for" macro. */
    ap = lp;				/* Save pointer to macro args. */

    if (*s == CMDQ) s++;		/* Skip past backslash if any. */
    while (*lp++ = *s++) ;		/* copy it */
    lp--; *lp++ = SP;			/* add a space */

    if ((y = cmnum("initial value","",10,&fx,xxstring)) < 0) {
	if (y == -3) return(-2);
	else return(y);
    }
    s = atmbuf;				/* Copy the atom buffer */
    if ((int)strlen(s) < 1) goto badfor;
    while (*lp++ = *s++) ;		/* (what they actually typed) */
    lp--; *lp++ = SP;

    if ((y = cmnum("final value","",10,&fy,xxstring)) < 0) {
	if (y == -3) return(-2);
	else return(y);
    }
    s = atmbuf;				/* Same deal */
    if ((int)strlen(s) < 1) goto badfor;
    while (*lp++ = *s++) ;
    lp--; *lp++ = SP;

    if ((y = cmnum("increment","1",10,&fz,xxstring)) < 0) {
	if (y == -3) return(-2);
	else return(y);
    }
    sprintf(tmpbuf,"%d ",fz);
    s = atmbuf;				/* same deal */
    if ((int)strlen(s) < 1) goto badfor;
    while (*lp++ = *s++) ;
    lp--; *lp++ = SP;

    /* Insert the appropriate comparison operator */
    if (fz < 0)
      *lp++ = '<';
    else
      *lp++ = '>';
    *lp++ = SP;

    if ((y = cmtxt("Command to execute","",&s,NULL)) < 0) return(y);
    if ((int)strlen(s) < 1) return(-2);
    
    if (litcmd(&s,&lp) < 0) {
	printf("?Unbalanced brackets\n");
	return(0);
    }
    if (fz == 0) {
	printf("?Zero increment not allowed\n");
	return(0);
    }
    x = mlook(mactab,"_forx",nmac);	/* Look up FOR macro definition */
    if (x < 0) {			/* Not there? */
	addmmac("_forx",for_def);	/* Put it back. */
	if ((x = mlook(mactab,"_forx",nmac)) < 0) { /* Look it up again. */
	    printf("?FOR macro definition gone!\n"); /* Shouldn't happen. */
	    return(success = 0);
	}
    }
    debug(F110,"FOR command",line,0);
    return(success = dodo(x,ap));	/* Execute the FOR macro. */

badfor: printf("?Incomplete FOR command\n");
    return(-2);
}
#endif /* NOSPL */

#ifndef NOFRILLS
/* Do the BUG command */

int
dobug() {
    printf("\n%s,%s\n Numeric: %ld",versio,ckxsys,vernum);
    if (verwho) printf("-%d",verwho);
    printf("\n\n\
Before deciding you have found a bug, please consult the documentation:\n");
    printf(" . \"Using C-Kermit\"\n");
#ifndef OS2
   printf(" . The CKCKER.BWR file\n");
#endif /* OS2 */
#ifdef UNIX
    printf(" . The CKUKER.BWR file\n");
#else
#ifdef VMS
    printf(" . The CKVKER.BWR file\n");
#else
#ifdef OS2
    printf(" . The CKERMIT.INF file\n");
#else
#ifdef datageneral
    printf(" . The CKDKER.BWR file\n");
#else
#ifdef STRATUS
    printf(" . The CKLKER.BWR file\n");
#else
#ifdef AMIGA
    printf(" . The CKIKER.BWR file\n");
#else
#ifdef GEMDOS
    printf(" . The CKSKER.BWR file\n");
#else
#ifdef MAC
    printf(" . The CKMKER.BWR file\n");
#else
#ifdef OSK
    printf(" . The CK9KER.BWR file\n");
#else
    printf(" . The appropriate system-dependent CK?KER.BWR file\n");
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

    printf("\nTo report C-Kermit bugs, send e-mail to:\n");
    printf(" kermit@columbia.edu (Internet)\n");
    printf(" KERMIT@CUVMA (EARN/CREN/BITNET)\n");
    printf(" ...!uunet!columbia.edu!kermit (Usenet)\n");
    printf("Or write to:\n Kermit Development\n Columbia University\n");
    printf(" 612 W 115 Street\n");
    printf(" New York, NY  10025  USA\nOr call:\n +1 212 854-5126 (USA)\n\n");
#ifndef NOSHOW
#ifndef NOFRILLS
    printf(
"Before reporting problems, please use the SHOW VERSION and SHOW FEATURES\n");
    printf(
"commands to get detailed program version and configuration information.\n\n");
#endif /* NOFRILLS */
#endif /* NOSHOW */
    return(1);
}
#endif /* NOFRILLS */

#ifndef NOSPL
int
dopaus(cx) int cx; {
    /* Both should take not only secs but also hh:mm:ss as argument. */
    if (cx == XXWAI)
      y = cmnum("seconds to wait","1",10,&x,xxstring);
    else if (cx == XXPAU)
      y = cmnum("seconds to pause","1",10,&x,xxstring);
    else
      y = cmnum("milliseconds to sleep","100",10,&x,xxstring);
    if (y < 0) return(y);
    if (x < 0) x = 0;
    switch (cx) {
      case XXPAU:			/* PAUSE */
      case XXMSL:			/* MSLEEP */
	if ((y = cmcfm()) < 0) return(y);
	break;
      case XXWAI:			/* WAIT */
	z = 0;				/* Modem signal mask */
	while (1) {			/* Read zero or more signal names */
	    y = cmkey(mstab,nms,"modem signal","",xxstring);
	    if (y == -3) break;		/* -3 means they typed CR */
	    if (y < 0) return(y);	/* Other negatives are errors */
	    z |= y;			/* OR the bit into the signal mask */
	}
	break;

      default:				/* Shouldn't happen */
	return(-2);
    }

/* Command is entered, now do it. */

    if (cx == XXMSL) {			/* Millisecond sleep */
	msleep(x);
	return(success = 1);
    }
    while (x--) {			/* Sleep loop */
	int mdmsig;
	if (y = conchk()) {		/* Did they type something? */
#ifdef COMMENT
	    while (y--) coninc(0);	/* Yes, gobble it up */
#endif
	    break;			/* And quit PAUSing or WAITing */
	}
	if (cx == XXWAI && z != 0) {
	    mdmsig = ttgmdm();
	    if (mdmsig < 0) return(success = 0);
	    if ((mdmsig & z) == z) return(success = 1);
	}
	sleep(1);			/* No interrupt, sleep one second */
    }
    if (cx == XXWAI) success = 0;
    else success = (x == -1);		/* Set SUCCESS/FAILURE for PAUSE. */
    return(0);
}
#endif /* NOSPL */


#ifndef NOFRILLS
int
dorenam() {
    if ((x = cmifi("File to rename","",&s,&y,xxstring)) < 0) {
	if (x == -3) {
	    printf("?Name of existing file required\n");
	    return(-9);
	} else return(x);
    }
    if (y) {				/* No wildcards allowed */
	printf("\n?Please specify a single file\n");
	return(-9);
    }
    strcpy(line,s);			/* Make a safe copy of the old name */
    p = line + (int)strlen(line) + 2;	/* Place for new name */
    if ((x = cmofi("New name","",&s,xxstring)) < 0) { /* Get new name */
	if (x == -3) {
	    printf("?New name for file required\n");
	    return(-9);
	} else return(x);
    }
    strcpy(p,s);			/* Make a safe copy of the new name */
    if ((y = cmcfm()) < 0) return(y);
#ifdef VMS
    conres();				/* Let Ctrl-C work. */
#endif /* VMS */
    debug(F110,"dorename line",line,0);
    debug(F110,"dorename p",p,0);
    if (zrename(line,p) < 0) {
	printf("?Can't rename %s to %s\n",line,p);
#ifdef VMS
	concb((char)escape);
#endif /* VMS */
	return(-9);
    } else {
#ifdef VMS
	concb((char)escape);
#endif /* VMS */
	return(success = 1);
    }
}
#endif /* NOFRILLS */


#ifndef NOSPL

/* Do the RETURN command */

int
doreturn(s) char *s; {
    int x; char *p;
    if (maclvl < 0) {
	printf("\n?Can't return from level %d\n",maclvl);
	return(success = 0);
    }
    lp = line;				/* Expand return value now */
    x = LINBUFSIZ-1;
    if (zzstring(s,&lp,&x) > -1) {
	s = line;
	debug(F110,"RETURN parse",s,0);
    }
    debug(F101,"RETURN maclvl 1","",maclvl);
    /* Pop from all FOR/WHILE/XIFs */
    while ((maclvl > 0) &&
	   (m_arg[maclvl-1][0]) &&
	   (cmdstk[cmdlvl].src == CMD_MD) &&
	   (!strncmp(m_arg[maclvl-1][0],"_xif",4) ||
	    !strncmp(m_arg[maclvl-1][0],"_for",4) ||
	    !strncmp(m_arg[maclvl-1][0],"_whi",4))) {
	debug(F110,"RETURN popping",m_arg[maclvl-1][0],0);
	dogta(XXPTA);		/* Put args back */
	popclvl();		/* Pop up two levels */
	popclvl();
	debug(F101,"RETURN maclvl 2","",maclvl);
    }
    popclvl();				/* Pop from enclosing TAKE or macro */
    debug(F101,"RETURN maclvl 3","",maclvl);

    x = (int)strlen(s);			/* Length of return value */
    if (x > 0) {			/* Have return value? */
	p = malloc(x+2);		/* Allocate a place to keep it */
	if (mrval[maclvl+1])		/* Free old one, if any */
	  free(mrval[maclvl+1]);
	if (p) {			/* Did we get a place? */
	    strcpy(p, s);		/* Yes, copy the string into it. */
	    mrval[maclvl+1] = p;	/* Make return value point to it. */
	    debug(F110,"RETURN copy",mrval[maclvl],0);
	} else {			/* No, could not get space. */
	    mrval[maclvl+1] = NULL;	/* Return null pointer. */
	    x = 0;			/* Set failure return code. */
	}
    } else mrval[maclvl+1] = NULL;	/* Blank return code */
    return(success = x ? 1 : 0);	/* Return status code */	
}
#endif /* NOSPL */

#ifndef NOSPL
/* Do the OPEN command */

int
doopen()  {				/* OPEN { append, read, write } */
    int x, y, z; char *s;
    static struct filinfo fcb;		/* (must be static) */
    if ((x = cmkey(opntab,nopn,"mode","",xxstring)) < 0) {
	if (x == -3) {
	    printf("?Mode required\n");
	    return(-9);
	} else return(x);
    }
    switch (x) {
      case XYFZ_O:			/* Old file (READ) */
	if (chkfn(ZRFILE) > 0) {
	    printf("?Read file already open\n");
	    return(-2);
	}
	if ((z = cmifi("File to read","",&s,&y,xxstring)) < 0) {
	    if (z == -3) {
		printf("?Input filename required\n");
		return(-9);
	    } else return(z);
	}
	if (y) {				/* No wildcards allowed */
	    printf("\n?Please specify a single file\n");
	    return(-2);
	}
	strcpy(line,s);
	if ((int)strlen(line) < 1) return(-2);
	if ((y = cmcfm()) < 0) return(y);
	return(success = zopeni(ZRFILE,line));

#ifndef MAC
#ifndef NOPUSH
      case XYFZ_Y:			/* Pipe/Process (READ) */
	if (chkfn(ZRFILE) > 0) {
	    printf("?Read file already open\n");
	    return(-2);
	}
        if ((y = cmtxt("System command to read from","",&s,xxstring)) < 0) {
	    if (y == -3) {
		printf("?Command name required\n");
		return(-9);
	    } else return(y);
	}
	strcpy(line,s);
	if ((int)strlen(line) < 1) return(-2);
	if ((y = cmcfm()) < 0) return(y);
	return(success = zxcmd(ZRFILE,line));

      case XYFZ_X:			/* Write to pipe */
	if (chkfn(ZWFILE) > 0) {
	    printf("?Write file already open\n");
	    return(-2);
	}
        if ((y = cmtxt("System command to write to","",&s,xxstring)) < 0) {
	    if (y == -3) {
		printf("?Command name required\n");
		return(-9);
	    } else return(y);
	}
	strcpy(line,s);
	if ((int)strlen(line) < 1) return(-2);
	if ((y = cmcfm()) < 0) return(y);
	success = zxcmd(ZWFILE,line);
	if (!success && msgflg)
	  printf("Can't open process for writing: %s\n",line);
	return(success);
#endif /* NOPUSH */
#endif /* MAC */

      case XYFZ_N:			/* New file (WRITE) */
      case XYFZ_A:			/* (APPEND) */
	if ((z = cmofi("Name of local file to create","",&s,xxstring)) < 0) {
	    if (z == -3) {
		printf("?Filename required\n");
		return(-9);
	    } else return(z);
	}
	if (z == 2) {
	    printf("?Sorry, %s is a directory name\n",s);
	    return(-9);
	}
	if (chkfn(ZWFILE) > 0) {
	    printf("?Write/Append file already open\n");
	    return(-2);
	}
        fcb.bs = fcb.cs = fcb.rl = fcb.fmt = fcb.org = fcb.cc = fcb.typ = 0;
	fcb.lblopts = 0;
	fcb.dsp = x;			/* Create or Append */
	strcpy(line,s);
	if ((int)strlen(line) < 1) return(-2);
	if ((y = cmcfm()) < 0) return(y);
	return(success = zopeno(ZWFILE,line,NULL,&fcb));

      default:
	printf("?Not implemented");
	return(-2);
    }
}
#endif /* NOSPL */

/* Finish parsing and do the GET command */

int
doget() {
    int x, y;
    char *cbp;

    cmarg2 = "";			/* Initialize as-name to nothing */
    x = 0;
#ifdef NOFRILLS
    if (*cmarg == NUL) {
	printf("?Remote filespec required\n");
	return(-3);
    }
#else
/*
  If remote file name omitted, get foreign and local names separately.
  But multine GET is allowed only if NOFRILLS is not defined.
*/
    if (*cmarg == NUL) {
 
	if (tlevel > -1
#ifndef NOSPL
	    && cmdstk[cmdlvl].src == CMD_TF
#endif /* NOSPL */
	    ) {

/* Input is from a command file. */

	    /* Read 2nd line of GET command */

	    if (getnct(cmdbuf,CMDBL) < 0) {
		printf("Command file ends prematurely in multiline GET\n");
		popclvl();
		return(-9);
	    }
	    cmres();			/* Parse it */
	    if ((x = cmtxt("Oofa","",&s,xxstring)) < 0)
	      return(x);
	    strcpy(line,brstrip(s));	/* Make a safe copy */
	    cmarg = line;		/* Point to remote filename */
	    if (*cmarg == NUL) {	/* Make sure there is one */
		printf("Remote filename missing in multiline GET\n");
		return(-9);
	    }
	    lp = line + strlen(line) + 1; /* Place for as-name */

	    /* And third line... */

	    cmarg2 = "";		/* Assume no as-name */
	    if (getnct(cmdbuf,CMDBL) < 0) { /* Get next line */
		popclvl();		/* There isn't one. */
	    } else {			/* There is... */
		if (*cmdbuf >= ' ') {	/* Parse as output filename */
		    cmres();
		    if ((x = cmofi("Mupeen",cmarg,&s,xxstring)) < 0)
		      return(x);
		    strcpy(lp,s);	/* Make a safe copy */
		    cmarg2 = lp;	/* Point as-name pointer at it */
		}
	    }
            x = 0;			/* Return code OK */

#ifndef NOSPL
/* Reading commands from a macro definition */

	} else if (cmdlvl > 0 && cmdstk[cmdlvl].src == CMD_MD) {

	    /* Read second line of GET command */

	    cbp = cmdbuf;
	    if (getncm(cbp,CMDBL) < 0) {
		printf("Macro definition ends prematurely in multiline GET\n");
		return(-9);
	    }
	    cmres();
	    if ((x = cmtxt("Oofa","",&s,xxstring)) < 0) return(x);
	    if (*s == NUL) {		/* Make sure we got something */
		printf("Remote filename missing in multiline GET\n");
		return(-9);
	    }
	    strcpy(line,brstrip(s));	/* Copy filename to safe place */
	    cmarg = line;		/* Point to it */
	    x = strlen(line);		/* Get its length */
	    lp = line + x + 1;		/* Where to put the next bit */
	    y = LINBUFSIZ - x - 1;	/* Room left for next bit */

	    /* And third line... */

	    cmarg2 = "";		/* Assume no as-name */
	    if (getncm(lp,y) > -1 && *lp >= ' ') { /* Read next line */
		x = strlen(lp);
		if (lp[x-1] == CR) lp[x-1] = NUL; /* Remove CR */
		cbp = cmdbuf;		/* Interpret the line */
		*cbp = NUL;		/* ... */
		y = CMDBL;		/* into the command buffer */
		zzstring(lp,&cbp,&y);
		if (*cmdbuf) {		/* If we have something */
		    cmres();		/* parse it as an output filename */
		    strcat(cmdbuf," ");
		    if ((x = cmofi("Mupeen","",&s,NULL)) < 0)
		      return(x);
		    strcpy(lp,s);	/* Copy the name to safe place */
		    cmarg2 = lp;	/* and make as-name pointer */
		}
	    }
            x = 0;			/* Return code OK */
#endif /* NOSPL */
        } else {			/* Input is from terminal */
 
	    cmsavp(psave,PROMPTL);
	    cmsetp(" Remote file specification: "); /* Make new one */
	    cmini(ckxech);
	    x = -1;
	    if (pflag) prompt(xxstring);
	    while (x == -1) {		/* Prompt till they answer */
	    	x = cmtxt("Name of remote file(s)","",&cmarg,xxstring);
		debug(F111," cmtxt",cmarg,x);
	    }
	    if (x < 0) {
		cmsetp(psave);
		return(x);
	    }
	    if (*cmarg == NUL) { 	/* If user types a bare CR, */
		printf("(cancelled)\n"); /* Forget about this. */
	    	cmsetp(psave);		/* Restore old prompt, */
		return(0);		/* and return. */
	    }
	    strcpy(line,brstrip(cmarg)); /* Make a safe copy */
	    cmarg = line;
	    cmsetp(" Local name to store it under: "); /* New prompt */
	    cmini(ckxech);
	    x = -1;
	    if (pflag) prompt(xxstring);
	    while (x == -1) {		/* Again, parse till answered */
	    	x = cmofi("Local file name","",&cmarg2,xxstring);
	    }
	    if (x < 0) {		/* Parse error */
		if (x == -3) {		/* CR = cancel */
		    printf("(cancelled)\n"); /* Print message */
		    x = 0;		/* Avoid further messages */
		}
		cmsetp(psave);		/* Restore prompt */
		return(x);
	    }	    
	    x = -1;			/* Get confirmation. */
	    while (x == -1) x = cmcfm();
	    cmsetp(psave);		/* Restore old prompt. */
        }
    }
#endif /* NOFRILLS */

    if (x == 0) {			/* Good return from cmtxt or cmcfm, */
	debug(F110,"xxget cmarg",cmarg,0);
	strncpy(fspec,cmarg,FSPECL);
	debug(F111,"xxget fspec",fspec,FSPECL);
	sstate = 'r';			/* Set start state. */
	if (local) {
	    displa = 1;
	    ttflui();
	}
    }
#ifndef NOFRILLS
#ifdef CK_TMPDIR
/* cmarg2 is also allowed to be a device or directory name */

	y = strlen(cmarg2);
	if (
#ifdef OS2
	    (isalpha(cmarg2[0]) &&
	     cmarg2[1] == ':' &&
	     cmarg2[2] == '\0') ||
	    isdir(cmarg2)
#else
#ifdef UNIX
	    (y > 0 && cmarg2[y-1] == '/') || isdir(cmarg2)
#else
#ifdef VMS
	    (y > 0) && isdir(cmarg2)
#else
#ifdef STRATUS
	    (y > 0) && isdir(cmarg2)
#endif /* STRATUS */
#endif /* VMS */
#endif /* UNIX */

#endif /* OS2 */
	    ) {
	    debug(F110,"RECEIVE arg disk or dir",cmarg2,0);
	    if (s = zgtdir()) {		/* Get current directory, */
		if (zchdir(cmarg2)) {	/* change to given disk/directory, */
		    strncpy(savdir,s,TMPDIRLEN); /* remember old disk/dir */
		    debug(F110,"tmpdir saving",savdir,0);
		    debug(F110,"tmpdir changing",cmarg2,0);
		    f_tmpdir = 1;	/* and that we did this */
		    cmarg2 = "";	/* and we don't have an as-name. */
		} else {
		    printf("?Can't access %s\n",cmarg2);
		    f_tmpdir = 0;
		    cmarg2 = "";
		    return(-9);
		}
	    } else {
		printf("?Can't get current directory\n");
		cmarg2 = "";
		f_tmpdir = 0;
		return(-9);
	    }
	}
#endif /* CK_TMPDIR */
#endif /* NOFRILLS */

    return(x);
}

#ifndef NOSPL

/*
  _ G E T A R G S

  Used by XIF, FOR, and WHILE, each of which are implemented as 2-level
  macros; the first level defines the macro, the second runs it.
  This routine hides the fact that they are macros by importing the
  macro arguments (if any) from two levels up, to make them available
  in the XIF, FOR, and WHILE commands themselves; for example as loop
  indices, etc.
*/
int
dogta(cx) int cx; {
    int i; char c; char mbuf[4]; char *p;

    if ((y = cmcfm()) < 0)
      return(y);
    if (cx == XXGTA)
      debug(F101,"_getargs maclvl","",maclvl);
    else if (cx == XXPTA)
      debug(F101,"_putargs maclvl","",maclvl);
    else
      return(-2);
    if (maclvl < 1)
      return(success = 0);

    debug(F101,"success","",success);

    mbuf[0] = '%'; mbuf[1] = '0'; mbuf[2] = '\0'; /* Argument name buf */
    for (i = 0; i < 10; i++) {		/* For all args */
	c = (char) i + '0';		/* Make name */
	mbuf[1] = c;			/* Insert digit */
	if (cx == XXGTA) {		/* Get arg from level-minus-2 */
	    if (maclvl == 1) p = g_var[c]; /* If at level 1 use globals 0..9 */
	    else p = m_arg[maclvl-2][i]; /* Otherwise they're on the stack */
	    if (!p) {
		debug(F111,"_getarg p","(null pointer)",i);
	    } else debug(F111,"_getarg p",p,i);
	    addmac(mbuf,p);
	} else if (cx == XXPTA) {	/* Put args level+2 */
#ifndef MAC
	    connoi();			/* Turn off interrupts. */
#endif /* MAC */
	    maclvl -= 2;		/* This is gross.. */
	    p = m_arg[maclvl+2][i];
	    if (p)
	      debug(F111,"_putarg m_arg[maclvl+2][i]",p,i);
	    else
	      debug(F111,"_putarg m_arg[maclvl+2][i]","(null pointer)",i);
	    addmac(mbuf,m_arg[maclvl+2][i]);
	    maclvl += 2;
#ifndef MAC
	    conint(trap,stptrap);	/* Restore interrupts */
#endif /* MAC */
	} else return(success = 0);
    }
    debug(F101,"_get/putarg exit","",i);
    debug(F101,"_get/putarg exit maclvl","",maclvl);
#ifdef COMMENT
/*
  Internal commands don't change success variable if they succeed.
*/
    return(success = 1);
#else
    return(1);
#endif /* COMMENT */

}
#endif /* NOSPL */

#ifndef NOSPL
/*
  Do the GOTO and FORWARD commands.
  s = Label to search for, cx = function code, XXGOTO or XXFWD.
*/
int
dogoto(s, cx) char *s; int cx; {
    int i, j, x, y;
    char tmplbl[50], *lp;

    debug(F101,"goto cx","",cx);
    debug(F101,"goto cmdlvl","",cmdlvl);
    debug(F101,"goto maclvl","",maclvl);
    debug(F101,"goto tlevel","",tlevel);
    debug(F110,"goto before conversion",s,0);
    y = (int)strlen(s);
    if (*s != ':') {			/* If the label mentioned */
	for (i = y; i > 0; i--) {	/* does not begin with a colon, */
	    s[i] = s[i-1];		/* then insert one. */
	}				/* Also, convert to lowercase. */
	s[0] = ':';
	s[++y] = '\0';
    }
    debug(F111,"goto after conversion",s,y);
    if (s[1] == '.' || s[1] == SP || s[1] == NUL) {
	printf("?Bad label syntax - '%s'\n",s);
	return(success = 0);
    }
    if (cmdlvl == 0) {
	printf("?Sorry, GOTO only works in a command file or macro\n");
	return(success = 0);
    }
    while (cmdlvl > 0) {		/* Only works inside macros & files */
	if (cmdstk[cmdlvl].src == CMD_MD) { /* GOTO inside macro */
	    int i, m, flag;
	    char *xp, *tp;

	    /* GOTO: rewind the macro; FORWARD: start at current position */

	    lp = (cx == XXGOTO) ? macx[maclvl] : macp[maclvl];
	    m = (int)strlen(lp) - y + 1;
	    debug(F111,"goto in macro",lp,m);

	    flag = 1;			/* flag for valid label position */
	    for (i = 0; i < m; i++,lp++) { /* search for label in macro body */
		if (*lp == ',') {	/* Really should also watch out */
		    flag = 1;		/* for braces here...  Commas in */
		    continue;		/* in braces are not really commas */
		}
		if (flag) {		/* If in valid label position */
		    if (*lp == SP)	/* eat leading spaces */
		      continue;
		    if (*lp != ':') {	/* Look for label introducer */
			flag = 0;	/* this isn't it */
			continue;	/* keep looking */
		    }
		}
		if (!flag)		/* We don't have a label */
		  continue;		/*  so keep looking... */
		xp = lp; tp = tmplbl;	/* Copy the label from the macro */
		j = 0;			/* to make it null-terminated */
		while (*tp = *xp) {
		    if (j++ > 50) break;  /* j = length of word from macro */
		    if (*tp < 33 || *tp == ',')	/* Look for end of word */
		      break;
		    else tp++, xp++;	/* Next character */
		}
		*tp = '\0';		/* In case we stopped early */
		/* Now do caseless string comparison, using longest length */
		debug(F111,"macro GOTO label",s,y);
		debug(F111,"macro target label",tmplbl,j);
		if (!xxstrcmp(s,tmplbl,(y > j) ? y : j))
		  break;
		else flag = 0;
	    }
	    if (i == m) {		/* didn't find the label */
		debug(F101,"goto failed at cmdlvl","",cmdlvl);
		if (!popclvl()) {	/* pop up to next higher level */
		    printf("?Label '%s' not found\n",s); /* if none */
		    return(0);		/* quit */
		} else continue;	/* otherwise look again */
	    }
	    debug(F110,"goto found macro label",lp,0);
	    macp[maclvl] = lp;		/* set macro buffer pointer */
	    return(1);
	} else if (cmdstk[cmdlvl].src == CMD_TF) {
	    x = 0;			/* GOTO issued in take file */
	    if (cx == XXGOTO)		/* If GOTO, but not FORWARD, */
	      rewind(tfile[tlevel]);	/* search file from beginning */
	    while (! feof(tfile[tlevel])) {
		if (fgets(line,LINBUFSIZ,tfile[tlevel]) == NULL) /* Get line */
		  break;		/* If no more, done, label not found */
		lp = line;		/* Got line */
		while (*lp == SP || *lp == HT)
		  lp++;			/* Strip leading whitespace */
		if (*lp != ':') continue; /* Check for label introducer */
		tp = lp;		/* Get end of word */
		j = 0;
		while (*tp) {		/* And null-terminate it */
		    if (*tp < 33) {
			*tp = '\0';
			break;
		    } else tp++, j++;
		}
		if (!xxstrcmp(lp,s,(y > j) ? y : j)) { /* Caseless compare */
		    x = 1;		/* Got it */
		    break;		/* done. */
		}
	    }
	    if (x == 0) {		/* If not found, print message */
		debug(F101,"goto failed at cmdlvl","",cmdlvl);
		if (!popclvl()) {	/* pop up to next higher level */
		    printf("?Label '%s' not found\n",s);	/* if none */
		    return(0);		/* quit */
		} else continue;	/* otherwise look again */
	    }
	    return(x);			/* Send back return code */
	}
    }
    printf("?Stack problem in GOTO %s\n",s); /* Shouldn't see this */
    return(0);
}
#endif /* NOSPL */

/* Finish parsing and do the IF, XIF, and WHILE commands */

char *
brstrip(p) char *p; {
    if (!p) return("");
    if (*p == '{') {
	int x;
	x = (int)strlen(p) - 1;
	if (p[x] == '}') {
	    p[x] = NUL;
	    p++;
	}
    }
    return(p);
}

#ifndef NOSPL
int
doif(cx) int cx; {
    int x, y, z; char *s, *p;

    not = 0;				/* Flag for whether "NOT" was seen */
    z = 0;				/* Initial IF condition */
    ifargs = 0;				/* Count of IF condition words */

ifagain:
    if ((ifc = cmkey(iftab,nif,"","",xxstring)) < 0) { /* If what?... */
	if (ifc == -3) {
	    printf("?Condition required\n");
	    return(-9);
	} else return(ifc);
    }
    switch (ifc) {			/* set z = 1 for true, 0 for false */
      case XXIFNO:			/* IF NOT */
	not ^= 1;			/* So NOT NOT NOT ... will work */
	ifargs++;
	goto ifagain;
      case XXIFSU:			/* IF SUCCESS */
	z = ( success != 0 );
	debug(F101,"if success","",z);
	ifargs += 1;
	break;
      case XXIFFA:			/* IF FAILURE */
	z = ( success == 0 );
	debug(F101,"if failure","",z);
	ifargs += 1;
	break;
      case XXIFDE:			/* IF DEFINED */
	if ((x = cmfld("Macro or variable name","",&s,NULL)) < 0) {
	    if (x == -3) return(-2);
	    else return(x);
	}
#ifdef COMMENT
	strcpy(line,s);			/* Make a copy */
        if ((int)strlen(line) < 1) return(-2);
	lp = line;
	if (line[0] == CMDQ && (line[1] == '%' || line[1] == '&')) lp++;
	if (*lp == '%')	{		/* Is it a variable? */
	    x = *(lp + 1);		/* Fold case */
	    if (isupper(x)) *(lp + 1) = tolower(x);
	    if (x >= '0' && x <= '9' && maclvl > -1) /* Digit is macro arg */
	      z = ( (m_arg[maclvl][x - '0'] != (char *)0)
		   && (int)strlen(m_arg[maclvl][x - '0']) != 0);
	    else			/* Otherwise it's a global variable */
	      z = ( (g_var[x] != (char *)0)
		   &&  (int)strlen(g_var[x]) != 0 );
	} else if (*lp == '&') {	/* Array reference */
	    int cc, nn;
	    if (arraynam(lp,&cc,&nn) < 0)
	      z = 0;
	    else z = (arrayval(cc,nn) == NULL ? 0 : 1);
	}
#else
        if ((int)strlen(s) < 1) return(-2);
	z = 0;				/* Assume failure. */
	if (*s == CMDQ) {		/* Object begins with backslash. */
	    char c;
	    c = s[1];			/* Character following backslash */
	    if (c) {
		c = islower(c) ? toupper(c) : c;
		if (c == '%' ||		/* Simple variable */
		    c == '&' ||		/* Array element */
		    c == '$' ||		/* Environment variable */
		    c == 'V' ||		/* Builtin named variable */
		    c == 'M' ||		/* Macro name */
		    c == 'F') {		/* Builtin function */
		    int t;		/* Let zzstring() evaluate it */
		    t = LINBUFSIZ-1;	/* This lets us test \v(xxx) */
		    lp = line;		/* and even \f...(xxx) */
		    zzstring(s,&lp,&t);	/* Evaluate it, whatever it is. */
		    t = strlen(line);	/* Get its length. */
		    debug(F111,"IF DEF",line,t);
		    z = t > 0;		/* If length > 0, it's defined */
		}
	    }
	}
#endif /* COMMENT */
	else {			/* Otherwise it's a macro name */
	    z = ( mxlook(mactab,s,nmac) > -1 ); /* Look for exact match */
	}
	debug(F111,"if defined",s,z);
	ifargs += 2;
	break;

      case XXIFBG:			/* IF BACKGROUND */
      case XXIFFG:			/* IF FOREGROUND */    
	bgchk();			/* Check background status */
	if (ifc == XXIFFG)		/* Foreground */
	  z = pflag ? 1 : 0;
        else z = pflag ? 0 : 1;		/* Background */
	ifargs += 1;
	break;

      case XXIFCO:			/* IF COUNT */
	z = ( --count[cmdlvl] > 0 );
	if (cx == XXWHI) count[cmdlvl] += 2; /* Don't ask... */
	debug(F101,"if count","",z);
	ifargs += 1;
	break;

      case XXIFEX:			/* IF EXIST */
#ifdef CK_TMPDIR
      case XXIFDI:			/* IF DIRECTORY */
#endif /* CK_TMPDIR */
	if ((x = cmfld(
		       ((ifc == XXIFEX) ? "File" : "Directory name"),
		       "",&s,
#ifdef OS2
		       NULL		/* This allows \'s in filenames */
#else
		       xxstring
#endif /* OS2 */
		       )) < 0) {
	    if (x == -3) {
		printf("?File or directory name required\n");
		return(-9);
	    } else return(x);
	}
	if (ifc == XXIFEX) {
	    z = ( zchki(s) > -1L );
	    debug(F101,"if exist 1","",z);
#ifdef OS2	    
	    if (!z) {			/* File not found. */
		int t;			/* Try expanding variables */
		t = LINBUFSIZ-1;	/* and looking again. */
		lp = line;
		zzstring(s,&lp,&t);
		s = line;
		z = ( zchki(s) > -1L );
		debug(F101,"if exist 2","",z);
	    }
#endif /* OS2 */
#ifdef CK_TMPDIR
	} else {
	    z = isdir(s)
#ifdef OS2
	      || (isalpha(cmarg2[0]) &&
		  cmarg2[1] == ':' &&
		  cmarg2[2] == '\0')
#endif /* OS2 */
	      ;
	    debug(F101,"if directory 1","",z);

	    if (!z) {			/* File not found. */
		int t;			/* Try expanding variables */
		t = LINBUFSIZ-1;	/* and looking again. */
		lp = line;
		zzstring(s,&lp,&t);
		s = line;
		z = isdir(s)
#ifdef OS2
		  || (isalpha(cmarg2[0]) &&
		      cmarg2[1] == ':' &&
		      cmarg2[2] == '\0')
#endif /* OS2 */
		    ;
		debug(F101,"if directory 2","",z);
	    }
#endif /* CK_TMPDIR */
	}
	ifargs += 2;
	break;

      case XXIFEQ: 			/* IF EQUAL (string comparison) */
      case XXIFLL:			/* IF Lexically Less Than */
      case XXIFLG:			/* If Lexically Greater Than */
	if ((x = cmfld("first word or variable name","",&s,xxstring)) < 0) {
	    if (x == -3) {
		printf("?Text required\n");
		return(-9);
	    } else return(x);
	}
	s = brstrip(s);			/* Strip braces */
	x = (int)strlen(s);
	if (x > LINBUFSIZ-1) {
	    printf("?IF: strings too long\n");
	    return(-2);
	}
	lp = line;			/* lp points to first string */
	strcpy(lp,s);
	if ((y = cmfld("second word or variable name","",&s,xxstring)) < 0) {
	    if (y == -3) {
		printf("?Text required\n");
		return(-9);
	    } else return(y);
	}
	s = brstrip(s);
	y = (int)strlen(s);
	if (x + y + 2 > LINBUFSIZ) {
	    printf("?IF: strings too long\n");
	    return(-2);
	}
	tp = lp + y + 2;		/* tp points to second string */
	strcpy(tp,s);
	debug(F111,"IF EQ string 1, x",lp,x);
	debug(F111,"IF EQ string 2, y",tp,y);
	if (inpcas[cmdlvl]) {		/* INPUT CASE OBSERVE */
	    x = strcmp(lp,tp);
	    debug(F101,"IF EQ strcmp","",x);
	} else {				/* INPUT CASE IGNORE */
	    x = xxstrcmp(lp,tp,(y > x) ? y : x); /* Use longest length */
	    debug(F101,"IF EQ xxstrcmp","",x);
	}
	switch (ifc) {
	  case XXIFEQ: 			/* IF EQUAL (string comparison) */
	    z = (x == 0);
	    break;
	  case XXIFLL:			/* IF Lexically Less Than */
	    z = (x < 0);
	    break;
	  case XXIFLG:			/* If Lexically Greater Than */
	    z = (x > 0);
	    break;
	}
	ifargs += 3;
	break;

      case XXIFAE:			/* IF (arithmetically) = */
      case XXIFLT:			/* IF (arithmetically) < */
      case XXIFGT: {			/* IF (arithmetically) > */
	/* Really should use longs here... */
	/* But cmnum parses ints. */
	int n1, n2;
	x = cmfld("first number or variable name","",&s,xxstring);
	if (x == -3) {
	    printf("?Quantity required\n");
	    return(-9);
	}
	if (x < 0) return(x);
	debug(F101,"xxifgt cmfld","",x);
	lp = line;
	strcpy(lp,s);
	debug(F110,"xxifgt exp1",lp,0);
	if (!xxstrcmp(lp,"count",5)) {
	    n1 = count[cmdlvl];
	} else if (!xxstrcmp(lp,"version",7)) {
	    n1 = (int) vernum;
	} else if (!xxstrcmp(lp,"argc",4)) {
	    n1 = (int) macargc[maclvl];
	} else {
	    if (!chknum(lp)) return(-2);
	    n1 = atoi(lp);
	}
	y = cmfld("second number or variable name","",&s,xxstring);
	if (y == -3) {
	    printf("?Quantity required\n");
	    return(-9);
	}
	if (y < 0) return(y);
        if ((int)strlen(s) < 1) return(-2);
	x = (int)strlen(lp);
	tp = line + x + 2;
	strcpy(tp,s);
	debug(F110,"xxifgt exp2",tp,0);
	if (!xxstrcmp(tp,"count",5)) {
	    n2 = count[cmdlvl];
	} else if (!xxstrcmp(tp,"version",7)) {
	    n2 = (int) vernum;
	} else if (!xxstrcmp(tp,"argc",4)) {
	    n2 = (int) macargc[maclvl];
	} else {
	    if (!chknum(tp)) return(-2);
	    n2 = atoi(tp);
	}
	debug(F101,"xxifft ifc","",ifc);
	z = ((n1 <  n2 && ifc == XXIFLT)
	  || (n1 == n2 && ifc == XXIFAE)
	  || (n1 >  n2 && ifc == XXIFGT));
	debug(F101,"xxifft n1","",n1);
	debug(F101,"xxifft n2","",n2);
	debug(F101,"xxifft z","",z);
	ifargs += 3;
	break; }

      case XXIFNU:			/* IF NUMERIC */
	x = cmfld("variable name or constant","",&s,xxstring);
	if (x == -3) {
	    printf("?Quantity required\n");
	    return(-9);
	}
	if (x < 0) return(x);
	debug(F111,"xxifnu cmfld",s,x);
	lp = line;
	strcpy(lp,s);
	debug(F110,"xxifnu quantity",lp,0);
        z = chknum(lp);
        debug(F101,"xxifnu chknum","",z);
	ifargs += 2;
	break;

#ifdef ZFCDAT
      case XXIFNE: {			/* IF NEWER */
	char d1[20], * d2;		/* Buffers for 2 dates */
	if ((z = cmifi("First file","",&s,&y,xxstring)) < 0)
	  return(z);
	strcpy(d1,zfcdat(s));
	if ((z = cmifi("Second file","",&s,&y,xxstring)) < 0)
	  return(z);
	d2 = zfcdat(s);
	if ((int)strlen(d1) != 17 || (int)strlen(d2) != 17) {
	    printf("?Failure to get file date\n");
	    return(-9);
	}
	debug(F110,"xxifnewer d1",d1,0);
	debug(F110,"xxifnewer d2",d2,0);
	z = (strcmp(d1,d2) > 0) ? 1 : 0;
        debug(F101,"xxifnewer","",z);
	ifargs += 2;
	break;
      }
#endif /* ZFCDAT */

#ifdef CK_IFRO
      case XXIFRO:			/* REMOTE-ONLY advisory */
	ifargs++;
	z = remonly;
	break;
#endif /* CK_IFRO */

      default:				/* Shouldn't happen */
	return(-2);
    }
    if (not) z = !z;			/* Handle NOT here for both IF & XIF */

    switch (cx) {			/* Separate handling for IF and XIF */

      case XXIF:			/* This is IF... */
	ifcmd[cmdlvl] = 1;		/* We just completed an IF command */
        debug(F101,"IF condition","",z);
	if (z) {			/* Condition is true */
	    iftest[cmdlvl] = 1;		/* Remember that IF succeeded */
	    if (maclvl > -1) {		/* In macro, */
		pushcmd();		/* save rest of command. */
	    } else if (tlevel > -1) {	/* In take file, */
		debug(F100, "doif: pushing command", "", 0);
		pushcmd();		/* save rest of command. */
	    } else {			/* If interactive, */
		cmini(ckxech);		/* just start a new command */
		printf("\n");		/* (like in MS-DOS Kermit) */
		if (pflag) prompt(xxstring);
	    }
	} else {			/* Condition is false */
	    iftest[cmdlvl] = 0;		/* Remember command failed. */
	    if ((y = cmtxt("command to be ignored","",&s,NULL)) < 0)
	      return(y);		/* Gobble up rest of line */
	}
	return(0);

      case XXIFX: {			/* This is XIF (Extended IF) */
	  char *p;
	  char e[5];
	  int i;
	  if ((y = cmtxt("Object command","",&s,NULL)) < 0)
	    return(y);			/* Get object command. */
	  p = s;
	  lp = line;
	  if (litcmd(&p,&lp) < 0) {	/* Insert quotes in THEN-part */
	      return(-2);
	  }
	  if (!z) {			/* Use ELSE-part, if any */
	      lp = line;		/* Write over THEN part. */
	      *lp = NUL;
	      while (*p == SP) p++;	/* Strip trailing spaces */
	      if (*p) {			/* At end? */
		  for (i = 0; i < 4; i++) e[i] = *p++; /* No, check for ELSE */
		  if (xxstrcmp(e,"else",4)) return(-2);	/* No, syntax error */
		  if (litcmd(&p,&lp) < 0) { /* Insert quotes */
		      return(-2);
		  }
		  while (*p == SP) p++;	/* Strip trailing spaces */
		  if (*p) return(-2);	/* Should be nothing here. */
	      }
	  }
	  if (line[0]) {
	      x = mlook(mactab,"_xif",nmac); /* get index of "_xif" macro. */
	      if (x < 0) {			/* Not there? */
		  addmmac("_xif",xif_def);	/* Put it back. */
		  if (mlook(mactab,"_xif",nmac) < 0) { /* Look it up again. */
		      printf("?XIF macro gone!\n");
		      return(success = 0);
		  }
	      }
	      dodo(x,line);		/* Do the XIF macro */
	  }
	  return(0);
      }
      case XXWHI: {			/* WHILE Command */
	  p = cmdbuf;			/* Capture IF condition */
	  ifcond[0] = NUL;		/* from command buffer */
	  while (*p == SP) p++;
	  while (*p != SP) p++;
	  ifcp = ifcond;
	  strcpy(ifcp,"{ \\flit(if not ");
	  ifcp += (int)strlen(ifcp);
	  while (*p != '{' && *p != NUL) *ifcp++ = *p++;
	  p = " goto bot) } ";
	  while (*ifcp++ = *p++) ;
	  debug(F110,"WHILE cmd",ifcond,0);

	  if ((y = cmtxt("Object command","",&s,NULL)) < 0)
	    return(y);			/* Get object command. */
	  p = s;
	  lp = line;
	  if (litcmd(&p,&lp) < 0) {	/* Insert quotes in object command */
	      return(-2);
	  }
	  debug(F110,"WHILE body",line,0);
	  if (line[0]) {
	      char *p;
	      x = mlook(mactab,"_while",nmac); /* index of "_while" macro. */
	      if (x < 0) {		/* Not there? */
		  addmmac("_while",whil_def); /* Put it back. */
		  if (mlook(mactab,"_while",nmac) < 0) { /* Look it up again */
		      printf("?WHILE macro definition gone!\n");
		      return(success = 0);
		  }
	      }
	      p = malloc((int)strlen(ifcond) + (int)strlen(line) + 2);
	      if (p) {
		  strcpy(p,ifcond);
		  strcat(p,line);
		  debug(F110,"WHILE dodo",p,0);
		  dodo(x,p);
		  free(p);
	      } else {
		  printf("?Can't allocate storage for WHILE command");
		  return(success = 0);
	      }
	  }
	  return(0);
      }
      default:
	return(-2);
    }
}
#endif /* NOSPL */

/* Set up a TAKE command file */

int
dotake(s) char *s; {
    if ((tfile[++tlevel] = fopen(s,"r")) == NULL) {
	perror(s);
	debug(F110,"Failure to open",s,0);
	success = 0;
	tlevel--;
    } else {
#ifdef VMS
	conres();			/* So Ctrl-C will work */
#endif /* VMS */
#ifndef NOSPL
	cmdlvl++;			/* Entering a new command level */
	if (cmdlvl > CMDSTKL) {
	    cmdlvl--;
	    printf("?TAKE files and/or DO commands nested too deeply\n");
	    return(success = 0);
	}
	if (tfnam[tlevel]) free(tfnam[tlevel]);	/* Copy the filename */
	if (tfnam[tlevel] = malloc(strlen(s) + 1))
	  strcpy(tfnam[tlevel],s);
	ifcmd[cmdlvl] = 0;		/* Set variables for this cmd file */
	iftest[cmdlvl] = 0;
	count[cmdlvl]  = count[cmdlvl-1];  /* Inherit this */
	intime[cmdlvl] = intime[cmdlvl-1]; /* Inherit this */
	inpcas[cmdlvl] = inpcas[cmdlvl-1]; /* Inherit this */
	takerr[cmdlvl] = takerr[cmdlvl-1]; /* Inherit this */
	merror[cmdlvl] = merror[cmdlvl-1]; /* Inherit this */
	cmdstk[cmdlvl].src = CMD_TF;	/* Say we're in a TAKE file */
	cmdstk[cmdlvl].lvl = tlevel;	/* nested at this level */
#else
	takerr[tlevel] = takerr[tlevel-1]; /* Inherit this */
#endif /* NOSPL */
    }
    return(1);
}
#endif /* NOICP */
