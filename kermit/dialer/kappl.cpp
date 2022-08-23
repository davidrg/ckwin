#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#define USE_WINDOW_APPLICATIONS
#include <stdio.h>
#include "dialer.hpp"
#include "kdconfig.hpp"
#include "kappl.hpp"
#include "usermsg.hpp"
#include "kconnect.hpp"

extern K_CONNECTOR * connector ;
K_APPLICATIONS::K_APPLICATIONS( KD_CONFIG * config)
   : ZAF_DIALOG_WINDOW("WINDOW_APPLICATIONS",defaultStorage),_config(config)
{ 
#ifdef COMMENT
#ifdef WIN32
    Information( I_SET_TEXT, "Kermit 95 Network Directory Files" ) ;
#else
    Information( I_SET_TEXT, "Kermit 95 for OS/2 Network Directory Files" ) ;
#ifdef COMMENT
    UIW_ICON * icon = (UIW_ICON *) Get( ICON_K95 ) ;
    *this - icon ;
    delete icon ;
    icon = new UIW_ICON( 0,0,"CKNKER","K95/2 Network Directory Files", 
			 ICF_MINIMIZE_OBJECT );
    *this + icon ;
#endif /* COMMENT */
#endif /* WIN32 */
#endif /* COMMENT */

    UIW_STRING * string = (UIW_STRING *) Get( TEXT_APPL_EDITOR ) ;
    string->DataSet( _config->_app_edit ) ;

#ifdef COMMENT
    string = (UIW_STRING *) Get( TEXT_APPL_FTP ) ;
    string->DataSet( _config->_app_ftp ) ;
#endif /* COMMENT */

    string = (UIW_STRING *) Get( TEXT_APPL_BROWSER ) ;
    string->DataSet( _config->_app_www ) ;

   windowManager->Center( this ) ;
};

EVENT_TYPE K_APPLICATIONS::Event( const UI_EVENT & event ) 
{ 
    EVENT_TYPE ccode = event.type ;
    UIW_STRING * string = NULL ;

    switch ( event.type )
    {
    case OPT_APPL_HELP:
        helpSystem->DisplayHelp( windowManager, HELP_APPLICATIONS ) ;
        break;

    case OPT_APPL_OK:
        string = (UIW_STRING *) Get( TEXT_APPL_EDITOR ) ;
        strncpy( _config->_app_edit, string->DataGet(), 256 ) ;
#ifdef COMMENT
        string = (UIW_STRING *) Get( TEXT_APPL_FTP ) ;
        strncpy( _config->_app_ftp, string->DataGet(), 256 ) ;
#endif /* COMMENT */
        string = (UIW_STRING *) Get( TEXT_APPL_BROWSER ) ;
        strncpy( _config->_app_www, string->DataGet(), 256 ) ;
        connector->SaveConfig() ;
        *windowManager - this ;
        break; 

    default:
        ccode = UIW_WINDOW::Event( event ) ;
    }
    return ccode;
}

