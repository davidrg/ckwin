#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_LOGIN_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetlogin.hpp"
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

ZIL_ICHAR K_DIALOG_LOGIN_SETTINGS::_className[] = "K_DIALOG_LOGIN_SETTINGS" ;

extern "C" {
VOID ck_decrypt( char * );
VOID ck_encrypt( char * );
}

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


// Null Constructor for K_DIALOG_LOGIN_SETTINGS
//
//

K_DIALOG_LOGIN_SETTINGS::
K_DIALOG_LOGIN_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_LOGIN_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

   //Login Settings
   UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_SET_SCRIPTFILE ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
   string->DataSet( "" ) ;
   string->woFlags |= WOF_NON_SELECTABLE ;
   string->Information(I_CHANGED_FLAGS,NULL) ;
   
    button = (UIW_BUTTON *) Get( BUTTON_BROWSE_SCRIPT ) ;
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_USERID ) ;
    string->DataSet( "" ) ;

    button = (UIW_BUTTON *) Get( CHECK_PROMPT_FOR_PASSWORD ) ;
    button->woStatus &= ~WOS_SELECTED;
    button->Information( I_CHANGED_STATUS, NULL );
    button->woFlags &= WOF_NON_SELECTABLE ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_PASSWD ) ;
    string->DataSet( "" ) ;

    string = (UIW_STRING *) Get( FIELD_HOST_PROMPT ) ;
    string->DataSet( "$" ) ;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
    text->Information( I_SET_VALUE, "" ) ;
    text->DataSet( "", 3000 ) ;
    text->woFlags &= ~WOF_VIEW_ONLY ;
    text->Information(I_CHANGED_FLAGS, NULL);

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_LOGIN_SETTINGS
//
//

K_DIALOG_LOGIN_SETTINGS::
K_DIALOG_LOGIN_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_LOGIN_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_LOGIN_SETTINGS::K_DIALOG_LOGIN_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Login Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

   //Login Settings
   UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_SET_SCRIPTFILE ) ;
   if ( entry->_script_file )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
   string->DataSet( entry->_script_fname ) ;
   if ( !entry->_script_file )
   {
      string->woFlags |= WOF_NON_SELECTABLE ;
      string->Information(I_CHANGED_FLAGS,NULL) ;
   }
   else
   {
      string->woFlags &= ~WOF_NON_SELECTABLE ;
      string->Information(I_CHANGED_FLAGS, NULL ) ;
   }

    button = (UIW_BUTTON *) Get( BUTTON_BROWSE_SCRIPT ) ;
    if ( entry->_script_file )
	button->woFlags &= ~WOF_NON_SELECTABLE ;
    else
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_USERID ) ;
    string->DataSet( entry->_userid ) ;

    button = (UIW_BUTTON *) Get( CHECK_PROMPT_FOR_PASSWORD ) ;
    if ( entry->_prompt_for_password )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_PASSWD ) ;
    ck_decrypt( entry->_password ) ;
    string->DataSet( entry->_password ) ;
    ck_encrypt( entry->_password ) ;
    if ( entry->_prompt_for_password )
        string->woFlags |= WOF_NON_SELECTABLE;
    else 
        string->woFlags &= ~WOF_NON_SELECTABLE;
#ifdef NICE_IDEA_BUT_NO_GO
    /* There is no way to get an event when the userid field changes */
    if ( entry->_access == FTP  && !strcmp(entry->_userid,"anonymous") )
        string->stFlags &= ~STF_PASSWORD;
    else
        string->stFlags |= STF_PASSWORD ;
#endif
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get( FIELD_HOST_PROMPT ) ;
    string->DataSet( entry->_prompt ) ;

   UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
   if ( entry->_script_file )
   {
      text->woFlags |= WOF_VIEW_ONLY ;
      text->Information(I_CHANGED_FLAGS,NULL) ;
      LoadFileIntoText( text, entry->_script_fname ) ;
   }
   else 
   {
       // Let's see if making it View Only first allows it to 
       // work in OS/2.
       text->woFlags |= WOF_VIEW_ONLY ;
       text->Information(I_CHANGED_FLAGS,NULL) ;
       text->DataSet( "", 30000 );
       // Now set it to the real values.
       text->woFlags &= ~WOF_VIEW_ONLY;
       text->Information(I_CHANGED_FLAGS, NULL);
       text->DataSet( entry->_script, 3000 ) ;
   }

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_LOGIN_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_LOGIN_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_LOGIN_SETTINGS::K_DIALOG_LOGIN_SETTINGS(entry)\n");
};

K_DIALOG_LOGIN_SETTINGS::~K_DIALOG_LOGIN_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_LOGIN_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_LOGIN_SETTINGS::Event()\n");

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

    case OPT_HELP_SET_LOGIN:
        helpSystem->DisplayHelp( windowManager, HELP_SET_LOGIN ) ;
        break;

    case OPT_LOGIN_OK: {
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
   case OPT_SCRIPT_FILE:
       string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
       text   = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
       button = (UIW_BUTTON *) Get( BUTTON_BROWSE_SCRIPT ) ;
       if ( FlagSet(Get( CHECK_SET_SCRIPTFILE )->woStatus, WOS_SELECTED ) )
       {
	   string->woFlags &= ~WOF_NON_SELECTABLE ;
	   text->woFlags |= WOF_VIEW_ONLY ;
	   LoadFileIntoText( text, string->DataGet() );
	   button->woFlags &= ~WOF_NON_SELECTABLE ;
       }	
       else 	
       {	
	   string->woFlags |= WOF_NON_SELECTABLE ;
	   text->woFlags &= ~WOF_VIEW_ONLY ;
	   text->DataSet( "", 3000 ) ;
	   button->woFlags |= WOF_NON_SELECTABLE ;
       }	
       string->Information(I_CHANGED_FLAGS, NULL);
       text->Information( I_CHANGED_FLAGS, NULL );
       button->Information( I_CHANGED_FLAGS, NULL );
       break;

   case OPT_BROWSE_SCRIPT:
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
       case OPT_BROWSE_SCRIPT:
	   string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
	   string->Information(I_SET_TEXT,tmp);
	   text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
	   LoadFileIntoText( text, string->DataGet() ) ;
	   break;
       }
       break;
    }
    case OPT_CHECK_PASSWORD_PROMPT:
	string = (UIW_STRING *) Get( FIELD_PASSWD ) ;
	if ( FlagSet(Get( CHECK_PROMPT_FOR_PASSWORD )->woStatus, WOS_SELECTED ) )
	    string->woFlags |= WOF_NON_SELECTABLE ;
	else 	
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL);
        break;


    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_LOGIN_SETTINGS::
ApplyChangesToEntry( void )
{
   //Login Settings
   _entry->_script_file = FlagSet( Get( CHECK_SET_SCRIPTFILE )->woStatus, WOS_SELECTED ) ;

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
   strncpy( _entry->_script_fname, string->DataGet(), 256 ) ;

   string = (UIW_STRING *) Get( FIELD_USERID ) ;
   strncpy( _entry->_userid, string->DataGet(), 20 ) ;

    _entry->_prompt_for_password = 
        FlagSet(Get( CHECK_PROMPT_FOR_PASSWORD )->woStatus, WOS_SELECTED );

   string = (UIW_STRING *) Get( FIELD_PASSWD ) ;
   strncpy( _entry->_password, string->DataGet(), 20 ) ;
   ck_encrypt( _entry->_password ) ;

   string = (UIW_STRING *) Get( FIELD_HOST_PROMPT ) ;
   strncpy( _entry->_prompt, string->DataGet(), 20 ) ;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
    if ( !_entry->_script_file ) {
#ifdef WIN32
        strcpy( _entry->_script, text->DataGet() );
#else /* WIN32 */
        ZIL_ICHAR * data = text->DataGet();
        int len = strlen(data);
        int i=0,j=0;
        for ( ; i<len ; i++,j++) {
            switch (data[i]) {
              case 13:
                  if (data[i+1] == 10) {
                      i++;
                      _entry->_script[j] = 10;
                  }
                  else {
                      _entry->_script[j] = 10;
                  }
                  break;
              case 10:
                  _entry->_script[j] = 10;
                  break;
              default:
                  _entry->_script[j] = data[i];
            }
        }
        _entry->_script[j] = '\0';
#endif /* WIN32 */
    }
    else {
	_entry->_script[0] = '\0';
    }
   _entry->_userdefined = 1 ;


}

