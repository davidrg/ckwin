/* C K N T E L  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
          Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckntel.h"             /* Kermit Telephony */
#include "ckucmd.h"
#include <string.h>

#ifdef CK_TAPI
#include <windows.h>
#include <tapi.h>
/* all functions in this module return TRUE to indicate success */
/* or FALSE to indicate failure */
#include "cknwin.h"

extern int cktapi ;
extern struct keytab * tapilinetab, * tapilocationtab ;
extern int ntapiline, ntapilocation ;

HINSTANCE hLib = NULL ;
extern HINSTANCE hInstance ;
extern char * termessage ;

int tapiopen = 0 ;

LONG (WINAPI *cklineInitialize)(LPHLINEAPP, HINSTANCE, LINECALLBACK, LPCSTR, LPDWORD ) = NULL ;
LONG (WINAPI *cklineNegotiateAPIVersion)(HLINEAPP, DWORD, DWORD, DWORD, LPDWORD, LPLINEEXTENSIONID) = NULL ;
LONG (WINAPI *cklineGetDevCaps)(HLINEAPP, DWORD, DWORD, DWORD, LPLINEDEVCAPS) = NULL ;
LONG (WINAPI *cklineShutdown)(HLINEAPP) = NULL ;
LONG (WINAPI *cklineOpen)(HLINEAPP, DWORD, LPHLINE, DWORD, DWORD, DWORD, DWORD, DWORD,
                  LPLINECALLPARAMS) = NULL ;
LONG (WINAPI *cklineMakeCall)(HLINE hLine, LPHCALL lphCall, LPCSTR lpszDestAddress,
                      DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams) = NULL ;
LONG (WINAPI *cklineDial)(HCALL hCall, LPCSTR lpszDestAddress, DWORD dwCountryCode) = NULL ;
LONG (WINAPI *cklineDrop)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineClose)(HLINE hLine) = NULL ;
LONG (WINAPI *cklineGetID)(HLINE hLine, DWORD dwAddressID, HCALL hCall,
                   DWORD dwSelect, LPVARSTRING lpDeviceID, LPCSTR lpszDeviceClass) = NULL ;
LONG (WINAPI *cklineGetTranslateCaps)( HLINEAPP hLineApp, DWORD,
                                       LPLINETRANSLATECAPS lpLineTranslateCaps) = NULL ;
LONG (WINAPI *cklineSetCurrentLocation)( HLINE hLine, DWORD dwLocationID ) = NULL ;

int cktapiunload(void)
{
   FreeLibrary( hLib ) ;
   hLib = NULL ;
   cklineInitialize = NULL ;
   return TRUE ;
}

int cktapiload(void)
{
   DWORD rc = 0 ;
    hLib = LoadLibrary("tapi32") ;
   if ( !hLib )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }

   if (((FARPROC) cklineInitialize = GetProcAddress( hLib, "lineInitialize" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineNegotiateAPIVersion = GetProcAddress( hLib, "lineNegotiateAPIVersion" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineGetDevCaps = GetProcAddress( hLib, "lineGetDevCaps" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineShutdown = GetProcAddress( hLib, "lineShutdown" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineOpen = GetProcAddress( hLib, "lineOpen" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineMakeCall = GetProcAddress( hLib, "lineMakeCall" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineDial = GetProcAddress( hLib, "lineDial" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineDrop = GetProcAddress( hLib, "lineDrop" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineClose = GetProcAddress( hLib, "lineClose" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineGetID = GetProcAddress( hLib, "lineGetID" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineGetTranslateCaps = GetProcAddress( hLib, "lineGetTranslateCaps" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if (((FARPROC) cklineSetCurrentLocation = GetProcAddress( hLib, "lineSetCurrentLocation" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
    return TRUE;
}

// Global TAPI variables.
HWND     g_hWndMainWindow = NULL;   // Apps main window.
HWND     g_hDlgParentWindow = NULL; // This will be the parent of all dialogs.
HLINEAPP g_hLineApp = NULL;
DWORD    g_dwNumDevs = 0;
DWORD    g_DataDevices = 0 ;

#define MAXDEVS 64
LPLINEDEVCAPS g_lpLineDevCaps[64] ;
extern HINSTANCE hInst ;

// Global variable that holds the handle to a TAPI dialog
// that needs to be dismissed if line conditions change.
HWND g_hDialog = NULL;

// Global flags to prevent re-entrancy problems.
BOOL g_bShuttingDown = FALSE;
BOOL g_bStoppingCall = FALSE;
BOOL g_bInitializing = FALSE;


// This sample only supports one call in progress at a time.
BOOL g_bTapiInUse = FALSE;


// Data needed per call.  This sample only supports one call.
HCALL g_hCall = NULL;
HLINE g_hLine = NULL;
DWORD g_dwDeviceID = 0;
DWORD g_dwAPIVersion = 0;
DWORD g_dwCallState = 0;
char  g_szDisplayableAddress[1024] = "";
char  g_szDialableAddress[1024] = "";
BOOL  g_bConnected = FALSE;
LPVOID g_lpDeviceConfig = NULL;
DWORD g_dwSizeDeviceConfig;

// Global variables to allow us to do various waits.
BOOL  g_bReplyRecieved;
DWORD g_dwRequestedID;
long  g_lAsyncReply;
BOOL  g_bCallStateReceived;

int cktapiinit(void)
{
   int i = 0 ;
   g_hWndMainWindow = g_hDlgParentWindow = hwndGUI; // This will be the parent of all dialogs.

   for ( i ; i < MAXDEVS ; i++ )
      g_lpLineDevCaps[i] = NULL ;

    if ( cktapiload() )
    {
        if (cktapiopen())
        {
           debug(F100,"TAPI open successful","",0);
           g_DataDevices = cktapidevenum() ;
           cktapiclose() ;
           return TRUE ;
        }
        else
        {
            debug(F100,"TAPI open failed","",0);
        }
    }
    return FALSE;
}

void
OutputDebugLastError( DWORD x, char * string )
{
    debug(F111,"TAPI LastError",string,x);
}

void
HandleNoMem( )
{
   debug(F100,"TAPI HandleNoMem","",0);
}

//
//  FUNCTION: LPVOID CheckAndReAllocBuffer(LPVOID, size_t, LPCSTR)
//
//  PURPOSE: Checks and ReAllocates a buffer if necessary.
//
//  PARAMETERS:
//    lpBuffer          - Pointer to buffer to be checked.  Can be NULL.
//    sizeBufferMinimum - Minimum size that lpBuffer should be.
//    szApiPhrase       - Phrase to print if an error occurs.
//
//  RETURN VALUE:
//    Returns a pointer to a valid buffer that is guarenteed to be
//    at least sizeBufferMinimum size.
//    Returns NULL if an error occured.
//
//  COMMENTS:
//
//    This function is a helper function intended to make all of the
//    line API Wrapper Functions much simplier.  It allocates (or
//    reallocates) a buffer of the requested size.
//
//    The returned pointer has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a pointer is passed in, it *must* have been allocated
//    with LocalAlloc and it could potentially be LocalFree()d.
//
//    If lpBuffer == NULL, then a new buffer is allocated.  It is
//    normal to pass in NULL for this parameter the first time and only
//    pass in a pointer if the buffer needs to be reallocated.
//
//    szApiPhrase is used only for debugging purposes.
//
//    It is assumed that the buffer returned from this function will be used
//    to contain a variable sized structure.  Thus, the dwTotalSize field
//    is always filled in before returning the pointer.
//
//

LPVOID CheckAndReAllocBuffer(
    LPVOID lpBuffer, size_t sizeBufferMinimum, LPCSTR szApiPhrase)
{
    size_t sizeBuffer;

    if (lpBuffer == NULL)  // Allocate the buffer if necessary.
    {
        sizeBuffer = sizeBufferMinimum;
        lpBuffer = (LPVOID) LocalAlloc(LPTR, sizeBuffer);

        if (lpBuffer == NULL)
        {
            OutputDebugString(szApiPhrase);
            OutputDebugLastError(GetLastError(),"LocalAlloc : ");
            HandleNoMem();
            return NULL;
        }
    }
    else // If the structure already exists, make sure its good.
    {
        sizeBuffer = LocalSize((HLOCAL) lpBuffer);

        if (sizeBuffer == 0) // Bad pointer?
        {
            OutputDebugString(szApiPhrase);
            OutputDebugLastError(GetLastError(),"LocalSize : ");
            return NULL;
        }

        // Was the buffer big enough for the structure?
        if (sizeBuffer < sizeBufferMinimum)
        {
            OutputDebugString(szApiPhrase);
            OutputDebugString("Reallocating structure\n");
            LocalFree(lpBuffer);
            return CheckAndReAllocBuffer(NULL, sizeBufferMinimum, szApiPhrase);
        }

        // Lets zero the buffer out.
        memset(lpBuffer, 0, sizeBuffer);
    }

    ((LPVARSTRING) lpBuffer ) -> dwTotalSize = (DWORD) sizeBuffer;
    return lpBuffer;
}


void UpdateStatusBar( char * str, long param1, long param2 )
{
    printf( str ) ;
}

void OutputDebugLineError( long param, char * str )
{
    debug(F111,"TAPI",str,param);
}

void OutputDebugLineCallback(
        DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    ;
}

//
//  FUNCTION: DoLineReply(..)
//
//  PURPOSE: Handle LINE_REPLY asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle associated with this LINE_REPLY.
//    dwMsg     - Should always be LINE_REPLY.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - Asynchronous request ID.
//    dwParam2  - success or LINEERR error value.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    All line API calls that return an asynchronous request ID
//    will eventually cause a LINE_REPLY message.  Handle it.
//
//    This sample assumes only one call at time, and that we wait
//    for a LINE_REPLY before making any other line API calls.
//
//    The only exception to the above is that we might shut down
//    the line before receiving a LINE_REPLY.
//
//

void DoLineReply(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    if ((long) dwParam2 != SUCCESS)
        OutputDebugLineError((long) dwParam2, "LINE_REPLY error: ");
    else
        OutputDebugString("LINE_REPLY: successfully replied.\n");

    // If we are currently waiting for this async Request ID
    // then set the global variables to acknowledge it.
    if (g_dwRequestedID == dwParam1)
    {
        g_bReplyRecieved = TRUE;
        g_lAsyncReply = (long) dwParam2;
    }
}


//
//  FUNCTION: DoLineClose(..)
//
//  PURPOSE: Handle LINE_CLOSE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle that was closed.
//    dwMsg     - Should always be LINE_CLOSE.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - Unused.
//    dwParam2  - Unused.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is sent when something outside our app shuts
//    down a line in use.
//
//    The hLine (and any hCall on this line) are no longer valid.
//
//

void DoLineClose(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Line has been shut down.  Clean up our internal variables.
    g_hLine = NULL;
    g_hCall = NULL;
    ttclos(0);
    termessage = "Call was shut down.";
}


//
//  FUNCTION: DoLineCreate(..)
//
//  PURPOSE: Handle LINE_LINECREATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Unused.
//    dwMsg     - Should always be LINE_CREATE.
//    dwCallbackInstance - Unused.
//    dwParam1  - dwDeviceID of new Line created.
//    dwParam2  - Unused.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is new for Windows 95.  It is sent when a new line is
//    added to the system.  This allows us to handle new lines without having
//    to REINIT.  This allows for much more graceful Plug and Play.
//
//    This sample just changes the number of devices available and can use
//    it next time a call is made.  It also tells the "Dial" dialog.
//
//

void DoLineCreate(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // dwParam1 is the Device ID of the new line.
    // Add one to get the number of total devices.
    if (g_dwNumDevs <= dwParam1)
        g_dwNumDevs = dwParam1+1;
    g_DataDevices = cktapidevenum() ;
}

//
//  FUNCTION: DoLineDevState(..)
//
//  PURPOSE: Handle LINE_LINEDEVSTATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle that was closed.
//    dwMsg     - Should always be LINE_LINEDEVSTATE.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - LINEDEVSTATE constant.
//    dwParam2  - Depends on dwParam1.
//    dwParam3  - Depends on dwParam1.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    The LINE_LINEDEVSTATE message is received if the state of the line
//    changes.  Examples are RINGING, MAINTENANCE, MSGWAITON.  Very few of
//    these are relevant to this sample.
//
//    Assuming that any LINEDEVSTATE that removes the line from use by TAPI
//    will also send a LINE_CLOSE message.
//
//

void DoLineDevState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    switch(dwParam1)
    {
        case LINEDEVSTATE_RINGING:
            UpdateStatusBar("Line Ringing",1,0);
            OutputDebugString("Line Ringing.\n");
            break;

        case LINEDEVSTATE_REINIT:
        // This is an important case!  Usually means that a service provider
        // has changed in such a way that requires TAPI to REINIT.
        // Note that there are both 'soft' REINITs and 'hard' REINITs.
        // Soft REINITs don't actually require a full shutdown but is instead
        // just an informational change that historically required a REINIT
        // to force the application to deal with.  TAPI API Version 1.3 apps
        // will still need to do a full REINIT for both hard and soft REINITs.

            switch(dwParam2)
            {
                // This is the hard REINIT.  No reason given, just REINIT.
                // TAPI is waiting for everyone to shutdown.
                // Our response is to immediately shutdown any calls,
                // shutdown our use of TAPI and notify the user.
                case 0:
                    ttclos(0);
                    cktapiclose();
                    termessage = "Tapi line configuration has changed.";
                    break;

                case LINE_CREATE:
                    OutputDebugString("Soft REINIT: LINE_CREATE.\n");
                    DoLineCreate(dwDevice, dwParam2, dwCallbackInstance,
                        dwParam3, 0, 0);
                    break;

                case LINE_LINEDEVSTATE:
                    OutputDebugString("Soft REINIT: LINE_LINEDEVSTATE.\n");
                    DoLineDevState(dwDevice, dwParam2, dwCallbackInstance,
                        dwParam3, 0, 0);
                    break;

                // There might be other reasons to send a soft reinit.
                // No need to to shutdown for these.
                default:
                    OutputDebugString("Ignoring soft REINIT\n");
                    break;
            }
            break;

        case LINEDEVSTATE_OUTOFSERVICE:
            termessage = "Line selected is now Out of Service.";
            ttclos(0);
            break;

        case LINEDEVSTATE_DISCONNECTED:
            termessage = "Line selected is now disconnected.";
            ttclos(0);
            break;

        case LINEDEVSTATE_MAINTENANCE:
            termessage = "Line selected is now out for maintenance.";
            ttclos(0);
            break;

    case LINEDEVSTATE_TRANSLATECHANGE:
           #ifdef COMMENT
            if (g_hDialog)
                PostMessage(g_hDialog, WM_COMMAND, IDC_CONFIGURATIONCHANGED, 0);
           #endif
            break;

        case LINEDEVSTATE_REMOVED:
            OutputDebugString("A Line device has been removed;"
                " no action taken.\n");
            break;

        default:
            OutputDebugString("Unhandled LINEDEVSTATE message\n");
    }
}



//
//  FUNCTION: DoLineCallState(..)
//
//  PURPOSE: Handle LINE_CALLSTATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Handle to Call who's state is changing.
//    dwMsg     - Should always be LINE_CALLSTATE.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - LINECALLSTATE constant specifying state change.
//    dwParam2  - Specific to dwParam1.
//    dwParam3  - LINECALLPRIVILEGE change, if any.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is received whenever a call changes state.  Lots of
//    things we do, ranging from notifying the user to closing the line
//    to actually connecting to the target of our phone call.
//
//    What we do is usually obvious based on the call state change.
//

void DoLineCallState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Error if this CALLSTATE doesn't apply to our call in progress.
    if ((HCALL) dwDevice != g_hCall)
    {
        debug(F101,"LINE_CALLSTATE: Unknown device ID '0x%lx'.","",dwDevice);
        return;
    }

    // This sets the global g_dwCallState variable so if we are waiting
    // for a specific call state change, we will know when it happens.
    g_dwCallState = dwParam1;
    g_bCallStateReceived = TRUE;

    // dwParam3 contains changes to LINECALLPRIVILEGE, if there are any.
    switch (dwParam3)
    {
        case 0:
            break; // no change to call state

         // close line if we are made monitor.  Shouldn't happen!
         case LINECALLPRIVILEGE_MONITOR:
            OutputDebugString("line given monitor privilege; closing\n");
            ttclos(0);
            return;

         // close line if we are made owner.  Shouldn't happen!
        case LINECALLPRIVILEGE_OWNER:
            OutputDebugString("line given owner privilege; closing\n");
            ttclos(0);
            break;

        default: // Shouldn't happen!  All cases handled.
            OutputDebugString("Unknown LINECALLPRIVILEGE message: closing\n");
            ttclos(0);
            return;
    }

    // dwParam1 is the specific CALLSTATE change that is occurring.
    switch (dwParam1)
    {
        case LINECALLSTATE_DIALTONE:
            UpdateStatusBar("Dial Tone",1,0);
            OutputDebugString("Dial Tone\n");
            break;

        case LINECALLSTATE_DIALING:
            UpdateStatusBar("Dialing Call",1,0);
            OutputDebugString("Dialing\n");
            break;

        case LINECALLSTATE_PROCEEDING:
            UpdateStatusBar("Call is Proceeding",1,0);
            OutputDebugString("Proceeding\n");
            break;

        case LINECALLSTATE_RINGBACK:
            UpdateStatusBar("RingBack",1,0);
            OutputDebugString("RingBack\n");
            break;

        case LINECALLSTATE_BUSY:
            UpdateStatusBar("Line is busy",1,0);
            OutputDebugString("Line busy, shutting down\n");
            ttclos(0);
            break;

        case LINECALLSTATE_IDLE:
            UpdateStatusBar("Line is idle",1,0);
            OutputDebugString("Line idle\n");
            ttclos(0);
            break;

        case LINECALLSTATE_SPECIALINFO:
            UpdateStatusBar(
                "Special Info, probably couldn't dial number",1,0);
            OutputDebugString(
                "Special Info, probably couldn't dial number\n");
            ttclos(0);
            break;

        case LINECALLSTATE_DISCONNECTED:
        {
            LPSTR pszReasonDisconnected;

            switch (dwParam2)
            {
                case LINEDISCONNECTMODE_NORMAL:
                    pszReasonDisconnected = "Remote Party Disconnected";
                    break;

                case LINEDISCONNECTMODE_UNKNOWN:
                    pszReasonDisconnected = "Disconnected: Unknown reason";
                    break;

                case LINEDISCONNECTMODE_REJECT:
                    pszReasonDisconnected = "Remote Party rejected call";
                    break;

                case LINEDISCONNECTMODE_PICKUP:
                    pszReasonDisconnected =
                        "Disconnected: Local phone picked up";
                    break;

                case LINEDISCONNECTMODE_FORWARDED:
                    pszReasonDisconnected = "Disconnected: Forwarded";
                    break;

                case LINEDISCONNECTMODE_BUSY:
                    pszReasonDisconnected = "Disconnected: Busy";
                    break;

                case LINEDISCONNECTMODE_NOANSWER:
                    pszReasonDisconnected = "Disconnected: No Answer";
                    break;

                case LINEDISCONNECTMODE_BADADDRESS:
                    pszReasonDisconnected = "Disconnected: Bad Address";
                    break;

                case LINEDISCONNECTMODE_UNREACHABLE:
                    pszReasonDisconnected = "Disconnected: Unreachable";
                    break;

                case LINEDISCONNECTMODE_CONGESTION:
                    pszReasonDisconnected = "Disconnected: Congestion";
                    break;

                case LINEDISCONNECTMODE_INCOMPATIBLE:
                    pszReasonDisconnected = "Disconnected: Incompatible";
                    break;

                case LINEDISCONNECTMODE_UNAVAIL:
                    pszReasonDisconnected = "Disconnected: Unavail";
                    break;

                case LINEDISCONNECTMODE_NODIALTONE:
                    pszReasonDisconnected = "Disconnected: No Dial Tone";
                    break;

                default:
                    pszReasonDisconnected =
                        "Disconnected: LINECALLSTATE; Bad Reason";
                    break;

            }

            UpdateStatusBar(pszReasonDisconnected,1,0);
            OutputDebugString(pszReasonDisconnected);
            OutputDebugString("\n");
            ttclos(0);
            break;
        }


        case LINECALLSTATE_CONNECTED:  // CONNECTED!!!
        {
            LPVARSTRING lpVarString = NULL;
            DWORD dwSizeofVarString = sizeof(VARSTRING) + 1024;
            HANDLE hCommFile = NULL;
            long lReturn;

            // Very first, make sure this isn't a duplicated message.
            // A CALLSTATE message can be sent whenever there is a
            // change to the capabilities of a line, meaning that it is
            // possible to receive multiple CONNECTED messages per call.
            // The CONNECTED CALLSTATE message is the only one in TapiComm
            // where it would cause problems if it where sent more
            // than once.

            if (g_bConnected)
                break;

            g_bConnected = TRUE;

            // Get the handle to the comm port from the driver so we can start
            // communicating.  This is returned in a LPVARSTRING structure.
            do
            {
                // Allocate the VARSTRING structure
                lpVarString = malloc( dwSizeofVarString ) ;

                if (lpVarString == NULL)
                    goto ErrorConnecting;

                // Fill the VARSTRING structure
                lReturn = cklineGetID(0, 0, g_hCall, LINECALLSELECT_CALL,
                    lpVarString, "comm/datamodem");

                if (HandleLineErr(lReturn))
                    ; // Still need to check if structure was big enough.
                else
                {
                    OutputDebugLineError(lReturn,
                        "lineGetID unhandled error: ");
                    goto ErrorConnecting;
                }

                // If the VARSTRING wasn't big enough, loop again.
                if ((lpVarString -> dwNeededSize) > (lpVarString -> dwTotalSize))
                {
                    dwSizeofVarString = lpVarString -> dwNeededSize;
                    lReturn = -1; // Lets loop again.
                }
            }
            while(lReturn != SUCCESS);

            OutputDebugString("Connected!  Starting communications!\n");

            // Again, the handle to the comm port is contained in a
            // LPVARSTRING structure.  Thus, the handle is the very first
            // thing after the end of the structure.  Note that the name of
            // the comm port is right after the handle, but I don't want it.
            hCommFile =
                *((LPHANDLE)((LPBYTE)lpVarString +
                    lpVarString -> dwStringOffset));


            // Started communications!
           // if (StartComm(hCommFile))
           // {
           //     char szBuff[300];
           //
           //     wsprintf(szBuff,"Connected to '%s'",g_szDisplayableAddress);
           //     UpdateStatusBar(szBuff, 1, 0);
           //
           //     free(lpVarString);
           //     break;
           // }

            // Couldn't start communications.  Clean up instead.
          ErrorConnecting:

            // Its very important that we close all Win32 handles.
            // The CommCode module is responsible for closing the hCommFile
            // handle if it succeeds in starting communications.
            if (hCommFile)
                CloseHandle(hCommFile);

            ttclos(0);
            {
                char szBuff[300];
                wsprintf(szBuff,"Failed to Connect to '%s'",
                    g_szDisplayableAddress);
                UpdateStatusBar(szBuff, 1, 0);
            }

            if (lpVarString)
                free(lpVarString);

            break;
        }

        default:
            OutputDebugString("Unhandled LINECALLSTATE message\n");
            break;
    }
}

void CALLBACK cklineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{

    OutputDebugLineCallback(
        dwDevice, dwMsg, dwCallbackInstance,
        dwParam1, dwParam2, dwParam3);

    // All we do is dispatch the dwMsg to the correct handler.
    switch(dwMsg)
    {
        case LINE_CALLSTATE:
            DoLineCallState(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_CLOSE:
            DoLineClose(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_LINEDEVSTATE:
            DoLineDevState(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_REPLY:
            DoLineReply(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_CREATE:
            DoLineCreate(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        default:
            OutputDebugString("lineCallbackFunc message ignored\n");
            break;

    }

    return;

}

BOOL HandleLineErr(long lLineErr)
{
   if ( lLineErr )
    return FALSE;
   else return TRUE ;
}

BOOL HandleNoDevicesInstalled(void)
{
    return FALSE;
}


int cktapiopen(void)
{
    long lReturn;
    BOOL bTryReInit = TRUE ;

   if ( tapiopen )
   {
      tapiopen++ ;
      return TRUE ;
   }

   // Initialize TAPI
    do
    {
        lReturn = cklineInitialize(&g_hLineApp, hInstance,
            cklineCallbackFunc, "Kermit", &g_dwNumDevs);

        // If we get this error, its because some other app has yet
        // to respond to the REINIT message.  Wait 5 seconds and try
        // again.  If it still doesn't respond, tell the user.
        if (lReturn == LINEERR_REINIT)
        {
            if (bTryReInit)
            {
                MSG msg;
                DWORD dwTimeStarted;

                dwTimeStarted = GetTickCount();

                while(GetTickCount() - dwTimeStarted < 5000)
                {
                    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                bTryReInit = FALSE;
                continue;
            }
            else
            {
                MessageBox(g_hDlgParentWindow,
                    "A change to the system configuration requires that "
                    "all Telephony applications relinquish their use of "
                    "Telephony before any can progress.  "
                    "Some have not yet done so."
                    ,"Warning",MB_OK);
                g_bInitializing = FALSE;
                return FALSE;
            }
        }

        if (lReturn == LINEERR_NODEVICE)
        {
            if (HandleNoDevicesInstalled())
                continue;
            else
            {
                OutputDebugString("No devices installed.\n");
                g_bInitializing = FALSE;
                return FALSE;
            }
        }

        if (HandleLineErr(lReturn))
            continue;
        else
        {
            OutputDebugLineError(lReturn,
                "lineInitialize unhandled error: ");
            g_bInitializing = FALSE;
            return FALSE;
        }
    }
   while(lReturn != SUCCESS);
   return TRUE;
}

int cktapidevenum( void )
{
   int i = 0 ;
   DWORD dwAPIVersion ;
   LINEEXTENSIONID ExtensionID ;
   int datalines = 0 ;

   /* Free existing LineDevCaps */
   for ( i = 0 ; i < MAXDEVS ; i++ )
   {
      if ( g_lpLineDevCaps[i] )
      {
         free( g_lpLineDevCaps[i] ) ;
         g_lpLineDevCaps[i] = NULL ;
      }
   }

   /* Enumerate current LineDevCaps */
   for ( i=0 ; i < g_dwNumDevs ; i++ )
   {
      g_lpLineDevCaps[i] = (LPLINEDEVCAPS) malloc (sizeof(LINEDEVCAPS)) ;
      g_lpLineDevCaps[i]->dwTotalSize = sizeof(LINEDEVCAPS) ;
      if ( cklineNegotiateAPIVersion(g_hLineApp, i,
                                    MAKELONG(4,1), MAKELONG(4,1),
                                    &dwAPIVersion, &ExtensionID))
      {
         free(g_lpLineDevCaps[i]);
         g_lpLineDevCaps[i] = NULL ;
         continue;
      }
      if (cklineGetDevCaps(g_hLineApp, i,
                          dwAPIVersion, 0, g_lpLineDevCaps[i]))
      {
         free(g_lpLineDevCaps[i]);
         g_lpLineDevCaps[i] = NULL ;
         continue;
      }

      if ( g_lpLineDevCaps[i]->dwNeededSize > g_lpLineDevCaps[i]->dwTotalSize )
      {
         DWORD NeededSize = g_lpLineDevCaps[i]->dwNeededSize;
         free(g_lpLineDevCaps[i]) ;
         g_lpLineDevCaps[i] = (LPLINEDEVCAPS) malloc (NeededSize) ;
         g_lpLineDevCaps[i]->dwTotalSize = NeededSize ;

         if (cklineGetDevCaps(g_hLineApp, i,
                               dwAPIVersion, 0, g_lpLineDevCaps[i]))
         {
            free(g_lpLineDevCaps[i]);
            g_lpLineDevCaps[i] = NULL ;
            continue;
         }
      }

      /* We now have a successful LineDevCaps structure */
      if ( g_lpLineDevCaps[i]->dwMediaModes & LINEMEDIAMODE_DATAMODEM )
      {
         /* then this is a valid line to use for data connections */
         datalines++ ;
      }
   }
   return datalines ;
}

int cktapiclose(void)
{
   if ( tapiopen > 0 )
      tapiopen-- ;

   if ( !tapiopen )
      cklineShutdown( g_hLineApp ) ;
   return TRUE;
}


int cktapidial(char * number)
{

    return TRUE;
}


int cktapiBuildLineTable( struct keytab ** pTable, int * pN )
{
   int i, n ;
   if ( *pTable )
   {
      for ( i=0 ; i < *pN ; i++ )
         free( (*pTable)[i].kwd ) ;
      free ( *pTable )  ;
   }

   *pTable = NULL ;
   *pN = 0 ;

   cktapiopen() ;
   n = cktapidevenum() ;
   cktapiclose() ;

   if ( n )
   {
      *pTable = malloc( sizeof(struct keytab) * n ) ;

      for ( i=0 ; i < g_dwNumDevs ; i++ )
         if ( g_lpLineDevCaps[i]->dwMediaModes & LINEMEDIAMODE_DATAMODEM )
         {
            (*pTable)[*pN].kwd = _strdup( ((char *)(g_lpLineDevCaps[i]))+g_lpLineDevCaps[i]->dwLineNameOffset) ;
            (*pTable)[*pN].kwval = i ;
            (*pTable)[*pN].flgs = 0 ;
            (*pN)++ ;
         }
   }
}

int cktapiBuildLocationTable( struct keytab ** pTable, int * pN )
{
   LPLINETRANSLATECAPS lpTranslateCaps = NULL;
   DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
   long lReturn = 0;
   DWORD dwCounter;
   LPLINELOCATIONENTRY lpLocationEntry;
   LPLINECARDENTRY lpLineCardEntry = NULL;
   DWORD dwPreferredCardID = MAXDWORD;
   int i = 0 ;


   if ( *pTable )
   {
      for ( i=0 ; i < *pN ; i++ )
         free( (*pTable)[i].kwd ) ;
      free ( *pTable )  ;
   }
   *pTable = NULL ;
   *pN = 0 ;

   cktapiopen() ;

   // First, get the TRANSLATECAPS
   do
   {
      lpTranslateCaps = (LPLINETRANSLATECAPS) CheckAndReAllocBuffer(
                 (LPVOID) lpTranslateCaps, dwSizeofTranslateCaps,
                 "cktapiFetchLocationInfo");

      if (lpTranslateCaps == NULL)
      {
         cktapiclose();
         return 0;
      }

      lReturn = (*cklineGetTranslateCaps)(g_hLineApp, 2,
                                      lpTranslateCaps);

      if (HandleLineErr(lReturn))
         ;
      else
      {
         OutputDebugLineError(lReturn,
                               "lineGetTranslateCaps unhandled error: ");
         LocalFree(lpTranslateCaps);
         cktapiclose();
         return 0;
      }

      if ((lpTranslateCaps -> dwNeededSize) >
           (lpTranslateCaps -> dwTotalSize))
      {
         dwSizeofTranslateCaps = lpTranslateCaps ->dwNeededSize;
         lReturn = -1; // Lets loop again.
      }
   }
   while(lReturn != SUCCESS);

   cktapiclose() ;

   // Find the location information in the TRANSLATECAPS
   lpLocationEntry = (LPLINELOCATIONENTRY)
      (((LPBYTE) lpTranslateCaps) + lpTranslateCaps->dwLocationListOffset);

   if ( lpTranslateCaps->dwNumLocations > 0 )
   {
      *pTable = malloc( sizeof(struct keytab) * lpTranslateCaps->dwNumLocations ) ;

      // enumerate all the locations
      for(dwCounter = 0;
           dwCounter < lpTranslateCaps -> dwNumLocations;
           dwCounter++)
      {

         // Put each one into the keytab
         (*pTable)[*pN].kwd = _strdup((((LPBYTE) lpTranslateCaps) +
                                                     lpLocationEntry[dwCounter].dwLocationNameOffset)) ;
         (*pTable)[*pN].kwval = i ;
         (*pTable)[*pN].flgs = 0 ;
         (*pN)++ ;
      }
   }
   LocalFree(lpTranslateCaps);
}

#ifdef COMMENT
int cktapiFetchLocationInfo( char * CurrentLocation,
                             char * CurrentCountry,
                             char * CurrentAreaCode )
{
   LPLINETRANSLATECAPS lpTranslateCaps = NULL;
   DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
   long lReturn;
   DWORD dwCounter;
   LPLINELOCATIONENTRY lpLocationEntry;
   LPLINECARDENTRY lpLineCardEntry = NULL;
   DWORD dwPreferredCardID = MAXDWORD;

   // First, get the TRANSLATECAPS
   do
   {
      lpTranslateCaps = (LPLINETRANSLATECAPS) CheckAndReAllocBuffer(
                 (LPVOID) lpTranslateCaps, dwSizeofTranslateCaps,
                 "cktapiFetchLocationInfo");

      if (lpTranslateCaps == NULL)
         return;

      lReturn = (*cklineGetTranslateCaps)(g_hLineApp, 2,
                                      lpTranslateCaps);

      if (HandleLineErr(lReturn))
         ;
      else
      {
         OutputDebugLineError(lReturn,
                               "lineGetTranslateCaps unhandled error: ");
         LocalFree(lpTranslateCaps);
         return;
      }

      if ((lpTranslateCaps -> dwNeededSize) >
           (lpTranslateCaps -> dwTotalSize))
      {
         dwSizeofTranslateCaps = lpTranslateCaps ->dwNeededSize;
         lReturn = -1; // Lets loop again.
      }
   }
   while(lReturn != SUCCESS);

   // Find the location information in the TRANSLATECAPS
   lpLocationEntry = (LPLINELOCATIONENTRY)
      (((LPBYTE) lpTranslateCaps) + lpTranslateCaps->dwLocationListOffset);

   // If lpszCurrentLocation, then make that location 'current'
   if (lpszCurrentLocation)
   {
      // loop through all locations, looking for a location match
      for(dwCounter = 0;
           dwCounter < lpTranslateCaps -> dwNumLocations;
           dwCounter++)
      {
         if (strcmp((((LPSTR) lpTranslateCaps) +
                      lpLocationEntry[dwCounter].dwLocationNameOffset),
                     lpszCurrentLocation)
              == 0)
         {
            // Found it!  Set the current location.
            (*cklineSetCurrentLocation)(g_hLineApp,
                                    lpLocationEntry[dwCounter].dwPermanentLocationID);

            // Set the return values.
            if (lpdwCountryID)
               *lpdwCountryID = lpLocationEntry[dwCounter].dwCountryID;

            if (lpszAreaCode)
               strcpy(lpszAreaCode, (((LPSTR) lpTranslateCaps) +
                                      lpLocationEntry[dwCounter].dwCityCodeOffset));

            // Store the preferred card ID for later use.
            dwPreferredCardID = lpLocationEntry[dwCounter].dwPreferredCardID;
            break;
         }
      }

      // Was a match for lpszCurrentLocation found?
      if (dwPreferredCardID == MAXDWORD)
      {
         OutputDebugString("lpszCurrentLocation not found\n");
         SendDlgItemMessage(hwndDlg, IDC_CALLINGCARD, WM_SETTEXT, 0,
                             (LPARAM) (LPCSTR) "Invalid Location Selected");
         LocalFree(lpTranslateCaps);
         return;
      }
   }
   else // fill the combobox and use the TAPI 'current' location.
   {
      // First empty the combobox
      SendDlgItemMessage(hwndDlg, IDC_LOCATION, CB_RESETCONTENT, 0, 0);

      // enumerate all the locations
      for(dwCounter = 0;
           dwCounter < lpTranslateCaps -> dwNumLocations;
           dwCounter++)
      {
         // Put each one into the combobox
         lReturn = SendDlgItemMessage(hwndDlg, IDC_LOCATION, CB_ADDSTRING,
                                       0, (LPARAM) (((LPBYTE) lpTranslateCaps) +
                                                     lpLocationEntry[dwCounter].dwLocationNameOffset));

         // Is this location the 'current' location?
         if (lpLocationEntry[dwCounter].dwPermanentLocationID ==
              lpTranslateCaps->dwCurrentLocationID)
         {
            // Return the requested information
            if (lpdwCountryID)
               *lpdwCountryID = lpLocationEntry[dwCounter].dwCountryID;

            if (lpszAreaCode)
               strcpy(lpszAreaCode, (((LPSTR) lpTranslateCaps) +
                                      lpLocationEntry[dwCounter].dwCityCodeOffset));

            // Set this to be the active location.
            SendDlgItemMessage(hwndDlg, IDC_LOCATION, CB_SETCURSEL, lReturn, 0);
            dwPreferredCardID = lpLocationEntry[dwCounter].dwPreferredCardID;
         }
      }
   }

   // Now locate the prefered card and display it.

   lpLineCardEntry = (LPLINECARDENTRY)
      (((LPBYTE) lpTranslateCaps) + lpTranslateCaps->dwCardListOffset);

   for(dwCounter = 0;
        dwCounter < lpTranslateCaps -> dwNumCards;
        dwCounter++)
    {
        if (lpLineCardEntry[dwCounter].dwPermanentCardID == dwPreferredCardID)
        {
            SendDlgItemMessage(hwndDlg, IDC_CALLINGCARD, WM_SETTEXT, 0,
                (LPARAM) (((LPBYTE) lpTranslateCaps) +
                    lpLineCardEntry[dwCounter].dwCardNameOffset));
            break;
        }
    }

    LocalFree(lpTranslateCaps);
}
#endif /* COMMENT */
#endif /* CK_TAPI */
