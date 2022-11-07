#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "usermsg.hpp"
#include "kmodem.hpp"
#include "kmdmdlg.hpp"

extern ZIL_UINT8 kd_major, kd_minor;
ZIL_ICHAR K_MODEM::_className[] = "K_MODEM" ;

int CompareKModems( void * p1, void * p2 )
{
   return K_MODEM::Compare( p1, p2 );
}

EVENT_TYPE 
KModemUser( UI_WINDOW_OBJECT * obj, UI_EVENT & event,
	       EVENT_TYPE ccode )
{
    K_MODEM * entry = (K_MODEM *) obj ;

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

K_MODEM::K_MODEM():
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
               BTF_NO_FLAGS,  // Button flags
	       WOF_NO_FLAGS,                     // Window flags
               KModemUser,                    // User function
               0)         // Send Message Value  
{
    _name[0] = '\0';
    _type[0] = '\0' ;
    _port[0] = '\0' ;
    _speed = 57600;
    _flow = AUTO_FLOW;
    _parity = NO_PARITY;
    _stopbits = STOPBITS_1_0;
    _correction = 1;
    _compression = 1 ;
    _carrier = 1;
    _speaker = 1;
    _volume = 1; 
    _speed_match = 0;
    _kermit_spoof = 0;

    _max_speed = 57600;	
    _at=1;
    _v25=0;
    _sb=1;
    _ec=1;
    _dc=1;
    _hw=1;
    _sw=1;
    _ks=0;
    _tb=0;
    _escape_char='+';
    _init_str[0] = '\0';
    _dial_mode_str[0] = '\0';
    _dial_mode_prompt_str[0] = '\0';
    _dial_str[0] = '\0';
    _dc_on_str[0] = '\0';
    _dc_off_str[0] = '\0';
    _ec_on_str[0] = '\0';
    _ec_off_str[0] = '\0';
    _sb_on_str[0] = '\0';
    _sb_off_str[0] = '\0';
    _aa_on_str[0] = '\0';
    _aa_off_str[0] = '\0';
    _sp_on_str[0] = '\0';
    _sp_off_str[0] = '\0';
    _vol1_str[0] = '\0';
    _vol2_str[0] = '\0';
    _vol3_str[0] = '\0';
    _hup_str[0] = '\0';
    _hwfc_str[0] = '\0';
    _swfc_str[0] = '\0';
    _nofc_str[0] = '\0';
    _pulse_str[0] = '\0';
    _tone_str[0] = '\0';
    _ignore_dial_tone_str[0] = '\0';
    _is_tapi = 0;

    _dialog = NULL;
}


K_MODEM::K_MODEM( K_MODEM & source ):
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               source._name, // no text, ownerdraw
               BTF_NO_FLAGS,  // Button flags
	       WOF_NO_FLAGS,                     // Window flags
               KModemUser,                    // User function
               0)         // Send Message Value  
{
    strcpy(_name, source._name);
    strcpy(_type, source._type);
    strcpy(_port, source._port);
    _speed = source._speed;
    _flow  = source._flow;
    _parity = source._parity;
    _stopbits = source._stopbits;
    _correction = source._correction;
    _compression = source._compression;
    _carrier = source._carrier;
    _speaker = source._speaker;
    _volume = source._volume;
    _speed_match = source._speed_match;
    _kermit_spoof = source._kermit_spoof;
    _max_speed = source._max_speed;
    _at = source._at;
    _v25 = source._v25;
    _sb = source._sb;
    _ec = source._ec;
    _dc = source._dc;
    _hw = source._hw;
    _sw = source._sw;
    _ks = source._ks;
    _tb = source._tb;
    _escape_char = source._escape_char;
    strcpy(_init_str,source._init_str);
    strcpy(_dial_mode_str,source._dial_mode_str);
    strcpy(_dial_mode_prompt_str,source._dial_mode_prompt_str);
    strcpy(_dial_str,source._dial_str);
    strcpy(_dc_on_str,source._dc_on_str);
    strcpy(_dc_off_str,source._dc_off_str);
    strcpy(_ec_on_str,source._ec_on_str);
    strcpy(_ec_off_str,source._ec_off_str);
    strcpy(_sb_on_str,source._sb_on_str);
    strcpy(_sb_off_str,source._sb_off_str);
    strcpy(_aa_on_str,source._aa_on_str);
    strcpy(_aa_off_str,source._aa_off_str);
    strcpy(_sp_on_str,source._sp_on_str);
    strcpy(_sp_off_str,source._sp_off_str);
    strcpy(_vol1_str,source._vol1_str);
    strcpy(_vol2_str,source._vol2_str);
    strcpy(_vol3_str,source._vol3_str);
    strcpy(_hup_str,source._hup_str);
    strcpy(_hwfc_str,source._hwfc_str);
    strcpy(_swfc_str,source._swfc_str);
    strcpy(_nofc_str,source._nofc_str);
    strcpy(_pulse_str,source._pulse_str);
    strcpy(_tone_str,source._tone_str);
    strcpy(_ignore_dial_tone_str,source._ignore_dial_tone_str);

    _is_tapi = source._is_tapi;

    _dialog = source._dialog;
}


K_MODEM::~K_MODEM()
{
}

int K_MODEM::Compare(void *p1, void *p2)
{
    K_MODEM *object1 = (K_MODEM *)p1;
    K_MODEM *object2 = (K_MODEM *)p2;

    return 
	!strcmp(object1->_name, object2->_name ) &&
        !strcmp(object1->_type, object2->_type ) &&
    	!strcmp(object1->_port, object2->_port ) &&
	object1->_speed == object2->_speed &&
	object1->_flow  == object2->_flow &&
        object1->_parity  == object2->_parity &&
        object1->_stopbits == object2->_stopbits &&
        object1->_correction == object2->_correction &&
	object1->_compression == object2->_compression &&
	object1->_carrier == object2->_carrier &&
	object1->_speaker == object2->_speaker &&
	object1->_volume == object2->_volume &&
	object1->_speed_match == object2->_speed_match &&
	object1->_kermit_spoof == object2->_kermit_spoof &&
	object1->_max_speed == object2->_max_speed &&
	object1->_at == object2->_at &&
	object1->_v25 == object2->_v25 &&
	object1->_sb == object2->_sb &&
	object1->_ec == object2->_ec &&
	object1->_dc == object2->_dc &&
	object1->_hw == object2->_hw &&
	object1->_sw == object2->_sw &&
	object1->_ks == object2->_ks &&
	object1->_escape_char == object2->_escape_char &&
        !strcmp(object1->_init_str, object2->_init_str) &&
        !strcmp(object1->_dial_mode_str, object2->_dial_mode_str) &&
        !strcmp(object1->_dial_mode_prompt_str, object2->_dial_mode_prompt_str) &&
        !strcmp(object1->_dial_str, object2->_dial_str) &&
        !strcmp(object1->_dc_on_str, object2->_dc_on_str) &&
        !strcmp(object1->_dc_off_str, object2->_dc_off_str     ) &&
        !strcmp(object1->_ec_on_str, object2->_ec_on_str     ) &&
        !strcmp(object1->_ec_off_str, object2->_ec_off_str     ) &&
        !strcmp(object1->_sb_on_str, object2->_sb_on_str     ) &&
	!strcmp(object1->_sb_off_str, object2->_sb_off_str     ) &&
	!strcmp(object1->_aa_on_str, object2->_aa_on_str     ) &&
        !strcmp(object1->_aa_off_str, object2->_aa_off_str   ) &&
        !strcmp(object1->_sp_on_str, object2->_sp_on_str   ) &&
        !strcmp(object1->_sp_off_str, object2->_sp_off_str         ) &&
        !strcmp(object1->_vol1_str, object2->_vol1_str        ) &&
	!strcmp(object1->_vol2_str, object2->_vol2_str        ) &&
	!strcmp(object1->_vol3_str, object2->_vol3_str        ) &&
	!strcmp(object1->_hup_str, object2->_hup_str          ) &&
        !strcmp(object1->_hwfc_str, object2->_hwfc_str          ) &&
	!strcmp(object1->_swfc_str, object2->_swfc_str          ) &&
	!strcmp(object1->_nofc_str, object2->_nofc_str          ) &&
	!strcmp(object1->_pulse_str, object2->_pulse_str          ) &&
	!strcmp(object1->_tone_str, object2->_tone_str          ) &&
        !strcmp(object1->_ignore_dial_tone_str, object2->_ignore_dial_tone_str) &&
        (object1->_is_tapi == object2->_is_tapi);
}

// ----- ZIL_PERSISTENCE ----------------------------------------------------

#if defined(ZIL_LOAD)
K_MODEM::K_MODEM(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
			ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable) :
    UIW_BUTTON( 0,  // Left
		0,  // Top
		0,  // Width
		(ZIL_ICHAR *)name, // no text, ownerdraw
		BTF_NO_FLAGS,  // Button flags
		WOF_NO_FLAGS,                                 // Window flags
		KModemUser,                     // User function
		0)              // Send Message Value   
{
    _name[0] = '\0';
    _type[0] = '\0' ;
    _port[0] = '\0' ;
    _speed = 57600;
    _flow = AUTO_FLOW;
    _parity = NO_PARITY;
    _stopbits = STOPBITS_1_0;
    _correction = 1;
    _compression = 1 ;
    _carrier = 1;
    _speaker = 1;
    _volume = 1; 
    _speed_match = 0;
    _kermit_spoof = 0;

    _max_speed = 57600;	
    _at=1;
    _v25=0;
    _sb=1;
    _ec=1;
    _dc=1;
    _hw=1;
    _sw=1;
    _ks=0;
    _tb=0;
    _escape_char='+';
    _init_str[0] = '\0';
    _dial_mode_str[0] = '\0';
    _dial_mode_prompt_str[0] = '\0';
    _dial_str[0] = '\0';
    _dc_on_str[0] = '\0';
    _dc_off_str[0] = '\0';
    _ec_on_str[0] = '\0';
    _ec_off_str[0] = '\0';
    _sb_on_str[0] = '\0';
    _sb_off_str[0] = '\0';
    _aa_on_str[0] = '\0';
    _aa_off_str[0] = '\0';
    _sp_on_str[0] = '\0';
    _sp_off_str[0] = '\0';
    _vol1_str[0] = '\0';
    _vol2_str[0] = '\0';
    _vol3_str[0] = '\0';
    _hup_str[0] = '\0';
    _hwfc_str[0] = '\0';
    _swfc_str[0] = '\0';
    _nofc_str[0] = '\0';
    _pulse_str[0] = '\0';
    _tone_str[0] = '\0';
    _ignore_dial_tone_str[0] = '\0';
    _is_tapi = 0;

    _dialog = NULL;

    // Initialize the information.
    K_MODEM::Load(name, directory, file, objectTable, userTable);
    UI_WINDOW_OBJECT::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::DataSet(text);  // This sets the button string which should be empty
}

void K_MODEM::Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
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
    if ( length > 60 )
        return;
    file->Load(_type, length) ;
    _type[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_port, length) ;
    _port[60] = '\0' ;      

    file->Load(&_speed);
    
    file->Load(&Enum) ;
    _flow = (enum FLOW) Enum ;

    file->Load(&Enum) ;
    _parity = (enum PARITY) Enum ;

    file->Load(&_correction);
    file->Load(&_compression);
    file->Load(&_carrier);
    file->Load(&_speaker);
    file->Load(&_volume);
    file->Load(&_speed_match);
    file->Load(&_kermit_spoof);
    file->Load(&_max_speed);
    file->Load(&_at);
    file->Load(&_v25);
    file->Load(&_sb);
    file->Load(&_ec);
    file->Load(&_dc);
    file->Load(&_hw);
    file->Load(&_sw);
    file->Load(&_ks);
    file->Load(&_tb);
    ZIL_UINT8 ch;
    file->Load(&ch);
    _escape_char = (ZIL_ICHAR) ch;

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_init_str, length) ;
    _init_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_dial_mode_str, length) ;
    _dial_mode_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_dial_mode_prompt_str, length) ;
    _dial_mode_prompt_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_dial_str, length) ;
    _dial_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_dc_on_str, length) ;
    _dc_on_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_dc_off_str, length) ;
    _dc_off_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_ec_on_str, length) ;
    _ec_on_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_ec_off_str, length) ;
    _ec_off_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_sb_on_str, length) ;
    _sb_on_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_sb_off_str, length) ;
    _sb_off_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_aa_on_str, length) ;
    _aa_on_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_aa_off_str, length) ;
    _aa_off_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_sp_on_str, length) ;
    _sp_on_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_sp_off_str, length) ;
    _sp_off_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_vol1_str, length) ;
    _vol1_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_vol2_str, length) ;
    _vol2_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_vol3_str, length) ;
    _vol3_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_hup_str, length) ;
    _hup_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_hwfc_str, length) ;
    _hwfc_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_swfc_str, length) ;
    _swfc_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_nofc_str, length) ;
    _nofc_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_pulse_str, length) ;
    _pulse_str[60] = '\0' ;      

    file->Load(&length) ;
    if ( length > 60 )
        return;
    file->Load(_tone_str, length) ;
    _tone_str[60] = '\0' ;      

    file->Load(&_is_tapi);

    /* New to 1.1.14 */
    if ( itemMinor < 13 ) {
        /* I guess what we want to do here is load the value */
        /* out of the internal tables, what a mess */
        _ignore_dial_tone_str[0] = '\0';
    }
    else {
        file->Load(&length) ;
        if ( length > 60 )
            return;
        file->Load(_ignore_dial_tone_str, length) ;
        _ignore_dial_tone_str[60] = '\0' ;      
    }

    /* New to 1.1.18 CU */
    if ( itemMinor < 19 ) {
        _stopbits = STOPBITS_1_0;
    }
    else {
        file->Load(&Enum) ;
        _stopbits = (enum STOPBITS) Enum ;
    }

}
#endif

#if defined(ZIL_STORE)
void K_MODEM::Store(const ZIL_ICHAR *name, ZIL_STORAGE *directory,
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

    length = 1+strlen(_type);
   file->Store(length);
   file->Store(_type) ;

   length = 1+strlen(_port);
   file->Store(length);
   file->Store(_port) ;

    file->Store(_speed);
    file->Store( Enum = _flow ) ;
    file->Store( Enum = _parity ) ;
    file->Store(_correction) ;
    file->Store(_compression) ;
    file->Store(_carrier) ;
    file->Store(_speaker) ;
    file->Store(_volume) ;
    file->Store(_speed_match) ;
    file->Store(_kermit_spoof) ;
    file->Store(_max_speed) ;
    file->Store(_at) ;
    file->Store(_v25) ;
    file->Store(_sb) ;
    file->Store(_ec) ;
    file->Store(_dc) ;
    file->Store(_hw) ;
    file->Store(_sw) ;
    file->Store(_ks) ;
    file->Store(_tb) ;
    ZIL_UINT8 ch = _escape_char;
    file->Store(ch) ;

    length = 1+strlen(_init_str);
    file->Store(length);
    file->Store(_init_str) ;

    length = 1+strlen(_dial_mode_str);
    file->Store(length);
    file->Store(_dial_mode_str) ;

    length = 1+strlen(_dial_mode_prompt_str);
    file->Store(length);
    file->Store(_dial_mode_prompt_str) ;

    length = 1+strlen(_dial_str);
    file->Store(length);
    file->Store(_dial_str) ;

    length = 1+strlen(_dc_on_str);
    file->Store(length);
    file->Store(_dc_on_str) ;

    length = 1+strlen(_dc_off_str);
    file->Store(length);
    file->Store(_dc_off_str) ;

    length = 1+strlen(_ec_on_str);
    file->Store(length);
    file->Store(_ec_on_str) ;

    length = 1+strlen(_ec_off_str);
    file->Store(length);
    file->Store(_ec_off_str) ;

    length = 1+strlen(_sb_on_str);
    file->Store(length);
    file->Store(_sb_on_str) ;

    length = 1+strlen(_sb_off_str);
    file->Store(length);
    file->Store(_sb_off_str) ;

    length = 1+strlen(_aa_on_str);
    file->Store(length);
    file->Store(_aa_on_str) ;

    length = 1+strlen(_aa_off_str);
    file->Store(length);
    file->Store(_aa_off_str) ;

    length = 1+strlen(_sp_on_str);
    file->Store(length);
    file->Store(_sp_on_str) ;

    length = 1+strlen(_sp_off_str);
    file->Store(length);
    file->Store(_sp_off_str) ;

    length = 1+strlen(_vol1_str);
    file->Store(length);
    file->Store(_vol1_str) ;

    length = 1+strlen(_vol2_str);
    file->Store(length);
    file->Store(_vol2_str) ;

    length = 1+strlen(_vol3_str);
    file->Store(length);
    file->Store(_vol3_str) ;

    length = 1+strlen(_hup_str);
    file->Store(length);
    file->Store(_hup_str) ;

    length = 1+strlen(_hwfc_str);
    file->Store(length);
    file->Store(_hwfc_str) ;

    length = 1+strlen(_swfc_str);
    file->Store(length);
    file->Store(_swfc_str) ;

    length = 1+strlen(_nofc_str);
    file->Store(length);
    file->Store(_nofc_str) ;

    length = 1+strlen(_pulse_str);
    file->Store(length);
    file->Store(_pulse_str) ;

    length = 1+strlen(_tone_str);
    file->Store(length);
    file->Store(_tone_str) ;

    file->Store(_is_tapi);

    length = 1+strlen(_ignore_dial_tone_str);
    file->Store(length);
    file->Store(_ignore_dial_tone_str) ;

    file->Store( Enum = _stopbits ) ;

}

EVENT_TYPE 
K_MODEM :: Event( const UI_EVENT & event ) 
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

