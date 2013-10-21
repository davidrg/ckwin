/*
  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com),
           Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/* assumes IBM TCP/IP 1.2.1 (or possibly higher) for OS/2 1.x and 2.x */

/* to avoid warnings/errors, you will have to fix the TCP/IP headers :
 *
 * in case of TCP/IP 1.2.1:
 * - in netlib.h, remove the useless SIGALRM definition
 * - in utils.h, replace // comments by regular ones
 * (both headers are not included directly but via types.h) *
 *
 * in case of TCP/IP 2.0:
 * - in types.h, remove the useless SIGALRM definition
 */

#ifdef NT
#include <winsock.h>
#else /* NT */
#include "ckotcp.h"
#endif /* NT */

#define PIPENAML  _MAX_PATH   /* Max length for named pipe name */

#ifdef COMMENT
/* Telnet constants */
#define IAC  255
#define AYT  246
#define IP   244
#define EC   247
#define EL   248
#define AO   245
#define GA   249
#endif /* COMMENT */

#define TCPIPLIB
#define CK_LINGER
#define CK_SOCKBUF
#define CK_NODELAY
#define CK_KEEPALIVE

#ifdef NT
#ifdef COMMENT
#define NT_TCP_OVERLAPPED
#endif /* COMMENT */
#ifndef caddr_t
#define caddr_t char *
#endif /* caddr_t */
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif /* bzero */
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif /* bcopy */
#define socket_errno           WSAGetLastError()
#ifndef NT_TCP_OVERLAPPED
#define socket_read(f,s,n)     recv(f,s,n,0)
#define socket_write(f,s,n)    send(f,s,n,0)
#define socket_recv(f,s,n,o)   recv(f,s,n,o)
#define socket_send(f,s,n,o)   send(f,s,n,o)
#else /* NT_TCP_OVERLAPPED */
_PROTOTYP(int socket_read, (int,char *,int));
_PROTOTYP(int socket_write, (int,char *,int));
_PROTOTYP(int socket_recv, (int,char *,int,int));
_PROTOTYP(int socket_send, (int,char *,int,int));
#endif /* NT_TCP_OVERLAPPED */
#define socket_close closesocket
#define socket_ioctl ioctlsocket
#define strupr _strupr
#else /* NT */
#define socket_errno           errno
#define socket_read(f,s,n)     recv(f,s,n,0)
#define socket_write(f,s,n)    send(f,s,n,0)
#define socket_recv(f,s,n,o)   recv(f,s,n,o)
#define socket_send(f,s,n,o)   send(f,s,n,o)
#define socket_close           soclose
#define socket_ioctl           ioctl
/* this is a bit dangerous, but we always read a long with sioctl */
#endif /* NT */

#ifdef CK_SOCKS
#ifdef CK_SOCKS5
_PROTOTYP(int SOCKSaccept, (int, struct sockaddr *, int *));
_PROTOTYP(int SOCKSbind, (int, struct sockaddr *, int));
_PROTOTYP(int SOCKSconnect, (int, struct sockaddr *, int));
_PROTOTYP(int SOCKSgetsockname, (int, struct sockaddr *, int *));
_PROTOTYP(int SOCKSlisten, (int, int));
#else /* CK_SOCKS5 */
_PROTOTYP(int Raccept, (int, struct sockaddr *, int *));
_PROTOTYP(int Rbind, (int, struct sockaddr *, int));
_PROTOTYP(int Rconnect, (int, struct sockaddr *, int));
_PROTOTYP(int Rgetsockname, (int, struct sockaddr *, int *));
_PROTOTYP(int Rlisten, (int, int));
#endif /* CK_SOCKS5 */
_PROTOTYP(int SOCKS_init, (VOID));
#endif /* CK_SOCKS */

_PROTOTYP( void netinit, (void) );
_PROTOTYP( int netcleanup, (void) );

_PROTOTYP( int os2_netopen, (char *, int *, int) );
_PROTOTYP( int os2_netclos, (void) );
_PROTOTYP( int os2_netflui, (void) );
_PROTOTYP( int os2_nettchk, (void) );
_PROTOTYP( int os2_netbreak, (void) );
_PROTOTYP( int os2_netinc, (int) );
_PROTOTYP( int os2_netxin, (int, char *) ) ;
_PROTOTYP( int os2_nettol, (char *, int) );
_PROTOTYP( int os2_nettoc, (int) );
_PROTOTYP( int os2_netxout, (char *, int) );

/* Can't place pragmas in ck[cu].[chw] modules */
extern CHAR ttibuf[];
#ifndef NT
#pragma seg16(ttibuf)
#endif /* NT */

#ifdef NETCMD
_PROTOTYP( void NetCmdInit, (void) );
_PROTOTYP( void NetCmdCleanup, (void) );
_PROTOTYP( int  NetCmdInBuf, (void) );
_PROTOTYP( int  NetCmdPutStr, (char *) );
_PROTOTYP( int  NetCmdPutChars, (char *, int) );
_PROTOTYP( int  NetCmdPutChar, (char) );
_PROTOTYP( int  NetCmdGetChar, (char *) );
#endif

#ifdef TCPSOCKET
_PROTOTYP(int tcpsrv_open,( char * name, int * lcl, int nett, int timo ));
_PROTOTYP(void tcpsrv_close,(void));
_PROTOTYP(int tcpsocket_open,( char * name, int * lcl, int nett, int timo));
#endif



