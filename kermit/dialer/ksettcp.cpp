#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_TCP_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksettcp.hpp"
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

ZIL_ICHAR K_DIALOG_TCP_SETTINGS::_className[] = "K_DIALOG_TCP_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_TCP_SETTINGS
//
//

K_DIALOG_TCP_SETTINGS::
K_DIALOG_TCP_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_TCP_SETTINGS",ADD_ENTRY),
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
    button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
    format->DataSet( "000.000.000.000");
    format->woFlags |= WOF_NON_SELECTABLE;
    format->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_SENDBUF ) ;
    int new_spin_int = 8192 ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= WOF_NON_SELECTABLE;
    spin->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_RECVBUF ) ;
    new_spin_int = 8192 ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= WOF_NON_SELECTABLE;
    spin->Information( I_CHANGED_FLAGS, NULL );

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
    combo->DataSet("Auto");
    combo->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_TCP_SETTINGS
//
//

K_DIALOG_TCP_SETTINGS::
K_DIALOG_TCP_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_TCP_SETTINGS",mode),
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

    printf("Entering K_DIALOG_TCP_SETTINGS::K_DIALOG_TCP_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"TCP/IP Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // TCP/IP Page
    button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    if ( entry->_default_ip_address ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? 
                         WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
    format->DataSet(entry->_tcp_ip_address);
    format->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP ||
                          entry->_default_ip_address) ? WOF_NON_SELECTABLE : 0);
    format->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_SENDBUF ) ;
    int new_spin_int = entry->_tcp_sendbuf ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    spin->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_RECVBUF ) ;
    new_spin_int = entry->_tcp_recvbuf ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    spin->Information( I_CHANGED_FLAGS, NULL );

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
    switch ( entry->_tcp_rdns ) {
    case AUTO:
        combo->DataSet("Auto");
        break;
    case ON:
        combo->DataSet("On");
        break;
    case OFF:
        combo->DataSet("Off");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
    if ( entry->_tcp_dns_srv ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( connector->_dns_srv_avail )
        button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_SOCKS_ADDR);
    string->DataSet(entry->_socks_ipaddress,256);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_SOCKS_PORT);
    string->DataSet(entry->_socks_ipport,32);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_PROXY_ADDR);
    string->DataSet(entry->_proxy_ipaddress,256);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_PROXY_PORT);
    string->DataSet(entry->_proxy_ipport,32);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_SOCKS_USER);
    string->DataSet(entry->_socks_user,64);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_SOCKS_PASS);
    string->DataSet(entry->_socks_pass,64);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_PROXY_USER);
    string->DataSet(entry->_proxy_user,64);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get(FIELD_TCP_PROXY_PASS);
    string->DataSet(entry->_proxy_pass,64);
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    string->Information(I_CHANGED_FLAGS,NULL);

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_TCP_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_TCP_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_TCP_SETTINGS::K_DIALOG_TCP_SETTINGS(entry)\n");
};

K_DIALOG_TCP_SETTINGS::~K_DIALOG_TCP_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_TCP_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_TCP_SETTINGS::Event()\n");

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

    case OPT_TCP_OK: {
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

    case OPT_CHECK_IP_ADDRESS:
        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        if ( FlagSet(Get( CHECK_TCP_IP_ADDRESS )->woStatus, WOS_SELECTED ) )
            format->woFlags |= WOF_NON_SELECTABLE;
        else
            format->woFlags &= ~WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );
        break;

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_TCP_SETTINGS::
ApplyChangesToEntry( void )
{
    // TCP/IP Page
    UIW_BUTTON * button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    _entry->_default_ip_address = button->woStatus & WOS_SELECTED;

    UIW_FORMATTED_STRING * format;
    format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS ) ;
    strncpy( _entry->_tcp_ip_address, format->DataGet(), 18 ) ;

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_SENDBUF ) ;
    int new_spin_int;
    spin->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_tcp_sendbuf = new_spin_int;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_RECVBUF ) ;
    spin->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_tcp_recvbuf = new_spin_int;

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
    ZIL_ICHAR * ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Auto") )
        _entry->_tcp_rdns = AUTO;
    else if ( !strcmp(ichar,"On") )
        _entry->_tcp_rdns = ON;
    else if ( !strcmp(ichar,"Off") )
        _entry->_tcp_rdns = OFF;

    button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
    _entry->_tcp_dns_srv = button->woStatus & WOS_SELECTED;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_TCP_SOCKS_ADDR );
    strncpy( _entry->_socks_ipaddress, string->DataGet(), 256 );
    _entry->_socks_ipaddress[256] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_SOCKS_PORT );
    strncpy( _entry->_socks_ipport, string->DataGet(), 32 );
    _entry->_socks_ipport[32] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_PROXY_ADDR );
    strncpy( _entry->_proxy_ipaddress, string->DataGet(), 256 );
    _entry->_proxy_ipaddress[256] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_PROXY_PORT );
    strncpy( _entry->_proxy_ipport, string->DataGet(), 32 );
    _entry->_proxy_ipport[32] = '\0';

    string = (UIW_STRING *) Get( FIELD_TCP_SOCKS_USER );
    strncpy( _entry->_socks_user, string->DataGet(), 64 );
    _entry->_socks_user[64] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_SOCKS_PASS );
    strncpy( _entry->_socks_pass, string->DataGet(), 64 );
    _entry->_socks_pass[64] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_PROXY_USER );
    strncpy( _entry->_proxy_user, string->DataGet(), 64 );
    _entry->_proxy_user[64] = '\0';
    string = (UIW_STRING *) Get( FIELD_TCP_PROXY_PASS );
    strncpy( _entry->_proxy_pass, string->DataGet(), 64 );
    _entry->_proxy_pass[64] = '\0';
}

