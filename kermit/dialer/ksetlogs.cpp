#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_LOGS_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetlogs.hpp"
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

ZIL_ICHAR K_DIALOG_LOGS_SETTINGS::_className[] = "K_DIALOG_LOGS_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_LOGS_SETTINGS
//
//

K_DIALOG_LOGS_SETTINGS::
K_DIALOG_LOGS_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_LOGS_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    /* Logs page */
    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_LOG_DEBUG ) ;
    string->DataSet( "debug.log" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
   
    button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_LOG_SESSION ) ;
   string->DataSet( "session.log" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;
   
    button = (UIW_BUTTON *) Get( CHECK_LOG_PACKETS ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_LOG_PACKETS ) ;
   string->DataSet( "packet.log" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;
   
    button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_LOG_TRANSACTIONS ) ;
   string->DataSet( "transact.log" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
   string->DataSet( "connection.log" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;
   
    button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION_APPEND ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG_APPEND ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION_APPEND ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_PACKETS_APPEND ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_APPEND ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_BRIEF ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_LOGS_SETTINGS
//
//

K_DIALOG_LOGS_SETTINGS::
K_DIALOG_LOGS_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_LOGS_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_LOGS_SETTINGS::K_DIALOG_LOGS_SETTINGS(entry,mode)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Log File Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    /* Logs page */
    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION ) ;
    if ( entry->_log_connection ) 
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION_APPEND ) ;
    if ( entry->_log_connection_append ) 
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
    string->DataSet( entry->_log_connection_fname ) ;
    if ( !entry->_log_connection )
    {
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else
    {
	string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
    }

    button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG ) ;
    if ( entry->_log_debug ) 
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG_APPEND ) ;
    if ( entry->_log_debug_append ) 
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_DEBUG ) ;
    string->DataSet( entry->_log_debug_fname ) ;
    if ( !entry->_log_debug )
    {
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else
    {
	string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
    }

    button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION ) ;
    if ( entry->_log_session )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION_APPEND ) ;
    if ( entry->_log_session_append )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_SESSION ) ;
    string->DataSet( entry->_log_session_fname ) ;
    if ( !entry->_log_session )
    {
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else
    {
	string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;

        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
    }

    switch ( entry->_log_session_type ) {
    case LOG_TEXT:
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information( I_CHANGED_STATUS, NULL ) ;
        break;
    case LOG_BINARY:
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information( I_CHANGED_STATUS, NULL ) ;
        break;
    case LOG_DEBUG:
        button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information( I_CHANGED_STATUS, NULL ) ;
        break;
    }

    button = (UIW_BUTTON *) Get( CHECK_LOG_PACKETS ) ;
    if ( entry->_log_packet )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_PACKETS_APPEND ) ;
    if ( entry->_log_packet_append )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_PACKETS ) ;
    string->DataSet( entry->_log_packet_fname ) ;
    if ( !entry->_log_packet )
    {
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else
    {
	string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
    }

    button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS ) ;
    if ( entry->_log_transaction )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_APPEND ) ;
    if ( entry->_log_transaction_append )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    UIW_BUTTON * button2 = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_BRIEF ) ;
    if ( entry->_log_transaction_brief )
	button2->woStatus |= WOS_SELECTED ;
    else
	button2->woStatus &= ~WOS_SELECTED ;
    button2->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_TRANSACTIONS ) ;
    string->DataSet( entry->_log_transaction_fname ) ;
    if ( !entry->_log_transaction )
    {
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS,NULL) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else
    {
	string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL ) ;
	button2->woFlags &= ~WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL ) ;
    }

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_LOGS_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_LOGS_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_LOGS_SETTINGS::K_DIALOG_LOGS_SETTINGS(entry)\n");
};

K_DIALOG_LOGS_SETTINGS::~K_DIALOG_LOGS_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_LOGS_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_LOGS_SETTINGS::Event()\n");

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
    case OPT_HELP_SET_LOGS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_LOGS ) ;
        break;

    case OPT_LOGS_OK: {
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
    case OPT_CHECK_LOG_DEBUG:
	string = (UIW_STRING *) Get( FIELD_LOG_DEBUG ) ;
	button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG_APPEND );
	if ( FlagSet(Get( CHECK_LOG_DEBUG )->woStatus, WOS_SELECTED ) )
	{
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	}	
	else 	
	{	
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    button->woFlags |= WOF_NON_SELECTABLE ;
	}	
	string->Information(I_CHANGED_FLAGS, NULL);
	button->Information(I_CHANGED_FLAGS, NULL);
	break;

    case OPT_CHECK_LOG_CONNECTION:
	string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
	button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION_APPEND );
	if ( FlagSet(Get( CHECK_LOG_CONNECTION )->woStatus, WOS_SELECTED ) )
	{
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	}	
	else 	
	{	
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    button->woFlags |= WOF_NON_SELECTABLE ;
	}	
	string->Information(I_CHANGED_FLAGS, NULL);
	button->Information(I_CHANGED_FLAGS, NULL);
	break;

    case OPT_CHECK_LOG_PACKET:
	string = (UIW_STRING *) Get( FIELD_LOG_PACKETS ) ;
	button = (UIW_BUTTON *) Get( CHECK_LOG_PACKETS_APPEND );
	if ( FlagSet(Get( CHECK_LOG_PACKETS )->woStatus, WOS_SELECTED ) )
	{
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	}	
	else 	
	{	
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    button->woFlags |= WOF_NON_SELECTABLE ;
	}	
	string->Information(I_CHANGED_FLAGS, NULL);
	button->Information(I_CHANGED_FLAGS, NULL);
	break;

    case OPT_CHECK_LOG_SESSION: {

        UIW_BUTTON * selected_button = NULL;

        if ( FlagSet(Get( CHECK_LOG_SESSION )->woStatus, WOS_SELECTED ) )
	{
            string = (UIW_STRING *) Get( FIELD_LOG_SESSION ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS, NULL);
            button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION_APPEND );
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT );
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY );
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG );
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            if (FlagSet(Get( RADIO_LOG_SESSION_TEXT )->woStatus, WOS_SELECTED ))
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT );
            else if (FlagSet(Get( RADIO_LOG_SESSION_DEBUG )->woStatus, WOS_SELECTED ))
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG );
            else if (FlagSet(Get( RADIO_LOG_SESSION_BINARY )->woStatus, WOS_SELECTED )) 
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY );

            if ( selected_button ) {
                selected_button->woStatus |= WOS_SELECTED ;
                selected_button->Information(I_CHANGED_STATUS,NULL);
            }
	}	
	else 	
	{	
            if (FlagSet(Get( RADIO_LOG_SESSION_TEXT )->woStatus, WOS_SELECTED ))
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT );
            else if (FlagSet(Get( RADIO_LOG_SESSION_DEBUG )->woStatus, WOS_SELECTED ))
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG );
            else if (FlagSet(Get( RADIO_LOG_SESSION_BINARY )->woStatus, WOS_SELECTED )) 
                selected_button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY );

            string = (UIW_STRING *) Get( FIELD_LOG_SESSION ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS, NULL);
            button = (UIW_BUTTON *) Get( CHECK_LOG_SESSION_APPEND );
	    button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_TEXT );
	    button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);
            button->woStatus &= ~WOS_SELECTED ;
            button->Information(I_CHANGED_STATUS,NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_BINARY );
	    button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);
            button->woStatus &= ~WOS_SELECTED ;
            button->Information(I_CHANGED_STATUS,NULL);

            button = (UIW_BUTTON *) Get( RADIO_LOG_SESSION_DEBUG );
	    button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);
            button->woStatus &= ~WOS_SELECTED ;
            button->Information(I_CHANGED_STATUS,NULL);

            if ( selected_button ) {
                // Due to a weird behavior when grouped radio buttons
                // are made unselectable, those that are still selectable
                // become selected.  So restore the correct value.
                selected_button->woFlags &= ~WOF_NON_SELECTABLE ;
                selected_button->Information(I_CHANGED_FLAGS, NULL);
                selected_button->woStatus |= WOS_SELECTED ;
                selected_button->Information(I_CHANGED_STATUS,NULL);
                selected_button->woFlags |= WOF_NON_SELECTABLE ;
                selected_button->Information(I_CHANGED_FLAGS, NULL);
            }
	}	

	break;
    }

    case OPT_CHECK_LOG_TRANSACTION:
	string = (UIW_STRING *) Get( FIELD_LOG_TRANSACTIONS ) ;
	button = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_APPEND );
	button2 = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_BRIEF );
	if ( FlagSet(Get( CHECK_LOG_TRANSACTIONS )->woStatus, WOS_SELECTED ) )
	{
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	    button2->woFlags &= ~WOF_NON_SELECTABLE ;
	}	
	else 	
	{	
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    button->woFlags |= WOF_NON_SELECTABLE ;
	    button2->woFlags |= WOF_NON_SELECTABLE ;
	}	
	string->Information(I_CHANGED_FLAGS, NULL);
	button->Information(I_CHANGED_FLAGS, NULL);
	button2->Information(I_CHANGED_FLAGS, NULL);
	break;

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_LOGS_SETTINGS::
ApplyChangesToEntry( void )
{
    /* Logs page */
    _entry->_log_connection = FlagSet( Get( CHECK_LOG_CONNECTION )->woStatus, WOS_SELECTED ) ;
    _entry->_log_connection_append = 
	FlagSet( Get( CHECK_LOG_CONNECTION_APPEND )->woStatus, WOS_SELECTED ) ;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
    strncpy( _entry->_log_connection_fname, string->DataGet(), 255 );

    _entry->_log_debug = FlagSet( Get( CHECK_LOG_DEBUG )->woStatus, WOS_SELECTED ) ;
    _entry->_log_debug_append = 
	FlagSet( Get( CHECK_LOG_DEBUG_APPEND )->woStatus, WOS_SELECTED ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_DEBUG ) ;
    strncpy( _entry->_log_debug_fname, string->DataGet(), 255 );

    _entry->_log_session = FlagSet( Get( CHECK_LOG_SESSION )->woStatus, WOS_SELECTED ) ;
    _entry->_log_session_append = 
	FlagSet( Get( CHECK_LOG_SESSION_APPEND )->woStatus, WOS_SELECTED ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_SESSION ) ;
    strncpy( _entry->_log_session_fname, string->DataGet(), 255 ) ;

    if ( FlagSet( Get( RADIO_LOG_SESSION_DEBUG )->woStatus, WOS_SELECTED ) )
        _entry->_log_session_type = LOG_DEBUG;
    if ( FlagSet( Get( RADIO_LOG_SESSION_BINARY )->woStatus, WOS_SELECTED ) )
        _entry->_log_session_type = LOG_BINARY;
    if ( FlagSet( Get( RADIO_LOG_SESSION_TEXT )->woStatus, WOS_SELECTED ) )
        _entry->_log_session_type = LOG_TEXT;

    _entry->_log_packet = FlagSet( Get( CHECK_LOG_PACKETS )->woStatus, WOS_SELECTED ) ;
    _entry->_log_packet_append = 
	FlagSet( Get( CHECK_LOG_PACKETS_APPEND )->woStatus, WOS_SELECTED ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_PACKETS ) ;
    strncpy( _entry->_log_packet_fname, string->DataGet(), 255 ) ;

    _entry->_log_transaction = FlagSet( Get( CHECK_LOG_TRANSACTIONS )->woStatus, WOS_SELECTED ) ;
    _entry->_log_transaction_append = 
	FlagSet( Get( CHECK_LOG_TRANSACTIONS_APPEND )->woStatus, WOS_SELECTED ) ;
    _entry->_log_transaction_brief = 
	FlagSet( Get( CHECK_LOG_TRANSACTIONS_BRIEF )->woStatus, WOS_SELECTED ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_TRANSACTIONS ) ;
    strncpy( _entry->_log_transaction_fname, string->DataGet(), 255 ) ;

}

