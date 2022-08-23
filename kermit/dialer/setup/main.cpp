// Include the appropriate directives.
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#ifdef COMMENT
#define USE_WINDOW_SETUP
#define USE_DIALOG_WELCOME
#define USE_DIALOG_DESTINATION
#define USE_DIALOG_NETWORK_ONLY
#define USE_DIALOG_TAPI_MODEM
#define USE_DIALOG_MODEM
#define USE_DIALOG_PRINTER
#define USE_DIALOG_LOCATION
#define USE_DIALOG_REGISTRATION
#define USE_DIALOG_REGISTRY
#define USE_DIALOG_DONE
#define USE_DIALOG_READ_ME
#define USE_DIALOG_READY
#define USE_DIALOG_UNPACKING
#endif /* COMMENT */
#include <direct.hpp>
#include <stdio.h>
#include "setup.hpp"
#include "ksetup.hpp"
#include "ksetdlg.hpp"
#include "kwelcome.hpp"

#ifdef WIN32
extern "C" {
#ifndef OS2
#define OS2
#endif
#ifndef NT
#define NT  
#endif
#include "ckcdeb.h"
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
extern HWND hwndConsole;
#ifdef printf
#undef printf
#endif
}
#endif /* WIN32 */

K_SETUP  *setup = NULL ;
UIW_TOOL_BAR *toolbar = NULL ;
UID_TIMER * timer = NULL ;
ZIL_ICHAR * pathname = "";
ZIL_UINT8    TapiAvail = 0;
ZIL_UINT8    StartMenu = 0;
ZIL_UINT8    Desktop   = 0;

K_SETUP_DIALOG * dialog = NULL;

int UI_APPLICATION::Main(void)
{
    pathname = argv[0] ;

    // Fix linkers that don't look for main in the .LIBs.
    UI_APPLICATION::LinkMain();

    // Open general storage file which contains windows.
    UI_WINDOW_OBJECT::defaultStorage = new ZIL_STORAGE_READ_ONLY("setup.dat");

#ifdef COMMENT2
#ifdef COMMENT
    delete windowManager ;
    windowManager = new K_WINDOW_MANAGER(display, eventManager ) ;
#endif

    // Check to see if the default storage was allocated.
    if (UI_WINDOW_OBJECT::defaultStorage->storageError)
    {
        delete UI_WINDOW_OBJECT::defaultStorage;
        return(-1);
    }

    // Create the service manager (level 0).
    _serviceManager = new ZAF_SERVICE_MANAGER;

    // Create and add base services (level 1).
    *_serviceManager
        + new ZAF_DIRECTORY_SERVICE;

#ifdef COMMENT
    if ( UI_WINDOW_OBJECT::helpSystem )
        delete UI_WINDOW_OBJECT::helpSystem ;
    UI_WINDOW_OBJECT::helpSystem = 
        new UI_HELP_SYSTEM( "setup.dat", windowManager, HELP_MENU ) ;
#endif /* COMMENT */

    if ( UI_WINDOW_OBJECT::errorSystem )
        delete UI_WINDOW_OBJECT::errorSystem ;
    UI_WINDOW_OBJECT::errorSystem = new UI_ERROR_SYSTEM ;

#ifdef WIN32
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    printf("PlatformID   = %d\n",osverinfo.dwPlatformId);
    printf("MajorVersion = %d\n",osverinfo.dwMajorVersion);
    printf("MinorVersion = %d\n",osverinfo.dwMinorVersion);
    printf("BuildNumber  = %d\n",osverinfo.dwBuildNumber);

    if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
         osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
         osverinfo.dwMajorVersion >= 4 ) {
        StartMenu = 1;
        Desktop = 1;
    }
#else /* WIN32 */
    Desktop = 1;
#endif /* WIN32 */

#ifdef WIN32
    if (TapiAvail = cktapiinit(hInstance)) {
        cktapiopen();
    }
#endif
#endif /* COMMENT2 */
    
    dialog = new K_WELCOME_DIALOG();
    *windowManager + dialog;

    // Add the control windows.
#ifdef _WIN32
    if (TapiAvail) {
        hwndConsole = dialog->screenID;
    }
#endif

    UI_APPLICATION::Control();

#ifdef WIN32
    if ( TapiAvail )
        cktapiclose();
#endif
    // Clean up and return success.
    delete UI_WINDOW_OBJECT::defaultStorage;
    delete _serviceManager;
    return (0);
}

