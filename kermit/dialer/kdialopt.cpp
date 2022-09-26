#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include <stdio.h>
#define USE_HELP_CONTEXTS
#define USE_WINDOW_LOCATION
#define USE_WINDOW_NEW_LOCATION
#include <stdio.h>
#include "dialer.hpp"
#include "kdialopt.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"
#include "klocation.hpp"

#ifdef WIN32
extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#undef printf
#undef fprintf
#include <windows.h>            	/* Windows Definitions */
#ifndef NODIAL
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
#endif
}
#endif 

ZIL_ICHAR K_DIAL_OPTIONS::_className[] = "K_DIAL_OPTIONS" ;
extern K_CONNECTOR  *connector;

K_DIAL_OPTIONS::K_DIAL_OPTIONS(void)
   : ZAF_DIALOG_WINDOW("WINDOW_LOCATION",defaultStorage)
{
   windowManager->Center(this) ;
   relative.bottom = relative.Height() - 1 ;
   relative.top = 0 ;
}

K_DIAL_OPTIONS::K_DIAL_OPTIONS( UI_LIST * LocationList )
   : ZAF_DIALOG_WINDOW("WINDOW_LOCATION",defaultStorage),SourceList(LocationList)
{
    windowManager->Center(this) ;

    _location_list = (UIW_VT_LIST *) Get( LIST_LOCATION );
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_LOCATION);

    K_LOCATION * item = NULL;
    K_LOCATION * newitem = NULL;
    _current = NULL;
    UI_EVENT AddObject( S_ADD_OBJECT );
    for ( item = (K_LOCATION *) LocationList->First(); item ; 
	  item = (K_LOCATION *) item->Next() ) {
        newitem = new K_LOCATION( *item );
	newitem->DataSet( newitem->_name );
	newitem->_dialog = this;
	AddObject.data = newitem;
	//*_location_list + newitem ;
	_location_list->Event(AddObject);
	if ( item == LocationList->Current() )
	{
	    _location_list->SetCurrent( newitem );
	    *combo + newitem;
	    _current = newitem;
	}
    }

    if ( _current == NULL ) {
	_current = (K_LOCATION *) _location_list->First();
	_location_list->SetCurrent( _current );
	*combo + _current;
    }

    if ( _current )
	*combo + _current;

#ifdef WIN32
    Information( I_SET_TEXT, "C-Kermit for Windows Location Definitions" ) ;
#else
    Information( I_SET_TEXT, "C-Kermit for OS/2 Location Definitions" ) ;
#ifdef COMMENT
    UIW_ICON * icon = (UIW_ICON *) Get( ICON_K95 ) ;
    *this - icon ;
    delete icon ;
    icon = new UIW_ICON( 0,0,"CKNKER","K95/2 Locations", 
			 ICF_MINIMIZE_OBJECT );
    *this + icon ;
#endif /* COMMENT */
#endif /* WIN32 */

    CopyValuesFromCurrent();

//    UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_LOCATION_ADD );
//    if ( button ) {
//	button->woFlags |= WOF_NON_SELECTABLE ;
//	button->Information(I_CHANGED_FLAGS,NULL) ;
//    }

    if ( combo->Count() <= 1 ) {
	UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_LOCATION_REMOVE );
	if ( button ) {
	    button->woFlags |= WOF_NON_SELECTABLE ;
	    button->Information(I_CHANGED_FLAGS,NULL) ;
	}
    }
}

void
K_DIAL_OPTIONS::CopyValuesFromCurrent()
{
    if ( _current == NULL )
	return;

    /* need to configure window settings */
    UIW_STRING * string = (UIW_STRING *) Get( FIELD_COUNTRY_CODE ) ;
    string->DataSet( _current->_country_code, 3 );
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_AREA_CODE ) ;
    string->DataSet( _current->_area_code, 8 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_DIALING_P ) ;
    string->DataSet( _current->_dial_prefix, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_DIALING_S ) ;
    string->DataSet( _current->_dial_suffix, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    UIW_BUTTON * button = ( UIW_BUTTON  * ) Get( CHECK_FORCE_LD ) ;
    if (_current->_force_long_dist ) 
	button->woStatus |= WOS_SELECTED ;
    else 
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_P ) ;
    string->DataSet( _current->_local_area_prefix, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    if ( _current->_force_long_dist )
        string->woFlags |= WOF_NON_SELECTABLE;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_S ) ;
    string->DataSet( _current->_local_area_suffix, 32 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    if ( _current->_force_long_dist )
        string->woFlags |= WOF_NON_SELECTABLE;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCAL_AC_LIST ) ;
    string->DataSet( _current->_local_area_codes_list, 128 ) ;
    string->woFlags |= WOF_AUTO_CLEAR ;
    if ( _current->_force_long_dist )
        string->woFlags |= WOF_NON_SELECTABLE;
    string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_LOCAL_LD_P ) ;
   string->DataSet( _current->_long_dist_prefix, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

   string = (UIW_STRING *) Get( FIELD_LOCAL_LD_S ) ;
   string->DataSet( _current->_long_dist_suffix, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

   string = (UIW_STRING *) Get( FIELD_INTL_P ) ;
   string->DataSet( _current->_intl_prefix, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

   string = (UIW_STRING *) Get( FIELD_INTL_S ) ;
   string->DataSet( _current->_intl_suffix, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

    string = (UIW_STRING *) Get( FIELD_TOLLFREE ) ;
   string->DataSet( _current->_dial_tf_area, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

   string = (UIW_STRING *) Get( FIELD_TOLL_FREE_PREFIX ) ;
   string->DataSet( _current->_dial_tf_prefix, 32 ) ;
   string->woFlags |= WOF_AUTO_CLEAR ;
   string->Information(I_CHANGED_FLAGS,NULL) ;

    button = ( UIW_BUTTON  * ) Get( CHECK_LOCALE_PBX ) ;
    if (_current->_use_pbx ) {
	button->woStatus |= WOS_SELECTED ;
	button->Information( I_CHANGED_STATUS, NULL ) ;

	string = (UIW_STRING *) Get( FIELD_PBX_OUT ) ;
	string->DataSet( _current->_pbx_out, 32 ) ;
	string->woFlags |= WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( FIELD_PBX_EXCH ) ;
	string->DataSet( _current->_pbx_exch, 32 ) ;
	string->woFlags |= WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( FIELD_PBX_IN ) ;
	string->DataSet( _current->_pbx_in, 32 ) ;
	string->woFlags |= WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
    }
    else  {
	button->woStatus &= ~WOS_SELECTED ;
	button->Information( I_CHANGED_STATUS, NULL ) ;

	string = (UIW_STRING *) Get( FIELD_PBX_OUT ) ;
	string->DataSet( _current->_pbx_out, 32 ) ;
	string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( FIELD_PBX_EXCH ) ;
	string->DataSet( _current->_pbx_exch, 32 ) ;
	string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;

	string = (UIW_STRING *) Get( FIELD_PBX_IN ) ;
	string->DataSet( _current->_pbx_in, 32 ) ;
	string->woFlags |= WOF_NON_SELECTABLE | WOF_AUTO_CLEAR ;
	string->Information(I_CHANGED_FLAGS,NULL) ;
    }

   UIW_GROUP * group = (UIW_GROUP *) Get( GROUP_DIALING_PROC_METHOD ) ;
   switch (_current->_dial_method) {
     case Tone:
       button = (UIW_BUTTON *) Get( RADIO_DIALING_PROC_METHOD_TONE );
       break;
     case Pulse:
       button = (UIW_BUTTON *) Get( RADIO_DIALING_PROC_METHOD_PULSE );
       break;
   }
    button->woStatus |= WOS_SELECTED ;
    button->Information(I_CHANGED_FLAGS,NULL);
   * group + button ;

   UIW_INTEGER * integer = (UIW_INTEGER *) Get ( INTEGER_REDIALS ) ;
   int value = _current->_redial_count ;
   integer->DataSet( &value );

   integer = (UIW_INTEGER *) Get ( INTEGER_INTERVAL ) ;
   value = _current->_redial_interval ;
   integer->DataSet( &value );

   integer = (UIW_INTEGER *) Get ( INTEGER_TIMEOUT ) ;
   value = _current->_dial_timeout ;
   integer->DataSet( &value );

    button = ( UIW_BUTTON  * ) Get( CHECK_IGNORE_DIALTONE ) ;
    if (_current->_ignore_dial_tone ) 
	button->woStatus |= WOS_SELECTED ;
    else 
	button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
}

void
K_DIAL_OPTIONS::CopyValuesToCurrent()
{
    if ( _current == NULL )
	return;

    UIW_STRING * string = (UIW_STRING *) Get( FIELD_COUNTRY_CODE ) ;
    strncpy( _current->_country_code, string->DataGet(), 3);

    string = (UIW_STRING *) Get( FIELD_AREA_CODE ) ;
    strncpy( _current->_area_code, string->DataGet(), 8);

    string = (UIW_STRING *) Get( FIELD_DIALING_P ) ;
    strncpy( _current->_dial_prefix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_DIALING_S ) ;
    strncpy( _current->_dial_suffix, string->DataGet(), 32);

    UIW_BUTTON * button = ( UIW_BUTTON  * ) Get( CHECK_FORCE_LD ) ;
    _current->_force_long_dist = (button->woStatus & WOS_SELECTED) ;

    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_P ) ;
    strncpy( _current->_local_area_prefix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_S ) ;
    strncpy( _current->_local_area_suffix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_LOCAL_AC_LIST ) ;
    strncpy( _current->_local_area_codes_list, string->DataGet(), 128);

    string = (UIW_STRING *) Get( FIELD_LOCAL_LD_P ) ;
    strncpy( _current->_long_dist_prefix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_LOCAL_LD_S ) ;
    strncpy( _current->_long_dist_suffix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_INTL_P ) ;
    strncpy( _current->_intl_prefix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_INTL_S ) ;
    strncpy( _current->_intl_suffix, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_TOLLFREE ) ;
    strncpy( _current->_dial_tf_area, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_TOLL_FREE_PREFIX ) ;
    strncpy( _current->_dial_tf_prefix, string->DataGet(), 32);

    button = ( UIW_BUTTON  * ) Get( CHECK_LOCALE_PBX ) ;
    _current->_use_pbx = (button->woStatus & WOS_SELECTED) ;

    string = (UIW_STRING *) Get( FIELD_PBX_OUT ) ;
    strncpy( _current->_pbx_out, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_PBX_EXCH ) ;
    strncpy( _current->_pbx_exch, string->DataGet(), 32);

    string = (UIW_STRING *) Get( FIELD_PBX_IN ) ;
    strncpy( _current->_pbx_in, string->DataGet(), 32);

    button = (UIW_BUTTON *) Get( RADIO_DIALING_PROC_METHOD_TONE );
    if ( button->woStatus & WOS_SELECTED )
	_current->_dial_method = Tone;
    else
	_current->_dial_method = Pulse;

    UIW_INTEGER * integer = (UIW_INTEGER *) Get ( INTEGER_REDIALS ) ;
   _current->_redial_count = integer->DataGet();

    integer = (UIW_INTEGER *) Get ( INTEGER_INTERVAL ) ;
    _current->_redial_interval = integer->DataGet();

    integer = (UIW_INTEGER *) Get ( INTEGER_TIMEOUT ) ;
    _current->_dial_timeout = integer->DataGet();

    button = ( UIW_BUTTON  * ) Get( CHECK_IGNORE_DIALTONE ) ;
    _current->_ignore_dial_tone = (button->woStatus & WOS_SELECTED) ;

}

void
K_DIAL_OPTIONS::AddLocation( ZIL_ICHAR * name )
{
    CopyValuesToCurrent();
    K_LOCATION * newitem = new K_LOCATION();
    strcpy( newitem->_name, name );
    newitem->DataSet( name );
    newitem->_dialog = this;
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_LOCATION);
    UI_EVENT AddObject(S_ADD_OBJECT) ;
    AddObject.data = newitem ;
    _location_list->Event(AddObject);
    combo->Event(AddObject);
    //*_location_list + newitem;
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    UI_EVENT Create(S_CREATE) ;
    combo->Event( Create );
    combo->Event( ReDisplay );
#ifdef __OS2__
    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
    *combo + newitem;
    _current = (K_LOCATION *) combo->Current();
    if ( combo->Count() > 1 ) {
	UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_LOCATION_REMOVE );
	if ( button ) {
	    button->woFlags &= ~WOF_NON_SELECTABLE ;
	    button->Information(I_CHANGED_FLAGS,NULL) ;
	}
    }
}

void
K_DIAL_OPTIONS::ApplyChanges( void )
{
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_LOCATION);
    K_LOCATION * item = NULL;
    K_LOCATION * newCurrent = _current ;

    for ( item = (K_LOCATION *) _delete_list.First(); item ; 
	  item = (K_LOCATION *) _delete_list.First() ) {
	_delete_list - item ;
	if (K_CONNECTOR::_userFile->ChDir("~Locations") == 0) {
            if (K_CONNECTOR::_userFile->DestroyObject( item->_name ) != 0)
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                            "Unable to remove location \"%s\": errorno %d", 
                                            item->_name,
                                            K_CONNECTOR::_userFile->storageError ) ;
                message->Control();
                delete message ;
                return;
            }
            K_CONNECTOR::_userFile->ChDir("~");
        }
	delete item;
    }
    K_CONNECTOR::_userFile->Save(2) ;

    CopyValuesToCurrent();

    SourceList->Destroy();

    UI_EVENT  SubtractObject(S_SUBTRACT_OBJECT);
    for ( item = (K_LOCATION *) _location_list->First(); item ; 
	  item = (K_LOCATION *) _location_list->First() ) {
	item->_dialog = NULL;
	if ( strcmp( item->_name, item->DataGet() ) )
	    strncpy( item->_name, item->DataGet(), 60 );

	if (K_CONNECTOR::_userFile->ChDir("~Locations") == 0) {
            ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
                                         item->_name, ID_K_LOCATION,
				     UIS_OPENCREATE | UIS_READWRITE ) ;
            item->Store( item->_name, K_CONNECTOR::_userFile, &_fileObj,   
			 ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
            if ( K_CONNECTOR::_userFile->storageError )
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					"Unable to Write location \"%s\": errorno %d", 
					item->_name,
                                            K_CONNECTOR::_userFile->storageError ) ;
                message->Control() ;	
                delete message ;
            }   
            K_CONNECTOR::_userFile->ChDir("~");
        }
	SubtractObject.data = item;
	_location_list->Event(SubtractObject);
	*SourceList + item;
	if ( item == newCurrent ) {
	    SourceList->SetCurrent( item );
#if defined(WIN32)
#ifndef NODIAL
	    if ( item->_is_tapi )
		cktapiSetCurrentLocationID( item->_tapi_location_id );
#endif
#endif
	}
    }
}

K_DIAL_OPTIONS::~K_DIAL_OPTIONS( void ) 
{
    
}

EVENT_TYPE K_DIAL_OPTIONS::Event( const UI_EVENT & event )
{
    EVENT_TYPE ccode = event.type ;

    switch ( event.type ) {
    case OPT_DIAL_HELP_GENERAL:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATION_GENERAL ) ;
	break;
    case OPT_DIAL_HELP_LA:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATION_LOCAL ) ;
	break;
    case OPT_DIAL_HELP_LD:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATIONS_LD ) ;
	break;
    case OPT_DIAL_HELP_INTL:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATION_INTL ) ;
	break;
    case OPT_DIAL_HELP_TF:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATION_TF ) ;
	break;
    case OPT_DIAL_HELP_PBX:
	helpSystem->DisplayHelp( windowManager, HELP_LOCATION_PBX ) ;
	break;

    case OPT_DIALING_PBX: {
	UIW_BUTTON * button = ( UIW_BUTTON  * ) Get( CHECK_LOCALE_PBX ) ;
	UIW_STRING * string;

	if ( FlagSet(button->woStatus, WOS_SELECTED) ) {
	    string = (UIW_STRING *) Get( FIELD_PBX_OUT ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_PBX_EXCH ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_PBX_IN ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;
	}
	else  {
	    string = (UIW_STRING *) Get( FIELD_PBX_OUT ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_PBX_EXCH ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_PBX_IN ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;
	}	
    }
	break;

    case OPT_DIALING_OK: {
	*windowManager - this ;
	ApplyChanges() ;
	connector->UpdateLocationMenuItems();
	break;
    }

    case OPT_DIAL_LOCATION_OBJECT: {
	UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_LOCATION);
	CopyValuesToCurrent();
	_current = (K_LOCATION *) combo->Current();
	CopyValuesFromCurrent();
	break;
								   }

    case OPT_DIAL_LOCATION_ADD: {
	K_LOC_NAME * newname = new K_LOC_NAME( this );
        newname->Control();
        delete newname;
	break;
    }

    case OPT_DIAL_LOCATION_REMOVE: {
	UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_LOCATION);
	if ( combo->Count() > 1 ) {
	    UI_EVENT ReDisplay(S_REDISPLAY) ;
	    UI_EVENT Create(S_CREATE) ;
	    UI_EVENT SubtractObject(S_SUBTRACT_OBJECT) ;
	    K_LOCATION * deletion = _current;
	    SubtractObject.data = _current;
	    //*combo - _current ;
	    combo->Event(SubtractObject);
	    combo->Event( Create );
	    combo->Event( ReDisplay );
#ifdef __OS2__
	    combo->Information(I_CHANGED_FLAGS,NULL);
#endif
	    _current = (K_LOCATION *) combo->Current();
	    CopyValuesFromCurrent();
	    _delete_list + deletion ;
	    if ( combo->Count() == 1 ) {
		UIW_BUTTON * button = (UIW_BUTTON *) Get( BUTTON_LOCATION_REMOVE );
		if ( button ) {
		    button->woFlags |= WOF_NON_SELECTABLE ;
		    button->Information(I_CHANGED_FLAGS,NULL) ;
		}
	    }
	}
	break;
    }

    case OPT_DIAL_FORCE_LD: {
        UIW_BUTTON * button = (UIW_BUTTON *) Get(CHECK_FORCE_LD);
	UIW_STRING * string;

	if ( !FlagSet(button->woStatus, WOS_SELECTED) ) {
	    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_P ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_S ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_LOCAL_AC_LIST ) ;
	    string->woFlags &= ~WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;
        }
        else {
	    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_P ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_LOCAL_LA_S ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;

	    string = (UIW_STRING *) Get( FIELD_LOCAL_AC_LIST ) ;
	    string->woFlags |= WOF_NON_SELECTABLE ;
	    string->Information(I_CHANGED_FLAGS,NULL) ;
        }
        break;
    }
    default:
	ccode = UIW_WINDOW::Event(event);
    }
    return ccode ;
}

K_LOC_NAME::K_LOC_NAME( K_DIAL_OPTIONS * Parent )
    : ZAF_DIALOG_WINDOW("WINDOW_NEW_LOCATION",defaultStorage), _parent(Parent) 
{
    windowManager->Center(this);
}

EVENT_TYPE K_LOC_NAME::Event( const UI_EVENT & event )
{
   EVENT_TYPE ccode = event.type ;
   UIW_STRING * string = NULL ;

   switch ( event.type ) {
      case OPT_DIALING_OK: {
	  UIW_STRING * string = (UIW_STRING *) Get( FIELD_NAME );
	  _parent->AddLocation( string->DataGet() );
	  *windowManager - this ;
	  break;
      }

      default:
         ccode = UIW_WINDOW::Event(event);
   }
   return ccode ;
}

K_LOC_NAME::~K_LOC_NAME( void ) 
{
    
}

