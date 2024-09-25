/* ckonov.c
 *
 * TCP/IP interface DLL between C-Kermit and Novell LAN Workplace OS/2 3.0
 *
 * Author:  Kai-Uwe Rommel <rommel@ars.muc.de>
 *          Jeffrey Altman <jaltman@kermit.columbia.edu>
 * Created: Fri Sep 16, 1994
 */

/*
  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include <stdlib.h>
#include <memory.h>

#define BSD_SELECT

#define CK_DLL
#include "ckotcp.h"

#include <os22comp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <netdb.h>
#include <bsd43.h>   /* Novell LAN Workplace OS/2 */
#undef s_addr        /* this causes all sorts of problems */

int ENTRY ck_sockinit(void)
{
  return sock_init();
}

/* socket calls */

int ENTRY ck_connect(int socket, struct ck_sockaddr *name, int namelen)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = ((struct ck_sockaddr_in *) name) -> sin_port;
  addr.sin_family      = ((struct ck_sockaddr_in *) name) -> sin_family;
  addr.sin_addr.S_un.S_addr = ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr;

  return connect(socket, (struct sockaddr *) &addr, sizeof(addr));
}

int ENTRY ck_ioctl(int socket, int cmd, char *data, int length)
{
  switch (cmd)
  {
  case CK_FIONREAD:
    cmd = FIONREAD;
    break;
  case CK_FIONBIO:
    cmd = FIONBIO;
    break;
  default:
    return -1;
  }

  return ioctl(socket, cmd, data);
}

int ENTRY ck_recv(int socket, char *buffer, int length, int flags)
{
  if (flags != 0)
    return -1;

  return recv(socket, buffer, length, flags);
}

int ENTRY ck_select(int *fds, int read, int write, int except, long timeout_ms)
{
  /* Warning: if the called select is 16-bit but the calling code is
   * 32-bit, only one fd can be pointed to by fds! However, calling
   * with one "read" socket is the only use of select() in
   * C-Kermit. Might change in the future, so this must be checked. */

#ifdef BSD_SELECT
  fd_set rfds;
  struct timeval tv;
  ldiv_t time_sec;
  int socket = *fds;
#endif

  if (read != 1 && write != 0 && except != 0)
    return -1;

#ifdef BSD_SELECT
  FD_ZERO(&rfds);
  FD_SET(socket, &rfds);
  time_sec = ldiv( timeout_ms, 1000L );
  tv.tv_sec = time_sec.quot;
  tv.tv_usec = time_sec.rem * 1000L;

  return select(FD_SETSIZE, &rfds, NULL, NULL, &tv) && FD_ISSET(socket, &rfds);
#else
  return select(FDPTR fds, read, write, except, timeout_ms);
#endif
}

int ENTRY ck_send(int socket, char *buffer, int length, int flags)
{
  if (flags != 0)
    return -1;

  return send(socket, buffer, length, flags);
}

int ENTRY ck_setsockopt(int socket, int level, int optname,
                  char *optvalue, int optlength)
{
  if (level == CK_SOL_SOCKET)
    level = SOL_SOCKET;
  else
    return -1;

  if (optname == CK_SO_OOBINLINE)
    optname = SO_OOBINLINE;
  else
    return -1;

  return setsockopt(socket, level, optname, optvalue, optlength);
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
  return soclose(socket);
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
  ckhost.h_address  = host -> h_addr;

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

unsigned long ENTRY ck_inet_addr(char *cp)
{
  return inet_addr(cp);
}

char * ENTRY ck_inet_ntoa(struct ck_in_addr in)
{
  struct in_addr addr;

  addr.S_un.S_addr = in.s_addr;

  return inet_ntoa(addr);
}

/* end of ckotcp.c */
