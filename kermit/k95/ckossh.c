#ifdef SSH_DLL
char *cksshv = "SSH-DLL support, 10.0,  28 July 2024";
#endif /* SSH_DLL */

/*
 * C K O S S H . C -- SSH DLL Interface for Kermit 95
 *
 * Copyright (C) 2024, David Goodwin <david@zx.net.nz>
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
 * This is the interface between Kermit 95 and a dynamically loaded SSH
 * backend.
 */

#include "ckcdeb.h"
#include "ckossh.h"
#include "ckuusr.h"
#include "ckcker.h"
#include "ckocon.h"
#include "ckoreg.h"
#include "ckctel.h"

#ifndef NT
#define INCL_NOPM
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_ERRORS
#define  INCL_DOSNMPIPES
#include <os2.h>
#undef COMMENT

/* This lives in p_common.c */
_PROTOTYP( unsigned long get_dir_len, (unsigned char *));
#endif /* Not NT */

/* Various global variables owned by the rest of C-Kermit */
extern char uidbuf[];                   /* User ID set via /user: */
extern char pwbuf[];                    /* Password set via /password: */
extern int  pwflg;                      /* Password has been set */

#ifdef TCP_NODELAY
extern int tcp_nodelay;                 /* Enable/disable Nagle's algorithm */
#endif /* TCP_NODELAY */

/** Gets the currently set username if there is one
 *
 * @return Currently set username
 */
const char* ssh_get_uid() {
    return uidbuf;
}

/** Gets the currently set password if there is one.
 *
 * @return Currently set password or NULL
 */
const char* ssh_get_pw() {
    return pwflg ? pwbuf : NULL;
}

/** Is TCP NoDelay (nagles algorithm) enabled?
 *
 * @return True if nodelay is enabled, False otherwise
 */
int ssh_get_nodelay_enabled() {
#ifdef TCP_NODELAY
    return tcp_nodelay;
#else
    return 0;
#endif
}

/** Gets the current terminal (VTERM) dimensions
 *
 * @param rows Out - number of rows (lines)
 * @param cols Out - number of columns (chars)
 */
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

/** Returns the current terminal type as a static string
 *
 * @return terminal type
 */
const char* get_current_terminal_type() {
    static char term_type[64];
    extern int tt_type, max_tt;
    extern struct tt_info_rec tt_info[];
    extern char * tn_term;

    debug(F100, "SSH Get terminal type", NULL, 0);

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

/** Is debug logging enabled?
 * @return True if debug logging is enabled, false otherwise.
 */
inline
int debug_logging() {
    return deblog;
}

unsigned char* get_display() {
    return tn_get_display();
}

int parse_displayname(char *displayname, int *familyp, char **hostp,
                      int *dpynump, int *scrnump, char **restp) {
    return fwdx_parse_displayname(displayname, familyp, hostp,
                                  dpynump, scrnump, restp);
}


/* Opens a socket for a new SSH connection if a proxy server is configured.
 * Returns INVALID_SOCKET if no proxy server is configured or opening the
 * socket failed in some way. In this situation, the caller should assume
 * no proxy server is configured and try opening the socket itself.
 *
 * @returns a socket or INVALID_SOCKET if no proxy server is configured.
 */
SOCKET ssh_open_socket(char* host, char* port) {
    /* TODO: If HTTP Proxy is configured, open a connection through
     *       that and return it.
     */

    return INVALID_SOCKET;
}

#ifdef SSH_DLL

/* SSH_DLL:
 * The SSH subsystem lives in a DLL that we've got to load at runtime, rather
 * than a particular SSH subsystem thats compiled in to the executable. All
 * the various SSH functions become stubs that call function pointers provided
 * by the DLL. */

#include <stdlib.h>

/* Typedefs for all the function pointers we *could* receive from an SSH
 * subsystem DLL */
typedef int (CKSSHAPI * p_ssh_dll_init_t)(ssh_init_parameters_t *);
typedef int (CKSSHAPI * p_ssh_set_iparam_t)(int, int);
typedef int (CKSSHAPI * p_ssh_get_iparam_t)(int);
typedef int (CKSSHAPI * p_ssh_set_sparam_t)(int, const char*);
typedef const char* (CKSSHAPI * p_ssh_get_sparam_t)(int);
typedef int (CKSSHAPI * p_ssh_set_identity_files_t)(const char**);
typedef int (CKSSHAPI * p_ssh_get_socket_t)();
typedef int (CKSSHAPI * p_ssh_open_t)();
typedef int (CKSSHAPI * p_ssh_clos_t)();
typedef int (CKSSHAPI * p_ssh_tchk_t)();
typedef int (CKSSHAPI * p_ssh_flui_t)();
typedef int (CKSSHAPI * p_ssh_break_t)();
typedef int (CKSSHAPI * p_ssh_inc_t)(int);
typedef int (CKSSHAPI * p_ssh_xin_t)(int, char*);
typedef int (CKSSHAPI * p_ssh_toc_t)(int);
typedef int (CKSSHAPI * p_ssh_tol_t)(char*,int);
typedef int (CKSSHAPI * p_ssh_snaws_t)();
typedef const char* (CKSSHAPI * p_ssh_proto_ver_t)();
typedef const char* (CKSSHAPI * p_ssh_impl_ver_t)();
typedef int (CKSSHAPI * p_sshkey_create_t)(char *, int, char *, int, char *);
typedef int (CKSSHAPI * p_sshkey_display_fingerprint_t)(char *, int);
typedef int (CKSSHAPI * p_sshkey_display_public_t)(char *, char *);
typedef int (CKSSHAPI * p_sshkey_display_public_as_ssh2_t)(char *,char *);
typedef int (CKSSHAPI * p_sshkey_change_passphrase_t)(char *, char *, char *);
typedef int (CKSSHAPI * p_ssh_fwd_remote_port_t)(char*, int, char *, int, BOOL);
typedef int (CKSSHAPI * p_ssh_fwd_local_port_t)(char*, int,char *,int, BOOL);
typedef int (CKSSHAPI * p_ssh_fwd_clear_local_ports_t)(BOOL);
typedef int (CKSSHAPI * p_ssh_fwd_clear_remote_ports_t)(BOOL);
typedef int (CKSSHAPI * p_ssh_fwd_remove_remote_port_t)(int, BOOL);
typedef int (CKSSHAPI * p_ssh_fwd_remove_local_port_t)(int, BOOL);
typedef ssh_port_forward_t* (CKSSHAPI * p_ssh_fwd_get_ports_t)();
#ifdef SSHTEST
typedef int (CKSSHAPI * p_sshkey_v1_change_comment_t)(char *, char *, char *);
#endif /* SSHTEST */
/*typedef char * (*p_sshkey_default_file_t)(int);*/
typedef void (CKSSHAPI * p_ssh_v2_rekey_t)();
typedef int (CKSSHAPI * p_ssh_agent_delete_file_t)(const char *);
typedef int (CKSSHAPI * p_ssh_agent_delete_all_t)();
typedef int (CKSSHAPI * p_ssh_agent_add_file_t)(const char*);
typedef int (CKSSHAPI * p_ssh_agent_list_identities_t)(int);
typedef void (CKSSHAPI * p_ssh_unload_t)();
typedef const char* (CKSSHAPI * p_ssh_dll_ver_t)();
typedef ktab_ret (CKSSHAPI * p_ssh_get_keytab_t)(int keytab_id);
typedef int (CKSSHAPI * p_ssh_feature_supported_t)(int feature_id);
typedef const char** (CKSSHAPI *p_ssh_get_set_help_t)();
typedef const char** (CKSSHAPI *p_ssh_get_help_t)();

/* Function pointers received from the currently loaded SSH subsystem DLL */
static p_ssh_dll_init_t p_ssh_init = NULL;
static p_ssh_set_iparam_t p_ssh_set_iparam = NULL;
static p_ssh_get_iparam_t p_ssh_get_iparam = NULL;
static p_ssh_set_sparam_t p_ssh_set_sparam = NULL;
static p_ssh_get_sparam_t p_ssh_get_sparam = NULL;
static p_ssh_set_identity_files_t p_ssh_set_identity_files = NULL;
static p_ssh_get_socket_t p_ssh_get_socket = NULL;
static p_ssh_open_t p_ssh_open = NULL;
static p_ssh_clos_t p_ssh_clos = NULL;
static p_ssh_tchk_t p_ssh_tchk = NULL;
static p_ssh_flui_t p_ssh_flui = NULL;
static p_ssh_break_t p_ssh_break = NULL;
static p_ssh_inc_t p_ssh_inc = NULL;
static p_ssh_xin_t p_ssh_xin = NULL;
static p_ssh_toc_t p_ssh_toc = NULL;
static p_ssh_tol_t p_ssh_tol = NULL;
static p_ssh_snaws_t p_ssh_snaws = NULL;
static p_ssh_proto_ver_t p_ssh_proto_ver = NULL;
static p_ssh_impl_ver_t p_ssh_impl_ver = NULL;
static p_sshkey_create_t p_sshkey_create = NULL;
static p_sshkey_display_fingerprint_t p_sshkey_display_fingerprint = NULL;
static p_sshkey_display_public_t p_sshkey_display_public = NULL;
static p_sshkey_display_public_as_ssh2_t p_sshkey_display_public_as_ssh2 = NULL;
static p_sshkey_change_passphrase_t p_sshkey_change_passphrase = NULL;
static p_ssh_fwd_remote_port_t p_ssh_fwd_remote_port = NULL;
static p_ssh_fwd_local_port_t p_ssh_fwd_local_port = NULL;
static p_ssh_fwd_clear_remote_ports_t p_ssh_fwd_clear_remote_ports = NULL;
static p_ssh_fwd_clear_local_ports_t p_ssh_fwd_clear_local_ports = NULL;
static p_ssh_fwd_remove_remote_port_t p_ssh_fwd_remove_remote_port = NULL;
static p_ssh_fwd_remove_local_port_t p_ssh_fwd_remove_local_port = NULL;
static p_ssh_fwd_get_ports_t p_ssh_fwd_get_ports = NULL;
#ifdef SSHTEST
static p_sshkey_v1_change_comment_t p_sshkey_v1_change_comment = NULL;  /* TODO */
#endif /* SSHTEST */
/*static p_sshkey_default_file_t p_sshkey_default_file = NULL;*/ /* TODO */
static p_ssh_v2_rekey_t p_ssh_v2_rekey = NULL;  /* TODO */
static p_ssh_agent_delete_file_t p_ssh_agent_delete_file = NULL;    /* TODO */
static p_ssh_agent_delete_all_t p_ssh_agent_delete_all = NULL;  /* TODO */
static p_ssh_agent_add_file_t p_ssh_agent_add_file = NULL;  /* TODO */
static p_ssh_agent_list_identities_t p_ssh_agent_list_identities = NULL;    /* TODO */
static p_ssh_unload_t p_ssh_unload = NULL;
static p_ssh_dll_ver_t p_ssh_dll_ver = NULL;
static p_ssh_get_keytab_t p_ssh_get_keytab = NULL;
static p_ssh_feature_supported_t p_ssh_feature_supported = NULL;
static p_ssh_get_set_help_t p_ssh_get_set_help = NULL;
static p_ssh_get_help_t p_ssh_get_help = NULL;

/* If a subsystem has been successfully loaded and initialised or not */
int ssh_subsystem_loaded = FALSE;

/* Name of the currently loaded SSH subsystem DLL */
char* dll_name = NULL;

/* Instance of the current SSH DLL */
#ifdef NT
#define F_CAST(t) (t)
static HINSTANCE hSSH;
#else
#define F_CAST(t) (t)
static HMODULE hSSH;
#endif

/** Called by the loaded SSH subsystem DLL to supply pointers to all of the
 * various fucntions it supports. Some functions are optional, others are not.
 *
 * @param function - Name of the function
 * @param p_function - Pointer to the function
 */
void ssh_install_func(const char* function, const void* p_function) {
    debug(F110, "ssh_install_func", function, 0);

    if ( !strcmp(function,"ssh_set_iparam") )
        p_ssh_set_iparam = F_CAST(p_ssh_set_iparam_t) p_function;
    else if ( !strcmp(function,"ssh_get_iparam") )
        p_ssh_get_iparam = F_CAST(p_ssh_get_iparam_t) p_function;
    else if ( !strcmp(function,"ssh_set_sparam") )
        p_ssh_set_sparam = F_CAST(p_ssh_set_sparam_t) p_function;
    else if ( !strcmp(function,"ssh_get_sparam") )
        p_ssh_get_sparam = F_CAST(p_ssh_get_sparam_t) p_function;
    else if ( !strcmp(function,"ssh_set_identity_files") )
        p_ssh_set_identity_files = F_CAST(p_ssh_set_identity_files_t) p_function;
    else if ( !strcmp(function,"ssh_get_socket") )
        p_ssh_get_socket = F_CAST(p_ssh_get_socket_t) p_function;
    else if ( !strcmp(function,"ssh_open") )
        p_ssh_open = F_CAST(p_ssh_open_t) p_function;
    else if ( !strcmp(function,"ssh_clos") )
        p_ssh_clos = F_CAST(p_ssh_clos_t) p_function;
    else if ( !strcmp(function,"ssh_tchk") )
        p_ssh_tchk = F_CAST(p_ssh_tchk_t) p_function;
    else if ( !strcmp(function,"ssh_flui") )
        p_ssh_flui = F_CAST(p_ssh_flui_t) p_function;
    else if ( !strcmp(function,"ssh_break") )
        p_ssh_break = F_CAST(p_ssh_break_t) p_function;
    else if ( !strcmp(function,"ssh_inc") )
        p_ssh_inc = F_CAST(p_ssh_inc_t) p_function;
    else if ( !strcmp(function,"ssh_xin") )
        p_ssh_xin = F_CAST(p_ssh_xin_t) p_function;
    else if ( !strcmp(function,"ssh_toc") )
        p_ssh_toc = F_CAST(p_ssh_toc_t) p_function;
    else if ( !strcmp(function,"ssh_tol") )
        p_ssh_tol = F_CAST(p_ssh_tol_t) p_function;
    else if ( !strcmp(function,"ssh_snaws") )
        p_ssh_snaws = F_CAST(p_ssh_snaws_t) p_function;
    else if ( !strcmp(function,"ssh_proto_ver") )
        p_ssh_proto_ver = F_CAST(p_ssh_proto_ver_t) p_function;
    else if ( !strcmp(function,"ssh_impl_ver") )
        p_ssh_impl_ver = F_CAST(p_ssh_impl_ver_t) p_function;
    else if ( !strcmp(function,"sshkey_create") )
        p_sshkey_create = F_CAST(p_sshkey_create_t) p_function;
    else if ( !strcmp(function,"sshkey_display_fingerprint") )
        p_sshkey_display_fingerprint = F_CAST(p_sshkey_display_fingerprint_t) p_function;
    else if ( !strcmp(function,"sshkey_display_public") )
        p_sshkey_display_public = F_CAST(p_sshkey_display_public_t) p_function;
    else if ( !strcmp(function,"sshkey_display_public_as_ssh2") )
        p_sshkey_display_public_as_ssh2 = F_CAST(p_sshkey_display_public_as_ssh2_t) p_function;
    else if ( !strcmp(function,"sshkey_change_passphrase") )
        p_sshkey_change_passphrase = F_CAST(p_sshkey_change_passphrase_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_remote_port") )
        p_ssh_fwd_remote_port = F_CAST(p_ssh_fwd_remote_port_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_local_port") )
        p_ssh_fwd_local_port = F_CAST(p_ssh_fwd_local_port_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_clear_remote_ports") )
        p_ssh_fwd_clear_remote_ports = F_CAST(p_ssh_fwd_clear_remote_ports_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_clear_local_ports") )
        p_ssh_fwd_clear_local_ports = F_CAST(p_ssh_fwd_clear_local_ports_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_remove_remote_port") )
        p_ssh_fwd_remove_remote_port = F_CAST(p_ssh_fwd_remove_remote_port_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_remove_local_port") )
        p_ssh_fwd_remove_local_port = F_CAST(p_ssh_fwd_remove_local_port_t) p_function;
    else if ( !strcmp(function,"ssh_fwd_get_ports") )
        p_ssh_fwd_get_ports = F_CAST(p_ssh_fwd_get_ports_t) p_function;
#ifdef SSHTEST
    else if ( !strcmp(function,"sshkey_v1_change_comment") )
        p_sshkey_v1_change_comment = F_CAST(p_sshkey_v1_change_comment_t) p_function;
#endif /* SSHTEST */
    /*else if ( !strcmp(function,"sshkey_default_file") )
        p_sshkey_default_file = (p_sshkey_default_file_t) p_function;*/
    else if ( !strcmp(function,"ssh_v2_rekey") )
        p_ssh_v2_rekey = F_CAST(p_ssh_v2_rekey_t) p_function;
    else if ( !strcmp(function,"ssh_agent_delete_file") )
        p_ssh_agent_delete_file = F_CAST(p_ssh_agent_delete_file_t) p_function;
    else if ( !strcmp(function,"ssh_agent_delete_all") )
        p_ssh_agent_delete_all = F_CAST(p_ssh_agent_delete_all_t) p_function;
    else if ( !strcmp(function,"ssh_agent_add_file") )
        p_ssh_agent_add_file = F_CAST(p_ssh_agent_add_file_t) p_function;
    else if ( !strcmp(function,"ssh_agent_list_identities") )
        p_ssh_agent_list_identities = F_CAST(p_ssh_agent_list_identities_t) p_function;
    else if ( !strcmp(function,"ssh_unload") )
        p_ssh_unload = F_CAST(p_ssh_unload_t) p_function;
    else if (!strcmp(function,"ssh_dll_ver"))
        p_ssh_dll_ver = F_CAST(p_ssh_dll_ver_t) p_function;
    else if (!strcmp(function,"ssh_get_keytab"))
        p_ssh_get_keytab = F_CAST(p_ssh_get_keytab_t) p_function;
    else if (!strcmp(function,"ssh_feature_supported"))
        p_ssh_feature_supported = F_CAST(p_ssh_feature_supported_t) p_function;
    else if (!strcmp(function,"ssh_get_set_help"))
        p_ssh_get_set_help = F_CAST(p_ssh_get_set_help_t) p_function;
    else if (!strcmp(function,"ssh_get_help"))
        p_ssh_get_help = F_CAST(p_ssh_get_help_t) p_function;
}

/** Attempts to load and initialise a particular SSH subsystem DLL
 * @return 0 on success, or a negative value on failure
 */
int ssh_load(char* dllname) {
    ULONG rc = 0;
    ssh_init_parameters_t init_params;

#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    int len;
    char fail[_MAX_PATH];
#endif

    debug(F110, "Attempting to load SSH DLL...", dllname, 0);

#ifdef NT
    hSSH = LoadLibrary(dllname);
    if (!hSSH) {
        rc = GetLastError();
        debug(F111, "K95 SSH LoadLibrary failed", dllname, rc);
        return -1;
    } else {
        p_ssh_init = (p_ssh_dll_init_t)GetProcAddress(hSSH, "ssh_dll_init");
        if (p_ssh_init == NULL) {
            rc = GetLastError();
            debug(F111, "K95 SSH GetProcAddress for ssh_dll_init failed", dllname, rc);
            FreeLibrary(hSSH);
            return -1;
        }
    }
#else
    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) ) {
        return -1;
    }
    sprintf(path, "%.*s%s", len, exe_path,dllname);         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hSSH);
    if (rc) {
        debug(F111, "K95 SSH LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hSSH);
    }
    if ( rc ) {
        debug(F111, "K95 SSH LoadLibrary failed",fail,rc) ;
        return -1;
    } else {
        debug(F111, "K95 SSH LoadLibrary success",fail,rc) ;
        if (rc = DosQueryProcAddr(hSSH,0,"ssh_dll_init",(PFN*)&p_ssh_init))
        {
            debug(F111,"K95 SSH GetProcAddress for ssh_dll_init failed", dllname, rc);
            DosFreeModule(hSSH);
            return -1;
        }
    }
#endif

    /* SSH DLL loaded successfully! Prepare init params! */
    init_params.version = 1;
    init_params.p_install_funcs = ssh_install_func;
    init_params.p_get_current_terminal_dimensions = get_current_terminal_dimensions;
    init_params.p_get_current_terminal_type = get_current_terminal_type;
    init_params.p_ssh_get_uid = ssh_get_uid;
    init_params.p_ssh_get_pw = ssh_get_pw;
    init_params.p_ssh_get_nodelay_enabled = ssh_get_nodelay_enabled;
    init_params.p_ssh_open_socket = ssh_open_socket;
    init_params.p_dodebug = dodebug;
    init_params.p_vscrnprintf = Vscrnprintf;
    init_params.p_uq_txt = uq_txt;
    init_params.p_uq_mtxt = uq_mtxt;
    init_params.p_uq_ok = uq_ok;
    init_params.p_uq_file = uq_file;
    init_params.p_zmkdir = zmkdir;
    init_params.p_ckmakxmsg = ckmakxmsg;
    init_params.p_whoami = whoami;
    init_params.p_GetAppData = GetAppData;
    init_params.p_GetHomePath = GetHomePath;
    init_params.p_GetHomeDrive = GetHomeDrive;
    init_params.p_ckstrncpy = ckstrncpy;
    init_params.p_debug_logging = debug_logging;
    init_params.p_get_display = tn_get_display;                  /* ckctel.c */
    init_params.p_parse_displayname = fwdx_parse_displayname;    /* ckctel.c */

    /* Initialise! */
    debug(F100, "Call ssh_dll_init()", NULL, 0);
    rc = p_ssh_init(&init_params);

    /* TODO: Check all mandatory functions are available */

    if (rc != 0) {
        debug(F111, "SSH DLL reported error", dllname, rc);
        ssh_dll_unload(TRUE);

        return -1;
    }

    ssh_subsystem_loaded = TRUE;
    debug(F111, "SSH DLL loaded", dllname, rc);
    return rc;
}

/** Resets all SSH function pointers to NULL and then unloads the current
 * SSH subsystem DLL.
 *
 * This function is currently only to be called if a problem is encountered
 * during the loading of an SSH DLL. As it doesn't terminate any active
 * SSH connection its not suitable for use after the SSH susbsytem has
 * been initialised and made available to the user.
 *
 * @param quiet If status messages can be written to the console or not
 * @return 0 on success
 */
int ssh_dll_unload(int quiet) {
    /* TODO: Kill any active SSH connection. Then perhaps make this an
     *       invisible command: "ssh unload" */

    if (p_ssh_unload) {
        p_ssh_unload();
    }

    ssh_subsystem_loaded = FALSE;

    p_ssh_init = NULL;
    p_ssh_set_iparam = NULL;
    p_ssh_get_iparam = NULL;
    p_ssh_set_sparam = NULL;
    p_ssh_get_sparam = NULL;
    p_ssh_set_identity_files = NULL;
    p_ssh_get_socket = NULL;
    p_ssh_open = NULL;
    p_ssh_clos = NULL;
    p_ssh_tchk = NULL;
    p_ssh_flui = NULL;
    p_ssh_break = NULL;
    p_ssh_inc = NULL;
    p_ssh_xin = NULL;
    p_ssh_toc = NULL;
    p_ssh_tol = NULL;
    p_ssh_snaws = NULL;
    p_ssh_proto_ver = NULL;
    p_ssh_impl_ver = NULL;
    p_sshkey_create = NULL;
    p_sshkey_display_fingerprint = NULL;
    p_sshkey_display_public = NULL;
    p_sshkey_display_public_as_ssh2 = NULL;
    p_sshkey_change_passphrase = NULL;
    p_ssh_fwd_remote_port = NULL;
    p_ssh_fwd_local_port = NULL;
    p_ssh_fwd_clear_remote_ports = NULL;
    p_ssh_fwd_clear_local_ports = NULL;
    p_ssh_fwd_remove_remote_port = NULL;
    p_ssh_fwd_remove_local_port = NULL;
    p_ssh_fwd_get_ports = NULL;
#ifdef SSHTEST
    p_sshkey_v1_change_comment = NULL;  /* TODO */
#endif /* SSHTEST */
    /*  p_sshkey_default_file = NULL;*/ /* TODO */
    p_ssh_v2_rekey = NULL;  /* TODO */
    p_ssh_agent_delete_file = NULL;    /* TODO */
    p_ssh_agent_delete_all = NULL;  /* TODO */
    p_ssh_agent_add_file = NULL;  /* TODO */
    p_ssh_agent_list_identities = NULL;    /* TODO */
    p_ssh_unload = NULL;
    p_ssh_get_set_help = NULL;
    p_ssh_get_help = NULL;

    #ifdef NT
    FreeLibrary(hSSH);
    #else
    DosFreeModule(hSSH);
    #endif /* NT */

    return 0;
}

/** Try to load an SSH subsystem DLL
 *
 * @param dll_names A list of one or more SSH subsystem DLLs to try loading
 * @param quiet If messages can be written to the console or not
 * @return 0 on success
 */
int ssh_dll_load(const char* dll_names, int quiet) {
    int rc = 0;
    char* dlls;
    char* dll;
    char* delim = ";";

    if (ssh_subsystem_loaded) {
        /* This should never happen as the "ssh load" command is removed when
         * the SSH subsystem is initialised. But just in case, do nothing. */
        return 0;
    }

    debug(F111, "SSH Init", dll_names, quiet);

    dlls = _strdup(dll_names);

    dll = strtok(dlls, delim);

    while (dll != NULL) {
        debug(F110, "Attempting DLL", dll, 0);
        rc = ssh_load(dll);
        if (rc >= 0) {
            /* success! */
            if (dll_name)
                free(dll_name);
            dll_name = _strdup(dll);
            free(dlls);

            debug(F101, "SSH Init complete!", NULL, rc);

            return rc;
        }
        debug(F110, "SSH init attempt failed with dll", dll, 0);
        dll = strtok(NULL, delim);
    }
    debug(F100, "SSH init: all out of DLLs to try. Init fails.", NULL, 0);

    if (rc < 0 && !quiet) {
        printf("Failed to load a compatible SSH backend DLL. SSH commands unavailable.\n");
    }
    return rc;
}

/** Returns true if SSH commands are currently available (if a SSH subsystem
 * has been loaded and initialised)
 * @return True if SSH commands are available
 */
int ssh_avail() {
    return ssh_subsystem_loaded;
}

/** Provides the filename of the currently loaded SSH DLL
 *
 * @return SSH DLL filename
 */
const char* ssh_dll_name() {
    if (ssh_avail())
        return dll_name;
    return NULL;
}

/** Provides the version of the currently loaded SSH DLL
 *
 * @return SSH DLL version
 */
const char* ssh_dll_ver() {
    if (p_ssh_dll_ver)
        return p_ssh_dll_ver();
    return NULL;
}

int ssh_set_iparam(int param, int value) {
    if (p_ssh_set_iparam)
        return p_ssh_set_iparam(param, value);

    /* ERROR - this function is mandatory */

    return -1;
}


int ssh_get_iparam(int param) {
    if (p_ssh_get_iparam)
        return p_ssh_get_iparam(param);

    /* These are the default values for the various settings in K95 2.1.3 */
    switch(param) {
        case SSH_IPARAM_CMP:
        case SSH_IPARAM_2_ARK:
        case SSH_IPARAM_GKX:
        case SSH_IPARAM_K5_IS_K4:
            return 1;
        case SSH_IPARAM_VRB:
        case SSH_IPARAM_SHK:
            return 2;
        case SSH_IPARAM_AFW:
        case SSH_IPARAM_XFW:
        case SSH_IPARAM_PRP:
        case SSH_IPARAM_CAS:
        case SSH_IPARAM_SHH:
        case SSH_IPARAM_VER:
        case SSH_IPARAM_CHKIP:
        case SSH_IPARAM_GWP:
        case SSH_IPARAM_DYF:
        case SSH_IPARAM_GSD:
        case SSH_IPARAM_K4TGT:
        case SSH_IPARAM_K5TGT:
        case SSH_IPARAM_CFG:
        case SSH_IPARAM_HBT:
        default:
            return 0;
    }

    return 0;
}

int ssh_set_sparam(int param, const char* value) {
    if (p_ssh_set_sparam)
        return p_ssh_set_sparam(param, value);

    /* ERROR - this function is mandatory */

    return -1;
}

const char* ssh_get_sparam(int param) {
    if (p_ssh_get_sparam)
        return p_ssh_get_sparam(param);

    return NULL;
}

/** Set the list of identity files to use for authentication.
 * @param identity_files The list of identity file names, null terminated.
 */
int ssh_set_identity_files(const char** identity_files) {
    if (p_ssh_set_identity_files)
        return p_ssh_set_identity_files(identity_files);
    return -1;
}

/** Get the socket currently in use by the SSH client.
 *
 * @returns Socket for the current SSH connection, or -1 if not implemented or
 *      no active connection
 */
int ssh_get_socket() {
    if (p_ssh_get_socket)
        return p_ssh_get_socket();
    return -1;
}

/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open(){
    if (p_ssh_open)
        return p_ssh_open();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Closes any existing SSH Session.
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos() {
    if (p_ssh_clos)
        return p_ssh_clos();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Network I/O Check. This function is polled by K95 to get the status
 * of the connection and find out how many bytes are waiting to be
 * processed. It is called frequently so must return quickly.
 *
 * @return >= 0 indicates number of bytes waiting to be read
 *          < 0 indicates a fatal error and the connection should be closed.
 */
int ssh_tchk() {
    if (p_ssh_tchk)
        return p_ssh_tchk();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui() {
    if (p_ssh_flui)
        return p_ssh_flui();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Network Break - send a break signal to the remote process.
 * This may or may not do anything. Supported on SSH V2 only.
 *
 * @return
 */
int ssh_break() {
   if (p_ssh_break)
        return p_ssh_break();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Input Character. Reads one character from the input queue
 *
 * @param timeout 0 for none (block until there is a character to read),
 *                positive for seconds, negative for milliseconds
 * @return -1 for timeout, >= 0 is a valid character, < -1 is a fatal error
 */
int ssh_inc(int timeout) {
    if (p_ssh_inc)
        return p_ssh_inc(timeout);

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
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
    if (p_ssh_xin)
        return p_ssh_xin(count, buffer);

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Terminal Output Character - send a single character. Blocks until the
 * character has been handled.
 *
 * @param c character to send
 * @return 0 for success, <0 for error
 */
int ssh_toc(int c) {
    if (p_ssh_toc)
        return p_ssh_toc(c);

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Terminal Output Line - send multiple characters.
 *
 * @param buffer characters to be sent
 * @param count number of characters to be sent
 * @return  >= 0 for number of characters sent, <0 for a fatal error.
 */
int ssh_tol(char * buffer, int count) {
    if (p_ssh_tol)
        return p_ssh_tol(buffer, count);

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Negotiate About Window Size. Let the remote host know the window dimensions
 * and terminal type if these have changed.
 *
 */
int ssh_snaws() {
    if (p_ssh_snaws)
        return p_ssh_snaws();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
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
int ssh_fwd_remote_port(char* address, int port, char * host, int host_port, BOOL apply) {
    if (p_ssh_fwd_remote_port)
        return p_ssh_fwd_remote_port(address, port, host, host_port, apply);

    /* optional feature not available */

    return -1;
}

/** Add a new Direct (local) port forward for future connections. This is
 * called by the following commands:
 *    SSH ADD LOCAL-PORT-FORWARD
 *    SSH FORWARD-LOCAL-PORT
 *
 * @param address Address K95 should listen on for new connections (reserved
 *              for future use)
 * @param port Port K95 will listen on for new connections
 * @param host Host connetions will be made to from the server
 * @param host_port port connections will be made to from the server
 * @param apply Add the new forwarding to any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_local_port(char* address, int port, char *host, int host_port, BOOL apply) {
    if (p_ssh_fwd_local_port)
        return p_ssh_fwd_local_port(address, port, host, host_port, apply);

    /* optional feature not available */

    return -1;
}

/** Clears all remote port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all remote ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_remote_ports(BOOL apply) {
    if (p_ssh_fwd_clear_remote_ports)
        return p_ssh_fwd_clear_remote_ports(apply);

    /* optional feature not available */

    return -1;
}

/** Clears all local port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all  ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_local_ports(BOOL apply) {
    if (p_ssh_fwd_clear_local_ports)
        return p_ssh_fwd_clear_local_ports(apply);

    /* optional feature not available */

    return -1;
}

 /** Remove a single reverse/remote port forward.
 *
 * @param port Reverse port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_remote_port(int port, BOOL apply) {
        if (p_ssh_fwd_remove_remote_port)
        return p_ssh_fwd_remove_remote_port(port, apply);

    /* optional feature not available */

    return -1;
}

/** Remove a single direct/local port forward.
 *
 * @param port Direct port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_local_port(int port, BOOL apply) {
    if (p_ssh_fwd_remove_local_port)
        return p_ssh_fwd_remove_local_port(port, apply);

    /* optional feature not available */

    return -1;
}

/** Gets all forwarded ports. The final entry in the list has a type of
 * SSH_PORT_FORWARD_NULL.
 *
 * @returns List of forwarded ports, or NULL on error or empty list
 */
const ssh_port_forward_t* ssh_fwd_get_ports() {
    if (p_ssh_fwd_get_ports)
        return p_ssh_fwd_get_ports();

    /* optional feature not available */

    return NULL;
}

/** Creates an SSH key pair
 *
 * @param filename File to write the private key to
 * @param bits Length of the key in bits. Valid options vary by key type
 * @param pp Passphrase
 * @param type Key type
 * @param cmd_comment SSH V1 RSA Comment (obsolete)
 * @return
 */
int sshkey_create(char * filename, int bits, char *pp, int type,
                  char *cmd_comment) {
    if (p_sshkey_create)
        return p_sshkey_create(filename, bits, pp, type, cmd_comment);

    /* optional feature not available */

    return -1;
}

/** Displays the fingerprint for the specified public key
 *
 * @param filename Key file to display the fingerprint for. If not supplied, one
 *                 will be prompted for
 * @param babble 0 - fingerprint, 1 - IETF (ignored)
 * @return 0
 */
int sshkey_display_fingerprint(char * filename, int babble) {
    if (p_sshkey_display_fingerprint)
        return p_sshkey_display_fingerprint(filename, babble);

    /* optional feature not available */

    return -1;
}

/** Outputs the public key for the specified private key - same as what you'd
 * put in authorized_keys.
 * @param filename key file - will be prompted if not specified
 * @param passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public(char * filename, char *identity_passphrase) {
    if (p_sshkey_display_public)
        return p_sshkey_display_public(filename, identity_passphrase);

    /* optional feature not available */

    return -1;
}

/** Like sshkey_display_public but puts some extra cruft around the output.
 *
 * @param filename key file - will be prompted if not specified
 * @param identity_passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    if (p_sshkey_display_public_as_ssh2)
        return p_sshkey_display_public_as_ssh2(filename, identity_passphrase);

    /* optional feature not available */

    return -1;
}

int sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    if (p_sshkey_change_passphrase)
        return p_sshkey_change_passphrase(filename, oldpp, newpp);

    /* optional feature not available */

    return -1;
}

#ifdef SSHTEST
int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    if (p_sshkey_v1_change_comment)
        return p_sshkey_v1_change_comment(filename, comment, pp);

    /* optional feature not available */

    return -1;
}
#endif /* SSHTEST */

#ifdef COMMENT
char * sshkey_default_file(int a) {
    return NULL; /* TODO */
}
#endif

void ssh_v2_rekey() {
    if (p_ssh_v2_rekey)
        p_ssh_v2_rekey();

    /* optional feature not available */
}

/** Return the current protocol version.
 *
 * @return Current protocol version (eg, "SSH-2.0")
 */
const char * ssh_proto_ver() {
    if (p_ssh_proto_ver)
        return p_ssh_proto_ver();

    return NULL;
}

/** Return the current SSH backend/implementation version.
 * @return SSH backend/implementation version
 */
const char * ssh_impl_ver() {
    if (p_ssh_impl_ver)
        return p_ssh_impl_ver();

    return NULL;
}

int ssh_agent_delete_file(const char *filename) {
    if (p_ssh_agent_delete_file)
        return p_ssh_agent_delete_file(filename);

    /* optional feature not available */

    return -1;
}

int ssh_agent_delete_all() {
    if (p_ssh_agent_delete_all)
        return p_ssh_agent_delete_all();

    /* optional feature not available */

    return -1;
}

int ssh_agent_add_file (const char *filename) {
    if (p_ssh_agent_add_file)
        return p_ssh_agent_add_file(filename);

    /* optional feature not available */

    return -1;
}

int ssh_agent_list_identities(int do_fp) {
    if (p_ssh_agent_list_identities)
        return p_ssh_agent_list_identities(do_fp);

    /* optional feature not available */

    return -1;
}

/** Gets the specified keyword table.
 *
 * @param keytab_id keyword table to get
 * @return the requested keyword table
 */
ktab_ret ssh_get_keytab(int keytab_id) {
    if (p_ssh_get_keytab)
        return p_ssh_get_keytab(keytab_id);

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    {
        ktab_ret ret;
        ret.rc = -1;
        return ret;
    }
}

/** Returns TRUE if the specified feature is supported by the SSH backend,
 * or FALSE if it is not. This is used to enable or disable various "ssh" and
 * "set ssh" commands and options.
 *
 * @param feature_id The feature being checked
 * @return TRUE if the feature is supported, or FALSE if it is not
 */
int ssh_feature_supported(int feature_id) {
    if (p_ssh_feature_supported)
        return p_ssh_feature_supported(feature_id);
    return FALSE; /* No features supported! */
}

const char** ssh_get_set_help() {
    const char** result;
    static const char *hmxyssh[] = {
"No help content for SET SSH was provided by the currently loaded SSH backend.",
""
};
    if (p_ssh_get_set_help)
        result = p_ssh_get_set_help();

    if (result != NULL)
        return result;

    return hmxyssh;
}

const char** ssh_get_help() {
    const char** result;
    static const char *hmxyssh[] = {
"No help content for SSH was provided by the currently loaded SSH backend.",
""
};
    if (p_ssh_get_help)
        result = p_ssh_get_help();

    if (result != NULL)
        return result;

    return hmxyssh;
}


#endif /* SSH_DLL */
