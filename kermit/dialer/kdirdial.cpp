#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_HELP_CONTEXTS
#define USE_WINDOW_DIAL_DIR_FILES
#include <stdio.h>
#include "dialer.hpp"
#include "kdirdial.hpp"
#include "kdconfig.hpp"
#include "usermsg.hpp"
#include "kconnect.hpp"

extern K_CONNECTOR * connector ;

K_DIAL_DIR::K_DIAL_DIR( KD_CONFIG * config) 
   :ZAF_DIALOG_WINDOW("WINDOW_DIAL_DIR_FILES",defaultStorage), _config(config)
{ 
#ifdef WIN32
    Information( I_SET_TEXT, "C-Kermit for Windows Dialing Directory Files" ) ;
#else
    Information( I_SET_TEXT, "C-Kermit for OS/2 Dialing Directory Files" ) ;
#ifdef COMMENT
    UIW_ICON * icon = (UIW_ICON *) Get( ICON_K95 ) ;
    *this - icon ;
    delete icon ;
    icon = new UIW_ICON( 0,0,"CKNKER","K95/2 Dialing Directory Files", 
			 ICF_MINIMIZE_OBJECT );
    *this + icon ;
#endif /* COMMENT */
#endif /* WIN32 */

   UIW_STRING * string = (UIW_STRING *) Get( FIELD_DIAL_DIR_NAMES ) ;
   string->DataSet( _config->_phone_dir ) ;

   UIW_BUTTON * button = (UIW_BUTTON *) Get( CHECKBOX_USE_DIAL_DIRS ) ;
   if ( _config->_use_phone_dir )
   {
      button->woStatus |= WOS_SELECTED ;
      button->Information( I_CHANGED_STATUS, NULL ) ;
   }
   else
   {
      string->woFlags |= WOF_NON_SELECTABLE ;
      string->Information(I_CHANGED_FLAGS,NULL) ;
   }
   windowManager->Center( this ) ;
};

EVENT_TYPE K_DIAL_DIR::Event( const UI_EVENT & event ) 
{ 
   EVENT_TYPE ccode = event.type ;
   UIW_BUTTON * button = NULL ;
   UIW_STRING * string = NULL ;

   switch ( event.type )
   {
      case OPT_HELP_DIAL_DIRECTORIES:
          helpSystem->DisplayHelp( windowManager, HELP_DIAL_DIRECTORIES ) ;
          break;

      case OPT_DIAL_DIR_OK:
         button = (UIW_BUTTON *) Get( CHECKBOX_USE_DIAL_DIRS ) ;
         string = (UIW_STRING *) Get( FIELD_DIAL_DIR_NAMES ) ;
         _config->_use_phone_dir = FlagSet( button->woStatus, WOS_SELECTED ) ;
         strncpy( _config->_phone_dir, string->DataGet(), 2000 ) ;
         connector->SaveConfig() ;
         *windowManager - this ;
         break; 

      case OPT_DIAL_DIR_CHECK:
         button = (UIW_BUTTON *) Get( CHECKBOX_USE_DIAL_DIRS ) ;
         string = (UIW_STRING *) Get( FIELD_DIAL_DIR_NAMES ) ;
         if ( FlagSet( button->woStatus, WOS_SELECTED ) )
         {
            string->woFlags &= ~WOF_NON_SELECTABLE ;
         }
         else
         {
               string->woFlags |= WOF_NON_SELECTABLE ;
         }
         string->Information( I_CHANGED_FLAGS, NULL ) ;
         break;

      default:
         ccode = UIW_WINDOW::Event( event ) ;
   }
   return ccode;
}

