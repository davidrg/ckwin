#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_GENERAL_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetgeneral.hpp"
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

ZIL_ICHAR K_DIALOG_GENERAL_SETTINGS::_className[] = "K_DIALOG_GENERAL_SETTINGS" ;
extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_GENERAL_SETTINGS
//
//

K_DIALOG_GENERAL_SETTINGS::
K_DIALOG_GENERAL_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_GENERAL_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _original_name(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    Information( I_SET_TEXT, "Add New Connection Entry") ;
    
    _ip[0] = _lat[0] = _phone[0] = '\0';

    // Configure default values
    // General Page
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
    string->DataSet( "", 64 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    string->DataSet( "", 64 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    _transport = PHONE;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_CONNECTION_TYPE ) ;
    UIW_BUTTON * button = (UIW_BUTTON *) Get( RADIO_PHONE_NUMBER ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;

#ifdef WIN32
    if ( !connector->_ssh_avail ) {
        /* TODO: This isn't working currently */
        button = (UIW_BUTTON *) Get( RADIO_SSH );
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
#else /* WIN32 */
    button = (UIW_BUTTON *) Get( RADIO_SSH );
    *combo - button;
#endif /* WIN32 */
    
    UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
    prompt->DataSet("Phone number:");

    string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
    string->DataSet( "", 256 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
    string->DataSet( "", 32 ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    InitTCPProtoList();

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL ) ;
    UIW_VT_LIST * list  = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    button = (UIW_BUTTON *) list->Get( "default" ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_STARTUP_DIR ) ;
    string->DataSet( "", 256 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_TEMPLATE ) ;
    button->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;

   UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_NOTES ) ;
   text->DataSet( "", 3000 ) ;

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_GENERAL_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_GENERAL_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_GENERAL_SETTINGS
//
//

K_DIALOG_GENERAL_SETTINGS::
K_DIALOG_GENERAL_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode )
   : K95_SETTINGS_DIALOG("DIALOG_GENERAL_SETTINGS",mode),
   _entry(entry),
   _original_name(NULL),
   _initialized(0)
{
    UIW_BUTTON * button, * button2;
    UIW_COMBO_BOX * combo;

    printf("Entering K_DIALOG_GENERAL_SETTINGS::K_DIALOG_GENERAL_SETTINGS(entry,mode)\n");

    ZIL_ICHAR dialogName[128];
    switch (_mode) {
    case EDIT_ENTRY:
        _original_name = strdup( _entry->_name ) ;
        sprintf(dialogName,"General Settings: %s",_entry->_name);
        button = (UIW_BUTTON *) Get( CHECK_TEMPLATE );
        if ( entry->_template && _mode != CLONE_ENTRY )
            button->woStatus |= WOS_SELECTED ;
        else
            button->woStatus &= ~WOS_SELECTED ;
        button->Information( I_CHANGED_STATUS, NULL ) ;
        break;
    case ADD_ENTRY:
        sprintf(dialogName,"Adding Connection Entry based on: %s",_entry->_name);
        button = (UIW_BUTTON *) Get( CHECK_TEMPLATE ) ;
        button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
		_entry->_access = TCPIP;
        break;
    case CLONE_ENTRY:
        sprintf(dialogName,"Cloning Connection Entry: %s", _entry->_name);
        button = (UIW_BUTTON *) Get( CHECK_TEMPLATE ) ;
        button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        break;
    }
    Information( I_SET_TEXT, dialogName ) ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    strncpy(_phone,entry->_phone_number, 256);
    strncpy(_lat, entry->_lataddress, 256);
    strncpy(_ip, entry->_ipaddress, 256);
    strncpy(_ipport, entry->_ipport, 32);
    strncpy(_sshport, entry->_sshport, 32);
    strncpy(_ftpport, entry->_ftpport, 32);
    _tcpproto = entry->_tcpproto;
    _sshproto = entry->_sshproto;

    // Apply entry to Notebook
    // General Page
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
    string->DataSet( mode == EDIT_ENTRY ? entry->_name : "", 64 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    string->DataSet( entry->_location, 64 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    /* Port combo box */
    button = NULL, button2=NULL ;
    switch ( _entry->_access ) {
    case TCPIP:
        InitTCPProtoList();
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        button = FindTCPButton(entry->_tcpproto);
        *combo + button;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->DataSet( entry->_ipport, 32 ) ;
        string->woFlags |= WOF_AUTO_CLEAR ;
        string->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    case SSH:
        InitSSHProtoList();

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        button = FindSSHButton( entry->_sshproto );
        *combo + button;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->DataSet( entry->_sshport, 32 ) ;
        string->woFlags |= WOF_AUTO_CLEAR ;
        string->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    }

    UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
    switch ( entry->_access ) {
    case PHONE:
        _transport = _entry->_access;
	string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_phone_number, 256 ) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_PHONE_NUMBER ) ;
        prompt->DataSet("Phone number:");
        break;

    case DIRECT:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( "", 0 ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL);

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS,NULL);

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_DIRECT ) ;
        prompt->DataSet("");
        break;

    case TCPIP:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_ipaddress, 256 ) ;

        button = (UIW_BUTTON *) Get( RADIO_IP_ADDRESS ) ;
        prompt->DataSet("Hostname or IP Address:");

        button = (UIW_BUTTON *) Get( RADIO_IP_ADDRESS ) ;
        break;

    case SSH:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_ipaddress, 256 ) ;

        button = (UIW_BUTTON *) Get( RADIO_IP_ADDRESS ) ;
        prompt->DataSet("Hostname or IP Address:");

        button = (UIW_BUTTON *) Get( RADIO_SSH ) ;
        break;

    case FTP:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_ipaddress, 256 ) ;

        button = (UIW_BUTTON *) Get( RADIO_IP_ADDRESS ) ;
        prompt->DataSet("Hostname or IP Address:");

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_FTP ) ;
        break;

    case SUPERLAT:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_lataddress, 256 ) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_LAT_ADDRESS ) ;
        prompt->DataSet("Service name or node/port");
        break;

    case TEMPLATE:
        _transport = TEMPLATE;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet("");
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = NULL ;
        prompt->DataSet("");
    }

    if ( button ) {
	button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        combo = (UIW_COMBO_BOX *) Get( COMBO_CONNECTION_TYPE );
	*combo + button ;
    }

#ifdef WIN32
    if ( !connector->_ssh_avail ) {
        button = (UIW_BUTTON *) Get( RADIO_SSH );
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
#else
    button = (UIW_BUTTON *) Get( RADIO_SSH );
    *combo - button;
#endif 
    
    string = (UIW_STRING *) Get( FIELD_STARTUP_DIR ) ;
    string->DataSet( entry->_startup_dir, 256 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_NOTES ) ;
    text->DataSet( entry->_notes, 3000 ) ;


//    if ( entry->_access == PHONE || entry->_access == DIRECT)
//    {
//	button->woFlags |= WOF_NON_SELECTABLE ;
//	button->Information( I_CHANGED_FLAGS, NULL ) ;
//    }
    
    button = (UIW_BUTTON *) Get( CHECK_AUTO_EXIT );
    if ( entry->_autoexit )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

//    if ( entry->_access == PHONE ||
//	 entry->_access == DIRECT )
//	button->woFlags |= WOF_NON_SELECTABLE ;
//    button->Information(I_CHANGED_FLAGS,NULL);

    // If a clone, get rid of the original
   if ( _mode == CLONE_ENTRY )
      _entry = new KD_LIST_ITEM(*entry);

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_GENERAL_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_GENERAL_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_GENERAL_SETTINGS::K_DIALOG_GENERAL_SETTINGS(entry,mode)\n");
};

K_DIALOG_GENERAL_SETTINGS::~K_DIALOG_GENERAL_SETTINGS(void)
{
    if ( _original_name )
        delete _original_name ;
    _original_name = NULL;
}

EVENT_TYPE K_DIALOG_GENERAL_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_GENERAL_SETTINGS::Event()\n");

    extern K_CONNECTOR * connector ; 
    EVENT_TYPE retval = event.type ; 
    UIW_STRING * string = NULL ;
    UIW_STRING * string2 = NULL;
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
    K_MODEM * modem = NULL;
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
    case OPT_HELP_SET_GENERAL:
        helpSystem->DisplayHelp( windowManager, HELP_SET_GENERAL ) ;
        break;

    case OPT_GENERAL_OK: {
        string = (UIW_STRING *) Get( FIELD_NAME ) ;
        ZIL_ICHAR * newName = string->DataGet() ;

        // Check fields for completion requirements 
        if ( strlen( newName ) == 0 ) {
	   ZAF_MESSAGE_WINDOW * message =
	       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
				       "A Name must be specified" ) ;
	   message->Control() ;
	   delete message ;
	   break;
       }

       // if ( successfully completed ) 
       if ( _original_name && strlen(_original_name) > 0 )
       {
	   if ( !strcmp( _original_name, newName ) )
	   {
               // The names are the same so we are replacing the previous values
               
	   }
	   else 
	   {
               // Different names, must Rename previous instance
               // RenameObject() doesn't work, so we will check the new
               // name and then destroy the original object completely
               if ( K_CONNECTOR::_userFile->FindFirstObject( newName ) )
               {
		   // Name already in use
		   ZAF_MESSAGE_WINDOW * message =
		       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                             "%s already exists", newName ) ;
		   message->Control() ;
		   delete message ;
		   break;
               }
               if ( K_CONNECTOR::_userFile->DestroyObject( _original_name ) )
               {
		   // Rename failed
		   ZAF_MESSAGE_WINDOW * message =
		       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                             "Unable to Destroy %s errno %d",
                                             _original_name, 
                                             K_CONNECTOR::_userFile->storageError) ;
		   message->Control() ;
		   delete message ;
		   break;
               }	
	   }
       }
       else 
       {
	   // This is a new entry, make sure it doesn't duplicate an existing name
	   // before creating it.
	   if ( K_CONNECTOR::_userFile->FindFirstObject( newName ) )
	   {
               // Name already in use
               ZAF_MESSAGE_WINDOW * message =
                  new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "%s already exists", newName ) ;
               message->Control() ;
               delete message ;
               break;
	   }
       }

        ApplyChangesToEntry() ;
        _entry->_userdefined = 1;
        connector->AddEntry( _entry ) ;

        ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, newName,
                                      ID_KD_LIST_ITEM, UIS_OPENCREATE | UIS_READWRITE ) ;
        if ( _fileObj.objectError )
        {
            // What went wrong?
	   ZAF_MESSAGE_WINDOW * message =
               new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                       "Unable to Open %s: errorno %d", newName, _fileObj.objectError ) ;
	   message->Control() ;
	   delete message ;
        }  
        else 
        {
	   _entry->Store( newName, K_CONNECTOR::_userFile, &_fileObj,
			  ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
	   K_CONNECTOR::_userFile->Save() ;
	   if ( K_CONNECTOR::_userFile->storageError )
	   {
               // What went wrong?
               ZAF_MESSAGE_WINDOW * message =
		   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to Open %s: errorno %d", newName, 
                                          K_CONNECTOR::_userFile->storageError ) ;
               message->Control() ;
               delete message ;
	   }

        }

        _cancelled = 0;
        UI_EVENT event(S_CLOSE);
        event.windowObject = this ;
        eventManager->Put(event);
        break;
    }

    case OPT_PHONE_CALL: {
        if ( _transport == PHONE )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Phone number:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT );
        switch ( _transport ) {
        case FTP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ftpport, string2->DataGet(), 32);
            break;

        case SSH:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _sshport, string2->DataGet(), 32);
            _sshproto = GetSSHProto();
            break;

        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ipport, string2->DataGet(), 32);
            _tcpproto = GetTCPProto();
            break;

        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        }
        string->DataSet(_phone,256);
        _transport = PHONE;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    }

    case OPT_TELNET: {
        if ( _transport == TCPIP )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Hostname or IP Address:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT );
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        case SSH:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _sshport, string2->DataGet(), 32);
            _sshproto = GetSSHProto();
            break;
        case FTP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ftpport, string2->DataGet(), 32);
            break;
        }
        string->DataSet(_ip,256);
        string2->DataSet(_ipport,32);
        _transport = TCPIP;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);
        string2->woFlags &= ~WOF_NON_SELECTABLE ;
        string2->Information(I_CHANGED_FLAGS, NULL);

        InitTCPProtoList();
        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        button = FindTCPButton(_tcpproto);
        *combo + button;
       break;
    }

    case OPT_SSH: {
        printf("OPT_SSH\n");
        if ( _transport == SSH ) {
            printf("Already SSH - give up\n");
            break;
        }

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Hostname or IP Address:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT );
        printf("Switch...\n");
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ipport, string2->DataGet(), 32);
            _tcpproto = GetTCPProto();
            break;
        case FTP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ftpport, string2->DataGet(), 32);
            break;
        }
        string->DataSet(_ip,256);
        string2->DataSet(_sshport,32);
        _transport = SSH;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        string2->woFlags &= ~WOF_NON_SELECTABLE ;
        string2->Information(I_CHANGED_FLAGS, NULL);

        printf("InitSSHProtoList...\n");
        InitSSHProtoList();
        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        button = FindSSHButton(_sshproto);
        *combo + button;
        printf("Done!");
        break;
    }

    case OPT_FTP: {
        if ( _transport == FTP )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Hostname or IP Address:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT );
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ipport, string2->DataGet(), 32);
            _tcpproto = GetTCPProto();
            break;
        case SSH:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _sshport, string2->DataGet(), 32);
            _sshproto = GetSSHProto();
            break;
        }
        string->DataSet(_ip,256);
        string2->DataSet(_ftpport,32);
        _transport = FTP;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        string2->woFlags &= ~WOF_NON_SELECTABLE ;
        string2->Information(I_CHANGED_FLAGS, NULL);

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    }

    case OPT_SUPERLAT: {
        if ( _transport == SUPERLAT )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Service name or node/port:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SSH:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _sshport, string2->DataGet(), 32);
            _sshproto = GetSSHProto();
            break;
        case FTP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ftpport, string2->DataGet(), 32);
            break;
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ipport, string2->DataGet(), 32);
            _tcpproto = GetTCPProto();
            break;
        }
        string->DataSet(_lat,256);
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);
        _transport = SUPERLAT;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
       break;
    }

    case OPT_DIRECT: {
        if ( _transport == DIRECT )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string2 = (UIW_STRING *) Get( ENTRY_IPPORT );
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        case SSH:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _sshport, string2->DataGet(), 32);
            _sshproto = GetSSHProto();
            break;
        case FTP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ftpport, string2->DataGet(), 32);
            break;
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            strncpy( _ipport, string2->DataGet(), 32);
            _tcpproto = GetTCPProto();
            break;
        }
        string->DataSet("",256);
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL);
        _transport = DIRECT;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    }

    case -11:
        switch ( _mode ) {
        case ADD_ENTRY:
        case CLONE_ENTRY:
            delete _entry;
        }
        // fallthrough to default

    default:
        printf("Fallthrough\n");
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_GENERAL_SETTINGS::
ApplyChangesToEntry( void )
{
    UIW_STRING * string, * string2;

    if ( _entry == NULL )
        _entry = new KD_LIST_ITEM ;

    string = (UIW_STRING *) Get( FIELD_NAME ) ;
    if ( !_original_name || strcmp( _original_name, "DEFAULT" ) )
        strncpy(_entry->_name, string->DataGet(), 64 ) ;
    else 
    {
        if ( strcmp( string->DataGet(), "DEFAULT" ) )
        {
            ZAF_MESSAGE_WINDOW * message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                        "Unable to rename DEFAULT template/entry." ) ;
            message->Control();
            delete message ;
        }
        strncpy(_entry->_name, _original_name, 64 ) ;
    }

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    strncpy( _entry->_location, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
    string2 = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
    switch ( _transport ) {     
    case PHONE:
        strncpy( _entry->_phone_number, string->DataGet(), 256 ) ;
        break;
    case TCPIP:
        strncpy( _entry->_ipaddress, string->DataGet(), 256 ) ;
        strncpy( _entry->_ipport, string2->DataGet(), 32 ) ;
        _entry->_tcpproto = GetTCPProto();
        break;
    case SSH:
        strncpy( _entry->_ipaddress, string->DataGet(), 256 ) ;
        strncpy( _entry->_sshport, string2->DataGet(), 32 ) ;
        _entry->_sshproto = GetSSHProto();
        break;
    case FTP:
        strncpy( _entry->_ipaddress, string->DataGet(), 256 ) ;
        strncpy( _entry->_ftpport, string2->DataGet(), 32 ) ;
        break;
    case SUPERLAT:
        strncpy( _entry->_lataddress, string->DataGet(), 256 ) ;
        break;
    }

    _entry->_access = _transport;

    string = (UIW_STRING *) Get( FIELD_STARTUP_DIR ) ;
    strncpy( _entry->_startup_dir, string->DataGet(), 256 ) ;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_NOTES ) ;
    strncpy( _entry->_notes, text->DataGet(), 3000 ) ;

   // The DEFAULT entry must be a template; we really should produce a Message Box
   // if this is not the case but I am really too lazy at this late date 9-27-95.
    if ( !strcmp( _entry->_name, "DEFAULT" ) )
        _entry->_template = 1 ;
    else 
        _entry->_template = FlagSet(Get( CHECK_TEMPLATE )->woStatus, WOS_SELECTED) ;

    _entry->_autoexit = FlagSet(Get( CHECK_AUTO_EXIT )->woStatus, WOS_SELECTED ) ;

}

void K_DIALOG_GENERAL_SETTINGS::
InitTCPProtoList()
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL ) ;
    while ( (button = (UIW_BUTTON *)list->First()) != NULL ) {
        *list - button;
        delete button;
    }

    button = new UIW_BUTTON( 0,0,0,"default",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"telnet (with negotiations)",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"rlogin",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"raw socket",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"telnet (without negotiations)",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"ek4login",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_krb4_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"ek5login",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_krb5_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"k4login",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_krb4_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"k5login",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_krb5_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"ssl",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_libeay_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"telnet over ssl",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_libeay_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"telnet over tls",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_libeay_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"tls",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    if ( !connector->_libeay_avail ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    *list + button;
}

void K_DIALOG_GENERAL_SETTINGS::
InitSSHProtoList()
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL ) ;
    while ( (button = (UIW_BUTTON *)list->First()) != NULL ) {
        *list - button;
        delete button;
    }

    button = new UIW_BUTTON( 0,0,0,"auto",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"v2",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    /*button = new UIW_BUTTON( 0,0,0,"v1",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/
}

UIW_BUTTON * K_DIALOG_GENERAL_SETTINGS::
FindTCPButton( enum TCPPROTO foo )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    UIW_BUTTON * button = NULL;

    switch ( foo ) {
    case TCP_DEFAULT:
        button = (UIW_BUTTON *) list->Get( "default" );
        break;
    case TCP_EK4LOGIN:
        button = (UIW_BUTTON *) list->Get( "ek4login" );
        break;
    case TCP_EK5LOGIN:
        button = (UIW_BUTTON *) list->Get( "ek5login" );
        break;
    case TCP_K4LOGIN:
        button = (UIW_BUTTON *) list->Get( "k4login" );
        break;
    case TCP_K5LOGIN:
        button = (UIW_BUTTON *) list->Get( "k5login" );
        break;
    case TCP_RAW:
        button = (UIW_BUTTON *) list->Get( "raw socket" );
        break;
    case TCP_RLOGIN:
        button = (UIW_BUTTON *) list->Get( "rlogin" );
        break;
    case TCP_TELNET:
        button = (UIW_BUTTON *) list->Get( "telnet (with negotiations)" );
        break;
    case TCP_TELNET_NO_INIT:
        button = (UIW_BUTTON *) list->Get( "telnet (without negotiations)" );
        break;
    case TCP_TELNET_SSL:
        button = (UIW_BUTTON *) list->Get( "telnet over ssl" );
        break;
    case TCP_TELNET_TLS:
        button = (UIW_BUTTON *) list->Get( "telnet over tls" );
        break;
    case TCP_SSL:
        button = (UIW_BUTTON *) list->Get( "ssl" );
        break;
    case TCP_TLS:
        button = (UIW_BUTTON *) list->Get( "tls" );
        break;
    }
    return button;
}

UIW_BUTTON * K_DIALOG_GENERAL_SETTINGS::
FindSSHButton( enum SSHPROTO foo )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    UIW_BUTTON * button = NULL;

    switch ( foo ) {
    case SSH_AUTO:
        button = (UIW_BUTTON *) list->Get( "auto" );
        break;
    case SSH_V1:
        button = (UIW_BUTTON *) list->Get( "v1" );
        break;
    case SSH_V2:
        button = (UIW_BUTTON *) list->Get( "v2" );
        break;
    }
    return button;
}

enum TCPPROTO K_DIALOG_GENERAL_SETTINGS::
GetTCPProto(void)
{       
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    UIW_BUTTON  * button = (UIW_BUTTON *) list->Current();
    ZIL_ICHAR * name = button ? button->DataGet() : "";

    if ( !strcmp("default",name) )
        return TCP_DEFAULT ;
    else if ( !strcmp("ek4login",name) )
        return TCP_EK4LOGIN ;
    else if ( !strcmp("ek5login",name) )
        return TCP_EK5LOGIN;
    else if ( !strcmp("k4login",name) )
        return TCP_K4LOGIN;
    else if ( !strcmp("k5login",name) )
        return TCP_K5LOGIN;
    else if ( !strcmp("raw socket",name) )
        return TCP_RAW;
    else if ( !strcmp("rlogin",name) )
        return TCP_RLOGIN;
    else if ( !strcmp("telnet (with negotiations)",name) )
        return TCP_TELNET;
    else if ( !strcmp("telnet (without negotiations)",name) )
        return TCP_TELNET_NO_INIT;
    else if ( !strcmp("telnet over ssl",name) )
        return TCP_TELNET_SSL;
    else if ( !strcmp("telnet over tls",name) )
        return TCP_TELNET_TLS;
    else if ( !strcmp("ssl",name) )
        return TCP_SSL;
    else if ( !strcmp("tls",name) )
        return TCP_TLS;
    else
        return TCP_TELNET;
}


enum SSHPROTO K_DIALOG_GENERAL_SETTINGS::
GetSSHProto(void)
{       
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    UIW_BUTTON  * button = (UIW_BUTTON *) list->Current();
    ZIL_ICHAR * name = button ? button->DataGet() : "";

    if ( !strcmp("auto",name) )
        return SSH_AUTO ;
    else if ( !strcmp("v1",name) )
        return SSH_V1 ;
    else if ( !strcmp("v2",name) )
        return SSH_V2;
    else
        return SSH_AUTO;
}

