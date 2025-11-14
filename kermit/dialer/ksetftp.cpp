#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_FTP_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetftp.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"

ZIL_ICHAR K_DIALOG_FTP_SETTINGS::_className[] = "K_DIALOG_FTP_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_FTP_SETTINGS
//
//

K_DIALOG_FTP_SETTINGS::
K_DIALOG_FTP_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_FTP_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // This constructor will never be called because this object
    // is only used to edit existing objects.

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_FTP_SETTINGS
//
//

K_DIALOG_FTP_SETTINGS::
K_DIALOG_FTP_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode )
   : K95_SETTINGS_DIALOG("DIALOG_FTP_SETTINGS", mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_FTP_SETTINGS::K_DIALOG_FTP_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"FTP Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Fill in details here for configuring the dialog controls
    // based upon the entry settings

    InitFTPProtectionLevel(entry,(UIW_VT_LIST *)Get(LIST_FTP_CPL));
    InitFTPProtectionLevel(entry,(UIW_VT_LIST *)Get(LIST_FTP_DPL));
    InitFTPAuthList(entry);
    InitFTPCharsetList(entry);

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOLOGIN ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_autologin )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_PASSIVE ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_passive )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOAUTH ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_autoauth )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOENC ) ;
    button->woFlags = ((entry->_access != FTP || !entry->_ftp_autoauth) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_autoenc )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_CREDFWD ) ;
    button->woFlags = ((entry->_access != FTP || !entry->_ftp_autoauth) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_credfwd )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_DEBUG ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_debug )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_DATES ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_dates )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_LITERAL ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_literal )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_VERBOSE ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_verbose )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_FTP_XLAT ) ;
    button->woFlags = (entry->_access != FTP ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ftp_xlat )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CPL ) ;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get(LIST_FTP_CPL);
    switch ( entry->_ftp_cpl ) {
    case PL_CLEAR:
        button = (UIW_BUTTON *) list->Get("clear") ;
        break;
    case PL_CONFIDENTIAL:
        button = (UIW_BUTTON *) list->Get("confidential") ;
        break;
    case PL_PRIVATE:
        button = (UIW_BUTTON *) list->Get("private") ;
        break;
    case PL_SAFE:
        button = (UIW_BUTTON *) list->Get("safe") ;
        break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ; 
    if ( entry->_access == FTP && entry->_ftp_autoauth )
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_DPL ) ;
    list = (UIW_VT_LIST *) Get(LIST_FTP_DPL);
    switch ( entry->_ftp_dpl ) {
    case PL_CLEAR:
        button = (UIW_BUTTON *) list->Get("clear") ;
        break;
    case PL_CONFIDENTIAL:
        button = (UIW_BUTTON *) list->Get("confidential") ;
        break;
    case PL_PRIVATE:
        button = (UIW_BUTTON *) list->Get("private") ;
        break;
    case PL_SAFE:
        button = (UIW_BUTTON *) list->Get("safe") ;
        break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ; 
    if ( entry->_access == FTP && entry->_ftp_autoauth )
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CHARSET ) ;
    SetComboToCharset( combo, entry->_ftp_charset ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_access != FTP || !entry->_ftp_xlat)
        combo->woFlags |= WOF_NON_SELECTABLE ;
    else
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL);

    list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("gssapi-kerberos5");
    if ( entry->_ftp_auth_gssk5 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("kerberos4");
    if ( entry->_ftp_auth_k4 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("srp");
    if ( entry->_ftp_auth_srp ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("ssl");
    if ( entry->_ftp_auth_ssl ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("tls");
    if ( entry->_ftp_auth_tls ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    if ( entry->_access != FTP || !entry->_ftp_autoauth)
        list->woFlags |= WOF_NON_SELECTABLE ;
    else
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    list->Information(I_CHANGED_FLAGS,NULL);

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_FTP_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_FTP_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_FTP_SETTINGS::K_DIALOG_FTP_SETTINGS(entry,mode)\n");
};

K_DIALOG_FTP_SETTINGS::~K_DIALOG_FTP_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_FTP_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_FTP_SETTINGS::Event()\n");

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
    case OPT_HELP_SET_FTP:
        helpSystem->DisplayHelp( windowManager, HELP_SET_FTP ) ;
        break;

    case OPT_FTP_OK: {
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

    case OPT_CHECK_FTP_AUTH:
        if (!FlagSet(Get(CHECK_FTP_AUTOAUTH)->woStatus, WOS_SELECTED )) {
            button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOENC ) ;
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS,NULL);

            button = (UIW_BUTTON *) Get( CHECK_FTP_CREDFWD ) ;
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS,NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CPL ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_DPL ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL);

            list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
            list->woFlags |= WOF_NON_SELECTABLE ;
            list->Information(I_CHANGED_FLAGS,NULL);
        } else {
            button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOENC ) ;
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS,NULL);

            button = (UIW_BUTTON *) Get( CHECK_FTP_CREDFWD ) ;
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS,NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CPL ) ;
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_DPL ) ;
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL);

            list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
            list->woFlags &= ~WOF_NON_SELECTABLE ;
            list->Information(I_CHANGED_FLAGS,NULL);
        }
        break;

    case OPT_CHECK_FTP_XLAT:
        combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CHARSET ) ;
        if (!FlagSet(Get( CHECK_FTP_XLAT)->woStatus, WOS_SELECTED ))
            combo->woFlags |= WOF_NON_SELECTABLE ;
        else
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
        break;

    default:
       retval = UIW_WINDOW::Event(event);
   }	
   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_FTP_SETTINGS::
ApplyChangesToEntry( void )
{
    // Copy new values back to the original list item.

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOLOGIN ) ;
    _entry->_ftp_autologin = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_PASSIVE ) ;
    _entry->_ftp_passive = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOAUTH ) ;
    _entry->_ftp_autoauth = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_AUTOENC ) ;
    _entry->_ftp_autoenc = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_CREDFWD ) ;
    _entry->_ftp_credfwd = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_DEBUG ) ;
    _entry->_ftp_debug = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_VERBOSE ) ;
    _entry->_ftp_verbose = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_DATES ) ;
    _entry->_ftp_dates = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_LITERAL ) ;
    _entry->_ftp_literal = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_FTP_XLAT ) ;
    _entry->_ftp_xlat = button->woStatus & WOS_SELECTED;

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_CPL ) ;
    button = (UIW_BUTTON *) combo->Current();
    if ( !strcmp(button->StringID(), "clear") )
        _entry->_ftp_cpl = PL_CLEAR;
    else if ( !strcmp(button->StringID(), "confidential") )
        _entry->_ftp_cpl = PL_CONFIDENTIAL;
    else if ( !strcmp(button->StringID(), "private") )
        _entry->_ftp_cpl = PL_PRIVATE;
    else if ( !strcmp(button->StringID(), "safe") )
        _entry->_ftp_cpl = PL_SAFE;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FTP_DPL ) ;
    button = (UIW_BUTTON *) combo->Current();
    if ( !strcmp(button->StringID(), "clear") )
        _entry->_ftp_dpl = PL_CLEAR;
    else if ( !strcmp(button->StringID(), "confidential") )
        _entry->_ftp_dpl = PL_CONFIDENTIAL;
    else if ( !strcmp(button->StringID(), "private") )
        _entry->_ftp_dpl = PL_PRIVATE;
    else if ( !strcmp(button->StringID(), "safe") )
        _entry->_ftp_dpl = PL_SAFE;

    _entry->_ftp_charset = GetFileCharsetFromCombo( (UIW_COMBO_BOX *) Get( COMBO_FTP_CHARSET ) ) ;

    UIW_VT_LIST * list = (UIW_VT_LIST *) Get(LIST_FTP_AUTHTYPE);
    button = (UIW_BUTTON *) list->Get("gssapi-kerberos5");
    _entry->_ftp_auth_gssk5 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("kerberos4");
    _entry->_ftp_auth_k4 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("srp");
    _entry->_ftp_auth_srp = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ssl");
    _entry->_ftp_auth_ssl = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("tls");
    _entry->_ftp_auth_tls = button->woStatus & WOS_SELECTED;
}

void K_DIALOG_FTP_SETTINGS::
InitFTPProtectionLevel( KD_LIST_ITEM * entry, UIW_VT_LIST * list)
{
    UIW_BUTTON * button;

    button = new UIW_BUTTON( 0,0,0,"clear",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"confidential",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"private",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"safe",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    if ( entry->_access == FTP )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

void K_DIALOG_FTP_SETTINGS::
InitFTPAuthList( KD_LIST_ITEM * entry )
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_FTP_AUTHTYPE ) ;
    button = new UIW_BUTTON( 0,0,0,"gssapi-kerberos5",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"kerberos4",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"srp",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"ssl",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"tls",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    if ( entry->_access == FTP )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

extern "C" {
    extern struct __fcs {
        char * name;
        enum FILECSET val;
    } _fcs[];
    extern int n_fcs;
};

void K_DIALOG_FTP_SETTINGS::
InitFTPCharsetList(KD_LIST_ITEM * entry)
{       
    int i;

    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_FTP_CHARSET ) ;
    for ( i=0; i<n_fcs; i++ ) {
        button = new UIW_BUTTON( 0,0,0,_fcs[i].name,BTF_NO_3D, WOF_NO_FLAGS );
        button->StringID(button->DataGet());
        *list + button;
    }

    if ( entry->_access == FTP )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

enum FILECSET K_DIALOG_FTP_SETTINGS::
GetFileCharsetFromCombo( UIW_COMBO_BOX * combo )
{
    ZIL_ICHAR * value = combo->DataGet() ;
    enum FILECSET charset = F_ASCII ;
    int i, x = 0;

    for (i = 0; i < n_fcs; i++) {
       if (!strcmp(_fcs[i].name, value)) {
	   x = 1;
	   break;
       }
    }
    if (x)
        charset = _fcs[i].val ;
    return charset ;
}

void K_DIALOG_FTP_SETTINGS::
SetComboToCharset( UIW_COMBO_BOX * combo, enum FILECSET charset )
{
    int i, x = 0;
   
    for (i = 0; i < n_fcs; i++) {
       if (_fcs[i].val == charset) {
	   x = 1;
	   break;
       }
    }
    if (x)
        combo->DataSet( _fcs[i].name ) ;
    else
        combo->DataSet( "ascii" ) ;
}

