// Include the appropriate directives.
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#define USE_WINDOW_DIALER
#define USE_DIALOG_DEMO
#include <direct.hpp>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "dialer.hpp"
#include "kconnect.hpp"
#include "kwinmgr.hpp"
#ifdef WIN32
extern "C" {
#define OS2 
#define NT  
#include "ckcdeb.h"
#include "ckoetc.h"
#ifndef NODIAL
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
#endif /* NODIAL */
extern HWND hwndConsole;
#ifdef printf
#undef printf
#endif
}
#else /* WIN32 */
#define _stat stat
#define _S_IFDIR S_IFDIR
#endif /* WIN32 */


K_CONNECTOR  *connector = NULL ;
UIW_TOOL_BAR *toolbar = NULL ;
UID_TIMER * timer = NULL ;
ZIL_UINT8 kd_major = 1, kd_minor = 36;
ZIL_ICHAR * pathname = "";
ZIL_UINT8    TapiAvail = 0;
ZIL_UINT8    StartMenu = 0;
ZIL_UINT8    Desktop   = 0;

int UI_APPLICATION::Main(void)
{
    pathname = argv[0] ;

    setbuf(stdout,NULL);
    printf("Main()\n");
    printf("Checking OS Version\n");

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

    HWND hwnd = FindWindow(NULL, "C-Kermit for Windows Dialer");
    if ( hwnd ) {
        if ( IsIconic(hwnd) )
            ShowWindow(hwnd, SW_SHOWNORMAL);
        else
            SetForegroundWindow( hwnd );
        return(0);
    }
#else /* WIN32 */
    Desktop = 1;
#endif /* WIN32 */

    // Fix linkers that don't look for main in the .LIBs.
    UI_APPLICATION::LinkMain();

    printf("Deleting windowManager\n");
    delete windowManager ;
    printf("Creating new WINDOW_MANAGER\n");
    windowManager = new K_WINDOW_MANAGER(display, eventManager ) ;

    /* Check TMP,TEMP,K95TMP */
    char * p = NULL;

    printf("Checking TMP,TEMP,K95TMP\n");
    p = getenv("K95TMP"); 		
    if ( p ) {
        struct _stat Stat;
        int rc = stat(p,&Stat);

        if ( rc && errno == ENOENT ) {
            printf("WARNING: K95TMP = %s which does not exist.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "K95TMP = %s.  Must point to an subdirectory that exists.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        } 

        if ( !(Stat.st_mode & _S_IFDIR) ) {
            printf("WARNING: K95TMP = %s which is a file.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "K95TMP = %s.  Must point to an subdirectory not a file.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        }
#ifdef WIN32
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                printf("WARNING: K95TMP = %s\n",p);
                ZAF_MESSAGE_WINDOW * MessageBox =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                            ZIL_MSG_OK, ZIL_MSG_OK,
                                            "K95TMP = %s.  Must point to a subdirectory.", 
                                            p) ;
                MessageBox->Control();
                delete MessageBox ;
                return(1);
            }
        }
#endif /* WIN32 */
    }
    p = getenv("TEMP");
    if ( p ) {
        struct _stat Stat;
        int rc = stat(p,&Stat);

        if ( rc && errno == ENOENT ) {
            printf("WARNING: TEMP = %s which does not exist.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "TEMP = %s.  Must point to an subdirectory that exists.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        } 

        if ( !(Stat.st_mode & _S_IFDIR) ) {
            printf("WARNING: TEMP = %s which is a file.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "TEMP = %s.  Must point to an subdirectory not a file.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        }

#ifdef WIN32
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                printf("WARNING: TEMP = %s\n",p);
                ZAF_MESSAGE_WINDOW * MessageBox =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                            ZIL_MSG_OK, ZIL_MSG_OK,
                                            "TEMP = %s.  Must point to a subdirectory.", 
                                            p) ;
                MessageBox->Control();
                delete MessageBox ;
                return(1);
            }
        }
#endif /* WIN32 */
    }
    p = getenv("TMP");
    if ( p ) {
        struct _stat Stat;
        int rc = stat(p,&Stat);

        if ( rc && errno == ENOENT ) {
            printf("WARNING: TMP = %s which does not exist.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "TMP = %s.  Must point to an subdirectory that exists.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        } 

        if ( !(Stat.st_mode & _S_IFDIR) ) {
            printf("WARNING: TMP = %s which is a file.\n",p);
            ZAF_MESSAGE_WINDOW * MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                        ZIL_MSG_OK, ZIL_MSG_OK,
                                        "TMP = %s.  Must point to an subdirectory not a file.", 
                                        p) ;
            MessageBox->Control();
            delete MessageBox ;
            return(1);
        }

#ifdef WIN32
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            if ( strlen(p) == 3 && p[1] == ':' && p[2] == '\\' ) {
                printf("WARNING: TMP = %s\n",p);
                ZAF_MESSAGE_WINDOW * MessageBox =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                            ZIL_MSG_OK, ZIL_MSG_OK,
                                            "TMP = %s.  Must point to a subdirectory.", 
                                            p) ;
                MessageBox->Control();
                delete MessageBox ;
                return(1);
            }
        }
#endif /* WIN32 */
    }

    printf("Creating defaultStorage object for dialer.dat\n");
    // Open general storage file which contains windows.
    UI_WINDOW_OBJECT::defaultStorage = new ZIL_STORAGE_READ_ONLY("dialer.dat");

    // Check to see if the default storage was allocated.
    if (UI_WINDOW_OBJECT::defaultStorage->storageError)
    {
        delete UI_WINDOW_OBJECT::defaultStorage;
        printf("Unable to allocate defaultStorage\n");
        ZAF_MESSAGE_WINDOW * MessageBox =
            new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                    ZIL_MSG_OK, ZIL_MSG_OK,
                                    "Unable to allocate storage for dialer.dat") ;
        MessageBox->Control();
        delete MessageBox ;
        return(-1);
    }

    if ( UI_WINDOW_OBJECT::errorSystem )
        delete UI_WINDOW_OBJECT::errorSystem ;
    printf("Adding ERROR_SYSTEM\n");
    UI_WINDOW_OBJECT::errorSystem = new UI_ERROR_SYSTEM ;

    // Create the service manager (level 0).
    printf("Creating SERVICE_MANAGER\n");
    _serviceManager = new ZAF_SERVICE_MANAGER;

    // Create and add base services (level 1).
    printf("Adding DIRECTORY_SERVICE\n");
    *_serviceManager
        + new ZAF_DIRECTORY_SERVICE;

    if ( UI_WINDOW_OBJECT::helpSystem )
        delete UI_WINDOW_OBJECT::helpSystem ;
    printf("Adding HELP_SYSTEM\n");
    UI_WINDOW_OBJECT::helpSystem = 
        new UI_HELP_SYSTEM( "dialer.dat", windowManager, HELP_MENU ) ;



#ifdef WIN32
#ifndef NODIAL
    printf("Checking TAPI\n");
    if (TapiAvail = cktapiinit(hInstance)) {
        printf("TAPI available\n");
        cktapiopen();
        printf("TAPI Open\n");
    }
#endif /* NODIAL */
#endif /* WIN32 */

    printf("Creating connector\n");
    connector = new K_CONNECTOR ;
    printf("Connector created\n");

#ifdef WIN32
    if ( argc == 4 && !strcmp(argv[1],"-k")) {
        connector->_hwndNextConnect = (HWND) atoi(argv[2]);
        connector->_dwNextConnectPid = atoi(argv[3]);
    }
#endif

    timer = new UID_TIMER( D_ON, 1000 ) ;
    *eventManager + timer ;

    UI_EVENT event( S_ADD_OBJECT );
    event.windowObject = connector ;
    timer->Event( event ) ;

    // Add the control windows.
    *windowManager + connector ;
    windowManager->screenID = connector->screenID ;
#ifdef WIN32
#ifndef NODIAL
    if (TapiAvail) {
        hwndConsole = connector->screenID;
    }
#endif
#endif

    // Process user responses.
    printf("Entering Control() loop\n");
    UI_APPLICATION::Control();

#ifdef WIN32
#ifndef NODIAL
    if ( TapiAvail ) {
        printf("Closing TAPI\n");
        cktapiclose();
    }
#endif /* NODIAL */
#endif /* WIN32 */

    // Clean up and return success.
    delete UI_WINDOW_OBJECT::defaultStorage;
    delete _serviceManager;

    printf("Done\n");
    return (0);
}

