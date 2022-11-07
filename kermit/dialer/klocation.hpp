#include "kenum.hpp"

class K_DIAL_OPTIONS;
class ZIL_EXPORT_CLASS K_LOCATION : public UIW_BUTTON
{
   public:
   static ZIL_ICHAR _className[] ;

   ZIL_ICHAR    _name[61];

   ZIL_ICHAR    _country_code[4] ;
   ZIL_ICHAR    _area_code[9] ;
   ZIL_ICHAR    _dial_prefix[33] ;
   ZIL_ICHAR    _dial_suffix[33] ;
   enum DIALMETHOD _dial_method ;
   ZIL_INT32    _dial_timeout ;
   ZIL_UINT8    _redial_count ;
   ZIL_INT32    _redial_interval ;
   ZIL_UINT8    _force_long_dist ;
   ZIL_UINT8    _ignore_dial_tone;

   ZIL_ICHAR    _local_area_prefix[33] ;
   ZIL_ICHAR    _local_area_suffix[33] ;
   ZIL_ICHAR    _local_area_codes_list[129] ;
   
   ZIL_ICHAR    _long_dist_prefix[33] ;
   ZIL_ICHAR    _long_dist_suffix[33] ;
   
   ZIL_ICHAR    _intl_prefix[33] ;
   ZIL_ICHAR    _intl_suffix[33] ;
   
   ZIL_ICHAR    _dial_tf_area[33] ;
   ZIL_ICHAR    _dial_tf_prefix[33] ;

   ZIL_UINT8    _use_pbx ;
   ZIL_ICHAR    _pbx_out[33] ;
   ZIL_ICHAR    _pbx_exch[33] ;
   ZIL_ICHAR    _pbx_in[33] ;

   /* TAPI stuff */
   ZIL_INT8  _is_tapi;
   ZIL_INT32 _tapi_location_id;
   ZIL_ICHAR _tapi_name[256];
   ZIL_INT32 _tapi_country_id;
   ZIL_INT32 _tapi_country_code;
   ZIL_ICHAR _tapi_country_name[32];
   ZIL_ICHAR _tapi_same_area_rule[32];
   ZIL_ICHAR _tapi_long_dist_rule[32];
   ZIL_ICHAR _tapi_intl_rule[32];
   ZIL_ICHAR _tapi_area_code[32];
   ZIL_INT32 _tapi_preferred_card_id;
   ZIL_INT32 _tapi_options;
   ZIL_ICHAR _tapi_local_access_code[32];
   ZIL_ICHAR _tapi_long_dist_code[32];
   ZIL_ICHAR _tapi_toll_prefix_list[4096];
   ZIL_ICHAR _tapi_cancel_call_waiting[32];

   K_LOCATION(void);
   K_LOCATION( K_LOCATION & );
   ~K_LOCATION(void);

   virtual EVENT_TYPE Event( const UI_EVENT & ) ;

   static int Compare(void *, void *);

	// --- Persistent object members ---
#if defined(ZIL_LOAD)
   static K_LOCATION * New(const ZIL_ICHAR *name,
			    ZIL_STORAGE_READ_ONLY *file = ZIL_NULLP(ZIL_STORAGE_READ_ONLY),
			    ZIL_STORAGE_OBJECT_READ_ONLY *object = ZIL_NULLP(ZIL_STORAGE_OBJECT_READ_ONLY),
			    UI_ITEM *_objectTable = ZIL_NULLP(UI_ITEM),
			    UI_ITEM *_userTable = ZIL_NULLP(UI_ITEM))
   { 
       K_LOCATION * config = new K_LOCATION(name, file, object, _objectTable, _userTable);
       return config ;
   }
	
   K_LOCATION(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *file,
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

   K_DIAL_OPTIONS * _dialog;
};
