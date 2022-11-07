//	LISTITM.HPP  - List Items example program.
//	COPYRIGHT (C) 1995.  All Rights Reserved.
//	Zinc Software Incorporated.  Pleasant Grove, Utah  USA
//	May be freely copied, used and distributed.

#ifndef LIST_ITEM_HPP
#define LIST_ITEM_HPP

#include "kcolor.hpp"
#include "kenum.hpp"

class ZIL_EXPORT_CLASS KD_LIST_ITEM : public UIW_BUTTON
{
public:
   static ZIL_ICHAR _className[];
   
   ZIL_ICHAR _name[65] ;
   ZIL_ICHAR _location[65] ;
   enum TRANSPORT _access ;         
   ZIL_ICHAR _phone_number[257] ;
   ZIL_ICHAR _ipaddress[257] ;
   ZIL_ICHAR _ipport[33];
   ZIL_ICHAR _lataddress[257] ;
   enum TCPPROTO _tcpproto;
   enum SSHPROTO _sshproto;
   ZIL_ICHAR _sshport[33];
   ZIL_ICHAR _ftpport[33];
   ZIL_UINT8 _template ;
   ZIL_ICHAR _startup_dir[257] ;
   ZIL_UINT8 _autoexit ;
   ZIL_ICHAR _notes[3001] ;

   enum TERMINAL _terminal ;
   ZIL_INT32   _charsize ;
   ZIL_UINT8   _local_echo ;
   ZIL_UINT8   _auto_wrap ;
   ZIL_UINT8   _apc_enabled ;
   enum YNA    _auto_download ;
   ZIL_UINT8   _status_line ;
   ZIL_INT32   _height ;
   ZIL_INT32   _width ;
   ZIL_INT32   _scrollback ;
   enum CURSOR_TYPE _cursor ;

   K_COLOR     _color_term_fg,
               _color_term_bg,
               _color_status_fg, 
               _color_status_bg,
               _color_popup_fg,
               _color_popup_bg,
               _color_select_fg,
               _color_select_bg,
               _color_underline_fg,
               _color_underline_bg,
               _color_command_fg,
               _color_command_bg ;
             
    enum TERMCSET  _term_charset ;
    enum PROTOCOL _protocol ;
    enum XFERMODE _xfermode ;
    enum COLLISION _collision ;

   ZIL_INT32 _packet_length ;
   ZIL_INT32 _max_windows ;

   ZIL_UINT8    _fname_literal ;
   ZIL_UINT8    _pathnames ;
   ZIL_UINT8    _keep_incomplete ;
   ZIL_UINT8    _streaming ;
   ZIL_UINT8    _clear_channel ;

   enum UNPREFIX_CC _unprefix_cc ;

   enum FILECSET _file_charset ;
   enum XFERCSET _xfer_charset ;

   ZIL_ICHAR    _line_device[61] ;
   ZIL_UINT8    _tapi_line ;
   ZIL_INT32  	_speed ;
   enum FLOW    _flow ;
   enum PARITY  _parity ;
   enum STOPBITS _stopbits;
   ZIL_ICHAR    _modem[61] ;
   ZIL_UINT8    _correction ;
   ZIL_UINT8    _compression ;
   ZIL_UINT8    _carrier ;

   enum BACKSPACE _backspace ;
   enum ENTER     _enter ;
   ZIL_UINT8       _mouse ;

   enum KEYMAP    _keymap ;
   ZIL_ICHAR      _keymap_file[257] ;
   ZIL_ICHAR      _keyscript[3001] ;

   ZIL_UINT8      _script_file ;
   ZIL_ICHAR      _script_fname[257] ;
   ZIL_ICHAR      _userid[21] ;
   ZIL_ICHAR      _password[21] ;
   ZIL_UINT8      _prompt_for_password;
   ZIL_ICHAR      _prompt[21] ;
   ZIL_ICHAR      _script[3001] ;

   ZIL_UINT8      _userdefined ;

   ZIL_UINT8	  _log_connection;
   ZIL_UINT8	  _log_connection_append;
   ZIL_ICHAR      _log_connection_fname[257];
   ZIL_UINT8	  _log_debug;
   ZIL_UINT8	  _log_debug_append;
   ZIL_ICHAR      _log_debug_fname[257];
   ZIL_UINT8	  _log_session;
   ZIL_UINT8	  _log_session_append;
   ZIL_ICHAR      _log_session_fname[257];
   enum LOGTYPE   _log_session_type;
   ZIL_UINT8	  _log_packet;
   ZIL_UINT8	  _log_packet_append;
   ZIL_ICHAR      _log_packet_fname[257];
   ZIL_UINT8	  _log_transaction;
   ZIL_UINT8	  _log_transaction_append;
   ZIL_ICHAR      _log_transaction_fname[257];
   ZIL_UINT8	  _log_transaction_brief;

   /* These are for dialup connections only */
   ZIL_UINT8      _use_mdm_speed;
   ZIL_UINT8      _use_mdm_flow;
   ZIL_UINT8      _use_mdm_parity;
   ZIL_UINT8      _use_mdm_stopbits;
   ZIL_UINT8      _use_mdm_ec;
   ZIL_UINT8      _use_mdm_dc;
   ZIL_UINT8      _use_mdm_carrier;

   ZIL_UINT8      _converted_1_11;

   enum PRINTER_TYPE _printer_type;
   ZIL_ICHAR      _printer_dos[257] ;
   ZIL_ICHAR      _printer_windows[257] ;
   ZIL_ICHAR      _printer_separator[257] ;
   ZIL_UINT8      _printer_formfeed;
   ZIL_ICHAR      _printer_terminator[65] ;
   ZIL_UINT16     _printer_timeout ;
   ZIL_UINT8      _printer_bidi;
   ZIL_INT32      _printer_speed ;
   enum FLOW      _printer_flow ;
   enum PARITY    _printer_parity ;
   ZIL_UINT8      _printer_ps;
   ZIL_UINT16     _printer_width;
   ZIL_UINT16     _printer_length;
   enum TERMCSET  _printer_charset ;

   enum TELNET_MODE _telnet_auth_mode;
   enum TELNET_MODE _telnet_binary_mode;
   enum TELNET_MODE _telnet_encrypt_mode;
   enum TELNET_MODE _telnet_kermit_mode;
   enum TELNET_MODE _telnet_fwdx_mode;
   enum TELNET_MODE _telnet_cpc_mode;
   ZIL_ICHAR      _telnet_ttype[41];
   ZIL_ICHAR      _telnet_location[65];
   ZIL_ICHAR      _telnet_acct[65];
   ZIL_ICHAR      _telnet_disp[65];
   ZIL_ICHAR      _telnet_job[65];
   ZIL_ICHAR      _telnet_printer[65];

   ZIL_UINT8      _default_ip_address;
   ZIL_ICHAR      _tcp_ip_address[18];
   ZIL_INT32      _tcp_sendbuf;
   ZIL_INT32      _tcp_recvbuf;
   enum THREE_WAY _tcp_rdns;
   ZIL_UINT8      _tcp_dns_srv;
   ZIL_UINT8      _fwd_cred;
   ZIL_UINT8      _telnet_wait;
   
   ZIL_ICHAR      _k4_realm[257];
   ZIL_ICHAR      _k4_princ[65];
   ZIL_UINT8      _k4_autoget;
   ZIL_UINT8      _k4_autodestroy;
   ZIL_UINT32     _k4_lifetime;

   ZIL_ICHAR      _k5_realm[257];
   ZIL_ICHAR      _k5_princ[65];
   ZIL_ICHAR      _k5_cache[257];
   ZIL_UINT8      _k5_autoget;
   ZIL_UINT8      _k5_autodestroy;
   ZIL_UINT32     _k5_lifetime;
   ZIL_UINT8      _k5_forwardable;
   ZIL_UINT8      _k5_getk4tkt;

   enum TELNET_MODE _telnet_starttls_mode;
   ZIL_ICHAR        _tls_cipher[257];
   ZIL_ICHAR        _tls_cert_file[257];
   ZIL_ICHAR        _tls_key_file[257];
   ZIL_ICHAR        _tls_verify_file[257];
   ZIL_ICHAR        _tls_verify_dir[257];
   ZIL_ICHAR        _tls_crl_file[257];
   ZIL_ICHAR        _tls_crl_dir[257];
   ZIL_UINT8        _tls_verbose;
   ZIL_UINT8        _tls_debug;
   enum TLS_VERIFY  _tls_verify_mode;
   ZIL_UINT8        _telnet_debug;
   ZIL_UINT8        _telnet_sb_delay;

   ZIL_UINT8            _ssh_compress;
   ZIL_UINT8            _ssh_x11;
   enum SSH1_CIPHER     _ssh1_cipher;
   enum SSH_HOST_CHECK  _ssh_host_check;
   ZIL_UINT8            _ssh2_auth_external_keyx,
                        _ssh2_auth_gssapi,
                        _ssh2_auth_hostbased,
                        _ssh2_auth_keyboard_interactive,
                        _ssh2_auth_password,
                        _ssh2_auth_publickey,
                        _ssh2_auth_srp_gex_sha1;
   ZIL_UINT8            _ssh2_cipher_3des,
                        _ssh2_cipher_aes128,
                        _ssh2_cipher_aes192,
                        _ssh2_cipher_aes256,
                        _ssh2_cipher_arcfour,
                        _ssh2_cipher_blowfish,
                        _ssh2_cipher_cast128;
   ZIL_UINT8            _ssh2_mac_md5,
                        _ssh2_mac_md5_96,
                        _ssh2_mac_ripemd160,
                        _ssh2_mac_sha1,
                        _ssh2_mac_sha1_96;
   ZIL_UINT8            _ssh2_hka_dss,
                        _ssh2_hka_rsa;

   ZIL_ICHAR            _ssh1_id_file[257];
   ZIL_ICHAR            _ssh1_kh_file[257];
   ZIL_ICHAR            _ssh2_id_file[257];
   ZIL_ICHAR            _ssh2_kh_file[257];

   ZIL_UINT8            _ftp_autologin;
   ZIL_UINT8            _ftp_passive;
   ZIL_UINT8            _ftp_autoauth;
   ZIL_UINT8            _ftp_autoenc;
   ZIL_UINT8            _ftp_credfwd;
   enum FTP_PL          _ftp_cpl;
   enum FTP_PL          _ftp_dpl;
   ZIL_UINT8            _ftp_debug;
   ZIL_UINT8            _ftp_dates;
   ZIL_UINT8            _ftp_literal;
   ZIL_UINT8            _ftp_verbose;
   ZIL_UINT8            _ftp_xlat;
   enum FILECSET        _ftp_charset ;
   ZIL_UINT8            _ftp_auth_gssk5;
   ZIL_UINT8            _ftp_auth_k4;
   ZIL_UINT8            _ftp_auth_srp;
   ZIL_UINT8            _ftp_auth_ssl;
   ZIL_UINT8            _ftp_auth_tls;

   ZIL_UINT8            _use_gui;
   ZIL_ICHAR            _facename[128];
   ZIL_UINT8            _fontsize;
   ZIL_UINT8            _startpos_auto;
   ZIL_UINT16           _startpos_x;
   ZIL_UINT16           _startpos_y;
   ZIL_UINT8            _rgb[16][3];

   ZIL_UINT8            _ssh_credfwd;

   ZIL_ICHAR            _proxy_ipaddress[257] ;
   ZIL_ICHAR            _proxy_ipport[33];
   ZIL_ICHAR            _socks_ipaddress[257] ;
   ZIL_ICHAR            _socks_ipport[33];

   ZIL_ICHAR            _proxy_user[65];
   ZIL_ICHAR            _proxy_pass[65];
   ZIL_ICHAR            _socks_user[65];
   ZIL_ICHAR            _socks_pass[65];

   enum GUI_RESIZE      _gui_resize;
   enum GUI_RUN         _gui_run;

   KD_LIST_ITEM(void);
   KD_LIST_ITEM(KD_LIST_ITEM &);
   KD_LIST_ITEM(TRANSPORT tType, ZIL_ICHAR *name, ZIL_ICHAR *location, 
                 ZIL_ICHAR *phoneNum, ZIL_ICHAR *hostname );
   ~KD_LIST_ITEM();

   void ConvertModemInfo(void);

   void ShowContextMenu(int left, int top);

   // --- Window object members ---
   virtual ZIL_ICHAR *ClassName(void) { return (_className); }

   virtual EVENT_TYPE DrawItem(const UI_EVENT &, EVENT_TYPE ccode);
   virtual EVENT_TYPE Event( const UI_EVENT & ) ;

   static int Compare(void *, void *);
      
	// --- Persistent object members ---
#if defined(ZIL_LOAD)
   void DataLoss(const ZIL_ICHAR * name);

   virtual ZIL_NEW_FUNCTION NewFunction(void) { return (KD_LIST_ITEM::New); }

   static UI_WINDOW_OBJECT *New(const ZIL_ICHAR *name,
				 ZIL_STORAGE_READ_ONLY *file = ZIL_NULLP(ZIL_STORAGE_READ_ONLY),
				 ZIL_STORAGE_OBJECT_READ_ONLY *object = ZIL_NULLP(ZIL_STORAGE_OBJECT_READ_ONLY),
				 UI_ITEM *_objectTable = ZIL_NULLP(UI_ITEM),
				 UI_ITEM *_userTable = ZIL_NULLP(UI_ITEM))
   { return (new KD_LIST_ITEM(name, file, object, _objectTable, _userTable)); }
	
   KD_LIST_ITEM(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *file,
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

   KD_LIST_ITEM * Next( void ) { return (KD_LIST_ITEM *) next ; }

protected:
   static TRANSPORT_IMAGE tImage[7];

};


#endif