#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_SERIAL_SETTINGS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetserial.hpp"
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

ZIL_ICHAR K_DIALOG_SERIAL_SETTINGS::_className[] = "K_DIALOG_SERIAL_SETTINGS" ;

extern K_CONNECTOR * connector ;

// Null Constructor for K_DIALOG_SERIAL_SETTINGS
//
//

K_DIALOG_SERIAL_SETTINGS::
K_DIALOG_SERIAL_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_SERIAL_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
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
    K_MODEM      * modem = connector->FindModem( "DEFAULT" );
    if ( !modem )
	modem = connector->FindModem( NULL );
    ZIL_ICHAR itoabuf[40] ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // Configure default values
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

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}

// KD_LIST_ITEM Constructor for K_DIALOG_SERIAL_SETTINGS
//
//

K_DIALOG_SERIAL_SETTINGS::
K_DIALOG_SERIAL_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode)
   : K95_SETTINGS_DIALOG("DIALOG_SERIAL_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_SERIAL_SETTINGS::K_DIALOG_SERIAL_SETTINGS(entry)\n");

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
    ZIL_ICHAR itoabuf[40] ;

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[128];
    sprintf(dialogName,"Serial Communications Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    K_MODEM * modem = connector->FindModem( entry->_modem );

    // Apply entry to Notebook
   // Communications
    InitModemList() ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
    combo->DataSet( entry->_modem ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
	 _entry->_access == SUPERLAT ||
	 _entry->_access == DIRECT ||
         _entry->_access == FTP)
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS,NULL) ;

    InitLineList( 0 ) ;
    combo = (UIW_COMBO_BOX *) Get( COMBO_LINES ) ;
    combo->DataSet( (!modem || entry->_access == DIRECT) ? entry->_line_device : modem->_port ) ;
    combo->woFlags |= WOF_AUTO_CLEAR ;
    if ( _entry->_access == TCPIP || 
         _entry->_access == SUPERLAT || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == PHONE )
	combo->woFlags |= WOF_NON_SELECTABLE ;
    combo->Information(I_CHANGED_FLAGS, NULL);

    button = (UIW_BUTTON *) Get( CHECK_OVER_PARITY );
    if ( !_entry->_use_mdm_parity )
	button->woStatus |= WOS_SELECTED ;
    else
	button->woStatus &= ~WOS_SELECTED ;
    button->Information(I_CHANGED_STATUS,NULL);
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP ||
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT )
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
    if ( _entry->_access == TCPIP || 
         _entry->_access == SSH ||
         _entry->_access == FTP ||
         _entry->_access == SUPERLAT || _entry->_access == DIRECT )
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

    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_SERIAL_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_SERIAL_CANCEL);
        button->DataSet("Cancel");
    }

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_SERIAL_SETTINGS::K_DIALOG_SERIAL_SETTINGS(entry)\n");
};

K_DIALOG_SERIAL_SETTINGS::~K_DIALOG_SERIAL_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_SERIAL_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_SERIAL_SETTINGS::Event()\n");

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

    case OPT_HELP_SET_COMMUNICATIONS:
        helpSystem->DisplayHelp( windowManager, HELP_SET_COMMUNICATIONS ) ;
        break;

    case OPT_SERIAL_OK: {
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

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_SERIAL_SETTINGS::
ApplyChangesToEntry( void )
{
    
   // Communications
    _entry->_tapi_line = 0;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_MODEM ) ;
    strncpy( _entry->_modem, combo->DataGet(), 60 ) ;
    UIW_BUTTON * button = (UIW_BUTTON *) combo->Current();
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
    ZIL_ICHAR * ichar = combo->DataGet() ;
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
}

void K_DIALOG_SERIAL_SETTINGS::
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

void K_DIALOG_SERIAL_SETTINGS::
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

void K_DIALOG_SERIAL_SETTINGS::
InitFlowList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_FLOW ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"None",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Xon/Xoff",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Rts/Cts",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"Auto",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_DIALOG_SERIAL_SETTINGS::
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

void K_DIALOG_SERIAL_SETTINGS::
InitStopBitList()
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_STOPBITS ) ;
    *list 
	+ new UIW_BUTTON( 0,0,0,"1.0",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"1.5",BTF_NO_3D, WOF_NO_FLAGS )
	+ new UIW_BUTTON( 0,0,0,"2.0",BTF_NO_3D, WOF_NO_FLAGS );
}

void K_DIALOG_SERIAL_SETTINGS::InitSpeedList( void ) 
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

