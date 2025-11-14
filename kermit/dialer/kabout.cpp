#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_ABOUT_K95
#include "dialer.hpp"
#include "kabout.hpp"
#include "usermsg.hpp"
extern "C" {
#define _PROTOTYP( func, parms ) func parms
#include "ckoetc.h"
}

K_ABOUT::K_ABOUT() 
   :UIW_WINDOW("ABOUT_K95",defaultStorage)
{ 
   UIW_STRING * string = NULL ;
   windowManager->Center(this) ;

#ifdef WIN32
    Information( I_SET_TEXT, "About Kermit 95" ) ;
#else
    Information( I_SET_TEXT, "About Kermit 95 for OS/2" ) ;
#ifdef COMMENT
    UIW_ICON * icon = (UIW_ICON *) Get( ICON_K95 ) ;
    *this - icon ;
    delete icon ;
    icon = new UIW_ICON( 0,0,"CKNKER","About K95/2", ICF_MINIMIZE_OBJECT );
    *this + icon ;
#endif /* COMMENT */
#endif /* WIN32 */

#ifdef COMMENT
   int dsn = isregistered(NULL) ;
   if ( dsn >= 99 && dsn < 10000  /* Academic Site License */
        || dsn >= 10000           /* Retail Package */
        ) {
      string = (UIW_STRING *) Get( FIELD_REG_NAME ) ;
      string->DataSet( get_reg_name() ) ;
      string = (UIW_STRING *) Get( FIELD_REG_CORP ) ;
      string->DataSet( get_reg_corp() ) ;
      string = (UIW_STRING *) Get( FIELD_REG_SERIAL ) ;
      string->DataSet( get_reg_sn() ) ;
   }
   else
   {
      string = (UIW_STRING *) Get( FIELD_REG_NAME ) ;
      string->DataSet( "Unregistered" ) ;
   }
#endif
};

EVENT_TYPE K_ABOUT::Event( const UI_EVENT & event ) 
{ 
   EVENT_TYPE ccode = event.type ;

   switch ( event.type )
   {
      case OPT_ABOUT_OK:
         *windowManager - this ;
         break; 

      default:
         ccode = UIW_WINDOW::Event( event ) ;
   }
   return ccode;
}

