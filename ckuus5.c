#include "ckcsym.h"

#ifndef NOICP

/*  C K U U S 5 --  "User Interface" for C-Kermit, part 5  */
 
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
#include "ckcnet.h"
#ifndef NOCSETS
#include "ckcxla.h"
#endif /* NOCSETS */
#ifdef MAC
#include "ckmasm.h"
#endif /* MAC */

/* For formatted screens, "more?" prompting, etc. */

#ifdef FT18
#define isxdigit(c) isdigit(c)
#endif /* FT18 */

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

/* External variables */

extern int carrier, cdtimo, local, backgrd, bgset, sosi, suspend,
  displa, binary, deblog, escape, xargs, flow, cmdmsk,
  duplex, ckxech, pktlog, seslog, tralog, what,
  keep, warn, tlevel, cwdf, nfuncs, unkcs, msgflg,
  mdmtyp, zincnt, cmask, rcflag, success, xitsta, pflag, lf_opts, tnlm, tn_nlm,
  xfrcan, xfrchr, xfrnum, xitwarn, debses, pktpaus;

#ifndef NOKVERBS
extern struct keytab kverbs[];		/* Table of \Kverbs */
extern int nkverbs;			/* Number of \Kverbs */
#endif /* NOKVERBS */

#ifdef CK_RECALL
extern int cm_recall;
#endif /* CK_RECALL */

extern char *ccntab[];
#ifdef OS2
extern char *printfile;
_PROTOTYP (int os2getcp, (void) );
_PROTOTYP (int os2getcplist, (int *, int) );
#ifdef OS2MOUSE
extern int tt_mouse ;
#endif /* OS2MOUSE */
#endif /* OS2 */

#ifndef NOFRILLS
extern int en_cwd, en_del, en_dir, en_fin, en_bye,
  en_get, en_hos, en_sen, en_set, en_spa, en_typ, en_who;
#endif /* NOFRILLS */
extern long vernum;
extern int srvtim, srvdis, inecho, insilence, nvars, verwho;
extern char *protv, *fnsv, *cmdv, *userv, *ckxv, *ckzv, *ckzsys, *xlav,
 *cknetv, *clcmds;
#ifdef OS2
extern char *ckonetv;
#ifdef CK_NETBIOS
extern char *ckonbiv;
#endif /* CK_NETBIOS */
#ifdef OS2MOUSE
extern char *ckomouv;
#endif /* OS2MOUSE */
#endif /* OS2 */
extern char *connv, *dialv, *loginv, *nvlook();

#ifndef NOSCRIPT
extern int secho;
#endif /* NOSCRIPT */

#ifndef NODIAL
extern int nmdm;
extern struct keytab mdmtab[];
#endif /* NODIAL */

#ifdef NETCONN
extern int network, ttnproto;
#endif /* NETCONN */

#ifdef OS2
/* SET TERMINAL items... */
extern int tt_type, tt_arrow, tt_keypad, tt_wrap, tt_answer, tt_scrsize;
extern int tt_bell, tt_roll, tt_ctstmo, tt_cursor, tt_hide, tt_pacing;
extern struct tt_info_rec tt_info[];	/* Indexed by terminal type */
extern int max_tt;
#endif /* OS2 */
_PROTOTYP( int ttgwsiz, (void) );

extern int tt_rows, tt_cols;
int cmd_rows = 24, cmd_cols = 80;

#ifdef CK_TMPDIR
extern int f_tmpdir;			/* Directory changed temporarily */
extern char savdir[];			/* Temporary directory */
#endif /* CK_TMPDIR */

extern int tt_crd;

#ifndef NOCSETS
extern int language, nfilc, tcsr, tcsl;
extern struct keytab fcstab[];
extern struct csinfo fcsinfo[];
#ifndef MAC
extern struct keytab ttcstab[];
#endif /* MAC */
#endif /* NOCSETS */

extern int atcapr,
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso; 

#ifdef STRATUS
extern int atfrmi, atfrmo, atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */

extern long speed;

extern char *DIRCMD, *PWDCMD, *DELCMD;
#ifndef NOXMIT
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw;
extern char xmitbuf[];
#endif /* NOXMIT */

extern char **xargv, *versio, *ckxsys, *dftty, *cmarg, *lp;

#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;		/* Command buffers */
#ifndef NOSPL
extern char *savbuf;			/* Command buffers */
#endif /* NOSPL */
#else
extern char cmdbuf[], atmbuf[];		/* Command buffers */
#ifndef NOSPL
extern char savbuf[];			/* Command buffers */
#endif /* NOSPL */
#endif /* DCMDBUF */

extern char toktab[], ttname[], psave[];
extern CHAR sstate;
extern int cmflgs, techo, repars, ncmd;
extern struct keytab cmdtab[];

#ifndef NOSETKEY
KEY *keymap;
MACRO *macrotab;
#endif /* NOSETKEY */

#ifndef NOSPL
extern struct mtab *mactab;
extern struct keytab vartab[], fnctab[];
extern int cmdlvl, maclvl, nmac, mecho;
#endif /* NOSPL */

FILE *tfile[MAXTAKE];			/* TAKE file stack */
char *tfnam[MAXTAKE];

#ifdef DCMDBUF				/* Initialization filespec */
char *kermrc = NULL;
#else
char kermrcb[KERMRCL];
char *kermrc = kermrcb;
#endif /* DCMDBUF */

int noherald = 0;
int cm_retry = 1;			/* Command retry enabled */
xx_strp xxstring = zzstring;

#ifndef NOSPL

extern int query;			/* QUERY active */

/* Local declarations */

static int nulcmd = 0;			/* Flag for next cmd to be ignored */

/* Definitions for predefined macros */

/* First, the single-line macros, installed with addmac()... */

/* IBM-LINEMODE macro */
char *m_ibm = "set parity mark, set dupl half, set handsh xon, set flow none";

/* FATAL macro */
char *m_fat = "if def \\%1 echo \\%1, if not = \\v(local) 0 hangup, stop 1";

/* Now the multiline macros, defined with addmmac()... */

/* FOR macro */
char *for_def[] = { "_assign _for\\v(cmdlevel) { _getargs,",
"define \\\\\\%1 \\%2,:top,if \\%5 \\\\\\%1 \\%3 goto bot,",
"\\%6,:inc,incr \\\\\\%1 \\%4,goto top,:bot,_putargs},",
"def break goto bot, def continue goto inc,",
"do _for\\v(cmdlevel) \\%1 \\%2 \\%3 \\%4 { \\%5 },_assign _for\\v(cmdlevel)",
""};

/* WHILE macro */
char *whil_def[] = { "_assign _whi\\v(cmdlevel) {_getargs,",
":inc,\\%1,\\%2,goto inc,:bot,_putargs},",
"_def break goto bot, _def continue goto inc,",
"do _whi\\v(cmdlevel),_assign _whi\\v(cmdlevel)",
""};

/* XIF macro */
char *xif_def[] = {
"_assign _if\\v(cmdlevel) {_getargs,\\%1,_putargs},",
"do _if\\v(cmdlevel),_assign _if\\v(cmdlevel)",
""};

/*
  Variables declared here for use by other ckuus*.c modules.
  Space is allocated here to save room in ckuusr.c.
*/
#ifdef DCMDBUF
struct cmdptr *cmdstk;
int *ifcmd, *count, *iftest, *intime, *inpcas, *takerr, *merror;
#else
struct cmdptr cmdstk[CMDSTKL];
int ifcmd[CMDSTKL], count[CMDSTKL], iftest[CMDSTKL], intime[CMDSTKL],
  inpcas[CMDSTKL], takerr[CMDSTKL], merror[CMDSTKL];
#endif /* DCMDBUF */

char *m_arg[MACLEVEL][NARGS];
char *g_var[GVARS], *macp[MACLEVEL], *mrval[MACLEVEL];
int macargc[MACLEVEL];
char *macx[MACLEVEL];
extern char varnam[];

char **a_ptr[27];			/* Array pointers, for arrays a-z */
int a_dim[27];				/* Dimensions for each array */

char inpbuf[INPBUFSIZ] = { NUL };	/* Buffer for INPUT and REINPUT */
char inpbufa[2] = { NUL, NUL };		/* Null terminators for INPUT buffer */
char inchar[2] = { NUL, NUL };		/* Last character that was INPUT */
int  incount = 0;			/* INPUT character count */
int pacing = 0;				/* OUTPUT pacing */

char lblbuf[LBLSIZ];			/* Buffer for labels */
#else
int takerr[MAXTAKE];
#endif /* NOSPL */

#ifdef DCMDBUF
char *line;				/* Character buffer for anything */
char *tmpbuf;
#else
char line[LINBUFSIZ];
char tmpbuf[TMPBUFSIZ];			/* Temporary buffer */
#endif /* DCMDBUF */
char *tp;				/* Temporary buffer pointer */

#ifdef CK_APC				/* Application Program Command (APC) */
int apcactive = 0;
int apcstatus = APC_OFF;		/* OFF by default everywhere */
#ifdef DCMDBUF
char *apcbuf;
#else
char apcbuf[APCBUFLEN];
#endif /* DCMDBUF */
#endif /* CK_APC */

#ifdef CK_CURSES
#ifdef DCMDBUF
char *trmbuf;				/* Character buffer for termcap */
#else
char trmbuf[TRMBUFL];			/* (Defined in ckcdeb.h) */
#endif /* DCMDBUF */
#endif /* CK_CURSES */

extern char pktfil[],
#ifdef DEBUG
  debfil[],
#endif /* DEBUG */
#ifdef TLOG
  trafil[],
#endif /* TLOG */
  sesfil[];

#ifndef NOFRILLS
extern int rmailf, rprintf;		/* REMOTE MAIL & PRINT items */
extern char optbuf[];
#endif /* NOFRILLS */

char *homdir = "";			/* Pointer to home directory string */

char numbuf[20];			/* Buffer for numeric strings. */
int noinit = 0;				/* Flag for skipping init file */

#ifndef NOSPL
_PROTOTYP( static long expon, (long, long) );
_PROTOTYP( static long gcd, (long, long) );
_PROTOTYP( static long fact, (long) );
int			/* Initialize macro data structures. */
macini() {		/* Allocate mactab and preset the first element. */
    if (!(mactab = (struct mtab *) malloc(sizeof(struct mtab) * MAC_MAX)))
      return(-1);
    mactab[0].kwd = NULL;
    mactab[0].mval = NULL;
    mactab[0].flgs = 0;
    return(0);
}
#endif /* NOSPL */

/*  C M D I N I  --  Initialize the interactive command parser  */
 
VOID
cmdini() {
    int i, x, y, z;
#ifndef NOSPL
/*
  On stack to allow recursion!
*/
    char vnambuf[VNAML];		/* Buffer for variable names */
#endif /* NOSPL */

#ifndef NOSETKEY			/* Allocate & initialize the keymap */
    if (!(keymap = (KEY *) malloc(sizeof(KEY)*KMSIZE)))
      fatal("cmdini: no memory for keymap");
    if (!(macrotab = (MACRO *) malloc(sizeof(MACRO)*KMSIZE)))
      fatal("cmdini: no memory for macrotab");
    for ( i = 0; i < KMSIZE; i++ ) {
	keymap[i] = i;
	macrotab[i] = NULL;
    }
#ifdef OS2
    keymapinit();
    keynaminit();
#endif /* OS2 */
#endif /* NOSETKEY */

#ifdef DCMDBUF
    if (cmsetup() < 0) fatal("Can't allocate command buffers!");
#ifndef NOSPL
    if (!(cmdstk = (struct cmdptr *) malloc(sizeof(struct cmdptr)*CMDSTKL)))
      fatal("cmdini: no memory for cmdstk");
    if (!(ifcmd = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for ifcmd");
    if (!(count = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for count");
    if (!(iftest = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for iftest");
    if (!(intime = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for intime");
    if (!(inpcas = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for inpcas");
    if (!(takerr = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for takerr");
    if (!(merror = (int *) malloc(sizeof(int)*CMDSTKL)))
      fatal("cmdini: no memory for merror");
    if (!kermrc)
      if (!(kermrc = (char *) malloc(KERMRCL+1)))
	fatal("cmdini: no memory for kermrc");
#endif /* NOSPL */
#ifdef CK_CURSES
/*
  Termcap buffer for fullscreen display, UNIX only.  VMS and OS/2 do it
  another way.  The fullscreen display is not supported on AOS/VS or OS-9,
  etc, yet, and the Mac has its own built-in fullscreen display.
*/
#ifdef UNIX
    if (!(trmbuf = malloc(TRMBUFL+1)))
	fatal("cmdini: no memory for termcap buffer");
#endif /* UNIX */
#endif /* CK_CURSES */

    if (!(line = malloc(LINBUFSIZ)))
	fatal("cmdini: no memory for line");
    if (!(tmpbuf = malloc(LINBUFSIZ)))
	fatal("cmdini: no memory for tmpbuf");

#ifdef CK_APC
    if (!(apcbuf = malloc(APCBUFLEN)))
	fatal("cmdini: no memory for apcbuf");
#endif /* CK_APC */

#endif /* DCMDBUF */

#ifdef CK_MINPUT
    {					/* Initialize MINPUT pointers */
	int i;
	extern char *ms[];
	for (i = 0; i < MINPMAX; i++)
	  ms[i] = NULL;
    }
#endif /* CK_MINPUT */

#ifndef NOSPL
    if (macini() < 0)
      fatal("Can't allocate macro buffers!");

    ifcmd[0] = 0;			/* Command-level related variables. */
    iftest[0] = 0;			/* Initialize variables at top level */
    count[0] = 0;			/* of stack... */
    intime[0] = 0;
    inpcas[0] = 0;
    takerr[0] = 0;
    merror[0] = 0;
#endif /* NOSPL */

#ifdef AMIGA
    if (tlevel < 0)    
      concb(escape);
#endif /* AMIGA */

#ifndef NOSPL
    cmdlvl = 0;				/* Start at command level 0 */
    cmdstk[cmdlvl].src = CMD_KB;
    cmdstk[cmdlvl].lvl = 0;
#endif /* NOSPL */

    tlevel = -1;			/* Take file level = keyboard */
    for (i = 0; i < MAXTAKE; i++)	/* Initialize command file names */
      tfnam[i] = NULL;

#ifdef MAC 
    cmsetp("Mac-Kermit>");		/* Set default prompt */
#else
    cmsetp("C-Kermit>");		/* Set default prompt */
#endif /* MAC */

#ifndef NOSPL
    initmac();				/* Initialize macro table */
/* Add one-line macros */
    addmac("ibm-linemode",m_ibm);	/* Add built-in macros. */
    addmac("fatal",m_fat);		/* FATAL macro. */
/* Add multiline macros */
    addmmac("_forx",for_def);		/* FOR macro. */
    addmmac("_xif",xif_def);		/* XIF macro. */
    addmmac("_while",whil_def);		/* WHILE macro. */
/* Fill in command line argument vector */
    sprintf(vnambuf,"\\&@[%d]",xargs); 	/* Command line argument vector */
    y = arraynam(vnambuf,&x,&z);	/* goes in array \&@[] */
    if (y > -1) {
	dclarray((char)x,z);		/* Declare the array */
	for (i = 0; i < xargs; i++) {	/* Fill it */
	    sprintf(vnambuf,"\\&@[%d]",i);
	    addmac(vnambuf,xargv[i]);
	}
    }
    *vnambuf = NUL;
#endif /* NOSPL */

#ifdef CK_NAWS    
    if (ttgwsiz() > 0) {
	if (tt_rows > 0 && tt_cols > 0) {
	    cmd_rows = tt_rows;
	    cmd_cols = tt_cols;
	}
    }
#endif /* CK_NAWS */

/* Get our home directory now.  This needed in lots of places. */

    homdir = zhome();

#ifdef MAC
    return;				/* Mac Kermit has no init file */

#else /* !MAC */

/* If skipping init file ('-Y' on Kermit command line), return now. */

    if (noinit) return;

#ifdef OS2
/*
  The -y init file must be fully specified or in the current directory.
  KERMRC is looked for via INIT, DPATH and PATH in that order.  Finally, our
  own executable file path is taken and the .EXE suffix is replaced by .INI
  and this is tried as the initialization file.
*/
    if (rcflag) {
	strcpy(line, kermrc);
    } else {
	_searchenv(kermrc,"INIT",line);
	if (line[0] == 0)
	  _searchenv(kermrc,"DPATH",line);
	if (line[0] == 0)
	  _searchenv(kermrc,"PATH",line);
	if (line[0] == 0) {
	    char *pgmptr = GetLoadPath();
	    if (pgmptr) {
	      lp = strrchr(pgmptr, '.');
	      strncpy(line, pgmptr, lp - pgmptr);
	      strcpy(line + (lp - pgmptr), ".ini");
	    }
	}
    }
    if ((tfile[0] = fopen(line,"r")) != NULL) {
        tlevel = 0;
	if (tfnam[tlevel] = malloc(strlen(line)+1))
	  strcpy(tfnam[tlevel],line);
#ifndef NOSPL
	cmdlvl++;
	cmdstk[cmdlvl].src = CMD_TF;
	cmdstk[cmdlvl].lvl = tlevel;
	ifcmd[cmdlvl] = 0;
	iftest[cmdlvl] = 0;
	count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
	intime[cmdlvl] = intime[cmdlvl-1];
	inpcas[cmdlvl] = inpcas[cmdlvl-1];
	takerr[cmdlvl] = takerr[cmdlvl-1];
	merror[cmdlvl] = merror[cmdlvl-1];
#endif /* NOSPL */
        debug(F110,"init file",line,0);
    } else {
        debug(F100,"no init file","",0);
    }
#else /* not OS2 */
    lp = line;
    lp[0] = '\0';
#ifdef GEMDOS
    zkermini(line, rcflag, kermrc);
#else
#ifdef VMS
    zkermini(line,LINBUFSIZ,kermrc);
#else /* not VMS */
    if (rcflag) {			/* If init file name from cmd line */
	strcpy(lp,kermrc);		/* use it, */
    } else {				/* otherwise... */
#ifdef CK_INI_A				/* If we've a system-wide init file */
	/* And it takes precedence over the user's... */
	strncpy(lp,CK_SYSINI,KERMRCL);	/* Use it */
	if (zchki(lp) < 0) {		/* (it it exists...) */
#endif /* CK_INI_A */
	    line[0] = NUL;
	    if (homdir) {		/* Home directory for init file. */
		strcpy(lp,homdir);
#ifdef STRATUS
		strcat(lp,">");		/* VOS separates dirs with >'s */
#else
		if (lp[0] == '/') strcat(lp,"/");
#endif /* STRATUS */
	    }
	    strcat(lp,kermrc);		/* Append the default file name */
#ifdef CK_INI_A
	}
#endif /* CK_INI_A */
    }
#ifdef CK_INI_B				/* System-wide init defined? */
    /* But user's ini file takes precedence */
    if (zchki(lp) < 0)			/* If user doesn't have her own, */
	strncpy(lp,CK_SYSINI,KERMRCL);	/* use system-wide one. */
#endif /* CK_INI_B */

#endif /* VMS */
#endif /* GEMDOS */

#ifdef AMIGA
    reqoff();				/* Disable requestors */
#endif /* AMIGA */

    debug(F110,"ini file is",line,0);
    if ((tfile[0] = fopen(line,"r")) != NULL) {	/* Try to open init file. */
	tlevel = 0;
	if (tfnam[tlevel] = malloc(strlen(line)+1))
	  strcpy(tfnam[tlevel],line);
#ifndef NOSPL
	cmdlvl++;
	ifcmd[cmdlvl] = 0;
	iftest[cmdlvl] = 0;
	count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
	intime[cmdlvl] = intime[cmdlvl-1];
	inpcas[cmdlvl] = inpcas[cmdlvl-1];
	takerr[cmdlvl] = takerr[cmdlvl-1];
	merror[cmdlvl] = merror[cmdlvl-1];
	debug(F101,"open ok","",cmdlvl);
	cmdstk[cmdlvl].src = CMD_TF;
	cmdstk[cmdlvl].lvl = tlevel;
#endif /* NOSPL */
	debug(F110,"init file",line,0);
    }

#ifdef datageneral
/* If CKERMIT.INI not found in home directory, look in searchlist */
    if (homdir && (tlevel < 0)) {
    	strcpy(lp,kermrc);
	if ((tfile[0] = fopen(line,"r")) != NULL) {
	    tlevel = 0;
	if (tfnam[tlevel] = malloc(strlen(line)+1))
	  strcpy(tfnam[tlevel],line);
#ifndef NOSPL
	    cmdlvl++;
	    cmdstk[cmdlvl].src = CMD_TF;
	    cmdstk[cmdlvl].lvl = tlevel;
	    ifcmd[cmdlvl] = 0;
	    iftest[cmdlvl] = 0;
	    count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
	    intime[cmdlvl] = intime[cmdlvl-1];
	    inpcas[cmdlvl] = inpcas[cmdlvl-1];
	    takerr[cmdlvl] = takerr[cmdlvl-1];
	    merror[cmdlvl] = merror[cmdlvl-1];
#endif /* NOSPL */
	}
    }
#endif /* datageneral */

#ifdef AMIGA				/* Amiga... */
    reqpop();				/* Restore requestors */
#endif /* AMIGA */

#endif /* OS2 */


#endif /* MAC */
}

#ifndef NOSPL
/*
  G E T N C M

  Get next command from current macro definition.

  Moved to a separate routine in edit 181 to allow multiline GET
  to work when issued in a macro.

  Command is copied into string pointed to by argument s, max length n.
  Returns:
   0 if a string was copied, -1 if there was no string to copy.
*/
int
getncm(s,n) char *s; int n; {
    int y,				/* Character counter */
#ifdef COMMENT
      quote = 0,
#endif /* COMMENT */
      kp = 0,				/* Brace up-down counter */
      pp = 0;				/* Parenthesis up-down counter */
    char *s2;				/* Copy of destination pointer */
    
    s2 = s;				/* Initialize string pointers */
    *s = NUL;				/* and destination buffer */
    
    debug(F101,"getncm dest length","",n);

    for (y = 0;				/* Loop for n bytes max */
 	 macp[maclvl] && *macp[maclvl] && y < n;
	 y++, s++, macp[maclvl]++) {
	
	*s = *macp[maclvl];		/* Get next char from macro def */
	/* debug(F000,"char","",*s); */
#ifdef COMMENT
/*
  The intention here was to allow quoting of commas, braces, etc,
  in macro definitions, e.g. "define rows mode co80\,\%1".  And it
  works, but it breaks just about everything else.
*/
	if (*s == CMDQ && quote == 0) {	/* Allow for quoting of */
	    quote = 1;			/* braces, commas, etc. */
	    continue;
	}
#endif /* COMMENT */
/*
  Allow braces around macro definition to prevent commas from being turned to
  end-of-lines and also treat any commas within parens as text so that
  multiple-argument functions won't cause the command to break prematurely.
*/
#ifdef COMMENT
	if (!quote) {
#endif /* COMMENT */
	    if (*s == '{') kp++;	/* Count braces */
	    if (*s == '}') kp--;
	    if (*s == '(') pp++;	/* Count parentheses. */
	    if (*s == ')') pp--;
	    if (*s == ',' && pp <= 0 && kp <= 0) {
		macp[maclvl]++;		/* Comma not in {} or () */
		debug(F110,"next cmd",s,0);
		kp = pp = 0;		/* so we have the next command */
		break;
	    }
#ifdef COMMENT
	}
#endif /* COMMENT */
    }					/* Reached end. */
    if (*s2 == NUL) {			/* If nothing was copied, */
	debug(F100,"getncm eom","",0);
	popclvl();			/* pop command level. */
	return(-1);
    } else {				/* otherwise, tack CR onto end */
	*s++ = CR;
	*s = '\0';
	if (mecho && pflag)		/* If MACRO ECHO ON, echo the cmd */
	  printf("%s\n",s2);
	debug(F110,"getncm returns ptr to",s2,0);
    }
    return(0);
}

/*  D O M A C  --  Define and then execute a macro */

int
domac(name, def) char *name, *def; {
    int x, m;
    m = maclvl;				/* Current macro stack level */
    debug(F101,"domac entry maclvl","",maclvl);
    x = addmac(name, def);		/* Define a new macro */
    if (x > -1) {			/* If successful, */
	dodo(x,NULL);			/* start it (increments maclvl). */
	while (maclvl > m) {		/* Keep going till done with it, */
	    debug(F101,"domac loop maclvl 1","",maclvl);
	    sstate = parser(1);		/* parsing & executing each command, */
	    debug(F101,"domac loop maclvl 2","",maclvl);
	    if (sstate) proto();	/* including protocol commands. */
	}
	debug(F101,"domac loop exit maclvl","",maclvl);
    }	    
    return(success);
}
#endif /* NOSPL */

/*
  G E T N C T

  Get next command from current TAKE file.

  Moved to a separate routine in edit 181 to allow multiline GET
  to work when issued in a macro.

  Command is copied into string pointed to by argument s, max length n.
  Returns:
   0 if a string was copied,
  -1 on EOF,
  -2 on malloc failure
  -3 if line not properly terminated
*/
int
getnct(s,n) char *s; int n; {
    int i, j;
    char c, *s2, *lp, *lp2;

    s2 = s;				/* Remember original pointer */

    debug(F101,"getnct","",n);
    if (!(lp2 = (char *) malloc(n+1))) { /* Get a temporary buffer */
	debug(F101,"getnct malloc failure","",0);
	return(-2);
    }
    while (1) {				/* Loop to read lines from file */
    
	if (fgets(lp2,n,tfile[tlevel]) == NULL) { /* EOF */
	    free(lp2);			/* Free temporary storage */
	    *s = NUL;			/* Make destination be empty */
	    return(-1);			/* Return failure code */
	}
	if (techo && pflag)		/* If TAKE ECHO ON, */
	  printf("%s",lp2);		/* echo it. */

	lp = lp2;			/* Make a working pointer */

/* Trim trailing whitespace */

	j = strlen(lp2) - 1;		/* Position of line terminator */
	debug(F111,"Line from TAKE file",lp2,j); /* Got a line */
	if (j < 0) j = 0;
	c = lp2[j];			/* Value of line terminator */
	debug(F101,"getnct nl","",c);
	if (c < LF || c > CR) {		/* It's not a terminator */
	    debug(F111,"getnct bad line",lp2,c);
	    if (feof(tfile[tlevel]) && j > 0 && j < n) {
		printf("Warning: Last line of TAKE file lacks terminator\n");
		c = lp2[++j] = '\n';
	    } else { free(lp2); return(-3); }
	}
	for (i = j - 1; i > -1; i--)	/* Back up over spaces and tabs */
	  if (lp2[i] != SP && lp2[i] != HT && lp2[i] != NUL)
	    break;
	debug(F101,"getnct i","",i);
	lp2[i+1] = c;			/* Move after last nonblank char */
	lp2[i+2] = NUL;			/* Terminate the string */
	debug(F110,"getnct lp2",lp2,0);
	while (*s++ = *lp++) {		/* Copy result to target buffer */
	    if (--n < 2) {
		printf("?Command too long, maximum length: %d.\n",CMDBL);
		free(lp2);
		return(dostop());
	    }

/* Check for trailing comment, " ;" or " #" */

	    if ((s > s2 + 1) &&
		(*(s-1) == ';' || *(s-1) == '#') &&
		(*(s-2) == SP  || *(s-2) == HT)) {
		debug(F100,"Trailing comment","",0);
		s -= 2;			/* Got one, back up buffer pointer */
		n += 2;			/* and adjust free space count. */
		while ((s >= s2)	/* Trim whitespace again. */
		       && (*s == SP || *s == HT))
		  s--, n++;
		s++;			/* Point after last character */
		*s++ = c;		/* Put back line terminator */
		*s++ = NUL;		/* and string terminator */
		n -= 3;			/* Adjust free count */
		debug(F110,"Comment trimmed & terminated",s2,0);
		break;
	    }
	}

/* Check whether this line is continued */

	if (s > s2 + 2) {
	    debug(F000,"Last char in line","",*(s-3));
	    if (*(s - 3) != CMDQ && *(s - 3) != '-') /* Line continued? */
	      break;			/* No, done. */
	    s -= 3;			/* No, back up pointer */
	    debug(F100,"Line is continued","",0); /* and continue */
	} else break;
    }
    untab(s2);				/* Done, convert tabs to spaces */
    free(lp2);				/* Free temporary storage */
    return(0);				/* Return success */
}

/*  P A R S E R  --  Top-level interactive command parser.  */
 
/*
  Call with:
    m = 0 for normal behavior: keep parsing and executing commands
          until an action command is parsed, then return with a
          Kermit start-state as the value of this function.
    m = 1 to parse only one command, can also be used to call parser()
          recursively.
    m = 2 to read but do not execute one command.
  In all cases, parser() returns:
    0     if no Kermit protocol action required
    > 0   with a Kermit protocol start-state.
    < 0   upon error.
*/
int
parser(m) int m; {
    int tfcode, xx, yy, zz;		/* Workers */

#ifndef NOSPL
    int inlevel;			/* Level we were called at */
#endif /* NOSPL */
    char *cbp;				/* Command buffer pointer */
#ifdef MAC
    extern char *lfiles;		/* Fake extern cast */
#endif /* MAC */

#ifdef AMIGA
    reqres();			/* restore AmigaDOS requestors */
#endif /* AMIGA */

    what = W_COMMAND;		/* Now we're parsing commands. */
    if (			/* If at top (interactive) level ... */
#ifndef NOSPL
	cmdlvl == 0
#else
	tlevel < 0
#endif /* NOSPL */
	)
	concb((char)escape);	/* ... put console in 'cbreak' mode. */

#ifdef CK_TMPDIR
/* If we were cd'd temporarily to another device or directory ... */
    if (f_tmpdir) {
	int x;
	x = zchdir((char *) savdir);	/* ... restore previous directory */
	f_tmpdir = 0;			/* and remember we did it. */
	debug(F111,"tmpdir restoring",savdir,x);
    }
#endif /* CK_TMPDIR */

#ifndef NOSPL
    inlevel = cmdlvl;		/* Current macro level */
    debug(F101,"&parser entry maclvl","",maclvl);
    debug(F101,"&parser entry inlevel","",inlevel);
    debug(F101,"&parser entry tlevel","",tlevel);
    debug(F101,"&parser entry cmdlvl","",cmdlvl);
    debug(F101,"&parser entry m","",m);
#endif /* NOSPL */

/*
  sstate becomes nonzero when a command has been parsed that requires some
  action from the protocol module.  Any non-protocol actions, such as local
  directory listing or terminal emulation, are invoked directly from below.
*/
    sstate = 0;				/* Start with no start state. */
#ifndef NOFRILLS
    rmailf = rprintf = 0;		/* MAIL and PRINT modifiers for SEND */
    *optbuf = NUL;			/* MAIL and PRINT options */
#endif /* NOFRILLS */
#ifndef NOSPL
    query = 0;				/* QUERY not active */
#endif /* NOSPL */

    while (sstate == 0) {		/* Parse cmds until action requested */
#ifdef DEBUG
	int nnn;
	debug(F100,"top of parse loop","",0);
	nnn = ttchk();
	debug(F101,"APC ttchk p1","",nnn);
#endif /* DEBUG */

    /* Take requested action if there was an error in the previous command */

#ifndef MAC
	conint(trap,stptrap);		/* In case we were just fg'd */
	bgchk();			/* Check background status */
#endif /* MAC */

	debug(F101,"tlevel","",tlevel);

#ifndef NOSPL
	if (success == 0) {
	    if (cmdstk[cmdlvl].src == CMD_TF && takerr[cmdlvl]) {
		printf("Command file terminated by error.\n");
		popclvl();
		if (cmdlvl == 0) return(0);
	    }
	    if (cmdstk[cmdlvl].src == CMD_MD && merror[cmdlvl]) {
		printf("Command error: macro terminated.\n");
		popclvl();
		if (m && (cmdlvl < inlevel))
		  return((int) sstate);
	    }
	}

	nulcmd = (m == 2);
#else
	if (success == 0 && tlevel > -1 && takerr[tlevel]) {
	    printf("Command file terminated by error.\n");
	    popclvl();
	    cmini(ckxech);		/* Clear the cmd buffer. */
	    if (tlevel < 0) 		/* Just popped out of cmd files? */
	      return(0);		/* End of init file or whatever. */
	}
#endif /* NOSPL */

#ifdef MAC
	/* Check for TAKE initiated by menu. */
	if ((tlevel == -1) && lfiles)
	    startlfile();
#endif /* MAC */

        /* If in TAKE file, check for EOF */
#ifndef NOSPL
#ifdef MAC
	if
#else
	while
#endif /* MAC */
	  ((cmdstk[cmdlvl].src == CMD_TF)  /* If end of take file */
	       && (tlevel > -1)
	       && feof(tfile[tlevel])) {
	    popclvl();			/* pop command level */
	    cmini(ckxech);		/* and clear the cmd buffer. */
	    if (cmdlvl == 0)		/* Just popped out of all cmd files? */
	      return(0);		/* End of init file or whatever. */
 	}
#ifdef MAC
	miniparser(1);
	if (sstate == 'a') {		/* if cmd-. cancel */
	    debug(F100, "parser: cancel take due to sstate", "", sstate);
	    sstate = '\0';
	    dostop();
	    return(0);			/* End of init file or whatever. */
	}
#endif /*  MAC */

#else /* NOSPL */
	if ((tlevel > -1) && feof(tfile[tlevel])) { /* If end of take */
	    popclvl();			/* Pop up one level. */
	    cmini(ckxech);		/* and clear the cmd buffer. */
	    if (tlevel < 0) 		/* Just popped out of cmd files? */
	      return(0);		/* End of init file or whatever. */
 	}
#endif /* NOSPL */

#ifndef NOSPL
        if (cmdstk[cmdlvl].src == CMD_MD) { /* Executing a macro? */
	    debug(F100,"parser macro","",0);
	    maclvl = cmdstk[cmdlvl].lvl; /* Get current level */
	    debug(F101,"parser maclvl","",maclvl);
	    cbp = cmdbuf;		/* Copy next cmd to command buffer. */
	    *cbp = NUL;
	    if (*savbuf) {		/* In case then-part of 'if' command */
		strcpy(cbp,savbuf);	/* was saved, restore it. */
		*savbuf = '\0';
	    } else {			/* Else get next cmd from macro def */
		if (getncm(cbp,CMDBL) < 0) {
		    if (m && (cmdlvl < inlevel))
		      return((int) sstate);
		    else /* if (!m) */ continue;
		}
	    }
	    debug(F110,"cmdbuf from macro",cmdbuf,0);

	} else if (cmdstk[cmdlvl].src == CMD_TF)
#else
	  if (tlevel > -1)  
#endif /* NOSPL */
	  {
#ifndef NOSPL
	    if (*savbuf) {		/* In case THEN-part of IF command */
		strcpy(cmdbuf,savbuf);	/* was saved, restore it. */
		*savbuf = '\0';
	    } else
#endif /* NOSPL */

	      /* Get next line from TAKE file */

	      if ((tfcode = getnct(cmdbuf,CMDBL)) < 0) {
		  if (tfcode < -1) {	/* Error */
		      printf("?Error in TAKE command file: %s\n",
			     (tfcode == -2) ? "Memory allocation failure" :
			     "Line too long or contains NUL characters"
			     );
		      popclvl();
		  }
		  continue;		/* -1 means EOF */
	      }	    

        /* If interactive, get next command from user. */

	} else {			/* User types it in. */
	    if (pflag) prompt(xxstring);
	    cmini(ckxech);
    	}

        /* Now know where next command is coming from. Parse and execute it. */

	repars = 1;			/* 1 = command needs parsing */
	displa = 0;			/* Assume no file transfer display */

	while (repars) {		/* Parse this cmd until entered. */
	    debug(F101,"parser top of while loop","",0);
	    cmres();			/* Reset buffer pointers. */
	    xx = cmkey2(cmdtab,ncmd,"Command","",toktab,xxstring);
	    debug(F101,"top-level cmkey2","",xx);
	    if (xx == -5) {
		yy = chktok(toktab);
		debug(F101,"top-level cmkey token","",yy);
		ungword();
		switch (yy) {
#ifndef NOPUSH
		  case '!': xx = XXSHE; break; /* Shell escape */
#endif /* NOPUSH */
		  case '#': xx = XXCOM; break; /* Comment */
		  case ';': xx = XXCOM; break; /* Comment */
#ifndef NOSPL
		  case ':': xx = XXLBL; break; /* GOTO label */
#endif /* NOSPL */
#ifndef NOPUSH
#ifdef CK_REDIR
                  case '<': xx = XXFUN; break; /* REDIRECT */
#endif /* CK_REDIR */
                  case '@': xx = XXSHE; break; /* Shell (DCL) escape */
#endif /* NOPUSH */
		  default: 
		    printf("\n?Invalid - %s\n",cmdbuf);
		    xx = -2;
		}
	    }

#ifndef NOSPL
            /* Special handling for IF..ELSE */

	    if (ifcmd[cmdlvl])		/* Count stmts after IF */
	      ifcmd[cmdlvl]++;
	    if (ifcmd[cmdlvl] > 2 && xx != XXELS && xx != XXCOM)
	      ifcmd[cmdlvl] = 0;

	    /* Execute the command and take action based on return code. */

	    if (nulcmd) {		/* Ignoring this command? */
		xx = XXCOM;		/* Make this command a comment. */
	    }
#endif /* NOSPL */

	    zz = docmd(xx);	/* Parse rest of command & execute. */
	    debug(F101,"docmd returns","",zz);
	    debug(F110,"cmdbuf",cmdbuf,"");
	    debug(F110,"atmbuf",atmbuf,"");
#ifdef MAC
	    if (tlevel > -1) {
		if (sstate == 'a') {	/* if cmd-. cancel */
		    debug(F110, "parser: cancel take, sstate:", "a", 0);
		    sstate = '\0';
		    dostop();
		    return(0);		/* End of init file or whatever. */
		}
	    }
#endif /* MAC */
	    switch (zz) {
		case -4:		/* EOF (e.g. on redirected stdin) */
		    doexit(GOOD_EXIT,xitsta); /* ...exit successfully */
	        case -1:		/* Reparse needed */
		    repars = 1;		/* Just set reparse flag and  */
		    continue;
		case -6:		/* Invalid command given w/no args */
	    	case -2:		/* Invalid command given w/args */
#ifdef COMMENT
#ifndef NOSPL
		    /* This is going to be really ugly... */
		    yy = mlook(mactab,atmbuf,nmac); /* Look in macro table */
		    if (yy > -1) {	            /* If it's there */
			if (zz == -2) {	            /* insert "do" */
			    char *mp;
			    mp = malloc((int)strlen(cmdbuf) + 5);
			    if (!mp) {
				printf("?malloc error 1\n");
				return(-2);
			    }
			    sprintf(mp,"do %s ",cmdbuf);
			    strcpy(cmdbuf,mp);
			    free(mp);
			} else sprintf(cmdbuf,"do %s %c",atmbuf, CR);
			if (ifcmd[cmdlvl] == 2)	/* This one doesn't count! */
			  ifcmd[cmdlvl]--;
			debug(F111,"stuff cmdbuf",cmdbuf,zz);
			repars = 1;	/* go for reparse */
			continue;
		    } else {
			char *p;
			int n;
			p = cmdbuf;
			lp = line;     
			n = LINBUFSIZ;
			if (cmflgs == 0) printf("\n");
			if (zzstring(p,&lp,&n) > -1) 
			  printf("?Invalid: %s\n",line);
			else
			  printf("?Invalid: %s\n",cmdbuf);
		    } /* (fall thru...) */
#else
		    printf("?Invalid: %s\n",cmdbuf);
#endif /* NOSPL */
#else
		    printf("?Invalid: %s\n",cmdbuf);
#endif /* COMMENT */

		case -9:		/* Bad, error message already done */
		    success = 0;
		    debug(F110,"top-level cmkey failed",cmdbuf,0);
		    /* If in background w/ commands coming stdin, terminate */
		    if (pflag == 0 && tlevel < 0)
		      fatal("Kermit command error in background execution");
/*
  Command retry feature, edit 190.  If we're at interactive prompting level,
  reprompt the user with as much of the command as didn't fail.
*/
#ifdef CM_RETRY
		    if (cm_retry &&	/* If command retry enabled... */
#ifndef NOSPL
			(cmdlvl == 0)	/* and at top (interactive) level... */
			
#else
			(tlevel < 0)
#endif /* NOSPL */
			) {
			int len;
			char *p, *s;
			len = strlen(cmdbuf); /* Length of command buffer */
			p = malloc(len + 1);  /* Allocate space for copy */
			if (p) {	      /* If we got the space */
			    strcpy(p,cmdbuf); /* copy the command buffer. */
			    /* Chop off final field, the one that failed. */
			    s = p + len - 1;          /* Point to end */
			    while (*s == SP && s > p) /* Trim blanks */
			      s--;
			    while (*s != SP && s > p) /* Trim last field */
			      s--;
			    if (s > p)        /* Keep the space */
			      s++;	      /* after last good field */
			    if (s >= p)	      /* Cut off remainder */
			      *s = NUL;
			    cmini(ckxech);    /* Reinitialize the parser */
			    strcpy(cmdbuf,p); /* Copy truncated cmd back */
			    free(p);          /* Free temporary storage */
			    prompt(xxstring); /* Reprint the prompt */
			    printf("%s",cmdbuf); /* Reprint partial command */
			    repars = 1;          /* Force reparse */
			    continue;
			}
		    } else
#endif /* CM_RETRY */
		      cmini(ckxech);	/* (fall thru) */

 	    	case -3:		/* Empty command OK at top level */
		    repars = 0;		/* Don't need to reparse. */
		    continue;		/* Go back and get another command. */

		default:		/* Command was successful. */
#ifndef NOSPL
		    debug(F101,"parser preparing to continue","",maclvl);
#endif /* NOSPL */
		    repars = 0;		/* Don't need to reparse. */
		    continue;		/* Go back and get another command. */
		}
	}
#ifndef NOSPL
	debug(F101,"parser breaks out of while loop","",maclvl);
	if (m && (cmdlvl < inlevel))  return((int) sstate);
#endif /* NOSPL */
    }

/* Got an action command, return start state. */
 
    return((int) sstate);
}

#ifndef NOSPL
/*
  OUTPUT command.
  Buffering and pacing added by L.I. Kirby, 5A(189), June 1993.
*/
#define OBSIZE 80			/* Size of local character buffer */

static int obn;				/* Buffer offset (high water mark) */
static char obuf[OBSIZE];		/* OUTPUT buffer. */
static char *obp;			/* Pointer to output buffer. */
_PROTOTYP( static int oboc, (char) );
_PROTOTYP( static int xxout, (char *, int) );

static int
#ifdef CK_ANSIC
xxout(char *obuf, int obsize)
#else
xxout(obuf, obsize) char *obuf; int obsize; 
#endif /* CK_ANSIC */
/* xxout */ {				/* OUTPUT command's output function */
    int i, rc;

    debug(F101,"xxout obsize","",obsize);
    debug(F101,"xxout pacing","",pacing);
    debug(F111,"xxout string",obuf,strlen(obuf));

    rc = 0;				/* Initial return code. */
    if (!obuf || (obsize <= 0))		/* Nothing to output. */
      goto xxout_x;			/* Return successfully */

    rc = -1;				  /* Now assume failure */
    if (pacing == 0) {			  /* Is pacing enabled? */
	if ((local ?		 	  /* No, write entire string at once */
	     ttol((CHAR *)obuf, obsize) : /* to communications device */
	     conxo(obsize, obuf))	  /* or to console */
	    != obsize)
	  goto xxout_x;
    } else {
	for (i = 0; i < obsize; i++) {	/* Write individual chars */
	    if ((local ? ttoc(obuf[i]) : conoc(obuf[i])) < 0)
	      goto xxout_x;
	    msleep(pacing);
	}
    }
    rc = 0;				/* Success */
  xxout_x:
    obn = 0;				/* Reset count */
    obp = obuf;				/* and pointers */
    return(rc);				/* return our return code */
}

#ifdef COMMENT
/*
  Macros for OUTPUT command execution, to make it go faster. 
*/
#define obfls() ((xxout(obuf,obn)<0)?-1:0)
#define oboc(c) ((*obp++=(char)(c)),*obp=0,(((++obn)>=OBSIZE)?obfls():0))

#else /* The macros cause some compilers to generate bad code. */

static int
#ifdef CK_ANSIC
oboc(char c)
#else
oboc(c) char c;
#endif /* CK_ANSIC */
/* oboc */ {				/* OUTPUT command's output function */

    *obp++ = c;				/* Deposit character */
    *obp = NUL;				/* Flush buffer if it's now full */

    return(((++obn) >= OBSIZE) ? xxout(obuf,obn) : 0);
}
#endif /* COMMENT */

/*  D O O U T P U T  --  Returns 0 on failure, 1 on success */

int
dooutput(s) char *s; {

    int x, xx, y, quote;		/* Workers */

    debug(F111,"dooutput s",s,(int)strlen(s));

#ifndef NOLOCAL
    if (local) {			/* Condition external line */
	y = ttvt(speed,flow);
	if (y < 0) return(0);
    }
#endif /* NOLOCAL */
    quote = 0;				/* Initialize backslash (\) quote */
    obn = 0;				/* Reset count */
    obp = obuf;				/* and pointers */

    while (x = *s++) {			/* Loop through the string */
	y = 0;				/* Error code, 0 = no error. */
	debug(F000,"dooutput","",x);
	if (x == CMDQ) {		/* Look for \b or \B in string */
            quote++;			/* Got \ */
	    continue;			/* Get next character */
	} else if (quote) {		/* This character is quoted */
	    if (quote == 1 && (x == 'n' || x == 'N')) { /* \n or \N */
		if (xxout(obuf,obn) < 0) /* Flush buffer */
		  goto outerr;
		debug(F100,"OUTPUT NUL","",0); /* Send a NUL */
		ttoc(NUL);
		quote = 0;
		continue;
	    } else if (quote == 1 && (x == 'b' || x == 'B')) { /* \b or \B */

		if (xxout(obuf,obn) < 0) /* Flush buffer first */
		  goto outerr;
		debug(F100,"OUTPUT BREAK","",0);
#ifndef NOLOCAL
		ttsndb();		/* Send BREAK signal */
#else
		ttoc(NUL);
#endif /* NOLOCAL */
		quote = 0;		/* Turn off quote flag */
		continue;		/* and not the b or B */
#ifdef CK_LBRK
	    } else if (quote == 1 && (x == 'l' || x == 'L')) { /* \l or \L */
		if (xxout(obuf,obn) < 0) /* Flush buffer first */
		  goto outerr;
		debug(F100,"OUTPUT Long BREAK","",0);
#ifndef NOLOCAL
		ttsndlb();		/* Send Long BREAK signal */
#else
		ttoc(NUL);
#endif /* NOLOCAL */
		quote = 0;		/* Turn off quote flag */
		continue;		/* and not the l or L */
#endif /* CK_LBRK */
	    } else {			/* if \ not followed by b or B */
		/* Note: Atari ST compiler won't allow macro call in "if ()" */
		xx = oboc(dopar(CMDQ));	/* Output the backslash. */
		if (xx < 0)
		  goto outerr;
		quote = 0;		/* Turn off quote flag */
	    }
	} else				/* No quote */
	  quote = 0;			/* Turn off quote flag */

	xx = oboc(dopar((char)x));	/* Output this character */
	debug(F111,"dooutput",obuf,obn);
	if (xx < 0)
	  goto outerr;
	if (seslog && duplex)		/* Log the character if log is on */
	  if (zchout(ZSFILE,(char)x) < 0) /* and connection is half duplex */
	    seslog = 0;
	if (x == '\015') {		/* String contains carriage return */
	    int stuff = -1;
	    if (tnlm) {			/* TERMINAL NEWLINE ON */
		stuff = LF;		/* Stuff LF */
#ifdef TNCODE
	    } else if (network &&	/* TELNET NEWLINE ON/OFF/RAW */
		       (ttnproto == NP_TELNET) &&
		       (tn_nlm != TNL_CR)) {
		stuff = (tn_nlm == TNL_CRLF) ? LF : NUL;
#endif /* TNCODE */
	    }
	    if (stuff > -1) {		/* Stuffing another character... */
		xx = oboc(dopar((CHAR)stuff)); 
		if (xx < 0)
		  goto outerr;
		if (seslog && duplex)	/* Log stuffed char if appropriate */
		  if (zchout(ZSFILE, (CHAR)stuff) < 0)
		    seslog = 0;
	    }
	    if (xxout(obuf,obn) < 0)	/* Flushing is required here! */
	      goto outerr;
	}
    }
    if (obn > 0)			/* OUTPUT done */
      if (xxout(obuf,obn) < 0)		/* Flush the buffer if necessary. */
	goto outerr;
    return(1);

outerr:					/* OUTPUT command error handler */
    if (msgflg) printf("OUTPUT error\n");
    return(0);

/* Remove "local" OUTPUT macro defininitions */

#ifdef COMMENT
/* No more macros ... */
#undef oboc
#undef obfls
#endif /* COMMENT */
}
#endif /* NOSPL */

/* Display version herald and initial prompt */

VOID
herald() {
    int x = 0;
    if (noherald || bgset > 0 || (bgset != 0 && backgrd != 0)) x = 1;
    debug(F101,"herald","",backgrd);
    if (x == 0) {
#ifdef datageneral
	printf("%s, for%s\n",versio,ckxsys);
#else
	printf("%s, for%s\n\r",versio,ckxsys);
#endif /* datageneral */
	printf(" Copyright (C) 1985, 1994,\n");
	printf("  Trustees of Columbia University in the City of New York.\n");
	printf("Type ? or HELP for help.\n");
    }
}

#ifndef NOSPL
/*  M L O O K  --  Lookup the macro name in the macro table  */
 
/*
 Call this way:  v = mlook(table,word,n);
 
   table - a 'struct mtab' table.
   word  - the target string to look up in the table.
   n     - the number of elements in the table.
 
 The keyword table must be arranged in ascending alphabetical order, and
 all letters must be lowercase.
 
 Returns the table index, 0 or greater, if the name was found, or:
 
  -3 if nothing to look up (target was null),
  -2 if ambiguous,
  -1 if not found.
 
 A match is successful if the target matches a keyword exactly, or if
 the target is a prefix of exactly one keyword.  It is ambiguous if the
 target matches two or more keywords from the table.
*/
int
mlook(table,cmd,n) struct mtab table[]; char *cmd; int n; {
 
    int i, v, cmdlen;
 
/* Lowercase & get length of target, if it's null return code -3. */
 
    if ((((cmdlen = lower(cmd))) == 0) || (n < 1)) return(-3);
 
/* Not null, look it up */
 
    for (i = 0; i < n-1; i++) {
        if (!strcmp(table[i].kwd,cmd) ||
           ((v = !strncmp(table[i].kwd,cmd,cmdlen)) &&
             strncmp(table[i+1].kwd,cmd,cmdlen))) {
                return(i);
             }
        if (v) return(-2);
    }   
 
/* Last (or only) element */
 
    if (!strncmp(table[n-1].kwd,cmd,cmdlen)) {
        return(n-1);
    } else return(-1);
}

/* mxlook is like mlook, but an exact full-length match is required */

int
mxlook(table,cmd,n) char *cmd; struct mtab table[]; int n; {
    int i, cmdlen;
    if ((((cmdlen = lower(cmd))) == 0) || (n < 1)) return(-3);
    for (i = 0; i < n; i++)
      if (((int)strlen(table[i].kwd) == cmdlen) &&
	  (!strncmp(table[i].kwd,cmd,cmdlen))) return(i);
    return(-1);
}

/*
  This routine is for the benefit of those compilers that can't handle
  long string constants or continued lines within them.  Long predefined
  macros like FOR, WHILE, and XIF have their contents broken up into
  arrays of string pointers.  This routine concatenates them back into a
  single string again, and then calls the real addmac() routine to enter
  the definition into the macro table.
*/
int
addmmac(nam,s) char *nam, *s[]; {	/* Add a multiline macro definition */
    int i, x, y; char *p;
    x = 0;				/* Length counter */
    for (i = 0; (y = (int)strlen(s[i])) > 0; i++) { /* Add up total length */
    	debug(F111,"addmmac line",s[i],y);	
	x += y;
    }
    debug(F101,"addmmac lines","",i);
    debug(F101,"addmmac loop exit","",y);
    debug(F111,"addmmac length",nam,x);
    if (x < 0) return(-1);

    p = malloc(x+1);			/* Allocate space for all of it. */
    if (!p) {
	printf("?addmmac malloc error: %s\n",nam);
	debug(F110,"addmmac malloc error",nam,0);
	return(-1);
    }
    *p = '\0';				/* Start off with null string. */
    for (i = 0; *s[i]; i++)		/* Concatenate them all together. */
      strcat(p,s[i]);
    y = (int)strlen(p);			/* Final precaution. */
    debug(F111,"addmmac constructed string",p,y);
    if (y == x) {
	y = addmac(nam,p);		/* Add result to the macro table. */
    } else {
	debug(F100,"addmmac length mismatch","",0);
	printf("\n!addmmac internal error!\n");
	y = -1;
    }
    free(p);				/* Free the temporary copy. */
    return(y);	
}

/* Here is the real addmac routine. */

int
addmac(nam,def) char *nam, *def; {	/* Add a macro to the macro table */
    int i, x, y, z, namlen, deflen;
    char *p, c;

    if (!nam) return(-1);
    namlen = (int)strlen(nam);		/* Get argument lengths */
    debug(F111,"addmac nam",nam,namlen);
    if (!def) {				/* Watch out for null pointer */
	deflen = 0;
	debug(F111,"addmac def","(null pointer)",deflen);
    } else {
	deflen = (int)strlen(def);
	debug(F111,"addmac def",def,deflen);
    }
    if (deflen < 0) return(-1);		/* strlen() failure, fail. */
    if (namlen < 1) return(-1);		/* No name given, fail. */

    if (*nam == CMDQ) nam++;		/* Backslash quote? */
    if (*nam == '%') {			/* Yes, if it's a variable name, */
	delmac(nam);			/* Delete any old value. */
	if (!(c = *(nam + 1))) return(-1); /* Variable name letter or digit */
	if (deflen < 1) {		/* Null definition */
	    p = NULL;			/* Better not malloc or strcpy! */
	} else {			/* A substantial definition */
	    p = malloc(deflen + 1);	/* Allocate space for it */
	    if (!p) {
		printf("?addmac malloc error 2\n");
		return(-1);
	    } else strcpy(p,def);	/* Copy definition into new space */
	}

	/* Now p points to the definition, or is a null pointer */

	if (p)
	  debug(F110,"addmac p",p,0);
	else
	  debug(F110,"addmac p","(null pointer)",0);

	if (c >= '0' && c <= '9') {	/* Digit variable */
	    if (maclvl < 0) {		/* Are we calling or in a macro? */
		g_var[c] = p;		/* No, it's a global "top level" one */
		debug(F101,"addmac numeric global maclvl","",maclvl);
	    } else {			/* Yes, it's a macro argument */
		m_arg[maclvl][c - '0'] = p;
		debug(F101,"addmac macro arg maclvl","",maclvl);
	    }
	} else {			/* It's a global variable */
	    if (c < 33 || c > GVARS) return(-1);
	    if (isupper(c)) c = tolower(c);
	    g_var[c] = p;		/* Put pointer in global-var table */
	    debug(F100,"addmac global","",0);
	}
	return(0);
    } else if (*nam == '&') {		/* An array reference? */
	char **q;
	if ((y = arraynam(nam,&x,&z)) < 0) /* If syntax is bad */
	  return(-1);			/* return -1. */
	if (chkarray(x,z) < 0)		/* If array not declared or */
	  return(-2);			/* subscript out of range, ret -2 */
	delmac(nam);			/* Delete any current definition. */
	x -= 96;			/* Convert name letter to index. */
	if ((q = a_ptr[x]) == NULL)	/* If array not declared, */
	  return(-3);			/* return -3. */
	if (deflen > 0) {
	    if ((p = malloc(deflen+1)) == NULL) { /* Allocate space */
		printf("addmac macro error 7: %s\n",nam);
		return(-4);		/* for new def, return -4 on fail. */
	    }
	    strcpy(p,def);		/* Copy definition into new space. */
	} else p = NULL;
	q[z] = p;			/* Store pointer to it. */
	return(0);			/* Done. */
    } else debug(F110,"addmac macro def",nam,0);

/* Not a macro argument or a variable, so it's a macro definition */

    lower(nam);				/* Lowercase the name */
    if (mxlook(mactab,nam,nmac) > -1)	/* Look up, requiring exact match */
      delmac(nam);			/* if it's there, delete it. */
    debug(F111,"addmac table size",nam,nmac);
    for (y = 0;				/* Find the alphabetical slot */
	 y < MAC_MAX && mactab[y].kwd != NULL && strcmp(nam,mactab[y].kwd) > 0;
	 y++) ;
    if (y == MAC_MAX) {			/* No more room. */
	debug(F101,"addmac table overflow","",y);
	return(-1);
    } else debug(F111,"addmac position",nam,y);
    if (mactab[y].kwd != NULL) {	/* Must insert */
	for (i = nmac; i > y; i--) {	/* Move the rest down one slot */
	    mactab[i].kwd = mactab[i-1].kwd;
	    mactab[i].mval = mactab[i-1].mval;
	    mactab[i].flgs = mactab[i-1].flgs;
	}
    }
    p = malloc(namlen + 1);		/* Allocate space for name */
    if (!p) {
	printf("?addmac malloc error 3: %s\n",nam);
	return(-1);
    }
    strcpy(p,nam);			/* Copy name into new space */
    mactab[y].kwd = p;			/* Add pointer to table */

    if (deflen > 0) {			/* Same deal for definition */
	p = malloc(deflen + 1);		/* but watch out for null pointer */
	if (p == NULL) {
	    printf("?addmac malloc error 5: %s\n", nam);
	    free(mactab[y].kwd);
	    mactab[y].kwd = NULL;
	    return(-1);
	} else strcpy(p,def);		/* Copy the definition */
    } else p = NULL;
    mactab[y].mval = p;
    mactab[y].flgs = 0;
    nmac++;				/* Count this macro */
    return(y);
}

int
delmac(nam) char *nam; {		/* Delete the named macro */
    int i, x, z;
    char *p, c;

    if (!nam) return(0);		/* Watch out for null pointer */
    debug(F110,"delmac nam",nam,0);
    if (*nam == CMDQ) nam++;
    if (*nam == '%') {			/* If it's a variable name */
	if (!(c = *(nam+1))) return(0);	/* Get variable name letter or digit */
	p = (char *)0;			/* Initialize value pointer */
	if (maclvl > -1 && c >= '0' && c <= '9') { /* Digit? */
	    p = m_arg[maclvl][c - '0'];	/* Get pointer from macro-arg table */
	    m_arg[maclvl][c - '0'] = NULL; /* Zero the table pointer */
	} else {			/* It's a global variable */
	    if (c < 33 || c > GVARS) return(0);
	    p = g_var[c];		/* Get pointer from global-var table */
	    g_var[c] = NULL;		/* Zero the table entry */
	}
	if (p) {
	    debug(F110,"delmac def",p,0);
	    free(p);			/* Free the storage */
	} else debug(F110,"delmac def","(null pointer)",0);
	return(0);
    }
	    
    if (*nam == '&') {			/* An array reference? */
	char **q;
	if (arraynam(nam,&x,&z) < 0)	/* If syntax is bad */
	  return(-1);			/* return -1. */
	x -= 96;			/* Convert name to number. */
	if ((q = a_ptr[x]) == NULL)	/* If array not declared, */
	  return(-2);			/* return -2. */
	if (z > a_dim[x])		/* If subscript out of range, */
	  return(-3);			/* return -3. */
	if (q[z]) {			/* If there is an old value, */
	    debug(F110,"delman def",q[z],0);
	    free(q[z]);			/* delete it. */
	    q[z] = NULL;
	} else debug(F110,"delmac def","(null pointer)",0);
    }

   /* Not a variable or an array, so it must be a macro. */

    if ((x = mlook(mactab,nam,nmac)) < 0) { /* Look it up */
	debug(F111,"delmac mlook",nam,x);
	return(x);
    }
    if (mactab[x].kwd)			/* Free the storage for the name */
      free(mactab[x].kwd);
    if (mactab[x].mval)			/* and for the definition */
      free(mactab[x].mval);

    for (i = x; i < nmac; i++) {	/* Now move up the others. */
	mactab[i].kwd = mactab[i+1].kwd;
	mactab[i].mval = mactab[i+1].mval;
	mactab[i].flgs = mactab[i+1].flgs;
    }
    nmac--;				/* One less macro */
    mactab[nmac].kwd = NULL;		/* Delete last item from table */
    mactab[nmac].mval = NULL;
    mactab[nmac].flgs = 0;
    return(0);
}

VOID
initmac() {				/* Init macro & variable tables */
    int i, j;

    nmac = 0;				/* No macros */
    for (i = 0; i < MAC_MAX; i++) {	/* Initialize the macro table */
	mactab[i].kwd = NULL;
	mactab[i].mval = NULL;
	mactab[i].flgs = 0;
    }
    for (i = 0; i < MACLEVEL; i++) {	/* Init the macro argument tables */
	mrval[i] = NULL;
	for (j = 0; j < 10; j++) {
	    m_arg[i][j] = NULL;
	}
    }
    for (i = 0; i < GVARS; i++) {	/* And the global variables table */
	g_var[i] = NULL;
    }
    for (i = 0; i < 26; i++) {		/* And the table of arrays */
	a_ptr[i] = (char **) NULL;	/* Null pointer for each */
	a_dim[i] = 0;			/* and a dimension of zero */
    }
}

int
popclvl() {				/* Pop command level, return cmdlvl */
    if (cmdlvl < 1) {			/* If we're already at top level */
	cmdlvl = 0;			/* just make sure all the */
	tlevel = -1;			/* stack pointers are set right */
	maclvl = -1;			/* and return */
    } else if (cmdstk[cmdlvl].src == CMD_TF) { /* Reading from TAKE file? */
	if (tlevel > -1) {		/* Yes, */
	    if (tfnam[tlevel]) {
		free(tfnam[tlevel]);
		tfnam[tlevel] = NULL;
	    }
	    fclose(tfile[tlevel--]);	/* close it and pop take level */
	    cmdlvl--;			/* pop command level */
	} else tlevel = -1;
    } else if (cmdstk[cmdlvl].src == CMD_MD) { /* In a macro? */
	if (maclvl > -1) {		/* Yes, */
	    debug(F111,"popclvl before",macx[maclvl],maclvl);
	    macp[maclvl] = "";		/* set macro pointer to null string */
	    *cmdbuf = '\0';		/* clear the command buffer */
	    if (mrval[maclvl+1]) {	/* Free any deeper return values. */
		free(mrval[maclvl+1]);
		mrval[maclvl+1] = NULL;
	    }
	    maclvl--;			/* pop macro level */
	    cmdlvl--;			/* and command level */
	    debug(F111,"popclvl after ",
		  macx[maclvl] ? macx[maclvl] : "",maclvl);
	} else maclvl = -1;
    }
#ifndef MAC
    if (cmdlvl < 1) {			/* If back at top level */
	conint(trap,stptrap);		/* Fix interrupts */
	bgchk();			/* Check background status */
	concb((char)escape);		/* Go into cbreak mode */
    }
#endif /* MAC */
    return(cmdlvl < 1 ? 0 : cmdlvl);	/* Return command level */
}
#else /* No script programming language */
int popclvl() {				/* Just close current take file. */
    if (tlevel > -1) {			/* if any... */
	if (tfnam[tlevel]) {
	    free(tfnam[tlevel]);
	    tfnam[tlevel] = NULL;
	}
	fclose(tfile[tlevel--]);
    }
    if (tlevel == -1) {			/* And if back at top level */
	conint(trap,stptrap);		/* check and set interrupts */
        bgchk();			/* and background status */
        concb((char)escape);		/* and go back into cbreak mode. */
    }
    return(tlevel + 1);
}
#endif /* NOSPL */

/* STOP - get back to C-Kermit prompt, no matter where from. */

int
dostop() {
    while ( popclvl() )  ;	/* Pop all macros & take files */
#ifndef NOSPL
    while (cmpop() > -1) ;	/* And all recursive cmd pkg invocations */
#endif /* NOSPL */
    cmini(ckxech);		/* Clear the command buffer. */
    return(0);
}


/* Close the given log */

int
doclslog(x) int x; {
    int y;
    switch (x) {
#ifdef DEBUG
	case LOGD:
	    if (deblog == 0) {
		printf("?Debugging log wasn't open\n");
		return(0);
	    }
	    *debfil = '\0';
	    deblog = 0;
	    return(zclose(ZDFILE));
#endif /* DEBUG */

	case LOGP:
	    if (pktlog == 0) {
		printf("?Packet log wasn't open\n");
		return(0);
	    }
	    *pktfil = '\0';
	    pktlog = 0;
	    return(zclose(ZPFILE));
 
#ifndef NOLOCAL
	case LOGS:
	    if (seslog == 0) {
		printf("?Session log wasn't open\n");
		return(0);
	    }
	    *sesfil = '\0';
	    seslog = 0;
	    return(zclose(ZSFILE));
#endif /* NOLOCAL */
 
#ifdef TLOG
    	case LOGT:
	    if (tralog == 0) {
		printf("?Transaction log wasn't open\n");
		return(0);
	    }
	    *trafil = '\0';
	    tralog = 0;
	    return(zclose(ZTFILE));
#endif /* TLOG */
 
#ifndef NOSPL
          case LOGW:			/* WRITE file */
	  case LOGR:			/* READ file */
	    y = (x == LOGR) ? ZRFILE : ZWFILE;
	    if (chkfn(y) < 1)		/* If no file to close */
	      return(1);		/* succeed silently. */
	    return(zclose(y));		/* Otherwise, close the file. */
#endif /* NOSPL */

	default:
	    printf("\n?Unexpected log designator - %d\n", x);
	    return(0);
	}
}

#ifndef NOSERVER
#ifndef NOFRILLS
static char *nm[] = { "disabled", "enabled" };
#endif /* NOFRILLS */
#endif /* NOSERVER */

static int slc = 0;			/* Screen line count */

#ifndef NOSHOW
char *
showoff(x) int x; {
    return( x ? "on" : "off");
}

#ifndef NOFRILLS
#define xxdiff(v,sys) strncmp(v,sys,strlen(sys))
VOID
shover() {
    printf("\nVersions:\n %s\n Numeric: %ld",versio,vernum);
    if (verwho) printf("-%d",verwho);
    printf(xxdiff(ckxv,ckxsys) ? "\n %s for%s\n" : "\n %s\n",ckxv,ckxsys);
    printf(xxdiff(ckzv,ckzsys) ? " %s for%s\n" : " %s\n",ckzv,ckzsys);
    printf(" %s\n",protv);
    printf(" %s\n",fnsv);
    printf(" %s\n %s\n",cmdv,userv);
#ifndef NOCSETS
    printf(" %s\n",xlav);
#endif /* NOCSETS */
#ifndef MAC
    printf(" %s\n",connv);
#endif /* MAC */
#ifndef NODIAL
    printf(" %s\n",dialv);
#endif /* NODIAL */
#ifndef NOSCRIPT
    printf(" %s\n",loginv);
#endif /* NOSCRIPT */
#ifdef NETCONN
    printf(" %s\n",cknetv);
#ifdef OS2
    printf(" %s\n",ckonetv);
#ifdef CK_NETBIOS
    printf(" %s\n",ckonbiv);
#endif /* CK_NETBIOS */
#ifdef OS2MOUSE
   printf(" %s\n",ckomouv);
#endif /* OS2MOUSE */
#endif /* OS2 */
#endif /* NETCONN */
    printf("\n");
}

VOID
shofea() {
    int flag = 0;
#ifdef OS2
#ifdef M_I286
    printf("\nOS/2 16-bit.\n");
#else
    printf("\nOS/2 32-bit.\n");
#endif /* M_I286 */
#endif /* OS2 */
    printf("\nSpecial features:\n");
#ifdef NETCONN
    printf(" Network support (type SHOW NET for further info)\n");
#endif /* NETCONN */
#ifdef CK_RTSCTS
    printf(" Hardware flow control\n");
#endif /* CK_RTSCTS */

#ifndef NOCSETS
    printf(" Latin-1 (West European) character-set translation\n");
#ifdef LATIN2
    printf(" Latin-2 (East European) character-set translation\n");
#endif /* LATIN2 */
#ifdef CYRILLIC
    printf(" Cyrillic (Russian, Ukrainian, etc) character-set translation\n");
#endif /* CYRILLIC */
#ifdef HEBREW
    printf(" Hebrew character-set translation\n");
#endif /* HEBREW */
#ifdef KANJI
    printf(" Kanji (Japanese) character-set translation\n");
#endif /* KANJI */
#endif /* NOCSETS */

#ifdef CK_REDIR
    printf(" REDIRECT command\n");
#endif /* CK_REDIR */
#ifdef CK_RESEND
    printf(" RESEND command\n");
#endif /* CK_RESEND */

#ifdef CK_CURSES
    printf(" Fullscreen file transfer display\n");
#endif /* CK_CURSES */

#ifdef CK_SPEED
    printf(" Control-character unprefixing\n");
#endif /* CK_SPEED */

#ifdef OS2MOUSE
   printf(" OS/2 Mouse support\n") ;
#endif /* OS2MOUSE */

#ifdef CK_REXX
   printf(" REXX script language interface\n");
#endif /* CK_REXX */

    printf("\nFeatures not included:\n");
#ifndef CK_CURSES
#ifndef MAC
    printf(" No fullscreen file transfer display\n");
    flag = 1;
#endif /* MAC */
#endif /* CK_CURSES */
#ifdef NOSERVER
    printf(" No server mode\n");
    flag = 1;
#endif /* NOSERVER */
#ifdef NODEBUG
    printf(" No debugging\n");
    flag = 1;
#endif /* NODEBUG */
#ifdef NOTLOG
    printf(" No transaction log\n");
    flag = 1;
#endif /* NOTLOG */
#ifdef NOHELP
    printf(" No built-in help\n");
    flag = 1;
#endif /* NOHELP */
#ifndef NETCONN
    printf(" No network support\n");
    flag = 1;
#endif /* NETCONN */
#ifdef NOMSEND
    printf(" No MSEND command\n");
    flag = 1;
#endif /* NOMSEND */
#ifdef NODIAL
    printf(" No DIAL command\n");
    flag = 1;
#else
#ifdef MINIDIAL
    printf(
	 " No DIAL command for modems other than Hayes, CCITT, and Unknown\n");
    flag = 1;
#endif /* MINIDIAL */
#ifndef CK_RTSCTS
#ifndef MAC
    printf(" No hardware flow control\n");
    flag = 1;
#endif /* MAC */
#endif /* CK_RTSCTS */
#endif /* NODIAL */
#ifdef NOXMIT
    printf(" No TRANSMIT command\n");
    flag = 1;
#endif /* NOXMIT */
#ifdef NOSCRIPT
    printf(" No SCRIPT command\n");
    flag = 1;
#endif /* NOSCRIPT */
#ifdef NOSPL
    printf(" No script programming features\n");
    flag = 1;
#endif /* NOSPL */
#ifdef NOCSETS
    printf(" No character-set translation\n");
    flag = 1;
#else
#ifndef LATIN2
    printf(" No Latin-2 character-set translation\n");
    flag = 1;
#endif /* LATIN2 */
#ifdef NOHEBREW
    printf(" No Hebrew character-set translation\n");
    flag = 1;
#endif /* NOHEBREW */
#ifdef NOCYRIL
    printf(" No Cyrillic character-set translation\n");
    flag = 1;
#endif /* NOCYRIL */
#ifndef KANJI
    printf(" No Kanji character-set translation\n");
    flag = 1;
#endif /* KANJI */
#endif /* NOCSETS */
#ifdef NOCMDL
    printf(" No command-line arguments\n");
    flag = 1;
#endif /* NOCMDL */
#ifdef NOFRILLS
    printf(" No frills\n");
    flag = 1;
#endif /* NOFRILLS */
#ifdef NOPUSH
    printf(" No escape to system\n");
    flag = 1;
#endif /* NOPUSH */
#ifdef NOJC
#ifdef UNIX
    printf(" No UNIX job control\n");
    flag = 1;
#endif /* UNIX */
#endif /* NOJC */
#ifdef NOSETKEY
    printf(" No SET KEY command\n");
    flag = 1;
#endif /* NOSETKEY */
#ifndef PARSENSE
    printf(" No automatic parity detection\n");
    flag = 1;
#endif /* PARSENSE */
#ifndef CK_SPEED
    printf(" No control-character unprefixing\n");
    flag = 1;
#endif /* CK_SPEED */
#ifndef CK_REDIR
    printf(" No REDIRECT command\n");
    flag = 1;
#endif /* CK_REDIR */
#ifndef CK_RESEND
    printf(" No RESEND command\n");
    flag = 1;
#endif /* CK_RESEND */
#ifdef OS2
#ifdef __32BIT__
#ifndef OS2MOUSE
    printf(" No OS/2 Mouse support\n") ;
    flag = 1;
#endif /* __32BIT__ */
#endif /* OS2 */
#endif /* OS2MOUSE */
#ifdef OS2
#ifndef CK_REXX
    printf(" No REXX script language interface\n");
    flag = 1;
#endif /* CK_REXX */
#endif /* OS2 */

    if (flag == 0) printf(" None\n");
/*
  Print compile-time (-D) options, as well as C preprocessor
  predefined symbols that might affect us...
*/
#ifdef __DATE__				/* GNU and other ANSI */
#ifdef __TIME__
    printf("\nCompiled %s %s, options:\n", __DATE__, __TIME__);
#else
    printf("\nCompiled %s, options:\n", __DATE__);
#endif /* __TIME__ */
#else /* !__DATE__ */
    printf("\nCompiler options:\n");
#endif /* __DATE__ */

#ifdef MAC
#ifdef MPW
    prtopt("MPW");
#else
#ifdef THINK_C
    prtopt("THINK_C");
#endif /* THINK_C */
#endif /* MPW */
#endif /* MAC */

#ifdef __386__
    prtopt("__386__");
#endif

#ifdef DEBUG
#ifdef IFDEBUG
    prtopt("IFDEBUG");
#else
    prtopt("DEBUG");
#endif /* IFDEBUG */
#endif /* DEBUG */
#ifdef TLOG
    prtopt("TLOG");
#endif /* TLOG */
#ifdef XFRCAN
    prtopt("XFRCAN");
#endif /* XFRCAN */
#ifdef CK_SPEED
    prtopt("CK_SPEED");
#endif /* CK_SPEED */
#ifdef CK_APC
    prtopt("CK_APC");
#endif /* CK_APC */
#ifdef CK_MKDIR
    prtopt("CK_MKDIR");
#endif /* CK_MKDIR */
#ifdef NOMKDIR
    prtopt("NOMKDIR");
#endif /* NOMKDIR */
#ifdef CK_LABELED
    prtopt("CK_LABELED");
#endif /* CK_LABELED */
#ifdef NODIAL
    prtopt("NODIAL");
#endif /* NODIAL */
#ifdef MINIDIAL
    prtopt("MINIDIAL");
#endif /* MINIDIAL */
#ifdef DYNAMIC
    prtopt("DYNAMIC");
#endif /* IFDEBUG */
#ifndef NOSPL
    sprintf(line,"CMDDEP=%d",CMDDEP);
    prtopt(line);
#endif /* NOSPL */
#ifdef UNIX
    prtopt("UNIX");
#endif /* UNIX */
#ifdef VMS
    prtopt("VMS");
#endif /* VMS */
#ifdef OLD_VMS
    prtopt("OLD_VMS");
#endif /* OLD_VMS */
#ifdef vms
    prtopt("vms");
#endif /* vms */
#ifdef VMSSHARE
    prtopt("VMSSHARE");
#endif /* VMSSHARE */
#ifdef datageneral
    prtopt("datageneral");
#endif /* datageneral */
#ifdef apollo
    prtopt("apollo");
#endif /* apollo */
#ifdef aegis
    prtopt("aegis");
#endif /* aegis */
#ifdef A986
    prtopt("A986");
#endif /* A986 */
#ifdef AMIGA
    prtopt("AMIGA");
#endif /* AMIGA */
#ifdef CONVEX9
    prtopt("CONVEX9");
#endif /* CONVEX9 */
#ifdef CONVEX10
    prtopt("CONVEX10");
#endif /* CONVEX9 */
#ifdef MAC
    prtopt("MAC");
#endif /* MAC */
#ifdef AUX
    prtopt("AUX");
#endif /* AUX */
#ifdef OS2
    prtopt("OS2");
#endif /* OS2 */
#ifdef OS9
    prtopt("OS9");
#endif /* OS9 */
#ifdef MSDOS
    prtopt("MSDOS");
#endif /* MSDOS */
#ifdef DIRENT
    prtopt("DIRENT");
#endif /* DIRENT */
#ifdef SDIRENT
    prtopt("SDIRENT");
#endif /* SDIRENT */
#ifdef NDIR
    prtopt("NDIR");
#endif /* NDIR */
#ifdef XNDIR
    prtopt("XNDIR");
#endif /* XNDIR */
#ifdef MATCHDOT
    prtopt("MATCHDOT");
#endif /* MATCHDOT */
#ifdef SAVEDUID
    prtopt("SAVEDUID");
#endif /* SAVEDUID */
#ifdef RENAME
    prtopt("RENAME");
#endif /* RENAME */
#ifdef CK_TMPDIR
    prtopt("CK_TMPDIR");
#endif /* CK_TMPDIR */
#ifdef NOCCTRAP
    prtopt("NOCCTRAP");
#endif /* NOCCTRAP */
#ifdef CK_NEED_SIG
    prtopt("CK_NEED_SIG");
#endif /* CK_NEED_SIG */
#ifdef CK_TTYFD
    prtopt("CK_TTYFD");
#endif /* CK_TTYFD */
#ifdef NETCONN
    prtopt("NETCONN");
#endif /* NETCONN */
#ifdef TCPSOCKET
    prtopt("TCPSOCKET");
#ifdef CK_LINGER
    prtopt("CK_LINGER");
#endif /* CK_LINGER */
#ifdef CK_SOCKBUF
    prtopt("CK_SOCKBUF");
#endif /* CK_SOCKBUF */
#endif /* TCPSOCKET */
#ifdef SUNX25
    prtopt("SUNX25");
#endif /* SUNX25 */
#ifdef DECNET
    prtopt("DECNET");
#endif /* DECNET */
#ifdef NPIPE
    prtopt("NPIPE");
#endif /* NPIPE */
#ifdef CK_NETBIOS
    prtopt("CK_NETBIOS");
#endif /* CK_NETBIOS */
#ifdef ATT7300
    prtopt("ATT7300");
#endif /* ATT7300 */
#ifdef ATT6300
    prtopt("ATT6300");
#endif /* ATT6300 */
#ifdef HDBUUCP
    prtopt("HDBUUCP");
#endif /* HDBUUCP */
#ifdef NOUUCP
    prtopt("NOUUCP");
#endif /* NOUUCP */
#ifdef LONGFN
    prtopt("LONGFN");
#endif /* LONGFN */
#ifdef RDCHK
    prtopt("RDCHK");
#endif /* RDCHK */
#ifdef SELECT
    prtopt("SELECT");
#endif /* SELECT */
#ifdef USLEEP
    prtopt("USLEEP");
#endif /* USLEEP */
#ifdef NAP
    prtopt("NAP");
#endif /* NAP */
#ifdef NAPHACK
    prtopt("NAPHACK");
#endif /* NAPHACK */
#ifdef CK_POLL
    prtopt("CK_POLL");
#endif /* CK_POLL */
#ifdef NOIEXTEN
    prtopt("NOIEXTEN");
#endif /* NOIEXTEN */
#ifdef EXCELAN
    prtopt("EXCELAN");
#endif /* EXCELAN */
#ifdef INTERLAN
    prtopt("INTERLAN");
#endif /* INTERLAN */
#ifdef NOFILEH
    prtopt("NOFILEH");
#endif /* NOFILEH */
#ifdef NOSYSIOCTLH
    prtopt("NOSYSIOCTLH");
#endif /* NOSYSIOCTLH */
#ifdef DCLPOPEN
    prtopt("DCLPOPEN");
#endif /* DCLPOPEN */
#ifdef NOSETBUF
    prtopt("NOSETBUF");
#endif /* NOSETBUF */
#ifdef NOFDZERO
    prtopt("NOFDZERO");
#endif /* NOFDZERO */
#ifdef NOPOPEN
    prtopt("NOPOPEN");
#endif /* NOPOPEN */
#ifdef NOPARTIAL
    prtopt("NOPARTIAL");
#endif /* NOPARTIAL */
#ifdef NOKVERBS
    prtopt("NOKVERBS");
#endif /* NOKVERBS */
#ifdef NOSETREU
    prtopt("NOSETREU");
#endif /* NOSETREU */
#ifdef _POSIX_SOURCE
    prtopt("_POSIX_SOURCE");
#endif /* _POSIX_SOURCE */
#ifdef LCKDIR
    prtopt("LCKDIR");
#endif /* LCKDIR */
#ifdef ACUCNTRL
    prtopt("ACUCNTRL");
#endif /* ACUCNTRL */
#ifdef BSD4
    prtopt("BSD4");
#endif /* BSD4 */
#ifdef BSD44
    prtopt("BSD44");
#endif /* BSD44 */
#ifdef BSD41
    prtopt("BSD41");
#endif /* BSD41 */
#ifdef BSD43
    prtopt("BSD43");
#endif /* BSD43 */
#ifdef BSD29
    prtopt("BSD29");
#endif /* BSD29 */
#ifdef BSDI
    prtopt("BSDI");
#endif /* BSDI */
#ifdef __bsdi__
    prtopt("__bsdi__");
#endif /* __bsdi__ */
#ifdef __NetBSD__
    prtopt("__NetBSD__");
#endif /* __NetBSD__ */
#ifdef __FreeBSD__
    prtopt("__FreeBSD__");
#endif /* __FreeBSD__ */
#ifdef __linux__
    prtopt("__linux__");
#endif /* __linux__ */
#ifdef LINUX_HI_SPD
    prtopt("LINUX_HI_SPD");
#endif /* LINUX_HI_SPD */
#ifdef LYNXOS
    prtopt("LYNXOS");
#endif /* LYNXOS */
#ifdef V7
    prtopt("V7");
#endif /* V7 */
#ifdef AIX370
    prtopt("AIX370");
#endif /* AIX370 */
#ifdef RTAIX
    prtopt("RTAIX");
#endif /* RTAIX */
#ifdef HPUX
    prtopt("HPUX");
#endif /* HPUX */
#ifdef HPUXPRE65
    prtopt("HPUXPRE65");
#endif /* HPUXPRE65 */
#ifdef DGUX
    prtopt("DGUX");
#endif /* DGUX */
#ifdef DGUX430
    prtopt("DGUX430");
#endif /* DGUX430 */
#ifdef DGUX540
    prtopt("DGUX540");
#endif /* DGUX540 */
#ifdef sony_news
    prtopt("sony_news");
#endif /* sony_news */
#ifdef CIE
    prtopt("CIE");
#endif /* CIE */
#ifdef XENIX
    prtopt("XENIX");
#endif /* XENIX */
#ifdef SCO_XENIX
    prtopt("SCO_XENIX");
#endif /* SCO_XENIX */
#ifdef ISIII
    prtopt("ISIII");
#endif /* ISIII */
#ifdef I386IX
    prtopt("I386IX");
#endif /* I386IX */
#ifdef RTU
    prtopt("RTU");
#endif /* RTU */
#ifdef PROVX1
    prtopt("PROVX1");
#endif /* PROVX1 */
#ifdef PYRAMID
    prtopt("PYRAMID");
#endif /* PYRAMID */
#ifdef TOWER1
    prtopt("TOWER1");
#endif /* TOWER1 */
#ifdef UTEK
    prtopt("UTEK");
#endif /* UTEK */
#ifdef ZILOG
    prtopt("ZILOG");
#endif /* ZILOG */
#ifdef TRS16
    prtopt("TRS16");
#endif /* TRS16 */
#ifdef MINIX
    prtopt("MINIX");
#endif /* MINIX */
#ifdef C70
    prtopt("C70");
#endif /* C70 */
#ifdef AIXPS2
    prtopt("AIXPS2");
#endif /* AIXPS2 */
#ifdef AIXRS
    prtopt("AIXRS");
#endif /* AIXRS */
#ifdef UTSV
    prtopt("UTSV");
#endif /* UTSV */
#ifdef ATTSV
    prtopt("ATTSV");
#endif /* ATTSV */
#ifdef SVR3
    prtopt("SVR3");
#endif /* SVR3 */
#ifdef SVR4
    prtopt("SVR4");
#endif /* SVR4 */
#ifdef DELL_SVR4
    prtopt("DELL_SVR4");
#endif /* DELL_SVR4 */
#ifdef ICL_SVR4
    prtopt("ICL_SVR4");
#endif /* ICL_SVR4 */
#ifdef OSF
    prtopt("OSF");
#endif /* OSF */
#ifdef OSF1
    prtopt("OSF1");
#endif /* OSF1 */
#ifdef __OSF
    prtopt("__OSF");
#endif /* __OSF */
#ifdef __OSF__
    prtopt("__OSF__");
#endif /* __OSF__ */
#ifdef __osf__
    prtopt("__osf__");
#endif /* __osf__ */
#ifdef __OSF1
    prtopt("__OSF1");
#endif /* __OSF1 */
#ifdef __OSF1__
    prtopt("__OSF1__");
#endif /* __OSF1__ */
#ifdef PTX
    prtopt("PTX");
#endif /* PTX */
#ifdef POSIX
    prtopt("POSIX");
#endif /* POSIX */
#ifdef SOLARIS
    prtopt("SOLARIS");
#endif /* SOLARIS */
#ifdef SUNOS4
    prtopt("SUNOS4");
#endif /* SUNOS4 */
#ifdef SUN4S5
    prtopt("SUN4S5");
#endif /* SUN4S5 */
#ifdef ENCORE
    prtopt("ENCORE");
#endif /* ENCORE */
#ifdef ultrix
    prtopt("ultrix");
#endif
#ifdef sxaE50
    prtopt("sxaE50");
#endif
#ifdef mips
    prtopt("mips");
#endif
#ifdef MIPS
    prtopt("MIPS");
#endif
#ifdef vax
    prtopt("vax");
#endif
#ifdef VAX
    prtopt("VAX");
#endif
#ifdef alpha
    prtopt("alpha");
#endif
#ifdef ALPHA
    prtopt("ALPHA");
#endif
#ifdef __ALPHA
    prtopt("__ALPHA");
#endif
#ifdef __alpha
    prtopt("__alpha");
#endif
#ifdef __AXP
    prtopt("__AXP");
#endif
#ifdef AXP
    prtopt("AXP");
#endif
#ifdef axp
    prtopt("axp");
#endif
#ifdef __ALPHA__
    prtopt("__ALPHA__");
#endif
#ifdef __alpha__
    prtopt("__alpha__");
#endif
#ifdef sun
    prtopt("sun");
#endif
#ifdef sun3
    prtopt("sun3");
#endif
#ifdef sun386
    prtopt("sun386");
#endif
#ifdef _SUN
    prtopt("_SUN");
#endif
#ifdef sun4
    prtopt("sun4");
#endif
#ifdef sparc
    prtopt("sparc");
#endif
#ifdef _CRAY
    prtopt("_CRAY");
#endif /* _CRAY */
#ifdef NEXT
    prtopt("NEXT");
#endif
#ifdef NeXT
    prtopt("NeXT");
#endif
#ifdef MACH
    prtopt("MACH");
#endif
#ifdef sgi
    prtopt("sgi");
#endif
#ifdef M_SYS5
    prtopt("M_SYS5");
#endif
#ifdef __SYSTEM_FIVE
    prtopt("__SYSTEM_FIVE");
#endif
#ifdef sysV
    prtopt("sysV");
#endif
#ifdef M_XENIX				/* SCO Xenix V and UNIX/386 */
    prtopt("M_XENIX");
#endif 
#ifdef M_UNIX				/* SCO UNIX */
    prtopt("M_UNIX");
#endif
#ifdef _M_UNIX				/* SCO UNIX 3.2v4 = ODT 2.0 */
    prtopt("_M_UNIX");
#endif
#ifdef M_I586
    prtopt("M_I586");
#endif
#ifdef _M_I586
    prtopt("_M_I586");
#endif
#ifdef i586
    prtopt("i586");
#endif
#ifdef M_I486
    prtopt("M_I486");
#endif
#ifdef _M_I486
    prtopt("_M_I486");
#endif
#ifdef i486
    prtopt("i486");
#endif
#ifdef M_I386
    prtopt("M_I386");
#endif
#ifdef _M_I386
    prtopt("_M_I386");
#endif
#ifdef i386
    prtopt("i386");
#endif
#ifdef i286
    prtopt("i286");
#endif
#ifdef M_I286
    prtopt("M_I286");
#endif
#ifdef mc68000
    prtopt("mc68000");
#endif
#ifdef mc68010
    prtopt("mc68010");
#endif
#ifdef mc68020
    prtopt("mc68020");
#endif
#ifdef mc68030
    prtopt("mc68030");
#endif
#ifdef mc68040
    prtopt("mc68040");
#endif
#ifdef M_68000
    prtopt("M_68000");
#endif
#ifdef M_68010
    prtopt("M_68010");
#endif
#ifdef M_68020
    prtopt("M_68020");
#endif
#ifdef M_68030
    prtopt("M_68030");
#endif
#ifdef M_68040
    prtopt("M_68040");
#endif
#ifdef m68k
    prtopt("m68k");
#endif
#ifdef m88k
    prtopt("m88k");
#endif
#ifdef pdp11
    prtopt("pdp11");
#endif
#ifdef iAPX
    prtopt("iAPX");
#endif
#ifdef __hp9000s800
    prtopt("__hp9000s800");
#endif
#ifdef __hp9000s700
    prtopt("__hp9000s700");
#endif
#ifdef __hp9000s500
    prtopt("__hp9000s500");
#endif
#ifdef __hp9000s300
    prtopt("__hp9000s300");
#endif
#ifdef __hp9000s200
    prtopt("__hp9000s200");
#endif
#ifdef AIX
    prtopt("AIX");
#endif
#ifdef _AIXFS
    prtopt("_AIXFS");
#endif
#ifdef u370
    prtopt("u370");
#endif
#ifdef u3b
    prtopt("u3b");
#endif
#ifdef u3b2
    prtopt("u3b2");
#endif
#ifdef multimax
    prtopt("multimax");
#endif
#ifdef balance
    prtopt("balance");
#endif
#ifdef ibmrt
    prtopt("ibmrt");
#endif
#ifdef _IBMRT
    prtopt("_IBMRT");
#endif
#ifdef ibmrs6000
    prtopt("ibmrs6000");
#endif
#ifdef _AIX
    prtopt("_AIX");
#endif /* _AIX */
#ifdef _IBMR2
    prtopt("_IBMR2");
#endif
#ifdef UNIXWARE
    prtopt("UNIXWARE");
#endif
#ifdef QNX
    prtopt("QNX");
#ifdef __QNX__
    prtopt("__QNX__");
#ifdef __16BIT__
    prtopt("__16BIT__");
#endif
#ifdef CK_QNX16
    prtopt("CK_QNX16");
#endif
#ifdef __32BIT__
    prtopt("__32BIT__");
#endif
#ifdef CK_QNX32
    prtopt("CK_QNX32");
#endif
#endif /* __QNX__ */
#endif /* QNX */

#ifdef __STRICT_BSD__
    prtopt("__STRICT_BSD__");
#endif
#ifdef __STRICT_ANSI__
    prtopt("__STRICT_ANSI__");
#endif
#ifdef _ANSI_C_SOURCE
    prtopt("_ANSI_C_SOURCE");
#endif
#ifdef __STDC__
    prtopt("__STDC__");
#endif
#ifdef __DECC
    prtopt("__DECC");
#endif
#ifdef __GNUC__				/* gcc in ansi mode */
    prtopt("__GNUC__");
#endif
#ifdef GNUC				/* gcc in traditional mode */
    prtopt("GNUC");
#endif
#ifdef __WATCOMC__
    prtopt("__WATCOMC__");
#endif
#ifdef CK_ANSIC
    prtopt("CK_ANSIC");
#endif
#ifdef CK_ANSILIBS
    prtopt("CK_ANSILIBS");
#endif
#ifdef _XOPEN_SOURCE
    prtopt("_XOPEN_SOURCE");
#endif
#ifdef _ALL_SOURCE
    prtopt("_ALL_SOURCE");
#endif
#ifdef _SC_JOB_CONTROL
    prtopt("_SC_JOB_CONTROL");
#endif
#ifdef _POSIX_JOB_CONTROL
    prtopt("_POSIX_JOB_CONTROL");
#endif
#ifdef CK_POSIX_SIG
    prtopt("CK_POSIX_SIG");
#endif
#ifdef SVR3JC
    prtopt("SVR3JC");
#endif
#ifdef _386BSD
    prtopt("_386BSD");
#endif
#ifdef _BSD
    prtopt("_BSD");
#endif
#ifdef TERMIOX
    prtopt("TERMIOX");
#endif /* TERMIOX */
#ifdef STERMIOX
    prtopt("STERMIOX");
#endif /* STERMIOX */
#ifdef CK_CURSES
    prtopt("CK_CURSES");
#endif /* CK_CURSES */
#ifdef CK_WREFRESH
    prtopt("CK_WREFRESH");
#endif /* CK_WREFRESH */
#ifdef CK_PCT_BAR
    prtopt("CK_PCT_BAR");
#endif /* CK_PCT_BAR */
#ifdef CK_DTRCD
    prtopt("CK_DTRCD");
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    prtopt("CK_DTRCTS");
#endif /* CK_DTRCTS */
#ifdef CK_RTSCTS
    prtopt("CK_RTSCTS");
#endif /* CK_RTSCTS */
#ifdef POSIX_CRTSCTS
    prtopt("POSIX_CRTSCTS");
#endif /* POSIX_CRTSCTS */
#ifdef CK_DSYSINI
    prtopt("CK_DSYSINI");
#endif /* CK_DSYSINI */
#ifdef CK_SYSINI
    prtopt("CK_SYSINI");
#endif /* CK_SYSINI */
#ifdef CK_INI_A
    prtopt("CK_INI_A");
#endif /* CK_INI_A */
#ifdef CK_INI_B
    prtopt("CK_INI_B");
#endif /* CK_INI_B */
#ifdef CK_NAWS
    prtopt("CK_NAWS");
#endif /* CK_NAWS */
#ifdef DCMDBUF
    prtopt("DCMDBUF");
#endif /* DCMDBUF */
#ifdef CK_RECALL
    prtopt("CK_RECALL");
#endif /* CK_RECALL */
#ifdef CLSOPN
    prtopt("CLSOPN");
#endif /* CLSOPN */
#ifdef STRATUS
    prtopt("STRATUS");
#endif /* STRATUS */
#ifdef __VOS__
    prtopt("__VOS__");
#endif /* __VOS__ */
#ifdef STRATUSX25
    prtopt("STRATUSX25");
#endif /* STRATUSX25 */
#ifdef OS2MOUSE 
   prtopt("OS2MOUSE") ;
#endif /* OS2MOUSE */
#ifdef CK_REXX
   prtopt("CK_REXX") ;
#endif /* CK_REXX */
    prtopt((char *)0);
    printf("\n\n");
}
#endif /* NOFRILLS */

#ifdef CK_LABELED
VOID
sholbl() {
#ifdef VMS
    printf("VMS Labeled File Features:\n");
    printf(" acl %s (ACL info %s)\n",
	   showoff(lf_opts & LBL_ACL),
	   lf_opts & LBL_ACL ? "preserved" : "discarded");
    printf(" backup-date %s (backup date/time %s)\n",
	   showoff(lf_opts & LBL_BCK),
	   lf_opts & LBL_BCK ? "preserved" : "discarded");
    printf(" name %s (original filename %s)\n",
	   showoff(lf_opts & LBL_NAM),
	   lf_opts & LBL_NAM ? "preserved" : "discarded");
    printf(" owner %s (original file owner id %s)\n",
	   showoff(lf_opts & LBL_OWN),
	   lf_opts & LBL_OWN ? "preserved" : "discarded");
    printf(" path %s (original file's disk:[directory] %s)\n",
	   showoff(lf_opts & LBL_PTH),
	   lf_opts & LBL_PTH ? "preserved" : "discarded");
#else
#ifdef OS2
    printf("OS/2 Labeled File features (attributes):\n");
    printf(" archive:   %s\n", showoff(lf_opts & LBL_ARC));
    printf(" extended:  %s\n", showoff(lf_opts & LBL_EXT));
    printf(" hidden:    %s\n", showoff(lf_opts & LBL_HID));
    printf(" read-only: %s\n", showoff(lf_opts & LBL_RO ));
    printf(" system:    %s\n", showoff(lf_opts & LBL_SYS));
#endif /* OS2 */
#endif /* VMS */
}
#endif /* CK_LABELED */

VOID
shotcs(cs1,cs2) int cs1, cs2; {		/* Show terminal character set */
#ifndef NOCSETS
#ifndef MAC
    char *s;
#ifdef CK_ANSIC
    int gettcs(int, int);
#else
    int gettcs();
#endif /* CK_ANSIC */

    printf(" Terminal character-set: ");
    if (cs1 == cs2) {			/* No translation */
	printf("transparent\n");
    } else {				/* Translation */
	printf("%s (remote) %s (local)\n",
	       fcsinfo[cs2].keyword,fcsinfo[cs1].keyword);
	if (cs2 != cs1) {
	    switch(gettcs(cs2,cs1)) {
	      case TC_USASCII:  s = "ascii";        break;
	      case TC_1LATIN:   s = "latin1-iso";   break;
	      case TC_2LATIN:   s = "latin2-iso";   break;
	      case TC_CYRILL:   s = "cyrillic-iso"; break;
	      case TC_JEUC:     s = "japanese-euc"; break;
	      case TC_HEBREW:   s = "hebrew-iso";   break;
	      default:          s = "transparent";  break;
	    }
	    if (strcmp(s,fcsinfo[cs1].keyword) &&
		strcmp(s,fcsinfo[cs2].keyword))
	      printf("                         (via %s)\n",s);
	}
    }
#endif /* MAC */
#endif /* NOCSETS */
}
#endif /* NOSHOW */

#ifndef NOSETKEY
VOID
shokeycode(c) int c; {
    KEY ch;
    CHAR *s;

    printf("\n Key code \\%d => ",c);
#ifndef NOKVERBS
    if (IS_KVERB(keymap[c])) {	/* \Kverb? */
	int i, kv;
	kv = keymap[c] & ~(F_KVERB);
	printf("Verb: ");
	for (i = 0; i < nkverbs; i++)
	  if (kverbs[i].kwval == kv) {
	      printf("\\K%s",kverbs[i].kwd);
	      break;
	  }
	printf("\n");
    } else
#endif /* NOKVERBS */
    if (macrotab[c]) {			/* See if there's a macro */
	printf("String: ");		/* If so, display its definition */
	s = macrotab[c];
	while (ch = *s++)
	  if (ch < 32 || ch == 127
/*
  Systems whose native character sets have graphic characters in C1...
*/
#ifndef NEXT				/* NeXT */
#ifndef AUX				/* Macintosh */
#ifndef XENIX				/* IBM PC */
#ifndef OS2				/* IBM PC */
	      || (ch > 127 && ch < 160)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
	      )
	    printf("\\{%d}",ch);	/* Display control characters */
	  else putchar((char) ch);	/* in backslash notation */
	printf("\n");
#ifndef NOKVERBS
    } else if (keymap[c] > 0x100) {	/* This means "undefined" */
	printf("Undefined\n");
#endif /* NOKVERBS */
    } else {			/* No macro, show single character */
	printf("Character: ");
	ch = keymap[c];
	if (ch < 32 || ch == 127 || ch > 255
#ifndef NEXT
#ifndef AUX
#ifndef XENIX
#ifndef OS2
	    || (ch > 127 && ch < 160)
#endif /* OS2 */
#endif /* XENIX */
#endif /* AUX */
#endif /* NEXT */
	    )
/*
  These used to be %d, but gcc 1.93 & later complain about type mismatches.
  %u is supposed to be totally portable.
*/
	  printf("\\%u",(unsigned int) ch);
	else printf("%c \\%u",(CHAR) (ch & 0xff),(unsigned int) ch);
	if (ch == (KEY) c)
	  printf(" (self, no translation)\n");
	else
	  printf("\n");
    }
}
#endif /* NOSETKEY */

#ifndef NOSHOW
int
doshow(x) int x; {
    int y, i; long zz;
    char *s;

#ifndef NOSETKEY
    if (x == SHKEY) {			/* SHOW KEY */
	int c;
	if ((y = cmcfm()) < 0) return(y);	    
#ifdef MAC
	printf("Not implemented\n");
	return(0);
#else /* Not MAC */
	printf(" Press key: ");
#ifdef UNIX
#ifdef NOSETBUF
	fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
	conbin((char)escape);		/* Put terminal in binary mode */
	c = congks(0);			/* Get character or scan code */
	concb((char)escape);		/* Restore terminal to cbreak mode */
	if (c < 0) {			/* Check for error */
	    printf("?Error reading key\n");
	    return(0);
	}
#ifndef OS2
/*
  Do NOT mask when it can be a raw scan code, perhaps > 255
*/
	c &= cmdmsk;			/* Apply command mask */
#endif /* OS2 */
	shokeycode(c);
	return(1);
#endif /* MAC */
    }
#ifndef NOKVERBS
    if (x == SHKVB) {			/* SHOW KVERBS */
	if ((y = cmcfm()) < 0) return(y);	    
	printf("\nThe following %d keyboard verbs are available:\n\n",nkverbs);
	kwdhelp(kverbs,nkverbs,"\\K","",3);
	return(1);
    }
#endif /* NOKVERBS */
#endif /* NOSETKEY */

#ifndef NOSPL
    if (x == SHMAC) {			/* SHOW MACRO */
	x = cmfld("Macro name, or carriage return to see them all","",&s,
		  NULL);
	if (x == -3)			/* This means they want see all */
	  *line = '\0';
	else if (x < 0)			/* Otherwise negative = parse error */
	  return(x);
	else				/* 0 or greater */
	  strcpy(line,s);		/* means they typed something */
	if ((y = cmcfm()) < 0) return(y); /* Get confirmation */
	if (*line) {
	    slc = 0;			/* Initial SHO MAC line number */
	    x = mlook(mactab,s,nmac);	/* Look up what they typed */
	    switch (x) {
	      case -3:			/* Nothing to look up */
		return(0);
	      case -1:			/* Not found */
		printf("%s - not found\n",s);
		return(0);
	      case -2:			/* Ambiguous, matches more than one */
		y = (int)strlen(line);
		slc = 1;
		for (x = 0; x < nmac; x++)
		  if (!strncmp(mactab[x].kwd,line,y))
		    if (shomac(mactab[x].kwd,mactab[x].mval) < 0) break;
		return(1);
	      default:			/* Matches one exactly */
		shomac(mactab[x].kwd,mactab[x].mval);
		return(1);
	    }
	} else {			/* They want to see them all */
	    printf("Macros:\n");
	    slc = 1;
	    for (y = 0; y < nmac; y++)
	      if (shomac(mactab[y].kwd,mactab[y].mval) < 0) break;
	    return(1);
	}
    }
#endif /* NOSPL */

/*
  Other SHOW commands only have two fields.  Get command confirmation here,
  then handle with big switch() statement.
*/
    if ((y = cmcfm()) < 0) return(y);
    switch (x) {

#ifdef ANYX25
        case SHPAD:
            shopad();
            break;
#endif /* ANYX25 */

#ifdef NETCONN
	case SHNET:
	    shonet();
	    break;
#endif /* NETCONN */

	case SHPAR:
	    shopar();
	    break;
 
        case SHATT:
	    shoatt();
	    break;
 
#ifndef NOSPL
	case SHCOU:
	    printf(" %d\n",count[cmdlvl]);
	    break;
#endif /* NOSPL */

#ifndef NOSERVER
        case SHSER:			/* Show Server */
#ifndef NOFRILLS
	    printf("Function           Status:\n");
	    printf(" GET                %s\n",nm[en_get]);
	    printf(" SEND               %s\n",nm[en_sen]);	    
	    printf(" REMOTE CD/CWD      %s\n",nm[en_cwd]);
	    printf(" REMOTE DELETE      %s\n",nm[en_del]);
	    printf(" REMOTE DIRECTORY   %s\n",nm[en_dir]);
	    printf(" REMOTE HOST        %s\n",nm[en_hos]);	    
	    printf(" REMOTE SET         %s\n",nm[en_set]);	    
	    printf(" REMOTE SPACE       %s\n",nm[en_spa]);	    
	    printf(" REMOTE TYPE        %s\n",nm[en_typ]);	    
	    printf(" REMOTE WHO         %s\n",nm[en_who]);
	    printf(" BYE                %s\n",nm[en_bye]);
            printf(" FINISH             %s\n",nm[en_fin]);
#endif /* NOFRILLS */
	    printf("Server timeout: %d\n",srvtim);
	    printf("Server display: %s\n\n", showoff(srvdis));
	    break;
#endif /* NOSERVER */

        case SHSTA:			/* Status of last command */
	    printf( " %s\n", success ? "SUCCESS" : "FAILURE" );
	    return(0);			/* Don't change it */

#ifdef MAC
	case SHSTK: {			/* Stack for MAC debugging */
	    long sp;

	    sp = -1;
	    loadA0 ((char *)&sp);	/* set destination address */
	    SPtoaA0();			/* move SP to destination */
	    printf("Stack at 0x%x\n", sp);
	    show_queue();		/* more debugging */
	    break; 
	}
#endif /* MAC */

#ifndef NOLOCAL
	case SHTER:			/* SHOW TERMINAL */
	    printf(" Command  bytesize:      %d bits\n",
		   (cmdmsk == 0377) ? 8 : 7);
	    printf(" Terminal bytesize:      %d bits\n",
		   (cmask == 0377) ? 8 : 7);
#ifdef OS2
	    printf(" Terminal type:          %s",
		   (tt_type >= 0 && tt_type <= max_tt) ?
		   tt_info[tt_type].x_name :
		   "unknown"
		   );
	    if (tt_type >= 0 && tt_type <= max_tt)
	      if (strlen(tt_info[tt_type].x_id))
		printf(", ID = <ESC>%s",tt_info[tt_type].x_id);
	    printf("\n");
#else
	    s = getenv("TERM");
	    printf(" Terminal type:          %s\n", s ? s : "(unknown)");
#endif /* OS2 */
	    printf(" Terminal echo:          %s\n",
		   duplex ? "local" : "remote");
	    printf(" Terminal locking-shift: %s\n", showoff(sosi));
	    printf(" Terminal newline-mode:  %s\n", showoff(tnlm));
	    printf(" Terminal cr-display:    %s\n",
		   tt_crd ? "crlf" : "normal");
#ifdef OS2
	    printf(" Terminal cursor:        %s",
		   (tt_cursor == 2) ? "full" :
		   (tt_cursor == 1) ? "half" : "underline");
	    printf(", hide-cursor: %s\n", showoff(tt_hide));
#endif /* OS2 */
	    printf(" Terminal debug:         %s\n",showoff(debses));
#ifdef OS2
/* Should show colors here too. */
/* But colors are hard, because we don't really know what they are without */
/* also knowing the state of the emulator. */
	    printf(" Terminal arrow-keys:    %s\n",
		   tt_arrow ? "application" : "cursor");
	    printf(" Terminal keypad-mode:   %s\n",
		   tt_keypad ? "application" : "numeric");
	    printf(" Terminal answerback:    %s\n",showoff(tt_answer));
	    printf(" Terminal wrap:          %s\n",showoff(tt_wrap));
	    printf(" Terminal bell:          %s\n",
		  tt_bell ? ((tt_bell == 1) ? "audible" : "visible") : "none");
	    printf(" Terminal transmit-timeout: %d",tt_ctstmo);
	    printf(", output-pacing: %d\n",tt_pacing);
	    printf(" Terminal roll:          %s\n",showoff(tt_roll));
	    printf(" Terminal scrollback:    %d\n", tt_scrsize);
#endif /* OS2 */
#ifdef CK_APC
	    if (apcstatus == APC_ON) s = "on";
	    else if (apcstatus == APC_OFF) s = "off";
	    else if (apcstatus == APC_UNCH) s = "unchecked";
	    printf(" Terminal APC:           %s\n", s);
#endif /* CK_APC */
#ifndef NOCSETS
	    shotcs(tcsl,tcsr);		/* Show terminal character sets */
#endif /* NOCSETS */
#ifdef OS2
	    printf(" Terminal code-page:     %d\n", os2getcp());
	    printf(" Available code pages:   ");
	    {
		int cplist[8], cps;
		cps = os2getcplist(cplist, sizeof(cplist));
		for (i = 1; i < cps; i++)
		  printf("%s%d", i == 1 ? "" : ", ", cplist[i]);
		printf("\n");
	  }
#endif /* OS2 */
	    if (seslog)
	      printf(" SESSION LOG ACTIVE:     %s\n",sesfil);
#ifdef CK_NAWS				/* Console terminal screen size */
	    ttgwsiz();			/* Try to get latest size */
	    printf(" Rows = %d, Columns = %d", tt_rows, tt_cols);
#ifdef OS2MOUSE
	    printf(", Mouse: %s\n",showoff(tt_mouse));
#else
	    printf("\n");
#endif /* OS2MOUSE */
#else
#ifdef OS2MOUSE
	    printf(" Terminal mouse:         %s\n",showoff(tt_mouse));
#endif /* OS2MOUSE */
#endif /* CK_NAWS */

#ifdef COMMENT
/* No more Alt-x */
#ifdef OS2
	    printf(
" To return to the C-Kermit> prompt from CONNECT mode, use Alt-x, or:\n"
		   );
#endif /* OS2 */
#endif /* COMMENT */
	    printf(" CONNECT-mode escape character: %d (Ctrl-%c, %s)\n",
	     escape,ctl(escape),(escape == 127 ? "DEL" : ccntab[escape]));
#ifdef UNIX
#ifndef NOJC
	    printf(" Suspend: %s\n", showoff(suspend));
#endif /* NOJC */
#endif /* UNIX */
	    break;
#endif /* NOLOCAL */

#ifndef NOFRILLS
	case SHVER:
	    shover();
	    break;
#endif /* NOFRILLS */
 
#ifndef NOSPL
	case SHBUI:			/* Built-in variables */
	case SHFUN:			/* or built-in functions */
#ifdef CK_NAWS    
	    if (ttgwsiz() > 0) {	/* Get current screen size */
		if (tt_rows > 0 && tt_cols > 0) {
		    cmd_rows = tt_rows;
		    cmd_cols = tt_cols;
		}
	    }
#endif /* CK_NAWS */
	    if (x == SHFUN) {		/* Functions */
		printf("\nThe following functions are available:\n\n");
		kwdhelp(fnctab,nfuncs,"\\F","()",3);
		break;
	    } else {			/* Variables */
		i = 0;
		for (y = 0; y < nvars; y++) {
		    char *s;
		    if ((vartab[y].flgs & CM_INV))
		      continue;
		    s = nvlook(vartab[y].kwd);
		    printf(" \\v(%s) = ",vartab[y].kwd);
		    if (vartab[y].kwval == VN_NEWL) { /* \v(newline) */
			while (*s)	/* Show control chars symbolically */
			  printf("\\{%d}",*s++);
			printf("\n");
		    } else if (vartab[y].kwval == VN_IBUF || /* \v(input) */
			       vartab[y].kwval == VN_QUE) {  /* \v(query) */
			int r = 12;	/* This one can wrap around */
			char buf[10];
			while (*s) {
			    if (isprint(*s)) {
				buf[0] = *s;
				buf[1] = NUL;
				r++;
			    } else {
				sprintf(buf,"\\{%d}",*s);
				r += (int) strlen(buf);
			    }
			    if (r >= cmd_cols - 1) {
				printf("\n");
				r = 0;
				i++;
			    }
			    printf("%s",buf);
			    s++;
			}
			printf("\n");
		    } else
		      printf("%s\n",s);
		    if (++i > (cmd_rows - 3)) {	/* More than a screenful... */
			if ((y >= nvars - 1) || !askmore())
			  break;
			else
			  i = 0;
		    }
		}
	    }
            break;

        case SHVAR:			/* Global variables */
	    x = 0;			/* Variable count */
	    slc = 1;			/* Screen line count for "more?" */
	    for (y = 33; y < GVARS; y++)
	      if (g_var[y]) {
		  if (x++ == 0) printf("Global variables:\n");
		  sprintf(line," \\%%%c",y);
		  if (shomac(line,g_var[y]) < 0) break;
	      }
	    if (!x) printf(" No variables defined\n");
	    break;

        case SHARG:			/* Args */
	    if (maclvl > -1) {
		printf("Macro arguments at level %d\n",maclvl);
		for (y = 0; y < 10; y++)
		  if (m_arg[maclvl][y])
		    printf(" \\%%%d = %s\n",y,m_arg[maclvl][y]);
	    } else {
		printf(" No macro arguments at top level\n");
	    }
	    break;

        case SHARR:			/* Arrays */
	    x = 0;
	    for (y = 0; y < 27; y++)
	      if (a_ptr[y]) {
		  if (x == 0) printf("Declared arrays:\n");
		  x = 1;
		  printf(" \\&%c[%d]\n",(y == 0) ? 64 : y + 96, a_dim[y]);
	      }
	    if (!x) printf(" No arrays declared\n");
	    break;
#endif /* NOSPL */

	case SHPRO:			/* Protocol parameters */
	    shoparp();
#ifdef XFRCAN
	    printf("\n Cancellation: %s",showoff(xfrcan));
	    if (xfrcan) printf(" %d %d\n", xfrchr, xfrnum);
	    else printf("\n");
#endif /* XFRCAN */
	    printf(" Send / Receive Pause:   %d (msec)\n\n",pktpaus);
	    break;

	case SHCOM:			/* Communication parameters */
	    printf("\n");
	    shoparc();
#ifndef NODIAL
	    printf("\n");
#ifdef NETCONN
	    if (!network)
#endif /* NETCONN */
	      shodial();
#endif /* NODIAL */
	    printf("\n");
#ifdef NETCONN
	    if (!network) {
#endif /* NETCONN */
		shomdm();
		printf("\n");
#ifdef NETCONN
	    }
#endif /* NETCONN */
	    break;

	case SHFIL:			/* File parameters */
	    shoparf();
	    printf("\n");
	    break;

#ifndef NOCSETS
	case SHLNG:			/* Languages */
	    shoparl();
	    break;
#endif /* NOCSETS */

#ifndef NOSPL
	case SHSCR:			/* Scripts */
	    printf(" Command Quoting: %s\n", showoff(cmdgquo()));
	    printf(" Take  Echo:      %s\n", showoff(techo));
	    printf(" Take  Error:     %s\n", showoff(takerr[cmdlvl]));
	    printf(" Macro Echo:      %s\n", showoff(mecho));
	    printf(" Macro Error:     %s\n", showoff(merror[cmdlvl]));
	    printf(" Input Case:      %s\n", inpcas[cmdlvl] ?
		   "Observe" : "Ignore");
	    printf(" Input Echo:      %s\n", showoff(inecho));
            printf(" Input Silence:   %d (seconds)\n", insilence);
	    printf(" Input Timeout:   %s\n", intime[cmdlvl] ?
		   "quit" : "proceed");
	    printf(" Output Pacing:   %d (milliseconds)\n",pacing);
#ifndef NOSCRIPT
	    printf(" Script Echo:     %s\n", showoff(secho));
#endif /* NOSCRIPT */
	    break;
#endif /* NOSPL */

#ifndef NOXMIT
	  case SHXMI:
	    printf(" File type: %s\n", binary ? "binary" : "text");
#ifndef NOCSETS
	    shotcs(tcsl,tcsr);
#endif /* NOCSETS */
	    printf(" Terminal echo: %s\n", duplex ? "local" : "remote");
            printf(" Transmit EOF: ");
	    if (*xmitbuf == NUL) {
		printf("none\n");
	    } else {
		char *p;
		p = xmitbuf;
		while (*p) {
		    if (*p < SP)
		      printf("^%c",ctl(*p));
		    else
		      printf("%c",*p);
		    p++;
		}
		printf("\n");
	    }
	    if (xmitf)
	      printf(" Transmit Fill: %d (fill character for blank lines)\n",
		   xmitf);
	    else printf(" Transmit Fill: none\n");
	    printf(" Transmit Linefeed: %s\n",
		   xmitl ? "on (send linefeeds too)" : "off");
	    if (xmitp) 
	      printf(" Transmit Prompt: %d (host line end character)\n",xmitp);
	    else printf(" Transmit Prompt: none\n");
	    printf(" Transmit Echo: %s\n", showoff(xmitx));
	    printf(" Transmit Locking-Shift: %s\n", showoff(xmits));
	    printf(" Transmit Pause: %d milliseconds\n", xmitw);
	    break;
#endif /* NOXMIT */

	  case SHMOD:			/* SHOW MODEM */
	    shmdmlin();
	    printf("\n");
#ifdef NETCONN
	    if (!network)
#endif /* NETCONN */
	      shomdm();
	    break;

#ifndef MAC
          case SHDFLT:
	    zsyscmd(PWDCMD);
            break;
#endif /* MAC */

#ifndef NOLOCAL
          case SHESC:
	    printf(" Escape character: Ctrl-%c (ASCII %d, %s)\r\n",
	       ctl(escape), escape, (escape == 127 ? "DEL" : ccntab[escape]));
	    break;
#endif /* NOLOCAL */

#ifndef NODIAL
	  case SHDIA:
	    shmdmlin();
	    printf(", speed: ");
	    if ((zz = ttgspd()) < 0) {
		printf("unknown\n");
	    } else {
		if (zz == 8880) printf("75/1200\n"); else printf("%ld\n",zz);
	    }
	    doshodial();
	    if (carrier == CAR_OFF) s = "off";
	    else if (carrier == CAR_ON) s = "on";
	    else if (carrier == CAR_AUT) s = "auto";
	    else s = "unknown";
	    printf(" Carrier: %s", s);
	    if (carrier == CAR_ON) {
		if (cdtimo) printf(", timeout: %d sec", cdtimo);
		else printf(", timeout: none");
	    }
	    if (local
#ifdef NETCONN
		&& !network
#endif /* NETCONN */
		) {
		printf("\n%s modem signals:\n",ttname);
		shomdm();
	    } else printf("\n");
	    break;
#endif /* NODIAL */

#ifdef CK_LABELED
	case SHLBL:			/* Labeled file info */
	    sholbl();
	    break;
#endif /* CK_LABELED */

	case SHCSE:			/* Character sets */
#ifdef NOCSETS
	    printf(
" Character set translation is not supported in this version of C-Kermit\n");
#else
	    shocharset();
            printf("\n Unknown-Char-Set: %s\n",
		   unkcs ? "Keep" : "Discard");
	    shotcs(tcsl,tcsr);
	    printf("\n");
#endif /* NOCSETS */
	    break;

#ifndef NOFRILLS
	  case SHFEA:			/* Features */
	    shofea();
	    break;
#endif /* NOFRILLS */

#ifdef CK_SPEED
	  case SHCTL:			/* Control-Prefix table */
	    shoctl();
	    break;
#endif /* CK_SPEED */

	  case SHEXI:
	    printf("\n EXIT WARNING %s\n", xitwarn ? "ON" : "OFF");
	    printf(" Current EXIT status: %d\n\n", xitsta);
	    break;

#ifdef OS2
	  case SHPRT:
	    printf("Printer: %s\n", printfile ? printfile : "PRN");
	    break;
#endif /* OS2 */

	  case SHCMD:
	    printf(" Command bytesize: %d bits\n",
		   (cmdmsk == 0377) ? 8 : 7);
#ifdef CK_RECALL
	    printf(" Command recall-buffer-size: %d\n",cm_recall);
#else
	    printf(" Command recall-buffer not available in this version\n");
#endif /* CK_RECALL */
#ifdef CM_RETRY
	    printf(" Command retry: %s\n",cm_retry ? "on" : "off");
#else
	    printf(" Command retry not available in this version\n");
#endif /* CM_RETRY */
	    printf(" Command quoting: %s\n", showoff(cmdgquo()));
#ifdef UNIX
#ifndef NOJC
	    printf(" Suspend: %s\n", showoff(suspend));
#endif /* NOJC */
#endif /* UNIX */
	    break;

	default:
	    printf("\nNothing to show...\n");
	    return(-2);
    }
    return(success = 1);
}

VOID
shmdmlin() {				/* Briefly show modem & line */
#ifndef NODIAL
    int i;
#ifndef MINIDIAL
    extern int tbmodel;
    _PROTOTYP( char * gtbmodel, (void) );
#endif /* MINIDIAL */
#endif /* NODIAL */
    if (local)
      printf(" Line: %s, Modem: ",ttname);
    else
      printf(" Communication device not yet selected with SET LINE\n Modem: ");
#ifndef NODIAL
    for (i = 0; i < nmdm; i++) {
	if (mdmtab[i].kwval == mdmtyp) {
	    if (!strcmp(mdmtab[i].kwd,"courier"))
	      printf("courier or sportster");
	    else
	      printf("%s",mdmtab[i].kwd);
	    break;
	}
    }
#ifndef MINIDIAL
    if (tbmodel) printf(" (%s)",gtbmodel()); /* Telebit model info */
#endif /* MINIDIAL */
#else
    printf("(disabled)");
#endif /* NODIAL */
}
#endif /* NOSHOW */

#ifdef GEMDOS
isxdigit(c) int c; {
    return(isdigit(c) ||
	   (c >= 'a' && c <= 'f') ||
	   (c >= 'A' && c <= 'F'));
}
#endif /* GEMDOS */

#ifndef NOSHOW
#ifndef NOSPL
int					/* SHO MACROS */
shomac(s1, s2) char *s1, *s2; {
    int x, n, pp;
    pp = 0;				/* Parenthesis counter */

    if (!s1)
      return(0);
    else
      printf("\n%s = ",s1);		/* Print blank line and macro name */
    slc++;				/* Count the line */
    n = (int)strlen(s1) + 4;		/* Width of current line */
    if (!s2) s2 = "(null definition)";

    while (x = *s2++) {			/* Loop thru definition */
	if (x == '(') pp++;		/* Treat commas within parens */
	if (x == ')') pp--;		/* as ordinary text */
	if (pp < 0) pp = 0;		/* Outside parens, */
	if (x == ',' && pp == 0) {	/* comma becomes comma-dash-NL. */
	    putchar(',');
	    putchar('-');
	    x = '\n';
	}
	putchar(x);			/* Output the character */
	if (x == '\n') {		/* If it was a newline */
#ifdef UNIX
#ifdef NOSETBUF
	    fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
	    putchar(' ');		/* Indent the next line 1 space */
	    while(*s2 == ' ') s2++;	/* skip past leading blanks */
	    n = 2;			/* restart the character counter */
	    slc++;			/* and increment the line counter. */
	} else if (++n > (cmd_cols - 1)) { /* If line is too wide */
	    putchar('-');		/* output a dash */
	    putchar(NL);		/* and a newline */
#ifdef UNIX
#ifdef NOSETBUF
	    fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
	    n = 1;			/* and restart the char counter */
	    slc++;			/* and increment the line counter */
	}
	if (n < 3 && slc > (cmd_rows - 3)) { /* If new line and screen full */
	    if (!askmore()) return(-1);	/* ask if they want more. */
	    n = 1;			/* They do, start a new line */
	    slc = 0;			/* and restart line counter */
	}
    }
    putchar(NL);			/* End of definition */
    if (++slc > (cmd_rows - 3)) {
	if (!askmore()) return(-1);
	slc = 0;
    }
    return(0);
}
#endif /* NOSPL */
#endif /* NOSHOW */

#ifndef NOSHOW
int
shoatt() {
    printf("Attributes: %s\n", showoff(atcapr));
    if (!atcapr) return(0);
    printf(" Blocksize: %s\n", showoff(atblki));
    printf(" Date: %s\n", showoff(atdati));
    printf(" Disposition: %s\n", showoff(atdisi));
    printf(" Encoding (Character Set): %s\n", showoff(atenci));
    printf(" Length: %s\n", showoff(atleni));
    printf(" Type (text/binary): %s\n", showoff(attypi));
    printf(" System ID: %s\n", showoff(atsidi));
    printf(" System Info: %s\n", showoff(atsysi));
#ifdef STRATUS
    printf(" Format: %s\n", showoff(atfrmi));
    printf(" Creator: %s\n", showoff(atcrei));
    printf(" Account: %s\n", showoff(atacti));
#endif /* STRATUS */
    return(0);
}
#endif /* NOSHOW */

#ifndef NOSPL
/* Evaluate an arithmetic expression. */
/* Code adapted from ev, by Howie Kaye of Columbia U & others. */

static int xerror;
static char *cp;
static long tokval;
static char curtok;
static long expval;

#define LONGBITS (8*sizeof (long))
#define NUMBER 'N'
#define EOT 'E'

/*
 Replacement for strchr() and index(), neither of which seem to be universal.
*/

static char *
#ifdef CK_ANSIC
windex(char * s, char c)
#else
windex(s,c) char *s, c;
#endif /* CK_ANSIC */
/* windex */ {
    while (*s != NUL && *s != c) s++;
    if (*s == c) return(s); else return(NULL);
}

/*
 g e t t o k

 Returns the next token.  If token is a NUMBER, sets tokval appropriately.
*/
static char
gettok() {
    while (isspace(*cp)) cp++ ;
    switch(*cp) {
      case '$':				/* ??? */
      case '+':				/* Add */
      case '-':				/* Subtract or Negate */
      case '@':				/* Greatest Common Divisor */
      case '*':				/* Multiply */
      case '/':				/* Divide */
      case '%':				/* Modulus */
      case '<':				/* Left shift */
      case '>':				/* Right shift */
      case '&':				/* And */
      case '|':				/* Or */
      case '#':				/* Exclusive Or */
      case '~':				/* Not */
      case '^':				/* Exponent */
      case '!':				/* Factorial */
      case '(':				/* Parens for grouping */
      case ')': return(*cp++);		/* operator, just return it */
      case '\n':
      case '\0': return(EOT);		/* end of line, return that */
    }
    if (isxdigit(*cp)) {		/* digit, must be a number */
	char tbuf[80],*tp;		/* buffer to accumulate number */
	int radix = 10;			/* default radix */
	for (tp=tbuf; isxdigit(*cp); cp++)
	  *tp++ = isupper(*cp) ? tolower(*cp) : *cp;
	*tp = '\0';			/* end number */
	switch(isupper(*cp) ? tolower(*cp) : *cp) { /* examine break char */
	  case 'h':
	  case 'x': radix = 16; cp++; break; /* if radix signifier... */
	  case 'o':
	  case 'q': radix = 8; cp++; break;
	  case 't': radix = 2; cp++; break;
	}
	for (tp=tbuf,tokval=0; *tp != '\0'; tp++)  {
	    int dig;
	    dig = *tp - '0';		/* convert number */
	    if (dig > 10) dig -= 'a'-'0'-10;
	    if (dig >= radix) {
		xerror = 1;
		if (cmdlvl == 0)
		  printf("invalid digit '%c' in number\n",*tp);
		return(NUMBER);
	    }
	    tokval = radix*tokval + dig;
	}
	return(NUMBER);
    }
    if (cmdlvl == 0)
      printf("Invalid character '%c' in input\n",*cp);
    xerror = 1;
    cp++;
    return(gettok());
}

static long
#ifdef CK_ANSIC
expon(long x, long y)
#else
expon(x,y) long x,y;
#endif /* CK_ANSIC */
/* expon */ {
    long result = 1;
    int sign = 1;
    if (y < 0) return(0);
    if (x < 0) {
	x = -x;
	if (y & 1) sign = -1;
    }
    while (y != 0) {
	if (y & 1) result *= x;
	y >>= 1;
	if (y != 0) x *= x;
  }
  return(result * sign);
}

/*
 * factor ::= simple | simple ^ factor
 *
 */
_PROTOTYP( static VOID simple, (void) );

static VOID
factor() {
    long oldval;
    simple();
    if (curtok == '^') {
	oldval = expval;
	curtok = gettok();
	factor();
	expval = expon(oldval,expval);
    }
}

/*
 * termp ::= NULL | {*,/,%,&} factor termp
 *
 */
static VOID
termp() {
    while (curtok == '*' || curtok == '/' || curtok == '%' || curtok == '&') {
	long oldval;
	char op;
	op = curtok;
	curtok = gettok();		/* skip past operator */
	oldval = expval;
	factor();
	switch(op) {
	  case '*': expval = oldval * expval; break;
	  case '/':
	    if (expval == 0) expval = -1; /* don't divide by 0 */
	    else expval = oldval / expval; break;
	  case '%': expval = oldval % expval; break;
	  case '&': expval = oldval & expval; break;
	}
    }
}

static long
#ifdef CK_ANSIC
fact(long x)
#else
fact(x) long x;
#endif /* CK_ANSIC */
/* fact */ {				/* factorial */
    long result = 1;
    while (x > 1)
      result *= x--;
    return(result);
}

/*
 * term ::= factor termp
 *
 */
static VOID
term() {
    factor();
    termp();
}

static long
#ifdef CK_ANSIC
gcd(long x, long y)
#else
gcd(x,y) long x,y;
#endif /* CK_ANSIC */
/* gcd */ {				/* Greatest Common Divisor */
    int nshift = 0;
    if (x < 0) x = -x;
    if (y < 0) y = -y;			/* validate arguments */
    if (x == 0 || y == 0) return(x + y);    /* this is bogus */
    
    while (!((x & 1) | (y & 1))) {	/* get rid of powers of 2 */
	nshift++;
	x >>= 1;
	y >>= 1;
    }
    while (x != 1 && y != 1 && x != 0 && y != 0) {
	while (!(x & 1)) x >>= 1;	/* eliminate unnecessary */
	while (!(y & 1)) y >>= 1;	/* powers of 2 */
	if (x < y) {			/* force x to be larger */
	    long t;
	    t = x;
	    x = y;
	    y = t;
	}
	x -= y;
    }
    if (x == 0 || y == 0) return((x + y) << nshift); /* gcd is non-zero one */
    else return((long) 1 << nshift);	/* else gcd is 1 */
}

/*
 * exprp ::= NULL | {+,-,|,...} term exprp
 *
 */
static VOID
exprp() {
    while (windex("+-|<>#@",curtok) != NULL) {
	long oldval;
	char op;
	op = curtok;
	curtok = gettok();		/* skip past operator */
	oldval = expval;
	term();
	switch(op) {
	  case '+' : expval = oldval + expval; break;
	  case '-' : expval = oldval - expval; break;
	  case '|' : expval = oldval | expval; break;
	  case '#' : expval = oldval ^ expval; break;
	  case '@' : expval = gcd(oldval,expval); break;
	  case '<' : expval = oldval << expval; break;
	  case '>' : expval = oldval >> expval; break;
	}
    }
}

/*
 * expr ::= term exprp
 *
 */
static VOID
expr() {
    term();
    exprp();
}

static long
xparse() {
    curtok = gettok();
    expr();
#ifdef COMMENT
    if (curtok == '$') {
	curtok = gettok();
	if (curtok != NUMBER) {
	    if (cmdlvl == 0) printf("illegal radix\n");
	    return(0);
	}
	curtok = gettok();
    }
#endif /* COMMENT */
    if (curtok != EOT) {
	if (cmdlvl == 0)
	  printf("extra characters after expression\n");
	xerror = 1;
    }
    return(expval);
}

char *
evala(s) char *s; {
    char *p;				/* Return pointer */
    long v;				/* Numeric value */

    xerror = 0;				/* Start out with no error */
    cp = s;				/* Make the argument global */
    v = xparse();			/* Parse the string */
    p = numbuf;				/* Convert long number to string */
    sprintf(p,"%ld",v);
    return(xerror ? "" : p);		/* Return empty string on error */
}

/*
 * simplest ::= NUMBER | ( expr )
 *
 */
static VOID
simplest() {
    if (curtok == NUMBER) expval = tokval;
    else if (curtok == '(') {
	curtok = gettok();		/* skip over paren */
	expr();
	if (curtok != ')') {
	    if (cmdlvl == 0) printf("missing right parenthesis\n");
	    xerror = 1;
	}
    }
    else {
	if (cmdlvl == 0) printf("operator unexpected\n");
	xerror = 1;
    }
    curtok = gettok();
}

/*
 * simpler ::= simplest | simplest !
 *
 */
static VOID
simpler() {
    simplest();
    if (curtok == '!') {
	curtok = gettok();
	expval = fact(expval);
    }
}

/*
 * simple ::= {-,~} simpler | simpler
 *
 */

static VOID
simple() {
    if (curtok == '-' || curtok == '~') {
	int op = curtok;
	curtok = gettok();		/* skip over - sign */
	simpler();			/* parse the factor again */
	expval = op == '-' ? -expval : ~expval;
    } else simpler();
}
#endif /* NOSPL */

#ifndef NOSPL
/*  D C L A R R A Y  --  Declare an array  */

int					/* Declare an array of size n */
#ifdef CK_ANSIC
dclarray(char a, int n)
#else
dclarray(a,n) char a; int n;
#endif /* CK_ANSIC */
/* dclarray */ {
    char **p; int i, n2;

    if (a > 63 && a < 96) a += 32;	/* Convert to lowercase */
    if (a < 96 || a > 122) return(-1);	/* Verify name */
    a -= 96;				/* Convert name to number */
    if ((p = a_ptr[a]) != NULL) {	/* Delete old array of same name */
	n2 = a_dim[a];
	for (i = 0; i <= n2; i++)	/* First delete its elements */
	  if (p[i]) free(p[i]);
	free(a_ptr[a]);			/* Then the element list */
	a_ptr[a] = (char **) NULL;	/* Remove pointer to element list */
	a_dim[a] = 0;			/* Set dimension at zero. */
    }
    if (n == 0) return(0);		/* If dimension 0, just deallocate. */

    p = (char **) malloc((n+1) * sizeof(char **)); /* Allocate for new array */
    if (p == NULL) return(-1);		/* Check */
    a_ptr[a] = p;			/* Save pointer to member list */
    a_dim[a] = n;			/* Save dimension */
    for (i = 0; i <= n; i++)		/* Initialize members to null */
      p[i] = NULL;
    return(0);
}

/*  A R R A Y N A M  --  Parse an array name  */

/*
  Call with pointer to string that starts with the array reference.
  String may begin with either \& or just &.
  On success,
    Returns letter ID (always lowercase) in argument c,
      which can also be accent grave (` = 96; '@' is converted to grave);
    Dimension or subscript in argument n;
    IMPORTANT: These arguments must be provided by the caller as addresses
    of ints (not chars), for example:
      char *s; int x, y;
      arraynam(s,&x,&y);
  On failure, returns a negative number, with args n and c set to zero.
*/
int
arraynam(ss,c,n) char *ss; int *c; int *n; {
    int i, y, pp;
    char x;
    char *s, *p, *sx, *vnp;
    char vnbuf[VNAML+1];		/* On stack to allow for */
    char ssbuf[VNAML+1];		/* recursive calls... */
    char sxbuf[VNAML+1];

    *c = *n = 0;			/* Initialize return values */
    for (i = 0; i < (int)strlen(ss); i++) /* Check length */
      if (ss[i] == ']')
	break;
    if (i > VNAML) {
	printf("?Array reference too long - %s\n",ss);
	return(-9);
    }
    strncpy(vnbuf,ss,VNAML);
    vnp = vnbuf;
    if (vnbuf[0] == CMDQ && vnbuf[1] == '&') vnp++;
    if (*vnp != '&') {
	printf("?Not an array - %s\n",vnbuf);
	return(-9);
    }
    x = *(vnp + 1);			/* Fold case of array name */
    /* We don't use isupper & tolower here on purpose because these */
    /* would produce undesired effects with accented letters. */
    if (x > 63 && x < 91) x  = *(vnp +1) = x + 32;
    if ((x < 96) || (x > 122) || (*(vnp+2) != '[')) {
	printf("?Invalid format for array name - %s\n",vnbuf);
	return(-9);
    }
    *c = x;				/* Return the array name */
    s = vnp+3;				/* Get dimension */
    p = ssbuf;    
    pp = 1;				/* Bracket counter */
    for (i = 0; i < VNAML && *s != NUL; i++) { /* Copy up to ] */
	if (*s == '[') pp++;
	if (*s == ']' && --pp == 0) break;
	*p++ = *s++;
    }
    if (*s != ']') {
	printf("?No closing bracket on array dimension - %s\n",vnbuf);
	return(-9);
    }
    *p = NUL;				/* Terminate subscript with null */
    p = ssbuf;				/* Point to beginning of subscript */
    sx = sxbuf;				/* Where to put expanded subscript */
    y = VNAML-1;
    zzstring(p,&sx,&y);			/* Convert variables, etc. */
    if (!chknum(sxbuf)) {		/* Make sure it's all digits */
	printf("?Array dimension or subscript must be numeric - %s\n",sxbuf);
	return(-9);
    }
    if ((y = atoi(sxbuf)) < 0) {
        if (cmflgs == 0) printf("\n");
        printf("?Array dimension or subscript must be positive or zero - %s\n",
	       sxbuf);
	return(-9);
    }
    *n = y;				/* Return the subscript or dimension */
    return(0);
}

int
chkarray(a,i) int a, i; {		/* Check if array is declared */
    int x;				/* and if subscript is in range */
    if (a == 64) a = 96;		/* Convert atsign to grave accent */
    x = a - 96;				/* Values must be in range 96-122 */
    if (x < 0 || x > 26) return(-2);	/* Not in range */
    if (a_ptr[x] == NULL) return(-1);	/* Not declared */
    if (i > a_dim[x]) return(-2);	/* Declared but out of range. */
    return(a_dim[x]);			/* All ok, return dimension */
}

char *
arrayval(a,i) int a, i; {		/* Return value of \&a[i] */
    int x; char **p;			/* (possibly NULL) */

    if (a == 64) a = 96;		/* Convert atsign to grave accent */
    x = a - 96;				/* Values must be in range 96-122 */
    if (x < 0 || x > 26) return(NULL);	/* Not in range */
    if ((p = a_ptr[x]) == NULL)		/* Array not declared */
      return(NULL);
    if (i > a_dim[x])			/* Subscript out of range. */
      return(NULL);
    return(p[i]);			/* All ok, return pointer to value. */
}
#endif /* NOSPL */

#ifndef NOSPL
/*  P A R S E V A R  --  Parse a variable name or array reference.  */
/*
 Call with:
   s  = pointer to candidate variable name or array reference.
   *c = address of integer in which to return variable ID.
   *i = address of integer in which to return array subscript.
 Returns:
   -2:  syntax error in variable name or array reference.
    1:  successful parse of a simple variable, with ID in c.
    2:  successful parse of an array reference, w/ID in c and subscript in i.
*/
int
parsevar(s,c,i) char *s; int *c, *i; {
    char *p;
    int x,y,z;

    p = s;
    if (*s == CMDQ) s++;		/* Point after backslash */

    if (*s != '%' && *s != '&') {	/* Make sure it's % or & */
	printf("?Not a variable name - %s\n",p);
	return(-9);
    }
    if ((int)strlen(s) < 2) {
	printf("?Incomplete variable name - %s\n",p);
	return(-9);
    }
    if (*s == '%' && *(s+2) != '\0') {
	printf("?Only one character after '%%' in variable name, please\n");
	return(-9);
    }
    if (*s == '&' && *(s+2) != '[') {
	printf("?Array subscript expected - %s\n",p);
	return(-9);
    }
    if (*s == '%') {			/* Simple variable. */
	y = *(s+1);			/* Get variable ID letter/char */
	if (isupper(y)) y -= ('a'-'A');	/* Convert upper to lower case */
	*c = y;				/* Set the return values. */
	*i = -1;			/* No array subscript. */
	return(1);			/* Return 1 = simple variable */
    }
    if (*s == '&') {			/* Array reference. */
	if (arraynam(s,&x,&z) < 0) { /* Go parse it. */
	    printf("?Invalid array reference - %s\n",p);
	    return(-9);
	}
	if (chkarray(x,z) < 0) {	/* Check if declared, etc. */
	    printf("?Array not declared or subscript out of range\n");
	    return(-9);
	}
	*c = x;				/* Return array letter */
	*i = z;				/* and subscript. */
	return(2);
    }    
    return(-2);				/* None of the above. */
}

#define VALN 20

/* Get the numeric value of a variable */
/*
  Call with pointer to variable name, pointer to int for return value.
  Returns:
    0 on success with second arg containing the value.
   -1 on failure (bad variable syntax, variable not defined or not numeric).
*/
int
varval(s,v) char *s; int *v; {
    char valbuf[VALN+1];		/* s is pointer to variable name */
    char *p;
    int y;

    p = valbuf;				/* Expand variable into valbuf. */
    y = VALN;
    if (zzstring(s,&p,&y) < 0) return(-1);
    p = valbuf;				/* Make sure value is numeric */
    if (!chknum(p)) return(-1);
    *v = atoi(p);			/* Convert numeric string to int */
    return(0);    
}

/* Increment or decrement a variable */
/* Returns -1 on failure, 0 on success, with 4th argument set to result */

int
incvar(s,x,z,r) char *s; int x, z, *r; { /* Increment a numeric variable */
    char valbuf[VALN+1];		/* s is pointer to variable name */
					/* x is amount to increment by */
    int n;				/* z != 0 means add */
					/* z = 0 means subtract */

    if (varval(s,&n) < 0)		/* Convert numeric string to int */
      return(-1);
    if (z)				/* Increment it by the given amount */
      n += x;
    else				/* or decrement as requested. */
      n -= x;
    sprintf(valbuf,"%d",n);		/* Convert back to numeric string */
    addmac(s,valbuf);			/* Replace old variable */
    *r = n;				/* Return the integer value */
    return(0);
}
#endif /* NOSPL */

/* Functions moved here from ckuusr.c to even out the module sizes... */

#ifndef NOSPL				/* Need xwords() function to break */
#define XWORDS				/* string up into words. */
#endif /* NOSPL */
#ifndef NODIAL
#ifndef XWORDS
#define XWORDS
#endif /* XWORDS */
#endif /* NODIAL */

#ifdef XWORDS
/*
  Breaks string s up into a list of up to max words.
  Pointers to each word go into the array list[].
  If list is NULL, then they are added to the macro table.
*/

VOID
xwords(s,max,list) char *s; int max; char *list[]; {
    char *p;
    int b, k, y, z;
#ifndef NOSPL
    int macro;
    macro = (list == NULL);
    debug(F101,"xwords macro","",macro);
#endif /* NOSPL */

    p = s;				/* Pointer to beginning of string */
    b = 0;				/* Flag for outer brace removal */
    k = 0;				/* Flag for in-word */
    y = 0;				/* Brace nesting level */
    z = 0;				/* Argument counter, 0 thru max */

    while (1) {				/* Go thru argument list */
	if (!s || (*s == '\0')) {	/* No more characters? */
	    if (k != 0) {
		if (z == max) break;	/* Only go up to max. */
		z++;			/* Count it. */
#ifndef NOSPL
		if (macro) {
		    varnam[1] = z + '0'; /* Assign last argument */
		    addmac(varnam,p);
		} else
#endif /* NOSPL */
		  list[z] = p;
		break;			/* And get out. */
	    } else break;
	} 
	if (k == 0 && (*s == SP || *s == HT)) { /* Eat leading blanks */
	    s++;
	    continue;
	} else if (*s == '{') {		/* An opening brace */
	    if (k == 0 && y == 0) {	/* If leading brace */
		p = s+1;		/* point past it */
		b = 1;			/* and flag that we did this */
	    }
	    k = 1;			/* Flag that we're in a word */
	    y++;			/* Count the brace. */
	} else if (*s == '}') {		/* A closing brace. */
	    y--;			/* Count it. */
	    if (y == 0 && b != 0) {	/* If it matches the leading brace */
		*s = SP;		/* change it to a space */
		b = 0;			/* and we're not in braces any more */
	    } else if (y < 0) k = 1;	/* otherwise just start a new word. */
	} else if (*s != SP && *s != HT) { /* Nonspace means we're in a word */
	    if (k == 0) p = s;		/* Mark the beginning */
	    k = 1;			/* Set in-word flag */
	}
	/* If we're not inside a braced quantity, and we are in a word, and */
	/* we have hit whitespace, then we have an argument to assign. */
	if ((y < 1) && (k != 0) && (*s == SP || *s == HT)) { 
	    *s = '\0';			/* terminate the arg with null */
	    k = 0;			/* say we're not in a word any more */
	    y = 0;			/* start braces off clean again */
	    if (z == max) break;	/* Only go up to max. */
	    z++;			/* count this arg */
#ifndef NOSPL
	    if (macro) {
		varnam[1] = z + '0';	/* compute its name */
		addmac(varnam,p);	/* add it to the macro table */
	    } else
#endif /* NOSPL */
	      list[z] = p;
	    p = s+1;
	}
	s++;				/* Point past this character */
    }
    if ((z == 0) && (y > 1)) {		/* Extra closing brace(s) at end */
	z++;
#ifndef NOSPL
	if (macro) {
	    varnam[1] = z + '0';	/* compute its name */
	    addmac(varnam,p);		/* Add rest of line to last arg */
	} else
#endif /* NOSPL */
	  list[z] = p;
    }
#ifndef NOSPL
    if (macro) macargc[maclvl] = z + 1;	/* Set \v(argc) variable */
#endif /* NOSPL */
    return;
}
#endif /* XWORDS */

#ifndef NOSPL
/* D O D O  --  Do a macro */

/*
  Call with x = macro table index, s = pointer to arguments.
  Returns 0 on failure, 1 on success.
*/

int
dodo(x,s) int x; char *s; {
    int y;

    debug(F101,"dodo maclvl","",maclvl);
    if (++maclvl > MACLEVEL) {		/* Make sure we have storage */
        debug(F101,"dodo maclvl too deep","",maclvl);
	--maclvl;
	printf("Macros nested too deeply\n");
	return(0);
    }
    macp[maclvl] = mactab[x].mval;	/* Point to the macro body */ 
    macx[maclvl] = mactab[x].mval;	/* Remember where the beginning is */
    debug(F111,"do macro",macp[maclvl],maclvl);

    cmdlvl++;				/* Entering a new command level */
    if (cmdlvl > CMDSTKL) {		/* Too many macros + TAKE files? */
        debug(F101,"dodo cmdlvl too deep","",cmdlvl);
	cmdlvl--;
	printf("?TAKE files and DO commands nested too deeply\n");
	return(0);
    }
#ifdef VMS
    conres();				/* So Ctrl-C, etc, will work. */
#endif /* VMS */
    ifcmd[cmdlvl] = 0;
    iftest[cmdlvl] = 0;
    count[cmdlvl] = count[cmdlvl-1]; /* Inherit COUNT from previous level */
    intime[cmdlvl] = intime[cmdlvl-1];	/* Inherit previous INPUT TIMEOUT */
    inpcas[cmdlvl] = inpcas[cmdlvl-1];	/*   and INPUT CASE */
    takerr[cmdlvl] = takerr[cmdlvl-1];	/*   and TAKE ERROR */
    merror[cmdlvl] = merror[cmdlvl-1];	/*   and MACRO ERROR */
    cmdstk[cmdlvl].src = CMD_MD;	/* Say we're in a macro */
    cmdstk[cmdlvl].lvl = maclvl;	/* and remember the macro level */
    mrval[maclvl] = NULL;		/* Initialize return value */

    debug(F110,"do macro",mactab[x].kwd,0);

/* Clear old %0..%9 arguments */

    addmac("%0",mactab[x].kwd);		/* Define %0 = name of macro */
    varnam[0] = '%';
    varnam[2] = '\0';
    for (y = 1; y < 10; y++) {		/* Clear args %1..%9 */
	varnam[1] = y + '0';
	delmac(varnam);
    }	

/* Assign the new args one word per arg, allowing braces to group words */

    xwords(s,9,NULL);
    return(1);
}

/* Insert "literal" quote around each comma-separated command to prevent */
/* its premature expansion.  Only do this if object command is surrounded */
/* by braces. */

static char* flit = "\\flit(";

int
litcmd(src,dest) char **src, **dest; {
    int bc = 0, pp = 0;
    char *s, *lp, *ss;

    s = *src;
    lp = *dest;

    while (*s == SP) s++;		/* strip extra leading spaces */
    if (*s == '{') {

        pp = 0;				/* paren counter */
	bc = 1;				/* count leading brace */
	*lp++ = *s++;			/* copy it */
	while (*s == SP) s++;		/* strip interior leading spaces */
	ss = flit;			/* point to "\flit(" */
	while (*lp++ = *ss++) ;		/* copy it */
	lp--;				/* back up over null */
	while (*s) {			/* go thru rest of text */
	    ss = flit;			/* point back to start of "\flit(" */
	    if (*s == '{') bc++;	/* count brackets */
	    if (*s == '(') pp++;	/* and parens */
	    if (*s == ')') pp--;
	    if (*s == '}') {		/* Closing brace. */
		if (--bc == 0) {	/* Final one? */
		    *lp++ = ')';	/* Add closing paren for "\flit()" */
		    *lp++ = *s++;
		    break;
		}
	    }
	    if (*s == ',' && pp == 0) {	/* comma not inside of parens */
		*lp++ = ')';		/* closing ) of \flit( */
		*lp++ = ',';		/* insert the comma */
		while (*lp++ = *ss++) ;	/* start new "\flit(" */
		lp--;			/* back up over null */
		s++;			/* skip over comma in source string */
		while (*s++ == SP);	/* eat leading spaces again. */
		s--;			/* back up over nonspace */
		continue;
	    }
            *lp++ = *s++;		/* Copy anything but comma here. */
        }
	*lp = NUL;
    } else {				/* No brackets around, */
	while (*lp++ = *s++) ;		/* just copy. */
	lp--;
    }
    *src = s;
    *dest = lp;
    if (bc) return(-1);
    else return(0);
}
#endif /* NOSPL */

int
docd() {				/* Do the CD command */
    int x;
    extern int quiet;
    char *s;
#ifdef MAC
    char temp[34];
#endif /* MAC */

#ifdef GEMDOS
    if ((x = cmdir("Name of local directory, or carriage return",homdir,&s,
		   NULL)) < 0 )
      return(x);
#else
#ifdef OS2
    if ((x = cmdir("Name of PC disk and/or directory,\n\
       or press the Enter key for the default",homdir,&s,
		   xxstring)) < 0 )
      return(x);
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
    if ((x = cmdir("Name of local directory, or carriage return",homdir,&s,
		   xxstring)) < 0 )
      return(x);
#endif /* MAC */
#endif /* OS2 */
#endif /* GEMDOS */
#ifndef MAC
    if (x == 2) {
	printf("?Wildcards not allowed in directory name\n");
	return(-9);
    }
#endif /* MAC */
    strcpy(line,s);			/* Make a safe copy */
    s = line;
#ifndef MAC
    if ((x = cmcfm()) < 0)		/* Get confirmation */
      return(x);
#endif /* MAC */
#ifdef datageneral
    x = strlen(line);			/* homdir ends in colon, */
    if (line[x-1] == ':')		/* and "dir" doesn't like that... */
      line[x-1] = NUL;
#endif /* datageneral */

#ifdef MAC
    cwdf = 1;
    if (! zchdir(s)) {
	cwdf = 0;
	if (*s != ':') {		/* If it failed, */
	    char *p;			/* supply leading colon */
	    p = malloc((int)strlen(s) + 2); /* and try again... */
	    if (p) {
		strcpy(p,":");
		strcat(p,s);
		if (zchdir(p))
		  cwdf = 1;
		free(p);
	    }
	}
    }
    if (!cwdf)
      perror(s);
#else
    if (! zchdir(s)) {
	cwdf = 0;
	perror(s);
    } else cwdf = 1;
#endif /* MAC */
    if (!quiet)
      printf("%s\n", zgtdir());
    return(cwdf);
}

VOID
fixcmd() {			/* Fix command parser after interruption */
    dostop();			/* Back to top level (also calls conint()). */
    bgchk();			/* Check background status */
    if (*psave) {		/* If old prompt saved, */
	cmsetp(psave);		/* restore it. */
	*psave = NUL;
    }
    success = 0;		/* Tell parser last command failed */
}

VOID
prtopt(s) char *s; {			/* Print an option */
    static int x = 0;			/* (used by SHOW VER) */
    int y;				/* Does word wrap. */
    if (!s) { x = 0; return; }		/* Call with null pointer to */
    y = (int)strlen(s) + 1;		/* reset horizontal position. */
    x += y;
    if (x > ((tt_cols > 40) ? (tt_cols - 1) : 79)) {
	printf("\n %s",s);
	x = y;
    } else printf(" %s",s);
}

#endif /* NOICP */
