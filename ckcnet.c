char *cknetv = "Network support, 6.0.078, 6 Sep 1996";

/*  C K C N E T  --  Network support  */

/*
  NOTE TO CONTRIBUTORS: This file, and all the other shared (ckc and cku)
  C-Kermit source files, must be compatible with C preprocessors that support
  only #ifdef, #else, #endif, #define, and #undef.  Please do not use #if,
  logical operators, or other preprocessor features in any of the portable
  C-Kermit modules.  You can, of course, use these constructions in
  system-specific modules when you know they are supported.  Also, don't use
  any ANSI C constructs except with #ifdef CK_ANSIC..#endif.
*/

/*
  Authors:

  Frank da Cruz (fdc@columbia.edu),
    Columbia University Academic Information Systems, New York City.
  netopen() routine for TCP/IP originally by Ken Yap, Rochester University
    (ken@cs.rochester.edu) (no longer at that address).
  Jeffrey E Altman (jaltman@columbia.edu) -- OS/2 & Windows, etc.
  Missing pieces for Excelan sockets library from William Bader.
  TELNET protocol by Frank da Cruz and Jeffrey Altman.
  TGV MultiNet code by Frank da Cruz.
  MultiNet code adapted to WIN/TCP by Ray Hunter of TWG.
  MultiNet code adapted to DEC TCP/IP by Lee Tibbert of DEC and Frank da Cruz.
  TCP/IP support adapted to IBM TCP/IP 1.2.1,2.0 for OS/2 by Kai Uwe Rommel.
  CMU-OpenVMS/IP modifications by Mike O'Malley, Digital (DEC).
  X.25 support by Marcello Frutig, Catholic University,
    Rio de Janeiro, Brazil (frutig@rnp.impa.br) with fixes from:
    Stefaan Eeckels, Eurokom, Luxembourg;
    David Lane, Status Computer.
  Other contributions as indicated below.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/
#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcker.h"
#ifdef I386IX				/* Has to come before ckcnet.h in */
#include <errno.h>			/* this version, but after in others */
#endif /* I386IX */
#include "ckcnet.h"

#ifdef NETCONN
/* Don't need these if there is no network support. */

#ifdef CK_SOCKS				/* SOCKS Internet relay package */
#define accept  Raccept
#define bind    Rbind
#define connect Rconnect
#define getsockname Rgetsockname
#define listen Rlisten
#endif /* CK_SOCKS */

#ifdef DEC_TCPIP
#include <time.h>
#include <inet.h>
#endif /* DEC_TCPIP */

#ifdef CMU_TCPIP
#include <time.h>
#endif /* CMU_TCPIP */

#ifdef WINTCP

#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
/*
  The WIN/TCP code path is the same as that for MultiNet.
  Only the routine names have changed ...
*/
#define socket_read 	netread
#define socket_ioctl	ioctl
#define socket_write 	netwrite
#define socket_close 	netclose

#ifdef OLD_TWG                         /* some routines have evolved */
	extern int vmserrno, uerrno;
#define socket_errno    uerrno
#define socket_perror   perror         /* which uses errno, not uerrno! */
#else
#define socket_errno    errno
#define socket_perror   win$perror
#endif /* OLD_TWG */

#else /* Not WINTCP */

#ifndef I386IX
#include <errno.h>			/* Already included above */
#endif /* I386IX */

#include <signal.h>			/* Everybody needs this */

#ifdef ZILOG				/* Zilog has different name for this */
#include <setret.h>
#else /* !ZILOG */
#include <setjmp.h>
#ifdef CK_POSIX_SIG			/* POSIX signal handling */
#endif /* CK_POSIX_SIG */
#endif /* ZILOG */

#endif /* WINTCP */

#ifdef datageneral			/* Data General AOS/VS */
#include <:usr:include:vs_tcp_errno.h>
#include <:usr:include:sys:vs_tcp_types.h>
#ifdef SELECT
/*
  NOTE: This can be compiled and linked OK with SELECT defined
  but it doesn't work at all.  Anybody who cares and knows how
  to fix it, feel free.
*/
#include <:usr:include:sys:vs_tcp_time.h>
#endif /* SELECT */
#include <:usr:include:sys:socket.h>
#include <:usr:include:netinet:in.h>
#include <:usr:include:netdb.h>
#endif /* datageneral */

extern
#ifdef OS2
SIGTYP (* volatile saval)();		/* For saving alarm handler */
#else /* OS2 */
SIGTYP (*saval)();			/* For saving alarm handler */
#endif /* OS2 */

_PROTOTYP( VOID bgchk, (void) );
_PROTOTYP( static VOID tn_debug, (char *) );

#ifdef RLOGCODE
#ifdef TCPIPLIB
_PROTOTYP( static VOID rlog_oob, (CHAR *, int) );
#else /* TCPIPLIB */
_PROTOTYP( static SIGTYP rlogoobh, ( int ) );
#endif /* TCPIPLIB */
_PROTOTYP( static int rlog_ini, (VOID) );
int rlog_mode = RL_COOKED;
int rlog_stopped = 0;
#endif /* RLOGCODE */

/* NAWS state - used in both TELNET and RLOGIN */
int nawsflg = 0;

extern int				/* External variables */
  duplex, debses, seslog, sessft,
  ttyfd, quiet, msgflg, what, nettype, ttmdm;

#ifdef DEBUG
extern int deblog;
#else
#define deblog 0
#endif /* DEBUG */

#ifdef OS2
extern int tt_rows[], tt_cols[];
extern int tt_status;
#else /* OS2 */
extern int tt_rows, tt_cols;		/* Everybody has this */
#endif /* OS2 */


#ifdef CK_TTGWSIZ
_PROTOTYP( int ttgwsiz, (void) );
#endif /* CK_TTGWSIZ */
#ifdef CK_NAWS				/* Negotiate About Window Size */
_PROTOTYP( int tn_snaws, (void) );
#ifdef RLOGCODE
#ifndef OS2
_PROTOTYP(static int rlog_naws, (void) );
#else
_PROTOTYP( int rlog_naws, (void) );
#endif /* OS2 */
#endif /* RLOGCODE */
#endif /* CK_NAWS */

#ifdef OS2				/* For terminal type name string */
#include "ckuusr.h"
#ifndef NT
#include <os2.h>
#endif /* NT */
#include "ckocon.h"
extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
extern char ttname[];
extern int  scrninitialized[];
#endif /* OS2 */

#include "ckcsig.h"

#ifndef OS2				/* For timeout longjumps */
static ckjmpbuf njbuf;
#endif /* OS2 */

#define NAMECPYL 100			/* Local copy of hostname */
#ifndef OS2                     
static					/* OS2 needs access in ckonet.c */
#endif /* OS2 */
char namecopy[NAMECPYL];        

char ipaddr[20] = { '\0' };		/* Global copy of IP address */
char myipaddr[20] = { '\0' };		/* Global copy of my IP address */

#endif /* NETCONN */

int ttnet = NET_NONE;			/* Network type */
int ttnproto = NP_NONE;			/* Network virtual terminal protocol */
int tn_init = 0;			/* Telnet protocol initialized flag */
int tn_exit = 0;			/* Exit on disconnect */
int tn_duplex = 1;			/* Initial echo status */
char *tn_term = NULL;			/* Terminal type override */
int tn_nlm = TNL_CRLF;			/* Telnet CR -> CR LF mode */
int tn_binary = TN_BM_AC;		/* Binary negotiation accepted */
int tn_b_nlm = TNL_CR;			/* Telnet Binary CR RAW mode */
int tn_b_meu = 0;			/* Telnet Binary ME means U too */
int tn_b_ume = 0;			/* Telnet Binary U means ME too */
#ifdef OS2
int ttnum = -1;				/* Last Telnet Terminal Type sent */
int ttnumend = 0;			/* Has end of list been found */
#endif /* OS2 */

#ifdef TNCODE
#ifdef CK_ENVIRONMENT
static char tn_msg[1024];		/* For debugging */
static char hexbuf[1024];
#else /* CK_ENVIRONMENT */
static char tn_msg[128];		/* For debugging */
static char hexbuf[6];
#endif /* CK_ENVIRONMENT */
#endif /* TNCODE */

#ifdef NT
extern int WSASafeToCancel;
#endif /* NT */

#ifndef NOTCPOPTS

/* Skip all this if NOTCPOPTS specified. */

#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
int tcp_nodelay = 0;			/* Nagle algorithm TCP_NODELAY */
#endif /* TCP_NODELAY */

#ifdef SO_LINGER
int tcp_linger  = 0;			/* SO_LINGER */
int tcp_linger_tmo = 0;			/* SO_LINGER timeout */
#endif /* SO_LINGER */

#ifdef HPUX				/* But the data structures */
#ifndef HPUX8				/* needed for linger are not */
#ifndef HPUX9				/* defined in HP-UX versions */
#ifndef HPUX10				/* prior to 8.00. */
#ifdef SO_LINGER
#undef SO_LINGER
#endif /* SO_LINGER */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* HPUX8 */
#endif /* HPUX */

#ifdef SO_SNDBUF 
int tcp_sendbuf = -1;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
int tcp_recvbuf = -1;
#endif /* SO_RCVBUF */
#ifdef SO_KEEPALIVE 
int tcp_keepalive = 1;
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */

#endif /* NOTCPOPTS */

#ifndef NETCONN
/*
  Network support not defined.
  Dummy functions here in case #ifdef's forgotten elsewhere.
*/
int					/* Open network connection */
netopen(name, lcl, nett) char *name; int *lcl, nett; {
    return(-1);
}
int					/* Close network connection */
netclos() {
    return(-1);
}
int					/* Check network input buffer */
nettchk() {
    return(-1);
}
int					/* Flush network input buffer */
netflui() {
    return(-1);
}
int					/* Send network BREAK */
netbreak() {
    return(-1);
}
int					/* Input character from network */
netinc(timo) int timo; {
    return(-1);
}
int					/* Output character to network */
#ifdef CK_ANSIC
nettoc(char c)
#else
nettoc(c) char c;
#endif /* CK_ANSIC */
/* nettoc */ {
    return(-1);
}
int
nettol(s,n) char *s; int n; {
    return(-1);
}

#else /* NETCONN is defined (rest of this module...) */

#ifdef VMS
/*
  In edit 190, we moved tn_ini() to be called from within netopen().
  But tn_ini() calls ttol(), and ttol() checks to see if it's a net 
  connection, but the flag for that isn't set until after netopen()
  is finished.  Since, in this module, we are always doing network
  output anyway, we just call nettol() directly, instead of going thru
  ttol().  Only needed for VMS, since UNIX, AOS/VS, and VOS can handle
  net connections just like regular connections in ttol(), and OS/2
  has a special routine for this.
*/
#define ttol nettol
#endif /* VMS */

extern int tn_binary;			/* Binary mode enabled */
int me_binary = 0;			/* I'm not in TELNET binary mode */
int u_binary = 0;			/* You're not in TELNET binary mode */

int tcpsrfd = -1;

#ifdef TCPSOCKET
#ifndef OS2
#ifndef NOLISTEN			/* For incoming connections */

#ifndef INADDR_ANY
#define INADDR_ANY 0           
#endif /* INADDR_ANY */

_PROTOTYP( int ttbufr, ( VOID ) );
_PROTOTYP( int tcpsrv_open, (char *, int *, int, int ) );

static unsigned short tcpsrv_port = 0;

#endif /* NOLISTEN */
#endif /* OS2 */
#endif /* TCPSOCKET */

#ifndef NOSIGWINCH
#ifdef CK_NAWS				/* Window size business */
#ifdef SIGWINCH
#ifdef UNIX
static int sw_armed = 0;		/* SIGWINCH armed flag */
SIGTYP
winchh(foo) int foo; {
    int x;
    debug(F100,"SIGWINCH caught","",0);
    signal(SIGWINCH,winchh);		/* Re-arm the signal */
    if (ttyfd == -1)
      return;
    x = ttgwsiz();			/* Get new window size */
/*
  This should be OK.  It might seem that sending this from
  interrupt level could interfere with another TELNET IAC string
  that was in the process of being sent.  But we always send
  TELNET strings with a single write(), which should prevent mixups.
*/
    if (x > 0 && tt_rows > 0 && tt_cols > 0) {
	tn_snaws();
#ifdef RLOGCODE
	rlog_naws();
#endif /* RLOGCODE */
    }
    return;
}
#endif /* UNIX */
#endif /* SIGWINCH */
#endif /* CK_NAWS */
#endif /* NOSIGWINCH */

/*
  TCPIPLIB means use separate socket calls for i/o, while on UNIX the
  normal file system calls are used for TCP/IP sockets too.
  Means "DEC_TCPIP or MULTINET or WINTCP or OS2" (defined in ckcnet.h).
*/

#ifdef TCPIPLIB

/* For buffered network reads... */
/*
  If the buffering code is written right, it shouldn't matter
  how long this buffer is.
*/
#ifdef OS2
#define TTIBUFL 32767
#else /* OS2 */
#define TTIBUFL 8191			/* Let's use 8K. */
#endif /* OS2 */

CHAR ttibuf[TTIBUFL+1];

/*
  select() is used in preference to alarm()/signal(), but different systems
  use different forms of select()...
*/
#ifndef NOSELECT         /* Option to override BSDSELECT */
#ifdef BELLV10
/*
  Note: Although BELLV10 does have TCP/IP support, and does use the unique
  form of select() that is evident in this module (and in ckutio.c), it does
  not have a sockets library and so we can't build Kermit TCP/IP support for
  it.  For this, somebody would have to write TCP/IP streams code.
*/
#define BELLSELECT
#ifndef FD_SETSIZE
#define FD_SETSIZE 128
#endif /* FD_SETSIZE */
#else
#ifdef WINTCP				/* VMS with Wollongong WIN/TCP */
#ifndef OLD_TWG				/* TWG 3.2 has only select(read) */
#define BSDSELECT
#endif /* OLD_TWG */
#else
#ifdef CMU_TCPIP			/* LIBCMU can do select */
#define BSDSELECT
#else
#ifdef DEC_TCPIP
#define BSDSELECT
#else
#ifdef OS2				/* OS/2 with TCP/IP */
#ifdef NT
#define BSDSELECT
#else /* NT */
#define IBMSELECT
#endif /* NT */
#endif /* OS2 */
#endif /* DEC_TCPIP */
#endif /* CMU_TCPIP */
#endif /* WINTCP */
#endif /* BELLV10 */
#endif /* NOSELECT */
/*
  Others (TGV, TCPware, ...) use alarm()/signal().  The BSDSELECT case does not
  compile at all; the IBMSELECT case compiles and links but crashes at runtime.
  NOTE: If any of these can be converted to select(), they should be for two
  reasons: (1) It's a lot faster; (2) certain sockets libraries do not like
  their socket_read() calls to be interrupted; subsequent socket_read()'s tend
  to fail with EBUSY.  This happened in the UCX case before it was converted
  to use select().
*/
#ifndef VMS
static					/* These are used in CKVTIO.C */
#endif /* VMS */
int
  ttibp = 0,
  ttibn = 0;
/*
  Read bytes from network into internal buffer ttibuf[].
  To be called when input buffer is empty, i.e. when ttibn == 0.

  Other network reading routines, like ttinc, ttinl, ttxin, should check the
  internal buffer first, and call this routine for a refill if necessary.

  Returns -1 on error, 0 if nothing happens.  When data is read successfully,
  returns number of bytes read, and sets global ttibn to that number and
  ttibp (the buffer pointer) to zero.
*/
_PROTOTYP( int ttbufr, ( VOID ) );
int
ttbufr() {				/* TT Buffer Read */
    int count;

    if (ttnet != NET_TCPB)		/* First make sure current net is */
      return(-1);			/* TCP/IP; if not, do nothing. */

    if (ttibn > 0)			/* Our internal buffer is not empty, */
      return(ttibn);			/* so keep using it. */
#ifdef WINTCP
    count = 512;			/* This works for WIN/TCP */
#else
#ifdef DEC_TCPIP
    count = 512;			/* UCX */
#else
#ifdef OS2
    count = TTIBUFL;
#else					/* Multinet, etc. */
    count = ttchk();			/* Check network input buffer, */
    if (ttibn > 0) return(ttibn);	/* which can put a char there! */
    if (count < 0)			/* Read error - connection closed */
      return(-2);
    else if (count > TTIBUFL)		/* Too many to read */
      count = TTIBUFL;
    else if (count == 0)		/* None, so force blocking read */
      count = 1;
#endif /* OS2 */
#endif /* DEC_TCPIP */
#endif /* WINTCP */
    debug(F101,"ttbufr count 1","",count);

#ifdef COMMENT
/*
 This is for nonblocking reads, which we don't do any more.  This code didn't
 work anyway, in the sense that a broken connection was never sensed.
*/
    if ((count = socket_read(ttyfd,ttibuf,count)) < 1) {
	if (count == -1 && socket_errno == EWOULDBLOCK) {
	    debug(F100,"ttbufr finds nothing","",0);
	    return(0);
	} else {
	    debug(F101,"ttbufr socket_read error","",socket_errno);
	    return(-1);
	}

    } else if (count == 0) {
	debug(F100,"ttbufr socket eof","",0);		
	return(-1);
    }
#else /* COMMENT */

/* This is for blocking reads */

#ifndef VMS
#ifdef SO_OOBINLINE
    {
	int outofband = 0;
#ifdef BELLSELECT
	if (select(128, NULL, NULL, efds, 0) > 0 && FD_ISSET(ttyfd, efds))
	  outofband = 1;
#else
#ifdef BSDSELECT
	fd_set efds;
	struct timeval tv;
	FD_ZERO(&efds);
	FD_SET(ttyfd, &efds);
	tv.tv_sec  = tv.tv_usec = 0L;
	debug(F100,"Out-of-Band BSDSELECT","",0);
#ifdef NT
	WSASafeToCancel = 1;
#endif /* NT */
	if (select(FD_SETSIZE, NULL, NULL, &efds, &tv) > 0 &&
	    FD_ISSET(ttyfd, &efds))
	  outofband = 1;
#ifdef NT
	WSASafeToCancel = 0;
#endif /* NT */
#else /* !BSDSELECT */
#ifdef IBMSELECT
/* Was used by OS/2, currently not used, but might come in handy some day... */
/* ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set */
/* and timeval stuff since this is the only place where it is used. */
	int socket = ttyfd;
	debug(F100,"Out-of-Band IBMSELECT","",0);
	if ((select(&socket, 0, 0, 1, 0L) == 1) && (socket == ttyfd))
	  outofband = 1;
#else /* !IBMSELECT */
/*
  If we can't use select(), then we use the regular alarm()/signal()
  timeout mechanism.
*/
      debug(F101,"Out-of-Band data not supported","",0);
      outofband = 0;

#endif /* IBMSELECT */
#endif /* BSDSELECT */
#endif /* BELLSELECT */
      if (outofband) {
         /* Get the Urgent Data */
         /* if OOBINLINE is disabled this should be only a single byte      */
         /* MS Winsock has a bug in Windows 95.  Extra bytes are delivered  */
         /* That were never sent.                                           */
	  if ((count = socket_recv(ttyfd,ttibuf,count,MSG_OOB)) <= 0) {
	      int s_errno = socket_errno;
	      debug(F101, "ttbufr socket_recv MSG_OOB","",count);
	      debug(F101, "ttbufr socket_errno","",s_errno);
#ifndef OS2
	      netclos();			/* *** *** */  
	      return(-2);
#else /* OS2 */
	      if (count == 0) {
		  debug(F100,"ttbufr Closing Connection","",0);
		  ttclos(0);		/* if the connection was  */
		  if (ttname[0] == '*') { /* incoming, wait for another */
		      int local;
		      os2_netopen(ttname,&local,ttnet);
		      debug(F101,
			    "ttbufr returns zero - try again immediately",
			    "",
			    0
			    );
		      return 0;		/* try again immediately */
		  } else {
		      debug(F101, "ttbufr returns hard error","",-3);
		      return -3;	/* return a hard error    */
		  }
	      } else {
#ifdef NT
		  if (s_errno == WSAETIMEDOUT)
		    debug(F100,"WSAETIMEDOUT","",0);
#endif /* NT */ 
		  switch (s_errno) {
#ifdef NT
		    case WSAETIMEDOUT:
#else
		    case SOCETIMEDOUT:    
		    case SOCETIMEDOUT - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr ETIMEDOUT","",0);
		      return(-1);
#ifdef NT
		    case WSAECONNRESET:
#else /* NT */
		    case SOCECONNRESET:
		    case SOCECONNRESET - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr ECONRESET","",0);
		      netclos();	/* *** *** */
		      return(-2);	/* Connection is broken. */
#ifdef NT
		    case WSAECONNABORTED:
#else /* NT */
		    case SOCECONNABORTED:
		    case SOCECONNABORTED - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr ECONNABORTED","",0);
		      netclos();	/* *** *** */
		      return(-2);	/* Connection is broken. */
#ifdef NT
		    case WSAENETRESET:  
#else /* NT */
		    case SOCENETRESET:
		    case SOCENETRESET - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr ENETRESET","",0);
		      netclos();	/* *** *** */
		      return(-2);	/* Connection is broken. */
#ifdef NT
		    case WSAENOTCONN:
#else /* NT */
		    case SOCENOTCONN:
		    case SOCENOTCONN - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr ENOTCONN","",0);
		      netclos();	/* *** *** */
		      return(-2);	/* Connection is broken. */
#ifdef NT
		    case WSAEWOULDBLOCK:
#else
		    case SOCEWOULDBLOCK:
		    case SOCEWOULDBLOCK - SOCBASEERR:
#endif /* NT */
		      debug(F100,"ttbufr EWOULDBLOCK","",0);
		      count = 1;
		      break;
#ifdef NT
		    case WSAEINVAL:
#else /* NT */
		    case SOCEINVAL:
		    case SOCEINVAL - SOCBASEERR:
#endif /* NT */
                      case 0:
                      case 23: /* ??? */
		      /* These appear in OS/2 - don't know why   */
		      /* ignore it and read as normal data       */
		      /* and break, then we will attempt to read */
		      /* the port using normal read() techniques */
		      debug(F100,"ttbufr handing as in-band data","",0);
		      count = 1;
		      break;

		    default:
		      debug(F101, "ttbufr Unknown Error ","",s_errno);
		      netclos();
		      return -2;	/* Return a hard error */
		  }
	      }   
#endif /* OS2 */
	  } else {			/* we got out-of-band data */
	      debug(F111,"ttbufr out-of-band chars","",count);
#ifdef RLOGCODE				/* blah */
	      if (ttnproto == NP_RLOGIN) {
		  /*
		    When urgent data is read with MSG_OOB and not OOBINLINE
		    then urgent data and normal data are not mixed.  So
		    treat the entire buffer as urgent data.
		  */
		  rlog_oob(ttibuf, count);
		  return ttbufr();
	      } else 
#endif /* RLOGCODE */ /* blah */
#ifdef COMMENT
            /*
	       I haven't written this yet, nor do I know what it should do
	     */
		if (ttnproto == NP_TELNET) {
		    tn_oob();
		    return 0;
		} else 
#endif /* COMMENT */
		  {
		   /* For any protocols we don't have a special out-of-band  */
                   /* handler for, just put the bytes in the normal buffer   */
                   /* and return                                             */

		      ttibp = 0;	/* Reset buffer pointer. */
		      ttibn = count;
#ifdef DEBUG
		      /* Got some bytes. */
		      debug(F101,"ttbufr count 2","",count);
		      if (count > 0) 
			ttibuf[count] = '\0';
		      debug(F111,"ttbufr ttibuf",ttibuf,ttibp);
#endif /* DEBUG */
		      return(ttibn);	/* Return buffer count. */
		  }
	  }
      }
    }
#endif /* SO_OOBINLINE */
#endif /* VMS */

    if ((count = socket_read(ttyfd,ttibuf,count)) <= 0) {
	int s_errno = socket_errno;
	debug(F101,"ttbufr socket_read","",count);
	debug(F101,"ttbufr socket_errno","",s_errno);
#ifndef OS2
	netclos();			/* *** *** */  
	return(-2);
#else /* OS2 */
	if (count == 0) {
	    debug(F100,"ttbufr Closing Connection","",0);
	    ttclos(0);			/* if the connection was  */
	    if (ttname[0] == '*') {	/* incoming, wait for another */
		int local;
		os2_netopen(ttname,&local,ttnet);
		debug(F101,
		      "ttbufr returns zero - try again immediately","",0);
		return 0;		/* try again immediately */
	    } else {
		debug(F101, "ttbufr returns hard error","",-3);
		return -3;		/* return a hard error    */
	    }
	} else {
#ifdef NT
	    if (s_errno == WSAETIMEDOUT)
	      debug(F100, "WSAETIMEDOUT","",0);
#endif /* NT */
	    switch (s_errno) {
#ifdef NT
	      case WSAETIMEDOUT:
#else
	      case SOCETIMEDOUT:
	      case SOCETIMEDOUT - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr ETIMEDOUT","",0);
		return(-1);
#ifdef NT
	      case WSAECONNRESET:
#else /* NT */
	      case SOCECONNRESET:
	      case SOCECONNRESET - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr ECONRESET","",0);
		netclos();			/* *** *** */
		return(-2);			/* Connection is broken. */
#ifdef NT
	      case WSAECONNABORTED:
#else /* NT */
	      case SOCECONNABORTED:
	      case SOCECONNABORTED - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr ECONNABORTED","",0);
		netclos();			/* *** *** */
		return(-2);			/* Connection is broken. */
#ifdef NT
	      case WSAENETRESET:  
#else /* NT */
	      case SOCENETRESET:
	      case SOCENETRESET - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr ENETRESET","",0);
		netclos();			/* *** *** */
		return(-2);			/* Connection is broken. */
#ifdef NT
	      case WSAENOTCONN:
#else /* NT */
	      case SOCENOTCONN:
	      case SOCENOTCONN - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr ENOTCONN","",0);
		netclos();			/* *** *** */
		return(-2);			/* Connection is broken. */
#ifdef NT
	      case WSAEWOULDBLOCK:
#else
	      case SOCEWOULDBLOCK:
	      case SOCEWOULDBLOCK - SOCBASEERR:
#endif /* NT */
		debug(F100,"ttbufr EWOULDBLOCK","",0);
		break;
	    }
	    debug(F101, "ttbufr returns timeout","",-1);
	    return -1;			/* Return a timeout */
	}
#endif /* OS2 */
    }
#endif /* COMMENT */
    else {
	ttibp = 0;			/* Reset buffer pointer. */
	ttibn = count;
#ifdef DEBUG
	debug(F101,"ttbufr count 2","",count); /* Got some bytes. */
	if (count > 0) 
          ttibuf[count] = '\0';
	debug(F111,"ttbufr ttibuf",ttibuf,ttibp);
#endif /* DEBUG */
	return(ttibn);			/* Return buffer count. */
    }
}
#endif /* TCPIPLIB */

#ifndef IBMSELECT
#ifndef BELLSELECT
#ifndef BSDSELECT		/* Non-TCPIPLIB case */
#ifdef SELECT
#define BSDSELECT
#endif /* SELECT */
#endif /* BSDSELECT */
#endif /* BELLSELECT */
#endif /* IBMSELECT */

#define	TELNET_PORT 23	 	/* Should do lookup, but it won't change */
#define RLOGIN_PORT 513
#define KERMIT_PORT 1649

/* This symbol is not known to, e.g., Ultrix 2.0 */
#ifndef TELOPT_TTYPE
#define TELOPT_TTYPE 24
#endif /* TELOPT_TTYPE */

/* This one seems to be not known to UCX */
#ifndef TELOPT_BINARY
#define TELOPT_BINARY 0
#endif /* TELOPT_BINARY */

/* Type needed as 5th argument (length) to get/setsockopt() */

#ifndef SOCKOPT_T
#define SOCKOPT_T int
#ifdef UNIXWARE
#undef SOCKOPT_T
#define SOCKOPT_T size_t
#else
#ifdef VMS
#ifdef DEC_TCPIP
#ifdef __DECC_VER
#undef SOCKOPT_T
#define SOCKOPT_T size_t
#endif /* __DECC_VER */
#endif /* DEC_TCPIP */
#endif /* VMS */
#endif /* UNIXWARE */
#endif /* SOCKOPT_T */

/*
  C-Kermit network open/close functions for BSD-sockets.
  Much of this code shared by SunLink X.25, which also uses the socket library.
*/

/*  N E T O P N  --  Open a network connection.  */
/*
  Call with:
    name of host (or host:service),
    lcl - local-mode flag to be set if this function succeeds,
    network type - value defined in ckunet.h.
*/
#ifdef TCPSOCKET
#ifdef EXCELAN
/*
  Most other BSD sockets implementations define these in header files
  and libraries.
*/
struct servent {
    unsigned short s_port;
};

struct hostent {
    short h_addrtype;
    struct in_addr h_addr;
    int h_length;
};

struct servent *
getservbyname(service, connection) char *service,*connection; {
    static struct servent servrec;
    int port;

    port = 0;
    if (strcmp(service, "telnet") == 0) port = 23;
    else if (strcmp(service, "smtp") == 0) port = 25;
    else port = atoi(service);

    debug(F101,"getservbyname return port ","",port);

    if (port > 0) {
    	servrec.s_port = htons(port);
    	return(&servrec);
    }
    return((struct servent *) NULL);
}

struct hostent *
gethostbyname(hostname) char *hostname; {
    return((struct hostent *) NULL);
}

unsigned long
inet_addr(name) char *name; {
    unsigned long addr;

    addr = rhost(&name);
    debug(F111,"inet_addr ",name,(int)addr);
    return(addr);
}

char *
inet_ntoa(in) struct in_addr in; {
    static char name[80];
    sprintf(name, "%d.%d.%d.%d", in.s_net, in.s_host, in.s_lh, in.s_impno);
    return(name);
}
#else
#ifdef DEC_TCPIP			/* UCX */
#ifndef __DECC				/* VAXC or GCC */

#define getservbyname my_getservbyname

#ifdef CK_ANSIC
globalref int (*C$$GA_UCX_GETSERVBYNAME)();
extern void C$$TRANSLATE();
extern void C$$SOCK_TRANSLATE();
#else
globalref int (*C$$GA_UCX_GETSERVBYNAME)();
extern VOID C$$TRANSLATE();
extern VOID C$$SOCK_TRANSLATE();
#endif /* CK_ANSIC */

struct servent *
my_getservbyname (service, proto) char *service, *proto; {
    static struct servent sent;
    struct iosb {
	union {
	    unsigned long status;
	    unsigned short st[2];
	} sb;
	unsigned long spare;
    } s;
    struct {
	struct iosb *s;
	char *serv;
	char *prot;
    } par;
    unsigned long e;
    char sbuf[30], pbuf[30];
    char *p;

    debug(F111,"UCX getservbyname",service,(int)C$$GA_UCX_GETSERVBYNAME);

    p = sbuf;
    strcpy(p, service);
    while (*p = toupper(*p), *p++) {}
    p = pbuf;
    strcpy(p, proto);
    while (*p = toupper(*p), *p++) {}

    par.s = &s;

    par.serv = "";
    par.prot = "";
    /* reset file pointer or something like that!?!? */
    e = (*C$$GA_UCX_GETSERVBYNAME)(&par, &sent, par.s);
    par.serv = sbuf;
    par.prot = pbuf;		/* that is don't care */
    e = (*C$$GA_UCX_GETSERVBYNAME)(&par, &sent, par.s);
    if ((long)e == -1L)
      return NULL;
    if ((e & 1) == 0L) {
	C$$TRANSLATE(e);
	return NULL;
    }
    if ((s.sb.st[0] & 1) == 0) {
	C$$SOCK_TRANSLATE(&s.sb.st[0]);
	return NULL;
    }
    /* sent.s_port is returned by UCX in network byte order. */
    /* Calling htons here swaps the bytes, which ruins everything. */

    /* Oh yeah?  WHICH VERSION of UCX???  Let's try this... */

#ifndef __alpha /* Maybe it should be __DECC, or some version thereof... */
/*
  Hunter says: "In fact, the "#ifndef __alpha" isn't even needed, since
  my_getservbyname() isn't included if "__DECC" is defined, and that's
  always defined on Alpha."  But if it doesn't hurt either, better not risk
  taking it out.
*/
#ifndef TCPWARE
#define DO_HTONS
#endif /* TCPWARE */
#endif /* __alpha */

#ifdef DO_HTONS
    sent.s_port = htons(sent.s_port);
    debug(F111,"UCX getservbyname","port",ntohs(sent.s_port));
#else
    debug(F111,"UCX getservbyname","port",sent.s_port);
#endif /* DO_HTONS */
    return &sent;
}
#endif /* __DECC */
#endif /* DEC_TCPIP */
#endif /* EXCELAN */
#endif /* TCPSOCKET */

#ifndef NOTCPOPTS
#ifndef datageneral
int
ck_linger(onoff, timo) int onoff; int timo; {
/*
  The following, from William Bader, turns off the socket linger parameter,
  which makes a close() block until all data is sent.  "I don't think that
  disabling linger can ever cause kermit to lose data, but you telnet to a
  flaky server (or to our modem server when the modem is in use), disabling
  linger prevents kermit from hanging on the close if you try to exit."

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_LINGER
    struct linger linger_opt;
    SOCKOPT_T x;

    if (ttyfd == -1 ||
	nettype != NET_TCPA && nettype != NET_TCPB || ttmdm >= 0) {
	tcp_linger = onoff;
	tcp_linger_tmo = timo;
	return(1);
    }
    x = sizeof(linger_opt);
    if (getsockopt(ttyfd, SOL_SOCKET, SO_LINGER, (char *)&linger_opt, &x)) {
	perror("could not get SO_LINGER");    
    } else if (x != sizeof(linger_opt)) {
	debug(F101,"linger error: SO_LINGER len","",x);
	debug(F101,"linger SO_LINGER expected len","",sizeof(linger_opt));
    } else if (linger_opt.l_onoff != onoff || linger_opt.l_linger != timo) {
	linger_opt.l_onoff  = onoff;
	linger_opt.l_linger = timo;
	if (setsockopt(ttyfd,
		       SOL_SOCKET,
		       SO_LINGER,
		       (char *)&linger_opt,
		       sizeof(linger_opt))) {
            perror("error setting SO_LINGER");
         } else {
	     debug(F101,"linger new SO_LINGER","",linger_opt.l_onoff);
	     tcp_linger = onoff;
	     tcp_linger_tmo = timo;
	     return 1;
         }
    } else {
	debug(F101,"setlinger SO_LINGER unchanged","",linger_opt.l_onoff);
	tcp_linger = onoff;
	tcp_linger_tmo = timo;
	return 1;
    }
#else
    debug(F100,"SO_LINGER not defined","",0);
#endif /* SO_LINGER */
#else
    debug(F100,"SO_SOCKET not defined","",0);
#endif /* SOL_SOCKET */
    return(0);
}

int 
sendbuf(size) int size; {
/*
  The following, from William Bader, allows changing of socket buffer sizes,
  in case that might affect performance.

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_SNDBUF
    int i, rc = 0;
    SOCKOPT_T x;

    if (ttyfd == -1 ||
	nettype != NET_TCPA && nettype != NET_TCPB || ttmdm >= 0) {
	tcp_sendbuf = size;
	return 1;
    }
    x = sizeof(i);
    if (getsockopt(ttyfd, SOL_SOCKET, SO_SNDBUF, (char *)&i, &x)) {
	perror("could not get SO_SNDBUF");
    } else if (x != sizeof(i)) {
	debug(F101,"setsockbuf error: SO_SNDBUF len","",x);
	debug(F101,"setsockbuf SO_SNDBUF expected len","",sizeof(i));
    } else if (size <= 0) {
	tcp_sendbuf = i;
	debug(F101,"setsockbuf SO_SNDBUF retrieved","",i);
	return 1;
    } else if (i != size) {
	x = size;
	if (setsockopt(ttyfd, SOL_SOCKET, SO_SNDBUF, (char *)&x, sizeof(x))) {
	    perror("error setting SO_SNDBUF");
	} else {
	    debug(F101,"setsockbuf old SO_SNDBUF","",i);
	    debug(F101,"setsockbuf new SO_SNDBUF","",x);
	    tcp_sendbuf = size;
	    return 1;
	}
    } else {
	debug(F101,"setsockbuf SO_SNDBUF unchanged","",i);
	tcp_sendbuf = size;
	return 1;
    }
#else
    debug(F100,"SO_SNDBUF not defined","",0);
#endif /* SO_SNDBUF */
#else
    debug(F100,"SO_SOCKET not defined","",0);
#endif /* SOL_SOCKET */
    return(0);
}

int 
recvbuf(size) int size; {
/*
  The following, from William Bader, allows changing of socket buffer sizes,
  in case that might affect performance.

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_RCVBUF
    int i, rc = 0;
    SOCKOPT_T x;

    if (ttyfd == -1 ||
	nettype != NET_TCPA && nettype != NET_TCPB || ttmdm >= 0) {
	tcp_recvbuf = size;
	return(1);
    }
    x = sizeof(i);
    if (getsockopt(ttyfd, SOL_SOCKET, SO_RCVBUF, (char *)&i, &x)) {
	perror("could not get SO_RCVBUF");
    } else if (x != sizeof(i)) {
	debug(F101,"setsockbuf error: SO_RCVBUF len","",x);
	debug(F101,"setsockbuf SO_RCVBUF expected len","",sizeof(i));
    } else if (size <= 0) {
	tcp_recvbuf = i;
	debug(F101,"setsockbuf SO_RCVBUF retrieved","",i);
	return 1;
    } else if (i != size) {
	x = size;
	if (setsockopt(ttyfd, SOL_SOCKET, SO_RCVBUF, (char *)&x, sizeof(x))) {
	    perror("error setting SO_RCVBUF");
	} else {
	    debug(F101,"setsockbuf old SO_RCVBUF","",i);
	    debug(F101,"setsockbuf new SO_RCVBUF","",x);
	    tcp_recvbuf = size;
	    return 1;
	}
    } else {
	debug(F101,"setsockbuf SO_RCVBUF unchanged","",i);
	tcp_recvbuf = size;
	return 1;
    }
#else
    debug(F100,"SO_RCVBUF not defined","",0);
#endif /* SO_RCVBUF */
#else
    debug(F100,"SO_SOCKET not defined","",0);
#endif /* SOL_SOCKET */
    return 0;
}

int
keepalive(onoff) int onoff; {
#ifdef SOL_SOCKET
#ifdef SO_KEEPALIVE
    int keepalive_opt;
    SOCKOPT_T x;

    if (ttyfd == -1 ||
	nettype != NET_TCPA && nettype != NET_TCPB || ttmdm >= 0) {
	tcp_keepalive = onoff;
	return 1;
    }
    x = sizeof(keepalive_opt);
    if (getsockopt(ttyfd,
		   SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive_opt, &x)) {
	perror("could not get SO_KEEPALIVE");
    } else if (x != sizeof(keepalive_opt)) {
	debug(F101,"setkeepalive error: SO_KEEPALIVE len","",x);
	debug(F101,"setkeepalive SO_KEEPALIVE expected len","",
	      sizeof(keepalive_opt));
    } else if (keepalive_opt != onoff) {
	keepalive_opt = onoff;
	if (setsockopt(ttyfd,
		       SOL_SOCKET,
		       SO_KEEPALIVE,
		       (char *)&keepalive_opt,
		       sizeof(keepalive_opt))) {
	    perror("error clearing SO_KEEPALIVE");
	} else {
	    debug(F101,"setkeepalive new SO_KEEPALIVE","",keepalive_opt);
	    tcp_keepalive = onoff;
	    return 1;
	}
    } else {
	debug(F101,"setkeepalive SO_KEEPALIVE unchanged","",keepalive_opt);
	tcp_keepalive = onoff;
	return 1;
    }
#else
    debug(F100,"SO_KEEPALIVE not defined","",0);
#endif /* SO_KEEPALIVE */
#else
    debug(F100,"SO_SOCKET not defined","",0);
#endif /* SOL_SOCKET */
    return(0);
}

int
no_delay(onoff)  int onoff; {
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
    int nodelay_opt;
    SOCKOPT_T x;

    if (ttyfd == -1 ||
	nettype != NET_TCPA && nettype != NET_TCPB || ttmdm >= 0) {
	tcp_nodelay = onoff;
	return(1);
    }
    x = sizeof(nodelay_opt);
    if (getsockopt(ttyfd, SOL_SOCKET, TCP_NODELAY, (char *)&nodelay_opt, &x)) {
	perror("could not get TCP_NODELAY");
    } else if (x != sizeof(nodelay_opt)) {
	debug(F101,"setnodelay error: TCP_NODELAY len","",x);
	debug(F101,"setnodelay TCP_NODELAY expected len","",
	      sizeof(nodelay_opt));
    } else if (nodelay_opt != onoff) {
	nodelay_opt = onoff;
	if (setsockopt(ttyfd,
		       SOL_SOCKET,
		       TCP_NODELAY,
		       (char *)&nodelay_opt,
		       sizeof(nodelay_opt))) {
	    perror("error clearing TCP_NODELAY");
	} else {
	    debug(F101,"setnodelay new TCP_NODELAY","",nodelay_opt);
	    tcp_nodelay = onoff;
	    return 1;
	}
    } else {
	debug(F101,"setnodelay TCP_NODELAY unchanged","",nodelay_opt);
	tcp_nodelay = onoff;
	return(1);
    }
#else
    debug(F100,"TCP_NODELAY not defined","",0);
#endif /* TCP_NODELAY */
#else
    debug(F100,"SO_SOCKET not defined","",0);
#endif /* SOL_SOCKET */
    return 0;
}
#endif /* datageneral */
#endif /* NOTCPOPTS */

#ifdef SUNX25
#ifndef X25_WR_FACILITY
/* For Solaris 2.3 / SunLink 8.x - see comments in ckcnet.h */
void
bzero(s,n) char *s; int n; {
    memset(s,0,n);
}
#endif /* X25_WR_FACILITY */
#endif /* SUNX25 */

#ifdef TCPSOCKET
#ifndef OS2
#ifndef NOLISTEN

#ifdef BSDSELECT
#ifndef VMS
#ifndef BELLV10
#ifndef datageneral
#ifdef hp9000s500			/* HP-9000/500 HP-U 5.21 */
#include <time.h>
#else
#include <sys/time.h>
#endif /* hp9000s500 */
#endif /* datageneral */
#endif /* BELLV10 */
#endif /* VMS */
#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */
#endif /* BSDSELECT */

#ifdef SELECT
#ifdef CK_SCOV5
#include <sys/select.h>
#endif /* CK_SCOV5 */
#endif /* SELECT */

#ifdef TCPSOCKET
#ifndef SO_OOBINLINE			/* Hopefully only HP-UX 7.0 */
#define SO_OOBINLINE 0x0100
#endif /* SO_OOBINLINE */
#endif /* TCPSOCKET */

#ifdef NOTUSED

/* T C P S O C K E T _ O P E N -- Open a preexisting socket number */

int
tcpsocket_open(name,lcl,nett,timo) char * name; int * lcl; int nett; int timo {
    int on = 1;
    static struct servent *service, servrec;
    static struct hostent *host;
    static struct sockaddr_in saddr;
    static int saddrlen;
#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
#else
#ifdef BELLSELECT
    fd_set rfds;
#endif /* BELLSELECT */
#endif /* BSDSELECT */

    debug(F101,"tcpsocket_open nett","",nett);
    *ipaddr = '\0';

    if (nett != NET_TCPB)
      return(-1);			/* BSD socket support */

    netclos();				/* Close any previous connection. */
    strncpy(namecopy, name, NAMECPYL);	/* Copy the hostname. */
    ttnproto = NP_NONE;			/* No protocol selected yet. */
    debug(F110,"tcpsocket_open namecopy",namecopy,0);

    /* Assign the socket number to ttyfd and then fill in tcp structures */
    ttyfd = atoi(&name[1]);
    debug(F111,"tcpsocket_open","ttyfd",ttyfd);

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

#ifndef datageneral
#ifdef TCP_NODELAY
    no_delay(tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
    keepalive(tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER 
    ck_linger(tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* datageneral */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

    /* Get the name of the host we are connected to */

    saddrlen = sizeof(saddr);
    getpeername(ttyfd,(struct sockaddr *)&saddr,&saddrlen); 

    if ((host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL) {
	debug(F100,"tcpsocket_open gethostbyname != NULL","",0);
	strncpy(name, host->h_name, 79);
	strncat(name, ":", 80 - strlen(name));
#ifdef COMMENT
	itoa(ntohs(saddr.sin_port), name + strlen(name), 10);
#else
	sprintf(name + strlen(name),"%d",ntohs(saddr.sin_port));
#endif /* COMMENT */
	sprintf(ipaddr,"%s", (char *)inet_ntoa(saddr.sin_addr));
	printf("%s connected on port %d\n",host->h_name,ntohs(saddr.sin_port));
    }
    ttnet = nett;			/* TCP/IP (sockets) network */

#ifdef RLOGCODE
    if ( ntohs(saddr.sin_port) == 23 )
	ttnproto = NP_LOGIN ;
    else 
#endif /* RLOGCODE */
    /* Assume the service is TELNET. */
    {
	ttnproto = NP_TELNET;		/* Yes, set global flag. */
	tn_ini();			/* Start TELNET negotiations. */
    }

    if (*lcl < 0) *lcl = 1;		/* Local mode. */

    return(0);				/* Done. */
}
#endif /* NOTUSED */

/*  T C P S R V _ O P E N  --  Open a TCP/IP Server connection  */
/*
  Calling conventions same as ttopen(), except third argument is network
  type rather than modem type.
*/
int
tcpsrv_open(name,lcl,nett,timo) char * name; int * lcl; int nett; int timo; {
    char *p;
    int i, x;
    SOCKOPT_T on = 1;
    int ready_to_accept = 0;
    static struct servent *service, servrec;
    static struct hostent *host;
    static struct sockaddr_in saddr;
    static SOCKOPT_T saddrlen;

#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
#else
#ifdef BELLSELCT
    fd_set rfds;
#endif /* BELLSELECT */
#endif /* BSDSELECT */

    debug(F101,"tcpsrv_open nett","",nett);
    *ipaddr = '\0';

    if (nett != NET_TCPB)
      return(-1);			/* BSD socket support */

    netclos();				/* Close any previous connection. */
    strncpy(namecopy, name, NAMECPYL);	/* Copy the hostname. */
    ttnproto = NP_NONE;			/* No protocol selected yet. */
    debug(F110,"tcpsrv_open namecopy",namecopy,0);

#ifdef COMMENT
    if (tcpsrfd != -1) {
	socket_close(tcpsrfd);
	tcpsrfd = -1;
	tcpsrv_port = 0;
    }
#endif /* COMMENT */

    p = namecopy;			/* Was a service requested? */
    while (*p != '\0' && *p != ':')
      p++; /* Look for colon */
    if (*p == ':') {			/* Have a colon */
	*p++ = '\0';			/* Get service name or number */
    } else {				/* Otherwise use kermit */
	p = "kermit";
    }
    debug(F110,"tcpsrv_open service requested",p,0);
    if (isdigit(*p)) {			/* Use socket number without lookup */
	service = &servrec;
	service->s_port = htons((unsigned short)atoi(p));
    } else {				/* Otherwise lookup the service name */
	service = getservbyname(p, "tcp");
    }
    if (!service && !strcmp("kermit",p)) { /* Use Kermit service port */
	service = &servrec;
	service->s_port = htons(1649);
    }
#ifdef RLOGCODE
    if (service && !strcmp("login",p) && service->s_port != htons(513)) {
	fprintf(stderr,
		"  Warning: login service on port %d instead of port 513\n", 
		 ntohs(service->s_port));
	fprintf(stderr, "  Edit SERVICES file if RLOGIN fails to connect.\n");
	debug(F101,"tcpsrv_open login on port","",ntohs(service->s_port));
    }
#endif /* RLOGCODE */
    if (!service) {
	fprintf(stderr, "Cannot find port for service %s\n", p);
	debug(F101,"tcpsrv_open can't get service","",errno);
	errno = 0;			/* rather than mislead */
	return(-1);
    }

    /* If we currently have a listen active but port has changed then close */

    debug(F101,"tcpsrv_open checking previous connection","",tcpsrfd);
    debug(F101,"tcpsrv_open previous tcpsrv_port","",tcpsrv_port);
    if (tcpsrfd != -1 &&
	tcpsrv_port != ntohs((unsigned short)service->s_port)) {
	debug(F100,"tcpsrv_open closing previous connection","",0);
#ifdef TCPIPLIB
	socket_close(tcpsrfd);
#else
	close(tcpsrfd);
#endif /* TCPIPLIB */
	tcpsrfd = -1;
    }
    debug(F100,"tcpsrv_open tcpsrfd","",tcpsrfd);
    if (tcpsrfd == -1) {

	/* Set up socket structure and get host address */

	bzero((char *)&saddr, sizeof(saddr));
	debug(F100,"tcpsrv_open bzero ok","",0);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Get a file descriptor for the connection. */

	saddr.sin_port = service->s_port;
	ipaddr[0] = '\0';

	debug(F100,"tcpsrv_open calling socket","",0);
	if ((tcpsrfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("TCP socket error");
	    debug(F101,"tcpsrv_open socket error","",errno);
	    return (-1);
	}
	errno = 0;

	/* Specify the Port may be reused */

	debug(F100,"tcpsrv_open calling setsockopt","",0);
	x = setsockopt(tcpsrfd,
		       SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof on);
	debug(F101,"tcpsrv_open setsockopt","",x);

#ifndef NOTCPOPTS
#ifndef datageneral
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
	no_delay(tcp_nodelay);
	debug(F101,"tcpsrv_open no_delay","",tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
	keepalive(tcp_keepalive);
	debug(F101,"tcpsrv_open keepalive","",tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER 
	ck_linger(tcp_linger, tcp_linger_tmo);
	debug(F101,"tcpsrv_open linger","",tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
	sendbuf(tcp_sendbuf);
	debug(F101,"tcpsrv_open sendbuf","",tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
	recvbuf(tcp_recvbuf);
	debug(F101,"tcpsrv_open recvbuf","",tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* datageneral */
#endif /* NOTCPOPTS */

       /* Now bind to the socket */
	printf("\nBinding socket to port %d ...\n",
	       ntohs((unsigned short)service->s_port));
	if (bind(tcpsrfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
	    i = errno;			/* Save error code */
	    close(tcpsrfd);
	    tcpsrfd = -1;
	    tcpsrv_port = 0;
	    ttyfd = -1;
	    errno = i;			/* and report this error */
	    debug(F101,"tcpsrv_open bind errno","",errno);
	    return(-1);
	}
	debug(F100,"tcpsrv_open bind OK","",0);
	printf("Listening ...\n");
	if (listen(tcpsrfd, 15) < 0) {
	    i = errno;			/* Save error code */
	    close(tcpsrfd);
	    tcpsrfd = -1;
	    tcpsrv_port = 0;
	    ttyfd = -1;
	    errno = i;			/* And report this error */
	    debug(F101,"tcpsrv_open listen errno","",errno);
	    return(-1);
	}
	debug(F100,"tcpsrv_open listen OK","",0);
	tcpsrv_port = ntohs((unsigned short)service->s_port);
    }
    printf("\nWaiting to Accept a TCP/IP connection on port %d ...\n",
           ntohs((unsigned short)service->s_port));
    saddrlen = sizeof(saddr);

#ifdef BSDSELECT
    tv.tv_sec  = tv.tv_usec = 0L;
    if (timo < 0)
      tv.tv_usec = (long) -timo * 10000L;
    else
      tv.tv_sec = timo;
    debug(F101,"tcpsrv_open BSDSELECT","",timo);
#else
    debug(F101,"tcpsrv_open not BSDSELECT","",timo);
#endif /* BSDSELECT */

    if (!timo) {
	while (!ready_to_accept) {
#ifdef BSDSELECT
	    FD_ZERO(&rfds);
	    FD_SET(tcpsrfd, &rfds);
	    ready_to_accept =
	      ((select(FD_SETSIZE, 
#ifdef HPUX
#ifdef HPUX1010
		       (fd_set *)
#else

		       (int *)
#endif /* HPUX1010 */
#else
#ifdef __DECC
		       (fd_set *)
#endif /* __DECC */
#endif /* HPUX */
		       &rfds, NULL, NULL, &tv) > 0) && 
	       FD_ISSET(tcpsrfd, &rfds));
#else /* BSDSELECT */
#ifdef IBMSELECT
#define ck_sleepint 250
	    ready_to_accept =
	      (select(&tcpsrfd, 1, 0, 0,
		      timo < 0 ? -timo :
		      (timo > 0 ? timo * 1000L : ck_sleepint)) == 1
	       );
#else
#ifdef BELLSELECT
	    FD_ZERO(rfds);
	    FD_SET(tcpsrfd, rfds);
	    ready_to_accept =
	      ((select(128, rfds, NULL, NULL, timo < 0 ? -timo :
		      (timo > 0 ? timo * 1000L)) > 0) && 
	       FD_ISSET(tcpsrfd, rfds));
#else
SOME_FORM_OF_SELECT_IS_NEEDED_HERE
#endif /* BELLSELECT */
#endif /* IBMSELECT */
#endif /* BSDSELECT */
	}
    }
    if (ready_to_accept) {
	if ((ttyfd = accept(tcpsrfd,
			    (struct sockaddr *)&saddr,&saddrlen)) < 0) {
	    i = errno;			/* save error code */
	    close(tcpsrfd);
	    ttyfd = -1;
	    tcpsrfd = -1;
	    tcpsrv_port = 0;
	    errno = i;			/* and report this error */
	    debug(F101,"tcpsrv_open accept errno","",errno);
	    return(-1);
	}
	setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

	ttnet = nett;			/* TCP/IP (sockets) network */
	/* See if the service is TELNET. */
	if ((x = ntohs((unsigned short)service->s_port)) ==
	    getservbyname("telnet", "tcp")->s_port) {
	    ttnproto = NP_TELNET;	/* Yes, set global flag. */
	    tn_ini();			/* Start TELNET negotiations. */
	} else {
	    tn_ini();			/* Initialize TELNET negotiations. */
	}
	debug(F101,"tcpsrv_open service","",x);
	if (*lcl < 0)			/* Set local mode. */
	  *lcl = 1;

#ifdef COMMENT
	close(tcpsrfd);
	tcpsrfd = -1;
	tcpsrv_port = 0;
#endif /* COMMENT */

	if (host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) {
	    debug(F100,"tcpsrv_open gethostbyname != NULL","",0);
	    name[0] = '*';
	    strncpy(&name[1],host->h_name,79);
	    strncat(name,":",80-strlen(name));
	    strncat(name,p,80-strlen(name));
	    sprintf(ipaddr,"%s", (char *)inet_ntoa(saddr.sin_addr));
	    printf("%s connected on port %s\n",host->h_name,p);
	}
	return(0);			/* Done. */
    } else {
	i = errno;			/* save error code */
	close(tcpsrfd);
	ttyfd = -1;
	tcpsrfd = -1;
	tcpsrv_port = 0;
	errno = i;			/* and report this error */
	debug(F101,"tcpsrv_open accept errno","",errno);
	return(-1);
    }
}
#endif /* NOLISTEN */
#endif /* OS2 */
#endif /* TCPSOCKET */

/*  N E T O P E N  --  Open a network connection  */
/*
  Calling conventions same as ttopen(), except third argument is network
  type rather than modem type.  Designed to be called from within ttopen.
*/
int
netopen(name, lcl, nett) char *name; int *lcl, nett; {
    char *p;
    int i, x;
#ifdef TCPSOCKET
    int isconnect = 0;
#ifdef SO_OOBINLINE
    int on = 1;
#endif /* SO_OOBINLINE */
    struct servent *service=NULL, servrec;
    struct hostent *host=NULL;
    struct sockaddr_in saddr;
#ifdef EXCELAN
    struct sockaddr_in send_socket;
#endif /* EXCELAN */
#endif /* TCPSOCKET */

#ifdef SUNX25				/* Code for SunLink X.25 support */
#define X29PID 1			/* X.29 Protocol ID */
_PROTOTYP(SIGTYP x25oobh, (int) );
    CONN_DB x25host;
#ifndef X25_WR_FACILITY
    FACILITY x25facil;
#else
    FACILITY_DB x25facil;
#endif /* X25_WR_FACILITY */
    static int needh = 1;
    PID_T pid;
    extern int linkid, lcn, x25ver;
    extern int revcall, closgr, cudata;
    extern char udata[];
#endif /* SUNX25 */

    debug(F101,"netopen nett","",nett);
    *ipaddr = '\0';			/* Initialize IP address string */

#ifdef SUNX25
    if (nett == NET_SX25) {		/* If network type is X.25 */
        netclos();			/* Close any previous net connection */
        ttnproto = NP_NONE;		/* No protocol selected yet */

        /* Set up host structure */
        bzero((char *)&x25host,sizeof(x25host));
        if ((x25host.hostlen = pkx121(name,x25host.host)) < 0) {
            fprintf (stderr,"Invalid X.121 host address %s\n",name);
            errno = 0;
            return (-1);
        }
        x25host.datalen = X29PIDLEN;
        x25host.data[0] = X29PID;

	/* Set call user data if specified */
        if (cudata) {
            strncpy((char *)x25host.data+X29PIDLEN,udata,(int)strlen(udata));
            x25host.datalen += (int)strlen(udata);
        }

        /* Open SunLink X.25 socket */
	if (!quiet && *name) printf(" Trying %s...\n", name);
        if ((ttyfd = socket(AF_X25, SOCK_STREAM, 0)) < 0) {
	    debug(F101,"netopen socket error","",errno);
            perror ("X.25 socket error");
            return (-1);
        }

        /* Setting X.25 out-of-band data handler */
        pid = getpid();
        if (ioctl(ttyfd,SIOCSPGRP,&pid)) {
            perror("X.25 set process group id error");
            return(-1);
        }
        (VOID) signal(SIGURG,x25oobh);

        /* Set reverse charge call and closed user group if requested */
        bzero ((char *)&x25facil,sizeof(x25facil));

#ifndef X25_WR_FACILITY
/*  New SunLink (7.0 or 8.0, not sure which)... */
	x25facil.type = T_REVERSE_CHARGE; /* Reverse Charge */
	x25facil.f_reverse_charge = revcall ? 1 : 0;
        if (ioctl(ttyfd,X25_SET_FACILITY,&x25facil) < 0) {
            perror ("Setting X.25 reverse charge");
            return (-1);
        }
	if (closgr > -1) {		/* Closed User Group (Outgoing) */
	    bzero ((char *)&x25facil,sizeof(x25facil));
	    x25facil.type = T_CUG;
	    x25facil.f_cug_req = CUG_REQ_ACS;
	    x25facil.f_cug_index = closgr;
	    if (ioctl(ttyfd,X25_SET_FACILITY,&x25facil) < 0) {
		perror ("Setting X.25 closed user group");
		return (-1);
	    }
	}
#else
/*  Old SunLink 6.0 (or 7.0?)... */
        if (revcall) x25facil.reverse_charge = revcall;
        if (closgr > -1) {
            x25facil.cug_req = 1;
            x25facil.cug_index = closgr;
        }
        if (ioctl(ttyfd,X25_WR_FACILITY,&x25facil) < 0) {
            perror ("Setting X.25 facilities");
            return (-1);
        }
#endif /* X25_WR_FACILITY */

        /*  Need X.25 header with bits Q and M */
        if (ioctl (ttyfd,X25_HEADER,&needh) < 0) {
            perror ("Setting X.25 header");
            return (-1);
        }

        /* Connects to remote host via SunLink X.25 */
        if (connect(ttyfd,(struct sockaddr *)&x25host,sizeof(x25host)) < 0) {
	    i = errno;
	    debug(F101,"netopen connect errno","",i);
	    if (i) {
		perror("netopen x25 connect");
		x25diag();
	    }
	    (VOID) netclos();
	    ttyfd = -1;
	    ttnproto = NP_NONE;
	    errno = i;
	    return (-1);
	}

        /* Get X.25 link identification used for the connection */
        if (ioctl(ttyfd,X25_GET_LINK,&linkid) < 0) {
            perror ("Getting X.25 link id");
            return (-1);
        }

        /* Get X.25 logical channel number used for the connection */
        if (ioctl(ttyfd,X25_RD_LCGN,&lcn) < 0) {
            perror ("Getting X.25 lcn");
            return (-1);
        }

        /* Get SunLink X.25 version */
        if (ioctl(ttyfd,X25_VERSION,&x25ver) < 0) {
            perror ("Getting SunLink X.25 version");
            return (-1);
        }
        ttnet = nett;                   /* Sunlink X.25 network */
        ttnproto = NP_X3;               /* PAD X.3, X.28, X.29 protocol */
        if (*lcl < 0) *lcl = 1;         /* Local mode */
        return(0);
    } else /* Note that SUNX25 support can coexist with TCP/IP support. */
#endif /* SUNX25 */

/*   Add support for other networks here. */

      if (nett != NET_TCPB) return(-1);	/* BSD socket support */

#ifdef TCPSOCKET
    netclos();				/* Close any previous connection. */
    strncpy(namecopy, name, NAMECPYL);	/* Copy the hostname. */
    ttnproto = NP_NONE;			/* No protocol selected yet. */
    debug(F110,"netopen namecopy",namecopy,0);

#ifndef NOLISTEN
    if (name[0] == '*')
      return(tcpsrv_open(name, lcl, nett, 0));
#endif /* NOLISTEN */

    p = namecopy;			/* Was a service requested? */
    while (*p != '\0' && *p != ':') p++; /* Look for colon */
    if (*p == ':') {			/* Have a colon */
	*p++ = '\0';			/* Get service name or number */
    } else {				/* Otherwise use telnet */
	p = "telnet";
    }
    debug(F110,"netopen service requested",p,0);
    if (isdigit(*p)) {			/* Use socket number without lookup */
	service = &servrec;
	service->s_port = htons((unsigned short)atoi(p));
    } else {				/* Otherwise lookup the service name */
	service = getservbyname(p, "tcp");
    }
    if (!service) {
	fprintf(stderr, "Cannot find port for service %s\n", p);
#ifdef TGVORWIN
	debug(F101,"netopen can't get service","",socket_errno);
#else
	debug(F101,"netopen can't get service","",errno);
#endif /* TGVORWIN */
	errno = 0;			/* rather than mislead */
	return(-1);
    }

#ifdef RLOGCODE
    if (service && !strcmp("login",p) && service->s_port != htons(513)) {
	fprintf(stderr,
		"  Warning: login service on port %d instead of port 513\n", 
		 ntohs(service->s_port));
	fprintf(stderr, "  Edit SERVICES file if RLOGIN fails to connect.\n");
	debug(F101,"tcpsrv_open login on port","",ntohs(service->s_port));
    }
#endif /* RLOGCODE */

    /* Set up socket structure and get host address */

    bzero((char *)&saddr, sizeof(saddr));
    debug(F100,"netopen bzero ok","",0);
    if (
#ifdef NT
        /* we found that Win95 tries to call the DNS  */
        /* when a numeric IP Address is specified.    */
        /* and of course the lookup fails resulting   */
        /* in a long delay.  So we test for the IP    */
        /* numeric value before calling gethostbyname */
        /* but only in Win32 so as not to             */
        /* alter current code that works properly     */
        /* everywhere else.                           */
        inet_addr(namecopy) == INADDR_NONE &&
#endif /* NT */
        (host = gethostbyname(namecopy)) != NULL) {
	debug(F100,"netopen gethostbyname != NULL","",0);
#ifdef OS2
	strncpy(name,host->h_name,80);
	strncat(name,":",80-strlen(name));
	strncat(name,p,80-strlen(name));
#endif /* OS2 */
	saddr.sin_family = host->h_addrtype;
#ifdef HADDRLIST
#ifdef h_addr
	/* This is for trying multiple IP addresses - see <netdb.h> */
	if (!(host->h_addr_list))
	  return(-1);
	bcopy(host->h_addr_list[0], (caddr_t)&saddr.sin_addr, host->h_length);
#else
	bcopy(host->h_addr, (caddr_t)&saddr.sin_addr, host->h_length);
#endif /* h_addr */
#else  /* HADDRLIST */
	bcopy(host->h_addr, (caddr_t)&saddr.sin_addr, host->h_length);
#endif /* HADDRLIST */
#ifndef EXCELAN
	debug(F111,"BCOPY","host->h_addr",host->h_addr);
#endif /* EXCELAN */
	debug(F111,"BCOPY"," (caddr_t)&saddr.sin_addr",
	      (caddr_t)&saddr.sin_addr);
	debug(F111,"BCOPY","host->h_length",host->h_length);
    } else {
#ifdef INADDRX
/* inet_addr() is of type struct in_addr */
	struct in_addr ina;
	unsigned long uu;
#ifdef datageneral
	extern struct in_addr inet_addr();
#endif /* datageneral */
	debug(F100,"netopen gethostbyname == NULL: INADDRX","",0);
	ina = inet_addr(namecopy);
	uu = *(unsigned long *)&ina;
#else /* Not INADDRX */
/* inet_addr() is unsigned long */
	unsigned long uu;
	debug(F100,"netopen gethostbyname == NULL: Not INADDRX","",0);
	uu = inet_addr(namecopy);
#endif /* INADDRX */
	debug(F101,"netopen uu","",uu);
	if ((saddr.sin_addr.s_addr = uu) != ((unsigned long)-1))
	  saddr.sin_family = AF_INET;
	else {
	    fprintf(stderr, "Can't get address for %s\n", namecopy);
#ifdef TGVORWIN
	    debug(F101,"netopen can't get address","",socket_errno);
#else
	    debug(F101,"netopen can't get address","",errno);
#endif /* TGVORWIN */
	    errno = 0;		/* Rather than mislead */
	    return(-1);
	}
    }

    /* Get a file descriptor for the connection. */

    saddr.sin_port = service->s_port;
    sprintf(ipaddr,"%s", (char *)inet_ntoa(saddr.sin_addr));
    debug(F110,"netopen trying",ipaddr,0);
    if (!quiet && *ipaddr) printf(" Trying %s...\n", ipaddr);

    /* Loop to try additional IP addresses, if any. */
 
    do {
#ifdef EXCELAN
	send_socket.sin_family = AF_INET;
	send_socket.sin_addr.s_addr = 0;
	send_socket.sin_port = 0;
	if ((ttyfd = socket(SOCK_STREAM, (struct sockproto *)0,
			    &send_socket, SO_REUSEADDR)) < 0)
#else  /* EXCELAN */
#ifdef NT
#ifdef COMMENT
       /*
	 Must make sure that all sockets are opened in
         Non-overlapped mode since we use the standard
         C RTL functions to read and write data.
         But it doesn't seem to work as planned.
       */
	  {
	      int optionValue = SO_SYNCHRONOUS_NONALERT;
	      if (setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, 
			     (char *) &optionValue, sizeof(optionValue))
		  != NO_ERROR)
		return(-1);
	  }
#endif /* COMMENT */
#endif /* NT */

	if ((ttyfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif /* EXCELAN */
	    {
#ifdef EXCELAN
		experror("TCP socket error");
#else
#ifdef TGVORWIN
#ifdef OLD_TWG
                errno = socket_errno;
#endif /* OLD_TWG */
		socket_perror("TCP socket error");
		debug(F101,"netopen socket error","",socket_errno);
#else
		perror("TCP socket error");
		debug(F101,"netopen socket error","",errno);
#endif /* TGVORWIN */
#endif /* EXCELAN */
		return (-1);
	    }
	errno = 0;

#ifdef RLOGCODE                                                    
       /* Not part of the RLOGIN RFC, but the BSD implementation     */
       /* requires that the client port be a priviliged port (<1024) */
       /* on a Unix system this would require SuperUser permissions  */
       /* thereby saying that the root of the Unix system has given  */
       /* permission for this connection to be created               */
       if (service->s_port == htons((unsigned short)RLOGIN_PORT)) {
	   struct sockaddr_in sin;
	   static unsigned short lport = 1024;	/* max reserved port */
	   int s_errno;

	   lport--;			/* Make sure we do not reuse a port */
	   if (lport == 512)
             lport = 1023;

	   sin.sin_family = AF_INET;
	   sin.sin_addr.s_addr = INADDR_ANY;
	   while (1) {
	       sin.sin_port = htons(lport);
	       if (bind(ttyfd, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
		 break;
#ifdef OS2
	       s_errno = socket_errno;
	       if (s_errno && /* OS2 bind fails with 0, if already in use */
#ifdef NT
		   s_errno != WSAEADDRINUSE
#else
		   s_errno != SOCEADDRINUSE &&
		   s_errno != (SOCEADDRINUSE - SOCBASEERR)
#endif /* NT */
		   )
#else /* OS2 */
#ifdef TGVORWIN
		 if (socket_errno != EADDRINUSE)
#else
		 if (errno != EADDRINUSE)
#endif /* TGVORWIN */
#endif /* OS2 */
		   {
		       printf("\nBind failed with errno %d  for port %d.\n",
#ifdef OS2
			      s_errno
#else
#ifdef TGVORWIN
			      socket_errno
#else
			      errno
#endif /* TGVORWIN */
#endif /* OS2 */
			      , lport
			      );
#ifdef OS2
                       debug(F101,"rlogin bind failed","",s_errno);
#else
#ifdef TGVORWIN
                       debug(F101,"rlogin bind failed","",socket_errno);
#ifdef OLD_TWG
                       errno = socket_errno;
#endif /* OLD_TWG */
                       socket_perror("rlogin bind");
#else
                       debug(F101,"rlogin bind failed","",errno);
                       perror("rlogin bind");
#endif /* TGVORWIN */
#endif /* OS2 */
		       netclos();
		       return -1;
		   }
	       lport--;
	       if (lport == 512 /* lowest reserved port to use */ ) {
		   printf("\nNo reserved ports available.\n");
		   netclos();
		   return -1;
	       }
	   }
	   debug(F101,"rlogin lport","",lport);
	   ttnproto = NP_RLOGIN;
       }
#endif /* RLOGCODE  */

/* Now connect to the socket on the other end. */

#ifdef EXCELAN
	if (connect(ttyfd, &saddr) < 0)
#else
#ifdef NT
	  WSASafeToCancel = 1;
#endif /* NT */
	if (connect(ttyfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
#endif /* EXCELAN */
	  {
#ifdef NT
	      WSASafeToCancel = 0;
#endif /* NT */
#ifdef OS2
	      i = socket_errno;
#else /* OS2 */
#ifdef TGVORWIN
              i = socket_errno;
#else
              i = errno;                /* Save error code */
#endif /* TGVORWIN */
#endif /* OS2 */
#ifdef RLOGCODE
	      if (
#ifdef OS2
                 i && /* OS2 bind fails with 0, if already in use */
#ifdef NT
                 i == WSAEADDRINUSE
#else
		 (i == SOCEADDRINUSE ||
		 i == (SOCEADDRINUSE - SOCBASEERR))
#endif /* NT */
#else /* OS2 */
#ifdef TGVORWIN
                  socket_errno == EADDRINUSE
#else
                  errno == EADDRINUSE
#endif /* TGVORWIN */
#endif /* OS2 */
		  && ttnproto == NP_RLOGIN) {
#ifdef TCPIPLIB
		   socket_close(ttyfd); /* Close it. */
#else
		   close(ttyfd);
#endif /* TCPIPLIB */
		   continue;		/* Try a different lport */
	       }
#endif /* RLOGCODE */
#ifdef HADDRLIST
#ifdef h_addr
	      if (host && host->h_addr_list && host->h_addr_list[1]) {
		  perror("");
		  host->h_addr_list++;
		  bcopy(host->h_addr_list[0],
			(caddr_t)&saddr.sin_addr,
			host->h_length);

		  sprintf(ipaddr,"%s", (char *)inet_ntoa(saddr.sin_addr));
		  debug(F110,"netopen h_addr_list",ipaddr,0);
		  if (!quiet && *ipaddr)
		    printf(" Trying %s...\n", ipaddr);
#ifdef TCPIPLIB
		  socket_close(ttyfd); /* Close it. */
#else
		  close(ttyfd);
#endif /* TCPIPLIB */
		  continue;
	      }
#endif /* h_addr */
#endif  /* HADDRLIST */
	      netclos();
	      ttyfd = -1;
	      ttnproto = NP_NONE;
	      errno = i;		/* And report this error */
#ifdef EXCELAN
	      if (errno) experror("netopen connect");
#else
#ifdef TGVORWIN
	      debug(F101,"netopen connect error","",socket_errno);
	      /* if (errno) socket_perror("netopen connect"); */
#ifdef OLD_TWG
              errno = socket_errno;
#endif /* OLD_TWG */
              socket_perror("netopen connect");
#else /* TGVORWIN */
	      debug(F101,"netopen connect errno","",errno);
#ifdef DEC_TCPIP
	      perror("netopen connect");
#endif /* DEC_TCPIP */
#ifdef CMU_TCPIP
	      perror("netopen connect");
#endif /* CMU_TCPIP */
#endif /* TGVORWIN */
#endif /* EXCELAN */
	      return(-1);
	  }
#ifdef NT
	WSASafeToCancel = 0;
#endif /* NT */
	isconnect = 1;
    } while (!isconnect);

#ifdef SO_OOBINLINE
/*
  The symbol SO_OOBINLINE is not known to Ultrix 2.0.
  It means "leave out of band data inline".  The normal value is 0x0100,
  but don't try this on systems where the symbol is undefined.
*/
/*
  Note from Jeff Altman: 12/13/95
  In implementing rlogin protocol I have come to the conclusion that it is 
  a really bad idea to read out-of-band data inline.  
  At least Windows and OS/2 does not handle this well.
  And if you need to know that data is out-of-band, then it becomes 
  absolutely pointless.

  Therefore, at least on OS2 and Windows (NT) I have changed the value of
  on to 0, so that out-of-band data stays out-of-band.

  12/18/95
  Actually, OOB data should be read inline when possible.  Especially with
  protocols that don't care about the Urgent flag.  This is true with Telnet.
  With Rlogin, you need to be able to catch OOB data.  However, the best 
  way to do this is to set a signal handler on SIGURG.  This isn't possible 
  on OS/2 and Windows.  But it is in UNIX.  We will also need OOB data for 
  FTP so better create a general mechanism.

  The reason for making OOB data be inline is that the standard ttinc/ttoc
  calls can be used for reading that data on UNIX systems.  If we didn't 
  have the OOBINLINE option set then we would have to use recv(,MSG_OOB) 
  to read it.

*/
#ifdef RLOGCODE
#ifdef TCPIPLIB
    if (ttnproto == NP_RLOGIN || ttnproto == NP_FTP)
      on = 0;
#else /* TCPIPLIB */
    if (ttnproto == NP_RLOGIN) {
	debug(F100,"Installing rlogoobh on SIGURG","",0);
	signal(SIGURG, rlogoobh);
    } else 
#ifdef FTPCODE
      if (ttnproto == NP_FTP) {
	  signal(SIGURG, ftpoobh);
      } else
#endif /* FTPCODE */
	{
	    signal(SIGURG, SIG_DFL);
	}
#endif /* TCPIPLIB */
#endif /* RLOGCODE */

#ifdef datageneral
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef BSD43
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef OSF1
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef POSIX
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef MOTSV88R4 
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef SOLARIS
/*
  Maybe this applies to all SVR4 versions, but the other (else) way has been
  compiling and working fine on all the others, so best not to change it.
*/
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef OSK
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef OS2
    {
	int rc;
	rc = setsockopt(ttyfd,
			SOL_SOCKET,
			SO_OOBINLINE,
			(char *) &on,
			sizeof on
			);
	debug(F111,"setsockopt SO_OOBINLINE",on ? "on" : "off" ,rc);
    }
#else
#ifdef VMS /* or, at least, VMS with gcc */
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif /* VMS */
#endif /* OS2 */
#endif /* OSK */
#endif /* SOLARIS */
#endif /* MOTSV88R4 */
#endif /* POSIX */
#endif /* BSD43 */
#endif /* OSF1 */
#endif /* datageneral */
#endif /* SO_OOBINLINE */

#ifndef NOTCPOPTS
#ifndef datageneral
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
    no_delay(tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
    keepalive(tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER 
    ck_linger(tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* datageneral */
#endif /* NOTCPOPTS */

    ttnet = nett;			/* TCP/IP (sockets) network */

    x = ntohs((unsigned short)service->s_port);
    /* See if the service is TELNET. */
    if (x == TELNET_PORT) {
	ttnproto = NP_TELNET;		/* Yes, set global flag. */
    }
#ifdef RLOGCODE
    else if (x == RLOGIN_PORT) {
	ttnproto = NP_RLOGIN;
	if (rlog_ini() < 0) {
	    debug(F100,"rlogin initialization failed","",0);
	    netclos();
	    return -1;
	}
    }
#endif /* RLOGCODE */
#ifdef COMMENT /* not yet */
    else if (x == KERMIT_PORT) {
	ttnproto = NP_KERMIT;
    }
#endif /* COMMENT */

#ifndef datageneral
/* Find out our own IP address */
    {
	struct sockaddr_in sa;

#ifndef GSOCKNAME_T
#define GSOCKNAME_T int
#ifdef UNIXWARE
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#else
#ifdef VMS
#ifdef DEC_TCPIP
#ifdef __DECC_VER
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#endif /* __DECC_VER */
#endif /* DEC_TCPIP */
#endif /* VMS */
#endif /* UNIXWARE */
#endif /* GSOCKNAME_T */

	GSOCKNAME_T slen;

	slen = sizeof(sa);
#ifdef COMMENT
/* memset is not portable */
	memset(&sa, 0, slen);
#else
	bzero((char *)&sa, slen);
#endif /* COMMENT */
#ifndef EXCELAN
	if (!getsockname(ttyfd, (struct sockaddr *)&sa, &slen) ) {
	    sprintf(myipaddr,"%s", (char *)inet_ntoa(sa.sin_addr));
	    debug(F110,"getsockname",myipaddr,0);
	}
#endif /* EXCELAN */
    }
#endif /* datageneral */
    tn_ini();
    debug(F101,"netopen service","",x);
    if (*lcl < 0) *lcl = 1;		/* Local mode. */
#endif /* TCPSOCKET */
    return(0);				/* Done. */
}

/*  N E T C L O S  --  Close current network connection.  */

int
netclos() {
    int x = 0;
    debug(F101,"netclos","",ttyfd);
    if (ttyfd == -1)			/* Was open? */
      return(0);			/* Wasn't. */
    if (ttyfd > -1) {			/* Was. */
#ifdef VMS	
	ck_cancio();			/* Cancel any outstanding reads. */
#endif /* VMS */
#ifdef TCPIPLIB
	x = socket_close(ttyfd);	/* Close it. */
#else
#ifndef OS2
	x = close(ttyfd);
#endif /* OS2 */
#endif /* TCPIPLIB */
    }
    ttyfd = -1;				/* Mark it as closed. */
    ttnproto = NP_NONE;			/* Reset the protocol type */
    debug(F100,"netclose setting tn_init = 0","",0);
    tn_init = 0;			/* Remember about telnet protocol... */
    *ipaddr = '\0';			/* Zero the IP address string */
#ifdef TCPIPLIB
/*
  Empty the internal buffers so they won't be used as invalid input on
  the next connect attempt (rlogin).
*/
    ttibp = 0;
    ttibn = 0;
#endif /* TCPIPLIB */
    return(x);
}

/*  N E T T C H K  --  Check if network up, and how many bytes can be read */
/*
  Returns number of bytes waiting, or -1 if connection has been dropped.
*/
int					/* Check how many bytes are ready */
nettchk() {				/* for reading from network */
#ifdef TCPIPLIB
    long count = 0;
    int x = 0;
    long y;
    char c;

    debug(F101,"nettchk entry ttibn","",ttibn);
    debug(F101,"nettchk entry ttibp","",ttibp);
#ifndef OS2
    socket_errno = 0; /* This is a function call in NT */
#endif /* OS2 */

    if (ttyfd == -1) {
	debug(F100,"nettchk socket is closed","",0);
	return(-1);
    }
/*
  Note: this socket_ioctl() call does NOT return an error if the
  connection has been broken.  (At least not in MultiNet.)
*/
#ifdef COMMENT
/*  Another trick that can be tried here is something like this: */

    if (ttnet == NET_TCPB) {
	char dummy;
	x = read(ttyfd,&dummy,0);	/* Try to read nothing */
	if (x < 0) {			/* "Connection reset by peer" */
	    perror("TCP/IP");		/* or somesuch... */
	    ttclos();			/* Close our end too. */
	    return(-1);
	}
    }
#endif /* COMMENT */

    if (socket_ioctl(ttyfd,FIONREAD,
#ifdef COMMENT
    /* Now we've changed the ioctl(..,..,x) prototype for DECC to (void *) */
#ifdef __DECC
    /* NOTE: "&count" might need to be "(char *)&count" in some settings. */
		     /* Cast needed for DECC 4.1 & later? */		     
		     /* Maybe, but __DECC_VER only exists in 5.0 and later */
		     (char *)
#endif /* __DECC */
#endif /* COMMENT */
		     &count
		     ) < 0) {
	debug(F101,"nettchk socket_ioctl error","",socket_errno);
	if (ttibn < 1) {
	    netclos();			/* *** *** */
	    return(-1);
	} else return(ttibn);
    }
    debug(F101,"nettchk count","",count);
/*
  The following code works well in most settings, but messes things up in
  others, including CMU/Tek TCP/IP and UCX 2.0, where it somehow manages to
  make it impossible to ever make a new connection to the same host again with
  CONNECT, once it has been logged out from the first time.  Not even if you
  HANGUP first, or SET HOST<CR>, or SET LINE<CR>.  Reportedly, however, it
  does work OK in later releases of UCX.  But there is no way we can
  accommodate both old and new -- we might have static linking or dynamic
  linking, etc etc.  If we have static, I only have access to 2.0, where this
  doesn't work, etc etc blah blah.

  In the following lines, we define a symbol NOCOUNT for builds where we want
  to omit this code.  By default, it is omitted for CMU/Tek.  You can force
  omission of it for other combinations by defining NOCOUNT in CFLAGS.  You
  can force inclusion of this code, even for CMU/Tek, by including NONOCOUNT
  in CFLAGS.
*/
#ifdef NONOCOUNT
#ifdef NOCOUNT
#undef NOCOUNT
#endif /* NOCOUNT */
#else
#ifndef NOCOUNT
#ifdef CMU_TCPIP
#define NOCOUNT
#endif /* CMU_TCPIP */
#endif /* NOCOUNT */
#endif /* NONOCOUNT */

    if (count == 0) {
#ifndef NOCOUNT
/*
  Here we need to tell the difference between a 0 count on an active
  connection, and a 0 count because the remote end of the socket broke the
  connection.  There is no mechanism in TGV MultiNet (or WIN/TCP?) to query
  the status of the connection, so we have to do a read.  -1 means there was
  no data available (socket_errno == EWOULDBLOCK), 0 means the connection is
  down.  But if, by chance, we actually get a character, we have to put it
  where it won't be lost.
*/
	y = 1;				/* Turn on nonblocking reads */
	x = socket_ioctl(ttyfd,FIONBIO,&y);
	debug(F101,"nettchk FIONBIO","",x);
	x = socket_read(ttyfd,&c,1);	/* Returns -1 if no data */
	debug(F101,"nettchk socket_read","",x);
	if (x == -1) {
	    int s_errno = socket_errno;	/* socket_errno may be a function */
	    debug(F101,"nettchk socket_read errno","",s_errno);
#ifdef OS2
	    switch (s_errno) {
#ifdef NT
	      case WSAECONNRESET:
#else /* NT */
	      case SOCECONNRESET:
	      case SOCECONNRESET - SOCBASEERR:
#endif /* NT */
		debug(F100,"nettchk ECONRESET","",0);
		netclos();		/* *** *** */
		return(-1);		/* Connection is broken. */
#ifdef NT
	      case WSAECONNABORTED:
#else /* NT */
	      case SOCECONNABORTED:
	      case SOCECONNABORTED - SOCBASEERR:
#endif /* NT */
		debug(F100,"nettchk ECONNABORTED","",0);
		netclos();		/* *** *** */
		return(-1);		/* Connection is broken. */
#ifdef NT
	      case WSAENETRESET:  
#else /* NT */
	      case SOCENETRESET:
	      case SOCENETRESET - SOCBASEERR:
#endif /* NT */
		debug(F100,"nettchk ENETRESET","",0);
		netclos();		/* *** *** */
		return(-1);		/* Connection is broken. */
#ifdef NT
	      case WSAENOTCONN:
#else /* NT */
	      case SOCENOTCONN:
	      case SOCENOTCONN - SOCBASEERR:
#endif /* NT */
		debug(F100,"nettchk ENOTCONN","",0);
		netclos();		/* *** *** */
		return(-1);		/* Connection is broken. */
#ifdef NT
	      case WSAEWOULDBLOCK:
#else
	      case SOCEWOULDBLOCK:
	      case SOCEWOULDBLOCK - SOCBASEERR:
#endif /* NT */
		debug(F100,"nettchk EWOULDBLOCK","",0);
		break;
	    }
#endif /* OS2 */
	}
	y = 0;				/* Turn them back off */
	socket_ioctl(ttyfd,FIONBIO,&y);
	if (x == 0) {
	    debug(F100,"nettchk connection closed","",0);
	    netclos();			/* *** *** */
	    return(-1);			/* Connection is broken. */
	}
	if (x == 1) {			/* Oops, actually got a byte? */
	    debug(F101,"nettchk socket_read char","",c);
	    debug(F101,"nettchk ttibp","",ttibp);
	    debug(F101,"nettchk ttibn","",ttibn);
/*
  So put the byte we got into the buffer at the current position.
  Increment the buffer count, but DON'T increment the buffer pointer.
*/
	    ttibuf[ttibp+ttibn] = c;
	    ttibn++;
#ifdef DEBUG
	    ttibuf[ttibp+ttibn] = '\0';
	    debug(F111,"nettchk ttibn",ttibuf,ttibn);
#endif /* DEBUG */
	}
#else
	if (ttnet == NET_TCPB) {
	    char dummy;
	    x = read(ttyfd,&dummy,0);	/* Try to read nothing */
	    if (x < 0) {		/* "Connection reset by peer" */
		perror("TCP/IP");	/* or somesuch... */
		ttclos();		/* Close our end too. */
		return(-1);
	    }
	}
#endif /* NOCOUNT */
    }
    debug(F101,"nettchk returns","",count+ttibn);
    return(count + ttibn);

#else /* Not TCPIPLIB */
/*
  UNIX just uses ttchk(), in which the ioctl() calls on the file descriptor
  seem to work OK.
*/
    return(0);
#endif /* TCPIPLIB */
/*
  But what about X.25?
*/
}

#ifndef OS2
VOID
nettout(i) int i; {			/* Catch the alarm interrupts */
    debug(F100,"nettout caught timeout","",0);
    ttimoff();
    cklongjmp(njbuf, -1);
}
#endif /* !OS2 */

#ifdef TCPIPLIB

VOID
#ifdef CK_ANSIC
donetinc(void * threadinfo)
#else /* CK_ANSIC */
donetinc(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* donetinc */ {
#ifdef NTSIG
    extern int TlsIndex;
    if (threadinfo) {			/* Thread local storage... */
	TlsSetValue(TlsIndex,threadinfo);
    }
#endif /* NTSIG */
    while (1) {
	if (ttbufr() < 0)		/* Keep trying to refill it. */
	  break;			/* Till we get an error. */
	if (ttibn > 0)			/* Or we get a character. */
	  break;
    }
}
#endif /* TCPIPLIB */

VOID
#ifdef CK_ANSIC
failnetinc(void * threadinfo)
#else /* CK_ANSIC */
failnetinc(threadinfo) VOID * threadinfo;
#endif /* CK_ANSIC */
/* failnetinc */ {
    ; /* Nothing to do on an error */
}

/* N E T X I N -- Input block of characters from network */

int 
netxin(n,buf) int n; char * buf; {
    int len;
    int rc;
    int i, j;
    if (ttyfd == -1) {
	debug(F100,"netinc socket is closed","",0);
	return(-2);
    }
#ifdef TCPIPLIB
    if (!ttibn)
      if ((rc = ttbufr()) <= 0)
	return(rc);

    if (ttibn <= n) {
	len = ttibn;
	memcpy(buf,&ttibuf[ttibp],len);
	ttibp += len;
	ttibn = 0;
    } else {
	/* Watch out, memcpy not portable... */
	memcpy(buf,&ttibuf[ttibp],n);
	ttibp += n;
	ttibn -= n;
	len = n;
    }
#else /* TCPIPLIB */
    for (i = 0; i < n; i++) {
    	if ((j = netinc(0)) < 0) {
	    if (j < -1)
	      return(j);
	    else 
	      break;
	}
    	buf[i] = j;
    }
    len = i;
#endif /* TCPIPLIB */
    return(len);
}

/*  N E T I N C --  Input character from network */

int			
netinc(timo) int timo; {
#ifdef TCPIPLIB
    int x; unsigned char c;		/* The locals. */

    if (ttyfd == -1) {
	debug(F100,"netinc socket is closed","",0);
	return(-2);
    }
    if (ttibn > 0) {			/* Something in internal buffer? */
#ifdef COMMENT
	debug(F100,"netinc char in buf","",0); /* Yes. */
#endif /* COMMENT */
	x = 0;				/* Success. */
    } else {				/* Else must read from network. */
	x = -1;				/* Assume failure. */
#ifdef DEBUG
	debug(F101,"netinc goes to net, timo","",timo);
	ttibuf[ttibp+1] = '\0';
	debug(F111,"netinc ttibuf",ttibuf,ttibp);
#endif /* DEBUG */
	if (timo == 0) {		/* Untimed case. */
	    while (1) {			/* Wait forever if necessary. */
		if (ttbufr() < 0)	/* Refill buffer. */
		  break;		/* Error, fail. */
		if (ttibn > 0) {	/* Success. */
		    x = 0;
		    break;
		}
	    }
	} else {			/* Timed case... */
#ifdef BSDSELECT
            fd_set rfds;
            struct timeval tv;
            FD_ZERO(&rfds);
            FD_SET(ttyfd, &rfds);
            tv.tv_sec  = tv.tv_usec = 0L;
            if (timo < 0)
#ifdef NT
              tv.tv_usec = (long) -timo * 1000L;
#else  /* NT */
              tv.tv_usec = (long) -timo * 10000L;
#endif /* NT */
            else
              tv.tv_sec = timo;
	    debug(F101,"netinc BSDSELECT","",timo);
#ifdef NT
	    WSASafeToCancel = 1;
#endif /* NT */
	    if (select(FD_SETSIZE,
#ifdef __DECC
		       (fd_set *)
#endif /* __DECC */
		       &rfds,
		       NULL, NULL, &tv) > 0 &&
		FD_ISSET(ttyfd, &rfds)) {
#ifdef NT
		WSASafeToCancel = 0;
#endif /* NT */
		while (1) {
		    if (ttbufr() < 0)	/* Keep trying to refill it. */
		      break;		/* Till we get an error. */
		    if (ttibn > 0) {	/* Or we get a character. */
			x = 0;
			break;
		    }
		}
	    }    
#ifdef NT
	    WSASafeToCancel = 0;
#endif /* NT */
#else /* !BSDSELECT */
#ifdef IBMSELECT
/*
  Was used by OS/2, currently not used, but might come in handy some day...
  ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set
  and timeval stuff since this is the only place where it is used.
*/
	    int socket = ttyfd;
	    debug(F101,"netinc IBMSELECT","",timo);
            if (select(&socket, 1, 0, 0,
                        timo < 0 ? -timo : timo * 1000L) == 1)
	      while (1) {
		  if (ttbufr() < 0)	/* Keep trying to refill it. */
		    break;		/* Till we get an error. */
		  if (ttibn > 0) {	/* Or we get a character. */
		      x = 0;
		      break;
		  }
	      }
#else /* !IBMSELECT */
#ifdef WINSOCK
       /* Actually, under WinSock we have a better mechanism than select() */
       /* for setting timeouts (SO_RCVTIMEO, SO_SNDTIMEO) */
	    SOCKET socket = ttyfd;
	    debug(F101,"netinc NTSELECT","",timo);
	    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timo, 
			    sizeof(timo))  == NO_ERROR)
	      while (1) {
		  if (ttbufr() < 0)	/* Keep trying to refill it. */
		    break;		/* Till we get an error. */
		  if (ttibn > 0) {	/* Or we get a character. */
		      x = 0;
		      break;
		  }
	      }
#else /* WINSOCK */
/*
  If we can't use select(), then we use the regular alarm()/signal()
  timeout mechanism.
*/
	    debug(F101,"netinc alarm","",timo);
	    x = alrm_execute(ckjaddr(njbuf),timo,nettout,donetinc,failnetinc);
	    ttimoff();			/* Timer off. */
#endif /* WINSOCK */
#endif /* IBMSELECT */
#endif /* BSDSELECT */
	}
    }
    if (x < 0) {			/* Return -1 if we failed. */
	debug(F100,"netinc timed out","",0);
	return(-1);
    } else {				/* Otherwise */
	ttibn--;			/* Return what we got. */
	c = ttibuf[ttibp++];
	if (deblog) {
#ifdef COMMENT
	    debug(F101,"netinc returning","",c);
#endif /* COMMENT */
	    if (c == 0) {
		debug(F101,"netinc 0 ttibn","",ttibn);
		debug(F101,"netinc 0 ttibp","",ttibp);
	    }
	}
	return((unsigned)(c & 0xff));
    }
#else /* Not using TCPIPLIB */
    return(-1);
#endif /* TCPIPLIB */
}

/*  N E T T O L  --  Output a string of bytes to the network  */
/*
  Call with s = pointer to string, n = length.
  Returns number of bytes actually written on success, or
  -1 on i/o error, -2 if called improperly.
*/

nettol(s,n) char *s; int n; {
#ifdef TCPIPLIB
    int count;
    if (ttyfd == -1) {
	debug(F100,"nettol socket is closed","",0);
	return -1;
    }
    debug(F101,"nettol TCPIPLIB ttnet","",ttnet);
    if (ttnet == NET_TCPB) {
#ifdef BSDSELECT
	fd_set wfds;
	struct timeval tv;
	FD_ZERO(&wfds);
	FD_SET(ttyfd, &wfds);
	tv.tv_sec  = tv.tv_usec = 0L;
	tv.tv_sec = 60;
	debug(F101,"nettol BSDSELECT","",0);
#ifdef NT
	WSASafeToCancel = 1;
#endif /* NT */
	if (!(select(FD_SETSIZE, NULL, &wfds, NULL, &tv) > 0 &&
	       FD_ISSET(ttyfd, &wfds))) {
#ifdef NT
	    WSASafeToCancel = 0;
#endif /* NT */
	    debug(F101,"nettol select failed","",socket_errno);
	    return(-1);
	}
#ifdef NT
	WSASafeToCancel = 0;
#endif /* NT */
#else /* BSDSELECT */
#ifdef IBMSELECT
        {
            int tries = 0;
            while (select(&ttyfd, 0, 1, 0, 1000) != 1) {
                if (tries++ >= 60) {
                    /* if after 60 seconds we can't get permission to write */
                    debug(F101,"nettol select failed","",socket_errno);
                    return(-1);
                }
#ifdef OS2
                {
                    char c;
                    socket_read(ttyfd,&c,0);
                }
#endif /* OS2 */
            }
        }
#endif /* IBMSELECT */
#endif /* BSDSELECT */

	if ((count = socket_write(ttyfd,s,n)) < 1) {
	    debug(F101,"nettol socket_write error","",socket_errno);
	    return(-1);
	}
	debug(F111,"nettol socket_write",s,count);
	return(count);
    } else return(-2);
#else
    debug(F100,"nettol TCPIPLIB not defined","",0);
    return(-2);
#endif /* TCPIPLIB */
}

/*  N E T T O C  --   Output character to network */
/*
  Call with character to be transmitted.
  Returns 0 if transmission was successful, or
  -1 upon i/o error, or -2 if called improperly.
*/
int			
#ifdef CK_ANSIC
nettoc(char c)
#else
nettoc(c) char c;
#endif /* CK_ANSIC */
/* nettoc */ {
#ifdef UNIX
    return(ttoc(c));
#else
#ifdef TCPIPLIB
    unsigned char cc;
    if (ttyfd == -1) {
	debug(F100,"nettoc socket is closed","",0);
	return -1;
    }
    cc = c;
    debug(F101,"nettoc cc","",cc);
    if (ttnet == NET_TCPB) {
#ifdef BSDSELECT
	fd_set wfds;
	struct timeval tv;
	FD_ZERO(&wfds);
	FD_SET(ttyfd, &wfds);
	tv.tv_sec  = tv.tv_usec = 0L;
	tv.tv_sec = 60;
	debug(F101,"nettoc BSDSELECT","",0);
#ifdef NT
	WSASafeToCancel = 1;
#endif /* NT */
	if (!(select(FD_SETSIZE, NULL, &wfds, NULL, &tv) > 0 &&
               FD_ISSET(ttyfd, &wfds))) {
#ifdef NT
	    WSASafeToCancel = 0;
#endif /* NT */
	    debug(F100,"nettoc select failed","",0);
	    return(-1);
	}
#ifdef NT
	WSASafeToCancel = 0;
#endif /* NT */
#else /* BSDSELECT */
#ifdef IBMSELECT
	if (select(&ttyfd, 0, 1, 0, 60) != 1) {
	    debug(F100,"nettoc select failed","",0);
	    return(-1);
	}
#endif /* IBMSELECT */
#endif /* BSDSELECT */
	if (socket_write(ttyfd,&cc,1) < 1) {
	    debug(F101,"nettoc socket_write error","",socket_errno);
	    return(-1);
	}
	debug(F101,"nettoc socket_write","", cc);
	return(0);
    } else return(-2);
#else
    return(-2);
#endif /* TCPIPLIB */
#endif /* UNIX */
}

/*  N E T F L U I  --  Flush network input buffer  */

int
netflui() {
    int n;
#ifdef TCPIPLIB
    ttibuf[ttibp+1] = '\0';
    debug(F111,"netflui 1",ttibuf,ttibn);
    ttibn = ttibp = 0;			/* Flush internal buffer *FIRST* */
    if (ttyfd < 1)
      return(0);
    if ((n = nettchk()) > 0) {		/* Now see what's waiting on the net */
	if (n > TTIBUFL) n = TTIBUFL;	/* and sponge it up */
	debug(F101,"netflui 2","",n);	/* ... */
	n = socket_read(ttyfd,ttibuf,n); /* into our buffer */
	if (n >= 0) ttibuf[n] = '\0';
	debug(F111,"netflui 3",ttibuf,n);
	ttibuf[0] = '\0';
    }
#else
/*
  It seems the UNIX ioctl()s don't do the trick, so we have to read the
  stuff ourselves.  This should be pretty much portable, if not elegant.
*/
    if (ttyfd < 1)
      return(0);
    if ((n = ttchk()) > 0) {
	debug(F101,"netflui","",n);
	while ((n--) && ttinc(0) > -1); /* Don't worry, it's buffered. */
    }
#endif /* TCPIPLIB */
    return(0);
}

#ifdef RLOGCODE			/* TCP/IP RLOGIN protocol support code */
#ifndef OS2
static
#endif /* OS2 */
int
rlog_naws() {
    struct rlog_naws {
	char id[4];
	unsigned short rows, cols, ypix, xpix;
    } nawsbuf;
    
    if (ttnet != NET_TCPB)
      return 0;
    if (ttnproto != NP_RLOGIN)
      return 0;
    if (!nawsflg)
      return 0;

    nawsbuf.id[0] = nawsbuf.id[1] = 0xFF;
    nawsbuf.id[2] = nawsbuf.id[3] = 's';
#ifdef OS2
    nawsbuf.rows = htons((unsigned short) VscrnGetHeight(VTERM)
                          -(tt_status?1:0));
    nawsbuf.cols = htons((unsigned short) VscrnGetWidth(VTERM));
#else /* OS2 */
    nawsbuf.rows = htons((unsigned short) tt_rows);
    nawsbuf.cols = htons((unsigned short) tt_cols);
#endif /* OS2 */
    nawsbuf.ypix = htons(0);		/* y pixels */
    nawsbuf.xpix = htons(0);		/* x pixels */
    if (ttol((CHAR *) &nawsbuf, 12) < 0)
      return -1;
    return 0;
}

static int
rlog_ini() {
    int flag = 0;
#define TERMLEN 16
    extern char uidbuf[];
    char localuser[255];
    int userlen = 0;
    char terminal[TERMLEN+1];
#ifdef CONGSPD
#define CONSPDLEN 16
    char conspeed[CONSPDLEN+1];
    long conspd = -1L;
#endif /* CONGSPD */
#ifdef OS2
    extern int tt_type, max_tt;
    extern struct tt_info_rec tt_info[];
#endif /* OS2 */
    int i, n;

    int rc = 0;
    nawsflg = 0;			/* Assume no NAWS */
#ifdef CK_TTGWSIZ
/*
  But compute the values anyway before the first read since the out-
  of-band NAWS request would arrive before the first data byte (NULL).
*/
#ifdef OS2
    /* Console terminal screen rows and columns */
    debug(F101,"rlog_ini tt_rows 1","",VscrnGetHeight(VTERM)
           -(tt_status?1:0));
    debug(F101,"rlog_ini tt_cols 1","",VscrnGetWidth(VTERM));
    /* Not known yet */
    if (VscrnGetWidth(VTERM) < 0 || 
	VscrnGetHeight(VTERM)-(tt_status?1:0) < 0) {
	ttgwsiz();			/* Try to get screen dimensions */
    }
    debug(F101,"rlog_ini tt_rows 2","",VscrnGetHeight(VTERM)-(tt_status?1:0));
    debug(F101,"rlog_ini tt_cols 2","",VscrnGetWidth(VTERM));
#else /* OS2 */
    debug(F101,"rlog_ini tt_rows 1","",tt_rows);
    debug(F101,"rlog_ini tt_cols 1","",tt_cols);
    if (tt_rows < 0 || tt_cols < 0) {	/* Not known yet */
	ttgwsiz();			/* Try to find out */
    }
    debug(F101,"rlog_ini tt_rows 2","",tt_rows);
    debug(F101,"rlog_ini tt_cols 2","",tt_cols);
#endif /* OS2 */
#endif /* CK_TTGWSIZ */

    rlog_mode = RL_COOKED;
    ttoc(0);				/* Send an initial NUL as wake-up */

    /* Followed by client username ... */

    localuser[0] = '\0';
#ifdef NT
    {
	char localuid[64];
	unsigned long len = 64;
	localuid[0] = '\0';
	WNetGetUser(NULL, localuid, &len);
	ttol(localuid,strlen(localuid)+1);  /* strlen + 1 */
    }
#else /* NT */
    {
	char *s; char *p;
	char * user = getenv("USER");
	debug(F110,"rlogin local user",user,0);
	if (!user)
	  user = "";
	userlen = strlen(user);
	s = user;
	p = (char *)localuser;
#ifdef VMS				/* Convert username to lowercase */
	for (s = user; *s; s++,p++)
	  *p = isupper(*s) ? tolower(*s) : *s;
	*p = '\0';
#else
	while (*p++ = *s++) ;
#endif /* VMS */
	ttol((CHAR *)localuser,userlen+1); /* strlen + 1 */
    }
#endif /* NT */

    /* Then the server userid... */

    if (uidbuf[0])
      ttol((CHAR *) uidbuf,strlen(uidbuf)+1); /* strlen + 1 */
    else if (localuser[0])
      ttol((CHAR *) localuser,userlen+1);
    else
      ttoc(0);

    /* Finally the terminal type and speed */

    terminal[0] = '\0';
    if (tn_term) {			/* SET TELNET TERMINAL-TYPE value */
	if (*tn_term) {			/* (if any) takes precedence. */
	    strncpy(terminal, tn_term, TERMLEN);
	    flag = 1;
	}
    } else {				/* Otherwise the local terminal type */
#ifdef OS2
	/* In terminal-emulating versions, it's the SET TERM TYPE value */
	strncpy(terminal, (tt_type >= 0 && tt_type <= max_tt) ?
		tt_info[tt_type].x_name : "network", TERMLEN);
#else
	/* In the others, we just look at the TERM environment variable */
	{
	    char *p = getenv("TERM");
	    if (p)
	      strncpy(terminal,p,TERMLEN);
	    else
	      terminal[0] = '\0';
#ifdef VMS
	    for (p = (char *) terminal; *p; p++) {
		if (*p == '-' && (!strcmp(p,"-80") || !strcmp(p,"-132")))
		  break;
		else if (isupper(*p))
		  *p = tolower(*p);
	    }
	    *p = '\0';
#endif /* VMS */
	}
#endif /* OS2 */
    }
    n = strlen(terminal);
    if (n > 0) {			/* We have a terminal type */
	if (!flag) {			/* If not user-specified */
	    for (i = 0; i < n; i++)	/* then lowercase it.    */
	      if (isupper(terminal[i]))
		terminal[i] = tolower(terminal[i]);
	}
	ttol((CHAR *)terminal,n);
#ifdef CONGSPD
	/* conspd() is not yet defined in all ck*tio.c modules */
	conspd = congspd();
	if (conspd > 0L) {
	    sprintf(conspeed,"/%ld",conspd);
	    n = strlen(conspeed);
	    ttol((CHAR *)conspeed,n+1);
	} else
#endif /* CONGSPD */
	  ttol((CHAR *)"/19200",7);	/* strlen + 1 */
    } else {
	ttoc(0); 
    }

    /* Now we are supposed to get back a single zero byte as confirmation */
    errno = 0;
    rc = ttinc(60);
    debug(F101,"rlogin first ttinc","",rc);
    if (rc > 0) {
	debug(F101,"rlogin ttinc 1","",rc);
	printf("Rlogin protocol error - 0x%x received instead of 0x00\n", rc);
	return(-1);
    } else if (rc < 0) {
	debug(F101,"rlogin ttinc errno","",errno);
	/* printf("Network error: %d\n", errno); */
	return(-1);
    }	
    return(0);
}

#ifdef TCPIPLIB
static VOID
rlog_oob(oobdata, count) CHAR * oobdata; int count; {
    int i;

    for (i = 0; i<count; i++)   {
	debug(F101,"rlogin out_of_band","",oobdata[i]);
	if (oobdata[i] == 0x02) { /* Flush Buffered Data not yet displayed */
	    debug(F101,"rlogin Flush Buffered Data command","",oobdata[i]);
	    ttflui();
	}
	if (oobdata[i] & 0x10) {	/* Switch to RAW mode */
	    debug(F101,"rlogin Raw Mode command","",oobdata[i]);
	    rlog_mode = RL_RAW;
	}

	if (oobdata[i] & 0x20) {	/* Switch to COOKED mode */
	    debug(F101,"rlogin Cooked Mode command","",oobdata[i]);
	    rlog_mode = RL_COOKED;
	}
	if (oobdata[i] & 0x80) {	/* Send Window Size Info */
	    debug(F101,"rlogin Window Size command","",oobdata[i]);
	    /* Remember to send WS Info when Window Size changes */
	    nawsflg = 1;
	    rlog_naws();
	}
    }
}
#else /* TCPIPLIB */
static SIGTYP
rlogoobh(sig) int sig; {
#ifdef SOLARIS
    char				/* Or should it be char for all? */
#else
    CHAR
#endif /* SOLARIS */
      oobdata;

    int  count = 0;

    while (recv(ttyfd, &oobdata, 1, MSG_OOB) < 0) { 
      /* 
       * We need to do some special processing here.
       * Just in case the socket is blocked for input
       * 
       */
	switch (errno) {
	  case EWOULDBLOCK:
	    break;
	  default:
	    return;
	}
    }
    debug(F101,"rlogin out_of_band","",oobdata);
    if (oobdata == 0x02) {	/* Flush Buffered Data not yet displayed */
	debug(F101,"rlogin Flush Buffered Data command","",oobdata);
	netflui();
    }
    if (oobdata & 0x10) {		/* Switch to raw mode */
	debug(F101,"rlogin Raw Mode command","",oobdata);
	rlog_mode = RL_RAW;
    }
    if (oobdata & 0x20) {		/* Switch to cooked mode */
	debug(F101,"rlogin Cooked Mode command","",oobdata);
	rlog_mode = RL_COOKED;
    }
    if (oobdata & 0x80) {		  /* Send Window Size Info */
	debug(F101,"rlogin Window Size command","",oobdata);
	nawsflg = 1; /* Remember to send WS Info when Window Size changes */
	rlog_naws();
    }
}
#endif /* TCPIPLIB */
#endif /* RLOGCODE */

#ifdef TNCODE				/* Compile in telnet support code */

/* TCP/IP TELNET protocol negotiation support code */

static int sgaflg = 0;			/* SUPRRESS GO-AHEAD state */
static int ttyflg = 0;			/* TERMINAL TYPE state */

static int wnawsflg = 0;		/* Initial WILL NAWS sent, no reply */
static int wmebinflg = 0,  /* initial WILL BINARY sent, no reply */
           dubinflg  = 0;  /* initial DO BINARY sent, no reply */

#ifndef TELCMDS
char *telcmds[] = {
    "SE", "NOP", "DMARK", "BRK",  "IP",   "AO", "AYT",  "EC",
    "EL", "GA",  "SB",    "WILL", "WONT", "DO", "DONT", "IAC"
};
int ntelcmds = sizeof(telcmds) / sizeof(char *);
#endif /* TELCMDS */

/*
   The following list is current as of October 1994 ASSIGNED NUMBERS
   RFC 1700.
*/

char *tnopts[] = {
    "BINARY",                           /* RFC 856 */
    "ECHO",                             /* RFC 857 */
    "RECONNECTION",                     /* NIC 50005 */
    "SUPPRESS-GO-AHEAD",                /* RFC 858 */
    "APPROX-MESSAGE-SIZE",              /* ETHERNET */
    "STATUS",                           /* RFC 859 */
    "TIMING-MARK",                      /* RFC 860 */
    "REMOTE-CONTROL-TRANS-ECHO",        /* RFC 726 */
    "OPTION-LINE-WIDTH",                /* NIC 50005 */
    "OPTION-PAGE-SIZE",                 /* NIC 50005 */
    "OUTPUT-CR-DISPOSITION",            /* RFC 652 */
    "OUTPUT-HORIZ-TABSTOPS",            /* RFC 653 */
    "OUTPUT-HORIZ-TAB-DISPOSITION",     /* RFC 654 */
    "OUTPUT-FF-DISPOSITION",            /* RFC 655 */
    "OUTPUT-VERT-TABSTOPS",             /* RFC 666 */
    "OUTPUT-VERT-TAB-DISPOSITION",      /* RFC 667 */
    "OUTPUT-LF-DISPOSITION",            /* RFC 668 */
    "EXTENDED-ASCII",                   /* RFC 698 */
    "LOGOUT",                           /* RFC 727 */
    "BYTE-MACRO",                       /* RFC 735 */
    "DATA-ENTRY-TERMINAL",              /* RFC 1043, RFC 732 */
    "SUPDUP",                           /* RFC 736, RFC 734 */
    "SUPDUP-OUTPUT",                    /* RFC 749 */
    "SEND-LOCATION",                    /* RFC 779 */
    "TERMINAL-TYPE",                    /* RFC 1091 */
    "END-OF-RECORD",                    /* RFC 885 */
    "TACACS-UID",                       /* RFC 927 */
    "OUTPUT-MARKING",                   /* RFC 933 */
    "TERMINAL-LOCATION-NUMBER",         /* RFC 946 */
    "TELNET-3270-REGIME",               /* RFC 1041 */
    "X.3-PAD",                          /* RFC 1053 */
    "NEGOTIATE-ABOUT-WINDOW-SIZE",      /* RFC 1073 */
    "TERMINAL-SPEED",                   /* RFC 1079 */
    "REMOTE-FLOW-CONTROL",              /* RFC 1372 */
    "LINEMODE",                         /* RFC 1184 */
    "X-DISPLAY-LOCATION",               /* RFC 1096 */
    "ENVIRONMENT",                      /* RFC 1408 */
    "AUTHENTICATION",                   /* RFC 1409 */
    "ENCRYPTION",                       /* No Reference */
    "NEW-ENVIRONMENT",                  /* RFC 1572 */
    "TN3270E"                           /* RFC 1647 */
#ifdef COMMENT
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "UNKNOWN",
    "EXTENDED-OPTIONS-LIST"		/* 255, RFC 861 */
#endif /* COMMENT */
};

/*
   Telnet Authentication Types

   In [RFC1409], a list of authentication types is introduced.  Additions
   to the list are registerd by the IANA and documented here.

   Type       Description                  Reference
     0        NULL                         [RFC1409]
     1        KERBEROS_V4                  [RFC1409]
     2        KERBEROS_V5                  [RFC1409]
     3        SPX                          [RFC1409]
     4-5      Unassigned
     6        RSA                          [RFC1409]
     7-9      Unassigned
    10        LOKI                         [RFC1409]
    11        SSA                          [Schoch]
*/

/*
  In order to prevent an infinite telnet negotiation loop we maintain a
  count of the number of times the same telnet negotiation message is
  sent. When this count hits MAXTNCNT, we do not send any more of the
  message. The count is stored in the tncnts[][] array.
  
  The tncnts[][] array is indexed by negotiation option (SUPPRESS GO AHEAD,
  TERMINAL TYPE, NAWS, etc. - see the tnopts[] array) and the four
  negotiation message types (WILL, WONT, DO, DONT).  All telnet negotiations
  are kept track of in this way.

  The count for a message is zeroed when the "opposite" message is sent.
  WILL is the opposite of WONT, and DO is the opposite of DONT.
  For example sending "WILL SGA" increments tncnts[TELOPT_SGA][0]
  and zeroes tncnts[TELOPT_SGA][1].

  The code that does this is in tn_sopt().

  rogersh@fsj.co.jp, 18/3/1995
*/

#define MAXTNCNT 4	/* Permits 4 intermediate telnet firewalls/gateways */

char tncnts[sizeof(tnopts) / sizeof(char *)][4];	/* counts */
char tnopps[4] = { 1,0,3,2 };				/* opposites */

int ntnopts = sizeof(tnopts) / sizeof(char *);
#endif /* TNCODE */

/* Send network BREAK */
/*
  Returns -1 on error, 0 if nothing happens, 1 if BREAK sent successfully.
*/
int
netbreak() {
    CHAR buf[3];
    if (ttnet == NET_TCPB) {
	if (ttnproto == NP_TELNET) {
#ifdef TNCODE
	    buf[0] = (CHAR) IAC; buf[1] = (CHAR) BREAK; buf[2] = (CHAR) 0;
	    if (
#ifdef OS2
		nettol((char *) buf, 2)
#else
		ttol(buf, 2)
#endif /* OS2 */
		< 2)
	      return(-1);
	    debug(F101,"telnet BREAK ok","",BREAK);
	    return(1);
#else
	    debug(F100,"netbreak no TNCODE","",0);
	    return(0);
#endif /* TNCODE */
	}
	/* Insert other TCP/IP protocols here */
    }
    /* Insert other networks here */
    return(0);
}

/* Send a telnet option, avoid loops. */
/* Returns 1 if command was sent, 0 if not, -1 on error */

int
tn_sopt(cmd,opt) int cmd, opt; {	/* TELNET SEND OPTION */
    CHAR buf[5];
    int n,m;

    if (ttnet != NET_TCPB) return(0);	/* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */
#ifdef TNCODE
    n = cmd - SE;
    m = cmd - WILL;
    if (n < 0 || n > ntelcmds) return(0);
    if (m >= 0 && m < 4 && opt < ntnopts) {	/* See comment above about   */
	if (tncnts[opt][m] > MAXTNCNT) {	/* preventing infinite loops */
	    if (debses || deblog) {
	    	sprintf(tn_msg,"TELNET negotiation loop %s %s",telcmds[n],
			tnopts[opt]);
		debug(F101,tn_msg,"",opt);
		if (debses) tn_debug(tn_msg);
	    }
	    return(0);
	}
	tncnts[opt][m]++;
	tncnts[opt][tnopps[m]] = 0;
    }
    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) (cmd & 0xff);
    buf[2] = (CHAR) (opt & 0xff);
    buf[3] = (CHAR) 0;
    if (ttol(buf,3) < 3)
      return(-1);
    if ((debses || deblog) && cmd != SB) {
	sprintf(tn_msg,"TELNET SENT %s %s",telcmds[n],
		(opt < ntnopts) ? tnopts[opt] : "UNKNOWN");
	debug(F101,tn_msg,"",opt);
	if (debses) tn_debug(tn_msg);
    }
    return(1);
#else
    debug(F100,"tn_sopt no TNCODE","",0);
    return(0);
#endif /* TNCODE */
}

/* Initialize a telnet connection. */
/* Returns -1 on error, 0 if nothing happens, 1 if init msgs sent ok */

int
tn_ini() {
    int x,i,j;
#ifndef TNCODE
    debug(F100,"tn_ini no TNCODE","",0);
    return(0);
#else /* TELNET protocol support */
    debug(F101,"tn_ini ttnproto","",ttnproto);
    debug(F101,"tn_ini tn_init","",tn_init);

    if (ttnet != NET_TCPB)		/* Make sure connection is TCP/IP */
      return(0);
    if (ttnproto == NP_RLOGIN)
      return(0);
    if (tn_init)			/* Have we done this already? */
      return(0);			/* Don't do it again. */

    /* Reset the TELNET OPTIONS counts */
    for (i = 0; i < sizeof(tnopts) / sizeof(char *); i++)
        for (j = 0; j < 4; j ++)
            tncnts[i][j] = 0;

    debug(F101,"tn_ini tn_duplex","",tn_duplex);
    duplex = tn_duplex;			/* Assume local echo. */
    sgaflg = 0;				/* Assume Go-Ahead suppressed. */
    nawsflg = 0;			/* Assume NAWS should not be sent. */
    if (ttnproto == NP_NONE) {		/* If not talking to a telnet port, */
	ttnproto = NP_TELNET;		/* pretend it's telnet anyway, */
	tn_init = 1;			/* but don't send initial options. */
	debug(F100,"tn_ini skipping telnet negotiations","",0);
	return(0);
    }
    debug(F100,"tn_ini about to send WILL TTYPE","",0);
/* 
  Talking to TELNET port, so send WILL TERMINAL TYPE and DO SGA.
  Also send WILL NAWS if we know our screen dimensions.
*/
    if ((x = tn_sopt(WILL,TELOPT_TTYPE)) < 0) { /* Will send terminal type. */
	debug(F101,"tn_ini tn_sopt WILL TTYPE failed","",x);
	return(-1);
    }
    debug(F100,"tn_ini sent WILL TTYPE ok","",0);
    ttyflg = 1;				/* Remember I said I would. */
#ifdef OS2
    ttnum = -1;
    ttnumend = 0;
#endif /* OS2 */
#ifdef CK_NAWS
    /* Console terminal screen rows and columns */
#ifdef OS2
    debug(F101,"tn_ini tt_rows 1","",VscrnGetHeight(VTERM)-(tt_status?1:0));
    debug(F101,"tn_ini tt_cols 1","",VscrnGetWidth(VTERM));
    /* Not known yet */
    if (VscrnGetWidth(VTERM) < 0 ||
	VscrnGetHeight(VTERM)-(tt_status?1:0) < 0) {
	ttgwsiz();			/* Try to find out */
    }
    debug(F101,"tn_ini tt_rows 2","",VscrnGetHeight(VTERM)-(tt_status?1:0));
    debug(F101,"tn_ini tt_cols 2","",VscrnGetWidth(VTERM));
    /* Now do we know? */
    if (VscrnGetWidth(VTERM) > 0 &&
	VscrnGetHeight(VTERM)-(tt_status?1:0) > 0) {
	if (tn_sopt(WILL, TELOPT_NAWS) < 0)
	  return(-1);
	wnawsflg = 1;			/* OK, we sent an initial WILL NAWS. */
    }
#else /* OS2 */
    debug(F101,"tn_ini tt_rows 1","",tt_rows);
    debug(F101,"tn_ini tt_cols 1","",tt_cols);
    if (tt_rows < 0 || tt_cols < 0) {	/* Not known yet */
	ttgwsiz();			/* Try to find out */
    }
    debug(F101,"tn_ini tt_rows 2","",tt_rows);
    debug(F101,"tn_ini tt_cols 2","",tt_cols);
    if (tt_rows > 0 && tt_cols > 0) {	/* Now do we know? */
	if (tn_sopt(WILL, TELOPT_NAWS) < 0)
	  return(-1);
	wnawsflg = 1;			/* Ok, we sent an initial WILL NAWS. */
    }
#endif /* OS2 */
#endif /* CK_NAWS */

    if (tn_binary == TN_BM_RQ) {
        if (tn_sopt(WILL, TELOPT_BINARY) < 0)
	  return(-1);
        wmebinflg = 1;			/* We sent an initial WILL BINARY. */
        if (tn_sopt(DO, TELOPT_BINARY) < 0)
	  return(-1);
        dubinflg = 1;			/* We sent an initial DO BINARY. */
    }
    me_binary = 0;			/* We are not in binary mode */
    u_binary = 0;


#ifdef CK_ENVIRONMENT
    /* Will send terminal environment. */
    if ((x = tn_sopt(WILL,TELOPT_NEWENVIRON)) < 0) {
	debug(F101,"tn_ini tn_sopt WILL NEWENVIRON failed","",x);
	return(-1);
    }
    debug(F100,"tn_ini sent WILL NEWENVIRON ok","",0);
#endif /* CK_ENVIRONMENT */

    if (tn_sopt(DO,TELOPT_SGA) < 0)	/* Please suppress go-ahead. */
      return(-1);

    tn_init = 1;			/* Set telnet-initialized flag. */

    /* Don't send anthing else! */

    debug(F101,"tn_ini duplex","",duplex);
    debug(F101,"tn_ini done, tn_init","",tn_init);
    return(1);
#endif /* TNCODE */
}

static VOID
tn_debug(s) char *s; {
#ifdef OS2
    void cwrite(unsigned short);
    char *p = s;
    _PROTOTYP (void os2bold, (void));
#endif /* OS2 */

#ifdef OS2
    debug(F111,"tn_debug",s,0);
    if (debses == 0) /* Emulator is always active in OS/2 */
      return;

    if (!scrninitialized[VTERM]) {
	USHORT x,y;
	checkscreenmode();
	GetCurPos(&y, &x);
	SaveCmdMode(x+1,y+1);
	scrninit();
	RestoreCmdMode();
    }
    os2bold();				/* Toggle boldness */
    while (*p) 
      cwrite((CHAR) *p++);		/* Go boldly ... */
    os2bold();				/* Toggle boldness back */
    debses = 0;
    cwrite((CHAR) '\015');
    cwrite((CHAR) '\012');
    debses = 1;
#else
    debug(F111,"tn_debug",s,what);
    if (what != W_CONNECT || debses == 0) /* CONNECT command must be active */
      return;
    conoll(s);
#endif /* OS2 */
}

/*
  Process in-band Telnet negotiation characters from the remote host.
  Call with the telnet IAC character and the current duplex setting
  (0 = remote echo, 1 = local echo), and a pointer to a function to call
  to read more characters.  Returns:
    3 if server has sent us a quoted IAC
    2 if local echo must be changed to remote
    1 if remote echo must be changed to local
    0 if nothing happens or no action necessary
   -1 on failure (= internal or i/o error)
*/

#ifdef CK_ENVIRONMENT
#define TSBUFSIZ 1024
char tn_env_acct[64];
char tn_env_disp[64];
char tn_env_job[64];
char tn_env_prnt[64];
char tn_env_sys[64];
extern char uidbuf[];
#else /* CK_ENVIRONMENT */
#define TSBUFSIZ 41
#endif /* CK_ENVIRONMENT */

unsigned char sb[TSBUFSIZ];		/* Buffer for subnegotiations */

int
#ifdef CK_ANSIC				/* TELNET DO OPTION */
tn_doop(CHAR z, int echo, int (*fn)(int))
#else
tn_doop(z, echo, fn) CHAR z; int echo; int (*fn)();
#endif /* CK_ANSIC */
/* tn_doop */ {
    int c, x, y, n, m, flag;

#ifndef TNCODE
    debug(F100,"tn_doop no TNCODE","",0);
    return(0);
#else
    if (z != (CHAR) IAC) {
	debug(F101,"tn_doop bad call","",z);
	return(-1);
    }
    if (ttnet != NET_TCPB)		/* Check network type */
      return(0);
    if (ttnproto != NP_TELNET)		/* Check protocol */
      return(0);

/* Have IAC, read command character. */

    c = (*fn)(0) & 0xff;		/* Read command character */
    m = c - SE;				/* Check validity */
    if (m < 0 || m > ntelcmds) {
	debug(F101,"tn_doop bad cmd","",c);
	return(0);
    }
    if (seslog && sessft) {		/* Copy to session log, if any. */
	if (zchout(ZSFILE, (char) z) < 0) /* Log IAC. */
	  seslog = 0;
	else if (zchout(ZSFILE, (char) c) < 0) /* Log command */
	  seslog = 0;
    }
    if (c == (CHAR) IAC)		/* Quoted IAC */
      return(3);
    if (c < SB)				/* Other command with no arguments. */
      return(0);

/* SB, WILL, WONT, DO, or DONT need more bytes... */

    if ((x = (*fn)(0)) < 0)		/* Get the option. */
      return(-1);
    x &= 0xff;				/* Trim to 8 bits. */

    if (seslog && sessft)		/* Session log */
      if (zchout(ZSFILE, (char) x) < 0)
	seslog = 0;

    if ((deblog || debses) && c != SB) {
	sprintf(tn_msg,"TELNET RCVD %s %s",telcmds[m],
		(x < ntnopts) ? tnopts[x] : "UNKNOWN");
	debug(F101,tn_msg,"",x);
	if (debses) tn_debug(tn_msg);
    }

    /* Now handle the command */

    switch (x) {
      case TELOPT_BINARY:		/* TELNET BINARY mode. */
	switch (c) {			/* Command... */
	  case WILL:
	    if (tn_binary == TN_BM_RF) { /* If binary mode disabled */
		if (tn_sopt(DONT,x) < 0) 
		  return(-1);
		else 
		  return(0);
	    } else if (!u_binary) {
		u_binary = 1;
		if (!dubinflg) {	/* Reply only if we did not initiate */
		    if (tn_sopt(DO,x) < 0)   
		      return -1;
		} else {
		    dubinflg = 0;
		}
		u_binary = 1;
	    }
	    return(0);
	  case WONT:
	    if (u_binary) {
		u_binary = 0;
		if (tn_sopt(DONT,x) < 0) 
		  return (-1);
		else
		  return(0);
	    } else return(0);
	  case DO:
	    if (tn_binary == TN_BM_RF) {
		if (tn_sopt(WONT,x) < 0) 
		  return (-1);
		else
		  return (0);
	    } else if (!me_binary) {
		if (!wmebinflg) {	/* Reply only if we did not initiate */
		    if (tn_sopt(WILL,x) < 0)   
		      return -1;
		} else {
		    wmebinflg = 0;
		}
		me_binary = 1;
		if (tn_binary == TN_BM_RQ && !u_binary) {
		    if (!dubinflg) {
			if (tn_sopt(DO, TELOPT_BINARY) < 0)
			  return(-1);
			dubinflg = 1;	/* We sent an initial DO BINARY. */
		    }
		}
	    }
	    return(0);
	  case DONT:
	    if (me_binary) {
		me_binary = 0;
		if (tn_sopt(WONT,x) < 0) 
		  return(-1);
		else
		  return(0);
	    } else
	      return(0);
	  default:
	    return(0);
	}

      case TELOPT_ECHO:			/* ECHO mode. */
	switch (c) {			/* Command... */
	  case WILL:			/* Host says it will echo.           */
	    if (echo) {			/* We're locally echoing right now,  */
		if (tn_sopt(DO,x) < 0)	/* so switch to remote.  */
		  return (-1); 
		else 
		  return(2);
	    } else
	      return(0);		/* We're already remote echoing.     */
	  case WONT:			/* Host says it won't echo.          */
	    if (!echo) {		/* We're remote echoing right now,   */
		if (tn_sopt(DONT,x) < 0) /* so switch to local. */
		  return (-1);
		else
		  return (1);
	    } else			/* We're already locally echoing.    */
	      return(0);
	  case DO:			/* Host wants me to echo.            */
	  case DONT:			/* Host doesn't want me to echo.     */
	    				/* ...But the client never echoes.   */
	      if (tn_sopt(WONT,x) < 0)
		return (-1);
	      else
		return (0);

	  default:
	    return(0);
	}

      case TELOPT_SGA:			/* Suppress Go-Ahead */
	switch (c) {			/* Command... */
	  case WILL:			/* Server says it will SGA.          */
#ifdef COMMENT
	    if (sgaflg) {		/* Remember new SGA state, but       */
#endif /* COMMENT */
	      sgaflg = 0;		/* don't change echo state.          */
	      if (tn_sopt(DO,x) < 0)
		return(-1);
	      else
		return(0);
#ifdef COMMENT
	    } else return(0);
#endif /* COMMENT */
	  case WONT:			/* Server says it won't SGA.         */
#ifdef COMMENT
	    if (!sgaflg) {		/* Remember new SGA state, and       */
#endif /* COMMENT */
	      sgaflg = 1;		/* switch to local echo if needed.   */
	      if (tn_sopt(DONT,x) < 0)
		return(-1);
#ifdef COMMENT
	    }
#endif /* COMMENT */
	    return(echo ? 0 : 1);
/*
  Note: The concerns expressed in the older comment below are now taken
  care of by the negotiation loop prevention code in tn_sopt() - see the
  comment about this near the definition of MAXTNCNT, above.
*/
/*
  Note: The following is proper behavior, and required for talking to the
  Apertus interface to the NOTIS library system, e.g. at Iowa State U:
  scholar.iastate.edu.  Without this reply, the server hangs forever.  This
  code should not be loop-inducing, since C-Kermit never sends WILL SGA as
  an initial bid, so if DO SGA comes, it is never an ACK.
*/
	  case DO:			/* Server wants me to SGA,           */
					/* so I will.                        */
	  case DONT:			/* Server wants me not to SGA,       */
					/* but I will anyway.                */
	    if (tn_sopt(WILL,x) < 0) 
	      return(-1);
	    else 
	      return (0);

	  default:
	    return(0);
	}

#ifdef TELOPT_TTYPE
      case TELOPT_TTYPE:		/* Terminal Type */
	switch (c) {
	  case DONT:
	    if (ttyflg) {
	      ttyflg = 0;
	      if (tn_sopt(WONT,x) < 0) 
		return (-1);
	      else 
		return (0);
	    } else return(0);
	  case DO:
	    if (!ttyflg) {
	      ttyflg = 1;
	      if (tn_sopt(WILL,x) < 0) 
		return(-1);
	      else 
		return (0);
	    } else
	      return(0);
	  case WILL:
	  case WONT:
	    if (!ttyflg) {
		ttyflg = 1;
		if (tn_sopt(DONT,x) < 0) 
		  return(-1);
		else 
		  return (0);
	    } else
	      return(0);
	  case SB:
	    n = flag = 0;		/* Flag for when done reading SB */
	    while (n < TSBUFSIZ) {	/* Loop looking for IAC SE */
		if ((y = (*fn)(0)) < 0)	/* Read a byte */
		  return(y);
		y &= 0xff;		/* Make sure it's just 8 bits. */
		sb[n++] = (char) y;	/* Deposit in buffer. */
		if (seslog && sessft)	/* Take care of session log */
		  if (zchout(ZSFILE, (char) y) < 0)
		    seslog = 0;
		if (y == IAC) {		/* If this is an IAC */
		    if (flag) {		/* If previous char was IAC */
			n--;		/* it's quoted, keep one IAC */
			flag = 0;	/* and turn off the flag. */
		    } else flag = 1;	/* Otherwise set the flag. */
		} else if (flag) {	/* Something else following IAC */
		    if (y == SE)	/* If not SE, it's a protocol error */
		      break;
		    else if ( y == DONT ) { /* Used DONT instead of SE */
			debug(F100,
"TELNET Subnegotiation error - used DONT instead of SE!",
			      "",0);
			if (debses)
			  tn_debug(
"TELNET Subnegotiation error - used DONT instead of SE!");
			flag = 3;
			break;
		    } else {		/* Other protocol error */
			flag = 0;
			break;
		    }
		} else if (!flag && y == SE) { /* Forgot the IAC ? */
		    flag = 2;
		    debug(F100,
"TELNET Subnegotiation error - forgot the IAC before SE!",
			  "",0);
		    if (debses)
		      tn_debug(
"TELNET Subnegotiation error - forgot the IAC before SE!");
		    break;
		}
	    }
	    if (!flag) {		/* Make sure we got a valid SB */
		debug(F100, "TELNET Subnegotiation prematurely broken", "",0);
		if (debses) 
		  tn_debug("TELNET Subnegotiation prematurely broken");
		return(0);	/* Was -1 but that would be taken as */
		                /* an I/O error, so absorb it and go on. */
	    }
	    if (deblog || debses) {
		int i;
		sprintf(tn_msg,"TELNET RCVD SB %s %s ",tnopts[TELOPT_TTYPE],
			sb[0] ? "SEND" : "IS");
		for (i = 1; i < n-2; i++) {
		    sprintf(hexbuf,"%c",sb[i]);
		    strcat(tn_msg,hexbuf);
		}
		if (flag == 2)
		  strcat(tn_msg," SE");
		else if (flag == 3)
		  strcat(tn_msg," IAC DONT");
		else 
		  strcat(tn_msg," IAC SE");
		debug(F100,tn_msg,"",0);
		if (debses) tn_debug(tn_msg);
	    }
	    if (sb[0] == 1) {		/* SEND terminal type? */
		if (tn_sttyp() < 0)	/* Yes, so send it. */
		  return(-1);
	    }
#ifdef OS2
	    else {  /* IS terminal type -- host has chosen from the list */
		int i=0;

		/* isolate the specified terminal type string */
		while (sb[i++] != IAC) {
		    if (i >= TSBUFSIZ)
		      return (-1);
		    if (sb[i] == IAC) {
			sb[i] = '\0';
			break;
                    }
                }
		strupr(&(sb[1])); /* Upper case it */
		for (i=0;i<=max_tt;i++) {    /* find it in our list */
		    if (!strcmp(&(sb[1]),tt_info[i].x_name)) {
			/* Set terminal type to the one chosen */
			settermtype(i,0);
			break;
		    }
                }
            }
#endif /* OS2 */
	  default:			/* Others, ignore */
	    return(0);
	}
#endif /* TELOPT_TTYPE */

#ifdef CK_NAWS
      case TELOPT_NAWS:			/* Terminal width and height */
	switch (c) {
	  case DO:
	    /* Get window size again in case it changed */
	    if (ttgwsiz() > 0) {
		if (!wnawsflg) {	/* Reply WILL only if we */
		    if (tn_sopt(WILL,x) < 0) /* didn't initiate this */
		      return(-1);	/* negotiation with a WILL */
		} else
		    wnawsflg = 0;

		nawsflg = 1;
#ifndef NOSIGWINCH
#ifdef SIGWINCH
#ifdef UNIX
		if (sw_armed++ < 1) {	/* Catch window-size changes. */ 
		    debug(F100,"tn_doop arming SIGWINCH","",0);
		    signal(SIGWINCH,winchh);
		}
#else
		debug(F100,"SIGWINCH defined but not used","",0);
#endif /* UNIX */    
#endif /* SIGWINCH */
#endif /* NOSIGWINCH */
		return((tn_snaws() < 0) 
#ifdef RLOGCODE
		       || (rlog_naws() < 0)
#endif /* RLOGCODE */
		       ? -1 : 0); /* And now do it. */
	    } else {
		nawsflg = 0;
		wnawsflg = 0;
		return((tn_sopt(WONT,x) < 0) ? -1 : 0);
	    }
	  case DONT:
	    nawsflg = 0;
	    wnawsflg = 0;
	    return ((tn_sopt(WONT,x) < 0) ? -1 : 0);
	  case WILL:	/* For when we are a server */
	  case WONT:
	    return ((tn_sopt(DONT,x) < 0) ? -1 : 0);
	  default:
	    return(0);
	}
#endif /* CK_NAWS */

#ifdef CK_ENVIRONMENT
      case TELOPT_NEWENVIRON: 		/* Telnet New-Environment */
	switch (c) {
	  case DO:
	    return ((tn_sopt(WILL,x) < 0) ? -1 : 0);
	  case DONT:
	    return ((tn_sopt(WONT,x) < 0) ? -1 : 0);
	  case WILL: /* For when we are a server */
	  case WONT:
	    return ((tn_sopt(DONT,x) < 0) ? -1 : 0);
	  case SB: {
	      n = flag = 0;		/* Flag for when done reading SB */
	      while (n < TSBUFSIZ) {	/* Loop looking for IAC SE */
		  if ((y = (*fn)(0)) < 0) /* Read a byte */
		    return(y);
		  y &= 0xff;		/* Make sure it's just 8 bits. */
		  sb[n++] = (char) y;	/* Deposit in buffer. */
		  if (seslog && sessft)	/* Take care of session log */
		    if (zchout(ZSFILE, (char) y) < 0)
		      seslog = 0;
		  if (y == IAC) {	/* If this is an IAC */
		      if (flag) {	/* If previous char was IAC */
			  n--;		/* it's quoted, keep one IAC */
			  flag = 0;	/* and turn off the flag. */
		      } else flag = 1;	/* Otherwise set the flag. */
		  } else if (flag) {	/* Something else following IAC */
		      if (y == SE)	/* If not SE, it's a protocol error */
			break;
		      else if (y == DONT) { /* Used DONT instead of SE */
			  debug(F100,
			  "TELNET SB error - got DONT instead of SE!",
				 "",0);
			  if (debses)
			    tn_debug(
"TELNET Subnegotiation error - got DONT instead of SE!");
			  flag = 3;
			  break;
		      } else {		/* Other protocol error */
			  flag = 0;
			  break;
		      }
		  } else if (!flag && y == SE) { /* Forgot the IAC ? */
		      flag = 2;
		      debug(F100,
"TELNET Subnegotiation error - forgot the IAC before SE!",
			    "",0);
		      if (debses)
			tn_debug(
"TELNET Subnegotiation error - forgot the IAC before SE!");
		      break;
		  }
	      }
	      if (!flag) {		/* Make sure we got a valid SB */
		  debug(F100,"TELNET Subnegotiation prematurely broken","",0);
		  if (debses) 
		    tn_debug("TELNET Subnegotiation prematurely broken");
		  return(0);		/* Was -1 but that would be taken as */
					/* an I/O error, so absorb & go on. */
	      }
	      if (deblog || debses) {
		  int i;
		  sprintf(tn_msg,
			  "TELNET RCVD SB %s %s ",
			  tnopts[TELOPT_NEWENVIRON],
			  sb[0] == 1 ? "SEND" : sb[0] == 0 ? "IS" : "INFO"
			  );
		  for (i = 1; i < n-2; i++) {
		      sprintf(hexbuf,"%c",sb[i]);
		      strcat(tn_msg,hexbuf);
		  }
		  if (flag == 2)
		    strcat(tn_msg," SE");
		  else if (flag == 3)
		    strcat(tn_msg," IAC DONT");
		  else 
		    strcat(tn_msg," IAC SE");
		  debug(F100,tn_msg,"",0);
		  if (debses) tn_debug(tn_msg);
	      }
	      switch (sb[0]) {
		case 0:			/* IS */
		  /* Ignore, we're not a server - yet */
		  break;
		case 1:			/* SEND */
		  /* We need to take the sb[] and build a structure */
		  /* containing all of the variables and types that */
		  /* we are supposed to keep track of and send to   */
		  /* the host, then call tn_snenv().                */
		  /* Or we can punt ...                             */
		  if (tn_snenv(&sb[1],n-3) < 0)	/* Yes, so send it. */
		    return(-1);
		  break;
		case 2:			/* INFO */
		  /* Ignore, we're not a server - yet */
		  break;
	      }
	  }
	  default:
	    return(0);
	}
#endif /* CK_ENVIRONMENT */

      default:				/* All others: refuse */
	switch(c) {
	  case WILL:			/* You will? */
	    return((tn_sopt(DONT,x) < 0) ? -1 : 0); /* Please don't. */
	  case WONT:			/* You won't? */
	    return(0);			/* I didn't want you to. */
	  case DO:			/* You want me to? */
	  case DONT:			/* You don't want me to? */
	    return((tn_sopt(WONT,x) < 0) ? -1 : 0); /* I won't */
	  default:
	    return(0);
	}
    }
#endif /* TNCODE */
}

#ifdef CK_ENVIRONMENT
/* Telnet send new environment */
/* Returns -1 on error, 0 if nothing happens, 1 on success */

int
#ifdef CK_ANSIC
tn_snenv(char * sb, int len) 
#else
tn_snenv() char * sb; int len;
#endif /* CK_ANSIC */
/* tn_snenv */ { 			/* Send new environment */
#ifndef TNCODE
    debug(F100,"tn_snenv no TNCODE","",0);
    return(0);
#else
    char varname[16];
    char * reply = 0, * s = 0;
    int i,j,n;				/* Worker. */
    int type = 0;	/* 0 for NONE, 1 for VAR, 2 for USERVAR in progress */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    /* First determine the size of the buffer we will need */
    for (i = 0, j = 0, n = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
	switch (sb[i]) {
	  case 0:			/* VAR */
	  case 3:			/* USERVAR */
	  case IAC:			/* End of the list */
	    switch (type) {
	      case 0:			/* Nothing in progress */
		/* If we get IAC only, then that means send all */
		/* VAR and USERVAR.  But since we don't support */
		/* USERVAR yet, we can just pass through        */
		if (!(j == 0 && sb[i] == IAC))
		  break;
	      case 1:			/* VAR in progress */
		varname[j] = '\0' ;
		if (!varname[0]) {	/* Send All */
		    if (uidbuf[0])
		      n += strlen(uidbuf) + 4 + 2;
		    if (tn_env_job[0])
		      n += strlen(tn_env_job) + 3 + 2;
		    if (tn_env_acct[0])
		      n += strlen(tn_env_acct) + 4 + 2;    
		    if (tn_env_prnt[0])
		      n += strlen(tn_env_prnt) + 7 + 2;
		    if (tn_env_sys[0])
		      n += strlen(tn_env_sys) + 10 + 2;
		    if (tn_env_disp[0])
		      n += strlen(tn_env_disp) + 7 + 2;
		} else if (!strcmp(varname,"USER") && uidbuf[0])
		  n += strlen(uidbuf) + 4 + 2;
		else if (!strcmp(varname,"JOB") && tn_env_job[0])
		  n += strlen(tn_env_job) + 3 + 2;
		else if (!strcmp(varname,"ACCT") && tn_env_acct[0])
		  n += strlen(tn_env_acct) + 4 + 2;
		else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0])
		  n += strlen(tn_env_prnt) + 7 + 2;
		else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0])
		  n += strlen(tn_env_sys) + 10 + 2;
		else if (!strcmp(varname,"DISPLAY") && tn_env_disp[0])
		  n += strlen(tn_env_disp) + 7 + 2;
		break;
	      case 2:			/* USERVAR in progress */
		break;			/* We don't support this yet */
	    }
	    varname[0] = '\0';
	    j = 0;
	    type = (sb[i] == 3 ? 2 :	/* USERVAR */
		    sb[i] == 0 ? 1 :	/* VAR */
		    0
		   );
	    break;
	  case 1:			/* VALUE */
	    /* Protocol Error */
	    debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
	    if (debses) 
	      tn_debug("TELNET Subnegotiation error - VALUE in SEND");
	    return(0);	/* Was -1 but that would be taken as */
	                /* an I/O error, so absorb it and go on. */
	  case 2:	/* ESC */
	    /* Not sure what this for.  Quote next character? */
	    break;
	  default:
	    varname[j++] = sb[i];
	}
    }
    reply = (CHAR *) malloc(n + 7);	/* Leave room for IAC stuff */
    if (!reply) {
	debug(F100, "TELNET Subnegotiation error - malloc failed", "",0);
	if (debses) 
	  tn_debug("TELNET Subnegotiation error - malloc failed");

	/* Send a return packet with no variables so that the host */
	/* may continue with additional negotiations               */
	sb[0] = (CHAR) IAC;		/* I Am a Command */
	sb[1] = (CHAR) SB;		/* Subnegotiation */
	sb[2] = TELOPT_NEWENVIRON;	/* New Environment */
	sb[3] = (CHAR) 0;		/* Is... */
	sb[4] = (CHAR) IAC;		/* End of Subnegotiation */
	sb[5] = (CHAR) SE;		/* marked by IAC SE */
	if (ttol((CHAR *)sb,6) < 0)	/* Send it. */
	  return(-1);
	sb[4] = '\0';			/* For debugging */
	if (deblog || debses) {
	    sprintf(tn_msg,"TELNET SENT SB %s IS  IAC SE",
		     tnopts[TELOPT_NEWENVIRON]);
	    debug(F100,tn_msg,"",0);
	    if (debses) tn_debug(tn_msg);
	}
	return(0);     
    }

    /* Now construct the real reply */
    reply[0] = (CHAR) IAC;			/* I Am a Command */
    reply[1] = (CHAR) SB;			/* Subnegotiation */
    reply[2] = TELOPT_NEWENVIRON;		/* New Environment */
    reply[3] = (CHAR) 0;			/* Is... */
    n = 4; 
/* Pairs of <type> [VAR=0, VALUE=1, ESC=2, USERVAR=3] <value> "unterminated" */
    /* follow here until done */
    for (i = 0, j = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
	switch (sb[i]) {
	  case 0:			/* VAR */
	  case 3:			/* USERVAR */
	  case IAC:			/* End of the list */
	    switch (type) {
	      case 0:			/* Nothing in progress */
		/* If we get IAC only, then that means send all */
		/* VAR and USERVAR.  But since we don't support */
		/* USERVAR yet, we can just pass through        */
		if (!(j == 0 && sb[i] == IAC))
		  break;
	      case 1:			/* VAR in progress */
		varname[j] = '\0';
		if (!varname[0]) {
		    /* Send All */
		    if ( uidbuf[0] ) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"USER");
			reply[n+5] = 1; 	/* VALUE */
			strcpy(&reply[n+6],uidbuf);
			n += strlen(uidbuf) + 4 + 2;
		    }
		    if (tn_env_job[0]) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"JOB");
			reply[n+4] = 1;	/* VALUE */
			strcpy(&reply[n+5],tn_env_job);
			n += strlen(tn_env_job) + 3 + 2;
		    }
		    if (tn_env_acct[0]) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"ACCT");
			reply[n+5] = 1;	/* VALUE */
			strcpy(&reply[n+6],tn_env_acct);
			n += strlen(tn_env_acct) + 4 + 2;
		    }
		    if (tn_env_prnt[0]) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"PRINTER");
			reply[n+8] = 1;	/* VALUE */
			strcpy(&reply[n+9],tn_env_prnt);
			n += strlen(tn_env_prnt) + 7 + 2;
		    }
		    if (tn_env_sys[0]) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"SYSTEMTYPE");
			reply[n+11] = 1; /* VALUE */
			strcpy(&reply[n+12],tn_env_sys);
			n += strlen(tn_env_sys) + 10 + 2;
		    }
		    if (tn_env_disp[0]) {
			reply[n] = 0;	/* VAR */
			strcpy(&reply[n+1],"DISPLAY");
			reply[n+8] = 1;	/* VALUE */
			strcpy(&reply[n+9],tn_env_disp);
			n += strlen(tn_env_disp) + 7 + 2;
		    }
		} else if (!strcmp(varname,"USER") && uidbuf[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"USER");
		    reply[n+5] = 1; 	/* VALUE */
		    strcpy(&reply[n+6],uidbuf);
                    n += strlen(uidbuf) + 4 + 2;
		} else if (!strcmp(varname,"JOB") && tn_env_job[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"JOB");
		    reply[n+4] = 1; 	/* VALUE */
		    strcpy(&reply[n+5],tn_env_job);
		    n += strlen(tn_env_job) + 3 + 2;
		} else if (!strcmp(varname,"ACCT") && tn_env_acct[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"ACCT");
		    reply[n+5] = 1; 	/* VALUE */
		    strcpy(&reply[n+6],tn_env_acct);
		    n += strlen(tn_env_acct) + 4 + 2;
		} else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"PRINTER");
		    reply[n+8] = 1; 	/* VALUE */
		    strcpy(&reply[n+9],tn_env_prnt);
		    n += strlen(tn_env_prnt) + 7 + 2;
		} else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"SYSTEMTYPE");
		    reply[n+11] = 1; 	/* VALUE */
		    strcpy(&reply[n+12],tn_env_sys);
		    n += strlen(tn_env_sys) + 10 + 2;
		} else if (!strcmp(varname,"DISPLAY") && tn_env_disp[0]) {
		    reply[n] = 0;	/* VAR */
		    strcpy(&reply[n+1],"DISPLAY");
		    reply[n+8] = 1; 	/* VALUE */
		    strcpy(&reply[n+9],tn_env_disp);
		    n += strlen(tn_env_disp) + 7 + 2;
		}
		break;
	    case 2:	/* USERVAR in progress */
		/* we don't support this yet */
		break;
	    }
	    varname[0] = '\0';
	    j = 0;
	    type = (sb[i] == 3 ? 2 :	/* USERVAR */
		    sb[i] == 0 ? 1 :	/* VAR */
		    0
		   );
	    break;
	  case 1: /* VALUE */
	    /* Protocol Error */
	    debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
	    if (debses) 
	      tn_debug("TELNET Subnegotiation error - VALUE in SEND");
	    return(0);	/* Was -1 but that would be taken as */
	                /* an I/O error, so absorb it and go on. */
	  case 2:	/* ESC */
	    /* Not sure what this for.  Quote next character? */
	    break;
	  default:
	    varname[j++] = sb[i];
	}
    }
    reply[n++] = (CHAR) IAC;		/* End of Subnegotiation */
    reply[n++] = (CHAR) SE;		/* marked by IAC SE */
    if (ttol((CHAR *)reply,n) < 0) {	/* Send it. */
     	free(reply);
	return(-1);
    }
    reply[n-2] = '\0';			/* For debugging */
    if (deblog || debses) {
	int i;
	sprintf(tn_msg,"TELNET SENT SB %s %s ",
		 tnopts[TELOPT_NEWENVIRON],
		 reply[3] == 1 ? "SEND" : reply[3] == 0 ? "IS" : "INFO");
	for (i = 4; i < n-2; i++) {
	    sprintf(hexbuf,"%c",reply[i]);
	    strcat(tn_msg,hexbuf);
	}
	strcat(tn_msg," IAC SE");
	debug(F100,tn_msg,"",0);
	if (debses) tn_debug(tn_msg);
    }
    free(reply);
    return(1);
#endif /* TNCODE */
}
#endif /* CK_ENVIRONMENT */

/* Telnet send terminal type */
/* Returns -1 on error, 0 if nothing happens, 1 if type sent successfully */

int
tn_sttyp() {				/* Send telnet terminal type. */
#ifndef TNCODE
    debug(F100,"tn_sttyp no TNCODE","",0);
    return(0);
#else
    char *ttn;				/* Name of terminal type. */
    int i;				/* Worker. */
    int tntermflg = 0;

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    ttn = NULL;

#ifdef OS2
    if (ttnum == -1) {
        ttnum = tt_type;
    } else if (ttnumend) {
        ttnumend = 0;
    } else {
        if (--tt_type < 0)
	  tt_type = max_tt;
        if (ttnum == tt_type)
	  ttnumend = 1;
    }
    if (tt_type >= 0 && tt_type <= max_tt) {
	ttn = tt_info[tt_type].x_name;
	settermtype(tt_type,0);
    } else
      ttn = NULL;
#endif /* OS2 */

    if (tn_term) {			/* Terminal type override? */
	debug(F110,"tn_sttyp",tn_term,0);
	if (*tn_term) {
	    ttn = tn_term;
	    tntermflg = 1;
	}
    } else debug(F100,"tn_sttyp no term override","",0);

#ifndef datageneral
    if (!ttn) {				/* If no override, */
	ttn = getenv("TERM");		/* get it from the environment. */
    }
#endif /* datageneral */
    if ((ttn == ((char *)0)) || ((int)strlen(ttn) >= TSBUFSIZ))
      ttn = "UNKNOWN";
    sb[0] = (CHAR) IAC;			/* I Am a Command */
    sb[1] = (CHAR) SB;			/* Subnegotiation */
    sb[2] = TELOPT_TTYPE;		/* Terminal Type */
    sb[3] = (CHAR) 0;			/* Is... */
    for (i = 4; *ttn; ttn++,i++) {	/* Copy and uppercase it */
#ifdef VMS
	if (!tntermflg && *ttn == '-' &&
	    (!strcmp(ttn,"-80") || !strcmp(ttn,"-132")))
	  break;
	else
#endif /* VMS */
	sb[i] = (char) ((!tntermflg && islower(*ttn)) ? toupper(*ttn) : *ttn);
    }
    ttn = (char *)sb;			/* Point back to beginning */
    sb[i++] = (CHAR) IAC;		/* End of Subnegotiation */
    sb[i++] = (CHAR) SE;		/* marked by IAC SE */
    if (ttol((CHAR *)sb,i) < 0)		/* Send it. */
      return(-1);
    sb[i-2] = '\0';			/* For debugging */
    if (deblog || debses) {
	sprintf(tn_msg,"TELNET SENT SB %s IS %s IAC SE",
        tnopts[TELOPT_TTYPE],sb+4);
	debug(F100,tn_msg,"",0);
	if (debses) tn_debug(tn_msg);
    }
    return(1);
#endif /* TNCODE */
}

#ifdef CK_NAWS			/*  NAWS = Negotiate About Window Size  */
int
tn_snaws() {			/*  Send terminal width and height, RFC 1073 */
#ifndef TNCODE
    debug(F100,"tn_snaws no TNCODE","",0);
#else
    int i = 0;
#ifdef OS2 
    int x = VscrnGetWidth(VTERM), 
    y = VscrnGetHeight(VTERM) - (tt_status ? 1 : 0);
#else /* OS2 */
    int x = tt_cols, y = tt_rows;
#endif /* OS2 */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!nawsflg) return(0);

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    sb[i++] = (CHAR) IAC;		/* Send a subnegotiation */
    sb[i++] = (CHAR) SB;
    sb[i++] = TELOPT_NAWS;
    sb[i++] = (CHAR) (x >> 8) & 0xff;
    /* IAC in data must be doubled */
    if ((CHAR) sb[i-1] == (CHAR) IAC) sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (x & 0xff);
    if ((CHAR) sb[i-1] == (CHAR) IAC) sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (y >> 8) & 0xff;
    if ((CHAR) sb[i-1] == (CHAR) IAC) sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (y & 0xff);
    if ((CHAR) sb[i-1] == (CHAR) IAC) sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) SE;
    if (ttol((CHAR *)sb,i) < 0)		/* Send it. */
      return(-1);
    if (deblog || debses) {
	sprintf(tn_msg,"TELNET SENT SB NAWS %d %d IAC SE",x,y);
	debug(F100,tn_msg,"",0);
	if (debses) tn_debug(tn_msg);
    }
#endif /* TNCODE */
    return (0);
}
#endif /* CK_NAWS */

#ifdef SUNX25
/*
  SunLink X.25 support by Marcello Frutig, Catholic University,
  Rio de Janeiro, Brazil, 1990.
*/

/* PAD X.3, X.28 and X.29 support */

static CHAR x29err[MAXPADPARMS+3] = { X29_ERROR, INVALID_PAD_PARM, '\0' };

/* Initialize PAD */

extern CHAR padparms[];

VOID
initpad() {
  padparms[PAD_BREAK_CHARACTER]        = 0;  /* Break character */
  padparms[PAD_ESCAPE]                 = 1;  /* Escape permitted */
  padparms[PAD_ECHO]                   = 1;  /* Kermit PAD does echo */
  padparms[PAD_DATA_FORWARD_CHAR]      = 2;  /* forward character CR */
  padparms[PAD_DATA_FORWARD_TIMEOUT]   = 0;  /* no timeout forward condition */
  padparms[PAD_FLOW_CONTROL_BY_PAD]    = 0;  /* not used */
  padparms[PAD_SUPPRESSION_OF_SIGNALS] = 1;  /* allow PAD service signals */
  padparms[PAD_BREAK_ACTION]           = 21; /* brk action: INT pk + brk ind*/
  padparms[PAD_SUPPRESSION_OF_DATA]    = 0;  /* no supression of user data */
  padparms[PAD_PADDING_AFTER_CR]       = 0;  /* no padding after CR */
  padparms[PAD_LINE_FOLDING]           = 0;  /* no line fold */
  padparms[PAD_LINE_SPEED]             = 0;  /* line speed - don't care */
  padparms[PAD_FLOW_CONTROL_BY_USER]   = 0;  /* flow cont of PAD - not used */
  padparms[PAD_LF_AFTER_CR]            = 0;  /* no LF insertion after CR */
  padparms[PAD_PADDING_AFTER_LF]       = 0;  /* no padding after LF */
  padparms[PAD_EDITING]                = 1;  /* can edit */
  padparms[PAD_CHAR_DELETE_CHAR]       = 8;  /* character delete character */
  padparms[PAD_BUFFER_DELETE_CHAR]     = 21; /* buffer delete character */
  padparms[PAD_BUFFER_DISPLAY_CHAR]    = 18; /* buffer display character */
}

/* Set PAD parameters */

VOID
setpad(s,n) CHAR *s; int n; {
    int i;
    CHAR *ps = s;

    if (n < 1) {
	initpad();
    } else {
	for (i = 0; i < n; i++) {
	    if (*ps > MAXPADPARMS)
	      x29err[i+2] = *ps;
	    else
	      padparms[*ps] = *(ps+1);
	    ps += 2;
	}
    }
}

/* Read PAD parameters */

VOID
readpad(s,n,r) CHAR *s; int n; CHAR *r; {
    int i;
    CHAR *ps = s;
    CHAR *pr = r;

    *pr++ = X29_PARAMETER_INDICATION;
    if (n > 0) {
	for (i = 0; i < n; i++, ps++) {
	    if (*ps > MAXPADPARMS) {
		x29err[i+2] = *ps++;
	    } else {
		*pr++ = *ps;
		*pr++ = padparms[*ps++];
	    }
	}
    } else {
	for (i = 1; i < MAXPADPARMS; i++) {
	    *pr++ = i;
	    *pr++ = padparms[i];
	}
    }
}

int
qbitpkt(s,n) CHAR *s; int n; {
    CHAR *ps = s;
    int x29cmd = *ps;
    CHAR *psa = s+1;
    CHAR x29resp[(MAXPADPARMS*2)+1];

    switch (x29cmd) {

        case X29_SET_PARMS:
            setpad (ps+1,n/2);
            if ((int)strlen((char *)x29err) > 2) {
                ttol(x29err,(int)strlen((char *)x29err));
                x29err[2] = '\0';
            }
            return (-2);
        case X29_READ_PARMS:
            readpad (ps+1,n/2,x29resp);
            setqbit ();
            ttol (x29resp,(n>1)?(n+1):(2*MAXPADPARMS+1));
            if ((int)strlen((char *)x29err) > 2) {
                ttol(x29err,(int)strlen((char *)x29err));
                x29err[2] = '\0';
            }
            resetqbit();
            break;
        case X29_SET_AND_READ_PARMS:
            setpad (ps+1,n/2);
            readpad (ps+1,n/2,x29resp);
            setqbit();
            ttol (x29resp,(n>1)?(n+1):(2*MAXPADPARMS+1));
            if ((int)strlen((char *)x29err) > 2) {
                ttol (x29err,(int)strlen((char *)x29err));
                x29err [2] = '\0';
            }
            resetqbit();
            return (-2);
        case X29_INVITATION_TO_CLEAR:
            (VOID) x25clear();
            return (-1);
        case X29_INDICATION_OF_BREAK:
	    break;
    }
    return (0);
}

/* PAD break action processor */

VOID
breakact() {
    extern char x25obuf[MAXOX25];
    extern int obufl;
    extern int active;
    extern unsigned char tosend;
    static CHAR indbrk[3] = {
	X29_INDICATION_OF_BREAK,
	PAD_SUPPRESSION_OF_DATA,
	1
    };
    CHAR intudat, cause, diag;

    if (x25stat() < 0) return;	/* Ignore if no virtual call established */

    if (padparms[PAD_BREAK_ACTION] != 0) /* Forward condition */
        if (ttol((CHAR *)x25obuf,obufl) < 0) {
            perror ("\r\nCan't send characters");
            active = 0;
        } else {
            bzero (x25obuf,sizeof(x25obuf));
            obufl = 0;
            tosend = 0;
        };

    switch (padparms[PAD_BREAK_ACTION]) {

       case 0 : break;			/* do nothing */
       case 1 : /* send interrupt packet with interrupt user data field = 1 */
	        intudat = 1;
                x25intr (intudat);
                break;
       case 2 : /* send reset packet with cause and diag = 0 */
		cause = diag = 0;
                x25reset (cause,diag);
                break;
       case 5 : /* send interrupt packet with interrupt user data field = 0 */
		intudat = 0;
                x25intr (intudat);
                setqbit ();
	        /* send indication of break without a parameter field */
                ttoc(X29_INDICATION_OF_BREAK);
                resetqbit ();
                break;
       case 8 : active = 0;		/* leave data transfer */
                conol ("\r\n");
                break;
       case 21: /* send interrupt packet with interrupt user data field = 0 */
		intudat = 0;
                x25intr (intudat);
                setpad (indbrk+1,2);	/* set pad to discard input */
                setqbit ();
		/* send indication of break with parameter field */
                ttol (indbrk,sizeof(indbrk));
                resetqbit ();
                break;
     }
}

/* X.25 support functions */

X25_CAUSE_DIAG diag;

/*
  Convert a null-terminated string representing an X.121 address
  to a packed BCD form.
*/
int
pkx121(str,bcd) char *str; CHAR *bcd; {
    int i, j;
    u_char c;

    i = j = 0;
    while (str[i]) {
        if (i >= 15 || str [i] < '0' || str [i] > '9')
	  return (-1);
        c = str [i] - '0';
        if (i & 1)
	  bcd [j++] |= c;
        else
	  bcd [j] = c << 4;
        i++;
    }
    return (i);
}

/* Reads and prints X.25 diagnostic */

int
x25diag () {
    int i;

    bzero ((char *)&diag,sizeof(diag));
    if (ioctl(ttyfd,X25_RD_CAUSE_DIAG,&diag)) {
        perror ("Reading X.25 diagnostic");
        return(-1);
    }
    if (diag.datalen > 0) {
        printf ("X.25 Diagnostic :");
        for (i = 0; i < (int)diag.datalen; i++)
	  printf(" %02x",diag.data[i]);
        printf ("\r\n");
    }
    return(0);
}

/* X.25 Out-of-Band Signal Handler */

SIGTYP
x25oobh(foo) int foo; {
    int oobtype;
    u_char oobdata;
    int t;

    (VOID) signal(SIGURG,x25oobh);
    do {
        if (ioctl(ttyfd,X25_OOB_TYPE,&oobtype)) {
            perror ("Getting signal type");
            return;
        }
        switch (oobtype) {
	  case INT_DATA:
	    if (recv(ttyfd,(char *)&oobdata,1,MSG_OOB) < 0) {
		perror ("Receiving X.25 interrupt data");
		return;
	    }
	    t = oobdata;
	    printf ("\r\nInterrupt received, data = %d\r\n", t);
	    break;
	  case VC_RESET:
	    printf ("\r\nVirtual circuit reset\r\n");
	    x25diag ();
	    break;
	  case N_RESETS:
	    printf ("\r\nReset timeout\r\n");
	    break;
	  case N_CLEARS:
	    printf ("\r\nClear timeout\r\n");
	    break;
	  case MSG_TOO_LONG:
	    printf ("\r\nMessage discarded, too long\r\n");
	    break;
	  default:
	    if (oobtype) printf("\r\nUnknown oob type %d\r\n",oobtype);
	    break;
	}
    } while (oobtype);
}

/* Send a X.25 interrupt packet */

int
#ifdef CK_ANSIC
x25intr(char intr)
#else
x25intr(intr) char intr;
#endif /* CK_ANSIC */
/* x25intr */ {
    if (send(ttyfd,&intr,1,MSG_OOB) < 0) return(-1);
    debug(F100,"X.25 intr","",0);
    return(0);
}

/* Reset X.25 virtual circuit */
int
#ifdef CK_ANSIC
x25reset(char cause, char diagn)
#else
x25reset(cause, diagn) char cause; char diagn;
#endif /* CK_ANSIC */
/* x25reset */ {
    bzero ((char *)&diag,sizeof(diag));
    diag.flags   = 0;
    diag.datalen = 2;
    diag.data[0] = cause;
    diag.data[1] = diagn;
    if (ioctl(ttyfd,X25_WR_CAUSE_DIAG,&diag) < 0)
      return(-1);
    debug(F100,"X.25 reset","",0);
    return(0);
}

/* Clear X.25 virtual circuit */
int
x25clear() {
    int i;
    debug(F100,"X.25 clear","",0);
    bzero ((char *)&diag,sizeof(diag));
    diag.flags = (1 << DIAG_TYPE);
    diag.datalen = 2;
    diag.data[0] = 0;
    diag.data[1] = 0;
    ioctl (ttyfd,X25_WR_CAUSE_DIAG,&diag); /* Send Clear Request */
    return(ttclos(0));			/* Close socket */
}

/* X.25 status */
int
x25stat() {
    if (ttyfd == -1) return (-1);
    return(0);
}

/* Set Q_BIT on */
VOID
setqbit() {
    static int qbiton = 1 << Q_BIT;
    ioctl (ttyfd,X25_SEND_TYPE,&qbiton);
}

/* Set Q_BIT off */
VOID
resetqbit() {
    static int qbitoff = 0;
    ioctl (ttyfd,X25_SEND_TYPE,&qbitoff);
}

/* Read n characters from X.25 circuit into buf */

int
x25xin(n,buf) int n; CHAR *buf; {
    register int x, c;
    int qpkt;

    do {
	x = read(ttyfd,buf,n);
	if (buf[0] & (1 << Q_BIT)) { /* If Q_BIT packet, process it */
	    /* If return -1 : invitation to clear; -2 : PAD changes */
	    if ((c=qbitpkt(buf+1,x-2)) < 0) return(c);
	    qpkt = 1;
	} else qpkt = 0;
    } while (qpkt);
    if (x > 0) buf[x] = '\0';
    if (x < 1) x = -1;
    debug(F101,"x25xin x","",x);

    return(x);
}

#ifdef COMMENT /* NO LONGER NEEDED! */
/* X.25 read a line */

int
#ifdef PARSENSE
#ifdef CK_ANSIC
x25inl(CHAR *dest, int max,int timo, CHAR eol, CHAR start)
#else
x25inl(dest,max,timo,eol,start) int max,timo; CHAR *dest, eol, start;
#endif /* CK_ANSIC */
#else /* not PARSENSE */
#ifdef CK_ANSIC
x25inl(CHAR *dest, int max,int timo, CHAR eol)
#else
x25inl(dest,max,timo,eol) int max,timo; CHAR *dest, eol;
#endif /* __SDTC__ */
#endif /*PARSENSE */
 /* x25inl */ {
    CHAR *pdest;
    int pktype, goteol, rest, n;
    int i, flag = 0;
    extern int ttprty, ttpflg;
    int ttpmsk;

    ttpmsk = (ttprty) ? 0177 : 0377;	/* Set parity stripping mask */

    debug(F101,"x25inl max","",max);
    debug(F101,"x25inl eol","",eol);
    pdest  = dest;
    rest   = max;
    goteol = 0;
    do {
	n = read(ttyfd,pdest,rest);
	n--;
	pktype = *pdest & 0x7f;
	switch (pktype) {
	  case 1 << Q_BIT:
	    if (qbitpkt(pdest+1,--n) < 0) return(-2);
	    break;
	  default:
	    if (flag == 0) { /* if not in packet, search start */
		for (i = 1; (i < n) &&
		     !(flag = ((dest[i] & 0x7f) == start));
		     i++);
		if (flag == 0) { /* not found, discard junk */
		    debug(F101,"x25inl skipping","",n);
		    continue;
		} else {		/* found, discard junk before start */
		    int k;
		    n = n - i + 1;
		    for (k = 1; k <= n; k++, i++) dest[k] = dest[i];
		}
	    }
	    for (i = 0; (i < n) && /* search for eol */
		 !(goteol=(((*pdest = *(pdest+1)&ttpmsk)&0x7f)== eol));
		 i++,pdest++);
	    *pdest = '\0';
	    rest -= n;
	}
    } while ((rest > 0) && (!goteol));

    if (goteol) {
	n = max - rest;
	debug (F111,"x25inl X.25 got",(char *) dest,n);
	if (timo) ttimoff();
	if (ttpflg++ == 0 && ttprty == 0) {
	    if ((ttprty = parchk(dest,start,n)) > 0) {
		int j;
		debug(F101,"x25inl senses parity","",ttprty);
		debug(F110,"x25inl packet before",(char *)dest,0);
		ttpmsk = 0x7f;
		for (j = 0; j < n; j++)
		  dest[j] &= 0x7f; /* Strip parity from packet */
		debug(F110,"x25inl packet after ",dest,0);
	    } else {
		debug(F101,"parchk","",ttprty);
		if (ttprty < 0) { ttprty = 0; n = -1; }
	    }
	}
	ttimoff();
	return(n);
    }
    ttimoff();
    return(-1);
}
#endif /* COMMENT */
#endif /* SUNX25 */

#endif /* NETCONN */
