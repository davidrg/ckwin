char *cksshv = "SSH support, 10.0.0,  28 July 2022";
/*
 *  C K O S S H . C --  SSH Subsystem Interface for C-Kermit
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
 * The interface is described in ckoshs.h and the actual implementation
 * where all the work is done (using libssh) lives in ckoshs.c.
 */

#include <libssh/libssh.h>

#include "ckcdeb.h"
#include "ckossh.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckoshs.h"


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
 *   TODO: SSH KEY
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
 *      TODO: IDENTITY-FILE filename
 *      TODO: KERBEROS4 TGT-PASSING {ON,OFF}    -- delete
 *      TODO: KERBEROS5 TGT-PASSING {ON,OFF}    -- delete
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
 *      SSH VERSION {2, AUTOMATIC}
 *          value is saved in ssh_ver. 0=auto.
 *      TODO: SSH X11-FORWARDING {ON, OFF}
 *          SET TELNET ENV DISPLAY is used to set the DISPLAY value
 *      TODO: XAUTH-LOCATION filename
 */

/* Features libssh provides that might be nice to support in the future:
 *   * Acting as a server (let iksd run over ssh rather than telnet)
 */

/* More TODO:
 *  - TODO: Figure out why nano doesn't correctly resume after being suspended
 *          - Possibly a terminal emulation issue. It works fine when emulating
 *            a VT220. Htop doesn't quite resume properly either - doesn't redraw
 *            the entire screen like it should which is probably the same issue
 *            just exposed differently.
 *  - TODO: Fix keyboard interactive authentication
 *          - Answering correctly results in the loop going around again with
 *            SSH_AUTH_INFO but no prompts. Returning at that point falls
 *            through to password auth and, if thats unsuccessful, disconnect.
 *            So for now keyboard interactive is disabled.
 *  - TODO: Other Settings
 *  - TODO: How do we know /command: has finished? EOF?
 *  - TODO: fix UI prompt look&feel (weird inset buttons)
 *  - TODO: Kermit subsystem (/subsystem:kermit) doesn't work
 *  - TODO: X11 Forwarding
 *  - TODO: Other forwarding
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


/* Global variables */
extern char uidbuf[];                   /* User ID set via /user: */
extern char pwbuf[];                    /* Password set via /password: */
extern int  pwflg;                      /* Password has been set */
extern int tcp_nodelay;                 /* Enable/disable Nagle's algorithm */
int ssh_sock;   /* TODO: get rid of this (unless its needed for connecting
                 *      through a proxy server?) */

/* Local variables */
ssh_client_t *ssh_client = NULL;  /* Interface to the ssh subsystem */
HANDLE hSSHClientThread = NULL;   /* SSH subsystem thread */


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


/** Returns the current terminal type as a static string
 *
 * @return terminal type
 */
static char* get_current_terminal_type() {
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


/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open() {
    ssh_parameters_t* parameters;
    int pty_height, pty_width;
    int rc;

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
        debug(F100, "ssh_open() - error - username not specified", "", 0);
        debug(F100, "Error - username not specified (uidbuf is null)", "", 0);
        return SSH_ERR_USER_NOT_SPECIFIED;
    }

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
            ssh2_unh, /* User known hosts file */
            ssh2_gnh, /* Global known hosts file*/
            uidbuf,   /* Username */
            pwflg ? pwbuf : NULL, /* Password (if supplied) */
            get_current_terminal_type(),
            pty_width,
            pty_height,
            ssh2_auth,  /* Allowed authentication methods */
            ssh2_cif,   /* Allowed ciphers */
            ssh_hbt,    /* Heartbeat in seconds */
            ssh2_hka,   /* Allowed host key algorithms */
            ssh2_mac,   /* Allowed MACs */
            ssh2_kex,   /* Key exchange methods */
            tcp_nodelay,/* Enable/disable Nagle's algorithm */
            ssh_pxc     /* Proxy Command */
            );
    if (parameters == NULL) {
        debug(F100, "ssh_open() - failed to construct parameters struct", "", 0);
        return SSH_ERR_MALLOC_FAILED;
    }

    /* This will be used to communicate with the SSH subsystem. It has
     * ring buffers, mutexes, semaphores, et. *WE* own this and must free it
     * on disconnect. */
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
            debug(F110, "Warning: failed to wait for SSH thread terminate. "
                        "error", GetLastError(), 0);
            return SSH_ERR_UNSPECIFIED;
        } else {
            debug(F100, "ssh_clos() - thread terminated. Cleaning up...", "", 0);
            /* SSH Client thread has stopped. It should have already cleaned up
             * all the things it was responsible for, now it's our turn. */
            CloseHandle(hSSHClientThread);
            hSSHClientThread = NULL;
            ssh_client_free(ssh_client);
            ssh_client = NULL;
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
    if (ssh_client == NULL)
        return SSH_ERR_NO_INSTANCE;

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
void ssh_terminfo(char * termtype, int height, int width) {

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
        case SSH_ERR_USER_NOT_SPECIFIED:
            return "Username not specified";
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


int ssh_fwd_remote_port(int port, char * host, int host_port)
{
    return SSH_ERR_NOT_IMPLEMENTED; /* TODO */
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
        output_filename = malloc(MAX_PATH * sizeof(char));
        default_pathname = malloc(MAX_PATH * sizeof(char));

        /* We'll suggest the user save in %USERPROFILE%\.ssh by default as thats
         * where both C-Kermit and the windows builds of OpenSSH look */
        snprintf(default_pathname, MAX_PATH, "%s%s.ssh/%s",
                 GetHomeDrive(), GetHomePath(), default_filename);
#ifdef CK_MKDIR
        /* Make the .ssh directory if it doesn't already exist */
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
        fn = malloc(MAX_PATH);
        int rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                "id_rsa",
                fn,
                MAX_PATH
        );
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
        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                "id_rsa",
                fn,
                MAX_PATH
        );
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

    /* We get here with the following:
     * ssh key display /format:openssh id_rsa
     */

    if (filename == NULL) {
        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                "id_rsa",
                fn,
                MAX_PATH
        );
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
        fn = malloc(MAX_PATH);
        rc = uq_file(
                /* Text mode only, text above the prompt */
                "Enter the filename of the key to display the fingerprint for:",
                "Open Key File",  /* file dialog title or text-mode prompt*/
                1,    /* existing file */
                NULL, /* Help text - not used */
                "id_rsa",
                fn,
                MAX_PATH
        );
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

int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    return SSH_ERR_NOT_IMPLEMENTED;
}

char * sshkey_default_file(int a) {
    return NULL; /* TODO */
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

/** Return the current protocol version. For now we only support SSH-2.0.
 *
 * @return "SSH-2.0"
 */
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