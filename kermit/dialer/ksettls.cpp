#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_TLS_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksettls.hpp"
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

ZIL_ICHAR K_DIALOG_TLS_SETTINGS::_className[] = "K_DIALOG_TLS_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_TLS_SETTINGS
//
//

K_DIALOG_TLS_SETTINGS::
K_DIALOG_TLS_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_TLS_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // SSL/TLS Page
    UIW_STRING * string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
    string->DataSet( "" ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
    combo->DataSet("Verify host certificates if presented");
    combo->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_TLS_SETTINGS
//
//

K_DIALOG_TLS_SETTINGS::
K_DIALOG_TLS_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_TLS_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_TLS_SETTINGS::K_DIALOG_TLS_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"TLS Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // SSL/TLS Page
    UIW_STRING * string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
    string->DataSet(entry->_tls_cipher) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
    string->DataSet(entry->_tls_cert_file) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
    string->DataSet(entry->_tls_key_file) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
    string->DataSet(entry->_tls_verify_file) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
    string->DataSet(entry->_tls_verify_dir) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
    string->DataSet(entry->_tls_crl_file) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
    string->DataSet(entry->_tls_crl_dir) ;
    string->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
    switch ( entry->_tls_verify_mode ) {
    case TLS_VERIFY_NO:
        combo->DataSet("Do not verify host certificates");
        break;
    case TLS_VERIFY_PEER:
        combo->DataSet("Verify host certificates if presented");
        break;
    case TLS_VERIFY_FAIL:
        combo->DataSet("Peer certificates must be presented and verified; if not, fail");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_tls_verbose )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
    button->woFlags |= ((entry->_access != TCPIP && entry->_access != FTP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_tls_debug )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    if (entry->_access != TCPIP && entry->_access != FTP) {
        button = (UIW_BUTTON *) Get(BUTTON_TLS_BROWSE_CERT);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
        button = (UIW_BUTTON *) Get(BUTTON_TLS_BROWSE_KEY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
        button = (UIW_BUTTON *) Get(BUTTON_BROWSE_TLS_CA);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
        button = (UIW_BUTTON *) Get(BUTTON_TLS_BROWSE_CRL);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
    }

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_TLS_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_TLS_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_TLS_SETTINGS::K_DIALOG_TLS_SETTINGS(entry,mode)\n");
};

K_DIALOG_TLS_SETTINGS::~K_DIALOG_TLS_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_TLS_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_TLS_SETTINGS::Event()\n");

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
    case OPT_HELP_SET_TLS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TLS ) ;
        break;

    case OPT_TLS_OK: {
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

    case OPT_BROWSE_TLS_CERT:
    case OPT_BROWSE_TLS_KEY:
    case OPT_BROWSE_TLS_CA:
    case OPT_BROWSE_TLS_CRL:
    {
        EVENT_TYPE OpenFile = OPT_FILE_OPEN ; 
        DirServRequestor = event.type ;
        directoryService->Information(I_SET_REQUESTOR,this);
        directoryService->Information(I_SET_REQUEST,&OpenFile);

        // I'm not sure why the value is 11003
        UI_EVENT  Reset_Filter(11003);   
        directoryService->Event(Reset_Filter);
        directoryService->Information(I_SET_FILTER,"*");
        directoryService->Information(I_ACTIVATE_SERVICE,ZIL_NULLP(void));
        break;
    }

    case -OPT_FILE_OPEN: {
        const ZIL_ICHAR * p = event.text;
        ZIL_ICHAR tmp[512], * q;
        if ( p ) {
            for ( q = tmp ; *p; p++, q++) {
                if ( *p == '\\' )
                    *q = '/';
                else
                    *q = *p;
            }
        }
        switch ( DirServRequestor ) {
        case OPT_BROWSE_TLS_CERT:
            string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        case OPT_BROWSE_TLS_KEY:
            string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        case OPT_BROWSE_TLS_CA:
            string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        case OPT_BROWSE_TLS_CRL:
            string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        }
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

void K_DIALOG_TLS_SETTINGS::
ApplyChangesToEntry( void )
{
    // SSL/TLS Page
    UIW_STRING * string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
    strncpy( _entry->_tls_cipher, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
    strncpy( _entry->_tls_cert_file, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
    strncpy( _entry->_tls_key_file, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
    strncpy( _entry->_tls_verify_file, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
    strncpy( _entry->_tls_verify_dir, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
    strncpy( _entry->_tls_crl_file, string->DataGet(), 256 ) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
    strncpy( _entry->_tls_crl_dir, string->DataGet(), 256 ) ;

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
    ZIL_ICHAR * ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Do not verify host certificates") )
	_entry->_tls_verify_mode = TLS_VERIFY_NO;
    else if ( !strcmp(ichar,"Verify host certificates if presented") )
	_entry->_tls_verify_mode = TLS_VERIFY_PEER;
    else if ( !strcmp(ichar,"Peer certificates must be presented and verified; if not, fail") )
	_entry->_tls_verify_mode = TLS_VERIFY_FAIL;

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
    _entry->_tls_verbose = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
    _entry->_tls_debug = button->woStatus & WOS_SELECTED;
}

