// Include the appropriate directives.
#include <ui_win.hpp>
#define USE_K95_INIT
#include <stdio.h>
#include "k95cinit.hpp"
#include "cinit.hpp"

static ZIL_ICHAR title[] = { 'H','e','l','l','o',' ','W','o','r','l','d',' ','W','i','n','d','o','w',0 };
static ZIL_ICHAR text[] = { 'H','e','l','l','o',',',' ','W','o','r','l','d','!',0 };
K_CINIT * cinit=NULL;

int 
UI_APPLICATION::Main(void)
{
    // Fix linkers that don't look for main in the .LIBs.
    UI_APPLICATION::LinkMain();

    // Open general storage file which contains windows.
    UI_WINDOW_OBJECT::defaultStorage = new ZIL_STORAGE_READ_ONLY("k95cinit.dat");

    // Check to see if the default storage was allocated.
    if (UI_WINDOW_OBJECT::defaultStorage->storageError)
    {
	delete UI_WINDOW_OBJECT::defaultStorage;
	return(-1);
    }

    if ( lpszCmdLine && *lpszCmdLine )
    {
	cinit = new K_CINIT(lpszCmdLine);
    }
    else
    {
    	cinit = new K_CINIT("COM1");
    }

    // Add the control windows.
    *windowManager + cinit ;
    UI_EVENT open_evt(OPEN_COMM);
    eventManager->Put(open_evt);

    // Process user responses.
    UI_APPLICATION::Control();

    // Clean up and return success.
    delete UI_WINDOW_OBJECT::defaultStorage;
    return (0);
}

