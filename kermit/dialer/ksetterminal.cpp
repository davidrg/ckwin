#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_TERMINAL_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetterminal.hpp"
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

ZIL_ICHAR K_DIALOG_TERMINAL_SETTINGS::_className[] = "K_DIALOG_TERMINAL_SETTINGS" ;

extern K_CONNECTOR * connector ;

static struct __tcs {
    char * name;
    enum TERMCSET val;
} _tcs[] = {
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
#ifdef COMMENT
    "utf7",  T_UTF7,
#endif /* COMMENT */
    "utf8",  T_UTF8
};
static int n_tcs = (sizeof(_tcs) / sizeof(struct __tcs));

// Null Constructor for K_DIALOG_TERMINAL_SETTINGS
//
//

K_DIALOG_TERMINAL_SETTINGS::
K_DIALOG_TERMINAL_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_TERMINAL_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // Configure default values
   // Terminal Page
    InitTermTypeList();
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;
    SetTermTypeList( "VT320" );

    UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
    UIW_BUTTON *button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    *group + button ;

   InitWidthList() ;

   UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
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

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_TERMINAL_SETTINGS
//
//

K_DIALOG_TERMINAL_SETTINGS::
K_DIALOG_TERMINAL_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_TERMINAL_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_TERMINAL_SETTINGS::K_DIALOG_TERMINAL_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Terminal Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

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
    case SUN:
        ichar = "SUN";
        break;
    case ANNARBOR:
        ichar = "ANNARBOR";
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
    case ADM3A:
        ichar = "ADM-3A";
        break;
    }
    SetTermTypeList( ichar );
    if ( entry->_access == FTP ) {
        list->woFlags |= WOF_NON_SELECTABLE ;
        list->Information(I_CHANGED_FLAGS,NULL);
    }

    UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_CHAR_SIZE ) ;
    UIW_BUTTON *button;
    switch ( entry->_charsize ) {
    case 7:
        button = (UIW_BUTTON *) Get( RADIO_BITS_7 ) ;
        break;
    case 8:
        button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
        break;
    }
    if ( button ) {
        button->woStatus |= WOS_SELECTED ;
        button->Information(I_CHANGED_STATUS,NULL);
        *group + button ;
    }
    if ( entry->_access == FTP ) {
        group->woFlags |= WOF_NON_SELECTABLE ;
        group->Information(I_CHANGED_FLAGS,NULL);
    }

   button = (UIW_BUTTON *) Get( CHECK_LOCAL_ECHO );
   if ( entry->_local_echo )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

   InitAutoDownList();
   SetAutoDownList( entry->_auto_download );
   UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *)Get(COMBO_AUTODOWNLOAD);
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   button = (UIW_BUTTON *) Get( CHECK_AUTO_WRAP );
   if ( entry->_auto_wrap )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

   button = (UIW_BUTTON *) Get( CHECK_APC );
   if ( entry->_apc_enabled )
      button->woStatus |= WOS_SELECTED ;
   else
      button->woStatus &= ~WOS_SELECTED ;
   button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

   InitWidthList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
   ZIL_ICHAR itoabuf[40] ;
   itoa(entry->_width, itoabuf, 10) ;
   combo->DataSet( itoabuf ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   InitHeightList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_HEIGHT ) ;
   itoa(entry->_height, itoabuf, 10) ;
   combo->DataSet( itoabuf ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get( CHECK_STATUS_LINE );
    if ( entry->_status_line )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( entry->_access == FTP ) {
        button->woFlags |= WOF_NON_SELECTABLE ;
        button->Information(I_CHANGED_FLAGS,NULL);
    }

    InitCursorList() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_CURSOR ) ;
   SetComboToCursor( combo, entry->_cursor ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_SCROLLBACK ) ;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( SPIN_INT_SCROLLBACK ) ;
    int new_spin_int = entry->_scrollback ;
    spin->Information( I_SET_VALUE, &new_spin_int ) ;
    if ( entry->_access == FTP ) {
        spin->woFlags |= WOF_NON_SELECTABLE ;
        spin->Information(I_CHANGED_FLAGS,NULL);
    }

   InitColorLists() ;

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_FG ) ;
   SetComboToColor( combo, entry->_color_term_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_TERM_BG ) ;
   SetComboToColor( combo, entry->_color_term_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_FG ) ;
   SetComboToColor( combo, entry->_color_status_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_STATUS_BG ) ;
   SetComboToColor( combo, entry->_color_status_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_FG ) ;
   SetComboToColor( combo, entry->_color_popup_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_POPUP_BG ) ;
   SetComboToColor( combo, entry->_color_popup_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_FG ) ;
   SetComboToColor( combo, entry->_color_select_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_SELECT_BG ) ;
   SetComboToColor( combo, entry->_color_select_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_FG ) ;
   SetComboToColor( combo, entry->_color_underline_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_UNDERLINE_BG ) ;
   SetComboToColor( combo, entry->_color_underline_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_FG ) ;
   SetComboToColor( combo, entry->_color_command_fg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   combo = (UIW_COMBO_BOX *) Get( COMBO_COLOR_COMMAND_BG ) ;
   SetComboToColor( combo, entry->_color_command_bg ) ;
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

   InitTermCharsetList( (UIW_VT_LIST *) Get( LIST_TERM_CHARSET ) ) ;
   combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
   SetComboToCharset( combo, entry->_term_charset );
   combo->woFlags |= WOF_AUTO_CLEAR ;
   combo->Information(I_CHANGED_FLAGS,NULL) ;
    if ( entry->_access == FTP ) {
        combo->woFlags |= WOF_NON_SELECTABLE ;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_TERMINAL_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_TERMINAL_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_TERMINAL_SETTINGS::K_DIALOG_TERMINAL_SETTINGS(entry)\n");
};

EVENT_TYPE K_DIALOG_TERMINAL_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_TERMINAL_SETTINGS::Event()\n");

    extern K_CONNECTOR * connector ; 
    EVENT_TYPE retval = event.type ; 
    UIW_STRING * string = NULL ;
    UIW_BUTTON * button = NULL ;
    UIW_BUTTON * button2 = NULL ;
    UIW_COMBO_BOX * combo = NULL ;
    UIW_TEXT * text = NULL ;
    UIW_INTEGER * integer = NULL ;
    UIW_GROUP * group = NULL ;
    UIW_VT_LIST * list = NULL ;
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
    case OPT_HELP_SET_TERMINAL:
        helpSystem->DisplayHelp( windowManager, HELP_SET_TERMINAL ) ;
        break;

    case OPT_TERMINAL_OK: {
        // Check fields for completion requirements 
       // Widths must be even
       combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
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

   case	OPT_RADIO_ANSI:
   case OPT_RADIO_AVATAR:
       combo = (UIW_COMBO_BOX *) Get( COMBO_TERM_CHARSET ) ;
       combo->DataSet( "transparent" ) ;
       button = (UIW_BUTTON *) Get( RADIO_BITS_8 ) ;
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
   case OPT_RADIO_ADM3A:
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
        break;

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

void K_DIALOG_TERMINAL_SETTINGS::SetComboToColor( UIW_COMBO_BOX * combo, K_COLOR & color )
{
    combo->DataSet( color.Name() ) ; 
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_TERMINAL_SETTINGS::
ApplyChangesToEntry( void )
{
    // Terminal Page
    UIW_VT_LIST *list = (UIW_VT_LIST *) Get( LIST_TTYPE );
    UIW_BUTTON *button = (UIW_BUTTON *) list->Current();
    ZIL_ICHAR *ichar  = button->DataGet() ;

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
    else if ( !strcmp(ichar,"SUN") )
        _entry->_terminal = SUN;
    else if ( !strcmp(ichar,"ANNARBOR") )
        _entry->_terminal = ANNARBOR;
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
    else if ( !strcmp(ichar,"ADM-3A") )
        _entry->_terminal = ADM3A;
    else _entry->_terminal = ANSI ;

    if ( FlagSet(Get( RADIO_BITS_7 )->woStatus, WOS_SELECTED ) )
        _entry->_charsize = 7 ;
    else if ( FlagSet(Get( RADIO_BITS_8 )->woStatus, WOS_SELECTED ) )
        _entry->_charsize = 8 ;

    _entry->_local_echo = FlagSet(Get( CHECK_LOCAL_ECHO )->woStatus, WOS_SELECTED) ;
    _entry->_auto_wrap = FlagSet(Get( CHECK_AUTO_WRAP )->woStatus, WOS_SELECTED) ;
    _entry->_apc_enabled = FlagSet(Get( CHECK_APC )->woStatus, WOS_SELECTED) ;
    _entry->_auto_download = GetAutoDownList() ;
    _entry->_status_line = FlagSet( Get( CHECK_STATUS_LINE )->woStatus, WOS_SELECTED );

    UIW_COMBO_BOX *combo = (UIW_COMBO_BOX *) Get( COMBO_SCREEN_COL ) ;
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
}

void K_DIALOG_TERMINAL_SETTINGS::
InitTermTypeList( void )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_TTYPE ) ;
    *list + new UIW_BUTTON( 0,0,0,"ADM-3A",BTF_NO_3D|BTF_SEND_MESSAGE, WOF_NO_FLAGS,
			  ZIL_NULLF(ZIL_USER_FUNCTION), OPT_RADIO_ADM3A );
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

void K_DIALOG_TERMINAL_SETTINGS::
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

void K_DIALOG_TERMINAL_SETTINGS::
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

void K_DIALOG_TERMINAL_SETTINGS::
InitCursorList( void )
{
      UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_CURSOR ) ;
      *list
         + new UIW_BUTTON( 0,0,0,"full",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"half",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"underline",BTF_NO_3D, WOF_NO_FLAGS ) ;
}

void K_DIALOG_TERMINAL_SETTINGS::
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

void K_DIALOG_TERMINAL_SETTINGS::
InitWidthList( void )
{
      UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_SCREEN_COL ) ;
      *list
         + new UIW_BUTTON( 0,0,0,"80",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"132",BTF_NO_3D, WOF_NO_FLAGS ) ;
}

void K_DIALOG_TERMINAL_SETTINGS::InitTermCharsetList( UIW_VT_LIST * list )   
{
    enum TERMCSET charset = T_TRANSPARENT ;

    for (int i = 0; i < n_tcs; i++) {
        *list + new UIW_BUTTON(0,0,0,_tcs[i].name,
                                BTF_NO_3D, WOF_NO_FLAGS);
    }
}

   
K_COLOR K_DIALOG_TERMINAL_SETTINGS::
GetColorFromCombo( UIW_COMBO_BOX * combo )
{
   return K_COLOR( combo->DataGet() ) ;
}

enum TERMCSET K_DIALOG_TERMINAL_SETTINGS::
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

void K_DIALOG_TERMINAL_SETTINGS::
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

void K_DIALOG_TERMINAL_SETTINGS::
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
K_DIALOG_TERMINAL_SETTINGS::GetCursorFromCombo( UIW_COMBO_BOX * combo ) 
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

void
K_DIALOG_TERMINAL_SETTINGS::InitAutoDownList(void)
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_AUTODOWNLOAD ) ;
    *list
        + new UIW_BUTTON( 0,0,0,"No",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"Yes",BTF_NO_3D, WOF_NO_FLAGS )
        + new UIW_BUTTON( 0,0,0,"Ask",BTF_NO_3D, WOF_NO_FLAGS );
}

void
K_DIALOG_TERMINAL_SETTINGS::SetAutoDownList(enum YNA yna)
{
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *)Get(COMBO_AUTODOWNLOAD);
    switch ( yna ) {
    case YNA_NO:
        combo->DataSet("No");
        break;
    case YNA_YES:
        combo->DataSet("Yes");
        break;
    case YNA_ASK:
        combo->DataSet("Ask");
        break;
    }
}

enum YNA
K_DIALOG_TERMINAL_SETTINGS::GetAutoDownList(void)
{
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *)Get(COMBO_AUTODOWNLOAD);
    ZIL_ICHAR * value = combo->DataGet() ;
    enum YNA yna = YNA_ASK;

    if ( !strcmp("No", value) )
        yna = YNA_NO ;

    else if ( !strcmp("Yes", value) )
        yna = YNA_YES ;

    else if (!strcmp("Ask", value)) 
        yna = YNA_ASK ;

    return yna;
}

