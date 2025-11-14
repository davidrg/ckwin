#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_TRANSFER_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetxfer.hpp"
#include "kconnect.hpp"
#include "kmodem.hpp"
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

ZIL_ICHAR K_DIALOG_TRANSFER_SETTINGS::_className[] = "K_DIALOG_TRANSFER_SETTINGS" ;

extern K_CONNECTOR * connector ;

extern "C" {
struct __fcs {
    char * name;
    enum FILECSET val;
} _fcs[] = {
    "ascii", F_ASCII,
    "british", F_BRITISH,
    "bulgaria-pc", F_BULGARIA_PC,
    "canadian-french", F_CANADIAN_FRENCH,
    "cp1250", F_CP1250,
    "cp1251-cyrillic", F_CP1251,
    "cp1252", F_CP1252,
    "cp437", F_CP437,
    "cp850", F_CP850,
    "cp852", F_CP852,
    "cp855-cyrillic", F_CP855,
    "cp858", F_CP858,
    "cp862-hebrew", F_CP862_HEBREW,
    "cp866-cyrillic", F_CP866_CYRILLIC,
    "cp869-greek", F_CP869,
    "cyrillic-iso", F_CYRILLIC_ISO,
    "danish", F_DANISH,
    "dec-kanji", F_DEC_KANJI,
    "dec-multinational", F_DEC_MULTINATIONAL,
    "dg-international", F_DG_INTERNATIONAL,
    "dutch", F_DUTCH,
    "elot927-greek", F_ELOT927_GREEK,
    "elot928-greek", F_ELOT928_GREEK,
    "euc-jp", F_EUC_JP,
    "finnish", F_FINNISH,
    "french", F_FRENCH,
    "german", F_GERMAN,
    "greek-iso", F_GREEK_ISO,
    "hebrew-7", F_HEBREW_7,
    "hebrew-iso", F_HEBREW_ISO,
    "hp-roman8", F_HP_ROMAN_8,
    "hungarian", F_HUNGARIAN,
    "iso2022jp-kanji", F_ISO2022_JP,
    "italian", F_ITALIAN,
    "japananese-euc", F_JAPAN_EUC,
    "jis7-kanji", F_JIS7_KANJI,
    "kio8", F_KIO8_CYRILLIC,
    "koi8r", F_KOI8_R,
    "koi8u", F_KOI8_U,
    "latin1-iso", F_LATIN1_ISO,
    "latin2-iso", F_LATIN2_ISO,
    "latin9-iso", F_LATIN9_ISO,
    "macintosh-latin", F_MACINTOSH_LATIN,
    "mazovia-pc", F_MAZOVIA_PC,
    "next-multinational", F_NEXT_MULTINATIONAL,
    "norwegian", F_NORWEGIAN,
    "portuguese", F_PORTUGUESE,
    "shift-jis-kanji", F_SHIFT_JIS_KANJI,
    "short-koi", F_SHORT_KOI,
    "spanish", F_SPANISH,
    "swedish", F_SWEDISH,
    "swiss", F_SWISS,
    "ucs2",  F_UCS2,
    "utf8",  F_UTF8
};
int n_fcs = (sizeof(_fcs) / sizeof(struct __fcs));

struct __xcs {
    char * name;
    enum XFERCSET val;
} _xcs[] = {
    "ascii",  X_ASCII,
    "cyrillic-iso",  X_CYRILLIC_ISO,
    "greek-iso",   X_GREEK_ISO,
    "hebrew-iso",  X_HEBREW_ISO,
    "japanese-euc", X_JAPANESE_EUC,
    "latin1-iso",  X_LATIN1_ISO,
    "latin2-iso",  X_LATIN2_ISO,
    "transparent",  X_TRANSPARENT
};
int n_xcs = (sizeof(_xcs) / sizeof(struct __xcs));
};

// Null Constructor for K_DIALOG_TRANSFER_SETTINGS
//
//

K_DIALOG_TRANSFER_SETTINGS::
K_DIALOG_TRANSFER_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_TRANSFER_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // Configure default values
    //File Transfer
    UIW_COMBO_BOX *combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
    UIW_BUTTON *button = (UIW_BUTTON *) Get( BUTTON_KERMIT ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    combo->woFlags &= ~WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE ) ;
    button = (UIW_BUTTON *) Get( BUTTON_KERMIT_FAST ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    combo->woFlags &= ~WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_CCU ) ;
    button = (UIW_BUTTON *) Get( BUTTON_CCU_CAUTIOUS ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
    spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_GROUP *group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_MODE ) ;
    button = (UIW_BUTTON *) Get(RADIO_XFER_BINARY ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

    group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_COLLISION ) ;
    button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_STREAMING ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_CLEAR_CHANNEL ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;


   InitFileCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_FCS ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_FCS ) ;
   combo->DataSet( "ascii" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitXferCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_TCS ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_TCS ) ;
   combo->DataSet( "transparent" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;


    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_TRANSFER_SETTINGS
//
//

K_DIALOG_TRANSFER_SETTINGS::
K_DIALOG_TRANSFER_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_TRANSFER_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_TRANSFER_SETTINGS::K_DIALOG_TRANSFER_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"File Transfer Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Apply entry to Notebook
   //File Transfer
    UIW_BUTTON * button;
    UIW_COMBO_BOX *combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
    switch ( entry->_protocol ) {
    case K_FAST:
    case K_CAUTIOUS:
    case K_ROBUST:
    case K_CUSTOM:
         button = (UIW_BUTTON *) Get( BUTTON_KERMIT ) ;
         break;
      case Z:
         button = (UIW_BUTTON *) Get( BUTTON_ZMODEM ) ;
         break;
      case Y:
         button = (UIW_BUTTON *) Get( BUTTON_YMODEM ) ;
         break;
      case YG:
         button = (UIW_BUTTON *) Get( BUTTON_YMODEM_G ) ;
         break;
      case X:
         button = (UIW_BUTTON *) Get( BUTTON_XMODEM ) ;
         break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    if (entry->_access == FTP)
        combo->woFlags |= WOF_NON_SELECTABLE ;
    else
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE ) ;
    switch ( entry->_protocol ) {
    case K_FAST:
        button = (UIW_BUTTON *) Get( BUTTON_KERMIT_FAST ) ;
        break;
    case K_CAUTIOUS:
        button = (UIW_BUTTON *) Get( BUTTON_KERMIT_CAUTIOUS ) ;
        break;
    case K_ROBUST:
        button = (UIW_BUTTON *) Get( BUTTON_KERMIT_ROBUST ) ;
        break;
    case K_CUSTOM:
        button = (UIW_BUTTON *) Get( BUTTON_KERMIT_CUSTOM ) ;
        break;
    default:
        button = NULL;
        break;
    }
    if (entry->_access == FTP) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
    } else if ( button ) {
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *combo + button ;    
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
    } else {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
    }

    combo = (UIW_COMBO_BOX *) Get( COMBO_CCU ) ;
    switch ( entry->_unprefix_cc ) {
    case NEVER:
        button = (UIW_BUTTON *) Get( BUTTON_CCU_NEVER ) ;
        break;
    case CAUTIOUSLY:
        button = (UIW_BUTTON *) Get( BUTTON_CCU_CAUTIOUS ) ;
        break;
    case MOST:
        button = (UIW_BUTTON *) Get( BUTTON_CCU_WILD ) ;
        break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    switch ( entry->_protocol ) {
    case K_FAST:
    case K_CAUTIOUS:
    case K_ROBUST:
    case X:
    case Y:
    case YG:
        combo->woFlags |= WOF_NON_SELECTABLE ;
        break;
    default:
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    }
    if ( entry->_access == FTP )
        combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_SPIN_CONTROL *spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    UIW_INTEGER *spin_int = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
    int new_spin_int = entry->_packet_length ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    switch ( entry->_protocol ) {
    case K_FAST:
    case K_CAUTIOUS:
    case K_ROBUST:
    case Z:
        spin->woFlags |= WOF_NON_SELECTABLE ;
        break;
    default:
        spin->woFlags &= ~WOF_NON_SELECTABLE ;
    }
    if ( entry->_access == FTP )
        spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
    spin_int = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
    new_spin_int = entry->_max_windows ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    switch ( entry->_protocol ) {
    case K_FAST:
    case K_CAUTIOUS:
    case K_ROBUST:
    case X:
    case Y:
    case YG:
        spin->woFlags |= WOF_NON_SELECTABLE ;
        break;
    default:
        spin->woFlags &= ~WOF_NON_SELECTABLE ;
    }
    if ( entry->_access == FTP )
        spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

   UIW_GROUP *group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_MODE ) ;
   switch ( entry->_xfermode ) {
      case BINARY:
         button = (UIW_BUTTON *) Get(RADIO_XFER_BINARY ) ;
         break;
      case TEXT:
         button = (UIW_BUTTON *) Get(RADIO_XFER_TEXT ) ;
         break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
   *group + button ;
    if ( entry->_access == FTP ) {
        group->woFlags |= WOF_NON_SELECTABLE ;
        group->Information(I_CHANGED_FLAGS,NULL);
    }

   group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_COLLISION ) ;
   switch ( entry->_collision ) {
      case COL_APPEND:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
         break;
      case COL_BACKUP:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
         break;
      case COL_DISCARD:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
         break;
      case COL_OVERWRITE:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
         break;
      case COL_RENAME:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
         break;
      case COL_UPDATE:
         button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
         break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;
    if ( entry->_access == FTP ) {
        group->woFlags |= WOF_NON_SELECTABLE ;
        group->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
    if ( entry->_keep_incomplete )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_STREAMING ) ;
    if ( entry->_streaming )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_CLEAR_CHANNEL ) ;
    if ( entry->_clear_channel )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
    if ( entry->_pathnames )
      button->woStatus |= WOS_SELECTED ;
    else
      button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
    if ( entry->_fname_literal )
      button->woStatus |= WOS_SELECTED ;
    else
      button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    InitFileCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_FCS ) ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_FCS ) ;
    SetComboToCharset( combo, entry->_file_charset ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    InitXferCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_TCS ) ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_TCS ) ;
    SetComboToCharset( combo, entry->_xfer_charset ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_TRANSFER_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_TRANSFER_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_TRANSFER_SETTINGS::K_DIALOG_TRANSFER_SETTINGS(entry)\n");
};

K_DIALOG_TRANSFER_SETTINGS::~K_DIALOG_TRANSFER_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_TRANSFER_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_TRANSFER_SETTINGS::Event()\n");

    extern K_CONNECTOR * connector ; 
    EVENT_TYPE retval = event.type ; 
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
    case OPT_HELP_SET_FILE_TRANSFER:
        helpSystem->DisplayHelp( windowManager, HELP_SET_FILE_TRANSFER ) ;
        break;
    case OPT_FILE_TRANSFER_OK: {
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

    case OPT_KERMIT: {
        combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        UI_EVENT newevent(event);

        ZIL_ICHAR * ichar = combo->DataGet() ;
        if ( !strcmp(ichar,"Fast") )
            newevent.type = OPT_KERMIT_FAST;
        else if ( !strcmp(ichar,"Cautious") )
            newevent.type = OPT_KERMIT_CAUTIOUS;
        else if ( !strcmp(ichar,"Robust") )
            newevent.type = OPT_KERMIT_ROBUST;
        else if ( !strcmp(ichar,"Custom") )
            newevent.type = OPT_KERMIT_CUSTOM;

        Event(newevent);
        break;
    }

   case OPT_KERMIT_FAST:
       spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
       spin->woFlags |= WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       value = 4000 ;
       spin->Information( I_SET_VALUE, &value ) ;

       spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
       spin->woFlags |= WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       value = 30 ;
       spin->Information( I_SET_VALUE, &value ) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        button = (UIW_BUTTON *) Get( BUTTON_CCU_CAUTIOUS ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *combo + button;
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable all other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

      case OPT_KERMIT_CAUTIOUS:
        spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
        integer = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
        spin->woFlags |= WOF_NON_SELECTABLE ;
        spin->Information( I_CHANGED_FLAGS, NULL ) ;
        value = 1000 ;
        spin->Information( I_SET_VALUE, &value ) ;

        spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
        integer = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
        spin->woFlags |= WOF_NON_SELECTABLE ;
        spin->Information( I_CHANGED_FLAGS, NULL ) ;
        value = 4 ;
        spin->Information( I_SET_VALUE, &value ) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        button = (UIW_BUTTON *) Get( BUTTON_CCU_CAUTIOUS ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *combo + button;
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable all other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

   case OPT_KERMIT_ROBUST:
        spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
        integer = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
        spin->woFlags |= WOF_NON_SELECTABLE ;
        spin->Information( I_CHANGED_FLAGS, NULL ) ;
        value = 94 ;
        spin->Information( I_SET_VALUE, &value ) ;

        spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
        integer = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
        spin->woFlags |= WOF_NON_SELECTABLE ;
        spin->Information( I_CHANGED_FLAGS, NULL ) ;
        value = 1 ;
        spin->Information( I_SET_VALUE, &value ) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        button = (UIW_BUTTON *) Get( BUTTON_CCU_NEVER ) ;
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *combo + button;
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable all other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;


    case OPT_KERMIT_CUSTOM:
        spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
        spin->woFlags &= ~WOF_NON_SELECTABLE ;
        spin->Information(I_CHANGED_FLAGS,NULL) ;

        spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
        spin->woFlags &= ~WOF_NON_SELECTABLE ;
        spin->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable all other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

    case OPT_ZMODEM:
        combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
       spin->woFlags |= WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       //value = 4096 ;
       //spin->Information( I_SET_VALUE, &value ) ;

       spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
       spin->woFlags &= ~WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       value = 0 ;
       spin->Information( I_SET_VALUE, &value ) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable most other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_BACKUP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_OVER ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_RENAME ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_APPEND ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_DISCARD ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

   case OPT_YMODEM:
   case OPT_YMODEM_G:
   case OPT_XMODEM:
        combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       spin = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
       spin->woFlags &= ~WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       value = event.type == OPT_XMODEM ? 128 : 1024 ;
       spin->Information( I_SET_VALUE, &value ) ;

       spin = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
       integer = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
       spin->woFlags |= WOF_NON_SELECTABLE ;
       spin->Information( I_CHANGED_FLAGS, NULL ) ;
       value = 0 ;
       spin->Information( I_SET_VALUE, &value ) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_CCU );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

       /* Enable most other items on File Transfer Page */
       button = (UIW_BUTTON *) Get( RADIO_SET_TRANSFER_COLL_UPDATE ) ;
       button->woFlags |= WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_TRANSFER_SETTINGS::
ApplyChangesToEntry( void )
{
    //File Transfer

    UIW_COMBO_BOX *combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
    ZIL_ICHAR * ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Kermit") ) {
        combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PERFORMANCE ) ;
        ichar = combo->DataGet() ;
        if ( !strcmp(ichar,"Fast") )
            _entry->_protocol = K_FAST ;
        else if ( !strcmp(ichar,"Cautious") )
            _entry->_protocol = K_CAUTIOUS ;
        else if ( !strcmp(ichar,"Robust") )
            _entry->_protocol = K_ROBUST ;
        else if ( !strcmp(ichar,"Custom") )
            _entry->_protocol = K_CUSTOM ;
    } else if ( !strcmp(ichar,"Zmodem") ) {
        _entry->_protocol = Z ;
    } else if ( !strcmp(ichar,"Ymodem") ) {
        _entry->_protocol = Y ;
    } else if ( !strcmp(ichar,"Ymodem-G") ) {
        _entry->_protocol = YG ;
    } else if ( !strcmp(ichar,"Xmodem") ) {
        _entry->_protocol = X ;
    }

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
    int value;
    spin->Information( I_GET_VALUE, &value );
    _entry->_packet_length = value;

    switch ( _entry->_protocol ) {
    case X:
    case Y:
    case YG:
        if ( _entry->_packet_length <= 128 )
            _entry->_packet_length = 128;
        else 
            _entry->_packet_length = 1024;
        break;
    }
    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
    spin_int = (UIW_INTEGER *) spin->Get( SPIN_WINDOW_RANGE ) ;
    spin->Information( I_GET_VALUE, &value );
    _entry->_max_windows = value;

    combo = (UIW_COMBO_BOX *) Get( COMBO_CCU ) ;
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Never") ) {
        _entry->_unprefix_cc = NEVER ;
    } else if ( !strcmp(ichar,"Cautiously") ) {
        _entry->_unprefix_cc = CAUTIOUSLY ;
    } else if ( !strcmp(ichar,"With wild abandon") ) {
        _entry->_unprefix_cc = MOST ;
    } 

   if ( FlagSet(Get( RADIO_XFER_BINARY )->woStatus, WOS_SELECTED ) )
      _entry->_xfermode = BINARY ;
   else if ( FlagSet(Get( RADIO_XFER_TEXT )->woStatus, WOS_SELECTED ) )
      _entry->_xfermode = TEXT ;

   if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_APPEND )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_APPEND ;
   else if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_BACKUP )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_BACKUP ;
   else if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_DISCARD )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_DISCARD ;
   else if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_OVER )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_OVERWRITE ;
   else if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_RENAME )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_RENAME ;
   else if ( FlagSet(Get( RADIO_SET_TRANSFER_COLL_UPDATE )->woStatus, WOS_SELECTED ) )
      _entry->_collision = COL_UPDATE ;

    _entry->_keep_incomplete = FlagSet( Get( CHECK_SET_TRANSFER_INCOMPLETE )->woStatus, WOS_SELECTED ) ;
    _entry->_streaming = FlagSet( Get( CHECK_STREAMING )->woStatus, WOS_SELECTED ) ;
    _entry->_clear_channel = FlagSet( Get( CHECK_CLEAR_CHANNEL )->woStatus, WOS_SELECTED ) ;
    _entry->_pathnames = FlagSet( Get( CHECK_SET_TRANSFER_PATH )->woStatus, WOS_SELECTED ) ;
    _entry->_fname_literal = FlagSet( Get( CHECK_SET_TRANSFER_FNLIT )->woStatus, WOS_SELECTED ) ;

   _entry->_file_charset = GetFileCharsetFromCombo( (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_FCS ) ) ;
   _entry->_xfer_charset = GetXferCharsetFromCombo( (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_TCS ) ) ;
}

void K_DIALOG_TRANSFER_SETTINGS::InitFileCharsetList( UIW_VT_LIST * list )
{
    *list + new UIW_BUTTON( 0,0,0,"ascii",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"british",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"bulgaria-pc",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"canadian-french",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp1250",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp1251-cyrillic",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp1252",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp437",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp850",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp852",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp855-cyrillic",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp858",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp862-hebrew",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp866-cyrillic",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cp869-greek",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cyrillic-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"danish",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"dec-kanji",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"dec-multinational",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"dg-international",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"dutch",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"elot927-greek",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"elot928-greek",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"euc-jp",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"finnish",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"french",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"german",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"greek-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"hebrew-7",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"hebrew-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"hp-roman8",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"hungarian",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"iso2022jp-kanji",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"italian",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"japanese-euc",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"jis7-kanji",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"koi8",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"koi8r",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"koi8u",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin1-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin2-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin9-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"macintosh-latin",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"mazovia-pc",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"next-multinational",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"norwegian",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"portuguese",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"shift-jis-kanji",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"short-koi",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"spanish",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"swedish",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"swiss",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"ucs2",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"utf8",BTF_NO_3D, WOF_NO_FLAGS );
}
void K_DIALOG_TRANSFER_SETTINGS::InitXferCharsetList( UIW_VT_LIST * list )   
{
    *list + new UIW_BUTTON( 0,0,0,"ascii",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"cyrillic-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"euc-jp",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"greek-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"hebrew-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"japanese-euc",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin1-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin2-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"latin9-iso",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"transparent",BTF_NO_3D, WOF_NO_FLAGS ) ;
    *list + new UIW_BUTTON( 0,0,0,"ucs2",BTF_NO_3D, WOF_NO_FLAGS );
    *list + new UIW_BUTTON( 0,0,0,"utf8",BTF_NO_3D, WOF_NO_FLAGS );
}

enum FILECSET K_DIALOG_TRANSFER_SETTINGS::
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

enum XFERCSET K_DIALOG_TRANSFER_SETTINGS::
GetXferCharsetFromCombo( UIW_COMBO_BOX * combo )
{
   ZIL_ICHAR * value = combo->DataGet() ;
   enum XFERCSET charset = X_TRANSPARENT ;
   int i, x = 0;
   for (i = 0; i < n_xcs; i++) {
       if (!strcmp(_xcs[i].name, value)) {
	   x = 1;
	   break;
       }
   }
   if (x)
     charset = _xcs[i].val ;
   return charset ;
}

void K_DIALOG_TRANSFER_SETTINGS::
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

void K_DIALOG_TRANSFER_SETTINGS::
SetComboToCharset( UIW_COMBO_BOX * combo, enum XFERCSET charset )
{
   int i, x = 0;
   for (i = 0; i < n_xcs; i++) {
       if (_xcs[i].val == charset) {
	   x = 1;
	   break;
       }
   }
   if (x)
     combo->DataSet( _xcs[i].name ) ;
   else
     combo->DataSet( "transparent" ) ;
}
