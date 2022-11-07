#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "usermsg.hpp"
#include "klocation.hpp"
#include "kdialopt.hpp"

extern ZIL_UINT8 kd_major, kd_minor;
ZIL_ICHAR K_LOCATION::_className[] = "K_LOCATION" ;

int CompareKLocations( void * p1, void * p2 )
{
   return K_LOCATION::Compare( p1, p2 );
}

EVENT_TYPE 
KLocationUser( UI_WINDOW_OBJECT * obj, UI_EVENT & event,
	       EVENT_TYPE ccode )
{
    K_LOCATION * entry = (K_LOCATION *) obj ;

    switch ( ccode )
    {
    case L_SELECT: 
#ifdef __OS2__
	if ( entry->_dialog && entry != entry->_dialog->_current ) {
	    entry->_dialog->CopyValuesToCurrent();
	    entry->_dialog->_current = entry;
	    entry->_dialog->CopyValuesFromCurrent();
	}
#endif
	break;

    case S_CURRENT:
	if (entry->_dialog ) {
	    entry->_dialog->_current = entry;
	    entry->_dialog->CopyValuesFromCurrent();
	}
	break;

    case S_NON_CURRENT:
	if ( entry->_dialog ) {
	    entry->_dialog->_current = entry;
	    entry->_dialog->CopyValuesToCurrent();
	}
	break;
    }
    return 0;
}

K_LOCATION::K_LOCATION():
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
               BTF_NO_FLAGS,  // Button flags
	       WOF_NO_FLAGS,                     // Window flags
               KLocationUser,                    // User function
               0)         // Send Message Value  
{
    _name[0] = '\0';
    _country_code[0] = '\0' ;      
    _area_code[0] = '\0' ;         
    _dial_prefix[0] = '\0' ;
    _dial_suffix[0] = '\0' ;      
    _dial_method = DialMethodDef ;      
    _dial_timeout = 90 ;         
    _redial_count = 0 ;         
    _redial_interval = 0 ;      
    _force_long_dist = 0;
    _ignore_dial_tone = 0;

    _local_area_prefix[0] = '\0' ; 
    _local_area_suffix[0] = '\0' ; 
    _local_area_codes_list[0] = '\0';
    
    _long_dist_prefix[0] = '\0' ;
    _long_dist_suffix[0] = '\0' ; 
    
    _intl_prefix[0] = '\0' ;
    _intl_suffix[0] = '\0' ;      
    
    _dial_tf_area[0] = '\0' ;    
    _dial_tf_prefix[0] = '\0' ;
                           
    _use_pbx = 0 ;              
    _pbx_out[0] = '\0' ;          
    _pbx_exch[0] = '\0'  ;         
    _pbx_in[0] = '\0'  ;           

    _is_tapi=0;
    _tapi_location_id=0;
    _tapi_name[0]='\0';
    _tapi_country_id=0;
    _tapi_country_code=0;
    _tapi_country_name[0]='\0';
    _tapi_same_area_rule[0]='\0';
    _tapi_long_dist_rule[0]='\0';
    _tapi_intl_rule[0]='\0';
    _tapi_area_code[0]='\0';
    _tapi_preferred_card_id=0;
    _tapi_options=0;
    _tapi_local_access_code[0]='\0';
    _tapi_long_dist_code[0]='\0';
    _tapi_toll_prefix_list[0]='\0';
    _tapi_cancel_call_waiting[0]='\0';

    _dialog = NULL;
}


K_LOCATION::K_LOCATION( K_LOCATION & source ):
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               source._name, // no text, ownerdraw
               BTF_NO_FLAGS,  // Button flags
	       WOF_NO_FLAGS,                     // Window flags
               KLocationUser,                    // User function
               0)         // Send Message Value  
{
    strcpy(_name, source._name);
    strcpy(_country_code,source._country_code) ;      
    strcpy(_area_code,source._area_code) ;         
    strcpy(_dial_prefix,source._dial_prefix) ;
    strcpy(_dial_suffix,source._dial_suffix) ;      
    _dial_method = source._dial_method ;      
    _dial_timeout = source._dial_timeout ;         
    _redial_count = source._redial_count ;         
    _redial_interval = source._redial_interval ;      
    _force_long_dist = source._force_long_dist;
    _ignore_dial_tone = source._ignore_dial_tone;

    strcpy(_local_area_prefix, source._local_area_prefix) ; 
    strcpy(_local_area_suffix, source._local_area_suffix) ; 
    strcpy(_local_area_codes_list, source._local_area_codes_list) ;
    
    strcpy(_long_dist_prefix, source._long_dist_prefix) ;
    strcpy(_long_dist_suffix, source._long_dist_suffix) ; 
    
    strcpy(_intl_prefix, source._intl_prefix) ;
    strcpy(_intl_suffix, source._intl_suffix) ;      
    
    strcpy(_dial_tf_area, source._dial_tf_area) ;    
    strcpy(_dial_tf_prefix, source._dial_tf_prefix) ;
                           
    _use_pbx = source._use_pbx ;              
    strcpy(_pbx_out, source._pbx_out) ;          
    strcpy(_pbx_exch, source._pbx_exch)  ;         
    strcpy(_pbx_in, source._pbx_in) ;           

    _is_tapi=source._is_tapi;
    _tapi_location_id=source._tapi_location_id;
    strcpy(_tapi_name,source._tapi_name);
    _tapi_country_id=source._tapi_country_id;
    _tapi_country_code=source._tapi_country_code;
    strcpy(_tapi_country_name,source._tapi_country_name);
    strcpy(_tapi_same_area_rule,source._tapi_same_area_rule);
    strcpy(_tapi_long_dist_rule,source._tapi_long_dist_rule);
    strcpy(_tapi_intl_rule,source._tapi_intl_rule);
    strcpy(_tapi_area_code,source._tapi_area_code);
    _tapi_preferred_card_id=source._tapi_preferred_card_id;
    _tapi_options=source._tapi_options;
    strcpy(_tapi_local_access_code,source._tapi_local_access_code);
    strcpy(_tapi_long_dist_code,source._tapi_long_dist_code);
    strcpy(_tapi_toll_prefix_list,source._tapi_toll_prefix_list);
    strcpy(_tapi_cancel_call_waiting,source._tapi_cancel_call_waiting);

    _dialog = NULL;
}


K_LOCATION::~K_LOCATION()
{
}

int K_LOCATION::Compare(void *p1, void *p2)
{
    K_LOCATION *object1 = (K_LOCATION *)p1;
    K_LOCATION *object2 = (K_LOCATION *)p2;

    return 
	!strcmp(object1->_name, object2->_name ) &&
        !strcmp(object1->_country_code, object2->_country_code ) &&
    	!strcmp(object1->_area_code, object2->_area_code ) &&
        !strcmp(object1->_local_area_prefix, object2->_local_area_prefix) &&
        !strcmp(object1->_local_area_suffix, object2->_local_area_suffix) &&
        !strcmp(object1->_local_area_codes_list, object2->_local_area_codes_list) &&
        !strcmp(object1->_long_dist_prefix, object2->_long_dist_prefix) &&
        !strcmp(object1->_long_dist_suffix, object2->_long_dist_suffix) &&
        !strcmp(object1->_intl_prefix, object2->_intl_prefix     ) &&
        !strcmp(object1->_intl_suffix, object2->_intl_suffix     ) &&
        !strcmp(object1->_dial_prefix, object2->_dial_prefix     ) &&
        !strcmp(object1->_dial_suffix, object2->_dial_suffix     ) &&
        !strcmp(object1->_dial_tf_area, object2->_dial_tf_area   ) &&
        !strcmp(object1->_dial_tf_prefix, object2->_dial_tf_prefix   ) &&
        (object1->_use_pbx == object2->_use_pbx           ) &&
        !strcmp(object1->_pbx_out, object2->_pbx_out         ) &&
        !strcmp(object1->_pbx_exch, object2->_pbx_exch        ) &&
        !strcmp(object1->_pbx_in, object2->_pbx_in          ) &&
        (object1->_dial_method == object2->_dial_method       ) &&
        (object1->_dial_timeout == object2->_dial_timeout      ) &&
        (object1->_redial_count == object2->_redial_count      ) &&
        (object1->_redial_interval == object2->_redial_interval   ) &&
	(object1->_force_long_dist == object2->_force_long_dist) &&
        (object1->_ignore_dial_tone == object2->_ignore_dial_tone);
}

// ----- ZIL_PERSISTENCE ----------------------------------------------------

#if defined(ZIL_LOAD)
K_LOCATION::K_LOCATION(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
			ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable) :
    UIW_BUTTON( 0,  // Left
		0,  // Top
		0,  // Width
		(ZIL_ICHAR *)name, // no text, ownerdraw
		BTF_NO_FLAGS,  // Button flags
		WOF_NO_FLAGS,                                 // Window flags
		KLocationUser,                     // User function
		0)              // Send Message Value   
{
    _name[0] = '\0';
    _country_code[0] = '\0' ;      
    _area_code[0] = '\0' ;         
    _dial_prefix[0] = '\0' ;
    _dial_suffix[0] = '\0' ;      
    _dial_method = DialMethodDef ;      
    _dial_timeout = 90 ;         
    _redial_count = 0 ;         
    _redial_interval = 0 ;      
    _force_long_dist = 0;
    _ignore_dial_tone = 0;

    _local_area_prefix[0] = '\0' ; 
    _local_area_suffix[0] = '\0' ; 
    _local_area_codes_list[0] = '\0';
    
    _long_dist_prefix[0] = '\0' ;
    _long_dist_suffix[0] = '\0' ; 
    
    _intl_prefix[0] = '\0' ;
    _intl_suffix[0] = '\0' ;      
    
    _dial_tf_area[0] = '\0' ;    
    _dial_tf_prefix[0] = '\0' ;
                           
    _use_pbx = 0 ;              
    _pbx_out[0] = '\0' ;          
    _pbx_exch[0] = '\0'  ;         
    _pbx_in[0] = '\0'  ;           

    _is_tapi=0;
    _tapi_location_id=0;
    _tapi_name[0]='\0';
    _tapi_country_id=0;
    _tapi_country_code=0;
    _tapi_country_name[0]='\0';
    _tapi_same_area_rule[0]='\0';
    _tapi_long_dist_rule[0]='\0';
    _tapi_intl_rule[0]='\0';
    _tapi_area_code[0]='\0';
    _tapi_preferred_card_id=0;
    _tapi_options=0;
    _tapi_local_access_code[0]='\0';
    _tapi_long_dist_code[0]='\0';
    _tapi_toll_prefix_list[0]='\0';
    _tapi_cancel_call_waiting[0]='\0';

    _dialog = NULL;


    // Initialize the information.
    K_LOCATION::Load(name, directory, file, objectTable, userTable);
    UI_WINDOW_OBJECT::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::DataSet(text);  // This sets the button string which should be empty
}

void K_LOCATION::Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
		       ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable){
    // Load the button information.
    UIW_BUTTON::Load(name, directory, file, objectTable, userTable);
    ZIL_UINT16 length ;
    ZIL_UINT16 Enum ;
    ZIL_UINT8 itemMajor, itemMinor ;

    file->Load(&itemMajor);
    file->Load(&itemMinor);

    if ( itemMajor < 1 )
	return ;

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_name, length) ;
    _name[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 3 )
        return;
    file->Load(_country_code, length) ;
    _country_code[3] = '\0' ;      

   file->Load(&length) ;
    if ( length > 8 )
        return;
   file->Load(_area_code, length) ;
   _area_code[8] = '\0' ;

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_dial_prefix, length) ;
    _dial_prefix[32] = '\0' ;      

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_dial_suffix, length) ;
    _dial_suffix[32] = '\0' ;      

    file->Load(&Enum) ;
    _dial_method = (enum DIALMETHOD) Enum ;

    file->Load(&_dial_timeout) ;
    file->Load(&_redial_count ) ;
    file->Load(&_redial_interval) ;
    file->Load(&_force_long_dist) ;

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_local_area_prefix, length) ;
    _local_area_prefix[32] = '\0' ; 

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_local_area_suffix, length) ;
    _local_area_suffix[32] = '\0' ; 

    file->Load(&length) ;
    if ( length > 128 )
        return;
    file->Load(_local_area_codes_list, length) ;
    _local_area_codes_list[128] = '\0' ; 

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_long_dist_prefix, length) ;
    _long_dist_prefix[32] = '\0' ; 

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_long_dist_suffix, length) ;
   _long_dist_suffix[32] = '\0' ;

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_intl_prefix, length) ;
   _intl_prefix[32] = '\0' ;      

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_intl_suffix, length) ;
   _intl_suffix[32] = '\0' ;      

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_dial_tf_area, length) ;
   _dial_tf_area[32] = '\0' ;    
                           
   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_dial_tf_prefix, length) ;
   _dial_tf_prefix[32] = '\0' ;    
                           
   file->Load(&_use_pbx);

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_pbx_out, length) ;
   _pbx_out[32] = '\0' ;          

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_pbx_exch, length) ;
   _pbx_exch[32] = '\0'  ;         

   file->Load(&length) ;
    if ( length > 32 )
        return;
   file->Load(_pbx_in, length) ;
   _pbx_in[32] = '\0'  ;           

    file->Load(&_is_tapi);
    file->Load(&_tapi_location_id);
    file->Load(&length) ;
    if ( length > 256 )
        return;
    file->Load(_tapi_name, length) ;
    _tapi_name[256] = '\0'  ;           
    file->Load(&_tapi_country_id);
    file->Load(&_tapi_country_code);
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_country_name, length) ;
    _tapi_country_name[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_same_area_rule, length) ;
    _tapi_same_area_rule[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_long_dist_rule, length) ;
    _tapi_long_dist_rule[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_intl_rule, length) ;
    _tapi_intl_rule[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_area_code, length) ;
    _tapi_area_code[32] = '\0'  ;           
    file->Load(&_tapi_preferred_card_id);
    file->Load(&_tapi_options);
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_local_access_code, length) ;
    _tapi_local_access_code[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_long_dist_code, length) ;
    _tapi_long_dist_code[32] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 4095 )
        return;
    file->Load(_tapi_toll_prefix_list, length) ;
    _tapi_toll_prefix_list[4095] = '\0'  ;           
    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_tapi_cancel_call_waiting, length) ;
    _tapi_cancel_call_waiting[32] = '\0'  ;           

    /* New to 1.1.14 */
    if ( itemMinor < 13 ) {
        _ignore_dial_tone = 0;
    }
    else {
        file->Load(&_ignore_dial_tone) ;
    }
}
#endif

#if defined(ZIL_STORE)
void K_LOCATION::Store(const ZIL_ICHAR *name, ZIL_STORAGE *directory,
	ZIL_STORAGE_OBJECT *file, UI_ITEM *objectTable, UI_ITEM *userTable)
{
    UIW_BUTTON::Store(name, directory, file, objectTable, userTable);
   file->Store(kd_major) ;
   file->Store(kd_minor) ;

   ZIL_UINT16 length ;
   ZIL_UINT16 Enum ;

    length = 1+strlen(_name);
    file->Store(length);
    file->Store(_name) ;

    length = 1+strlen(_country_code);
   file->Store(length);
   file->Store(_country_code) ;

   length = 1+strlen(_area_code);
   file->Store(length);
   file->Store(_area_code) ;

    length = 1+strlen(_dial_prefix);
    file->Store(length);
    file->Store(_dial_prefix) ;

    length = 1+strlen(_dial_suffix);
    file->Store(length);
    file->Store(_dial_suffix) ;

    file->Store( Enum = _dial_method ) ;

    file->Store(_dial_timeout) ;
    file->Store(_redial_count) ;
    file->Store(_redial_interval) ;
    file->Store(_force_long_dist) ;

    length = 1+strlen(_local_area_prefix);
    file->Store(length);
    file->Store(_local_area_prefix) ;

    length = 1+strlen(_local_area_suffix);
    file->Store(length);
    file->Store(_local_area_suffix) ;

    length = 1+strlen(_local_area_codes_list);
    file->Store(length);
    file->Store(_local_area_codes_list) ;

    length = 1+strlen(_long_dist_prefix);
   file->Store(length);
   file->Store(_long_dist_prefix) ;

   length = 1+strlen(_long_dist_suffix);
   file->Store(length);
   file->Store(_long_dist_suffix) ;

   length = 1+strlen(_intl_prefix);
   file->Store(length);
   file->Store(_intl_prefix) ;

   length = 1+strlen(_intl_suffix);
   file->Store(length);
   file->Store(_intl_suffix) ;

   length = 1+strlen(_dial_tf_area);
   file->Store(length);
   file->Store(_dial_tf_area) ;

   length = 1+strlen(_dial_tf_prefix);
   file->Store(length);
   file->Store(_dial_tf_prefix) ;

   file->Store(_use_pbx) ;
   
   length = 1+strlen(_pbx_out);
   file->Store(length);
   file->Store(_pbx_out) ;

   length = 1+strlen(_pbx_exch);
   file->Store(length);
   file->Store(_pbx_exch) ;

   length = 1+strlen(_pbx_in);
   file->Store(length);
   file->Store(_pbx_in) ;

    file->Store(_is_tapi);
    file->Store(_tapi_location_id);
    length = 1+strlen(_tapi_name);
    file->Store(length) ;
    file->Store(_tapi_name) ;
    file->Store(_tapi_country_id);
    file->Store(_tapi_country_code);
    length = 1+strlen(_tapi_country_name);
    file->Store(length) ;
    file->Store(_tapi_country_name) ;
    length = 1+strlen(_tapi_same_area_rule);
    file->Store(length) ;
    file->Store(_tapi_same_area_rule) ;
    length = 1+strlen(_tapi_long_dist_rule);
    file->Store(length) ;
    file->Store(_tapi_long_dist_rule) ;
    length = 1+strlen(_tapi_intl_rule);
    file->Store(length) ;
    file->Store(_tapi_intl_rule) ;
    length = 1+strlen(_tapi_area_code);
    file->Store(length) ;
    file->Store(_tapi_area_code) ;
    file->Store(_tapi_preferred_card_id);
    file->Store(_tapi_options);
    length = 1+strlen(_tapi_local_access_code);
    file->Store(length) ;
    file->Store(_tapi_local_access_code) ;
    length = 1+strlen(_tapi_long_dist_code);
    file->Store(length) ;
    file->Store(_tapi_long_dist_code) ;
    length = 1+strlen(_tapi_toll_prefix_list);
    file->Store(length) ;
    file->Store(_tapi_toll_prefix_list) ;
    length = 1+strlen(_tapi_cancel_call_waiting);
    file->Store(length) ;
    file->Store(_tapi_cancel_call_waiting) ;

    /* New to 1.1.14 */
    file->Store(_ignore_dial_tone) ;
}

EVENT_TYPE 
K_LOCATION :: Event( const UI_EVENT & event ) 
{
    EVENT_TYPE retval = event.type;

    switch ( event.type ) {
#ifdef WIN32
    case E_MSWINDOWS:
	if ( event.message.message == WM_RBUTTONUP ) {
	// Array of UI_ITEMS used to initialize the pop-up menu
	UI_ITEM menuItems[]=
	{
	    {S_CLOSE_TEMPORARY, ZIL_NULLP(void), "Cancel",MNIF_SEND_MESSAGE},
	    {0, ZIL_NULLP(void), "", MNIF_SEPARATOR},
	    {L_EXIT_FUNCTION, ZIL_NULLP(void), "Exit",MNIF_SEND_MESSAGE},
	    {0, CloseWindow, "Close", MNIF_SEND_MESSAGE},
	    {0, ZIL_NULLP(void), 0, 0}
	};
	UIW_POP_UP_MENU *menu = new UIW_POP_UP_MENU(trueRegion.right, trueRegion.bottom, 
						     WNF_NO_FLAGS, menuItems);
	menu->woStatus |= WOS_GRAPHICS;
	menu->woAdvancedFlags |= WOAF_TEMPORARY;
	*windowManager + menu;
	}
	retval = UIW_BUTTON::Event(event) ;
	break;
#endif /* WIN32 */

    case L_END_SELECT:
    case S_CURRENT:

    default:
	retval = UIW_BUTTON::Event(event) ;
    }
    
    return retval ; 
}
#endif

