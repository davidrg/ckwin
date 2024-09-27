//	LSTITM.CPP  - List Items example program.
//	COPYRIGHT (C) 1995.  All Rights Reserved.
//	Zinc Software Incorporated.  Pleasant Grove, Utah  USA
//	May be freely copied, used and distributed.

#define INCL_DOSQUEUES
#include <ui_win.hpp>
#define USE_WINDOW_DIALER
#include <stdio.h>
#include "dialer.hpp"
#include "usermsg.hpp"
#include "lstitm.hpp"
#include "kconnect.hpp"
#include "kstatus.hpp"
#include "kmodem.hpp"
#ifdef COMMENT
#undef COMMENT
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

extern "C" {
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#include "kmdminf.h"
MDMINF * FindMdmInf( char * name );
}
void CopyMdmInfToModem( MDMINF *, K_MODEM * );

extern ZIL_UINT8 kd_major, kd_minor;
extern K_CONNECTOR * connector ;
extern UIW_TOOL_BAR * toolbar ;
extern ZIL_INT32 KermitInstance ;

ZIL_ICHAR KD_LIST_ITEM::_className[] = "KD_LIST_ITEM" ;

int CompareKDListItems( void * p1, void * p2 )
{
   return KD_LIST_ITEM::Compare( p1, p2 );
}

EVENT_TYPE KDListItemUser( UI_WINDOW_OBJECT * obj, UI_EVENT & event, 
			   EVENT_TYPE ccode )
{
   KD_LIST_ITEM * entry = (KD_LIST_ITEM *) obj ;

   switch ( ccode )
   {
   case L_DOUBLE_CLICK: {
       connector->Connect( event ) ;
       break;
   }

   case S_CURRENT:
       if (connector)
	   connector->UpdateStatusLine(TRUE) ;
       break;

   case S_NON_CURRENT:
       break;
   }
   return 0;
}

KD_LIST_ITEM::KD_LIST_ITEM():
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
               BTF_DOUBLE_CLICK | BTF_NO_3D | BTF_SEND_MESSAGE,  // Button flags
	       WOF_NO_FLAGS,                      // Window flags
               KDListItemUser,                    // User function
               OPT_CONNECT_TO_ENTRY)              // Send Message Value  
{
    woStatus |= WOS_OWNERDRAW;
    strcpy(_name,"DEFAULT")  ;
    _location[0] = '\0' ;
    _access = NO_TRANSPORT ;
    _phone_number[0] = '\0' ;
    _ipaddress[0] = '\0' ;
    _lataddress[0] = '\0' ;
    _template = 1;
    strcpy(_startup_dir,"\\v(personal)download") ;
    _autoexit = 1 ;
    _notes[0] = '\0' ;
    _terminal = VT100;
    _telnet_ttype[0] = '\0';
    _charsize = 8 ;
    _local_echo = 0 ;
    _auto_wrap = 1 ;
    _apc_enabled = 0 ;
    _auto_download = YNA_ASK;
    _status_line = 1 ;
    _height = 24 ;
    _width = 80 ;
    _scrollback = 512 ;
    _cursor = FULL_CURSOR ;
    _color_term_fg = K_LIGHTGRAY ;
    _color_term_bg = K_BLUE ;
    _color_status_fg = K_LIGHTGRAY ;
    _color_status_bg = K_CYAN ;
    _color_popup_fg = K_LIGHTGRAY ;
    _color_popup_bg = K_CYAN ;
    _color_select_fg = K_BLACK ;
    _color_select_bg = K_YELLOW ;
    _color_underline_fg = K_LIGHTGRAY ;
    _color_underline_bg = K_BLACK ;
    _color_command_fg = K_LIGHTGRAY ;
    _color_command_bg = K_BLACK ;
    _term_charset = T_LATIN1_ISO ;
    _protocol = K_FAST ;
    _xfermode = BINARY ;
    _collision = COL_BACKUP ;
    _packet_length = 4096 ;
    _max_windows = 20 ;
    _fname_literal = 1 ;
    _pathnames = 0 ;
    _keep_incomplete = 1 ;
    _streaming = 1;
    _clear_channel = 1;
    _unprefix_cc = CAUTIOUSLY ;
    _file_charset = F_CP437 ;
    _xfer_charset = X_LATIN1_ISO ;
    _line_device[0] = '\0' ;
    _tapi_line = 0;
    _speed = 57600 ;
    _flow = AUTO_FLOW ;
    _parity = NO_PARITY ;
    strcpy( _modem, "DEFAULT" ) ;
    _correction = 1 ;
    _compression = 1 ;
    _carrier = 1 ;
    _backspace = DEL ;
    _enter = CR ;
    _mouse = 1 ;
    _keymap = VT100_KEY ;
    _keymap_file[0] = '\0' ;
    _keyscript[0] = '\0';
    _script_file = 0 ;
    _script_fname[0] = '\0' ;
    _userid[0] = '\0' ;
    _password[0] = '\0' ;  
    _prompt[0] = '\0' ;
    _script[0] = '\0';
    _log_debug = 0;
    _log_debug_append = 0;
    strcpy(_log_debug_fname,"\\v(appdata)debug.log");
    _log_session = 0;
    _log_session_append = 0;
    strcpy(_log_session_fname,"\\v(appdata)session.log");
    _log_packet = 0;
    _log_packet_append = 0;
    strcpy(_log_packet_fname,"\\v(appdata)packet.log");
    _log_transaction = 0;
    _log_transaction_append = 0;
    strcpy(_log_transaction_fname,"\\v(appdata)transact.log");

    _use_mdm_speed = 1;
    _use_mdm_flow = 1;
    _use_mdm_parity = 1;
    _use_mdm_ec = 1;
    _use_mdm_dc = 1;
    _use_mdm_carrier = 1;

    _converted_1_11 = 1;

#ifdef NT
    _printer_type = PrinterWindows;
#else
    _printer_type = PrinterDOS;
#endif
    strcpy(_printer_dos,"LPT1:");
    strcpy(_printer_windows,"<default>");
    _printer_separator[0] = '\0';
    _printer_formfeed = 0;       
    _printer_terminator[0] = '\0';
    _printer_timeout = 0 ;       
    _printer_bidi=0;           
    _printer_speed=9600;         
    _printer_flow = NO_FLOW ;          
    _printer_parity = NO_PARITY;
    _printer_charset = T_CP437 ;

    _prompt_for_password = 0;

    _log_transaction_brief = 0;
    _telnet_auth_mode = TelnetAccept;
    _telnet_binary_mode = TelnetAccept;
    _telnet_encrypt_mode = TelnetAccept;
    _telnet_location[0] = '\0';
    _default_ip_address = 1;
    _tcp_ip_address[0] = '\0';
    _tcp_sendbuf = 8192;
    _tcp_recvbuf = 8192;
    _tcp_rdns = AUTO;
    _telnet_acct[0] = '\0';
    _telnet_disp[0] = '\0';
    _telnet_job[0] = '\0';
    _telnet_printer[0] = '\0';
    _k4_realm[0] = '\0';
    _k4_princ[0] = '\0';
    _k5_realm[0] = '\0';
    _k5_princ[0] = '\0';
    _k5_cache[0] = '\0';

    _ipport[0] = '\0';
    _sshport[0] = '\0';
    _ftpport[0] = '\0';
    _stopbits = STOPBITS_1_0;
    _use_mdm_stopbits = 1;
    _telnet_kermit_mode = TelnetAccept;
    _tcp_dns_srv = 0;
    _fwd_cred = 0;
    _telnet_wait = 1;
    _k4_autoget = 0;
    _k4_autodestroy = 0;
    _k4_lifetime = 600;
    _k5_autoget = 0;
    _k5_autodestroy = 0;
    _k5_lifetime = 600;
    _k5_forwardable = 0;
    _k5_getk4tkt = 1;
    _printer_ps = 0;
    _printer_width = 80;
    _printer_length = 66;
    _log_connection = 0;
    _log_connection_append = 1;
    strcpy(_log_connection_fname,"connection.log");
    _log_session_type = LOG_BINARY;

    _telnet_starttls_mode = TelnetAccept;
    _tls_cipher[0] = '\0';
    _tls_cert_file[0] = '\0';
    _tls_key_file[0] = '\0';
    _tls_verify_file[0] = '\0';
    _tls_verify_dir[0] = '\0';
    _tls_crl_file[0] = '\0';
    _tls_crl_dir[0] = '\0';
    _tls_verbose = 0;
    _tls_debug = 0;
    _tls_verify_mode = TLS_VERIFY_PEER;
    _telnet_debug = 0;
    _telnet_sb_delay = 1;

    _tcpproto = TCP_DEFAULT;
    _sshproto = SSH_AUTO;

    _ssh_compress = 1;
    _ssh_x11 = 0;
    _ssh1_cipher = SSH1_CIPHER_3DES; /* OBSOLETE */
    _ssh_host_check = HC_ASK;

    _ssh2_auth_external_keyx = 0; /* OBSOLETE */
    _ssh2_auth_gssapi = 1;
    _ssh2_auth_hostbased = 0; /* OBSOLETE */
    _ssh2_auth_keyboard_interactive = 1;
    _ssh2_auth_password = 1;
    _ssh2_auth_publickey = 1;
    _ssh2_auth_srp_gex_sha1 = 0; /* OBSOLETE */

    _ssh2_cipher_3des = 1;
    _ssh2_cipher_aes128 = 1;
    _ssh2_cipher_aes192 = 1;
    _ssh2_cipher_aes256 = 1;
    _ssh2_cipher_arcfour = 0; /* OBSOLETE */
    _ssh2_cipher_blowfish = 0; /* OBSOLETE */
    _ssh2_cipher_cast128 = 0; /* OBSOLETE */
    _ssh2_cipher_aes128ctr = 1;
    _ssh2_cipher_aes192ctr = 1;
    _ssh2_cipher_aes256ctr = 1;
    _ssh2_cipher_aes256_gcm_openssh = 1;
    _ssh2_cipher_chachae20_poly1305 = 1;
    _ssh2_cipher_aes128_gcm_openssh = 1;

    _ssh2_mac_md5 = 1;
    _ssh2_mac_md5_96 = 0; /* OBSOLETE */
    _ssh2_mac_ripemd160 = 0; /* OBSOLETE */
    _ssh2_mac_sha1 = 1;
    _ssh2_mac_sha1_96 = 0; /* OBSOLETE */
    _ssh2_mac_none = 0;
    _ssh2_mac_sha2_512 = 1;
    _ssh2_mac_sha2_256 = 1;
    _ssh2_mac_sha1_etm_openssh = 1;
    _ssh2_mac_sha2_512_etm_openssh = 1;
    _ssh2_mac_sha2_256_etm_openssh = 1;

    _ssh2_hka_dss = 0; /* deprecated as of CKWIN */
    _ssh2_hka_rsa = 1;
    _ssh2_hka_ssh_ed25519 = 1;
    _ssh2_hka_ecdsa_sha2_nistp521 = 1;
    _ssh2_hka_ecdsa_sha2_nistp384 = 1;
    _ssh2_hka_ecdsa_sha2_nistp256 = 1;
    _ssh2_hka_rsa_sha2_512 = 1;
    _ssh2_hka_rsa_sha2_256 = 1;

    _ssh2_kex_curve25519_sha256 = 1;
    _ssh2_kex_curve25519_sha256_libssh = 1;
    _ssh2_kex_ecdh_sha2_nistp256 = 1;
    _ssh2_kex_ecdh_sha2_nistp384 = 1;
    _ssh2_kex_ecdh_sha2_nistp521 = 1;
    _ssh2_kex_dh_group18_sha512 = 1;
    _ssh2_kex_dh_group16_sha512 = 1;
    _ssh2_kex_dh_group_exchange_sha256 = 1;
    _ssh2_kex_dh_group14_sha256 = 1;
    _ssh2_kex_dh_group14_sha1 = 1;
    _ssh2_kex_dh_group1_sha1 = 1;
    _ssh2_kex_ext_info_c = 1;
    _ssh2_kex_dh_group_exchange_sha1 = 1;

    _ssh1_id_file[0] = '\0';
    _ssh1_kh_file[0] = '\0';
    _ssh2_id_file[0] = '\0';
    _ssh2_kh_file[0] = '\0';

    _ftp_autologin = 1;  
    _ftp_passive   = 1;    
    _ftp_autoauth  = 1;   
    _ftp_autoenc   = 1;    
    _ftp_credfwd   = 0;    
    _ftp_cpl       = PL_PRIVATE;  
    _ftp_dpl       = PL_PRIVATE;     
    _ftp_debug     = 0;      
    _ftp_dates     = 1;      
    _ftp_literal   = 1;    
    _ftp_verbose   = 1;    
    _ftp_xlat      = 0;       
    _ftp_charset   = F_ASCII;   
    _ftp_auth_gssk5 = 1; 
    _ftp_auth_k4   = 1;    
    _ftp_auth_srp  = 1;   
    _ftp_auth_ssl  = 1;   
    _ftp_auth_tls  = 1;   
    _telnet_fwdx_mode = TelnetAccept;
    _telnet_cpc_mode = TelnetAccept;

    _use_gui = 1;
    strcpy(_facename,"Courier New");
    _fontsize = 12;
    _startpos_auto = 1;
    _startpos_x = 0;
    _startpos_y = 0;

    // Idx  Colour   R   G   B
    // 0    Black    0   0   0
    _rgb[0][0] = 0;
    _rgb[0][1] = 0;
    _rgb[0][2] = 0;

    // 1    Blue        0   0   127
    _rgb[1][0] = 0;
    _rgb[1][1] = 0;
    _rgb[1][2] = 127;

    // 2    Green       0   127 0
    _rgb[2][0] = 0;
    _rgb[2][1] = 127;
    _rgb[2][2] = 0;

    // 3    Cyan        0   127 127
    _rgb[3][0] = 0;
    _rgb[3][1] = 127;
    _rgb[3][2] = 127;

    // 4    Red         127 0   0
    _rgb[4][0] = 128;
    _rgb[4][1] = 0;
    _rgb[4][2] = 0;

    // 5    Magenta     127 0   127
    _rgb[5][0] = 127;
    _rgb[5][1] = 0;
    _rgb[5][2] = 127;

    // 6    Brown       127 127 0
    _rgb[6][0] = 127;
    _rgb[6][1] = 127;
    _rgb[6][2] = 0;

    // 7    Lightgray   192 192 192
    _rgb[7][0] = 192;
    _rgb[7][1] = 192;
    _rgb[7][2] = 192;

    // 8    darkgray    127 127 127
    _rgb[8][0] = 127;
    _rgb[8][1] = 127;
    _rgb[8][2] = 127;

    // 9    lightblue   0   0   255
    _rgb[9][0] = 0;
    _rgb[9][1] = 0;
    _rgb[9][2] = 255;

    // 10   lightgreen  0   255 0
    _rgb[10][0] = 0;
    _rgb[10][1] = 255;
    _rgb[10][2] = 0;

    // 11   lightcyan   0   255 255
    _rgb[11][0] = 0;
    _rgb[11][1] = 255;
    _rgb[11][2] = 255;

    // 12   lightred    255 0   0
    _rgb[12][0] = 255;
    _rgb[12][1] = 0;
    _rgb[12][2] = 0;

    // 13   lightmagenta    255 0   255
    _rgb[13][0] = 255;
    _rgb[13][1] = 0;
    _rgb[13][2] = 255;

    // 14   yellow      255 255 0
    _rgb[14][0] = 255;
    _rgb[14][1] = 255;
    _rgb[14][2] = 0;

    // 15   white       255 255 255
    _rgb[15][0] = 255;
    _rgb[15][1] = 255;
    _rgb[15][2] = 255;

    _ssh_credfwd = 0;
    _proxy_ipaddress[0] = '\0';
    _proxy_ipport[0] = '\0';
    _socks_ipaddress[0] = '\0';
    _socks_ipport[0] = '\0';
    _proxy_user[0] = '\0';
    _proxy_pass[0] = '\0';
    _socks_user[0] = '\0';
    _socks_pass[0] = '\0';

    _gui_resize = RES_CHANGE_DIM;
    _gui_run = RUN_RES;
}

KD_LIST_ITEM::KD_LIST_ITEM(KD_LIST_ITEM & item):
   UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
               BTF_DOUBLE_CLICK | BTF_NO_3D | BTF_SEND_MESSAGE,  // Button flags
	       WOF_NO_FLAGS,                      // Window flags
               KDListItemUser,                    // User function
               OPT_CONNECT_TO_ENTRY)              // Send Message Value  
{
    woStatus = item.woStatus;
    strcpy(_name,item._name);
    strcpy(_location,item._location);
    _access = item._access;
    strcpy(_phone_number,item._phone_number);
    strcpy(_ipaddress,item._ipaddress);
    strcpy(_lataddress,item._ipaddress);
    _template = item._template;
    strcpy(_startup_dir,item._startup_dir);
    _autoexit = item._autoexit;
    strcpy(_notes,item._notes);
    _terminal = item._terminal;
    strcpy(_telnet_ttype,item._telnet_ttype);
    _charsize = item._charsize;
    _local_echo = item._local_echo;
    _auto_wrap = item._auto_wrap;
    _apc_enabled = item._apc_enabled;
    _auto_download = item._auto_download;
    _status_line = item._status_line;
    _height = item._height;
    _width = item._width;
    _scrollback = item._scrollback;
    _cursor = item._cursor;
    _color_term_fg = item._color_term_fg;
    _color_term_bg = item._color_term_bg;
    _color_status_fg = item._color_status_fg;
    _color_status_bg = item._color_status_bg;
    _color_popup_fg = item._color_popup_fg;
    _color_popup_bg = item._color_popup_bg;
    _color_select_fg = item._color_select_fg;
    _color_select_bg = item._color_select_bg;
    _color_underline_fg = item._color_underline_fg;
    _color_underline_bg = item._color_underline_bg;
    _color_command_fg = item._color_command_fg;
    _color_command_bg = item._color_command_bg;
    _term_charset = item._term_charset;
    _protocol = item._protocol;
    _xfermode = item._xfermode;
    _collision = item._collision;
    _packet_length = item._packet_length;
    _max_windows = item._max_windows;
    _fname_literal = item._fname_literal;
    _pathnames = item._pathnames;
    _keep_incomplete = item._keep_incomplete;
    _streaming = item._streaming;
    _clear_channel = item._clear_channel;
    _unprefix_cc = item._unprefix_cc;
    _file_charset = item._file_charset;
    _xfer_charset = item._xfer_charset;
    strcpy(_line_device,item._line_device);
    _tapi_line = item._tapi_line;
    _speed = item._speed;
    _flow = item._flow;
    _parity = item._parity;
    strcpy( _modem, item._modem) ;
    _correction = item._correction;
    _compression = item._compression ;
    _carrier = item._carrier ;
    _backspace = item._backspace;
    _enter = item._enter;
    _mouse = item._mouse;
    _keymap = item._keymap;
    strcpy(_keymap_file,item._keymap_file);
    strcpy(_keyscript,item._keyscript);
    _script_file = item._script_file;
    strcpy(_script_fname,item._script_fname);
    strcpy(_userid,item._userid);
    strcpy(_password,item._password);  
    strcpy(_prompt,item._prompt);
    strcpy(_script,item._script);
    _log_debug = item._log_debug;
    _log_debug_append = item._log_debug_append;
    strcpy(_log_debug_fname,item._log_debug_fname);
    _log_session = item._log_session;
    _log_session_append = item._log_session_append;
    strcpy(_log_session_fname,item._log_session_fname);
    _log_packet = item._log_packet;
    _log_packet_append = item._log_packet_append;
    strcpy(_log_packet_fname,item._log_packet_fname);
    _log_transaction = item._log_transaction;
    _log_transaction_append = item._log_transaction_append;
    strcpy(_log_transaction_fname,item._log_transaction_fname);

    _use_mdm_speed = item._use_mdm_speed;
    _use_mdm_flow = item._use_mdm_flow;
    _use_mdm_parity = item._use_mdm_parity;
    _use_mdm_ec = item._use_mdm_ec;
    _use_mdm_dc = item._use_mdm_dc;
    _use_mdm_carrier = item._use_mdm_carrier;

    _converted_1_11 = item._converted_1_11;

    _printer_type = item._printer_type;
    strcpy(_printer_dos,item._printer_dos);
    strcpy(_printer_windows,item._printer_windows);
    strcpy(_printer_separator,item._printer_separator);
    _printer_formfeed = item._printer_formfeed;
    strcpy(_printer_terminator,item._printer_terminator);
    _printer_timeout = item._printer_timeout; 
    _printer_bidi=item._printer_bidi;
    _printer_speed= item._printer_speed;  
    _printer_flow = item._printer_flow;
    _printer_parity = item._printer_parity;
    _printer_charset = item._printer_charset;

    _prompt_for_password = item._prompt_for_password;

    _log_transaction_brief = item._log_transaction_brief;
    _telnet_auth_mode = item._telnet_auth_mode;
    _telnet_binary_mode = item._telnet_binary_mode;
    _telnet_encrypt_mode = item._telnet_encrypt_mode;
    strcpy(_telnet_location,item._telnet_location);
    _default_ip_address = item._default_ip_address;
    strcpy(_tcp_ip_address,item._tcp_ip_address);
    _tcp_sendbuf = item._tcp_sendbuf;
    _tcp_recvbuf = item._tcp_recvbuf;
    _tcp_rdns = item._tcp_rdns;
    strcpy(_telnet_acct,item._telnet_acct);
    strcpy(_telnet_disp,item._telnet_disp);
    strcpy(_telnet_job,item._telnet_job);
    strcpy(_telnet_printer,item._telnet_printer);
    strcpy(_k4_realm,item._k4_realm);
    strcpy(_k4_princ,item._k4_princ);
    strcpy(_k5_realm,item._k5_realm);
    strcpy(_k5_princ,item._k5_princ);
    strcpy(_k5_cache,item._k5_cache);

    strcpy(_ipport,item._ipport);
    strcpy(_sshport,item._sshport);
    strcpy(_ftpport,item._ftpport);
    _stopbits = item._stopbits;
    _use_mdm_stopbits = item._use_mdm_stopbits;
    _telnet_kermit_mode = item._telnet_kermit_mode;
    _telnet_fwdx_mode = item._telnet_fwdx_mode;
    _telnet_cpc_mode = item._telnet_cpc_mode;
    _tcp_dns_srv = item._tcp_dns_srv;
    _fwd_cred = item._fwd_cred;
    _telnet_wait = item._telnet_wait;
    _k4_autoget = item._k4_autoget;
    _k4_autodestroy = item._k4_autodestroy;
    _k4_lifetime = item._k4_lifetime;
    _k5_autoget = item._k5_autoget;
    _k5_autodestroy = item._k5_autodestroy;
    _k5_lifetime = item._k5_lifetime;
    _k5_forwardable = item._k5_forwardable;
    _k5_getk4tkt = item._k5_getk4tkt;
    _printer_ps = item._printer_ps;
    _printer_width = item._printer_width;
    _printer_length = item._printer_length;
    _log_connection = item._log_connection;
    _log_connection_append = item._log_connection_append;
    strcpy(_log_connection_fname,item._log_connection_fname);
    _log_session_type = item._log_session_type;

    _telnet_starttls_mode = item._telnet_starttls_mode;
    strcpy(_tls_cipher,item._tls_cipher);
    strcpy(_tls_cert_file,item._tls_cert_file);
    strcpy(_tls_key_file,item._tls_key_file);
    strcpy(_tls_verify_file,item._tls_verify_file);
    strcpy(_tls_verify_dir,item._tls_verify_dir);
    strcpy(_tls_crl_file,item._tls_crl_file);
    strcpy(_tls_crl_dir,item._tls_crl_dir);
    _tls_verbose = item._tls_verbose;
    _tls_debug = item._tls_debug;
    _tls_verify_mode = item._tls_verify_mode;
    _telnet_debug = item._telnet_debug;
    _telnet_sb_delay = item._telnet_sb_delay;

    _tcpproto = item._tcpproto;
    _sshproto = item._sshproto;

    _ssh_compress = item._ssh_compress;
    _ssh_x11 = item._ssh_x11;
    _ssh1_cipher = item._ssh1_cipher;
    _ssh_host_check = item._ssh_host_check;
    _ssh2_auth_external_keyx = item._ssh2_auth_external_keyx;
    _ssh2_auth_gssapi = item._ssh2_auth_gssapi;
    _ssh2_auth_hostbased = item._ssh2_auth_hostbased;
    _ssh2_auth_keyboard_interactive = item._ssh2_auth_keyboard_interactive;
    _ssh2_auth_password = item._ssh2_auth_password;
    _ssh2_auth_publickey = item._ssh2_auth_publickey;
    _ssh2_auth_srp_gex_sha1 = item._ssh2_auth_srp_gex_sha1;

    _ssh2_cipher_3des = item._ssh2_cipher_3des;
    _ssh2_cipher_aes128 = item._ssh2_cipher_aes128;
    _ssh2_cipher_aes192 = item._ssh2_cipher_aes192;
    _ssh2_cipher_aes256 = item._ssh2_cipher_aes256;
    _ssh2_cipher_arcfour = item._ssh2_cipher_arcfour; /* OBSOLETE */
    _ssh2_cipher_blowfish = item._ssh2_cipher_blowfish; /* OBSOLETE */
    _ssh2_cipher_cast128 = item._ssh2_cipher_cast128; /* OBSOLETE */
    _ssh2_cipher_aes128ctr = item._ssh2_cipher_aes128ctr;
    _ssh2_cipher_aes192ctr = item._ssh2_cipher_aes192ctr;
    _ssh2_cipher_aes256ctr = item._ssh2_cipher_aes256ctr;
    _ssh2_cipher_aes256_gcm_openssh = item._ssh2_cipher_aes256_gcm_openssh;
    _ssh2_cipher_chachae20_poly1305 = item._ssh2_cipher_chachae20_poly1305;
    _ssh2_cipher_aes128_gcm_openssh = item._ssh2_cipher_aes128_gcm_openssh;

    _ssh2_mac_md5 = item._ssh2_mac_md5;
    _ssh2_mac_md5_96 = item._ssh2_mac_md5_96;
    _ssh2_mac_ripemd160 = item._ssh2_mac_ripemd160;
    _ssh2_mac_sha1 = item._ssh2_mac_sha1;
    _ssh2_mac_sha1_96 = item._ssh2_mac_sha1_96;
    _ssh2_mac_none = item._ssh2_mac_none;
    _ssh2_mac_sha2_512 = item._ssh2_mac_sha2_512;
    _ssh2_mac_sha2_256 = item._ssh2_mac_sha2_256;
    _ssh2_mac_sha1_etm_openssh = item._ssh2_mac_sha1_etm_openssh;
    _ssh2_mac_sha2_512_etm_openssh = item._ssh2_mac_sha2_512_etm_openssh;
    _ssh2_mac_sha2_256_etm_openssh = item._ssh2_mac_sha2_256_etm_openssh;

    _ssh2_hka_dss = item._ssh2_hka_dss; /* deprecated as of CKWIN */
    _ssh2_hka_rsa = item._ssh2_hka_rsa;
    _ssh2_hka_ssh_ed25519 = item._ssh2_hka_ssh_ed25519;
    _ssh2_hka_ecdsa_sha2_nistp521 = item._ssh2_hka_ecdsa_sha2_nistp521;
    _ssh2_hka_ecdsa_sha2_nistp384 = item._ssh2_hka_ecdsa_sha2_nistp384;
    _ssh2_hka_ecdsa_sha2_nistp256 = item._ssh2_hka_ecdsa_sha2_nistp256;
    _ssh2_hka_rsa_sha2_512 = item._ssh2_hka_rsa_sha2_512;
    _ssh2_hka_rsa_sha2_256 = item._ssh2_hka_rsa_sha2_256;

    _ssh2_kex_curve25519_sha256 = item._ssh2_kex_curve25519_sha256;
    _ssh2_kex_curve25519_sha256_libssh = item._ssh2_kex_curve25519_sha256_libssh;
    _ssh2_kex_ecdh_sha2_nistp256 = item._ssh2_kex_ecdh_sha2_nistp256;
    _ssh2_kex_ecdh_sha2_nistp384 = item._ssh2_kex_ecdh_sha2_nistp384;
    _ssh2_kex_ecdh_sha2_nistp521 = item._ssh2_kex_ecdh_sha2_nistp521;
    _ssh2_kex_dh_group18_sha512 = item._ssh2_kex_dh_group18_sha512;
    _ssh2_kex_dh_group16_sha512 = item._ssh2_kex_dh_group16_sha512;
    _ssh2_kex_dh_group_exchange_sha256 = item._ssh2_kex_dh_group_exchange_sha256;
    _ssh2_kex_dh_group14_sha256 = item._ssh2_kex_dh_group14_sha256;
    _ssh2_kex_dh_group14_sha1 = item._ssh2_kex_dh_group14_sha1;
    _ssh2_kex_dh_group1_sha1 = item._ssh2_kex_dh_group1_sha1;
    _ssh2_kex_ext_info_c = item._ssh2_kex_ext_info_c;
    _ssh2_kex_dh_group_exchange_sha1 = item._ssh2_kex_dh_group_exchange_sha1;

    strcpy(_ssh1_id_file,item._ssh1_id_file);
    strcpy(_ssh1_kh_file,item._ssh1_kh_file);
    strcpy(_ssh2_id_file,item._ssh2_id_file);
    strcpy(_ssh2_kh_file,item._ssh2_kh_file);

    _ftp_autologin = item.  _ftp_autologin;  
    _ftp_passive = item.    _ftp_passive;    
    _ftp_autoauth = item.   _ftp_autoauth;   
    _ftp_autoenc = item.    _ftp_autoenc;    
    _ftp_credfwd = item.    _ftp_credfwd;    
    _ftp_cpl = item.        _ftp_cpl;        
    _ftp_dpl = item.        _ftp_dpl;        
    _ftp_debug = item.      _ftp_debug;      
    _ftp_dates = item.      _ftp_dates;      
    _ftp_literal = item.    _ftp_literal;    
    _ftp_verbose = item.    _ftp_verbose;    
    _ftp_xlat = item.       _ftp_xlat;       
    _ftp_charset  = item.   _ftp_charset ;   
    _ftp_auth_gssk5 = item. _ftp_auth_gssk5; 
    _ftp_auth_k4 = item.    _ftp_auth_k4;    
    _ftp_auth_srp = item.   _ftp_auth_srp;   
    _ftp_auth_ssl = item.   _ftp_auth_ssl;   
    _ftp_auth_tls = item.   _ftp_auth_tls;   

    _use_gui = item._use_gui;
    strcpy(_facename,item._facename);
    _fontsize = item._fontsize;
    _startpos_auto = item._startpos_auto;
    _startpos_x = item._startpos_x;
    _startpos_y = item._startpos_y;
    _rgb[0][0] = item._rgb[0][0];
    _rgb[0][1] = item._rgb[0][1];
    _rgb[0][2] = item._rgb[0][2];
    _rgb[1][0] = item._rgb[1][0];
    _rgb[1][1] = item._rgb[1][1];
    _rgb[1][2] = item._rgb[1][2];
    _rgb[2][0] = item._rgb[2][0];
    _rgb[2][1] = item._rgb[2][1];
    _rgb[2][2] = item._rgb[2][2];
    _rgb[3][0] = item._rgb[3][0];
    _rgb[3][1] = item._rgb[3][1];
    _rgb[3][2] = item._rgb[3][2];
    _rgb[4][0] = item._rgb[4][0];
    _rgb[4][1] = item._rgb[4][1];
    _rgb[4][2] = item._rgb[4][2];
    _rgb[5][0] = item._rgb[5][0];
    _rgb[5][1] = item._rgb[5][1];
    _rgb[5][2] = item._rgb[5][2];
    _rgb[6][0] = item._rgb[6][0];
    _rgb[6][1] = item._rgb[6][1];
    _rgb[6][2] = item._rgb[6][2];
    _rgb[7][0] = item._rgb[7][0];
    _rgb[7][1] = item._rgb[7][1];
    _rgb[7][2] = item._rgb[7][2];
    _rgb[8][0] = item._rgb[8][0];
    _rgb[8][1] = item._rgb[8][1];
    _rgb[8][2] = item._rgb[8][2];
    _rgb[9][0] = item._rgb[9][0];
    _rgb[9][1] = item._rgb[9][1];
    _rgb[9][2] = item._rgb[9][2];
    _rgb[10][0] = item._rgb[10][0];
    _rgb[10][1] = item._rgb[10][1];
    _rgb[10][2] = item._rgb[10][2];
    _rgb[11][0] = item._rgb[11][0];
    _rgb[11][1] = item._rgb[11][1];
    _rgb[11][2] = item._rgb[11][2];
    _rgb[12][0] = item._rgb[12][0];
    _rgb[12][1] = item._rgb[12][1];
    _rgb[12][2] = item._rgb[12][2];
    _rgb[13][0] = item._rgb[13][0];
    _rgb[13][1] = item._rgb[13][1];
    _rgb[13][2] = item._rgb[13][2];
    _rgb[14][0] = item._rgb[14][0];
    _rgb[14][1] = item._rgb[14][1];
    _rgb[14][2] = item._rgb[14][2];
    _rgb[15][0] = item._rgb[15][0];
    _rgb[15][1] = item._rgb[15][1];
    _rgb[15][2] = item._rgb[15][2];

    _ssh_credfwd = item._ssh_credfwd;    

    strcpy(_proxy_ipaddress,item._proxy_ipaddress);
    strcpy(_proxy_ipport,item._proxy_ipport);
    strcpy(_socks_ipaddress,item._socks_ipaddress);
    strcpy(_socks_ipport,item._socks_ipport);

    strcpy(_proxy_user,item._proxy_user);
    strcpy(_proxy_pass,item._proxy_pass);
    strcpy(_socks_user,item._socks_user);
    strcpy(_socks_pass,item._socks_pass);

    _gui_resize = item._gui_resize;
    _gui_run    = item._gui_run;
}

KD_LIST_ITEM::KD_LIST_ITEM(TRANSPORT tType, 
                            ZIL_ICHAR *name,
                            ZIL_ICHAR *location,
                            ZIL_ICHAR *phoneNum,
                            ZIL_ICHAR *hostname
                          ) :
    UIW_BUTTON( 0,  // Left
		0,  // Top
		0,  // Width
		ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
		BTF_DOUBLE_CLICK | BTF_NO_3D | BTF_SEND_MESSAGE,  // Button flags
		WOF_NO_FLAGS,                       // Window flags
		KDListItemUser,                     // User function
		OPT_CONNECT_TO_ENTRY)               // Send Message Value  
{
    woStatus |= WOS_OWNERDRAW;

    _access = tType;
    strncpy(_name,name,64);
    strncpy(_location,location,64);
    strncpy(_phone_number,phoneNum,256);
    strncpy(_ipaddress, hostname, 256) ;
    strncpy(_lataddress, hostname, 256) ;

    _template = (_access == TEMPLATE) ;
    strcpy(_startup_dir,"\\v(personal)download") ;
    _autoexit = 1 ;
    _notes[0] = '\0' ;
    _terminal = VT100 ;
    _telnet_ttype[0] = '\0';
    _charsize = 8 ;
    _local_echo = 0 ;
    _auto_wrap = 1 ;
    _apc_enabled = 0 ;
    _auto_download = YNA_ASK;
    _status_line = 1;
    _height = 24 ;
    _width = 80 ;
    _scrollback = 512 ;
    _cursor = FULL_CURSOR ;
    _color_term_fg = K_LIGHTGRAY ;
    _color_term_bg = K_BLUE ;
    _color_status_fg = K_LIGHTGRAY ;
    _color_status_bg = K_CYAN ;
    _color_popup_fg = K_LIGHTGRAY ;
    _color_popup_bg = K_CYAN ;
    _color_select_fg = K_BLACK ;
    _color_select_bg = K_YELLOW ;
    _color_underline_fg = K_LIGHTGRAY ;
    _color_underline_bg = K_BLACK ;
    _color_command_fg = K_LIGHTGRAY ;
    _color_command_bg = K_BLACK ;
    _term_charset = T_ASCII ;
    _protocol = K_FAST ;
    _xfermode = BINARY ;
    _collision = COL_BACKUP ;
    _packet_length = 2000 ;
    _max_windows = 20 ;
    _fname_literal = 1 ;
    _pathnames = 0 ;
    _keep_incomplete = 1 ;
    _streaming = 1;
    _clear_channel = 1;
    _unprefix_cc = CAUTIOUSLY ;
    _file_charset = F_CP437 ;
    _xfer_charset = X_LATIN1_ISO ;
    _line_device[0] = '\0' ;
    _tapi_line = 0;
    _speed = 57600 ;
    _flow = AUTO_FLOW ;
    _parity = NO_PARITY ;
    strcpy( _modem, "DEFAULT" ) ;
    _correction = 1 ;
    _compression = 1 ;
    _carrier = 1 ;
    _backspace = DEL ;
    _enter = CR ;
    _mouse = 1 ;
    _keymap = VT100_KEY ;
    _keymap_file[0] = '\0' ;
    _keyscript[0] = '\0';
    _script_file = 0 ;
    _script_fname[0] = '\0' ;
    _userid[0] = '\0' ;
    _password[0] = '\0' ;  
    _prompt[0] = '\0' ;
    _script[0] = '\0';
    _log_debug = 0;
    _log_debug_append = 0;
    strcpy(_log_debug_fname,"\v(appdata)debug.log");
    _log_session = 0;
    _log_session_append = 0;
    strcpy(_log_session_fname,"\v(appdata)session.log");
    _log_packet = 0;
    _log_packet_append = 0;
    strcpy(_log_packet_fname,"\v(appdata)packet.log");
    _log_transaction = 0;
    _log_transaction_append = 0;
    strcpy(_log_transaction_fname,"\v(appdata)transact.log");

    _use_mdm_speed = 1;
    _use_mdm_flow = 1;
    _use_mdm_parity = 1;
    _use_mdm_ec = 1;
    _use_mdm_dc = 1;
    _use_mdm_carrier = 1;

    _converted_1_11 = 0;

    strcpy(_startup_dir,"\\v(personal)download") ;

    _log_transaction_brief = 0;
    _telnet_auth_mode = TelnetAccept;
    _telnet_binary_mode = TelnetAccept;
    _telnet_encrypt_mode = TelnetAccept;
    _telnet_location[0] = '\0';
    _default_ip_address = 1;
    _tcp_ip_address[0] = '\0';
    _tcp_sendbuf = 8192;
    _tcp_recvbuf = 8192;
    _tcp_rdns = AUTO;
    _telnet_acct[0] = '\0';
    _telnet_disp[0] = '\0';
    _telnet_job[0] = '\0';
    _telnet_printer[0] = '\0';
    _k4_realm[0] = '\0';
    _k4_princ[0] = '\0';
    _k5_realm[0] = '\0';
    _k5_princ[0] = '\0';
    _k5_cache[0] = '\0';

    _ipport[0] = '\0';
    _sshport[0] = '\0';
    _ftpport[0] = '\0';
    _stopbits = STOPBITS_1_0;
    _use_mdm_stopbits = 1;
    _telnet_kermit_mode = TelnetAccept;
    _telnet_fwdx_mode = TelnetAccept;
    _telnet_cpc_mode = TelnetAccept;
    _tcp_dns_srv = 0;
    _fwd_cred = 0;
    _telnet_wait = 1;
    _k4_autoget = 0;
    _k4_autodestroy = 0;
    _k4_lifetime = 600;
    _k5_autoget = 0;
    _k5_autodestroy = 0;
    _k5_lifetime = 600;
    _k5_forwardable = 0;
    _k5_getk4tkt = 1;
    _printer_ps = 0;
    _printer_width = 80;
    _printer_length = 66;
    _printer_charset = T_CP437 ;
    _log_connection = 0;
    _log_connection_append = 1;
    strcpy(_log_connection_fname,"connection.log");
    _log_session_type = LOG_BINARY;


    _telnet_starttls_mode = TelnetAccept;
    _tls_cipher[0] = '\0';
    _tls_cert_file[0] = '\0';
    _tls_key_file[0] = '\0';
    _tls_verify_file[0] = '\0';
    _tls_verify_dir[0] = '\0';
    _tls_crl_file[0] = '\0';
    _tls_crl_dir[0] = '\0';
    _tls_verbose = 0;
    _tls_debug = 0;
    _tls_verify_mode = TLS_VERIFY_PEER;
    _telnet_debug = 0;
    _telnet_sb_delay = 1;

    _tcpproto = TCP_DEFAULT;
    _sshproto = SSH_AUTO;

    _ssh_compress = 1;
    _ssh_x11 = 0;
    _ssh1_cipher = SSH1_CIPHER_3DES;
    _ssh_host_check = HC_ASK;
    _ssh2_auth_external_keyx = 0; /* OBSOLETE */
    _ssh2_auth_gssapi = 1;
    _ssh2_auth_hostbased = 0; /* OBSOLETE */
    _ssh2_auth_keyboard_interactive = 1;
    _ssh2_auth_password = 1;
    _ssh2_auth_publickey = 1;
    _ssh2_auth_srp_gex_sha1 = 0; /* OBSOLETE */

    _ssh2_cipher_3des = 1;
    _ssh2_cipher_aes128 = 1;
    _ssh2_cipher_aes192 = 1;
    _ssh2_cipher_aes256 = 1;
    _ssh2_cipher_arcfour = 0; /* OBSOLETE */
    _ssh2_cipher_blowfish = 0; /* OBSOLETE */
    _ssh2_cipher_cast128 = 0; /* OBSOLETE */
    _ssh2_cipher_aes128ctr = 1;
    _ssh2_cipher_aes192ctr = 1;
    _ssh2_cipher_aes256ctr = 1;
    _ssh2_cipher_aes256_gcm_openssh = 1;
    _ssh2_cipher_chachae20_poly1305 = 1;
    _ssh2_cipher_aes128_gcm_openssh = 1;

    _ssh2_mac_md5 = 1;
    _ssh2_mac_md5_96 = 0; /* OBSOLETE */
    _ssh2_mac_ripemd160 = 0; /* OBSOLETE */
    _ssh2_mac_sha1 = 1;
    _ssh2_mac_sha1_96 = 0; /* OBSOLETE */
    _ssh2_mac_none = 0;
    _ssh2_mac_sha2_512 = 1;
    _ssh2_mac_sha2_256 = 1;
    _ssh2_mac_sha1_etm_openssh = 1;
    _ssh2_mac_sha2_512_etm_openssh = 1;
    _ssh2_mac_sha2_256_etm_openssh = 1;

    _ssh2_hka_dss = 0; /* deprecated as of CKWIN */
    _ssh2_hka_rsa = 1;
    _ssh2_hka_ssh_ed25519 = 1;
    _ssh2_hka_ecdsa_sha2_nistp521 = 1;
    _ssh2_hka_ecdsa_sha2_nistp384 = 1;
    _ssh2_hka_ecdsa_sha2_nistp256 = 1;
    _ssh2_hka_rsa_sha2_512 = 1;
    _ssh2_hka_rsa_sha2_256 = 1;

    _ssh2_kex_curve25519_sha256 = 1;
    _ssh2_kex_curve25519_sha256_libssh = 1;
    _ssh2_kex_ecdh_sha2_nistp256 = 1;
    _ssh2_kex_ecdh_sha2_nistp384 = 1;
    _ssh2_kex_ecdh_sha2_nistp521 = 1;
    _ssh2_kex_dh_group18_sha512 = 1;
    _ssh2_kex_dh_group16_sha512 = 1;
    _ssh2_kex_dh_group_exchange_sha256 = 1;
    _ssh2_kex_dh_group14_sha256 = 1;
    _ssh2_kex_dh_group14_sha1 = 1;
    _ssh2_kex_dh_group1_sha1 = 1;
    _ssh2_kex_ext_info_c = 1;
    _ssh2_kex_dh_group_exchange_sha1 = 1;

    _ssh1_id_file[0] = '\0';
    _ssh1_kh_file[0] = '\0';
    _ssh2_id_file[0] = '\0';
    _ssh2_kh_file[0] = '\0';

    _ftp_autologin = 1;  
    _ftp_passive   = 1;    
    _ftp_autoauth  = 1;   
    _ftp_autoenc   = 1;    
    _ftp_credfwd   = 0;    
    _ftp_cpl       = PL_PRIVATE;  
    _ftp_dpl       = PL_PRIVATE;     
    _ftp_debug     = 0;      
    _ftp_dates     = 1;      
    _ftp_literal   = 1;    
    _ftp_verbose   = 1;    
    _ftp_xlat      = 0;       
    _ftp_charset   = F_ASCII;   
    _ftp_auth_gssk5 = 1; 
    _ftp_auth_k4   = 1;    
    _ftp_auth_srp  = 1;   
    _ftp_auth_ssl  = 1;   
    _ftp_auth_tls  = 1;   

    _use_gui = 1;
    strcpy(_facename,"Courier New");
    _fontsize = 12;
    _startpos_auto = 1;
    _startpos_x = 0;
    _startpos_y = 0;
    _rgb[0][0] = 0;
    _rgb[0][1] = 0;
    _rgb[0][2] = 0;
    _rgb[1][0] = 0;
    _rgb[1][1] = 128;
    _rgb[1][2] = 0;
    _rgb[2][0] = 0;
    _rgb[2][1] = 128;
    _rgb[2][2] = 128;
    _rgb[3][0] = 128;
    _rgb[3][1] = 0;
    _rgb[3][2] = 0;
    _rgb[4][0] = 128;
    _rgb[4][1] = 0;
    _rgb[4][2] = 128;
    _rgb[5][0] = 128;
    _rgb[5][1] = 128;
    _rgb[5][2] = 0;
    _rgb[6][0] = 192;
    _rgb[6][1] = 192;
    _rgb[6][2] = 192;
    _rgb[7][0] = 128;
    _rgb[7][1] = 128;
    _rgb[7][2] = 128;
    _rgb[8][0] = 0;
    _rgb[8][1] = 0;
    _rgb[8][2] = 255;
    _rgb[9][0] = 0;
    _rgb[9][1] = 255;
    _rgb[9][2] = 0;
    _rgb[10][0] = 0;
    _rgb[10][1] = 255;
    _rgb[10][2] = 0;
    _rgb[11][0] = 0;
    _rgb[11][1] = 255;
    _rgb[11][2] = 255;
    _rgb[12][0] = 255;
    _rgb[12][1] = 0;
    _rgb[12][2] = 0;
    _rgb[13][0] = 255;
    _rgb[13][1] = 0;
    _rgb[13][2] = 255;
    _rgb[14][0] = 255;
    _rgb[14][1] = 255;
    _rgb[14][2] = 0;
    _rgb[15][0] = 255;
    _rgb[15][1] = 255;
    _rgb[15][2] = 255;

    _ssh_credfwd   = 0;    

    _proxy_ipaddress[0] = '\0';
    _proxy_ipport[0] = '\0';
    _socks_ipaddress[0] = '\0';
    _socks_ipport[0] = '\0';
    _proxy_user[0] = '\0';
    _proxy_pass[0] = '\0';
    _socks_user[0] = '\0';
    _socks_pass[0] = '\0';

    _gui_resize = RES_CHANGE_DIM;
    _gui_run = RUN_RES;
}

KD_LIST_ITEM::~KD_LIST_ITEM()
{
}

EVENT_TYPE KD_LIST_ITEM::DrawItem(const UI_EVENT &, EVENT_TYPE ccode)
{
    UI_PALETTE *palette = LogicalPalette(ccode);

    display->VirtualGet(screenID, trueRegion);

    UI_REGION region = trueRegion;
    display->Rectangle(screenID, trueRegion, palette, 0, TRUE, FALSE, &clip);
    if (display->isText)
	display->Text(screenID, region.left + display->cellWidth,
		       region.top + display->preSpace, (_template ? tImage[TEMPLATE].text : tImage[_access].text),
		       palette,-1, FALSE, FALSE, &clip);
    else
	display->Bitmap(screenID, region.left + display->cellWidth,
			 region.top + display->preSpace, 16, 8, 
			 (_template ? tImage[TEMPLATE].graphic : tImage[_access].graphic),
			 ZIL_NULLP(UI_PALETTE), &clip);
			
    display->Text(screenID, region.left + display->cellWidth * 7,
		   region.top + display->preSpace, _name, palette,
		   -1, FALSE, FALSE, &clip);
    display->Text(screenID, region.left + display->cellWidth * 35,
		   region.top + display->preSpace, _location, palette,
		   -1, FALSE, FALSE, &clip);
    switch ( _access ) {
    case PHONE:
	display->Text(screenID, region.left + display->cellWidth * 55,
		       region.top + display->preSpace, _phone_number, palette,
		       -1, FALSE, FALSE, &clip);
	break; 
    case SSH:
    case FTP:
    case TCPIP:
	display->Text(screenID, region.left + display->cellWidth * 55,
		       region.top + display->preSpace, _ipaddress, palette,
		       -1, FALSE, FALSE, &clip);
	break; 	
    case SUPERLAT:
	display->Text(screenID, region.left + display->cellWidth * 55,
		       region.top + display->preSpace, _lataddress, palette,
		       -1, FALSE, FALSE, &clip);
	break; 
    }
#ifdef _COMMENT
   // an example of right justification
	int left = region.right - display->TextWidth(_location, screenID, font) - display->cellWidth;
	if (left < region.left + display->cellWidth * 30 - display->TextWidth(_location, screenID, font))
		left = region.left + display->cellWidth * 30 - display->TextWidth(_location, screenID, font);
	display->Text(screenID, left, region.top + display->preSpace, _location,
		palette, -1, FALSE, FALSE, &clip);
#endif

    UI_WINDOW_OBJECT *object = this;
	for(; object && FlagSet(object->woStatus, WOS_CURRENT);
             object = object->parent)
            ;
	if (!object)
            DrawFocus((ZIL_SCREENID)screenID, region, ccode);

	display->VirtualPut(screenID);
	return (TRUE);

}

int 
KD_LIST_ITEM::Compare(void *p1, void *p2)
{
   if ( !p1 )
      return -1 ;
   if ( !p2 )
      return -2 ;
	KD_LIST_ITEM *object1 = (KD_LIST_ITEM *)p1;
	KD_LIST_ITEM *object2 = (KD_LIST_ITEM *)p2;

   return stricmp(object1->_name, object2->_name) ;
}


void 
KD_LIST_ITEM::ConvertModemInfo( void )
{
    K_MODEM * modem = NULL ;
    MDMINF *  mdminf = NULL;


    if ( !_template &&
	 ( connector->FindModem( _modem ) ||
	  _access != PHONE ))
	return;	/* valid modem && not template */

    if ( !strnicmp("COM",_line_device,3) &&
	 atoi(&_line_device[3]) > 0 ) {
	/* we have found a COM port */
	ZIL_ICHAR newname[61];
	sprintf(newname, "%s (%s)",_modem, _line_device);
	if ( modem = connector->FindModem( newname ) ) {
	    strcpy( _modem, newname );
	    strcpy( _line_device, modem->_port );
	}
	else {
	    modem = new K_MODEM();
	    strcpy( modem->_name, newname );
	    strcpy( modem->_type, _modem ) ;
	    strcpy( modem->_port, _line_device );
	    modem->_speed = _speed;
	    modem->_flow = _flow;
	    modem->_parity = _parity;
            modem->_stopbits = _stopbits;
	    modem->_correction = _correction;
	    modem->_compression = _compression ;
	    modem->_carrier = _carrier;

	    mdminf = FindMdmInf( modem->_type );
	    CopyMdmInfToModem( mdminf, modem );

	    strcpy( _modem, newname );
	    connector->_modem_list + modem ;

	    if (K_CONNECTOR::_userFile->ChDir("~Modems") == 0) {
                ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
					 modem->_name, ID_K_MODEM,
					 UIS_OPENCREATE | UIS_READWRITE ) ;
                modem->Store( modem->_name, K_CONNECTOR::_userFile, &_fileObj,   
                              ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
                if ( K_CONNECTOR::_userFile->storageError )
                {
                    // What went wrong?
                    ZAF_MESSAGE_WINDOW * message =
		    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					    "Unable to Write modem \"%s\": errorno %d", 
					    modem->_name,
					    K_CONNECTOR::_userFile->storageError ) ;
                    message->Control() ;	
                    delete message ;
                }   
                K_CONNECTOR::_userFile->ChDir("~");
            }
	}

	if ( _template &&
		!connector->FindModem(_name)) {
	    K_MODEM * newmodem = new K_MODEM( *modem );
	    strcpy(newmodem->_name,_name);
	    connector->_modem_list + newmodem;

	    if (K_CONNECTOR::_userFile->ChDir("~Modems") == 0) {
                ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
					 newmodem->_name, ID_K_MODEM,
					 UIS_OPENCREATE | UIS_READWRITE ) ;
                newmodem->Store( newmodem->_name, K_CONNECTOR::_userFile, &_fileObj,   
			 ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
                if ( K_CONNECTOR::_userFile->storageError )
                {
                    // What went wrong?
                    ZAF_MESSAGE_WINDOW * message =
                        new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					    "Unable to Write modem \"%s\": errorno %d", 
					    newmodem->_name,
					    K_CONNECTOR::_userFile->storageError ) ;
                    message->Control() ;	
                    delete message ;
                }   
                K_CONNECTOR::_userFile->ChDir("~");
            }
	}
    }
    else {
	/* this must be a template entry */
	/* we assume that the template exists and that */
	/* it will create its own entry */
	strcpy(_modem,_line_device);
    }
    _converted_1_11 = 1;
}

TRANSPORT_IMAGE KD_LIST_ITEM::tImage[7] =
{
#define _ (ZIL_UINT8)BACKGROUND
#define B (ZIL_UINT8)BLACK
	{ "Phone",
		{	_, _, _, B, B, B, B, B, B, B, B, B, B, _, _, _,
			_, _, B, B, B, B, B, B, B, B, B, B, B, B, _, _,
			_, B, B, B, _, B, _, _, _, _, B, _, B, B, B, _,
			_, B, B, B, _, B, _, B, B, _, B, _, B, B, B, _,
			_, _, _, _, B, B, B, B, B, B, B, B, _, _, _, _,
			_, _, _, B, B, B, B, B, B, B, B, B, B, _, _, _,
			_, _, B, B, B, B, B, B, B, B, B, B, B, B, _, _,
			_, _, B, B, B, B, B, B, B, B, B, B, B, B, _, _	}	},
{ "TCP/IP",
		{	_, _, _, _, _, _, B, B, _, _, _, _, _, _, _, _,
			B, _, _, _, _, _, _, B, B, B, _, _, _, _, _, _,
			_, B, B, _, _, _, _, _, B, B, B, B, _, _, _, _,
			_, B, B, B, B, B, B, B, B, B, B, B, B, B, B, _,
			_, B, B, _, _, _, _, _, B, B, B, B, _, _, _, _,
			B, _, _, _, _, _, _, B, B, B, _, _, _, _, _, _,
			_, _, _, _, _, _, B, B, _, _, _, _, _, _, _, _,
			_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _	}	},
#ifndef COMMENT
{ "Superlat",
		{	_, _, _, _, _, _, B, B, _, _, _, _, _, _, _, _,
			B, _, _, _, _, _, _, B, B, B, _, _, _, _, _, _,
			_, B, B, _, _, _, _, _, B, B, B, B, _, _, _, _,
			_, B, B, B, B, B, B, B, B, B, B, B, B, B, B, _,
			_, B, B, _, _, _, _, _, B, B, B, B, _, _, _, _,
			B, _, _, _, _, _, _, B, B, B, _, _, _, _, _, _,
			_, _, _, _, _, _, B, B, _, _, _, _, _, _, _, _,
			_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _	}	},
#else /* COMMENT */
	{ "Superlat",
		{	_, _, _, _, _, _, _, _, _, _, _, _, B, B, B, B,
			_, _, _, _, _, _, _, _, _, _, _, _, B, B, B, B,
			_, _, _, _, _, _, B, _, _, _, _, _, B, B, B, B,
			_, _, _, _, _, B, B, B, _, _, _, B, B, B, B, B,
			B, B, B, B, B, B, _, B, B, _, B, B, _, _, _, _,
			B, B, B, B, B, _, _, _, B, B, B, _, _, _, _, _,
			B, B, B, B, _, _, _, _, _, B, _, _, _, _, _, _,
			B, B, B, B, _, _, _, _, _, _, _, _, _, _, _, _	}	},
#endif /* COMMENT */
	{ "Direct",
		{	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
			_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
			B, B, B, B, _, _, _, _, _, _, _, _, B, B, B, B,
			B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B,
			B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B,
			B, B, B, B, _, _, _, _, _, _, _, _, B, B, B, B,
			_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
			_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _	}	},
	{ "Template",
		{	_, _, _, _, B, B, B, B, B, B, B, B, _, _, _, _,
			_, _, _, _, B, _, _, _, _, _, _, B, _, _, _, _,
			_, _, _, _, B, _, B, B, B, B, _, B, _, _, _, _,
			_, _, _, _, B, _, _, _, _, _, _, B, _, _, _, _,
			_, _, _, _, B, _, B, B, B, B, _, B, _, _, _, _,
			_, _, _, _, B, _, _, _, _, _, _, B, _, _, _, _,
			_, _, _, _, B, _, B, B, B, B, _, B, _, _, _, _,
			_, _, _, _, B, B, B, B, B, B, B, B, _, _, _, _	}	},
        { "SSH",
          {    
              _, B, B, B, _,_, B, B, B, _, B, _, _, _, B, _,
              B, _, _, _, B,B, _, _, _, B, B, _, _, _, B, _,
              B, _, _, _, _,B, _, _, _, _, B, _, _, _, B, _,
              _, B, B, _, _,_, B, B, _, _, B, _, _, _, B, _,
              _, _, B, B, _,_, _, B, B, _, B, B, B, B, B, _,
              B, _, _, B, B,B, _, _, B, B, B, _, _, _, B, _,
              B, _, _, _, B,B, _, _, _, B, B, _, _, _, B, _,
              _, B, B, B, _,_, B, B, B, _, B, _, _, _, B, _	
          }	
        },
        { "FTP",
          {    
              B, B, B, B, _,B, B, B, B, B, _, B, B, B, _, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, B, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, B, _,
              B, B, B, _, _,_, _, B, _, _, _, B, B, B, _, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, _, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, _, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, _, _,
              B, _, _, _, _,_, _, B, _, _, _, B, _, _, _, _	
          }	
        }

#undef B
#undef _
};

// ----- ZIL_PERSISTENCE ----------------------------------------------------

#if defined(ZIL_LOAD)
void
KD_LIST_ITEM::DataLoss(const ZIL_ICHAR * name)
{
    ZAF_MESSAGE_WINDOW * message =
        new ZAF_MESSAGE_WINDOW( "Data Corruption Error", 
                                "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                "Data corruption detected while loading \"%s\"",
                                name);
    if ( message ) {
        message->Control() ;	
        delete message ;
    }
    
}

KD_LIST_ITEM::KD_LIST_ITEM(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
	ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable) :
	UIW_BUTTON( 0,  // Left
               0,  // Top
               0,  // Width
               ZIL_NULLP(ZIL_ICHAR), // no text, ownerdraw
               BTF_DOUBLE_CLICK | BTF_NO_3D | BTF_SEND_MESSAGE,  // Button flags
				   WOF_NO_FLAGS,                                 // Window flags
               KDListItemUser,                     // User function
               OPT_CONNECT_TO_ENTRY)                             // Send Message Value  
{
    woStatus |= WOS_OWNERDRAW;
    _name[0] = '\0'  ;
    _location[0] = '\0' ;
    _access = NO_TRANSPORT ;
    _phone_number[0] = '\0' ;
    _ipaddress[0] = '\0' ;
    _lataddress[0] = '\0' ;
    _template = 0 ;
    strcpy(_startup_dir,"\\v(personal)download") ;
    _autoexit = 1 ;
    _notes[0] = '\0' ;
    _terminal = VT100 ;
    _telnet_ttype[0] = '\0';
    _charsize = 8 ;
    _local_echo = 0 ;
    _auto_wrap = 1 ;
    _apc_enabled = 0 ;
    _auto_download = YNA_ASK;
    _status_line = 1 ;
    _height = 24 ;
    _width = 80 ;
    _scrollback = 512 ;
    _cursor = FULL_CURSOR ;
    _color_term_fg = K_LIGHTGRAY ;
    _color_term_bg = K_BLUE ;
    _color_status_fg = K_LIGHTGRAY ;
    _color_status_bg = K_CYAN ;
    _color_popup_fg = K_LIGHTGRAY ;
    _color_popup_bg = K_CYAN ;
    _color_select_fg = K_BLACK ;
    _color_select_bg = K_YELLOW ;
    _color_underline_fg = K_LIGHTGRAY ;
    _color_underline_bg = K_BLACK ;
    _color_command_fg = K_LIGHTGRAY ;
    _color_command_bg = K_BLACK ;
    _term_charset = T_ASCII ;
    _protocol = K_FAST ;
    _xfermode = BINARY ;
    _collision = COL_BACKUP ;
    _packet_length = 2000 ;
    _max_windows = 20 ;
    _fname_literal = 1 ;
    _pathnames = 0 ;
    _keep_incomplete = 1 ;
    _streaming = 1;
    _clear_channel = 1;
    _unprefix_cc = CAUTIOUSLY ;
    _file_charset = F_CP437 ;
    _xfer_charset = X_LATIN1_ISO ;
    _line_device[0] = '\0' ;
    _tapi_line = 0;
    _speed = 57600 ;
    _flow = AUTO_FLOW ;
    _parity = NO_PARITY ;
    strcpy( _modem, "DEFAULT" ) ;
    _correction = 1 ;
    _compression = 1 ;
    _carrier = 1 ;
    _backspace = DEL ;
    _enter = CR ;
    _mouse = 1 ;
    _keymap = VT100_KEY ;
    _keymap_file[0] = '\0' ;
    _keyscript[0] = '\0';
    _script_file = 0 ;
    _script_fname[0] = '\0' ;
    _userid[0] = '\0' ;
    _password[0] = '\0' ;  
    _prompt[0] = '\0' ;
    _script[0] = '\0';
    _log_debug = 0;
    _log_debug_append = 0;
    strcpy(_log_debug_fname,"\v(appdata)debug.log");
    _log_session = 0;
    _log_session_append = 0;
    strcpy(_log_session_fname,"\v(appdata)session.log");
    _log_packet = 0;
    _log_packet_append = 0;
    strcpy(_log_packet_fname,"\v(appdata)packet.log");
    _log_transaction = 0;
    _log_transaction_append = 0;
    strcpy(_log_transaction_fname,"\v(appdata)transact.log");

    _use_mdm_speed = 1;
    _use_mdm_flow = 1;
    _use_mdm_parity = 1;
    _use_mdm_ec = 1;
    _use_mdm_dc = 1;
    _use_mdm_carrier = 1;

    _converted_1_11 = 0;

    _printer_type = PrinterDOS;
    _printer_dos[0] = '\0' ;
    _printer_windows[0] = '\0' ;
    _printer_separator[0] = '\0';
    _printer_formfeed = 0;       
    _printer_terminator[0] = '\0';
    _printer_timeout = 0 ;       
    _printer_bidi=0;           
    _printer_speed=9600;         
    _printer_flow = NO_FLOW ;          
    _printer_parity = NO_PARITY;

    _prompt_for_password = 0;

    _log_transaction_brief = 0;
    _telnet_auth_mode = TelnetAccept;
    _telnet_binary_mode = TelnetAccept;
    _telnet_encrypt_mode = TelnetAccept;
    _telnet_location[0] = '\0';
    _default_ip_address = 1;
    _tcp_ip_address[0] = '\0';
    _tcp_sendbuf = 8192;
    _tcp_recvbuf = 8192;
    _tcp_rdns = AUTO;
    _telnet_acct[0] = '\0';
    _telnet_disp[0] = '\0';
    _telnet_job[0] = '\0';
    _telnet_printer[0] = '\0';
    _k4_realm[0] = '\0';
    _k4_princ[0] = '\0';
    _k5_realm[0] = '\0';
    _k5_princ[0] = '\0';
    _k5_cache[0] = '\0';

    _ipport[0] = '\0';
    _sshport[0] = '\0';
    _ftpport[0] = '\0';
    _stopbits = STOPBITS_1_0;
    _use_mdm_stopbits = 1;
    _telnet_kermit_mode = TelnetAccept;
    _telnet_fwdx_mode = TelnetAccept;
    _telnet_cpc_mode = TelnetAccept;
    _tcp_dns_srv = 0;
    _fwd_cred = 0;
    _telnet_wait = 1;
    _k4_autoget = 0;
    _k4_autodestroy = 0;
    _k4_lifetime = 600;
    _k5_autoget = 0;
    _k5_autodestroy = 0;
    _k5_lifetime = 600;
    _k5_forwardable = 0;
    _k5_getk4tkt = 1;
    _printer_ps = 0;
    _printer_width = 80;
    _printer_length = 66;
    _printer_charset = T_CP437 ;
    _log_connection = 0;
    _log_connection_append = 1;
    strcpy(_log_connection_fname,"connection.log");
    _log_session_type = LOG_BINARY;

    _telnet_starttls_mode = TelnetAccept;
    _tls_cipher[0] = '\0';
    _tls_cert_file[0] = '\0';
    _tls_key_file[0] = '\0';
    _tls_verify_file[0] = '\0';
    _tls_verify_dir[0] = '\0';
    _tls_crl_file[0] = '\0';
    _tls_crl_dir[0] = '\0';
    _tls_verbose = 0;
    _tls_debug = 0;
    _tls_verify_mode = TLS_VERIFY_PEER;
    _telnet_debug = 0;
    _telnet_sb_delay = 1;

    _tcpproto = TCP_DEFAULT;
    _sshproto = SSH_AUTO;
    _converted_1_11 = 0;

    _ssh_compress = 1;
    _ssh_x11 = 0;
    _ssh1_cipher = SSH1_CIPHER_3DES;
    _ssh_host_check = HC_ASK;

    _ssh2_auth_external_keyx = 0; /* OBSOLETE */
    _ssh2_auth_gssapi = 1;
    _ssh2_auth_hostbased = 0; /* OBSOLETE */
    _ssh2_auth_keyboard_interactive = 1;
    _ssh2_auth_password = 1;
    _ssh2_auth_publickey = 1;
    _ssh2_auth_srp_gex_sha1 = 0; /* OBSOLETE */

    _ssh2_cipher_3des = 1;
    _ssh2_cipher_aes128 = 1;
    _ssh2_cipher_aes192 = 1;
    _ssh2_cipher_aes256 = 1;
    _ssh2_cipher_arcfour = 0; /* OBSOLETE */
    _ssh2_cipher_blowfish = 0; /* OBSOLETE */
    _ssh2_cipher_cast128 = 0; /* OBSOLETE */
    _ssh2_cipher_aes128ctr = 1;
    _ssh2_cipher_aes192ctr = 1;
    _ssh2_cipher_aes256ctr = 1;
    _ssh2_cipher_aes256_gcm_openssh = 1;
    _ssh2_cipher_chachae20_poly1305 = 1;
    _ssh2_cipher_aes128_gcm_openssh = 1;

    _ssh2_mac_md5 = 1;
    _ssh2_mac_md5_96 = 0; /* OBSOLETE */
    _ssh2_mac_ripemd160 = 0; /* OBSOLETE */
    _ssh2_mac_sha1 = 1;
    _ssh2_mac_sha1_96 = 0; /* OBSOLETE */
    _ssh2_mac_none = 0;
    _ssh2_mac_sha2_512 = 1;
    _ssh2_mac_sha2_256 = 1;
    _ssh2_mac_sha1_etm_openssh = 1;
    _ssh2_mac_sha2_512_etm_openssh = 1;
    _ssh2_mac_sha2_256_etm_openssh = 1;

    _ssh2_hka_dss = 0; /* deprecated as of CKWIN */
    _ssh2_hka_rsa = 1;
    _ssh2_hka_ssh_ed25519 = 1;
    _ssh2_hka_ecdsa_sha2_nistp521 = 1;
    _ssh2_hka_ecdsa_sha2_nistp384 = 1;
    _ssh2_hka_ecdsa_sha2_nistp256 = 1;
    _ssh2_hka_rsa_sha2_512 = 1;
    _ssh2_hka_rsa_sha2_256 = 1;

    _ssh2_kex_curve25519_sha256 = 1;
    _ssh2_kex_curve25519_sha256_libssh = 1;
    _ssh2_kex_ecdh_sha2_nistp256 = 1;
    _ssh2_kex_ecdh_sha2_nistp384 = 1;
    _ssh2_kex_ecdh_sha2_nistp521 = 1;
    _ssh2_kex_dh_group18_sha512 = 1;
    _ssh2_kex_dh_group16_sha512 = 1;
    _ssh2_kex_dh_group_exchange_sha256 = 1;
    _ssh2_kex_dh_group14_sha256 = 1;
    _ssh2_kex_dh_group14_sha1 = 1;
    _ssh2_kex_dh_group1_sha1 = 1;
    _ssh2_kex_ext_info_c = 1;
    _ssh2_kex_dh_group_exchange_sha1 = 1;

    _ssh1_id_file[0] = '\0';
    _ssh1_kh_file[0] = '\0';
    _ssh2_id_file[0] = '\0';
    _ssh2_kh_file[0] = '\0';

    _ftp_autologin = 1;  
    _ftp_passive   = 1;    
    _ftp_autoauth  = 1;   
    _ftp_autoenc   = 1;    
    _ftp_credfwd   = 0;    
    _ftp_cpl       = PL_PRIVATE;  
    _ftp_dpl       = PL_PRIVATE;     
    _ftp_debug     = 0;      
    _ftp_dates     = 1;      
    _ftp_literal   = 1;    
    _ftp_verbose   = 1;    
    _ftp_xlat      = 0;       
    _ftp_charset   = F_ASCII;   
    _ftp_auth_gssk5 = 1; 
    _ftp_auth_k4   = 1;    
    _ftp_auth_srp  = 1;   
    _ftp_auth_ssl  = 1;   
    _ftp_auth_tls  = 1;   

    _use_gui = 1;
    strcpy(_facename,"Courier New");
    _fontsize = 12;
    _startpos_auto = 1;
    _startpos_x = 0;
    _startpos_y = 0;
    _rgb[0][0] = 0;
    _rgb[0][1] = 0;
    _rgb[0][2] = 0;
    _rgb[1][0] = 0;
    _rgb[1][1] = 127;
    _rgb[1][2] = 0;
    _rgb[2][0] = 0;
    _rgb[2][1] = 127;
    _rgb[2][2] = 127;
    _rgb[3][0] = 127;
    _rgb[3][1] = 0;
    _rgb[3][2] = 0;
    _rgb[4][0] = 127;
    _rgb[4][1] = 0;
    _rgb[4][2] = 127;
    _rgb[5][0] = 127;
    _rgb[5][1] = 127;
    _rgb[5][2] = 0;
    _rgb[6][0] = 192;
    _rgb[6][1] = 192;
    _rgb[6][2] = 192;
    _rgb[7][0] = 127;
    _rgb[7][1] = 127;
    _rgb[7][2] = 127;
    _rgb[8][0] = 0;
    _rgb[8][1] = 0;
    _rgb[8][2] = 255;
    _rgb[9][0] = 0;
    _rgb[9][1] = 255;
    _rgb[9][2] = 0;
    _rgb[10][0] = 0;
    _rgb[10][1] = 255;
    _rgb[10][2] = 0;
    _rgb[11][0] = 0;
    _rgb[11][1] = 255;
    _rgb[11][2] = 255;
    _rgb[12][0] = 255;
    _rgb[12][1] = 0;
    _rgb[12][2] = 0;
    _rgb[13][0] = 255;
    _rgb[13][1] = 0;
    _rgb[13][2] = 255;
    _rgb[14][0] = 255;
    _rgb[14][1] = 255;
    _rgb[14][2] = 0;
    _rgb[15][0] = 255;
    _rgb[15][1] = 255;
    _rgb[15][2] = 255;

    _ssh_credfwd   = 0;    

    _proxy_ipaddress[0] = '\0';
    _proxy_ipport[0] = '\0';
    _socks_ipaddress[0] = '\0';
    _socks_ipport[0] = '\0';
    _proxy_user[0] = '\0';
    _proxy_pass[0] = '\0';
    _socks_user[0] = '\0';
    _socks_pass[0] = '\0';

    _gui_resize = RES_CHANGE_DIM;
    _gui_run = RUN_RES;

    // Initialize the information.
    KD_LIST_ITEM::Load(name, directory, file, objectTable, userTable);
    UI_WINDOW_OBJECT::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::Information(I_INITIALIZE_CLASS, ZIL_NULLP(void));
    UIW_BUTTON::DataSet(text);  // This sets the button string which should be empty
}

void KD_LIST_ITEM::Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
	ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable)
{
    // Load the button information.
    UIW_BUTTON::Load(name, directory, file, objectTable, userTable);
    ZIL_UINT16 length ;
    ZIL_UINT16 Enum ;
    ZIL_UINT8 itemMajor, itemMinor ;

    _converted_1_11 = 0;

   file->Load(&itemMajor);
   file->Load(&itemMinor);

   if ( itemMajor < 1 )
      return;

   file->Load(&length) ;
   file->Load(_name, length) ;
   _name[64] = '\0' ;

   file->Load(&length) ;
    file->Load(_location,length) ;
   _location[64] = '\0' ;

   file->Load(&Enum) ;
   _access = (enum TRANSPORT) Enum ;

   file->Load(&length) ;
   file->Load(_phone_number,length) ;
   _phone_number[256] = '\0' ; 

   file->Load(&length) ;
   file->Load(_ipaddress,length) ;
   _ipaddress[256] = '\0' ;

   file->Load(&_template) ;
   
   file->Load(&length) ;
   file->Load(_startup_dir,length) ;
   _startup_dir[256] = '\0' ;

   file->Load(&length) ;
   file->Load(_notes,length) ;
   _notes[3000] = '\0' ;

   file->Load(&Enum) ;
   _terminal = (enum TERMINAL) Enum ;

   file->Load(&_charsize) ;
   file->Load(&_local_echo) ;
   file->Load(&_auto_wrap) ;
   file->Load(&_apc_enabled) ;
   file->Load(&_height) ;
   file->Load(&_width) ;
   file->Load(&_scrollback) ;

   DWORD color ;
   file->Load(&color) ;     
   _color_term_fg      = color;
   file->Load(&color) ;      
   _color_term_bg      = color;
   file->Load(&color) ;      
   _color_status_fg    = color;
   file->Load(&color) ;      
   _color_status_bg    = color;
   file->Load(&color) ;      
   _color_popup_fg     = color;
   file->Load(&color) ;      
   _color_popup_bg     = color;
   file->Load(&color) ;      
   _color_select_fg    = color;
   file->Load(&color) ;      
   _color_select_bg    = color;
   file->Load(&color) ;      
   _color_underline_fg = color;
   file->Load(&color) ;      
   _color_underline_bg = color;

   file->Load(&Enum) ;
   _term_charset = (enum TERMCSET) Enum ;

   file->Load(&Enum) ;
   _protocol = (enum PROTOCOL) Enum ;

   file->Load(&Enum) ;
   _xfermode = (enum XFERMODE) Enum ;

   file->Load(&Enum) ;
   _collision = (enum COLLISION) Enum ;

   file->Load(&_packet_length) ;
   file->Load(&_max_windows) ;
   file->Load(&_fname_literal) ;
   file->Load(&_pathnames) ;
   file->Load(&_keep_incomplete) ;

   file->Load(&Enum) ;
   _unprefix_cc = (enum UNPREFIX_CC) Enum ;

   file->Load(&Enum) ; 
   _file_charset = (enum FILECSET) Enum ;

   file->Load(&Enum) ;
   _xfer_charset = (enum XFERCSET) Enum ;

   file->Load(&length) ;
   file->Load(_line_device,length) ;
   _line_device[60] = '\0' ;

   file->Load(&_speed) ;

   file->Load(&Enum) ;
   _flow = (enum FLOW) Enum ;

   file->Load(&Enum) ;
   _parity = (enum PARITY) Enum ;

   file->Load(&length) ;
   file->Load(_modem,length) ;
   _modem[60] = '\0';

   /* Modem names that have changed have to be altered */
    if ( !strcmp( "supra", _modem ) )
	strcpy( _modem, "suprafaxmodem" );
    else if ( !strcmp( "ccitt-v25bis", _modem ) )
	strcpy( _modem, "itu-t-v25bis" );
    else if ( !strcmp( "usr-212a", _modem ) )
	strcpy( _modem, "hayes-2400" );
    else if ( !strcmp( "usr-courier", _modem ) )
	strcpy( _modem, "usrobotics" );
    else if ( !strcmp( "usr-sportster", _modem ) )
	strcpy( _modem, "usrobotics" );

   file->Load(&_correction) ;
   file->Load(&_compression) ;
   file->Load(&_carrier) ;

   file->Load(&Enum) ;
   _backspace = (enum BACKSPACE) Enum ;

   file->Load(&Enum) ;
   _enter = (enum ENTER) Enum ;

   file->Load(&_mouse) ;

   file->Load(&Enum) ;
   _keymap = (enum KEYMAP) Enum ;

   file->Load(&length) ;
   file->Load(_keymap_file,length) ;
   _keymap_file[256] = '\0' ;

   file->Load(&_script_file) ;

   file->Load(&length) ;
   file->Load(_script_fname,length) ;
   _script_fname[256] = '\0' ;

   file->Load(&length) ;
   file->Load(_userid,length) ;
   _userid[20] = '\0';

   file->Load(&length) ;
   file->Load(_password,length) ;
   _password[20] = '\0';

   file->Load(&length) ;
   file->Load(_script,length) ;
   _script[3000] = '\0' ;

  ver_1_1:
   /* New items in version 1.1 */
   if ( itemMinor < 1 )
   {
      _auto_download = YNA_NO;
      strcpy( _prompt, "$" ) ;
      _color_command_fg = K_LIGHTGRAY ;
      _color_command_bg = K_BLACK ;
   }
   else 
   {
       ZIL_UINT8 enum8;
       file->Load(&enum8);
       if (itemMinor < 35) { 
           if (enum8 == 0)
               _auto_download = YNA_NO;
           else
               _auto_download = YNA_YES;
       } else {
           _auto_download = (enum YNA) enum8;
       }
       file->Load(&length);
       if ( length > 20 ) {
           itemMinor = 0;
           DataLoss(name);
           goto ver_1_1;
       }
       file->Load(_prompt, length) ;
       _prompt[20] = '\0' ;

       file->Load(&color) ;      
       _color_command_fg = color;
       file->Load(&color) ;      
       _color_command_bg = color;
   }

  ver_1_2:
    /* New items in version 1.2 */
    if ( itemMinor < 2 )
    {
        _cursor = FULL_CURSOR ;
    }
    else 
    {
        file->Load(&Enum);
        _cursor = (enum CURSOR_TYPE) Enum ;
    }

  ver_1_3:
    /* New items in version 1.3 */
    if ( itemMinor < 3 )
    {
    	_lataddress[0] = '\0' ;
    }
    else {
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 2;
            DataLoss(name);
            goto ver_1_3;
        }
	file->Load(_lataddress,length) ;
	_lataddress[256] = '\0' ;
    }	       

  ver_1_4:
    /* New items in version 1.1.7 (minor = 4 ) */
    if ( itemMinor < 4 )
    {
    	_autoexit = 0 ;
	switch ( _terminal ) {
	case SCOANSI:
	case AT386:
	    _status_line = 0 ;
	    break;
	default:
	    _status_line = 1 ;
	}
    }
    else
    {
    	file->Load(&_autoexit);
	file->Load(&_status_line);
    }

  ver_1_7:
    /* New items in version 1.1.12 (minor = 7 ) */
    if ( itemMinor < 7 )
    {
	_telnet_ttype[0] = '\0';
	_tapi_line = 0;
    }
    else
    {
	file->Load(&length) ;
        if ( length > 40 ) {
            itemMinor = 6;
            DataLoss(name);
            goto ver_1_7;
        }
	file->Load(_telnet_ttype,length) ;
	_telnet_ttype[40] = '\0' ;
	file->Load(&_tapi_line);
    }

  ver_1_8:
    /* New items in version 1.1.12 (minor = 9 ) */
    if ( itemMinor < 8 )
    {
	_log_debug = 0;
	strcpy(_log_debug_fname,"\v(appdata)debug.log");
	_log_session = 0;
	strcpy(_log_session_fname,"\v(appdata)session.log");
	_log_packet = 0;
	strcpy(_log_packet_fname,"\v(appdata)packet.log");
	_log_transaction = 0;
	strcpy(_log_transaction_fname,"\v(appdata)transact.log");
    }
    else
    {
	file->Load(&_log_debug);
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 7;
            DataLoss(name);
            goto ver_1_8;
        }
	file->Load(_log_debug_fname,length) ;
	_log_debug_fname[256] = '\0' ;
	file->Load(&_log_session);
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 7;
            DataLoss(name);
            goto ver_1_8;
        }
	file->Load(_log_session_fname,length) ;
	_log_session_fname[256] = '\0' ;
	file->Load(&_log_packet);
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 7;
            DataLoss(name);
            goto ver_1_8;
        }
	file->Load(_log_packet_fname,length) ;
	_log_packet_fname[256] = '\0' ;
	file->Load(&_log_transaction);
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 7;
            DataLoss(name);
            goto ver_1_8;
        }
	file->Load(_log_transaction_fname,length) ;
	_log_transaction_fname[256] = '\0' ;
    }

  ver_1_10:
    /* New items in version 1.1.12 (minor = 10 ) */
    if ( itemMinor < 10 )
    {
	_log_debug_append = 0;
	_log_session_append = 0;
	_log_packet_append = 0;
	_log_transaction_append = 0;
    }
    else
    {
	file->Load(&_log_debug_append);
	file->Load(&_log_session_append);
	file->Load(&_log_packet_append);
	file->Load(&_log_transaction_append);
    }

  ver_1_11:
    if ( itemMinor < 11 )
    {
	_use_mdm_speed = 0;
	_use_mdm_flow = 0;
	_use_mdm_parity = 0;
	_use_mdm_ec = 0;
	_use_mdm_dc = 0;
	_use_mdm_carrier = 0;

	/* Need to convert modem info to new format */
	ConvertModemInfo();
    }
    else 
    {
	file->Load(&_use_mdm_speed);
	file->Load(&_use_mdm_flow);
	file->Load(&_use_mdm_parity);
	file->Load(&_use_mdm_ec);
	file->Load(&_use_mdm_dc);
	file->Load(&_use_mdm_carrier);
    }

  ver_1_12:
    if ( itemMinor < 12 ) {
	_printer_dos[0] = '\0';
    }
    else {
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 11;
            DataLoss(name);
            goto ver_1_12;
        }
	file->Load(_printer_dos,length) ;
	_printer_dos[256] = '\0' ;
    }

    ver_1_15:
    if ( itemMinor < 15 ) {
        _printer_type = PrinterDOS;
        _printer_windows[0] = '\0' ;
        _printer_separator[0] = '\0';
        _printer_formfeed = 0;       
        _printer_terminator[0] = '\0';
        _printer_timeout = 0 ;       
        _printer_bidi=0;           
        _printer_speed=9600;         
        _printer_flow = NO_FLOW ;          
        _printer_parity = NO_PARITY;

        _prompt_for_password = 0;
    }
    else {
        file->Load(&Enum) ;
        _printer_type = (enum PRINTER_TYPE) Enum ;

	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 14;
            DataLoss(name);
            goto ver_1_15;
        }
	file->Load(_printer_windows,length) ;
        _printer_windows[256] = '\0' ;
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 14;
            DataLoss(name);
            goto ver_1_15;
        }
	file->Load(_printer_separator,length) ;
        _printer_separator[256] = '\0';
        file->Load(&_printer_formfeed);       
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 14;
            DataLoss(name);
            goto ver_1_15;
        }
	file->Load(_printer_terminator,length) ;
        _printer_terminator[64] = '\0';
        file->Load(&_printer_timeout);       
        file->Load(&_printer_bidi);
        
        file->Load(&_printer_speed) ;

        file->Load(&Enum) ;
        _printer_flow = (enum FLOW) Enum ;

        file->Load(&Enum) ;
        _printer_parity = (enum PARITY) Enum ;

        file->Load(&_prompt_for_password);
    }

    ver_1_17:
    if ( itemMinor < 17 ) {
        _log_transaction_brief = 0;
        _telnet_auth_mode = TelnetAccept;
        _telnet_binary_mode = TelnetAccept;
        _telnet_encrypt_mode = TelnetAccept;
        _telnet_location[0] = '\0';
        _tcp_ip_address[0] = '\0';
        _tcp_sendbuf = 8192;
        _tcp_recvbuf = 8192;
        _tcp_rdns = AUTO;
        _telnet_acct[0] = '\0';
        _telnet_disp[0] = '\0';
        _telnet_job[0] = '\0';
        _telnet_printer[0] = '\0';
        _k4_realm[0] = '\0';
        _k4_princ[0] = '\0';
        _k5_realm[0] = '\0';
        _k5_princ[0] = '\0';
        _k5_cache[0] = '\0';
    } else {
        file->Load(&_log_transaction_brief);
        file->Load(&Enum) ;
        _telnet_auth_mode = (enum TELNET_MODE)Enum;
        file->Load(&Enum) ;
        _telnet_binary_mode = (enum TELNET_MODE)Enum;
        file->Load(&Enum) ;
        _telnet_encrypt_mode = (enum TELNET_MODE)Enum;
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_telnet_location,length);
        _telnet_location[64] = '\0';

        file->Load(&length);
        if ( length > 17 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_tcp_ip_address,length);
        _tcp_ip_address[17] = '\0';

        file->Load(&_tcp_sendbuf);
        file->Load(&_tcp_recvbuf);
        file->Load(&Enum) ;
        _tcp_rdns = (enum THREE_WAY)Enum;
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_telnet_acct,length);
        _telnet_acct[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_telnet_disp,length);
        _telnet_disp[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_telnet_job,length);
        _telnet_job[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_telnet_printer,length);
        _telnet_printer[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_k4_realm,length);
        _k4_realm[256] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_k4_princ,length);
        _k4_princ[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_k5_realm,length);
        _k5_realm[256] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_k5_princ,length);
        _k5_princ[64] = '\0';
        file->Load(&length);
        if ( length > 64 ) {
            itemMinor = 16;
            DataLoss(name);
            goto ver_1_17;
        }
        file->Load(_k5_cache,length);
        _k5_cache[256] = '\0';
    }

    ver_1_18:
    if ( itemMinor < 18 ) {
        _default_ip_address = 1;
    } else {
        file->Load(&_default_ip_address);
    }

    ver_1_19:
    /* 1.1.18 CU */
    if ( itemMinor < 19 ) {
        _ipport[0] = '\0';
        _stopbits = STOPBITS_1_0;
        _use_mdm_stopbits = 1;
        _telnet_kermit_mode = TelnetAccept;
        _tcp_dns_srv = 0;
        _fwd_cred = 0;
        _telnet_wait = 1;
        _k4_autoget = 0;
        _k4_autodestroy = 0;
        _k4_lifetime = 600;
        _k5_autoget = 0;
        _k5_autodestroy = 0;
        _k5_lifetime = 600;
        _k5_forwardable = 0;
        _k5_getk4tkt = 1;
        _printer_ps = 0;
        _printer_width = 80;
        _printer_length = 66;
        _log_connection = 0;
        _log_connection_append = 1;
        strcpy(_log_connection_fname,"connection.log");
        _log_session_type = LOG_BINARY;
    } else {
        file->Load(&length);
        if ( length > 32 ) {
            itemMinor = 18;
            DataLoss(name);
            goto ver_1_19;
        }
        file->Load(_ipport,length);
        _ipport[32] = '\0';
        file->Load(&Enum) ;
        switch ( Enum ) {
        case STOPBITS_1_0:
            _stopbits = STOPBITS_1_0;
            break;
        case STOPBITS_1_5:
            _stopbits = STOPBITS_1_5;
            break;
        case STOPBITS_2_0:
            _stopbits = STOPBITS_2_0;
            break;
        }
        file->Load(&Enum) ;
        _telnet_kermit_mode = (enum TELNET_MODE)Enum;
        file->Load(&_tcp_dns_srv);
        file->Load(&_fwd_cred);
        file->Load(&_telnet_wait);
        file->Load(&_k4_autoget);
        file->Load(&_k4_autodestroy);
        file->Load(&_k4_lifetime);
        file->Load(&_k5_autoget);
        file->Load(&_k5_autodestroy);
        file->Load(&_k5_lifetime);
        file->Load(&_k5_forwardable);
        file->Load(&_k5_getk4tkt);
        file->Load(&_printer_ps);
        file->Load(&_printer_width);
        file->Load(&_printer_length);
	file->Load(&_log_connection);
	file->Load(&_log_connection_append);
	file->Load(&length) ;
        if ( length > 256 ) {
            itemMinor = 18;
            DataLoss(name);
            goto ver_1_19;
        }
	file->Load(_log_connection_fname,length) ;
	_log_connection_fname[256] = '\0' ;
        file->Load(&Enum) ;
        _log_session_type = (enum LOGTYPE)Enum;
    }


    ver_1_20:
    /* 1.1.19  */
    if ( itemMinor < 20 ) {
        _telnet_starttls_mode = TelnetAccept;
        _tls_cipher[0] = '\0';
        _tls_cert_file[0] = '\0';
        _tls_key_file[0] = '\0';
        _tls_verify_file[0] = '\0';
        _tls_verify_dir[0] = '\0';
        _tls_crl_file[0] = '\0';
        _tls_crl_dir[0] = '\0';
        _tls_verbose = 0;
        _tls_debug = 0;
        _tls_verify_mode = TLS_VERIFY_PEER;

    } else {
        file->Load(&Enum) ;
        _telnet_starttls_mode = (enum TELNET_MODE)Enum;
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_cipher,length);
        _tls_cipher[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_cert_file,length);
        _tls_cert_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_key_file,length);
        _tls_key_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_verify_file,length);
        _tls_verify_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_verify_dir,length);
        _tls_verify_dir[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_crl_file,length);
        _tls_crl_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 19;
            DataLoss(name);
            goto ver_1_20;
        }
        file->Load(_tls_crl_dir,length);
        _tls_crl_dir[256] = '\0';
	file->Load(&_tls_verbose);
	file->Load(&_tls_debug);
        file->Load(&Enum) ;
        _tls_verify_mode = (enum TLS_VERIFY)Enum;
    }

    ver_1_21:
    if ( itemMinor < 21 ) {
        _telnet_debug = 0;
    } else {
        file->Load(&_telnet_debug);
    }

    ver_1_22:
    if ( itemMinor < 22 )
        _telnet_sb_delay = 1;
    else
        file->Load(&_telnet_sb_delay);

    ver_1_23:
    if ( itemMinor < 23 )
        _tcpproto = TCP_DEFAULT;
    else {
        file->Load(&Enum) ;
        _tcpproto = (enum TCPPROTO)Enum;
    }

    ver_1_24:
    if ( itemMinor < 24 ) {
        _streaming = 1;
        _clear_channel = 1;
    } else {
        file->Load(&_streaming);
    }   file->Load(&_clear_channel);

  ver_1_25:
    if ( itemMinor < 25 ) {
        _sshproto = SSH_AUTO;
        _sshport[0] = '\0';
    } else {
        file->Load(&Enum) ;
        _sshproto = (enum SSHPROTO)Enum;
        file->Load(&length);
        if ( length > 32 ) {
            itemMinor = 24;
            DataLoss(name);
            goto ver_1_25;
        }
        file->Load(_sshport,length);
        _sshport[32] = '\0';
    }

  ver_1_26:
    if ( itemMinor < 26 ) {
        _ssh_compress = 1;
        _ssh_x11 = 0;
        _ssh1_cipher = SSH1_CIPHER_3DES;
        _ssh_host_check = HC_ASK;
        _ssh2_auth_external_keyx = 1;
        _ssh2_auth_gssapi = 1;
        _ssh2_auth_hostbased = 1;
        _ssh2_auth_keyboard_interactive = 1;
        _ssh2_auth_password = 1;
        _ssh2_auth_publickey = 1;
        _ssh2_auth_srp_gex_sha1 = 1;
        _ssh2_cipher_3des = 1;
        _ssh2_cipher_aes128 = 1;
        _ssh2_cipher_aes192 = 1;
        _ssh2_cipher_aes256 = 1;
        _ssh2_cipher_arcfour = 1;
        _ssh2_cipher_blowfish = 1;
        _ssh2_cipher_cast128 = 1;
        _ssh2_mac_md5 = 1;
        _ssh2_mac_md5_96 = 1;
        _ssh2_mac_ripemd160 = 1;
        _ssh2_mac_sha1 = 1;
        _ssh2_mac_sha1_96 = 1;
        _ssh2_hka_dss = 1;
        _ssh2_hka_rsa = 1;
        _ssh1_id_file[0] = '\0';
        _ssh1_kh_file[0] = '\0';
        _ssh2_id_file[0] = '\0';
        _ssh2_kh_file[0] = '\0';
    } else {
        file->Load(&_ssh_compress);
        file->Load(&_ssh_x11);
        file->Load(&Enum);
        _ssh1_cipher = (enum SSH1_CIPHER)Enum;
        file->Load(&Enum);
        _ssh_host_check = (enum SSH_HOST_CHECK)Enum;
        file->Load(&_ssh2_auth_external_keyx);
        file->Load(&_ssh2_auth_gssapi);
        file->Load(&_ssh2_auth_hostbased);
        file->Load(&_ssh2_auth_keyboard_interactive);
        file->Load(&_ssh2_auth_password);
        file->Load(&_ssh2_auth_publickey);
        file->Load(&_ssh2_auth_srp_gex_sha1);
        file->Load(&_ssh2_cipher_3des);
        file->Load(&_ssh2_cipher_aes128);
        file->Load(&_ssh2_cipher_aes192);
        file->Load(&_ssh2_cipher_aes256);
        file->Load(&_ssh2_cipher_arcfour);
        file->Load(&_ssh2_cipher_blowfish);
        file->Load(&_ssh2_cipher_cast128);
        file->Load(&_ssh2_mac_md5);
        file->Load(&_ssh2_mac_md5_96);
        file->Load(&_ssh2_mac_ripemd160);
        file->Load(&_ssh2_mac_sha1);
        file->Load(&_ssh2_mac_sha1_96);
        file->Load(&_ssh2_hka_dss);
        file->Load(&_ssh2_hka_rsa);
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 25;
            DataLoss(name);
            goto ver_1_26;
        }
        file->Load(_ssh1_id_file,length);
        _ssh1_id_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 25;
            DataLoss(name);
            goto ver_1_26;
        }
        file->Load(_ssh1_kh_file,length);
        _ssh1_kh_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 25;
            DataLoss(name);
            goto ver_1_26;
        }
        file->Load(_ssh2_id_file,length);
        _ssh2_id_file[256] = '\0';
        file->Load(&length);
        if ( length > 256 ) {
            itemMinor = 25;
            DataLoss(name);
            goto ver_1_26;
        }
        file->Load(_ssh2_kh_file,length);
        _ssh2_kh_file[256] = '\0';
    }

  ver_1_27:
    if ( itemMinor < 27 ) {
        _ftpport[0] = '\0';
    } else {
        file->Load(&length);
        if ( length > 32 ) {
            itemMinor = 26;
            DataLoss(name);
            goto ver_1_27;
        }
        file->Load(_ftpport,length);
        _ftpport[32] = '\0';
    }

  ver_1_28:
    if ( itemMinor < 28 ) {
        _ftp_autologin = 1;  
        _ftp_passive   = 1;    
        _ftp_autoauth  = 1;   
        _ftp_autoenc   = 1;    
        _ftp_credfwd   = 0;    
        _ftp_cpl       = PL_PRIVATE;  
        _ftp_dpl       = PL_PRIVATE;
        _ftp_debug     = 0;      
        _ftp_dates     = 1;      
        _ftp_literal   = 1;    
        _ftp_verbose   = 1;    
        _ftp_xlat      = 0;       
        _ftp_charset   = F_ASCII;   
        _ftp_auth_gssk5 = 1; 
        _ftp_auth_k4   = 1;    
        _ftp_auth_srp  = 1;   
        _ftp_auth_ssl  = 1;   
        _ftp_auth_tls  = 1;   
    } else {
        file->Load(&_ftp_autologin);  
        file->Load(&_ftp_passive);
        file->Load(&_ftp_autoauth);
        file->Load(&_ftp_autoenc);
        file->Load(&_ftp_credfwd);
        file->Load(&Enum);
        _ftp_cpl       = (enum FTP_PL)Enum;  
        file->Load(&Enum);
        _ftp_dpl       = (enum FTP_PL)Enum;
        file->Load(&_ftp_debug);
        file->Load(&_ftp_dates);
        file->Load(&_ftp_literal);
        file->Load(&_ftp_verbose);
        file->Load(&_ftp_xlat);
        file->Load(&Enum);
        _ftp_charset = (enum FILECSET) Enum;
        file->Load(&_ftp_auth_gssk5);
        file->Load(&_ftp_auth_k4);
        file->Load(&_ftp_auth_srp);
        file->Load(&_ftp_auth_ssl);
        file->Load(&_ftp_auth_tls);
    }

  ver_1_29:
    if ( itemMinor < 29 ) {
        _telnet_fwdx_mode = TelnetAccept;
        _telnet_cpc_mode = TelnetAccept;
    } else {
        file->Load(&Enum);
        _telnet_fwdx_mode = (enum TELNET_MODE) Enum;
        file->Load(&Enum);
        _telnet_cpc_mode = (enum TELNET_MODE) Enum;
    }

  ver_1_30:
    if ( itemMinor < 30 ) {
        _use_gui = 1;
        strcpy(_facename,"Courier New");
        _fontsize = 12;
        _startpos_auto = 1;
        _startpos_x = 0;
        _startpos_y = 0;
        _rgb[0][0] = 0;
        _rgb[0][1] = 0;
        _rgb[0][2] = 0;
        _rgb[1][0] = 0;
        _rgb[1][1] = 0;
        _rgb[1][2] = 128;
        _rgb[2][0] = 0;
        _rgb[2][1] = 128;
        _rgb[2][2] = 0;
        _rgb[3][0] = 0;
        _rgb[3][1] = 128;
        _rgb[3][2] = 128;
        _rgb[4][0] = 128;
        _rgb[4][1] = 0;
        _rgb[4][2] = 0;
        _rgb[5][0] = 128;
        _rgb[5][1] = 0;
        _rgb[5][2] = 128;
        _rgb[6][0] = 128;
        _rgb[6][1] = 128;
        _rgb[6][2] = 0;
        _rgb[7][0] = 192;
        _rgb[7][1] = 192;
        _rgb[7][2] = 192;
        _rgb[8][0] = 128;
        _rgb[8][1] = 128;
        _rgb[8][2] = 128;
        _rgb[9][0] = 0;
        _rgb[9][1] = 0;
        _rgb[9][2] = 255;
        _rgb[10][0] = 0;
        _rgb[10][1] = 255;
        _rgb[10][2] = 0;
        _rgb[11][0] = 0;
        _rgb[11][1] = 255;
        _rgb[11][2] = 255;
        _rgb[12][0] = 255;
        _rgb[12][1] = 0;
        _rgb[12][2] = 0;
        _rgb[13][0] = 255;
        _rgb[13][1] = 0;
        _rgb[13][2] = 255;
        _rgb[14][0] = 255;
        _rgb[14][1] = 255;
        _rgb[14][2] = 0;
        _rgb[15][0] = 255;
        _rgb[15][1] = 255;
        _rgb[15][2] = 255;
    } else {
        file->Load(&_use_gui);
        file->Load(&length);
        if ( length > 127 ) {
            itemMinor = 29;
            DataLoss(name);
            goto ver_1_30;
        }
        file->Load(_facename,length);
        _facename[127] = '\0';
        file->Load(&_fontsize);
        file->Load(&_startpos_auto);
        file->Load(&_startpos_x);
        file->Load(&_startpos_y);
        file->Load(&_rgb[0][0]);
        file->Load(&_rgb[0][1]);
        file->Load(&_rgb[0][2]);
        file->Load(&_rgb[1][0]);
        file->Load(&_rgb[1][1]);
        file->Load(&_rgb[1][2]);
        file->Load(&_rgb[2][0]);
        file->Load(&_rgb[2][1]);
        file->Load(&_rgb[2][2]);
        file->Load(&_rgb[3][0]);
        file->Load(&_rgb[3][1]);
        file->Load(&_rgb[3][2]);
        file->Load(&_rgb[4][0]);
        file->Load(&_rgb[4][1]);
        file->Load(&_rgb[4][2]);
        file->Load(&_rgb[5][0]);
        file->Load(&_rgb[5][1]);
        file->Load(&_rgb[5][2]);
        file->Load(&_rgb[6][0]);
        file->Load(&_rgb[6][1]);
        file->Load(&_rgb[6][2]);
        file->Load(&_rgb[7][0]);
        file->Load(&_rgb[7][1]);
        file->Load(&_rgb[7][2]);
        file->Load(&_rgb[8][0]);
        file->Load(&_rgb[8][1]);
        file->Load(&_rgb[8][2]);
        file->Load(&_rgb[9][0]);
        file->Load(&_rgb[9][1]);
        file->Load(&_rgb[9][2]);
        file->Load(&_rgb[10][0]);
        file->Load(&_rgb[10][1]);
        file->Load(&_rgb[10][2]);
        file->Load(&_rgb[11][0]);
        file->Load(&_rgb[11][1]);
        file->Load(&_rgb[11][2]);
        file->Load(&_rgb[12][0]);
        file->Load(&_rgb[12][1]);
        file->Load(&_rgb[12][2]);
        file->Load(&_rgb[13][0]);
        file->Load(&_rgb[13][1]);
        file->Load(&_rgb[13][2]);
        file->Load(&_rgb[14][0]);
        file->Load(&_rgb[14][1]);
        file->Load(&_rgb[14][2]);
        file->Load(&_rgb[15][0]);
        file->Load(&_rgb[15][1]);
        file->Load(&_rgb[15][2]);
   }

  ver_1_31:
    if ( itemMinor < 31 ) {
        _ssh_credfwd = 0;  
    } else {
        file->Load(&_ssh_credfwd);
    }

  ver_1_32:
    if ( itemMinor < 32 ) {
        _proxy_ipaddress[0] = '\0';
        _proxy_ipport[0] = '\0';
        _socks_ipaddress[0] = '\0';
        _socks_ipport[0] = '\0';
    } else {
        file->Load(&length);
        if ( length > 257 ) {
            itemMinor = 31;
            DataLoss(name);
            goto ver_1_32;
        }
        file->Load(_proxy_ipaddress,length);

        file->Load(&length);
        if ( length > 33 ) {
            itemMinor = 31;
            DataLoss(name);
            goto ver_1_32;
        }
        file->Load(_proxy_ipport,length);

        file->Load(&length);
        if ( length > 257 ) {
            itemMinor = 31;
            DataLoss(name);
            goto ver_1_32;
        }
        file->Load(_socks_ipaddress,length);

        file->Load(&length);
        if ( length > 33 ) {
            itemMinor = 31;
            DataLoss(name);
            goto ver_1_32;
        }
        file->Load(_socks_ipport,length);
    }

  ver_1_33:
    if ( itemMinor < 33 ) {
        _keyscript[0] = '\0';
    } else {
        file->Load(&length) ;
        if ( length > 3000 ) {
            itemMinor = 32;
            DataLoss(name);
            goto ver_1_33;
        }
        file->Load(_keyscript,length) ;
        _keyscript[3000] = '\0' ;
    }

  ver_1_34:
    if ( itemMinor < 34 ) {
        _gui_resize = RES_CHANGE_DIM;
        _gui_run = RUN_RES;
    } else {
        file->Load(&Enum);
        _gui_resize = (enum GUI_RESIZE) Enum;
        file->Load(&Enum);
        _gui_run = (enum GUI_RUN) Enum;
    }

  ver_1_35:
    if ( itemMinor < 35 ) {
        _proxy_user[0] = '\0';
        _proxy_pass[0] = '\0';
        _socks_user[0] = '\0';
        _socks_pass[0] = '\0';
    } else {
        file->Load(&length);
        if ( length > 65 ) {
            itemMinor = 34;
            DataLoss(name);
            goto ver_1_35;
        }
        file->Load(_proxy_user,length);

        file->Load(&length);
        if ( length > 65 ) {
            itemMinor = 34;
            DataLoss(name);
            goto ver_1_35;
        }
        file->Load(_proxy_pass,length);

        file->Load(&length);
        if ( length > 65 ) {
            itemMinor = 34;
            DataLoss(name);
            goto ver_1_35;
        }
        file->Load(_socks_user,length);

        file->Load(&length);
        if ( length > 65 ) {
            itemMinor = 34;
            DataLoss(name);
            goto ver_1_35;
        }
        file->Load(_socks_pass,length);
    }

  ver_1_36:
    if ( itemMinor < 36 ) {
        _printer_charset = T_CP437;
    } else {
        file->Load(&Enum) ;
        _printer_charset = (enum TERMCSET) Enum ;
    }

  ver_1_37:
    if ( itemMinor < 37 ) {
        _ssh2_cipher_aes128ctr = 1;
        _ssh2_cipher_aes192ctr = 1;
        _ssh2_cipher_aes256ctr = 1;
        _ssh2_cipher_aes256_gcm_openssh = 1;
        _ssh2_cipher_chachae20_poly1305 = 1;
        _ssh2_cipher_aes128_gcm_openssh = 1;

        _ssh2_mac_none = 0;
        _ssh2_mac_sha2_512 = 1;
        _ssh2_mac_sha2_256 = 1;
        _ssh2_mac_sha1_etm_openssh = 1;
        _ssh2_mac_sha2_512_etm_openssh = 1;
        _ssh2_mac_sha2_256_etm_openssh = 1;

        _ssh2_hka_ssh_ed25519 = 1;
        _ssh2_hka_ecdsa_sha2_nistp521 = 1;
        _ssh2_hka_ecdsa_sha2_nistp384 = 1;
        _ssh2_hka_ecdsa_sha2_nistp256 = 1;
        _ssh2_hka_rsa_sha2_512 = 1;
        _ssh2_hka_rsa_sha2_256 = 1;

        _ssh2_kex_curve25519_sha256 = 1;
        _ssh2_kex_curve25519_sha256_libssh = 1;
        _ssh2_kex_ecdh_sha2_nistp256 = 1;
        _ssh2_kex_ecdh_sha2_nistp384 = 1;
        _ssh2_kex_ecdh_sha2_nistp521 = 1;
        _ssh2_kex_dh_group18_sha512 = 1;
        _ssh2_kex_dh_group16_sha512 = 1;
        _ssh2_kex_dh_group_exchange_sha256 = 1;
        _ssh2_kex_dh_group14_sha256 = 1;
        _ssh2_kex_dh_group14_sha1 = 1;
        _ssh2_kex_dh_group1_sha1 = 1;
        _ssh2_kex_ext_info_c = 1;
        _ssh2_kex_dh_group_exchange_sha1 = 1;
    } else {
        file->Load(&_ssh2_cipher_aes128ctr);
        file->Load(&_ssh2_cipher_aes192ctr);
        file->Load(&_ssh2_cipher_aes256ctr);
        file->Load(&_ssh2_cipher_aes256_gcm_openssh);
        file->Load(&_ssh2_cipher_chachae20_poly1305);
        file->Load(&_ssh2_cipher_aes128_gcm_openssh);

        file->Load(&_ssh2_mac_none);
        file->Load(&_ssh2_mac_sha2_512);
        file->Load(&_ssh2_mac_sha2_256);
        file->Load(&_ssh2_mac_sha1_etm_openssh);
        file->Load(&_ssh2_mac_sha2_512_etm_openssh);
        file->Load(&_ssh2_mac_sha2_256_etm_openssh);

        file->Load(&_ssh2_hka_ssh_ed25519);
        file->Load(&_ssh2_hka_ecdsa_sha2_nistp521);
        file->Load(&_ssh2_hka_ecdsa_sha2_nistp384);
        file->Load(&_ssh2_hka_ecdsa_sha2_nistp256);
        file->Load(&_ssh2_hka_rsa_sha2_512);
        file->Load(&_ssh2_hka_rsa_sha2_256);

        file->Load(&_ssh2_kex_curve25519_sha256);
        file->Load(&_ssh2_kex_curve25519_sha256_libssh);
        file->Load(&_ssh2_kex_ecdh_sha2_nistp256);
        file->Load(&_ssh2_kex_ecdh_sha2_nistp384);
        file->Load(&_ssh2_kex_ecdh_sha2_nistp521);
        file->Load(&_ssh2_kex_dh_group18_sha512);
        file->Load(&_ssh2_kex_dh_group16_sha512);
        file->Load(&_ssh2_kex_dh_group_exchange_sha256);
        file->Load(&_ssh2_kex_dh_group14_sha256);
        file->Load(&_ssh2_kex_dh_group14_sha1);
        file->Load(&_ssh2_kex_dh_group1_sha1);
        file->Load(&_ssh2_kex_ext_info_c);
        file->Load(&_ssh2_kex_dh_group_exchange_sha1);
    }
}
#endif

#if defined(ZIL_STORE)
void KD_LIST_ITEM::Store(const ZIL_ICHAR *name, ZIL_STORAGE *directory,
	ZIL_STORAGE_OBJECT *file, UI_ITEM *objectTable, UI_ITEM *userTable)
{
	// Store the button information.
    UIW_BUTTON::Store(name, directory, file, objectTable, userTable);
   file->Store(kd_major) ;
   file->Store(kd_minor) ;

   ZIL_UINT16 length = 1+strlen(_name);
   ZIL_UINT16 Enum ;
   file->Store(length);
   file->Store(_name) ;

   length = 1+strlen(_location);
   file->Store(length);
   file->Store(_location) ;

   file->Store(Enum = _access) ;

   length = 1+strlen(_phone_number);
   file->Store(length);
   file->Store(_phone_number) ;

   length = 1+strlen(_ipaddress);
   file->Store(length);
   file->Store(_ipaddress) ;

   file->Store(_template) ;
   
   length = 1+strlen(_startup_dir);
   file->Store(length);
   file->Store(_startup_dir) ;

   length = 1+strlen(_notes);
   file->Store(length);
   file->Store(_notes) ;

   file->Store(Enum = _terminal) ;
   file->Store(_charsize) ;
   file->Store(_local_echo) ;
   file->Store(_auto_wrap) ;
   file->Store(_apc_enabled) ;
   file->Store(_height) ;
   file->Store(_width) ;
   file->Store(_scrollback) ;
   file->Store(_color_term_fg) ;
   file->Store(_color_term_bg) ;
   file->Store(_color_status_fg) ;
   file->Store(_color_status_bg) ;
   file->Store(_color_popup_fg) ;
   file->Store(_color_popup_bg) ;
   file->Store(_color_select_fg) ;
   file->Store(_color_select_bg) ;
   file->Store(_color_underline_fg) ;
   file->Store(_color_underline_bg) ;
   file->Store(Enum = _term_charset) ;
   file->Store(Enum = _protocol) ;
   file->Store(Enum = _xfermode) ;
   file->Store(Enum = _collision) ;
   file->Store(_packet_length) ;
   file->Store(_max_windows) ;
   file->Store(_fname_literal) ;
   file->Store(_pathnames) ;
   file->Store(_keep_incomplete) ;
   file->Store(Enum = _unprefix_cc) ;
   file->Store(Enum = _file_charset) ;
   file->Store(Enum = _xfer_charset) ;

   length = 1+strlen(_line_device);
   file->Store(length);
   file->Store(_line_device) ;

   file->Store(_speed) ;
   file->Store(Enum = _flow) ;
   file->Store(Enum = _parity) ;

   length = 1+strlen(_modem);
   file->Store(length);
   file->Store(_modem) ;

   file->Store(_correction) ;
   file->Store(_compression) ;
   file->Store(_carrier) ;
   file->Store(Enum = _backspace) ;
   file->Store(Enum = _enter) ;
   file->Store(_mouse) ;
   file->Store(Enum = _keymap) ;
   
   length = 1+strlen(_keymap_file);
   file->Store(length);
   file->Store(_keymap_file) ;

   file->Store(_script_file) ;

   length = 1+strlen(_script_fname);
   file->Store(length);
   file->Store(_script_fname) ;

   length = 1+strlen(_userid);
   file->Store(length);
   file->Store(_userid) ;

   length = 1+strlen(_password);
   file->Store(length);
   file->Store(_password) ;  

   length = 1+strlen(_script);
   file->Store(length);
   file->Store(_script);

   /* Items new to version 1.1 */
   /* In version 1.35 we changed _auto_download to an enum but 
    * the old value was 8-bit and enums are 16-bit.  So must store
    * as an 8-bit value for compatibilility
    */
   ZIL_UINT8 enum8 = (ZIL_UINT8) _auto_download;
   file->Store(enum8);

   length = 1 + strlen(_prompt) ;
   file->Store(length) ;
   file->Store(_prompt) ;

   file->Store(_color_command_fg) ;
   file->Store(_color_command_bg) ;

   /* Items new to version 1.2 */
   file->Store(Enum = _cursor) ;

    /* Items new to version 1.3 */
    length = 1+strlen(_lataddress);
    file->Store(length);
    file->Store(_lataddress) ;

    /* Items new to 1.1.7 (minor = 4) */
    file->Store(_autoexit);
    file->Store(_status_line);

    /* Items new to 1.1.12 (minor = 7) */
    length = 1+strlen(_telnet_ttype);
    file->Store(length);
    file->Store(_telnet_ttype) ;

    /* Items new to 1.1.12 (minor = 8) */
    file->Store(_tapi_line);

    /* Items new to 1.1.12 (minor = 9) */
    file->Store(_log_debug);
    length = 1+strlen(_log_debug_fname);
    file->Store(length);
    file->Store(_log_debug_fname);
    file->Store(_log_session);
    length = 1+strlen(_log_session_fname);
    file->Store(length);
    file->Store(_log_session_fname);
    file->Store(_log_packet);
    length = 1+strlen(_log_packet_fname);
    file->Store(length);
    file->Store(_log_packet_fname);
    file->Store(_log_transaction);
    length = 1+strlen(_log_transaction_fname);
    file->Store(length);
    file->Store(_log_transaction_fname);

    /* Items new to 1.1.12 (minor = 10) */
    file->Store(_log_debug_append);
    file->Store(_log_session_append);
    file->Store(_log_packet_append);
    file->Store(_log_transaction_append);

    /* Items new to 1.1.12 (minor = 11) */
    file->Store(_use_mdm_speed);
    file->Store(_use_mdm_flow);
    file->Store(_use_mdm_parity);
    file->Store(_use_mdm_ec);
    file->Store(_use_mdm_dc);
    file->Store(_use_mdm_carrier);

    /* Items new to 1.1.12 (minor = 12) */
    length = 1+strlen(_printer_dos);
    file->Store(length);
    file->Store(_printer_dos);

    /* Items new to 1.1.14 (minor = 15) */
    file->Store(Enum = _printer_type) ;
    length = 1+strlen(_printer_windows);
    file->Store(length);
    file->Store(_printer_windows);
    length = 1+strlen(_printer_separator);
    file->Store(length);
    file->Store(_printer_separator);
    file->Store(_printer_formfeed);       
    length = 1+strlen(_printer_terminator);
    file->Store(length);
    file->Store(_printer_terminator);
    file->Store(_printer_timeout);       
    file->Store(_printer_bidi);
    file->Store(_printer_speed) ;
    file->Store(Enum=_printer_flow);
    file->Store(Enum= _printer_parity) ;
    file->Store(_prompt_for_password);

    /* Items new to 1.1.16 (minor = 16) */
    file->Store(_log_transaction_brief);
    file->Store(Enum = _telnet_auth_mode);
    file->Store(Enum = _telnet_binary_mode);
    file->Store(Enum = _telnet_encrypt_mode);
    length = 1+strlen(_telnet_location);
    file->Store(length);
    file->Store(_telnet_location);
    length = 1 + strlen(_tcp_ip_address);
    file->Store(length);
    file->Store(_tcp_ip_address);
    file->Store(_tcp_sendbuf);
    file->Store(_tcp_recvbuf);
    file->Store(Enum = _tcp_rdns);
    length = 1 + strlen(_telnet_acct);
    file->Store(length);
    file->Store(_telnet_acct);
    length = 1 + strlen(_telnet_disp);
    file->Store(length);
    file->Store(_telnet_disp);
    length = 1 + strlen(_telnet_job);
    file->Store(length);
    file->Store(_telnet_job);
    length = 1 + strlen(_telnet_printer);
    file->Store(length);
    file->Store(_telnet_printer);
    length = 1 + strlen(_k4_realm);
    file->Store(length);
    file->Store(_k4_realm);
    length = 1 + strlen(_k4_princ);
    file->Store(length);
    file->Store(_k4_princ);
    length = 1 + strlen(_k5_realm);
    file->Store(length);
    file->Store(_k5_realm);
    length = 1 + strlen(_k5_princ);
    file->Store(length);
    file->Store(_k5_princ);
    length = 1 + strlen(_k5_cache);
    file->Store(length);
    file->Store(_k5_cache);

    file->Store(_default_ip_address);

    // Items new to 1.1.18 CU (minor == 19)
    length = 1 + strlen(_ipport);
    file->Store(length);
    file->Store(_ipport);
    file->Store(Enum = _stopbits);
    file->Store(Enum = _telnet_kermit_mode);
    file->Store(_tcp_dns_srv);
    file->Store(_fwd_cred);
    file->Store(_telnet_wait);
    file->Store(_k4_autoget);
    file->Store(_k4_autodestroy);
    file->Store(_k4_lifetime);
    file->Store(_k5_autoget);
    file->Store(_k5_autodestroy);
    file->Store(_k5_lifetime);
    file->Store(_k5_forwardable);
    file->Store(_k5_getk4tkt);
    file->Store(_printer_ps);
    file->Store(_printer_width);
    file->Store(_printer_length);
    file->Store(_log_connection);
    file->Store(_log_connection_append);
    length = 1 + strlen(_log_connection_fname);
    file->Store(length);
    file->Store(_log_connection_fname);
    file->Store(Enum = _log_session_type);

    /* 1.1.18 General Release (minor == 20) */
    file->Store(Enum = _telnet_starttls_mode);
    length = 1 + strlen(_tls_cipher);
    file->Store(length);
    file->Store(_tls_cipher);
    length = 1 + strlen(_tls_cert_file);
    file->Store(length);
    file->Store(_tls_cert_file);
    length = 1 + strlen(_tls_key_file);
    file->Store(length);
    file->Store(_tls_key_file);
    length = 1 + strlen(_tls_verify_file);
    file->Store(length);
    file->Store(_tls_verify_file);
    length = 1 + strlen(_tls_verify_dir);
    file->Store(length);
    file->Store(_tls_verify_dir);
    length = 1 + strlen(_tls_crl_file);
    file->Store(length);
    file->Store(_tls_crl_file);
    length = 1 + strlen(_tls_crl_dir);
    file->Store(length);
    file->Store(_tls_crl_dir);
    file->Store(_tls_verbose);
    file->Store(_tls_debug);
    file->Store(Enum = _tls_verify_mode);

    // itemMinor = 21
    file->Store(_telnet_debug);

    // itemMinor = 22
    file->Store(_telnet_sb_delay);

    // itemMinor = 23;
    file->Store(Enum = _tcpproto);

    // itemMinor = 24
    file->Store(_streaming) ;
    file->Store(_clear_channel) ;

    // itemMinor = 25
    file->Store(Enum = _sshproto);
    length = 1 + strlen(_sshport);
    file->Store(length);
    file->Store(_sshport);

    // itemMinor = 26
    file->Store(_ssh_compress);
    file->Store(_ssh_x11);
    file->Store(Enum = _ssh1_cipher);
    file->Store(Enum = _ssh_host_check);
    file->Store(_ssh2_auth_external_keyx);
    file->Store(_ssh2_auth_gssapi);
    file->Store(_ssh2_auth_hostbased);
    file->Store(_ssh2_auth_keyboard_interactive);
    file->Store(_ssh2_auth_password);
    file->Store(_ssh2_auth_publickey);
    file->Store(_ssh2_auth_srp_gex_sha1);
    file->Store(_ssh2_cipher_3des);
    file->Store(_ssh2_cipher_aes128);
    file->Store(_ssh2_cipher_aes192);
    file->Store(_ssh2_cipher_aes256);
    file->Store(_ssh2_cipher_arcfour);
    file->Store(_ssh2_cipher_blowfish);
    file->Store(_ssh2_cipher_cast128);
    file->Store(_ssh2_mac_md5);
    file->Store(_ssh2_mac_md5_96);
    file->Store(_ssh2_mac_ripemd160);
    file->Store(_ssh2_mac_sha1);
    file->Store(_ssh2_mac_sha1_96);
    file->Store(_ssh2_hka_dss);
    file->Store(_ssh2_hka_rsa);
    length = 1 + strlen(_ssh1_id_file);
    file->Store(length);
    file->Store(_ssh1_id_file);
    length = 1 + strlen(_ssh1_kh_file);
    file->Store(length);
    file->Store(_ssh1_kh_file);
    length = 1 + strlen(_ssh2_id_file);
    file->Store(length);
    file->Store(_ssh2_id_file);
    length = 1 + strlen(_ssh2_kh_file);
    file->Store(length);
    file->Store(_ssh2_kh_file);

    // itemMinor = 27
    length = 1 + strlen(_ftpport);
    file->Store(length);
    file->Store(_ftpport);

    // itemMinor = 28
    file->Store(_ftp_autologin);  
    file->Store(_ftp_passive);
    file->Store(_ftp_autoauth);
    file->Store(_ftp_autoenc);
    file->Store(_ftp_credfwd);
    file->Store(Enum = _ftp_cpl);  
    file->Store(Enum = _ftp_dpl);
    file->Store(_ftp_debug);
    file->Store(_ftp_dates);
    file->Store(_ftp_literal);
    file->Store(_ftp_verbose);
    file->Store(_ftp_xlat);
    file->Store(Enum = _ftp_charset);
    file->Store(_ftp_auth_gssk5);
    file->Store(_ftp_auth_k4);
    file->Store(_ftp_auth_srp);
    file->Store(_ftp_auth_ssl);
    file->Store(_ftp_auth_tls);

    // itemMinor = 29 (1.1.21 final release)
    file->Store(Enum = _telnet_fwdx_mode);
    file->Store(Enum = _telnet_cpc_mode);

    // itemMinor = 30
    file->Store(_use_gui);
    length = 1 + strlen(_facename);
    file->Store(length);
    file->Store(_facename);
    file->Store(_fontsize);
    file->Store(_startpos_auto);
    file->Store(_startpos_x);
    file->Store(_startpos_y);
    file->Store(_rgb[0][0]);
    file->Store(_rgb[0][1]);
    file->Store(_rgb[0][2]);
    file->Store(_rgb[1][0]);
    file->Store(_rgb[1][1]);
    file->Store(_rgb[1][2]);
    file->Store(_rgb[2][0]);
    file->Store(_rgb[2][1]);
    file->Store(_rgb[2][2]);
    file->Store(_rgb[3][0]);
    file->Store(_rgb[3][1]);
    file->Store(_rgb[3][2]);
    file->Store(_rgb[4][0]);
    file->Store(_rgb[4][1]);
    file->Store(_rgb[4][2]);
    file->Store(_rgb[5][0]);
    file->Store(_rgb[5][1]);
    file->Store(_rgb[5][2]);
    file->Store(_rgb[6][0]);
    file->Store(_rgb[6][1]);
    file->Store(_rgb[6][2]);
    file->Store(_rgb[7][0]);
    file->Store(_rgb[7][1]);
    file->Store(_rgb[7][2]);
    file->Store(_rgb[8][0]);
    file->Store(_rgb[8][1]);
    file->Store(_rgb[8][2]);
    file->Store(_rgb[9][0]);
    file->Store(_rgb[9][1]);
    file->Store(_rgb[9][2]);
    file->Store(_rgb[10][0]);
    file->Store(_rgb[10][1]);
    file->Store(_rgb[10][2]);
    file->Store(_rgb[11][0]);
    file->Store(_rgb[11][1]);
    file->Store(_rgb[11][2]);
    file->Store(_rgb[12][0]);
    file->Store(_rgb[12][1]);
    file->Store(_rgb[12][2]);
    file->Store(_rgb[13][0]);
    file->Store(_rgb[13][1]);
    file->Store(_rgb[13][2]);
    file->Store(_rgb[14][0]);
    file->Store(_rgb[14][1]);
    file->Store(_rgb[14][2]);
    file->Store(_rgb[15][0]);
    file->Store(_rgb[15][1]);
    file->Store(_rgb[15][2]);

    // itemMinor = 31
    file->Store(_ssh_credfwd);

    // itemMinor = 32
    length = 1+strlen(_proxy_ipaddress);
    file->Store(length);
    file->Store(_proxy_ipaddress) ;
    length = 1+strlen(_proxy_ipport);
    file->Store(length);
    file->Store(_proxy_ipport) ;
    length = 1+strlen(_socks_ipaddress);
    file->Store(length);
    file->Store(_socks_ipaddress) ;
    length = 1+strlen(_socks_ipport);
    file->Store(length);
    file->Store(_socks_ipport) ;

    // itemMinor = 33
    length = strlen(_keyscript);
    file->Store(length) ;
    file->Store(_keyscript) ;

    // itemMinor = 34
    file->Store(Enum = _gui_resize) ;
    file->Store(Enum = _gui_run) ;

    // itemMinor = 35
    length = 1+strlen(_proxy_user);
    file->Store(length);
    file->Store(_proxy_user) ;
    length = 1+strlen(_proxy_pass);
    file->Store(length);
    file->Store(_proxy_pass) ;
    length = 1+strlen(_socks_user);
    file->Store(length);
    file->Store(_socks_user) ;
    length = 1+strlen(_socks_pass);
    file->Store(length);
    file->Store(_socks_pass) ;

    // itemMinor = 36
    file->Store(Enum = _printer_charset) ;

    // itemMinor = 37
    file->Store(_ssh2_cipher_aes128ctr);
    file->Store(_ssh2_cipher_aes192ctr);
    file->Store(_ssh2_cipher_aes256ctr);
    file->Store(_ssh2_cipher_aes256_gcm_openssh);
    file->Store(_ssh2_cipher_chachae20_poly1305);
    file->Store(_ssh2_cipher_aes128_gcm_openssh);
    file->Store(_ssh2_mac_none);
    file->Store(_ssh2_mac_sha2_512);
    file->Store(_ssh2_mac_sha2_256);
    file->Store(_ssh2_mac_sha1_etm_openssh);
    file->Store(_ssh2_mac_sha2_512_etm_openssh);
    file->Store(_ssh2_mac_sha2_256_etm_openssh);
    file->Store(_ssh2_hka_ssh_ed25519);
    file->Store(_ssh2_hka_ecdsa_sha2_nistp521);
    file->Store(_ssh2_hka_ecdsa_sha2_nistp384);
    file->Store(_ssh2_hka_ecdsa_sha2_nistp256);
    file->Store(_ssh2_hka_rsa_sha2_512);
    file->Store(_ssh2_hka_rsa_sha2_256);
    file->Store(_ssh2_kex_curve25519_sha256);
    file->Store(_ssh2_kex_curve25519_sha256_libssh);
    file->Store(_ssh2_kex_ecdh_sha2_nistp256);
    file->Store(_ssh2_kex_ecdh_sha2_nistp384);
    file->Store(_ssh2_kex_ecdh_sha2_nistp521);
    file->Store(_ssh2_kex_dh_group18_sha512);
    file->Store(_ssh2_kex_dh_group16_sha512);
    file->Store(_ssh2_kex_dh_group_exchange_sha256);
    file->Store(_ssh2_kex_dh_group14_sha256);
    file->Store(_ssh2_kex_dh_group14_sha1);
    file->Store(_ssh2_kex_dh_group1_sha1);
    file->Store(_ssh2_kex_ext_info_c);
    file->Store(_ssh2_kex_dh_group_exchange_sha1);
}
#endif

EVENT_TYPE 
KD_LIST_ITEM :: Event( const UI_EVENT & event ) 
{
    EVENT_TYPE retval = event.type;

    switch ( event.type ) {
#ifdef WIN32
    case E_MSWINDOWS:
        /* 2022-11-06 DavidG:
         *   This no longer appears to work. The WM_RBUTTONDOWN never gets this
         *   far anymore. Don't know if its a bug in OpenZinc 1.0, or some
         *   change elsewhere in the Dialer made between the final commercial
         *   Kermit 95 release and the dialers open-sourcing. Whatever the
         *   cause, this code appears dead now. The context menu is now
         *   triggered in response to WM_CONTEXTMENU in kconnect.cpp, function:
         *   EVENT_TYPE K_CONNECTOR::
         *   Event( const UI_EVENT & event ).
         */
	if ( event.message.message == WM_RBUTTONDOWN
#ifdef WM_CONTEXTMENU
    || event.message.message == WM_CONTEXTMENU
#endif /* WM_CONTEXTMENU */
    ) {
#ifndef COMMENT
            UIW_VT_LIST * list = (UIW_VT_LIST *) connector->Get( LIST_ENTRIES ) ; 
            list->SetCurrent( this );

            UI_WINDOW_OBJECT * obj = this;
            int left, top;
#ifdef WM_CONTEXTMENU
            if (event.message.message == WM_CONTEXTMENU) {
                left = GET_X_LPARAM(event.message.lParam);
                top = GET_Y_LPARAM(event.message.lParam);
            } else {
#endif /* WM_CONTEXTMENU */
                left = event.position.column + trueRegion.left;
                top = event.position.line;
                for (; obj->parent ; obj = obj->parent );
                left += obj->trueRegion.left;
                top += obj->trueRegion.top;

                UI_WINDOW_OBJECT * obj2 = obj->Get( MENU );
                top += obj2->relative.bottom - obj2->relative.top;
                UI_WINDOW_OBJECT * obj3 = obj->Get( TOOL_BAR );
                top += obj3->relative.bottom - obj3->relative.top;
                UI_WINDOW_OBJECT * obj4 = obj->Get( STATUS_BAR );
                top += obj4->relative.bottom - obj4->relative.top;
#ifdef WM_CONTEXTMENU
            }
#endif /* WM_CONTEXTMENU */

            ShowContextMenu(left, top);
#else
            retval = UIW_BUTTON::Event(event);
#endif
	}
	else
            retval = UIW_BUTTON::Event(event) ;
	break;
#endif /* WIN32 */

    case L_END_SELECT:
    case S_CURRENT:
	if (connector)
	    connector->UpdateStatusLine(TRUE) ;
	break;

    default:
	retval = UIW_BUTTON::Event(event) ;
    }
    
    return retval ; 

}

void KD_LIST_ITEM :: ShowContextMenu(int left, int top) {

    UIW_POP_UP_MENU *popup;
    UIW_POP_UP_ITEM *edit, *generate, *item;
    popup = new UIW_POP_UP_MENU(left, top, WNF_NO_FLAGS, WOF_NO_FLAGS, WOAF_NO_FLAGS);
    popup->woStatus |= WOS_GRAPHICS;
    popup->woAdvancedFlags |= WOAF_TEMPORARY;

    popup->Add(item = new UIW_POP_UP_ITEM("Add", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                          WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                          10000));
    popup->Add(item = new UIW_POP_UP_ITEM("Clone", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                          WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                          10001));
    if ( !_template ) {
        popup->Add(item = new UIW_POP_UP_ITEM("Connect", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                              WNF_NO_FLAGS,
                                              (ZIL_USER_FUNCTION) NULL,
                                              10003));
        //item->woFlags |= _template ? WOF_NON_SELECTABLE : 0;
        //item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    edit = new UIW_POP_UP_ITEM("Edit", MNIF_SEND_MESSAGE, BTF_NO_3D,
                               WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                               10008);
    edit->Add(item = new UIW_POP_UP_ITEM("General", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10138));
    edit->Add(item = new UIW_POP_UP_ITEM("Terminal", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10139));
    edit->Add(item = new UIW_POP_UP_ITEM("File Transfer", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10140));
    if ( !(_access != PHONE && _access != DIRECT) ) {
        edit->Add(item = new UIW_POP_UP_ITEM("Serial", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10141));
        // item->woFlags |= (_access != PHONE && _access != DIRECT) ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( !(_access == PHONE || _access == SUPERLAT || _access == DIRECT || _access == SSH
           || _access == SSH) ) {
        edit->Add(item = new UIW_POP_UP_ITEM("Telnet", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10142));
        // item->woFlags |= (_access == PHONE || _access == SUPERLAT || _access == DIRECT) ?
        // WOF_NON_SELECTABLE : 0;
        //item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( _access == FTP ) {
        edit->Add(item = new UIW_POP_UP_ITEM("FTP", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10170));
        // item->woFlags |= _access != FTP ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( _access == SSH && connector->_ssh_avail ) {
        edit->Add(item = new UIW_POP_UP_ITEM("SSH", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10149));
        // item->woFlags |= _access != SSH ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( !(_access == PHONE || _access == SUPERLAT || _access == DIRECT) ) {
        edit->Add(item = new UIW_POP_UP_ITEM("TCP/IP", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10173));
        // item->woFlags |= (_access == PHONE || _access == SUPERLAT || _access == DIRECT) ?
        // WOF_NON_SELECTABLE : 0;
        //item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( !(_access == PHONE || _access == SUPERLAT || _access == DIRECT) &&
         (connector->_krb5_avail || connector->_krb4_avail)) {
        edit->Add(item = new UIW_POP_UP_ITEM("Kerberos", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10143));
        // item->woFlags |=  (_access == PHONE || _access == SUPERLAT || _access == DIRECT) ?
        //     WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( !(_access == PHONE || _access == SUPERLAT || _access == DIRECT) &&
         connector->_libeay_avail ) {
        edit->Add(item = new UIW_POP_UP_ITEM("SSL/TLS", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS,
                                             (ZIL_USER_FUNCTION) NULL,
                                             10144));
        // item->woFlags |=  (_access == PHONE || _access == SUPERLAT || _access == DIRECT) ?
        //   WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    edit->Add(item = new UIW_POP_UP_ITEM("Keyboard", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10145));
    edit->Add(item = new UIW_POP_UP_ITEM("Login", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10146));
    edit->Add(item = new UIW_POP_UP_ITEM("Printer", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10147));
    edit->Add(item = new UIW_POP_UP_ITEM("Logs", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                         WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                         10148));
#ifdef WIN32
    edit->Add(item = new UIW_POP_UP_ITEM("GUI", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                           WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                           10154));
#endif /* WIN32 */
    // item->woFlags = WOF_NON_SELECTABLE;
    // item->Information( I_CHANGED_FLAGS, NULL ) ;
    popup->Add(edit);
    generate = new UIW_POP_UP_ITEM("Generate Script File",
                                   MNIF_NO_FLAGS, BTF_NO_3D,
                                   WNF_NO_FLAGS,
                                   (ZIL_USER_FUNCTION) NULL,
                                   0);
    generate->Add(item = new UIW_POP_UP_ITEM("Connect", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                             10039));
    generate->Add(item = new UIW_POP_UP_ITEM("Location", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                             WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                             11035));
    if ( !_template ) {
        generate->Add(item = new UIW_POP_UP_ITEM("Modem", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                                 WNF_NO_FLAGS, (ZIL_USER_FUNCTION) NULL,
                                                 11053));
        // generate->woFlags |= _template ? WOF_NON_SELECTABLE : 0;
        // generate->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    popup->Add(generate);
    K_STATUS * status = K_STATUS::Find(_name);
    if ( status && status->_state != K_STATUS::IDLE ) {
        popup->Add(item = new UIW_POP_UP_ITEM("Hangup", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                              WNF_NO_FLAGS,
                                              (ZIL_USER_FUNCTION) NULL,
                                              10004));
        // item->woFlags = (!status || status->_state == K_STATUS::IDLE) ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }
    if ( _userdefined ) {
        popup->Add(item = new UIW_POP_UP_ITEM("Remove", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                              WNF_NO_FLAGS,
                                              (ZIL_USER_FUNCTION) NULL,
                                              10002));
        // item->woFlags |= !_userdefined ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL );
    }
    if ( !_template ) {
        popup->Add(item = new UIW_POP_UP_ITEM("Shortcut", MNIF_SEND_MESSAGE, BTF_NO_3D,
                                              WNF_NO_FLAGS,
                                              (ZIL_USER_FUNCTION) NULL,
                                              10089));
        // item->woFlags |= _template ? WOF_NON_SELECTABLE : 0;
        // item->Information( I_CHANGED_FLAGS, NULL ) ;
    }

    *windowManager + popup;
}
