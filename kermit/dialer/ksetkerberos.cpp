#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_KERBEROS_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetkerberos.hpp"
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

ZIL_ICHAR K_DIALOG_KERBEROS_SETTINGS::_className[] = "K_DIALOG_KERBEROS_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_KERBEROS_SETTINGS
//
//

K_DIALOG_KERBEROS_SETTINGS::
K_DIALOG_KERBEROS_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_KERBEROS_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // Kerberos Page
    UIW_STRING * string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
    string->DataSet( "", 256 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    UIW_BUTTON * button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    UIW_INTEGER * integer = NULL ;
    integer = (UIW_INTEGER *) Get( INTEGER_K4_LIFETIME);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    int value = 600 ;
    integer->Information( I_SET_VALUE, &value ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
    string->DataSet( "", 256 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
    string->DataSet( "", 256 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    integer = (UIW_INTEGER *) Get( INTEGER_K5_LIFETIME);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = 600 ;
    integer->Information( I_SET_VALUE, &value ) ;

    button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_KERBEROS_SETTINGS
//
//

K_DIALOG_KERBEROS_SETTINGS::
K_DIALOG_KERBEROS_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_KERBEROS_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_KERBEROS_SETTINGS::K_DIALOG_KERBEROS_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Kerberos Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Kerberos
    UIW_STRING * string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
    string->DataSet( entry->_k4_realm, 256 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
    string->DataSet( entry->_k4_princ, 64 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    UIW_BUTTON * button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
    if ( entry->_k4_autoget ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
    if ( entry->_k4_autodestroy ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    UIW_INTEGER * integer = NULL ;
    integer = (UIW_INTEGER *) Get( INTEGER_K4_LIFETIME);
    integer->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    int value = entry->_k4_lifetime;
    integer->Information( I_SET_VALUE, &value ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
    string->DataSet( entry->_k5_realm, 256 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
    string->DataSet( entry->_k5_princ, 64 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
    if ( entry->_k5_autoget ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
    if ( entry->_k5_autodestroy ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    integer = (UIW_INTEGER *) Get( INTEGER_K5_LIFETIME);
    integer->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = entry->_k5_lifetime;
    integer->Information( I_SET_VALUE, &value ) ;

    button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
    if ( entry->_k5_forwardable ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
    if ( entry->_k5_getk4tkt ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
    string->DataSet( entry->_k5_cache, 256 );
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != SSH) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );


    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_KERBEROS_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_KERBEROS_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_KERBEROS_SETTINGS::K_DIALOG_KERBEROS_SETTINGS(entry)\n");
};

K_DIALOG_KERBEROS_SETTINGS::~K_DIALOG_KERBEROS_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_KERBEROS_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_KERBEROS_SETTINGS::Event()\n");

    extern K_CONNECTOR * connector ; 
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
    case OPT_HELP_SET_KERBEROS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_KERBEROS ) ;
        break;

    case OPT_KERBEROS_OK: {
        // Check fields for completion requirements 
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

void K_DIALOG_KERBEROS_SETTINGS::
ApplyChangesToEntry( void )
{
    // Kerberos
    UIW_STRING *string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM ) ;
    strncpy( _entry->_k4_realm, string->DataGet(), 256 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC ) ;
    strncpy( _entry->_k4_princ, string->DataGet(), 64 ) ;

    UIW_BUTTON * button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
    _entry->_k4_autoget = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
    _entry->_k4_autodestroy = button->woStatus & WOS_SELECTED;

    UIW_INTEGER * spin_int = (UIW_INTEGER *) Get( INTEGER_K4_LIFETIME ) ;
    int new_spin_int;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_k4_lifetime = new_spin_int;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM ) ;
    strncpy( _entry->_k5_realm, string->DataGet(), 256 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC ) ;
    strncpy( _entry->_k5_princ, string->DataGet(), 64 ) ;

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
    _entry->_k5_autoget = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
    _entry->_k5_autodestroy = button->woStatus & WOS_SELECTED;

    spin_int = (UIW_INTEGER *) Get( INTEGER_K5_LIFETIME ) ;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_k5_lifetime = new_spin_int;

    button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
    _entry->_k5_forwardable = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
    _entry->_k5_getk4tkt = button->woStatus & WOS_SELECTED;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE ) ;
    strncpy( _entry->_k5_cache, string->DataGet(), 256 ) ;
}

