/* C K N S I G  --  Kermit signal handling for Win32 systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

#include "ckcsym.h"
#include "ckcasc.h"                     /* ASCII character symbols */
#include "ckcdeb.h"                     /* Debug & other symbols */
#include "ckcker.h"                     /* Kermit symbols */
#include "ckcnet.h"                     /* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

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
#include "ckothr.h"
#include "ckosyn.h"

#include <signal.h>
#ifndef NOCCTRAP
#include <setjmp.h>
#endif /* NOCCTRAP */
#include "ckcsig.h"          /* requires ckocon.h to be earlier */


#define THRDSTKSIZ      (2*131072)

extern int network, ttyfd ;
extern int TlsIndex ;

int
cc_execute( ckjptr(sj_buf), ck_sigfunc dofunc, ck_sigfunc failfunc ) {
    int rc = 0 ;
    TID tidThread ;
    ULONG semid ;
    ULONG ccindex ;
#ifdef NT
    HANDLE hevThread ;
#else /* NT */
    HEV hevThread ;
#endif /* NT */
   struct _threadinfo * threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex) ;

    debug(F100,"cc_execute enter","",0);
    /* create an event semaphore for new thread        */
#ifdef NT
    hevThread = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
#else
    DosCreateEventSem( NULL, &hevThread, DC_SEM_SHARED, FALSE ) ;
#endif /* NT */
    CreateCtrlCSem( FALSE, &ccindex ) ;
    CreateCtrlCMuxWait( ccindex, hevThread ) ;

    /* Begin new thread with dofunc                    */

    tidThread = ckThreadBegin(dofunc,THRDSTKSIZ,(void *)&hevThread,TRUE,
                               threadinfo ? threadinfo->id : NULL );

    /* Wait for the event semaphore or Ctrl-C          */
    /*    semaphore to be set                          */
    /* when Ctrl-C semaphore is set execute failfunc   */
    /*    and return -1                                */
    /* else if event semaphore is set return 0         */
    semid = WaitCtrlCMuxWait( ccindex, SEM_INDEFINITE_WAIT ) ;
    if ( semid == 1 ) {                 /* Ctrl-C */
        debug(F100,"cc_execute ctrl-c","",0);
        ResetCtrlCSem(ccindex) ;
        if ( !network ) {
            debug(F100,"cc_execute about to PurgeComm","",0);
            if (PurgeComm( (HANDLE) ttyfd, PURGE_TXABORT | PURGE_RXABORT )) {
                debug( F100, "cc_execute PurgeComm successful","",0);
            } else {
                debug( F101, "cc_execute PurgeComm failed","",GetLastError() );
            }
            msleep(500);
        }
        ckThreadKill( tidThread ) ;
        rc = -1 ;
        (*failfunc)(0) ;
    } else if ( semid == 2 ) {          /* Thread completed successfully */
        debug(F100,"cc_execute thread completed","",0);
        ResetSem( hevThread ) ;
    }
    CloseCtrlCMuxWait(ccindex) ;
    CloseCtrlCSem( ccindex ) ;
#ifdef NT
    CloseHandle( hevThread ) ;
#ifdef NTSIG
    ck_ih();
#endif /* NTSIG */
#else /* NT */
    DosCloseEventSem( hevThread );
#endif /* NT */
    return rc ;
}

int
alrm_execute(ckjptr(sj_buf),
             int timo,
             ck_sighand handler,
             ck_sigfunc dofunc,
             ck_sigfunc failfunc
             )
/* alrm_execute */ {
    int rc = 0;
    TID tidThread ;
    ULONG semid ;
    ULONG alrmindex ;
#ifdef NT
    HANDLE hevThread ;
#else /* NT */
    HEV hevThread ;
#endif /* NT */
    int savalrm = 0;
_PROTOTYP( SIGTYP (*savhandler), (int) );
   struct _threadinfo * threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex) ;

    debug(F100,"alrm_execute enter","",0);
    savalrm = alarm(timo);
    savhandler = signal( SIGALRM, handler );

    /* create an event semaphore for new thread        */
#ifdef NT
    hevThread = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
#else
    DosCreateEventSem( NULL, &hevThread, DC_SEM_SHARED, FALSE ) ;
#endif /* NT */
    CreateAlarmSigSem( FALSE, &alrmindex ) ;
    CreateAlarmSigMuxWait( alrmindex, hevThread ) ;

    /* begin new thread with dofunc                    */

    tidThread = ckThreadBegin(dofunc,THRDSTKSIZ,(void *)&hevThread,TRUE,
                               threadinfo ? threadinfo->id : NULL );


    /* wait for the event semaphore or Ctrl-C          */
    /*    semaphore to be set                          */
    /* when Ctrl-C semaphore is set execute failfunc   */
    /*    and return -1                                */
    /* else if event semaphore is set return 0         */
    semid = WaitAlarmSigMuxWait( alrmindex, SEM_INDEFINITE_WAIT ) ;
    if ( semid == 1 )  {                /* Alarm */
        debug(F100,"alrm_execute timeout","",0);
        (*handler)(SIGALRM) ;
        ResetAlarmSigSem(alrmindex) ;
        if ( !network ) {
            debug(F100,"alrm_execute about to PurgeComm","",0);
            if (PurgeComm( (HANDLE) ttyfd, PURGE_TXABORT | PURGE_RXABORT )) {
                debug( F100, "alrm_execute PurgeComm successful","",0);
            } else {
                debug(F101,"alrm_execute PurgeComm failed","",GetLastError() );
            }
            msleep(500);
        }
        ckThreadKill( tidThread ) ;
        rc = -1 ;
        (*failfunc)(0) ;
    } else if ( semid == 2 ) {          /* Thread completed successfully */
        debug(F100,"alrm_execute thread completed","",0);
        ResetSem( hevThread ) ;
    }
    CloseAlarmSigMuxWait(alrmindex) ;
    CloseAlarmSigSem( alrmindex ) ;
#ifdef NT
    CloseHandle( hevThread ) ;
#else /* NT */
    DosCloseEventSem( hevThread );
#endif /* NT */
    alarm(savalrm) ;
    if ( savhandler )
      signal( SIGALRM, savhandler ) ;
#ifdef NTSIG
    ck_ih();
#endif /* NTSIG */
    return rc ;
}

int
cc_alrm_execute(ckjptr(sj_buf),
             int timo,
             ck_sighand handler,
             ck_sigfunc dofunc,
             ck_sigfunc failfunc
             )
/* cc_alrm_execute */ {
    int rc = 0;
    TID tidThread ;
    ULONG semid ;
    ULONG alrmindex, ccindex ;
#ifdef NT
    HANDLE hevThread ;
#else /* NT */
    HEV hevThread ;
#endif /* NT */
   struct _threadinfo * threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex) ;
    int savalrm = 0;
_PROTOTYP( SIGTYP (*savhandler), (int) );

    debug(F100,"cc_alrm_execute enter","",0);
    savalrm = alarm(timo);
    savhandler = signal( SIGALRM, handler );

    /* create an event semaphore for new thread        */
#ifdef NT
    hevThread = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
#else
    DosCreateEventSem( NULL, &hevThread, DC_SEM_SHARED, FALSE ) ;
#endif /* NT */
    CreateCtrlCSem( FALSE, &ccindex ) ;
    CreateAlarmSigSem( FALSE, &alrmindex ) ;
    CreateCtrlCAlarmSigMuxWait( ccindex, alrmindex, hevThread ) ;

    /* begin new thread with dofunc                    */

    tidThread = ckThreadBegin(dofunc,THRDSTKSIZ,(void *)&hevThread,TRUE,
                               threadinfo ? threadinfo->id : NULL );

    /* wait for the event semaphore or Ctrl-C          */
    /*    semaphore to be set                          */
    /* when Ctrl-C semaphore is set execute failfunc   */
    /*    and return -1                                */
    /* else if event semaphore is set return 0         */
    semid = WaitCtrlCAlarmSigMuxWait(ccindex,alrmindex,SEM_INDEFINITE_WAIT);
    if ( semid == 1 ) {                 /* CtrlC  */
        debug(F100,"cc_alrm_execute ctrl-c","",0);
        ResetCtrlCSem(ccindex) ;
        ckThreadKill( tidThread ) ;
        rc = -1 ;
        (*failfunc)(0) ;
    } else if ( semid == 2 ) {          /* Alarm */
        debug(F100,"cc_alrm_execute timeout","",0);
        (*handler)(SIGALRM) ;
        ResetAlarmSigSem(alrmindex) ;
        if ( !network ) {
            debug(F100,"cc_alrm_execute about to PurgeComm","",0);
            if (PurgeComm( (HANDLE) ttyfd, PURGE_TXABORT | PURGE_RXABORT )) {
                debug( F100, "cc_alrm_execute PurgeComm successful","",0);
            } else {
                debug(F101,"cc_alrm_execute PurgeComm failed","",
                      GetLastError() );
            }
            msleep(500);
        }
        ckThreadKill( tidThread ) ;
        rc = -1 ;
        (*failfunc)(0) ;
    } else if ( semid == 3 ) {          /* Thread completed successfully */
        debug(F100,"cc_alrm_execute thread completed","",0);
        ResetSem( hevThread ) ;
    }
    CloseCtrlCAlarmSigMuxWait(alrmindex) ;
    CloseCtrlCSem( ccindex ) ;
    CloseAlarmSigSem( alrmindex ) ;
#ifdef NT
    CloseHandle( hevThread ) ;
#else /* NT */
    DosCloseEventSem( hevThread );
#endif /* NT */
    alarm(savalrm) ;
    if ( savhandler )
      signal( SIGALRM, savhandler ) ;
#ifdef NTSIG
    ck_ih();
#endif /* NTSIG */
    return rc ;
}

