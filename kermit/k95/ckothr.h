/* C K O T H R  --  Kermit thread management functions for OS/2 and NT systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1996,2004, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/

#ifndef CKOTHR_H
#define CKOTHR_H
#ifdef NT
#ifndef APIRET
#define APIRET DWORD
#endif /* APIRET */
#ifndef SEM_INDEFINITE_WAIT
#define SEM_INDEFINITE_WAIT INFINITE
#endif /* SEM_INDEFINITE_WAIT */
#ifndef DCWW_WAIT
#define DCWW_WAIT -1
#endif /* DCWW_WAIT */
#ifndef ERROR_VIO_ROW
#define ERROR_VIO_ROW 1
#endif
#ifndef ERROR_VIO_COL
#define ERROR_VIO_COL 2
#endif
#ifndef CCHMAXPATH
#define CCHMAXPATH _MAX_PATH
#endif
#ifndef TID
#define TID HANDLE
#endif
#ifndef PIB
#define PIB HANDLE
#define PPIB HANDLE*
#endif
#ifndef TIB
#define TIB HANDLE
#define PTIB HANDLE*
#endif
#ifndef HVIO
#define HVIO HANDLE
#endif
#ifndef HMTX
#define HMTX HANDLE
#endif
#ifndef HAB
#define HAB HANDLE
#endif
#ifndef HKBD
#define HKBD HANDLE
#endif
#ifndef HDIR
#define HDIR HANDLE
#endif
#endif /* NT */

#ifdef NT
_PROTOTYP( TID  ckThreadBegin, ( void (*)(void*), unsigned long, void *, BOOL, HANDLE) ) ;
#else
_PROTOTYP( TID  ckThreadBegin, ( void (*)(void*), unsigned long, void *, BOOL, TID) ) ;
#endif
_PROTOTYP( BOOL ckThreadKill, ( TID ) ) ;
_PROTOTYP( BOOL ckThreadCleanup, ( TID ) ) ;
_PROTOTYP( VOID ckThreadEnd, ( void * ) ) ;
_PROTOTYP( VOID ckThreadDie, ( void * ) ) ;
#endif /* CKOTHR_H */
