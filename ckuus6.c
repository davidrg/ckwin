#include "ckcsym.h"
#ifndef NOICP
 
/*  C K U U S 6 --  "User Interface" for Unix Kermit (Part 6)  */

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

/* Includes */
 
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"
#include "ckcnet.h"			/* Network symbols */
#include <signal.h>
 
#ifdef datageneral
#define fgets(stringbuf,max,fd) dg_fgets(stringbuf,max,fd)
#endif /* datageneral */

/* External Kermit Variables, see ckmain.c for description. */
 
extern xx_strp xxstring;

extern int size, rpsiz, urpsiz, local, stdinf, sndsrc, xitsta,
  displa, binary, parity, escape, flow, cmd_rows,
  turn, duplex, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  turnch, dfloc, keep, maxrps, warn, cnflg, tlevel, pflag, msgflg,
  mdmtyp, zincnt, fblksiz, frecl, frecfm, atcapr, atdiso, verwho, quiet;
extern int repars, techo, network;
 
#ifdef CK_IFRO
extern int remonly;
#endif /* CK_IFRO */

#ifdef OS2
extern int StartedFromDialer ;
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
#endif /* OS2 */

extern long vernum, speed;
extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *dialv, *loginv, *for_def[], *whil_def[], *xif_def[], *sw_def[];
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
extern char * dldir;
#endif /* CK_TMPDIR */

/* Declarations from cmd package */
 
#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;		/* Command buffers */
#else
extern char cmdbuf[], atmbuf[];		/* Command buffers */
#endif /* DCMDBUF */

#ifndef NOPUSH
extern int nopush ;
#endif /* NOPUSH */

#ifndef NOSPL
extern struct mtab *mactab;
extern int nmac;
extern long ck_alarm;
extern char alrm_date[], alrm_time[];
extern int x_ifnum;
#endif /* NOSPL */

/* Declarations from ck?fio.c module */
 
extern int backgrd;			/* Kermit executing in background */
 
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

int readblock = 4096;			/* READ buffer size */
CHAR * readbuf = NULL;			/* Pointer to read buffer */
int readsize = 0;			/* Number of chars actually read */

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
    "alarm",      XXIFAL, 0,
    "background", XXIFBG, 0,
    "count",      XXIFCO, 0,
    "defined",    XXIFDE, 0,
#ifdef CK_TMPDIR
    "directory",  XXIFDI, 0,
#endif /* CK_TMPDIR */
#ifdef COMMENT
    "eof",        XXIFEO, 0,
#endif /* COMMENT */
    "emulation",  XXIFEM, 0,
    "equal",      XXIFEQ, 0,
    "error",      XXIFFA, CM_INV,
    "exist",      XXIFEX, 0,
    "failure",    XXIFFA, 0,
    "false",      XXIFNT, 0,
    "foreground", XXIFFG, 0,
    "llt",        XXIFLL, 0,
    "lgt",        XXIFLG, 0,
#ifdef OS2
    "terminal-macro", XXIFTM, CM_INV,
#endif /* OS2 */
#ifdef ZFCDAT
    "newer",      XXIFNE, 0,
#endif /* ZFCDAT */
    "not",        XXIFNO, 0,
    "ok",         XXIFSU, CM_INV,
    "numeric",    XXIFNU, 0,
    "remote-only",XXIFRO, 0,
    "started-from-dialer",XXIFSD, CM_INV,
    "success",    XXIFSU, 0,
    "true",       XXIFTR, 0
};
int nif = (sizeof(iftab) / sizeof(struct keytab));
#endif /* NOSPL */

/* Variables and symbols local to this module */
 
#ifndef NODIAL 
_PROTOTYP(static int ddcvt, (char *, FILE *, int) );
_PROTOTYP(static int dncvt, (int, int) );
_PROTOTYP(char * getdname, (void) );

char *dialnum = (char *)0;		/* Remember DIAL number for REDIAL */
int dirline = 0;			/* Dial directory line number */
extern char * dialdir[];		/* Dial directory file names */
extern int dialdpy;			/* DIAL DISPLAY on/off */
extern int ndialdir;			/* How many dial directories */
#ifdef NETCONN
extern int nnetdir;			/* How many network directories */
#endif /* NETCONN */
extern int ntollfree;			/* Toll-free call info */
extern char *dialtfc[];
extern int tttapi;
extern int dialatmo;
extern char * dialnpr, * dialsfx;
extern char * diallcc;			/* Dial local country code */
extern char * diallac;			/* Dial local area code */
extern char * dialixp, * dialixs;
extern char * dialldp, * diallds, * dialtfp;
extern char * dialpxx, * dialpxi, * dialpxo;
extern int dialcnf;			/* DIAL CONFIRMATION */
int dialsrt = 1;			/* DIAL SORT ON */
int dialrstr = 6;			/* DIAL RESTRICTION */

extern int dialsta;			/* Dial status */
int dialrtr = 0,			/* Dial retries */
    dialint = 10;			/* Dial retry interval */
extern long dialcapas;			/* Modem capabilities */
extern int dialcvt;			/* DIAL CONVERT-DIRECTORY */
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
/* Flags for ENABLE/DISABLE */
extern int en_cwd, en_cpy, en_del, en_dir, en_fin, 
   en_get, en_hos, en_ren, en_sen, en_set, en_spa, en_typ, en_who, en_bye,
   en_asg, en_que, en_ret, en_mai, en_pri;
#endif /* NOSERVER */

extern FILE *tfile[];			/* File pointers for TAKE command */
extern char *tfnam[];			/* Names of TAKE files */
extern int tfline[];			/* TAKE-file line number */

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

#ifdef OS2
_PROTOTYP( int os2settitle, (char *, int) );
#endif /* OS2 */

extern struct keytab yesno[];
extern int nyesno;

#ifndef NOSPL

/* Do the ASK, ASKQ, GETOK, and READ commands */

int
doask(cx) int cx; {
    extern int cmflgs;
#ifdef CK_RECALL
    int sv_recall;
    extern int on_recall;
#endif /* CK_RECALL */

    char vnambuf[VNAML];		/* Buffer for variable names */
    char *vnp = NULL;			/* Pointer to same */
    if (cx != XXGOK && cx != XXRDBL) {	/* Get variable name */
	if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
	    if (y == -3) {
		printf("?Variable name required\n");
		return(-9);
	    } else return(y);
	}
	strcpy(vnambuf,s);		/* Make a copy. */
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
    if (cx == XXREA || cx == XXRDBL) {	/* READ or READBLOCK command */
	if ((y = cmcfm()) < 0)		/* Get confirmation */
	  return(y);
	if (chkfn(ZRFILE) < 1) {	/* File open? */
	    printf("?Read file not open\n");
	    return(0);
	}
	if (!(s = (char *)readbuf)) {		/* Where to read into. */
	    printf("?Oops, no READ buffer!\n");
	    return(0);
	}
#ifdef BINREAD
	if (cx == XXRDBL) {		/* READBLOCK */
	    y = zxin(ZRFILE, s, readblock);
	    if (y < 1) {
		zclose(ZRFILE);		/* close the file, */
		return(success = 0);
	    }
	    readsize = y;
	    printf("READBLOCK %d\n",y);
	} else
#endif /* BINREAD */
	  {
	    y = zsinl(ZRFILE, s, readblock); /* Read a line. */
	    debug(F111,"read zsinl",s,y);
	    if (y < 0) {		/* On EOF or other error, */
		zclose(ZRFILE);		/* close the file, */
		delmac(vnp);		/* delete the variable, */
		return(success = 0);	/* and return failure. */
	    } else {			/* Read was OK. */
		readsize = (int) strlen(s);
		success = (addmac(vnp,s) < 0 ? 0 : 1); /* Define variable */
		debug(F111,"read addmac",vnp,success);
		return(success);	/* Return success. */
	    }
	}
    }

    /* ASK, ASKQ, or GETOK */

    if ((y = cmtxt("Prompt, enclose in { braces } to preserve\n\
leading and trailing spaces, precede question mark with backslash (\\).",
		   cx == XXGOK ? "{ Yes or no? }" : "",
		   &p,NULL)) < 0) {
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
reprompt:
    if (cx == XXASKQ) {			/* For ASKQ, */
	concb((char)escape);		/* put console in cbreak mode */
	cmini(0);			/* and no-echo mode. */
    } else {				/* For others, regular echoing. */
	cmini(ckxech);
    }
    x = -1;				/* This means to reparse. */
    cmflgs = 0;
    if (pflag) prompt(xxstring);	/* Issue prompt. */
reparse:
    cmres();
    if (cx == XXGOK) {
#ifdef CK_RECALL
	sv_recall = on_recall;
	on_recall = 0;
#endif /* CK_RECALL */
	x = cmkey(yesno,nyesno,"","",xxstring);	/* GETOK uses keyword table */
	if (x < 0) {			/* Parse error */
	    if (x == -3) {		/* No answer? */
		printf("Please respond Yes or No\n"); /* Make them answer */
		cmini(ckxech);
		goto reprompt;
	    } else if (x == -1) {
		goto reparse;
	    } else
	      goto reprompt;
	}
	if (cmcfm() < 0)		/* Get confirmation */
	  goto reparse;
	cmsetp(psave);			/* Restore prompt */
#ifdef VMS
	if (cmdlvl > 0)			/* In VMS and not at top level, */
	  conres();			/*  restore console again. */
#endif /* VMS */
#ifdef CK_RECALL
	on_recall = sv_recall;
#endif /* CK_RECALL */
	return(x);			/* Return success or failure */
    } else if (cx == XXGETC) {		/* GETC */
	char tmp[2];
	x = coninc(0);			/* Just read one character */
	if (x > -1) {
	    printf("\r\n");
	    tmp[0] = (char) (x & 0xff);
	    tmp[1] = NUL;
	    y = addmac(vnp,tmp);	/* Add it to the macro table. */
	    debug(F111,"getc addmac",vnp,y);
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
	    debug(F111,"ASK cmtxt",s,x);
	    cmres();
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
	    delmac(vnp);		/* delete variable if it exists, */
#ifdef VMS
	    if (cmdlvl > 0)		/* In VMS and not at top level, */
	      conres();			/*  restore console again. */
#endif /* VMS */
	    return(success = 1);	/* and return. */
	}
	y = addmac(vnp,s);		/* Add it to the macro table. */
	debug(F111,"ask addmac",vnp,y);
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
    char vnambuf[VNAML+1];		/* Buffer for variable names */

    if ((y = cmfld("Variable name","",&s,NULL)) < 0) {
	if (y == -3) {
	    printf("?Variable name required\n");
	    return(-9);
	} else return(y);
    }
    strncpy(vnambuf,s,VNAML);
    if ((y = cmnum("by amount","1",10,&x,xxstring)) < 0)
      return(y);
    if ((y = cmcfm()) < 0)
      return(y);

    z = (cx == XXINC ? 1 : 0);		/* Increment or decrement? */

    if (incvar(vnambuf,x,z) < 0) {
	printf("?Variable %s not defined or not numeric\n",vnambuf);
	return(success = 0);
    }
    return(success = 1);
}
#endif /* NOSPL */


/* Do the (_)DEFINE, (_)ASSIGN, and UNDEFINE commands */

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
	if (cx == XXUNDEF) {		/* Undefine */
	    if ((y = cmcfm()) < 0) return(y);
	    delmac(vnp);
	    return(success = 1);
	}
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
	if (cx == XXUNDEF) {		/* Undefine */
	    if ((y = cmcfm()) < 0) return(y);
	    delmac(vnp);
	    return(success = 1);
	}
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
/*
   L U D I A L  --  Lookup up dialing directory entry.
  
   Call with string to look up and file descriptor of open dialing directory
   file.  On success, returns number of matches found, with numbers stored
   in an array accessible via getdnum().
*/
static char *dn_p[MAXDNUMS + 1];	/* Dial Number pointers */
static char *dn_p2[MAXDNUMS + 1];	/* Converted dial number pointers */
static int dn_x[MAXDNUMS + 1];		/* Type of call */
static int dncount = 0;
char * d_name = NULL;			/* Dial name pointer */

char *					/* Get dial directory entry name */
getdname() {
    return(d_name ? d_name : "");
}

char *
getdnum(n) int n; {			/* Get dial number n from directory */
    if (n < 0 || n > dncount || n > MAXDNUMS)
      return("");
    else
      return(dn_p[n]);
}

char *			/* Check area code for spurious leading digit */
chk_ac(i,buf) int i; char buf[]; {
    char *p;
    if (!buf)
      return("");
    p = (char *) buf;			/* Country we are calling: */
    if (i ==  44 ||			/* UK */
	i ==  49 ||			/* Germany */
	i ==  39 ||			/* Italy */
	i ==  31 ||			/* Netherlands */
	i == 351 ||			/* Portugal */
	i ==  55 ||			/* Brazil */
	i == 972 ||			/* Israel */
	i ==  41 ||			/* Switzerland */
	i ==  43 ||			/* Austria */
	i ==  42 ||			/* Czech Republic */
	i ==  36 ||			/* Hungary */
	i ==  30 ||			/* Greece */
	i == 352 ||			/* Luxembourg */
	i ==  48 ||			/* Poland */
	i ==  27 ||			/* South Africa */
	((i == 33) && ((int) strcmp(zzndate(),"19961017") > 0))	/* France */
	) {
	if (buf[0] == '0')
	  p++;
    } else if (i == 358) {		/* Finland */
	char c = '9';
	if ((int) strcmp(zzndate(),"19961011") > 0)
	  c = '0';
	if (buf[0] == c)		/* But only until 12 Oct 96! */
	  p++;
    }
    return(p);
}

static int
dncvt(k,cx) int k, cx; {		/* Dial Number Convert */
    int i, j, n, what;
    char *ss;
    char *p, *pxo;
    char **pp;
    char *lac;
    char *npr;
    char *sfx;
    char ccbuf[128];
    int cc;
    char acbuf[24];
    char *acptr;
    char outbuf[256];

#define DN_INTERN 0
#define DN_FREE   1
#define DN_LOCAL  2
#define DN_UNK    3
#define DN_LONG   4
#define DN_INTL   5
/*
  First pass for strict (punctuation-based) interpretation.
  If it fails, we try the looser (length-based) one.
*/
    what = 0;				/* Type of call */
    s = dn_p[k];			/* Number to be converted. */
    debug(F111,"dncvt",s,k);
    if (dn_p2[k]) {
	free(dn_p2[k]);
	dn_p2[k] = NULL;
    }
    if (!s) {
	printf("Error - No phone number to convert\n");
	return(-1);
    }
    pp = &(dn_p2[k]);			/* Address for converted number */

    if (tttapi) {			/* When using TAPI */
	npr = "";			/* TAPI supplies all the */
	sfx = "";			/* prefixes and suffixes */
	pxo = "";
    } else {
	npr = dialnpr ? dialnpr : "";
	sfx = dialsfx ? dialsfx : "";
	pxo = dialpxo ? dialpxo : "";
    }
    lac = diallac ? diallac : "";	/* Local area code */

    outbuf[0] = NUL;			/* Initialize conversion buffer */
    ss = s;				/* Remember original string */

    if (*s != '+') {			/* Literal number */
	sprintf(outbuf,			/* Sandwich it between */
		"%s%s%s%s",		/* DIAL PREFIX and SUFFIX */
		pxo,npr,s,sfx
		);
	makestr(pp, outbuf);
	dn_x[k] = DN_UNK;		/* Sort key is "unknown". */
	return(0);			/* Done. */
    }
    i = 0;				/* Portable number */
    s++;				/* Tiptoe past the plus sign */
    ccbuf[0] = NUL;			/* Do country code first */

    if (!diallcc) {			/* Do we know our own? */
	if (cx != XXLOOK)
	  printf("Error - prior SET DIAL COUNTRY-CODE command required\n");
	return(-1);
    }

    /* Parse the number */

    while (1) {				/* Get the country code */
        while (*s == HT || *s == SP)
	  s++;
	if (!s)				/* Not in standard format */
	  break;
        if (*s == '(') {		/* Beginning of area code  */
	    s++;			/* Skip past parenthesis   */
	    ccbuf[i] = NUL;		/* End of country code */
	    if (!s) {			/* Check for end of string */
		printf("Error - phone number ends prematurely: \"%s\"\n",ss);
		return(-1);
	    }
	    break;
	} else {			/* Collect country code */
	    if (isdigit(*s))
	      ccbuf[i++] = *s;		/* copy this character */
	    s++;
	    if (!*s || i > 127)		/* watch out for memory leak */
	      break;
	}
    }
    cc = atoi(ccbuf);			/* Numeric version of country code */

    i = 0;				/* Now get area code */
    acbuf[0] = NUL;			/* Initialize area-code buffer */
    acptr = acbuf;			/* and pointer. */
    while (1) {
        while (*s == HT || *s == SP)	/* Ignore whitespace */
	  s++;
	if (!s)				/* String finished */
	  break;
	if (*s == ')') {		/* End of area code  */
	    s++;			/* Skip past parenthesis   */
	    acbuf[i] = NUL;		/* Terminate area-code buffer */
	    break;
	} else {			/* Part of area code */
	    if (isdigit(*s))		/* If it's a digit, */
	      acbuf[i++] = *s;		/* copy this character */
	    s++;			/* Point to next */
	    if (!*s || i > 23)		/* Watch out for overflow */
	      break;
	}
    }

/*
   Here we strip any leading 0 for countries that we know have
   0 as a long-distance prefix and do not have any area codes that
   start with 0 (ditto for "9" in Finland...)
*/
    i = atoi(ccbuf);
    acptr = chk_ac(i,acbuf);

    while (*s == HT || *s == SP)	/* Skip whitespace */
      s++;

/* printf("S=[%s], ACPTR=[%s]\n",s,acptr); */

    if (*s && *acptr) {			/* Area code was delimited */

	while (*s == '-' || *s == '.')	/* Skip past gratuitious punctuation */
	  s++;
	if (!*s) s--;			/* But not to end of string */

	if (strcmp(diallcc,ccbuf)) {	/* Out of country? */
	    char * p2;
	    if (!dialixp) {		/* Need intl-prefix */
		if (cx != XXLOOK)
		  printf("Error - No international dialing prefix defined\n");
		return(-1);
	    }
	    what = dn_x[k] = DN_INTL;
	    if (tttapi) {
		p  = "";		/* Intl-suffix */
		p2 = "";		/* Intl-prefix */
	    } else {
		p  = dialixp ? dialixp : ""; /* Intl-prefix */
		p2 = dialixs ? dialixs : ""; /* Intl-suffix */
	    }
	    sprintf(outbuf,		/* Form the final phone number */
		    "%s%s%s%s%s%s%s%s",
		    pxo,npr,p,ccbuf,acptr,s,p2,sfx
		    );
	} else if (strcmp(lac,acptr)) { /* In-country */
	    char * p2;
#ifdef COMMENT
/* Wrong - Sometimes it has to be null, e.g. for NANP 10-digit dialing... */
	    if (!dialldp) {		/* Out of area code */
		if (cx != XXLOOK)	/* Need ld-prefix */
		  printf("Error - No long-distance prefix defined\n");
		return(-1);
	    }
#endif /* COMMENT */
	    if (!diallac && cx != XXLOOK) { /* Don't know my own area code */
		if (cc == 1)
		  printf("WARNING - Prior SET DIAL AREA-CODE needed\n");
	    }

	    what = dn_x[k] = DN_LONG;	/* Long-distance */

	    for (i = 0; i < ntollfree; i++) { /* Check for toll-free call */
		if (!strcmp(acptr,dialtfc[i])) {
		    what = dn_x[k] = DN_FREE;		  
		    break;
		}
	    }
	    if (tttapi) {		/* TAPI supplies its own */
		p  = "";		/* ld-suffix */
		p2 = "";		/* ld-prefix */
	    } else if (what == DN_FREE) { /* Toll-free call */
		p = dialtfp ? dialtfp : (dialldp ? dialldp : "");
		p2 = "";		/* no suffix */
	    } else {			/* normal long distance */
		p  = dialldp ? dialldp : ""; /* ld-prefix */
		p2 = diallds ? diallds : ""; /* ld-suffix */
	    }
	    sprintf(outbuf,"%s%s%s%s%s%s%s", /* Form the number to be dialed */
		    pxo,npr,p,acptr,s,p2,sfx
		    );
	} else {			/* Same country, same area code */
	    what = dn_x[k] = DN_LOCAL;	/* So it's a local call. */
	    if (!dialpxo || tttapi) {	/* TAPI or not dialing out from PBX */
		sprintf(outbuf,"%s%s%s",
			npr,s,sfx
			);
	    } else {			/* Dialing from a PBX and not TAPI */
		if (dialpxx) {		/* Is it internal? */
		    i = (int) strlen(dialpxx);
		    j = (int) strlen(s);
		    x = -1;
		    if (j > i)
		      x = xxstrcmp(dialpxx,s,i);
		    if (!x) {
			what = dn_x[k] = DN_INTERN;   /* Internal call. */
			s += i;
			p = (dialpxi) ? dialpxi : ""; /* Internal prefix */
			sprintf(outbuf,"%s%s%s%s",    
				npr,p,s,sfx
				);
		    } else {	/* External local call */
			sprintf(outbuf,"%s%s%s%s",
				dialpxo,npr,s,sfx
				);
		    }
		}
	    }
	}

    } else {				/* Area code was not delimited */

	char xbuf[256];			/* Comparison based only on length */
	char ybuf[256];
	s = ss;
	for (i = 0; i < 255; i++) {
	    if (!*s)
	      break;
	    while (!isdigit(*s))	/* Only pay attention to digits */
	      s++;
	    xbuf[i] = *s++;
	}
	xbuf[i] = NUL;
	sprintf(ybuf,"%s%s",diallcc,lac);
	n = (int) strlen(ybuf);
	if (n > 0 && !xxstrcmp(xbuf,ybuf,n)) { /* Local call */
	    dn_x[k] = DN_LOCAL;
	    s = xbuf + n;
	    sprintf(outbuf,"%s%s%s%s",pxo,npr,s,sfx);
	} else {			/* Not local */
	    sprintf(ybuf,"%s",diallcc);
	    n = (int) strlen(ybuf);
	    if (n > 0 && !xxstrcmp(xbuf,ybuf,n)) { /* Long distance */
		char * p2;
		dn_x[k] = DN_LONG;
#ifdef COMMENT
		if (!dialldp) {
		    if (cx != XXLOOK && !tttapi)
		      printf("Error - No long-distance prefix defined\n");
		    return(-1);
		}
#endif /* COMMENT */
		if (tttapi) {
		    p = "";
		    p2 = "";
		} else {
		    p = dialldp ? dialldp : "";
		    p2 = diallds ? diallds : "";
		}
		s = xbuf + n;
		while (*s == '-' || *s == '.')
		  s++;
		sprintf(outbuf,"%s%s%s%s%s%s",pxo,npr,p,s,p2,sfx);
	    } else {
		char * p2;		/* International */
		dn_x[k] = DN_INTL;
		if (!dialixp) {
		    if (cx != XXLOOK && !tttapi) {
			printf(
			  "Error - No international dialing prefix defined\n"
			       );
			return(-1);
		    }
		}
		if (tttapi) {
		    p = "";
		    p2 = "";
		} else {
		    p = dialixp ? dialixp : ""; 
		    p2 = dialixs ? dialixs : "";
		}
		sprintf(outbuf,"%s%s%s%s%s%s",pxo,npr,p,xbuf,p2,sfx);
	    }
	}
    }
    makestr(pp, outbuf);
    return(0);
}

static int
ddcvt(s, f, n) char * s; FILE * f; int n; { /* Dial Directory Convert */
    char *line, *s2;			/* buffers */
#ifdef VMS
    char * temp;
#endif /* VMS */
    char *info[8];			/* Pointers to words from entry */
    FILE * f2;
    int x, rc;
    rc = -1;

    if (!s || !f)			/* No filename or file */
      return(-1);
    if ((int) strlen(s) < 1)
      return(-1);
    if (!(line = malloc(1024)))		/* Allocate input buffer */
      return(-1);
    f2 = NULL;

    fclose(f);
    znewn(s,&s2);			/* s2 = address of static buffer */
#ifdef VMS
    temp = s2;				/* Swap - otherwise the new */
    s2 = s;				/* version has the older version */
    s = temp;				/* number... */
    if (temp =  (char *)malloc((int)strlen(s)+1))
      strcpy(temp,s);
    if (dialdir[n])			/* Replace filename in list */
      free(dialdir[n]);
    dialdir[n] = temp;
    s = temp;
#else
    if (zrename(s,s2) < 0) {		/* Not VMS - rename old file */
	perror(s2);			/* to new (wierd) name. */
	goto ddexit;
    }
#endif /* VMS */
    if ((f = fopen(s2,"r")) == NULL) {	/* Reopen old file with wierd name */
	dirline = 0;			/* (or in VMS, old version) */
	perror(s2);
	goto ddexit;
    }
    if ((f2 = fopen(s,"w")) == NULL) {	/* Create new file with old name */
	perror(s);			/* (or in VMS, new version) */
	goto ddexit;
    }
    printf("\nSaving old directory as %s.\nConverting %s...",s2,s);
    fprintf(f2,"; %s - Kermit dialing directory\n", s);
    fprintf(f2,"%-16s %-20s ; %5s %-6s ; %s\n",
	       "; Name","Number","Speed","Parity","Comment"
	       );

    while (1) {
	line[0] = NUL;			/* Read a line */
	if (fgets(line,1023,f) == NULL)
	  break;
	if (!line[0]) {			/* Empty line */
	    fprintf(f2,"\n");
	    continue;
	}
	x = (int) strlen(line);		/* Strip line terminator, */
	while (x-- > 0) {		/* if any. */
	    if (line[x] <= SP)
	      line[x] = NUL;
	    else
	      break;
	}
	xwords(line,5,info,1);		/* Parse it the old way */
	for (x = 1; x < 6; x++)
	  if (!info[x]) info[x] = "";
	fprintf(f2,"%-16s %-20s ; %5s %-6s %s\n",
	       info[1],info[2],info[3],info[4],info[5]
	       );
    }	
    printf(" OK\n\n");
    rc = 0;				/* Success */
  ddexit:
    if (f) fclose(f);
    if (f2) fclose(f2);
    if (line) free(line);
    return(rc);
}

int					/* s = name to look up   */
#ifdef CK_ANSIC				/* cx = index of command */
ludial(char *s, int cx)			/* (DIAL, LOOKUP, etc)   */
#else
ludial(s, cx) char *s; int cx;
#endif /* CK_ANSIC */
/* ludial */ {

    int dd, n1, n2, n3, i, j, t;	/* Workers */
    int olddir, newdir, oldentry, newentry;
    int pass = 0;
    int oldflg = 0;
    int ambiguous = 0;			/* Flag for lookup was ambiguous */
    char *info[7];			/* Pointers to words from entry */
    char *pp;				/* Pointer to element of array */
    FILE * f;
    char *line;				/* File input buffer */

/* #define LUDEBUG */

#ifdef LUDEBUG
int zz = 1;
#endif /* LUDEBUG */

    if (!s || ndialdir < 1)		/* Validate arguments */
      return(-1);

    if ((n1 = (int) strlen(s)) < 1)	/* Length of string to look up */
      return(-1);

    if (!(line = malloc(1024)))		/* Allocate input buffer */
      return(-1);

#ifdef LUDEBUG
if (zz) printf("LUDIAL 1 s[%s], n1=%d\n",s,n1);
#endif /* LUDEBUG */

    pass = 0;
  lu_again:
    f = NULL;				/* Dial directory file descriptor */
    t = dncount = 0;			/* Dial-number match count */
    dd = 0;				/* Directory counter */
    olddir = 0;
    newdir = 0;
/*
  We need to recognize both old- and new-style directories.
  But we can't allow old-style and new-style entries in the same
  directory because there is no way to tell for sure the difference between
  and old-style entry like this:

    foo  5551212  9600

  and a new-style literal entry like this:

    foo  555 9600

  I.e. is the "9600" a speed, or part of the phone number?
*/
    while (1) {				/* We make one pass */
	if (!f) {			/* Directory not open */
            if (dd >= ndialdir)		/* No directories left? */
	      break;			/* Done. */
	    if ((f = fopen(dialdir[dd],"r")) == NULL) { /* Open it */
		perror(dialdir[dd]);	/* Can't, print message saying why */
		if (line) {
		    free(line);
		    line = NULL;
		}
		dd++;			/* Go on to next one, if any... */
		continue;
	    }
	    dirline = 0;		/* Directory file line number */
	    if (dialdpy && !pass)
	      printf("Opening: %s...\n",dialdir[dd]);
            dd++;
	    if (!oldflg) olddir = 0;
	    newdir = 0;
	}
	oldentry = 0;
	newentry = 0;
	line[0] = NUL;
	if (getnct(line,1023,f,1) < 0) { /* Read a line */
	    if (f) {			/* f can be clobbered! */
		fclose(f);		/* Close the file */
		f = NULL;		/* Indicate next one needs opening */
		oldflg = 0;
	    }
	    continue;
	}
	if (!line[0])			/* Empty line */
	  continue;
#ifdef LUDEBUG
if (zz) printf("LUDIAL 2 s[%s]\n",s);
#endif /* LUDEBUG */

	/* Make a copy and parse it the old way */
	/* A copy is needed because xwords() pokes NULs into the string */

	if (pp = malloc((int)strlen(line) + 1)) {
	    strcpy(pp,line);
	    xwords(pp,5,info,0);	/* Parse it the old way */

#ifdef LUDEBUG
if (zz) printf("LUDIAL 3 s[%s]\n",s);
#endif /* LUDEBUG */

	    if (!info[1])
	      continue;
	    if (*info[1] == ';') {	/* If full-line comment, */
		newdir = 1;		/* (only new directories have them) */
		continue;		/* keep reading. */
	    }
	    if (!info[2]) 
	      continue;
	    if (*info[2] == '+')
	      newentry = 1;
	    if (info[4]) {
		if ((*info[4] == '=') ||
		    !xxstrcmp(info[4],"none", 4) ||
		    !xxstrcmp(info[4],"even", 4) ||
		    !xxstrcmp(info[4],"space",5) ||
		    !xxstrcmp(info[4],"mark", 4) ||
		    !xxstrcmp(info[4],"odd",  3)
		    )
		  oldentry = 1;
	    }
	}
	if (pp) {
	    free(pp);
	    pp = NULL;
	}

	/* Check consistency */

	if ((oldentry || olddir) && (newentry || newdir)) {
	    printf(
"\nERROR: You seem to have old- and new-format entries mixed in your\n");
	    printf(
"dialing directory.  You'll have to edit it by hand to convert it to the\n");
#ifndef NOHELP
	    printf("new format.  Type HELP DIAL for further information.\n\n");
#else
	    printf("new format.\n\n");
#endif /* NOHELP */
	    if (line) {
		free(line);
		line = NULL;
	    }
	    return(-1);
	}
	if (!olddir && oldentry) {
	    int convert = 0;
	    olddir = 1;
	    if (dialcvt == 2) {		/* 2 == ASK */
		printf(
"\nWARNING: Old-style dialing directory detected:\n%s\n\n", line);
		convert = getyesno("Shall I convert it for you? ");
	    } else
	      convert = dialcvt;
	    if (convert) {
		if (ddcvt(dialdir[dd-1],f,dd-1) < 0) {
		    oldflg = 1;
		    printf(
"  Sorry, can't convert.");
		    printf(
"  Will ignore speed and parity fields, continuing...\n\n");
		} else {
		    olddir = newdir = 0;
		}
		dd--;
		f = NULL;
		continue;
	    } else {
		if (dialcvt == 2)
		  printf(
"  OK, will ignore speed and parity fields, continuing...\n\n");
		olddir = 1;
	    }
	}

#ifdef LUDEBUG
if (zz) printf("LUDIAL XX s[%s], n1=%d\n",s,n1);
#endif /* LUDEBUG */

	/* Now parse again for real */

	if (oldentry)			/* Parse it the old way */
	  xwords(line,5,info,0);
	else				/* Parse it the new way */
	  xwords(line,2,info,1);

#ifdef LUDEBUG
if (zz) printf("LUDIAL YY s[%s], n1=%d\n",s,n1);
if (zz) printf("%s [%s]\n",info[1],info[2]);
#endif /* LUDEBUG */

	if (info[1]) {			/* First word is entry name */
	    if ((n3 = (int) strlen(info[1])) < 1) /* Its length */
	      continue;			/* If no first word, keep reading. */
	    if (n3 < n1)		/* Search name is longer */
	      continue;			/* Can't possibly match */
	    if (ambiguous && n3 != n1)
	      continue;

#ifdef LUDEBUG
if (zz) printf("MATCHING: [%s] [%s], n1=%d\n",s,info[1],n1);
#endif /* LUDEBUG */

	    if (xxstrcmp(s,info[1],n1)) /* Caseless string comparison */
	      continue;

#ifdef LUDEBUG
if (zz) printf("MATCH OK: [%s] [%s], n1=%d\n",s,info[1],n1);
#endif /* LUDEBUG */

	    if (!info[2])		/* No phone number given */
	      continue;
	    if ((n2 = (int) strlen(info[2])) < 1) /* Length of phone number */
	      continue;			/* Ignore empty phone numbers */

	    /* Got one */

	    if (!(pp = (char *)malloc(n2 + 1))) { /* Allocate storage for it */
		printf("?internal error - ludial malloc 1\n");
		if (line) {
		    free(line);
		    line = NULL;
		}
		dncount = 0;
		return(-1);
	    }
	    strcpy(pp,info[2]);		/* Copy number into malloc'd storage */

	    if (dncount > MAXDNUMS) {
		printf("Warning: %d matches found, %d max\n",
		       dncount,
		       MAXDNUMS
		       );
		dncount = MAXDNUMS;
		break;
	    }
	    dn_p[dncount++] = pp;	/* Add pointer to array. */
	    if (dncount == 1) {		/* First one... */
		if (d_name) free(d_name);
		if (!(d_name = (char *)malloc(n3 + 1))) { /* Save its name */
		    printf("?internal error - ludial malloc 2\n");
		    if (line) {
			free(line);
			line = NULL;
		    }
		    dncount = 0;
		    return(-1);
		}
		t = n3;			/* And its length */
		strcpy(d_name,info[1]);
	    } else {			/* Second or subsequent one */

#ifdef LUDEBUG
		if (zz)
		  printf("d_name=[%s],info[1]=%s,t=[%d]\n",d_name,info[1],t);
#endif /* LUDEBUG */

		if ((int) strlen(info[1]) == t) /* Lengths compare */
		  if (!xxstrcmp(d_name,info[1],t)) /* Caseless compare OK */
		    continue;

		/* Name given by user matches entries with different names */

		if (ambiguous)		/* Been here before */
		  break;

		ambiguous = 1;		/* Now an exact match is required */
		for (j = 0; j < dncount; j++) { /* Clean out previous list */
		    if (dn_p[j]) {
			free(dn_p[j]);
			dn_p[j] = NULL;
		    }
		}
		pass++;			/* Second pass... */
		goto lu_again;		/* Do it all over again. */
	    }
	}
    }
    if (line) free(line);
    if (dncount == 0 && ambiguous) {
	printf(" Lookup: \"%s\" - ambiguous%s\n",
	       s,
	       cx == XXLOOK ? "" : " - dialing skipped"
	       );
	return(-2);
    }
    return(dncount);
}

static char *dscopy = NULL;

int
dodial(cx) int cx; {			/* DIAL or REDIAL */
    int i = 0, x = 0;			/* Workers */
    int sparity = -1;			/* For saving global parity value */
    int partial  = 0;			/* For partial dial */
    int previous = 0;
    int len = 0;
    int literal = 0;

    char *p = NULL, *s3 = NULL, **s2 = NULL;
    int j = 0, t = 0, n = 0;

    if (cx == XXPDIA) {			/* Shortcut... */
	cx = XXDIAL;
	partial = 1;
    }
    previous = dialsta;			/* Status of previous call, if any */

    if (cx != XXLOOK) {
	if (mdmtyp < 1) {
	    if (network)
	      printf("Please SET HOST first, and then SET MODEM\n");
	    else
	      printf("Sorry, you must SET MODEM first\n");
	    dialsta = DIA_NOMO;
	    return(0);
	}
	if (!local) {
	    printf("Sorry, you must SET LINE or SET HOST first\n");
	    dialsta = DIA_NOLI;
	    return(0);
	}
	if (!network &&
	    (speed < 0L)
#ifdef UNIX
	    && (strcmp(ttname,"/dev/null"))
#else
#ifdef OSK
	    && (strcmp(ttname,"/nil"))
#endif /* OSK */
#endif /* UNIX */
	    ) {
	    printf("Sorry, you must SET SPEED first\n");
	    dialsta = DIA_NOSP;
	    return(0);
	}
    }
    s = NULL;				/* Initialize user's dial string */
    if (cx == XXRED) {			/* REDIAL or... */
	if ((y = cmcfm()) < 0)
	  return(y);
    } else if (cx == XXANSW) {		/* ANSWER or ... */
	if ((y = cmnum("timeout (seconds)","0",10,&x,xxstring)) < 0)
	  return(y);
	dialatmo = x;
	if ((y = cmcfm()) < 0)
	  return(y);
    } else {				/* DIAL or LOOKUP */
	if (ndialdir > 0)
	  s3 = "Number to dial or entry from dial directory";
	else
	  s3 = "Number to dial";
	if ((x = cmtxt(s3, dialnum ? dialnum : "",&s,xxstring)) < 0)
	  return(x);
	if (s) {
	    len = (int) strlen(s);
	    if (len > 1) {		/* Strip outer braces if given */
		if (*s == '{') {
		    if (s[len-1] == '}') {
			s[len-1] = NUL;
			s++;
			len -= 2;
		    }
		}
	    }
	}		
    }
    if (cx != XXANSW) {
	for (j = 0; j < MAXDNUMS; j++) { /* Initialize dial-number list */
	    if (!dialnum) {		/* First time dialing */
		dn_p[j] = NULL;		/* initialize all pointers. */
		dn_p2[j] = NULL;
	    } else if (dn_p[j]) {	/* Not the first time, */
		free(dn_p[j]);		/* free previous, if any, */
		dn_p[j] = NULL;		/* then set to NULL. */
		if (dn_p2[j])
		  free(dn_p2[j]);
		dn_p2[j] = NULL;
	    } else break;		/* Already NULL */
	}
	if (len == 0)
	  s = NULL;
	if (!s)
	  s = dialnum;
	if (!s) {
	    if (cx == XXLOOK)
	      printf("?Lookup what?\n");
	    else
	      printf("%s\n", (cx == XXRED) ?
		   "?No DIAL command given yet" :
		   "?You must specify a number to dial"
		   );
	    return(-9);
	}

    /* Now we have the "raw" dial or lookup string and s is not NULL */

	makestr(&dscopy,s);		/* Put it in a safe place */
	s = dscopy;
	n = 0;

	debug(F111,"dodial",s,ndialdir);

	if (isalpha(*s)) {
	    if (ndialdir > 0) {		/* Do we have a dialing directory? */
		n = ludial(s,cx);	/* Look up what the user typed */
		if (n == 0)
		  printf(" Lookup: \"%s\" - not found%s\n",
			 s,
			 cx == XXLOOK ? "" : " - dialing as given\n"
			 );
	    }
	    debug(F101,"dodial",s,n);
	    if (n < 0 && cx != XXLOOK) { /* Error out if they wanted to dial */
		if (n == -1)		/* -2 means ludial already gave msg */
		  printf(" Lookup: fatal error - dialing skipped\n");
		dialsta = DIA_DIR;
		return(-9);
	    }    
	} else {			/* "=" forces no lookup. */
	    n = 0;
	    if (*s == '=') {		/* If number starts with = sign */
		s++;			/* strip it */
		literal = 1;		/* remember this */
		while (*s == SP) s++;	/* and then also any leading spaces */
	    }
	    if (ndialdir > 0)
	      printf(" Lookup: skipped\n");
	}

	/* Save DIAL or successful LOOKUP string for future DIAL or REDIAL */
	/* But don't save pieces of partial dial ... */

	if ((cx == XXDIAL && partial == 0 && previous != DIA_PART) ||
	    (cx == XXLOOK && n > 0)) {
	    makestr(&dialnum,dscopy);
	}
	if (n > 0) {
	    if (!quiet && !backgrd && dialdpy) {
		if (!strcmp(d_name,s))
		  printf(" Lookup: \"%s\" - exact match\n",s);
		else
		  printf(" Lookup: \"%s\" - uniquely matches \"%s\"\n",
			 s,
			 d_name
			 );
	    }
	    if ((cx == XXLOOK) || (n > 1)  && !quiet && !backgrd && dialdpy) {
		printf(" %d telephone number%sfound for \"%s\"%s\n",
		       n,
		       (n == 1) ? " " : "s ",
		       s,
		       (n > 0) ? ":" : "."
		       );
		s3 = getdname();
	    }
	    for (i = 0; i < n; i++) {	/* Convert */
		dn_x[i] = -1;
		if (dncvt(i,cx) < 0) {
		    if (cx != XXLOOK) {
			dialsta = DIA_DIR;
			return(-9);
		    }
		}
	    }
	    if (dialsrt && n > 1) {	/* Sort into optimal order */
		for (i = 0; i < n-1; i++) {
		    for (j = i+1; j < n; j++) {
			if (dn_x[j] < dn_x[i]) {
			    t = dn_x[j];
			    dn_x[j] = dn_x[i];
			    dn_x[i] = t;
			    p = dn_p[j];
			    dn_p[j] = dn_p[i];
			    dn_p[i] = p;
			    p = dn_p2[j];
			    dn_p2[j] = dn_p2[i];
			    dn_p2[i] = p;
			}
		    }
		}
	    }
	    if ((cx == XXLOOK) || (n > 1)  && !quiet && !backgrd && dialdpy) {
		int nn = n;
		if (cx != XXLOOK)
		  if (n > 12) nn = 12;
		for (i = 0; i < nn; i++) {
		    printf("%3d. %-12s  %-20s =>  %-20s  (%d)\n",i+1,
			   s3, dn_p[i],
			   dn_p2[i] ? dn_p2[i] : "(processing failed)",
			   dn_x[i]
			   );
		}
		if (cx != XXLOOK && n != nn)
		  printf("And %d more...\n", n - nn);
	    }
	} else if (n == 0) {		/* Not found in directory */
	    makestr(&(dn_p[0]),literal ? s : dscopy);
	    makestr(&d_name,literal ? s : dscopy);
	    dncount = 1;
	    n = 1;
	    if (dncvt(0,cx) < 0) {	/* In case they typed a */
		dialsta = DIA_DIR;	/* portable-format number ... */
		return(-9);
	    }
	}

#ifdef NETCONN
	/* It's not good that the networks directory depends on NOT-NODIAL.. */
	if (cx == XXLOOK && dscopy) {	/* Networks here too... */
	    extern char *nh_p[], *nh_p2[], *n_name;
	    extern char *nh_px[4][MAXDNUMS+1];
	    n = 0;
	    if (nnetdir > 0) {		/* Do we have a network directory? */
		dirline = 0;
		n = lunet(dscopy);	/* Look up what the user typed */
	    }
	    if (n > -1) {
		int k;
		if (cx == XXLOOK && n == 0)
		  printf(" Lookup: \"%s\" - not found\n",dscopy);
		else
		  printf("%s %d network entr%s found for \"%s\"%s\n",
			 cx == XXLOOK ? " Lookup:" : "",
			 n,
			 (n == 1) ? "y" : "ies",
			 dscopy,
			 (n > 0) ? ":" : "."
			 );

		for (i = 0; i < n; i++) {

		    printf("%3d. %-12s => %-9s %s",
			   i+1,n_name,nh_p2[i],nh_p[i]);
		    for (k = 0; k < 4; k++) {
			if (nh_px[k][i])
			  printf(" %s",nh_px[k][i]);
			else
			  break;
		    }
		    printf("\n");
		}
	    }
	}
#endif /* NETCONN */
	if (cx == XXLOOK)
	  return(success = 1);
    } /* cx != XXANSW */

#ifdef VMS
    conres();			/* So Ctrl-C/Y will work */
#endif /* VMS */
/*
  Some modems do not react well to parity.  Also, if we are dialing through a
  TCP/IP TELNET modem server, parity can be fatally misinterpreted as TELNET
  negotiations.

  This should work even if the user interrupts the DIAL command, because the
  DIAL module has its own interrupt handler.  BUT... if, for some reason, a
  dialing device actually *requires* parity (e.g. CCITT V.25bis says that even
  parity should be used), this might prevent successful dialing.  For that
  reason, we don't do this for V.25bis modems.
*/
    sparity = parity;			/* Save current parity */
    if (dialcapas & CKD_V25 == 0)	/* If not V.25bis...  */
      parity = 0;			/* Set parity to NONE */

    if (cx == XXANSW) {			/* ANSWER */
	success = ckdial("",0,0,1);
	goto dialfin;
    }

/* Edit 192 adds the ability to dial repeatedly. */

    i = 0;
    do {
	if (i > 0) printf("\nDial attempt %d of %d...\n", i+1, dialrtr);
	success = 0;
	/* And the ability to dial alternate numbers. */
	/* Loop to dial each in a list of numbers for the same name... */
	for (j = 0; j < n && !success; j++) { /* until one answers. */
	    s = dn_p2[j];		/* Next number in list */
	    if (dn_x[j] >= dialrstr) {	/* Dial restriction */
		printf("Restricted: %s, skipping...\n",dn_p[j]);
		continue;
	    }
	    if (!s) s = dn_p[j];
	    if (i == 0 && dialcnf) {
		printf("Dialing %s\n",s);
		x = getyesno(" Is this number correct? ");
		if (!x) {
		    char **p;
#ifdef CK_RECALL
		    int sv_recall;
		    extern int on_recall;
#endif /* CK_RECALL */
		    cmsavp(psave,PROMPTL);
		    cmsetp(
#ifdef OS2
" Please enter the correct number,\r\n or press Enter to skip: "
#else
" Please enter the correct number,\r\n or press Return to skip: "
#endif /* OS2 */
);
		    cmini(ckxech);
		    x = -1;
		    if (pflag) prompt(NULL);
#ifdef CK_RECALL
		    sv_recall = on_recall;
		    on_recall = 0;
#endif /* CK_RECALL */
		    y = cmdgquo();
		    cmdsquo(0);
		    while (x < 0) {
			x = cmtxt("Corrected phone number","",&s,NULL);
			cmres();
		    }
		    if ((int) strlen(s) < 1) {
			cmsetp(psave);
			continue;
		    }
		    makestr(&(dn_p2[j]), s);
		    cmdsquo(y);
#ifdef CK_RECALL
		    on_recall = sv_recall;
#endif /* CK_RECALL */
		    cmsetp(psave);
		}
	    }
#ifdef COMMENT
/* for testing without dialing ... */
	    success = 0;
#else
	    success = ckdial(s,i,j,partial ? 3 : 0); /* Dial it */
	    if (!success)
	      if (dialsta < 8 ||	/* Break out if unrecoverable error */
		  dialsta == DIA_INTR ||
		  dialsta == DIA_ERR ||
		  previous == DIA_PART
		  )
	      break;
#endif /* COMMENT */
	}
	if (success)			/* Succeeded, leave the outer loop */
	  break;
	if (dialsta < 8 ||		/* Break out if unrecoverable error */
	    dialsta == DIA_INTR ||
	    dialsta == DIA_ERR ||
	    previous == DIA_PART)
	  break;
	if (++i >= dialrtr)		/* Break out if too many tries */
	  break;
	if (!backgrd && !quiet) {
		printf(
"\nWill redial in %d second%s- press any key to redial immediately.\n",
		       dialint,
		       dialint == 1 ? " " : "s "
		       );
		printf("Ctrl-C to cancel...\n");
	    }
	    x = dialint;		/* Redial interval */
	    while (x-- > 0) {
		if (y = conchk())	/* Did they type something? */
		  break;		/* If so, wake up */
		sleep(1);		/* No interrupt, sleep a sec */
	    }
    } while (!success);

  dialfin:

    if (cx != XXLOOK) {
	bleep(success ? BP_NOTE : BP_FAIL);
#ifdef OS2
	setint();			/* Fix OS/2 interrupts */
#endif /* OS2 */
	if (sparity > -1)
	  parity = sparity;		/* Restore parity if we saved it */
#ifdef OS2
	ttres();			/* Restore DIAL device */
#endif /* OS2 */
#ifdef VMS
	concb((char)escape);		/* Restore console */
#endif /* VMS */
#ifdef OS2
	{				/* Set session title */
	    char * p, name[72];		/* in window list. */
	    char * q;
	    if (cx == XXANSW) {
		q = "Incoming call";
	    } else {
		if (d_name)
		  q = d_name;
		else if (dialnum)
		  q = dialnum;
		else if (ttname[0])
		  q = ttname;
		else q = "";
	    }
	    p = name;
	    if (success) {
		strncpy(name,q,48);
		while (*p) {		/* Uppercase it for emphasis. */
		    if (islower(*p))
		      *p = toupper(*p);
		    p++;
		}
	    } else 
	      name[0] = '\0' ;
	    os2settitle((char *) name, TRUE);
	}
#endif /* OS2 */
    }
    return(success);
}
#endif /* NODIAL */

/*  D O T Y P E  --  Type a file  */

int
dotype(file) char * file; {		/* Do the TYPE command */
#ifdef VMS
    char command[512];
    sprintf(command,"type %s",file);	/* Construct TYPE command */
    conres();				/* Let user interrupt */
    success = zshcmd((char *)command);	/* Execute it */
    concb((char)escape);		/* Back to console CBREAK mode */
    return(success);
#else
    char * p, name[257]; 
#ifdef MAC
    int count = 100;
#endif /* MAC */
    int rc = 1;
    int c;
    int save;

    save = binary;			/* Save file type */

#ifdef OS2
    if (*file) {
        strcpy( name, file );		/* Change / to \. */
        p = name;
        while (*p) {
            if (*p == '/') *p = '\\';
            p++;
        }
    } else
      return(0);
#else
    strcpy(name, file);
#endif /* OS2 */

    if (zchki(name) == -2)		/* It's a directory */
      return(0);

    binary = 0;				/* Set file type to text for zopeni */
    if (!zopeni(ZIFILE, name)) {	/* Not a directory, open it */
	binary = save;			/* Failed, restore file type */
	return(0);			/* and return */
    }
    while ((c = zminchar()) != -1) {	/* Loop for all characters in file */
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
		rc = 0;
		break;
	    }
	}
#else /* Not MAC */
	if (putchar(c) == EOF) {	/* Echo character on screen */
	    rc = 0;
	    break;
	}
#endif /* MAC */
    }
    zclose(ZIFILE);			/* Done, close the file */
    binary = save;
    return(rc);
#endif /* VMS */
}

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
#ifdef ONETERMUPD
    {
	char name[257], *p = NULL;
	char * mstr = NULL, * dstr = NULL;
	long len, ndirs, nfiles, nbytes;
	short month, date, year, hour, minute, seconds;

	/* Note: cmifi2() parses a filespec OR a directory name */

	x = cmifi2("Device, directory, and/or file specification",
		   "*",
		   &s,
		   &y,
		   1,
		   NULL,
		   xxstring
		   );
	if (x == -4 || x == -1)
	  return(x);
	if (x == -2) {
#ifdef OS2
	    if (!ckindex(".",s,1,0,0) && !ckindex("*",s,1,0,0)) {
		if (s[1] == ':' && s[2] == NUL)	/* e.g. "dir a:" */
		  sprintf(line,"%s*.*",s);
		else
		  sprintf(line,"%s.*",s);
		s = line;
		if (zxpand(s) < 1) {
		    printf("%s - not found\n",s);
		    return(-9);
		}
	    } else {
#endif /* OS2 */
		printf("%s - not found\n",s);
		return(-9);
#ifdef OS2
	    }
#endif /* OS2 */
	}
	if ((y = cmcfm()) < 0)
	  return(y);

	/* Lower-level functions change / to \. */
	p = s;
	while (*p) {			/* Change them back to \ */
	    if (*p == '/') *p = '\\';
	    p++;
	}
	ndirs = nfiles = nbytes = 0L;
	printf("\nDirectory of %s\n\n",s);
	if (zchki(s) == -2) {
	    /* Found a directory */
#ifdef OS2
	    if (p != s) {
		p--;
		if (*p == '\\' || *p == '/')
		  strcat(s, "*.*");
		else if (*p == ':')
		  strcat(s, ".\\*.*");
		else
		  strcat(s, "\\*.*");
	    } else {
		strcat(s, "*.*");
	    }
#else
	    if (p != s) {
		p--;
		if (*p == '\\' || *p == '/')
		  strcat(s, "*");
		else if (*p == ':')
		  strcat(s, ".");
		else
		  strcat(s, "\\*");
	    } else {
		strcat(s, "*");
	    }
#endif
	}
#ifdef OS2
	else if (!ckindex(".",s,1,0,0) && !ckindex("*",s,1,0,0)) {
	    sprintf(line,"%s.*",s);
	    s = line;
	}
#endif /* OS2 */

	x = zxpand(s);
	while (x-- > 0) {
	    if (!znext(name))
	      break;
	    dstr = zfcdat(name);
	    month = (dstr[4]-48)*10 + (dstr[5]-48);
	    switch(month) {
	      case 1:  mstr = "Jan"; break;
	      case 2:  mstr = "Feb"; break;
	      case 3:  mstr = "Mar"; break;
	      case 4:  mstr = "Apr"; break;
	      case 5:  mstr = "May"; break;
	      case 6:  mstr = "Jun"; break;
	      case 7:  mstr = "Jul"; break;
	      case 8:  mstr = "Aug"; break;
	      case 9:  mstr = "Sep"; break;
	      case 10: mstr = "Oct"; break;
	      case 11: mstr = "Nov"; break;
	      case 12: mstr = "Dec"; break;
	      default: mstr = "   ";
	    }
	    date   = (dstr[6]-48)*10 + (dstr[7]-48);
	    year  = (((dstr[0]-48)*10 
		      + (dstr[1]-48))*10
		     + (dstr[2]-48))*10
		       + (dstr[3]-48);
	    hour  = (dstr[9]-48)*10 + (dstr[10]-48);
	    minute = (dstr[12]-48)*10 + (dstr[13]-48);
	    seconds = (dstr[15]-48)*10 + (dstr[16]-48);
	    len = zchki(name);
	    /* find just the name of the file */
	    for (p = name + (int) strlen(name); 
		 p != name && *p != '/' 
		 && *p != '\\' && *p != ':' ;
		 p--
		 )
	      ;
	    if (*p == '/' || *p == '\\' || *p == ':')
	      p++ ;

	    if (len > -1L) {
		nfiles++;
		nbytes += len;
		printf(" %3s-%02d-%04d  %02d:%02d %10ld %s\n", 
		       mstr, date, year, hour, minute, len, p
		       );
	    } else {
		ndirs++;
		printf(" %3s-%02d-%04d  %02d:%02d %10s %s\n", 
		       mstr, date, year, hour, minute, "<DIR>", p);
	    }
	}
	printf("\n%ld director%s, %ld file%s, %ld byte%s\n\n",
	       ndirs,
	       (ndirs == 1) ? "y" : "ies",
	       nfiles,
	       (nfiles == 1) ? "" : "s",
	       nbytes,
	       (nbytes == 1) ? "" : "s"
	       );
	return(success = 1);
    }
#else /* ONETERMUPD */
    tmpbuf[0] = NUL;
    if ((x = cmifi2(
"Device, directory, and/or file specification,\n\
 or switch(es), or '> file'","*.*",
		    &s,&y,1,NULL,xxstring)) < 0) {
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
	    printf("%s - not found\n",s);
	    return(-9);
	} else
	  return(x);
    }
#endif /* ONETERMUPD */
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
doenable(y,x) int y, x; {
    switch (x) {
      case EN_ALL:
	en_cwd = en_cpy = en_del = en_dir = en_fin = en_get = y;
	en_ren = en_sen = en_set = en_spa = en_typ = en_who = en_ret = y;
        en_mai = en_pri = y;
#ifndef datageneral
        en_bye = y;
#endif /* datageneral */
#ifndef NOPUSH
	if (!nopush)
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
    case EN_CPY:
        en_cpy = y;
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
	if (!nopush)
         en_hos = y;
	break;
#endif /* NOPUSH */
    case EN_REN:
        en_ren = y;
        break;
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
      case EN_RET:
	en_ret = y;
	break;
      case EN_MAI:
	en_mai = y;
	break;
      case EN_PRI:
	en_pri = y;
	break;
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
    debug(F110,"xxdel tmpbuf",tmpbuf,0);
    strcpy(line,tmpbuf);		/* Now copy it back */
#else /* OS2 */
   debug(F110,"xxdel tmpbuf",tmpbuf,0);
   sprintf(line,"%s %s",DELCMD,tmpbuf); /* Construct the system command. */
#endif /* OS2 */
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
#ifdef OS2
    {
        int filespace = 0;
        int len = 0;
        int count = 0;

        s = line;
        z = zxpand(line);
        if (z > 0) {
            int i;
            success = 1;

            if ( msgflg )
              printf("\n");

            for ( i = 0; i < z; i++) {
		znext(tmpbuf);
		len = zchki(tmpbuf);
		if (len >= 0) { 
		    zdelet(tmpbuf);
		    if (zchki(tmpbuf) < 0) {
			filespace += len;
			count++;
			if (msgflg)
			  printf(" %s - deleted\n",tmpbuf);
		    } else {
			success = 0;
			if (msgflg)
			  printf(" %s - not deleted\n",tmpbuf);
		    }
		}
            }
	    if (msgflg)
              printf("\n%d files deleted, %d bytes freed\n",count,filespace);
        } else {
            if (msgflg)
	      printf("?Can not delete file: %s\n", line );
        }
    }
#else /* OS2 */
    s = tmpbuf;
    xsystem(line);			/* Let the system do it. */
    zl = zchki(tmpbuf);
    success = (zl == -1L);
#endif /* OS2 */
#endif /* MAC */
#ifndef OS2 
    if (msgflg)
      printf("%s - %sdeleted\n",s, success ? "" : "not ");
#ifdef VMS
    concb((char)escape);
#endif /* VMS */
#endif /* OS2 */
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
doswitch() {
    char *lp, *ap;			/* macro argument pointer */

    /* Get variable name */
    if ((y = cmfld("Variable name","",&s,xxstring)) < 0)
      return(y);
    if (*s == CMDQ) {
	if (chkvar(s) < 1) {
	    printf("?Variable name required\n");
	    return(-9);
	}
    }
    lp = line;
    strcpy(lp,"_switx ");		/* _switx + space */
    lp += (int)strlen(line);
    ap = lp;
    debug(F110,"SWITCH",atmbuf,0);
    strcpy(lp,atmbuf);			/* + variable name */
    lp += (int)strlen(atmbuf);
    strcat(lp," ");			/* + space */
    lp++;
    debug(F110,"SWITCH 2",line,0);

    /* Get body */

    if ((y = cmtxt("series of cases","",&s,NULL)) < 0) return(y);
    if ((int)strlen(s) < 1) return(-2);
    
    if (litcmd(&s,&lp) < 0) {
	printf("?Unbalanced brackets\n");
	return(0);
    }
    debug(F110,"SWITCH 3",line,0);

    x = mlook(mactab,"_switx",nmac);	/* Look up SWITCH macro definition */
    if (x < 0) {			/* Not there? */
	addmmac("_switx",sw_def);	/* Put it back. */
	if ((x = mlook(mactab,"_switx",nmac)) < 0) { /* Look it up again. */
	    printf("?SWITCH macro definition gone!\n"); /* Shouldn't happen. */
	    return(success = 0);
	}
    }
    debug(F110,"SWITCH command",line,0); /* Execute the SWITCH macro. */
    return(success = dodo(x,ap,cmdstk[cmdlvl].ccflgs));
}

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
    debug(F101,"dofor fx","",fx);
    s = atmbuf;				/* Copy the atom buffer */

    if ((int)strlen(s) < 1) goto badfor;
/*
  In edit 192, we change the loop variables to be evaluated at loop entry,
  not each time through the loop.  This was required in order to allow
  \v(argc) to be used as a loop variable, or in a loop-variable expression.
  Thus, we can't have FOR loops that modify their own exit conditions by
  changing the final value or the increment.  The problem with \v(argc) was
  that it is on the macro stack; after entry into the _forx macro, it is at
  the wrong 
*/
	sprintf(tmpbuf,"%d",fx);	/* Substitute actual value */
	s = tmpbuf;

    while (*lp++ = *s++) ;		/* (what they actually typed) */
    lp--; *lp++ = SP;

    if ((y = cmnum("final value","",10,&fy,xxstring)) < 0) {
	if (y == -3) return(-2);
	else return(y);
    }
    debug(F101,"dofor fy","",fy);
    s = atmbuf;				/* Same deal */
    if ((int)strlen(s) < 1) goto badfor;

	sprintf(tmpbuf,"%d",fy);
	s = tmpbuf;

    while (*lp++ = *s++) ;
    lp--;
    *lp++ = SP;

    if ((y = cmnum("increment","1",10,&fz,xxstring)) < 0) {
	if (y == -3) return(-2);
	else return(y);
    }
    debug(F101,"dofor fz","",fz);
    s = atmbuf;				/* Same deal */
    if ((int)strlen(s) < 1) goto badfor;

	sprintf(tmpbuf,"%d",fz);
	s = tmpbuf;

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
#ifdef COMMENT
/* Too strict */
    if (fz == 0) {
	printf("?Zero increment not allowed\n");
	return(0);
    }
#endif /* COMMENT */
    x = mlook(mactab,"_forx",nmac);	/* Look up FOR macro definition */
    if (x < 0) {			/* Not there? */
	addmmac("_forx",for_def);	/* Put it back. */
	if ((x = mlook(mactab,"_forx",nmac)) < 0) { /* Look it up again. */
	    printf("?FOR macro definition gone!\n"); /* Shouldn't happen. */
	    return(success = 0);
	}
    }
    debug(F110,"FOR command",line,0);	/* Execute the FOR macro. */
    return(success = dodo(x,ap,cmdstk[cmdlvl].ccflgs));

badfor: printf("?Incomplete FOR command\n");
    return(-2);
}
#endif /* NOSPL */

#ifndef NOFRILLS
/* Do the BUG command */

int
dobug() {
    int n;
#ifdef COMMENT
    printf("\n%s,%s\n Numeric: %ld",versio,ckxsys,vernum);
    if (verwho) printf("-%d",verwho);
#endif /* COMMENT */
    printf(
"\nBefore requesting technical support from Columbia U., please consult:\n\n"
	   );
    n = 6;
#ifdef NT
    printf(" . Your \"Kermit 95\" user manual.\n");
    printf(" . The technical reference manual, \"Using C-Kermit\".\n");
    printf(" . The READ.ME file in Kermit 95's directory on your disk.\n");
    n += 3;
#else
    printf(" . The book \"Using C-Kermit\".\n");
    n += 1;    
#ifndef OS2
    printf(" . The CKCKER.UPD and CKCKER.BWR files.\n");
    n += 1;    
#endif /* OS2 */
#ifdef UNIX
    printf(" . The CKUKER.BWR and CKUINS.DOC files.\n");
    n += 1;    
#else
#ifdef VMS
    printf(" . The CKVKER.BWR and CKVINS.DOC files.\n");
    n += 1;    
#else
#ifdef OS2ONLY
    printf(" . The CKERMIT.INF file (use the UPDATES command).\n");
    n += 1;    
#else
#ifdef datageneral
    printf(" . The CKDKER.BWR file\n");
    n += 1;    
#else
#ifdef STRATUS
    printf(" . The CKLKER.BWR file\n");
    n += 1;    
#else
#ifdef AMIGA
    printf(" . The CKIKER.BWR file\n");
    n += 1;
#else
#ifdef GEMDOS
    printf(" . The CKSKER.BWR file\n");
    n += 1;
#else
#ifdef MAC
    printf(" . The CKMKER.BWR file\n");
    n += 1;
#else
#ifdef OSK
    printf(" . The CK9KER.BWR file\n");
    n += 1;
#else
    printf(" . The appropriate system-dependent CK?KER.BWR file\n");
    n += 1;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif /* NT */

    printf(" . Your own organization's support staff, if any.\n");
    printf(
" . The comp.protocols.kermit.* newsgroups if you have Netnews access.\n");
    printf(
" . Our FAQ, \
http://www.columbia.edu/kermit/faq.html, if you have Web access.\n");
    n += 2;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("\n\
If you still need help or have a bug to report after consulting these sources,"
	   );
    printf("\nsend e-mail to:\n\n");
    n += 2;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("  kermit-support@columbia.edu\n\n");
    n += 1;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("Or contact us by post:\n\n");
    printf(
"  Kermit, Columbia University, 612 W 115 Street, New York NY  10025, USA\n\n"
	   );
    n += 1;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("Or by fax at +1 (212) 663-8202.\n\n");
    n += 1;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("Telephone support is available too:\n\n");
    n += 1;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("  +1 (900) 555-5595, USA only, $2.50 per minute\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf("  +1 (212) 854-5126, from anywhere, $25.00 per call, MC/Visa\n\n");
    n += 1;
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
#ifndef NOSHOW
#ifndef NOFRILLS
    printf(
"Before reporting problems, please use the SHOW VERSION and SHOW FEATURES\n");
    if (++n > cmd_rows - 3) if (!askmore()) return(0); else n = 0;
    printf(
"commands to get detailed program version and configuration information.\n\n");
#endif /* NOFRILLS */
#endif /* NOSHOW */
    return(1);
}
#endif /* NOFRILLS */

#ifndef NOSPL

/*  T O D 2 S E C  --  Convert time of day as hh:mm:ss to secs since midnite */
/*
  Call with a string hh:mm or hh:mm:ss.
  Returns a 0 to 86400 on success, or a negative number on failure.
*/
long
tod2sec(t) char * t; {
    long t2;
    long hh = 0L, mm = 0L, ss = 0L;

    if (!t) t = "";
    if (!*t)
      return(-3);
    debug(F110,"tod2sec",t,0);

    if (isdigit(*t))			/* Get hours from argument */
      hh = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      hh = hh * 10 + *t++ - '0';
    if (hh > 24L)
      return(-1L);
    if (*t == ':')
      t++;
    else if (!*t)
      goto xtod2sec;
    else
      return(-1L);
        	
    if (isdigit(*t))			/* Minutes */
      mm = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      mm = mm * 10 + *t++ - '0';
    if (mm > 60L)
      return(-1L);
    if (*t == ':')
      t++;
    else if (!*t)
      goto xtod2sec;
    else
      return(-1L);

    if (isdigit(*t))			/* Seconds */
      ss = *t++ - '0';
    else
      return(-1L);
    if (isdigit(*t))
      ss = ss * 10 + *t++ - '0';
    if (ss > 60L)
      return(-1L);

    if (*t > 32)			/* No trailing junk allowed */
      return(-1L);

  xtod2sec:

    t2 = hh * 3600L + mm * 60L + ss;	/* Seconds since midnight from arg */
    debug(F100,"tod2sec t2","",t2);

    return(t2);
}

int
dopaus(cx) int cx; {
    long zz;

    zz = -1L;
    x_ifnum = 1;			/* Turn off internal complaints */
    if (cx == XXWAI)
      y = cmnum("seconds to wait, or time of day hh:mm:ss","1",10,&x,xxstring);
    else if (cx == XXPAU)
      y = cmnum("seconds to pause, or time of day hh:mm:ss",
		"1",10,&x,xxstring);
    else
      y = cmnum("milliseconds to sleep, or time of day hh:mm:ss",
		"100",10,&x,xxstring);
    x_ifnum = 0;
    if (y < 0) {
	if (y == -2) {			/* Invalid number or expression */
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
		if (zz < tnow)		/* User's time before now */
		  zz += 86400L;		/* So make it tomorrow */
		zz -= tnow;		/* Seconds from now. */
	    }
	} else
	  return(y);
    }
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

    if (zz > -1L) {			/* Time of day given? */
	x = zz;
	if (zz != (long) x) {
	    printf(
"Sorry, arithmetic overflow - hh:mm:ss not usable on this platform.\n"
		   );
	    return(-9);
	}
    }
    if (cx == XXMSL) {			/* Millisecond sleep */
	msleep(zz < 0 ? x : x * 1000);
	return(success = 1);
    }
    while (x--) {			/* Sleep loop */
	int mdmsig;
	if (y = conchk()) {		/* Did they type something? */
#ifdef COMMENT
	    while (y--) coninc(0);	/* Yes, gobble it up */
#else
	    /* There is a debate over whether PAUSE should absorb    */
	    /* its cancelling character(s).  There are several       */
	    /* reasons why it should gobble at least one character:  */
	    /* (1) MS-DOS Kermit does it                             */
	    /* (2) if not, subsequent PAUSE commands will terminate  */
	    /*     immediately                                       */
	    /* (3) if not, subsequent ASK commands will use it as    */
	    /*     valid input.  If \13, then it will get no input   */
	    /* (4) if not, then the character appears on the command */
	    /*     line after all enclosing macros are complete      */
	    coninc(0);			/* Gobble one up */
#endif /* COMMENT */
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
#ifdef ZCOPY
int
docopy() {
    if ((x = cmifi("File to copy","",&s,&y,xxstring)) < 0) {
	if (x == -3) {
	    printf("?Name of existing file required\n");
	    return(-9);
	} else return(x);
    }
    if (y) {				/* No wildcards allowed */
	printf("\n?Please specify a single file\n");
	return(-9);
    }
    strcpy(line,s);			/* Make a safe copy of source name */
    p = line + (int)strlen(line) + 2;	/* Place for destination name */
    if ((x = cmofi("destination name","",&s,xxstring)) < 0) {
        /* Get destination name */
	if (x == -3) {
	    printf("?Name for destination file required\n");
	    return(-9);
	} else return(x);
    }
    strcpy(p,s);			/* Safe copy of destination name */
    if ((y = cmcfm()) < 0) return(y);
#ifdef VMS
    conres();				/* Let Ctrl-C work. */
#endif /* VMS */
    debug(F110,"docopy line",line,0);
    debug(F110,"docopy p",p,0);
    if (zcopy(line,p) < 0) {
	printf("?Can't copy %s to %s\n",line,p);
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
#endif /* ZCOPY */
#endif /* NOFRILLS */

#ifndef NOFRILLS
#ifdef ZRENAME
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
#endif /* ZRENAME */
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
	if (mrval[maclvl+1]) {		/* Free old one, if any */
	    free(mrval[maclvl+1]);
	    mrval[maclvl+1] = NULL;
	}
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
	if ((z = cmnum("buffer size","4096",10,&y,xxstring)) < 0)
	  return(z);
	if (y < 1) {
	    printf("?Positive number required\n");
	    return(-9);
	}
	if ((z = cmcfm()) < 0) return(z);
        readblock = y;
	if (readbuf)
	  free(readbuf);
	if (!(readbuf = (CHAR *) malloc(readblock+1))) {
	    printf("?Can't allocate read buffer\n");
	    return(-9);
	}
	return(success = zopeni(ZRFILE,line));

#ifndef MAC
#ifndef NOPUSH
      case XYFZ_Y:			/* Pipe/Process (READ) */
	if (nopush) {
	    printf("?Read from pipe disabled\n");
	    return(success=0);
	}
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
	if (!readbuf) {
	    if (!(readbuf = (CHAR *) malloc(readblock+1))) {
		printf("?Can't allocate read buffer\n");
		return(-9);
	    }
	}
	return(success = zxcmd(ZRFILE,line));

      case XYFZ_X:			/* Write to pipe */
	if (nopush) {
	    printf("?Write to pipe disabled\n");
	    return(success=0);
	}
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

/* Finish parsing and do the GET or REGET command */

int
doget(cx) int cx; {
    int x, y, rc;
    char *cbp;

#ifdef CK_TMPDIR
    if (dldir && !f_tmpdir) {	/* If they have a download directory */
	if (s = zgtdir()) {		/* Get current directory, */
	    if (zchdir(line)) {	/* change to download directory */
		strncpy(savdir,s,TMPDIRLEN);
		f_tmpdir = 1;	/* remember that we did this */
	    }
	}
    }
#endif /* CK_TMPDIR */

    cmarg2 = "";			/* Initialize as-name to nothing */
    x = 0;
#ifdef NOFRILLS
    if (*cmarg == NUL) {
	printf("?Remote filespec required\n");
	rc = -3;
	goto endget;
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

	    if (getnct(cmdbuf,CMDBL,tfile[tlevel],0) < 0) {
		printf("Command file ends prematurely in multiline GET\n");
		popclvl();
		rc = -9;
		goto endget;
	    }
	    cmres();			/* Parse it */
	    if ((x = cmtxt("Oofa","",&s,xxstring)) < 0) {
		rc = x;
		goto endget;
	    }
	    strcpy(line,brstrip(s));	/* Make a safe copy */
	    cmarg = line;		/* Point to remote filename */
	    if (*cmarg == NUL) {	/* Make sure there is one */
		printf("Remote filename missing in multiline GET\n");
		rc = -9;
		goto endget;
	    }
	    lp = line + strlen(line) + 1; /* Place for as-name */

	    /* And third line... */

	    cmarg2 = "";		/* Assume no as-name */
	    if (getnct(cmdbuf,CMDBL,tfile[tlevel],0) < 0) { /* Get next line */
		popclvl();		/* There isn't one. */
	    } else {			/* There is... */
		if (*cmdbuf >= ' ') {	/* Parse as output filename */
		    cmres();
		    if ((x = cmofi("Mupeen",cmarg,&s,xxstring)) < 0) {
			rc = x;
			goto endget;
		    }
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
		rc = -9;
		goto endget;
	    }
	    cmres();
	    if ((x = cmtxt("Oofa","",&s,xxstring)) < 0) return(x);
	    if (*s == NUL) {		/* Make sure we got something */
		printf("Remote filename missing in multiline GET\n");
		rc = -9;
		goto endget;
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
		    if ((x = cmofi("Mupeen","",&s,NULL)) < 0) {
			rc = x;
			goto endget;
		    }
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
		rc = x;
		goto endget;
	    }
	    if (*cmarg == NUL) { 	/* If user types a bare CR, */
		printf("(cancelled)\n"); /* Forget about this. */
	    	cmsetp(psave);		/* Restore old prompt, */
		rc = 0;
		goto endget;
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
		rc = x;
		goto endget;
	    }	    
	    x = -1;			/* Get confirmation. */
	    while (x == -1) x = cmcfm();
	    cmsetp(psave);		/* Restore old prompt. */
        }
    }
#endif /* NOFRILLS */

    if (x == 0) {			/* Good return from cmtxt or cmcfm, */
	debug(F110,"xxget cmarg",cmarg,0);
	strncpy(fspec,cmarg,CKMAXPATH);
	debug(F111,"xxget fspec",fspec,CKMAXPATH);
	if (cx == XXRETR)
	  sstate = (CHAR) 'h';
	else
	  sstate = (CHAR) ((cx == XXGET) ? 'r' : 'j'); /* Set start state. */
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
#ifdef OSK
	(y > 0) && isdir(cmarg2)
#else
#ifdef VMS
	(y > 0) && isdir(cmarg2)
#else
#ifdef STRATUS
	(y > 0) && isdir(cmarg2)
#endif /* STRATUS */
#endif /* VMS */
#endif /* OSK */
#endif /* UNIX */

#endif /* OS2 */
	) {
	debug(F110,"RECEIVE arg disk or dir",cmarg2,0);
	if (!f_tmpdir) {
	    s = zgtdir();
	    if (s) {
		strncpy(savdir,s,TMPDIRLEN); /* remember old disk/dir */
		f_tmpdir = 1;	/* and that we did this */
		cmarg2 = "";	/* and we don't have an as-name. */
	    } else {
		printf("?Can't get current directory\n");
		cmarg2 = "";
		f_tmpdir = 0;
		rc = -9;
		goto endget;
	    }
	    if (!zchdir(cmarg2)) {	/* change to given disk/directory, */
		printf("?Can't access %s\n",cmarg2);
		cmarg2 = "";
		rc = -9;
		goto endget;
	    }
	}
    }
#endif /* CK_TMPDIR */
#endif /* NOFRILLS */

    return(x);

  endget:
#ifdef CK_TMPDIR
    if (f_tmpdir) {
	zchdir(savdir);
	f_tmpdir = 0;
    }
#endif /* CK_TMPDIR */
    return(rc);
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
	c = (char) (i + '0');		/* Make name */
	mbuf[1] = (char) c;		/* Insert digit */
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
    debug(F101,"_get/putarg exit argc maclvl","",macargc[maclvl]);

    if (cx == XXGTA && maclvl > 1) {
	macargc[maclvl] = macargc[maclvl - 2];
	/* macargc[maclvl - 1] = macargc[maclvl - 2]; */
    }

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
    int i, j, x, y, z, bc;
    int stopflg;
    char tmplbl[50], *lp;

    stopflg = (cx == XXXFWD);		/* _FORWARD (used in SWITCH) */
    bc = 0;				/* Brace counter */

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
		if (*lp == '{')		/* But only at this level */
		  bc++;			/* Anything inside braces is off */
		else if (*lp == '}')	/* limits. */
		  bc--;
		if (stopflg && bc > 0)	/* This is good for SWITCH */
		  continue;		/* but interferes with WHILE, etc. */
		if (*lp == ',') {
		    flag = 1;
		    continue;
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
		z = (stopflg && inpcas[cmdlvl]) ?
		  strcmp(s,tmplbl) :
		    xxstrcmp(s,tmplbl,(y > j) ? y : j);
		if (!z)
		  break;
		else if (stopflg &&
		    !xxstrcmp(":default",tmplbl,(8 > j) ? 8 : j))
		  break;
		else
		  flag = 0;
	    }
	    if (i == m) {		/* didn't find the label */
		debug(F101,"goto failed at cmdlvl","",cmdlvl);
		if (stopflg)
		  return(0);
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
	    if (cx == XXGOTO) {		/* If GOTO, but not FORWARD, */
		rewind(tfile[tlevel]);	/* search file from beginning */
		tfline[tlevel] = 0;
	    }
	    while (! feof(tfile[tlevel])) {
		if (fgets(line,LINBUFSIZ,tfile[tlevel]) == NULL) /* Get line */
		  break;		/* If no more, done, label not found */
		tfline[tlevel]++;
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
		} else if (stopflg &&
			   !xxstrcmp(":default",tmplbl,(8 > j) ? 8 : j)) {
		    x = 1;
		    break;
		}
	    }
	    if (x == 0) {		/* If not found, print message */
		debug(F101,"goto failed at cmdlvl","",cmdlvl);
		if (stopflg)
		  return(0);
		if (!popclvl()) {	/* pop up to next higher level */
		    printf("?Label '%s' not found\n",s); /* if none */
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

/*  C H K V A R  --  Check (if it's a) Variable  */
  
/*
  Crude and disgusting, but needed for OS/2, DOS, and Windows, where filenames
  have backslashes in them.  How do we know if a backslash in a filename is a
  directory separator, or if it's a Kermit backslash?  This routine does a
  rough syntax check of the next few characters and if it looks like it MIGHT
  be a variable, then it tries to evaluate it, and if the result is not empty,
  we say it's a variable, although sometimes it might not be -- some cases are
  truly ambiguous.  For example there might a DOS directory called \&a, and
  we also have a variable with the same name.  This is all for the sake of not
  having to tell PC users that they have to double all backslashes in file
  and directory names.

  Call with a string pointer pointing at the backslash of the suspected
  variable.  Returns 1 if it seems to be a variable, 0 if not.
*/
int
chkvar(s) char *s; {
    int z = 0;				/* Return code - assume failure */
    if ((int)strlen(s) < 1) return(-2);
    if (*s == CMDQ) {			/* Object begins with backslash. */
	char c;
	c = s[1];			/* Character following backslash */
	if (c) {
	    int t = 0;
	    c = (char) (islower(c) ? toupper(c) : c);
	    if (c == '%') {		/* Simple variable */
		t = 1;
	    } else if (c == '&') {	/* Array */
		if (s[3] == '[')
		  t = ckindex("]",s,4,0,1);
	    } else if (c == '$' ||	/* Environment variable */
		       c == 'V' ||	/* Built-in variable */
		       c == 'M')	/* Macro name */
	        t = (s[2] == '(');
	    else if (c == 'F') {	/* Function reference */
		int x;
		if (x = ckindex("(",s,3,0,1))
		  if (x = ckindex(")",s,x,0,1))
		    t = 1;
	    }
	    if (t) {
		t = LINBUFSIZ-1;	/* This lets us test \v(xxx) */
		lp = line;		/* and even \f...(xxx) */
		zzstring(s,&lp,&t);	/* Evaluate it, whatever it is. */
		t = strlen(line);	/* Get its length. */
		debug(F111,"chkvar",line,t);
		z = t > 0;		/* If length > 0, it's defined */
	    }
	}
    }
    return(z);
}

/*  D O I F  --  Do the IF command  */

int
doif(cx) int cx; {
    int x, y, z; char *s, *p;
    char *q;
    _PROTOTYP(char * evala, (char *));		
#ifdef OS2
    extern int keymac;
#endif /* OS2 */

    not = 0;				/* Flag for whether "NOT" was seen */
    z = 0;				/* Initial IF condition */
    ifargs = 0;				/* Count of IF condition words */

ifagain:
    if ((ifc = cmkeyx(iftab,nif,"","",xxstring)) < 0) { /* If what?... */
	if (ifc == -3) {
	    printf("?Condition required\n");
	    return(-9);
	} else if (chknum(atmbuf)) {
	    ifc = 9999;
	} else {
	    if (ifc == -9)
	      printf("?No keywords match - \"%s\"\n", atmbuf);
	    return(ifc);
	}
    }
    if (ifc == 9999)
      z = !(atoi(atmbuf) == 0);
    else
    switch (ifc) {			/* set z = 1 for true, 0 for false */
      case XXIFNO:			/* IF NOT */
	not ^= 1;			/* So NOT NOT NOT ... will work */
	ifargs++;
	goto ifagain;
      case XXIFTR:			/* IF TRUE */
	z = 1;
	debug(F101,"if true","",z);
	ifargs += 1;
	break;
      case XXIFNT:			/* IF FALSE */
	z = 0;
	debug(F101,"if true","",z);
	ifargs += 1;
	break;
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
	if (*s == CMDQ)
	  z = chkvar(s);			/* \-thing */
	else
	  z = ( mxlook(mactab,s,nmac) > -1 ); /* Look for exact match */
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
		extern int cmflgs;
		if (cmflgs == 1) {
		    printf("?File or directory name required\n");
		    return(-9);
		}
	    } else return(x);
	}
	if (ifc == XXIFEX) {
	    z = (zchki(s) > -1L);
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
#ifdef VMS
	    z = (zchki(s) == -2)
#else
/* Because this doesn't catch $DISK1:[FOO]BLAH.DIR;1 */
	    z = isdir(s)
#ifdef OS2
	      || (isalpha(cmarg2[0]) &&
		  cmarg2[1] == ':' &&
		  cmarg2[2] == '\0')
#endif /* OS2 */
#endif /* VMS */
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

/* The following bit is for compatibility with old versions of MS-DOS Kermit */

	if (!xxstrcmp(lp,"count",5)) {
	    n1 = count[cmdlvl];
	} else if (!xxstrcmp(lp,"version",7)) {
	    n1 = (int) vernum;
	} else if (!xxstrcmp(lp,"argc",4)) {
	    n1 = (int) macargc[maclvl];
	} else {

/* End of compatibility bit */

	    if (chknum(lp)) {
		n1 = atoi(lp);
	    } else {			/* Check for arithmetic expression */
		q = evala(lp);		/* cmnum() does this but ... */
		if (chknum(q))		/* we're not using cmnum(). */
		  n1 = atoi(q);
		else
		  return(-2);
	    }		
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
	    if (chknum(tp)) {
		n2 = atoi(tp);
	    } else {
		q = evala(tp);
		if (chknum(q))
		  n2 = atoi(q);
		else
		  return(-2);
	    }		
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
	    extern int cmflgs;
	    if (cmflgs == 1) {
		printf("?Quantity required\n");
		return(-9);
	    }
	} else if (x < 0)
	  return(x);
	debug(F111,"xxifnu cmfld",s,x);
	lp = line;
	strcpy(lp,s);
	debug(F110,"xxifnu quantity",lp,0);
        z = chknum(lp);
#ifdef COMMENT
/*
  This works, but it's not wise -- IF NUMERIC is mostly used to see if a
  string really does contain only numeric characters.  If they want to force
  evaluation, they can use \feval().
*/
	if (!z) {			/* Not a number */
	    x_ifnum = 1;		/* Avoid "eval" error messages */
	    q = evala(lp);		/* Maybe it's an expression */
	    z = chknum(q);		/* that evaluates to a number */
	    x_ifnum = 0;		/* Put eval messages back to normal */
	    if (z) debug(F110,"xxifnu exp",lp,0);
	}
#endif /* COMMENT */
        debug(F101,"xxifnu chknum","",z);
	ifargs += 2;
	break;

#ifdef ZFCDAT
      case XXIFNE: {			/* IF NEWER */
	char d1[20], * d2;		/* Buffers for 2 rrrrrrrrrrdates */
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
#ifdef NOLOCAL
	z = 1;
#else
	z = remonly;
#endif /* NOLOCAL */
	break;
#endif /* CK_IFRO */

      case XXIFAL: {			/* ALARM */
	  ifargs++;
	  if (ck_alarm < 1L || alrm_date[0] < '0' || alrm_time[0] < '0') {
	      z = 0;			/* ALARM not SET */
	      break;			/* so IF ALARM fails */
	  }
	  x = 9;			/* Compare current date */
	  s = tmpbuf;			/* and time with alarm time */
	  zzstring("\\v(ndate)",&s,&x);
	  z = (int) strncmp(tmpbuf,alrm_date,8); /* Compare dates */
	  debug(F111,"IF ALARM date",alrm_date,z);
	  if (z >= 0) {
	      x = 6;			/* If dates equal, compare times */
	      s = tmpbuf;
	      zzstring("\\v(ntime)",&s,&x);
	      z = strncmp(tmpbuf,alrm_time,5);
	      debug(F111,"IF ALARM time",alrm_time,z);
	  }	
	  tmpbuf[0] = NUL;		/* z >= 0 if alarm is passed */
	  z = (z>=0 ? 1 : 0);		/* z <  0 otherwise */
	  debug(F101,"IF ALARM","",z);
        }
	break;

#ifdef OS2
      case XXIFSD:			/* Started-From-Dialer */
	ifargs++;
	z = StartedFromDialer;
	break;

      case XXIFTM:			/* Terminal-Macro */
	ifargs++;
	z = cmdstk[cmdlvl].ccflgs & CF_KMAC;
	break;
#endif /* OS2 */

      case XXIFEM:			/* Emulation is active */
#ifdef OS2	
	z = 1;
#else
	z = 0;
#endif /* OS2 */
	break;

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
	      dodo(x,line,cmdstk[cmdlvl].ccflgs); /* Do the XIF macro */
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
	  p = " goto _..bot) } ";
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
		  dodo(x,p,cmdstk[cmdlvl].ccflgs);
		  free(p);
		  p = NULL;
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
	tlevel--;
	return(success = 0);
    } else {
	tfline[tlevel] = 0;		/* Line counter */
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
	if (tfnam[tlevel]) {		/* Copy the filename */
	    free(tfnam[tlevel]);
	    tfnam[tlevel] = NULL;
	}
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
	cmdstk[cmdlvl].ccflgs = cmdstk[cmdlvl-1].ccflgs;
#else
	takerr[tlevel] = takerr[tlevel-1]; /* Inherit this */
#endif /* NOSPL */
    }
    return(1);
}
#endif /* NOICP */
