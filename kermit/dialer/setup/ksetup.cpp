#ifndef WIN32
#define INCL_WINWORKPLACE
#define INCL_WINSHELLDATA
#endif /* WIN32 */
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_WINDOW_SETUP
#define USE_HELP_CONTEXTS
#undef COMMENT
#ifdef WIN32
#include <shellapi.h>
#include <shlguid.h>
#include <shlobj.h>
#include <objbase.h>
#endif /* WIN32 */
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <direct.h>
#include <sys/stat.h>
#include "setup.hpp"
#include "ksetup.hpp"
#include "ksetdlg.hpp"
#include "kwelcome.hpp"

extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#undef printf
#undef fprintf
#include "kmdminf.h"
MDMINF * FindMdmInf( char * name );
}

extern K_SETUP  *setup;
extern ZIL_UINT8 kd_major, kd_minor;

#ifdef WIN32
extern "C" {
#include <windows.h>            	/* Windows Definitions */
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
    extern struct keytab * tapilinetab, * _tapilinetab;
    extern int ntapiline;
}
#endif /* WIN32 */

extern ZIL_UINT8 TapiAvail, Desktop, StartMenu;
extern "C" {
int ck_encrypt( char * );
}


K_SETUP::K_SETUP(void)
   : ZAF_DIALOG_WINDOW("WINDOW_SETUP",defaultStorage)
{
    woStatus |= WOS_MAXIMIZED;
    Information(I_CHANGED_STATUS,NULL);
}

K_SETUP::~K_SETUP(void)
{
	
}

EVENT_TYPE K_SETUP::
Event( const UI_EVENT & event )
{
    UI_EVENT myevent(event);

    K_SETUP_DIALOG * dialog;
    switch ( event.type ) {
    case 10000:
        myevent.windowObject = this;
        myevent.type = 10001;
        eventManager->Put(myevent);
        break;
    case 10001:
        dialog = new K_WELCOME_DIALOG();
        do {
            *windowManager + dialog;
            dialog->Control();
            K_SETUP_DIALOG * next = dialog->next;
            delete dialog;
            dialog = next;
        } while (dialog);

#ifdef COMMENT
        myevent.windowObject = this;
        myevent.type = S_CLOSE;
        eventManager->Put(myevent);
#endif

        break;
    default:
        return(UIW_WINDOW::Event(event));
    }
    return(event.type);
}
