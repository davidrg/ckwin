/*  C K U U S R . H  --  Symbol definitions for C-Kermit ckuus*.c modules  */
 
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
#ifndef CKUUSR_H
#define CKUUSR_H

#include "ckucmd.h"			/* Get symbols from command package */
 
/* Sizes of things */

#define FSPECL 300			/* Max length for MSEND/GET string */
#define VNAML 64			/* Max length for variable name */
#define ARRAYREFLEN 128			/* Max length for array reference */
#define FORDEPTH 10			/* Maximum depth of nested FOR loops */
#define GVARS 126			/* Highest global var allowed */
#define MAXTAKE 32			/* Maximum nesting of TAKE files */
#define MACLEVEL 64			/* Maximum nesting for macros */
#define NARGS 10			/* Max number of macro arguments */
#define LINBUFSIZ (CMDBL + 10)		/* Size of line[] buffer */
#define TMPBUFSIZ 257			/* Size of temporary buffer */
#define LBLSIZ 50			/* Maximum length for a GOTO label */
#define INPBUFSIZ 256			/* Size of INPUT buffer */
#define CMDSTKL ( MACLEVEL + MAXTAKE + 2) /* Command stack depth */
#define MAC_MAX 256			/* Maximum number of macros */
#define MSENDMAX 100			/* Number of filespecs for MSEND */
#define PROMPTL 256			/* Max length for prompt */

#ifndef NOMINPUT			/* MINPUT command */
#ifndef NOSPL
#define CK_MINPUT
#ifndef MINPMAX
#ifdef BIGBUFOK
#define MINPMAX 96			/* Max number of MINPUT strings */
#else
#define MINPMAX 16
#endif /* BIGBUFOK */
#endif /* MINPMAX */
#define MINPBUFL 256			/* Size of MINPUT temp buffer */
#endif /* NOSPL */
#endif /* NOMINPUT */

#define ARRAYBASE 95			/* Lowest array-name character */

/* Bit values (1, 2, 4, 8, ...) for the ccflgs field */

#define CF_APC  1			/* Executing an APC command */
#define CF_KMAC 2			/* Executing a \Kmacro */
#define CF_CMDL 4			/* Macro from -C "blah" command line */
#define CF_REXX 8			/* Macro from REXX interpreter */

struct cmdptr {				/* Command stack structure */
    int src;				/* Command Source */
    int lvl;				/* Current TAKE or DO level */
    int ccflgs;				/* Flags */
};

struct mtab {				/* Macro table, like keyword table */
    char *kwd;				/* But with pointers for vals */
    char *mval;				/* instead of ints. */
    short flgs;
};

struct localvar {			/* Local variable structure. */
    char * lv_name;
    char * lv_value;
    struct localvar * lv_next;
};

/*
  C-Kermit Initialization file...

  System-dependent defaults are built into the program, see below.
  These can be overridden in either of two ways:
  1. CK_DSYSINI is defined at compile time, in which case a default
     system-wide initialization file name is chosen from below, or:
  2: CK_SYSINI is defined to be a string, which lets the program
     builder choose the initialization filespec at compile time. 
  These can be given on the CC command line, so the source code need not be
  changed.
*/

#ifndef CK_SYSINI			/* If no initialization file given, */
#ifdef CK_DSYSINI			/* but CK_DSYSINI is defined... */

/* Supply system-dependent "default default" initialization file */

#ifdef UNIX				/* For UNIX, system-wide */
/* This allows one copy of the standard init file on the whole system, */
/* rather than a separate copy in each user's home directory. */
#ifdef HPUX10
#define CK_SYSINI "/usr/share/lib/kermit/ckermit.ini"
#else
#ifdef CU_ACIS
#define CK_SYSINI "/usr/share/lib/kermit/ckermit.ini"
#else
#define CK_SYSINI "/usr/local/bin/ckermit.ini"
#endif /* CU_ACIS */
#endif /* HPUX10 */
/* Fill in #else..#ifdef's here for VMS, OS/2, etc. */
/* Fill in matching #endif's here. */
#endif /* UNIX */

#endif /* CK_DSYSINI */
#endif /* CK_SYSINI */

#ifdef CK_SYSINI			/* Init-file precedence */
#ifndef CK_INI_A			/* A means system-wide file first */
#ifndef CK_INI_B			/* B means user's first */
#define CK_INI_A			/* A is default */
#endif /* CK_INI_B */
#endif /* CK_INI_A */
#else
#ifdef CK_INI_A				/* Otherwise */
#undef CK_INI_A				/* make sure these aren't defined */
#endif /* CK_INI_A */
#ifdef CK_INI_B
#undef CK_INI_B
#endif /* CK_INI_B */
#endif /* CK_SYSINI */

#ifdef CK_SYSINI			/* One more check... */
#ifdef CK_INI_A				/* Make sure they're not both */
#ifdef CK_INI_B				/* defined. */
#undef CK_INI_B
#endif /* CK_INI_B */
#endif /* CK_INI_A */
#endif /* CK_SYSINI */
/*
  If neither CK_DSYSINI nor CK_SYSINI are defined, these are the
  built-in defaults for each system:
*/
#ifdef vms
#define KERMRC "CKERMIT.INI"
#define KERMRCL 256
#else
#ifdef OS2
#ifdef NT
#define KERMRC "k95.ini"
#else
#define KERMRC "k2.ini"
#endif /* NT */
#define KERMRCL 256
#else
#ifdef UNIX
#define KERMRC ".kermrc"
#define KERMRCL 256
#else
#ifdef OSK
#define KERMRC ".kermrc"
#define KERMRCL 256
#else
#ifdef STRATUS
#define KERMRC "ckermit.ini"
#define KERMRCL 256
#else
#define KERMRC "CKERMIT.INI"
#define KERMRCL 256
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */
#endif /* OS2 */
#endif /* vms */

#ifndef KERMRCL
#define KERMRCL 256
#endif /* KERMRCL */

/* User interface features */

#ifdef CK_CURSES			/* Thermometer */
#ifndef NO_PCT_BAR
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* NO_PCT_BAR */
#endif /* CK_PCT_BAR */
#endif /* CK_CURSES */

#ifdef KUI			/* KUI requires the Thermometer code */
#ifndef NO_PCT_BAR
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* NO_PCT_BAR */
#endif /* CK_PCT_BAR */
#endif /* KUI */

/* Includes */

#ifdef MINIX
/* why? */
#include <sys/types.h>
#endif /* MINIX */

/* Symbols for command source */

#define CMD_KB 0			/* KeyBoard or standard input */
#define CMD_TF 1			/* TAKE command File */
#define CMD_MD 2			/* Macro Definition */

/*
  SET TRANSFER CANCELLATION command should be available in all versions.
  But for now...
*/
#ifdef UNIX				/* UNIX has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* UNIX */
#ifdef VMS				/* VMS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* VMS */
#ifdef datageneral			/* DG AOS/VS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* datageneral */
#ifdef STRATUS				/* Stratus VOS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* STRATUS */
#ifdef OSK				/* OS-9 */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* OSK */

/* Top Level Commands */
/* Values associated with top-level commands must be 0 or greater. */
 
#define XXBYE   0	/* BYE */
#define XXCLE   1	/* CLEAR */
#define XXCLO   2	/* CLOSE */
#define XXCON   3	/* CONNECT */
#define XXCPY   4	/* COPY */
#define XXCWD   5	/* CWD (Change Working Directory) */
#define XXDEF	6	/* DEFINE (a command macro) */
#define XXDEL   7	/* (Local) DELETE */
#define XXDIR   8	/* (Local) DIRECTORY */
#define XXDIS   9	/* DISABLE */
#define XXECH  10	/* ECHO */
#define XXEXI  11	/* EXIT */
#define XXFIN  12	/* FINISH */
#define XXGET  13	/* GET */
#define XXHLP  14	/* HELP */
#define XXINP  15	/* INPUT */
#define XXLOC  16	/* LOCAL */
#define XXLOG  17	/* LOG */
#define XXMAI  18	/* MAIL */
#define XXMOU  19	/* (Local) MOUNT */
#define XXMSG  20	/* (Local) MESSAGE */
#define XXOUT  21	/* OUTPUT */
#define XXPAU  22	/* PAUSE */
#define XXPRI  23	/* (Local) PRINT */
#define XXQUI  24	/* QUIT */
#define XXREC  25	/* RECEIVE */
#define XXREM  26	/* REMOTE */
#define XXREN  27	/* (Local) RENAME */
#define XXSEN  28	/* SEND */
#define XXSER  29   	/* SERVER */
#define XXSET  30	/* SET */
#define XXSHE  31	/* Command for SHELL */
#define XXSHO  32	/* SHOW */
#define XXSPA  33	/* (Local) SPACE */
#define XXSTA  34	/* STATISTICS */
#define XXSUB  35	/* (Local) SUBMIT */
#define XXTAK  36	/* TAKE */
#define XXTRA  37	/* TRANSMIT */
#define XXTYP  38	/* (Local) TYPE */
#define XXWHO  39	/* (Local) WHO */
#define XXDIAL 40	/* (Local) DIAL */
#define XXLOGI 41	/* (Local) SCRIPT */
#define XXCOM  42	/* Comment */
#define XXHAN  43       /* HANGUP */
#define XXXLA  44	/* TRANSLATE */
#define XXIF   45	/* IF */
#define XXLBL  46       /* label */
#define XXGOTO 47	/* GOTO */
#define XXEND  48       /* END */
#define XXSTO  49       /* STOP */
#define XXDO   50       /* DO */
#define XXPWD  51       /* PWD */
#define XXTES  52       /* TEST */
#define XXASK  53       /* ASK */
#define XXASKQ 54       /* ASKQ */
#define XXASS  55       /* ASSIGN */
#define XXREI  56       /* REINPUT */
#define XXINC  57       /* INCREMENT */
#define XXDEC  59       /* DECREMENT */
#define XXELS  60       /* ELSE */
#define XXEXE  61	/* EXECUTE */
#define XXWAI  62	/* WAIT */
#define XXVER  63       /* VERSION */
#define XXENA  64       /* ENABLE */
#define XXWRI  65       /* WRITE */
#define XXCLS  66       /* CLS (clear screen) */
#define XXRET  67	/* RETURN */
#define XXOPE  68       /* OPEN */
#define XXREA  69	/* READ */
#define XXON   70       /* ON */
#define XXDCL  71       /* DECLARE */
#define XXBEG  72       /* BEGIN (not used) */
#define XXFOR  72       /* FOR */
#define XXWHI  73       /* WHILE */
#define XXIFX  74       /* Extended IF */
#define XXCMS  75       /* SEND from command output (not yet) */
#define XXCMR  76       /* RECEIVE to a command's input (not yet) */
#define XXCMG  77       /* GET to a command's input (not yet) */
#define XXSUS  78       /* SUSPEND */
#define XXERR  79       /* ERROR */
#define XXMSE  80       /* MSEND */
#define XXBUG  81       /* BUG */
#define XXPAD  82       /* PAD (as in X.25 PAD) ANYX25 */
#define XXRED  83       /* REDIAL */
#define XXGTA  84	/* _getargs (invisible internal) */
#define XXPTA  85	/* _putargs (invisible internal) */
#define XXGOK  86       /* GETOK - Ask for YES/NO */
#define XXTEL  87	/* TELNET */
#define XXASX  88	/* _ASSIGN (evaluates var name) */
#define XXDFX  89	/* _DEFINE (evaluates var name) */
#define XXPNG  90	/* PING (for TCP/IP) */
#define XXINT  91       /* INTRODUCTION */
#define XXCHK  92	/* CHECK (a feature) */
#define XXMSL  93       /* MSLEEP, MPAUSE (millisecond sleep) */
#define XXNEW  94       /* NEWS */
#define XXAPC  95       /* APC */
#define XXFUN  96       /* REDIRECT */
#define XXWRL  97	/* WRITE-LINE */
#define XXREXX 98	/* Rexx */
#define XXMINP 100	/* MINPUT */
#define XXRSEN 101	/* RESEND */
#define XXPSEN 102	/* PSEND */
#define XXGETC 103	/* GETC */
#define XXEVAL 104	/* EVALUATE */
#define XXFWD  105	/* FORWARD */
#define XXUPD  106      /* UPDATES */
#define XXBEEP 107      /* BEEP */
#define XXMOVE 108      /* MOVE */
#define XXMMOVE 109     /* MMOVE */
#define XXREGET 110     /* REGET */
#define XXLOOK  111	/* LOOKUP */
#define XXVIEW  112     /* VIEW (terminal buffer) */
#define XXANSW  113	/* ANSWER (the phone) */
#define XXPDIA  114	/* PDIAL (partial dial) */
#define XXASC   115	/* ASCII / TEXT */
#define XXBIN   116	/* BINARY */
#define XXFTP   117	/* FTP */
#define XXMKDIR 118	/* MKDIR */
#define XXRMDIR 119	/* RMDIR */
#define XXTELOP 120	/* TELOPT */
#define XXRLOG  121	/* RLOGIN */
#define XXUNDEF 122	/* UNDEFINE */
#define XXNPSH  123	/* NOPUSH */
#define XXADD   124	/* ADD */
#define XXLOCAL 125	/* LOCAL */
#define XXKERMI 126	/* KERMIT */
#define XXDATE  127	/* DATE */
#define XXSWIT  128     /* SWITCH */
#define XXXFWD  129	/* _FORWARD */
#define XXSAVE  130     /* SAVE */
#define XXXECH  131     /* XECHO */
#define XXRDBL  132     /* READBLOCK */
#define XXWRBL  133     /* WRITEBLOCK */
#define XXRETR  134     /* RETRIEVE */
#define XXEIGHT 135     /* EIGHTBIT */

/* IF conditions */

#define  XXIFCO 0       /* IF COUNT */
#define  XXIFER 1       /* IF ERRORLEVEL */
#define  XXIFEX 2       /* IF EXIST */
#define  XXIFFA 3       /* IF FAILURE */
#define  XXIFSU 4       /* IF SUCCESS */
#define  XXIFNO 5       /* IF NOT */
#define  XXIFDE 6       /* IF DEFINED */
#define  XXIFEQ 7	/* IF EQUAL (strings) */
#define  XXIFAE 8       /* IF = (numbers) */
#define  XXIFLT 9       /* IF < (numbers) */
#define  XXIFGT 10      /* IF > (numbers) */
#define  XXIFLL 11      /* IF Lexically Less Than (strings) */
#define  XXIFLG 12      /* IF Lexically Greater Than (strings) */
#define  XXIFEO 13      /* IF EOF (READ file) */
#define  XXIFBG 14      /* IF BACKGROUND */
#define  XXIFNU 15	/* IF NUMERIC */
#define  XXIFFG 16      /* IF FOREGROUND */
#define  XXIFDI 17      /* IF DIRECTORY */
#define  XXIFNE 18      /* IF NEWER */
#define  XXIFRO 19      /* IF REMOTE-ONLY */
#define  XXIFAL 20	/* IF ALARM */
#define  XXIFSD 21      /* IF STARTED-FROM-DIALER */
#define  XXIFTR 22      /* IF TRUE */
#define  XXIFNT 23      /* IF FALSE */
#define  XXIFTM 24      /* IF TERMINAL-MACRO */
#define  XXIFEM 25      /* IF EMULATION */

/* SET parameters */
 
#define XYBREA  0	/* BREAK simulation */
#define XYCHKT  1	/* Block check type */
#define XYDEBU  2	/* Debugging */
#define XYDELA  3	/* Delay */
#define XYDUPL  4	/* Duplex */
#define XYEOL   5	/* End-Of-Line (packet terminator) */
#define XYESC   6	/* Escape character */
#define XYFILE  7	/* File Parameters */
			/* (this space available) */
#define XYFLOW  9	/* Flow Control */
#define XYHAND 10	/* Handshake */
#define XYIFD  11	/* Incomplete File Disposition */
#define XYIMAG 12	/* "Image Mode" */
#define XYINPU 13	/* INPUT command parameters */
#define XYLEN  14	/* Maximum packet length to send */
#define XYLINE 15	/* Communication line to use */
#define XYLOG  16	/* Log file */
#define XYMARK 17	/* Start of Packet mark */
#define XYNPAD 18	/* Amount of padding */
#define XYPADC 19	/* Pad character */
#define XYPARI 20	/* Parity */
#define XYPAUS 21	/* Interpacket pause */
#define XYPROM 22	/* Program prompt string */
#define XYQBIN 23	/* 8th-bit prefix */
#define XYQCTL 24	/* Control character prefix */
#define XYREPT 25	/* Repeat count prefix */
#define XYRETR 26	/* Retry limit */
#define XYSPEE 27	/* Line speed (baud rate) */
#define XYTACH 28	/* Character to be doubled */
#define XYTIMO 29	/* Timeout interval */
#define XYMODM 30	/* Modem - also see XYDIAL */

#define XYSEND 31	/* SET SEND parameters */
#define XYRECV 32   	/* SET RECEIVE parameters */
#define XYTERM 33	/* SET TERMINAL parameters */
#define   XYTBYT 0      /*  Terminal Bytesize (7 or 8) */
#define   XYTTYP 1      /*  Terminal emulation Type */
#define     TT_NONE  0	/*    NONE, no emulation */
/*
  Note, the symbols for VT and VT-like terminals should be in ascending 
  numerical order, so that higher ones can be treated as supersets of
  lower ones with respect to capabilities.
*/
#define     TT_DG200    1 	/*    Data General 200/210 */
#define     TT_HP2621   2 	/*    Hewlett-Packard 2621A */
#define     TT_HZL1500  3 	/*    Hazeltine 1500 */
#define     TT_VC4404   4 	/*    Volker Craig VC4404/404 */
#define     TT_WY30     5	/*    WYSE-30/30+ */
#define     TT_WY50     6 	/*    WYSE-50/50+ */
#define     TT_WY60     7       /*    WYSE-60	 */
#define     TT_VT52     8	/*    DEC VT-52  */
#define     TT_H19      9	/*    Heath-19 */
#define     TT_ANSI    10	/*    IBM ANSI.SYS (BBS) */
#define     TT_SCOANSI 11	/*    SCOANSI (Unix mode) */
#define     TT_AT386   12 	/*    Unixware AT386 (Unix mode) */
#define     TT_VT100   13	/*    DEC VT-100 */
#define     TT_VT102   14	/*    DEC VT-102 */
#define     TT_VT220   15	/*    DEC VT-220 */
#define     TT_VT320   16	/*    DEC VT-320 */
#define     TT_WY370   17	/*    WYSE 370 ANSI Terminal */
#define     TT_TVI910  18	/*    TVI 910+ */
#define     TT_TVI925  19       /*    TVI 925  */
#define     TT_TVI950  20       /*    TVI950   */
#define     TT_MAX   TT_TVI950
#define     TT_VT420   96	/*    DEC VT-420 */
#define     TT_VT520   97	/*    DEC VT-520/525 */	
#define     TT_IBM     98       /*    IBM 31xx */
#define     TT_TEK40 99	/*    Tektronix 401x */

#define ISANSI(x)  (x >= TT_ANSI && x <= TT_AT386)
#define ISSCO(x)   (x == TT_SCOANSI)
#define ISAT386(x) (x == TT_AT386)
#define ISAVATAR(x) (x == TT_ANSI)
#define ISUNIXCON(x) (x == TT_SCOANSI || x == TT_AT386)
#define ISDG200(x) (x == TT_DG200)
#define ISHZL(x)   (x == TT_HZL1500)
#define ISH19(x)   (x == TT_H19) 
#define ISIBM(x)   (x == TT_IBM)
#define ISTVI(x)   (x >= TT_TVI910 && x <= TT_TVI950)
#define ISTVI910(x) (x == TT_TVI910)
#define ISTVI925(x) (x == TT_TVI925)
#define ISTVI950(x) (x == TT_TVI950)
#define ISVT52(x)  (x == TT_VT52 || x == TT_H19)
#define ISVT100(x) (x >= TT_VT100 && x <= TT_WY370)
#define ISVT102(x) (x >= TT_VT102 && x <= TT_WY370)
#define ISVT220(x) (x >= TT_VT220 && x <= TT_WY370)
#define ISVT320(x) (x >= TT_VT320 && x <= TT_WY370)
#define ISVT420(x) (x >= TT_VT420 && x <= TT_VT520)
#define ISVT520(x) (x == TT_VT520)
#define ISWY30(x)  (x == TT_WY30)
#define ISWY50(x)  (x >= TT_WY30 && x <= TT_WY60)
#define ISWY60(x)  (x == TT_WY60)
#define ISWY370(x) (x == TT_WY370)
#define ISVC(x)    (x == TT_VC4404)
#define ISHP(x)    (x == TT_HP2621)

#define   XYTCS  2      /*  Terminal Character Set */
#define   XYTSO  3	/*  Terminal Shift-In/Shift-Out */
#define   XYTNL  4      /*  Terminal newline mode */
#define   XYTCOL 5      /*  Terminal colors */
#define   XYTEC  6	/*  Terminal echo = duplex = local-echo */
#define   XYTCUR 7	/*  Terminal cursor */
#define     TTC_ULINE 0 
#define     TTC_HALF  1
#define     TTC_BLOCK 2
#define   XYTARR 8	/*  Terminal arrow-key mode */
#define   XYTKPD 9      /*  Terminal keypad mode */
#define    TTK_NORM 0   /*    Normal mode for arrow / keyad keys */
#define    TTK_APPL 1   /*    Application mode for arrow / keyad keys */
#define   XYTWRP 10     /*  Terminal wrap */
#define   XYTCRD 11	/*  Terminal CR-display */
#define   XYTANS 12	/*  Terminal answerback */
#define   XYSCRS 13     /*  Terminal scrollback buffer size */
#define   XYTAPC 14	/*  Terminal APC */
#define   XYTBEL 15     /*  Terminal Bell */
#define   XYTDEB 16	/*  Terminal Debug */
#define   XYTROL 17     /*  Terminal Rollback */
#define     TTR_OVER   0  /*  Rollback Overwrite */
#define     TTR_INSERT 1  /*  Rollback Insert */
#define   XYTCTS 18     /*  Terminal Transmit-Timeout */
#define   XYTCPG 19     /*  Terminal Code Page */
#ifdef COMMENT
#define   XYTHCU 20     /*  Terminal Hide-Cursor */
#endif /* COMMENT */
#define   XYTPAC 21	    /*  Terminal Output-Pacing */
#define   XYTMOU 22	    /*  Terminal Mouse */
#define   XYTHIG 23     /*  Terminal Width */
#define   XYTWID 24     /*  Terminal Height */
#define   XYTUPD 25     /*  Terminal Screen-update */
#define    TTU_FAST 0   /*     FAST but jerky */
#define    TTU_SMOOTH 1 /*     SMOOTH but slow */
#define   XYTFON 26     /*  Terminal Full screen Font */
#define    TTF_ROM    0 /*     ROM font */
#define    TTF_CY1    1 /*     CYRILL1 font */
#define    TTF_CY2    2 /*     CYRILL2 font */
#define    TTF_CY3    3 /*     CYRILL3 font */
#define    TTF_111  111 /*     CP111 font */
#define    TTF_112  112 /*     CP112 font */
#define    TTF_113  113 /*     CP113 font */
#define    TTF_437  437 /*     CP437 font */
#define    TTF_850  850 /*     CP850 font */
#define    TTF_851  851 /*     CP851 font */
#define    TTF_852  852 /*     CP852 font */
#define    TTF_853  853 /*     CP853 font */
#define    TTF_860  860 /*     CP860 font */
#define    TTF_861  861 /*     CP861 font */
#define    TTF_862  862 /*     CP862 font */
#define    TTF_863  863 /*     CP863 font */
#define    TTF_864  864 /*     CP864 font */
#define    TTF_865  865 /*     CP865 font */
#define    TTF_866  866 /*     CP866 font */
#define    TTF_880  880 /*     CP880 font */
#define    TTF_881  881 /*     CP881 font */
#define    TTF_882  882 /*     CP882 font */
#define    TTF_883  883 /*     CP883 font */
#define    TTF_884  884 /*     CP884 font */
#define    TTF_885  885 /*     CP885 font */
#define   XYTVCH 27     /* SET TERMINAL VIDEO-CHANGE */
#define   XYTAUTODL 28  /* SET TERMINAL AUTODOWNLOAD */
#define   XYTAUTOUL 29  /* SET TERMINAL AUTOUPLOAD   */
#define   XYTATTBUG 30  /* SET TERM ATTR-BUG */
#define   XYTSTAT   31  /* SET TERM STATUSLINE */
#define   XYTESC    32  /* SET TERM ESCAPE-CHARACTER */
#define   XYTCTRL   33  /* SET TERM CONTROLS */
#define   XYTATTR   34  /* SET TERM ATTRIBUTE representation */
#define   XYTSGRC   35  /* SET TERM SGRCOLORS */
#define   XYTLCS    36  /* SET TERM LOCAL-CHARACTER-SET */
#define   XYTRCS    37  /* SET TERM REMOTE-CHARACTER-SET */
#define   XYTUNI    38  /* SET TERM UNICODE */
#define   XYTKEY    39  /* SET TERM KEY */
#define   XYTSEND   40  /* SET TERM SEND-DATA */
#define   XYTSEOB   41  /* SET TERM SEND-END-OF-BLOCK */

#define XYATTR 34       /* Attribute packets */
#define XYSERV 35	/* Server parameters */
#define   XYSERT 0      /*  Server timeout   */
#define   XYSERD 1	/*  Server display   */
#define   XYSERI 2      /*  Server idle      */
#define   XYSERP 3	/*  Server get-path  */
#define   XYSERL 4	/*  Server login     */
#define XYWIND 36       /* Window size */
#define XYXFER 37       /* Transfer */
#define   XYX_CAN 0	/*   Cancellation  */
#define   XYX_CSE 1	/*   Character-Set */
#define   XYX_LSH 2	/*   Locking-Shift */
#define   XYX_PRO 3	/*   Protocol      */
#define   XYX_MOD 4	/*   Mode          */
#define   XYX_DIS 5	/*   Display       */
#define   XYX_SLO 6	/*   Slow-start    */
#define   XYX_CRC 7	/*   CRC calculation */
#define   XYX_BEL 8	/*   Bell */
#define XYLANG 38       /* Language */
#define XYCOUN 39       /* Count */
#define XYTAKE 40       /* Take */ 
#define XYUNCS 41       /* Unknown-character-set */
#define XYKEY  42       /* Key */
#define XYMACR 43       /* Macro */
#define XYHOST 44       /* Hostname on network */
#define XYNET  45       /* SET NETWORK things */

#define XYNET_D 99	/* NETWORK DIRECTORY */
#define XYNET_T 100	/* NETWORK TYPE */

#define XYCARR 46	/* Carrier */
#define XYXMIT 47       /* Transmit */

#define XYDIAL 48       /* Dial options */

/* And now we interrupt the flow to bring you lots of stuff about dialing */

#ifndef MAXDNUMS
#ifdef BIGBUFOK
#define MAXDDIR 32	/* Maximum number of dialing directories */
#define MAXDNUMS 4095	/* Max numbers to fetch from dialing directories */
#else
#define MAXDDIR 12
#define MAXDNUMS 1024
#endif /* BIGBUFOK */
#endif /* MAXDNUMS */
/*
  IMPORTANT: In 5A(192), the old SET DIAL command was split into two commands:
  SET MODEM (for modem-related parameters) and SET DIAL (for dialing items).
  To preserve the old formats, etc, invisibly we keep one symbol space for
  both commands.
*/
#define  XYDHUP  0	/*   Dial Hangup */
#define  XYDINI  1      /*   MODEM (dial) Initialization string */
#define  XYDKSP  2      /*   MODEM (dial) Kermit-Spoof */
#define  XYDTMO  3      /*   Dial Timeout */
#define  XYDDPY  4      /*   Dial Display */
#define  XYDSPD  5      /*   Dial Speed matching */
#define  XYDMNP  6	/*   MODEM (dial) MNP negotiation enabled (obsolete) */
#define  XYDEC   7	/*   MODEM (dial) error correction enabled */
#define  XYDDC   8      /*   MODEM (dial) compression enabled */
#define  XYDHCM  9      /*   MODEM (dial) hangup-string (moved elsewhere) */
#define  XYDDIR 10	/*   Dial directory */
#define  XYDDIA 11	/*   MODEM (dial) dial-command */
#define  XYDMHU 12	/*   MODEM HANGUP (dial modem-hangup) */
#define  XYDNPR 13      /*   Dial PREFIX */
#define  XYDSTR 14	/*   MODEM COMMAND (dial string) ... */

#define   XYDS_DC 0	/*    Data compression */
#define   XYDS_EC 1	/*    Error correction */
#define   XYDS_HU 2     /*    Hangup command */
#define   XYDS_HW 3     /*    Hardware flow control */
#define   XYDS_IN 4     /*    Init-string */
#define   XYDS_NF 5     /*    No flow control */
#define   XYDS_PX 6     /*    Prefix (no, this goes in SET DIAL) */
#define   XYDS_SW 7     /*    Software flow control */
#define   XYDS_DT 8     /*    Tone dialing command */
#define   XYDS_DP 9     /*    Pulse dialing command */
#define   XYDS_AN 10    /*    Autoanswer */
#define   XYDS_RS 11    /*    Reset */
#define   XYDS_MS 12    /*    Dial mode string */
#define   XYDS_MP 13    /*    Dial mode prompt */

#define   XYDM_D  0     /*    Method: Default */
#define   XYDM_T  1     /*      Tone */
#define   XYDM_P  2     /*      Pulse */

#define  XYDFC   15	/*   MODEM (dial) flow-control */
#define  XYDMTH  16	/*   Dial method */
#define  XYDESC  17     /*   MODEM (dial) escape-character */
#define  XYDMAX  18	/*   MODEM (dial) maximum interface speed */
#define  XYDCAP  19     /*   MODEM (dial) capabilities */
#define  XYDTYP  20	/*   MODEM TYPE */
#define  XYDINT  21	/*   DIAL retries */
#define  XYDRTM  22	/*   DIAL time between retries */
#define  XYDNAM  23	/*   MODEM NAME */
#define  XYDLAC  24	/*   DIAL LOCAL-AREA-CODE */
#define  XYDMCD  25	/*   MODEM CARRIER */

#define  XYDCNF  26	/*   DIAL CONFIRMATION */
#define  XYDCVT  27	/*   DIAL CONVERT-DIRECTORY */
#define  XYDIXP  28	/*   DIAL INTERNATIONAL-PREFIX */
#define  XYDIXS  29	/*   DIAL INTERNATIONAL-SUFFIX */
#define  XYDLCC  30	/*   DIAL LOCAL-COUNTRY-CODE */
#define  XYDLDP  31	/*   DIAL LONG-DISTANCE-PREFIX */
#define  XYDLDS  32	/*   DIAL LONG-DISTANCE-SUFFIX */
#define  XYDPXX  33	/*   DIAL PBX-EXCHANGE */
#define  XYDPXI  34	/*   DIAL PBX-INTERNAL-PREFIX */
#define  XYDPXO  35	/*   DIAL PBX-OUTSIDE-PREFIX */
#define  XYDSFX  36	/*   DIAL SUFFIX */
#define  XYDSRT  37	/*   DIAL SORT */
#define  XYDTFC  38	/*   DIAL TOLL-FREE-AREA-CODE */
#define  XYDTFP  39	/*   DIAL TOLL-FREE-PREFIX */
#define  XYDTFS  40	/*   DIAL TOLL-FREE-SUFFIX */
#define  XYDCON  41     /*   DIAL CONNECT */
#define  XYDRSTR 42     /*   DIAL RESTRICT */
#define  XYDRSET 42     /*   MODEM RESET */

#define XYSESS 49       /* SET SESSION options */
#define XYBUF  50       /* Buffer length */
#define XYBACK 51	/* Background */
#define XYDFLT 52       /* Default */
#define XYDOUB 53	/* Double */
#define XYCMD  54       /* COMMAND */

/* SET COMMAND items... */

#define SCMD_BSZ 0	/* BYTESIZE */
#define SCMD_RCL 1	/* RECALL */
#define SCMD_RTR 2	/* RETRY */
#define SCMD_QUO 3	/* QUOTING */
#define SCMD_COL 4	/* COLOR */
#define SCMD_HIG 5	/* HEIGHT */
#define SCMD_WID 6	/* WIDTH */
#define SCMD_CUR 7	/* CURSOR-POSITION */
#define SCMD_SCR 8	/* SCROLLBACK */
#define SCMD_MOR 9	/* MORE-PROMPTING */

#define XYCASE 55       /* Case */
#define XYCOMP 56       /* Compression */
#define XYX25  57       /* X.25 parameter (ANYX25) */
#define XYPAD  58       /* X.3 PAD parameter (ANYX25) */
#define XYWILD 59       /* Wildcard expansion method */
#define XYSUSP 60       /* Suspend */
#define XYMAIL 61	/* Mail-Command */
#define XYPRIN 62	/* Print-Command */
#define XYQUIE 63	/* Quiet */
#define XYLCLE 64	/* Local-echo */
#define XYSCRI 65	/* SCRIPT command paramaters */
#define XYMSGS 66       /* MESSAGEs ON/OFF */
#define XYTEL  67       /* TELNET parameters */
#define  CK_TN_EC 0	/*  TELNET ECHO */
#define  CK_TN_TT 1	/*  TELNET TERMINAL-TYPE */
#define  CK_TN_NL 2     /*  TELNET NEWLINE-MODE */
#define  CK_TN_BM 3     /*  TELNET BINARY-MODE */
#define  CK_TN_BUG 4    /*  TELNET BUG */
#define  CK_TN_ENV 5    /*  TELNET ENVIRONMENT */
#define    TN_ENV_USR  0 /*    VAR USER */
#define    TN_ENV_JOB  1 /*    VAR JOB */
#define    TN_ENV_ACCT 2 /*    VAR ACCT */
#define    TN_ENV_PRNT 3 /*    VAR PRINTER */
#define    TN_ENV_SYS  4 /*    VAR SYSTEMTYPE */
#define    TN_ENV_DISP 5 /*    VAR DISPLAY */
#define    TN_ENV_UVAR 6 /*    USERVAR */
#define XYOUTP 68	/* OUTPUT command parameters */
#define  OUT_PAC 0	/*  OUTPUT pacing */
#define XYEXIT  69	/* SET EXIT */
#define XYPRTR  70	/* SET PRINTER */
#define XYFPATH 71	/* PATHNAME */
#define XYMOUSE 72	/* MOUSE SUPPORT */

#define  XYM_ON     0   /* Mouse ON/OFF        */
#define  XYM_BUTTON 1   /* Define Mouse Events */
#define  XYM_CLEAR  2   /* Clear Mouse Events  */
/* These must match similar definitions in ckokey.h */
#define   XYM_B1 0      /* Mouse Button One */
#define   XYM_B2 1      /* Mouse Button Two */
#define   XYM_B3 2      /* Mouse Button Three */
#define   XYM_ALT   1     /* Alt */
#define   XYM_CTRL  2     /* Ctrl */
#define   XYM_SHIFT 4     /* Shift */
#define   XYM_C1    0     /* Single Click */
#define   XYM_C2    8     /* Double Click */
#define   XYM_DRAG  16    /* Drag Event */

#define XYBELL 73   /* BELL */

#define   XYB_NONE  0     /* No bell */
#define   XYB_AUD   1     /* Audible bell */
#define   XYB_VIS   2     /* Visible bell */
#define   XYB_BEEP  0     /* Audible Beep */
#define   XYB_SYS   4     /* Audible System Sounds */

#define XYPRTY     74   /* Thread Priority Level */

#define   XYP_IDLE  1 
#define   XYP_REG   2
#define   XYP_SRV   4
#define   XYP_RTP   3

#define XYALRM     75	/* SET ALARM */
#define XYPROTO    76	/* SET PROTOCOL */
#define XYPREFIX   77   /* SET PREFIXING */
#define XYLOGIN    78   /* Login info for script programs... */

#define  LOGI_UID   0	/* User ID  */
#define  LOGI_PSW   1	/* Password */
#define  LOGI_PRM   2	/* Prompt   */

#define XYSTARTUP  79    /* Startup file */
#define XYTMPDIR   80    /* Temporary directory */
#define XYTAPI     81    /* Microsoft Telephone API options */
#define   XYTAPI_CFG     1  /* TAPI Configure-Line */
#define   XYTAPI_DIAL    2  /* TAPI Dialing-Properties */
#define   XYTAPI_LIN     3  /* TAPI Line */
#define   XYTAPI_LOC     4  /* TAPI Location */

#define XYTCP  82       /* TCP options */
#define  XYTCP_NODELAY   1  /* No Delay */
#define  XYTCP_SENDBUF   2  /* Send Buffer Size */
#define  XYTCP_LINGER    3  /* Linger */
#define  XYTCP_RECVBUF   4  /* Receive Buffer Size */
#define  XYTCP_KEEPALIVE 5  /* Keep Alive packets */

#define XYMSK  83       /* MS-DOS Kermit compatibility options */
#define  MSK_COLOR 0    /*  Terminal color handling   */
#define  MSK_KEYS  1    /*  SET KEY uses MSK keycodes */

#define XYDEST 84	/* SET DESTINATION as in MS-DOS Kermit */
#define XYWIN95 85	/* SET WIN95 work arounds  */
#define   XYWKEY  0	/*    Keyboard translation */
#define   XYWAGR  1     /*    Alt-Gr               */
#define   XYWOIO  2     /*    Overlapped I/O       */
#define XYDLR  86 	/* SET K95 DIALER work arounds */
#define XYTITLE 87	/* SET TITLE of window */

/* #ifdef ANYX25 */
/* PAD command parameters */

#define XYPADL 0        /* clear virtual call */
#define XYPADS 1        /* status of virtual call */
#define XYPADR 2        /* reset of virtual call */
#define XYPADI 3        /* send an interrupt packet */

/* Used with XYX25... */
#define XYUDAT 0       /* X.25 call user data */
#define XYCLOS 1       /* X.25 closed user group call */
#define XYREVC 2       /* X.25 reverse charge call */
/* #endif */ /* ANYX25 */

/* SHOW command symbols */

#define SHPAR 0				/* Parameters */
#define SHVER 1				/* Versions */
#define SHCOM 2				/* Communications */
#define SHPRO 3				/* Protocol */
#define SHFIL 4				/* File */
#define SHLNG 5				/* Language */
#define SHCOU 6				/* Count */
#define SHMAC 7				/* Macros */
#define SHKEY 8				/* Key */
#define SHSCR 9				/* Scripts */
#define SHSPD 10			/* Speed */
#define SHSTA 11			/* Status */
#define SHSER 12			/* Server */
#define SHXMI 13			/* Transmit */
#define SHATT 14			/* Attributes */
#define SHMOD 15			/* Modem */
#define SHDFLT 16			/* Default (as in VMS) */
#define SHVAR 17			/* Show global variables */
#define SHARG 18			/* Show macro arguments */
#define SHARR 19			/* Show arrays */
#define SHBUI 20			/* Show builtin variables */
#define SHFUN 21			/* Show functions */
#define SHPAD 22			/* Show (X.25) PAD */
#define SHTER 23			/* Show terminal settings */
#define SHESC 24			/* Show escape character */
#define SHDIA 25			/* Show DIAL parameters */
#define SHNET 26			/* Show network parameters */
#define SHLBL 27			/* Show VMS labeled file parameters */
#define SHSTK 28			/* Show stack, MAC debugging */
#define SHCSE 29			/* Show character sets */
#define SHFEA 30			/* Show features */
#define SHCTL 31			/* Show control-prefix table */
#define SHEXI 32			/* Show EXIT items */
#define SHPRT 33			/* Show printer */
#define SHCMD 34			/* Show command parameters */
#define SHKVB 35			/* Show \Kverbs */
#define SHMOU 36			/* Show Mouse (like Show Key) */
#define SHTAB 37			/* Show Tabs */
#define SHVSCRN 38			/* Show Virtual Screen (OS/2) */
#define SHALRM  39			/* ALARM */
#define SHSFL   40			/* SEND-LIST */
#define SHUDK   41                      /* DEC VT UDKs (OS/2) */

/* REMOTE command symbols */
 
#define XZCPY  0	/* Copy */
#define XZCWD  1	/* Change Working Directory */
#define XZDEL  2	/* Delete */
#define XZDIR  3	/* Directory */
#define XZHLP  4	/* Help */
#define XZHOS  5	/* Host */
#define XZKER  6	/* Kermit */
#define XZLGI  7	/* Login */
#define XZLGO  8	/* Logout */
#define XZMAI  9	/* Mail <-- wrong, this should be top-level */
#define XZMOU 10	/* Mount */
#define XZMSG 11	/* Message */
#define XZPRI 12	/* Print */
#define XZREN 13	/* Rename */
#define XZSET 14	/* Set */
#define XZSPA 15	/* Space */
#define XZSUB 16	/* Submit */
#define XZTYP 17	/* Type */
#define XZWHO 18	/* Who */
#define XZPWD 19	/* Print Working Directory */
#define XZQUE 20	/* Query */
#define XZASG 21	/* Assign */
 
/* SET INPUT command parameters */

#define IN_DEF  0			/* Default timeout */
#define IN_TIM  1			/* Timeout action */
#define IN_CAS  2			/* Case (matching) */
#define IN_ECH  3			/* Echo */
#define IN_SIL  4			/* Silence */
#define IN_BUF  5			/* Buffer size */
#define IN_PAC  6                       /* Input Pacing (debug) */

/* ENABLE/DISABLE command parameters */

#define EN_ALL  0			/* ALL */
#define EN_CWD  1			/* CD/CWD */
#define EN_DIR  2			/* DIRECTORY */
#define EN_FIN  3			/* FINISH */
#define EN_GET  4			/* GET */
#define EN_HOS  5			/* HOST command */
#define EN_KER  6			/* KERMIT command */
#define EN_LOG  7			/* LOGIN */
#define EN_SEN  8			/* SEND */
#define EN_SET  9			/* SET */
#define EN_SPA 10			/* SPACE */
#define EN_TYP 11			/* TYPE */
#define EN_WHO 12			/* WHO, finger */
#define EN_DEL 13			/* Delete */
#define EN_BYE 14			/* BYE (as opposed to FINISH) */
#define EN_QUE 15			/* QUERY */
#define EN_ASG 16			/* ASSIGN */
#define EN_CPY 17			/* COPY */
#define EN_REN 18			/* RENAME */
#define EN_RET 19			/* RETRIEVE */
#define EN_MAI 20			/* MAIL */
#define EN_PRI 21			/* PRINT */

/* BEEP TYPES */
#define BP_BEL  0           /* Terminal BEL         */
#define BP_NOTE 1           /* Notify the user      */
#define BP_WARN 2           /* Warn the user        */
#define BP_FAIL 3           /* Failure has occurred */

/* CLEAR command symbols */
#define CLR_DEV  1                /* Clear Device Buffers */
#define CLR_INP  2                /* Clear Input Buffers */
#define CLR_BTH  CLR_DEV|CLR_INP  /* Clear Device and Input */
#define CLR_SCL  4                /* Clear Scrollback buffer */
#define CLR_CMD  8                /* Clear Command Screen */
#define CLR_TRM  16               /* Clear Terminal Screen */
#define CLR_DIA  32		  /* Clear Dial Status */
#define CLR_SFL  64		  /* Clear Send-File-List */
#define CLR_APC 128		  /* Clear APC */

/* Symbols for logs */
 
#define LOGD 0	    	/* Debugging */
#define LOGP 1          /* Packets */
#define LOGS 2          /* Session */
#define LOGT 3          /* Transaction */
#define LOGX 4          /* Screen */
#define LOGR 5		/* The "OPEN read file */
#define LOGW 6          /* The "OPEN" write/append file */
#define LOGE 7		/* Error (e.g. stderr) */

/* Symbols for builtin variables */

#define VN_ARGC 0			/* ARGC */
#define VN_COUN 1			/* COUNT */
#define VN_DATE 2			/* DATE */
#define VN_DIRE 3			/* DIRECTORY */
#define VN_ERRO 4			/* ERRORLEVEL */
#define VN_TIME 5			/* TIME */
#define VN_VERS 6			/* VERSION */
#define VN_IBUF 7			/* INPUT buffer */
#define VN_SUCC 8			/* SUCCESS flag */
#define VN_LINE 9			/* LINE */
#define VN_ARGS 10			/* Program command-line arg count */
#define VN_SYST 11			/* System type */
#define VN_SYSV 12			/* System version */
#define VN_RET  13			/* RETURN value */
#define VN_FILE 14			/* Most recent filespec */
#define VN_NDAT 15			/* Numeric date yyyy/mm/dd */
#define VN_HOME 16			/* Home directory */
#define VN_SPEE 17			/* Transmission speed */
#define VN_HOST 18			/* Host name */
#define VN_TTYF 19			/* TTY file descriptor (UNIX only) */
#define VN_PROG 20			/* Program name */
#define VN_NTIM 21			/* NTIME */
#define VN_FFC  22			/* Characters in last file xferred */
#define VN_TFC  23			/* Chars in last file group xferred */
#define VN_CPU  24			/* CPU type */
#define VN_CMDL 25			/* Command level */
#define VN_DAY  26                      /* Day of week, string */
#define VN_NDAY 27                      /* Day of week, numeric */
#define VN_LCL  28			/* Local (vs) remote mode */
#define VN_CMDS 29			/* Command source */
#define VN_CMDF 30			/* Command file name */
#define VN_MAC  31			/* Macro name */
#define VN_EXIT 32			/* Exit status */
#define VN_ICHR 33			/* INPUT character */
#define VN_ICNT 34			/* INPUT count */
#define VN_PRTY 35			/* Current parity */
#define VN_DIAL 36			/* DIAL status */
#define VN_KEYB 37			/* Keyboard type */
#define VN_CPS  38			/* Chars per second, last transfer */
#define VN_RPL  39			/* Receive packet length */
#define VN_SPL  40			/* Send packet length */
#define VN_MODE 41			/* Transfer mode (text, binary) */
#define VN_REXX 42			/* Rexx return value */
#define VN_NEWL 43			/* Newline character or sequence */
#define VN_COLS 44			/* Columns on console screen */
#define VN_ROWS 45			/* Rows on console screen */
#define VN_TTYP 46			/* Terminal type */
#define VN_MINP 47			/* MINPUT result */
#define VN_CONN 48			/* Connection type */
#define VN_SYSI 49			/* System ID */
#define VN_TZ   50			/* Timezone */
#define VN_SPA  51			/* Space */
#define VN_QUE  52			/* Query */
#define VN_STAR 53			/* Startup directory */
#define VN_CSET 54			/* Local character set */
#define VN_MDM  55			/* Modem type */
#define VN_EVAL 56			/* Most recent EVALUATE result */

#define VN_D_CC 57			/* DIAL COUNTRY-CODE */
#define VN_D_AC 58			/* DIAL AREA-CODE */
#define VN_D_IP 59			/* DIAL INTERNATIONAL-PREFIX */
#define VN_D_LP 60			/* DIAL LD-PREFIX */

#define VN_UID  61
#define VN_PWD  62
#define VN_PRM  63

#define VN_PROTO 64			/* Protocol */
#define VN_DLDIR 65			/* Download directory */

#define VN_M_AAA 66			/* First MODEM one */
#define VN_M_INI 66			/* Modem init string */
#define VN_M_DCM 67			/* Modem dial command */
#define VN_M_DCO 68			/* Modem data compression on */
#define VN_M_DCX 69			/* Modem data compression off */
#define VN_M_ECO 70			/* Modem error correction on */
#define VN_M_ECX 71			/* Modem error correction off */
#define VN_M_AAO 72			/* Modem autoanswer on */
#define VN_M_AAX 73			/* Modem autoanswer off */
#define VN_M_HUP 74			/* Modem hangup command */
#define VN_M_HWF 75			/* Modem hardware flow command */
#define VN_M_SWF 76			/* Modem software flow command */
#define VN_M_NFC 77			/* Modem no flow-control command */
#define VN_M_PDM 78			/* Modem pulse dialing mode */
#define VN_M_TDM 79			/* Modem tone dialing mode */
#define VN_M_ZZZ 79			/* Last MODEM one */

#define VN_SELCT 80			/* Selected Text from Mark Mode */
#define VN_TEMP  81			/* Temporary directory */
#define VN_ISTAT 82			/* INPUT command status */
#define VN_INI   83			/* INI (kermrc) directory */
#define VN_EXEDIR 84			/* EXE directory */
#define VN_ERRNO  85			/* Value of errno */
#define VN_ERSTR  86			/* Corresponding error message */
#define VN_TFLN   87			/* TAKE file line number */
#define VN_XVNUM  88			/* Product-specific version number */
#define VN_RPSIZ  89			/* Receive packet length */
#define VN_WINDO  90			/* Window size */
#define VN_MDMSG  91			/* Modem message */
#define VN_DNUM   92			/* Dial number */
#define VN_APC    93			/* APC active */
#define VN_IPADDR 94			/* My IP address */
#define VN_CRC16  95			/* CRC-16 of most recent file group */
#define VN_TRMK   96                    /* Macro executed from Terminal Mode */

/* INPUT status values */

#define INP_OK  0			/* Succeeded */
#define INP_TO  1			/* Timed out */
#define INP_UI  2			/* User interrupted */
#define INP_IE  3			/* Internal error */
#define INP_IO  4			/* I/O error or connection lost */

/* Symbols for builtin functions */

#define FNARGS 6			/* Maximum number of function args */

#define FN_IND 0			/* Index (of string 1 in string 2) */
#define FN_LEN 1			/* Length (of string) */
#define FN_LIT 2			/* Literal (don't expand the string) */
#define FN_LOW 3			/* Lower (convert to lowercase) */
#define FN_MAX 4			/* Max (maximum) */
#define FN_MIN 5			/* Min (minimum) */
#define FN_MOD 6			/* Mod (modulus) */
#define FN_EVA 7			/* Eval (evaluate arith expression) */
#define FN_SUB 8			/* Substr (substring) */
#define FN_UPP 9			/* Upper (convert to uppercase) */
#define FN_REV 10			/* Reverse (a string) */
#define FN_REP 11			/* Repeat (a string) */
#define FN_EXE 12			/* Execute (a macro) */
#define FN_VAL 13			/* Return value (of a macro) */
#define FN_LPA 14			/* LPAD (left pad) */
#define FN_RPA 15			/* RPAD (right pad) */
#define FN_DEF 16			/* Definition of a macro, unexpanded */
#define FN_CON 17			/* Contents of a variable, ditto */
#define FN_FIL 18                       /* File list */
#define FN_FC  19			/* File count */
#define FN_CHR 20			/* Character (like BASIC CHR$()) */
#define FN_RIG 21			/* Right (like BASIC RIGHT$()) */
#define FN_COD 22			/* Code value of character */
#define FN_RPL 23			/* Replace */
#define FN_FD  24			/* File date */
#define FN_FS  25			/* File size */
#define FN_RIX 26			/* Rindex (index from right) */
#define FN_VER 27			/* Verify */
#define FN_IPA 28			/* Find and return IP address */
#define FN_CRY 39			/* ... */
#define FN_OOX 40			/* ... */
#define FN_HEX 41			/* Hexify */
#define FN_UNH 42			/* Unhexify */
#define FN_BRK 43			/* Break */
#define FN_SPN 44			/* Span */
#define FN_TRM 45			/* Trim */
#define FN_LTR 46			/* Left-Trim */
#define FN_CAP 47			/* Capitalize */
#define FN_TOD 48			/* Time-of-day-to-secs-since-midnite */
#define FN_SEC 49			/* Secs-since-midnite-to-time-of-day */
#define FN_FFN 50			/* Full file name */
#define FN_CHK 51			/* Checksum of text */
#define FN_CRC 52			/* CRC-16 of text */
#define FN_BSN 53			/* Basename of file */

/* Screen line numbers */

#define CW_BAN  0			/* Curses Window Banner */
#define CW_DIR  2			/* Current directory */
#define CW_LIN  3			/* Communication device */
#define CW_SPD  4			/* Communication speed */
#define CW_PAR  5			/* Parity */
#define CW_TMO  6
#define CW_NAM  7			/* Filename */
#define CW_TYP  8			/* File type */
#define CW_SIZ  9			/* File size */
#define CW_PCD 10			/* Percent done */

#ifndef CK_PCT_BAR
#define CW_TR  11			/* Time remaining */
#define CW_CP  12			/* Characters per second */
#define CW_WS  13			/* Window slots */
#define CW_PT  14			/* Packet type */
#define CW_PC  15			/* Packet count */
#define CW_PL  16			/* Packet length */
#define CW_PR  17			/* Packet retry */
#ifdef COMMENT
#define CW_PB  17			/* Packet block check */
#endif /* COMMENT */
#else /* CK_PCT_BAR */
#define CW_BAR 11       /* Percent Bar Scale */
#define CW_TR  12			/* Time remaining */
#define CW_CP  13			/* Chars per sec */
#define CW_WS  14			/* Window slots */
#define CW_PT  15			/* Packet type */
#define CW_PC  16			/* Packet count */
#define CW_PL  17			/* Packet length */
#define CW_PR  18			/* Packet retry */
#ifdef COMMENT
#define CW_PB  18			/* Packet block check */
#endif /* COMMENT */
#endif /* CK_PCT_BAR */

#define CW_ERR 19			/* Error message */
#define CW_MSG 20			/* Info message */
#define CW_INT 22			/* Instructions */

/* Save Commands */
#define XSKEY   0			/* Key map file */

/* ANSI-style prototypes for user interface functions */

_PROTOTYP( char * brstrip, (char *) );
_PROTOTYP( int parser, ( int ) );
_PROTOTYP( int chkvar, (char *) );
_PROTOTYP( int zzstring, (char *, char **, int *) );
#ifndef NOFRILLS
_PROTOTYP( int yystring, (char *, char **) );
#endif /* NOFRILLS */
_PROTOTYP( int xxstrcmp, (char *, char *, int) );
_PROTOTYP( int getncm, (char *, int) );
_PROTOTYP( int getnct, (char *, int, FILE *, int) );
_PROTOTYP( VOID bgchk, (void) );
_PROTOTYP( char * nvlook, (char *) );
_PROTOTYP( char * arrayval, (int, int) );
_PROTOTYP( int arraynam, (char *, int *, int *) );
_PROTOTYP( char * bldlen, (char *, char *) );
_PROTOTYP( int chkarray, (int, int) );
_PROTOTYP( int dclarray, (char, int) );
_PROTOTYP( int parsevar, (char *, int *, int *) );
_PROTOTYP( int macini, (void) );
_PROTOTYP( VOID initmac, (void) );
_PROTOTYP( int delmac, (char *) );
_PROTOTYP( int addmac, (char *, char *) );
_PROTOTYP( int domac, (char *, char *, int) );
_PROTOTYP( int addmmac, (char *, char *[]) );
_PROTOTYP( int dobug, (void) );
_PROTOTYP( int docd, (void) );
_PROTOTYP( int doclslog, (int) );
_PROTOTYP( int docmd, (int) );
_PROTOTYP( int doconect, (int) );
_PROTOTYP( int dodo, (int, char *, int) );
_PROTOTYP( int doenable, (int, int) );
_PROTOTYP( int doget, (int) );
_PROTOTYP( int dogoto, (char *, int) );
_PROTOTYP( int dogta, (int) );
_PROTOTYP( int dohlp, (int) );
_PROTOTYP( int dohrmt, (int) );
_PROTOTYP( int doif, (int) );
_PROTOTYP( int doinput, (int, char *[]) );
_PROTOTYP( int doreinp, (int, char *) );
_PROTOTYP( int dolog, (int) );
_PROTOTYP( int dologin, (char *) );
_PROTOTYP( int doopen, (void) );
_PROTOTYP( int doprm, (int, int) );
_PROTOTYP( int doreturn, (char *) );
_PROTOTYP( int dormt, (int) );
_PROTOTYP( int doshow, (int) );
_PROTOTYP( int doshodial, (void) );
_PROTOTYP( int dostat, (void) );
_PROTOTYP( int dostop, (void) );
_PROTOTYP( int dotype, (char *) );
_PROTOTYP( int transmit, (char *, char) );
_PROTOTYP( int xlate, (char *, char *, int, int) );
_PROTOTYP( int litcmd, (char **, char **) );
_PROTOTYP( int incvar, (char *, int, int) );
_PROTOTYP( int ckdial, (char *, int, int, int) );
_PROTOTYP( char * getdws, (int) );
_PROTOTYP( char * getdcs, (int) );
_PROTOTYP( int hmsg, (char *) );
_PROTOTYP( int hmsga, (char * []) );
_PROTOTYP( int mlook, (struct mtab [], char *, int) );
_PROTOTYP( int mxlook, (struct mtab [], char *, int) );
_PROTOTYP( int prtopt, (int *, char *) );
_PROTOTYP( CHAR rfilop, (char *, char) );
_PROTOTYP( int setcc, (char *, int *) );
_PROTOTYP( int setnum, (int *, int, int, int) );
_PROTOTYP( int seton, (int *) );
_PROTOTYP( VOID shmdmlin, (void) );
_PROTOTYP( VOID initmdm, (int) );
_PROTOTYP( char * showoff, (int) );
_PROTOTYP( int shoatt, (void) );
_PROTOTYP( VOID shocharset, (void) );
_PROTOTYP( int shomac, (char *, char *) );
_PROTOTYP( VOID shopar, (void) );
_PROTOTYP( VOID shoparc, (void) );
_PROTOTYP( VOID shoparc, (void) );
_PROTOTYP( VOID shoparf, (void) );
_PROTOTYP( VOID shoparp, (void) );
#ifndef NOCSETS
_PROTOTYP( VOID shoparl, (void) );
#endif /* NOCSETS */
_PROTOTYP( VOID shodial, (void) );
_PROTOTYP( VOID shomdm, (void) );
_PROTOTYP( VOID shonet, (void) );
_PROTOTYP( VOID shover, (void) );
_PROTOTYP( int pktopn, (char *,int) );
_PROTOTYP( int traopn, (char *,int) );
_PROTOTYP( int sesopn, (char *,int) );
_PROTOTYP( int debopn, (char *,int) );
_PROTOTYP( char * parnam, (char) );
_PROTOTYP( int popclvl, (void) );
_PROTOTYP( int varval, (char *, int *) );
_PROTOTYP( char * evala, (char *) );
_PROTOTYP( int setat, (int) );
_PROTOTYP( int setinp, (void) );
_PROTOTYP( int setlin, (int, int) );
_PROTOTYP( int setmodem, (void) );
_PROTOTYP( int setfil, (int) );
#ifdef OS2    
_PROTOTYP( int settapi, (void) ) ;
#ifdef OS2MOUSE
_PROTOTYP( int setmou, (void) );
#endif /* OS2MOUSE */
_PROTOTYP( int setbell, (void) );
#endif /* OS2 */
_PROTOTYP( int settrm, (void) );
_PROTOTYP( int setsr, (int, int) );
_PROTOTYP( int setxmit, (void) );
_PROTOTYP( int set_key, (void) );
_PROTOTYP( int dochk, (void) );
_PROTOTYP( int ludial, (char *, int) );
_PROTOTYP( char * getdnum, (int) );
_PROTOTYP( VOID getnetenv, (void) );
_PROTOTYP( VOID setflow, (void) );
_PROTOTYP( int getyesno, (char *) );
_PROTOTYP( VOID xwords, (char *, int, char *[], int) );
_PROTOTYP( VOID shotcs, (int, int) );
_PROTOTYP( char *hhmmss, (long) );
_PROTOTYP( VOID shoctl, (void) );
_PROTOTYP( VOID keynaminit, (void) );
_PROTOTYP( int xlookup, (struct keytab[], char *, int, int *) );	
_PROTOTYP( VOID shokeycode, (int) );
_PROTOTYP( int hupok, (int) );
_PROTOTYP( VOID shods, (char *) );
_PROTOTYP( char * zzndate, (void) );
_PROTOTYP( char * chk_ac, (int, char[]) );
_PROTOTYP( char * gmdmtyp, (void) );
_PROTOTYP( char * gfmode, (int) );
_PROTOTYP( int setdest, (void) );
_PROTOTYP( VOID ndinit, (void) );
_PROTOTYP( int doswitch, (void) );
_PROTOTYP( int dolocal, (void) );
_PROTOTYP( long tod2sec, (char *) );
_PROTOTYP( int shomodem, (void) );
_PROTOTYP( int lunet, (char *) );
_PROTOTYP( int doxdis, (void) );
_PROTOTYP( int dosave, (int) );

#endif /* CKUUSR_H */

/* End of ckuusr.h */
