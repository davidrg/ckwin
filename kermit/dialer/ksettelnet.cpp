#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_TELNET_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksettelnet.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"
#ifdef WIN32
extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#undef printf
#undef fprintf
#include <windows.h>            	/* Windows Definitions */
}
#endif /* WIN32 */

ZIL_ICHAR K_DIALOG_TELNET_SETTINGS::_className[] = "K_DIALOG_TELNET_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_TELNET_SETTINGS
//
//

K_DIALOG_TELNET_SETTINGS::
K_DIALOG_TELNET_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_TELNET_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    UIW_STRING * string = NULL ;
    UIW_BUTTON * button = NULL ;
    UIW_BUTTON * button2 = NULL ;
    UIW_COMBO_BOX * combo = NULL ;
    UIW_TEXT * text = NULL ;
    UIW_INTEGER * integer = NULL ;
    UIW_SPIN_CONTROL * spin = NULL ;
    UIW_INTEGER * spin_int = NULL;
    UIW_GROUP * group = NULL ;
    UIW_VT_LIST * list = NULL ;
    UIW_FORMATTED_STRING * format = NULL;
    int value = 0 ;
    KD_LIST_ITEM * _template = NULL ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // TCP/IP Page
    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
    string->DataSet( "", 40 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_DISP );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_JOB );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_TELNET_SETTINGS
//
//

K_DIALOG_TELNET_SETTINGS::
K_DIALOG_TELNET_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_TELNET_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    UIW_STRING * string = NULL ;
    UIW_BUTTON * button = NULL ;
    UIW_BUTTON * button2 = NULL ;
    UIW_COMBO_BOX * combo = NULL ;
    UIW_TEXT * text = NULL ;
    UIW_INTEGER * integer = NULL ;
    UIW_SPIN_CONTROL * spin = NULL ;
    UIW_INTEGER * spin_int = NULL;
    UIW_GROUP * group = NULL ;
    UIW_VT_LIST * list = NULL ;
    UIW_FORMATTED_STRING * format = NULL;
    int value = 0 ;
    KD_LIST_ITEM * _template = NULL ;

    printf("Entering K_DIALOG_TELNET_SETTINGS::K_DIALOG_TELNET_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Telnet Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Telnet Page
    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    switch ( entry->_telnet_auth_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    switch ( entry->_telnet_binary_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    switch ( entry->_telnet_encrypt_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    switch ( entry->_telnet_kermit_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    switch ( entry->_telnet_starttls_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_FWDX );
    switch ( entry->_telnet_fwdx_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_CPC );
    switch ( entry->_telnet_cpc_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;


    string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
    string->DataSet( entry->_telnet_location, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0)
	| WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
    string->DataSet( entry->_telnet_ttype, 40 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0)
	| WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
    string->DataSet( entry->_telnet_acct, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_DISP );
    string->DataSet( entry->_telnet_disp, 64 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? 
                         WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_JOB );
    string->DataSet( entry->_telnet_job, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
    string->DataSet( entry->_telnet_printer, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    if ( entry->_fwd_cred ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    if ( entry->_telnet_wait ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    if ( entry->_telnet_debug ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    if ( entry->_telnet_sb_delay ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_TELNET_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_TELNET_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_TELNET_SETTINGS::K_DIALOG_TELNET_SETTINGS(entry)\n");
};

K_DIALOG_TELNET_SETTINGS::~K_DIALOG_TELNET_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_TELNET_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_TELNET_SETTINGS::Event()\n");

    EVENT_TYPE retval = event.type ; 
    UIW_STRING * string = NULL ;
    UIW_BUTTON * button = NULL ;
    UIW_BUTTON * button2 = NULL ;
    UIW_COMBO_BOX * combo = NULL ;
    UIW_TEXT * text = NULL ;
    UIW_INTEGER * integer = NULL ;
    UIW_SPIN_CONTROL * spin = NULL ;
    UIW_GROUP * group = NULL ;
    UIW_VT_LIST * list = NULL ;
    UIW_FORMATTED_STRING * format = NULL;
    int value = 0 ;
    KD_LIST_ITEM * _template = NULL ;
    static EVENT_TYPE DirServRequestor = 0 ;

    if ( !_initialized ) {
	if ( event.type == OPT_INITIALIZED )
	    _initialized = 1 ;
	else 
	    retval = UIW_WINDOW::Event(event);
    }
    else 
    switch( event.type ) 
    {

    case OPT_HELP_SET_TCPIP:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TCPIP ) ;
        break;

    case OPT_TELNET_OK: {
       ApplyChangesToEntry() ;
       ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, _entry->_name,
                                      ID_KD_LIST_ITEM, UIS_OPENCREATE | UIS_READWRITE ) ;
       if ( _fileObj.objectError )
       {
            // What went wrong?
	   ZAF_MESSAGE_WINDOW * message =
               new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                       "Unable to Open %s: errorno %d", _entry->_name, _fileObj.objectError ) ;
	   message->Control() ;
	   delete message ;
       }  
       else 
       {
	   _entry->Store( _entry->_name, K_CONNECTOR::_userFile, &_fileObj,
			  ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
	   K_CONNECTOR::_userFile->Save() ;
	   if ( K_CONNECTOR::_userFile->storageError )
	   {
               // What went wrong?
               ZAF_MESSAGE_WINDOW * message =
		   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to Open %s: errorno %d", _entry->_name, 
                                          K_CONNECTOR::_userFile->storageError ) ;
               message->Control() ;
               delete message ;
	   }

	   connector->AddEntry( _entry ) ;
       }
        _cancelled = 0;
        UI_EVENT event(S_CLOSE);
        event.windowObject = this ;
        eventManager->Put(event);
        break;
    }

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_TELNET_SETTINGS::
ApplyChangesToEntry( void )
{
    // TCP/IP Page
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    ZIL_ICHAR * ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_auth_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_auth_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_auth_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_auth_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_binary_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_binary_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_binary_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_binary_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_encrypt_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_encrypt_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_encrypt_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_encrypt_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_kermit_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_kermit_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_kermit_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_kermit_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_starttls_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_starttls_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_starttls_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_starttls_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_FWDX );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_fwdx_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_fwdx_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_fwdx_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_fwdx_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_CPC );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_cpc_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_cpc_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_cpc_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_cpc_mode = TelnetRequire;

    UIW_STRING *string = (UIW_STRING *) Get( TEXT_TCP_LOCATION ) ;
    strncpy( _entry->_telnet_location, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE ) ;
    strncpy( _entry->_telnet_ttype, string->DataGet(), 40 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT ) ;
    strncpy( _entry->_telnet_acct, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_DISP ) ;
    strncpy( _entry->_telnet_disp, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_JOB ) ;
    strncpy( _entry->_telnet_job, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER ) ;
    strncpy( _entry->_telnet_printer, string->DataGet(), 64 ) ;

    UIW_BUTTON *button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    _entry->_fwd_cred = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    _entry->_telnet_wait = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    _entry->_telnet_debug = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    _entry->_telnet_sb_delay = button->woStatus & WOS_SELECTED;

}

