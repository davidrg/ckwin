char *cksshv = "SSH support (LibSSH), 10.0,  18 Apr 2023";
/*
 *  C K O L S S H . C --  LibSSH Subsystem Interface for C-Kermit
 *
 * Copyright (C) 2022, David Goodwin <david@zx.net.nz>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  + Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  + Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  + Neither the name of Columbia University nor the names of any
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is the interface between the SSH Subsystem and the rest of C-Kermit.
 * The SSH Subsystem lives off on its own thread because it's not usefully
 * thread safe. So the code here just interacts with the SSH Subsystem via
 * a pair of ring buffers, a mutex, and a collection of events.
 *
 * The interface is described in ckolsshs.h and the actual implementation
 * where all the work is done (using libssh) lives in ckolsshs.c.
 *
 * While libssh is supposedly OS/2-compatible, the current implementation
 * in ckolsshs.{ch} and ckolssh.c is Windows-specific.
 *
 * This SSH subsystem can either be compiled into C-Kermit, or compiled
 * as a runtime-loadable DLL (k95ssh.dll) by defining SSH_DLL at build time.
 */

#include <libssh/libssh.h>

#include "ckcdeb.h"
#include "ckolssh.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckolsshs.h"
#include "ckossh.h"

/* for FamilyLocal, FamilyInternet, etc */
#include "ckctel.h"

#ifndef SSH_DLL
#include "ckclib.h"
#include "ckoreg.h"
#endif


/* Global Variables:
 *   These used to be all declared in ckuus3.c around like 8040, but since
 *   the SSH_DLL refactoring they now live here and are no longer global.
 *
 *   Typ   Name       Def     Description
 *   ----- ---------- ------- -----------------------------------------------
 *   int   ssh_cas    0       Command-as-subsystem. Request the subsystem named
 *                            by ssh_cmd instead of a PTY or running a command.
 *   int   ssh_cfg    0       Use OpenSSH config? 0=no, 1=yes
 *   int   ssh_cmp    1       Use compression? yes/no
 *   int   ssh_gsd    0       GSSAPI Delegate Credentials (0=off)
 *   int   ssh_shk    2       Strict host key checking. Options are no/yes/ask
 *                            Set via "set ssh strict-host-key-check x"
 *   int   ssh_ver    0       SSH version - 0=auto, 1=1, 2=2
 *   int   ssh_vrb    2       Report Errors - Verbosity Level.
 *                            Set via "set ssh verbose x", range 0-7
 *   char* ssh_hst    NULL    Hostname
 *   char* ssh_prt    NULL    port/service
 *   char* ssh_cmd    NULL
 *   char* ssh2_unh   NULL    SSH-2 User Known Hosts file
 *   char* ssh2_gnh   NULL    SSH-2 Global Known Hosts file
 *   int   pwflg      0       Password has been supplied (/password:)
 *   int   ssh_hbt    0       Heartbeat (keepalive) setting
 *   int   tcp_nodelay 0      Enable/disable nagle algorithm
 *   char* pwbuf      "\0"    Supplied password
 *   char* uidbuf     ""      Supplied username (if any)
 *   char* ssh2_auth  NULL    Comma-separated list of allowed auth methods
 *   char* ssh2_cif   NULL    Comma-separated list of SSH v2 ciphers allowed
 *   char* ssh2_hka   NULL    Comma-separated list of host key algorithms
 *   char* ssh2_mac   NULL    Comma-separated list of MACs
 *   char* ssh2_kex   NULL    Comma-separated list of key exchange methods
 *   char* ssh_pxc    NULL    Proxy Command
 *
 * Unused Global Variables:
 *   ssh_afw, ssh_xfw, ssh_prp, ssh_shh, ssh_chkip,
 *   ssh_gwp, ssh_dyf, ssh_k4tgt, ssh_k5tgt, ssh2_ark,
 *   ssh_gkx, ssh_k5_is_k4
 *
 *   ssh_xal (xauth location)
 *
 * SSH Logging ("set ssh verbose x", ssh_vrb) levels:
 *  0   SSH_LOG_NOLOG       No logging at all
 *  1   SSH_LOG_WARNING     Only warnings
 *  2   SSH_LOG_PROTOCOL    Log high level protocol information
 *  3   SSH_LOG_PACKET      Lower level protocol information, packet level
 *  4   SSH_LOG_FUNCTIONS   Every function path
 *  5   -- not used --
 *  6   -- not used --      (not used = SSH_LOG_FUNCTIONS)
 *  7   -- not used --
 */

/* SSH Commands:
 *   SSH [OPEN] host [port]
 *      host: hostname or IP to connect to
 *            Saved in ssh_hst.
 *      port: port name or number to connect on.
 *            Saved in ssh_prt.
 *      /COMMAND:command
 *          Command to execute instead of your default shell
 *          Value is saved in ssh_cmd if specified
 *      /USER:user
 *          Defaults to \v(userid)
 *          Value saved in uidbuf
 *      /PASSWORD:pass
 *          Value saved in pwbuf if specified
 *      /VERSION:{1,2}
 *          Just reports an error if version is 1 (SSH-1 not supported)
 *          value is saved in ssh_ver. 0=auto.
 *      /SUBSYSTEM:name
 *      /X11-FORWARDING: {on,off}
 *   SSH ADD
 *      LOCAL-PORT-FORWARD local-port host port
 *      REMOTE-PORT-FORWARD remote-port host port
 *   TODO: SSH AGENT
 *      TODO: ADD identity-file
 *      TODO: DELETE identity-file
 *      TODO: LIST
 *          TODO: /FINGERPRINT
 *  SSH CLEAR
 *      LOCAL-PORT-FORWARD
 *      REMOTE-PORT-FORWARD
 *   SSH KEY
 *      CHANGE-PASSPHRASE
 *          /NEW-PASSPHRASE:passphrase
 *          /OLD-PASSPHRASE:passphrase
 *          filename
 *      CREATE
 *          /BITS:bits
 *          /PASSPHRASE: passphrase
 *          /TYPE:{ DSS, ECDSA, ED25519, RSA  }
 *          filename
 *      DISPLAY
 *          /FORMAT:{fingerprint, openssh, ssh.com}
 *          filename
 *   SET SSH
 *      TODO: AGENT-FORWARDING {ON,OFF}
 *      TODO: CHECK-HOST-IP {ON,OFF}
 *      COMPRESSION {ON,OFF}
 *          Value stored in ssh_cmp
 *      TODO: DYNAMIC-FORWARDING {ON,OFF}
 *      TODO: GATEWAY-PORTS {ON,OFF}
 *      GSSAPI DELEGATE-CREDENTIALS {ON,OFF}
 *          Value is stored in ssh_gsd
 *      HEARTBEAT-INTERVAL interval
 *      IDENTITY-FILE filename
 *      TODO: PRIVILEGED-PORT {ON,OFF}
 *      TODO: QUIET {ON,OFF}
 *          -> This should suppress all printfs
 *      STRICT-HOST-KEY-CHECK {ASK, ON, OFF}
 *      USE-OPENSSH-CONFIG {ON,OFF}
 *          Value is stored in ssh_cfg
 *      V2 AUTHENTICATION {EXTERNAL-KEYX, GSSAPI, HOSTBASED, KEYBOARD-INTERACTIVE, PASSWORD, PUBKEY, SRP-GEX-SHA1}
 *          Value stored in ssh2_auth as a comma-separated list
 *          We can support (eventually in some cases):
 *              GSSAPI, KEYBOARD-INTERACTIVE, PASSWORD, PUBKEY
 *          Not supported by libssh:
 *              EXTERNAL-KEYX, HOSTBASED, SRP-GEX-SHA1
 *      TODO: V2 Auto-rekey {on, off}
 *      V2 CIPHERS {3des-cbc, aes128-cbc, aes192-cbc, aes256-cbc, chachae20-poly1305, aes256-gcm@openssh.com, aes128-gcm@openssh.com, aes256-ctr, aes192-ctr, aes128-ctr}
 *      V2 GLOBAL-KNOWN-HOSTS-FILE filename
 *          Stored in ssh2_gnh
 *      V2 HOSTKEY-ALGORITHMS {ssh-ed25519, ecdsa-sha2-nistp256, ecdsa-sha2-nistp384, ecdsa-sha2-nistp521, ssh-rsa, rsa-sha2-512, rsa-sha2-256,ssh-ds}
 *           Stored in ssh2_hka
 *      V2 KEY-EXCHANGE-METHODS {...}
 *      V2 MACS {hmac-sha1, hmac-sha2-256-etm@openssh.com, hmac-sha2-512-etm@openssh.com, hmac-sha1-etm@openssh.com, hmac-sha2-512, hmac-sha2-256,  none}
 *          Stored in ssh2_mac
 *      V2 USER-KNOWN-HOSTS-FILE filename
 *          Stored in ssh2_unh
 *      VERBOSE level
 *          Report Errors - Verbosity Level. Range 0-7. Value stored in ssh_vrb
 *      VERSION {2, AUTOMATIC}
 *          value is saved in ssh_ver. 0=auto.
 *      X11-FORWARDING {ON, OFF}
 *          SET TELNET ENV DISPLAY is used to set the DISPLAY value
 *      TODO: XAUTH-LOCATION filename
 */

/* Features libssh provides that might be nice to support in the future:
 *   * Acting as a server (let iksd run over ssh rather than telnet)
 */

/* More TODO:
 *  - TODO: Other Settings
 *  - TODO: How do we know /command: has finished? EOF?
 *  - TODO: Build libssh with GSSAPI, pthreads and kerberos
 *          https://github.com/jwinarske/pthreads4w
 *  - TODO: Deal with flush
 *  - TODO: Deal with break
 *  - TODO: SFTP
 *      -DSFTP_BUILTIN
 *  - TODO: HTTP Proxying - this was something the previous implementaiton could
 *          handle?
  * - TODO: deal with changing terminal type after connect ? (K95 doesn't)
 */


/* The TO-DO list from the Kermit 95 2.1.3 SSH module:
  . Kerberos 4 and 5 Auto-get/auto-destroy support

  . Improve the debugging info to the debug log

  . listen to SET TCP REVERSE-DNS-LOOKUPS

  . listen to SET TCP DNS-SRV-RECORDS

  . make SET TELNET ENV DISPLAY more portable or SSH equivalent

  . SET SSH V2 AUTO-REKEY functionality

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
 */

/* ==== LibSSH Settings ====
 *
 * Settings we could use now:
 *  SSH_OPTIONS_ADD_IDENTITY        Add new identity file to the identity list
 *  SSH_OPTIONS_STRICTHOSTKEYCHECK  Strict host key check
 *      -> Command exists, just don't know how it lines up with this setting.
 *
 * Settings we could use with UI changes (new commands or updated options):
 *  SSH_OPTIONS_SSH_DIR             Directory where know_hosts and identity files live
 *  SSH_OPTIONS_COMPRESSION_LEVEL   Compression level
 *  SSH_OPTIONS_HMAC_C_S            Set message authentication code algo client to server
 *  SSH_OPTIONS_HMAC_S_C            Set message authentication code algo server to client
 *  SSH_OPTIONS_KEY_EXCHANGE        Set key exchange methods
 *
 * Settings we probably don't care about:
 *  SSH_OPTIONS_FD                  To supply our own socket if we want
 *  SSH_OPTIONS_BINDADDR            Bind address
 *  SSH_OPTIONS_TIMEOUT             Set connection timeout in seconds
 *  SSH_OPTIONS_PUBLICKEY_ACCEPTED_TYPES    Set preferred public key algorithms
 *  SSH_OPTIONS_PROXYCOMMAND        Set the commend to be executed to connect to server
 *  SSH_OPTIONS_GSSAPI_SERVER_IDENTITY
 *  SSH_OPTIONS_GSSAPI_CLIENT_IDENTITY
 *  SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS
 *  SSH_OPTIONS_NODELAY
 *  SSH_OPTIONS_REKEY_DATA
 *  SSH_OPTIONS_REKEY_TIME
 */

/*
 * Keyword tables for some of the "set ssh" options - these are the tables whose
 * values are most likely to vary from one SSH implementation to another.
 *
 * These used to live in ckuus3.c
 */

/*
#define SSHA_CRS   1
#define SSHA_DSA   2
*/
#define SSHA_GSS   3
#define SSHA_HOS   4
#define SSHA_KBD   5
/*
#define SSHA_K4    6
#define SSHA_K5    7
*/
#define SSHA_PSW   8
#define SSHA_PK    9
/*
#define SSHA_SKE  10
#define SSHA_TIS  11
*/
#define SSHA_EXT  12
#define SSHA_SRP  13

static struct keytab ssh2aut[] = {      /* SET SSH V2 AUTH command table */
        /*{ "external-keyx",      SSHA_EXT, 0 },*/
        { "gssapi",             SSHA_GSS, 0 },  /* TODO: Hide if not supported */
        /*{ "hostbased",          SSHA_HOS, 0 },*/
        { "keyboard-interactive",  SSHA_KBD, 0 },
        { "password",           SSHA_PSW, 0 },
        { "publickey",          SSHA_PK,  0 },
        /*{ "srp-gex-sha1",       SSHA_SRP, 0 },*/
        { "", 0, 0 }
};
static int nssh2aut = (sizeof(ssh2aut) / sizeof(struct keytab)) - 1;

#ifdef SSH_V1
/*#define SSHC_3DES 1*/                     /* 3DES */
#endif /* SSH_V1 */
#define SSHC_3CBC 2                     /* 3DES-CBC */
#define SSHC_A128 3                     /* AES128-CBC */
#define SSHC_A192 4                     /* AES192-CBC */
#define SSHC_A256 5                     /* AES256-CBC */
/*#define SSHC_ARC4 6*/                     /* ARCFOUR */
#ifdef SSH_V1
#define SSHC_FISH 7                     /* BLOWFISH */
#endif /* SSH_V1 */
/*#define SSHC_C128 8*/                     /* CAST128-CBC */
/*#define SSHC_BCBC 9*/                     /* BLOWFISH-CBC */
#ifdef SSH_V1
#define SSHC_1DES 10                    /* DES */
#endif /* SSH_V1 */
#define SSHC_CHPO 11                    /* chachae20-poly1305 */
#define SSHC_A1GC 12                    /* aes128-gcm@openssh.com */
#define SSHC_A2GC 13                    /* aes256-gcm@openssh.com */
#define SSHC_A12C 14                    /* aes128-ctr */
#define SSHC_A19C 15                    /* aes192-ctr */
#define SSHC_A25C 16                    /* aes256-ctr */


static struct keytab ssh2ciphers[] = {  /* SET SSH V2 CIPHERS command table */
        { "3des-cbc",        SSHC_3CBC, 0 },
        { "aes128-cbc",      SSHC_A128, 0 },
        { "aes192-cbc",      SSHC_A192, 0 },
        { "aes256-cbc",      SSHC_A256, 0 },
        /*{ "arcfour",         SSHC_ARC4, 0 },
        { "blowfish-cbc",    SSHC_FISH, 0 },
        { "cast128-cbc",     SSHC_C128, 0 },
        { "rijndael128-cbc", SSHC_A128, 0 },
        { "rijndael192-cbc", SSHC_A192, 0 },
        { "rijndael256-cbc", SSHC_A256, 0 },*/
        { "aes128-ctr", SSHC_A12C, 0 },
        { "aes192-ctr", SSHC_A19C, 0 },
        { "aes256-ctr", SSHC_A25C, 0 },
        { "aes128-gcm@openssh.com", SSHC_A1GC, 0 },
        { "aes256-gcm@openssh.com", SSHC_A2GC, 0 },
        { "chachae20-poly1305", SSHC_CHPO, 0 },
        { "", 0, 0 }
};
static int nssh2ciphers = (sizeof(ssh2ciphers) / sizeof(struct keytab)) - 1;

#ifdef SSH_V1
/* SSH V1 support - disabled for now as libssh doesn't support it */
static struct keytab ssh1ciphers[] = {
    { "3des",         SSHC_3DES, 0 },
    { "blowfish",     SSHC_FISH, 0 },
    { "des",          SSHC_1DES, 0 },
    { "", 0, 0 }
};
static int nssh1ciphers = (sizeof(ssh1ciphers) / sizeof(struct keytab)) - 1;
#endif /* SSH_V1 */

#define SSHM_SHA        1               /* HMAC-SHA1 */
#ifdef COMMENT
#define SSHM_SHA_96     2               /* HMAC-SHA1-96 */
#define SSHM_MD5        3               /* HMAC-MD5 */
#define SSHM_MD5_96     4               /* HMAC-MD5-96 */
#define SSHM_RIPE       5               /* HMAC-RIPEMD160 */
#endif
#define SSHM_SHA1_ETM   6               /* hmac-sha1-etm@openssh.com */
#define SSHM_SHA2_256   7               /* hmac-sha2-256 */
#define SSHM_SHA2_2ETM  8               /* hmac-sha2-256-etm@openssh.com */
#define SSHM_SHA2_512   9               /* hmac-sha2-512 */
#define SSHM_SHA2_5ETM  10              /* hmac-sha2-512-etm@openssh.com */
#define SSHM_NONE       11              /* none */

static struct keytab ssh2macs[] = {     /* SET SSH V2 MACS command table */
        /*
         { "hmac-md5",       SSHM_MD5,    0 },
         { "hmac-md5-96",    SSHM_MD5_96, 0 },
         { "hmac-ripemd160", SSHM_RIPE,   0 },*/
        { "hmac-sha1",      SSHM_SHA,    0 },
        /*{ "hmac-sha1-96",   SSHM_SHA_96, 0 },*/
        { "hmac-sha1-etm@openssh.com",      SSHM_SHA1_ETM,    0 },
        { "hmac-sha2-256",                  SSHM_SHA2_256,    0 },
        { "hmac-sha2-256-etm@openssh.com",  SSHM_SHA2_2ETM,    0 },
        { "hmac-sha2-512",                  SSHM_SHA2_512,    0 },
        { "hmac-sha2-512-etm@openssh.com",  SSHM_SHA2_5ETM,    0 },
        { "none",                           SSHM_NONE,    0 },
        { "", 0, 0 }
};
static int nssh2macs = (sizeof(ssh2macs) / sizeof(struct keytab)) - 1;

#define HKA_RSA 1
#define HKA_DSS 2
#define HKA_EC2 3
#define HKA_EC3 4
#define HKA_EC5 5
#define HKA_ED2 6
#define HKA_S22 7
#define HKA_S25 8

static struct keytab hkatab[] = {
        { "ecdsa-sha2-nistp256", HKA_EC2, 0, },
        { "ecdsa-sha2-nistp384", HKA_EC3, 0, },
        { "ecdsa-sha2-nistp521", HKA_EC5, 0, },
        { "rsa-sha2-256", HKA_S22, 0, },
        { "rsa-sha2-512", HKA_S25, 0, },
        { "ssh-dss", HKA_DSS, 0, },
        { "ssh-ed25519", HKA_ED2, 0, },
        { "ssh-rsa", HKA_RSA, 0, },
        { "", 0, 0 }
};
static int nhkatab = (sizeof(hkatab) / sizeof(struct keytab)) - 1;

static struct keytab sshkextab[] = {
        { "curve25519-sha256",              1, 0, },
        { "curve25519-sha256@libssh.org",   2, 0, },
        { "diffie-hellman-group1-sha1",     3, 0, },
        { "diffie-hellman-group14-sha1",    4, 0, },
        { "diffie-hellman-group14-sha256",  5, 0, },
        { "diffie-hellman-group16-sha512",  6, 0, },
        { "diffie-hellman-group18-sha512",  7, 0, },
        { "diffie-hellman-group-exchange-sha1",   8, 0, },
        { "diffie-hellman-group-exchange-sha256", 9, 0, },
        { "ecdh-sha2-nistp256",             10, 0, },
        { "ecdh-sha2-nistp384",             11, 0, },
        { "ecdh-sha2-nistp521",             12, 0, },
        /*{ "ext-info-c",                     13, 0, },*/
        { "", 0, 0 }
};
static int nsshkextab = (sizeof(sshkextab) / sizeof(struct keytab)) - 1;

/* Global variables */

static int                                     /* SET SSH variables */
    ssh_afw = 0,                          /* agent forwarding */
    ssh_xfw = 0,                          /* x11 forwarding   */
    ssh_prp = SET_OFF,                    /* privileged ports */
    ssh_cmp = 1,                          /* compression */
    ssh_cas = 0,                          /* command as subsys */
    ssh_shh = 0,                          /* quiet       */
    ssh_ver = 0,                          /* protocol version (auto,1,2) */
    ssh_vrb = 2,                          /* Report errors */
    ssh_chkip = 0,                        /* SSH Check Host IP flag */
    ssh_gwp = 0,                          /* gateway ports */
    ssh_dyf = 0,                          /* dynamic forwarding */
    ssh_gsd = 0,                          /* gssapi delegate credentials */
    ssh_k4tgt = 0,                        /* k4 tgt passing */
    ssh_k5tgt = 0,                        /* k5 tgt passing */
    ssh_shk = 2,                          /* Strict host key (no, yes, ask) */
    ssh2_ark = 1,                         /* Auto re-key */
    ssh_cfg = 0,                          /* use OpenSSH config? */
    ssh_gkx = 1,                          /* gssapi key exchange */
    ssh_k5_is_k4 = 1,                     /* some SSH v1 use same codes */
    ssh_hbt = 0,                          /* heartbeat (seconds) */
    ssh_dummy = 0;                        /* bottom of list */

static char                             /* The following are to be malloc'd */
    * ssh1_cif = NULL,                    /* v1 cipher */
    * ssh2_cif = NULL,                    /* v2 cipher list */
    * ssh2_mac = NULL,                    /* v2 mac list */
    * ssh2_auth = NULL,                   /* v2 authentication list */
    * ssh2_hka = NULL,                    /* Host Key Algorithms */
    * ssh_hst = NULL,                     /* hostname */
    * ssh_prt = NULL,                     /* port/service */
    * ssh_cmd = NULL,                     /* command to execute */
    * ssh_xal = NULL,                     /* xauth-location */
    * ssh1_gnh = NULL,                    /* v1 global known hosts file */
    * ssh1_unh = NULL,                    /* v1 user known hosts file */
    * ssh2_gnh = NULL,                    /* v2 global known hosts file */
    * ssh2_unh = NULL,                    /* v2 user known hosts file */
    * ssh2_kex = NULL,                    /* Key Exchange Methods */
    * ssh_pxc = NULL,                     /* Proxy command */
    * ssh_dir = NULL,                     /* SSH Directory */
    * xxx_dummy = NULL;

static const char **ssh_idf = NULL;                    /* Identity files */

/* The SSH subsystem actually tracks port forwards with a linked list so it has
 * no particular limit on the number it will support, but we need to be able to
 * track these things separately from the SSH subsystem to allow them to be set
 * up before the connection is made, and for now a simple array is easier than
 * a linked list.
 *
 * Why 65?
 *  Because Kermit 95 2.1.3 allowed 32 local forwardings and 32 remote
 *  forwardings, so a limit of 64 means all previously valid set of forwardings
 *  will sill be valid here. And one more to terminate the list makes 65.
 */
#define MAX_PORT_FORWARDS 65

/*
 * Array of port forwards. The array is null-terminated (the last entry has
 * type set to SSH_PORT_FORWARD_NULL) to make passing it around easier.
 */
static ssh_port_forward_t *port_forwards = NULL;

/* Local variables */
ssh_client_t *ssh_client = NULL;  /* Interface to the ssh subsystem */
HANDLE hSSHClientThread = NULL;   /* SSH subsystem thread */

#ifdef SSH_DLL

void ssh_initialise();

static void (*p_get_current_terminal_dimensions)(int* rows, int* cols) = NULL;
const char* (*p_get_current_terminal_type)() = NULL;
const char* (*p_ssh_get_uid)();
const char* (*p_ssh_get_pw)();
int (*p_ssh_get_nodelay_enabled)();
static int (*p_dodebug)(int,char *,char *,CK_OFF_T)=NULL;
static int (*p_vscrnprintf)(const char *, ...)=NULL;
static int (*p_uq_txt)(char *,char *,int,char **,char *,int,char *,int) = NULL;
static int (*p_uq_mtxt) (char *,char **,int,struct txtbox[]) = NULL;
int (*p_uq_ok)(char *,char *,int,char **,int) = NULL;
int (*p_uq_file)(char *,char *,int,char **,char *,char *,int) = NULL;
int (*p_zmkdir)(char *) = NULL;
int (*p_ckmakxmsg)(char * buf, int len, char *s1, char *s2, char *s3,
        char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
        char *s10, char *s11, char *s12) = NULL;
char* (*p_whoami)() = NULL;
char* (*p_GetAppData)(int common) = NULL;
char* (*p_GetHomePath)() = NULL;
char* (*p_GetHomeDrive)() = NULL;
int (*p_ckstrncpy)(char * dest, const char * src, int len) = NULL;
int (*p_debug_logging)() = NULL;
unsigned char* (*p_get_display)() = NULL;
int (*p_parse_displayname)(char *displayname, int *familyp, char **hostp,
                        int *dpynump, int *scrnump, char **restp) = NULL;

void get_current_terminal_dimensions(int* rows, int* cols) {
    p_get_current_terminal_dimensions(rows, cols);
}

const char* get_current_terminal_type() {
    return p_get_current_terminal_type();
}

const char* ssh_get_uid() {
    return p_ssh_get_uid();
}

const char* ssh_get_pw() {
    return p_ssh_get_pw();
}

int ssh_get_nodelay_enabled() {
    return p_ssh_get_nodelay_enabled();
}

int dodebug(int flag, char * s1, char * s2, CK_OFF_T n)
{
    if ( p_dodebug )
        return(p_dodebug(flag,s1,s2,n));
    else
        return(-1);
}

static char myprtfstr[4096];
int Vscrnprintf(const char * format, ...) {
    int i, len, rc=0;
    char *cp;
    va_list ap;

    va_start(ap, format);
#ifdef NT
    rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
    rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
    va_end(ap);

    if ( p_vscrnprintf )
        return(p_vscrnprintf(myprtfstr));
    else
        return(-1);
}

int uq_txt(char * preface, char * prompt, int echo, char ** help, char * buf,
       int buflen, char *dflt, int timer) {
    return p_uq_txt(preface, prompt, echo, help, buf, buflen, dflt, timer);
}

int uq_mtxt(char * preface,char **help, int n, struct txtbox field[]) {
    return p_uq_mtxt(preface, help, n, field);
}

int uq_ok(char * preface, char * prompt, int mask,char ** help, int dflt) {
    return p_uq_ok(preface, prompt, mask, help, dflt);
}

int uq_file(char * preface, char * fprompt, int fc, char ** help,
	char * dflt, char * result, int rlength) {
    return p_uq_file(preface, fprompt, fc, help, dflt, result, rlength);
}


int zmkdir(char *path) {
    return p_zmkdir(path);
}

int ckmakxmsg(char * buf, int len, char *s1, char *s2, char *s3,
        char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
        char *s10, char *s11, char *s12) {
    return p_ckmakxmsg(buf, len, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
                       s12);
}

#define ckmakmsg(buf,len,s1,s2,s3,s4) ckmakxmsg(buf, len, s1, s2, s3, s4, \
            NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL)

char* whoami() {
    return p_whoami();
}

char* GetAppData(int common) {
    return p_GetAppData(common);
}

char* GetHomePath() {
    return p_GetHomePath();
}

char* GetHomeDrive() {
    return p_GetHomeDrive();
}

int ckstrncpy(char * dest, const char * src, int len) {
    return p_ckstrncpy(dest, src, len);
}

int debug_logging() {
    return p_debug_logging();
}

unsigned char* get_display() {
    return p_get_display();
}

int parse_displayname(char *displayname, int *familyp, char **hostp,
                        int *dpynump, int *scrnump, char **restp) {
    return p_parse_displayname(displayname, familyp, hostp,
                               dpynump, scrnump, restp);
}

#undef malloc
#undef realloc
#undef free
#undef strdup

static void
fatal(char *msg) {
    if (!msg) msg = "";

    printf(msg);
    exit(1);        /* Exit indicating failure */
}

void *
kmalloc(size_t size)
{
    void *ptr;

    if (size == 0) {
        fatal("kmalloc: zero size");
    }
    ptr = malloc(size);
    if (ptr == NULL) {
        fatal("kmalloc: out of memory");
    }
    return ptr;
}

void *
krealloc(void *ptr, size_t new_size)
{
    void *new_ptr;

    if (new_size == 0) {
        fatal("krealloc: zero size");
    }
    if (ptr == NULL)
        new_ptr = malloc(new_size);
    else
        new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) {
        fatal("krealloc: out of memory");
    }
    return new_ptr;
}

void
kfree(void *ptr)
{
    if (ptr == NULL) {
        printf("kfree: NULL pointer given as argument");
        return;
    }
    free(ptr);
}

char *
kstrdup(const char *str)
{
    size_t len;
    char *cp;

    if (str == NULL) {
        fatal("kstrdup: NULL pointer given as argument");
    }
    len = strlen(str) + 1;
    cp = kmalloc(len);
    if (cp)
        memcpy(cp, str, len);
    return cp;
}

/*
 * Quick macro to check if a function pointer is null and, if it is, log
 * the event and return an error.
 */
#define STR(x) #x
#define CHECK_FP(fp) if (fp == NULL) { \
    debug(F110, "ERROR: Required function pointer is null", STR(fp), 0); \
    return -1;}

int ssh_dll_init(ssh_init_parameters_t *params) {
    /* Store pointers to helper functions provided by K95 */
    p_get_current_terminal_dimensions = params->p_get_current_terminal_dimensions;
    CHECK_FP(p_get_current_terminal_dimensions)
    p_get_current_terminal_type = params->p_get_current_terminal_type;
    CHECK_FP(p_get_current_terminal_type)
    p_ssh_get_uid = params->p_ssh_get_uid;
    CHECK_FP(p_ssh_get_uid)
    p_ssh_get_pw = params->p_ssh_get_pw;
    CHECK_FP(p_ssh_get_pw)
    p_ssh_get_nodelay_enabled = params->p_ssh_get_nodelay_enabled;
    CHECK_FP(p_ssh_get_nodelay_enabled)
    p_dodebug = params->p_dodebug;
    CHECK_FP(p_dodebug)
    p_vscrnprintf = params->p_vscrnprintf;
    CHECK_FP(p_vscrnprintf)
    p_uq_txt = params->p_uq_txt;
    CHECK_FP(p_uq_txt)
    p_uq_mtxt = params->p_uq_mtxt;
    CHECK_FP(p_uq_mtxt)
    p_uq_ok = params->p_uq_ok;
    CHECK_FP(p_uq_ok)
    p_uq_file = params->p_uq_file;
    CHECK_FP(p_uq_file)
    p_zmkdir = params->p_zmkdir;
    CHECK_FP(p_zmkdir)
    p_ckmakxmsg = params->p_ckmakxmsg;
    CHECK_FP(p_ckmakxmsg)
    p_whoami = params->p_whoami;
    CHECK_FP(p_whoami)
    p_GetAppData = params->p_GetAppData;
    CHECK_FP(p_GetAppData)
    p_GetHomePath = params->p_GetHomePath;
    CHECK_FP(p_GetHomePath)
    p_GetHomeDrive = params->p_GetHomeDrive;
    CHECK_FP(p_GetHomeDrive)
    p_ckstrncpy = params->p_ckstrncpy;
    CHECK_FP(p_ckstrncpy)
    p_debug_logging = params->p_debug_logging;
    CHECK_FP(p_debug_logging)
    p_get_display = params->p_get_display;
    CHECK_FP(p_get_display)
    p_parse_displayname = params->p_parse_displayname;
    CHECK_FP(p_get_display)

    /* And then supply pointers to all our functions to K95 */
    params->p_install_funcs("ssh_set_iparam", ssh_set_iparam);
    params->p_install_funcs("ssh_get_iparam", ssh_get_iparam);
    params->p_install_funcs("ssh_set_sparam", ssh_set_sparam);
    params->p_install_funcs("ssh_get_sparam", ssh_get_sparam);
    params->p_install_funcs("ssh_set_identity_files", ssh_set_identity_files);
    params->p_install_funcs("ssh_get_socket", ssh_get_socket);
    params->p_install_funcs("ssh_open", ssh_open);
    params->p_install_funcs("ssh_clos", ssh_clos);
    params->p_install_funcs("ssh_tchk", ssh_tchk);
    params->p_install_funcs("ssh_flui", ssh_flui);
    params->p_install_funcs("ssh_break", ssh_break);
    params->p_install_funcs("ssh_inc", ssh_inc);
    params->p_install_funcs("ssh_xin", ssh_xin);
    params->p_install_funcs("ssh_toc", ssh_toc);
    params->p_install_funcs("ssh_tol", ssh_tol);
    params->p_install_funcs("ssh_snaws", ssh_snaws);
    params->p_install_funcs("ssh_proto_ver", ssh_proto_ver);
    params->p_install_funcs("ssh_impl_ver", ssh_impl_ver);
    params->p_install_funcs("sshkey_create", sshkey_create);
    params->p_install_funcs("sshkey_display_fingerprint", sshkey_display_fingerprint);
    params->p_install_funcs("sshkey_display_public", sshkey_display_public);
    params->p_install_funcs("sshkey_display_public_as_ssh2", sshkey_display_public_as_ssh2);
    params->p_install_funcs("sshkey_change_passphrase", sshkey_change_passphrase);
    params->p_install_funcs("ssh_fwd_remote_port", ssh_fwd_remote_port);
    params->p_install_funcs("ssh_fwd_local_port", ssh_fwd_local_port);
    params->p_install_funcs("ssh_fwd_clear_remote_ports", ssh_fwd_clear_remote_ports);
    params->p_install_funcs("ssh_fwd_clear_local_ports", ssh_fwd_clear_local_ports);
    params->p_install_funcs("ssh_fwd_remove_remote_port", ssh_fwd_remove_remote_port);
    params->p_install_funcs("ssh_fwd_remove_local_port", ssh_fwd_remove_local_port);
    params->p_install_funcs("ssh_fwd_get_ports", ssh_fwd_get_ports);
#ifdef SSHTEST
    params->p_install_funcs("sshkey_v1_change_comment", sshkey_v1_change_comment); /* TODO */
#endif
    /* params->p_install_funcs("sshkey_default_file", sshkey_default_file); */ /* TODO */
    params->p_install_funcs("ssh_v2_rekey", ssh_v2_rekey); /* TODO */
    params->p_install_funcs("ssh_agent_delete_file", ssh_agent_delete_file); /* TODO */
    params->p_install_funcs("ssh_agent_delete_all", ssh_agent_delete_all); /* TODO */
    params->p_install_funcs("ssh_agent_add_file", ssh_agent_add_file); /* TODO */
    params->p_install_funcs("ssh_agent_list_identities", ssh_agent_list_identities); /* TODO */
#ifdef COMMENT
    /* Not supported: */
    params->p_install_funcs("ssh_unload", ssh_unload);
#endif /* COMMENT */
    params->p_install_funcs("ssh_dll_ver", ssh_dll_ver);
    params->p_install_funcs("ssh_get_keytab", ssh_get_keytab);
    params->p_install_funcs("ssh_feature_supported", ssh_feature_supported);

    /* And lastly do any other initialisation work that is independent of
     * whether we're a DLL or not */
    ssh_initialise();

    return 0;
}
#else
/* These live in ckossh.c */
unsigned char* get_display();
int parse_displayname(char *displayname, int *familyp, char **hostp,
                      int *dpynump, int *scrnump, char **restp);
#endif /* SSH_DLL */

int ssh_set_iparam(int param, int value) {
    switch(param) {
        case SSH_IPARAM_AFW:
            ssh_afw = value;
            break;
        case SSH_IPARAM_XFW:
            ssh_xfw = value;
            break;
        case SSH_IPARAM_PRP:
            ssh_prp = value;
            break;
        case SSH_IPARAM_CMP:
            ssh_cmp = value;
            break;
        case SSH_IPARAM_CAS:
            ssh_cas = value;
            break;
        case SSH_IPARAM_SHH:
            ssh_shh = value;
            break;
        case SSH_IPARAM_VER:
            ssh_ver = value;
            break;
        case SSH_IPARAM_VRB:
            ssh_vrb = value;
            break;
        case SSH_IPARAM_CHKIP:
            ssh_chkip = value;
            break;
        case SSH_IPARAM_GWP:
            ssh_gwp = value;
            break;
        case SSH_IPARAM_DYF:
            ssh_dyf = value;
            break;
        case SSH_IPARAM_GSD:
            ssh_gsd = value;
            break;
        case SSH_IPARAM_K4TGT:
            ssh_k4tgt = value;
            break;
        case SSH_IPARAM_K5TGT:
            ssh_k5tgt = value;
            break;
        case SSH_IPARAM_SHK:
            ssh_shk = value;
            break;
        case SSH_IPARAM_2_ARK:
            ssh2_ark = value;
            break;
        case SSH_IPARAM_CFG:
            ssh_cfg = value;
            break;
        case SSH_IPARAM_GKX:
            ssh_gkx = value;
            break;
        case SSH_IPARAM_K5_IS_K4:
            ssh_k5_is_k4 = value;
            break;
        case SSH_IPARAM_HBT:
            ssh_hbt = value;
            break;
        default:
            return 1;
    }
    return 0;
}

int ssh_get_iparam(int param) {
    switch(param) {
        case SSH_IPARAM_AFW:
            return ssh_afw;
        case SSH_IPARAM_XFW:
            return ssh_xfw;
        case SSH_IPARAM_PRP:
            return ssh_prp;
        case SSH_IPARAM_CMP:
            return ssh_cmp;
        case SSH_IPARAM_CAS:
            return ssh_cas;
        case SSH_IPARAM_SHH:
            return ssh_shh;
        case SSH_IPARAM_VER:
            return ssh_ver;
        case SSH_IPARAM_VRB:
            return ssh_vrb;
        case SSH_IPARAM_CHKIP:
            return ssh_chkip;
        case SSH_IPARAM_GWP:
            return ssh_gwp;
        case SSH_IPARAM_DYF:
            return ssh_dyf;
        case SSH_IPARAM_GSD:
            return ssh_gsd;
        case SSH_IPARAM_K4TGT:
            return ssh_k4tgt;
        case SSH_IPARAM_K5TGT:
            return ssh_k5tgt;
        case SSH_IPARAM_SHK:
            return ssh_shk;
        case SSH_IPARAM_2_ARK:
            return ssh2_ark;
        case SSH_IPARAM_CFG:
            return ssh_cfg;
        case SSH_IPARAM_GKX:
            return ssh_gkx;
        case SSH_IPARAM_K5_IS_K4:
            return ssh_k5_is_k4;
        case SSH_IPARAM_HBT:
            return ssh_hbt;
    }
    return 0;
}

void copy_set_sparam(char **dest, const char* src) {
    if (*dest) {
        free(*dest);
        *dest = NULL;
    }
    if (src) {
        *dest = _strdup(src);
    }
}

int ssh_set_sparam(int param, const char* value) {
    switch(param) {
        case SSH_SPARAM_1_CIF:
            break;
        case SSH_SPARAM_2_CIF:
            copy_set_sparam(&ssh2_cif, value);
            break;
        case SSH_SPARAM_2_MAC:
            copy_set_sparam(&ssh2_mac, value);
            break;
        case SSH_SPARAM_2_AUTH:
            copy_set_sparam(&ssh2_auth, value);
            break;
        case SSH_SPARAM_2_HKA:
            copy_set_sparam(&ssh2_hka, value);
            break;
        case SSH_SPARAM_HST:
            copy_set_sparam(&ssh_hst, value);
            break;
        case SSH_SPARAM_PRT:
            copy_set_sparam(&ssh_prt, value);
            break;
        case SSH_SPARAM_CMD:
            copy_set_sparam(&ssh_cmd, value);
            break;
        case SSH_SPARAM_XAL:
            copy_set_sparam(&ssh_xal, value);
            break;
        case SSH_SPARAM_1_GNH:
            break;
        case SSH_SPARAM_1_UNH:
            break;
        case SSH_SPARAM_2_GNH:
            copy_set_sparam(&ssh2_gnh, value);
            break;
        case SSH_SPARAM_2_UNH:
            copy_set_sparam(&ssh2_unh, value);
            break;
        case SSH_SPARAM_2_KEX:
            copy_set_sparam(&ssh2_kex, value);
            break;
        case SSH_SPARAM_PXC:
            copy_set_sparam(&ssh_pxc, value);
            break;
        case SSH_SPARAM_DIR:
            copy_set_sparam(&ssh_dir, value);
            break;
        default:
            return 1;
    }
    return 0;
}

const char* ssh_get_sparam(int param) {
    switch(param) {
        case SSH_SPARAM_1_CIF:
            return NULL;
        case SSH_SPARAM_2_CIF:
            return ssh2_cif;
        case SSH_SPARAM_2_MAC:
            return ssh2_mac;
        case SSH_SPARAM_2_AUTH:
            return ssh2_auth;
        case SSH_SPARAM_2_HKA:
            return ssh2_hka;
        case SSH_SPARAM_HST:
            return ssh_hst;
        case SSH_SPARAM_PRT:
            return ssh_prt;
        case SSH_SPARAM_CMD:
            return ssh_cmd;
        case SSH_SPARAM_XAL:
            return ssh_xal;
        case SSH_SPARAM_1_GNH:
            return NULL;
        case SSH_SPARAM_1_UNH:
            return NULL;
        case SSH_SPARAM_2_GNH:
            return ssh2_gnh;
        case SSH_SPARAM_2_UNH:
            return ssh2_unh;
        case SSH_SPARAM_2_KEX:
            return ssh2_kex;
        case SSH_SPARAM_PXC:
            return ssh_pxc;
        case SSH_SPARAM_DIR:
            return ssh_dir;
        default:
            return NULL;
    }
    return NULL;
}

/** Set the list of SSH identity files to use for authentication
 *
 * @param identity_files List of identity files, null terminated.
 * @returns 0 on success, -1 if not supported
 */
int ssh_set_identity_files(const char** identity_files) {
    /* TODO: We really *should* make a copy of this rather than
     *      just holding on to the pointer given to us by K95.
     *      Currently its "ok" as the array of identity files is
     *      statically allocated by K95, and there is little
     *      reason for this to change in the future.
     */
    ssh_idf = identity_files;
    return 0;
}

/** This is called by ssh_dll_init when the DLL is loaded (SSH_DLL defined)
 * or directly by K95 on application startup (SSH_DLL not defined) at the
 * point where the DLL would normally have been loaded.
 *
 * Here we can set defaults.
 */
void ssh_initialise() {

}


/* Similar to "show ssh"
 * TODO: Delete this once all the not-implemented functions are implemented */
static void debug_params(const char* function) {
    if (function) printf("Function: %s\n", function);
    printf("Integer params:\n");
    printf("\t  Agent forwarding: %d\t       X-11 forwarding: %d\n", ssh_afw, ssh_xfw);
    printf("\t   Privileged Port? %d\t       Use compression? %d\n", ssh_prp, ssh_cmp);
    printf("\t Command as subsys? %d\t                 Quiet? %d\n", ssh_cas, ssh_shh);
    printf("\t           Version: %d\t               Verbose? %d\n", ssh_ver, ssh_vrb);
    printf("\tCheck Host IP flag? %d\t         Gateway Ports: %d\n", ssh_chkip, ssh_gwp);
    printf("\tDynamic Forwarding: %d\t GSSAPI Delegate creds: %d\n", ssh_dyf, ssh_gsd);
    printf("\t    K4 TGT Passing: %d\t        K5 TGT Passing: %d\n", ssh_k4tgt, ssh_k5tgt);
    printf("\t   Strict host key: %d\t            Auto Rekey: %d\n", ssh_shk, ssh2_ark);
    printf("\tUse OpenSSH config: %d\t   GSSAPI key Exchange: %d\n", ssh_cfg, ssh_gkx);
    printf("\t          K5 is K4: %d\t             Heartbeat: %d\n", ssh_k5_is_k4, ssh_hbt);
    printf("\nString Params:\n");
    if (ssh1_cif)  printf("\tv1 cipher: %s\n", ssh1_cif);
    if (ssh2_cif)  printf("\tv2 cipher: %s\n", ssh2_cif);
    if (ssh2_mac)  printf("\tv2 MAC List: %s\n", ssh2_mac);
    if (ssh2_auth) printf("\tv2 Authentication List: %s\n", ssh2_auth);
    if (ssh2_hka)  printf("\tv2 Host Key Algorithm List: %s\n", ssh2_auth);
    if (ssh_hst)   printf("\tHostname: %s\n", ssh_hst);
    if (ssh_prt)   printf("\tPort/Service: %s\n", ssh_prt);
    if (ssh_cmd)   printf("\tCommand to Execute: %s\n", ssh_cmd);
    if (ssh_xal)   printf("\tXauth-location: %s\n", ssh_xal);
    if (ssh1_gnh)  printf("\tv1 Global known hosts file: %s\n", ssh1_gnh);
    if (ssh1_unh)  printf("\tv1 User known hosts file: %s\n", ssh1_unh);
    if (ssh2_gnh)  printf("\tv2 global known hosts file: %s\n", ssh2_gnh);
    if (ssh2_unh)  printf("\tv2 user known hosts file: %s\n", ssh2_unh);
    printf("-- end of debug output --\n");

    /* Write all the SSH config out to the debug log */
    debug(F100, "SSH connection parameters:-", "", 0);
    debug(F111, "Agent forwarding", "ssh_afw", ssh_afw);
    debug(F111, "X-11 forwarding", "ssh_xfw", ssh_xfw);
    debug(F111, "Privileged Port?", "ssh_prp", ssh_prp);
    debug(F111, "Use compression?", "ssh_cmp", ssh_cmp);
    debug(F111, "Command as subsys?", "ssh_cas", ssh_cas);
    debug(F111, "Quiet?", "ssh_shh", ssh_shh);
    debug(F111, "Version", "ssh_ver", ssh_ver);
    debug(F111, "Verbose?", "ssh_vrb", ssh_vrb);
    debug(F111, "Check Host IP flag?", "ssh_chkip", ssh_chkip);
    debug(F111, "Gateway Ports", "ssh_gwp", ssh_gwp);
    debug(F111, "Dynamic Forwarding", "ssh_dyf", ssh_dyf);
    debug(F111, "GSSAPI Delegate creds", "ssh_gsd", ssh_gsd);
    debug(F111, "K4 TGT Passing", "ssh_k4tgt", ssh_k4tgt);
    debug(F111, "K5 TGT Passing", "ssh_k5tgt", ssh_k5tgt);
    debug(F111, "Strict host key", "ssh_shk", ssh_shk);
    debug(F111, "Auto Rekey", "ssh2_ark", ssh2_ark);
    debug(F111, "Use OpenSSH config", "ssh_cfg", ssh_cfg);
    debug(F111, "GSSAPI key Exchange", "ssh_gkx", ssh_gkx);
    debug(F111, "K5 is K4", "ssh_k5_is_k4", ssh_k5_is_k4);
    debug(F111, "Heartbeat", "ssh_hbt", ssh_hbt);
    debug(F111, "v1 cipher (ssh1_cif)", ssh1_cif, 0);
    debug(F111, "v2 cipher (ssh2_cif)", ssh2_cif, 0);
    debug(F111, "v2 MAC List (ssh2_mac)", ssh2_mac, 0);
    debug(F111, "v2 Authentication List (ssh2_auth)", ssh2_auth, 0);
    debug(F111, "v2 Host Key Algorithm List (ssh2_auth)", ssh2_auth, 0);
    debug(F111, "Hostname (ssh_hst)", ssh_hst, 0);
    debug(F111, "Port/Service (ssh_prt)", ssh_prt, 0);
    debug(F111, "Command to Execute (ssh_cmd)", ssh_cmd, 0);
    debug(F111, "Xauth-location (ssh_xal)", ssh_xal, 0);
    debug(F111, "v1 Global known hosts file (ssh1_gnh)", ssh1_gnh, 0);
    debug(F111, "v1 User known hosts file (ssh1_unh)", ssh1_unh, 0);
    debug(F111, "v2 global known hosts file (ssh2_gnh)", ssh2_gnh, 0);
    debug(F111, "v2 user known hosts file (ssh2_unh)", ssh2_unh, 0);
    debug(F100, "/end SSH connection parameters", "", 0);
}


/** Get the socket currently in use by the SSH client.
 *
 * @returns Socket for the current SSH connection, or -1 if not implemented or
 *      no active connection
 */
int ssh_get_socket() {

    /* TODO: Get the libssh socket */

    return -1;
}

/** Checks that the SSH thread is alive and has not reported an error.
 *
 * @return Error status or 0 if everything is ok.
 */
static int get_ssh_error() {
    int error = SSH_ERR_UNSPECIFIED;

    /* If there is no ssh_client instance then the client is absolutely not
     * running (or we're about to crash) */
    if (ssh_client == NULL) {
        return SSH_ERR_NO_INSTANCE;
    }

    /* Check if the client thread is alive. If not we're disconnected, and we
     * can just return whatever its error state is */
    if (WaitForSingleObject(hSSHClientThread, 0) == WAIT_OBJECT_0) {
        /* Thread is not running - send back whatever status it set when it
         * stopped. No need to acquire the mutex.
         */

        int client_error, rc;

        debug(F100, "get_ssh_error() - SSH subsystem thread has terminated. Closing session", "", 0);
        client_error = ssh_client->error;
        rc = ssh_clos();
        if (rc != SSH_ERR_OK) {
            debug(F111, "get_ssh_error() - Error closing connection", "rc", rc);
            debug(F101, "get_ssh_error() - Thread error state is", "", ssh_client->error);
            return rc;
        }

        return client_error;
    }

    /* Thread is alive though its possible it may have signalled an error and
     * is waiting to be told to disconnect. */
    if (acquire_mutex(ssh_client->mutex, INFINITE)) {
        error = ssh_client->error;
        ReleaseMutex(ssh_client->mutex);
    }

    return error;
}

/* Returns the SSH directory in a new string. The string must be freed by
 * the caller.
 *
 * @returns A new string containing the SSH directory.
 */
char* ssh_directory() {

    char* dir;
    if (ssh_dir != NULL) {      /* SSH Directory */
        dir = _strdup(ssh_dir);
    } else {
        dir = malloc(sizeof(char)*MAX_PATH);

        /* \v(appdata)ssh/ */
        ckmakmsg(dir, MAX_PATH, GetAppData(0), "Kermit 95/", "ssh/", NULL);
    }
    return dir;
}

/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open() {
    ssh_parameters_t* parameters;
    char* user = NULL;
    int pty_height, pty_width;
    int rc;
    const char* uidbuf;
#define NHPATHMAX 1024
    char *unh = NULL, *gnh = NULL, *dir = NULL;

    /* X11 forwarding details */
    int display_number = 0, screen_number = 0;
    char *x11_host = NULL;

    uidbuf = ssh_get_uid();

    debug(F100, "ssh_open()", "", 0);

    if (ssh_ver == 1) {
        /* libssh doesn't support SSH-1 anymore, so we don't either */
        debug(F100, "ssh_open() - unsupported SSH version", "ssh_ver", ssh_ver);
        printf("SSH-1 is not supported - please use SSH-2\n");
        return SSH_ERR_UNSUPPORTED_VERSION;
    }

    /* Need a hostname to connect to a host... */
    if (ssh_hst == NULL) {
        debug(F100, "ssh_open() - error - hostname not specified", "", 0);
        debug(F100, "Error - host not specified (ssh_hst is null)", "", 0);
        return SSH_ERR_HOST_NOT_SPECIFIED;
    }

    if (strlen(uidbuf) == 0) {
        /* Username is not set - prompt for one */
        char tmp[BUFSIZ] = {'\0', '\0'};
        char prompt[256];
        char *myname;
        int ok;

        /* This returns a statically allocated char* so we don't need to free
         * the result when we're done with it */
        myname = whoami();

        if (!myname) {
            myname = "";
        }

        if (myname[0]) {
            ckmakxmsg(prompt, sizeof(prompt), " Name (", myname, "): ",
                      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        } else {
            strcpy(prompt, " Name: ");
        }

        ok = uq_txt("Username required for SSH\n", prompt, 1, NULL,
                    tmp, sizeof(tmp), NULL, 0);

        if (!ok || (*tmp == '\0')) {
            user = _strdup(myname);
        } else {
            user = _strdup(tmp);
        }
    } else {
        user = _strdup(uidbuf);
    }
    debug(F110, "username", user, 0);

    /* Check if the client thread is alive. If it is we'll need a successful
     * disconnect before we can proceed. */
    if (hSSHClientThread) {
        debug(F100, "ssh_open() - client thread handle exists - checking state", "", 0);
        if (WaitForSingleObject(hSSHClientThread, 0) != WAIT_OBJECT_0) {
            /* Client is still connected - disconnect */
            debug(F100, "ssh_open() - client thread is alive. Requesting disconnect.", "", 0);
            rc = ssh_clos();
            if (rc != SSH_ERR_NO_ERROR) {
                /* Failed to close the existing connection. Can't start a new one.*/
                return rc;
            }
        } else {
            /* Not running - close the handle */
            CloseHandle(hSSHClientThread);
            hSSHClientThread = NULL;
        }
    }

    debug(F100, "ssh_open() - get terminal dimensions", "", 0);
    get_current_terminal_dimensions(&pty_height, &pty_width);
    debug(F111, "ssh_open() - get terminal dimensions", "height", pty_height);
    debug(F111, "ssh_open() - get terminal dimensions", "width", pty_width);

    if (ssh_xfw) {
        unsigned char* display = get_display();
        char *rest = NULL;
        int family;

        debug(F110, "ssh_open() DISPLAY", display, 0);

        if (parse_displayname(display, &family, &x11_host, &display_number, &screen_number, &rest)) {
            /* We don't support unix domain sockets. Change it to the IP
             * loopback interface and hope it works. */
            if (family == FamilyLocal) {
                debug(F100, "ssh_open() - display proto is local, converting to IP", NULL, 0);
                family = FamilyInternet;
                if (x11_host)  {
                    free(x11_host);
                    x11_host = NULL;
                }

                x11_host = malloc(strlen("localhost") + 1);

                if (x11_host) {
                    strcpy(x11_host, "localhost");
                } else {
                    ssh_xfw = FALSE;    /* Can't connect to the specified display */
                }
            }

            debug(F110, "ssh_open() Host & display number", x11_host, display_number);

        } else {
            debug(F100, "ssh_open() failed to parse DISPLAY, disabling forwarding", NULL, 0);
            ssh_xfw = FALSE;
        }

        if (rest) free(rest);
    }

    /* Sort out default files and directories */
    if (ssh2_unh != NULL) {     /* SSHv2 User Known Hosts file */
        unh = _strdup(ssh2_unh);
    } else if (ssh_dir == NULL) {
        /* Set the default user known hosts file to
         * \v(appdata)ssh/known_hosts2 only if:
         *      -> The user has not specified the user known hosts file with the
         *         set ssh v2 user-known-hosts-file
         *      -> The user has not specified an SSH directory
         * This is to retain compatibility with Kermit 95 2.1.3 and earlier
         * which named the SSHv2 UNH file "known_hosts2" and the SSHv1 UNH file
         * "known_hosts". If the user changes their SSH directory to something
         * else, we don't need to worry about K95 2.1.3 and earlier named this
         * file, and we'll just go with "known_hosts" which is what OpenSSH
         * uses.
         */

        unh = malloc(sizeof(char)*NHPATHMAX);

        /* \v(appdata) = GetAppData(0) + "Kermit 95/" */
        ckmakmsg(unh, NHPATHMAX, GetAppData(0), "Kermit 95/", "ssh/", "known_hosts2");
    }

    if (ssh2_gnh != NULL) {     /* SSHv2 Global Known Hosts file */
        gnh = _strdup(ssh2_gnh);
    } else {
        gnh = malloc(sizeof(char)*NHPATHMAX);

        /* \v(common) = GetAppData(1) + "Kermit 95/" */
        ckmakmsg(gnh, NHPATHMAX, GetAppData(1), "Kermit 95/", "ssh/", "known_hosts2");
    }

    dir = ssh_directory();

    /* The SSH Subsystem will take ownership of this and handle cleaning it up
     * on disconnect */
    debug(F100, "ssh_open() - construct parameters", "", 0);
    parameters = ssh_parameters_new(
            ssh_hst,  /* Hostname */
            ssh_prt,  /* Port or Service Name */
            ssh_vrb,  /* Log verbosity */
            ssh_cmd,  /* Command or subsystem name */
            ssh_cas,  /* Subsystem, not command */
            ssh_cmp,  /* Use compression? */
            ssh_cfg,  /* Read openssh configuration */
            ssh_gsd,  /* GSSAPI Delegate Credentials */
            ssh_shk,  /* Strict Host Key Checking */
            unh,      /* User known hosts file */
            gnh,      /* Global known hosts file*/
            user,     /* Username */
            ssh_get_pw(), /* Password (if supplied) */
            get_current_terminal_type(),
            pty_width,
            pty_height,
            ssh2_auth,  /* Allowed authentication methods */
            ssh2_cif,   /* Allowed ciphers */
            ssh_hbt,    /* Heartbeat in seconds */
            ssh2_hka,   /* Allowed host key algorithms */
            ssh2_mac,   /* Allowed MACs */
            ssh2_kex,   /* Key exchange methods */
            ssh_get_nodelay_enabled(),/* Enable/disable Nagle's algorithm */
            ssh_pxc,    /* Proxy Command */
            port_forwards,  /* Direct and Reverse port forward config */
            ssh_xfw,        /* Forward X11 */
            x11_host,       /* Host to forward X11 too */
            display_number, /* X11 display number */
            ssh_xal,        /* Xauth location */
            dir,            /* SSH Dir*/
            ssh_idf         /* Identity files */
            );

    if (user) free(user);
    if (unh) free(unh);
    if (gnh) free(gnh);
    if (dir) free(dir);
    if (x11_host) {
        free(x11_host);
        x11_host = NULL;
    }

    if (parameters == NULL) {
        debug(F100, "ssh_open() - failed to construct parameters struct", "", 0);
        return SSH_ERR_MALLOC_FAILED;
    }

    /* This will be used to communicate with the SSH subsystem. It has
     * ring buffers, mutexes, semaphores, et. *WE* own this and must free it
     * on disconnect. */
    debug(F100, "ssh_open() - create client", NULL, 0);
    ssh_client = ssh_client_new();
    if (ssh_client == NULL) {
        debug(F100, "ssh_open() - failed to construct client struct", "", 0);
        ssh_parameters_free(parameters);
        return SSH_ERR_MALLOC_FAILED;
    }

    debug(F100, "ssh_open() - start SSH subsystem", "", 0);
    return start_ssh_subsystem(parameters, ssh_client, &hSSHClientThread);
}


/** Closes any existing SSH Session.
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos() {
    BOOL success;
    DWORD result;
    debug(F100, "ssh_clos()", "", 0);

    if (ssh_client == NULL)
        return SSH_ERR_NO_ERROR; /* Nothing to close */

    /* Signal the SSH thread we'd like it to disconnect and terminate please. */
    debug(F100, "ssh_clos() - requesting disconnect", "", 0);
    success = SetEvent(ssh_client->disconnectEvent);

    /* The SSH subsystem should signal the clientStopped event on disconnect,
     * but we'll check if the thread is still alive instead as that's more
     * important. The clientStopped event is really more so we can detect if
     * the thread failed to start in the first place (started, hit an error,
     * and stopped).
     */

    if (success) {
        /* Wait up to 30 seconds for the thread to terminate which really is
         * extremely generous. If it takes longer than a second something has
         * probably gone seriously wrong. */
        debug(F100, "ssh_clos() - waiting for thread terminate...", "", 0);
        result = WaitForSingleObject(hSSHClientThread, 30000);
        if (result == WAIT_TIMEOUT) {
            /* SSH Client thread didn't terminate in a reasonable time. */
            debug(F100, "Warning: SSH thread did not terminate on disconnect "
                        "request within the allocated time. SSH thread is "
                        "still live!", "", 0);
            return SSH_ERR_ZOMBIE_THREAD;
        } else if (result == WAIT_FAILED) {
            debug(F101, "Warning: failed to wait for SSH thread terminate. "
                        "error", "", GetLastError());
            return SSH_ERR_UNSPECIFIED;
        } else {
            ssh_client_t *temp;
            debug(F100, "ssh_clos() - thread terminated. Cleaning up...", "", 0);
            /* SSH Client thread has stopped. It should have already cleaned up
             * all the things it was responsible for, now it's our turn. */
            CloseHandle(hSSHClientThread);
            hSSHClientThread = NULL;

            temp = ssh_client;
            ssh_client = NULL;

            if (ring_buffer_lock(temp->inputBuffer, INFINITE)) {
                if (ring_buffer_lock(temp->outputBuffer, INFINITE)) {
                    ssh_client_free(ssh_client);
                }
            }

            return SSH_ERR_NO_ERROR;
        }
    } else {
        debug(F100, "Warning: Failed to signal SSH thread to disconnect", "", 0);
        return SSH_ERR_DISCONNECT_FAILED;
    }
}


/** Network I/O Check. This function is polled by K95 to get the status
 * of the connection and find out how many bytes are waiting to be
 * processed. It is called frequently so must return quickly.
 *
 * @return >= 0 indicates number of bytes waiting to be read
 *          < 0 indicates a fatal error and the connection should be closed.
 */
int ssh_tchk() {
    int rc = 0;

    if (ssh_client == NULL) {
        debug(F100, "ssh_tchk - error: no instance!", "", 0);
        return SSH_ERR_NO_INSTANCE;
    }

    /* If the client is connected then the number of bytes waiting to be read
     * is whatever is in the threads output buffer */

    if (ring_buffer_lock(ssh_client->outputBuffer, 0)) {
        if (ssh_client == NULL) {
            debug(F100, "ssh_tchk - error: no instance!", "", 0);
            return SSH_ERR_NO_INSTANCE;
        }

        rc = ring_buffer_length(ssh_client->outputBuffer);
        ring_buffer_unlock(ssh_client->outputBuffer);
    } else {
        debug(F100, "ssh_tchk - failed to get lock on output buffer", "", 0);
    }

    if (rc == 0) {
        /* No data. Check the thread is alive, client is connected, and there
         * isn't any error status. Zero is all ok which is also the number of
         * bytes we got back, so we're fine to just return this. */
        rc = get_ssh_error();
        debug(F111, "ssh_tchk - checked for subsystem error, got this", "rc", rc);
    }

    if (rc == 0) {
        debug(F100, "ssh_tchk - no data, no error - poking subsystem", "", 0);

        /* No errors either. Just no data. Give the SSH thread a poke to let it
         * know we're ready for data if there is any.
         *
         * This is only necessary because the SSH Thread can't currently monitor
         * the SSH connection for data arrival. Ideally we'd do a select
         * equivalent on the TTY channel but there seems to be no way of waiting
         * on both the SSH connection *and* events coming from CKWIN - its one
         * or the other.
         **/
        if (!SetEvent(ssh_client->dataConsumedEvent)) {
            debug(F100, "ssh_tchk - failed to signal data consumed event!",
                  "", 0);
        }
    }
    return rc;
}


/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui() {
    if (ssh_client == NULL) {
        return SSH_ERR_NO_INSTANCE;
    }

    if (!SetEvent(ssh_client->flushEvent)) {
        debug(F100, "ssh_flui() - failed to signal flush event!", "", 0);
        return SSH_ERR_EVENT_SIGNAL_FAILED;
    }

    return SSH_ERR_NO_ERROR;
}


/** Network Break - send a break signal to the remote process.
 * This may or may not do anything. Supported on SSH V2 only.
 *
 * @return
 */
int ssh_break() {
    if (ssh_client == NULL)
        return SSH_ERR_NO_INSTANCE;

    if (!SetEvent(ssh_client->breakEvent)) {
        debug(F100, "ssh_break() - failed to signal break event!", "", 0);
        return SSH_ERR_EVENT_SIGNAL_FAILED;
    }

    return SSH_ERR_NO_ERROR;
}


/** Input Character. Reads one character from the input queue
 *
 * @param timeout 0 for none (block until there is a character to read),
 *                positive for seconds, negative for milliseconds
 * @return -1 for timeout, >= 0 is a valid character, < -1 is a fatal error
 */
int ssh_inc(int timeout) {
    int timeout_ms;
    int rc;
    char buffer;

    if (ssh_client == NULL)
        return SSH_ERR_NO_INSTANCE;

    if (timeout == 0) {
        timeout_ms = INFINITE; /* Infinite timeout */
    } else if (timeout < 0) {
        timeout_ms = timeout * -1; /* timeout is in milliseconds already */
    } else {
        timeout_ms = timeout * 1000; /* timeout is in seconds - convert */
    }

    /* If an infinite timeout was requested then repeat the call until we get
     * a value other than try again */
    do {
        rc = ring_buffer_get_blocking(ssh_client->outputBuffer, &buffer, timeout_ms);
    } while (timeout == 0 && rc == RING_BUFFER_TRY_AGAIN);

    if (rc == RING_BUFFER_SUCCESS) {
        if (!SetEvent(ssh_client->dataConsumedEvent)) {
            debug(F100, "ssh_inc - failed to signal data consumed event!",
                  "", 0);
        }
        return buffer;
    } else if (rc == RING_BUFFER_TIMEOUT || rc == RING_BUFFER_LOCK_ERROR) {
        /* Treat a lock error as a timeout rather than returning it has a
         * character (as it has a positive value) or as an error (which
         * may conflict with values set via ring_buffer_signal_error) */
        return -1;
    }

    /* Else some error occurred */
    return rc;
}


/** Extended Input - reads multiple characters from the network. This
 * will never be called requesting more characters than previously
 * reported as available by ssh_tchk(). We aren't required to return
 * the number of characters requested but we must not return more (this
 * is the size of the buffer allocated)
 *
 * @param count Maximum number of characters to be read
 * @param buffer Buffer (of length count) to read characters into
 * @return >= 0 indicates the number of characters read, < 0 indicates error
 */
int ssh_xin(int count, char * buffer) {
    int rc = 0;
    int buffer_length;

    debug(F111, "ssh_xin", "count", count);

    if (ssh_client == NULL) {
        debug(F100, "ssh_xin - no instance", "", 0);
        return SSH_ERR_NO_INSTANCE;
    }

    if(ring_buffer_lock(ssh_client->outputBuffer, 0)) {
        if (ssh_client == NULL) {
            debug(F100, "ssh_xin - no instance", "", 0);
            return SSH_ERR_NO_INSTANCE;
        }

        buffer_length = ring_buffer_length(ssh_client->outputBuffer);
        debug(F101, "ssh_xin - bytes available", "", buffer_length);
        rc = ring_buffer_read(ssh_client->outputBuffer, buffer, count);
        ring_buffer_unlock(ssh_client->outputBuffer);

        /* Let the SSH thread know there is free space in the output buffer
         * again in case it has data waiting to be read from the network */
        if (!SetEvent(ssh_client->dataConsumedEvent)) {
            debug(F100, "ssh_xin - failed to signal data consumed event!",
                  "", 0);
        }

        debug(F111, "ssh_xin - bytes read", "rc", rc);
        buffer_length = ring_buffer_length(ssh_client->outputBuffer);
    } else {
        debug(F100, "ssh_xin - failed to get lock on output buffer", "", 0);
    }

    return rc;
}

/** Terminal Output Character - send a single character. Blocks until the
 * character has been handled.
 *
 * @param c character to send
 * @return 0 for success, <0 for error
 */
int ssh_toc(int c) {
    int rc;

    if (ssh_client == NULL)
        return SSH_ERR_NO_INSTANCE;

    /* Repeat the call until we get a value other than try again */
    do {
        rc = ring_buffer_put_blocking(ssh_client->inputBuffer, c, INFINITE);
    } while (rc == RING_BUFFER_TRY_AGAIN);

    if (rc == RING_BUFFER_SUCCESS) {
        /* Let the SSH client know there is data to transmit */
        if (!SetEvent(ssh_client->dataArrivedEvent)) {
            debug(F100, "ssh_toc - failed to signal data arrived event!", "", 0);
        }

        return SSH_ERR_NO_ERROR;
    } else if (rc == RING_BUFFER_TIMEOUT) {
        /* This should never happen */
        debug(F100, "ssh_toc() unexpected timeout on infinite timeout put",
              "", 0);
        return rc;
    }

    /* Else an error occurred */
    debug(F111, "ssh_toc() call to blocking ringbuf put failed", "error", rc);
    return rc;
}


/** Terminal Output Line - send multiple characters.
 *
 * @param buffer characters to be sent
 * @param count number of characters to be sent
 * @return  >= 0 for number of characters sent, <0 for a fatal error.
 */
int ssh_tol(char * buffer, int count) {
    int rc = 0;

    if (ssh_client == NULL)
        return SSH_ERR_NO_INSTANCE;

    if (ring_buffer_lock(ssh_client->inputBuffer, 0)) {
        if (ssh_client == NULL) {
            debug(F100, "ssh_tol - error: no instance!", "", 0);
            return SSH_ERR_NO_INSTANCE;
        }

        rc = ring_buffer_write(ssh_client->inputBuffer, buffer, count);
        ring_buffer_unlock(ssh_client->inputBuffer);
        if (!SetEvent(ssh_client->dataArrivedEvent)) {
            debug(F100, "ssh_tol - failed to signal data arrived event!", "", 0);
        }
    } /* Else couldn't get a lock immediately. Report zero bytes written - the
       * caller can try again */

    return rc;
}


/** Terminal information - called whenever the terminal type or dimensions
 * change.
 *
 * @param termtype Type of the terminal currently selected
 * @param height Height (lines)
 * @param width  width (columns)
 */
void ssh_terminfo(const char * termtype, int height, int width) {

    if (ssh_client == NULL)
        return;

    if (acquire_mutex(ssh_client->mutex, INFINITE)) {
        ssh_client->pty_height = height;
        ssh_client->pty_width = width;
        ReleaseMutex(ssh_client->mutex);
        if (!SetEvent(ssh_client->ptySizeChangedEvent)) {
            debug(F100, "ssh_terminfo - failed to signal pty size changed "
                        "event!", "", 0);
        }
    }

    /* Can we set the terminal type after the PTY has already been created?
     * libssh doesn't seem to provide an API to do it. A quick test with
     * Kermit 95 v2.1.2 suggests it doesn't do this, so I don't think we'd be
     * required to either. But it would be nice if it's possible.
     **/
}


/** Turn an error number into a string
 *
 */
const char * ssh_errorstr(int error) {
    switch (error) {
        case SSH_ERR_UNSPECIFIED:
            return "Unspecified error";
        case SSH_ERR_NEW_SESSION_FAILED:
            return "New session failed";
        case SSH_ERR_HOST_NOT_SPECIFIED:
            return "Remote host not specified";
        case SSH_ERR_SSH_ERROR:
            if (ssh_client && ssh_client->error_message) {
                return ssh_client->error_message;
            }
            return "SSH Error (no session available)";
        case SSH_ERR_HOST_VERIFICATION_FAILED:
            return "Host verification failed";
        case SSH_ERR_AUTH_ERROR:
            return "Authentication error";
        case SSH_ERR_EOF:
            return "End Of File received";
        case SSH_ERR_CHANNEL_CLOSED:
            return "Channel closed";
        case SSH_ERR_WAIT_FAILED:
            return "SSH Thread wait failed";
        case SSH_ERR_USER_CANCELED:
            return "User canceled";
        case SSH_ERR_UNSUPPORTED_VERSION:
            return "Unsupported SSH Version";
        case SSH_ERR_NOT_IMPLEMENTED:
            return "Feature not implemented";
        case SSH_ERR_ACCESS_DENIED:
            return "Access denied";
        case SSH_ERR_STATE_MALLOC_FAILED:
            return "SSH state malloc failed";
        case SSH_ERR_NO_INSTANCE:
            return "No SSH client instance";
        case SSH_ERR_MALLOC_FAILED:
            return "SSH Malloc failed";
        case SSH_ERR_ZOMBIE_THREAD:
            return "SSH thread survived disconnect attempt.";
        case SSH_ERR_DISCONNECT_FAILED:
            return "Failed to signal SSH thread to disconnect";
        case SSH_ERR_EVENT_SIGNAL_FAILED:
            return "Failed to signal event to SSH thread";
        case SSH_ERR_BUFFER_CONSUME_FAILED:
            return "Buffer consume failed";
        case SSH_ERR_BUFFER_WRITE_FAILED:
            return "Buffer write failed";
        case SSH_ERR_THREAD_STATE_UNKNOWN:
            return "SSH thread did not complete startup in expected time; "
                   "state is unknown";
        default:
            return "Unknown error";
    }
}


#ifdef COMMENT
/*
 * Not sure why these are here as they're never called anywhere. Perhaps I
 * copied them from the previous SSH implementation, or maybe C-Kermit changes
 * at some point removed the calls to them.
 */

/** Switching from terminal to VT mode. The CONNECT or DIAL commands might
 * be about to send printable text or escape sequences.
 *
 * @return 0 on success, < 0 on error.
 */
int ssh_ttvt() {
    /* Just report success here. Returning any kind of error just results
     * in a weird message to the user like
     * "Sorry, Can't condition communication line" which is pretty meaningless
     * for an SSH connection */
    return 0;
}


/** About to switch from terminal to packet mode. A file transfer operation is
 * about to start.
 *
 * @return 0 on success, < 0 otherwise.
 */
int ssh_ttpkt() {
    /* Nothing much to do here. Just return an error if we have one. */
    return get_ssh_error();
}


/** Terminal restore mode. Restore to default settings.
 *
 * @return 0 on success, < 0 on failure.
 */
int ssh_ttres() {
    /* Nothing much to do here. Just return an error if we have one. */
    return get_ssh_error();
}
#endif


/** Negotiate About Window Size. Let the remote host know the window dimensions
 * and terminal type if these have changed.
 *
 */
int ssh_snaws() {
    int rows, cols;
    debug(F100, "ssh_snaws()", "", 0);

    if (ssh_client == NULL)
        return 0;

    get_current_terminal_dimensions(&rows, &cols);
    ssh_terminfo(get_current_terminal_type(), rows, cols);

    return 0;
}


static int ssh_fwd_port(int type, char* address, int port, char* host, int host_port, BOOL apply) {
    /* Search for a free port forwarding entry */
    if (port_forwards == NULL) {
        /* Array doesn't exist yet. Create it. */
        port_forwards = malloc(sizeof(ssh_port_forward_t) * MAX_PORT_FORWARDS);
        memset(port_forwards, 0, sizeof(ssh_port_forward_t) * MAX_PORT_FORWARDS);
    }

    /* Validate the forwarding request - port number must be unique for all
     * forwardings of the same kind */
    for (int i = 0; i < MAX_PORT_FORWARDS; i++) {
        if (type == SSH_PORT_FORWARD_LOCAL &&
            port_forwards[i].type == type &&
            port_forwards[i].port == port) {

            printf("?Error: a local port forwarding already exists on port %d\n", port);
            return SSH_ERR_DUPLICATE_PORT_FWD;
        } else if (type == SSH_PORT_FORWARD_REMOTE &&
            port_forwards[i].type == type &&
            port_forwards[i].port == port) {

            printf("?Error: a remote port forwarding already exists on port %d\n", port);
            return SSH_ERR_DUPLICATE_PORT_FWD;
        }
    }


    /* Add the new forwarding to the list. Not <= MAX_PORT_FORWARDS as we want
     * to leave one entry at the end fo the array with its type set to
     * SSH_PORT_FORWARD_NULL to mark the end of the array */
    for (int i = 0; i < MAX_PORT_FORWARDS; i++) {
        if (port_forwards[i].type == SSH_PORT_FORWARD_NULL ||
            port_forwards[i].type == SSH_PORT_FORWARD_INVALID) {

            port_forwards[i].type = type;
            port_forwards[i].port = port;
            port_forwards[i].hostname = _strdup(host);
            port_forwards[i].host_port = host_port;

            /* For direct (local) forwardings, this just appears in the server
             * logs at the moment, but in the future it should specify the
             * local interface to listen on for new connections
             *
             * For reverse (remote) forwardings, this is the server address to
             * bind on. It should be either a specific address (like localhost
             * I guess), or NULL to listen on all addresses.
             */
            port_forwards[i].address = _strdup(address);

            if (apply) {
                /* TODO: If the SSH connection is currently active, add it to
                 *       the connection */
            }

            return SSH_ERR_NO_ERROR;
        }
    }

    /* TODO: While we're out of space to store the port forwarding, we could
     *       still add it to an active SSH session if there is one. We'd want
     *       to notify the user its only a temporary forwarding though and that
     *       they'll have to add it again if they close and re-open the SSH
     *       session. Such forwardings also wouldn't be able to appear in the
     *       "show ssh" screen unless we switched to using a linked list to
     *       return forwarding config to K95.
     */

    /* The list of port forwards is full */
    printf("?Error: Maximum number of port forwardings already specified\n");
    return SSH_ERR_TOO_MANY_FORWARDS;
}

/** Add a new Reverse (remote) port forward for future connections. This is
 * called by the following commands:
 *    SSH ADD REMOTE-PORT-FORWARD
 *    SSH FORWARD-REMOTE-PORT
 *
 * @param address Address the remote SSH server should listen on (reserved for
 *          future use)
 * @param port Port the remote SSH server will listen on
 * @param host Host connetions will be made to (on the client side)
 * @param host_port Port connections will be made to (on the client side)
 * @param apply Add the forwarding to any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_remote_port(char* address, int port, char * host, int host_port, BOOL apply)
{
    return ssh_fwd_port(SSH_PORT_FORWARD_REMOTE, address, port, host, host_port, apply);
}

/** Add a new Direct (local) port forward for future connections. This is
 * called by the following commands:
 *    SSH ADD LOCAL-PORT-FORWARD
 *    SSH FORWARD-LOCAL-PORT
 *
 * @param address Address the remote SSH server should listen on (reserved for
 *          future use)
 * @param port Port K95 will listen on for new connections
 * @param host Host connetions will be made to from the server
 * @param host_port port connections will be made to from the server
 * @param apply Add the new forwarding to any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_local_port(char* address, int port, char * host, int host_port, BOOL apply) {
    return ssh_fwd_port(SSH_PORT_FORWARD_LOCAL, address, port, host, host_port, apply);
}

/** Clears all remote port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all remote ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_remote_ports(BOOL apply) {

    /* Clear out all SSH_PORT_FORWARD_REMOTE entries*/
    for (int i = 0; i < MAX_PORT_FORWARDS; i++) {
        if (port_forwards[i].type == SSH_PORT_FORWARD_REMOTE) {

            if (apply) {
                /* TODO: Also remove from the active connection (if any) */
            }

            port_forwards[i].type = SSH_PORT_FORWARD_INVALID;
            port_forwards[i].port = 0;
            port_forwards[i].host_port = 0;

            if (port_forwards[i].hostname != NULL) {
                free(port_forwards[i].hostname);
                port_forwards[i].hostname = NULL;
            }

            if (port_forwards[i].address != NULL) {
                free(port_forwards[i].address);
                port_forwards[i].address = NULL;
            }
        }
    }

    return 0;
}

/** Clears all local port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all  ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_local_ports(BOOL apply) {
    /* Clear out all SSH_PORT_FORWARD_LOCAL entries*/
    for (int i = 0; i < MAX_PORT_FORWARDS; i++) {
        if (port_forwards[i].type == SSH_PORT_FORWARD_LOCAL) {

            if (apply) {
                /* TODO: Also remove from the active connection (if any) */
            }

            port_forwards[i].type = SSH_PORT_FORWARD_INVALID;
            port_forwards[i].port = 0;
            port_forwards[i].host_port = 0;

            if (port_forwards[i].hostname != NULL) {
                free(port_forwards[i].hostname);
                port_forwards[i].hostname = NULL;
            }

            if (port_forwards[i].address != NULL) {
                free(port_forwards[i].address);
                port_forwards[i].address = NULL;
            }
        }
    }

    return 0;
}

static int ssh_fwd_remove_port(int type, int port, BOOL apply) {

    for (int i = 0; i < MAX_PORT_FORWARDS; i++) {
        if (port_forwards[i].type == type && port_forwards[i].port == port) {

            if (apply) {
                /* TODO: Also remove from the active connection (if any) */
            }

            port_forwards[i].type = SSH_PORT_FORWARD_INVALID;
            port_forwards[i].port = 0;
            port_forwards[i].host_port = 0;

            if (port_forwards[i].hostname != NULL) {
                free(port_forwards[i].hostname);
                port_forwards[i].hostname = NULL;
            }

            if (port_forwards[i].address != NULL) {
                free(port_forwards[i].address);
                port_forwards[i].address = NULL;
            }
        }
    }

    return 0;
}

/** Remove a single reverse/remote port forward.
 *
 * @param port Reverse port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_remote_port(int port, BOOL apply) {
    return ssh_fwd_remove_port(SSH_PORT_FORWARD_REMOTE, port, apply);
}

/** Remove a single direct/local port forward.
 *
 * @param port Direct port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_local_port(int port, BOOL apply) {
    return ssh_fwd_remove_port(SSH_PORT_FORWARD_LOCAL, port, apply);
}

/** Gets all forwarded ports. The final entry in the list has a type of
 * SSH_PORT_FORWARD_NULL.
 *
 * @returns List of forwarded ports, or NULL on error or empty list
 */
const ssh_port_forward_t* ssh_fwd_get_ports() {

    return port_forwards;
}

/* These live in ckoreg.c */
char* GetHomePath();
char* GetHomeDrive();

/** Creates an SSH key pair
 *
 * @param filename File to write the private key to
 * @param bits Length of the key in bits. Valid options vary by key type
 * @param pp Passphrase
 * @param type Key type
 * @param cmd_comment SSH V1 RSA Comment (obsolete)
 * @return
 */
int sshkey_create(char * filename, int bits, char * pp, int type, char * cmd_comment) {
    enum ssh_keytypes_e ktype;
    char *output_filename = NULL, *pubkey_output_filename = NULL,
         *passphrase = NULL, *default_filename = NULL;
    ssh_key key = NULL;
    int rc;

    debug(F100, "sshkey_create", "", 0);

    /* By default, openssh searches for id_rsa, id_ecdsa, id_ecdsa_sk,
     * id_ed25519, id_ed25519_sk and id_dsa */
    switch(type) {
        case SSHKT_DSS:
            default_filename = "id_dsa";
            ktype = SSH_KEYTYPE_DSS;
            if (bits == 0) bits = 1024;
            if (bits != 1024 && bits != 2048) {
                printf("Invalid key length %d - valid options are: 1024, 2048\n");
            }
            break;
        case SSHKT_RSA:
            default_filename = "id_rsa";
            ktype = SSH_KEYTYPE_RSA;
            if (bits == 0) bits = 3072;
            if (bits != 1024 && bits != 2048 && bits != 3072 &&
                    bits != 2048 && bits != 8192) {
                printf("Invalid key length %d - valid options are: 1024, 2048, 3072, 4096, 8192\n");
                return SSH_ERR_UNSPECIFIED;
            }
            break;
        case SSHKT_RSA1:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_RSA1;
            if (bits == 0) bits = 3072;
            if (bits != 1024 && bits != 2048 && bits != 3072 &&
                bits != 2048 && bits != 8192) {
                printf("Invalid key length %d - valid options are: 1024, 2048, 3072, 4096, 8192\n");
                return SSH_ERR_UNSPECIFIED;
            }
            break;
        case SSHKT_ECDSA:
            default_filename = "id_ecdsa";
            ktype = SSH_KEYTYPE_ECDSA;
            if (bits == 0) bits = 256;
            if (bits != 256 && bits != 384 && bits != 521) {
                printf("Invalid key length %d - valid options are: 256, 384, 521\n");
                return SSH_ERR_UNSPECIFIED;
            }
            break;
        case SSHKT_ED25519:
            default_filename = "id_ed25519";
            ktype = SSH_KEYTYPE_ED25519;
            bits = 0; /* No bits */
            break;
        case SSHKT_DSS_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_DSS_CERT01;
            break;
        case SSHKT_RSA_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_RSA_CERT01;
            break;
        case SSHKT_ECDSA_P256:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P256;
            break;
        case SSHKT_ECDSA_P384:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P384;
            break;
        case SSHKT_ECDSA_P521:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P521;
            break;
        case SSHKT_ECDSA_P256_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P256_CERT01;
            break;
        case SSHKT_ECDSA_P384_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P384_CERT01;
            break;
        case SSHKT_ECDSA_P521_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ECDSA_P521_CERT01;
            break;
        case SSHKT_ED25519_CERT01:
            default_filename = NULL;
            ktype = SSH_KEYTYPE_ED25519_CERT01;
            break;
        default:
            printf("Unrecognised or unsupported key type\n");
            return SSH_ERR_UNSPECIFIED;
    }

    if (filename) {
        output_filename = _strdup(filename);
    } else {
        char* default_pathname;
        char* dir;
        output_filename = malloc(MAX_PATH * sizeof(char));
        default_pathname = malloc(MAX_PATH * sizeof(char));

        dir = ssh_directory();

        /* We'll suggest the user save in their ssh directory by default as
         * that's where K95 looks by default */
        snprintf(default_pathname, MAX_PATH, "%s%s",
                 dir, default_filename);
        free(dir);

#ifdef CK_MKDIR
        /* Make the ssh directory if it doesn't already exist */
        zmkdir(default_pathname);
#endif

        /* GetHomePath gives unix-style directory separators which the windows
         * file dialog doesn't seem to like. So convert to DOS separators */
        for (int i = 0; i < MAX_PATH; i++) {
            if (default_pathname[i] == '\0') break;
            if (default_pathname[i] == '/') default_pathname[i] = '\\';
        }

        int rc = uq_file(
                "Enter a filename to save the generated keys to:", /* Text mode only, text above the prompt */
                "Save As",  /* file dialog title or text-mode prompt*/
                5, /* New file, don't append */
                NULL, /* Help text - not used */
                default_pathname,
                output_filename,
                MAX_PATH
                );
        free(default_pathname);

        if (rc == 0) {
            free(output_filename);
            return SSH_ERR_USER_CANCELED;
        } else if (rc < 0 ) {
            free(output_filename);
            return SSH_ERR_UNSPECIFIED;
        }
    }

    if (pp) {
        passphrase = _strdup(pp);
    }
    else {
        /* Prompt for passphrase. Two fields to get confirmation. */
        char pp1[250], pp2[250];
        struct txtbox fields[2];

        fields[0].t_buf = pp1;
        fields[0].t_len = sizeof(pp1);
        fields[0].t_lbl = "New passphrase: ";
        fields[0].t_dflt = NULL;
        fields[0].t_echo = 2;

        fields[1].t_buf = pp2;
        fields[1].t_len = sizeof(pp2);
        fields[1].t_lbl = "New passphrase (again): ";
        fields[1].t_dflt = NULL;
        fields[1].t_echo = 2;

        rc = uq_mtxt("Enter SSH Key passphrase. Leave both fields empty empty "
                     "for no passphrase.",
                     NULL, 2, fields);

        if ( !rc ) {
            printf("User cancelled\n");
            free(output_filename);
            return(SSH_ERR_USER_CANCELED);
        }

        if (strcmp(pp1, pp2) != 0) {
            printf("Passphrase mismatch, no action taken\n");
            free(output_filename);
            return(SSH_ERR_UNSPECIFIED);
        }

        if (strlen(pp1) > 0) {
            passphrase = _strdup(pp1);
        }
    }

    printf("Generating private key...\n");
    rc = ssh_pki_generate(ktype, bits, &key);
    if (rc != SSH_OK) {
        printf("Failed to generate private key\n");
        return SSH_ERR_UNSPECIFIED;
    }

    /* TODO: use auth_prompt instead of the above here \|/ */
    rc = ssh_pki_export_privkey_file(key, passphrase, NULL, NULL, output_filename);
    if (rc != SSH_OK) {
        printf("Failed to write private key to %s - error %d\n", output_filename, rc);
    } else {
        printf("Your identification has been saved in %s\n", output_filename);
        pubkey_output_filename = malloc(MAX_PATH);

        snprintf(pubkey_output_filename, MAX_PATH, "%s.pub", output_filename);
        rc = ssh_pki_export_pubkey_file(key, pubkey_output_filename);
        if (rc != SSH_OK) {
            printf("Failed to write public key to %s\n", pubkey_output_filename);
        } else {
            unsigned char *hash = NULL;
            size_t hlen = 0;
            char* fingerprint = NULL;

            printf("Your public key has been saved in %s\n", pubkey_output_filename);
            rc = ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen);
            if (rc != SSH_OK) {
                printf("Failed to get key fingerprint\n");
            } else {
                fingerprint = ssh_get_fingerprint_hash(
                        SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
                if (fingerprint != NULL) {
                    printf("The key fingerprint is:\n%s\n", fingerprint);
                    /* TODO: We should output the comment if there is one too */
                    ssh_string_free_char(fingerprint);
                } else {
                    printf("Failed to get the key fingerprint\n");
                }
                ssh_clean_pubkey_hash(&hash);
            }
        }

        free(pubkey_output_filename);
    }


    free(output_filename);
    free(passphrase);
    ssh_key_free(key);
    return SSH_ERR_NO_ERROR;
}

/** Gets the default key filename. This will be something like id_rsa
 * in the SSH directory.
 *
 * The returned string must be freed by the caller.
 *
 * @return Default SSH key filename. Must be freed by caller.
 */
char* default_key_filename() {
    char *default_filename, *dir;
    dir = ssh_directory();
    default_filename = malloc(MAX_PATH * sizeof(char));

    /* We'll suggest the user save in their ssh directory by default as
     * that's where K95 looks by default */
    snprintf(default_filename, MAX_PATH, "%s%s",
             dir, "id_rsa");
    free(dir);
    return default_filename;
}

/** Displays the fingerprint for the specified public key
 *
 * @param filename Key file to display the fingerprint for. If not supplied, one
 *                 will be prompted for
 * @param babble 0 - fingerprint, 1 - IETF (ignored)
 * @return 0
 */
int sshkey_display_fingerprint(char * filename, int babble) {
    ssh_key key = NULL;
    int rc;
    unsigned char *hash = NULL;
    size_t hlen = 0;
    char* fingerprint = NULL;
    char* fn = NULL;

    /* We get here via one of the following:
     * ssh key display /format:fingerprint filename
     * ssh key display /format:ietf filename
     * babble 1 = /format:ietf
     * babble 0 = /format:fingerprint
     *
     * For the "fingerprint" format Kermit95 used to use the MD5 hash output in
     * the hex representation and for the "ietf" format the SHA1 has was used
     * in the "bubblebabble" representation.
     *
     * Libssh has deprecated the MD5 hash and doesn't support the "bubblebabble"
     * representation. So we always use the SHA256 hash output in the base64
     * representation here.
     */

/*    printf("sshkey_display_fingerprint\nFilename: %s\nBabble: %d\n", filename, babble);*/

    if (filename == NULL) {
        char *default_filename = default_key_filename();

        fn = malloc(MAX_PATH);
        int rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                default_filename,
                fn,
                MAX_PATH
        );
        free(default_filename);
        if (rc == 0) {
            free(fn);
            return SSH_ERR_USER_CANCELED;
        }
    } else {
        fn = _strdup(filename);
    }

    rc = ssh_pki_import_pubkey_file (fn, &key);
    if (rc != SSH_OK) {
        printf("Failed to open key: %s\n", fn);
        free(fn);
        return SSH_ERR_UNSPECIFIED;
    }
    free(fn);

    rc = ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen);
    if (rc != SSH_OK) {
        printf("Failed to get key fingerprint\n");
    } else {
        fingerprint = ssh_get_fingerprint_hash(
                SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
        if (fingerprint != NULL) {
            printf("%s\n", fingerprint);
            /* TODO: We should output the comment if there is one too */
            ssh_string_free_char(fingerprint);
        } else {
            printf("Failed to get the key fingerprint\n");
        }
        ssh_clean_pubkey_hash(&hash);
    }

    ssh_key_free(key);

    return SSH_ERR_OK;
}

/** Outputs the public key for the specified private key - same as what you'd
 * put in authorized_keys.
 * @param filename key file - will be prompted if not specified
 * @param passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public(char * filename, char *identity_passphrase) {
    ssh_key key = NULL;
    int rc;
    char* fn = NULL, * blob;

    /* We get here with the following:
     * ssh key display /format:openssh id_rsa
     */

    if (filename == NULL) {
        char *default_filename = default_key_filename();

        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                default_filename,
                fn,
                MAX_PATH
        );
        free(default_filename);
        if (rc == 0) {
            free(fn);
            return SSH_ERR_USER_CANCELED;
        }
    } else {
        fn = _strdup(filename);
    }

    rc = ssh_pki_import_privkey_file(fn, identity_passphrase, auth_prompt,
                                     NULL, &key);
    free(fn); fn = NULL;

    if (rc != SSH_OK) {
        if (rc == SSH_EOF) {
            printf("Failed to open private key file: %s - file not found "
                   "or permission denied\n", fn);
        } else {
            printf("Failed to open private key file: %s\n", fn);
        }
        return SSH_ERR_UNSPECIFIED;
    }

    rc = ssh_pki_export_pubkey_base64(key, &blob);
    if (rc != SSH_OK) {
        printf("Failed to export public key\n");
        return SSH_ERR_UNSPECIFIED;
    }
    printf("%s %s\n", ssh_key_type_to_char(ssh_key_type(key)), blob);
    ssh_string_free_char(blob);
    ssh_key_free(key);

    return SSH_ERR_OK;
}

/** Like sshkey_display_public but puts some extra cruft around the output.
 *
 * @param filename key file - will be prompted if not specified
 * @param identity_passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    /* ssh key display /format:ssh.com id_rsa */

    ssh_key key = NULL;
    int rc;
    char* fn = NULL, * blob;

    if (filename == NULL) {
        char *default_filename = default_key_filename();

        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                default_filename,
                fn,
                MAX_PATH
        );
        free(default_filename);
        if (rc == 0) {
            free(fn);
            return SSH_ERR_USER_CANCELED;
        }
    } else {
        fn = _strdup(filename);
    }

    rc = ssh_pki_import_privkey_file(fn, identity_passphrase, auth_prompt,
                                     NULL, &key);
    free(fn); fn = NULL;

    if (rc != SSH_OK) {
        if (rc == SSH_EOF) {
            printf("Failed to open private key file: %s - file not found "
                   "or permission denied\n", fn);
        } else {
            printf("Failed to open private key file: %s\n", fn);
        }
        return SSH_ERR_UNSPECIFIED;
    }

    /*
     * Here we should display the public key in a format something like:
     * ---- BEGIN SSH2 PUBLIC KEY ----
     * Comment: "1024-bit RSA, converted from OpenSSH by david@LAPTOP-TBIBQL8D"
     * AAAAB3NzaC1yc2EAAAABIwAAAIEA0N7KNPaqX7j2bJJu31n9RmnTpmJpRzog+9sTwgzV+l
     * WgTa973gL29zso/8dlRXgzmD9xgZyiJUvlgsd/QBQt5iYlOwreEIRRFanJi2jgWTT/vNpO
     * 9n6tCpFIeve724NQtp86JY6bsIW+BGB/FG0rJQhAg7pVQ1vvZg92N6hPmtU=
     * ---- END SSH2 PUBLIC KEY ----
     *
     * We don't have any actual of getting the comment so its just being used
     * for the key type at the moment.
     */
    rc = ssh_pki_export_pubkey_base64(key, &blob);
    if (rc != SSH_OK) {
        printf("Failed to export public key\n");
        return SSH_ERR_UNSPECIFIED;
    }
    printf("---- BEGIN SSH2 PUBLIC KEY ----\nComment: \"%s\"\n%s\n---- END SSH2 PUBLIC KEY ----\n",
           ssh_key_type_to_char(ssh_key_type(key)), blob);
    ssh_string_free_char(blob);
    ssh_key_free(key);

    return SSH_ERR_OK;
}

int sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    ssh_key key = NULL;
    int rc;
    char* fn = NULL;
    char* pp = NULL;

    if (filename == NULL) {
        char *default_filename = default_key_filename();
        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                default_filename,
                fn,
                MAX_PATH
        );
        free(default_filename);
        if (rc == 0) {
            free(fn);
            return SSH_ERR_USER_CANCELED;
        }
    } else {
        fn = _strdup(filename);
    }

    /* Open the key with the old passphrase */
    rc = ssh_pki_import_privkey_file(fn, oldpp, auth_prompt, NULL, &key);
    if (rc != SSH_OK) {
        if (rc == SSH_EOF) {
            printf("Failed to open private key file: %s - file not found "
                   "or permission denied\n", fn);
        } else {
            printf("Failed to open private key file: %s\n", fn);
        }
        free(fn); fn = NULL;
        return SSH_ERR_UNSPECIFIED;
    }

    /* Make sure we have a new passphrase */
    if (newpp) {
        pp = _strdup(newpp);
    }
    else {
        /* Prompt for passphrase. Two fields to get confirmation. */
        char pp1[250], pp2[250];
        struct txtbox fields[2];

        fields[0].t_buf = pp1;
        fields[0].t_len = sizeof(pp1);
        fields[0].t_lbl = "New passphrase: ";
        fields[0].t_dflt = NULL;
        fields[0].t_echo = 2;

        fields[1].t_buf = pp2;
        fields[1].t_len = sizeof(pp2);
        fields[1].t_lbl = "New passphrase (again): ";
        fields[1].t_dflt = NULL;
        fields[1].t_echo = 2;

        rc = uq_mtxt("Enter SSH Key passphrase. Leave both fields empty empty "
                     "for no passphrase.",
                     NULL, 2, fields);

        if ( !rc ) {
            printf("User cancelled\n");
            free(fn);
            ssh_key_free(key);
            return(SSH_ERR_USER_CANCELED);
        }

        if (strcmp(pp1, pp2) != 0) {
            printf("Passphrase mismatch, no action taken\n");
            free(fn);
            ssh_key_free(key);
            return(SSH_ERR_UNSPECIFIED);
        }

        if (strlen(pp1) > 0) {
            pp = _strdup(pp1);
        }
    }

    /* Write out with new passphrase */
    rc = ssh_pki_export_privkey_file(key, pp, NULL, NULL, fn);
    if (rc != SSH_OK) {
        printf("Failed to write private key to %s - error %d\n", fn, rc);
    } else {
        printf("Your identification has been saved with the new passphrase.\n");
    }

    free(pp);
    free(fn);
    ssh_key_free(key);
    return SSH_ERR_OK;
}

#ifdef SSHTEST
int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    return SSH_ERR_NOT_IMPLEMENTED;
}
#endif /* SSHTEST */

#ifdef COMMENT
char * sshkey_default_file(int a) {
    return NULL; /* TODO */
}
#endif

void ssh_v2_rekey() {
    /* TODO */
}

/** Return the current protocol version. For now we only support SSH-2.0.
 *
 * @return "SSH-2.0"
 */
const char * ssh_proto_ver() {
    static char buf[16];
    snprintf(buf, sizeof buf, "SSH-2.0");
    return buf;
}

/** Return the current SSH backend/implementation version.
 */
const char * ssh_impl_ver() {
    static char buf[64];
    snprintf(buf, sizeof(buf), "libssh %s", ssh_version(0));
    return buf;
}

const char * ssh_dll_ver() {
    return cksshv;
}

int ssh_agent_delete_file(const char *filename) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ssh_agent_delete_all() {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ssh_agent_add_file(const char *filename) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ssh_agent_list_identities(int do_fp) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

ktab_ret ssh_get_keytab(int keytab_id) {
    ktab_ret ret;
    ret.rc = 0;

    switch(keytab_id) {
        case SSH_KTAB_V2_AUT: {
            ret.ktab = ssh2aut;
            ret.ktab_len = nssh2aut;
            break;
        }
        case SSH_KTAB_V2_CIPHERS: {
            ret.ktab = ssh2ciphers;
            ret.ktab_len = nssh2ciphers;
            break;
        }
        case SSH_KTAB_V2_MACS: {
            ret.ktab = ssh2macs;
            ret.ktab_len = nssh2macs;
            break;
        }
        case SSH_KTAB_HKA: {
            ret.ktab = hkatab;
            ret.ktab_len = nhkatab;
            break;
        }
        case SSH_KTAB_KEX: {
            ret.ktab = sshkextab;
            ret.ktab_len = nsshkextab;
            break;
        }
#ifdef SSH_V1
            case SSH_KTAB_V1_CIPHERS: {
            ret.ktab = ssh1ciphers;
            ret.ktab_len = nssh1ciphers;
            break;
        }
#endif /* SSH_V1 */
        default: {
            ret.rc = -1;
        }
    }
    return ret;
}

int ssh_feature_supported(int feature_id) {
    switch(feature_id) {

        case SSH_FEAT_OPENSSH_CONF: /* Configuration via openssh config file */
        case SSH_FEAT_KEY_MGMT:     /* SSH key creation, etc */
        case SSH_FEAT_PORT_FWD:     /* Local and remote port forwarding */
        case SSH_FEAT_X11_FWD:      /* X11 forwarding */
        case SSH_FEAT_REKEY_AUTO:   /* TODO: do we implement this? */
            return TRUE;

        case SSH_FEAT_SSH_V1:         /* Not supported by libssh anymore */
        case SSH_FEAT_PROXY_CMD:      /* Not supported by libssh on windows yet */
        case SSH_FEAT_ADV_KERBEROS4:  /* Not supported by libssh */
        case SSH_FEAT_ADV_KERBEROS5:  /* Not supported by libssh */
        case SSH_FEAT_REKEY_MANUAL:   /* Not supported by libssh */
        case SSH_FEAT_FROM_PRIV_PRT:  /* Not supported by libssh */
        case SSH_FEAT_GSSAPI_KEYEX:   /* Not supported by libssh */
        case SSH_FEAT_DYN_PORT_FWD:   /* Requires a SOCKS server implementation */
        case SSH_FEAT_X11_XAUTH:      /* TODO - not implemented here yet */
        case SSH_FEAT_AGENT_FWD:      /* TODO - not implemented here yet */
        case SSH_FEAT_GSSAPI_DELEGAT: /* TODO: can we support this ? I think so */
        case SSH_FEAT_AGENT_MGMT:     /* TODO: can we support this ? */
        default:
            return FALSE;
    }
}
