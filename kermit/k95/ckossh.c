char *cksshv = "SSH support, 10.0.0,  28 July 2022";
/*
 *  C K O S S H . C --  libssh Interface for C-Kermit
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

#include <libssh/libssh.h>

#include "ckcdeb.h"
#include "ckossh.h"
#include "ckcker.h"

/* Only so we can get a definition for VTERM and VNUM */
#include "ckocon.h"

/* Global Variables:
 *   These are all declared in ckuus3.c around like 8040
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
 *   char* pwbuf      "\0"    Supplied password
 *   char* uidbuf     ""      Supplied username (if any)
 *
 * Unused Global Variables:
 *   ssh_afw, ssh_xfw, ssh_prp, ssh_shh, ssh_chkip,
 *   ssh_gwp, ssh_dyf, ssh_k4tgt, ssh_k5tgt, ssh2_ark,
 *   ssh_gkx, ssh_k5_is_k4, ssh_hbt, ssh_dummy
 *
 *   ssh2_cif, ssh2_mac, ssh2_auth, ssh_xal, ssh2_hka, xxx_dummy
 *
 * Obsolete or not used:
 *    char* ssh1_cif    SSH-1 Not supported     SSH-1 Cipher.
 *    char* ssh1_gnh    SSH-1 Not supported     SSH-1 Global Knownhosts file
 *    char* ssh1_unh    SSH-1 Not supported     SSH-1 User Knownhosts file
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
 *      TODO /X11-FORWARDING: {on,off}
 *   TODO: SSH ADD
 *      TODO: LOCAL-PORT-FORWARD local-port host port
 *      TODO: REMOTE-PORT-FORWARD remote-port host port
 *      TODO: AGENT ADD identity-file
 *      TODO: AGENT DELETE identity-file
 *      TODO: AGENT LIST
 *          TODO: /FINGERPRINT
 *      TODO: CLEAR
 *          TODO: LOCAL-PORT-FORWARD
 *          TODO: REMOTE-PORT-FORWARD
 *      TODO: KEY
 *          TODO: CHANGE-PASSPHRASE
 *              TODO: /NEW-PASSPHRASE:passphrase
 *              TODO: /OLD-PASSPHRASE:passphrase
 *              TODO: filename
 *          TODO: CREATE
 *              TODO: /BITS:bits
 *              TODO: /PASSPHRASE: passphrase
 *              TODO: /TYPE:{V1-RSA,V2-DSA,V2-RSA}
 *              TODO: /V1-RSA-COMMENT: comment
 *              TODO: filename
 *          TODO: DISPLAY
 *              TODO: /FORMAT:{fingerprint, ietf,openssh,ssh.com}
 *              TODO: filename
 *          TODO: V1 SET-COMMENT filename comment
 *          TODO: V2 REKEY
 *   SET SSH
 *      TODO: AGENT-FORWARDING {ON,OFF}
 *      TODO: CHECK-HOST-IP {ON,OFF}
 *      COMPRESSION {ON,OFF}
 *          Value stored in ssh_cmp
 *      TODO: DYNAMIC-FORWARDING {ON,OFF}
 *      TODO: GATEWAY-PORTS {ON,OFF}
 *      GSSAPI DELEGATE-CREDENTIALS {ON,OFF}
 *          Value is stored in ssh_gsd
 *      TODO: IDENTITY-FILE filename
 *      TODO: KERBEROS4 TGT-PASSING {ON,OFF}    -- delete
 *      TODO: KERBEROS5 TGT-PASSING {ON,OFF}    -- delete
 *      TODO: PRIVILEGED-PORT {ON,OFF}
 *      TODO: QUIET {ON,OFF}
 *      TODO: STRICT-HOST-KEY-CHECK {ASK, ON, OFF}
 *      USE-OPENSSH-CONFIG {ON,OFF}
 *          Value is stored in ssh_cfg
 *      V1 CIPHER {3DES, BLOWFISH, DES}
 *          Ignored (SSH-1 not supported)
 *      V1 GLOBAL-KNOWN-HOSTS-FILE filename
 *          Ignored (SSH-1 not supported)
 *      V1 USER-KNOWN-HOSTS-FILE filename
 *          Ignored (SSH-1 not supported)
 *      TODO: V2 AUTHENTICATION {EXTERNAL-KEYX, GSSAPI, HOSTBASED, KEYBOARD-INTERACTIVE, PASSWORD, PUBKEY, SRP-GEX-SHA1}
 *      TODO: V2 CIPHERS {3DES-CBC, AES128-CBC, AES192-CBC, AES256-CBC, ARCFOUR, BLOWFISH-CBC, CAST128-CBC, RIJNDAEL128-CBC, RIJNDAEL192-CBC, RIJNDAEL256-CBC}
 *                 libssh:3des-cbc, aes128-cbc, aes192-cbc, aes256-cbc, chachae20-poly1305, aes256-gcm@openssh.com, aes128-gcm@openssh.com, aes256-ctr, aes192-ctr, aes128-ctr,
 *                 -> will require changes to ckuus3.c
 *      V2 GLOBAL-KNOWN-HOSTS-FILE filename
 *          Stored in ssh2_gnh
 *      TODO: V2 HOSTKEY-ALGORITHMS {SSH-DSA, SSH-RSA}
 *      TODO: V2 MACS {HMAC-SHA1, HMAC-MD5, HMAC-MD5-96, HMAC-RIPEMD160, HMAC-SHA1-96}
 *              libssh:hmac-sha1, hmac-sha2-256-etm@openssh.com, hmac-sha2-512-etm@openssh.com, hmac-sha1-etm@openssh.com, hmac-sha2-512, hmac-sha2-256,  none
 *      V2 USER-KNOWN-HOSTS-FILE filename
 *          Stored in ssh2_unh
 *      VERBOSE level
 *          Report Errors - Verbosity Level. Range 0-7. Value stored in ssh_vrb
 *      SSH VERSION {1, 2, AUTOMATIC}
 *          Just reports an error if version is 1 (SSH-1 not supported)
 *          value is saved in ssh_ver. 0=auto.
 *      TODO: SSH X11-FORWARDING {ON, OFF}
 *          SET TELNET ENV DISPLAY is used to set the DISPLAY value
 *      TODO: XAUTH-LOCATION filename
 */

/* Features libssh provides that might be nice to support in the future:
 *   * Acting as a server (let iksd run over ssh rather than telnet)
 *   * SCP
 */

/* More TODO:
 *  - TODO: fix "linux" term type - running top or nano messes with the encoding
 *          - seems fine on weatherctl
 *          - term=xterm works fine
 *          - Perhaps its a problem with the termcap config on modern debian?
 *  - TODO: Banner isn't working
 *  - TODO: Sort out makefile situation
 *  - TODO: More Authentication options
 *      - https://api.libssh.org/stable/libssh_tutor_authentication.html
 *  - TODO: Settings
 *  - TODO: X11 Forwarding
 *  - TODO: Sort out host verification
 *          - it is working. So just needs tidying.
 *  - TODO: How do we know /command: has finished? EOF?
 *  - TODO: deal with changing terminal type after connect ? (K95 doesn't)
 *  - TODO: Deal with flush
 *  - TODO: Deal with break
 *  - TODO: fix UI prompt look&feel (weird inset buttons)
 *  - TODO: test /subsystem qualifier somehow (setup kermit as a subsystem?)
 *  - TODO: SFTP
 *      -DSFTP_BUILTIN
 *  - TODO: Build libssh with GSSAPI, pthreads and kerberos
 *          https://github.com/jwinarske/pthreads4w
 *  - TODO: Kermit subsystem
 *  - TODO: HTTP Proxying - this was something the previous implementaiton could
 *          handle?
 */

/* Settings that can't be set */
#define SSH_MAX_PASSWORD_PROMPTS 3      /* Number of times you can retry password auth */

/* Errors we can return */
#define SSH_ERR_MORE_AUTH_NEEDED 1      /* Partial auth succeeded, more auth needed with different methods */
#define SSH_ERR_NO_ERROR 0              /* Everything is OK */
#define SSH_ERR_UNSPECIFIED -1          /* Unspecified error */
#define SSH_ERR_NEW_SESSION_FAILED -2   /* Failed to create new SSH session */
#define SSH_ERR_HOST_NOT_SPECIFIED -3   /* Hostname was null */
#define SSH_ERR_SSH_ERROR -4            /* Libssh error, call ssh_get_error(session) */
#define SSH_ERR_HOST_VERIFICATION_FAILED -5 /* Verification of remote host failed */
#define SSH_ERR_AUTH_ERROR -6           /* Serious auth error */
#define SSH_ERR_EOF -7                  /* Remote has sent EOF */
#define SSH_ERR_CHANNEL_CLOSED -8       /* Channel is closed */
#define SSH_ERR_BUFFER_ERROR -9         /* Buffer overflow */
#define SSH_ERR_USER_CANCELED -10       /* User canceled */
#define SSH_ERR_SESSION_CLOSED -11      /* No session (session is null) */
#define SSH_ERR_UNSUPPORTED_VERSION -12 /* Unsupported SSH version */
#define SSH_ERR_OPENSSH_CONFIG_ERR -13  /* Error parsing OpenSSH Config */
#define SSH_ERR_NOT_IMPLEMENTED -14     /* Feature not implemented yet */

/* SSH Strict Host Key Checking options */
#define SSH_SHK_NO 0                    /* Trust everything implicitly */
#define SSH_SHK_YES 1                   /* Don't trust anything unexpected */
#define SSH_SHK_ASK 2                   /* Ask the user */

/* Global variables */
extern int tt_rows[];                   /* Screen rows */
extern int tt_cols[];                   /* Screen columns */
extern int tt_status[VNUM];             /* Terminal status line displayed */
extern char uidbuf[];                   /* User ID set via /user: */
extern char pwbuf[];                    /* Password set via /password: */
extern int  pwflg;                      /* Password has been set */
int ssh_sock;   // TODO: get rid of this

/* Local variables */
static ssh_session session = NULL;      /* Current SSH session (if any) */
static ssh_channel channel = NULL;      /* The tty channel - shell or command */
static int pty_height, pty_width;       /* Dimensions of the pty */

/* Similar to "show ssh" */
void debug_params(const char* function) {
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

/** LibSSH logging callback. Forwards all libssh logging on to the C-Kermit
 * logging infrastructure.
 *
 * @param priority Priority - smaller is more important
 * @param function Function that produced the log message
 * @param buffer   Log message
 * @param userdata User data.
 */
static void logging_callback(int priority, const char *function,
                             const char *buffer, void *userdata)
{
    char* buf;
    char timebuf[100];
    int msglen = 1; /* Need at least 1 for null termination */
    time_t now = time (0);

    (void)userdata; /* Don't care about this */

    strftime(timebuf, 100, "%Y-%m-%d %H:%M:%S", localtime (&now));

    msglen += strlen(function) + strlen(buffer) + strlen(timebuf) + 100;
    buf = malloc(msglen);

    snprintf(buf, msglen, "[%s, %d] %s: %s",
             timebuf, priority, function, buffer);

    debug(F100, buf, "", 0);

    free(buf);
}



int log_verbosity() {
    /* ssh_vrb is set via "set ssh verbose" and has a range of 0-7
     * libssh only has 5 logging verbosity levels so 5/6/7 are unused
     * and treated the same as 4 (max verbosity)
     * */
    switch (ssh_vrb) {
        case 0:
            return SSH_LOG_NOLOG;
        case 1:
            return SSH_LOG_WARNING;
        case 2:
            return SSH_LOG_PROTOCOL;
        case 3:
            return SSH_LOG_PACKET;
        case 4:
            return SSH_LOG_FUNCTIONS;
        case 5: /* not used */
        case 6: /* not used */
        case 7: /* not used */
        default:
            return SSH_LOG_FUNCTIONS;
    }
}

/** Returns the current terminal type as a static string
 *
 * @return terminal type
 */
char* get_current_terminal_type() {
    static char term_type[64];
    extern int tt_type, max_tt;
    extern struct tt_info_rec tt_info[];
    extern char * tn_term;

    term_type[0] = '\0';

    if (tn_term) {
        /* Terminal type override - set via "Set telnet terminal-type" */
        debug(F111, "terminal type override", tn_term, 0);
        if (*tn_term) {
            ckstrncpy(term_type, tn_term, sizeof(term_type));
        }
    } else {
        if (tt_type >= 0 && tt_type <= max_tt) {
            ckstrncpy(term_type, tt_info[tt_type].x_name, sizeof(term_type));
        } else {
            debug(F101, "Invalid terminal type ID", "tt_type", tt_type);
        }
    }

    if (!term_type[0]) {
        /* No valid terminal type set anywhere */
        ckstrncpy(term_type, "UNKNOWN", sizeof(term_type));
    }

    cklower(term_type);
    return(term_type);
}

void get_current_terminal_dimensions(int* rows, int* cols) {
    /* TODO: Elsewhere in K95 one is taken off the row count if the status line
     *       is on. But this seems to produce incorrect results - it looks like
     *       the tt_rows value already accounts for the status line.
     **/
    *rows = tt_rows[VTERM];/* - (tt_status[VTERM]?1:0); */
    *cols = tt_cols[VTERM];
}

/** Checks to see if the host being connected to is known.
 *
 * TODO: Overhaul this function.
 *
 * @return An error code (0 = success)
 */
int verify_known_host() {
    int rc = 0;
    ssh_key server_pubkey = NULL;
    unsigned char* hash = NULL;
    size_t hash_length = 0;
    enum ssh_known_hosts_e state;
    char *hexa;
    char msg[1024];

    rc = ssh_get_server_publickey(session, &server_pubkey);
    if (rc != SSH_OK) {
        printf("Failed to get public key\n");
        return SSH_ERR_SSH_ERROR;
    }

    rc = ssh_get_publickey_hash(server_pubkey,
                                SSH_PUBLICKEY_HASH_SHA256,
                                &hash,
                                &hash_length);
    ssh_key_free(server_pubkey);
    if (rc != SSH_OK) {
        printf("Failed to get public key hash\n");
        return SSH_ERR_SSH_ERROR;
    }

    state = ssh_session_is_known_server(session);
    /* TODO: Redo all of this properly. */
    switch (state) {
        case SSH_KNOWN_HOSTS_OK:
            /* Cool! */
            printf("Host verified!\n");
            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            /* Previously:
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
             *
             * If strict host key checking then display a message saying can't connect
             * If set to ask, ask user if continue or not.
             * If not requested, allow without password auth, agent forwarding,
             *      X11 forwarding and port forwarding.
             */
            printf("WARNING! The server key has changed. "
                   "This may indicate a possible attack.\n");
            hexa = ssh_get_hexa(hash, hash_length);
            printf("Public key hash: %s\n", hexa);
            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);
            return SSH_ERR_HOST_VERIFICATION_FAILED;
        case SSH_KNOWN_HOSTS_OTHER:
            /* Previously: If IP status == HOST_NEW then msg = "is unknown"
             *             If IP status == HOST_OK then msg = "is unchanged"
             * Message:
               "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
               "@       WARNING: POSSIBLE DNS SPOOFING DETECTED!          @\n"
               "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
               "The ", type, " host key for ", host, " has changed,\n"
               "and the key for the according IP address ", ip, "\n",
               msg, ". This could either mean that\n",
               "DNS SPOOFING is happening or the IP address for the host\n"
               "and its host key have changed at the same time.\n",
             */
            printf("Warning: Server key type has changed. An attacker may change the "
                   "server key type to confuse clients into thinking the key does "
                   "not exist.\n");
            ssh_clean_pubkey_hash(&hash);
            return SSH_ERR_HOST_VERIFICATION_FAILED;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            printf("Could not find the known hosts file. If you accept the key here "
                   "it will be created automatically.\n");
        case SSH_KNOWN_HOSTS_UNKNOWN:
            /*
             * The server is unknown. The user must confirm the public key hash is
             * correct.
             */
            if (ssh_shk == SSH_SHK_YES) {
                snprintf(msg, sizeof(msg),
                         "No host key is known for %s and\n"
                         "you have requested strict host checking.\n"
                         "If you wish to make an untrusted connection,\n"
                         "SET SSH STRICT-HOST-KEY-CHECK OFF and try again.",
                         ssh_hst);
                printf(msg);
#ifdef KUI
                uq_ok(NULL, msg, 1, NULL, 0);
#endif /* KUI */
            } else if (ssh_shk = SSH_SHK_ASK) {
                hexa = ssh_get_hexa(hash, hash_length);
                snprintf(msg, sizeof(msg),
                         "The authenticity of host '%s' can't be established.\n"
                         "The key fingerprint is %s.\n",
                         ssh_hst,  hexa);
                ssh_string_free_char(hexa);
                ssh_clean_pubkey_hash(&hash);

                if (!uq_ok(msg, "Are you sure you want to continue "
                                "connecting (yes/no)? ", 3, NULL, -1)) {
                    printf("Aborted by user!");
                    ssh_clean_pubkey_hash(&hash);
                    return SSH_ERR_HOST_VERIFICATION_FAILED;
                }
            } else if (ssh_shk != SSH_SHK_NO) {
                printf("Invalid Strict Host Key Check value!");
                ssh_clean_pubkey_hash(&hash);
                return SSH_ERR_HOST_VERIFICATION_FAILED;
            }

            rc = ssh_session_update_known_hosts(session);
            if (rc < 0) {
                printf("Error %s\n", strerror(errno));
                ssh_clean_pubkey_hash(&hash);
                return SSH_ERR_SSH_ERROR;
            }
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            printf("Error %s", ssh_get_error(session));
            ssh_clean_pubkey_hash(&hash);
            return SSH_ERR_SSH_ERROR;
    }

    ssh_clean_pubkey_hash(&hash);
    return SSH_ERR_NO_ERROR;
}

int password_authenticate() {
    char *user = NULL;
    char password[256] = "";
    char prompt[1024] = "";
    int i = 0;
    int rc = 0;
    int ok = FALSE;

    rc = ssh_options_get(session, SSH_OPTIONS_USER, &user);
    if (rc != SSH_OK) {
        debug(F100, "SSH - Failed to get user ID", "rc", rc);
        return rc;
    }

    debug(F110, "Attempting password authentication for user", user, 0);

    for (i = 0; i < SSH_MAX_PASSWORD_PROMPTS; i++) {
        if (i == 0 && pwbuf[0] && pwflg) {
            /* Password has already been supplied. Try that */
            debug(F100, "Using pre-entered password", "", 0);
            ckstrncpy(password,pwbuf,sizeof(password));
            ok = TRUE;
        } else {
            snprintf(prompt, sizeof(prompt), "%s%.30s@%.128s's password: ",
                     i == 0 ? "" : "Permission denied, please try again.\n",
                     user, ssh_hst);
            /* Prompt user for password */
            ok = uq_txt(prompt,"Password: ",2,NULL,password, sizeof(password),NULL,
                        DEFAULT_UQ_TIMEOUT);
        }

        if (!ok || strcmp(password, "") == 0) {
            /* User canceled */
            debug(F100, "User canceled password login", "", 0);
            ssh_string_free_char(user);
            return SSH_ERR_USER_CANCELED;
        }

        rc = ssh_userauth_password(session, NULL, password);
        memset(password, 0, strlen(password));
        if (rc == SSH_AUTH_SUCCESS) {
            debug(F100, "Password login succeeded", "", 0);
            ssh_string_free_char(user);
            return SSH_ERR_NO_ERROR;
        } else if (rc == SSH_AUTH_ERROR) {
            debug(F111, "SSH Auth Error - password login failed", "rc", rc);
            /* A serious error has occurred.  */
            ssh_string_free_char(user);
            return SSH_ERR_AUTH_ERROR;
        } else if (rc == SSH_AUTH_PARTIAL) {
            debug(F100, "SSH Partial authentication - "
                        "more authentication needed", "", 0);
            ssh_string_free_char(user);
            return SSH_ERR_MORE_AUTH_NEEDED;
        }
        /* Else: SSH_AUTH_DENIED - try again. */
        debug(F100, "SSH Password auth failed - access denied", "", 0);
    }

    ssh_string_free_char(user);
    return rc;
}

/** Opens the TTY channel and nothing else.
 *
 * @return SSH_OK on success, else an error
 */
int open_tty_channel() {
    int rc = 0;

    debug(F100, "Opening SSH tty channel", "", 0);

    channel = ssh_channel_new(session);
    if (channel == NULL) {
        debug(F100, "Failed to create channel", "", 0);
        return SSH_ERR_SSH_ERROR;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        debug(F111, "Channel open failed", "rc", rc);
        ssh_channel_free(channel);
        channel = NULL;
    }

    return rc;
}

/** Closes the tty channel if its currently open.
 *
 */
void close_tty_channel() {
    if (channel) {
        debug(F100, "Closing ssh tty channel", "", 0);
        ssh_channel_close(channel);
        ssh_channel_send_eof(channel);
        ssh_channel_free(channel);
        channel = NULL;
    }
}

/** Run a command on the remote host then disconnect.
 *
 * @param command Command to run
 * @return SSH_OK on success, or an error
 */
int ssh_rexec(const char* command) {
    int rc;

    debug(F110,"ssh running command", command, 0);

    rc = open_tty_channel();
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_exec(channel, command);
    if (rc != SSH_OK) {
        debug(F111, "SSH exec failed", "rc", rc);
        close_tty_channel();
        return rc;
    }

    debug(F100, "SSH exec succeeded", "", 0);

    return rc;
}

int ssh_subsystem(const char* subsystem) {
    int rc;

    /* TODO: Not working? */

    debug(F110,"ssh requesting subsystem", subsystem, 0);

    rc = open_tty_channel();
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_subsystem(channel, subsystem);
    if (rc != SSH_OK) {
        debug(F111, "SSH subsystem request failed", "rc", rc);
        close_tty_channel();
        return rc;
    }

    debug(F100, "SSH subsystem request succeeded", "", 0);

    return rc;
}


/** Opens the tty channel for a shell. This also sets up a PTY.
 *
 * @return SSH_OK on success, or an error.
 */
int open_shell() {
    int rc;
    char* termtype;

    debug(F100, "SSH open shell", "", 0);

    get_current_terminal_dimensions(&pty_height, &pty_width);
    termtype = get_current_terminal_type();
    debug(F111, "SSH pty request", "rows", pty_height);
    debug(F111, "SSH pty request", "cols", pty_width);
    debug(F111, "SSH pty request - termtype: ", termtype, 0);

    rc = open_tty_channel();
    if (rc != SSH_OK) {
        debug(F111, "open tty channel failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_pty_size(channel, termtype, pty_width, pty_height);
    if (rc != SSH_OK) {
        debug(F111, "PTY request failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK) {
        debug(F111, "Shell request failed", "rc", rc);
        return rc;
    }

    return rc;
}

/** Opens an SSH connection. Connection parameters are passed through the
 * following global variables:
 *   int   ssh_vrb    Logging verbosity
 *   char* ssh_hst    Hostname
 *
 *
 * @return An error code (0 = success)
 */
int ssh_open() {
    int verbosity = SSH_LOG_PROTOCOL;
    int rc = 0;
    char* banner = NULL;

    debug(F100, "ssh_open()", "", 0);
    //debug_params("ssh_open");

    if (ssh_ver == 1) {
        /* libssh doesn't support SSH-1 anymore so we don't either */
        printf("SSH-1 is not supported - please use SSH-2\n");
        return SSH_ERR_UNSUPPORTED_VERSION;
    }

    ssh_set_log_callback(logging_callback);

    /* Need a hostname to connect to a host... */
    if (ssh_hst == NULL) {
        debug(F100, "Error - host not specified (ssh_hst is null)", "", 0);
        return SSH_ERR_HOST_NOT_SPECIFIED;
    }

    session = ssh_new();
    if (session == NULL) {
        debug(F100, "Failed to create SSH session", "", 0);
        return SSH_ERR_NEW_SESSION_FAILED;
    }

    /* Set options */
    debug(F100, "Configure session...", "", 0);
    verbosity = log_verbosity();
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_HOST, ssh_hst);
    ssh_options_set(session, SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS, &ssh_gsd);
    ssh_options_set(session, SSH_OPTIONS_PROCESS_CONFIG, &ssh_cfg);
    if (!ssh_cmp) {
        ssh_options_set(session, SSH_OPTIONS_COMPRESSION_C_S, "no");
        ssh_options_set(session, SSH_OPTIONS_COMPRESSION_S_C, "no");
    }

    if (ssh_prt)
        ssh_options_set(session, SSH_ERR_BUFFER_ERROR, ssh_prt);
    if (uidbuf[0])
        ssh_options_set(session, SSH_OPTIONS_USER, uidbuf);
    if (ssh2_unh)
        ssh_options_set(session, SSH_OPTIONS_KNOWNHOSTS, ssh2_unh);
    if (ssh2_gnh)
        ssh_options_set(session, SSH_OPTIONS_GLOBAL_KNOWNHOSTS, ssh2_gnh);
    // TODO: Set SSH_OPTIONS_SSH_DIR to where the known_hosts and keys live
    // TODO: SSH_OPTIONS_STRICTHOSTKEYCHECK ?

    // identity fiels (set ssh identity-file)
    // stored in ssh_idf[32]
    // add with SSH_OPTIONS_ADD_IDENTITY

    if (ssh_cfg) {
        /* Parse OpenSSH Config */
        rc = ssh_options_parse_config(session,
                                      NULL);  /* Use default filename */
        if (rc < 0) {
            return SSH_ERR_OPENSSH_CONFIG_ERR;
        }
    }

    /* Connect! */
    debug(F100, "SSH Connect...", "", 0);
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        debug(F111,"Error connecting to host", ssh_get_error(session), rc);
        ssh_free(session); // TODO: This probably makes the error message unavailable.
                           //       check if K95 will call ssh_clos() after fetching
                           //       the error message. If it does we don't need to
                           //       clean up here.
        return SSH_ERR_SSH_ERROR;
    }

    /* Check the hosts key is valid */
    rc = verify_known_host();
    if (rc != SSH_ERR_NO_ERROR) {
        debug(F111, "Host verification failed", "rc", rc);
        printf("Host verification failed.\n");
        ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
        return rc;
    }

    /* TODO: This isn't working for some reason */
    banner = ssh_get_issue_banner(session);
    if (banner) {
        printf(banner);
        ssh_string_free_char(banner);
        banner = NULL;
    }

    /* Authenticate! */
    rc = password_authenticate();
    if (rc != SSH_ERR_NO_ERROR ) {
        /* TODO: Handle SSH_ERR_MORE_AUTH_NEEDED - partially authenticated */
        debug(F111, "Authentication failed - disconnecting", "rc", rc);
        ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
        return rc;
    }

    printf("Authenticated - starting session\n");

    /* TODO: Setup session (shell, port forwarding, etc) */
    debug(F100, "Authentication succeeded - starting session", "", 0);
    if (ssh_cmd && *ssh_cmd) {
        if (ssh_cas) {
            /* User has supplied the /SUBSYSTEM: qualifier */
            rc = ssh_subsystem(ssh_cmd);
        } else {
            /* User has supplied the /COMMAND: qualifier. */
            rc = ssh_rexec(ssh_cmd);
        }
    } else {
        /* Open a shell */
        rc = open_shell();
    }

    if (rc != SSH_OK) {
        debug(F111, "Session start failed - disconnecting", "rc", rc);
        ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
        return SSH_ERR_SSH_ERROR;
    }

    printf("Connection open!");
    debug(F100, "SSH connected.", "", 0);
    return SSH_ERR_NO_ERROR;
}

/** Closes any existing SSH Session
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos() {
    debug(F100, "ssh_clos()", "", 0);
    if (session) {
        close_tty_channel();
        ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
    }
    return SSH_ERR_NO_ERROR;
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

    if (ssh_channel_is_closed(channel)) {
        /* Channel is closed */
        debug(F100, "ssh_tchk() - channel is closed", "", 0);
        return SSH_ERR_CHANNEL_CLOSED;
    }

    rc = ssh_channel_poll(channel, 0);
    if (rc == SSH_EOF) {
        debug(F111, "ssh_tchk() - channel is EOF", "rc", rc);
        return SSH_ERR_EOF;
    } else if (rc == SSH_ERROR) {
        debug(F111,"ssh_tchk() - channel poll error", ssh_get_error(session), rc);
        return SSH_ERR_SSH_ERROR;
    }

    return rc;
}

/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui() {
    int rc = 0;
    debug(F100, "ssh_flui()", "", 0);
    //debug_params("ssh_flui");
    /* TODO: Call ssh_channel_flush(session) ? */

    if (ssh_channel_is_closed(channel)) {
        debug(F100, "ssh_flui() - channel is closed", "", 0);
        return SSH_ERR_CHANNEL_CLOSED;
    }

    /* TODO: Is this correct ?
     *   NO, it is not - ssh_channel_flush does not exist (its internal/private)
     * */
    /*rc = ssh_channel_flush(session);
    if (rc != SSH_OK) {
        return SSH_ERR_SSH_ERROR;
    }*/

    return SSH_ERR_NO_ERROR;
}

/** Network Break - send a break signal to the remote process.
 * This may or may not do anything. Supported on SSH V2 only.
 *
 * @return
 */
int ssh_break() {
    debug(F100, "ssh_break()", "", 0);
    if (ssh_channel_is_closed(channel)) {
        debug(F100, "ssh_break() - channel is closed", "", 0);
        return SSH_ERR_CHANNEL_CLOSED;
    }

    /* TODO: Is this correct? The old implementation didn't seem to
     *       do breaks */
    ssh_channel_request_send_break(
            channel,
            5);  /* TODO" Break length in milliseconds */

    return SSH_ERR_UNSPECIFIED; /* TODO */
}


/** Input Character. Reads one character from the input queue
 *
 * @param timeout 0 for none, positive for seconds, negative for milliseconds
 * @return -1 for timeout, >= 0 is a valid character, < -1 is a fatal error
 */
int ssh_inc(int timeout) {
    int timeout_ms;
    char buffer;
    int rc;

    if (timeout == 0) {
        timeout_ms = -1; /* Infinite timeout */
    } else if (timeout < 0) {
        timeout_ms = timeout * -1; /* timeout is in milliseconds already */
    } else {
        timeout_ms = timeout * 1000; /* timeout is in seconds - convert */
    }

    if (ssh_channel_is_closed(channel)) {
        debug(F100, "ssh_inc() - channel is closed", "", 0);
        return SSH_ERR_CHANNEL_CLOSED;
    } else if (ssh_channel_is_eof(channel)) {
        debug(F100, "ssh_inc() - channel is EOF", "", 0);
        return SSH_ERR_EOF;
    }

    rc = ssh_channel_read_timeout(
            channel,        /* Channel to read from */
            &buffer,        /* Buffer to read into */
            1,              /* bytes to read */
            0,              /* read from stderr rather than stdout */
            timeout_ms);    /* Timeout in milliseconds */

    if (rc == 1) {
        /* One byte read - good */
        return buffer;
    } else if (rc > 1) {
        /* Oops! We read multiple bytes into a buffer one byte long. */
        debug(F111, "ssh_inc() ERROR: buffer overflow - expecting 1 char, got ", "count", rc);
        return SSH_ERR_BUFFER_ERROR;
    } else if (rc == 0) {
        /* No bytes read - assumed timeout */
        debug(F100, "ssh_inc() timeout ", "", 0);
        return -1;
    } else if (rc == SSH_ERROR) {
        /* Error of some kind */
        debug(F111,"ssh_inc() - channel read error", ssh_get_error(session), rc);
        return SSH_ERR_SSH_ERROR;
    }

    debug(F111, "ssh_inc() unexpected result", "rc", rc);

    /* Unexpected error of some kind */
    return SSH_ERR_UNSPECIFIED;
}

/** Extended Input - reads multiple characters from the network. This
 * will never be called requesting more characters than previously
 * reported as available by ssh_tchk(). We are'nt required to return
 * the number of characters requested but we must not return more (this
 * is the size of the buffer allocated)
 *
 * @param count Maximum number of characters to be read
 * @param buffer Buffer (of length count) to read characters into
 * @return >= 0 indicates the number of characters read, < 0 indicates error
 */
int ssh_xin(int count, char * buffer) {
    int rc = 0;
    if (ssh_channel_is_closed(channel)) {
        debug(F100, "ssh_xin() - channel is closed", "", 0);
        return SSH_ERR_CHANNEL_CLOSED;
    }
    if (ssh_channel_is_eof(channel)) {
        debug(F100, "ssh_xin() - channel is EOF", "", 0);
        return SSH_ERR_EOF;
    }

    debug(F111, "ssh_xin() read", "count", count);

    /* This function does not block - it may return less than the number
     * of characters requested */
    rc = ssh_channel_read_nonblocking(
            channel,    /* Channel to read from */
            buffer,     /* Buffer to read into */
            count,      /* Number of bytes to be read */
            0           /* Read from stderr instead of stdout */
            );

    if (rc == 0) {
        debug(F100, "ssh_xin() - nothing available", "", 0);
        return 0;
    } else if (rc < 0) {
        debug(F111,"ssh_xin() - read error ", ssh_get_error(session), rc);
        return SSH_ERR_SSH_ERROR;
    }

    debug(F111, "ssh_xin() read", "chars", rc);

    return rc; /* Number of characters read */
}

/** Terminal Output Character - send a single character.
 *
 * @param c character to send
 * @return 0 for success, <0 for error
 */
int ssh_toc(int c) {
    int rc;

    rc = ssh_channel_write(channel,
                           &c,
                           1);
    if (rc == 1) return 0; /* Success */

    debug(F111,"ssh_toc() - channel write error", ssh_get_error(session), rc);

    return SSH_ERR_SSH_ERROR; /* Failure */
}

/** Terminal Output Line - send multiple characters.
 *
 * @param buffer characters to be sent
 * @param count number of characters to be sent
 * @return  >= 0 for number of characters sent, <0 for a fatal error.
 */
int ssh_tol(char * buffer, int count) {
    int rc;

    debug(F111,"ssh_tol() - write ", "count", count);

    rc = ssh_channel_write(channel,
                           buffer,
                           count);
    if (rc == count) return rc; /* Success */

    debug(F111,"ssh_tol() - channel write error", ssh_get_error(session), rc);

    return SSH_ERR_SSH_ERROR; /* Failure */
}

/** Terminal information - called whenever the terminal type or dimensions
 * change.
 *
 * @param termtype Type of the terminal currently selected
 * @param height Height (lines)
 * @param width  width (columns)
 */
void ssh_terminfo(char * termtype, int height, int width) {
    int rc;

    debug(F100, "ssh_terminfo() - termtype", termtype, 0);
    debug(F100, "ssh_terminfo()", "height", height);
    debug(F100, "ssh_terminfo()", "width", width);

    if (!session) return;
    if (channel_is_closed(channel)) return;
    if (channel_is_eof(channel)) return;

    if (height != pty_height || width != pty_width) {
        pty_height = height;
        pty_width = width;

        rc = ssh_channel_change_pty_size(channel, width, height);
        if (rc != SSH_OK) {
            debug(F111,"ssh_terminfo() - failed to change pty size", ssh_get_error(session), rc);
        }
    }

    /* Can we set the terminal type after the PTY has already been created?
     * libssh doesn't seem to provide an API to do it. A quick test with
     * Kermit 95 v2.1.2 suggests it doesn't do this so I don't think we'd be
     * required to either. But it would be nice if its possible.
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
            if (session) {
                return ssh_get_error(session);
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
        case SSH_ERR_BUFFER_ERROR:
            return "Potential buffer overflow (should never occur)";
        case SSH_ERR_USER_CANCELED:
            return "User canceled";
        case SSH_ERR_SESSION_CLOSED:
            return "No SSH session";
        case SSH_ERR_UNSUPPORTED_VERSION:
            return "Unsupported SSH Version";
        case SSH_ERR_OPENSSH_CONFIG_ERR:
            return "Error parsing OpenSSH Configuration File";
        case SSH_ERR_NOT_IMPLEMENTED:
            return "Feature not implemented";
        default:
            return "Unknown error";
    }
}

/** Switching from terminal to VT mode. The CONNECT or DIAL commands might
 * be about to send printable text or escape sequences.
 *
 * @return 0 on success, < 0 on error.
 */
int ssh_ttvt() {
    /* We don't really care about this for SSH. Just return 0 if we're
     * connected or an error otherwise. */
    if (!session) return SSH_ERR_SESSION_CLOSED;
    if (channel_is_closed(channel)) return SSH_ERR_CHANNEL_CLOSED;
    if (channel_is_eof(channel)) return SSH_ERR_EOF;

    return 0;
}

/** About to switch from terminal to packet mode. A file transfer operation is
 * about to start.
 *
 * @return 0 on success, < 0 otherwise.
 */
int ssh_ttpkt() {
    /* We don't really care about this for SSH. Just return 0 if we're
     * connected or an error otherwise. */
    if (!session) return SSH_ERR_SESSION_CLOSED;
    if (channel_is_closed(channel)) return SSH_ERR_CHANNEL_CLOSED;
    if (channel_is_eof(channel)) return SSH_ERR_EOF;

    return 0;
}

/** Terminal restore mode. Restore to default settings.
 *
 * @return 0 on success, < 0 on failure.
 */
int ssh_ttres() {
    /* We don't really care about this for SSH. Just return 0 if we're
     * connected or an error otherwise. */
    if (!session) return SSH_ERR_SESSION_CLOSED;
    if (channel_is_closed(channel)) return SSH_ERR_CHANNEL_CLOSED;
    if (channel_is_eof(channel)) return SSH_ERR_EOF;

    return 0;
}

/** Negotiate About Window Size. Let the remote host know the window dimensions
 * and terminal type if these have changed.
 *
 */
int ssh_snaws() {
    int rows, cols;
    debug(F100, "ssh_snaws()", "", 0);

    get_current_terminal_dimensions(&rows, &cols);
    ssh_terminfo(get_current_terminal_type(), rows, cols);

    return 0;
}

int ssh_fwd_remote_port(int port, char * host, int host_port)
{
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_create(char * filename, int bits, char * pp, int type, char * cmd_comment) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_display_fingerprint(char * filename, int babble) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_display_public(char * filename, char *identity_passphrase) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

char * sshkey_default_file(int a) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ssh_fwd_local_port(int a, char *b, int c) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

int ssh_few_remote_port(int a, char *b, int c) {
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
}

void ssh_v2_rekey() {
    /* TODO */
}

char * ssh_proto_ver() {
    static char buf[16];
    snprintf(buf, sizeof buf, "SSH-2.0");
    return buf;
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