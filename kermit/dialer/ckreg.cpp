// Include the appropriate directives.
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#include "dialer.hpp"
#include "kreg.hpp"

K_REG * regwin = NULL ;

int UI_APPLICATION::Main(void)
{
    // Fix linkers that don't look for main in the .LIBs.
    UI_APPLICATION::LinkMain();

    // Open general storage file which contains windows.
    UI_WINDOW_OBJECT::defaultStorage = new ZIL_STORAGE_READ_ONLY("dialer.dat");

    // Check to see if the default storage was allocated.
    if (UI_WINDOW_OBJECT::defaultStorage->storageError)
    {
	delete UI_WINDOW_OBJECT::defaultStorage;
	return(-1);
    }

    UI_WINDOW_OBJECT::helpSystem = new UI_HELP_SYSTEM( "dialer.dat", windowManager, HELP_MENU ) ;
    UI_WINDOW_OBJECT::errorSystem = new UI_ERROR_SYSTEM ;

#ifdef WIN32
    if ( lpszCmdLine && *lpszCmdLine && !strncmp(lpszCmdLine, "-S", 2) )
    {
	regwin = new K_REG( -atol( lpszCmdLine+1 ) ) ;
    }
    else
    {
	regwin = new K_REG(0) ;
    }
#else
    regwin = new K_REG(0);
#endif

    if ( regwin->Prepare() )
    {
	// Add the control windows.
	*windowManager + regwin ;
	windowManager->screenID = regwin->screenID ;

	// Process user responses.
	UI_APPLICATION::Control();
    }

    // Clean up and return success.

    delete UI_WINDOW_OBJECT::defaultStorage;
    return (0);
}

