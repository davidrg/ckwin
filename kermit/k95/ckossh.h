#ifndef _CKOSSH_H
#define _CKOSSH_H

#ifndef SSH_PF_T
#define SSH_PF_T
/* Note: This also exists in ckolsshs.h */
#define SSH_PORT_FORWARD_NULL       0
#define SSH_PORT_FORWARD_LOCAL      1
#define SSH_PORT_FORWARD_REMOTE     2
#define SSH_PORT_FORWARD_INVALID   99       /* Invalid entry / free for re-use */
typedef struct ssh_port_forward {
    /* Type of port forward. One of:
     *  SSH_PORT_FORWARD_LOCAL      Local (Direct) port forward
     *  SSH_PORT_FORWARD_REMOTE     Remote (Reverse) port forward
     *  SSH_PORT_FORWARD_INVALID    Empty list entry. Can be overwritten with a
     *                              new entry. Should otherwise be skipped over
     *  SSH_PORT_FORWARD_NULL       End of list marker
     *  */
    int type;

    /* For remote (reverse) forwards: address on the server to bind to.
     * Use NULL to bind ot all addresses.
     *
     * For local (direct) forwards: the address/host name for the servers
     * logs.
     * */
    char* address;

    /* This is the port that listens for new connections. Its either on the
     * local host (Local/Direct forwarding) or the remote host (Remote/Reverse
     * forwarding.
     *
     * For Remote/Reverse forwarding, you can set this to 0 to allow the server
     * to choose the port.
     * */
    int port;

    /* This is the host and port that connections will be made to when
     * something makes a connection to port 1. For Local (Direct) forwarding,
     * it's something accessible to the remote host, and for Remote (Reverse)
     * forwarding it's something accessible to the local host. */
    char* hostname;
    int host_port;
} ssh_port_forward_t;
#endif /* SSH_PF_T */

#define SSH_ERR_TOO_MANY_FORWARDS 1
#define SSH_ERR_DUPLICATE_PORT_FWD 2

/* Integer parameters. Set with ssh_set_iparam, get with ssh_get_iparam */
#define SSH_IPARAM_AFW      1       /* agent forwarding */
#define SSH_IPARAM_XFW      2       /* x11 forwarding   */
#define SSH_IPARAM_PRP      3       /* privileged ports */
#define SSH_IPARAM_CMP      4       /* compression */
#define SSH_IPARAM_CAS      5       /* command as subsys */
#define SSH_IPARAM_SHH      6       /* quiet       */
#define SSH_IPARAM_VER      7       /* protocol version (auto,1,2) */
#define SSH_IPARAM_VRB      8       /* Report errors */
#define SSH_IPARAM_CHKIP    9       /* SSH Check Host IP flag */
#define SSH_IPARAM_GWP      10      /* gateway ports */
#define SSH_IPARAM_DYF      11      /* dynamic forwarding */
#define SSH_IPARAM_GSD      12      /* gssapi delegate credentials */
#define SSH_IPARAM_K4TGT    13      /* k4 tgt passing */
#define SSH_IPARAM_K5TGT    14      /* k5 tgt passing */
#define SSH_IPARAM_SHK      15      /* Strict host key (no, yes, ask) */
#define SSH_IPARAM_2_ARK    16      /* Auto re-key */
#define SSH_IPARAM_CFG      17      /* use OpenSSH config? */
#define SSH_IPARAM_GKX      18      /* gssapi key exchange */
#define SSH_IPARAM_K5_IS_K4 19      /* some SSH v1 use same codes */
#define SSH_IPARAM_HBT      20      /* heartbeat (seconds) */

/* String parameters. Set with ssh_set_sparam, get with ssh_get_sparam */
#define SSH_SPARAM_1_CIF    1       /* v1 cipher */
#define SSH_SPARAM_2_CIF    2       /* v2 cipher list */
#define SSH_SPARAM_2_MAC    3       /* v2 mac list */
#define SSH_SPARAM_2_AUTH   4       /* v2 authentication list */
#define SSH_SPARAM_2_HKA    5       /* Host Key Algorithms */
#define SSH_SPARAM_HST      6       /* hostname */
#define SSH_SPARAM_PRT      7       /* port/service */
#define SSH_SPARAM_CMD      8       /* command to execute */
#define SSH_SPARAM_XAL      9       /* xauth-location */
#define SSH_SPARAM_1_GNH    10      /* v1 global known hosts file */
#define SSH_SPARAM_1_UNH    11      /* v1 user known hosts file */
#define SSH_SPARAM_2_GNH    12      /* v2 global known hosts file */
#define SSH_SPARAM_2_UNH    13      /* v2 user known hosts file */
#define SSH_SPARAM_2_KEX    14      /* Key Exchange Methods */
#define SSH_SPARAM_PXC      15      /* Proxy command */
#define SSH_SPARAM_DIR      16      /* SSH Directory */
#define SSH_SPARAM_AGENTLOC 17      /* SSH Agent location */

/* Setters and getters for the various "set ssh" options. set_ssh_sparam takes
 * a copy of the supplied string rather than taking ownership of it.*/
_PROTOTYP(int ssh_set_iparam,(int param, int value));
_PROTOTYP(int ssh_get_iparam,(int param));
_PROTOTYP(int ssh_set_sparam,(int param, const char* value));
_PROTOTYP(const char* ssh_get_sparam,(int param));
_PROTOTYP(int ssh_set_identity_files,(const char** identity_files));
_PROTOTYP(int ssh_get_socket,(VOID));

/* Getters for various global values within C-Kermit */
_PROTOTYP(const char* ssh_get_uid,(VOID));
_PROTOTYP(const char* ssh_get_pw,(VOID));
_PROTOTYP(int ssh_get_nodelay_enabled,(VOID));
_PROTOTYP(void get_current_terminal_dimensions,(int*,int*));
_PROTOTYP(const char* get_current_terminal_type,(VOID));

/* SSH Interface */
_PROTOTYP(int ssh_open,(VOID));
_PROTOTYP(int ssh_clos,(VOID));
_PROTOTYP(int ssh_tchk,(VOID));
_PROTOTYP(int ssh_flui,(VOID));
_PROTOTYP(int ssh_break,(VOID));
_PROTOTYP(int ssh_inc,(int));
_PROTOTYP(int ssh_xin,(int,char *));
_PROTOTYP(int ssh_toc,(int));
_PROTOTYP(int ssh_tol,(char *,int));
_PROTOTYP(int ssh_snaws, (VOID));

/* SSH Key management */
_PROTOTYP(int sshkey_create,(char * filename, int bits, char * pp,
                             int type, char * cmd_comment));
_PROTOTYP(int sshkey_display_fingerprint,(char * filename, int babble));
_PROTOTYP(int sshkey_display_public,(char * filename, char *identity_passphrase));
_PROTOTYP(int sshkey_display_public_as_ssh2,(char * filename,char *identity_passphrase));
_PROTOTYP(int sshkey_change_passphrase,(char * filename, char * oldpp, char * newpp));

/* Port forwarding configuration */
_PROTOTYP(int ssh_fwd_remote_port, (char* address, int port, char * host, int host_port, BOOL apply));
_PROTOTYP(int ssh_fwd_local_port,(char* address, int port,char * host, int host_port, BOOL apply));
_PROTOTYP(int ssh_fwd_clear_remote_ports,(BOOL apply));
_PROTOTYP(int ssh_fwd_clear_local_ports,(BOOL apply));
_PROTOTYP(int ssh_fwd_remove_remote_port,(int port, BOOL apply));
_PROTOTYP(int ssh_fwd_remove_local_port,(int port, BOOL apply));
_PROTOTYP(const ssh_port_forward_t* ssh_fwd_get_ports,(VOID));

#ifdef SSHTEST
_PROTOTYP(int sshkey_v1_change_comment,(char * filename, char * comment, char * pp));
#endif /* SSHTEST */
/*_PROTOTYP(char * sshkey_default_file,(int));*/
_PROTOTYP(void ssh_v2_rekey,(VOID));

/* SSH Agent */
_PROTOTYP(int ssh_agent_delete_file,(const char *filename));
_PROTOTYP(int ssh_agent_delete_all, (VOID));
_PROTOTYP(int ssh_agent_add_file, (const char *filename));
_PROTOTYP(int ssh_agent_list_identities,(int do_fp));

/* Information */
_PROTOTYP(const char * ssh_proto_ver,(VOID));
_PROTOTYP(const char * ssh_impl_ver,(VOID));
_PROTOTYP(const char * ssh_dll_name,(VOID));
_PROTOTYP(const char * ssh_dll_ver,(VOID));
_PROTOTYP(int ssh_avail,(VOID));
_PROTOTYP(void ssh_unload,(VOID));
_PROTOTYP(const char ** ssh_get_set_help,(VOID));
_PROTOTYP(const char ** ssh_get_help,(VOID));

typedef struct {
    int rc;
    struct keytab* ktab;
    int ktab_len;
} ktab_ret;

#define SSH_KTAB_V2_AUT         1
#define SSH_KTAB_V2_CIPHERS     2
#define SSH_KTAB_V2_MACS        3
#define SSH_KTAB_HKA            4
#define SSH_KTAB_KEX            5
#define SSH_KTAB_V1_CIPHERS     6

_PROTOTYP(ktab_ret ssh_get_keytab,(int keytab_id));

#define SSH_FEAT_SSH_V1         1
#define SSH_FEAT_PORT_FWD       2
#define SSH_FEAT_X11_FWD        3
#define SSH_FEAT_PROXY_CMD      4
#define SSH_FEAT_OPENSSH_CONF   5
#define SSH_FEAT_GSSAPI_DELEGAT 6
#define SSH_FEAT_GSSAPI_KEYEX   7
#define SSH_FEAT_ADV_KERBEROS4  8
#define SSH_FEAT_ADV_KERBEROS5  9
#define SSH_FEAT_AGENT_FWD      10
#define SSH_FEAT_AGENT_MGMT     11
#define SSH_FEAT_KEY_MGMT       12
#define SSH_FEAT_REKEY_MANUAL   13
#define SSH_FEAT_REKEY_AUTO     14
#define SSH_FEAT_FROM_PRIV_PRT  15
#define SSH_FEAT_DYN_PORT_FWD   16
#define SSH_FEAT_X11_XAUTH      17
#define SSH_FEAT_AGENT_LOC      18

_PROTOTYP(int ssh_feature_supported,(int feature_id));

_PROTOTYP(void ssh_initialise,(VOID));

#ifndef SOCKET
/* On OS/2, SOCKET is just int. */
#define SOCKET int
#endif /* SOCKET */
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif /* INVALID_SOCKET */

#ifdef SSH_DLL

/*
 * below is setup of calling convention used between Kermit and SSH DLL
 *
 * 64-bit Windows use calling convention defined by ABI (default)
 * 32-bit systems has not specified ABI therefore it can use any
 * for all 32-bit systems it uses cdecl for better interoperatibility
 * now you can use DLL created by any compiler togather with application
 * compiled by another compiler
 */
#ifndef _WIN64
#define SSH_DLL_CALLCONV    cdecl
#endif /* _WIN64 */

#ifdef SSH_DLL_CALLCONV
#define CKSSHAPI            SSH_DLL_CALLCONV
#define CKSSHDLLENTRY       SSH_DLL_CALLCONV
#else
#define CKSSHAPI
#define CKSSHDLLENTRY
#endif /* CKSSHAPI */

/* prototypes for all callback function provided by Kermit */
typedef void CKSSHAPI install_dllfunc_callback(const char*, const void*);
typedef void CKSSHAPI get_current_terminal_dimensions_callback(int* rows, int* cols);
typedef const char* CKSSHAPI get_current_terminal_type_callback(void);
typedef const char* CKSSHAPI ssh_get_uid_callback(void);
typedef const char* CKSSHAPI ssh_get_pw_callback(void);
typedef int CKSSHAPI ssh_get_nodelay_enabled_callback(void);
typedef SOCKET CKSSHAPI ssh_open_socket_callback(char* host, char* port);
typedef int CKSSHAPI dodebug_callback(int,char *,char *,CK_OFF_T);
typedef int CKSSHAPI scrnprint_callback(const char *str);
typedef int CKSSHAPI uq_txt_callback(char *,char *,int,char **,char *,int,char *,int);
typedef int CKSSHAPI uq_mtxt_callback(char *,char **,int,struct txtbox[]);
typedef int CKSSHAPI uq_ok_callback(char *,char *,int,char **,int);
typedef int CKSSHAPI uq_file_callback(char *,char *,int,char **,char *,char *,int);
typedef int CKSSHAPI zmkdir_callback(char *);
typedef int CKSSHAPI ckmakxmsg_callback(char * buf, int len, char *s1, char *s2, char *s3,
            char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
            char *s10, char *s11, char *s12);
typedef char* CKSSHAPI whoami_callback(void);
typedef char* CKSSHAPI GetAppData_callback(int);
typedef char* CKSSHAPI GetHomePath_callback(void);
typedef char* CKSSHAPI GetHomeDrive_callback(void);
typedef int CKSSHAPI ckstrncpy_callback(char * dest, const char * src, int len);
typedef int CKSSHAPI debug_logging_callback(void);
typedef unsigned char* CKSSHAPI get_display_callback(void);
typedef int CKSSHAPI parse_displayname_callback(char *displayname, int *familyp,
            char **hostp, int *dpynump, int *scrnump, char **restp);

typedef struct  {

    /* Version 1 */
    int version;
    install_dllfunc_callback *callbackp_install_dllfunc;
    get_current_terminal_dimensions_callback *callbackp_get_current_terminal_dimensions;
    get_current_terminal_type_callback *callbackp_get_current_terminal_type;
    ssh_get_uid_callback *callbackp_ssh_get_uid;
    ssh_get_pw_callback *callbackp_ssh_get_pw;
    ssh_get_nodelay_enabled_callback *callbackp_ssh_get_nodelay_enabled;
    ssh_open_socket_callback *callbackp_ssh_open_socket;
    dodebug_callback *callbackp_dodebug;
    scrnprint_callback *callbackp_scrnprint;
    uq_txt_callback *callbackp_uq_txt;
    uq_mtxt_callback *callbackp_uq_mtxt;
    uq_ok_callback *callbackp_uq_ok;
    uq_file_callback *callbackp_uq_file;
    zmkdir_callback *callbackp_zmkdir;
    ckmakxmsg_callback *callbackp_ckmakxmsg;
    whoami_callback *callbackp_whoami;
    GetAppData_callback *callbackp_GetAppData;
    GetHomePath_callback *callbackp_GetHomePath;
    GetHomeDrive_callback *callbackp_GetHomeDrive;
    ckstrncpy_callback *callbackp_ckstrncpy;
    debug_logging_callback *callbackp_debug_logging;

    /* Returns a statically allocated string containing the currently
     * configured X11 display
     */
    get_display_callback *callbackp_get_display;

    /* Utility function for parsing the display name. Result is returned
     * via:
     *   *familyp   - protocol family (FamilyInternet, FamilyLocal, FamilyDECnet)
     *   **hostp    - host string
     *   *dpynump   - Display number
     *   *scrnump   - Screen number
     *   **restp    - Anything else at the end
     */
    parse_displayname_callback *callbackp_parse_displayname;
} ssh_dll_init_data;

/* prototypes for all functions provided by DLL */
typedef int CKSSHAPI ssh_set_iparam_dllfunc(int, int);
typedef int CKSSHAPI ssh_get_iparam_dllfunc(int);
typedef int CKSSHAPI ssh_set_sparam_dllfunc(int, const char*);
typedef const char* CKSSHAPI ssh_get_sparam_dllfunc(int);
typedef int CKSSHAPI ssh_set_identity_files_dllfunc(const char**);
typedef int CKSSHAPI ssh_get_socket_dllfunc(void);
typedef int CKSSHAPI ssh_open_dllfunc(void);
typedef int CKSSHAPI ssh_clos_dllfunc(void);
typedef int CKSSHAPI ssh_tchk_dllfunc(void);
typedef int CKSSHAPI ssh_flui_dllfunc(void);
typedef int CKSSHAPI ssh_break_dllfunc(void);
typedef int CKSSHAPI ssh_inc_dllfunc(int);
typedef int CKSSHAPI ssh_xin_dllfunc(int, char*);
typedef int CKSSHAPI ssh_toc_dllfunc(int);
typedef int CKSSHAPI ssh_tol_dllfunc(char*,int);
typedef int CKSSHAPI ssh_snaws_dllfunc(void);
typedef const char* CKSSHAPI ssh_proto_ver_dllfunc(void);
typedef const char* CKSSHAPI ssh_impl_ver_dllfunc(void);
typedef int CKSSHAPI sshkey_create_dllfunc(char *, int, char *, int, char *);
typedef int CKSSHAPI sshkey_display_fingerprint_dllfunc(char *, int);
typedef int CKSSHAPI sshkey_display_public_dllfunc(char *, char *);
typedef int CKSSHAPI sshkey_display_public_as_ssh2_dllfunc(char *,char *);
typedef int CKSSHAPI sshkey_change_passphrase_dllfunc(char *, char *, char *);
typedef int CKSSHAPI ssh_fwd_remote_port_dllfunc(char*, int, char *, int, BOOL);
typedef int CKSSHAPI ssh_fwd_local_port_dllfunc(char*, int,char *,int, BOOL);
typedef int CKSSHAPI ssh_fwd_clear_local_ports_dllfunc(BOOL);
typedef int CKSSHAPI ssh_fwd_clear_remote_ports_dllfunc(BOOL);
typedef int CKSSHAPI ssh_fwd_remove_remote_port_dllfunc(int, BOOL);
typedef int CKSSHAPI ssh_fwd_remove_local_port_dllfunc(int, BOOL);
typedef const ssh_port_forward_t* CKSSHAPI ssh_fwd_get_ports_dllfunc(void);
typedef int CKSSHAPI sshkey_v1_change_comment_dllfunc(char *, char *, char *);
typedef char * CKSSHAPI sshkey_default_file_dllfunc(int);
typedef void CKSSHAPI ssh_v2_rekey_dllfunc(void);
typedef int CKSSHAPI ssh_agent_delete_file_dllfunc(const char *);
typedef int CKSSHAPI ssh_agent_delete_all_dllfunc(void);
typedef int CKSSHAPI ssh_agent_add_file_dllfunc(const char*);
typedef int CKSSHAPI ssh_agent_list_identities_dllfunc(int);
typedef void CKSSHAPI ssh_unload_dllfunc(void);
typedef const char* CKSSHAPI ssh_dll_ver_dllfunc(void);
typedef ktab_ret CKSSHAPI ssh_get_keytab_dllfunc(int keytab_id);
typedef int CKSSHAPI ssh_feature_supported_dllfunc(int feature_id);
typedef const char** CKSSHAPI ssh_get_set_help_dllfunc(void);
typedef const char** CKSSHAPI ssh_get_help_dllfunc(void);

/*
 *  k95sshg.dll     libssh + gssapi (kerberos).
 *  k95ssh.dll      libssh
 *  k95sshgx.dll    libssh + gssapi (kerberos) patched for Windows XP
 *  k95sshx.dll     libssh patched for Windows XP
 *  k95plink.dll    PuTTYs plink as a dll - someday. perhaps.
 */
#define SSH_AUTO_DLLS "k95sshg.dll;k95ssh.dll;k95sshgx.dll;k95sshx.dll;k95plink.dll"

extern int ssh_dll_load(const char* dll_names, int quiet);
extern int ssh_dll_unload(int quiet);

#endif /* SSH_DLL */

#endif /* _CKOSSH_H */
