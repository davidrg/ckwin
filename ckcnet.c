char *cknetv = "Network support, 5A(039) 30 Sep 94";

/*  C K C N E T  --  Network support  */

/*
  NOTE TO CONTRIBUTORS: This file, and all the other shared (ckc and cku)
  C-Kermit source files, must be compatible with C preprocessors that support
  only #ifdef, #else, #endif, #define, and #undef.  Please do not use #if,
  logical operators, or other preprocessor features in any of the portable
  C-Kermit modules.  You can, of course, use these constructions in
  system-specific modules when you know they are supported.
*/

/*
  Authors:

  Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
    Columbia University Academic Information Systems, New York City.
  netopen() routine for TCP/IP originally by Ken Yap, Rochester University
    (ken@cs.rochester.edu) (no longer at that address).
  Missing pieces for Excelan sockets library from William Bader, Moravian
    College <bader@moravian.edu>.
  TELNET protocol by Frank da Cruz.
  TGV MultiNet code by Frank da Cruz.
  MultiNet code adapted to WIN/TCP by Ray Hunter of TWG.
  MultiNet code adapted to DEC TCP/IP by Lee Tibbert of DEC and Frank da Cruz.
  TCP/IP support adapted to IBM TCP/IP 1.2.1 for OS/2 by Kai Uwe Rommel.
  CMU-OpenVMS/IP modifications by Mike O'Malley, Digital (DEC).
  SunLink X.25 support by Marcello Frutig, Catholic University,
    Rio de Janeiro, Brazil (frutig@rnp.impa.br) with fixes from:
    Stefaan Eeckels, Eurokom, Luxembourg;
    David Lane, Status Computer.
  Other contributions as indicated below.

  Copyright (C) 1985, 1994, Trustees of Columbia University in the City of New
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
#define listen  Rlisten
#endif /* CK_SOCKS */

#ifdef DEC_TCPIP
#include <time.h>
#endif /* DEC_TCPIP */

#ifdef CMU_TCPIP
#include <time.h>
#endif /* CMU_TCPIP */

#ifdef WINTCP

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
/*
  The WIN/TCP code path is the same as that for MultiNet.
  Only the routine names have changed ...
*/
#define socket_errno 	errno
#define socket_read 	netread
#define socket_ioctl	ioctl
#define socket_write 	netwrite
#define socket_perror   win$perror
#define socket_close 	netclose

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
#include <:usr:include:sys:socket.h>
#include <:usr:include:netinet:in.h>
#include <:usr:include:netdb.h>
#endif /* datageneral */

extern SIGTYP (*saval)();		/* For saving alarm handler */

_PROTOTYP( VOID bgchk, (void) );
_PROTOTYP( static VOID tn_debug, (char *) );

extern int				/* External variables */
  duplex, debses, seslog, sessft, ttyfd, quiet, msgflg, deblog, what;

#ifdef CK_NAWS				/* Negotiate About Window Size */
extern int tt_rows, tt_cols;
_PROTOTYP( int ttgwsiz, (void) );
_PROTOTYP( int tn_snaws, (void) );
#endif /* CK_NAWS */

#ifdef OS2				/* For terminal type name string */
#include "ckuusr.h"			/* For  */
extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
#endif /* OS2 */

#ifndef OS2				/* For timeout longjumps */
#ifndef WINTCP
#ifdef CK_POSIX_SIG
static sigjmp_buf njbuf;
#else
static jmp_buf njbuf;
#endif /* CK_POSIX_SIG */
#endif /* OS2 */
#endif /* WINTCP */

#define NAMECPYL 100			/* Local copy of hostname */
static char namecopy[NAMECPYL];

char ipaddr[20] = { '\0' };		/* Global copy of IP address */

#endif /* NETCONN */

int ttnet = NET_NONE;			/* Network type */
int ttnproto = NP_NONE;			/* Network virtual terminal protocol */
int tn_init = 0;			/* Telnet protocol initialized flag */
int tn_duplex = 1;			/* Initial echo status */
char *tn_term = NULL;			/* Terminal type override */
int tn_nlm = TNL_CRLF;			/* Telnet CR -> CR LF mode */

#ifdef TNCODE
static char tn_msg[128];		/* For debugging */
static char hexbuf[6];
#endif /* TNCODE */

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

static int wnawsflg = 0;		/* WILL/WONT NAWS has been sent */

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
    if (ttyfd < 0)
      return;
    x = ttgwsiz();			/* Get new window size */
/*
  This should be OK.  It might seem that sending this from
  interrupt level could interfere with another TELNET IAC string
  that was in the process of being sent.  But we always send
  TELNET strings with a single write(), which should prevent mixups.
*/
    if (x > 0 && tt_rows > 0 && tt_cols > 0)
      tn_snaws();
    return;
}
#endif /* UNIX */
#endif /* SIGWINCH */
#endif /* CK_NAWS */
#endif /* NOSIGWINCH */

/*
  TCPIPLIB means use separate socket calls, while on UNIX the
  normal file system calls are used for TCP/IP sockets too,
  i.e. "DEC_TCPIP or MULTINET or WINTCP or OS2" (defined in ckcnet.h).
*/
#ifdef TCPIPLIB

/* For buffered network reads... */
/*
  If the buffering code is written right, it shouldn't matter
  how long this buffer is.
*/
#define TTIBUFL 8192			/* Let's use 8K. */

CHAR 	ttibuf[TTIBUFL+1];
int 	ttibp = 0, ttibn = 0;
/*
  Read bytes from network into internal buffer ttibuf[].
  To be called when input buffer is empty, i.e. when ttibn == 0.

  Other network reading routines, like ttinc, ttinl, ttxin, should check the
  internal buffer first, and call this routine for a refill if necessary.

  Returns -1 on error, 0 if nothing happens.  When data is read successfully,
  returns number of bytes read, and sets global ttibn to that number and
  ttibp (the buffer pointer) to zero.
*/
int
ttbufr() {				/* TT Buffer Read */
    int count;

    if (ttnet != NET_TCPB) {		/* First make sure current net is */
	return(-1);			/* TCP/IP; if not, do nothing. */
    } else {
	if (ttibn > 0)			/* Out internal buffer is not empty, */
	  return(ttibn);		/* so keep using it. */
#ifdef WINTCP
	count = 512;			/* This works for WIN/TCP */
#else
#ifdef DEC_TCPIP
	count = 512;			/* UCX */
#else					/* Multinet, etc. */
	count = nettchk();		/* Check network input buffer, */
	if (ttibn > 0) return(ttibn);	/* which can put a char there! */
	if (count < 0)			/* Read error */
	  return(-1);
	else if (count > TTIBUFL)	/* Too many to read */
	  count = TTIBUFL;
	else if (count == 0)		/* None, so force blocking read */
	  count = 1;
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
#else
/* This is for blocking reads */
	if ((count = socket_read(ttyfd,ttibuf,count)) < 1) {
	    debug(F101,"ttbufr socket_read","",count);
	    debug(F101,"ttbufr socket_errno","",socket_errno);
	    netclos();			/* *** *** */
	    return(-1);
	}
#endif /* COMMENT */
	ttibp = 0;			/* Reset buffer pointer. */
	ttibn = count;
#ifdef DEBUG
	debug(F101,"ttbufr count 2","",count); /* Got some bytes. */
	if (count > 0) ttibuf[count] = '\0';
	debug(F111,"ttbufr ttibuf",ttibuf,ttibp);
#endif /* DEBUG */
	return(ttibn);			/* Return buffer count. */
    }
}
#endif /* TCPIPLIB */

/*
  C-Kermit network open/close functions for BSD-sockets.
  Much of this code shared by SunLink X.25, which also uses the socket library.
*/

/*  N E T O P E N  --  Open a network connection.  */

/*  Returns 0 on success, -1 on failure.  */

#define	TELNET_PORT	23	   /* Should do lookup, but it won't change */

/* This symbol is not known to, e.g., Ultrix 2.0 */
#ifndef TELOPT_TTYPE
#define TELOPT_TTYPE 24
#endif /* TELOPT_TTYPE */

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
    	return( &servrec );
    }
    return( (struct servent *) NULL );
}

struct hostent *
gethostbyname(hostname) char *hostname; {
    return( (struct hostent *) NULL );
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
#endif /* EXCELAN */
#endif /* TCPSOCKET */

#ifdef SUNX25
#ifndef X25_WR_FACILITY
/* For Solaris 2.3 / SunLink 8.x - see comments in ckcnet.h */
void
bzero(s,n) char *s; int n; {
    memset(s,0,n);
}
#endif /* X25_WR_FACILITY */
#endif /* SUNX25 */

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
#ifdef SO_OOBINLINE
    int on = 1;
#endif /* SO_OOBINLINE */
    struct servent *service, servrec;
    struct hostent *host;
    struct sockaddr_in saddr;
#ifdef EXCELAN
    struct sockaddr_in send_socket;
#endif /* EXCELAN */
#endif /* TCPSOCKET */

#ifdef SUNX25				/* Code for SunLink X.25 support */
#define X29PID 1			/* X.29 Protocol ID */
_PROTOTYP( SIGTYP x25oobh, (int) );
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
    extern char udata[MAXCUDATA];
#endif /* SUNX25 */

    debug(F101,"netopen nett","",nett);
    *ipaddr = '\0';			/* Initialize IP address string */

#ifdef SUNX25
    if (nett == NET_SX25) {		/* If network type is X.25 */
        netclos();			/* Close any previous net connection */
        ttnproto = NP_NONE;		/* No protocol selected yet */

        /* Set up host structure */
        bzero ((char *)&x25host,sizeof(x25host));
        if ((x25host.hostlen = pkx121 (name,x25host.host)) < 0) {
            fprintf (stderr,"Invalid X.121 host address %s\n",name);
            errno = 0;
            return (-1);
        }
        x25host.datalen = X29PIDLEN;
        x25host.data[0] = X29PID;

	/* Set call user data if specified */
        if (cudata) {
            strncpy(x25host.data+X29PIDLEN,udata,(int)strlen(udata));
            x25host.datalen += (int)strlen(udata);
        }

        /* Open SunLink X.25 socket */
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
            debug(F101,"netopen connect errno","",errno);
            i = errno;
	    if (errno) {
                perror("netopen x25 connect");
                x25diag();
            }
            (VOID) close (ttyfd);
            ttyfd = -1;
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
/*
  Add support for other networks here.
*/
      if (nett != NET_TCPB) return(-1);	/* BSD socket support */

#ifdef TCPSOCKET
    netclos();				/* Close any previous connection. */
    strncpy(namecopy, name, NAMECPYL);	/* Copy the hostname. */
    ttnproto = NP_NONE;			/* No protocol selected yet. */
    debug(F110,"netopen namecopy",namecopy,0);

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
#ifdef MULTINET
	debug(F101,"netopen can't get service","",socket_errno);
#else
	debug(F101,"netopen can't get service","",errno);
#endif /* MULTINET */
	errno = 0;			/* rather than mislead */
	return(-1);
    }

    /* Set up socket structure and get host address */

    bzero((char *)&saddr, sizeof(saddr));
    debug(F100,"netopen bzero ok","",0);
    if ((host = gethostbyname(namecopy)) != NULL) {
	debug(F100,"netopen gethostbyname != NULL","",0);
	saddr.sin_family = host->h_addrtype;
	bcopy(host->h_addr, (caddr_t)&saddr.sin_addr, host->h_length);
	debug(F111,"BCOPY","host->h_addr",host->h_addr);
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
#ifdef MULTINET
	    debug(F101,"netopen can't get address","",socket_errno);
#else
	    debug(F101,"netopen can't get address","",errno);
#endif /* MULTINET */
	    errno = 0;		/* Rather than mislead */
	    return(-1);
	}
    }

    /* Get a file descriptor for the connection. */

    saddr.sin_port = service->s_port;
    sprintf(ipaddr,"%s", (char *)inet_ntoa(saddr.sin_addr));
    if (!quiet && *ipaddr) printf(" Trying %s...\n", ipaddr);

#ifdef EXCELAN
    send_socket.sin_family = AF_INET;
    send_socket.sin_addr.s_addr = 0;
    send_socket.sin_port = 0;
    if ((ttyfd = socket(SOCK_STREAM, (struct sockproto *)0,
			&send_socket, SO_REUSEADDR)) < 0)
#else
      if ((ttyfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif /* EXCELAN */
	{
#ifdef EXCELAN
	    experror("TCP socket error");
#else
#ifdef MULTINET
	    socket_perror("TCP socket error");
	    debug(F101,"netopen socket error","",socket_errno);
#else
	    perror("TCP socket error");
	    debug(F101,"netopen socket error","",errno);
#endif /* MULTINET */
#endif /* EXCELAN */
	    return (-1);
	}
    errno = 0;

    /* Now connect to the socket on the other end. */

#ifdef EXCELAN
    if (connect(ttyfd, &saddr) < 0)
#else
      if (connect(ttyfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
#endif /* EXCELAN */
	{
	    i = errno;			/* save error code */
	    close(ttyfd);
	    ttyfd = -1;
	    errno = i;			/* and report this error */
#ifdef EXCELAN
	    if (errno) experror("netopen connect");
#else
#ifdef MULTINET
	    debug(F101,"netopen connect error","",socket_errno);
	    if (errno) socket_perror("netopen connect");
#else
	    debug(F101,"netopen connect errno","",errno);
#ifdef	WINTCP
	    perror("netopen connect");
#endif	/* WINTCP */
#ifdef DEC_TCPIP
	    perror("netopen connect");
#endif /* DEC_TCPIP */
#ifdef CMU_TCPIP
	    perror("netopen connect");
#endif /* CMU_TCPIP */
#endif /* MULTINET */
#endif /* EXCELAN */
	    return(-1);
	}
#ifdef SO_OOBINLINE
/*
  The symbol SO_OOBINLINE is not known to Ultrix 2.0.
  It means "leave out of band data inline".  The normal value is 0x0100,
  but don't try this on systems where the symbol is undefined.
*/
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
#ifdef SOLARIS
/*
  Maybe this applies to all SVR4 versions, but the other (else) way has been
  compiling and working fine on all the others, so best not to change it.
*/
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef OS2
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
#ifdef VMS /* or, at least, VMS with gcc */
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);
#else
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif /* VMS */
#endif /* OS2 */
#endif /* SOLARIS */
#endif /* POSIX */
#endif /* BSD43 */
#endif /* OSF1 */
#endif /* datageneral */
#endif /* SO_OOBINLINE */

#ifdef CK_LINGER
/*
  The following, from William Bader, turns off the socket linger parameter,
  which makes a close() block until all data is sent.  "I don't think that
  disabling linger can ever cause kermit to lose data, but you telnet to a
  flaky server (or to our modem server when the modem is in use), disabling
  linger prevents kermit from hanging on the close if you try to exit."
  Define CK_LINGER if you want to use this code.
*/
#ifdef SOL_SOCKET
#ifdef SO_LINGER
    {
      struct linger linger_opt;
      x = sizeof(linger_opt);
      if (getsockopt(ttyfd, SOL_SOCKET, SO_LINGER, (char *)&linger_opt, &x)) {
	  perror("could not get SO_LINGER");
      } else if (x != sizeof(linger_opt)) {
	  debug(F101,"netopen error: SO_LINGER len","",x);
	  debug(F101,"netopen SO_LINGER expected len","",sizeof(linger_opt));
      } else if (linger_opt.l_onoff) {
	  linger_opt.l_onoff = 0;
	  if (setsockopt(ttyfd,
			 SOL_SOCKET,
			 SO_LINGER,
			 (char *)&linger_opt,
			 sizeof(linger_opt))) {
	      perror("error clearing SO_LINGER");
	  } else {
	      debug(F101,"netopen new SO_LINGER","",linger_opt.l_onoff);
	  }
      } else {
	  debug(F101,"netopen SO_LINGER unchanged","",linger_opt.l_onoff);
      }
  }
#else
    debug(F100,"SO_LINGER not defined","",0);
#endif /* SO_LINGER */
#endif /* SOL_SOCKET */
#endif /* CK_LINGER */

/*
  The following, from William Bader, allows changing of socket buffer sizes,
  in case that might affect performance.
*/
#ifdef CK_SOCKBUF
#ifdef SOL_SOCKET
#ifdef SO_SNDBUF
    x = sizeof(i);
    if (getsockopt(ttyfd, SOL_SOCKET, SO_SNDBUF, (char *)&i, &x)) {
	perror("could not get SO_SNDBUF");
    } else if (x != sizeof(i)) {
	debug(F101,"netopen error: SO_SNDBUF len","",x);
	debug(F101,"netopen SO_SNDBUF expected len","",sizeof(i));
    } else if (i < 32768) {
	x = 32768;
	if (setsockopt(ttyfd, SOL_SOCKET, SO_SNDBUF, (char *)&x, sizeof(x))) {
	    perror("error setting SO_SNDBUF");
	} else {
	    debug(F101,"netopen old SO_SNDBUF","",i);
	    debug(F101,"netopen new SO_SNDBUF","",x);
	}
    } else {
	debug(F101,"netopen SO_SNDBUF unchanged","",i);
    }
#else
    debug(F100,"SO_SNDBUF not defined","",0);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    x = sizeof(i);
    if (getsockopt(ttyfd, SOL_SOCKET, SO_RCVBUF, (char *)&i, &x)) {
	perror("could not get SO_RCVBUF");
    } else if (x != sizeof(i)) {
	debug(F101,"netopen error: SO_RCVBUF len","",x);
	debug(F101,"netopen SO_RCVBUF expected len","",sizeof(i));
    } else if (i < 32768) {
	x = 32768;
	if (setsockopt(ttyfd, SOL_SOCKET, SO_RCVBUF, (char *)&x, sizeof(x))) {
	    perror("error setting SO_RCVBUF");
	} else {
	    debug(F101,"netopen old SO_RCVBUF","",i);
	    debug(F101,"netopen new SO_RCVBUF","",x);
	}
    } else {
	debug(F101,"netopen SO_RCVBUF unchanged","",i);
    }
#else
    debug(F100,"SO_RCVBUF not defined","",0);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* CK_SOCKBUF */

    /* See if the service is TELNET. */
    if ((x = ntohs((unsigned short)service->s_port)) == TELNET_PORT)
      ttnproto = NP_TELNET;		/* Yes, set global flag. */
    debug(F101,"netopen service","",x);
    ttnet = nett;			/* TCP/IP (sockets) network */
    tn_ini();				/* Start TELNET negotiations. */
    if (*lcl < 0) *lcl = 1;		/* Local mode. */
#endif /* TCPSOCKET */
    return(0);				/* Done. */
}

/*  N E T C L O S  --  Close current network connection.  */

int
netclos() {
    int x = 0;
    debug(F101,"netclos","",ttyfd);
    if (ttyfd < 0)			/* Was open? */
      return(0);			/* Wasn't. */
    if (ttyfd > -1)			/* Was. */
#ifdef TCPIPLIB
      x = socket_close(ttyfd);		/* Close it. */
#else
      x = close(ttyfd);
#endif /* TCPIPLIB */
    ttyfd = -1;				/* Mark it as closed. */
    debug(F100,"netclose setting tn_init = 0","",0);
    tn_init = 0;			/* Remember about telnet protocol... */
    *ipaddr = '\0';			/* Zero the IP address string */
    return(x);
}

/*  N E T T C H K  --  Check if network up, and how many bytes can be read */
/*
  Returns number of bytes waiting, or -1 if connection has been dropped.
*/
int					/* Check how many bytes are ready */
nettchk() {				/* for reading from network */
#ifdef TCPIPLIB
    unsigned long count = 0;
    int x;
    long y;
    char c;

    debug(F101,"nettchk entry ttibn","",ttibn);
    debug(F101,"nettchk entry ttibp","",ttibp);
    socket_errno = 0;
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

    if (socket_ioctl(ttyfd,FIONREAD,&count) < 0) {
	debug(F101,"nettchk socket_ioctl error","",socket_errno);
	if (ttibn < 1) {
	    netclos();			/* *** *** */
	    return(-1);
	} else return(ttibn);
    }
    debug(F101,"nettchk count","",count);

#ifndef DEC_TCPIP
#ifndef CMU_TCPIP
/*
  Let's see if we can skip this for UCX, since it seems to cause trouble.
  Ditto for CMU-OpenVMS/IP.
*/
    if (count == 0) {
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
	debug(F101,"nettchk before FIONBIO","",x);
	x = socket_ioctl(ttyfd,FIONBIO,&y);
	debug(F101,"nettchk FIONBIO","",x);
	x = socket_read(ttyfd,&c,1);	/* Returns -1 if no data */
	debug(F101,"nettchk socket_read","",x);
	y = 0;				/* Turn them back off */
	socket_ioctl(ttyfd,FIONBIO,&y);
	if (x == 0) {
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
    }
#endif /* !CMU_TCPIP */
#endif /* !DEC_TCPIP */
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
#ifndef WINTCP
VOID
nettout(i) int i; {			/* Catch the alarm interrupts */
    debug(F100,"nettout caught timeout","",0);
    ttimoff();
#ifdef CK_POSIX_SIG
    siglongjmp(njbuf, -1);
#else
    longjmp(njbuf, -1);
#endif /* CK_POSIX_SIG */
}
#endif /* !WINTCP */
#endif /* !OS2 */

/*  N E T I N C --  Input character from network */

int			
netinc(timo) int timo; {
#ifdef TCPIPLIB
    int x; unsigned char c;		/* The locals. */

    if (ttibn > 0) {			/* Something in internal buffer? */
	debug(F100,"netinc char in buf","",0); /* Yes. */
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
/*
  select() is used in preference to alarm()/signal(), but different systems
  use different forms of select()...
*/
#ifdef WINTCP				/* VMS with Wollongong WIN/TCP */
#define BSDSELECT
#else
#ifdef CMU_TCPIP			/* LIBCMU can do select */
#define BSDSELECT
#else
#ifdef DEC_TCPIP
#define BSDSELECT
#else
#ifdef OS2				/* OS/2 with TCP/IP */
#define IBMSELECT
#endif /* OS2 */
#endif /* DEC_TCPIP */
#endif /* CMU_TCPIP */
#endif /* WINTCP */

/*
  Others (TGV, TCPware, ...) use alarm()/signal().  The BSDSELECT case does not
  compile at all; the IBMSELECT case compiles and links but crashes at runtime.
  NOTE: If any of these can be converted to select(), they should be for two
  reasons: (1) It's a lot faster; (2) certain sockets libraries do not like
  their socket_read() calls to be interrupted; subsequent socket_read()'s tend
  to fail with EBUSY.  This happened in the UCX case before it was converted
  to use select().
*/

#ifdef BSDSELECT
            fd_set rfds;
            struct timeval tv;
            FD_ZERO(&rfds);
            FD_SET(ttyfd, &rfds);
            tv.tv_sec  = tv.tv_usec = 0L;
            if (timo < 0)
              tv.tv_usec = (long) -timo * 10000L;
            else
              tv.tv_sec = timo;
	    debug(F101,"netinc BSDSELECT","",timo);
            if ( select(FD_SETSIZE, &rfds, NULL, NULL, &tv) &&
                 FD_ISSET(ttyfd, &rfds) )
#else /* !BSDSELECT */
#ifdef IBMSELECT
/* Was used by OS/2, currently not used, but might come in handy some day... */
/* ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set */
/* and timeval stuff since this is the only place where it is used. */
          int socket = ttyfd;
	    debug(F101,"netinc IBMSELECT","",timo);
            if ( select(&socket, 1, 0, 0,
                        timo < 0 ? -timo * 10 : timo * 1000L) == 1)
#else /* !IBMSELECT */
/*
  If we can't use select(), then we use the regular alarm()/signal()
  timeout mechanism.
*/
	    debug(F101,"netinc alarm","",timo);
	    saval = signal(SIGALRM,nettout); /* Enable timer interrupt */
	    alarm(timo);		/* for requested interval. */
	    if (
#ifdef CK_POSIX_SIG
		sigsetjmp(njbuf,1)
#else
		setjmp(njbuf)
#endif /* CK_POSIX_SIG */
		) {			/* Timer went off? */
		x = -1;			/* Yes, fail. */
	    } else {
#endif /* IBMSELECT */
#endif /* BSDSELECT */
		while (1) {
		    if (ttbufr() < 0)	/* Keep trying to refill it. */
		      break;		/* Till we get an error. */
		    if (ttibn > 0) {	/* Or we get a character. */
			x = 0;
			break;
		    }
		}
#ifndef BSDSELECT
#ifndef IBMSELECT
	    }
	    ttimoff();			/* Timer off. */
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
	    debug(F101,"netinc returning","",c);
	    if (c == 0) {
		debug(F101,"netinc 0 ttibn","",ttibn);
		debug(F101,"netinc 0 ttibp","",ttibp);
	    }
	}
	return((c & 0xff));
    }
#else /* Not MULTINET or WINTCP */
    return(-1);
#endif /* TCPIPLIB */
}

/*  N E T T O L  --  Output a string of bytes to the network  */
/*
  Call with s = pointer to string, n = length.
  Returns number of bytes actually written on success, or
  -1 on i/o error, -2 if called improperly.
*/
int
nettol(s,n) char *s; int n; {
#ifdef TCPIPLIB
    int count;
    debug(F101,"nettol TCPIPLIB ttnet","",ttnet);
    if (ttnet == NET_TCPB) {
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
#ifdef TCPIPLIB
    unsigned char cc;
    cc = c;
    if (ttnet == NET_TCPB) {
	debug(F101,"nettoc cc","",cc);
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
}

/*  N E T F L U I  --  Flush network input buffer  */

int
netflui() {
    int n;
#ifdef TCPIPLIB
    ttibuf[ttibp+1] = '\0';
    debug(F111,"netflui 1",ttibuf,ttibn);
    ttibn = ttibp = 0;			/* Flush internal buffer *FIRST* */
    if ((n = nettchk()) > 0) {		/* Now see what's waiting on the net */
	if (n > TTIBUFL) n = TTIBUFL;	/* and sponge it up */
	debug(F101,"netflui 2","",n);	/* ... */
	n = socket_read(ttyfd,ttibuf,n) ; /* into our buffer */
	if (n >= 0) ttibuf[n] = '\0';
	debug(F111,"netflui 3",ttibuf,n);
	ttibuf[0] = '\0';
    }
#else
/*
  It seems the UNIX ioctl()s don't do the trick, so we have to read the
  stuff ourselves.  This should be pretty much portable, if not elegant.
*/
    if ((n = ttchk()) > 0) {
	debug(F101,"netflui","",n);
	while ((n--) && ttinc(0) > -1) ; /* Don't worry, it's buffered. */
    }
#endif /* TCPIPLIB */
    return(0);
}

#ifdef TNCODE				/* Compile in telnet support code */

/* TCP/IP TELNET protocol negotiation support code */

static int sgaflg = 0;			/* SUPRRESS GO-AHEAD state */
static int dosga  = 0;			/* 1 if I sent DO SGA from tn_ini() */
static int wttflg = 0;			/* ditto for WILL TERMINAL TYPE */

#ifndef TELCMDS
char *telcmds[] = {
    "SE", "NOP", "DMARK", "BRK",  "IP",   "AO", "AYT",  "EC",
    "EL", "GA",  "SB",    "WILL", "WONT", "DO", "DONT", "IAC",
};
int ntelcmds = sizeof(telcmds) / sizeof(char *);
#endif /* TELCMDS */

char *tnopts[] = {
	"BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
	"STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
	"NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
	"NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
	"DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
	"SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
	"TACACS UID","OUTPUT MARKING","TTYLOC","3270 REGIME",
	"X.3 PAD","NAWS","TSPEED","LFLOW","LINEMODE"
};
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
    int n;
    if (ttnet != NET_TCPB) return(0);	/* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */
#ifdef TNCODE
    n = cmd - SE;
    if (n < 0 || n > ntelcmds) return(0);
    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) cmd & 0xff;
    buf[2] = (CHAR) opt & 0xff;
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
    int x;
#ifndef TNCODE
    debug(F100,"tn_ini no TNCODE","",0);
    return(0);
#else /* TELNET protocol support */
    debug(F101,"tn_ini ttnproto","",ttnproto);
    debug(F101,"tn_ini tn_init","",tn_init);

    if (ttnet != NET_TCPB)		/* Make sure connection is TCP/IP */
      return(0);
    if (tn_init)			/* Have we done this already? */
      return(0);			/* Don't do it again. */
    debug(F101,"tn_ini tn_duplex","",tn_duplex);
    duplex = tn_duplex;			/* Assume local echo. */
    sgaflg = 0;				/* Assume Go-Ahead suppressed. */
    wttflg = 0;				/* Did not send WILL TERM TYPE yet. */
    wnawsflg = 0;			/* Did not send WILL/WONT NAWS yet. */
    dosga  = 0;				/* Did not send DO SGA yet. */
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
    wttflg = 1;				/* Remember I said I would. */
#ifdef CK_NAWS
    /* Console terminal screen rows and columns */
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
	wnawsflg = 1;			/* Remember I said I would. */
    }
#endif /* CK_NAWS */

    if (tn_sopt(DO,TELOPT_SGA) < 0)	/* Please suppress go-ahead. */
      return(-1);
    dosga = 1;				/* Remember I said DO SGA */
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
    void cwrite(unsigned char);
    char *p = s;
    _PROTOTYP (void os2bold, (void) );
#endif /* OS2 */

    debug(F111,"tn_debug",s,what);
    if (what != W_CONNECT || debses == 0) /* CONNECT command must be active */
      return;

#ifdef OS2
    os2bold();				/* Toggle boldness */
    while (*p) cwrite((CHAR) *p++);	/* Go boldly ... */
    os2bold();				/* Toggle boldness back */
    debses = 0;
    cwrite((CHAR) '\015');
    cwrite((CHAR) '\012');
    debses = 1;
#else
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

#define TSBUFSIZ 41
char sb[TSBUFSIZ];			/* Buffer for subnegotiations */

int
#ifdef CK_ANSIC				/* TELNET DO OPTION */
tn_doop( CHAR z, int echo, int (*fn)(int) )
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
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0); 	/* Check protocol */

/* Have IAC, read command character. */

    c = (*fn)(0) & 0xff;		/* Read command character */
    m = c - SE;				/* Check validity */
    if (m < 0 || m > ntelcmds) {
	debug(F101,"tn_doop bad cmd","",c);
	return(0);
    }
    if (seslog && sessft) {		/* Copy to session log, if any. */
	if (zchout(ZSFILE, (char) z) < 0) seslog = 0; /* Log IAC. */
	else if (zchout(ZSFILE, (char) c) < 0) seslog = 0; /* Log command */
    }
    if (c == (CHAR) IAC) return(3);	/* Quoted IAC */
    if (c < SB) return(0);		/* Other command with no arguments. */

/* SB, WILL, WONT, DO, or DONT need more bytes... */

    if ((x = (*fn)(0)) < 0) return(-1);	/* Get the option. */
    x &= 0xff;				/* Trim to 8 bits. */

    if (seslog && sessft)		/* Session log */
      if (zchout(ZSFILE, (char) x) < 0) seslog = 0;

    if ((deblog || debses) && c != SB) {
	sprintf(tn_msg,"TELNET RCVD %s %s",telcmds[m],
		(x < ntnopts) ? tnopts[x] : "UNKNOWN");
	debug(F101,tn_msg,"",x);
	if (debses) tn_debug(tn_msg);
    }

    /* Now handle the command */

    switch (x) {
      case TELOPT_ECHO:			/* ECHO negotiation. */
	switch (c) {			/* Command */
	  case WILL:			/* Host says it will echo. */
	    if (echo) {			/* Only reply if change required. */
		int t;
		debug(F101,"tn_doop got WILL ECHO, echo","",echo);
		t = tn_sopt(DO,x);	/* Please do. */
		debug(F101,"tn_sopt returns","",t);
		t = (t < 0) ? -1 : 2;
		debug(F101,"tn_doop returns","",t);
		return(t);
	    } else return(0);		/* Otherwise no change. */
	  case WONT:			/* Host says it won't echo. */
	    if (!echo)			/* If I'm full duplex */
	      return ((tn_sopt(DONT,x) < 0) ? -1 : 1); /* Switch to half */
	    else return(0);		/* Otherwise, no change.  */
	  case DO:			/* Host wants me to echo */
	    if (tn_sopt(WONT,x) < 0)	/* but the client never echoes */
	      return(-1);		/* back to the server. */
	  default:			/* Don't reply to anything else */
	    return(0);
	}

      case TELOPT_SGA:			/* Suppress Go-Ahead */
	switch (c) {			/* Command... */
	  case WONT:			/* Host says it won't. */
	    if (!sgaflg) {
		sgaflg = 1;		/* Remember. */
		if (tn_sopt(DONT,x) < 0) /* acknowledge, */
		  return(-1);
	    }
	    return(echo ? 0 : 1);	/* Switch to half duplex */
	  case WILL:			/* Server says it will SGA */
	    if (sgaflg || !dosga) {	/* ACK only if necessary */
		if (tn_sopt(DO,x) < 0)
		  return(-1);
		sgaflg = 0;		/* Remember new SGA state. */
	    }
	    return(0);			/* But don't change echo state. */
  
	  case DONT:
/*
  Note: The following is proper behavior, and required for talking to the
  Apertus interface to the NOTIS library system, e.g. at Iowa State U:
  scholar.iastate.edu.  Without this reply, the server hangs forever.  This
  code should not be loop-inducing, since C-Kermit never sends WILL SGA as
  an initial bid, so if DO SGA comes, it is never an ACK.
*/
	  case DO:			/* Server wants me to SGA. */
	    if (tn_sopt(WILL,x) < 0)	/* I have to say WILL SGA, */
	      return(-1);		/* even tho I'm not changing state */
	    return(0);			/* or else server might hang. */
	}

#ifdef TELOPT_TTYPE
      case TELOPT_TTYPE:		/* Terminal Type */
	switch (c) {
	  case DO:			/* DO terminal type. */
	    if (wttflg == 0) {		/* If I haven't said so before, */
		if (tn_sopt((CHAR)WILL,x) < 0) /* say I'll send it if asked. */
		  return(-1);
		wttflg++;
	    }
	    return(0);
	  case SB:
	    n = flag = 0;		/* Flag for when done reading SB */
	    while (n < TSBUFSIZ) {	/* Loop looking for IAC SE */
		if ((y = (*fn)(0)) < 0)	/* Read a byte */
		  return(-1);
		y &= 0xff;		/* Make sure it's just 8 bits. */
		sb[n++] = y;		/* Deposit in buffer. */
		if (seslog && sessft)	/* Take care of session log */
		  if (zchout(ZSFILE, (char) y) < 0)
		    seslog = 0;
		if (y == IAC) {		/* If this is an IAC */
		    if (flag) {		/* If previous char was IAC */
			n--;		/* it's quoted, keep one IAC */
			flag = 0;	/* and turn off the flag. */
		    } else flag = 1;	/* Otherwise set the flag. */
		} else if (flag) { 	/* Something else following IAC */
		    if (y != SE)	/* If not SE, it's a protocol error */
		      flag = 0;
		    break;
		}
	    }
	    if (!flag) {		/* Make sure we got a valid SB */
		debug(F100, "TELNET Subnegotian prematurely broken", "",0);
		return(-1);
	    }
	    if (deblog || debses) {
		int i;
		sprintf(tn_msg,"TELNET RCVD SB %s ",tnopts[TELOPT_TTYPE]);
		for (i = 0; i < n-2; i++) {
		    sprintf(hexbuf,"%02x",(unsigned int) sb[i]);
		    strcat(tn_msg,hexbuf);
		}
		strcat(tn_msg," IAC SE");
		debug(F100,tn_msg,"",0);
		if (debses) tn_debug(tn_msg);
	    }
	    if (sb[0] == 1) {		/* SEND terminal type? */
		if (tn_sttyp() < 0)	/* Yes, so send it. */
		  return(-1);
	    }
	  default:			/* Others, ignore */
	    return(0);
	}
#endif /* TELOPT_TTYPE */

#ifdef CK_NAWS
      case TELOPT_NAWS:			/* Terminal width and height */
	if (c == DO) {
	    /* Get window size again in case it changed */
	    ttgwsiz();
	    if (wnawsflg == 0) {	/* Haven't said WILL/WONT before? */
		if (tn_sopt( (tt_rows < 0 || tt_cols < 0) ?
			    (CHAR) WONT :
			    (CHAR) WILL
			    , x) < 0)	/* Say it */
		  return(-1);
		wnawsflg = 1;		/* And remember I said it. */
	    }
	    if (tt_rows > 0 && tt_cols > 0) {
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
		tn_snaws();		/* And now actually do it. */
	    }
	}
	return(0);
#endif /* CK_NAWS */

      default:				/* All others: refuse */
	switch(c) {
	  case WILL:			/* You will? */
	    if (tn_sopt(DONT,x) < 0)	/* Please don't. */
	      return(-1);		/* (Could this cause a loop?) */
	    break;
	  case DO:			/* You want me to? */
	    if (tn_sopt(WONT,x) < 0)	/* I won't. */
	      return(-1);
	    break;
	  case DONT:			/* You don't want me to? */
	    if (tn_sopt(WONT,x) < 0)	/* I won't. */
	      return(-1);		/* (Could this cause a loop?) */
	  case WONT:			/* You won't? */
	    break;			/* I didn't want you to. */
	}				/* Anything else, treat as user data */
	return(0);
    }
#endif /* TNCODE */
}

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

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    ttn = NULL;

#ifdef OS2
    if (tt_type >= 0 && tt_type <= max_tt)
      ttn = tt_info[tt_type].x_name;
#endif /* OS2 */

    if (tn_term) {			/* Terminal type override? */
	debug(F110,"tn_sttyp",tn_term,0);
	if (*tn_term) ttn = tn_term;
    } else debug(F100,"tn_sttyp no term override","",0);

#ifndef datageneral
    if (!ttn)				/* If no override, */
      ttn = getenv("TERM");		/* get it from the environment. */
#endif /* datageneral */
    if ((ttn == ((char *)0)) || ((int)strlen(ttn) >= TSBUFSIZ))
      ttn = "UNKNOWN";
    sb[0] = (CHAR) IAC;			/* I Am a Command */
    sb[1] = (CHAR) SB;			/* Subnegotiation */
    sb[2] = TELOPT_TTYPE;		/* Terminal Type */
    sb[3] = (CHAR) 0;			/* Is... */
    for (i = 4; *ttn; ttn++,i++)	/* Copy and uppercase it */
      sb[i] = (islower(*ttn)) ? toupper(*ttn) : *ttn;
    ttn = sb;				/* Point back to beginning */
    sb[i++] = (CHAR) IAC;		/* End of Subnegotiation */
    sb[i++] = (CHAR) SE;		/* marked by IAC SE */
    if (ttol((CHAR *)sb,i) < 0)		/* Send it. */
      return(-1);
    sb[i-2] = '\0';			/* For debugging */
    if (deblog || debses) {
	sprintf(tn_msg,"TELNET SENT SB TERMINAL TYPE 00 %s IAC SE",sb+4);
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
    int x = tt_cols, y = tt_rows;

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

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

static CHAR x29err [MAXPADPARMS+3] = { X29_ERROR, INVALID_PAD_PARM, '\0' };


/* Initialize PAD */

extern CHAR padparms[MAXPADPARMS+1];

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
            if ((int)strlen(x29err) > 2) {
                ttol (x29err,(int)strlen(x29err));
                x29err[2] = '\0';
            }
            return (-2);
        case X29_READ_PARMS:
            readpad (ps+1,n/2,x29resp);
            setqbit ();
            ttol (x29resp,(n>1)?(n+1):(2*MAXPADPARMS+1));
            if ((int)strlen(x29err) > 2) {
                ttol (x29err,(int)strlen(x29err));
                x29err[2] = '\0';
            }
            resetqbit();
            break;
        case X29_SET_AND_READ_PARMS:
            setpad (ps+1,n/2);
            readpad (ps+1,n/2,x29resp);
            setqbit();
            ttol (x29resp,(n>1)?(n+1):(2*MAXPADPARMS+1));
            if ((int)strlen(x29err) > 2) {
                ttol (x29err,(int)strlen(x29err));
                x29err [2] = '\0';
            }
            resetqbit();
            return (-2);
        case X29_INVITATION_TO_CLEAR:
            (VOID) x25clear();
            return (-1) ;
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
        if (ttol(x25obuf,obufl) < 0) {
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
                x25intr (intudat) ;
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
        if ( i >= 15 || str [i] < '0' || str [i] > '9' )
	  return (-1);
        c = str [i] - '0';
        if ( i & 1 )
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
        for (i = 0; i < diag.datalen; i++) printf (" %02x",diag.data[i]);
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
    if (ttyfd < 0) return (-1);
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
    } while ( (rest > 0) && (!goteol) );

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
