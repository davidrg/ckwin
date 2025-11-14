/* C K N T A P  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1997, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
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
#include <stdio.h>

#include <windows.h>            	/* Windows Definitions */
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
/* all functions in this module return TRUE to indicate success */
/* or FALSE to indicate failure */
#include "ktapi.h"
#include "cknwin.h"
#include "ckntapi.h"

#undef debug
#undef printf
#undef fprintf
#undef putc
#undef putchar

#define DBUFL 2300
/*
  WARNING: This routine is not thread-safe, especially when Kermit is
  executing on multiple CPUs -- as different threads write to the same
  static buffer, the debug statements are all interleaved.  To be fixed
  later...
*/
static char *dbptr = (char *)0;

int
#ifdef CK_ANSIC
debug(int f, char *s1, char *s2, long n)
#else
debug(f,s1,s2,n) int f; char *s1, *s2; long n;
#endif /* CK_ANSIC */
/* dodebug */ {
    char *sp;
    int len1, len2;

    /*
  This prevents infinite recursion in case we accidentally put a debug()
  call in this routine, or call another routine that contains debug() calls.
  From this point on, all returns from this return must be via goto xdebug,
  which sets deblog back to 1.
*/
    if (!dbptr) {			/* Allocate memory buffer */
	dbptr = malloc(DBUFL+1);	/* This only happens once */
	if (!dbptr) {
	    return(0);
	}
    }
    if (!s1) s1="(NULL)";
    if (!s2) s2="(NULL)";

    len1 = strlen(s1);
    len2 = strlen(s2);

/* Need to accept longer strings when debugging authenticated connections */
    if (f == F010) {
        if (len2 + 2 >= DBUFL) s2 = "(string too long)";
    } else if (f != F011 && f != F100) {
	if (len1 > 100) s1 = "(string too long)";
	if (len2 + 101 >= DBUFL) s2 = "(string too long)";
    }

    sp = dbptr;

    switch (f) {		/* Write log record according to format. */
      case F000:		/* 0 = print both strings, and n as a char. */
	if (len2 > 0) {
	    if ((n > 31 && n < 127) || (n > 159 && n < 256))
	      printf("%s[%s]=%c\n",s1,s2,(CHAR) n);
	    else if (n < 32 || n == 127)
	      printf("%s[%s]=^%c\n",s1,s2,(CHAR) ((n+64) & 0x7F));
	    else if (n > 127 && n < 160)
	      printf("%s[%s]=~^%c\n",s1,s2,(CHAR)((n-64) & 0x7F));
	    else printf("%s[%s]=%ld\n",s1,s2,n);
	} else {
	    if ((n > 31 && n < 127) || (n > 159 && n < 256))
	      printf("%s=%c\n",s1,(CHAR) n);
	    else if (n < 32 || n == 127)
	      printf("%s=^%c\n",s1,(CHAR) ((n+64) & 0x7F));
	    else if (n > 127 && n < 160)
	      printf("%s=~^%c\n",s1,(CHAR)((n-64) & 0x7F));
	    else printf("%s=%ld\n",s1,n);
	}
	break;
      case F001:			/* 1, "=n" */
	printf("=%ld\n",n);
	break;
      case F010:			/* 2, "[s2]" */
	printf("[%s]\n",s2);
	break;
/*
  This one treats n as the length of the string s2, which may contain NULs.
  It's good for logging NUL-bearing packets and other data in the debug log.
*/
      case F011:			/* 3, "[s2]=n" */
	break;
      case F100:			/* 4, "s1" */
          printf(s1);
	break;
      case F101:			/* 5, "s1=n" */
	printf("%s=%ld\n",s1,n);
	break;
      case F110:			/* 6, "s1[s2]" */
	printf("%s[%s]\n",s1,s2);
	break;
      case F111:			/* 7, "s1[s2]=n" */
	printf("%s[%s]=%ld\n",s1,s2,n);
	break;
      default:
	printf("\n?Invalid format for debug() - %d\n",f);
	break;
    }
  xdebug:				/* Common exit point */
    return(0);
}


#ifdef CK_TAPI
_PROTOTYP( char * cktapiErrorString, (DWORD));

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
char tapiloc[257] = "" ;
int tapilocid = -1 ;

DWORD  tapiCountryID = -1L ;
char   tapiAreaCode[65] = "" ;
DWORD  tapiPreferredCardID=MAXDWORD;
DWORD  tapiCountryCode=0;     
char   tapiLocalAccessCode[65] = "" ;
char   tapiLongDistAccessCode[32] = "";
char   tapiTollPrefixList[4097]="";
DWORD  tapiOptions=0;
char   tapiCancelCallWaiting[65] = "";
char   tapiCountryName[65] = "";
char   tapiSameAreaRule[65] = "";
char   tapiLongDistanceRule[65] = "";
char   tapiInternationalRule[65] = "";

static cklineInitialize_t cklineInitialize = NULL ;
static cklineNegotiateAPIVersion_t cklineNegotiateAPIVersion = NULL ;
static cklineGetDevCaps_t cklineGetDevCaps = NULL ;
static cklineShutdown_t cklineShutdown = NULL ;
static cklineOpen_t cklineOpen = NULL ;
static cklineMakeCall_t cklineMakeCall = NULL ;
static cklineDial_t cklineDial = NULL ;
static cklineDrop_t cklineDrop = NULL ;
static cklineAnswer_t cklineAnswer = NULL ;
static cklineAccept_t cklineAccept = NULL ;
static cklineDeallocateCall_t cklineDeallocateCall = NULL ;
static cklineSetCallPrivilege_t cklineSetCallPrivilege = NULL ;
static cklineClose_t cklineClose = NULL ;
static cklineHandoff_t cklineHandoff = NULL ;
static cklineGetID_t cklineGetID = NULL ;
static cklineGetTranslateCaps_t cklineGetTranslateCaps = NULL ;
static cklineSetCurrentLocation_t cklineSetCurrentLocation = NULL ;
static cklineSetStatusMessages_t cklineSetStatusMessages = NULL ;
static cklineConfigDialog_t cklineConfigDialog = NULL ;
static cklineTranslateDialog_t cklineTranslateDialog = NULL ;
static cklineTranslateAddress_t cklineTranslateAddress = NULL ;
static cklineGetCountry_t cklineGetCountry = NULL;
static cklineGetDevConfig_t cklineGetDevConfig = NULL;
static cklineGetLineDevStatus_t cklineGetLineDevStatus=NULL;
static cklineSetDevConfig_t cklineSetDevConfig=NULL;
static cklineGetCallInfo_t cklineGetCallInfo=NULL;
static cklineMonitorMedia_t cklineMonitorMedia=NULL;
static cklineGetAppPriority_t cklineGetAppPriority=NULL;
static cklineSetAppPriority_t cklineSetAppPriority=NULL;
static cklineGetNumRings_t cklineGetNumRings=NULL;
static cklineSetNumRings_t cklineSetNumRings=NULL;
static cklineSetCallParams_t cklineSetCallParams=NULL;

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

   if ((cklineInitialize = (cklineInitialize_t)GetProcAddress( hLib, "lineInitialize" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineNegotiateAPIVersion = (cklineNegotiateAPIVersion_t)GetProcAddress( hLib, "lineNegotiateAPIVersion" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineGetDevCaps = (cklineGetDevCaps_t)GetProcAddress( hLib, "lineGetDevCaps" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineShutdown = (cklineShutdown_t)GetProcAddress( hLib, "lineShutdown" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineOpen = (cklineOpen_t)GetProcAddress( hLib, "lineOpen" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineMakeCall = (cklineMakeCall_t)GetProcAddress( hLib, "lineMakeCall" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineDial = (cklineDial_t)GetProcAddress( hLib, "lineDial" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineDrop = (cklineDrop_t)GetProcAddress( hLib, "lineDrop" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
    if ((cklineAnswer = (cklineAnswer_t)GetProcAddress( hLib, "lineAnswer" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineAccept = (cklineAccept_t)GetProcAddress( hLib, "lineAccept" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineDeallocateCall = (cklineDeallocateCall_t)GetProcAddress( hLib, "lineDeallocateCall" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineClose = (cklineClose_t)GetProcAddress( hLib, "lineClose" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineGetID = (cklineGetID_t)GetProcAddress( hLib, "lineGetID" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineGetTranslateCaps = (cklineGetTranslateCaps_t)GetProcAddress( hLib, "lineGetTranslateCaps" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineSetCurrentLocation = (cklineSetCurrentLocation_t)GetProcAddress( hLib, "lineSetCurrentLocation" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
   if ((cklineSetStatusMessages = (cklineSetStatusMessages_t)GetProcAddress( hLib, "lineSetStatusMessages" )) == NULL )
   {
      rc = GetLastError() ;
      debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
      return FALSE;
   }
    if ((cklineConfigDialog = (cklineConfigDialog_t)GetProcAddress( hLib, "lineConfigDialog" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineTranslateDialog = (cklineTranslateDialog_t)GetProcAddress( hLib, "lineTranslateDialog" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineTranslateAddress = 
	  (cklineTranslateAddress_t)GetProcAddress( hLib, "lineTranslateAddress" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetCountry = 
	  (cklineGetCountry_t)GetProcAddress( hLib, "lineGetCountry" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetLineDevStatus = 
	  (cklineGetLineDevStatus_t)GetProcAddress( hLib, "lineGetLineDevStatus" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetDevConfig = 
	  (cklineGetDevConfig_t)GetProcAddress( hLib, "lineGetDevConfig" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineSetDevConfig = 
	  (cklineSetDevConfig_t)GetProcAddress( hLib, "lineSetDevConfig" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineHandoff = 
	  (cklineHandoff_t)GetProcAddress( hLib, "lineHandoff" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineSetCallPrivilege = 
	  (cklineSetCallPrivilege_t)GetProcAddress( hLib, "lineSetCallPrivilege" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetCallInfo = 
	  (cklineGetCallInfo_t)GetProcAddress( hLib, "lineGetCallInfo" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineMonitorMedia = 
	  (cklineMonitorMedia_t)GetProcAddress( hLib, "lineMonitorMedia" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetAppPriority = 
	  (cklineGetAppPriority_t)GetProcAddress( hLib, "lineGetAppPriority" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineSetAppPriority = 
	  (cklineSetAppPriority_t)GetProcAddress( hLib, "lineSetAppPriority" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineGetNumRings = 
	  (cklineGetNumRings_t)GetProcAddress( hLib, "lineGetNumRings" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineSetNumRings = 
	  (cklineSetNumRings_t)GetProcAddress( hLib, "lineSetNumRings" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111, "cktapiload LoadLibrary failed","tapi32",rc) ;
	return FALSE;
    }
    if ((cklineSetCallParams = 
	  (cklineSetCallParams_t)GetProcAddress( hLib, "lineSetCallParams" )) == NULL )
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
HLINEAPP g_hLineApp = (HLINEAPP) 0;
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
HCALL g_hCall = (HCALL) 0;
HLINE g_hLine = (HLINE) 0;
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
BOOL  g_bReplyReceived=FALSE;
DWORD g_dwRequestedID=0;
long  g_lAsyncReply=SUCCESS;
BOOL  g_bCallStateReceived=FALSE;
BOOL  g_bAnswering = FALSE;
BOOL  g_bDialing = FALSE;
BOOL  g_bHangingUp = FALSE;
BOOL  g_bClosing = FALSE;

int 
cktapiinit(HINSTANCE hInst)
{
    int i = 0 ;
    HMODULE hntdll
    static const char *(CDECL *pwine_get_version)(void);

    // This will be the parent of all dialogs.
    g_hWndMainWindow = g_hDlgParentWindow = hwndConsole; 
    hInstance = hInst;

    hntdll = GetModuleHandle("ntdll.dll");
    debug(F100,"Checking for WINE","",0);
    pwine_get_version = (void *)GetProcAddress(hntdll, "wine_get_version");
    if (hntdll != NULL) {
        if (pwine_get_version) {
            debug(F100,"TAPI disabled under WINE","",0);
            return FALSE;
        }
    }

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
OutputDebugLineCallback(
        DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance, 
        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    char buf[1024];
    sprintf(buf,"(%d,%d,%d,%d,%d,%d)",
	     dwDevice,dwMsg,dwCallbackInstance,
	     dwParam1,dwParam2,dwParam3);
    // MessageBox();  ????
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
    if ((long) dwParam2 != SUCCESS)
        OutputDebugLineError((long) dwParam2, "LINE_REPLY error: ");
#ifdef BETATEST_X
    else
        printf("LINE_REPLY: successfully replied.\n");
#endif /* BETATEST */

    // If we are currently waiting for this async Request ID
    // then set the global variables to acknowledge it.
    if (g_dwRequestedID == dwParam1)
    {
#ifdef BETATEST_X
	printf("LINE_REPLY: dwRequestedID == dwParam1\n");
#endif
        g_lAsyncReply = (long) dwParam2;
        g_bReplyReceived = TRUE;
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

void 
DoLineClose(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Line has been shut down.  Clean up our internal variables.
    ;
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
    if (g_lNumDevs <= dwParam1)
        g_lNumDevs = dwParam1+1;
    g_DataDevices = cktapidevenum() ;
    debug(F111,"TAPI","A new line device has been added to the system",
	   g_DataDevices);
#ifdef BETATEST_X
    printf("A new line device has been added to the system.\n");
#endif /* BETATEST */
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
#ifdef BETATEST_X
	    printf("Line configuration has changed\n");
#endif /* BETATEST */
	    if ( (HLINE) dwDevice == g_hLine ) {
		debug(F111,"TAPI LINEDEVSTATE_REINIT-0","Shutdown required",dwDevice);
	    }
	    break;

	case LINE_CREATE:
#ifdef BETATEST_X
	    printf("Soft REINIT: LINE_CREATE.\n");
#endif /* BETATEST */
	    DoLineCreate(dwDevice, dwParam2, dwCallbackInstance,
			  dwParam3, 0, 0);
	    break;	

	case LINE_LINEDEVSTATE:
#ifdef BETATEST_X
	    printf("Soft REINIT: LINE_LINEDEVSTATE.\n");
#endif /* BETATEST */

	    DoLineDevState(dwDevice, dwParam2, dwCallbackInstance,
			    dwParam3, 0, 0);
	    break;	

	    // There might be other reasons to send a soft reinit.
	    // No need to to shutdown for these.
	default:
#ifdef BETATEST_X
	    printf("Ignoring soft REINIT\n");
#endif /* BETATEST */
	    break;
	}
	break;

    case LINEDEVSTATE_OUTOFSERVICE:
	if ( (HLINE) dwDevice == g_hLine ) {
	    debug(F111,"TAPI LINEDEVSTATE_OUTOFSERVICE",szModemName,dwDevice);
	}
	break;

    case LINEDEVSTATE_DISCONNECTED:
	if ( (HLINE) dwDevice == g_hLine &&
	     g_hCall != (HCALL)0 ) {
	    debug(F111,"TAPI LINEDEVSTATE_DISCONNECTED",szModemName,dwDevice);
	}
	break;

    case LINEDEVSTATE_MAINTENANCE:
	if ( (HLINE) dwDevice == g_hLine ) {
	    debug(F111,"TAPI LINEDEVSTATE_MAINTENANCE",szModemName,dwDevice);
	}
	break;

    case LINEDEVSTATE_TRANSLATECHANGE:
#ifdef BETATEST_X
	printf("Translate Change\n");
#endif 
	debug(F100,"TAPI LINEDEVSTATE_TRANSLATECHANGE","",0);
	/* Reload TAPI Location Information From Scratch */
	break;

    case LINEDEVSTATE_REMOVED:
#ifdef BETATEST_X
	printf("A Line device has been removed; no action taken.\n");
#endif
	if (g_lNumDevs <= dwParam1)
	    g_lNumDevs = dwParam1+1;
	g_DataDevices = cktapidevenum() ;
	debug(F111,"TAPI LINEDEVSTATE_REMOVED",
	       "A line device has been removed; no action taken",
	       g_DataDevices);
	break;

    case LINEDEVSTATE_CLOSE:
#ifdef BETATEST_X
	printf("A Line device has been closed; no action taken.\n");
#endif /* BETATEST */
	debug(F110,"TAPI LINEDEVSTATE_CLOSE","A Line device has been closed; no action taken",
	       0);
	break;

    default:
#ifdef BETATEST_X
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
#ifdef BETATEST_X
	printf("LINE_CALLSTATE: Unknown device ID '0x%lx\n",dwDevice);
#endif
	rc = (*cklineDeallocateCall)((HCALL)dwDevice);
	OutputDebugLastError(rc,"lineDeallocateCall: ");
#ifdef BETATEST_X
	printf("  lineDeallocateCall = %x\n",rc);
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
#ifdef BETATEST_X
	printf("line has monitor privilege\n");
#endif
	//cktapiclose();
	break;

	// close line if we are made owner.  Shouldn't happen!
    case LINECALLPRIVILEGE_OWNER:
#ifdef BETATEST_X
	printf("line has owner privilege\n");
#endif
	//cktapiclose();
	break;

    default: // Shouldn't happen!  All cases handled.
#ifdef BETATEST_X
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
	}
	break;

    case LINECALLSTATE_IDLE:
	UpdateStatusBar("Line is idle",1,0);
	break;

    case LINECALLSTATE_SPECIALINFO:
	UpdateStatusBar("Special Info, probably couldn't dial number",1,0);
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
		    g_hCall = (HCALL)0;
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
            (HANDLE) ttyfd = 
                *((LPHANDLE)((LPBYTE)lpVarString +
                    lpVarString -> dwStringOffset));


           ErrorConnecting:

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
		    return;
		}
		memset(lpCallInfo,0,dwSize);
		lpCallInfo->dwTotalSize = dwSize ;
	    }
	    else if ( lrc ) {
		/* some kind of TAPI error */
		OutputDebugLastError(lrc,"lineGetCallInfo:");
		free(lpCallInfo);
		return;
	    }
	    else 
		break;  /* success */
	} while ( TRUE );

	dwMediaMode = lpCallInfo->dwMediaMode;
	free(lpCallInfo);

	/* Get the hCall */
	if ( dwParam3 == LINECALLPRIVILEGE_OWNER ) {

	    /* Need to check the Bearer Mode of the call */

	    /* are we even answering calls? */
	    if ( g_bAnswering )
		;
	    else {
		rc = (*cklineHandoff)(g_hCall, NULL, LINEMEDIAMODE_DATAMODEM);
		OutputDebugLastError(rc,"lineHandoff:");
		rc = (*cklineDeallocateCall)(g_hCall);
		OutputDebugLastError(rc,"lineDeallocateCall:");
		g_hCall = (HCALL)0;
	    }
	}
	else /* if ( dwParam3 == LINECALLPRIVILEGE_MONITOR ) */ {
	    if ( g_bAnswering && 
		 !(dwMediaMode & ~(LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_UNKNOWN))
		 ) {
		rc = (*cklineSetCallPrivilege)( g_hCall, LINECALLPRIVILEGE_OWNER );
		OutputDebugLastError(rc,"lineSetCallPrivilege:");
		if ( !rc ) {
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
	DoLineCallState(dwDevice, dwMsg, dwCallbackInstance,
			 dwParam1, dwParam2, dwParam3);
	break;

        case LINE_CLOSE:
	debug(F110,"TAPI Callback","Line Close",0);
	DoLineClose(dwDevice, dwMsg, dwCallbackInstance,
		     dwParam1, dwParam2, dwParam3);
	break;

        case LINE_LINEDEVSTATE:
	debug(F110,"TAPI Callback","Line LineDevState",0);
	DoLineDevState(dwDevice, dwMsg, dwCallbackInstance,
			dwParam1, dwParam2, dwParam3);
	break;

        case LINE_REPLY:
	debug(F110,"TAPI Callback","Line Reply",0);
	DoLineReply(dwDevice, dwMsg, dwCallbackInstance,
		     dwParam1, dwParam2, dwParam3);
	break;

        case LINE_CREATE:
	debug(F110,"TAPI Callback","Line Create",0);
	DoLineCreate(dwDevice, dwMsg, dwCallbackInstance,
		      dwParam1, dwParam2, dwParam3);
	break;

        default:
	debug(F110,"TAPI Callback","default",0);
	break;
    }

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
        lReturn = (*cklineInitialize)(&g_hLineApp, hInstance, 
             cklineCallbackFunc, "K95 Dialer", &g_lNumDevs);
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
		g_hLine = (HLINE)0 ;
		ttyfd = -1;
	    }
	}
	if ( tapiopen == 0 ) {
	    rc = (*cklineShutdown)( g_hLineApp ) ;
	    OutputDebugLastError(rc,"lineShutdown:");
	    g_hLineApp = (HLINEAPP)0 ;
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
						       "SaveTapiLocationInfo");

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
			  LPLINELOCATIONENTRY *lpLocationEntry, 
                          int * pN )
{
    LPLINETRANSLATECAPS lpTranslateCaps=NULL;
   DWORD dwSizeofTranslateCaps = sizeof(LINETRANSLATECAPS);
   long lReturn = 0;
   DWORD dwCounter=0;
   LPLINECARDENTRY lpLineCardEntry = NULL;
   int i = 0 ;

    *lpTC = NULL;
    *lpLocationEntry = NULL;
    *pN = 0;

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

    /* Toll Free Prefix */
    if ( dialtfp )
        free(dialtfp);
    /* The true Toll Free Prefix is the tapiLongDistAccessCode */
    /* plus the prefix portion of the tapiLongDistanceRule         */
    dialtfp = (char *) malloc(strlen(tapiLongDistAccessCode) +
                               strlen(tapiLongDistanceRule) + 1);
    if ( dialtfp ) {
        strcpy(dialtfp,tapiLongDistAccessCode);
        strcat(dialtfp,tapiLongDistanceRule);
        p = dialtfp;
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

    if ( hLine == (HLINE)0 )
	hLine = g_hLine;

    if ( hLine == (HLINE)0 )
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
	lrc = (*cklineGetID)(hLine, 0L, (HLINEAPP)0,
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

    if ( g_hLine == (HLINE)0 ) {
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
		    debug(F111,"TAPI lineOpen","g_hLine",(long)g_hLine);
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

    if ( g_hLine == (HLINE)0 )
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

int
cktapiSetModemSettings( LPDEVCFG lpDevCfg )
{
    ULONG lrc=0 ;

    if ( lpDevCfg == NULL )
	return FALSE;

    lrc = (*cklineSetDevConfig)( LineDeviceId, 
				 lpDevCfg,
				 lpDevCfg->dfgHdr.dwSize, 
				 "comm/datamodem" );
    OutputDebugLastError(lrc,"lineSetDevConfig:");
    if ( lrc < 0 )
    {
    	debug(F101,"cktapiSetModemSettings","rc",lrc);
	return FALSE;
    }
    return TRUE;
}
#endif /* CK_TAPI */
