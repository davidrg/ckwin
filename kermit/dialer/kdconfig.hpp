#include "kenum.hpp"
class ZIL_EXPORT_CLASS KD_CONFIG
{
   public:
   static ZIL_ICHAR _className[] ;

   ZIL_ICHAR   _country_code[4] ;
   ZIL_ICHAR   _area_code[9] ;
   ZIL_ICHAR   _local_area_prefix[33] ;
   ZIL_ICHAR   _local_area_suffix[33] ;
   ZIL_ICHAR   _long_dist_prefix[33] ;
   ZIL_ICHAR   _long_dist_suffix[33] ;
   ZIL_ICHAR   _intl_prefix[33] ;
   ZIL_ICHAR   _intl_suffix[33] ;
   ZIL_ICHAR   _dial_prefix[33] ;
   ZIL_ICHAR   _dial_suffix[33] ;
   ZIL_ICHAR   _dial_tf_area[33] ;
   ZIL_ICHAR   _dial_tf_prefix[33] ;

   ZIL_UINT8   _use_pbx ;
   ZIL_ICHAR   _pbx_out[33] ;
   ZIL_ICHAR   _pbx_exch[33] ;
   ZIL_ICHAR   _pbx_in[33] ;

   enum DIALMETHOD _dial_method ;
   ZIL_INT32   _dial_timeout ;
   ZIL_UINT8    _redial_count ;
   ZIL_INT32   _redial_interval ;

   ZIL_UINT8   _use_phone_dir ;
   ZIL_ICHAR   _phone_dir[2001] ;

   ZIL_UINT8   _use_network_dir ;
   ZIL_ICHAR   _network_dir[2001] ;

   ZIL_UINT8   _confirm_number ;
   ZIL_UINT8   _convert_dir ;
   ZIL_UINT8   _display_dial ;
   ZIL_UINT8   _hangup ;
   ZIL_UINT8   _sort_dir ;

   ZIL_UINT8   _load_templates ;
   ZIL_UINT8   _load_network ;
   ZIL_UINT8   _load_ftp;
   ZIL_UINT8   _load_dialup ;
   ZIL_UINT8   _load_direct ;
   ZIL_UINT8   _load_predefined ;
   ZIL_UINT8   _load_userdefined ;

   ZIL_INT32   _top;
   ZIL_INT32   _left;
   ZIL_INT32   _right;
   ZIL_INT32   _bottom;

   ZIL_ICHAR   _current_entry[65];
   ZIL_UINT8   _min_on_use;

   enum TAPICONV _tapi_conversions;
   enum TAPIDIAL _tapi_dialing;

   ZIL_ICHAR   _current_location[61];
   ZIL_UINT8   _convert_location;

   ZIL_UINT8   _shortcut_desktop;
   ZIL_UINT8   _shortcut_startmenu;

   ZIL_ICHAR   _app_ftp[257];
   ZIL_ICHAR   _app_www[257];
   ZIL_ICHAR   _app_edit[257];

   KD_CONFIG(void);
   ~KD_CONFIG(void);

	static int Compare(void *, void *);

	// --- Persistent object members ---
#if defined(ZIL_LOAD)
   static KD_CONFIG * New(const ZIL_ICHAR *name,
		ZIL_STORAGE_READ_ONLY *file = ZIL_NULLP(ZIL_STORAGE_READ_ONLY),
		ZIL_STORAGE_OBJECT_READ_ONLY *object = ZIL_NULLP(ZIL_STORAGE_OBJECT_READ_ONLY),
		UI_ITEM *_objectTable = ZIL_NULLP(UI_ITEM),
		UI_ITEM *_userTable = ZIL_NULLP(UI_ITEM))
		{ 
           KD_CONFIG * config = new KD_CONFIG(name, file, object, _objectTable, _userTable);
           return config ;
        }
	
   KD_CONFIG(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *file,
		ZIL_STORAGE_OBJECT_READ_ONLY *object,
		UI_ITEM *objectTable = ZIL_NULLP(UI_ITEM),
		UI_ITEM *userTable = ZIL_NULLP(UI_ITEM));

     virtual void Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *file,
		ZIL_STORAGE_OBJECT_READ_ONLY *object, UI_ITEM *objectTable, UI_ITEM *userTable);
#endif
#if defined(ZIL_STORE)
     virtual void Store(const ZIL_ICHAR *name, ZIL_STORAGE *file,
		ZIL_STORAGE_OBJECT *object, UI_ITEM *objectTable, UI_ITEM *userTable);
#endif

};
