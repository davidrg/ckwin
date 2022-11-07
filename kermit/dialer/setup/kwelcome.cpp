#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT 
#endif /* COMMENT */
#include <stdio.h>
#include "ksetdlg.hpp"
#include "kwelcome.hpp"

ZIL_ICHAR K_WELCOME_DIALOG::_className[] = "K_WELCOME_DIALOG";

K_WELCOME_DIALOG::
K_WELCOME_DIALOG(void) : K_SETUP_DIALOG("DIALOG_WELCOME")
{
}

EVENT_TYPE K_WELCOME_DIALOG::
Event (const UI_EVENT & event )
{
    EVENT_TYPE retval = event.type ; 
    K_SETUP_DIALOG    * dialog = NULL;

    switch ( retval ) {
    case 10001:         /* Previous */
        dialog = Previous();
        if ( dialog != this ) {
            *windowManager - this;
            next = dialog;
        }

        break;
    case 10002:         /* Next */
        dialog = Next();
        if ( dialog != this ) {
            *windowManager - this;
            next = dialog;
        }
        break;
        
    }
    return retval;
}

K_WELCOME_DIALOG::
~K_WELCOME_DIALOG()
{
        
}

K_SETUP_DIALOG * K_WELCOME_DIALOG::
Previous( void )
{
    return this;
}

K_SETUP_DIALOG * K_WELCOME_DIALOG::
Next( void )
{
    return this;
}

