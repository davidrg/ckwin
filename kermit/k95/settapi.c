/* C K N T A P  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1997, Trustees of Columbia University in the City of New
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
#ifndef ZIL_VERSION
#include "ckntap.h"             /* Kermit Telephony */
#else /* ZINC */
#include "ktapi.h"
#endif
#include "cknwin.h"

#undef debug
#define debug(a,b,c,d)

#ifdef CK_TAPI
_PROTOTYP( char * cktapiErrorString, (DWORD));
_PROTOTYP( void ReloadTapiLocations, (void));

struct keytab * tapilinetab=NULL, * _tapilinetab=NULL, * tapilocationtab=NULL ;
int ntapiline=0, ntapilocation=0 ;

HINSTANCE hLib = NULL ;
HINSTANCE hInstance=NULL;
HWND hwndConsole=(HWND)0 ;
extern char * termessage ;
/*  Failure codes from ckudia.c */
extern int fail_code;

#define F_TIME		1		/* timeout */
#define F_INT		2		/* interrupt */
#define F_MODEM		3		/* modem-detected failure */
#define F_MINIT		4		/* cannot initialize modem */

int tapiopen = 0 ;
int tapiconv = CK_AUTO ; 		/* TAPI Conversion mode */
int tapilights = TRUE;
int tapipreterm = FALSE;
int tapipostterm = FALSE;
int tapimanual = FALSE;
int tapiinactivity = 0;
int tapibong = 8;
int tapiusecfg = CK_OFF;

/* TAPI Location Information */
char tapiloc[256] = "" ;
int tapilocid = -1 ;

DWORD  tapiCountryID = -1L ;
char   tapiAreaCode[32] = "" ;
DWORD  tapiPreferredCardID=MAXDWORD;
DWORD  tapiCountryCode=0;     
char   tapiLocalAccessCode[32] = "" ;
char   tapiLongDistAccessCode[32] = "";
char   tapiTollPrefixList[4096]="";
DWORD  tapiOptions=0;
char   tapiCancelCallWaiting[32] = "";
char   tapiCountryName[32] = "";
char   tapiSameAreaRule[32] = "";
char   tapiLongDistanceRule[32] = "";
char   tapiInternationalRule[32] = "";

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
    return TRUE;
}

// Global TAPI variables.
HWND     g_hWndMainWindow = NULL;   // Apps main window.
HWND     g_hDlgParentWindow = NULL; // This will be the parent of all dialogs.
HLINEAPP g_hLineApp = NULL;
LONG     g_lNumDevs = -1;
DWORD    g_DataDevices = 0 ;

#define MAXDEVS 64
LPLINEDEVCAPS g_lpLineDevCaps[64] ;

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
int ttyfd=-1 ; /* this holds the HLINE hLine */
int mdmtyp=0;
static int mdmtyp_sav=0;
CHAR szModemName[256] ;
DWORD LineDeviceId = -1;
DWORD LineDeviceAPIVersion = 0 ;
DWORD LineAddressCount = 0 ;
DWORD LineMediaModes = 0 ;

DWORD g_dwAPIVersion = 0;

DWORD g_dwCallState = 0;
char  g_szDisplayableAddress[1024] = "";
char  g_szDialableAddress[1024] = "";
BOOL  g_bConnected = FALSE;
LPVOID g_lpDeviceConfig = NULL;
DWORD g_dwSizeDeviceConfig;

// Global variables to allow us to do various waits.
BOOL  g_bReplyReceived;
DWORD g_dwRequestedID;
long  g_lAsyncReply;
BOOL  g_bCallStateReceived;
BOOL  g_bAnswering = FALSE;
BOOL  g_bDialing = FALSE;
BOOL  g_bHangingUp = FALSE;
BOOL  g_bClosing = FALSE;

int 
cktapiinit()
{
    int i = 0 ;
    // This will be the parent of all dialogs.
    g_hWndMainWindow = g_hDlgParentWindow = hwndConsole; 

    for ( i=0 ; i < MAXDEVS ; i++ )
	g_lpLineDevCaps[i] = NULL ;
    g_lNumDevs = 0;

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
#ifdef BETATEST_X
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
#ifdef COMMENT	/* TAPI 2.0 */
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
    // MessageBox();  ????
}

void 
OutputDebugLineError( long param, char * str )
{
    char buf[1024];
    sprintf(buf,"%s %s",str,cktapiErrorString(param));
    // MessageBox();  ????
}


void 
CALLBACK
cklineCallbackFunc( DWORD dwDevice, DWORD dwMsg, 
		    DWORD dwCallbackInstance,
		    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    return;

}

BOOL 
HandleLineErr(long lLineErr)
{
    if ( lLineErr )
	return FALSE;
    else return TRUE ;
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
        lReturn = (*cklineInitialize)(&g_hLineApp, GetModuleHandle(NULL), 
             cklineCallbackFunc, "K95 Setup", &g_lNumDevs);
	OutputDebugLastError(lReturn,"lineInitialize:");

        // If we get this error, its because some other app has yet
        // to respond to the REINIT message.  Wait 5 seconds and try
        // again.  If it still doesn't respond, tell the user.
        if (lReturn == LINEERR_REINIT)
        {
            if (bTryReInit)
            {
		Sleep(5000);	/* wait 5 secs, try again */
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
                return;
            }
        }

        if (lReturn == LINEERR_NODEVICE)
        {
            if (HandleNoDevicesInstalled())
                continue;
            else
            {
                g_bInitializing = FALSE;
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
            return;
        }
    }
    while(lReturn != SUCCESS);

    g_bLineInitialized = TRUE;
}

int 
cktapiopen(void)
{
    if ( g_bClosing ) {
	debug(F110,"TAPI Open","Close in progress",0);
	while ( g_bClosing )
	    Sleep(500);
    }
    if ( tapiopen )
    {
	tapiopen++ ;
	return TRUE ;
    }

    debug(F100,"TAPI Open","",0);
    g_bLineInitialized = FALSE;
    wintapiinit();

    if ( !g_bLineInitialized )
	return FALSE;

    tapiopen++;
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

    debug(F111,"TAPI Enumerate Devices","g_lNumDevs",g_lNumDevs);

    /* Free existing LineDevCaps */
    for ( i = 0 ; i < g_lNumDevs ; i++ )
    {
	if ( g_lpLineDevCaps[i] )
	{
	    free( g_lpLineDevCaps[i] ) ;
	    g_lpLineDevCaps[i] = NULL ;
	}
    }

    /* Enumerate current LineDevCaps */
    for ( i=0 ; i < g_lNumDevs ; i++ )
    {
	g_lpLineDevCaps[i] = (LPLINEDEVCAPS) malloc (sizeof(LINEDEVCAPS)) ;
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
cktapiclose(void)
{
    DWORD rc ;

    if ( g_bClosing ) {
	debug(F110,"TAPI Close","already in progress",0);
	while ( g_bClosing )
	    Sleep(500);
	return TRUE;
    }

    if ( tapiopen > 0 )
    {
	tapiopen-- ;

	if ( tapiopen == 1 ) {
	    g_bClosing = TRUE;
	    debug(F100,"TAPI Close","",0);
	    if ( g_hLine )
	    {	
		rc = (*cklineClose)( g_hLine );
		OutputDebugLastError(rc,"lineClose:");
		g_hLine = NULL ;
		ttyfd = -1;
	    }
	}
	if ( tapiopen == 0 ) {
	    rc = (*cklineShutdown)( g_hLineApp ) ;
	    OutputDebugLastError(rc,"lineShutdown:");
	    g_hLineApp = NULL ;
	}
	g_bClosing = FALSE;
    }
    return TRUE;
}

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

    cktapiopen() ;
    n = cktapidevenum() ;
    cktapiclose() ;

    debug(F100,"TAPI cktapiBuildLineTable","",0);
    if ( n )
    {
	*pTable = malloc( sizeof(struct keytab) * n ) ;
	*pTable2 = malloc( sizeof(struct keytab) * n ) ;

        for ( i=0 ; i < g_lNumDevs ; i++ ) {
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
   return *pN;
}

int 
cktapiGetCountryEntries( LPLINECOUNTRYLIST *lpCL, int * pN )
{
    LPLINECOUNTRYLIST  lpCountryList = NULL;
    LPLINECOUNTRYENTRY lpCountryEntry = NULL;
    DWORD dwSizeofCountryList = sizeof(LINECOUNTRYLIST);
    long lReturn=0;
    int i=0;
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;

    do
    {
	lpCountryList = 
	    (LPLINECOUNTRYLIST) CheckAndReAllocBuffer((LPVOID) lpCountryList, 
						       dwSizeofCountryList,
						       "cktapiGetCountryEntries");

	if (lpCountryList == NULL)
	    return 0;

	/* Win95 doesn't return anything is CountryID is 1 */
	lReturn = (*cklineGetCountry)(
#ifdef COMMENT
			osverinfo.dwPlatformId != 
			VER_PLATFORM_WIN32_WINDOWS ? tapiCountryID :
#endif
				       0, TAPI_CURRENT_VERSION,
				       lpCountryList);

	if (HandleLineErr(lReturn))
	    ;
	else
	{
	    OutputDebugLineError(lReturn, 
				  "lineGetCountry unhandled error: ");
	    LocalFree(lpCountryList);
	    return 0;
	}

	if ((lpCountryList -> dwNeededSize) >
	     (lpCountryList -> dwTotalSize))
	{
	    dwSizeofCountryList = lpCountryList ->dwNeededSize;
	    lReturn = -1; // Lets loop again.
	}
    }
    while(lReturn != SUCCESS);

    *lpCL = lpCountryList;
    *pN = lpCountryList->dwNumCountries;
    return 1;
}

int 
cktapiGetLocationEntries( LPLINETRANSLATECAPS *lpTC,
			  LPLINELOCATIONENTRY *lpLocationEntry, int * pN )
{
    LPLINETRANSLATECAPS lpTranslateCaps=NULL;
   DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
   long lReturn = 0;
   DWORD dwCounter;
   LPLINECARDENTRY lpLineCardEntry = NULL;
   int i = 0 ;

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
	  lpTranslateCaps = NULL;
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
   *lpLocationEntry = (LPLINELOCATIONENTRY)
      (((LPBYTE) lpTranslateCaps) + lpTranslateCaps->dwLocationListOffset);
    *pN = lpTranslateCaps->dwNumLocations;
    *lpTC = lpTranslateCaps;
   return 1;
}

int
cktapiGetCurrentLocationID( void )
{
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
    long lReturn;
    int  locationID=-1;
	
    // First, get the TRANSLATECAPS
    do
    {
	lpTranslateCaps = 
	    (LPLINETRANSLATECAPS) CheckAndReAllocBuffer((LPVOID) lpTranslateCaps, 
							 dwSizeofTranslateCaps,
							 "cktapiGetCurrentLocationID");

	if (lpTranslateCaps == NULL)
	    return -1;

	lReturn = (*cklineGetTranslateCaps)(g_hLineApp, TAPI_CURRENT_VERSION, 
					     lpTranslateCaps);

	if (HandleLineErr(lReturn))
	    ;
	else
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
    return locationID;
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
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;

    tapilocid = lpLocationEntry->dwPermanentLocationID;
    strncpy(tapiloc, 
	     (((LPSTR) lpTranslateCaps) +
	       lpLocationEntry->dwLocationNameOffset),
	     lpLocationEntry->dwLocationNameSize);

    tapiCountryID = lpLocationEntry->dwCountryID;
    tapiCountryCode = lpLocationEntry->dwCountryCode;

    do
    {
	lpCountryList = 
	    (LPLINECOUNTRYLIST) CheckAndReAllocBuffer((LPVOID) lpCountryList, 
							 dwSizeofCountryList,
							 "SaveTapiLocationInfo");

	if (lpCountryList == NULL)
	    return;

	/* Win95 doesn't return anything is CountryID is 1 */
	lReturn = (*cklineGetCountry)(osverinfo.dwPlatformId == 
				       VER_PLATFORM_WIN32_WINDOWS ? 0 
				       : tapiCountryID, TAPI_CURRENT_VERSION,
				       lpCountryList);

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
	strncpy(tapiCountryName,((LPSTR) lpCountryList +
				  lpCountryEntry->dwCountryNameOffset),
		 lpCountryEntry->dwCountryNameSize);
	strncpy(tapiSameAreaRule,((LPSTR) lpCountryList +
				  lpCountryEntry->dwSameAreaRuleOffset),
		 lpCountryEntry->dwSameAreaRuleSize);
	strncpy(tapiLongDistanceRule,((LPSTR) lpCountryList +
				  lpCountryEntry->dwLongDistanceRuleOffset),
		 lpCountryEntry->dwLongDistanceRuleSize);
	strncpy(tapiInternationalRule,((LPSTR) lpCountryList +
				  lpCountryEntry->dwInternationalRuleOffset),
		 lpCountryEntry->dwInternationalRuleSize);
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
	     lpLocationEntry->dwCityCodeSize);

    tapiPreferredCardID = lpLocationEntry->dwPreferredCardID;

    tapiOptions = lpLocationEntry->dwOptions;

    strncpy(tapiLocalAccessCode, 
	     (((LPSTR) lpTranslateCaps) +
	       lpLocationEntry->dwLocalAccessCodeOffset),
	     lpLocationEntry->dwLocalAccessCodeSize);

    strncpy(tapiLongDistAccessCode, 
	     (((LPSTR) lpTranslateCaps) +
	       lpLocationEntry->dwLongDistanceAccessCodeOffset),
	     lpLocationEntry->dwLongDistanceAccessCodeSize);

    strncpy(tapiTollPrefixList, 
	     (((LPSTR) lpTranslateCaps) +
	       lpLocationEntry->dwTollPrefixListOffset),
	     lpLocationEntry->dwTollPrefixListSize);

    strncpy(tapiCancelCallWaiting, 
	     (((LPSTR) lpTranslateCaps) +
	       lpLocationEntry->dwCancelCallWaitingOffset),
	     lpLocationEntry->dwCancelCallWaitingSize);

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
                 "cktapiFetchLocationInfo");

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

    // Was a match for lpszCurrentLocation found?
    if (lpLocationEntry[dwCounter].dwPreferredCardID == MAXDWORD)
    {
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
                 "cktapiFetchLocationInfo");

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

    // Was a match for lpszCurrentLocation found?
    if (lpLocationEntry[dwCounter].dwPreferredCardID == MAXDWORD)
    {
	LocalFree(lpTranslateCaps);
	return FALSE;
    }

    SaveTapiLocationInfo( lpTranslateCaps, &lpLocationEntry[dwCounter] );
    LocalFree(lpTranslateCaps);
    return TRUE;
}


cktapiSetCurrentLocationID( DWORD ID )
{
    long lReturn;
    lReturn = 
	(*cklineSetCurrentLocation)(g_hLineApp, ID);
    return (lReturn == SUCCESS);
}

void
CopyTapiLocationInfoToKermitDialCmd( void )
{
}

#ifdef COMMENT
void
CopyTapiLocationInfoToKermitDialCmd( void )
{
    extern char *dialnpr;		/* DIAL PREFIX */
    extern char *diallac;		/* DIAL LOCAL-AREA-CODE */
    extern char *diallcc;		/* DIAL LOCAL-COUNTRY-CODE */
    extern char *dialixp;		/* DIAL INTL-PREFIX */
    extern char *dialixs;		/* DIAL INTL-SUFFIX */
    extern char *diallcp;		/* DIAL LOCAL-PREFIX */
    extern char *diallcs;		/* DIAL LOCAL-SUFFIX */
    extern char *dialldp;		/* DIAL LD-PREFIX */
    extern char *diallds;		/* DIAL LD-SUFFIX */
    extern char *dialpxx;		/* DIAL PBX-EXCHANGE */
    extern char *dialpxi;		/* DIAL INTERNAL-PREFIX */
    extern char *dialpxo;		/* DIAL OUTSIDE-PREFIX */
    extern char *dialsfx;		/* DIAL SUFFIX */
    extern char *dialtfp;		/* DIAL TOLL-FREE-PREFIX */
    extern int dialmth;			/* DIAL METHOD TONE/PULSE */

    char * p = NULL;

    /* Make sure there are valid values */
    if ( tapilocid == -1 ) {
	tapilocid = cktapiGetCurrentLocationID();
	cktapiFetchLocationInfoByID( tapilocid );
	if ( tapilocid == -1 )
	    return;
    }

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
	strcat(dialldp,tapiLongDistanceRule);
	p = dialldp;
	/* find the end of the prefix and null terminate it */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) 
	    p++;
	*p = '\0';
    }
    
    /* The Long Distance Suffix is the suffix portion of the       */
    /* tapiLongDistanceRule (if any) 				   */
    if ( diallds )
	free(diallds);
    diallds = strdup( tapiLongDistanceRule );
    if ( diallds ) {
	p = diallds;
	/* blank out the prefix */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) {
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
	strcat(dialixp,tapiInternationalRule);
	p = dialixp;
	/* find the end of the prefix and null terminate it */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) 
	    p++;
	*p = '\0';
    }
    
    /* The International Suffix is the suffix portion of the       */
    /* tapiInternationalRule (if any) 				   */
    if ( dialixs )
	free(dialixs);
    dialixs = strdup( tapiInternationalRule );
    if ( dialixs ) {
	p = dialixs;
	/* blank out the prefix */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) {
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

    /* We currently ignore the tapiLocalAccessCode                 */
    /* Local Prefix */
    if ( diallcp )
	free(diallcp);
    /* The true Local Prefix is the tapiLocalAccessCode */
    /* plus the prefix portion of the tapiSameAreaRule         */
    diallcp = (char *) malloc(strlen(tapiLocalAccessCode) + 
			       strlen(tapiSameAreaRule) + 1);
    if ( diallcp ) {
	strcpy(diallcp,tapiLocalAccessCode);
	strcat(diallcp,tapiSameAreaRule);
	p = diallcp;
	/* find the end of the prefix and null terminate it */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) 
	    p++;
	*p = '\0';
    }
    
    /* The Local Access Suffix is the suffix portion of the       */
    /* tapiLocalAccessRule (if any) 				   */
    if ( diallcs )
	free(diallcs);
    diallcs = strdup( tapiSameAreaRule );
    if ( diallcs ) {
	p = diallcs;
	/* blank out the prefix */
	while ( *p && *p != 'E' && *p != 'F' && *p != 'G' ) {
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
}
#endif /* COMMENT */

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

    if ( hLine == NULL )
	hLine = g_hLine;

    if ( hLine == NULL )
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
	lrc = (*cklineGetID)(hLine, 0L, NULL,
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
    int ID;

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
    ID = cktapiGetCurrentLocationID();
    cktapiFetchLocationInfoByID( ID );
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

    if ( !cktapiopen() )
    {
	return -1;
    }

    /* Find the Line ID */
    for ( i=0 ; i<ntapiline ; i++ )
    {
	if ( !strcmp( devicename, tapilinetab[i].kwd ) )
	{
	    LineDeviceId = tapilinetab[i].kwval ;
	    break;
	}
    }

    if ( i == ntapiline )
    {
	cktapiclose();
	return -1 ;
    }

    /* Negotiate API Version for the Line Device */
    {
	rc = (*cklineNegotiateAPIVersion)( g_hLineApp, LineDeviceId,
					   TAPI_CURRENT_VERSION, TAPI_CURRENT_VERSION,
					   &LineDeviceAPIVersion, 
					   &ExtId ) ;
	if ( rc < 0 )
	{
	    cktapiclose();
	    return -1;
	}
    }

    /* Get Line Device Capabilities */
    {
	/* The line device capabilities have already been placed into */
	/* g_lpLineDevCaps[LineDeviceId]                              */
    }

    /* Get Addresses and Media modes */
    {
	LineAddressCount = g_lpLineDevCaps[LineDeviceId]->dwNumAddresses ;
	LineMediaModes = g_lpLineDevCaps[LineDeviceId]->dwMediaModes ;
    }

    if ( g_hLine == NULL ) {
	/* Call lineOpen() */
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
		    g_hLine = NULL;
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
    ttyfd = -2;			/* We don't have a real handle yet */

    /* Do this just to set the name of the Modem */
    CloseHandle( GetModemHandleFromLine( g_hLine ) );
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

    if ( g_hCall != NULL )
	return FALSE;

    if ( g_hLine == NULL )
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
		- (g_hCall!=NULL?1:0);

    LocalFree( lpLineDevStatus );
    return InProgress;
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
    
    if ( vs != NULL ) {			/* free previous VARSTRING */
	free(vs);
	vs = NULL;
    }

    if ( g_hLine == NULL )
	return FALSE;

    /* The next four lines prepare a VARSTRING structure to
    pass to Windows though lineGetID */
    vs = (VARSTRING *) malloc (1024);
    if ( !vs ) {
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
		return FALSE;
	    }
	    memset(vs,0,sizeof(vs));
	    vs->dwTotalSize = dwSize ;
	}
	else if ( lrc ) {
	    /* some kind of TAPI error */
	    OutputDebugLastError(lrc,"lineGetDevConfig:");
	    free(vs);
	    return FALSE;
	}
	else 
	    break;  /* success */
    } while ( TRUE );

    lpDevCfg     = (LPDEVCFG) ((LPSTR)vs + vs->dwStringOffset);
    if ( lpDevCfg == NULL ) {
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

#endif /* CK_TAPI */
