/* C-Kermit for Windows SSH Subsystem
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

#include "ckcdeb.h"
#include "ckcker.h"

/* Only so we can get a definition for VTERM and VNUM */
#include "ckocon.h"

#include "ckoshs.h"


/* SSH Strict Host Key Checking options */
#define SSH_SHK_NO 0                    /* Trust everything implicitly */
#define SSH_SHK_YES 1                   /* Don't trust anything unexpected */
#define SSH_SHK_ASK 2                   /* Ask the user */


/** This holds all the global state for the SSH Thread.
 */
typedef struct {
    ssh_parameters_t *parameters;    /* Connection parameters */
    ssh_session session;             /* Current SSH session (if any) */
    ssh_channel ttyChannel;          /* The tty channel - shell or command */
    int pty_height, pty_width;       /* Dimensions of the pty */
} ssh_client_state_t;


typedef struct {
    ssh_parameters_t* parameters;
    ssh_client_t *ssh_client;
} ssh_thread_params_t;

static void close_tty_channel(ssh_client_state_t * state);


void get_current_terminal_dimensions(int* rows, int* cols) {
    extern int tt_rows[];                   /* Screen rows */
    extern int tt_cols[];                   /* Screen columns */
    extern int tt_status[VNUM];             /* Terminal status line displayed */

    /* TODO: Elsewhere in K95 one is taken off the row count if the status line
     *       is on. But this seems to produce incorrect results - it looks like
     *       the tt_rows value already accounts for the status line. Need to
     *       check elsewhere and confirm this. Smells like a bug somewhere.
     **/
    *rows = tt_rows[VTERM]; /* - (tt_status[VTERM]?1:0); */
    *cols = tt_cols[VTERM];
}


ssh_parameters_t* ssh_parameters_new(
        char* hostname, char* port, int verbosity, char* command,
        BOOL subsystem, BOOL compression, BOOL use_openssh_config,
        BOOL gssapi_delegate_credentials, int host_key_checking_mode,
        char* user_known_hosts_file, char* global_known_hosts_file,
        char* username, char* password, char* terminal_type, int pty_width,
        int pty_height) {
    ssh_parameters_t* params;

    params = malloc(sizeof(ssh_parameters_t));

    params->hostname = NULL;
    params->port = NULL;
    params->command_or_subsystem = NULL;
    params->user_known_hosts_file = NULL;
    params->global_known_hosts_file = NULL;
    params->username = NULL;
    params->password = NULL;
    params->terminal_type = NULL;

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

    params->log_verbosity = verbosity;
    params->compression = compression;
    params->use_openssh_config = use_openssh_config;
    params->gssapi_delegate_credentials = gssapi_delegate_credentials;
    params->host_key_checking_mode = host_key_checking_mode;
    params->pty_width = pty_width;
    params->pty_height = pty_height;

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
    if (parameters->password)
        free(parameters->password);
    if (parameters->terminal_type)
        free(parameters->terminal_type);

    free(parameters);
}


ssh_client_t * ssh_client_new() {
    ssh_client_t *client = malloc(sizeof(ssh_client_t));
    client->outputBuffer = ring_buffer_new(1024);
    client->inputBuffer = ring_buffer_new(1024);
    client->error = SSH_ERR_NO_ERROR;
    client->error_message = NULL;
    client->mutex = CreateMutex(
            NULL,   /* default security attributes */
            FALSE,  /* initially not owned */
            NULL);  /* unnamed */
    client->disconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->ptySizeChangedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->flushEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    client->breakEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
    return client;
}


void ssh_client_free(ssh_client_t *client) {
    if (client == NULL) return;
    ring_buffer_free(client->outputBuffer);
    ring_buffer_free(client->inputBuffer);
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
    free(client);
}


/** Allocates a new ssh_client_state_t which holds all the global state for the
 * SSH Thread
 *
 * @param parameters SSH Client parameters
 */
static ssh_client_state_t* ssh_client_state_new(ssh_parameters_t* parameters) {
    ssh_client_state_t * state;

    state = malloc(sizeof(ssh_client_state_t));
    state->parameters = parameters;
    state->pty_width = 80;
    state->pty_height = 25;
    state->session = NULL;
    state->ttyChannel = NULL;

    get_current_terminal_dimensions(&state->pty_height, &state->pty_width);

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
    if (exit_status == SSH_ERR_SSH_ERROR) {
        error_message = _strdup(ssh_get_error(state->session));
        debug(F100, "sshsubsys - have libssh error message", error_message, 0);
    }

    if (state != NULL)
        ssh_client_state_free(state);

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


/** SSH Authentication callback for password and publickey auth
 *
 * @param prompt Prompt to be displayed
 * @param buf Buffer to save the password. Should be null terminated.
 * @param len Length of the buffer.
 * @param echo Enable or disable the echo of what you type
 * @param verify Should the password be verified?
 * @param userdata Userdata to be passed to the callback function
 */
static int auth_prompt(const char* prompt, char* buf, size_t len, int echo,
                       int verify, void* userdata) {

    debug(F110, "ssh auth_prompt", prompt, 0);
    debug(F111, "ssh auth_prompt", "echo", echo);
    debug(F111, "ssh auth_prompt", "verify", verify);
    debug(F111, "ssh auth_prompt", "len", len);

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

        rc = uq_mtxt(prompt, NULL, 2, tb);
        if (rc == 0) {
            debug(F100, "auth_prompt - user canceled", "", 0);
            rc = -1; /* failed */
        } else {
            if (strncmp(buf, verifyBuf, len) == 0) {
                rc = 0; /* Success */
            } else {
                debug(F100, "auth_prompt - verify failed", "", 0);
                rc = -1; /* error */
            }
        }

        free(verifyBuf);
        return rc;
    } else {
        int rc;
        rc = uq_txt(NULL, prompt, echo ? 1 : 2, NULL, buf, len, NULL,
                    DEFAULT_UQ_TIMEOUT);
        if (rc == 1) return 0; /* 1 == success */
        debug(F100, "auth_prompt - user canceled", "", 0);
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
 * TODO: Overhaul this function.
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
    char *hexa;
    char msg[1024];

    rc = ssh_get_server_publickey(state->session, &server_pubkey);
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

    host_state = ssh_session_is_known_server(state->session);
    /* TODO: Redo all of this properly. */
    switch (host_state) {
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
            if (state->parameters->host_key_checking_mode == SSH_SHK_YES) {
                snprintf(msg, sizeof(msg),
                         "No host key is known for %s and\n"
                         "you have requested strict host checking.\n"
                         "If you wish to make an untrusted connection,\n"
                         "SET SSH STRICT-HOST-KEY-CHECK OFF and try again.",
                         state->parameters->hostname);
                printf(msg);

                uq_ok(NULL, msg, 1, NULL, 0);

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

    debug(F110, "Attempting password authentication for user", user, 0);

    rc = ssh_options_get(state->session, SSH_OPTIONS_USER, &user);
    if (rc != SSH_OK) {
        debug(F100, "SSH - Failed to get user ID", "rc", rc);
        return rc;
    }

    for (i = 0; i < SSH_MAX_PASSWORD_PROMPTS; i++) {
        if (i == 0 && state->parameters->password) {
            /* Password has already been supplied. Try that */
            debug(F100, "Using pre-entered password", "", 0);
            ckstrncpy(password,state->parameters->password,sizeof(password));
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
            debug(F100, "User canceled password login", "", 0);
            ssh_string_free_char(user);
            *canceled = TRUE;
            return SSH_AUTH_DENIED;
        }

        rc = ssh_userauth_password(state->session, NULL, password);
        memset(password, 0, strlen(password));
        if (rc == SSH_AUTH_SUCCESS) {
            debug(F100, "Password login succeeded", "", 0);
            ssh_string_free_char(user);
            return rc;
        } else if (rc == SSH_AUTH_ERROR) {
            debug(F111, "SSH Auth Error - password login failed", "rc", rc);
            /* A serious error has occurred.  */
            ssh_string_free_char(user);
            return rc;
        } else if (rc == SSH_AUTH_PARTIAL) {
            debug(F100, "SSH Partial authentication - "
                        "more authentication needed", "", 0);
            ssh_string_free_char(user);
            return rc;
        }
        /* Else: SSH_AUTH_DENIED - try again. */
        debug(F100, "SSH Password auth failed - access denied", "", 0);
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

    debug(F100, "kbd_interactive_authenticate", "", 0);

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

        debug(F110, "kbd_int_auth name", name, 0);
        debug(F110, "kbd_int_auth instructions", instructions, 0);
        debug(F101, "kbd_int_auth prompts", "", nprompts);

        if (nprompts == 0) {
            debug(F100, "No more prompts! Unable to continue interrogating "
                        "user.", "nprompts", nprompts);
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
                debug(F100, "kbd_interactive_authenticate - failed to malloc "
                            "for instructions", "", 0);
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
                    state->session, i, &echo);

            debug(F110, "kdbint auth - single prompt mode - prompt:", prompt, 0);

            rc = uq_txt(combined_instructions, prompt, echo ? 1 : 2, NULL,
                        buffer, sizeof(buffer), NULL, DEFAULT_UQ_TIMEOUT);
            if (rc == 1) {
                rc = ssh_userauth_kbdint_setanswer(
                        state->session, 0, buffer);
                debug(F111, "ssh_userauth_kbdint says", "rc", rc);
                if (rc < 0) {
                    /* An error of some kind occurred. Don't bother feeding
                     * in any further responses. We'll only keep going around
                     * the loop to clean up the response array */
                    failed = TRUE;
                    debug(F101, "prompt rejected", "", i);
                }
            } else {
                debug(F100, "kdbint auth - user canceled", "", 0);
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
                debug(F100, "kbd_interactive_authenticate - textbox malloc failed", "", 0);
                return SSH_AUTH_ERROR;
            }
            memset(tb, 0, sizeof(struct txtbox) * nprompts);

            /* Allocate an array to hold all the responses. */
            responses = malloc(sizeof(char *) * nprompts);
            if (responses == NULL) {
                debug(F100, "kbd_interactive_authenticate - response array malloc failed", "", 0);
                return SSH_AUTH_ERROR;
            }

            /* Build up an array of text boxes to show the user */
            for (i = 0; i < nprompts; i++) {
                char echo;
                const char *prompt = ssh_userauth_kbdint_getprompt(
                        state->session, i, &echo);
                responses[i] = malloc(128 * sizeof(char));

                debug(F111, "kdb_interactive_authenticate prompt", prompt, i);

                if (responses[i] == NULL) {
                    debug(F111, "kbd_interactive_authenticate - response buffer "
                                "malloc failed", "response", i);
                    return SSH_AUTH_ERROR;
                }

                memset(responses[i], 0, sizeof(responses[i]));

                tb[i].t_buf = responses[i];
                tb[i].t_len = 128;
                tb[i].t_lbl = prompt;
                tb[i].t_dflt = NULL;
                tb[i].t_echo = echo ? 1 /* yes */ : 2; /* no - asterisks */
            }

            /* Ask the user all the prompts in one go  */
            rc = uq_mtxt(combined_instructions, NULL, nprompts, tb);

            if (rc == 0) { /* 0 = no/cancel, 1 = yes/ok */
                debug(F100, "kdbint auth - user canceled", "", 0);
                failed = TRUE;
                *canceled = TRUE;
            }

            /* Then process the responses freeing buffers as we go */
            for (i = 0; i < nprompts; i++) {
                debug(F101, "processing prompt", "", i);
                if (!failed) {
                    /* User hasn't canceled and haven't hit an error yet. Ask the
                     * server what it thinks of an answer.*/
                    rc = ssh_userauth_kbdint_setanswer(
                            state->session, i, responses[i]);
                    debug(F111, "ssh_userauth_kbdint says", "rc", rc);
                    if (rc < 0) {
                        /* An error of some kind occurred. Don't bother feeding
                         * in any further responses. We'll only keep going around
                         * the loop to clean up the response array */
                        failed = TRUE;
                        debug(F101, "prompt rejected", "", i);
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
        if (rc == SSH_AUTH_INFO) debug(F101, "ssh_userauth_kbdint says SSH_AUTH_INFO", "", rc);
    }

    debug(F111, "ssh kbdint finished with", "rc", rc);

    return rc;
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
    int methods, rc;

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

    /* Get a list of available auth methods. We'll try them one after another
     * until the server is satisfied */
    methods = ssh_userauth_list(state->session, NULL);

    if (methods & SSH_AUTH_METHOD_NONE && !*canceled) {
        rc = ssh_userauth_none(state->session, NULL);
        if (rc == SSH_AUTH_SUCCESS) return rc;
    }
    if (methods & SSH_AUTH_METHOD_PUBLICKEY && !*canceled) {
        rc = ssh_userauth_publickey_auto(state->session, NULL, NULL);
        if (rc == SSH_AUTH_SUCCESS) return rc;
    }
    /* TODO: Not working quite right at the moment.
     *    After answering all prompts ssh_userauth_kbdint still gives
     *    SSH_AUTH_INFO indicating more answers are required - even though there
     *    are no more prompts to answer.
     *
     *    Probably need to test a simpler example just in case its something
     *    like threading causing problems.
     *
    if (methods & SSH_AUTH_METHOD_INTERACTIVE && !*canceled) {
        rc = kbd_interactive_authenticate(state, canceled);
        if (rc == SSH_AUTH_SUCCESS) return rc;
    } */
    if (methods & SSH_AUTH_METHOD_PASSWORD && !*canceled) {
        rc = password_authenticate(state, canceled);
        if (rc == SSH_AUTH_SUCCESS) return rc;
    }

    if (canceled) {
        return SSH_ERR_USER_CANCELED;
    }

    /* TODO: ssh_userauth_gssapi() */

    return rc;
}


/** Opens the TTY channel and nothing else.
 *
 * @param state SSH Client State
 * @return SSH_OK on success, else an error
 */
static int open_tty_channel(ssh_client_state_t * state) {
    int rc = 0;

    debug(F100, "Opening SSH tty channel", "", 0);

    state->ttyChannel = ssh_channel_new(state->session);
    if (state->ttyChannel == NULL) {
        debug(F100, "Failed to create channel", "", 0);
        return SSH_ERR_SSH_ERROR;
    }

    rc = ssh_channel_open_session(state->ttyChannel);
    if (rc != SSH_OK) {
        debug(F111, "Channel open failed", "rc", rc);
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

    debug(F110,"ssh running command", command, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_exec(state->ttyChannel, command);
    if (rc != SSH_OK) {
        debug(F111, "SSH exec failed", "rc", rc);
        close_tty_channel(state);
        return rc;
    }

    debug(F100, "SSH exec succeeded", "", 0);

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

    /* TODO: Not working? */

    debug(F110,"ssh requesting subsystem", subsystem, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        return rc;
    }

    rc = ssh_channel_request_subsystem(state->ttyChannel, subsystem);
    if (rc != SSH_OK) {
        debug(F111, "SSH subsystem request failed", "rc", rc);
        close_tty_channel(state);
        return rc;
    }

    debug(F100, "SSH subsystem request succeeded", "", 0);

    return rc;
}


/** Opens the tty channel for a shell. This also sets up a PTY.
 *
 * @param state SSH Client State
 * @return SSH_OK on success, or an error.
 */
static int open_shell(ssh_client_state_t * state) {
    int rc;

    debug(F100, "SSH open shell", "", 0);

    debug(F111, "SSH pty request", "rows", state->pty_height);
    debug(F111, "SSH pty request", "cols", state->pty_width);
    debug(F111, "SSH pty request - termtype: ",
          state->parameters->terminal_type, 0);

    rc = open_tty_channel(state);
    if (rc != SSH_OK) {
        debug(F111, "open tty channel failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_pty_size(state->ttyChannel,
                                      state->parameters->terminal_type,
                                      state->pty_width,
                                      state->pty_height);
    if (rc != SSH_OK) {
        debug(F111, "PTY request failed", "rc", rc);
        return rc;
    }

    rc = ssh_channel_request_shell(state->ttyChannel);
    if (rc != SSH_OK) {
        debug(F111, "Shell request failed", "rc", rc);
        return rc;
    }

    return rc;
}


/** Applies all configuration to the SSH Session
 *
 * @param state SSH Client state
 * @returns An error code if anything goes wrong
 */
static int configure_session(ssh_client_state_t * state) {
    int rc = SSH_ERR_OK;
    int verbosity = SSH_LOG_PROTOCOL;

    static struct ssh_callbacks_struct cb = {
            .auth_function = auth_prompt
    };

    ssh_callbacks_init(&cb);

    /* Set options */
    debug(F100, "Configure session...", "", 0);
    verbosity = log_verbosity(state->parameters->log_verbosity);
    ssh_options_set(state->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_set_callbacks(state->session, &cb);
    ssh_options_set(state->session, SSH_OPTIONS_HOST, state->parameters->hostname);
    ssh_options_set(state->session, SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS,
                    &state->parameters->gssapi_delegate_credentials);
    ssh_options_set(state->session, SSH_OPTIONS_PROCESS_CONFIG,
                    &state->parameters->use_openssh_config);
    if (!state->parameters->compression) {
        ssh_options_set(state->session, SSH_OPTIONS_COMPRESSION_C_S, "no");
        ssh_options_set(state->session, SSH_OPTIONS_COMPRESSION_S_C, "no");
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
    // TODO: Set SSH_OPTIONS_SSH_DIR to where the known_hosts and keys live
    // TODO: SSH_OPTIONS_STRICTHOSTKEYCHECK ?

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
    int rc;

    /* Read from the TTY channel into the output buffer */
    if (ring_buffer_lock(client->outputBuffer, INFINITE)) {
        char* buf;
        int space_available;

        space_available = ring_buffer_free_space(client->outputBuffer);

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
                              "ERROR: fewer bytes written to output buffer "
                              "than available free space", "free",
                              space_available);

                        /* TODO: It would be nice to retry but really this should
                         *       never happen (it would be a pretty big problem if
                         *       it did!)*/
                        rc = SSH_ERR_BUFFER_WRITE_FAILED;
                    }
                }

                free(buf);
            }
        }

        ring_buffer_unlock(client->outputBuffer);
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
    int rc;

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
                        debug(F111, "SSH - Failed to consume bytes from "
                                    "the input ring buffer", "bytes",
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

        ring_buffer_unlock(client->inputBuffer);
    }

    if (rc >= 0) {
        rc = SSH_ERR_OK;
    }
    return rc;
}


/** Configures libssh and connects to the server
 *
 * @param state SSH Client State
 * @return An error code on failure
 */
static int connect_ssh(ssh_client_state_t* state) {
    int rc;
    BOOL user_canceled;
    char* banner = NULL;

    /* Connect! */
    debug(F100, "SSH Connect...", "", 0);

    /* Apply configuration to the SSH session */
    rc = configure_session(state);
    if (rc != SSH_ERR_OK) {
        return rc;
    }

    rc = ssh_connect(state->session);
    if (rc != SSH_OK) {
        debug(F111,"Error connecting to host",
              ssh_get_error(state->session), rc);
        return rc;
    }

    /* Check the hosts key is valid */
    rc = verify_known_host(state);
    if (rc != SSH_ERR_NO_ERROR) {
        debug(F111, "Host verification failed", "rc", rc);
        printf("Host verification failed.\n");
        return rc;
    }

    /* This is apparently required for some reason in order for
     * get_issue_banner to work */
    rc = ssh_userauth_none(state->session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        return rc;
    }

    banner = ssh_get_issue_banner(state->session);
    if (banner) {
        printf(banner);
        ssh_string_free_char(banner);
        banner = NULL;
    }

    /* Authenticate! */
    rc = authenticate(state, &user_canceled);
    if (rc != SSH_AUTH_SUCCESS ) {
        debug(F111, "Authentication failed - disconnecting", "rc", rc);
        printf("Authentication failed - disconnecting.\n");

        if (rc == SSH_AUTH_ERROR) rc = SSH_ERR_AUTH_ERROR;
        if (rc == SSH_AUTH_PARTIAL) rc = SSH_ERR_AUTH_ERROR;
        if (rc == SSH_AUTH_DENIED) rc = SSH_ERR_ACCESS_DENIED;

        return rc;
    }

    debug(F100, "Authenticated - starting session", "", 0);

    debug(F100, "Authentication succeeded - starting session", "", 0);

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
void ssh_thread(ssh_thread_params_t *parameters) {
    int rc = 0;

    ssh_client_state_t* state = NULL;
    ssh_client_t *client;
    socket_t socket;
    HANDLE events[7];


    debug(F100, "ssh thread started", "", 0);

    client = parameters->ssh_client;

    state = malloc(sizeof(ssh_client_state_t));
    if (state == NULL) {
        ssh_client_close(NULL, client, SSH_ERR_STATE_MALLOC_FAILED);
        return;
    }

    state->parameters = parameters->parameters;
    state->pty_height = state->parameters->pty_height;
    state->pty_width = state->parameters->pty_width;

    free(parameters); /* Don't need it anymore */

    ssh_set_log_callback(logging_callback);

    state->session = ssh_new();
    if (state->session == NULL) {
        debug(F100, "Failed to create SSH session", "", 0);
        ssh_client_close(state, client, SSH_ERR_NEW_SESSION_FAILED);
        return;
    }

    rc = connect_ssh(state);

    if (rc != SSH_OK) {
        debug(F111, "Session start failed - disconnecting", "rc", rc);
        ssh_client_close(state, client, rc);
        return;
    }

    /* TODO: Setup port forwarding, etc */

    printf("Connection open!");
    debug(F100, "SSH connected.", "", 0);

    SetEvent(client->clientStarted);

    socket = ssh_get_fd(state->session);
    events[0] = WSACreateEvent();
    WSAEventSelect(socket, events[0], FD_READ | FD_WRITE);
    // TODO: The above puts the socket in nonblocking mode. Will libssh mind?
    events[1] = client->disconnectEvent;
    events[2] = client->ptySizeChangedEvent;
    events[3] = client->flushEvent;
    events[4] = client->breakEvent;
    events[5] = client->dataArrivedEvent;
    events[6] = client->dataConsumedEvent;

    rc = SSH_ERR_WAIT_FAILED;

    debug(F100, "sshsubsys - starting client loop", "", 0);

    /* Now wait until we've got something to do */
    while(TRUE) {
        WSANETWORKEVENTS  netEvents;
        DWORD waitResult;

        debug(F100, "sshsubsys - waiting...", "", 0);
        waitResult = WSAWaitForMultipleEvents(
                7, /* Number of events */
                events, /* Array of events to wait on */
                FALSE, /* Return when *any* event is signalled, rather than all */
                WSA_INFINITE, /* Wait forever */
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

        if (ssh_channel_is_closed(state->ttyChannel)) {
            debug(F100, "tty channel is closed - ending session", "", 0);
            rc = SSH_ERR_CHANNEL_CLOSED;
            break;
        }

        if (ssh_channel_is_eof(state->ttyChannel)) {
            debug(F100, "tty channel is EOF - ending session", "", 0);
            rc = SSH_ERR_EOF;
            break;
        }

        if (WaitForSingleObjectEx(events[0], 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - network event", "", 0);
            WSAResetEvent(events[0]);
            rc = ssh_tty_read(state, client);
            if (rc != SSH_ERR_OK) {
                break; /* Read error - fail */
            }

            rc = ssh_tty_write(state, client);
            if (rc != SSH_ERR_OK) {
                break; /* Write error - fail */
            }
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
                        debug(F111, "ssh thread - failed to change pty size",
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

        if (WaitForSingleObjectEx(client->flushEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - flush event", "", 0);
            // TODO: Should we sit in a loop until everything has been written
            //       to the network?
            rc = ssh_tty_write(state, client);
            ResetEvent(client->flushEvent);
            if (rc != SSH_ERR_OK) {
                break;
            }
        }

        if (WaitForSingleObjectEx(client->dataArrivedEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - data arrived event", "", 0);
            rc = ssh_tty_write(state, client);
            ResetEvent(client->dataArrivedEvent);
            if (rc != SSH_ERR_OK) {
                break;
            }
        }

        /* Data has been consumed from a full output buffer. If we stopped
         * writing data to it because it was full, now is our chance to resume
         * writing.*/
        if (WaitForSingleObjectEx(client->dataConsumedEvent, 0, TRUE) != WAIT_TIMEOUT) {
            debug(F100, "sshsubsys - data consumed event", "", 0);
            rc = ssh_tty_read(state, client);
            ResetEvent(client->dataConsumedEvent);
            if (rc != SSH_ERR_OK) {
                break;
            }
        }
    }

    /* We've either been asked to disconnect or hit an error. Clean up and end
     * the thread. */
    WSACloseEvent(events[0]); /* Close the socket event created earlier */
    ssh_client_close(state, client, rc);
    debug(F100, "sshsubsys - thread terminate", "", 0);
    return;
}


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
