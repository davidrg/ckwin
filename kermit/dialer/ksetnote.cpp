#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_WINDOW_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetnote.hpp"
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
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
    extern struct keytab * tapilinetab, * _tapilinetab;
    extern int ntapiline;
}
#endif /* WIN32 */

extern ZIL_UINT8 TapiAvail;

ZIL_ICHAR K_SETTINGS_NOTEBOOK::_className[] = "K_SETTINGS_NOTEBOOK" ;

extern "C" {
VOID ck_decrypt( char * );
VOID ck_encrypt( char * );
extern struct __fcs {
    char * name;
    enum FILECSET val;
} _fcs[];
extern int n_fcs;
extern struct __xcs {
    char * name;
    enum XFERCSET val;
} _xcs[];
extern int n_xcs;
}

extern K_CONNECTOR * connector ;

extern struct __tcs {
    char * name;
    enum TERMCSET val;
} _tcs[];
extern int n_tcs;
#ifdef COMMENT 
= {
    "arabic-iso",T_ARABIC_ISO,
    "ascii", T_ASCII,
    "british", T_BRITISH,
    "bulgaria-pc",      T_CP856,
    "canadian_french", T_CANADIAN_FRENCH,
#ifdef COMMENT 
    "cp037", T_CP37,
#endif 
    "cp437", T_CP437,
    "cp813", T_CP813,
    "cp819", T_CP819,
    "cp850", T_CP850,
    "cp852", T_CP852,
    "cp855", T_CP855,
    "cp856", T_CP856,
    "cp857", T_CP857,
    "cp858", T_CP858,
    "cp862-hebrew", T_CP862_HEBREW,
    "cp864", T_CP864,
    "cp866", T_CP866_CYRILLIC,
    "cp869", T_CP869,
    "cp912", T_CP912,
    "cp913", T_CP913,
    "cp914", T_CP914,
    "cp915", T_CP915,
    "cp916", T_CP916,
    "cp920", T_CP920,
    "cp923", T_CP923,
    "cp1051", T_CP1051,
    "cp1089", T_CP1089,
    "cp1250", T_CP1250,
    "cp1251", T_CP1251,
    "cp1252", T_CP1252,
    "cp1253", T_CP1253,
    "cp1254", T_CP1254,
    "cp1255", T_CP1255,
    "cp1256", T_CP1256,
    "cp1257", T_CP1257,
    "cp1258", T_CP1258,
    "cp10000", T_CP10000,
    "cyrillic-iso", T_CYRILLIC_ISO,
    "danish", T_DANISH,
    "dec-multinational", T_DEC_MULTINATIONAL,
    "dec-special", T_DEC_SPECIAL,
    "dec-technical", T_DEC_TECHNICAL,
    "dg-international", T_DG_INTERNATIONAL,
    "dg-specialgraphics", T_DG_SPECIAL,
    "dg-linedrawing", T_DG_LINEDRAW,
    "dg-wordprocessing", T_DG_WORDPROC,
    "dutch", T_DUTCH,
    "elot927-greek", T_ELOT927_GREEK,
    "elot928-greek", T_ELOT928,
    "finnish", T_FINNISH,
    "french", T_FRENCH,
    "german", T_GERMAN,
    "greek-iso", T_GREEK_ISO,
    "hebrew-7", T_HEBREW_7,
    "hebrew-iso", T_HEBREW_ISO,
    "hp-line-drawing", T_HP_LINE,
    "hp-math/technical", T_HP_MATH,
    "hp-roman8", T_HP_ROMAN_8,
    "hungarian", T_HUNGARIAN,
    "italian", T_ITALIAN,
    "japanese-roman",T_JAPANESE_ROMAN,
    "katakana",T_KATAKANA,
    "kio8",  T_KIO8,
    "koi8r", T_KOI8_R,
    "koi8u", T_KOI8_U,
    "latin1-iso", T_LATIN1_ISO,
    "latin2-iso", T_LATIN2_ISO,
    "latin3-iso", T_LATIN3_ISO,
    "latin4-iso", T_LATIN4_ISO,
    "latin5-iso", T_LATIN5_ISO,
    "latin6-iso", T_LATIN6_ISO,
    "latin9-iso", T_8859_15,
    "macintosh-latin", T_MACINTOSH_LATIN,
    "mazovia-pc",       T_MAZOVIA,
    "next-multinational", T_NEXT_MULTINATIONAL,
    "norwegian", T_NORWEGIAN,
    "portuguese", T_PORTUGUESE,
    "qnx-console", T_QNX_CONSOLE,
    "short-koi", T_SHORT_KOI,
    "sni-blanks", T_SNI_BLANKS,
    "sni-brackets", T_SNI_BRACKETS,
    "sni-euro", T_SNI_EURO,
    "sni-facet", T_SNI_FACET,
    "sni-ibm", T_SNI_IBM,
    "spanish", T_SPANISH,
    "swedish", T_SWEDISH,
    "swiss", T_SWISS,   
    "transparent", T_TRANSPARENT,
    "utf7",  T_UTF7,
    "utf8",  T_UTF8
};
int n_tcs = (sizeof(_tcs) / sizeof(struct __tcs));

struct __fcs {
    char * name;
    enum FILECSET val;
} _fcs[];
extern int n_fcs;
= {
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
} _xcs[];
extern int n_xcs;
= {
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
#endif /* COMMENT */

char *
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


ZIL_INT32
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


// Null Constructor for K_SETTINGS_NOTEBOOK
//
//

K_SETTINGS_NOTEBOOK::
K_SETTINGS_NOTEBOOK(void)
   : ZAF_DIALOG_WINDOW("WINDOW_SETTINGS",defaultStorage),
   _entry(NULL),
   _original_name(NULL),
   _mode(ADD_ENTRY),
   _initialized(0)
{
    K_MODEM      * modem = connector->FindModem( "DEFAULT" );
    if ( !modem )
	modem = connector->FindModem( NULL );
    ZIL_ICHAR itoabuf[40] ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    Information( I_SET_TEXT, "Kermit 95 Connection Entry" ) ;

    _ip[0] = _lat[0] = _phone[0] = '\0';

    // Configure default values
    // General Page
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
    string->DataSet( "", 28 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    string->DataSet( "", 20 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    _transport = PHONE;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_CONNECTION_TYPE ) ;
    UIW_BUTTON * button = (UIW_BUTTON *) Get( RADIO_PHONE_NUMBER ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;
    
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

    string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
    string->DataSet( "", 32 ) ;
    string->woFlags |= WOF_NON_SELECTABLE ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL ) ;
    button = (UIW_BUTTON *) Get( PROTO_DEFAULT ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *combo + button ;    
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL ) ;
    button = (UIW_BUTTON *) Get( SSH_PROTO_AUTO ) ;
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

   // Terminal Page
    InitTermTypeList();
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;
    SetTermTypeList( "VT320" );

    UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
    button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

   InitWidthList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
   combo->DataSet( "80" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitHeightList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
   combo->DataSet( "24" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE );
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

   InitCursorList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_CURSOR ) ;
   combo->DataSet( "full" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_SCROLLBACK ) ;
   UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( SPIN_INT_SCROLLBACK ) ;

   InitTermCharsetList( (UIW_VT_LIST *) Get( LIST_TERM_CHARSET ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
   combo->DataSet( "transparent" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitColorLists() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
   combo->DataSet( "LightGray" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
   combo->DataSet( "Blue" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_FG ) ;
   combo->DataSet( "LightGray" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_BG ) ;
   combo->DataSet( "Cyan" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_FG ) ;
   combo->DataSet( "LightGray" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_BG ) ;
   combo->DataSet( "Cyan" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_FG ) ;
   combo->DataSet( "Black" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_BG ) ;
   combo->DataSet( "Yellow" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
   combo->DataSet( "LightGray" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
   combo->DataSet( "Red" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_FG ) ;
   combo->DataSet( "LightGray" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_BG ) ;
   combo->DataSet( "Black" ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

    //File Transfer
    combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
    button = (UIW_BUTTON *) Get( BUTTON_KERMIT ) ;
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

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_WINDOW_SIZE ) ;
    spin->woFlags |= WOF_NON_SELECTABLE ;
    spin->Information(I_CHANGED_FLAGS,NULL) ;

    group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_MODE ) ;
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

    // Communications
    InitModemList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
    if ( modem )
	combo->DataSet( modem->_name ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    InitLineList( 0 ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
    if ( modem )
	combo->DataSet( modem->_port ) ;
    combo->woFlags |= WOF_AUTO_CLEAR | WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

    InitParityList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
    if ( modem )
    switch ( modem->_parity )
    {
    case NO_PARITY:
	combo->DataSet("None");
	break;
    case SPACE:
	combo->DataSet("Space");
	break;
    case MARK:
	combo->DataSet("Mark");
	break;
    case EVEN:
	combo->DataSet("Even");
	break;
    case ODD:
	combo->DataSet("Odd");
	break;
    case SPACE_8:
	combo->DataSet("Space_8bits");
	break;
    case MARK_8:
	combo->DataSet("Mark_8bits");
	break;
    case EVEN_8:
	combo->DataSet("Even_8bits");
	break;
    case ODD_8:
	combo->DataSet("Odd_8bits");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR | WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

    InitStopBitList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
    if ( modem )
    switch ( modem->_stopbits )
    {
    case STOPBITS_1_0:
	combo->DataSet("1.0");
	break;
    case STOPBITS_1_5:
	combo->DataSet("1.5");
	break;
    case STOPBITS_2_0:
	combo->DataSet("2.0");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR | WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

    InitFlowList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
    if ( modem )
    switch ( modem->_flow ) {
    case NO_FLOW:
	combo->DataSet("None");
	break;
    case XON_XOFF:
	combo->DataSet("Xon/Xoff");
	break;
    case RTS_CTS:
	combo->DataSet("Rts/Cts");
	break;
    case AUTO_FLOW:
	combo->DataSet("Auto");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR | WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

    InitSpeedList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
    if ( modem ) {
	itoa( modem->_speed, itoabuf, 10 ) ;
	combo->DataSet( itoabuf ) ;
    }
    combo->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_OVER_EC );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_EC );
    if ( modem ) {
    if ( modem->_correction )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    }
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_DC );
    if ( modem ) {
    if ( modem->_compression )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    }
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_CARRIER );
    if ( modem ) {
    if ( modem->_carrier )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    }
    button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    // TCP/IP Page
    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    combo->DataSet("Accept");
    combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
    string->DataSet( "", 40 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_DISP );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_JOB );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    UIW_FORMATTED_STRING * format;
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

    button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    button->woStatus &= ~WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    button->woStatus |= WOS_SELECTED;
    button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);
    button->Information(I_CHANGED_STATUS,NULL);

    // Kerberos Page
    string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
    string->DataSet( "", 256 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
    string->DataSet( "", 64 );
    string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
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

    // Keyboard Settings
    group = (UIW_GROUP *) Get( GROUP_BACKSPACE ) ;
    button = (UIW_BUTTON *) Get( RADIO_BS_DEL ) ;
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

   text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
   text->woFlags |= WOF_VIEW_ONLY ;
   text->Information( I_CHANGED_FLAGS,NULL ) ;

   string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
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

   //Login Settings
   button = (UIW_BUTTON *) Get( CHECK_SET_SCRIPTFILE ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
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

    text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
    text->Information( I_SET_VALUE, "" ) ;
    text->DataSet( "", 3000 ) ;
    text->woFlags &= ~WOF_VIEW_ONLY ;
    text->Information(I_CHANGED_FLAGS, NULL);

    /* Printer page */

    InitPrinterTypeList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINTER_TYPE ) ;
    combo->DataSet("DOS Device");
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL);

    InitPrinterDosList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_DOS_DEVICE ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->woFlags &= ~WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    InitPrinterWindowsList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_WINDOWS ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_PRINT_FORMFEED );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
    string->woFlags |= WOF_AUTO_CLEAR;
    string->woFlags &= ~WOF_NON_SELECTABLE ;
    string->Information( I_CHANGED_FLAGS, NULL);

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT );
    spin_int = (UIW_INTEGER *) spin->Get( INTEGER_PRINT_TIMEOUT ) ;
    new_spin_int = 0 ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;

    button = (UIW_BUTTON *) Get( CHECK_PRINT_BIDI );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    InitPrinterParityList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
    combo->DataSet("None");
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS, NULL);

    InitPrinterFlowList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
    combo->DataSet("None");
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS, NULL);

    InitPrinterSpeedList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
    combo->DataSet( "9600" ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_PRINT_POSTSCRIPT );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = 80 ;
    integer->Information( I_SET_VALUE, &value ) ;

    integer = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = 66 ;
    integer->Information( I_SET_VALUE, &value ) ;

    /* Logs page */
    button = (UIW_BUTTON *) Get( CHECK_LOG_DEBUG ) ;
    button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_DEBUG ) ;
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

    // SSL/TLS Page
    string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
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
    
    combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
    combo->DataSet("Verify host certificates if presented");
    combo->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
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

// KD_LIST_ITEM Constructor for K_SETTINGS_NOTEBOOK
//
//

K_SETTINGS_NOTEBOOK::
K_SETTINGS_NOTEBOOK( KD_LIST_ITEM * entry, enum ENTRYMODE mode )
   : ZAF_DIALOG_WINDOW("WINDOW_SETTINGS",  defaultStorage),
   _entry(entry),
   _original_name(NULL),
   _mode(mode) ,
   _initialized(0)
{
    printf("Entering K_SETTINGS_NOTEBOOK::K_SETTINGS_NOTEBOOK(entry,mode)\n");

    if ( _mode == EDIT_ENTRY )
        _original_name = strdup( _entry->_name ) ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    Information( I_SET_TEXT, "Kermit 95 Connection Entry" ) ;

    K_MODEM * modem = connector->FindModem( entry->_modem );

    strncpy(_phone,entry->_phone_number, 256);
    strncpy(_lat, entry->_lataddress, 256);
    strncpy(_ip, entry->_ipaddress, 256);

    // Apply entry to Notebook
    // General Page
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
    string->DataSet( mode == CLONE_ENTRY ? "" : entry->_name, 28 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    string->DataSet( entry->_location, 20 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
    string->DataSet( entry->_ipport, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
    string->DataSet( entry->_sshport, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
    UIW_BUTTON * button = NULL, * button2=NULL ;
    UIW_GROUP * group;
    switch ( entry->_tcpproto ) {
    case TCP_DEFAULT:
        button = (UIW_BUTTON *) Get( PROTO_DEFAULT );
        break;
    case TCP_EK4LOGIN:
        button = (UIW_BUTTON *) Get( PROTO_EK4LOGIN );
        break;
    case TCP_EK5LOGIN:
        button = (UIW_BUTTON *) Get( PROTO_EK5LOGIN );
        break;
    case TCP_K4LOGIN:
        button = (UIW_BUTTON *) Get( PROTO_K4LOGIN );
        break;
    case TCP_K5LOGIN:
        button = (UIW_BUTTON *) Get( PROTO_K5LOGIN );
        break;
    case TCP_RAW:
        button = (UIW_BUTTON *) Get( PROTO_RAW );
        break;
    case TCP_RLOGIN:
        button = (UIW_BUTTON *) Get( PROTO_RLOGIN );
        break;
    case TCP_TELNET:
        button = (UIW_BUTTON *) Get( PROTO_TELNET );
        break;
    case TCP_TELNET_NO_INIT:
        button = (UIW_BUTTON *) Get( PROTO_TELNET_NO_INIT );
        break;
    case TCP_TELNET_SSL:
        button = (UIW_BUTTON *) Get( PROTO_TELNET_SSL );
        break;
    case TCP_TELNET_TLS:
        button = (UIW_BUTTON *) Get( PROTO_TELNET_TLS );
        break;
    case TCP_SSL:
        button = (UIW_BUTTON *) Get( PROTO_SSL );
        break;
    case TCP_TLS:
        button = (UIW_BUTTON *) Get( PROTO_TLS );
        break;
    }
    *combo + button;

    combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
    switch ( entry->_sshproto ) {
    case SSH_AUTO:
        button = (UIW_BUTTON *) Get( SSH_PROTO_AUTO );
        break;
    case SSH_V1:
        button = (UIW_BUTTON *) Get( SSH_PROTO_V1 );
        break;
    case SSH_V2:
        button = (UIW_BUTTON *) Get( SSH_PROTO_V2 );
        break;
    }
    *combo + button;

    UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
    switch ( entry->_access ) {
    case PHONE:
        _transport = _entry->_access;
	string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_phone_number, 256 ) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
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

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
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

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        break;

    case SSH:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_ipaddress, 256 ) ;

        button = (UIW_BUTTON *) Get( RADIO_IP_ADDRESS ) ;
        prompt->DataSet("Hostname or IP Address:");

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        break;

    case SUPERLAT:
        _transport = _entry->_access;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->DataSet( entry->_lataddress, 256 ) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( RADIO_LAT_ADDRESS ) ;
        prompt->DataSet("Service name or node/port");
        break;

    case TEMPLATE:
        _transport = TEMPLATE;
        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
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

    string = (UIW_STRING *) Get( FIELD_STARTUP_DIR ) ;
    string->DataSet( entry->_startup_dir, 256 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_TEXT * text = (UIW_TEXT *) Get( TEXT_NOTES ) ;
    text->DataSet( entry->_notes, 3000 ) ;

    button = (UIW_BUTTON *) Get( CHECK_TEMPLATE );
    if ( entry->_template && _mode != CLONE_ENTRY )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

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


   // Terminal Page
    InitTermTypeList();
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;
    ZIL_ICHAR * ichar = NULL ;
    switch ( entry->_terminal ) {
    case VT320:
	ichar = "VT320";
	break;
    case VT220:
	ichar = "VT220";
	break;
    case VT102:
	ichar = "VT102";
	break;
    case VT100:
	ichar = "VT100";
	break;
    case VT52:    
	ichar = "VT52";
	break;
    case ANSI:
	ichar = "ANSI-BBS";
	break;
    case TTY:
	ichar = "TTY";
	break;
    case WY30:
	ichar = "WY30";
	break;
    case WY50:
	ichar = "WY50";
	break;
    case WY60:
	ichar = "WY60";
	break;
    case WY160:
	ichar = "WY160";
	break;
    case WY370:
	ichar = "WY370";
	break;
    case DG200:
	ichar = "DG200";
	break;
    case DG210:
	ichar = "DG210";
	break;
    case DG217:
	ichar = "DG217";
	break;
    case SCOANSI:
	ichar = "SCOANSI";
	break;
    case AT386:
	ichar = "AT386";
	break;
    case AVATAR:
	ichar = "AVATAR/0+";
	break;
    case HEATH19:
	ichar = "HEATH19";
	break;
    case HP2621:
	ichar = "HP2621";
	break;
    case HZ1500:
	ichar = "HZ1500";
	break;
    case TVI910:
	ichar = "TVI910+";
	break;
    case TVI925:
	ichar = "TVI925";
	break;
    case TVI950:
	ichar = "TVI950";
	break;
    case VC404:
	ichar = "VC404";
	break;
    case VIP7809:
	ichar = "VIP7809";
	break;
    case HPTERM:
	ichar = "HPTERM";
	break;
    case BEOS:
	ichar = "BEOS-ANSI";
	break;
    case QNX:
	ichar = "QNX";
	break;
    case QANSI:
	ichar = "QANSI";
	break;
    case SNI97801:
	ichar = "SNI-97801";
	break;
    case BA80:
	ichar = "BA80";
	break;
    case AIXTERM:
	ichar = "AIXTERM";
	break;
    case HFT:
	ichar = "HFT";
	break;
    case LINUX:
	ichar = "LINUX";
	break;
    case VTNT:
	ichar = "VTNT";
	break;
    case IBM3151:
	ichar = "IBM3151";
	break;
    case SUN:
	ichar = "SUN";
	break;
    case ANNARBOR:
	ichar = "ANNARBOR";
	break;
    }
    SetTermTypeList( ichar );

   group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
   switch ( entry->_charsize ) {
      case 7:
         button = (UIW_BUTTON *) Get( RADIO_BITS_7 ) ;
         break;
      case 8:
         button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
         break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
   *group + button ;

   button = (UIW_BUTTON *) Get( CHECK_LOCAL_ECHO );
   if ( entry->_local_echo )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   button = (UIW_BUTTON *) Get( CHECK_AUTODOWNLOAD );
   if ( entry->_auto_download )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   button = (UIW_BUTTON *) Get( CHECK_AUTO_WRAP );
   if ( entry->_auto_wrap )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   button = (UIW_BUTTON *) Get( CHECK_APC );
   if ( entry->_apc_enabled )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   InitWidthList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
   ZIL_ICHAR itoabuf[40] ;
   itoa(entry->_width, itoabuf, 10) ;
   combo->DataSet( itoabuf ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitHeightList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
   itoa(entry->_height, itoabuf, 10) ;
   combo->DataSet( itoabuf ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE );
    if ( entry->_status_line )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    InitCursorList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_CURSOR ) ;
   SetComboToCursor( combo, entry->_cursor ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_SCROLLBACK ) ;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( SPIN_INT_SCROLLBACK ) ;
    int new_spin_int = entry->_scrollback ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;

   InitColorLists() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
   SetComboToColor( combo, entry->_color_term_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
   SetComboToColor( combo, entry->_color_term_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_FG ) ;
   SetComboToColor( combo, entry->_color_status_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_BG ) ;
   SetComboToColor( combo, entry->_color_status_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_FG ) ;
   SetComboToColor( combo, entry->_color_popup_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_BG ) ;
   SetComboToColor( combo, entry->_color_popup_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_FG ) ;
   SetComboToColor( combo, entry->_color_select_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_BG ) ;
   SetComboToColor( combo, entry->_color_select_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
   SetComboToColor( combo, entry->_color_underline_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
   SetComboToColor( combo, entry->_color_underline_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_FG ) ;
   SetComboToColor( combo, entry->_color_command_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_BG ) ;
   SetComboToColor( combo, entry->_color_command_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitTermCharsetList( (UIW_VT_LIST *) Get( LIST_TERM_CHARSET ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
   SetComboToCharset( combo, entry->_term_charset );
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   //File Transfer
    combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
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
    if ( button ) {
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
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    spin_int = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
    new_spin_int = entry->_packet_length ;
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
    spin->Information(I_CHANGED_FLAGS,NULL) ;

   group = (UIW_GROUP *) Get( GROUP_SET_TRANSFER_MODE ) ;
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

   button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_INCOMPLETE ) ;
   if ( entry->_keep_incomplete )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_STREAMING ) ;
    if ( entry->_streaming )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_CLEAR_CHANNEL ) ;
    if ( entry->_clear_channel )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

  button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_PATH ) ;
   if ( entry->_pathnames )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   button = (UIW_BUTTON *) Get( CHECK_SET_TRANSFER_FNLIT ) ;
   if ( entry->_fname_literal )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   InitFileCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_FCS ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_FCS ) ;
   SetComboToCharset( combo, entry->_file_charset ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   InitXferCharsetList( (UIW_VT_LIST *) Get( LIST_SET_TRANSFER_TCS ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_SET_TRANSFER_TCS ) ;
   SetComboToCharset( combo, entry->_xfer_charset ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;

   // Communications
    InitModemList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
    combo->DataSet( entry->_modem ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || 
	 _entry->_access == SUPERLAT ||
	 _entry->_access == DIRECT )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    InitLineList( 0 ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
    combo->DataSet( (!modem || entry->_access == DIRECT) ? entry->_line_device : modem->_port ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY );
    if ( !_entry->_use_mdm_parity )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    InitParityList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
    switch ( (modem && _entry->_use_mdm_parity && _entry->_access != DIRECT) ? 
	     modem->_parity : _entry->_parity )
    {
    case NO_PARITY:
	combo->DataSet("None");
	break;
    case SPACE:
	combo->DataSet("Space");
	break;
    case MARK:
	combo->DataSet("Mark");
	break;
    case EVEN:
	combo->DataSet("Even");
	break;
    case ODD:
	combo->DataSet("Odd");
	break;
    case SPACE_8:
	combo->DataSet("Space_8bits");
	break;
    case MARK_8:
	combo->DataSet("Mark_8bits");
	break;
    case EVEN_8:
	combo->DataSet("Even_8bits");
	break;
    case ODD_8:
	combo->DataSet("Odd_8bits");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_parity && _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS );
    if ( !_entry->_use_mdm_stopbits )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    InitStopBitList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
    switch ( (modem && _entry->_use_mdm_stopbits && _entry->_access != DIRECT) ? 
	     modem->_stopbits : _entry->_stopbits )
    {
    case STOPBITS_1_0:
	combo->DataSet("1.0");
	break;
    case STOPBITS_1_5:
	combo->DataSet("1.5");
	break;
    case STOPBITS_2_0:
	combo->DataSet("2.0");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_parity && _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);


    button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW );
    if ( !_entry->_use_mdm_flow )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    InitFlowList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
    switch ( (modem && _entry->_use_mdm_flow && _entry->_access != DIRECT) ? 
	     modem->_flow : _entry->_flow )
    {
    case NO_FLOW:
	combo->DataSet("None");
	break;
    case XON_XOFF:
	combo->DataSet("Xon/Xoff");
	break;
    case RTS_CTS:
	combo->DataSet("Rts/Cts");
	break;
    case AUTO_FLOW:
	combo->DataSet("Auto");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_flow && _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED );
    if ( !_entry->_use_mdm_speed )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    InitSpeedList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
    if ( modem && _entry->_use_mdm_speed && _entry->_access != DIRECT )
	itoa( modem->_speed, itoabuf, 10 ) ;
    else
	itoa( entry->_speed, itoabuf, 10 ) ;
    combo->DataSet( itoabuf ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_speed && _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_OVER_EC );
    if ( !_entry->_use_mdm_ec )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_EC ) ;
    if ( modem && _entry->_use_mdm_ec && _entry->_access != DIRECT )
    {
    	if ( modem->_correction )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    else 
    {
    	if ( _entry->_correction )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_ec && _entry->_access == PHONE )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_DC );
    if ( !_entry->_use_mdm_dc )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_DC ) ;
    if ( modem && _entry->_use_mdm_dc && _entry->_access != DIRECT )
    {
    	if ( modem->_compression )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    else 
    {
    	if ( _entry->_compression )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_dc && _entry->_access == PHONE )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER );
    if ( !_entry->_use_mdm_carrier )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || _entry->_access == SUPERLAT || _entry->_access == DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
    if ( modem && _entry->_use_mdm_carrier && _entry->_access == PHONE )
    {
    	if ( modem->_carrier )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    else 
    {
    	if ( _entry->_carrier )
	    button->woStatus |= WOS_SELECTED;
	else
	    button->woStatus &= ~WOS_SELECTED;
    }
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access != PHONE && _entry->_access != DIRECT )
	button->woFlags |= WOF_NON_SELECTABLE ;
    else if ( _entry->_use_mdm_carrier && _entry->_access == PHONE )
	button->woFlags |= WOF_NON_SELECTABLE ;
    button->Information(I_CHANGED_FLAGS,NULL);

    // TCP/IP Page
    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    switch ( entry->_telnet_auth_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    switch ( entry->_telnet_binary_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    switch ( entry->_telnet_encrypt_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    switch ( entry->_telnet_kermit_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    switch ( entry->_telnet_starttls_mode ) {
    case TelnetAccept:
        combo->DataSet("Accept");
        break;
    case TelnetRefuse:
        combo->DataSet("Refuse");
        break;
    case TelnetRequest:
        combo->DataSet("Request");
        break;
    case TelnetRequire:
        combo->DataSet("Require");
        break;
    }
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
    string->DataSet( entry->_telnet_location, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0)
	| WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
    string->DataSet( entry->_telnet_ttype, 40 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0)
	| WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
    string->DataSet( entry->_telnet_acct, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_DISP );
    string->DataSet( entry->_telnet_disp, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_JOB );
    string->DataSet( entry->_telnet_job, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
    string->DataSet( entry->_telnet_printer, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    if ( entry->_default_ip_address ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    UIW_FORMATTED_STRING * format;
    format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
    format->DataSet(entry->_tcp_ip_address);
    format->woFlags |= ((entry->_access != TCPIP ||
                          entry->_default_ip_address) ? WOF_NON_SELECTABLE : 0);
    format->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_SENDBUF ) ;
    new_spin_int = entry->_tcp_sendbuf ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    spin->Information( I_CHANGED_FLAGS, NULL );

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_RECVBUF ) ;
    new_spin_int = entry->_tcp_recvbuf ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    spin->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
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
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
    if ( entry->_tcp_dns_srv ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( connector->_dns_srv_avail )
        button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    else
        button->woFlags |= WOF_NON_SELECTABLE;
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    if ( entry->_fwd_cred ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    if ( entry->_telnet_wait ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    if ( entry->_telnet_debug ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    if ( entry->_telnet_sb_delay ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    // Kerberos
    string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
    string->DataSet( entry->_k4_realm, 256 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
    string->DataSet( entry->_k4_princ, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
    if ( entry->_k4_autoget ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
    if ( entry->_k4_autodestroy ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    UIW_INTEGER * integer = NULL ;
    integer = (UIW_INTEGER *) Get( INTEGER_K4_LIFETIME);
    integer->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    int value = entry->_k4_lifetime;
    integer->Information( I_SET_VALUE, &value ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
    string->DataSet( entry->_k5_realm, 256 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
    string->DataSet( entry->_k5_princ, 64 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
    if ( entry->_k5_autoget ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
    if ( entry->_k5_autodestroy ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    integer = (UIW_INTEGER *) Get( INTEGER_K5_LIFETIME);
    integer->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = entry->_k5_lifetime;
    integer->Information( I_SET_VALUE, &value ) ;

    button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
    if ( entry->_k5_forwardable ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
    if ( entry->_k5_getk4tkt ) 
        button->woStatus |= WOS_SELECTED;
    else 
        button->woStatus &= ~WOS_SELECTED;
    button->Information(I_CHANGED_STATUS,NULL);
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
    string->DataSet( entry->_k5_cache, 256 );
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information( I_CHANGED_FLAGS, NULL );


   // Keyboard Settings
   group = (UIW_GROUP *) Get( GROUP_BACKSPACE ) ;
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
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

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
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

   group = (UIW_GROUP *) Get( GROUP_MOUSE ) ;
   button = (UIW_BUTTON *) Get( entry->_mouse ? RADIO_MOUSE_ON : RADIO_MOUSE_OFF ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
   *group + button ;

   text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
   text->woFlags |= WOF_VIEW_ONLY ;
   text->Information( I_CHANGED_FLAGS,NULL ) ;

   string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
   string->DataSet( entry->_keymap_file ) ;

   group = (UIW_GROUP *) Get( GROUP_KEYMAP ) ;
   switch ( entry->_keymap ) {
      case VT100_KEY:
         button = (UIW_BUTTON *) Get( RADIO_KEYMAP_DEFAULT ) ;
         string->woFlags |= WOF_NON_SELECTABLE ;
         string->Information(I_CHANGED_FLAGS,NULL) ;
	  button2 = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
	  button2->woFlags |= WOF_NON_SELECTABLE ;
	  button2->Information( I_CHANGED_FLAGS, NULL ) ;
	  break;
      case MAPFILE:
         button = (UIW_BUTTON *) Get( RADIO_KEYMAP_FILE ) ;
         LoadFileIntoText( text, string->DataGet() ) ;
	  button2 = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
	  button2->woFlags &= ~WOF_NON_SELECTABLE ;
	  button2->Information( I_CHANGED_FLAGS, NULL ) ;
	  break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

   //Login Settings
   button = (UIW_BUTTON *) Get( CHECK_SET_SCRIPTFILE ) ;
   if ( entry->_script_file )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;

   string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
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
    string->Information(I_CHANGED_FLAGS,NULL);

    string = (UIW_STRING *) Get( FIELD_HOST_PROMPT ) ;
    string->DataSet( entry->_prompt ) ;

   text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
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


    /* Printer page */
    InitPrinterTypeList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINTER_TYPE ) ;
    switch ( entry->_printer_type ) {
    case PrinterDOS:
        combo->DataSet("DOS Device");
        break;
    case PrinterWindows:
        combo->DataSet("Windows Printer Queue");
        break;
    case PrinterFile:
        combo->DataSet("File");
        break;
    case PrinterPipe:
        combo->DataSet("Command (Pipe)");
        break;
    case PrinterNone:
        combo->DataSet("None");
        break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL);

    InitPrinterDosList( entry->_printer_dos );
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_DOS_DEVICE ) ;
    combo->DataSet( entry->_printer_dos ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_printer_type != PrinterWindows )
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    InitPrinterWindowsList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_WINDOWS ) ;
    combo->DataSet( entry->_printer_windows ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_printer_type == PrinterWindows )
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
    else
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
    string->DataSet( entry->_printer_separator);
    string->woFlags |= WOF_AUTO_CLEAR;
    string->Information( I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_PRINT_FORMFEED );
    if ( entry->_printer_formfeed )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
    string->DataSet( entry->_printer_terminator);
    string->woFlags |= WOF_AUTO_CLEAR;
    if ( !entry->_printer_formfeed )
	string->woFlags &= ~WOF_NON_SELECTABLE ;
    else
	string->woFlags |= WOF_NON_SELECTABLE ;
    string->Information( I_CHANGED_FLAGS, NULL);

    spin = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT ) ;
    spin_int = (UIW_INTEGER *) spin->Get( INTEGER_PRINT_TIMEOUT ) ;
    new_spin_int = entry->_printer_timeout ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;

    button = (UIW_BUTTON *) Get( CHECK_PRINT_BIDI );
    if ( entry->_printer_bidi )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    InitPrinterParityList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
    switch ( entry->_printer_parity )
    {
    case NO_PARITY:
	combo->DataSet("None");
	break;
    case SPACE:
	combo->DataSet("Space");
	break;
    case MARK:
	combo->DataSet("Mark");
	break;
    case EVEN:
	combo->DataSet("Even");
	break;
    case ODD:
	combo->DataSet("Odd");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_printer_type != PrinterDOS )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else 
        combo->woFlags &= ~WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    InitPrinterFlowList();
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
    switch ( entry->_printer_flow )
    {
    case NO_FLOW:
	combo->DataSet("None");
	break;
    case XON_XOFF:
	combo->DataSet("Xon/Xoff");
	break;
    case RTS_CTS:
	combo->DataSet("Rts/Cts");
	break;
    case AUTO_FLOW:
	combo->DataSet("Auto");
	break;
    }
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_printer_type != PrinterDOS )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else 
        combo->woFlags &= ~WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS, NULL);

    InitPrinterSpeedList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
    itoa( entry->_printer_speed, itoabuf, 10 ) ;
    combo->DataSet( itoabuf ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( entry->_printer_type != PrinterDOS )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    else 
        combo->woFlags &= ~WOF_NON_SELECTABLE;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_PRINT_POSTSCRIPT );
    if ( entry->_printer_ps )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH);
    new_spin_int = entry->_printer_width;
    integer->Information( I_SET_VALUE, &new_spin_int ) ;
    if ( !entry->_printer_ps )
	integer->woFlags |= WOF_NON_SELECTABLE ;
    else 
        integer->woFlags &= ~WOF_NON_SELECTABLE;
    integer->Information(I_CHANGED_FLAGS,NULL) ;

    integer = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH);
    new_spin_int = entry->_printer_length;
    integer->Information( I_SET_VALUE, &new_spin_int ) ;
    if ( !entry->_printer_ps )
	integer->woFlags |= WOF_NON_SELECTABLE ;
    else 
        integer->woFlags &= ~WOF_NON_SELECTABLE;
    integer->Information(I_CHANGED_FLAGS,NULL) ;

    /* Logs page */
    button = (UIW_BUTTON *) Get( CHECK_LOG_CONNECTION ) ;
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

    string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
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

    button2 = (UIW_BUTTON *) Get( CHECK_LOG_TRANSACTIONS_BRIEF ) ;
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


    // SSL/TLS Page
    string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
    string->DataSet(entry->_tls_cipher) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
    string->DataSet(entry->_tls_cert_file) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
    string->DataSet(entry->_tls_key_file) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
    string->DataSet(entry->_tls_verify_file) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
    string->DataSet(entry->_tls_verify_dir) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
    string->DataSet(entry->_tls_crl_file) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
    string->DataSet(entry->_tls_crl_dir) ;
    string->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0) | WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;
    
    combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
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
    combo->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_tls_verbose )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
    button->woFlags |= ((entry->_access != TCPIP) ? WOF_NON_SELECTABLE : 0);
    button->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_tls_debug )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    // If a clone, get rid of the original
   if ( _mode == CLONE_ENTRY )
      _entry = new KD_LIST_ITEM ;

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_SETTINGS_NOTEBOOK::K_SETTINGS_NOTEBOOK(entry,mode)\n");
};

K_SETTINGS_NOTEBOOK::~K_SETTINGS_NOTEBOOK(void)
{
   if ( _original_name )
      delete _original_name ;
   _original_name = NULL;
}

EVENT_TYPE K_SETTINGS_NOTEBOOK::Event( const UI_EVENT & event )
{
    printf("K_SETTINGS_NOTEBOOK::Event()\n");

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
    ZIL_ICHAR itoabuf[40] ;
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

    case OPT_LINE_DEVICE_IS_PORT: {
	break;
    }

    case OPT_MODEM_IS_MODEM: {
	// need to copy modem values to those fields that
	// are using defaults

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES );
	modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	if ( modem ) {
	    combo->DataSet( modem->_port ) ;
	}
	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED );
	if ( !FlagSet(Get( CHECK_OVER_SPEED )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		itoa( modem->_speed, itoabuf, 10 ) ;
		combo->DataSet( itoabuf ) ;
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );
    	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW );
	if ( !FlagSet(Get( CHECK_OVER_FLOW )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		switch ( modem->_flow ) {
		case NO_FLOW:
		    combo->DataSet("None");
		    break;
		case XON_XOFF:
		    combo->DataSet("Xon/Xoff");
		    break;
		case RTS_CTS:
		    combo->DataSet("Rts/Cts");
		    break;
		case AUTO_FLOW:
		    combo->DataSet("Auto");
		    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );

    	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	if ( !FlagSet(Get( CHECK_OVER_PARITY )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		switch ( modem->_parity )
		{
		case NO_PARITY:
		    combo->DataSet("None");
		    break;
		case SPACE:
		    combo->DataSet("Space");
		    break;
		case MARK:
		    combo->DataSet("Mark");
		    break;
		case EVEN:
		    combo->DataSet("Even");
		    break;
		case ODD:
		    combo->DataSet("Odd");
		    break;
		case SPACE_8:
		    combo->DataSet("Space_8bit");
		    break;
		case MARK_8:
		    combo->DataSet("Mark_8bit");
		    break;
		case EVEN_8:
		    combo->DataSet("Even_8bit");
		    break;
		case ODD_8:
		    combo->DataSet("Odd_8bit");
		    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );

    	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS );
	if ( !FlagSet(Get( CHECK_OVER_STOPBITS )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		switch ( modem->_stopbits )
		{
                case STOPBITS_1_0:
                    combo->DataSet("1.0");
                    break;
                case STOPBITS_1_5:
                    combo->DataSet("1.5");
                    break;
                case STOPBITS_2_0:
                    combo->DataSet("2.0");
                    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get( CHECK_EC );
	if ( !FlagSet(Get( CHECK_OVER_EC )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		if ( modem->_correction )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	button = (UIW_BUTTON *) Get( CHECK_DC );
	if ( !FlagSet(Get( CHECK_OVER_DC )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		if ( modem->_compression )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	button = (UIW_BUTTON *) Get( CHECK_CARRIER );
	if ( !FlagSet(Get( CHECK_OVER_CARRIER )->woStatus, WOS_SELECTED ) ) {
	    if ( modem ) {
		if ( modem->_carrier )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	break;
    }

    case OPT_MODEM_IS_TAPI:
    case OPT_LINE_DEVICE_IS_TAPI: {
	break;
    }

    case OPT_USE_MODEM_SPEED:
	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED );
	if ( !FlagSet(Get( CHECK_OVER_SPEED )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		itoa( modem->_speed, itoabuf, 10 ) ;
		combo->DataSet( itoabuf ) ;
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );
 	break;

    case OPT_USE_MODEM_FLOW:
    	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW );
	if ( !FlagSet(Get( CHECK_OVER_FLOW )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		switch ( modem->_flow ) {
		case NO_FLOW:
		    combo->DataSet("None");
		    break;
		case XON_XOFF:
		    combo->DataSet("Xon/Xoff");
		    break;
		case RTS_CTS:
		    combo->DataSet("Rts/Cts");
		    break;
		case AUTO_FLOW:
		    combo->DataSet("Auto");
		    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_USE_MODEM_PARITY:
    	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	if ( !FlagSet(Get( CHECK_OVER_PARITY )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		switch ( modem->_parity )
		{
		case NO_PARITY:
		    combo->DataSet("None");
		    break;
		case SPACE:
		    combo->DataSet("Space");
		    break;
		case MARK:
		    combo->DataSet("Mark");
		    break;
		case EVEN:
		    combo->DataSet("Even");
		    break;
		case ODD:
		    combo->DataSet("Odd");
		    break;
		case SPACE_8:
		    combo->DataSet("Space_8bit");
		    break;
		case MARK_8:
		    combo->DataSet("Mark_8bit");
		    break;
		case EVEN_8:
		    combo->DataSet("Even_8bit");
		    break;
		case ODD_8:
		    combo->DataSet("Odd_8bit");
		    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_USE_MODEM_STOPBITS:
    	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS );
	if ( !FlagSet(Get( CHECK_OVER_STOPBITS )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		switch ( modem->_stopbits )
		{
                case STOPBITS_1_0:
                    combo->DataSet("1.0");
                    break;
                case STOPBITS_1_5:
                    combo->DataSet("1.5");
                    break;
                case STOPBITS_2_0:
                    combo->DataSet("2.0");
                    break;
		}
	    }
	    combo->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    combo->woFlags &= ~WOF_NON_SELECTABLE;
	combo->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_USE_MODEM_EC:
	button = (UIW_BUTTON *) Get( CHECK_EC );
	if ( !FlagSet(Get( CHECK_OVER_EC )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		if ( modem->_correction )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_USE_MODEM_DC:
	button = (UIW_BUTTON *) Get( CHECK_DC );
	if ( !FlagSet(Get( CHECK_OVER_DC )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		if ( modem->_compression )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_USE_MODEM_CARRIER:
	button = (UIW_BUTTON *) Get( CHECK_CARRIER );
	if ( !FlagSet(Get( CHECK_OVER_CARRIER )->woStatus, WOS_SELECTED ) ) {
	    modem = connector->FindModem( ((UIW_COMBO_BOX *) Get(COMBO_MODEM))->DataGet() );
	    if ( modem ) {
		if ( modem->_carrier )
		    button->woStatus |= WOS_SELECTED;
		else
		    button->woStatus &= ~WOS_SELECTED;
                button->Information(I_CHANGED_STATUS,NULL);
	    }
	    button->woFlags |= WOF_NON_SELECTABLE;
	}
	else
	    button->woFlags &= ~WOF_NON_SELECTABLE;
	button->Information( I_CHANGED_FLAGS, NULL );
	break;

    case OPT_HELP_SET_GENERAL:
        helpSystem->DisplayHelp( windowManager, HELP_SET_GENERAL ) ;
        break;

    case OPT_HELP_SET_TERMINAL:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TERMINAL ) ;
        break;

    case OPT_HELP_SET_FILE_TRANSFER:
        helpSystem->DisplayHelp( windowManager, HELP_SET_FILE_TRANSFER ) ;
        break;

    case OPT_HELP_SET_COMMUNICATIONS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_COMMUNICATIONS ) ;
        break;

    case OPT_HELP_SET_KEYBOARD:
        helpSystem->DisplayHelp( windowManager, HELP_SET_KEYBOARD ) ;
        break;

    case OPT_HELP_SET_LOGIN:
        helpSystem->DisplayHelp( windowManager, HELP_SET_LOGIN ) ;
        break;

    case OPT_HELP_SET_PRINTER:
        helpSystem->DisplayHelp( windowManager, HELP_SET_PRINTER ) ;
        break;

    case OPT_HELP_SET_KERBEROS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_KERBEROS ) ;
        break;

    case OPT_HELP_SET_TLS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TLS ) ;
        break;

    case OPT_HELP_SET_LOGS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_LOGS ) ;
        break;

    case OPT_HELP_SET_TCPIP:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TCPIP ) ;
        break;

    case OPT_SET_OK: {
        UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
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

       // Widths must be even
       UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
       ZIL_INT8 width = atoi(combo->DataGet()) ;

       if ( width % 2 == 1 ) {
	   ZAF_MESSAGE_WINDOW * message =
	       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, 
				       ZIL_MSG_OK,
				       "Terminal width must be an Even value." ) ;
	   message->Control();
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

	   connector->AddEntry( _entry ) ;
       }
       break;
    }
    case OPT_PHONE_CALL: {
        if ( _transport == PHONE )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Phone number:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        switch ( _transport ) {
        case SSH:
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
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

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_DISP );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_JOB );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        format->woFlags |= WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K4_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K5_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

	combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
	K_MODEM * modem = connector->FindModem( combo->DataGet() );
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS,NULL) ;

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
	if ( modem )
	    combo->DataSet( modem->_port );
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
	if ( modem ) {
	    ZIL_ICHAR itoabuf[40] ;
	    itoa( modem->_speed, itoabuf, 10 ) ;
	    combo->DataSet( itoabuf );
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    combo->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
	if ( modem )
	switch ( modem->_flow ) {
	case NO_FLOW:
	    combo->DataSet("None");
	    break;
	case XON_XOFF:
	    combo->DataSet("Xon/Xoff");
	    break;
	case RTS_CTS:
	    combo->DataSet("Rts/Cts");
	    break;
	case AUTO_FLOW:
	    combo->DataSet("Auto");
	    break;
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    combo->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
	if ( modem )
	switch ( modem->_parity )
	{
	case NO_PARITY:
	    combo->DataSet("None");
	    break;
	case SPACE:
	    combo->DataSet("Space");
	    break;
	case MARK:
	    combo->DataSet("Mark");
	    break;
	case EVEN:
	    combo->DataSet("Even");
	    break;
	case ODD:
	    combo->DataSet("Odd");
	    break;
	case SPACE_8:
	    combo->DataSet("Space_8bit");
	    break;
	case MARK_8:
	    combo->DataSet("Mark_8bit");
	    break;
	case EVEN_8:
	    combo->DataSet("Even_8bit");
	    break;
	case ODD_8:
	    combo->DataSet("Odd_8bit");
	    break;
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    combo->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
	if ( modem )
	switch ( modem->_stopbits )
	{
        case STOPBITS_1_0:
            combo->DataSet("1.0");
            break;
        case STOPBITS_1_5:
            combo->DataSet("1.5");
            break;
        case STOPBITS_2_0:
            combo->DataSet("2.0");
            break;
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    combo->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_EC ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_EC ) ;
	if ( modem ) {
	if ( modem->_correction )
	    button2->woStatus |= WOS_SELECTED ;
	else
	    button2->woStatus &= ~WOS_SELECTED ;
	}
        button2->Information(I_CHANGED_STATUS,NULL);
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    button2->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    button2->woFlags &= ~WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_DC ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_DC ) ;
	if ( modem ) {
            if ( modem->_compression )
                button2->woStatus |= WOS_SELECTED ;
            else
                button2->woStatus &= ~WOS_SELECTED ;
            button2->Information(I_CHANGED_STATUS,NULL);
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    button2->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    button2->woFlags &= ~WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER ) ;
	button->woFlags &= ~WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
	if ( modem ) {
            if ( modem->_carrier )
                button2->woStatus |= WOS_SELECTED ;
            else
                button2->woStatus &= ~WOS_SELECTED ;
            button2->Information(I_CHANGED_STATUS,NULL);
	}
	if ( !FlagSet(button->woStatus, WOS_SELECTED ) ) 
	    button2->woFlags |= WOF_NON_SELECTABLE ;
	else 
	    button2->woFlags &= ~WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
        break;
    }

    case OPT_TELNET: {
        if ( _transport == TCPIP )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Hostname or IP Address:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        }
        string->DataSet(_ip,256);
        _transport = TCPIP;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_DISP );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_JOB );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        if ( !(button->woStatus & WOS_SELECTED) )
            format->woFlags &= ~WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
        spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
        spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        if ( connector->_dns_srv_avail ) {
            button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
            button->woFlags &= ~WOF_NON_SELECTABLE;
            button->Information(I_CHANGED_FLAGS,NULL) ;
        }

        button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K4_LIFETIME);
        integer->woFlags &= ~WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K5_LIFETIME);
        integer->woFlags &= ~WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

	combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS,NULL) ;

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_EC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_EC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_DC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_DC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
       break;
    }
    case OPT_SSH: {
        if ( _transport == SSH )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Hostname or IP Address:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        }
        string->DataSet(_ip,256);
        _transport = SSH;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

	string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;
        string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_DISP );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_JOB );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        if ( !(button->woStatus & WOS_SELECTED) )
            format->woFlags &= ~WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
        spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
        spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        if ( connector->_dns_srv_avail ) {
            button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
            button->woFlags &= ~WOF_NON_SELECTABLE;
            button->Information(I_CHANGED_FLAGS,NULL) ;
        }

        button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K4_LIFETIME);
        integer->woFlags &= ~WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K5_LIFETIME);
        integer->woFlags &= ~WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

	combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS,NULL) ;

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_EC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_EC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_DC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_DC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
       break;
    }
    case OPT_SUPERLAT: {
        if ( _transport == SUPERLAT )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("Service name or node/port:");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SSH:
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            break;
        }
        string->DataSet(_lat,256);
        string->woFlags &= ~WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);
        _transport = SUPERLAT;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_DISP );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_JOB );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        format->woFlags |= WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K4_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K5_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

	combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS,NULL) ;

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_EC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_EC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_DC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_DC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
       break;
    }
    case OPT_DIRECT: {
        if ( _transport == DIRECT )
            break;

        UIW_PROMPT * prompt = (UIW_PROMPT *) Get( PROMPT_ADDRESS );
        prompt->DataSet("");

        string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
        switch ( _transport ) {
        case PHONE:
            strncpy( _phone, string->DataGet(), 256 );
            break;
        case SUPERLAT:
            strncpy( _lat, string->DataGet(), 256 );
            break;
        case SSH:
        case TCPIP:
            strncpy( _ip, string->DataGet(), 256 );
            break;
        }
        string->DataSet("",256);
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL);
        _transport = DIRECT;

        string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
        string->woFlags |= WOF_NON_SELECTABLE ;
        string->Information(I_CHANGED_FLAGS, NULL);

	string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
	string->woFlags |= WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL );
	combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        string = (UIW_STRING *) Get( TEXT_TCP_LOCATION );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_ACCT );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_DISP );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_JOB );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_PRINTER );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS );
        format->woFlags |= WOF_NON_SELECTABLE;
        format->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information( I_CHANGED_FLAGS, NULL );

        combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K4_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_REALM );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        string = (UIW_STRING *) Get( TEXT_TCP_K5_PRINC );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTOGET);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_AUTODESTROY);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        integer = (UIW_INTEGER *) Get(INTEGER_K5_LIFETIME);
        integer->woFlags |= WOF_NON_SELECTABLE;
        integer->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_FORWARDABLE);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_K5_GETK4);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        string = (UIW_STRING *) Get( TEXT_TCP_K5_CACHE );
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information( I_CHANGED_FLAGS, NULL );

	combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
	combo->woFlags |= WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS,NULL) ;

	combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
       combo->woFlags &= ~WOF_NON_SELECTABLE ;
       combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_SPEED ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_FLOW ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW ) ;
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY ) ;
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_STOPBITS ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS ) ;
	combo->woFlags &= ~WOF_NON_SELECTABLE ;
	combo->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_EC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_EC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_DC ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_DC ) ;
	button2->woFlags |= WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

	button = (UIW_BUTTON *) Get( CHECK_OVER_CARRIER ) ;
	button->woFlags |= WOF_NON_SELECTABLE ;
	button->Information(I_CHANGED_FLAGS, NULL);

	button2 = (UIW_BUTTON *) Get( CHECK_CARRIER ) ;
	button2->woFlags &= ~WOF_NON_SELECTABLE ;
	button2->Information(I_CHANGED_FLAGS, NULL);

        string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CERT_KEY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_VERIFY_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_FILE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        string = (UIW_STRING *) Get( TEXT_TLS_CRL_DIR ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS,NULL) ;
    
        combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;

        button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL) ;
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

   case OPT_VT100_KEYMAP:
       string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
       string->woFlags |= WOF_NON_SELECTABLE ;
       string->Information(I_CHANGED_FLAGS,NULL) ;
       text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
       text->DataSet( "" ) ;
       button = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
       button->woFlags |= WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

   case OPT_FILE_KEYMAP:
       string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
       string->woFlags &= ~WOF_NON_SELECTABLE ;
       string->Information(I_CHANGED_FLAGS,NULL) ;
       text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
       LoadFileIntoText( text, string->DataGet() ) ;
       button = (UIW_BUTTON *) Get( BUTTON_BROWSE_KEYMAP ) ;
       button->woFlags &= ~WOF_NON_SELECTABLE ;
       button->Information( I_CHANGED_FLAGS, NULL ) ;
       break;

   case	OPT_RADIO_ANSI:
   case OPT_RADIO_AVATAR:
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       combo->DataSet( "transparent" ) ;
       button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	combo->DataSet("None");
       button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
       button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus |= WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "24" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Black" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, T_TRANSPARENT );
       break;

    case OPT_RADIO_SCOANSI:
    case OPT_RADIO_AT386:
    case OPT_RADIO_QNX:
    case OPT_RADIO_QANSI:
    case OPT_RADIO_BEOS:
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       combo->DataSet( "transparent" ) ;
       button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	combo->DataSet("None");
       button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
       button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus &= ~WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "25" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Black" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, T_TRANSPARENT );
       break;

    case OPT_RADIO_SUN:
    case OPT_RADIO_ANNARBOR:
    case OPT_RADIO_LINUX:
    case OPT_RADIO_AIXTERM:
    case OPT_RADIO_HFT:
    case OPT_RADIO_WY370:
    case OPT_RADIO_VT320:
    case OPT_RADIO_VT220:
    case OPT_RADIO_SNI97801:
       button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	combo->DataSet("None");
       button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
       button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus |= WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "24" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Blue" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, event.type == OPT_RADIO_LINUX ? T_CP437 : T_LATIN1_ISO );
       break;

   case	OPT_RADIO_WY30:
   case OPT_RADIO_WY50:
   case OPT_RADIO_WY60:
   case OPT_RADIO_WY160:
   case OPT_RADIO_TVI910:
   case OPT_RADIO_TVI925:
   case OPT_RADIO_TVI950:
   case OPT_RADIO_HEATH19:
   case OPT_RADIO_HZ1500:
   case OPT_RADIO_HP2621:
   case OPT_RADIO_HPTERM:
   case OPT_RADIO_VC404:
   case OPT_RADIO_IBM3151:
       button = (UIW_BUTTON *) Get( RADIO_BITS_7 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
       button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus |= WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "24" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Blue" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, T_ASCII );
       break;

   case OPT_RADIO_DG200:
   case OPT_RADIO_DG210:
   case OPT_RADIO_DG217:
	button = (UIW_BUTTON *) Get( RADIO_BITS_7 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
       button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus |= WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "24" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Blue" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, T_DG_INTERNATIONAL );
       break;

    case OPT_RADIO_BA80:
    case OPT_RADIO_VIP7809:
    case OPT_RADIO_VT102:
    case OPT_RADIO_VT100:
    case OPT_RADIO_VT52:
    case OPT_RADIO_TTY:
       button = (UIW_BUTTON *) Get( RADIO_BITS_7 ) ;
       group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
       button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
       button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE ) ;
       button->woStatus |= WOS_SELECTED ;
       button->Information(I_CHANGED_STATUS,NULL);
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
       combo->DataSet( "24" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
       combo->DataSet( "Blue" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
       combo->DataSet( "LightGray" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
       combo->DataSet( "Red" ) ;
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       SetComboToCharset( combo, T_ASCII );
       break;

   case OPT_RADIO_CHARSIZE_8:
	combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
	combo->DataSet("None");
        button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY ) ;
        button->woStatus &= ~WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        break;

   case OPT_BROWSE_SCRIPT:
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

   case -OPT_FILE_OPEN:
       switch ( DirServRequestor ) {
       case OPT_BROWSE_SCRIPT:
	   string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
	   string->Information(I_SET_TEXT,event.text);
	   text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
	   LoadFileIntoText( text, string->DataGet() ) ;
	   break;
       case OPT_BROWSE_KEYMAP:
	   string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
	   string->Information(I_SET_TEXT,event.text);
	   text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );
	   LoadFileIntoText( text, string->DataGet() ) ;
	   break;
       }
       break;

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

    case OPT_CHECK_PASSWORD_PROMPT:
	string = (UIW_STRING *) Get( FIELD_PASSWD ) ;
	if ( FlagSet(Get( CHECK_PROMPT_FOR_PASSWORD )->woStatus, WOS_SELECTED ) )
	    string->woFlags |= WOF_NON_SELECTABLE ;
	else 	
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL);
        break;

    case OPT_CHECK_PRINT_BIDI:
        // no longer used
        break;

    case OPT_CHECK_POSTSCRIPT:
	if ( FlagSet(Get( CHECK_PRINT_POSTSCRIPT )->woStatus, WOS_SELECTED ) ) {
            integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH );
            integer->woFlags &= ~WOF_NON_SELECTABLE;
            integer->Information(I_CHANGED_FLAGS,NULL);
            integer = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH );
            integer->woFlags &= ~WOF_NON_SELECTABLE;
            integer->Information(I_CHANGED_FLAGS,NULL);
        } else {
            integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH );
            integer->woFlags |= WOF_NON_SELECTABLE;
            integer->Information(I_CHANGED_FLAGS,NULL);
            integer = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH );
            integer->woFlags |= WOF_NON_SELECTABLE;
            integer->Information(I_CHANGED_FLAGS,NULL);
        }
        break;

    case OPT_PRINTER_TYPE: {
        enum PRINTER_TYPE type = 
            GetPrinterTypeFromCombo( (UIW_COMBO_BOX *) Get(COMBO_PRINTER_TYPE) );
        switch ( type ) {
        case PrinterWindows:
            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_DOS_DEVICE);
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_WINDOWS);
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL) ;

            spin = (UIW_SPIN_CONTROL *) Get(SPIN_PRINTER_TIMEOUT);
            spin->woFlags &= ~WOF_NON_SELECTABLE ;
            spin->Information(I_CHANGED_FLAGS,NULL) ;

            string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;

            button = (UIW_BUTTON *) Get(CHECK_PRINT_FORMFEED);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;
            break;

        case PrinterFile:
        case PrinterPipe:
            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_DOS_DEVICE);
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_WINDOWS);
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL) ;

            spin = (UIW_SPIN_CONTROL *) Get(SPIN_PRINTER_TIMEOUT);
            spin->woFlags &= ~WOF_NON_SELECTABLE ;
            spin->Information(I_CHANGED_FLAGS,NULL) ;

            string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;

            button = (UIW_BUTTON *) Get(CHECK_PRINT_FORMFEED);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;
            break;

        case PrinterNone:
            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_DOS_DEVICE);
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_WINDOWS);
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL) ;

            spin = (UIW_SPIN_CONTROL *) Get(SPIN_PRINTER_TIMEOUT);
            spin->woFlags |= WOF_NON_SELECTABLE ;
            spin->Information(I_CHANGED_FLAGS,NULL) ;

            string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
            string->woFlags |= WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;

            button = (UIW_BUTTON *) Get(CHECK_PRINT_FORMFEED);
            button->woFlags |= WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
            string->woFlags |= WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;
            break;

        case PrinterDOS:
        default:
            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_DOS_DEVICE);
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX*) Get(COMBO_PRINT_WINDOWS);
            combo->woFlags |= WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            button = (UIW_BUTTON *) Get(CHECK_PRINT_BIDI);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY ) ;
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW ) ;
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS, NULL);

            combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
            combo->woFlags &= ~WOF_NON_SELECTABLE ;
            combo->Information(I_CHANGED_FLAGS,NULL) ;

            spin = (UIW_SPIN_CONTROL *) Get(SPIN_PRINTER_TIMEOUT);
            spin->woFlags &= ~WOF_NON_SELECTABLE ;
            spin->Information(I_CHANGED_FLAGS,NULL) ;

            string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;

            button = (UIW_BUTTON *) Get(CHECK_PRINT_FORMFEED);
            button->woFlags &= ~WOF_NON_SELECTABLE ;
            button->Information(I_CHANGED_FLAGS, NULL);

            string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
            string->woFlags &= ~WOF_NON_SELECTABLE ;
            string->Information(I_CHANGED_FLAGS,NULL) ;
            break;
            
        }
        break;
    }

    case OPT_CHECK_END_OF_JOB_FF:
	string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR ) ;
	if ( FlagSet(Get( CHECK_PRINT_FORMFEED )->woStatus, WOS_SELECTED ) )
	    string->woFlags |= WOF_NON_SELECTABLE ;
	else 	
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	string->Information(I_CHANGED_FLAGS, NULL);
        break;

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

void K_SETTINGS_NOTEBOOK::SetComboToColor( UIW_COMBO_BOX * combo, K_COLOR & color )
{
    combo->DataSet( color.Name() ) ; 
}

// Apply Current Settings of Notebook to Entry
//
//

void K_SETTINGS_NOTEBOOK::
ApplyChangesToEntry( void )
{
    if ( _entry == NULL )
        _entry = new KD_LIST_ITEM ;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME ) ;
    if ( !_original_name || strcmp( _original_name, "DEFAULT" ) )
        strncpy(_entry->_name, string->DataGet(), 28 ) ;
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
        strncpy(_entry->_name, _original_name, 28 ) ;
    }

    string = (UIW_STRING *) Get( FIELD_LOCATION ) ;
    strncpy( _entry->_location, string->DataGet(), 20 ) ;

    string = (UIW_STRING *) Get( ENTRY_ADDRESS ) ;
    switch ( _transport ) {     
    case PHONE:
        strncpy( _entry->_phone_number, string->DataGet(), 256 ) ;
        break;
    case TCPIP:
    case SSH:
        strncpy( _entry->_ipaddress, string->DataGet(), 256 ) ;
        break;
    case SUPERLAT:
        strncpy( _entry->_lataddress, string->DataGet(), 256 ) ;
        break;
    }

    string = (UIW_STRING *) Get( ENTRY_IPPORT ) ;
    strncpy( _entry->_ipport, string->DataGet(), 32 ) ;

    string = (UIW_STRING *) Get( ENTRY_SSHPORT ) ;
    strncpy( _entry->_sshport, string->DataGet(), 32 ) ;

    _entry->_access = _transport;

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_PROTOCOL ) ;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TCP_PROTOCOL );
    UIW_BUTTON  * button = (UIW_BUTTON *) list->Current();
    ZIL_ICHAR * ichar;
    if ( Get( PROTO_DEFAULT ) == button )
        _entry->_tcpproto = TCP_DEFAULT ;
    else if ( Get( PROTO_EK4LOGIN ) == button )
        _entry->_tcpproto = TCP_EK4LOGIN ;
    else if ( Get( PROTO_EK5LOGIN ) == button )
        _entry->_tcpproto = TCP_EK5LOGIN;
    else if ( Get( PROTO_K4LOGIN ) == button )
        _entry->_tcpproto = TCP_K4LOGIN;
    else if ( Get( PROTO_K5LOGIN ) == button )
        _entry->_tcpproto = TCP_K5LOGIN;
    else if ( Get( PROTO_RAW ) == button )
        _entry->_tcpproto = TCP_RAW;
    else if ( Get( PROTO_RLOGIN ) == button )
        _entry->_tcpproto = TCP_RLOGIN;
    else if ( Get( PROTO_TELNET ) == button )
        _entry->_tcpproto = TCP_TELNET;
    else if ( Get( PROTO_TELNET_NO_INIT ) == button )
        _entry->_tcpproto = TCP_TELNET_NO_INIT;
    else if ( Get( PROTO_TELNET_SSL ) == button )
        _entry->_tcpproto = TCP_TELNET_SSL;
    else if ( Get( PROTO_TELNET_TLS ) == button )
        _entry->_tcpproto = TCP_TELNET_TLS;
    else if ( Get( PROTO_SSL ) == button )
        _entry->_tcpproto = TCP_SSL;
    else if ( Get( PROTO_TLS ) == button )
        _entry->_tcpproto = TCP_TLS;

    combo = (UIW_COMBO_BOX *) Get( COMBO_SSH_PROTOCOL ) ;
    list = (UIW_VT_LIST *) Get( LIST_SSH_PROTOCOL );
    button = (UIW_BUTTON *) list->Current();
    if ( Get( SSH_PROTO_AUTO ) == button )
        _entry->_sshproto = SSH_AUTO ;
    else if ( Get( SSH_PROTO_V1 ) == button )
        _entry->_sshproto = SSH_V1 ;
    else if ( Get( SSH_PROTO_V2 ) == button )
        _entry->_sshproto = SSH_V2;

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

    // Terminal Page
    list = (UIW_VT_LIST *) Get( LIST_TTYPE );
    button = (UIW_BUTTON *) list->Current();
    ichar  = button->DataGet() ;

    if ( !strcmp(ichar,"VT320") )
	_entry->_terminal = VT320 ;
    else if ( !strcmp(ichar,"VT220") )
        _entry->_terminal = VT220 ;
    else if ( !strcmp(ichar,"VT102") )
        _entry->_terminal = VT102 ;
    else if ( !strcmp(ichar,"VT52") )
        _entry->_terminal = VT52 ;
    else if ( !strcmp(ichar,"VT100") )
        _entry->_terminal = VT100 ;
    else if ( !strcmp(ichar,"ANSI-BBS") )
        _entry->_terminal = ANSI ;
    else if ( !strcmp(ichar,"TTY") )
        _entry->_terminal = TTY ;
    else if ( !strcmp(ichar,"WY30") )
        _entry->_terminal = WY30 ;
    else if ( !strcmp(ichar,"WY50") )
        _entry->_terminal = WY50 ;
    else if ( !strcmp(ichar,"WY60") )
        _entry->_terminal = WY60 ;
    else if ( !strcmp(ichar,"WY160") )
        _entry->_terminal = WY160 ;
    else if ( !strcmp(ichar,"WY370") )
        _entry->_terminal = WY370 ;
    else if ( !strcmp(ichar,"DG200") )
        _entry->_terminal = DG200 ;
    else if ( !strcmp(ichar,"SCOANSI") )
        _entry->_terminal = SCOANSI ;
    else if ( !strcmp(ichar,"AT386") )
        _entry->_terminal = AT386 ;
    else if ( !strcmp(ichar,"AVATAR/0+") )
        _entry->_terminal = AVATAR ;
    else if ( !strcmp(ichar,"DG210") )
        _entry->_terminal = DG210 ;
    else if ( !strcmp(ichar,"DG217") )
        _entry->_terminal = DG217 ;
    else if ( !strcmp(ichar,"HEATH19") )
        _entry->_terminal = HEATH19 ;
    else if ( !strcmp(ichar,"HP2621") )
        _entry->_terminal = HP2621 ;
    else if ( !strcmp(ichar,"HPTERM") )
        _entry->_terminal = HPTERM ;
    else if ( !strcmp(ichar,"HZ1500") )
        _entry->_terminal = HZ1500 ;
    else if ( !strcmp(ichar,"TVI910+") )
        _entry->_terminal = TVI910 ;
    else if ( !strcmp(ichar,"TVI925") )
        _entry->_terminal = TVI925 ;
    else if ( !strcmp(ichar,"TVI950") )
        _entry->_terminal = TVI950 ;
    else if ( !strcmp(ichar,"VC404") )
        _entry->_terminal = VC404 ;
    else if ( !strcmp(ichar,"VIP7809") )
        _entry->_terminal = VIP7809;
    else if ( !strcmp(ichar,"BEOS-ANSI") )
        _entry->_terminal = BEOS;
    else if ( !strcmp(ichar,"QNX") )
        _entry->_terminal = QNX;
    else if ( !strcmp(ichar,"QANSI") )
        _entry->_terminal = QANSI;
    else if ( !strcmp(ichar,"SNI-97801") )
        _entry->_terminal = SNI97801;
    else if ( !strcmp(ichar,"BA80") )
        _entry->_terminal = BA80;
    else if ( !strcmp(ichar,"AIXTERM") )
        _entry->_terminal = AIXTERM;
    else if ( !strcmp(ichar,"HFT") )
        _entry->_terminal = HFT;
    else if ( !strcmp(ichar,"LINUX") )
        _entry->_terminal = LINUX;
    else if ( !strcmp(ichar,"VTNT") )
        _entry->_terminal = VTNT;
    else if ( !strcmp(ichar,"IBM3151") )
        _entry->_terminal = IBM3151 ;
    else if ( !strcmp(ichar,"SUN") )
        _entry->_terminal = SUN ;
    else if ( !strcmp(ichar,"ANNARBOR") )
        _entry->_terminal = ANNARBOR ;
    else _entry->_terminal = ANSI ;

    if ( FlagSet(Get( RADIO_BITS_7 )->woStatus, WOS_SELECTED ) )
        _entry->_charsize = 7 ;
    else if ( FlagSet(Get( RADIO_BITS_8 )->woStatus, WOS_SELECTED ) )
        _entry->_charsize = 8 ;

    _entry->_local_echo = FlagSet(Get( CHECK_LOCAL_ECHO )->woStatus, WOS_SELECTED) ;
    _entry->_auto_wrap = FlagSet(Get( CHECK_AUTO_WRAP )->woStatus, WOS_SELECTED) ;
    _entry->_apc_enabled = FlagSet(Get( CHECK_APC )->woStatus, WOS_SELECTED) ;
    _entry->_auto_download = FlagSet(Get( CHECK_AUTODOWNLOAD )->woStatus, WOS_SELECTED) ;
    _entry->_status_line = FlagSet( Get( CHECK_STATUS_LINE )->woStatus, WOS_SELECTED );

    combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
    _entry->_width = atoi(combo->DataGet()) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
    _entry->_height = atoi(combo->DataGet()) ;

    _entry->_cursor = GetCursorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_CURSOR ) ) ;

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_SCROLLBACK ) ;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( SPIN_INT_SCROLLBACK ) ;
    int value;
    spin->Information( I_GET_VALUE, &value );
    _entry->_scrollback = value;

   _entry->_color_term_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ) ;
   _entry->_color_term_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ) ;
   _entry->_color_status_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_FG ) ) ;
   _entry->_color_status_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_BG ) ) ;
   _entry->_color_popup_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_FG ) ) ;
   _entry->_color_popup_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_BG ) ) ;
   _entry->_color_select_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_FG ) ) ;
   _entry->_color_select_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_BG ) ) ;
   _entry->_color_underline_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ) ;
   _entry->_color_underline_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ) ;
   _entry->_color_command_fg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_FG ) ) ;
   _entry->_color_command_bg = GetColorFromCombo( (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_BG ) ) ;

   _entry->_term_charset = GetTermCharsetFromCombo( (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ) ;

    //File Transfer

    combo = (UIW_COMBO_BOX *) Get( COMBO_XFER_PROTOCOL ) ;
    ichar = combo->DataGet() ;
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

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PACKET_LENGTH ) ;
    spin_int = (UIW_INTEGER *) spin->Get( SPIN_PACKET_RANGE ) ;
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

   // Communications
    _entry->_tapi_line = 0;
    combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
    strncpy( _entry->_modem, combo->DataGet(), 60 ) ;
    button = (UIW_BUTTON *) combo->Current();
    EVENT_TYPE ButtonMsg;
    button->Information( I_GET_VALUE, &ButtonMsg );
    _entry->_tapi_line |= (ButtonMsg == OPT_LINE_DEVICE_IS_TAPI);

    combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
    strncpy( _entry->_line_device, combo->DataGet(), 60 ) ;
    button = (UIW_BUTTON *) combo->Current();
    button->Information( I_GET_VALUE, &ButtonMsg );
    _entry->_tapi_line |= (ButtonMsg == OPT_LINE_DEVICE_IS_TAPI);

    _entry->_use_mdm_parity = !FlagSet(Get(CHECK_OVER_PARITY)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_stopbits = !FlagSet(Get(CHECK_OVER_STOPBITS)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_flow = !FlagSet(Get(CHECK_OVER_FLOW)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_speed = !FlagSet(Get(CHECK_OVER_SPEED)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_ec = !FlagSet(Get(CHECK_OVER_EC)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_dc = !FlagSet(Get(CHECK_OVER_DC)->woStatus, WOS_SELECTED);
    _entry->_use_mdm_carrier = !FlagSet(Get(CHECK_OVER_CARRIER)->woStatus, WOS_SELECTED);

   combo = (UIW_COMBO_BOX *) Get( COMBO_SPEED ) ;
   _entry->_speed = atoi( combo->DataGet() ) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_FLOW );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"None") )
	_entry->_flow = NO_FLOW;
    else if ( !strcmp(ichar, "Xon/Xoff") )
	_entry->_flow = XON_XOFF;
    else if ( !strcmp(ichar, "Rts/Cts") )
	_entry->_flow = RTS_CTS;
    else if ( !strcmp(ichar, "Auto") )
	_entry->_flow = AUTO_FLOW;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PARITY );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"None") )
	_entry->_parity = NO_PARITY;
    else if ( !strcmp(ichar, "Space") )
	_entry->_parity = SPACE;
    else if ( !strcmp(ichar, "Mark") )
	_entry->_parity = MARK;
    else if ( !strcmp(ichar, "Even") )
	_entry->_parity = EVEN;
    else if ( !strcmp(ichar, "Odd") )
	_entry->_parity = ODD;
    else if ( !strcmp(ichar, "Space_8bits") )
	_entry->_parity = SPACE_8;
    else if ( !strcmp(ichar, "Mark_8bits") )
	_entry->_parity = MARK_8;
    else if ( !strcmp(ichar, "Even_8bits") )
	_entry->_parity = EVEN_8;
    else if ( !strcmp(ichar, "Odd_8bits") )
	_entry->_parity = ODD_8;

    combo = (UIW_COMBO_BOX *) Get( COMBO_STOPBITS );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"1.0") )
        _entry->_stopbits = STOPBITS_1_0;
    else if ( !strcmp(ichar,"1.5") )
        _entry->_stopbits = STOPBITS_1_5;
    else if ( !strcmp(ichar,"2.0") )
        _entry->_stopbits = STOPBITS_2_0;

    _entry->_correction = FlagSet(Get(CHECK_EC)->woStatus, WOS_SELECTED);
    _entry->_compression = FlagSet(Get(CHECK_DC)->woStatus, WOS_SELECTED);
    _entry->_carrier = FlagSet(Get(CHECK_CARRIER)->woStatus, WOS_SELECTED);

    // TCP/IP Page
    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_AUTH );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_auth_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_auth_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_auth_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_auth_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_BINARY );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_binary_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_binary_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_binary_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_binary_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_ENCRYPT );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_encrypt_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_encrypt_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_encrypt_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_encrypt_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_KERMIT );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_kermit_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_kermit_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_kermit_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_kermit_mode = TelnetRequire;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_START_TLS );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Accept") )
        _entry->_telnet_starttls_mode = TelnetAccept;
    else if ( !strcmp(ichar,"Refuse") )
        _entry->_telnet_starttls_mode = TelnetRefuse;
    else if ( !strcmp(ichar,"Request") )
        _entry->_telnet_starttls_mode = TelnetRequest;
    else if ( !strcmp(ichar,"Require") )
        _entry->_telnet_starttls_mode = TelnetRequire;

    string = (UIW_STRING *) Get( TEXT_TCP_LOCATION ) ;
    strncpy( _entry->_telnet_location, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_TERMTYPE ) ;
    strncpy( _entry->_telnet_ttype, string->DataGet(), 40 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_ACCT ) ;
    strncpy( _entry->_telnet_acct, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_DISP ) ;
    strncpy( _entry->_telnet_disp, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_JOB ) ;
    strncpy( _entry->_telnet_job, string->DataGet(), 64 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_PRINTER ) ;
    strncpy( _entry->_telnet_printer, string->DataGet(), 64 ) ;

    button = (UIW_BUTTON *) Get(CHECK_TCP_IP_ADDRESS);
    _entry->_default_ip_address = button->woStatus & WOS_SELECTED;

    UIW_FORMATTED_STRING * format;
    format = (UIW_FORMATTED_STRING *) Get( FORMAT_TCP_ADDRESS ) ;
    strncpy( _entry->_tcp_ip_address, format->DataGet(), 18 ) ;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_SENDBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_SENDBUF ) ;
    int new_spin_int;
    spin->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_tcp_sendbuf = new_spin_int;

    spin     = (UIW_SPIN_CONTROL *) Get( SPIN_TCP_RECVBUF );
    spin_int = (UIW_INTEGER *) spin->Get( INT_TCP_RECVBUF ) ;
    spin->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_tcp_recvbuf = new_spin_int;

    combo = (UIW_COMBO_BOX *) Get( COMBO_TCP_RDNS );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Auto") )
        _entry->_tcp_rdns = AUTO;
    else if ( !strcmp(ichar,"On") )
        _entry->_tcp_rdns = ON;
    else if ( !strcmp(ichar,"Off") )
        _entry->_tcp_rdns = OFF;

    button = (UIW_BUTTON *) Get(CHECK_TCP_DNS_SRV);
    _entry->_tcp_dns_srv = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TCP_FWD_CRED);
    _entry->_fwd_cred = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TCP_WAIT);
    _entry->_telnet_wait = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TELNET_DEBUG);
    _entry->_telnet_debug = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_TELNET_SB_DELAY);
    _entry->_telnet_sb_delay = button->woStatus & WOS_SELECTED;

    // Kerberos
    string = (UIW_STRING *) Get( TEXT_TCP_K4_REALM ) ;
    strncpy( _entry->_k4_realm, string->DataGet(), 256 ) ;

    string = (UIW_STRING *) Get( TEXT_TCP_K4_PRINC ) ;
    strncpy( _entry->_k4_princ, string->DataGet(), 64 ) ;

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTOGET);
    _entry->_k4_autoget = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get(CHECK_K4_AUTODESTROY);
    _entry->_k4_autodestroy = button->woStatus & WOS_SELECTED;

    spin_int = (UIW_INTEGER *) Get( INTEGER_K4_LIFETIME ) ;
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

   if ( FlagSet(Get( RADIO_KEYMAP_DEFAULT )->woStatus, WOS_SELECTED ) )
      _entry->_keymap = VT100_KEY ;
   else if ( FlagSet(Get( RADIO_KEYMAP_FILE )->woStatus, WOS_SELECTED ) )
      _entry->_keymap = MAPFILE;

   text = (UIW_TEXT *) Get( TEXT_KEYMAPFILE );

   string = (UIW_STRING *) Get( FIELD_KEYMAP_FILE ) ;
   strncpy( _entry->_keymap_file, string->DataGet(), 256 ) ;

   //Login Settings
   _entry->_script_file = FlagSet( Get( CHECK_SET_SCRIPTFILE )->woStatus, WOS_SELECTED ) ;

   string = (UIW_STRING *) Get( FIELD_SCRIPTFILE ) ;
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

    text = (UIW_TEXT *) Get( TEXT_SCRIPT ) ;
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


    /* Printer page */
    _entry->_printer_type = 
        GetPrinterTypeFromCombo( (UIW_COMBO_BOX *) Get(COMBO_PRINTER_TYPE) );

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_DOS_DEVICE ) ;
    strncpy( _entry->_printer_dos, combo->DataGet(), 256 ) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_WINDOWS ) ;
    strncpy( _entry->_printer_windows, combo->DataGet(), 256 ) ;

    string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
    strncpy( _entry->_printer_separator, string->DataGet(), 256 ) ;

    _entry->_printer_formfeed = 
        FlagSet(Get( CHECK_PRINT_FORMFEED )->woStatus, WOS_SELECTED );

    string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
    strncpy( _entry->_printer_terminator, string->DataGet(), 64 ) ;

    spin = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT );
    spin->Information( I_GET_VALUE, &value );
    _entry->_printer_timeout = value;

    _entry->_printer_bidi =
        FlagSet(Get( CHECK_PRINT_BIDI )->woStatus, WOS_SELECTED );

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
    _entry->_printer_speed = atoi( combo->DataGet() ) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"None") )
	_entry->_printer_flow = NO_FLOW;
    else if ( !strcmp(ichar, "Xon/Xoff") )
	_entry->_printer_flow = XON_XOFF;
    else if ( !strcmp(ichar, "Rts/Cts") )
	_entry->_printer_flow = RTS_CTS;
    else if ( !strcmp(ichar, "Auto") )
	_entry->_printer_flow = AUTO_FLOW;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_PARITY );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"None") )
	_entry->_printer_parity = NO_PARITY;
    else if ( !strcmp(ichar, "Space") )
	_entry->_printer_parity = SPACE;
    else if ( !strcmp(ichar, "Mark") )
	_entry->_printer_parity = MARK;
    else if ( !strcmp(ichar, "Even") )
	_entry->_printer_parity = EVEN;
    else if ( !strcmp(ichar, "Odd") )
	_entry->_printer_parity = ODD;

    _entry->_printer_ps =
        FlagSet(Get( CHECK_PRINT_POSTSCRIPT )->woStatus, WOS_SELECTED );

    spin_int = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH ) ;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_printer_width = new_spin_int;

    spin_int = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH ) ;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_printer_length = new_spin_int;

    /* Logs page */
    _entry->_log_connection = FlagSet( Get( CHECK_LOG_CONNECTION )->woStatus, WOS_SELECTED ) ;
    _entry->_log_connection_append = 
	FlagSet( Get( CHECK_LOG_CONNECTION_APPEND )->woStatus, WOS_SELECTED ) ;

    string = (UIW_STRING *) Get( FIELD_LOG_CONNECTION ) ;
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

    // SSL/TLS Page
    string = (UIW_STRING *) Get( TEXT_TLS_CIPHER ) ;
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

    combo = (UIW_COMBO_BOX *) Get( COMBO_TLS_VERIFY_MODE );
    ichar = combo->DataGet() ;
    if ( !strcmp(ichar,"Do not verify host certificates") )
	_entry->_tls_verify_mode = TLS_VERIFY_NO;
    else if ( !strcmp(ichar,"Verify host certificates if presented") )
	_entry->_tls_verify_mode = TLS_VERIFY_PEER;
    else if ( !strcmp(ichar,"Peer certificates must be presented and verified; if not, fail") )
	_entry->_tls_verify_mode = TLS_VERIFY_FAIL;

    button = (UIW_BUTTON *) Get( CHECK_TLS_VERBOSE ) ;
    _entry->_tls_verbose = button->woStatus & WOS_SELECTED;

    button = (UIW_BUTTON *) Get( CHECK_TLS_DEBUG ) ;
    _entry->_tls_debug = button->woStatus & WOS_SELECTED;
}

void K_SETTINGS_NOTEBOOK::
InitTermTypeList( void )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;
    *list + new UIW_BUTTON( 0,0,0,"AIXTERM",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_AIXTERM );
    *list + new UIW_BUTTON( 0,0,0,"ANNARBOR",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_ANNARBOR );
    *list + new UIW_BUTTON( 0,0,0,"ANSI-BBS",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_ANSI );
    *list + new UIW_BUTTON( 0,0,0,"AT386",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_AT386 );
    *list + new UIW_BUTTON( 0,0,0,"AVATAR/0+",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_AVATAR );
    *list + new UIW_BUTTON( 0,0,0,"BA80",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_BA80 );
    *list + new UIW_BUTTON( 0,0,0,"BEOS-ANSI",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_BEOS );
    *list + new UIW_BUTTON( 0,0,0,"DG200",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_DG200 );
    *list + new UIW_BUTTON( 0,0,0,"DG210",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_DG210 );
    *list + new UIW_BUTTON( 0,0,0,"DG217",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_DG217 );
    *list + new UIW_BUTTON( 0,0,0,"HEATH19",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_HEATH19 );
    *list + new UIW_BUTTON( 0,0,0,"HFT",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_HFT );
    *list + new UIW_BUTTON( 0,0,0,"HP2621",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_HP2621 );
    *list + new UIW_BUTTON( 0,0,0,"HPTERM",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_HPTERM );
    *list + new UIW_BUTTON( 0,0,0,"HZ1500",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_HZ1500 );
    *list + new UIW_BUTTON( 0,0,0,"IBM3151",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_IBM3151 );
    *list + new UIW_BUTTON( 0,0,0,"LINUX",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_LINUX );
    *list + new UIW_BUTTON( 0,0,0,"QANSI",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_QANSI );
    *list + new UIW_BUTTON( 0,0,0,"QNX",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_QNX );
    *list + new UIW_BUTTON( 0,0,0,"SCOANSI",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_SCOANSI );
    *list + new UIW_BUTTON( 0,0,0,"SNI-97801",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_SNI97801 );
    *list + new UIW_BUTTON( 0,0,0,"SUN",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_SUN );
    *list + new UIW_BUTTON( 0,0,0,"TTY",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_TTY );
    *list + new UIW_BUTTON( 0,0,0,"TVI910+",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_TVI910 );
    *list + new UIW_BUTTON( 0,0,0,"TVI925",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_TVI925 );
    *list + new UIW_BUTTON( 0,0,0,"TVI950",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_TVI950 );
    *list + new UIW_BUTTON( 0,0,0,"VC404",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VC404 );
    *list + new UIW_BUTTON( 0,0,0,"VIP7809",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VIP7809 );
    *list + new UIW_BUTTON( 0,0,0,"VT100",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VT100 );
    *list + new UIW_BUTTON( 0,0,0,"VT102",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VT102 );
    *list + new UIW_BUTTON( 0,0,0,"VT220",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VT220 );
    *list + new UIW_BUTTON( 0,0,0,"VT320",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VT320 );
    *list + new UIW_BUTTON( 0,0,0,"VT52",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VT52 );
    *list + new UIW_BUTTON( 0,0,0,"VTNT",BTF_NO_3D|BTF_SEND_MESSAGE, 
#ifdef NT
                            WOF_NO_FLAGS,
#else /* NT */
                            WOF_NON_SELECTABLE,
#endif /* NT */
                            ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_VTNT );
    *list + new UIW_BUTTON( 0,0,0,"WY30",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_WY30 );
    *list + new UIW_BUTTON( 0,0,0,"WY50",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_WY50 );
    *list + new UIW_BUTTON( 0,0,0,"WY60",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_WY60 );
    *list + new UIW_BUTTON( 0,0,0,"WY160",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_WY160 );
    *list + new UIW_BUTTON( 0,0,0,"WY370",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_WY370 );
}
void K_SETTINGS_NOTEBOOK::
SetTermTypeList( ZIL_ICHAR * ttype )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;

    UIW_BUTTON * listitem = (UIW_BUTTON *) list->First() ;
    UIW_BUTTON * lastlistitem = (UIW_BUTTON *) list->Last() ;
    if ( listitem )
	do { 
	    if ( !strcmp( ttype, listitem->DataGet() ) ) {
		*list + listitem ;
		return;
	    }
	    if (listitem == lastlistitem )
		break ;
	    else
		listitem = (UIW_BUTTON *) listitem->Next() ;
	} while ( listitem );
}

void K_SETTINGS_NOTEBOOK::
InitColorLists( void )
{
   UIW_VT_LIST * color_list[12] ;
   color_list[0] = (UIW_VT_LIST *) Get( LIST_COLOR_TERM_FG ) ;
   color_list[1] = (UIW_VT_LIST *) Get( LIST_COLOR_TERM_BG ) ;
   color_list[2] = (UIW_VT_LIST *) Get( LIST_COLOR_STATUS_FG ) ;
   color_list[3] = (UIW_VT_LIST *) Get( LIST_COLOR_STATUS_BG ) ;
   color_list[4] = (UIW_VT_LIST *) Get( LIST_COLOR_POPUP_FG ) ;
   color_list[5] = (UIW_VT_LIST *) Get( LIST_COLOR_POPUP_BG ) ;
   color_list[6] = (UIW_VT_LIST *) Get( LIST_COLOR_SELECT_FG ) ;
   color_list[7] = (UIW_VT_LIST *) Get( LIST_COLOR_SELECT_BG ) ;
   color_list[8] = (UIW_VT_LIST *) Get( LIST_COLOR_UNDERLINE_FG ) ;
   color_list[9] = (UIW_VT_LIST *) Get( LIST_COLOR_UNDERLINE_BG ) ;
   color_list[10] = (UIW_VT_LIST *) Get( LIST_COLOR_COMMAND_FG ) ;
   color_list[11] = (UIW_VT_LIST *) Get( LIST_COLOR_COMMAND_BG ) ;

   for ( int i=0; i<12; i++ )
   {
      K_COLOR::InitList( color_list[i] ) ;
   }
    
}

void K_SETTINGS_NOTEBOOK::
InitCursorList( void )
{
      UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_CURSOR ) ;
      *list
         + new UIW_BUTTON( 0,0,0,"full",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"half",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"underline",BTF_NO_3D, WOF_NO_FLAGS ) ;
}

void K_SETTINGS_NOTEBOOK::
InitHeightList( void )
{
      UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SCREEN_HEIGHT ) ;
      *list
         + new UIW_BUTTON( 0,0,0,"24",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"25",BTF_NO_3D, WOF_NO_FLAGS )
	 + new UIW_BUTTON( 0,0,0,"42",BTF_NO_3D, WOF_NO_FLAGS )
	 + new UIW_BUTTON( 0,0,0,"43",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"49",BTF_NO_3D, WOF_NO_FLAGS )
     	 + new UIW_BUTTON( 0,0,0,"50",BTF_NO_3D, WOF_NO_FLAGS ) ;
}

void K_SETTINGS_NOTEBOOK::
InitWidthList( void )
{
      UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SCREEN_COL ) ;
      *list
         + new UIW_BUTTON( 0,0,0,"80",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"132",BTF_NO_3D, WOF_NO_FLAGS ) ;
}

void K_SETTINGS_NOTEBOOK::
InitLineList( ZIL_UINT8 _templates )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_LINES ) ;
    for ( int i=1; i<=128; i++ ) {
        char buf[12];
        sprintf(buf,"COM%d",i);
        *list
            + new UIW_BUTTON( 0,0,0,buf,BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
                              ZIL_NULLF(ZIL_USER_FUNCTION), OPT_LINE_DEVICE_IS_PORT );
    }

#ifdef WIN32
    if ( TapiAvail )
    {
	cktapiBuildLineTable( &tapilinetab, &_tapilinetab, &ntapiline );
	for ( int i=0 ; i<ntapiline ; i++ )
	    *list + new UIW_BUTTON( 0,0,0,_tapilinetab[i].kwd,BTF_NO_3D|BTF_SEND_MESSAGE, 
				   WOF_NO_FLAGS,ZIL_NULLF(ZIL_USER_FUNCTION), 
				   OPT_LINE_DEVICE_IS_TAPI ) ;
    }
#endif /* WIN32 */
}

void K_SETTINGS_NOTEBOOK::
InitModemList( void )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_MODEM ) ;
    K_MODEM * modem;
    for ( modem = (K_MODEM *) connector->_modem_list.First() ; modem ;
	  modem = (K_MODEM *) modem->Next())
	*list 
	+ new UIW_BUTTON( 0,0,0,modem->_name,BTF_NO_3D|BTF_SEND_MESSAGE, 
			  WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), 
			  OPT_MODEM_IS_MODEM ) ;
}

void K_SETTINGS_NOTEBOOK::
InitFlowList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_FLOW ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Xon/Xoff",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Rts/Cts",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Auto",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::
InitParityList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PARITY ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Space",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Even",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Odd",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Mark",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"Space_8bits",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Even_8bits",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Odd_8bits",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Mark_8bits",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::
InitStopBitList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_STOPBITS ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"1.0",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"1.5",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"2.0",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::InitSpeedList( void ) 
{
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SPEED ) ;
   *list
       + new UIW_BUTTON( 0,0,0,"50",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"75",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"110",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"150",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"300",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"1200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"2400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"3600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"4800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"7200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"9600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"14400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"19200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"28800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"38400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"57600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"76800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"115200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"230400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"460800",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::InitTermCharsetList( UIW_VT_LIST * list )   
{
    enum TERMCSET charset = T_TRANSPARENT ;

    for (int i = 0; i < n_tcs; i++) {
        *list + new UIW_BUTTON(0,0,0,_tcs[i].name,
                                BTF_NO_3D, WOF_NO_FLAGS);
    }
}

void K_SETTINGS_NOTEBOOK::InitFileCharsetList( UIW_VT_LIST * list )
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
void K_SETTINGS_NOTEBOOK::InitXferCharsetList( UIW_VT_LIST * list )   
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

void K_SETTINGS_NOTEBOOK::
InitPrinterTypeList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINTER_TYPE ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"DOS Device",BTF_NO_3D|BTF_SEND_MESSAGE, 
                          WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), OPT_PRINTER_TYPE )
#ifdef WIN32
        + new UIW_BUTTON( 0,0,0,"Windows Printer Queue",BTF_NO_3D|BTF_SEND_MESSAGE,
                          WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), OPT_PRINTER_TYPE )
#endif /* WIN32 */
	+ new UIW_BUTTON( 0,0,0,"File",BTF_NO_3D|BTF_SEND_MESSAGE, 
                          WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), OPT_PRINTER_TYPE )
        + new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D|BTF_SEND_MESSAGE, 
                          WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), OPT_PRINTER_TYPE )
        + new UIW_BUTTON( 0,0,0,"Command (Pipe)",BTF_NO_3D|BTF_SEND_MESSAGE, 
                      WOF_NO_FLAGS, ZIL_NULLF(ZIL_USER_FUNCTION), OPT_PRINTER_TYPE );
}

enum PRINTER_TYPE K_SETTINGS_NOTEBOOK::
GetPrinterTypeFromCombo( UIW_COMBO_BOX * combo )
{
    ZIL_ICHAR * data = combo->DataGet();
    if ( !strcmp(data,"DOS Device" ) )
        return PrinterDOS;
    else if ( !strcmp(data, "Windows Printer Queue") )
        return PrinterWindows;
    else if ( !strcmp(data, "File") )
        return PrinterFile;
    else if ( !strcmp(data, "Command (Pipe)") )
        return PrinterPipe;
    else if ( !strcmp(data, "None") )
        return PrinterNone;
    return PrinterDOS;
}

void K_SETTINGS_NOTEBOOK::
InitPrinterDosList(ZIL_ICHAR * user)
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_DOS_DEVICE ) ;
    *list 
        + new UIW_BUTTON( 0,0,0,"prn",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"lpt1",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"lpt2",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"lpt3",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"com1",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"com2",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"com3",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"com4",BTF_NO_3D, WOF_NO_FLAGS ) 
        + new UIW_BUTTON( 0,0,0,"kermit.prn",BTF_NO_3D, WOF_NO_FLAGS ); 
    if (user[0] && 
        strcmp(user,"prn") && strcmp(user,"lpt1") &&
        strcmp(user,"lpt2") && strcmp(user,"lpt3") &&
        strcmp(user,"com1") && strcmp(user,"com2") &&
        strcmp(user,"com3") && strcmp(user,"com4") &&
        strcmp(user,"kermit.prn"))
        *list 
            + new UIW_BUTTON( 0,0,0,user,BTF_NO_3D, WOF_NO_FLAGS );
}

#ifdef WIN32
int
Win32EnumPrt( struct keytab ** pTable, struct keytab ** pTable2, int * pN );
#ifdef COMMENT
{
    DWORD  dwBytesNeeded;
    DWORD  dwPrtRet2;
    LPTSTR lpName = NULL;
    DWORD  dwEnumFlags = PRINTER_ENUM_LOCAL;
    LPPRINTER_INFO_2 pPrtInfo2=NULL;
    int i;

    if ( *pTable )
    {
	for ( i=0 ; i < *pN ; i++ )
	    free( (*pTable)[i].kwd ) ;
	free ( *pTable )  ;
    }
    *pTable = NULL;
    if ( *pTable2 )
    {
	for ( i=0 ; i < *pN ; i++ )
	    free( (*pTable2)[i].kwd ) ;
	free ( *pTable2 )  ;
    }
    *pTable2 = NULL;
    *pN = 0;

    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;

    if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
        dwEnumFlags |= PRINTER_ENUM_CONNECTIONS;
    }


    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //

    EnumPrinters (dwEnumFlags, lpName, 2, NULL, 0, &dwBytesNeeded,
                   &dwPrtRet2);

    //
    // (simple error checking, if these work assume rest will too)
    //

    if (!(pPrtInfo2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwBytesNeeded)))
    {
        return(FALSE);
    }

    if (!EnumPrinters (dwEnumFlags, lpName, 2, (LPBYTE) pPrtInfo2,
                        dwBytesNeeded, &dwBytesNeeded, &dwPrtRet2))
    {
        LocalFree( pPrtInfo2 );
        return(FALSE);
    }

    /* we now have an enumeration of all printer names */

    (*pTable) = (struct keytab *) malloc( dwPrtRet2 * sizeof(struct keytab) );
    if ( !(*pTable) )
    {
        LocalFree( pPrtInfo2 );
        return(FALSE);
    }
    (*pTable2) = (struct keytab *) malloc( dwPrtRet2 * sizeof(struct keytab) );
    if ( !(*pTable2) )
    {
        free(pTable);
        pTable = NULL;
        LocalFree( pPrtInfo2 );
        return(FALSE);
    }

    for ( i=0 ; i<dwPrtRet2 ; i++ ) {
        char * s = NULL;
        (*pTable)[i].kwd = strdup( pPrtInfo2[i].pPrinterName );
        (*pTable2)[i].kwd = strdup( pPrtInfo2[i].pPrinterName );
        (*pTable)[i].kwval = i;
        (*pTable2)[i].kwval = i;
        (*pTable)[i].flgs = 0;
        (*pTable2)[i].flgs = 0;

        /* Now we must remove illegal characters from the pTable2 kwd */
        s = (*pTable2)[i].kwd;
        while ( *s ) {
            switch ( *s ) {
            case ' ':
                *s = '_';
                break;
            case ',':
                *s = '.';
                break;
            case ';':
                *s = ':';
                break;
            case '\\':
                *s = '/';
                break;
            case '?':
                *s = '!';
                break;
            case '{':
                *s = '[';
                break;
            case '}':
                *s = ']';
                break;
            }
            s++;
        }
    }
    *pN = dwPrtRet2 ;

    LocalFree( pPrtInfo2 );
    return(TRUE);
}
#endif /* WIN32 */
#endif /* COMMENT */

void K_SETTINGS_NOTEBOOK::
InitPrinterWindowsList()
{
#ifdef WIN32
    struct keytab * printers=NULL, * _printers=NULL;
    int    num=0;

    if ( !Win32EnumPrt( &printers, &_printers, &num) ) {
        if ( printers )
            free(printers);
        if ( _printers )
            free(printers);
        return;
    }

    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_WINDOWS ) ;
    *list 
        + new UIW_BUTTON( 0,0,0,"<default>",
                          BTF_NO_3D, WOF_NO_FLAGS );

    for ( int i=0 ; i<num; i++ ) {
        *list 
            + new UIW_BUTTON( 0,0,0,_printers[i].kwd,
                              BTF_NO_3D, WOF_NO_FLAGS );
    }

    if ( printers ) {
        free(printers);
        printers = NULL;
    }
    if ( _printers ) {
        free(_printers);
        _printers = NULL;
    }
#endif /* WIN32 */
}

void K_SETTINGS_NOTEBOOK::
InitPrinterFlowList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_FLOW ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Xon/Xoff",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Rts/Cts",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::
InitPrinterParityList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_PARITY ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Space",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Even",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Odd",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Mark",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_SETTINGS_NOTEBOOK::
InitPrinterSpeedList( void )
{
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_SPEED ) ;
   *list
       + new UIW_BUTTON( 0,0,0,"50",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"75",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"110",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"150",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"300",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"1200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"2400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"3600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"4800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"7200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"9600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"14400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"19200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"28800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"38400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"57600",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"76800",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"115200",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"230400",BTF_NO_3D, WOF_NO_FLAGS )
       + new UIW_BUTTON( 0,0,0,"460800",BTF_NO_3D, WOF_NO_FLAGS );
}

   
K_COLOR K_SETTINGS_NOTEBOOK::
GetColorFromCombo( UIW_COMBO_BOX * combo )
{
   return K_COLOR( combo->DataGet() ) ;
}

enum TERMCSET K_SETTINGS_NOTEBOOK::
GetTermCharsetFromCombo( UIW_COMBO_BOX * combo )
{
   ZIL_ICHAR * value = combo->DataGet() ;
   enum TERMCSET charset = T_TRANSPARENT ;

   int i, x = 0;
   for (i = 0; i < n_tcs; i++) {
       if (!strcmp(_tcs[i].name, value)) {
	   x = 1;
	   break;
       }
   }
   if (x)
     charset = _tcs[i].val ;
   return charset ;
}

enum FILECSET K_SETTINGS_NOTEBOOK::
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

enum XFERCSET K_SETTINGS_NOTEBOOK::
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

void K_SETTINGS_NOTEBOOK::
SetComboToCharset( UIW_COMBO_BOX * combo, enum TERMCSET charset )
{
   int i, x = 0;
   for (i = 0; i < n_tcs; i++) {
       if (_tcs[i].val == charset) {
	   x = 1;
	   break;
       }
   }
   if (x)
     combo->DataSet( _tcs[i].name ) ;
   else
     combo->DataSet( "transparent" ) ;
}

void K_SETTINGS_NOTEBOOK::
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

void K_SETTINGS_NOTEBOOK::
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

void K_SETTINGS_NOTEBOOK::
SetComboToCursor( UIW_COMBO_BOX * combo, enum CURSOR_TYPE cursor )
{
   switch ( cursor ) {
   case HALF_CURSOR:
      combo->DataSet( "half" ) ;
      break;
   case UNDERLINE_CURSOR:
      combo->DataSet( "underline" ) ;
      break;
   case FULL_CURSOR:
   default:
      combo->DataSet( "full" ) ;
      break;
   }
}

enum CURSOR_TYPE
K_SETTINGS_NOTEBOOK::GetCursorFromCombo( UIW_COMBO_BOX * combo ) 
{
   ZIL_ICHAR * value = combo->DataGet() ;
   enum CURSOR_TYPE cursor = FULL_CURSOR ;

   if ( !strcmp("underline", value) )
      cursor = UNDERLINE_CURSOR ;

   else if ( !strcmp("half", value) )
      cursor = HALF_CURSOR ;

   else if (!strcmp("full", value)) 
      cursor = FULL_CURSOR ;

   return cursor ;
}
