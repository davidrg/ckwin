#include "ckcsym.h"

#ifndef NOICP

/*  C K U U S 5 --  "User Interface" for C-Kermit, part 5  */
 
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

#ifdef OS2
#include "ckoetc.h"
#ifndef NT
#define INCL_NOPM
#define INCL_VIO /* Needed for ckocon.h */
#include <os2.h>
#else /* NT */
#define APIRET ULONG
extern int DialerHandle;
extern int StartedFromDialer;
#endif /* NT */
#include "ckouni.h"
#include "ckocon.h"
#include "ckokey.h"
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)
extern int cursor_save ;
extern bool cursorena[] ;
#endif /* OS2 */

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

extern int carrier, cdtimo, local, quiet, backgrd, bgset, sosi, suspend,
  displa, binary, deblog, escape, xargs, flow, cmdmsk, bye_active,
  duplex, ckxech, pktlog, seslog, tralog, what, protocol,
  keep, warn, tlevel, cwdf, nfuncs, unkcs, msgflg, remfile, rempipe, remappd,
  mdmtyp, zincnt, cmask, rcflag, success, xitsta, pflag, lf_opts, tnlm, tn_nlm,
  xfrcan, xfrchr, xfrnum, xitwarn, debses, pktpaus, moving, autodl, xaskmore;

extern int ngetpath, exitonclose;
extern char * getpath[];

#ifndef NOSERVER
  extern char * x_user, * x_passwd, * x_acct;
#endif /* NOSERVER */

#ifndef NOSPL
extern int cfilef, DeleteStartupFile;
extern char cmdfil[];

struct localvar * localhead[MACLEVEL];
struct localvar * localtail = NULL;
struct localvar * localnext = NULL;

_PROTOTYP( static char gettok,   (void) );
_PROTOTYP( static VOID factor,   (void) );
_PROTOTYP( static VOID term,     (void) );
_PROTOTYP( static VOID termp,    (void) );
_PROTOTYP( static VOID exprp,    (void) );
_PROTOTYP( static VOID expr,     (void) );
_PROTOTYP( static VOID simple,   (void) );
_PROTOTYP( static VOID simpler,  (void) );
_PROTOTYP( static VOID simplest, (void) );
_PROTOTYP( static long xparse,   (void) );
#endif /* NOSPL */

#ifdef MAC
char * ckprompt = "Mac-Kermit>";	/* Default prompt for Macintosh */
#else  /* Not MAC */
#ifdef NOSPL
#ifdef NT
char * ckprompt = "K-95> ";		/* Default prompt for Win32 */
#else
char * ckprompt = "C-Kermit>";
#endif /* NT */
#else  /* NOSPL */
#ifdef NT
char * ckprompt = "[\\v(dir)] K-95> ";	/* Default prompt for Windows NT */
#else
#ifdef VMS
char * ckprompt = "\\v(dir) C-Kermit>";	/* Default prompt VMS */
#else
char * ckprompt = "[\\v(dir)] C-Kermit>";	/* Default prompt for others */
#endif /* VMS */
#endif /* NT */
#endif /* NOSPL */
#endif /* MAC */

#ifndef CCHMAXPATH
#define CCHMAXPATH 257
#endif /* CCHMAXPATH */
char inidir[CCHMAXPATH] = { NUL, NUL };	/* Directory INI file executed from */

#ifdef TCPSOCKET 
extern int tn_b_nlm;			/* TELNET BINARY newline mode */
extern int me_binary, u_binary;
#endif /* TCPSOCKET */

#ifndef NOKVERBS
extern struct keytab kverbs[];		/* Table of \Kverbs */
extern int nkverbs;			/* Number of \Kverbs */
#endif /* NOKVERBS */

#ifndef NOPUSH
extern int nopush;
#endif /* NOPUSH */

#ifdef CK_RECALL
extern int cm_recall;
#endif /* CK_RECALL */

extern char *ccntab[];
extern char *printfile;
extern int printpipe;

#ifdef OS2
_PROTOTYP (int os2getcp, (void) );
_PROTOTYP (int os2getcplist, (int *, int) );
#ifdef OS2MOUSE
extern int tt_mouse;
#endif /* OS2MOUSE */
extern int tt_update, tt_updmode, tt_font, updmode;
extern struct keytab termfont[];
extern int ntermfont;
extern unsigned char colornormal, colorunderline, colorstatus,
    colorhelp, colorselect, colorborder, colorgraphic, colordebug,
    colorreverse;
extern int priority;
extern struct keytab prtytab[];
extern int nprty;
char * cmdmac = NULL;
#endif /* OS2 */

#ifdef VMS
_PROTOTYP (int zkermini, (char *, int, char *) );
#endif /* VMS */

#ifndef NOFRILLS
extern int en_asg, en_cwd, en_cpy, en_del, en_dir, en_fin, en_bye, en_ret,
  en_get, en_hos, en_que, en_ren, en_sen, en_set, en_spa, en_typ, en_who,
  en_mai, en_pri;
#endif /* NOFRILLS */
extern long vernum;
extern int srvtim, srvdis, inecho, insilence, inbufsize, nvars, verwho;
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
extern int nmdm, dirline;
extern struct keytab mdmtab[];
#endif /* NODIAL */

#ifdef NETCONN
extern int network, ttnproto;
#endif /* NETCONN */

#ifdef OS2
/* SET TERMINAL items... */
extern int tt_type, tt_arrow, tt_keypad, tt_wrap, tt_answer, tt_scrsize[];
extern int tt_bell, tt_roll[], tt_ctstmo, tt_cursor, tt_pacing;
extern char answerback[];  
#ifdef COMMENT
extern int tt_hide;
#endif /* COMMENT */
extern struct tt_info_rec tt_info[];	/* Indexed by terminal type */
extern int max_tt;
#endif /* OS2 */

#ifdef CK_TTGWSIZ
_PROTOTYP( int ttgwsiz, (void) );
#endif /* CK_TTGWSIZ */
_PROTOTYP( VOID shotrm, (void) );
_PROTOTYP( VOID shofea, (void) );

#ifdef OS2
extern int tt_rows[], tt_cols[];
#else /* OS2 */
extern int tt_rows, tt_cols;
#endif /* OS2 */
int cmd_rows = 24, cmd_cols = 80;

#ifdef CK_TMPDIR
extern int f_tmpdir;			/* Directory changed temporarily */
extern char savdir[];			/* Temporary directory */
#endif /* CK_TMPDIR */

#ifndef NOLOCAL
extern int tt_crd, tt_escape;
#endif /* NOLOCAL */

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
extern CHAR sstate, feol;
extern int cmflgs, techo, repars, ncmd;
extern struct keytab cmdtab[];

#ifndef NOSETKEY
KEY *keymap;
#ifndef OS2
#define mapkey(x) keymap[x]
#endif /* OS2 */
MACRO *macrotab;
_PROTOTYP( VOID shostrdef, (CHAR *) );
#endif /* NOSETKEY */

#ifndef NOSPL
extern struct mtab *mactab;
extern struct keytab vartab[], fnctab[];
extern int cmdlvl, maclvl, nmac, mecho;
#endif /* NOSPL */

FILE *tfile[MAXTAKE];			/* TAKE file stack */
char *tfnam[MAXTAKE];
int tfline[MAXTAKE];

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

extern long ck_alarm;
extern char alrm_date[], alrm_time[];

extern int query;			/* QUERY active */

/* Local declarations */

static int nulcmd = 0;			/* Flag for next cmd to be ignored */

/* Definitions for predefined macros */

/* First, the single-line macros, installed with addmac()... */

/* IBM-LINEMODE macro */
char *m_ibm = "set parity mark, set dupl half, set handsh xon, set flow none";

/* FATAL macro */
char *m_fat = "if def \\%1 echo \\%1, if not = \\v(local) 0 hangup, stop 1";

#ifdef CK_SPEED
#ifdef pdp11
char *m_fast = "set win 3, set rec pack 1024, set prefix minimal";
#else
#ifdef BIGBUFOK
char *m_fast = "set win 20, set rec pack 4096, set prefix minimal";
#else
char *m_fast = "set win 4, set rec pack 2200, set prefix minimal";
#endif /* BIGBUFOK */
#endif /* pdp11 */
#ifdef pdp11
char *m_cautious = "set win 2, set rec pack 512, set prefixing cautious";
#else
char *m_cautious = "set win 4, set rec pack 1000, set prefixing cautious";
#endif /* pdp11 */
char *m_robust = "set win 1, set rec pack 90, set prefixing all";
#else
#ifdef BIGBUFOK
char *m_fast = "set win 20, set rec pack 4096";
#else
char *m_fast = "set win 4, set rec pack 2200";
#endif /* BIGBUFOK */
char *m_cautious = "set win 4, set rec pack 1000";
char *m_robust = "set win 1, set rec pack 90";
#endif /* CK_SPEED */

/* Now the multiline macros, defined with addmmac()... */

/* FOR macro */
char *for_def[] = { "_assign _for\\v(cmdlevel) { _getargs,",
"define \\\\\\%1 \\feval(\\%2),:_..top,if \\%5 \\\\\\%1 \\%3 goto _..bot,",
"\\%6,:_..inc,incr \\\\\\%1 \\%4,goto _..top,:_..bot,_putargs},",
"def break goto _..bot, def continue goto _..inc,",
"do _for\\v(cmdlevel) \\%1 \\%2 \\%3 \\%4 { \\%5 },_assign _for\\v(cmdlevel)",
""};

/* WHILE macro */
char *whil_def[] = { "_assign _whi\\v(cmdlevel) {_getargs,",
":_..inc,\\%1,\\%2,goto _..inc,:_..bot,_putargs},",
"_def break goto _..bot, _def continue goto _..inc,",
"do _whi\\v(cmdlevel),_assign _whi\\v(cmdlevel)",
""};

/* SWITCH macro */
char *sw_def[] = { "_assign _sw_\\v(cmdlevel) {_getargs,",
"_forward \\%1,\\%2,:_..bot,_putargs},_def break goto _..bot,",
"do _sw_\\v(cmdlevel),_assign _sw_\\v(cmdlevel)",
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

/* Dimension must be 'z' - ARRAYBASE + 1 */

char **a_ptr[28];			/* Array pointers, for arrays a-z */
int a_dim[28];				/* Dimensions for each array */

char * inpbuf = NULL;			/* Buffer for INPUT and REINPUT */
extern char * inpbp;			/* Global/static pointer to it  */
char inchar[2] = { NUL, NUL };		/* Last character that was INPUT */
int  incount = 0;			/* INPUT character count */
extern int instatus;			/* INPUT status */
static char * i_text[] = {		/* INPUT status text */
    "success", "timeout", "interrupted", "internal error", "i/o error"
};

char lblbuf[LBLSIZ];			/* Buffer for labels */
#else
int takerr[MAXTAKE];
#endif /* NOSPL */

int pacing = 0;				/* OUTPUT pacing */

#ifdef DCMDBUF
char *line;				/* Character buffer for anything */
char *tmpbuf;
#else
char line[LINBUFSIZ];
char tmpbuf[TMPBUFSIZ];			/* Temporary buffer */
#endif /* DCMDBUF */
char *tp;				/* Temporary buffer pointer */

#ifdef CK_APC				/* Application Program Command (APC) */
int apcactive = APC_INACTIVE;
int apcstatus = APC_OFF;		/* OFF by default everywhere */
#ifdef DCMDBUF
char *apcbuf;
#else
char apcbuf[APCBUFLEN];
#endif /* DCMDBUF */
#endif /* CK_APC */

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
_PROTOTYP( VOID freelocal, (int) );
_PROTOTYP( static long expon, (long, long) );
_PROTOTYP( static long gcd, (long, long) );
_PROTOTYP( static long fact, (long) );

int			/* Initialize macro data structures. */
macini() {		/* Allocate mactab and preset the first element. */
    int i;
    if (!(mactab = (struct mtab *) malloc(sizeof(struct mtab) * MAC_MAX)))
      return(-1);
    mactab[0].kwd = NULL;
    mactab[0].mval = NULL;
    mactab[0].flgs = 0;
    for (i = 0; i < MACLEVEL; i++)
      localhead[i] = NULL;
    return(0);
}
#endif /* NOSPL */

/*  C M D I N I  --  Initialize the interactive command parser  */
 
static int cmdinited = 0;

VOID
cmdini() {
    int i, x, y, z, n;
#ifndef NOSPL
/*
  On stack to allow recursion!
*/
    char vnambuf[VNAML];		/* Buffer for variable names */
#endif /* NOSPL */

    if (cmdinited)
      return;

#ifndef NOSETKEY			/* Allocate & initialize the keymap */
    if (!(keymap = (KEY *) malloc(sizeof(KEY)*KMSIZE)))
      fatal("cmdini: no memory for keymap");
    if (!(macrotab = (MACRO *) malloc(sizeof(MACRO)*KMSIZE)))
      fatal("cmdini: no memory for macrotab");
    for (i = 0; i < KMSIZE; i++) {
       keymap[i] = (KEY) i;
       macrotab[i] = NULL;
    }
#endif /* NOSETKEY */

#ifndef NOSPL
    if (!inpbuf) {
	if (!(inpbuf = (char *) malloc(INPBUFSIZ+1)))
	  fatal("cmdini: no memory for INPUT buffer");
    }
    for (x = 0; x < INPBUFSIZ; x++)
      inpbuf[x] = NUL;
    inpbp = inpbuf;
    inbufsize = INPBUFSIZ;
#endif /* NOSPL */

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
    cmdlvl = 0;				/* Initialize the command stack */
    cmdstk[cmdlvl].src = CMD_KB;	/* Source is console */
    cmdstk[cmdlvl].lvl = 0;		/* Level is 0 */
    cmdstk[cmdlvl].ccflgs = 0;		/* No flags */
#endif /* NOSPL */

    tlevel = -1;			/* Take file level = keyboard */
    for (i = 0; i < MAXTAKE; i++)	/* Initialize command file names */
      tfnam[i] = NULL;

#ifdef NT
/*  We tell the difference between Windows NT and 95 at runtime. */
    if (isWin95())
#ifdef NOSPL
      ckprompt = "K-95> ";		/* Default prompt for Windows 95 */
#else
      ckprompt = "[\\v(dir)] K-95> ";
#endif /* NOSPL */
#endif /* NT */
    cmsetp(ckprompt);

#ifndef NOSPL
    initmac();				/* Initialize macro table */
/* Add one-line macros */
    addmac("ibm-linemode",m_ibm);	/* Add built-in macros. */
    addmac("fatal",m_fat);		/* FATAL macro  */
    y = addmac("fast",m_fast);		/* FAST macro   */
    addmac("cautious",m_cautious);	/* CAUTIOUS macro   */
    addmac("robust",m_robust);		/* ROBUST macro */

/* Add multiline macros */
    addmmac("_forx",for_def);		/* FOR macro. */
    addmmac("_xif",xif_def);		/* XIF macro. */
    addmmac("_while",whil_def);		/* WHILE macro. */
    addmmac("_switx",sw_def);		/* SWITCH macro. */
/* Predefine the macro argument vector */
    strcpy(vnambuf,"\\&_[10]"); 	/* Macro argument vector */
    y = arraynam(vnambuf,&x,&z);	/* goes in array \&_[] */
    if (y > -1) {
	dclarray((char)x,z);		/* Declare the array */
	for (i = 0; i < 10; i++) {	/* Fill it */
	    sprintf(vnambuf,"\\&_[%d]",i); /* with NULL pointers */
	    addmac(vnambuf,NULL);
	}
    }
    *vnambuf = NUL;
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

#ifdef CK_TTGWSIZ
#ifdef OS2
    ttgcwsz();
#else /* OS2 */
    if (ttgwsiz() > 0) {
	if (tt_rows > 0 && tt_cols > 0) {
	    cmd_rows = tt_rows;
	    cmd_cols = tt_cols;
    	}
    }
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

/* Get our home directory now.  This needed in lots of places. */

    homdir = zhome();
    cmdinited = 1;
}

VOID
doinit() {
#ifdef OS2
    char * ptr = 0;
    extern int initvik;
#endif /* OS2 */

    if (!cmdinited)
      cmdini();

#ifdef MAC
    return;				/* Mac Kermit has no init file */

#else /* !MAC */

#ifdef OS2
    keymapinit();
    keynaminit();
#ifdef OS2MOUSE
    mousemapinit(-1,-1);
#endif /* OS2MOUSE */
    initvik = 1;
#endif /* OS2 */

/* If skipping init file ('-Y' on Kermit command line), return now. */

    if (noinit) {
	kermrc[0] = '\0'; 
	inidir[0] = '\0'; 
/*
  But returning from here results in inidir[] never being set to anything.
  Instead it should be set to wherever the init file *would* have been
  executed from.  So this bit of code should be removed, and then we should
  sprinkle "if (noinit)" tests throughout the following code until we have
  set inidir[], and then return without actually taking the init file.
*/
	return;
    }

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
        char * env = 0;
#ifdef NT
	env = getenv("K95.INI");
#else
	env = getenv("K2.INI");
#endif /* NT */
	if (!env)
	  env = getenv("CKERMIT.INI");
	if (!env)
	  env = getenv("CKERMIT_INI");
        line[0] = '\0';

	if (env)
	  strcpy(line,env);
	if (line[0] == 0)
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
	tfline[tlevel] = 0;
	if (tfnam[tlevel] = malloc(strlen(line)+1))
	  strcpy(tfnam[tlevel],line);
#ifndef NOSPL
	cmdlvl++;
	cmdstk[cmdlvl].src = CMD_TF;
	cmdstk[cmdlvl].lvl = tlevel;
	cmdstk[cmdlvl].ccflgs = 0;
	ifcmd[cmdlvl] = 0;
	iftest[cmdlvl] = 0;
	count[cmdlvl] =  count[cmdlvl-1]; /* Inherit from previous level */
	intime[cmdlvl] = intime[cmdlvl-1];
	inpcas[cmdlvl] = inpcas[cmdlvl-1];
	takerr[cmdlvl] = takerr[cmdlvl-1];
	merror[cmdlvl] = merror[cmdlvl-1];
#endif /* NOSPL */
        debug(F110,"doinit init file",line,0);
    } else {
        debug(F100,"doinit no init file","",0);
    }
    strncpy(kermrc,line,KERMRCL);	/* XXXX */
    for (ptr = kermrc; *ptr; ptr++)	/* Convert backslashes to slashes */
       if (*ptr == '\\')
         *ptr = '/';
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
	if (zchki(lp) < 0) {		/* (if it exists...) */
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
	tfline[tlevel] = 0;
	if (tfnam[tlevel] = malloc(strlen(line)+1))
	  strcpy(tfnam[tlevel],line);

	strncpy(kermrc,line,KERMRCL);	/* XXXX */

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
	cmdstk[cmdlvl].ccflgs = 0;
#endif /* NOSPL */
	debug(F110,"init file",line,0);
    }

#ifdef datageneral
/* If CKERMIT.INI not found in home directory, look in searchlist */
    if (homdir && (tlevel < 0)) {
    	strcpy(lp,kermrc);
	if ((tfile[0] = fopen(line,"r")) != NULL) {
	    tlevel = 0;
	    tfline[tlevel] = 0;
	    if (tfnam[tlevel] = malloc(strlen(line)+1))
	      strcpy(tfnam[tlevel],line);
#ifndef NOSPL
	    cmdlvl++;
	    cmdstk[cmdlvl].src = CMD_TF;
	    cmdstk[cmdlvl].lvl = tlevel;
	    cmdstk[cmdlvl].ccflgs = 0;
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

    /* Assign value to inidir */

    strcpy(inidir, kermrc);
    {
	int x;
	x = strlen(inidir);
	if (x > 0) {
	    int i;
	    for (i = x - 1; i > 0; i-- ) {
		if (inidir[i] ==
#ifdef MAC
		    '/'
#else
#ifdef UNIX
		    '/'
#else
#ifdef OSK
		    '/'
#else
#ifdef STRATUS
		    '>'
#else
#ifdef VMS
		    ']' || inidir[i] == ':'
#else
#ifdef datageneral
		    ':'
#else
#ifdef OS2
		    '/' || inidir[i+1] == '\\'
#else
#ifdef AMIGA
		    '/' || inidir[i+1] == ':'

#endif /* AMIGA */
#endif /* OS2 */
#endif /* datageneral */
#endif /* VMS */
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */
#endif /* MAC */
		    ) {
		    inidir[i+1] = NUL;
		    break;
		}
	    }
	}
    }
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
domac(name, def, flags) char *name, *def; int flags; {
    int x, m;
    m = maclvl;				/* Current macro stack level */
    debug(F101,"domac entry maclvl","",maclvl);
    x = addmac(name, def);		/* Define a new macro */
    if (x > -1) {			/* If successful, */
	dodo(x,NULL, flags);		/* start it (increments maclvl). */
	while (maclvl > m) {		/* Keep going till done with it, */
	    debug(F101,"domac loop maclvl 1","",maclvl);
	    sstate = (CHAR) parser(1);	/* parsing & executing each command, */
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

  Get next command from TAKE (command) file.

  Call with:
   s     Pointer to buffer to read into
   n     Length of buffer
   f     File descriptor of file to read from
   flag  0 == keep line terminator on and allow continuation 
         1 == discard line terminator and don't allow continuation

  Call with flag == 0 to read a command from a TAKE file;
  Call with flag != 0 to read a line from a dialing or network directory.

  In both cases, trailing comments and/or trailing whitespace is stripped.
  If flag == 0, continued lines are combined into one line.  A continued line
  is one that ends in hypen or backslash, or any line in a "block", which
  starts with "{" at the end of a line and ends with "}" at the beginning of
  a line; blocks may be nested.

  Returns:
   0 if a string was copied,
  -1 on EOF,
  -2 on malloc failure
  -3 if line is not properly terminated
  -4 if (possibly continued) line is too long. */
int
getnct(s,n,f,flag) char *s; int n; FILE *f; int flag; {
    int i, len, buflen;
    char c, cc, ccl, *s2, *lp, *lp2, *lp3, *lastcomma = NULL;
    int bc = 0;				/* Block counter */

    s2 = s;				/* Remember original pointer */
    buflen = n;				/* Remember original buffer length */

    debug(F101,"getnct","",n);
    if (!(lp2 = (char *) malloc(n+n+1))) { /* Get a temporary buffer */
	debug(F101,"getnct malloc failure","",0);
	return(-2);
    }
    while (1) {				/* Loop to read lines from file */
	if (fgets(lp2,n,f) == NULL) {	/* Read a line into lp2 */
	    debug(F100,"getnct fgets EOF","",0); /* EOF */
	    free(lp2);			/* Free temporary storage */
	    lp2 = NULL;
	    *s = NUL;			/* Make destination be empty */
	    return(-1);			/* Return failure code */
	}
#ifndef NODIAL
	if (flag)			/* Count this line */
	  dirline++;
	else
#endif /* NODIAL */
	  tfline[tlevel]++;
	len = strlen(lp2) - 1;		/* Position of line terminator */
	debug(F111,"getnct fgets ok",lp2,len);
	if (len < 0)
	  len = 0;
	if (techo && pflag)		/* If TAKE ECHO ON, */
	  printf("%3d. %s",		/* echo it this line. */
#ifndef NODIAL
		 flag ? dirline :
#endif /* NODIAL */
		 tfline[tlevel],
		 lp2
		 );
        lp3 = lp2;			/* Working pointer */
	i = len;
	while (*lp3 == SP || *lp3 == HT) { /* First nonwhitespace character */
	    i--;
	    lp3++;
	}
	if (i == 0 && bc > 0)		/* Blank line in {...} block */
	  continue;

	/* Isolate, remove, and check terminator */

	c = lp2[len];			/* Value of line terminator */
	debug(F101,"getnct terminator","",c);
	if (c < LF || c > CR) {		/* It's not a terminator */
	    debug(F111,"getnct bad line",lp2,c);
	    if (feof(f) && len > 0 && len < n) {
		/* Kludge Alert... */
		printf("Warning: Last line of %s lacks terminator\n",
		       s2 == cmdbuf ? "command file" : "directory file");
		c = lp2[++len] = '\n';	/* No big deal - supply one. */
	    } else {			/* Something's wrong, fail. */
		free(lp2);
		lp2 = NULL;
		return(-3);
	    }
	}

	/* Trim trailing whitespace */

	for (i = len - 1; i > -1; i--)	/* Back up over spaces and tabs */
	  if (lp2[i] != SP && lp2[i] != HT && lp2[i] != NUL)
	    break;
	debug(F101,"getnct i","",i);
	lp2[i+1] = NUL;			/* Terminate the string */
	debug(F110,"getnct lp2",lp2,0);
	lp = lp2;			/* Make a working pointer */

	/* Remove trailing or full-line comment */

	while (cc = *lp) {
	    if (cc == ';' || cc == '#') { /* Comment introducer? */
		if (lp == lp2) {	/* First char on line */
		    *lp = NUL;
		    break;
		} else if (*(lp - 1) == SP || *(lp - 1) == HT) {
		    lp--;
		    *lp = NUL;	/* Or preceded by whitespace */
		    break;
		}
	    }
	    lp++;
	}
	if (lp > lp2)
	  lp--;				/* Back up over the NUL */

	/* Now trim any space that preceded the comment */

	while ((*lp == SP || *lp == HT) && lp >= lp2) {
	    *lp = NUL;
	    if (lp <= lp2)
	      break;
	    lp--;
	}
	debug(F110,"getnct comment trimmed",lp2,0);

	len = strlen(lp2);		/* Length after trimming */

	if (n - len < 2) {		/* Check remaining space */
	    debug(F111,"getnct command too long",s2,buflen);
	    printf("?Line too long, maximum length: %d.\n",buflen);
	    free(lp2);
	    lp2 = NULL;
	    return(-4);
	}
	ccl = (len > 0) ? lp2[len-1] : 0; /* Last character in line */

	lp = lp2;
	while (*s++ = *lp++)		/* Copy result to target buffer */
	  n--;				/* accounting for length */
	s--;				/* Back up over the NUL */

	/* Check whether this line is continued */

        if (flag)			/* No line continuation when flag=1 */
	  break;			/* So break out of read-lines loop */

	debug(F000,"getnct first char","",*lp3);
	debug(F000,"getnct last char","",ccl);

	if (bc > 0 && *lp3 == '}')	/* First char on line is '}' */
	  bc--;				/* Decrement block counter */

	if (bc == 0 &&			/* Line is continued if bc > 0 */
#ifdef COMMENT
	    /* Not supported as of C-Kermit 6.0 */
	    ccl != CMDQ &&		/* or line ends with CMDQ */
#endif /* COMMENT */
	    ccl != '-'  &&		/* or line ends with dash */
	    ccl != '{')			/* or line ends with opening brace */
	  break;			/* None of those, we're done. */

	if (ccl == '{') {		/* Last char on line is '{'? */
	    bc++;			/* Count the block opener. */
	} else if (ccl == '-'		/* Explicit continue? */
#ifdef COMMENT
/* Not supported as of C-Kermit 6.0. */
		   || ccl == CMDQ
#endif /* COMMENT */
		   ) {
	    s--;			/* Yes, back up over terminators */
	    n++;			/* and over continuation character */
	} else {			/* None of those but (bc > 0) */
	    lastcomma = s;
	    *s++ = ',';			/* and insert a comma */
	    n--;
	}
	debug(F101,"getnct bc","",bc);
	debug(F100,"getnct continued","",0);
    } /* read-lines while loop */

    if (lastcomma)
      *lastcomma = SP;
    if (!flag)				/* Tack line terminator back on */
      *s++ = c;
    *s++ = NUL;				/* Terminate the string */
    untab(s2);				/* Done, convert tabs to spaces */
    debug(F110,"getnct return",s2,0);
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

#ifdef OS2
    if ( cursor_save > -1 ) {		/* restore cursor if it was */
	cursorena[VCMD] = cursor_save ;	/* turned off in file xfer  */
	cursor_save = -1 ;		/* mode */
    }
#endif /* OS2 */

    what = W_COMMAND;		/* Now we're parsing commands. */
    moving = 0;			/* We're not MOVE'ing */
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
	debug(F111,"parser tmpdir restoring",savdir,x);
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
#ifdef WHAT_THE_HECK_IS_THIS
	int nnn;
	debug(F100,"top of parse loop","",0);
	nnn = ttchk();
	debug(F101,"APC ttchk p1","",nnn);
#endif /* WHAT_THE_HECK_IS_THIS */

	remfile = 0;			/* Clear these in case REMOTE */
	remappd = 0;			/* command was interrupted... */
	rempipe = 0;

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
	    if (cmdlvl == 0) {		/* Just popped out of all cmd files? */
		if (
#ifdef NT
		    StartedFromDialer &&
#endif /* NT */
		    cfilef) {		/* Delete startup file? */
		    if (DeleteStartupFile) {
			debug(F101,"DeleteStartupFile",cmdfil,0);
			zdelet(cmdfil);
		    }
		}
		return(0);		/* End of init file or whatever. */
	    }
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

	      if ((tfcode = getnct(cmdbuf,CMDBL,tfile[tlevel],0)) < 0) {
		  if (tfcode < -1) {	/* Error */
		      printf("?Error in TAKE command file: %s\n",
			     (tfcode == -2) ? "Memory allocation failure" :
			     "Line too long or contains NUL characters"
			     );
		      dostop();
		  }
		  continue;		/* -1 means EOF */
	      }	    

        /* If interactive, get next command from user. */

	} else {			/* User types it in. */
	    if (pflag) prompt(xxstring);
	    cmini(ckxech);
    	}

    /* Now we know where next command is coming from. Parse and execute it. */

	repars = 1;			/* 1 = command needs parsing */
	displa = 0;			/* Assume no file transfer display */

	while (repars) {		/* Parse this cmd until entered. */
	    debug(F101,"parser top of while loop","",0);
	    cmres();			/* Reset buffer pointers. */

#ifdef OS2
#ifdef COMMENT
	    /* we check to see if a macro is waiting to be executed */
	    /* if so, we call domac on it */
	    if (cmdmac) {
		strncpy(cmdbuf, cmdmac, CMDBL);
		free(cmdmac);
		cmdmac = NULL;
	    }
#endif /* COMMENT */
#endif /* OS2 */
	    bye_active = 0;
	    xx = cmkey2(cmdtab,ncmd,"Command","",toktab,xxstring,1);
	    debug(F101,"top-level cmkey2","",xx);
	    if (xx == -5) {
		yy = chktok(toktab);
		debug(F101,"top-level cmkey token","",yy);
		ungword();
		switch (yy) {
		  case '#': xx = XXCOM; break; /* Comment */
		  case ';': xx = XXCOM; break; /* Comment */
#ifndef NOSPL
		  case ':': xx = XXLBL; break; /* GOTO label */
#endif /* NOSPL */

#ifndef NOPUSH
#ifdef CK_REDIR            
                  case '<':
#endif /* CK_REDIR */      
                  case '@':
                  case '!':                   
		    if (!nopush) {
			switch(yy) {
#ifdef CK_REDIR
			  case '<': xx = XXFUN; break; /* REDIRECT */
#endif /* CK_REDIR */
			  case '@':
			  case '!': xx = XXSHE; break; /* Shell escape */
			}
		    }
		    break;
#endif /* NOPUSH */
		  default: 
		    printf("\n?Invalid - %s\n",cmdbuf);
		    if (tlevel > -1) {
			printf("Command file: %s, line %d\n",
			       tfnam[tlevel] ? tfnam[tlevel] : "",
			       tfline[tlevel]
			       );
		    }
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

	    zz = docmd(xx);		/* Parse rest of command & execute. */
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
	      case -4:			/* EOF (e.g. on redirected stdin) */
		doexit(GOOD_EXIT,xitsta); /* ...exit successfully */
	      case -1:			/* Reparse needed */
		repars = 1;		/* Just set reparse flag and... */
		continue;
#ifdef OS2
	      case -7:			/* They typed a disk letter */
		if (!zchdir((char *)cmdbuf)) {
		    perror((char *)cmdbuf);
		    success = 0;
		} else success = 1;
		repars = 0;
		continue;
		
#endif /* OS2 */
	      case -6:			/* Invalid command given w/no args */
	      case -2:			/* Invalid command given w/args */
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
			    mp = NULL;
			} else sprintf(cmdbuf,"do %s %c",atmbuf, CR);
			if (ifcmd[cmdlvl] == 2)	/* This one doesn't count! */
			  ifcmd[cmdlvl]--;
			debug(F111,"stuff cmdbuf",cmdbuf,zz);
			repars = 1;	/* Go for reparse */
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
#else /* Not COMMENT */
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
#ifdef CK_RECALL
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
			    p = NULL;
			    prompt(xxstring); /* Reprint the prompt */
			    printf("%s",cmdbuf); /* Reprint partial command */
			    repars = 1;          /* Force reparse */
			    continue;
			}
		    } else
#endif /* CK_RECALL */
		      if (tlevel > -1) {
			  printf("Command file: %s, line %d\n",
				 tfnam[tlevel] ? tfnam[tlevel] : "",
				 tfline[tlevel]
				 );
		      }
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
static char obuf[OBSIZE+1];		/* OUTPUT buffer. */
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
#endif /* NOSPL */


#ifndef NOSPL

/*  Routines for handling local variables -- also see popclvl().  */

VOID
freelocal(m) int m; {			/* Free local variables */
    struct localvar * v, * tv;		/* at macro level m... */
    debug(F101,"freelocal level","",m);
    if (m < 0) return;
    v = localhead[m];			/* List head for level m */
    while (v) {
	if (v->lv_name)			/* Variable name */
	  free(v->lv_name);
	if (v->lv_value)		/* Value */
	  free(v->lv_value);
	tv = v;				/* Save pointer to this node */
	v = v->lv_next;			/* Get next one */
	if (tv)				/* Free this one */
	  free(tv);
    }
    localhead[m] = (struct localvar *) NULL; /* Done, set list head to NULL */
}

int
dolocal() {				/* Do the LOCAL command */
    int i, x, z, len;
    char * s, * p;
    char * list[65];			/* Up to 64 variables per line */
    struct localvar * v, *prev;

    if ((x = cmtxt("Variable name(s)","",&s,NULL) < 0))
      return(x);

    xwords(s,64,list,0);		/* Break up line into "words" */
/*
  loop thru variable list.
  if variable is defined, allocate a node for it,
  copy its value to the node,
  and undefine it.
*/
    if (v = localhead[cmdlvl]) {	/* Already have some at this level? */
	while (v) {			/* Find end of list */
	    prev = v;
	    v = v->lv_next;
	}
    }
    for (i = 1; i < 10 && list[i]; i++) { /* Go through the list */
	if (*(list[i]) == CMDQ)  {
	    if (parsevar(list[i],&x,&z) < 0) /* Check for valid name */
	      continue;
	    sprintf(tmpbuf,"\\fcontents(%s)",list[i]); /* Get definition */
	} else {			/* Macro */
	    sprintf(tmpbuf,"\\fdefinition(%s)",list[i]); /* Get def */
	}
	s = line;			/* Point to destination buffer */
	x = LINBUFSIZ;			/* Length of destination buffer */
	zzstring(tmpbuf,&s,&x);		/* Get definition */

/* Name is in list[i], definition is in line[] */

	v = (struct localvar *) malloc(sizeof(struct localvar));
	if (!v) {
	    printf("?Failure to allocate storage for local variables");
	    return(-9);
	}
	if (!localhead[cmdlvl])		/* If first, set list head */
	  localhead[cmdlvl] = v;
	else				/* Otherwise link previous to this */
	  prev->lv_next = v;
	prev = v;			/* And make this previous */
	v->lv_next = (struct localvar *) NULL; /* No next yet */

	if (!(v->lv_name = (char *) malloc((int) strlen(list[i]) + 1)))
	  goto localbad;	  
	strcpy(v->lv_name, list[i]);	/* Copy name into new node */

	if (*line) {
	    if (!(v->lv_value = (char *) malloc((int) strlen(line) + 1)))
	      goto localbad;
	    strcpy(v->lv_value, line);	/* Copy value into new node */
	} else
	  v->lv_value = NULL;

	delmac(list[i]);		/* Delete the original macro */
    }
    return(success = 1);

  localbad:
    printf("?Failure to allocate storage for local variables");
    freelocal(cmdlvl);
    return(-9);
}
#endif /* NOSPL */

#ifndef NOSPL
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
    if (!cmdgquo()) {			/* COMMAND QUOTING OFF */
	x = strlen(s);			/* Just send the string literally */
	xx = local ? ttol((CHAR *)s,x) : conxo(x,s);
	return(success = (xx == x) ? 1 : 0);
    }
    quote = 0;				/* Initialize backslash (\) quote */
    obn = 0;				/* Reset count */
    obp = obuf;				/* and pointers */

    while (x = *s++) {			/* Loop through the string */
	y = 0;				/* Error code, 0 = no error. */
	debug(F000,"dooutput","",x);
	if (quote) {			/* This character is quoted */
#ifndef NOKVERBS
#define K_BUFLEN 30
#define SEND_BUFLEN 255
#define sendbufd(x) { sendbuf[sendndx++] = x;\
 if (sendndx == SEND_BUFLEN) {dooutput(s); sendndx = 0;}}

           if (x == 'k' || x == 'K') { /* \k or \K */
               extern struct keytab kverbs[];
               extern int nkverbs;
               extern char * keydefptr;
               extern int keymac;
               extern int keymacx;      
               int x, y, brace = 0;
               int pause;
               char * p, * b;
               char kbuf[K_BUFLEN + 1];	/* Key verb name buffer */
	       char sendbuf[SEND_BUFLEN +1];
               int  sendndx = 0;

	       if (xxout(obuf,obn) < 0) /* Flush buffer */
		 goto outerr;
	       debug(F100,"OUTPUT KVERB","",0); /* Send a KVERB */
               {			/* Have K verb? */
		   if (!*s) {
                       break;
                   }
/*
  We assume that the verb name is {braced}, or it extends to the end of the
  string, s, or it ends with a space, control character, or backslash.
*/
                   p = kbuf;		/* Copy verb name into local buffer */
                   x = 0;
                   while ((x++ < K_BUFLEN) && (*s > SP) && (*s != CMDQ)) {
                       if (brace && *s == '}') {
                           break;
                       }
                       *p++ = *s++;
                   }
                   if (*s && !brace)	/* If we broke because of \, etc, */
                     s--;		/*  back up so we get another look. */
                   brace = 0;
                   *p = NUL;		/* Terminate. */
                   p = kbuf;		/* Point back to beginning */
                   debug(F110,"dooutput kverb",p,0);
                   y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
                   debug(F101,"dooutput lookup",0,y);
                   if (y > -1) {
                       if ( sendndx ) { 
                           dooutput( sendbuf ); 
                           sendndx = 0; 
                       }
                       dokverb(VCMD,y);      
#ifndef NOSPL
                   } else {		/* Is it a macro? */
                       y = mxlook(mactab,p,nmac);
                       if (y > -1) {
                           cmpush();
                           keymac = 1;	/* Flag for key macro active */
                           keymacx = y;	/* Key macro index */
                           keydefptr = s; /* Where to resume next time */
                           debug(F111,"dooutput mxlook",keydefptr,y);
                           parser(1);
                           cmpop();
                       }
#endif /* NOSPL */
                   }
               }
               quote = 0;
	       continue;
	   } else
#endif /* NOKVERBS */
	     if (x == 'n' || x == 'N') { /* \n or \N */
		 if (xxout(obuf,obn) < 0) /* Flush buffer */
		   goto outerr;
		 debug(F100,"OUTPUT NUL","",0); /* Send a NUL */
		 if (local)
		   ttoc(NUL);
		 else
		   conoc(NUL);
		 quote = 0;
		 continue;

	     } else if (x == 'b' || x == 'B') { /* \b or \B */

		if (xxout(obuf,obn) < 0) /* Flush buffer first */
		  goto outerr;
		debug(F100,"OUTPUT BREAK","",0);
#ifndef NOLOCAL
		ttsndb();		/* Send BREAK signal */
#else
		 if (local)
		   ttoc(NUL);
		 else
		   conoc(NUL);
#endif /* NOLOCAL */
		quote = 0;		/* Turn off quote flag */
		continue;		/* and not the b or B */
#ifdef CK_LBRK
	     } else if (x == 'l' || x == 'L') { /* \l or \L */
		 if (xxout(obuf,obn) < 0) /* Flush buffer first */
		   goto outerr;
		 debug(F100,"OUTPUT Long BREAK","",0);
#ifndef NOLOCAL
		 ttsndlb();		/* Send Long BREAK signal */
#else
		 if (local)
		   ttoc(NUL);
		 else
		   conoc(NUL);
#endif /* NOLOCAL */
		 quote = 0;		/* Turn off quote flag */
		 continue;		/* and not the l or L */
#endif /* CK_LBRK */

	     } else if (x == CMDQ) {	/* Backslash itself */
		 debug(F100,"OUTPUT CMDQ","",0);
		 xx = oboc(dopar(CMDQ)); /* Output the backslash. */
		 if (xx < 0)
		   goto outerr;
		 quote = 0;
		 continue;

	     } else {			/* if \ not followed by b or B */
		/* Note: Atari ST compiler won't allow macro call in "if ()" */
		 xx = oboc(dopar(CMDQ)); /* Output the backslash. */
		 if (xx < 0)
		   goto outerr;
		 quote = 0;		/* Turn off quote flag */
	     }
	} else if (x == CMDQ) {		/* This is the quote character */
            quote = 1;			/* Go back and get next character */
	    continue;			/* which is quoted */
	} 
	xx = oboc(dopar((char)x));	/* Output this character */
	debug(F111,"dooutput",obuf,obn);
	if (xx < 0)
	  goto outerr;
	if (seslog && duplex)		/* Log the character if log is on */
	  if (zchout(ZSFILE,(char)x) < 0) /* and connection is half duplex */
	    seslog = 0;
	if (x == '\015') {		/* String contains carriage return */
	    int stuff = -1, stuff2 = -1;
	    if (tnlm) {			/* TERMINAL NEWLINE ON */
		stuff = LF;		/* Stuff LF */
        }
#ifdef TNCODE
        /* TELNET NEWLINE ON/OFF/RAW */
	    if (network && (ttnproto == NP_TELNET)) {
            switch (me_binary ? tn_b_nlm : tn_nlm) { /* NVT or BINARY */
                case TNL_CR:
                    break;
                case TNL_CRNUL:
                    stuff2 = stuff;
                    stuff  = NUL;
                    break;
                case TNL_CRLF:
                    stuff2 = stuff;
                    stuff = LF;
                    break;
                }
    	    }
#endif /* TNCODE */
	    if (stuff > -1) {		/* Stuffing another character... */
		xx = oboc(dopar((CHAR)stuff)); 
		if (xx < 0)
		  goto outerr;
		if (seslog && duplex)	/* Log stuffed char if appropriate */
		  if (zchout(ZSFILE, (CHAR)stuff) < 0)
		    seslog = 0;
	    }
	    if (stuff2 > -1) {		/* Stuffing another character... */
		xx = oboc(dopar((CHAR)stuff2)); 
		if (xx < 0)
		  goto outerr;
		if (seslog && duplex)	/* Log stuffed char if appropriate */
		  if (zchout(ZSFILE, (CHAR)stuff2) < 0)
		    seslog = 0;
	    }
	    if (xxout(obuf,obn) < 0)	/* Flushing is required here! */
	      goto outerr;
	}
    }
    if (quote == 1)			/* String ended with backslash */
      xx = oboc(dopar(CMDQ));

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
#ifdef OSK
	printf("%s, for%s\n",versio,ckxsys);
#else
	printf("%s, for%s\n\r",versio,ckxsys);
#endif /* OSK */
#endif /* datageneral */
	printf(" Copyright (C) 1985, 1996,\n");
	printf("  Trustees of Columbia University in the City of New York.\n");

#ifdef OS2
       shoreg();
#endif /* OS2 */

	if (!quiet && !backgrd) {
	    char *s = NULL;
	    switch (binary) {
	      case XYFT_T: s = "TEXT";	       break;
#ifdef VMS
	      case XYFT_B: s = "BINARY FIXED"; break;
	      case XYFT_I: s = "IMAGE";        break;
	      case XYFT_L: s = "LABELED";      break;
	      case XYFT_U: s = "BINARY UNDEF"; break;
#else
#ifdef MAC
	      case XYFT_B: s = "BINARY";       break;
	      case XYFT_M: s = "MACBINARY";    break;
#else
	      case XYFT_B: s = "BINARY";       break;
#ifdef CK_LABELED
	      case XYFT_L: s = "LABELED";      break;
#endif /* CK_LABELED */
#endif /* MAC */
#endif /* VMS */
	    }
	    if (s) printf("Default file-transfer mode is %s\n", s);
#ifdef COMMENT
#ifdef NT
	    printf("\n");
	    printf("StartedFromDialer = %d\n",StartedFromDialer);
	    printf("Window Handle = %ld\n",DialerHandle);
	    printf("\n");
#endif /* NT */
#endif /* COMMENT */
	    printf("Type ? or HELP for help.\n");
	}
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

/* Returns -1 on failure, 0 on success. */

int
addmac(nam,def) char *nam, *def; {	/* Add a macro to the macro table */
    int i, x, y, z, namlen, deflen;
    char * p = NULL, c;

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
		char **q;
		m_arg[maclvl][c - '0'] = p; /* Assign the value */
		debug(F101,"addmac macro arg maclvl","",maclvl);
		q = a_ptr[0];		/* Copy pointer to macro arg vector */
		q[c - '0'] = p;
	    }
	} else {			/* It's a global variable */
	    if (c < 33 || c > GVARS) return(-1);
	    if (isupper(c)) c = (char) tolower(c);
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
	x -= ARRAYBASE;			/* Convert name letter to index. */
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
    if (deflen < 1)
      return(0);
    debug(F111,"addmac table size",nam,nmac);
    for (y = 0;				/* Find the alphabetical slot */
	 y < MAC_MAX && mactab[y].kwd != NULL && strcmp(nam,mactab[y].kwd) > 0;
	 y++);
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
	    if (mactab[y].kwd) {
		free(mactab[y].kwd);
		mactab[y].kwd = NULL;
	    }
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
	    p = NULL;
	} else debug(F110,"delmac def","(null pointer)",0);
	return(0);
    }
	    
    if (*nam == '&') {			/* An array reference? */
	char **q;
	if (arraynam(nam,&x,&z) < 0)	/* If syntax is bad */
	  return(-1);			/* return -1. */
	x -= ARRAYBASE;			/* Convert name to number. */
	if ((q = a_ptr[x]) == NULL)	/* If array not declared, */
	  return(-2);			/* return -2. */
	if (z > a_dim[x])		/* If subscript out of range, */
	  return(-3);			/* return -3. */
	if (q[z]) {			/* If there is an old value, */
	    debug(F110,"delman def",q[z],0);
	    if (x != 0)			/* Macro arg vector is just a copy */
	      free(q[z]);		/* Others are real so free them */
	    q[z] = NULL;
	} else debug(F110,"delmac def","(null pointer)",0);
    }

   /* Not a variable or an array, so it must be a macro. */

    if ((x = mlook(mactab,nam,nmac)) < 0) { /* Look it up */
	debug(F111,"delmac mlook",nam,x);
	return(x);
    }
    if (mactab[x].kwd) {		/* Free the storage for the name */
	free(mactab[x].kwd);
	mactab[x].kwd = NULL;
    }
    if (mactab[x].mval) {		/* and for the definition */
	free(mactab[x].mval);
	mactab[x].mval = NULL;
    }
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
    /* And the table of arrays */
    for (i = 0; i < (int) 'z' - ARRAYBASE + 1; i++) {
	a_ptr[i] = (char **) NULL;	/* Null pointer for each */
	a_dim[i] = 0;			/* and a dimension of zero */
    }
}

int
popclvl() {				/* Pop command level, return cmdlvl */
    struct localvar * v;
    debug(F101,"popclvl cmdlvl","",cmdlvl);
    if (cmdlvl > 0) {
	if (v = localhead[cmdlvl]) { /* Did we save any variables? */
	    while (v) {			/* Yes */
		if (v->lv_value)	/* Copy old ones back */
		  addmac(v->lv_name,v->lv_value);
		else
		  delmac(v->lv_name);
		v = v->lv_next;
	    }
	    freelocal(cmdlvl);	/* Free local storage */
	}
    }
    if (cmdlvl < 1) {			/* If we're already at top level */
	cmdlvl = 0;			/* just make sure all the */
	tlevel = -1;			/* stack pointers are set right */
	maclvl = -1;			/* and return */
    } else if (cmdstk[cmdlvl].src == CMD_TF) { /* Reading from TAKE file? */
	debug(F101,"popclvl tlevel","",tlevel);
	if (tlevel > -1) {		/* Yes, */
	    fclose(tfile[tlevel]);	/* close it */
	    if (tfnam[tlevel]) {	/* free storage for name */
		free(tfnam[tlevel]);
		tfnam[tlevel] = NULL;
	    }
	    tlevel--;			/* and pop take level */
	    cmdlvl--;			/* and command level */
	} else tlevel = -1;
    } else if (cmdstk[cmdlvl].src == CMD_MD) { /* In a macro? */
	debug(F101,"popclvl maclvl","",maclvl);
	if (maclvl > -1) {		/* Yes, */
	    int i;
	    char **q;
	    debug(F111,"popclvl mac 1",macx[maclvl],maclvl);
	    debug(F111,"popclvl mac 2",macp[maclvl],maclvl);
	    macp[maclvl] = "";		/* set macro pointer to null string */
	    debug(F111,"popclvl mac 3",macp[maclvl],maclvl);
	    *cmdbuf = '\0';		/* clear the command buffer */
	    debug(F111,"popclvl mac 4",mrval[maclvl+1],maclvl);
	    if (mrval[maclvl+1]) {	/* Free any deeper return values. */
		free(mrval[maclvl+1]);
		mrval[maclvl+1] = NULL;
	    }
	    debug(F111,"popclvl mac 6",mrval[maclvl+1],maclvl);
	    maclvl--;			/* Pop macro level */
	    cmdlvl--;			/* and command level */
	    q = a_ptr[0];
	    debug(F101,"popclvl mac 7","",maclvl);
	    if (maclvl > -1)
	      for (i = 0; i < 10; i++)	/* Restore previous arg vector */
		q[i] = m_arg[maclvl][i];
	    debug(F111,"popclvl mac 8",
		  (maclvl > -1 && macx[maclvl]) ? macx[maclvl] : "",maclvl);
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
    while (popclvl()) ;		/* Pop all macros & take files */
#ifndef NOSPL
    while (cmpop() > -1);	/* And all recursive cmd pkg invocations */
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
	    if (deblog <= 0) {
		printf("?Debugging log wasn't open\n");
		return(0);
	    }
            debug(F100,"Debug Log Closed","",0L);
	    *debfil = '\0';
	    deblog = 0;
	    return(zclose(ZDFILE));
#endif /* DEBUG */

	case LOGP:
	    if (pktlog <= 0) {
		printf("?Packet log wasn't open\n");
		return(0);
	    }
	    *pktfil = '\0';
	    pktlog = 0;
	    return(zclose(ZPFILE));
 
#ifndef NOLOCAL
	case LOGS:
	    if (seslog <= 0) {
		printf("?Session log wasn't open\n");
		return(0);
	    }
	    *sesfil = '\0';
	    seslog = 0;
	    return(zclose(ZSFILE));
#endif /* NOLOCAL */
 
#ifdef TLOG
    	case LOGT:
	    if (tralog <= 0) {
		printf("?Transaction log wasn't open\n");
		return(0);
	    }
            tlog(F100,"Transaction Log Closed","",0L);
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

static char *nm[] =  { "disabled", "local only", "remote only", "enabled" };

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
    extern long xvernum;
    extern char *ck_ver;
    printf("\nVersions:\n %s\n Numeric: %ld",versio,vernum);
    if (verwho) printf("-%d",verwho);
    printf("\n");
    if (xvernum)
      printf("C-Kermit %s\n",ck_ver);
    printf(xxdiff(ckxv,ckxsys) ? " %s for%s\n" : " %s\n",ckxv,ckxsys);
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
    int lines = 5;
#ifdef OS2
#ifdef NT
    if (isWin95())
      printf("\nWindows 95.\n");
    else 
      printf("\nWindows NT.\n");
    lines++;
#else /* NT */
#ifdef M_I286
    printf("\nOS/2 16-bit.\n");
#else
    printf("\nOS/2 32-bit.\n");
#endif /* M_I286 */
    lines++;
#endif /* NT */
#endif /* OS2 */
    printf("\nMajor optional features included:\n");
    lines++;
#ifdef NETCONN
    printf(" Network support (type SHOW NET for further info)\n");
    lines++;
#endif /* NETCONN */
#ifdef CK_RTSCTS
    printf(" Hardware flow control\n");
    lines++;
#endif /* CK_RTSCTS */

#ifdef CK_XYZ
#ifdef XYZ_INTERNAL
    printf(" Built-in XYZMODEM protocols\n");
#else
    printf(" External XYZMODEM protocol support\n");
#endif /* XYZ_INTERNAL */
    lines++;
#endif /* CK_XYZ */

#ifndef NOCSETS
    printf(" Latin-1 (West European) character-set translation\n");
    lines++;
#ifdef LATIN2
    printf(" Latin-2 (East European) character-set translation\n");
    lines++;
#endif /* LATIN2 */
#ifdef CYRILLIC
    printf(" Cyrillic (Russian, Ukrainian, etc) character-set translation\n");
    lines++;
#endif /* CYRILLIC */
#ifdef HEBREW
    printf(" Hebrew character-set translation\n");
    lines++;
#endif /* HEBREW */
#ifdef KANJI
    printf(" Kanji (Japanese) character-set translation\n");
    lines++;
#endif /* KANJI */
#ifdef CKOUNI
    if (isunicode())
      printf(" Unicode support for ISO-2022 Terminal Emulation\n");
    else
      printf(" Unicode translation for Terminal Character-Sets\n");
#endif /* CKOUNI */
#endif /* NOCSETS */

#ifdef CK_REDIR
    printf(" REDIRECT command\n");
    lines++;
#endif /* CK_REDIR */
#ifdef CK_RESEND
    printf(" RESEND command\n");
    lines++;
#endif /* CK_RESEND */

#ifdef CK_CURSES
    printf(" Fullscreen file transfer display\n");
    lines++;
#endif /* CK_CURSES */

#ifdef CK_SPEED
    printf(" Control-character unprefixing\n");
    lines++;
#endif /* CK_SPEED */

#ifdef OS2MOUSE
    printf(" Mouse support\n");
    lines++;
#endif /* OS2MOUSE */

#ifdef CK_REXX
    printf(" REXX script language interface\n");
    lines++;
#endif /* CK_REXX */

    printf("\nMajor optional features not included:\n");
    lines++;
#ifndef CK_CURSES
#ifndef MAC
    printf(" No fullscreen file transfer display\n");
    lines++;
    flag = 1;
#endif /* MAC */
#endif /* CK_CURSES */
#ifdef NOSERVER
    printf(" No server mode\n");
    lines++;
    flag = 1;
#endif /* NOSERVER */
#ifdef NODEBUG
    printf(" No debugging\n");
    lines++;
    flag = 1;
#endif /* NODEBUG */
#ifdef NOTLOG
    printf(" No transaction log\n");
    lines++;
    flag = 1;
#endif /* NOTLOG */
#ifdef NOHELP
    printf(" No built-in help\n");
    lines++;
    flag = 1;
#endif /* NOHELP */
#ifndef NETCONN
    printf(" No network support\n");
    lines++;
    flag = 1;
#endif /* NETCONN */
#ifdef NOMSEND
    printf(" No MSEND command\n");
    lines++;
    flag = 1;
#endif /* NOMSEND */
#ifdef NODIAL
    printf(" No DIAL command\n");
    lines++;
    flag = 1;
#else
#ifdef MINIDIAL
    printf(
	 " No DIAL command for modems other than Hayes, CCITT, and Unknown\n");
    lines++;
    flag = 1;
#endif /* MINIDIAL */
#ifndef CK_RTSCTS
#ifndef MAC
    printf(" No hardware flow control\n");
    lines++;
    flag = 1;
#endif /* MAC */
#endif /* CK_RTSCTS */
#endif /* NODIAL */
#ifdef NOXMIT
    printf(" No TRANSMIT command\n");
    lines++;
    flag = 1;
#endif /* NOXMIT */
#ifdef NOSCRIPT
    printf(" No SCRIPT command\n");
    lines++;
    flag = 1;
#endif /* NOSCRIPT */
#ifdef NOSPL
    printf(" No script programming features\n");
    lines++;
    flag = 1;
#endif /* NOSPL */
#ifndef CK_XYZ
    printf(" No built-in XYZMODEM protocols\n");
    lines++;
    flag = 1;
#endif /* CK_XYZ */
#ifdef NOCSETS
    printf(" No character-set translation\n");
    lines++;
    flag = 1;
#else
#ifndef LATIN2
    printf(" No Latin-2 character-set translation\n");
    lines++;
    flag = 1;
#endif /* LATIN2 */
#ifdef NOHEBREW
    printf(" No Hebrew character-set translation\n");
    lines++;
    flag = 1;
#endif /* NOHEBREW */
#ifdef NOCYRIL
    printf(" No Cyrillic character-set translation\n");
    lines++;
    flag = 1;
#endif /* NOCYRIL */
#ifndef KANJI
    printf(" No Kanji character-set translation\n");
    lines++;
    flag = 1;
#endif /* KANJI */
#endif /* NOCSETS */
#ifdef NOCMDL
    printf(" No command-line arguments\n");
    lines++;
    flag = 1;
#endif /* NOCMDL */
#ifdef NOFRILLS
    printf(" No frills\n");
    lines++;
    flag = 1;
#endif /* NOFRILLS */
#ifdef NOPUSH
    printf(" No escape to system\n");
    lines++;
    flag = 1;
#endif /* NOPUSH */
#ifdef NOJC
#ifdef UNIX
    printf(" No UNIX job control\n");
    lines++;
    flag = 1;
#endif /* UNIX */
#endif /* NOJC */
#ifdef NOSETKEY
    printf(" No SET KEY command\n");
    lines++;
    flag = 1;
#endif /* NOSETKEY */
#ifndef PARSENSE
    printf(" No automatic parity detection\n");
    lines++;
    flag = 1;
#endif /* PARSENSE */
#ifndef CK_SPEED
    printf(" No control-character unprefixing\n");
    lines++;
    flag = 1;
#endif /* CK_SPEED */
#ifndef CK_REDIR
    printf(" No REDIRECT command\n");
    lines++;
    flag = 1;
#endif /* CK_REDIR */
#ifndef CK_RESEND
    printf(" No RESEND command\n");
    lines++;
    flag = 1;
#endif /* CK_RESEND */
#ifdef OS2
#ifdef __32BIT__
#ifndef OS2MOUSE
    printf(" No Mouse support\n");
    lines++;
    flag = 1;
#endif /* __32BIT__ */
#endif /* OS2 */
#endif /* OS2MOUSE */
#ifdef OS2
#ifndef NT
#ifndef CK_REXX
    printf(" No REXX script language interface\n");
    lines++;
    flag = 1;
#endif /* CK_REXX */
#endif /* NT */
#endif /* OS2 */

    if (flag == 0) { printf(" None\n"); lines++; }
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
    lines++;

#ifdef MAC
#ifdef MPW
    if (!prtopt(&lines,"MPW")) return;
#else
#ifdef THINK_C
    if (!prtopt(&lines,"THINK_C")) return;
#endif /* THINK_C */
#endif /* MPW */
#endif /* MAC */

#ifdef __386__
    if (!prtopt(&lines,"__386__")) return;
#endif /* __386__ */

#ifdef DEBUG
#ifdef IFDEBUG
    if (!prtopt(&lines,"IFDEBUG")) return;
#else
    if (!prtopt(&lines,"DEBUG")) return;
#endif /* IFDEBUG */
#endif /* DEBUG */
#ifdef TLOG
    if (!prtopt(&lines,"TLOG")) return;
#endif /* TLOG */
#ifdef BIGBUFOK
    if (!prtopt(&lines,"BIGBUFOK")) return;
#endif /* BIGBUFOK */
#ifdef XFRCAN
    if (!prtopt(&lines,"XFRCAN")) return;
#endif /* XFRCAN */
#ifdef CK_SPEED
    if (!prtopt(&lines,"CK_SPEED")) return;
#endif /* CK_SPEED */
#ifdef CK_APC
    if (!prtopt(&lines,"CK_APC")) return;
#endif /* CK_APC */
#ifdef CK_AUTODL
    if (!prtopt(&lines,"CK_AUTODL")) return;
#endif /* CK_AUTODL */
#ifdef CK_MKDIR
    if (!prtopt(&lines,"CK_MKDIR")) return;
#endif /* CK_MKDIR */
#ifdef NOMKDIR
    if (!prtopt(&lines,"NOMKDIR")) return;
#endif /* NOMKDIR */
#ifdef CK_LABELED
    if (!prtopt(&lines,"CK_LABELED")) return;
#endif /* CK_LABELED */
#ifdef NODIAL
    if (!prtopt(&lines,"NODIAL")) return;
#endif /* NODIAL */
#ifdef MINIDIAL
    if (!prtopt(&lines,"MINIDIAL")) return;
#endif /* MINIDIAL */
#ifdef WHATAMI
    if (!prtopt(&lines,"WHATAMI")) return;
#endif /* WHATAMI */
#ifdef DYNAMIC
    if (!prtopt(&lines,"DYNAMIC")) return;
#endif /* IFDEBUG */
#ifndef NOSPL
    sprintf(line,"CMDDEP=%d",CMDDEP);
    if (!prtopt(&lines,line)) return;
#endif /* NOSPL */

    sprintf(line,"CKMAXPATH=%d",CKMAXPATH);
    if (!prtopt(&lines,line)) return;

#ifdef MAXGETPATH
    sprintf(line,"MAXGETPATH=%d",MAXGETPATH);
    if (!prtopt(&lines,line)) return;
#endif /* MAXGETPATH */

#ifdef CMDBL
    sprintf(line,"CMDBL=%d",CMDBL);
    if (!prtopt(&lines,line)) return;
#endif /* CMDBL */

#ifdef VNAML
    sprintf(line,"VNAML=%d",VNAML);
    if (!prtopt(&lines,line)) return;
#endif /* VNAML */

#ifdef ARRAYREFLEN
    sprintf(line,"ARRAYREFLEN=%d",ARRAYREFLEN);
    if (!prtopt(&lines,line)) return;
#endif /* ARRAYREFLEN */

#ifdef FORDEPTH
    sprintf(line,"FORDEPTH=%d",FORDEPTH);
    if (!prtopt(&lines,line)) return;
#endif /* FORDEPTH */

#ifdef MAXTAKE
    sprintf(line,"MAXTAKE=%d",MAXTAKE);
    if (!prtopt(&lines,line)) return;
#endif /* MAXTAKE */

#ifdef MACLEVEL
    sprintf(line,"MACLEVEL=%d",MACLEVEL);
    if (!prtopt(&lines,line)) return;
#endif /* MACLEVEL */

#ifdef MAC_MAX
    sprintf(line,"MAC_MAX=%d",MAC_MAX);
    if (!prtopt(&lines,line)) return;
#endif /* MAC_MAX */

#ifdef MINPUTMAX
    sprintf(line,"MINPUTMAX=%d",MINPUTMAX);
    if (!prtopt(&lines,line)) return;
#endif /* MINPUTMAX */

#ifdef MSENDMAX
    sprintf(line,"MSENDMAX=%d",MSENDMAX);
    if (!prtopt(&lines,line)) return;
#endif /* MSENDMAX  */

#ifdef MAXDDIR
    sprintf(line,"MAXDDIR=%d",MAXDDIR);
    if (!prtopt(&lines,line)) return;
#endif /* MAXDDIR  */

#ifdef MAXDNUMS
    sprintf(line,"MAXDNUMS=%d",MAXDNUMS);
    if (!prtopt(&lines,line)) return;
#endif /* MAXDNUMS  */

#ifdef UNIX
    if (!prtopt(&lines,"UNIX")) return;
#endif /* UNIX */
#ifdef VMS
    if (!prtopt(&lines,"VMS")) return;
#ifdef __VMS_VER
    sprintf(line,"__VMS_VER=%d",__VMS_VER);
    if (!prtopt(&lines,line)) return;
#endif /* __VMS_VER */
#ifdef VMSV70
    if (!prtopt(&lines,"VMSV70")) return;
#endif /* VMSV70 */
#endif /* VMS */
#ifdef OLD_VMS
    if (!prtopt(&lines,"OLD_VMS")) return;
#endif /* OLD_VMS */
#ifdef vms
    if (!prtopt(&lines,"vms")) return;
#endif /* vms */
#ifdef VMSSHARE
    if (!prtopt(&lines,"VMSSHARE")) return;
#endif /* VMSSHARE */
#ifdef datageneral
    if (!prtopt(&lines,"datageneral")) return;
#endif /* datageneral */
#ifdef apollo
    if (!prtopt(&lines,"apollo")) return;
#endif /* apollo */
#ifdef aegis
    if (!prtopt(&lines,"aegis")) return;
#endif /* aegis */
#ifdef A986
    if (!prtopt(&lines,"A986")) return;
#endif /* A986 */
#ifdef AMIGA
    if (!prtopt(&lines,"AMIGA")) return;
#endif /* AMIGA */
#ifdef CONVEX9
    if (!prtopt(&lines,"CONVEX9")) return;
#endif /* CONVEX9 */
#ifdef CONVEX10
    if (!prtopt(&lines,"CONVEX10")) return;
#endif /* CONVEX9 */
#ifdef MAC
    if (!prtopt(&lines,"MAC")) return;
#endif /* MAC */
#ifdef AUX
    if (!prtopt(&lines,"AUX")) return;
#endif /* AUX */
#ifdef OS2
    if (!prtopt(&lines,"OS2")) return;
#ifdef NT
    if (!prtopt(&lines,"NT")) return;
#endif /* NT */
#endif /* OS2 */
#ifdef OSK
    if (!prtopt(&lines,"OS9")) return;
#endif /* OSK */
#ifdef MSDOS
    if (!prtopt(&lines,"MSDOS")) return;
#endif /* MSDOS */
#ifdef DIRENT
    if (!prtopt(&lines,"DIRENT")) return;
#endif /* DIRENT */
#ifdef SDIRENT
    if (!prtopt(&lines,"SDIRENT")) return;
#endif /* SDIRENT */
#ifdef NDIR
    if (!prtopt(&lines,"NDIR")) return;
#endif /* NDIR */
#ifdef XNDIR
    if (!prtopt(&lines,"XNDIR")) return;
#endif /* XNDIR */
#ifdef MATCHDOT
    if (!prtopt(&lines,"MATCHDOT")) return;
#endif /* MATCHDOT */
#ifdef SAVEDUID
    if (!prtopt(&lines,"SAVEDUID")) return;
#endif /* SAVEDUID */
#ifdef RENAME
    if (!prtopt(&lines,"RENAME")) return;
#endif /* RENAME */
#ifdef CK_TMPDIR
    if (!prtopt(&lines,"CK_TMPDIR")) return;
#endif /* CK_TMPDIR */
#ifdef NOCCTRAP
    if (!prtopt(&lines,"NOCCTRAP")) return;
#endif /* NOCCTRAP */
#ifdef CK_NEED_SIG
    if (!prtopt(&lines,"CK_NEED_SIG")) return;
#endif /* CK_NEED_SIG */
#ifdef CK_TTYFD
    if (!prtopt(&lines,"CK_TTYFD")) return;
#endif /* CK_TTYFD */
#ifdef NETCONN
    if (!prtopt(&lines,"NETCONN")) return;
#endif /* NETCONN */
#ifdef TCPSOCKET
    if (!prtopt(&lines,"TCPSOCKET")) return;
#ifdef NOTCPOPTS
    if (!prtopt(&lines,"NOTCPOPTS")) return;
#endif /* NOTCPOPTS */
#ifdef NOLISTEN
    if (!prtopt(&lines,"NOLISTEN")) return;
#endif /* NOLISTEN */
#ifdef SOL_SOCKET
    if (!prtopt(&lines,"SOL_SOCKET")) return;
#endif /* SOL_SOCKET */
#ifdef TCP_NODELAY
    if (!prtopt(&lines,"TDP_NODELAY")) return;
#endif /* TCP_NODELAY */
#ifdef HADDRLIST
    if (!prtopt(&lines,"HADDRLIST")) return;
#endif /* HADDRLIST */
#ifdef CK_SOCKS
    if (!prtopt(&lines,"CK_SOCKS")) return;
#endif /* CK_SOCKS */
#ifdef RLOGCODE
    if (!prtopt(&lines,"RLOGCODE")) return;
#endif /* RLOGCODE */
#ifdef CONGSPD
    if (!prtopt(&lines,"CONGSPD")) return;
#endif /* CONGSPDCK_SOCKS */
#endif /* TCPSOCKET */
#ifdef SUNX25
    if (!prtopt(&lines,"SUNX25")) return;
#endif /* SUNX25 */
#ifdef DECNET
    if (!prtopt(&lines,"DECNET")) return;
#endif /* DECNET */
#ifdef SUPERLAT
   if (!prtopt(&lines,"SUPERLAT")) return;
#endif /* SUPERLAT */
#ifdef NPIPE
    if (!prtopt(&lines,"NPIPE")) return;
#endif /* NPIPE */
#ifdef CK_NETBIOS
    if (!prtopt(&lines,"CK_NETBIOS")) return;
#endif /* CK_NETBIOS */
#ifdef ATT7300
    if (!prtopt(&lines,"ATT7300")) return;
#endif /* ATT7300 */
#ifdef ATT6300
    if (!prtopt(&lines,"ATT6300")) return;
#endif /* ATT6300 */
#ifdef HDBUUCP
    if (!prtopt(&lines,"HDBUUCP")) return;
#endif /* HDBUUCP */
#ifdef NOUUCP
    if (!prtopt(&lines,"NOUUCP")) return;
#endif /* NOUUCP */
#ifdef LONGFN
    if (!prtopt(&lines,"LONGFN")) return;
#endif /* LONGFN */
#ifdef RDCHK
    if (!prtopt(&lines,"RDCHK")) return;
#endif /* RDCHK */
#ifdef SELECT
    if (!prtopt(&lines,"SELECT")) return;
#endif /* SELECT */
#ifdef USLEEP
    if (!prtopt(&lines,"USLEEP")) return;
#endif /* USLEEP */
#ifdef NAP
    if (!prtopt(&lines,"NAP")) return;
#endif /* NAP */
#ifdef NAPHACK
    if (!prtopt(&lines,"NAPHACK")) return;
#endif /* NAPHACK */
#ifdef CK_POLL
    if (!prtopt(&lines,"CK_POLL")) return;
#endif /* CK_POLL */
#ifdef NOIEXTEN
    if (!prtopt(&lines,"NOIEXTEN")) return;
#endif /* NOIEXTEN */
#ifdef EXCELAN
    if (!prtopt(&lines,"EXCELAN")) return;
#endif /* EXCELAN */
#ifdef INTERLAN
    if (!prtopt(&lines,"INTERLAN")) return;
#endif /* INTERLAN */
#ifdef NOFILEH
    if (!prtopt(&lines,"NOFILEH")) return;
#endif /* NOFILEH */
#ifdef NOSYSIOCTLH
    if (!prtopt(&lines,"NOSYSIOCTLH")) return;
#endif /* NOSYSIOCTLH */
#ifdef DCLPOPEN
    if (!prtopt(&lines,"DCLPOPEN")) return;
#endif /* DCLPOPEN */
#ifdef NOSETBUF
    if (!prtopt(&lines,"NOSETBUF")) return;
#endif /* NOSETBUF */
#ifdef NOFDZERO
    if (!prtopt(&lines,"NOFDZERO")) return;
#endif /* NOFDZERO */
#ifdef NOPOPEN
    if (!prtopt(&lines,"NOPOPEN")) return;
#endif /* NOPOPEN */
#ifdef NOPARTIAL
    if (!prtopt(&lines,"NOPARTIAL")) return;
#endif /* NOPARTIAL */
#ifdef NOKVERBS
    if (!prtopt(&lines,"NOKVERBS")) return;
#endif /* NOKVERBS */
#ifdef NOSETREU
    if (!prtopt(&lines,"NOSETREU")) return;
#endif /* NOSETREU */
#ifdef _POSIX_SOURCE
    if (!prtopt(&lines,"_POSIX_SOURCE")) return;
#endif /* _POSIX_SOURCE */
#ifdef LCKDIR
    if (!prtopt(&lines,"LCKDIR")) return;
#endif /* LCKDIR */
#ifdef ACUCNTRL
    if (!prtopt(&lines,"ACUCNTRL")) return;
#endif /* ACUCNTRL */
#ifdef BSD4
    if (!prtopt(&lines,"BSD4")) return;
#endif /* BSD4 */
#ifdef BSD44
    if (!prtopt(&lines,"BSD44")) return;
#endif /* BSD44 */
#ifdef BSD41
    if (!prtopt(&lines,"BSD41")) return;
#endif /* BSD41 */
#ifdef BSD43
    if (!prtopt(&lines,"BSD43")) return;
#endif /* BSD43 */
#ifdef BSD29
    if (!prtopt(&lines,"BSD29")) return;
#endif /* BSD29 */
#ifdef BSDI
    if (!prtopt(&lines,"BSDI")) return;
#endif /* BSDI */
#ifdef __bsdi__
    if (!prtopt(&lines,"__bsdi__")) return;
#endif /* __bsdi__ */
#ifdef __NetBSD__
    if (!prtopt(&lines,"__NetBSD__")) return;
#endif /* __NetBSD__ */
#ifdef __FreeBSD__
    if (!prtopt(&lines,"__FreeBSD__")) return;
#endif /* __FreeBSD__ */
#ifdef __linux__
    if (!prtopt(&lines,"__linux__")) return;
#endif /* __linux__ */
#ifdef LINUX_HI_SPD
    if (!prtopt(&lines,"LINUX_HI_SPD")) return;
#endif /* LINUX_HI_SPD */
#ifdef LYNXOS
    if (!prtopt(&lines,"LYNXOS")) return;
#endif /* LYNXOS */
#ifdef V7
    if (!prtopt(&lines,"V7")) return;
#endif /* V7 */
#ifdef AIX370
    if (!prtopt(&lines,"AIX370")) return;
#endif /* AIX370 */
#ifdef RTAIX
    if (!prtopt(&lines,"RTAIX")) return;
#endif /* RTAIX */
#ifdef HPUX
    if (!prtopt(&lines,"HPUX")) return;
#endif /* HPUX */
#ifdef HPUXPRE65
    if (!prtopt(&lines,"HPUXPRE65")) return;
#endif /* HPUXPRE65 */
#ifdef DGUX
    if (!prtopt(&lines,"DGUX")) return;
#endif /* DGUX */
#ifdef DGUX430
    if (!prtopt(&lines,"DGUX430")) return;
#endif /* DGUX430 */
#ifdef DGUX540
    if (!prtopt(&lines,"DGUX540")) return;
#endif /* DGUX540 */
#ifdef sony_news
    if (!prtopt(&lines,"sony_news")) return;
#endif /* sony_news */
#ifdef CIE
    if (!prtopt(&lines,"CIE")) return;
#endif /* CIE */
#ifdef XENIX
    if (!prtopt(&lines,"XENIX")) return;
#endif /* XENIX */
#ifdef SCO_XENIX
    if (!prtopt(&lines,"SCO_XENIX")) return;
#endif /* SCO_XENIX */
#ifdef ISIII
    if (!prtopt(&lines,"ISIII")) return;
#endif /* ISIII */
#ifdef I386IX
    if (!prtopt(&lines,"I386IX")) return;
#endif /* I386IX */
#ifdef RTU
    if (!prtopt(&lines,"RTU")) return;
#endif /* RTU */
#ifdef PROVX1
    if (!prtopt(&lines,"PROVX1")) return;
#endif /* PROVX1 */
#ifdef PYRAMID
    if (!prtopt(&lines,"PYRAMID")) return;
#endif /* PYRAMID */
#ifdef TOWER1
    if (!prtopt(&lines,"TOWER1")) return;
#endif /* TOWER1 */
#ifdef UTEK
    if (!prtopt(&lines,"UTEK")) return;
#endif /* UTEK */
#ifdef ZILOG
    if (!prtopt(&lines,"ZILOG")) return;
#endif /* ZILOG */
#ifdef TRS16
    if (!prtopt(&lines,"TRS16")) return;
#endif /* TRS16 */
#ifdef MINIX
    if (!prtopt(&lines,"MINIX")) return;
#endif /* MINIX */
#ifdef C70
    if (!prtopt(&lines,"C70")) return;
#endif /* C70 */
#ifdef AIXPS2
    if (!prtopt(&lines,"AIXPS2")) return;
#endif /* AIXPS2 */
#ifdef AIXRS
    if (!prtopt(&lines,"AIXRS")) return;
#endif /* AIXRS */
#ifdef UTSV
    if (!prtopt(&lines,"UTSV")) return;
#endif /* UTSV */
#ifdef ATTSV
    if (!prtopt(&lines,"ATTSV")) return;
#endif /* ATTSV */
#ifdef SVR3
    if (!prtopt(&lines,"SVR3")) return;
#endif /* SVR3 */
#ifdef SVR4
    if (!prtopt(&lines,"SVR4")) return;
#endif /* SVR4 */
#ifdef DELL_SVR4
    if (!prtopt(&lines,"DELL_SVR4")) return;
#endif /* DELL_SVR4 */
#ifdef ICL_SVR4
    if (!prtopt(&lines,"ICL_SVR4")) return;
#endif /* ICL_SVR4 */
#ifdef OSF
    if (!prtopt(&lines,"OSF")) return;
#endif /* OSF */
#ifdef OSF1
    if (!prtopt(&lines,"OSF1")) return;
#endif /* OSF1 */
#ifdef __OSF
    if (!prtopt(&lines,"__OSF")) return;
#endif /* __OSF */
#ifdef __OSF__
    if (!prtopt(&lines,"__OSF__")) return;
#endif /* __OSF__ */
#ifdef __osf__
    if (!prtopt(&lines,"__osf__")) return;
#endif /* __osf__ */
#ifdef __OSF1
    if (!prtopt(&lines,"__OSF1")) return;
#endif /* __OSF1 */
#ifdef __OSF1__
    if (!prtopt(&lines,"__OSF1__")) return;
#endif /* __OSF1__ */
#ifdef PTX
    if (!prtopt(&lines,"PTX")) return;
#endif /* PTX */
#ifdef POSIX
    if (!prtopt(&lines,"POSIX")) return;
#endif /* POSIX */
#ifdef Plan9
    if (!prtopt(&lines,"Plan9")) return;
#endif /* Plan9 */
#ifdef SOLARIS
    if (!prtopt(&lines,"SOLARIS")) return;
#endif /* SOLARIS */
#ifdef SUNOS4
    if (!prtopt(&lines,"SUNOS4")) return;
#endif /* SUNOS4 */
#ifdef SUN4S5
    if (!prtopt(&lines,"SUN4S5")) return;
#endif /* SUN4S5 */
#ifdef ENCORE
    if (!prtopt(&lines,"ENCORE")) return;
#endif /* ENCORE */
#ifdef ultrix
    if (!prtopt(&lines,"ultrix")) return;
#endif
#ifdef sxaE50
    if (!prtopt(&lines,"sxaE50")) return;
#endif
#ifdef mips
    if (!prtopt(&lines,"mips")) return;
#endif
#ifdef MIPS
    if (!prtopt(&lines,"MIPS")) return;
#endif
#ifdef vax
    if (!prtopt(&lines,"vax")) return;
#endif
#ifdef VAX
    if (!prtopt(&lines,"VAX")) return;
#endif
#ifdef alpha
    if (!prtopt(&lines,"alpha")) return;
#endif
#ifdef ALPHA
    if (!prtopt(&lines,"ALPHA")) return;
#endif
#ifdef __ALPHA
    if (!prtopt(&lines,"__ALPHA")) return;
#endif
#ifdef __alpha
    if (!prtopt(&lines,"__alpha")) return;
#endif
#ifdef __AXP
    if (!prtopt(&lines,"__AXP")) return;
#endif
#ifdef AXP
    if (!prtopt(&lines,"AXP")) return;
#endif
#ifdef axp
    if (!prtopt(&lines,"axp")) return;
#endif
#ifdef __ALPHA__
    if (!prtopt(&lines,"__ALPHA__")) return;
#endif
#ifdef __alpha__
    if (!prtopt(&lines,"__alpha__")) return;
#endif
#ifdef sun
    if (!prtopt(&lines,"sun")) return;
#endif
#ifdef sun3
    if (!prtopt(&lines,"sun3")) return;
#endif
#ifdef sun386
    if (!prtopt(&lines,"sun386")) return;
#endif
#ifdef _SUN
    if (!prtopt(&lines,"_SUN")) return;
#endif
#ifdef sun4
    if (!prtopt(&lines,"sun4")) return;
#endif
#ifdef sparc
    if (!prtopt(&lines,"sparc")) return;
#endif
#ifdef _CRAY
    if (!prtopt(&lines,"_CRAY")) return;
#endif /* _CRAY */
#ifdef NEXT33
    if (!prtopt(&lines,"NEXT33")) return;
#endif
#ifdef NEXT
    if (!prtopt(&lines,"NEXT")) return;
#endif
#ifdef NeXT
    if (!prtopt(&lines,"NeXT")) return;
#endif
#ifdef MACH
    if (!prtopt(&lines,"MACH")) return;
#endif
#ifdef sgi
    if (!prtopt(&lines,"sgi")) return;
#endif
#ifdef M_SYS5
    if (!prtopt(&lines,"M_SYS5")) return;
#endif
#ifdef __SYSTEM_FIVE
    if (!prtopt(&lines,"__SYSTEM_FIVE")) return;
#endif
#ifdef sysV
    if (!prtopt(&lines,"sysV")) return;
#endif
#ifdef M_XENIX				/* SCO Xenix V and UNIX/386 */
    if (!prtopt(&lines,"M_XENIX")) return;
#endif 
#ifdef M_UNIX				/* SCO UNIX */
    if (!prtopt(&lines,"M_UNIX")) return;
#endif
#ifdef _M_UNIX				/* SCO UNIX 3.2v4 = ODT 2.0 */
    if (!prtopt(&lines,"_M_UNIX")) return;
#endif
#ifdef M_I586
    if (!prtopt(&lines,"M_I586")) return;
#endif
#ifdef _M_I586
    if (!prtopt(&lines,"_M_I586")) return;
#endif
#ifdef i586
    if (!prtopt(&lines,"i586")) return;
#endif
#ifdef M_I486
    if (!prtopt(&lines,"M_I486")) return;
#endif
#ifdef _M_I486
    if (!prtopt(&lines,"_M_I486")) return;
#endif
#ifdef i486
    if (!prtopt(&lines,"i486")) return;
#endif
#ifdef M_I386
    if (!prtopt(&lines,"M_I386")) return;
#endif
#ifdef _M_I386
    if (!prtopt(&lines,"_M_I386")) return;
#endif
#ifdef i386
    if (!prtopt(&lines,"i386")) return;
#endif
#ifdef i286
    if (!prtopt(&lines,"i286")) return;
#endif
#ifdef M_I286
    if (!prtopt(&lines,"M_I286")) return;
#endif
#ifdef mc68000
    if (!prtopt(&lines,"mc68000")) return;
#endif
#ifdef mc68010
    if (!prtopt(&lines,"mc68010")) return;
#endif
#ifdef mc68020
    if (!prtopt(&lines,"mc68020")) return;
#endif
#ifdef mc68030
    if (!prtopt(&lines,"mc68030")) return;
#endif
#ifdef mc68040
    if (!prtopt(&lines,"mc68040")) return;
#endif
#ifdef M_68000
    if (!prtopt(&lines,"M_68000")) return;
#endif
#ifdef M_68010
    if (!prtopt(&lines,"M_68010")) return;
#endif
#ifdef M_68020
    if (!prtopt(&lines,"M_68020")) return;
#endif
#ifdef M_68030
    if (!prtopt(&lines,"M_68030")) return;
#endif
#ifdef M_68040
    if (!prtopt(&lines,"M_68040")) return;
#endif
#ifdef m68k
    if (!prtopt(&lines,"m68k")) return;
#endif
#ifdef m88k
    if (!prtopt(&lines,"m88k")) return;
#endif
#ifdef pdp11
    if (!prtopt(&lines,"pdp11")) return;
#endif
#ifdef iAPX
    if (!prtopt(&lines,"iAPX")) return;
#endif
#ifdef __hp9000s800
    if (!prtopt(&lines,"__hp9000s800")) return;
#endif
#ifdef __hp9000s700
    if (!prtopt(&lines,"__hp9000s700")) return;
#endif
#ifdef __hp9000s500
    if (!prtopt(&lines,"__hp9000s500")) return;
#endif
#ifdef __hp9000s300
    if (!prtopt(&lines,"__hp9000s300")) return;
#endif
#ifdef __hp9000s200
    if (!prtopt(&lines,"__hp9000s200")) return;
#endif
#ifdef AIX
    if (!prtopt(&lines,"AIX")) return;
#endif
#ifdef _AIXFS
    if (!prtopt(&lines,"_AIXFS")) return;
#endif
#ifdef u370
    if (!prtopt(&lines,"u370")) return;
#endif
#ifdef u3b
    if (!prtopt(&lines,"u3b")) return;
#endif
#ifdef u3b2
    if (!prtopt(&lines,"u3b2")) return;
#endif
#ifdef multimax
    if (!prtopt(&lines,"multimax")) return;
#endif
#ifdef balance
    if (!prtopt(&lines,"balance")) return;
#endif
#ifdef ibmrt
    if (!prtopt(&lines,"ibmrt")) return;
#endif
#ifdef _IBMRT
    if (!prtopt(&lines,"_IBMRT")) return;
#endif
#ifdef ibmrs6000
    if (!prtopt(&lines,"ibmrs6000")) return;
#endif
#ifdef _AIX
    if (!prtopt(&lines,"_AIX")) return;
#endif /* _AIX */
#ifdef _IBMR2
    if (!prtopt(&lines,"_IBMR2")) return;
#endif
#ifdef UNIXWARE
    if (!prtopt(&lines,"UNIXWARE")) return;
#endif
#ifdef QNX
    if (!prtopt(&lines,"QNX")) return;
#ifdef __QNX__
    if (!prtopt(&lines,"__QNX__")) return;
#ifdef __16BIT__
    if (!prtopt(&lines,"__16BIT__")) return;
#endif
#ifdef CK_QNX16
    if (!prtopt(&lines,"CK_QNX16")) return;
#endif
#ifdef __32BIT__
    if (!prtopt(&lines,"__32BIT__")) return;
#endif
#ifdef CK_QNX32
    if (!prtopt(&lines,"CK_QNX32")) return;
#endif
#endif /* __QNX__ */
#endif /* QNX */

#ifdef __STRICT_BSD__
    if (!prtopt(&lines,"__STRICT_BSD__")) return;
#endif
#ifdef __STRICT_ANSI__
    if (!prtopt(&lines,"__STRICT_ANSI__")) return;
#endif
#ifdef _ANSI_C_SOURCE
    if (!prtopt(&lines,"_ANSI_C_SOURCE")) return;
#endif
#ifdef __STDC__
    if (!prtopt(&lines,"__STDC__")) return;
#endif
#ifdef __DECC
    if (!prtopt(&lines,"__DECC")) return;
#ifdef __DECC_VER
    sprintf(line,"__DECC_VER=%d",__DECC_VER);
    if (!prtopt(&lines,line)) return;
#endif /* __DECC_VER */
#endif /* __DECC */
#ifdef __GNUC__				/* gcc in ansi mode */
    if (!prtopt(&lines,"__GNUC__")) return;
#endif
#ifdef GNUC				/* gcc in traditional mode */
    if (!prtopt(&lines,"GNUC")) return;
#endif
#ifdef __WATCOMC__
    if (!prtopt(&lines,"__WATCOMC__")) return;
#endif
#ifdef CK_ANSIC
    if (!prtopt(&lines,"CK_ANSIC")) return;
#endif
#ifdef CK_ANSILIBS
    if (!prtopt(&lines,"CK_ANSILIBS")) return;
#endif
#ifdef _XOPEN_SOURCE
    if (!prtopt(&lines,"_XOPEN_SOURCE")) return;
#endif
#ifdef _ALL_SOURCE
    if (!prtopt(&lines,"_ALL_SOURCE")) return;
#endif
#ifdef _SC_JOB_CONTROL
    if (!prtopt(&lines,"_SC_JOB_CONTROL")) return;
#endif
#ifdef _POSIX_JOB_CONTROL
    if (!prtopt(&lines,"_POSIX_JOB_CONTROL")) return;
#endif
#ifdef CK_POSIX_SIG
    if (!prtopt(&lines,"CK_POSIX_SIG")) return;
#endif
#ifdef SVR3JC
    if (!prtopt(&lines,"SVR3JC")) return;
#endif
#ifdef _386BSD
    if (!prtopt(&lines,"_386BSD")) return;
#endif
#ifdef _BSD
    if (!prtopt(&lines,"_BSD")) return;
#endif
#ifdef TERMIOX
    if (!prtopt(&lines,"TERMIOX")) return;
#endif /* TERMIOX */
#ifdef STERMIOX
    if (!prtopt(&lines,"STERMIOX")) return;
#endif /* STERMIOX */
#ifdef CK_CURSES
    if (!prtopt(&lines,"CK_CURSES")) return;
#endif /* CK_CURSES */
#ifdef CK_NEWTERM
    if (!prtopt(&lines,"CK_NEWTERM")) return;
#endif /* CK_NEWTERM */
#ifdef CK_WREFRESH
    if (!prtopt(&lines,"CK_WREFRESH")) return;
#endif /* CK_WREFRESH */
#ifdef CK_PCT_BAR
    if (!prtopt(&lines,"CK_PCT_BAR")) return;
#endif /* CK_PCT_BAR */
#ifdef CK_DTRCD
    if (!prtopt(&lines,"CK_DTRCD")) return;
#endif /* CK_DTRCD */
#ifdef CK_DTRCTS
    if (!prtopt(&lines,"CK_DTRCTS")) return;
#endif /* CK_DTRCTS */
#ifdef CK_RTSCTS
    if (!prtopt(&lines,"CK_RTSCTS")) return;
#endif /* CK_RTSCTS */
#ifdef POSIX_CRTSCTS
    if (!prtopt(&lines,"POSIX_CRTSCTS")) return;
#endif /* POSIX_CRTSCTS */
#ifdef CK_DSYSINI
    if (!prtopt(&lines,"CK_DSYSINI")) return;
#endif /* CK_DSYSINI */
#ifdef CK_SYSINI
    if (!prtopt(&lines,"CK_SYSINI")) return;
#endif /* CK_SYSINI */
#ifdef CK_INI_A
    if (!prtopt(&lines,"CK_INI_A")) return;
#endif /* CK_INI_A */
#ifdef CK_INI_B
    if (!prtopt(&lines,"CK_INI_B")) return;
#endif /* CK_INI_B */
#ifdef CK_TTGWSIZ
    if (!prtopt(&lines,"CK_TTGWSIZ")) return;
#endif /* CK_TTGWSIZ */
#ifdef CK_NAWS
    if (!prtopt(&lines,"CK_NAWS")) return;
#endif /* CK_NAWS */
#ifdef DCMDBUF
    if (!prtopt(&lines,"DCMDBUF")) return;
#endif /* DCMDBUF */
#ifdef CK_RECALL
    if (!prtopt(&lines,"CK_RECALL")) return;
#endif /* CK_RECALL */
#ifdef CLSOPN
    if (!prtopt(&lines,"CLSOPN")) return;
#endif /* CLSOPN */
#ifdef STRATUS
    if (!prtopt(&lines,"STRATUS")) return;
#endif /* STRATUS */
#ifdef __VOS__
    if (!prtopt(&lines,"__VOS__")) return;
#endif /* __VOS__ */
#ifdef STRATUSX25
    if (!prtopt(&lines,"STRATUSX25")) return;
#endif /* STRATUSX25 */
#ifdef OS2MOUSE 
    if (!prtopt(&lines,"OS2MOUSE") ) return;
#endif /* OS2MOUSE */
#ifdef CK_REXX
    if (!prtopt(&lines,"CK_REXX") ) return;
#endif /* CK_REXX */
#ifdef CK_TIMERS
    if (!prtopt(&lines,"CK_TIMERS") ) return;
#endif /* CK_TIMERS */
    if (!prtopt(&lines,(char *)0)) return;
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
shotcs(csl,csr) int csl, csr; {		/* Show terminal character set */
#ifndef NOCSETS
#ifdef OS2
    extern struct _vtG G[4], *GL, *GR;

    printf(" Terminal character-sets:\n");
#ifndef CKOUNI
    printf("    Local: %s\n",
        csl == FC_TRANSP ? "transparent" : fcsinfo[csl].keyword );
    printf("   Remote: %sG0: %s (%s)\n",
        GL == &G[0] ? "GL->" : GR == &G[0] ? "GR->" : "    ",
        G[0].designation == FC_TRANSP ? "transparent" :
        G[0].designation == FC_DECSPEC ? "dec-special" :
	G[0].designation == FC_DECTECH ? "dec-technical" :
        fcsinfo[G[0].designation].keyword,
        G[0].designation == FC_TRANSP ? "" :
        G[0].size == cs94 ? "94 chars" :
        G[0].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG1: %s (%s)\n",
        GL == &G[1] ? "GL->" : GR == &G[1] ? "GR->" : "    ",
        G[1].designation == FC_TRANSP ? "transparent" :
        G[1].designation == FC_DECSPEC ? "dec-special" :
	G[1].designation == FC_DECTECH ? "dec-technical" :
	fcsinfo[G[1].designation].keyword,
        G[1].designation == FC_TRANSP ? "" :
        G[1].size == cs94 ? "94 chars" :
        G[1].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG2: %s (%s)\n",
        GL == &G[2] ? "GL->" : GR == &G[2] ? "GR->" : "    ",
        G[2].designation == FC_TRANSP ? "transparent" :
        G[2].designation == FC_DECSPEC ? "dec-special" :
	G[2].designation == FC_DECTECH ? "dec-technical" :
        fcsinfo[G[2].designation].keyword,
        G[2].designation == FC_TRANSP ? "" :
        G[2].size == cs94 ? "94 chars" :
        G[2].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG3: %s (%s)\n",
        GL == &G[3] ? "GL->" : GR == &G[3] ? "GR->" : "    ",
        G[3].designation == FC_TRANSP ? "transparent" :
        G[3].designation == FC_DECSPEC ? "dec-special" :
	G[3].designation == FC_DECTECH ? "dec-technical" :
        fcsinfo[G[3].designation].keyword,
        G[3].designation == FC_TRANSP ? "" :
        G[3].size == cs94 ? "94 chars" :
        G[3].size == cs96 ? "96 chars" : "multi-byte" );
#else /* CKOUNI */
    printf("    Local: %s\n",
	   isunicode() ? "Unicode" :
	   csl == TX_TRANSP ? "transparent" :
	   csl == TX_UNDEF ? "undefined" : txrinfo[csl]->keywd );
    printf("   Remote: %sG0: %s (%s)\n",
	   GL == &G[0] ? "GL->" : GR == &G[0] ? "GR->" : "    ",
	   txrinfo[G[0].designation]->keywd,
	   G[0].size == cs94 ? "94 chars" :
	   G[0].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG1: %s (%s)\n",
	   GL == &G[1] ? "GL->" : GR == &G[1] ? "GR->" : "    ",
	   txrinfo[G[1].designation]->keywd,
	   G[1].size == cs94 ? "94 chars" :
	   G[1].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG2: %s (%s)\n",
	   GL == &G[2] ? "GL->" : GR == &G[2] ? "GR->" : "    ",
	   txrinfo[G[2].designation]->keywd,
	   G[2].size == cs94 ? "94 chars" :
	   G[2].size == cs96 ? "96 chars" : "multi-byte" );
    printf("           %sG3: %s (%s)\n",
	   GL == &G[3] ? "GL->" : GR == &G[3] ? "GR->" : "    ",
	   txrinfo[G[3].designation]->keywd,
	   G[3].size == cs94 ? "94 chars" :
	   G[3].size == cs96 ? "96 chars" : "multi-byte" );
#endif /* CKOUNI */
#else /* OS2 */
#ifndef MAC
    char *s;
#ifdef CK_ANSIC
    int gettcs(int, int);
#else
    int gettcs();
#endif /* CK_ANSIC */

    printf(" Terminal character-set: ");
    if (csl == csr) {			/* No translation */
	printf("transparent\n");
    } else {				/* Translation */
	printf("%s (remote) %s (local)\n",
	       fcsinfo[csr].keyword,fcsinfo[csl].keyword);
	if (csr != csl) {
	    switch(gettcs(csr,csl)) {
	      case TC_USASCII:  s = "ascii";        break;
	      case TC_1LATIN:   s = "latin1-iso";   break;
	      case TC_2LATIN:   s = "latin2-iso";   break;
	      case TC_CYRILL:   s = "cyrillic-iso"; break;
	      case TC_JEUC:     s = "japanese-euc"; break;
	      case TC_HEBREW:   s = "hebrew-iso";   break;
	      default:          s = "transparent";  break;
	    }
	    if (strcmp(s,fcsinfo[csl].keyword) &&
		strcmp(s,fcsinfo[csr].keyword))
	      printf("                         (via %s)\n",s);
	}
    }
#endif /* MAC */
#endif /* OS2 */
#endif /* NOCSETS */
}
#endif /* NOSHOW */

#ifndef NOSETKEY
VOID
shostrdef(s) CHAR * s; {
    CHAR ch;
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
}

VOID
shokeycode(c) int c; {
    KEY ch;
    CHAR *s;
#ifdef NEWMAPKEY
    con_event km;
#else /* NEWMAPKEY */
    int km;
#endif /* NEWMAPKEY */

#ifdef OS2
    extern int mskkeys;

    if (c >= KMSIZE) {
	bleep(BP_FAIL);
	return;
    }
    s = keyname(c);
    printf(" Key code \\%d %s => ", mskkeys ? cktomsk(c) : c, 
	    s == NULL ? "" : s);
#else /* OS2 */
    printf(" Key code \\%d => ", c);
#endif /* OS2 */

#ifndef NEWMAPKEY
    km = mapkey(c);

#ifndef NOKVERBS
    if (IS_KVERB(km)) {			/* \Kverb? */
	int i, kv;
	kv = km & ~(F_KVERB);
	printf("Verb: ");
	for (i = 0; i < nkverbs; i++)
	  if (kverbs[i].kwval == kv) {
	      printf("\\K%s",kverbs[i].kwd);
	      break;
	  }
	printf("\n");
    } else
#endif /* NOKVERBS */
      if (IS_CSI(km)) {
	  int xkm = km & 0xFF;
	  if (xkm <= 32 || xkm >= 127)
	    printf("String: \\{27}[\\{%d}\n",xkm);
	  else 
	    printf("String: \\{27}[%c\n",xkm);
      } else if (IS_ESC(km)) {
	  int xkm = km & 0xFF;
	  if (xkm <= 32 || xkm >= 127)
	    printf("String: \\{27}\\{%d}\n",xkm);
	  else 
	    printf("String: \\{27}%c\n",xkm);
      } else if (macrotab[c]) {		/* See if there's a macro */
	  printf("String: ");		/* If so, display its definition */
	  s = macrotab[c];
	  shostrdef(s);
	  printf("\n");
#ifndef NOKVERBS
    } else if (km >= 0x100) {		/* This means "undefined" */
	printf("Undefined\n");
#endif /* NOKVERBS */
    } else {				/* No macro, show single character */
	printf("Character: ");
	ch = km;
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
#else /* NEWMAPKEY */
    km = mapkey(c);

    switch (km.type) {
#ifndef NOKVERBS
      case kverb: {
	  int i, kv;
	  kv = km.kverb.id & ~(F_KVERB);
	  printf("Verb: ");
	  for (i = 0; i < nkverbs; i++) {
	      if (kverbs[i].kwval == kv) {
		  printf("\\K%s",kverbs[i].kwd);
		  break;
	      }
	  }
	  printf("\n");
	  break;
      }
#endif /* NOKVERBS */
      case csi: {
	  int xkm = km.csi.key & 0xFF;
	  if (xkm <= 32 || xkm >= 127)
	    printf("String: \\{27}[\\{%d}\n",xkm);
	  else 
	    printf("String: \\{27}[%c\n",xkm);
	  break;
      }
      case esc: {
	  int xkm = km.esc.key & 0xFF;
	  if (xkm <= 32 || xkm >= 127)
	    printf("String: \\{27}\\{%d}\n",xkm);
	  else 
	    printf("String: \\{27}%c\n",xkm);
	  break;
      }
      case macro: {
	  printf("String: ");		/* Macro, display its definition */
	  shostrdef(km.macro.string);
	  printf("\n");
	  break;
      }
      case error: {
	  printf("Undefined\n");
	  break;
      }
      case key: {
	  printf("Character: ");
	  ch = km.key.scancode;
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
	  break;
      }
    }
#endif /* NEWMAPKEY */
}
#endif /* NOSETKEY */

#ifndef NOSHOW
#ifndef NOLOCAL
#ifdef OS2
extern char htab[];
void
shotabs()
{
    int i;

    printf("Tab Stops:\n\n");
    for ( i = 1; i <= 70; i++ )   
        printf("%c",htab[i]=='T'?'T':'-');
    printf("\n1.......10........20........30........40........50........60\
........70\n\n");
    for ( ; i <= 140; i++ )
        printf("%c",htab[i]=='T'?'T':'-');
    printf("\n........80........90.......100.......110.......120.......130\
.......140\n\n");
    for ( ; i <= 210; i++ )
        printf("%c",htab[i]=='T'?'T':'-');
    printf("\n.......150.......160.......170.......180.......190.......200\
.......210\n\n");
    for ( ; i <= 255; i++ )
        printf("%c",htab[i]=='T'?'T':'-');
    printf("\n.......220.......230.......240.......250..255\n");
}

#endif /* OS2 */
#endif /* NOLOCAL */
#endif /* NOSHOW */

#ifndef NOSHOW
#ifndef NOLOCAL
VOID
shotrm() {
    char *s;

#ifdef PCFONTS
    int i;
    char *font;

    if ( IsOS2FullScreen() ) {		/* Determine the font name */
        if ( !os2LoadPCFonts() ) {
            for ( i=0; i <ntermfont; i++ ) {
                if ( tt_font == termfont[i].kwval ) {
		    font = termfont[i].kwd;
		    break;
		}       
	    }
	} else {
            font = "(DLL not available)";
	}
    } else {
        font =     "(full screen only)";
    }
#endif /* PCFONTS */

#ifndef OS2
    printf("\n");
#endif /* OS2 */

    printf("Terminal parameters:\n");
    printf(" %19s: %1d%-12s  %13s: %1d%-14s\n",
	   "Bytesize: Command",
           (cmdmsk == 0377) ? 8 : 7, 
           " bits","Terminal",
           (cmask == 0377) ? 8 : 7," bits");
#ifdef OS2
    printf(" %19s: %-13s","Type",
	   (tt_type >= 0 && tt_type <= max_tt) ?
	   tt_info[tt_type].x_name :
	   "unknown" );
    if (tt_type >= 0 && tt_type <= max_tt)
      if (strlen(tt_info[tt_type].x_id))
	printf("  %13s: <ESC>%s","ID",tt_info[tt_type].x_id);
    printf("\n");
#else
    s = getenv("TERM");
    printf(" %19s: %-13s\n","Type", s ? s : "(unknown)");
#endif /* OS2 */
    printf(" %19s: %-13s  %13s: %-15s\n","Echo",
	   duplex ? "local" : "remote","Locking-shift",showoff(sosi));
    printf(" %19s: %-13s  %13s: %-15s\n","Newline-mode",
	   showoff(tnlm),"Cr-display",tt_crd ? "crlf" : "normal");
#ifdef OS2
    printf(" %19s: %-13s  %13s: %-15s\n","Cursor",
	   (tt_cursor == 2) ? "full" :
	   (tt_cursor == 1) ? "half" : "underline",
#ifdef CK_AUTODL            
	   "autodownload",autodl ? "on" : "off"
#else /* CK_AUTODL */
	   "", ""
#endif /* CK_AUTODL */
	   );
#endif /* OS2 */
#ifdef OS2
    printf(" %19s: %-13s  %13s: %-15s\n","Arrow-keys",
	   tt_arrow ? "application" : "cursor",
           "Keypad-mode", tt_keypad ? "application" : "numeric"
	   );

    /* Just to make sure we are using current info */
    updanswerbk();

    /*
       This line doesn't end with '\n' because the answerback string
       is terminated with a newline
    */
    printf(" %19s: %-13s  %13s: %-15s","Answerback",
	   showoff(tt_answer),"response",answerback);
    switch ( tt_bell ) {
      case XYB_NONE:
	s = "none";
	break;
      case XYB_VIS:
	s= "visible";
	break;
      case XYB_AUD | XYB_BEEP:
	s="beep";
	break;
      case XYB_AUD | XYB_SYS:
	s="system sounds";
	break;
      default:
	s="(unknown)";
    }
    printf(" %19s: %-13s  %13s: %-15s\n","Bell",s,
	   "Wrap",showoff(tt_wrap));
    printf(" %19s: %-13d  %13s: %-15d\n","Transmit-timeout",tt_ctstmo,
	   "Output-pacing",tt_pacing);
    printf(" %19s: %-13s  %13s: %-15d\n","Roll-mode",
	   tt_roll[VTERM]?"insert":"overwrite","Scrollback", tt_scrsize[VTERM]
	   );
#endif /* OS2 */

#ifdef CK_APC
    if (apcstatus == APC_ON) s = "on";
    else if (apcstatus == APC_OFF) s = "off";
    else if (apcstatus == APC_UNCH) s = "unchecked";
    printf(" %19s: %-13s  %13s: %-15s\n",
	   "APC", s,
#ifdef OS2
#ifdef PCFONTS
	   "Font",font
#else /* PCFONTS */
	   "",""
#endif /* PCFONTS */
#else
#ifdef CK_AUTODL
	   "Autodownload", autodl ? "on" : "off"
#else
	   "",""
#endif /* CK_AUTODL */
#endif /* OS2 */
	   );
#endif /* CK_APC */

#ifdef OS2                               
    {
        char cpbuf[50];
	int cplist[8], cps;
	cps = os2getcplist(cplist, sizeof(cplist));
	sprintf(cpbuf,"%3d,%3d,%3d,%3d", 
		cps > 1 ? cplist[1] : 0, cps > 2 ? cplist[2] : 0,
		cps > 3 ? cplist[3] : 0, cps > 4 ? cplist[4] : 0 );
	printf(" %19s: %-13d  %13s: %-15s\n","Code-page: active",
	       os2getcp(),"available",cpbuf);
    }
#endif /* OS2 */

#ifdef CK_TTGWSIZ			/* Console terminal screen size */
#ifdef OS2
    if ( tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0 )
      ttgwsiz();			/* Try to get latest size */
    printf(" %19s: %-13d  %13s: %-15d\n","Height",tt_rows[VTERM],
           "Width",tt_cols[VTERM]);
#else /* OS2 */
    ttgwsiz();				/* Try to get latest size */
    printf(" %19s: %-13d  %13s: %-15d\n","Height",tt_rows, "Width", tt_cols);
#endif /* OS2 */
#else
#endif /* CK_TTGWSIZ */

#ifdef OS2
    if (updmode == tt_updmode)
      if (updmode == TTU_FAST)
	s = "fast (fast)";
      else
	s = "smooth (smooth)";
    else
      if (updmode == TTU_FAST)
	s = "fast (smooth)";
      else
	s = "smooth (fast)";

    printf(" %19s: %-13s  %13s: %-15d\n","Screen-update: mode",s,
	   "update freq",tt_update);
#endif /* OS2 */
    printf(" %19s: %-13s  %13s: %-15s\n","Debug",
	   showoff(debses),"Session log", seslog? sesfil : "(none)" ); 
#ifdef OS2
    {
        USHORT row, col;
        char * colors[16] = {
            "black","blue","green","cyan","red","magenta","brown","lgray",
            "dgray","lblue","lgreen","lcyan","lred","lmagent","yellow","white"
	};
        printf("\n Color:");
#ifndef ONETERMUPD
        GetCurPos( &row, &col );
        WrtCharStrAtt("border",    6, row, 9, &colorborder );
	WrtCharStrAtt("debug",     5, row, 17, &colordebug );
        WrtCharStrAtt("helptext",  8, row, 25, &colorhelp );
	WrtCharStrAtt("reverse",   7, row, 34, &colorreverse );
        WrtCharStrAtt("select",    6, row, 42, &colorselect );
        WrtCharStrAtt("status",    6, row, 50, &colorstatus );
        WrtCharStrAtt("terminal",  8, row, 58, &colornormal );
	WrtCharStrAtt("underline",  9, row, 67, &colorunderline );
#endif /* ONETERMUPD */
        row = VscrnGetCurPos(VCMD)->y+1;
        VscrnWrtCharStrAtt(VCMD, "border",    6, row, 9, &colorborder );
	VscrnWrtCharStrAtt(VCMD, "debug",     5, row, 17, &colordebug );
        VscrnWrtCharStrAtt(VCMD, "helptext",  8, row, 25, &colorhelp );
	VscrnWrtCharStrAtt(VCMD, "reverse",   7, row, 34, &colorreverse );
        VscrnWrtCharStrAtt(VCMD, "select",    6, row, 42, &colorselect );
        VscrnWrtCharStrAtt(VCMD, "status",    6, row, 50, &colorstatus );
        VscrnWrtCharStrAtt(VCMD, "terminal",  8, row, 58, &colornormal );
	VscrnWrtCharStrAtt(VCMD, "underline",  9, row, 67, &colorunderline );
        printf( "\n" );

        /* Foreground color names */
        printf("%6s: %-8s%-8s%-9s%-8s%-8s%-8s%-9s%-9s\n","fore",
		"",
		colors[colordebug&0x0F],
		colors[colorhelp&0x0F],
		colors[colorreverse&0x0F],
		colors[colorselect&0x0F],
		colors[colorstatus&0x0F],
		colors[colornormal&0x0F],
		colors[colorunderline&0x0F] );

        /* Background color names */
        printf("%6s: %-8s%-8s%-9s%-8s%-8s%-8s%-9s%-9s\n","back",
		colors[colorborder],
		colors[colordebug>>4],
		colors[colorhelp>>4],
		colors[colorreverse>>4],
		colors[colorselect>>4],
		colors[colorstatus>>4],
		colors[colornormal>>4],
		colors[colorunderline>>4] );
    }
#endif /* OS2 */
    printf("\n");
    printf(" CONNECT-mode escape character: %d (Ctrl-%c, %s): %s\n",
	   escape,ctl(escape),(escape == 127 ? "DEL" : ccntab[escape]),
	   nm[tt_escape]
	   );
#ifdef OS2
    printf(" See SHOW CHARACTER-SETS for character-set info\n");
#else /* OS2 */
#ifndef NOCSETS
    shotcs(tcsl,tcsr);		/* Show terminal character sets */
#endif /* NOCSETS */
#endif /* OS2 */

#ifdef UNIX
#ifndef NOJC
    printf(" %19s: %-13s\n\n","Suspend", showoff(suspend));
#endif /* NOJC */
#endif /* UNIX */
}
#endif /* NOLOCAL */
#endif /* NOSHOW */

#ifndef NOSHOW
#ifndef NOLOCAL
#ifdef OS2MOUSE

void
shomou() {
    int button, event, id, i;
    char * name = "";

    printf("Mouse settings:\n");
    printf("   Active:         %s\n\n",showoff(tt_mouse));

    for ( button = 0; button < MMBUTTONMAX; button++ )
      for ( event = 0; event < MMEVENTSIZE; event++ )
	if ( mousemap[button][event].type != error )
	  switch ( mousemap[button][event].type ) {
	    case key:
	      printf("   %s = Character: %c \\%d\n",
		     mousename(button,event),
		     mousemap[button][event].key.scancode,
		     mousemap[button][event].key.scancode );
	      break;
	    case kverb:
	      id = mousemap[button][event].kverb.id & ~(F_KVERB);
	      if ( id != K_IGNORE ) {
		  for ( i = 0; i< nkverbs; i++)
		    if ( id == kverbs[i].kwval ) {
			name = kverbs[i].kwd;
			break;
		    }
		  printf("   %s = Kverb: \\K%s\n",
			 mousename(button,event),
			 name
			 );
	      }
	      break;
	    case macro:
	      printf("   %s = Macro: ",
		     mousename(button,event) );
	      shostrdef(mousemap[button][event].macro.string);
	      printf("\n");
	      break;
	  }
}
#endif /* OS2MOUSE */
#endif /* NOLOCAL */
#endif /* NOSHOW */

#ifndef NOSHOW
#ifdef OS2
static struct keytab shokeytab[] = {	/* SHOW KEY modes */
    "all",    1, 0,
    "one",    0, 0
};
static int nshokey = (sizeof(shokeytab) / sizeof(struct keytab));
#endif /* OS2 */

int
doshow(x) int x; {
    int y, i; long zz;
    char *s;
#ifdef OS2
    extern int os2gks;
#endif /* OS2 */

#ifndef NOSETKEY
    if (x == SHKEY) {			/* SHOW KEY */
        int c;
#ifdef OS2 
        if ((x = cmkey(shokeytab,nshokey,"How many keys should be shown?",
                        "one",xxstring)) < 0) return(x);
#endif /* OS2 */
        if ((y = cmcfm()) < 0) return(y);
#ifdef MAC
	printf("Not implemented\n");
	return(0);
#else /* Not MAC */
#ifdef OS2
        if ( x ) {
#ifndef NEWMAPKEY
            extern KEY * keymap;
            for ( c = 0; c < KMSIZE; c++ )
              if ( mapkey(c) != c || macrotab[c] )
                shokeycode(c);
#else /* NEWMAPKEY */
	    con_event evt;
	    for (c = 0; c < KMSIZE; c++) {
		evt = mapkey(c);
		if (evt.type != error) {
		    shokeycode(c);
		}
	    }
#endif /* NEWMAPKEY */
        } else {
#endif /* OS2 */
	    printf(" Press key: ");
#ifdef UNIX
#ifdef NOSETBUF
	    fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
	    conbin((char)escape);	/* Put terminal in binary mode */
#ifdef OS2
	    os2gks = 0;			/* Turn off Kverb preprocessing */
#endif /* OS2 */
	    c = congks(0);		/* Get character or scan code */
#ifdef OS2
	    os2gks = 1;			/* Turn on Kverb preprocessing */
#endif /* OS2 */
	    concb((char)escape);	/* Restore terminal to cbreak mode */
	    if (c < 0) {		/* Check for error */
		printf("?Error reading key\n");
		return(0);
	    }
#ifndef OS2
/*
  Do NOT mask when it can be a raw scan code, perhaps > 255
*/
	    c &= cmdmsk;		/* Apply command mask */
#endif /* OS2 */
	    printf("\n");
	    shokeycode(c);
#ifdef OS2 
        }
#endif /* OS2 */
	return(1);
#endif /* MAC */
    }
#ifndef NOKVERBS
    if (x == SHKVB) {			/* SHOW KVERBS */
	if ((y = cmcfm()) < 0) return(y);	    
	printf("\nThe following %d keyboard verbs are available:\n\n",nkverbs);
	kwdhelp(kverbs,nkverbs,"","\\K","",3);
	printf("\n");
	return(1);
    }
#ifdef OS2
    if (x == SHUDK) {			/* SHOW UDKs */
	extern void showudk(void);
	if ((y = cmcfm()) < 0) return(y);
	showudk();
	return(1);
    }
#endif /* OS2 */
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
	i = 0;
#ifndef NOFRILLS
	printf("Function           Status:\n");
	i++;
	printf(" GET                %s\n",nm[en_get]);
	i++;
	printf(" RETRIEVE           %s\n",nm[en_ret]);
	i++;
	printf(" SEND               %s\n",nm[en_sen]);
	i++;
	printf(" MAIL               %s\n",nm[en_mai]);
	i++;
	printf(" PRINT              %s\n",nm[en_pri]);
	i++;
#ifndef NOSPL
	printf(" REMOTE ASSIGN      %s\n",nm[en_asg]);
	i++;
#endif /* NOSPL */
	printf(" REMOTE CD/CWD      %s\n",nm[en_cwd]);
	i++;
#ifdef ZCOPY
	printf(" REMOTE COPY        %s\n",nm[en_cpy]);
	i++;
#endif /* ZCOPY */
	printf(" REMOTE DELETE      %s\n",nm[en_del]);
	printf(" REMOTE DIRECTORY   %s\n",nm[en_dir]);
	printf(" REMOTE HOST        %s\n",nm[en_hos]);
	i += 3;
#ifndef NOSPL
	printf(" REMOTE QUERY       %s\n",nm[en_que]);
	i++;
#endif /* NOSPL */
	printf(" REMOTE RENAME      %s\n",nm[en_ren]);
	printf(" REMOTE SET         %s\n",nm[en_set]);	    
	printf(" REMOTE SPACE       %s\n",nm[en_spa]);	    
	printf(" REMOTE TYPE        %s\n",nm[en_typ]);	    
	printf(" REMOTE WHO         %s\n",nm[en_who]);
	printf(" BYE                %s\n",nm[en_bye]);
	printf(" FINISH             %s\n",nm[en_fin]);
	i += 7;
#endif /* NOFRILLS */
	printf("Server timeout:     %d\n",srvtim);
	printf("Server display:     %s\n", showoff(srvdis));
        printf("Server login:       ");
	if (!x_user) {
	    printf("(none)\n");
	} else {
	    char *s;
	    printf("\"%s\", \"%s\", \"%s\"\n",
		   x_user,
		   x_passwd ? x_passwd : "",
		   x_acct ? x_acct : ""
		   );
	}	    
	printf("Server get-path:");
	if (ngetpath == 0) {
	    printf("    (none)\n");
	} else {
	    printf("\n");
	    i += 3;
	    for (x = 0; x < ngetpath; x++) {
		if (getpath[x]) printf(" %d. %s\n", x, getpath[x]);
		if (++i > (cmd_rows - 3)) { /* More than a screenful... */
		    if (!askmore())
		      break;
		    else
		      i = 0;
		}
	    }
	}
	break;
#endif /* NOSERVER */

      case SHSTA:			/* Status of last command */
	printf( " %s\n", success ? "SUCCESS" : "FAILURE" );
	return(0);			/* Don't change it */

#ifdef MAC
      case SHSTK: {			/* Stack for MAC debugging */
	  long sp;

	  sp = -1;
	  loadA0 ((char *)&sp);		/* set destination address */
	  SPtoaA0();			/* move SP to destination */
	  printf("Stack at 0x%x\n", sp);
	  show_queue();			/* more debugging */
	  break; 
      }
#endif /* MAC */

#ifndef NOLOCAL
#ifdef OS2
      case SHTAB:			/* SHOW TABS */
        shotabs();
        break;
#endif /* OS2 */

      case SHTER:			/* SHOW TERMINAL */
	shotrm();
	break;

#ifdef OS2
      case SHVSCRN:			/* SHOW Virtual Screen - for debug */
	shovscrn();
        break;
#endif /* OS2 */

#ifdef OS2MOUSE
      case SHMOU:			/* SHOW MOUSE */
        shomou();
        break;
#endif /* OS2MOUSE */
#endif /* NOLOCAL */

#ifndef NOFRILLS
      case SHVER:
	shover();
	break;
#endif /* NOFRILLS */
 
#ifndef NOSPL
      case SHBUI:			/* Built-in variables */
      case SHFUN:			/* or built-in functions */
#ifdef CK_TTGWSIZ
#ifdef OS2
	if (tt_cols[VTERM] < 0 || tt_rows[VTERM] < 0)
	  ttgwsiz();
#else /* OS2 */
	if (ttgwsiz() > 0) {		/* Get current screen size */
	    if (tt_rows > 0 && tt_cols > 0) {
		cmd_rows = tt_rows;
		cmd_cols = tt_cols;
	    }
	}
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

	if (x == SHFUN) {		/* Functions */
	    printf("\nThe following functions are available:\n\n");
	    kwdhelp(fnctab,nfuncs,"","\\F","()",3);
	    printf("\n");
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
		    while (*s)		/* Show control chars symbolically */
		      printf("\\{%d}",*s++);
		    printf("\n");
		} else if ( vartab[y].kwval == VN_IBUF ||  /* \v(input) */
			   vartab[y].kwval == VN_QUE  ||  /* \v(query) */
#ifdef OS2
			    vartab[y].kwval == VN_SELCT || /* \v(select) */
#endif /* OS2 */
			   (vartab[y].kwval >= VN_M_AAA && /* modem ones */
			    vartab[y].kwval <= VN_M_ZZZ)
			   ) {
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
	x = 0;				/* Variable count */
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
	for (y = 0; y < (int) 'z' - ARRAYBASE + 1; y++)
	  if (a_ptr[y]) {
	      if (x == 0) printf("Declared arrays:\n");
	      x = 1;
	      printf(" \\&%c[%d]\n",
		     (y == 1) ? 64 : y + ARRAYBASE, a_dim[y]);
	  }
	if (!x) printf(" No arrays declared\n");
	break;
#endif /* NOSPL */

      case SHPRO:			/* Protocol parameters */
	shoparp();
	if (protocol == PROTO_K) {
#ifdef XFRCAN
	    printf("\n Cancellation: %s",showoff(xfrcan));
	    if (xfrcan) printf(" %d %d\n", xfrchr, xfrnum);
	    else printf("\n");
#endif /* XFRCAN */
	    printf(" Send / Receive Pause:   %d (msec)\n\n",pktpaus);
	}
	break;

      case SHCOM:			/* Communication parameters */
	printf("\n");
	shoparc();
#ifdef OS2
	{
	    int i;
	    char *s = "(unknown)";
	    for (i = 0; i < nprty; i++)
	      if (prtytab[i].kwval == priority) {
		  s = prtytab[i].kwd;
		  break;
	      }
	    printf(" Priority: %s\n", s );
	}
#endif /* OS2 */

	printf("\n");
#ifdef NETCONN
	if (!network) {
#endif /* NETCONN */
	    shomdm();
	    printf("\n");
#ifdef NETCONN
	}
#endif /* NETCONN */

#ifdef COMMENT				/* No more space for this! */
#ifndef NODIAL
	printf("\n");
#ifdef NETCONN
	if (!network)
#endif /* NETCONN */
	  shodial();
#endif /* NODIAL */
#else
#ifndef NODIAL
	printf("Type SHOW DIAL to see DIAL-related items\n");
#endif /* NODIAL */
#endif /* COMMENT */
	break;

      case SHFIL:			/* File parameters */
	shoparf();
	/* printf("\n"); */		/* (out o' space) */
	break;

#ifndef NOCSETS
      case SHLNG:			/* Languages */
	shoparl();
	break;
#endif /* NOCSETS */

#ifndef NOSPL
      case SHSCR:			/* Scripts */
	printf(" Command Quoting:     %s\n", showoff(cmdgquo()));
	printf(" Take  Echo:          %s\n", showoff(techo));
	printf(" Take  Error:         %s\n", showoff(takerr[cmdlvl]));
	printf(" Macro Echo:          %s\n", showoff(mecho));
	printf(" Macro Error:         %s\n", showoff(merror[cmdlvl]));
	printf(" Input Case:          %s\n", inpcas[cmdlvl] ?
	       "Observe" : "Ignore");
	printf(" Input Buffer-length: %d\n", inbufsize);
	printf(" Input Echo:          %s\n", showoff(inecho));
	printf(" Input Silence:       %d (seconds)\n", insilence);
	printf(" Input Timeout:       %s\n", intime[cmdlvl] ?
	       "quit" : "proceed");
	if (instatus < 0)
	  printf(" Last INPUT:          -1 (INPUT command not yet given)\n");
	else
	  printf(" Last INPUT:          %d (%s)\n", instatus,i_text[instatus]);
	printf(" Output Pacing:       %d (milliseconds)\n",pacing);
#ifndef NOSCRIPT
	printf(" Script Echo:         %s\n", showoff(secho));
#endif /* NOSCRIPT */
	printf(" Command buffer:      %d\n", CMDBL);
	printf(" Atom buffer:         %d\n", ATMBL);

	break;
#endif /* NOSPL */

#ifndef NOXMIT
      case SHXMI:
	printf(" File type: %s\n", binary ? "binary" : "text");
#ifndef OS2
        printf(" See SHOW CHARACTER-SETS for character-set info\n");
#else /* OS2 */
#ifndef NOCSETS
	shotcs(tcsl,tcsr);
#endif /* NOCSETS */
#endif /* OS2 */
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
	else
	  printf(" Transmit Fill: none\n");
	printf(" Transmit Linefeed: %s\n",
	       xmitl ? "on (send linefeeds too)" : "off");
	if (xmitp) 
	  printf(" Transmit Prompt: %d (host line end character)\n",xmitp);
	else
	  printf(" Transmit Prompt: none\n");
	printf(" Transmit Echo: %s\n", showoff(xmitx));
	printf(" Transmit Locking-Shift: %s\n", showoff(xmits));
	printf(" Transmit Pause: %d milliseconds\n", xmitw);
	break;
#endif /* NOXMIT */

#ifndef NODIAL
      case SHMOD:			/* SHOW MODEM */
	shomodem();			/* Show SET MODEM items */
	break;
#endif /* NODIAL */

#ifndef MAC
      case SHDFLT:
	zsyscmd(PWDCMD);
	break;
#endif /* MAC */

#ifndef NOLOCAL
      case SHESC:
	printf(" Escape character: Ctrl-%c (ASCII %d, %s): %s\r\n",
	       ctl(escape), escape, (escape == 127 ? "DEL" : ccntab[escape]),
	   nm[tt_escape]
	   );
	break;
#endif /* NOLOCAL */

#ifndef NODIAL
      case SHDIA:			/* SHOW DIAL */
	shmdmlin();
	printf(", speed: ");
	if ((zz = ttgspd()) < 0) {
	    printf("unknown");
	} else {
	    if (zz == 8880) printf("75/1200"); else printf("%ld",zz);
	}
	if (carrier == CAR_OFF) s = "off";
	else if (carrier == CAR_ON) s = "on";
	else if (carrier == CAR_AUT) s = "auto";
	else s = "unknown";
	printf(", carrier: %s", s);
	if (carrier == CAR_ON) {
	    if (cdtimo) printf(", timeout: %d sec", cdtimo);
	    else printf(", timeout: none");
	}
	printf("\n");
	doshodial();
	if (local
#ifdef NETCONN
	    && !network
#endif /* NETCONN */
	    ) {
	    printf("Type SHOW MODEM to see modem settings.\n");
	    printf("Type SHOW COMMUNICATIONS to see modem signals.\n");
	}
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
#ifdef OS2
        printf("\n");
#endif /* OS2 */
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
	printf("\n Exit warning %s\n", xitwarn ? 
	       (xitwarn == 1 ? "on" : "always") : "off");
	printf(" Exit on-disconnect: %s\n", showoff(exitonclose));
	printf(" Current exit status: %d\n\n", xitsta);
	break;

      case SHPRT:
	printf("Printer: %s%s\n",
	       printpipe ? "| " : "",
	       printfile ? printfile :
#ifdef OS2
	       "PRN"
#else
	       "(default)"
#endif /* OS2 */
	       );
	break;

      case SHCMD:
	printf(" Command bytesize: %d bits\n",
	       (cmdmsk == 0377) ? 8 : 7);
#ifdef CK_RECALL
	printf(" Command recall-buffer-size: %d\n",cm_recall);
#else
	printf(" Command recall-buffer not available in this version\n");
#endif /* CK_RECALL */
#ifdef CK_RECALL
	printf(" Command retry: %s\n",cm_retry ? "on" : "off");
#else
	printf(" Command retry not available in this version\n");
#endif /* CK_RECALL */
	printf(" Command quoting: %s\n", showoff(cmdgquo()));
	printf(" Command more-prompting: %s\n", showoff(xaskmore));
	printf(" Command height: %d\n", cmd_rows);
	printf(" Command width:  %d\n", cmd_cols);
	printf(" Maximum command length: %d\n", CMDBL);
#ifndef NOSPL
	printf(" Maximum number of macros: %d\n", MAC_MAX);
	printf(" Macros defined: %d\n", nmac);
	printf(" Maximum macro depth: %d\n", MACLEVEL);
	printf(" Maximum TAKE depth: %d\n", MAXTAKE);
#endif /* NOSPL */
#ifdef UNIX
#ifndef NOJC
	printf(" Suspend: %s\n", showoff(suspend));
#endif /* NOJC */
#endif /* UNIX */
	break;

#ifndef NOSPL
      case SHALRM:
	if (ck_alarm)
	  printf("Alarm at %s %s\n",alrm_date,alrm_time);
	else
	  printf("(no alarm set)\n");
	break;
#endif /* NOSPL */

#ifndef NOMSEND
      case SHSFL: {
	  extern struct filelist * filehead;
	  if (!filehead) {
	      printf("send-list is empty\n");
	  } else {
	      struct filelist * flp;
	      char * s;
	      flp = filehead;
	      while (flp) {
		  s = flp->fl_alias;
		  if (!s) s = "(none)";
		  printf("%s, mode: %s, alias: %s\n",
			 flp->fl_name,
			 gfmode(flp->fl_mode),
			 s
			 );
		  flp = flp->fl_next;
	      }
	  }
      }
      break;

#endif /* NOMSEND */

      default:
	printf("\nNothing to show...\n");
	return(-2);
    }
    return(success = 1);
}
#endif /* NOSHOW */

VOID
shmdmlin() {				/* Briefly show modem & line */
#ifndef NODIAL
    int i;
#ifndef MINIDIAL
#ifdef OLDTBCODE
    extern int tbmodel;
    _PROTOTYP( char * gtbmodel, (void) );
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
#endif /* NODIAL */
    if (local)
#ifdef OS2
      printf(" Port: %s, Modem type: ",ttname);
#else
      printf(" Line: %s, Modem type: ",ttname);
#endif /* OS2 */
    else
      printf(
#ifdef OS2
" Communication device not yet selected with SET PORT\n Modem type: "
#else
" Communication device not yet selected with SET LINE\n Modem type: "
#endif /* OS2 */
	     );
#ifndef NODIAL
    printf("%s",gmdmtyp());
#ifndef MINIDIAL
#ifdef OLDTBCODE
    if (tbmodel) printf(" (%s)",gtbmodel()); /* Telebit model info */
#endif /* OLDTBCODE */
#endif /* MINIDIAL */
#else
    printf("(disabled)");
#endif /* NODIAL */
}

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

int x_ifnum = 0;			/* Flag for IF NUMERIC active */

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
#define N_EOT 'E'

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

    while (isspace(*cp))		/* Skip past leading spaces */
      cp++;

    switch (*cp) {
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
      case '\0': return(N_EOT);		/* End of line, return that */
    }
    if (isxdigit(*cp)) {		/* Digit, must be a number */
	char tbuf[80],*tp;		/* Buffer to accumulate number */
	int radix = 10;			/* Default radix */
	for (tp = tbuf; isxdigit(*cp); cp++)
	  *tp++ = (char) (isupper(*cp) ? tolower(*cp) : *cp);
	*tp = '\0';			/* End number */
	switch(isupper(*cp) ? tolower(*cp) : *cp) { /* Examine break char */
	  case 'h':
	  case 'x': radix = 16; cp++; break; /* if radix signifier... */
	  case 'o':
	  case 'q': radix = 8; cp++; break;
	  case 't': radix = 2; cp++; break;
	}
	for (tp = tbuf, tokval = 0; *tp != '\0'; tp++)  {
	    int dig;
	    dig = *tp - '0';		/* Convert number */
	    if (dig > 10) dig -= 'a'-'0'-10;
	    if (dig >= radix) {
		xerror = 1;
		if (cmdlvl == 0 && !x_ifnum)
		  printf("invalid digit '%c' in number\n",*tp);
		return(NUMBER);
	    }
	    tokval = radix*tokval + dig;
	}
	return(NUMBER);
    }
    if (cmdlvl == 0 && !x_ifnum)
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
	    if (cmdlvl == 0 && !x_ifnum)
	      printf("illegal radix\n");
	    return(0);
	}
	curtok = gettok();
    }
#endif /* COMMENT */
    if (curtok != N_EOT) {
	if (cmdlvl == 0 && !x_ifnum)
	  printf("extra characters after expression\n");
	xerror = 1;
    }
    return(expval);
}

char *
evala(s) char *s; {
    char *p;				/* Return pointer */
    long v;				/* Numeric value */

    if (!s) return("");
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
	    if (cmdlvl == 0 && !x_ifnum)
	      printf("missing right parenthesis\n");
	    xerror = 1;
	}
    } else {
	if (cmdlvl == 0 && !x_ifnum)
	  printf("operator unexpected\n");
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

    if (a > 63 && a < 91) a += 32;	/* Convert letters to lowercase */
    if (a < ARRAYBASE || a > 122)	/* Verify name */
      return(-1);
    a -= ARRAYBASE;			/* Convert name to number */
    if ((p = a_ptr[a]) != NULL) {	/* Delete old array of same name */
	n2 = a_dim[a];
	for (i = 0; i <= n2; i++) {	/* First delete its elements */
	    if (p[i]) {
		free(p[i]);
		p[i] = NULL;
	    }
	}
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
    /* On stack to allow for recursive calls... */
    char vnbuf[ARRAYREFLEN+1];		/* Entire array reference */
    char ssbuf[ARRAYREFLEN+1];		/* Subscript in "plain text" */
    char sxbuf[16];			/* Evaluated subscript */

    *c = *n = 0;			/* Initialize return values */
    for (i = 0; i < (int)strlen(ss); i++) /* Check length */
      if (ss[i] == ']')
	break;
    if (i > ARRAYREFLEN) {
	printf("?Array reference too long - %s\n",ss);
	return(-9);
    }
    strncpy(vnbuf,ss,ARRAYREFLEN);
    vnp = vnbuf;
    if (vnbuf[0] == CMDQ && vnbuf[1] == '&') vnp++;
    if (*vnp != '&') {
	printf("?Not an array - %s\n",vnbuf);
	return(-9);
    }
    x = *(vnp + 1);			/* Fold case of array name */
    /* We don't use isupper & tolower here on purpose because these */
    /* would produce undesired effects with accented letters. */
    if (x > 63 && x < 91) x  = *(vnp + 1) = (char) (x + 32);
    if ((x < ARRAYBASE) || (x > 122) || (*(vnp+2) != '[')) {
	printf("?Invalid format for array name - %s\n",vnbuf);
	return(-9);
    }
    *c = x;				/* Return the array name */
    s = vnp+3;				/* Get dimension */
    p = ssbuf;    
    pp = 1;				/* Bracket counter */
    for (i = 0; i < ARRAYREFLEN && *s != NUL; i++) { /* Copy up to ] */
	if (*s == '[') pp++;
	if (*s == ']' && --pp == 0) break;
	*p++ = *s++;
    }
    if (*s != ']') {
	printf("?No closing bracket on array dimension - %s\n",vnbuf);
	return(-9);
    }
    *p--;				/* Trim whitespace from end */
    while (*p == SP || *p == HT)
      p--;
    p++;
    *p = NUL;				/* Terminate subscript with null */
    p = ssbuf;				/* Point to beginning of subscript */
    while (*p == SP || *p == HT)	/* Trim whitespace from beginning */
      p++;
    sx = sxbuf;				/* Where to put expanded subscript */
    y = 16;
    zzstring(p,&sx,&y);			/* Convert variables, etc. */

    p = evala(sxbuf);			/* Run it thru \fneval()... */
    if (p) if (*p) strcpy(sxbuf,p);	/* We know it has to be a number. */

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
    x = a - ARRAYBASE;			/* Values must be in range 95-122 */
    if (x < 0 || x > 'z' - ARRAYBASE)	/* Not in range */
      return(-2);
    if (a_ptr[x] == NULL) return(-1);	/* Not declared */
    if (i > a_dim[x]) return(-2);	/* Declared but out of range. */
    return(a_dim[x]);			/* All ok, return dimension */
}

#ifdef COMMENT				/* This isn't used. */
char *
arrayval(a,i) int a, i; {		/* Return value of \&a[i] */
    int x; char **p;			/* (possibly NULL) */
    if (a == 64) a = 96;		/* Convert atsign to grave accent */
    x = a - ARRAYBASE;			/* Values must be in range 95-122 */
    if (x < 0 || x > 27) return(NULL);	/* Not in range */
    if ((x > 0) && (p = a_ptr[x]) == NULL) /* Array not declared */
      return(NULL);
    if (i > a_dim[x])			/* Subscript out of range. */
      return(NULL);
    return(p[i]);			/* All ok, return pointer to value. */
}
#endif /* COMMENT */
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
    char name[256];
    char *p;
    int y;

    if (*s != CMDQ) {			/* Handle macro names too */
	sprintf(name,"\\m(%s)", s);
	s = name;
    }
    p = valbuf;				/* Expand variable into valbuf. */
    y = VALN;
    if (zzstring(s,&p,&y) < 0) return(-1);
    p = valbuf;				/* Make sure value is numeric */
    if (chknum(p)) {			/* Convert numeric string to int */
	*v = atoi(p);			/* OK */
    } else {				/* Not OK */
	p = evala(p);			/* Maybe it's an expression */
	if (!chknum(p))			/* Did it evaluate? */
	  return(-1);			/* No, failure. */
	else				/* Yes, */
	  *v = atoi(p);			/* success */
    }
    return(0);    
}

/* Increment or decrement a variable */
/* Returns -1 on failure, 0 on success */

int
incvar(s,x,z) char *s; int x; int z; {	/* Increment a numeric variable */
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
    return(0);
}
#endif /* NOSPL */

/* Functions moved here from ckuusr.c to even out the module sizes... */

/*
  Breaks up string s -- IN PLACE! -- into a list of up to max words.
  Pointers to each word go into the array list[].
  max is the maximum number of words (pointers).
  If list is NULL, then they are added to the macro table.
  flag = 0 means the last field is to be one word, like all the other fields,
         so anything after it is discarded.
  flag = 1 means the last field extends to the end of the string, even if
         there are lots of words left, so the last field contains the
         remainder of the string.
*/
VOID
xwords(s,max,list,flag) char *s; int max; char *list[]; int flag; {
    char *p;
    int b, i, k, y, z;
#ifndef NOSPL
    int macro;
    macro = (list == NULL);
    debug(F101,"xwords macro","",macro);
#endif /* NOSPL */

#ifdef XWORDSDEBUG
    printf("XWORDS max=%d\n",max);
#endif /* XWORDSDEBUG */
    p = s;				/* Pointer to beginning of string */
    b = 0;				/* Flag for outer brace removal */
    k = 0;				/* Flag for in-word */
    y = 0;				/* Brace nesting level */
    z = 0;				/* "Word" counter, 0 thru max */

    if (list)
      for (i = 0; i <= max; i++)	/* Initialize pointers */
	list[i] = NULL;

    if (flag) max--;

    while (1) {				/* Go thru word list */
	if (!s || (*s == '\0')) {	/* No more characters? */
	    if (k != 0) {		/* Was I in a word? */
		if (z == max) break;	/* Yes, only go up to max. */
		z++;			/* Count this word. */
#ifdef XWORDSDEBUG
		printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
		if (macro) {		/* Doing macro args */
		    varnam[1] = (char) (z + '0'); /* Assign last argument */
		    addmac(varnam,p);
		} else {		/* Not doing macro args */
#endif /* NOSPL */
		    list[z] = p;	/* Assign pointer. */
#ifdef XWORDSDEBUG
		    printf("[1]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
		}
#endif /* NOSPL */
		break;			/* And get out. */
	    } else break;		/* Was not in a word */
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
	    if (k == 0) {		/* If we weren't in a word before, */
		p = s;			/* Mark the beginning */
		if (flag && z == max) {	/* Want last word to be remainder? */
		    z++;
#ifdef XWORDSDEBUG
		    printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */
		    list[z] = p;	/* Yes, point to it */
#ifdef XWORDSDEBUG
		    printf("[4]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
		    break;		/* and quit */
		}
		k = 1;			/* Set in-word flag */
	    }
	}
	/* If we're not inside a braced quantity, and we are in a word, and */
	/* we have hit whitespace, then we have a word. */
	if ((y < 1) && (k != 0) && (*s == SP || *s == HT)) { 
	    if (!flag || z < max)	/* if we don't want to keep rest */
	      *s = '\0';		/* terminate the arg with null */
	    k = 0;			/* say we're not in a word any more */
	    y = 0;			/* start braces off clean again */
	    if (z == max) break;	/* Only go up to max. */
	    z++;			/* count this arg */
#ifdef XWORDSDEBUG
	    printf("1 z++ = %d\n", z);
#endif /* XWORDSDEBUG */

#ifndef NOSPL
	    if (macro) {
		varnam[1] = (char) (z + '0');	/* compute its name */
		addmac(varnam,p);	/* add it to the macro table */
	    } else {
#endif /* NOSPL */
		list[z] = p;
#ifdef XWORDSDEBUG
		printf("[2]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
	    }
#endif /* NOSPL */
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
	} else {
#endif /* NOSPL */
	    list[z] = p;
#ifdef XWORDSDEBUG
	    printf("[3]LIST[%d]=\"%s\"\n",z,list[z]);
#endif /* XWORDSDEBUG */
#ifndef NOSPL
	}
#endif /* NOSPL */
    }
#ifndef NOSPL
    if (macro)
      macargc[maclvl] = z + 1;		/* Set \v(argc) variable */
#endif /* NOSPL */
    return;
}

#ifndef NOSPL
/* D O D O  --  Do a macro */

/*
  Call with x = macro table index, s = pointer to arguments.
  Returns 0 on failure, 1 on success.
*/

int
dodo(x,s,flags) int x; char *s; int flags; {
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
    count[cmdlvl] = count[cmdlvl-1];	/* Inherit COUNT from previous level */
    intime[cmdlvl] = intime[cmdlvl-1];	/* Inherit previous INPUT TIMEOUT */
    inpcas[cmdlvl] = inpcas[cmdlvl-1];	/*   and INPUT CASE */
    takerr[cmdlvl] = takerr[cmdlvl-1];	/*   and TAKE ERROR */
    merror[cmdlvl] = merror[cmdlvl-1];	/*   and MACRO ERROR */
    cmdstk[cmdlvl].src = CMD_MD;	/* Say we're in a macro */
    cmdstk[cmdlvl].lvl = maclvl;	/* and remember the macro level */
    cmdstk[cmdlvl].ccflgs = flags;	/* set flags */
    mrval[maclvl] = NULL;		/* Initialize return value */

    debug(F110,"do macro",mactab[x].kwd,0);

/* Clear old %0..%9 arguments */

    addmac("%0",mactab[x].kwd);		/* Define %0 = name of macro */
    varnam[0] = '%';
    varnam[2] = '\0';
    for (y = 1; y < 10; y++) {		/* Clear args %1..%9 */
	varnam[1] = (char) (y + '0');
	delmac(varnam);
    }	

/* Assign the new args one word per arg, allowing braces to group words */

    xwords(s,9,NULL,0);
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
    char * ppp;

    s = *src;
    lp = *dest;
    ppp = *dest;

    debug(F110,"litcmd",s,0);

    while (*s == SP) s++;		/* strip extra leading spaces */
    debug(F110,"litcmd2",s,0);

    if (*s == '{') {

        pp = 0;				/* paren counter */
	bc = 1;				/* count leading brace */
	*lp++ = *s++;			/* copy it */
	debug(F110,"litcmd3",s,0);
	while (*s == SP) s++;		/* strip interior leading spaces */
	debug(F110,"litcmd4",s,0);
	ss = flit;			/* point to "\flit(" */
	while (*lp++ = *ss++);		/* copy it */

	debug(F110,"litcmd5",s,0);

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
	    *lp = NUL;
	    debug(F110,"litcmd6",s,0);

#ifdef COMMENT
/* This was from before \flit() stopped breaking on commas... */
	    if (*s == ',' && pp == 0) {	/* comma not inside of parens */
		*lp++ = ')';		/* closing ) of \flit( */
		*lp++ = ',';		/* insert the comma */
		while (*lp++ = *ss++);	/* start new "\flit(" */
		lp--;			/* back up over null */
		s++;			/* skip over comma in source string */
		while (*s++ == SP);	/* eat leading spaces again. */
		s--;			/* back up over nonspace */
		continue;
	    }
#endif /* COMMENT */
            *lp++ = *s++;		/* Copy anything but comma here. */
        }
	*lp = NUL;
	debug(F110,"litcmd7",s,0);

    } else {				/* No brackets around, */
	while (*lp++ = *s++)		/* just copy. */
	  ;
	lp--;
	debug(F110,"litcmd8",s,0);
    }
    debug(F110,"litcmd9",s,0);

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
		p = NULL;
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
    if (!quiet && 
#ifndef NOSPL
	cmdlvl == 0
#else
	tlevel == -1
#endif /* NOSPL */
	)
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

int
prtopt(lines,s) int * lines; char *s; {	/* Print an option */
    static int x = 0;			/* (used by SHOW FEATURES) */
    int y, i;				/* Does word wrap. */
    if (!s) { x = 0; return(1); }	/* Call with null pointer to */
    y = (int)strlen(s) + 1;		/* reset horizontal position. */
    i = *lines;
    x += y;
    if (
#ifdef OS2
	x > ((cmd_cols > 40) ? (cmd_cols - 1) : 79)
#else /* OS2 */
	x > ((tt_cols > 40) ? (tt_cols - 1) : 79)
#endif /* OS2 */
	) {
	printf("\n");
	if (++i > (cmd_rows - 3)) {
	    if (!askmore())
	      return(0);
	    else
	      *lines = 0;
	}
	printf(" %s",s);
	x = y;
    } else printf(" %s",s);
    return(1);
}
#endif /* NOICP */
