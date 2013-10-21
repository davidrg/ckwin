/* ckotcp.h
 *
 * TCP/IP interface for OS/2 C-Kermit
 *
 * Author:  Kai-Uwe Rommel <rommel@ars.muc.de>
 * Created: Wed May 25 1994
 */

/*
  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef CKOTCP_H
#define CKOTCP_H

#ifdef  NT
#include <nerrno.h>
#endif /* NT */

/* macro functions */

#ifndef CK_DLL
#define byteswap(x)       ((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF))
#define longswap(x)       ((((x) & 0xFF) << 24) | ((((x) >> 8) & 0xFF) << 16) \
                           | ((((x) >> 16) & 0xFF) << 8) | (((x) >> 24) & 0xFF))
#define htons(x)          (byteswap(x))
#define ntohs(x)          (byteswap(x))
#define htonl(x)          (longswap(x))
#define ntohl(x)          (longswap(x))
#define bzero(x, y)       memset((x), 0, (y))
#define bcopy(x, y, z)    memcpy((y), (x), (z))
#endif

/* defines */

#ifndef CK_DLL
#define AF_INET           CK_AF_INET
#define INADDR_ANY        CK_INADDR_ANY
#define SOCK_STREAM       CK_SOCK_STREAM
#define SOCK_DGRAM        CK_SOCK_DGRAM
#define FIONREAD          CK_FIONREAD
#define FIONBIO           CK_FIONBIO
#define SOL_SOCKET        CK_SOL_SOCKET
#define SO_KEEPALIVE      CK_SO_KEEPALIVE
#define SO_LINGER         CK_SO_LINGER
#define SO_SNDBUF         CK_SO_SNDBUF
#define SO_RCVBUF         CK_SO_RCVBUF
#define SO_OOBINLINE      CK_SO_OOBINLINE
#define SO_REUSEADDR      CK_SO_REUSEADDR
#define PF_INET           CK_PF_INET
#define SIOCATMARK        CK_SIOCATMARK
#define MSG_OOB           CK_MSG_OOB
#endif

#define CK_AF_INET        2
#define CK_PF_INET        CK_AF_INET
#define CK_SOCK_STREAM    1
#define CK_SOCK_DGRAM     2

#define CK_FIONREAD       1
#define CK_FIONBIO        2
#define CK_SIOCATMARK     3

#define CK_SOL_SOCKET     0xffff
#define CK_SO_KEEPALIVE   0x0008
#define CK_SO_LINGER      0x0080
#define CK_SO_SNDBUF      0x1001
#define CK_SO_RCVBUF      0x1002
#define CK_SO_OOBINLINE   0x0100
#define CK_SO_REUSEADDR   0x0004

#define CK_MSG_OOB        0x1
#define CK_INADDR_ANY     0x00000000L


/* type definitions */

#ifndef CK_DLL
typedef char *caddr_t;
#define in_addr           ck_in_addr
#define ck_sockaddr_in    sockaddr_in
#define ck_sockaddr       sockaddr
#define ck_hostent        hostent
#define ck_servent        servent
#define ck_linger         linger
#endif

/*
 * Structure used for manipulating linger option.
 */
struct  ck_linger {
        int     l_onoff;                /* option on/off */
        int     l_linger;               /* linger time */
};


struct ck_in_addr
{
  unsigned long s_addr;
};

struct ck_sockaddr_in
{
  short sin_family;
  unsigned short sin_port;
  struct ck_in_addr sin_addr;
  char sin_zero[8];
};

struct ck_sockaddr
{
  unsigned short sa_family;
  char sa_data[14];
};

struct ck_hostent
{
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
#ifndef CK_DLL
#define h_addr h_addr_list[0]
#endif
};

struct ck_servent
{
  char *s_name;
  char **s_aliases;
  int s_port;
  char *s_proto;
};

/* prototypes */

#ifdef CK_DLL
  #ifdef __32BIT__
  #define ENTRY _System _Export
  #else
  #define ENTRY _export _loadds
  #endif
#else
  #ifdef __32BIT__
  #define SYSTEM _System
  #else
  #define SYSTEM
  #endif
  #define ck_sockinit       (* SYSTEM sockinit)
  #define ck_connect        (* SYSTEM connect)
  #define ck_bind           (* SYSTEM bind)
  #define ck_listen         (* SYSTEM listen)
  #define ck_accept         (* SYSTEM accept)
  #define ck_ioctl          (* SYSTEM ioctl)
  #define ck_recv           (* SYSTEM recv)
  #define ck_select         (* SYSTEM select)
  #define ck_send           (* SYSTEM send)
  #define ck_setsockopt     (* SYSTEM setsockopt)
  #define ck_getsockopt     (* SYSTEM getsockopt)
  #define ck_socket         (* SYSTEM socket)
  #define ck_soclose        (* SYSTEM soclose)
  #define ck_gethostbyname  (* SYSTEM gethostbyname)
  #define ck_getservbyname  (* SYSTEM getservbyname)
  #define ck_gethostbyaddr  (* SYSTEM gethostbyaddr)
  #define ck_getservbyport  (* SYSTEM getservbyport)
  #define ck_inet_addr      (* SYSTEM inet_addr)
  #define ck_inet_ntoa      (* SYSTEM inet_ntoa)
  #define ck_getpeername    (* SYSTEM getpeername)
  #define ck_getsockname    (* SYSTEM getsockname)
  #define ck_addsockettolist (* SYSTEM addsockettolist)
  #define ck_gethostname    (* SYSTEM gethostname)
#endif

#ifndef ENTRY
#define ENTRY
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN int ENTRY ck_sockinit(void);

EXTERN int ENTRY ck_connect(int socket, struct ck_sockaddr *name, int namelen);
EXTERN int ENTRY ck_bind(int socket, struct ck_sockaddr *name, int namelen);
EXTERN int ENTRY ck_listen(int socket, int backlog);
EXTERN int ENTRY ck_accept(int socket, struct ck_sockaddr *name, int *namelen);
EXTERN int ENTRY ck_ioctl(int socket, int cmd, long *data);
EXTERN int ENTRY ck_recv(int socket, char *buffer, int length, int flags);
EXTERN int ENTRY ck_select(int *fds, int nread, int write, int except, long timeout);
EXTERN int ENTRY ck_send(int socket, char *buffer, int length, int flags);
EXTERN int ENTRY ck_setsockopt(int socket, int level, int optname,
                               char *optvalue, int optlength);
EXTERN int ENTRY ck_getsockopt(int socket, int level, int optname,
                               char *optvalue, int *optlength);
EXTERN int ENTRY ck_socket(int domain, int type, int protocol);
EXTERN int ENTRY ck_soclose(int socket);

EXTERN struct ck_hostent * ENTRY ck_gethostbyname(char *name);
EXTERN struct ck_servent * ENTRY ck_getservbyname(char *name, char *protocol);
EXTERN struct ck_hostent * ENTRY ck_gethostbyaddr( char *, int, int );
EXTERN struct ck_servent * ENTRY ck_getservbyport( int, char * );
EXTERN unsigned long ENTRY ck_inet_addr(char *cp);
EXTERN char * ENTRY ck_inet_ntoa(struct ck_in_addr in);
EXTERN int ENTRY ck_getpeername(int, struct ck_sockaddr *name, int *namelen);
EXTERN int ENTRY ck_getsockname(int, struct ck_sockaddr *name, int *namelen);
EXTERN int ENTRY ck_addsockettolist(int);
EXTERN int ENTRY ck_gethostname(char *, int);

#ifndef CK_DLL
/* Ensure that IBM header files are not loaded */
#define __SOCKET_32H
#define __UTILS_32H
#define __TYPES_32H
#define __IN_32H
#define __NETDB_32H
#define __IF_32H
#define __IF_ARPA_32H
#define _SYS_PARAM_H_
#define _ARPA_INET_H_

/* For Kerberos 5 */
#define SIZEOF_INT      4
#define SIZEOF_SHORT    2
#define SIZEOF_LONG     4

#define getpid _getpid
#endif /* CK_DLL */

/* end of ckotcp.h */
#endif /* CKOTCP_H */
