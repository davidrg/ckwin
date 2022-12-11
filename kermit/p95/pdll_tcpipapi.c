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

/* Routines to load TCP/IP API */

#ifdef XYZ_DLL
#ifdef OS2
#ifndef NT
#include <stdio.h>
#define INCL_DOSMODULEMGR
#include <os2.h>

#include "ckcdeb.h"

#include "p_type.h"
#include "pdll_error.h"
#include "pdll_modules.h"
#include "pdll_tcpipapi.h"

static HMODULE so32dll_h;
static HMODULE tcp32dll_h;

int (*sock_init)(void) = NULL;
int (*setsockopt)(int, int, int, char *, int) = NULL;
int (*socket)(int, int, int) = NULL;
int (*bind)(int, struct sockaddr *, int) = NULL;
int (*listen)(int, int) = NULL;
int (*ioctl)(int, int, char *, int) = NULL;
int (*accept)(int, struct sockaddr *, int *) = NULL;
int (*connect)(int, struct sockaddr *, int) = NULL;
int (*soclose)(int) = NULL;
int (*select)(int *, int, int, int, long) = NULL;
int (*recv)(int, char *, int, int) = NULL;
int (*send)(int, char *, int, int) = NULL;
struct hostent *(*gethostbyname)(char *) = NULL;
struct hostent *(*gethostbyaddr)(char *, int, int) = NULL;
unsigned long (*inet_addr)(char *) = NULL;
char *(*inet_ntoa)(struct in_addr) = NULL;
int *h_errno = NULL;

int (*sock_errno)(void) = NULL;
VOID (*psock_errno)(char *) = NULL;

unsigned long (*lswap)(unsigned long) = NULL;
unsigned short (*bswap)(unsigned short) = NULL;

VOID load_proc(int h, char *name, PFN *fn) {
  
  APIRET rc=0;

  rc = DosQueryProcAddr(h, 0, name, fn);
  if (rc)
    p_error(P_ERROR_DOSQUERYPROCADDR, rc,
	    MODULE_TCPIPAPI, __LINE__, (int)name);
}

VOID load_tcpip(void) {

  APIRET rc=0;

  rc = DosLoadModule(NULL, 0L, "SO32DLL",  &so32dll_h);
  if (rc)
    p_error(P_ERROR_DOSLOADMODULE, rc,
	    MODULE_TCPIPAPI, __LINE__, (int)"SO32DLL");
  rc = DosLoadModule(NULL, 0L, "TCP32DLL", &tcp32dll_h);
  if (rc)
    p_error(P_ERROR_DOSLOADMODULE, rc,
	    MODULE_TCPIPAPI, __LINE__, (int)"TCP32DLL");

  load_proc(so32dll_h, "SOCK_INIT", (PFN *)&sock_init);
  load_proc(so32dll_h, "SETSOCKOPT", (PFN *)&setsockopt);
  load_proc(so32dll_h, "SOCKET", (PFN *)&socket);
  load_proc(so32dll_h, "BIND", (PFN *)&bind);
  load_proc(so32dll_h, "LISTEN", (PFN *)&listen);
  load_proc(so32dll_h, "IOCTL", (PFN *)&ioctl);
  load_proc(so32dll_h, "ACCEPT", (PFN *)&accept);
  load_proc(so32dll_h, "CONNECT", (PFN *)&connect);
  load_proc(so32dll_h, "SOCLOSE", (PFN *)&soclose);
  load_proc(so32dll_h, "SELECT", (PFN *)&select);
  load_proc(so32dll_h, "RECV", (PFN *)&recv);
  load_proc(so32dll_h, "SEND", (PFN *)&send);
  load_proc(so32dll_h, "SOCK_ERRNO", (PFN *)&sock_errno);
  load_proc(so32dll_h, "PSOCK_ERRNO", (PFN *)&psock_errno);

  load_proc(tcp32dll_h, "GETHOSTBYNAME", (PFN *)&gethostbyname);
  load_proc(tcp32dll_h, "GETHOSTBYADDR", (PFN *)&gethostbyaddr);
  load_proc(tcp32dll_h, "INET_ADDR", (PFN *)&inet_addr);
  load_proc(tcp32dll_h, "INET_NTOA", (PFN *)&inet_ntoa);
  load_proc(tcp32dll_h, "H_ERRNO", (PFN *)&h_errno);
  load_proc(tcp32dll_h, "LSWAP", (PFN *)&lswap);
  load_proc(tcp32dll_h, "BSWAP", (PFN *)&bswap);
}

VOID unload_tcpip(void) {

  APIRET rc=0;

  rc = DosFreeModule(tcp32dll_h);
  if (rc)
    p_error(P_ERROR_DOSFREEMODULE, rc,
	    MODULE_TCPIPAPI, __LINE__, (int)"TCP32DLL");
  rc = DosFreeModule(so32dll_h);
  if (rc)
    p_error(P_ERROR_DOSFREEMODULE, rc,
	    MODULE_TCPIPAPI, __LINE__, (int)"SO32DLL");
}
#endif /* NT */
#endif /* OS2 */
#endif /* XYZ_DLL */
