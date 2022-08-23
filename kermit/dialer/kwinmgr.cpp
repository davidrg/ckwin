#ifndef WIN32
#define INCL_DOSSESMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#endif /* WIN32 */
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_WINDOW_DIALER
#define USE_HELP_CONTEXTS
#include "kenum.hpp"
#include "usermsg.hpp"
#include "kwinmgr.hpp"
#include "ksetdlg.hpp"

EVENT_TYPE
K_WINDOW_MANAGER::Event( const UI_EVENT & event )
{
    EVENT_TYPE rc = event.type ;
    switch ( event.type ) {
    case S_CLOSE: {
	UI_WINDOW_OBJECT * currentWindow = First() ;
	if ( strcmp( currentWindow->ClassName(), "K_SETTINGS_NOTEBOOK" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_GENERAL_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_TERMINAL_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_TRANSFER_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_SERIAL_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_TELNET_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_TCP_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_KERBEROS_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_TLS_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_KEYBOARD_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_LOGIN_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_PRINTER_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_LOGS_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_GUI_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_SSH_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIALOG_FTP_SETTINGS" ) &&
	     strcmp( currentWindow->ClassName(), "K_DIAL_OPTIONS" ) &&
	     strcmp( currentWindow->ClassName(), "K_MODEM_CONFIG" ) &&
	     strcmp( currentWindow->ClassName(), "K_MODEM_DIALOG" )
	     ) {
	    rc = UI_WINDOW_MANAGER::Event( event ) ;
	} else {
            K95_SETTINGS_DIALOG * dialog = (K95_SETTINGS_DIALOG *) currentWindow;
            if ( dialog->_cancelled ) {

                /* Get user confirmation */
                ZAF_MESSAGE_WINDOW * confirm = 
                    new ZAF_MESSAGE_WINDOW( "Cancel?", "QUESTION", 
					ZIL_MSG_YES | ZIL_MSG_NO,
					ZIL_MSG_NO,
                                            (!strcmp(dialog->ClassName(), "K_DIALOG_GENERAL_SETTINGS") || 
                                            dialog->GetMode() != ADD_ENTRY) ?
					"Are you sure you want to Cancel?" :
                                        "Are you sure you are Finished?") ;
                if ( confirm->Control() != ZIL_DLG_YES ) {
                    rc = 0;
                }
                else {
                    rc = UI_WINDOW_MANAGER::Event( event ) ;
                }
                delete confirm;
            } else
                rc = UI_WINDOW_MANAGER::Event( event );
	}
	break;
    }

    default:
	rc = UI_WINDOW_MANAGER::Event( event ) ;
    }
    return rc;
}
