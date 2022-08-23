#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_WINDOW_DIALER
#define USE_WINDOW_QUICK
#define USE_HELP_CONTEXTS
#include <stdio.h>
#include "dialer.hpp"
#include "kenum.hpp"
#include "kquick.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"
#include "lstitm.hpp"
#include "kdconfig.hpp"
#include "kstatus.hpp"
#ifdef COMMENT
#undef COMMENT
#endif /* COMMENT */

extern K_CONNECTOR * connector ;

K_QUICK::K_QUICK( KD_CONFIG * config , KD_LIST_ITEM * templ)
   : UIW_WINDOW("WINDOW_QUICK",defaultStorage), _config(config), 
   _template(templ)
{
#ifdef WIN32
    Information( I_SET_TEXT, "Kermit 95 Quick Connect" ) ;
#else
    Information( I_SET_TEXT, "Kermit 95 for OS/2 Quick Connect" ) ;
#endif /* WIN32 */

    PopulateList(PHONE);
    windowManager->Center(this);
}

void K_QUICK::PopulateList( enum TRANSPORT type )
{
    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get( COMBO_QUICK );
    UIW_VT_LIST * quicklist  = (UIW_VT_LIST *) Get( LIST_QUICK );
    UIW_BUTTON * button = NULL;
    while ( button = (UIW_BUTTON *) quicklist->First() ) {
        *quicklist - button;
        delete button;
    }

    UIW_VT_LIST * list = (UIW_VT_LIST *) connector->Get( LIST_ENTRIES ) ;
    KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
    KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
    if ( listitem )
        do { 
            if ( listitem->_access == type )
                *quicklist + new UIW_BUTTON( 0,0,0,listitem->_name,
                                            BTF_NO_3D, 
                                            WOF_NO_FLAGS,
                                            ZIL_NULLF(ZIL_USER_FUNCTION),
                                            OPT_LINE_DEVICE_IS_TEMPLATE) ;
            if (listitem == lastlistitem )
                break ;
            else
                listitem = listitem->Next() ;
        } while ( listitem );
    
    listitem = (KD_LIST_ITEM *) connector->GetEntryList()->First() ;
    lastlistitem = (KD_LIST_ITEM *) connector->GetEntryList()->Last() ;
    if ( listitem )
        do { 
            if ( listitem->_access == type )
                *quicklist + new UIW_BUTTON( 0,0,0,listitem->_name,
                                            BTF_NO_3D, 
                                            WOF_NO_FLAGS,
                                            ZIL_NULLF(ZIL_USER_FUNCTION),
                                            OPT_LINE_DEVICE_IS_TEMPLATE) ;
            if (listitem == lastlistitem )
                break ;
            else
                listitem = listitem->Next() ;
        } while ( listitem );


    listitem = (KD_LIST_ITEM *) quicklist->First();
    if ( !listitem ) {
        UIW_STRING * string = (UIW_STRING *) Get( FIELD_HOST_PHONE ) ;
        string->woFlags |= WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS, NULL);
        combo->DataSet( "(no entries defined)" ) ;
        combo->woFlags |= WOF_AUTO_CLEAR | WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS, NULL);
        button = (UIW_BUTTON *) Get(BUTTON_QUICK_OK);
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS, NULL);
    } else {
        UIW_STRING * string = (UIW_STRING *) Get( FIELD_HOST_PHONE ) ;
        string->woFlags &= ~WOF_NON_SELECTABLE;
        string->Information(I_CHANGED_FLAGS, NULL);
        combo->DataSet( "(choose one)" ) ;
        combo->woFlags |= WOF_AUTO_CLEAR;
        combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS, NULL);
        button = (UIW_BUTTON *) Get(BUTTON_QUICK_OK);
        button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS, NULL);
    }
}

EVENT_TYPE K_QUICK::Event( const UI_EVENT & event )
{
    EVENT_TYPE ccode = event.type ;
    ZAF_MESSAGE_WINDOW * message ;
    UIW_STRING * string = NULL ;
    ZIL_UINT32 process_id;

   switch ( event.type ) {
   case OPT_PHONE_CALL: 
       PopulateList(PHONE);
       break;
   case OPT_TELNET:
       PopulateList(TCPIP);
       break;
   case OPT_SUPERLAT:
       PopulateList(SUPERLAT);
       break;
   case OPT_SSH:
       PopulateList(SSH);
       break;
   case OPT_FTP:
       PopulateList(FTP);
       break;

   case OPT_HELP_QUICK:
       helpSystem->DisplayHelp( windowManager, HELP_QUICK ) ;
       break;
   case OPT_QUICK_OK: {
       // if phone or network, do the proper thing
       KD_LIST_ITEM quick_entry;
       
       string = (UIW_STRING *) Get( FIELD_HOST_PHONE ) ;
       KD_LIST_ITEM * entry = connector->FindEntry( ((UIW_COMBO_BOX *) Get( COMBO_QUICK ))->DataGet() ) ;

       if ( !entry ) {
           ZAF_MESSAGE_WINDOW * error = 
               new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                       ZIL_MSG_OK,
                                       ZIL_MSG_OK,
                                       "No Connection Entry available to base connection on.");
           if ( !error )
               connector->OutofMemory("Unable to create MESSAGE_WINDOW Disconnect 1");
           error->Control() ;
           break;
       }

       quick_entry = *entry;

       if ( FlagSet( WOS_SELECTED, Get( RADIO_NETWORK )->woStatus ) )
       {
           strncpy( quick_entry._ipaddress, string->DataGet(), 128 ) ;
       }
       else if ( FlagSet( WOS_SELECTED, Get( RADIO_SSH )->woStatus ) )
       {
           strncpy( quick_entry._ipaddress, string->DataGet(), 128 ) ;
       }     
       else if ( FlagSet( WOS_SELECTED, Get( RADIO_FTP )->woStatus ) )
       {
           strncpy( quick_entry._ipaddress, string->DataGet(), 128 ) ;
       }
       else if ( FlagSet( WOS_SELECTED, Get( RADIO_SUPERLAT )->woStatus ) )
       {
           strncpy( quick_entry._lataddress, string->DataGet(), 128 ) ;
       }     
       else if ( FlagSet( WOS_SELECTED, Get( RADIO_DIALUP )->woStatus ) )
       {
           strncpy( quick_entry._phone_number, string->DataGet(), 64 ) ;
       }

       process_id = connector->StartKermit( &quick_entry, _config, _template ) ; 
       if ( !process_id )
       {
           if ( connector->StartKermitErrorCode )
           {
               ZIL_ICHAR * name = quick_entry._name ;
               message =
                  new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                        "Unable to start K95: session: QuickConnect %s errorno %d", 
                        name, connector->StartKermitErrorCode ) ;
               message->Control() ;
               delete message ;
           }
       }
       else 
       {
           /* Entry is added to List in the constructor */
           new K_STATUS( quick_entry._name, process_id, 
                         &K_STATUS::List ) ;
       }
       *windowManager - this ;
       break;
   }
   default:
       ccode = UIW_WINDOW::Event(event);
   }
   return ccode ;
}

K_QUICK::~K_QUICK( void ) 
{
    
}
