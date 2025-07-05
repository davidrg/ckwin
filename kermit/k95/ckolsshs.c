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

#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <process.h>
#include <time.h>
#include <Ws2tcpip.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef UNIX_PATH_MAX
/* This is the important stuff in afunix.h which is only available in very
 * recent Windows SDKs and may not be present in MinGW.
 */
#define UNIX_PATH_MAX 108
struct sockaddr_un {
    ADDRESS_FAMILY sun_family;
    char sun_path[UNIX_PATH_MAX];
};
#endif /* UNIX_PATH_MAX */

#include "ckcdeb.h"
#include "ckcker.h"

/* Only so we can get a definition for VTERM and VNUM */
#include "ckocon.h"

#include "ckolsshs.h"

#ifdef COMMENT
/* This won't work from SSH_DLL */
#ifdef KUI
extern HWND hwndConsole;
#endif
#endif /* COMMENT */

#define SSH_FWD_TYPE_DIRECT  1
#define SSH_FWD_TYPE_REVERSE 2
#define SSH_FWD_TYPE_X11     3
#define SSH_FWD_TYPE_AGENT   4

#define SSH_FWD_STATE_PENDING   1
#define SSH_FWD_STATE_OPEN      2
#define SSH_FWD_STATE_CLOSED    3
#define SSH_FWD_STATE_ERROR     4

typedef struct ssh_forward_connection {
    struct ssh_forward_connection* next;

    ssh_channel channel;
    SOCKET socket;
    HANDLE thread;  /* Handles copying data between the buffers and socket */

    /* Event that can be set to indicate to the connection thread we want it
     * to finish */
    HANDLE disconnectEvent;

    /* Data output to socket handling thread */
    ring_buffer_handle_t outputBuffer;

    /* Data input from the socket handling thread*/
    ring_buffer_handle_t inputBuffer;

    /* State for this particular connection */
    int state;
} ssh_forward_connection_t;

typedef struct ssh_fwd_thread_params {
    SOCKET socket;
    /* Data in and out of this thread */
    ring_buffer_handle_t outputBuffer, inputBuffer;

    /* Event that is set whenever data is written to the output
     * ring buffer */
    HANDLE readyRead;

    /* Event that is set to tell the thread to disconnect and terminate */
    HANDLE disconnectEvent;
} ssh_fwd_thread_params_t;

/* Linked list of SSH port, reverse port and X11 forwards */
typedef struct ssh_forward {
    struct ssh_forward* next;

    int state;      /* one of SSH_FWD_STATE_* */
    int type;       /* one of SSH_FWD_TYPE_* */

    ssh_forward_connection_t *connections;

    /* For Direct (local) forwards */
    SOCKET listen_socket;
    char* remoteHost;
    int remotePort;
    char* localHost;
    int localPort;

} ssh_forward_t;

/* SSH Strict Host Key Checking options */
#define SSH_SHK_NO 0                    /* Trust everything implicitly */
#define SSH_SHK_YES 1                   /* Don't trust anything unexpected */
#define SSH_SHK_ASK 2                   /* Ask the user */


/** This holds all the global state for the SSH Thread.
 */
typedef struct {
    ssh_parameters_t *parameters;    /* Connection parameters */
    ssh_client_t *client;            /* The client instance */
    ssh_session session;             /* Current SSH session (if any) */
    ssh_channel ttyChannel;          /* The tty channel - shell or command */
    int pty_height, pty_width;       /* Dimensions of the pty */
    HANDLE forwardingReadyRead;      /* Data arrived at a forwarding thread */
    BOOL forwarding_ok;
} ssh_client_state_t;


typedef struct {
    ssh_parameters_t* parameters;
    ssh_client_t *ssh_client;
} ssh_thread_params_t;

static void close_tty_channel(ssh_client_state_t * state);


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
        const char* agent_location, int agent_forwarding,
        const char* environment_variables[MAX_ENVIRONMENT_VARIABLES][2]) {
    ssh_parameters_t* params;

    params = (ssh_parameters_t*)malloc(sizeof(ssh_parameters_t));
    memset(params, 0, sizeof(ssh_parameters_t));

    params->existing_socket = INVALID_SOCKET;
    params->hostname = NULL;
    params->port = NULL;
    params->command_or_subsystem = NULL;
    params->user_known_hosts_file = NULL;
    params->global_known_hosts_file = NULL;
    params->username = NULL;
    params->password = NULL;
    params->terminal_type = NULL;
    params->allowed_ciphers = NULL;
    params->allowed_hostkey_algorithms = NULL;
    params->macs = NULL;
    params->key_exchange_methods = NULL;
    params->keepalive_seconds = heartbeat;
    params->nodelay = nodelay;
    params->proxy_command = NULL;
    params->ssh_dir = NULL;
    params->agent_location = NULL;
    params->agent_forwarding = FALSE;

    params->identity_files = identity_files;

    params->existing_socket = socket;

    /* Copy hostname and port*/
    params->hostname = _strdup(hostname);
    if (port) params->port = _strdup(port);

    /* Copy command/subsystem name (if there is one) and set session type */
    params->session_type = SESSION_TYPE_PTY;
    if (command) {
        params->command_or_subsystem = _strdup(command);
        if (subsystem) params->session_type = SESSION_TYPE_SUBSYSTEM;
        else params->session_type = SESSION_TYPE_COMMAND;
    }

    if (user_known_hosts_file)
        params->user_known_hosts_file = _strdup(user_known_hosts_file);
    if (global_known_hosts_file)
        params->global_known_hosts_file = _strdup(global_known_hosts_file);
    if (username) params->username = _strdup(username);
    if (password) params->password = _strdup(password);
    if (terminal_type) params->terminal_type = _strdup(terminal_type);
    if (ciphers) params->allowed_ciphers = _strdup(ciphers);
    if (hostkey_algorithms)
        params->allowed_hostkey_algorithms = _strdup(hostkey_algorithms);
    if (macs) params->macs = _strdup(macs);
    if (key_exchange_methods)
        params->key_exchange_methods = _strdup(key_exchange_methods);
    if (proxy_command) params->proxy_command = _strdup(proxy_command);
    if (ssh_dir) params->ssh_dir = strdup(ssh_dir);
    if (agent_location) {
#ifdef SSH_AGENT_SUPPORT
        params->agent_location = _strdup(agent_location);
        params->agent_forwarding = agent_forwarding;
#else
        params->agent_location = NULL;
        params->agent_forwarding = FALSE;
#endif
    }

    params->log_verbosity = verbosity;
    params->compression = compression;
    params->use_openssh_config = use_openssh_config;
    params->gssapi_delegate_credentials = gssapi_delegate_credentials;
    params->host_key_checking_mode = host_key_checking_mode;
    params->pty_width = pty_width;
    params->pty_height = pty_height;
    params->port_forwards = port_forwards;

    /* Grab a copy of the environment variables */
    for (int i = 0; i < MAX_ENVIRONMENT_VARIABLES; i++) {
        if (environment_variables[i][0] != NULL && environment_variables[i][1] != NULL) {
            params->environment_variables[i][0] = _strdup(environment_variables[i][0]);
            params->environment_variables[i][1] = _strdup(environment_variables[i][1]);
        } else {
            params->environment_variables[i][0] = NULL;
            params->environment_variables[i][1] = NULL;
        }
    }

    /* If the user has supplied a list of authentication types then only those
     * types specified will be allowed.*/
    if (auth_methods) {
        int len, index = 0;
        char* temp, *token;
        char* delim = ",";

        /*
         * Libssh requires we try the none auth method first
         * in order to get the list of available auth methods:
         * https://api.libssh.org/stable/group__libssh__auth.html#ga35d44897a44b4bb3b7c01108c1812a37
         */
        params->authentication_methods[index] = SSH_AUTH_METHOD_NONE;
        index++;

        /* Copy the input string as strtok will modify it */
        len = strlen(auth_methods) + 1;
        temp = (char*)malloc(len * sizeof(char));
        memset(temp, 0, len * sizeof(char));
        strcpy_s(temp, len * sizeof(char),
                 auth_methods);

        token = strtok(temp, delim);

        while(token != NULL && index < MAX_AUTH_METHODS) {
            debug(F111, "sshsubsys - adding auth-method", token, index);

            if (strcmp(token, "gssapi") == 0) {
                params->authentication_methods[index] = SSH_AUTH_METHOD_GSSAPI_MIC;
            } else if (strcmp(token, "keyboard-interactive") == 0) {
                params->authentication_methods[index] = SSH_AUTH_METHOD_INTERACTIVE;
            } else if (strcmp(token, "password") == 0) {
                params->authentication_methods[index] = SSH_AUTH_METHOD_PASSWORD;
                params->allow_password_auth = TRUE;
            } else if (strcmp(token, "publickey") == 0) {
                params->authentication_methods[index] = SSH_AUTH_METHOD_PUBLICKEY;
            } else if (strcmp(token, "none") == 0) {
                params->authentication_methods[index] = SSH_AUTH_METHOD_NONE;
            }

            if (params->authentication_methods[index] != 0) {
                index++;
            }

            token = strtok(NULL, delim);
        }

        free(temp);
    } else {
        /* The Kermit 95 default order was:
         *   external-keyx, gssapi, hostbased, publickey, srp-gex-sha1,
         *   publickey, keyboard-interactive, password, none
         *
         * Our default order is:
         *   none, gssapi, publickey, keyboard-interactive, password
         * We attempt none first as libssh may require this in order
         * to correctly get the list of supported auth methods along
         * with other details like the banner
         */
        params->authentication_methods[0] = SSH_AUTH_METHOD_NONE;
        params->authentication_methods[1] = SSH_AUTH_METHOD_GSSAPI_MIC;
        params->authentication_methods[2] = SSH_AUTH_METHOD_PUBLICKEY;
        params->authentication_methods[3] = SSH_AUTH_METHOD_INTERACTIVE;
        params->authentication_methods[4] = SSH_AUTH_METHOD_PASSWORD;
        params->allow_password_auth = TRUE;
    }

    if (forward_x && display_host != NULL) {
        params->forward_x = forward_x;
        params->x11_host = _strdup(display_host);
        params->x11_display = display_number;
        params->xauth_location = _strdup(xauth_location);
    }

    return params;
}


void ssh_parameters_free(ssh_parameters_t* parameters) {
    if (parameters->hostname) free(parameters->hostname);
    if (parameters->port) free(parameters->port);
    if (parameters->command_or_subsystem)
        free(parameters->command_or_subsystem);
    if (parameters->user_known_hosts_file)
        free(parameters->user_known_hosts_file);
    if (parameters->global_known_hosts_file)
        free(parameters->global_known_hosts_file);
    if (parameters->username)
        free(parameters->username);
    if (parameters->password) {
        SecureZeroMemory(parameters->password, sizeof(parameters->password));
        free(parameters->password);
    }
    if (parameters->terminal_type)
        free(parameters->terminal_type);
    if (parameters->allowed_ciphers)
        free(parameters->allowed_ciphers);
    if (parameters->allowed_hostkey_algorithms)
        free(parameters->allowed_hostkey_algorithms);
    if (parameters->macs)
        free(parameters->macs);
    if (parameters->key_exchange_methods)
        free(parameters->key_exchange_methods);
    if (parameters->proxy_command)
        free(parameters->proxy_command);
    if (parameters->ssh_dir)
        free(parameters->ssh_dir);
    if (parameters->x11_host)
        free(parameters->x11_host);
    if (parameters->xauth_location)
        free(parameters->xauth_location);
    if (parameters->agent_location)
        free(parameters->agent_location);

    for (int i = 0; i < MAX_ENVIRONMENT_VARIABLES; i++) {
        if (parameters->environment_variables[i][1] != NULL) {
            free(parameters->environment_variables[i][1]);
            parameters->environment_variables[i][1] = NULL;
        }
        if (parameters->environment_variables[i][0] != NULL) {
            free(parameters->environment_variables[i][0]);
            parameters->environment_variables[i][0] = NULL;
        }
    }

    /* Note: parameters->identity_files should *not* be freed as we're not
     *       currently taking a copy of it */

    free(parameters);
}


ssh_client_t * ssh_client_new() {
    ssh_client_t *client = malloc(sizeof(ssh_client_t));
    client->outputBuffer = ring_buffer_new(1024*1024);
    client->inputBuffer = ring_buffer_new(1024*1024);
    client->error = SSH_ERR_NO_ERROR;
    client->error_message = NULL;
    client->mutex = CreateMutex(
            NULL,   /* default security attributes */
            FALSE,  /* initially not owned */
            NULL);  /* unnamed */
    client->disconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->ptySizeChangedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->flushEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    client->breakEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    client->dataArrivedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->dataConsumedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    client->clientStarted = CreateEvent(NULL, /* Security Attributes */
                                        TRUE, /* Manual Reset */
                                        FALSE, /* Initial state (non-signaled) */
                                        NULL); /* Name */
    client->clientStopped = CreateEvent(NULL, /* Security Attributes */
                                        TRUE, /* Manual Reset */
                                        FALSE, /* Initial state (non-signaled) */
                                        NULL); /* Name */

    client->forwards_mutex = CreateMutex(
            NULL,   /* default security attributes */
            FALSE,  /* initially not owned */
            NULL);  /* unnamed */;
    client->forwards = NULL;
    client->forwardingConfigChanged = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (client->forwards_mutex == NULL) {
        debug(F100, "sshsubsys - WARNING: Failed to create mutex for SSH "
                    "forwarding config", "", 0);
    }

    return client;
}


void ssh_client_free(ssh_client_t *client) {
    if (client == NULL) return;
    ring_buffer_free(client->outputBuffer);
    client->outputBuffer = NULL;
    ring_buffer_free(client->inputBuffer);
    client->inputBuffer = NULL;

    if (client->error_message) {
        free(client->error_message);
        client->error_message = NULL;
    }
    CloseHandle(client->mutex);

    CloseHandle(client->disconnectEvent);
    CloseHandle(client->ptySizeChangedEvent);
    CloseHandle(client->flushEvent);
    CloseHandle(client->breakEvent);
    CloseHandle(client->dataArrivedEvent);
    CloseHandle(client->dataConsumedEvent);

    CloseHandle(client->clientStarted);
    CloseHandle(client->clientStopped);

    CloseHandle(client->forwardingConfigChanged);
    CloseHandle(client->forwards_mutex);
    free(client);
}


/** Allocates a new ssh_client_state_t which holds all the global state for the
 * SSH Thread
 *
 * @param parameters SSH Client parameters
 */
static ssh_client_state_t* ssh_client_state_new(ssh_parameters_t* parameters,
                                                ssh_client_t *client) {
    ssh_client_state_t * state;

    debug(F100, "sshsubsys - ssh_client_state_new", NULL, 0);

    state = malloc(sizeof(ssh_client_state_t));
    state->parameters = parameters;
    state->client = client;
    state->pty_width = parameters->pty_width;
    state->pty_height = parameters->pty_height;
    state->session = NULL;
    state->ttyChannel = NULL;
    state->forwardingReadyRead = CreateEvent(NULL, TRUE, FALSE, NULL);
    state->forwarding_ok = TRUE;

    debug(F100, "sshsubsys - ssh_client_state_new - get term dimensions", NULL, 0);
    get_current_terminal_dimensions(&state->pty_height, &state->pty_width);

    debug(F100, "sshsubsys - ssh_client_state_new - done", NULL, 0);
    return state;
}


/** Frees a ssh_client_state_t
 */
static void ssh_client_state_free(ssh_client_state_t * state) {
    debug(F100, "sshsubsys - freeing client state", "", 0);
    ssh_parameters_free(state->parameters);

    close_tty_channel(state);

    if (state->session != NULL) {
        ssh_disconnect(state->session);
        ssh_free(state->session);
        state->session = NULL;
    }

    CloseHandle(state->forwardingReadyRead);

    free(state);
}


/** Disconnects the SSH client, frees all resources, and sets the exit status.
 *
 * @param state Client State
 * @param client SSH Client interface struct
 * @param exit_status Exit status
 */
static void ssh_client_close(ssh_client_state_t* state, ssh_client_t *client,
                      int exit_status) {

    char* error_message = NULL;

    debug(F100, "sshsubsys - Subsystem shutdown", "exit_status", exit_status);

    /* Grab a copy of the libssh error message if there is one */
    if (exit_status == SSH_ERR_SSH_ERROR && state != NULL) {
        error_message = _strdup(ssh_get_error(state->session));
        debug(F110, "sshsubsys - have libssh error message", error_message, 0);
    }

    /* Close the existing socket if we were supplied one rather than
     * letting libssh create it */
    if (state->parameters->existing_socket != INVALID_SOCKET) {
        closesocket(state->parameters->existing_socket);
    }

    if (state != NULL) {
        ssh_client_state_free(state);
        state = NULL;
    }

    if (acquire_mutex(client->mutex, INFINITE)) {
        client->error = exit_status;
        client->error_message = error_message;

        SetEvent(client->clientStopped);

        /* We don't own the ssh_client_t - whoever started the thread is
         * responsible for cleaning it up. We do need to signal to anything that
         * is waiting on further input (or output) that the client is dead
         * though otherwise it could be waiting a long time... */
        ring_buffer_signal_error(client->inputBuffer, exit_status);
        ring_buffer_signal_error(client->outputBuffer, exit_status);

        ReleaseMutex(client->mutex);
        debug(F100, "sshsubsys - SSH subsystem stop signalled", "", 0);
    }
}


/** LibSSH logging callback. Forwards all libssh logging on to the C-Kermit
 * logging infrastructure.
 *
 * @param priority Priority - smaller is more important
 * @param function Function that produced the log message
 * @param buffer   Log message
 * @param userdata User data.
 */
void logging_callback(int priority, const char *function,
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

    snprintf(buf, msglen, "sshsubsys - libssh - [%s, %d] %s: %s",
             timebuf, priority, function, buffer);

    debug(F100, buf, "", 0);

    free(buf);
}


/** SSH Authentication callback for password and publickey auth
 *
 * @param prompt Prompt to be displayed
 * @param buf Buffer to save the password. Should be null terminated.
 * @param len Length of the buffer.
 * @param echo Enable or disable the echo of what you type
 * @param verify Should the password be verified?
 * @param userdata Userdata to be passed to the callback function
 */
int auth_prompt(const char* prompt, char* buf, size_t len, int echo,
                       int verify, void* userdata) {

    debug(F110, "sshsubsys - ssh auth_prompt", prompt, 0);
    debug(F111, "sshsubsys - ssh auth_prompt", "echo", echo);
    debug(F111, "sshsubsys - ssh auth_prompt", "verify", verify);
    debug(F111, "sshsubsys - ssh auth_prompt", "len", len);

    if (verify) {
        struct txtbox tb[2];
        char *verifyBuf;
        static char again[10] = "Again:";
        int rc;

        verifyBuf = malloc(len * sizeof(char));

        tb[0].t_buf = buf;
        tb[0].t_len = len;
        tb[0].t_lbl = NULL;
        tb[0].t_dflt = NULL;
        tb[0].t_echo = echo ? 1 : 2;

        tb[1].t_buf = verifyBuf;
        tb[1].t_len = len;
        tb[1].t_lbl = again;
        tb[1].t_dflt = NULL;
        tb[1].t_echo = echo ? 1 : 2;

        rc = uq_mtxt((char*)prompt, NULL, 2, tb);
        if (rc == 0) {
            debug(F100, "sshsubsys - auth_prompt - user canceled", "", 0);
            rc = -1; /* failed */
        } else {
            if (strncmp(buf, verifyBuf, len) == 0) {
                rc = 0; /* Success */
            } else {
                debug(F100, "sshsubsys - auth_prompt - verify failed", "", 0);
                rc = -1; /* error */
            }
        }

        free(verifyBuf);
        return rc;
    } else {
        int rc;
        rc = uq_txt(NULL, (char*)prompt, echo ? 1 : 2, NULL, buf, len, NULL,
                    DEFAULT_UQ_TIMEOUT);
        if (rc == 1) return 0; /* 1 == success */
        debug(F100, "sshsubsys - auth_prompt - user canceled", "", 0);
        return -1; /* 0 = error - user canceled */
    }
}


/** Converts a log verbosity level (0-7) to a libssh verbosity level.
 * @param verbosity Logging verbosity.
 * @returns libssh logging verbosity level
 */
static int log_verbosity(int verbosity) {
    /* ssh_vrb is set via "set ssh verbose" and has a range of 0-7
     * libssh only has 5 logging verbosity levels so 5/6/7 are unused
     * and treated the same as 4 (max verbosity)
     * */
    switch (verbosity) {
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


/** Checks to see if the host being connected to is known.
 *
 * @param state SSH Client State
 * @return An error code (0 = success)
 */
static int verify_known_host(ssh_client_state_t * state) {
    int rc = 0;
    ssh_key server_pubkey = NULL;
    unsigned char* hash = NULL;
    size_t hash_length = 0;
    enum ssh_known_hosts_e host_state;
    const char* key_type;
    char* hexa;
    char msg[2048], msg2[2048];;
    char* error_msg;

    rc = ssh_get_server_publickey(state->session, &server_pubkey);
    if (rc != SSH_OK) {
        debug(F100, "Failed to get public key", "", 0);
        return SSH_ERR_SSH_ERROR;
    }

    key_type = ssh_key_type_to_char(ssh_key_type(server_pubkey));

    rc = ssh_get_publickey_hash(server_pubkey,
                                SSH_PUBLICKEY_HASH_SHA256,
                                &hash,
                                &hash_length);
    ssh_key_free(server_pubkey);
    if (rc != SSH_OK) {
        debug(F100, "Failed to get public key hash", "", 0);
        return SSH_ERR_SSH_ERROR;
    }

    host_state = ssh_session_is_known_server(state->session);

    /*
     * TODO: Can we check for simultaneous IP and host key change (DNS Spoofing)
     *       like openssh does?
     *
     * TODO: We should also include the servers IP *and* hostname in some
     *       messages rather than only the user-supplied hostname-or-ip.
     */

    switch (host_state) {
        case SSH_KNOWN_HOSTS_OK:
            /* Cool! */
            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            char* user_knownhosts_file;

            hexa = ssh_get_hexa(hash, hash_length);

            ssh_options_get(state->session, SSH_OPTIONS_KNOWNHOSTS,
                            &user_knownhosts_file);

            hexa = ssh_get_hexa(hash, hash_length);

            rc = SSH_ERR_HOST_VERIFICATION_FAILED;

            snprintf(msg, sizeof(msg),
                      "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
                      "@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @\n"
                      "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
            printf(msg);
            snprintf(msg, sizeof(msg),
                      "IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!\n"
                      "Someone could be eavesdropping on you right now (man-in-the-middle attack)!\n"
                      "It is also possible that a host key has just been changed.\n"
                      "The fingerprint for the %s key sent by the remote host is\n"
                      "%s\nPlease contact your system administrator.\n"
                      "Add correct host key in %s to get rid of this message.\n",
                      key_type, hexa, user_knownhosts_file);

            ssh_string_free_char(user_knownhosts_file);

            /* TODO: Ideally we'd output the file the key was found in and the
             *       line the key was on. But there is currently no easy way to
             *       get this information out of libssh */

            if (state->parameters->host_key_checking_mode == SSH_SHK_YES) {
                snprintf(msg2, sizeof(msg2),
                         "\n%s host key for '%s' has changed and you have "
                         "requested strict checking.\nHost key verification "
                         "failed.\n", state->parameters->hostname, key_type);
                strncat(msg, msg2, sizeof(msg) - strlen(msg2) - 1);
            } else if (state->parameters->host_key_checking_mode == SSH_SHK_NO) {
                BOOL port_forwarding_requested = FALSE;
                if (state->parameters->allow_password_auth) {
                    strncat(msg, "Password authentication is disabled to avoid trojan horses.\n",
                            sizeof(msg) - strlen(msg) - 1);
                    state->parameters->allow_password_auth = FALSE;
                }

#ifdef SSH_AGENT_SUPPORT
                if (state->parameters->agent_forwarding) {
                    strncat(msg, "Agent forwarding is disabled to avoid trojan horses.\n",
                            sizeof(msg) - strlen(msg) - 1);
                    state->parameters->agent_forwarding = FALSE;
                }
#endif

                if (state->parameters->forward_x) {
                    strncat(msg, "X11 forwarding is disabled to avoid trojan horses.\n",
                            sizeof(msg) - strlen(msg) - 1);
                    state->parameters->forward_x = FALSE;
                }

                /* Check if we were asked to forward any ports */
                if (state->parameters->port_forwards != NULL) {
                    const ssh_port_forward_t *fwd = state->parameters->port_forwards;

                    /* Search through the list to see if there is at least one
                     * forwarding present */
                    while (fwd->type != SSH_PORT_FORWARD_NULL) {
                        if (fwd->type != SSH_PORT_FORWARD_INVALID) {
                            port_forwarding_requested = TRUE;
                            break;
                        }
                        fwd++;
                    }
                }
                if (port_forwarding_requested) {
                    strncat(msg, "Port forwarding is disabled to avoid trojan horses.\n",
                            sizeof(msg) - strlen(msg) - 1);
                }
                state->forwarding_ok = FALSE;

                /* TODO: Should we also forbid sending environment variables? */

                /* Otherwise we allow connection to proceed */
                rc = SSH_ERR_NO_ERROR;
            } else if (state->parameters->host_key_checking_mode != SSH_SHK_ASK) {
                printf("Invalid strict host key check value!\n");
            }

            printf(msg);
#ifdef COMMENT
            /* This won't work from SSH_DLL */
#ifdef KUI
            snprintf(msg2, sizeof(msg2), "REMOTE HOST IDENTIFICATION HAS CHANGED! \n\n%s", msg);
            MessageBox(hwndConsole,
                         msg2,
                         "WARNING - POTENTIAL SECURITY BREACH",
                         MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
#endif
#endif /* COMMENT */

            if (state->parameters->host_key_checking_mode == SSH_SHK_ASK) {
                snprintf(msg, sizeof(msg),
                         "The authenticity of host '%.200s' can't be established.\n"
                         "%s key fingerprint is %s\n",
                         state->parameters->hostname, key_type, hexa );
                if (uq_ok(msg,
                             "Are you sure you want to continue connecting (yes/no)? ",
                            3, NULL, -1)) {
                    rc = SSH_ERR_NO_ERROR;
                } else {
                    printf("Aborted by user!\n");
                }
            }

            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);

            return rc;
        case SSH_KNOWN_HOSTS_OTHER:
            rc = SSH_ERR_NO_ERROR;

            snprintf(msg, sizeof(msg),
                     "Warning: Server key type has changed. An attacker may change the "
                     "server key type to confuse clients into thinking the key does "
                     "not exist.\n" );
            if (!uq_ok(msg,
                         "Are you sure you want to continue connecting (yes/no)? ",
                            3, NULL, -1)) {
                printf("Aborted by user!\n");
                rc = SSH_ERR_HOST_VERIFICATION_FAILED;
            }

            ssh_clean_pubkey_hash(&hash);
            return rc;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            printf("Could not find the known hosts file. If you accept the key here "
                   "it will be created automatically.\n");
            /* fall through */
        case SSH_KNOWN_HOSTS_UNKNOWN:
            /*
             * The server is unknown. The user must confirm the public key hash is
             * correct.
             */
            if (state->parameters->host_key_checking_mode == SSH_SHK_YES) {
                snprintf(msg, sizeof(msg),
                         "No host key is known for %s and\n"
                         "you have requested strict host checking.\n"
                         "If you wish to make an untrusted connection,\n"
                         "SET SSH STRICT-HOST-KEY-CHECK OFF and try again.",
                         state->parameters->hostname);
                printf(msg);

                uq_ok(NULL, msg, 1, NULL, 0);

                ssh_clean_pubkey_hash(&hash);
                return SSH_ERR_HOST_VERIFICATION_FAILED;
            } else if (state->parameters->host_key_checking_mode = SSH_SHK_ASK) {
                hexa = ssh_get_hexa(hash, hash_length);
                snprintf(msg, sizeof(msg),
                         "The authenticity of host '%s' can't be established.\n"
                         "The key fingerprint is %s.\n",
                         state->parameters->hostname, hexa);
                ssh_string_free_char(hexa);
                ssh_clean_pubkey_hash(&hash);

                if (!uq_ok(msg, "Are you sure you want to continue "
                                "connecting (yes/no)? ", 3, NULL, -1)) {
                    printf("Aborted by user!");
                    ssh_clean_pubkey_hash(&hash);
                    return SSH_ERR_HOST_VERIFICATION_FAILED;
                }
            } else if (state->parameters->host_key_checking_mode != SSH_SHK_NO) {
                printf("Invalid Strict Host Key Check value!");
                ssh_clean_pubkey_hash(&hash);
                return SSH_ERR_HOST_VERIFICATION_FAILED;
            }

            rc = ssh_session_update_known_hosts(state->session);
            if (rc < 0) {
                printf("Error %s\n", strerror(errno));
                ssh_clean_pubkey_hash(&hash);
                return SSH_ERR_SSH_ERROR;
            }
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            printf("Error %s", ssh_get_error(state->session));
            ssh_clean_pubkey_hash(&hash);
            return SSH_ERR_SSH_ERROR;
    }

    ssh_clean_pubkey_hash(&hash);
    return SSH_ERR_NO_ERROR;
}


/** Attempt password authentication
 *
 * @param state SSH Client State
 * @param canceled out - set to True if the user cancels
 * @return SSH_AUTH_SUCCESS on success, SSH_AUTH_DENIED on failure,
 *         SSH_AUTH_ERROR on serious failure.
 */
static int password_authenticate(ssh_client_state_t * state, BOOL *canceled) {
    char *user = NULL;
    char password[256] = "";
    char prompt[1024] = "";
    int i = 0;
    int rc = 0;
    int ok = FALSE;

    debug(F110, "sshsubsys - Attempting password authentication for user", user, 0);

    rc = ssh_options_get(state->session, SSH_OPTIONS_USER, &user);
    if (rc != SSH_OK) {
        debug(F100, "sshsubsys - SSH - Failed to get user ID", "rc", rc);
        return rc;
    }

    for (i = 0; i < SSH_MAX_PASSWORD_PROMPTS; i++) {
        if (i == 0 && state->parameters->password) {
            /* Password has already been supplied. Try that */
            debug(F100, "sshsubsys - Using pre-entered password", "", 0);
            ckstrncpy(password,state->parameters->password,sizeof(password));
            SecureZeroMemory(state->parameters->password, sizeof(state->parameters->password));
            free(state->parameters->password);
            state->parameters->password = NULL;
            ok = TRUE;
        } else {
            snprintf(prompt, sizeof(prompt), "%s%.30s@%.128s's password: ",
                     i == 0 ? "" : "Permission denied, please try again.\n",
                     user, state->parameters->hostname);
            /* Prompt user for password */
            ok = uq_txt(prompt,"Password: ",2,NULL,password, sizeof(password),NULL,
                        DEFAULT_UQ_TIMEOUT);
        }

        if (!ok || strcmp(password, "") == 0) {
            /* User canceled */
            debug(F100, "sshsubsys - User canceled password login", "", 0);
            ssh_string_free_char(user);
            *canceled = TRUE;
            return SSH_AUTH_DENIED;
        }

        rc = ssh_userauth_password(state->session, NULL, password);
        SecureZeroMemory(password, sizeof(password));
        if (rc == SSH_AUTH_SUCCESS) {
            debug(F100, "sshsubsys - Password login succeeded", "", 0);
            ssh_string_free_char(user);
            return rc;
        } else if (rc == SSH_AUTH_ERROR) {
            debug(F111, "sshsubsys - SSH Auth Error - password login failed", "rc", rc);
            /* A serious error has occurred.  */
            ssh_string_free_char(user);
            return rc;
        } else if (rc == SSH_AUTH_PARTIAL) {
            debug(F100, "sshsubsys - SSH Partial authentication - "
                        "more authentication needed", "", 0);
            ssh_string_free_char(user);
            return rc;
        }
        /* Else: SSH_AUTH_DENIED - try again. */
        debug(F100, "sshsubsys - SSH Password auth failed - access denied", "", 0);
    }

    ssh_string_free_char(user);
    return rc;
}


/** Attempt keyboard interactive authentication
 *
 * @param state SSH Client State
 * @param canceled out - set to True if the user cancels
 * @return SSH_AUTH_SUCCESS on success, SSH_AUTH_DENIED on failure,
 *         SSH_AUTH_ERROR on serious failure.
 */
static int kbd_interactive_authenticate(ssh_client_state_t * state, BOOL *canceled) {
    int rc;

    debug(F100, "sshsubsys - kbd_interactive_authenticate", "", 0);

    rc = ssh_userauth_kbdint(state->session, NULL, NULL);

    while (rc == SSH_AUTH_INFO) {
        const char* name, *instructions;
        int nprompts, i, combined_text_len;
        struct txtbox *tb = NULL;
        char** responses = NULL;
        char* combined_instructions = NULL;
        BOOL failed = FALSE;

        name = ssh_userauth_kbdint_getname(state->session);
        instructions = ssh_userauth_kbdint_getinstruction(state->session);
        nprompts = ssh_userauth_kbdint_getnprompts(state->session);

        debug(F110, "sshsubsys - kbd_int_auth name", name, 0);
        debug(F110, "sshsubsys - kbd_int_auth instructions", instructions, 0);
        debug(F101, "sshsubsys - kbd_int_auth prompts", "", nprompts);

        if (nprompts == 0) {
            debug(F100, "sshsubsys - No more prompts! Unable to continue "
                        "interrogating user.", "nprompts", nprompts);

            /* Some SSH servers send an empty query at the end of the exchange
             * for some reason. Check if the server is really sure there are
             * more prompts... */

            rc = ssh_userauth_kbdint(
                    state->session, NULL, NULL);
            if (rc == SSH_AUTH_INFO)
                debug(F101, "sshsubsys - ssh_userauth_kbdint still insists "
                            "there are more prompts than it originally "
                            "reported. Giving up.", "", rc);
            else debug(F101, "sshsubsys - ssh_userauth_kbdint has decided "
                             "actually there are no more prompts. We're done. ",
                             "", rc);
            break;
        }

        combined_text_len = 0;
        if (name) combined_text_len += strlen(name);
        if (instructions) combined_text_len += strlen(instructions);

        if (combined_text_len > 0) {
            /* Only prepare instructions if the server sent us at least a name
             * or instructions. If both were null then we'll skip this
             * entirely.*/
            combined_instructions += 100;

            combined_instructions = malloc(combined_text_len * sizeof(char));
            if (combined_instructions == NULL) {
                debug(F100, "sshsubsys - kbd_interactive_authenticate - failed "
                            "to malloc for instructions", "", 0);
                return SSH_AUTH_ERROR;
            }
            snprintf(combined_instructions,
                     combined_text_len,
                     "--- %s ---\n%s",
                     name,
                     instructions
            );
        }

        if (nprompts == 1) {
            /*
             * If there is only one prompt then we'll use the single field
             * uq_txt instead of the multi-field uq_mtxt as it looks a lot
             * less wierd - especially when there is no instruction text.
             */
            char echo;
            char buffer[128];
            const char *prompt = ssh_userauth_kbdint_getprompt(
                    state->session, 0, &echo);

            debug(F110, "sshsubsys - kdbint auth - single prompt mode - prompt:",
                  prompt, 0);

            rc = uq_txt(combined_instructions, (char*)prompt, echo ? 1 : 2, NULL,
                        buffer, sizeof(buffer), NULL, DEFAULT_UQ_TIMEOUT);
            if (rc == 1) {
                rc = ssh_userauth_kbdint_setanswer(
                        state->session, 0, buffer);
                debug(F111, "sshsubsys - ssh_userauth_kbdint says", "rc", rc);
                if (rc < 0) {
                    /* An error of some kind occurred. Don't bother feeding
                     * in any further responses. We'll only keep going around
                     * the loop to clean up the response array */
                    failed = TRUE;
                    debug(F101, "sshsubsys - prompt rejected", "", i);
                }
            } else {
                debug(F100, "sshsubsys - kdbint auth - user canceled", "", 0);
                failed = TRUE;
                *canceled = TRUE;
            }
        } else {
            /*
             * More than one prompt this time around. We'll use uq_mtxt and ask
             * for all of them in one go.
             */

            /* Allocate an array of textboxes to hold the prompts */
            tb = (struct txtbox *) malloc(sizeof(struct txtbox) * nprompts);
            if (tb == NULL) {
                debug(F100, "sshsubsys - kbd_interactive_authenticate - textbox malloc failed", "", 0);
                return SSH_AUTH_ERROR;
            }
            memset(tb, 0, sizeof(struct txtbox) * nprompts);

            /* Allocate an array to hold all the responses. */
            responses = malloc(sizeof(char *) * nprompts);
            if (responses == NULL) {
                debug(F100, "sshsubsys - kbd_interactive_authenticate - response array malloc failed", "", 0);
                return SSH_AUTH_ERROR;
            }

            /* Build up an array of text boxes to show the user */
            for (i = 0; i < nprompts; i++) {
                char echo;
                const char *prompt = ssh_userauth_kbdint_getprompt(
                        state->session, i, &echo);
                responses[i] = malloc(128 * sizeof(char));

                debug(F111, "sshsubsys - kdb_interactive_authenticate prompt", prompt, i);

                if (responses[i] == NULL) {
                    debug(F111, "sshsubsys - kbd_interactive_authenticate - response buffer "
                                "malloc failed", "response", i);
                    return SSH_AUTH_ERROR;
                }

                memset(responses[i], 0, sizeof(responses[i]));

                tb[i].t_buf = responses[i];
                tb[i].t_len = 128;
                tb[i].t_lbl = (char*)prompt;
                tb[i].t_dflt = NULL;
                tb[i].t_echo = echo ? 1 /* yes */ : 2; /* no - asterisks */
            }

            /* Ask the user all the prompts in one go  */
            rc = uq_mtxt(combined_instructions, NULL, nprompts, tb);

            if (rc == 0) { /* 0 = no/cancel, 1 = yes/ok */
                debug(F100, "sshsubsys - kdbint auth - user canceled", "", 0);
                failed = TRUE;
                *canceled = TRUE;
            }

            /* Then process the responses freeing buffers as we go */
            for (i = 0; i < nprompts; i++) {
                debug(F101, "sshsubsys - processing prompt", "", i);
                if (!failed) {
                    /* User hasn't canceled and haven't hit an error yet. Ask the
                     * server what it thinks of an answer.*/
                    rc = ssh_userauth_kbdint_setanswer(
                            state->session, i, responses[i]);
                    debug(F111, "sshsubsys - ssh_userauth_kbdint says", "rc", rc);
                    if (rc < 0) {
                        /* An error of some kind occurred. Don't bother feeding
                         * in any further responses. We'll only keep going around
                         * the loop to clean up the response array */
                        failed = TRUE;
                        debug(F101, "sshsubsys - prompt rejected", "", i);
                    }
                }
                memset(responses[i], 0, sizeof(responses[i]));
                if (responses[i]) {
                    free(responses[i]);
                    responses[i] = NULL;
                }
            }
            if (responses) {
                free(responses);
                responses = NULL;
            }
            if (tb) {
                free(tb);
                tb = NULL;
            }
        }
        if (combined_instructions) {
            free(combined_instructions);
            combined_instructions = NULL;
        }
        if (failed) {
            /* Now that all the resources have been cleaned up we can finally
             * act on that failure */
            return SSH_AUTH_ERROR;
        }

        /* See if we need to go round again and ask *more* questions */
        rc = ssh_userauth_kbdint(
                state->session, NULL, NULL);
        if (rc == SSH_AUTH_INFO) debug(F101, "sshsubsys - ssh_userauth_kbdint says SSH_AUTH_INFO", "", rc);
    }

    debug(F111, "sshsubsys - ssh kbdint finished with", "rc", rc);

    return rc;
}

void print_auth_methods(int methods, BOOL debugOnly) {
    if (debugOnly) {
        debug(F100, "sshsubsys - supported authentication methods:", "", 0);
        if (methods & SSH_AUTH_METHOD_PUBLICKEY) debug(F110, "sshsubsys\t- ","publickey",0);
        if (methods & SSH_AUTH_METHOD_INTERACTIVE) debug(F110, "sshsubsys\t- ","keyboard-interactive", 0);
        if (methods & SSH_AUTH_METHOD_GSSAPI_MIC) debug(F110, "sshsubsys\t- ","gssapi-mic", 0);
        if (methods & SSH_AUTH_METHOD_PASSWORD) debug(F110, "sshsubsys\t- ","password", 0);
    } else {
        if (methods & SSH_AUTH_METHOD_PUBLICKEY) printf("publickey ");
        if (methods & SSH_AUTH_METHOD_INTERACTIVE) printf("keyboard-interactive ");
        if (methods & SSH_AUTH_METHOD_GSSAPI_MIC) printf("gssapi-mic ");
        if (methods & SSH_AUTH_METHOD_PASSWORD) printf("password ");
    }
}

/** Attempt to authenticate the user using one of the methods supported by
 * the server.
 *
 * @param state SSH Client State
 * @param canceled out - set to True if the user cancels
 * @returns SSH_AUTH_SUCCESS on success, SSH_AUTH_DENIED on denied,
 *          SSH_AUTH_ERROR on a serious error.
 */
static int authenticate(ssh_client_state_t * state, BOOL *canceled) {
    int methods = 0, rc, attemptedMethods = 0;
    BOOL no_auth_methods = TRUE;

    /* If the user cancels anytime during the authentication process this will
     * be set, and we'll know not to attempt any further authentication methods
     */
    *canceled = FALSE;

    rc = ssh_userauth_none(state->session, NULL);
    if (rc == SSH_AUTH_SUCCESS) {
        /* Authenticated anonymously!? */
        return SSH_ERR_NO_ERROR;
    } else if (rc == SSH_AUTH_ERROR) {
        /* A serious error of some kind happened. We can not proceed */
        return SSH_ERR_AUTH_ERROR;
    }

    for (int i = 0; i < MAX_AUTH_METHODS; i++) {

        if (!state->parameters->authentication_methods[i]) {
            /* Out of authentication methods */
            break;
        }

        /* Check to see if the set of available auth methods has
         * changed - some SSH servers may vary what is allowed based
         * on past authentication steps */
        int new_methods = ssh_userauth_list(state->session, NULL);
        if (new_methods != methods) {
            print_auth_methods(methods, TRUE);
        }
        methods = new_methods;

        switch(state->parameters->authentication_methods[i]) {
            case SSH_AUTH_METHOD_NONE:
                if (methods & SSH_AUTH_METHOD_NONE) {
                    debug(F110, "sshsubsys - Attempting auth method", "none", 0);
                    no_auth_methods = FALSE;
                    attemptedMethods |= SSH_AUTH_METHOD_NONE;
                    rc = ssh_userauth_none(state->session, NULL);
                } else {
                    debug(F110, "sshsubsys - auth method not supported by server at this time", "none", 0);
                }
                break;
            case SSH_AUTH_METHOD_PASSWORD:
                if (methods & SSH_AUTH_METHOD_PASSWORD
                        && state->parameters->allow_password_auth) {
                    debug(F110, "sshsubsys - Attempting auth method", "password", 0);
                    no_auth_methods = FALSE;
                    attemptedMethods |= SSH_AUTH_METHOD_PASSWORD;
                    rc = password_authenticate(state, canceled);
                } else {
                    debug(F110, "sshsubsys - auth method not supported by server at this time", "password", 0);
                }
                break;
            case SSH_AUTH_METHOD_PUBLICKEY:
                if (methods & SSH_AUTH_METHOD_PUBLICKEY) {
                    debug(F110, "sshsubsys - Attempting auth method", "publickey", 0);
                    no_auth_methods = FALSE;
                    attemptedMethods |= SSH_AUTH_METHOD_PUBLICKEY;
                    rc = ssh_userauth_publickey_auto(state->session, NULL, NULL);
                } else {
                    debug(F110, "sshsubsys - auth method not supported by server at this time", "publickey", 0);
                }
                break;
            case SSH_AUTH_METHOD_INTERACTIVE:
                if (methods & SSH_AUTH_METHOD_INTERACTIVE) {
                    debug(F110, "sshsubsys - Attempting auth method", "interactive", 0);
                    no_auth_methods = FALSE;
                    attemptedMethods |= SSH_AUTH_METHOD_INTERACTIVE;
                    rc = kbd_interactive_authenticate(state, canceled);
                } else {
                    debug(F110, "sshsubsys - auth method not supported by server at this time", "interactive", 0);
                }
                break;
            case SSH_AUTH_METHOD_GSSAPI_MIC:
                if (methods & SSH_AUTH_METHOD_GSSAPI_MIC) {
                    debug(F110, "sshsubsys - Attempting auth method", "gssapi", 0);
                    no_auth_methods = FALSE;
                    attemptedMethods |= SSH_AUTH_METHOD_GSSAPI_MIC;
                    rc = ssh_userauth_gssapi(state->session);
                } else {
                    debug(F110, "sshsubsys - auth method not supported by server at this time", "gssapi", 0);
                }
                break;
        }

        if (*canceled) {
            printf("User canceled.\n");
            return SSH_ERR_USER_CANCELED;
        }

        if (rc == SSH_AUTH_SUCCESS) {
            /* Done! */
            return rc;
        }
    }

    if (no_auth_methods) {
        printf("No supported authentication methods!\n");
        printf("The server supports: ");
        print_auth_methods(methods, FALSE);
        printf("\n");
    } /*else {
        printf("Authentication failed after attempting the following methods:\n\t");
        print_auth_methods(attemptedMethods, FALSE);
        printf("\n");
    }*/

    return rc;
}


/** Opens the TTY channel and nothing else.
 *
 * @param state SSH Client State
 * @return SSH_OK on success, else an error
 */
static int open_tty_channel(ssh_client_state_t * state) {
    int rc = 0;

    if (state->ttyChannel != NULL) {
        return SSH_OK; /* already open */
    }

    debug(F100, "sshsubsys - Opening SSH tty channel", "", 0);

    state->ttyChannel = ssh_channel_new(state->session);
    if (state->ttyChannel == NULL) {
        debug(F100, "sshsubsys - Failed to create channel", "", 0);
        return SSH_ERR_SSH_ERROR;
    }

    rc = ssh_channel_open_session(state->ttyChannel);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - Channel open failed", "rc", rc);
        ssh_channel_free(state->ttyChannel);
        state->ttyChannel = NULL;
    }

    return rc;
}


/** Closes the tty channel if its currently open.
 *
 * @param state SSH Client State
 */
static void close_tty_channel(ssh_client_state_t * state) {
    debug(F100, "sshsubsys - TTY channel close requested", "", 0);
    if (state->ttyChannel) {
        debug(F100, "sshsubsys - Closing ssh tty channel", "", 0);
        ssh_channel_send_eof(state->ttyChannel);
        ssh_channel_close(state->ttyChannel);
        ssh_channel_free(state->ttyChannel);
        state->ttyChannel = NULL;
    }
}


/** Run a command on the remote host then disconnect.
 *
 * @param state SSH Client State
 * @param command Command to run
 * @return SSH_OK on success, or an error
 */
static int ssh_rexec(ssh_client_state_t * state, const char* command) {
    int rc;

    debug(F110,"sshsubsys - ssh running command", command, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_exec(state->ttyChannel, command);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - SSH exec failed", "rc", rc);
        close_tty_channel(state);
        return rc;
    }

    debug(F100, "sshsubsys - SSH exec succeeded", "", 0);

    return rc;
}


/** Opens the tty channel for a subsystem.
 *
 * @param state SSH Client State
 * @param subsystem Subsystem to request
 * @return SSH_OK on success, or an error.
 */
static int ssh_subsystem(ssh_client_state_t * state, const char* subsystem) {
    int rc;

    debug(F110,"sshsubsys - ssh requesting subsystem", subsystem, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_pty_size(state->ttyChannel,
                                      state->parameters->terminal_type,
                                      state->pty_width,
                                      state->pty_height);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - PTY request failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_subsystem(state->ttyChannel, subsystem);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - SSH subsystem request failed", "rc", rc);
        close_tty_channel(state);
        return rc;
    }

    debug(F100, "sshsubsys - SSH subsystem request succeeded", "", 0);

    return rc;
}


/** Opens the tty channel for a shell. This also sets up a PTY.
 *
 * @param state SSH Client State
 * @return SSH_OK on success, or an error.
 */
static int open_shell(ssh_client_state_t * state) {
    int rc;

    debug(F100, "sshsubsys - SSH open shell", "", 0);

    debug(F111, "sshsubsys - SSH pty request", "rows", state->pty_height);
    debug(F111, "sshsubsys - SSH pty request", "cols", state->pty_width);
    debug(F111, "sshsubsys - SSH pty request - termtype: ",
          state->parameters->terminal_type, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - open tty channel failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_pty_size(state->ttyChannel,
                                      state->parameters->terminal_type,
                                      state->pty_width,
                                      state->pty_height);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - PTY request failed", "rc", rc);
        return rc;
    }

    /* Send environment varaibles */
    for (int i = 0; i < MAX_ENVIRONMENT_VARIABLES; i++) {
        if (state->parameters->environment_variables[i][0] != NULL &&
            state->parameters->environment_variables[i][1] != NULL) {

            rc = ssh_channel_request_env(
                state->ttyChannel,
                state->parameters->environment_variables[i][0],  /* name */
                state->parameters->environment_variables[i][1]); /* value */

            if (rc != SSH_OK) {
                debug(F111, "sshsubsys - failed to set environment variable", "rc", rc);
                debug(F110, "sshsubsys - failed to set environment variable",
                        state->parameters->environment_variables[i][0], 0);
            }
        }
    }

    rc = ssh_channel_request_shell(state->ttyChannel);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - Shell request failed", "rc", rc);
        return rc;
    }

    return rc;
}

ssh_channel auth_agent_request_callback(ssh_session session, void *userdata);

/** Applies all configuration to the SSH Session
 *
 * @param state SSH Client state
 * @returns An error code if anything goes wrong
 */
static int configure_session(ssh_client_state_t * state) {
    int rc = SSH_ERR_OK;
    int verbosity = SSH_LOG_PROTOCOL;

    /* TODO: ssh_channel_callbacks_struct
     *   Can we make use of the channel_data_function and channel_eof_function
     *   callbacks to handle initiating copies to the various ring buffers?
     *   And channel_close_function to handle tidying up forwardings?
     * */
    static struct ssh_callbacks_struct cb = {
            .auth_function = auth_prompt,
#ifdef SSH_AGENT_SUPPORT
            .channel_open_request_auth_agent_function =
                auth_agent_request_callback
#endif
    };

    cb.userdata = state;

    ssh_callbacks_init(&cb);

    /* Set options */
    debug(F100, "sshsubsys - Configure session...", "", 0);
    verbosity = log_verbosity(state->parameters->log_verbosity);
    ssh_options_set(state->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_set_callbacks(state->session, &cb);
    ssh_options_set(state->session, SSH_OPTIONS_HOST, state->parameters->hostname);
    if (state->parameters->existing_socket != INVALID_SOCKET) {
        ssh_options_set(state->session, SSH_OPTIONS_FD,
                        &(state->parameters->existing_socket));
    }
    ssh_options_set(state->session, SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS,
                    &state->parameters->gssapi_delegate_credentials);
    ssh_options_set(state->session, SSH_OPTIONS_PROCESS_CONFIG,
                    &state->parameters->use_openssh_config);
    ssh_options_set(state->session, SSH_OPTIONS_NODELAY,
                    &state->parameters->nodelay);
    if (!state->parameters->compression) {
        ssh_options_set(state->session, SSH_OPTIONS_COMPRESSION_C_S, "no");
        ssh_options_set(state->session, SSH_OPTIONS_COMPRESSION_S_C, "no");
    }

    if (state->parameters->allowed_ciphers) {
        ssh_options_set(state->session, SSH_OPTIONS_CIPHERS_C_S,
                        state->parameters->allowed_ciphers);
        ssh_options_set(state->session, SSH_OPTIONS_CIPHERS_S_C,
                        state->parameters->allowed_ciphers);
    }

    if (state->parameters->allowed_hostkey_algorithms) {
        ssh_options_set(state->session, SSH_OPTIONS_HOSTKEYS,
                        state->parameters->allowed_hostkey_algorithms);
    }
    if (state->parameters->macs) {
        ssh_options_set(state->session, SSH_OPTIONS_HMAC_C_S,
                        state->parameters->macs);
        ssh_options_set(state->session, SSH_OPTIONS_HMAC_S_C,
                        state->parameters->macs);
    }
    if (state->parameters->key_exchange_methods) {
        ssh_options_set(state->session, SSH_OPTIONS_KEY_EXCHANGE,
                        state->parameters->key_exchange_methods);
    }
    if (state->parameters->proxy_command) {
        ssh_options_set(state->session, SSH_OPTIONS_PROXYCOMMAND,
                        state->parameters->proxy_command);
    }
    if (state->parameters->ssh_dir) {
        ssh_options_set(state->session, SSH_OPTIONS_SSH_DIR,
                        state->parameters->ssh_dir);
    }

    if (state->parameters->port)
        ssh_options_set(state->session, SSH_OPTIONS_PORT_STR, state->parameters->port);
    ssh_options_set(state->session, SSH_OPTIONS_USER, state->parameters->username);
    if (state->parameters->user_known_hosts_file)
        ssh_options_set(state->session, SSH_OPTIONS_KNOWNHOSTS,
                        state->parameters->user_known_hosts_file);
    if (state->parameters->global_known_hosts_file)
        ssh_options_set(state->session, SSH_OPTIONS_GLOBAL_KNOWNHOSTS,
                        state->parameters->global_known_hosts_file);

    if (state->parameters->identity_files != NULL) {
        int i = 0;
        while (state->parameters->identity_files[i] != NULL) {
            debug(F111, "Add identity file", state->parameters->identity_files[i], i);
            ssh_options_set(state->session, SSH_OPTIONS_ADD_IDENTITY ,
                            state->parameters->identity_files[i]);
            i++;
        }
    }

    if (state->parameters->agent_location) {
        debug(F110, "sshsubsys - set agent location", state->parameters->agent_location, 0);
        ssh_options_set(state->session, SSH_OPTIONS_IDENTITY_AGENT,
                        state->parameters->agent_location);
    }

    // identity fields (set ssh identity-file)
    // stored in ssh_idf[32]
    // add with SSH_OPTIONS_ADD_IDENTITY

    if (state->parameters->use_openssh_config) {
        /* Parse OpenSSH Config */
        rc = ssh_options_parse_config(state->session,
                                      NULL);  /* Use default filename */

    }
    return rc;
}


/** Reads any available data from the TTY Channel and writes it to
 * the output ring buffer.
 *
 * @param state SSH Client State
 * @param client SSH Client Interface
 * @returns An error code on failure
 */
static int ssh_tty_read(ssh_client_state_t* state, ssh_client_t *client) {
    int rc = 0, net_available;
    char* buf = NULL;

    /* Check if there is actually anything to do before we go acquiring locks
     * on buffers */
    net_available = ssh_channel_poll(state->ttyChannel, 0);
    if (net_available == 0) {
        debug(F100, "sshsubsys - tty-read - nothing to read", "", 0);
        return SSH_ERR_OK;
    } else if (net_available < 0) {
        debug(F111, "sshsubsys - tty-read - error on poll", "rc", net_available);
        return rc;
    }

    debug(F100, "sshsubsys - tty-read", "", 0);

    /* Read from the TTY channel into the output buffer */
    if (ring_buffer_lock(client->outputBuffer, INFINITE)) {
        int space_available, space_used;

        space_available = ring_buffer_free_space(client->outputBuffer);
        space_used = ring_buffer_length(client->outputBuffer);

        debug(F101, "sshsubsys - tty-read - buffer space used", "", space_used);
        debug(F101, "sshsubsys - tty-read - buffer space available", "", space_available);

        if (space_available > 0) {
            buf = malloc(sizeof(char) * space_available);
            if (buf != NULL) {
                /* rc is number of bytes read */
                rc = ssh_channel_read_timeout(state->ttyChannel,
                                              buf,
                                              space_available,
                                              0,
                                              0);

                if (rc != SSH_ERROR) {
                    debug(F101, "sshsubsys - tty-read - read bytes from network", "", rc);

                    size_t written = ring_buffer_write(
                            client->outputBuffer, buf, rc);

                    if (written < rc) {
                        debug(F111,
                              "sshsubsys - ERROR: fewer bytes written to output buffer "
                              "than available free space", "free",
                              space_available);

                        /* TODO: It would be nice to retry but really this should
                         *       never happen (it would be a pretty big problem if
                         *       it did!)*/
                        rc = SSH_ERR_BUFFER_WRITE_FAILED;
                    }
                } else {
                    debug(F100, "sshsubsys - tty-read - error reading from network", "", 0);
                }
            }
        }
        ring_buffer_unlock(client->outputBuffer);
    }

    if (buf != NULL) {
        free(buf);
    }

    if (rc >= 0) {
        rc = SSH_ERR_OK;
    }
    return rc;
}


/** Writes any available data in the input ring buffer to the TTY Channel.
 *
 * @param state SSH Client State
 * @param client SSH Client Interface
 * @returns An error code on failure
 */
int ssh_tty_write(ssh_client_state_t* state, ssh_client_t *client) {
    int rc = 0;

    /* Read from the input buffer and write it to the tty channel */
    if (ring_buffer_lock(client->inputBuffer, INFINITE)) {
        char* buf;

        size_t bytes_available;

        bytes_available = ring_buffer_length(client->inputBuffer);
        debug(F101, "sshsubsys - tty-write - bytes available", "", bytes_available);

        if (bytes_available > 0) {

            buf = malloc(sizeof(char) * bytes_available);
            if (buf != NULL) {
                size_t bytes_read = ring_buffer_peek(
                        client->inputBuffer, buf, bytes_available);

                debug(F101, "sshsubsys - tty-write - "
                            "got bytes from input buffer", "", bytes_read);

                /* rc is the number of bytes written to the channel */
                rc = ssh_channel_write(state->ttyChannel, buf, bytes_read);

                if (rc != SSH_ERROR) {
                    BOOL result = ring_buffer_consume(
                            client->inputBuffer, rc);

                    if (!result) {
                        debug(F111, "sshsubsys - SSH - Failed to consume bytes from "
                                    "the input ring buffer", "bytes",
                              rc);
                        /* This should never happen and there is not much
                         * we can do if it does. The ring buffer is now
                         * inconsistent with what we have delivered to K95.
                         * We should probably drop the connection.*/

                        rc = SSH_ERR_BUFFER_CONSUME_FAILED;
                    }
                }

                free(buf);
            }
        }

        ring_buffer_unlock(client->inputBuffer);
    }

    if (rc >= 0) {
        rc = SSH_ERR_OK;
    }
    return rc;
}


/** Acquires a lock on the forwarding config
 *
 * @param client Client instance
 * @param msTimeout Timeout
 * @return
 */
BOOL ssh_forwarding_lock(ssh_client_t* client, DWORD msTimeout) {
    DWORD dwWaitResult;

    if (client->forwards_mutex == NULL) {
        debug(F100, "sshsubsys - no forwarding config mutex.", "", 0);
        return FALSE;
    };

    dwWaitResult = WaitForSingleObject(
            client->forwards_mutex,
            msTimeout);
    switch(dwWaitResult) {
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        case WAIT_ABANDONED:
            debug(F100, "sshsubsys - forwarding mutex acquired in abandoned "
                        "state. Forwarding config may be in an inconsistent "
                        "state", "", 0);
            return TRUE;
        case WAIT_FAILED:
            debug(F101, "sshsubsys - wait on forwarding mutex failed with "
                        "error", "", GetLastError());
            return FALSE;
    }
    return FALSE; /* Should never happen */
}

/** Releases a lock ont he forwarding config
 *
 * @param client  Client instance
 */
void ssh_forwarding_unlock(ssh_client_t* client) {
    if (client->forwards_mutex == NULL) {
        debug(F100, "sshsubsys - forwarding mutex does not exist!", "", 0);
        return;
    };
    ReleaseMutex(client->forwards_mutex);
}


/** Adds a new direct SSH forward to the active list of forwards.
 * Forwarding will begin (nearly) immediately if an SSH session is
 * currently active.
 *
 * @param client SSH Client instance
 * @param remoteHost Remote host to connect to
 * @param remotePort Remote port to connect to
 * @param localHost Local hostname (mostly for logging purposes on the server)
 * @param localPort Local port to listen on for new connections
 * @param timeout Maximum time to wait before giving up on this request
 * @return The new forward instance, or NULL if the timeout expired.
 */
ssh_forward_t * add_ssh_direct_forward(ssh_client_t* client,
                           char* remoteHost,
                           int remotePort,
                           char* localHost,
                           int localPort,
                           int timeout) {

    debug(F111, "sshsubsys - Add direct port forward: remote", remoteHost, remotePort);
    debug(F111, "sshsubsys - Add direct port forward: local", localHost, localPort);
    debug(F101, "sshsubsys - Add direct port forward: timeout", NULL, timeout);

    if (ssh_forwarding_lock(client, timeout)) {
        debug(F100, "sshsubsys - Add direct port forward: got mutex", NULL, 0);

        ssh_forward_t *fwd = malloc(sizeof(ssh_forward_t));
        fwd->next = NULL;
        fwd->state = SSH_FWD_STATE_PENDING;
        fwd->connections = NULL;

        fwd->type = SSH_FWD_TYPE_DIRECT;
        fwd->remoteHost = _strdup(remoteHost);
        fwd->remotePort = remotePort;
        fwd->localHost = _strdup(localHost);
        fwd->localPort = localPort;

        if (client->forwards == NULL) {
            client->forwards = fwd;
        } else {
            /* Find the last entry in the list */
            ssh_forward_t * node = client->forwards;
            while (node->next != NULL) {
                node = node->next;
            }
            node->next = fwd;
        }

        ssh_forwarding_unlock(client);
        SetEvent(client->forwardingConfigChanged);
        return fwd;
    }
    debug(F100, "sshsubsys - failed to get a lock on the forwarding "
                "config to add new entry.", NULL, 0);
    return NULL;
}

/** Adds a new reverse SSH forward to the active list of forwards.
 * Forwarding will begin (nearly) immediately if an SSH session is
 * currently active.
 *
 * @param client SSH Client instance
 * @param remoteHost Remote address to listen on
 * @param remotePort Remote port to listen on
 * @param localHost Local host to connect to
 * @param localPort Local port to connect to
 * @param timeout Maximum time to wait before giving up on this request
 * @return The new forward instance, or NULL if the timeout expired.
 */
ssh_forward_t * add_ssh_reverse_forward(ssh_client_t *client,
                                        char* remoteHost,
                                        int remotePort,
                                        char* localHost,
                                        int localPort,
                                        int timeout) {
    debug(F111, "sshsubsys - Add reverse port forward: remote", remoteHost, remotePort);
    debug(F111, "sshsubsys - Add reverse port forward: local", localHost, localPort);
    debug(F101, "sshsubsys - Add reverse port forward: timeout", NULL, timeout);

    if (ssh_forwarding_lock(client, timeout)) {
        debug(F100, "sshsubsys - Add reverse port forward: got mutex", NULL, 0);

        ssh_forward_t *fwd = malloc(sizeof(ssh_forward_t));
        fwd->next = NULL;
        fwd->state = SSH_FWD_STATE_PENDING;
        fwd->connections = NULL;

        fwd->type = SSH_FWD_TYPE_REVERSE;
        fwd->remoteHost = _strdup(remoteHost);
        fwd->remotePort = remotePort;
        fwd->localHost = _strdup(localHost);
        fwd->localPort = localPort;

        if (client->forwards == NULL) {
            client->forwards = fwd;
        } else {
            /* Find the last entry in the list */
            ssh_forward_t * node = client->forwards;
            while (node->next != NULL) {
                node = node->next;
            }
            node->next = fwd;
        }

        ssh_forwarding_unlock(client);
        SetEvent(client->forwardingConfigChanged);
        return fwd;
    }
    debug(F100, "sshsubsys - failed to get a lock on the forwarding "
                "config to add new entry.", NULL, 0);
    return NULL;
}


/** Opens a new channel for a new direct (local) port forward
 * connection.
 *
 * @param client Client instance
 * @param state Client state
 * @param fwd Direct SSH forward
 * @param con Connection being setup.
 * @return
 */
static int open_direct_forward(ssh_client_t* client,
                               ssh_client_state_t* state,
                               ssh_forward_t* fwd,
                               ssh_forward_connection_t* con) {
    int rc = SSH_ERROR;

    debug(F100, "sshsubsys - open direct port forward...", NULL, 0);

    /* ssh_forward_t must be in the PENDING (ready to open) state. */
    if (con->state != SSH_FWD_STATE_PENDING) {
        debug(F100, "sshsubsys - ERROR - cannot open direct forward - "
                    "forward not in pending state", NULL, fwd->state);
        return rc;
    }

    con->channel = ssh_channel_new(state->session);
    if (con->channel == NULL) {
        debug(F101, "sshsubsys - failed to create channel for direct "
                    "port forward", NULL, rc);
        return rc; /* Fail */
    }

    rc = ssh_channel_open_forward(con->channel,
                                  fwd->remoteHost,
                                  fwd->remotePort,
                                  fwd->localHost,
                                  fwd->localPort);
    if (rc != SSH_OK) {
        debug(F101, "sshsubsys - failed to open direct forward channel",
              NULL, rc);
        ssh_channel_free(con->channel);
        con->channel = NULL;
        return rc;
    }

    return rc;
}


/** Starts a TCP server to listen to incoming direct forward
 * connections. You should have the forwarding config mutex
 * before calling this outside of connection startup.
 *
 * @param fwd SSH direct forward configuration
 * @return SSH_ERR_NO_ERROR on success, or an error.
 */
static int start_direct_forward_server(ssh_forward_t *fwd) {
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int rc;
    char localPort[32];
    u_long mode = 1;  /* Socket mode 1 for non-blocking */

    if (fwd->type != SSH_FWD_TYPE_DIRECT) {
        debug(F111, "sshsubsys - ERROR: attempted to start direct forward "
                    "server for non-direct SSH forward. This is likely a bug. "
                    "Supplied type was:", "type",
                    fwd->type);
        return SSH_ERR_UNSPECIFIED;
    }

    debug(F111, "sshsubsys - Starting direct forward server", "local port", fwd->localPort);
    debug(F111, "sshsubsys - remote host&port", fwd->remoteHost, fwd->remotePort);

    fwd->state = SSH_FWD_STATE_ERROR;

    /* Setup hints */
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;          /* IPv4 address family */
             /* Use AF_INET6 for IPv6, or PF_UNSPEC for either */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    /* Resolve server address and port */
    snprintf(localPort, 32, "%d", fwd->localPort);
    rc = getaddrinfo(fwd->localHost,
                     localPort,
                     &hints,
                     &result);
    if (rc != 0) {
        debug(F101, "sshsubsys - call to getaddrinfo failed while "
                    "setting up direct forward server", NULL, rc);
        return SSH_ERR_DIRECTFWD_GETADDRINFO_FAILED;
    }

    /* Create socket to listen for client connections */
    fwd->listen_socket = socket(
            result->ai_family,
            result->ai_socktype,
            result->ai_protocol
            );
    if (fwd->listen_socket == INVALID_SOCKET) {
        debug(F101, "sshsubsys - failed to create listen socket while setting "
                    "up direct forward server", NULL, WSAGetLastError());
        freeaddrinfo(result);
        return SSH_ERR_LISTEN_SOCKET_CREATE_FAILED;
    }

    /* set up the listen socket */
    rc = bind(fwd->listen_socket,
              result->ai_addr,
              (int)result->ai_addrlen);
    if (rc == SOCKET_ERROR) {
        debug(F101, "sshsubsys - bind failed while setting up direct forward "
                    "server", NULL, rc);
        freeaddrinfo(result);
        closesocket(fwd->listen_socket);
        fwd->listen_socket = INVALID_SOCKET;
        return SSH_ERR_DIRECTFWD_BIND_FAILED;
    }

    /* Set the server socket to non-blocking as it is being dealt with on
     * the main SSH thread */
    ioctlsocket(fwd->listen_socket, FIONBIO, &mode);

    /* Don't need this anymore */
    freeaddrinfo(result);
    result = NULL;

    rc = listen(fwd->listen_socket, SOMAXCONN);
    if (rc == SOCKET_ERROR) {
        debug(F101, "sshsubsys  listen failed while setting up direct forward "
                    "server", NULL, rc);
        closesocket(fwd->listen_socket);
        fwd->listen_socket = INVALID_SOCKET;
        return SSH_ERR_DIRECTFWD_LISTEN_FAILED;
    }

    /* We're listening for connections! */
    fwd->state = SSH_FWD_STATE_OPEN;

    return SSH_ERR_NO_ERROR;
}


/** Asks the server to start listening for connections on
 * the specified port. If the port is 0 the server is free
 * to choose one. The server will be asked to listen on all
 * addresses/interfaces unless a particular address is specified
 * via the remoteHost value.
 *
 * @param fwd SSH reverse forwarding configuration
 * @return SSH_OK on success, or an error.
 */
static int start_reverse_forward_server(ssh_forward_t *fwd,
                                        ssh_client_state_t *state) {
    int rc, bound_port;

    rc = ssh_channel_listen_forward(state->session,
                                    fwd->remoteHost,
                                    fwd->remotePort,
                                    &bound_port);
    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - reverse forward start listen - "
                    "error opening remote port",
              ssh_get_error(state->session), rc);
        fwd->state = SSH_FWD_STATE_ERROR;
        return rc;
    }

    /* If the user did not request a particular port on the server... */
    if (fwd->remotePort == 0) {
        /* then update the forwarding config with the servers chosen port */
        fwd->remotePort = bound_port;
    }

    fwd->state = SSH_FWD_STATE_OPEN;

    return SSH_OK;
}

/** Starts listening for new connections for the specified
 * forwarding config.
 *
 * @param fwd Forwarding config to start listening for connections on
 * @return 0 on success, <0 on error
 */
static int start_forward_server(ssh_forward_t *fwd,
                                ssh_client_state_t *state) {
    int rc = SSH_ERR_OK;

    if (fwd->type == SSH_PORT_FORWARD_LOCAL) {
        rc = start_direct_forward_server(fwd);

        if (rc != SSH_ERR_NO_ERROR) {
            /* Failed to start listening */
            debug(F100, "sshsubsys - forwarding entry state set "
                        "to error", NULL, 0);
        }
    } else if (fwd->type == SSH_PORT_FORWARD_REMOTE) {
        /* Start listening. No need to acquire the config
                             * mutex here as nothing else will be messing with
                             * it yet.*/
        rc = start_reverse_forward_server(fwd, state);
        if (rc != SSH_OK) {
            debug(F100, "sshsubsys - failed to start reverse "
                        "forward server", NULL, 0);
        }
    }

    return rc;
}



/** Starts X11 forwarding on the tty channel.
 *
 * @param state Client state
 * @return 0 on success.
 */
static int start_X11_forwarding(ssh_client_state_t *state) {
    char *proto = NULL, *cookie = NULL;
    int rc;

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        printf("Failed to open tty channel for X11 forwarding!\n");
    }

    /* TODO: Call xauth (if specified) to get the proto and cookie.
     *      (K95 2.1.3 never actually did this, despite having an xauth
     *      location setting)
     */

    rc = ssh_channel_request_x11(
            state->ttyChannel,
            0,      /* Only one X11 app will be redirected? No. */
            proto,  /* X11 authentication protocol. NULL to use MIT-MAGIC-COOKIE-1. */
            cookie, /* X11 authentication cookie. NULL to generate a random one. */
            0);     /* Screen number */
    if (rc != SSH_OK) {
        printf("X11 forwarding request failed: %d\n", rc);
    }
    return rc;
}

/** Reads any available data from a port forwarding Channel and writes it to
 * the output ring buffer to be consumed by the thread handling that
 * connection.
 *
 * @param fwd Port forwarding connection
 * @returns An error code on failure
 */
static int ssh_port_fwd_read(ssh_forward_connection_t *fwd) {
    int rc = 0, net_available;
    char* buf = NULL;

    /* Check if there is actually anything to do before we go acquiring locks
     * on buffers */
    net_available = ssh_channel_poll(fwd->channel, 0);
    if (net_available == 0) {
        debug(F100, "sshsubsys - fwd-read - nothing to read", "", 0);
        return SSH_ERR_OK;
    } else if (net_available < 0) {
        debug(F111, "sshsubsys - fwd-read - error on poll", "rc", net_available);
        /* TODO: This probably indicates the channel is dead and we should
         *       terminate the connection */
        return rc;
    }

    debug(F100, "sshsubsys - fwd-read", "", 0);

    /* Read from the port forwarding channel into the output buffer */
    if (ring_buffer_lock(fwd->outputBuffer, INFINITE)) {
        int space_available, space_used;

        space_available = ring_buffer_free_space(fwd->outputBuffer);
        space_used = ring_buffer_length(fwd->outputBuffer);

        debug(F101, "sshsubsys - fwd-read - buffer space used", "", space_used);
        debug(F101, "sshsubsys - fwd-read - buffer space available", "", space_available);

        if (space_available > 0) {
            buf = malloc(sizeof(char) * space_available);
            if (buf != NULL) {
                /* rc is number of bytes read */
                rc = ssh_channel_read_timeout(fwd->channel,
                                              buf,
                                              space_available,
                                              0,
                                              0);

                if (rc != SSH_ERROR) {
                    debug(F101, "sshsubsys - fwd-read - read bytes from channel", "", rc);

                    size_t written = ring_buffer_write(
                            fwd->outputBuffer, buf, rc);

                    if (written < rc) {
                        debug(F111,
                              "sshsubsys - ERROR: fewer bytes written to output buffer "
                              "than available free space", "free",
                              space_available);

                        /* TODO: It would be nice to retry but really this should
                         *       never happen (it would be a pretty big problem if
                         *       it did!)*/
                        rc = SSH_ERR_BUFFER_WRITE_FAILED;
                    }
                } else {
                    debug(F100, "sshsubsys - fwd-read - error reading from network", "", 0);
                }
            }
        }
        ring_buffer_unlock(fwd->outputBuffer);
    }

    if (buf != NULL) {
        free(buf);
    }

    if (rc >= 0) {
        rc = SSH_ERR_OK;
    }
    return rc;
}


/** Writes any available data in the input ring buffer to the port forwarding
 * channel.
 *
 * @param fwd Port forwarding connection
 * @returns An error code on failure
 */
static int ssh_port_fwd_write(ssh_forward_connection_t *fwd) {
    int rc = 0;

    /* Read from the input buffer and write it to the ports channel */
    if (ring_buffer_lock(fwd->inputBuffer, INFINITE)) {
        char* buf;

        size_t bytes_available;

        bytes_available = ring_buffer_length(fwd->inputBuffer);
        debug(F101, "sshsubsys - fwd-write - input bytes available", "", bytes_available);

        if (bytes_available > 0) {

            buf = malloc(sizeof(char) * bytes_available);
            if (buf != NULL) {
                size_t bytes_read = ring_buffer_peek(
                        fwd->inputBuffer, buf, bytes_available);

                debug(F101, "sshsubsys - fwd-write - "
                            "got bytes from forwarding input buffer", "", bytes_read);

                /* rc is the number of bytes written to the channel */
                rc = ssh_channel_write(fwd->channel, buf, bytes_read);

                if (rc != SSH_ERROR) {
                    BOOL result = ring_buffer_consume(
                            fwd->inputBuffer, rc);

                    if (!result) {
                        debug(F111, "sshsubsys - SSH - Failed to consume bytes from "
                                    "the forwarding input ring buffer", "bytes",
                              rc);
                        /* This should never happen and there is not much
                         * we can do if it does. The ring buffer is now
                         * inconsistent with what we have delivered to CKW.
                         * We should probably drop the connection.*/

                        rc = SSH_ERR_BUFFER_CONSUME_FAILED;
                    }
                }

                free(buf);
            }
        }

        ring_buffer_unlock(fwd->inputBuffer);
    }

    if (rc >= 0) {
        rc = SSH_ERR_OK;
    }
    return rc;
}


/** This thread handles copying data between a socket and a pair of
 * ring buffers. Whenever data is copied to the output ring buffer,
 * a global (to the SSH connection) event is set to indicate one or
 * more forwarding sockets have produced data.
 *
 * This allows the SSH thread to wait on a single event for all
 * sockets, rather than having to wait on each socket. This lets
 * us avoid Windows' limit on the number of objects a thread can
 * wait on at any given time.
 *
 * @param parameters Thread parameters
 * @return
 */
unsigned int __stdcall ssh_forwarding_connection_thread(
        ssh_fwd_thread_params_t *parameters) {
    int rc;
    HANDLE events[3];
    BOOL disconnect = FALSE;
    char* buf = NULL;

    debug(F101, "sshsubsys - forwarding connection thread start for socket", NULL, parameters->socket);

    events[0] = WSACreateEvent();
    WSAEventSelect(parameters->socket, events[0], FD_READ | FD_WRITE);
    events[1] = ring_buffer_get_ready_read_event(parameters->inputBuffer);
    events[2] = parameters->disconnectEvent;
    /* TODO: A disconnect event */

    while(TRUE) {
        WSANETWORKEVENTS netEvents;
        DWORD waitResult;

        debug(F111, "sshsubsys - fwd-thread -waiting...", "socket", parameters->socket);
        waitResult = WSAWaitForMultipleEvents(
                3, /* Number of events */
                events, /* Array of events to wait on */
                FALSE, /* Return when *any* event is signalled, rather than all */
                1000, /* Wait for up to 1s */
                TRUE /* Place thread in alertable wait state to allow execution of
                      * I/O completion routines. */
        );

        if (waitResult == WSA_WAIT_FAILED) {
            int error = WSAGetLastError();
            debug(F111, "sshsubsys - fwd-thread - Wait failed!", "error", error);
            switch(error) {
                case WSANOTINITIALISED:
                    debug(F100, "sshsubsys - fwd-thread - WSA Not Initialised", "", 0);
                    break;
                case WSAENETDOWN:
                    debug(F100, "sshsubsys - fwd-thread - WSA - network subsystem failure", "", 0);
                    break;
                case WSAEINPROGRESS:
                    debug(F100, "sshsubsys - fwd-thread - WSA - A blocking winsock 1.1 call is in progress", "", 0);
                    break;
                case WSA_NOT_ENOUGH_MEMORY:
                    debug(F100, "sshsubsys - fwd-thread - WSA - not enough memory", "", 0);
                    break;
                case WSA_INVALID_HANDLE:
                    debug(F100, "sshsubsys - fwd-thread - WSA - invalid handle", "", 0);
                    break;
                case WSA_INVALID_PARAMETER:
                    debug(F100, "sshsubsys - fwd-thread - WSA - invalid parameter", "", 0);
                    break;
            }

            rc = SSH_ERR_WAIT_FAILED;
            break;
        } else if (waitResult == WSA_WAIT_IO_COMPLETION) {
            continue;
        }

        debug(F111, "sshsubsys - fwd-thread - wake!", "socket", parameters->socket);

        /* Check if we've been told to disconnect */
        if (WaitForSingleObjectEx(parameters->disconnectEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F111, "sshsubsys - forwarding connection thread disconnect signalled", "socket", parameters->socket);
            break;
        }

        /* Reset the socket and ready-read events */
        WSAResetEvent(events[0]);
        WSAResetEvent(events[1]);

        rc = SSH_ERR_OK;

        /* Read from the input buffer and write it to the socket */
        if (ring_buffer_lock(parameters->inputBuffer, INFINITE)) {
            BOOL disconnect = FALSE;

            size_t bytes_available;

            bytes_available = ring_buffer_length(parameters->inputBuffer);
            debug(F101, "sshsubsys - fwd-thread - input bytes available", "", bytes_available);

            if (bytes_available > 0) {

                buf = malloc(sizeof(char) * bytes_available);
                if (buf != NULL) {
                    BOOL error = FALSE;

                    size_t bytes_read = ring_buffer_peek(
                            parameters->inputBuffer, buf, bytes_available);

                    debug(F101, "sshsubsys - fwd-thread - "
                                "got bytes from forwarding input buffer",
                                "", bytes_read);

                    /* rc is the number of bytes written to the channel */
                    rc = send(parameters->socket, buf, bytes_read, 0);

                    if (rc != SOCKET_ERROR) {
                        BOOL result = ring_buffer_consume(
                                parameters->inputBuffer, rc);

                        if (!result) {
                            debug(F111,
                                  "sshsubsys - fwd-thread - Failed to consume "
                                  "bytes from the forwarding input ring buffer",
                                  "bytes",
                                  rc);
                            /* This should never happen and there is not much
                             * we can do if it does. The ring buffer is now
                             * inconsistent with what we have delivered to CKW.
                             * We should probably drop the connection.*/

                            rc = SSH_ERR_BUFFER_CONSUME_FAILED;
                            disconnect = TRUE;
                        }
                    } else {
                        int err = WSAGetLastError();
                        if (err != WSAEWOULDBLOCK) {
                            debug(F101, "sshsubsys - fwd-thread - Error writing to "
                                        "socket", NULL, err);
                            disconnect = TRUE;

                            /* TODO: We should probably put this error somewhere the
                             *       user can see it and set the connect state to
                             *       ERROR */
                        }
                    }

                    free(buf);
                }
            }

            ring_buffer_unlock(parameters->inputBuffer);
        }

        if (disconnect) {
            break;
        }

        /* Read from the socket into the output buffer */
        if (ring_buffer_lock(parameters->outputBuffer, INFINITE)) {
            int space_available, space_used;

            space_available = ring_buffer_free_space(parameters->outputBuffer);
            space_used = ring_buffer_length(parameters->outputBuffer);

            /*debug(F101, "sshsubsys - fwd-thread - buffer space used", "", space_used);
            debug(F101, "sshsubsys - fwd-thread - buffer space available", "", space_available);*/

            if (space_available > 0) {
                buf = malloc(sizeof(char) * space_available);
                if (buf != NULL) {
                    /* rc is number of bytes read */
                    rc = recv(parameters->socket, buf, space_available, 0);

                    if (rc > 0) {
                        debug(F101, "sshsubsys - fwd-thread - read bytes from "
                                    "network", "", rc);


                        size_t written = ring_buffer_write(
                                parameters->outputBuffer, buf, rc);
                        SetEvent(parameters->readyRead);

                        if (written < rc) {
                            debug(F111,
                                  "sshsubsys - fwd-thread - ERROR: fewer bytes"
                                  " written to output buffer "
                                  "than available free space", "free",
                                  space_available);

                            /* TODO: It would be nice to retry but really this should
                             *       never happen (it would be a pretty big problem if
                             *       it did!)*/
                            rc = SSH_ERR_BUFFER_WRITE_FAILED;
                        }
                    } else if (rc == 0) {
                        debug(F111, "sshsubsys - fwd-thread - socket closed ",
                              "socket", parameters->socket);
                        disconnect = TRUE;
                    } else {
                        rc = WSAGetLastError();
                        if (rc != WSAEWOULDBLOCK) {
                            debug(F101, "sshsubsys - fwd-thread - error reading from "
                                        "network ",
                                  NULL, rc);
                            disconnect = TRUE;
                            /* TODO: We should probably put this error somewhere the
                             *       user can see it and set the connect state to
                             *       ERROR*/
                        }
                    }
                    free(buf);
                }
            } else {
                debug(F111, "sshsubsys - fwd-thread - output buffer full", "socket", parameters->socket);
            }
            ring_buffer_unlock(parameters->outputBuffer);
        }

        if (disconnect) {
            break;
        }
    }

    debug(F111, "sshsubsys - forwarding connection thread terminate", "socket", parameters->socket);

    WSACloseEvent(events[0]);
    free(parameters);
    return 0;
}


#ifdef SSH_AGENT_SUPPORT
/** Accepts an agent forwarding request.
 *
 * @param session Session to accept a forwarding request for
 * @param userdata This should be the ssh client state
 * @returns a new ssh channel if the request is accepted
 */
ssh_channel auth_agent_request_callback(ssh_session session, void *userdata) {
    ssh_client_state_t *state = (ssh_client_state_t*)userdata;
    ssh_client_t *client = NULL;
    ssh_forward_t *fwd = NULL;
    ssh_channel channel = NULL;
    struct sockaddr_un addr;
    SOCKET sock = INVALID_SOCKET;
    ssh_forward_connection_t *con, *conNode;
    ssh_fwd_thread_params_t *threadParams;
    int iResult = 0;

    debug(F100, "sshsubsys - Received agent-request", NULL, 0);

    if (state == NULL) {
        debug(F100, "sshsubsys - Regecting agent request: state is NULL!", NULL, 0);
        return channel;
    }

    client = state->client;

    if (!state->parameters->agent_forwarding) {
        /* Agent forwarding not enabled */
        debug(F100, "sshsubsys - Rejecting agent-request: not enabled", NULL, 0);
        return channel;
    }

    /* Try to find the forwarding entry. This is what will track the forwarding
     * connection allowing it to be cleaned up properly later. Its localHost
     * parameter contains the local socket the agent is listening on*/
    fwd = client->forwards;
    while (fwd && fwd->next != NULL) {
        if (fwd->type == SSH_FWD_TYPE_AGENT)
            break;

        fwd = fwd->next;
    }

    if (fwd == NULL || fwd->type != SSH_FWD_TYPE_AGENT) {
        debug(F100, "sshsubsys - Rejecting agent-request: "
                    "agent forwarding not configured", NULL, 0);
        return channel;
    }

    /*
     *  From here on, it's all Unix Domain Sockets (AF_UNIX). This is
     *  incompatible with both the version of ssh-agent supplied with Windows,
     *  and how Pageant normally works.
     *
     *  Ideally we'd support Named Pipes here too as this is what ssh-agent
     *  always uses, and what Pageant uses by default. But right now libssh
     *  doesn't support Named Pipes for agent authentication, so there is
     *  little point in supporting them for agent forwarding.
     */

    /* Create the AF_UNIX socket */
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", fwd->localHost);

    /* Convert socket name to windows path separators (K95 uses unix
     * separators internally) */
    for (int i = 0; i < sizeof(addr.sun_path); i++) {
        if (addr.sun_path[i] == '\0')
            break;
        if (addr.sun_path[i] == '/')
            addr.sun_path[i] = '\\';
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        debug(F110, "sshsubsys - Rejecting agent-request: "
                    "socket create failed", fwd->localHost, 0);
        return channel;
    }

    /* And connect it */
    iResult = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (iResult == SOCKET_ERROR) {
        /* Connect failed */
        closesocket(sock);
        sock = INVALID_SOCKET;
        debug(F111, "sshsubsys - Rejecting agent-request: "
                    "failed to connect to agent socket",
              addr.sun_path, iResult);
        return channel;
    }

    /* We're connected! Create a channel */
    channel = ssh_channel_new(state->session);

    /* Create the connection entry to track this agent forwarding */
    con = malloc(sizeof(ssh_forward_connection_t));
    con->next = NULL;
    con->channel = channel;
    con->outputBuffer = NULL;
    con->inputBuffer = NULL;
    con->state = SSH_FWD_STATE_OPEN;

    con->disconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    /* Add the connection to the list */
    if (fwd->connections == NULL) {
        fwd->connections = con;
    } else {
        conNode = fwd->connections;
        while (conNode->next != NULL) {
            conNode = conNode->next;
        }
        conNode->next = con;
    }

    /* Set up the I/O buffers */
    con->outputBuffer = ring_buffer_new(1024*1024);
    con->inputBuffer = ring_buffer_new(1024*1024);

    /* The socket */
    con->socket = sock;

    /* Then launch a thread to handle the socket */
    debug(F100, "sshsubsys - new agent fwd connection, launch thread", NULL, 0);
    threadParams = malloc(sizeof(ssh_fwd_thread_params_t));
    threadParams->socket = con->socket;
    threadParams->outputBuffer = con->inputBuffer;
    threadParams->inputBuffer = con->outputBuffer;
    threadParams->readyRead = state->forwardingReadyRead;
    threadParams->disconnectEvent = con->disconnectEvent;
    con->thread = (HANDLE)_beginthreadex(
            NULL,           /* Security info */
            65536,          /* Stack size */
            ssh_forwarding_connection_thread,     /* Start address */
            (void *)threadParams,   /* Arg list */
            0,              /* init flags - start immediately */
            NULL            /* Thread identifier */
    );

    return channel;
}
#endif


/** Accepts any waiting connections for direct forwarding.
 *
 */
static void accept_direct_forwarding_connections(
        ssh_client_state_t *clientState,
        ssh_client_t *client,
        ssh_forward_t *fwd) {
    SOCKET sock = SOCKET_ERROR;
    debug(F100, "sshsubsys - checking for incoming direct fwd connections", NULL, 0);

    do {
        sock = accept(fwd->listen_socket, NULL, NULL);
        debug(F101, "sshsubsys - accept says:", NULL, sock);

        if (sock != SOCKET_ERROR) {
            ssh_forward_connection_t *con, *conNode;
            ssh_fwd_thread_params_t *threadParams;

            debug(F100, "sshsubsys - new direct fwd connection", NULL, 0);

            /* New connection! */
            con = malloc(sizeof(ssh_forward_connection_t));
            con->next = NULL;
            con->channel = NULL;
            con->outputBuffer = NULL;
            con->inputBuffer = NULL;
            con->state = SSH_FWD_STATE_PENDING;

            con->disconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            /* Add the connection to the list */
            if (fwd->connections == NULL) {
                fwd->connections = con;
            } else {
                conNode = fwd->connections;
                while (conNode->next != NULL) {
                    conNode = conNode->next;
                }
                conNode->next = con;
            }

            /* Then set up the new connection */
            /* First set up a new SSH channel for it */
            open_direct_forward(client, clientState, fwd, con);

            /* Set up the I/O buffers */
            con->outputBuffer = ring_buffer_new(1024*1024);
            con->inputBuffer = ring_buffer_new(1024*1024);

            /* The socket */
            con->socket = sock;

            /* Then launch a thread to handle the socket */
            debug(F100, "sshsubsys - new direct fwd connection, launch thread", NULL, 0);
            threadParams = malloc(sizeof(ssh_fwd_thread_params_t));
            threadParams->socket = con->socket;
            threadParams->outputBuffer = con->inputBuffer;
            threadParams->inputBuffer = con->outputBuffer;
            threadParams->readyRead = clientState->forwardingReadyRead;
            threadParams->disconnectEvent = con->disconnectEvent;
            con->thread = (HANDLE)_beginthreadex(
                    NULL,           /* Security info */
                    65536,          /* Stack size */
                    ssh_forwarding_connection_thread,     /* Start address */
                    (void *)threadParams,   /* Arg list */
                    0,              /* init flags - start immediately */
                    NULL            /* Thread identifier */
            );
        }
    } while (sock != SOCKET_ERROR);
}


/** Creates a new connection to something on the client network using
 * the details specified in the supplied fwd (localHost, localPort).
 * If successful, the connection is added to the fwds list of connections
 * and a new thread is launched to service it.
 *
 * This is called when setting up new incoming remote forward connections, and
 * new X11 connections.
 *
 * @param channel SSH channel to create a new TCP/IP connection for
 * @param fwd ssh_forward_t to create a new TCP/IP connection for
 * @param clientState Client state.
 */
static void create_local_connection(ssh_channel channel,
                                    ssh_forward_t *fwd,
                                    ssh_client_state_t *clientState) {
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;
    char* peer_address = NULL;
    int port = 0, peer_port = 0, iResult=0;
    char localPort[32];
    ssh_forward_connection_t *con, *conNode;
    ssh_fwd_thread_params_t *threadParams;
    SOCKET sock = INVALID_SOCKET;

    /* Setup hints */
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;          /* IPv4 address family */
    /* Use AF_INET6 for IPv6, or PF_UNSPEC for either */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /* Resolve server address & port */
    snprintf(localPort, 32, "%d", fwd->localPort);
    iResult = getaddrinfo(fwd->localHost, localPort, &hints, &result);

    if ( iResult != 0 ) {
        /* Fail! */

        debug(F101, "sshsubsys - getaddrinfo failed with error", NULL, iResult);
        ssh_channel_send_eof(channel);
        ssh_channel_free(channel);
        return;
    }

    /* Walk through the list of addresses until we find one we can use
     * to connect to the local host and port */
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        /* Create socket */
        sock = socket(ptr->ai_family,
                      ptr->ai_socktype,
                      ptr->ai_protocol);

        if (sock == INVALID_SOCKET) {
            debug(F101, "sshsubsys - failed to create socket",
                  NULL, WSAGetLastError());
            ssh_channel_send_eof(channel);
            ssh_channel_free(channel);
            break;
        }

        // Connect to server.
        iResult = connect( sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            /* Failed to connect with this socket - try the next */
            closesocket(sock);
            sock = INVALID_SOCKET;
            return;
        }
        break;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET) {
        debug(F100, "sshsubsys - reverse/x11 fwd connect failed", NULL, 0);
        ssh_channel_send_eof(channel);
        ssh_channel_free(channel);
        return;
    }

    debug(F101, "sshsubsys - new reverse/x11 forward connected on socket",
          NULL, sock);

    /* Create a new connection instance */
    /* New connection! */
    con = malloc(sizeof(ssh_forward_connection_t));
    con->next = NULL;
    con->channel = channel;
    con->outputBuffer = NULL;
    con->inputBuffer = NULL;
    con->state = SSH_FWD_STATE_OPEN;

    con->disconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    /* Add the connection to the list */
    if (fwd->connections == NULL) {
        fwd->connections = con;
    } else {
        conNode = fwd->connections;
        while (conNode->next != NULL) {
            conNode = conNode->next;
        }
        conNode->next = con;
    }

    /* Set up the I/O buffers */
    con->outputBuffer = ring_buffer_new(1024*1024);
    con->inputBuffer = ring_buffer_new(1024*1024);

    /* The socket */
    con->socket = sock;

    /* Then launch a thread to handle the socket */
    debug(F100, "sshsubsys - new remote/x11 fwd connection, launch thread", NULL, 0);
    threadParams = malloc(sizeof(ssh_fwd_thread_params_t));
    threadParams->socket = con->socket;
    threadParams->outputBuffer = con->inputBuffer;
    threadParams->inputBuffer = con->outputBuffer;
    threadParams->readyRead = clientState->forwardingReadyRead;
    threadParams->disconnectEvent = con->disconnectEvent;
    con->thread = (HANDLE)_beginthreadex(
            NULL,           /* Security info */
            65536,          /* Stack size */
            ssh_forwarding_connection_thread,     /* Start address */
            (void *)threadParams,   /* Arg list */
            0,              /* init flags - start immediately */
            NULL            /* Thread identifier */
    );
}


/** Accepts any pending reverse forwarding connections
 *
 * @param clientState Client state we're accepting connections for
 * @param client SSH client we're accepting connections for
 * @param fwd Forwarding config we're accepting connections for.
 */
static void accept_reverse_forwarding_connections(
        ssh_client_state_t *clientState,
        ssh_client_t *client) {
    ssh_channel channel = NULL;
    debug(F100, "sshsubsys - checking for incoming direct fwd connections", NULL, 0);

    do {
        char* peer_address = NULL;
        int port = 0, peer_port = 0;
        ssh_forward_t *fwd = client->forwards;

        channel = ssh_channel_open_forward_port(
                clientState->session,
                0,      /* Timeout */
                &port,
                &peer_address,
                &peer_port
                );

        if (channel == NULL) {
            return;     /* no pending connections */
        }

        debug(F101, "sshsubsys - got SSH forwarding request - port", NULL, port);
        debug(F111, "sshsubsys - got SSH forwarding request - peer", peer_address, peer_port);

        /* Don't need this anymore now that we've logged it */
        ssh_string_free_char(peer_address);

        /* Find the matching forwarding config */
        while (fwd != NULL) {
            if (fwd->type == SSH_FWD_TYPE_REVERSE
                && fwd->remotePort == port) {
                break; /* Found it! */
            }

            fwd++;
        }

        if (fwd == NULL) {
            debug(F100, "sshsubsys - warning! Could not find forwarding "
                        "connection for received direct forwarding request. "
                        "Rejecting request.", NULL, 0);
            ssh_channel_send_eof(channel);
            ssh_channel_free(channel);
            continue;
        }

        create_local_connection(channel, fwd, clientState);

    } while (channel != NULL);
}


/** Accepts any waiting inbound X11 forwarding connections
 *
 * @param clientState Clientstate
 * @param client Client
 * @param fwd Forwarding with X server connection details (LocalHost, LocalPort)
 *            to which the new connection will be added
 */
static void accept_x11_forwarding_connections(
        ssh_client_state_t *clientState,
        ssh_client_t *client,
        ssh_forward_t *fwd) {
    ssh_channel channel = NULL;
    debug(F100, "sshsubsys - checking for incoming X11 connections", NULL, 0);

    do {
        int port = 0, iResult=0;
        ssh_forward_connection_t *con, *conNode;
        ssh_fwd_thread_params_t *threadParams;
        SOCKET sock = INVALID_SOCKET;
        ssh_forward_t *fwd = client->forwards;
        struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;
        char localPort[32];

        channel = ssh_channel_accept_x11(clientState->ttyChannel, 0);

        if (channel == NULL) {
            return; /* no pending connections */
        }

        debug(F100, "sshsubsys - got X11 forwarding request", NULL, 0);

        create_local_connection(channel, fwd, clientState);
    } while (channel != NULL);
}

/** Frees a forwarded connection struct. It will signal to the
 * connection thread to terminate and wait a short time for it
 * to do so if its still running.
 *
 * If the thread is running and does not terminate in a reasonable
 * timeframe this function will log a warning and return without
 * freeing resources.
 *
 * @param conn Connection to free.
 */
static void free_fwd_connection(ssh_forward_connection_t *conn) {

    /* Signal to the thread (if its still running) that we want it to
     * terminate */
    SetEvent(conn->disconnectEvent);

    /* Wait for the thread to terminate before freeing up its resources. If it
     * doesn't terminate in a reasonable time we'll just let its resources leak
     * rather than potentially crashing */
    if (WaitForSingleObject(conn->thread, 2000) == WAIT_OBJECT_0) {

        CloseHandle(conn->thread);
        conn->thread = NULL;
        closesocket(conn->socket);
        if (conn->channel != NULL) {
            ssh_channel_send_eof(conn->channel);
            ssh_channel_free(conn->channel);
            conn->channel = NULL;
        }
        CloseHandle(conn->disconnectEvent);
        ring_buffer_free(conn->inputBuffer);
        ring_buffer_free(conn->outputBuffer);
        conn->inputBuffer = NULL;
        conn->outputBuffer = NULL;
        conn->state = SSH_FWD_STATE_CLOSED;
        free(conn);

    } else {
        debug(F100, "sshsubsys - WARNING: SSH forwarding connection thread "
                    "failed to terminate in reasonable timeframe. Resources "
                    "will be leaked!", NULL, 0);
    }
}


static void configure_forwarding(ssh_client_state_t *state,
                                 ssh_client_t *client) {
    /* This is set to false if host key verification fails and strict host key
     * checking is set to NO.
     */
    if (state->forwarding_ok) {

        /* Set up local and remote port forwards */
        if (state->parameters->port_forwards != NULL &&
            state->parameters->port_forwards[0].type != 0) {
            debug(F100, "sshsubsys - Configuring forwarded ports..", "", 0);

            if (ssh_forwarding_lock(client, 2000)) {
                const ssh_port_forward_t *fwd = state->parameters->port_forwards;
                while (fwd->type != SSH_PORT_FORWARD_NULL) {
                    if (fwd->type == SSH_PORT_FORWARD_LOCAL) {
                        int rc;
                        ssh_forward_t *fwdNode;

                        fwdNode = add_ssh_direct_forward(
                                client,
                                fwd->hostname,
                                fwd->host_port,
                                fwd->address,
                                fwd->port,
                                INFINITE
                        );

                        if (fwdNode == NULL) {
                            continue; /* Failed to configure port forward */
                        }

                        rc = start_forward_server(fwdNode, state);

                        if (rc != SSH_ERR_NO_ERROR) {
                            /* Failed to start listening */
                            debug(F100, "sshsubsys - forwarding entry state set "
                                        "to error", NULL, 0);
                            continue;
                        }
                    } else if (fwd->type == SSH_PORT_FORWARD_REMOTE) {
                        int rc;
                        ssh_forward_t *fwdNode;

                        fwdNode = add_ssh_reverse_forward(
                                client,
                                fwd->address,  /* Server address to listen on */
                                fwd->port,     /* Server port to listen on */
                                fwd->hostname,  /* Local host to forward to */
                                fwd->host_port, /* Local port to forward to */
                                INFINITE
                        );

                        if (fwdNode == NULL) {
                            continue; /* Failed to configure port forward */
                        }

                        /* Start listening. No need to acquire the config
                         * mutex here as nothing else will be messing with
                         * it yet.*/
                        rc = start_forward_server(fwdNode, state);
                        if (rc != SSH_OK) {
                            debug(F100, "sshsubsys - failed to start reverse "
                                        "forward server", NULL, 0);
                        }
                    }

                    fwd++;
                }
                ssh_forwarding_unlock(client);
            }
        }

#ifdef SSH_AGENT_SUPPORT
        /* Set up Agent forwarding */
        if (state->parameters->agent_forwarding && ssh_forwarding_lock(client, 2000)) {
            int rc = 0;

            debug(F100, "sshsubsys - Enable agent forwarding", NULL, 0);
            ssh_forward_t *fwd = malloc(sizeof(ssh_forward_t));
            fwd->next = NULL;
            fwd->state = SSH_FWD_STATE_OPEN;
            fwd->connections = NULL;

            fwd->type = SSH_FWD_TYPE_AGENT;
            fwd->remoteHost = NULL;
            fwd->remotePort = 0;
            fwd->localHost = _strdup(state->parameters->agent_location);
            fwd->localPort = 0;

            if (client->forwards == NULL) {
                client->forwards = fwd;
            } else {
                /* Find the last entry in the list */
                ssh_forward_t * node = client->forwards;
                while (node->next != NULL) {
                    node = node->next;
                }
                node->next = fwd;
            }

            ssh_forwarding_unlock(client);

            rc = open_tty_channel(state);
            if (rc != SSH_OK) {
                printf("Failed to open tty channel for Agent forwarding!\n");
            } else {
                ssh_channel_request_auth_agent(state->ttyChannel);
            }
        }
#endif /* AGENT_FORWARDING */
    }
}

static void setup_x11_forwarding(ssh_client_state_t *state,
                                 ssh_client_t *client) {
    /* Set up X11 tunnel */
    if (state->forwarding_ok &&
        state->parameters->forward_x &&
        state->parameters->x11_host != NULL) {


        /* Add an entry to the forwards list so we have a way of
         * tracking connections to the local X server */
        ssh_forward_t *fwd = malloc(sizeof(ssh_forward_t));
        fwd->next = NULL;
        fwd->state = SSH_FWD_STATE_PENDING;
        fwd->connections = NULL;

        fwd->type = SSH_FWD_TYPE_X11;
        fwd->remoteHost = NULL;
        fwd->remotePort = 0;
        fwd->localHost = _strdup(state->parameters->x11_host);
        fwd->localPort = 6000 + state->parameters->x11_display;

        if (client->forwards == NULL) {
            client->forwards = fwd;
        } else {
            /* Find the last entry in the list */
            ssh_forward_t * node = client->forwards;
            while (node->next != NULL) {
                node = node->next;
            }
            node->next = fwd;
        }

        if (start_X11_forwarding(state) == SSH_OK) {
            fwd->state = SSH_FWD_STATE_OPEN;
        } else {
            fwd->state = SSH_FWD_STATE_ERROR;
            state->parameters->forward_x = FALSE;
        }
    }
}


/** Configures libssh and connects to the server
 *
 * @param state SSH Client State
 * @return An error code on failure
 */
static int connect_ssh(ssh_client_state_t* state, ssh_client_t *client) {
    int rc;
    BOOL user_canceled;
    char* banner = NULL;
    BOOL forwarding_ok = TRUE;

    /* Connect! */
    debug(F100, "sshsubsys - SSH Connect...", "", 0);

    /* Apply configuration to the SSH session */
    rc = configure_session(state);
    if (rc != SSH_ERR_OK) {
        return rc;
    }

    rc = ssh_connect(state->session);
    if (rc != SSH_OK) {
        debug(F111,"sshsubsys - Error connecting to host",
              ssh_get_error(state->session), rc);
        return rc;
    }

    /* Check the hosts key is valid */
    rc = verify_known_host(state);
    if (rc != SSH_ERR_NO_ERROR) {
        debug(F111, "sshsubsys - Host verification failed", "rc", rc);
        printf("Host verification failed.\n");
        return rc;
    }

    /* This is apparently required for some reason in order for
     * get_issue_banner to work */
    rc = ssh_userauth_none(state->session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        return rc;
    }
    if (rc != SSH_AUTH_SUCCESS) {
        banner = ssh_get_issue_banner(state->session);
        if (banner) {
            printf(banner);
            ssh_string_free_char(banner);
            banner = NULL;
        }

        /* Authenticate! */
        rc = authenticate(state, &user_canceled);
        if (rc != SSH_AUTH_SUCCESS ) {
            debug(F111, "sshsubsys - Authentication failed - disconnecting", "rc", rc);
            printf("Authentication failed - disconnecting.\n");

            if (rc == SSH_AUTH_ERROR) rc = SSH_ERR_AUTH_ERROR;
            if (rc == SSH_AUTH_PARTIAL) rc = SSH_ERR_AUTH_ERROR;
            if (rc == SSH_AUTH_DENIED) rc = SSH_ERR_ACCESS_DENIED;

            return rc;
        }
    }

    debug(F100, "sshsubsys - Authentication succeeded", "", 0);

    /* Setup direct, reverse and agent forwarding if it's been requested */
    configure_forwarding(state, client);


    /* Setup X11 forwarding. What the libssh documentation fails to mention
     * is that this must be done *before* the session is started */
    setup_x11_forwarding(state, client);

    if (state->forwarding_ok && state->parameters->agent_forwarding) {
        rc = ssh_channel_request_auth_agent(state->ttyChannel);
    }

    debug(F100, "sshsubsys - Starting session...", "", 0);
    if (state->parameters->session_type == SESSION_TYPE_SUBSYSTEM) {
        /* User has supplied the /SUBSYSTEM: qualifier */
        rc = ssh_subsystem(state, state->parameters->command_or_subsystem);
    } else if (state->parameters->session_type == SESSION_TYPE_COMMAND){
        /* User has supplied the /COMMAND: qualifier. */
        rc = ssh_rexec(state, state->parameters->command_or_subsystem);
    } else {
        /* Open a shell */
        rc = open_shell(state);
    }

    return rc;
}

#define MAX_EVENTS 10

/** The SSH Client thread - where libssh lives because libssh is strictly one
 * SSH session per thread and C-Kermit is multi-threaded.
 *
 * Configures libssh, sets up any channels necessary, then sits in a loop
 * moving data around. The client is configured via the supplied
 * ssh_parameters_t struct and communication with the rest of C-Kermit and its
 * various threads is done via members of the ssh_client_t struct.
 *
 * @param parameters SSH client thread parameters
 */
unsigned int __stdcall ssh_thread(ssh_thread_params_t *parameters) {
    int rc = 0;

    ssh_client_state_t* state = NULL;
    ssh_client_t *client;
    socket_t socket;
    HANDLE events[MAX_EVENTS];
    LARGE_INTEGER keepaliveDueTime;

    debug(F100, "sshsubsys - SSH Subsystem starting up...", "", 0);

    for(int i = 0; i < MAX_EVENTS; i++) {
        events[i] = INVALID_HANDLE_VALUE;
    }

    client = parameters->ssh_client;

    state = ssh_client_state_new(parameters->parameters, client);
    if (state == NULL) {
        debug(F100, "sshsubsys - failed to create client state. Giving up.", NULL, 0);
        ssh_client_close(NULL, client, SSH_ERR_STATE_MALLOC_FAILED);
        return 0;
    }

    debug(F100, "sshsubsys - Cleaning up.", NULL, 0);
    free(parameters); /* Don't need it anymore */

    debug(F100, "sshsubsys - Set logging callback.", NULL, 0);
    ssh_set_log_callback(logging_callback);

    debug(F100, "sshsubsys - init ssh client.", NULL, 0);
    state->session = ssh_new();
    if (state->session == NULL) {
        debug(F100, "sshsubsys - Failed to create SSH session", "", 0);
        ssh_client_close(state, client, SSH_ERR_NEW_SESSION_FAILED);
        return 0;
    }

    rc = connect_ssh(state, client);

    if (rc != SSH_OK) {
        debug(F111, "sshsubsys - Session start failed - disconnecting", "rc", rc);
        ssh_client_close(state, client, rc);
        return 0;
    }

    debug(F100, "sshsubsys - SSH connected.", "", 0);

    SetEvent(client->clientStarted);

    socket = ssh_get_fd(state->session);

    events[0] = WSACreateEvent();
    WSAEventSelect(socket, events[0], FD_READ | FD_WRITE);
    // TODO: The above puts the socket in nonblocking mode. Will libssh mind?
    events[1] = CreateWaitableTimerW(NULL, TRUE, NULL); /* Keepalive timer */
    events[2] = client->disconnectEvent;
    events[3] = client->ptySizeChangedEvent;
    events[4] = client->flushEvent;
    events[5] = client->breakEvent;
    events[6] = client->dataArrivedEvent;
    events[7] = client->dataConsumedEvent;
    events[8] = client->forwardingConfigChanged;
    events[9] = state->forwardingReadyRead;

    /* Setup keepalive timer (if enabled) */
    if (state->parameters->keepalive_seconds > 0) {
        /* Value is in 100 nanosecond intervals, negative for relative time */
        keepaliveDueTime.QuadPart =
                state->parameters->keepalive_seconds * (-10000000LL);

        if (!SetWaitableTimer(events[1], &keepaliveDueTime, 0, NULL, NULL, 0))
        {
            debug(F111, "sshsubsys - failed to set keepalive timer",
                  "interval", state->parameters->keepalive_seconds);
        }
    }

    rc = SSH_ERR_WAIT_FAILED;

    debug(F100, "sshsubsys - entering client loop", "", 0);

    /* Now wait until we've got something to do */
    while(TRUE) {
        WSANETWORKEVENTS netEvents;
        DWORD waitResult;

        debug(F100, "sshsubsys - waiting...", "", 0);
        waitResult = WSAWaitForMultipleEvents(
                MAX_EVENTS, /* Number of events */
                events, /* Array of events to wait on */
                FALSE, /* Return when *any* event is signalled, rather than all */
                1000, /* Wait for up to 1s */
                TRUE /* Place thread in alertable wait state to allow execution of
                      * I/O completion routines. */
        );

        if (waitResult == WSA_WAIT_FAILED) {
            int error = WSAGetLastError();
            debug(F111, "sshsubsys - Wait failed!", "error", error);
            switch(error) {
                case WSANOTINITIALISED:
                    debug(F100, "sshsubsys - WSA Not Initialised", "", 0);
                    break;
                case WSAENETDOWN:
                    debug(F100, "sshsubsys - WSA - network subsystem failure", "", 0);
                    break;
                case WSAEINPROGRESS:
                    debug(F100, "sshsubsys - WSA - A blocking winsock 1.1 call is in progress", "", 0);
                    break;
                case WSA_NOT_ENOUGH_MEMORY:
                    debug(F100, "sshsubsys - WSA - not enough memory", "", 0);
                    break;
                case WSA_INVALID_HANDLE:
                    debug(F100, "sshsubsys - WSA - invalid handle", "", 0);
                    break;
                case WSA_INVALID_PARAMETER:
                    debug(F100, "sshsubsys - WSA - invalid parameter", "", 0);
                    break;
            }

            rc = SSH_ERR_WAIT_FAILED;
            break;
        } else if (waitResult == WSA_WAIT_IO_COMPLETION) {
            continue;
        }

        rc = SSH_ERR_OK;

        /* Check for errors */
        if (ssh_channel_is_closed(state->ttyChannel)) {
            debug(F100, "sshsubsys - tty channel is closed - ending session", "", 0);
            rc = SSH_ERR_CHANNEL_CLOSED;
            break;
        }

        /* Check for EOF */
        if (ssh_channel_is_eof(state->ttyChannel)) {
            debug(F100, "sshsubsys - tty channel is EOF - ending session", "", 0);
            rc = SSH_ERR_EOF;
            break;
        }

        /* See if there is any network data available.    */
        if (WaitForSingleObjectEx(events[0], 0, TRUE) != WAIT_TIMEOUT) {
            WSAResetEvent(events[0]);
            debug(F100, "sshsubsys - network event", "", 0);
        }

        /* Check for disconnect event */
        if (WaitForSingleObjectEx(client->disconnectEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - disconnect event", "", 0);
            break; /* Break out of loop to disconnect. No need to reset the event. */
        }

        /* Check for PTY Resize Event */
        if (WaitForSingleObjectEx(client->ptySizeChangedEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - pty size changed event", "", 0);
            if (acquire_mutex(client->mutex, INFINITE)) {
                int pty_width = client->pty_width;
                int pty_height = client->pty_height;
                ReleaseMutex(client->mutex);
                ResetEvent(client->ptySizeChangedEvent);

                if (pty_width != state->pty_width ||
                    pty_height != state->pty_height) {
                    int result = ssh_channel_change_pty_size(state->ttyChannel,
                                                             client->pty_width,
                                                             client->pty_height);
                    if (result == SSH_OK) {
                        state->pty_width = client->pty_width;
                        state->pty_height = client->pty_height;
                    } else {
                        debug(F111, "sshsubsys - failed to change pty size",
                              ssh_get_error(state->session), rc);
                        /* Not really an error serious enough to warrant
                         * killing the connection - if everything else is still
                         * working the PTY will just be the wrong size. If there
                         * really is a problem something else will fail soon
                         * enough and cause a disconnect. */
                    }
                }
            }
        }

        /* Check for break event */
        if (WaitForSingleObjectEx(client->breakEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - break event", "", 0);
            /* TODO: Is this even correct? The old ssh subsystem based on
             *       OpenSSH didn't seem to support break at all */
            ssh_channel_request_send_break(
                    state->ttyChannel,
                    5);
            ResetEvent(client->breakEvent);
        }

        /* Check for flush event */
        if (WaitForSingleObjectEx(client->flushEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - flush event", "", 0);
            // TODO: Should we sit in a loop until everything has been written
            //       to the network?
            rc = ssh_tty_write(state, client);
            ResetEvent(client->flushEvent);
            if (rc != SSH_ERR_OK) {
                debug(F111, "flush event returned an error state", "rc", rc);
                break;
            }
        }

        /* Check for data ready to be sent */
        if (WaitForSingleObjectEx(client->dataArrivedEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - data arrived event", "", 0);
            ResetEvent(client->dataArrivedEvent);
        }

        /* Data has been consumed from a full output buffer. If we stopped
         * writing data to it because it was full, now is our chance to resume
         * writing.*/
        if (WaitForSingleObjectEx(client->dataConsumedEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - data consumed event", "", 0);
            ResetEvent(client->dataConsumedEvent);
        }

        /* Check if it's time to send a keepalive packet */
        if (WaitForSingleObjectEx(events[1], 0, TRUE) != WAIT_TIMEOUT) {
            /* Then send some data the server should just ignore. This should
             * keep the connection alive and prevent timeouts */
            ssh_send_ignore(state->session, "\0");

            debug(F100, "sshsubsys - sent keepalive packet", NULL, 0);

            /* Reset the timer for the next keepalive packet */
            if (!SetWaitableTimer(events[1], &keepaliveDueTime, 0, NULL, NULL, 0))
            {
                debug(F111, "sshsubsys - failed to set keepalive timer after "
                            "timer was signaled", "interval",
                            state->parameters->keepalive_seconds);
            }
        }

        /* We process the send and receive buffers every time around even if
         * there haven't been any related events as it significantly reduces
         * or eliminates a certain race condition that was breaking file
         * transfers. Its perhaps not the correct solution but it works and
         * doesn't really have much of a downside. */
        rc = ssh_tty_write(state, client);
        if (rc != SSH_ERR_OK) {
            debug(F111, "ssh_tty_write returned an error state", "rc", rc);
            break;
        }

        rc = ssh_tty_read(state, client);
        if (rc != SSH_ERR_OK) {
            debug(F111, "ssh_tty_read returned an error state", "rc", rc);
            break;
        }

        /* Handle any port or X11 forwarding. If we can't get a lock on the
         * mutex right now then we'll deal with this next time around the loop.
         */
        if (ssh_forwarding_lock(client, 0)) {
            ssh_forward_t *fwdNode = client->forwards;
            int xx = 0;

            if (fwdNode != NULL) {
                /* Accept any pending reverse forwarding connections */
                accept_reverse_forwarding_connections(state, client);
            }

            while (fwdNode != NULL) {
                debug(F101, "sshsubsys - process fwd", NULL, xx);

                if (fwdNode->state == SSH_FWD_STATE_PENDING
                    && fwdNode->type == SSH_FWD_TYPE_DIRECT) {
                    int rc;

                    /* This forward is not setup yet - it was probably added
                     * after the session was started */
                    debug(F101, "sshsubsys - fwd state pending, starting server", NULL, xx);

                    rc = start_forward_server(fwdNode, state);

                    if (rc != SSH_ERR_NO_ERROR) {
                        /* Failed to start listening */
                        debug(F100, "sshsubsys - forwarding entry state set "
                                    "to error", NULL, 0);
                    } else {
                        debug(F101, "sshsubsys - forwarding server started for id", NULL, xx);
                    }
                }

                if (fwdNode->state == SSH_FWD_STATE_OPEN
                    && fwdNode->listen_socket == INVALID_SOCKET) {
                    debug(F101, "sshsubsys - server socket has gone invalid for fwd, marking as closed", NULL, xx);

                    /* Stopped listening */
                    fwdNode->state = SSH_FWD_STATE_CLOSED;

                    /* TODO: Option/command to stop listening *and* force close
                     *       all connections and channels? */
                }

                if (fwdNode->state == SSH_FWD_STATE_OPEN) {
                    ssh_forward_connection_t *prev = NULL;
                    ssh_forward_connection_t *conn = fwdNode->connections;
                    int xy = 0;

                    debug(F101, "sshsubsys - fwd state open", NULL, xx);

                    /* Accept any pending connections */
                    if (fwdNode->type == SSH_FWD_TYPE_DIRECT) {
                        debug(F101, "sshsubsys - fwd state open, accept connections", NULL, xx);
                        accept_direct_forwarding_connections(state, client, fwdNode);
                    } else if (fwdNode->type == SSH_FWD_TYPE_X11) {
                        debug(F101, "sshsubsys - X11 forwarding open, accept connections", NULL, xx);
                        accept_x11_forwarding_connections(state, client, fwdNode);
                    }

                    /* Then handle communications for each active connection */
                    while (conn != NULL) {
                        debug(F101, "sshsubsys - fwd state open, connection", NULL, xy);

                        /* Check if the thread for this connection is still
                         * live */
                        if (WaitForSingleObject(conn->thread, 0) !=
                                WAIT_OBJECT_0) {

                            /* Check if the channel is actually still open */
                            if (ssh_channel_is_eof(conn->channel)) {
                                debug(F111, "sshsubsys - channel for connection is EOF", "connection", xy);
                                /* Signal a disconnect */
                                WSASetEvent(conn->disconnectEvent);
                                continue;
                            }

                            if (ssh_channel_is_closed(conn->channel)) {
                                debug(F111, "sshsubsys - channel for connection is closed", "connection", xy);
                                /* Signal a disconnect */
                                WSASetEvent(conn->disconnectEvent);
                                continue;
                            }

                            /* connection is still live - do communications */
                            debug(F101, "sshsubsys - fwd state open, do communications for connection", NULL, xy);
                            ssh_port_fwd_read(conn);
                            ssh_port_fwd_write(conn);

                            prev = conn;
                            conn = conn->next;
                        } else {
                            /* Thread isn't running. Connection is (or should
                             * be) closed. Clean up. */
                            debug(F101, "sshsubsys - fwd state open, thread for connection gone, cleanup", NULL, xy);
                            ssh_forward_connection_t *temp = conn;
                            conn = conn->next;

                            /* Remove the connection struct from the list */
                            if (prev == NULL) {
                                /* This was the first connection in the list */
                                fwdNode->connections = temp->next;
                            } else {
                                prev->next = temp->next;
                            }

                            /* Clean up all resources associated with the
                             * connection. Set force=TRUE as we know the thread
                             * is already gone */
                            free_fwd_connection(temp);
                        }
                        xy++;
                    }
                }

                fwdNode = fwdNode->next;
                xx++;
            }

            ResetEvent(client->forwardingConfigChanged);
            ResetEvent(state->forwardingReadyRead);
        }
    }

    /* Free up all forwarded connections */
    if (ssh_forwarding_lock(client, 5000)) {
        ssh_forward_t *fwdNode = client->forwards;
        debug(F100, "sshsubsys - cleaning up forwarded connections...", NULL, 0);

        while (fwdNode != NULL) {
            ssh_forward_t *fwdTemp;
            debug(F100, "sshsubsys - clean up fwd", NULL, 0);

            if (fwdNode->state == SSH_FWD_STATE_OPEN) {
                debug(F100, "sshsubsys - fwd supposedly open", NULL, 0);
                ssh_forward_connection_t *conn = fwdNode->connections;
                fwdNode->connections = NULL;

                /* Stop listening for new connections */
                if (fwdNode->type == SSH_FWD_TYPE_DIRECT) {
                    debug(F100, "sshsubsys - fwd supposedly open - closing listen socket", NULL, 0);
                    closesocket(fwdNode->listen_socket);
                    fwdNode->state = SSH_FWD_STATE_CLOSED;
                    fwdNode->listen_socket = INVALID_SOCKET;
                } else if (fwdNode->type == SSH_FWD_TYPE_REVERSE) {
                    /* Tell the SSH server to stop forwarding */
                    ssh_channel_cancel_forward(
                            state->session,
                            fwdNode->remoteHost,
                            fwdNode->remotePort
                            );
                    /* TODO: Do we need to check for SSH_AGAIN and retry? */
                } else if (fwdNode->type == SSH_FWD_TYPE_X11) {
                    /* There doesn't appear to be a way to stop X11 forwarding
                     * short of perhaps closing the associated channel (the tty
                     * channel)
                     */
                }

                /* Then close all existing connections */
                while (conn != NULL) {
                    debug(F100, "sshsubsys - fwd supposedly open - close connection", NULL, 0);
                    ssh_forward_connection_t *temp = conn;
                    conn = conn->next;

                    free_fwd_connection(temp);
                }
            }

            fwdTemp = fwdNode;
            fwdNode = fwdNode->next;

            if (fwdTemp->remoteHost != NULL) {
                free(fwdTemp->remoteHost);
            }

            if (fwdTemp->localHost != NULL) {
                free(fwdTemp->localHost);
            }

            free(fwdTemp);
        }
        client->forwards = NULL;
        debug(F100, "sshsubsys - cleaned up forwarded connections.", NULL, 0);
    }

    /* We've either been asked to disconnect or hit an error. Clean up and end
     * the thread. */
    WSACloseEvent(events[0]); /* Close the socket event created earlier */
    CloseHandle(events[1]); /* Close the keepalive timer */
    ssh_client_close(state, client, rc);
    debug(F100, "sshsubsys - thread terminate", "", 0);
    return 0;
}


/** Starts the SSH subsystem
 *
 * @param parameters SSH Parameters
 * @param ssh_client SSH Client instance
 * @param threadHandle Handle for the SSH thread
 * @return
 */
int start_ssh_subsystem(ssh_parameters_t* parameters, ssh_client_t *ssh_client,
                        HANDLE *threadHandle) {
    ssh_thread_params_t *thread_params;
    HANDLE events[2];

    debug(F100, "start_ssh_subsystem() - Launch SSH Subsystem", "", 0);

    thread_params = malloc(sizeof(ssh_thread_params_t));

    thread_params->ssh_client = ssh_client;
    thread_params->parameters = parameters;

    debug(F100, "start_ssh_subsystem() - begin thread", "", 0);
    *threadHandle = (HANDLE)_beginthreadex(
            NULL,           /* Security info */
            65536,          /* Stack size */
            ssh_thread,     /* Start address */
            (void *)thread_params,   /* Arg list */
            0,              /* init flags - start immediately */
            NULL            /* Thread identifier */
    );

    events[0] = ssh_client->clientStarted;
    events[1] = ssh_client->clientStopped;

    debug(F100, "start_ssh_subsystem() - waiting for subsystem start event...", "", 0);
    if (WaitForMultipleObjects(2, events, FALSE, 60000) == WAIT_TIMEOUT) {
        debug(F100, "Timeout waiting for SSH client to start or fail", "", 0);

        /* TODO: Client thread is stuck. Do we try to kill it? */

        return SSH_ERR_THREAD_STATE_UNKNOWN;
    }

    debug(F100, "start_ssh_subsystem() - subsystem start completed. Checking result.", "", 0);

    /* Client thread has signalled a status - check if that status was stopped
     * as that likely a failure of some kind */
    if (WaitForSingleObjectEx(ssh_client->clientStopped, 0, TRUE) != WAIT_TIMEOUT) {
        debug(F111, "start_ssh_subsystem() - SSH Subsystem started then "
                    "stopped.", "error", ssh_client->error);
        /* Client failed to start */
        return ssh_client->error;
    }

    debug(F100, "start_ssh_subsystem() - subsystem started successfully!", "", 0);
    return SSH_ERR_OK;
}


/** Acquires the specified mutex if it can be done so within the given
 * timeframe
 *
 * @param mutex Mutex to acquire
 * @param msTimeout Maximum time to wait for the mutex to become available
 * @return TRUE if the mutex was acquired, FALSE otherwise.
 */
BOOL acquire_mutex(HANDLE mutex, DWORD msTimeout) {
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
            mutex,
            msTimeout);
    switch(dwWaitResult) {
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        case WAIT_ABANDONED:
            debug(F100, "mutex acquired in abandoned state. Protected data state may be inconsistent", "", 0);
            return TRUE;
        case WAIT_FAILED:
            debug(F101, "wait on mutex failed with error", "", GetLastError());
            return FALSE;
    }
    return FALSE; /* Should never happen */
}
