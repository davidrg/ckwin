/* ckoibm.c
 *
 * TCP/IP interface DLL between C-Kermit and IBM TCP/IP
 *
 * Updated by: Jeffrey Altman (jaltman@secure-endpoints.com)
 *               Secure Endpoints Inc., New York City
 *    SOCKS Support (thanks to David Singer, IBM (dsinger@almaden.ibm.com)
 *    Multiple IP packages
 *
 * Original author:  Kai-Uwe Rommel <rommel@ars.muc.de>
 * Created: Wed May 25 1994
 */

/*
  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#include <os2.h>
#undef COMMENT

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#ifdef __IBMC__
#define off_t _dummy_off_t  /* avoid warning */
#include <types.h>
#undef off_t
#else
#define far _far
#define cdecl _cdecl
#include <types.h>
#endif

#ifdef __TYPES_32H
#define FDPTR                                  /* TCP/IP 2.0 */
#define PACKED
#else
#define FDPTR (short * _Seg16)                 /* TCP/IP 1.2.1 */
#define PACKED _Packed
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#ifdef SOCKS_ENABLED
#include <arpa/nameser.h>
#include <resolv.h>
#endif /* SOCKS_ENABLED */

#define BSD_SELECT
#include <sys/time.h>
#include <sys/select.h>

#define CK_DLL
#include "ckotcp.h"

#ifdef SOCKS_ENABLED
/* SOCKS 4.2 code derived from CSTC_RELEASE 4.2 on ftp.nec.com /pub/security */

static int usesocks = FALSE ;
static int usesocksns = FALSE ;
static int socks_usens = FALSE ;
static struct in_addr socks_server, socks_ns, default_server ;
static int server_count ;
static char * username ;
enum socks_action { deny, direct, sockd } ;

/* Current SOCKS protocol version */
#define SOCKS_VERSION   4

#define CSTC_RELEASE    "4.2"

/*
 **  Response commands/codes
 */
#define SOCKS_CONNECT   1
#define SOCKS_BIND      2
#define SOCKS_RESULT    90
#define SOCKS_FAIL      91
#define SOCKS_NO_IDENTD 92 /* Failed to connect to Identd on client machine */
#define SOCKS_BAD_ID    93 /* Client's Identd reported a different user-id */

typedef unsigned long u_int32;

typedef struct {
    u_int32  host; /* in network byte order */
    unsigned short          port; /* in network byte oreder */
    unsigned char           version;
    unsigned char           cmd;
    } Socks_t;

enum portcmp { e_lt, e_gt, e_eq, e_neq, e_le, e_ge, e_nil };
typedef enum portcmp Portcmp;

/* structure for caching configurations.  this is improves performance in
 * clients when VERSATILE_CLIENTS is defined or in servers
 * when NOT_THROUGH_INETD is defined.
 * Also used in the SOCKS library.
 */

struct config {
    enum socks_action action;
    int use_identd;
    Portcmp tst;
    char *userlist, *serverlist;
    struct in_addr saddr,   /* source addr */
    smask,              /* source mask */
    daddr,          /* destination addr */
    dmask,          /* destination mask */
    interface;      /* interface (for route file) */
    char *cmdp;
    unsigned short dport;
    };

#define CONF_INCR       100     /* step increment for realloc */
void    SOCKS_mkargs();
int     SOCKS_getaddr();
int     SOCKS_getquad();
long    SOCKS_getport();
int     SOCKS_checkuser();
char    socks_src_user[];
struct sockaddr_in       socks_nsin;
ULONG SocksHost;
char *socks_def_server = NULL ;
char *socks_serverlist = NULL ;

static struct config *confPtr, **confNtries = NULL;
static int Ntries = 0;

/*
**  Simple 'SOCKS_mkargs' doesn't handle \, ", or '.
*/
void SOCKS_mkargs(cp, argc, argv, max)
char    *cp;
int     *argc;
char    *argv[];
int     max;
{
        *argc = 0;
        while (isspace(*cp))
                cp++;

        while (*cp != '\0') {
                argv[(*argc)++] = cp;
                if (*argc >= max)
                        return;

                while (!isspace(*cp) && (*cp != '\0'))
                        cp++;
                while (isspace(*cp))
                        *cp++ = '\0';
        }
}


int SOCKS_getquad(dotquad, addr)
char            *dotquad;
struct in_addr  *addr;
/* dotquad must be in dotted quad form. Returns -1 if not. */
{
        if ((addr->s_addr = inet_addr(dotquad)) != (ULONG) -1)
                return 0;
        if (strcmp(dotquad, "255.255.255.255") == 0)
                return 0;
        return -1;
}

/*
**  Get address, must be dotted quad, or full domain name, or network name.
**  Returns -1 if none of the above.
*/
int SOCKS_getaddr(name, addr)
char            *name;
struct in_addr  *addr;
{
        struct hostent  *hp;
        struct netent   *np;

        if (SOCKS_getquad(name, addr) != -1)
                return 0;
        if ((hp = gethostbyname(name)) != NULL) {
                bcopy(hp->h_addr_list[0], &(addr->s_addr), hp->h_length);
                return 0;
        }
        if ((np = getnetbyname(name)) != NULL) {
                addr->s_addr = np->n_net;
                return 0;
        }
        return -1;
}

long SOCKS_getport(name)
char            *name;
{
        struct servent  *sp;

        if ((sp = getservbyname(name, "tcp")) != NULL) {
                return sp->s_port;
        }
        if (!isdigit(*name))
                return -1;
        return htons(atol(name));
}

/*
 *   -1 no socks.conf file or no valid entries
 *    0 everything okay
 */
static int
SOCKS_read_config( void )
{
        FILE            *fd;
        static char     filename[256];
        static char     buf[1024];
        char            *bp;
        int             linenum = 0;
        char    *argv[10];
        int     argc;
        struct in_addr  daddr, dmask;
        int     next_arg;
        enum socks_action  action;
        char    *userlist = NULL, *server_list = NULL;
        long    p;
        unsigned short dport;
        char    *cmdp = NULL;
        struct  in_addr self;
        Portcmp tst;

    /* find the etc directory */
    if ( DosScanEnv("ETC",&bp) )
        return -1 ;

    /* check for the socks.conf or socks.cnf */
    strcpy( filename, bp ) ;
    if ( bp[strlen(bp)] != '\\' )
        strcat( filename, "\\" ) ;
    strcat( filename, "socks.conf" ) ;
    fd = fopen( filename, "r" ) ;

    if ( !fd ) {
        strcpy( filename, bp ) ;
        if ( bp[strlen(bp)] != '\\' )
            strcat( filename, "\\" ) ;
        strcat( filename, "socks.cnf" ) ;
        fd = fopen( filename, "r" ) ;
        }

    if ( !fd )
        return -1 ;

        while (fgets(buf, sizeof(buf) - 1, fd) != NULL) {
                linenum++;
                /*
                **  Comments start with a '#' anywhere on the line
                */
                cmdp = (char *)0;
                if ((bp = index(buf, '\n')) != NULL)
                        *bp = '\0';
                for (bp = buf; *bp != '\0'; bp++) {
                        if (*bp == ':') {
                                *bp++ = '\0';
                                cmdp = bp;
                                break;
                        } else if (*bp == '#') {
                                *bp = '\0';
                                break;
                        } else if (*bp == '\t')
                                *bp = ' ';
                }
                if (strlen(buf) == 0) continue;
                SOCKS_mkargs(buf, &argc, argv, 7);
                if (argc == 0) {
                        continue;
                }

                if ((argc < 3) || (argc > 7)) {
                        printf("Invalid entry at line %d in file %s\n",linenum, filename);
                        continue;
                }

                /* parse the whole entry now, once. */
                next_arg = 1;
                server_list = (char *)0;

                if (!strcmp(*argv, "sockd")) {
                        server_list = socks_def_server;
                        action = sockd;
                        if (strncmp(*(argv +next_arg), "@=", 2) == 0) {
                                server_list = *(argv +next_arg) + 2;
                                if(*server_list == '\0')
                                        server_list = socks_def_server;
                                next_arg++;
                        }
                } else if (strncmp(*argv, "sockd@", 6) == 0) {
                        action = sockd;
                        server_list = *(argv) + 6;
                        if (*server_list == '\0')
                                server_list = socks_def_server;
                } else if (!strcmp(*argv, "direct")) {
                        action = direct;
                } else if (!strcmp(*argv, "deny")) {
                        action = deny;
                } else {
                        printf("Invalid sockd/direct/deny field at line %d in file %s\n", linenum, filename);
                        continue;
                }

                userlist = (char *)0;
                if (strncmp(*(argv +next_arg), "*=", 2) == 0) {
                        if (*(argv +next_arg) +2) userlist = *(argv +next_arg) + 2;
                        next_arg++;
                }
                if(argc <= next_arg+1) {
                        printf("Invalid entry at line %d in file %s\n", linenum, filename);
                        continue;
                }
                if (SOCKS_getaddr(*(argv +next_arg++), &daddr) == -1){
                        printf("illegal destination field at line %d in file %s\n", linenum, filename);
                        continue;
                }
                if (SOCKS_getquad(*(argv +next_arg++), &dmask) == -1) {
                        printf("illegal destination mask at line %d in file %s\n", linenum, filename);
                        continue;
                }
                if (argc > next_arg + 1) {
                        if (!strcmp(*(argv +next_arg), "eq"))
                                tst = e_eq;
                        else if (!strcmp(*(argv +next_arg), "neq"))
                                tst = e_neq;
                        else if (!strcmp(*(argv +next_arg), "lt"))
                                tst = e_lt;
                        else if (!strcmp(*(argv +next_arg), "gt"))
                                tst = e_gt;
                        else if (!strcmp(*(argv +next_arg), "le"))
                                tst = e_le;
                        else if (!strcmp(*(argv +next_arg), "ge"))
                                tst = e_ge;
                        else {
                                printf("Invalid comparison at line %d in file %s\n", linenum, filename);
                                continue;
                        }

                        if (((p = SOCKS_getport(*(argv +next_arg+1))) < 0) ||
                                (p >= (1L << 16))) {
                                printf("Invalid port number at line %d in file %s\n", linenum, filename);
                                continue;
                        } else {
                                dport = p;
                        }
                } else {
                        tst = e_nil;
                        dport = 0;
                }

#ifdef DEBUG
                {
                        char msg[1024];
                        if (userlist)
                                sprintf(msg,"%s %s 0x%08x 0x%08x %s %u",
                                        *argv, userlist, daddr.s_addr, dmask,
                                                tst == e_eq ? "==" :
                                                tst == e_neq ? "!=" :
                                                tst == e_lt ? "<" :
                                                tst == e_gt ? ">" :
                                                tst == e_le ? "<=" :
                                                tst == e_ge ? ">=" : "NIL",
                                                dport);
                        else
                                sprintf(msg,"%s 0x%08x 0x%08x %s %u",
                                        *argv, daddr.s_addr, dmask,
                                                tst == e_eq ? "==" :
                                                tst == e_neq ? "!=" :
                                                tst == e_lt ? "<" :
                                                tst == e_gt ? ">" :
                                                tst == e_le ? "<=" :
                                                tst == e_ge ? ">=" : "NIL",
                                                dport);
                        printf("%s\n", msg);
                }
#endif
                /* we have a parsed line.  cache it. */
                if (!confNtries || confPtr - *confNtries >= Ntries) {
                        /* some systems can't be counted on to handle
                         * realloc(NULL, ...) correctly.
                         */
                        if (confNtries == NULL)
                            confNtries =
                          (struct config **) malloc(CONF_INCR *sizeof(struct config **));
                        else confNtries = (struct config **)
                          realloc(confNtries, (Ntries +CONF_INCR) *sizeof(struct config));
                }
                *(confNtries +Ntries) = (struct config *) malloc(sizeof(struct config));
                confPtr = *(confNtries +Ntries);
                Ntries++;
                confPtr->action = action;
                confPtr->tst = tst;
                if (server_list) {
                        confPtr->serverlist = (char *)malloc(strlen(server_list) +1);
                        strcpy(confPtr->serverlist, server_list);
                }
                else confPtr->serverlist = NULL;
                if (userlist) {
                        confPtr->userlist = (char *)malloc(strlen(userlist) +1);
                        strcpy(confPtr->userlist, userlist);
                }
                else confPtr->userlist = NULL;
                confPtr->daddr.s_addr = daddr.s_addr;
                confPtr->dmask.s_addr = dmask.s_addr;
                if (cmdp) {
                        confPtr->cmdp = (char *) malloc(strlen(cmdp) +1);
                        strcpy(confPtr->cmdp, cmdp);
                }
                else confPtr->cmdp = NULL;
                confPtr->dport = dport;

        }
        fclose(fd);
        if (confNtries == NULL) {
                printf("No valid entires in file %s\n", filename);
                return(-1);
        }
        return 0;
}

int
SOCKS_init( void )
{
    PSZ var ;
    struct hostent *host ;

    socks_server.s_addr = 0 ;
    socks_ns.s_addr = 0 ;

    /* get the resolv info */
    res_init() ;
    server_count = _res.nscount ;
    default_server.s_addr = _res.nsaddr_list[0].sin_addr.s_addr ;

    /* get the username if any */
    if ( !DosScanEnv("USER",&var) )
        username = strdup(var);
    else
        username = "os2user";

    /* get the SOCKS server if any */
    if ( !DosScanEnv("SOCKS_SERVER",&var) ) {
        socks_def_server = strdup( var ) ;
        host = gethostbyname( var ) ;
        if ( host ) {
            memcpy( &socks_server, host->h_addr_list[0],
                sizeof(socks_server));
            }
        else {
            socks_server.s_addr = inet_addr(var);
            }
        }

    /* get the SOCKS Name Server if any */
    if ( socks_server.s_addr && (socks_server.s_addr != -1) ) {
        usesocks = TRUE ;
        if ( !DosScanEnv("SOCKS_NS",&var) ) {
            host = gethostbyname( var ) ;
            if ( host ) {
                memcpy( &socks_ns.s_addr, host->h_addr_list[0],
                    sizeof(socks_ns.s_addr)) ;
                }
            else {
                socks_ns.s_addr = inet_addr( var ) ;
                }

            /* now install this name server as the default */
            if ( socks_ns.s_addr && ( socks_ns.s_addr != -1 ) ) {
                _res.nsaddr_list[0].sin_addr.s_addr = socks_ns.s_addr ;
                _res.nscount = 1 ;
                usesocksns = TRUE ;
                }
            }
        }

    /* read socks.conf file */
    if ( usesocks ) {
        printf( "C-Kermit is SOCKS 4.2 enabled\n" ) ;
        SOCKS_read_config() ;
        }

    return( usesocks ) ;
}

enum socks_action
SOCKS_validate( struct sockaddr_in * dest )
{
    int i = 0 ;
    enum socks_action action = sockd ;

    for ( i = 0 ; i < Ntries ; i++ ) {
        if ( ( confNtries[i]->daddr.s_addr & confNtries[i]->dmask.s_addr ) ==
            ( dest->sin_addr.s_addr & confNtries[i]->dmask.s_addr ) )
            if ( confNtries[i]->tst == e_nil ) {
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_eq) && (dest->sin_port == confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_neq) && (dest->sin_port != confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_lt) && (dest->sin_port < confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_gt) && (dest->sin_port > confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_le) && (dest->sin_port <= confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
            if ((confNtries[i]->tst == e_ge) && (dest->sin_port >= confNtries[i]->dport)){
                action = confNtries[i]->action ;
                break;
                }
        }
    return action ;
}
#endif /* SOCKS_ENABLED */

int ENTRY ck_sockinit(void)
{
#ifdef SOCKS_ENABLED
    SOCKS_init() ;
#endif /* SOCKS_ENABLED */
    return sock_init();
}

/* socket calls */

int ENTRY ck_connect(int socket, struct ck_sockaddr *name, int namelen)
{
    struct sockaddr_in addr;
    int rc ;

#ifdef SOCKS_ENABLED
    char request[100];
    char *next ;
    int packetsize ;
    int sockret ;

    if (usesocks) {

        memset(&addr, 0, sizeof(addr));
        addr.sin_port        = ((struct ck_sockaddr_in *) name)
            ->sin_port;
        addr.sin_family      = ((struct ck_sockaddr_in *) name)
            ->sin_family;
        addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name)
            ->sin_addr.s_addr;

        /* Check local cached values from SOCKS.CONF */
        switch ( SOCKS_validate( &addr ) ) {
            case deny:
                return -1 ;

            case direct:
                break;

            case sockd:
                addr.sin_family = AF_INET ;
                addr.sin_addr = socks_server ;
                addr.sin_port = htons(1080) ;      /* The SOCKS Service */

                /* Connect to the SOCKS Server */
                if ( connect(socket, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
                    return -1 ;
                }

                /* Build the request packet */
                next = request;
                *next++ = SOCKS_VERSION ;
                *next++ = SOCKS_CONNECT ;
                memcpy(next, &((struct ck_sockaddr_in *) name)->sin_port,
                        sizeof(((struct ck_sockaddr_in *) name)->sin_port) ) ;
                next += sizeof(((struct ck_sockaddr_in *) name)->sin_port) ;
                memcpy(next, &((struct ck_sockaddr_in *) name)->sin_addr,
                        sizeof(((struct ck_sockaddr_in *) name)->sin_addr) ) ;
                next += sizeof(((struct ck_sockaddr_in *) name)->sin_addr) ;
                strcpy(next,username) ;
                next += strlen( username ) + 1;

                /* Send the request */
                rc = send(socket, request, next - request, 0) ;

                /* Now retrieve the response */
                packetsize = 8 ; /* VN 1, CD 1, PORT 2, IP 4 */
                next = request ;
                while ( rc > 0 && packetsize > (next - request) ) {
                    rc = recv( socket, next, packetsize - (next - request), 0 ) ;
                    next += rc ;
                }
                sockret = request[1] ;    /* Get the SOCKS return value */
                return ( sockret == SOCKS_RESULT ? 0 : -1 ) ;
            }
    }

    /* Direct connection */
#endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_port        = ((struct ck_sockaddr_in *) name)
        ->sin_port;
    addr.sin_family      = ((struct ck_sockaddr_in *) name)
        ->sin_family;
    addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name)
        ->sin_addr.s_addr;
    rc = connect(socket, (struct sockaddr *) &addr, sizeof(addr));

    return rc  ;
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
#ifdef __SOCKET_32H
  int len ;
#else
short len ;
#endif
  int rc = 0 ;

  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = ((struct ck_sockaddr_in *) name) -> sin_port;
  addr.sin_family      = ((struct ck_sockaddr_in *) name) -> sin_family;
  addr.sin_addr.s_addr = ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr;
  len = sizeof(addr) ;

  rc = accept(socket, (struct sockaddr *) &addr, &len);
  ((struct ck_sockaddr_in *) name) -> sin_port = addr.sin_port ;
  ((struct ck_sockaddr_in *) name) -> sin_family = addr.sin_family ;
  ((struct ck_sockaddr_in *) name) -> sin_addr.s_addr = addr.sin_addr.s_addr;
  *namelen = len ;

  return rc ;
}

int ENTRY ck_ioctl(int socket, int cmd, long *data)
{
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

  return ioctl(socket, cmd, (char *)data, 4);
}

int ENTRY ck_recv(int socket, char *buffer, int length, int flags)
{
  return recv(socket, buffer, length, flags);
}

int ENTRY ck_select(int *fds, int read, int write, int except, long timeout)
{
  /* Warning: if the called select is 16-bit but the calling code is
   * 32-bit, only one fd can be pointed to by fds! However, calling
   * with one "read" socket is the only use of select() in
   * C-Kermit. Might change in the future, so this must be checked. */

  int rc ;
  ULONG nestings ;  /* MustComplete sections */
#ifdef BSD_SELECT
  fd_set rfds;
  struct timeval tv;
  int socket = *fds;
#endif

  if ((read + write + except) != 1)
    return -1;

#ifdef BSD_SELECT
  FD_ZERO(&rfds);
  FD_SET(socket, &rfds);
  tv.tv_sec = tv.tv_usec = 0L;

  if (timeout < 1000)
    tv.tv_usec = (long) timeout * 1000L;
  else
    tv.tv_sec = timeout / 1000L;

  rc =  (read ? select(FD_SETSIZE, &rfds, NULL, NULL, &tv) : 1) &&
        (write ? select(FD_SETSIZE, NULL, &rfds, NULL, &tv) : 1) &&
        (except ? select(FD_SETSIZE, NULL, NULL, &rfds, &tv) : 1) &&
            FD_ISSET(socket, &rfds);
#else
  rc = select(FDPTR fds, read, write, except, timeout);
#endif
    return rc ;
}

int ENTRY ck_send(int socket, char *buffer, int length, int flags)
{
  return send(socket, buffer, length, flags);
}

int ENTRY ck_setsockopt(int socket, int level, int optname,
                  char *optvalue, int optlength)
{
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

  return setsockopt(socket, level, optname, optvalue, optlength);
}

int ENTRY ck_getsockopt(int socket, int level, int optname,
                  char *optvalue, int * optlength)
{
#ifdef __SOCKET_32H
    int len = *optlength;
#else
    short len = (short) *optlength ;
#endif
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
    return rc;
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
  static char * h_addr_list[64] ;
  static struct ck_hostent ckhost = {NULL,NULL,0,0,h_addr_list};
  int i = 0 ;

#ifdef SOCKS_ENABLED
  if ( (host == NULL) && usesocksns ) {
    _res.nscount = server_count ;
    _res.nsaddr_list[0].sin_addr.s_addr = default_server.s_addr ;
    host = gethostbyname(name) ;
    _res.nscount = 1 ;
    _res.nsaddr_list[0].sin_addr.s_addr = socks_ns.s_addr ;
    }
#endif /* SOCKS_ENABLED */

  if (host == NULL)
    return NULL;

  ckhost.h_name     = host -> h_name;
  ckhost.h_aliases  = NULL;
  ckhost.h_addrtype = host -> h_addrtype;
  ckhost.h_length   = host -> h_length;

  while( host->h_addr_list[i] != NULL && i < 63) {
    ckhost.h_addr_list[i] = host->h_addr_list[i] ;
    i++ ;
    }
  ckhost.h_addr_list[63] = NULL ;

  return &ckhost;
}

struct ck_servent * ENTRY ck_getservbyname(char *name, char *protocol)
{
  PACKED struct servent *serv = getservbyname(name, protocol);
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
  static char * h_addr_list[64] ;
  static struct ck_hostent ckhost = {NULL,NULL,0,0,h_addr_list};
  int i = 0 ;

#ifdef SOCKS_ENABLED
  if ( (host == NULL) && usesocksns ) {
    _res.nscount = server_count ;
    _res.nsaddr_list[0].sin_addr.s_addr = default_server.s_addr ;
    host = gethostbyaddr(addr, len, net) ;
    _res.nscount = 1 ;
    _res.nsaddr_list[0].sin_addr.s_addr = socks_ns.s_addr ;
    }
#endif /* SOCKS_ENABLED */

  if (host == NULL)
    return NULL;

  ckhost.h_name     = host -> h_name;
  ckhost.h_aliases  = NULL;
  ckhost.h_addrtype = host -> h_addrtype;
  ckhost.h_length   = host -> h_length;

  while( host->h_addr_list[i] != NULL && i < 63) {
    ckhost.h_addr_list[i] = host->h_addr_list[i] ;
    i++ ;
    }
  ckhost.h_addr_list[63] = NULL ;

  return &ckhost;
}

struct ck_servent * ENTRY ck_getservbyport(int port, char * name)
{
  PACKED struct servent *serv = getservbyport(port, name);
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
#ifdef __SOCKET_32H
    int len = *namelen;
#else
    short len = (short) *namelen ;
#endif
    int rc = 0 ;

    rc = getpeername( socket, (struct sockaddr *) name, &len ) ;
    *namelen = len ;
    return rc;
}

int ENTRY ck_getsockname( int socket, struct ck_sockaddr * name, int * namelen )
{
#ifdef __SOCKET_32H
    int len = *namelen;
#else
    short len = (short) *namelen ;
#endif
    int rc = 0 ;

    rc = getsockname( socket, (struct sockaddr *) name, &len ) ;
    *namelen = len ;
    return rc;
}

#ifdef __SOCKET_32H
int ENTRY ck_addsockettolist( int socket )
{
    int rc = 0 ;
    rc = addsockettolist( socket ) ;
    return rc;
}
#endif

int ENTRY ck_gethostname( char * buf, int len )
{
#ifdef __SOCKET_32H
    int rc = 0 ;
    rc = gethostname( buf, len ) ;
    return rc;
#else
    if (buf) *buf = '\0';
    return(-1);
#endif
}
/* end of ckotcp.c */
