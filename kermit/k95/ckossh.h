#ifndef _CKOSSH_H
#define _CKOSSH_H

/* TODO: These all need to be accessed via functions, not as globals. */
extern char * ssh_idf[32];              /* identity files */
extern int ssh_idf_n;

extern int ssh_sock;                    /* SSH socket */

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

/* Setters and getters for the various "set ssh" options. set_ssh_sparam takes
 * a copy of the supplied string rather than taking ownership of it.*/
_PROTOTYP(int ssh_set_iparam,(int param, int value));
_PROTOTYP(int ssh_get_iparam,(int param));
_PROTOTYP(int ssh_set_sparam,(int param, const char* value));
_PROTOTYP(const char* ssh_get_sparam,(int param));

/* Getters for various global values within C-Kermit */
_PROTOTYP(const char* ssh_get_uid,(VOID));
_PROTOTYP(const char* ssh_get_pw,(VOID));
_PROTOTYP(int ssh_get_nodelay_enabled,(VOID));
_PROTOTYP(void get_current_terminal_dimensions,(int*,int*));
_PROTOTYP(const char* get_current_terminal_type,());

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
_PROTOTYP(int ssh_snaws, (void));

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
_PROTOTYP(const ssh_port_forward_t* ssh_fwd_get_ports,());

#ifdef SSHTEST
_PROTOTYP(int sshkey_v1_change_comment,(char * filename, char * comment, char * pp));
#endif /* SSHTEST */
/*_PROTOTYP(char * sshkey_default_file,(int));*/
_PROTOTYP(void ssh_v2_rekey,(void));

/* SSH Agent */
_PROTOTYP(int ssh_agent_delete_file,(const char *filename));
_PROTOTYP(int ssh_agent_delete_all, (void));
_PROTOTYP(int ssh_agent_add_file, (const char *filename));
_PROTOTYP(int ssh_agent_list_identities,(int do_fp));

/* Information */
_PROTOTYP(const char * ssh_proto_ver,(void));
_PROTOTYP(const char * ssh_impl_ver,(void));
_PROTOTYP(const char * ssh_dll_name,(void));
_PROTOTYP(const char * ssh_dll_ver,(void));
_PROTOTYP(int ssh_avail,(VOID));
_PROTOTYP(void ssh_unload,(VOID));

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

_PROTOTYP(int ssh_feature_supported,(int feature_id));


#ifdef SSH_DLL
typedef struct  {

    /* Version 1 */
    int version;
    void (*p_install_funcs)(const char*, const void*);
    void (*p_get_current_terminal_dimensions)(int* rows, int* cols);
    const char* (*p_get_current_terminal_type)();
    const char* (*p_ssh_get_uid)();
    const char* (*p_ssh_get_pw)();
    int (*p_ssh_get_nodelay_enabled)();
    int (*p_dodebug)(int,char *,char *,CK_OFF_T);
    int (*p_vscrnprintf)(char *, ...);
    int (*p_uq_txt)(char *,char *,int,char **,char *,int,char *,int);
    int (*p_uq_mtxt) (char *,char **,int,struct txtbox[]);
    int (*p_uq_ok)(char *,char *,int,char **,int);
    int (*p_uq_file)(char *,char *,int,char **,char *,char *,int);
	int (*p_zmkdir)(char *);
	int (*p_ckmakxmsg)(char * buf, int len, char *s1, char *s2, char *s3,
            char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
            char *s10, char *s11, char *s12);
	char* (*p_whoami)();
	char* (*p_GetHomePath)();
	char* (*p_GetHomeDrive)();
    int (*p_ckstrncpy)(char * dest, const char * src, int len);
    int (*p_debug_logging)();
} ssh_init_parameters_t;

/*
 *  k95sshg.dll     libssh + gssapi (kerberos).
 *  k95ssh.dll      libssh
 *  k95sshgx.dll    libssh + gssapi (kerberos) patched for Windows XP
 *  k95sshx.dll     libssh patched for Windows XP
 *  k95plink.dll    PuTTYs plink as a dll - someday. perhaps.
 */
#define SSH_AUTO_DLLS "k95sshg.dll;k95ssh.dll;k95sshgx.dll;k95sshx.dll;k95plink.dll"

_PROTOTYP(int ssh_dll_load,(const char* dll_names, int quiet));
_PROTOTYP(int ssh_dll_unload,(int quiet));

#endif /* SSH_DLL */

#endif /* _CKOSSH_H */
