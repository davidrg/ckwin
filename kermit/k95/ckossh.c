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

void update_setssh_options();   /* This lives in ckuus3.c */
void update_ssh_options();      /* This lives in ckuusr.c */
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
const char* ssh_get_uid(void) {
    return uidbuf;
}

/** Gets the currently set password if there is one.
 *
 * @return Currently set password or NULL
 */
const char* ssh_get_pw(void) {
    return pwflg ? pwbuf : NULL;
}

/** Is TCP NoDelay (nagles algorithm) enabled?
 *
 * @return True if nodelay is enabled, False otherwise
 */
int ssh_get_nodelay_enabled(void) {
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
const char* get_current_terminal_type(void) {
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
int debug_logging(void) {
    return deblog;
}

unsigned char* get_display(void) {
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

typedef int CKSSHDLLENTRY ssh_dll_init_dllentry(ssh_dll_init_data *);
static ssh_dll_init_dllentry *dllentryp_ssh_init = NULL;

/* Function pointers received from the currently loaded SSH subsystem DLL */
static ssh_set_iparam_dllfunc *dllfuncp_ssh_set_iparam = NULL;
static ssh_get_iparam_dllfunc *dllfuncp_ssh_get_iparam = NULL;
static ssh_set_sparam_dllfunc *dllfuncp_ssh_set_sparam = NULL;
static ssh_get_sparam_dllfunc *dllfuncp_ssh_get_sparam = NULL;
static ssh_set_identity_files_dllfunc *dllfuncp_ssh_set_identity_files = NULL;
static ssh_get_socket_dllfunc *dllfuncp_ssh_get_socket = NULL;
static ssh_open_dllfunc *dllfuncp_ssh_open = NULL;
static ssh_clos_dllfunc *dllfuncp_ssh_clos = NULL;
static ssh_tchk_dllfunc *dllfuncp_ssh_tchk = NULL;
static ssh_flui_dllfunc *dllfuncp_ssh_flui = NULL;
static ssh_break_dllfunc *dllfuncp_ssh_break = NULL;
static ssh_inc_dllfunc *dllfuncp_ssh_inc = NULL;
static ssh_xin_dllfunc *dllfuncp_ssh_xin = NULL;
static ssh_toc_dllfunc *dllfuncp_ssh_toc = NULL;
static ssh_tol_dllfunc *dllfuncp_ssh_tol = NULL;
static ssh_snaws_dllfunc *dllfuncp_ssh_snaws = NULL;
static ssh_proto_ver_dllfunc *dllfuncp_ssh_proto_ver = NULL;
static ssh_impl_ver_dllfunc *dllfuncp_ssh_impl_ver = NULL;
static sshkey_create_dllfunc *dllfuncp_sshkey_create = NULL;
static sshkey_display_fingerprint_dllfunc *dllfuncp_sshkey_display_fingerprint = NULL;
static sshkey_display_public_dllfunc *dllfuncp_sshkey_display_public = NULL;
static sshkey_display_public_as_ssh2_dllfunc *dllfuncp_sshkey_display_public_as_ssh2 = NULL;
static sshkey_change_passphrase_dllfunc *dllfuncp_sshkey_change_passphrase = NULL;
static ssh_fwd_remote_port_dllfunc *dllfuncp_ssh_fwd_remote_port = NULL;
static ssh_fwd_local_port_dllfunc *dllfuncp_ssh_fwd_local_port = NULL;
static ssh_fwd_clear_remote_ports_dllfunc *dllfuncp_ssh_fwd_clear_remote_ports = NULL;
static ssh_fwd_clear_local_ports_dllfunc *dllfuncp_ssh_fwd_clear_local_ports = NULL;
static ssh_fwd_remove_remote_port_dllfunc *dllfuncp_ssh_fwd_remove_remote_port = NULL;
static ssh_fwd_remove_local_port_dllfunc *dllfuncp_ssh_fwd_remove_local_port = NULL;
static ssh_fwd_get_ports_dllfunc *dllfuncp_ssh_fwd_get_ports = NULL;
#ifdef SSHTEST
static sshkey_v1_change_comment_dllfunc *dllfuncp_sshkey_v1_change_comment = NULL;  /* TODO */
#endif /* SSHTEST */
#if 0
static sshkey_default_file_dllfunc *dllfuncp_sshkey_default_file = NULL; /* TODO */
#endif
static ssh_v2_rekey_dllfunc *dllfuncp_ssh_v2_rekey = NULL;  /* TODO */
static ssh_agent_delete_file_dllfunc *dllfuncp_ssh_agent_delete_file = NULL;    /* TODO */
static ssh_agent_delete_all_dllfunc *dllfuncp_ssh_agent_delete_all = NULL;  /* TODO */
static ssh_agent_add_file_dllfunc *dllfuncp_ssh_agent_add_file = NULL;  /* TODO */
static ssh_agent_list_identities_dllfunc *dllfuncp_ssh_agent_list_identities = NULL;    /* TODO */
static ssh_set_environment_variable_dllfunc *dllfuncp_ssh_set_environment_variable = NULL;
static ssh_clear_environment_variables_dllfunc *dllfuncp_ssh_clear_environment_variables= NULL;
static ssh_unload_dllfunc *dllfuncp_ssh_unload = NULL;
static ssh_dll_ver_dllfunc *dllfuncp_ssh_dll_ver = NULL;
static ssh_get_keytab_dllfunc *dllfuncp_ssh_get_keytab = NULL;
static ssh_feature_supported_dllfunc *dllfuncp_ssh_feature_supported = NULL;
static ssh_get_set_help_dllfunc *dllfuncp_ssh_get_set_help = NULL;
static ssh_get_help_dllfunc *dllfuncp_ssh_get_help = NULL;

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

static int scrnprint(const char *str)
{
    return( Vscrnprintf(str) );
}

#ifdef SSH_DLL_CALLCONV

/* define prototypes for callback functions */
static ssh_get_uid_callback callback_ssh_get_uid;
static ssh_get_pw_callback callback_ssh_get_pw;
static ssh_get_nodelay_enabled_callback callback_ssh_get_nodelay_enabled;
static get_current_terminal_dimensions_callback callback_get_current_terminal_dimensions;
static get_current_terminal_type_callback callback_get_current_terminal_type;
static get_display_callback callback_get_display;
static parse_displayname_callback callback_parse_displayname;
static scrnprint_callback callback_scrnprint;
static ckstrncpy_callback callback_ckstrncpy;
static ssh_open_socket_callback callback_ssh_open_socket;
static dodebug_callback callback_dodebug;
static uq_txt_callback callback_uq_txt;
static uq_mtxt_callback callback_uq_mtxt;
static uq_ok_callback callback_uq_ok;
static uq_file_callback callback_uq_file;
static GetAppData_callback callback_GetAppData;
static GetHomePath_callback callback_GetHomePath;
static GetHomeDrive_callback callback_GetHomeDrive;
static whoami_callback callback_whoami;
static zmkdir_callback callback_zmkdir;
static ckmakxmsg_callback callback_ckmakxmsg;
static debug_logging_callback callback_debug_logging;

static const char* CKSSHAPI callback_ssh_get_uid(void) {
    return ssh_get_uid();
}

static const char* CKSSHAPI callback_ssh_get_pw(void) {
    return ssh_get_pw();
}

static int CKSSHAPI callback_ssh_get_nodelay_enabled(void) {
    return ssh_get_nodelay_enabled();
}

static void CKSSHAPI callback_get_current_terminal_dimensions(int* rows, int* cols) {
    get_current_terminal_dimensions(rows, cols);
}

static const char* CKSSHAPI callback_get_current_terminal_type(void) {
    return( get_current_terminal_type() );
}

static unsigned char* CKSSHAPI callback_get_display(void) {
    return get_display();
}

static int CKSSHAPI callback_parse_displayname(char *displayname, int *familyp, char **hostp,
                      int *dpynump, int *scrnump, char **restp) {
    return parse_displayname(displayname, familyp, hostp,
                                  dpynump, scrnump, restp);
}

static int CKSSHAPI callback_scrnprint(const char *str)
{
    return( scrnprint(str) );
}

static int CKSSHAPI callback_ckstrncpy(char * dest, const char * src, int len)
{
    return( ckstrncpy(dest, src, len) );
}

static SOCKET CKSSHAPI callback_ssh_open_socket(char* host, char* port) {
    return ssh_open_socket(host, port);
}

static int CKSSHAPI callback_dodebug(int flag, char * s1, char * s2, CK_OFF_T n)
{
    return( dodebug(flag, s1, s2, n) );
}

static int CKSSHAPI callback_uq_txt(char * preface, char * prompt, int echo, char ** help, char * buf,
       int buflen, char *dflt, int timer) {
    return uq_txt(preface, prompt, echo, help, buf, buflen, dflt, timer);
}

static int CKSSHAPI callback_uq_mtxt(char * preface,char **help, int n, struct txtbox field[]) {
    return uq_mtxt(preface, help, n, field);
}

static int CKSSHAPI callback_uq_ok(char * preface, char * prompt, int mask,char ** help, int dflt) {
    return uq_ok(preface, prompt, mask, help, dflt);
}

static int CKSSHAPI callback_uq_file(char * preface, char * fprompt, int fc, char ** help,
        char * dflt, char * result, int rlength) {
    return uq_file(preface, fprompt, fc, help, dflt, result, rlength);
}

static char* CKSSHAPI callback_GetAppData(int common) {
    return GetAppData(common);
}

static char* CKSSHAPI callback_GetHomePath(void) {
    return GetHomePath();
}

static char* CKSSHAPI callback_GetHomeDrive(void) {
    return GetHomeDrive();
}

static char* CKSSHAPI callback_whoami(void) {
    return whoami();
}

static int CKSSHAPI callback_zmkdir(char *path) {
    return zmkdir(path);
}

static int CKSSHAPI callback_ckmakxmsg(char * buf, int len, char *s1, char *s2, char *s3,
        char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
        char *s10, char *s11, char *s12) {
    return ckmakxmsg(buf, len, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12);
}

static int CKSSHAPI callback_debug_logging(void) {
    return debug_logging();
}

#else /* SSH_DLL_CALLCONV */

#define callback_ssh_get_uid                ssh_get_uid
#define callback_ssh_get_pw                 ssh_get_pw
#define callback_ssh_get_nodelay_enabled    ssh_get_nodelay_enabled
#define callback_get_current_terminal_dimensions    get_current_terminal_dimensions
#define callback_get_current_terminal_type  get_current_terminal_type
#define callback_get_display                get_display
#define callback_parse_displayname          parse_displayname
#define callback_scrnprint                  scrnprint
#define callback_ckstrncpy                  ckstrncpy
#define callback_ssh_open_socket            ssh_open_socket
#define callback_dodebug                    dodebug
#define callback_uq_txt                     uq_txt
#define callback_uq_mtxt                    uq_mtxt
#define callback_uq_ok                      uq_ok
#define callback_uq_file                    uq_file
#define callback_GetAppData                 GetAppData
#define callback_GetHomePath                GetHomePath
#define callback_GetHomeDrive               GetHomeDrive
#define callback_whoami                     whoami
#define callback_zmkdir                     zmkdir
#define callback_ckmakxmsg                  ckmakxmsg
#define callback_debug_logging              debug_logging

#endif /* SSH_DLL_CALLCONV */

/** Called by the loaded SSH subsystem DLL to supply pointers to all of the
 * various fucntions it supports. Some functions are optional, others are not.
 *
 * @param function - Name of the function
 * @param p_function - Pointer to the function
 */
static void CKSSHAPI callback_install_dllfunc(const char* function, const void* p_function) {
    debug(F110, "ssh_install_func", function, 0);

    if ( !strcmp(function,"ssh_set_iparam") )
        dllfuncp_ssh_set_iparam = F_CAST(ssh_set_iparam_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_get_iparam") )
        dllfuncp_ssh_get_iparam = F_CAST(ssh_get_iparam_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_set_sparam") )
        dllfuncp_ssh_set_sparam = F_CAST(ssh_set_sparam_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_get_sparam") )
        dllfuncp_ssh_get_sparam = F_CAST(ssh_get_sparam_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_set_identity_files") )
        dllfuncp_ssh_set_identity_files = F_CAST(ssh_set_identity_files_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_get_socket") )
        dllfuncp_ssh_get_socket = F_CAST(ssh_get_socket_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_open") )
        dllfuncp_ssh_open = F_CAST(ssh_open_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_clos") )
        dllfuncp_ssh_clos = F_CAST(ssh_clos_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_tchk") )
        dllfuncp_ssh_tchk = F_CAST(ssh_tchk_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_flui") )
        dllfuncp_ssh_flui = F_CAST(ssh_flui_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_break") )
        dllfuncp_ssh_break = F_CAST(ssh_break_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_inc") )
        dllfuncp_ssh_inc = F_CAST(ssh_inc_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_xin") )
        dllfuncp_ssh_xin = F_CAST(ssh_xin_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_toc") )
        dllfuncp_ssh_toc = F_CAST(ssh_toc_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_tol") )
        dllfuncp_ssh_tol = F_CAST(ssh_tol_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_snaws") )
        dllfuncp_ssh_snaws = F_CAST(ssh_snaws_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_proto_ver") )
        dllfuncp_ssh_proto_ver = F_CAST(ssh_proto_ver_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_impl_ver") )
        dllfuncp_ssh_impl_ver = F_CAST(ssh_impl_ver_dllfunc *)p_function;
    else if ( !strcmp(function,"sshkey_create") )
        dllfuncp_sshkey_create = F_CAST(sshkey_create_dllfunc *)p_function;
    else if ( !strcmp(function,"sshkey_display_fingerprint") )
        dllfuncp_sshkey_display_fingerprint = F_CAST(sshkey_display_fingerprint_dllfunc *)p_function;
    else if ( !strcmp(function,"sshkey_display_public") )
        dllfuncp_sshkey_display_public = F_CAST(sshkey_display_public_dllfunc *)p_function;
    else if ( !strcmp(function,"sshkey_display_public_as_ssh2") )
        dllfuncp_sshkey_display_public_as_ssh2 = F_CAST(sshkey_display_public_as_ssh2_dllfunc *)p_function;
    else if ( !strcmp(function,"sshkey_change_passphrase") )
        dllfuncp_sshkey_change_passphrase = F_CAST(sshkey_change_passphrase_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_remote_port") )
        dllfuncp_ssh_fwd_remote_port = F_CAST(ssh_fwd_remote_port_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_local_port") )
        dllfuncp_ssh_fwd_local_port = F_CAST(ssh_fwd_local_port_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_clear_remote_ports") )
        dllfuncp_ssh_fwd_clear_remote_ports = F_CAST(ssh_fwd_clear_remote_ports_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_clear_local_ports") )
        dllfuncp_ssh_fwd_clear_local_ports = F_CAST(ssh_fwd_clear_local_ports_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_remove_remote_port") )
        dllfuncp_ssh_fwd_remove_remote_port = F_CAST(ssh_fwd_remove_remote_port_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_remove_local_port") )
        dllfuncp_ssh_fwd_remove_local_port = F_CAST(ssh_fwd_remove_local_port_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_fwd_get_ports") )
        dllfuncp_ssh_fwd_get_ports = F_CAST(ssh_fwd_get_ports_dllfunc *)p_function;
#ifdef SSHTEST
    else if ( !strcmp(function,"sshkey_v1_change_comment") )
        dllfuncp_sshkey_v1_change_comment = F_CAST(sshkey_v1_change_comment_dllfunc *)p_function;
#endif /* SSHTEST */
    /*else if ( !strcmp(function,"sshkey_default_file") )
        dllfuncp_sshkey_default_file = (sshkey_default_file_dllfunc *)p_function;*/
    else if ( !strcmp(function,"ssh_v2_rekey") )
        dllfuncp_ssh_v2_rekey = F_CAST(ssh_v2_rekey_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_agent_delete_file") )
        dllfuncp_ssh_agent_delete_file = F_CAST(ssh_agent_delete_file_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_agent_delete_all") )
        dllfuncp_ssh_agent_delete_all = F_CAST(ssh_agent_delete_all_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_agent_add_file") )
        dllfuncp_ssh_agent_add_file = F_CAST(ssh_agent_add_file_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_agent_list_identities") )
        dllfuncp_ssh_agent_list_identities = F_CAST(ssh_agent_list_identities_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_set_environment_variable") )
        dllfuncp_ssh_set_environment_variable = F_CAST(ssh_set_environment_variable_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_clear_environment_variables") )
        dllfuncp_ssh_clear_environment_variables = F_CAST(ssh_clear_environment_variables_dllfunc *)p_function;
    else if ( !strcmp(function,"ssh_unload") )
        dllfuncp_ssh_unload = F_CAST(ssh_unload_dllfunc *)p_function;
    else if (!strcmp(function,"ssh_dll_ver"))
        dllfuncp_ssh_dll_ver = F_CAST(ssh_dll_ver_dllfunc *)p_function;
    else if (!strcmp(function,"ssh_get_keytab"))
        dllfuncp_ssh_get_keytab = F_CAST(ssh_get_keytab_dllfunc *)p_function;
    else if (!strcmp(function,"ssh_feature_supported"))
        dllfuncp_ssh_feature_supported = F_CAST(ssh_feature_supported_dllfunc *)p_function;
    else if (!strcmp(function,"ssh_get_set_help"))
        dllfuncp_ssh_get_set_help = F_CAST(ssh_get_set_help_dllfunc *)p_function;
    else if (!strcmp(function,"ssh_get_help"))
        dllfuncp_ssh_get_help = F_CAST(ssh_get_help_dllfunc *)p_function;
}

/** Attempts to load and initialise a particular SSH subsystem DLL
 * @return 0 on success, or a negative value on failure
 */
int ssh_load(char* dllname) {
    ULONG rc = 0;
    ssh_dll_init_data init;
#ifdef CK_COLORS_24BIT
    extern int colorpalette;
#endif

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
        dllentryp_ssh_init = F_CAST(ssh_dll_init_dllentry *)GetProcAddress(hSSH, "ssh_dll_init");
        if (dllentryp_ssh_init == NULL) {
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
        if (rc = DosQueryProcAddr(hSSH,0,"ssh_dll_init",(PFN*)&dllentryp_ssh_init))
        {
            debug(F111,"K95 SSH GetProcAddress for ssh_dll_init failed", dllname, rc);
            DosFreeModule(hSSH);
            return -1;
        }
    }
#endif

    /* SSH DLL loaded successfully! Prepare init params! */
    init.version = 1;
    init.callbackp_install_dllfunc = callback_install_dllfunc;
    init.callbackp_get_current_terminal_dimensions = callback_get_current_terminal_dimensions;
    init.callbackp_get_current_terminal_type = callback_get_current_terminal_type;
    init.callbackp_ssh_get_uid = callback_ssh_get_uid;
    init.callbackp_ssh_get_pw = callback_ssh_get_pw;
    init.callbackp_ssh_get_nodelay_enabled = callback_ssh_get_nodelay_enabled;
    init.callbackp_ssh_open_socket = callback_ssh_open_socket;
    init.callbackp_dodebug = callback_dodebug;
    init.callbackp_scrnprint = callback_scrnprint;
    init.callbackp_uq_txt = callback_uq_txt;
    init.callbackp_uq_mtxt = callback_uq_mtxt;
    init.callbackp_uq_ok = callback_uq_ok;
    init.callbackp_uq_file = callback_uq_file;
    init.callbackp_zmkdir = callback_zmkdir;
    init.callbackp_ckmakxmsg = callback_ckmakxmsg;
    init.callbackp_whoami = callback_whoami;
    init.callbackp_GetAppData = callback_GetAppData;
    init.callbackp_GetHomePath = callback_GetHomePath;
    init.callbackp_GetHomeDrive = callback_GetHomeDrive;
    init.callbackp_ckstrncpy = callback_ckstrncpy;
    init.callbackp_debug_logging = callback_debug_logging;
    init.callbackp_get_display = callback_get_display;
    init.callbackp_parse_displayname = callback_parse_displayname;

    /* Initialise! */
    debug(F100, "Call ssh_dll_init()", NULL, 0);
    rc = dllentryp_ssh_init(&init);

    /* TODO: Check all mandatory functions are available */

    if (rc != 0) {
        debug(F111, "SSH DLL reported error", dllname, rc);
        ssh_dll_unload(TRUE);

        return -1;
    }

#ifdef CK_COLORS_24BIT
    if (colorpalette == CK_PALETTE_XTRGB || colorpalette == CK_PALETTE_XTRGB88) {
        ssh_set_environment_variable("COLORTERM", "truecolor");
    }
#endif

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

    if (dllfuncp_ssh_unload) {
        dllfuncp_ssh_unload();
    }

    ssh_subsystem_loaded = FALSE;

    dllentryp_ssh_init = NULL;

    dllfuncp_ssh_set_iparam = NULL;
    dllfuncp_ssh_get_iparam = NULL;
    dllfuncp_ssh_set_sparam = NULL;
    dllfuncp_ssh_get_sparam = NULL;
    dllfuncp_ssh_set_identity_files = NULL;
    dllfuncp_ssh_get_socket = NULL;
    dllfuncp_ssh_open = NULL;
    dllfuncp_ssh_clos = NULL;
    dllfuncp_ssh_tchk = NULL;
    dllfuncp_ssh_flui = NULL;
    dllfuncp_ssh_break = NULL;
    dllfuncp_ssh_inc = NULL;
    dllfuncp_ssh_xin = NULL;
    dllfuncp_ssh_toc = NULL;
    dllfuncp_ssh_tol = NULL;
    dllfuncp_ssh_snaws = NULL;
    dllfuncp_ssh_proto_ver = NULL;
    dllfuncp_ssh_impl_ver = NULL;
    dllfuncp_sshkey_create = NULL;
    dllfuncp_sshkey_display_fingerprint = NULL;
    dllfuncp_sshkey_display_public = NULL;
    dllfuncp_sshkey_display_public_as_ssh2 = NULL;
    dllfuncp_sshkey_change_passphrase = NULL;
    dllfuncp_ssh_fwd_remote_port = NULL;
    dllfuncp_ssh_fwd_local_port = NULL;
    dllfuncp_ssh_fwd_clear_remote_ports = NULL;
    dllfuncp_ssh_fwd_clear_local_ports = NULL;
    dllfuncp_ssh_fwd_remove_remote_port = NULL;
    dllfuncp_ssh_fwd_remove_local_port = NULL;
    dllfuncp_ssh_fwd_get_ports = NULL;
#ifdef SSHTEST
    dllfuncp_sshkey_v1_change_comment = NULL;  /* TODO */
#endif /* SSHTEST */
    /*  dllfuncp_sshkey_default_file = NULL;*/ /* TODO */
    dllfuncp_ssh_v2_rekey = NULL;  /* TODO */
    dllfuncp_ssh_agent_delete_file = NULL;    /* TODO */
    dllfuncp_ssh_agent_delete_all = NULL;  /* TODO */
    dllfuncp_ssh_agent_add_file = NULL;  /* TODO */
    dllfuncp_ssh_agent_list_identities = NULL;    /* TODO */
    dllfuncp_ssh_set_environment_variable = NULL;
    dllfuncp_ssh_clear_environment_variables = NULL;
    dllfuncp_ssh_unload = NULL;
    dllfuncp_ssh_get_set_help = NULL;
    dllfuncp_ssh_get_help = NULL;

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

            update_setssh_options();
            update_ssh_options();

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
int ssh_avail(void) {
    return ssh_subsystem_loaded;
}

/** Provides the filename of the currently loaded SSH DLL
 *
 * @return SSH DLL filename
 */
const char* ssh_dll_name(void) {
    if (ssh_avail())
        return dll_name;
    return NULL;
}

/** Provides the version of the currently loaded SSH DLL
 *
 * @return SSH DLL version
 */
const char* ssh_dll_ver(void) {
    if (dllfuncp_ssh_dll_ver)
        return dllfuncp_ssh_dll_ver();
    return NULL;
}

int ssh_set_iparam(int param, int value) {
    if (dllfuncp_ssh_set_iparam)
        return dllfuncp_ssh_set_iparam(param, value);

    /* ERROR - this function is mandatory */

    return -1;
}


int ssh_get_iparam(int param) {
    if (dllfuncp_ssh_get_iparam)
        return dllfuncp_ssh_get_iparam(param);

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
    if (dllfuncp_ssh_set_sparam)
        return dllfuncp_ssh_set_sparam(param, value);

    /* ERROR - this function is mandatory */

    return -1;
}

const char* ssh_get_sparam(int param) {
    if (dllfuncp_ssh_get_sparam)
        return dllfuncp_ssh_get_sparam(param);

    return NULL;
}

/** Set the list of identity files to use for authentication.
 * @param identity_files The list of identity file names, null terminated.
 */
int ssh_set_identity_files(const char** identity_files) {
    if (dllfuncp_ssh_set_identity_files)
        return dllfuncp_ssh_set_identity_files(identity_files);
    return -1;
}

/** Get the socket currently in use by the SSH client.
 *
 * @returns Socket for the current SSH connection, or -1 if not implemented or
 *      no active connection
 */
int ssh_get_socket(void) {
    if (dllfuncp_ssh_get_socket)
        return dllfuncp_ssh_get_socket();
    return -1;
}

/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open(void){
    if (dllfuncp_ssh_open)
        return dllfuncp_ssh_open();

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Closes any existing SSH Session.
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos(void) {
    if (dllfuncp_ssh_clos) {
        int rc = dllfuncp_ssh_close();
        if (rc < 0) ttclos(0);
        return rc;
    }

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
int ssh_tchk(void) {
    if (dllfuncp_ssh_tchk) {
        int rc = dllfuncp_ssh_tchk();
        if (rc < 0) {
            debug(F111, "ssh_tchk returning error. Calling ttclos()", "rc", rc);
            ttclos(0);
        }
        return rc;
    }

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui(void) {
    if (dllfuncp_ssh_flui)
        return dllfuncp_ssh_flui();

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
int ssh_break(void) {
   if (dllfuncp_ssh_break)
        return dllfuncp_ssh_break();

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
    if (dllfuncp_ssh_inc) {
        int rc = dllfuncp_ssh_inc(timeout);
        if (rc < -1) ttclos(0);
        return rc;
    }

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
    if (dllfuncp_ssh_xin) {
        int rc = dllfuncp_ssh_xin(count, buffer);
        if (rc < -1) ttclos(0);
        return rc;
    }

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
    if (dllfuncp_ssh_toc) {
        int rc = dllfuncp_ssh_toc(c);
        if (rc < -1) ttclos(0);
        return rc;
    }

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
    if (dllfuncp_ssh_tol) {
        int rc = dllfuncp_ssh_tol(buffer, count);
        if (rc < -1) ttclos(0);
        return rc;
    }

    /* ERROR - this function is mandatory. This should never happen.
     * Mark SSH as unavailable.*/
    ssh_subsystem_loaded = FALSE;

    return -1;
}

/** Negotiate About Window Size. Let the remote host know the window dimensions
 * and terminal type if these have changed.
 *
 */
int ssh_snaws(void) {
    if (dllfuncp_ssh_snaws)
        return dllfuncp_ssh_snaws();

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
    if (dllfuncp_ssh_fwd_remote_port)
        return dllfuncp_ssh_fwd_remote_port(address, port, host, host_port, apply);

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
    if (dllfuncp_ssh_fwd_local_port)
        return dllfuncp_ssh_fwd_local_port(address, port, host, host_port, apply);

    /* optional feature not available */

    return -1;
}

/** Clears all remote port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all remote ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_remote_ports(BOOL apply) {
    if (dllfuncp_ssh_fwd_clear_remote_ports)
        return dllfuncp_ssh_fwd_clear_remote_ports(apply);

    /* optional feature not available */

    return -1;
}

/** Clears all local port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all  ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_local_ports(BOOL apply) {
    if (dllfuncp_ssh_fwd_clear_local_ports)
        return dllfuncp_ssh_fwd_clear_local_ports(apply);

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
        if (dllfuncp_ssh_fwd_remove_remote_port)
        return dllfuncp_ssh_fwd_remove_remote_port(port, apply);

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
    if (dllfuncp_ssh_fwd_remove_local_port)
        return dllfuncp_ssh_fwd_remove_local_port(port, apply);

    /* optional feature not available */

    return -1;
}

/** Gets all forwarded ports. The final entry in the list has a type of
 * SSH_PORT_FORWARD_NULL.
 *
 * @returns List of forwarded ports, or NULL on error or empty list
 */
const ssh_port_forward_t* ssh_fwd_get_ports(void) {
    if (dllfuncp_ssh_fwd_get_ports)
        return dllfuncp_ssh_fwd_get_ports();

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
    if (dllfuncp_sshkey_create)
        return dllfuncp_sshkey_create(filename, bits, pp, type, cmd_comment);

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
    if (dllfuncp_sshkey_display_fingerprint)
        return dllfuncp_sshkey_display_fingerprint(filename, babble);

    /* optional feature not available */

    return -1;
}

/** Outputs the public key for the specified private key - same as what you'd
 * put in authorized_keys.
 * @param filename key file - will be prompted if not specified
 * @param passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public(char * filename, char *identity_passphrase) {
    if (dllfuncp_sshkey_display_public)
        return dllfuncp_sshkey_display_public(filename, identity_passphrase);

    /* optional feature not available */

    return -1;
}

/** Like sshkey_display_public but puts some extra cruft around the output.
 *
 * @param filename key file - will be prompted if not specified
 * @param identity_passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    if (dllfuncp_sshkey_display_public_as_ssh2)
        return dllfuncp_sshkey_display_public_as_ssh2(filename, identity_passphrase);

    /* optional feature not available */

    return -1;
}

int sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    if (dllfuncp_sshkey_change_passphrase)
        return dllfuncp_sshkey_change_passphrase(filename, oldpp, newpp);

    /* optional feature not available */

    return -1;
}

#ifdef SSHTEST
int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    if (dllfuncp_sshkey_v1_change_comment)
        return dllfuncp_sshkey_v1_change_comment(filename, comment, pp);

    /* optional feature not available */

    return -1;
}
#endif /* SSHTEST */

#ifdef COMMENT
char * sshkey_default_file(int a) {
    return NULL; /* TODO */
}
#endif

void ssh_v2_rekey(void) {
    if (dllfuncp_ssh_v2_rekey)
        dllfuncp_ssh_v2_rekey();

    /* optional feature not available */
}

/** Return the current protocol version.
 *
 * @return Current protocol version (eg, "SSH-2.0")
 */
const char * ssh_proto_ver(void) {
    if (dllfuncp_ssh_proto_ver)
        return dllfuncp_ssh_proto_ver();

    return NULL;
}

/** Return the current SSH backend/implementation version.
 * @return SSH backend/implementation version
 */
const char * ssh_impl_ver(void) {
    if (dllfuncp_ssh_impl_ver)
        return dllfuncp_ssh_impl_ver();

    return NULL;
}

int ssh_agent_delete_file(const char *filename) {
    if (dllfuncp_ssh_agent_delete_file)
        return dllfuncp_ssh_agent_delete_file(filename);

    /* optional feature not available */

    return -1;
}

int ssh_agent_delete_all(void) {
    if (dllfuncp_ssh_agent_delete_all)
        return dllfuncp_ssh_agent_delete_all();

    /* optional feature not available */

    return -1;
}

int ssh_agent_add_file (const char *filename) {
    if (dllfuncp_ssh_agent_add_file)
        return dllfuncp_ssh_agent_add_file(filename);

    /* optional feature not available */

    return -1;
}

int ssh_agent_list_identities(int do_fp) {
    if (dllfuncp_ssh_agent_list_identities)
        return dllfuncp_ssh_agent_list_identities(do_fp);

    /* optional feature not available */

    return -1;
}

/** Attempts to send an environment variable to the remote host on connect
 */
int ssh_set_environment_variable(const char *name, const char *value) {
    if (dllfuncp_ssh_set_environment_variable)
        return dllfuncp_ssh_set_environment_variable(name, value);

    return -1;
}

/** Clears any environment variables previously set with
 * ssh_set_environment_variable so they are not set on the next connection
 */
int ssh_clear_environment_variables() {
    if (dllfuncp_ssh_clear_environment_variables)
        return dllfuncp_ssh_clear_environment_variables();
}

/** Gets the specified keyword table.
 *
 * @param keytab_id keyword table to get
 * @return the requested keyword table
 */
ktab_ret ssh_get_keytab(int keytab_id) {
    if (dllfuncp_ssh_get_keytab)
        return dllfuncp_ssh_get_keytab(keytab_id);

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
    if (dllfuncp_ssh_feature_supported)
        return dllfuncp_ssh_feature_supported(feature_id);
    return FALSE; /* No features supported! */
}

const char** ssh_get_set_help(void) {
    const char** result;
    static const char *hmxyssh[] = {
"No help content for SET SSH was provided by the currently loaded SSH backend.",
""
};
    if (dllfuncp_ssh_get_set_help)
        result = dllfuncp_ssh_get_set_help();

    if (result != NULL)
        return result;

    return hmxyssh;
}

const char** ssh_get_help(void) {
    const char** result;
    static const char *hmxyssh[] = {
"No help content for SSH was provided by the currently loaded SSH backend.",
""
};
    if (dllfuncp_ssh_get_help)
        result = dllfuncp_ssh_get_help();

    if (result != NULL)
        return result;

    return hmxyssh;
}


#endif /* SSH_DLL */
