#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_KEYBOARD_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetkeyboard.hpp"
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

ZIL_ICHAR K_DIALOG_KEYBOARD_SETTINGS::_className[] = "K_DIALOG_KEYBOARD_SETTINGS" ;

extern K_CONNECTOR * connector ;

static char *
os2getinidir( void ) 
{
    static char buffer[512] ;
    char * kermrc = "k95.ini", *lp ;
/*
  The -y init file must be fully specified or in the current directory.
  KERMRC is looked for via INIT, DPATH and PATH in that order.  Finally, our
  own executable file path is taken and the .EXE suffix is replaced by .INI
  and this is tried as the initialization file.
*/
    char * env = 0 ;
#ifdef WIN32
    env = getenv("K95.INI") ;
#else
    env = getenv("K2.INI");
#endif
  
    buffer[0] = '\0' ;

    if (env)
	strcpy(buffer,env);
    if (buffer[0] == 0)
	_searchenv(kermrc,"INIT",buffer);
    if (buffer[0] == 0)
	_searchenv(kermrc,"DPATH",buffer);
    if (buffer[0] == 0)
	_searchenv(kermrc,"PATH",buffer);
    if (buffer[0] == 0) {
#ifdef WIN32
	GetModuleFileName( NULL, buffer, 512 ) ;
#else
	PTIB pptib;
	PPIB pppib;
	char *szPath;

	DosGetInfoBlocks(&pptib, &pppib);

	szPath = pppib -> pib_pchcmd;

	while (*szPath)
	    szPath = strchr(szPath, 0) + 1;
	strcpy(buffer,szPath);
#endif
	if (buffer) {
	    lp = strrchr(buffer, '\\');
	    if ( !lp )
		lp = buffer ;
#ifdef WIN32
	    strcpy(lp+1, "k95.ini");
#else
	    strcpy(lp+1, "k2.ini");
#endif
	}
   }
   return buffer;
}       


static ZIL_INT32
LoadFileIntoText( UIW_TEXT * text, ZIL_ICHAR * filename )
{
   ULONG i ;
   FILE * InFile = NULL;
   ZIL_ICHAR buffer[30001] ;
   ZIL_ICHAR startupdir[512], inidir[512], exedir[512], * p ;
   ZIL_ICHAR searchpath[512] ;
   int n2 ;

    memset( buffer, 0, 30001 ) ;
    text->DataSet( "", 30000 ) ;

   for ( i=0; InFile == NULL && (i<=15) ; i++ )
   {
      switch( i )
      {
      case 0:
         strcpy( searchpath, filename ) ;
         break;
      case 1:
         /* Identify the startupdir */
         getcwd(startupdir,512) ;
         n2 = strlen( startupdir ) ;
         if ( startupdir[n2-1] != '\\' )
         {
            startupdir[n2++] = '\\' ;
            startupdir[n2] = '\0' ;
         }
         if ( n2 > -1 && n2 < 256 )
            if ( startupdir[n2-1] != '\\' )
            {
               startupdir[n2] = '\\' ;
               startupdir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 2:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 3:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 4:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 5:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 6:
         /* Identify the inidir */
         strcpy( inidir, os2getinidir() );
         if ( (p = strrchr( inidir, '\\')) != NULL )
            *p = '\0' ;
         n2 = strlen( inidir ) ;
         if ( n2 > -1 && n2 < 256 )
            if ( inidir[n2-1] != '\\' )
            {
               inidir[n2] = '\\' ;
               inidir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 7:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 8:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 9:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 10:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 11:
         /* Identify the exedir */
#ifdef WIN32
          GetModuleFileName( NULL, exedir, 512 ) ;
#else 
	  extern ZIL_ICHAR * pathname ;
	  strcpy( exedir, pathname ) ;
#endif
         if ( (p = strrchr( exedir, '\\')) != NULL )
            *p = '\0' ;
         n2 = strlen( exedir ) ;
         if ( n2 > -1 && n2 < 256 )
            if ( exedir[n2-1] != '\\' )
            {
               exedir[n2] = '\\' ;
               exedir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 12:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 13:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 14:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 15:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      }

      InFile = fopen ( searchpath, "rb" ) ;
   }

   /* Finally read the file */
   if ( InFile )
   {
       for ( int i=0;i<30000;i++ ) {
	   if ( feof(InFile) )
	       break;
	   buffer[i] = fgetc(InFile) ;
       }
       if ( buffer[0] ) {
           text->Information( I_SET_VALUE, buffer ) ;
	   text->DataSet( buffer, 30000 ) ;
       }
       fclose( InFile ) ; 
   }    
   return 0 ;
}


// Null Constructor for K_DIALOG_KEYBOARD_SETTINGS
//
//

K_DIALOG_KEYBOARD_SETTINGS::
K_DIALOG_KEYBOARD_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_KEYBOARD_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // Keyboard Settings
    UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_BACKSPACE ) ;
    UIW_BUTTON * button = (UIW_BUTTON *) Get( RADIO_BS_DEL ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

    group = (UIW_GROUP *) Get( GROUP_SET_KEYBOARD_ENTER ) ;
    button = (UIW_BUTTON *) Get( RADIO_SET_KEYBOARD_ENTER_CR ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

    group = (UIW_GROUP *) Get( GROUP_MOUSE ) ;
    button = (UIW_BUTTON *) Get( RADIO_MOUSE_ON ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

   // UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
   // text->woFlags |= WOF_VIEW_ONLY ;
   // text->Information( I_CHANGED_FLAGS,NULL ) ;

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
   string->DataSet( "" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

   group = (UIW_GROUP *) Get( GROUP_KEYMAP ) ;
   button = (UIW_BUTTON *) Get( RADIO_KEYMAP_DEFAULT ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

    button = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_KEYBOARD_SETTINGS
//
//

K_DIALOG_KEYBOARD_SETTINGS::
K_DIALOG_KEYBOARD_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_KEYBOARD_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_KEYBOARD_SETTINGS::K_DIALOG_KEYBOARD_SETTINGS(entry,mode)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Keyboard Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

   // Keyboard Settings
   UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_BACKSPACE ) ;
    UIW_BUTTON * button = NULL;
   switch ( entry->_backspace ) {
   case CTRL_H:
       button = (UIW_BUTTON *) Get( RADIO_BS_BS ) ;
       break;
   case DEL:
       button = (UIW_BUTTON *) Get( RADIO_BS_DEL ) ;
       break;
   case CTRL_Y:
       button = (UIW_BUTTON *) Get( RADIO_BS_EM ) ;
       break;
   }
    if ( button ) {
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
    }

   group = (UIW_GROUP *) Get( GROUP_SET_KEYBOARD_ENTER ) ;
   switch ( entry->_enter ) {
   case CR:
       button = (UIW_BUTTON *) Get( RADIO_SET_KEYBOARD_ENTER_CR ) ;
       break;
   case CRLF:
       button = (UIW_BUTTON *) Get( RADIO_SET_KEYBOARD_ENTER_CRLF ) ;
       break;
   case LF:
       button = (UIW_BUTTON *) Get( RADIO_SET_KEYBOARD_ENTER_LF ) ;
       break;
   }
    if ( button ) {
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
    }

   group = (UIW_GROUP *) Get( GROUP_MOUSE ) ;
   button = (UIW_BUTTON *) Get( entry->_mouse ? RADIO_MOUSE_ON : RADIO_MOUSE_OFF ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
   *group + button ;

   UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
   string->DataSet( entry->_keymap_file ) ;

   group = (UIW_GROUP *) Get( GROUP_KEYMAP ) ;
    UIW_BUTTON * button2;
   switch ( entry->_keymap ) {
      case VT100_KEY:
         button = (UIW_BUTTON *) Get( RADIO_KEYMAP_DEFAULT ) ;
         string->woFlags |= WOF_NON_SELECTABLE ;
         string->Information(I_CHANGED_FLAGS,NULL) ;
	  button2 = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
	  button2->woFlags |= WOF_NON_SELECTABLE ;
	  button2->Information( I_CHANGED_FLAGS, NULL ) ;
          text->DataSet(entry->_keyscript);
          text->woFlags &= ~WOF_VIEW_ONLY ;
          text->Information( I_CHANGED_FLAGS,NULL ) ;
	  break;
      case MAPFILE:
         button = (UIW_BUTTON *) Get( RADIO_KEYMAP_FILE ) ;
         LoadFileIntoText( text, string->DataGet() ) ;
          text->woFlags |= WOF_VIEW_ONLY ;
          text->Information( I_CHANGED_FLAGS,NULL ) ;
	  button2 = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
	  button2->woFlags &= ~WOF_NON_SELECTABLE ;
	  button2->Information( I_CHANGED_FLAGS, NULL ) ;
	  break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;


    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_KEYBOARD_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_KEYBOARD_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_KEYBOARD_SETTINGS::K_DIALOG_KEYBOARD_SETTINGS(entry)\n");
};

K_DIALOG_KEYBOARD_SETTINGS::~K_DIALOG_KEYBOARD_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_KEYBOARD_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_KEYBOARD_SETTINGS::Event()\n");

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

    case OPT_HELP_SET_KEYBOARD:
        helpSystem->DisplayHelp( windowManager, HELP_SET_KEYBOARD ) ;
        break;

    case OPT_KEYBOARD_OK: {
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
   case OPT_VT100_KEYMAP:
       string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
       string->woFlags |= WOF_NON_SELECTABLE ;
       string->Information(I_CHANGED_FLAGS,NULL) ;
       text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
       text->DataSet( _entry->_keyscript ) ;
        text->woFlags &= ~WOF_VIEW_ONLY ;
        text->Information( I_CHANGED_FLAGS,NULL ) ;
        button = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
       button->woFlags |= WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

   case OPT_FILE_KEYMAP:
        string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS,NULL) ;
       text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
        text->woFlags |= WOF_VIEW_ONLY ;
        text->Information( I_CHANGED_FLAGS,NULL ) ;
        LoadFileIntoText( text, string->DataGet() ) ;
       button = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

    case OPT_BROWSE_KEYMAP:
        {
            EVENT_TYPE OpenFile = OPT_FILE_OPEN ; 
            DirServRequestor = event.type ;
            directoryService->Information(I_SET_REQUESTOR,this);
            directoryService->Information(I_SET_REQUEST,&OpenFile);
            UI_EVENT  Reset_Filter(11003);   
            directoryService->Event(Reset_Filter);
            directoryService->Information(I_SET_FILTER,
                                           "*.ksc");
            directoryService->Information(I_ACTIVATE_SERVICE,ZIL_NULLP(void));
        }
        break;

    case -OPT_FILE_OPEN: {
        const ZIL_ICHAR * p = event.text;
        ZIL_ICHAR tmp[512], * q;
        if ( p ) {
            for ( q = tmp ; *p; p++, q++ ) {
                if ( *p == '\\' )
                    *q = '/';
                else
                    *q = *p;
            }
        }
        switch ( DirServRequestor ) {
        case OPT_BROWSE_KEYMAP:
            string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
            LoadFileIntoText( text, string->DataGet() ) ;
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

void K_DIALOG_KEYBOARD_SETTINGS::
ApplyChangesToEntry( void )
{
   // Keyboard Settings
   if ( FlagSet(Get( RADIO_BS_BS )->woStatus, WOS_SELECTED ) )
      _entry->_backspace = CTRL_H ;
   else if ( FlagSet(Get( RADIO_BS_DEL )->woStatus, WOS_SELECTED ) )
      _entry->_backspace = DEL ;
   else if ( FlagSet(Get( RADIO_BS_EM )->woStatus, WOS_SELECTED ) )
      _entry->_backspace = CTRL_Y ;

   if ( FlagSet(Get( RADIO_SET_KEYBOARD_ENTER_CR )->woStatus, WOS_SELECTED ) )
      _entry->_enter = CR ;
   else if ( FlagSet(Get( RADIO_SET_KEYBOARD_ENTER_CRLF )->woStatus, WOS_SELECTED ) )
      _entry->_enter = CRLF;
   else if ( FlagSet(Get( RADIO_SET_KEYBOARD_ENTER_LF )->woStatus, WOS_SELECTED ) )
      _entry->_enter = LF;

   if ( FlagSet(Get( RADIO_MOUSE_ON )->woStatus, WOS_SELECTED ) )
      _entry->_mouse = 1 ;
   else if ( FlagSet(Get( RADIO_MOUSE_OFF )->woStatus, WOS_SELECTED ) )
      _entry->_mouse = 0;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
    if ( FlagSet(Get( RADIO_KEYMAP_DEFAULT )->woStatus, WOS_SELECTED ) ) {
        _entry->_keymap = VT100_KEY ;
        strncpy( _entry->_keyscript, text->DataGet(), 3000 ) ;
        _entry->_keyscript[3000] = '\0';
    } else if ( FlagSet(Get( RADIO_KEYMAP_FILE )->woStatus, WOS_SELECTED ) )
      _entry->_keymap = MAPFILE;


    UIW_STRING * string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
    strncpy( _entry->_keymap_file, string->DataGet(), 256 ) ;
    _entry->_keymap_file[256] = '\0';
}

