/* ckoftp.c
 *
 * TCP/IP interface DLL between C-Kermit and FTP PC/TCP
 *
 */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/* PCTCP include file */
#define INCL_DOSEXCEPTIONS
#include <pctcp.h>

/* BSD include files  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <net\if.h>
#include <netdb.h>

#ifndef __IBMC__
#include <stddef.h>
#include <memory.h>
#endif

#define CK_DLL
#include "ckotcp.h"

int ENTRY ck_sockinit(void)
{
  /* no sock_init() in ftp pc/tcp */
  return 0;
}

/* socket calls */

int ENTRY ck_connect(int socket, struct ck_sockaddr *name, int namelen)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = ((struct ck_sockaddr_in *) name) -> sin_port;
  addr.sin_family      = ((struct ck_sockaddr_in *) name) -> sin_family;
  addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr;

  return connect(socket, (struct sockaddr *) &addr, sizeof(addr));
}

int ENTRY ck_bind(int socket, struct ck_sockaddr *name, int namelen)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = ((struct ck_sockaddr_in *) name) -> sin_port;
  addr.sin_family      = ((struct ck_sockaddr_in *) name) -> sin_family;
  addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr;

  return bind(socket, (struct sockaddr *) &addr, sizeof(addr));
}

int ENTRY ck_listen(int socket, int backlog)
{
    return listen( socket, backlog );
}

int ENTRY ck_accept(int socket, struct ck_sockaddr *name, int * namelen)
{
  struct sockaddr_in addr;
  int rc = 0 ;
  short len = sizeof(addr);

  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = ((struct ck_sockaddr_in *) name) -> sin_port;
  addr.sin_family      = ((struct ck_sockaddr_in *) name) -> sin_family;
  addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr;

  rc = accept(socket, (struct sockaddr *) &addr, &len);
  *namelen = len ;
  ((struct ck_sockaddr_in *) name) -> sin_port = addr.sin_port ;
  ((struct ck_sockaddr_in *) name) -> sin_family = addr.sin_family ;
  ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr = addr.sin_addr.s_addr;

  return rc ;
}

int ENTRY ck_ioctl(int socket, int cmd, long *data)
{
    long dat = *data ;
    int rc = 0 ;

    switch (cmd)
    {
      case CK_FIONREAD:
          cmd = FIONREAD;
          break;
      case CK_FIONBIO:
          cmd = FIONBIO;
          break;
      case CK_SIOCATMARK:
          cmd = SIOCATMARK;
          break;
      default:
          return -1;
    }

    rc =  ioctl(socket, cmd, (char *)&dat);
    *data = dat ;
    return rc;
}

int ENTRY ck_recv(int socket, char *buffer, int length, int flags)
{
  return recv(socket, buffer, length,flags);
}

int ENTRY ck_select(int *fds, int read, int write, int except, long timeout_ms)
{
  /* Warning: if the called select is 16-bit but the calling code is
   * 32-bit, only one fd can be pointed to by fds! However, calling
   * with one "read" socket is the only use of select() in
   * C-Kermit. Might change in the future, so this must be checked. */

  int rc ;
  ULONG nestings ;  /* MustComplete sections */

  fd_set rfds;
  struct timeval tv;
  ldiv_t time_sec;
  int socket = *fds;

  if ((read + write + except) != 1)
    return -1;

  FD_ZERO(&rfds);
  FD_SET(socket, &rfds);
  time_sec = ldiv( timeout_ms, 1000L );
  tv.tv_sec = time_sec.quot;
  tv.tv_usec = time_sec.rem * 1000L;

  rc =  (read ? select(FD_SETSIZE, &rfds, NULL, NULL, &tv) : 1) &&
        (write ? select(FD_SETSIZE, NULL, &rfds, NULL, &tv) : 1) &&
        (except ? select(FD_SETSIZE, NULL, NULL, &rfds, &tv) : 1) &&
            FD_ISSET(socket, &rfds);
  return rc;
}

int ENTRY ck_send(int socket, char *buffer, int length, int flags)
{
  return send(socket, buffer, length,flags);
}

int ENTRY ck_setsockopt(int socket, int level, int optname,
                  char *optvalue, int optlength)
{
    int rc=0;
    short  optname16;
    char * optvalue16;
    short  optlength16;

    short  optval16;

    if (level == CK_SOL_SOCKET)
        level = SOL_SOCKET;
    else
        return -1;

    switch ( optname )
    {
    case CK_SO_OOBINLINE:
        optname16 = SO_OOBINLINE ;
        optval16 = (short)*((int *)optvalue);
        rc = setsockopt(socket, level, optname16, &optval16, optlength);
        break;
    case CK_SO_REUSEADDR:
        optname16 = SO_REUSEADDR ;
        break;
    case CK_SO_LINGER:
        optname16 = SO_LINGER ;
        break;
    case CK_SO_KEEPALIVE:
        optname16 = SO_KEEPALIVE ;
        break;
    case CK_SO_SNDBUF:
        optname16 = SO_SNDBUF ;
        break;
    case CK_SO_RCVBUF:
        optname16 = SO_RCVBUF ;
        break;
    default:
        return -1;
    }

  return rc;
}

int ENTRY ck_getsockopt(int socket, int level, int optname,
                  char *optvalue, int * optlength)
{
    short len = *optlength ;
    int rc = 0 ;

  if (level == CK_SOL_SOCKET)
    level = SOL_SOCKET;
  else
    return -1;

   switch ( optname )
   {
   case CK_SO_OOBINLINE:
      optname = SO_OOBINLINE ;
      break;
   case CK_SO_REUSEADDR:
      optname = SO_REUSEADDR ;
      break;
   case CK_SO_LINGER:
      optname = SO_LINGER ;
      break;
   case CK_SO_KEEPALIVE:
      optname = SO_KEEPALIVE ;
      break;
   case CK_SO_SNDBUF:
      optname = SO_SNDBUF ;
      break;
   case CK_SO_RCVBUF:
      optname = SO_RCVBUF ;
      break;
   default:
      return -1;
   }

  rc = getsockopt(socket, level, optname, optvalue, &len);
    *optlength = len ;
    return rc ;
}

int ENTRY ck_socket(int domain, int type, int protocol)
{
  if (domain == CK_AF_INET)
    domain = AF_INET;
  else
    return -1;

  if (type == CK_SOCK_STREAM)
    type = SOCK_STREAM;
  else
    return -1;

  if (protocol != 0)
    return -1;

  return socket(domain, type, protocol);
}

int ENTRY ck_soclose(int socket)
{
  return closesocket(socket);
}

/* utility calls */

struct ck_hostent * ENTRY ck_gethostbyname(char *name)
{
  struct hostent *host = gethostbyname(name);
  static struct ck_hostent ckhost;

  if (host == NULL)
    return NULL;

  ckhost.h_name     = host -> h_name;
  ckhost.h_aliases  = NULL;
  ckhost.h_addrtype = host -> h_addrtype;
  ckhost.h_length   = host -> h_length;
  ckhost.h_addr_list  = host -> h_addr_list;

  return &ckhost;
}

struct ck_servent * ENTRY ck_getservbyname(char *name, char *protocol)
{
  struct servent *serv = getservbyname(name, protocol);
  static struct ck_servent ckserv;

  if (serv == NULL)
    return NULL;

  ckserv.s_name    = serv -> s_name;
  ckserv.s_aliases = NULL;
  ckserv.s_port    = serv -> s_port;
  ckserv.s_proto   = serv -> s_proto;

  return &ckserv;
}

struct ck_hostent * ENTRY ck_gethostbyaddr(char *addr, int len, int net)
{
  struct hostent *host = gethostbyaddr(addr, len, net);
  static struct ck_hostent ckhost;

  if (host == NULL)
    return NULL;

  ckhost.h_name     = host -> h_name;
  ckhost.h_aliases  = NULL;
  ckhost.h_addrtype = host -> h_addrtype;
  ckhost.h_length   = host -> h_length;
  ckhost.h_addr_list  = host -> h_addr_list;

  return &ckhost;
}

struct ck_servent * ENTRY ck_getservbyport(int port, char *name)
{
  struct servent *serv = getservbyport(port, name);
  static struct ck_servent ckserv;

  if (serv == NULL)
    return NULL;

  ckserv.s_name    = serv -> s_name;
  ckserv.s_aliases = NULL;
  ckserv.s_port    = serv -> s_port;
  ckserv.s_proto   = serv -> s_proto;

  return &ckserv;
}

unsigned long ENTRY ck_inet_addr(char *cp)
{
  return inet_addr(cp);
}

char * ENTRY ck_inet_ntoa(struct ck_in_addr in)
{
  struct in_addr addr;

  addr.s_addr = in.s_addr;

  return inet_ntoa(addr);
}

int ENTRY ck_getpeername( int socket, struct ck_sockaddr * name, int * namelen )
{
    int rc ;
    short len = (short) *namelen ;

    rc = getpeername( socket, (struct sockaddr *) name, &len ) ;
    *namelen = len ;
    return rc;
}

int ENTRY ck_getsockname( int socket, struct ck_sockaddr * name, int * namelen )
{
    int rc ;
    short len = (short) *namelen ;

    rc = getsockname( socket, (struct sockaddr *) name, &len ) ;
    *namelen = len ;
    return rc;
}

int ENTRY ck_gethostname( char * buf, int len )
{
    int rc ;
    short slen = (short) len ;

    rc = gethostname( buf, slen ) ;
    return rc;
}

/* end of ckotcp.c */
