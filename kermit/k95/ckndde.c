/* C K N D D E -- Kermit Dynamic Data Exchange support for Win32 */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"                     /* Typedefs, debug formats, etc */
#include "ckcasc.h"                     /* ASCII character names */
#include "ckcker.h"                     /* Kermit definitions */
#include "ckuusr.h"                     /* Command definitions - needed for ckokey.h */
#include <ctype.h>                      /* Character types */
#include <stdio.h>                      /* Standard i/o */
#include <io.h>                         /* File io function declarations */
#include <process.h>                    /* Process-control functions */
#include <string.h>                     /* String manipulation declarations */
#include <stdlib.h>                     /* Standard library declarations */
#include <signal.h>

#ifdef NT
#include <windows.h>
#include <dde.h>
#include <ddeml.h>
#define strdup _strdup
#endif /* NT */

/* Begin Keyboard Handler Thread Code */
int
DDEHandlerInit( void ) {
    int rc = 0 ;

    debug(F100,"DDEHandlerInit called","",0);
    DDEActive = 1 ;

    CreateDDEThreadSem( FALSE ) ;
    tidDDEHandler = (TID) ckThreadBegin( &DDEHandlerThread,
                                         THRDSTKSIZ, 0, FALSE, 0 ) ;
    if ( tidDDEHandler == (TID) -1 ) {
        printf( "Sorry, can't create DDEHandlerThread\n" ) ;
        rc = -1 ;
    }
    else {
        WaitAndResetDDEThreadSem( SEM_INDEFINITE_WAIT ) ;
    }
    return rc ;
}

int
DDEHandlerCleanup( void ) {
    APIRET rc=0 ;
    int n = 0;
    debug(F100,"DDEHandlerCleanup called","",0);
    DDEActive = 0 ;
    while ( !WaitAndResetDDEThreadSem( 1000 ) && n++ < 50) {
        debug(F100,"Waiting for DDEThreadSem","",0) ;
    }
    tidDDEHandler = (TID) 0 ;
    CloseDDEThreadSem() ;
    return 0 ;
}

void
DDEHandlerThread( void * pArgList ) {
    DWORD count = 0;
    int rc, c, i ;
    extern BYTE vmode ;

    debug(F100,"DDEHandlerThread is born","",0);

    PostDDEThreadSem() ;
    debug(F100,"DDEHandlerThread dies","",0);
    ckThreadEnd(pArgList);
}

