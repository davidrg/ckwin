#ifdef NT
char *ckonetv = "Win32 Network support, 8.0.071, 21 Apr 2004";
#else /* NT */
char *ckonetv = "OS/2 Network support, 8.0.071, 21 Apr 2004";
#endif /* NT */

/*  C K O N E T  --  OS/2 and Win32 specific network support  */

/*
  COPYRIGHT NOTICE:

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

/*
   Currently supported network services:

   - DECnet (PATHWORKS) LAT (support resides in this module)
   - Meridian Technologies SuperLAT [Jeffrey Altman]
   - TCP/IP Telnet (for which this module acts as a front end to ckcnet.c)
   - Named pipes  [Jeffrey Altman]
   - NETBIOS      [Jeffrey Altman]
   - COMMAND      [Jeffrey Altman]
   - DLL          [Jeffrey Altman]
   - SSH          [Jeffrey Altman]
*/
#include "ckcdeb.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcasc.h"

#define EXTERN
#include "ckcnet.h"  /* include ckonet.h and this includes ckotcp.h */

#ifndef NETCONN
/*
  Network support not defined.
  Dummy functions here in case #ifdef's forgotten elsewhere.
*/
int                                     /* Open network connection */
os2_netopen(char * name, int * lcl, int nett) {
    return(-1);
}
int                                     /* Close network connection */
os2_netclos(void) {
    return(-1);
}
int                                     /* Check network input buffer */
os2_nettchk(void) {
    return(-1);
}
int                                     /* Flush network input buffer */
os2_netflui(void) {
    return(-1);
}
int                                     /* Send network BREAK */
os2_netbreak(void) {
    return(-1);
}
int                                     /* Input character from network */
os2_netinc(int timo) {
    return(-1);
}
int
os2_netxin(int n,char* buf) {
    return(-1);
}
int                                     /* Output character to network */
os2_nettoc(int c) {
    return(-1);
}
int
os2_nettol(char *s, int n) {
    return(-1);
}
int
os2_netxout(char *s, int n) {
    return(1);
}

#else /* NETCONN is defined (rest of this module...) */

#ifndef __32BIT__
#define far _far
#define near _near
#define pascal _pascal
#endif

#ifdef NPIPE
#include <time.h>
extern char pipename[PIPENAML+1];
#endif /* NPIPE */

#ifdef NT
#include <windows.h>
#define itoa _itoa
#else /* NT */
#define INCL_NOPM
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_ERRORS
#define INCL_DOSDATETIME
#define  INCL_DOSNMPIPES
#include <os2.h>
#undef COMMENT
#endif /* NT */

#ifndef SEM_INDEFINITE_WAIT
#define SEM_INDEFINITE_WAIT INFINITE
#endif /* SEM_INDEFINITE_WAIT */

#include "ckocon.h"
#ifdef NT
#include "cknwin.h"
#endif /* NT */
#include "ckowin.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>

#include <setjmp.h>
#include "ckcsig.h"

#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */

extern int ttnproto, tn_deb;
#ifndef NOTERM
extern int tt_type;
extern char *tn_term;
#endif /* NOTERM */
extern int ck_sleepint ;
char tcpname[512];                      /* For SHOW NET */
int tcp_avail = 0;
#ifdef NT
int winsock_version = 0;
#endif /* NT */
#ifdef DECNET
int dnet_avail = 0;
#endif /* DECNET */
#ifdef SUPERLAT
int slat_avail = 0 ;
char slat_pwd[18] ;
#endif /* SUPERLAT */
USHORT netbiosAvail = 0;

#ifdef DECNET
#ifdef OS2ONLY
#ifdef __32BIT__
#pragma seg16(lcb)
#pragma seg16(latinfo)
#pragma seg16(inbuf)
#else /* __32BIT__ */
#define _Seg16
#define APIENTRY16 APIENTRY
#define APIRET16 USHORT
#endif /* __32BIT__ */
#include "ckolat.h"
static APIRET16 (* APIENTRY16 LATENTRY)(struct lat_cb * _Seg16) = NULL;
static struct lat_cb lcb;
static struct lat_info latinfo;
#endif /* OS2ONLY */
#ifdef NT
#include "cknlat.h"
/******************************************************************************
;                                                                             *
; Function Declarations provided in DECTAL.DLL                                *
;                                                                             *
******************************************************************************/

static int (* InstalledAccess)( BYTE bType ) = NULL;
static int (* InquireServices)( BYTE bType ) = NULL;
static int (* GetNextService)( LPSTR lpServiceName,
                               BYTE bType )=NULL;
static int (* OpenSession)( LPDWORD SessionID,
                            LPSTR lpServiceName,
                            LPVOID ConnectData,
                            BYTE bType)=NULL;
static int (* CloseSession)( DWORD SessionID )=NULL;
static int (* ReadData)( DWORD SessionID, LPSTR lpString,
                         DWORD dwLength )=NULL;
static int (* WriteData)( DWORD SessionID, LPSTR lpString,
                          DWORD dwLength )=NULL;
static int (* SendBreak)( DWORD SessionID )=NULL;
static int (* GetDetailError)( DWORD, DWORD)=NULL;
static int (* DataNotify)( DWORD SessionID, HWND hWnd,
                                          UINT wMsg, DWORD dwEventMask)=NULL;
#endif /* NT */
#endif /* DECNET */

#ifdef SUPERLAT
#include <winioctl.h>
#include <ntddtdi.h>
#include <tihdr.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long queue_t;

#include <tdi.h>
#include <packoff.h>
#include <latioc.h>
#include <packon.h>
#endif /* SUPERLAT */

/* N E T I N C - Input Buffer */
static unsigned long size = 0, pos = 0;
static unsigned char inbuf[MAXRP+1] ;

/* T C P S R V _ O P E N */
/* T C P S O C K E T _ O P E N */
int tcpsrv_open( char * name, int * lcl, int nett, int timo ) ;
void tcpsrv_close(void);
int tcpsocket_open( char * name, int * lcl, int nett, int timo ) ;
#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif /* INADDR_ANY */
int tcpsrv_fd = -1 ;
static unsigned short tcpsrv_port = 0 ;
extern int tcp_incoming;
extern int sstelnet;
extern int tcp_rdns;
extern char *tcp_address;               /* Preferred Local Address */
#ifdef IKSD
extern int inserver;
#endif/* IKSD */

#define NAMECPYL 100                    /* Local copy of hostname */
extern char namecopy[] ;
extern char ipaddr[20] ;                /* Global copy of IP address */
extern int ttnet  ;             /* Network type */

extern int duplex, debses, seslog, ttyfd, quiet, doconx; /* Extern vars */
extern int nettype;
extern int exithangup;
extern char ttname[] ;
extern char myhost[] ;

#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
extern int tcp_nodelay ; /* Nagle algorithm TCP_NODELAY */
#endif /* TCP_NODELAY */
#ifdef SO_DONTROUTE
extern int tcp_dontroute;
#endif /* SO_DONTROUTE */
#ifdef SO_LINGER
extern int tcp_linger ;    /* SO_LINGER */
extern int tcp_linger_tmo ; /* SO_LINGER timeout */
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
extern int tcp_sendbuf ;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
extern int tcp_recvbuf ;
#endif /* SO_RCVBUF */
#ifdef SO_KEEPALIVE
extern int tcp_keepalive ;
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */

#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */


#ifdef NPIPE
#ifdef NT
 HANDLE hPipe = INVALID_HANDLE_VALUE;
#else
 HPIPE  hPipe = 0 ;
#endif /* NT */
 UCHAR  PipeName[PIPENAML+1]; /* Pipe name */
#endif /* NPIPE */

#ifdef CK_NETBIOS
#ifndef NT
#include "ckonbi.h"
#endif /* NT */
extern PNCB pWorkNCB ;
extern PNCB pListenNCB ;
extern PNCB pRecvNCB ;
extern PNCB pSendNCB[MAXWS] ;
extern BYTE NetBiosLSN  ;
extern HEV  hevNetBiosLSN ;
extern UCHAR  NetBiosRemote[NETBIOS_NAME_LEN+1] ;
extern UCHAR  NetBiosName[NETBIOS_NAME_LEN+1] ;
extern UCHAR NetBiosAdapter ;
extern TID ListenThreadID ;
extern BYTE NetBiosRecvBuf[MAXRP] ;
extern BYTE * NetBiosSendBuf[MAXWS] ;
extern USHORT MaxCmds,MaxSess,MaxNames,MaxWs ;
void NetbiosListenThread(void * pArgList);
#endif /* CK_NETBIOS */

#ifdef NETCMD
#ifdef NT
static HANDLE hSaveStdIn=NULL, hSaveStdOut=NULL, hSaveStdErr=NULL;
static HANDLE hChildStdinRd=NULL, hChildStdinWr=NULL, hChildStdinWrDup=NULL,
              hChildStdoutRd=NULL, hChildStdoutRdDup=NULL, hChildStdoutWr=NULL,
              hInputFile=NULL, hSaveStdin=NULL, hSaveStdout=NULL;
static SECURITY_ATTRIBUTES saAttr;
static BOOL fSuccess;
static PROCESS_INFORMATION procinfo ;
static STARTUPINFO         startinfo ;
#else /* NT */
static HFILE hSaveStdIn=-1, hSaveStdOut=-1, hSaveStdErr=-1;
static HFILE hChildStdinRd=-1, hChildStdinWr=-1, hChildStdinWrDup=-1,
             hChildStdoutRd=-1, hChildStdoutRdDup=-1, hChildStdoutWr=-1,
             hInputFile=-1, hSaveStdin=-1, hSaveStdout=-1;
static BOOL fSuccess;
static PID  pid=0;
#define STILL_ACTIVE -1L
#endif /* NT */


/* The following functions will provide the interface for the local echo */
/* buffer to be used when 'duplex' is TRUE                               */

#define NET_CMD_BUFSIZE 512
static USHORT NetCmdBuf[NET_CMD_BUFSIZE] ;
static int NetCmdStart=0, NetCmdEnd=0, NetCmdData=0 ;

void
NetCmdInit( void ) {
   int i;

    CreateNetCmdAvailSem( FALSE );
    CreateNetCmdMutex( TRUE ) ;
    for ( i = 0 ; i < NET_CMD_BUFSIZE ; i++ )
        memset(NetCmdBuf,0,NET_CMD_BUFSIZE*sizeof(USHORT)) ;
    NetCmdStart = 0 ;
    NetCmdEnd = 0 ;
    NetCmdData = 0;
    ReleaseNetCmdMutex() ;
}

void
NetCmdCleanup( void ) {
    CloseNetCmdMutex() ;
    CloseNetCmdAvailSem() ;
}

int
NetCmdInBuf( void ) {
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    if ( NetCmdStart != NetCmdEnd )
    {
        rc = (NetCmdEnd - NetCmdStart + NET_CMD_BUFSIZE)%NET_CMD_BUFSIZE;
    }
    ReleaseNetCmdMutex() ;
    return rc ;
}

int
NetCmdPutStr( char * s )
{
    char * p ;
    int rc = 0 ;
    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    for ( p=s; *p && !rc ; p++ )
      rc = NetCmdPutChar( *p ) ;
    ReleaseNetCmdMutex() ;
    return rc ;
}

int
NetCmdPutChars( char * s, int n )
{
    int rc = 0 ;
    int i = 0;

    ckhexdump("NetCmdPutChars",s,n);
    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    for ( i=0 ; i<n ; i++ )
      rc = NetCmdPutChar( s[i] ) ;
    ReleaseNetCmdMutex() ;
    debug(F101,"NetCmdPutChars","",rc);
    return rc ;
}

int
NetCmdPutChar( char ch ) {
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    while ( (NetCmdStart - NetCmdEnd == 1) ||
            ( NetCmdStart == 0 && NetCmdEnd == NET_CMD_BUFSIZE - 1 ) )
        /* Buffer is full */
    {
        debug(F111,"NetCmdPutChar","Buffer is Full",ch);
        ReleaseNetCmdMutex() ;
        msleep(250);
        RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    }

    NetCmdBuf[NetCmdEnd++] = ch ;
    if ( NetCmdEnd == NET_CMD_BUFSIZE )
        NetCmdEnd = 0 ;
    NetCmdData = TRUE;
    PostNetCmdAvailSem()  ;
    ReleaseNetCmdMutex() ;
    return rc ;
}

int
NetCmdGetChar( char * pch )
{
    int rc = 0 ;

    RequestNetCmdMutex( SEM_INDEFINITE_WAIT ) ;
    if ( NetCmdStart != NetCmdEnd ) {
        *pch = NetCmdBuf[NetCmdStart] ;
        NetCmdBuf[NetCmdStart]=0;
        NetCmdStart++ ;

        if ( NetCmdStart == NET_CMD_BUFSIZE )
          NetCmdStart = 0 ;

        if ( NetCmdStart == NetCmdEnd ) {
            NetCmdData = FALSE;
            ResetNetCmdAvailSem() ;
        }
        rc++ ;
    }
    else
    {
        *pch = 0 ;
    }
    ReleaseNetCmdMutex() ;
    return rc ;
}

#ifdef NT
void
NetCmdReadThread( HANDLE pipe )
{
    int success = 1;
    CHAR c;
    DWORD io;

    while ( success && ttyfd != -1 ) {
        if ( success = ReadFile(pipe, &c, 1, &io, NULL ) )
        {
            NetCmdPutChar(c);
        }
    }
}
#else /* NT */
void
NetCmdReadThread( HFILE pipe )
{
    int success = 1;
    CHAR c;
    ULONG io;

    while ( success && ttyfd != -1 ) {
        if ( success = !DosRead(pipe, &c, 1, &io) )
        {
            NetCmdPutChar(c);
        }
    }
}
#endif  /* NT */
#endif /* NETCMD */

#ifdef NETDLL
HINSTANCE hNetDll=NULL;

int    (*net_dll_netopen)(char *,char *,int,int,
                         int(*)(char*,char*,int))=NULL;
int    (*net_dll_netclos)(void)=NULL;
int    (*net_dll_nettchk)(void)=NULL;
int    (*net_dll_netflui)(void)=NULL;
int    (*net_dll_netbreak)(void)=NULL;
int    (*net_dll_netinc)(int)=NULL;
int    (*net_dll_netxin)(int,char*)=NULL;
int    (*net_dll_nettoc)(int)=NULL;
int    (*net_dll_nettol)(char *,int)=NULL;
int    (*net_dll_ttpkt)(void)=NULL;
int    (*net_dll_ttvt)(void)=NULL;
int    (*net_dll_ttres)(void)=NULL;
void   (*net_dll_terminfo)(char *,int,int)=NULL;
char * (*net_dll_version)(void)=NULL;
char * (*net_dll_errorstr)(int)=NULL;

int
netdll_unload(void)
{
    CloseHandle(hNetDll);
    hNetDll = NULL;

    net_dll_netopen=NULL;
    net_dll_netclos=NULL;
    net_dll_nettchk=NULL;
    net_dll_netflui=NULL;
    net_dll_netbreak=NULL;
    net_dll_netinc=NULL;
    net_dll_netxin=NULL;
    net_dll_nettoc=NULL;
    net_dll_nettol=NULL;
    net_dll_terminfo=NULL;
    net_dll_version=NULL;
    net_dll_errorstr=NULL;
    net_dll_ttpkt=NULL;
    net_dll_ttvt=NULL;
    net_dll_ttres=NULL;
    return(0);
}

int
netdll_load( char * dllname )
{
    int rc=0;

    hNetDll = LoadLibrary(dllname) ;
    if ( !hNetDll )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load LoadLibrary failed",dllname,rc) ;
        if ( rc == 126 )
            printf("?Unable to find %s or a required module\n",dllname);
        else
            printf("?Unable to load %s\n",dllname);
        bleep(BP_FAIL);
        return (-1);
    }

    if (((FARPROC) net_dll_netopen =
          GetProcAddress( hNetDll, "netopen" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netopen",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_netclos =
          GetProcAddress( hNetDll, "netclos" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netclos",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_nettchk =
          GetProcAddress( hNetDll, "nettchk" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","nettchk",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_netflui =
          GetProcAddress( hNetDll, "netflui" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netflui",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_netbreak =
          GetProcAddress( hNetDll, "netbreak" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netbreak",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_netinc =
          GetProcAddress( hNetDll, "netinc" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netinc",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_netxin =
          GetProcAddress( hNetDll, "netxin" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","netxin",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_nettoc =
          GetProcAddress( hNetDll, "nettoc" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","nettoc",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_nettol =
          GetProcAddress( hNetDll, "nettol" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","nettol",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_terminfo =
          GetProcAddress( hNetDll, "terminfo" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","terminfo",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_version =
          GetProcAddress( hNetDll, "version" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","version",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_errorstr =
          GetProcAddress( hNetDll, "errorstr" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","errorstr",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_ttpkt =
          GetProcAddress( hNetDll, "ttpkt" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","ttpkt",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_ttres =
          GetProcAddress( hNetDll, "ttres" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","ttres",rc) ;
        netdll_unload();
        return(-1);
    }
    if (((FARPROC) net_dll_ttvt =
          GetProcAddress( hNetDll, "ttvt" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "netdll_load GetProcAddress failed","ttvt",rc) ;
        netdll_unload();
        return(-1);
    }

    printf("%s loaded.\n",net_dll_version());
    return(0);
}
#endif /* NETDLL */

#ifdef NT
/*
WinSOCK --
*/

int WSASafeToCancel = 0 ;
#endif /* NT */

extern int ishandle;
static int NulCheck = 0;

/*  N E T O P E N  --  Open a network connection.  */
/*  Returns 0 on success, -1 on failure.  */

/*
  Calling conventions same as ttopen(), except third argument is network
  type rather than modem type.  Designed to be called from within ttopen.
*/
int
os2_netopen(name, lcl, nett) char *name; int *lcl, nett; {

#ifdef NPIPE
#ifdef NT
    DWORD   OpenMode;
    DWORD   PipeMode;
    DWORD   OutBufSize;
    DWORD   InBufSize;
    DWORD   TimeOut;
    DWORD   AvailData ;
#else
   ULONG   OpenMode;
   ULONG   PipeMode;
   ULONG   OutBufSize;
   ULONG   InBufSize;
   ULONG   TimeOut;

   ULONG   ActionTaken ;
   ULONG   OpenFlag ;

   ULONG   BytesRead ;
   AVAILDATA AvailData ;
   ULONG   PipeState ;
#endif /* NT */
#endif /* NPIPE */

   int rc = -1;

    errno = 0;
        nettype = nett;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        debug(F100,"os2_netopen calling ssh_open()","",0);
        rc = ssh_open();
        if (rc == 0) {
            ttnet = nett;
            ttyfd = -3;
        }
        return(rc);
    }
#endif /* SSHBUILTIN */

#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        ULONG PostCount ;
        UCHAR RemoteName[NETBIOS_NAME_LEN+1] = "                \0" ;

        if ( !netbiosAvail )
          return -1 ;

        ckstrncpy( RemoteName, name, NETBIOS_NAME_LEN+1 ) ;

        if ( NetBiosLSN > 0             /* Make sure a handle doesn't exist */
             || ttyfd > -1 )
          return 0 ;

        DosResetEventSem( hevNetBiosLSN, &PostCount ) ;

        if ( !strcmp( "*               ", RemoteName ) ) { /* Server Mode */
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS)
              return 0 ;

            printf("Listening for a NetBios connection\n") ;
            rc = NCBListen( NetbeuiAPI,
                            pListenNCB, NetBiosAdapter, NetBiosName, RemoteName,
                            NB_RECV_TIMEOUT, NB_SEND_TIMEOUT, FALSE ) ;
            if ( rc )
              return -1 ;

            ttyfd = NetBiosLSN = 0 ;

            ListenThreadID = _beginthread( &NetbiosListenThread, 0, 16384, 0 );
            if ( ListenThreadID == -1 ) {
                Dos16SemWait( pListenNCB->basic_ncb.ncb_semaphore,
                              SEM_INDEFINITE_WAIT ) ;
                switch (pListenNCB->basic_ncb.bncb.ncb_retcode) {
                case NB_COMMAND_SUCCESSFUL:
                    ttyfd = NetBiosLSN = pListenNCB->basic_ncb.bncb.ncb_lsn ;
                    return 0 ;
                    break;
                default:
                    return -1 ;
                }
            }
            return 0 ;
        }
        else
        {                               /* Remote Mode */
            int oldalarm = alarm(0) ;
            printf("Calling \"%s\" via NetBios\n", RemoteName ) ;
            rc = NCBCall( NetbeuiAPI, pWorkNCB, NetBiosAdapter, NetBiosName,
                          RemoteName, NB_RECV_TIMEOUT, NB_SEND_TIMEOUT, FALSE ) ;
            rc = Dos16SemWait( pWorkNCB->basic_ncb.ncb_semaphore,
                               SEM_INDEFINITE_WAIT ) ;
            if (rc)
              return -1 ;

            switch ( pWorkNCB->basic_ncb.bncb.ncb_retcode ) {
            case NB_COMMAND_SUCCESSFUL:
                ckstrncpy( NetBiosRemote, pWorkNCB->basic_ncb.bncb.ncb_callname,
                         NETBIOS_NAME_LEN ) ;
                ttyfd = NetBiosLSN = pWorkNCB->basic_ncb.bncb.ncb_lsn ;
                NCBReceive( NetbeuiAPI, pRecvNCB, NetBiosAdapter, NetBiosLSN,
                            NetBiosRecvBuf, sizeof(NetBiosRecvBuf), FALSE ) ;
                Dos16SemClear(pListenNCB->basic_ncb.ncb_semaphore ) ;
                DosPostEventSem( hevNetBiosLSN ) ;
                rc = 0 ;
                break;
            case ERROR_FILE_NOT_FOUND:
            case ERROR_ACCESS_DENIED:
            case ERROR_INVALID_PARAMETER:
            default:
                rc = -1 ;
            }
            alarm(oldalarm) ;
            return rc ;
        }
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
   if ( nettype == NET_PIPE ) {
       if ( hPipe ) {                   /* Make sure a pipe isn't open */
           char buffer[64];
#ifdef NT
           if (PeekNamedPipe(hPipe, NULL, 0, NULL, &AvailData, NULL))
               return 0;
           
           rc = GetLastError();
           if ( rc == ERROR_BAD_PIPE || rc == ERROR_PIPE_NOT_CONNECTED )
               ttclos(0);
#else
           rc = DosPeekNPipe(hPipe, buffer, sizeof(buffer),
                             &BytesRead, &AvailData, &PipeState);
           switch ( rc ) {
             case NO_ERROR:
               return 0 ;
             case ERROR_BAD_PIPE:
             case ERROR_PIPE_NOT_CONNECTED:
               ttclos(0);
               break;
             default:
               break;
           }
#endif
       }
#ifdef NT
       if ( *name == '\0' ) {
           ckstrncpy( PipeName, "\\PIPE\\Kermit", PIPENAML+1 ) ;
       } else {
           ckstrncpy( PipeName, name, PIPENAML+1 ) ;
       }

       rc = 0;
       if ( PipeName[0] == '\\' && PipeName[1] == '\\' ) {
           /* Client Mode */
           hPipe = CreateFile( PipeName, 
                               PIPE_ACCESS_DUPLEX,
                               FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL, /* security */
                               OPEN_EXISTING,
                               0,   /* ignored */
                               NULL );
           if ( hPipe == INVALID_HANDLE_VALUE ) {
               rc = GetLastError();
               debug( F101, "CreateFile error: return code","",rc) ;
               printf( "\nCreateFile error: return code = %ld\n",rc ) ;
           } else {
               DWORD Mode = PIPE_NOWAIT | PIPE_READMODE_BYTE;
               ttyfd = (int) hPipe ;
               SetNamedPipeHandleState( hPipe, &Mode, NULL, NULL) ;
               printf("\nNamed Pipe %s open.\nConnection to server complete.\n",
                      PipeName);
           }
       } else {                         /* Server Mode */
           int i;
		   if (strlen(PipeName) + 3 > PIPENAML) {
			   printf("?Pipename too long\n");
			   return -1;
		   }

           for ( i=strlen(PipeName)+1; i>=0; i--) {
               PipeName[i+3] = PipeName[i];
           }
           PipeName[0] = '\\';
           PipeName[1] = '\\';
           PipeName[2] = '.';

/* Visual C++ 6 doesn't know about this */
#ifndef FILE_FLAG_FIRST_PIPE_INSTANCE
#define FILE_FLAG_FIRST_PIPE_INSTANCE 0x00080000
#endif           
           
           OpenMode = PIPE_ACCESS_DUPLEX | 
                      FILE_FLAG_FIRST_PIPE_INSTANCE |
                      FILE_FLAG_WRITE_THROUGH;
           PipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT;
           OutBufSize = MAXSP + 4 ;
           InBufSize = MAXRP + 4 ;
           TimeOut = 10000;
           hPipe = CreateNamedPipe( PipeName, OpenMode, PipeMode, 0x01, 
                                    OutBufSize, InBufSize, TimeOut, NULL);
           if (hPipe == INVALID_HANDLE_VALUE) {
               rc = GetLastError();
               debug( F101,"CreateNamedPipe error: return code","",rc) ;
               printf("\nCreateNamedPipe error: return code = %ld\n", rc);
           } else {
               ttyfd = (int) hPipe;
               printf("\nNamed Pipe %s created.\nWaiting for client connection ...\n\n",
                       PipeName ) ;
               if (!ConnectNamedPipe( hPipe, NULL )) {
                   rc = GetLastError();
                   if (rc != ERROR_PIPE_NOT_CONNECTED) {
                       debug( F101,"DosConnectNPipe error: return code","",rc) ;
                       printf("\nDosConnectNPipe error: return code = %ld\n", rc);
                   }
               } else {
                   DWORD Mode = PIPE_NOWAIT | PIPE_READMODE_BYTE;
                   ttyfd = (int) hPipe ;
                   SetNamedPipeHandleState( hPipe, &Mode, NULL, NULL) ;
               }
           }
       }
#else /* NT */
       if ( *name == '\0' ) {
           ckstrncpy( PipeName, "\\PIPE\\Kermit", PIPENAML+1 ) ;
       } else {
           ckstrncpy( PipeName, name, PIPENAML+1 ) ;
       }

       if ( PipeName[0] == '\\' && PipeName[1] == '\\' ) {
           /* Client Mode */
           OpenFlag = OPEN_ACTION_OPEN_IF_EXISTS ;
           OpenMode = OPEN_FLAGS_WRITE_THROUGH |
                      OPEN_FLAGS_FAIL_ON_ERROR |
                        OPEN_FLAGS_RANDOM |
                           OPEN_SHARE_DENYNONE |
                           OPEN_ACCESS_READWRITE ;

           if (PipeName[2] == '.' && PipeName[3] == '\\') { /* Internal use. */
               int i, n = strlen(PipeName);
               for (i = 3; i <= n; i++)
                 PipeName[i-3] = PipeName[i];
           }

           rc = DosOpen( PipeName, &hPipe, &ActionTaken, 0, 0,
                        OpenFlag, OpenMode, 0 ) ;
           switch (rc) {
             case NO_ERROR:
               ttyfd = hPipe ;
               DosSetNPHState( hPipe, NP_NOWAIT | NP_READMODE_BYTE ) ;
               printf(
                     "\nNamed Pipe %s open.\nConnection to server complete.\n",
                      PipeName
                      );
               break;
             case ERROR_BAD_NETPATH:
               printf("\nInvalid Server specified in Pipe Name: %s\n",
                      PipeName ) ;
               break;
             case ERROR_PATH_NOT_FOUND:
             case ERROR_FILE_NOT_FOUND:
               printf("\nNonexistent Named Pipe: %s\n", PipeName ) ;
               break;
             case ERROR_PIPE_BUSY:
               printf("\nNamed Pipe Already in Use: %s\n", PipeName ) ;
               break;
             default:
               debug( F101, "DosOpen error: return code","",rc) ;
               printf( "\nDosOpen error: return code = %ld\n",rc ) ;
           }
       } else {                         /* Server Mode */
           OpenMode = NP_ACCESS_DUPLEX | NP_INHERIT | NP_NOWRITEBEHIND |
             NP_TYPE_BYTE | NP_READMODE_BYTE ;
           PipeMode = NP_NOWAIT | 0x01;
           OutBufSize = MAXSP + 4 ;
           InBufSize = MAXRP + 4 ;
           TimeOut = 10000;
           rc = DosCreateNPipe( PipeName, &hPipe, OpenMode,
                               PipeMode, OutBufSize, InBufSize, TimeOut);
           switch (rc) {
             case NO_ERROR:
               ttyfd = hPipe ;
               rc = DosConnectNPipe( hPipe ) ;
               if (rc == NO_ERROR || rc == ERROR_PIPE_NOT_CONNECTED) {
     printf("\nNamed Pipe %s created.\nWaiting for client connection ...\n\n",
                  PipeName ) ;
                  rc = 0 ;
               } else {
                   debug( F101,"DosConnectNPipe error: return code","",rc) ;
                   printf("\nDosConnectNPipe error: return code = %ld\n", rc);
               }
               break;
             case ERROR_PATH_NOT_FOUND:
               printf("\nInvalid Pipe Name: %s\n", PipeName ) ;
               break;
             case ERROR_PIPE_BUSY:
               printf("\nNamed Pipe Already in Use: %s\n", PipeName ) ;
               break;
             default:
               debug( F101,"DosCreateNPipe error: return code","",rc) ;
               printf("\nDosCreateNPipe error: return code = %ld\n", rc);
           }
       }
#endif /* NT */
       return ( rc ? -1 : 0 ) ;
   }
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB )
    {
        if (!tcp_avail)
            return -1 ;

#ifdef CK_SOCKS
        SOCKS_init();
#endif /* CK_SOCKS */

        switch ( name[0] )
        {
        case '*':
            /* server mode -- not supported in ckcnet.c */
            rc = tcpsrv_open(name, lcl, nett, 0) ;
            break;
        case '_':
            /* raw socket number -- not supported in ckcnet.c */
            rc = tcpsocket_open(name, lcl, nett, 0);
            ishandle = 1;    /* Remember so we don't close it */
            exithangup = 0;  /* Do not close on exit */
            break;
        case '$':
            /* server side raw socket number -- not supported in ckcnet.c */
            tcp_incoming = 1;
            sstelnet = 1;
            rc = tcpsocket_open(name, lcl, nett, 0);
            break;
#ifdef OS2ONLY
        case '!':
            /* raw socket number -- not supported in ckcnet.c */
            rc = tcpsocket_open(name, lcl, nett, 0);
            break;
#endif /* OS2ONLY */
        default:
            rc = netopen(name, lcl, nett) ;
        }
        if (!rc) {
            DialerSend( OPT_KERMIT_CONNECT, 0 ) ;
        }
        return(rc);
    }
#endif /* TCPSOCKET */

#ifdef DECNET
   if ( nettype == NET_DEC ) {
#ifdef OS2ONLY
       if ( LATENTRY == NULL )
          return -1;
#endif /* OS2ONLY */

       if (ttnproto == NP_LAT) {
#ifdef OS2ONLY
           memset( &lcb, 0, sizeof(struct lat_cb) ) ;
           lcb.LatFunction = START_SESSION;
           lcb.BufferSize = strlen(name);
           lcb.BufferPtr = (void * _Seg16) name;

           LATENTRY(&lcb);

           ttyfd = lcb.SessionHandle;
           printf(lcb.LatStatus ? "failed.\n" : "OK.\n");

           rc = (lcb.LatStatus == 0) ? 0 : -1;
#endif /* OS2ONLY */
#ifdef NT
           DWORD SessionID=0;
           rc = OpenSession( &SessionID, name, NULL, LAT_ACCESS );
           if ( rc ) {
               printf("failed.\n");
               debug(F111,"DECNet LAT OpenSession failed",name,rc);
               rc = -1;
           }
           else {
               printf("OK.\n");
               ttyfd = SessionID;
           }
#endif /* NT */
       }
       else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
           rc = -1;             /* Not supported */
#endif /* OS2ONLY */
#ifdef NT
           DWORD SessionID=0;
           rc = OpenSession( &SessionID, name, NULL, CTERM_ACCESS );
           if ( rc ) {
               printf("failed.\n");
               debug(F111,"DECNet CTERM OpenSession failed",name,rc);
               rc = -1;
           }
           else {
               printf("OK.\n");
               ttyfd = SessionID;
           }
#endif /* NT */
       }
   }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      L_bind bindstruct ;
      ULONG BytesReturned = 0;
      TA_LAT_ADDRESS connectstruct ;
      OVERLAPPED OverLapped ;
      HANDLE thehEvent ;
      int i=0, service = 0 ;
      char * port = NULL ;

      ttnproto = NP_LAT ;

      /* Create a file handle, first try NT way, then try Win95 way */
      ttyfd = (int) CreateFile ("\\\\.\\Lat",
                        GENERIC_READ    | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL);

      if ((HANDLE) ttyfd == INVALID_HANDLE_VALUE)
      {
         ttyfd = (int) CreateFile ("\\\\.\\slatwin",
                           GENERIC_READ    | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                           NULL);
         if ( (HANDLE) ttyfd == INVALID_HANDLE_VALUE )
         {
            ttyfd = -1 ;
            return -1 ;
         }
      }

      memset( &bindstruct, 0, sizeof(bindstruct) ) ;
      bindstruct.type = L_CLIENT ;

      if ( !DeviceIoControl( (HANDLE) ttyfd, IOCTL_TDI_ASSOCIATE_ADDRESS,
                             &bindstruct, sizeof(bindstruct),
                             NULL, 1, &BytesReturned, NULL ) )
      {
         printf( "... Error: bind ack failed\n") ;
         CloseHandle( (HANDLE) ttyfd ) ;
         ttyfd = -1 ;
         return -1;
      }

      service = 1 ;  /* assume it is a service name */
      for ( i=0 ; name[i] ; i++ )
         if ( name[i] == '/' )
         {
            service = 0 ;    /* no its a node/port specification */
            port = & name[i+1] ;
            break;
         }
      if ( service )
      {
         connectstruct.Address[0].Address[0].CONN_type = L_SESSION;
         ckstrncpy (connectstruct.Address[0].Address[0].CONN_service,
                     name, NA_MAX);
         connectstruct.Address[0].Address[0].CONN_node[0] = 0 ;
         connectstruct.Address[0].Address[0].CONN_port[0] = 0 ;
      }
      else
      {
         connectstruct.Address[0].Address[0].CONN_type = L_HIC;
         connectstruct.Address[0].Address[0].CONN_service[0] = 0 ;
         ckstrncpy(connectstruct.Address[0].Address[0].CONN_node,
                    name,NA_MAX);
         ckstrncpy(connectstruct.Address[0].Address[0].CONN_port,
                    port,NA_MAX);
      }

      if ( slat_pwd[0] )
         ckstrncpy( connectstruct.Address[0].Address[0].CONN_password,
                    slat_pwd, NA_MAX ) ;
      else
         connectstruct.Address[0].Address[0].CONN_password[0] = 0 ;

      connectstruct.Address[0].AddressLength = sizeof (L_connect);
      connectstruct.Address[0].AddressType = TDI_ADDRESS_TYPE_LAT;
      connectstruct.TAAddressCount = 1;

      OverLapped.Offset = 0;
      OverLapped.OffsetHigh = 0;
      thehEvent = CreateEvent (0, TRUE, 0, NULL);
      OverLapped.hEvent = thehEvent;

      if ( !DeviceIoControl ((HANDLE) ttyfd, IOCTL_TDI_CONNECT,
                                  &connectstruct, sizeof (connectstruct),
                                  NULL, 0, &BytesReturned, &OverLapped))
      {
         if (GetLastError() == ERROR_IO_PENDING)
            GetOverlappedResult ((HANDLE) ttyfd, &OverLapped, &BytesReturned, TRUE);
         else BytesReturned = 1;
      }

      if (BytesReturned)
      {
         printf ("Connect failure..\r\n");
         CloseHandle( (HANDLE) ttyfd ) ;
         CloseHandle( thehEvent ) ;
         ttyfd = -1 ;
         return -1 ;
      }

      rc = 0 ;
   }
#endif /* SUPERLAT */

#ifdef NETFILE
   if ( nettype == NET_FILE )
   {
#ifdef NT
      OVERLAPPED OverLapped ;

      /* Create a file handle */
      ttyfd = (int) CreateFile (name,
                        GENERIC_READ   ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

      if ((HANDLE) ttyfd == INVALID_HANDLE_VALUE)
      {
         ttyfd = -1 ;
         return -1 ;
      }
       rc = 0 ;
#else /* NT */
       ULONG Action;
       rc = DosOpen( name, (PHFILE) &ttyfd, &Action, 0, FILE_NORMAL,
                     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0);
       if ( rc ) {
           ttyfd = -1;
           return -1;
       }
#endif /* NT */
   }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
        char cmd_line[256], *cmd_exe, *args, *p;
        int argslen;

#ifdef NT
        hSaveStdOut = GetStdHandle( STD_OUTPUT_HANDLE ) ;
        hSaveStdIn  = GetStdHandle( STD_INPUT_HANDLE ) ;
        hSaveStdErr = GetStdHandle( STD_ERROR_HANDLE ) ;

        /* Set the bInheritHandle flag so pipe handles are inherited. */

        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        /*
        * The steps for redirecting child's STDOUT:
        *     1.  Save current STDOUT, to be restored later.
        *     2.  Create anonymous pipe to be STDOUT for child.
        *     3.  Set STDOUT of parent to be write handle of pipe, so
        *         it is inherited by child.
        */

        /* Create a pipe for the child's STDOUT. */

        if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 65535))
            debug(F100,"Stdout pipe creation failed\n","",0);

        /* Set a write handle to the pipe to be STDOUT. */

        if (! SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr) ||
             ! SetStdHandle(STD_ERROR_HANDLE, hChildStdoutWr) )
            debug(F100,"Redirecting STDOUT/STDERR failed","",0);

        /*
        * The steps for redirecting child's STDIN:
        *     1.  Save current STDIN, to be restored later.
        *     2.  Create anonymous pipe to be STDIN for child.
        *     3.  Set STDIN of parent to be read handle of pipe, so
        *         it is inherited by child.
        *     4.  Create a noninheritable duplicate of write handle,
        *         and close the inheritable write handle.
        */

        /* Create a pipe for the child's STDIN. */

        if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 65535))
            debug(F100,"Stdin pipe creation failed\n","",0);

        /* Set a read handle to the pipe to be STDIN. */

        if (! SetStdHandle(STD_INPUT_HANDLE, hChildStdinRd))
            debug(F100,"Redirecting Stdin failed","",0);

        /* Duplicate the write handle to the pipe so it is not inherited. */

        fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
                                  GetCurrentProcess(), &hChildStdinWrDup, 0,
                                  FALSE,       /* not inherited */
                                  DUPLICATE_SAME_ACCESS);
        if (! fSuccess) {
            debug(F100,"DuplicateHandle failed","",0);

            SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut );
            SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn );
            SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr );

            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWr);  hChildStdinWr = NULL;
        }

        fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                                    GetCurrentProcess(), &hChildStdoutRdDup, 0,
                                    FALSE,       /* not inherited */
                                    DUPLICATE_SAME_ACCESS);
        if (! fSuccess) {
            debug(F100,"DuplicateHandle failed","",0);

            SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut );
            SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn );
            SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr );

            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWr);  hChildStdinWr = NULL;
        }

        CloseHandle(hChildStdinWr);
        hChildStdinWr = NULL;

        cmd_line[0] = '\0' ;
        /* Now create the child process. */

        cmd_exe = getenv("SHELL");
        if ( !cmd_exe )
            cmd_exe = getenv("COMSPEC");
        if ( !cmd_exe )
            cmd_exe = "cmd.exe";
        ckstrncpy(cmd_line, cmd_exe,256);
        args = cmd_line + strlen(cmd_line); /* don't skip zero */
        argslen = 256-strlen(cmd_line);

        /* Look for MKS Shell, if found use -c instead of /c */
        _strlwr(cmd_exe);
        debug(F110,"os2_netopen NET_CMD cmd_exe",cmd_exe,0);
        p = strstr(cmd_exe,"sh.exe");
        if ( !p )
            p = strstr(cmd_exe,"bash.exe");
        if ( !p )
            p = strstr(cmd_exe,"ash.exe");
        if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
            sprintf(args, " -c \"%s\"",name);
        }
        else {
            ckstrncpy(args, " /c ",argslen);
            ckstrncat(args, name,argslen);
        }
        debug(F110,"os2_netopen NET_CMD args",args,0);

        memset( &startinfo, 0, sizeof(STARTUPINFO) ) ;
        startinfo.cb = sizeof(STARTUPINFO) ;

        fSuccess = CreateProcess( NULL,               /* application name */
                     cmd_line,              /* command line */
                     NULL,                  /* process security attributes */
                     NULL,                  /* primary thread security attrs */
                     TRUE,                  /* inherit handles */
                     NORMAL_PRIORITY_CLASS, /* creation flags */
                     NULL,                  /* use parent's environment */
                     NULL,                  /* use parent's current directory */
                     &startinfo,            /* startup info */
                     &procinfo ) ;          /* proc info returned */

        if ( !fSuccess )
            debug(F111,"os2_netopen unable to start process",cmd_line,GetLastError());

        CloseHandle(procinfo.hProcess);
        CloseHandle(procinfo.hThread);

        if ( !SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdOut ) ||
             !SetStdHandle( STD_INPUT_HANDLE, hSaveStdIn ) ||
             !SetStdHandle( STD_ERROR_HANDLE, hSaveStdErr ) )
        {
            debug( F101,"os2_netopen Unable to restore standard handles","",GetLastError() ) ;
            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;
            return -1;
        }

        if ( !fSuccess ) {
            CloseHandle(hChildStdoutRd);  hChildStdoutRd = NULL;
            CloseHandle(hChildStdoutWr);  hChildStdoutWr = NULL;
            CloseHandle(hChildStdinRd);  hChildStdinRd = NULL;
            CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;
            return(-1);
        }
#else /* NT */
        HFILE temp;
        char fail[256]="";
        RESULTCODES res;

        /* Save existing handles */
        rc = DosDupHandle(0,&hSaveStdIn);
        rc = DosSetFHState(hSaveStdIn, OPEN_FLAGS_NOINHERIT);
        rc = DosDupHandle(1,&hSaveStdOut);
        rc = DosSetFHState(hSaveStdOut, OPEN_FLAGS_NOINHERIT);
        rc = DosDupHandle(2,&hSaveStdErr);
        rc = DosSetFHState(hSaveStdErr, OPEN_FLAGS_NOINHERIT);

        /* Create a pipe for the child's STDOUT */
        if (rc = DosCreatePipe(&hChildStdoutRd, &hChildStdoutWr, 65535))
            debug(F100,"Stdout pipe creation failed\n","",0);

        /* Set a write handle to the pipe to be STDOUT */
        temp = 1;       /* stdout */
        rc = DosDupHandle(hChildStdoutWr, &temp);
        temp = 2;       /* stderr */
        rc = DosDupHandle(hChildStdoutWr, &temp);

        /* Create a pipe for the child's STDOUT */
        if (rc = DosCreatePipe(&hChildStdinRd, &hChildStdinWr, 4096))
            debug(F100,"Stdout pipe creation failed\n","",0);

        /* Set a write handle to the pipe to be STDOUT */
        temp = 0;       /* stdin */
        rc = DosDupHandle(hChildStdinRd, &temp);

        rc = DosSetFHState(hChildStdoutRd, OPEN_FLAGS_NOINHERIT);
        rc = DosSetFHState(hChildStdinWr, OPEN_FLAGS_NOINHERIT);

        rc = DosDupHandle(hChildStdinWr,&hChildStdinWrDup);
        rc = DosDupHandle(hChildStdoutRd,&hChildStdoutRdDup);
        rc = DosClose(hChildStdinWr);
        hChildStdinWr = -1;
        rc = DosClose(hChildStdoutRd);
        hChildStdoutRd = -1;

        cmd_line[0] = '\0' ;
        /* Now create the child process. */

        cmd_exe = getenv("SHELL");
        if ( !cmd_exe )
            cmd_exe = getenv("COMSPEC");
        if ( !cmd_exe )
            cmd_exe = "cmd.exe";
        ckstrncpy(cmd_line, cmd_exe, 256);
        args = cmd_line + strlen(cmd_line); /* don't skip zero */

        /* Look for MKS Shell, if found use -c instead of /c */
        strlwr(cmd_exe);
        debug(F110,"os2_netopen NET_CMD cmd_exe",cmd_exe,0);
        p = strstr(cmd_exe,"sh.exe");
        if ( !p )
            p = strstr(cmd_exe,"bash.exe");
        if ( !p )
            p = strstr(cmd_exe,"ash.exe");
        if ( p && (p == cmd_exe || *(p-1) == '\\' || *(p-1) == '/')) {
            sprintf(args, "-c%c%s%c",0,name,0);
        }
        else {
            sprintf(args, "/c%c%s%c",0,name,0);
        }
        debug(F110,"os2_netopen NET_CMD args",args,0);

        rc = DosExecPgm(fail, sizeof(fail), EXEC_ASYNCRESULT,
                        args, 0, &res, cmd_exe);

        /* Restore Standard Handles for this process */
        temp = 0;
        rc = DosDupHandle(hSaveStdIn,&temp);
        temp = 1;
        rc = DosDupHandle(hSaveStdOut,&temp);
        temp = 2;
        rc = DosDupHandle(hSaveStdErr,&temp);

        if (rc) {
            debug(F111,"ttruncmd failed",fail,rc);
            rc = DosClose(hChildStdinRd); hChildStdinRd = -1;
            rc = DosClose(hChildStdinWr); hChildStdinWr = -1;
            rc = DosClose(hChildStdoutRd);hChildStdoutRd = -1;
            rc = DosClose(hChildStdoutWr);hChildStdoutWr = -1;
            return 1;
        }

        pid = res.codeTerminate;            /* save the Process ID */
#endif /* NT */

        ttyfd = 999;

        /* Start reading from pipe */
        _beginthread( NetCmdReadThread,
#ifndef NT
                     0,
#endif /* NT */
                     65536, hChildStdoutRdDup );
        rc = 0;
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        extern int tt_type, max_tt;
        extern int tt_rows[], tt_cols[];
        extern struct tt_info_rec tt_info[];
        extern char *tn_term;
        int readpass(char *,char *,int);
        char * cmdline = NULL;

        if ( net_dll_netopen ) {
            cmdline = strdup(name);
            if ((rc = net_dll_netopen(cmdline,
                                     tn_term ? tn_term :
                                     (tt_type >= 0 && tt_type <= max_tt) ?
                                     tt_info[tt_type].x_name :
                                     "UNKNOWN",
                                     tt_rows[VTERM],
                                     tt_cols[VTERM],
                                     readpass)
                  ) >= 0) {
                ttyfd = 999;
                rc = 0;
            }
            else if (net_dll_errorstr) {
                printf("? %s\n",net_dll_errorstr(rc));
                rc = -1;
            }
            free(cmdline);
        }
        else
            rc = -1;
    }
#endif /* NETDLL */

   return rc;
}

/*  N E T C L O S  --  Close current network connection.  */

int
os2_netclos() {
   int rc = 0;

   if (ttyfd == -1)                     /* Was open? */
      return(rc);                       /* Wasn't. */


#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
                ttyfd = -1;
        return(ssh_clos());
    }
#endif /* SSHBUILTIN */

#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        ULONG PostCount ;
        if ( NetBiosLSN >= 0 ) {
            NCB CleanupNCB ;
            int i ;

            if ( pWorkNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS )
              NCBCancel( NetbeuiAPI, &CleanupNCB, NetBiosAdapter, pWorkNCB ) ;
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS )
            {
                NCBCancel( NetbeuiAPI, &CleanupNCB, NetBiosAdapter, pListenNCB ) ;
                DosWaitEventSem( hevNetBiosLSN, SEM_INDEFINITE_WAIT ) ;
            }
            if ( pRecvNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS )
              NCBCancel( NetbeuiAPI, &CleanupNCB, NetBiosAdapter, pRecvNCB ) ;

            for ( i=0 ; i<MAXWS ; i++ ) {
                if ( pSendNCB[i]->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS )
                  NCBCancel( NetbeuiAPI, &CleanupNCB, NetBiosAdapter, pSendNCB[i] ) ;
            }

            NCBHangup( NetbeuiAPI, pWorkNCB, NetBiosAdapter, NetBiosLSN ) ;
        }
        NetBiosLSN = 0 ;
        ckstrncpy( NetBiosRemote, "               ", NETBIOS_NAME_LEN+1 ) ;
        DosResetEventSem( hevNetBiosLSN, &PostCount ) ;
        ttyfd = -1 ;
        Dos16SemSet( pListenNCB->basic_ncb.ncb_semaphore ) ;
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
        if ( hPipe ) {
#ifdef NT
            CloseHandle( hPipe );
            hPipe = INVALID_HANDLE_VALUE;
#else /* NT */
            rc = DosClose( hPipe ) ;
            hPipe = 0 ;
#endif /* NT */
            ttyfd = -1 ;
        }
        return rc ;
    }
#endif /* NPIPE */


#ifdef TCPSOCKET
    if ( nettype == NET_TCPB )
    {
        if (!tcp_avail)
            return -1 ;
        if (!ishandle)
            rc = netclos();
        else {
#ifdef TCPIPLIB
            extern int ttibp, ttibn;
#endif /* TCPIPLIB */
            extern int tn_init;

            ttyfd = -1;                         /* Mark it as closed. */
            ishandle = 0;
            NulCheck = 0;

            debug(F100,"os2_netclos setting tn_init = 0","",0);
            tn_init = 0;                        /* Remember about telnet protocol... */
            *ipaddr = '\0';                     /* Zero the IP address string */
            tcp_incoming = 0;                   /* No longer incoming */
            sstelnet = 0;                       /* Client-side Telnet */
            /* Don't reset ttnproto so that we can remember which protocol is in use */
#ifdef TCPIPLIB
            /*
            Empty the internal buffers so they won't be used as invalid input on
            the next connect attempt (rlogin).
            */
            ttibp = 0;
            ttibn = 0;
#endif /* TCPIPLIB */
        }
        return(rc);
    }
#endif /* TCPSOCKET */

#ifdef DECNET
   if ( nettype == NET_DEC ) {
       if (ttyfd > -1) {
           if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
               memset( &lcb, 0, sizeof(struct lat_cb) ) ;
               lcb.LatFunction = STOP_SESSION;
               lcb.SessionHandle = ttyfd;
               LATENTRY(&lcb);
               rc = (lcb.LatStatus == 0) ? 0 : -1;
#endif /* OS2ONLY */
#ifdef NT
               rc = CloseSession( (DWORD) ttyfd );
               if ( rc ) {
                   debug(F101,"DECNet LAT CloseSession failed","",rc);
                   rc = -1;
               }
#endif /* NT */
           }
           else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
               rc = -1;         /* not implemented */
#endif /* OS2ONLY */
#ifdef NT
               rc = CloseSession( (DWORD) ttyfd );
               if ( rc ) {
                   debug(F101,"DECNet CTERM CloseSession failed","",rc);
                   rc = -1;
               }
#endif /* NT */
           }
       }
   }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      CloseHandle( (HANDLE) ttyfd ) ;
   }
#endif /* SUPERLAT */

#ifdef NETFILE
    if ( nettype == NET_FILE )
    {
#ifdef NT
        if (!CloseHandle( (HANDLE) ttyfd ))
            rc = -1;
#else /* NT */
        if (DosClose( ttyfd ))
            rc = -1;
#endif /* NT */
    }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
#ifdef NT
        DWORD exitcode=1;

        if (WaitForSingleObject(procinfo.hProcess, 0L) == WAIT_OBJECT_0)
            GetExitCodeProcess(procinfo.hProcess, &exitcode);
        else if (!TerminateProcess(procinfo.hProcess,exitcode)) {
            int gle = GetLastError();
            debug(F111,"net_clos NET_CMD","unable to TerminateProcess",gle);
        }
        else {
            if (WaitForSingleObject(procinfo.hProcess, 5000) == WAIT_OBJECT_0) {
                GetExitCodeProcess(procinfo.hProcess, &exitcode);
                debug(F111,"os2_netclos NET_CMD","exitcode",exitcode);
            } else {
                printf("!ERROR: Unable to terminate network command!\n");
                debug(F110,"os2_netclose NET_CMD",
                       "unable to termiate network command",0);
            }
        }

        /* Close the pipe handle so the child stops reading. */
        CloseHandle(hChildStdoutRd);    hChildStdoutRd = NULL;
        CloseHandle(hChildStdoutWr);    hChildStdoutWr = NULL;
        CloseHandle(hChildStdinRd);     hChildStdinRd = NULL;
        CloseHandle(hChildStdinWrDup);  hChildStdinWrDup = NULL;
        CloseHandle(hChildStdoutRdDup); hChildStdoutRdDup = NULL;

        CloseHandle( procinfo.hProcess ) ;
        CloseHandle( procinfo.hThread ) ;
#else /* NT */
        ULONG exitcode=STILL_ACTIVE;
        RESULTCODES res;
        PID pid2;

        res.codeTerminate = 99;
        rc = DosWaitChild(DCWA_PROCESSTREE, DCWW_NOWAIT, &res, &pid2, pid);
        if ( (rc==0) && (res.codeTerminate != 99) ) {
            exitcode = res.codeResult;
        }

        if ( exitcode == STILL_ACTIVE && pid )
        {
            DosKillProcess(DKP_PROCESSTREE,pid);
            exitcode = 128;
            if (!DosWaitChild(DCWA_PROCESSTREE, DCWW_WAIT, &res, &pid2, pid))
                exitcode = res.codeResult;
            debug(F111,"ttruncmd","res.codeResult",res.codeResult);

        }

        DosClose(hChildStdinRd); hChildStdinRd = -1;
        DosClose(hChildStdinWr); hChildStdinWr = -1;
        DosClose(hChildStdoutRd); hChildStdoutRd = -1;
        DosClose(hChildStdoutWr); hChildStdoutWr = -1;
        DosClose(hChildStdinWrDup);  hChildStdinWrDup = -1;
        DosClose(hChildStdoutRdDup); hChildStdoutRdDup = -1;
        pid = 0;
#endif /* NT */
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_netclos ) {
            rc = net_dll_netclos();
            debug(F111,"net_dll_netclos()",
                   net_dll_errorstr?net_dll_errorstr(rc):"",rc);
            rc = (rc < 0 ? -1 : 0 );
        }
        else
            rc = -1;
    }
#endif /* NETDLL */

   ttyfd = -1;                          /* Mark it as closed. */
   return(rc);
}

/*  N E T T C H K  --  Check if network up, and how many bytes can be read */
/*
  Returns number of bytes waiting, or -1 if connection has been dropped.
*/
int                                     /* Check how many bytes are ready */
os2_nettchk() {                         /* for reading from network */
   int rc = 0 ;
#ifdef NPIPE
#ifdef NT
    DWORD availdata ;
#else /* NT */
    ULONG bytesread ;
    AVAILDATA availdata ;
    ULONG PipeState ;
#endif /* NT */
#endif /* NPIPE */

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_tchk());
    }
#endif /* SSHBUILTIN */

#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        if ( NetBiosLSN > 0 )
        {
            SESSIONINFO statusinfo ;

            if ( pos < size )
              return size - pos ;

#ifndef NONBTCHK
      /* This is for testing on a machine with LAN Distance installed
         when we want to run two CKs on the same machine without connecting
         to a remote network.  NCBSessionStatus fails to return under
         these conditions.
      */
            NCBSessionStatus( NetbeuiAPI, pWorkNCB, NetBiosAdapter, NetBiosName,
                              &statusinfo, sizeof(SESSIONINFO), TRUE ) ;
            if ( pWorkNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_SUCCESSFUL &&
                 statusinfo.session_state >= NB_SESSION_STATE_SESSION_ACTIVE )
#endif /* NONBTCHK */
              return 0 ;
        }
        return -1 ;
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
        char buffer[64];

        if ( pos < size )
            return size - pos ;

#ifdef NT
        if ( PeekNamedPipe(hPipe, NULL, 0, NULL, &availdata, NULL) ) {
            return availdata;
        } else {
            DWORD error = GetLastError();
            switch ( error ) {
            case ERROR_PIPE_LISTENING:
                return 0;
            default:
                debug(F111,"os2_nettchk","named pipe",error);
                return -1;
            }
        }
#else /* NT */
        if ( !(rc = DosPeekNPipe(hPipe, buffer, sizeof(buffer),
                                  &bytesread, &availdata, &PipeState)) ) 
        {
            switch ( PipeState ) {
            case NP_STATE_DISCONNECTED:
            case NP_STATE_CLOSING:
                if ( PipeName[0] != PipeName[1] ) {
                    /* Server Mode */
                    DosDisConnectNPipe( hPipe ) ;
                    DosConnectNPipe( hPipe ) ;
                } else {
                    /* Client Mode */
                    ttclos(0) ;
                }   
                return -2 ;
                break;
            case NP_STATE_LISTENING:
                return 0 ;
                break;      
            case NP_STATE_CONNECTED:
                return availdata.cbpipe ;
                break;
            default:
                return -1 ;
            } /* switch */
        } 
        return (rc > 0 ? -rc : rc) ;
#endif /* NT */
    }
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB )
      {
      if (!tcp_avail)
         return -1 ;
      return nettchk();
      }
#endif /* TCPSOCKET */

#ifdef DECNET
   if ( nettype == NET_DEC ) {
       if ( pos < size ) {
           return size - pos ;
       } else {
           if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
               memset( &lcb, 0, sizeof(struct lat_cb) ) ;
               lcb.LatFunction = GET_STATUS ;
               lcb.SessionHandle = ttyfd;
               LATENTRY(&lcb);
               debug(F101,"os2_nettchk (DECNET) lcb.SessionStatus",
                      "",lcb.SessionStatus) ;
               debug(F101,"os2_nettchk (DECNET) lcb.LatStatus","",lcb.LatStatus) ;
               return ( lcb.LatStatus & LS_RxData ) ? 1 : 0 ;
#endif /* OS2ONLY */
#ifdef NT
               rc = 0;          /* No way to check the network status */
#endif /* NT */
           }
           else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
               rc = -1;         /* Not implemented */
#endif /* OS2ONLY */
#ifdef NT
               rc = 0;          /* No way to check the network status */
#endif /* NT */
           }
       }
   }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      ULONG BytesReturned = 0;
      static OVERLAPPED OverLapped;
      static HANDLE  thehEvent = NULL;
      static char eventString[80];
      static ReadActive = 0 ;
      int i;

      if ( !slat_avail )
         return -1 ;
      else if ( pos < size )
         return size - pos ;
      else
      {
#ifdef COMMENT
         /*
            we can't check to see how much data is waiting,
            so just check to see if the network is still up
         */

         if ( !thehEvent )
            for (i=0; i<200; i++)
            {
               thehEvent = CreateEvent (NULL, TRUE, FALSE, eventString);
               if (!GetLastError())
                  break;
            }

         OverLapped.Offset = 0;
         OverLapped.OffsetHigh = 0;
         OverLapped.hEvent = thehEvent;
                 ResetSem( thehEvent ) ;
         if (!DeviceIoControl ((HANDLE)ttyfd, IOCTL_TDI_RECEIVE,
                                NULL, 0,
                                NULL, 0,
                                &BytesReturned, NULL))
         {
            /* call failed */
            return -1;
         }
         return 0 ;
#else /* COMMENT */
          int cmd=0;
          int ReadCount=-1;
          int rc=0;
          cmd = LIOC_PEND_READS;

          /* Returns non-zero on success */
          rc = DeviceIoControl ((HANDLE)ttyfd,
                                 IOCTL_TDI_ACTION,
                                 &cmd, sizeof (cmd),
                                 &ReadCount, sizeof (ReadCount),
                                 &BytesReturned,
                                 NULL);


          debug(F111,"os2_nettchk SUPERLAT","ReadCount",ReadCount);
          debug(F111,"os2_nettchk SUPERLAT","rc",rc);
          if ( rc )
          {
              if ( ReadCount >= 0 )
                  return (ReadCount);
              else {
                  return -1 ;
              }
         }
          else {
              int gle = GetLastError();
              debug(F111,"os2_nettchk SUPERLAT","GetLastError",
                     gle);
              if ( gle == ERROR_NOT_SUPPORTED )
                  return(0);
              else
                  return(-1);
         }
#endif /* COMMENT */
      }
   }
#endif /* SUPERLAT */

#ifdef NETFILE
    if ( nettype == NET_FILE ) {
        rc = 0 ;
    }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
#ifdef NT
        if (WaitForSingleObject(procinfo.hProcess, 0L) == WAIT_OBJECT_0) {
            ttclos(0);
            return(-1);
        }
#else   /* NT */
        RESULTCODES res;
        PID pid2;
        ULONG rc;

        res.codeTerminate = 99;

        rc = DosWaitChild(DCWA_PROCESSTREE, DCWW_NOWAIT, &res, &pid2, pid);
        if ( (rc==0) && (res.codeTerminate != 99) ) {
            ttclos(0);
            return(-1);
        }

#endif  /* NT */
        else
            return(NetCmdInBuf());
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_nettchk ) {
            rc = net_dll_nettchk();
            debug(F111,"net_dll_nettchk()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc < 0 ? -1 : rc );
        }
        else
            return(-1);
    }
#endif /* NETDLL */

    return(0);
}

/* N E T X I N -- Input a block of characters from network     */
/*                handling TELNET negotiations and conversions */

static char * tngcp = NULL ;
static int    tngcl = 0 ;
static int
ckotngc(int timo) {
    if ( tngcl == 0 )
        return os2_netinc(timo) ;
    else if ( tngcl == 1 ) {
        char c = *tngcp ;
#ifdef CK_ENCRYPTION
        if ( TELOPT_U(TELOPT_ENCRYPTION) )
            ck_tn_decrypt(&c,1);
#endif /* CK_ENCRYPTION */
        *tngcp = 0;
        tngcl = 0;
        return c;
    }
    else {
        char c = *tngcp ;
#ifdef CK_ENCRYPTION
        if ( TELOPT_U(TELOPT_ENCRYPTION) )
            ck_tn_decrypt(&c,1);
#endif /* CK_ENCRYPTION */
        memmove(tngcp,tngcp+1,--tngcl);
        tngcp[tngcl] = 0;
        return c;
    }
}

int
os2_netxin(int n, CHAR * buf) {
    int len;
    int rc ;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_xin(n,buf));
    }
#endif /* SSHBUILTIN */
#ifdef TCPSOCKET
    if ( nettype == NET_TCPB ) {
        if (!tcp_avail)
            return -1 ;
        len = netxin(n,buf);
        debug(F101,"os2_netxin returns","",len);
        if ( len <= 0 )
            return(len);
#ifdef TNCODE
        if (ttnproto == NP_TELNET) {
            /* Check for Telnet negotiations */
            extern int u_binary, me_binary, tn_b_meu, tn_b_ume ;
            int i,tx;

            ckhexdump("os2_netxin buf[]",buf,len);

            for ( i=0;i<len;i++ ) {
#ifdef CK_ENCRYPTION
                if ( TELOPT_U(TELOPT_ENCRYPTION) )
                    ck_tn_decrypt(&buf[i],1);
#endif /* CK_ENCRYPTION */

                if (i == 0 && NulCheck) {
                    debug(F110,"os2_netxin","Checking for NUL or LF",0);
                    NulCheck = 0;
                    if ( buf[0] == NUL ) {
                        debug(F111,"os2_netxin","TELNET NUL FOUND",buf[0]);
                        memmove(buf, &buf[1], len-1 );
                        len-- ;
                        i--;
                        continue;
#ifdef COMMENT
                        ckhexdump("os2_netxin buf[]",buf,len);
#endif /* COMMENT */
                    }
                    else if ( buf[0] == LF ) {
                        debug(F111,"os2_netxin","TELNET LF FOUND",buf[0]);
                        debug(F111,"os2_netxin","len",len);
                    }
                    else {
                        debug(F111,"os2_netxin","TELNET ERROR - NUL NOT FOUND",buf[0]);
#ifdef COMMENT
                        ckhexdump("os2_netxin buf[]",buf,len);
#endif /* COMMENT */
                    }

                }

                if ( buf[i] == IAC ) {
                    debug(F111,"os2_netxin","TELNET IAC FOUND",i);
#ifdef COMMENT
                    ckhexdump("os2_netxin buf[]",buf,len);
#endif /* COMMENT */
                    len-- ;             /* take one character out */
                    tngcp = &buf[i];
                    tngcl = len-i;
                    if ( tngcl > 0 )
                        memmove(tngcp,tngcp+1,tngcl);
                    tngcp[tngcl] = 0;
                    if ((tx = tn_doop((CHAR)(IAC),duplex,ckotngc)) == 0) {
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        i--;
                        continue;
                    } else if (tx < 0) {        /* I/O error */
                        debug(F111,"os2_netxin","TELNET ERROR I/O",i);
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        return tx;
                    } else if (tx == 0) {       /* Not TELNET */
                        debug(F111,"os2_netxin","TELNET ERROR - NOT VALID IAC SEQUENCE",i);
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        return tx;
                    } else if (tx == 1) {       /* ECHO change */
                        duplex = 1;             /* Get next char */
                        debug(F111,"os2_netxin","TELNET DUPLEX",i);
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        i--;
                        continue;
                    } else if (tx == 2) {       /* ECHO change */
                        duplex = 0;             /* Get next char */
                        debug(F111,"os2_netxin","TELNET DUPLEX",i);
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        i--;
                        continue;
                    } else if (tx == 3) {       /* Quoted IAC */
                        memmove(tngcp+1,tngcp,tngcl);
                        *tngcp = IAC ;
                        debug(F111,"os2_netxin","TELNET IAC QUOTED",i);
                        len = i + tngcl + 1;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                    } else {
                        debug(F111,"os2_netxin","TELNET OTHER",i);
                        len = i + tngcl;
                        debug(F111,"os2_netxin","len",len);
                        tngcl = 0 ;
                        tngcp = NULL ;
                        i--;
                        continue;
                    }
                }
                else if ( (!TELOPT_U(TELOPT_BINARY) &&
                            !(TELOPT_ME(TELOPT_BINARY) && tn_b_meu)) &&
                          buf[i] == CR )
                {
                    debug(F111,"os2_netxin","TELNET CR FOUND",i);
                    debug(F111,"os2_netxin","len",len);
                    i++ ;

                    if ( i == len ) {
                        NulCheck = 1;
                        debug(F110,"os2_netxin","Check NUL or LF on next call",0);
                    }
                    else {
#ifdef CK_ENCRYPTION
                        if ( TELOPT_U(TELOPT_ENCRYPTION) )
                            ck_tn_decrypt(&buf[i],1);
#endif /* CK_ENCRYPTION */
                        if ( buf[i] == NUL ) {
                            debug(F111,"os2_netxin","TELNET NUL FOUND",i);
                            memmove(&buf[i], &buf[i+1], len-(i+1) );
                            len-- ;
                            i--;        /* so that we don't miss an IAC or CR */
#ifdef COMMENT
                            ckhexdump("os2_netxin buf[]",buf,len);
#endif /* COMMENT */
                        }
                        else if ( buf[i] == LF ) {
                            debug(F111,"os2_netxin","TELNET LF FOUND",i);
                            debug(F111,"os2_netxin","len",len);
                        }
                        else {
                            debug(F111,"os2_netxin","TELNET ERROR - NUL NOT FOUND",i);
#ifdef COMMENT
                            ckhexdump("os2_netxin buf[]",buf,len);
#endif /* COMMENT */
                        }
                    }
                }
            }
        }
        else
#endif /* TNCODE */
#ifdef CK_ENCRYPTION
            if ( TELOPT_U(TELOPT_ENCRYPTION) )
                ck_tn_decrypt(buf,len);
#endif /* CK_ENCRYPTION */
        return len;
    }
#endif /* TCPSOCKET */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        int rc = 0;
        if ( net_dll_netxin ) {
            rc = net_dll_netxin(n,buf);
            debug(F111,"net_dll_netxin()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */

    if ( pos == size ) {
        if ( (rc = os2_netinc(0)) < 0 )
            return(rc);
        pos-- ;         /* move it back one position */
    }
    len = size - pos ;
    if (len <= n) {
        memcpy(buf, &inbuf[pos], len );
        pos = size ;
        return(len);
    }
    else {
        memcpy(buf, &inbuf[pos], n ) ;
        pos += n ;
        return(n);
    }
    return(-1);
}

/*  N E T T I N C --  Input character from network */

int
os2_netinc(timo) int timo; {

    int chr = -1;
    int rc = 0 ;
#ifdef NT
    extern int ck_sleepint;
#endif /* NT */

    time_t timer, timenow ;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_inc(timo));
    }
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        if ( NetBiosLSN == -1 )
          return -1 ;
        else if ( NetBiosLSN == 0 ) {
            if (pListenNCB->basic_ncb.bncb.ncb_retcode != NB_COMMAND_IN_PROCESS)
            {
                return -1 ;
            } else {
                rc = DosWaitEventSem( hevNetBiosLSN,
                                       timo > 0 ? timo * 1000 :
                                       timo < -1 ?
                                       1000 :
                                       SEM_INDEFINITE_WAIT ) ;
                if (rc)
                    return -1 ;
            }
        }
        if ( pos < size ) {
            chr = inbuf[pos++];
        } else {
            rc = 0 ;
            if ((pRecvNCB->basic_ncb.bncb.ncb_retcode) == NB_COMMAND_IN_PROCESS)
            {
                rc = Dos16SemWait( pRecvNCB->basic_ncb.ncb_semaphore,
                                    timo > 0 ? timo * 1000 :
                                    timo < -1 ? 1000 : SEM_INDEFINITE_WAIT ) ;
            }
            if ( rc ) {
                return -1 ;
            }
            switch ( pRecvNCB->basic_ncb.bncb.ncb_retcode ) {
            case NB_COMMAND_SUCCESSFUL:
                if ( (size = pRecvNCB->basic_ncb.bncb.ncb_length) == 0 )
                  return -1 ;
                memcpy( inbuf, NetBiosRecvBuf, size ) ;
                rc = NCBReceive( NetbeuiAPI, pRecvNCB, NetBiosAdapter,
                                 NetBiosLSN, NetBiosRecvBuf,
                                 sizeof(NetBiosRecvBuf), FALSE );
                break;
            case NB_SESSION_CLOSED:
            case NB_SESSION_ENDED_ABNORMALLY:
                ttclos(0) ;
#ifdef COMMENT
                if ( ttname[0] == '*' ) {
                    os2_netopen( "*",0,0 ) ;
                    return -1 ;
                } else
#endif /* COMMENT */
                  return -3 ;
                break;
            case NB_OS_ERROR_DETECTED:
                ttclos(0) ;
                return -3 ;
                break;
            case NB_COMMAND_IN_PROCESS:
            case NB_COMMAND_TIME_OUT:
            case NB_COMMAND_CANCELLED:
            default:
                return -1 ;
            }
            pos = 0 ;
            chr = inbuf[pos++] ;
        }
        return chr ;
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
        if ( timo < -1 )
          timo = 1 ;                    /* Can't set timeout less than 1 sec */
        if ( pos < size ) {
            chr = inbuf[pos++];
        } else {
            if ( !timo ) {
                while ( !os2_nettchk() )
                  msleep(100) ;
            } else {
                timer = 0 ;
                while (1) {
                    if ( os2_nettchk() )
                      break;
                    if ( timo > 0 ) {
                        if ( !timer )
                          timer = time(0) ;
                        timenow = time(0) ;
                        if ( timenow - timer > timo )
                          return -1 ;
                    }
                    msleep(100) ;
                }
            }
#ifdef NT
            if (ReadFile( hPipe, &inbuf, sizeof(inbuf), &size, NULL)) {
                if ( !size )
                    return -1 ;
            } else {
                DWORD error = GetLastError();
                switch ( error ) {
                case ERROR_BROKEN_PIPE:
                    if ( PipeName[0] != PipeName[1] ) {
                        /* Server Mode */
                        DisconnectNamedPipe( hPipe ) ;
                        ConnectNamedPipe( hPipe, NULL ) ;
                    } else {
                        /* Client Mode */
                        ttclos(0) ;
                    }
                    return -2 ;
                    break;
                default:
                    return -1 ;
                }
            }
#else /* NT */
            rc = DosRead( hPipe, &inbuf, sizeof(inbuf), &size ) ;
            switch ( rc ) {
              case NO_ERROR:
                if ( !size )
                  return -1 ;
                break;
              case ERROR_BROKEN_PIPE:
                if ( PipeName[0] != PipeName[1] ) {
                    /* Server Mode */
                    DosDisConnectNPipe( hPipe ) ;
                    DosConnectNPipe( hPipe ) ;
                } else {
                    /* Client Mode */
                    ttclos(0) ;
                }
                return -2 ;
                break;
              default:
                return -1 ;
            }
#endif /* NT */
            pos = 0 ;
            chr = inbuf[pos++] ;
        }
        return chr ;
    }
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB ) {
#ifdef NT
        int interval, tr, tt, i, t;
        if (!tcp_avail)
          return -1 ;

        if (timo == 0) {                        /* Untimed read. */
            do {
                chr = netinc(-ck_sleepint);
                if ( NulCheck && chr > -1 ) {
                    if ( chr == NUL ) {
                        debug(F110,"os2_netinc","TELNET NUL FOUND",0);
                        chr = -1;
                    } else
                        debug(F111,"os2_netinc","TELNET NUL NOT FOUND",chr);

                    NulCheck = 0;
                }
#ifdef NTSIG
                ck_ih();
#endif /* NTSIG */
            } while (chr == -1);                /* Wait for a character. */
            return(chr);
        }

        if (timo < 0)
        {
            timo = -timo;
            t = 0 ;
        }
        else
        {
            t = timo ;
            timo *= 1000;
        }

        tr = timo % ck_sleepint;
        tt = timo / ck_sleepint;
        if ( tt )
            interval = ck_sleepint + tr / tt ;
        else
        {
            tt = 1 ;
            interval = tr ;
        }
        for (i = 0; i < tt; i++) {
            chr = netinc(-interval);
            if ( NulCheck && chr > -1 ) {
                if ( chr == NUL ) {
                    debug(F110,"os2_netinc","TELNET NUL FOUND",0);
                    chr = -1;
                } else
                    debug(F111,"os2_netinc","TELNET NUL NOT FOUND",chr);

                NulCheck = 0;
            }
#ifdef NTSIG
            ck_ih();
#endif /* NTSIG */
            if (chr != -1 )
                return(chr);
        }
        return (nettchk() == -1) ? -2 : -1;
#else /* NT */
        if (!tcp_avail)
            return -1 ;

        if ( (chr = netinc(timo)) != -1 ) {
            if ( NulCheck && chr > -1 ) {
                if ( chr == NUL ) {
                    debug(F110,"os2_netinc","TELNET NUL FOUND",0);
                    chr = -1;
                    NulCheck = 0;
                    chr = os2_netinc(timo);
                } else
                    debug(F110,"os2_netinc","TELNET NUL NOT FOUND",chr);

                NulCheck = 0;
            }
            return chr;
        } else
            return (nettchk() == -1) ? -2 : -1;
#endif /* NT */
    }
#endif /* TCPSOCKET */

#ifdef DECNET
    if ( nettype == NET_DEC ) {
        if ( pos < size )
          return inbuf[pos++];

        if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
            memset( &lcb, 0, sizeof(struct lat_cb) ) ;
            memset( inbuf, 0, sizeof(inbuf) ) ;
            lcb.LatFunction = GET_CHAR;
            lcb.SessionHandle = ttyfd;
            lcb.BufferSize = 1;
            lcb.BufferPtr = (void * _Seg16) inbuf;

            if ( timo < -1 )
                lcb.WaitTime = 10L * -timo ;
            else if ( timo > 0 )
                lcb.WaitTime = 1000L * timo ;
            else
                lcb.WaitTime = LAT_INDEFINITE_WAIT ;

            LATENTRY(&lcb);
            debug(F101,"os2_netinc (DECNET) lcb.SessionStatus","",
                   lcb.SessionStatus) ;
            debug(F101,"os2_netinc (DECNET) lcb.LatStatus","",lcb.LatStatus) ;
            if ( (lcb.SessionStatus & 0xFF) == SS_Stopped )
                return -2;

            if ( lcb.LatStatus ) {
                if ( lcb.LatStatus & LS_InvalidSize )
                    debug(F101,"os2_netinc (DECNET) LS_InvalidSize","",
                           lcb.BufferSize) ;
                return -1 ;
            }
            pos = 0;
            size = lcb.BufferSize;
            debug(F111,"os2_netinc (DECNET) lcb.BufferSize",inbuf,lcb.BufferSize) ;
            chr = inbuf[pos++];
#endif /* OS2ONLY */
#ifdef NT
            rc = ReadData((DWORD) ttyfd, inbuf, sizeof(inbuf));
            if ( rc < 0 ) {
                debug(F101,"DECNet LAT ReadData failed","",rc);
                return -2;
            }
            else if ( rc == 0 ) {
                msleep(50);
                return -1;
            }
            else {
                pos = 0;
                size = rc;
                chr = inbuf[pos++];
                inbuf[size]='\0';
                debug(F111,"DECNet LAT ReadData",inbuf,rc);
            }
#endif /* NT */
        }
        else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
            return -2;
#endif /* OS2ONLY */
#ifdef NT
            rc = ReadData((DWORD) ttyfd, inbuf, sizeof(inbuf));
            if ( rc < 0 ) {
                debug(F101,"DECNet CTERM ReadData failed","",rc);
                return -2;
            }
            else if ( rc == 0 ) {
                msleep(50);
                return -1;
            }
            else {
                pos = 0;
                size = rc;
                chr = inbuf[pos++];
                inbuf[size]='\0';
                debug(F111,"DECNet CTERM ReadData",inbuf,rc);
            }
#endif /* NT */
        }
    }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      static ULONG BytesReturned = 0;
      static OVERLAPPED OverLapped;
      static HANDLE  thehEvent = NULL;
      static char eventString[80] ;
      static ReadActive = 0 ;
      int i;
          static HANDLE fd = 0 ;

      if ( pos < size )
         return inbuf[pos++];


       if ( !thehEvent ) {
           for (i=0; i<200; i++)
           {
               sprintf(eventString,"netinc_event_%d",i);
               thehEvent = CreateEvent (NULL, TRUE, FALSE, eventString);
               if (!GetLastError())
                   break;
           }
           debug(F110,"os2_netinc SUPERLAT CreateEvent",eventString,0);
       }

      if ( ReadActive )
      {
         if ( !timo )
         {
            if ( !GetOverlappedResult ((HANDLE) fd, &OverLapped, &BytesReturned, TRUE) )
            {
               debug(F111,"os2_netinc SUPERLAT error","GetOverlappedResult(TRUE)",
                      0xFFFF & GetLastError() ) ;
               ReadActive = 0 ;
               return -2 ;
            }
            ReadActive = 0 ;
         }
         else
         {
            if ( !GetOverlappedResult ((HANDLE) fd, &OverLapped, &BytesReturned, FALSE) )
            {
               DWORD LastError = GetLastError() ;
               if ( LastError == ERROR_IO_INCOMPLETE )
               {
                  DWORD WaitRet = WaitForSingleObject( OverLapped.hEvent, timo > 0 ? timo * 1000 : -timo ) ;
                  if ( WaitRet == WAIT_OBJECT_0 )
                  {
                     if ( !GetOverlappedResult ((HANDLE) fd, &OverLapped, &BytesReturned, FALSE))
                     {
                        debug(F111,"os2_netinc SUPERLAT error",
                               "WaitRet == WAIT_OBJECT_0 &&GetOverlappedResult(TRUE)",
                               0xFFFF & GetLastError() ) ;
                        ReadActive = 0 ;
                        return -2 ;
                     }
                     ReadActive = 0 ;
                  }
                  else if ( WaitRet == WAIT_TIMEOUT )
                  {
#ifdef COMMENT
                      debug(F111,"os2_netinc SUPERLAT error",
                             "WaitRet == WAIT_TIMEOUT &&GetOverlappedResult(TRUE)",
                             0xFFFF & GetLastError() ) ;
#endif /* COMMENT */
                      return -1 ;
                  }
                  else
                  {
                     debug(F111,"os2_netinc SUPERLAT error","WaitForSingleObject()",
                            0xFFFF & GetLastError() ) ;
                     ReadActive = 0 ;
                     return -1;
                  }
               }
               else if (LastError == ERROR_OPERATION_ABORTED)
               {
                   debug(F111,"os2_netinc SUPERLAT error Operation Aborted",
                          "GetOverlappedResult(FALSE)",LastError);
                   ReadActive = 0 ;
                   return -1 ;
               }
               else
               {
                  debug(F101,"os2_netinc SUPERLAT error","GetOverlappedResult(FALSE)",
                         0xFFFF & LastError ) ;
                  ReadActive = 0 ;  /* This outstanding request failed */
                  return -2 ;
               }
            }
            ReadActive = 0 ;
         }
      }
      else
      {
         OverLapped.Offset = 0;
         OverLapped.OffsetHigh = 0;
         OverLapped.hEvent = thehEvent;
                 ResetSem( thehEvent ) ;
                 fd = (HANDLE) ttyfd ;
         if (!DeviceIoControl ((HANDLE)fd, IOCTL_TDI_RECEIVE,
                                NULL, 0,
                                inbuf, sizeof (inbuf),
                                &BytesReturned, &OverLapped))
         {
            DWORD LastError = GetLastError() ;
            if (LastError == ERROR_IO_PENDING) {
               if ( !timo )
               {
                  if ( !GetOverlappedResult ((HANDLE) fd, &OverLapped, &BytesReturned, TRUE))
                  {
                     return -1 ;
                  }
               }
               else
               {
                  DWORD WaitRet = WaitForSingleObject( OverLapped.hEvent, timo > 0 ? timo * 1000 : -timo ) ;
                  ReadActive = 1 ;
                  if ( WaitRet == WAIT_OBJECT_0 )
                  {
                     if ( !GetOverlappedResult ((HANDLE) fd, &OverLapped, &BytesReturned, FALSE))
                     {
                        debug(F111,"os2_netinc SUPERLAT error",
                               "WaitRet == WAIT_OBJECT_0 && GetOverlappedResult(FALSE)",
                               0xFFFF & GetLastError() ) ;
                        ReadActive = 0 ;
                        return -2;
                     }
                     ReadActive = 0 ;
                  }
                  else if ( WaitRet == WAIT_TIMEOUT )
                  {
#ifdef COMMENT
                      debug(F111,"os2_netinc SUPERLAT error",
                             "WaitRet == WAIT_TIMEOUT && GetOverlappedResult(FALSE)",
                             0xFFFF & GetLastError() ) ;
#endif /* COMMENT */
                      return -1 ;
                  }
                  else
                  {
                     debug(F111,"os2_netinc SUPERLAT error",
                            "WaitForSingleObject()",
                            0xFFFF & GetLastError() ) ;
                     ReadActive = 0 ;
                     return -2;
                  }
               }
            } else {
               debug(F111,"os2_netinc SUPERLAT error","GetOverlappedResult()",
                      0xFFFF & LastError ) ;
               ReadActive = 0 ; /* This Read Request failed */
               return -2;
            }
         }
      }

       if ( (LONG) BytesReturned <= 0 ) {
           debug(F101,"os2_netinc (SUPERLAT) BytesReturned","",BytesReturned) ;
           return(BytesReturned?-2:-1);
       }
       else {
           pos = 0;
           size = BytesReturned ;
#ifdef COMMENT
           if ( size < (sizeof(inbuf)-1) )
               inbuf[size] = '\0' ;
#endif /* COMMENT */
           debug(F111,"os2_netinc (SUPERLAT) BytesReturned",inbuf,BytesReturned) ;
           chr = inbuf[pos++];
       }
   }
#endif /* SUPERLAT */

#ifdef NETFILE
   if ( nettype == NET_FILE )
   {
#ifdef NT
       DWORD bytesread = 0;
       CHAR  c ;
       if ( ReadFile( (HANDLE) ttyfd, &c, 1, &bytesread, NULL ) && bytesread )
            chr = c ;
       else
       {
           return -2;
       }
#else /* NT */
       ULONG bytesread = 0 ;
       UCHAR c ;
       if ( DosRead( ttyfd, &c, 1, &bytesread ) || !bytesread )
           return -2 ;
       else
           chr = c ;
#endif /* NT */
          debug(F111,"netinc NET_FILE","chr",chr);
   }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
        CHAR  c ;

        if ( !WaitNetCmdAvailSem(timo<0?-timo:timo*1000) ) {
            if (NetCmdGetChar(&c))
                return(c);
            else
                return(-1);
        }
        else
            return(-1);
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        int rc=0;
        if ( net_dll_netinc ) {
            rc = net_dll_netinc(timo);
            debug(F111,"net_dll_netinc()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */
    return chr;
}

#ifdef CK_NETBIOS
static int NextSendNCB = 0 ;
#endif /* CK_NETBIOS */

/*  N E T T O C  --   Output character to network */
/*
  Call with character to be transmitted.
  Returns 0 if transmission was successful, or
  -1 upon i/o error, or -2 if called improperly.
*/
int
os2_nettoc(c) int c; {

#ifdef DECNET
#ifdef NT
    char ch = c;
#else /* NT */
    int i;
#endif /* NT */
#endif /* DECNET */
    int rc = -1;
    ULONG bytesWritten ;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_toc(c));
    }
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        int i = 0 ;
        int SendNCB ;
        UCHAR chr = c ;
        if ( NetBiosLSN == -1 )
          return -1 ;
        else if ( NetBiosLSN == 0 ) {
#ifdef COMMENT
/*
  This code enables us to issue a blocking send prior to the
  establishment of a connection.  The problem with this is that
  when a connection terminates and we are in Kermit Server mode,
  the Kermit Server will send a NAK in response to the timeout it
  received when the connection was lost.  This NAK will be the
  first packet received by the next Client to complete a connection,
  and all packets in the exchange will be out of sequence.

  Therefore, we don't allow packets to be issued prior to the
  establishment of a real connection.
*/
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode !=
                 NB_COMMAND_IN_PROCESS )
            {
                return -1 ;
            } else {
                rc = DosWaitEventSem( hevNetBiosLSN,
                                      SEM_INDEFINITE_WAIT ) ;
                if (rc)
                    return -1 ;
            }
#else
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode !=
                 NB_COMMAND_SUCCESSFUL )
              return -1 ;
#endif /* COMMENT */
        }

        /* If there are no Window Slot NCBs, then just return an error */
        if ( MaxWs <= 0 )
            return -1 ;

        /* Find an unused NCB, or block until one becomes free */
        SendNCB = 0 ;
        while (1) {
            if ( SendNCB == MaxWs ) {
                SendNCB = 0 ;
                msleep(250);
            }
            if (pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode
                 != NB_COMMAND_IN_PROCESS )
              break;
            SendNCB++ ;
        }

        if (pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode
             == NB_COMMAND_IN_PROCESS) {
            NCB CancelNCB ;
            rc = NCBCancel( NetbeuiAPI, &CancelNCB, NetBiosAdapter,
                            pSendNCB[SendNCB] ) ;
            Dos16SemWait( pSendNCB[SendNCB]->basic_ncb.ncb_semaphore,
                          SEM_INDEFINITE_WAIT ) ;
        }
        memcpy( NetBiosSendBuf[SendNCB], &chr, sizeof(chr) ) ;
        rc = NCBSend( NetbeuiAPI, pSendNCB[SendNCB], NetBiosAdapter,
                      NetBiosLSN, NetBiosSendBuf[SendNCB], sizeof(chr), FALSE );
#ifdef COMMENT
        /* Let's try a nonblocking Send */
        Dos16SemWait( pSendNCB[SendNCB]->basic_ncb.ncb_semaphore,
                      SEM_INDEFINITE_WAIT ) ;
#endif /* COMMENT */

        switch ( pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode ) {
        case NB_COMMAND_SUCCESSFUL:
        case NB_COMMAND_IN_PROCESS:
            return 0 ;
            break;
        case NB_SESSION_CLOSED:
        case NB_SESSION_ENDED_ABNORMALLY:
            ttclos(0) ;
#ifdef COMMENT
            if ( ttname[0] == '*' ) {
                os2_netopen( "*",0,0 ) ;
                return -1 ;
            } else
#endif /* COMMENT */
              return -2 ;
            break;
        case NB_OS_ERROR_DETECTED:
            ttclos(0) ;
            return -3 ;
            break;
        case NB_COMMAND_TIME_OUT:
        case NB_COMMAND_CANCELLED:
        default:
            return -1 ;
        }
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
        UCHAR  chr = c ;
#ifdef NT
        if (WriteFile( hPipe, &chr, sizeof(chr), &bytesWritten, NULL ))
            return 0;
        else 
            return -1;
#else /* NT */
        rc = DosWrite( hPipe, &chr, sizeof(chr), &bytesWritten ) ;
        if ( rc )
          return -1 ;
        else
          return 0 ;
#endif /* NT */
    }
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB ) {
        if (!tcp_avail)
          return -1 ;
        return nettoc((char) c);
    }
#endif /* TCPSOCKET */

#ifdef DECNET
    if ( nettype == NET_DEC ) {

        if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
            debug(F100,"os2_nettoc (DECNET) begin send char","",0);

            /* send the character */
            memset( &lcb, 0, sizeof(struct lat_cb) ) ;
            lcb.LatFunction = SEND_CHAR;
            lcb.SessionHandle = ttyfd;
            lcb.CharByte = c;

            LATENTRY(&lcb);
            debug(F101,"os2_nettoc (DECNET) lcb.SessionStatus","",
                   lcb.SessionStatus) ;
            debug(F101,"os2_nettoc (DECNET) lcb.LatStatus","",lcb.LatStatus) ;

            for (i = 0; i < 10000 && ( lcb.LatStatus & LS_CharNotSent ); i++) {
                msleep(1);              /* give up rest of current time slice */
                LATENTRY(&lcb) ;
                debug(F100,"os2_nettoc (DECNET) repeat char send","",0);
                debug(F101,"os2_nettoc (DECNET) lcb.SessionStatus",
                       "",lcb.SessionStatus) ;
                debug(F101,"os2_nettoc (DECNET) lcb.LatStatus","",lcb.LatStatus) ;
            }

            debug(F100,"os2_nettoc (DECNET) end send char","",0);

            rc = (lcb.LatStatus == LS_NoError) ? 0 : -1;
#endif /* OS2ONLY */
#ifdef NT
            debug(F111,"DECNet LAT WriteData","ch",ch);
            rc = WriteData((DWORD) ttyfd, &ch, 1);
            while ( rc == TAL_TXQUEFULL ) {
                debug(F111,"DECNet LAT WriteData","TXQUEFULL",rc);
                msleep(250);
                rc = WriteData((DWORD) ttyfd, &ch, 1);
            }
            if ( rc ) {
                debug(F101,"DECNet LAT WriteData failed","",rc);
                rc = -1;
            }
#endif /* NT */
        }
        else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
            rc = -1;
#endif /* OS2ONLY */
#ifdef NT
            debug(F111,"DECNet CTERM WriteData","ch",ch);
            rc = WriteData((DWORD) ttyfd, &ch, 1);
            while ( rc == TAL_TXQUEFULL ) {
                debug(F111,"DECNet CTERM WriteData","TXQUEFULL",rc);
                msleep(250);
                rc = WriteData((DWORD) ttyfd, &ch, 1);
            }
            if ( rc ) {
                debug(F101,"DECNet CTERM WriteData failed","",rc);
                rc = -1;
            }
#endif /* NT */
        }
    }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      static HANDLE  thehEvent=NULL;
      int     i;
      char    ch;
      static DWORD BytesWritten=0;
      DWORD LastError = 0 ;
      static OVERLAPPED OverLapped;
      static char eventString[80];
          HANDLE fd = (HANDLE) ttyfd ;

       if ( !thehEvent ) {
           for (i=0; i<200; i++)
           {
               sprintf (eventString, "nettoc_event_%d", i);
               thehEvent = CreateEvent (NULL, FALSE, FALSE, eventString);

               if (!GetLastError())
                   break;
           }
           debug(F110,"os2_nettoc SUPERLAT CreateEvent",eventString,0);
       }

      OverLapped.Offset = 0;
      OverLapped.OffsetHigh = 0;
      OverLapped.hEvent = thehEvent;
      ResetSem( thehEvent ) ;

      ch = (char) c ;
      SetLastError (0);
      if (!DeviceIoControl ((HANDLE)fd, IOCTL_TDI_SEND,
                             &ch, sizeof (ch),
                             NULL, 0, &BytesWritten,
                             isWin95() ? NULL : &OverLapped ))
      {
         LastError = GetLastError() ;
         if (LastError == ERROR_IO_PENDING) {
#ifdef COMMENT
            DWORD WaitRet = WAIT_TIMEOUT ;
            while ( WaitRet == WAIT_TIMEOUT )
            {
               WaitRet = WaitForSingleObject( OverLapped.hEvent, ck_sleepint );
#ifdef NTSIG
               ck_ih() ;
#endif /* NTSIG */
            }

            if ( ttyfd >= 0 && WaitRet == WAIT_OBJECT_0)
            {
            }
            else
            {
               debug(F101, "nettoc SUPERLAT WaitForSingleObject error","",GetLastError());
               BytesWritten = -1 ;
            }
#else /* COMMENT */
            if ( !GetOverlappedResult ((HANDLE)fd, &OverLapped, &BytesWritten, TRUE))
            {
               debug(F101, "nettoc SUPERLAT GetOverlappedResult error","",GetLastError());
               BytesWritten = -1;
            }
#endif /* COMMENT */
         } else {
            BytesWritten = -2; /* Anything else is a hard error */
         }
      }
      rc = BytesWritten == 1 ? 0 : -1 ;
   }
#endif /* SUPERLAT */

#ifdef NETFILE
    if ( nettype == NET_FILE ) {
        rc = 0;
        debug(F111,"nettoc NET_FILE","c",c);
    }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
#ifdef NT
        ULONG byteswritten=0;
        if ( !WriteFile( hChildStdinWrDup, &c, 1, &byteswritten, NULL ) )
        {
            debug(F101,"nettoc unable to write to child process","",GetLastError());
            return(-1);
        }
        else if (byteswritten == 1)
            return(0);
        else
            return(-1);
#else   /* NT */
        UINT written;
        if(DosWrite(hChildStdinWrDup,&c,1,(PVOID)&written))
            return -1 ;
        else if (written == 1)
            return(0);
        else
            return -1;
#endif /* NT */
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_nettoc ) {
            int rc=0;
            rc = net_dll_nettoc(c);
            debug(F111,"net_dll_nettoc()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */

    return rc;
}

/*  N E T X O U T --  Output block of characters to network       */
/*                    performing TELNET translations as necessary */
/*
  Call with s = pointer to string, n = length.
  Returns number of bytes actually written on success, or
  -1 on i/o error, -2 if called improperly.
  Relies on nettol() to do the actual transmission.
*/
int
os2_netxout(char *s, int n)
{
    int rc = 0 ;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_tol(s,n));
    }
#endif /* SSHBUILTIN */
    if ( nettype == NET_TCPB ) {
        if (!tcp_avail)
            return -1 ;
#ifdef TNCODE
        if ( ttnproto == NP_TELNET ) {
            /* Handle TELNET conversions */
            extern int u_binary, me_binary, tn_b_meu, tn_b_ume ;
            static char buf[12288] ;
            int len = 0, i=0 ;

            while ( i < n ) {
            for ( ; i<n && len<12277 ; i++,len++ ) {
                buf[len] = s[i] ;
                if ( s[i] == IAC ) {
                    len++ ;
                    buf[len] = IAC ;
                }
                else if ( s[i] == CR && !TELOPT_ME(TELOPT_BINARY) &&
                          !(TELOPT_U(TELOPT_BINARY) && tn_b_ume) ) {
                    len++ ;
                    buf[len] = NUL ;
                }
            }
            rc = nettol(buf,len);
            if ( rc < 0 )
                return(rc);
            else if ( rc != len )
                return(i);
            else
                len = 0 ;
            }
            rc = n ;
        }
        else
#endif /* TNCODE */
            rc = nettol(s,n);
    }
    else
        rc = os2_nettol(s,n);
    return rc;
}


/*  N E T T O L  --  Output a string of bytes to the network  */
/*
  Call with s = pointer to string, n = length.
  Returns number of bytes actually written on success, or
  -1 on i/o error, -2 if called improperly.
*/
int
os2_nettol(s,n) char *s; int n; {
    int rc=0;
#ifdef CK_NETBIOS
    int SendNCB ;
#endif /* CK_NETBIOS */
#ifdef NPIPE
    ULONG bytesWritten ;
#endif /* NPIPE */

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_tol(s,n));
    }
#endif /* SSHBUILTIN */
#ifdef TCPSOCKET
    if ( nettype == NET_TCPB ) {
        if (!tcp_avail)
          return -1 ;
        return nettol(s, n);
    }
#endif /* TCPSOCKET */

#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        if ( NetBiosLSN == -1 )
          return -3 ;
        else if ( NetBiosLSN == 0 ) {
#ifdef COMMENT
/*
  This code enables us to issue a blocking send prior to the
  establishment of a connection.  The problem with this is that
  when a connection terminates and we are in Kermit Server mode,
  the Kermit Server will send a NAK in response to the timeout it
  received when the connection was lost.  This NAK will be the
  first packet received by the next Client to complete a connection,
  and all packets in the exchange will be out of sequence.

  Therefore, we don't allow packets to be issued prior to the
  establishment of a real connection.
*/
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode !=
                 NB_COMMAND_IN_PROCESS )
            {
                return -1 ;
            } else {
                rc = DosWaitEventSem( hevNetBiosLSN,
                                      SEM_INDEFINITE_WAIT ) ;
                if (rc)
                    return -1 ;
            }
#else
            if ( pListenNCB->basic_ncb.bncb.ncb_retcode !=
                 NB_COMMAND_SUCCESSFUL )
              return -1 ;
#endif /* COMMENT */
        }

        /* If there are no Window Slot NCBs, then just return an error */
        if ( MaxWs <= 0 )
            return -1 ;

        /* Find an unused NCB, or block until one becomes free */
        SendNCB = 0 ;
        while (1) {
            if ( SendNCB == MaxWs ) {
                SendNCB = 0 ;
                msleep(250);
            }
            if (pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode
                 != NB_COMMAND_IN_PROCESS )
              break;
            SendNCB++ ;
        }

        if (pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode
             == NB_COMMAND_IN_PROCESS) {
            NCB CancelNCB ;
            rc = NCBCancel( NetbeuiAPI, &CancelNCB, NetBiosAdapter,
                            pSendNCB[SendNCB] );
            Dos16SemWait( pSendNCB[SendNCB]->basic_ncb.ncb_semaphore,
                          SEM_INDEFINITE_WAIT ) ;
        }
        memcpy( NetBiosSendBuf[SendNCB], s, n ) ;
        rc = NCBSend( NetbeuiAPI, pSendNCB[SendNCB], NetBiosAdapter,
                      NetBiosLSN, NetBiosSendBuf[SendNCB], n, FALSE ) ;
#ifdef COMMENT
        /* Lets try a non blocking Send */
        Dos16SemWait( pSendNCB[SendNCB]->basic_ncb.ncb_semaphore,
                      SEM_INDEFINITE_WAIT ) ;
#endif /* COMMENT */

        debug(F101,"NETTOL: SendNCB","",SendNCB ) ;
        debug(F101,"NETTOL: NCB_retcode","",
               pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode ) ;

        switch ( pSendNCB[SendNCB]->basic_ncb.bncb.ncb_retcode ) {
        case NB_COMMAND_SUCCESSFUL:
        case NB_COMMAND_IN_PROCESS:
            return pSendNCB[SendNCB]->basic_ncb.bncb.ncb_length ;
            break;
        case NB_SESSION_CLOSED:
        case NB_SESSION_ENDED_ABNORMALLY:
            ttclos(0) ;
#ifdef COMMENT
            if ( ttname[0] == '*' ) {
                os2_netopen( "*",0,0 ) ;
                return -1 ;
            } else
#endif /* COMMENT */
              return -2 ;
            break;
        case NB_OS_ERROR_DETECTED:
            ttclos(0) ;
            return -3 ;
            break;
        case NB_MAX_CMNDS_EXCEEDED:
        case NB_COMMAND_TIME_OUT:
        case NB_COMMAND_CANCELLED:
        default:
            return -1 ;
        }
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
#ifdef NT
        if (WriteFile( hPipe, s, n, &bytesWritten, NULL ))
            return bytesWritten;
        else
            return -1;
#else /* NT */
        rc = DosWrite( hPipe, s, n, &bytesWritten ) ;
        return ( rc ? -1 : bytesWritten ) ;
#endif /* NT */
    }
#endif /* NPIPE */

#ifdef DECNET
    if ( nettype == NET_DEC ) {
        int i ;
        if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
            for ( rc = 0; rc < n; rc++, s++ )
                if ( i = os2_nettoc(*s) ) {
                    return i ;
                }
#endif /* OS2ONLY */
#ifdef NT
            debug(F111,"DECNet LAT WriteData",s,n);
            rc = WriteData((DWORD) ttyfd, s, n);
            while ( rc == TAL_TXQUEFULL ) {
                debug(F111,"DECNet LAT WriteData","TXQUEFULL",rc);
                msleep(250);
                rc = WriteData((DWORD) ttyfd, s, n);
            }
            if ( rc ) {
                debug(F101,"DECNet LAT WriteData failed","",rc);
                rc = -1;
            }
            else {
                rc = n;
            }
#endif /* NT */
        }
        else if ( ttnproto == NP_CTERM ) {
#ifdef OS2ONLY
            rc = -1;
#endif /* OS2ONLY */
#ifdef NT
            debug(F111,"DECNet CTERM WriteData",s,n);
            rc = WriteData((DWORD) ttyfd, s, n);
            while ( rc == TAL_TXQUEFULL ) {
                debug(F111,"DECNet CTERM WriteData","TXQUEFULL",rc);
                msleep(250);
                rc = WriteData((DWORD) ttyfd, s, n);
            }
            if ( rc ) {
                debug(F101,"DECNet CTERM WriteData failed","",rc);
                rc = -1;
            }
            else {
                rc = n;
            }
#endif /* NT */
        }
    }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      static HANDLE  thehEvent=NULL;
      int     i;
      static DWORD BytesWritten=0;
      DWORD LastError=0;
      static OVERLAPPED OverLapped;
      static char eventString[80];
     HANDLE fd = (HANDLE) ttyfd ;

      if ( !thehEvent )
         for (i=0; i<200; i++)
         {
            sprintf (eventString, "nettol_event_%d", i);
            thehEvent = CreateEvent (NULL, FALSE, FALSE, eventString);

            if (!GetLastError())
               break;
         }

      OverLapped.Offset = 0;
      OverLapped.OffsetHigh = 0;
      OverLapped.hEvent = thehEvent;
      ResetSem( thehEvent ) ;

      SetLastError (0);
      if (!DeviceIoControl ((HANDLE)fd, IOCTL_TDI_SEND,
                             s, n, NULL, 0, &BytesWritten,
                             isWin95() ? NULL : &OverLapped ))
      {
         LastError = GetLastError() ;
         if (LastError == ERROR_IO_PENDING) {
#ifdef COMMENT
            DWORD WaitRet = WAIT_TIMEOUT ;
            while ( WaitRet == WAIT_TIMEOUT )
            {
#ifdef NTSIG
               ck_ih() ;
#endif /* NTSIG */
               WaitRet = WaitForSingleObject( OverLapped.hEvent, ck_sleepint );
            }

            if ( WaitRet == WAIT_OBJECT_0 && ttyfd >= 0 )
            {
               if ( !GetOverlappedResult ((HANDLE)fd, &OverLapped, &BytesWritten, TRUE))
               {
                  debug(F101, "nettol SUPERLAT GetOverlappedResult error","",GetLastError());
                  BytesWritten = -1;
               }
            }
            else
            {
               debug(F101, "nettol SUPERLAT WaitForSingleObject error","",GetLastError());
               BytesWritten = -1 ;
            }
#else /* COMMENT */
            if ( !GetOverlappedResult ((HANDLE)fd, &OverLapped, &BytesWritten, TRUE))
            {
               debug(F101, "nettol SUPERLAT GetOverlappedResult error","",GetLastError());
               BytesWritten = -1;
            }
#endif /* COMMENT */
         } else {
            BytesWritten = -2;  /* Hard Error */
         }
      }
      rc = BytesWritten ;
   }
#endif /* SUPERLAT */

#ifdef NETFILE
   if ( nettype == NET_FILE ) {
       int i;
       for ( i=0;i<n;i++ )
           nettoc(s[i]);
       rc = n;

   }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
#ifdef NT
        ULONG byteswritten=0;
        if ( !WriteFile( hChildStdinWrDup, s, n, &byteswritten, NULL ) )
        {
            debug(F101,"nettol unable to write to child process","",GetLastError());
            return(-1);
        }
        else
            return(byteswritten);
#else   /* NT */
        UINT written;
        if(DosWrite(hChildStdinWrDup, s,n,(PVOID)&written))
            return -1 ;
        else
            return written;
#endif  /* NT */
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_nettol ) {
            int rc = 0;
            rc = net_dll_nettol(s,n);
            debug(F111,"net_dll_nettol()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */

    return(rc);
}

/*  N E T F L U I  --  Flush network input buffer  */

int
os2_netflui() {

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_flui());
    }
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
      pos = size ;
      return 0 ;
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE ) {
      pos = size ;
      return 0 ;
    }
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB )
    {
        if (!tcp_avail)
          return 0 ;
        return netflui();
    }
#endif /* TCPSOCKET */

#ifdef DECNET
    if ( nettype == NET_DEC ) {
      pos = size ;
      return (0) ;
    }
#endif /* DECNET */

#ifdef SUPERLAT
   if ( nettype == NET_SLAT )
   {
      pos = size ;
      return(0);
   }
#endif /* SUPERLAT */

#ifdef NETFILE
    if ( nettype == NET_FILE ) {
       return 0;
    }
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
        return(0);
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_netflui ) {
            int rc=0;
            rc = net_dll_netflui();
            debug(F111,"net_dll_netflui()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */
    return(0);
}

/* Send network BREAK */
/*
  Returns -1 on error, 0 if nothing happens, 1 if BREAK sent successfully.
*/
int
os2_netbreak() {

    int rc = -1;

#ifdef SSHBUILTIN
    if ( nettype == NET_SSH ) {
        return(ssh_break());
    }
#endif /* SSHBUILTIN */
#ifdef CK_NETBIOS
    if ( nettype == NET_BIOS ) {
        return 0 ;
    }
#endif /* CK_NETBIOS */

#ifdef NPIPE
    if ( nettype == NET_PIPE )
      return 0 ;
#endif /* NPIPE */

#ifdef TCPSOCKET
    if ( nettype == NET_TCPB ) {
        if (!tcp_avail)
          return -1 ;
        return netbreak();
    }
#endif /* TCPSOCKET */

#ifdef DECNET
    if ( nettype == NET_DEC ) {
        if ( ttnproto == NP_LAT ) {
#ifdef OS2ONLY
            memset( &lcb, 0, sizeof(struct lat_cb) ) ;
            lcb.LatFunction = SEND_BREAK;
            lcb.SessionHandle = ttyfd;

            LATENTRY(&lcb);

            rc = (lcb.LatStatus == 0) ? 0 : -1;
#endif /* OS2ONLY */
#ifdef NT
            rc = SendBreak((DWORD) ttyfd);
            if ( rc ) {
                debug(F101,"DECNet LAT SendBreak failed","",rc);
                rc = -1;
            }
#endif /* NT */
        }
        else if ( ttnproto == NP_CTERM ) {
            /* CTERM does not support BREAK */
            return 0;
        }
    }
#endif /* DECNET */

#ifdef SUPERLAT
    if ( nettype == NET_SLAT )
    {
        return(-1);
    }
#endif /* SUPERLAT */

#ifdef NETFILE
    if ( nettype == NET_FILE )
        return(-1);
#endif /* NETFILE */

#ifdef NETCMD
    if ( nettype == NET_CMD ) {
        return(-1);
    }
#endif /* NETCMD */
#ifdef NETDLL
    if ( nettype == NET_DLL ) {
        if ( net_dll_netbreak ) {
            int rc=0;
            rc = net_dll_netbreak();
            debug(F111,"net_dll_netbreak()",
                   (net_dll_errorstr && rc<0)?net_dll_errorstr(rc):"",rc);
            return(rc<0?-1:rc);
        }
        else
            return(-1);
    }
#endif /* NETDLL */
    return rc;
}

/*
 * what follows is all dynamic link stuff to let the same executable
 * run on machines with and without networking software
 */

#ifdef __32BIT__
#define GetProc(h, n, e) DosQueryProcAddr(h, 0, n, (PFN *) e)
#define PREFIX
#else
#define GetProc(h, n, e) DosGetProcAddr(h, n, (PFN *) e)
#define PREFIX "_"
#endif



#ifdef TCPSOCKET
#ifdef CK_ENVIRONMENT
    extern char tn_env_acct[64];
    extern char tn_env_disp[64];
    extern char tn_env_job[64];
    extern char tn_env_prnt[64];
    extern char tn_env_sys[64];
    extern char uidbuf[], pwbuf[];
    char * env=NULL;
#endif /* CK_ENVIRONMENT */

int
os2_tcpipinit() {
    int rc=1 ;
#ifdef NT
    WSADATA data ;
#else /* NT */
    char dll[_MAX_PATH], fail[_MAX_PATH];
    HMODULE library;
    extern char exedir[];
    char * CKTCPIPDLL ;
    char *p, *q;
#endif /* NT */

#ifndef NT
    p = q = strdup(exedir);
    if (p) {
        while (*p) {
            if (*p == '/') *p = '\\';
            if (islower(*p)) *p = toupper(*p);
            p++;
        }
        p = q;
    } else p = exedir;

    tcp_avail = 0 ;

    if (deblog) {
        printf( "  TCP/IP support..." ) ;
        debug(F100,"TCP/IP support...","",0);
    }
    tcpname[0] = NUL;

    /* User can override DLL search order with an environment variable */

    CKTCPIPDLL = getenv( "CKTCPIPDLL" ) ;
    if ( CKTCPIPDLL ) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, CKTCPIPDLL, _MAX_PATH );
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        if (!rc) {
            if (deblog)
                printf( "CKTCPIPDLL=%s loaded...",CKTCPIPDLL) ;
            sprintf(tcpname,"CKTCPIPDLL=%s",CKTCPIPDLL) ;
            debug(F111,"CKTCPIPDLL var loaded",CKTCPIPDLL,rc);
        } else {
            debug(F110,"CKTCPIPDLL",CKTCPIPDLL,0);
            debug(F111,"CKTCPIPDLL DosLoadModule failed",fail,rc);
        }
    } else
        debug(F100,"CKTCPIPDLL not defined","",0);

#ifdef __32BIT__
/*
  Attempt to load in the following order:
  1. CKTCPIPDLL environment variable
  2. IBM 2.0
  3. FTP 1.3
  4. IBM 1.2
*/
    if (rc != 0) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, "CKO32I20.DLL", _MAX_PATH);
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        if ( rc ) {
            ckstrncpy(dll,"CKO32I20", _MAX_PATH);
            rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        }
        if (!rc) {
            if (deblog) printf( "32bit IBM TCP/IP 2.0 or higher loaded...") ;
            sprintf(tcpname,"%s = 32-bit IBM TCP/IP 2.0 or higher", dll);
            debug(F111,"32bit IBM TCP/IP 2.0 or higher loaded",dll,rc);
        } else
          debug(F111,"32bit IBM TCP/IP 2.0 or higher load failed",dll,rc);
    }
    if (rc != 0) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, "CKO32F13.DLL", _MAX_PATH);
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        if ( rc ) {
            ckstrncpy(dll, "CKO32F13", _MAX_PATH);
            rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        }
        if (!rc) {
            if (deblog) printf( "32bit FTP PC/TCP 1.3 loaded...") ;
            sprintf(tcpname,"%s = 32-bit FTP PC/TCP 1.3", dll);
            debug(F111,"32bit FTP PC/TCP 1.3 loaded",dll,rc);
        } else
          debug(F111,"32bit FTP PC/TCP 1.3 load failed",dll,rc);
    }
    if (rc != 0) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, "CKO32I12.DLL", _MAX_PATH);
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        if ( rc ) {
            ckstrncpy(dll, "CKO32I12", _MAX_PATH);
            rc = DosLoadModule(fail, sizeof(fail), "CKO32I12", &library) ;
        }
        if (!rc)
        {
            HMODULE tcpiplib ;
            PFN pfn ;
            rc = DosLoadModule(fail, sizeof(fail), "TCPIPDLL", &tcpiplib) ;
            rc = GetProc(tcpiplib,"_bsdselect", &pfn);
        }
        if (!rc) {
            if (deblog)
                printf( "32bit IBM TCP/IP 1.2 (or compatible) loaded...") ;
            sprintf(tcpname,"%s = 32-bit IBM TCP/IP 1.2 (or compatible)", dll);
            debug(F111,"32bit IBM TCP/IP 1.2 (or compatible) loaded",dll,rc);
        } else
            debug(F111,
                   "32bit IBM TCP/IP 1.2 (or compatible) load failed",dll,rc);
    }
#else
/*
  Attempt to load in the following order:
  1. CKTCPIPDLL environment variable
  2. FTP 1.3
  3. IBM 1.2
*/
    if (rc != 0) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, "CKO16F13.DLL", _MAX_PATH);
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
        if (!rc) {
            if (deblog) printf( "16bit FTP PC/TCP 1.3 loaded...") ;
            sprintf(tcpname,"%s = 16-bit FTP PC/TCP 1.3", dll);
            debug(F111,"16bit FTP PC/TCP 1.3 loaded",dll,rc);
        } else
          debug(F111,"16bit FTP PC/TCP 1.3 load failed",dll,rc);
    }
    if (rc != 0) {
        ckstrncpy(dll, p, _MAX_PATH);
        ckstrncat(dll, "CKO16I12.DLL", _MAX_PATH);
        rc = DosLoadModule(fail, sizeof(fail), dll, &library) ;
      if (!rc)
         {
         HMODULE tcpiplib ;
         PFN pfn ;
         rc = DosLoadModule(fail, sizeof(fail), "TCPIPDLL", &tcpiplib) ;
         rc = GetProc(tcpiplib,"_bsdselect", &pfn);
         }
        if (!rc) {
            if (deblog)
              printf( "16bit IBM TCP/IP 1.2 (or compatible) loaded...") ;
            sprintf(tcpname,"%s = 16-bit IBM TCP/IP 1.2", dll);
            debug(F111,"16bit IBM TCP/IP 1.2 (or compatible) loaded",dll,rc);
        } else
          debug(F111,
                "16bit IBM TCP/IP 1.2 (or compatible) load failed",dll,rc);
    }
#endif /* __32BIT__ */

    if (rc != 0) {
        if (deblog) {
            printf( "Not installed\n") ;
            debug(F101,"TCP/IP not installed - rc","",rc);
        }
        sprintf(tcpname," - ERROR: Unable to load a DLL");
        return -1;
    }
    if (GetProc(library, PREFIX "ck_sockinit", &sockinit) ||
         GetProc(library, PREFIX "ck_connect", &connect) ||
         GetProc(library, PREFIX "ck_bind", &bind) ||
         GetProc(library, PREFIX "ck_listen", &listen) ||
         GetProc(library, PREFIX "ck_accept", &accept) ||
         GetProc(library, PREFIX "ck_ioctl", &ioctl) ||
         GetProc(library, PREFIX "ck_recv", &recv) ||
         GetProc(library, PREFIX "ck_select", &select) ||
         GetProc(library, PREFIX "ck_send", &send) ||
         GetProc(library, PREFIX "ck_setsockopt", &setsockopt) ||
         GetProc(library, PREFIX "ck_getsockopt", &getsockopt) ||
         GetProc(library, PREFIX "ck_socket", &socket) ||
         GetProc(library, PREFIX "ck_soclose", &soclose) ||
         GetProc(library, PREFIX "ck_gethostbyname", &gethostbyname) ||
         GetProc(library, PREFIX "ck_getservbyname", &getservbyname) ||
         GetProc(library, PREFIX "ck_gethostbyaddr", &gethostbyaddr) ||
         GetProc(library, PREFIX "ck_getservbyport", &getservbyport) ||
         GetProc(library, PREFIX "ck_getpeername", &getpeername) ||
         GetProc(library, PREFIX "ck_getsockname", &getsockname) ||
         GetProc(library, PREFIX "ck_gethostname", &gethostname) ||
         GetProc(library, PREFIX "ck_inet_addr", &inet_addr) ||
         GetProc(library, PREFIX "ck_inet_ntoa", &inet_ntoa))  {
        if (deblog) {
            printf( "Not installed\n") ;
            debug(F100,"TCP/IP not installed","",0);
        }
        ckstrncat(tcpname," - ERROR: Missing Entry DLL Points",sizeof(tcpname));
        return -1;
    }

    if ( GetProc(library, PREFIX "ck_addsockettolist", &addsockettolist) )
    {
        debug(F100,"INETD support not available","",0);
        addsockettolist = NULL;
    }
    else
        debug(F100,"INETD support available","",0);

    rc = sockinit() ;
#else /* NT */
    winsock_version = 0;
    rc = WSAStartup( MAKEWORD( 2, 0 ), &data ) ;
    if ( rc == WSAVERNOTSUPPORTED )
    {
       debug(F110,"tcpipinit",
              "WSAStartup(2.0,&data) returns WSAVERNOTSUPPORTED",0);
       WSACleanup() ;
       rc = WSAStartup( MAKEWORD( 1, 1 ), &data ) ;
       if ( rc == WSAVERNOTSUPPORTED ) {
           debug(F110,"tcpipinit",
                  "WSAStartup(1.1,&data) returns WSAVERNOTSUPPORTED",0);
       } else
           winsock_version = 11;
    } else
        winsock_version = 20;
    if ( rc == 0 ) {
        debug(F110,"WSADATA.szDescription",data.szDescription,0);
        debug(F110,"WSADATA.szSystemStatus",data.szSystemStatus,0);
#ifdef BETADEBUG
        printf("WSADATA.szDescription: %s\n",data.szDescription);
        printf("WSADATA.szSystemStatus: %s\n",data.szSystemStatus);
#else
        if ( deblog ) {
            printf("WSADATA.szDescription: %s\n",data.szDescription);
            printf("WSADATA.szSystemStatus: %s\n",data.szSystemStatus);
        }
#endif /* BETADEBUG */
    }
#endif /* NT */

   if (rc)
   {
      tcp_avail = 0 ;
      if (deblog) {
            printf( "init failed\n" ) ;
            ckstrncat(tcpname," - ERROR: Initialization Failed",sizeof(tcpname));
            debug(F101,"TCP/IP sockinit() failed","",rc);
        }
    } else {
        tcp_avail = 1;
        if (deblog) {
            printf( "initialized\n" ) ;
            debug(F101,"TCP/IP sockinit() successful","",rc);
        }
    }
#ifdef CK_ENVIRONMENT
    if ( env = getenv("ACCT") ) {
        ckstrncpy(tn_env_acct,env,64);
    }
    else
        tn_env_acct[0] = NUL;
    if ( env = getenv("DISPLAY") ) {
        ckstrncpy(tn_env_disp,env,64);
    }
    else
        tn_env_disp[0] = NUL;
    if ( env = getenv("JOB") ) {
        ckstrncpy(tn_env_job,env,64);
    }
    else
        tn_env_job[0] = NUL;
    if ( env = getenv("PRINTER") ) {
        ckstrncpy(tn_env_prnt,env,64);
    }
    else
        tn_env_prnt[0] = NUL;
#ifdef NT
    ckstrncpy(tn_env_sys,"WIN32",64);
#else
    ckstrncpy(tn_env_sys,"OS/2",64);
#endif /* NT */
#endif /* CK_ENVIRONMENT */
    return rc;
}
#endif /* TCPSOCKET */

#ifdef DECNET
#ifdef NT
HINSTANCE hDECTAL=NULL;
int
LoadDECTAL( void )
{
    int       rc=0;

    hDECTAL = LoadLibrary("dectal") ;
    if ( !hDECTAL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL LoadLibrary failed","dectal",rc) ;
        return FALSE;
    }
    if (((FARPROC) InstalledAccess = GetProcAddress( hDECTAL, "InstalledAccess" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","InstalledAccess",rc) ;
        return FALSE;
    }
    if (((FARPROC) InquireServices = GetProcAddress( hDECTAL, "InquireServices" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","InquireServices",rc) ;
        return FALSE;
    }
    if (((FARPROC) GetNextService = GetProcAddress( hDECTAL, "GetNextService" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","GetNextService",rc) ;
        return FALSE;
    }
    if (((FARPROC) OpenSession = GetProcAddress( hDECTAL, "OpenSession" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","OpenSession",rc) ;
        return FALSE;
    }
    if (((FARPROC) CloseSession = GetProcAddress( hDECTAL, "CloseSession" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","CloseSession",rc) ;
        return FALSE;
    }
    if (((FARPROC) ReadData = GetProcAddress( hDECTAL, "ReadData" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","ReadData",rc) ;
        return FALSE;
    }
    if (((FARPROC) WriteData = GetProcAddress( hDECTAL, "WriteData" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","WriteData",rc) ;
        return FALSE;
    }
    if (((FARPROC) SendBreak = GetProcAddress( hDECTAL, "SendBreak" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","SendBreak",rc) ;
        return FALSE;
    }
    if (((FARPROC) GetDetailError = GetProcAddress( hDECTAL, "GetDetailError" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","GetDetailError",rc) ;
        return FALSE;
    }
    if (((FARPROC) DataNotify = GetProcAddress( hDECTAL, "DataNotify" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "LoadDECTAL GetProcAddress failed","DataNotify",rc) ;
        return FALSE;
    }
    return TRUE;
}
#endif /* NT */
#endif /* DECNET */

void
netinit() {
    extern int nettype;
    char fail[256];
    HMODULE library;
    extern unsigned long startflags;

   if (deblog) {
      printf( "Initializing Network Support:\n" ) ;
      debug(F100,"Initializing Network Support:","",0) ;
   }

    if ( !(startflags & 2) ) {
#ifdef CK_NETBIOS
        os2_netbiosinit() ;
#endif /* CK_NETBIOS */

#ifdef DECNET
        if (deblog) {
            printf("  DECNet support..." ) ;
            debug(F100,"DECNet support...","",0);
        }
#ifdef OS2ONLY
        if ( !DosLoadModule(fail, sizeof(fail), "LATCALLS", &library) ) {
            if ( !GetProc(library, "LATENTRY", &LATENTRY) ) {
                dnet_avail = 1;
                if (deblog) {
                    printf("OK\n") ;
                    debug(F100,"DECNet OK","",0);

                    memset( &lcb, 0, sizeof(struct lat_cb) ) ;
                    lcb.LatFunction = READ_LATINFO;
                    lcb.BufferSize = sizeof(latinfo);
                    lcb.BufferPtr = (void * _Seg16) &latinfo;

                    LATENTRY(&lcb);

                    debug(F101,"DECNet LatStatus","",lcb.LatStatus) ;
                    if (!lcb.LatStatus) {
                        debug(F101,"DECNet Major Version","",latinfo.vermjr) ;
                        debug(F101,"DECNet Minor Version","",latinfo.vermir) ;
                        debug(F101,"DECNet Max Sessions","",latinfo.NSessions) ;
                        debug(F101,"DECNet Max Buffers per Session","",
                               latinfo.NBuffers) ;
                        debug(F101,"DECNet Max Services","",latinfo.NServices) ;
                    }
                }
            } else {
                dnet_avail = 0 ;
                if (deblog) {
                    printf("Not installed\n" ) ;
                    debug(F100,"DECNet not installed","",0) ;
                }
            }
        } else {
            dnet_avail = 0 ;
            if (deblog) {
                printf("Not installed\n" ) ;
                debug(F100,"DECNet not installed","",0) ;
            }
        }
#endif /* OS2ONLY */
#ifdef NT
        if (dnet_avail = LoadDECTAL()) {
            if (deblog) {
                int version=0;

                printf("OK\n") ;
                debug(F100,"DECNet OK","",0);

                version = InstalledAccess(LAT_ACCESS);
                if ( version >= 0 ) {
                    printf("DECNet LAT available: version %d\n",version);
                    debug(F111,"DECNet LAT available","version",version);
                }
                else {
                    printf("DECNet LAT not available: error %d\n",version);
                    debug(F111,"DECNet LAT not available","error",version);
                }
                version = InstalledAccess(CTERM_ACCESS);
                if ( version >= 0 ) {
                    printf("DECNet CTERM available: version %d\n",version);
                    debug(F111,"DECNet CTERM available","version",version);
                }
                else {
                    printf("DECNet CTERM not available: error %d\n",version);
                    debug(F111,"DECNet CTERM not available","error",version);
                }
                version = InstalledAccess(TELNET_ACCESS);
                if ( version >= 0 ) {
                    printf("DECNet TELNET available: version %d\n",version);
                    debug(F111,"DECNet TELNET available","version",version);
                }
                else {
                    printf("DECNet TELNET not available: error %d\n",version);
                    debug(F111,"DECNet TELNET not available","error",version);
                }
            }
        }
        else {
            if (deblog) {
                printf("Not installed\n" ) ;
                debug(F100,"DECNet not installed","",0) ;
            }
        }
#endif /* NT */
#endif /* DECNET */

#ifdef SUPERLAT
        /* Create a file handle, first try NT way, then try Win95 way */
        ttyfd = (int) CreateFile ("\\\\.\\Lat",
                                   GENERIC_READ    | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                   NULL);

        if ((HANDLE) ttyfd == INVALID_HANDLE_VALUE)
        {
            ttyfd = (int) CreateFile ("\\\\.\\slatwin",
                                       GENERIC_READ    | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                       NULL);
            if ( (HANDLE) ttyfd == INVALID_HANDLE_VALUE )
            {
                ttyfd = -1 ;
            }
        }

        if ( (HANDLE) ttyfd != INVALID_HANDLE_VALUE )
        {
            slat_avail = 1 ;
            CloseHandle( (HANDLE) ttyfd ) ;
            ttyfd = -1 ;
        }
#endif /* SUPERLAT */
    }

#ifdef TCPSOCKET
#ifdef OS2ONLY
    if ( !(startflags & 2) )            /* OS/2 is not linked to the DLLs */
#endif /* OS2ONLY */
        os2_tcpipinit() ;
#endif /* TCPSOCKET */

    if (tcp_avail)                      /* Set the default network type */
        nettype = NET_TCPB;               /* TCP/IP first */
#ifdef DECNET
    else if (dnet_avail)                /* Then DECnet */
        nettype = NET_DEC;
#endif /* DECNET */
#ifdef SUPERLAT
    else if (slat_avail)                /* Then SuperLAT */
        nettype = NET_SLAT;
#endif /* SUPERLAT */

#ifdef COMMENT
/*
  NetBios cannot be set as a default network type even though
  we would like to because it requires that a NetBiosName be
  specified.  If the default is already in use, then we will
  not know what name to use.  Not providing a name will leave
  Kermit in an unknown state (network type NetBios without any
  means for addressing this session.)  Better never to allow
  it to be set as a default.

  All the code to implement it is shown below though just in case.
*/
    else if (netbiosAvail) {            /* Then NETBIOS */
        NCB ncb ;
        APIRET rc ;
        int x,y ;

        nettype = NET_BIOS;

        ckstrncpy(NetBiosName, (*myhost?myhost:"kermit"),NETBIOS_NAME_LEN+1);
        for (x = y; x < NETBIOS_NAME_LEN; x++)
          NetBiosName[x] = SP;
        NetBiosName[NETBIOS_NAME_LEN] = NUL;

        printf("Verifying \"%s\" is a unique NetBIOS node name ...\n",
                NetBiosName) ;
        rc = NCBAddName( NetbeuiAPI, &ncb, 0, NetBiosName ) ;
        if ( rc ) {
          printf("?Sorry, \"%s\" is already in use by another NetBIOS node.\n",
                  NetBiosName);
            for ( x=0; x < NETBIOS_NAME_LEN; x++)
               NetBiosName[x] = SP ;
        }
    }
#endif /* COMMENT */
#ifdef NPIPE
    else {                              /* Otherwise Named Pipes, */
        nettype = NET_PIPE;             /* which is always there. */
        ckstrncpy(pipename,"kermit",PIPENAML);  /* better set the pipename */
    }
#else
    else nettype = NET_NONE;
#endif /* NPIPE */

#ifdef NETCMD
    NetCmdInit();
#endif /* NETCMD */

}

int
netcleanup() {
#ifdef CK_NETBIOS
    os2_netbioscleanup() ;
#endif /* CK_NETBIOS */

#ifdef TCPSOCKET
    if ( tcpsrv_fd >= 0 )
    {
       debug(F100,"netcleanup closing tcpsrv_fd","",0);
        tcpsrv_close();
    }
#ifdef NT
   WSACleanup() ;
#endif /* NT */
#endif /* TCPSOCKET */
    return 0 ;


#ifdef NETCMD
    NetCmdCleanup();
#endif /* NETCMD */
}

#ifdef CK_NETBIOS
void
NetbiosListenThread(void * pArgList) {
    APIRET rc = 0 ;
    rc = Dos16SemWait( pListenNCB->basic_ncb.ncb_semaphore,
                       SEM_INDEFINITE_WAIT ) ;

    if ( rc ||
         pListenNCB->basic_ncb.bncb.ncb_retcode != NB_COMMAND_SUCCESSFUL )
    {
        os2_netclos() ;
    } else {
        ttyfd = NetBiosLSN = pListenNCB->basic_ncb.bncb.ncb_lsn ;
        ckstrncpy( NetBiosRemote,
                 pListenNCB->basic_ncb.bncb.ncb_callname,
                 NETBIOS_NAME_LEN+1 ) ;

#ifdef COMMENT
        if ( pRecvNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS ) {
            NCB CleanupNCB ;
            NCBCancel( NetbeuiAPI, &CleanupNCB, NetBiosAdapter, pRecvNCB ) ;
            Dos16SemWait( pRecvNCB->basic_ncb.ncb_semaphore,
                          SEM_INDEFINITE_WAIT ) ;
        }
#endif /* COMMENT */

        NCBReceive( NetbeuiAPI, pRecvNCB, NetBiosAdapter, NetBiosLSN,
                    NetBiosRecvBuf, sizeof(NetBiosRecvBuf), FALSE ) ;
    }
    DosPostEventSem( hevNetBiosLSN ) ;
    ListenThreadID = -1 ;
}
#endif /* CK_NETBIOS */

#ifdef TCPSOCKET


#ifdef OS2
#ifdef NT
#define BSDSELECT
#else  /* NT */
#define IBMSELECT
#endif /* NT */
#endif /* OS2 */

/* T C P S O C K E T _ O P E N -- Open a preexisting socket number */
int
tcpsocket_open( char * name, int * lcl, int nett, int timo )
{
    int on = 1;
    static struct servent *service, servrec;
    static struct hostent *host;
    static struct sockaddr_in saddr;
    static int saddrlen ;
    extern char myipaddr[];
    struct sockaddr_in l_addr;
    int l_slen;
#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
#endif
    extern int tcp_rdns;
#ifdef CK_SSL
    int ssl_failed = 0;
#endif /* CK_SSL */

    debug(F101,"tcpsocket_open nett","",nett);
    *ipaddr = '\0' ;

    /*
    Add support for other networks here.
    */
    if (nett != NET_TCPB)
        return(-1);     /* BSD socket support */

    if ( ttyfd == atoi(&name[1]) )
        return(0);

    netclos();                          /* Close any previous connection. */
    ckstrncpy(namecopy, name, NAMECPYL);  /* Copy the hostname. */
    ttnproto = NP_NONE;
    debug(F110,"tcpsocket_open namecopy",namecopy,0);

    /* assign the socket number to ttyfd and then fill in tcp structures */
    ttyfd = atoi( &name[1] ) ;
    debug(F111,"tcpsocket_open","ttyfd",ttyfd);

#ifdef OS2ONLY
    /* In OS/2 we must register as owning the socket */
    /* the program that gave it to us must call      */
    /* removesocketfromlist( int ) before making the */
    /* call to us                                    */
    if ( (name[0] == '!' || name[0] == '$') && addsockettolist )
        addsockettolist( ttyfd ) ;
    /* need to add this to the dll and then import it */
#endif /* OS2ONLY */

#ifdef SOL_SOCKET
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof on);

#ifdef TCP_NODELAY
    no_delay(ttyfd,tcp_nodelay) ;
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
    keepalive(ttyfd,tcp_keepalive) ;
#endif /* SO_KEEPALIVE */
#ifdef SO_DONTROUTE
    dontroute(ttyfd,tcp_dontroute);
#endif /* SO_DONTROUTE */
#ifdef SO_LINGER
    ck_linger(ttyfd,tcp_linger, tcp_linger_tmo) ;
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(ttyfd,tcp_sendbuf);
#ifdef COMMENT
    if (tcp_sendbuf < (2*MAXSP))
        sendbuf(((2*MAXSP+1460)/1460)*1460);
#endif /* COMMENT */
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(ttyfd,tcp_recvbuf);
#ifdef COMMENT
    if (tcp_recvbuf < (10*MAXRP))
        recvbuf(((10*MAXRP+1460)/1460)*1460);
#endif /* COMMENT */
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */

    ttnet = nett;                       /* TCP/IP (sockets) network */

#ifdef CK_SSL
    if (ck_ssleay_is_installed()) {
        debug(F110,"tcpsocket_open","ssl installed",0);
        if (!ssl_tn_init(tcp_incoming?SSL_SERVER:SSL_CLIENT)) {
            ssl_failed = 1;
#ifdef COMMENT
            if (bio_err!=NULL) {
                BIO_printf(bio_err,"do_ssleay_init() failed\n");
                ERR_print_errors(bio_err);
            } else {
                fflush(stderr);
                fprintf(stderr,"do_ssleay_init() failed\n");
                ERR_print_errors_fp(stderr);
            }
#endif /* COMMENT */
            
            switch (ttnproto) {
            case NP_SSL:
            case NP_TLS:
	    case NP_SSL_RAW:
	    case NP_TLS_RAW:
            case NP_SSL_TELNET:
            case NP_TLS_TELNET:
                socket_close(ttyfd);
                ttyfd = -1;
                return(-1);
            }
            /* we will continue to accept the connection */
            /* without SSL or TLS support.               */
            TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
        }
    }
#endif /* CK_SSL */

#ifdef IKSD
    if ( inserver ) {
        *lcl = 0;
    } else 
#endif /* IKSD */
    {
        if (*lcl < 0)
            *lcl = 1;           /* Local mode. */
    }

    l_slen = sizeof(l_addr);
    bzero((char *)&l_addr, l_slen);
    if (getsockname(ttyfd, (struct sockaddr *)&l_addr, &l_slen) >= 0) {
        char * s = (char *)inet_ntoa(l_addr.sin_addr);
        sprintf(myipaddr, "%s", s);
        debug(F110,"getsockname",myipaddr,0);
    }

    /* Get the name of the host we are connected to */
    saddrlen = sizeof(saddr) ;
    getpeername( ttyfd,(struct sockaddr *)&saddr,&saddrlen) ;

    if ( tcp_rdns ) {
        if ((host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL) {
            debug(F100,"tcpsocket_open gethostbyname != NULL","",0);
            host = ck_copyhostent(host);
            if ( strlen(host->h_name)+strlen(name)+6 < 80 ) {
                ckstrncpy(name,host->h_name,80);
                strncat(name,":",80-strlen(name));
                itoa( ntohs(saddr.sin_port), name+strlen(name), 10 ) ;
            } else {
                ckstrncpy(name,"host-name-too-long", sizeof(name));
            }
            ckstrncpy(ipaddr,(char *)inet_ntoa(saddr.sin_addr),20);
        }
    }
    if ( !tcp_rdns || !host ) {
        int len;
        ckstrncpy(ipaddr,(char *)inet_ntoa(saddr.sin_addr),20);
        ckstrncpy(name,ipaddr,80);
        len = strlen(name);
        strncat(name,":",80-len);
        itoa( ntohs(saddr.sin_port), name+strlen(name), 10 ) ;
    }

#ifdef CK_SECURITY
    /* Before Initialization Telnet/Rlogin Negotiations Init Kerberos */
    ck_auth_init((tcp_rdns && host && host->h_name && host->h_name[0]) ?
                host->h_name : ipaddr,
                ipaddr,
#ifdef IKSD
                inserver ? "" :
#endif /* IKSD */
                  uidbuf,
                ttyfd
                );
#endif /* CK_SECURITY */

#ifdef CK_SSL
        if (ck_ssleay_is_installed() && !ssl_failed) {
            if ( ck_ssl_incoming(ttyfd) < 0 ) {
                socket_close(ttyfd);
                ttyfd = -1;
                return(-1);
            }
        }
#endif /* CK_SSL */

    if (!quiet && !doconx)
        printf("%s connected on port %d\n",name,ntohs(saddr.sin_port));

    /* If we are IKSD assume then the service is TELNET; otherwise assume 
     * the service is TELNET but do not perform Negotiation. */
#ifdef IKSD
    ttnproto = inserver ? NP_TELNET : NP_NONE;
#else
    ttnproto = NP_NONE;
#endif
    if (tn_ini()<0)                     /* Initialize Telnet. */
        if (ttchk()<0)
            return(-1);

    return(0);                          /* Done. */
}


/*  T C P S R V _ O P E N  --  Open a TCP/IP Server connection  */
/*
  Calling conventions same as Uttopen(), except third argument is network
  type rather than modem type.  Designed to be called from within os2_netopen.
*/
void
tcpsrv_close(void)
{
    socket_close(tcpsrv_fd) ;
    tcpsrv_fd = -1 ;
    tcpsrv_port = 0 ;
    tcp_incoming = 0;               /* Not an incoming connection */
    sstelnet = 0;
}

int
tcpsrv_open(char * name, int * lcl, int nett, int timo) {
   char *p;
   int i, x;
   int on = 1;
   int ready_to_accept = 0 ;
   static struct servent *service, servrec;
   static struct hostent *host;
   static struct sockaddr_in saddr;
   static int saddrlen ;
#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
#endif
#ifdef CK_SSL
    int ssl_failed = 0;
#endif /* CK_SSL */

    debug(F110,"tcpsrv_open name",name,0);
    debug(F101,"tcpsrv_open nett","",nett);
    *ipaddr = '\0' ;

/*
  Add support for other networks here.
*/
    if (nett != NET_TCPB)
        return(-1);     /* BSD socket support */

    netclos();                          /* Close any previous connection. */
    strncpy(namecopy, name, NAMECPYL);  /* Copy the hostname. */

    /* ttnproto = NP_NONE;                      /* No protocol selected yet. - don't do this */
    debug(F110,"tcpsrv_open namecopy",namecopy,0);

   p = namecopy;                        /* Was a service requested? */
   while (*p != '\0' && *p != ':')
      p++; /* Look for colon */
   if (*p == ':') {                     /* Have a colon */
      *p++ = '\0';                      /* Get service name or number */
   } else {                             /* Otherwise use kermit */
      p = "kermit";
   }
   debug(F110,"tcpsrv_open service requested",p,0);
   if (isdigit(*p)) {                   /* Use socket number without lookup */
      service = &servrec;
      service->s_port = htons((unsigned short)atoi(p));
   } else {                             /* Otherwise lookup the service name */
      service = getservbyname(p, "tcp");
   }
   if ( !service && !strcmp( "kermit", p ) )
   {  /* use now assigned Kermit Service Port */
      service = &servrec ;
      service->s_port = htons( 1649 ) ;
   }

   if (!service) {
      fprintf(stderr, "Cannot find port for service %s\n", p);

      debug(F101,"tcpsrv_open can't get service","",errno);

      errno = 0;                        /* rather than mislead */
      return(-1);
   }

    if ( service && !strcmp("login",p) && service->s_port != htons(513) ) {
        fprintf(stderr, "  Warning: login service on port %d instead of port 513\n",
                 ntohs(service->s_port));
        fprintf(stderr, "  Edit SERVICES file if RLOGIN fails to connect.\n");
        debug(F101,"tcpsrv_open login on port","",ntohs(service->s_port));
    }

    /* if we currently have a listen active but port has changed then close */
   if ( tcpsrv_fd != -1 && tcpsrv_port != ntohs((unsigned short)service->s_port))
   {
      socket_close( tcpsrv_fd ) ;
      tcpsrv_fd = -1 ;
   }

   if ( tcpsrv_fd == -1 )
   {
      /* Set up socket structure and get host address */

      bzero((char *)&saddr, sizeof(saddr));
      debug(F100,"tcpsrv_open bzero ok","",0);
      saddr.sin_family = AF_INET ;
       if ( tcp_address )
           saddr.sin_addr.s_addr = inet_addr(tcp_address);
       else
           saddr.sin_addr.s_addr = INADDR_ANY ;

      /* Get a file descriptor for the connection. */

      saddr.sin_port = service->s_port;
      ipaddr[0] = '\0' ;

      if ((tcpsrv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
         perror("TCP socket error");
         debug(F101,"tcpsrv_open socket error","",errno);
         return (-1);
      }
      errno = 0;

      /* Specify the Port may be reused */
      setsockopt(tcpsrv_fd, SOL_SOCKET, SO_REUSEADDR,(char *) &on, sizeof on);

#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
      no_delay(tcpsrv_fd,tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
      keepalive(tcpsrv_fd,tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_DONTROUTE
       dontroute(tcpsrv_fd,tcp_dontroute);
#endif /* SO_DONTROUTE */
#ifdef SO_LINGER
      ck_linger(tcpsrv_fd,tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(tcpsrv_fd,tcp_sendbuf);
#ifdef COMMENT
    if (tcp_sendbuf < (2*MAXSP))
      sendbuf(tcpsrv_fd,((2*MAXSP+1460)/1460)*1460);
#endif /* COMMENT */
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(tcpsrv_fd,tcp_recvbuf);
#ifdef COMMENT
    if (tcp_recvbuf < (10*MAXRP))
      recvbuf(tcpsrv_fd,((10*MAXRP+1460)/1460)*1460);
#endif /* COMMENT */
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */

      /* Now bind to the socket */
      printf("\nBinding socket to port %d ...\n",
              ntohs((unsigned short)service->s_port)) ;
      if (bind(tcpsrv_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
      {
         i = errno;                     /* save error code */
         socket_close(tcpsrv_fd) ;
         tcpsrv_fd = -1 ;
         tcpsrv_port = 0 ;
         ttyfd = -1;
         errno = i;                     /* and report this error */
         debug(F101,"tcpsrv_open bind errno","",errno);
         return(-1);
      }

      printf("Listening ...\n");
      if (listen(tcpsrv_fd, 15) < 0)
      {
         i = errno;                     /* save error code */
         socket_close(tcpsrv_fd) ;
         tcpsrv_fd = -1 ;
         tcpsrv_port = 0 ;
         ttyfd = -1;
         errno = i;                     /* and report this error */
         debug(F101,"tcpsrv_open listen errno","",errno);
         return(-1);
      }
      tcpsrv_port = ntohs((unsigned short)service->s_port) ;
   }

#ifdef CK_SSL
    if (ck_ssleay_is_installed()) {
        if (!ssl_tn_init(SSL_SERVER)) {
            ssl_failed = 1;
            if (bio_err!=NULL) {
                BIO_printf(bio_err,"do_ssleay_init() failed\n");
                ERR_print_errors(bio_err);
            } else {
                fflush(stderr);
                fprintf(stderr,"do_ssleay_init() failed\n");
                ERR_print_errors_fp(stderr);
            }

            switch (ttnproto) {
            case NP_SSL:
            case NP_TLS:
	    case NP_SSL_RAW:
	    case NP_TLS_RAW:
            case NP_SSL_TELNET:
            case NP_TLS_TELNET:
                i = errno;                     /* save error code */
                socket_close(tcpsrv_fd) ;
                tcpsrv_fd = -1 ;
                tcpsrv_port = 0 ;
                ttyfd = -1;
                errno = i;                     /* and report this error */
                return(-1);
            }

            /* otherwise, we will continue to accept the connection */
            /* without SSL or TLS support.               */
            TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
            TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
        }
    }
#endif /* CK_SSL */

   printf("\nWaiting to Accept a TCP/IP connection on port %d ...\n",
           ntohs((unsigned short)service->s_port) ) ;
   saddrlen = sizeof(saddr) ;

#ifdef BSDSELECT
    tv.tv_sec  = tv.tv_usec = 0L;
    if (timo < 0) {
#ifdef NT
        tv.tv_usec = (long) -timo * 1000L;
#else  /* NT */
        tv.tv_usec = (long) -timo * 10000L;
#endif /* NT */
    } else if ( timo > 0 )
        tv.tv_sec = timo;
    else
    {
        tv.tv_usec = ck_sleepint ;
    }
    debug(F101,"tcpsrv_open BSDSELECT","",timo);
#endif /* BSDSELECT */

#ifdef NT
    WSASafeToCancel = 1 ;
#endif /* NT */

    do {
#ifdef BSDSELECT
            FD_ZERO(&rfds);
            FD_SET(tcpsrv_fd, &rfds);
            ready_to_accept = ((select(FD_SETSIZE, &rfds,
                                        NULL, NULL, &tv ) > 0) &&
                                FD_ISSET(tcpsrv_fd, &rfds)) ;
#else /* BSDSELECT */
#ifdef IBMSELECT
            ready_to_accept = ( select(&tcpsrv_fd, 1, 0, 0,
                                        timo < 0 ? -timo :
                                        (timo > 0 ? timo * 1000L :
                                          ck_sleepint )) == 1) ;
#endif /* IBMSELECT */
#endif /* BSDSELECT */
#ifdef NTSIG
         ck_ih() ;
#endif /* NTSIG */

        if ( ready_to_accept )
            break;

        msleep(100);      /* Do not hog the machine */
    } while ( timo == 0 );

    if ( ready_to_accept )
    {
       printf("Ready to Accept ...\n");
        if ((ttyfd = accept(tcpsrv_fd, (struct sockaddr *)&saddr,&saddrlen)) < 0)
        {
#ifdef NT
            WSASafeToCancel = 0 ;
#endif /* NT */
            i = errno;                  /* save error code */
            socket_close(tcpsrv_fd) ;
            ttyfd = -1;
            tcpsrv_close();
            errno = i;                  /* and report this error */
            debug(F101,"tcpsrv_open accept errno","",errno);
            return(-1);
        }
#ifdef NT
        WSASafeToCancel = 0 ;
#endif /* NT */

        setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,(char *) &on, sizeof(int));
#ifdef SO_RCVTIMEO
#ifdef SO_SNDTIMEO
        i = 30000;
        setsockopt(ttyfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &i, sizeof(int));
        setsockopt(ttyfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &i, sizeof(int));
#endif /* SO_SNDTIMEO */
#endif /* SO_RCVTIMEO */

#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
        no_delay(ttyfd,tcp_nodelay);
        debug(F101,"tcpsrv_open no_delay","",tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
        keepalive(ttyfd,tcp_keepalive);
        debug(F101,"tcpsrv_open keepalive","",tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_DONTROUTE
        dontroute(ttyfd,tcp_dontroute);
        debug(F101,"tcpsrv_open dontroute","",tcp_dontroute);
#endif /* SO_DONTROUTE */
#ifdef SO_LINGER
        ck_linger(ttyfd,tcp_linger, tcp_linger_tmo);
        debug(F101,"tcpsrv_open linger","",tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
        sendbuf(ttyfd,tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
        recvbuf(ttyfd,tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */

        ttnet = nett;                   /* TCP/IP (sockets) network */
        tcp_incoming = 1;               /* This is an incoming connection */
        sstelnet = 1;                   /* Server side telnet */

        /* See if the service is TELNET. */
        if ((x = ntohs((unsigned short)service->s_port)) ==
             getservbyname("telnet", "tcp")->s_port ) {
            if (ttnproto != NP_TCPRAW)  /* Yes and if raw port not requested */
                ttnproto = NP_TELNET;   /* Yes, set global flag. */
        }

        /* Store the ipaddress */
        ckstrncpy(ipaddr,(char *)inet_ntoa(saddr.sin_addr),20);

        if (tcp_rdns) {                 /* Get real host name */
#ifdef NT
            if ( isWin95() )
                sleep(1);
#endif /* NT */

            printf("  Reverse DNS Lookup...");
            if ((host = gethostbyaddr((char *)&saddr.sin_addr,4,PF_INET)) != NULL) {
                host = ck_copyhostent(host);
                debug(F100,"tcpsrv_open gethostbyname != NULL","",0);
                name[0]='*' ;
                ckstrncpy(&name[1],host->h_name,79);
                strncat(name,":",80-strlen(name));
                strncat(name,p,80-strlen(name));
                ckstrncpy(ipaddr,(char *)inet_ntoa(saddr.sin_addr),20);
                if (!quiet && !doconx)
                    printf("%s connected on port %s\n",host->h_name,p) ;
            }
        }
        if ( !tcp_rdns || !host ) {
            name[0] = '*';
            ckstrncpy(&name[1],ipaddr,79);
            strncat(name,":",80-strlen(name));
            itoa( ntohs(saddr.sin_port), name+strlen(name), 10 ) ;
            if (!quiet && !doconx)
                printf("%s connected on port %d\n",
                        ipaddr,ntohs(saddr.sin_port)) ;
        }

#ifdef CK_SECURITY
        /* Before Initialization Telnet/Rlogin Negotiations Init Kerberos */
        ck_auth_init((tcp_rdns && host && host->h_name && host->h_name[0]) ?
                     host->h_name : ipaddr,
                     ipaddr,
                     uidbuf,
                     ttyfd
                     );
#endif /* CK_SECURITY */


#ifdef CK_SSL
        if (ck_ssleay_is_installed() && !ssl_failed) {
            if ( ck_ssl_incoming(ttyfd) < 0 ) {
                socket_close(ttyfd);
                ttyfd = -1;
                return(-1);
            }
        }
#endif /* CK_SSL */

        if (tn_ini()<0)                 /* Start TELNET negotiations. */
            if (ttchk()<0) {
#ifdef NT
                WSASafeToCancel = 0 ;
#endif /* NT */
                i = errno;                      /* save error code */
                ttyfd = -1;
                errno = i;                      /* and report this error */
                debug(F101,"tcpsrv_open accept errno","",errno);
                return(-1);
            }
        debug(F101,"tcpsrv_open service","",x);
        if (*lcl < 0)
            *lcl = 1;           /* Local mode. */

#ifdef CK_KERBEROS
#ifdef KRB5
        if (ttnproto == NP_K5U2U ) {
            if (k5_user_to_user_server_auth() != 0) {
                socket_close(ttyfd);
                ttyfd = -1;
                return(-1);
            }
        }
#endif /* KRB5 */
#endif /* CK_KERBEROS */

#ifdef NT_TCP_OVERLAPPED
        /* Reset the Overlapped I/O structures */
        OverlappedWriteInit();
        OverlappedReadInit();
#endif /* NT_TCP_OVERLAPPED */
        return(ttyfd == -1 ? -1 : 0);           /* Done. */
    }
    else
    {
#ifdef NT
        WSASafeToCancel = 0 ;
#endif /* NT */
        i = errno;                      /* save error code */
        ttyfd = -1;
        tcpsrv_close();
        errno = i;                      /* and report this error */
        debug(F101,"tcpsrv_open accept errno","",errno);
        return(-1);
    }
}
#endif /* TCPSOCKET */

char *
os2_gethostname( void )
{
    static char * host=NULL;
#ifdef NT
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;
#endif /* NT */

    if ( host )
        return host;

#ifdef NT
    if ( !RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 0,
                KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "HostName";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            host = lpszKeyValue;
        }
        RegCloseKey( hkSubKey );
    }

    if ( host )
        return host;

    if ( !RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\VxD\\MSTCP", 0,
                KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "HostName";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            host = lpszKeyValue;
        }
        RegCloseKey( hkSubKey );
    }

    if ( host )
        return host;

    if ( !RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Control\\ComputerName\\ComputerName", 0,
                KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ComputerName";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            host = lpszKeyValue;
        }
        RegCloseKey( hkSubKey );
    }
#else /* NT */
    host = getenv("HOSTNAME");          /* (Created by TCP/IP install) */
    if (!host) host = getenv("SYSTEMNAME");     /* (Created by PATHWORKS install?) */
#endif /* NT */
    return host;
}

#ifdef CK_SOCKS
#ifdef CK_SOCKS5
!Not implemented yet
#else /* CK_SOCKS5 */
#endif /* CK_SOCKS5 */
/* SOCKS 4.2 code derived from CSTC_RELEASE 4.2 on ftp.nec.com /pub/security */

static int usesocks = FALSE ;
static int usesocksns = FALSE ;
static int socks_usens = FALSE ;
static struct in_addr socks_server, socks_ns, default_server ;
static unsigned short socks_port;
static int server_count ;
static char * username ;
enum socks_action { deny, direct, sockd } ;

char *tcp_socks_svr = NULL;             /* SOCKS Server location */
char *tcp_socks_user = NULL;            /* SOCKS Username */


/* Current SOCKS protocol version */
#define SOCKS_VERSION_4  4

/*
 **  Response commands/codes
 */
#define SOCKS_CONNECT   1
#define SOCKS_BIND      2
#define SOCKS_SUCCESS   90
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

#ifdef interface
#undef interface
#endif

struct config {
    enum socks_action action;
    int use_identd;
    Portcmp tst;
    char *userlist, *serverlist;
    struct in_addr saddr;       /* source addr */
    struct in_addr smask;       /* source mask */
    struct in_addr daddr;       /* destination addr */
    struct in_addr dmask;       /* destination mask */
    struct in_addr interface;   /* interface (for route file) */
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

static struct config *confPtr=NULL, **confNtries = NULL;
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
        hp = ck_copyhostent(hp);
        bcopy(hp->h_addr_list[0], &(addr->s_addr), hp->h_length);
        return 0;
    }
#ifdef COMMENT
    /* Not implemented on Windows and we only use AF_INET anyway */
    if ((np = getnetbyname(name)) != NULL) {
        addr->s_addr = np->n_net;
        return 0;
    }
#else /* COMMENT */
    addr->s_addr = AF_INET;
#endif /* COMMENT */
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

    if ( confNtries ) {
        int i;
        for ( i = 0 ; i < Ntries; i++)
            if (confNtries[i])
                free(confNtries[i]);
        free(confNtries);
        confNtries = NULL;
        confPtr = NULL;
        Ntries = 0;
    }

    /* find the etc directory */
    bp = getenv("ETC");
#ifdef NT
    if ( !bp ) {
        unsigned int len;
        len = sizeof(buf);

        len = GetWindowsDirectory(buf,len);
        if ( len > 0 && len < sizeof(buf)) {
            if ( !isWin95() ) {
                if ( len == 1 )
                    ckstrncat(buf,"SYSTEM32/DRIVERS/ETC",sizeof(buf));
                else
                    ckstrncat(buf,"/SYSTEM32/DRIVERS/ETC",sizeof(buf));
            }
        }
        bp = buf;
    }
#endif /* NT */
    if ( bp == NULL )
        return -1 ;

    /* check for the socks.conf or socks.cnf */
    ckstrncpy( filename, bp, sizeof(filename) ) ;
    if ( bp[strlen(bp)] != '\\' )
        ckstrncat( filename, "\\", sizeof(filename) ) ;
    ckstrncat( filename, "socks.conf", sizeof(filename) ) ;
    fd = fopen( filename, "r" ) ;

    if ( !fd ) {
        ckstrncpy( filename, bp, sizeof(filename) ) ;
        if ( bp[strlen(bp)] != '\\' )
            ckstrncat( filename, "\\", sizeof(filename) ) ;
        ckstrncat( filename, "socks.cnf", sizeof(filename) ) ;
        fd = fopen( filename, "r" ) ;
    }

    if ( !fd )
        return -1 ;

    while (fgets(buf, sizeof(buf) - 1, fd) != NULL) {
        int idx;
        linenum++;
        /*
        **  Comments start with a '#' anywhere on the line
        */
        cmdp = (char *)0;
        idx = ckindex("\n", buf, 0, 0, 1);
        if ( idx == 0 )
            bp = NULL;
        else {
            bp = &buf[idx];
            *bp = '\0';
        }
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
        }
        else {
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
        else
            confPtr->serverlist = NULL;
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
        printf("No valid SOCKS entries in file: %s\n", filename);
        return(-1);
    }
    return 0;
}

int
SOCKS_init( void )
{
    PSZ var = NULL;
    struct hostent *host ;

    /* Reset */
    socks_server.s_addr = 0 ;
    socks_ns.s_addr = 0 ;
    if ( socks_def_server ) {
        free(socks_def_server);
        socks_def_server = NULL;
    }

#ifdef CK_SOCKS_NS
    /* get the resolv info */
    res_init() ;
    server_count = _res.nscount ;
    default_server.s_addr = _res.nsaddr_list[0].sin_addr.s_addr ;
#endif /* CK_SOCKS_NS */

    /* get the SOCKS server if any */
    if ( tcp_socks_svr )
        var = tcp_socks_svr;
    else
        var = getenv("SOCKS_SERVER");
    if ( var ) {
        char * p;
        socks_def_server = strdup( var ) ;
        for (p=socks_def_server; *p && *p != ':'; p++);
        if ( *p == ':' ) {
            *p++ = '\0';
            socks_port = (unsigned short)atoi(p);
        } else {
            struct servent *service = getservbyname("socks", "tcp");
            if ( service )
                socks_port = ntohs(service->s_port);
            else
                socks_port = 1080;
        }
        host = gethostbyname( socks_def_server ) ;
        if ( host ) {
            host = ck_copyhostent(host);
            memcpy( &socks_server, host->h_addr_list[0],
                    sizeof(socks_server));
        }
        else {
            socks_server.s_addr = inet_addr(var);
        }
    }

    usesocks = socks_server.s_addr && (socks_server.s_addr != -1);

    if ( usesocks ) {
#ifdef CK_SOCKS_NS
        /* get the SOCKS Name Server if any */
        if ( var = getenv("SOCKS_NS") ) {
            host = gethostbyname( var ) ;
            if ( host ) {
                host = ck_copyhostent(host);
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
#endif /* CK_SOCKS_NS */
        SOCKS_read_config() ;

        if ( !quiet )
            printf( "SOCKS 4.2 is enabled\r\n" ) ;
    }

    /* read socks.conf file */
    return( usesocks ) ;
}

enum socks_action
SOCKS_validate( struct sockaddr_in * dest )
{
    int i = 0 ;
    enum socks_action action = sockd ;

    for ( i = 0 ; i < Ntries ; i++ ) {
        if ( ( confNtries[i]->daddr.s_addr & confNtries[i]->dmask.s_addr ) ==
             ( dest->sin_addr.s_addr & confNtries[i]->dmask.s_addr ) ) {
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
    }
    return action ;
}

int
Rconnect(int socket, struct sockaddr * name, int namelen)
{
    struct sockaddr_in addr;
    int rc ;
    CHAR localuser[UIDBUFLEN+1];
    char request[100];
    char *next ;
    int packetsize ;
    int sockret ;
    extern int ck_lcname;

    if (usesocks) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_port        = ((struct sockaddr_in *) name)->sin_port;
        addr.sin_family      = ((struct sockaddr_in *) name)->sin_family;
        addr.sin_addr.s_addr = ((struct sockaddr_in *) name)->sin_addr.s_addr;

        /* Check local cached values from SOCKS.CONF */
        switch ( SOCKS_validate( &addr ) ) {
        case deny:
            return -1 ;

        case direct:
            break;

        case sockd:
        default:
            addr.sin_family = AF_INET ;
            addr.sin_addr = socks_server ;
            addr.sin_port = htons(socks_port) ;      /* The SOCKS Service */

            /* Connect to the SOCKS Server */
            if ( connect(socket, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
                return -1 ;
            }

            /* Build the request packet */
            next = request;
            *next++ = SOCKS_VERSION_4 ;
            *next++ = SOCKS_CONNECT ;
            memcpy(next, &((struct sockaddr_in *) name)->sin_port,
                        sizeof(((struct sockaddr_in *) name)->sin_port) ) ;
            next += sizeof(((struct sockaddr_in *) name)->sin_port) ;
            memcpy(next, &((struct sockaddr_in *) name)->sin_addr,
                    sizeof(((struct sockaddr_in *) name)->sin_addr) ) ;
            next += sizeof(((struct sockaddr_in *) name)->sin_addr) ;

            localuser[0] = '\0';
            if ( tcp_socks_user )
                ckstrncpy((char*)localuser,tcp_socks_user,UIDBUFLEN);
#ifdef NT
            if ( !localuser[0] )
            {
                char localuid[UIDBUFLEN+1];
                unsigned long len = UIDBUFLEN;
                localuid[0] = '\0';
                GetUserName(localuid,&len);
                ckstrncpy((char *)localuser,localuid,UIDBUFLEN);
            }

#endif /* NT */
            if ( !localuser[0] )
            {
                char * user = getenv("USER");
                if (!user)
                    user = "";
                debug(F110,"Rconnect getenv(USER)",user,0);
                ckstrncpy((char *)localuser,user,UIDBUFLEN);
                debug(F110,"Rconnect localuser 1",localuser,0);
            }
            if ( !localuser[0] )
                strcpy((char *)localuser,"unknown");
            else if (ck_lcname) {
                cklower((char *)localuser);
                debug(F110,"Rconnect localuser 2",localuser,0);
            }

            strcpy(next,localuser) ;
            next += strlen(localuser) + 1;

            /* Send the request */
            rc = send(socket, request, next - request, 0) ;

            /* Now retrieve the response */
            packetsize = 8 ; /* VN 1, CD 1, PORT 2, IP 4 */
            next = request ;
            while ( packetsize > (next - request) ) {
                rc = recv( socket, next, packetsize - (next - request), 0 );
                if ( rc <= 0 )
                    return(-1);
                next += rc ;
            }
            ckhexdump("Rconnect response from SOCKS server",request,packetsize);
            return ( request[1] == SOCKS_SUCCESS ? 0 : -1 );
        }
    }

    /* Direct connection */
    memset(&addr, 0, sizeof(addr));
    addr.sin_port        = ((struct sockaddr_in *) name)
        ->sin_port;
    addr.sin_family      = ((struct sockaddr_in *) name)
        ->sin_family;
    addr.sin_addr.s_addr = ((struct sockaddr_in *) name)
        ->sin_addr.s_addr;
    rc = connect(socket, (struct sockaddr *) &addr, sizeof(addr));

    return rc  ;
}

int
Rbind(int socket, struct sockaddr *name, int namelen)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_port        = ((struct sockaddr_in *) name) -> sin_port;
    addr.sin_family      = ((struct sockaddr_in *) name) -> sin_family;
    addr.sin_addr.s_addr = ((struct sockaddr_in *) name) -> sin_addr.s_addr;

    return bind(socket, (struct sockaddr *) &addr, sizeof(addr));
}

int
Rlisten(int socket, int backlog)
{
    return listen( socket, backlog );
}

int
Raccept(int socket, struct sockaddr *name, int * namelen)
{
    struct sockaddr_in addr;
    int len ;
    int rc = 0 ;

    memset(&addr, 0, sizeof(addr));
    addr.sin_port        = ((struct sockaddr_in *) name) -> sin_port;
    addr.sin_family      = ((struct sockaddr_in *) name) -> sin_family;
    addr.sin_addr.s_addr = ((struct sockaddr_in *) name) -> sin_addr.s_addr;
    len = sizeof(addr) ;

    rc = accept(socket, (struct sockaddr *) &addr, &len);
    ((struct sockaddr_in *) name) -> sin_port = addr.sin_port ;
    ((struct sockaddr_in *) name) -> sin_family = addr.sin_family ;
    ((struct sockaddr_in *) name) -> sin_addr.s_addr = addr.sin_addr.s_addr;
    *namelen = len ;

    return rc ;
}

int
Rgetsockname( int socket, struct sockaddr * name, int * namelen )
{
    int len = *namelen;
    int rc = 0 ;

    rc = getsockname( socket, name, &len ) ;
    *namelen = len ;
    return rc;
}
#endif /* CK_SOCKS */

#ifdef NT_TCP_OVERLAPPED
/* These are Win32 specific functions which are used in place of #defines */
/* The hope is to add Overlapped I/O support to these calls               */

_PROTOTYP( int OverlappedWrite, (int, char *, int));
_PROTOTYP( int OverlappedRead,  (int, char *, int, int));
int ionoblock = 0;
extern int owwait;

int
socket_read(int f, char * s, int n)
{
    extern char ttibuf[];
    extern int  ttibp, ttibn;
    int rc = 0;

    if ( owwait )
        return(recv(f,s,n,0));

    RequestTCPIPMutex(SEM_INDEFINITE_WAIT);
    if ( ttibn )
    {
        if ( n < ttibn ) {
            memcpy(s,&ttibuf[ttibp],n);
            ttibp += n;
            ttibn -= n;
            rc = n;
        }
        else {
            rc = ttibn;
            memcpy(s,&ttibuf[ttibp],ttibn);
            ttibp = ttibn = 0;
        }
        ReleaseTCPIPMutex();
        return(rc);
    }

    /* The real test here should be if n is small AND there are no overlapped */
    /* Reads in progress OR if we are using a non-Microsoft Winsock stack     */
    if ( n < 32768 ) {
        rc = OverlappedRead( 0, ttibuf, 32768, ionoblock ? 10 : -1 );
        if ( rc > 0 ) {
            ttibp = 0;
            ttibn = rc;
            rc = socket_read(f,s,n);
            ReleaseTCPIPMutex();
            return rc;
        }
        else if ( rc == 0 || rc == -1 ) {
            ReleaseTCPIPMutex();
            return(-1);
        } else if ( rc <= -2 ) {        /* Connection failed */
            ReleaseTCPIPMutex();
            return(0);
        }
    }
    else {
        rc = OverlappedRead( 0, s, n, ionoblock ? 10 : -1 );
        if ( rc <= -2 ) {        /* Connection failed */
            ReleaseTCPIPMutex();
            return(0);
        } else if ( rc == 0 || rc == -1 ) {
            ReleaseTCPIPMutex();
            return(-1);
        } else {
            ReleaseTCPIPMutex();
            return(rc);
        }
    }
    ReleaseTCPIPMutex();
    return(0);
}

int
socket_write(int f, char * s, int n)
{
    int rc;

    if ( owwait )
        return(send(f,s,n,0));

    rc = OverlappedWrite( 0, s, n );
    if ( rc < 0 )
        return(-1);
    else
        return(rc);
}

int
socket_recv(int f, char * s, int n, int o)
{
    return(recv(f,s,n,o));
}

int
socket_send(int f, char * s, int n, int o)
{
    return(send(f,s,n,o));
}

#endif /* NT_TCP_OVERLAPPED */
#endif /* NETCONN */
