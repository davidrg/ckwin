#include "kenum.hpp"

class K_MODEM_DIALOG;
class ZIL_EXPORT_CLASS K_MODEM : public UIW_BUTTON
{
   public:
   static ZIL_ICHAR _className[] ;

   ZIL_ICHAR   _name[61];

   ZIL_ICHAR    _type[61] ;
   ZIL_ICHAR    _port[61] ;
   ZIL_INT32  	_speed ;
   enum FLOW    _flow ;
   enum PARITY  _parity ;
   enum STOPBITS _stopbits;
   ZIL_UINT8    _correction ;
   ZIL_UINT8    _compression ;
   ZIL_UINT8    _carrier ;
   ZIL_UINT8    _speaker ;
   ZIL_UINT8    _volume ; 
   ZIL_UINT8    _speed_match;
   ZIL_UINT8    _kermit_spoof;

   ZIL_UINT8    _is_tapi;

   /* Modem Definition */
   ZIL_INT32    _max_speed;	
   ZIL_UINT8    _at ;
   ZIL_UINT8    _v25;
   ZIL_UINT8    _sb ;
   ZIL_UINT8    _ec ;
   ZIL_UINT8    _dc ;
   ZIL_UINT8    _hw ;
   ZIL_UINT8    _sw ;
   ZIL_UINT8    _ks ;
   ZIL_UINT8    _tb ;
   ZIL_ICHAR    _escape_char;
   ZIL_ICHAR    _init_str[61];
   ZIL_ICHAR    _dial_mode_str[61];
   ZIL_ICHAR    _dial_mode_prompt_str[61];
   ZIL_ICHAR    _dial_str[61];
   ZIL_ICHAR    _dc_on_str[61];
   ZIL_ICHAR    _dc_off_str[61];
   ZIL_ICHAR    _ec_on_str[61];
   ZIL_ICHAR    _ec_off_str[61];
   ZIL_ICHAR    _aa_on_str[61];
   ZIL_ICHAR    _aa_off_str[61];
   ZIL_ICHAR    _sp_on_str[61];
   ZIL_ICHAR    _sp_off_str[61];
   ZIL_ICHAR    _vol1_str[61];
   ZIL_ICHAR    _vol2_str[61];
   ZIL_ICHAR    _vol3_str[61];
   ZIL_ICHAR    _hup_str[61];
   ZIL_ICHAR    _hwfc_str[61];
   ZIL_ICHAR    _swfc_str[61];
   ZIL_ICHAR    _nofc_str[61];
   ZIL_ICHAR    _pulse_str[61];
   ZIL_ICHAR    _tone_str[61];
   ZIL_ICHAR    _sb_on_str[61];
   ZIL_ICHAR    _sb_off_str[61];
   ZIL_ICHAR    _ignore_dial_tone_str[61];

   K_MODEM(void);
   K_MODEM( K_MODEM & );
   ~K_MODEM(void);

   virtual EVENT_TYPE Event( const UI_EVENT & ) ;

   static int Compare(void *, void *);

	// --- Persistent object members ---
#if defined(ZIL_LOAD)
   static K_MODEM * New(const ZIL_ICHAR *name,
			    ZIL_STORAGE_READ_ONLY *file = ZIL_NULLP(ZIL_STORAGE_READ_ONLY),
			    ZIL_STORAGE_OBJECT_READ_ONLY *object = ZIL_NULLP(ZIL_STORAGE_OBJECT_READ_ONLY),
			    UI_ITEM *_objectTable = ZIL_NULLP(UI_ITEM),
			    UI_ITEM *_userTable = ZIL_NULLP(UI_ITEM))
   { 
       K_MODEM * config = new K_MODEM(name, file, object, _objectTable, _userTable);
       return config ;
   }
	
   K_MODEM(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *file,
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

   K_MODEM_DIALOG * _dialog;
};
