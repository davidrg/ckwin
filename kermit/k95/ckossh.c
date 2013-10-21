char *cksshv = "SSH support, 8.0.051,  26 Sep 2005";
/*
  C K O S S H . C --  OpenSSH Interface for Kermit 95

  Copyright (C) 2001-2005,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Portions Copyright (c) 2000-2002 Markus Friedl.
  Portions Copyright (c) 1999 Niels Provos.
  Portions Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
    All rights reserved

  Includes work by Niels Provos <provos@citi.umich.edu> to work with OpenSSL.

*/

#ifdef TO_DO_LIST
  . Kerberos 4 and 5 Auto-get/auto-destroy support

  . listen to SET TCP REVERSE-DNS-LOOKUPS

  . listen to SET TCP DNS-SRV-RECORDS

  . make SET TELNET ENV DISPLAY more portable or SSH equivalent

  . Implement service name processing in apply_kermit_settings()

  . add \f...()'s to process the .ssh\... key files (pub, priv, host)

  . document using Kermit file transfer to upload a public key identity
    file and append it to the authorized_keys file on the host

  . all strings are to be tranmitted as UTF-8. (username, passphrase, ...)
    need to examine what conversions are necessary.

  . do not close the SSH connection when the terminal session stops
    leave it open until an SSH CLOSE command is given.  This will allow
    port forwarding and additional terminal sessions to be created on top
    of the SSH connection.

  . Incoming SSH connections SET HOST *
#endif /* TO_DO_LIST */

#include "ckcdeb.h"

#ifdef CK_SSL
#ifdef SSHBUILTIN
#ifndef OPENSSL_NO_KRB5
#define OPENSSL_NO_KRB5
#endif

#include "ckuusr.h"
#include "ckcnet.h"
#ifdef OS2                              /* OS/2 with TCP/IP */
#ifdef NT
#define BSDSELECT
#else /* NT */
#define IBMSELECT
#endif /* NT */
#endif /* OS2 */

#include "ssh/includes.h"
#include <fcntl.h>
#include <stdio.h>
#ifdef KRB4
#define  des_cblock Block
#define  const_des_cblock const Block
#define  des_key_schedule Schedule
#ifdef KRB524
#ifdef NT
#define _WINDOWS
#endif /* NT */
#include "kerberosIV/krb.h"
#ifndef OS2
_PROTOTYP(const char * krb_get_err_text_entry, (int));
#endif /* OS2 */
#else /* KRB524 */
#ifdef SOLARIS
#ifndef sun
/* for some reason the Makefile entries for the Solaris systems have -Usun */
#define sun
#endif /* sun */
#endif /* SOLARIS */
#include "krb.h"
#define krb_get_err_text_entry krb_get_err_text
#endif /* KRB524 */
#else /* KRB4 */
#ifdef CK_SSL
#define  des_cblock Block
#define  const_des_cblock const Block
#define  des_key_schedule Schedule
#endif /* CK_SSL */
#endif /* KRB4 */
#ifdef KRB5
#include <krb5.h>
#define GSSAPI
#endif /* KRB5 */
#ifdef AFS
#include <kafs.h>
#include "ssh/radix.h"
#endif
#ifndef HEADER_DES_H
#define HEADER_DES_H
#endif /* HEADER_DES_H */
#include "ck_ssl.h"
#include "ssh/ssh.h"
#include "ssh/ssh1.h"
#include "ssh/ssh2.h"
#include "ssh/compat.h"
#include "ssh/cipher.h"
#include "ssh/packet.h"
#include "ssh/buffer.h"
#include "ssh/bufaux.h"
#include "ssh/uidswap.h"
#include "ssh/channels.h"
#include "ssh/key.h"
#include "ssh/authfd.h"
#include "ssh/authfile.h"
#include "ssh/pathnames.h"
#include "ssh/clientloop.h"
#include "ssh/log.h"
#include "ssh/readconf.h"
#include "ssh/sshconnect.h"
#include "ssh/tildexpand.h"
#include "ssh/dispatch.h"
#include "ssh/misc.h"
#include "ssh/kex.h"
#include "ssh/mac.h"
#include "ssh/match.h"
#ifndef OS2
#include "ssh/sshtty.h"
#endif /* OS2 */
#include "ssh/version.h"
#include "ssh/myproposal.h"
#include "ssh/hostfile.h"

#include "ssh/rsa.h"
#include "ssh/mpaux.h"
#include "ssh/uidswap.h"
#include "ssh/authfile.h"
#include "ssh/canohost.h"
#ifdef GSSAPI
#include "ssh/ssh-gss.h"
#endif /* GSSAPI */
#ifdef CK_SRP
#include "ssh/srp.h"
#endif /* CK_SRP */
#ifdef SMARTCARD
#include <sectok.h>
#include <openssl/engine.h>
#include "ssh/scard.h"
#endif

#include "ckcker.h"
#include "ckocon.h"
#include "ckctel.h"
#ifndef NO_KERBEROS
#define MAP_KRB4
#define MAP_KRB5
#define MAP_GSSAPI
#ifdef CK_ENCRYPTION
#define MAP_DES
#endif /* CK_ENCRYPTION */
#endif /* NO_KERBEROS */
#include "ckoath.h"
#include "ckossh.h"

#ifdef CK_SOCKS                         /* SOCKS Internet relay package */
#ifdef CK_SOCKS5                        /* SOCKS 5 */
#define accept  SOCKSaccept
#define bind    SOCKSbind
#define connect SOCKSconnect
#define getsockname SOCKSgetsockname
#define listen SOCKSlisten
#else  /* Not SOCKS 5 */
#define accept  Raccept
#define bind    Rbind
#define connect Rconnect
#define getsockname Rgetsockname
#define listen Rlisten
#endif /* CK_SOCKS5 */
#endif /* CK_SOCKS */

#ifndef NOHTTP
extern char * tcp_http_proxy;           /* Name[:port] of http proxy server */
extern int    tcp_http_proxy_errno;
extern char * tcp_http_proxy_user;
extern char * tcp_http_proxy_pwd;
extern char * tcp_http_proxy_agent;
#define HTTPCPYL 1024
static char proxyhost[HTTPCPYL];
#endif /* NOHTTP */
#define ERRBUFSZ 1024
static char errbuf[ERRBUFSZ];

#ifdef NT
extern int WSASafeToCancel;
extern int win95selectbug;             /* For TCP/IP stacks whose select() */
/* always fails on write requests such as Cisco and Quarterdeck */
#define stricmp _stricmp
#endif /* NT */

extern int quiet, filecase;

/* Flag indicating whether IPv4 or IPv6.  This can be set on the command line.
   Default value is AF_UNSPEC means both IPv4 and IPv6. */
#ifdef IPV4_DEFAULT
int IPv4or6 = AF_INET;
#else
int IPv4or6 = AF_UNSPEC;
#endif

/* Save the socket handle */
int ssh_sock = -1;

/* Flag indicating whether debug mode is on.  This can be set on the command line. */
int debug_flag = 0;

/* Flag indicating whether a tty should be allocated */
int tty_flag = 0;
int no_tty_flag = 0;
int force_tty_flag = 0;

/* don't exec a shell */
int no_shell_flag = 0;

/*
 * Flag indicating that nothing should be read from stdin.  This can be set
 * on the command line.
 */
int stdin_null_flag = 0;

/*
 * Flag indicating that ssh should fork after authentication.  This is useful
 * so that the passphrase can be entered manually, and then ssh goes to the
 * background.
 */
int fork_after_authentication_flag = 0;

/*
 * General data structure for command line options and options configurable
 * in configuration files.  See readconf.h.
 */
static Options options;
static int opt_init = 0;

/*
 * Name of the host we are connecting to.  This is the name given on the
 * command line, or the HostName specified for the user-supplied name in a
 * configuration file.
 */
static char *host = 0;

/* socket address the host resolves to */
struct sockaddr_storage hostaddr;

/*
 * Flag to indicate that we have received a window change signal which has
 * not yet been processed.  This will cause a message indicating the new
 * window size to be sent to the server a little later.  This is volatile
 * because this is updated in a signal handler.
 */
static volatile int received_window_change_signal = 0;
static volatile int received_signal = 0;
static int sent_initial_ws = 0;
static int ssh_height = 0;
static int ssh_width = 0;

/* Private host keys. */
Sensitive sensitive_data;

/* command to be executed */
static Buffer command;
static int buf_init = 0;

/* Should we execute a command or invoke a subsystem? */
static int subsystem_flag = 0;

/* Number of replies received for global requests */
static int client_global_request_id = 0;

/* Flag indicating whether the user\'s terminal is in non-blocking mode. */
static int in_non_blocking_mode = 0;

/* Common data for the client loop code. */
static int quit_pending = 0;    /* Set to non-zero to quit the client loop. */
static int escape_char  = 0;    /* Escape character. */
static int escape_pending = 0;  /* Last character was the escape character */
static int last_was_cr  = 0;    /* Last character was a newline. */
static int exit_status  = 0;    /* Used to store the exit status of the command. */
static int stdin_eof = 0;       /* EOF has been encountered on standard error. */
static Buffer stdin_buffer;     /* Buffer for stdin data. */
static Buffer stdout_buffer;    /* Buffer for stdout data. */
static Buffer stderr_buffer;    /* Buffer for stderr data. */
static u_long stdin_bytes = 0, stdout_bytes = 0, stderr_bytes = 0;
static u_int buffer_high= 0;    /* Soft max buffer size. */
static int connection_in = 0;   /* Connection to server (input). */
static int connection_out = 0;  /* Connection to server (output). */
static int need_rekeying = 0;   /* Set to non-zero if rekeying is requested. */
static int session_closed = 0;  /* In SSH2: login session closed. */

static void     client_init_dispatch(void);
int     session_ident = -1;

char *client_version_string = NULL;
char *server_version_string = NULL;

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
extern int tcp_nodelay;                    /* Nagle algorithm TCP_NODELAY */
#endif /* TCP_NODELAY */

#ifdef SO_DONTROUTE
extern int tcp_dontroute;
#endif /* SO_DONTROUTE */

#ifdef SO_LINGER
extern int tcp_linger;                     /* SO_LINGER */
extern int tcp_linger_tmo;                 /* SO_LINGER timeout */
#endif /* SO_LINGER */

#ifdef SO_SNDBUF
extern int tcp_sendbuf;
#endif /* SO_SNDBUF */

#ifdef SO_RCVBUF
extern int tcp_recvbuf;
#endif /* SO_RCVBUF */

#ifdef SO_KEEPALIVE
extern int tcp_keepalive;
#endif /* SO_KEEPALIVE */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

#ifdef CK_DNS_SRV
static struct sockaddr * ssh_dns_addrs = NULL;
static int ssh_dns_naddrs = 0;
#endif /* CK_DNS_SRV */

extern char pwbuf[];
extern int  pwflg, pwcrypt;

/*
 * SSH2 key exchange
 */

u_char *session_id2 = NULL;
int session_id2_len = 0;

char *xxx_host;
struct sockaddr *xxx_hostaddr;

Kex *xxx_kex = NULL;

#ifdef OS2
extern int CreateSocketPair(SOCKET *pair);
SOCKET inPair[2] = { INVALID_SOCKET, INVALID_SOCKET };
SOCKET outPair[2] = { INVALID_SOCKET, INVALID_SOCKET };
SOCKET sock_stdin = INVALID_SOCKET;
SOCKET sock_stdout = INVALID_SOCKET;
SOCKET sock_stderr = INVALID_SOCKET;
#define SOCK_OUT inPair[1]
#define SOCK_IN  outPair[0]

/* BEGIN SSHDLL.C */
static int open_connection=0;
extern char uidbuf[];

static HANDLE hClientLoopThread=NULL;
static DWORD  ClientLoopThreadID=0;
static int    clientloopID = 0;

static DWORD WINAPI
ClientLoopThread(void * dummy) {
    int rc, id = *((int *)dummy);
    rc = client_loop(1, options.escape_char, id);

    /* WaitForSingleObject(hClientLoopThread,INFINITE); */
    if ( hClientLoopThread ) {
        CloseHandle(hClientLoopThread);
        hClientLoopThread = NULL;
        ClientLoopThreadID = 0;
    }
    return(rc);
}
#endif /* OS2 */

char *
getterm()
{
    static char ttn[64];
    int i;
#ifndef NOTERM
    extern char * tn_term;
#ifdef OS2
    extern int tt_type;
    extern int tt_type, max_tt;
    extern struct tt_info_rec tt_info[];
    extern char ttname[];

    if (!tn_term) {
        if (tt_type >= 0 && tt_type <= max_tt) {
            ckstrncpy(ttn,tt_info[tt_type].x_name,sizeof(ttn));
        } else
          ttn[0] = '\0';
    }
#endif /* OS2 */

    if (tn_term) {                      /* Terminal type override? */
        debug(F110,"ssh_getterm",tn_term,0);
        if (*tn_term)
            ckstrncpy(ttn,tn_term,sizeof(ttn));
    } else
        debug(F100,"ssh_getterm no term override","",0);
#endif /* NOTERM */

#ifndef OS2
#ifndef datageneral
    if (!ttn[0]) {                         /* If no override, */
        char * env = getenv("TERM");      /* get it from the environment. */
        if ( env )
            ckstrncpy(ttn,env,sizeof(ttn));
    }
#endif /* datageneral */
#endif /* OS2 */
    if (!ttn[0])
      ckstrncpy(ttn,"UNKNOWN",sizeof(ttn));
    cklower(ttn);
    return(ttn);
}

/* _getwinsize() -- Get console size (like ioctl() with TIOCGWINSZ)

   /// LIMITATION ///
   xxx, x and y pixels of returned structure are not correct
   (calculated as fixed size 8x16 dot font).
 */
int
_getwinsize( struct winsize *winsize )
{
    extern int tt_status[];
    winsize->ws_col = VscrnGetWidth(VTERM);
    winsize->ws_row = VscrnGetHeight(VTERM) - (tt_status[VTERM]?1:0);
    winsize->ws_xpixel = winsize->ws_col * 8; /* xxx */
    winsize->ws_ypixel = winsize->ws_row * 16; /* xxx */
    return 0;
}

/*
 * Expands tildes in the file name.  Returns data allocated by malloc.
 * Warning: this calls getpw*.
 */
static char *
tilde_expand_filename(const char *filename, uid_t my_uid)
{
    const char *cp;
    const char *home;
    u_int userlen;
    char *expanded;
    char user[100];
    char userdir[MAXPATHLEN+1];
    int len;

    /* Return immediately if no tilde. */
    if (filename[0] != '~')
        return strdup(filename);

    /* Skip the tilde. */
    filename++;

    /* Find where the username ends. */
    cp = strchr(filename, '/');
    if (cp)
        userlen = cp - filename;        /* Something after username. */
    else
        userlen = strlen(filename);     /* Nothing after username. */

    if (userlen > sizeof(user) - 1) {
        fatal("User name after tilde too long.");
        return NULL;
    }

    if (userlen)
        memcpy(user, filename, userlen);
    user[userlen] = 0;

    if (userlen == 0 || !ckstrcmp(user,(char *)GetLocalUser(),-1,filecase))
        home = zhome();
    else {
        char * homedrive = (char *)GetHomeDrive();
        /* Tilde refers to someone elses home directory. */

        snprintf(userdir,MAXPATHLEN+1, "%s:/Documents and Settings/%s/",
                  homedrive[0]?homedrive:"C:",(char *)GetLocalUser());
        home = userdir;
    }

    /* If referring to someones home directory, return it now. */
    if (!cp) {
        /* Only home directory specified */
        return strdup(home);
    }
    /* Build a path combining the specified directory and path. */
    len = strlen(home) + strlen(cp + 1) + 2;
    if (len > MAXPATHLEN) {
        fatal("Home directory too long (%d > %d", len-1, MAXPATHLEN-1);
        return NULL;
    }
    expanded = malloc(len);
    snprintf(expanded, len, "%s%s", home, cp + 1);
    return expanded;
}


static int passwd_attempt = 0;
static int kbdint_attempt = 0;
static int srp_attempt = 0;
static int external_attempt = 0;
static int gssapi_attempt = 0;
static int pubkey_agent_called = 0;
static int pubkey_idx = 0;
static int gssapi_mech = 0;

void
userauth_reset(void)
{
    passwd_attempt = 0;
    kbdint_attempt = 0;
    srp_attempt = 0;
    external_attempt = 0;
    gssapi_attempt = 0;
    gssapi_mech = 0;
    pubkey_agent_called = 0;
    pubkey_idx = 0;
}

static int      ssh_session(void);
static int      ssh_session2(void);
static void     load_public_identity_files(void);

static void
apply_kermit_settings(Options * options)
{
    extern int tcp_dns_srv;
    int i, x;

    switch (ssh_ver) {
    case 1:
        options->protocol = SSH_PROTO_1;
        break;
    case 2:
        options->protocol = SSH_PROTO_2;
        break;
    default:
        options->protocol = SSH_PROTO_1|SSH_PROTO_2;
    }

    options->use_privileged_port = ssh_prp;
    options->forward_x11 = ssh_xfw;
    options->forward_agent = ssh_afw;
    options->gateway_ports = ssh_gwp;
    options->check_host_ip = ssh_chkip;
    options->strict_host_key_checking = ssh_shk;
    options->keepalives = tcp_keepalive;
    options->heartbeat_interval = ssh_hbt;

    if ( ssh_xal && *ssh_xal )
        makestr(&options->xauth_location,ssh_xal);
    else
        makestr(&options->xauth_location,NULL);

    if ( ssh1_gnh && *ssh1_gnh )
        makestr(&options->system_hostfile,ssh1_gnh);
    else
        makestr(&options->system_hostfile,NULL);

    if ( ssh1_unh && *ssh1_unh )
        makestr(&options->user_hostfile,ssh1_unh);
    else
        makestr(&options->user_hostfile,NULL);

    if ( ssh2_gnh && *ssh2_gnh )
        makestr(&options->system_hostfile2,ssh2_gnh);
    else
        makestr(&options->system_hostfile2,NULL);

    if ( ssh2_unh && *ssh2_unh )
        makestr(&options->user_hostfile2,ssh2_unh);
    else
        makestr(&options->user_hostfile2,NULL);

    if ( ssh_idf_n > 0) {
        int i;
        for ( i=0; i < SSH_MAX_IDENTITY_FILES && i < ssh_idf_n ; i++ )
            makestr(&options->identity_files[i],ssh_idf[i]);
        options->num_identity_files = i;
    } else
        options->num_identity_files = 0;

    options->rhosts_authentication = 1;
    options->rsa_authentication = 1;
    options->pubkey_authentication = 1;
    options->challenge_response_authentication = 1;
    options->password_authentication = 1;
    options->kbd_interactive_authentication = 1;
    options->rhosts_rsa_authentication = 1;
    options->hostbased_authentication = 1;

#ifdef CK_SRP
    if ( ck_srp_is_installed() )
        options->srp_authentication = 1;
    else
        options->srp_authentication = 0;
#endif /* CK_SRP */
#ifdef KRB5
    if ( ck_krb5_is_installed() ) {
        options->kerberos_tgt_passing = ssh_k5tgt;
        options->krb5_authentication = 1;
    } else {
        options->kerberos_tgt_passing = 0;
        options->krb5_authentication = 0;
    }
#endif /* KRB5 */
#ifdef GSSAPI
    if ( ck_gssapi_is_installed() ) {
        options->gss_authentication = 1;
        options->gss_deleg_creds = ssh_gsd;
    } else {
        options->gss_deleg_creds = 0;
        options->gss_authentication = 1;
    }
#endif /* GSSAPI */
#ifdef KRB4
    if ( ck_krb4_is_installed() ) {
        options->krb4_authentication = 1;
        options->kerberos_tgt_passing = ssh_k4tgt;
    } else {
        options->krb4_authentication = 1;
        options->kerberos_tgt_passing = 0;
    }
#endif /* KRB4 */
#ifdef AFS
    options->afs_token_passing = -1;
#endif /* AFS */

    /* options.log_level = SYSLOG_LEVEL_DEBUG[123], SYSLOG_LEVEL_QUIET */
    options->log_level = ssh_vrb;

    /* Currently no escape character. Would be any ascii value. */
    options->escape_char = SSH_ESCAPECHAR_NONE;

    if (ssh2_cif && *ssh2_cif) {
        makestr(&options->ciphers,ssh2_cif);
        options->cipher = SSH_CIPHER_ILLEGAL;
    } else
        makestr(&options->ciphers,NULL);

    if (ssh1_cif && *ssh1_cif) {
        /* SSH1 only */
        options->cipher = cipher_number(ssh1_cif);
        if (options->cipher == -1) {
            fprintf(stderr, "Unknown cipher type '%s'\n", ssh1_cif);
        } else if (options->ciphers == NULL) {
            if (options->cipher == SSH_CIPHER_3DES) {
                options->ciphers= "3des-cbc";
            } else if (options->cipher == SSH_CIPHER_BLOWFISH) {
                options->ciphers = "blowfish-cbc";
            } else {
                options->ciphers = (char *)-1;
            }
        }
    } else
        options->cipher = -1;

    if ( ssh2_mac && *ssh2_mac )
        makestr(&options->macs,ssh2_mac);
        else
            makestr(&options->macs,NULL);

    if ( ssh2_hka && *ssh2_hka ) {
        if ( key_names_valid2(ssh2_hka) )
            makestr(&options->hostkeyalgorithms,ssh2_hka);
        else {
            printf("Invalid v2 host key algorithms: %s\n",ssh2_hka);
            makestr(&options->hostkeyalgorithms,NULL);
        }
    } else
        makestr(&options->hostkeyalgorithms,NULL);

    if ( ssh2_auth && *ssh2_auth )
        makestr(&options->preferred_authentications,ssh2_auth);
    else
        makestr(&options->preferred_authentications,NULL);

    if (ssh_prt == NULL)
	ssh_prt = SSH_SERVICE_NAME;

    x = atoi(ssh_prt);
    if (x > 0)
	options->port = x;
    else {
	struct servent * service = NULL;
#ifdef CK_DNS_SRV
	if (ssh_dns_addrs) {
	    free(ssh_dns_addrs);
	    ssh_dns_addrs = NULL;
	    ssh_dns_naddrs = 0;
	}
	if (tcp_dns_srv && !quiet) {
	    printf(" DNS SRV Lookup... ");
	    fflush(stdout);
	}
	if (tcp_dns_srv &&
	     locate_srv_dns(ssh_hst,
			     ssh_prt,
			     "tcp",
			     &ssh_dns_addrs,
			     &ssh_dns_naddrs
			     )
	     )
	{
	    /* Use the first one.  Eventually we should cycle through all */
	    /* the returned IP addresses and port numbers. */
	    struct sockaddr_in *sin = NULL;
#ifdef BETADEBUG
	    int i;
	    printf("\r\n");
	    for ( i=0;i<ssh_dns_naddrs;i++ ) {
		sin = (struct sockaddr_in *) &ssh_dns_addrs[i];
		printf("ssh_dns_addrs[%d] = %s %d\r\n", i,
			(char *)inet_ntoa(sin->sin_addr),
			ntohs(sin->sin_port));
	    }
#endif /* BETADEBUG */
	    sin = (struct sockaddr_in *) &ssh_dns_addrs[0];
	    options->port = ntohs(sin->sin_port);
	} else
#endif /* CK_DNS_SRV */
	{
	    service = getservbyname(ssh_prt, "tcp");
	    if (service)
		options->port = ntohs(service->s_port);
	}
    }
    
    if (uidbuf[0])
        makestr(&options->user,uidbuf);
    else {
        char tmp[BUFSIZ], prompt[256];
        char *user;
        char *myname;
        int ok;

        myname = whoami();
        if (!myname) myname = "";
        if (myname[0])
          ckmakxmsg(prompt,sizeof(prompt)," Name (",myname,"): ",
                    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        else
          strcpy(prompt," Name: ");
        tmp[0] = '\0';
        ok = uq_txt("Username required for SSH\n",prompt,1,NULL,tmp,sizeof(tmp),
                    NULL,DEFAULT_UQ_TIMEOUT);
        if (!ok || (*tmp == '\0'))
          user = myname;
        else
          user = tmp;
        makestr(&options->user,user);
    }

    for ( i=0 ; i < ssh_pf_lcl_n ; i++ )
        add_local_forward(options, ssh_pf_lcl[i].p1,
                           ssh_pf_lcl[i].host, ssh_pf_lcl[i].p2);
    for ( i=0 ; i < ssh_pf_rmt_n ; i++ )
        add_remote_forward(options, ssh_pf_rmt[i].p1,
                            ssh_pf_rmt[i].host, ssh_pf_rmt[i].p2);

    if ( ssh_dyf )
        add_local_forward(options, 1080, "socks4", 0);

    options->compression = ck_zlib_is_installed() ? ssh_cmp : 0;

    subsystem_flag = ssh_cas;
}

int
ssh_open(VOID)
{
    int i, opt, optind, type, cerr, fwd_port=0, fwd_host_port=0, authfd;
    char *optarg=NULL, *cp=NULL, buf[256];
    struct _stat st;
    int interactive = 0, dummy;
    char *pw_dir=NULL, *pw_name=NULL;
    struct passwd *pw=NULL;
    Sensitive sensitive_data;

    if (!ck_ssleay_is_installed()) {
        printf("? SSH not available\r\n");
        return -1;
    }

    /* Check that we got a host name. */
    if (!ssh_hst)
        return(-114);

    myproposal[0] = KEX_DEFAULT_KEX;
    myproposal[1] = KEX_DEFAULT_PK_ALG;
    myproposal[2] = KEX_DEFAULT_ENCRYPT;
    myproposal[3] = KEX_DEFAULT_ENCRYPT;
    myproposal[4] = KEX_DEFAULT_MAC;
    myproposal[5] = KEX_DEFAULT_MAC;
    myproposal[6] = KEX_DEFAULT_COMP;
    myproposal[7] = KEX_DEFAULT_COMP;
    myproposal[8] = KEX_DEFAULT_LANG;
    myproposal[9] = KEX_DEFAULT_LANG;

    debug(F100,"ssh_open starting resets","",0);
    reset_compat();
    channel_reset();
    packet_reset();
    authmethod_reset();
    userauth_reset();
    buffer_compress_uninit();
    sent_initial_ws = 0;
    received_window_change_signal = 0;
    session_closed = 0;
    debug(F100,"ssh_open finished resets","",0);

    /* Initialize option structure to indicate that no values have been set. */
    if ( !opt_init ) {
        initialize_options(&options);
        opt_init = 1;
    }

    /* Set options. */
    host = ssh_hst;
    debug(F100,"ssh_open applying kermit settings","",0);
    apply_kermit_settings(&options);

    /* Initialize the command to execute on remote host. */
    if ( buf_init )
        buffer_clear(&command);
    else {
        buffer_init(&command);
        buf_init = 1;
    }

    /* Save the command to execute on the remote host in a buffer.  There is
    no limit on the length of the command, except by the maximum packet
    size.  Also sets the tty flag if there is no command. */
    if ( ssh_cmd && *ssh_cmd )
    {
        /* A command has been specified.  Store it into the buffer. */
        buffer_append(&command, ssh_cmd, strlen(ssh_cmd));
    }

    /* Allocate a tty by default if no command specified. */
    tty_flag = (buffer_len(&command) == 0);

    debug(F100,"ssh_open initalizing pw","",0);
    /* Get home directory and initialize struct passwd */
    makestr(&pw_dir,zhome());
    makestr(&pw_name, uidbuf);

    pw = malloc(sizeof(struct passwd));
    if ( pw ) {
        pw->pw_name = pw_name;
        pw->pw_dir = pw_dir;
        pw->pw_passwd = NULL;
        pw->pw_shell = NULL;
        pw->pw_uid = 1;                 /* meaningless on windows */
        pw->pw_gid = 1;                 /* ditto */
    }

    debug(F100,"ssh_open log_init() 1","",0);
    /* Initialize "log" output.  Since we are the client all output actually
    goes to the terminal. */
    log_init("Kermit Builtin SSH", 1, SYSLOG_FACILITY_USER, 1);

    if (ssh_cfg) {
    /* Read per-user configuration file. pw_dir contains trailing slash */
        sprintf(buf, "%.100s%s", pw_dir, (char *)_PATH_SSH_USER_CONFFILE);
        read_config_file(buf, host, &options);

        /* Read systemwide configuration file. */
        read_config_file((char *)_PATH_HOST_CONFIG_FILE, host, &options);
    }

    debug(F100,"ssh_open calling fill_default_options()","",0);
    /* Fill configuration defaults. */
    fill_default_options(&options);

    debug(F100,"ssh_open calling log_init() 2","",0);
    /* reinit */
    log_init("Kermit Builtin SSH", options.log_level, SYSLOG_FACILITY_USER, 1);

    debug(F100,"ssh_open calling seed_rng()","",0);
    seed_rng();

    if (options.user == NULL)
        options.user = strdup(pw_name);

    if (options.hostname != NULL)
        host = options.hostname;

    /* Disable rhosts authentication if not running as root. */
    if (!options.use_privileged_port)
        options.rhosts_authentication = 0;

    /* Open a connection to the remote host.  This needs root privileges if
    rhosts_authentication is true.  Note that the random_state is not
    yet used by this call, although a pointer to it is stored, and thus it
    need not be initialized. */
    cerr = ssh_connect(host, &hostaddr, options.port, IPv4or6,
                     options.connection_attempts,
                     options.use_privileged_port,
                     NULL);
    if ( cerr ) {
        if (!quiet) {
            error("Could not connect to %s.",host);
#ifdef COMMENT
#ifdef KUI
            ckmakmsg(errbuf,ERRBUFSZ,"Could not connect to ",host,NULL,NULL);
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */
#endif /* COMMENT */
        }
        return -116;
    }
    /* Successful connection. */

    /*
    * If we successfully made the connection, load the host private key
    * in case we will need it later for combined rsa-rhosts
    * authentication. This must be done before releasing extra
    * privileges, because the file is only readable by root.
    * If we cannot access the private keys, load the public keys
    * instead and try to execute the ssh-keysign helper instead.
    */
    sensitive_data.nkeys = 0;
    sensitive_data.keys = NULL;
    sensitive_data.external_keysign = 0;
    if (options.rhosts_rsa_authentication ||
        options.hostbased_authentication) {
        sensitive_data.nkeys = 3;
        sensitive_data.keys = malloc(sensitive_data.nkeys*sizeof(Key));
        sensitive_data.keys[0] = key_load_private_type(KEY_RSA1,
                                                        (char *)_PATH_HOST_KEY_FILE, "", NULL);
        sensitive_data.keys[1] = key_load_private_type(KEY_DSA,
                                                        (char *)_PATH_HOST_DSA_KEY_FILE, "", NULL);
        sensitive_data.keys[2] = key_load_private_type(KEY_RSA,
                                                        (char *)_PATH_HOST_RSA_KEY_FILE, "", NULL);

        if (options.hostbased_authentication == 1 &&
             sensitive_data.keys[0] == NULL &&
             sensitive_data.keys[1] == NULL &&
             sensitive_data.keys[2] == NULL) {

            sensitive_data.keys[1] = key_load_public(
                                                      (char *)_PATH_HOST_DSA_KEY_FILE, NULL);
            sensitive_data.keys[2] = key_load_public(
                                                      (char *)_PATH_HOST_RSA_KEY_FILE, NULL);
            sensitive_data.external_keysign = 1;
        }
    }

    /*
    * Create ~/.ssh directory if it doesn\'t already exist.
    */
    snprintf(buf, sizeof buf, "%s", (char *)_PATH_SSH_USER_DIR);
    if (stat(buf, &st) < 0)
        if (mkdir(buf, 0700) < 0) {
            error("Could not create directory '%.256s'.", buf);
#ifdef KUI
            ckmakmsg(errbuf,ERRBUFSZ,"Could not create directory: \"",
                      buf,"\"",NULL);
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */
        }

    /* load options.identity_files */
    load_public_identity_files();

    /* Expand ~ in known host file names. */
    /* XXX mem-leaks: */
    options.system_hostfile =
        tilde_expand_filename(options.system_hostfile, -1);
    options.user_hostfile =
        tilde_expand_filename(options.user_hostfile, -1);
    options.system_hostfile2 =
        tilde_expand_filename(options.system_hostfile2, -1);
    options.user_hostfile2 =
        tilde_expand_filename(options.user_hostfile2, -1);

#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif /* SIGPIPE */

    /* Log into the remote system.  This never returns if the login fails. */
    cerr = ssh_login(&sensitive_data, host, (struct sockaddr *)&hostaddr, pw);

    /* We no longer need the private host keys.  Clear them now. */
    if (sensitive_data.nkeys != 0) {
        for (i = 0; i < sensitive_data.nkeys; i++) {
            if (sensitive_data.keys[i] != NULL) {
                /* Destroys contents safely */
                debug3("clear hostkey %d", i);
                key_free(sensitive_data.keys[i]);
                sensitive_data.keys[i] = NULL;
            }
        }
        free(sensitive_data.keys);
    }
    for (i = 0; i < options.num_identity_files; i++) {
        if (options.identity_files[i]) {
                free(options.identity_files[i]);
                options.identity_files[i] = NULL;
        }
        if (options.identity_keys[i]) {
                key_free(options.identity_keys[i]);
                options.identity_keys[i] = NULL;
        }
    }
    free(pw);

    /* If ssh_login() failed, return. */
    if (cerr) {
        if ( !quiet )
            error("login failed");
#ifdef COMMENT
#ifdef KUI
        uq_ok(NULL, "login failed", 1, NULL, 1);
#endif /* KUI */
#endif /* COMMENT */
        ssh_clos();
        return(-122);
    }

#ifdef OS2
    CreateSocketPair(inPair);
    CreateSocketPair(outPair);
    sock_stdin = inPair[0];
    sock_stdout = outPair[1];
    sock_stderr = outPair[1];
#endif /* OS2 */

    /* Enable Window Size reporting */
    TELOPT_ME(TELOPT_NAWS) = 1;

    exit_status = compat20 ? ssh_session2() : ssh_session();
    
    return exit_status;
}

#ifndef NOLOCAL
_PROTOTYP(VOID slrestor,(VOID));
#endif /* NOLOCAL */
int
ssh_clos() {

    if (quit_pending) {
        while (quit_pending)
            msleep(50);
        return(0);
    }

    if (hClientLoopThread) {
        quit_pending = 1;           /* Instruct Client Loop to end */
        ssh_toc(0);
        while (quit_pending && hClientLoopThread)
            msleep(50);
        open_connection = 0;
        quit_pending = 0;
    }

    packet_close();

    if ( SOCK_IN != INVALID_SOCKET ) {
        closesocket(SOCK_IN);
        SOCK_IN = INVALID_SOCKET;
    }
    if ( SOCK_OUT != INVALID_SOCKET ) {
        closesocket(SOCK_OUT);
        SOCK_OUT = INVALID_SOCKET;
    }

#ifndef NOLOCAL
    slrestor();
#endif /* NOLOCAL */

    ssh_sock = -1;

    /* Exit with the status returned by the program on the remote side. */
    return(0);
}

#ifndef NOBUF
#ifdef OS2
#ifdef NT
#define SSHIBUFL 64240                   /* 44 * 1460 (MSS) */
#else
#define SSHIBUFL 32120                   /* 22 * 1460 (MSS) */
#endif /* NT */
#else /* OS2 */
#define SSHIBUFL 8191                    /* Let's use 8K. */
#endif /* OS2 */

static CHAR sshibuf[SSHIBUFL+1];
static int sshibp = 0, sshibn = 0;
#endif /* NOBUF */

int
ssh_tchk(void) {
    long count = 0;
    int x = 0;
    long y;
    char c;

    if ( !open_connection )
        return(-1);

    if (ioctlsocket(SOCK_IN,FIONREAD,&count) < 0) {
#ifdef NT
        int s_errno = WSAGetLastError();
        debug3("ssh_tchk socket = %u s_errno = %d",SOCK_IN, s_errno);
#endif /* NT */ 

#ifndef NOBUF
        if ( sshibn > 0 ) {
            return(sshibn);
        } else 
#endif /* NOBUF */
        {
            ttclos(0);
            return(-1);
        }
    }
#ifndef NOBUF
    return(count+sshibn);
#else
    return(count);
#endif /* NOBUF */
}

int
ssh_flui(void) {
    char c;
    if ( !open_connection )
        return(-1);

    /* Not safe to flush encrypted/compressed connections */
    return(0);
}

int
ssh_break(void) {
    if ( !open_connection )
        return(-1);

    /* No equivalent to Telnet Break */
    return(0);
}

static int
sshbufr(void) {                              /* TT Buffer Read */
    int count;

#ifdef OS2
    RequestTCPIPMutex(SEM_INDEFINITE_WAIT);
#endif /* OS2 */

    if (sshibn > 0) {                    /* Our internal buffer is not empty, */
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(sshibn);                  /* so keep using it. */
    }

    if ( !open_connection ) {
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(-2);
    }

    sshibp = 0;                          /* Else reset pointer to beginning */

#ifdef OS2
    count = SSHIBUFL;
#else                                   /* Multinet, etc. */
    count = ttchk();                    /* Check network input buffer, */
    if (sshibn > 0) {                    /* which can put a char there! */
        debug(F111,"sshbufr","ttchk() returns",count);
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(sshibn);
    }
    if (count < 0) {                     /* Read error - connection closed */
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(-2);
    }
    else if (count > SSHIBUFL)           /* Too many to read */
      count = SSHIBUFL;
    else if (count == 0)                /* None, so force blocking read */
      count = 1;
#endif /* OS2 */
    debug(F101,"sshbufr count 1","",count);


/* This is for blocking reads */

#ifndef VMS
#ifdef SO_OOBINLINE
    {
        int outofband = 0;
#ifdef BELLSELECT
        if (select(128, NULL, NULL, efds, 0) > 0 && FD_ISSET(SOCK_IN, efds))
          outofband = 1;
#else
#ifdef BSDSELECT
        fd_set efds;
        struct timeval tv;
        FD_ZERO(&efds);
        FD_SET(SOCK_IN, &efds);
        tv.tv_sec  = tv.tv_usec = 0L;
        debug(F100,"Out-of-Band BSDSELECT","",0);
#ifdef NT
        WSASafeToCancel = 1;
#endif /* NT */
        if (select(FD_SETSIZE, NULL, NULL, &efds, &tv) > 0 &&
            FD_ISSET(SOCK_IN, &efds))
          outofband = 1;
#ifdef NT
        WSASafeToCancel = 0;
#endif /* NT */
#else /* !BSDSELECT */
#ifdef IBMSELECT
/* Is used by OS/2 ... */
/* ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set */
/* and timeval stuff since this is the only place where it is used. */
        int socket = SOCK_IN;
        debug(F100,"Out-of-Band IBMSELECT","",0);
        if ((select(&socket, 0, 0, 1, 0L) == 1) && (socket == SOCK_IN))
          outofband = 1;
#else /* !IBMSELECT */
/*
  If we can't use select(), then we use the regular alarm()/signal()
  timeout mechanism.
*/
      debug(F101,"Out-of-Band data not supported","",0);
      outofband = 0;

#endif /* IBMSELECT */
#endif /* BSDSELECT */
#endif /* BELLSELECT */
      if (outofband) {
         /* Get the Urgent Data */
         /* if OOBINLINE is disabled this should be only a single byte      */
         /* MS Winsock has a bug in Windows 95.  Extra bytes are delivered  */
         /* That were never sent.                                           */
#ifdef OS2
          RequestTCPIPMutex(SEM_INDEFINITE_WAIT);
#endif /* OS2 */
          count = socket_recv(SOCK_IN,&sshibuf[sshibp+sshibn],count,MSG_OOB);
#ifdef OS2
          ReleaseTCPIPMutex();
#endif /* OS2 */
          if (count <= 0) {
              int s_errno = socket_errno;
              debug(F101, "sshbufr socket_recv MSG_OOB","",count);
              debug(F101, "sshbufr socket_errno","",s_errno);
#ifdef OS2ONLY
              if (count < 0 && (s_errno == 0 || s_errno == 23)) {
                  /* These appear in OS/2 - don't know why   */
                  /* ignore it and read as normal data       */
                  /* and break, then we will attempt to read */
                  /* the port using normal read() techniques */
                  debug(F100,"sshbufr handing as in-band data","",0);
                  count = 1;
              } else {
                  ttclos(0);                    /* *** *** */
#ifdef OS2
                  ReleaseTCPIPMutex();
#endif /* OS2 */
                  return(-2);
              }
#else /* OS2ONLY */
              ttclos(0);                        /* *** *** */
#ifdef OS2
              ReleaseTCPIPMutex();
#endif /* OS2 */
              return(-2);
#endif /* OS2ONLY */
          } else {                      /* we got out-of-band data */
              hexdump("sshbufr out-of-band chars",&sshibuf[sshibp+sshibn],count);
#ifdef BETADEBUG
              bleep(BP_NOTE);
#endif /* BETADEBUG */
              {
                  /* For any protocols we don't have a special out-of-band  */
                  /* handler for, just put the bytes in the normal buffer   */
                  /* and return                                             */

                  sshibp += 0;       /* Reset buffer pointer. */
                  sshibn += count;
#ifdef DEBUG
                  /* Got some bytes. */
                  debug(F101,"sshbufr count 2","",count);
                  if (count > 0)
                      sshibuf[sshibp+sshibn] = '\0';
                  debug(F111,"sshbufr sshibuf",sshibuf,sshibp);
#endif /* DEBUG */
#ifdef OS2
                  ReleaseTCPIPMutex();
#endif /* OS2 */
                  return(sshibn);    /* Return buffer count. */
              }
          }
      }
    }
#endif /* SO_OOBINLINE */
#endif /* VMS */

    count = socket_read(SOCK_IN,&sshibuf[sshibp+sshibn],count);
    if (count <= 0) {
        int s_errno = socket_errno;
        debug(F101,"sshbufr socket_read","",count);
        debug(F101,"sshbufr socket_errno","",s_errno);
#ifdef OS2
        if (count == 0 || os2socketerror(s_errno) < 0) {
            ttclos(0);
            ReleaseTCPIPMutex();
            return(-2);
        }
        ReleaseTCPIPMutex();
        return(-1);
#else /* OS2 */
        ttclos(0);                      /* *** *** */
        return(-2);
#endif /* OS2 */
    } else {
        sshibp = 0;                      /* Reset buffer pointer. */
        sshibn += count;
#ifdef DEBUG
        debug(F101,"sshbufr count 2","",count); /* Got some bytes. */
        if (count > 0)
          sshibuf[sshibp+sshibn] = '\0';
        debug(F111,"sshbufr sshibuf",&sshibuf[sshibp],sshibn);
#endif /* DEBUG */
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(sshibn);                  /* Return buffer count. */
    }
}

int
ssh_inc(int timo)  {
#ifdef NOBUF
    char c=0;
    fd_set rfds;
    struct timeval tv;
    int count;

    if ( !open_connection )
        return(-1);

  begin:
    FD_ZERO(&rfds);
    FD_SET(SOCK_IN, &rfds);
    tv.tv_sec  = tv.tv_usec = 0L;
    if (timo < 0)
        tv.tv_usec = (long) -timo * 1000L;
    else
        tv.tv_sec = timo;
    if (select(FD_SETSIZE, (fd_set *) &rfds,
                NULL, NULL, &tv) > 0 &&
         FD_ISSET(SOCK_IN, &rfds)) {

        if ((count = recv(SOCK_IN,&c,1,0)) <= 0) {
            int s_errno = WSAGetLastError();
            if (count == 0) {
                ttclos(0);              /* if the connection was  */
                return -3;              /* return a hard error    */
            } else {
                switch (s_errno) {
                case WSAETIMEDOUT:
                    if ( timo == 0 )
                        goto begin;
                    return(-1);
                case WSAECONNRESET:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAECONNABORTED:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAENETRESET:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAENOTCONN:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAEWOULDBLOCK:
                    break;
                }
                if ( timo == 0 )
                    goto begin;
                return -1;                      /* Return a timeout */
            }
        }
        return(c);                              /* Return char read */
    }
    if ( timo == 0 )
        goto begin;
    return(-1);                                 /* Return a timeout */
#else /* NOBUF */
    int x, c;
#ifdef OS2
    RequestTCPIPMutex(SEM_INDEFINITE_WAIT);
#endif /* OS2 */
    if (sshibn > 0) {                    /* Something in internal buffer? */
#ifdef COMMENT
        debug(F100,"ssh_inc char in buf","",0); /* Yes. */
#endif /* COMMENT */
        x = 0;                          /* Success. */
    } else {                            /* Else must read from network. */
        x = -1;                         /* Assume failure. */
#ifdef DEBUG
        debug(F101,"ssh_inc goes to net, timo","",timo);
#endif /* DEBUG */
        {
#ifdef BSDSELECT
            fd_set rfds;
            struct timeval tv;
            int timeout = timo < 0 ? -timo : 1000 * timo;
            debug(F101,"ssh_inc BSDSELECT","",timo);

            for ( ; timeout >= 0; timeout -= (timo ? 100 : 0)) {
                int rc;
                debug(F111,"ssh_inc","timeout",timeout);
                /* Don't move select() initialization out of the loop. */
                FD_ZERO(&rfds);
                FD_SET(SOCK_IN, &rfds);
                tv.tv_sec  = tv.tv_usec = 0L;
                if (timo)
                  tv.tv_usec = (long) 100000L;
                else
                  tv.tv_sec = 30;
#ifdef NT
                WSASafeToCancel = 1;
#endif /* NT */
                rc = select(FD_SETSIZE,
#ifndef __DECC
                            (fd_set *)
#endif /* __DECC */
                            &rfds, NULL, NULL, &tv);
                if (rc < 0) {
                    int s_errno = socket_errno;
                    debug(F111,"ssh_inc","select",rc);
                    debug(F111,"ssh_inc","socket_errno",s_errno);
                    if (s_errno) {
#ifdef OS2
                        ReleaseTCPIPMutex();
#endif /* OS2 */
                        return(-1);
                    }
                }
                debug(F111,"ssh_inc","select",rc);
#ifdef NT
                WSASafeToCancel = 0;
#endif /* NT */
                if (!FD_ISSET(SOCK_IN, &rfds)) {
#ifdef LEBUF
                    if (le_inbuf() > 0) {
                        timeout = -1;
                        break;
                    }
#endif /* LEBUF */
                    /* If waiting forever we have no way of knowing if the */
                    /* socket closed so try writing a 0-length TCP packet  */
                    /* which should force an error if the socket is closed */
                    if (!timo) {
                        if ((rc = socket_write(SOCK_IN,"",0)) < 0) {
                            int s_errno = socket_errno;
                            debug(F101,"ssh_inc socket_write error","",s_errno);
#ifdef OS2
                            if (os2socketerror(s_errno) < 0) {
                              ReleaseTCPIPMutex();
                              return(-2);
                            }
                            ReleaseTCPIPMutex();
#endif /* OS2 */
                            return(-1); /* Call it an i/o error */
                        }
                    }
                    continue;
                }
                while (1) {
                    if (sshbufr() < 0) { /* Keep trying to refill it. */
                        timeout = -1;
                        break;          /* Till we get an error. */
                    }
                    if (sshibn > 0) {    /* Or we get a character. */
                        x = 0;
                        timeout = -1;
                        break;
                    }
                }
            }
#ifdef NT
            WSASafeToCancel = 0;
#endif /* NT */
#else /* !BSDSELECT */
#ifdef IBMSELECT
/*
  Was used by OS/2, currently not used, but might come in handy some day...
  ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set
  and timeval stuff since this is the only place where it is used.
*/
            int socket = SOCK_IN;
            int timeout = timo < 0 ? -timo : 1000 * timo;

            debug(F101,"ssh_inc IBMSELECT","",timo);
            for ( ; timeout >= 0; timeout -= (timo ? 100 : 0)) {
                if (select(&socket, 1, 0, 0, 100L) == 1) {
                    while (1) {
                        if (sshbufr() < 0) { /* Keep trying to refill it. */
                            timeout = -1;
                            break;      /* Till we get an error. */
                        }
                        if (sshibn > 0) { /* Or we get a character. */
                            x = 0;
                            timeout = -1;
                            break;
                        }
                    }
                }
#ifdef LEBUF
                else if (le_inbuf() > 0)  {
                    timeout = -1;
                    break;
                }
#endif /* LEBUF */
            }
#else /* !IBMSELECT */
#ifdef WINSOCK
       /* Actually, under WinSock we have a better mechanism than select() */
       /* for setting timeouts (SO_RCVTIMEO, SO_SNDTIMEO) */
            SOCKET socket = SOCK_IN;
            debug(F101,"ssh_inc NTSELECT","",timo);
            if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timo,
                            sizeof(timo))  == NO_ERROR)
              while (1) {
                  if (sshbufr() < 0)     /* Keep trying to refill it. */
                    break;              /* Till we get an error. */
                  if (sshibn > 0) {      /* Or we get a character. */
                      x = 0;
                      break;
                  }
              }
#else /* WINSOCK */
/*
  If we can't use select(), then we use the regular alarm()/signal()
  timeout mechanism.
*/
            debug(F101,"ssh_inc alarm","",timo);
            x = alrm_execute(ckjaddr(njbuf),timo,nettout,dossh_inc,failssh_inc);
            sshimoff();                  /* Timer off. */
#endif /* WINSOCK */
#endif /* IBMSELECT */
#endif /* BSDSELECT */
        }
    }

#ifdef LEBUF
    if (le_inbuf() > 0) {               /* If data was inserted into the */
        if (le_getchar((CHAR *)&c) > 0) {/* Local Echo buffer while the   */
#ifdef OS2                               /* was taking place do not mix   */
          ReleaseTCPIPMutex();           /* the le data with the net data */
#endif /* OS2 */
          return(c);
        }
    }
#endif /* LEBUF */
    if (x < 0) {                        /* Return -1 if we failed. */
        debug(F100,"ssh_inc timed out","",0);
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(-1);
    } else {                            /* Otherwise */
        c = sshibuf[sshibp];              /* Return the first char in sshibuf[] */
        if (deblog) {
#ifndef COMMENT
            debug(F101,"ssh_inc returning","",c);
#endif /* COMMENT */
            if (c == 0) {
                debug(F101,"ssh_inc 0 sshibn","",sshibn);
                debug(F101,"ssh_inc 0 sshibp","",sshibp);
#ifdef BETADEBUG
                {
#ifdef OS2
                    extern int tt_type_mode;
                    if ( !ISVTNT(tt_type_mode) )
#endif /* OS2 */
                    hexdump("ssh_inc &ttbuf[sshibp]",&sshibuf[sshibp],sshibn);
                }
#endif /* BETADEBUG */
            }
        }
        sshibp++;
        sshibn--;
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(c);
    }
#endif /* NOBUF */
}


int
ssh_xin(int n, char * buf) {
#ifdef NOBUF
    int count;
    char c=0;
    fd_set rfds;
    struct timeval tv;

    if ( !open_connection )
        return(-1);

  begin:
    FD_ZERO(&rfds);
    FD_SET(SOCK_IN, &rfds);
    tv.tv_sec  = tv.tv_usec = 0L;       /* No timeout */

    if (select(FD_SETSIZE, (fd_set *) &rfds,
                NULL, NULL, &tv) > 0 &&
         FD_ISSET(SOCK_IN, &rfds)) {

        if ((count = recv(SOCK_IN,buf,n,0)) <= 0) {
            int s_errno = WSAGetLastError();
            if (count == 0) {
                ttclos(0);              /* if the connection was  */
                return -3;              /* return a hard error    */
            } else {
                switch (s_errno) {
                case WSAETIMEDOUT:
                    return(-1);
                case WSAECONNRESET:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAECONNABORTED:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAENETRESET:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAENOTCONN:
                    ttclos(0);                  /* *** *** */
                    return(-2);                 /* Connection is broken. */
                case WSAEWOULDBLOCK:
                    break;
                }
                return 0;                       /* Return a timeout */
            }
        }
        return(count);
    }
    return(0);
#else /* NOBUF */
    int len, i, j;
    int rc;

    if ( !open_connection ) {
        debug(F100,"ssh_xin socket is closed","",0);
        return(-2);
    }
#ifdef OS2
    RequestTCPIPMutex(SEM_INDEFINITE_WAIT);
#endif /* OS2 */
    if (sshibn == 0)
      if ((rc = sshbufr()) <= 0) {
#ifdef OS2
        ReleaseTCPIPMutex();
#endif /* OS2 */
        return(rc);
      }

    if (sshibn <= n) {
        len = sshibn;
        memcpy(buf,&sshibuf[sshibp],len);         /* safe */
        sshibp += len;
        sshibn = 0;
    } else {
        memcpy(buf,&sshibuf[sshibp],n);           /* safe */
        sshibp += n;
        sshibn -= n;
        len = n;
    }
#ifdef OS2
    ReleaseTCPIPMutex();
#endif /* OS2 */

    return(len);
#endif /* NOBUF */
}


int
ssh_toc(int c) {
    char ch = c;
    fd_set wfds;
    struct timeval tv;

    if ( !open_connection )
        return(-1);

    FD_ZERO(&wfds);
    FD_SET(SOCK_OUT, &wfds);
    tv.tv_sec  = tv.tv_usec = 0L;
    tv.tv_sec = 60;
    if (!(select(FD_SETSIZE, NULL, &wfds, NULL, &tv) > 0 &&
           FD_ISSET(SOCK_OUT, &wfds))) {
              return(-1);
    }

    if (send(SOCK_OUT,&ch,1,0) < 1) {
        int s_errno = WSAGetLastError();
        switch (s_errno) {
        case WSAETIMEDOUT:
            return(-1);
        case WSAECONNRESET:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAECONNABORTED:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAENETRESET:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAENOTCONN:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAEWOULDBLOCK:
            break;
        }
        return(-1);
    }
    return(0);
}

int
ssh_tol(char * buffer, int count) {
    fd_set wfds;
    struct timeval tv;

    if ( !open_connection )
        return(-1);

    FD_ZERO(&wfds);
    FD_SET(SOCK_OUT, &wfds);
    tv.tv_sec  = tv.tv_usec = 0L;
    tv.tv_sec = 60;
    if (!(select(FD_SETSIZE, NULL, &wfds, NULL, &tv) > 0 &&
           FD_ISSET(SOCK_OUT, &wfds))) {
              return(-1);
    }

    if ((count = send(SOCK_OUT,buffer,count,0)) < 1) {
        int s_errno = WSAGetLastError();         /* maybe a function */
        switch (s_errno) {
        case WSAETIMEDOUT:
            return(-1);
        case WSAECONNRESET:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAECONNABORTED:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAENETRESET:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAENOTCONN:
            ttclos(0);                  /* *** *** */
            return(-2);                 /* Connection is broken. */
        case WSAEWOULDBLOCK:
            break;
        }
        return(-1);
    }
    return(count);
}

VOID
ssh_terminfo(char * ttype, int h, int w) {
    static int height=0, width=0;
    if ( h != height || w != width ) {
        height = h;
        width = w;
        packet_start(SSH_CMSG_WINDOW_SIZE);
        packet_put_int(h);
        packet_put_int(w);
        packet_put_int(0);
        packet_put_int(0);
        packet_send();
    }
}



const char *
ssh_version(void) {
    static char msg[256];
    sprintf(msg,
             "%s, SSH protocols %d.%d/%d.%d, OpenSSL 0x%8.8lx\n",
             SSH_VERSION,
             PROTOCOL_MAJOR_1, PROTOCOL_MINOR_1,
             PROTOCOL_MAJOR_2, PROTOCOL_MINOR_2,
             SSLeay());
    return(msg);
}

const char *
ssh_errorstr(int error) {
    switch ( error ) {
    case 0:
        return("success");
        break;
    case -1:
        return("timeout");
        break;
    case -2:
        return("connection closed");
    case -100:
        return("usage error: matching parenthese not found");
    case -101:
        return("usage error: missing operand");
    case -102:
        return("usage error: option missing argument");
        break;
    case -103:
        return("usage error: switch specified is more than one letter");
    case -104:
        return("usage error: identity file does not exist");
    case -105:
        return("usage error: too many identity files specified");
    case -106:
        return(ssh_version());
    case -107:
        return("usage error: Bad escape character");
    case -108:
        return("usage error: unknown cipher type");
    case -109:
        return("usage error: bad port");
    case -110:
        return("usage error: bad remote forwarding specification");
    case -111:
        return("usage error: bad local forwarding specification");
    case -112:
        return("usage error: privileged ports can only be forwarded by root");
    case -113:
        return("usage error: unknown switch");
    case -114:
        return("usage error: missing host");
    case -115:
        return("cannot fork into background without a command to execute");
    case -116:
        return("connection failed");
    case -117:
        return("unable to create directory $(HOME)/.ssh");
    case -118:
        return("Compression level must be from 1 (fast) to 9 (slow, best)");
    case -119:
        return("fork after authentication not implemented on Windows NT");
    case -120:
        return("authentication failed");
    case -121:
        return("unable to create socket pair");
    case -122:
        return("login failure");
    default:
        return("unknown");
    }
}

int
ssh_ttvt(void)
{
    if ( !open_connection )
        return(-1);
    return(0);
}

int
ssh_ttpkt(void)
{
    if ( !open_connection )
        return(-1);
    return(0);
}

int
ssh_ttres(void)
{
    if ( !open_connection )
        return(-1);
    return(0);
}
/* END SSHDLL.C */

#ifdef COMMENT
/* Keep so we know what the SSH personality command line should look like */
VOID
ssh_usage(void)
{
        fprintf(stderr, "Usage: %s [options] host [command]\n", __progname);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -l user     Log in using this user name.\n");
        fprintf(stderr, "  -n          Redirect input from " _PATH_DEVNULL ".\n");
        fprintf(stderr, "  -A          Enable authentication agent forwarding.\n");
        fprintf(stderr, "  -a          Disable authentication agent forwarding.\n");
#ifdef AFS
        fprintf(stderr, "  -k          Disable Kerberos ticket and AFS token forwarding.\n");
#endif                          /* AFS */
        fprintf(stderr, "  -X          Enable X11 connection forwarding.\n");
        fprintf(stderr, "  -x          Disable X11 connection forwarding.\n");
        fprintf(stderr, "  -i file     Identity for public key authentication "
            "(default: ~/.ssh/identity)\n");
        fprintf(stderr, "  -t          Tty; allocate a tty even if command is given.\n");
        fprintf(stderr, "  -T          Do not allocate a tty.\n");
        fprintf(stderr, "  -v          Verbose; display verbose debugging messages.\n");
        fprintf(stderr, "              Multiple -v increases verbosity.\n");
        fprintf(stderr, "  -V          Display version number only.\n");
        fprintf(stderr, "  -q          Quiet; don't display any warning messages.\n");
        fprintf(stderr, "  -f          Fork into background after authentication.\n");
        fprintf(stderr, "  -e char     Set escape character; ``none'' = disable (default: ~).\n");

        fprintf(stderr, "  -c cipher   Select encryption algorithm: "
            "``3des'', ``blowfish''\n");
        fprintf(stderr, "  -m macs     Specify MAC algorithms for protocol version 2.\n");
        fprintf(stderr, "  -p port     Connect to this port.  Server must be on the same port.\n");
        fprintf(stderr, "  -L listen-port:host:port   Forward local port to remote address\n");
        fprintf(stderr, "  -R listen-port:host:port   Forward remote port to local address\n");
        fprintf(stderr, "              These cause %s to listen for connections on a port, and\n", __progname);
        fprintf(stderr, "              forward them to the other side by connecting to host:port.\n");
        fprintf(stderr, "  -C          Enable compression.\n");
        fprintf(stderr, "  -N          Do not execute a shell or command.\n");
        fprintf(stderr, "  -g          Allow remote hosts to connect to forwarded ports.\n");
        fprintf(stderr, "  -1          Force protocol version 1.\n");
        fprintf(stderr, "  -2          Force protocol version 2.\n");
        fprintf(stderr, "  -4          Use IPv4 only.\n");
        fprintf(stderr, "  -6          Use IPv6 only.\n");
        fprintf(stderr, "  -o 'option' Process the option as if it was read from a configuration file.\n");
        fprintf(stderr, "  -s          Invoke command (mandatory) as SSH2 subsystem.\n");
        exit(1);
}
#endif /* COMMENT */

static void
x11_get_proto(char ** _proto, char **_data)
{
    char line[512];
    static char proto[512], data[512];
    FILE *f;
    int got_data = 0, i;
    char *display;
    struct _stat st;

    *_proto = proto;
    *_data  = data;
    proto[0] = data[0] = '\0';

#ifndef OS2
    if (options.xauth_location ||
        (stat(options.xauth_location, &st) == -1)) 
    {
        debug("No xauth program.");
    } else {
        if ((display = tn_get_display()) == NULL) {
            debug("x11_get_proto: DISPLAY not set");
            return;
        }

        /* Try to get Xauthority information for the display. */
        if (strncmp(display, "localhost:", 10) == 0)
            snprintf(line, sizeof line, "%.100s list %.200s 2>" _PATH_DEVNULL,
                      options.xauth_location, display+10);
        else
            snprintf(line, sizeof line, "%.100s list %.200s 2>" _PATH_DEVNULL,
                  options.xauth_location, display);
        debug2("x11_get_proto: %s", line);
        f = _popen(line, "r");
        if (f && fgets(line, sizeof(line), f) &&
             sscanf(line, "%*s %511s %511s", proto, data) == 2)
            got_data = 1;
        if (f)
            _pclose(f);
    }
#endif /* OS2 */
    /*
     * If we didn't get authentication data, just make up some
     * data.  The forwarding code will check the validity of the
     * response anyway, and substitute this data.  The X11
     * server, however, will ignore this fake data and use
     * whatever authentication mechanisms it was using otherwise
     * for the local connection.
     */
    if (!got_data) {
        u_int32_t rand = 0;

        log("Warning: No xauth data; using fake authentication data for X11 forwarding");
        strlcpy(proto, "MIT-MAGIC-COOKIE-1", sizeof(proto));
        for (i = 0; i < 16; i++) {
            if (i % 4 == 0)
                rand = arc4random();
            snprintf(data + 2 * i, sizeof(data) - 2 * i, "%02x", rand & 0xff);
            rand >>= 8;
        }
    }
}

void
ssh_v2_rekey(void)
{
    if (compat20 && !(datafellows & SSH_BUG_NOREKEY))
        need_rekeying = 1;
}

int
ssh_fwd_local_port(int port, char * host, int host_port)
{
    return channel_setup_local_fwd_listener(port,host,host_port,ssh_gwp);
}

int
ssh_fwd_remote_port(int port, char * host, int host_port)
{
    return channel_request_remote_forwarding(port,host,host_port);
}

static void
ssh_init_forwarding(void)
{
    int success = 0;
    int i;

    /* Initiate local TCP/IP port forwardings. */
    for (i = 0; i < options.num_local_forwards; i++) {
        debug(F111,"SSH: Connections to","local port",
               options.local_forwards[i].port);
        debug(F111,"SSH: Forwarded to",
               options.local_forwards[i].host,
               options.local_forwards[i].host_port);
        success += channel_setup_local_fwd_listener(
               options.local_forwards[i].port,
               options.local_forwards[i].host,
               options.local_forwards[i].host_port,
               options.gateway_ports);
    }
    if (i > 0 && success == 0) {
        error("Could not request local forwarding.");
#ifdef KUI
        uq_ok(NULL, "Could not request local forwarding", 1, NULL, 0);
#endif /* KUI */
    }
    /* Initiate remote TCP/IP port forwardings. */
    for (i = 0; i < options.num_remote_forwards; i++) {
        debug(F111,"SSH: Connections to","remote port",
               options.remote_forwards[i].port);
        debug(F111,"SSH: Forwarded to local address",
               options.remote_forwards[i].host,
               options.remote_forwards[i].host_port);
        channel_request_remote_forwarding(options.remote_forwards[i].port,
                                           options.remote_forwards[i].host,
                                           options.remote_forwards[i].host_port);
    }
}

static void
check_agent_present(void)
{
    if (options.forward_agent) {
        /* Clear agent forwarding if we don\'t have an agent. */
        if (!ssh_agent_present())
            options.forward_agent = 0;
    }
}

static int
ssh_session(void)
{
    int type;
    int interactive = 0;
    int have_tty = 0;
    struct winsize ws;
    char *cp;

    /* Enable compression if requested. */
    if (options.compression) {
        debug(F111,"SSH","Requesting compression at level", options.compression_level);

        if (options.compression_level < 1 || options.compression_level > 9) {
            ssh_fatal("Compression level must be from 1 (fast) to 9 (slow, best).");
            return -1;
        }
        /* Send the request. */
        packet_start(SSH_CMSG_REQUEST_COMPRESSION);
        packet_put_int(options.compression_level);
        packet_send();
        packet_write_wait();
        type = packet_read();
        if (type == SSH_SMSG_SUCCESS)
            packet_start_compression(options.compression_level);
        else if (type == SSH_SMSG_FAILURE)
            log("Warning: Remote host refused compression.");
        else {
            packet_disconnect("Protocol error waiting for compression response.");
            return(-1);
        }
    }
    /* Allocate a pseudo tty if appropriate. */
    if (tty_flag) {
        debug(F110,"SSH","Requesting pty.",0);

        /* Start the packet. */
        packet_start(SSH_CMSG_REQUEST_PTY);

        /* Store TERM in the packet.  There is no limit on the
           length of the string. */
        cp = getterm();
        if (!cp)
            cp = "";
        packet_put_cstring(cp);

        /* Store window size in the packet. */
#ifdef OS2
        if (_getwinsize( &ws ) == -1)
            memset(&ws,0,sizeof(ws));
#else
        if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) < 0)
            memset(&ws, 0, sizeof(ws));
#endif
        received_window_change_signal = 0;
        sent_initial_ws = 1;
        ssh_width = ws.ws_col;
        ssh_height = ws.ws_row;
        packet_put_int(ws.ws_row);
        packet_put_int(ws.ws_col);
        packet_put_int(ws.ws_xpixel);
        packet_put_int(ws.ws_ypixel);

        /* Store tty modes in the packet. */
        tty_make_modes(fileno(stdin), NULL);

        /* Send the packet, and wait for it to leave. */
        packet_send();
        packet_write_wait();

        /* Read response from the server. */
        type = packet_read();
        if (type == SSH_SMSG_SUCCESS) {
            interactive = 1;
            have_tty = 1;
        } else if (type == SSH_SMSG_FAILURE)
            log("Warning: Remote host failed or refused to allocate a pseudo tty.");
        else {
            packet_disconnect("Protocol error waiting for pty request response.");
            return -1;
        }
    }
    /* Request X11 forwarding if enabled. (display is checked in x11_get_proto) */
    if (options.forward_x11) 
    {
        char *proto, *data;
        /* Get reasonable local authentication information. */
        x11_get_proto(&proto, &data);
        /* Request forwarding with authentication spoofing. */
        debug(F110,"SSH","Requesting X11 forwarding with authentication spoofing.",0);
        x11_request_forwarding_with_spoofing(0, proto, data);

        /* Read response from the server. */
        type = packet_read();
        if (type == SSH_SMSG_SUCCESS) {
            interactive = 1;
        } else if (type == SSH_SMSG_FAILURE) {
            log("Warning: Remote host denied X11 forwarding.");
        } else {
            packet_disconnect("Protocol error waiting for X11 forwarding");
            return -1;
        }
    }
    /* Tell the packet module whether this is an interactive session. */
    packet_set_interactive(interactive);

    /* Request authentication agent forwarding if appropriate. */
    check_agent_present();

    if (options.forward_agent) {
        debug(F110,"SSH","Requesting authentication agent forwarding.",0);
        auth_request_forwarding();

        /* Read response from the server. */
        type = packet_read();
        packet_check_eom();
        if (type != SSH_SMSG_SUCCESS)
            log("Warning: Remote host denied authentication agent forwarding.");
    }

    /* Initiate port forwardings. */
    ssh_init_forwarding();

    /* If requested, let ssh continue in the background. */
    if (fork_after_authentication_flag)
    {
#ifdef OS2
        fprintf(stderr,
                 "fork after authentication is not implemented on Win32 environment\n");
        exit(1);
#else
        if (daemon(1, 1) < 0)
            fatal("daemon() failed: %.200s", strerror(errno));
#endif /* OS2 */
    }

    /*
     * If a command was specified on the command line, execute the
     * command now. Otherwise request the server to start a shell.
     */
    if (buffer_len(&command) > 0) {
        int len = buffer_len(&command);
        if (len > 900)
            len = 900;
        debug1("Sending command: %.*s", len, (u_char *)buffer_ptr(&command));
        packet_start(SSH_CMSG_EXEC_CMD);
        packet_put_string(buffer_ptr(&command), buffer_len(&command));
        packet_send();
        packet_write_wait();
    } else {
        debug1("Requesting shell.");
        packet_start(SSH_CMSG_EXEC_SHELL);
        packet_send();
        packet_write_wait();
    }

    /* Enter the interactive session. */
#ifdef OS2
    /* Start the Client Loop Thread */
    clientloopID = 0;
    hClientLoopThread = CreateThread(NULL, 0, ClientLoopThread,
                                      &clientloopID, 0, &ClientLoopThreadID);
    msleep(100);
    open_connection = 1;
    return(0);
#else
    return client_loop(have_tty, tty_flag ? options.escape_char : SSH_ESCAPECHAR_NONE, 0);
#endif /* OS2 */
}

static int
client_subsystem_reply(int type, u_int32_t seq, void *ctxt)
{
    int id, len;

    id = packet_get_int();
    len = buffer_len(&command);
    if (len > 900)
        len = 900;
    packet_check_eom();
    if (type == SSH2_MSG_CHANNEL_FAILURE) {
        fatal("Request for subsystem '%.*s' failed on channel %d",
               len, (u_char *)buffer_ptr(&command), id);
        quit_pending = 1;
        return(-1);
    }
    return(0);
}

static int
client_global_request_reply(int type, u_int32_t seq, void *ctxt)
{
    int i;

    i = client_global_request_id++;
    if (i >= options.num_remote_forwards) {
        debug1("client_global_request_reply: too many replies %d > %d",
               i, options.num_remote_forwards);
        return 0;
    }
    debug1("remote forward %s for: listen %d, connect %s:%d",
           type == SSH2_MSG_REQUEST_SUCCESS ? "success" : "failure",
           options.remote_forwards[i].port,
           options.remote_forwards[i].host,
           options.remote_forwards[i].host_port);
    if (type == SSH2_MSG_REQUEST_FAILURE)
        log("Warning: remote port forwarding failed for listen port %d",
             options.remote_forwards[i].port);
    return 0;
}

/* request pty/x11/agent/tcpfwd/shell for channel */
static void
ssh_session2_setup(int id, void *arg)
{
    int len;
    int interactive = 0;
#ifndef OS2
    struct termios tio;
#endif /* OS2 */

    debug2("ssh_session2_setup: id: %d arg: ",id);

    if (tty_flag) {
        struct winsize ws;
        char *cp;
        cp = getterm();
        if (!cp)
            cp = "";
        /* Store window size in the packet. */
#ifdef OS2
        if (_getwinsize( &ws ) == -1)
            memset(&ws,0,sizeof(ws));
#else
        if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) < 0)
            memset(&ws, 0, sizeof(ws));
#endif
        received_window_change_signal = 0;
        sent_initial_ws = 1;
        ssh_width = ws.ws_col;
        ssh_height = ws.ws_row;
        channel_request_start(id, "pty-req", 0);
        packet_put_cstring(cp);
        packet_put_int(ws.ws_col);
        packet_put_int(ws.ws_row);
        packet_put_int(ws.ws_xpixel);
        packet_put_int(ws.ws_ypixel);

#ifdef OS2
        tty_make_modes(0,0);
#else
        tio = get_saved_tio();
        tty_make_modes(/*ignored*/ 0, &tio);
#endif /* OS2 */
        packet_send();
        interactive = 1;
        /* XXX wait for reply */
    }
    if (options.forward_x11) {
        char *proto, *data;
        /* Get reasonable local authentication information. */
        x11_get_proto(&proto, &data);
        /* Request forwarding with authentication spoofing. */
        debug1("Requesting X11 forwarding with authentication spoofing.");
        x11_request_forwarding_with_spoofing(id, proto, data);
        interactive = 1;
        /* XXX wait for reply */
    }

    check_agent_present();
    if (options.forward_agent) {
        debug1("Requesting authentication agent forwarding.");
        channel_request_start(id, "auth-agent-req@openssh.com", 0);
        packet_send();
    }

    len = buffer_len(&command);
    if (len > 0) {
        if (len > 900)
            len = 900;
        if (subsystem_flag) {
            debug1("Sending subsystem: %.*s", len, (u_char *)buffer_ptr(&command));
            channel_request_start(id, "subsystem", /*want reply*/ 1);
            /* register callback for reply */
            /* XXX we assume that client_loop has already been called */
            dispatch_set(SSH2_MSG_CHANNEL_FAILURE, &client_subsystem_reply);
            dispatch_set(SSH2_MSG_CHANNEL_SUCCESS, &client_subsystem_reply);
        } else {
            debug1("Sending command: %.*s",len, (u_char *)buffer_ptr(&command));
            channel_request_start(id, "exec", 0);
        }
        packet_put_string(buffer_ptr(&command), buffer_len(&command));
        packet_send();
    } else {
        channel_request_start(id, "shell", 0);
		packet_send();
    }
    packet_set_interactive(interactive);
}

int
ssh_session2_open(void)
{
    Channel *c;
    int window, packetmax;
    int in, out, err;

#ifdef OS2
    in = sock_stdin;
    out = sock_stdout;
    err = sock_stderr;
#else
    if (stdin_null_flag) {
        in = open(_PATH_DEVNULL, O_RDONLY);
    } else {
        in = dup(STDIN_FILENO);
    }
    out = dup(STDOUT_FILENO);
    err = dup(STDERR_FILENO);

    if (in < 0 || out < 0 || err < 0) {
        fatal("dup() in/out/err failed");
        return(-1);
    }

    /* enable nonblocking unless tty */
    if (!isatty(in))
        set_nonblock(in);
    if (!isatty(out))
        set_nonblock(out);
    if (!isatty(err))
        set_nonblock(err);
#endif /* OS2 */

    window = CHAN_SES_WINDOW_DEFAULT;
    packetmax = CHAN_SES_PACKET_DEFAULT;
    if (!tty_flag) {
        window >>= 1;
        packetmax >>= 1;
    }
    c = channel_new("session", SSH_CHANNEL_OPENING, in, out, err,
                      window, packetmax, CHAN_EXTENDED_WRITE,
                      strdup("client-session"), /*nonblock*/0);
    if ( c == NULL ) {
        debug1("ssh_session2_open: channel_new failed");
        return -1;
    }
    debug3("ssh_session2_open: channel_new: %d", c->self);

    channel_send_open(c->self);
    if ( !no_shell_flag )
        channel_register_confirm(c->self, ssh_session2_setup);

    return c->self;
}

static int
ssh_session2(void)
{
    int id = -1;

    /* XXX should be pre-session */
    ssh_init_forwarding();

    if ( !no_shell_flag || (datafellows & SSH_BUG_DUMMYCHAN) )
        id = ssh_session2_open();

    /* If requested, let ssh continue in the background. */
    if (fork_after_authentication_flag) {
#ifdef OS2
        fprintf(stderr,
                 "fork after authentication is not implemented on Win32 environment\n");
        return(-1);
#else
        if (daemon(1, 1) < 0) {
            fatal("daemon() failed: %.200s", strerror(errno));
            return(-1);
        }
#endif /* OS2 */
    }
#ifdef OS2
    /* Start the Client Loop Thread */
    clientloopID = id;
    hClientLoopThread = CreateThread(NULL, 0, ClientLoopThread,
                                      &clientloopID, 0, &ClientLoopThreadID);

    open_connection = 1;
    return(0);
#else
    return client_loop(tty_flag, tty_flag ? options.escape_char : SSH_ESCAPECHAR_NONE, id);
#endif /* OS2 */
}

static void
load_public_identity_files(void)
{
    char *filename;
    int i = 0;
    Key *public;

#ifdef SMARTCARD
    Key **keys;

    if (options.smartcard_device != NULL &&
         options.num_identity_files < SSH_MAX_IDENTITY_FILES &&
         (keys = sc_get_keys(options.smartcard_device, NULL)) != NULL ) {
        int count = 0;
        for (i = 0; keys[i] != NULL; i++) {
                count++;
                memmove(&options.identity_files[1], &options.identity_files[0],
                    sizeof(char *) * (SSH_MAX_IDENTITY_FILES - 1));
                memmove(&options.identity_keys[1], &options.identity_keys[0],
                    sizeof(Key *) * (SSH_MAX_IDENTITY_FILES - 1));
                options.num_identity_files++;
                options.identity_keys[0] = keys[i];
                options.identity_files[0] = strdup("smartcard key");;
        }
        if (options.num_identity_files > SSH_MAX_IDENTITY_FILES)
                options.num_identity_files = SSH_MAX_IDENTITY_FILES;
        i = count;
        free(keys);
    }
#endif /* SMARTCARD */
    for (; i < options.num_identity_files; i++) {
        filename = tilde_expand_filename(options.identity_files[i], -1);
        public = key_load_public(filename, NULL);
        debug1("identity file: \"%s\" type: %d", filename,
               public ? public->type : -1);
        free(options.identity_files[i]);
        options.identity_files[i] = filename;
        options.identity_keys[i] = public;
    }
}

/*
 * Creates a (possibly privileged) socket for use as the ssh connection.
 */
int
ssh_create_socket(int privileged, int family)
{
    extern char * tcp_address;
    SOCKET sock;

    /*
     * If we are running as root and want to connect to a privileged
     * port, bind our own socket to a privileged port.
     */
    if (privileged) {
        int p = IPPORT_RESERVED - 1;
        sock = rresvport_af(&p, family);
        if (sock < 0)
            error("rresvport: af=%d %.100s", family, strerror(errno));
        else
            debug1("Allocated local port: %d", p);
    } else {
        sock = socket(family, SOCK_STREAM, 0);
#ifdef OS2
        if (sock == SOCKET_ERROR)
            error("socket: %.100s", sock_strerror(sock_lasterror()));
#else
        if (sock < 0)
            error("socket: %.100s", strerror(errno));
#endif /* OS2 */
    }

#ifndef datageneral
    if (tcp_address && family == AF_INET) {
        struct sockaddr_in sin;
#ifdef INADDRX
        struct in_addr inaddrx;
#endif /* INADDRX */
        int s_errno;

        debug1("ssh_create_socket binding socket to: %s",tcp_address);
        memset((char *)&sin,0,sizeof(sin));
        sin.sin_family = family;
#ifdef INADDRX
        inaddrx = inet_addr(tcp_address);
        sin.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
        sin.sin_addr.s_addr = inet_addr(tcp_address);
#endif /* INADDRX */
        sin.sin_port = 0;
        if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            s_errno = WSAGetLastError(); /* Save error code */
#ifdef TCPIPLIB
            socket_close(sock);
#else /* TCPIPLIB */
            close(sock);
#endif /* TCPIPLIB */
            sock = -1;
            errno = s_errno;       /* and report this error */
            debug1("ssh_create_socket bind errno: %d",errno);
            return(-1);
        }
    }
#endif /* datageneral */

    return sock;
}

#ifndef INET6_ADDRSTRLEN                /* for non IPv6 machines */
#define INET6_ADDRSTRLEN 46
#endif

static const char *
sockaddr_ntop(struct sockaddr *sa)
{
    void *addr;
    static char addrbuf[INET6_ADDRSTRLEN];

    switch (sa->sa_family) {
    case AF_INET:
        addr = &((struct sockaddr_in *)sa)->sin_addr;
        break;
    case AF_INET6:
        addr = &((struct sockaddr_in6 *)sa)->sin6_addr;
        break;
    default:
        /* This case should be protected against elsewhere */
        abort();
    }
    inet_ntop(sa->sa_family, addr, addrbuf, sizeof(addrbuf));
    return addrbuf;
}

static int show_other_keys(const char *, Key *);

/*
 * Opens a TCP/IP connection to the remote server on the given host.
 * The address of the remote host will be returned in hostaddr.
 * If port is 0, the default port will be used.  If needpriv is true,
 * a privileged port will be allocated to make the connection.
 * This requires super-user privileges if needpriv is true.
 * Connection_attempts specifies the maximum number of tries (one per
 * second).  If proxy_command is non-NULL, it specifies the command (with %h
 * and %p substituted for host and port, respectively) to use to contact
 * the daemon.
 * Return values:
 *    0 for OK
 *    ECONNREFUSED if we got a "Connection Refused" by the peer on any address
 *    ECONNABORTED if we failed without a "Connection refused"
 * Suitable error messages for the connection failure will already have been
 * printed.
 */
int
ssh_connect(const char *host, struct sockaddr_storage * hostaddr,
            u_short port, int family, int connection_attempts,
            int needpriv, const char *proxy_command)
{
    int gaierr;
    int on = 1;
    int attempt, i;
    char ntop[NI_MAXHOST], strport[NI_MAXSERV];
    struct addrinfo hints, *ai = NULL, *aitop = NULL;
    struct servent *sp = NULL;
    /*
    * Did we get only other errors than "Connection refused" (which
    * should block fallback to rsh and similar), or did we get at least
    * one "Connection refused"?
    */
    int full_failure = 1;
#ifndef NOHTTP
    char * proxy_user=NULL, * proxy_pwd=NULL;
    char uid[UIDBUFLEN];
    char pwd[256];
#endif /* NOHTTP */

#ifdef UNIX
    debug2("ssh_connect: needpriv %d", needpriv);
#endif /* UNIX */

    /* Get default port if port has not been set. */
    if (port == 0) {
        sp = getservbyname(SSH_SERVICE_NAME, "tcp");
        if (sp)
            port = ntohs(sp->s_port);
        else
            port = SSH_DEFAULT_PORT;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(strport, sizeof strport, "%d", port);
    if ((gaierr = getaddrinfo(host, strport, &hints, &aitop)) != 0) {
        fatal("ssh: %.100s: %s", host, gai_strerror(gaierr));
        return -1;
    }

    /*
     * Try to connect several times.  On some machines, the first time
     * will sometimes fail.  In general socket code appears to behave
     * quite magically on many machines.
     */
    for (attempt = 0;;) {
        if (attempt > 0 && !quiet)
            printf("Trying again...");

        /* Loop through addresses for this host, and try each one in
           sequence until the connection succeeds. */
        for (ai = aitop; ai; ai = ai->ai_next) {
            if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
                continue;
            if (getnameinfo(ai->ai_addr, ai->ai_addrlen,
                             ntop, sizeof(ntop), strport, sizeof(strport),
                             NI_NUMERICHOST|NI_NUMERICSERV) != 0) {
                error("ssh_connect: getnameinfo failed");
                continue;
            }
            debug1("Connecting to %s:%s %s",host,strport,ntop);

#ifndef NOHTTP
            if (tcp_http_proxy) {
                struct   sockaddr_in sa_proxy;
#ifdef OS2
                char * agent = "Kermit 95"; /* Default user agent */
#else
                char * agent = "C-Kermit";
#endif /* OS2 */
                register struct hostent *hp = 0;
                struct servent *destsp;
                char proxyhost[128], *p, *q;
                char proxyto[128];
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
                int tos;
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */
                int s;
#ifdef DEBUG
                extern int debtim;
                int xdebtim;
                xdebtim = debtim;
                debtim = 1;
#endif /* DEBUG */

                ckmakmsg(proxyto,512,(char*)host,":",strport,NULL);

                memset((char *)&sa_proxy, 0, sizeof (sa_proxy));
                for (p = tcp_http_proxy, q=proxyhost; *p != '\0' && *p != ':'; p++, q++)
                    *q = *p;
                *q = '\0';

                sa_proxy.sin_addr.s_addr = inet_addr(proxyhost);
                if (sa_proxy.sin_addr.s_addr != -1) {
                    debug(F110,"ssh_connect A",proxyhost,0);
                    sa_proxy.sin_family = AF_INET;
                } else {
                    debug(F110,"ssh_connect B",proxyhost,0);
                    hp = gethostbyname(proxyhost);
#ifdef HADDRLIST
                    hp = ck_copyhostent(hp); /* make safe copy that won't change */
#endif /* HADDRLIST */
                    if (hp == NULL) {
                        fprintf(stderr, "ssh: %s: Unknown host", proxyhost);
#ifdef DEBUG
                        debtim = xdebtim;
#endif /* DEBUG */
                        goto next_ai;
                    }
                    sa_proxy.sin_family = hp->h_addrtype;
#ifdef HADDRLIST
                    memcpy((char *)&sa_proxy.sin_addr, hp->h_addr_list[0],
                            sizeof(sa_proxy.sin_addr));
#else /* HADDRLIST */
                    memcpy((char *)&sa_proxy.sin_addr, hp->h_addr,
                            sizeof(sa_proxy.sin_addr));
#endif /* HADDRLIST */
                }

              http_restart:
                ssh_sock = ssh_create_socket(needpriv, ai->ai_family);
                debug(F101,"ssh_connect socket","",ssh_sock);
                if (ssh_sock < 0) {
                    perror("ssh: socket");
#ifdef DEBUG
                    debtim = xdebtim;
#endif /* DEBUG */
                    goto next_ai;
                }
                if (*p == ':')
                    p++;
                else
                    p = "http";

                destsp = getservbyname(p,"tcp");
                if (destsp)
                    sa_proxy.sin_port = destsp->s_port;
                else if (p)
                    sa_proxy.sin_port = htons(atoi(p));
                else
                    sa_proxy.sin_port = htons(80);
                errno = 0;
#ifdef HADDRLIST
                debug(F100,"ssh_connect HADDRLIST","",0);
                while
#else
                debug(F100,"ssh_connect no HADDRLIST","",0);
                if
#endif /* HADDRLIST */
                    (connect(ssh_sock, (struct sockaddr *)&sa_proxy,
                              sizeof (sa_proxy)) < 0) 
                {
                    debug(F101,"ssh_connect connect failed","",errno);
#ifdef HADDRLIST
                    if (hp && hp->h_addr_list[1]) 
                    {
                        int oerrno = errno;

                        fprintf(stderr,
                                 "ssh: connect to address %s: ",
                                 inet_ntoa(sa_proxy.sin_addr)
                                 );
                        errno = oerrno;
                        perror((char *)0);
                        hp->h_addr_list++;
                        memcpy((char *)&sa_proxy.sin_addr,
                                hp->h_addr_list[0],
                                sizeof(sa_proxy.sin_addr));
                        if ( !quiet )
                            fprintf(stdout, "Trying %s...\n",
                                 inet_ntoa(sa_proxy.sin_addr));
                        shutdown(ssh_sock, SHUT_RDWR);
#ifdef TCPIPLIB
                        socket_close(ssh_sock);
#else /* TCPIPLIB */
                        close(ssh_sock);
#endif /* TCPIPLIB */
                        ssh_sock = ssh_create_socket(needpriv, ai->ai_family);
                        if (ssh_sock < 0) {
                            perror("ssh: socket");
#ifdef DEBUG
                            debtim = xdebtim;
#endif /* DEBUG */
                            goto next_ai;
                        }
                        continue;
                    }
#endif /* HADDRLIST */
                    perror("ssh: connect");
                    goto next_attempt;
                }
                if (http_connect(ssh_sock,
                                 tcp_http_proxy_agent ? tcp_http_proxy_agent : agent,
                                 NULL,
                                 tcp_http_proxy_user,
                                 tcp_http_proxy_pwd,
                                 0,
                                 proxyto
                                 ) < 0) 
                {

                    shutdown(ssh_sock, SHUT_RDWR);
#ifdef TCPIPLIB
                    socket_close(ssh_sock);
#else /* TCPIPLIB */
                    close(ssh_sock);
#endif /* TCPIPLIB */
                    
                    if (tcp_http_proxy_errno == 401 ||
                         tcp_http_proxy_errno == 407 ) 
                    {
                        struct txtbox tb[2];
                        int ok;

                        tb[0].t_buf = uid;
                        tb[0].t_len = UIDBUFLEN;
                        tb[0].t_lbl = "Proxy Userid: ";
                        tb[0].t_dflt = NULL;
                        tb[0].t_echo = 1;
                        tb[1].t_buf = pwd;
                        tb[1].t_len = 256;
                        tb[1].t_lbl = "Proxy Passphrase: ";
                        tb[1].t_dflt = NULL;
                        tb[1].t_echo = 2;

                        ok = uq_mtxt("Proxy Server Authentication Required\n",
                                      NULL, 2, tb);

                        if (ok && uid[0]) {
                            if ( !proxy_user ) {
                                proxy_user = tcp_http_proxy_user;
                                tcp_http_proxy_user = uid;
                            }
                            if ( !proxy_user ) {
                                proxy_pwd  = tcp_http_proxy_pwd;
                                tcp_http_proxy_pwd = pwd;
                            }
                            goto http_restart;
                        }
                    }
                    perror("ssh: connect");
                    goto next_attempt;
                } else {
                    /* Successful connection. */
                    memcpy(hostaddr, ai->ai_addr, ai->ai_addrlen);
                    break;
                }
            } else
#endif /* NOHTTP */
            {
                /* Create a socket for connecting. */
                ssh_sock = ssh_create_socket(needpriv, ai->ai_family);
                if (ssh_sock < 0)
                    continue;

                if (connect(ssh_sock, ai->ai_addr, ai->ai_addrlen) >= 0) {
                    /* Successful connection. */
                    memcpy(hostaddr, ai->ai_addr, ai->ai_addrlen);
                    break;
                } else {
                    if (sock_lasterror() == WSAECONNREFUSED)
                        full_failure = 0;
                    log("ssh: connect to address %s port %s: %s",
                         sockaddr_ntop(ai->ai_addr), strport,
#ifdef OS2
                         sock_strerror(sock_lasterror())
#else
                         strerror(errno)
#endif /* OS2 */
                         );
                    /*
                    * Close the failed socket; there appear to
                    * be some problems when reusing a socket for
                    * which connect() has already returned an
                    * error.
                    */
                    shutdown(ssh_sock, SHUT_RDWR);
#ifdef OS2
                    closesocket(ssh_sock);
#else
                    close(ssh_sock);
#endif /* OS2 */
                }
            }
          next_ai:
            ;
        }
        if (ai)
            break;      /* Successful connection. */

      next_attempt:
        if ( ++attempt >= connection_attempts )
            break;

        /* Sleep a moment before retrying. */
        sleep(1);
    }

#ifndef NOHTTP
    if ( proxy_user )
        tcp_http_proxy_user = proxy_user;
    if ( proxy_pwd ) {
        memset(pwd,0,sizeof(pwd));
        tcp_http_proxy_pwd = proxy_pwd;
    }
#endif /* NOHTTP */

    freeaddrinfo(aitop);

    /* Return failure if we didn't get a successful connection. */
    if (attempt >= connection_attempts) {
        log("ssh: connect to host %s port %s: %s",
             host, strport, sock_strerror(sock_lasterror()));
        return full_failure ? WSAECONNABORTED : WSAECONNREFUSED;
    }
    debug1("Connection established.");

#ifndef NOTCPOPTS
#ifndef datageneral
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
    no_delay(ssh_sock,tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
    keepalive(ssh_sock,tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
    ck_linger(ssh_sock,tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(ssh_sock,tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(ssh_sock,tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* datageneral */
#endif /* NOTCPOPTS */

    /* Set the connection. */
    packet_set_connection(ssh_sock, ssh_sock);
    return 0;
}

static int minor1 = PROTOCOL_MINOR_1;
char *
ssh_proto_ver(void)
{
    static char buf[16];
    snprintf(buf, sizeof buf, "SSH-%d.%d",
              compat20 ? PROTOCOL_MAJOR_2 : PROTOCOL_MAJOR_1,
              compat20 ? PROTOCOL_MINOR_2 : minor1);
    return buf;
}

/*
 * Waits for the server identification string, and sends our own
 * identification string.
 */
static int
ssh_exchange_identification(void)
{
    char buf[256], remote_version[256]; /* must be same size! */
    int remote_major, remote_minor, i, mismatch;
    int connection_in = packet_get_connection_in();
    int connection_out = packet_get_connection_out();

    minor1 = PROTOCOL_MINOR_1;

    /* Read other side\'s version identification. */
    for (;;) {
        for (i = 0; i < sizeof(buf) - 1; i++) {
#ifdef OS2
            int len;
#ifdef BSDSELECT
            fd_set rfds;
            struct timeval tv;
            int ready;

            FD_ZERO(&rfds);
            FD_SET(connection_in, &rfds);
            tv.tv_sec = 30;
            tv.tv_usec = 0L;

            ready = ((select(FD_SETSIZE, &rfds, NULL, NULL, &tv) > 0) &&
               FD_ISSET(connection_in, &rfds));
            if ( !ready ) {
                fatal("ssh_exchange_identification: select: 30 second timeout");
                return -1;
            }
#endif /* BSDSELECT */

            len = recv(connection_in, &buf[i], 1, 0);
            if (len < 0) {
                fatal("ssh_exchange_identification: read: %.100s",
                       sock_strerror(sock_lasterror()));
                return -1;
            }
#else
            int len = atomicio(read, connection_in, &buf[i], 1);
            if (len < 0) {
                fatal("ssh_exchange_identification: read: %.100s", strerror(errno));
                return -1;
            }
#endif /* OS2 */
            if (len != 1) {
                fatal("ssh_exchange_identification: Connection closed by remote host");
                return -1;
            }
            if (buf[i] == '\r') {
                buf[i] = '\n';
                buf[i + 1] = 0;
                continue;               /**XXX wait for \n */
            }
            if (buf[i] == '\n') {
                buf[i + 1] = 0;
                break;
            }
        }
        buf[sizeof(buf) - 1] = 0;
        if (strncmp(buf, "SSH-", 4) == 0)
            break;
        debug1("ssh_exchange_identification: %s", buf);
    }
    server_version_string = strdup(buf);

    /*
     * Check that the versions match.  In future this might accept
     * several versions and set appropriate flags to handle them.
     */
    if (sscanf(server_version_string, "SSH-%d.%d-%[^\n]\n",
                &remote_major, &remote_minor, remote_version) != 3) {
        fatal("Bad remote protocol version identification: '%.100s'", buf);
        return -1;
    }
    debug1("Remote protocol version (major) %d (minor) %d",
            remote_major, remote_minor);
    debug1("Remote software version: %s",remote_version);

    compat_datafellows(remote_version);
    mismatch = 0;

    switch(remote_major) {
    case 1:
        if (remote_minor == 99 &&
             (options.protocol & SSH_PROTO_2) &&
             !(options.protocol & SSH_PROTO_1_PREFERRED)) {
            enable_compat20();
            break;
        }
        if (!(options.protocol & SSH_PROTO_1)) {
            mismatch = 1;
            break;
        }
        if (remote_minor < 3) {
            fatal("Remote machine has too old SSH software version.");
            return -1;
        } else if (remote_minor == 3 || remote_minor == 4) {
            /* We speak 1.3, too. */
            enable_compat13();
            minor1 = 3;
            if (options.forward_agent) {
                log("Agent forwarding disabled for protocol 1.3");
                options.forward_agent = 0;
            }
        }
        break;
    case 2:
        if (options.protocol & SSH_PROTO_2) {
            enable_compat20();
            break;
        }
        /* FALLTHROUGH */
    default:
        mismatch = 1;
        break;
    }
    if (mismatch)
        fatal("Protocol major versions differ: %d vs. %d",
               (options.protocol & SSH_PROTO_2) ? PROTOCOL_MAJOR_2 : PROTOCOL_MAJOR_1,
               remote_major);
    /* Send our own protocol version identification. */
    snprintf(buf, sizeof buf, "SSH-%d.%d-%.100s\n",
              compat20 ? PROTOCOL_MAJOR_2 : PROTOCOL_MAJOR_1,
              compat20 ? PROTOCOL_MINOR_2 : minor1,
              SSH_VERSION);
#ifdef OS2
    if (send(connection_out, buf, strlen(buf), 0) != strlen(buf)) {
        fatal("write: %.100s", sock_strerror(sock_lasterror()));
        return -1;
    }
#else
    if (atomicio(write, connection_out, buf, strlen(buf)) != strlen(buf)) {
        fatal("write: %.100s", strerror(errno));
        return -1;
    }
#endif /* OS2 */
    client_version_string = strdup(buf);
    chop(client_version_string);
    chop(server_version_string);
    debug1("Local version string: %s", client_version_string);
    return 0;
}

/* defaults to 'no' */
static int
confirm(const char *preface, const char *prompt, int defval)
{
    char buf[1024];
    FILE *f;
    int retval = -1;

    if (options.batch_mode)
        return 0;
    retval = uq_ok((char *)preface, (char *)prompt, 3, NULL, defval);
    if ( retval < 0 )
        retval = defval;
    return(retval);
}

/*
 * check whether the supplied host key is valid, return -1 if the key
 * is not valid. the user_hostfile will not be updated if 'readonly' is true.
 */

int
check_host_key(char *host, struct sockaddr *hostaddr, Key *host_key,
        int readonly, const char *user_hostfile, const char *system_hostfile)
{
    Key *file_key;
    char *type = key_type(host_key);
    char *ip = NULL;
    char hostline[1000], *hostp, *fp;
    HostStatus host_status;
    HostStatus ip_status;
    int local = 0, host_ip_differ = 0;
    int salen;
    char ntop[NI_MAXHOST];
    char msg[1024];
    int len, host_line, ip_line, has_keys;
    const char *host_file = NULL, *ip_file = NULL;

    /*
     * Force accepting of the host key for loopback/localhost. The
     * problem is that if the home directory is NFS-mounted to multiple
     * machines, localhost will refer to a different machine in each of
     * them, and the user will get bogus HOST_CHANGED warnings.  This
     * essentially disables host authentication for localhost; however,
     * this is probably not a real problem.
     */
    /**  hostaddr == 0! */
    switch (hostaddr->sa_family) {
    case AF_INET:
        local = (ntohl(((struct sockaddr_in *)hostaddr)->sin_addr.s_addr) >> 24) == IN_LOOPBACKNET;
        salen = sizeof(struct sockaddr_in);
        break;
    case AF_INET6:
        local = IN6_IS_ADDR_LOOPBACK(&(((struct sockaddr_in6 *)hostaddr)->sin6_addr));
        salen = sizeof(struct sockaddr_in6);
        break;
    default:
        local = 0;
        salen = sizeof(struct sockaddr_storage);
        break;
    }
    if (options.no_host_authentication_for_localhost == 1 &&
        local && options.host_key_alias == NULL) {
        debug1("Forcing accepting of host key for "
               "loopback/localhost.");
        return 0;
    }

    /*
     * We don't have the remote ip-address for connections
     * using a proxy command
     */
    if (!tcp_http_proxy) {
        if (getnameinfo(hostaddr, salen, ntop, sizeof(ntop),
                         NULL, 0, NI_NUMERICHOST) != 0) {
            fatal("check_host_key: getnameinfo failed");
            return -1;
        }
        ip = strdup(ntop);
    } else {
        ip = strdup("<no hostip for proxy command>");
    }
    /*
     * Turn off check_host_ip if the connection is to localhost, via proxy
     * command or if we don't have a hostname to compare with
     */
    if (options.check_host_ip &&
         (local || strcmp(host, ip) == 0 || tcp_http_proxy))
        options.check_host_ip = 0;

    /*
     * Allow the user to record the key under a different name. This is
     * useful for ssh tunneling over forwarded connections or if you run
     * multiple sshd's on different ports on the same machine.
     */
    if (options.host_key_alias != NULL) {
        host = options.host_key_alias;
        debug1("using hostkeyalias: %s", host);
    }

    /*
     * Store the host key from the known host file in here so that we can
     * compare it with the key for the IP address.
     */
    file_key = key_new(host_key->type);

    /*
     * Check if the host key is present in the user\'s list of known
     * hosts or in the systemwide list.
     */
    host_file = user_hostfile;
    host_status = check_host_in_hostfile(host_file, host, host_key, file_key, &host_line);
    if (host_status == HOST_NEW) {
        host_file = system_hostfile;
        host_status = check_host_in_hostfile(host_file, host, host_key, file_key, &host_line);
    }
    /*
     * Also perform check for the ip address, skip the check if we are
     * localhost or the hostname was an ip address to begin with
     */
    if (options.check_host_ip) {
        Key *ip_key = key_new(host_key->type);

        ip_file = user_hostfile;
        ip_status = check_host_in_hostfile(ip_file, ip, host_key, ip_key, &ip_line);
        if (ip_status == HOST_NEW) {
            ip_file = system_hostfile;
            ip_status = check_host_in_hostfile(ip_file, ip, host_key, ip_key, &ip_line);
        }
        if (host_status == HOST_CHANGED &&
             (ip_status != HOST_CHANGED || !key_equal(ip_key, file_key)))
            host_ip_differ = 1;

        key_free(ip_key);
    } else
        ip_status = host_status;

    key_free(file_key);

    switch (host_status) {
    case HOST_OK:
        /* The host is known and the key matches. */
        debug1("Host is known: %s and matches the host key: %s found in %s (%d)",
                host,type,host_file,host_line);
        if (options.check_host_ip && ip_status == HOST_NEW) {
            if (readonly)
                log("%s host key for IP address "
                     "'%.128s' not in list of known hosts.",
                     type, ip);
            else if (!add_host_to_hostfile(user_hostfile, ip, host_key))
                log("Failed to add the %s host key for IP address '%.128s' to the list of known hosts (%.30s).",
                     type, ip, user_hostfile);
            else
                log("Warning: Permanently added the %s host key for IP address '%.128s' to the list of known hosts.",
                     type, ip);
        }
        break;
    case HOST_NEW:
        /* The host is new. */
        if ( readonly )
            goto fail;
        if (options.strict_host_key_checking == 1) {
            /* User has requested strict host key checking.  We will not add the host key
            automatically.  The only alternative left is to abort. */
            ckmakxmsg(errbuf,ERRBUFSZ,
                       "No ",type," host key is known for ",host,
                       " and\n"
                       "you have requested strict host checking.\n"
                       "If you wish to make an untrusted connection,\n"
                       "SET SSH STRICT-HOST-KEY-CHECK OFF and try again.",
                       NULL,NULL,NULL,NULL,NULL,NULL,NULL);
            error(errbuf);
#ifdef KUI
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */
            goto fail;
        } else if (options.strict_host_key_checking == 2) {
            has_keys = show_other_keys(host, host_key);
            /* The default */
            fp = key_fingerprint(host_key, SSH_FP_MD5, SSH_FP_HEX);
            snprintf(msg, sizeof(msg),
                      "The authenticity of host '%.200s (%s)' can't be established%s\n"
                      "%s key fingerprint is %s.\n",
                      host, ip, 
                      has_keys ? ",\nbut keys of different type are already known for this host." : ".",
                      type, fp);
            free(fp);
            if (!confirm(msg, 
                         "Are you sure you want to continue connecting (yes/no)? ",
                         -1)) {
                error("Aborted by user!");
                goto fail;
            }
        } else if (options.strict_host_key_checking != 0) {
            error("Invalid Strict Host Key Check value!");
            goto fail;
        }
        if (options.check_host_ip && ip_status == HOST_NEW) {
            snprintf(hostline, sizeof(hostline), "%s,%s", host, ip);
            hostp = hostline;
        } else
            hostp = host;

        /* If not in strict mode, add the key automatically to the local known_hosts file. */
        if (!add_host_to_hostfile(user_hostfile, hostp, host_key))
            log("Failed to add the host to the list of known hosts (%.500s).",
                 user_hostfile);
        else
            log("Warning: Permanently added '%.200s' (%s) to the list of known hosts.",
                 hostp, type);
        break;
    case HOST_CHANGED:
        if (options.check_host_ip && host_ip_differ) {
            char *msg;
            if (ip_status == HOST_NEW)
                msg = "is unknown";
            else if (ip_status == HOST_OK)
                msg = "is unchanged";
            else
                msg = "has a different value";
            ckmakxmsg(errbuf,ERRBUFSZ,
                       "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
                       "@       WARNING: POSSIBLE DNS SPOOFING DETECTED!          @\n"
                       "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
                       "The ", type, " host key for ", host, " has changed,\n"
                       "and the key for the according IP address ", ip, "\n",
                       msg, ". This could either mean that\n",
                       "DNS SPOOFING is happening or the IP address for the host\n"
                       "and its host key have changed at the same time.\n",
                       NULL, NULL);
            if ( ip_status != HOST_NEW ) {
                ckstrncat(errbuf, "Offending key for IP in ", ERRBUFSZ);
                ckstrncat(errbuf, ip_file, ERRBUFSZ);
                ckstrncat(errbuf, ":", ERRBUFSZ);
                ckstrncat(errbuf, ckitoa(ip_line), ERRBUFSZ);
            }
            error(errbuf);
#ifdef KUI
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */

        }
        /* The host key has changed. */
        fp = key_fingerprint(host_key, SSH_FP_MD5, SSH_FP_HEX);
        ckmakxmsg(errbuf,ERRBUFSZ,
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
        "@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @\n"
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
        "IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!\n"
        "Someone could be eavesdropping on you right now (man-in-the-middle attack)!\n"
        "It is also possible that the ", type, " host key has just been changed.\n"
        "The fingerprint for the ", type, " key sent by the remote host is\n",
        fp, "\nPlease contact your system administrator.\n"
        "Add correct host key in ", (char*)user_hostfile, " to get rid of this message.\n"
        "Offending key in ", (char *)host_file, ":", ckitoa(host_line));
        free(fp);
        error(errbuf);
#ifdef KUI
        uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */

        /*
         * If strict host key checking is in use, the user will have
         * to edit the key manually and we can only abort.
         */
        if (options.strict_host_key_checking == 1) {
            ckmakmsg(errbuf,ERRBUFSZ, type, " host key for ", host,
                      "has changed and you have requested strict checking.");
            error(errbuf);
#ifdef KUI
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */
            goto fail;
        } else if (options.strict_host_key_checking == 2) {
            /* The default */
            char prompt[1024];
            fp = key_fingerprint(host_key, SSH_FP_MD5, SSH_FP_HEX);
            snprintf(prompt, sizeof(prompt),
                      "The authenticity of host '%.200s (%s)' can't be established.\n"
                      "%s key fingerprint is %s.\n",
                      host, ip, type, fp);
            free(fp);
            if (!confirm(prompt, 
                         "Are you sure you want to continue connecting (yes/no)? ",
                         -1)) {
                error("Aborted by user!");
                goto fail;
            }
        } else if (options.strict_host_key_checking != 0) {
            error("Invalid Strict Host Key Check value!");
            goto fail;
        }

        /*
         * If strict host key checking has not been requested, allow
         * the connection but without password authentication or
         * agent forwarding.
         */
        if (options.password_authentication) {
            error("Password authentication is disabled to avoid trojan horses.");
            options.password_authentication = 0;
        }
        if (options.forward_agent) {
            error("Agent forwarding is disabled to avoid trojan horses.");
            options.forward_agent = 0;
        }
        if (options.forward_x11) {
            error("X11 forwarding is disabled to avoid trojan horses.");
            options.forward_x11 = 0;
        }
        if (options.num_local_forwards > 0 || options.num_remote_forwards > 0) {
            error("Port forwarding is disabled to avoid trojan horses.");
            options.num_local_forwards = options.num_remote_forwards = 0;
        }
        /*
         * XXX Should permit the user to change to use the new id.
         * This could be done by converting the host key to an
         * identifying sentence, tell that the host identifies itself
         * by that sentence, and ask the user if he/she whishes to
         * accept the authentication.
         */
        break;
    case HOST_FOUND:
        error("Internal Error!");
        goto fail;
    }

    if (options.check_host_ip && host_status != HOST_CHANGED &&
         ip_status == HOST_CHANGED) {
        ckmakxmsg(errbuf,ERRBUFSZ,"Warning: the ",
                  type, " host key for '", host, "' differs from the key for the IP address '",
                  ip, "'\nOffending key for IP in ", (char *)ip_file, ":", ckitoa(ip_line), NULL, NULL);
        if (host_status == HOST_OK) {
            ckstrncat(errbuf, "\nMatching host key in ", ERRBUFSZ);
            ckstrncat(errbuf, host_file, ERRBUFSZ);
            ckstrncat(errbuf, ":", ERRBUFSZ);
            ckstrncat(errbuf, ckitoa(host_line), ERRBUFSZ);
        }

        if (options.strict_host_key_checking == 1) {
            log(errbuf);
            ckstrncat(errbuf, "\nConnection Failure: Strict Host Key Checking enabled.", ERRBUFSZ);
            error(errbuf);
#ifdef KUI
            uq_ok(NULL, errbuf, 1, NULL, 0);
#endif /* KUI */
            goto fail;
        } else if (options.strict_host_key_checking == 2) {
            ckstrncat(errbuf,"\n",
                      ERRBUFSZ);
            if (!confirm(errbuf, 
                          "Are you sure you want to continue connecting (yes/no)? ",
                          -1)) {
                error("Aborted by user!");
                goto fail;
            }
        } else if (options.strict_host_key_checking != 0) {
            log(errbuf);
            error("Invalid Strict Host Key Check value!");
            goto fail;
        } else {
            log(errbuf);
        }
    }

    free(ip);
    return(0);

  fail:
    free(ip);
    return(-1);
}

int
verify_host_key(char *host, struct sockaddr *hostaddr, Key *host_key)
{
        struct _stat st;

        /* return ok if the key can be found in an old keyfile */
        if (stat(options.system_hostfile2, &st) == 0 ||
            stat(options.user_hostfile2, &st) == 0) {
                if (check_host_key(host, hostaddr, host_key, /*readonly*/ 1,
                    options.user_hostfile2, options.system_hostfile2) == 0)
                        return 0;
        }
        return check_host_key(host, hostaddr, host_key, /*readonly*/ 0,
            options.user_hostfile, options.system_hostfile);
}

/*
 * Starts a dialog with the server, and authenticates the current user on the
 * server.  This does not need any extra privileges.  The basic connection
 * to the server must already have been established before this is called.
 * If login fails, this function prints an error and never returns.
 * This function does not require super-user privileges.
 */
int
ssh_login(Sensitive *sensitive, const char *orighost,
    struct sockaddr *hostaddr, struct passwd *pw)
{
    char *host, *cp;
    char *server_user, *local_user;

    local_user = strdup(pw->pw_name);
    server_user = options.user ? options.user : local_user;

    /* Convert the user-supplied hostname into all lowercase. */
    host = strdup(orighost);
    for (cp = host; *cp; cp++)
        if (isupper(*cp))
            *cp = tolower(*cp);

    /* Exchange protocol version identification strings with the server. */
    if (ssh_exchange_identification() < 0)
        return(-1);

    /* Put the connection into non-blocking mode. */
    packet_set_nonblocking();

    /* key exchange */
    /* authenticate user */
    if (compat20) {
        if (ssh_kex2(host, hostaddr) < 0)
            return -1;
        if (ssh_userauth2(local_user, server_user, host, sensitive) < 0)
            return -1;
    } else {
        if (ssh_kex(host, hostaddr) < 0)
            return -1;
        if (ssh_userauth1(local_user, server_user, host, sensitive) < 0)
            return -1;
    }
    return(0);
}

static int
show_key_from_file(const char *file, const char *host, int keytype)
{
    Key *found;
    char *fp;
    int line, ret;

    found = key_new(keytype);
    if ((ret = lookup_key_in_hostfile_by_type(file, host,
                                               keytype, found, &line))) {
        fp = key_fingerprint(found, SSH_FP_MD5, SSH_FP_HEX);
        log("WARNING: %s key found for host %s\n"
             "in %s:%d\n"
             "%s key fingerprint %s.",
             key_type(found), host, file, line,
             key_type(found), fp);
        free(fp);
    }
    key_free(found);
    return (ret);
}

/* print all known host keys for a given host, but skip keys of given type */
static int
show_other_keys(const char *host, Key *key)
{
    int type[] = { KEY_RSA1, KEY_RSA, KEY_DSA, -1};
    int i, found = 0;

    for (i = 0; type[i] != -1; i++) {
        if (type[i] == key->type)
            continue;
        if (type[i] != KEY_RSA1 &&
             show_key_from_file(options.user_hostfile2, host, type[i])) {
            found = 1;
            continue;
        }
        if (type[i] != KEY_RSA1 &&
             show_key_from_file(options.system_hostfile2, host, type[i])) {
            found = 1;
            continue;
        }
        if (show_key_from_file(options.user_hostfile, host, type[i])) {
            found = 1;
            continue;
        }
        if (show_key_from_file(options.system_hostfile, host, type[i])) {
            found = 1;
            continue;
        }
        debug2("no key of type %d for host %s", type[i], host);
    }
    return (found);
}

void
ssh_put_password(char *password)
{
    int size;
    char *padded;

    if (datafellows & SSH_BUG_PASSWORDPAD) {
        packet_put_cstring(password);
        return;
    }
    size = roundup(strlen(password) + 1, 32);
    padded = malloc(size);
    memset(padded, 0, size);
    strlcpy(padded, password, size);
    packet_put_string(padded, size);
    memset(padded, 0, size);
    free(padded);
}


/* Session id for the current session. */
u_char session_id[16];
u_int supported_authentications = 0;

#ifdef KRB5
krb5_context ssh_context = NULL;
krb5_auth_context auth_context = NULL;
#endif /* KRB5 */

/*
 * Checks if the user has an authentication agent, and if so, tries to
 * authenticate using the agent.
 */
static int
try_agent_authentication(void)
{
    int type;
    char *comment;
    AuthenticationConnection *auth;
    u_char response[16];
    u_int i;
    Key *key;
    BIGNUM *challenge;

    /* Get connection to the agent. */
    auth = ssh_get_authentication_connection();
    if (!auth)
        return 0;

    challenge = BN_new();
    if (challenge == NULL) {
        fatal("try_agent_authentication: BN_new_failed");
        return(0);
    }

    /* Loop through identities served by the agent. */
    for (key = ssh_get_first_identity(auth, &comment, 1);
          key != NULL;
          key = ssh_get_next_identity(auth, &comment, 1)) {

        /* Try this identity. */
        debug1("Trying RSA authentication via agent with: %s", comment);
        free(comment);

        /* Tell the server that we are willing to authenticate using this key. */
        packet_start(SSH_CMSG_AUTH_RSA);
        packet_put_bignum(key->rsa->n);
        packet_send();
        packet_write_wait();

        /* Wait for server's response. */
        type = packet_read();

        /* The server sends failure if it doesn\'t like our key or
           does not support RSA authentication. */
        if (type == SSH_SMSG_FAILURE) {
            debug1("Server refused our key.");
            key_free(key);
            continue;
        }
        /* Otherwise it should have sent a challenge. */
        if (type != SSH_SMSG_AUTH_RSA_CHALLENGE) {
            packet_disconnect("Protocol error during RSA authentication: %d",
                               type);
            return 0;
        }
        packet_get_bignum(challenge);
        packet_check_eom();

        debug1("Received RSA challenge from server.");

        /* Ask the agent to decrypt the challenge. */
        if (!ssh_decrypt_challenge(auth, key, challenge, session_id, 1, response)) {
            /*
             * The agent failed to authenticate this identifier
             * although it advertised it supports this.  Just
             * return a wrong value.
             */
            log("Authentication agent failed to decrypt challenge.");
            memset(response, 0, sizeof(response));
        }
        key_free(key);
        debug1("Sending response to RSA challenge.");

        /* Send the decrypted challenge back to the server. */
        packet_start(SSH_CMSG_AUTH_RSA_RESPONSE);
        for (i = 0; i < 16; i++)
            packet_put_char(response[i]);
        packet_send();
        packet_write_wait();

        /* Wait for response from the server. */
        type = packet_read();

        /* The server returns success if it accepted the authentication. */
        if (type == SSH_SMSG_SUCCESS) {
            ssh_close_authentication_connection(auth);
            BN_clear_free(challenge);
            debug1("RSA authentication accepted by server.");
            return 1;
        }
        /* Otherwise it should return failure. */
        if (type != SSH_SMSG_FAILURE) {
            packet_disconnect("Protocol error waiting RSA auth response: %d",
                               type);
            return 0;
        }
    }
    ssh_close_authentication_connection(auth);
    BN_clear_free(challenge);
    debug1("RSA authentication using agent refused.");
    return 0;
}

/*
 * Computes the proper response to a RSA challenge, and sends the response to
 * the server.
 */
static void
respond_to_rsa_challenge(BIGNUM * challenge, RSA * prv)
{
    u_char buf[32], response[16];
    MD5_CTX md;
    int i, len;

    /* Decrypt the challenge using the private key. */
    /* XXX think about Bleichenbacher, too */
    if (rsa_private_decrypt(challenge, challenge, prv) <= 0) {
        packet_disconnect("respond_to_rsa_challenge: rsa_private_decrypt failed");
        return;
    }
    /* Compute the response. */
    /* The response is MD5 of decrypted challenge plus session id. */
    len = BN_num_bytes(challenge);
    if (len <= 0 || len > sizeof(buf)) {
        packet_disconnect("respond_to_rsa_challenge: bad challenge length %d", len);
        return;
    }
    memset(buf, 0, sizeof(buf));
    BN_bn2bin(challenge, buf + sizeof(buf) - len);
    MD5_Init(&md);
    MD5_Update(&md, buf, 32);
    MD5_Update(&md, session_id, 16);
    MD5_Final(response, &md);

    debug1("Sending response to host key RSA challenge.");

    /* Send the response back to the server. */
    packet_start(SSH_CMSG_AUTH_RSA_RESPONSE);
    for (i = 0; i < 16; i++)
        packet_put_char(response[i]);
    packet_send();
    packet_write_wait();

    memset(buf, 0, sizeof(buf));
    memset(response, 0, sizeof(response));
    memset(&md, 0, sizeof(md));
}

/*
 * Checks if the user has authentication file, and if so, tries to authenticate
 * the user using it.
 */
int
try_rsa_authentication(int idx)
{
    BIGNUM *challenge;
    Key *public, *private;
    char buf[300], passphrase[300]="", *comment, *authfile;
    int type, i, quit;

    public = options.identity_keys[idx];
    authfile = options.identity_files[idx];
    comment = strdup(authfile);

    debug1("Trying RSA authentication with key: %s", comment);

    /* Tell the server that we are willing to authenticate using this key. */
    packet_start(SSH_CMSG_AUTH_RSA);
    packet_put_bignum(public->rsa->n);
    packet_send();
    packet_write_wait();

    /* Wait for server's response. */
    type = packet_read();

    /*
     * The server responds with failure if it doesn\'t like our key or
     * doesn\'t support RSA authentication.
     */
    if (type == SSH_SMSG_FAILURE) {
        debug1("Server refused our key.");
        free(comment);
        return 0;
    }
    /* Otherwise, the server should respond with a challenge. */
    if (type != SSH_SMSG_AUTH_RSA_CHALLENGE) {
        packet_disconnect("Protocol error during RSA authentication: %d", type);
        return 0;
    }
    /* Get the challenge from the packet. */
    challenge = BN_new();
    if (challenge == NULL) {
        fatal("try_rsa_authentication: BN_new_failed");
        return(0);
    }
    packet_get_bignum(challenge);
    packet_check_eom();

    debug1("Received RSA challenge from server.");

    /*
     * If the key is not stored in external hardware, we have to
     * load the private key.  Try first with empty passphrase; if it
     * fails, ask for a passphrase.
     */
    if (public->flags & KEY_FLAG_EXT)
        private = public;
    else
        private = key_load_private_type(KEY_RSA1, authfile, "", NULL);

    if (private == NULL && !options.batch_mode) {
        int ok;
        snprintf(buf, sizeof(buf),
                  "Enter passphrase for RSA key '%.100s': ", comment);
        for (i = 0; i < options.number_of_password_prompts; i++) {
            if ( i == 0 && pwbuf[0] && pwflg ) {
                ckstrncpy(passphrase,pwbuf,sizeof(passphrase));
#ifdef OS2
                if ( pwcrypt )
                    ck_encrypt((char *)passphrase);
#endif /* OS2 */
            } else {
                ok = uq_txt(buf,"Passphrase: ",2,NULL,passphrase,
                            sizeof(passphrase),NULL,DEFAULT_UQ_TIMEOUT);
                if ( !ok ) {
                    quit = 1;
                    break;
                }
            }
            if (strcmp(passphrase, "") != 0) {
                private = key_load_private_type(KEY_RSA1,
                                                 authfile, passphrase, NULL);
                quit = 0;
            } else {
                debug2("no passphrase given, try next key");
                quit = 1;
            }
            memset(passphrase, 0, strlen(passphrase));
            if (private != NULL || quit)
                break;
            debug2("bad passphrase given, try again...");
        }
    }
    /* We no longer need the comment. */
    free(comment);

    if (private == NULL) {
        if (!options.batch_mode)
            error("Bad passphrase.");
#ifdef KUI
        uq_ok(NULL, "Bad passphrase.", 1, NULL, 0);
#endif /* KUI */

            /* Send a dummy response packet to avoid protocol error. */
            packet_start(SSH_CMSG_AUTH_RSA_RESPONSE);
            for (i = 0; i < 16; i++)
                packet_put_char(0);
            packet_send();
            packet_write_wait();

            /* Expect the server to reject it... */
            packet_read_expect(SSH_SMSG_FAILURE);
            BN_clear_free(challenge);
            return 0;
    }

    /* Compute and send a response to the challenge. */
    respond_to_rsa_challenge(challenge, private->rsa);

    /* Destroy the private key unless it in external hardware. */
    if (!(private->flags & KEY_FLAG_EXT))
        key_free(private);

    /* We no longer need the challenge. */
    BN_clear_free(challenge);

    /* Wait for response from the server. */
    type = packet_read();
    if (type == SSH_SMSG_SUCCESS) {
        debug1("RSA authentication accepted by server.");
        return 1;
    }
    if (type != SSH_SMSG_FAILURE) {
        packet_disconnect("Protocol error waiting RSA auth response: %d", type);
        return 0;
    }
    debug1("RSA authentication refused.");
    return 0;
}

/*
 * Tries to authenticate the user using combined rhosts or /etc/hosts.equiv
 * authentication and RSA host authentication.
 */
static int
try_rhosts_rsa_authentication(const char *local_user, Key * host_key)
{
    int type;
    BIGNUM *challenge;

    debug1("Trying rhosts or /etc/hosts.equiv with RSA host authentication.");

    /* Tell the server that we are willing to authenticate using this key. */
    packet_start(SSH_CMSG_AUTH_RHOSTS_RSA);
    packet_put_cstring(local_user);
    packet_put_int(BN_num_bits(host_key->rsa->n));
    packet_put_bignum(host_key->rsa->e);
    packet_put_bignum(host_key->rsa->n);
    packet_send();
    packet_write_wait();

    /* Wait for server's response. */
    type = packet_read();

    /* The server responds with failure if it doesn't admit our
       .rhosts authentication or doesn't know our host key. */
    if (type == SSH_SMSG_FAILURE) {
        debug1("Server refused our rhosts authentication or host key.");
        return 0;
    }
    /* Otherwise, the server should respond with a challenge. */
    if (type != SSH_SMSG_AUTH_RSA_CHALLENGE) {
        packet_disconnect("Protocol error during RSA authentication: %d", type);
        return 0;
    }
    /* Get the challenge from the packet. */
    challenge = BN_new();
    if (challenge == NULL) {
        fatal("try_rhosts_rsa_authentication: BN_new_failed");
        return(0);
    }
    packet_get_bignum(challenge);
    packet_check_eom();

    debug1("Received RSA challenge for host key from server.");

    /* Compute a response to the challenge. */
    respond_to_rsa_challenge(challenge, host_key->rsa);

    /* We no longer need the challenge. */
    BN_clear_free(challenge);

    /* Wait for response from the server. */
    type = packet_read();
    if (type == SSH_SMSG_SUCCESS) {
        debug1("Rhosts or /etc/hosts.equiv with RSA host authentication accepted by server.");
        return 1;
    }
    if (type != SSH_SMSG_FAILURE) {
        packet_disconnect("Protocol error waiting RSA auth response: %d", type);
        return 0;
    }
    debug1("Rhosts or /etc/hosts.equiv with RSA host authentication refused.");
    return 0;
}

#ifdef KRB5
static void
krb5_cleanup(void *ignore)
{
    if (auth_context) {
        krb5_auth_con_free(ssh_context, auth_context);
        auth_context = NULL;
    }
    if (ssh_context) {
        krb5_free_context(ssh_context);
        ssh_context = NULL;
    }
}

static int
try_krb5_authentication(krb5_context *context, krb5_auth_context *auth_context)
{
    krb5_error_code problem;
    const char *tkfile;
    struct _stat buf;
    krb5_ccache ccache = NULL;
    const char *remotehost;
    krb5_data ap;
    int type;
    krb5_ap_rep_enc_part *reply = NULL;
    int ret;

    memset(&ap, 0, sizeof(ap));

    problem = krb5_init_context(context);
    if (problem) {
        debug1("try_krb5_authentication: krb5_init_context() returns: %s",
               krb5_get_err_text(*context,problem));
        ret = 0;
        goto out;
    }

    problem = krb5_auth_con_init(*context, auth_context);
    if ( problem ) {
        debug1("Kerberos V5: krb5_auth_con_init failed");
        ret = 0;
        goto out;
    }
#ifndef HEIMDAL
        problem = krb5_auth_con_setflags(*context, *auth_context,
                                         KRB5_AUTH_CONTEXT_RET_TIME);
        if (problem) {
                debug1("Kerberos v5: krb5_auth_con_setflags failed");
                ret = 0;
                goto out;
        }
#endif

    tkfile = krb5_cc_default_name(*context);
    if (strncmp(tkfile, "FILE:", 5) == 0) {
        tkfile += 5;

        /* XXX ssh should not be suid root here but ...*/
        if (stat(tkfile, &buf) == 0
#ifndef OS2
             && getuid() != buf.st_uid
#endif /* OS2 */
             ) {
            debug1("Kerberos V5: could not get default ccache (permission denied): %s",
                   tkfile);
            ret = 0;
            goto out;
        }
    }

    problem = krb5_cc_default(*context, &ccache);
    if (problem) {
        debug1("Kerberos v5: krb5_cc_default failed: %s",
               krb5_get_err_text(*context, problem));
        ret = 0;
        goto out;
    }

    remotehost = get_canonical_hostname(1);

    problem = krb5_mk_req(*context, auth_context, AP_OPTS_MUTUAL_REQUIRED,
                           "host", (char *)remotehost, NULL, ccache, &ap);
    if (problem) {
        debug1("Kerberos v5: krb5_mk_req failed: %s",
               krb5_get_err_text(*context, problem));
        ret = 0;
        goto out;
    }

    packet_start(ssh_k5_is_k4 ? SSH_CMSG_AUTH_KRB4 : SSH_CMSG_AUTH_KRB5);
    packet_put_string((char *) ap.data, ap.length);
    packet_send();
    packet_write_wait();

    free(ap.data);
    ap.length = 0;

    type = packet_read();
    switch (type) {
    case SSH_SMSG_FAILURE:
        /* Should really be SSH_SMSG_AUTH_KERBEROS_FAILURE */
        debug1("Kerberos v5 authentication failed.");
        ret = 0;
        break;

    case SSH_SMSG_AUTH_KRB4_RESPONSE:
        /* for compatibility versions of OpenSSH which reuse the K4 message codes */
        /* fallthrough */
    case SSH_SMSG_AUTH_KRB5_RESPONSE:
        debug1("Kerberos v5 authentication accepted.");

        /* Get server's response. */
        ap.data = packet_get_string((unsigned int *) &ap.length);

        packet_check_eom();

        problem = krb5_rd_rep(*context, *auth_context, &ap, &reply);
        if (problem) {
            ret = 0;
        }
        ret = 1;
        break;

    default:
        packet_disconnect("Protocol error on Kerberos v5 response: %d",
                           type);
        ret = 0;
        break;

    }

  out:
    if (ccache != NULL)
        krb5_cc_close(*context, ccache);
    if (reply != NULL)
        krb5_free_ap_rep_enc_part(*context, reply);
    if (ap.length > 0) {
#ifdef HEIMDAL
        krb5_data_free(&ap);
#else
        krb5_free_data_contents(*context, &ap);
#endif
    }
    return (ret);
}

static void
send_krb5_tgt(krb5_context context, krb5_auth_context auth_context)
{
    int fd, type;
    krb5_error_code problem;
    krb5_data outbuf;
    krb5_ccache ccache = NULL;
    krb5_creds creds;
#ifdef HEIMDAL
    krb5_kdc_flags flags;
#else
    int forwardable;
#endif
    const char *remotehost;

    memset(&creds, 0, sizeof(creds));
    memset(&outbuf, 0, sizeof(outbuf));

    fd = packet_get_connection_in();

#ifdef HEIMDAL
    problem = krb5_auth_con_setaddrs_from_fd(context, auth_context, &fd);
    if (problem)
        goto out;
#else
    problem = krb5_auth_con_genaddrs(context, auth_context, fd,
                        KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR |
                        KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR);
    if (problem) {
        debug1("krb5_auth_con_genaddrs: %.100s", error_message(problem));
        goto out;
    }
#endif

    problem = krb5_cc_default(context, &ccache);
    if (problem)
        goto out;

    problem = krb5_cc_get_principal(context, ccache, &creds.client);
    if (problem)
        goto out;

    remotehost = get_canonical_hostname(1);

#ifdef HEIMDAL
    problem = krb5_build_principal(context, &creds.server,
            strlen(creds.client->realm), creds.client->realm,
            "krbtgt", creds.client->realm, NULL);
#else
        problem = krb5_build_principal(context, &creds.server,
            creds.client->realm.length, creds.client->realm.data,
            "krbtgt", creds.client->realm.data, NULL);
#endif

    if (problem)
        goto out;

    creds.times.endtime = 0;

#ifdef HEIMDAL
    flags.i = 0;
    flags.b.forwarded = 1;
    flags.b.forwardable = krb5_config_get_bool(context,  NULL,
                                "libdefaults", "forwardable", NULL);

    problem = krb5_get_forwarded_creds(context, auth_context,
                      ccache, flags.i, remotehost, &creds, &outbuf);
    if (problem)
        goto out;
#else
    forwardable = 1;
    problem = krb5_fwd_tgt_creds (context, auth_context,
                                   (char *)remotehost, creds.client, creds.server, ccache, forwardable,
                                   &outbuf);
    if (problem)
        goto out;
#endif

    packet_start(ssh_k5_is_k4 ? SSH_CMSG_HAVE_KRB4_TGT : SSH_CMSG_HAVE_KRB5_TGT);
    packet_put_string((char *)outbuf.data, outbuf.length);
    packet_send();
    packet_write_wait();

    type = packet_read();

    if (type == SSH_SMSG_SUCCESS) {
        char *pname;

        krb5_unparse_name(context, creds.client, &pname);
        debug1("Kerberos v5 TGT forwarded (%s).", pname);
        free(pname);
    } else
        debug1("Kerberos v5 TGT forwarding failed.");

    return;

 out:
    if (problem)
        debug1("Kerberos v5 TGT forwarding failed: %s",
               krb5_get_err_text(context, problem));
    if (creds.client)
        krb5_free_principal(context, creds.client);
    if (creds.server)
        krb5_free_principal(context, creds.server);
    if (ccache)
        krb5_cc_close(context, ccache);
    if (outbuf.data)
        free(outbuf.data);
}
#endif /* KRB5 */

#ifdef KRB4
static int
try_krb4_authentication(void)
{
    KTEXT_ST auth;              /* Kerberos data */
    char *reply;
    char inst[INST_SZ];
    char *realm;
#ifdef OS2
    LEASH_CREDENTIALS cred;
#else /* OS2 */
    CREDENTIALS cred;
#endif /* OS2 */
    int r, type;
    socklen_t slen;
    Schedule schedule;
    u_long checksum, cksum;
    MSG_DAT msg_data;
    struct sockaddr_in local, foreign;

    /* Don't do anything if we don't have any tickets. */
    if (ck_krb4_get_tkts() < 1)
        return 0;

    ckstrncpy(inst, (char *) krb_get_phost((char *)get_canonical_hostname(1)), INST_SZ);

    realm = (char *) krb_realmofhost((char *)get_canonical_hostname(1));
    if (!realm) {
        debug1("Kerberos V4: no realm for: %s", (char *)get_canonical_hostname(1));
        return 0;
    }
    /* This can really be anything. */
    checksum = (u_long) getpid();

    r = krb_mk_req(&auth, KRB4_SERVICE_NAME, inst, realm, checksum);
    if (r != KSUCCESS) {
        debug1("Kerberos V4 krb_mk_req failed: %s (%d)", krb_get_err_text_entry(r), r);
        return 0;
    }
    /* Get session key to decrypt the server's reply with. */
    r = krb_get_cred(KRB4_SERVICE_NAME, inst, realm, &cred);
    if (r != KSUCCESS) {
        debug1("Kerberos V4: get_cred failed: %s (%d)", krb_get_err_text_entry(r), r);
        return 0;
    }
    des_key_sched(cred.session, schedule);

    /* Send authentication info to server. */
    packet_start(SSH_CMSG_AUTH_KRB4);
    packet_put_string((char *) auth.dat, auth.length);
    packet_send();
    packet_write_wait();

    /* Zero the buffer. */
    (void) memset(auth.dat, 0, MAX_KTXT_LEN);

    slen = sizeof(local);
    memset(&local, 0, sizeof(local));
    if (getsockname(packet_get_connection_in(),
                     (struct sockaddr *) & local, &slen) < 0)
        debug1("getsockname failed: %s",
#ifdef NT
                sock_strerror(sock_lasterror())
#else
                strerror(errno)
#endif
                );

    slen = sizeof(foreign);
    memset(&foreign, 0, sizeof(foreign));
    if (getpeername(packet_get_connection_in(),
                     (struct sockaddr *) & foreign, &slen) < 0) {
        debug1("getpeername failed: %s",
#ifdef NT
                sock_strerror(sock_lasterror())
#else
                strerror(errno)
#endif
                );
        fatal_cleanup();
        return(-1);
    }
    /* Get server reply. */
    type = packet_read();
    switch (type) {
    case SSH_SMSG_FAILURE:
        /* Should really be SSH_SMSG_AUTH_KRB4_FAILURE */
        debug1("Kerberos V4 authentication failed.");
        return 0;
        break;

    case SSH_SMSG_AUTH_KRB4_RESPONSE:
        /* SSH_SMSG_AUTH_KRB4_SUCCESS */
        debug1("Kerberos V4 authentication accepted.");

        /* Get server's response. */
        reply = packet_get_string((u_int *) &auth.length);
        if ( auth.length >= MAX_KTXT_LEN ) {
            fatal("Keberos V4: Malformed response from server.");
            return -1;
        }
        memcpy(auth.dat, reply, auth.length);
        free(reply);

        packet_check_eom();

        /*
         * If his response isn't properly encrypted with the session
         * key, and the decrypted checksum fails to match, he's
         * bogus. Bail out.
         */
        r = krb_rd_priv(auth.dat, auth.length, schedule,
#ifdef KRB524
                         cred.session,
#else /* KRB524 */
                         &cred.session,
#endif /* KRB524 */
                         &foreign, &local, &msg_data);
        if (r != KSUCCESS) {
            debug1("Kerberos V4 krb_rd_priv failed: %s (%d)", krb_get_err_text_entry(r),r);
            packet_disconnect("Kerberos V4 challenge failed!");
            return 0;
        }
        /* Fetch the (incremented) checksum that we supplied in the request. */
        (void) memcpy((char *) &cksum, (char *) msg_data.app_data, sizeof(cksum));
        cksum = ntohl(cksum);

        /* If it matches, we're golden. */
        if (cksum == checksum + 1) {
            debug1("Kerberos V4 challenge successful.");
            return 1;
        } else {
            packet_disconnect("Kerberos V4 challenge failed!");
            return 0;
        }
        break;

    default:
        packet_disconnect("Protocol error on Kerberos V4 response: %d", type);
        return 0;
    }
    return 0;
}
#endif /* KRB4 */

#ifdef AFS
static void
send_krb4_tgt(void)
{
#ifdef OS2
    LEASH_CREDENTIALS *creds;
#else
    CREDENTIALS *creds;
#endif
    char pname[ANAME_SZ], pinst[INST_SZ], prealm[REALM_SZ];
    int problem, type;
    char buffer[8192];
    struct _stat st;

    /* Don't do anything if we don't have any tickets. */
    if (stat(tkt_string(), &st) < 0)
        return;

    creds = malloc(sizeof(*creds));

    problem = krb_get_tf_fullname(TKT_FILE, pname, pinst, prealm);
    if (problem)
        goto out;

    problem = krb_get_cred("krbtgt", prealm, prealm, creds);
    if (problem)
        goto out;

    if (time(0) > krb_life_to_time(creds->issue_date, creds->lifetime)) {
        problem = RD_AP_EXP;
        goto out;
    }
    creds_to_radix(creds, (u_char *)buffer, sizeof(buffer));

    packet_start(SSH_CMSG_HAVE_KRB4_TGT);
    packet_put_cstring(buffer);
    packet_send();
    packet_write_wait();

    type = packet_read();

    if (type == SSH_SMSG_SUCCESS)
        debug1("Kerberos v4 TGT forwarded (%s%s%s@%s).",
               creds->pname, creds->pinst[0] ? "." : "",
               creds->pinst, creds->realm);
    else
        debug1("Kerberos v4 TGT rejected.");

    free(creds);
    return;

  out:
    debug1("Kerberos v4 TGT passing failed: %s", get_krb_err_txt_entry(problem));
    free(creds);
}

static void
send_afs_tokens(void)
{
#ifdef OS2
    LEASH_CREDENTIALS creds;
#else
    CREDENTIALS creds;
#endif /* OS2 */
    struct ViceIoctl parms;
    struct ClearToken ct;
    int i, type, len;
    char buf[2048], *p, *server_cell;
    char buffer[8192];

    /* Move over ktc_GetToken, here's something leaner. */
    for (i = 0; i < 100; i++) { /* just in case */
        parms.in = (char *) &i;
        parms.in_size = sizeof(i);
        parms.out = buf;
        parms.out_size = sizeof(buf);
        if (k_pioctl(0, VIOCGETTOK, &parms, 0) != 0)
            break;
        p = buf;

        /* Get secret token. */
        memcpy(&creds.ticket_st.length, p, sizeof(u_int));
        if (creds.ticket_st.length > MAX_KTXT_LEN)
            break;
        p += sizeof(u_int);
        memcpy(creds.ticket_st.dat, p, creds.ticket_st.length);
        p += creds.ticket_st.length;

        /* Get clear token. */
        memcpy(&len, p, sizeof(len));
        if (len != sizeof(struct ClearToken))
            break;
        p += sizeof(len);
        memcpy(&ct, p, len);
        p += len;
        p += sizeof(len);       /* primary flag */
        server_cell = p;

        /* Flesh out our credentials. */
        strlcpy(creds.service, "afs", sizeof creds.service);
        creds.instance[0] = '\0';
        strlcpy(creds.realm, server_cell, REALM_SZ);
        memcpy(creds.session, ct.HandShakeKey, DES_KEY_SZ);
        creds.issue_date = ct.BeginTimestamp;
        creds.lifetime = krb_time_to_life(creds.issue_date, ct.EndTimestamp);
        creds.kvno = ct.AuthHandle;
        snprintf(creds.pname, sizeof(creds.pname), "AFS ID %d", ct.ViceId);
        creds.pinst[0] = '\0';

        /* Encode token, ship it off. */
        if (creds_to_radix(&creds, (u_char *) buffer, sizeof buffer) <= 0)
            break;
        packet_start(SSH_CMSG_HAVE_AFS_TOKEN);
        packet_put_cstring(buffer);
        packet_send();
        packet_write_wait();

        /* Roger, Roger. Clearance, Clarence. What's your vector,
        Victor? */
        type = packet_read();

        if (type == SSH_SMSG_FAILURE)
            debug1("AFS token for cell rejected: %s", server_cell);
                else if (type != SSH_SMSG_SUCCESS) {
                    packet_disconnect("Protocol error on AFS token response: %d", type);
                    return 0;
                }
    }
}

#endif /* AFS */

/*
 * Tries to authenticate with any string-based challenge/response system.
 * Note that the client code is not tied to s/key or TIS.
 */
static int
try_challenge_response_authentication(void)
{
    int type, i;
    u_int clen;
    char prompt[1024], response[256]="";
    char *challenge;

    debug1("Doing challenge response authentication.");

    for (i = 0; i < options.number_of_password_prompts; i++) {
        int ok;

        /* request a challenge */
        packet_start(SSH_CMSG_AUTH_TIS);
        packet_send();
        packet_write_wait();

        type = packet_read();
        if (type != SSH_SMSG_FAILURE &&
            type != SSH_SMSG_AUTH_TIS_CHALLENGE) {
            packet_disconnect("Protocol error: got %d in response "
                               "to SSH_CMSG_AUTH_TIS", type);
            return 0;
        }
        if (type != SSH_SMSG_AUTH_TIS_CHALLENGE) {
            debug1("No challenge.");
            return 0;
        }
        challenge = packet_get_string(&clen);
        packet_check_eom();
        snprintf(prompt, sizeof prompt, "%s%s%s",
                  i == 0 ? "" : "Permission denied, please try again.\n",
                  challenge,
                 strchr(challenge, '\n') ? "" : "\n");
        free(challenge);
        if (i != 0)
            bleep(BP_FAIL);
        if ( i == 0 && pwbuf[0] && pwflg ) {
            if (options.cipher == SSH_CIPHER_NONE)
                log("WARNING: Encryption is disabled! "
                    "Response will be transmitted in clear text.");
            ckstrncpy(response,pwbuf,sizeof(response));
#ifdef OS2
            if ( pwcrypt )
                ck_encrypt((char *)response);
#endif /* OS2 */
            ok = 1;
        } else { 
            if (options.cipher == SSH_CIPHER_NONE)
                ckstrncat(prompt,"WARNING: Encryption is disabled! "
                          "Response will be transmitted in clear text.\n",1024);
            ok = uq_txt(prompt,"Response: ",2,NULL,response, sizeof(response),
                        NULL,DEFAULT_UQ_TIMEOUT);
        }
        if (!ok || strcmp(response, "") == 0)
            break;
        packet_start(SSH_CMSG_AUTH_TIS_RESPONSE);
        ssh_put_password(response);
        memset(response, 0, strlen(response));
        packet_send();
        packet_write_wait();
        type = packet_read();
        if (type == SSH_SMSG_SUCCESS)
            return 1;
        if (type != SSH_SMSG_FAILURE) {
            packet_disconnect("Protocol error: got %d in response "
                               "to SSH_CMSG_AUTH_TIS_RESPONSE", type);
            return 0;
        }
    }
    /* failure */
    return 0;
}

/*
 * Tries to authenticate with plain passwd authentication.
 */
static int
try_password_authentication(const char *server_user, char *host)
{
    int type, i, ok;
    char password[256]="";
    char prompt[1024];

    debug1("Doing password authentication.");
    for (i = 0; i < options.number_of_password_prompts; i++) {
        if (i != 0)
            bleep(BP_FAIL);
        snprintf(prompt, sizeof(prompt), "%s%.30s@%.128s's password: ",
                  i == 0 ? "" : "Permission denied, please try again.\n",
                  server_user, host);
        if ( i == 0 && pwbuf[0] && pwflg ) {
            if (options.cipher == SSH_CIPHER_NONE)
                log("WARNING: Encryption is disabled! Password will be transmitted in clear text.");
            ckstrncpy(password,pwbuf,sizeof(password));
#ifdef OS2
            if ( pwcrypt )
                ck_encrypt((char *)password);
#endif /* OS2 */
            ok = 1;
        } else {
            if (options.cipher == SSH_CIPHER_NONE)
                ckstrncat(prompt,
                "WARNING: Encryption is disabled! "
                "Password will be transmitted in clear text.\n",1024);
            ok = uq_txt(prompt,"Response: ",2,NULL,password, sizeof(password),NULL,
                        DEFAULT_UQ_TIMEOUT);
        }
        packet_start(SSH_CMSG_AUTH_PASSWORD);
        if (ok)
            ssh_put_password(password);
        memset(password, 0, strlen(password));
        packet_send();
        packet_write_wait();

        type = packet_read();
        if (type == SSH_SMSG_SUCCESS)
            return 1;
        if (type != SSH_SMSG_FAILURE) {
            packet_disconnect("Protocol error: got %d in response to passwd auth", type);
            return 0;
        }
    }
    /* failure */
    return 0;
}

/*
 * SSH1 key exchange
 */
int
ssh_kex(char *host, struct sockaddr *hostaddr)
{
    int i;
    BIGNUM *key;
    Key *host_key, *server_key;
    int bits, rbits;
    int ssh_cipher_default = SSH_CIPHER_3DES;
    u_char session_key[SSH_SESSION_KEY_LENGTH];
    u_char cookie[8];
    u_int supported_ciphers;
    u_int server_flags, client_flags;
    u_int32_t rand = 0;

    debug1("Waiting for server public key.");

    /* Wait for a public key packet from the server. */
    packet_read_expect(SSH_SMSG_PUBLIC_KEY);

    /* Get cookie from the packet. */
    for (i = 0; i < 8; i++)
        cookie[i] = packet_get_char();

    /* Get the public key. */
    server_key = key_new(KEY_RSA1);
    bits = packet_get_int();
    packet_get_bignum(server_key->rsa->e);
    packet_get_bignum(server_key->rsa->n);

    rbits = BN_num_bits(server_key->rsa->n);
    if (bits != rbits) {
        log("Warning: Server lies about size of server public key: "
             "actual size is %d bits vs. announced %d.", rbits, bits);
        log("Warning: This may be due to an old implementation of ssh.");
    }
    /* Get the host key. */
    host_key = key_new(KEY_RSA1);
    bits = packet_get_int();
    packet_get_bignum(host_key->rsa->e);
    packet_get_bignum(host_key->rsa->n);

    rbits = BN_num_bits(host_key->rsa->n);
    if (bits != rbits) {
        log("Warning: Server lies about size of server host key: "
             "actual size is %d bits vs. announced %d.", rbits, bits);
        log("Warning: This may be due to an old implementation of ssh.");
    }

    /* Get protocol flags. */
    server_flags = packet_get_int();
    packet_set_protocol_flags(server_flags);

    supported_ciphers = packet_get_int();
    supported_authentications = packet_get_int();
    packet_check_eom();

    debug1("Received server public key (%d bits) ", BN_num_bits(server_key->rsa->n));
    debug1("and host key (%d bits).", BN_num_bits(host_key->rsa->n));

    if (verify_host_key(host, hostaddr, host_key) == -1) {
        fatal("host key verification failed.");
        return(-1);
    }

    client_flags = SSH_PROTOFLAG_SCREEN_NUMBER | SSH_PROTOFLAG_HOST_IN_FWD_OPEN;

    compute_session_id(session_id, cookie, host_key->rsa->n, server_key->rsa->n);

    /* Generate a session key. */
    arc4random_stir();

    /*
     * Generate an encryption key for the session.   The key is a 256 bit
     * random number, interpreted as a 32-byte key, with the least
     * significant 8 bits being the first byte of the key.
     */
    for (i = 0; i < 32; i++) {
        if (i % 4 == 0)
            rand = arc4random();
        session_key[i] = rand & 0xff;
        rand >>= 8;
    }

    /*
     * According to the protocol spec, the first byte of the session key
     * is the highest byte of the integer.  The session key is xored with
     * the first 16 bytes of the session id.
     */
    key = BN_new();
    if (key == NULL) {
        fatal("ssh_kex: BN_new_failed");
        return(0);
    }

    BN_set_word(key, 0);
    for (i = 0; i < SSH_SESSION_KEY_LENGTH; i++) {
        BN_lshift(key, key, 8);
        if (i < 16)
            BN_add_word(key, session_key[i] ^ session_id[i]);
        else
            BN_add_word(key, session_key[i]);
    }

    /*
     * Encrypt the integer using the public key and host key of the
     * server (key with smaller modulus first).
     */
    if (BN_cmp(server_key->rsa->n, host_key->rsa->n) < 0) {
        /* Public key has smaller modulus. */
        if (BN_num_bits(host_key->rsa->n) <
             BN_num_bits(server_key->rsa->n) + SSH_KEY_BITS_RESERVED) {
            fatal("respond_to_rsa_challenge: host_key %d < server_key %d + "
                   "SSH_KEY_BITS_RESERVED %d",
                   BN_num_bits(host_key->rsa->n),
                   BN_num_bits(server_key->rsa->n),
                   SSH_KEY_BITS_RESERVED);
            return -1;
        }
        rsa_public_encrypt(key, key, server_key->rsa);
        rsa_public_encrypt(key, key, host_key->rsa);
    } else {
        /* Host key has smaller modulus (or they are equal). */
        if (BN_num_bits(server_key->rsa->n) <
             BN_num_bits(host_key->rsa->n) + SSH_KEY_BITS_RESERVED) {
            fatal("respond_to_rsa_challenge: server_key %d < host_key %d + "
                   "SSH_KEY_BITS_RESERVED %d",
                   BN_num_bits(server_key->rsa->n),
                   BN_num_bits(host_key->rsa->n),
                   SSH_KEY_BITS_RESERVED);
            return -1;
        }
        rsa_public_encrypt(key, key, host_key->rsa);
        rsa_public_encrypt(key, key, server_key->rsa);
    }

    /* Destroy the public keys since we no longer need them. */
    RSA_free(server_key->rsa);
    RSA_free(host_key->rsa);

    if (options.cipher == SSH_CIPHER_NOT_SET) {
        if (cipher_mask_ssh1(1) & supported_ciphers & (1 << ssh_cipher_default))
            options.cipher = ssh_cipher_default;
    } else if (options.cipher == SSH_CIPHER_ILLEGAL ||
                !(cipher_mask_ssh1(1) & (1 << options.cipher))) {
        log("No valid SSH1 cipher, using %.100s instead.",
             cipher_name(ssh_cipher_default));
        options.cipher = ssh_cipher_default;
    }
    /* Check that the selected cipher is supported. */
    if (!(supported_ciphers & (1 << options.cipher))) {
        fatal("Selected cipher type %.100s not supported by server.",
               cipher_name(options.cipher));
        return -1;
    }

    debug1("Encryption type", cipher_name(options.cipher));

    /* Send the encrypted session key to the server. */
    packet_start(SSH_CMSG_SESSION_KEY);
    packet_put_char(options.cipher);

    /* Send the cookie back to the server. */
    for (i = 0; i < 8; i++)
        packet_put_char(cookie[i]);

    /* Send and destroy the encrypted encryption key integer. */
    packet_put_bignum(key);
    BN_clear_free(key);

    /* Send protocol flags. */
    packet_put_int(client_flags);

    /* Send the packet now. */
    packet_send();
    packet_write_wait();

    debug1("Sent encrypted session key.");

    /* Set the encryption key. */
    packet_set_encryption_key(session_key, SSH_SESSION_KEY_LENGTH, options.cipher);

    /* We will no longer need the session key here.  Destroy any extra copies. */
    memset(session_key, 0, sizeof(session_key));

    /*
     * Expect a success message from the server.  Note that this message
     * will be received in encrypted form.
     */
    packet_read_expect(SSH_SMSG_SUCCESS);

    debug1("Received encrypted confirmation.");
    return 0;
}

/*
 * Authenticate user
 */
int
ssh_userauth1(const char *local_user, const char *server_user, char *host,
    Sensitive *sensitive)
{
    int i, type;
#ifdef KRB5
    krb5_context context = NULL;
    krb5_auth_context auth_context = NULL;
#endif

    if (supported_authentications == 0) {
        fatal("ssh_userauth1: server supports no auth methods");
        return -1;
    }

    /* Send the name of the user to log in as on the server. */
    packet_start(SSH_CMSG_USER);
    packet_put_cstring(server_user);
    packet_send();
    packet_write_wait();

    /*
     * The server should respond with success if no authentication is
     * needed (the user has no password).  Otherwise the server responds
     * with failure.
     */
    type = packet_read();

    /* check whether the connection was accepted without authentication. */
    if (type == SSH_SMSG_SUCCESS)
        goto success;
    if (type != SSH_SMSG_FAILURE) {
        packet_disconnect("Protocol error: got %d in response to SSH_CMSG_USER",
                           type);
        return -1;
    }
#ifdef KRB5
    if ((supported_authentications &
          (1 << (ssh_k5_is_k4 ? SSH_AUTH_KRB4 : SSH_AUTH_KRB5))) &&
        options.krb5_authentication) {
        debug1("ssh_userauth1: Trying Kerberos V5 authentication.");
        if (try_krb5_authentication(&context, &auth_context)) {
            /* The server should respond with success or failure. */
                type = packet_read();
            if (type == SSH_SMSG_SUCCESS)
                goto success;
            if (type != SSH_SMSG_FAILURE) {
                if (auth_context)
                    krb5_auth_con_free(context, auth_context);
                if (context)
                    krb5_free_context(context);
                packet_disconnect("Protocol error: got %d in response to Kerberos V5 auth", type);
                return -1;
            }
        }
    }
#endif /* KRB5 */

#ifdef KRB4
    if ((supported_authentications & (1 << SSH_AUTH_KRB4)) &&
         options.krb4_authentication) {
        debug1("Trying Kerberos V4 authentication.");
        if (try_krb4_authentication()) {
            /* The server should respond with success or failure. */
            type = packet_read();
            if (type == SSH_SMSG_SUCCESS)
                goto success;
            if (type != SSH_SMSG_FAILURE) {
                packet_disconnect("Protocol error: got %d in response to Kerberos V4 auth", type);
                return -1;
            }
        }
    }
#endif /* KRB4 */

    /*
     * Use rhosts authentication if running in privileged socket and we
     * do not wish to remain anonymous.
     */
    if ((supported_authentications & (1 << SSH_AUTH_RHOSTS)) &&
         options.rhosts_authentication) {
        debug1("Trying rhosts authentication.");
        packet_start(SSH_CMSG_AUTH_RHOSTS);
        packet_put_cstring(local_user);
        packet_send();
        packet_write_wait();

        /* The server should respond with success or failure. */
        type = packet_read();
        if (type == SSH_SMSG_SUCCESS)
            goto success;
        if (type != SSH_SMSG_FAILURE) {
            packet_disconnect("Protocol error: got %d in response to rhosts auth",
                               type);
            return -1;
        }
    }
    /*
     * Try .rhosts or /etc/hosts.equiv authentication with RSA host
     * authentication.
     */
    if ((supported_authentications & (1 << SSH_AUTH_RHOSTS_RSA)) &&
         options.rhosts_rsa_authentication) {
        for (i = 0; i < sensitive->nkeys; i++) {
            if (sensitive->keys[i] != NULL && 
                sensitive->keys[i]->type == KEY_RSA1 &&
                 try_rhosts_rsa_authentication(local_user, sensitive->keys[i]))
                goto success;
        }
    }
    /* Try RSA authentication if the server supports it. */
    if ((supported_authentications & (1 << SSH_AUTH_RSA)) &&
         options.rsa_authentication) {
        /*
         * Try RSA authentication using the authentication agent. The
         * agent is tried first because no passphrase is needed for
         * it, whereas identity files may require passphrases.
         */
        if (try_agent_authentication())
            goto success;

        /* Try RSA authentication for each identity. */
        for (i = 0; i < options.num_identity_files; i++)
            if (options.identity_keys[i] != NULL &&
                 options.identity_keys[i]->type == KEY_RSA1 &&
                 try_rsa_authentication(i))
                goto success;
    }
    /* Try challenge response authentication if the server supports it. */
    if ((supported_authentications & (1 << SSH_AUTH_TIS)) &&
         options.challenge_response_authentication && !options.batch_mode) {
        if (try_challenge_response_authentication())
            goto success;
    }
    /* Try password authentication if the server supports it. */
    if ((supported_authentications & (1 << SSH_AUTH_PASSWORD)) &&
         options.password_authentication && !options.batch_mode) {
        if (try_password_authentication(server_user,host))
            goto success;
    }


    /* All authentication methods have failed.  Exit with an error message. */
    fatal("Permission denied.");
    bleep(BP_FAIL);
    return -1;

  success:
#ifdef KRB5
    /* Try Kerberos v5 TGT passing. */
    if ((supported_authentications & (1 << SSH_PASS_KERBEROS_TGT)) &&
         options.kerberos_tgt_passing && context && auth_context) {
        if (options.cipher == SSH_CIPHER_NONE)
            log("WARNING: Encryption is disabled! Ticket will be transmitted in the clear!");
        send_krb5_tgt(context, auth_context);
    }
    if (auth_context)
        krb5_auth_con_free(context, auth_context);
    if (context)
        krb5_free_context(context);
#endif

#ifdef AFS
    /* Try Kerberos v4 TGT passing if the server supports it. */
    if ((supported_authentications & (1 << SSH_PASS_KERBEROS_TGT)) &&
         options.kerberos_tgt_passing) {
        if (options.cipher == SSH_CIPHER_NONE)
            log("WARNING: Encryption is disabled! Ticket will be transmitted in the clear!");
        send_krb4_tgt();
    }

    /* Try AFS token passing if the server supports it. */
    if ((supported_authentications & (1 << SSH_PASS_AFS_TOKEN)) &&
         options.afs_token_passing && k_hasafs()) {
        if (options.cipher == SSH_CIPHER_NONE)
            log("WARNING: Encryption is disabled! Token will be transmitted in the clear!");
        send_afs_tokens();
    }
#endif /* AFS */
    return 0;
}

static int
verify_host_key_callback(Key *hostkey)
{
    if (verify_host_key(xxx_host, xxx_hostaddr, hostkey) < 0)
        return -1;
    return 0;
}

int
ssh_kex2(char *host, struct sockaddr *hostaddr)
{
    Kex *kex;
#ifdef GSSAPI
    char *orig, *gss;
    int len;
#endif

    xxx_host = host;
    xxx_hostaddr = hostaddr;

#ifdef GSSAPI
    if ( ssh_gkx )
    {
        /* Add the GSSAPI mechanisms currently supported on this client to
         * the key exchange algorithm proposal 
         */
        orig = myproposal[PROPOSAL_KEX_ALGS];
        gss = ssh_gssapi_client_mechanisms(host);
        if (gss) {
            len = strlen(orig)+strlen(gss)+2;
            myproposal[PROPOSAL_KEX_ALGS]=malloc(len);
            snprintf(myproposal[PROPOSAL_KEX_ALGS],len,"%s,%s",gss,orig);
        }
    }
#endif

    if (options.ciphers == (char *)-1) {
        log("No valid ciphers for protocol version 2 given, using defaults.");
        options.ciphers = NULL;
    }
    if (options.ciphers != NULL) {
        myproposal[PROPOSAL_ENC_ALGS_CTOS] =
            myproposal[PROPOSAL_ENC_ALGS_STOC] = options.ciphers;
    }
    myproposal[PROPOSAL_ENC_ALGS_CTOS] =
        compat_cipher_proposal(myproposal[PROPOSAL_ENC_ALGS_CTOS]);
    myproposal[PROPOSAL_ENC_ALGS_STOC] =
        compat_cipher_proposal(myproposal[PROPOSAL_ENC_ALGS_STOC]);
    if (options.compression) {
        myproposal[PROPOSAL_COMP_ALGS_CTOS] =
            myproposal[PROPOSAL_COMP_ALGS_STOC] = "zlib,none";
    } else {
        myproposal[PROPOSAL_COMP_ALGS_CTOS] =
            myproposal[PROPOSAL_COMP_ALGS_STOC] = "none,zlib";
    }
    if (options.macs != NULL) {
        myproposal[PROPOSAL_MAC_ALGS_CTOS] =
            myproposal[PROPOSAL_MAC_ALGS_STOC] = options.macs;
    }
    if (options.hostkeyalgorithms != NULL)
        myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS] =
            options.hostkeyalgorithms;

#ifdef GSSAPI
    if ( ssh_gkx )
    {
        if (gss) {
            /* If we've got GSSAPI algorithms, then we also support the
            * 'null' hostkey, as a last resort */
            orig=myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS];
            len = strlen(orig)+sizeof(",null");
            myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS]=malloc(len);
            snprintf(myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS],len,"%s,null",orig);
        }
    }
#endif

    /* start key exchange */
    kex = kex_setup(myproposal);
    kex->kex[KEX_DH_GRP1_SHA1] = kexdh_client;
    kex->kex[KEX_DH_GEX_SHA1] = kexgex_client;
#ifdef GSSAPI
    kex->kex[KEX_GSS_GRP1_SHA1] = kexgss_client;
#endif 
    kex->client_version_string=client_version_string;
    kex->server_version_string=server_version_string;
    kex->verify_host_key=&verify_host_key_callback;
    kex->host=host;

#ifdef GSSAPI
    kex->options.gss_deleg_creds=options.gss_deleg_creds;
#endif

    xxx_kex = kex;

    if (dispatch_run(DISPATCH_BLOCK, &kex->done, kex) < 0)
        return(-1);

    session_id2 = kex->session_id;
    session_id2_len = kex->session_id_len;
#ifdef GSSAPI
    if ( kex->kex_type == KEX_GSS_GRP1_SHA1 )
        datafellows |= SSH_BUG_NOREKEY;
#endif

#ifdef DEBUG_KEXDH
    /* send 1st encrypted/maced/compressed message */
    packet_start(SSH2_MSG_IGNORE);
    packet_put_cstring("markus");
    packet_send();
    packet_write_wait();
#endif
    return (0);
}

/*
 * Authenticate user
 */

typedef struct Authctxt Authctxt;
typedef struct Authmethod Authmethod;

typedef int sign_cb_fn(
    Authctxt *authctxt, Key *key,
    u_char **sigp, u_int *lenp, u_char *data, u_int datalen);

struct Authctxt {
    const char *server_user;
    const char *local_user;
    const char *host;
    const char *service;
    Authmethod *method;
    int success;
    char *authlist;
    /* pubkey */
    Key *last_key;
    sign_cb_fn *last_key_sign;
    int last_key_hint;
    AuthenticationConnection *agent;
    /* hostbased */
    Sensitive *sensitive;
#ifdef CK_SRP
    int server_auth;        /* SRP authenticates _both_ sides;
                             * this is set when the client
                             * authenticates the server. */
    SRP_CTX *srp;
#endif
    /* kbd-interactive */
    int info_req_seen;
    /* generic */
    void    *methoddata;
};

struct Authmethod {
    char        *name;          /* string to compare against server's list */
    int (*userauth)(Authctxt *authctxt);
    int *enabled;       /* flag in option struct that enables method */
    int *batch_flag;    /* flag in option struct that disables method */
};

int     input_userauth_success(int type, u_int32_t seq, void *ctxt);
int     input_userauth_failure(int type, u_int32_t seq, void *ctxt);
int     input_userauth_banner(int type, u_int32_t seq, void *ctxt);
int     input_userauth_error(int type, u_int32_t seq, void *ctxt);
int     input_userauth_info_req(int type, u_int32_t seq, void *ctxt);
int     input_userauth_pk_ok(int type, u_int32_t seq, void *ctxt);

int     input_userauth_passwd_changereq(int, u_int32_t, void *);

int     userauth_none(Authctxt *authctxt);
int     userauth_pubkey(Authctxt *authctxt);
int     userauth_passwd(Authctxt *authctxt);
int     userauth_kbdint(Authctxt *authctxt);
int     userauth_hostbased(Authctxt *authctxt);

int     userauth(Authctxt *authctxt, char *authlist);

#ifdef CK_SRP
int     userauth_srp(Authctxt *);
int     input_userauth_srp_reply(int type, u_int32_t packlen, void *ctxt);
int     input_userauth_srp_value(int type, u_int32_t packlen, void *ctxt);
int     input_userauth_srp_proof(int type, u_int32_t packlen, void *ctxt);
#endif


#ifdef GSSAPI
int     userauth_external(Authctxt *authctxt);
int     userauth_gssapi(Authctxt *authctxt);
int    input_gssapi_response(int type, u_int32_t plen, void *ctxt);
int    input_gssapi_token(int type, u_int32_t plen, void *ctxt);
int    input_gssapi_hash(int type, u_int32_t plen, void *ctxt);
int    input_gssapi_error(int, u_int32_t, void *);
int    input_gssapi_errtok(int, u_int32_t, void *);
#endif

static int
sign_and_send_pubkey(Authctxt *authctxt, Key *k,
                      sign_cb_fn *sign_callback);
static void     clear_auth_state(Authctxt *authctxt);

static Authmethod *authmethod_get(char *authlist);
static Authmethod *authmethod_lookup(const char *name);
static char *authmethods_get(void);

/* These two structures must be the same */
static Authmethod authmethods_data[] = {
#ifdef GSSAPI
    {"external-keyx",
         userauth_external,
         &options.gss_authentication,
         NULL},
    {"gssapi",
         userauth_gssapi,
         &options.gss_authentication,
         NULL},
#endif
    {"hostbased",
         userauth_hostbased,
         &options.hostbased_authentication,
         NULL},
    {"publickey",
         userauth_pubkey,
         &options.pubkey_authentication,
         NULL},
#ifdef CK_SRP
    {SRP_GEX_SHA1,
         userauth_srp,
         &options.srp_authentication,
         &options.batch_mode},
#endif
    {"keyboard-interactive",
         userauth_kbdint,
         &options.kbd_interactive_authentication,
         &options.batch_mode},
    {"password",
         userauth_passwd,
         &options.password_authentication,
         &options.batch_mode},
    {"none",
         userauth_none,
         NULL,
         NULL},
    {NULL, NULL, NULL, NULL}
};

static Authmethod authmethods[] = {
#ifdef GSSAPI
    {"external-keyx",
         userauth_external,
         &options.gss_authentication,
         NULL},
    {"gssapi",
         userauth_gssapi,
         &options.gss_authentication,
         NULL},
#endif
    {"hostbased",
         userauth_hostbased,
         &options.hostbased_authentication,
         NULL},
    {"publickey",
         userauth_pubkey,
         &options.pubkey_authentication,
         NULL},
#ifdef CK_SRP
    {SRP_GEX_SHA1,
         userauth_srp,
         &options.srp_authentication,
         &options.batch_mode},
#endif
    {"keyboard-interactive",
         userauth_kbdint,
         &options.kbd_interactive_authentication,
         &options.batch_mode},
    {"password",
         userauth_passwd,
         &options.password_authentication,
         &options.batch_mode},
    {"none",
         userauth_none,
         NULL,
         NULL},
    {NULL, NULL, NULL, NULL}
};

/* XXX internal state */
static Authmethod *current = NULL;
static char *supported = NULL;
static char *preferred = NULL;

int
authmethod_reset(void)
{
    memcpy(&authmethods,&authmethods_data,sizeof(authmethods));
    current = NULL;
    supported = NULL;
    preferred = NULL;
    return(0);
}

int
ssh_userauth2(const char *local_user, const char *server_user, char *host,
    Sensitive *sensitive)
{
    Authctxt authctxt;
    int type;

    if (options.challenge_response_authentication)
        options.kbd_interactive_authentication = 1;

    packet_start(SSH2_MSG_SERVICE_REQUEST);
    packet_put_cstring("ssh-userauth");
    packet_send();
    debug1("SSH2_MSG_SERVICE_REQUEST sent");
    packet_write_wait();
    type = packet_read();
    if (type != SSH2_MSG_SERVICE_ACCEPT) {
        fatal("Service denied authentication request: %d", type);
        return -1;
    }
    if (packet_remaining() > 0) {
        char *reply = packet_get_string(NULL);
        debug2("service_accept: %s", reply);
        free(reply);
    } else {
        debug2("buggy server: service_accept w/o service");
    }
    packet_check_eom();
    debug1("SSH2_MSG_SERVICE_ACCEPT received");

    if (options.preferred_authentications == NULL)
        options.preferred_authentications = authmethods_get();

    /* setup authentication context */
    memset(&authctxt, 0, sizeof(authctxt));
    authctxt.agent = ssh_get_authentication_connection();
    authctxt.server_user = server_user;
    authctxt.local_user = local_user;
    authctxt.host = host;
    authctxt.service = "ssh-connection";                /* service name */
    authctxt.success = 0;
    authctxt.method = authmethod_lookup("none");
    authctxt.authlist = NULL;
    authctxt.methoddata = NULL;
    authctxt.sensitive = sensitive;
    authctxt.info_req_seen = 0;
    if (authctxt.method == NULL) {
        fatal("ssh_userauth2: internal error: cannot send userauth none request");
        return -1;
    }

    /* initial userauth request */
    userauth_none(&authctxt);

    dispatch_init(&input_userauth_error);
    dispatch_set(SSH2_MSG_USERAUTH_SUCCESS, &input_userauth_success);
    dispatch_set(SSH2_MSG_USERAUTH_FAILURE, &input_userauth_failure);
    dispatch_set(SSH2_MSG_USERAUTH_BANNER, &input_userauth_banner);
    if (dispatch_run(DISPATCH_BLOCK, &authctxt.success, &authctxt) < 0) /* loop until success */
                return -1;

    if (authctxt.agent != NULL)
        ssh_close_authentication_connection(authctxt.agent);

    debug1("Authentication succeeded (%s).", authctxt.method->name);
    return (0);
}

int
userauth(Authctxt *authctxt, char *authlist)
{
    if (authctxt->methoddata!=NULL) {
        free(authctxt->methoddata);
        authctxt->methoddata=NULL;
    }
    if (authlist == NULL) {
        authlist = authctxt->authlist;
    } else {
        if (authctxt->authlist)
            free(authctxt->authlist);
        authctxt->authlist = authlist;
    }
    for (;;) {
        Authmethod *method = authmethod_get(authlist);
        if (method == NULL) {
            fatal("Permission denied (%s).", authlist);
            bleep(BP_FAIL);
            return -1;
        }
        authctxt->method = method;
        if (method->userauth(authctxt) > 0) {
            debug2("we sent a %s packet, wait for reply", method->name);
            break;
        } else {
            debug2("we did not send a packet, disable method: %s",method->name);
            method->enabled = NULL;
        }
    }
    return 0;
}

int
input_userauth_error(int type, u_int32_t seq, void *ctxt)
{
    fatal("input_userauth_error: bad message during authentication: "
           "type %d", type);
    return -1;
}
int
input_userauth_banner(int type, u_int32_t seq, void *ctxt)
{
    char *msg, *lang;
    debug3("input_userauth_banner");
    msg = packet_get_string(NULL);
    lang = packet_get_string(NULL);
    fprintf(stderr, "%s", msg);
    free(msg);
    free(lang);
    return 0;
}

int
input_userauth_success(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    if (authctxt == NULL) {
        fatal("input_userauth_success: no authentication context");
        return -1;
    }
#ifdef CK_SRP
    if (strcmp(authctxt->method->name, SRP_GEX_SHA1) == 0 &&
         !authctxt->server_auth) {
        fatal("SRP unauthenticated server sent a SUCCESS packet");
        return(-1);
    }
#endif
    if (authctxt->authlist)
        free(authctxt->authlist);
    if (authctxt->methoddata)
        free(authctxt->methoddata);
    clear_auth_state(authctxt);
    authctxt->success = 1;                      /* break out */
    return 0;
}
int
input_userauth_failure(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    char *authlist = NULL;
    int partial;

    if (authctxt == NULL) {
        fatal("input_userauth_failure: no authentication context");
        return -1;
    }
    authlist = packet_get_string(NULL);
    partial = packet_get_char();
    packet_check_eom();

    if (partial != 0)
        log("Authenticated with partial success.");
    debug1("input_userauth_failure: authentications that can continue: %s", authlist);

    clear_auth_state(authctxt);
    if (userauth(authctxt, authlist) < 0) {
        return -1;
    }
    return 0;
}
int
input_userauth_pk_ok(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    Key *key = NULL;
    Buffer b;
    int pktype, sent = 0;
    u_int alen, blen;
    char *pkalg, *fp;
    u_char *pkblob;

    if (authctxt == NULL) {
        fatal("input_userauth_pk_ok: no authentication context");
        return -1;
    }
    if (datafellows & SSH_BUG_PKOK) {
        /* this is similar to SSH_BUG_PKAUTH */
        debug2("input_userauth_pk_ok: SSH_BUG_PKOK");
        pkblob = packet_get_string(&blen);
        buffer_init(&b);
        buffer_append(&b, pkblob, blen);
        pkalg = buffer_get_string(&b, &alen);
        buffer_free(&b);
    } else {
        pkalg = packet_get_string(&alen);
        pkblob = packet_get_string(&blen);
    }
    packet_check_eom();

    debug1("Server accepts key: pkalg: %s blen: %u lastkey: %p hint %d",
           pkalg, blen, authctxt->last_key, authctxt->last_key_hint);

    do {
        if (authctxt->last_key == NULL ||
             authctxt->last_key_sign == NULL) {
            debug1("no last key or no sign cb");
            break;
        }
        if ((pktype = key_type_from_name(pkalg)) == KEY_UNSPEC) {
            debug1("unknown pkalg %s", pkalg);
            break;
        }
        if ((key = key_from_blob(pkblob, blen)) == NULL) {
            debug1("no key from blob. pkalg %s", pkalg);
            break;
        }
        if ( key->type != pktype ) {
            error("input_userauth_pk_ok: type mismatch "
                   "for decoded key (received %d, expected %d)",
                   key->type, pktype);
            break;
        }
        fp = key_fingerprint(key, SSH_FP_MD5, SSH_FP_HEX);
        debug1("input_userauth_pk_ok: fp %s", fp);
        free(fp);
        if (!key_equal(key, authctxt->last_key)) {
            debug1("key != last_key");
            break;
        }
        sent = sign_and_send_pubkey(authctxt, key,
                                     authctxt->last_key_sign);
    } while(0);

    if (key != NULL)
        key_free(key);
    free(pkalg);
    free(pkblob);

    /* unregister */
    clear_auth_state(authctxt);
    dispatch_set(SSH2_MSG_USERAUTH_PK_OK, NULL);

    /* try another method if we did not send a packet*/
    if (sent == 0)
        if (userauth(authctxt, NULL) < 0)
            return -1;
    return 0;
}

#ifdef CK_SRP
void fatal_bignum_error();

int
userauth_srp(Authctxt *authctxt)
{
    if (srp_attempt++ >= options.number_of_password_prompts)
        return 0;


#ifdef COMMENT
    /* Merged with the prompt */
    if (srp_attempt != 1) {
        error("Permission denied, please try again.");
        bleep(BP_FAIL);
    }
#endif /* COMMENT */

    /* Allocate an SRP context and update the authentication context. */

    authctxt->srp = SRP_CTX_new();
    authctxt->server_auth = 0;

    /* We begin by sending an authorization request for the method. */

    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_send();

    /* Round 1 begins when we get the reply from the server. */

    dispatch_set(SSH2_MSG_USERAUTH_SRP_REPLY, &input_userauth_srp_reply);
    return 1;
}

/* Round 1: get the SRP parameters and send the public value e. */

static int
input_userauth_srp_reply(int type, u_int32_t seq, void *ctxt)
{
    int i, plen;
    BIGNUM *tmp;
    Authctxt *authctxt;
    SRP_CTX *srp;

    /* Make sure we're where we're supposed to be. */

    authctxt = ctxt;
    if (authctxt == NULL || authctxt->srp == NULL) {
        fatal("input_userauth_srp_reply: no context");
        return -1;
    }
    srp = authctxt->srp;
    dispatch_set(SSH2_MSG_USERAUTH_SRP_REPLY, NULL);

    /* The server sent us the prime p, generator g, and salt. */

    packet_get_bignum2(srp->p);
    packet_get_bignum2(srp->g);
    srp->salt = packet_get_string(&srp->slen);
    packet_check_eom();

    /*
     * Make sure the group is OK.  XXX note that there is no check
     * that this is really the group that the user chose when
     * he or she created the verifier (that requires persistent
     * client state).
     */
    if (!is_safe_group(srp->p, srp->g)) {
        packet_disconnect("unknown/unsafe SRP group");
        return -1;
    }

    /* We'll need p-1 later. */

    if (!BN_sub(srp->pm1, srp->p, BN_value_one()))
        goto err;

    /*
     * The random exponent a is ALEN bits long, or the size of p,
     * whichever is less.
     */
    plen = BN_num_bits(srp->p);
    i = ALEN;
    if (plen < ALEN)
        i = plen;

    /*
     * Generate the random exponent a.  Loop until lg(p) < a < p-1.
     * This guarantees that 1 < e < p-1 and that e isn't some
     * obvious power of g.
     */
    if ((tmp = BN_new()) == NULL)
        goto err;
    if (!BN_set_word(tmp, plen))
        goto err;
    for (;;) {
        if (!BN_rand(srp->a, i, 0, 0))
            goto err;
        if (BN_ucmp(srp->a, tmp) > 0 && BN_ucmp(srp->a, srp->pm1) < 0)
            break;
    }
    BN_free(tmp);

    /* Generate the public value e = g^a mod p. */

    if (!BN_mod_exp(srp->e, srp->g, srp->a, srp->p, srp->ctx))
        goto err;

    /* Send it. */

    packet_start(SSH2_MSG_USERAUTH_SRP_VALUE);
    packet_put_bignum2(srp->e);
    packet_send();

    /* Round 2 begins when we get the server's public value. */

    dispatch_set(SSH2_MSG_USERAUTH_SRP_VALUE, &input_userauth_srp_value);
    return 0;

err:
    fatal_bignum_error();
    return -1;
}

static int
input_userauth_srp_value(int type, u_int32_t seq, void *ctxt)
{
    int i, plen, ok;
    char passphrase[256]="", prompt[300];
    BIGNUM *x, *K;
    Authctxt *authctxt;
    SRP_CTX *srp;

    /* Make sure we're where we're supposed to be. */

    authctxt = ctxt;
    if (authctxt == NULL || authctxt->srp == NULL) {
        fatal("input_userauth_srp_value: no context");
        return -1;
    }
    srp = authctxt->srp;
    dispatch_set(SSH2_MSG_USERAUTH_SRP_VALUE, NULL);

    /*
     * The server sent f = (v + g^b) mod p, where v is the
     * user's verifier and b is another random number.
     */
    packet_get_bignum2(srp->f);
    packet_check_eom();

    /* Sanity check f.  Abort if f is not in (1, p-1). */

    if (BN_cmp(srp->f, BN_value_one()) <= 0 ||
         BN_ucmp(srp->f, srp->pm1) >= 0) {
        packet_disconnect("bad SRP f value");
        return -1;
    }

    /* Calculate u from f. */

    srp->u = srp_u_calc(srp->f);
    if (srp->u == 0) {
        packet_disconnect("bad SRP u value");
        return -1;
    }

    /* Get the passphrase from the user. */

    /*
     * XXX it would actually be nice if we could display the prompt
     * and set no-echo mode earlier, and read the passphrase here,
     * since the calculations above can be lengthy and could happen
     * while the user is typing (we only need the passphrase in
     * memory at this point), but the cli_* routines don't have an
     * API for that.  :-(
     */

    if (srp_attempt != 1)
        bleep(BP_FAIL);

    if ( srp_attempt == 1 && pwbuf[0] && pwflg ) {
        ckstrncpy(passphrase,pwbuf,sizeof(passphrase));
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)passphrase);
#endif /* OS2 */
        ok = 1;
    } else {
        snprintf(prompt, sizeof(prompt), "%sEnter %.30s@%.128s's SRP passphrase: ",
              srp_attempt == 1 ? "" : "Permission denied, please try again.\n",
              authctxt->server_user, authctxt->host);
        ok = uq_txt(prompt,"Passphrase: ",2,NULL,passphrase, sizeof(passphrase),NULL,
                    DEFAULT_UQ_TIMEOUT);
    }

    plen = ok ? strlen(passphrase) : 0;

    /* Calculate x = HASH(salt | HASH(username | passphrase)). */

    if ((x = BN_new()) == NULL)
        goto err;
    srp_x_calc(srp->salt, srp->slen, authctxt->server_user, 
               ok ? passphrase : "", x);

    memset(passphrase, 0, sizeof(passphrase));

    /* Calculate v = g^x mod p. */

    if (!BN_mod_exp(srp->v, srp->g, x, srp->p, srp->ctx))
        goto err;

    /* Now the shared secret K = (f - v) ^ (a + u * x) mod p. */

    if (!BN_mul_word(x, srp->u))
        goto err;
    if (!BN_add(x, srp->a, x))              /* x = (a + u * x) */
        goto err;

    if ((K = BN_new()) == NULL)
        goto err;
    if (BN_copy(K, srp->f) == NULL)         /* use K as temp */
        goto err;
    if (BN_cmp(K, srp->v) < 0) {
        if (!BN_add(K, K, srp->p))
            goto err;
    }
    if (!BN_sub(srp->a, K, srp->v))         /* a = (f - v) */
        goto err;

    if (!BN_mod_exp(K, srp->a, x, srp->p, srp->ctx))
        goto err;
    BN_clear_free(x);

    /* Calculate the exchange hash and proofs. */

    srp_hash_calc(authctxt->server_user, authctxt->service,
                   authctxt->method->name, srp, K);
    BN_clear_free(K);

    /* Send the client authenticator. */

    packet_start(SSH2_MSG_USERAUTH_SRP_PROOF);
    packet_put_string(srp->m1, srp->m1len);
    packet_send();

    /* Wait for the server's authenticator. */

    dispatch_set(SSH2_MSG_USERAUTH_SRP_PROOF, &input_userauth_srp_proof);
    return 0;

err:
    fatal_bignum_error();
    return -1;
}

/* Round 3: check the proofs. */

static int
input_userauth_srp_proof(int type, u_int32_t seq, void *ctxt)
{
    int i;
    u_int mlen;
    u_char *mbuf;
    Authctxt *authctxt;
    SRP_CTX *srp;

    /* Make sure we're where we're supposed to be. */

    authctxt = ctxt;
    if (authctxt == NULL || authctxt->srp == NULL) {
        fatal("input_userauth_srp_proof: no context");
        return -1;
    }
    srp = authctxt->srp;
    dispatch_set(SSH2_MSG_USERAUTH_SRP_PROOF, NULL);

    /* Get the server's version. */

    i = packet_get_char();
    mbuf = NULL;
    if (i)
        mbuf = packet_get_string(&mlen);
    packet_check_eom();

    /* If the flag is set, we were OK; check the server. */

    if (i) {
        /*
        * If we were authenticated, but the server was not,
        * there is likely to be a serious problem.
        */
        if (mlen != srp->m2len || memcmp(mbuf, srp->m2, mlen) != 0) {
            packet_disconnect("SRP server authentication failure!");
            return -1;
        }

        authctxt->server_auth = 1;
        free(mbuf);
    }

    /*
     * Done.  Free the SRP context, and return to await the server's
     * reply packet.
     */
    SRP_CTX_free(srp);
    return 0;
}
#endif /* SRP */

#ifdef GSSAPI
int
userauth_gssapi(Authctxt *authctxt)
{
    Gssctxt *gssctxt = NULL;
    static gss_OID_set supported = NULL;
    OM_uint32 min;
    int ok=0;

    /* Things work better if we send one mechanism at a time, rather
     * than them all at once.  This means that if we fail at some point
     * in the middle of a negotiation, we can come back and try something
     * different.
     */
    if ( datafellows & SSH_OLD_GSSAPI ) return 0;

    /* Before we offer a mechanism, check that we can support it.  Don't
     * bother trying to get credentials - as the standard fallback will
     * deal with that kind of failure.
     */

    if ( supported == NULL )
        if (gss_indicate_mechs(&min, &supported) == GSS_S_UNAVAILABLE)
			return 0;

    while ( gssapi_mech<supported->count && !ok ) {
        if ( gssctxt ) ssh_gssapi_delete_ctx(&gssctxt);
        ssh_gssapi_build_ctx(&gssctxt);
        ssh_gssapi_set_oid(gssctxt,&supported->elements[gssapi_mech]);

        /* The DER encoding below only works for lengths<128, 
         * so check this here 
         */
        if ( supported->elements[gssapi_mech].length < 128 &&
             !GSS_ERROR(ssh_gssapi_import_name(gssctxt, authctxt->host))) 
        {
            ok = 1; /* Mechanism works */
        } else {
            gssapi_mech++;
        }
    }
    if ( !ok )
        return 0;

    authctxt->methoddata=(void *)gssctxt;

    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);

    packet_put_int(1);

    /* The newest gsskeyex draft stipulates that OIDs should 
     * be DER encoded, so we need to add the object type and
     * length information back on
     */
    if ( datafellows & SSH_BUG_GSSAPI_BER )
    {
        packet_put_string(supported->elements[gssapi_mech].elements,
                          supported->elements[gssapi_mech].length);
    } else {
        packet_put_int((supported->elements[gssapi_mech].length)+2);
        packet_put_char(0x06);
        packet_put_char(supported->elements[gssapi_mech].length);
        packet_put_raw(supported->elements[gssapi_mech].elements,
                        supported->elements[gssapi_mech].length);
    }

    packet_send();
    packet_write_wait();

    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_RESPONSE,&input_gssapi_response);
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN,&input_gssapi_token);
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERROR,&input_gssapi_error);
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK,&input_gssapi_errtok);

    gssapi_mech++; /* Move along to the next candidate */
    return 1;
}

int
input_gssapi_response(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    Gssctxt *gssctxt;
    OM_uint32 status,ms;
    int oidlen;
    char *oidv;
    gss_buffer_desc send_tok;

    if (authctxt == NULL) {
        fatal("input_gssapi_response: no authentication context");
        return -1;
    }
    gssctxt = authctxt->methoddata;

    /* Setup our OID */
    oidv=packet_get_string(&oidlen);

    if ( datafellows & SSH_BUG_GSSAPI_BER ) {
        if ( !ssh_gssapi_check_oid(gssctxt,oidv,oidlen) ) {
            fatal("Server returned different OID than expected");
            return -1;
        }
        ssh_gssapi_set_oid_data(gssctxt,oidv,oidlen);
    } else {
        if ( oidv[0] != 0x06 || oidv[1] != oidlen-2 ) {
            debug1("Badly encoded mechanism OID received");
            clear_auth_state(authctxt);
            userauth(authctxt,NULL);
            return 0;
        }
        if ( !ssh_gssapi_check_oid(gssctxt,oidv+2,oidlen-2) ) {
            fatal("Server returned different OID than expected");
            return -1;
        }
        ssh_gssapi_set_oid_data(gssctxt,oidv+2,oidlen-2);
    }

    packet_check_eom();

    status = ssh_gssapi_init_ctx(gssctxt, options.gss_deleg_creds,
                                  GSS_C_NO_BUFFER, &send_tok,
                                  NULL);
    if (GSS_ERROR(status)) {
        if ( send_tok.length>0 ) {
            packet_start(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK);
            packet_put_string(send_tok.value,send_tok.length);
            packet_send();
            packet_write_wait();
        }
        /* Start again with next method on list */
        debug1("input_gssapi_response. Trying to start again");
        clear_auth_state(authctxt);
        return userauth(authctxt,NULL);
    }

    /* We must have data to send */
    packet_start(SSH2_MSG_USERAUTH_GSSAPI_TOKEN);
    packet_put_string(send_tok.value,send_tok.length);
    packet_send();
    packet_write_wait();
    gss_release_buffer(&ms, &send_tok);
    return 0;
}

int
input_gssapi_token(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    Gssctxt *gssctxt;
    gss_buffer_desc send_tok,recv_tok;
    OM_uint32 status;

    if (authctxt == NULL) {
        fatal("input_gssapi_response: no authentication context");
        return -1;
    }
    gssctxt = authctxt->methoddata;

    recv_tok.value=packet_get_string(&recv_tok.length);

    status=ssh_gssapi_init_ctx(gssctxt, options.gss_deleg_creds,
                                   &recv_tok, &send_tok, NULL);

    packet_check_eom();

    if (GSS_ERROR(status)) {
        if ( send_tok.length>0 ) {
            packet_start(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK);
            packet_put_string(send_tok.value,send_tok.length);
            packet_send();
            packet_write_wait();
        }
        /* Start again with the next method in the list */
        debug1("input_gssapi_token. Trying to start again");
        clear_auth_state(authctxt);
        return userauth(authctxt,NULL);
    }

    if (send_tok.length>0) {
        packet_start(SSH2_MSG_USERAUTH_GSSAPI_TOKEN);
        packet_put_string(send_tok.value,send_tok.length);
        packet_send();
        packet_write_wait();
    }

    if (status == GSS_S_COMPLETE) {
        /* If that succeeded, send a exchange complete message */
        packet_start(SSH2_MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE);
        packet_send();
        packet_write_wait();
    }
    return 0;
}

int
input_gssapi_errtok(int type, u_int32_t plen, void *ctxt)          
{                                                                  
	Authctxt *authctxt = ctxt;                                      
	Gssctxt *gssctxt;                                               
	gss_buffer_desc send_tok,recv_tok;                              
	OM_uint32 status;                                               
	                                                                
	if (authctxt == NULL) {
		fatal("input_gssapi_response: no authentication context");  
        return -1;
    }
	gssctxt = authctxt->methoddata;                                 
	                                                                
	recv_tok.value=packet_get_string(&recv_tok.length);             
                                                                   
	/* Stick it into GSSAPI and see what it says */                 
	status=ssh_gssapi_init_ctx(gssctxt, options.gss_deleg_creds,    
				   &recv_tok, &send_tok, NULL);                     
                                                                   
	packet_check_eom();                                             
	                                                                
	/* We can't send a packet to the server */                      
                                                                   
	/* The draft says that we should wait for the server to fail    
	 * before starting the next authentication. So, we clear the    
	 * state, but don't do anything else */                         
	clear_auth_state(authctxt);                                     
	return 0;                                                         
}                                                                  

int
input_gssapi_error(int type, u_int32_t seq, void *ctxt) {
    OM_uint32 maj, min;
    char * msg;
    char * lang;

    maj=packet_get_int();
    min=packet_get_int();
    msg=packet_get_string(NULL);
    lang=packet_get_string(NULL);

    packet_check_eom();

    xfprintf(stderr, "Server GSSAPI Error:\n%s\n", msg);
    free(msg);
    free(lang);
    return 0;
}

int
userauth_external(Authctxt *authctxt)
{
    if (external_attempt++ >= 1)
        return 0;

    debug2("userauth_external");
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_send();
    packet_write_wait();
    return 1;
}
#endif /* GSSAPI */

int
userauth_none(Authctxt *authctxt)
{
    /* initial userauth request */
    debug2("userauth_none");
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_send();
    return 1;
}

int
userauth_passwd(Authctxt *authctxt)
{
    char prompt[150];
    char password[256];
    int  ok;

    if (passwd_attempt++ >= options.number_of_password_prompts)
        return 0;

    debug2("userauth_password");

    if(passwd_attempt != 1)
        bleep(BP_FAIL);
    if ( passwd_attempt == 1 && pwbuf[0] && pwflg ) {
        ckstrncpy(password,pwbuf,sizeof(password));
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)password);
#endif /* OS2 */
        ok = 1;
    } else {
        snprintf(prompt, sizeof(prompt), "%s%.30s@%.128s's password: ",
             passwd_attempt == 1 ? "" : "Permission denied, please try again.\n",
             authctxt->server_user, authctxt->host);
        ok = uq_txt(prompt,"Password: ",2,NULL,password, sizeof(password),NULL,
                    DEFAULT_UQ_TIMEOUT);
    }
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_char(0);
    packet_put_cstring(ok ? password : "");
    memset(password, 0, strlen(password));
    packet_add_padding(64);
    packet_send();

    dispatch_set(SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ, &input_userauth_passwd_changereq);
    return 1;
}

/*
 * parse PASSWD_CHANGEREQ, prompt user and send SSH2_MSG_USERAUTH_REQUEST
 */
int
input_userauth_passwd_changereq(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    char *info, *lang, oldpw[256]="", newpw[256]="", retype[256]="";
    struct txtbox tb[3];
    char prompt[150];
    int ok;

    debug2("input_userauth_passwd_changereq");

    if (authctxt == NULL) {
        fatal("input_userauth_passwd_changereq: "
               "no authentication context");
        return 0;
    }

    info = packet_get_string(NULL);
    lang = packet_get_string(NULL);
    if (strlen(info) > 0)
        log("%s", info);
    free(info);
    free(lang);
#ifdef COMMENT
    /* This uses three separate prompts. */
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_char(1);			/* additional info */
    snprintf(prompt, sizeof(prompt),
              "Enter %.30s@%.128s's old password: ",
              authctxt->server_user, authctxt->host);
    ok = uq_txt(prompt,"Old Password: ",2,NULL,
                 password, sizeof(password),NULL,DEFAULT_UQ_TIMEOUT);
    packet_put_cstring(password);
    memset(password, 0, strlen(password));
    while (!password[0]) {
        snprintf(prompt, sizeof(prompt),
                  "Enter %.30s@%.128s's new password: ",
                  authctxt->server_user, authctxt->host);
        ok = uq_txt(prompt,"New Password: ",2,NULL,
                     password, sizeof(password),NULL,DEFAULT_UQ_TIMEOUT);
        if (!ok || !password[0])
            return 1;
        snprintf(prompt, sizeof(prompt),
                  "Retype %.30s@%.128s's new password: ",
                  authctxt->server_user, authctxt->host);
        ok = uq_txt(prompt,"Retype New Password: ",2,NULL,
                     retype, sizeof(retype),NULL,DEFAULT_UQ_TIMEOUT);
        if (!ok)
            return 1;
        if (strcmp(password, retype) != 0) {
            memset(password, 0, strlen(password));
            log("Mismatch; try again, EOF to quit.");
        }
        memset(retype, 0, strlen(retype));
    }
    packet_put_cstring(password);
    memset(password, 0, strlen(password));
#else
    memset(tb,0,sizeof(struct txtbox) * 3);
    tb[0].t_buf = oldpw;
    tb[0].t_len = 256;
    tb[0].t_lbl = "Old Password: ";
    tb[0].t_dflt = NULL;
    tb[0].t_echo = 2;
    tb[1].t_buf = newpw;
    tb[1].t_len = 256;
    tb[1].t_lbl = "New Password: ";
    tb[1].t_dflt = NULL;
    tb[1].t_echo = 2;
    tb[2].t_buf = retype;
    tb[2].t_len = 256;
    tb[2].t_lbl = "New Password (again): ";
    tb[2].t_dflt = NULL;
    tb[2].t_echo = 2;

    snprintf(prompt, sizeof(prompt),
              "Password Change Requested for %.30s@%.128s.",
              authctxt->server_user, authctxt->host);
    while ( !newpw[0] ) {
        ok = uq_mtxt(prompt,NULL,3,tb);
        if (!ok)
            return 1;
        if (strcmp(newpw, retype) != 0) {
            memset(newpw, 0, strlen(newpw));
            log("Mismatch; try again, EOF to quit.");
        }
        memset(retype, 0, strlen(retype));
    }

    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_char(1);			/* additional info */
    packet_put_cstring(oldpw);
    memset(oldpw, 0, strlen(oldpw));
    packet_put_cstring(newpw);
    memset(newpw, 0, strlen(newpw));
#endif /* COMMENT */
    packet_add_padding(64);
    packet_send();

    dispatch_set(SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ, &input_userauth_passwd_changereq);
    return 1;
}

void
clear_auth_state(Authctxt *authctxt)
{
    /* XXX clear authentication state */
    dispatch_set(SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ, NULL);

#ifdef GSSAPI
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_RESPONSE, NULL);
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, NULL);
    dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERROR, NULL);
#endif 

    if (authctxt->last_key != NULL && authctxt->last_key_hint == -1) {
        debug3("clear_auth_state: key_free %p", authctxt->last_key);
        key_free(authctxt->last_key);
    }
    authctxt->last_key = NULL;
    authctxt->last_key_hint = -2;
    authctxt->last_key_sign = NULL;
}

static int
sign_and_send_pubkey(Authctxt *authctxt, Key *k, sign_cb_fn *sign_callback)
{
    Buffer b;
    u_char *blob, *signature;
    u_int bloblen, slen;
    int skip = 0;
    int ret = -1;
    int have_sig = 1;

    debug3("sign_and_send_pubkey");

    if (key_to_blob(k, &blob, &bloblen) == 0) {
        /* we cannot handle this key */
        debug3("sign_and_send_pubkey: cannot handle key");
        return 0;
    }
    memset(&b,0,sizeof(Buffer));
    /* data to be signed */
    buffer_init(&b);
    if (datafellows & SSH_OLD_SESSIONID) {
        buffer_append(&b, session_id2, session_id2_len);
        skip = session_id2_len;
    } else {
        buffer_put_string(&b, session_id2, session_id2_len);
        skip = buffer_len(&b);
    }
    buffer_put_char(&b, SSH2_MSG_USERAUTH_REQUEST);
    buffer_put_cstring(&b, authctxt->server_user);
    buffer_put_cstring(&b,
                        datafellows & SSH_BUG_PKSERVICE ?
                        "ssh-userauth" :
                        authctxt->service);
    if (datafellows & SSH_BUG_PKAUTH) {
        buffer_put_char(&b, have_sig);
    } else {
        buffer_put_cstring(&b, authctxt->method->name);
        buffer_put_char(&b, have_sig);
        buffer_put_cstring(&b, key_ssh_name(k));
    }
    buffer_put_string(&b, blob, bloblen);

    /* generate signature */
    ret = (*sign_callback)(authctxt, k, &signature, &slen,
                            buffer_ptr(&b), buffer_len(&b));
    if (ret == -1) {
        free(blob);
        buffer_free(&b);
        return 0;
    }
#ifdef DEBUG_PK
    buffer_dump(&b);
#endif
    if (datafellows & SSH_BUG_PKSERVICE) {
        buffer_clear(&b);
        buffer_append(&b, session_id2, session_id2_len);
        skip = session_id2_len;
        buffer_put_char(&b, SSH2_MSG_USERAUTH_REQUEST);
        buffer_put_cstring(&b, authctxt->server_user);
        buffer_put_cstring(&b, authctxt->service);
        buffer_put_cstring(&b, authctxt->method->name);
        buffer_put_char(&b, have_sig);
        if (!(datafellows & SSH_BUG_PKAUTH))
            buffer_put_cstring(&b, key_ssh_name(k));
        buffer_put_string(&b, blob, bloblen);
    }
    free(blob);

    /* append signature */
    buffer_put_string(&b, signature, slen);
    free(signature);

    /* skip session id and packet type */
    if (buffer_len(&b) < skip + 1) {
        fatal("userauth_pubkey: internal error");
        return -1;
    }
    buffer_consume(&b, skip + 1);

    /* put remaining data from buffer into packet */
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_raw(buffer_ptr(&b), buffer_len(&b));
    buffer_free(&b);
    packet_send();

    return 1;
}

static int
send_pubkey_test(Authctxt *authctxt, Key *k, sign_cb_fn *sign_callback,
    int hint)
{
    u_char *blob;
    u_int bloblen, have_sig = 0;

    debug3("send_pubkey_test");

    if (key_to_blob(k, &blob, &bloblen) == 0) {
        /* we cannot handle this key */
        debug3("send_pubkey_test: cannot handle key");
        return 0;
    }
    /* register callback for USERAUTH_PK_OK message */
    authctxt->last_key_sign = sign_callback;
    authctxt->last_key_hint = hint;
    authctxt->last_key = k;
    dispatch_set(SSH2_MSG_USERAUTH_PK_OK, &input_userauth_pk_ok);

    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_char(have_sig);
    if (!(datafellows & SSH_BUG_PKAUTH))
        packet_put_cstring(key_ssh_name(k));
    packet_put_string(blob, bloblen);
    free(blob);
    packet_send();
    return 1;
}

static Key *
load_identity_file(char *filename)
{
    Key *private;
    char prompt[300], passphrase[256]="";
    int quit, i, ok;
    struct _stat st;

    if (stat(filename, &st) < 0) {
        debug3("no such identity: %s", filename);
        return NULL;
    }
    private = key_load_private_type(KEY_UNSPEC, filename, "", NULL);
    if (private == NULL) {
        if (options.batch_mode)
            return NULL;
        for (i = 0; i < options.number_of_password_prompts; i++) {
            if ( i == 0 && pwbuf[0] && pwflg ) {
                ckstrncpy(passphrase,pwbuf,sizeof(passphrase));
#ifdef OS2
                if ( pwcrypt )
                    ck_encrypt((char *)passphrase);
#endif /* OS2 */
                ok = 1;
            } else {
                snprintf(prompt, sizeof prompt,
                      "%sEnter passphrase for key '%.100s': ",
                      i == 0 ? "" : "Bad passphrase given, try again...\n",
                      filename);
                ok = uq_txt(prompt,"Passphrase: ",2,NULL,
                             passphrase, sizeof(passphrase),NULL,DEFAULT_UQ_TIMEOUT);
            }
            if (ok && strcmp(passphrase, "") != 0) {
                private = key_load_private_type(KEY_UNSPEC, filename,
                                                 passphrase, NULL);
                quit = 0;
            } else {
                debug2("no passphrase given, try next key");
                quit = 1;
            }
            memset(passphrase, 0, strlen(passphrase));
            if (private != NULL || quit)
                break;
            debug2("bad passphrase given, try again...");
        }
    }
    return private;
}

static int
identity_sign_cb(Authctxt *authctxt, Key *key, u_char **sigp, u_int *lenp,
    u_char *data, u_int datalen)
{
    Key *private;
    int idx, ret;

    idx = authctxt->last_key_hint;
    if (idx < 0)
        return -1;

    /* private key is stored in external hardware */
    if (options.identity_keys[idx]->flags & KEY_FLAG_EXT)
        return key_sign(options.identity_keys[idx], sigp, lenp, data, datalen);

    private = load_identity_file(options.identity_files[idx]);
    if (private == NULL)
        return -1;
    ret = key_sign(private, sigp, lenp, data, datalen);
    key_free(private);
    return ret;
}

static int
agent_sign_cb(Authctxt *authctxt, Key *key, u_char **sigp, u_int *lenp,
    u_char *data, u_int datalen)
{
    return ssh_agent_sign(authctxt->agent, key, sigp, lenp, data, datalen);
}

static int
key_sign_cb(Authctxt *authctxt, Key *key, u_char **sigp, u_int *lenp,
    u_char *data, u_int datalen)
{
    return key_sign(key, sigp, lenp, data, datalen);
}

static int
userauth_pubkey_agent(Authctxt *authctxt)
{
    int ret = 0;
    char *comment;
    Key *k;

    if (pubkey_agent_called == 0) {
        if (ssh_get_num_identities(authctxt->agent, 2) == 0)
            debug2("userauth_pubkey_agent: no keys at all");
        pubkey_agent_called = 1;
    }
    k = ssh_get_next_identity(authctxt->agent, &comment, 2);
    if (k == NULL) {
        debug1("userauth_pubkey_agent: no more keys");
    } else {
        debug1("Offering agent key %s", comment);
        free(comment);
        ret = send_pubkey_test(authctxt, k, agent_sign_cb, -1);
        if (ret == 0)
            key_free(k);
    }
    if (ret == 0)
        debug2("userauth_pubkey_agent: no message sent");
    return ret;
}

int
userauth_pubkey(Authctxt *authctxt)
{
    int sent = 0;
    Key *key;
    char *filename;

    if (authctxt->agent != NULL) {
        do {
            sent = userauth_pubkey_agent(authctxt);
        } while(!sent && authctxt->agent->howmany > 0);
    }
    while (!sent && pubkey_idx < options.num_identity_files) {
        key = options.identity_keys[pubkey_idx];
        filename = options.identity_files[pubkey_idx];
        if (key == NULL) {
            debug1("Trying private key: %s", filename);
            key = load_identity_file(filename);
            if (key != NULL) {
                sent = sign_and_send_pubkey(authctxt, key,
                                             key_sign_cb);
                key_free(key)   ;
            }
        } else if (key->type != KEY_RSA1) {
            debug1("Offering public key: %s", filename);
            sent = send_pubkey_test(authctxt, key,
                                     identity_sign_cb, pubkey_idx);
        }
        pubkey_idx++;
    }
    return sent;
}

/*
 * Send userauth request message specifying keyboard-interactive method.
 */
int
userauth_kbdint(Authctxt *authctxt)
{
    if (kbdint_attempt++ >= options.number_of_password_prompts)
        return 0;

    /* disable if no SSH2_MSG_USERAUTH_INFO_REQUEST has been seen */
    if (kbdint_attempt > 1 && !authctxt->info_req_seen) {
        debug3("userauth_kbdint: disable: no info_req_seen");
        dispatch_set(SSH2_MSG_USERAUTH_INFO_REQUEST, NULL);
        return 0;
    }

    debug2("userauth_kbdint");
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_cstring("");                                     /* lang */
    packet_put_cstring(options.kbd_interactive_devices ?
                        options.kbd_interactive_devices : "");
    packet_send();

    dispatch_set(SSH2_MSG_USERAUTH_INFO_REQUEST, &input_userauth_info_req);
    return 1;
}

/*
 * parse INFO_REQUEST, prompt user and send INFO_RESPONSE
 */
int
input_userauth_info_req(int type, u_int32_t seq, void *ctxt)
{
    Authctxt *authctxt = ctxt;
    char *name, *inst, *lang, *prompt, response[256];
    u_int num_prompts, i;
    int echo = 0;
    struct txtbox * tb = NULL;

    debug2("input_userauth_info_req");

    if (authctxt == NULL) {
        fatal("input_userauth_info_req: no authentication context");
        return -1;
    }

    authctxt->info_req_seen = 1;

    name = packet_get_string(NULL);
    inst = packet_get_string(NULL);
    lang = packet_get_string(NULL);
    if (strlen(name) > 0)
        log("%s", name);
    if (strlen(inst) > 0)
        log("%s", inst);
    free(name);
    free(inst);
    free(lang);

    num_prompts = packet_get_int();
    /*
     * Begin to build info response packet based on prompts requested.
     * We commit to providing the correct number of responses, so if
     * further on we run into a problem that prevents this, we have to
     * be sure and clean this up and send a correct error response.
     */
    packet_start(SSH2_MSG_USERAUTH_INFO_RESPONSE);
    packet_put_int(num_prompts);

    debug2("input_userauth_info_req: num_prompts %d", num_prompts);
    if ( num_prompts == 1 && pwbuf[0] && pwflg ) {
        prompt = packet_get_string(NULL);
        echo = packet_get_char();

        ckstrncpy(response,pwbuf,sizeof(response));
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)response);
#endif /* OS2 */
        
        packet_put_cstring(response);
        memset(response, 0, strlen(response));
        free(prompt);
    } else if (num_prompts > 0) {
        char preface[256];
        tb = (struct txtbox *) malloc(sizeof(struct txtbox) * num_prompts);
        if ( tb != NULL ) {
            int ok;
            memset(tb,0,sizeof(struct txtbox) * num_prompts);
            for ( i=0; i < num_prompts; i++ ) {
                tb[i].t_buf = malloc(256);
                tb[i].t_len = 256;
                tb[i].t_lbl = packet_get_string(NULL);
                tb[i].t_dflt = NULL;
                tb[i].t_echo = (packet_get_char() ? 1 : 2);
            }

            ckmakmsg(preface,256,"SSH Input User Authentication for \"",
                     (char *)authctxt->local_user,"\"\n",NULL);
            ok = uq_mtxt(preface,NULL,num_prompts,tb);

            for ( i=0; i < num_prompts; i++ ) {
                if (ok)
                    packet_put_cstring(tb[i].t_buf);
                memset(tb[i].t_buf, 0, tb[i].t_len);
                free(tb[i].t_lbl);
                free(tb[i].t_buf);
            }
            free(tb);
        }
    }

    packet_check_eom(); /* done with parsing incoming message. */

    packet_add_padding(64);
    packet_send();
    return (0);
}

#ifdef UNIX
static int
ssh_keysign(Key *key, u_char **sigp, u_int *lenp,
    u_char *data, u_int datalen)
{
	Buffer b;
	struct _stat st;
	pid_t pid;
	int to[2], from[2], status, version = 2;

	debug2("ssh_keysign called");

	if (stat(_PATH_SSH_KEY_SIGN, &st) < 0) {
		error("ssh_keysign: no installed: %s", strerror(errno));
		return -1;
	}
	if (fflush(stdout) != 0)
		error("ssh_keysign: fflush: %s", strerror(errno));
	if (pipe(to) < 0) {
		error("ssh_keysign: pipe: %s", strerror(errno));
		return -1;
	}
	if (pipe(from) < 0) {
		error("ssh_keysign: pipe: %s", strerror(errno));
		return -1;
	}
	if ((pid = fork()) < 0) {
		error("ssh_keysign: fork: %s", strerror(errno));
		return -1;
	}
	if (pid == 0) {
		seteuid(getuid());
		setuid(getuid());
		close(from[0]);
		if (dup2(from[1], STDOUT_FILENO) < 0)
			fatal("ssh_keysign: dup2: %s", strerror(errno));
		close(to[1]);
		if (dup2(to[0], STDIN_FILENO) < 0)
			fatal("ssh_keysign: dup2: %s", strerror(errno));
		close(from[1]);
		close(to[0]);
		execl(_PATH_SSH_KEY_SIGN, _PATH_SSH_KEY_SIGN, (char *) 0);
		fatal("ssh_keysign: exec(%s): %s", _PATH_SSH_KEY_SIGN,
		    strerror(errno));
	}
	close(from[1]);
	close(to[0]);

	buffer_init(&b);
	buffer_put_int(&b, packet_get_connection_in()); /* send # of socket */
	buffer_put_string(&b, data, datalen);
	ssh_msg_send(to[1], version, &b);

	if (ssh_msg_recv(from[0], &b) < 0) {
		error("ssh_keysign: no reply");
		buffer_clear(&b);
		return -1;
	}
	close(from[0]);
	close(to[1]);

	while (waitpid(pid, &status, 0) < 0)
		if (errno != EINTR)
			break;

	if (buffer_get_char(&b) != version) {
		error("ssh_keysign: bad version");
		buffer_clear(&b);
		return -1;
	}
	*sigp = buffer_get_string(&b, lenp);
	buffer_clear(&b);

	return 0;
}
#endif /* UNIX */

int
userauth_hostbased(Authctxt *authctxt)
{
    Key *private = NULL;
    Sensitive * sensitive = authctxt->sensitive;
    Buffer b;
    u_char *signature, *blob;
    char *chost, *pkalg, *p;
    const char *service;
    u_int blen, slen;
    int ok, i, len, found = 0;

    /* check for a useful key */
    for (i = 0; i < sensitive->nkeys; i++) {
        private = sensitive->keys[i];
        if (private && private->type != KEY_RSA1) {
            found = 1;
            /* we take and free the key */
            sensitive->keys[i] = NULL;
            break;
        }
    }
    if (!found) {
        debug1("No more client hostkeys for hostbased authentication.");
        return 0;
    }
    if (key_to_blob(private, &blob, &blen) == 0) {
        key_free(private);
        return 0;
    }
    /* figure out a name for the client host */
    p = get_local_name(packet_get_connection_in());
    if (p == NULL) {
        error("userauth_hostbased: cannot get local ipaddr/name");
        key_free(private);
        return 0;
    }
    len = strlen(p) + 2;
    chost = malloc(len);
    strlcpy(chost, p, len);
    strlcat(chost, ".", len);
    free(p);
    debug2("userauth_hostbased: chost %s", chost);

    service = datafellows & SSH_BUG_HBSERVICE ? "ssh-userauth" :
        authctxt->service;
    pkalg = strdup(key_ssh_name(private));
    buffer_init(&b);
    /* construct data */
    buffer_put_string(&b, session_id2, session_id2_len);
    buffer_put_char(&b, SSH2_MSG_USERAUTH_REQUEST);
    buffer_put_cstring(&b, authctxt->server_user);
    buffer_put_cstring(&b, service);
    buffer_put_cstring(&b, authctxt->method->name);
    buffer_put_cstring(&b, pkalg);
    buffer_put_string(&b, blob, blen);
    buffer_put_cstring(&b, chost);
    buffer_put_cstring(&b, authctxt->local_user);
#ifdef DEBUG_PK
    buffer_dump(&b);
#endif
#ifdef UNIX
    if ( sensitive->external_keysign )
        ok = ssh_keysign(private, &signature, &slen, buffer_ptr(&b), buffer_len(&b));
    else
#endif /* UNIX */
        ok = key_sign(private, &signature, &slen, buffer_ptr(&b), buffer_len(&b));
    key_free(private);
    buffer_free(&b);
    if (ok != 0) {
        error("key_sign failed");
        free(chost);
        free(pkalg);
        return 0;
    }
    packet_start(SSH2_MSG_USERAUTH_REQUEST);
    packet_put_cstring(authctxt->server_user);
    packet_put_cstring(authctxt->service);
    packet_put_cstring(authctxt->method->name);
    packet_put_cstring(pkalg);
    packet_put_string(blob, blen);
    packet_put_cstring(chost);
    packet_put_cstring(authctxt->local_user);
    packet_put_string(signature, slen);
    memset(signature, 's', slen);
    free(signature);
    free(chost);
    free(pkalg);

    packet_send();
    return 1;
}

/* find auth method */

/*
 * given auth method name, if configurable options permit this method fill
 * in auth_ident field and return true, otherwise return false.
 */
static int
authmethod_is_enabled(Authmethod *method)
{
    if (method == NULL)
        return 0;
    /* return false if options indicate this method is disabled */
    if  (method->enabled == NULL || *method->enabled == 0)
        return 0;
    /* return false if batch mode is enabled but method needs interactive mode */
    if  (method->batch_flag != NULL && *method->batch_flag != 0)
        return 0;
    return 1;
}

static Authmethod *
authmethod_lookup(const char *name)
{
    Authmethod *method = NULL;
    if (name != NULL)
        for (method = authmethods; method->name != NULL; method++)
            if (strcmp(name, method->name) == 0)
                return method;
    debug2("Unrecognized authentication method name: %s", name ? name : "NULL");
    return NULL;
}

/*
 * Given the authentication method list sent by the server, return the
 * next method we should try.  If the server initially sends a nil list,
 * use a built-in default list.
 */
static Authmethod *
authmethod_get(char *authlist)
{

    char *name = NULL;
    u_int next;

    /* Use a suitable default if we're passed a nil list.  */
    if (authlist == NULL || strlen(authlist) == 0)
        authlist = options.preferred_authentications;

    if (supported == NULL || strcmp(authlist, supported) != 0) {
        debug3("start over, passed a different list %s", authlist);
        if (supported != NULL)
            free(supported);
        supported = strdup(authlist);
        preferred = options.preferred_authentications;
        debug3("preferred %s", preferred);
        current = NULL;
    } else if (current != NULL && authmethod_is_enabled(current))
        return current;

    for (;;) {
        if ((name = match_list(preferred, supported, &next)) == NULL) {
            debug1("No more authentication methods to try.");
            current = NULL;
            return NULL;
        }
        preferred += next;
        debug3("authmethod_lookup %s", name);
        debug3("remaining preferred: %s", preferred);
        if ((current = authmethod_lookup(name)) != NULL &&
             authmethod_is_enabled(current)) {
            debug3("authmethod_is_enabled %s", name);
            debug1("Next authentication method: %s", name);
            return current;
        }
    }
    /* NOT REACHED */
}


static char *
authmethods_get(void)
{
    Authmethod *method = NULL;
    Buffer b;
    char *list;

	memset(&b,0,sizeof(Buffer));
    buffer_init(&b);
    for (method = authmethods; method->name != NULL; method++) {
        if (authmethod_is_enabled(method)) {
            if ( buffer_len(&b) > 0 )
                buffer_append(&b, ",", 1);
            buffer_append(&b, method->name, strlen(method->name));
        }
    }
    buffer_append(&b, "\0", 1);
    list = strdup(buffer_ptr(&b));
    buffer_free(&b);
    return list;
}

/* Format of the configuration file:

   # Configuration data is parsed as follows:
   #  1. command line options
   #  2. user-specific file
   #  3. system-wide file
   # Any configuration value is only changed the first time it is set.
   # Thus, host-specific definitions should be at the beginning of the
   # configuration file, and defaults at the end.

   # Host-specific declarations.  These may override anything above.  A single
   # host may match multiple declarations; these are processed in the order
   # that they are given in.

   Host *.ngs.fi ngs.fi
     User foo

   Host fake.com
     HostName another.host.name.real.org
     User blaah
     Port 34289
     ForwardX11 no
     ForwardAgent no

   Host books.com
     RemoteForward 9999 shadows.cs.hut.fi:9999
     Cipher 3des

   Host fascist.blob.com
     Port 23123
     User tylonen
     RhostsAuthentication no
     PasswordAuthentication no

   Host puukko.hut.fi
     User t35124p
     ProxyCommand ssh-proxy %h %p

   Host *.fr
     PublicKeyAuthentication no

   Host *.su
     Cipher none
     PasswordAuthentication no

   # Defaults for various options
   Host *
     ForwardAgent no
     ForwardX11 no
     RhostsAuthentication yes
     PasswordAuthentication yes
     RSAAuthentication yes
     RhostsRSAAuthentication yes
     StrictHostKeyChecking yes
     KeepAlives no
     IdentityFile ~/.ssh/identity
     Port 22
     EscapeChar ~

*/

/* Keyword tokens. */

typedef enum {
        oBadOption,
        oForwardAgent, oForwardX11, oGatewayPorts, oRhostsAuthentication,
        oPasswordAuthentication, oRSAAuthentication,
        oChallengeResponseAuthentication, oXAuthLocation,
#if defined(KRB4)
        oKrb4Authentication,
#endif /* KRB4 */
#if defined(KRB5)
        oKrb5Authentication,
#endif /* KRB5 */
#ifdef GSSAPI
        oGssAuthentication, oGssDelegateCreds,
#ifdef GSI
        oGssGlobusDelegateLimitedCreds,
#endif /* GSI */
#endif /* GSSAPI */
#if defined(AFS) || defined(KRB5)
        oKerberosTgtPassing,
#endif
#ifdef AFS
        oAFSTokenPassing,
#endif
#ifdef SRP
        oSRPAuthentication,
#endif
        oIdentityFile, oHostName, oPort, oCipher, oRemoteForward, oLocalForward,
        oUser, oHost, oEscapeChar, oRhostsRSAAuthentication, oProxyCommand,
        oGlobalKnownHostsFile, oUserKnownHostsFile, oConnectionAttempts,
        oBatchMode, oCheckHostIP, oStrictHostKeyChecking, oCompression,
        oCompressionLevel, oKeepAlives, oHeartbeat, oNumberOfPasswordPrompts,
        oUsePrivilegedPort, oLogLevel, oCiphers, oProtocol, oMacs,
        oGlobalKnownHostsFile2, oUserKnownHostsFile2, oPubkeyAuthentication,
        oKbdInteractiveAuthentication, oKbdInteractiveDevices, oHostKeyAlias,
        oDynamicForward, oPreferredAuthentications, oHostbasedAuthentication,
        oHostKeyAlgorithms, oBindAddress, oSmartcardDevice,
        oClearAllForwardings, oNoHostAuthenticationForLocalhost,
        oEnableSSHKeysign, 
        oDeprecated
} OpCodes;

/* Textual representations of the tokens. */

static struct {
        const char *name;
        OpCodes opcode;
} keywords[] = {
        { "forwardagent", oForwardAgent },
        { "forwardx11", oForwardX11 },
        { "xauthlocation", oXAuthLocation },
        { "gatewayports", oGatewayPorts },
        { "useprivilegedport", oUsePrivilegedPort },
        { "rhostsauthentication", oRhostsAuthentication },
        { "passwordauthentication", oPasswordAuthentication },
        { "kbdinteractiveauthentication", oKbdInteractiveAuthentication },
        { "kbdinteractivedevices", oKbdInteractiveDevices },
        { "rsaauthentication", oRSAAuthentication },
        { "pubkeyauthentication", oPubkeyAuthentication },
        { "dsaauthentication", oPubkeyAuthentication },             /* alias */
        { "rhostsrsaauthentication", oRhostsRSAAuthentication },
        { "hostbasedauthentication", oHostbasedAuthentication },
        { "challengeresponseauthentication", oChallengeResponseAuthentication },
        { "skeyauthentication", oChallengeResponseAuthentication }, /* alias */
        { "tisauthentication", oChallengeResponseAuthentication },  /* alias */
#if defined(KRB4)
        { "krb4authentication", oKrb4Authentication },
#endif /* KRB4 */
#if defined(KRB5)
        { "krb5authentication", oKrb5Authentication },
#endif /* KRB5 */
#ifdef GSSAPI
        { "gssapiauthentication", oGssAuthentication },
        { "gssapidelegatecredentials", oGssDelegateCreds },
#ifdef GSI
        /* For backwards compatability with old 1.2.27 client code */
        { "forwardgssapiglobusproxy", oGssDelegateCreds }, /* alias */
        { "forwardgssapiglobuslimitedproxy", oGssGlobusDelegateLimitedCreds },
#endif /* GSI */
#endif /* GSSAPI */
#if defined(AFS) || defined(KRB5)
        { "kerberostgtpassing", oKerberosTgtPassing },
#endif
#ifdef AFS
        { "afstokenpassing", oAFSTokenPassing },
#endif
#ifdef SRP
        { "srpauthentication", oSRPAuthentication },
#endif
        { "fallbacktorsh", oDeprecated },
        { "usersh", oDeprecated },
        { "identityfile", oIdentityFile },
        { "identityfile2", oIdentityFile },                     /* alias */
        { "hostname", oHostName },
        { "hostkeyalias", oHostKeyAlias },
        { "proxycommand", oProxyCommand },
        { "port", oPort },
        { "cipher", oCipher },
        { "ciphers", oCiphers },
        { "macs", oMacs },
        { "protocol", oProtocol },
        { "remoteforward", oRemoteForward },
        { "localforward", oLocalForward },
        { "user", oUser },
        { "host", oHost },
        { "escapechar", oEscapeChar },
        { "globalknownhostsfile", oGlobalKnownHostsFile },
        { "userknownhostsfile", oUserKnownHostsFile },          /* obsolete */
        { "globalknownhostsfile2", oGlobalKnownHostsFile2 },
        { "userknownhostsfile2", oUserKnownHostsFile2 },        /* obsolete */
        { "connectionattempts", oConnectionAttempts },
        { "batchmode", oBatchMode },
        { "checkhostip", oCheckHostIP },
        { "stricthostkeychecking", oStrictHostKeyChecking },
        { "compression", oCompression },
        { "compressionlevel", oCompressionLevel },
        { "keepalive", oKeepAlives },
        { "heartbeat", oHeartbeat },
        { "numberofpasswordprompts", oNumberOfPasswordPrompts },
        { "loglevel", oLogLevel },
        { "dynamicforward", oDynamicForward },
        { "preferredauthentications", oPreferredAuthentications },
        { "hostkeyalgorithms", oHostKeyAlgorithms },
        { "bindaddress", oBindAddress },
        { "smartcarddevice", oSmartcardDevice },
        { "clearallforwardings", oClearAllForwardings },
        { "enablesshkeysign", oEnableSSHKeysign },
        { "nohostauthenticationforlocalhost", oNoHostAuthenticationForLocalhost },
        { NULL, oBadOption }
};

/*
 * Adds a local TCP/IP port forward to options.  Never returns if there is an
 * error.
 */

int
add_local_forward(Options *options, u_short port, const char *host,
                  u_short host_port)
{
    Forward *fwd;

    if (options->num_local_forwards >= SSH_MAX_FORWARDS_PER_DIRECTION) {
        fatal("Too many local forwards (max %d).", SSH_MAX_FORWARDS_PER_DIRECTION);
        return -1;
    }
    fwd = &options->local_forwards[options->num_local_forwards++];
    fwd->port = port;
    fwd->host = strdup(host);
    fwd->host_port = host_port;
    return 0;
}

/*
 * Adds a remote TCP/IP port forward to options.  Never returns if there is
 * an error.
 */

int
add_remote_forward(Options *options, u_short port, const char *host,
                   u_short host_port)
{
    Forward *fwd;
    if (options->num_remote_forwards >= SSH_MAX_FORWARDS_PER_DIRECTION) {
        fatal("Too many remote forwards (max %d).",
               SSH_MAX_FORWARDS_PER_DIRECTION);
        return -1;
    }
    fwd = &options->remote_forwards[options->num_remote_forwards++];
    fwd->port = port;
    fwd->host = strdup(host);
    fwd->host_port = host_port;
    return 0;
}

static void
clear_forwardings(Options *options)
{
    int i;

    for (i = 0; i < options->num_local_forwards; i++)
        free(options->local_forwards[i].host);
    options->num_local_forwards = 0;
    for (i = 0; i < options->num_remote_forwards; i++)
        free(options->remote_forwards[i].host);
    options->num_remote_forwards = 0;
}

/*
 * Returns the number of the token pointed to by cp or oBadOption.
 */

static OpCodes
parse_token(const char *cp, const char *filename, int linenum)
{
    u_int i;

    for (i = 0; keywords[i].name; i++)
        if (strcasecmp(cp, keywords[i].name) == 0)
            return keywords[i].opcode;

    error("%s: line %d: Bad configuration option: %s",
           filename, linenum, cp);
    return oBadOption;
}

/*
 * Processes a single option line as used in the configuration files. This
 * only sets those values that have not already been set.
 */

#define WHITESPACE " \t\r\n"

int
process_config_line(Options *options, const char *host,
                    char *line, const char *filename, int linenum,
                    int *activep)
{
    char buf[256], *s, **charptr, *endofnumber, *keyword, *arg;
    int opcode, *intptr, value;
    size_t len;
    u_short fwd_port, fwd_host_port;
    char sfwd_host_port[6];

    s = line;
    /* Get the keyword. (Each line is supposed to begin with a keyword). */
    keyword = strdelim(&s);
    /* Ignore leading whitespace. */
    if (*keyword == '\0')
        keyword = strdelim(&s);
    if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
        return 0;

    opcode = parse_token(keyword, filename, linenum);

    switch (opcode) {
    case oBadOption:
        /* don't panic, but count bad options */
        return -1;
        /* NOTREACHED */
    case oForwardAgent:
        intptr = &options->forward_agent;
      parse_flag:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing yes/no argument.", filename, linenum);
            return -1;
        }
        value = 0;      /* To avoid compiler warning... */
        if (strcmp(arg, "yes") == 0 || strcmp(arg, "true") == 0)
            value = 1;
        else if (strcmp(arg, "no") == 0 || strcmp(arg, "false") == 0)
            value = 0;
        else
            fatal("%.200s line %d: Bad yes/no argument.", filename, linenum);
        if (*activep && *intptr == -1)
            *intptr = value;
        break;

    case oForwardX11:
        intptr = &options->forward_x11;
        goto parse_flag;

    case oGatewayPorts:
        intptr = &options->gateway_ports;
        goto parse_flag;

    case oUsePrivilegedPort:
        intptr = &options->use_privileged_port;
        goto parse_flag;

    case oRhostsAuthentication:
        intptr = &options->rhosts_authentication;
        goto parse_flag;

    case oPasswordAuthentication:
        intptr = &options->password_authentication;
        goto parse_flag;

        case oKbdInteractiveAuthentication:
        intptr = &options->kbd_interactive_authentication;
        goto parse_flag;

    case oKbdInteractiveDevices:
        charptr = &options->kbd_interactive_devices;
        goto parse_string;

    case oPubkeyAuthentication:
        intptr = &options->pubkey_authentication;
        goto parse_flag;

    case oRSAAuthentication:
        intptr = &options->rsa_authentication;
        goto parse_flag;

    case oRhostsRSAAuthentication:
        intptr = &options->rhosts_rsa_authentication;
        goto parse_flag;

    case oHostbasedAuthentication:
        intptr = &options->hostbased_authentication;
        goto parse_flag;

    case oChallengeResponseAuthentication:
        intptr = &options->challenge_response_authentication;
        goto parse_flag;

#if defined (KRB4)
    case oKrb4Authentication:
        intptr = &options->krb4_authentication;
        goto parse_flag;
#endif /* KRB4 / KRB5 */

#if defined(KRB5)
    case oKrb5Authentication:
        intptr = &options->krb5_authentication;
        goto parse_flag;
#endif /* KRB4 / KRB5 */

#ifdef GSSAPI
    case oGssAuthentication:
        intptr = &options->gss_authentication;
        goto parse_flag;

    case oGssDelegateCreds:
        intptr = &options->gss_deleg_creds;
        goto parse_flag;

#ifdef GSI
    case oGssGlobusDelegateLimitedCreds:
        intptr = &options->gss_globus_deleg_limited_proxy;
        goto parse_flag;
#endif /* GSI */
#endif /* GSSAPI */

#if defined(AFS) || defined(KRB5)
    case oKerberosTgtPassing:
        intptr = &options->kerberos_tgt_passing;
        goto parse_flag;
#endif
#ifdef AFS
    case oAFSTokenPassing:
        intptr = &options->afs_token_passing;
        goto parse_flag;
#endif

#ifdef SRP
    case oSRPAuthentication:
        intptr = &options->srp_authentication;
        goto parse_flag;
#endif

    case oBatchMode:
        intptr = &options->batch_mode;
        goto parse_flag;

    case oCheckHostIP:
        intptr = &options->check_host_ip;
        goto parse_flag;

    case oStrictHostKeyChecking:
        intptr = &options->strict_host_key_checking;
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing yes/no/ask argument.",
                   filename, linenum);
            return -1;
        }
        value = 0;      /* To avoid compiler warning... */
        if (strcmp(arg, "yes") == 0 || strcmp(arg, "true") == 0)
            value = 1;
        else if (strcmp(arg, "no") == 0 || strcmp(arg, "false") == 0)
            value = 0;
        else if (strcmp(arg, "ask") == 0)
            value = 2;
        else {
            fatal("%.200s line %d: Bad yes/no/ask argument.", filename, linenum);
            return -1;
        }
        if (*activep && *intptr == -1)
            *intptr = value;
        break;

    case oCompression:
        intptr = &options->compression;
        goto parse_flag;

    case oKeepAlives:
        intptr = &options->keepalives;
        goto parse_flag;

    case oHeartbeat:
        intptr = &options->heartbeat_interval;
        goto parse_flag;

    case oNumberOfPasswordPrompts:
        intptr = &options->number_of_password_prompts;
        goto parse_int;

    case oCompressionLevel:
        intptr = &options->compression_level;
        goto parse_int;

    case oIdentityFile:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (*activep) {
            intptr = &options->num_identity_files;
            if (*intptr >= SSH_MAX_IDENTITY_FILES)
                fatal("%.200s line %d: Too many identity files specified (max %d).",
                       filename, linenum, SSH_MAX_IDENTITY_FILES);
            charptr =  &options->identity_files[*intptr];
            *charptr = strdup(arg);
            *intptr = *intptr + 1;
        }
        break;

    case oXAuthLocation:
        charptr=&options->xauth_location;
        goto parse_string;

    case oUser:
        charptr = &options->user;
      parse_string:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (*activep && *charptr == NULL)
            *charptr = strdup(arg);
        break;

    case oGlobalKnownHostsFile:
        charptr = &options->system_hostfile;
        goto parse_string;

    case oUserKnownHostsFile:
        charptr = &options->user_hostfile;
        goto parse_string;

    case oGlobalKnownHostsFile2:
        charptr = &options->system_hostfile2;
        goto parse_string;

    case oUserKnownHostsFile2:
        charptr = &options->user_hostfile2;
        goto parse_string;

    case oHostName:
        charptr = &options->hostname;
        goto parse_string;

    case oHostKeyAlias:
        charptr = &options->host_key_alias;
        goto parse_string;

    case oPreferredAuthentications:
        charptr = &options->preferred_authentications;
        goto parse_string;

    case oBindAddress:
        charptr = &options->bind_address;
        goto parse_string;

    case oSmartcardDevice:
        charptr = &options->smartcard_device;
        goto parse_string;

    case oProxyCommand:
        charptr = &options->proxy_command;
        len = strspn(s, WHITESPACE "=");
        if (*activep && *charptr == NULL)
            *charptr = strdup(s + len);
        return 0;

    case oPort:
        intptr = &options->port;
      parse_int:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (arg[0] < '0' || arg[0] > '9') {
            fatal("%.200s line %d: Bad number.", filename, linenum);
            return -1;
        }

        /* Octal, decimal, or hex format? */
        value = strtol(arg, &endofnumber, 0);
        if (arg == endofnumber) {
            fatal("%.200s line %d: Bad number.", filename, linenum);
            return -1;
        }
        if (*activep && *intptr == -1)
            *intptr = value;
        break;

    case oConnectionAttempts:
        intptr = &options->connection_attempts;
        goto parse_int;

    case oCipher:
        intptr = &options->cipher;
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        value = cipher_number(arg);
        if (value == -1) {
            fatal("%.200s line %d: Bad cipher '%s'.",
                   filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && *intptr == -1)
            *intptr = value;
        break;

    case oCiphers:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (!ciphers_valid(arg)) {
            fatal("%.200s line %d: Bad SSH2 cipher spec '%s'.",
                              filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && options->ciphers == NULL)
            options->ciphers = strdup(arg);
        break;

    case oMacs:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (!mac_valid(arg)) {
            fatal("%.200s line %d: Bad SSH2 Mac spec '%s'.",
                   filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && options->macs == NULL)
            options->macs = strdup(arg);
        break;

    case oHostKeyAlgorithms:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (!key_names_valid2(arg)) {
            fatal("%.200s line %d: Bad protocol 2 host key algorithms '%s'.",
                              filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && options->hostkeyalgorithms == NULL)
            options->hostkeyalgorithms = strdup(arg);
        break;

    case oProtocol:
        intptr = &options->protocol;
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        value = proto_spec(arg);
        if (value == SSH_PROTO_UNKNOWN) {
            fatal("%.200s line %d: Bad protocol spec '%s'.",
                              filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && *intptr == SSH_PROTO_UNKNOWN)
            *intptr = value;
        break;

    case oLogLevel:
        intptr = (int *) &options->log_level;
        arg = strdelim(&s);
        value = log_level_number(arg);
        if (value == SYSLOG_LEVEL_NOT_SET) {
            fatal("%.200s line %d: unsupported log level '%s'",
                              filename, linenum, arg ? arg : "<NONE>");
            return -1;
        }
        if (*activep && (LogLevel) * intptr == SYSLOG_LEVEL_NOT_SET)
            *intptr = (LogLevel) value;
        break;

    case oLocalForward:
    case oRemoteForward:
        arg = strdelim(&s);
        if (!arg || *arg == '\0')
            fatal("%.200s line %d: Missing port argument.",
                   filename, linenum);
        if ((fwd_port = a2port(arg)) == 0)
            fatal("%.200s line %d: Bad listen port.",
                   filename, linenum);
        arg = strdelim(&s);
        if (!arg || *arg == '\0')
            fatal("%.200s line %d: Missing second argument.",
                   filename, linenum);
        if (sscanf(arg, "%255[^:]:%5[0-9]", buf, sfwd_host_port) != 2 &&
             sscanf(arg, "%255[^/]/%5[0-9]", buf, sfwd_host_port) != 2)
            fatal("%.200s line %d: Bad forwarding specification.",
                   filename, linenum);
        if ((fwd_host_port = a2port(sfwd_host_port)) == 0)
            fatal("%.200s line %d: Bad forwarding port.",
                   filename, linenum);
        if (*activep) {
            if (opcode == oLocalForward)
                add_local_forward(options, fwd_port, buf,
                                   fwd_host_port);
            else if (opcode == oRemoteForward)
                add_remote_forward(options, fwd_port, buf,
                                    fwd_host_port);
                }
                break;

    case oDynamicForward:
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing port argument.",
                   filename, linenum);
            return -1;
        }
        fwd_port = a2port(arg);
        if (fwd_port == 0) {
            fatal("%.200s line %d: Badly formatted port number.",
                   filename, linenum);
            return -1;
        }
        add_local_forward(options, fwd_port, "socks4", 0);
        break;

    case oClearAllForwardings:
        intptr = &options->clear_forwardings;
        goto parse_flag;

    case oHost:
        *activep = 0;
        while ((arg = strdelim(&s)) != NULL && *arg != '\0')
            if (match_pattern(host, arg)) {
                debug1("Applying options for %s", arg);
                *activep = 1;
                break;
            }
        /* Avoid garbage check below, as strdelim is done. */
        return 0;

    case oEscapeChar:
        intptr = &options->escape_char;
        arg = strdelim(&s);
        if (!arg || *arg == '\0') {
            fatal("%.200s line %d: Missing argument.", filename, linenum);
            return -1;
        }
        if (arg[0] == '^' && arg[2] == 0 &&
             (u_char) arg[1] >= 64 && (u_char) arg[1] < 128)
            value = (u_char) arg[1] & 31;
        else if (strlen(arg) == 1)
            value = (u_char) arg[0];
        else if (strcmp(arg, "none") == 0)
            value = -2;
        else {
            fatal("%.200s line %d: Bad escape character.",
                   filename, linenum);
            value = 0;  /* Avoid compiler warning. */
            return -1;
        }
        if (*activep && *intptr == -1)
            *intptr = value;
        break;

    case oEnableSSHKeysign:
        intptr = &options->enable_ssh_keysign;
        goto parse_flag;

    case oDeprecated:
        debug1("%s line %d: Deprecated option \"%s\"",
               filename, linenum, keyword);
        return 0;

    default:
        fatal("process_config_line: Unimplemented opcode %d", opcode);
        return -1;
    }

    /* Check that there is no garbage at end of line. */
    if ((arg = strdelim(&s)) != NULL && *arg != '\0') {
        fatal("%.200s line %d: garbage at end of line; \"%.200s\".",
               filename, linenum, arg);
        return -1;
    }
    return 0;
}


/*
 * Reads the config file and modifies the options accordingly.  Options
 * should already be initialized before this call.  This never returns if
 * there is an error.  If the file does not exist, this returns immediately.
 */

int
read_config_file(const char *filename, const char *host, Options *options)
{
    FILE *f;
    char line[1024];
    int active, linenum;
    int bad_options = 0;

    /* Open the file. */
    f = fopen(filename, "r");
    if (!f)
        return 0;

    debug1("Reading configuration data from %s", filename);

    /*
     * Mark that we are now processing the options.  This flag is turned
     * on/off by Host specifications.
     */
    active = 1;
    linenum = 0;
    while (fgets(line, sizeof(line), f)) {
        /* Update line number counter. */
        linenum++;
        if (process_config_line(options, host, line, filename, linenum, &active) != 0)
            bad_options++;
    }
    fclose(f);
    if (bad_options > 0) {
        fatal("%s: terminating, %d bad configuration options",
               filename, bad_options);
        return -1;
    }
    return 0;
}

/*
 * Initializes options to special values that indicate that they have not yet
 * been set.  Read_config_file will only set options with this value. Options
 * are processed in the following order: command line, user config file,
 * system config file.  Last, fill_default_options is called.
 */

void
initialize_options(Options * options)
{
    int i;

    memset(options,0,sizeof(Options));
    for ( i=0; i < SSH_MAX_IDENTITY_FILES ; i++ ) {
        options->identity_files[i] = NULL;
        options->identity_keys[i] = NULL;
    }
    for ( i=0 ; i < SSH_MAX_FORWARDS_PER_DIRECTION ; i++ ) {
        options->local_forwards[i].port = 0;
        options->local_forwards[i].host = NULL;
        options->local_forwards[i].host_port = 0;
        options->remote_forwards[i].port = 0;
        options->remote_forwards[i].host = NULL;
        options->remote_forwards[i].host_port = 0;
    }
    options->forward_agent = -1;
    options->forward_x11 = -1;
    options->xauth_location = NULL;
    options->gateway_ports = -1;
    options->use_privileged_port = -1;
    options->rhosts_authentication = -1;
    options->rsa_authentication = -1;
    options->pubkey_authentication = -1;
    options->challenge_response_authentication = -1;
#if defined(KRB4)
    options->krb4_authentication = -1;
#endif
#if defined(KRB5)
    options->krb5_authentication = -1;
#endif
#ifdef GSSAPI
    options->gss_authentication = -1;
    options->gss_deleg_creds = -1;
#ifdef GSI
    options->gss_globus_deleg_limited_proxy = -1;
#endif /* GSI */
#endif /* GSSAPI */
#if defined(AFS) || defined(KRB5)
    options->kerberos_tgt_passing = -1;
#endif
#ifdef AFS
    options->afs_token_passing = -1;
#endif
#ifdef SRP
    options->srp_authentication = -1;
#endif
    options->password_authentication = -1;
    options->kbd_interactive_authentication = -1;
    options->kbd_interactive_devices = NULL;
    options->rhosts_rsa_authentication = -1;
    options->hostbased_authentication = -1;
    options->batch_mode = -1;
    options->check_host_ip = -1;
    options->strict_host_key_checking = -1;
    options->compression = -1;
    options->keepalives = -1;
    options->heartbeat_interval = -1;
    options->compression_level = -1;
    options->port = -1;
    options->connection_attempts = -1;
    options->number_of_password_prompts = -1;
    options->cipher = -1;
    options->ciphers = NULL;
    options->macs = NULL;
    options->hostkeyalgorithms = NULL;
    options->protocol = SSH_PROTO_UNKNOWN;
    options->num_identity_files = 0;
    options->hostname = NULL;
    options->host_key_alias = NULL;
    options->proxy_command = NULL;
    options->user = NULL;
    options->escape_char = -1;
    options->system_hostfile = NULL;
    options->user_hostfile = NULL;
    options->system_hostfile2 = NULL;
    options->user_hostfile2 = NULL;
    options->num_local_forwards = 0;
    options->num_remote_forwards = 0;
    options->clear_forwardings = -1;
    options->log_level = SYSLOG_LEVEL_NOT_SET;
    options->preferred_authentications = NULL;
    options->bind_address = NULL;
    options->smartcard_device = NULL;
    options->enable_ssh_keysign = -1;
    options->no_host_authentication_for_localhost = - 1;
}       

/*
 * Called after processing other sources of option data, this fills those
 * options for which no value has been specified with their default values.
 */

void
fill_default_options(Options * options)
{
    int len;

    if ( !options )
        return;

    debug(F101,"fill_default_options","",1);
    if (options->forward_agent == -1)
        options->forward_agent = 0;
    if (options->forward_x11 == -1)
        options->forward_x11 = 0;
#ifdef _XAUTH_PATH
    debug(F101,"fill_default_options","",2);
    if (options->xauth_location == NULL)
        makestr(&options->xauth_location,_XAUTH_PATH);
#endif /* XAUTH_PATH */
    debug(F101,"fill_default_options","",3);
    if (options->gateway_ports == -1)
        options->gateway_ports = 0;
    if (options->use_privileged_port == -1)
        options->use_privileged_port = 0;
    if (options->rhosts_authentication == -1)
        options->rhosts_authentication = 0;
    if (options->rsa_authentication == -1)
        options->rsa_authentication = 1;
    if (options->pubkey_authentication == -1)
        options->pubkey_authentication = 1;
    if (options->challenge_response_authentication == -1)
        options->challenge_response_authentication = 0;
#if defined(KRB4)
    if (options->krb4_authentication == -1)
        options->krb4_authentication = 1;
#endif /* KRB4 / KRB5 */
#if defined(KRB5)
    if (options->krb5_authentication == -1)
        options->krb5_authentication = 1;
#endif /* KRB4 / KRB5 */
#ifdef GSSAPI
    if (options->gss_authentication == -1)
        options->gss_authentication = 1;
    if (options->gss_deleg_creds == -1)
        options->gss_deleg_creds = 1;
#ifdef GSI
    if (options->gss_globus_deleg_limited_proxy == -1)
        options->gss_globus_deleg_limited_proxy = 0;
#endif /* GSI */
#endif /* GSSAPI */
#if defined(AFS) || defined(KRB5)
#ifdef KRB5
    if (options->kerberos_tgt_passing == -1)
    {
#ifdef HEIMDAL
        krb5_context context;

        if (krb5_init_context(&context))
            options->kerberos_tgt_passing = 0;
        else {
            options->kerberos_tgt_passing = krb5_config_get_bool(context,
                NULL, "libdefaults", "forward", NULL);
            krb5_free_context(context);
        }
#else /* HEIMDAL */
        options->kerberos_tgt_passing = 1;
#endif /* HEIMDAL */
    }
#else /* KRB5 */
    if (options->kerberos_tgt_passing == -1)
        options->kerberos_tgt_passing = 1;
#endif /* KRB5 */
#endif /* AFS/KRB5 */
#ifdef AFS
    if (options->afs_token_passing == -1)
        options->afs_token_passing = 1;
#endif /* AFS */
#ifdef SRP
    if (options->srp_authentication == -1)
        options->srp_authentication = 1;
#endif
    if (options->password_authentication == -1)
        options->password_authentication = 1;
    if (options->kbd_interactive_authentication == -1)
        options->kbd_interactive_authentication = 1;
    if (options->rhosts_rsa_authentication == -1)
        options->rhosts_rsa_authentication = 0;
    if (options->hostbased_authentication == -1)
        options->hostbased_authentication = 0;
    if (options->batch_mode == -1)
        options->batch_mode = 0;
    if (options->check_host_ip == -1)
        options->check_host_ip = 1;
    if (options->strict_host_key_checking == -1)
        options->strict_host_key_checking = 2;  /* 2 is default */
    if (options->compression == -1)
        options->compression = 0;
    if (options->keepalives == -1)
        options->keepalives = 1;
    if (options->heartbeat_interval == -1)
        options->heartbeat_interval = 0;
    if (options->compression_level == -1)
        options->compression_level = 6;
    if (options->port == -1)
        options->port = 0;      /* Filled in ssh_connect. */
    if (options->connection_attempts == -1)
        options->connection_attempts = 1;
    if (options->number_of_password_prompts == -1)
        options->number_of_password_prompts = 3;
    /* Selected in ssh_login(). */
    if (options->cipher == -1)
        options->cipher = SSH_CIPHER_NOT_SET;
    /* options->ciphers, default set in myproposals.h */
    /* options->macs, default set in myproposals.h */
    /* options->hostkeyalgorithms, default set in myproposals.h */
    if (options->protocol == SSH_PROTO_UNKNOWN)
        options->protocol = SSH_PROTO_1|SSH_PROTO_2;
    if (options->num_identity_files == 0) {
        debug(F101,"fill_default_options","",4);
        if (options->protocol & SSH_PROTO_1) { 
            makestr(&options->identity_files[options->num_identity_files++],
                     (char *)_PATH_SSH_CLIENT_IDENTITY);
        }       
        debug(F101,"fill_default_options","",5);
        if (options->protocol & SSH_PROTO_2) {
            makestr(&options->identity_files[options->num_identity_files++],
                     (char *)_PATH_SSH_CLIENT_ID_RSA);
            makestr(&options->identity_files[options->num_identity_files++],
                     (char *)_PATH_SSH_CLIENT_ID_DSA);
        }       
        debug(F101,"fill_default_options","",6);
    }       
    if (options->escape_char == -1)
        options->escape_char = '~';
    if (options->system_hostfile == NULL)
        makestr(&options->system_hostfile,(char *)_PATH_SSH_SYSTEM_HOSTFILE);
    debug(F101,"fill_default_options","",7);
    if (options->user_hostfile == NULL)
        makestr(&options->user_hostfile,(char *)_PATH_SSH_USER_HOSTFILE);
    debug(F101,"fill_default_options","",8);
    if (options->system_hostfile2 == NULL)
        makestr(&options->system_hostfile2,(char *)_PATH_SSH_SYSTEM_HOSTFILE2);
    debug(F101,"fill_default_options","",9);
    if (options->user_hostfile2 == NULL)
        makestr(&options->user_hostfile2,(char *)_PATH_SSH_USER_HOSTFILE2);
    debug(F101,"fill_default_options","",10);
    if (options->log_level == SYSLOG_LEVEL_NOT_SET)
        options->log_level = SYSLOG_LEVEL_INFO;
    if (options->clear_forwardings == 1)
        clear_forwardings(options);
    if (options->no_host_authentication_for_localhost == - 1)
        options->no_host_authentication_for_localhost = 0;
    if (options->enable_ssh_keysign == -1)
        options->enable_ssh_keysign = 0;
    /* options->proxy_command should not be set by default */
    /* options->user will be set in the main program if appropriate */
    /* options->hostname will be set in the main program if appropriate */
    /* options->host_key_alias should not be set by default */
    /* options->preferred_authentications will be set in ssh */
}

/* Restores stdin to blocking mode. */

static void
leave_non_blocking(void)
{
#ifndef OS2
    if (in_non_blocking_mode) {
        (void) fcntl(fileno(stdin), F_SETFL, 0);
        in_non_blocking_mode = 0;
        fatal_remove_cleanup((void (*) (void *)) leave_non_blocking, NULL);
    }
#endif /* !OS2 */
}

/* Puts stdin terminal in non-blocking mode. */

static void
enter_non_blocking(void)
{
#ifndef OS2
    in_non_blocking_mode = 1;
    (void) fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    fatal_add_cleanup((void (*) (void *)) leave_non_blocking, NULL);
#endif /* !OS2 */
}

/*
 * Signal handler for the window change signal (SIGWINCH).  This just sets a
 * flag indicating that the window has changed.
 */
#ifndef OS2
static void
window_change_handler(int sig)
{
    received_window_change_signal = 1;
    signal(SIGWINCH, window_change_handler);
}
#else
static void
ssh_window_change(void)
{
    if (sent_initial_ws)
        received_window_change_signal = 1;
}

int
ssh_snaws(void)
{
    extern int tt_status[];
    if (ssh_height != (VscrnGetHeight(VTERM) - (tt_status[VTERM]?1:0)) ||
        ssh_width != VscrnGetWidth(VTERM))
        ssh_window_change();
    return 0;
}
#endif /* !OS2 */

/*
 * Signal handler for signals that cause the program to terminate.  These
 * signals must be trapped to restore terminal modes.
 */

static void
signal_handler(int sig)
{
    received_signal = sig;
    quit_pending = 1;
}

/*
 * This is called when the interactive is entered.  This checks if there is
 * an EOF coming on stdin.  We must check this explicitly, as select() does
 * not appear to wake up when redirecting from /dev/null.
 */

static void
client_check_initial_eof_on_stdin(void)
{
    int len;
    char buf[1];

    /*
     * If standard input is to be "redirected from /dev/null", we simply
     * mark that we have seen an EOF and send an EOF message to the
     * server. Otherwise, we try to read a single character; it appears
     * that for some files, such /dev/null, select() never wakes up for
     * read for this descriptor, which means that we never get EOF.  This
     * way we will get the EOF if stdin comes from /dev/null or similar.
     */
    if (stdin_null_flag) {
        /* Fake EOF on stdin. */
        debug1("Sending eof.");
        stdin_eof = 1;
        packet_start(SSH_CMSG_EOF);
        packet_send();
    } else {
        enter_non_blocking();

        /* Check for immediate EOF on stdin. */
#ifdef OS2
        len = recv(sock_stdin, buf, 1, 0);
#else
        len = read(fileno(stdin), buf, 1);
#endif /* OS2 */
        if (len == 0) {
            /* EOF.  Record that we have seen it and send EOF to server. */
            debug1("Sending eof.");
            stdin_eof = 1;
            packet_start(SSH_CMSG_EOF);
            packet_send();
        } else if (len > 0) {
            /*
             * Got data.  We must store the data in the buffer,
             * and also process it as an escape character if
             * appropriate.
             */
            if ((u_char) buf[0] == escape_char)
                escape_pending = 1;
            else
                buffer_append(&stdin_buffer, buf, 1);
        }
        leave_non_blocking();
    }
}


/*
 * Make packets from buffered stdin data, and buffer them for sending to the
 * connection.
 */

static int
client_make_packets_from_stdin_data(void)
{
    u_int len;

    /* Send buffered stdin data to the server. */
    while (buffer_len(&stdin_buffer) > 0 &&
            packet_not_very_much_data_to_write()) {
        len = buffer_len(&stdin_buffer);
        /* Keep the packets at reasonable size. */
        if (len > packet_get_maxsize())
            len = packet_get_maxsize();
        packet_start(SSH_CMSG_STDIN_DATA);
        packet_put_string(buffer_ptr(&stdin_buffer), len);
        packet_send();
        if (buffer_consume(&stdin_buffer, len) < 0)
            return(-1);
        stdin_bytes += len;
        /* If we have a pending EOF, send it now. */
        if (stdin_eof && buffer_len(&stdin_buffer) == 0) {
            packet_start(SSH_CMSG_EOF);
            packet_send();
        }
    }
    return(0);
}

/*
 * Checks if the client window has changed, and sends a packet about it to
 * the server if so.  The actual change is detected elsewhere (by a software
 * interrupt on Unix); this just checks the flag and sends a message if
 * appropriate.
 */

static void
client_check_window_change(void)
{
    struct winsize ws;

    if (! received_window_change_signal)
        return;

    /** XXX race */
    received_window_change_signal = 0;

#ifdef OS2
    if (_getwinsize( &ws ) == -1)
        memset(&ws,0,sizeof(ws));
#else /* OS2 */
    if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) < 0)
        return;
#endif /* OS2 */

    debug2("client_check_window_change: changed");

    received_window_change_signal = 0;
    ssh_width = ws.ws_col;
    ssh_height = ws.ws_row;
    if (compat20) {
        channel_request_start(session_ident, "window-change", 0);
        packet_put_int(ws.ws_col);
        packet_put_int(ws.ws_row);
        packet_put_int(ws.ws_xpixel);
        packet_put_int(ws.ws_ypixel);
        packet_send();
    } else {
        packet_start(SSH_CMSG_WINDOW_SIZE);
        packet_put_int(ws.ws_row);
        packet_put_int(ws.ws_col);
        packet_put_int(ws.ws_xpixel);
        packet_put_int(ws.ws_ypixel);
        packet_send();
    }
}

/*
 * Waits until the client can do something (some data becomes available on
 * one of the file descriptors).
 */

static time_t idle_time_last = 0;

static void
client_wait_until_can_do_something(fd_set **readsetp, fd_set **writesetp,
    int *maxfdp, int *nallocp, int rekeying)
{
    int ret;
    struct timeval tv;

    /* Add any selections by the channel mechanism. */
    channel_prepare_select(readsetp, writesetp, maxfdp, nallocp, rekeying);

    if (!compat20) {
        /* Read from the connection, unless our buffers are full. */
        if (buffer_len(&stdout_buffer) < buffer_high &&
             buffer_len(&stderr_buffer) < buffer_high &&
             channel_not_very_much_buffered_data())
            FD_SET(connection_in, *readsetp);
        /*
         * Read from stdin, unless we have seen EOF or have very much
         * buffered data to send to the server.
         */
        if (!stdin_eof && packet_not_very_much_data_to_write()) {
#ifdef OS2
            FD_SET(sock_stdin, *readsetp);
#else
            FD_SET(fileno(stdin), *readsetp);
#endif /* OS2 */
        }
        /* Select stdout/stderr if have data in buffer. */
        if (buffer_len(&stdout_buffer) > 0) {
#ifdef OS2
            return;
#else
            FD_SET(fileno(stdout), *writesetp);
#endif /* OS2 */
        }
        if (buffer_len(&stderr_buffer) > 0) {
#ifdef OS2
            return;
#else
            FD_SET(fileno(stderr), *writesetp);
#endif /* OS2 */
        }
    } else {
        /* channel_prepare_select could have closed the last channel */
        if (session_closed && (channel_still_open() <= 0) &&
            !packet_have_data_to_write()) {
                /* clear mask since we did not call select() */
#ifdef OS2
            FD_ZERO(*readsetp);
            FD_ZERO(*writesetp);
#else
            memset(*readsetp, 0, *nallocp);
            memset(*writesetp, 0, *nallocp);
#endif
            return;
        } else {
            FD_SET(connection_in, *readsetp);
        }
    }

    /* Select server connection if have data to write to the server. */
    if (packet_have_data_to_write())
        FD_SET(connection_out, *writesetp);

    /*
     * Wait for something to happen.  This will suspend the process until
     * some selected descriptor can be read, written, or has some other
     * event pending. Note: if you want to implement SSH_MSG_IGNORE
     * messages to fool traffic analysis, this might be the place to do
     * it: just have a random timeout for the select, and send a random
     * SSH_MSG_IGNORE packet when the timeout expires.
     */

    tv.tv_sec = 0;
    tv.tv_usec = 500 * 1000;            /* time slot is 0.5 sec */

    ret = select((*maxfdp)+1, *readsetp, *writesetp, NULL, NULL);
    if (ret < 0) {
        /*
         * We have to clear the select masks, because we return.
         * We have to return, because the mainloop checks for the flags
         * set by the signal handlers.
         */
#ifdef OS2
        FD_ZERO(*readsetp);
        FD_ZERO(*writesetp);
#else
        memset(*readsetp, 0, *nallocp);
        memset(*writesetp, 0, *nallocp);
#endif /* OS2 */

        if (errno == EINTR || quit_pending)
            return;
        /* Note: we might still have data in the buffers. */
        debug1("select: %s", sock_strerror(sock_lasterror()));
        quit_pending = 1;
    }

    /* If the output channel has been silent for more than a specified
     * time, send a keepalive packet (heartbeat) to the server.
     * Keepalive packet is useful for keeping the connection over
     * IP masquerade / NAT boxes, firewalls, etc.
     * Some servers equipped with a watchdog timer require keepalive
     * packets (heartbeats) to detect link down.
     *
     * Note: Although the interval between keepalive packets is not
     * very precise, it's okay.
     *
     * Note: Some old servers may crash when they receive SSH_MSG_IGNORE.
     * Those who want to connect to such a server should turn this
     * function off by the option setting (e.g. Heartbeat 0).
     */
    if (options.heartbeat_interval > 0) {
        if (FD_ISSET(connection_out,*writesetp)) {
                /* Update the time of last data transmission. */
                idle_time_last = time(NULL);
        }
        else if (time(NULL) - idle_time_last >= (int)options.heartbeat_interval) {
             if (compat20) {
                 packet_start(SSH2_MSG_IGNORE);
                }       
             else {
                 packet_start(SSH_MSG_IGNORE);
             }
             packet_put_string("", 0);
             packet_send();
             /* fputs("*",stderr); */
         }
    }
}

#ifndef OS2
static void
client_suspend_self(Buffer *bin, Buffer *bout, Buffer *berr)
{
#ifndef OS2
    struct winsize oldws, newws;
#endif

    /* Flush stdout and stderr buffers. */
    if (buffer_len(bout) > 0)
        atomicio(write, fileno(stdout), buffer_ptr(bout), buffer_len(bout));
    if (buffer_len(berr) > 0)
        atomicio(write, fileno(stderr), buffer_ptr(berr), buffer_len(berr));

    leave_raw_mode();

    /*
     * Free (and clear) the buffer to reduce the amount of data that gets
     * written to swap.
     */
    buffer_free(bin);
    buffer_free(bout);
    buffer_free(berr);

#ifndef OS2
    /* Save old window size. */
    ioctl(fileno(stdin), TIOCGWINSZ, &oldws);

    /* Send the suspend signal to the program itself. */
    kill(getpid(), SIGTSTP);

    /* Check if the window size has changed. */
    if (ioctl(fileno(stdin), TIOCGWINSZ, &newws) >= 0 &&
         (oldws.ws_row != newws.ws_row ||
           oldws.ws_col != newws.ws_col ||
           oldws.ws_xpixel != newws.ws_xpixel ||
           oldws.ws_ypixel != newws.ws_ypixel))
        received_window_change_signal = 1;
#endif /* !OS2 */

    /* OK, we have been continued by the user. Reinitialize buffers. */
    buffer_init(bin);
    buffer_init(bout);
    buffer_init(berr);

    enter_raw_mode();
}
#endif /* OS2 */

static void
client_process_net_input(fd_set * readset)
{
    int len;
    char buf[8192];

    /*
     * Read input from the server, and add any such data to the buffer of
     * the packet subsystem.
     */
    if (FD_ISSET(connection_in, readset)) {
        /* Read as much as possible. */
#ifdef OS2
        len = recv(connection_in, buf, sizeof(buf),0);
#else
        len = read(connection_in, buf, sizeof(buf));
#endif /* OS2 */
        if (len == 0) {
            /* Received EOF.  The remote host has closed the connection. */
            xfprintf(stderr,"Connection to %.300s closed by remote host.\r\n",
                      host);
            quit_pending = 1;
            return;
        }
        /*
         * There is a kernel bug on Solaris that causes select to
         * sometimes wake up even though there is no data available.
         */
#ifdef OS2
        if (len < 0 && (sock_lasterror() == EAGAIN))
            len = 0;
#else
        if (len < 0 && (errno == EAGAIN || errno == EINTR))
            len = 0;
#endif /* OS2 */

        if (len < 0) {
            /* An error has encountered.  Perhaps there is a network problem. */
#ifdef OS2
            debug1("Read from remote host %.300s: %.100s",
                      host, sock_strerror(sock_lasterror()));
#else
            snprintf(buf, sizeof buf, "Read from remote host %.300s: %.100s\r\n",
                      host, strerror(errno));
            buffer_append(&stderr_buffer, buf, strlen(buf));
#endif /* OS2 */
            quit_pending = 1;
            return;
        }
        packet_process_incoming(buf, len);
    }
}

#ifdef UNIX
static void
process_cmdline(void)
{
	void (*handler)(int);
	char *s, *cmd;
	u_short fwd_port, fwd_host_port;
	char buf[1024], sfwd_port[6], sfwd_host_port[6];
	int local = 0;

	leave_raw_mode();
	handler = signal(SIGINT, SIG_IGN);
	cmd = s = read_passphrase("\r\nssh> ", RP_ECHO);
	if (s == NULL)
		goto out;
	while (*s && isspace(*s))
		s++;
	if (*s == 0)
		goto out;
	if (strlen(s) < 2 || s[0] != '-' || !(s[1] == 'L' || s[1] == 'R')) {
		log("Invalid command.");
		goto out;
	}
	if (s[1] == 'L')
		local = 1;
	if (!local && !compat20) {
		log("Not supported for SSH protocol version 1.");
		goto out;
	}
	s += 2;
	while (*s && isspace(*s))
		s++;

	if (sscanf(s, "%5[0-9]:%255[^:]:%5[0-9]",
	    sfwd_port, buf, sfwd_host_port) != 3 &&
	    sscanf(s, "%5[0-9]/%255[^/]/%5[0-9]",
	    sfwd_port, buf, sfwd_host_port) != 3) {
		log("Bad forwarding specification.");
		goto out;
	}
	if ((fwd_port = a2port(sfwd_port)) == 0 ||
	    (fwd_host_port = a2port(sfwd_host_port)) == 0) {
		log("Bad forwarding port(s).");
		goto out;
	}
	if (local) {
		if (channel_setup_local_fwd_listener(fwd_port, buf,
		    fwd_host_port, options.gateway_ports) < 0) {
			log("Port forwarding failed.");
			goto out;
		}
	} else
		channel_request_remote_forwarding(fwd_port, buf,
		    fwd_host_port);
	log("Forwarding port.");
out:
	signal(SIGINT, handler);
	enter_raw_mode();
	if (cmd)
		free(cmd);
}
#endif /* UNIX */

/* process the characters one by one */
static int
process_escapes(Buffer *bin, Buffer *bout, Buffer *berr, char *buf, int len)
{
    char string[1024];
    pid_t pid;
    int bytes = 0;
    u_int i;
    u_char ch;
    char *s;

    for (i = 0; i < len; i++) {
        /* Get one character at a time. */
        ch = buf[i];

        if (escape_pending) {
            /* We have previously seen an escape character. */
            /* Clear the flag now. */
            escape_pending = 0;

            /* Process the escaped character. */
            switch (ch) {
            case '.':
                /* Terminate the connection. */
                snprintf(string, sizeof string, "%c.\r\n", escape_char);
                buffer_append(berr, string, strlen(string));
                quit_pending = 1;
                return -1;

#ifndef OS2
            case 'Z' - 64:
                /* Suspend the program. */
                /* Print a message to that effect to the user. */
                snprintf(string, sizeof string, "%c^Z [suspend ssh]\r\n", escape_char);
                buffer_append(berr, string, strlen(string));

                /* Restore terminal modes and suspend. */
                client_suspend_self(bin, bout, berr);

                /* We have been continued. */
                continue;
#endif /* OS2 */
            case 'R':
                if (compat20) {
                    if (datafellows & SSH_BUG_NOREKEY)
                        log("Server does not support re-keying");
                    else
                        need_rekeying = 1;
                }
                continue;

            case '&':
                /*
                * Detach the program (continue to serve connections,
                * but put in background and no more new connections).
                */
                /* Restore tty modes. */
                leave_raw_mode();

                /* Stop listening for new connections. */
                channel_stop_listening();

                snprintf(string, sizeof string,
                          "%c& [backgrounded]\n", escape_char);
                buffer_append(berr, string, strlen(string));

#ifndef OS2
                /* Fork into background. */
                pid = fork();
                if (pid < 0) {
                    error("fork: %.100s", strerror(errno));
                    continue;
                }
                if (pid != 0) { /* This is the parent. */
                    /* The parent just exits. */
                    exit(0);
                }
                /* The child continues serving connections. */
#endif /* OS2 */
                if (compat20) {
                    buffer_append(bin, "\004", 1);
                    /* fake EOF on stdin */
                    return -1;
                } else if (!stdin_eof) {
                    /*
                    * Sending SSH_CMSG_EOF alone does not always appear
                    * to be enough.  So we try to send an EOF character
                    * first.
                    */
                    packet_start(SSH_CMSG_STDIN_DATA);
                    packet_put_string("\004", 1);
                    packet_send();
                    /* Close stdin. */
                    stdin_eof = 1;
                    if (buffer_len(bin) == 0) {
                        packet_start(SSH_CMSG_EOF);
                        packet_send();
                    }
                }
                continue;

            case '?':
                snprintf(string, sizeof string,
                          "%c?\r\n"
                          "Supported escape sequences:\r\n"
                          "%c.  - terminate connection\r\n"
#ifdef UNIX
                          "%cC - open a command line\r\n"
#endif /* UNIX */
                          "%cR - Request rekey (SSH protocol 2 only)\r\n"
                          "%c^Z - suspend ssh\r\n"
                          "%c#  - list forwarded connections\r\n"
                          "%c&  - background ssh (when waiting for connections to terminate)\r\n"
                          "%c?  - this message\r\n"
                          "%c%c  - send the escape character by typing it twice\r\n"
                          "(Note that escapes are only recognized immediately after newline.)\r\n",
                          escape_char,escape_char,escape_char,escape_char,escape_char,
                          escape_char,escape_char,escape_char,escape_char,escape_char);
                buffer_append(berr, string, strlen(string));
                continue;

            case '#':
                snprintf(string, sizeof string, "%c#\r\n", escape_char);
                buffer_append(berr, string, strlen(string));
                s = channel_open_message();
                buffer_append(berr, s, strlen(s));
                free(s);
                s = NULL;
                continue;

#ifdef UNIX
            case 'C':
                process_cmdline();
                continue;
#endif /* UNIX */

            default:
                if (ch != escape_char) {
                    buffer_put_char(bin, escape_char);
                    bytes++;
                }
                /* Escaped characters fall through here */
                break;
            }
        } else {
            /*
            * The previous character was not an escape char. Check if this
            * is an escape.
            */
            if (last_was_cr && ch == escape_char) {
                /* It is. Set the flag and continue to next character. */
                escape_pending = 1;
                continue;
            }
        }

        /*
        * Normal character.  Record whether it was a newline,
        * and append it to the buffer.
        */
        last_was_cr = (ch == '\r' || ch == '\n');
        buffer_put_char(bin, ch);
        bytes++;
    }
    return bytes;
}

static void
client_process_input(fd_set * readset)
{
    int len;
    char buf[8192];
    int fd;

#ifdef OS2
    fd = sock_stdin;
#else
    fd = fileno(stdin);
#endif /* OS2 */

    /* Read input from stdin. */
    if (FD_ISSET(fd, readset)) {
        /* Read as much as possible. */
#ifdef OS2
        len = recv(fd, buf, sizeof(buf),0);
#else
        len = read(fd, buf, sizeof(buf));
#endif /* OS2 */
        if (len < 0 &&
#ifdef OS2
             (sock_lasterror() == EAGAIN)
#else
             (errno == EAGAIN || errno == EINTR)
#endif /* OS2 */
             )
            return;             /* we'll try again later */
        if (len <= 0) {
            /*
            * Received EOF or error.  They are treated
            * similarly, except that an error message is printed
            * if it was an error condition.
            */
            if (len < 0) {
#ifdef OS2
                snprintf(buf, sizeof buf, "read: %.100s\r\n",
                          sock_strerror(sock_lasterror()));
#else
                snprintf(buf, sizeof buf, "read: %.100s\r\n", strerror(errno));
#endif /* OS2 */
                buffer_append(&stderr_buffer, buf, strlen(buf));
            }
            /* Mark that we have seen EOF. */
            stdin_eof = 1;
            /*
            * Send an EOF message to the server unless there is
            * data in the buffer.  If there is data in the
            * buffer, no message will be sent now.  Code
            * elsewhere will send the EOF when the buffer
            * becomes empty if stdin_eof is set.
            */
            if (buffer_len(&stdin_buffer) == 0) {
                packet_start(SSH_CMSG_EOF);
                packet_send();
            }
        } else if (escape_char == SSH_ESCAPECHAR_NONE) {
            /*
            * Normal successful read, and no escape character.
            * Just append the data to buffer.
            */
            buffer_append(&stdin_buffer, buf, len);
        } else {
            /*
            * Normal, successful read.  But we have an escape character
            * and have to process the characters one by one.
            */
            if (process_escapes(&stdin_buffer, &stdout_buffer,
                                 &stderr_buffer, buf, len) == -1)
                return;
        }
    }
}

static int
client_process_output(fd_set * writeset)
{
    int len;
    char buf[100];
    int set;

#ifdef OS2
    set = buffer_len(&stdout_buffer) > 0;
#else
    set = FD_ISSET(fileno(stdout), writeset);
#endif /* OS2 */

    /* Write buffered output to stdout. */
    if (set) {
#ifdef OS2
        char * p = buffer_ptr(&stdout_buffer);

        len = buffer_len(&stdout_buffer);
        hexdump("SSH client_process_output stdout",p,len);
        send(sock_stdout,p,len,0);
#else
        /* Write as much data as possible. */
        len = write(fileno(stdout), buffer_ptr(&stdout_buffer),
                     buffer_len(&stdout_buffer));
        if (len <= 0) {
            if (errno == EINTR || errno == EAGAIN)
                len = 0;
            else {
                /*
                * An error or EOF was encountered.  Put an
                * error message to stderr buffer.
                */
                snprintf(buf, sizeof buf, "write stdout: %.50s\r\n", strerror(errno));
                buffer_append(&stderr_buffer, buf, strlen(buf));
                quit_pending = 1;
                return(0);
            }
        }
#endif
        /* Consume printed data from the buffer. */
        if (buffer_consume(&stdout_buffer, len) < 0)
            return(-1);
        stdout_bytes += len;
    }
#ifdef OS2
    set = buffer_len(&stderr_buffer) > 0;
#else
    set = FD_ISSET(fileno(stderr), writeset);
#endif /* OS2 */
    /* Write buffered output to stderr. */
    if (set) {
        /* Write as much data as possible. */
#ifdef OS2
        char * p = buffer_ptr(&stderr_buffer);

        len = buffer_len(&stderr_buffer);
        hexdump("SSH client_process_output stderr",p,len);
        send(sock_stderr,p,len,0);
#else
        len = write(fileno(stderr), buffer_ptr(&stderr_buffer),
                     buffer_len(&stderr_buffer));
        if (len <= 0) {
            if (errno == EINTR || errno == EAGAIN)
                len = 0;
            else {
                /* EOF or error, but can't even print error message. */
                quit_pending = 1;
                return(0);
            }
        }
#endif
        /* Consume printed characters from the buffer. */
        if (buffer_consume(&stderr_buffer, len) < 0)
            return(-1);
        stderr_bytes += len;
    }
    return(0);
}

/*
 * Get packets from the connection input buffer, and process them as long as
 * there are packets available.
 *
 * Any unknown packets received during the actual
 * session cause the session to terminate.  This is
 * intended to make debugging easier since no
 * confirmations are sent.  Any compatible protocol
 * extensions must be negotiated during the
 * preparatory phase.
 */

static void
client_process_buffered_input_packets(void)
{
    dispatch_run(DISPATCH_NONBLOCK, &quit_pending, compat20 ? xxx_kex : NULL);
}

/* scan buf[] for '~' before sending data to the peer */

static int
simple_escape_filter(Channel *c, char *buf, int len)
{
    /* XXX we assume c->extended is writeable */
    return process_escapes(&c->input, &c->output, &c->extended, buf, len);
}

static void
client_channel_closed(int id, void *arg)
{
    if (id != session_ident)
        error("client_channel_closed: id %d != session_ident %d",
               id, session_ident);
    channel_cancel_cleanup(id);
    session_closed = 1;
    if (in_raw_mode())
        leave_raw_mode();
}

/*
 * Implements the interactive session with the server.  This is called after
 * the user has been authenticated, and a command has been started on the
 * remote host.  If escape_char != SSH_ESCAPECHAR_NONE, it is the character used as an escape
 * character for terminating or suspending the session.
 */

int
client_loop(int have_pty, int escape_char_arg, int ssh2_chan_id)
{
    fd_set *readset = NULL, *writeset = NULL;
    time_t start_time, total_time, rekey_time;
    int max_fd = 0, max_fd2 = 0, len, rekeying = 0, nalloc = 0;
    char buf[100];

    debug1("Entering interactive session.");

#ifndef OS2
    /* REALLY send debug messages to the application */
    fflush(stderr);
#endif /* OS2 */

    start_time = rekey_time = idle_time_last = time(NULL);

    /* Initialize variables. */
    escape_pending = 0;
    last_was_cr = 1;
    exit_status = -1;
    stdin_eof = 0;
    buffer_high = 64 * 1024;
    connection_in = packet_get_connection_in();
    connection_out = packet_get_connection_out();
    max_fd = MAX(connection_in, connection_out);

    if (!compat20) {
#ifndef OS2
        /* enable nonblocking unless tty */
        if (!isatty(fileno(stdin)))
            set_nonblock(fileno(stdin));
        if (!isatty(fileno(stdout)))
            set_nonblock(fileno(stdout));
        if (!isatty(fileno(stderr)))
            set_nonblock(fileno(stderr));
        max_fd = MAX(max_fd, fileno(stdin));
        max_fd = MAX(max_fd, fileno(stdout));
        max_fd = MAX(max_fd, fileno(stderr));
#endif /* OS2 */
        stdin_bytes = 0;
        stdout_bytes = 0;
        stderr_bytes = 0;
    }
    quit_pending = 0;
    escape_char = escape_char_arg;

    /* Initialize buffers. */
    buffer_init(&stdin_buffer);
    buffer_init(&stdout_buffer);
    buffer_init(&stderr_buffer);

    client_init_dispatch();

    /* Set signal handlers to restore non-blocking mode.  */
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, signal_handler);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
        signal(SIGTERM, signal_handler);
#ifndef OS2
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
        signal(SIGQUIT, signal_handler);
    if (have_pty)
        signal(SIGWINCH, window_change_handler);

    if (have_pty)
        enter_raw_mode();
#endif /* !OS2 */

    if (compat20) {
        session_ident = ssh2_chan_id;
        if (escape_char != SSH_ESCAPECHAR_NONE)
            channel_register_filter(session_ident,
                                     simple_escape_filter);
        if (session_ident != -1)
            channel_register_cleanup(session_ident,
                                      client_channel_closed);
    }
#ifndef OS2
    else {
        /* Check if we should immediately send eof on stdin. */
        client_check_initial_eof_on_stdin();
    }
#endif /* OS2 */

#ifdef OS2
#ifdef NT
        SetThreadPrty(XYP_RTP,isWin95() ? 14 : 22);
#else /* NT */
        SetThreadPrty(XYP_RTP,3);
#endif /* NT */
#endif /* OS2 */

    /* Main loop of the client for the interactive session mode. */
    while (!quit_pending) {

        /* Process buffered packets sent by the server. */
        client_process_buffered_input_packets();

        if (compat20 && session_closed && (channel_still_open() <= 0)) {
            debug1("client_loop: session_closed");
#ifdef OS2
            /* Process output to stdout and stderr.   Output to the connection
               is processed elsewhere (above). */
            if ((buffer_len(&stdout_buffer) > 0) ||
                 (buffer_len(&stderr_buffer) > 0)) {
                client_process_output(writeset);
            }
#endif /* OS2 */
            break;
        }

        rekeying = (compat20 && xxx_kex != NULL && !xxx_kex->done);

        if (rekeying) {
            debug1("rekeying in progress");
            rekey_time = time(NULL);
        } else {
            /*
             * Make packets of buffered stdin data, and buffer
             * them for sending to the server.
             */
            if (!compat20)
                client_make_packets_from_stdin_data();

            /*
             * Make packets from buffered channel data, and
             * enqueue them for sending to the server.
             */
            if (packet_not_very_much_data_to_write())
                channel_output_poll();

            /*
             * Check if the window size has changed, and buffer a
             * message about it to the server if so.
             */
            client_check_window_change();

            if (quit_pending)
                break;
        }

#ifdef OS2
        /* Process output to stdout and stderr.   Output to the connection
           is processed elsewhere (above). */
        if ((buffer_len(&stdout_buffer) > 0) ||
             (buffer_len(&stderr_buffer) > 0))
            client_process_output(writeset);
#endif /* OS2 */

        /*
         * Wait until we have something to do (something becomes
         * available on one of the descriptors).
         */
        max_fd2 = max_fd;
        client_wait_until_can_do_something(&readset, &writeset,
                    &max_fd2, &nalloc, rekeying);

        if (quit_pending)
            break;


        /* Do channel operations unless rekeying in progress. */
        if (!rekeying) {
            channel_after_select(readset, writeset);

            if ( compat20 && ssh2_ark && !(datafellows & SSH_BUG_NOREKEY)) {
				/* If auto-rekeying, rekey every hour */
				time_t now_t = time(NULL);
                if ( now_t - rekey_time > 360 )
                    need_rekeying = 1;
            }

            if (need_rekeying) {
                debug1("user requests rekeying");
                if ( xxx_kex ) {
                    xxx_kex->done = 0;
                    kex_send_kexinit(xxx_kex);
                }
                need_rekeying = 0;
            }
        }

        /* Buffer input from the connection.  */
        client_process_net_input(readset);

        if (quit_pending)
            break;

        if (!compat20) {
            /* Buffer data from stdin */
            client_process_input(readset);
            /*
             * Process output to stdout and stderr.  Output to
             * the connection is processed elsewhere (above).
             */
#ifdef OS2
            if ((buffer_len(&stdout_buffer) > 0) ||
                 (buffer_len(&stderr_buffer) > 0))
#endif
                client_process_output(writeset);
        }

        /* Send as much buffered packet data as possible to the sender. */
        if (FD_ISSET(connection_out, writeset))
            packet_write_poll();
    }
    if (readset) {
        free(readset);
        readset = NULL;
    }
    if (writeset) {
        free(writeset);
        writeset = NULL;
    }

    /* Terminate the session. */

#ifndef OS2
    /* Stop watching for window change. */
    if (have_pty)
        signal(SIGWINCH, SIG_DFL);
#endif /* OS2 */

    /* Stop listening for connections. */
    channel_free_all();

    if (have_pty)
        leave_raw_mode();

#ifndef OS2
    /* restore blocking io */
    if (!isatty(fileno(stdin)))
        unset_nonblock(fileno(stdin));
    if (!isatty(fileno(stdout)))
        unset_nonblock(fileno(stdout));
    if (!isatty(fileno(stderr)))
        unset_nonblock(fileno(stderr));
#endif /* OS2 */

    if (received_signal) {
        if (in_non_blocking_mode)       /* XXX */
            leave_non_blocking();
        fatal("Killed by signal %d.", (int) received_signal);
        return -1;
    }

    /* Output any buffered data for stdout. */
    while (buffer_len(&stdout_buffer) > 0) {
#ifdef OS2
        char * p = buffer_ptr(&stdout_buffer);
        int i;

        len = buffer_len(&stdout_buffer);
        if (len > 0)
            send(sock_stdout,buffer_ptr(&stdout_buffer),len,0);
#else
        len = write(fileno(stdout), buffer_ptr(&stdout_buffer),
                     buffer_len(&stdout_buffer));
        if (len <= 0) {
            error("Write failed flushing stdout buffer.");
            break;
        }
#endif
        buffer_consume(&stdout_buffer, len);
        stdout_bytes += len;
    }

    /* Output any buffered data for stderr. */
    while (buffer_len(&stderr_buffer) > 0) {
#ifdef OS2
        char * p = buffer_ptr(&stderr_buffer);
        int i;

        len = buffer_len(&stderr_buffer);
        if ( len > 0 )
            send(sock_stderr,buffer_ptr(&stdout_buffer),len,0);
#else
        len = write(fileno(stderr), buffer_ptr(&stderr_buffer),
                     buffer_len(&stderr_buffer));
        if (len <= 0) {
            error("Write failed flushing stderr buffer.");
            break;
        }
#endif
        buffer_consume(&stderr_buffer, len);
        stderr_bytes += len;
    }
    
    if (!compat20) {
        /* Close the connection to the remote host. */
        packet_disconnect("Client loop terminated");

        closesocket(sock_stdin);
        closesocket(sock_stdout);
        closesocket(sock_stderr);
    }
    /* Clear and free any buffers. */
    memset(buf, 0, sizeof(buf));
    buffer_free(&stdin_buffer);
    buffer_free(&stdout_buffer);
    buffer_free(&stderr_buffer);

    if ( !compat20 ) {
        /* Report bytes transferred, and transfer rates. */
        total_time = time(NULL) - start_time;
        debug1("Bytes Transferred %d stdin",stdin_bytes);
        debug1("Bytes Transferred %d stdout",stdout_bytes);
        debug1("Bytes Transferred %d stderr",stderr_bytes);
        debug1("Total Time %d seconds",total_time);

        if (total_time > 0) {
            debug1("Stdin %f BPS",(long)stdin_bytes/total_time);
            debug1("Stdout %f BPS",(long)stdout_bytes/total_time);
            debug1("Stderr %f BPS",(long)stderr_bytes/total_time);
        }
    }

    /* Return the exit status of the program. */
    debug1("Exit status: %d", exit_status);
    quit_pending = 0;           /* no longer pending */
    return exit_status;
}

/*********/

static int
client_input_stdout_data(int type, u_int32_t seq, void *ctxt)
{
    u_int data_len;
    char *data = packet_get_string(&data_len);
    packet_check_eom();
    buffer_append(&stdout_buffer, data, data_len);
    memset(data, 0, data_len);
    free(data);
    return 0;
}
static int
client_input_stderr_data(int type, u_int32_t seq, void *ctxt)
{
    u_int data_len;
    char *data = packet_get_string(&data_len);
    packet_check_eom();
    buffer_append(&stderr_buffer, data, data_len);
    memset(data, 0, data_len);
    free(data);
    return 0;
}
static int
client_input_exit_status(int type, u_int32_t seq, void *ctxt)
{
    exit_status = packet_get_int();
    packet_check_eom();
    /* Acknowledge the exit. */
    packet_start(SSH_CMSG_EXIT_CONFIRMATION);
    packet_send();
    /*
     * Must wait for packet to be sent since we are
     * exiting the loop.
     */
    packet_write_wait();
    /* Flag that we want to exit. */
    quit_pending = 1;
    return 0;
}

static Channel *
client_request_forwarded_tcpip(const char *request_type, int rchan)
{
    Channel* c = NULL;
    char *listen_address, *originator_address;
    int listen_port, originator_port;
    int sock;

    /* Get rest of the packet */
    listen_address = packet_get_string(NULL);
    listen_port = packet_get_int();
    originator_address = packet_get_string(NULL);
    originator_port = packet_get_int();
    packet_done_null();

    debug1("client_request_forwarded_tcpip: listen host %s port %d",
           listen_address, listen_port);
    debug1("client_request_forwarded_tcpip: originator host %s port %d",
           originator_address, originator_port);

    sock = channel_connect_by_listen_address(listen_port);
    if (sock < 0) {
        free(originator_address);
        free(listen_address);
        return NULL;
    }
    c = channel_new("forwarded-tcpip",
        SSH_CHANNEL_CONNECTING, sock, sock, -1,
        CHAN_TCP_WINDOW_DEFAULT, CHAN_TCP_WINDOW_DEFAULT, 0,
        strdup(originator_address), 1);
    free(originator_address);
    free(listen_address);
    return c;
}

static Channel*
client_request_x11(const char *request_type, int rchan)
{
    Channel *c = NULL;
    char *originator;
    int originator_port;
    int sock;

    if (!options.forward_x11) {
        error("Warning: ssh server tried X11 forwarding.");
        error("Warning: this is probably a break in attempt by a malicious server.");
        return NULL;
    }
    originator = packet_get_string(NULL);
    if (datafellows & SSH_BUG_X11FWD) {
        debug2("buggy server: x11 request w/o originator_port");
        originator_port = 0;
    } else {
        originator_port = packet_get_int();
    }
    packet_done_null();

    /* XXX check permission */
    debug1("client_request_x11: request from originator host %s port %d",
           originator, originator_port);
    free(originator);
    sock = x11_connect_display();
    if (sock < 0)
        return NULL;
   c = channel_new("x11",
       SSH_CHANNEL_X11_OPEN, sock, sock, -1,
       CHAN_TCP_WINDOW_DEFAULT, CHAN_X11_PACKET_DEFAULT, 0,
       strdup("x11"), 1);
    c->force_drain = 1;
    return c;
}

static Channel*
client_request_agent(const char *request_type, int rchan)
{
    Channel *c = NULL;
    int sock;

    if (!options.forward_agent) {
        error("Warning: ssh server tried agent forwarding.");
        error("Warning: this is probably a break in attempt by a malicious server.");
        return NULL;
    }
    sock =  ssh_get_authentication_socket();
    if (sock < 0)
        return NULL;
    c = channel_new("authentication agent connection",
                     SSH_CHANNEL_OPEN, sock, sock, -1,
                     CHAN_X11_WINDOW_DEFAULT, CHAN_TCP_WINDOW_DEFAULT, 0,
                     strdup("authentication agent connection"), 1);
    c->force_drain = 1;
    return c;
}

/* XXXX move to generic input handler */
static int
client_input_channel_open(int type, u_int32_t seq, void *ctxt)
{
    Channel *c = NULL;
    char *ctype;
    int rchan;
    u_int rmaxpack, rwindow, len;

    ctype = packet_get_string(&len);
    rchan = packet_get_int();
    rwindow = packet_get_int();
    rmaxpack = packet_get_int();

    debug1("client_input_channel_open: ctype %s rchan %d window %d max %d",
            ctype,rchan,rwindow,rmaxpack);

    if (strcmp(ctype, "forwarded-tcpip") == 0) {
        c = client_request_forwarded_tcpip(ctype, rchan);
    } else if (strcmp(ctype, "x11") == 0) {
        c = client_request_x11(ctype, rchan);
    } else if (strcmp(ctype, "auth-agent@openssh.com") == 0) {
        c = client_request_agent(ctype, rchan);
    }
/* XXX duplicate : */
    if (c != NULL) {
        debug1("confirm %s", ctype);
        c->remote_id = rchan;
        c->remote_window = rwindow;
        c->remote_maxpacket = rmaxpack;

        packet_start(SSH2_MSG_CHANNEL_OPEN_CONFIRMATION);
        packet_put_int(c->remote_id);
        packet_put_int(c->self);
        packet_put_int(c->local_window);
        packet_put_int(c->local_maxpacket);
        packet_send();
    } else {
        debug1("failure %s", ctype);
        packet_start(SSH2_MSG_CHANNEL_OPEN_FAILURE);
        packet_put_int(rchan);
        packet_put_int(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
        if (!(datafellows & SSH_BUG_OPENFAILURE)) {
            packet_put_cstring("open failed");
            packet_put_cstring("");
        }
        packet_send();
    }
    free(ctype);
    return(0);
}
static int
client_input_channel_req(int type, u_int32_t seq, void *ctxt)
{
    Channel *c = NULL;
    int id, reply, success = 0;
    char *rtype;

    id = packet_get_int();
    rtype = packet_get_string(NULL);
    reply = packet_get_char();

    debug1("client_input_channel_req","channel %d rtype %s reply %s",
            id,rtype,reply);

    if (session_ident == -1) {
        error("client_input_channel_req: no channel %d", session_ident);
    } else if (id != session_ident) {
        error("client_input_channel_req: channel %d: wrong channel: %d",
               session_ident, id);
    }
    c = channel_lookup(id);
    if (c == NULL) {
        error("client_input_channel_req: channel %d: unknown channel", id);
    } else if (strcmp(rtype, "exit-status") == 0) {
        success = 1;
        exit_status = packet_get_int();
        packet_check_eom();
    }
    if (reply) {
        packet_start(success ?
                      SSH2_MSG_CHANNEL_SUCCESS : SSH2_MSG_CHANNEL_FAILURE);
        packet_put_int(c->remote_id);
        packet_send();
    }
    free(rtype);
    return (success ? 0 : -1);
}

static int
client_input_global_request(int type, u_int32_t seq, void *ctxt)
{
	char *rtype;
	int want_reply;
	int success = 0;

	rtype = packet_get_string(NULL);
	want_reply = packet_get_char();
	debug1("client_input_global_request: rtype %s want_reply %d", rtype, want_reply);
	if (want_reply) {
		packet_start(success ?
		    SSH2_MSG_REQUEST_SUCCESS : SSH2_MSG_REQUEST_FAILURE);
		packet_send();
		packet_write_wait();
	}
	free(rtype);
    return 0;
}

static void
client_init_dispatch_20(void)
{
    dispatch_init(&dispatch_protocol_error);
    dispatch_set(SSH2_MSG_CHANNEL_CLOSE, &channel_input_oclose);
    dispatch_set(SSH2_MSG_CHANNEL_DATA, &channel_input_data);
    dispatch_set(SSH2_MSG_CHANNEL_EOF, &channel_input_ieof);
    dispatch_set(SSH2_MSG_CHANNEL_EXTENDED_DATA, &channel_input_extended_data);
    dispatch_set(SSH2_MSG_CHANNEL_OPEN, &client_input_channel_open);
    dispatch_set(SSH2_MSG_CHANNEL_OPEN_CONFIRMATION, &channel_input_open_confirmation);
    dispatch_set(SSH2_MSG_CHANNEL_OPEN_FAILURE, &channel_input_open_failure);
    dispatch_set(SSH2_MSG_CHANNEL_REQUEST, &client_input_channel_req);
    dispatch_set(SSH2_MSG_CHANNEL_WINDOW_ADJUST, &channel_input_window_adjust);
    dispatch_set(SSH2_MSG_GLOBAL_REQUEST, &client_input_global_request);

    /* rekeying */
    dispatch_set(SSH2_MSG_KEXINIT, &kex_input_kexinit);

    /* global request reply messages */
    dispatch_set(SSH2_MSG_REQUEST_FAILURE, &client_global_request_reply);
    dispatch_set(SSH2_MSG_REQUEST_SUCCESS, &client_global_request_reply);
}
static void
client_init_dispatch_13(void)
{
    dispatch_init(NULL);
    dispatch_set(SSH_MSG_CHANNEL_CLOSE, &channel_input_close);
    dispatch_set(SSH_MSG_CHANNEL_CLOSE_CONFIRMATION, &channel_input_close_confirmation);
    dispatch_set(SSH_MSG_CHANNEL_DATA, &channel_input_data);
    dispatch_set(SSH_MSG_CHANNEL_OPEN_CONFIRMATION, &channel_input_open_confirmation);
    dispatch_set(SSH_MSG_CHANNEL_OPEN_FAILURE, &channel_input_open_failure);
    dispatch_set(SSH_MSG_PORT_OPEN, &channel_input_port_open);
    dispatch_set(SSH_SMSG_EXITSTATUS, &client_input_exit_status);
    dispatch_set(SSH_SMSG_STDERR_DATA, &client_input_stderr_data);
    dispatch_set(SSH_SMSG_STDOUT_DATA, &client_input_stdout_data);

    dispatch_set(SSH_SMSG_AGENT_OPEN, options.forward_agent ?
                  &auth_input_open_request : &deny_input_open);
    dispatch_set(SSH_SMSG_X11_OPEN, options.forward_x11 ?
                  &x11_input_open : &deny_input_open);
}
static void
client_init_dispatch_15(void)
{
    client_init_dispatch_13();
    dispatch_set(SSH_MSG_CHANNEL_CLOSE, &channel_input_ieof);
    dispatch_set(SSH_MSG_CHANNEL_CLOSE_CONFIRMATION, &channel_input_oclose);
}
static void
client_init_dispatch(void)
{
    if (compat20)
        client_init_dispatch_20();
    else if (compat13)
        client_init_dispatch_13();
    else
        client_init_dispatch_15();
}

/* SSH-KEYGEN */
static char *
key_type_name(int type)
{
    switch (type)
    {
    case KEY_RSA1:
        return "ssh1-rsa";
    case KEY_RSA:
        return "ssh2-rsa";
    case KEY_DSA:
        return "ssh2-dsa";
    case KEY_NULL:
        return "null";
    default:
        return "unspecified";
    }
}

static char identity_file[1024];

char *
sshkey_default_file(int type)
{
    char buf[1024], buf2[400];
    char *name = NULL;

    switch (type) {
    case KEY_RSA1:
        name = (char *)_PATH_SSH_CLIENT_IDENTITY;
        break;
    case KEY_SRP:
        name = (char *)_PATH_SSH_USER_VERIFIER;
        break;
    case KEY_DSA:
        name = (char *)_PATH_SSH_CLIENT_ID_DSA;
        break;
    case KEY_RSA:
        name = (char *)_PATH_SSH_CLIENT_ID_RSA;
        break;
    default:
        name = "unknown";
        break;
    }
    snprintf(identity_file, sizeof identity_file, "%s", name);
    return identity_file;
}

static char *
ask_filename(const char *prompt, int type)
{
    char buf[1024], buf2[400];
    int ok;

    snprintf(buf2, sizeof(buf2), "%s\n(%s)", prompt, sshkey_default_file(type));
    ok = uq_txt(NULL,buf2,1,NULL,buf,sizeof(buf),sshkey_default_file(type),DEFAULT_UQ_TIMEOUT);
    if (ok && strcmp(buf, "") != 0)
        strlcpy(identity_file, buf, sizeof(identity_file));
    return identity_file;
}

static Key *
sshkey_load_identity_file(char *filename, char * identity_passphrase)
{
    char pass[300];
    Key *prv;

    prv = key_load_private(filename, "", NULL);
    if (prv == NULL) {
        if (identity_passphrase)
            ckstrncpy(pass,identity_passphrase,sizeof(pass));
        else {
            char preface[256];
            int ok;

            ckmakmsg(preface,256,"Loading SSH Identity File: \"",
                     filename, "\"\n", NULL);
            ok = uq_txt(preface, "Enter passphrase: ", 2, NULL,
                        pass,300,NULL,DEFAULT_UQ_TIMEOUT);
            if (!ok)
                pass[0] = '\0';
        }
        prv = key_load_private(filename, pass, NULL);
        memset(pass, 0, strlen(pass));
    }
    return prv;
}

#define SSH_COM_PUBLIC_BEGIN            "---- BEGIN SSH2 PUBLIC KEY ----"
#define SSH_COM_PUBLIC_END              "---- END SSH2 PUBLIC KEY ----"
#define SSH_COM_PRIVATE_BEGIN           "---- BEGIN SSH2 ENCRYPTED PRIVATE KEY ----"
#define SSH_COM_PRIVATE_KEY_MAGIC       0x3f6ff9eb

int
sshkey_display_public_as_ssh2(char * filename, char * identity_passphrase)
{
    Key *k;
    int len;
    u_char *blob;
    struct _stat st;
    char kg_hostname[MAXHOSTNAMELEN];

    if (filename == NULL)
        filename = ask_filename("Enter file in which the key is", KEY_UNSPEC);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }
    if ((k = key_load_public(filename, NULL)) == NULL) {
        if ((k = sshkey_load_identity_file(filename, identity_passphrase)) == NULL) {
            fprintf(stderr, "load failed\n");
            return -1;
        }
    }

    if (gethostname(kg_hostname, sizeof(kg_hostname)) < 0) {
        perror("get_hostname");
        return -1;
    }

    if (key_to_blob(k, &blob, &len) <= 0) {
        fprintf(stderr, "key_to_blob failed\n");
        return -1;
    }
    printf("%s\n", SSH_COM_PUBLIC_BEGIN);
    printf("Comment: \"%d-bit %s, converted from OpenSSH by %s@%s\"\n",
             key_size(k), key_type(k),
             uidbuf, kg_hostname);
    dump_base64(stdout, blob, len);
    printf("%s\n", SSH_COM_PUBLIC_END);
    key_free(k);
    free(blob);
    return 0;
}

static int
buffer_get_bignum_bits(Buffer *b, BIGNUM *value)
{
    int bits = buffer_get_int(b);
    int bytes = (bits + 7) / 8;

    if (buffer_len(b) < bytes) {
        ssh_error(SSH_ERROR_BUFFER_TOO_SMALL,
                   "buffer_get_bignum_bits: input buffer too small: "
                   "need %d have %d", bytes, buffer_len(b));
        return(-1);
    }
    BN_bin2bn((u_char *)buffer_ptr(b), bytes, value);
    buffer_consume(b, bytes);
    return(0);
}

static Key *
do_convert_private_ssh2_from_blob(char *blob, int blen)
{
    Buffer b;
    Key *key = NULL;
    char *type, *cipher;
    u_char *sig, data[] = "abcde12345";
    int magic, rlen, ktype, i1, i2, i3, i4;
    u_int slen;
    u_long e;

    memset(&b,0,sizeof(Buffer));
    buffer_init(&b);
    buffer_append(&b, blob, blen);

    magic  = buffer_get_int(&b);
    if (magic != SSH_COM_PRIVATE_KEY_MAGIC) {
        error("bad magic 0x%x != 0x%x", magic, SSH_COM_PRIVATE_KEY_MAGIC);
        buffer_free(&b);
        return NULL;
    }
    i1 = buffer_get_int(&b);
    type   = buffer_get_string(&b, NULL);
    cipher = buffer_get_string(&b, NULL);
    i2 = buffer_get_int(&b);
    i3 = buffer_get_int(&b);
    i4 = buffer_get_int(&b);
    debug1("ignore (%d %d %d %d)", i1,i2,i3,i4);

    if (strcmp(cipher, "none") != 0) {
        error("unsupported cipher %s", cipher);
        free(cipher);
        buffer_free(&b);
        free(type);
        return NULL;
    }
    free(cipher);

    if (strstr(type, "dsa")) {
        ktype = KEY_DSA;
    } else if (strstr(type, "rsa")) {
        ktype = KEY_RSA;
    } else {
        free(type);
        return NULL;
    }
    key = key_new_private(ktype);
    free(type);

        switch (key->type) {
        case KEY_DSA:
            if (buffer_get_bignum_bits(&b, key->dsa->p) < 0 ||
                 buffer_get_bignum_bits(&b, key->dsa->g) < 0 ||
                 buffer_get_bignum_bits(&b, key->dsa->q) < 0 ||
                 buffer_get_bignum_bits(&b, key->dsa->pub_key) < 0 ||
                 buffer_get_bignum_bits(&b, key->dsa->priv_key) < 0)
                return(NULL);
            break;
        case KEY_RSA:
            e  = buffer_get_char(&b);
            if (e < 30) {
                e <<= 8;
                e += buffer_get_char(&b);
                e <<= 8;
                e += buffer_get_char(&b);
            }
            if (!BN_set_word(key->rsa->e, e)) {
                buffer_free(&b);
                key_free(key);
                return NULL;
            }
            if (buffer_get_bignum_bits(&b, key->rsa->d) < 0 ||
                 buffer_get_bignum_bits(&b, key->rsa->n) < 0 ||
                 buffer_get_bignum_bits(&b, key->rsa->iqmp) < 0 ||
                 buffer_get_bignum_bits(&b, key->rsa->q) < 0 ||
                 buffer_get_bignum_bits(&b, key->rsa->p) < 0)
                return(NULL);
            generate_additional_parameters(key->rsa);
            break;
        }
    rlen = buffer_len(&b);
    if(rlen != 0)
        error("do_convert_private_ssh2_from_blob: "
               "remaining bytes in key blob %d", rlen);
    buffer_free(&b);
    /* try the key */
    key_sign(key, &sig, &slen, data, sizeof data);
    key_verify(key, sig, slen, data, sizeof data);
    free(sig);
    return key;
}

#ifdef COMMENT
static int
do_convert_from_ssh2(char * source, char * dest)
{
    Key *k;
    int blen;
    char line[1024], *p;
    u_char blob[8096];
    char encoded[8096];
    struct _stat st;
    int escaped = 0, private = 0, ok;
    FILE *fp;

    if (filename == NULL)
        filename = ask_filename("Enter file in which the key is",KEY_UNSPEC);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        return -1;
    }
    encoded[0] = '\0';
    while (fgets(line, sizeof(line), fp)) {
        if (!(p = strchr(line, '\n'))) {
            fprintf(stderr, "input line too long.\n");
            return -1;
        }
        if (p > line && p[-1] == '\\')
            escaped++;
        if (strncmp(line, "----", 4) == 0 ||
             strstr(line, ": ") != NULL) {
            if (strstr(line, SSH_COM_PRIVATE_BEGIN) != NULL)
                private = 1;
            if (strstr(line, " END ") != NULL) {
                break;
            }
            /* xfprintf(stderr, "ignore: %s", line); */
            continue;
        }
        if (escaped) {
            escaped--;
                        /* xfprintf(stderr, "escaped: %s", line); */
            continue;
        }
        *p = '\0';
        strlcat(encoded, line, sizeof(encoded));
    }
    blen = uudecode(encoded, (u_char *)blob, sizeof(blob));
    if (blen < 0) {
        xfprintf(stderr, "uudecode failed.\n");
        return -1;
    }
    k = private ?
        do_convert_private_ssh2_from_blob(blob, blen) :
            key_from_blob(blob, blen);
    if (k == NULL) {
        fprintf(stderr, "decode blob failed.\n");
        return -1;
    }
    ok = private ?
        (k->type == KEY_DSA ?
          PEM_write_DSAPrivateKey(stdout, (char *)k->dsa, NULL, NULL, 0, NULL, NULL) :
          PEM_write_RSAPrivateKey(stdout, (char *)k->rsa, NULL, NULL, 0, NULL, NULL)) :
              key_write(k, stdout);
    if (!ok) {
        fprintf(stderr, "key write failed");
        return -1;
    }
    key_free(k);
    fprintf(stdout, "\n");
    fclose(fp);
    return 0;
}
#endif /* COMMENT */

#ifdef SMARTCARD
#define NUM_RSA_KEY_ELEMENTS 5+1
#define COPY_RSA_KEY(x, i) \
        do { \
                len = BN_num_bytes(prv->rsa->x); \
                elements[i] = malloc(len); \
                debug1("#bytes %d", len); \
                if (BN_bn2bin(prv->rsa->x, elements[i]) < 0) \
                        goto done; \
        } while(0)

static int
get_AUT0(char *aut0)
{
        EVP_MD *evp_md = EVP_sha1();
        EVP_MD_CTX md;
        char *pass;

        pass = read_passphrase("Enter passphrase for smartcard: ", RP_ALLOW_STDIN);
        if (pass == NULL)
                return -1;
        EVP_DigestInit(&md, evp_md);
        EVP_DigestUpdate(&md, pass, strlen(pass));
        EVP_DigestFinal(&md, aut0, NULL);
        memset(pass, 0, strlen(pass));
        free(pass);
        return 0;
}

int
sshkey_do_upload(struct passwd *pw, const char *sc_reader_id, char *identity_passphrase)
{
        Key *prv = NULL;
        struct _stat st;
        u_char *elements[NUM_RSA_KEY_ELEMENTS];
        u_char key_fid[2];
        u_char DEFAUT0[] = {0xad, 0x9f, 0x61, 0xfe, 0xfa, 0x20, 0xce, 0x63};
        u_char AUT0[EVP_MAX_MD_SIZE];
        int len, status = 1, i, fd = -1, ret;
        int sw = 0, cla = 0x00;

        for (i = 0; i < NUM_RSA_KEY_ELEMENTS; i++)
                elements[i] = NULL;
        if (!have_identity)
                ask_filename(pw, "Enter file in which the key is");
        if (stat(identity_file, &st) < 0) {
                perror(identity_file);
                goto done;
        }
        prv = load_identity(identity_file, identity_passphrase);
        if (prv == NULL) {
                error("load failed");
                goto done;
        }
        COPY_RSA_KEY(q, 0);
        COPY_RSA_KEY(p, 1);
        COPY_RSA_KEY(iqmp, 2);
        COPY_RSA_KEY(dmq1, 3);
        COPY_RSA_KEY(dmp1, 4);
        COPY_RSA_KEY(n, 5);
        len = BN_num_bytes(prv->rsa->n);
        fd = sectok_friendly_open(sc_reader_id, STONOWAIT, &sw);
        if (fd < 0) {
                error("sectok_open failed: %s", sectok_get_sw(sw));
                goto done;
        }
        if (! sectok_cardpresent(fd)) {
                error("smartcard in reader %s not present",
                    sc_reader_id);
                goto done;
        }
        ret = sectok_reset(fd, 0, NULL, &sw);
        if (ret <= 0) {
                error("sectok_reset failed: %s", sectok_get_sw(sw));
                goto done;
        }
        if ((cla = cyberflex_inq_class(fd)) < 0) {
                error("cyberflex_inq_class failed");
                goto done;
        }
        memcpy(AUT0, DEFAUT0, sizeof(DEFAUT0));
        if (cyberflex_verify_AUT0(fd, cla, AUT0, sizeof(DEFAUT0)) < 0) {
                if (get_AUT0(AUT0) < 0 ||
                    cyberflex_verify_AUT0(fd, cla, AUT0, sizeof(DEFAUT0)) < 0) {
                        error("cyberflex_verify_AUT0 failed");
                        goto done;
                }
        }
        key_fid[0] = 0x00;
        key_fid[1] = 0x12;
        if (cyberflex_load_rsa_priv(fd, cla, key_fid, 5, 8*len, elements,
            &sw) < 0) {
                error("cyberflex_load_rsa_priv failed: %s", sectok_get_sw(sw));
                goto done;
        }
        if (!sectok_swOK(sw))
                goto done;
        log("cyberflex_load_rsa_priv done");
        key_fid[0] = 0x73;
        key_fid[1] = 0x68;
        if (cyberflex_load_rsa_pub(fd, cla, key_fid, len, elements[5],
            &sw) < 0) {
                error("cyberflex_load_rsa_pub failed: %s", sectok_get_sw(sw));
                goto done;
        }
        if (!sectok_swOK(sw))
                goto done;
        log("cyberflex_load_rsa_pub done");
        status = 0;
        log("loading key done");
done:

        memset(elements[0], '\0', BN_num_bytes(prv->rsa->q));
        memset(elements[1], '\0', BN_num_bytes(prv->rsa->p));
        memset(elements[2], '\0', BN_num_bytes(prv->rsa->iqmp));
        memset(elements[3], '\0', BN_num_bytes(prv->rsa->dmq1));
        memset(elements[4], '\0', BN_num_bytes(prv->rsa->dmp1));
        memset(elements[5], '\0', BN_num_bytes(prv->rsa->n));

        if (prv)
                key_free(prv);
        for (i = 0; i < NUM_RSA_KEY_ELEMENTS; i++)
                if (elements[i])
                        free(elements[i]);
        if (fd != -1)
                sectok_close(fd);
        return(status);
}

int
sshkey_do_download(struct passwd *pw, const char *sc_reader_id)
{
        Key *pub = NULL;

        pub = sc_get_key(sc_reader_id);
        if (pub == NULL)
                fatal("cannot read public key from smartcard");
        key_write(pub, stdout);
        key_free(pub);
        fprintf(stdout, "\n");
        return(0);
}
#endif /* SMARTCARD */

int
sshkey_display_public(char * filename, char * identity_passphrase)
{
    Key *prv;
    struct _stat st;

    if (filename == NULL)
        filename = ask_filename("Enter file in which the key is",KEY_UNSPEC);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }
    prv = sshkey_load_identity_file(filename,identity_passphrase);
    if (prv == NULL) {
        fprintf(stderr, "load failed\n");
        return -1;
    }
    if (!key_write(prv, stdout))
        fprintf(stderr, "key_write failed");
    key_free(prv);
    fprintf(stdout, "\n");
    return 0;
}

int
sshkey_display_fingerprint(char * filename, int babble)
{
    FILE *f;
    Key *public;
    char *comment = NULL, *cp, *ep, line[16*1024], *fp;
    int i, skip = 0, num = 1, invalid = 1, rep, fptype;
    struct _stat st;

    fptype = babble ? SSH_FP_SHA1 : SSH_FP_MD5;
    rep =    babble ? SSH_FP_BUBBLEBABBLE : SSH_FP_HEX;

    if (filename == NULL)
        filename = ask_filename("Enter file in which the key is",KEY_UNSPEC);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }

    /* Attempt to process the file as a V1 RSA key */
    public = key_load_public(filename, &comment);
    if (public != NULL) {
        fp = key_fingerprint(public, fptype, rep);
        printf("%d %s %s\n", key_size(public), fp, comment);
        key_free(public);
        free(comment);
        free(fp);
        return 0;
    }
    if (comment)
        free(comment);

    /* Its a V2 key */
    f = fopen(filename, "r");
    if (f != NULL) {
        while (fgets(line, sizeof(line), f)) {
            i = strlen(line) - 1;
            if (line[i] != '\n') {
                error("line %d too long: %.40s...", num, line);
                skip = 1;
                continue;
            }
            num++;
            if (skip) {
                skip = 0;
                continue;
            }
            line[i] = '\0';

            /* Skip leading whitespace, empty and comment lines. */
            for (cp = line; *cp == ' ' || *cp == '\t'; cp++)
                ;
            if (!*cp || *cp == '\n' || *cp == '#')
                                continue ;
            i = strtol(cp, &ep, 10);
            if (i == 0 || ep == NULL || (*ep != ' ' && *ep != '\t')) {
                int quoted = 0;
                comment = cp;
                for (; *cp && (quoted || (*cp != ' ' && *cp != '\t')); cp++) {
                    if (*cp == '\\' && cp[1] == '"')
                        cp++;   /* Skip both */
                    else if (*cp == '"')
                        quoted = !quoted;
                }
                if (!*cp)
                    continue;
                *cp++ = '\0';
            }
            ep = cp;
            public = key_new(KEY_RSA1);
            if (key_read(public, &cp) != 1) {
                cp = ep;
                key_free(public);
                public = key_new(KEY_UNSPEC);
                if (key_read(public, &cp) != 1) {
                    key_free(public);
                    continue;
                }
            }
            comment = *cp ? cp : comment;
            fp = key_fingerprint(public, fptype, rep);
            xprintf("%d %s %s\n", key_size(public), fp,
                     comment ? comment : "no comment");
            free(fp);
            key_free(public);
            invalid = 0;
        }
        fclose(f);
    }
    if (invalid) {
        printf("%s is not a valid key file.\n", filename);
        return -1;
    }
    return 0;
}

/*
 * Perform changing a passphrase.  The argument is the passwd structure
 * for the current user.
 */
int
sshkey_change_passphrase(char * filename, char * oldpp, char * newpp)
{
    char *comment;
    char old_passphrase[300], passphrase1[300], passphrase2[300];
    struct _stat st;
    Key *private;
    int ok;

    if (!filename)
        filename = ask_filename("Enter name of file in which the key is located.",KEY_DSA);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }
    /* Try to load the file with empty passphrase. */
    private = key_load_private(filename, "", &comment);
    if (private == NULL) {
        if (oldpp) {
            strncpy(old_passphrase,oldpp,sizeof(old_passphrase));
            ok = 1;
        } else {
            char preface[512];
            ckmakmsg(preface,512,"Enter old passphrase to access '",
                      filename,"'.\n",NULL);
            ok = uq_txt(preface,"Old Passphrase: ",2,NULL,old_passphrase, 
                         sizeof(old_passphrase),NULL,DEFAULT_UQ_TIMEOUT);
        }
        if ( !ok ) {
            printf("Cancelled\n");
            return(-1);
        }
        private = key_load_private(filename, old_passphrase , &comment);
        memset(old_passphrase, 0, strlen(old_passphrase));
        if (private == NULL) {
            printf("Bad passphrase.\n");
            return -1;
        }
    }
    printf("Key has comment '%s'\n", comment);

    /* Ask the new passphrase (twice). */
    if (newpp) {
        ckstrncpy(passphrase1,newpp,sizeof(passphrase1));
        passphrase2[0] = '\0';
        ok = 1;
    } else {
        struct txtbox tb[2];
        tb[0].t_buf = passphrase1;
        tb[0].t_len = sizeof(passphrase1);
        tb[0].t_lbl = "New passphrase (empty for no passphrase): ";
        tb[0].t_dflt = NULL;
        tb[0].t_echo = 2;
        tb[1].t_buf = passphrase2;
        tb[1].t_len = sizeof(passphrase2);
        tb[1].t_lbl = "New passphrase (again): ";
        tb[1].t_dflt = NULL;
        tb[1].t_echo = 2;

        ok = uq_mtxt(NULL, NULL, 2, tb);

        if ( !ok ) {
            printf("Cancelled\n");
            return(-1);
        }

        /* Verify that they are the same. */
        if (strcmp(passphrase1, passphrase2) != 0) {
            memset(passphrase1, 0, strlen(passphrase1));
            memset(passphrase2, 0, strlen(passphrase2));
            printf("Pass phrases do not match.  Try again.\n");
            return -1;
        }
        /* Destroy the other copy. */
        memset(passphrase2, 0, strlen(passphrase2));
    }

    /* Save the file using the new passphrase. */
    if (!key_save_private(private, filename, passphrase1, comment)) {
        printf("Saving the key failed: %s.\n", filename);
        memset(passphrase1, 0, strlen(passphrase1));
        key_free(private);
        free(comment);
        return -1;
    }
    /* Destroy the passphrase and the copy of the key in memory. */
    memset(passphrase1, 0, strlen(passphrase1));
    key_free(private);           /* Destroys contents */
    free(comment);

    printf("Your identification has been saved with the new passphrase.\n");
    return 0;
}

/*
 * Change the comment of a private key file.
 */
int
sshkey_v1_change_comment(char * filename, char * new_comment, char * pp)
{
    char buf[1024], *comment, passphrase[300], pubfile[300];
    Key *private;
    Key *public;
    struct _stat st;
    FILE *f;
    int fd, ok;

    if (filename == NULL)
        filename = ask_filename("Enter file in which the key is",KEY_RSA1);
    if (filename == NULL)
        return -1;
    if (stat(filename, &st) < 0) {
        perror(filename);
        return -1;
    }
    private = key_load_private(filename, "", &comment);
    if (private == NULL) {
        if (pp) {
            strncpy(passphrase, pp, sizeof(passphrase));
            ok = 1;
        } else {
            ok = uq_txt(NULL,"Enter passphrase: ",2,NULL, 
                        passphrase, sizeof(passphrase), NULL,DEFAULT_UQ_TIMEOUT);
        }

        if ( !ok ) {
            printf("Cancelled\n");
            return(-1);
        }

        /* Try to load using the passphrase. */
        private = key_load_private(filename, passphrase, &comment);
        if (private == NULL) {
            memset(passphrase, 0, strlen(passphrase));
            printf("Bad passphrase.\n");
            return -1;
        }
    } else {
        passphrase[0] = '\0';
    }
    if (private->type != KEY_RSA1) {
        xfprintf(stderr, "Comments are only supported for RSA1 keys.\n");
        key_free(private);
        return -1;
    }

    if (new_comment == NULL) {
        char preface[256];
        int ok;
        ckmakmsg(preface,256,"Old Comment '",comment,"'\n",NULL);
        ok = uq_txt(preface,"New Comment: ",1,NULL,buf,sizeof(buf),NULL,DEFAULT_UQ_TIMEOUT);
        if ( !ok ) {
            printf("Cancelled\n");
            memset(passphrase, 0, strlen(passphrase));
            key_free(private);
            free(comment);
        }
        new_comment = buf;
    } else {
        printf("Old comment '%s'\n", comment);
        printf("New comment '%s'\n", new_comment);
    }

    /* Save the file using the new passphrase. */
    if (!key_save_private(private, filename, passphrase, new_comment)) {
        printf("Saving the key failed: %s.\n", filename);
        memset(passphrase, 0, strlen(passphrase));
        key_free(private);
        free(comment);
        return -1;
    }
    memset(passphrase, 0, strlen(passphrase));
    public = key_from_private(private);
    key_free(private);

    ckstrncpy(pubfile,filename,sizeof(pubfile));
    ckstrncat(pubfile,".pub",sizeof(pubfile));
    fd = open(pubfile, _O_WRONLY | _O_CREAT | _O_TRUNC, 0644);
        if (fd == -1) {
            printf("Could not save your public key in %s\n", pubfile);
            return -1;
        }
    f = _fdopen(fd, "w");
    if (f == NULL) {
        printf("_fdopen %s failed", pubfile);
        return -1;
    }
    if (!key_write(public, f))
        fprintf(stderr, "write key failed");
    key_free(public);
    fprintf(f, " %s\n", new_comment);
    fclose(f);

    free(comment);

    printf("The comment in your key file has been changed.\n");
    return 0;
}

#ifdef CK_SRP
/* Keep track of the number of different bit sizes we know. */

typedef struct bitslist BITSLIST;
struct bitslist {
    int bits;
    int count;
    BITSLIST *next;
};

static BITSLIST *Bits = NULL;

static BITSLIST *
bits_new(int bits, BITSLIST *next)
{
    BITSLIST *p;

    p = malloc(sizeof(BITSLIST));
    p->bits = bits;
    p->count = 1;
    p->next = next;

    return p;
}

static void
bits_free(BITSLIST ** pBits)
{
    BITSLIST * next;

    if ( pBits == NULL )
        return;

    if ( *pBits == NULL )
        return;

    next = (*pBits)->next;
    free(*pBits);
    (*pBits) = NULL;

    if ( next )
        bits_free(&next);
}

static void
bits_add(int bits)
{
    BITSLIST *p, *pp = NULL;

    /* Do a little insertion sort, just for fun. */

    if (Bits == NULL) {
        Bits = bits_new(bits, NULL);
        return;
    }
    for (p = Bits; p; p = p->next) {
        if (p->bits == bits) {
            p->count++;
            return;
        }
        if (p->bits > bits)
            break;
        pp = p;
    }
    if (pp == NULL) {
        p = Bits;
        Bits = bits_new(bits, p);
    } else {
        p = pp->next;
        pp->next = bits_new(bits, p);
    }
}

static void
bits_dump(void)
{
    BITSLIST *p;

    if (!Bits) {
        fprintf(stderr, "no known prime groups!\n");
        exit(1);
    }
    printf("Known prime group sizes (count):\n");
    for (p = Bits; p; p = p->next)
        printf("\t%d (%d)\n", p->bits, p->count);
}
#endif /* CK_SRP */

int
sshkey_create(char * filename, int bits, char * pp, int type, char * cmd_comment)
{
    char dotsshdir[16 * 1024], comment[1024], passphrase1[300], passphrase2[300];
    char kg_hostname[MAXHOSTNAMELEN];
    char pubfile[300];
    Key *private, *public;
    int fd;
    struct _stat st;
    FILE *f;
    int ok;

    if (bits < 512 || bits > 32768) {
        printf("Bits has bad value.\n");
        return -1;
    }

    if (type > KEY_SRP) {
        fprintf(stderr, "unsupported key type %d\n", type);
        return -1;
    }

    arc4random_stir();

    if (filename == NULL) {
        filename = ask_filename("Enter file in which to save the key",type);
        if (filename == NULL)
            return -1;
    }

    /* Create ~/.ssh directory if it doesn't already exist. */
    snprintf(dotsshdir, sizeof dotsshdir, "%s", (char *)_PATH_SSH_USER_DIR);
    if (strstr(filename, dotsshdir) != NULL &&
        stat(dotsshdir, &st) < 0) {
        if (mkdir(dotsshdir, 0700) < 0)
            error("Could not create directory '%s'.", dotsshdir);
        else if (!quiet)
            printf("Created directory '%s'.\n", dotsshdir);
    }

    /* If the file already exists, ask the user to confirm. */
    if (stat(filename, &st) >= 0) {
        char buf[300];
        int x;
        snprintf(buf, sizeof buf, "%s already exists.\n Overwrite (y/n)?",
                 filename);
#ifdef COMMENT
        if (!getyesno(buf,0))
            return -1;
#else /* COMMENT */
        x = uq_ok( "Creating SSH Key File",
                   buf, 3, NULL, 0);
        if ( x <= 0 ) 
            return(-1);
#endif /* COMMENT */
    }

#ifdef CK_SRP
    if ( type == KEY_SRP ) {
        BIGNUM *p, *g, *s, *x, *v;
        BN_CTX *ctx, *ctx2;
        int i, j, c, which, slen;
        char * salt = NULL, *buf=NULL;
        FILE *outfile;

        /* Allocate BIGNUMs and stuff. */
        if ((ctx = BN_CTX_new()) == NULL)
            goto err;
        if ((ctx2 = BN_CTX_new()) == NULL)
            goto err;
        BN_CTX_start(ctx);
        p = BN_CTX_get(ctx);
        g = BN_CTX_get(ctx);
        s = BN_CTX_get(ctx);
        x = BN_CTX_get(ctx);
        v = BN_CTX_get(ctx);
        if (v == NULL)
            goto err;

        /*
         * Count the number of primes we know that are bits long.
         * Just use the built-in values for now.  Better would be
         * to include the external tables as well, ignoring any
         * duplicates.  Maybe later.
         */
        c = 0;
        for (i = 0; i < SRP_nparams; i++) {
            srp_get_param(i, p, g);
            j = BN_num_bits(p);
            bits_add(j);
            if (j == bits)
                c++;
        }
        if (c == 0) {
            printf("no primes found with %d bits\n", bits);
            bits_dump();
            return(-1);
        }
        printf("%d candidate groups found.\n", c);

        /* Pick one, and get it. */
        which = arc4random() % c;
        c = 0;
        for (i = 0; i < SRP_nparams; i++) {
            srp_get_param(i, p, g);
            j = BN_num_bits(p);
            if (j == bits) {
                if (c == which)
                    break;
                c++;
            }
        }

        /* Pick a random salt. */

        if (!BN_rand(s, SALTLEN, 0, 0))
            goto err;
        slen = BN_num_bytes(s);
        salt = malloc(slen);
        if (!BN_bn2bin(s, salt))
            goto err;

        if ( pp ) {
            ckstrncpy(passphrase1,pp,sizeof(passphrase1));
            passphrase2[0] = '\0';
            ok = 1;
        } else {
            struct txtbox tb[2];
            tb[0].t_buf = passphrase1;
            tb[0].t_len = sizeof(passphrase1);
            tb[0].t_lbl = "New passphrase (empty for no passphrase): ";
            tb[0].t_dflt = NULL;
            tb[0].t_echo = 2;
            tb[1].t_buf = passphrase2;
            tb[1].t_len = sizeof(passphrase2);
            tb[1].t_lbl = "New passphrase (again): ";
            tb[1].t_dflt = NULL;
            tb[1].t_echo = 2;

            /* Get the (new) passphrase. */
            snprintf(comment, sizeof(comment),
                      "Enter SRP passphrase for '%s'\n", uidbuf);

            ok = uq_mtxt(comment, NULL, 2, tb);

            if ( !ok ) {
                printf("Cancelled\n");
                return(-1);
            }

            if (strcmp(passphrase1, passphrase2) != 0) {
                memset(passphrase1, 0, strlen(passphrase1));
                memset(passphrase2, 0, strlen(passphrase2));
                fprintf(stderr,"passphrase mismatch, no action taken\n");
                return(-1);
            }
        }

        /* Calculate x = HASH(salt | HASH(username | passphrase)). */
        srp_x_calc(salt, slen, uidbuf, passphrase1, x);

        memset(passphrase1, 0, strlen(passphrase1));
        memset(passphrase2, 0, strlen(passphrase2));

        /* Calculate v = g^x mod p. */

        if (!BN_mod_exp(v, g, x, p, ctx2))
            goto err;

        /* Create the file. */

        fd = open(filename, _O_WRONLY | _O_CREAT | _O_TRUNC, 0600);
        if (fd < 0) {
            fprintf(stderr, "can't create '%s'\n", filename);
            return(-1);
        }
        outfile = _fdopen(fd, "w");
        if (outfile == NULL) {
            fprintf(stderr, "can't open '%s'\n", filename);
            close(fd);
            return(-1);
        }

        /* Convert to base64 and write it all out. */
        buf = malloc(8 * 1024);

        fprintf(outfile, "%s:", uidbuf);

        srp_bn2tfmt(buf, v);
        fprintf(outfile, "%s:", buf);

        srp_bn2tfmt(buf, s);
        fprintf(outfile, "%s::", buf);

        srp_bn2tfmt(buf, p);
        fprintf(outfile, "%s:", buf);

        srp_bn2tfmt(buf, g);
        fprintf(outfile, "%s\n", buf);

        fclose(outfile);

        if ( Bits )
            bits_free(&Bits);
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
        BN_CTX_free(ctx2);
        return 0;

      err:
        if ( Bits )
            bits_free(&Bits);
        fprintf(stderr, "BIGNUM error\n");
        return -1;
    } else
#endif /* CK_SRP */
    {
        if (!quiet)
            printf("Generating public/private %s key pair.\n", key_type_name(type));
        private = key_generate(type, bits);
        if (private == NULL) {
            fprintf(stderr, "key_generate failed");
            return -1;
        }
        public  = key_from_private(private);

        /* Ask for a passphrase (twice). */
        if (pp)
            ckstrncpy(passphrase1, pp, sizeof(passphrase1));
        else {
            struct txtbox tb[2];
            tb[0].t_buf = passphrase1;
            tb[0].t_len = sizeof(passphrase1);
            tb[0].t_lbl = "New passphrase (empty for no passphrase): ";
            tb[0].t_dflt = NULL;
            tb[0].t_echo = 2;
            tb[1].t_buf = passphrase2;
            tb[1].t_len = sizeof(passphrase2);
            tb[1].t_lbl = "New passphrase (again): ";
            tb[1].t_dflt = NULL;
            tb[1].t_echo = 2;

            /* Get the (new) passphrase. */
            snprintf(comment, sizeof(comment),
                      "Enter SSH passphrase\n");

            ok = uq_mtxt(comment, NULL, 2, tb);

            if ( !ok ) {
                printf("Cancelled\n");
                return(-1);
            }

            if (strcmp(passphrase1, passphrase2) != 0) {
                fprintf(stderr,
                         "passphrase mismatch, no action taken\n");
                return(-1);
            }
            if (strcmp(passphrase1, passphrase2) != 0) {
                memset(passphrase1, 0, strlen(passphrase1));
                memset(passphrase2, 0, strlen(passphrase2));
                fprintf(stderr,"passphrase mismatch, no action taken\n");
                return(-1);
            }

            /* Clear the other copy of the passphrase. */
            memset(passphrase2, 0, strlen(passphrase2));
        }

        if (cmd_comment) {
            strlcpy(comment, cmd_comment, sizeof(comment));
        } else {
            /* Create default commend field for the passphrase. */
            if (gethostname(kg_hostname, sizeof(kg_hostname)) < 0) {
                perror("get_hostname");
                return -1;
            }
            snprintf(comment, sizeof comment, "%s@%s", uidbuf, kg_hostname);
        }

        /* Save the key with the given passphrase and comment. */
        if (!key_save_private(private, filename, passphrase1, comment)) {
            printf("Saving the key failed: %s.\n", filename);
            memset(passphrase1, 0, strlen(passphrase1));
            return -1;
        }
        /* Clear the passphrase. */
        memset(passphrase1, 0, strlen(passphrase1));

        /* Clear the private key and the random number generator. */
        key_free(private);
        arc4random_stir();

        if (!quiet)
            printf("Your identification has been saved in %s.\n", filename);

        ckstrncpy(pubfile,filename,sizeof(pubfile));
        ckstrncat(pubfile,".pub",sizeof(pubfile));
        fd = open(pubfile, _O_WRONLY | _O_CREAT | _O_TRUNC, 0644);
        if (fd == -1) {
            printf("Could not save your public key in %s\n", pubfile);
            return -1;
        }
        f = _fdopen(fd, "w");
        if (f == NULL) {
            printf("_fdopen %s failed", pubfile);
            return -1;
        }
        if (!key_write(public, f))
            fprintf(stderr, "write key failed");
        fprintf(f, " %s\n", comment);
        fclose(f);

        if (!quiet) {
            char *fp = key_fingerprint(public, SSH_FP_MD5, SSH_FP_HEX);
            printf("Your public key has been saved in %s.\n",
                    pubfile);
            printf("The key fingerprint is:\n");
            printf("%s %s\n", fp, comment);
            free(fp);
        }

        key_free(public);
    }
    return 0;
}

void
ssh_start_agent(void)
{
    extern char exedir[];
    char buf[CKMAXPATH];
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    
    ckstrncpy(buf,exedir,CKMAXPATH);
    ckstrncat(buf,"ssh-agent.exe",CKMAXPATH);

    memset( &si, 0, sizeof(STARTUPINFO) );     //  Initialize struct
    si.cb          = sizeof(STARTUPINFO);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = 0;        //  Don't show the console window (DOS box)

    printf("Starting SSH Agent...");

    if (CreateProcess ( NULL,
                        buf,
                        NULL,
                        NULL,
                        FALSE, // bInheritHandler
                        0,
                        NULL,
                        NULL,
                        &si,
                        &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        sleep(3);
        printf("Complete\n");
        return;
    }
    printf("Failed\n");
}

int
ssh_agent_delete_file(const char *filename)
{
    Key *public;
    char *comment = NULL;
    int success;
    AuthenticationConnection *ac = NULL;

    /* At first, get a connection to the authentication agent. */
    ac = ssh_get_authentication_connection();
    if (ac == NULL) {
        ssh_start_agent();
        ac = ssh_get_authentication_connection();
    }
    if (ac == NULL) {
        xprintf("?Could not open a connection to authentication agent: ssh-agent.exe.\n");
        return(-1);
    }

    public = key_load_public(filename, &comment);
    if (public == NULL) {
        xprintf("Bad key file %s\n", filename);
        return -1;
    }

    if (success = ssh_remove_identity(ac, public)) {
        if ( comment && strcmp(filename,comment))
            xfprintf(stderr, "Identity removed: %s (%s)\n", filename, comment);
        else
            xfprintf(stderr, "Identity removed: %s\n", filename);
    } else
        xfprintf(stderr, "Could not remove identity: %s\n", filename);
    key_free(public);
    if (comment) free(comment);

    ssh_close_authentication_connection(ac);
    return(success ? 0 : -1);
}

/* Send a request to remove all identities. */
int
ssh_agent_delete_all(void)
{
    int success = 1;
    AuthenticationConnection *ac = NULL;

    /* At first, get a connection to the authentication agent. */
    ac = ssh_get_authentication_connection();
    if (ac == NULL) {
        ssh_start_agent();
        ac = ssh_get_authentication_connection();
    }
    if (ac == NULL) {
        xprintf("?Could not open a connection to authentication agent: ssh-agent.exe.\n");
        return(-1);
    }

    if (!ssh_remove_all_identities(ac, 1))
        success = 0;
    /* ignore error-code for ssh2 */
    ssh_remove_all_identities(ac, 2);

    if (success)
        xfprintf(stderr, "All identities removed.\n");
    else
        xfprintf(stderr, "Failed to remove all identities.\n");

    ssh_close_authentication_connection(ac);
    return(success ? 0 : -1);
}

int
ssh_agent_add_file(const char *filename)
{
    struct _stat st;
    Key *private;
    char *comment = NULL;
    char msg[1024], prompt[1024];
    char pass[300]="";
    int success;
    AuthenticationConnection *ac = NULL;

    /* At first, get a connection to the authentication agent. */
    ac = ssh_get_authentication_connection();
    if (ac == NULL) {
        ssh_start_agent();
        ac = ssh_get_authentication_connection();
    }
    if (ac == NULL) {
        xprintf("?Could not open a connection to authentication agent: ssh-agent.exe.\n");
        return(-1);
    }

    if (stat(filename, &st) < 0) {
        perror(filename);
        return(-1);
    }
    /* At first, try empty passphrase */
    private = key_load_private(filename, "", &comment);
    /* try last */
    if (private == NULL) {
        /* clear passphrase since it did not work */
        snprintf(msg, sizeof msg, "Passphrase required to access %.200s\n", 
                 filename);
        if (comment != NULL)
            snprintf(prompt, sizeof prompt, "Enter passphrase for \"%.200s\": ",
                      comment);
        else
            snprintf(prompt, sizeof prompt, "Enter passphrase: ");

        for (;;) {
            int ok = uq_txt(msg,prompt,2,NULL,pass,300,NULL,DEFAULT_UQ_TIMEOUT);
            if (!ok || strcmp(pass, "") == 0) {
                free(comment);
                memset(pass,0,strlen(pass));
                return(0);
            }
            private = key_load_private(filename, pass, &comment);
            if (private != NULL)
                break;
            snprintf(msg, sizeof msg, 
                     "Bad passphrase, try again!\n"
                     "Need passphrase for %.200s\n",
                     filename);
            memset(pass,0,strlen(pass));
        }
    }
    if (success = ssh_add_identity(ac, private, comment ? comment : filename)) {
        if ( comment && strcmp(filename,comment))
            xfprintf(stderr, "Identity added: %s (%s)\n", filename, comment);
        else
            xfprintf(stderr, "Identity added: %s\n", filename);
    } else {
        xfprintf(stderr, "Could not add identity: %s\n", filename);
    }
    if (comment) free(comment);
    key_free(private);
    ssh_close_authentication_connection(ac);
    return(success ? 0 : -1);
}

int
ssh_agent_list_identities(int do_fp)
{
    Key *key;
    char *comment, *fp;
    int had_identities = 0;
    int version;
    AuthenticationConnection *ac = NULL;

    /* At first, get a connection to the authentication agent. */
    ac = ssh_get_authentication_connection();
    if (ac == NULL) {
        ssh_start_agent();
        ac = ssh_get_authentication_connection();
    }
    if (ac == NULL) {
        xprintf("?Could not open a connection to authentication agent: ssh-agent.exe.\n");
        return(-1);
    }

    for (version = 1; version <= 2; version++) {
        for (key = ssh_get_first_identity(ac, &comment, version);
              key != NULL;
              key = ssh_get_next_identity(ac, &comment, version)) {
            had_identities = 1;
            if (do_fp) {
                fp = key_fingerprint(key, SSH_FP_MD5,
                                      SSH_FP_HEX);
                xprintf("%d %s %s (%s)\n",
                         key_size(key), fp, comment, key_type(key));
                free(fp);
            } else {
                if (!key_write(key, stdout))
                    xfprintf(stderr, "key_write failed");
                xfprintf(stdout, " %s\n", comment);
            }
            key_free(key);
            free(comment);
        }
    }
    if (!had_identities)
        xprintf("The agent has no identities.\n");
    ssh_close_authentication_connection(ac);
    return (0);
}
#endif /* SSHBUILTIN */
#endif /* CK_SSL */

#ifdef COMMENT
ssh [-l login_name] hostname | user@hostname [command]

ssh  [ - afgknqstvxACNPTX1246]  [ - b  bind_address]  [ -  c
                                                        cipher_spec]  [ - e  escape_char]  [ - i  identity_file] [-l
                                                                                                                   login_name] [-m mac_spec] [-o option] [-p port] [-F  config-
                                                                                                                                                                     file]  [ - L port:host:hostport] [-R port:host:hostport] [-D
                                                                                                                                                                                                                                port] hostname | user@hostname [command]


The options are as follows:

-a    Disables forwarding of the authentication  agent  con-
nection.

-A    Enables forwarding of the authentication agent connec-
tion.   This  can also be specified on a per-host basis
in a configuration file.

-b bind_address
Specify the interface to transmit from on machines with
multiple interfaces or aliased addresses.

-c blowfish|3des|des
Selects the cipher to use for encrypting  the  session.
3des  is used by default.  It is believed to be secure.
3des (triple-des) is an encrypt-decrypt-encrypt  triple
    with  three  different  keys.  blowfish is a fast block
    cipher, it appears very secure and is much faster  than
    3des.   des  is  only  supported  in the ssh client for
    interoperability with legacy protocol 1 implementations
    that  do  not  support  the  3des  cipher.   Its use is
    strongly discouraged due to cryptographic weaknesses.

    -c cipher_spec
    Additionally, for protocol version 2 a  comma-separated
    list   of   ciphers   can  be  specified  in  order  of
    preference.  See Ciphers for more information.

    -e ch|^ch|none
    Sets the escape  character  for  sessions  with  a  pty
    (default:  `~' ) .  The escape character is only recog-
      nized at the beginning of a line.  The escape character
      followed  by  a  dot (`.')  closes the connection, fol-
                             lowed by control-Z suspends the  connection,  and  fol-
                             lowed  by itself sends the escape character once.  Set-
                             ting the character to ``none'' disables any escapes and
                             makes the session fully transparent.

                             -f    Requests ssh to go to background just  before  command
                             execution.   This  is useful if ssh is going to ask for
                             passwords or passphrases, but the user wants it in  the
                             background.   This  implies -n.  The recommended way to
                             start X11 programs at a remote site is  with  something
                             like ssh -f host xterm.

                             -g    Allows remote hosts  to  connect  to  local  forwarded
                             ports.

                             -i identity_file
                             Selects a file from which the  identity  (private  key)
                             for  RSA or DSA authentication is read.  The default is
                             $HOME/.ssh/identity  for  protocol   version   1,   and
                             $HOME/.ssh/id_rsa  and  $HOME/.ssh/id_dsa  for protocol
                             version 2.  Identity files may also be specified  on  a
                             per-host basis in the configuration file.  It is possi-
                             ble to have multiple -i options (and  multiple  identi-
                                                               ties specified in configuration files).

                             -I smartcard_device
                             Specifies which smartcard device to use.  The  argument
                             is  the  device  ssh  should  use to communicate with a
                             smartcard used for storing the user's private RSA key.

                             - k    Disables  forwarding  of  Kerberos  tickets  and  AFS
                             tokens.  This may also be specified on a per-host basis
                             in the configuration file.

                             -l login_name
                             Specifies the user to log in as on the remote  machine.
                             This  also  may be specified on a per-host basis in the
                             configuration file.

                             -m mac_spec
                             Additionally, for protocol version 2 a  comma-separated
                             list  of  MAC  (message authentication code) algorithms
                             can be specified in order of preference.  See the  MACs
                             keyword for more information.

                             -n    Redirects stdin  from  /dev/null  (actually,  prevents
                                                                       reading from stdin).  This must be used when ssh is run
                             in the background.  A common trick is to  use  this  to
                             run X11 programs on a remote machine.  For example, ssh
                             -n shadows.cs.hut.fi emacs & will  start  an  emacs  on
                             shadows.cs.hut.fi,  and  the  X11  connection  will  be
                             automatically forwarded over an encrypted channel.  The
                             ssh  program will be put in the background.  (This does
                                                                            not work  if  ssh  needs  to  ask  for  a  password  or
                                                                            passphrase; see also the -f option.)

                             -N    Do not execute a remote command.  This is  useful  for
                             just forwarding ports (protocol version 2 only).

                             -o option
                             Can be used to give options in the format used  in  the
                             configuration  file.   This  is  useful  for specifying
                             options for which there  is  no  separate  command-line
                             flag.

                             -p port
                             Port to connect to on the remote  host.   This  can  be
                             specified  on  a  per-host  basis  in the configuration
                             file.

                             -P    Use a non-privileged port  for  outgoing  connections.
                             This  can be used if a firewall does not permit connec-
                             tions from privileged ports.   Note  that  this  option
                             turns off RhostsAuthentication and RhostsRSAAuthentica-
                             tion for older servers.

                             -q    Quiet mode.  Causes all warning  and  diagnostic  mes-
                             sages to be suppressed.

                             -s    May be used to request invocation of  a  subsystem  on
                             the remote system. Subsystems are a feature of the SSH2
                             protocol which facilitate the use of SSH  as  a  secure
                             transport  for  other applications (eg. sftp). The sub-
                             system is specified as the remote command.

                             -t    Force pseudo-tty allocation.  This can be used to exe-
                             cute   arbitrary  screen-based  programs  on  a  remote
                             machine, which can be very useful,  e.g.,  when  imple-
                             menting  menu  services.  Multiple -t options force tty
                             allocation, even if ssh has no local tty.

                             -T    Disable pseudo-tty allocation.

                             -v    Verbose mode.  Causes ssh to print debugging  messages
                             about  its progress.  This is helpful in debugging con-
                             nection, authentication,  and  configuration  problems.
                             Multiple  - v options increases the verbosity.  Maximum
                             is 3.

                             -x    Disables X11 forwarding.

                             -X    Enables X11 forwarding.  This can also be specified on
                             a per-host basis in a configuration file.

                             -C    Requests compression of  all  data  (including  stdin,
                                                                         stdout,  stderr,  and data for forwarded X11 and TCP/IP
                                                                         connections).  The compression algorithm  is  the  same
                             used by gzip(1), and the ``level'' can be controlled by
                             the CompressionLevel option (see  below).   Compression
                             is desirable on modem lines and other slow connections,
                             but will only slow down things on fast  networks.   The
                             default value can be set on a host-by-host basis in the
                             configuration files; see the Compression option below.

                             -F configfile
                             Specifies an alternative per-user  configuration  file.
                             If  a  configuration file is given on the command line,
                             the        system-wide        configuration        file
                             (/etc/ssh/ssh_config) will be ignored.  The default for
                             the per-user configuration file is $HOME/.ssh/config.

                             -L port:host:hostport
                             Specifies that the given port  on  the  local  (client)
                             host  is  to be forwarded to the given host and port on
                             the remote side.  This works by allocating a socket  to
                             listen  to  port on the local side, and whenever a con-
                             nection is made to this port, the  connection  is  for-
                             warded  over  the  secure  channel, and a connection is
                             made to host port hostport  from  the  remote  machine.
                             Port  forwardings  can  also be specified in the confi-
                             guration file.  Only root can forward privileged ports.
                             IPv6  addresses  can  be  specified with an alternative
                           syntax:  port/host/hostport

                             -R port:host:hostport
                             Specifies that the given port on  the  remote  (server)
                             host  is  to be forwarded to the given host and port on
                             the local side.  This works by allocating a  socket  to
                             listen  to port on the remote side, and whenever a con-
                             nection is made to this port, the  connection  is  for-
                             warded  over  the  secure  channel, and a connection is
                             made to host port  hostport  from  the  local  machine.
                             Port  forwardings  can  also be specified in the confi-
                             guration file.  Privileged ports can be forwarded  only
                             when  logging  in  as root on the remote machine.  IPv6
                             addresses can be specified with an alternative  syntax:
                             port/host/hostport

                             -D port
                             Specifies a local  ``dynamic''  application-level  port
                             forwarding.   This  works  by  allocating  a  socket to
                             listen to port on the local side, and whenever  a  con-
                             nection  is  made  to this port, the connection is for-
                             warded over the secure  channel,  and  the  application
                             protocol  is then used to determine where to connect to
                             from the remote machine.  Currently the SOCKS4 protocol
                             is  supported,  and  ssh  will  act as a SOCKS4 server.
                             Only root can forward privileged ports.   Dynamic  port
                             forwardings  can also be specified in the configuration
                             file.

                             -1    Forces ssh to try protocol version 1 only.

                             -2    Forces ssh to try protocol version 2 only.

                             -4    Forces ssh to use IPv4 addresses only.

                             -6    Forces ssh to use IPv6 addresses only.
#endif /* COMMENT */
