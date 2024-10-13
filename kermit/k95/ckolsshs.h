/* Kermit 95 SSH Subsystem
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

#ifndef CKWIN_SSHSUBSYS_H
#define CKWIN_SSHSUBSYS_H

#include <windows.h>

#include "ckorbf.h"

#ifndef NO_SSH_AGENT_SUPPORT
#define SSH_AGENT_SUPPORT
#endif

#ifndef NO_SSH_GSSAPI_SUPPORT
#define SSH_GSSAPI_SUPPORT
#endif

#ifdef SSH_DLL
#undef debug
#define debug(a,b,c,d) \
((void)(debug_logging()?dodebug(a,b,(char *)(c),(CK_OFF_T)(d)):0))
#endif /* SSH_DLL */

#define SESSION_TYPE_PTY        0
#define SESSION_TYPE_COMMAND    1
#define SESSION_TYPE_SUBSYSTEM  2

/* Settings that can't be set */
#define SSH_MAX_PASSWORD_PROMPTS 3      /* Number of times you can retry password auth */

/* Errors we can return */
/* TODO: Confirm which of these are still used */
#define SSH_ERR_NO_ERROR 0              /* Everything is OK */
#define SSH_ERR_OK SSH_ERR_NO_ERROR
#define SSH_ERR_UNSPECIFIED -1          /* Unspecified error */
#define SSH_ERR_NEW_SESSION_FAILED -2   /* Failed to create new SSH session */
#define SSH_ERR_HOST_NOT_SPECIFIED -3   /* Hostname was null */
#define SSH_ERR_SSH_ERROR -4            /* Libssh error, call ssh_get_error(session) */
#define SSH_ERR_HOST_VERIFICATION_FAILED -5 /* Verification of remote host failed */
#define SSH_ERR_AUTH_ERROR -6           /* Serious auth error */
#define SSH_ERR_EOF -7                  /* Remote has sent EOF */
#define SSH_ERR_CHANNEL_CLOSED -8       /* Channel is closed */
#define SSH_ERR_WAIT_FAILED -9          /* Failed to wait in SSH thread */
#define SSH_ERR_USER_CANCELED -10       /* User canceled */
/* -11 : not used */
#define SSH_ERR_UNSUPPORTED_VERSION -12 /* Unsupported SSH version */
/* -13 : not used */
#define SSH_ERR_NOT_IMPLEMENTED -14     /* Feature not implemented yet */
#define SSH_ERR_ACCESS_DENIED -15       /* Login failed */
/* -16 : not used */
#define SSH_ERR_STATE_MALLOC_FAILED -17 /* Failed to create ssh client state struct */
/* -18 : not used  */
#define SSH_ERR_NO_INSTANCE -19         /* No current SSH Client instance */
#define SSH_ERR_MALLOC_FAILED -20       /* Failed to malloc something */
#define SSH_ERR_ZOMBIE_THREAD -21       /* SSH thread survived disconnect request */
#define SSH_ERR_DISCONNECT_FAILED -22   /* Failed to signal disconnect event */
#define SSH_ERR_EVENT_SIGNAL_FAILED -23 /* Failed to signal some event */
#define SSH_ERR_BUFFER_CONSUME_FAILED -24 /* Failed to consume processed data from a ring buffer */
#define SSH_ERR_BUFFER_WRITE_FAILED -25 /* Writing to a buffer failed, data has been lost */
#define SSH_ERR_THREAD_STATE_UNKNOWN -26 /* SSH thread failed to start or fail in a reasonable time. State is now unknown */
#define SSH_ERR_LISTEN_SOCKET_CREATE_FAILED -27 /* Failed to create listen socket for direct forward */
#define SSH_ERR_DIRECTFWD_GETADDRINFO_FAILED -28 /* Call to getaddrinfo failed while setting up direct forward server */
#define SSH_ERR_DIRECTFWD_BIND_FAILED -29 /* Call to bind failed while setting up direct forward server */
#define SSH_ERR_DIRECTFWD_LISTEN_FAILED -30 /* Call to listen failed while setting up direct forward server */
#define SSH_ERR_MUTEX_TIMEOUT -31       /* Timeout waiting for mutext */

#ifndef SSH_PF_T
#define SSH_PF_T
/* Copied from ckossh.h */
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

#define MAX_AUTH_METHODS 10

/** Parameters passed to the SSH thread on startup telling it what to connect
 * to, settings, etc.
 */
typedef struct {
    char* hostname;                             /* ssh_hst */
    char* port;                                 /* ssh_prt - port or service */
    int log_verbosity;                          /* ssh_vrb, Verbosity 1-7 */
    int session_type;                           /* 0=pty, 1=command, 2=subsys */
    char* command_or_subsystem;                 /* ssh_cmd */
    BOOL gssapi_delegate_credentials;           /* ssh_gsd */
    BOOL compression;                           /* ssh_cmp */
    BOOL use_openssh_config;                    /* ssh_cfg */
    int host_key_checking_mode;                 /* ssh_shk */
    char* user_known_hosts_file;                /* ssh2_unh */
    char* global_known_hosts_file;              /* ssh2_gnh */
    char* username;                             /* uidbuf */
    char* password;                             /* pwbuf if pwflg */
    int pty_height, pty_width;                  /* Terminal dimensions */
    char* terminal_type;                        /* Terminal type */
    char* allowed_ciphers;                      /* Comma separated list of
                                                 * allowed ciphers */
    char* allowed_hostkey_algorithms;           /* Comma separated list */
    char* macs;                                 /* Comma separated list */
    char* key_exchange_methods;                 /* Comma separated list */
    int keepalive_seconds;                      /* Keepalive interval in
                                                 * seconds, 0 disables. */
    int nodelay;                                /* Set to disable nagles agorithm */
    char* proxy_command;                        /* Command to execute to connect to the server */
    char* ssh_dir;                              /* SSH Directory */
    char** identity_files;                      /* SSH Identity Files */
    SOCKET existing_socket;                     /* Connect with an existing socket */
    char* agent_location;                       /* SSH Agent Location */
    int agent_forwarding;                       /* Enable agent forwarding */

    /* Which authentication methods should be attempted and their order. */
    int authentication_methods[MAX_AUTH_METHODS];

    /* Host verification may need to force password authentication off
     * if it detects something suspicious. Easier to set a flag than
     * go through authentication_methods removing all occurrences of
     * the password auth option. */
    BOOL allow_password_auth;

    const ssh_port_forward_t *port_forwards;

    BOOL forward_x;             /* Forward X11 ? */
    char* x11_host;             /* Host where the X server is running */
    int x11_display;            /* X11 display number */
    char* xauth_location;       /* Xauth location (filename) */
} ssh_parameters_t;


/** This is how the outside world communicates with the SSH thread */
typedef struct {
    /* These ring buffers have their own mutexes which you can acquire and
     * release with ring_buffer_lock() and ring_buffer_unlock(). */
    ring_buffer_handle_t outputBuffer;
    ring_buffer_handle_t inputBuffer;

    /* Events raised by C-Kermit to signal conditions to the SSH thread */
    HANDLE disconnectEvent;   /* Disconnect requested by C-Kermit */
    HANDLE ptySizeChangedEvent;   /* K95 Terminal size changed. Store new values
                                   * in pty_height and pty_width */
    HANDLE flushEvent; /* Flush requested by C-Kermit */
    HANDLE breakEvent; /* Send break requested by C-Kermit */
    HANDLE dataArrivedEvent; /* K95 has put data in the input buffer to be sent */

    /* K95 has consumed data from the output buffer. Only raised when the
     * output buffer transitions from full to less than full in case the
     * network has more data waiting but couldn't do anything with it because of
     * a full buffer */
    HANDLE dataConsumedEvent;

    /* Events raised by the SSH thread */
    HANDLE clientStarted;
    HANDLE clientStopped;

    /* Everything else here is guarded by a single mutex (which you can ignore
     * if the thread is not running) */
    HANDLE mutex;
    int error; /* SSH thread puts error codes here */
    char* error_message; /* An error message, if any */
    int pty_height, pty_width; /* For sending terminal size changes to the SSH thread */

    HANDLE forwards_mutex;           /* Guards the list of ssh_forward_t */
    struct ssh_forward *forwards;         /* Linked list of SSH forwarding config */
    HANDLE forwardingConfigChanged;
} ssh_client_t;


/** Gets the current terminal (VTERM) dimensions
 *
 * @param rows Out - number of rows (lines)
 * @param cols Out - number of columns (chars)
 */
void get_current_terminal_dimensions(int* rows, int* cols);


/** Constructs and populates a new ssh_parameters_t.
 *
 * @param hostname Hostname to connect to
 * @param port Port or service name
 * @param verbosity Logging verbosity level (0-7)
 * @param command Command or subsystem to execute
 * @param subsystem If command is a subsystem name rather than a command to
 *                  execute
 * @param compression If compression should be used in both directions or not
 * @param use_openssh_config If the OpenSSH client configuration file should
 *                           be read (if it exists)
 * @param gssapi_delegate_credentials GSSAPI Delegate Credentials
 * @param host_key_checking_mode Strict Host Key Checking Mode
 * @param user_known_hosts_file Pathname for user known hosts file
 * @param global_known_hosts_file Pathname for global known hosts file
 * @param username Username
 * @param password Password
 * @param terminal_type Terminal emulation type
 * @param pty_width Current terminal width
 * @param pty_height Current terminal height
 * @param auth_methods Comma-separated list of allowed authentication methods
 * @param ciphers Comma-separated list of allowed ciphers
 * @param heartbeat Heartbeat interval in seconds
 * @param hostkey_algorithms Comma-separated list of allowed hostkey algorithms
 * @param macs Comma-separated list of allowed macs
 * @param key_exchange_methods Comma-separated list of key exchange methods
 * @param nodelay Set to disable Nagle's algorithm
 * @param proxy_command Set the command to be executed in order to connect to server
 * @param forward_x Forward X11
 * @param display_host Host running the X11 server
 * @param display_number X11 display number
 * @param xauth_location Xauth location
 * @param ssh_dir SSH Directory
 * @param identity_files List of identity files
 * @param socket Existing socket to use for the connection
 * @param agent_location SSH agent location
 * @param agent_forwarding Enable agent forwarding
 * @return A new ssh_parameters_t instance.
 */
ssh_parameters_t* ssh_parameters_new(
        const char* hostname, char* port, int verbosity, const char* command,
        BOOL subsystem, BOOL compression, BOOL use_openssh_config,
        BOOL gssapi_delegate_credentials, int host_key_checking_mode,
        const char* user_known_hosts_file, const char* global_known_hosts_file,
        const char* username, const char* password, const char* terminal_type,
        int pty_width, int pty_height, const char* auth_methods,
        const char* ciphers, int heartbeat, const char* hostkey_algorithms,
        const char* macs, const char* key_exchange_methods, int nodelay,
        const char* proxy_command, const ssh_port_forward_t *port_forwards,
        BOOL forward_x, const char* display_host, int display_number,
        const char* xauth_location, const char* ssh_dir,
        const char** identity_files, SOCKET socket,
        const char* agent_location, int agent_forwarding);

/** Frees the ssh_parameters_t struct and all its members.
 *
 * @param parameters Parameters struct to free
 */
void ssh_parameters_free(ssh_parameters_t* parameters);


/** Creates a new ssh_client_t struct
 *
 * @return A new ssh_client_t struct
 */
ssh_client_t * ssh_client_new();


/** Frees a ssh_client_t struct and all its members
 *
 * @param client ssh_client_t struct to free.
 */
void ssh_client_free(ssh_client_t *client);


/** Starts the SSH Subsystem. Blocks until the thread has started and either
 * connects successfully or encounters an error.
 *
 * @param parameters SSH Connection Parameters
 * @param ssh_client The SSH Client Interface
 * @param threadHandle Out - handle for the created thread
 * @return 0 on success or an error.
 */
int start_ssh_subsystem(ssh_parameters_t* parameters, ssh_client_t *ssh_client,
                        HANDLE *threadHandle);


/** Try to acquire a mutex. Release it with ReleaseMutex(mutex).
 *
 * @param mutex Mutex to try and acquire
 * @param msTimeout Timeout in milliseconds. Pass INFINITE for no timeout.
 * @return TRUE if the mutex was acquired or FALSE on timeout or error.
 */
BOOL acquire_mutex(HANDLE mutex, DWORD msTimeout);

/** Libssh ssh_auth_callback implementation
 */
int auth_prompt(const char* prompt, char* buf, size_t len, int echo,
                int verify, void* userdata);

#endif /* CKWIN_SSHSUBSYS_H */
