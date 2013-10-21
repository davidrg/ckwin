/* C K O C O 5 . C */

/*
  Author: Frank da Cruz  (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
            Columbia University Academic Information Systems, New York City.
          Jeffrey E Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef NT
#include "ckcdeb.h"

#define INCL_WIN
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#define INCL_DOSMEMMGR
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */

#include "ckocon.h"
#include <stdio.h>
#include <stdlib.h>

extern int tnlm, tn_nlm;        /* Terminal newline mode, ditto for TELNET */
extern videobuffer vscrn[];
extern enum markmodes markmodeflag ;
extern bool xprintff, printon ;
extern HAB hab ;

HMUX hmuxClipbrdSrv = 0 ;
HEV hevClipbrdGet  = 0 ;
HEV hevClipbrdPut = 0 ;
HEV hevClipbrdData = 0 ;
HEV hevClipbrdDone = 0 ;

APIRET
OpenClipboardServer( void )
{
    APIRET rc = 0 ;

    rc = DosOpenMutexSem( "\\SEM32\\CKERMIT\\CLIPBRD\\MUX", &hmuxClipbrdSrv ) ;
    if ( rc ) {
        return rc ;
        }

    rc = DosOpenEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\GET", &hevClipbrdGet ) ;
    if ( rc ) {
        DosCloseMutexSem( hmuxClipbrdSrv ) ;
        return rc ;
        }

    rc = DosOpenEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\DATA", &hevClipbrdData ) ;
    if ( rc ) {
        DosCloseMutexSem( hmuxClipbrdSrv ) ;
        DosCloseEventSem( hevClipbrdGet ) ;
        return rc ;
        }

    rc = DosOpenEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\DONE", &hevClipbrdDone ) ;
    if ( rc ) {
        DosCloseMutexSem( hmuxClipbrdSrv ) ;
        DosCloseEventSem( hevClipbrdGet ) ;
        DosCloseEventSem( hevClipbrdData ) ;
        return rc ;
        }

    rc = DosOpenEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\PUT", &hevClipbrdPut ) ;
    if ( rc ) {
        DosCloseMutexSem( hmuxClipbrdSrv ) ;
        DosCloseEventSem( hevClipbrdGet ) ;
        DosCloseEventSem( hevClipbrdData ) ;
        DosCloseEventSem( hevClipbrdDone ) ;
        return rc ;
        }
    return rc ;
}

APIRET
CloseClipboardServer( void )
{
    if ( hmuxClipbrdSrv )
        DosCloseMutexSem( hmuxClipbrdSrv ) ;
    if ( hevClipbrdGet )
        DosCloseEventSem( hevClipbrdGet ) ;
    if ( hevClipbrdPut )
        DosCloseEventSem( hevClipbrdPut ) ;
    if ( hevClipbrdData )
        DosCloseEventSem( hevClipbrdData ) ;
    if ( hevClipbrdDone )
        DosCloseEventSem( hevClipbrdDone ) ;

    hmuxClipbrdSrv = 0 ;
    hevClipbrdGet  = 0 ;
    hevClipbrdData = 0 ;
    hevClipbrdDone = 0 ;
    return 0 ;
}

void
SetPtrWait( void )
{
    WinSetPointer( HWND_DESKTOP,
        WinQuerySysPointer( HWND_DESKTOP, FALSE, SPTR_WAIT ) ) ;
}

void
SetPtrArrow( void )
{
    WinSetPointer( HWND_DESKTOP,
        WinQuerySysPointer( HWND_DESKTOP, FALSE, SPTR_ARROW ) ) ;
}

BOOL
PutTextToClipboardServer( PCHAR pString )
{
    ULONG postcount = 0 ;
    PCHAR pSharedMem = NULL ;
    APIRET rc = 0 ;

    if ( !hmuxClipbrdSrv && OpenClipboardServer() ) {
        debug( F100,"CLIP Server not available","",0);
        return FALSE ;
        }

    if ( rc = DosRequestMutexSem( hmuxClipbrdSrv, 1000 ) ) {
        debug( F101, "CLIP Request Mutex failed rc","",rc ) ;
        CloseClipboardServer() ;
        return FALSE ;
        }

    rc = DosGetNamedSharedMem( (PPVOID) &pSharedMem,
        "\\SHAREMEM\\CKERMIT\\CLIPBRD\\DATA",  PAG_WRITE ) ;
    debug( F101, "CLIP Get Named Shared RAM rc","",rc ) ;

    /* We now have exclusive access to the named shared memory */

    if ( !rc && pSharedMem ) {
        strncpy( pSharedMem, pString, 65535 ) ;
        pSharedMem[65534] = '\0' ;
        }

    /* Now signal the Clipboard Server that we have something for it to do */
    rc = DosPostEventSem( hevClipbrdPut ) ;
    debug( F101, "CLIP Post Put Sem rc","",rc ) ;

    /* Now wait for it to tell us it is done */
    rc = DosWaitEventSem( hevClipbrdDone, SEM_INDEFINITE_WAIT ) ;
    debug( F101, "CLIP Wait Done Sem rc","",rc ) ;
    rc = DosResetEventSem( hevClipbrdDone, &postcount ) ;

    rc = DosReleaseMutexSem( hmuxClipbrdSrv ) ;
    return TRUE ;
}

PCHAR
GetTextFromClipboardServer( void )
{
    ULONG postcount = 0 ;
    PCHAR pSharedMem = NULL ;
    PCHAR pString = NULL ;
    APIRET rc = 0 ;

    if ( !hmuxClipbrdSrv && OpenClipboardServer() ) {
        debug( F100,"CLIP Server not available","",0);
        return NULL ;
        }

    if ( rc = DosRequestMutexSem( hmuxClipbrdSrv, 1000 ) ) {
        debug( F101, "CLIP Request Mutex failed rc","",rc ) ;
        CloseClipboardServer() ;
        return NULL ;
        }

    rc = DosPostEventSem( hevClipbrdGet ) ;
    debug( F101, "CLIP Post Get Sem rc","",rc ) ;

    if ( rc = DosWaitEventSem( hevClipbrdData, 1000 ) ) {
        debug( F101, "CLIP Wait Data Sem failed rc","",0 ) ;
        rc = DosReleaseMutexSem( hmuxClipbrdSrv ) ;
        CloseClipboardServer();
        return NULL ;
        }

    rc = DosResetEventSem( hevClipbrdData, &postcount ) ;
    debug( F101, "CLIP Reset Data Sem rc","",rc ) ;

    rc = DosGetNamedSharedMem( (PPVOID) &pSharedMem,
        "\\SHAREMEM\\CKERMIT\\CLIPBRD\\DATA",  PAG_READ ) ;
    debug( F101, "CLIP Get Named Shared RAM rc","",rc ) ;

    if ( !rc && pSharedMem ) {
        pString = strdup( pSharedMem ) ;
        DosFreeMem( pSharedMem ) ;
        }

    rc = DosPostEventSem( hevClipbrdDone ) ;
    debug( F101, "CLIP Post Done Sem rc","",rc ) ;
    rc = DosReleaseMutexSem( hmuxClipbrdSrv ) ;
    return pString ;
}
#endif /* NT */


