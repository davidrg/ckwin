// Include the appropriate directives.
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#define USE_WINDOW_REGISTRY
#include <direct.hpp>
#include <stdio.h>
#include "registry.hpp"
#include "kregistry.hpp"

K_WINDOW * k_window = NULL;
ZIL_ICHAR * pathname = NULL;

int UI_APPLICATION::Main(void)
{
    pathname = argv[0] ;

    // Fix linkers that don't look for main in the .LIBs.
    UI_APPLICATION::LinkMain();

    // Open general storage file which contains windows.
    UI_WINDOW_OBJECT::defaultStorage = 
        new ZIL_STORAGE_READ_ONLY("registry.dat");

    // Check to see if the default storage was allocated.
    if (UI_WINDOW_OBJECT::defaultStorage->storageError)
    {
        delete UI_WINDOW_OBJECT::defaultStorage;
        return(-1);
    }

    k_window = new K_WINDOW ;

    UI_EVENT event( S_ADD_OBJECT );
    event.windowObject = k_window ;

    // Add the control windows.
    *windowManager + k_window ;
    windowManager->screenID = k_window->screenID ;

    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    printf("PlatformID   = %d\n",osverinfo.dwPlatformId);
    printf("MajorVersion = %d\n",osverinfo.dwMajorVersion);
    printf("MinorVersion = %d\n",osverinfo.dwMinorVersion);
    printf("BuildNumber  = %d\n",osverinfo.dwBuildNumber);

    if ( !(osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
         osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
         osverinfo.dwMajorVersion >= 4) ) {
        ZAF_MESSAGE_WINDOW * MessageBox =
            new ZAF_MESSAGE_WINDOW( "Error", 
                                    "EXCLAMATION", 
                                    ZIL_MSG_OK, ZIL_MSG_OK,
                                    "Incorrect version of Windows");
        MessageBox->Control();
        delete MessageBox ;
        return(1);
    }


    // Process user responses.
    UI_APPLICATION::Control();

    // Clean up and return success.
    delete UI_WINDOW_OBJECT::defaultStorage;
    return (0);
}

