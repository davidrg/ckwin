/* C K O S I G  --  Kermit signal handling for OS/2 and Win32 systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcsym.h"
#include "ckcasc.h"                     /* ASCII character symbols */
#include "ckcdeb.h"                     /* Debug & other symbols */
#include "ckcker.h"                     /* Kermit symbols */
#include "ckcnet.h"                     /* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

#include <signal.h>
#include <process.h>
#ifndef NOCCTRAP
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#endif /* NOCCTRAP */
#include "ckusig.h"

#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_WIN
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */
#include "ckocon.h"

#define THRDSTKSIZ      (2*131072)

int
cc_execute( jmp_buf * sj_buf, ck_sigfunc dofunc, ck_sigfunc failfunc )
{
    int rc = 0 ;
    TID tidThread ;
    ULONG semid ;
    ULONG ccindex ;
   #ifdef NT
    HANDLE hevThread ;
   #else /* NT */
    HEV hevThread ;
   #endif /* NT */

    /* create an event semaphore for new thread        */
   #ifdef NT
    hevThread = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
   #else
    DosCreateEventSem( NULL, &hevThread, DC_SEM_SHARED, FALSE ) ;
   #endif /* NT */
    CreateCtrlCSem( FALSE, &ccindex ) ;
    CreateCtrlCMuxWait( ccindex, hevThread ) ;

    /* begin new thread with dofunc                    */
    tidThread = (TID) _beginthread( dofunc,
                                                                         #ifndef NT
                                                                          0,
                                                                         #endif /* NT */
                                                                          THRDSTKSIZ, (void *) &hevThread ) ;

    /* wait for the event semaphore or Ctrl-C          */
    /*    semaphore to be set                          */
    /* when Ctrl-C semaphore is set execute failfunc   */
    /*    and return -1                                */
    /* else if event semaphore is set return 0         */
    semid = WaitCtrlCMuxWait( ccindex, SEM_INDEFINITE_WAIT ) ;
    if ( semid == 1 )  /* Ctrl-C */
    {
        ResetCtrlCSem(ccindex) ;
       #ifdef NT
        KillThread( tidThread ) ;
       #else /* NT */
        DosKillThread( tidThread ) ;
       #endif /* NT */
        rc = -1 ;
        (*failfunc)(0) ;
    } else if ( semid == 2 )  /* Thread completed successfully */
    {
        ResetSem( hevThread ) ;
    }

    CloseCtrlCMuxWait(ccindex) ;
    CloseCtrlCSem( ccindex ) ;
   #ifdef NT
    CloseHandle( hevThread ) ;
   #else /* NT */
    DosCloseEventSem( hevThread );
   #endif /* NT */
    return rc ;
}

int
alrm_execute(jmp_buf * sj_buf,
             int timo,
             ck_sighand handler,
             ck_sigfunc dofunc,
             ck_sigfunc failfunc
             )
{

    int rc = 0;
    int savalrm = 0;
_PROTOTYP( SIGTYP (*savhandler), (int) );

    savalrm = alarm(timo);
    savhandler = signal( SIGALRM, handler );
    if (
        setjmp(*sj_buf)
        ) {                             /* Alarm trap returns to here. */
        (*failfunc)(NULL) ;
        rc = -1 ;
    } else {
        (*dofunc)(NULL) ;
    }
    alarm(savalrm) ;
    if ( savhandler )
        signal( SIGALRM, savhandler ) ;
    return rc ;
}

