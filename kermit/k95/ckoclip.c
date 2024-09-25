/*
  Authors: Frank da Cruz  (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
             Columbia University Academic Information Systems, New York City.
           Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#define INCL_WIN
#define INCL_WINMLE
#define INCL_GPICONTROL
#define INCL_WINCLIPBOARD
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMEMMGR
#include <os2.h>
#undef COMMENT
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "ckoclip.h"

/* definitions & macros */
#define PAPA( x)        WinQueryWindow( x, QW_PARENT)
#define MENU( x)        WinWindowFromID( x, FID_MENU)
#define HAB( x)         WinQueryAnchorBlock( x)

#define FCF_WPS     FCF_HIDEMAX | FCF_STANDARD & ~FCF_MINMAX

#define CT_MLE      1000
#define MAXCHARS    65535

#define WM_PASSPROC     WM_USER + 0
#define WM_GETDATA      WM_USER + 1
#define WM_PUTDATA      WM_USER + 2

/* function prototypes */
int main( void) ;
MRESULT EXPENTRY ClientWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;

/* globals */
HAB hab ;                              /* Handle to the Anchor Block */
HWND hwndFrame,                        /* Handle to the Frame Window */
     hwndClient ;                      /* Handle to the Client Window ! */
int  killthread = 0 ;                  /* Signal to Server thread to die */
HMUX hmuxClipbrdSrv = 0 ;              /* Only allow one client at a time */
HEV hevClipbrdGet  = 0 ;               /* Signal to get clipboard data */
HEV hevClipbrdPut  = 0 ;               /* Signal to put clipboard data */
HEV hevClipbrdReady = 0 ;              /* The data is ready for client */
HEV hevClipbrdData = 0 ;               /* Let the client know its ready */
HEV hevClipbrdDone = 0 ;               /* The Client is done; reset */
PSZ pSharedMem =0;                     /* The shared memory buffer */


/* This function is the Clipboard Server thread.  It handles all
   communication with our clients via Named Shared Semaphores.
   We can't access the clipboard from here, so we post a message
   to our message queue thread when we need the clipboard data.
   The Window Procedure then gets the data and signals us via a
   private semaphore that it the data is ready.  At that time
   we signal our client, and wait for the client to complete.
   If the client, doesn't release us we will wait forever.  But better
   that then allow two clients to possibly crash into each other
*/
void
clipsrv( void * param )
{
  ULONG   clipsize =0;
  PBYTE * clipdata =0;
  ULONG postcount =0;
  APIRET rc = 0 ;
  HMUX    hmuxWaitForWork = 0 ;
  SEMRECORD work_rec[2] ;
  ULONG jobtype ;
#define GETTEXT 0L
#define PUTTEXT 1L

  rc = DosAllocSharedMem( (PPVOID) &pSharedMem,
          "\\SHAREMEM\\CKERMIT\\CLIPBRD\\DATA",
          MAXCHARS, PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_TILE ) ;

  work_rec[0].hsemCur = (HSEM) hevClipbrdGet ;
  work_rec[0].ulUser  = GETTEXT ;
  work_rec[1].hsemCur = (HSEM) hevClipbrdPut ;
  work_rec[1].ulUser  = PUTTEXT ;

  rc = DosCreateMuxWaitSem( NULL, &hmuxWaitForWork, 2, work_rec,
        DCMW_WAIT_ANY | DC_SEM_SHARED ) ;

  while ( !killthread ) {
    pSharedMem[0] = '\0' ;

    /* Wait for something to do */
    rc = DosWaitMuxWaitSem( hmuxWaitForWork, SEM_INDEFINITE_WAIT, &jobtype ) ;

    switch ( jobtype ) {
        case GETTEXT:
            rc = DosResetEventSem( hevClipbrdGet, &postcount ) ;

            WinPostMsg( hwndClient, WM_GETDATA, 0, 0 );

            rc = DosWaitEventSem( hevClipbrdReady, SEM_INDEFINITE_WAIT );
            rc = DosResetEventSem( hevClipbrdReady, &postcount );

            /* Now let the requester know its ready */

            rc = DosPostEventSem( hevClipbrdData ) ;

            /* Now wait for the requester to finish using the memory */

            rc = DosWaitEventSem( hevClipbrdDone, SEM_INDEFINITE_WAIT ) ;
            rc = DosResetEventSem( hevClipbrdDone, &postcount ) ;
            break;

        case PUTTEXT:
            rc = DosResetEventSem( hevClipbrdPut, &postcount ) ;

            /* There is text in the shared memory to be sent to the Clipboard */
            /* so let the Window Message thread know                          */

            WinPostMsg( hwndClient, WM_PUTDATA, 0 , 0 ) ;

            /* Wait for the Window Message thread to finish the Copy */
            rc = DosWaitEventSem( hevClipbrdReady, SEM_INDEFINITE_WAIT );
            rc = DosResetEventSem( hevClipbrdReady, &postcount );

            /* Let the client know we are Done */
            rc = DosPostEventSem( hevClipbrdDone ) ;
            break;
        }

    }
    rc = DosCloseMuxWaitSem( hmuxWaitForWork ) ;
    rc = DosFreeMem( pSharedMem ) ;
}

int
initserver( void )
{
    APIRET rc ;

    if ( rc = DosCreateMutexSem( "\\SEM32\\CKERMIT\\CLIPBRD\\MUX",
        &hmuxClipbrdSrv, DC_SEM_SHARED, TRUE ) )
        return 1 ;

    if ( rc = DosCreateEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\GET", &hevClipbrdGet,
        DC_SEM_SHARED, FALSE ) ) {
            DosCloseMutexSem( hmuxClipbrdSrv ) ;
            return 1 ;
            }
    if ( rc = DosCreateEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\DATA", &hevClipbrdData,
        DC_SEM_SHARED, FALSE ) ) {
            DosCloseMutexSem( hmuxClipbrdSrv ) ;
            DosCloseEventSem( hevClipbrdGet ) ;
            return 1 ;
            }
    if ( rc = DosCreateEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\DONE", &hevClipbrdDone,
        DC_SEM_SHARED, FALSE ) ) {
            DosCloseMutexSem( hmuxClipbrdSrv ) ;
            DosCloseEventSem( hevClipbrdGet ) ;
            DosCloseEventSem( hevClipbrdData ) ;
            return 1 ;
            }

    if ( rc = DosCreateEventSem( NULL, &hevClipbrdReady, 0, FALSE ) ) {
            DosCloseMutexSem( hmuxClipbrdSrv ) ;
            DosCloseEventSem( hevClipbrdGet ) ;
            DosCloseEventSem( hevClipbrdData ) ;
            DosCloseEventSem( hevClipbrdDone ) ;
            return 1 ;
            }
    if ( rc = DosCreateEventSem( "\\SEM32\\CKERMIT\\CLIPBRD\\PUT", &hevClipbrdPut,
        DC_SEM_SHARED, FALSE ) ) {
            DosCloseMutexSem( hmuxClipbrdSrv ) ;
            DosCloseEventSem( hevClipbrdGet ) ;
            DosCloseEventSem( hevClipbrdData ) ;
            DosCloseEventSem( hevClipbrdDone ) ;
            DosCloseEventSem( hevClipbrdReady ) ;
            return 1 ;
            }
    return 0 ;
}

void
killserver( void )
{
    DosRequestMutexSem( hmuxClipbrdSrv, SEM_INDEFINITE_WAIT ) ;
    killthread = 1 ;
    DosCloseMutexSem( hmuxClipbrdSrv ) ;
    DosCloseEventSem( hevClipbrdGet ) ;
    DosCloseEventSem( hevClipbrdData ) ;
    DosCloseEventSem( hevClipbrdDone ) ;
    DosCloseEventSem( hevClipbrdPut ) ;
}

/* Main thread.  Allocate all semaphores.  If we can't it probably
   means that another copy of is already running.  So just die.
*/

int main( void)
{
    CHAR szWindowTitle[255] ;
    HMQ hmq ;
    QMSG qmsg ;
    ULONG flFrameFlags = FCF_WPS & ~FCF_ACCELTABLE ;
    APIRET rc ;

    hab = WinInitialize( 0) ;
    hmq = WinCreateMsgQueue( hab, 0L) ;

    if ( initserver() ) {  /* couldn't create semaphores */
        WinLoadString(  hab, NULLHANDLE, ST_CREATION_ERROR,
                        sizeof( szWindowTitle), szWindowTitle) ;
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szWindowTitle,
            (PSZ) NULL, 1, MB_ERROR  ) ;
        return 1 ;
        }
    else {
        /* load window title from resource file */
        WinLoadString(  hab, NULLHANDLE, ST_WINDOWTITLE,
                        sizeof( szWindowTitle), szWindowTitle) ;

        /* create main window */
        hwndFrame = WinCreateStdWindow( HWND_DESKTOP,
                                        0,
                                        &flFrameFlags,
                                        WC_MLE,
                                        szWindowTitle,
                                        WS_CLIPCHILDREN,
                                        NULLHANDLE,
                                        RS_ALL,
                                        &hwndClient) ;

        WinSetWindowPos( hwndFrame, HWND_BOTTOM, 0L, 0L, 450L, 100L,
            SWP_SIZE | SWP_SHOW | SWP_ZORDER) ;
        WinShowWindow( hwndFrame, FALSE ) ;

        /* subclass the MLE */
        WinSendMsg( hwndClient, WM_PASSPROC,
            MPFROMP( WinSubclassWindow( hwndClient, ClientWndProc)), 0L) ;

        /* Start processing client requests */
        _beginthread( clipsrv, 0, 65535, NULL ) ;
        rc = DosReleaseMutexSem( hmuxClipbrdSrv ) ;

        while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0L, 0L))
            WinDispatchMsg( hab, &qmsg) ;

        /* destroy resources */
        killserver() ;
        WinDestroyWindow (hwndFrame) ;
        WinDestroyMsgQueue (hmq) ;
        WinTerminate (hab) ;
        }
    return 0L ;
}

/* The Clients Window Procedure.  It is here that we actually get the
   Clipboard Data and place it into the shared memory.
*/
MRESULT EXPENTRY ClientWndProc( HWND hwnd,
                                ULONG msg,
                                MPARAM mp1,
                                MPARAM mp2)
{
    static PFNWP pfnwp ;
    static HWND hmenu ;
    APIRET rc ;
    PSZ pClipboard ;

    switch( msg)
    {
        case WM_PASSPROC:
            pfnwp = (PFNWP)mp1 ;
            /* limit text to MAXCHARS characters and make it readonly*/
            WinSendMsg( hwnd, MLM_SETTEXTLIMIT, MPFROMLONG(MAXCHARS), 0L) ;
            WinSendMsg( hwnd, MLM_SETREADONLY, MPFROMLONG(1L), 0L) ;

            /* get the menu handle */
            hmenu = MENU(PAPA(hwnd)) ;
            return 0L ;

        case WM_GETDATA:
          /* Get the data from the clipboard */
          rc = WinOpenClipbrd(hab) ;
          if ( rc ) {
              pClipboard = (PSZ) WinQueryClipbrdData( hab, CF_TEXT ) ;

              /* Copy it to our Shared Memory */
              if ( pClipboard )
                strcpy( pSharedMem, pClipboard ) ;

              WinCloseClipbrd(hab) ;
              }

            /* set text in the MLE window */
            WinSetWindowText( hwnd, pSharedMem) ;

            DosPostEventSem( hevClipbrdReady );

            return 0L;

        case WM_PUTDATA:
            /* The data is in our named shared memory */
            /* we must allocate shared memory to give to the clipboard */
            /* copy the text into it */
            /* and pass it off to the clipboard */

            rc = DosAllocSharedMem( (PPVOID) &pClipboard,
                NULL, strlen(pSharedMem)+1, PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE );

            if ( pClipboard ) {
                strcpy( pClipboard, pSharedMem ) ;
                rc = WinOpenClipbrd(hab);
                if ( rc ) {
                    rc = WinSetClipbrdData( hab, (ULONG) pClipboard, CF_TEXT,
                        CFI_POINTER ) ;
                    WinCloseClipbrd( hab ) ;
                    WinSetWindowText( hwnd, pSharedMem ) ;
                    }
                else {
                    WinSetWindowText( hwnd, "Unable to open Clipboard." ) ;
                    }
                }
            else {
                WinSetWindowText( hwnd, "Unable to allocate Shared Memory for PUT operation.");
                }
            DosPostEventSem( hevClipbrdReady );
            return 0L;

        case WM_INITMENU:
            break ;

        case WM_COMMAND:
            switch( COMMANDMSG( &msg) -> cmd)
            {
                case MN_EXIT:
                    WinPostMsg( hwnd, WM_QUIT, NULL, NULL) ;
                    break ;

                default:
                    break ;
            }
            break ;

        default:
            break;
    }
    return (*pfnwp)( hwnd, msg, mp1, mp2) ;
}


