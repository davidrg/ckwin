#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_PRINTER_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetprinter.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"
#ifdef WIN32
extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#ifdef printf
#undef printf
#endif
#ifdef fprintf
#undef fprintf
#endif
#include <windows.h>            	/* Windows Definitions */
}
#endif /* WIN32 */

ZIL_ICHAR K_DIALOG_PRINTER_SETTINGS::_className[] = "K_DIALOG_PRINTER_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_PRINTER_SETTINGS
//
//

K_DIALOG_PRINTER_SETTINGS::
K_DIALOG_PRINTER_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_PRINTER_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    /* Printer page */

    InitPrinterTypeList();
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_PRINTER_TYPE ) ;
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

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_PRINT_FORMFEED );
    button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    UIW_STRING * string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
    string->woFlags |= WOF_AUTO_CLEAR;
    string->woFlags &= ~WOF_NON_SELECTABLE ;
    string->Information( I_CHANGED_FLAGS, NULL);

    UIW_SPIN_CONTROL * spin     = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT );
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( INTEGER_PRINT_TIMEOUT ) ;
    int new_spin_int = 0 ;
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

    UIW_INTEGER * integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    int value = 80 ;
    integer->Information( I_SET_VALUE, &value ) ;

    integer = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH);
    integer->woFlags &= ~WOF_NON_SELECTABLE ;
    integer->Information( I_CHANGED_FLAGS, NULL ) ;
    value = 66 ;
    integer->Information( I_SET_VALUE, &value ) ;


    InitTermCharsetList( (UIW_VT_LIST *) Get( LIST_PRN_CHARSET ) ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRN_CHARSET ) ;
    combo->DataSet( "cp437" ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_PRINTER_SETTINGS
//
//

K_DIALOG_PRINTER_SETTINGS::
K_DIALOG_PRINTER_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_PRINTER_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    ZIL_ICHAR itoabuf[40] ;
    printf("Entering K_DIALOG_PRINTER_SETTINGS::K_DIALOG_PRINTER_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Printer Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    /* Printer page */
    InitPrinterTypeList();
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_PRINTER_TYPE ) ;
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

    UIW_STRING * string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
    string->DataSet( entry->_printer_separator);
    string->woFlags |= WOF_AUTO_CLEAR;
    string->Information( I_CHANGED_FLAGS, NULL);

    UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECK_PRINT_FORMFEED );
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

    UIW_SPIN_CONTROL * spin = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT ) ;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) spin->Get( INTEGER_PRINT_TIMEOUT ) ;
    int new_spin_int = entry->_printer_timeout ;
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

    UIW_INTEGER * integer = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH);
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

    InitTermCharsetList( (UIW_VT_LIST *) Get( LIST_PRN_CHARSET ) ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_PRN_CHARSET ) ;
    SetComboToCharset( combo, entry->_printer_charset ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_PRINTER_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_PRINTER_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_PRINTER_SETTINGS::K_DIALOG_PRINTER_SETTINGS(entry,mode)\n");
};

K_DIALOG_PRINTER_SETTINGS::~K_DIALOG_PRINTER_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_PRINTER_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_PRINTER_SETTINGS::Event()\n");

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
    case OPT_HELP_SET_PRINTER:
        helpSystem->DisplayHelp( windowManager, HELP_SET_PRINTER ) ;
        break;

    case OPT_PRINTER_OK: {
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

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_PRINTER_SETTINGS::
ApplyChangesToEntry( void )
{
    /* Printer page */
    _entry->_printer_type = 
        GetPrinterTypeFromCombo( (UIW_COMBO_BOX *) Get(COMBO_PRINTER_TYPE) );

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_DOS_DEVICE ) ;
    strncpy( _entry->_printer_dos, combo->DataGet(), 256 ) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_WINDOWS ) ;
    strncpy( _entry->_printer_windows, combo->DataGet(), 256 ) ;

    UIW_STRING * string = (UIW_STRING *) Get( STRING_PRINT_SEPARATOR );
    strncpy( _entry->_printer_separator, string->DataGet(), 256 ) ;

    _entry->_printer_formfeed = 
        FlagSet(Get( CHECK_PRINT_FORMFEED )->woStatus, WOS_SELECTED );

    string = (UIW_STRING *) Get( STRING_PRINT_TERMINATOR );
    strncpy( _entry->_printer_terminator, string->DataGet(), 64 ) ;

    UIW_SPIN_CONTROL * spin = (UIW_SPIN_CONTROL *) Get( SPIN_PRINTER_TIMEOUT );
    int value;
    spin->Information( I_GET_VALUE, &value );
    _entry->_printer_timeout = value;

    _entry->_printer_bidi =
        FlagSet(Get( CHECK_PRINT_BIDI )->woStatus, WOS_SELECTED );

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_SPEED ) ;
    _entry->_printer_speed = atoi( combo->DataGet() ) ;

    combo = (UIW_COMBO_BOX *) Get( COMBO_PRINT_FLOW );
    ZIL_ICHAR * ichar = combo->DataGet() ;
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

    int new_spin_int;
    UIW_INTEGER * spin_int = (UIW_INTEGER *) Get( INTEGER_PRINT_WIDTH ) ;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_printer_width = new_spin_int;

    spin_int = (UIW_INTEGER *) Get( INTEGER_PRINT_LENGTH ) ;
    spin_int->Information( I_GET_VALUE, &new_spin_int ) ;
    _entry->_printer_length = new_spin_int;

    _entry->_printer_charset = GetTermCharsetFromCombo( (UIW_COMBO_BOX *) Get( COMBO_PRN_CHARSET ) );
}


void K_DIALOG_PRINTER_SETTINGS::
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

enum PRINTER_TYPE K_DIALOG_PRINTER_SETTINGS::
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

void K_DIALOG_PRINTER_SETTINGS::
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
Win32EnumPrt( struct keytab ** pTable, struct keytab ** pTable2, int * pN )
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

    if ((pPrtInfo2 = (LPPRINTER_INFO_2)LocalAlloc (LPTR, dwBytesNeeded)) == NULL)
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


void K_DIALOG_PRINTER_SETTINGS::
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

void K_DIALOG_PRINTER_SETTINGS::
InitPrinterFlowList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_PRINT_FLOW ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Xon/Xoff",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Rts/Cts",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_DIALOG_PRINTER_SETTINGS::
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

void K_DIALOG_PRINTER_SETTINGS::
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

void K_DIALOG_PRINTER_SETTINGS::InitTermCharsetList( UIW_VT_LIST * list )   
{
    enum TERMCSET charset = T_CP437;

    for (int i = 0; i < n_tcs; i++) {
        *list + new UIW_BUTTON(0,0,0,_tcs[i].name,
                                BTF_NO_3D, WOF_NO_FLAGS);
    }
}

enum TERMCSET K_DIALOG_PRINTER_SETTINGS::
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

void K_DIALOG_PRINTER_SETTINGS::
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

