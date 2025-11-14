#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_SSH_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetssh.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"

ZIL_ICHAR K_DIALOG_SSH_SETTINGS::_className[] = "K_DIALOG_SSH_SETTINGS" ;

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


// Null Constructor for K_DIALOG_SSH_SETTINGS
//
//

K_DIALOG_SSH_SETTINGS::
K_DIALOG_SSH_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_SSH_SETTINGS",ADD_ENTRY),
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

// KD_LIST_ITEM Constructor for K_DIALOG_SSH_SETTINGS
//
//

void setListItemState(UIW_VT_LIST * list, const char* listItemValue, ZIL_UINT8 state) {
    UIW_BUTTON *  button = (UIW_BUTTON *) list->Get(listItemValue);
    if ( state ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);
}

K_DIALOG_SSH_SETTINGS::
K_DIALOG_SSH_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode )
   : K95_SETTINGS_DIALOG("DIALOG_SSH_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_SSH_SETTINGS::K_DIALOG_SSH_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"SSH Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Fill in details here for configuring the dialog controls
    // based upon the entry settings

    //InitSSH1CipherList(entry);
    InitSSH2AuthList(entry);
    InitSSH2CipherList(entry);
    InitSSH2MacList(entry);
    InitSSH2HKAList(entry);

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_SSH_COMPRESSION ) ;
    button->woFlags = (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ssh_compress )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SSH_X11 ) ;
    button->woFlags = (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ssh_x11 )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_SSH_FWD_CRED ) ;
    button->woFlags = (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    button->Information( I_CHANGED_FLAGS, NULL ) ;
    if ( entry->_ssh_credfwd )
        button->woStatus |= WOS_SELECTED;
    else
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);

    /*UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_V1_CIPHER ) ;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get(LIST_SSH_V1_CIPHER);
    switch ( entry->_ssh1_cipher ) {
    case SSH1_CIPHER_3DES:
        button = (UIW_BUTTON *) list->Get("3des") ;
        break;
    case SSH1_CIPHER_BLOWFISH:
        button = (UIW_BUTTON *) list->Get("blowfish") ;
        break;
    case SSH1_CIPHER_DES:
        button = (UIW_BUTTON *) list->Get("des") ;
        break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ; 
    if ( entry->_access == SSH )
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;*/

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_HOST_CHECK ) ;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_HOST_CHECK );
    switch ( entry->_ssh_host_check ) {
    case HC_STRICT:
        button = (UIW_BUTTON *) list->Get(BUTTON_SSH_STRICT);
        break;
    case HC_ASK:
        button = (UIW_BUTTON *) list->Get(BUTTON_SSH_ASK);
        break;
    case HC_NONE:
        button = (UIW_BUTTON *) list->Get(BUTTON_SSH_NO_CHECK);
        break;
    }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    if ( entry->_access == SSH )
        combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_AUTH);
    /*button = (UIW_BUTTON *) list->Get("external-keyx");
    if ( entry->_ssh2_auth_external_keyx ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    button = (UIW_BUTTON *) list->Get("gssapi");
    if ( entry->_ssh2_auth_gssapi ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    /*button = (UIW_BUTTON *) list->Get("hostbased");
    if ( entry->_ssh2_auth_hostbased ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    button = (UIW_BUTTON *) list->Get("keyboard-interactive");
    if ( entry->_ssh2_auth_keyboard_interactive ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("password");
    if ( entry->_ssh2_auth_password ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("publickey");
    if ( entry->_ssh2_auth_publickey ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    /*button = (UIW_BUTTON *) list->Get("srp-gex-sha1");
    if ( entry->_ssh2_auth_srp_gex_sha1 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_CIPHERS);
    button = (UIW_BUTTON *) list->Get("3des-cbc");
    if ( entry->_ssh2_cipher_3des ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("aes128-cbc");
    if ( entry->_ssh2_cipher_aes128 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("aes192-cbc");
    if ( entry->_ssh2_cipher_aes192 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("aes256-cbc");
    if ( entry->_ssh2_cipher_aes256 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    setListItemState(list, "aes128-ctr", entry->_ssh2_cipher_aes128ctr);
    setListItemState(list, "aes192-ctr", entry->_ssh2_cipher_aes192ctr);
    setListItemState(list, "aes256-ctr", entry->_ssh2_cipher_aes256ctr);
    setListItemState(list, "aes256-gcm@openssh.com", entry->_ssh2_cipher_aes256_gcm_openssh);
    setListItemState(list, "chachae20-poly1305", entry->_ssh2_cipher_chachae20_poly1305);
    setListItemState(list, "aes128-gcm@openssh.com", entry->_ssh2_cipher_aes128_gcm_openssh);

    /*
    button = (UIW_BUTTON *) list->Get("arcfour");
    if ( entry->_ssh2_cipher_arcfour ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("blowfish-cbc");
    if ( entry->_ssh2_cipher_blowfish ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("cast128-cbc");
    if ( entry->_ssh2_cipher_cast128 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_MACS);
    /*button = (UIW_BUTTON *) list->Get("hmac-md5");
    if ( entry->_ssh2_mac_md5 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("hmac-md5-96");
    if ( entry->_ssh2_mac_md5_96 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("hmac-ripemd160");
    if ( entry->_ssh2_mac_ripemd160 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    button = (UIW_BUTTON *) list->Get("hmac-sha1");
    if ( entry->_ssh2_mac_sha1 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    /*button = (UIW_BUTTON *) list->Get("hmac-sha1-96");
    if ( entry->_ssh2_mac_sha1_96 ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);*/

    setListItemState(list, "hmac-sha2-256-etm@openssh.com", entry->_ssh2_mac_sha2_256_etm_openssh);
    setListItemState(list, "hmac-sha2-512-etm@openssh.com", entry->_ssh2_mac_sha2_512_etm_openssh);
    setListItemState(list, "hmac-sha1-etm@openssh.com", entry->_ssh2_mac_sha1_etm_openssh);
    setListItemState(list, "hmac-sha2-256", entry->_ssh2_mac_sha2_256);
    setListItemState(list, "hmac-sha2-512", entry->_ssh2_mac_sha2_512);
    setListItemState(list, "none", entry->_ssh2_mac_none);

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_HKA);
    button = (UIW_BUTTON *) list->Get("ssh-dss");
    if ( entry->_ssh2_hka_dss ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) list->Get("ssh-rsa");
    if ( entry->_ssh2_hka_rsa ) {
        button->woStatus |= WOS_SELECTED;
    } else {
        button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);

    setListItemState(list, "ssh-ed25519", entry->_ssh2_hka_ssh_ed25519);
    setListItemState(list, "ecdsa-sha2-nistp521", entry->_ssh2_hka_ecdsa_sha2_nistp521);
    setListItemState(list, "ecdsa-sha2-nistp384", entry->_ssh2_hka_ecdsa_sha2_nistp384);
    setListItemState(list, "ecdsa-sha2-nistp256", entry->_ssh2_hka_ecdsa_sha2_nistp256);
    setListItemState(list, "rsa-sha2-512", entry->_ssh2_hka_rsa_sha2_512);
    setListItemState(list, "rsa-sha2-256", entry->_ssh2_hka_rsa_sha2_256);

    /*UIW_STRING * string = (UIW_STRING *) Get( FIELD_SSH_V1_ID_FILE ) ;
    string->DataSet( entry->_ssh1_id_file, 256 ) ;
    string->woFlags |= (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_SSH_V1_KNOWN_HOSTS ) ;
    string->DataSet( entry->_ssh1_kh_file, 256 ) ;
    string->woFlags |= (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;*/

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_SSH_V2_ID_FILE ) ;
    string->DataSet( entry->_ssh2_id_file, 256 ) ;
    string->woFlags |= (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_SSH_V2_KNOWN_HOSTS ) ;
    string->DataSet( entry->_ssh2_kh_file, 256 ) ;
    string->woFlags |= (entry->_access != SSH ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    /*button = (UIW_BUTTON *) Get(BUTTON_SSH1_BROWSE_IDF);
    if ( entry->_access == SSH )
        button->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL) ;*/

    button = (UIW_BUTTON *) Get(BUTTON_SSH2_BROWSE_IDF);
    if ( entry->_access == SSH )
        button->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL) ;

    /*button = (UIW_BUTTON *) Get(BUTTON_SSH1_BROWSE_KNF);
    if ( entry->_access == SSH )
        button->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL) ;*/

    button = (UIW_BUTTON *) Get(BUTTON_SSH2_BROWSE_KHF);
    if ( entry->_access == SSH )
        button->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL) ;


    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_SET_SSH_OK);
        button->DataSet("Save/Next");           
        button = (UIW_BUTTON *) Get(BUTTON_SET_SSH_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_SSH_SETTINGS::K_DIALOG_SSH_SETTINGS(entry,mode)\n");
};

K_DIALOG_SSH_SETTINGS::~K_DIALOG_SSH_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_SSH_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_SSH_SETTINGS::Event()\n");

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
    case OPT_HELP_SET_SSH:
        helpSystem->DisplayHelp( windowManager, HELP_SET_SSH ) ;
        break;

    case OPT_SSH_OK: {
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
    //case OPT_BROWSE_SSH1_IDF:
    //case OPT_BROWSE_SSH1_KHF:
    case OPT_BROWSE_SSH2_IDF:
    case OPT_BROWSE_SSH2_KHF:
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
            for ( q = tmp ; *p; p++, q++ ) {
                if ( *p == '\\' )
                    *q = '/';
                else
                    *q = *p;
            }
        }
        switch ( DirServRequestor ) {
        /*case OPT_BROWSE_SSH1_IDF:
            string = (UIW_STRING *) Get( FIELD_SSH_V1_ID_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        case OPT_BROWSE_SSH1_KHF:
            string = (UIW_STRING *) Get( FIELD_SSH_V1_KNOWN_HOSTS ) ;
            string->Information(I_SET_TEXT,tmp);
            break;*/
        case OPT_BROWSE_SSH2_IDF:
            string = (UIW_STRING *) Get( FIELD_SSH_V2_ID_FILE ) ;
            string->Information(I_SET_TEXT,tmp);
            break;
        case OPT_BROWSE_SSH2_KHF:
            string = (UIW_STRING *) Get( FIELD_SSH_V2_KNOWN_HOSTS ) ;
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

void K_DIALOG_SSH_SETTINGS::
ApplyChangesToEntry( void )
{
    // Copy new values back to the original list item.

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_SSH_COMPRESSION ) ;
    _entry->_ssh_compress = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_SSH_X11 ) ;
    _entry->_ssh_x11 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_SSH_FWD_CRED ) ;
    _entry->_ssh_credfwd = button->woStatus & WOS_SELECTED;

    /*UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_V1_CIPHER ) ;
    button = (UIW_BUTTON *) combo->Current();
    if ( !strcmp(button->StringID(), "3des") )
        _entry->_ssh1_cipher = SSH1_CIPHER_3DES;
    else if ( !strcmp(button->StringID(), "blowfish") )
        _entry->_ssh1_cipher = SSH1_CIPHER_BLOWFISH;
    else if ( !strcmp(button->StringID(), "des") )
        _entry->_ssh1_cipher = SSH1_CIPHER_DES;*/

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_HOST_CHECK ) ;
    button = (UIW_BUTTON *) combo->Current();
    switch (button->NumberID()) {
    case BUTTON_SSH_STRICT:
        _entry->_ssh_host_check = HC_STRICT;
        break;
    case BUTTON_SSH_ASK:
        _entry->_ssh_host_check = HC_ASK;
        break;
    case BUTTON_SSH_NO_CHECK:
        _entry->_ssh_host_check = HC_NONE;
        break;
    }

    UIW_VT_LIST * list = (UIW_VT_LIST *) Get(LIST_SSH_V2_AUTH);

    /*button = (UIW_BUTTON *) list->Get("external-keyx");
    _entry->_ssh2_auth_external_keyx = button->woStatus & WOS_SELECTED;*/

    button = (UIW_BUTTON *) list->Get("gssapi");
    _entry->_ssh2_auth_gssapi = button->woStatus & WOS_SELECTED;

    /*button = (UIW_BUTTON *) list->Get("hostbased");
    _entry->_ssh2_auth_hostbased = button->woStatus & WOS_SELECTED;*/

    button = (UIW_BUTTON *) list->Get("keyboard-interactive");
    _entry->_ssh2_auth_keyboard_interactive = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("password");
    _entry->_ssh2_auth_password = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("publickey");
    _entry->_ssh2_auth_publickey = button->woStatus & WOS_SELECTED;

    /*button = (UIW_BUTTON *) list->Get("srp-gex-sha1");
    _entry->_ssh2_auth_srp_gex_sha1 = button->woStatus & WOS_SELECTED;*/

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_CIPHERS);
    button = (UIW_BUTTON *) list->Get("3des-cbc");
    _entry->_ssh2_cipher_3des = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes128-cbc");
    _entry->_ssh2_cipher_aes128 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes192-cbc");
    _entry->_ssh2_cipher_aes192 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes256-cbc");
    _entry->_ssh2_cipher_aes256 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("chachae20-poly1305");
    _entry->_ssh2_cipher_chachae20_poly1305 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes256-gcm@openssh.com");
    _entry->_ssh2_cipher_aes256_gcm_openssh = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes128-gcm@openssh.com");
    _entry->_ssh2_cipher_aes128_gcm_openssh = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes256-ctr");
    _entry->_ssh2_cipher_aes256ctr = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes192-ctr");
    _entry->_ssh2_cipher_aes192ctr = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("aes128-ctr");
    _entry->_ssh2_cipher_aes128ctr = button->woStatus & WOS_SELECTED;

    /*button = (UIW_BUTTON *) list->Get("arcfour");
    _entry->_ssh2_cipher_arcfour = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("blowfish-cbc");
    _entry->_ssh2_cipher_blowfish = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("cast128-cbc");
    _entry->_ssh2_cipher_cast128 = button->woStatus & WOS_SELECTED;*/

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_MACS);
    /*button = (UIW_BUTTON *) list->Get("hmac-md5");
    _entry->_ssh2_mac_md5 = button->woStatus & WOS_SELECTED;*/

    button = (UIW_BUTTON *) list->Get("hmac-sha2-256-etm@openssh.com");
    _entry->_ssh2_mac_sha2_256_etm_openssh = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha2-512-etm@openssh.com");
    _entry->_ssh2_mac_sha2_512_etm_openssh = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha1-etm@openssh.com");
    _entry->_ssh2_mac_sha1_etm_openssh = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha2-256");
    _entry->_ssh2_mac_sha2_256 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha2-512");
    _entry->_ssh2_mac_sha2_512 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("none");
    _entry->_ssh2_mac_none = button->woStatus & WOS_SELECTED;

    /*button = (UIW_BUTTON *) list->Get("hmac-md5-96");
    _entry->_ssh2_mac_md5_96 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-ripemd160");
    _entry->_ssh2_mac_ripemd160 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha1");
    _entry->_ssh2_mac_sha1 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("hmac-sha1-96");
    _entry->_ssh2_mac_sha1_96 = button->woStatus & WOS_SELECTED;*/

    list = (UIW_VT_LIST *) Get(LIST_SSH_V2_HKA);
    button = (UIW_BUTTON *) list->Get("ssh-dss");
    _entry->_ssh2_hka_dss = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ssh-rsa");
    _entry->_ssh2_hka_rsa = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ssh-ed25519");
    _entry->_ssh2_hka_ssh_ed25519 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ecdsa-sha2-nistp521");
    _entry->_ssh2_hka_ecdsa_sha2_nistp521 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ecdsa-sha2-nistp384");
    _entry->_ssh2_hka_ecdsa_sha2_nistp384 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("ecdsa-sha2-nistp256");
    _entry->_ssh2_hka_ecdsa_sha2_nistp256 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("rsa-sha2-512");
    _entry->_ssh2_hka_rsa_sha2_512 = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) list->Get("rsa-sha2-256");
    _entry->_ssh2_hka_rsa_sha2_256 = button->woStatus & WOS_SELECTED;

    /*UIW_STRING * string = (UIW_STRING *) Get( FIELD_SSH_V1_ID_FILE ) ;
    strcpy(_entry->_ssh1_id_file, string->DataGet()) ;

    string = (UIW_STRING *) Get( FIELD_SSH_V1_KNOWN_HOSTS ) ;
    strcpy(_entry->_ssh1_kh_file, string->DataGet() ) ;*/

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_SSH_V2_ID_FILE ) ;
    strcpy(_entry->_ssh2_id_file, string->DataGet() ) ;

    string = (UIW_STRING *) Get( FIELD_SSH_V2_KNOWN_HOSTS ) ;
    strcpy(_entry->_ssh2_kh_file, string->DataGet() ) ;
}

/*void K_DIALOG_SSH_SETTINGS::
InitSSH1CipherList( KD_LIST_ITEM * entry )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_V1_CIPHER ) ;
    UIW_BUTTON * button;

    button = new UIW_BUTTON( 0,0,0,"3des",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"blowfish",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"des",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    if ( entry->_access == SSH )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}*/

void K_DIALOG_SSH_SETTINGS::
InitSSH2CipherList( KD_LIST_ITEM * entry )
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_V2_CIPHERS ) ;
    button = new UIW_BUTTON( 0,0,0,"3des-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes128-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes128-ctr",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes128-gcm@openssh.com",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes192-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes192-ctr",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes256-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes256-ctr",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"aes256-gcm@openssh.com",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"chachae20-poly1305",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    /*button = new UIW_BUTTON( 0,0,0,"arcfour",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"blowfish-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"cast128-cbc",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/

    if ( entry->_access == SSH )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

void K_DIALOG_SSH_SETTINGS::
InitSSH2AuthList(KD_LIST_ITEM * entry)
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_V2_AUTH ) ;
    /*button = new UIW_BUTTON( 0,0,0,"external-keyx",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/
    button = new UIW_BUTTON( 0,0,0,"gssapi",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    /*button = new UIW_BUTTON( 0,0,0,"hostbased",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/
    button = new UIW_BUTTON( 0,0,0,"keyboard-interactive",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"password",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"publickey",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    /*button = new UIW_BUTTON( 0,0,0,"srp-gex-sha1",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/

    if ( entry->_access == SSH )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

void K_DIALOG_SSH_SETTINGS::
InitSSH2MacList(KD_LIST_ITEM * entry)
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_V2_MACS ) ;

    /*button = new UIW_BUTTON( 0,0,0,"hmac-md5",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"hmac-md5-96",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;
    button = new UIW_BUTTON( 0,0,0,"hmac-ripemd160",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/

    button = new UIW_BUTTON( 0,0,0,"hmac-sha1",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"hmac-sha1-etm@openssh.com",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"hmac-sha2-256",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"hmac-sha2-256-etm@openssh.com",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"hmac-sha2-512",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"hmac-sha2-512-etm@openssh.com",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"none",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    /*button = new UIW_BUTTON( 0,0,0,"hmac-sha1-96",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;*/

    if ( entry->_access == SSH )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}

void K_DIALOG_SSH_SETTINGS::
InitSSH2HKAList(KD_LIST_ITEM * entry)
{
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SSH_V2_HKA ) ;

    button = new UIW_BUTTON( 0,0,0,"ecdsa-sha2-nistp256",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"ecdsa-sha2-nistp384",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"ecdsa-sha2-nistp521",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"rsa-sha2-256",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"rsa-sha2-512",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"ssh-dss",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"ssh-ed25519",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    button = new UIW_BUTTON( 0,0,0,"ssh-rsa",BTF_NO_3D, WOF_NO_FLAGS );
    button->StringID(button->DataGet());
    *list + button;

    if ( entry->_access == SSH )
        list->woFlags &= ~WOF_NON_SELECTABLE ;
    else
        list->woFlags |= WOF_NON_SELECTABLE;
    list->Information(I_CHANGED_FLAGS,NULL) ;
}
