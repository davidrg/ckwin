/* C K N T A P  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckucmd.h"
#include "ckuusr.h"
#include "ckowin.h"
#include <string.h>

#include <windows.h>
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
/* all functions in this module return TRUE to indicate success */
/* or FALSE to indicate failure */
#include "ckntap.h"             /* Kermit Telephony */
#include "cknwin.h"

#ifdef CK_TAPI

_PROTOTYP( char * cktapiErrorString, (DWORD));

extern int tttapi ;                     /* is Line TAPI ? */
extern struct keytab * tapilinetab, * tapilocationtab ;
extern int ntapiline, ntapilocation ;
extern long speed;
extern int parity, flow;
#ifndef NODIAL
extern int dialdpy;
extern int
dialidt,                        /* DIAL IGNORE-DIAL-TONE */
dialec,                         /* DIAL ERROR-CORRECTION */
dialdc,                         /* DIAL COMPRESSION  */
dialfc,                         /* DIAL FLOW-CONTROL */
dialmth,                        /* DIAL METHOD */
dialmauto,                      /* DIAL METHOD AUTO */
mdmspd,                         /* SPEED-MATCHING */
mdmspk,                         /* SPEAKER ON/OFF */
mdmvol;                         /* SPEAKER VOLUME */
extern int waitct;                  /* DIAL TIMEOUT */
extern MDMINF *modemp[];
extern MDMINF GENERIC;
#else /* NODIAL */
int dialdpy = 0;
int waitct = 254;
#endif /* NODIAL */

HINSTANCE hLib = NULL ;
extern HINSTANCE hInstance ;
extern HWND hwndConsole ;
extern char * termessage ;
#ifndef NODIAL
/*  Failure codes from ckudia.c */
extern int fail_code;

#define F_TIME          1               /* timeout */
#define F_INT           2               /* interrupt */
#define F_MODEM         3               /* modem-detected failure */
#define F_MINIT         4               /* cannot initialize modem */
#endif /* NODIAL */

int tapiopen = 0;
int tapipass = CK_ON ;                  /* TAPI Passthrough mode */
int tapiconv = CK_AUTO ;                /* TAPI Conversion mode */
int tapilights = TRUE;
int tapipreterm = FALSE;
int tapipostterm = FALSE;
int tapimanual = FALSE;
int tapiinactivity = 0;
int tapibong = 8;
int tapiusecfg = CK_OFF;

/* TAPI Location Information */
char tapiloc[257] = "" ;
int tapilocid = -1 ;

DWORD  tapiCountryID = -1 ;
char   tapiAreaCode[65] = "" ;
DWORD  tapiPreferredCardID=MAXDWORD;
DWORD  tapiCountryCode=0;
char   tapiLocalAccessCode[65] = "" ;
char   tapiLongDistAccessCode[65] = "";
char   tapiTollPrefixList[4097]="";
DWORD  tapiOptions=0;
char   tapiCancelCallWaiting[65] = "";
char   tapiCountryName[65] = "";
char   tapiSameAreaRule[65] = "";
char   tapiLongDistanceRule[65] = "";
char   tapiInternationalRule[65] = "";

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
LONG (WINAPI *cklineAnswer)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineAccept)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineDeallocateCall)(HCALL hCall) = NULL ;
LONG (WINAPI *cklineSetCallPrivilege)(HCALL,DWORD) = NULL ;
LONG (WINAPI *cklineClose)(HLINE hLine) = NULL ;
LONG (WINAPI *cklineHandoff)(HCALL,LPCSTR,DWORD) = NULL ;
LONG (WINAPI *cklineGetID)(HLINE hLine, DWORD dwAddressID, HCALL hCall,
               DWORD dwSelect, LPVARSTRING lpDeviceID, LPCSTR lpszDeviceClass) = NULL ;
LONG (WINAPI *cklineGetTranslateCaps)( HLINEAPP hLineApp, DWORD,
                                       LPLINETRANSLATECAPS lpLineTranslateCaps) = NULL ;
LONG (WINAPI *cklineSetCurrentLocation)( HLINEAPP hLineApp, DWORD dwLocationID ) = NULL ;
LONG (WINAPI *cklineSetStatusMessages)( HLINE hLine, DWORD dwLineStates,
                                        DWORD dwAddressStates ) = NULL ;
LONG (WINAPI *cklineConfigDialog)( DWORD dwLine, HWND hwin, LPCSTR lpszTypes ) = NULL ;
LONG (WINAPI *cklineTranslateDialog)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
                                      HWND hwndOwner, LPCSTR lpszAddressIn ) = NULL ;
LONG (WINAPI *cklineTranslateAddress)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
                                       LPCSTR lpszAddressIn, DWORD dwCard,
                                       DWORD dwTranslateOptions,
                                       LPLINETRANSLATEOUTPUT lpTranslateOutput) = NULL ;
LONG (WINAPI *cklineGetCountry)( DWORD, DWORD, LPLINECOUNTRYLIST ) = NULL;
LONG (WINAPI *cklineGetDevConfig)(DWORD, LPVARSTRING, LPCSTR) = NULL;
LONG (WINAPI *cklineGetLineDevStatus)(HLINE hLine,LPLINEDEVSTATUS lpLineDevStatus)=NULL;
LONG (WINAPI *cklineSetDevConfig)(DWORD,LPVOID const,DWORD,LPCSTR)=NULL;
LONG (WINAPI *cklineGetCallInfo)(HCALL, LPLINECALLINFO)=NULL;
LONG (WINAPI *cklineMonitorMedia)(HCALL,DWORD)=NULL;
LONG (WINAPI *cklineGetAppPriority)(LPCSTR,DWORD,LPLINEEXTENSIONID,
                                     DWORD,LPVARSTRING,LPDWORD)=NULL;
LONG (WINAPI *cklineSetAppPriority)(LPCSTR,DWORD,LPLINEEXTENSIONID,
                                     DWORD,LPCSTR,DWORD)=NULL;
LONG (WINAPI *cklineGetNumRings)(HLINE,DWORD,LPDWORD)=NULL;
LONG (WINAPI *cklineSetNumRings)(HLINE,DWORD,DWORD)=NULL;
LONG (WINAPI *cklineSetCallParams)(HCALL,DWORD,DWORD,DWORD,LPLINEDIALPARAMS)=NULL;


int
cktapiunload(void)
{
    debug(F100,"Closing TAPI Semaphores","",0);
    CloseTAPIInitSem();
    CloseTAPIConnectSem();
    CloseTAPIAnswerSem();
    debug(F100,"Freeing the TAPI Library","",0);
    FreeLibrary( hLib ) ;
    hLib = NULL ;
    cklineInitialize = NULL ;
    debug(F100,"TAPI unload complete","",0);
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
    if (((FARPROC) cklineAnswer = GetProcAddress( hLib, "lineAnswer" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineAccept = GetProcAddress( hLib, "lineAccept" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineDeallocateCall = GetProcAddress( hLib, "lineDeallocateCall" )) == NULL )
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
   if (((FARPROC) cklineSetStatusMessages = GetProcAddress( hLib, "lineSetStatusMessages" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
    if (((FARPROC) cklineConfigDialog = GetProcAddress( hLib, "lineConfigDialog" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineTranslateDialog = GetProcAddress( hLib, "lineTranslateDialog" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineTranslateAddress =
          GetProcAddress( hLib, "lineTranslateAddress" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetCountry =
          GetProcAddress( hLib, "lineGetCountry" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetLineDevStatus =
          GetProcAddress( hLib, "lineGetLineDevStatus" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetDevConfig =
          GetProcAddress( hLib, "lineGetDevConfig" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineSetDevConfig =
          GetProcAddress( hLib, "lineSetDevConfig" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineHandoff =
          GetProcAddress( hLib, "lineHandoff" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineSetCallPrivilege =
          GetProcAddress( hLib, "lineSetCallPrivilege" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetCallInfo =
          GetProcAddress( hLib, "lineGetCallInfo" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineMonitorMedia =
          GetProcAddress( hLib, "lineMonitorMedia" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetAppPriority =
          GetProcAddress( hLib, "lineGetAppPriority" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineSetAppPriority =
          GetProcAddress( hLib, "lineSetAppPriority" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineGetNumRings =
          GetProcAddress( hLib, "lineGetNumRings" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineSetNumRings =
          GetProcAddress( hLib, "lineSetNumRings" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }
    if (((FARPROC) cklineSetCallParams =
          GetProcAddress( hLib, "lineSetCallParams" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
        return FALSE;
    }

    CreateTAPIInitSem( FALSE );
    CreateTAPIConnectSem( FALSE );
    CreateTAPIAnswerSem( FALSE );
    return TRUE;
}

// Global TAPI variables.
HWND     g_hWndMainWindow = NULL;   // Apps main window.
HWND     g_hDlgParentWindow = NULL; // This will be the parent of all dialogs.
HLINEAPP g_hLineApp = (HLINEAPP) 0;
DWORD    g_dwNumDevs = -1;
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
HCALL g_hCall = (HCALL) 0;
HLINE g_hLine = (HLINE) 0;
extern int ttyfd ; /* this holds the HLINE hLine */
extern int mdmtyp ;
static int mdmtyp_sav=0;
CHAR szModemName[256] ;
DWORD LineDeviceId = -1;
DWORD LineDeviceAPIVersion = 0 ;
DWORD LineAddressCount = 0 ;
DWORD LineMediaModes = 0 ;

DWORD g_dwAPIVersion = 0;

DWORD g_dwCallState = 0;
BOOL  g_bConnected = FALSE;

// Global variables to allow us to do various waits.
BOOL  g_bReplyReceived=FALSE;
DWORD g_dwRequestedID=0;
long  g_lAsyncReply=SUCCESS;
BOOL  g_bCallStateReceived=FALSE;
BOOL  g_bAnswering = FALSE;
BOOL  g_bDialing = FALSE;
BOOL  g_bHangingUp = FALSE;
BOOL  g_bClosing = FALSE;

int
cktapiinit(void)
{
    int i = 0 ;
    // This will be the parent of all dialogs.
    g_hWndMainWindow = g_hDlgParentWindow = hwndConsole;

    for ( i=0 ; i < MAXDEVS ; i++ )
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
    char buf[1024];
    sprintf(buf,"%s %s",string,cktapiErrorString(x));
    debug(F111,"TAPI LastError",buf,x);
#ifdef BETADEBUG
    printf("TAPI LastError %s=%x\n",string,x);
#endif
}

void
HandleNoMem( )
{
   debug(F100,"TAPI Out of Memory","",0);
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

LPVOID
CheckAndReAllocBuffer(LPVOID lpBuffer, size_t sizeBufferMinimum,
                       LPCSTR szApiPhrase)
{
    size_t sizeBuffer;

    debug(F110,"CheckAndReAllocBuffer",szApiPhrase,0);
    debug(F111,"CheckAndReAllocBuffer","lpBuffer",lpBuffer);
    debug(F111,"CheckAndReAllocBuffer","sizeBufferMinimum",sizeBufferMinimum);

    if (lpBuffer == NULL)  // Allocate the buffer if necessary.
    {
        sizeBuffer = sizeBufferMinimum;
        lpBuffer = (LPVOID) LocalAlloc(LPTR, sizeBuffer);

        if (lpBuffer == NULL)
        {
            HandleNoMem();
            return NULL;
        }
    }
    else // If the structure already exists, make sure its good.
    {
        sizeBuffer = LocalSize((HLOCAL) lpBuffer);

        if (sizeBuffer == 0) // Bad pointer?
        {
            return NULL;
        }

        // Was the buffer big enough for the structure?
        if (sizeBuffer < sizeBufferMinimum)
        {
            LocalFree(lpBuffer);
            return CheckAndReAllocBuffer(NULL, sizeBufferMinimum, szApiPhrase);
        }

        // Lets zero the buffer out.
        memset(lpBuffer, 0, sizeBuffer);
    }

    ((LPVARSTRING) lpBuffer ) -> dwTotalSize = (DWORD) sizeBuffer;
    return lpBuffer;
}

char *
cktapiErrorString(DWORD error)
{
    char * estr = NULL;
    switch ( error ) {
    case 0:
        estr = "Success";
        break;
    case LINEERR_ALLOCATED:
        estr = "Line already allocated";
        break;
    case LINEERR_BADDEVICEID:
        estr = "Bad device ID";
        break;
    case LINEERR_BEARERMODEUNAVAIL:
        estr = "Requested Bearer Mode Unavailable";
        break;
    case LINEERR_CALLUNAVAIL:
        estr = "Unable to place call on current device";
        break;
    case LINEERR_COMPLETIONOVERRUN               :
        estr = "Completion Overrun";
        break;
    case LINEERR_CONFERENCEFULL                  :
        estr = "Conference Full";
        break;
    case LINEERR_DIALBILLING                     :
        estr = "Dial Billing error";
        break;
    case LINEERR_DIALDIALTONE                    :
        estr = "Wait for Dial Tone not supported";
        break;
    case LINEERR_DIALPROMPT                      :
        estr = "Wait for Dial Prompt not supported";
        break;
    case LINEERR_DIALQUIET                       :
        estr = "Wait for Quiet not supported";
        break;
    case LINEERR_INCOMPATIBLEAPIVERSION          :
        estr = "Incompatible TAPI Version";
        break;
    case LINEERR_INCOMPATIBLEEXTVERSION          :
        estr = "Incompatible Extension Version";
        break;
    case LINEERR_INIFILECORRUPT                  :
        estr = "TELEPHON.INI file corrupt";
        break;
    case LINEERR_INUSE                           :
        estr = "Line in use";
        break;
    case LINEERR_INVALADDRESS                    :
        estr = "Invalid Line Address";
        break;
    case LINEERR_INVALADDRESSID                  :
        estr = "Invalid Line Address ID";
        break;
    case LINEERR_INVALADDRESSMODE                :
        estr = "Invalid Line Address Mode";
        break;
    case LINEERR_INVALADDRESSSTATE               :
        estr = "Invalid Line Address State";
        break;
    case LINEERR_INVALAPPHANDLE                  :
        estr = "Invalid Application Handle";
        break;
    case LINEERR_INVALAPPNAME                    :
        estr = "Invalid Application Name";
        break;
    case LINEERR_INVALBEARERMODE                 :
        estr = "Invalid Bearer Mode";
        break;
    case LINEERR_INVALCALLCOMPLMODE              :
        estr = "Invalid Call Completion Mode";
        break;
    case LINEERR_INVALCALLHANDLE                 :
        estr = "Invalid Call Handle";
        break;
    case LINEERR_INVALCALLPARAMS                 :
        estr = "Invalid Call Parameters";
        break;
    case LINEERR_INVALCALLPRIVILEGE              :
        estr = "Invalid Call Privilege";
        break;
    case LINEERR_INVALCALLSELECT                 :
        estr = "Invalid Call Selection";
        break;
    case LINEERR_INVALCALLSTATE                  :
        estr = "Invalid Call State";
        break;
    case LINEERR_INVALCALLSTATELIST              :
        estr = "Invalid Call State List";
        break;
    case LINEERR_INVALCARD                       :
        estr = "Invalid Card";
        break;
    case LINEERR_INVALCOMPLETIONID               :
        estr = "Invalid Completion ID";
        break;
    case LINEERR_INVALCONFCALLHANDLE             :
        estr = "Invalid Conference Call Handle";
        break;
    case LINEERR_INVALCONSULTCALLHANDLE          :
        estr = "Invalid Consulation Call Handle";
        break;
    case LINEERR_INVALCOUNTRYCODE                :
        estr = "Invalid Country Code";
        break;
    case LINEERR_INVALDEVICECLASS                :
        estr = "Invalid Device Class";
        break;
    case LINEERR_INVALDEVICEHANDLE               :
        estr = "Invalid Device Handle";
        break;
    case LINEERR_INVALDIALPARAMS                 :
        estr = "Invalid Dial Parameters";
        break;
    case LINEERR_INVALDIGITLIST                  :
        estr = "Invalid Digit List";
        break;
    case LINEERR_INVALDIGITMODE                  :
        estr = "Invalid Digit Mode";
        break;
    case LINEERR_INVALDIGITS                     :
        estr = "Invalid Digits";
        break;
    case LINEERR_INVALEXTVERSION                 :
        estr = "Invalid Extension Version";
        break;
    case LINEERR_INVALGROUPID                    :
        estr = "Invalid Group ID";
        break;
    case LINEERR_INVALLINEHANDLE                 :
        estr = "Invalid Line Handle";
        break;
    case LINEERR_INVALLINESTATE                  :
        estr = "Invalid Line State";
        break;
    case LINEERR_INVALLOCATION                   :
        estr = "Invalid Location";
        break;
    case LINEERR_INVALMEDIALIST                  :
        estr = "Invalid Media List";
        break;
    case LINEERR_INVALMEDIAMODE                  :
        estr = "Invalid Media Mode";
        break;
    case LINEERR_INVALMESSAGEID                  :
        estr = "Invalid Message ID";
        break;
    case LINEERR_INVALPARAM                      :
        estr = "Invalid Parameter";
        break;
    case LINEERR_INVALPARKID                     :
        estr = "Invalid Park ID";
        break;
    case LINEERR_INVALPARKMODE                   :
        estr = "Invaldi Park Mode";
        break;
    case LINEERR_INVALPOINTER                    :
        estr = "Invalid Pointer";
        break;
    case LINEERR_INVALPRIVSELECT                 :
        estr = "Invalid Privilege Select";
        break;
    case LINEERR_INVALRATE                       :
        estr = "Invalid Rate";
        break;
    case LINEERR_INVALREQUESTMODE                :
        estr = "Invalid Request Mode";
        break;
    case LINEERR_INVALTERMINALID                 :
        estr = "Invalid Terminal ID";
        break;
    case LINEERR_INVALTERMINALMODE               :
        estr = "Invalid Terminal Mode";
        break;
    case LINEERR_INVALTIMEOUT                    :
        estr = "Invalid Timeout";
        break;
    case LINEERR_INVALTONE                       :
        estr = "Invalid Tone";
        break;
    case LINEERR_INVALTONELIST                   :
        estr = "Invalid Tone List";
        break;
    case LINEERR_INVALTONEMODE                   :
        estr = "Invalid Tone Mode";
        break;
    case LINEERR_INVALTRANSFERMODE               :
        estr = "Invalid Transfer Mode";
        break;
    case LINEERR_LINEMAPPERFAILED                :
        estr = "Line Mapper Failed";
        break;
    case LINEERR_NOCONFERENCE                    :
        estr = "No Conference available";
        break;
    case LINEERR_NODEVICE                        :
        estr = "No Device available";
        break;
    case LINEERR_NODRIVER                        :
        estr = "No Driver available";
        break;
    case LINEERR_NOMEM                           :
        estr = "No memory available";
        break;
    case LINEERR_NOREQUEST                       :
        estr = "No Request";
        break;
    case LINEERR_NOTOWNER                        :
        estr = "Not owner";
        break;
    case LINEERR_NOTREGISTERED                   :
        estr = "Not Registered";
        break;
    case LINEERR_OPERATIONFAILED                 :
        estr = "Operation Failed";
        break;
    case LINEERR_OPERATIONUNAVAIL                :
        estr = "Operation Unavailable";
        break;
    case LINEERR_RATEUNAVAIL                     :
        estr = "Rate Unavailable";
        break;
    case LINEERR_RESOURCEUNAVAIL                 :
        estr = "Resource Unavailable";
        break;
    case LINEERR_REQUESTOVERRUN                  :
        estr = "Request Overrun";
        break;
    case LINEERR_STRUCTURETOOSMALL               :
        estr = "Structure Too Small";
        break;
    case LINEERR_TARGETNOTFOUND                  :
        estr = "Target Not Found";
        break;
    case LINEERR_TARGETSELF                      :
        estr = "Target Self";
        break;
    case LINEERR_UNINITIALIZED                   :
        estr = "TAPI Unititialized";
        break;
    case LINEERR_USERUSERINFOTOOBIG              :
        estr = "User Info Too Big";
        break;
    case LINEERR_REINIT                          :
        estr = "TAPI Reinitialization Required";
        break;
    case LINEERR_ADDRESSBLOCKED                  :
        estr = "Address Blocked";
        break;
    case LINEERR_BILLINGREJECTED                 :
        estr = "Billing Rejected";
        break;
    case LINEERR_INVALFEATURE                    :
        estr = "Invalid Feature";
        break;
    case LINEERR_NOMULTIPLEINSTANCE              :
        estr = "Multiple Instances not supported";
        break;
#ifdef COMMENT  /* TAPI 2.0 */
    case LINEERR_INVALAGENTID                    :
        estr = "Invalid Agent ID";
        break;
    case LINEERR_INVALAGENTGROUP                 :
        estr = "Invalid Agent Group";
        break;
    case LINEERR_INVALPASSWORD                   :
        estr = "Invalid Password";
        break;
    case LINEERR_INVALAGENTSTATE                 :
        estr = "Invalid Agent State";
        break;
    case LINEERR_INVALAGENTACTIVITY              :
        estr = "Invalid Agent Activity";
        break;
    case LINEERR_DIALVOICEDETECT                 :
        estr = "Voice detected during dialing";
        break;
#endif /* COMMENT */
    default:
        estr = ((LONG) error) > 0 ? "Request ID" : "Unknown TAPI error";
        break;
    }
    return estr;
}

void
UpdateStatusBar( char * str, long param1, long param2 )
{
    char buf[1024];
    sprintf(buf,"%s::%d,%d",str,param1,param2);
    debug(F110,"TAPI Status Message",buf,0);

    if ( dialdpy )
        printf( "%s\n",str ) ;
}

void
OutputDebugLineError( long param, char * str )
{
    char buf[1024];
    sprintf(buf,"%s %s",str,cktapiErrorString(param));
#ifdef BETADEBUG
    printf("TAPI Debug Line Error: %s=%s(0x%x)\n",str,
            cktapiErrorString(param),param);
#endif /* BETADEBUG */
    debug(F111,"TAPI Debug Line Error",buf,param);
}

void
OutputDebugLineCallback(
        DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    char buf[1024];
    sprintf(buf,"(%d,%d,%d,%d,%d,%d)",
             dwDevice,dwMsg,dwCallbackInstance,
             dwParam1,dwParam2,dwParam3);
    debug(F110,"TAPI Line Callback",buf,0);
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

void
DoLineReply(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    debug(F111,"TAPI LINE REPLY","Requested ID",g_dwRequestedID);
    debug(F111,"TAPI LINE REPLY","Message ID",dwParam1);
    OutputDebugLineError((long) dwParam2, "LINE_REPLY error: ");

    while ( g_dwRequestedID == 0 ) {
        msleep(100);
    }

    // If we are currently waiting for this async Request ID
    // then set the global variables to acknowledge it.
    if (g_dwRequestedID == dwParam1)
    {
#ifdef BETADEBUG
        printf("LINE_REPLY: dwRequestedID == dwParam1\n");
#endif
        g_lAsyncReply = (long) dwParam2;
        g_bReplyReceived = TRUE;
    }
#ifdef BETADEBUG
    else {
        printf("LINE_REPLY: dwRequestedID != dwParam1\n");
    }
#endif
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

void
DoLineClose(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Line has been shut down.  Clean up our internal variables.
    if ( (HLINE) dwDevice == g_hLine ) {
        cktapicloseasync();
        debug(F110,"TAPI","Line was shutdown by TAPI",0);
        printf("%s was shut down.\n",szModemName);
    }
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
//

void
DoLineCreate(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // dwParam1 is the Device ID of the new line.
    // Add one to get the number of total devices.
    if (g_dwNumDevs <= dwParam1)
        g_dwNumDevs = dwParam1+1;
    g_DataDevices = cktapidevenum() ;
    debug(F111,"TAPI","A new line device has been added to the system",
           g_DataDevices);
#ifdef BETADEBUG
    printf("A new line device has been added to the system.\n");
#endif /* BETADEBUG */
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
//    changes.  Examples are RINGING, MAINTENANCE, MSGWAITON.
//
//    Assuming that any LINEDEVSTATE that removes the line from use by TAPI
//    will also send a LINE_CLOSE message.
//
//

void
DoLineDevState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    switch(dwParam1)
    {
    case LINEDEVSTATE_RINGING: {
        UpdateStatusBar("Line Ringing",1,0);
        break;
    }

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
#ifdef BETADEBUG
            printf("Line configuration has changed\n");
#endif /* BETADEBUG */
            if ( (HLINE) dwDevice == g_hLine ) {
                cktapicloseasync();
                printf("TAPI line configuration has changed.  Shutdown required.\n");
                debug(F111,"TAPI LINEDEVSTATE_REINIT-0","Shutdown required",dwDevice);
            }
            break;

        case LINE_CREATE:
#ifdef BETADEBUG
            printf("Soft REINIT: LINE_CREATE.\n");
#endif /* BETADEBUG */
            DoLineCreate(dwDevice, dwParam2, dwCallbackInstance,
                          dwParam3, 0, 0);
            break;

        case LINE_LINEDEVSTATE:
#ifdef BETADEBUG
            printf("Soft REINIT: LINE_LINEDEVSTATE.\n");
#endif /* BETADEBUG */

            DoLineDevState(dwDevice, dwParam2, dwCallbackInstance,
                            dwParam3, 0, 0);
            break;

            // There might be other reasons to send a soft reinit.
            // No need to to shutdown for these.
        default:
#ifdef BETADEBUG
            printf("Ignoring soft REINIT\n");
#endif /* BETADEBUG */
            break;
        }
        break;

    case LINEDEVSTATE_OUTOFSERVICE:
        if ( (HLINE) dwDevice == g_hLine ) {
            cktapicloseasync();
            printf("%s is now Out of Service\n",szModemName);
            debug(F111,"TAPI LINEDEVSTATE_OUTOFSERVICE",szModemName,dwDevice);
        }
        break;

    case LINEDEVSTATE_DISCONNECTED:
        if ( (HLINE) dwDevice == g_hLine &&
             g_hCall != (HCALL) 0 ) {
            cktapidisconnect();
            printf("%s is now disconnected\n",szModemName);
            debug(F111,"TAPI LINEDEVSTATE_DISCONNECTED",szModemName,dwDevice);
        }
        break;

    case LINEDEVSTATE_MAINTENANCE:
        if ( (HLINE) dwDevice == g_hLine ) {
            cktapicloseasync();
            printf("%s is now out for maintenance\n",szModemName);
            debug(F111,"TAPI LINEDEVSTATE_MAINTENANCE",szModemName,dwDevice);
        }
        break;

    case LINEDEVSTATE_TRANSLATECHANGE:
#ifdef BETADEBUG
        printf("Translate Change\n");
#endif
        debug(F100,"TAPI LINEDEVSTATE_TRANSLATECHANGE","",0);
#ifndef NODIAL
        if ( 1 || tapiconv == CK_ON ||
             tapiconv == CK_AUTO && tttapi && !tapipass ) {
            /* Reload TAPI Location Information From Scratch */
            tapilocid = -1;
            CopyTapiLocationInfoToKermitDialCmd();
        }
#endif /* NODIAL */
        break;

    case LINEDEVSTATE_REMOVED:
#ifdef BETADEBUG
        printf("A Line device has been removed; no action taken.\n");
#endif
        if (g_dwNumDevs <= dwParam1)
            g_dwNumDevs = dwParam1+1;
        g_DataDevices = cktapidevenum() ;
        debug(F111,"TAPI LINEDEVSTATE_REMOVED",
               "A line device has been removed; no action taken",
               g_DataDevices);
        break;

    case LINEDEVSTATE_CLOSE:
#ifdef BETADEBUG
        printf("A Line device has been closed; no action taken.\n");
#endif /* BETADEBUG */
        debug(F110,"TAPI LINEDEVSTATE_CLOSE","A Line device has been closed; no action taken",
               0);
        break;

    case LINEDEVSTATE_OPEN:
#ifdef BETADEBUG
        printf("A Line device has been opened; no action taken.\n");
#endif /* BETADEBUG */
        debug(F110,"TAPI LINEDEVSTATE_OPEN","A Line device has been openned; no action taken",
               0);
        break;

    default:
#ifdef BETADEBUG
        printf("Unhandled LINEDEVSTATE message\n");
#endif
        debug(F111,"TAPI LINEDEVSTATE_???",
               "Unhandled LINEDEVSTATE message",
               dwParam1);
        break;
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

void
DoLineCallState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Error if this CALLSTATE doesn't apply to our call in progress.
    if ( g_hCall != (HCALL)0 && (HCALL) dwDevice != g_hCall ||
         g_hCall == (HCALL)0 && !g_bDialing && !g_bAnswering )
    {
        int rc=0;
        debug(F101,"LINE_CALLSTATE: Unknown device ID '0x%lx'.","",dwDevice);
#ifdef BETADEBUG
        printf("LINE_CALLSTATE: Unknown device ID '0x%lx\n",dwDevice);
#endif
        rc = (*cklineDeallocateCall)((HCALL)dwDevice);
        OutputDebugLastError(rc,"DoLineCallState() lineDeallocateCall: ");
#ifdef BETADEBUG
        printf("  DoLineCallState() lineDeallocateCall = %x\n",rc);
#endif
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
#ifdef BETADEBUG
        printf("line has monitor privilege\n");
#endif
        //cktapiclose();
        break;

        // close line if we are made owner.  Shouldn't happen!
    case LINECALLPRIVILEGE_OWNER:
#ifdef BETADEBUG
        printf("line has owner privilege\n");
#endif
        //cktapiclose();
        break;

    default: // Shouldn't happen!  All cases handled.
#ifdef BETADEBUG
        printf("Unknown LINECALLPRIVILEGE message\n");
#endif
        //cktapiclose();
        return;
    }

    // dwParam1 is the specific CALLSTATE change that is occurring.
    switch (dwParam1)
    {
    case LINECALLSTATE_DIALTONE:
        if ( g_bDialing )
            UpdateStatusBar("Dial Tone",1,0);
        break;

    case LINECALLSTATE_DIALING:
        if ( g_bDialing )
            UpdateStatusBar("Dialing Call",1,0);
        break;

    case LINECALLSTATE_PROCEEDING:
        if ( g_bDialing )
            UpdateStatusBar("Call is Proceeding",1,0);
        break;

    case LINECALLSTATE_RINGBACK:
        UpdateStatusBar("RingBack",1,0);
        break;

    case LINECALLSTATE_BUSY:
        if ( g_bDialing ) {
            UpdateStatusBar("Line is busy",1,0);
            PostTAPIConnectSem();
        }
        break;

    case LINECALLSTATE_IDLE:
        msleep(1000);
        UpdateStatusBar("Line is idle",1,0);
        if ( ttyfd != -1 && ttyfd != -2 ) {
            cktapidisconnect();
            SetConnectMode(FALSE);
        }
        PostTAPIConnectSem();
        break;

    case LINECALLSTATE_SPECIALINFO:
        UpdateStatusBar("Special Info, probably couldn't dial number",1,0);
        PostTAPIConnectSem();
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
            if ( ttyfd != -1 && ttyfd != -2 ) {
                cktapidisconnect();
                SetConnectMode(FALSE);
            }
            PostTAPIConnectSem() ;
            UpdateStatusBar(pszReasonDisconnected,1,0);
            break;
        }


    case LINECALLSTATE_CONNECTED:  // CONNECTED!!!
        {
            int rc;
            LPVARSTRING lpVarString = NULL;
            DWORD dwSizeofVarString = sizeof(VARSTRING) + 1024;
            long lReturn;

            // Very first, make sure this isn't a duplicated message.
            // A CALLSTATE message can be sent whenever there is a
            // change to the capabilities of a line, meaning that it is
            // possible to receive multiple CONNECTED messages per call.
            // The CONNECTED CALLSTATE message is the only one in TapiComm
            // where it would cause problems if it where sent more
            // than once.

            if ( g_bConnected && (HCALL) dwDevice == g_hCall )
                break;

            if (!(g_bDialing || g_bAnswering)) {
                /* Get the hCall */
                g_hCall = (HCALL) dwDevice;
                debug(F110,"TAPI LINECALLSTATE_CONNECTED",
                       "were not Dialing and we're not Answering",0);

                if ( dwParam3 == LINECALLPRIVILEGE_OWNER ) {
                    /* Need to check the Bearer Mode of the call */

                    /* are we even answering calls? */
                    rc = (*cklineHandoff)(g_hCall, NULL, LINEMEDIAMODE_DATAMODEM);
                    OutputDebugLastError(rc,"lineHandoff:");
                    rc = (*cklineDeallocateCall)(g_hCall);
                    OutputDebugLastError(rc,"lineDeallocateCall:");
                    if ( rc < 0 ) {
                        cktapidisconnect();
                    }
                    else {
                        g_hCall = (HCALL)0;
                    }
                }
                else /* if ( dwParam3 == LINECALLPRIVILEGE_MONITOR ) */ {
                    rc = (*cklineDeallocateCall)(g_hCall);
                    OutputDebugLastError(rc,"lineDeallocateCall:");
                    g_hCall = (HCALL)0;
                }
                break;
            }

            if ( g_bAnswering && g_hCall == (HCALL)0 ) {
                UpdateStatusBar("Incoming call being offered in connected state.",1,0);
                g_hCall = (HCALL) dwDevice;
                PostTAPIAnswerSem();
            }

            UpdateStatusBar("Connection complete.",1,0);
            g_bConnected = TRUE;

            // Get the handle to the comm port from the driver so we can start
            // communicating.  This is returned in a LPVARSTRING structure.
            do
            {
                // Free any preexiting VARSTRING
                if ( lpVarString != NULL )
                    free(lpVarString);

                // Allocate the VARSTRING structure
                lpVarString = malloc( dwSizeofVarString ) ;
                if (lpVarString == NULL)
                    goto ErrorConnecting;
                lpVarString->dwTotalSize = dwSizeofVarString;

                // Fill the VARSTRING structure
                lReturn = (*cklineGetID)(0, 0, g_hCall, LINECALLSELECT_CALL,
                    lpVarString, "comm/datamodem");

                if (HandleLineErr(lReturn))
                    ; // Still need to check if structure was big enough.
                else
                {
                    OutputDebugLineError(lReturn,
                        "lineGetID unhandled error:");
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


            // Again, the handle to the comm port is contained in a
            // LPVARSTRING structure.  Thus, the handle is the very first
            // thing after the end of the structure.  Note that the name of
            // the comm port is right after the handle, but I don't want it.
            debug(F111,"TAPI DoLineCallState","ttyfd",ttyfd);
            (HANDLE) ttyfd =
                *((LPHANDLE)((LPBYTE)lpVarString +
                    lpVarString -> dwStringOffset));
            debug(F111,"TAPI DoLineCallState","ttyfd",ttyfd);
            lReturn = SetCommMask( (HANDLE) ttyfd, EV_RXCHAR ) ;
            debug(F111,"TAPI DoLineCallState","SetCommMask",lReturn);
            lReturn = SetupComm( (HANDLE) ttyfd, 20000, 20000 ) ;
            debug(F111,"TAPI DoLineCallState","SetupComm",lReturn);
            lReturn = PurgeComm( (HANDLE) ttyfd,
                       PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR );
            debug(F111,"TAPI DoLineCallState","PurgeComm",lReturn);

          ErrorConnecting:
            PostTAPIConnectSem() ;
            debug(F110,"TAPI DoLineCallState","TAPI Connect Sem posted",0);

            if (lpVarString)
                free(lpVarString);
            break;
        }

    case LINECALLSTATE_OFFERING: {
        int rc;
        LPLINECALLINFO lpCallInfo = NULL;
        DWORD          dwSize     = sizeof(LINECALLINFO);
        LONG           lrc = 0;
        DWORD          dwMediaMode= LINEMEDIAMODE_UNKNOWN;
        DWORD          dwNumRings = 0;

        UpdateStatusBar("Incoming call being offered",1,0);
        g_hCall = (HCALL) dwDevice;

        /* Check Media Mode */

        /* The next four lines prepare a VARSTRING structure to
        pass to Windows though lineGetID */
        lpCallInfo = (LPLINECALLINFO) malloc (dwSize);
        if ( !lpCallInfo ) {
            cktapidisconnect();
            return ;
        }
        memset(lpCallInfo,0,dwSize);
        lpCallInfo->dwTotalSize = dwSize;

        do {
            /* get modem handle associated with the line */
            lrc = (*cklineGetCallInfo)(g_hCall, lpCallInfo);
            if ( ( lrc == LINEERR_STRUCTURETOOSMALL || lrc == 0 ) &&
                 (lpCallInfo->dwTotalSize < lpCallInfo->dwNeededSize) ) {
                /* the next six lines reallocate the VARSTRING */
                dwSize = lpCallInfo->dwNeededSize ;
                free(lpCallInfo);
                lpCallInfo = (LPLINECALLINFO) malloc(dwSize);
                if ( !lpCallInfo ) {
                    cktapidisconnect();
                    return;
                }
                memset(lpCallInfo,0,dwSize);
                lpCallInfo->dwTotalSize = dwSize ;
            }
            else if ( lrc ) {
                /* some kind of TAPI error */
                OutputDebugLastError(lrc,"lineGetCallInfo:");
                free(lpCallInfo);
                cktapidisconnect();
                return;
            }
            else
                break;  /* success */
        } while ( TRUE );

        dwMediaMode = lpCallInfo->dwMediaMode;
        free(lpCallInfo);

#ifdef BETADEBUG
        /* what is the media mode */
        if ( dwMediaMode & LINEMEDIAMODE_UNKNOWN )
            printf("  MediaMode is UNKNOWN (it may be one of the following)\n");
        if ( dwMediaMode & LINEMEDIAMODE_INTERACTIVEVOICE )
            printf("  MediaMode is Interactive Voice\n");
        if ( dwMediaMode & LINEMEDIAMODE_AUTOMATEDVOICE )
            printf("  MediaMode is Automated Voice\n");
        if ( dwMediaMode & LINEMEDIAMODE_TDD )
            printf("  MediaMode is TDD\n");
        if ( dwMediaMode & LINEMEDIAMODE_DIGITALDATA )
            printf("  MediaMode is Digital Data\n");
        if ( dwMediaMode & LINEMEDIAMODE_TELETEX )
            printf("  MediaMode is TeleTex\n");
        if ( dwMediaMode & LINEMEDIAMODE_VIDEOTEX )
            printf("  MediaMode is VideoTex\n");
        if ( dwMediaMode & LINEMEDIAMODE_MIXED )
            printf("  MediaMode is Mixed\n");
        if ( dwMediaMode & LINEMEDIAMODE_ADSI )
            printf("  MediaMode is ADSI\n");
        if ( dwMediaMode & LINEMEDIAMODE_VOICEVIEW )
            printf("  MediaMode is VoiceView\n");
        if ( dwMediaMode & LINEMEDIAMODE_DATAMODEM )
            printf("  MediaMode is DataModem\n");
        if ( dwMediaMode & LINEMEDIAMODE_G3FAX )
            printf("  MediaMode is G3Fax\n");
        if ( dwMediaMode & LINEMEDIAMODE_G4FAX )
            printf("  MediaMode is G4Fax\n");
        if ( dwMediaMode > LAST_LINEMEDIAMODE*2-1 )
            printf("  MediaMode is %x\n", dwMediaMode );
#endif /* BETADEBUG */

#ifdef COMMENT
        /* Activate Media Mode Monitoring by the Service Provider */
        (*cklineMonitorMedia)( g_hCall,
                               dwMediaMode & ~LINEMEDIAMODE_UNKNOWN );
#endif /* COMMENT */

#ifdef BETADEBUG
        (*cklineGetNumRings)(g_hLine,0,&dwNumRings);
        printf("  User requests we wait %d rings before answering.\n",
                dwNumRings);
#endif /* BETADEBUG */

        /* Get the hCall */
        if ( dwParam3 == LINECALLPRIVILEGE_OWNER ) {

            /* Need to check the Bearer Mode of the call */

            /* are we even answering calls? */
            if ( g_bAnswering )
                PostTAPIAnswerSem();
            else {
                rc = (*cklineHandoff)(g_hCall, NULL, LINEMEDIAMODE_DATAMODEM);
                OutputDebugLastError(rc,"lineHandoff:");
                rc = (*cklineDeallocateCall)(g_hCall);
                OutputDebugLastError(rc,"lineDeallocateCall:");
                if ( rc < 0 ) {
                    cktapidisconnect();
                }
                else {
                    g_hCall = (HCALL)0;
                }
            }
        }
        else /* if ( dwParam3 == LINECALLPRIVILEGE_MONITOR ) */ {
            if ( g_bAnswering &&
                 !(dwMediaMode & ~(LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_UNKNOWN))
                 ) {
                rc = (*cklineSetCallPrivilege)( g_hCall, LINECALLPRIVILEGE_OWNER );
                OutputDebugLastError(rc,"lineSetCallPrivilege:");
                if ( !rc ) {
                    PostTAPIAnswerSem();
                }
                else {
                    rc = (*cklineDeallocateCall)(g_hCall);
                    OutputDebugLastError(rc,"lineDeallocateCall:");
                    g_hCall = (HCALL)0;
                }
            }
            else {
                rc = (*cklineDeallocateCall)(g_hCall);
                OutputDebugLastError(rc,"lineDeallocateCall:");
                g_hCall = (HCALL)0;
            }
        }
        break;
    }

    case LINECALLSTATE_ACCEPTED:
        UpdateStatusBar("Incoming call has been accepted",1,0);
        break;

    default:
        UpdateStatusBar("?Unhandled LINECALLSTATE message",1,0);
        break;
    }
}

void
CALLBACK
cklineCallbackFunc( DWORD dwDevice, DWORD dwMsg,
                    DWORD dwCallbackInstance,
                    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{

    OutputDebugLineCallback(
        dwDevice, dwMsg, dwCallbackInstance,
        dwParam1, dwParam2, dwParam3);

    // All we do is dispatch the dwMsg to the correct handler.
    switch(dwMsg)
    {
    case LINE_CALLSTATE:
        debug(F110,"TAPI Callback","Line Callstate",0);
#ifdef BETADEBUG
        printf("TAPI Callback - Line Callstate %x,%x,%x\n",
                dwParam1, dwParam2, dwParam3);
#endif
        DoLineCallState(dwDevice, dwMsg, dwCallbackInstance,
                         dwParam1, dwParam2, dwParam3);
        break;

        case LINE_CLOSE:
        debug(F110,"TAPI Callback","Line Close",0);
#ifdef BETADEBUG
        printf("TAPI Callback - Line Close\n");
#endif
        DoLineClose(dwDevice, dwMsg, dwCallbackInstance,
                     dwParam1, dwParam2, dwParam3);
        break;

        case LINE_LINEDEVSTATE:
        debug(F110,"TAPI Callback","Line LineDevState",0);
#ifdef BETADEBUG
        printf("TAPI Callback - Line LineDevState %x,%x,%x\n",
                dwParam1, dwParam2, dwParam3);
#endif
        DoLineDevState(dwDevice, dwMsg, dwCallbackInstance,
                        dwParam1, dwParam2, dwParam3);
        break;

        case LINE_REPLY:
        debug(F110,"TAPI Callback","Line Reply",0);
#ifdef BETADEBUG
        printf("TAPI Callback - Line Reply %x,%x,%x\n",
                dwParam1, dwParam2, dwParam3);
#endif
        DoLineReply(dwDevice, dwMsg, dwCallbackInstance,
                     dwParam1, dwParam2, dwParam3);
        break;

        case LINE_CREATE:
        debug(F110,"TAPI Callback","Line Create",0);
#ifdef BETADEBUG
        printf("TAPI Callback - Line Create\n");
#endif
        DoLineCreate(dwDevice, dwMsg, dwCallbackInstance,
                      dwParam1, dwParam2, dwParam3);
        break;

        default:
        debug(F110,"TAPI Callback","default",0);
#ifdef BETADEBUG
        printf("TAPI Callback - default\n");
#endif
        break;
    }

    return;

}

BOOL
HandleLineErr(long lLineErr)
{
    debug(F101,"TAPI HandleLineErr","",lLineErr);
    if ( lLineErr )
        return FALSE;
    else
        return TRUE ;
}

BOOL
HandleNoDevicesInstalled(void)
{
    return FALSE;
}

BOOL g_bLineInitialized = FALSE;
void
wintapiinit( void )
{
    long lReturn;
    BOOL bTryReInit = TRUE ;

    // Initialize TAPI
    do
    {
        debug(F111,"TAPI wintapiinit","g_hLineApp",g_hLineApp);
        debug(F111,"TAPI wintapiinit","hInst",hInst);
        debug(F111,"TAPI wintapiinit","cklineCallbackFunc",cklineCallbackFunc);
        debug(F111,"TAPI wintapiinit","g_dwNumDevs",g_dwNumDevs);

        lReturn = (*cklineInitialize)(&g_hLineApp, hInst,
             cklineCallbackFunc, "Kermit-95", &g_dwNumDevs);
        OutputDebugLastError(lReturn,"lineInitialize:");

        // If we get this error, its because some other app has yet
        // to respond to the REINIT message.  Wait 5 seconds and try
        // again.  If it still doesn't respond, tell the user.
        if (lReturn == LINEERR_REINIT)
        {
            if (bTryReInit)
            {
                sleep(5);       /* wait 5 secs, try again */
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
                PostTAPIInitSem();
                return;
            }
        }

        if (lReturn == LINEERR_NODEVICE)
        {
            if (HandleNoDevicesInstalled())
                continue;
            else
            {
                printf("No devices installed.\n");
                g_bInitializing = FALSE;
                PostTAPIInitSem();
                return;
            }
        }

        if (HandleLineErr(lReturn))
            continue;
        else
        {
            OutputDebugLineError(lReturn,
                "lineInitialize unhandled error: ");
            g_bInitializing = FALSE;
            PostTAPIInitSem();
            return;
        }
    }
    while(lReturn != SUCCESS);

    debug(F111,"TAPI wintapiinit SUCCESS","g_hLineApp",g_hLineApp);
    debug(F111,"TAPI wintapiinit SUCCESS","hInst",hInst);
    debug(F111,"TAPI wintapiinit SUCCESS","cklineCallbackFunc",cklineCallbackFunc);
    debug(F111,"TAPI wintapiinit SUCCESS","g_dwNumDevs",g_dwNumDevs);

    g_bLineInitialized = TRUE;
    PostTAPIInitSem();
    debug(F111,"TAPI wintapiinit SUCCESS","g_bLineInitialized",g_bLineInitialized);
}

void
wintapishutdown(void)
{
    DWORD rc ;
    debug(F101,"wintapishutdown","",g_hLineApp);
    rc = (*cklineShutdown)( g_hLineApp ) ;
    OutputDebugLastError(rc,"lineShutdown:");
    g_hLineApp = (HLINEAPP)0 ;
    PostTAPIInitSem();
}
int
cktapiopen(void)
{
    debug(F111,"TAPI cktapiopen","tapiopen",tapiopen);
    if ( g_bClosing ) {
        int n=0;
        if ( dialdpy )
            printf("Waiting for previous Close request to complete...\n");
        debug(F110,"TAPI cktapiopen","Close in progress",0);
        while ( g_bClosing && n++ < 30)
            msleep(500);
        if ( n >= 30 ) {
            debug(F110,"TAPI cktapiopen","Close never completed",0);
            printf("?ERROR in Windows Telephony: close never completed.\n");
            bleep(BP_FAIL);
            return(FALSE);
        }
    }
    if ( tapiopen )
    {
        tapiopen++ ;
        debug(F111,"TAPI cktapiopen","tapiopen",tapiopen);
        return TRUE ;
    }

    debug(F110,"TAPI cktapiopen","sending OPT_TAPI_INIT",0);
    g_bLineInitialized = FALSE;
    ResetTAPIInitSem();
    PostMessage( hwndGUI, OPT_TAPI_INIT, 0, 0 ) ;
    /* Wait for completion */
    if (!WaitAndResetTAPIInitSem(60000))
    {
        debug(F110,"TAPI cktapiopen","TAPIInitSem never completed",0);
        return FALSE;
    }

    if ( !g_bLineInitialized ) {
        debug(F110,"TAPI cktapiopen","Line not Initialized",0);
        return FALSE;
    }

    tapiopen++;
    debug(F111,"TAPI cktapiopen","tapiopen",tapiopen);
    return TRUE;
}

int
cktapidevenum( void )
{
    int i = 0 ;
    DWORD dwAPIVersion ;
    LINEEXTENSIONID ExtensionID ;
    int datalines = 0 ;
    LONG rc;

    debug(F111,"TAPI Enumerate Devices","g_dwNumDevs",g_dwNumDevs);

    /* Free existing LineDevCaps */
    for ( i = 0 ; i < g_dwNumDevs ; i++ )
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
        if ( g_lpLineDevCaps[i] == NULL ) {
            debug(F111,"TAPI cktapidevenum",
                   "unable to allocate memory for Device Caps",i);
            continue;
        }
        g_lpLineDevCaps[i]->dwTotalSize = sizeof(LINEDEVCAPS) ;
        if ( rc = (*cklineNegotiateAPIVersion)(g_hLineApp, i,
                                           TAPI_CURRENT_VERSION,
                                           TAPI_CURRENT_VERSION,
                                           &dwAPIVersion, &ExtensionID))
        {
            OutputDebugLastError(rc,"lineNegotiateAPIVersion:");
            free(g_lpLineDevCaps[i]);
            g_lpLineDevCaps[i] = NULL ;
            continue;
        }
        if (rc = (*cklineGetDevCaps)(g_hLineApp, i,
                                 dwAPIVersion, 0, g_lpLineDevCaps[i]))
        {
            OutputDebugLastError(rc,"lineGetDevCaps:");
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

            if ((*cklineGetDevCaps)(g_hLineApp, i,
                               dwAPIVersion, 0, g_lpLineDevCaps[i]))
            {
                OutputDebugLastError(rc,"lineGetDevCaps:");
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
    debug(F111,"TAPI Enumerate Devices","datalines",datalines);
    return datalines ;
}

int
cktapihangup( void )
{
    int rc;

    if ( g_bHangingUp )
    {
        int n=0;
        debug(F110,"TAPI Hangup","already in progress",0);
        while (g_bHangingUp && n++ < 30) {
            msleep(500);
        }
        if (n < 30) {
            debug(F110,"TAPI Hangup","previous hangup request completed",0);
            return TRUE;
        }
        else {
            debug(F110,"TAPI Hangup","previous hangup request didn't complete",0);
            g_bHangingUp = FALSE;
        }
    }

    debug(F111,"TAPI Hangup","hCall",g_hCall);
    if ( g_hCall )
    {
#ifdef BETADEBUG
        if ( tapipass )
            printf( "TAPI: Closing the line\n");
        else
            printf("TAPI: Hanging up call\n");
#endif
        g_bReplyReceived = FALSE;
        g_lAsyncReply = SUCCESS;
        g_bHangingUp     = TRUE;

        if ( tapipass ) {
            /* lineSetCallParams */
            g_dwRequestedID = 0;
            rc = g_dwRequestedID =
                (cklineSetCallParams)(g_hCall,LINEBEARERMODE_VOICE,3100,3100,NULL);
            OutputDebugLastError(rc,"lineSetCallParams:");
            if ( rc > 0 ) {
                int x=0;
                /* Wait for LineReply */
                while ( !g_bReplyReceived ) {
                    msleep(100);
                    x += 100;
                    if ( x == 5000 ) {
                        debug(F110,"TAPI Hangup","lineSetCallParams reply never received",0);
                        break;
                    }
                }
                if ( x < 5000 )
                    OutputDebugLastError(g_lAsyncReply,"lineSetCallParams (Async):");
            }
        }

        if ( dialdpy && !tapipass )
            printf("Dropping the call, wait...\n");
        g_bReplyReceived = FALSE;
        g_lAsyncReply = SUCCESS;
        g_dwRequestedID = 0;
        rc = g_dwRequestedID = (*cklineDrop)(g_hCall, NULL, 0 );
        OutputDebugLastError(rc,"lineDrop:");

        if ( rc > 0 ) {
            int x=0;
            /* Wait for LineReply */
            while ( !g_bReplyReceived && g_hCall != (HCALL)0 ) {
                msleep(100);
                x += 100;
                if ( x == 30000 ) {
                    debug(F110,"TAPI Hangup","lineDrop reply never received",0);
                    g_bHangingUp = FALSE;
                    if ( !g_bClosing ) {
                        return cktapiclose();
                    }
                    return FALSE;
                }
            }
            OutputDebugLastError(g_lAsyncReply,"lineDrop (Async):");

            if ( g_hCall != (HCALL)0 ) {
                rc = (*cklineDeallocateCall)(g_hCall);
                OutputDebugLastError(rc,"TAPI Hangup - lineDeallocateCall:");
                g_hCall = (HCALL)0;
            }
        }
        else {
            // g_hCall = NULL;
            g_bHangingUp     = FALSE;
            return FALSE;
        }
        g_bHangingUp     = FALSE;
    }

    if ( g_hLine != (HLINE)0 ) {
        debug(F111,"TAPI Hangup","ttyfd",ttyfd);
        if ( ttyfd != -1 && ttyfd != -2 )
        {
            rc = CloseHandle( (HANDLE) ttyfd );
            debug(F111,"TAPI Debug Last Error",
                   rc?"CloseHandle: Success" : "CloseHandle: Failure",
                   rc?0:GetLastError());
            ttyfd = -2;
        }
    }
    return TRUE;
}


int
cktapiclose(void)
{
    DWORD rc ;

    debug(F111,"TAPI cktapiclose","g_bClosing",g_bClosing);
    if ( g_bClosing ) {
        int n=0;
        debug(F110,"TAPI Close","already in progress",0);
        while ( g_bClosing && n++ < 30 )
            msleep(500);
        if ( n >= 30 ) {
            debug(F110,"TAPI Close","Previous Close never completed",0);
            return FALSE;
        }
        return TRUE;
    }

    debug(F111,"TAPI cktapiclose","tapiopen",tapiopen);
    if ( tapiopen > 0 )
    {
        tapiopen-- ;
        if ( tapiopen == 1 ) {
            g_bClosing = TRUE;
            debug(F100,"TAPI Close","",0);
            cktapihangup();
            if ( g_hLine )
            {
                debug(F101,"Calling TAPI lineClose","",g_hLine);
                rc = (*cklineClose)( g_hLine );
                OutputDebugLastError(rc,"lineClose:");
                g_hLine = (HLINE)0 ;
                ttyfd = -1;
            }
        }
        if ( tapiopen == 0 ) {
            ResetTAPIInitSem();
            PostMessage( hwndGUI, OPT_TAPI_SHUTDOWN, 0, 0 ) ;
            /* Wait for completion */
            if (!WaitAndResetTAPIInitSem(60000))
            {
                debug(F100,"TAPIInitSem never completed","",0);
                return FALSE;
            }
        }
        g_bClosing = FALSE;
    }
    debug(F100,"TAPI Close successful","",0);
    return TRUE;
}

int
cktapiconfigline()
{
    LPDEVCFG        lpDevCfg = NULL;
    LPCOMMCONFIG    lpCommConfig = NULL;
    LPMODEMSETTINGS lpModemSettings = NULL;
    DCB *           lpDCB = NULL;

    if (!cktapiGetModemSettings( &lpDevCfg,
                                 &lpModemSettings,
                                 &lpCommConfig,
                                 &lpDCB )) {
        return FALSE;
    }

    if ( !tapiusecfg ) {
        /* we need to set TAPI values for
         tapilights;
         tapipreterm;
         tapipostterm;
         tapimanual;
         tapiinactivity;
         tapibong;
         */

        lpDevCfg->dfgHdr.fwOptions = (tapilights ? 8 : 0)
        + (tapipreterm ? 1 : 0)
            + (tapipostterm ? 2 : 0)
                + (tapimanual ? 4 : 0) ;
        lpDevCfg->dfgHdr.wWaitBong = tapibong;
        lpModemSettings->dwCallSetupFailTimer = waitct;
        lpModemSettings->dwInactivityTimeout = tapiinactivity;

        lpDCB->BaudRate = speed;

        lpDCB->fBinary = TRUE ;
        lpDCB->fErrorChar = FALSE ;
        lpDCB->fAbortOnError = FALSE ;
        lpDCB->ErrorChar = '?' ;
        lpDCB->Parity = NOPARITY ;
        lpDCB->fParity = TRUE ;    /* Tell Win32 not to perform any Parity checking at all */
        lpDCB->ByteSize = 8 ;
        lpDCB->StopBits = ONESTOPBIT ;

        lpDCB->fDsrSensitivity = FALSE ;
        lpDCB->fDtrControl = DTR_CONTROL_ENABLE ;

#ifndef NODIAL
        if ( dialec )
            lpModemSettings->dwPreferredModemOptions |= MDM_ERROR_CONTROL;
        else
            lpModemSettings->dwPreferredModemOptions &= ~MDM_ERROR_CONTROL;

        if ( dialdc )
            lpModemSettings->dwPreferredModemOptions |= MDM_COMPRESSION;
        else
            lpModemSettings->dwPreferredModemOptions &= ~MDM_COMPRESSION;

#ifdef COMMENT
        if ( mdmspd )
            lpModemSettings->dwPreferredModemOptions |= MDM_SPEED_ADJUST;
        else
            lpModemSettings->dwPreferredModemOptions &= ~MDM_SPEED_ADJUST;
#endif /* COMMENT */

        if ( dialidt )
            lpModemSettings->dwPreferredModemOptions |= MDM_BLIND_DIAL;
        else
            lpModemSettings->dwPreferredModemOptions &= ~MDM_BLIND_DIAL;

        if ( dialmth == XYDM_T ) {
            lpModemSettings->dwPreferredModemOptions |= MDM_TONE_DIAL;
            dialmauto = 0;
        } else if ( dialmth == XYDM_D ) {
            lpModemSettings->dwPreferredModemOptions &= ~MDM_TONE_DIAL;
            dialmauto = 0;
        }

        if ( dialfc == FLO_XONX ) {
            lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_HARD;
            lpModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_SOFT;
        }
        else if ( dialfc == FLO_RTSC ) {
            lpModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_HARD;
            lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_SOFT;
        }
        else if ( dialfc == FLO_NONE ) {
            lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_HARD;
            lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_SOFT;
        }
#endif /* NODIAL */

        switch(flow) {
        case FLO_XONX:
            lpDCB->fOutX = TRUE ;
            lpDCB->fInX = TRUE ;
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE ;
            lpDCB->fOutxCtsFlow = FALSE ;
            lpDCB->fTXContinueOnXoff = FALSE ;
#ifndef NODIAL
            if ( dialfc == FLO_AUTO ) 
#endif /* NODIAL */
            {
                lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_HARD;
                lpModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_SOFT;
            }
            break;
        case FLO_RTSC:
            lpDCB->fOutX = FALSE ;
            lpDCB->fInX = FALSE ;
            lpDCB->fRtsControl = RTS_CONTROL_HANDSHAKE ;
            lpDCB->fOutxCtsFlow = TRUE ;
            lpDCB->fTXContinueOnXoff = TRUE ;
#ifndef NODIAL
            if ( dialfc == FLO_AUTO )
#endif /* NODIAL */
            {
                lpModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_HARD;
                lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_SOFT;
            }
            break;
        case FLO_KEEP:
            /* leave things exactly as they are */
            break;
        case FLO_NONE:
            /* turn off all flow control completely */
            lpDCB->fOutX = FALSE ;
            lpDCB->fInX = FALSE ;
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE ;
            lpDCB->fOutxCtsFlow = FALSE ;
            lpDCB->fTXContinueOnXoff = TRUE ;
#ifndef NODIAL
            if ( dialfc == FLO_AUTO ) 
#endif /* NODIAL */
            {
                lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_HARD;
                lpModemSettings->dwPreferredModemOptions &= ~MDM_FLOWCONTROL_SOFT;
            }
            break;
        }

#ifndef NODIAL
        /* Speaker settings */
        lpModemSettings->dwSpeakerMode   = mdmspk;
        lpModemSettings->dwSpeakerVolume = mdmvol-1;
#endif /* NODIAL */

        lpDCB->XonChar = 0x11 ;
        lpDCB->XoffChar = 0x13;
        lpDCB->XonLim = 10 ;
        lpDCB->XoffLim = 10 ;
    }

    if (!cktapiSetModemSettings(lpDevCfg,lpCommConfig)) {
        return FALSE;
    }

    return (TRUE);

}

#ifndef NODIAL
int
cktapidial(char * number)
{
    LINECALLPARAMS lcp;
    int rc=0;

    fail_code = 0;
    if ( tapipass ) {
        debug(F110,"cktapidial - should never have been called",number,0);
        return FALSE;
    }

    debug(F110,"cktapidial",number,0);

    g_bReplyReceived = FALSE;
    g_bConnected = FALSE;
    g_bDialing = TRUE;
    g_lAsyncReply = SUCCESS;
    ResetTAPIConnectSem();

    if ( g_hCall == (HCALL)0 ) {
        cktapiconfigline();

        memset(&lcp,0,sizeof(LINECALLPARAMS));
        lcp.dwBearerMode = LINEBEARERMODE_VOICE;
        lcp.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
        lcp.dwTotalSize = sizeof(LINECALLPARAMS);
        lcp.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE | LINECALLPARAMFLAGS_SECURE;
#ifdef COMMENT
        lcp.dwNoAnswerTimeout = waitct; /* TAPI 2.0 */
#endif /* COMMENT */
        g_dwRequestedID = 0;
        rc = g_dwRequestedID = (*cklineMakeCall)( g_hLine, &g_hCall, number, 0, &lcp ) ;
        OutputDebugLastError(rc,"lineMakeCall:");
    }
    else {
        g_dwRequestedID = 0;
        rc = g_dwRequestedID = (*cklineDial)( g_hCall, number, 0 ) ;
        OutputDebugLastError(rc,"lineDial:");
    }
    if ( rc < 0 )
    {
       if ( rc == LINEERR_CALLUNAVAIL ||
            rc == LINEERR_INUSE ||
            rc == LINEERR_NODEVICE ) {
           fail_code = F_MINIT;
       }

#ifdef BETADEBUG
        printf("lineMakeCall failed rc=%x\n",rc ) ;
#endif
        if ( dialdpy )
            printf("Call failed: %s\n",cktapiErrorString(rc));
        g_hCall = (HCALL)0;
        cktapiclose();
        g_bDialing = FALSE;
        return FALSE;
    }
    else {
        int x = 0;
#ifdef BETADEBUG
        printf("lineMakeCall successful\n");
#endif
        /* Wait for LineReply */
        while ( !g_bReplyReceived && fail_code == 0 ) {
            msleep(100);
            x += 100;
            if ( x == 5000 ) {
                debug(F110,"cktapidial",
                       "lineMakeCall/Dial Reply never received",0);
                break;
            }
        }

        if ( fail_code == 0 )
            OutputDebugLastError(g_lAsyncReply,
                                  "lineMakeCall/Dial (Async):");
        else
            debug(F111,"cktapidial","fail_code",fail_code);

        if ( fail_code == F_INT ||
             fail_code == F_MINIT ) {     /* Catch early if possible */
            cktapihangup();
            g_bDialing = FALSE;
            return FALSE;
        }

        if ( g_lAsyncReply != SUCCESS )
        {
#ifdef BETADEBUG
            printf("lineMakeCall failed (Async Reply) rc=%x\n",g_lAsyncReply ) ;
#endif
            if ( dialdpy )
                printf("Call failed: %s\n",cktapiErrorString(g_lAsyncReply));
            g_hCall = (HCALL)0;
            cktapiclose();
            g_bDialing = FALSE;
            return FALSE;
        }

        /* Are we partial dialing? */
        if ( number[strlen(number)-1] == ';' )
            return TRUE;

        /* Wait for Success or Failure of Connection */
        if (!WaitAndResetTAPIConnectSem(waitct*1000))
        {
            debug(F101,"TAPI Connect Timeout","",waitct);
#ifdef BETADEBUG
            printf("TAPI connect semaphore never posted\n");
#endif
            if ( dialdpy )
                printf("Call failed: Timeout\n");
            g_bDialing = FALSE;
            return FALSE;
        }
        if ( fail_code == F_INT ||
             fail_code == F_MINIT ) {
            cktapihangup();
            g_bDialing = FALSE;
            return FALSE;
        }
    }

    if ( !g_bConnected ) {
        cktapihangup();
        g_bDialing = FALSE;
        return FALSE;
    }

    if ( ttyfd == -1 || ttyfd == -2 ) {
        /* if we did not get the Comm handle via the CONNECT message */
        /* then get it now                                           */
        ttyfd = (int) GetModemHandleFromLine( g_hLine );
        SetCommMask( (HANDLE) ttyfd, EV_RXCHAR ) ;
        SetupComm( (HANDLE) ttyfd, 20000, 20000 ) ;
        PurgeComm( (HANDLE) ttyfd,
                   PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR ) ;
    }
    if ( !ttyfd || ttyfd == -1 ) {
        ttyfd = -2 ;
        g_bDialing = FALSE;
        return FALSE;
    }
#ifdef BETADEBUG
        DisplayCommProperties((HANDLE)ttyfd);
#endif /* BETADEBUG */

    g_bDialing = FALSE;
    return TRUE;
}

int
cktapianswer( void )
{
    int rc=0,x=0;
    extern int dialatmo;
    char * filename=NULL, *p=NULL;

    filename = GetLoadPath();
    if ( filename && *filename ) {
        p = filename + strlen(filename);
        while ( p != filename ) {
            if ( *p == '\\' || *p == '/' )
            {
                filename = p + 1 ;
                break;
            }
            p--;
        }
        if ( filename >= p ) {
            rc = (*cklineSetAppPriority)(filename,LINEMEDIAMODE_DATAMODEM,
                                     NULL,0,NULL,1);
            OutputDebugLastError(rc,"lineSetAppPriority:");
#ifdef BETADEBUG
            printf("  lineSetAppPriority rc=0x%x\n",rc);
#endif
        }
    }

    /* Wait for LINECALLSTATE_OFFERING to get the hCall */
    fail_code = 0;
    g_bAnswering = TRUE;
    ResetTAPIAnswerSem();
    rc = WaitAndResetTAPIAnswerSem(dialatmo<=0?INFINITE:dialatmo*1000);

    if ( !rc ) {
#ifdef BETADEBUG
        printf("TAPI Answer Sem never SET\n",rc ) ;
#endif
        if ( dialdpy )
            printf("Answer failed: Timeout\n");
        return FALSE;
    }
    if ( fail_code == F_INT ) {
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }

    /* Then use lineAnswer() to CONNECT to the call */
    g_bReplyReceived = FALSE;
    g_lAsyncReply = SUCCESS;
    g_bConnected = FALSE;
    ResetTAPIConnectSem();

    g_dwRequestedID = 0;
    rc = g_dwRequestedID = (*cklineAccept)( g_hCall, NULL, 0) ;
    OutputDebugLastError(rc,"lineAccept:");
    if ( rc < 0 )
    {
#ifdef BETADEBUG
        printf("lineAccept failed rc=%x\n",rc ) ;
#endif
        if ( dialdpy )
            printf("Answer failed: %s\n",cktapiErrorString(rc));
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }

    /* Wait for LineReply */
    while ( !g_bReplyReceived && fail_code == 0 ) {
        msleep(100);
        x += 100;
        if ( x == 5000 ) {
            debug(F110,"cktapianswer",
                   "lineAccept Reply never received",0);
            if ( dialdpy )
                printf("Answer failed: required TAPI reply never received.\n");
            g_bAnswering = FALSE;
            cktapihangup();
            return FALSE;
        }
    }

    if ( g_lAsyncReply != SUCCESS && g_lAsyncReply != LINEERR_OPERATIONUNAVAIL)
    {
#ifdef BETADEBUG
        printf("lineAccept failed (Async Reply) rc=%x\n",g_lAsyncReply ) ;
#endif
        if ( dialdpy )
            printf("Answer failed: %s\n",cktapiErrorString(g_lAsyncReply));
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }
#ifdef BETADEBUG
    printf("lineAccept successful\n");
#endif

    g_bReplyReceived = FALSE;
    g_lAsyncReply = SUCCESS;
    g_dwRequestedID = 0;
    rc = g_dwRequestedID = (*cklineAnswer)( g_hCall, NULL, 0) ;
    OutputDebugLastError(rc,"lineAnswer:");
    if ( rc < 0 )
    {
#ifdef BETADEBUG
        printf("lineAnswer failed rc=%x\n",rc ) ;
#endif
        if ( dialdpy )
            printf("Answer failed: %s\n",cktapiErrorString(rc));
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }

    /* Wait for LineReply */
    while ( !g_bReplyReceived && fail_code == 0 ) {
        msleep(100);
        x += 100;
        if ( x == waitct*1000 ) {
            debug(F110,"cktapianswer",
                   "lineAnswer Reply never received",0);
            if ( dialdpy )
                printf("Answer failed: required TAPI reply never received.\n");
            g_bAnswering = FALSE;
            cktapihangup();
            return FALSE;
        }
    }

    OutputDebugLastError(g_lAsyncReply,"lineAnswer (Async):");

    if ( g_lAsyncReply  != SUCCESS )
    {
#ifdef BETADEBUG
        printf("lineAnswer failed (Async Reply) rc=%x\n",g_lAsyncReply ) ;
#endif
        if ( dialdpy )
            printf("Answer failed: %s\n",cktapiErrorString(g_lAsyncReply));
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }

#ifdef BETADEBUG
    printf("lineAnswer successful\n");
#endif

    /* Wait for Success or Failure of Connection */
    if (!WaitAndResetTAPIConnectSem(waitct>0?waitct*1000:60000))
    {
#ifdef BETADEBUG
        printf("TAPI connect semaphore never posted\n");
#endif
        if ( dialdpy )
            printf("Answer failed: Carrier Detect Timeout\n");
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }
    if ( fail_code == F_INT || !g_bConnected ) {
        g_bAnswering = FALSE;
        cktapihangup();
        return FALSE;
    }

#ifdef BETADEBUG
    /* But is the call of the correct Media Mode? */
    {
        LPLINECALLINFO lpCallInfo = NULL;
        DWORD          dwSize     = sizeof(LINECALLINFO);
        LONG           lrc = 0;

        lpCallInfo = (LPLINECALLINFO) malloc (dwSize);
        if ( !lpCallInfo ) {
            g_bAnswering = FALSE;
            cktapihangup();
            return FALSE;
        }
        memset(lpCallInfo,0,dwSize);
        lpCallInfo->dwTotalSize = dwSize;

        do {
            /* get modem handle associated with the line */
            lrc = (*cklineGetCallInfo)(g_hCall, lpCallInfo);
            if ( ( lrc == LINEERR_STRUCTURETOOSMALL || lrc == 0 ) &&
                 (lpCallInfo->dwTotalSize < lpCallInfo->dwNeededSize) ) {
                /* the next six lines reallocate the VARSTRING */
                dwSize = lpCallInfo->dwNeededSize ;
                free(lpCallInfo);
                lpCallInfo = (LPLINECALLINFO) malloc(dwSize);
                if ( !lpCallInfo ) {
                    g_bAnswering = FALSE;
                    cktapihangup();
                    return FALSE;
                }
                memset(lpCallInfo,0,dwSize);
                lpCallInfo->dwTotalSize = dwSize ;
            }
            else if ( lrc ) {
                /* some kind of TAPI error */
                OutputDebugLastError(lrc,"lineGetCallInfo:");
                free(lpCallInfo);
                g_bAnswering = FALSE;
                cktapihangup();
                return FALSE;
            }
            else
                break;  /* success */
        } while ( TRUE );

        /* what is the media mode */
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_UNKNOWN )
            printf("  MediaMode is UNKNOWN (it may be one of the following)\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_INTERACTIVEVOICE )
            printf("  MediaMode is Interactive Voice\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_AUTOMATEDVOICE )
            printf("  MediaMode is Automated Voice\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_TDD )
            printf("  MediaMode is TDD\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_DIGITALDATA )
            printf("  MediaMode is Digital Data\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_TELETEX )
            printf("  MediaMode is TeleTex\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_VIDEOTEX )
            printf("  MediaMode is VideoTex\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_MIXED )
            printf("  MediaMode is Mixed\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_ADSI )
            printf("  MediaMode is ADSI\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_VOICEVIEW )
            printf("  MediaMode is VoiceView\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_DATAMODEM )
            printf("  MediaMode is DataModem\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_G3FAX )
            printf("  MediaMode is G3Fax\n");
        if ( lpCallInfo->dwMediaMode & LINEMEDIAMODE_G4FAX )
            printf("  MediaMode is G4Fax\n");
        if ( lpCallInfo->dwMediaMode > LAST_LINEMEDIAMODE*2-1 )
            printf("  MediaMode is %x\n", lpCallInfo->dwMediaMode );
        free(lpCallInfo);
    }
#endif /* BETADEBUG */

    if ( ttyfd == -1 || ttyfd == -2 ) {
        /* if we did not get the Comm handle via the CONNECT message */
        /* then get it now                                           */
        ttyfd = (int) GetModemHandleFromLine( g_hLine );
        SetCommMask( (HANDLE) ttyfd, EV_RXCHAR ) ;
        SetupComm( (HANDLE) ttyfd, 20000, 20000 ) ;
        PurgeComm( (HANDLE) ttyfd,
                   PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR ) ;
    }
    if ( !ttyfd || ttyfd == -1 ) {
        ttyfd = -2 ;
        g_bAnswering = FALSE;
        return FALSE;
    }

#ifdef BETADEBUG
    DisplayCommProperties((HANDLE)ttyfd);
#endif /* BETADEBUG */

    /* turn off media montoring since we have already accepted the call */
    /* as a data modem call                                             */
    (*cklineMonitorMedia)(g_hCall,0);
    g_bAnswering = FALSE;
    return TRUE;
}
#endif /* NODIAL */

int
cktapiBuildLineTable( struct keytab ** pTable, struct keytab ** pTable2, int * pN )
{
    int i, n ;
    if ( *pTable )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable)[i].kwd ) ;
        free ( *pTable )  ;
    }

    if ( *pTable2 )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable2)[i].kwd ) ;
        free ( *pTable2 )  ;
    }

    *pTable = NULL ;
    *pTable2 = NULL ;
    *pN = 0 ;

    if (!TAPIAvail)
        return(0);

    cktapiopen() ;
    n = cktapidevenum() ;
    cktapiclose() ;

    debug(F100,"TAPI cktapiBuildLineTable","",0);
    if ( n )
    {
        *pTable = malloc( sizeof(struct keytab) * n ) ;
        *pTable2 = malloc( sizeof(struct keytab) * n ) ;

        for ( i=0 ; i < g_dwNumDevs ; i++ ) {
            if ( g_lpLineDevCaps[i] == NULL )
            {
                debug(F111,"TAPI LineDevCaps entry is NULL","i",i);
            }
            else if ( g_lpLineDevCaps[i]->dwMediaModes & LINEMEDIAMODE_DATAMODEM )
            {
               char * newstr = _strdup( ((char *)(g_lpLineDevCaps[i]))+g_lpLineDevCaps[i]->dwLineNameOffset) ;
               char * newstr2 = _strdup( ((char *)(g_lpLineDevCaps[i]))+g_lpLineDevCaps[i]->dwLineNameOffset) ;
               int    newval = i;
               int j = 0, len = 0;

               debug(F111,"TAPI LINEMEDIAMODE_DATAMODEM",newstr,i);

               /* Make a version that uses Underscores instead of spaces */
               len = strlen(newstr2);
               for ( j=0 ; j<len ; j++) {
                   switch ( newstr2[j] ) {
                   case ' ':
                       newstr2[j] = '_';
                       break;
                   case ',':
                       newstr2[j] = '.';
                       break;
                   case ';':
                       newstr2[j] = ':';
                       break;
                   case '\\':
                       newstr2[j] = '/';
                       break;
                   case '?':
                       newstr2[j] = '!';
                       break;
                   case '{':
                       newstr2[j] = '[';
                       break;
                   case '}':
                       newstr2[j] = ']';
                       break;
                   }
               }

               for ( j=0 ; j < (*pN) ; j++ ) {
                   int tempval=0;
                   char * tempstr=NULL,*tempstr2=NULL;

                   if ( _stricmp( (*pTable)[j].kwd, newstr ) > 0 )
                   {
                       tempval = (*pTable)[j].kwval;
                       tempstr = (*pTable)[j].kwd;
                       tempstr2 = (*pTable2)[j].kwd;
                       (*pTable)[j].kwd = newstr ;
                       (*pTable)[j].kwval = newval;
                       (*pTable2)[j].kwd = newstr2 ;
                       (*pTable2)[j].kwval = newval;
                       newval = tempval;
                       newstr = tempstr;
                       (*pTable)[j].flgs = 0;
                       newstr2 = tempstr2;
                       (*pTable2)[j].flgs = 0;
                   }
               }
               (*pTable)[*pN].kwd = newstr ;
               (*pTable)[*pN].kwval = newval;
               (*pTable)[*pN].flgs = 0 ;
               (*pTable2)[*pN].kwd = newstr2 ;
               (*pTable2)[*pN].kwval = newval;
               (*pTable2)[*pN].flgs = 0 ;
               (*pN)++ ;
           }
            else
            {
               debug(F111,"TAPI ~LINEMEDIAMODE_DATAMODEM",
                      ((char *)(g_lpLineDevCaps[i]))+g_lpLineDevCaps[i]->dwLineNameOffset,
                      i);
            }
       }

       if ( *pN == 0 ) {
           /* TAPI Devices exist, but none can be used by Kermit */
           free ( *pTable )  ;
           free ( *pTable2 )  ;
           *pTable = NULL;
           *pTable2 = NULL;
       }
   }
    return(1);
}


int
cktapiBuildLocationTable( struct keytab ** pTable, int * pN )
{
   LPLINETRANSLATECAPS lpTranslateCaps = NULL;
   DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
   long lReturn = 0;
   DWORD dwCounter;
   LPLINELOCATIONENTRY lpLocationEntry;
   LPLINECARDENTRY lpLineCardEntry = NULL;
   int i = 0 ;

   if ( *pTable )
   {
      for ( i=0 ; i < *pN ; i++ )
         free( (*pTable)[i].kwd ) ;
      free ( *pTable )  ;
   }
   *pTable = NULL ;
   *pN = 0 ;

    if (!TAPIAvail)
        return(0);

    cktapiopen() ;

   // First, get the TRANSLATECAPS
   do
   {
      lpTranslateCaps = (LPLINETRANSLATECAPS) CheckAndReAllocBuffer(
                 (LPVOID) lpTranslateCaps, dwSizeofTranslateCaps,
                 "cktapiBuildLocationTable");

      if (lpTranslateCaps == NULL)
      {
         cktapiclose();
         return 0;
      }

      lReturn = (*cklineGetTranslateCaps)(g_hLineApp, TAPI_CURRENT_VERSION,
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

    debug(F100,"TAPI cktapiBuildLocationTable","",0);
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
          char * newstr = _strdup((((LPBYTE) lpTranslateCaps) +
                                        lpLocationEntry[dwCounter].dwLocationNameOffset)) ;
          int    newval = lpLocationEntry[dwCounter].dwPermanentLocationID;
          int j = 0, len = 0;

          debug(F111,"TAPI lpLocationEntry.dwLocationName",newstr,i);

          /* Use Underscores instead of spaces */
          len = strlen(newstr);
          for ( j=0;j<len;j++ )
          {
              switch ( newstr[j] ) {
              case ' ':
                  newstr[j] = '_';
                  break;
              case ',':
                  newstr[j] = '.';
                  break;
              case ';':
                  newstr[j] = ':';
                  break;
              case '\\':
                  newstr[j] = '/';
                  break;
              case '?':
                  newstr[j] = '!';
                  break;
              case '{':
                  newstr[j] = '[';
                  break;
              case '}':
                  newstr[j] = ']';
                  break;
              }
          }

          for ( j=0 ; j < dwCounter ; j++ ) {
              int tempval=0;
              char * tempstr=NULL;

              if ( _stricmp( (*pTable)[j].kwd, newstr ) > 0 )
              {
                  tempval = (*pTable)[j].kwval;
                  tempstr = (*pTable)[j].kwd;
                  (*pTable)[j].kwd = newstr ;
                  (*pTable)[j].kwval = newval;
                  newval = tempval;
                  newstr = tempstr;
                  (*pTable)[j].flgs = 0;
              }
          }
          (*pTable)[*pN].kwd = newstr;
          (*pTable)[*pN].kwval = newval ;
          (*pTable)[*pN].flgs = 0 ;
          (*pN)++ ;
      }
    }
    LocalFree(lpTranslateCaps);
    return(1);
}

int
cktapiGetCurrentLocationID( void )
{
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
    long lReturn;
    int  locationID=-1;

    debug(F100,"cktapiGetCurrentLocationID","",0);

    // First, get the TRANSLATECAPS
    do
    {
        lpTranslateCaps =
            (LPLINETRANSLATECAPS) CheckAndReAllocBuffer((LPVOID) lpTranslateCaps,
                                                         dwSizeofTranslateCaps,
                                                         "cktapiGetCurrentLocationID");

        if (lpTranslateCaps == NULL) {
            debug(F110,"cktapiGetCurrentLocationID","lpTranslateCaps == NULL",0);
            return -1;
        }


        lReturn = (*cklineGetTranslateCaps)(g_hLineApp, TAPI_CURRENT_VERSION,
                                             lpTranslateCaps);
        debug(F111,"cktapiGetCurrentLocationID","cklineGetTranslateCaps returns",lReturn);

        if (!HandleLineErr(lReturn))
        {
            OutputDebugLineError(lReturn,
                                  "lineGetTranslateCaps unhandled error: ");
            LocalFree(lpTranslateCaps);
            return -1;
        }

        if ((lpTranslateCaps -> dwNeededSize) >
             (lpTranslateCaps -> dwTotalSize))
        {
            dwSizeofTranslateCaps = lpTranslateCaps ->dwNeededSize;
            lReturn = -1; // Lets loop again.
        }
    }
    while(lReturn != SUCCESS);

    locationID = lpTranslateCaps->dwCurrentLocationID;

    LocalFree(lpTranslateCaps);
    debug(F111,"cktapiGetCurrentLocationID","locationID",locationID);
    return locationID;
}

void
cktapiDisplayTapiLocationInfo( void )
{
    printf("TAPI Location (%d): %s\n", tapilocid, tapiloc );
    printf("  Country ID         = %d\n", tapiCountryID);
    printf("  CountryCode        = %d\n", tapiCountryCode);
    printf("   CountryName       = %s\n", tapiCountryName);
    printf("   SameAreaRule      = %s\n", tapiSameAreaRule);
    printf("   LongDistanceRule  = %s\n", tapiLongDistanceRule);
    printf("   InternationalRule = %s\n", tapiInternationalRule);
    printf("  AreaCode           = %s\n", tapiAreaCode);
    printf("  PreferredCardID    = %d\n", tapiPreferredCardID);
    printf("  LocalAccessCode    = %s\n", tapiLocalAccessCode);
    printf("  LongDistAccessCode = %s\n", tapiLongDistAccessCode);
    printf("  TollPrefixList     = %s\n", tapiTollPrefixList);
    printf("  CancelCallWaiting  = %s\n", tapiCancelCallWaiting);
    printf("  Options            = %d\n", tapiOptions);
}

void
SaveTapiLocationInfo( LPLINETRANSLATECAPS lpTranslateCaps,
                      LPLINELOCATIONENTRY lpLocationEntry )
{
    LPLINECOUNTRYLIST  lpCountryList  = NULL;
    LPLINECOUNTRYENTRY lpCountryEntry = NULL;
    DWORD dwSizeofCountryList = sizeof(LINECOUNTRYLIST);
    long lReturn=0;
    int i=0;

    debug(F111,"SaveTapiLocationInfo","lpTranslateCaps",lpTranslateCaps);
    debug(F111,"SaveTapiLocationInfo","lpLocationEntry",lpLocationEntry);

    tapilocid = lpLocationEntry->dwPermanentLocationID;
    tapiCountryID = lpLocationEntry->dwCountryID;
    tapiCountryCode = lpLocationEntry->dwCountryCode;
    strncpy(tapiloc,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwLocationNameOffset),
             lpLocationEntry->dwLocationNameSize >= 256 ? 256 :
             lpLocationEntry->dwLocationNameSize);
    tapiloc[256]='\0';
    debug(F111,"SaveTapiLocationInfo",tapiloc,lpLocationEntry->dwLocationNameSize);

    do
    {
        lpCountryList =
            (LPLINECOUNTRYLIST) CheckAndReAllocBuffer((LPVOID) lpCountryList,
                                                         dwSizeofCountryList,
                                                         "SaveTapiLocationInfo");

        if (lpCountryList == NULL)
            return;

        /* Win95 doesn't return anything is CountryID is 1 */
        lReturn = (*cklineGetCountry)(isWin95()?0:tapiCountryID, TAPI_CURRENT_VERSION, lpCountryList);

        if (HandleLineErr(lReturn))
            ;
        else
        {
            OutputDebugLineError(lReturn,
                                  "lineGetCountry unhandled error: ");
            LocalFree(lpCountryList);
            return;
        }

        if ((lpCountryList -> dwNeededSize) >
             (lpCountryList -> dwTotalSize))
        {
            dwSizeofCountryList = lpCountryList ->dwNeededSize;
            lReturn = -1; // Lets loop again.
        }
    }
    while(lReturn != SUCCESS);

    for ( i=0; i<lpCountryList->dwNumCountries; i++)
    {
        lpCountryEntry = (LPLINECOUNTRYENTRY) ((BYTE *)lpCountryList +
                                                lpCountryList->dwCountryListOffset +
                                                (i * sizeof(LINECOUNTRYENTRY)));
        if ( lpCountryEntry->dwCountryID == tapiCountryID )
            break;
    }
    if ( i == lpCountryList->dwNumCountries )
        lpCountryEntry = NULL;

    if ( lpCountryEntry ) {
        /* 'E' = Country Code */
        /* 'F' = Area Code    */
        /* 'G' = Local Number */
        strncpy(tapiCountryName,
                 ((LPSTR) lpCountryList +
                   lpCountryEntry->dwCountryNameOffset),
                 lpCountryEntry->dwCountryNameSize >= 64 ? 64 :
                 lpCountryEntry->dwCountryNameSize);
        tapiCountryName[64]='\0';

        strncpy(tapiSameAreaRule,((LPSTR) lpCountryList +
                                  lpCountryEntry->dwSameAreaRuleOffset),
                 lpCountryEntry->dwSameAreaRuleSize >= 64 ? 64 :
                 lpCountryEntry->dwSameAreaRuleSize);
        tapiSameAreaRule[64]='\0';

        strncpy(tapiLongDistanceRule,((LPSTR) lpCountryList +
                                  lpCountryEntry->dwLongDistanceRuleOffset),
                 lpCountryEntry->dwLongDistanceRuleSize >= 64 ? 64 :
                 lpCountryEntry->dwLongDistanceRuleSize);
        tapiLongDistanceRule[64]='\0';

        strncpy(tapiInternationalRule,((LPSTR) lpCountryList +
                                  lpCountryEntry->dwInternationalRuleOffset),
                 lpCountryEntry->dwInternationalRuleSize >= 64 ? 64 :
                 lpCountryEntry->dwInternationalRuleSize);
        tapiInternationalRule[64]='\0';
    }
    else {
        tapiCountryName[0] = '\0';
        tapiSameAreaRule[0] = '\0';
        tapiLongDistanceRule[0] = '\0';
        tapiInternationalRule[0] = '\0';
    }

    strncpy(tapiAreaCode,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwCityCodeOffset),
             lpLocationEntry->dwCityCodeSize >= 64 ? 64 :
             lpLocationEntry->dwCityCodeSize);
    tapiAreaCode[64]='\0';

    tapiPreferredCardID = lpLocationEntry->dwPreferredCardID;

    tapiOptions = lpLocationEntry->dwOptions;

    strncpy(tapiLocalAccessCode,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwLocalAccessCodeOffset),
             lpLocationEntry->dwLocalAccessCodeSize >= 64 ? 64 :
             lpLocationEntry->dwLocalAccessCodeSize);
    tapiLocalAccessCode[64]='\0';

    strncpy(tapiLongDistAccessCode,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwLongDistanceAccessCodeOffset),
             lpLocationEntry->dwLongDistanceAccessCodeSize >= 64 ? 64 :
             lpLocationEntry->dwLongDistanceAccessCodeSize);
    tapiLongDistAccessCode[64]='\0';

    strncpy(tapiTollPrefixList,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwTollPrefixListOffset),
             lpLocationEntry->dwTollPrefixListSize >= 4096 ? 4096 :
             lpLocationEntry->dwTollPrefixListSize);
    tapiTollPrefixList[4096]='\0';

    strncpy(tapiCancelCallWaiting,
             (((LPSTR) lpTranslateCaps) +
               lpLocationEntry->dwCancelCallWaitingOffset),
             lpLocationEntry->dwCancelCallWaitingSize >= 64 ? 64 :
             lpLocationEntry->dwCancelCallWaitingSize);
    tapiCancelCallWaiting[64]='\0';

#ifdef BETADEBUG
    cktapiDisplayTapiLocationInfo();
#endif /* BETADEBUG */

    LocalFree(lpCountryList);
}

int
cktapiFetchLocationInfoByName( char * CurrentLocation )
{
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
    long lReturn;
    DWORD dwCounter;
    LPLINELOCATIONENTRY lpLocationEntry;
    LPLINECARDENTRY lpLineCardEntry = NULL;

    /* If no name specified, use Current Location */
    if ( !CurrentLocation || !CurrentLocation[0] ) {
        int ID = cktapiGetCurrentLocationID();
        return cktapiFetchLocationInfoByID( ID );
    }

    // First, get the TRANSLATECAPS
    do
    {
        lpTranslateCaps = (LPLINETRANSLATECAPS) CheckAndReAllocBuffer(
                 (LPVOID) lpTranslateCaps, dwSizeofTranslateCaps,
                 "cktapiFetchLocationInfoByName");

        if (lpTranslateCaps == NULL)
            return FALSE;

        lReturn = (*cklineGetTranslateCaps)(g_hLineApp, TAPI_CURRENT_VERSION,
                                      lpTranslateCaps);

        if (HandleLineErr(lReturn))
            ;
        else
        {
            OutputDebugLineError(lReturn,
                                  "lineGetTranslateCaps unhandled error: ");
            LocalFree(lpTranslateCaps);
            return FALSE;
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
    debug(F110,"cktapiFetchLocationInfoByName","lpLocationEntry",lpLocationEntry);

    // loop through all locations, looking for a location match
    for(dwCounter = 0;
         dwCounter < lpTranslateCaps -> dwNumLocations;
         dwCounter++)
    {
        if (_stricmp((((LPSTR) lpTranslateCaps) +
                       lpLocationEntry[dwCounter].dwLocationNameOffset),
                      CurrentLocation)
             == 0)
        {
            // Found it!  Set the current location, if necessary.
            int ID = cktapiGetCurrentLocationID();
            if ( ID != lpLocationEntry[dwCounter].dwPermanentLocationID )
            {
                lReturn =
                    (*cklineSetCurrentLocation)(g_hLineApp,
                        lpLocationEntry[dwCounter].dwPermanentLocationID);
                OutputDebugLastError(lReturn,"lineSetCurrentLocation:");
            }
            break;
        }
    }

    if ( dwCounter == lpTranslateCaps -> dwNumLocations ) {
        debug(F110,"cktapiFetchLocationInfoByName","LocationEntry not found",0);
        LocalFree(lpTranslateCaps);
        return FALSE;
    }

    // Was a match for lpszCurrentLocation found?
    if (lpLocationEntry[dwCounter].dwPreferredCardID == MAXDWORD)
    {
        debug(F110,"cktapiFetchLocationInfoByName","LocationEntry not found",0);
        LocalFree(lpTranslateCaps);
        return FALSE;
    }

    SaveTapiLocationInfo( lpTranslateCaps, &lpLocationEntry[dwCounter] );
    LocalFree(lpTranslateCaps);
    return TRUE;
}

int
cktapiFetchLocationInfoByID( int LocationID )
{
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
    long lReturn;
    DWORD dwCounter;
    LPLINELOCATIONENTRY lpLocationEntry;
    LPLINECARDENTRY lpLineCardEntry = NULL;

   // First, get the TRANSLATECAPS
   do
   {
      lpTranslateCaps = (LPLINETRANSLATECAPS) CheckAndReAllocBuffer(
                 (LPVOID) lpTranslateCaps, dwSizeofTranslateCaps,
                 "cktapiFetchLocationInfoByID");

      if (lpTranslateCaps == NULL)
         return FALSE;

      lReturn = (*cklineGetTranslateCaps)(g_hLineApp, TAPI_CURRENT_VERSION,
                                      lpTranslateCaps);

      if (HandleLineErr(lReturn))
         ;
      else
      {
         OutputDebugLineError(lReturn,
                               "lineGetTranslateCaps unhandled error: ");
         LocalFree(lpTranslateCaps);
         return FALSE;
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

    debug(F110,"cktapiFetchLocationInfoByID","lpLocationEntry",lpLocationEntry);

    // If lpszCurrentLocation, then make that location 'current'
    if (LocationID == -1)
    {
        LocationID = cktapiGetCurrentLocationID();
    }

    // loop through all locations, looking for a location match
    for(dwCounter = 0;
         dwCounter < lpTranslateCaps -> dwNumLocations;
         dwCounter++)
    {
        if ( LocationID == lpLocationEntry[dwCounter].dwPermanentLocationID )
        {
            // Found it!  Set the current location, if necessary.
            int ID = cktapiGetCurrentLocationID();
            if ( ID != lpLocationEntry[dwCounter].dwPermanentLocationID )
            {
                lReturn =
                    (*cklineSetCurrentLocation)(g_hLineApp,
                        lpLocationEntry[dwCounter].dwPermanentLocationID);
                OutputDebugLastError(lReturn,"lineSetCurrentLocation:");
            }
            break;
        }
    }

    if ( dwCounter == lpTranslateCaps -> dwNumLocations ) {
        debug(F110,"cktapiFetchLocationInfoByID","LocationEntry not found",0);
        LocalFree(lpTranslateCaps);
        return FALSE;
    }

    // Was a match for lpszCurrentLocation found?
    if (lpLocationEntry[dwCounter].dwPreferredCardID == MAXDWORD)
    {
        debug(F110,"cktapiFetchLocationInfoByID","PreferredCardID == MAXDWORD",0);
        LocalFree(lpTranslateCaps);
        return FALSE;
    }

    SaveTapiLocationInfo( lpTranslateCaps, &lpLocationEntry[dwCounter] );
    LocalFree(lpTranslateCaps);
    return TRUE;
}

#ifndef NODIAL
void
CopyTapiLocationInfoToKermitDialCmd( void )
{
    extern char *dialnpr;               /* DIAL PREFIX */
    extern char *diallac;               /* DIAL LOCAL-AREA-CODE */
    extern char *diallcc;               /* DIAL LOCAL-COUNTRY-CODE */
    extern char *dialixp;               /* DIAL INTL-PREFIX */
    extern char *dialixs;               /* DIAL INTL-SUFFIX */
    extern char *diallcp;               /* DIAL LOCAL-PREFIX */
    extern char *diallcs;               /* DIAL LOCAL-SUFFIX */
    extern char *dialldp;               /* DIAL LD-PREFIX */
    extern char *diallds;               /* DIAL LD-SUFFIX */
    extern char *dialpxx;               /* DIAL PBX-EXCHANGE */
    extern char *dialpxi;               /* DIAL INTERNAL-PREFIX */
    extern char *dialpxo;               /* DIAL OUTSIDE-PREFIX */
    extern char *dialsfx;               /* DIAL SUFFIX */
    extern char *dialtfp;               /* DIAL TOLL-FREE-PREFIX */
    extern int dialmth;                 /* DIAL METHOD TONE/PULSE */
    char * p = NULL;

    debug(F111,"CopyTapiLocationInfoToKermitDialCmd","tapilocid",tapilocid);

    /* Make sure there are valid values */
    if ( tapilocid == -1 ) {
        tapilocid = cktapiGetCurrentLocationID();
        if ( tapilocid == -1 ) {
            debug(F111,"CopyTapiLocationInfoToKermitDialCmd",
                   "tapilocid still -1",tapilocid);
            return;
        }
        cktapiFetchLocationInfoByID( tapilocid );
    }

    debug(F110,"CopyTapiLocationInfoToKermitDialCmd",
           "copying string values",0);

    /* Local Country Code */
    if ( diallcc )
        free( diallcc );
    diallcc = (char *) malloc( 20 ) ;
    if ( !diallcc )
        return;
    _itoa( tapiCountryCode, diallcc, 10 );

    /* Local Area Code */
    if ( diallac )
        free(diallac);
    diallac = strdup( tapiAreaCode );

    /* Long Distance Prefix */
    if ( dialldp )
        free(dialldp);
    /* The true Long Distance Prefix is the tapiLongDistAccessCode */
    /* plus the prefix portion of the tapiLongDistanceRule         */
    dialldp = (char *) malloc(strlen(tapiLongDistAccessCode) +
                               strlen(tapiLongDistanceRule) + 1);
    if ( dialldp ) {
        strcpy(dialldp,tapiLongDistAccessCode);
        p = dialldp;
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
        strcpy(p,tapiLongDistanceRule);
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
    }

    /* The Long Distance Suffix is the suffix portion of the       */
    /* tapiLongDistanceRule (if any)                               */
    if ( diallds )
        free(diallds);
    diallds = strdup( tapiLongDistanceRule );
    if ( diallds ) {
        p = diallds;
        /* blank out the prefix */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G') {
            *p = ' ';
            p++;
        }
        /* and then the dialing rule */
        while ( *p == 'E' || *p == 'F' || *p == 'G' ) {
            *p = ' ';
            p++;
        }
        if ( *p == '\0' ) {
            free(diallds);
            diallds = NULL;
        }
    }

    /* Toll Free Prefix */
    if ( dialtfp )
        free(dialtfp);
    /* The true Toll Free Prefix is the tapiLongDistAccessCode */
    /* plus the prefix portion of the tapiLongDistanceRule         */
    dialtfp = (char *) malloc(strlen(tapiLongDistAccessCode) +
                               strlen(tapiLongDistanceRule) + 1);
    if ( dialtfp ) {
        strcpy(dialtfp,tapiLongDistAccessCode);
        p = dialtfp;
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
        strcpy(p,tapiLongDistanceRule);
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
    }

    /* The International Prefix is the prefix portion of the       */
    /* tapiInternationalRule         */
    if ( dialixp )
        free(dialixp);
    /* The true International Prefix is the tapiLongDistAccessCode */
    /* plus the prefix portion of the tapiInternationalRule        */
    dialixp = (char *) malloc(strlen(tapiLongDistAccessCode) +
                               strlen(tapiInternationalRule) + 1);
    if ( dialixp ) {
        strcpy(dialixp,tapiLongDistAccessCode);
        p = dialixp;
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
        strcpy(p,tapiInternationalRule);
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
    }

    /* The International Suffix is the suffix portion of the       */
    /* tapiInternationalRule (if any)                              */
    if ( dialixs )
        free(dialixs);
    dialixs = strdup( tapiInternationalRule );
    if ( dialixs ) {
        p = dialixs;
        /* blank out the prefix */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G') {
            *p = ' ';
            p++;
        }
        /* and then the dialing rule */
        while ( *p == 'E' || *p == 'F' || *p == 'G' ) {
            *p = ' ';
            p++;
        }
        if ( *p == '\0' ) {
            free(dialixs);
            dialixs = NULL;
        }
    }

    /* Local Prefix */
    if ( diallcp )
        free(diallcp);
    /* The true Local Prefix is the tapiLocalAccessCode */
    /* plus the prefix portion of the tapiSameAreaRule         */
    diallcp = (char *) malloc(strlen(tapiLocalAccessCode) +
                               strlen(tapiSameAreaRule) + 1);
    if ( diallcp ) {
        strcpy(diallcp,tapiLocalAccessCode);
        p = diallcp;
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
        strcpy(p,tapiSameAreaRule);
        /* find the beginning of the prefix  */
        while ( *p == ' ' || *p == 'N' || *p == 'S' ) {
            *p = ' ';
            p++;
        }
        /* find the end of the prefix and null terminate it */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G' )
            p++;
        *p = '\0';
    }

    /* The Local Access Suffix is the suffix portion of the       */
    /* tapiLocalAccessRule (if any)                                */
    if ( diallcs )
        free(diallcs);
    diallcs = strdup( tapiSameAreaRule );
    if ( diallcs ) {
        p = diallcs;
        /* blank out the prefix */
        while ( *p && *p != 'E' && *p != 'F' && *p != 'G') {
            *p = ' ';
            p++;
        }
        /* and then the dialing rule */
        while ( *p == 'E' || *p == 'F' || *p == 'G' ) {
            *p = ' ';
            p++;
        }
        if ( *p == '\0' ) {
            free(diallcs);
            diallcs = NULL;
        }
    }

    /* All purpose Prefix */
    if ( dialnpr )
        free( dialnpr );
    dialnpr = strdup( tapiCancelCallWaiting );

    /* Pulse Dialing? */
    if ( tapiOptions & LINELOCATIONOPTION_PULSEDIAL )
        dialmth = XYDM_P;
    else
        dialmth = XYDM_T;

    debug(F110,"CopyTapiLocationInfoToKermitDialCmd","done",0);
}
#endif /* NODIAL */

typedef struct tagCommID {
    HANDLE hModem ;
    char szModemName[1] ;
} CommID ;

HANDLE
GetModemHandleFromLine( HLINE hLine )
{
    CommID * cid;
    VARSTRING *vs;
    ULONG lrc ;
    DWORD dwSize;
    HANDLE hModem ;

    if ( hLine == (HLINE) 0 )
        hLine = g_hLine;

    if ( hLine == (HLINE) 0 )
        return NULL;

    /* The next four lines prepare a VARSTRING structure to
    pass to Windows though lineGetID */
    vs = (VARSTRING *) malloc (1024);
    if ( !vs )
        return NULL;
    memset(vs,0,1024);
    vs->dwTotalSize = 1024;

    do {
        /* get modem handle associated with the line */
        lrc = (*cklineGetID)(hLine, 0L, (HCALL)0,
                              LINECALLSELECT_LINE,
                              vs, "comm/datamodem") ;
        if ( ( lrc == LINEERR_STRUCTURETOOSMALL || lrc == 0 ) &&
             (vs->dwTotalSize < vs->dwNeededSize) ) {
            /* the next six lines reallocate the VARSTRING */
            dwSize = vs->dwNeededSize ;
            free(vs);
            vs = (VARSTRING *) malloc(dwSize);
            if ( !vs )
                return NULL;
            memset(vs,0,sizeof(vs));
            vs->dwTotalSize = dwSize ;
        }
        else if ( lrc ) {
            /* some kind of TAPI error */
            OutputDebugLastError(lrc,"lineGetID:");
            free(vs);
            return NULL;
        }
        else
            break;  /* success */
    } while ( TRUE );

    cid = (CommID *) ((LPSTR)vs + vs->dwStringOffset);

    if ( !cid->hModem ) {
        SECURITY_ATTRIBUTES security ;

        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.lpSecurityDescriptor = NULL ;
        security.bInheritHandle = TRUE ;
        strcpy( szModemName, "\\\\.\\" ) ;
        strcat( szModemName, &cid->szModemName[0] ) ;
        hModem = CreateFile( szModemName,
                             GENERIC_READ | GENERIC_WRITE,
                             TRUE, /* do not share */
                             &security,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             NULL);
    }
    else {
        lstrcpy( szModemName, &cid->szModemName[0] ) ;
        hModem = cid->hModem ;
    }

    free(vs);
    return hModem;
}

void
cktapiConfigureLine( int lineID )
{
    LPVARSTRING lpVarString = NULL;
    DWORD dwSizeofVarString = sizeof(VARSTRING) + 1024;
    LPCSTR binaryConfigString=NULL;
    LONG lReturn=-1;

    if ( !cktapiopen() )
        return;

    if ( lineID == -1 )
        lineID = LineDeviceId;

    debug(F111,"TAPI Configure Line Dialog","lineID",lineID);
    (*cklineConfigDialog)( lineID, hwndConsole, NULL );

    do
    {
        if ( lpVarString != NULL )
            free(lpVarString);

        // Allocate the VARSTRING structure
        lpVarString = malloc( dwSizeofVarString ) ;
        if (lpVarString == NULL)
            goto ErrorConfiguring;
        lpVarString->dwTotalSize = dwSizeofVarString;

        // Fill the VARSTRING structure
        lReturn = (*cklineGetDevConfig)( lineID, lpVarString, "comm/datamodem" );
        OutputDebugLastError(lReturn,"lineGetDevConfig:");

        if (HandleLineErr(lReturn))
            ; // Still need to check if structure was big enough.
            else
            {
                OutputDebugLineError(lReturn,
                                      "lineGetDevConfig unhandled error: ");
                goto ErrorConfiguring;
            }

        // If the VARSTRING wasn't big enough, loop again.
        if ((lpVarString -> dwNeededSize) > (lpVarString -> dwTotalSize))
        {
            dwSizeofVarString = lpVarString -> dwNeededSize;
            lReturn = -1; // Lets loop again.
        }
    }
    while(lReturn != SUCCESS);

    /* See cktapiGetModemSettings; we may know the format of this string */
    binaryConfigString = ((LPCSTR)((LPBYTE)lpVarString +
                                    lpVarString->dwStringOffset));

    ErrorConfiguring:
    if (lpVarString)
        free(lpVarString);

    cktapiclose();
}

void
cktapiDialingProp( void )
{
    DWORD dwAPIVersion ;
    LINEEXTENSIONID ExtensionID ;
    LONG rc;

    if ( !cktapiopen() )
        return;

    debug(F111,"TAPI Dialing Properties Dialog","device ID",LineDeviceId);
    rc = (*cklineNegotiateAPIVersion)(g_hLineApp,
                                       LineDeviceId == -1 ? 0 : LineDeviceId,
                                       TAPI_CURRENT_VERSION,
                                       TAPI_CURRENT_VERSION,
                                       &dwAPIVersion, &ExtensionID);
    rc = (*cklineTranslateDialog)( g_hLineApp,
                                   LineDeviceId == -1 ? 0 : LineDeviceId,
                                   dwAPIVersion, hwndConsole, NULL );

    tapilocid = -1;
#ifndef NODIAL
    CopyTapiLocationInfoToKermitDialCmd();
#endif /* NODIAL */
    cktapiclose();
}

int
cktapiConvertPhoneNumber(char * source, char ** converted)
{
    DWORD dwAPIVersion ;
    LINEEXTENSIONID ExtensionID ;
    DWORD dwCard=0;
    DWORD dwTranslateOptions=LINETRANSLATEOPTION_CANCELCALLWAITING;
    LPLINETRANSLATEOUTPUT lplineTranslateOutput=NULL;
    DWORD dwSize=0;
    LONG rc = 0;

    if ( !cktapiopen() )
        return FALSE;

    rc = (*cklineNegotiateAPIVersion)(g_hLineApp,
                                       LineDeviceId == -1 ? 0 : LineDeviceId,
                                       TAPI_CURRENT_VERSION,
                                       TAPI_CURRENT_VERSION,
                                       &dwAPIVersion, &ExtensionID);
    if ( rc ) {
        cktapiclose();
        return FALSE;
    }
    dwSize = sizeof(LINETRANSLATEOUTPUT);
    lplineTranslateOutput = (LPLINETRANSLATEOUTPUT) malloc( dwSize ) ;
    if ( !lplineTranslateOutput ) {
        cktapiclose();
        return FALSE;
    }
    memset(lplineTranslateOutput,0,dwSize);
    lplineTranslateOutput->dwTotalSize = dwSize ;

    do {
        rc = (*cklineTranslateAddress)(g_hLineApp,
                                        LineDeviceId == -1 ? 0 : LineDeviceId,
                                        dwAPIVersion,
                                        source,
                                        dwCard,
                                        dwTranslateOptions,
                                        lplineTranslateOutput
                                        );
        if ( ( rc == LINEERR_STRUCTURETOOSMALL || rc == 0 ) &&
             (lplineTranslateOutput->dwTotalSize < lplineTranslateOutput->dwNeededSize) ) {
            /* the next six lines reallocate the LINETRANSLATEOUTPUT struct */
            dwSize = lplineTranslateOutput->dwNeededSize ;
            free(lplineTranslateOutput);
            lplineTranslateOutput = (LPLINETRANSLATEOUTPUT) malloc( dwSize ) ;
            if ( !lplineTranslateOutput ) {
                cktapiclose();
                return FALSE;
            }
            memset(lplineTranslateOutput,0,dwSize);
            lplineTranslateOutput->dwTotalSize = dwSize ;
        }
        else if ( rc ) {
            /* some kind of TAPI error */
            OutputDebugLastError(rc,"lineTranslateAddress:");
            free(lplineTranslateOutput);
            cktapiclose();
            return FALSE;
        }
        else
            break;  /* success */
    } while ( TRUE );

    /* we now have the result, lets do something with it */
    *converted = (char *) malloc( lplineTranslateOutput->dwDialableStringSize+1);
    strncpy( *converted, (LPCSTR) lplineTranslateOutput +
                         lplineTranslateOutput->dwDialableStringOffset,
             lplineTranslateOutput->dwDialableStringSize);
    (*converted)[lplineTranslateOutput->dwDialableStringSize]='\0';

    free(lplineTranslateOutput);
    cktapiclose();

    return TRUE;
}

int
tapi_open( char * devicename )
{
    extern struct keytab * tapilinetab ;
    extern int ntapiline ;
    int i ;
    LINEEXTENSIONID ExtId ;
    int rc ;
    LPDEVCFG        lpDevCfg = NULL;
    LPCOMMCONFIG    lpCommConfig = NULL;
    LPMODEMSETTINGS lpModemSettings = NULL;
    DCB *           lpDCB = NULL;

    debug(F110,"tapi_open devicename",devicename,0);
    if ( !cktapiopen() )
    {
        debug(F100,"tapi_open !cktapiopen()","",0);
        printf("TAPI Open fails\n");
        return -1;
    }

    /* Find the Line ID */
    for ( i=0 ; i<ntapiline ; i++ )
    {
        // printf("%s == %d\n",tapilinetab[i].kwd, tapilinetab[i].kwval);
        if ( !strcmp( devicename, tapilinetab[i].kwd ) )
        {
            LineDeviceId = tapilinetab[i].kwval ;
            // printf("LineDeviceId == %d\n",LineDeviceId);
            break;
        }
    }

    if ( i == ntapiline )
    {
        debug(F100,"tapi_open device not found","",0);
        printf("%s not in TAPI Device Table\n",devicename);
        cktapiclose();
        return -1 ;
    }

    /* Negotiate API Version for the Line Device */
    {
        rc = (*cklineNegotiateAPIVersion)( g_hLineApp, LineDeviceId,
                                           TAPI_CURRENT_VERSION, TAPI_CURRENT_VERSION,
                                           &LineDeviceAPIVersion,
                                           &ExtId ) ;
        if ( rc < 0 ) {
            debug(F101,"tapi_open cklineNegotiateAPIVersion() failed","",rc);
            printf("Error negotiating LineDevice API Version\n");
            cktapiclose();
            return -1;
        }
    }

    /* Get Addresses and Media modes */
    LineAddressCount = g_lpLineDevCaps[LineDeviceId]->dwNumAddresses ;
    LineMediaModes = g_lpLineDevCaps[LineDeviceId]->dwMediaModes ;

    if ( g_hLine == (HLINE)0 ) {        /* Call lineOpen() */
        rc = -1 ;
        while ( rc < 0 )
        {
            rc = (*cklineOpen)( g_hLineApp, LineDeviceId, &g_hLine,
                                LineDeviceAPIVersion, 0,
                                (DWORD) hInstance,
                                LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_MONITOR,
                                LINEMEDIAMODE_DATAMODEM,
                                NULL);
            OutputDebugLastError(rc,"lineOpen:");
            switch ( rc ) {
            case LINEERR_ALLOCATED:
                printf("TAPI Device already in use by a non-TAPI application.\n");
                cktapiclose();
                return -1;

            default:
                if ( HandleLineErr(rc) )
                {
                    continue;
                }
                else
                {
                    debug(F111,"TAPI lineOpen","g_hLine",g_hLine);
                    printf("Error opening Line Device: %s\n",cktapiErrorString(rc));
                    g_hLine = (HLINE)0;
                    cktapiclose();
                    return -1;
                }
            }
        }
    }

    /* Specify which event messages we are interested in */
    rc = (*cklineSetStatusMessages)(g_hLine,
                                     LINEDEVSTATE_CONNECTED |
                                     LINEDEVSTATE_DISCONNECTED |
                                     LINEDEVSTATE_OUTOFSERVICE |
                                     LINEDEVSTATE_MAINTENANCE |
                                     LINEDEVSTATE_OPEN |
                                     LINEDEVSTATE_CLOSE |
                                     LINEDEVSTATE_RINGING |
                                     LINEDEVSTATE_OTHER |
                                     LINEDEVSTATE_REINIT,
                                     0);
    OutputDebugLastError(rc,"lineSetStatusMessages:");
    if ( rc )
    {
        cktapiclose();
        return -1 ;
    }


    if ( tapipass ) {
        /* Try to make a call without specifying an address */
        LINECALLPARAMS lcp;
        memset(&lcp,0,sizeof(LINECALLPARAMS));
        lcp.dwBearerMode = LINEBEARERMODE_PASSTHROUGH;
        lcp.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
        lcp.dwTotalSize = sizeof(LINECALLPARAMS);
        lcp.dwMinRate = lcp.dwMaxRate = 3100 ;
        lcp.dwAddressMode = LINEADDRESSMODE_ADDRESSID ;
        lcp.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE | LINECALLPARAMFLAGS_SECURE ;

#ifndef NODIAL
        fail_code = 0;
#endif /* NODIAL */
        g_bReplyReceived = FALSE;
        g_lAsyncReply = SUCCESS;
        g_bConnected = FALSE;
        g_bDialing = TRUE;
        ResetTAPIConnectSem();
        g_dwRequestedID = 0;
        rc = g_dwRequestedID = (*cklineMakeCall)( g_hLine, &g_hCall, NULL, 0, &lcp) ;
        OutputDebugLastError(rc,"lineMakeCall:");
        if ( rc < 0 )
        {
#ifdef BETADEBUG
            printf("lineMakeCall failed rc=%x\n",rc ) ;
#endif
            if ( dialdpy )
                printf("Open failed: %s\n",cktapiErrorString(rc));
            g_hCall = (HCALL)0;
            cktapiclose();
            g_bDialing = FALSE;
            return -1;
        }
        else {
            int x = 0;

            /* Wait for LineReply */
            while ( !g_bReplyReceived
#ifndef NODIAL
                    && fail_code == 0
#endif /* NODIAL */
                    ) {
                msleep(100);
                x += 100;
                if ( x == 5000 ) {
                    debug(F110,"tapi_open",
                           "lineMakeCall Reply never received",0);
                    if ( dialdpy )
                        printf("Open failed: required TAPI reply never received.\n");
                    g_hCall = (HCALL)0;
                    cktapiclose();
                    g_bDialing = FALSE;
                    return -1;
                }
            }

            OutputDebugLastError(g_lAsyncReply,"lineMakeCall (Async):");

            if ( g_lAsyncReply  != SUCCESS )
            {
#ifdef BETADEBUG
                printf("lineMakeCall failed rc=%x\n",g_lAsyncReply ) ;
#endif
                if ( dialdpy )
                    printf("Open failed: %s\n",cktapiErrorString(g_lAsyncReply));
                g_hCall = (HCALL)0;
                cktapiclose();
                g_bDialing = FALSE;
                return -1;
            }

            /* Wait for Success or Failure of Connection */
            if (!WaitAndResetTAPIConnectSem(10000))
            {
                debug(F111,"TAPI Connect Timeout","seconds",10);
#ifdef BETADEBUG
                printf("TAPI connect semaphore never posted\n");
#endif
                if ( dialdpy )
                    printf("Open failed: Timeout\n");
                cktapiclose();
                g_bDialing = FALSE;
                return -1;
            }
#ifndef NODIAL
            if ( fail_code == F_INT ) {
                cktapiclose();
                g_bDialing = FALSE;
                return FALSE;
            }
#endif /* NODIAL */
        }

        if ( ttyfd == -1 || ttyfd == -2 ) {
            /* if we did not get the Comm handle via the CONNECT message */
            /* then get it now                                           */
            ttyfd = (int) GetModemHandleFromLine( g_hLine );
            SetCommMask( (HANDLE) ttyfd, EV_RXCHAR ) ;
            SetupComm( (HANDLE) ttyfd, 20000, 20000 ) ;
            PurgeComm( (HANDLE) ttyfd,
                       PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR ) ;
        }
        if ( !ttyfd || ttyfd == -1 ) {
            ttyfd = -1 ;
            cktapiclose();
            g_bDialing = FALSE;
            return -1;
        }
#ifdef BETADEBUG
        DisplayCommProperties((HANDLE)ttyfd);
#endif /* BETADEBUG */
        g_bDialing = FALSE;
    }
    else {                              /* Use TAPI based dialing */
        ttyfd = -2;                     /* We don't have a real handle yet */

        /* Do this just to set the name of the Modem */
        CloseHandle( GetModemHandleFromLine( g_hLine ) );
    }

#ifndef NODIAL
    modemp[n_TAPI] = cktapiGetModemInf(LineDeviceId,(HANDLE)ttyfd);
    if ( !modemp[n_TAPI] )
        modemp[n_TAPI] = &GENERIC;
#endif /* NODIAL */

    rc = cktapiGetModemSettings(&lpDevCfg,&lpModemSettings,&lpCommConfig,&lpDCB);
    debug(F101,"tapi_open cktapiGetModemSettings()","",rc);
    if ( rc )
    {
        extern long speed;
        extern int parity, flow;
        extern int ttmdm;                   /* Modem or Direct */
#ifndef NODIAL
        extern int
            dialec,                         /* DIAL ERROR-CORRECTION */
            dialdc,                         /* DIAL COMPRESSION  */
            dialfc,                         /* DIAL FLOW-CONTROL */
            dialmth,                        /* DIAL METHOD */
            mdmspd,                         /* SPEED-MATCHING */
            mdmspk,                         /* SPEAKER ON/OFF */
            mdmvol;                         /* SPEAKER VOLUME */
        extern int dialtmo;                 /* DIAL TIMEOUT */
#endif /* NODIAL */

        speed   = lpDCB->BaudRate;
        parity  = lpDCB->Parity;
#ifndef NODIAL
        dialtmo = lpModemSettings->dwCallSetupFailTimer;
        dialec  = lpModemSettings->dwPreferredModemOptions & MDM_ERROR_CONTROL;
        dialdc  = lpModemSettings->dwPreferredModemOptions & MDM_COMPRESSION;
#ifdef COMMENT
        mdmspd  = lpModemSettings->dwPreferredModemOptions & MDM_SPEED_ADJUST;
#endif /* COMMENT */
        dialmth = (lpModemSettings->dwPreferredModemOptions & MDM_TONE_DIAL) ?
                  XYDM_T : XYDM_D ;
        dialfc = FLO_AUTO;
        if ( lpModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD )
            flow = FLO_RTSC;
        else if ( lpModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_SOFT )
            flow = FLO_XONX;
        else
            flow = FLO_NONE;

        mdmspk = lpModemSettings->dwSpeakerMode;
        mdmvol = lpModemSettings->dwSpeakerVolume+1;

        debug(F111,"tapi_open","dialtmo",dialtmo);
        debug(F111,"tapi_open","speed",speed);
        debug(F111,"tapi_open","parity",parity);
        debug(F111,"tapi_open","dialec",dialec);
        debug(F111,"tapi_open","dialdc",dialdc);
        debug(F111,"tapi_open","dialmth",dialmth);
        debug(F111,"tapi_open","dialfc",dialfc);
        debug(F111,"tapi_open","flow",flow);
        debug(F111,"tapi_open","mdmspk",mdmspk);
        debug(F111,"tapi_open","mdmvol",mdmvol);
#endif /* NODIAL */
        if ( !tapipass ) {
            if ( cktapiIsModem() )
                ttmdm = 38;     /* ckudia.c */
            else 
                ttmdm = 0;
        }
#ifdef BETADEBUG
        cktapiDisplayModemSettings(lpDevCfg,lpModemSettings,lpCommConfig,lpDCB);
#endif
    }
    return 0;
}


int
tapi_clos( void )
{
   cktapiclose() ;
   return -1;
}

int
cktapiCallInProgress( void )
{
    LPLINEDEVSTATUS lpLineDevStatus=NULL;
    DWORD dwSizeofLineDevStatus = sizeof(LINEDEVSTATUS);
    LONG lReturn;
    int InProgress = TRUE;

    if ( g_hCall != (HCALL)0 )
        return FALSE;

    if ( g_hLine == (HLINE)0 )
        return TRUE;

    do
    {
        lpLineDevStatus =
            (LPLINEDEVSTATUS) CheckAndReAllocBuffer((LPVOID) lpLineDevStatus,
                                                     dwSizeofLineDevStatus,
                                                     "cktapiCallInProgress");
        if (lpLineDevStatus == NULL)
        {
            return TRUE;
        }

        lReturn = (*cklineGetLineDevStatus)(g_hLine, lpLineDevStatus);
        if (HandleLineErr(lReturn))
            ;
        else
        {
            OutputDebugLineError(lReturn,
                                  "lineGetDevStatus unhandled error: ");
            LocalFree(lpLineDevStatus);
            return TRUE;
        }

        if ((lpLineDevStatus -> dwNeededSize) >
             (lpLineDevStatus -> dwTotalSize))
        {
            dwSizeofLineDevStatus = lpLineDevStatus ->dwNeededSize;
            lReturn = -1; // Lets loop again.
        }
    }
    while(lReturn != SUCCESS);

    InProgress = lpLineDevStatus->dwNumActiveCalls
        + lpLineDevStatus->dwNumOnHoldCalls
            + lpLineDevStatus->dwNumOnHoldPendCalls
                - (g_hCall!=(HCALL)0?1:0);

    LocalFree( lpLineDevStatus );
    return InProgress;
}

int
cktapiIsModem(void)
{
    LPCSTR lpModemKey;
    HKEY  hkModemKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;
    int   modem = 0;

    if ( LineDeviceId == (DWORD) -1 || g_lpLineDevCaps[LineDeviceId]->dwDevSpecificSize == 0 ) 
        return 0;
    
    lpModemKey = ((char *)g_lpLineDevCaps[LineDeviceId] + g_lpLineDevCaps[LineDeviceId]->dwDevSpecificOffset + 8);

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpModemKey, 0, KEY_READ, &hkModemKey) )
        return 0;         /* failed */

    if ( !RegOpenKeyEx(hkModemKey, "Answer", 0, KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            modem = strcmp("CLIENTSERVER",lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Settings", 0,
                        KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "DialPrefix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            modem &= strcmp("CLIENT",lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }
    RegCloseKey( hkModemKey );

    return modem;
}

int
cktapiDisplayModemSettings( LPDEVCFG lpDevCfg,
                            LPMODEMSETTINGS lpModemSettings,
                            LPCOMMCONFIG lpCommConfig,
                            DCB *     lpDCB )
{
    printf("TAPI UniModem Configuration:\n");
    printf("  Modem Name: %s\n", szModemName );
    if ( LineDeviceId != (DWORD) -1 &&
         g_lpLineDevCaps[LineDeviceId]->dwDevSpecificSize ) {
        printf("  Registry Key: %s\n",
                (char *)g_lpLineDevCaps[LineDeviceId]
                + g_lpLineDevCaps[LineDeviceId]->dwDevSpecificOffset + 8);
        cktapiDisplayRegistryModemInfo((char *)g_lpLineDevCaps[LineDeviceId]
                + g_lpLineDevCaps[LineDeviceId]->dwDevSpecificOffset + 8);
    }

    if ( lpDevCfg ) {
        printf("  Version                 = %x\n",lpDevCfg->dfgHdr.dwVersion);
        if ( lpDevCfg->dfgHdr.fwOptions == 0 )
            printf("  No Unimodem options\n");
        else {
            printf("  Unimodem options:\n");
            if ( lpDevCfg->dfgHdr.fwOptions & 1 )
                printf("    Pre-dial Terminal Screen\n");
            if ( lpDevCfg->dfgHdr.fwOptions & 2 )
                printf("    Post-dial Terminal Screen\n");
            if ( lpDevCfg->dfgHdr.fwOptions & 4 )
                printf("    Manual Dialing\n");
            if ( lpDevCfg->dfgHdr.fwOptions & 8 )
                printf("    Display Launch Lights (modem tray icon)\n");
        }
        printf("  Wait for bong           = %d seconds\n",lpDevCfg->dfgHdr.wWaitBong);
    }
    if ( lpModemSettings ) {
        printf("  Modem Settings (read/write):\n");
        printf("    CallSetupFailTimer    = %d seconds\n",
                lpModemSettings->dwCallSetupFailTimer);
        printf("    InactivityTimeout     = %d seconds\n",
                lpModemSettings->dwInactivityTimeout);
        printf("    SpeakerVolume         = %d (0-Low,1-Medium,2-High)\n",
                lpModemSettings->dwSpeakerVolume);
        printf("    SpeakerMode           = %d (0-Off,1-Dial,2-On,3-CallSetup)\n",
                lpModemSettings->dwSpeakerMode);
        if ( lpModemSettings->dwPreferredModemOptions == 0 )
            printf("    No Preferred Modem Options\n");
        else {
            printf("    Preferred Modem Options:\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_COMPRESSION )
                printf("      Modem Compression\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_ERROR_CONTROL )
                printf("      Error Control\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_FORCED_EC )
                printf("      Forced Error Control\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_CELLULAR )
                printf("      Cellular\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD )
                printf("      Hardware Flow Control\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_SOFT )
                printf("      Xon/Xoff Flow Control\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_CCITT_OVERRIDE )
                printf("      CCITT Override\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_SPEED_ADJUST )
                printf("      Speed Adjust (Modulation)\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_TONE_DIAL )
                printf("      Tone Dial\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_BLIND_DIAL )
                printf("      Blind Dial\n");
            if ( lpModemSettings->dwPreferredModemOptions & MDM_V23_OVERRIDE )
                printf("      V.23 Override\n");
        }
        printf("  Modem Settings (read only) from current call (if any):\n");
        if ( lpModemSettings->dwNegotiatedModemOptions == 0 )
            printf("    No Negotiated Modem Options\n");
        else {
            printf("    Negotiated Modem Options:\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_COMPRESSION )
                printf("      Modem Compression\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_ERROR_CONTROL )
                printf("      Error Control\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_FORCED_EC )
                printf("      Forced Error Control\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_CELLULAR )
                printf("      Cellular\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_FLOWCONTROL_HARD )
                printf("      Hardware Flow Control\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_FLOWCONTROL_SOFT )
                printf("      Xon/Xoff Flow Control\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_CCITT_OVERRIDE )
                printf("      CCITT Override\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_SPEED_ADJUST )
                printf("      Speed Adjust (modulation)\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_TONE_DIAL )
                printf("      Tone Dial\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_BLIND_DIAL )
                printf("      Blind Dial\n");
            if ( lpModemSettings->dwNegotiatedModemOptions & MDM_V23_OVERRIDE )
                printf("      V.23 Override\n");
        }
        printf("    NegotiatedDCERate     = %d bits/s\n",lpModemSettings->dwNegotiatedDCERate);
    }
    if ( lpCommConfig ) {
        printf("  Communication Settings:\n");
        printf("    ProviderSubType       = ");
        switch ( lpCommConfig->dwProviderSubType ) {
        case PST_UNSPECIFIED     :
            printf("Unspecified");
            break;
        case PST_RS232           :
            printf("RS232");
            break;
        case PST_PARALLELPORT    :
            printf("Parallel Port");
            break;
        case PST_RS422           :
            printf("RS422");
            break;
        case PST_RS423           :
            printf("RS423");
            break;
        case PST_RS449           :
            printf("RS449");
            break;
        case PST_MODEM           :
            printf("Modem");
            break;
        case PST_FAX             :
            printf("Fax");
            break;
        case PST_SCANNER         :
            printf("Scanner");
        case PST_NETWORK_BRIDGE  :
            printf("Network Bridge");
            break;
        case PST_LAT             :
            printf("LAT");
            break;
        case PST_TCPIP_TELNET    :
            printf("TCP/IP Telnet");
            break;
        case PST_X25             :
            printf("X.25");
            break;
        default:
            printf("Unknown");
        }
        printf("\n");
    }
    if ( lpDCB ) {
        printf("    BaudRate                     = %d\n",lpDCB->BaudRate);
        printf("    Binary Mode (no EOF check)   = %d\n",lpDCB->fBinary);
        printf("    Enable Parity Checking       = %d\n",lpDCB->fParity);
        printf("    CTS Handshaking on output    = %d\n",lpDCB->fOutxCtsFlow);
        printf("    DSR Handshaking on output    = %d\n",lpDCB->fOutxDsrFlow);
        printf("    DTR Flow Control             = %d\n",lpDCB->fDtrControl);
        printf("    DSR Sensitivity              = %d\n",lpDCB->fDsrSensitivity);
        printf("    Continue TX when Xoff sent   = %d\n",lpDCB->fTXContinueOnXoff);
        printf("    Enable output X-ON/X-OFF     = %d\n",lpDCB->fOutX);
        printf("    Enable input X-ON/X-OFF      = %d\n",lpDCB->fInX);
        printf("    Enable Err Replacement       = %d\n",lpDCB->fErrorChar);
        printf("    Enable Null stripping        = %d\n",lpDCB->fNull);
        printf("    RTS flow control             = %d\n",lpDCB->fRtsControl);
        printf("    Abort all I/O on Error       = %d\n",lpDCB->fAbortOnError);
        printf("    Transmit X-ON threshold      = %d\n",lpDCB->XonLim);
        printf("    Transmit X-OFF threshold     = %d\n",lpDCB->XoffLim);
        printf("    Num of bits/byte, 4-8        = %d\n",lpDCB->ByteSize);
        printf("    Parity (0-4=N,O,E,M,S)       = %d\n",lpDCB->Parity);
        printf("    StopBits (0,1,2 = 1, 1.5, 2) = %d\n",lpDCB->StopBits);
        printf("    Tx and Rx X-ON character     = %d\n",lpDCB->XonChar);
        printf("    Tx and Rx X-OFF character    = %d\n",lpDCB->XoffChar);
        printf("    Error replacement char       = %d\n",lpDCB->ErrorChar);
        printf("    End of Input character       = %d\n",lpDCB->EofChar);
        printf("    Received Event character     = %d\n",lpDCB->EvtChar);
        printf("\n");
    }
    return TRUE;
}

int
cktapiGetModemSettings( LPDEVCFG * lppDevCfg,
                        LPMODEMSETTINGS * lppModemSettings,
                        LPCOMMCONFIG * lppCommConfig,
                        DCB **     lppDCB )
{
    static VARSTRING *vs=NULL;
    DWORD dwSize=0;
    ULONG lrc=0 ;
    LPDEVCFG        lpDevCfg = NULL;
    LPCOMMCONFIG    lpCommConfig = NULL;
    LPMODEMSETTINGS lpModemSettings = NULL;
    DCB *           lpDCB = NULL;

    if ( lppDevCfg )
        *lppDevCfg = NULL;
    if ( lppCommConfig )
        *lppCommConfig = NULL;
    if ( lppDCB )
        *lppDCB = NULL;
    if ( lppModemSettings )
        *lppModemSettings = NULL;

    if ( vs != NULL ) {                 /* free previous VARSTRING */
        free(vs);
        vs = NULL;
    }

    if ( g_hLine == (HLINE)0 ) {
        debug(F100,"cktapiGetModemSettings g_hLine == NULL","",0);
        return FALSE;
    }

    /* The next four lines prepare a VARSTRING structure to
    pass to Windows though lineGetID */
    vs = (VARSTRING *) malloc (1024);
    if ( !vs ) {
        debug(F100,"cktapiGetModemSettings malloc failed","",0);
        return FALSE;
    }
    memset(vs,0,1024);
    vs->dwTotalSize = 1024;

    do {
        /* get Modem Device Configuration */
        lrc = (*cklineGetDevConfig)( LineDeviceId, vs, "comm/datamodem" );
        if ( ( lrc == LINEERR_STRUCTURETOOSMALL || lrc == 0 ) &&
             (vs->dwTotalSize < vs->dwNeededSize) ) {
            /* the next six lines reallocate the VARSTRING */
            dwSize = vs->dwNeededSize ;
            free(vs);
            vs = (VARSTRING *) malloc(dwSize);
            if ( !vs ) {
                debug(F100,"cktapiGetModemSettings malloc 2 failed","",0);
                return FALSE;
            }
            memset(vs,0,sizeof(vs));
            vs->dwTotalSize = dwSize ;
        }
        else if ( lrc ) {
            /* some kind of TAPI error */
            OutputDebugLastError(lrc,"lineGetDevConfig:");
            free(vs);
            debug(F101,"cktapiGetModemSettings tapi error","",lrc);
            return FALSE;
        }
        else
            break;  /* success */
    } while ( TRUE );

    lpDevCfg     = (LPDEVCFG) ((LPSTR)vs + vs->dwStringOffset);
    if ( lpDevCfg == NULL ) {
        debug(F100,"cktapiGetModemSettings lpDevCfg == NULL","",0);
        free(vs);
        return FALSE;
    }

    lpCommConfig = (LPCOMMCONFIG) &lpDevCfg->commconfig;

    lpDCB = &lpCommConfig->dcb;

    lpModemSettings = (LPMODEMSETTINGS)((LPSTR)lpCommConfig +
                                         lpCommConfig->dwProviderOffset);

    if ( lppDevCfg )
        *lppDevCfg = lpDevCfg;
    if ( lppCommConfig )
        *lppCommConfig = lpCommConfig;
    if ( lppDCB )
        *lppDCB = lpDCB;
    if ( lppModemSettings )
        *lppModemSettings = lpModemSettings;
    return TRUE;
}

int
cktapiSetModemSettings( LPDEVCFG lpDevCfg, LPCOMMCONFIG lpCommConfig )
{
    LONG lrc=0 ;
    HANDLE hModem = NULL;

    if ( lpDevCfg == NULL )
        return FALSE;

    /* First try with a Modem Handle */
    hModem = GetModemHandleFromLine((HLINE)0);
    if ( hModem )
    {
        lrc = SetCommConfig( hModem, lpCommConfig, lpCommConfig->dwSize );
        CloseHandle(hModem);
        hModem = NULL;
        if ( lrc )
            return(TRUE);

        lrc = GetLastError();
        debug(F111,"cktapiSetModemSettings","SetCommConfig",lrc);
    }

    lrc = (*cklineSetDevConfig)( LineDeviceId,
                                 lpDevCfg,
                                 lpDevCfg->dfgHdr.dwSize,
                                 "comm/datamodem" );
    OutputDebugLastError(lrc,"lineSetDevConfig:");
    if ( lrc < 0 )
    {
        debug(F111,"cktapiSetModemSettings","lineSetDevConfig",lrc);
#ifdef BETADEBUG
        printf("cktapiSetModemSettings rc=%x\n",lrc);
#endif /* BETADEBUG */
        return(FALSE);
    }
    return(TRUE);
}


int
cktapidisconnect( void )
{
    int i=5;
    if ( !g_bHangingUp && !g_bClosing ) {
        _beginthread( cktapihangup, 65535, 0 );
    }
    do {
        msleep(50);
    } while ( (g_bHangingUp || g_bClosing) && i-- );
    return !(g_bHangingUp || g_bClosing) ;
}

int
cktapicloseasync( void )
{
    int i=5;
    if ( !g_bClosing ) {
        _beginthread( cktapiclose, 65535, 0 );
    }
    do {
        msleep(50);
    } while ( g_bClosing && i-- );
    return !g_bClosing;
}

#ifndef NODIAL
_PROTOTYP (int getok, (int,int) );
struct mdminf *
cktapiGetModemInf( DWORD LineID, HANDLE hModem )
{
    LPCSTR lpModemKey = (char *)g_lpLineDevCaps[LineID]
        + g_lpLineDevCaps[LineID]->dwDevSpecificOffset + 8;
    HKEY  hkModemKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256]="", lpszBlindOff[256]="";
    CHAR  lpszPrefix[16]="";
    CHAR  lpszPostfix[16]="";
    CHAR  lpszBuf[256]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;
    COMMPROP *     lpCommProp = NULL;
    LPMODEMDEVCAPS lpModemDevCaps = NULL;
    int i=0,j=0,rc=0;
    static struct mdminf mdmInf = {
        NULL, NULL, NULL, 0, NULL, 0, NULL, 0,
        NULL, NULL, NULL, NULL, 0, 0, 0,
        NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        0, 0, getok
    };

    /* initialize the MDMINF structure */
    if ( mdmInf.name ) {
        free(mdmInf.name);
        mdmInf.name = NULL;
    }
    if ( mdmInf.pulse ) {
        free(mdmInf.pulse);
        mdmInf.pulse = NULL;
    }
    if ( mdmInf.tone ) {
        free(mdmInf.tone);
        mdmInf.tone = NULL;
    }
    mdmInf.dial_time = 0;
    if ( mdmInf.pause_chars ) {
        free(mdmInf.pause_chars);
        mdmInf.pause_chars = NULL;
    }
    mdmInf.pause_time = 0;
    if ( mdmInf.wake_str ) {
        free(mdmInf.wake_str);
        mdmInf.wake_str = NULL;
    }
    mdmInf.wake_rate=0;
    if ( mdmInf.dmode_str ) {
        free(mdmInf.dmode_str);
        mdmInf.dmode_str = NULL;
    }
    if ( mdmInf.dmode_prompt ) {
        free(mdmInf.dmode_prompt);
        mdmInf.dmode_prompt = NULL;
    }
    if ( mdmInf.dial_str ) {
        free(mdmInf.dial_str);
        mdmInf.dial_str = NULL;
    }
    mdmInf.dial_rate = 0;
    mdmInf.esc_time = 0;
    mdmInf.esc_char = 0;
    if ( mdmInf.hup_str ) {
        free(mdmInf.hup_str);
        mdmInf.hup_str = NULL;
    }
    if ( mdmInf.hwfc_str ) {
        free(mdmInf.hwfc_str);
        mdmInf.hwfc_str = NULL;
    }
    if ( mdmInf.swfc_str ) {
        free(mdmInf.swfc_str);
        mdmInf.swfc_str = NULL;
    }
    if ( mdmInf.nofc_str ) {
        free(mdmInf.nofc_str);
        mdmInf.nofc_str = NULL;
    }
    if ( mdmInf.ec_on_str ) {
        free(mdmInf.ec_on_str);
        mdmInf.ec_on_str = NULL;
    }
    if ( mdmInf.ec_off_str ) {
        free(mdmInf.ec_off_str);
        mdmInf.ec_off_str = NULL;
    }
    if ( mdmInf.dc_on_str ) {
        free(mdmInf.dc_on_str);
        mdmInf.dc_on_str = NULL;
    }
    if ( mdmInf.dc_off_str ) {
        free(mdmInf.dc_off_str);
        mdmInf.dc_off_str = NULL;
    }
    if ( mdmInf.aa_on_str ) {
        free(mdmInf.aa_on_str);
        mdmInf.aa_on_str = NULL;
    }
    if ( mdmInf.aa_off_str ) {
        free(mdmInf.aa_off_str);
        mdmInf.aa_off_str = NULL;
    }
    if ( mdmInf.sb_on_str ) {
        free(mdmInf.sb_on_str);
        mdmInf.sb_on_str = NULL;
    }
    if ( mdmInf.sb_off_str ) {
        free(mdmInf.sb_off_str);
        mdmInf.sb_off_str = NULL;
    }
    if ( mdmInf.sp_on_str ) {
        free(mdmInf.sp_on_str);
        mdmInf.sp_on_str = NULL;
    }
    if ( mdmInf.sp_off_str ) {
        free(mdmInf.sp_off_str);
        mdmInf.sp_off_str = NULL;
    }
    if ( mdmInf.vol1_str ) {
        free(mdmInf.vol1_str);
        mdmInf.vol1_str = NULL;
    }
    if ( mdmInf.vol2_str ) {
        free(mdmInf.vol2_str);
        mdmInf.vol2_str = NULL;
    }
    if ( mdmInf.vol3_str ) {
        free(mdmInf.vol3_str);
        mdmInf.vol3_str = NULL;
    }
    if ( mdmInf.ignoredt ) {
        free(mdmInf.ignoredt);
        mdmInf.ignoredt = NULL;
    }
    if ( mdmInf.ini2 ) {
        free(mdmInf.ini2);
        mdmInf.ini2 = NULL;
    }
    mdmInf.max_speed=0;
    mdmInf.capas=0;

    if ( LineDeviceId == (DWORD) -1 ||
         g_lpLineDevCaps[LineDeviceId]->dwDevSpecificSize == 0 )
        return NULL;

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpModemKey, 0,
                        KEY_READ, &hkModemKey) )
        return NULL;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "FriendlyName";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        mdmInf.name = strdup(lpszKeyValue);
    } else {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Model";
        if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            mdmInf.name = strdup(lpszKeyValue);
        }
    }

#ifdef COMMENT
    /* There is no place appropriate to place this stuff */
    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Reset";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        for ( i=0,j=0;j<dwSize;i++,j++ )
        {
            if ( lpszKeyValue[j] == '<' ) {
                if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                    lpszBuf[i] = '\015' ;
                    j += 3;
                }
                else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                    lpszBuf[i] = '0' ;
                    j += 2;
                }
                else lpszBuf[i] = lpszKeyValue[j];
            }
            else {
                lpszBuf[i] = lpszKeyValue[j];
            }
        }
        mdmInf.?????_str = strdup(lpszBuf);
    }
#endif /* COMMENT */

    if ( !RegOpenKeyEx(hkModemKey, "Settings", 0,
                        KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Prefix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            for ( i=0,j=0;j<dwSize;i++,j++ )
            {
                if ( lpszKeyValue[j] == '<' ) {
                    if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                        lpszPrefix[i] = '\015' ;
                        j += 3;
                    }
                    else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                        lpszPrefix[i] = '0' ;
                        j += 2;
                    }
                    else lpszPrefix[i] = lpszKeyValue[j];
                }
                else {
                    lpszPrefix[i] = lpszKeyValue[j];
                }
            }
        }
#ifdef COMMENT
        else strcpy(lpszPrefix,"AT");
#endif
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Terminator";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            for ( i=0,j=0;j<dwSize;i++,j++ )
            {
                if ( lpszKeyValue[j] == '<' ) {
                    if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                        lpszPostfix[i] = '\015' ;
                        j += 3;
                    }
                    else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                        lpszPostfix[i] = '0' ;
                        j += 2;
                    }
                    else lpszPostfix[i] = lpszKeyValue[j];
                }
                else {
                    lpszPostfix[i] = lpszKeyValue[j];
                }
            }
        }
#ifdef COMMENT
        else strcpy(lpszPostfix,"\015");
#endif /* COMMENT */

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "DialPrefix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s%s",lpszPrefix,lpszKeyValue,"%s",lpszPostfix);
            mdmInf.dial_str = strdup(lpszBuf);
        }
        else mdmInf.dial_str = strdup("ATD%s\015");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Hard";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.hwfc_str = strdup(lpszBuf);
        }
        else mdmInf.hwfc_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Soft";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.swfc_str = strdup(lpszBuf);
        }
        else mdmInf.swfc_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.nofc_str = strdup(lpszBuf);
        }
        else mdmInf.nofc_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ErrorControl_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.ec_on_str = strdup(lpszBuf);
        }
        else mdmInf.ec_on_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ErrorControl_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.ec_off_str = strdup(lpszBuf);
        }
        else mdmInf.ec_off_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Compression_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.dc_on_str = strdup(lpszBuf);
        }
        else mdmInf.dc_on_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Compression_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.dc_off_str = strdup(lpszBuf);
        }
        else mdmInf.dc_off_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_Dial";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.sp_on_str = strdup(lpszBuf);
        }
        else mdmInf.sp_on_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.sp_off_str = strdup(lpszBuf);
        }
        else mdmInf.sp_off_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_Low";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.vol1_str = strdup(lpszBuf);
        }
        else mdmInf.vol1_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_Med";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.vol2_str = strdup(lpszBuf);
        }
        else mdmInf.vol2_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_High";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.vol3_str = strdup(lpszBuf);
        }
        else mdmInf.vol3_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Pulse";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.pulse = strdup(lpszBuf);
        }
        else mdmInf.pulse = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Tone";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.tone = strdup(lpszBuf);
        }
        else mdmInf.tone = strdup("");

        /* We save Blind_Off and then append it to the Init string */
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Blind_Off";
        RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszBlindOff, &dwSize );

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Blind_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.ignoredt = strdup(lpszBuf);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeedNegotiation_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.sb_on_str = strdup(lpszBuf);
        }
        else
            mdmInf.sb_on_str = strdup("");

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeedNegotiation_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
            mdmInf.sb_off_str = strdup(lpszBuf);
        }
        else
            mdmInf.sb_off_str = strdup("");

        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Hangup", 0,
                        KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            for ( i=0,j=0;j<dwSize;i++,j++ )
            {
                if ( lpszKeyValue[j] == '<' ) {
                    if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                        lpszBuf[i] = '\015' ;
                        j += 3;
                    }
                    else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                        lpszBuf[i] = '0' ;
                        j += 2;
                    }
                    else lpszBuf[i] = lpszKeyValue[j];
                }
                else {
                    lpszBuf[i] = lpszKeyValue[j];
                }
            }
            mdmInf.hup_str = strdup(lpszBuf);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Init", 0,
                        KEY_READ, &hkSubKey) )
    {
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            if ( ckstrcmp(lpszKeyValue,"None",-1,0) == 0 ) {
                lpszBuf[0] = '\0';
                i++;
            } else {
            for ( i=0,j=0;j<dwSize;i++,j++ )
            {
                if ( lpszKeyValue[j] == '<' ) {
                    if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                        lpszBuf[i] = '\015' ;
                        j += 3;
                    }
                    else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                        lpszBuf[i] = '0' ;
                        j += 2;
                    }
                    else lpszBuf[i] = lpszKeyValue[j];
                }
                else if ( lpszKeyValue[j] == ' ' ) {
                    i--;        /* skip the space */
                }
                else {
                    lpszBuf[i] = lpszKeyValue[j];
                }
            }
            }
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "2";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            if ( ckstrcmp(lpszKeyValue,"NoResponse",-1,0) != 0 ) {
                i--;
                j=0;

                /* Remove Postfix if necessary */
                if ( !strcmp(&lpszBuf[i-strlen(lpszPostfix)],lpszPostfix) )
                {
                    i -= strlen(lpszPostfix);
                }

                if ( !strncmp(lpszKeyValue,lpszPrefix,strlen(lpszPrefix)) )
                {
                    /* skip the prefix */
                    j += strlen(lpszPrefix);
                }
                for ( ;j<dwSize;i++,j++ )
                {
                    if ( lpszKeyValue[j] == '<' ) {
                        if ( !strncmp( "<cr>", &lpszKeyValue[j], 4 ) ) {
                            lpszBuf[i] = '\015' ;
                            j += 3;
                        }
                        else if ( !strncmp( "<#>", &lpszKeyValue[j], 3 ) ) {
                            lpszBuf[i] = '0' ;
                            j += 2;
                        }
                        else lpszBuf[i] = lpszKeyValue[j];
                    }
                    else if ( lpszKeyValue[j] == ' ' ) {
                        i--;        /* skip the space */
                    }
                    else {
                        lpszBuf[i] = lpszKeyValue[j];
                    }
                }
            }
        }
        RegCloseKey( hkSubKey );

        if ( lpszBlindOff[0] ) {
            i--;
            /* Remove Postfix if necessary */
            if ( !strcmp(&lpszBuf[i-strlen(lpszPostfix)],lpszPostfix) )
            {
                i -= strlen(lpszPostfix);
            }
            strcpy(&lpszBuf[i],lpszBlindOff);
            i += strlen(lpszBlindOff);
            strcpy(&lpszBuf[i],lpszPostfix);
            i += strlen(lpszPostfix);
            i++;
        }
        mdmInf.wake_str = strdup(lpszBuf);

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "UserInit";
        if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            if ( lpszKeyValue[0] ) {
                sprintf(lpszBuf,"%s%s%s",lpszPrefix,lpszKeyValue,lpszPostfix);
                mdmInf.ini2 = strdup(lpszBuf);
            }
        }
    }
    RegCloseKey( hkModemKey );

    /* Things we just set to default values */
    mdmInf.dial_time = 60;
    mdmInf.pause_chars = strdup(",$@W");
    mdmInf.pause_time = 2;
    mdmInf.wake_rate = 0;
    mdmInf.wake_prompt = strdup("OK\015");
    mdmInf.dmode_prompt = strdup("");
    mdmInf.dial_rate = 0;
    mdmInf.esc_time = 1100;
    mdmInf.esc_char = 43;
    mdmInf.aa_on_str = strdup("ATS0=1\015");
    mdmInf.aa_off_str = strdup("ATS0=0\015");

    /* leave enough room for provider specific information */
    lpCommProp = (COMMPROP *) malloc( 1024 );
    if ( lpCommProp != NULL ) {
        memset( lpCommProp, 0, 1024 );
        lpCommProp->wPacketLength = 1024;
        lpCommProp->dwProvSpec1 = COMMPROP_INITIALIZED;

        rc = GetCommProperties( hModem, lpCommProp );
    }
    else
        rc = 0;
    if ( rc &&
         lpCommProp->dwProvSubType == PST_MODEM &&
         lpCommProp->wcProvChar[0]) {
        lpModemDevCaps = (LPMODEMDEVCAPS) lpCommProp->wcProvChar;
        mdmInf.max_speed = lpModemDevCaps->dwMaxDTERate;
        mdmInf.capas =
            (!_stricmp("AT",lpszPrefix) ? CKD_AT : 0) |
            (lpModemDevCaps->dwModemOptions & MDM_FLOWCONTROL_HARD ? CKD_HW : 0) |
            (lpModemDevCaps->dwModemOptions & MDM_FLOWCONTROL_SOFT ? CKD_SW : 0) |
            (lpModemDevCaps->dwModemOptions & MDM_SPEED_ADJUST ? 0 : CKD_SB) |
            (lpModemDevCaps->dwModemOptions & MDM_COMPRESSION ? CKD_DC : 0) |
            (lpModemDevCaps->dwModemOptions & MDM_ERROR_CONTROL ? CKD_EC : 0);
    }
    else {
        mdmInf.max_speed=115200;

        /* Define the modem capabilities */
        mdmInf.capas =
            (!_stricmp("AT",lpszPrefix) ? CKD_AT : 0) |
            ((mdmInf.sb_on_str[0] ||
               mdmInf.ec_on_str[0] && mdmInf.dc_on_str[0]) ? CKD_SB : 0) |
            (mdmInf.ec_on_str[0] ? CKD_EC : 0) |
            (mdmInf.dc_on_str[0] ? CKD_DC : 0) |
            (mdmInf.hwfc_str[0]  ? CKD_HW : 0) |
            (mdmInf.swfc_str[0]  ? CKD_SW : 0) ;
    }

    if ( lpCommProp )
        free(lpCommProp);
    return &mdmInf;
}
#endif /* NODIAL */

void
cktapiDisplayRegistryModemInfo(LPCSTR lpModemKey )
{
    HKEY  hkModemKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;

    printf("  Modem Registry Info:\n");
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpModemKey, 0,
                        KEY_READ, &hkModemKey) )
        return;         /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "AttachedTo";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "FriendlyName";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Manufacturer";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Model";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Reset";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "PortDriver";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "InfPath";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "InfSection";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "ProviderName";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "DriverDesc";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "ResponsesKeyName";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    if ( !RegOpenKeyEx(hkModemKey, "EnableCallerID", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    EnableCallerID:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Answer", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    Answer:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Hangup", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    Hangup:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Init", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    Init:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "2";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Monitor", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    Monitor:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "2";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    if ( !RegOpenKeyEx(hkModemKey, "Settings", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("    Settings:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Prefix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Terminator";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "DialPrefix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "DialSuffix";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_Low";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_Med";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerVolume_High";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_Dial";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeakerMode_Setup";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Hard";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "FlowControl_Soft";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ErrorControl_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ErrorControl_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "ErrorControl_Forced";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Compression_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Compression_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Modulation_CCITT";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Modulation_Bell";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Pulse";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Tone";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Blind_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "Blind_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "CallSetupFailTimer";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "InactivityTimeout";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeedNegotiation_Off";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "SpeedNegotiation_On";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }

        RegCloseKey( hkSubKey );
    }

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "UserInit";
    if ( !RegQueryValueEx( hkModemKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        printf("    %s: %s\n", lpszValueName, lpszKeyValue);
    }

    if ( !RegOpenKeyEx(hkModemKey, "Settings\\Init", 0,
                        KEY_READ, &hkSubKey) )
    {
        printf("\n    Settings\\Init:\n");
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "1";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "2";
        if ( !RegQueryValueEx( hkSubKey, lpszValueName, NULL, &dwType,
                               lpszKeyValue, &dwSize ))
        {
            printf("        %s: %s\n", lpszValueName, lpszKeyValue);
        }
        RegCloseKey( hkSubKey );
    }

    RegCloseKey( hkModemKey );
    printf("\n");
    return;
}
#endif /* CK_TAPI */
