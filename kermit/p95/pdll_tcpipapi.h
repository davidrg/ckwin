/*
 * Copyright 1995 Jyrki Salmi, Online Solutions Oy (www.online.fi)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef XYZ_DLL
#ifdef NT
#include <winsock.h>
#else /* NT */
/*
 * The structure definitions here are only to make it easier to
 * compile the DLL under different compilers. EMX/GCC has these in
 * its own include files, IBM's TCP/IP has these in another place and
 * so on.
 *
 * If you don't plan to support different compilers for your code, you
 * can change it so that it doesn't need the structure definitions below
 * at all.
 *
 * These are taken from BSD UNIX include files (actually from EMX/GCC
 * include files), so that's why there is this copyright notice:
 */

/*
 * Copyright (c) 1983, 1987, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

struct sockaddr {

  unsigned short sa_family;
  char sa_data[14];
};

struct in_addr {
  unsigned long s_addr;
};

struct sockaddr_in {

  short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct  hostent {
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
#define h_addr  h_addr_list[0]
};

#define	INADDR_ANY	0x00000000

#define	SOCK_STREAM	1		/* stream socket */

#define	SO_REUSEADDR	0x0004		/* allow local address reuse */

#define	SOL_SOCKET	0xffff		/* options for socket level */

#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */

#define _IOC(a,b) ((a<<8)|b)
#define FIONBIO         _IOC('f', 126)

/* This is an OS/2 specific definition */
#define SOCEWOULDBLOCK	10035		/* Operation would block */

extern int (*sock_init)(void);
extern int (*setsockopt)(int, int, int, char *, int);
extern int (*socket)(int, int, int);
extern int (*bind)(int, struct sockaddr *, int);
extern int (*listen)(int, int);
extern int (*ioctl)(int, int, char *, int);
extern int (*accept)(int, struct sockaddr *, int *);
extern int (*connect)(int, struct sockaddr *, int);
extern int (*soclose)(int);
extern int (*select)(int *, int, int, int, long);
extern int (*recv)(int, char *, int, int);
extern int (*send)(int, char *, int, int);
extern struct hostent *(*gethostbyname)(char *);
extern struct hostent *(*gethostbyaddr)(char *, int, int);
extern unsigned long (*inet_addr)(char *);
extern char *(*inet_ntoa)(struct in_addr);
extern int *h_errno;

extern int (*sock_errno)(void);
extern void (*psock_errno)(char *);

#define htonl(x)   (lswap(x))
#define ntohl(x)   (lswap(x))
#define htons(x)   (bswap(x))
#define ntohs(x)   (bswap(x))

extern unsigned long (*lswap)(unsigned long);
extern unsigned short (*bswap)(unsigned short);

extern char *tcpip_errlist(int);

extern void load_tcpip(void);
extern void unload_tcpip(void);
#endif /* NT */
#endif /* XYZ_DLL */
