#include "ckcsym.h"
#define XFATAL fatal

#ifndef NOCMDL

/*  C K U U S Y --  "User Interface" for Unix Kermit, part Y  */

/*  Command-Line Argument Parser */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"
#include "ckuusr.h"
#include <signal.h>

#ifdef OS2
#include <io.h>
#endif /* OS2 */

extern int inserver, fncnv, f_save, xfermode;
#ifdef PATTERNS
extern int patterns;
#endif /* PATTERNS */

#ifndef NOICP
extern int cmdint;
#endif /* NOICP */
extern int suspend;

#ifdef NETCONN
#ifdef ANYX25
extern int revcall, closgr, cudata;
extern char udata[];
extern int x25fd;
#endif /* ANYX25 */
#ifndef VMS
#ifndef OS2
#ifndef OSK
extern
#endif /* OSK */
#endif /* OS2 */
#endif /* VMS */

int telnetfd;
extern struct keytab netcmd[];
extern int tn_exit;
#ifndef NOICP
#ifndef NODIAL
extern int nnets, nnetdir;              /* Network services directory */
extern char *netdir[];
extern char *nh_p[];                    /* Network directory entry pointers */
extern char *nh_p2[];                   /* Network directory entry nettype */
extern char *nh_px[4][MAXDNUMS + 1];
#endif /* NODIAL */
extern int nhcount;
extern char * n_name;                   /* Network name pointer */
#endif /* NOICP */
#endif /* NETCONN */

#ifndef NOSPL
extern int nmac;
extern struct mtab *mactab;
extern char uidbuf[];
#endif /* NOSPL */

#ifdef CK_LOGIN
extern int logintimo;
#endif /* CK_LOGIN */

extern char * myname, * dftty;
extern int howcalled;

extern char *ckxsys, *ckzsys, **xargv, **cmlist, *clcmds;

extern int action, cflg, xargc, cnflg, local, quiet, escape, network, mdmtyp,
  bgset, backgrd, xargs, binary, parity, turn, turnch, duplex, flow, clfils,
  noinit, stayflg, nettype, cfilef, noherald, cmask, cmdmsk, exitonclose,
  haveline, justone, cxtype, xfinish, ttnproto;

extern long speed;
extern char ttname[];
extern char * pipedata, * cmdfil;

char * xargv0 = "";

#ifndef NOXFER
extern char *cmarg, *cmarg2;

extern int nfils, stdouf, stdinf, displa, maxrps, rpsiz, ckwarn, urpsiz,
  wslotr, swcapr, ckdelay, recursive, reliable, xreliable, fnspath, fncact,
  clearrq, setreliable;

#ifdef PIPESEND
extern int usepipes, pipesend;
#endif /* PIPESEND */
extern int protocol;
#endif /* NOXFER */

#ifdef OS2
extern struct keytab os2devtab[];
extern int nos2dev;
extern int ttslip;
#ifdef OS2PM
extern int os2pm;
#endif /* OS2PM */
#endif /* OS2 */

#ifdef CK_NETBIOS
extern unsigned char NetBiosAdapter;
#endif /* CK_NETBIOS */

#ifdef XFATAL
#undef XFATAL
#endif /* XFATAL */

#ifndef NOICP
#ifndef NODIAL
extern int nmdm, telephony;
extern struct keytab mdmtab[];
extern int usermdm, dialudt;
#endif /* NODIAL */
extern int what;
_PROTOTYP(static int pmsg, (char *) );
_PROTOTYP(static int fmsg, (char *) );
static int pmsg(s) char *s; { printf("%s\n", s); return(0); }
static int fmsg(s) char *s; { fatal(s); return(0); }
#define XFATAL(s) return(what==W_COMMAND?pmsg(s):fmsg(s))
#else
#define XFATAL fatal
#endif /* NOICP */

#ifndef NOCMDL
/* Command-Line usage message (must fit in 24x80) */
static
char *hlp1[] = {
#ifndef NOICP
" [cmdfile] [-x arg [-x arg]...[-yyy]..] [ = text ] ]\n",
#else
"[-x arg [-x arg]...[-yyy]..]\n",
#endif /* NOICP */
"  -x is an option requiring an argument, -y an option with no argument.\n",
"actions:\n",
"  -s files  send files                  -r  receive files\n",
"  -s -      send from stdin             -k  receive files to stdout\n",
#ifndef NOSERVER
"  -x        enter server mode           -f  finish remote server\n",
#else
"  -f        finish remote server\n",
#endif /* NOSERVER */
"  -g files  get remote files from server (quote wildcards)\n",
"  -a name   alternate file name, used with -s, -r, -g\n",
#ifndef NOLOCAL
"  -c        connect (before file transfer), used with -l and -b\n",
"  -n        connect (after file transfer), used with -l and -b\n",
#endif /* NOLOCAL */
"settings:\n",
#ifndef NOLOCAL
"  -l dev    communication line device   -q  quiet during file transfer\n",
#ifdef NETCONN
"  -j host   network host name[:port]    -i  binary transfer (-T = text)\n",
#else
"  -i        binary file transfer\n",
#endif /* NETCONN */
"  -b bps    line speed, e.g. 19200      -t  half duplex, xon handshake\n",
#else
"  -i        binary file transfer\n",
#endif /* NOLOCAL */
#ifdef DEBUG
"  -p x      parity, x = e,o,m,s, or n   -d  log debug info to debug.log\n",
#else
"  -p x      parity, x = e,o,m,s, or n\n",
#endif /* DEBUG */
#ifndef NOICP
"  -y name   alternate init file name    -Y  no init file\n",
#else
#endif /* NOICP */
"  -e n      receive packet length       -w  write over files\n",
#ifdef UNIX
"  -v n      sliding window slots        -z  force foreground\n",
#else
"  -v n      sliding window slots\n",
#endif /* UNIX */
#ifndef NODIAL
"  -m name   modem type                  -R  remote-only advisory\n",
#endif /* NODIAL */
/*
  If all this stuff is defined, we run off the screen...
*/
#ifdef CK_NETBIOS
"  -N n      NetBIOS adapter number\n",
#endif /* CK_NETBIOS */
#ifdef ANYX25
" -o index   X.25 closed user group call -X  X.25 address\n",
" -U string  X.25 call user data         -u  X.25 reverse charge call\n",
" -Z n       X.25 connection open file descriptor\n",
#endif /* ANYX25 */
#ifndef NOICP
"If no action command is included, or -S is, enter interactive dialog.\n",
"Type HELP OPTIONS at the prompt for further info.\n",
#else
"Operation by command-line options only.\n",
"See the manual \"Using C-Kermit\" for complete information.\n",
#endif /* NOICP */
""
};

#ifndef NOHELP

/* Command-line option help lines.  Update this when adding new options! */

char * opthlp[128];                     /* Option help */
char * arghlp[128];                     /* Argument for option */
int optact[128];                        /* Action-option flag */
#endif /* NOHELP */

VOID
fatal2(msg1,msg2) char *msg1, *msg2; {
    char buf[256];
    if (!msg1) msg1 = "";
    if (!msg2) msg2 = "";
    sprintf(buf,"\"%s\" - %s",msg1,msg2);
#ifndef NOICP
    if (what == W_COMMAND)
      printf("%s\n",buf);
    else
#endif /* NOICP */
      fatal((char *)buf);
}

static SIGTYP
#ifdef CK_ANSI
cl_int(int dummy)
#else /* CK_ANSI */
cl_int(dummy) int dummy;
#endif /* CK_ANSI */
{					/* Command-line interrupt handler */
    doexit(BAD_EXIT,1);
    SIGRETURN;
}

/*  U S A G E */

VOID
usage() {
#ifndef MINIX
    conol("Usage: ");
    conol(xargv0);
    conola(hlp1);
#else
    conol("Usage: ");
    conol(xargv0);
    conol(" [-x arg [-x arg]...[-yyy]..] ]\n");
#endif /* MINIX */
}
#endif /* NOCMDL */

/*  C M D L I N  --  Get arguments from command line  */

int
cmdlin() {
    char x;                             /* Local general-purpose char */

#ifndef NOXFER
    cmarg = "";                         /* Initialize globals */
    cmarg2 = "";
#endif /* NOXFER */
    action = 0;
    cflg = 0;

    xargv0 = xargv[0];
    debug(F111,"cmdlin myname",myname,howcalled);
    signal(SIGINT,cl_int);

/* Here we handle different "Command Line Personalities" */

#ifdef TCPSOCKET
    if (howcalled == I_AM_TELNET) {     /* If I was called as Telnet... */
        if (--xargc > 0) {              /* And I have a hostname... */
            xargv++;
            ckstrncpy(ttname,*xargv,TTNAMLEN+1);
            debug(F110,"cmdlin telnet host",ttname,0);

#ifndef NOICP
#ifndef NODIAL
            nhcount = 0;                /* Check network directory */
            debug(F101,"cmdlin nnetdir","",nnetdir);
            if (nnetdir > 0)            /* If there is a directory... */
              lunet(*xargv);            /* Look up the name */
            else                        /* If no directory */
              nhcount = 0;              /* we didn't find anything there */
#ifdef DEBUG
            if (deblog) {
                debug(F101,"cmdlin lunet nhcount","",nhcount);
                if (nhcount > 0) {
                    debug(F110,"cmdlin lunet nh_p[0]",nh_p[0],0);
                    debug(F110,"cmdlin lunet nh_p2[0]",nh_p2[0],0);
                    debug(F110,"cmdlin lunet nh_px[0][0]",nh_px[0][0],0);
                }
            }
#endif /* DEBUG */
            if (nhcount > 0 && nh_p2[0]) /* If network type specified */
              if (ckstrcmp(nh_p2[0],"tcp/ip",6,0)) /* it must be TCP/IP */
                nhcount = 0;
            if (nhcount == 1) {         /* Still OK, so make substitution */
                ckstrncpy(ttname,nh_p[0],TTNAMLEN+1);
                debug(F110,"cmdlin lunet substitution",ttname,0);
            }
#endif /* NODIAL */
#endif /* NOICP */

            if (--xargc > 0) {          /* Service specified on cmd line? */
                xargv++;
                strcat(ttname,":");
                strcat(ttname,*xargv);
                debug(F110,"cmdlin telnet host2",ttname,0);
            }
#ifndef NOICP
#ifndef NODIAL
            else if (nhcount) {         /* No - how about in net directory? */
                if (nh_px[0][0]) {
                    strcat(ttname,":");
                    strcat(ttname,nh_px[0][0]);
                }
            }
#endif /* NODIAL */
#endif /* NOICP */
            local = 1;                  /* Try to open the connection */
            nettype = NET_TCPB;
            mdmtyp = -nettype;
            if (ttopen(ttname,&local,mdmtyp,0) < 0) {
                XFATAL("can't open host connection");
            }
            network = 1;                /* It's open */
#ifdef CKLOGDIAL
            dolognet();
#endif /* CKLOGDIAL */
#ifndef NOXFER
            reliable = 1;               /* It's reliable */
            xreliable = 1;              /* ... */
            setreliable = 1;
#endif /* NOXFER */
            cflg = 1;                   /* Connect */
            stayflg = 1;                /* Stay */
            tn_exit = 1;                /* Telnet-like exit condition */
            quiet = 1;
            exitonclose = 1;            /* Exit when connection closes */
#ifndef NOSPL
            if (local) {
                if (nmac) {                     /* Any macros defined? */
                    int k;                      /* Yes */
                    k = mlook(mactab,"on_open",nmac);   /* Look this up */
                    if (k >= 0) {                       /* If found, */
                        if (dodo(k,ttname,0) > -1)      /* set it up, */
                          parser(1);                    /* and execute it */
                    }
                }
            }
#endif /* NOSPL */
        }
        return(0);
    }
#ifdef COMMENT
#ifdef RLOGCODE
    else if (howcalled == I_AM_RLOGIN) { /* If I was called as Rlogin... */
        /* Add rlogin command-line parsing here... */
        return(0);
    }
#endif /* RLOGCODE */
#endif /* COMMENT */
#endif /* TCPSOCKET */

/*
  From here down: We were called as "kermit" or "iksd".

  If we were started directly from a Kermit application file, its name is
  in argv[1], so skip past it.
*/
    if (xargc > 1) {
        int n = 1;
        if (*xargv[1] != '-') {
#ifdef KERBANG
            /* If we were started with a Kerbang script, the script */
            /* arguments were already picked up in prescan / cmdini() */
            /* and there is nothing here for us anyway. */
            if (!strcmp(xargv[1],"+"))
              return(0);
#endif /* KERBANG */

            if (cfilef) {               /* Command file found in prescan() */
                xargc -= n;             /* Skip past it */
                xargv += n;
                cfilef = 0;
                debug(F101,"cmdlin cfilef set to 0","",cfilef);
            }
        }
    }
/*
 Regular Unix-style command line parser, conforming with 'A Proposed Command
 Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World, Vol.1,
 No.3, 1984.
*/
    while (--xargc > 0) {               /* Go through command line words */
        xargv++;
        debug(F111,"cmdlin xargv",*xargv,xargc);
        if (**xargv == '=')
          return(0);
        if (!strcmp(*xargv,"--"))       /* getopt() conformance */
          return(0);
#ifdef VMS
        else if (**xargv == '/')
          continue;
#endif /* VMS */
        else if (**xargv == '-') {      /* Got an option (begins with dash) */
            int xx;
            x = *(*xargv+1);            /* Get the option letter */
            debug(F111,"cmdlin args 1",*xargv,xargc);
            xx = doarg(x);
            debug(F101,"cmdlin doarg","",xx);
            debug(F111,"cmdlin args 2",*xargv,xargc);
            if (xx < 0) {
#ifndef NOICP
                if (what == W_COMMAND)
                  return(0);
                else
#endif /* NOICP */
                  {
#ifdef OS2
                      sleep(1);         /* Give it a chance... */
#endif /* OS2 */
                      doexit(BAD_EXIT,1); /* Go handle option */
                  }
            }
        } else {                        /* No dash where expected */
            char buf[128];
            sprintf(buf,
                    "invalid command-line option, type \"%s -h\" for help",
                    myname
                    );
            fatal2(*xargv,buf);
        }
    }
#ifdef DEBUG
    if (deblog) {
#ifndef NOICP
        debug(F101,"cmdlin what","",what);
#endif /* NOICP */
        debug(F101,"cmdlin action","",action);
#ifndef NOXFER
        debug(F101,"cmdlin stdouf","",stdouf);
#endif /* NOXFER */
    }
#endif /* DEBUG */

#ifdef NOICP
    if (!action && !cflg && !cnflg) {
        debug(F100,"cmdlin NOICP fatal no action","",0);
        XFATAL("?No actions specified on command line");
    }
#else
    if (inserver && what == 0) {        /* Internet Kermit server checks */
        if (local || (action != 0 && action != 'x')) {
            if (local)
              printf("local\r\n");
            if (action)
              printf("action=%c\r\n",action);
            debug(F100,"cmdlin fatal 1","",0);
            XFATAL("No actions or connections allowed with -A");
        }
    }
#endif /* NOICP */

#ifndef NOLOCAL
    if (!local) {
        if ((action == 'c') || (cflg != 0)) {
            debug(F100,"cmdlin fatal 2","",0);
            XFATAL("-l or -j or -X required");
        }
    }
#endif /* NOLOCAL */
#ifndef NOXFER
    if (*cmarg2 != 0) {
        if ((action != 's') && (action != 'r') && (action != 'v')) {
            debug(F100,"cmdlin fatal 3","",0);
            XFATAL("-a without -s, -r, or -g");
        }
        if (action == 'r' || action == 'v') {
#ifdef CK_TMPDIR
            if (isdir(cmarg2)) {        /* -a is a directory */
                if (!zchdir(cmarg2)) {  /* try to change to it */
                    debug(F100,"cmdlin fatal 4","",0);
                    XFATAL("can't change to '-a' directory");
                } else cmarg2 = "";
            } else
#endif /* CK_TMPDIR */
              if (zchko(cmarg2) < 0) {
                  debug(F100,"cmdlin fatal 5","",0);
                  XFATAL("write access to -a file denied");
              }
        }
    }
    if ((action == 'v') && (stdouf) && (!local)) {
        if (is_a_tty(1)) {
            debug(F100,"cmdlin fatal 6","",0);
            XFATAL("unredirected -k can only be used in local mode");
        }
    }
    if ((action == 's') || (action == 'v') ||
        (action == 'r') || (action == 'x')) {
        if (local)
          displa = 1;
        if (stdouf) {
            displa = 0;
            quiet = 1;
        }
    }
    if (quiet) displa = 0;              /* No display if quiet requested */
#endif /* NOXFER */
#ifdef DEBUG
    if (action)
      debug(F000,"cmdlin returns action","",action);
    else
      debug(F101,"cmdlin returns action","",action);
#endif /* DEBUG */
    return(action);                     /* Then do any requested protocol */
}

/* Extended argument parsing: --keyword[:value] (or =value) */

/*
  XA_xxxx symbols are defined in ckuusr.h.
  If you add a new one, also remember to update doshow(),
  SHXOPT section, in ckuus5.c.
*/
#ifndef NOICP
struct keytab xargtab[] = {
#ifdef CK_LOGIN
    "anonymous",   XA_ANON, CM_ARG|CM_PRE,
#endif /* CK_LOGIN */
    "bannerfile",  XA_BAFI, CM_ARG,
    "cdfile",      XA_CDFI, CM_ARG,
    "cdmessage",   XA_CDMS, CM_ARG,
    "cdmsg",       XA_CDMS, CM_ARG|CM_INV,
#ifdef IKSDB
    "database",    XA_DBAS, CM_ARG|CM_PRE,
    "dbfile",      XA_DBFI, CM_ARG|CM_PRE,
#endif /* IKSDB */
    "help",        XA_HELP, 0,
#ifndef NOHELP
    "helpfile",    XA_HEFI, CM_ARG,
#endif /* NOHELP */
#ifdef CK_LOGIN
    "initfile",    XA_ANFI, CM_ARG|CM_PRE,
#endif /* CK_LOGIN */
    "nointerrupts",XA_NOIN, CM_PRE,
#ifdef CK_LOGIN
#ifdef CK_PERM
    "permissions", XA_PERM, CM_ARG|CM_PRE,
    "perms",       XA_PERM, CM_ARG|CM_PRE|CM_INV,
#endif /* CK_PERM */
#ifdef CK_LOGIN
    "privid",      XA_PRIV, CM_ARG|CM_PRE,
#endif /* CK_LOGIN */
#ifdef UNIX
    "root",        XA_ROOT, CM_ARG|CM_PRE,
#endif /* UNIX */
#ifdef CKSYSLOG
    "syslog",      XA_SYSL, CM_ARG|CM_PRE,
    "timeout",     XA_TIMO, CM_ARG|CM_PRE,
#endif /* CKSYSLOG */
    "userfile",    XA_USFI, CM_ARG|CM_PRE,
#ifdef CKWTMP
    "wtmpfile",    XA_WTFI, CM_ARG|CM_PRE,
    "wtmplog",     XA_WTMP, CM_ARG|CM_PRE,
#endif /* CKWTMP */
#endif /* CK_LOGIN */
    "xferfile",    XA_IKFI, CM_ARG|CM_PRE,
    "xferlog",     XA_IKLG, CM_ARG|CM_PRE,
    "",            0,       0
};
int nxargs = sizeof(xargtab)/sizeof(struct keytab) - 1;

static struct keytab oktab[] = {
    "0",     0, 0,
    "1",     1, 0,
    "2",     2, 0,
    "3",     3, 0,
    "4",     4, 0,
    "5",     5, 0,
    "6",     6, 0,
    "7",     7, 0,
    "8",     8, 0,
    "9",     9, 0,
    "false", 0, 0,
    "no",    0, 0,
    "off",   0, 0,
    "ok",    1, 0,
    "on",    1, 0,
    "true",  1, 0,
    "yes",   1, 0
};
static int noktab = sizeof(oktab)/sizeof(struct keytab);

#define XARGBUFL 32

char * bannerfile = NULL;
char * helpfile = NULL;
extern int xferlog;
extern char * xferfile;

#ifndef NOHELP
char * xopthlp[XA_MAX+1];               /* Extended option help */
char * xarghlp[XA_MAX+1];               /* Extended argument for option */

static VOID
inixopthlp() {
    int i, j, n;
    for (i = 0; i <= XA_MAX; i++) {     /* Initialize all to null */
        xopthlp[i] = NULL;
        xarghlp[i] = NULL;
    }
    for (i = 0; i < nxargs; i++) {      /* Then for each defined keyword */
        j = xargtab[i].kwval;           /* index by associated value */
        if (j < 0 || j > XA_MAX)
          continue;
        switch (j) {
#ifdef CK_LOGIN
          case XA_ANON:                 /* "--anonymous" */
            xopthlp[j] = "--anonymous:{on,off} [IKSD only]";
            xarghlp[j] = "Whether to allow anonymous IKSD logins";
            break;
	  case XA_PRIV:
            xopthlp[j] = "--privid:{on,off} [IKSD only]";
            xarghlp[j] = "Whether to allow privileged IDs to login to IKSD";
            break;
#endif /* CK_LOGIN */
          case XA_BAFI:                 /* "--bannerfile" */
            xopthlp[j] = "--bannerfile:<filename>";
            xarghlp[j] = "File to display upon startup or IKSD login";
            break;
          case XA_CDFI:                 /* "--cdfile" */
            xopthlp[j] = "--cdfile:<filename>";
            xarghlp[j] = "File to display when server changes directory";
            break;
          case XA_CDMS:                 /* "--cdmessage" */
            xopthlp[j] = "--cdmessage:{on,off}";
            xarghlp[j] = "Whether to display CD message file";
            break;
          case XA_HELP:                 /* "--help" */
            xopthlp[j] = "--help";
            xarghlp[j] = "Print this help text about extended options";
            break;
          case XA_HEFI:                 /* "--help" */
            xopthlp[j] = "--helpfile:<filename>";
            xarghlp[j] = "File containing custom info for HELP command";
            break;
          case XA_IKFI:                 /* "--xferfile" */
            xopthlp[j] = "--xferfile:<filename> [IKSD only]";
            xarghlp[j] = "Name of ftpd-like logfile.";
            break;
          case XA_IKLG:                 /* "--xferlog" */
            xopthlp[j] = "--xferlog:{on,off} [IKSD only]";
            xarghlp[j] = "Whether to keep an ftpd-like logfile.";
            break;
#ifdef CK_LOGIN
          case XA_ANFI:                 /* "--initfile" */
            xopthlp[j] = "--initfile:<filename> [IKSD only]";
            xarghlp[j] = "Initialization file for anonymous users.";
            break;
#ifdef CK_PERM
          case XA_PERM:                 /* "--permissions" */
            xopthlp[j] = "--permissions:<octalnum> [IKSD only]";
            xarghlp[j] = "Permissions for files uploaded by anonymous users.";
            break;
#endif /* CK_PERM */
#ifdef UNIX
          case XA_ROOT:                 /* "--root" */
            xopthlp[j] = "--root:<directory> [IKSD only]";
            xarghlp[j] = "File-system root for anonymous users.";
            break;
#endif /* UNIX */
#endif /* CK_LOGIN */
#ifdef CKSYSLOG
          case XA_SYSL:                 /* "--syslog" */
            xopthlp[j] = "--syslog:<digit> [IKSD only]";
            xarghlp[j] = "Syslog recording level, 0-6.";
            break;
#endif /* CKSYSLOG */
          case XA_USFI:                 /* "--userfile" */
            xopthlp[j] = "--userfile:<filename> [IKSD only]";
            xarghlp[j] = "Forbidden user file.";
            break;
#ifdef CKWTMP
          case XA_WTFI:                 /* "--wtmpfile" */
            xopthlp[j] = "--wtmpfile:<filename> [IKSD only]";
            xarghlp[j] = "Name of wtmp logfile.";
            break;
          case XA_WTMP:                 /* "--wtmplog" */
            xopthlp[j] = "--wtmplog:{on,off} [IKSD only]";
            xarghlp[j] = "Whether to keep a wtmp logfile.";
            break;
#endif /* CKWTMP */
#ifdef CK_LOGIN
          case XA_TIMO:                 /* "--timeout" */
            xopthlp[j] = "--timeout:<seconds> [IKSD only]";
            xarghlp[j] =
 "How long to wait for login before closing the connection.";
            break;
#endif /* CK_LOGIN */
          case XA_NOIN:
            xopthlp[j] = "--nointerrupts";
            xarghlp[j] = "Disable keyboard interrupts.";
            break;
#ifdef IKSDB
          case XA_DBAS:
            xopthlp[j] = "--database:{on,off}";
            xarghlp[j] = "Enable/Disable IKSD database (IKSD only)";
            break;
          case XA_DBFI:
            xopthlp[j] = "--dbfile:<filename>";
            xarghlp[j] = "Specify IKSD database file (IKSD only)";
            break;
#endif /* IKSDB */
        }
    }
}

VOID
iniopthlp() {
    int i;
    for (i = 0; i < 128; i++) {
        optact[i] = 0;
        switch(i) {
#ifdef OS2
          case '#':                     /* K95 Startup Flags */
            opthlp[i] = "Kermit 95 Startup Flags";
            arghlp[i] =
              "   1 - turn off Win95 special fixes\n"\
              "   2 - do not load optional network dlls\n"\
              "   4 - do not load optional tapi dlls\n"\
              "   8 - do not load optional kerberos dlls\n"\
              "  16 - do not load optional zmodem dlls\n"\
              "  32 - use stdin for input instead of the console\n"\
              "  64 - use stdout for output instead of the console\n"\
              " 128 - do not terminate process in response to Session Logoff";
            break;
#endif /* OS2 */
          case '0':                     /* In the middle */
            opthlp[i] =
              "100% transparent CONNECT mode for \"in-the-middle\" operation";
            arghlp[i] = NULL;
            break;

          case '8':
            opthlp[i] = "Connection is 8-bit clean";
            arghlp[i] = NULL;
            break;
          case 'A':
            opthlp[i] = "C-Kermit is to be started as an Internet service";
#ifdef NT
            arghlp[i] = "  socket handle of incoming connection";
#else /* NT */
            arghlp[i] = NULL;
#endif /* NT */
            break;
          case 'B': opthlp[i] =
            "C-Kermit is running in Batch (no controlling terminal)";
            break;
#ifndef NOSPL
          case 'C':
            opthlp[i] = "Interactive-mode Commands to be executed";
            arghlp[i] = "Commands separated by commas, list in doublequotes";
            break;
#endif /* NOSPL */
          case 'D':
            opthlp[i] = "Delay before starting to send";
            arghlp[i] = "Number of seconds";
            break;
          case 'E':
            opthlp[i] = "Exit automatically when connection closes";
            arghlp[i] = NULL;
            break;
#ifdef TCPSOCKET
          case 'F':
            opthlp[i] = "Make a TCP connection";
            arghlp[i] = "Numeric file descriptor of open TCP connection";
            break;
#endif /* TCPSOCKET */
          case 'G':
            opthlp[i] = "GET from server, send to standard output";
            arghlp[i] = "Remote file specification";
            optact[i] = 1;
            break;
          case 'H':
            opthlp[i] = "Suppress program startup Herald and greeting";
            arghlp[i] = NULL;
            break;
          case 'I':
            opthlp[i] = "Connection is relIable, streaming is allowed";
            arghlp[i] = NULL;
            break;
#ifdef TCPSOCKET
          case 'J':
            opthlp[i] = "'Be like Telnet'";
            arghlp[i] = "IP hostname/address optionally followed by socket";
            break;
#endif /* TCPSOCKET */
          case 'L':
            opthlp[i] = "Recursive directory descent for files in -s option";
            arghlp[i] = NULL;
            break;
          case 'M':
            opthlp[i] = "My user name (for use with Telnet, Rlogin, etc)";
            arghlp[i] = "Username string";
            break;
#ifdef NETBIOS
          case 'N':
            opthlp[i] = "NETBIOS adapter number";
            arghlp[i] = "Number";
            break;
#endif /* NETBIOS */
          case 'O':                     /* Be a server for One command only */
            opthlp[i] = "Be a server for One command only";
            arghlp[i] = NULL;
            optact[i] = 1;
            break;
          case 'P':
            opthlp[i] = "Literal file (Path) names";
            arghlp[i] = NULL;
            break;
          case 'Q':
            opthlp[i] = "Quick (FAST) Kermit protocol settings";
            arghlp[i] = NULL;
            break;
          case 'R':                     /* Remote-Only */
            opthlp[i] = "Remote-only (makes IF REMOTE true)";
            arghlp[i] = NULL;
            break;
          case 'S':                     /* "Stay" - enter interactive */
            opthlp[i] = "Stay (enter command parser after action options)";
            arghlp[i] = NULL;
            break;
          case 'T':                     /* Text file transfer mode */
            opthlp[i] = "Transfer files in Text mode";
            arghlp[i] = NULL;
            break;
#ifdef ANYX25
          case 'U':                     /* X.25 call user data */
            opthlp[i] = "X.25 call User data";
            arghlp[i] = "Call-user-data string";
            break;
#endif /* ANYX25 */
          case 'V':                     /* No automatic filetype switching */
            opthlp[i] = "Disable automatic filetype switching";
            arghlp[i] = NULL;
            break;
#ifdef COMMENT
#ifdef OS2
          case 'W':                     /* Win32 Window Handle */
            opthlp[i] = "";
            arghlp[i] = NULL;
            break;
#endif /* OS2 */
#endif /* COMMENT */
#ifdef ANYX25
          case 'X':                     /* SET HOST to X.25 address */
            opthlp[i] = "Make an X.25 connection";
            arghlp[i] = "X.25 or X.121 address";
            break;
#endif /* ANYX25 */
          case 'Y':                     /* No initialization file */
            opthlp[i] = "Skip initialization file";
            arghlp[i] = NULL;
            break;
#ifdef ANYX25
          case 'Z':                     /* SET HOST to X.25 file descriptor */
            opthlp[i] = "Make an X.25 connection";
            arghlp[i] = "Numeric file descriptor of open X.25 connection";
            break;
#endif /* ANYX25 */
          case 'a':                     /* as-name */
            opthlp[i] = "As-name for file(s) in -s, -r, or -g";
            arghlp[i] = "As-name string";
            break;
          case 'b':                     /* Set bits-per-second for serial */
            opthlp[i] = "Speed for serial device";
            arghlp[i] = "Numeric Bits per second";
            break;
          case 'c':                     /* Connect before */
            optact[i] = 1;
            opthlp[i] = "CONNECT before transferring files";
            arghlp[i] = NULL;
            break;
          case 'd':                     /* DEBUG */
            opthlp[i] = "Create debug.log file";
            arghlp[i] = NULL;
            break;
          case 'e':                     /* Extended packet length */
            opthlp[i] = "Receive packet length";
            arghlp[i] = "Length in bytes";
            break;
          case 'f':                     /* finish */
            optact[i] = 1;
            opthlp[i] = "Send Finish command to server";
            arghlp[i] = NULL;
            break;
          case 'g':                     /* get */
            optact[i] = 1;
            opthlp[i] = "GET file(s)";
            arghlp[i] = "Remote file specification";
            break;
          case 'h':                     /* help */
            optact[i] = 1;
            opthlp[i] = "Print brief Help (usage) text";
            arghlp[i] = NULL;
            break;
          case 'i':                     /* Treat files as binary */
            opthlp[i] ="Transfer files in bInary mode";
            arghlp[i] = NULL;
            break;
#ifdef TCPSOCKET
          case 'j':                     /* SET HOST (TCP/IP socket) */
            opthlp[i] = "Make a TCP connection";
            arghlp[i] = "TCP host name/address and optional socket number";
            break;
#endif /* TCPSOCKET */
          case 'k':                     /* receive to stdout */
            optact[i] = 1;
            opthlp[i] = "RECEIVE file(s) to standard output";
            arghlp[i] = NULL;
            break;
          case 'l':                     /* SET LINE */
            opthlp[i] = "Make connection on serial communications device";
            arghlp[i] = "Device name string";
            break;
          case 'm':                     /* Modem type */
            opthlp[i] = "Modem type for use with -l device";
            arghlp[i] = "Modem type string as in SET MODEM TYPE command";
            break;
          case 'n':                     /* connect after */
            optact[i] = 1;
            opthlp[i] = "CONNECT after transferring files";
            arghlp[i] = NULL;
            break;
#ifdef ANYX25
          case 'o':                     /* X.25 closed user group */
            opthlp[i] = "X.25 closed user group";
            arghlp[i] = "User group string";
            break;
#endif /* ANYX25 */
          case 'p':                     /* SET PARITY */
            opthlp[i] = "Parity";
            arghlp[i] = "One of the following: even, odd, mark, none, space";
            break;
          case 'q':                     /* Quiet */
            opthlp[i] = "Quiet (suppress most messages)";
            arghlp[i] = NULL;
            break;
          case 'r':                     /* receive */
            optact[i] = 1;
            opthlp[i] = "RECEIVE file(s)";
            arghlp[i] = NULL;
            break;
          case 's':                     /* send */
            optact[i] = 1;
            opthlp[i] = "SEND file(s)";
            arghlp[i] = "One or more file specifications";
            break;
          case 't':                     /* Line turnaround handshake */
            opthlp[i] = "XON Turnaround character for half-duplex connections";
            arghlp[i] = NULL;
            break;
#ifdef ANYX25
          case 'u':                     /* X.25 reverse charge call */
            opthlp[i] = "X.25 reverse charge call";
            arghlp[i] = NULL;
            break;
#endif /* ANYX25 */
          case 'v':                     /* Vindow size */
            opthlp[i] = "Window size";
            arghlp[i] = "Number, 1 to 32";
            break;
          case 'w':                     /* Writeover */
            opthlp[i] = "Incoming files Write over existing files";
            arghlp[i] = NULL;
            break;
          case 'x':                     /* Server */
            optact[i] = 1;
            opthlp[i] = "Be a Kermit SERVER";
            arghlp[i] = NULL;
            break;
          case 'y':                     /* Alternate init-file name */
            opthlp[i] = "Alternative initialization file";
            arghlp[i] = "File specification";
            break;
          case 'z':                     /* Not background */
            opthlp[i] = "Force foreground behavior";
            arghlp[i] = NULL;
            break;
          default:
            opthlp[i] = NULL;
            arghlp[i] = NULL;
        }
    }
    inixopthlp();
}
#endif /* NOHELP */

int
doxarg(s,pre) char ** s; int pre; {
#ifdef CK_LOGIN
    extern int ckxsyslog, ckxwtmp, ckxanon, ckxpriv, ckxperms;
    extern char * anonfile, * userfile, * anonroot;
#ifdef CKWTMP
    extern char * wtmpfile;
#endif /* CKWTMP */
#endif /* CK_LOGIN */
    extern int srvcdmsg;
    extern char * cdmsgfile[], * cdmsgstr;
    char tmpbuf[CKMAXPATH+1];

    int i, x, y, z, havearg = 0;
    char buf[XARGBUFL], c, * p;

    if (nxargs < 1)
      return(-1);

    c = *(*s + 1);                      /* Hyphen or Plus sign */

    p = *s + 2;
    for (i = 0; *p && i < XARGBUFL; i++) {
        buf[i] = *p++;
        if (buf[i] == '=' || buf[i] == ':') {
            havearg = 1;
            buf[i] = NUL;
            break;
        } else if (buf[i] < ' ') {
            buf[i] = NUL;
            break;
        }
    }
    if (i > XARGBUFL - 1)
      return(-1);
    buf[i] = NUL;

    x = lookup(xargtab,buf,nxargs,&z);  /* Lookup the option keyword */

    if (x < 0)                          /* On any kind of error */
      return(-1);                       /* fail. */

    /* Handle prescan versus post-initialization file */

    if (((xargtab[z].flgs & CM_PRE) || (c == '+')) && !pre)
      return(0);
    else if (pre && !(xargtab[z].flgs & CM_PRE) && (c != '+'))
      return(0);

    /* Ensure that argument is given if and only if required */

    p = havearg ? *s + i + 3 : NULL;

    if ((xargtab[z].flgs & CM_ARG) && !havearg)
      return(-1);
    else if ((!(xargtab[z].flgs & CM_ARG)) && havearg)
      return(-1);

    switch (x) {                        /* OK to process this option... */
#ifdef CKSYSLOG
      case XA_SYSL:                     /* IKS: Syslog level */
        y = 0;
        if (isdigit(*p)) {
            while (*p) {
                if (*p < '0' || *p > '9')
                  return(-1);
                y = y * 10 + (*p++ - '0');
            }
        } else {
            y = lookup(oktab,p,noktab,&z);
            if (y > 0) y = SYSLG_DF;    /* Yes = default logging level */
        }
#ifndef SYSLOGLEVEL
        /* If specified on cc command line, user can't change it. */
        if (!inserver)                  /* Don't allow voluminous syslogging */
          if (y > SYSLG_FA)             /* by ordinary users. */
            y = SYSLG_FA;
#endif /* SYSLOGLEVEL */
        if (y < 0) return(-1);
#ifdef DEBUG
        if (y >= SYSLG_DB)
          if (!deblog)
            deblog = debopn("debug.log",0);
#endif /* DEBUG */
#ifdef SYSLOGLEVEL
        /* If specified on cc command line, user can't change it. */
        y = SYSLOGLEVEL;
#endif /* SYSLOGLEVEL */
        ckxsyslog = y;
        /* printf("ckxsyslog=%d\n",ckxsyslog); */
        break;
#endif /* CKSYSLOG */

#ifdef CK_LOGIN
#ifdef CKWTMP
      case XA_WTMP:                     /* IKS: wtmp log */
        y = lookup(oktab,p,noktab,&z);
        if (y < 0) return(-1);
        ckxwtmp = y;
        /* printf("ckxwtmp=%d\n",ckxwtmp); */
        break;

      case XA_WTFI:                     /* IKS: wtmp logfile */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&wtmpfile,p);
        /* printf("wtmpfile=%s\n",wtmpfile); */
        break;
#endif /* CKWTMP */

      case XA_ANON:                     /* IKS: Anonymous login allowed */
        y = lookup(oktab,p,noktab,&z);
        if (y < 0) return(-1);
        ckxanon = y;
        /* printf("ckxanon=%d\n",ckxanon); */
        break;

      case XA_PRIV:                     /* IKS: Priv'd login allowed */
        y = lookup(oktab,p,noktab,&z);
        if (y < 0) return(-1);
        ckxpriv = y;
        /* printf("ckxpriv=%d\n",ckxpriv); */
        break;

      case XA_PERM:                     /* IKS: Anonymous Upload Permissions */
        y = 0;
        while (*p) {
            if (*p < '0' || *p > '7')
              return(-1);
            y = y * 8 + (*p++ - '0');
        }
        ckxperms = y;
        /* printf("ckxperms=%04o\n",ckxperms); */
        break;

      case XA_ANFI:                     /* Anonymous init file */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&anonfile,p);
        /* printf("anonfile=%s\n",anonfile); */
        break;

      case XA_USFI:                     /* IKS: Forbidden user file */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&userfile,p);
        /* printf("userfile=%s\n",userfile); */
        break;

      case XA_ROOT:                     /* IKS: Anonymous root */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&anonroot,p);
        /* printf("anonroot=%s\n",anonroot); */
        break;
#endif /* CK_LOGIN */

#ifndef NOICP
      case XA_CDFI:                     /* CD filename */
#ifdef COMMENT
        /* Do NOT expand this one! */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
#endif /* COMMENT */
        makelist(p,cdmsgfile,16);
        makestr(&cdmsgstr,p);
        /* printf("cdmsgstr=%s\n",cdmsgstr); */
        break;
#endif /* NOICP */

      case XA_CDMS:                     /* CD messages */
        y = lookup(oktab,p,noktab,&z);
        if (y < 0) return(-1);
        srvcdmsg = y;
        /* printf("srvcdmsg=%d\n",srvcdmsg); */
        break;

#ifndef NOXFER
      case XA_IKLG:                     /* Transfer log on/off */
        y = lookup(oktab,p,noktab,&z);
        if (y < 0) return(-1);
        xferlog = y;
        /* printf("xferlog=%d\n",xferlog); */
        break;

      case XA_IKFI:                     /* Transfer log file */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&xferfile,p);
        xferlog = 1;
        /* printf("xferfile=%s\n",xferfile); */
        break;

      case XA_BAFI:                     /* IKS: banner (greeting) file */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&bannerfile,p);
        /* printf("bannerfile=%s\n",bannerfile); */
        break;
#endif /* NOXFER */

#ifndef NOHELP
      case XA_HELP:                     /* Help */
        /* printf("help\n"); */
        for (i = 0; i <= XA_MAX; i++)
          if (xopthlp[i])
            printf("%s\n   %s\n\n",xopthlp[i],xarghlp[i]);
#ifndef NOICP
        if (stayflg || what == W_COMMAND)
          break;
        else
#endif /* NOICP */
          doexit(GOOD_EXIT,-1);
#endif /* NOHELP */

#ifndef NOHELP
      case XA_HEFI:                     /* IKS: custom help file */
        if (zfnqfp(p,CKMAXPATH,tmpbuf))
          p = tmpbuf;
        makestr(&helpfile,p);
        /* printf("helpfile=%s\n",helpfile); */
        break;
#endif /* NOHELP */

#ifdef CK_LOGIN
      case XA_TIMO:
        if (!rdigits(p))
          return(-1);
        logintimo = atoi(p);
        /* printf("logintimo=%d\n",p); */
        break;
#endif /* CK_LOGIN */

      case XA_NOIN:                     /* No interrupts */
#ifndef NOICP
        cmdint = 0;
#endif /* NOICP */
        suspend = 0;
        break;

#ifdef IKSDB
      case XA_DBFI: {
          extern char * dbdir, * dbfile;
          extern int dbenabled;
          struct zfnfp * zz;
          if ((zz = zfnqfp(p,CKMAXPATH,tmpbuf))) {
              makestr(&dbdir,zz->fpath);
              makestr(&dbfile,(char *)tmpbuf);
          }
          dbenabled = 1;
          break;
      }
      case XA_DBAS: {
          extern int dbenabled;
          y = lookup(oktab,p,noktab,&z);
          if (y < 0) return(-1);
          dbenabled = y;
          break;
      }
#endif /* IKSDB */

      default:
        return(-1);
    }
    return(0);
}
#endif /* NOICP */

/*  D O A R G  --  Do a command-line argument.  */

int
#ifdef CK_ANSIC
doarg(char x)
#else
doarg(x) char x;
#endif /* CK_ANSIC */
/* doarg */ {
    int i, n, y, z, xx; long zz; char *xp;

#ifdef NETCONN
#define YYBUFLEN 256
    char tmpbuf[YYBUFLEN+1];            /* Local storage for network things */
    char line[YYBUFLEN+1];
#endif /* NETCONN */

#ifdef IKSD
    /* Internet Kermit Server set some way besides -A... */
    if (inserver)
      dofast();
#endif /* IKSD */

    xp = *xargv+1;                      /* Pointer for bundled args */
    debug(F111,"doarg entry",xp,xargc);
    while (x) {
        debug(F000,"doarg arg","",x);
        switch (x) {                    /* Big switch on arg */

#ifndef NOICP
case '-':                               /* Extended commands... */
    if (doxarg(xargv,0) < 0) {
        XFATAL("Extended option error");
    }                                   /* Full thru... */
case '+':                               /* Extended command for prescan() */
    return(0);
#else  /* NOICP */
case '-':
case '+':
    XFATAL("Extended options not configured");
#endif /* NOICP */

#ifndef NOSPL
case 'C': {                             /* Commands for parser */
    char * s;
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("No commands given for -C");
    }
    s = *xargv;                         /* Get the argument (must be quoted) */
    if (!*s)                            /* If empty quotes */
      s = NULL;                         /* ignore this option */
    if (s) {
        makestr(&clcmds,s);             /* Make pokeable copy */
        s = clcmds;                     /* Change tabs to spaces */
        while (*s) {
            if (*s == '\t') *s = ' ';
            s++;
        }
    }
    break;
  }
#endif /* NOSPL */

#ifndef NOXFER
case 'D':                               /* Delay */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing delay value");
    }
    z = atoi(*xargv);                   /* Convert to number */
    if (z > -1)                         /* If in range */
      ckdelay = z;                      /* set it */
    else {
        XFATAL("bad delay value");
    }
    break;
#endif /* NOXFER */

case 'E':                               /* Exit on close */
#ifdef NETCONN
    tn_exit = 1;
#endif /* NETCONN */
    exitonclose = 1;
    break;

#ifndef NOICP
case 'S':                               /* "Stay" - enter interactive */
    stayflg = 1;                        /* command parser after executing */
    xfinish = 0;
    break;                              /* command-line actions. */
#endif /* NOICP */

case 'T':                               /* File transfer mode = text */
    binary = XYFT_T;
    xfermode = XMODE_M;			/* Transfer mode manual */
#ifdef PATTERNS
    patterns = 0;
#endif /* PATTERNS */
    break;

case '7':
    break;

case 'A': {				/* Internet server */
    /* Already done in prescan() */
    /* but implies 'x' &&  'Q'   */
#ifdef NT
    char * p;
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    /* Support for Pragma Systems Telnet/Terminal Servers */
    p = getenv("PRAGMASYS_INETD_SOCK");
    if (!(p && atoi(p) != 0)) {
        xargv++, xargc--;
        if (xargc < 1 || **xargv == '-') {
            XFATAL("missing socket handle");
	}
    }
#endif /* NT */
#ifdef NOICP                            /* If no Interactive Command Parser */
    action = 'x';                       /* -A implies -x. */
#endif /* NOICP */
#ifndef NOXFER
    dofast();
#endif /* NOXFER */
    break;
}

#ifndef NOXFER
case 'Q':                               /* Quick (i.e. FAST) */
    dofast();
    break;
#endif /* NOXFER */

case 'R':                               /* Remote-Only */
    break;                              /* This is handled in prescan(). */

#ifndef NOSERVER
case 'x':                               /* server */
case 'O':                               /* (for One command only) */
    if (action) {
        XFATAL("conflicting actions");
    }
    if (x == 'O') justone = 1;
    xfinish = 1;
    action = 'x';
    break;
#endif /* NOSERVER */

#ifndef NOXFER
case 'f':                               /* finish */
    if (action) {
        XFATAL("conflicting actions");
    }
    action = setgen('F',"","","");
    break;
#endif /* NOXFER */

case 'r': {                             /* receive */
    if (action) {
        XFATAL("conflicting actions");
    }
    action = 'v';
    break;
  }

#ifndef NOXFER
case 'k':                               /* receive to stdout */
    if (action) {
        XFATAL("conflicting actions");
    }
    stdouf = 1;
    action = 'v';
    break;

case 's':                               /* send */
    if (action) {
        XFATAL("conflicting actions");
    }
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -s");
    }
    nfils = 0;                          /* Initialize file counter */
    z = 0;                              /* Flag for stdin */
    cmlist = xargv + 1;                 /* Remember this pointer */
    while (--xargc > 0) {               /* Traverse the list */
        xargv++;
#ifdef PIPESEND
        if (usepipes && protocol == PROTO_K && **xargv == '!') {
            cmarg = *xargv;
            cmarg++;
            debug(F110,"doarg pipesend",cmarg,0);
            nfils = -1;
            z = 1;
            pipesend = 1;
        } else
#endif /* PIPESEND */
        if (**xargv == '-') {           /* Check for sending stdin */
            if (strcmp(*xargv,"-") != 0) /* Watch out for next option. */
              break;
            z++;                        /* "-" alone means send from stdin. */
#ifdef RECURSIVE
        } else if (!strcmp(*xargv,".")) {
            nfils++;
            recursive = 1;
#endif /* RECURSIVE */
        } else if (zchki(*xargv) > -1) { /* Check if file exists */
            nfils++;                    /* Bump file counter */
        } else if (iswild(*xargv) && nzxpand(*xargv,0) > 0) {
            /* or contains wildcard characters matching real files */
            nfils++;
        }
    }
    xargc++, xargv--;                   /* Adjust argv/argc */
    if (nfils < 1 && z == 0) {
#ifdef VMS
        XFATAL("%CKERMIT-E-SEARCHFAIL, no files for -s");
#else
        XFATAL("No files for -s");
#endif /* VMS */
    }
    if (z > 1) {
        XFATAL("-s: too many -'s");
    }
    if (z == 1 && nfils > 0) {
        XFATAL("invalid mixture of filenames and '-' in -s");
    }
    debug(F101,"doarg s nfils","",nfils);
    debug(F101,"doarg s z","",z);
    if (nfils == 0) {
        if (is_a_tty(0)) {              /* (used to be is_a_tty(1) - why?) */
            XFATAL("sending from terminal not allowed");
        } else stdinf = 1;
    }
    debug(F101,"doarg s stdinf","",stdinf);
    debug(F111,"doarg",*xargv,nfils);
    action = 's';
    break;

case 'g':                               /* get */
case 'G':                               /* get to stdout */
    if (action) {
        XFATAL("conflicting actions");
    }
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -g");
    }
    xargv++, xargc--;
    if ((xargc == 0) || (**xargv == '-')) {
        XFATAL("missing filename for -g");
    }
    if (x == 'G') stdouf = 1;
    cmarg = *xargv;
    action = 'r';
    break;
#endif /* NOXFER */

#ifndef NOLOCAL
case 'c':                               /* connect before */
    cflg = 1;
    break;

case 'n':                               /* connect after */
    cnflg = 1;
    break;
#endif /* NOLOCAL */

case 'h':                               /* help */
    usage();
#ifndef NOICP
    if (stayflg || what == W_COMMAND)
      break;
    else
#endif /* NOICP */
      doexit(GOOD_EXIT,-1);

#ifndef NOXFER
case 'a':                               /* "as" */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -a");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing name in -a");
    }
    cmarg2 = *xargv;
    debug(F111,"doarg a",cmarg2,xargc);
    break;
#endif /* NOXFER */

#ifndef NOICP
case 'Y':                               /* No initialization file */
    noinit = 1;
    break;

case 'y':                               /* Alternate init-file name */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -y");
    }
    xargv++, xargc--;
    if (xargc < 1) {
        XFATAL("missing filename in -y");
    }
    /* strcpy(kermrc,*xargv); ...this was already done in prescan()... */
    break;
#endif /* NOICP */

#ifndef NOXFER
case 'I':                               /* Assume we have an "Internet" */
    reliable = 1;                       /* or other reliable connection */
    xreliable = 1;
    setreliable = 1;

    /* I'm not so sure about this -- what about VMS? (next comment) */
    clearrq = 1;                        /* therefore the channel is clear */

#ifndef VMS
/*
  Since this can trigger full control-character unprefixing, we need to
  ensure that our terminal or pty driver is not doing Xon/Xoff; otherwise
  we can become deadlocked the first time we receive a file that contains
  Xoff.
*/
    flow = FLO_NONE;
#endif /* VMS */
    break;
#endif /* NOXFER */

#ifndef NOLOCAL
case 'l':                               /* SET LINE */
#ifdef NETCONN
#ifdef ANYX25
case 'X':                               /* SET HOST to X.25 address */
#ifdef SUNX25
case 'Z':                               /* SET HOST to X.25 file descriptor */
#endif /* SUNX25 */
#endif /* ANYX25 */
#ifdef TCPSOCKET
case 'J':
case 'j':                               /* SET HOST (TCP/IP socket) */
#endif /* TCPSOCKET */
#endif /* NETCONN */
#ifndef NOXFER
    if (x == 'j' || x == 'J' || x == 'X' || x == 'Z') {
        reliable = 1;                   /* or other reliable connection */
        xreliable = 1;
        setreliable = 1;
    }
#endif /* NOXFER */
    network = 0;
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -l or -j");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("communication line device name missing");
    }

#ifdef NETCONN
    if (x == 'J') {
        cflg    = 1;                    /* Connect */
        stayflg = 1;                    /* Stay */
        tn_exit = 1;                    /* Telnet-like exit condition */
        exitonclose = 1;
    }
#endif /* NETCONN */
    ckstrncpy(ttname,*xargv,TTNAMLEN+1);
    local = (strcmp(ttname,CTTNAM) != 0);
    if (local && strcmp(ttname,"0") == 0)
      local = 0;
/*
  NOTE: We really do not need to call ttopen here, since it should be called
  again later, automatically, when we first try to condition the device via
  ttpkt or ttvt.  Calling ttopen here has the bad side effect of making the
  order of the -b and -l options significant, but order of command-line options
  should not matter.  However, the network cases immediately below complicate
  matters a bit, so we'll settle this in a future edit.
*/
    if (x == 'l') {
        if (ttopen(ttname,&local,mdmtyp,0) < 0) {
            XFATAL("can't open device");
        }
#ifdef CKLOGDIAL
        dologline();
#endif /* CKLOGDIAL */
        debug(F101,"doarg speed","",speed);
        cxtype = (mdmtyp > 0) ? CXT_MODEM : CXT_DIRECT;
        speed = ttgspd();               /* Get the speed. */
        setflow();                      /* Do something about flow control. */
#ifndef NOSPL
        if (local) {
            if (nmac) {                 /* Any macros defined? */
                int k;                  /* Yes */
                k = mlook(mactab,"on_open",nmac);       /* Look this up */
                if (k >= 0) {                   /* If found, */
                    if (dodo(k,ttname,0) > -1)  /* set it up, */
                      parser(1);                        /* and execute it */
                }
            }
        }
#endif /* NOSPL */

#ifdef NETCONN
    } else {
        if (x == 'j' || x == 'J') {     /* IP network host name */
            char * s = line;
            char * service = tmpbuf;
            if (xargc > 0) {            /* Check if it's followed by */
                /* A service name or number */
                if (*(xargv+1) && *(*(xargv+1)) != '-') {
                    xargv++, xargc--;
                    strcat(ttname,":");
                    strcat(ttname,*xargv);
                }
            }
            nettype = NET_TCPB;
            mdmtyp = -nettype;          /* Perhaps already set in init file */
            telnetfd = 1;               /* Or maybe an open file descriptor */

            ckstrncpy(line, ttname, YYBUFLEN); /* Working copy of the name */
            for (s = line; *s != '\0' && *s != ':'; s++); /* and service */
            if (*s) {
                *s++ = '\0';
                ckstrncpy(service, s, YYBUFLEN);
            } else *service = '\0';
            s = line;
#ifndef NODIAL
#ifndef NOICP
            /* Look up in network directory */
            x = 0;
            if (*s == '=') {            /* If number starts with = sign */
                s++;                    /* strip it */
                while (*s == SP)        /* and also any leading spaces */
                  s++;
                ckstrncpy(line,s,YYBUFLEN); /* Do this again. */
                nhcount = 0;
            } else if (!isdigit(line[0])) {
/*
  nnetdir will be greater than 0 if the init file has been processed and it
  contained a SET NETWORK DIRECTORY command.
*/
                xx = 0;                 /* Initialize this */
                if (nnetdir > 0)        /* If there is a directory... */
                  xx = lunet(line);     /* Look up the name */
                else                    /* If no directory */
                  nhcount = 0;          /* we didn't find anything there */
                if (xx < 0) {           /* Lookup error: */
                    sprintf(tmpbuf,
                            "?Fatal network directory lookup error - %s\n",
                            line
                            );
                    XFATAL(tmpbuf);
                }
            }
#endif /* NOICP */
#endif /* NODIAL */
            /* Add service to line specification for ttopen() */
            if (*service) {             /* There is a service specified */
                strcat(line, ":");
                strcat(line, service);
                ttnproto = NP_DEFAULT;
            } else {
                strcat(line, ":telnet");
                ttnproto = NP_TELNET;
            }

#ifndef NOICP
#ifndef NODIAL
            if ((nhcount > 1) && !quiet && !backgrd) {
                printf("%d entr%s found for \"%s\"%s\n",
                       nhcount,
                       (nhcount == 1) ? "y" : "ies",
                       s,
                       (nhcount > 0) ? ":" : "."
                       );
                for (i = 0; i < nhcount; i++)
                  printf("%3d. %s %-12s => %s\n",
                         i+1, n_name, nh_p2[i], nh_p[i]
                         );
            }
            if (nhcount == 0)
              n = 1;
            else
              n = nhcount;
#else
            n = 1;
            nhcount = 0;
#endif /* NODIAL */
            for (i = 0; i < n; i++) {
#ifndef NODIAL
                if (nhcount >= 1) {
                    /* Copy the current entry to line */
                    ckstrncpy(line,nh_p[i],LINBUFSIZ);
                    /* Check to see if the network entry contains a service */
                    for (s = line ; (*s != '\0') && (*s != ':'); s++)
                      ;
                    /* If directory does not have a service ... */
                    if (!*s && *service) { /* and the user specified one */
                        strcat(line, ":");
                        strcat(line, service);
                    }
                    if (lookup(netcmd,nh_p2[i],nnets,&z) > -1) {
                        mdmtyp = 0 - netcmd[z].kwval;
                    } else {
                        printf("Error - network type \"%s\" not supported\n",
                               nh_p2[i]
                               );
                        continue;
                    }
                }
#endif /* NODIAL */
            }
#endif /* NOICP */
            ckstrncpy(ttname, line,TTNAMLEN+1);
            cxtype = CXT_TCPIP;         /* Set connection type */
            setflow();                  /* Set appropriate flow control. */
#ifdef SUNX25
        } else if (x == 'X') {          /* X.25 address */
            nettype = NET_SX25;
            mdmtyp = -nettype;
        } else if (x == 'Z') {          /* Open X.25 file descriptor */
            nettype = NET_SX25;
            mdmtyp = -nettype;
            x25fd = 1;
#endif /* SUNX25 */
#ifdef STRATUSX25
        } else if (x == 'X') {          /* X.25 address */
            nettype = NET_VX25;
            mdmtyp = -nettype;
#endif /* STRATUSX25 */
#ifdef IBMX25
        } else if (x == 'X') {          /* X.25 address */
            nettype = NET_IX25;
            mdmtyp = -nettype;
#endif /* IBMX25 */
#ifdef HPX25
        } else if (x == 'X') {          /* X.25 address */
            nettype = NET_HX25;
            mdmtyp = -nettype;
#endif /* HPX25 */
        }
        if (ttopen(ttname,&local,mdmtyp,0) < 0) {
            XFATAL("can't open host connection");
        }
        network = 1;
#ifdef CKLOGDIAL
        dolognet();
#endif /* CKLOGDIAL */
        cxtype = CXT_X25;               /* Set connection type */
        setflow();                      /* Set appropriate flow control. */
#ifndef NOSPL
        if (local) {
            if (nmac) {                 /* Any macros defined? */
                int k;                  /* Yes */
                k = mlook(mactab,"on_open",nmac); /* Look this up */
                if (k >= 0) {           /* If found, */
                    if (dodo(k,ttname,0) > -1) /* set it up, */
                      parser(1);        /* and execute it */
                }
            }
        }
#endif /* NOSPL */
#endif /* NETCONN */
    }
    /* add more here -- decnet, etc... */
    haveline = 1;
    break;

#ifdef ANYX25
case 'U':                               /* X.25 call user data */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing call user data string");
    }
    ckstrncpy(udata,*xargv,MAXCUDATA);
    if ((int)strlen(udata) <= MAXCUDATA) {
        cudata = 1;
    } else {
        XFATAL("Invalid call user data");
    }
    break;

case 'o':                               /* X.25 closed user group */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing closed user group index");
    }
    z = atoi(*xargv);                   /* Convert to number */
    if (z >= 0 && z <= 99) {
        closgr = z;
    } else {
        XFATAL("Invalid closed user group index");
    }
    break;

case 'u':                               /* X.25 reverse charge call */
    revcall = 1;
    break;
#endif /* ANYX25 */
#endif /* NOLOCAL */

case 'b':                               /* Bits-per-second for serial device */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing bps");
    }
    zz = atol(*xargv);                  /* Convert to long int */
    i = zz / 10L;
#ifndef NOLOCAL
    if (ttsspd(i) > -1)                 /* Check and set it */
#endif /* NOLOCAL */
      speed = ttgspd();                 /* and read it back. */
#ifndef NOLOCAL
    else {
        XFATAL("unsupported transmission rate");
    }
#endif /* NOLOCAL */
    break;

#ifndef NODIAL
#ifndef NOICP
case 'm':                               /* Modem type */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -m");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("modem type missing");
    }
    y = lookup(mdmtab,*xargv,nmdm,&z);
    if (y < 0) {
        XFATAL("unknown modem type");
    }
    usermdm = 0;
    usermdm = (y == dialudt) ? x : 0;
    initmdm(y);
    break;
#endif /* NOICP */
#endif /* NODIAL */

#ifndef NOXFER
case 'e':                               /* Extended packet length */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -e");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing length");
    }
    z = atoi(*xargv);                   /* Convert to number */
    if (z > 10 && z <= maxrps) {
        rpsiz = urpsiz = z;
        if (z > 94) rpsiz = 94;         /* Fallback if other Kermit can't */
    } else {
        XFATAL("Unsupported packet length");
    }
    break;

case 'v':                               /* Vindow size */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing or bad window size");
    }
    z = atoi(*xargv);                   /* Convert to number */
    if (z < 32) {                       /* If in range */
        wslotr = z;                     /* set it */
        if (z > 1) swcapr = 1;          /* Set capas bit if windowing */
    } else {
        XFATAL("Unsupported packet length");
    }
    break;
#endif /* NOXFER */

case 'i':                               /* Treat files as binary */
    binary = XYFT_B;
    xfermode = XMODE_M;			/* Transfer mode manual */
#ifdef PATTERNS
    patterns = 0;
#endif /* PATTERNS */
    break;

#ifndef NOXFER
case 'w':                               /* Writeover */
    ckwarn = 0;
    fncact = XYFX_X;
    break;
#endif /* NOXFER */

case 'q':                               /* Quiet */
    quiet = 1;
    break;

#ifdef DEBUG
case 'd':                               /* DEBUG */
    break;                              /* Handled in prescan() */
#endif /* DEBUG */

case '0': {                             /* In the middle */
    extern int tt_escape, lscapr;
    tt_escape = 0;                      /* No escape character */
    flow = 0;                           /* No Xon/Xoff (what about hwfc?) */
#ifndef NOXFER
    lscapr = 0;                         /* No locking shifts */
#endif /* NOXFER */
#ifdef CK_APC
    {
        extern int apcstatus;           /* No APCs */
        apcstatus = APC_OFF;
    }
#endif /* CK_APC */
#ifdef CK_AUTODL
    {                                   /* No autodownload */
        extern int autodl;
        autodl = 0;
    }
#endif /* CK_AUTODL */
#ifndef NOCSETS
    {
        extern int tcsr, tcsl;          /* No character-set translation */
        tcsr = 0;
        tcsl = tcsr;                    /* Make these equal */
    }
#endif /* NOCSETS */
#ifdef TNCODE
    TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = TN_NG_RF;
    TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = TN_NG_RF;
    TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = TN_NG_RF;
    TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = TN_NG_RF;
#endif /* TNCODE */
}
/* Fall thru... */

case '8':                               /* 8-bit clean */
    parity = 0;
    cmdmsk = 0xff;
    cmask = 0xff;
    break;

case 'V': {
    extern int xfermode;
#ifdef PATTERNS
    extern int patterns;
    patterns = 0;                       /* No patterns */
#endif /* PATTERNS */
    xfermode = XMODE_M;                 /* Manual transfer mode */
    break;
}

case 'p':                               /* SET PARITY */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing parity");
    }
    switch(x = **xargv) {
        case 'e':
        case 'o':
        case 'm':
        case 's': parity = x; break;
        case 'n': parity = 0; break;
        default:  { XFATAL("invalid parity"); }
        }
    break;

case 't':                               /* Line turnaround handshake */
    turn = 1;
    turnch = XON;                       /* XON is turnaround character */
    duplex = 1;                         /* Half duplex */
    flow = 0;                           /* No flow control */
    break;

case 'B':
    bgset = 1;                          /* Force background (batch) */
    backgrd = 1;
    break;

case 'z':                               /* Force foreground */
    bgset = 0;
    backgrd = 0;
    break;

#ifndef NOXFER
#ifdef RECURSIVE
case 'L':
    recursive = 2;
    fnspath = PATH_REL;
    break;
#endif /* RECURSIVE */
#endif /* NOXFER */

#ifndef NOSPL
case 'M':                               /* My User Name */
    /* Already done in prescan() */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
    }
#ifdef COMMENT
    if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
    }
#ifdef IKSD
    if (!inserver)
#endif /* IKSD */
      ckstrncpy(uidbuf,tmpusrid,UIDBUFLEN);
#endif /* COMMENT */
    break;
#endif /* NOSPL */

#ifdef CK_NETBIOS
case 'N':                              /* NetBios Adapter Number follows */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -N");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing NetBios Adapter number");
    }
    if ((strlen(*xargv) != 1) ||
        (*xargv)[0] != 'X' &&
        (atoi(*xargv) < 0) &&
         (atoi(*xargv) > 9)) {
        XFATAL("Invalid NetBios Adapter - Adapters 0 to 9 are valid");
    }
    break;
#endif /* CK_NETBIOS */

#ifdef NETCONN
case 'F':
    network = 1;
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -F");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("network file descriptor missing");
    }
    ckstrncpy(ttname,*xargv,TTNAMLEN+1);
    nettype = NET_TCPB;
    mdmtyp = -nettype;
    telnetfd = 1;
    local = 1;
    break;
#endif /* NETCONN */

#ifdef COMMENT
#ifdef OS2PM
case 'P':                               /* OS/2 Presentation Manager */
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -P");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("pipe data missing");
    }
    pipedata = *xargv;
    break;
#endif /* OS2PM */
#else
case 'P':				/* Filenames literal */
    fncnv  = XYFN_L;
    f_save = XYFN_L;
    break;
#endif /* COMMENT */

#ifndef NOICP
case 'H':
    noherald = 1;
    break;
#endif /* NOICP */

#ifdef OS2
case 'W':
    if (*(xp+1)) {
        XFATAL("invalid argument bundling after -W");
    }
    xargv++, xargc--;
    if ((xargc < 1)) { /* could be negative */
        XFATAL("Window handle missing");
    }
    xargv++, xargc--;
    if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("Kermit Instance missing");
    }
    /* Action done in prescan */
    break;

case '#':                               /* K95 stdio threads */
    xargv++, xargc--;                   /* Skip past argument */
    break;                              /* Action done in prescan */
#endif /* OS2 */

default:
    fatal2(*xargv,
#ifdef NT
                   "invalid command-line option, type \"k95 -h\" for help"
#else
#ifdef OS2
                   "invalid command-line option, type \"k2 -h\" for help"
#else
                   "invalid command-line option, type \"kermit -h\" for help"
#endif /* OS2 */
#endif /* NT */
           );
        }
    if (!xp) break;
    x = *++xp;                          /* See if options are bundled */
    }
    return(0);
}
#else /* No command-line interface... */

extern int xargc;
int
cmdlin() {
    if (xargc > 1) {
        XFATAL("Sorry, command-line options disabled.");
    }
}
#endif /* NOCMDL */
