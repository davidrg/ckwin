#include "ckcsym.h"

/*  C K U U S X --  "User Interface" common functions. */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  This module contains user interface functions needed by both the interactive
  user interface and the command-line-only user interface.
*/

/* Includes */

#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"


#ifndef NOHTERMCAP
#ifdef NOTERMCAP
#define NOHTERMCAP
#else
#ifndef BSD44
#define NOHTERMCAP
#else
#ifdef __bsdi__
#define NOHTERMCAP
#else
#ifdef OPENBSD
#define NOHTERMCAP
#else
#ifdef MACOSX
#define NOHTERMCAP
#endif /* MACOSX */
#endif /* OPENBSD */
#endif /* __bsdi__ */
#endif /* BSD44 */
#endif /* NOTERMCAP */
#endif /* NOHTERMCAP */

#ifndef NOTERMCAP
#ifdef BSD44
#ifndef NOHTERMCAP
#include <termcap.h>
#endif /* NOHTERMCAP */
#endif /* BSD44 */
#else  /* !BSD44 */
#ifdef linux
#include <term.h>
#endif /* linux */
#endif /* NOTERMCAP */

#ifdef OS2
#include <string.h>
_PROTOTYP(char * os2_gethostname, (void));
#define getpid _getpid
#endif /* OS2 */
#ifdef BSD44
#include <errno.h>
#endif /* BSD44 */

extern xx_strp xxstring;

#ifdef OS2
#include "ckcnet.h"
#else /* OS2 */
_PROTOTYP(int getlocalipaddr, (void));

#ifndef NETCONN
/*
  We should just pull in ckcnet.h here, but it causes a conflict with curses.h.
*/
#ifdef TCPSOCKET
#define NETCONN
#else
#ifdef SUNX25
#define NETCONN
#else
#ifdef STRATUSX25
#define NETCONN
#else
#ifdef IBMX25
#define NETCONN
#else
#ifdef HPX25
#define NETCONN
#else
#ifdef DECNET
#define NETCONN
#else
#ifdef NPIPE
#define NETCONN
#else
#ifdef CK_NETBIOS
#define NETCONN
#ifdef SUPERLAT
#define NETCONN
#else
#endif /* SUPERLAT */
#endif /* TCPSOCKET */
#endif /* SUNX25 */
#endif /* STRATUSX25 */
#endif /* IBMX25 */
#endif /* HPX25 */
#endif /* DECNET */
#endif /* NPIPE */
#endif /* CK_NETBIOS */
#endif /* NETCONN */
#endif /* OS2 */

#ifndef TCPSOCKET
#ifdef MULTINET
#define TCPSOCKET
#endif /* MULTINET */
#ifdef DEC_TCPIP
#define TCPSOCKET
#endif /* DEC_TCPIP */
#ifdef WINTCP
#define TCPSOCKET
#endif /* WINTCP */
#ifdef TCPWARE
#define TCPSOCKET
#endif /* TCPWARE */
#endif /* TCPSOCKET */

#ifdef OS2
#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#else /* NT */
#define INCL_VIO
#include <os2.h>
#endif /* NT */
#ifdef COMMENT                          /* Would you believe */
#undef COMMENT                          /* <os2.h> defines this ? */
#endif /* COMMENT */
#ifdef CK_NETBIOS
#include "ckonbi.h"
#endif /* CK_NETBIOS */

#include "ckocon.h"
extern ascreen commandscreen;
#ifdef KUI
#include "ikui.h"
#endif /* KUI */
#endif /* OS2 */

#ifdef NT
#include "cknwin.h"
#endif /* NT */
#ifdef OS2
#include "ckowin.h"
#endif /* OS2 */

#ifdef CK_TAPI
extern int tttapi;
extern int tapipass;
#endif /* CK_TAPI */

#ifdef CK_KERBEROS
#include "ckuath.h"
#endif /* CK_KERBEROS */

#ifndef WINTCP
#include <signal.h>
#endif /* WINTCP */

#ifdef VMS
#include <descrip.h>
#include <ssdef.h>
#include <stsdef.h>
#ifndef OLD_VMS
#include <lib$routines.h>  /* Not for VAX C 2.3 */
#else
#include <libdef.h>
#endif /* OLD_VMS */
#ifdef WINTCP
#include <signal.h>
#endif /* WINTCP */
#endif /* VMS */

int tt_crd = 0;                         /* Carriage return display */
int interrupted = 0;                    /* Interrupted from keyboard flag */
static int fxd_inited = 0;              /* Fullscreen stuff initialized */

#ifdef DEBUG
char debfil[CKMAXPATH+1];               /* Debugging log file name */
#endif /* DEBUG */

#ifdef TLOG
char trafil[CKMAXPATH+1];               /* Transaction log file name */
#endif /* TLOG */

char sesfil[CKMAXPATH+1];               /* Session log file name */

#ifdef CKLOGDIAL
char diafil[CKMAXPATH+1];               /* Connection log file name */
char cxlogbuf[CXLOGBUFL+1];             /* Connection log record buffer */
int cx_active = 0;                      /* Connection is active */
extern int dialog;
#endif /* CKLOGDIAL */

#ifdef DYNAMIC
static char *cmdstr = NULL;             /* Place to build generic command */
#else
#ifdef pdp11
static char cmdstr[256];
#else
static char cmdstr[4096];
#endif /* pdp11 */
#endif /* DYNAMIC */

#ifndef NOMSEND
char fspec[CMDBL+4];                    /* Filename string for \v(filespec) */
int fspeclen = CMDBL;
#else
char fspec[CKMAXPATH+4];
int fspeclen = CKMAXPATH;
#endif /* NOMSEND */

char * rfspec = NULL;
char * sfspec = NULL;
char * srfspec = NULL;
char * rrfspec = NULL;

#ifdef CK_TTGWSIZE
int xaskmore = 0;                       /* Whether to use more-prompting */
#else
int xaskmore = 1;
#endif /* CK_TTGWSIZE */
extern int saveask;

int success = 1,                        /* Command success/failure flag */
    cmdlvl = 0,                         /* Command level */
#ifndef NOSPL
#endif /* NOSPL */
    action,                             /* Action selected on command line */
#ifdef OS2
    sessft = XYFT_B,                    /* Session log file type */
#else /* OS2 */
    sessft = XYFT_T,                    /* Session log file type */
#endif /* OS2 */
    pflag = 1,                          /* Print prompt */
    msgflg = 1;                         /* Print informational messages */

#ifdef CK_APC
extern int apcactive;
#endif /* CK_APC */
/* External variables */

extern int local, quiet, binary, network, what, parity, xitsta, escape,
  tlevel, bgset, backgrd, suspend, cmdint, nettype, seslog, dfloc;

extern int cmd_rows, cmd_cols;

extern char cmdfil[];

#ifdef VMS
extern int batch;
#endif /* VMS */

#ifdef datageneral                      /* 2/12/92 ENH */
#include <sysid.h>
extern int con_reads_mt, conint_ch, conint_avl;
#endif /* datageneral */

extern long speed;

extern char ttname[], *dftty, *cmarg, **cmlist, *versio, myhost[];

#ifndef NOCSETS
extern int fcharset, tcharset;
extern struct csinfo fcsinfo[], tcsinfo[];
#endif /* NOCSETS */

#ifdef NOXFER

int fdispla = XYFD_N;

#else  /* NOXFER is not defined */

#ifdef OS2                              /* File transfer display type */
int fdispla = XYFD_C;                   /* Curses (fullscreen) if we have it */
#else
#ifdef CK_CURSES
int fdispla = XYFD_C;
#else
int fdispla = XYFD_S;                   /* Otherwise CRT */
#endif /* CK_CURSES */
#endif /* OS2 */

extern struct ck_p ptab[];
extern int protocol, xfrbel, xfrint;

#ifdef STREAMING
extern int streaming, streamok;
#endif /* STREAMING */

/* Used internally */

_PROTOTYP( VOID screenc, (int, char, long, char *) );

#ifdef CK_CURSES
#ifndef DYNAMIC
static char xtrmbuf[TRMBUFL];           /* tgetent() buffer */
char * trmbuf = xtrmbuf;
#else
char * trmbuf = NULL;
#endif /* DYNAMIC */
_PROTOTYP( static VOID dpyinit, (void) );
_PROTOTYP( static long shocps, (int, long, long) );
_PROTOTYP( static long shoetl, (long, long, long, long) );
#endif /* CK_CURSES */

static int ft_win = 0;  /* Fullscreen file transfer display window is active */

/* Variables declared here */

char pktfil[CKMAXPATH+1];               /* Packet log file name */

#ifndef NOMSEND                         /* Multiple SEND */
char *msfiles[MSENDMAX];
#endif /* NOMSEND */

#ifdef CK_TIMERS
extern long rttdelay;
extern int  rttflg;
#endif /* CK_TIMERS */
extern int rcvtimo;

#ifdef CK_RESEND
extern int sendmode;
extern long sendstart, rs_len;
#endif /* CK_RESEND */

#ifdef CK_PCT_BAR                       /* File transfer thermometer */
int thermometer = 1;                    /* ON by default */
#endif /* CK_PCT_BAR */

#ifdef GFTIMER
CKFLOAT gtv = -1.0, oldgtv = -1.0;
#else
#ifndef OS2
static
#endif /* OS2 */
  long gtv = -1L, oldgtv = -1L;
#endif /* GFTIMER */

#ifdef OS2
extern unsigned char colorcmd;
#endif /* OS2 */

extern int server, bctu, rptflg, ebqflg, spsiz, urpsiz, wmax, czseen, cxseen,
  winlo, displa, timint, npad, ebq, bctr, rptq, atcapu, lpcapu,
  swcapu, wslotn, wslotr, rtimo, mypadn, sq, capas, rpsiz, tsecs,
  pktlog, lscapu, dest, srvdis, wslots, spackets, spktl, rpktl,
  retrans, wcur, numerrs, fsecs, whatru, crunched, timeouts,
  rpackets, fncnv, bye_active, discard, inserver;

extern long filcnt, filrej, ffc, tfc, rptn, fsize, filcps, tfcps, cps, peakcps;

long oldcps = 0L;

extern CHAR *rdatap, padch, seol, ctlq, mypadc, eol, *epktmsg;

extern char whoareu[];

#ifdef IKSDB
FILE * dbfp = NULL;                     /* File pointer to database file */

int dbenabled = 1;                      /* Flag for database is enabled */
extern int ikdbopen;                    /* Flag for database is open */

unsigned long mydbseek = 0L;            /* Seek pointer to my record */
int mydbslot = 0;                       /* My slot number */
unsigned long myflags = 0L;             /* My flags */
unsigned long myatype = 0L;             /* My authorization type */
unsigned long myamode = 0L;             /* My authorization mode */
unsigned long mystate = 0L;             /* My state (SEND, RECEIVE, etc) */
unsigned long mypid = 0L;               /* My PID */
unsigned long myip = 0L;                /* My IP address */
unsigned long peerip = 0L;              /* My peer's IP address */

unsigned long dbip = 0L;                /* IP address in db record */
unsigned long dbpid = 0L;               /* PID in db record */
unsigned long dbflags = 0L;             /* Flags field in db record */
unsigned long dblastused = 0L;          /* Last in-use record in db */
char dbrec[DB_RECL];                    /* Database record buffer */

char * dbdir   = NULL;                  /* Database directory */
char * dbfile  = NULL;                  /* Database file full pathname */
char myhexip[33] = { NUL, NUL };        /* My IP address in hex */
char peerhexip[33] = { NUL, NUL };      /* Client's IP address in hex */
#endif /* IKSDB */

#ifdef GFTIMER
extern CKFLOAT fpfsecs, fptsecs, fpxfsecs;
#else
extern long xfsecs;
#endif /* GFTIMER */
#endif /* NOXFER */

#ifdef TCPSOCKET
extern char myipaddr[];
#endif /* TCPSOCKET */
#ifndef NOICP
#ifndef NOSPL
    extern struct mtab *mactab;         /* For ON_EXIT macro. */
    extern int nmac;
#endif /* NOSPL */
#ifdef DCMDBUF
extern char *cmdbuf;                    /* Command buffer */
#else
extern char cmdbuf[];                   /* Command buffer */
#endif /* DCMDBUF */
extern int cmd_quoting;
#endif /* NOICP */

#ifndef NOCCTRAP
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckcsig.h"
extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */

#ifndef NOXFER
/*  P R E S E T  --  Reset global protocol variables  */

extern int recursive;

#ifndef NOICP
#ifdef CK_LABELED
extern int g_lf_opts, lf_opts;
#endif /* CK_LABELED */
extern int g_matchdot;
extern int g_binary, g_proto, g_displa, g_spath, g_rpath, g_fncnv;
extern int g_recursive;
extern int g_xfermode, xfermode;
extern int g_urpsiz, g_spsizf, g_spsiz;
extern int g_spsizr, g_spmax, g_wslotr, g_prefixing, g_fnact, g_fncact;
extern int g_fnspath, g_fnrpath, g_skipbup;
#ifndef NOSPL
extern int g_pflg, pwflg, g_pcpt, pwcrypt;
extern char * g_pswd, pwbuf[];
#endif /* NOSPL */
#ifdef PATTERNS
extern int g_patterns, patterns;
#endif /* PATTERNS */
#endif /* NOICP */

extern int spsizf, spsizr, spmax, prefixing, fncact, fnspath, fnrpath;
extern int moving;                      /* SEND criteria */
extern char sndafter[], sndbefore[], *sndexcept[], *rcvexcept[];
extern long sndlarger, sndsmaller, calibrate, skipbup;
extern int rmailf, rprintf;
extern char optbuf[];

#ifdef PIPESEND
extern char * g_sfilter, * g_rfilter;
extern char * sndfilter, * rcvfilter;
#endif /* PIPESEND */

VOID
ftreset() {
#ifndef NOICP
    int i;
    extern char * filefile;
    extern int reliable, xreliable, c_save, ss_save, slostart, urclear;
    extern int oopts, omode, oname, opath, kactive, autopath;
    extern char * snd_move;             /* Directory to move sent files to */
    extern char * snd_rename;           /* What to rename sent files to */
    extern char * rcv_move;
    extern char * rcv_rename;
    extern char * g_snd_move;
    extern char * g_snd_rename;
    extern char * g_rcv_move;
    extern char * g_rcv_rename;

#ifdef CK_SPEED
    extern int f_ctlp;
#ifdef COMMENT
    extern short s_ctlp[], ctlp[];
#endif /* COMMENT */
#endif /* CK_SPEED */

#ifndef NOCSETS
    extern int fcs_save, tcs_save;
#endif /* NOCSETS */

/* Restore / reset per-command file-transfer switches */

    makestr(&snd_move,g_snd_move);
    makestr(&rcv_move,g_rcv_move);
    makestr(&snd_rename,g_snd_rename);
    makestr(&rcv_rename,g_rcv_rename);

    kactive = 0;                        /* Kermit protocol no longer active */
    oopts = -1;                         /* O-Packet Options */
    omode = -1;                         /* O-Packet Transfer Mode */
    oname = -1;                         /* O-Packet Filename Options */
    opath = -1;                         /* O-Packet Pathname Options */

#ifdef COMMENT
#ifdef CK_SPEED
    if (f_ctlp) {
        for (i = 0; i < 256; i++)
          ctlp[i] = s_ctlp[i];
        f_ctlp = 0;
    }
#endif /* CK_SPEED */
#endif /* COMMENT */

    calibrate = 0L;                     /* Calibration run */
    if (xreliable > -1)
      reliable = xreliable;
    urclear = 0;

    if (autopath) {                     /* SET RECEIVE PATHNAMES AUTO */
        fnrpath = PATH_AUTO;
        autopath = 0;
    }
    if (filefile) {                     /* File list */
        zclose(ZMFILE);
        makestr(&filefile,NULL);
    }
    if (c_save > -1) {                  /* Block Check Type */
        bctr = c_save;
        c_save = -1;
    }
    if (ss_save > -1) {                 /* Slow Start */
        slostart = ss_save;
        ss_save = -1;
    }
#ifdef CK_LABELED
    if (g_lf_opts > -1) {
        lf_opts = g_lf_opts;            /* Restore labeled transfer options */
        g_lf_opts = -1;
    }
#endif /* CK_LABELED */

#ifndef NOCSETS
    if (tcs_save > -1) {                /* Character sets */
        tcharset = tcs_save;
        tcs_save = -1;
    }
    if (fcs_save > -1) {
        fcharset = fcs_save;
        fcs_save = -1;
    }
    setxlatype(tcharset,fcharset);      /* Translation type */
#endif /* NOCSETS */

#ifdef NETCONN
#ifndef NOSPL
    if (g_pswd) {
        ckstrncpy(pwbuf,g_pswd,PWBUFL);
        makestr(&g_pswd,NULL);
    }
    if (g_pflg > -1) {
        pwflg = g_pflg;
        g_pflg = -1;
    }
    if (g_pcpt > -1) {
        pwcrypt = g_pcpt;
        g_pcpt = -1;
    }
#endif /* NOSPL */
#endif /* NETCONN */

    if (g_binary > -1) {                /* File type */
        binary = g_binary;
        g_binary = -1;
    }
    if (g_xfermode > -1) {              /* Transfer mode */
        xfermode = g_xfermode;
        g_xfermode = -1;
    }
#ifdef PATTERNS
    if (g_patterns > -1) {              /* Filename patterns */
        patterns = g_patterns;
        g_patterns = -1;
    }
#endif /* PATTERNS */

    if (g_matchdot > -1) {
        matchdot = g_matchdot;
        g_matchdot = -1;
    }
    if (saveask > -1) {
        xaskmore = saveask;
        saveask = -1;
    }
    if (g_proto > -1) {                 /* Protocol */
        protocol = g_proto;
        g_proto = -1;
    }
    if (g_urpsiz > -1) {
        urpsiz = g_urpsiz;
        debug(F101,"ftreset restoring urpsiz","",urpsiz);
        g_urpsiz = -1;
    }
    if (g_spsizf > -1) {
        spsizf = g_spsizf;
        debug(F101,"ftreset restoring spsizf","",spsizf);
        g_spsizf = -1;
    }
    if (g_spsiz > -1) {
        spsiz = g_spsiz;
        debug(F101,"ftreset restoring spsiz","",spsiz);
        g_spsiz = -1;
    }
    if (g_spsizr > -1) {
        spsizr = g_spsizr;
        debug(F101,"ftreset restoring spsizr","",spsizr);
        g_spsizr = -1;
    }
    if (g_spmax > -1) {
        spmax = g_spmax;
        g_spmax = -1;
    }
    if (g_wslotr > -1) {
        wslotr = g_wslotr;
        g_wslotr = -1;
    }
    if (g_prefixing > -1) {
        prefixing = g_prefixing;
        g_prefixing = -1;
    }
    if (g_fncact > -1) {
        fncact = g_fncact;
        g_fncact = -1;
    }
    if (g_fncnv > -1) {
        fncnv = g_fncnv;
        g_fncnv = -1;
    }
    if (g_fnspath > -1) {
        fnspath = g_fnspath;
        g_fnspath = -1;
    }
    if (g_fnrpath > -1) {
        fnrpath = g_fnrpath;
        g_fnrpath = -1;
    }
    if (g_skipbup > -1) {
        skipbup = g_skipbup;
        g_skipbup = -1;
    }
    recursive = 0;                      /* RECURSIVE can never be global */

    if (g_displa > -1) {                /* File transfer display */
        fdispla = g_displa;
        g_displa = -1;
    }
    if (g_spath > -1) {                 /* Send pathnames */
        fnspath = g_spath;
        g_spath = -1;
    }
    if (g_rpath > -1) {                 /* Receive pathnames */
        fnrpath = g_rpath;
        g_rpath = -1;
    }
    if (g_fncnv > -1) {                 /* Filename conversion */
        fncnv = g_fncnv;
        g_fncnv = -1;
    }
#ifdef PIPESEND
    if (g_sfilter) {                    /* Send filter */
        if (sndfilter) free(sndfilter);
        sndfilter = g_sfilter;
        g_sfilter = NULL;
    }
    if (g_rfilter) {                    /* Receive filter */
        if (rcvfilter) free(rcvfilter);
        rcvfilter = g_rfilter;
        g_rfilter = NULL;
    }
#endif /* PIPESEND */

#ifndef NOFRILLS
    rmailf = rprintf = 0;               /* MAIL and PRINT modifiers for SEND */
    optbuf[0] = NUL;                    /* MAIL and PRINT options */
#endif /* NOFRILLS */

    moving = 0;                         /* Reset delete-after-send indicator */
    sndafter[0]  = NUL;                 /* Reset SEND selection switches */
    sndbefore[0] = NUL;

    for (i = 0; i < 8; i++) {
        if (sndexcept[i])
          free(sndexcept[i]);
        sndexcept[i] = NULL;
        if (rcvexcept[i])
          free(rcvexcept[i]);
        rcvexcept[i] = NULL;
    }
    sndlarger =  -1L;
    sndsmaller = -1L;
#ifdef GFTIMER
    gtv = -1.0;
    oldgtv = -1.0;
#else
    gtv = -1L;
    oldgtv = -1L;
#endif /* GFTIMER */
#endif /* NOICP */
}
#endif /* NOXFER */

/*  C K _ E R R S T R  --  Return message from most recent system error */

char *
ck_errstr() {
#ifdef USE_STRERROR
#ifndef CK_ANSILIBS
    /* Should have been declared in <string.h> */
_PROTOTYP( char * strerror, (int) );
#endif /* CK_ANSILIBS */
    return(strerror(errno));
#else
#ifdef VMS
    extern char * ckvmserrstr(unsigned long);
    return(ckvmserrstr(0L));
#else
#ifdef ATTSV
#ifdef BSD44
#ifdef __386BSD__
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#else
#ifndef __bsdi__
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern const char *const sys_errlist[];
#endif /* NDSYSERRLIST */
#endif /* __bsdi__ */
#endif /* __386BSD__ */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#else /* !BSD44 */
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#endif /* BSD44 */

#else /* !ATTSV */

#ifdef BSD4
#ifndef NDSYSERRLIST
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
    if (errno >= sys_nerr)
      return("Error number out of range");
    else
      return((char *) sys_errlist[errno]);
#else
#ifdef OS2
#ifndef NDSYSERRLIST
    extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#ifdef NT
    extern int_sys_nerr;
#endif /* NT */
    char *e;
    e = (errno > -1
#ifdef NT
         && errno <= _sys_nerr
#endif /* NT */
         ) ?
#ifdef NT
         (char *) sys_errlist[errno]
#else /* NT */
         /* I don't know how to get a CLIB error string in OS/2 */
         strerror(errno)
#endif /* NT */
             : "";
    return(e ? e : "");
#else /* OS2 */
    return("");
#endif /* OS2 */
#endif /* BSD4 */
#endif /* ATTSV */
#endif /* VMS */
#endif /* USE_STRERROR */
}

#ifdef TCPSOCKET
#ifdef NT
extern int WSASafeToCancel;
#endif /* NT */
#endif /* TCPSOCKET */

VOID
setflow() {
    extern int flow, autoflow, mdmtyp, cxtype, cxflow[];
#ifndef NODIAL
    extern int dialcapas, dialfc;
    extern MDMINF * modemp[];
    MDMINF * p = NULL;
    long bits = 0;
#endif /* NODIAL */

    debug(F101,"setflow autoflow","",autoflow);

/* #ifdef COMMENT */
/* WHY WAS THIS COMMENTED OUT? */
    if (!autoflow)                      /* Only if FLOW is AUTO */
      return;
/* #endif */ /* COMMENT */

    debug(F101,"setflow local","",local);
    debug(F101,"setflow network","",network);
    debug(F101,"setflow cxtype","",cxtype);

    if (network || !local || cxtype == CXT_DIRECT) {
        flow = cxflow[cxtype];          /* Set appropriate flow control */
        debug(F101,"setflow flow","",flow);
        return;
    }
    if (cxtype != CXT_MODEM)            /* Connection type should be modem */
      return;

#ifndef NODIAL
    bits = dialcapas;                   /* Capability bits */
    if (!bits) {                        /* No bits? */
        p = modemp[mdmtyp];             /* Look in modem info structure */
        if (p)
          bits = p->capas;
    }
    if (dialfc == FLO_AUTO) {           /* If DIAL flow is AUTO */
#ifdef CK_RTSCTS                        /* If we can do RTS/CTS flow control */
        if (bits & CKD_HW)              /* and modem can do it too */
          flow = FLO_RTSC;              /* then switch to RTS/CTS */
        else                            /* otherwise */
          flow = FLO_XONX;              /* use Xon/Xoff. */
#else
#ifndef NEXT
#ifndef IRIX
        flow = FLO_XONX;                /* Use Xon/Xoff. */
#endif /* IRIX */
#endif /* NEXT */
#endif /* CK_RTSCTS */
    }
#endif /* NODIAL */
    debug(F101,"setflow modem flow","",flow);
    return;
}

#ifndef NOLOCAL
#ifdef CK_TRIGGER

/*  A U T O E X I T C H K  --  Check for CONNECT-mode trigger string  */
/*
  Returns -1 if trigger not found, or else the trigger index, 0 or greater.
  (Replace with fancier and more efficient matcher later...)
  NOTE: to prevent unnecessary function call overhead, call this way:

    x = tt_trigger[0] ? autoexitchk(c) : -1;

*/
int
#ifdef CK_ANSIC
autoexitchk(CHAR c)
#else
autoexitchk(c) CHAR c;
#endif /* CK_ANSIC */
/* autoexitchk */ {
    extern CHAR * tt_trmatch[];
    extern char * tt_trigger[];
    int i, n;
    for (i = 0; i < TRIGGERS; i++) {
        if (!tt_trigger[i]) {           /* No more triggers in list */
            break;
        } else if (*tt_trigger[i]) {
            if (!tt_trmatch[i])         /* Just starting? */
              tt_trmatch[i] = (CHAR *)tt_trigger[i]; /* Set match pointer */
            if (c == *tt_trmatch[i]) {  /* Compare this character */
                tt_trmatch[i]++;        /* It matches */
                if (!*tt_trmatch[i]) {  /* End of match string? */
                    tt_trmatch[i] = (CHAR *) tt_trigger[i]; /* Yes, rewind, */
                    debug(F101,"autoexitchk",tt_trigger[i],i); /* log, */
                    return(i);          /* and return success */
                }
            } else                      /* No match */
              tt_trmatch[i] = (CHAR *) tt_trigger[i]; /* Rewind match string */
        } /* and go on the next match string */
    }
    return(-1);                         /* No match found */
}
#endif /* CK_TRIGGER */

#ifndef NOSHOW
/*  S H O M D M  --  Show modem signals  */

VOID
shomdm() {
/*
  Note use of "\r\n" to make sure this report prints right, even when
  called during CONNECT mode.
*/
    int y;
    y = ttgmdm();
    switch (y) {
      case -3: printf(
                 "Modem signals unavailable in this version of Kermit\r\n");
               break;
      case -2: printf("No modem control for this device\r\n"); break;
      case -1: printf("Modem signals unavailable\r\n"); break;
      default:
#ifndef MAC
        printf(
          " Carrier Detect      (CD):  %s\r\n",(y & BM_DCD) ? "On": "Off");
        printf(
          " Dataset Ready       (DSR): %s\r\n",(y & BM_DSR) ? "On": "Off");
#endif /* MAC */
        printf(
          " Clear To Send       (CTS): %s\r\n",(y & BM_CTS) ? "On": "Off");
#ifndef STRATUS
#ifndef MAC
        printf(
          " Ring Indicator      (RI):  %s\r\n",(y & BM_RNG) ? "On": "Off");
#endif /* MAC */
        printf(
          " Data Terminal Ready (DTR): %s\r\n",
#ifdef NT
          "(unknown)"
#else /* NT */
          (y & BM_DTR) ? "On": "Off"
#endif /* NT */
          );
#ifndef MAC
        printf(
          " Request To Send     (RTS): %s\r\n",
#ifdef NT
          "(unknown)"
#else /* NT */
          (y & BM_RTS) ? "On": "Off"
#endif /* NT */
          );
#endif /* MAC */
#endif /* STRATUS */
    }
#ifdef BETATEST
#ifdef CK_TAPI
    if (tttapi && !tapipass) {
        LPDEVCFG        lpDevCfg = NULL;
        LPCOMMCONFIG    lpCommConfig = NULL;
        LPMODEMSETTINGS lpModemSettings = NULL;
        DCB *           lpDCB = NULL;

        if (cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,
                                    &lpCommConfig,&lpDCB)) {
            printf("\n");
            cktapiDisplayModemSettings(lpDevCfg,lpModemSettings,
                                       lpCommConfig,lpDCB);
        }
    }
#endif /* CK_TAPI */
#endif /* BETATEST */
}
#endif /* NOSHOW */
#endif /* NOLOCAL */

#ifndef NOXFER
/*  S D E B U  -- Record spar results in debugging log  */

VOID
sdebu(len) int len; {
    debug(F111,"spar: data",(char *) rdatap,len);
    debug(F101," spsiz ","", spsiz);
    debug(F101," timint","",timint);
    debug(F101," npad  ","",  npad);
    debug(F101," padch ","", padch);
    debug(F101," seol  ","",  seol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  rptq);
    debug(F101," rptflg","",rptflg);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotn","", wslotn);
    debug(F101," whatru","", whatru);
}
/*  R D E B U -- Debugging display of rpar() values  */

VOID
rdebu(d,len) CHAR *d; int len; {
    debug(F111,"rpar: data",d,len);
    debug(F101," rpsiz ","", xunchar(d[0]));
    debug(F101," rtimo ","", rtimo);
    debug(F101," mypadn","",mypadn);
    debug(F101," mypadc","",mypadc);
    debug(F101," eol   ","",   eol);
    debug(F101," ctlq  ","",  ctlq);
    debug(F101," sq    ","",    sq);
    debug(F101," ebq   ","",   ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," bctr  ","",  bctr);
    debug(F101," rptq  ","",  d[8]);
    debug(F101," rptflg","",rptflg);
    debug(F101," capas ","", capas);
    debug(F101," bits  ","",d[capas]);
    debug(F101," lscapu","",lscapu);
    debug(F101," atcapu","",atcapu);
    debug(F101," lpcapu","",lpcapu);
    debug(F101," swcapu","",swcapu);
    debug(F101," wslotr","", wslotr);
    debug(F101," rpsiz(extended)","",rpsiz);
}

#ifdef COMMENT
/*  C H K E R R  --  Decide whether to exit upon a protocol error  */

VOID
chkerr() {
    if (backgrd && !server) fatal("Protocol error");
}
#endif /* COMMENT */
#endif /* NOXFER */

/*  F A T A L  --  Fatal error message */

VOID
fatal(msg) char *msg; {
    extern int initflg;
    if (!msg) msg = "";
    debug(F111,"fatal",msg,initflg);
    if (!initflg)                       /* In case called from prescan. */
      sysinit();
#ifdef VMS
    if (strncmp(msg,"%CKERMIT",8))
      conol("%CKERMIT-E-FATAL, ");
    conoll(msg);
#else /* !VMS */
    conoll(msg);
#endif /* VMS */
    debug(F111,"fatal",msg,xitsta);
    tlog(F110,"Fatal:",msg,0L);
#ifdef OS2
    if (xfrbel) bleep(BP_FAIL);
    sleep(1);
    if (xfrbel) bleep(BP_FAIL);
#endif /* OS2 */
    doexit(BAD_EXIT,xitsta | 1);        /* Exit indicating failure */
}

#ifndef NOXFER
/*  B L D L E N  --  Make length-encoded copy of string  */

char *
bldlen(str,dest) char *str, *dest; {
    int len;
    len = (int)strlen(str);
    if (len > 94)
      *dest = SP;
    else
      *dest = (char) tochar(len);
    strcpy(dest+1,str);
    return(dest+len+1);
}


/*  S E T G E N  --  Construct a generic command  */
/*
  Call with Generic command character followed by three string arguments.
  Trailing strings are allowed to be empty ("").  Each string except the last
  non-empty string must be less than 95 characters long.  The final nonempty
  string is allowed to be longer.
*/
CHAR
#ifdef CK_ANSIC
setgen(char type, char * arg1, char * arg2, char * arg3)
#else
setgen(type,arg1,arg2,arg3) char type, *arg1, *arg2, *arg3;
#endif /* CK_ANSIC */
/* setgen */ {
    char *upstr, *cp;
#ifdef DYNAMIC
    if (!cmdstr)
      if (!(cmdstr = malloc(MAXSP + 1)))
        fatal("setgen: can't allocate memory");
#endif /* DYNAMIC */

    cp = cmdstr;
    *cp++ = type;
    *cp = NUL;
    if (!arg1) arg1 = "";
    if (!arg2) arg2 = "";
    if (!arg3) arg3 = "";
    if (*arg1 != NUL) {
        upstr = bldlen(arg1,cp);
        if (*arg2 != NUL) {
            upstr = bldlen(arg2,upstr);
            if (*arg3 != NUL) bldlen(arg3,upstr);
        }
    }
    cmarg = cmdstr;
    debug(F110,"setgen",cmarg,0);

    return('g');
}
#endif /* NOXFER */

#ifndef NOMSEND
static char *mgbufp = NULL;

/*  F N P A R S E  --  */

/*
  Argument is a character string containing one or more filespecs.
  This function breaks the string apart into an array of pointers, one
  to each filespec, and returns the number of filespecs.  Used by server
  when it receives a GET command to allow it to process multiple file
  specifications in one transaction.  Sets cmlist to point to a list of
  file pointers, exactly as if they were command line arguments.

  This version of fnparse treats spaces as filename separators.  If your
  operating system allows spaces in filenames, you'll need a different
  separator.

  This version of fnparse mallocs a string buffer to contain the names.  It
  cannot assume that the string that is pointed to by the argument is safe.
*/
int
fnparse(string) char *string; {
    char *p, *s, *q;
    int r = 0, x;                       /* Return code */
#ifdef RECURSIVE
    debug(F111,"fnparse",string,recursive);
#endif /* RECURSIVE */

    if (mgbufp) free(mgbufp);           /* Free this from last time. */
    mgbufp = malloc((int)strlen(string)+2);
    if (!mgbufp) {
        debug(F100,"fnparse malloc error","",0);
        return(0);
    }
#ifndef NOICP
#ifndef NOSPL
    ckstrncpy(fspec,string,fspeclen);   /* Make copy for \v(filespec) */
#endif /* NOSPL */
#endif /* NOICP */
    s = string;                         /* Input string */
    p = q = mgbufp;                     /* Point to the copy */
    r = 0;                              /* Initialize our return code */
    while (*s == SP || *s == HT)        /* Skip leading spaces and tabs */
      s++;
    for (x = strlen(s);                 /* Strip trailing spaces */
         (x > 1) && (s[x-1] == SP || s[x-1] == HT);
         x--)
      s[x-1] = NUL;
    while (1) {                         /* Loop through rest of string */
        if (*s == CMDQ) {               /* Backslash (quote character)? */
            if ((x = xxesc(&s)) > -1) { /* Go interpret it. */
                *q++ = (char) x;        /* Numeric backslash code, ok */
            } else {                    /* Just let it quote next char */
                s++;                    /* get past the backslash */
                *q++ = *s++;            /* deposit next char */
            }
            continue;
        } else if (*s == SP || *s == NUL) { /* Unquoted space or NUL? */
            *q++ = NUL;                 /* End of output filename. */
            msfiles[r] = p;             /* Add this filename to the list */
            debug(F111,"fnparse",msfiles[r],r);
            r++;                        /* Count it */
            if (*s == NUL) break;       /* End of string? */
            while (*s == SP) s++;       /* Skip repeated spaces */
            p = q;                      /* Start of next name */
            continue;
        } else *q++ = *s;               /* Otherwise copy the character */
        s++;                            /* Next input character */
    }
    debug(F101,"fnparse r","",r);
    msfiles[r] = "";                    /* Put empty string at end of list */
    cmlist = msfiles;
    return(r);
}
#endif /* NOMSEND */

char *                                  /* dbchr() for DEBUG SESSION */
dbchr(c) int c; {
    static char s[8];
    char *cp = s;

    c &= 0xff;
    if (c & 0x80) {                     /* 8th bit on */
        *cp++ = '~';
        c &= 0x7f;
    }
    if (c < SP) {                       /* Control character */
        *cp++ = '^';
        *cp++ = (char) ctl(c);
    } else if (c == DEL) {
        *cp++ = '^';
        *cp++ = '?';
    } else {                            /* Printing character */
        *cp++ = (char) c;
    }
    *cp = '\0';                         /* Terminate string */
    cp = s;                             /* Return pointer to it */
    return(cp);
}

/*  C K H O S T  --  Get name of local host (where C-Kermit is running)  */

/*
  Call with pointer to buffer to put hostname in, and length of buffer.
  Copies hostname into buffer on success, puts null string in buffer on
  failure.
*/
#ifdef BSD44
#define BSD4
#undef ATTSV
#endif /* BSD44 */

#ifdef SVORPOSIX
#ifndef BSD44
#ifndef apollo
#include <sys/utsname.h>
#endif /* apollo */
#endif /* BSD44 */
#else
#ifdef BELLV10
#include <utsname.h>
#endif /* BELLV10 */
#endif /* SVORPOSIX*/

#ifdef CKSYSLOG
extern char uidbuf[], * clienthost;
#endif /* CKSYSLOG */

VOID
ckhost(vvbuf,vvlen) char * vvbuf; int vvlen; {

#ifndef NOPUSH
    extern int nopush;
#ifndef NOSERVER
    extern int en_hos;
#endif /* NOSERVER */
#endif /* NOPUSH */

#ifdef pdp11
    *vvbuf = NUL;
#else  /* Everything else - rest of this routine */

    char *g;
#ifdef VMS
    int x;
#endif /* VMS */

#ifdef SVORPOSIX
#ifndef BSD44
#ifndef _386BSD
#ifndef APOLLOSR10
    struct utsname hname;
#endif /* APOLLOSR10 */
#endif /* _386BSD */
#endif /* BSD44 */
#endif /* SVORPOSIX */
#ifdef datageneral
    int ac0 = (char *) vvbuf, ac1 = -1, ac2 = 0;
#endif /* datageneral */

#ifndef NOPUSH
    if (getenv("CK_NOPUSH")) {          /* No shell access allowed */
        nopush = 1;                     /* on this host... */
#ifndef NOSERVER
        en_hos = 0;
#endif /* NOSERVER */
    }
#endif /* NOPUSH */

    *vvbuf = NUL;                       /* How let's get our host name ... */

#ifndef BELLV10                         /* Does not have gethostname() */
#ifndef OXOS
#ifdef SVORPOSIX
#ifdef APOLLOSR10
    ckstrncpy(vvbuf,"Apollo",vvlen);
#else
#ifdef BSD44
    if (gethostname(vvbuf,vvlen) < 0)
      *vvbuf = NUL;
#else
#ifdef _386BSD
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
#ifdef QNX
#ifdef TCPSOCKET
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else
    if (uname(&hname) > -1) ckstrncpy(vvbuf,hname.nodename,vvlen);
#endif /* TCPSOCKET */
#else /* SVORPOSIX but not _386BSD or BSD44 */
    if (uname(&hname) > -1) ckstrncpy(vvbuf,hname.nodename,vvlen);
#endif /* QNX */
#endif /* _386BSD */
#endif /* BSD44 */
#endif /* APOLLOSR10 */
#else /* !SVORPOSIX */
#ifdef BSD4
    if (gethostname(vvbuf,vvlen) < 0) *vvbuf = NUL;
#else /* !BSD4 */
#ifdef VMS
    g = getenv("SYS$NODE");
    if (g) ckstrncpy(vvbuf,g,vvlen);
    x = (int)strlen(vvbuf);
    if (x > 1 && vvbuf[x-1] == ':' && vvbuf[x-2] == ':') vvbuf[x-2] = NUL;
#else
#ifdef datageneral
    if (sys($HNAME,&ac0,&ac1,&ac2) == 0) /* successful */
        vvlen = ac2 + 1;                /* enh - have to add one */
#else
#ifdef OS2                              /* OS/2 */
    g = os2_gethostname();
    if (g) ckstrncpy(vvbuf,g,vvlen);
#else /* OS2 */
#ifdef OSK
#ifdef TCPSOCKET
        if (gethostname(vvbuf, vvlen) < 0) *vvbuf = NUL;
#endif /* TCPSOCKET */
#endif /* OSK */
#endif /* OS2 */
#endif /* datageneral */
#endif /* VMS */
#endif /* BSD4 */
#endif /* SVORPOSIX */
#else /* OXOS */
    /* If TCP/IP is not installed, gethostname() fails, use uname() */
    if (gethostname(vvbuf,vvlen) < 0) {
        if (uname(&hname) > -1)
            ckstrncpy(vvbuf,hname.nodename,vvlen);
        else
            *vvbuf = NUL;
    }
#endif /* OXOS */
#endif /* BELLV10 */
    if (*vvbuf == NUL) {                /* If it's still empty */
        g = getenv("HOST");             /* try this */
        if (g) ckstrncpy(vvbuf,g,vvlen);
    }
    vvbuf[vvlen-1] = NUL;               /* Make sure result is terminated. */
#endif /* pdp11 */
}
#ifdef BSD44
#undef BSD4
#define ATTSV
#endif /* BSD44 */

/*
  A S K M O R E  --  Poor person's "more".
  Returns 0 if no more, 1 if more wanted.
*/
int
askmore() {
    char c; int rv;
#ifdef IKSD
    int cx;
    extern int timelimit;
#endif /* IKSD */

#ifdef NOICP
    return(1);
#else
    if (!xaskmore)
      return(1);
#ifdef VMS
    if (batch)
      return(1);
#else
#ifdef UNIX
    if (backgrd)
      return(1);
#endif /* UNIX */
#endif /* VMS */

    concb((char)escape);                /* Force CBREAK mode. */

    rv = -1;
    while (rv < 0) {
#ifndef OS2
        printf("more? ");
#ifdef UNIX
#ifdef NOSETBUF
        fflush(stdout);
#endif /* NOSETBUF */
#endif /* UNIX */
#else
        printf("more? (Y or space-bar for yes, N for no) ");
        fflush(stdout);
#endif /* OS2 */
#ifdef IKSD
        if (inserver) {
            cx = cmdgetc(timelimit);
            if (cx < -1 && timelimit) {
                printf("\n?IKS idle timeout - Goodbye.\n");
                doexit(GOOD_EXIT,0);
            }
            c = (char) cx;
        } else
#endif /* IKSD */
        c = (char) cmdgetc(0);
        switch (c) {
          /* Yes */
          case SP: case 'y': case 'Y': case 012:  case 015:
#ifdef OSK
            write(1, "\015      \015", sizeof "\015      \015" - 1);
#else
            printf("\015      \015");
#endif /* OSK */
            rv = 1;
            break;
          /* No */
          case 'n': case 'N': case 'q': case 'Q':
#ifdef OSK
            printf("\n");
#else
            printf("\015\012");
#endif /* OSK */
            rv = 0;
            break;
          /* Invalid answer */
          default:
            debug(F111,"askmore","invalid answer",c);
            printf("Y or space-bar for yes, N for no\n");
            continue;
        }
#ifdef OS2
        printf("\r                                         \r");
        fflush(stdout);
#endif /* OS2 */
    }
    return(rv);
#endif /* NOICP */
}

/*  T R A P  --  Terminal interrupt handler */

SIGTYP
#ifdef CK_ANSIC
trap(int sig)
#else
trap(sig) int sig;
#endif /* CK_ANSIC */
/* trap */ {
    extern int b_save, f_save;
#ifndef NOICP
    extern int timelimit;
#endif /* NOICP */
#ifdef OS2
    extern int os2gks;
    int i;
#endif /* OS2 */
#ifndef NOSPL
    extern int i_active, instatus;
#endif /* NOSPL */
#ifdef VMS
    int i; FILE *f;
#endif /* VMS */
    extern int zchkod, zchkid;

#ifdef NTSIG
    connoi();
#endif /* NTSIG */
#ifdef __EMX__
    signal(SIGINT, SIG_ACK);
#endif
#ifdef GEMDOS
/* GEM is not reentrant, no i/o from interrupt level */
    cklongjmp(cmjbuf,1);                /* Jump back to parser now! */
#endif /* GEMDOS */

    debug(F101,"trap signal","",sig);
#ifndef NOICP
    timelimit = 0;                      /* In case timed ASK interrupted */
#endif /* NOICP */
    zchkod = 0;                         /* Or file expansion interrupted... */
    zchkid = 0;
    interrupted = 1;

    if (what == W_CONNECT) {            /* Are we in CONNECT mode? */
/*
  The HP workstation Reset key sends some kind of ueber-SIGINT that can not
  be SIG_IGNored, so we wind up here somehow (even though this is *not* the
  current SIGINT handler).  Just return.
*/
        debug(F101,"trap: SIGINT caught during CONNECT","",sig);
        SIGRETURN;
    }
#ifndef NOSPL
    if (i_active) {                     /* INPUT command was active? */
        i_active = 0;                   /* Not any more... */
        instatus = INP_UI;              /* INPUT status = User Interrupted */
    }
#endif /* NOSPL */

#ifndef NOXFER
    ftreset();                          /* Restore global protocol settings */
    binary = b_save;                    /* Then restore these */
    fncnv  = f_save;
    bye_active = 0;
#endif /* NOXFER */
    zclose(ZIFILE);                     /* If we were transferring a file, */
    zclose(ZOFILE);                     /* close it. */
#ifndef NOICP
    cmdsquo(cmd_quoting);               /* If command quoting was turned off */
    if (saveask > -1) {
        xaskmore = saveask;
        saveask = -1;
    }
#endif /* NOICP */
#ifdef CK_APC
    delmac("_apc_commands");
    apcactive = APC_INACTIVE;
#endif /* CK_APC */

#ifdef VMS
/*
  Fix terminal.
*/
    if (ft_win) {                       /* If curses window open */
        debug(F100,"^C trap() curses","",0);
        xxscreen(SCR_CW,0,0L,"");       /* Close it */
        conres();                       /* Restore terminal */
        i = printf("^C...");            /* Echo ^C to standard output */
    } else {
        conres();
        i = printf("^C...\n");          /* Echo ^C to standard output */
    }
    if (i < 1 && ferror(stdout)) {      /* If there was an error */
        debug(F100,"^C trap() error","",0);
        fclose(stdout);                 /* close standard output */
        f = fopen(dftty, "w");          /* open the controlling terminal */
        if (f) stdout = f;              /* and make it standard output */
        printf("^C...\n");              /* and echo the ^C again. */
    }
#else                                   /* Not VMS */
#ifdef STRATUS
    conres();                           /* Set console back to normal mode */
#endif /* STRATUS */
#ifndef NOXFER
    if (ft_win) {                       /* If curses window open, */
        debug(F100,"^C trap() curses","",0);
        xxscreen(SCR_CW,0,0L,"");               /* close it. */
        printf("^C...");                /* Echo ^C to standard output */
    } else {
#endif /* NOXFER */
        printf("^C...\n");
#ifndef NOXFER
    }
#endif /* NOXFER */
#endif /* VMS */
#ifdef datageneral
    connoi_mt();                        /* Kill asynch task that listens to */
    ttimoff();
    conres();                           /* the keyboard */
#endif /* datageneral */

#ifndef NOCCTRAP
/*  This is stupid -- every version should have ttimoff()...  */
#ifdef UNIX
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef OSK
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef STRATUS
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef OS2
    os2gks = 1;                         /* Turn back on keycode mapping  */
    for (i = 0; i < VNUM; i++)
      VscrnResetPopup(i);
#ifdef TCPSOCKET
#ifdef NT
    /* WSAIsBlocking() returns FALSE in Win95 during a blocking accept call */
    if ( WSASafeToCancel /* && WSAIsBlocking() */ ) {
        WSACancelBlockingCall();
    }
#endif /* NT */
#endif /* TCPSOCKET */
#ifdef CK_NETBIOS
    NCBCancelOutstanding();
#endif /* CK_NETBIOS */
    ttimoff();                          /* Turn off any timer interrupts */
#else
#ifdef VMS
    ttimoff();                          /* Turn off any timer interrupts */
#endif /* VMS */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* OSK */
#endif /* UNIX */

#ifdef OSK
    sigmask(-1);
/*
  We are in an intercept routine but do not perform a F$RTE (done implicitly
  but rts).  We have to decrement the sigmask as F$RTE does.  Warning:
  longjump only restores the cpu registers, NOT the fpu registers.  So don't
  use fpu at all or at least don't use common fpu (double or float) register
  variables.
*/
#endif /* OSK */

#ifdef NTSIG
    PostCtrlCSem();
#else /* NTSIG */
#ifdef NT
    cklongjmp(ckjaddr(cmjbuf),1);
#else /* NT */
    cklongjmp(cmjbuf,1);
#endif /* NT */
#endif /* NTSIG */
#else /* NOCCTRAP */
/* No Ctrl-C trap, just exit. */
#ifdef CK_CURSES                        /* Curses support? */
    xxscreen(SCR_CW,0,0L,"");           /* Close curses window */
#endif /* CK_CURSES */
    doexit(BAD_EXIT,what);              /* Exit poorly */
#endif /* NOCCTRAP */
    SIGRETURN;
}

/*  C K _ T I M E  -- Returns pointer to current time. */

char *
ck_time() {
    static char tbuf[10];
    char *p;
    int x;

    ztime(&p);                          /* "Thu Feb  8 12:00:00 1990" */
    if (!p)                             /* like asctime()! */
      return("");
    if (*p) {
        for (x = 11; x < 19; x++)       /* copy hh:mm:ss */
          tbuf[x - 11] = p[x];          /* to tbuf */
        tbuf[8] = NUL;                  /* terminate */
    }
    return(tbuf);                       /* and return it */
}

/*  C C _ C L E A N  --  Cleanup after terminal interrupt handler */

#ifdef GEMDOS
int
cc_clean() {
    zclose(ZIFILE);                     /* If we were transferring a file, */
    zclose(ZOFILE);                     /* close it. */
    printf("^C...\n");                  /* Not VMS, no problem... */
}
#endif /* GEMDOS */


/*  S T P T R A P -- Handle SIGTSTP (suspend) signals */

SIGTYP
#ifdef CK_ANSIC
stptrap(int sig)
#else
stptrap(sig) int sig;
#endif /* CK_ANSIC */
/* stptrap */ {

#ifndef NOJC
    int x; extern int cmflgs;
    debug(F101,"stptrap() caught signal","",sig);
    if (!suspend) {
        printf("\r\nsuspend disabled\r\n");
#ifndef NOICP
        if (what == W_COMMAND) {        /* If we were parsing commands */
            prompt(xxstring);           /* reissue the prompt and partial */
            if (!cmflgs)                /* command (if any) */
              printf("%s",cmdbuf);
        }
#endif /* NOICP */
    } else {
        conres();                       /* Reset the console */
#ifndef OS2
        /* Flush pending output first, in case we are continued */
        /* in the background, which could make us block */
        fflush(stdout);

        x = psuspend(suspend);          /* Try to suspend. */
        if (x < 0)
#endif /* OS2 */
          printf("Job control not supported\r\n");
        conint(trap,stptrap);           /* Rearm the trap. */
        debug(F100,"stptrap back from suspend","",0);
        switch (what) {
          case W_CONNECT:               /* If suspended during CONNECT? */
            conbin((char)escape);       /* put console back in binary mode */
            debug(F100,"stptrap W_CONNECT","",0);
            break;
#ifndef NOICP
          case W_COMMAND:               /* Suspended in command mode */
            debug(F101,"stptrap W_COMMAND pflag","",pflag);
            concb((char)escape);        /* Put back CBREAK tty mode */
            if (pflag) {                /* If command parsing was */
                prompt(xxstring);       /* reissue the prompt and partial */
                if (!cmflgs)            /* command (if any) */
                  printf("%s",cmdbuf);
            }
            break;
#endif /* NOICP */
          default:                      /* All other cases... */
            debug(F100,"stptrap default","",0);
            concb((char)escape);        /* Put it back in CBREAK mode */
            break;
        }
    }
#endif /* NOJC */
    SIGRETURN;
}

#ifdef TLOG
#define TBUFL 300

/*  T L O G  --  Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1,s2 - String arguments 1 and 2.
   n  - Int, argument 3.
*/
VOID
#ifdef CK_ANSIC
dotlog(int f, char *s1, char *s2, long n)
#else
dotlog(f,s1,s2,n) int f; long n; char *s1, *s2;
#endif /* CK_ANSIC */
/* dotlog */ {
    static char s[TBUFL];
    char *sp = s; int x;
    if (!s1) s1 = "";
    if (!s2) s2 = "";

    if (!tralog) return;                /* If no transaction log, don't */
    switch (f) {
      case F000:                        /* 0 (special) "s1 n s2"  */
        if ((int)strlen(s1) + (int)strlen(s2) + 15 > TBUFL)
          sprintf(sp,"?T-Log string too long");
        else sprintf(sp,"%s %ld %s",s1,n,s2);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F001:                        /* 1, " n" */
        sprintf(sp," %ld",n);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F010:                        /* 2, "[s2]" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if (x + 6 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"[%s]",s2);
        if (zsoutl(ZTFILE,"") < 0) tralog = 0;
        break;
      case F011:                        /* 3, "[s2] n" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if (x + 6 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"[%s] %ld",s2,n);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F100:                        /* 4, "s1" */
        if (zsoutl(ZTFILE,s1) < 0) tralog = 0;
        break;
      case F101:                        /* 5, "s1: n" */
        if ((int)strlen(s1) + 15 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"%s: %ld",s1,n);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F110:                        /* 6, "s1 s2" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if ((int)strlen(s1) + x + 4 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"%s %s",s1,s2);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      case F111:                        /* 7, "s1 s2: n" */
        x = (int)strlen(s2);
        if (s2[x] == '\n') s2[x] = '\0';
        if ((int)strlen(s1) + x + 15 > TBUFL)
          sprintf(sp,"?String too long");
        else sprintf(sp,"%s %s: %ld",s1,s2,n);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
        break;
      default:
        sprintf(sp,"?Invalid format for tlog() - %ld",n);
        if (zsoutl(ZTFILE,s) < 0) tralog = 0;
    }
}

/*
  D O X L O G

  This is the transaction-log writer for BRIEF format.
  The idea is produce one record (line) per file.  Each record
  has the following delimited fields:
    Date (yyyymmdd)
    Time (hh:mm:ss)
    Action: SEND or RECV
    File name
    File size
    Transfer mode (text, binary, image, labeled, etc).
    Status: OK or FAILED
    Free-form comments in doublequotes
  The default separator is comma.
  If a field contains the separator, it is enclosed in doublequotes.
*/
VOID
#ifdef CK_ANSIC
doxlog(int x, char * fn, long fs, int fm, int status, char * msg)
#else
doxlog(x, fn, fs, fm, status, msg)
    int x; char * fn; long fs; int fm; int status; char * msg;
#endif /* CK_ANSIC */
/* doxlog */ {
    extern int tlogsep;
    char sep[2];
    char buf[CKMAXPATH+256];
    char tmpbuf[32];
    char * s, * p;
    int len;

    if (!tralog) return;                /* If no transaction log, don't */

    if (!fn) fn = "";                   /* Protect against null pointers */
    if (!msg) msg = "";

    sep[0] = (char) tlogsep;
    sep[1] = NUL;
    if (!sep[0]) sep[0] = ',';

    p = zzndate();                      /* Date and time */
    if (p) {
        ckstrncpy(buf,p,CKMAXPATH);     /* Shouldn't be more than 30 chars */
    } else {
        strcpy(buf,"00000000");
    }
    ztime(&p);
    strcat(buf,sep);
    strncat(buf,p+11,8);
    strcat(buf,sep);
    debug(F110,"doxlog 1",buf,0);

    strcat(buf, (x == W_SEND) ? "SEND" : "RECV"); /* What */
    strcat(buf,sep);
    debug(F110,"doxlog 2",buf,0);

    if (ckstrchr(fn,sep[0])) {          /* Filename */
        strcat(buf,"\"");               /* Might contain separator */
        strcat(buf,fn);
        strcat(buf,"\"");
    } else
      strcat(buf,fn);
    strcat(buf,sep);
    debug(F110,"doxlog 3",buf,0);

    sprintf(tmpbuf,"%ld",fs);           /* Size */
    strcat(buf,tmpbuf);
    strcat(buf,sep);
    debug(F110,"doxlog 4",buf,0);

#ifdef NOICP
    strcpy(tmpbuf, (binary ? "binary" : "text")); /* Transfer mode */
#else
    strcpy(tmpbuf,gfmode(fm,0));        /* Transfer mode */
#endif /* NOICP */
    if (ckstrchr(tmpbuf,sep[0])) {      /* Might contain spaces */
        strcat(buf,"\"");
        strcat(buf,tmpbuf);
        strcat(buf,"\"");
    } else
      strcat(buf,tmpbuf);
    strcat(buf,sep);
    debug(F110,"doxlog 5",buf,0);

    strcat(buf, status ? "FAILED" : "OK");
    len = strlen(buf);
    debug(F110,"doxlog buf 1", buf, len);
    s = buf + len;
    if (status == 0) {
        long cps;
#ifdef GFTIMER
        cps = (long)((CKFLOAT) fs / fpxfsecs);
        sprintf(s,"%s\"%0.3fsec %ldcps\"",sep,fpxfsecs,cps);
#else
        cps = fs / xfsecs;
        sprintf(s,"%s\"%ldsec %ldcps\"",sep,xfsecs,cps);
#endif /* GFTIMER */
    } else {
        sprintf(s,"%s\"%s\"",sep,msg);
    }
    debug(F110,"doxlog 5",buf,0);
    x = zsoutl(ZTFILE,buf);
    debug(F101,"doxlog zsoutl","",x);
    if (x < 0) tralog = 0;
}
#endif /* TLOG */

#ifndef MAC
/*
  The rest of this file is for all implementations but the Macintosh.
*/

#ifndef NOXFER
/*  C H K I N T  --  Check for console interrupts  */

/*
  Used during file transfer in local mode only:
  . If user has not touched the keyboard, returns 0 with no side effects.
  . If user typed S or A (etc, see below) prints status message and returns 0.
  . If user typed X or F (etc, see below) returns 0 with cxseen set to 1.
  . If user typed Z or B (etc, see below) returns 0 with czseen set to 1.
  . If user typed E or C (etc, see below) returns -1.
*/
#ifdef CK_CURSES
static int repaint = 0;                 /* Transfer display needs repainting */
#endif /* CK_CURSES */

int
chkint() {
    int ch, cn, ofd; long zz;
    if (!xfrint)
      return(0);
    if ((!local) || (quiet)) return(0); /* Only do this if local & not quiet */
#ifdef datageneral
    if (con_reads_mt)                   /* if conint_mt task is active */
      if (conint_avl) {                 /* and there's an interrupt pending */
          cn = 1;                       /* process it */
          ch = conint_ch;
          conint_avl = 0;               /* turn off flag so conint_mt can */
      } else                            /* proceed */
        return(0);
    else                                /* if conint_mt not active */
      if ((ch = coninc(2)) < 0)         /* try to get char manually */
        return(0);                      /* I/O error, or no data */
      else                              /* if successful, set cn so we */
        cn = 1;                         /* know we got one */
    debug(F101,"chkint got keyboard character",ch,cn);
#else /* !datageneral */
#ifdef NTSIG
    {
        extern int TlsIndex;
        struct _threadinfo * threadinfo;
        threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex);
        if (threadinfo) {
            if (!WaitSem(threadinfo->DieSem,0))
              return -1;                /* Cancel Immediately */
        }
    }
#endif /* NTSIG */
    cn = conchk();                      /* Any input waiting? */
    debug(F101,"conchk","",cn);
    if (cn < 1) return(0);
    ch = coninc(5) ;
    debug(F101,"coninc","",ch);
    if (ch < 0) return(0);
#endif /* datageneral */

    switch (ch & 0177) {
      case 'A': case 'a': case 0001:    /* Status report */
      case 'S': case 's':
        if (fdispla != XYFD_R && fdispla != XYFD_S && fdispla != XYFD_N)
          return(0);                    /* Only for serial, simple or none */
        ofd = fdispla;                  /* [MF] Save file display type */
        if (fdispla == XYFD_N)
          fdispla = XYFD_R;             /* [MF] Pretend serial if no display */
        xxscreen(SCR_TN,0,0l,"Status report:");
        xxscreen(SCR_TN,0,0l," file type: ");
        if (binary) {
            switch(binary) {
              case XYFT_L: xxscreen(SCR_TZ,0,0l,"labeled"); break;
              case XYFT_I: xxscreen(SCR_TZ,0,0l,"image"); break;
              case XYFT_U: xxscreen(SCR_TZ,0,0l,"binary undefined"); break;
              default:
              case XYFT_B: xxscreen(SCR_TZ,0,0l,"binary"); break;
            }
        } else {
#ifdef NOCSETS
            xxscreen(SCR_TZ,0,0l,"text");
#else
            xxscreen(SCR_TU,0,0l,"text, ");
            if (tcharset == TC_TRANSP) {
                xxscreen(SCR_TZ,0,0l,"transparent");
            } else {
                if (what == W_SEND) {
                    xxscreen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
                    xxscreen(SCR_TU,0,0l," => ");
                    xxscreen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
                } else {
                    xxscreen(SCR_TZ,0,0l,fcsinfo[fcharset].keyword);
                    xxscreen(SCR_TU,0,0l," => ");
                    xxscreen(SCR_TZ,0,0l,tcsinfo[tcharset].keyword);
                }
            }
#endif /* NOCSETS */
        }
        xxscreen(SCR_QE,0,filcnt," file number");
        if (fsize) xxscreen(SCR_QE,0,fsize," size");
        xxscreen(SCR_QE,0,ffc," characters so far");
        if (fsize > 0L) {
#ifdef CK_RESEND
            zz = what == W_SEND ? sendstart : what == W_RECV ? rs_len : 0;
            zz = ( (ffc + zz) * 100L ) / fsize;
#else
            zz = ( ffc * 100L ) / fsize;
#endif /* CK_RESEND */
            xxscreen(SCR_QE,0,zz,      " percent done");
        }
        if (bctu == 4) {                /* Block check */
            xxscreen(SCR_TU,0,0L," block check: ");
            xxscreen(SCR_TZ,0,0L,"blank-free-2");
        } else xxscreen(SCR_QE,0,(long)bctu,  " block check");
        xxscreen(SCR_QE,0,(long)rptflg," compression");
        xxscreen(SCR_QE,0,(long)ebqflg," 8th-bit prefixing");
        xxscreen(SCR_QE,0,(long)lscapu," locking shifts");
        if (!network)
          xxscreen(SCR_QE,0, speed, " speed");
        if (what == W_SEND)

          xxscreen(SCR_QE,0,(long)spsiz, " packet length");
        else if (what == W_RECV || what == W_REMO)
          xxscreen(SCR_QE,0,(long)urpsiz," packet length");
        xxscreen(SCR_QE,0,(long)wslots,  " window slots");
        fdispla = ofd; /* [MF] Restore file display type */
        return(0);

      case 'B': case 'b': case 0002:    /* Cancel batch */
      case 'Z': case 'z': case 0032:
        czseen = 1;
        interrupted = 1;
        xxscreen(SCR_ST,ST_MSG,0l,
                 (((what == W_RECV) && (wslots > 1)) ?
                  "Canceling batch, wait... " :
                  "Canceling batch... ")
                 );
        return(0);

      case 'F': case 'f': case 0006:    /* Cancel file */
      case 'X': case 'x': case 0030:
        cxseen = 1;
        interrupted = 1;
        xxscreen(SCR_ST,ST_MSG,0l,
                 (((what == W_RECV) && (wslots > 1)) ?
                  "Canceling file, wait... " :
                  "Canceling file... ")
                 );
        return(0);

      case 'R': case 'r': case 0022:    /* Resend packet */
      case 0015: case 0012:
#ifdef STREAMING
        if (streaming)
          return(0);
#endif /* STREAMING */
        xxscreen(SCR_ST,ST_MSG,0l,"Resending packet... ");
        numerrs++;
        resend(winlo);
        return(0);

#ifdef datageneral
      case '\03':                       /* We're not trapping ^C's with */
        trap(0);                        /* signals, so we check here    */
#endif /* datageneral */

      case 'C': case 'c':               /* Ctrl-C */
#ifndef datageneral
      case '\03':
#endif /* datageneral */

      case 'E': case 'e':               /* Send error packet */
      case 0005:
        interrupted = 1;
        return(-1);

#ifdef CK_CURSES
      case 0014:                        /* Ctrl-L to refresh screen */
      case 'L': case 'l':               /* Also accept L (upper, lower) */
      case 0027:                        /* Ctrl-W synonym for VMS & Ingres */
        repaint = 1;
        return(0);
#endif /* CK_CURSES */

      default:                          /* Anything else, print message */
        intmsg(1L);
        return(0);
    }
}

/*  I N T M S G  --  Issue message about terminal interrupts  */

VOID
#ifdef CK_ANSIC
intmsg(long n)
#else
intmsg(n) long n;
#endif /* CK_ANSIC */
/* intmsg */ {
#ifdef CK_NEED_SIG
    char buf[80];
#endif /* CK_NEED_SIG */

    if (!displa || quiet)               /* Not if we're being quiet */
      return;
    if (server && (!srvdis || n > -1L)) /* Special for server */
      return;
#ifdef CK_NEED_SIG
    buf[0] = NUL;                       /* Keep compilers happy */
#endif /* CK_NEED_SIG */
#ifndef OXOS
#ifdef SVORPOSIX
    conchk();                           /* Clear out pending escape-signals */
#endif /* SVORPOSIX */
#endif /* ! OXOS */
#ifdef VMS
    conres();                           /* So Ctrl-C will work */
#endif /* VMS */
    if ((!server && n == 1L) || (server && n < 0L)) {

#ifdef CK_NEED_SIG
        if (xfrint) {
            sprintf(buf,
                    "Type escape character (%s) followed by:",
                    dbchr(escape)
                    );
            xxscreen(SCR_TN,0,0l,buf);
        }
#endif /* CK_NEED_SIG */

        if (xfrint) {
            if (protocol == PROTO_K) {
 xxscreen(SCR_TN,0,0l,"X to cancel file,  CR to resend current packet");
 xxscreen(SCR_TN,0,0l,"Z to cancel group, A for status report");
 xxscreen(SCR_TN,0,0l,"E to send Error packet, Ctrl-C to quit immediately: ");
            } else {
                xxscreen(SCR_TN,0,0l,"Ctrl-C to cancel file transfer: ");
            }
        } else {
            xxscreen(SCR_TN,0,0l,"Transfer interruption disabled. ");
        }
    }
    else xxscreen(SCR_TU,0,0l," ");
}

#ifndef NODISPLAY
static int newdpy = 0;                  /* New display flag */
static char fbuf[80];                   /* Filename buffer */
static char abuf[80];                   /* As-name buffer */
static char a2buf[80];                  /* Second As-name buffer */
static long oldffc = 0L;
static long dots = 0L;
static int hpos = 0;

static VOID                             /* Initialize Serial or CRT display */
dpyinit() {
    int m = 0, n = 0;
    char * s = "";

    newdpy = 0;                         /*  Don't do this again */
    oldffc = 0L;                        /*  Reset this */
    dots = 0L;                          /*  and this.. */
    oldcps = cps = 0L;

    conoll("");                                         /* New line */
    if (what == W_SEND) s = "Sending: ";                /* Action */
    else if (what == W_RECV) s = "Receiving: ";
    n = (int)strlen(s) + (int)strlen(fbuf);
    conol(fbuf);
    m = (int)strlen(abuf) + 4;
    if (n + m > cmd_cols) {
        conoll("");
        n = 0;
    } else
      n += m;
    if (*abuf) {
        conol(" => ");
        conol(abuf);
    }
    m = (int)strlen(a2buf) + 4;
    if (n + m > cmd_cols) {
        conoll("");
        n = 0;
    } else
      n += m;
    if (*a2buf) {
        conol(" => ");
        conol(a2buf);
    }
    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
    conoll("");
    if (fsize > -1L) {                                  /* Size */
        sprintf(fbuf,"Size: %ld, Type: ",fsize);
        conol(fbuf); *fbuf = NUL;
    } else conol("Size: unknown, Type: ");
    if (binary) {                                       /* Type */
        switch(binary) {
              case XYFT_L: conoll("labeled"); break;
              case XYFT_I: conoll("image"); break;
              case XYFT_U: conoll("binary undefined"); break;
              default:
              case XYFT_B: conoll("binary"); break;
        }
    } else {
#ifdef NOCSETS
        conoll("text");
#else
        conol("text, ");
        if (tcharset == TC_TRANSP) {
            conoll("transparent");
        } else {
            if (what == W_SEND) {
                conol(fcsinfo[fcharset].keyword);
                conol(" => ");
                conoll(tcsinfo[tcharset].keyword);
            } else {
                conol(tcsinfo[tcharset].keyword);
                conol(" => ");
                conoll(fcsinfo[fcharset].keyword);
            }
        }
#endif /* NOCSETS */
    }
    if (fdispla == XYFD_S) {            /* CRT field headings */
/*
  Define CK_CPS to show current transfer rate.
  Leave it undefined to show estimated time remaining.
  Estimated-time-remaining code from Andy Fyfe, not tested on
  pathological cases.
*/
#define CK_CPS

#ifdef CK_CPS
        conoll("    File   Percent       Packet");
        conoll("    Bytes  Done     CPS  Length");
#else
        conoll("    File   Percent  Secs Packet");
        conoll("    Bytes  Done     Left Length");
#endif /* CK_CPS */
        newdpy = 0;
    }
    hpos = 0;
}

/*
  showpkt(c)
  c = completion code: 0 means transfer in progress, nonzero means it's done.
  Show the file transfer progress counter and perhaps verbose packet type.
*/
VOID
#ifdef CK_ANSIC
showpkt(char c)
#else
showpkt(c) char c;
#endif /* CK_ANSIC */
/* showpkt */ {

    long howfar;                        /* How far into file */
    long et;                            /* Elapsed time, entire batch  */
    long pd;                            /* Percent done, this file     */
    long tp;                            /* Transfer rate, entire batch */
    long ps;                            /* Packet size, current packet */
    long mytfc;                         /* Local copy of byte counter  */

#ifdef GFTIMER
    CKFLOAT tnow;
#endif /* GFTIMER */

    if (newdpy)                         /* Put up filenames, etc, */
      dpyinit();                        /* if they're not there already. */

    howfar = ffc;                       /* How far */
/*
  Calculate CPS rate even if not displaying on screen for use in file
  transfer statistics.
*/
#ifdef GFTIMER
    tnow = gftimer();                   /* Time since we started */
    ps = (what == W_RECV) ? rpktl : spktl; /* Packet size */
#ifdef CK_RESEND
    if (what == W_SEND)                 /* In case we didn't start at */
      howfar += sendstart;              /*  the beginning... */
    else if (what == W_RECV)
      howfar += rs_len;
#endif /* CK_RESEND */
    pd = -1;                            /* Percent done. */
    if (c == NUL) {                     /* Still going, figure % done */
        if (fsize == 0L) return;        /* Empty file, don't bother */
        pd = (fsize > 99L) ? (howfar / (fsize / 100L)) : 0L;
        if (pd > 100) pd = 100;         /* Expansion */
    }
    if (c != NUL)
      if (!cxseen && !discard && !czseen)
        pd = 100;                       /* File complete, so 100%. */

    mytfc = (pd < 100) ? tfc + ffc : tfc;    /* CPS */
    tp = (long)((tnow > 0.0) ? (CKFLOAT) mytfc / tnow : 0);
    if (c && (tp == 0))
      tp = ffc;

    cps = tp;                           /* Set global variable */
    if (cps > peakcps &&                /* Peak transfer rate */
         ((what == W_SEND && spackets > wslots + 4) ||
         (what != W_SEND && spackets > 10))) {
        peakcps = cps;
    }

#else  /* Not GFTIMER */

    et = gtimer();                      /* Elapsed time  */
    ps = (what == W_RECV) ? rpktl : spktl; /* Packet length */
#ifdef CK_RESEND
    if (what == W_SEND)                 /* And if we didn't start at */
      howfar += sendstart;              /*  the beginning... */
    else if (what == W_RECV)
      howfar += rs_len;
#endif /* CK_RESEND */
    pd = -1;                            /* Percent done. */
    if (c == NUL) {                     /* Still going, figure % done */
        if (fsize == 0L) return;        /* Empty file, don't bother */
        pd = (fsize > 99L) ? (howfar / (fsize / 100L)) : 0L;
        if (pd > 100) pd = 100;         /* Expansion */
    }
    if (c != NUL)
      if (!cxseen && !discard && !czseen)
        pd = 100;                       /* File complete, so 100%. */


#ifndef CK_CPS
/*
  fsecs = time (from gtimer) that this file started (set in sfile()).
  Rate so far is ffc / (et - fsecs),  estimated time for remaining bytes
  is (fsize - ffc) / ( ffc / (et - fsecs )).
*/
    tp = (howfar > 0L) ? (fsize - howfar) * (et - fsecs) / howfar : 0L;
#endif /* CK_CPS */

#ifdef CK_CPS
    mytfc = (pd < 100) ? tfc + ffc : tfc;
    tp = (et > 0) ? mytfc / et : 0; /* Transfer rate */
    if (c && (tp == 0))         /* Watch out for subsecond times */
        tp = ffc;

    cps = tp;                       /* Set global variable */
    if (cps > peakcps &&                /* Peak transfer rate */
         ((what == W_SEND && spackets > wslots + 4) ||
         (what != W_SEND && spackets > 10))) {
        peakcps = cps;
    }
#endif /* CK_CPS */

#endif /* GFTIMER */

    if (fdispla == XYFD_S) {            /* CRT display */
        char buffer[40];

        if (pd > -1L)
          sprintf(buffer, "%c%9ld%5ld%%%8ld%8ld ", CR, howfar, pd, tp, ps);
        else
          sprintf(buffer, "%c%9ld      %8ld%8ld ", CR, howfar, tp, ps);
        conol(buffer);
        hpos = 31;
    } else if (fdispla == XYFD_R) {     /* SERIAL */
        long i, k;
        if (howfar - oldffc < 1024)     /* Update display every 1K */
          return;
        oldffc = howfar;                /* Time for new display */
        k = (howfar / 1024L) - dots;    /* How many K so far */
        for (i = 0L; i < k; i++) {
            if (hpos++ > (cmd_cols - 3)) { /* Time to wrap? */
                conoll("");
                hpos = 0;
            }
            conoc('.');                 /* Print a dot for this K */
            dots++;                     /* Count it */
        }
    }
}


/*  C K S C R E E N  --  Screen display function  */

/*
  ckscreen(f,c,n,s)
    f - argument descriptor
    c - a character or small integer
    n - a long integer
    s - a string.
  Fill in this routine with the appropriate display update for the system.
    FILE DISPLAY SERIAL:     Dots, etc, works on any terminal, even hardcopy.
    FILE DISPLAY CRT:        Works on any CRT, writes over current line.
    FILE DISPLAY FULLSCREEN: Requires terminal-dependent screen control.
    FILE DISPLAY BRIEF:      Like SERIAL but only filename & completion status.
*/
VOID
#ifdef CK_ANSIC
ckscreen(int f, char c,long n,char *s)
#else
ckscreen(f,c,n,s) int f; char c; long n; char *s;
#endif /* CK_ANSIC */
/* screen */ {
    char buf[80];
    int len;                            /* Length of string */
#ifdef UNIX
#ifndef NOJC
    int obg;
_PROTOTYP( VOID conbgt, (int) );
#endif /* NOJC */
#endif /* UNIX */

    if (!local) return;                 /* In remote mode - don't do this */

    if (!fxd_inited)                    /* Initialize if necessary */
      fxdinit(fdispla);

#ifdef UNIX
#ifndef NOJC
    obg = backgrd;                      /* Previous background status */
    conbgt(1);                          /* See if running in background */
    if (!backgrd && obg) {              /* Just came into foreground? */
        concb((char)escape);            /* Put console back in CBREAK mode */
        setint();                       /* Restore interrupts */
    }
#endif /* NOJC */
#endif /* UNIX */

    if ((f != SCR_WM) && (f != SCR_EM)) /* Always update warnings & errors */
      if (!displa ||
          (backgrd && bgset) ||
          fdispla == XYFD_N ||
          (server && !srvdis)
          )
        return;

#ifdef VMS
    if (f == SCR_FN)                    /* VMS - shorten the name */
      s = zrelname(s,zgtdir());
#endif /* VMS */

    if (dest == DEST_S)                 /* SET DESTINATION SCREEN */
      return;                           /*  would interfere... */

#ifdef CK_CURSES
    if (fdispla == XYFD_C) {            /* If fullscreen display selected */
        screenc(f,c,n,s);               /* call the fullscreen version */
        return;
    }
#endif /* CK_CURSES */

    len = (int)strlen(s);               /* Length of string */

    switch (f) {                        /* Handle our function code */
      case SCR_FN:                      /* Filename */
        if (fdispla == XYFD_B) {
            printf(" %s %s", what == W_SEND ? "SEND" : "RECV", s);
#ifdef UNIX
            fflush(stdout);
#endif /* UNIX */
            return;
        }
#ifdef MAC
        conoll(""); conol(s); conoc(SP); hpos = len + 1;
#else
        ckstrncpy(fbuf,s,80);
        abuf[0] = a2buf[0] = NUL;
        newdpy = 1;                     /* New file so refresh display */
#endif /* MAC */
        return;

      case SCR_AN:                      /* As-name */
        if (fdispla == XYFD_B) {
#ifdef COMMENT
            printf("(as %s) ",s);
#endif /* COMMENT */
            return;
        }
#ifdef MAC
        if (hpos + len > 75) { conoll(""); hpos = 0; }
        conol("=> "); conol(s);
        if ((hpos += (len + 3)) > 78) { conoll(""); hpos = 0; }
#else
        if (abuf[0]) {
            ckstrncpy(a2buf,s,80);
        } else {
            ckstrncpy(abuf,s,80);
        }
#endif /* MAC */
        return;

      case SCR_FS:                      /* File-size */
        if (fdispla == XYFD_B) {
            printf(" (%s) (%ld byte%s)",
#ifdef NOICP
                   (binary ? "binary" : "text")
#else
                   gfmode(binary,0)
#endif /* NOICP */
                   , n, n == 1L ? "" : "s");
#ifdef UNIX
            fflush(stdout);
#endif /* UNIX */
            return;
        }
#ifdef MAC
        sprintf(buf,", Size: %ld",n);  conoll(buf);  hpos = 0;
#endif /* MAC */
        return;

      case SCR_XD:                      /* X-packet data */
        if (fdispla == XYFD_B)
          return;
#ifdef MAC
        conoll(""); conoll(s); hpos = 0;
#else
        ckstrncpy(fbuf,s,80);
        abuf[0] = a2buf[0] = NUL;
#endif /* MAC */
        return;

      case SCR_ST:                      /* File status */
        switch (c) {
          case ST_OK:                   /* Transferred OK */
            showpkt('Z');               /* Update numbers one last time */
            if (fdispla == XYFD_B) {
#ifdef GFTIMER
                printf(": OK (%0.3f sec, %ld cps)\n",fpxfsecs,
                       (long)((CKFLOAT)ffc / fpxfsecs));
#else
                printf(": OK (%d sec, %ld cps)\n",xfsecs,ffc/xfsecs);
#endif /* GFTIMER */
                return;
            }
            if ((hpos += 5) > 78) conoll(""); /* Wrap screen line. */
            conoll(" [OK]"); hpos = 0;  /* Print OK message. */
            if (fdispla == XYFD_S) {    /* We didn't show Z packet when */
                conoc('Z');             /* it came, so show it now. */
                hpos = 1;
            }
            return;

          case ST_DISC:                 /*  Discarded */
            if (fdispla == XYFD_B) {
                printf(": DISCARDED\n");
                return;
            }
            if ((hpos += 12) > 78) conoll("");
            conoll(" [discarded]"); hpos = 0;
            return;

          case ST_INT:                  /*  Interrupted */
            if (fdispla == XYFD_B) {
                printf(": INTERRUPTED\n");
                return;
            }
            if ((hpos += 14) > 78) conoll("");
            conoll(" [interrupted]"); hpos = 0;
            return;

          case ST_SKIP:                 /*  Skipped */
            if (fdispla == XYFD_B) {
                printf(": SKIPPED\n");
                return;
            } else if (fdispla == XYFD_S) {
                if (fdispla == XYFD_S && fbuf[0]) { /* CRT display */
                    conoll("");         /* New line */
                    if (what == W_SEND) conol("Sending: "); /* Action */
                    else if (what == W_RECV) conol("Receiving: ");
                    conol(fbuf);
                    if (*abuf) conol(" => "); conol(abuf); /* Names */
                    if (*a2buf) conol(" => "); conol(a2buf); /* Names */
                    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
                }
                conoll(" [skipped]");
                return;
            }
            if ((hpos += 10) > 78) conoll("");
            conol(" [skipped]"); hpos = 0;
            return;

          case ST_ERR:                  /* Error */
            if (fdispla == XYFD_B) {
                printf(": ERROR: %s\n",s);
                return;
            }
            conoll("");
            conol("Error: "); conoll(s); hpos = 0;
            return;

          case ST_MSG:                  /* Message */
            if (fdispla == XYFD_B)
              return;
            conoll("");
            conol("Message: ");
            conoll(s);
            hpos = 0;
            return;

          case ST_REFU:                 /* Refused */
            if (fdispla == XYFD_B) {
                printf(": REFUSED\n");
                return;
            } else if (fdispla == XYFD_S) {
                if (fdispla == XYFD_S && fbuf[0]) { /* CRT display */
                    conoll("");         /* New line */
                    if (what == W_SEND) conol("Sending: "); /* Action */
                    else if (what == W_RECV) conol("Receiving: ");
                    conol(fbuf);
                    if (*abuf) conol(" => "); conol(abuf);      /* Names */
                    if (*a2buf) conol(" => "); conol(a2buf);    /* Names */
                    *fbuf = NUL; *abuf = NUL; *a2buf = NUL;
                    conoll("");
                }
                conol("Refused: "); conoll(s);
                return;
            }
            conoll("");
            conol("Refused: "); conoll(s); hpos = 0;
            return;

          case ST_INC:                  /* Incomplete */
            if (fdispla == XYFD_B) {
                printf(": INCOMPLETE\n");
                return;
            }
            if ((hpos += 12) > 78) conoll("");
            conoll(" [incomplete]"); hpos = 0;
            return;

          default:
            conoll("*** screen() called with bad status ***");
            hpos = 0;
            return;
        }

#ifdef MAC
      case SCR_PN:                      /* Packet number */
        if (fdispla == XYFD_B) {
            return;
        }
        sprintf(buf,"%s: %ld",s,n);
        conol(buf); hpos += (int)strlen(buf); return;
#endif /* MAC */

      case SCR_PT:                      /* Packet type or pseudotype */
        if (fdispla == XYFD_B)
          return;
        if (c == 'Y') return;           /* Don't bother with ACKs */
        if (c == 'D') {                 /* In data transfer phase, */
            showpkt(NUL);               /* show progress. */
            return;
        }
#ifndef AMIGA
        if (hpos++ > 77) {              /* If near right margin, */
            conoll("");                 /* Start new line */
            hpos = 0;                   /* and reset counter. */
        }
#endif /* AMIGA */
        if (c == 'Z' && fdispla == XYFD_S)
          return;
        else
          conoc(c);                     /* Display the packet type. */
#ifdef AMIGA
        if (c == 'G') conoll("");       /* New line after G packets */
#endif /* AMIGA */
        return;

      case SCR_TC:                      /* Transaction complete */
        if (xfrbel) bleep(BP_NOTE);
        if (fdispla == XYFD_B) {        /* Brief display... */
            if (filcnt > 1) {
                long fx;
                fx = filcnt - filrej;
                printf(" SUMMARY: %ld file%s", fx, ((fx == 1L) ? "" : "s"));
                printf(", %ld byte%s", tfc, ((tfc == 1L) ? "" : "s"));
#ifdef GFTIMER
                printf(", %0.3f sec, %ld cps", fptsecs, tfcps);
#else
                printf(", %ld sec, %ld cps", tsecs, tfcps);
#endif /* GFTIMER */
                printf(".\n");
            }
        } else {
            conoll("");
        }
#ifdef UNIX
        fflush(stdout);
#endif /* UNIX */
        return;

      case SCR_EM:                      /* Error message */
        if (fdispla == XYFD_B) {
            printf(" ERROR: %s\n",s);
            return;
        }
        conoll(""); conoc('?'); conoll(s); hpos = 0; return;

      case SCR_WM:                      /* Warning message */
        if (fdispla == XYFD_B) {
            printf(" WARNING: %s\n",s);
            return;
        }
        conoll(""); conoll(s); hpos = 0; return;

      case SCR_TU:                      /* Undelimited text */
        if (fdispla == XYFD_B)
          return;
        if ((hpos += len) > 77) { conoll(""); hpos = len; }
        conol(s); return;

      case SCR_TN:                      /* Text delimited at beginning */
        if (fdispla == XYFD_B)
          return;
        conoll(""); conol(s); hpos = len; return;

      case SCR_TZ:                      /* Text delimited at end */
        if (fdispla == XYFD_B)
          return;
        if ((hpos += len) > 77) { conoll(""); hpos = len; }
        conoll(s); return;

      case SCR_QE:                      /* Quantity equals */
        if (fdispla == XYFD_B)
          return;
        sprintf(buf,"%s: %ld",s,n);
        conoll(buf); hpos = 0; return;

      case SCR_CW:                      /* Close fullscreen window */
        return;                         /* No window to close */

      case SCR_CD:
        return;

      default:
        conoll("*** screen() called with bad object ***");
        hpos = 0;
        return;
    }
}
#endif /* NODISPLAY */

/*  E R M S G  --  Nonfatal error message  */

/* Should be used only for printing the message text from an Error packet. */

VOID
ermsg(msg) char *msg; {                 /* Print error message */
    debug(F110,"ermsg",msg,0);
    if (local)
      xxscreen(SCR_EM,0,0L,msg);
    tlog(F110,"Protocol Error:",msg,0L);
}
#endif /* NOXFER */

VOID
doclean(fc) int fc; {                   /* General cleanup */
#ifdef OS2ORUNIX
    extern int ttyfd;
#endif /* OS2ORUNIX */
#ifndef NOICP
    if (fc > 0)
      dostop();                 /* Stop all command files and end macros */
#endif /* NOICP */

#ifndef NOXFER
    if (pktlog) {
        *pktfil = '\0';
        pktlog = 0;
        zclose(ZPFILE);
    }
#endif /* NOXFER */
    if (seslog) {
        *sesfil = '\0';
        seslog = 0;
        zclose(ZSFILE);
    }
#ifdef TLOG
    if (tralog) {
        tlog(F100,"Transaction Log Closed","",0L);
        *trafil = '\0';
        tralog = 0;
        zclose(ZTFILE);
    }
#endif /* TLOG */

#ifdef CKLOGDIAL
    debug(F100,"doclean calling dologend","",0);
    dologend();                         /* End current log record if any */
    if (dialog) {                       /* If connection log open */
        *diafil = '\0';                 /* close it. */
        dialog = 0;
        zclose(ZDIFIL);
    }
#endif /* CKLOGDIAL */

#ifndef NOICP
#ifndef NOSPL
    zclose(ZRFILE);                     /* READ and WRITE files, if any. */
    zclose(ZWFILE);
    zclose(ZIFILE);                     /* And other files too */
    zclose(ZOFILE);
    zclose(ZSYSFN);
    zclose(ZMFILE);

    if (fc < 1) {                       /* RESETing, not EXITing */
#ifdef DEBUG
        if (deblog) {                   /* Close the debug log. */
            *debfil = '\0';
            deblog = 0;
            zclose(ZDFILE);
        }
#endif /* DEBUG */
        return;
    }
#endif /* NOSPL */
#endif /* NOICP */

/*
  Put console terminal back to normal.  This is done here because the
  ON_EXIT macro calls the parser, which meddles with console terminal modes.
*/
#ifndef NOLOCAL
    if (local) {
        extern int haslock;
        if (ttchk() >= 0
#ifdef OS2
            || ttyfd != -1
#else
#ifdef UNIX
            || haslock                  /* Make sure we get lockfile! */
            || (!network && ttyfd > -1)
#endif /* UNIX */
#endif /* OS2 */
            ) {
            extern int wasclosed, whyclosed;
            if (!quiet) {
#ifdef UNIX
                fflush(stdout);
#endif /* UNIX */
                printf("Closing %s...",ttname);
            }
            ttclos(0);                  /* Close external line, if any */
            if (!quiet) {
                printf("OK\n");
#ifdef UNIX
                fflush(stdout);
#endif /* UNIX */
            }
            if (wasclosed) {
                whyclosed = WC_CLOS;
#ifndef NOSPL
                if (nmac) {             /* Any macros defined? */
                    int k;              /* Yes */
                    k = mlook(mactab,"on_close",nmac);  /* Look this up */
                    if (k >= 0) {                       /* If found, */
                        wasclosed = 0;
                        /* printf("ON_CLOSE DOCLEAN\n"); */
                        *(mactab[k].kwd) = NUL;         /* See comment below */
                        if (dodo(k,ckitoa(whyclosed),0) > -1) /* set it up, */
                          parser(1);                    /* and execute it */
                    }
                }
#endif /* NOSPL */
                wasclosed = 0;
            }
        }
        strcpy(ttname,dftty);           /* Restore default tty */
        local = dfloc;                  /* And default remote/local status */
    }
#endif /* NOLOCAL */

#ifndef NOSPL
/*
  If a macro named "on_exit" is defined, execute it.  Also remove it from the
  macro table, in case its definition includes an EXIT or QUIT command, which
  would cause much recursion and would prevent the program from ever actually
  EXITing.
*/
    if (nmac) {                         /* Any macros defined? */
        int k;                          /* Yes */
        char * cmd = "on_exit";         /* MSVC 2.x compiler error */
        k = mlook(mactab,cmd,nmac);     /* Look up "on_exit" */
        if (k >= 0) {                   /* If found, */
            *(mactab[k].kwd) = NUL;     /* poke its name from the table, */
            if (dodo(k,"",0) > -1)      /* set it up, */
              parser(1);                /* and execute it */
        }
    }
#endif /* NOSPL */

    conres();                           /* Restore console terminal. */

#ifdef COMMENT
/* Should be no need for this, and maybe it's screwing things up? */
    connoi();                           /* Turn off console interrupt traps */
#endif /* COMMENT */

    /* Delete the Startup File if we are supposed to. */
#ifndef NOICP
    {
        extern int DeleteStartupFile;
        debug(F111,"doclean DeleteStartupFile",cmdfil,DeleteStartupFile);
        if (DeleteStartupFile) {
            int rc = zdelet(cmdfil);
            debug(F111,"doclean zdelet",cmdfil,rc);
        }
    }
#endif /* NOICP */
    syscleanup();                       /* System-dependent cleanup, last */
}

/*  D O E X I T  --  Exit from the program.  */

/*
  First arg is general, system-independent symbol: GOOD_EXIT or BAD_EXIT.
  If second arg is -1, take 1st arg literally.
  If second arg is not -1, work it into the exit code.
*/
VOID
doexit(exitstat,code) int exitstat, code; {
    extern int x_logged;
#ifdef OS2
    extern int display_demo;
#endif /* OS2 */
#ifdef CK_KERBEROS
#ifdef KRB4
    extern int krb4_autodel;
#endif /* KRB4 */
#ifdef KRB5
    extern int krb5_autodel;
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef VMS
    char envstr[64];
    static $DESCRIPTOR(symnam,"CKERMIT_STATUS");
    static struct dsc$descriptor_s symval;
    int i;
#endif /* VMS */

    debug(F101,"doexit exitstat","",exitstat);
    debug(F101,"doexit code","",code);
    debug(F101,"doexit xitsta","",xitsta);

#ifdef CK_KERBEROS
    /* If we are automatically destroying Kerberos credentials on Exit */
    /* do it now. */
#ifdef KRB4
    if (krb4_autodel == KRB_DEL_EX) {
        extern struct krb_op_data krb_op;
        krb_op.version = 4;
        krb_op.cache = NULL;
        ck_krb4_destroy(&krb_op);
    }
#endif /* KRB4 */
#ifdef KRB5
    if (krb5_autodel == KRB_DEL_EX) {
        extern struct krb_op_data krb_op;
        extern char * krb5_d_cc;
        krb_op.version = 5;
        krb_op.cache = krb5_d_cc;
        ck_krb5_destroy(&krb_op);
    }
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef OS2
    /* If there is a demo screen to be displayed, display it */
    if (display_demo) {
        demoscrn();
        display_demo = 0;
    }
    DialerSend(OPT_KERMIT_EXIT,exitstat);
    debug(F100,"doexit about to msleep","",0);
    if (exitstat == BAD_EXIT)
      msleep(125);                      /* Wait for screen updates */
#endif /* OS2 */

    debug(F100,"doexit about to doclean","",0);
    doclean(1);                         /* Clean up most things */

#ifdef VMS
    if (code == -1)
      code = 0;                         /* Since we set two different items */
    sprintf(envstr,"%d", exitstat | code);
    symval.dsc$w_length = (int)strlen(envstr);
    symval.dsc$a_pointer = envstr;
    symval.dsc$b_class = DSC$K_CLASS_S;
    symval.dsc$b_dtype = DSC$K_DTYPE_T;
    i = 2;                              /* Store in global table */
#ifdef COMMENT                          /* Martin Zinser */
    LIB$SET_SYMBOL(&symnam, &symval, &i);
#else
    lib$set_symbol(&symnam, &symval, &i);
#endif /* COMMENT */
    if (exitstat == BAD_EXIT)
      exitstat = SS$_ABORT | STS$M_INHIB_MSG;
    if (exitstat == GOOD_EXIT)
      exitstat = SS$_NORMAL | STS$M_INHIB_MSG;
#else /* Not VMS */
    if (code != -1)                     /* Take 1st arg literally */
      exitstat |= code;
#endif /* VMS */

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && x_logged)
      zvlogout();
#endif /* CK_LOGIN */
#ifdef IKSDB
    debug(F101,"doexit ikdbopen","",ikdbopen);
    if (ikdbopen && dbfp) {             /* If IKSD database open */
        int x;
        x = freeslot(mydbslot);         /* Free our slot... */
        debug(F101,"doexit freeslot","",x);
        fclose(dbfp);                   /* and close it. */
    }
#endif /* IKSDB */
#endif /* IKSD */

/* We have put this off till the very last moment... */

#ifdef DEBUG
    if (deblog) {                       /* Close the debug log. */
        debug(F101,"C-Kermit EXIT status","",exitstat);
        *debfil = '\0';
        deblog = 0;
        zclose(ZDFILE);
    }
#endif /* DEBUG */

#ifdef OS2
    _exit(exitstat);            /* Exit from C-Kermit (no matter what) */
#else /* OS2 */
    exit(exitstat);                     /* Exit from C-Kermit */
#endif /* OS2 */
}

VOID
bgchk() {                               /* Check background status */
    if (bgset < 0) {                    /* They didn't type SET BACKGROUND */
#ifdef VMS                              /* Set prompt flag based on */
        pflag = !batch;                 /* what we detected at startup. */
#else
        pflag = !backgrd;
#endif /* VMS */
    } else {                            /* Otherwise SET BACKGROUND value */
        pflag = (bgset == 0 ? 1 : 0);
    }

#ifndef NOICP
    /* Message flag on only if at top level, pflag is on, and QUIET is OFF */
    if (!cmdsrc())
      msgflg = (pflag == 0) ? 0 : !quiet;
    else msgflg = 0;
#else
    msgflg = 0;
#endif /* NOICP */
}

/* Set console interrupts */

VOID
setint() {                              /* According to SET COMMAND INTERRUP */
    int x = 0;
    if (cmdint)  x |= 1;
    if (suspend) x |= 2;
    debug(F101,"setint","",x);

    switch (x) {                        /* Set the desired combination */
      case 0: connoi(); break;          /* No interrupts */
      case 1: conint(trap,SIG_IGN); break;
      case 2: conint(SIG_IGN,stptrap); break;
      case 3: conint(trap,stptrap); break;
    }
    bgchk();                            /* Check background status */
}

#ifdef DEBUG
/*  D E B U G  --  Enter a record in the debugging log  */

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
        If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Long int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.

   f=F011 (3) is also special; in this case s2 is interpeted as a counted
   string that might contain NULs.  n is the length.  If n is negative, this
   means the string has been truncated and ".." should be printed after the
   first n bytes.  NUL and LF bytes are printed as "<NUL>" and "<LF>".
*/
/*
  WARNING: Don't change DEBUFL without changing sprintf() formats below,
  accordingly.
*/
#define DBUFL 2300
/*
  WARNING: This routine is not thread-safe, especially when Kermit is
  executing on multiple CPUs -- as different threads write to the same
  static buffer, the debug statements are all interleaved.  To be fixed
  later...
*/
static char *dbptr = (char *)0;

int
#ifdef CK_ANSIC
dodebug(int f, char *s1, char *s2, long n)
#else
dodebug(f,s1,s2,n) int f; char *s1, *s2; long n;
#endif /* CK_ANSIC */
/* dodebug */ {
    char *sp;
    int len1, len2;
    extern int debtim;

    if (!deblog)                        /* If no debug log, don't. */
      return(0);
/*
  This prevents infinite recursion in case we accidentally put a debug()
  call in this routine, or call another routine that contains debug() calls.
  From this point on, all returns from this return must be via goto xdebug,
  which sets deblog back to 1.
*/
#ifndef OS2
    deblog = 0;                         /* Prevent infinite recursion */
#endif /* OS2 */

    if (!dbptr) {                       /* Allocate memory buffer */
        dbptr = malloc(DBUFL+1);        /* This only happens once */
        if (!dbptr) {
            zclose(ZDFILE);
            return(0);
        }
    }
    if (debtim) {                       /* Timestamp */
        char *tb, tsbuf[48];
        ztime(&tb);
        ckstrncpy(tsbuf,tb,32);
        tsbuf[20] = NUL;
        if (ztmsec > -1L)
          sprintf(tsbuf+19,".%03ld ",ztmsec);
        zsout(ZDFILE,tsbuf+11);
    }
    if (!s1) s1="(NULL)";
    if (!s2) s2="(NULL)";

    len1 = strlen(s1);
    len2 = strlen(s2);

#ifdef COMMENT
/*
  This should work, but it doesn't.
  So instead we'll cope with overflow via sprintf formats.
  N.B.: UNFORTUNATELY, this means we have to put constants in the
  sprintf formats.
*/
    if (f != F011 && (!f || (f & 6))) { /* String argument(s) included? */
        x = (int) strlen(s1) + (int) strlen(s2) + 18;
        if (x > dbufl) {                /* Longer than buffer? */
            if (dbptr)                  /* Yes, free previous buffer */
              free(dbptr);
            dbptr = (char *) malloc(x + 2); /* Allocate a new one */
            if (!dbptr) {
                zsoutl(ZDFILE,"DEBUG: Memory allocation failure");
                deblog = 0;
                zclose(ZDFILE);
                goto xdebug;
            } else {
                dbufl = x;
                sprintf(dbptr,"DEBUG: Buffer expanded to %d\n", x + 18);
                zsoutl(ZDFILE,dbptr);
            }
        }
    }
#endif /* COMMENT */

#ifdef COMMENT
/* The aforementioned sprintf() formats were like this: */
        if (n > 31 && n < 127)
          sprintf(sp,"%.100s%.2000s:%c\n",s1,s2,(CHAR) n);
        else if (n < 32 || n == 127)
          sprintf(sp,"%.100s%.2000s:^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
        else if (n > 127 && n < 160)
          sprintf(sp,"%.100s%.2000s:~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
        else if (n > 159 && n < 256)
          sprintf(sp,"%.100s%.2000s:~%c\n",s1,s2,(CHAR) (n & 0x7F));
        else sprintf(sp,"%.100s%.2000s:%ld\n",s1,s2,n);
/*
  But, naturally, it turns out these are not portable either, so now
  we do the stupidest possible thing.
*/
#endif /* COMMENT */

#ifdef BIGBUFOK
/* Need to accept longer strings when debugging authenticated connections */
    if (f == F010) {
        if (len2 + 2 >= DBUFL) s2 = "(string too long)";
    } else if (f != F011 && f != F100) {
        if (len1 > 100) s1 = "(string too long)";
        if (len2 + 101 >= DBUFL) s2 = "(string too long)";
    }
#else
    if (f != F011) {
        if (len1 > 100) s1 = "(string too long)";
        if (len2 + 101 >= DBUFL) s2 = "(string too long)";
    }
#endif /* BIGBUFOK */

    sp = dbptr;

    switch (f) {                /* Write log record according to format. */
      case F000:                /* 0 = print both strings, and n as a char. */
        if (len2 > 0) {
            if ((n > 31 && n < 127) || (n > 159 && n < 256))
              sprintf(sp,"%s[%s]=%c\n",s1,s2,(CHAR) n);
            else if (n < 32 || n == 127)
              sprintf(sp,"%s[%s]=^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
            else if (n > 127 && n < 160)
              sprintf(sp,"%s[%s]=~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
            else sprintf(sp,"%s[%s]=0x%lX\n",s1,s2,n);
        } else {
            if ((n > 31 && n < 127) || (n > 159 && n < 256))
              sprintf(sp,"%s=%c\n",s1,(CHAR) n);
            else if (n < 32 || n == 127)
              sprintf(sp,"%s=^%c\n",s1,(CHAR) ((n+64) & 0x7F));
            else if (n > 127 && n < 160)
              sprintf(sp,"%s=~^%c\n",s1,(CHAR)((n-64) & 0x7F));
            else sprintf(sp,"%s=0x%lX\n",s1,n);
        }
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F001:                        /* 1, "=n" */
#ifdef COMMENT
        /* This was never used */
        sprintf(sp,"=%ld\n",n);
#else
        /* Like F111, but shows number n in hex */
        sprintf(sp,"%s%s=0x%lX\n",s1,s2,n);
#endif /* COMMENT */
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F010:                        /* 2, "[s2]" */
        sprintf(sp,"[%s]\n",s2);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
/*
  This one treats n as the length of the string s2, which may contain NULs.
  It's good for logging NUL-bearing packets and other data in the debug log.
*/
      case F011:                        /* 3, "[s2]=n" */
#ifdef COMMENT                          /* This format was never used */
        sprintf(sp,"[%s]=%ld\n",s2,n);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#else
#ifdef DEBUG
        {
            int i, j, contd = 0;
            char * p = s2, *pbuf = NULL;
            int m;

            if (n < 0) {                /* n = size of source */
                n = 0 - n;
                contd = 1;
            }
            if (n == 0)
              goto xdebug;

            m = n + 32;                 /* m = size of dest */

            pbuf = (char *) malloc(m+1);
            if (!pbuf)
              goto xdebug;
            i = 0;
            pbuf[i++] = '[';
            for (; i <= n; p++) {
                if (*p == LF) {
                    if (i >= m-4)
                      break;
                    pbuf[i++] = '<';
                    pbuf[i++] = 'L';
                    pbuf[i++] = 'F';
                    pbuf[i++] = '>';
                    continue;
                } else if (*p) {
                    pbuf[i++] = *p;
                    continue;
                } else {
                    if (i >= m-5)
                      break;
                    pbuf[i++] = '<';
                    pbuf[i++] = 'N';
                    pbuf[i++] = 'U';
                    pbuf[i++] = 'L';
                    pbuf[i++] = '>';
                    continue;
                }
            }
            if (i < m-2 || contd) {
                pbuf[i++] = '.';
                pbuf[i++] = '.';
            }
            pbuf[i++] = ']';
#ifdef COMMENT
            pbuf[i] = NUL;
#else
            sprintf(pbuf+i,"=%ld",n);
#endif /*  */
            if (!s1) s1 = "";
            if (*s1) {
                if (zsout(ZDFILE,s1) < 0) {
                    deblog = 0;
                    zclose(ZDFILE);
                }
            }
            if (zsoutl(ZDFILE,pbuf) < 0) {
                deblog = 0;
                zclose(ZDFILE);
            }
#ifdef CKSYSLOG
            if (ckxsyslog >= SYSLG_DB && ckxlogging) {
                cksyslog(SYSLG_DB,1,"debug",s1,pbuf);
            }
#endif /* CKSYSLOG */
            free(pbuf);
        }
#endif /* DEBUG */
#endif /* COMMENT */
        break;
      case F100:                        /* 4, "s1" */
        if (zsoutl(ZDFILE,s1) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",s1,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F101:                        /* 5, "s1=n" */
        sprintf(sp,"%s=%ld\n",s1,n);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F110:                        /* 6, "s1[s2]" */
        sprintf(sp,"%s[%s]\n",s1,s2);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      case F111:                        /* 7, "s1[s2]=n" */
        sprintf(sp,"%s[%s]=%ld\n",s1,s2,n);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
      default:
        sprintf(sp,"\n?Invalid format for debug() - %d\n",f);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
        }
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_DB && ckxlogging) {
            cksyslog(SYSLG_DB,1,"debug",dbptr,NULL);
        }
#endif /* CKSYSLOG */
        break;
    }
  xdebug:                               /* Common exit point */
#ifndef OS2
    deblog = 1;                         /* Restore this */
#endif /* OS2 */
    return(0);
}

VOID
#ifdef CK_ANSIC
dohexdump(CHAR *msg, CHAR *st, int cnt)
#else
dohexdump(msg,st,cnt) CHAR *msg; CHAR *st; int cnt;
#endif /* CK_ANSIC */
/* dohexdump */ {
    int i = 0, j = 0;
    char tmp[8];

    if (!deblog) return;                /* If no debug log, don't. */
    if (!dbptr) {                       /* Allocate memory buffer */
        dbptr = malloc(DBUFL+1);        /* This only happens once */
        if (!dbptr) {
            deblog = 0;
            zclose(ZDFILE);
            return;
        }
    }
    if (msg != NULL) {
        sprintf(dbptr,"HEXDUMP: %s (%d bytes)\n",msg,cnt);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
            return;
        }
    } else {
        sprintf(dbptr,"HEXDUMP: (%d bytes)\n",cnt);
        zsout(ZDFILE,dbptr);
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
            return;
        }
    }
    for (i = 0; i < cnt; i++) {
        dbptr[0] = '\0';
        for(j = 0 ; (j < 16) && ((i + j) < cnt) ; j++) {
            sprintf(tmp,
                    "%s%02x%s",
                    (j == 8 ? "| " : ""),
                    (CHAR) st[i + j],
                    (j < 16 && i < cnt ? "  " : "")
                    );
            strcat(dbptr,tmp);
        }
        strcat(dbptr,"\n");
        i += j - 1;
        if (zsout(ZDFILE,dbptr) < 0) {
            deblog = 0;
            zclose(ZDFILE);
            return;
        }
    } /* end for */
}
#endif /* DEBUG */

VOID
#ifdef OS2
logchar(unsigned short c)
#else /* OS2 */
#ifdef CK_ANSIC
logchar(char c)
#else
logchar(c) char c;
#endif /* CK_ANSIC */
#endif /* OS2 */
/* logchar */ {                         /* Log character c to session log */
#ifndef NOLOCAL
    if (seslog)
      if ((sessft != XYFT_T) ||
          (c != '\r' &&
           c != '\0' &&
           c != XON &&
           c != XOFF))
        if (zchout(ZSFILE,(CHAR)(c & 0xFF)) < 0) {
            conoll("");
            conoll("ERROR WRITING SESSION LOG, LOG CLOSED!");
            seslog = 0;
            zclose(ZSFILE);
        }
#endif /* NOLOCAL */
}

VOID
logstr(s, len) char * s; int len; {     /* Log string to session log */
#ifndef NOLOCAL
    char c;
    int n = 0;
    if (!seslog || !s)
      return;
    while (n < len) {
        c = s[n];
        n++;
        if ((sessft != XYFT_T) ||
            (c != '\r' &&
             c != '\0' &&
             c != XON &&
             c != XOFF))
          if (zchout(ZSFILE,c) < 0) {
              conoll("");
              conoll("ERROR WRITING SESSION LOG, LOG CLOSED!");
              seslog = 0;
              zclose(ZSFILE);
              return;
          }
    }
#endif /* NOLOCAL */
}

#ifdef CK_CURSES
#ifdef STRATUS
/* VOS has curses but no tgetent() */
int
tgetent(s1, s2) char * s1, * s2; {
    return(1);
}
#endif /* STRATUS */

#ifdef VMS
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */

/*
  There are three different ways to do fullscreen on VMS.
  1. Use the real curses library, VAXCCURSE.
  2. Use do-it-yourself code.
  3. Use the Screen Manager, SMG$.

  Method 1 doesn't work quite right; you can't call endwin(), so once you've
  started curses mode, you can never leave.

  Method 2 doesn't optimize the screen, and so much more time is spent in
  screen writes.  This actually causes file transfers to fail because the
  tty device input buffer can be overrun while the screen is being updated,
  especially on a slow MicroVAX that has small typeahead buffers.

  In the following #ifdef block, #define one of them and #undef the other 2.

  So now let's try method 3...
*/
#ifdef VMS
#define CK_SMG                          /* Screen Manager */
#undef MYCURSES                         /* Do-it-yourself */
#undef VMSCURSE                         /* VAXCCURSE library */
#endif /* VMS */
/*
  But just before New Years, 2000, the SMG library seemed to break on
  both VMS systems we have here (an Alpha with VMS 7.1 and a VAX with 5.5).
  So back to MYCURSES, which works fine.
*/
#ifdef VMS
#undef CK_SMG
#define MYCURSES
#endif /* VMS */

#ifdef MYCURSES
#define stdscr 0
#ifdef CK_WREFRESH
#undef CK_WREFRESH
#endif /* CK_WREFRESH */
#endif /* MYCURSES */

/*  S C R E E N C  --  Screen display function, uses curses  */

/* Idea for curses display contributed by Chris Pratt of APV Baker, UK */

/* Avoid conficts with curses.h */

#ifdef QNX
/* Same as ckcasc.h, but in a different radix... */
#ifdef ESC
#undef ESC
#endif /* ESC */
#endif /* QNX */

#ifndef MYCURSES
#undef VOID                             /* This was defined in ckcdeb.h */
#endif /* MYCURSES */

#undef BS                               /* These were defined in ckcasc.h */
#undef CR
#undef NL
#undef SO
#undef US
#undef SP                               /* Used in ncurses */
#define CHR_SP 32                       /* Use this instead */

#ifdef VMS                              /* VMS fullscreen display */
#ifdef MYCURSES                         /* Do-it-yourself method */
extern int isvt52;                      /* From CKVTIO.C */
#define printw printf
#else
#ifdef VMSCURSE                         /* VMS curses library VAXCCURSE */
#include <curses.h>
/* Note: Screen manager doesn't need a header file */
#endif /* VMSCURSE */
#endif /* MYCURSES */
#else                                   /* Not VMS */
#ifdef MYCURSES                         /* Do-it-yourself method */
#define isvt52 0                        /* Used by OS/2, VT-100/ANSI always */
#ifdef CKXPRINTF
#define printw ckxprintf
#else /* CKXPRINTF */
#define printw printf
#endif /* CKXPRINTF */
#else                                   /* Use real curses */
#ifdef CK_NCURSES                       /* or ncurses... */
#ifdef CKXPRINTF                        /* Our printf macro conflicts with */
#undef printf                           /* use of "printf" in ncurses.h */
#endif /* CKXPRINTF */
#include <ncurses.h>
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#else  /* Not ncurses */
#ifdef CKXPRINTF                        /* Our printf macro conflicts with */
#undef printf                           /* use of "printf" in curses.h */
#endif /* CKXPRINTF */
#include <curses.h>
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */
#endif /* CK_NCURSES */
#endif /* MYCURSES */
#endif /* VMS */

#endif /* CK_CURSES */

/*  F X D I N I T  --  File Xfer Display Initialization  */

#ifdef CK_CURSES
#ifndef MYCURSES
#ifndef CK_SMG
static
#ifdef CK_ANSIC
/* Can't use VOID because of curses.h */
void
ck_termset(int);
#else
ck_termset();
#endif /* CK_ANSIC */
#endif /* CK_SMG */
#endif /* MYCURSES */
#endif /* CK_CURSES */

#ifdef NOTERMCAP
static int notermcap = 1;
#else
static int notermcap = 0;
#endif /* NOTERMCAP */

#ifndef NODISPLAY
#ifdef OSK
VOID
#else
#ifdef CK_ANSIC
void
#endif /* CKANSIC */
#endif /* OSK */
fxdinit(xdispla) int xdispla; {
#ifndef COHERENT
#ifndef OS2
#ifndef STRATUS
    char *s;
    int x, dummy;

    debug(F101,"fxdinit xdispla","",xdispla);
    debug(F101,"fxdinit fxd_inited","",fxd_inited);

#ifdef IKSD
#ifndef NOXFER
    /* No curses for IKSD */
    if (inserver) {
        fdispla = XYFD_N;
        return;
    }
    if (fxd_inited)                     /* Only do this once */
      return;
#endif /* NOXFER */
#endif /* IKSD */

#ifdef CK_CURSES
#ifdef VMS
    /* Force BRIEF in Batch logs */
    if (batch && (xdispla == XYFD_C || xdispla == XYFD_S))
      xdispla = XYFD_B;
#else
    if (xdispla == XYFD_C || xdispla == 9999) {

#ifdef DYNAMIC
        if (!trmbuf) {
/*
  Allocate tgetent() buffer.  Make it big -- some termcaps can be huge;
  tgetent() merrily writes past the end of the buffer, causing core dumps
  or worse.
*/
            trmbuf = (char *)malloc(TRMBUFL);
            if (!trmbuf) {
                notermcap = 1;
                debug(F101,"fxdinit malloc trmbuf","FAILED",TRMBUFL);
                fdispla = XYFD_S;
                return;
            }
            debug(F111,"fxdinit malloc trmbuf","OK",TRMBUFL);
            debug(F001,"fxdinit trmbuf","",trmbuf);
#ifdef COMMENT
            memset(trmbuf,'\0',(size_t)TRMBUFL);
            debug(F100,"fxdinit memset OK","",0);
#endif /* COMMENT */
        }
#endif /* DYNAMIC */

        debug(F100,"fxdinit before getenv(TERM)","",0);
        s = getenv("TERM");
        debug(F110,"fxdinit after getenv(TERM)",s,0);
        if (!s) s = "";
        if (*s) {
            debug(F110,"fxdinit before tgetent()",s,0);
            x = tgetent(trmbuf,s);
            debug(F111,"fxdinit tgetent",s,x);
        } else {
            x = 0;
            notermcap = 1;
            debug(F110,"fxdinit TERM null - no tgetent",s,0);
        }
        if (x < 1 && !quiet && !backgrd
#ifdef VMS
            && !batch
#endif /* VMS */
            ) {
            printf("Warning: terminal type unknown: \"%s\"\n",s);
            printf("SCREEN command will use ANSI sequences.\n");
            if (local)
              printf("Fullscreen file transfer display disabled.\n");
            fdispla = XYFD_S;
        }
#ifndef MYCURSES
#ifndef CK_SMG
        ck_termset(x);
#endif /* CK_SMG */
#endif /* MYCURSES */
        fxd_inited = 1;
    }
#endif /* CK_CURSES */
#endif /* VMS */
#endif /* STRATUS */
#endif /* OS2 */
#endif /* COHERENT */
}
#endif /* NODISPLAY */

#ifdef CK_CURSES
#ifdef CK_SMG
/*
  Long section for Screen Manager starts here...
  By William Bader.
*/
#include "ckvvms.h"
#ifdef OLD_VMS
#include <smgdef.h>                     /* use this on VAX C 2.4 */
/* #include <smgmsg.h> */
#else
#include <smg$routines.h>               /* Martin Zinser */
#endif /* OLD_VMS */

extern unsigned int vms_status;     /* Used for system service return status */

static long smg_pasteboard_id = -1;     /* pasteboard identifier */
static long smg_display_id = -1;        /* display identifier */
static int smg_open = 0;                /* flag if smg current open */
static int smg_inited = 0;              /* flag if smg initialized */

#ifdef COMMENT
#define clrtoeol()      SMG$ERASE_LINE(&smg_display_id, 0, 0)

#define clear()         SMG$ERASE_DISPLAY(&smg_display_id, 0, 0, 0, 0)

#define touchwin(scr)   SMG$REPAINT_SCREEN(&smg_pasteboard_id)

#else  /* Not COMMENT */

#define clrtoeol()      smg$erase_line(&smg_display_id, 0, 0)

#define clear()         smg$erase_display(&smg_display_id, 0, 0, 0, 0)

#define touchwin(scr)   smg$repaint_screen(&smg_pasteboard_id)
#endif /* COMMENT */

#define clearok(curscr,ok)              /* Let wrefresh() do the work */

#define wrefresh(cursrc) touchwin(scr)

static void
move(row, col) int row, col; {
    /* Change from 0-based for curses to 1-based for SMG */
    if (!smg_open)
      return;
    ++row; ++col;
    debug(F111,"VMS smg move",ckitoa(row),col);
#ifdef COMMENT                          /* Martin Zinser */
    CHECK_ERR("move: smg$set_cursor_abs",
              SMG$SET_CURSOR_ABS(&smg_display_id, &row, &col));
#else
    CHECK_ERR("move: smg$set_cursor_abs",
              smg$set_cursor_abs(&smg_display_id, &row, &col));
#endif /* COMMENT */
    debug(F101,"VMS smg move vms_status","",vms_status);
}

#ifdef VMS_V40
#define OLD_VMS
#endif /* VMS_V40 */
#ifdef VMS_V42
#define OLD_VMS
#endif /* VMS_V42 */
#ifdef VMS_V44
#define OLD_VMS
#endif /* VMS_V44 */

static int
initscr() {
    int rows = 24, cols = 80;
    int row = 1, col = 1;

    debug(F101,"VMS initscr smg_pasteboard_id A","",smg_pasteboard_id);

    if (smg_pasteboard_id == -1) { /* Open the screen */
#ifdef OLD_VMS                     /* Note: Routine calls lowercased 9/96 */
        CHECK_ERR("initscr: smg$create_pasteboard",
                  smg$create_pasteboard(&smg_pasteboard_id, 0, 0, 0, 0));
#else
        /* For VMS V5, not tested */
        CHECK_ERR("initscr: smg$create_pasteboard",
                  smg$create_pasteboard(&smg_pasteboard_id, 0, 0, 0, 0, 0));
#endif /* OLD_VMS */
    }
    debug(F101,"VMS initscr smg_pasteboard_id B","",smg_pasteboard_id);
    if (smg_pasteboard_id == -1) {
	printf("?Error initializing fullscreen display\n");
	fdispla = XYFD_S;
	dpyinit();
	return(0);
    }
    debug(F101,"VMS initscr smg_display_id","",smg_display_id);
    if (smg_display_id == -1) {         /* Create a display window */

#ifdef COMMENT                          /* Martin Zinser */
        CHECK_ERR("initscr: smg$create_virtual_display",
                  SMG$CREATE_VIRTUAL_DISPLAY(&rows, &cols, &smg_display_id,
                                             0, 0, 0));

        /* Connect the display window to the screen */
        CHECK_ERR("initscr: smg$paste_virtual_display",
                  SMG$PASTE_VIRTUAL_DISPLAY(&smg_display_id,&smg_pasteboard_id,
                                            &row,&col));
#else
        CHECK_ERR("initscr: smg$create_virtual_display",
                  smg$create_virtual_display(&rows, &cols, &smg_display_id,
                                             0, 0, 0));

        /* Connect the display window to the screen */
        CHECK_ERR("initscr: smg$paste_virtual_display",
                  smg$paste_virtual_display(&smg_display_id,&smg_pasteboard_id,
                                            &row,&col));
#endif /* COMMENT */
    }
    debug(F101,"VMS initscr smg_open A","",smg_open);
    if (!smg_open) {                    /* Start a batch update */
        smg_open = 1;
#ifdef COMMENT
        CHECK_ERR("initscr: smg$begin_pasteboard_update",
                  SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
        CHECK_ERR("initscr: smg$begin_pasteboard_update",
                  smg$begin_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */
	debug(F101,"VMS initscr smg$begin_pasteboard_update","",vms_status);
    }
    debug(F101,"VMS initscr smg_open B","",smg_open);
    smg_inited = 1;
    return(1);
}

static void
refresh() {
    debug(F101,"refresh smg_pasteboard_id","",smg_pasteboard_id);

    if (smg_open == 0 || smg_pasteboard_id == -1)
      return;

#ifdef COMMENT                          /* Martin Zinser */
    CHECK_ERR("refresh: smg$end_pasteboard_update",
              SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));
    CHECK_ERR("refresh: smg$begin_pasteboard_update",
              SMG$BEGIN_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
    CHECK_ERR("refresh: smg$end_pasteboard_update",
              smg$end_pasteboard_update(&smg_pasteboard_id));
    CHECK_ERR("refresh: smg$begin_pasteboard_update",
              smg$begin_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */
}

static void
endwin() {
    if (!smg_open)
      return;

    smg_open = 0;

#ifdef COMMENT
    CHECK_ERR("endwin: smg$end_pasteboard_update",
              SMG$END_PASTEBOARD_UPDATE(&smg_pasteboard_id));
#else
    CHECK_ERR("endwin: smg$end_pasteboard_update",
              smg$end_pasteboard_update(&smg_pasteboard_id));
#endif /* COMMENT */

    move(22, 0);

#ifdef COMMENT
/*
  These calls clear the screen.
  (convert routine calls to lowercase - Martin Zinser)
*/
    CHECK_ERR("endwin: smg$delete_virtual_display",
              SMG$DELETE_VIRTUAL_DISPLAY(&smg_display_id));
    smg_display_id = -1;

    CHECK_ERR("endwin: smg$delete_pasteboard",
              SMG$DELETE_PASTEBOARD(&smg_pasteboard_id, 0));
    smg_pasteboard_id = -1;
#endif /* COMMENT */
}

#ifdef COMMENT
/* DECC 6.2 screams bloody murder about printw ("not enough args") */
/* but adding the following prototype only makes it holler louder. */
#ifdef __DECC
/* "varargs" prototype for printw */
_PROTOTYP(static int printw,(char *, ...));
#endif /* __DECC */
#endif /* COMMENT */

#ifdef __DECC
#include <stdarg.h>
_PROTOTYP(static void printw,(char *, ...));
static void
printw(char *str,...) {
    char buf[255];
    va_list ap;
    $DESCRIPTOR(text_dsc, 0);
    text_dsc.dsc$a_pointer=buf;
    if (!smg_open)
      return;
    va_start(ap,str);
    text_dsc.dsc$w_length = vsprintf(buf, str, ap);
    va_end(ap);
    CHECK_ERR("printw: smg$put_chars",
              smg$put_chars(&smg_display_id, &text_dsc, 0, 0, 0, 0, 0));
}
#else
static void
printw(str, a1, a2, a3, a4, a5, a6, a7, a8)
    char *str;
    long a1, a2, a3, a4, a5, a6, a7, a8;
/* printw */ {
    char buf[255];
    $DESCRIPTOR(text_dsc, 0);
    if (!smg_open)
      return;
    text_dsc.dsc$a_pointer=buf;
    text_dsc.dsc$w_length = sprintf(buf, str, a1, a2, a3, a4, a5, a6, a7, a8);
    CHECK_ERR("printw: smg$put_chars",
              smg$put_chars(&smg_display_id, &text_dsc, 0, 0, 0, 0, 0));
}
#endif /* __DECC */

#define CK_CURPOS
int
ck_curpos(row, col) {
    debug(F111,"VMS smg ck_curpos",ckitoa(row),col);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg curpos smg_open","",smg_open);
    if (!smg_open)
      return(0);
    debug(F111,"VMS smg ck_curpos",ckitoa(row-1),col-1);
    move(row - 1, col - 1);             /* SMG is 0-based */
    refresh();
    /* endwin(); */
    return(0);
}

int
ck_cls() {
    debug(F101,"VMS smg ck_cls smg_inited","",smg_inited);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg ck_cls smg_open","",smg_open);
    if (!smg_open)
      return(0);
    clear();
    refresh();
    /* endwin(); */
    return(0);
}

int
ck_cleol() {
    debug(F101,"VMS smg ck_cleol smg_inited","",smg_inited);
    if (!smg_inited || !smg_open) {
        initscr();
    }
    debug(F101,"VMS smg ck_cleol smg_open","",smg_open);
    if (!smg_open)
      return(0);
    clrtoeol();
    refresh();
    /* endwin(); */
    return(0);
}
#endif /* CK_SMG */

#ifdef MYCURSES
/*
  Do-it-yourself curses implementation for VMS, OS/2 and other ANSI/VT-100's.
  Supports only the VT52 and VT1xx (and later VT2xx/3xx/4xx) terminals.
  By Terry Kennedy, St Peters College.

  First, some stuff we can just ignore:
*/

static int
touchwin(x) int x; {
    return(0);
}
static int
initscr() {
    return(0);
}
static int
refresh() {
    return(0);
}
static int
endwin() {
    return(0);
}

/*
 * Now, some stuff we need to do:
 */

_PROTOTYP( int move, (int, int) );
#ifndef OS2
int
move(row, col) int row, col; {
    if (isvt52)
      printf("\033Y%c%c", row + 037, col + 037);
    else
      printf("\033[%d;%dH", row + 1, col + 1);
    return(0);
}

int
clear() {
    move(0,0);
    if (isvt52)
      printf("\033J");
    else
      printf("\033[J");
    return(0);
}

int
clrtoeol() {
    if (isvt52)
      printf("\033K");
    else
      printf("\033[K");
    return(0);
}

#define CK_CURPOS
int
ck_cls() {
    return(clear());
}

int
ck_cleol() {
    return(clrtoeol());
}

int
ck_curpos(row, col) int row, col; {
    move(row, col);
    return(0);
}

#else /* OS2 */
/* Windows NT and Windows 95 do not provide ANSI emulation */
/* Therefore we might as well not use it for OS/2 either   */

int
move(row, col) int row, col; {
#ifndef ONETERMUPD
    SetCurPos(row, col);
#endif /* ONETERMUPD */
    lgotoxy( VCMD, col+1, row+1);
    VscrnIsDirty(VCMD);
    return(0);
}

int
clear() {
    viocell cell;
    move(0,0);
#ifdef ONETERMUPD
    if (VscrnGetBufferSize(VCMD) > 0) {
        VscrnScroll(VCMD, UPWARD, 0,
                    VscrnGetHeight(VCMD)-(1),
                    VscrnGetHeight(VCMD)-(0), TRUE, CHR_SP);
        cleartermscreen(VCMD);
    }
#else
    cell.c = ' ';
    cell.a = colorcmd;
    WrtNCell(cell, cmd_rows * cmd_cols, 0, 0);
#endif /* ONETERMUPD */
    return(0);
}

int
clrtoeol() {
    USHORT row, col;
    viocell cell;

    cell.c = ' ';
    cell.a = colorcmd;
#ifndef ONETERMUPD
    GetCurPos(&row, &col );
    WrtNCell(cell, cmd_cols - col -1, row, col);
#endif /* ONETERMUPD */
    clrtoeoln(VCMD,CHR_SP);
    return(0);
}

#define CK_CURPOS
int
ck_curpos(row, col) int row, col; {
    move(row, col);
    return(0);
}

int
ck_cls() {
    return(clear());
}

int
ck_cleol() {
    return(clrtoeol());
}

#endif /* OS2 */
#endif /* MYCURSES */

#ifndef NOTERMCAP
#ifndef CK_CURPOS
#define CK_CURPOS

/* Termcap/Terminfo section */

static char cur_cls[32] = { NUL, NUL };
static char cur_cleol[32] = { NUL, NUL };
static char cur_cm[64] = { NUL, NUL };
static char tgsbuf[128] = { NUL, NUL };

static
#ifdef CK_ANSIC
void
#endif /* CK_ANSIC */
ck_termset(x) int x; {
    cur_cls[0] = NUL;
    cur_cleol[0] = NUL;
    cur_cm[0] = NUL;
#ifdef tgetent
    debug(F100,"tgetent is a macro","",0);
#endif /* tgetent */
#ifdef tgetstr
    debug(F100,"tgetstr is a macro","",0);
#endif /* tgetstr */
#ifdef tputs
    debug(F100,"tputs is a macro","",0);
#endif /* tputs */
#ifdef tgoto
    debug(F100,"tgoto is a macro","",0);
#endif /* tgoto */
#ifdef NOTERMCAP
    /* tgetstr() gets a segmentation fault on OSF/1 */
    debug(F100,"ck_termset NOTERMCAP","",0);
#else
    if (notermcap) {
        debug(F100,"ck_termset notermcap","",0);
        return;
    }
    debug(F101,"ck_termset x","",x);
    if (x > 0) {
        char * bp;
        bp = tgsbuf;
        *bp = NUL;
        debug(F110,"ck_termset calling tgetstr","cl",0);
        if (tgetstr("cl", &bp)) {       /* Get clear-screen code */
            debug(F110,"ck_termset tgetstr cl",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 32)
              strcpy(cur_cls,tgsbuf);
        } else
          return;
        bp = tgsbuf;
        if (tgetstr("ce", &bp)) {       /* Get clear-to-end-of-line code */
            debug(F110,"ck_termset tgetstr ce",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 32)
              strcpy(cur_cleol,tgsbuf);
        } else
          return;
        bp = tgsbuf;
        if (tgetstr("cm", &bp)) {       /* Get cursor-movement code */
            debug(F110,"ck_termset tgetstr cm",tgsbuf,"");
            if ((int)strlen(tgsbuf) < 64)
              strcpy(cur_cm,tgsbuf);
        } else
          return;
    }
#endif /* NOTERMCAP */
}

#ifndef TPUTSFNTYPE
#ifdef TPUTSISVOID
#define TPUTSFNTYPE void
#else
#define TPUTSFNTYPE int
#endif /* TPUTSISVOID */
#endif /* TPUTSFNTYPE */

#ifndef TPUTSARGTYPE
#ifdef HPUX9
#define TPUTSARGTYPE char
#else
#ifdef HPUX10
#define TPUTSARGTYPE char
#else
#define TPUTSARGTYPE int
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* TPUTSARGTYPE */

static TPUTSFNTYPE
#ifdef CK_ANSIC
ck_outc(TPUTSARGTYPE x)
#else
ck_outc(x) TPUTSARGTYPE x;
#endif /* CK_ANSIC */
{                                       /* To satisfy tputs() arg3 prototype */
    int rc;
    char c;
    c = (char) x;
    rc = (inserver) ? ttoc(c) : conoc(c);
#ifndef TPUTSISVOID
    return(rc);
#endif /* TPUTSISVOID */
}

int
ck_curpos(row, col) {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cm[0]) {                   /* We don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        /* Both C-Kermit's SCREEN command and ANSI/VT100 are 1-based */
        printf("\033[%d;%dH", row, col); /* Or default to ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        /* termcap/terminfo is 0-based */
        tputs(
#ifdef TPUTSARG1CONST
              (const char *)
#endif /* TPUTSARG1CONST */
              tgoto(cur_cm,col-1,row-1),1,fn);
    }
    return(0);
}

int
ck_cls() {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cls[0]) {                  /* If we don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        printf("\033[;H\033[2J");       /* Or default to ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        debug(F111,"ck_cls 2",cur_cls,fxd_inited);
        tputs(cur_cls,cmd_rows,fn);
    }
    return(0);
}

int
ck_cleol() {
#ifdef CK_ANSIC
    TPUTSFNTYPE (*fn)(TPUTSARGTYPE);
#else
    TPUTSFNTYPE (*fn)();
#endif /* CK_ANSIC */
    if (!fxd_inited)
      fxdinit(9999);
    if (!cur_cleol[0]) {                /* If we don't have escape sequences */
#ifdef COMMENT
        return(-1);                     /* Do nothing */
#else
        printf("\033[K");               /* Or use ANSI */
#endif /* COMMENT */
    } else {
        fn = ck_outc;
        tputs(cur_cleol,1,fn);
    }
    return(0);
}
#endif /* CK_CURPOS */
#else
static void
ck_termset(x) int x; {
    if (x) return;
}
#endif /* NOTERMCAP */

#ifndef CK_CURPOS
#define CK_CURPOS
int
ck_cls() {
    printf("\033[;H\033[2J");
    return(0);
}

int
ck_cleol() {
    printf("\033[K");
    return(0);
}

int
ck_curpos(row, col) int row, col; {
    printf("\033[%d;%dH", row, col);
    return(0);
}
#endif /* CK_CURPOS */


static int cinit = 0;                   /* Flag for curses init'd */
static int cendw = 0;                   /* endwin() was called */

static
#ifdef CK_ANSIC                         /* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */

#ifdef CK_ANSIC                         /* Update % transfered and % bar */
updpct(long old, long new)
#else /* CK_ANSIC */
updpct(old, new) long old, new;
#endif /* CK_ANSIC */
/* updpct */ {
#ifdef COMMENT
    int m, n;
    move(CW_PCD,22);
    printw("%ld", new);
#ifdef KUI
#ifndef K95G
    KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_PCD, (long) new);
#endif /* K95G */
#endif /* KUI */
#ifdef CK_PCT_BAR
    if (thermometer) {
        if (old > new) {
            old = 0;
            move(CW_PCD, 26);
            clrtoeol();
        }
        m = old/2;
        move(CW_PCD, 26 + m);
        n = new / 2 - m;
#ifndef OS2
        while (n > 0) {
            if ((m + 1) % 5 == 0)
              printw("*");
            else
              printw("=");
            m++;
            n--;
        }
        if (new % 2 != 0) printw("-");
        /* move(CW_PCD, 22+53); */
#else /* OS2 */
        while (n > 0) {
            printw("%c", '\333');
            m++; n--;
        }
        if (new % 2 != 0)
          printw("%c", '\261');
#endif /* OS2 */
    }
#endif /* CK_PCT_BAR */
    /* clrtoeol(); */
#else  /* !COMMENT */
#ifdef OS2
#define CHAR1   '\333'          /* OS2 */
#define CHAR2   '\261'
#else
#define CHAR1   '/'             /* Default */
#define CHAR2   '-'
#endif /* OS2 */
    debug(F101,"updpct old","",old);
    debug(F101,"updpct new","",new);
    move(CW_PCD,22);
    printw("%-3ld", new); /*  (was)   printw("%ld", new);  */
#ifdef KUI
#ifndef K95G
    KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PCD, (long) new );
#endif /* K95G */
#endif /* KUI */
#ifdef CK_PCT_BAR
    if (thermometer) {
        int m, n;

        if (old > new) {
            old = 0 ;
            move(CW_PCD, 26);
            clrtoeol();
        }
        if (new <= 100L) {
            m = old / 2;
            n = new / 2 - m;
            move(CW_PCD, 26+m);
            while (n-- > 0)
              printw("%c", CHAR1);
            if (new % 2 != 0)
              printw("%c", CHAR2);
        }
    }
#endif /* CK_PCT_BAR */
#endif /* COMMENT */
}

static long old_tr = -1L;               /* Time remaining previously */

static long
#ifdef CK_ANSIC
shoetl(long old_tr, long cps, long fsiz, long howfar)
#else
shoetl(old_tr, cps, fsiz, howfar) long old_tr, cps, fsiz, howfar;
#endif /* CK_ANSIC */
/* shoetl */ {                          /* Estimated time left in transfer */
    long tr;                            /* Time remaining, seconds */

#ifdef GFTIMER
    if (fsiz > 0L && cps > 0L)
      tr = (long)((CKFLOAT)(fsiz - howfar) / (CKFLOAT)cps);
    else
      tr = -1L;
#else
    tr = (fsiz > 0L && cps > 0L) ?
      ((fsiz - howfar) / cps) :
        -1L;
#endif /* GFTIMER */
    debug(F101,"SUNDAY tr","",tr);
    debug(F101,"SUNDAY old_tr","",old_tr);
    move(CW_TR,22);
    if (tr > -1L) {
        if (tr != old_tr) {
            printw("%s",hhmmss(tr));
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long)CW_TR, (long)hhmmss(tr));
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
        }
    } else {
        printw("(unknown)");
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TR, (long) "(unknown)" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
    }
    return(tr);
}

static long
#ifdef CK_ANSIC
shocps(int pct, long fsiz, long howfar)
#else
shocps(pct, fsiz, howfar) int pct; long fsiz, howfar;
#endif /* CK_ANSIC */
/* shocps */ {
    static long oldffc = 0L;
#ifdef GFTIMER
    CKFLOAT secs, xx;
#else
    long secs, xx;
#endif /* GFTIMER */

#ifdef GFTIMER
    xx = (gtv >= 0.0) ? gtv : 0.0;      /* Floating-point version */
    gtv = gftimer();
    if ((gtv - oldgtv) < (CKFLOAT) 1.0) /* Only do this once per second */
      return(oldcps);
    oldgtv = xx;
#else
    xx = (gtv >= 0) ? gtv : 0;          /* Whole-number version */
    gtv = gtimer();
    if ((gtv - oldgtv) < 1)
      return(oldcps);
    oldgtv = xx;
#endif /* GFTIMER */

#ifdef CPS_WEIGHTED
    debug(F100,"SHOCPS: WEIGHTED","",0);
    if (gtv != oldgtv) {                /* The first packet is ignored */
        if (ffc < oldffc)
          oldffc = ffc;
        oldcps = cps;
        if (oldcps && oldgtv >
#ifdef GFTIMER
            1.0
#else
            1
#endif /* GFTIMER */
            ) {                         /* The first second is ignored */
/*
  This version of shocps() produces a weighted average that some
  people like, but most people find it disconcerting and bombard us
  with questions and complaints about why the CPS figure fluctuates so
  wildly.  So now you only get the weighted average if you build the
  program yourself with CPS_WEIGHTED defined.
*/
#ifndef CPS_VINCE
#ifdef GFTIMER
            cps = (long)((((CKFLOAT)oldcps * 3.0) +
                   (CKFLOAT)(ffc - oldffc) / (gtv-oldgtv) ) / 4.0);
#else
            cps = ( (oldcps * 3) + (ffc - oldffc) / (gtv-oldgtv) ) / 4;
#endif /* GFTIMER */
#else
/* And an alternate weighting scheme from Vincent Fatica... */
            cps = (3 *
             ((1+pct/300)*oldffc/oldgtv+(1-pct/100)*(ffc-oldffc)/(gtv-oldgtv)))
              / 4;
#endif /* CPS_VINCE */
        } else {
            /* No weighted average since there is nothing to weigh */
#ifdef GFTIMER
            cps = (long)(gtv != 0.0 ?
              (CKFLOAT)(ffc - oldffc) / (gtv - oldgtv) :
                (ffc - oldffc)) ;
#else
            cps = gtv ? (ffc - oldffc) / (gtv - oldgtv) : (ffc - oldffc) ;
#endif /* GFTIMER */
        }
#ifdef DEBUG
        if (deblog) {
            debug(F101,"SHOCPS: pct   ","",pct);
            debug(F101,"SHOCPS: gtv   ","",gtv);
            debug(F101,"SHOCPS: oldgtv","",oldgtv);
            debug(F101,"SHOCPS: dgtv  ","",(long)(gtv-oldgtv));
            debug(F101,"SHOCPS: ffc   ","",ffc);
            debug(F101,"SHOCPS: oldffc","",oldffc);
            debug(F101,"SHOCPS: dffc  ","",ffc-oldffc);
            debug(F101,"SHOCPS: cps   ","",cps);
        }
#endif /* DEBUG */
        move(CW_CP,22);
        printw("%ld", cps);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        oldffc = ffc;
    }
#else /* !CPS_WEIGHTED */
#ifdef DEBUG
    debug(F100,"SHOCPS: NOT WEIGHTED","",0);
    if (deblog) {
        debug(F101,"SHOCPS: pct    ","",pct);
        debug(F101,"SHOCPS: gtv    ","",gtv);
        debug(F101,"SHOCPS: oldgtv ","",oldgtv);
        debug(F101,"SHOCPS: dgtv   ","",(long)gtv - (long)oldgtv);
        debug(F101,"SHOCPS: ffc    ","",ffc);
        debug(F101,"SHOCPS: oldffc ","",oldffc);
        debug(F101,"SHOCPS: dffc   ","",ffc-oldffc);
        debug(F101,"SHOCPS: cps    ","",cps);
        debug(F101,"SHOCPS: filcnt ","",filcnt);
#ifdef GFTIMER
        debug(F101,"SHOCPS: fpfsecs","",fpfsecs);
#endif /* GFTIMER */
    }
    debug(F101,"shocps gtv","",gtv);
#endif /* DEBUG */
#ifdef GFTIMER
    debug(F101,"shocps fpfsecs","",fpfsecs);
    secs = gtv - fpfsecs;
    debug(F101,"shocps secs","",(long)secs);
    if (secs > 0.0) {
        cps = (long)((CKFLOAT) ffc / secs);
        debug(F101,"shocps cps","",cps);
        move(CW_CP,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        printw("%ld", cps);
        clrtoeol();
    }
#else  /* Not GFTIMER */
    if ((secs = gtv - fsecs) > 0) {
        cps = (secs < 1L) ? ffc : ffc / secs;
        move(CW_CP,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, (long) cps );
#endif /* K95G */
#endif /* KUI */
        printw("%ld", cps);
        clrtoeol();
    }
#endif /* GFTIMER */
#endif /* CPS_WEIGHTED */

    if (cps > peakcps &&                /* Peak transfer rate */
        ((what == W_SEND && spackets > wslots + 4) ||
        (what != W_SEND && spackets > 10))) {
        peakcps = cps;
    }
    old_tr = shoetl(old_tr, cps, fsiz, howfar);
    return(cps);
}

static
#ifdef CK_ANSIC                         /* Because VOID used by curses.h */
void
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
#endif /* CK_ANSIC */
scrft() {                               /* Display file type */
    char xferstr[80];
    xferstr[0] = NUL;
    if (binary) {
        switch(binary) {
          case XYFT_L:
            strcpy( xferstr, "LABELED" ) ;
            break;
          case XYFT_I:
            strcpy( xferstr, "IMAGE" );
            break;
          case XYFT_U:
            strcpy( xferstr, "BINARY UNDEFINED" );
            break;
          default:
          case XYFT_B:
            strcpy( xferstr, "BINARY" );
            break;
        }
#ifdef CK_RESEND
        if (what == W_SEND && sendstart > 0L) {
            if (sendmode == SM_PSEND) {
                strcat( xferstr, " / partial");
            } else if (sendmode == SM_RESEND) {
                strcat( xferstr, " / resend");
            }
        } else if (what == W_RECV && rs_len > 0L) {
            strcat( xferstr, " / resend");
        }
#endif /* CK_RESEND */
    } else {
        strcpy(xferstr, "TEXT") ;
#ifndef NOCSETS
        if (tcharset == TC_TRANSP) {
            strcat( xferstr, " (no translation)");
        } else {
            if (what == W_SEND) {
                sprintf( &xferstr[strlen(xferstr)],
                        " (%s => %s)",
                        fcsinfo[fcharset].keyword,
                        tcsinfo[tcharset].keyword);
            } else {
                sprintf( &xferstr[strlen(xferstr)],
                        " (%s => %s)",
                        tcsinfo[tcharset].keyword,
                        fcsinfo[fcharset].keyword);
            }
        }
#endif /* NOCSETS */
    }
    move(CW_TYP,22);
    printw("%s", xferstr);
    clrtoeol();
#ifdef KUI
#ifndef K95G
    KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TYP, (long) xferstr );
#endif /* K95G */
#endif /* KUI */
    return;
}

#ifdef CK_NEWTERM
static FILE *ck_stdout = NULL;
static int ck_fd = -1;
#endif /* CK_NEWTERM */

static long pct = 0L, oldpct = 0L, oldrtt = -1L;
static int oldtyp = 0, oldwin = -1, oldtry = -1, oldlen = -1, oldtim = -1;

#ifdef NETCONN
static char *netname[] = {
    "none", "TCP/IP", "TCP/IP", "X.25", "DECnet", "VAX PSI", "Named Pipes",
    "X.25", "NetBIOS", "SuperLAT", "File", "Command", "DLL", "SSH",
    "X.25", "X.25", NULL
};
#endif /* NETCONN */

#ifdef CK_ANSIC
void
screenc(int f, char c,long n,char *s)
#else
#ifdef MYCURSES
VOID
#else
int
#endif /* MYCURSES */
screenc(f,c,n,s)
int f;          /* argument descriptor */
char c;         /* a character or small integer */
long n;         /* a long integer */
char *s;        /* a string */
#endif /* CK_ANSIC */
/* screenc() */ {
#ifdef CK_SSL
    extern int tls_active_flag, ssl_active_flag;
#endif /* CK_SSL */
#ifdef RLOGCODE
    extern int ttnproto;
#endif /* RLOGCODE */

    static int q = 0;
    static long fsiz = -1L;   /* Copy of file size */
    static long fcnt = 0L;    /* Number of files transferred */
    static long fbyt = 0L;    /* Total file bytes of all files transferred */
    static long howfar = 0L;  /* How much of current file has been xfer'd. */
    static int  pctlbl = 0L;  /* Percent done vs Bytes so far */
    long cps = 0L;

    int len;                            /* Length of string */
    int errors = 0;                     /* Error counter */
    int x;                              /* Worker */

    debug(F101,"screenc cinit","",cinit);
    debug(F101,"screenc cendw","",cendw);

    if (!s) s = "";                     /* Always do this. */

    if (cinit == 0 || cendw > 0) {      /* Handle borderline cases... */
        if (f == SCR_CW) {              /* Close window, but it's not open */
            ft_win = 0;
            return;
        }
        debug(F111,"screenc fatal A",s,f);
        if (f == SCR_EM ||
           (f == SCR_PT && c == 'E')) { /* Fatal error before window open */
            conoll(""); conoc('?'); conoll(s); return; /* Regular display */
        }
    }
    if (cinit == 0) {                   /* Only call initscr() once */
        cendw = 1;                      /* New window needs repainting */
#ifdef COMMENT
        if (!initscr()) {               /* Oops, can't initialize window? */
/*
  In fact, this doesn't happen.  "man curses" says initscr() halts the
  entire program if it fails, which is true on the systems where I've
  tested it.  It will fail if your terminal type is not known to it.
  That's why SET FILE DISPLAY FULLSCREEN calls tgetent() to make sure the
  terminal type is known before allowing a curses display.
*/
            fprintf(stderr,"CURSES INITSCR ERROR\r\n");
            fdispla = XYFD_S;           /* Fall back to CRT display */
            return;
        } else {
            cinit++;                    /* Window initialized ok */
            debug(F100,"CURSES INITSCR OK","",0);
        }
#else                                   /* Save some memory. */
#ifdef CK_NEWTERM
        /* (From Andy Fyfe <andy@vlsi.cs.caltech.edu>)
           System V curses seems to reserve the right to alter the buffering
           on the output FILE* without restoring it.  Fortunately System V
           curses provides newterm(), an alternative to initscr(), that
           allows us to specify explicitly the terminal type and input and
           output FILE pointers.  Thus we duplicate stdout, and let curses
           have the copy.  The original remains unaltered.  Unfortunately,
           newterm() seems to be particular to System V.
        */
        s = getenv("TERM");
        if (ck_fd < 0) {
            ck_fd = dup(fileno(stdout));
            ck_stdout = (ck_fd >= 0) ? fdopen(ck_fd, "w") : NULL;
        }
        debug(F100,"screenc newterm...","",0);
        if (ck_stdout == NULL || newterm(s, ck_stdout, stdin) == 0) {
            fprintf(stderr,
              "Fullscreen display not supported for terminal type: %s\r\n",s);
            fdispla = XYFD_S;           /* Use CRT instead */
            return;
        }
        debug(F100,"screenc newterm ok","",0);
#else
        debug(F100,"screen calling initscr","",0);
        initscr();                      /* Initialize curses. */
        debug(F100,"screen initscr ok","",0);
#endif /* CK_NEWTERM */
        cinit++;                        /* Remember curses was initialized. */
#endif /* COMMENT */
    }
    ft_win = 1;                         /* Window is open */
    if (repaint) {
#ifdef CK_WREFRESH
/*
  This totally repaints the screen, just what we want, but we can only
  do this with real curses, and then only if clearok() and wrefresh() are
  provided in the curses library.
*/
#ifdef OS2
        RestoreCmdMode();
#else
#ifdef QNX
#ifndef QNX16
        clearok(stdscr, 1);             /* QNX doesn't have curscr */
#endif /* QNX16 */
        wrefresh(stdscr);
#else
        wrefresh(curscr);
#endif /* QNX */
#endif /* OS2 */
#else  /* No CK_WREFRESH */
/*
  Kermit's do-it-yourself method, works with all types of fullscreen
  support, but does not repaint all the fields.  For example, the filename
  is lost, because it arrives at a certain time and never comes again, and
  Kermit presently does not save it anywhere.  Making this method work for
  all fields would be a rather major recoding task, and would add a lot of
  complexity and storage space.
*/
        cendw = 1;
#endif /* CK_WREFRESH */
        repaint = 0;
    }
    if (cendw) {                        /* endwin() was called previously */
#ifdef VMS
        initscr();                      /* (or should have been!) */
        clear();
        touchwin(stdscr);
        refresh();
#else
#ifdef QNX
/*
  In QNX, if we don't call initscr() here we core dump.
  I don't have any QNX curses documentation, but other curses manuals
  say that initscr() should be called only once per application, and
  experience shows that on other systems, calling initscr() here generally
  results in a core dump.
*/
        debug(F100,"screenc re-calling initscr QNX","",0);
        initscr();
        clear();
        refresh();
#ifdef COMMENT
/*
  But even so, second and subsequent curses displays are messed up.
  Calling touchwin, refresh, etc, doesn't make any difference.
*/
        debug(F100,"screenc calling touchwin QNX","",0);
        touchwin(stdscr);
        debug(F100,"screenc calling refresh QNX","",0);
        refresh();
#endif /* COMMENT */

#else /* All others... */
        debug(F100,"screenc calling clear","",0);
        clear();
        debug(F100,"screenc clear ok","",0);
#endif /* QNX */
#endif /* VMS */
        debug(F100,"screenc setup ok","",0);
        debug(F100,"screenc doing first move","",0);
        move(CW_BAN,0);                 /* Display the banner */
        debug(F110,"screenc myhost",myhost,0);
#ifdef TCPSOCKET
        debug(F110,"screenc myipaddr",myipaddr,0);
#endif /* TCPSOCKET */
#ifdef HPUX1010
        debug(F100,"screenc calling first printw...","",0);
/* Right here is where HP-UX 10.10 libxcurse.1 Rev 76.20 hangs... */
#endif /* HPUX1010 */
        if (myhost[0]) {
#ifdef TCPSOCKET
            if (!myipaddr[0]
#ifdef OS2
                 /* We need to perform this test because on non-TCP/IP */
                 /* systems the call to getlocalipaddr() results in a  */
                 /* DNS Lookup which takes several minutes to time out */
                 && network &&
                 (nettype == NET_TCPA || nettype == NET_TCPB)
#endif /* OS2 */
                 )
              getlocalipaddr();
            if (myipaddr[0] && strcmp((char *)myhost,(char *)myipaddr))
              printw("%s, %s [%s]",versio,(char *)myhost,(char *)myipaddr);
            else
#endif /* TCPSOCKET */
              printw("%s, %s",versio,(char *)myhost);
        } else {
            printw("%s",versio);
        }
#ifdef HPUX1010
        debug(F100,"screenc first printw returns","",0);
#endif /* HPUX1010 */
        move(CW_DIR,3);
        printw("Current Directory: %s",zgtdir());
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) zgtdir() );
#endif /* K95G */
#endif /* KUI */
        if (network) {
            move(CW_LIN,8);
            printw("Network Host: %s",ttname);
        } else {
            move(CW_LIN,0);
            printw("Communication Device: %s",ttname);
        }
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_LIN, (long) ttname );
#endif /* K95G */
#endif /* KUI */

        if (network) {
            move(CW_SPD,8);
            printw("Network Type: ");
        } else {
            move(CW_SPD,1);
            printw("Communication Speed: ");
        }
        move(CW_SPD,22);                /* Serial speed or network type */
        if (network) {
#ifdef NETCONN
            if (0
#ifdef CK_ENCRYPTION
                || ck_tn_encrypting() && ck_tn_decrypting()
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
                || tls_active_flag || ssl_active_flag
#endif /* CK_SSL */
#ifdef RLOGCODE
#ifdef CK_KERBEROS
#ifdef CK_ENCRYPTION
                || ttnproto == NP_EK4LOGIN || ttnproto == NP_EK5LOGIN
#endif /* CK_ENCRYPTION */
#endif /* CK_KERBEROS */
#endif /* RLOGCODE */
                 ) {
#ifdef KUI
#ifndef K95G
                char buf[30];
                sprintf(buf,"%s (SECURE)",netname[nettype]);
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) buf
                               );
#endif /* K95G */
#endif /* KUI */
                printw("%s (SECURE)",netname[nettype]);
            } else {
                printw("%s",netname[nettype]);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) netname[nettype]
                               );
#endif /* K95G */
#endif /* KUI */
            }
#else
            printw("(network)");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_SPD,
                           (long) "(network)"
                           );
#endif /* K95G */
#endif /* KUI */
#endif /* NETCONN */
        } else {
            if (speed < 0L)
              speed = ttgspd();
            if (speed > 0L) {
                if (speed == 8880) {
                    printw("75/1200");
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) "75/1200"
                                   );
#endif /* K95G */
#endif /* KUI */
                } else {
                    char speedbuf[64] ;
                    sprintf(speedbuf, "%ld", speed);
                    printw("%s",speedbuf);
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER,
                                   (long) CW_SPD,
                                   (long) speedbuf
                                   );
#endif /* K95G */
#endif /* KUI */
                }
            } else {
                printw("unknown");
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_SPD,
                               (long) "(unknown)"
                               );
#endif /* K95G */
#endif /* KUI */
            }
        }
        move(CW_PAR,14);
        printw("Parity: %s",parnam((char)parity));
#ifdef KUI
#ifndef K95G
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_PAR,
                       (long) parnam((char)parity)
                       );
#endif /* K95G */
#endif /* KUI */
#ifdef CK_TIMERS
        if (rttflg && protocol == PROTO_K) {
            move(CW_TMO, 9); printw("RTT/Timeout:"); }
#endif /* CK_TIMERS */
        move(CW_TYP,11); printw("File Type:");
        move(CW_SIZ,11); printw("File Size:");
        move(CW_PCD, 8);
        pctlbl = (what == W_SEND);
        printw("%s:", pctlbl ? "Percent Done" : "Bytes so far");

#ifdef XYZ_INTERNAL
        move(CW_BAR, 1);
        printw("%10s Protocol:",ptab[protocol].p_name);
#endif /* XYZ_INTERNAL */
#ifdef CK_PCT_BAR
        if (thermometer) {
            oldpct = pct = 0;
            move(CW_BAR,22);
            printw("    ...10...20...30...40...50...60...70...80...90..100");
            move(CW_BAR,22+56);
        }
#endif /* CK_PCT_BAR */
        move(CW_TR,  1); printw("Estimated Time Left:");
        move(CW_CP,  2); printw("Transfer Rate, CPS:");
        move(CW_WS,  8); printw("Window Slots:%s",
                                protocol == PROTO_K ?
                                "" : " N/A"
                                );
        move(CW_PT,  9); printw("Packet Type:");
#ifdef XYZ_INTERNAL
        if (protocol != PROTO_K) {
            move(CW_PC,  11); printw("I/O Count:");
            move(CW_PL,  10); printw("I/O Length:");
        } else {
#endif /* XYZ_INTERNAL */
            move(CW_PC,  8); printw("Packet Count:");
            move(CW_PL,  7); printw("Packet Length:");
#ifdef XYZ_INTERNAL
        }
#endif /* XYZ_INTERNAL */
#ifndef COMMENT
        move(CW_PR,  9); printw("Error Count:");
#else
        move(CW_PR,  2); printw("Packet Retry Count:");
#endif
#ifdef COMMENT
        move(CW_PB,  2); printw("Packet Block Check:");
#endif /* COMMENT */
        move(CW_ERR,10); printw("Last Error:");
        move(CW_MSG, 8); printw("Last Message:");

        move(CW_INT, 0);
        if (!xfrint) {
            printw("(Transfer interruption is disabled)");
        } else {
#ifdef CK_NEED_SIG
            printw(
"<%s>X to cancel file, <%s>Z to cancel group, <%s><CR> to resend last packet",
                   dbchr(escape), dbchr(escape), dbchr(escape)
                   );
            move(CW_INT + 1, 0);
            printw(
"<%s>E to send Error packet, ^C to quit immediately, <%s>L to refresh screen.",
                   dbchr(escape), dbchr(escape)
                   );
#else /* !CK_NEED_SIG */
            move(CW_INT, 0);
#ifdef OS2
            if (protocol == PROTO_K) {
                printw(
"X to cancel file, Z to cancel group, <Enter> to resend last packet,"
                       );
            }
#else /* !OS2 */
#ifdef VMS                              /* In VMS avoid bottom line */
            printw(
"X: Cancel this file; E: Cancel transfer; ^C: Quit now; ^W: Refresh screen."
                   );
#else
            printw(
"X to cancel file, Z to cancel group, <CR> to resend last packet,"
                   );
#endif /* VMS */
#endif /* OS2 */

#ifndef VMS
            move(CW_INT + 1, 0);
            if (protocol == PROTO_K) {
                printw(
"E to send Error packet, ^C to quit immediately, ^L to refresh screen."
                       );
            } else {
                printw("^C to cancel file transfer.");
            }
#endif /* VMS */
#endif /* CK_NEED_SIG */
        }
        refresh();
        cendw = 0;
    }
    debug(F101,"SCREENC switch","",f);
    debug(F000,"SCREENC c","",c);
    debug(F101,"SCREENC n","",n);

    len = strlen(s);                    /* Length of argument string */
    switch (f) {                        /* Handle our function code */
      case SCR_FN:                      /* Filename */
        oldpct = pct = 0L;              /* Reset percents */
#ifdef GFTIMER
        gtv = (CKFLOAT) -1.0;
        /* oldgtv = (CKFLOAT) -1.0; */
#else
        gtv = -1L;
        /* oldgtv = -1L; */
#endif /* GFTIMER */
        oldwin = -1;
        fsiz = -1L;                     /* Invalidate previous file size */
        move(CW_PCD,22);                /* Erase percent done from last time */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PCD, (long) 0 );
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_FFC, (long) 0 );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        move(CW_SIZ,22);                /* Erase file size from last time */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) 0 );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        move(CW_ERR,22);                /* And last error message */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
#ifdef COMMENT
#ifdef STREAMING
        if (protocol == PROTO_K && streamok) {
            move(CW_BAR, 1);
#ifdef XYZ_INTERNAL
            printw("   Kermit STREAMING:");
#else
            printw("          STREAMING:");
#endif /* XYZ_INTERNAL */
        }
#endif /* STREAMING */
#endif /* COMMENT */

        if (what == W_SEND) {           /* If we're sending... */
#ifdef CK_RESEND
            switch (sendmode) {
              case SM_RESEND:
                move(CW_NAM,11);
                printw("RESENDING:");
                break;
              default:
                move(CW_NAM,13);
                printw("SENDING:");
                break;
            }
#else
            move(CW_NAM,13);
            printw("SENDING:");
#endif /* CK_RESEND */

        } else if (what == W_RECV) {    /* If we're receiving... */
            move(CW_NAM,11);
            printw("RECEIVING:");
        } else {                        /* If we don't know... */
            move(CW_NAM,11);            /* (should never see this) */
            printw("File Name:");
        }
        move(CW_NAM,22);                /* Display the filename */
        if (len > 57) {
            printw("%.55s..",s);
            len = 57;
        } else printw("%s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        q = len;                        /* Remember name length for later */
        clrtoeol();
        scrft();                        /* Display file type (can change) */
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_AN:                      /* File as-name */
        if (q + len + 4 < 58) {         /* Will fit */
            move(CW_NAM, 22 + q);
            printw(" => %s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        } else {                        /* Too long */
            move(CW_NAM, 22);           /* Overwrite previous name */
            q = 0;
            if (len + 4 > 57) {                                 /* wg15 */
                printw(" => %.51s..",s);                        /* wg15 */
                len = 53;                                       /* wg15 */
            } else printw(" => %s",s);                          /* wg15 */
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s  );
#endif /* K95G */
#endif /* KUI */
        }
        q += len + 4;                   /* Remember horizontal position */
        clrtoeol();
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_FS:                      /* File size */
        fsiz = n;
        move(CW_SIZ,22);
        if (fsiz > -1L) {
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_SIZ, (long) n );
#endif /* K95G */
#endif /* KUI */
            printw("%ld",n);
        }
        clrtoeol();
        move(CW_PCD, 8);
        if (fsiz > -1L) {               /* Put up percent label */
            pctlbl = 1;
            printw("Percent Done:");
        }
        clrtoeol();
        scrft();                        /* File type */
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_PT:                      /* Packet type or pseudotype */
        if (spackets < 5) {
            extern int sysindex;
            extern struct sysdata sysidlist[];
            /* Things that won't change after the 4th packet */
            move(CW_PAR,22);
            printw("%s",parnam((char)parity));
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER,
                           (long) CW_PAR,
                           (long) parnam((char)parity)
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
#ifdef COMMENT
            move(CW_PB, 22);            /* Block check on this packet */
            if (bctu == 4)
              printw("B");
            else
              printw("%d",bctu);
            clrtoeol();
#endif /* COMMENT */
            if (spackets == 4) {
                move(CW_LIN,8);
                if (protocol == PROTO_K && sysindex > -1) {
                    if (network) {
                        move(CW_LIN,8);
                        printw("Network Host: %s (%s)",
                             ttname,
                             sysidlist[sysindex].sid_name
                             );
                    }
                    else {
                        move(CW_LIN,0);
                        printw("Communication Device: %s (remote host is %s)",
                             ttname,
                             sysidlist[sysindex].sid_name
                             );
                    }
                    clrtoeol();
                }
            }
        }
#ifdef CK_TIMERS
        if (rttflg && protocol == PROTO_K) {
            long xx;
            if (
#ifdef STREAMING
                streaming && oldwin != -2
#else
                0
#endif /* STREAMING */
                ) {
                move(CW_TMO, 22);
                printw("00 / 00");
                clrtoeol();
            } else {
                xx = (rttdelay + 500) / 1000;
                if (xx != oldrtt || rcvtimo != oldtim) {
                    move(CW_TMO, 22);
                    printw("%02ld / %02d", xx, rcvtimo);
                    oldrtt = xx;
                    oldtim = rcvtimo;
                    clrtoeol();
                }
            }
        }
#endif /* CK_TIMERS */

        x = (what == W_RECV) ?          /* Packet length */
          rpktl+(protocol==PROTO_K?1:0) :
            spktl;
        if (x != oldlen) {              /* But only if it changed. */
            move(CW_PL, 22);
            printw("%d",x);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PL, (long) x );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldlen = x;
        }
        move(CW_PC, 22);                /* Packet count (always). */

        printw("%d", (what == W_RECV) ? rpackets : spackets);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PC, (long) spackets );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();

        if (protocol == PROTO_K) {      /* Window slots */
            char ws[16];
            int flag;
            flag = 0;
#ifdef STREAMING
            if (streaming) {
                if (oldwin != -2) {
                    sprintf(ws,"STREAMING");
                    flag = 1;
                    oldwin = -2;
                }
            } else
#endif /* STREAMING */
              if (wcur != oldwin) {
                  sprintf(ws, "%d of %d", wcur < 1 ? 1 : wcur, wslotn);
                  flag = 1;
                  oldwin = wcur;
              }
            if (flag) {
                move(CW_WS, 22);
                printw("%s", ws);
                clrtoeol();
#ifdef KUI
#ifndef K95G
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_WS, (long) ws );
#endif /* K95G */
#endif /* KUI */
            }
        }
        errors = retrans + crunched + timeouts;
        if (errors != oldtry) {         /* Retry count, if changed */
            move(CW_PR, 22);
            printw("%d",errors);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PR, (long) errors );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldtry = errors;
        }
        if (c != oldtyp && c != 'Y' && c != 'N') { /* Sender's packet type */
            char type[2];
            sprintf(type, "%c",c);
            move(CW_PT,22);
            printw("%s", type);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_PT, (long) type );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            oldtyp = c;
        }
        switch (c) {                    /* Now handle specific packet types */
          case 'S':                     /* Beginning of transfer */
            fcnt = fbyt = 0L;           /* Clear counters */
#ifdef GFTIMER
            gtv = -1.0;
#else /* GFTIMER */
            gtv = -1L;                  /* And old/new things... */
#endif /* GFTIMER */
            oldpct = pct = 0L;
            break;

          case 'Z':                     /* or EOF */
            debug(F101,"screenc SCR_PT Z pktnum","",n);
            debug(F101,"screenc SCR_PT Z oldpct","",oldpct);
            debug(F101,"screenc SCR_PT Z pct","",pct);
          case 'D':                     /* Data packet */
            if (fsiz > 0L) {            /* Show percent done if known */
                oldpct = pct;           /* Remember previous percent */
                howfar = ffc;
#ifdef CK_RESEND
                if (what == W_SEND)     /* Account for PSEND or RESEND */
                  howfar += sendstart;
                else if (what == W_RECV)
                  howfar += rs_len;
#endif /* CK_RESEND */
                /* Percent done, to be displayed... */
                if (c == 'Z') {
                    if (!discard && !cxseen && !czseen) pct = 100L;
                } else
                  pct = (fsiz > 99L) ? (howfar / (fsiz / 100L)) : 0L;
                if (pct > 100L ||       /* Allow for expansion and */
                   (oldpct == 99L && pct < 0L)) /* other boundary conditions */
                  pct = 100L;
                if (pct != oldpct)      /* Only do this 100 times per file */
                  updpct(oldpct, pct);
            } else {
                move(CW_PCD,22);
                printw("%ld", ffc);
            }
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) howfar);
#endif /* K95G */
#endif /* KUI */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            break;

          case '%':                     /* Timeouts, retransmissions */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */

            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                move(CW_PR, 22);
                printw("%d",errors);
                clrtoeol();
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR, (long) errors
                               );
#endif /* K95G */
#endif /* KUI */
                }
                oldtry = errors;
                if (s) if (*s) {
                    move(CW_ERR,22);
                    printw("%s",s);
                    clrtoeol();
#ifdef KUI
#ifndef K95G
                    KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_ERR, (long) s);
#endif /* K95G */
#endif /* KUI */
            }
            break;

          case 'E':                     /* Error packet */
#ifdef COMMENT
            move(CW_ERR,22);            /* Print its data field */
            if (*s) {
                printw("%s",s);
#ifdef KUI
#ifndef K95G
                KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
            }
            clrtoeol();
#endif /* COMMENT */
            fcnt = fbyt = 0L;           /* So no bytes for this file */
            break;
          case 'Q':                     /* Crunched packet */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            move(CW_ERR,22);
            printw("Damaged Packet");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Damaged Packet"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            break;
          case 'q':                     /* Ctrl-C or connection lost */
            move(CW_MSG,22);
            if (!s) s = "";
            printw(*s ? s : "User interruption or connection lost");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_MSG,
                           (long) s
                           );
#endif /* K95G */
#endif /* KUI */
            break;
          case 'T':                     /* Timeout */
            cps = shocps((int) pct, fsiz, howfar);
            /* old_tr = shoetl(old_tr, cps, fsiz, howfar); */
            move(CW_ERR,22);
            printw("Timeout %d sec",rcvtimo);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Timeout"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol();
            errors = retrans + crunched + timeouts;
            if (errors != oldtry) {     /* Error count, if changed */
                move(CW_PR, 22);
                printw("%d",errors);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,
                               (long) CW_PR, (long) errors
                               );
#endif /* K95G */
#endif /* KUI */
                clrtoeol();
                oldtry = errors;
            }
            break;
          default:                      /* Others, do nothing */
            break;
        }
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      case SCR_ST:                      /* File transfer status */
        debug(F101,"screenc SCR_ST c","",c);
        debug(F101,"screenc SCR_ST success","",success);
        debug(F101,"screenc SCR_ST cxseen","",cxseen);
#ifdef COMMENT
        move(CW_PCD,22);                /* Update percent done */
        if (c == ST_OK) {               /* OK, print 100 % */
            if (pctlbl)
              updpct(oldpct,100);
            else
              printw("%ld", ffc);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
#endif /* K95G */
#endif /* KUI */
            pct = 100;
            oldpct = 0;
        } else if (fsiz > 0L)           /* Not OK, update final percent */
/*
  The else part writes all over the screen -- howfar and/or fsiz have
  been reset as a consequence of the not-OKness of the transfer.
*/
          if (pctlbl)
            updpct(oldpct, (howfar * 100L) / fsiz);
        clrtoeol();
#else
        if (c == ST_OK) {               /* OK, print 100 % */
            move(CW_PCD,22);            /* Update percent done */
            if (pctlbl)
              updpct(oldpct,100);
            else
              printw("%ld", ffc);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER, (long) CW_FFC, (long) ffc);
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = 100;
            oldpct = 0;
#endif /* COMMENT */
            clrtoeol();
        }
#endif /* COMMENT */

#ifdef COMMENT
/* No, leave it there so they can read it */
        move(CW_MSG,22);                /* Remove any previous message */
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_MSG, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh();
#endif /* COMMENT */

        move(CW_TR, 22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_TR, (long) "" );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh();

        switch (c) {                    /* Print new status message */
          case ST_OK:                   /* Transfer OK */
            fcnt++;                     /* Count this file */
            fbyt += ffc;                /* Count its bytes */
            move(CW_MSG,22);
            printw("Transfer OK");      /* Say Transfer was OK */
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_MSG,
                           (long) "Transfer OK"
                           );
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh();
            return;

          case ST_DISC:                 /* Discarded */
            move(CW_ERR,22);
            printw("File discarded");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "File discarded"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_INT:                  /* Interrupted */
            move(CW_ERR,22);
            printw("Transfer interrupted");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "Transfer interrupted"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_SKIP:                 /* Skipped */
            move(CW_ERR,22);
            printw("File skipped");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,
                           (long) CW_ERR,
                           (long) "File skipped"
                           );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_ERR:                  /* Error message */
            move(CW_ERR,22);
            if (!s) s = (char *)epktmsg;
            printw("%s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_REFU:                 /* Refused */
            move(CW_ERR,22);
            if (*s) {
                char errbuf[64] ;
                sprintf( errbuf, "Refused, %s", s ) ;
                printw("%s", errbuf);
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,(long) CW_ERR,(long) errbuf);
#endif /* K95G */
#endif /* KUI */
            } else {
                printw("Refused");
#ifdef KUI
#ifndef K95G
                KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Refused");
#endif /* K95G */
#endif /* KUI */
            }
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_INC:
            move(CW_ERR,22);
            printw("Incomplete");
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_ERR,(long)"Incomplete");
#endif /* K95G */
#endif /* KUI */
#ifdef COMMENT
            pct = oldpct = 0;
#endif /* COMMENT */
            clrtoeol(); refresh();
            return;

          case ST_MSG:
            move(CW_MSG,22);
            printw("%s",s);
#ifdef KUI
#ifndef K95G
            KuiSetProperty(KUI_FILE_TRANSFER,(long)CW_MSG,(long)s);
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh();
            return;

          default:                      /* Bad call */
            move(CW_ERR,22);
            printw("*** screen() called with bad status ***");
#ifdef KUI
#ifndef K95G
            KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR,
                       (long) "*** screen() called with bad status ***" );
#endif /* K95G */
#endif /* KUI */
            clrtoeol(); refresh(); return;
        }

      case SCR_TC: {                    /* Transaction complete */
          char msgbuf[128];
          int eff = -1;
          move(CW_CP,22);               /* Overall transfer rate */
#ifdef KUI
#ifndef K95G
          KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_CP, tfcps);
#endif /* K95G */
#endif /* KUI */
          printw("%ld", tfcps);
          clrtoeol();
          if (success) {
              move(CW_MSG,22);          /* Print statistics in message line */
              clrtoeol();
          }
          if (success) {
              sprintf(msgbuf,
                      "SUCCESS.  Files: %ld, Bytes: %ld, %ld CPS",
                      fcnt,
                      fbyt,
                      tfcps
                      );
              printw("%s", msgbuf);
#ifdef KUI
#ifndef K95G
              KuiSetProperty(KUI_FILE_TRANSFER,
                             (long) CW_MSG,
                             (long) msgbuf
                             );
#endif /* K95G */
#endif /* KUI */
              clrtoeol();

          }
          move(CW_TR, 1);
          printw("       Elapsed Time: %s",hhmmss((long)
#ifdef GFTIMER
                                                  (fptsecs + 0.5)
#else
                                                  tsecs
#endif /* GFTIMER */
                                                   ));
#ifdef KUI
#ifndef K95G
          KuiSetProperty(KUI_FILE_TRANSFER,
                         (long) CW_TR,
                         (long) hhmmss((long)
#ifdef GFTIMER
                                       (fptsecs + 0.5)
#else
                                       tsecs
#endif /* GFTIMER */
                                       ));
#endif /* K95G */
#endif /* KUI */
          clrtoeol();
          move(23,0); clrtoeol();       /* Clear instructions lines */
          move(22,0); clrtoeol();       /* to make room for prompt. */
          refresh();

#ifdef GFTIMER
          oldgtv = (CKFLOAT) -1.0;
#else
          oldgtv = -1L;
#endif /* GFTIMER */

#ifndef VMSCURSE
          endwin();
#ifdef SOLARIS
          conres();
#endif /* SOLARIS */
#endif /* VMSCURSE */
#ifdef COMMENT
          pct = 100; oldpct = 0;        /* Reset these for next time. */
#endif /* COMMENT */
          oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
          oldtim = -1;
          cendw = 1;
          if (xfrbel) bleep(BP_NOTE);   /* Close window, then beep. */
#ifdef UNIX
          fflush(stdout);
#endif /* UNIX */
          ft_win = 0;                   /* Window closed. */
          return;
      }
      case SCR_EM:                      /* Error packet (fatal) */
        move (CW_ERR,22);
        printw("FAILURE: %s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_ERR, (long) s );
#endif /* K95G */
#endif /* KUI */
        if (xfrbel) bleep(BP_FAIL);
#ifdef COMMENT
        pct = oldpct = 0;
#endif /* COMMENT */
        clrtoeol(); refresh(); return;

      case SCR_QE:                      /* Quantity equals */
      case SCR_TU:                      /* Undelimited text */
      case SCR_TN:                      /* Text delimited at start */
      case SCR_TZ:                      /* Text delimited at end */
        return;                         /* (ignored in fullscreen display) */

      case SCR_XD:                      /* X-packet data */
        pct = oldpct = 0;
        move(CW_NAM,22);
        printw("%s",s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_NAM, (long) s );
#endif /* K95G */
#endif /* KUI */
        clrtoeol(); refresh(); return;

      case SCR_CW:                      /* Close Window */
        clrtoeol(); move(23,0); clrtoeol(); move(22,0); clrtoeol();
        refresh();
#ifdef COMMENT
        pct = 100; oldpct = 0;          /* Reset these for next time. */
#endif /* COMMENT */
        oldtyp = 0; oldrtt = -1L; oldtry = -1; oldlen = -1;
        oldtim = -1;

#ifndef VMSCURSE
        endwin();
#endif /* VMSCURSE */
        ft_win = 0;                     /* Flag that window is closed. */
        cendw = 1; return;

      case SCR_CD:                      /* Display current directory */
        move(CW_DIR,22);
         printw("%s", s);
#ifdef KUI
#ifndef K95G
        KuiSetProperty( KUI_FILE_TRANSFER, (long) CW_DIR, (long) s );
#endif /* K95G */
#endif /* KUI */
        clrtoeol();
        refresh();
#ifdef OS2
        SaveCmdMode(0, 0);
#endif /* OS2 */
        return;

      default:                          /* Bad call */
        move (CW_ERR,22);
#ifdef KUI
#ifndef K95G
        KuiSetProperty(KUI_FILE_TRANSFER,
                       (long) CW_ERR,
                       (long) "*** screen() called with bad function code ***"
                       );
#endif /* K95G */
#endif /* KUI */
        printw("*** screen() called with bad function code ***");
        clrtoeol(); refresh(); return;
    }
}
#endif /* CK_CURSES */
#endif /* MAC */

#ifndef CK_CURPOS
/* Dummies for when cursor control is not supported */
int
ck_curpos(row, col) {
    return(-1);
}

int
ck_cls() {
    return(-1);
}

int
ck_cleol() {
    return(-1);
}
#endif /* CK_CURPOS */

#ifndef NOIKSD
#ifdef IKSDB

struct iksdbfld dbfld[] = {
   /* Offset    Length    Type   */
    { DB_FLAGS, dB_FLAGS, DBT_HEX },    /*  0 db_FLAGS Flags */
    { DB_ATYPE, dB_ATYPE, DBT_HEX },    /*  1 db_ATYPE Auth type */
    { DB_AMODE, dB_AMODE, DBT_HEX },    /*  3 db_AMODE Auth mode */
    { DB_STATE, dB_STATE, DBT_HEX },    /*  2 db_STATE State */
    { DB_MYPID, dB_MYPID, DBT_HEX },    /*  5 db_MYPID PID */
    { DB_SADDR, dB_SADDR, DBT_HEX },    /*  4 db_SADDR Server address */
    { DB_CADDR, dB_CADDR, DBT_HEX },    /*  6 db_CADDR Client address */
    { DB_START, dB_START, DBT_DAT },    /*  7 db_START Session start */
    { DB_LASTU, dB_LASTU, DBT_DAT },    /*  8 db_LASTU Last update */
    { DB_ULEN,  dB_ULEN,  DBT_HEX },    /*  9 db_ULEN  Username length */
    { DB_DLEN,  dB_DLEN,  DBT_HEX },    /* 10 db_DLEN  Directory name length */
    { DB_ILEN,  dB_ILEN,  DBT_HEX },    /* 11 db_ILEN  Info length */
    { DB_PAD1,  dB_PAD1,  DBT_UND },    /* 12 db_PAD1  (Reserved) */
    { DB_USER,  dB_USER,  DBT_STR },    /* 13 db_USER  Username */
    { DB_DIR,   dB_DIR,   DBT_STR },    /* 14 db_DIR   Current Directory */
    { DB_INFO,  dB_INFO,  DBT_STR }     /* 15 db_INFO  State-specific info */
};

static char lcknam[CKMAXPATH+1];        /* Lockfile pathname */
static char tmplck[CKMAXPATH+1];        /* Temporary lockfile name */

static char * updmode =                 /* Update mode for fopen() */
#ifdef OS2
  "r+b"
#else
#ifdef VMS
  "r+b"
#else
  "r+"
#endif /* VMS */
#endif /* OS2 */
  ;

/*  D B I N I T  --  Initialize the IKSD database...  */

int
dbinit() {
    extern int dbinited;
    int x = 0;
    debug(F110,"dbinit dbdir 1",dbdir,0);
    debug(F110,"dbinit dbfile 1",dbfile,0);
    if (dbinited)
      return(0);
#ifdef OS2
    if (!dbdir) {
#ifdef NT
        char * p = NULL;
        if (!isWin95()) {
            p = getenv("SystemRoot");
        } else {
            p = getenv("winbootdir");
            if (!p)  p = getenv("windir");
        }
        if (!p) p = "C:/";
        dbdir = malloc(strlen(p)+2);
        strcpy(dbdir,p);
        p = dbdir;
        while (*p) {
            if (*p == '\\')
              *p = '/';
            p++;
        }
#else /* NT */
        makestr(&dbdir,"C:/");
#endif /* NT */
    }
#else /* OS2 */
    if (!dbdir)
      makestr(&dbdir,IK_DBASEDIR);
#endif /* OS2 */

    if (!dbfile) {
        char * s = "";
        x = strlen(dbdir);
        if (dbdir[x-1] != '/') {
            s = "/";
            x++;
        }
        x += (int)strlen(IK_DBASEFIL);
        dbfile = (char *)malloc(x+1);
        sprintf(dbfile,"%s%s%s",dbdir,s,IK_DBASEFIL);
    }
    debug(F110,"dbinit dbdir 2",dbdir,0);
    debug(F110,"dbinit dbfile 2",dbfile,0);
    mypid = getpid();                   /* Get my pid */
    debug(F101,"dbinit mypid","",mypid);

    if (!myhexip[0]) {                  /* Set my hex IP address */
#ifdef TCPSOCKET
        extern unsigned long myxipaddr;
        if (getlocalipaddr() > -1) {
            myip = myxipaddr;
            sprintf(myhexip,"%08lx",myip); /* (Needs fixing for IPv6) */
        } else
#endif /* TCPSOCKET */
          ckstrncpy(myhexip,"00000000",9);
    }
    debug(F011,"dbinit myip",myhexip,myip);
    if (!peerhexip[0]) {                /* Get peer's  hex IP address */
#ifdef TCPSOCKET
        extern unsigned long peerxipaddr;
        if (ckgetpeer()) {
            peerip = peerxipaddr;
            sprintf(peerhexip,"%08lx",peerip); /* (Needs fixing for IPv6) */
            debug(F011,"dbinit peerip",peerhexip,peerip);
        } else {
            debug(F101,"dbinit ckgetpeer failure","",errno);
            ckstrncpy(peerhexip,"00000000",9);
        }
#else
        ckstrncpy(peerhexip,"00000000",9);
#endif /* TCPSOCKET */
    }
    debug(F111,"dbinit peerip",peerhexip,peerip);
    debug(F101,"dbinit dbenabled","",dbenabled);
    if (dbenabled && inserver) {
        mydbslot = getslot();
        debug(F111,"dbinit getslot",ckitoa(ikdbopen),x);
        if (ikdbopen) dbinited = 1;
    }
    return(0);
}

/*  U P D S L O T  --  Update slot n  */

/*
  Opens the database if necessary, seeks to slot n, writes current record
  and adds current time to last-update field.  n is the record sequence number
  (0, 1, 2, ...), not the seek pointer.   Returns -1 on failure, 0 on success.
*/
int
updslot(n) int n; {                     /* Update our slot */
    int x, rc = 0;
    long position;
    if (!ikdbopen)                      /* Not if not ok */
      return(0);
    if (!dbfp) {                        /* Open database if not open */
        dbfp = fopen(dbfile,updmode);   /* In update no-truncate mode */
        if (!dbfp) {
            debug(F110,"updslot fopen failed",dbfile,0);
            ikdbopen = 0;
            return(-1);
        }
    }
    position = n * DB_RECL;
    if (fseek(dbfp,position,0) < 0) {   /* Seek to desired slot */
        debug(F111,"updslot fseek failed",dbfile,mydbseek);
        ikdbopen = 0;
        rc = -1;
    } else {
        /* Update the update time */
        strncpy(&dbrec[dbfld[db_LASTU].off],
                ckdate(),
                dbfld[db_LASTU].len
                );
        if (fwrite(dbrec,1,DB_RECL,dbfp) < DB_RECL) { /* Write the record */
            debug(F110,"updslot fwrite failed",dbfile,0);
            ikdbopen = 0;
            rc = -1;
        } else {                        /* Flush the write */
            fflush(dbfp);
        }
    }
    return(rc);
}

/*  I N I T S L O T --  Initialize slot n with my info  */

int
initslot(n) int n; {                    /* Initialize slot */
    int i, j, k;
    char * s;
#ifdef TCPSOCKET
    extern unsigned long peerxipaddr;
#endif /* TCPSOCKET */

    debug(F101,"initslot","",n);

#ifdef USE_MEMCPY
    memset(dbrec,32,DB_RECL);
#else
    for (i = 0; i < DB_RECL; i++)
      dbrec[i] = '\040';
#endif /* USE_MEMCPY */

    myflags = DBF_INUSE;                /* Set in-use flag */
    mystate = W_NOTHING;
    myatype = 0L;
    myamode = 0L;

    k = dbfld[db_FLAGS].len;            /* Length of flags field */
    strncpy(&dbrec[dbfld[db_FLAGS].off],ulongtohex(myflags,k),k);

    k = dbfld[db_ATYPE].len;
    strncpy(&dbrec[dbfld[db_ATYPE].off],ulongtohex(myatype,k),k);

    k = dbfld[db_AMODE].len;
    strncpy(&dbrec[dbfld[db_AMODE].off],ulongtohex(myamode,k),k);

    k = dbfld[db_STATE].len;
    strncpy(&dbrec[dbfld[db_STATE].off],ulongtohex(mystate,k),k);

    k = dbfld[db_SADDR].len;
    strncpy(&dbrec[dbfld[db_SADDR].off],ulongtohex(myip,k),k);

#ifdef TCPSOCKET
    ckgetpeer();
    k = dbfld[db_CADDR].len;
    strncpy(&dbrec[dbfld[db_CADDR].off],ulongtohex(peerxipaddr,k),k);
#else
    k = dbfld[db_CADDR].len;
    strncpy(&dbrec[dbfld[db_CADDR].off],ulongtohex(0L,k),k);
#endif /* TCPSOCKET */

    k = dbfld[db_MYPID].len;
    strncpy(&dbrec[dbfld[db_MYPID].off],ulongtohex(mypid,k),k);

    k = dbfld[db_START].len;
    strncpy(&dbrec[dbfld[db_START].off],ckdate(),k);

    k = dbfld[db_ULEN].len;
    strncpy(&dbrec[dbfld[db_ULEN].off],"0000",4);

    k = dbfld[db_DLEN].len;
    strncpy(&dbrec[dbfld[db_DLEN].off],"0000",4);

    k = dbfld[db_ILEN].len;
    strncpy(&dbrec[dbfld[db_ILEN].off],"0000",4);

    strncpy(&dbrec[dbfld[db_INFO].off],"INIT",4);
    return(updslot(n));
}

int
slotstate(x,s1,s2,s3) int x; char *s1, *s2, *s3; {
    int k, l1, l2, l3, z;
    mystate = x;
    debug(F101,"slotstate ikdbopen","",ikdbopen);
    if (!ikdbopen)
      return(-1);
    if (!s1) s1 = "";
    l1 = strlen(s1);
    if (!s2) s2 = "";
    l2 = strlen(s2);
    if (!s3) s3 = "";
    l3 = strlen(s3);
    strncpy(&dbrec[DB_STATE],ulongtohex(mystate,4),4);
    k = dbfld[db_ILEN].len;
    z = l1 + l2 + l3 + 2;
    if (z > dB_INFO)
      z = dB_INFO;
    strncpy(&dbrec[DB_ILEN],ulongtohex((unsigned long)z,k),k);
    k = dbfld[db_INFO].len;
    z = dbfld[db_INFO].off;
    if (l1 <= k) {
        lset(&dbrec[z],s1,l1+1,32);
        z += l1+1;
        k -= l1+1;
        if (l2 <= k) {
            lset(&dbrec[z],s2,l2+1,32);
            z += l2+1;
            k -= l2+1;
            if (l3 <= k)
              lset(&dbrec[z],s3,k,32);
        }
    }
#ifdef DEBUG
    if (deblog) {
        char buf[128];
        int i;
        strncpy(buf,&dbrec[DB_INFO],127);
        buf[127] = NUL;
        for (i = 126; i > 0 && buf[i] == 32; i--) buf[i] = 0;
        debug(F111,"slotstate",buf,mystate);
    }
#endif /* DEBUG */
    z = updslot(mydbslot);
    debug(F111,"slotstate updslot","",z);
    return(z);
}

int
slotdir(s1,s2) char * s1, * s2; {       /* Update current directory */
    int k, len1, len2;
    if (!ikdbopen)
      return(-1);
    if (!s1) s1 = "";
    if (!s2) s2 = "";
    len1 = strlen(s1);
    len2 = strlen(s2);
    k = dbfld[db_DLEN].len;
    strncpy(&dbrec[DB_DLEN],ulongtohex((unsigned long)(len1+len2),k),k);
    k = dbfld[db_DIR].len;
    if (len1 > 0) {
        lset(&dbrec[dbfld[db_DIR].off],s1,len1,32);
        lset(&dbrec[dbfld[db_DIR].off+len1],s2,k-len1,32);
    } else {
        lset(&dbrec[dbfld[db_DIR].off],s2,k,32);
    }
    return(updslot(mydbslot));
}

/*  F R E E S L O T  --  Free slot n  */

int
freeslot(n) int n; {
    int k;
    if (!ikdbopen)
      return(0);
    dbflags = 0L;
    if (n == mydbslot) {
        dbflags = myflags & ~DBF_INUSE;
        dbflags &= ~DBF_LOGGED;
    }
    k = dbfld[db_FLAGS].len;
    strncpy(&dbrec[dbfld[db_FLAGS].off],ulongtohex(dbflags,k),k);
    return(updslot(n));
}

/*  G E T S L O T  --  Find a free database slot; returns slot number  */

int
getslot() {                             /* Find a free slot for us */
    FILE * rfp = NULL;                  /* Returns slot number (0, 1, ...) */
    char idstring[64];                  /* PID string buffer (decimal) */
    char pidbuf[64], * s;
    int j, k, n, x, rc = -1;
    int lockfd, tries, haveslot = 0;
    long lockpid, i;
    char ipbuf[17];

    if (!myhexip[0])                    /* Set my hex IP address if not set */
      ckstrncpy((char *)myhexip,"7F000001",33);
    sprintf(idstring,"%08lx:%010ld\n",myip,mypid);
    debug(F110,"getslot idstring", idstring, 0);

    /* Make temporary lockfile name IP.PID (hex.hex) */
    /* This should fit in 14 chars -- huge PIDs are usually not possible */
    /* on 14-char filename systems. */

    sprintf(tmplck,"%s%08lx.%lx",dbdir,myip,mypid);
    debug(F110,"getslot tempfile",tmplck,0);

    /* Make a temporary file */

    lockfd = creat(tmplck, 0600);
    if (lockfd < 0) {
        debug(F111,"getslock temp lockfile create failure", tmplck, errno);
        return(-1);
    }
    /* Write my (decimal) PID into the temp file */

    write(lockfd,idstring,(int)strlen(idstring));
    if (close(lockfd) < 0) {            /* Close lockfile */
        debug(F101,"getslot error closing temp lockfile", "", errno);
        return(-1);
    }
    sprintf(lcknam,"%s%s",dbdir,IK_LOCKFILE); /* Build lockfile name */
    debug(F110,"getslot lockfile",lcknam,0);

    rfp = fopen(lcknam,"r");            /* See if lockfile exists */
    if (rfp) {                          /* If so... */
        int sameip = 0;
        rset(pidbuf,"",64,0);
        x = fread(pidbuf,1,63,rfp);     /* Read ID string from it */
        fclose(rfp);                    /* and close it quickly */
        debug(F110,"getslot lock exists",pidbuf,0);
        if (x > 0) {                    /* If we have a PID, check it */
            char * s = pidbuf;
            while (*s) {
                if (islower(*s)) *s = toupper(*s);
                if (*s == ':') {
                    *s = NUL;
                    debug(F110,"getslot lock IP",pidbuf,0);
                    debug(F110,"gteslot my   IP",myhexip,0);
                    if (!strcmp(pidbuf,myhexip)) { /* Same IP address? */
                        lockpid = atol(s+1); /* Yes, now get PID */
                        debug(F101,"getslot lockpid","",lockpid);

                        /* Check if PID lockpid on this computer is alive */
                        x = zchkpid(lockpid);
                        if (!x) {
                            debug(F100,"getslot PID stale,removing lock","",0);
                            unlink(lcknam);
                        }
                        break;
                    }
                }
                s++;
            }
        } else {
            debug(F111,"getslot lockfile open failure",lcknam,errno);
        }
    }
    /* Try IK_LCKTRIES (16) times to rename temp file to lockfile */

    for (tries = IK_LCKTRIES; tries > 0; tries--) {
        if (zrename(tmplck,lcknam) == 0)
          break;
        debug(F101,"getslot database locked by pid", "", dbpid);
        sleep(IK_LCKSLEEP);
    }
    if (tries < 1) {                    /* Couldn't */
        debug(F110,"getslot create lock failure",lcknam,0);
        return(-1);
    }
    /* Have lock, open database */

    debug(F110,"getslot has lock",lcknam,0); /* Have lock */

    if (!dbfile)
      return(-1);

    /* If database doesn't exist, create it. */

    debug(F110,"getslot dbfile",dbfile,0);
    if (zchki(dbfile) < 0) {
        debug(F110,"getslot creating new database",dbfile,0);
        x = creat(dbfile,0660);
        if (x < 0) {
            debug(F111,"getslot creat() failed", dbfile, errno);
            goto xslot;
        }
        close(x);
    }
    dbfp = fopen(dbfile,updmode);       /* Open it in update mode */
    if (!dbfp) {
        debug(F111,"getslot fopen failed",dbfile,errno);
        goto xslot;
    }
    /* Now find a free (or new) slot... */

    dblastused = 0L;                    /* Seek pointer to last record inuse */
    mydbseek = 0L;                      /* Seek pointer for my record */

    /* Quickly read the whole database; n = record counter, i = seek pointer */

    for (n = 0, i = 0; !feof(dbfp); i += DB_RECL, n++) {
        x = fread(dbrec,1,DB_RECL,dbfp); /* Read a record */
        if (x < 1)                      /* EOF not caught by feof() */
          break;
#ifndef NOFTRUNCATE
        if (x != DB_RECL) {             /* Watch out for trailing junk */
            debug(F101,"getslot bad size","",x);  /* (Shouldn't happen...) */
#ifdef COHERENT
            chsize(fileno(dbfp),i);
#else
            ftruncate(fileno(dbfp),i);
#endif /* COHERENT */
            x = 0;
            fseek(dbfp,i,0);
            break;
        }
#endif /* NOFTRUNCATE */
        debug(F101,"getslot record","",n);
        k = dbfld[db_FLAGS].off;
        j = dbfld[db_FLAGS].len;
        dbflags = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbflags","",dbflags);
        k = dbfld[db_MYPID].off;
        j = dbfld[db_MYPID].len;
        dbpid  = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbpid","",dbpid);
        k = dbfld[db_SADDR].off;
        j = dbfld[db_SADDR].len;
        dbip = hextoulong(&dbrec[k],j);
        debug(F001,"getslot dbip","",dbip);

        if (dbflags & DBF_INUSE) {      /* Remember last slot in use */
            x = 0;                      /* Make sure it's REALLY in use */
            if (dbpid == mypid && dbip == myip) { /* Check for PID == my PID */
                x = 1;
                debug(F101,"getslot record pid","",dbpid);
            } else {                    /* Or for stale PID */
                x = zchkpid(dbpid);
                debug(F101,"getslot zchkpid()","",x);
            }
            if (!x) {                   /* Bogus record */
                x = freeslot(n);
                debug(F101,"getslot stale record pid: freeslot()","",x);
                if (x > -1 && !haveslot)
                  dbflags = 0;
            } else {                    /* It's really in use */
                dblastused = i;
            }
        }
        if (!haveslot) {                /* If I don't have a slot yet */
            if (!(dbflags & DBF_INUSE)) {       /* Claim this one */
                debug(F101,"getslot free slot", "", n);
                haveslot = 1;
                mydbseek = i;
                mydbslot = n;           /* But keep going... */
            }
        }
    }
    /* Come here with i == seek pointer to first record after eof */

    if (!haveslot) {                    /* Found no free slot so add to end */
        debug(F101,"getslot new slot","",n);
        haveslot = 1;
        mydbseek = i;
        mydbslot = n;
    }
    ikdbopen = 1;                       /* OK to make database entries */
    debug(F101,"getslot records","",n);
    debug(F101,"getslot dblastused","",dblastused);
    debug(F101,"getslot i","",i);

    /* Trim stale records from end */

#ifndef NOFTRUNCATE
    if (i > dblastused+DB_RECL) {
        debug(F101,"getslot truncating at","",dblastused+DB_RECL);
#ifdef COHERENT
        x = chsize(fileno(dbfp),dblastused+DB_RECL);
#else
        x = ftruncate(fileno(dbfp),dblastused+DB_RECL);
#endif /* COHERENT */
        if (x < 0)                      /* (Not fatal) */
          debug(F101,"getslot ftruncate failed", "", errno);
    }
#endif /* NOFTRUNCATE */

    /* Initialize my record */

    if (initslot(mydbslot) < 0) {
        debug(F101,"getslot initslot() error","",n);
        ikdbopen = 0;
        goto xslot;
    }
    debug(F101,"getslot OK","",mydbslot);
    rc = mydbslot;                      /* OK return code */

  xslot:                                /* Unlock the database and return */
    if (unlink(lcknam) < 0) {
        debug(F111,"getslot lockfile removal failed",lcknam,errno);
        rc = -1;
    }
    return(rc);
}
#endif /* IKSDB */
#endif /* NOIKSD */
