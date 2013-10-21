/* C K O S Y N  --  Kermit synchronization functions for OS/2 and NT systems */

/*
  Author:
    Jeffrey Altman (jaltman@secure-endpoints.com),
      Secure Endpoints Inc., New York City.

  Copyright (C) 1996,2004 Trustees of Columbia University in the City of New
  York.
*/

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckuusr.h"             /* needed for Priority definitions */
#include <signal.h>

#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "cknalm.h"
#include "ckntap.h"

#include "ckocon.h"

HANDLE hmtxKeyStroke[VNUM] = {(HANDLE) 0,(HANDLE) 0,(HANDLE) 0}  ;
HANDLE hmtxLocalEcho = (HANDLE) 0 ;
HANDLE hmtxTCPIP = (HANDLE) 0 ;
HANDLE hmtxComm = (HANDLE) 0;
#ifdef NETCMD
HANDLE hmtxNetCmd = (HANDLE) 0 ;
#endif /* NETCMD */
HANDLE hmtxKeyboard  = (HANDLE) 0 ;
HANDLE hmtxAlarm = (HANDLE) 0 ;
HANDLE hmtxScreen = (HANDLE) 0 ;
HANDLE hmtxVscrn[VNUM] = { (HANDLE) 0,(HANDLE) 0, (HANDLE) 0} ;
HANDLE hmtxVscrnDirty[VNUM] = { (HANDLE) 0, (HANDLE) 0, (HANDLE) 0 } ;
HANDLE hmtxConnectMode = (HANDLE) 0 ;
HANDLE hmtxDebug = (HANDLE) 0;
HANDLE hmtxTelnet = (HANDLE) 0;
HANDLE hmtxSerial = (HANDLE) 0 ;
HANDLE hmtxThreadMgmt = (HANDLE) 0 ;
#ifdef CK_SSL
HANDLE hmtxSSL = (HANDLE) 0;
#endif /* CK_SSL */

HANDLE hevEventAvail[VNUM] = {(HANDLE) 0,(HANDLE) 0,(HANDLE) 0} ;
HANDLE hevLocalEchoAvail = (HANDLE) 0 ;
#ifdef NETCMD
HANDLE hevNetCmdAvail = (HANDLE) 0 ;
#endif
HANDLE hevAlarm = (HANDLE) 0 ;
HANDLE hevTerminalMode = (HANDLE) 0 ;
HANDLE hevCommandMode = (HANDLE) 0 ;
HANDLE hevRdComWrtScrThread = (HANDLE) 0 ;
HANDLE hevConKbdHandlerThread = (HANDLE) 0 ;
HANDLE hevTermScrnUpdThread = (HANDLE) 0 ;
HANDLE hevRdComWrtScrThreadDown = (HANDLE) 0 ;
HANDLE hevConKbdHandlerThreadDown = (HANDLE) 0 ;
HANDLE hevTermScrnUpdThreadDown = (HANDLE) 0 ;
HANDLE hevKeyMapInit = (HANDLE) 0 ;
HANDLE hevKbdThread = (HANDLE) 0 ;
HANDLE hevCtrlC[4]      = {(HANDLE) 0,(HANDLE) 0,(HANDLE) 0,(HANDLE) 0} ;
HANDLE hevAlarmSig[4]   = {(HANDLE) 0,(HANDLE) 0,(HANDLE) 0,(HANDLE) 0} ;

UINT htimAlarm = (UINT) 0 ;
UINT htimVscrn[VNUM] = {(UINT) 0, (UINT) 0, (UINT) 0} ;

HANDLE hevVscrnTimer[VNUM] = { (HANDLE) 0, (HANDLE) 0,(HANDLE) 0 } ;
HANDLE hevVscrnDirty[VNUM] = { (HANDLE) 0, (HANDLE) 0,(HANDLE) 0 } ;

HANDLE hevVscrnUpdate[VNUM][2] = {{(HANDLE) NULL, (HANDLE) NULL},
   {(HANDLE) NULL, (HANDLE) NULL},
   {(HANDLE) NULL, (HANDLE) NULL}};
HANDLE hmuxCtrlC[4][2] = { { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL } } ;
HANDLE hmuxAlarmSig[4][2] = { { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL },
                          { (HANDLE) NULL, (HANDLE) NULL } } ;
HANDLE hmuxCtrlCAlarmSig[3] = { (HANDLE) NULL, (HANDLE) NULL, (HANDLE) NULL } ;

#ifdef CK_TAPI
HANDLE hevTAPIConnect = (HANDLE) 0 ;
HANDLE hevTAPIAnswer = (HANDLE) 0 ;
HANDLE hevTAPIInit = (HANDLE) 0 ;
#endif /* CK_TAPI */

HANDLE hevRichEditInit = (HANDLE) 0;
HANDLE hevRichEditClose = (HANDLE) 0;
HANDLE hmtxRichEdit = (HANDLE) 0 ;
#else /* NT */
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */

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

#include "ckocon.h"

HMTX hmtxKeyStroke[VNUM] = {(HMTX) 0,(HMTX) 0,(HMTX) 0} ;
#ifdef NETCMD
HMTX hmtxNetCmd = (HMTX) 0 ;
#endif
HMTX hmtxLocalEcho = (HMTX) 0 ;
HMTX hmtxTCPIP = (HMTX) 0 ;
HMTX hmtxComm = (HMTX) 0 ;
HMTX hmtxKeyboard  = (HMTX) 0 ;
HMTX hmtxAlarm = (HMTX) 0 ;
HMTX hmtxScreen = (HMTX) 0 ;
HMTX hmtxVscrn[VNUM] = { (HMTX) 0, (HMTX) 0, (HMTX) 0}  ;
HMTX hmtxVscrnDirty[VNUM] = { (HMTX) 0, (HMTX) 0, (HMTX) 0} ;
HMTX hmtxConnectMode = (HMTX) 0 ;
HMTX hmtxDebug = (HMTX) 0 ;
HMTX hmtxTelnet = (HMTX) 0 ;
HMTX hmtxThreadMgmt = (HMTX) 0 ;
#ifdef CK_SSL
HMTX hmtxSSL = (HMTX) 0;
#endif /* CK_SSL */

HEV  hevEventAvail[VNUM] = {(HEV) 0,(HEV) 0,(HEV) 0} ;
HEV  hevLocalEchoAvail = (HEV) 0 ;
#ifdef NETCMD
HEV  hevNetCmdAvail = (HEV) 0 ;
#endif
HEV  hevAlarm = (HEV) 0 ;
HEV  hevTerminalMode = (HEV) 0 ;
HEV  hevCommandMode = (HEV) 0 ;
HEV  hevRdComWrtScrThread = (HEV) 0 ;
HEV  hevConKbdHandlerThread = (HEV) 0 ;
HEV  hevTermScrnUpdThread = (HEV) 0 ;
HEV  hevRdComWrtScrThreadDown = (HEV) 0 ;
HEV  hevConKbdHandlerThreadDown = (HEV) 0 ;
HEV  hevTermScrnUpdThreadDown = (HEV) 0 ;
HEV  hevKeyMapInit = (HEV) 0 ;
HEV  hevKbdThread = (HEV) 0 ;
HEV  hevVscrnTimer[VNUM] = { (HEV) 0, (HEV) 0, (HEV) 0 } ;
HEV  hevVscrnDirty[VNUM] = { (HEV) 0, (HEV) 0, (HEV) 0 } ;
HEV  hevCtrlC[4]   = { (HEV) 0,(HEV) 0,(HEV) 0,(HEV) 0 } ;
HEV  hevAlarmSig[4] = { (HEV) 0,(HEV) 0,(HEV) 0,(HEV) 0 } ;

HTIMER htimAlarm = (HTIMER) 0 ;
HTIMER htimVscrn[VNUM] = {(HTIMER) 0,(HTIMER) 0,(HTIMER) 0} ;

HMUX hmuxVscrnUpdate[VNUM] = {(HMUX) 0,(HMUX) 0,(HMUX) 0} ;
HMUX hmuxCtrlC[4]    = { (HMUX) 0,(HMUX) 0,(HMUX) 0,(HMUX) 0 } ;
HMUX hmuxAlarmSig[4] = { (HMUX) 0,(HMUX) 0,(HMUX) 0,(HMUX) 0 } ;
HMUX hmuxCtrlCAlarmSig = (HMUX) 0 ;
#endif /* NT */

int CtrlCCount = -1 ;
#define MAXCTRLC 3
int AlarmSigCount = -1 ;
#define MAXALARMSIG 3


/* Semaphore functions */

APIRET
CreateKeyStrokeMutex( BOOL owned )
{
    int j ;

    for ( j=0; j<VNUM ; j++ )
    {
        if ( hmtxKeyStroke[j] )
#ifdef NT
          CloseHandle( hmtxKeyStroke[j] ) ;
        hmtxKeyStroke[j] = CreateMutex( NULL, owned, NULL ) ;
        if (hmtxKeyStroke[j] == NULL)
          return GetLastError();
#else /* not NT */
        DosCloseMutexSem( hmtxKeyStroke[j] ) ;
        DosCreateMutexSem( NULL, &hmtxKeyStroke[j], 0, owned ) ;
#endif /* NT */
    }
    return 0;
}

APIRET
RequestKeyStrokeMutex( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxKeyStroke[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxKeyStroke[vmode], timo ) ;
#endif /* NT */
}

APIRET
ReleaseKeyStrokeMutex( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxKeyStroke[vmode] ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxKeyStroke[vmode] ) ;
#endif /* NT */
}

APIRET
CloseKeyStrokeMutex( void )
{
    int j;
#ifdef NT
    BOOL rc = 0 ;
    for ( j=0; j<VNUM ; j++ )
    {
        rc = CloseHandle( hmtxKeyStroke[j] ) ;
        hmtxKeyStroke[j] = (HANDLE) NULL ;
    }
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    for ( j=0; j<VNUM ; j++ )
    {
        rc = DosCloseMutexSem( hmtxKeyStroke[j] ) ;
        hmtxKeyStroke[j] = 0 ;
    }
    return rc ;
#endif /* NT */
}

APIRET
CreateKeyboardMutex( BOOL owned )
{
    if ( hmtxKeyboard )
#ifdef NT
      CloseHandle( hmtxKeyboard ) ;
    hmtxKeyboard = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxKeyboard == NULL)
          return GetLastError() ;
#else /* not NT */
        DosCloseMutexSem( hmtxKeyboard ) ;
        DosCreateMutexSem( NULL, &hmtxKeyboard, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestKeyboardMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxKeyboard, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxKeyboard, timo ) ;
#endif /* NT */
}

APIRET
ReleaseKeyboardMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxKeyboard ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxKeyboard ) ;
#endif /* NT */
}

APIRET
CloseKeyboardMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxKeyboard ) ;
    hmtxKeyboard = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxKeyboard ) ;
    hmtxKeyboard = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateAlarmMutex( BOOL owned )
{
    if ( hmtxAlarm )
#ifdef NT
        CloseHandle( hmtxAlarm ) ;
    hmtxAlarm = CreateMutex( NULL, owned, NULL ) ;
    return hmtxAlarm == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxAlarm ) ;
    return DosCreateMutexSem( NULL, &hmtxAlarm, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestAlarmMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxAlarm, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxAlarm, timo ) ;
#endif /* NT */
}

APIRET
ReleaseAlarmMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxAlarm ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxAlarm ) ;
#endif /* NT */
}

APIRET
CloseAlarmMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxAlarm ) ;
    hmtxAlarm = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxAlarm ) ;
    hmtxAlarm = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateScreenMutex( BOOL owned )
{
    if ( hmtxScreen )
#ifdef NT
        CloseHandle( hmtxScreen ) ;
    hmtxScreen = CreateMutex( NULL, owned, NULL ) ;
    return hmtxScreen == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxScreen ) ;
    return DosCreateMutexSem( NULL, &hmtxScreen, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestScreenMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxScreen, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxScreen, timo ) ;
#endif /* NT */
}

APIRET
ReleaseScreenMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxScreen ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxScreen ) ;
#endif /* NT */
}

APIRET
CloseScreenMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxScreen ) ;
    hmtxScreen = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxScreen ) ;
    hmtxScreen = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateVscrnMutex( BOOL owned )
{
   int i, rc = 0 ;
   for ( i = 0 ; i < VNUM ; i++ )
   {
    if ( hmtxVscrn[i] )
#ifdef NT
        CloseHandle( hmtxVscrn[i] ) ;
    hmtxVscrn[i] = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxVscrn[i] == NULL)
       return GetLastError() ;
#else /* not NT */
      DosCloseMutexSem( hmtxVscrn[i] ) ;
      rc = DosCreateMutexSem( NULL, &hmtxVscrn[i], 0, owned ) ;
      if ( rc )
         return rc;
#endif /* NT */
   }
   return 0;
}

APIRET
RequestVscrnMutex( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;
    rc = WaitForSingleObjectEx( hmtxVscrn[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : GetLastError() ;
#else /* not NT */
    return DosRequestMutexSem( hmtxVscrn[vmode], timo ) ;
#endif /* NT */
}

APIRET
ReleaseVscrnMutex( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxVscrn[vmode] ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxVscrn[vmode] ) ;
#endif /* NT */
}

APIRET
CloseVscrnMutex( void )
{
   int i ;
   int rc  = 0 ;
   for ( i=0 ; i<VNUM ; i++ )
   {
#ifdef NT
    rc = CloseHandle( hmtxVscrn[i] ) ;
    hmtxVscrn[i] = (HANDLE) NULL ;
    if (!rc)
       return GetLastError() ;
#else /* not NT */
    rc = DosCloseMutexSem( hmtxVscrn[i] ) ;
    hmtxVscrn[i] = 0 ;
    if (rc)
       return rc ;
#endif /* NT */
   }
   return 0;
}

APIRET
CreateVscrnDirtyMutex( BOOL owned )
{
   int i, rc ;
   for ( i=0; i<VNUM; i++)
   {
    if ( hmtxVscrnDirty[i] )
#ifdef NT
        CloseHandle( hmtxVscrnDirty[i] ) ;
    hmtxVscrnDirty[i] = CreateMutex( NULL, owned, NULL ) ;
    if ( hmtxVscrnDirty[i] == NULL)
       return GetLastError() ;
#else /* not NT */
        DosCloseMutexSem( hmtxVscrnDirty[i] ) ;
    rc = DosCreateMutexSem( NULL, &hmtxVscrnDirty[i], 0, owned ) ;
      if ( rc )
         return rc;
#endif /* NT */
   }
   return 0;
}

APIRET
RequestVscrnDirtyMutex( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxVscrnDirty[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : GetLastError() ;
#else /* not NT */
    return DosRequestMutexSem( hmtxVscrnDirty[vmode], timo ) ;
#endif /* NT */
}

APIRET
ReleaseVscrnDirtyMutex( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxVscrnDirty[vmode] ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxVscrnDirty[vmode] ) ;
#endif /* NT */
}

APIRET
CloseVscrnDirtyMutex( void )
{
   int i, rc = 0 ;
   for ( i=0 ; i<VNUM ; i++ )
   {
#ifdef NT
    rc = CloseHandle( hmtxVscrnDirty[i] ) ;
    hmtxVscrnDirty[i] = (HANDLE) NULL ;
    if (!rc)
       return GetLastError() ;
#else /* not NT */
    rc = DosCloseMutexSem( hmtxVscrnDirty[i] ) ;
    hmtxVscrnDirty[i] = 0 ;
    if (rc)
       return rc ;
#endif /* NT */
   }
   return 0;
}


APIRET
CreateConnectModeMutex( BOOL owned )
{
    if ( hmtxConnectMode )
#ifdef NT
        CloseHandle( hmtxConnectMode ) ;
    hmtxConnectMode = CreateMutex( NULL, owned, NULL ) ;
    return hmtxConnectMode == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxConnectMode ) ;
    return DosCreateMutexSem( NULL, &hmtxConnectMode, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestConnectModeMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxConnectMode, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxConnectMode, timo ) ;
#endif /* NT */
}

APIRET
ReleaseConnectModeMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxConnectMode ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxConnectMode ) ;
#endif /* NT */
}

APIRET
CloseConnectModeMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxConnectMode ) ;
    hmtxConnectMode = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxConnectMode ) ;
    hmtxConnectMode = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateDebugMutex( BOOL owned )
{
    if ( hmtxDebug )
#ifdef NT
        CloseHandle( hmtxDebug ) ;
    hmtxDebug = CreateMutex( NULL, owned, NULL ) ;
    return hmtxDebug == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxDebug ) ;
    return DosCreateMutexSem( NULL, &hmtxDebug, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestDebugMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxDebug, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxDebug, timo ) ;
#endif /* NT */
}

APIRET
ReleaseDebugMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxDebug ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxDebug ) ;
#endif /* NT */
}

APIRET
CloseDebugMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxDebug ) ;
    hmtxDebug = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxDebug ) ;
    hmtxDebug = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateTelnetMutex( BOOL owned )
{
    if ( hmtxTelnet )
#ifdef NT
        CloseHandle( hmtxTelnet ) ;
    hmtxTelnet = CreateMutex( NULL, owned, NULL ) ;
    return hmtxTelnet == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxTelnet ) ;
    return DosCreateMutexSem( NULL, &hmtxTelnet, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestTelnetMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxTelnet, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxTelnet, timo ) ;
#endif /* NT */
}

APIRET
ReleaseTelnetMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxTelnet ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxTelnet ) ;
#endif /* NT */
}

APIRET
CloseTelnetMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxTelnet ) ;
    hmtxTelnet = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxTelnet ) ;
    hmtxTelnet = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateEventAvailSem( BOOL posted )
{
    int j;

    for ( j=0; j<VNUM; j++ )
    {
        if ( hevEventAvail[j] )
#ifdef NT
        CloseHandle( hevEventAvail[j] ) ;
    hevEventAvail[j] = CreateEvent( NULL, TRUE, posted, NULL ) ;
    if (hevEventAvail == NULL)
      return GetLastError() ;
#else /* not NT */
        DosCloseEventSem( hevEventAvail[j] ) ;
        DosCreateEventSem( NULL, &hevEventAvail[j], 0, posted ) ;
#endif /* NT */
    }
    return 0;
}

APIRET
PostEventAvailSem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevEventAvail[vmode] ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevEventAvail[vmode] ) ;
#endif /* NT */
}

APIRET
WaitEventAvailSem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevEventAvail[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;
    rc = DosWaitEventSem( hevEventAvail[vmode], timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetEventAvailSem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevEventAvail[vmode], timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevEventAvail[vmode] ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevEventAvail[vmode], timo ) ;
    if ( !rc )
        DosResetEventSem( hevEventAvail[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetEventAvailSem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevEventAvail[vmode] ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevEventAvail[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseEventAvailSem( void )
{
    int j;
#ifdef NT
    BOOL rc = 0 ;
    for ( j=0 ; j<VNUM ; j++ )
    {
        rc = CloseHandle( hevEventAvail[j] ) ;
        hevEventAvail[j] = (HANDLE) NULL ;
    }
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    for ( j=0 ; j<VNUM ; j++ )
    {
        rc = DosCloseEventSem( hevEventAvail[j] ) ;
        hevEventAvail[j] = 0 ;
    }
    return rc ;
#endif /* NT */
}

APIRET
CreateAlarmSem( BOOL posted )
{
    if ( hevAlarm )
#ifdef NT
        CloseHandle( hevAlarm ) ;
    hevAlarm = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevAlarm == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevAlarm ) ;
    return DosCreateEventSem( NULL, &hevAlarm, DC_SEM_SHARED, posted ) ;
#endif /* NT */
}

APIRET
PostAlarmSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevAlarm ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevAlarm ) ;
#endif /* NT */
}

APIRET
WaitAlarmSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevAlarm, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevAlarm, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetAlarmSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevAlarm, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevAlarm ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevAlarm, timo ) ;
    if ( !rc )
        DosResetEventSem( hevAlarm, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetAlarmSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevAlarm ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevAlarm, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseAlarmSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevAlarm ) ;
    hevAlarm = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevAlarm ) ;
    hevAlarm = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateRdComWrtScrThreadSem( BOOL posted )
{
    if ( hevRdComWrtScrThread )
#ifdef NT
        CloseHandle( hevRdComWrtScrThread ) ;
    hevRdComWrtScrThread = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevRdComWrtScrThread == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevRdComWrtScrThread ) ;
    return DosCreateEventSem( NULL, &hevRdComWrtScrThread, 0, posted ) ;
#endif /* NT */
}

APIRET
PostRdComWrtScrThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevRdComWrtScrThread ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevRdComWrtScrThread ) ;
#endif /* NT */
}

APIRET
WaitRdComWrtScrThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRdComWrtScrThread, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevRdComWrtScrThread, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetRdComWrtScrThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRdComWrtScrThread, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevRdComWrtScrThread ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevRdComWrtScrThread, timo ) ;
    if ( !rc )
        DosResetEventSem( hevRdComWrtScrThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetRdComWrtScrThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevRdComWrtScrThread ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevRdComWrtScrThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseRdComWrtScrThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevRdComWrtScrThread ) ;
    hevRdComWrtScrThread = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevRdComWrtScrThread ) ;
    hevRdComWrtScrThread = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateConKbdHandlerThreadSem( BOOL posted )
{
    if ( hevConKbdHandlerThread )
#ifdef NT
        CloseHandle( hevConKbdHandlerThread ) ;
    hevConKbdHandlerThread = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevConKbdHandlerThread == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevConKbdHandlerThread ) ;
    return DosCreateEventSem( NULL, &hevConKbdHandlerThread, 0, posted ) ;
#endif /* NT */
}

APIRET
PostConKbdHandlerThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevConKbdHandlerThread ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevConKbdHandlerThread ) ;
#endif /* NT */
}

APIRET
WaitConKbdHandlerThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevConKbdHandlerThread, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevConKbdHandlerThread, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetConKbdHandlerThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevConKbdHandlerThread, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevConKbdHandlerThread ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevConKbdHandlerThread, timo ) ;
    if ( !rc )
        DosResetEventSem( hevConKbdHandlerThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetConKbdHandlerThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevConKbdHandlerThread ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevConKbdHandlerThread, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseConKbdHandlerThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevConKbdHandlerThread ) ;
    hevConKbdHandlerThread = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevConKbdHandlerThread ) ;
    hevConKbdHandlerThread = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateTermScrnUpdThreadSem( BOOL posted )
{
    if ( hevTermScrnUpdThread )
#ifdef NT
        CloseHandle( hevTermScrnUpdThread ) ;
    hevTermScrnUpdThread = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTermScrnUpdThread == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevTermScrnUpdThread ) ;
    return DosCreateEventSem( NULL, &hevTermScrnUpdThread, 0, posted ) ;
#endif /* NT */
}

APIRET
PostTermScrnUpdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTermScrnUpdThread ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTermScrnUpdThread ) ;
#endif /* NT */
}

APIRET
WaitTermScrnUpdThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTermScrnUpdThread, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTermScrnUpdThread, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTermScrnUpdThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTermScrnUpdThread, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTermScrnUpdThread ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTermScrnUpdThread, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTermScrnUpdThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTermScrnUpdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTermScrnUpdThread ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTermScrnUpdThread, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseTermScrnUpdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTermScrnUpdThread ) ;
    hevTermScrnUpdThread = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTermScrnUpdThread ) ;
    hevTermScrnUpdThread = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateRdComWrtScrThreadDownSem( BOOL posted )
{
    if ( hevRdComWrtScrThreadDown )
#ifdef NT
        CloseHandle( hevRdComWrtScrThreadDown ) ;
    hevRdComWrtScrThreadDown = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevRdComWrtScrThreadDown == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevRdComWrtScrThreadDown ) ;
    return DosCreateEventSem( NULL, &hevRdComWrtScrThreadDown, 0, posted ) ;
#endif /* NT */
}

APIRET
PostRdComWrtScrThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevRdComWrtScrThreadDown ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevRdComWrtScrThreadDown ) ;
#endif /* NT */
}

APIRET
WaitRdComWrtScrThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRdComWrtScrThreadDown, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevRdComWrtScrThreadDown, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetRdComWrtScrThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRdComWrtScrThreadDown, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevRdComWrtScrThreadDown ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevRdComWrtScrThreadDown, timo ) ;
    if ( !rc )
        DosResetEventSem( hevRdComWrtScrThreadDown, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetRdComWrtScrThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevRdComWrtScrThreadDown ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevRdComWrtScrThreadDown, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseRdComWrtScrThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevRdComWrtScrThreadDown ) ;
    hevRdComWrtScrThreadDown = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevRdComWrtScrThreadDown ) ;
    hevRdComWrtScrThreadDown = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateConKbdHandlerThreadDownSem( BOOL posted )
{
    if ( hevConKbdHandlerThreadDown )
#ifdef NT
        CloseHandle( hevConKbdHandlerThreadDown ) ;
    hevConKbdHandlerThreadDown = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevConKbdHandlerThreadDown == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevConKbdHandlerThreadDown ) ;
    return DosCreateEventSem( NULL, &hevConKbdHandlerThreadDown, 0, posted ) ;
#endif /* NT */
}

APIRET
PostConKbdHandlerThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevConKbdHandlerThreadDown ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevConKbdHandlerThreadDown ) ;
#endif /* NT */
}

APIRET
WaitConKbdHandlerThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevConKbdHandlerThreadDown, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevConKbdHandlerThreadDown, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetConKbdHandlerThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevConKbdHandlerThreadDown, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevConKbdHandlerThreadDown ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevConKbdHandlerThreadDown, timo ) ;
    if ( !rc )
        DosResetEventSem( hevConKbdHandlerThreadDown, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetConKbdHandlerThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevConKbdHandlerThreadDown ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevConKbdHandlerThreadDown, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseConKbdHandlerThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevConKbdHandlerThreadDown ) ;
    hevConKbdHandlerThreadDown = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevConKbdHandlerThreadDown ) ;
    hevConKbdHandlerThreadDown = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateTermScrnUpdThreadDownSem( BOOL posted )
{
    if ( hevTermScrnUpdThreadDown )
#ifdef NT
        CloseHandle( hevTermScrnUpdThreadDown ) ;
    hevTermScrnUpdThreadDown = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTermScrnUpdThreadDown == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevTermScrnUpdThreadDown ) ;
    return DosCreateEventSem( NULL, &hevTermScrnUpdThreadDown, 0, posted ) ;
#endif /* NT */
}

APIRET
PostTermScrnUpdThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTermScrnUpdThreadDown ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTermScrnUpdThreadDown ) ;
#endif /* NT */
}

APIRET
WaitTermScrnUpdThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTermScrnUpdThreadDown, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTermScrnUpdThreadDown, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTermScrnUpdThreadDownSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTermScrnUpdThreadDown, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTermScrnUpdThreadDown ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTermScrnUpdThreadDown, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTermScrnUpdThreadDown, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTermScrnUpdThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTermScrnUpdThreadDown ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTermScrnUpdThreadDown, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseTermScrnUpdThreadDownSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTermScrnUpdThreadDown ) ;
    hevTermScrnUpdThreadDown = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTermScrnUpdThreadDown ) ;
    hevTermScrnUpdThreadDown = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateKbdThreadSem( BOOL posted )
{
    if ( hevKbdThread )
#ifdef NT
        CloseHandle( hevKbdThread ) ;
    hevKbdThread = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevKbdThread == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevKbdThread ) ;
    return DosCreateEventSem( NULL, &hevKbdThread, 0, posted ) ;
#endif /* NT */
}

APIRET
PostKbdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevKbdThread ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevKbdThread ) ;
#endif /* NT */
}

APIRET
WaitKbdThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevKbdThread, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevKbdThread, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetKbdThreadSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevKbdThread, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevKbdThread ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0;

    rc = DosWaitEventSem( hevKbdThread, timo ) ;
    if ( !rc )
        DosResetEventSem( hevKbdThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetKbdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevKbdThread ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevKbdThread, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseKbdThreadSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevKbdThread ) ;
    hevKbdThread = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevKbdThread ) ;
    hevKbdThread = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateKeyMapInitSem( BOOL posted )
{
    if ( hevKeyMapInit )
#ifdef NT
        CloseHandle( hevKeyMapInit ) ;
    hevKeyMapInit = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevKeyMapInit == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevKeyMapInit ) ;
    return DosCreateEventSem( NULL, &hevKeyMapInit, 0, posted ) ;
#endif /* NT */
}

APIRET
PostKeyMapInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevKeyMapInit ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevKeyMapInit ) ;
#endif /* NT */
}

APIRET
WaitKeyMapInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevKeyMapInit, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevKeyMapInit, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetKeyMapInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevKeyMapInit, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevKeyMapInit ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0;

    rc = DosWaitEventSem( hevKeyMapInit, timo ) ;
    if ( !rc )
        DosResetEventSem( hevKeyMapInit, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetKeyMapInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevKeyMapInit ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevKeyMapInit, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseKeyMapInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevKeyMapInit ) ;
    hevKeyMapInit = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevKeyMapInit ) ;
    hevKeyMapInit = 0 ;
    return rc ;
#endif /* NT */
}

#ifdef NT
void CALLBACK
TimeProc(
    UINT IDEvent,
    UINT uReserved,
    DWORD dwUser,
    DWORD dwReserved1,
    DWORD dwReserved2 )
{
    if (IDEvent == htimVscrn[0]) {
        PostVscrnTimerSem(0);
        }
    if (IDEvent == htimVscrn[1]) {
        PostVscrnTimerSem(1);
        }
    if (IDEvent == htimVscrn[2]) {
        PostVscrnTimerSem(2);
        }
    else if (IDEvent == htimAlarm) {
        PostAlarmSem() ;
        }
}
#endif /* NT */

APIRET
StartVscrnTimer( ULONG interval )
{
    int i = 0 ;
    for ( i=0;i<VNUM;i++ )
    {
#ifdef NT

        if ( isWin95() )
        {
            htimVscrn[i] = ckTimerStart( (UINT) interval,
                                      (UINT) interval/20,
                                      &TimeProc,
                                      0, TIME_PERIODIC ) ;
        }
        else
        {
            htimVscrn[i] = timeSetEvent( (UINT) interval,
                                         (UINT) interval/20,
                                         &TimeProc,
                                         0, TIME_PERIODIC ) ;
        }
        if ( htimVscrn[i] == 0 )
            return -(i+1);
#else /* not NT */
        if ( DosStartTimer( interval,
                       (HSEM) hevVscrnTimer[i], &htimVscrn[i] ))
            return -(i+1);
#endif /* NT */
    }
    return 0;
}

APIRET
StopVscrnTimer( void )
{
    int i ;
    APIRET rc = 0 ;
    for ( i=0;i<VNUM;i++ )
    {
#ifdef NT
        if ( isWin95() )
        {
            rc = ckTimerKill( htimVscrn[i] ) ;
        }
        else
        {
            rc = timeKillEvent( htimVscrn[i] ) ;
        }
#else /* not NT */
        rc = DosStopTimer( htimVscrn[i] ) ;
#endif /* NT */
        htimVscrn[i] = 0 ;
    }
    return rc;
}

APIRET
StartAlarmTimer( ULONG interval )
{
#ifdef NT
   if ( isWin95() )
   {
      htimAlarm = ckTimerStart( (UINT) interval,(UINT) interval/20,
        &TimeProc, 0, TIME_ONESHOT ) ;
   }
   else
   {
      htimAlarm = timeSetEvent( (UINT) interval,(UINT) interval/20,
        &TimeProc, 0, TIME_ONESHOT ) ;
   }
    return htimAlarm == 0 ? 1 : 0 ;
#else /* not NT */
    ResetAlarmSem() ;
    return DosAsyncTimer( interval, (HSEM) hevAlarm, &htimAlarm ) ;
#endif /* NT */
}

APIRET
StopAlarmTimer( void )
{
#ifdef NT
    APIRET rc = 0 ;
    if ( isWin95() )
    {
        rc = ckTimerKill( htimAlarm ) ;
    }
    else
    {
        rc = timeKillEvent( htimAlarm ) ;
    }
    htimAlarm = 0 ;
    return rc ;
#else /* not NT */
    APIRET rc ;
    rc = DosStopTimer( htimAlarm ) ;
    htimAlarm = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateVscrnTimerSem( BOOL posted )
{
   int i, rc = 0 ;
   for ( i=0; i<VNUM ; i++ )
   {
    if ( hevVscrnTimer[i] )
#ifdef NT
        CloseHandle( hevVscrnTimer[i] ) ;
       hevVscrnTimer[i] = CreateEvent( NULL, TRUE, posted, NULL ) ;
       if (hevVscrnTimer[i] == NULL)
           return GetLastError() ;
#else /* not NT */
       DosCloseEventSem( hevVscrnTimer[i] ) ;
       rc = DosCreateEventSem( NULL, &hevVscrnTimer[i],
                               DC_SEM_SHARED, posted ) ;
       if ( rc )
           return rc ;
#endif /* NT */
   }
   return 0;
}

APIRET
PostVscrnTimerSem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevVscrnTimer[vmode] ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevVscrnTimer[vmode] ) ;
#endif /* NT */
}

APIRET
WaitVscrnTimerSem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevVscrnTimer[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevVscrnTimer[vmode], timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetVscrnTimerSem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevVscrnTimer[vmode], timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevVscrnTimer[vmode] ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevVscrnTimer[vmode], timo ) ;
    if ( !rc )
        DosResetEventSem( hevVscrnTimer[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetVscrnTimerSem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevVscrnTimer[vmode] ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevVscrnTimer[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseVscrnTimerSem( void )
{
   int i,rc=0 ;

   for ( i=0; i<VNUM ; i++ )
   {
#ifdef NT
    rc = CloseHandle( hevVscrnTimer[i] ) ;
    hevVscrnTimer[i] = (HANDLE) NULL ;
    if (!rc)
       return GetLastError() ;
#else /* not NT */
    rc = DosCloseEventSem( hevVscrnTimer[i] ) ;
    hevVscrnTimer[i] = 0 ;
    if (rc)
       return rc ;
#endif /* NT */
   }
   return 0;
}

APIRET
CreateVscrnDirtySem( BOOL posted )
{
   int i, rc = 0 ;
   for ( i=0 ; i<VNUM ; i++ )
   {
    if ( hevVscrnDirty[i] )
#ifdef NT
        CloseHandle( hevVscrnDirty[i] ) ;
    hevVscrnDirty[i] = CreateEvent( NULL, TRUE, posted, NULL ) ;
    if (hevVscrnDirty[i] == NULL)
       return GetLastError();
#else /* not NT */
        DosCloseEventSem( hevVscrnDirty[i] ) ;
    rc = DosCreateEventSem( NULL, &hevVscrnDirty[i], DC_SEM_SHARED, posted ) ;
      if ( rc )
         return rc ;
#endif /* NT */
   }
   return 0;
}

APIRET
PostVscrnDirtySem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevVscrnDirty[vmode] ) ;
#ifdef COMMENT
    debug(F101,"PostVscrnDirtySem rc","",rc) ;
#endif
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevVscrnDirty[vmode] ) ;
#endif /* NT */
}

APIRET
WaitVscrnDirtySem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevVscrnDirty[vmode], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevVscrnDirty[vmode], timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetVscrnDirtySem( int vmode, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevVscrnDirty[vmode], timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevVscrnDirty[vmode] ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevVscrnDirty[vmode], timo ) ;
    if ( !rc )
        DosResetEventSem( hevVscrnDirty[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetVscrnDirtySem( int vmode )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevVscrnDirty[vmode] ) ;
#ifdef COMMENT
    debug(F101,"ResetVscrnDirtySem rc","",rc) ;
    if ( !rc )
       debug(F101,"ResetVscrnDirtySem lasterror","",GetLastError()) ;
#endif /* COMMENT */
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevVscrnDirty[vmode], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseVscrnDirtySem( void )
{
   int i, rc = 0 ;
   for ( i=0 ; i<VNUM ; i++ )
   {
#ifdef NT
    rc = CloseHandle( hevVscrnDirty[i] ) ;
    hevVscrnDirty[i] = (HANDLE) NULL ;
    if (!rc)
       return GetLastError() ;
#else /* not NT */
    rc = DosCloseEventSem( hevVscrnDirty[i] ) ;
    hevVscrnDirty[i] = 0 ;
    if (rc)
       return rc ;
#endif /* NT */
   }
   return 0;
}

APIRET
CreateVscrnMuxWait( int vmode )
{
#ifdef NT
    if ( hevVscrnDirty[vmode] == NULL ||
         hevVscrnTimer[vmode] == NULL )
        return 1 ;
    hevVscrnUpdate[vmode][0] = hevVscrnDirty[vmode] ;
    hevVscrnUpdate[vmode][1] = hevVscrnTimer[vmode] ;
    return 0 ;
#else /* not NT */
#define VSCRN_DIRTY 1
#define VSCRN_TIMER 2
    static SEMRECORD upd_scr_rec[VNUM][2] ;
    APIRET rc ;

    upd_scr_rec[vmode][0].hsemCur = (HSEM) hevVscrnDirty[vmode] ;
    upd_scr_rec[vmode][0].ulUser  = VSCRN_DIRTY ;
    upd_scr_rec[vmode][1].hsemCur = (HSEM) hevVscrnTimer[vmode] ;
    upd_scr_rec[vmode][1].ulUser  = VSCRN_TIMER ;
    if ( hmuxVscrnUpdate[vmode] )
        DosCloseMuxWaitSem( hmuxVscrnUpdate[vmode] ) ;
    rc = DosCreateMuxWaitSem( NULL, &hmuxVscrnUpdate[vmode], 2, upd_scr_rec[vmode],
        DCMW_WAIT_ALL | DC_SEM_SHARED );
    return rc ;
#endif /* NT */
}

APIRET
WaitVscrnMuxWait( int vmode, ULONG timo )
{
#ifdef NT
    APIRET rc = 0 ;
    rc = WaitForMultipleObjectsEx( 2, hevVscrnUpdate[vmode], TRUE, timo, TRUE ) ;
#ifdef COMMENT
    debug(F101,"WaitVscrnMuxWait waitformultipleobjects rc","",rc);
#endif /* COMMENT */
    return ( rc == WAIT_OBJECT_0 || rc == WAIT_OBJECT_0 + 1 )? rc+1 : 0 ;
#else /* not NT */
    APIRET rc ;
    ULONG semid = 0;
    rc = DosWaitMuxWaitSem( hmuxVscrnUpdate[vmode], timo, &semid ) ;
    return semid ;
#endif /* NT */
}

APIRET
CloseVscrnMuxWait( int vmode )
{
#ifdef NT
    hevVscrnUpdate[vmode][0] = NULL ;
    hevVscrnUpdate[vmode][1] = NULL ;
    return 0 ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMuxWaitSem( hmuxVscrnUpdate[vmode] ) ;
    hmuxVscrnUpdate[vmode] = 0 ;
    return rc ;
#endif /* NT */
}

/* Process and Thread management */

APIRET
ResetThreadPrty( void )
{
#ifdef NT
   SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL ) ;
#else
    DosSetPriority( PRTYS_THREAD, XYP_REG, PRTYD_MINIMUM, 0 ) ;
#endif
   return 0;
}

APIRET
SetThreadPrty( int priority, int class )
{
#ifdef NT
    switch ( priority ) {
    case XYP_IDLE:
        class -= 3;
        break;
    case XYP_SRV:
        class += 3;
        break;
    case XYP_RTP:
        class += 6;
        break;
    }
    SetThreadPriority( GetCurrentThread(), class ) ;
    return 0;
#else /* not NT */
    DosSetPriority( PRTYS_THREAD, priority, PRTYD_MINIMUM, 0 ) ;
    return DosSetPriority( PRTYS_THREAD, priority, class, 0 ) ;
#endif /* NT */
}

APIRET
KillProcess( int pid )
{
#ifdef NT
        HANDLE process ;
    debug(F101,"KillProcess pid","",pid);
    process = OpenProcess( PROCESS_TERMINATE, FALSE, pid ) ;
        TerminateProcess( process, 1 ) ;
        CloseHandle( process ) ;
    return 0;
#else /* NT */
    debug(F100,"KillProcess","",0);
    return DosKillProcess(DKP_PROCESS, pid);
#endif /* NT */
}

#ifdef NT
APIRET
CreateSerialMutex( BOOL owned )
{
    if ( hmtxSerial )
#ifdef NT
        CloseHandle( hmtxSerial ) ;
    hmtxSerial = CreateMutex( NULL, owned, NULL ) ;
    return hmtxSerial == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxSerial ) ;
    return DosCreateMutexSem( NULL, &hmtxSerial, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestSerialMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxSerial, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxSerial, timo ) ;
#endif /* NT */
}

APIRET
ReleaseSerialMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxSerial ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxSerial ) ;
#endif /* NT */
}

APIRET
CloseSerialMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxSerial ) ;
    hmtxSerial = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxSerial ) ;
    hmtxSerial = 0 ;
    return rc ;
#endif /* NT */
}
#endif /* NT */
APIRET
CreateThreadMgmtMutex( BOOL owned )
{
    if ( hmtxThreadMgmt )
#ifdef NT
        CloseHandle( hmtxThreadMgmt ) ;
    hmtxThreadMgmt = CreateMutex( NULL, owned, NULL ) ;
    return hmtxThreadMgmt == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxThreadMgmt ) ;
    return DosCreateMutexSem( NULL, &hmtxThreadMgmt, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestThreadMgmtMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxThreadMgmt, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxThreadMgmt, timo ) ;
#endif /* NT */
}

APIRET
ReleaseThreadMgmtMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxThreadMgmt ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxThreadMgmt ) ;
#endif /* NT */
}

APIRET
CloseThreadMgmtMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxThreadMgmt ) ;
    hmtxThreadMgmt = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxThreadMgmt ) ;
    hmtxThreadMgmt = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateCtrlCSem( BOOL posted, ULONG * index )
{
    APIRET rc = 0 ;
    if ( CtrlCCount == MAXCTRLC ) {
        *index = -1 ;
        return -1;
    }
    CtrlCCount++ ;
    if ( hevCtrlC[CtrlCCount] )
#ifdef NT
        CloseHandle( hevCtrlC[CtrlCCount] ) ;
    hevCtrlC[CtrlCCount] = CreateEvent( NULL, TRUE, posted, NULL ) ;
    if ( hevCtrlC[CtrlCCount] == NULL )
    {
        CtrlCCount-- ;
        rc = GetLastError() ;
    }
#else /* not NT */
        DosCloseEventSem( hevCtrlC[CtrlCCount] ) ;
    rc = DosCreateEventSem( NULL, &hevCtrlC[CtrlCCount], DC_SEM_SHARED, posted ) ;
    if ( rc != 0 )
        CtrlCCount-- ;
#endif /* NT */
    *index = CtrlCCount ;
    return rc ;
}

APIRET
PostCtrlCSem(void)
{
    APIRET rc = 0 ;
    if ( CtrlCCount < 0 )
        return -1 ;
#ifdef NT
    rc = SetEvent( hevCtrlC[CtrlCCount] ) == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    rc = DosPostEventSem( hevCtrlC[CtrlCCount] ) ;
#endif /* NT */
    return rc ;
}

APIRET
WaitCtrlCSem( ULONG index, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevCtrlC[index], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevCtrlC[index], timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetCtrlCSem( ULONG index, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevCtrlC[index], timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevCtrlC[index] ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevCtrlC[index], timo ) ;
    if ( !rc )
        DosResetEventSem( hevCtrlC[index], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetCtrlCSem( ULONG index )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevCtrlC[index] ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevCtrlC[index], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseCtrlCSem( ULONG index )
{
    APIRET rc = 0 ;
    if ( index != CtrlCCount )
        return -1;
#ifdef NT
    rc = CloseHandle( hevCtrlC[index] ) == TRUE ? 0 : GetLastError() ;
    hevCtrlC[index] = (HANDLE) NULL ;
#else /* not NT */
    rc = DosCloseEventSem( hevCtrlC[index] ) ;
    hevCtrlC[index] = 0 ;
#endif /* NT */
    CtrlCCount-- ;
    return rc;
}

APIRET
CreateCtrlCMuxWait( ULONG ccindex,
#ifdef NT
                  HANDLE hevThread
#else /* NT */
                  HEV hevThread
#endif /* NT */
                  )
{
#ifdef NT
    if ( hevCtrlC[ccindex] == NULL || hevThread == NULL )
        return -1 ;
    hmuxCtrlC[ccindex][0] = hevCtrlC[ccindex] ;
    hmuxCtrlC[ccindex][1] = hevThread ;
    return 0 ;
#else /* not NT */
#define CTRL_C 1
#define THREAD 2
    SEMRECORD upd_scr_rec[2] ;
    APIRET rc ;

    upd_scr_rec[0].hsemCur = (HSEM) hevCtrlC[ccindex] ;
    upd_scr_rec[0].ulUser  = CTRL_C ;
    upd_scr_rec[1].hsemCur = (HSEM) hevThread ;
    upd_scr_rec[1].ulUser  = THREAD ;
    if ( hmuxCtrlC[ccindex] )
        DosCloseMuxWaitSem( hmuxCtrlC[ccindex] ) ;
    rc = DosCreateMuxWaitSem( NULL, &hmuxCtrlC[ccindex], 2, upd_scr_rec,
        DCMW_WAIT_ANY | DC_SEM_SHARED );
    return rc ;
#endif /* NT */
}

APIRET
WaitCtrlCMuxWait( ULONG ccindex, ULONG timo )
{
#ifdef NT
    APIRET rc = 0 ;
    rc = WaitForMultipleObjectsEx( 2, hmuxCtrlC[ccindex], FALSE, timo, TRUE ) ;
    return ( rc == WAIT_OBJECT_0 || rc == WAIT_OBJECT_0 + 1 )
        ? rc - WAIT_OBJECT_0 + 1 : 0 ;
#else /* not NT */
    APIRET rc ;
    ULONG semid = 0;
    do
    {
        rc = DosWaitMuxWaitSem( hmuxCtrlC[ccindex], timo, &semid ) ;
    }
    while ( rc == ERROR_INTERRUPT );
    return semid ;
#endif /* NT */
}

APIRET
CloseCtrlCMuxWait( ULONG ccindex )
{
#ifdef NT
    hmuxCtrlC[ccindex][0] = NULL ;
    hmuxCtrlC[ccindex][1] = NULL ;
    return 0 ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMuxWaitSem( hmuxCtrlC[ccindex] ) ;
    hmuxCtrlC[ccindex] = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateAlarmSigSem( BOOL posted, ULONG * index )
{
    APIRET rc = 0 ;
    if ( AlarmSigCount == MAXALARMSIG ) {
        *index = -1 ;
        return -1;
    }
    AlarmSigCount++ ;
    if ( hevAlarmSig[AlarmSigCount] )
#ifdef NT
        CloseHandle( hevAlarmSig[AlarmSigCount] ) ;
    hevAlarmSig[AlarmSigCount] = CreateEvent( NULL, TRUE, posted, NULL ) ;
    if ( hevAlarmSig[AlarmSigCount] == NULL )
    {
        AlarmSigCount-- ;
        rc = GetLastError() ;
    }
#else /* not NT */
        DosCloseEventSem( hevAlarmSig[AlarmSigCount] ) ;
    rc = DosCreateEventSem( NULL, &hevAlarmSig[AlarmSigCount], DC_SEM_SHARED, posted ) ;
    if ( rc != 0 )
        AlarmSigCount-- ;
#endif /* NT */
    *index = AlarmSigCount ;
    debug(F101,"CreateAlarmSigSem AlarmSigCount","",AlarmSigCount) ;
    return rc ;
}

APIRET
PostAlarmSigSem(void)
{
    APIRET rc = 0 ;
    debug(F101,"PostAlarmSigSem AlarmSigCount","",AlarmSigCount) ;
    if ( AlarmSigCount < 0 )
        return -1 ;
#ifdef NT
    rc = SetEvent( hevAlarmSig[AlarmSigCount] ) == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    rc = DosPostEventSem( hevAlarmSig[AlarmSigCount] ) ;
#endif /* NT */
    return rc ;
}

APIRET
WaitAlarmSigSem( ULONG index, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevAlarmSig[index], timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevAlarmSig[index], timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetAlarmSigSem( ULONG index, ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevAlarmSig[index], timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevAlarmSig[index] ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevAlarmSig[index], timo ) ;
    if ( !rc )
        DosResetEventSem( hevAlarmSig[index], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetAlarmSigSem( ULONG index )
{
#ifdef NT
    BOOL rc = 0 ;
    debug(F101,"ResetAlarmSigSem index","",index) ;
    rc = ResetEvent( hevAlarmSig[index] ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevAlarmSig[index], &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseAlarmSigSem( ULONG index )
{
    APIRET rc = 0 ;
    debug(F101,"CloseAlarmSigSem index","",index) ;
    if ( index != AlarmSigCount )
        return -1;
#ifdef NT
    rc = CloseHandle( hevAlarmSig[index] ) == TRUE ? 0 : GetLastError() ;
    hevAlarmSig[index] = (HANDLE) NULL ;
#else /* not NT */
    rc = DosCloseEventSem( hevAlarmSig[index] ) ;
    hevAlarmSig[index] = 0 ;
#endif /* NT */
    AlarmSigCount-- ;
    return rc;
}

APIRET
CreateAlarmSigMuxWait( ULONG alrmindex,
#ifdef NT
                  HANDLE hevThread
#else /* NT */
                  HEV hevThread
#endif /* NT */
                  )
{
#ifdef NT
    if ( hevAlarmSig[alrmindex] == NULL || hevThread == NULL )
        return -1 ;
    hmuxAlarmSig[alrmindex][0] = hevAlarmSig[alrmindex] ;
    hmuxAlarmSig[alrmindex][1] = hevThread ;
    return 0 ;
#else /* not NT */
#define ALRM 1
#define THREAD 2
    SEMRECORD upd_scr_rec[2] ;
    APIRET rc ;

    upd_scr_rec[0].hsemCur = (HSEM) hevAlarmSig[alrmindex] ;
    upd_scr_rec[0].ulUser  = ALRM ;
    upd_scr_rec[1].hsemCur = (HSEM) hevThread ;
    upd_scr_rec[1].ulUser  = THREAD ;
    if ( hmuxAlarmSig[alrmindex] )
        DosCloseMuxWaitSem( hmuxAlarmSig[alrmindex] ) ;
    rc = DosCreateMuxWaitSem( NULL, &hmuxAlarmSig[alrmindex], 2, upd_scr_rec,
        DCMW_WAIT_ANY | DC_SEM_SHARED );
    return rc ;
#endif /* NT */
}

APIRET
WaitAlarmSigMuxWait( ULONG alrmindex, ULONG timo )
{
#ifdef NT
    APIRET rc = 0 ;
    rc = WaitForMultipleObjectsEx( 2, hmuxAlarmSig[alrmindex], FALSE, timo, TRUE ) ;
    return ( rc == WAIT_OBJECT_0 || rc == WAIT_OBJECT_0 + 1 )
        ? rc - WAIT_OBJECT_0 + 1 : 0 ;
#else /* not NT */
    APIRET rc ;
    ULONG semid = 0;
    do
    {
        rc = DosWaitMuxWaitSem( hmuxAlarmSig[alrmindex], timo, &semid ) ;
    }
    while ( rc == ERROR_INTERRUPT );
    return semid ;
#endif /* NT */
}

APIRET
CloseAlarmSigMuxWait( ULONG alrmindex )
{
#ifdef NT
    hmuxAlarmSig[alrmindex][0] = NULL ;
    hmuxAlarmSig[alrmindex][1] = NULL ;
    return 0 ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMuxWaitSem( hmuxAlarmSig[alrmindex] ) ;
    hmuxAlarmSig[alrmindex] = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateCtrlCAlarmSigMuxWait( ULONG ccindex, ULONG alrmindex,
#ifdef NT
                  HANDLE hevThread
#else /* NT */
                  HEV hevThread
#endif /* NT */
                  )
{
#ifdef NT
    if ( hevCtrlC[ccindex] == NULL ||
                 hevAlarmSig[alrmindex] == NULL ||
                 hevThread == NULL )
        return -1 ;
    hmuxCtrlCAlarmSig[0] = hevCtrlC[ccindex] ;
    hmuxCtrlCAlarmSig[1] = hevAlarmSig[alrmindex] ;
    hmuxCtrlCAlarmSig[2] = hevThread ;
    return 0 ;
#else /* not NT */
    SEMRECORD upd_scr_rec[3] ;
    APIRET rc ;

    upd_scr_rec[0].hsemCur = (HSEM) hevCtrlC[ccindex] ;
    upd_scr_rec[0].ulUser  = 1;
    upd_scr_rec[1].hsemCur = (HSEM) hevAlarmSig[alrmindex] ;
    upd_scr_rec[1].ulUser  = 1;
    upd_scr_rec[2].hsemCur = (HSEM) hevThread ;
    upd_scr_rec[2].ulUser  = 2 ;
    if ( hmuxCtrlCAlarmSig )
        DosCloseMuxWaitSem( hmuxCtrlCAlarmSig ) ;
    rc = DosCreateMuxWaitSem( NULL, &hmuxCtrlCAlarmSig, 3, upd_scr_rec,
        DCMW_WAIT_ANY | DC_SEM_SHARED );
    return rc ;
#endif /* NT */
}

APIRET
WaitCtrlCAlarmSigMuxWait( ULONG ccindex, ULONG alrmindex, ULONG timo )
{
#ifdef NT
    APIRET rc = 0 ;
    rc = WaitForMultipleObjectsEx( 3, hmuxCtrlCAlarmSig, FALSE, timo, TRUE ) ;
    return ( rc == WAIT_OBJECT_0 || rc == WAIT_OBJECT_0 + 1 || rc == WAIT_OBJECT_0 + 2 )
        ? rc - WAIT_OBJECT_0 + 1 : 0 ;
#else /* not NT */
    APIRET rc ;
    ULONG semid = 0;
    do
    {
        rc = DosWaitMuxWaitSem( hmuxCtrlCAlarmSig, timo, &semid ) ;
    }
    while ( rc == ERROR_INTERRUPT );
    return semid ;
#endif /* NT */
}

APIRET
CloseCtrlCAlarmSigMuxWait( ULONG ccindex, ULONG alrmindex )
{
#ifdef NT
    hmuxCtrlCAlarmSig[0] = NULL ;
    hmuxCtrlCAlarmSig[1] = NULL ;
    hmuxCtrlCAlarmSig[2] = NULL ;
   return 0 ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMuxWaitSem( hmuxCtrlCAlarmSig ) ;
    hmuxCtrlCAlarmSig = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
#ifdef NT
PostSem( HANDLE hev )
#else /* NT */
PostSem( HEV hev )
#endif /* NT */
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hev ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hev ) ;
#endif /* NT */
}

APIRET
#ifdef NT
WaitSem( HANDLE hev, ULONG timo )
#else /* NT */
WaitSem( HEV hev, ULONG timo )
#endif /* NT */
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hev, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hev, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
#ifdef NT
WaitAndResetSem( HANDLE hev, ULONG timo )
#else /* NT */
WaitAndResetSem( HEV hev, ULONG timo )
#endif /* NT */
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hev, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hev ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hev, timo ) ;
    if ( !rc )
        DosResetEventSem( hev, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
#ifdef NT
ResetSem( HANDLE hev )
#else /* NT */
ResetSem( HEV hev )
#endif /* NT */
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hev ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hev, &semcount ) ;
    return semcount ;
#endif /* NT */
}

#ifdef CK_TAPI
APIRET
CreateTAPIConnectSem( BOOL posted )
{
    if ( hevTAPIConnect )
#ifdef NT
        CloseHandle( hevTAPIConnect ) ;
    hevTAPIConnect = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTAPIConnect == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevTAPIConnect ) ;
    return DosCreateEventSem( NULL, &hevTAPIConnect, 0, posted ) ;
#endif /* NT */
}

APIRET
PostTAPIConnectSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTAPIConnect ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTAPIConnect ) ;
#endif /* NT */
}

APIRET
WaitTAPIConnectSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIConnect, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTAPIConnect, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTAPIConnectSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIConnect, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTAPIConnect ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTAPIConnect, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTAPIConnect, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTAPIConnectSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTAPIConnect ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTAPIConnect, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseTAPIConnectSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTAPIConnect ) ;
    hevTAPIConnect = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTAPIConnect ) ;
    hevTAPIConnect = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateTAPIAnswerSem( BOOL posted )
{
    if ( hevTAPIAnswer )
#ifdef NT
        CloseHandle( hevTAPIAnswer ) ;
    hevTAPIAnswer = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTAPIAnswer == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevTAPIAnswer ) ;
    return DosCreateEventSem( NULL, &hevTAPIAnswer, 0, posted ) ;
#endif /* NT */
}

APIRET
PostTAPIAnswerSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTAPIAnswer ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTAPIAnswer ) ;
#endif /* NT */
}

APIRET
WaitTAPIAnswerSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIAnswer, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTAPIAnswer, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTAPIAnswerSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIAnswer, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTAPIAnswer ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTAPIAnswer, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTAPIAnswer, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTAPIAnswerSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTAPIAnswer ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTAPIAnswer, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseTAPIAnswerSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTAPIAnswer ) ;
    hevTAPIAnswer = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTAPIAnswer ) ;
    hevTAPIAnswer = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateTAPIInitSem( BOOL posted )
{
    if ( hevTAPIInit )
#ifdef NT
        CloseHandle( hevTAPIInit ) ;
    hevTAPIInit = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTAPIInit == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevTAPIInit ) ;
    return DosCreateEventSem( NULL, &hevTAPIInit, 0, posted ) ;
#endif /* NT */
}

APIRET
PostTAPIInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTAPIInit ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTAPIInit ) ;
#endif /* NT */
}

APIRET
WaitTAPIInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIInit, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTAPIInit, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTAPIInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTAPIInit, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTAPIInit ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTAPIInit, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTAPIInit, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTAPIInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTAPIInit ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTAPIInit, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseTAPIInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTAPIInit ) ;
    hevTAPIInit = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTAPIInit ) ;
    hevTAPIInit = 0 ;
    return rc ;
#endif /* NT */
}
#endif /* CK_TAPI */

APIRET
CreateRichEditInitSem( BOOL posted )
{
    if ( hevRichEditInit )
#ifdef NT
        CloseHandle( hevRichEditInit ) ;
    hevRichEditInit = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevRichEditInit == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevRichEditInit ) ;
    return DosCreateEventSem( NULL, &hevRichEditInit, 0, posted ) ;
#endif /* NT */
}

APIRET
PostRichEditInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevRichEditInit ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevRichEditInit ) ;
#endif /* NT */
}

APIRET
WaitRichEditInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRichEditInit, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevRichEditInit, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetRichEditInitSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRichEditInit, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevRichEditInit ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevRichEditInit, timo ) ;
    if ( !rc )
        DosResetEventSem( hevRichEditInit, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetRichEditInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevRichEditInit ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevRichEditInit, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseRichEditInitSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevRichEditInit ) ;
    hevRichEditInit = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevRichEditInit ) ;
    hevRichEditInit = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateRichEditCloseSem( BOOL posted )
{
    if ( hevRichEditClose )
#ifdef NT
        CloseHandle( hevRichEditClose ) ;
    hevRichEditClose = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevRichEditClose == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevRichEditClose ) ;
    return DosCreateEventSem( NULL, &hevRichEditClose, 0, posted ) ;
#endif /* NT */
}

APIRET
PostRichEditCloseSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevRichEditClose ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevRichEditClose ) ;
#endif /* NT */
}

APIRET
WaitRichEditCloseSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRichEditClose, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevRichEditClose, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetRichEditCloseSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevRichEditClose, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevRichEditClose ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevRichEditClose, timo ) ;
    if ( !rc )
        DosResetEventSem( hevRichEditClose, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetRichEditCloseSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevRichEditClose ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevRichEditClose, &semcount ) ;
    return rc ;
#endif /* NT */
}

APIRET
CloseRichEditCloseSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevRichEditClose ) ;
    hevRichEditClose = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevRichEditClose ) ;
    hevRichEditClose = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateRichEditMutex( BOOL owned )
{
    if ( hmtxRichEdit )
#ifdef NT
        CloseHandle( hmtxRichEdit ) ;
    hmtxRichEdit = CreateMutex( NULL, owned, NULL ) ;
    return hmtxRichEdit == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseMutexSem( hmtxRichEdit ) ;
    return DosCreateMutexSem( NULL, &hmtxRichEdit, 0, owned ) ;
#endif /* NT */
}

APIRET
RequestRichEditMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxRichEdit, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxRichEdit, timo ) ;
#endif /* NT */
}

APIRET
ReleaseRichEditMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxRichEdit ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxRichEdit ) ;
#endif /* NT */
}

APIRET
CloseRichEditMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxRichEdit ) ;
    hmtxRichEdit = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxRichEdit ) ;
    hmtxRichEdit = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateLocalEchoMutex( BOOL owned )
{
    if ( hmtxLocalEcho )
#ifdef NT
        CloseHandle( hmtxLocalEcho ) ;
    hmtxLocalEcho = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxLocalEcho == NULL)
        return GetLastError();
#else /* not NT */
    DosCloseMutexSem( hmtxLocalEcho ) ;
    DosCreateMutexSem( NULL, &hmtxLocalEcho, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestLocalEchoMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxLocalEcho, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxLocalEcho, timo ) ;
#endif /* NT */
}

APIRET
ReleaseLocalEchoMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxLocalEcho ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxLocalEcho ) ;
#endif /* NT */
}

APIRET
CloseLocalEchoMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxLocalEcho ) ;
    hmtxLocalEcho = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxLocalEcho ) ;
    hmtxLocalEcho = 0 ;
    return rc ;
#endif /* NT */
}


APIRET
CreateLocalEchoAvailSem( BOOL posted )
{
    if ( hevLocalEchoAvail )
#ifdef NT
        CloseHandle( hevLocalEchoAvail ) ;
    hevLocalEchoAvail = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevLocalEchoAvail == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevLocalEchoAvail ) ;
    return DosCreateEventSem( NULL, &hevLocalEchoAvail, DC_SEM_SHARED, posted ) ;
#endif /* NT */
}

APIRET
PostLocalEchoAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevLocalEchoAvail ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevLocalEchoAvail ) ;
#endif /* NT */
}

APIRET
WaitLocalEchoAvailSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevLocalEchoAvail, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevLocalEchoAvail, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetLocalEchoAvailSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevLocalEchoAvail, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevLocalEchoAvail ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevLocalEchoAvail, timo ) ;
    if ( !rc )
        DosResetEventSem( hevLocalEchoAvail, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetLocalEchoAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevLocalEchoAvail ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevLocalEchoAvail, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseLocalEchoAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevLocalEchoAvail ) ;
    hevLocalEchoAvail = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevLocalEchoAvail ) ;
    hevLocalEchoAvail = 0 ;
    return rc ;
#endif /* NT */
}

#ifdef NETCMD
APIRET
CreateNetCmdMutex( BOOL owned )
{
    if ( hmtxNetCmd )
#ifdef NT
        CloseHandle( hmtxNetCmd ) ;
    hmtxNetCmd = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxNetCmd == NULL)
        return GetLastError();
#else /* not NT */
    DosCloseMutexSem( hmtxNetCmd ) ;
    DosCreateMutexSem( NULL, &hmtxNetCmd, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestNetCmdMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxNetCmd, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxNetCmd, timo ) ;
#endif /* NT */
}

APIRET
ReleaseNetCmdMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxNetCmd ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxNetCmd ) ;
#endif /* NT */
}

APIRET
CloseNetCmdMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxNetCmd ) ;
    hmtxNetCmd = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxNetCmd ) ;
    hmtxNetCmd = 0 ;
    return rc ;
#endif /* NT */
}


APIRET
CreateNetCmdAvailSem( BOOL posted )
{
    if ( hevNetCmdAvail )
#ifdef NT
        CloseHandle( hevNetCmdAvail ) ;
    hevNetCmdAvail = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevNetCmdAvail == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevNetCmdAvail ) ;
    return DosCreateEventSem( NULL, &hevNetCmdAvail, DC_SEM_SHARED, posted ) ;
#endif /* NT */
}

APIRET
PostNetCmdAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevNetCmdAvail ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevNetCmdAvail ) ;
#endif /* NT */
}

APIRET
WaitNetCmdAvailSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevNetCmdAvail, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevNetCmdAvail, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetNetCmdAvailSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevNetCmdAvail, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevNetCmdAvail ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevNetCmdAvail, timo ) ;
    if ( !rc )
        DosResetEventSem( hevNetCmdAvail, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetNetCmdAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevNetCmdAvail ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevNetCmdAvail, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseNetCmdAvailSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevNetCmdAvail ) ;
    hevNetCmdAvail = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevNetCmdAvail ) ;
    hevNetCmdAvail = 0 ;
    return rc ;
#endif /* NT */
}
#endif /* NETCMD */

APIRET
CreateTCPIPMutex( BOOL owned )
{
    if ( hmtxTCPIP )
#ifdef NT
        CloseHandle( hmtxTCPIP ) ;
    hmtxTCPIP = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxTCPIP == NULL)
        return GetLastError();
#else /* not NT */
    DosCloseMutexSem( hmtxTCPIP ) ;
    DosCreateMutexSem( NULL, &hmtxTCPIP, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestTCPIPMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxTCPIP, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxTCPIP, timo ) ;
#endif /* NT */
}

APIRET
ReleaseTCPIPMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxTCPIP ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxTCPIP ) ;
#endif /* NT */
}

APIRET
CloseTCPIPMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxTCPIP ) ;
    hmtxTCPIP = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxTCPIP ) ;
    hmtxTCPIP = 0 ;
    return rc ;
#endif /* NT */
}

APIRET
CreateCommMutex( BOOL owned )
{
    if ( hmtxComm )
#ifdef NT
        CloseHandle( hmtxComm ) ;
    hmtxComm = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxComm == NULL)
        return GetLastError();
#else /* not NT */
    DosCloseMutexSem( hmtxComm ) ;
    DosCreateMutexSem( NULL, &hmtxComm, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestCommMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxComm, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxComm, timo ) ;
#endif /* NT */
}

APIRET
ReleaseCommMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxComm ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxComm ) ;
#endif /* NT */
}

APIRET
CloseCommMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxComm ) ;
    hmtxComm = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxComm ) ;
    hmtxComm = 0 ;
    return rc ;
#endif /* NT */
}

#ifdef CK_SSL
APIRET
CreateSSLMutex( BOOL owned )
{
    if ( hmtxSSL )
#ifdef NT
        CloseHandle( hmtxSSL ) ;
    hmtxSSL = CreateMutex( NULL, owned, NULL ) ;
    if (hmtxSSL == NULL)
        return GetLastError();
#else /* not NT */
    DosCloseMutexSem( hmtxSSL ) ;
    DosCreateMutexSem( NULL, &hmtxSSL, 0, owned ) ;
#endif /* NT */
    return 0;
}

APIRET
RequestSSLMutex( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hmtxSSL, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    return DosRequestMutexSem( hmtxSSL, timo ) ;
#endif /* NT */
}

APIRET
ReleaseSSLMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ReleaseMutex( hmtxSSL ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosReleaseMutexSem( hmtxSSL ) ;
#endif /* NT */
}

APIRET
CloseSSLMutex( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hmtxSSL ) ;
    hmtxSSL = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseMutexSem( hmtxSSL ) ;
    hmtxSSL = 0 ;
    return rc ;
#endif /* NT */
}
#endif /* CK_SSL */


APIRET
CreateTerminalModeSem( BOOL posted )
{
#ifdef NT
    if ( hevTerminalMode )
        CloseHandle( hevTerminalMode ) ;
    hevTerminalMode = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevTerminalMode == NULL ? GetLastError() : 0 ;
#else /* not NT */
    if ( hevTerminalMode )
        DosCloseEventSem( hevTerminalMode ) ;
    return DosCreateEventSem( NULL, &hevTerminalMode, DC_SEM_SHARED, posted ) ;
#endif /* NT */
}

APIRET
PostTerminalModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevTerminalMode ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevTerminalMode ) ;
#endif /* NT */
}

APIRET
CreateCommandModeSem( BOOL posted )
{
    if ( hevCommandMode )
#ifdef NT
        CloseHandle( hevCommandMode ) ;
    hevCommandMode = CreateEvent( NULL, TRUE, posted, NULL ) ;
    return hevCommandMode == NULL ? GetLastError() : 0 ;
#else /* not NT */
        DosCloseEventSem( hevCommandMode ) ;
    return DosCreateEventSem( NULL, &hevCommandMode, DC_SEM_SHARED, posted ) ;
#endif /* NT */
}

APIRET
PostCommandModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = SetEvent( hevCommandMode ) ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    return DosPostEventSem( hevCommandMode ) ;
#endif /* NT */
}

APIRET
WaitCommandModeSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevCommandMode, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevCommandMode, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetCommandModeSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevCommandMode, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevCommandMode ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevCommandMode, timo ) ;
    if ( !rc )
        DosResetEventSem( hevCommandMode, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetCommandModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevCommandMode ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevCommandMode, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseCommandModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevCommandMode ) ;
    hevCommandMode = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevCommandMode ) ;
    hevCommandMode = 0 ;
    return rc ;
#endif /* NT */
}


APIRET
WaitTerminalModeSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTerminalMode, timo, TRUE ) ;
    return rc == WAIT_OBJECT_0 ? 0 : rc ;
#else /* not NT */
    APIRET rc = 0 ;

    rc = DosWaitEventSem( hevTerminalMode, timo ) ;
    return rc ;
#endif /* NT */
}

APIRET
WaitAndResetTerminalModeSem( ULONG timo )
{
#ifdef NT
    DWORD rc = 0 ;

    rc = WaitForSingleObjectEx( hevTerminalMode, timo, TRUE ) ;
    if ( rc == WAIT_OBJECT_0 )
        ResetEvent( hevTerminalMode ) ;
    return rc == WAIT_OBJECT_0 ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG  semcount = 0 ;

    rc = DosWaitEventSem( hevTerminalMode, timo ) ;
    if ( !rc )
        DosResetEventSem( hevTerminalMode, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
ResetTerminalModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;

    rc = ResetEvent( hevTerminalMode ) ;
    return rc ;
#else /* not NT */
    APIRET rc = 0 ;
    ULONG semcount = 0 ;

    rc = DosResetEventSem( hevTerminalMode, &semcount ) ;
    return semcount ;
#endif /* NT */
}

APIRET
CloseTerminalModeSem( void )
{
#ifdef NT
    BOOL rc = 0 ;
    rc = CloseHandle( hevTerminalMode ) ;
    hevTerminalMode = (HANDLE) NULL ;
    return rc == TRUE ? 0 : GetLastError() ;
#else /* not NT */
    APIRET rc ;
    rc = DosCloseEventSem( hevTerminalMode ) ;
    hevTerminalMode = 0 ;
    return rc ;
#endif /* NT */
}

