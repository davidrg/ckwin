#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "usermsg.hpp"
#include "kdconfig.hpp"

extern ZIL_UINT8 kd_major, kd_minor;
ZIL_ICHAR KD_CONFIG::_className[] = "KD_CONFIG" ;

int CompareKDConfigs( void * p1, void * p2 )
{
   return KD_CONFIG::Compare( p1, p2 );
}

KD_CONFIG::KD_CONFIG()
{
    _country_code[0] = '\0' ;      
    _area_code[0] = '\0' ;         
    _local_area_prefix[0] = '\0' ; 
    _local_area_suffix[0] = '\0' ; 
    _long_dist_prefix[0] = '\0' ; 
    _long_dist_suffix[0] = '\0' ; 
    _intl_prefix[0] = '\0' ;      
    _intl_suffix[0] = '\0' ;      
    _dial_prefix[0] = '\0' ;      
    _dial_suffix[0] = '\0' ;      
    _dial_tf_area[0] = '\0' ;    
    _dial_tf_prefix[0] = '\0' ;
                           
   _use_pbx = 0 ;              
   _pbx_out[0] = '\0' ;          
   _pbx_exch[0] = '\0'  ;         
   _pbx_in[0] = '\0'  ;           
                           
   _dial_method = DialMethodDef ;      
   _dial_timeout = 90 ;         
   _redial_count = 0 ;         
   _redial_interval = 0 ;      
                           
   _use_phone_dir = 1 ;        
   _phone_dir[0] = '\0' ;       
   strcpy( _phone_dir, "" ) ;
                           
   _use_network_dir = 1 ;      
   _network_dir[0] = '\0' ;  
   strcpy( _network_dir, "" ) ;
                           
   _confirm_number = 0 ;       
   _convert_dir = 0 ;          
   _display_dial = 0 ;         
   _hangup = 1 ;               
   _sort_dir = 1 ;             

   _load_templates = 1;  
   _load_network = 1;    
   _load_dialup = 1;     
   _load_direct = 1;
   _load_predefined = 1; 
   _load_userdefined = 1;

    _top = -1 ;
    _left = -1; 
    _right = -1;
    _bottom = -1;

    _min_on_use = 0 ;

    _tapi_conversions = ConvKermit;
    _tapi_dialing = DialKermit;

    _current_location[0] = '\0';
    _convert_location = 0;

    _shortcut_desktop   = 1;
    _shortcut_startmenu = 1;

    _app_ftp[0]='\0';
    _app_www[0]='\0';
    _app_edit[0]='\0';

    _load_ftp = 1;
}


KD_CONFIG::~KD_CONFIG()
{
}

int KD_CONFIG::Compare(void *p1, void *p2)
{
	KD_CONFIG *object1 = (KD_CONFIG *)p1;
	KD_CONFIG *object2 = (KD_CONFIG *)p2;

    if ( !object1 || !object2 )
        return FALSE;

   return 
      !strcmp(object1->_country_code, object2->_country_code ) &&
      !strcmp(object1->_area_code, object2->_area_code ) &&
      !strcmp(object1->_local_area_prefix, object2->_local_area_prefix) &&
      !strcmp(object1->_local_area_suffix, object2->_local_area_suffix) &&
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
      (object1->_use_phone_dir == object2->_use_phone_dir     ) &&
      !strcmp(object1->_phone_dir, object2->_phone_dir       ) &&
      ( object1->_use_network_dir == object2->_use_network_dir ) &&
      !strcmp(object1->_network_dir, object2->_network_dir     ) &&
      ( object1->_confirm_number == object2->_confirm_number    ) &&
      ( object1->_convert_dir    == object2->_convert_dir       ) &&
      ( object1->_display_dial   == object2->_display_dial      ) &&
      ( object1->_hangup         == object2->_hangup            ) &&
      ( object1->_sort_dir       == object2->_sort_dir          ) &&
      ( object1->_load_templates   == object2->_load_templates ) &&
      ( object1->_load_network     ==   object2->_load_network ) &&
      ( object1->_load_ftp         ==   object2->_load_ftp ) &&
      ( object1->_load_dialup      ==   object2->_load_dialup ) &&     
      ( object1->_load_direct      ==   object2->_load_direct ) &&     
      ( object1->_load_predefined  ==   object2->_load_predefined ) && 
      ( object1->_load_userdefined == object2->_load_userdefined ) &&
      ( object1->_top    ==   object2->_top ) &&
      ( object1->_left   ==   object2->_left ) &&     
      ( object1->_right ==   object2->_right ) &&     
      ( object1->_bottom  ==   object2->_bottom ) &&
      ( object1->_tapi_conversions == object2->_tapi_conversions ) &&
      ( object1->_tapi_dialing   ==   object2->_tapi_dialing ) &&     
      ( object1->_min_on_use == object2->_min_on_use ) &&
      !strcmp(object1->_current_location, object2->_current_location) &&
      ( object1->_shortcut_desktop == object2->_shortcut_desktop ) &&
      ( object1->_shortcut_startmenu == object2->_shortcut_startmenu ) &&
      !strcmp(object1->_app_ftp, object2->_app_ftp) &&
      !strcmp(object1->_app_www, object2->_app_www) &&
      !strcmp(object1->_app_edit, object2->_app_edit);
}

// ----- ZIL_PERSISTENCE ----------------------------------------------------

#if defined(ZIL_LOAD)
KD_CONFIG::KD_CONFIG(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
	ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable) 
{
    // Initialize the information.
    _country_code[0] = '\0' ;      
    _area_code[0] = '\0' ;         
    _local_area_prefix[0] = '\0' ; 
    _local_area_suffix[0] = '\0' ; 
    _long_dist_prefix[0] = '\0' ; 
    _long_dist_suffix[0] = '\0' ; 
    _intl_prefix[0] = '\0' ;      
    _intl_suffix[0] = '\0' ;      
    _dial_prefix[0] = '\0' ;      
    _dial_suffix[0] = '\0' ;      
    _dial_tf_area[0] = '\0' ;    
    _dial_tf_prefix[0] = '\0' ;
                           
   _use_pbx = 0 ;              
   _pbx_out[0] = '\0' ;          
   _pbx_exch[0] = '\0'  ;         
   _pbx_in[0] = '\0'  ;           
                           
   _dial_method = DialMethodDef ;      
   _dial_timeout = 90 ;         
   _redial_count = 0 ;         
   _redial_interval = 0 ;      
                           
   _use_phone_dir = 1 ;        
   _phone_dir[0] = '\0' ;       
   strcpy( _phone_dir, "" ) ;
                           
   _use_network_dir = 1 ;      
   _network_dir[0] = '\0' ;  
   strcpy( _network_dir, "" ) ;
                           
   _confirm_number = 0 ;       
   _convert_dir = 0 ;          
   _display_dial = 0 ;         
   _hangup = 1 ;               
   _sort_dir = 1 ;             

   _load_templates = 1;  
   _load_network = 1;    
   _load_dialup = 1;     
   _load_direct = 1;
   _load_predefined = 1; 
   _load_userdefined = 1;

    _top = -1 ;
    _left = -1; 
    _right = -1;
    _bottom = -1;

    _min_on_use = 0 ;

    _tapi_conversions = ConvKermit;
    _tapi_dialing = DialKermit;

    _current_location[0] = '\0';
    _convert_location = 0;

    _shortcut_desktop   = 1;
    _shortcut_startmenu = 1;

    _app_ftp[0]='\0';
    _app_www[0]='\0';
    _app_edit[0]='\0';

    _load_ftp = 1;

    KD_CONFIG::Load(name, directory, file, objectTable, userTable);
}

void KD_CONFIG::Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
	ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable)
{
    ZIL_UINT16 length ;
    ZIL_UINT16 Enum ;
    ZIL_UINT8 itemMajor, itemMinor ;

    file->Load(&itemMajor);
    file->Load(&itemMinor);

    if ( itemMajor < 1 )
        return ;

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
    file->Load(_dial_prefix, length) ;
    _dial_prefix[32] = '\0' ;      

    file->Load(&length) ;
    if ( length > 32 )
        return;
    file->Load(_dial_suffix, length) ;
    _dial_suffix[32] = '\0' ;      

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

    file->Load(&Enum) ;
    _dial_method = (enum DIALMETHOD) Enum ;

    file->Load(&_dial_timeout) ;
    file->Load(&_redial_count ) ;
    file->Load(&_redial_interval) ;
    file->Load(&_use_phone_dir) ;        

    file->Load(&length) ;
    if ( length > 1023 )
        return;
    file->Load(_phone_dir, length) ;
    _phone_dir[1023] = '\0' ;       
                           
    file->Load(&_use_network_dir) ;      
    file->Load(&length) ;
    if ( length > 1023 )
        return;
    file->Load(_network_dir, length) ;
    _network_dir[1023] = '\0' ;     
                           
    file->Load(&_confirm_number) ;       
    file->Load(&_convert_dir) ;          
    file->Load(&_display_dial) ;         
    file->Load(&_hangup) ;               
    file->Load(&_sort_dir) ;             

    file->Load(&_load_templates  );
    file->Load(&_load_network    );
    file->Load(&_load_dialup     );
    file->Load(&_load_predefined );
    file->Load(&_load_userdefined);

    if ( itemMinor < 4 ) {
        _load_direct = 1 ;
    }
    else { 
        file->Load(&_load_direct);
    }

    if ( itemMinor < 5 ) {
	_top = _bottom = _left = _right = -1;
    }
    else { 
	file->Load(&_top);
	file->Load(&_left);
	file->Load(&_right);
	file->Load(&_bottom);
	file->Load(&length);
	file->Load(_current_entry, length);
    }

    if ( itemMinor < 6 ) {
	_min_on_use = 0 ;
    }
    else {
	file->Load(&_min_on_use);
    }

  ver_1_8:
    if ( itemMinor < 8 ) {
	_local_area_prefix[0] = '\0' ; 
	_local_area_suffix[0] = '\0' ;
	_tapi_conversions = ConvKermit;
	_tapi_dialing = DialKermit;
    }
    else {
	file->Load(&length) ;
        if ( length > 32 ) {
            itemMinor = 7;
            goto ver_1_8;
        }
	file->Load(_local_area_prefix, length) ;
        _local_area_prefix[32] = '\0';

	file->Load(&length) ;
        if ( length > 32 ) {
            itemMinor = 7;
            goto ver_1_8;
        }
	file->Load(_local_area_suffix, length) ;
        _local_area_suffix[32] = '\0';
	file->Load(&Enum) ;
	_tapi_conversions = (enum TAPICONV) Enum ;
	file->Load(&Enum) ;
	_tapi_dialing = (enum TAPIDIAL) Enum ;
    }

  ver_1_11:
    if ( itemMinor < 11 ) {
	strcpy(_current_location,"K95 Default");
	/* convert location information */
	_convert_location = 1;
    }	
    else {
	file->Load(&length);
        if ( length > 60 ) {
            itemMinor = 10;
            goto ver_1_11;
        }
	file->Load(_current_location, length);
        _current_location[60] = '\0';
	_convert_location = 0;
    }

    if ( itemMinor < 14 ) {
	_shortcut_desktop = 1;
	_shortcut_startmenu = 1;
    }	
    else {
        file->Load(&_shortcut_desktop);
        file->Load(&_shortcut_startmenu);
    }

  ver_1_17:
    if ( itemMinor < 17 ) {
        _app_ftp[0]='\0';
        _app_www[0]='\0';
        _app_edit[0]='\0';
    } else {
	file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 16;
            goto ver_1_17;
        }
	file->Load(_app_ftp, length);
	_app_ftp[256] = '\0';
	file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 16;
            goto ver_1_17;
        }
	file->Load(_app_www, length);
	_app_www[256] = '\0';;
	file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 16;
            goto ver_1_17;
        }
	file->Load(_app_edit, length);
        _app_edit[256] = '\0';;
    }

    if ( itemMinor < 27 ) {
	_load_ftp = 1;
    }	
    else {
        file->Load(&_load_ftp);
    }
}
#endif

#if defined(ZIL_STORE)
void KD_CONFIG::Store(const ZIL_ICHAR *name, ZIL_STORAGE *directory,
	ZIL_STORAGE_OBJECT *file, UI_ITEM *objectTable, UI_ITEM *userTable)
{
   file->Store(kd_major) ;
   file->Store(kd_minor) ;

   ZIL_UINT16 length ;
   ZIL_UINT16 Enum ;

   length = 1+strlen(_country_code);
   file->Store(length);
   file->Store(_country_code) ;

   length = 1+strlen(_area_code);
   file->Store(length);
   file->Store(_area_code) ;

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

   length = 1+strlen(_dial_prefix);
   file->Store(length);
   file->Store(_dial_prefix) ;

   length = 1+strlen(_dial_suffix);
   file->Store(length);
   file->Store(_dial_suffix) ;

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

   file->Store( Enum = _dial_method ) ;

   file->Store(_dial_timeout) ;
   file->Store(_redial_count) ;
   file->Store(_redial_interval) ;
   file->Store(_use_phone_dir) ;

   length = 1+strlen(_phone_dir);
   file->Store(length);
   file->Store(_phone_dir) ;

   file->Store(_use_network_dir) ;

   length = 1+strlen(_network_dir);
   file->Store(length);
   file->Store(_network_dir) ;

   file->Store(_confirm_number) ;
   file->Store(_convert_dir) ;
   file->Store(_display_dial) ;
   file->Store(_hangup) ;
   file->Store(_sort_dir) ;

   file->Store(_load_templates  );
   file->Store(_load_network    );
   file->Store(_load_dialup     );
   file->Store(_load_predefined );
   file->Store(_load_userdefined);

   // items new to 1.1.7 (itemMinor = 4)
    file->Store(_load_direct);

    // items new to 1.1.7 (itemMinor = 5)
    file->Store(_top);
    file->Store(_left);
    file->Store(_right);
    file->Store(_bottom);

    length = 1+strlen(_current_entry);
    file->Store(length);
    file->Store(_current_entry) ;

    // items new to 1.1.8 (itemMinor = 6)
    file->Store(_min_on_use);

    // items new to 1.1.12 (itemMinor = 8)
    length = 1+strlen(_local_area_prefix);
    file->Store(length);
    file->Store(_local_area_prefix) ;

    length = 1+strlen(_local_area_suffix);
    file->Store(length);
    file->Store(_local_area_suffix) ;
    file->Store(Enum = _tapi_conversions);
    file->Store(Enum = _tapi_dialing);

    // items new to 1.1.12 (itemMinor = 9)
    length = 1+strlen(_current_location);
    file->Store(length);
    file->Store(_current_location) ;

    // items new to 1.1.14 (itemMinor = 14)
    file->Store(_shortcut_desktop);
    file->Store(_shortcut_startmenu);

    // items new to 1.1.16 (itemMinor = 17)
    length = 1+strlen(_app_ftp);
    file->Store(length);
    file->Store(_app_ftp) ;
    length = 1+strlen(_app_www);
    file->Store(length);
    file->Store(_app_www) ;
    length = 1+strlen(_app_edit);
    file->Store(length);
    file->Store(_app_edit) ;

    // items new to 1.1.21 (itemMinor = 27)
    file->Store(_load_ftp);
}
#endif

