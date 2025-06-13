/*
 *  C K O N S S H . C --  Null Subsystem Interface for Kermit 95
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
 * This is a "null" SSH Subsystem for Kermit 95. It implements the bare
 * minimum to be loadable by Kermit 95, but it can't make SSH connections.
 *
 * It exists to serve as an example of Kermit 95s SSH Subsystem DLL
 * interface, and also to allow for performing some basic testing against
 * Kermit 95 without getting a full SSH implementation involved.
 *
 * Note that SSH Subsystems should *not* assume they're being loaded into
 * a graphical application. Kermit 95 is available in both GUI and text mode
 * (console) variants. The supplied helper functions uq_txt, uq_mtxt, uq_ok
 * and uq_file support both GUI and text-mode interfaces for getting user
 * input.
 */

#include "ckcdeb.h"
#include "ckossh.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef NT
#include <windows.h>
#else
#define INCL_DOSPROCESS
#include <os2.h>
#undef COMMENT
#endif

/*
 * Keyword tables for some of the "set ssh" options - these are the tables whose
 * values are most likely to vary from one SSH implementation to another.
 *
 * These used to live in ckuus3.c
 */


#define SSHA_CRS   1
#define SSHA_DSA   2
#define SSHA_GSS   3
#define SSHA_HOS   4
#define SSHA_KBD   5
#define SSHA_K4    6
#define SSHA_K5    7
#define SSHA_PSW   8
#define SSHA_PK    9
#define SSHA_SKE  10
#define SSHA_TIS  11
#define SSHA_EXT  12
#define SSHA_SRP  13

static struct keytab ssh2aut[] = {      /* SET SSH V2 AUTH command table */
        { "external-keyx",      SSHA_EXT, 0 },
        { "gssapi",             SSHA_GSS, 0 },
        { "hostbased",          SSHA_HOS, 0 },
        { "keyboard-interactive",  SSHA_KBD, 0 },
        { "password",           SSHA_PSW, 0 },
        { "publickey",          SSHA_PK,  0 },
        { "srp-gex-sha1",       SSHA_SRP, 0 },
        { "", 0, 0 }
};
static int nssh2aut = (sizeof(ssh2aut) / sizeof(struct keytab)) - 1;

#define SSHC_3DES 1                     /* 3DES */
#define SSHC_3CBC 2                     /* 3DES-CBC */
#define SSHC_A128 3                     /* AES128-CBC */
#define SSHC_A192 4                     /* AES192-CBC */
#define SSHC_A256 5                     /* AES256-CBC */
#define SSHC_ARC4 6                     /* ARCFOUR */
#define SSHC_FISH 7                     /* BLOWFISH */
#define SSHC_C128 8                     /* CAST128-CBC */
#define SSHC_BCBC 9                     /* BLOWFISH-CBC */
#define SSHC_1DES 10                    /* DES */
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
        { "arcfour",         SSHC_ARC4, 0 },
        { "blowfish-cbc",    SSHC_FISH, 0 },
        { "cast128-cbc",     SSHC_C128, 0 },
        { "rijndael128-cbc", SSHC_A128, 0 },
        { "rijndael192-cbc", SSHC_A192, 0 },
        { "rijndael256-cbc", SSHC_A256, 0 },
        { "aes128-ctr", SSHC_A12C, 0 },
        { "aes192-ctr", SSHC_A19C, 0 },
        { "aes256-ctr", SSHC_A25C, 0 },
        { "aes128-gcm@openssh.com", SSHC_A1GC, 0 },
        { "aes256-gcm@openssh.com", SSHC_A2GC, 0 },
        { "chachae20-poly1305", SSHC_CHPO, 0 },
        { "", 0, 0 }
};
static int nssh2ciphers = (sizeof(ssh2ciphers) / sizeof(struct keytab)) - 1;

static struct keytab ssh1ciphers[] = {
    { "3des",         SSHC_3DES, 0 },
    { "blowfish",     SSHC_FISH, 0 },
    { "des",          SSHC_1DES, 0 },
    { "", 0, 0 }
};
static int nssh1ciphers = (sizeof(ssh1ciphers) / sizeof(struct keytab)) - 1;

#define SSHM_SHA        1               /* HMAC-SHA1 */
#define SSHM_SHA_96     2               /* HMAC-SHA1-96 */
#define SSHM_MD5        3               /* HMAC-MD5 */
#define SSHM_MD5_96     4               /* HMAC-MD5-96 */
#define SSHM_RIPE       5               /* HMAC-RIPEMD160 */
#define SSHM_SHA1_ETM   6               /* hmac-sha1-etm@openssh.com */
#define SSHM_SHA2_256   7               /* hmac-sha2-256 */
#define SSHM_SHA2_2ETM  8               /* hmac-sha2-256-etm@openssh.com */
#define SSHM_SHA2_512   9               /* hmac-sha2-512 */
#define SSHM_SHA2_5ETM  10              /* hmac-sha2-512-etm@openssh.com */
#define SSHM_NONE       11              /* none */

static struct keytab ssh2macs[] = {     /* SET SSH V2 MACS command table */
        { "hmac-md5",       SSHM_MD5,    0 },
        { "hmac-md5-96",    SSHM_MD5_96, 0 },
        { "hmac-ripemd160", SSHM_RIPE,   0 },
        { "hmac-sha1",      SSHM_SHA,    0 },
        { "hmac-sha1-96",   SSHM_SHA_96, 0 },
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
        { "ext-info-c",                     13, 0, },
        { "", 0, 0 }
};
static int nsshkextab = (sizeof(sshkextab) / sizeof(struct keytab)) - 1;

#define SET_OFF  0

/* Integer parameters accessed via ssh_set_iparam and ssh_get_iparam */
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

/* String parameters accessed via ssh_set_sparam and ssh_get_sparam */
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

#ifdef SSH_DLL

static get_current_terminal_dimensions_callback *callbackp_get_current_terminal_dimensions = NULL;
static get_current_terminal_type_callback   *callbackp_get_current_terminal_type = NULL;
static ssh_get_uid_callback                 *callbackp_ssh_get_uid = NULL;
static ssh_get_pw_callback                  *callbackp_ssh_get_pw = NULL;
static ssh_get_nodelay_enabled_callback     *callbackp_ssh_get_nodelay_enabled = NULL;
static ssh_open_socket_callback             *callbackp_ssh_open_socket = NULL;
static dodebug_callback                     *callbackp_dodebug = NULL;
static scrnprint_callback                   *callbackp_scrnprint = NULL;
static uq_txt_callback                      *callbackp_uq_txt = NULL;
static uq_mtxt_callback                     *callbackp_uq_mtxt = NULL;
static uq_ok_callback                       *callbackp_uq_ok = NULL;
static uq_file_callback                     *callbackp_uq_file = NULL;
static zmkdir_callback                      *callbackp_zmkdir = NULL;
static ckmakxmsg_callback                   *callbackp_ckmakxmsg = NULL;
static whoami_callback                      *callbackp_whoami = NULL;
static GetAppData_callback                  *callbackp_GetAppData = NULL;
static GetHomePath_callback                 *callbackp_GetHomePath = NULL;
static GetHomeDrive_callback                *callbackp_GetHomeDrive = NULL;
static ckstrncpy_callback                   *callbackp_ckstrncpy = NULL;
static debug_logging_callback               *callbackp_debug_logging = NULL;
static get_display_callback                 *callbackp_get_display = NULL;
static parse_displayname_callback           *callbackp_parse_displayname = NULL;

void get_current_terminal_dimensions(int* rows, int* cols) {
    callbackp_get_current_terminal_dimensions(rows, cols);
}

const char* get_current_terminal_type(void) {
    return callbackp_get_current_terminal_type();
}

const char* ssh_get_uid(void) {
    return callbackp_ssh_get_uid();
}

const char* ssh_get_pw(void) {
    return callbackp_ssh_get_pw();
}

int ssh_get_nodelay_enabled(void) {
    return callbackp_ssh_get_nodelay_enabled();
}

SOCKET ssh_open_socket(char* host, char* port) {
    return callbackp_ssh_open_socket(host, port);
}

int dodebug(int flag, char * s1, char * s2, CK_OFF_T n)
{
    if ( callbackp_dodebug )
        return(callbackp_dodebug(flag,s1,s2,n));
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

    if ( callbackp_scrnprint )
        return(callbackp_scrnprint(myprtfstr));
    else
        return(-1);
}

int uq_txt(char * preface, char * prompt, int echo, char ** help, char * buf,
       int buflen, char *dflt, int timer) {
    return callbackp_uq_txt(preface, prompt, echo, help, buf, buflen, dflt, timer);
}

int uq_mtxt(char * preface,char **help, int n, struct txtbox field[]) {
    return callbackp_uq_mtxt(preface, help, n, field);
}

int uq_ok(char * preface, char * prompt, int mask,char ** help, int dflt) {
    return callbackp_uq_ok(preface, prompt, mask, help, dflt);
}

int uq_file(char * preface, char * fprompt, int fc, char ** help,
	char * dflt, char * result, int rlength) {
    return callbackp_uq_file(preface, fprompt, fc, help, dflt, result, rlength);
}

int zmkdir(char *path) {
    return callbackp_zmkdir(path);
}

int ckmakxmsg(char * buf, int len, char *s1, char *s2, char *s3,
        char *s4, char *s5, char *s6, char *s7, char *s8, char *s9,
        char *s10, char *s11, char *s12) {
    return callbackp_ckmakxmsg(buf, len, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
                       s12);
}

char* whoami(void) {
    return callbackp_whoami();
}

char* GetAppData(int common) {
    return callbackp_GetAppData(common);
}

char* GetHomePath(void) {
    return callbackp_GetHomePath();
}

char* GetHomeDrive(void) {
    return callbackp_GetHomeDrive();
}

int ckstrncpy(char * dest, const char * src, int len) {
    return callbackp_ckstrncpy(dest, src, len);
}

int debug_logging(void) {
    return callbackp_debug_logging();
}

unsigned char* get_display(void) {
    return callbackp_get_display();
}

int parse_displayname(char *displayname, int *familyp, char **hostp,
                        int *dpynump, int *scrnump, char **restp) {
    return callbackp_parse_displayname(displayname, familyp, hostp,
                               dpynump, scrnump, restp);
}

#ifdef SSH_DLL_CALLCONV

/* define prototypes for DLL functions */
static ssh_set_iparam_dllfunc               dllfunc_ssh_set_iparam;
static ssh_get_iparam_dllfunc               dllfunc_ssh_get_iparam;
static ssh_set_sparam_dllfunc               dllfunc_ssh_set_sparam;
static ssh_get_sparam_dllfunc               dllfunc_ssh_get_sparam;
static ssh_set_identity_files_dllfunc       dllfunc_ssh_set_identity_files;
static ssh_get_socket_dllfunc               dllfunc_ssh_get_socket;
static ssh_open_dllfunc                     dllfunc_ssh_open;
static ssh_clos_dllfunc                     dllfunc_ssh_clos;
static ssh_tchk_dllfunc                     dllfunc_ssh_tchk;
static ssh_flui_dllfunc                     dllfunc_ssh_flui;
static ssh_break_dllfunc                    dllfunc_ssh_break;
static ssh_inc_dllfunc                      dllfunc_ssh_inc;
static ssh_xin_dllfunc                      dllfunc_ssh_xin;
static ssh_toc_dllfunc                      dllfunc_ssh_toc;
static ssh_tol_dllfunc                      dllfunc_ssh_tol;
static ssh_snaws_dllfunc                    dllfunc_ssh_snaws;
static ssh_proto_ver_dllfunc                dllfunc_ssh_proto_ver;
static ssh_impl_ver_dllfunc                 dllfunc_ssh_impl_ver;
static sshkey_create_dllfunc                dllfunc_sshkey_create;
static sshkey_display_fingerprint_dllfunc   dllfunc_sshkey_display_fingerprint;
static sshkey_display_public_dllfunc        dllfunc_sshkey_display_public;
static sshkey_display_public_as_ssh2_dllfunc dllfunc_sshkey_display_public_as_ssh2;
static sshkey_change_passphrase_dllfunc     dllfunc_sshkey_change_passphrase;
static ssh_fwd_remote_port_dllfunc          dllfunc_ssh_fwd_remote_port;
static ssh_fwd_local_port_dllfunc           dllfunc_ssh_fwd_local_port;
static ssh_fwd_clear_remote_ports_dllfunc   dllfunc_ssh_fwd_clear_remote_ports;
static ssh_fwd_clear_local_ports_dllfunc    dllfunc_ssh_fwd_clear_local_ports;
static ssh_fwd_remove_remote_port_dllfunc   dllfunc_ssh_fwd_remove_remote_port;
static ssh_fwd_remove_local_port_dllfunc    dllfunc_ssh_fwd_remove_local_port;
static ssh_fwd_get_ports_dllfunc            dllfunc_ssh_fwd_get_ports;
static sshkey_v1_change_comment_dllfunc     dllfunc_sshkey_v1_change_comment;
static sshkey_default_file_dllfunc          dllfunc_sshkey_default_file;
static ssh_v2_rekey_dllfunc                 dllfunc_ssh_v2_rekey;
static ssh_agent_delete_file_dllfunc        dllfunc_ssh_agent_delete_file;
static ssh_agent_delete_all_dllfunc         dllfunc_ssh_agent_delete_all;
static ssh_agent_add_file_dllfunc           dllfunc_ssh_agent_add_file;
static ssh_agent_list_identities_dllfunc    dllfunc_ssh_agent_list_identities;
static ssh_unload_dllfunc                   dllfunc_ssh_unload;
static ssh_dll_ver_dllfunc                  dllfunc_ssh_dll_ver;
static ssh_get_keytab_dllfunc               dllfunc_ssh_get_keytab;
static ssh_feature_supported_dllfunc        dllfunc_ssh_feature_supported;
static ssh_get_set_help_dllfunc             dllfunc_ssh_get_set_help;
static ssh_get_help_dllfunc                 dllfunc_ssh_get_help;


/* calling convention layer for DLL functions on DLL side */
static int CKSSHAPI dllfunc_ssh_set_iparam(int param, int value) {
    return ssh_set_iparam(param, value);
}

static int CKSSHAPI dllfunc_ssh_get_iparam(int param) {
    return ssh_get_iparam(param);
}

static int CKSSHAPI dllfunc_ssh_set_sparam(int param, const char* value) {
    return ssh_set_sparam(param, value);
}

static const char* CKSSHAPI dllfunc_ssh_get_sparam(int param) {
    return ssh_get_sparam(param);
}

static int CKSSHAPI dllfunc_ssh_set_identity_files(const char** identity_files) {
    return ssh_set_identity_files(identity_files);
}

static int CKSSHAPI dllfunc_ssh_get_socket(void) {
    return ssh_get_socket();
}

static int CKSSHAPI dllfunc_ssh_open(void) {
    return ssh_open();
}

static int CKSSHAPI dllfunc_ssh_clos(void) {
    return ssh_clos();
}

static int CKSSHAPI dllfunc_ssh_tchk(void) {
    return ssh_tchk();
}

static int CKSSHAPI dllfunc_ssh_flui(void) {
    return ssh_flui();
}

static int CKSSHAPI dllfunc_ssh_break(void) {
    return ssh_break();
}

static int CKSSHAPI dllfunc_ssh_inc(int timeout) {
    return ssh_inc(timeout);
}

static int CKSSHAPI dllfunc_ssh_xin(int count, char * buffer) {
    return ssh_xin(count, buffer);
}

static int CKSSHAPI dllfunc_ssh_toc(int c) {
    return ssh_toc(c);
}

static int CKSSHAPI dllfunc_ssh_tol(char * buffer, int count) {
    return ssh_tol(buffer, count);
}

static int CKSSHAPI dllfunc_ssh_snaws(void) {
    return ssh_snaws();
}

static const char * CKSSHAPI dllfunc_ssh_proto_ver(void) {
    return ssh_proto_ver();
}

static const char * CKSSHAPI dllfunc_ssh_impl_ver(void) {
    return ssh_impl_ver();
}

#ifdef COMMENT
static int CKSSHAPI dllfunc_sshkey_create(char * filename, int bits, char * pp,
                             int type, char * cmd_comment) {
    return sshkey_create(filename, bits, pp, type, cmd_comment);
}

static int CKSSHAPI dllfunc_sshkey_display_fingerprint(char * filename, int babble) {
    return sshkey_display_fingerprint(filename, babble);
}

static int CKSSHAPI dllfunc_sshkey_display_public(char * filename, char *identity_passphrase) {
    return sshkey_display_public(filename, identity_passphrase);
}

static int CKSSHAPI dllfunc_sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    return sshkey_display_public_as_ssh2(filename, identity_passphrase);
}

static int CKSSHAPI dllfunc_sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    return sshkey_change_passphrase(filename, oldpp, newpp);
}

static int CKSSHAPI dllfunc_ssh_fwd_remote_port(char* address, int port, char * host, int host_port, BOOL apply) {
    return ssh_fwd_remote_port(address, port, host, host_port, apply);
}

static int CKSSHAPI dllfunc_ssh_fwd_local_port(char* address, int port,char * host, int host_port, BOOL apply) {
    return ssh_fwd_local_port(address, port, host, host_port, apply);
}

static int CKSSHAPI dllfunc_ssh_fwd_clear_remote_ports(BOOL apply) {
    return ssh_fwd_clear_remote_ports(apply);
}

static int CKSSHAPI dllfunc_ssh_fwd_clear_local_ports(BOOL apply) {
    return ssh_fwd_clear_local_ports(apply);
}

static int CKSSHAPI dllfunc_ssh_fwd_remove_remote_port(int port, BOOL apply) {
    return ssh_fwd_remove_remote_port(port, apply);
}

static int CKSSHAPI dllfunc_ssh_fwd_remove_local_port(int port, BOOL apply) {
    return ssh_fwd_remove_local_port(port, apply);
}

static const ssh_port_forward_t* CKSSHAPI dllfunc_ssh_fwd_get_ports(void) {
    return ssh_fwd_get_ports();
}

#ifdef SSHTEST
static int CKSSHAPI dllfunc_sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    return sshkey_v1_change_comment(filename, comment, pp);
}
#endif /* SSHTEST */

#ifdef COMMENT
static char * CKSSHAPI dllfunc_sshkey_default_file(int a) {
    return sshkey_default_file(a);
}
#endif /* COMMENT */

static void CKSSHAPI dllfunc_ssh_v2_rekey(void) {
    ssh_v2_rekey();
}

static int CKSSHAPI dllfunc_ssh_agent_delete_file(const char *filename) {
    return ssh_agent_delete_file(filename);
}

static int CKSSHAPI dllfunc_ssh_agent_delete_all(void) {
    return ssh_agent_delete_all();
}

static int CKSSHAPI dllfunc_ssh_agent_add_file(const char *filename) {
    return ssh_agent_add_file(filename);
}

static int CKSSHAPI dllfunc_ssh_agent_list_identities(int do_fp) {
    return ssh_agent_list_identities(do_fp);
}

static void CKSSHAPI dllfunc_ssh_unload(void) {
    ssh_unload();
}
#endif /* COMMENT */

static const char * CKSSHAPI dllfunc_ssh_dll_ver(void) {
    return ssh_dll_ver();
}

static ktab_ret CKSSHAPI dllfunc_ssh_get_keytab(int keytab_id) {
    return ssh_get_keytab(keytab_id);
}

static int CKSSHAPI dllfunc_ssh_feature_supported(int feature_id) {
    return ssh_feature_supported(feature_id);
}

static const char ** CKSSHAPI dllfunc_ssh_get_set_help(void) {
    return ssh_get_set_help();
}

static const char ** CKSSHAPI dllfunc_ssh_get_help(void) {
    return ssh_get_help();
}

#else /* SSH_DLL_CALLCONV */

/* directly use DLL functions without calling convention layer */
#define dllfunc_ssh_set_iparam              ssh_set_iparam
#define dllfunc_ssh_get_iparam              ssh_get_iparam
#define dllfunc_ssh_set_sparam              ssh_set_sparam
#define dllfunc_ssh_get_sparam              ssh_get_sparam
#define dllfunc_ssh_set_identity_files      ssh_set_identity_files
#define dllfunc_ssh_get_socket              ssh_get_socket
#define dllfunc_ssh_open                    ssh_open
#define dllfunc_ssh_clos                    ssh_clos
#define dllfunc_ssh_tchk                    ssh_tchk
#define dllfunc_ssh_flui                    ssh_flui
#define dllfunc_ssh_break                   ssh_break
#define dllfunc_ssh_inc                     ssh_inc
#define dllfunc_ssh_xin                     ssh_xin
#define dllfunc_ssh_toc                     ssh_toc
#define dllfunc_ssh_tol                     ssh_tol
#define dllfunc_ssh_snaws                   ssh_snaws
#define dllfunc_ssh_proto_ver               ssh_proto_ver
#define dllfunc_ssh_impl_ver                ssh_impl_ver
#ifdef COMMENT
#define dllfunc_sshkey_create               sshkey_create
#define dllfunc_sshkey_display_fingerprint  sshkey_display_fingerprint
#define dllfunc_sshkey_display_public       sshkey_display_public
#define dllfunc_sshkey_display_public_as_ssh2 sshkey_display_public_as_ssh2
#define dllfunc_sshkey_change_passphrase    sshkey_change_passphrase
#define dllfunc_ssh_fwd_remote_port         ssh_fwd_remote_port
#define dllfunc_ssh_fwd_local_port          ssh_fwd_local_port
#define dllfunc_ssh_fwd_clear_remote_ports  ssh_fwd_clear_remote_ports
#define dllfunc_ssh_fwd_clear_local_ports   ssh_fwd_clear_local_ports
#define dllfunc_ssh_fwd_remove_remote_port  ssh_fwd_remove_remote_port
#define dllfunc_ssh_fwd_remove_local_port   ssh_fwd_remove_local_port
#define dllfunc_ssh_fwd_get_ports           ssh_fwd_get_ports
#ifdef SSHTEST
#define dllfunc_sshkey_v1_change_comment    sshkey_v1_change_comment
#endif /* SSHTEST */
#ifdef COMMENT
#define dllfunc_sshkey_default_file         sshkey_default_file
#endif /* COMMENT */
#define dllfunc_ssh_v2_rekey                ssh_v2_rekey
#define dllfunc_ssh_agent_delete_file       ssh_agent_delete_file
#define dllfunc_ssh_agent_delete_all        ssh_agent_delete_all
#define dllfunc_ssh_agent_add_file          ssh_agent_add_file
#define dllfunc_ssh_agent_list_identities   ssh_agent_list_identities
#define dllfunc_ssh_unload                  ssh_unload
#endif /* COMMENT */
#define dllfunc_ssh_dll_ver                 ssh_dll_ver
#define dllfunc_ssh_get_keytab              ssh_get_keytab
#define dllfunc_ssh_feature_supported       ssh_feature_supported
#define dllfunc_ssh_get_set_help            ssh_get_set_help
#define dllfunc_ssh_get_help                ssh_get_help

#endif /* SSH_DLL_CALLCONV */

/*
 * Quick macro to check if a function pointer is null and, if it is, log
 * the event and return an error.
 */
#define CHECK_FP(fp)

/** Called by Kermit 95 when the DLL is loaded. This should make
 * the DLL ready for use by storing copies of all the needed
 * callback functions supplied by Kermit 95, and supplying to
 * Kermit 95 via the install_dllfunc all of the SSH functions
 * this DLL provides.
 * @param params SSH initialisation parameters from Kermit 95
 */
int CKSSHDLLENTRY ssh_dll_init(ssh_dll_init_data *init) {
    /* Store pointers to helper functions provided by K95 */
    callbackp_get_current_terminal_dimensions = init->callbackp_get_current_terminal_dimensions;
    CHECK_FP(callbackp_get_current_terminal_dimensions)
    callbackp_get_current_terminal_type = init->callbackp_get_current_terminal_type;
    CHECK_FP(callbackp_get_current_terminal_type)
    callbackp_ssh_get_uid = init->callbackp_ssh_get_uid;
    CHECK_FP(callbackp_ssh_get_uid)
    callbackp_ssh_get_pw = init->callbackp_ssh_get_pw;
    CHECK_FP(callbackp_ssh_get_pw)
    callbackp_ssh_get_nodelay_enabled = init->callbackp_ssh_get_nodelay_enabled;
    CHECK_FP(callbackp_ssh_get_nodelay_enabled)
    callbackp_ssh_open_socket = init->callbackp_ssh_open_socket;
    CHECK_FP(callbackp_ssh_open_socket)
    callbackp_dodebug = init->callbackp_dodebug;
    CHECK_FP(callbackp_dodebug)
    callbackp_scrnprint = init->callbackp_scrnprint;
    CHECK_FP(callbackp_scrnprint)
    callbackp_uq_txt = init->callbackp_uq_txt;
    CHECK_FP(callbackp_uq_txt)
    callbackp_uq_mtxt = init->callbackp_uq_mtxt;
    CHECK_FP(callbackp_uq_mtxt)
    callbackp_uq_ok = init->callbackp_uq_ok;
    CHECK_FP(callbackp_uq_ok)
    callbackp_uq_file = init->callbackp_uq_file;
    CHECK_FP(callbackp_uq_file)
    callbackp_zmkdir = init->callbackp_zmkdir;
    CHECK_FP(callbackp_zmkdir)
    callbackp_ckmakxmsg = init->callbackp_ckmakxmsg;
    CHECK_FP(callbackp_ckmakxmsg)
    callbackp_whoami = init->callbackp_whoami;
    CHECK_FP(callbackp_whoami)
    callbackp_GetAppData = init->callbackp_GetAppData;
    CHECK_FP(callbackp_GetAppData)
    callbackp_GetHomePath = init->callbackp_GetHomePath;
    CHECK_FP(callbackp_GetHomePath)
    callbackp_GetHomeDrive = init->callbackp_GetHomeDrive;
    CHECK_FP(callbackp_GetHomeDrive)
    callbackp_ckstrncpy = init->callbackp_ckstrncpy;
    CHECK_FP(callbackp_ckstrncpy)
    callbackp_debug_logging = init->callbackp_debug_logging;
    CHECK_FP(callbackp_debug_logging)
    callbackp_get_display = init->callbackp_get_display;
    CHECK_FP(callbackp_get_display)
    callbackp_parse_displayname = init->callbackp_parse_displayname;
    CHECK_FP(callbackp_parse_displayname)

    init->callbackp_install_dllfunc("ssh_set_iparam", dllfunc_ssh_set_iparam);
    init->callbackp_install_dllfunc("ssh_get_iparam", dllfunc_ssh_get_iparam);
    init->callbackp_install_dllfunc("ssh_set_sparam", dllfunc_ssh_set_sparam);
    init->callbackp_install_dllfunc("ssh_get_sparam", dllfunc_ssh_get_sparam);
    init->callbackp_install_dllfunc("ssh_set_identity_files", dllfunc_ssh_set_identity_files);
    init->callbackp_install_dllfunc("ssh_get_socket", dllfunc_ssh_get_socket);
    init->callbackp_install_dllfunc("ssh_open", dllfunc_ssh_open);
    init->callbackp_install_dllfunc("ssh_clos", dllfunc_ssh_clos);
    init->callbackp_install_dllfunc("ssh_tchk", dllfunc_ssh_tchk);
    init->callbackp_install_dllfunc("ssh_flui", dllfunc_ssh_flui);
    init->callbackp_install_dllfunc("ssh_break", dllfunc_ssh_break);
    init->callbackp_install_dllfunc("ssh_inc", dllfunc_ssh_inc);
    init->callbackp_install_dllfunc("ssh_xin", dllfunc_ssh_xin);
    init->callbackp_install_dllfunc("ssh_toc", dllfunc_ssh_toc);
    init->callbackp_install_dllfunc("ssh_tol", dllfunc_ssh_tol);
    init->callbackp_install_dllfunc("ssh_snaws", dllfunc_ssh_snaws);
    init->callbackp_install_dllfunc("ssh_proto_ver", dllfunc_ssh_proto_ver);
    init->callbackp_install_dllfunc("ssh_impl_ver", dllfunc_ssh_impl_ver);

    /* These functions are all optional */
#ifdef COMMENT
    init->callbackp_install_dllfunc("sshkey_create", dllfunc_sshkey_create);
    init->callbackp_install_dllfunc("sshkey_display_fingerprint", dllfunc_sshkey_display_fingerprint);
    init->callbackp_install_dllfunc("sshkey_display_public", dllfunc_sshkey_display_public);
    init->callbackp_install_dllfunc("sshkey_display_public_as_ssh2", dllfunc_sshkey_display_public_as_ssh2);
    init->callbackp_install_dllfunc("sshkey_change_passphrase", dllfunc_sshkey_change_passphrase);
    init->callbackp_install_dllfunc("ssh_fwd_remote_port", dllfunc_ssh_fwd_remote_port);
    init->callbackp_install_dllfunc("ssh_fwd_local_port", dllfunc_ssh_fwd_local_port);
    init->callbackp_install_dllfunc("ssh_fwd_clear_remote_ports", dllfunc_ssh_fwd_clear_remote_ports);
    init->callbackp_install_dllfunc("ssh_fwd_clear_local_ports", dllfunc_ssh_fwd_clear_local_ports);
    init->callbackp_install_dllfunc("ssh_fwd_remove_remote_port", dllfunc_ssh_fwd_remove_remote_port);
    init->callbackp_install_dllfunc("ssh_fwd_remove_local_port", dllfunc_ssh_fwd_remove_local_port);
    init->callbackp_install_dllfunc("ssh_fwd_get_ports", dllfunc_ssh_fwd_get_ports);
#ifdef SSHTEST
    init->callbackp_install_dllfunc("sshkey_v1_change_comment", dllfunc_sshkey_v1_change_comment); /* TODO */
#endif /* SSHTEST */
#ifdef COMMENT
    init->callbackp_install_dllfunc("sshkey_default_file", dllfunc_sshkey_default_file); */ /* TODO */
#endif /* COMMENT */
    init->callbackp_install_dllfunc("ssh_v2_rekey", dllfunc_ssh_v2_rekey); /* TODO */
    init->callbackp_install_dllfunc("ssh_agent_delete_file", dllfunc_ssh_agent_delete_file); /* TODO */
    init->callbackp_install_dllfunc("ssh_agent_delete_all", dllfunc_ssh_agent_delete_all); /* TODO */
    init->callbackp_install_dllfunc("ssh_agent_add_file", dllfunc_ssh_agent_add_file); /* TODO */
    init->callbackp_install_dllfunc("ssh_agent_list_identities", dllfunc_ssh_agent_list_identities); /* TODO */
#ifdef COMMENT
    /* Not supported: */
    init->callbackp_install_dllfunc("ssh_unload", dllfunc_ssh_unload);
#endif /* COMMENT */
#endif /* COMMENT */
    init->callbackp_install_dllfunc("ssh_dll_ver", dllfunc_ssh_dll_ver);
    init->callbackp_install_dllfunc("ssh_get_keytab", dllfunc_ssh_get_keytab);
    init->callbackp_install_dllfunc("ssh_feature_supported", dllfunc_ssh_feature_supported);
    init->callbackp_install_dllfunc("ssh_get_set_help", dllfunc_ssh_get_set_help);
    init->callbackp_install_dllfunc("ssh_get_help", dllfunc_ssh_get_help);

    return 0;
}

#undef malloc
#undef realloc
#undef free
#undef strdup

void
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

#endif /* SSH_DLL */

/** Sets an integer parameter
 *
 * @param param Parameter to set
 * @param value New value
 * @return
 */
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

/** Gets an integer parameter
 *
 * @param param The parameter to get (SSH_IPARAM_)
 * @return Integer parameter value
 */
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

/** Utility function for copying a string to a destination or freeing
 * the destination if the source is null
 *
 * @param dest destination
 * @param src new value
 */
static void copy_set_sparam(char **dest, const char* src) {
    if (*dest) {
        free(*dest);
        *dest = NULL;
    }
    if (src) {
        *dest = _strdup(src);
    }
}

/** Set a string parameter. Takes a copy of the supplied value.
 *
 * @param param Parameter to set (SSH_SPARAM_)
 * @param value New value for the parameter
 * @return
 */
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

/** Get String parameter
 *
 * @param param Parameter to get (SSH_SPARAM_)
 * @return The string parameter
 */
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
    return -1;
}

/** Get the socket currently in use by the SSH client.
 *
 * @returns Socket for the current SSH connection, or -1 if not implemented or
 *      no active connection
 */
int ssh_get_socket(void) {

    /* If there is an active SSH session, this function should return its
     * socket. It's used by the various "set tcp" commands to set socket
     * options.
     *
     * If there is no active connection, or the SSH client can't supply
     * the socket its using, return -1
     */

    return -1;
}

/** This is the equivalent of ssh_dll_init - when the SSH module is
 * compiled into the K95 executable (SSH_DLL not defined), this is
 * called on application startup to give the SSH subsystem an
 * opportunity to set sensible defaults, etc.
 */
void ssh_initialise(void) {

}

/* This a message we'll output to Kermit 95 if the user tries to make a
 * connection using this null SSH backend
 */
static char* message =
"This is the null SSH backend. It serves two purposes:\r\n"
"  1. To allow testing Kermit 95s ability to load SSH DLLs in the absence\r\n"
"     of a real SSH backend DLL to test against (such as on Windows or OS/2\r\n"
"     versions where no backend is available)\r\n"
"  2. To provide a template for the development of alternative SSH \r\n"
"     backends for Kermit 95\r\n"
"\n\n"
"Aside from producing this message, the null SSH backend does nothing.\r\n"
"You may now return to the command screen using the exit sequence (Alt+x\r\n"
"by default).\0";

/* How long the message is, and what the next character to return to K95 is */
static int message_length = 0, message_position = 0;

/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open(void){
    /* Reset the message to the start */
    message_length = strlen(message);
    message_position = 0;

    /*
     * If Kermit 95 is configured with proxy server details
     * ("set tcp http-proxy" or "set tcp socks-proxy"), then Kermit 95 can
     * open a connection through that proxy server and hand that connection
     * over ready to make an SSH connection through.
     *
     * To do this, just call ssh_open_socket(). If you get a socket back,
     * you're free to go ahead and use it. If you get INVALID_SOCKET back
     * then you'll need to make the connection yourself.
     *
     * If ssh_open_socket() returns a valid socket then you are responsible
     * for closing it when the SSH connection is closed (or fails to open).
     * Kermit 95 doesn't track it and won't close it for you.
     */

    return 0;
}

/** Closes any existing SSH Session.
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos(void) {
    return 0;
}

/** Network I/O Check. This function is polled by K95 to get the status
 * of the connection and find out how many bytes are waiting to be
 * processed. It is called frequently so must return quickly.
 *
 * @return >= 0 indicates number of bytes waiting to be read
 *          < 0 indicates a fatal error and the connection should be closed.
 */
int ssh_tchk(void) {
    /* Respond with how many characters are left in the message */
    return message_length - message_position;
}

/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui(void) {
    return 0;
}

/** Network Break - send a break signal to the remote process.
 * This may or may not do anything. Supported on SSH V2 only.
 *
 * @return
 */
int ssh_break(void) {
    return 0;
}

/** Input Character. Reads one character from the input queue
 *
 * @param timeout 0 for none (block until there is a character to read),
 *                positive for seconds, negative for milliseconds
 * @return -1 for timeout, >= 0 is a valid character, < -1 is a fatal error
 */
int ssh_inc(int timeout) {
    int result;

    if (message_position >= message_length) {
        /* We're at the end of the message and there is nothing more to
         * return. If K95 wants us to block until another byte arrives
         * (which it will never do), just sleep for a while to not waste
         * CPU */
        int tmo = timeout;
        if (tmo == 0) tmo = 1000;
#ifdef NT
        Sleep(1000);
#else
        DosSleep(1000);
#endif
        return -1;
    };

    /* Return the next character in the message */
    result = message[message_position];
    message_position++;
    return result;
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
    int length = message_length - message_position;

    if (count < length) length = count;

    /* We're at the end of the message - nothing more to return */
    if (length <= 0) return 0;

    /* Return the requested portion of the message */
    memcpy(buffer, message, length);
    message_position += length;

    return length;
}

/** Terminal Output Character - send a single character. Blocks until the
 * character has been handled.
 *
 * @param c character to send
 * @return 0 for success, <0 for error
 */
int ssh_toc(int c) {
    return 0;
}

/** Terminal Output Line - send multiple characters.
 *
 * @param buffer characters to be sent
 * @param count number of characters to be sent
 * @return  >= 0 for number of characters sent, <0 for a fatal error.
 */
int ssh_tol(char * buffer, int count) {
    return 0;
}

/** Negotiate About Window Size. Let the remote host know the window dimensions
 * and terminal type if these have changed.
 *
 */
int ssh_snaws(void) {
    return 0;
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
    return 0;
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
    return 0;
}

/** Clears all remote port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all remote ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_remote_ports(BOOL apply) {

    return 0;
}

/** Clears all local port forwards for future SSH sessions
 *
 * @param apply Also stop forwarding all  ports in any active SSH session
 * @returns 0 on success
 */
int ssh_fwd_clear_local_ports(BOOL apply) {

    return 0;
}

/** Remove a single reverse/remote port forward.
 *
 * @param port Reverse port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_remote_port(int port, BOOL apply);

/** Remove a single direct/local port forward.
 *
 * @param port Direct port forward to remove
 * @param apply Also remove the port forward from any active session. Does not
 *      close any established connections.
 * @return 0 on success
 */
int ssh_fwd_remove_local_port(int port, BOOL apply);

/** Gets all forwarded ports. The final entry in the list has a type of
 * SSH_PORT_FORWARD_NULL.
 *
 * @returns List of forwarded ports, or NULL on error or empty list
 */
const ssh_port_forward_t* ssh_fwd_get_ports(void) {
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
    return 0;
}

/** Displays the fingerprint for the specified public key
 *
 * @param filename Key file to display the fingerprint for. If not supplied, one
 *                 will be prompted for
 * @param babble 0 - fingerprint, 1 - IETF (ignored)
 * @return 0
 */
int sshkey_display_fingerprint(char * filename, int babble) {
    return 0;
}

/** Outputs the public key for the specified private key - same as what you'd
 * put in authorized_keys.
 * @param filename key file - will be prompted if not specified
 * @param passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public(char * filename, char *identity_passphrase) {
    return 0;
}

/** Like sshkey_display_public but puts some extra cruft around the output.
 *
 * @param filename key file - will be prompted if not specified
 * @param identity_passphrase key passphrase - will be prompted if not specified
 */
int sshkey_display_public_as_ssh2(char * filename,char *identity_passphrase) {
    return 0;
}

int sshkey_change_passphrase(char * filename, char * oldpp, char * newpp) {
    return 0;
}

#ifdef SSHTEST
int sshkey_v1_change_comment(char * filename, char * comment, char * pp) {
    return 0;
}
#endif /* SSHTEST */

#ifdef COMMENT
char * sshkey_default_file(int a) {
    return NULL; /* TODO */
}
#endif /* COMMENT */

/** Manually re-key the SSH connection
 */
void ssh_v2_rekey(void) {

}

/** Return the current protocol version.
 *
 * @return Current protocol version (eg, "SSH-2.0")
 */
const char * ssh_proto_ver(void) {
    return NULL;
}

/** Return the current SSH implementation version, which may be different from
 * the SSH backend version (ssh_dll_ver())
 *
 * @return SSH implementation version
 */
const char * ssh_impl_ver(void) {
    return "Null SSH";
}

int ssh_agent_delete_file(const char *filename) {
    return 0;
}

int ssh_agent_delete_all(void) {
    return 0;
}

int ssh_agent_add_file (const char *filename) {
    return 0;
}

int ssh_agent_list_identities(int do_fp) {
    return 0;
}

/** Called by Kermit 95 before the DLL is unloaded. This is only called if
 * ssh_dll_init was previously called. It is not called on application exit.
 *
 * The normal reason for ssh_unload() to be called is that Kermit 95 has
 * decided the DLL is unsuitable/incompatible in some way or because
 * ssh_dll_init() returned an error code.
 *
 * @return
 */
void ssh_unload(void) {

}

/** Returns version information for this SSH backend DLL which may be different
 * from the SSH Implementation version (ssh_impl_ver()).
 *
 * @return
 */
const char * ssh_dll_ver(void) {
    return "NULL SSH Subsystem (does nothing)";
}

/** Gets the specified keyword table.
 *
 * @param keytab_id keyword table to get
 * @return the requested keyword table
 */
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
        case SSH_KTAB_V1_CIPHERS: {
            ret.ktab = ssh1ciphers;
            ret.ktab_len = nssh1ciphers;
            break;
        }
        default: {
            ret.rc = -1;
        }
    }
    return ret;
}

/** Returns TRUE if the specified feature is supported by the SSH backend,
 * or FALSE if it is not. This is used to enable or disable various "ssh" and
 * "set ssh" commands and options.
 *
 * @param feature_id The feature being checked
 * @return TRUE if the feature is supported, or FALSE if it is not
 */
int ssh_feature_supported(int feature_id) {
    switch(feature_id) {
        case SSH_FEAT_OPENSSH_CONF:     /* Configuration via openssh config file */
        case SSH_FEAT_KEY_MGMT:         /* SSH key creation, etc */
        case SSH_FEAT_REKEY_AUTO:       /* Automatic rekeying options */
        case SSH_FEAT_SSH_V1:           /* SSHv1 protocol support */
        case SSH_FEAT_PROXY_CMD:        /* Proxy command */
        case SSH_FEAT_ADV_KERBEROS4:    /* Advanced Kerberos IV options */
        case SSH_FEAT_ADV_KERBEROS5:    /* Advanced Kerberos V options */
        case SSH_FEAT_REKEY_MANUAL:     /* Manual rekeying */
        case SSH_FEAT_FROM_PRIV_PRT:    /* Connect from private port */
        case SSH_FEAT_GSSAPI_KEYEX:     /* GSSAPI Key Exchange */
        case SSH_FEAT_PORT_FWD:         /* Local and remote port forwarding */
        case SSH_FEAT_X11_FWD:          /* X11 forwarding */
        case SSH_FEAT_AGENT_FWD:        /* SSH Agent Forwarding */
        case SSH_FEAT_GSSAPI_DELEGAT:   /* GSSAPI Delegation */
        case SSH_FEAT_AGENT_MGMT:       /* SSH Agent management */
        case SSH_FEAT_DYN_PORT_FWD:     /* Dynamic port forwarding */
        default:
            return TRUE;
    }
}

/** This function should return the text for "HELP SET SSH". Any commands not
 * supported should be excluded.
 *
 * @return Help text for HELP SET SSH.
 */
const char** ssh_get_set_help(void) {

    /*
     * TODO: Remove help text for any commands reported as not supported by
     *  ssh_feature_supported(), and adjust the parameter lists to match
     *  those returned by ssh_get_keytab()
     */

    static const char *hmxyssh[] = {
/* Feature: SSH_FEAT_AGENT_FWD */
"SET SSH AGENT-FORWARDING { ON, OFF }",
"  If an authentication agent is in use, setting this value to ON",
"  results in the connection to the agent being forwarded to the remote",
"  computer.  The default is OFF.",
" ",
"SET SSH CHECK-HOST-IP { ON, OFF }",
"  Specifies whether the remote host's ip-address should be checked",
"  against the matching host key in the known_hosts file.  This can be",
"  used to determine if the host key changed as a result of DNS spoofing.",
"  The default is ON.",
" ",
"SET SSH COMPRESSION { ON, OFF }",
"  Specifies whether compression will be used.  The default is ON.",
" ",
"SET SSH DIRECTORY directory",
"  Specifies where Kermit 95 should look for the default SSH user files",
"  such as the user-known-hosts file and identity files (id_rsa, etc).",
"  By default Kermit 95 looks for these in \\v(appdata)ssh.",
" ",
/* Feature: SSH_FEAT_DYN_PORT_FWD */
"SET SSH DYNAMIC-FORWARDING { ON, OFF }",
"  Specifies whether Kermit is to act as a SOCKS4 service on port 1080",
"  when connected to a remote host via SSH.  When Kermit acts as a SOCKS4",
"  service, it accepts connection requests and forwards the connections",
"  through the remote host.  The default is OFF.",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SET SSH GATEWAY-PORTS { ON, OFF }",
"  Specifies whether Kermit should act as a gateway for forwarded",
"  connections received from the remote host.  The default is OFF.",
" ",
/* Feature: SSH_FEAT_GSSAPI_DELEGAT */
"SET SSH GSSAPI DELEGATE-CREDENTIALS { ON, OFF }",
"  Specifies whether Kermit should delegate GSSAPI credentials to ",
"  the remote host after authentication.  Delegating credentials allows",
"  the credentials to be used from the remote host.  The default is OFF.",
" ",
"SET SSH HEARTBEAT-INTERVAL <seconds>",
"  Specifies a number of seconds of idle time after which an IGNORE",
"  message will be sent to the server.  This pulse is useful for",
"  maintaining connections through HTTP Proxy servers and Network",
"  Address Translators.  The default is OFF (0 seconds).",
" ",
"SET SSH IDENTITY-FILE filename [ filename [ ... ] ]",
"  Specifies one or more files from which the user's authorization",
"  identities (private keys) are to be read when using public key",
"  authorization.  These are files used in addition to the default files:",
" ",
"    \\v(appdata)ssh/identity      V1 RSA",     /* Feature: SSH_FEAT_SSH_V1 */
"    \\v(appdata)ssh/id_rsa        V2 RSA",
"    \\v(appdata)ssh/id_dsa        V2 DSA",
"    \\v(appdata)ssh/id_ecdsa      ECDSA",
"    \\v(appdata)ssh/id_ed25519    ED25519",
" ",
/* Feature: SSH_FEAT_ADV_KERBEROS4 */
"SET SSH KERBEROS4 TGT-PASSING { ON, OFF }",
"  Specifies whether Kermit should forward Kerberos 4 TGTs to the host.",
"  The default is OFF.",
" ",
/* Feature: SSH_FEAT_ADV_KERBEROS5 */
"SET SSH KERBEROS5 TGT-PASSING { ON, OFF }",
"  Specifies whether Kermit should forward Kerberos 5 TGTs to to the",
"  host.  The default is OFF.",
" ",
/* Feature: SSH_FEAT_FROM_PRIV_PRT */
"SET SSH PRIVILEGED-PORT { ON, OFF }",
"  Specifies whether a privileged port (less than 1024) should be used",
"  when connecting to the host.  Privileged ports are not required except",
"  when using SSH V1 with Rhosts or RhostsRSA authorization.  The default",
"  is OFF.",
" ",
/* Feature: SSH_FEAT_PROXY_CMD */
"SET SSH PROXY-COMMAND [ command ]",
"  Specifies the command to be executed in order to connect to the remote",
"  host. ",
" ",
"SET SSH QUIET { ON, OFF }",
"  Specifies whether all messages generated in conjunction with SSH",
"  protocols should be suppressed.  The default is OFF.",
" ",
"SET SSH STRICT-HOST-KEY-CHECK { ASK, ON, OFF }",
"  Specifies how Kermit should behave if the the host key check fails.",
"  When strict host key checking is OFF, the new host key is added to the",
"  protocol-version-specific user-known-hosts-file.  When strict host key",
"  checking is ON, the new host key is refused and the connection is",
"  dropped.  When set to ASK, Kermit prompt you to say whether the new",
"  host key should be accepted.  The default is ASK.",
" ",
"  Strict host key checking protects you against Trojan horse attacks.",
"  It depends on you to maintain the contents of the known-hosts-file",
"  with current and trusted host keys.",
" ",
/* Feature: SSH_FEAT_OPENSSH_CONF */
"SET SSH USE-OPENSSH-CONFIG { ON, OFF }",
"  Specifies whether Kermit should parse an OpenSSH configuration file",
"  after applying Kermit's SET SSH commands.  The configuration file",
"  would be located at \\v(home)ssh/ssh_config.  The default is OFF.",
" ",
/* Feature: SSH_FEAT_SSH_V1 */
"SET SSH V1 CIPHER { 3DES, BLOWFISH, DES }",
"  Specifies which cipher should be used to protect SSH version 1",
"  connections.  The default is 3DES.",
" ",
/* Feature: SSH_FEAT_SSH_V1 */
"SET SSH V1 GLOBAL-KNOWN-HOSTS-FILE filename",
"  Specifies the location of the system-wide known-hosts file.  The",
"  default is:",
" ",
"    \v(common)ssh_known_hosts",
" ",
/* Feature: SSH_FEAT_SSH_V1 */
"SET SSH V1 USER-KNOWN-HOSTS-FILE filename",
"  Specifies the location of the user-known-hosts-file.  The default",
"  location is:",
" ",
"    \\v(appdata)ssh/known_hosts",
" ",
"SET SSH V2 AUTHENTICATION { GSSAPI, KEYBOARD-INTERACTIVE, PASSWORD, ",
"    PUBKEY, NONE } [ ... ]",
"  Specifies an ordered list of SSH version 2 authentication methods to",
"  be used when connecting to the remote host. The SSH client requires ",
"  none to be attempted first, so the default list is:",
" ",
"    none gssapi publickey keyboard-interactive password",
" ",
/* Feature: SSH_FEAT_REKEY_AUTO */
"SET SSH V2 AUTO-REKEY { ON, OFF }",
"  Specifies whether Kermit automatically issues rekeying requests",
"  once an hour when SSH version 2 in in use.  The default is ON.",
" ",
"SET SSH V2 CIPHERS { 3DES-CBC, AES128-CBC, AES192-CBC, AES256-CBC, ",
"     AES128-CTR, AES192-CTR, AES256-CTR, AES128-GCM@OPENSSH.COM, ",
"     AES256-GCM@OPENSSH.COM, CHACHAE20-POLY1305 }",
"  Specifies an ordered list of SSH version ciphers to be used to encrypt",
"  the established connection.  The default list is:",
" ",
"    aes256-gcm@openssh.com aes128-gcm@openssh.com aes256-ctr aes192-ctr",
"    aes128-ctr aes256-cbc aes192-cbc aes128-cbc 3des-cbc",
" ",
"SET SSH V2 GLOBAL-KNOWN-HOSTS-FILE filename",
"  Specifies the location of the system-wide known-hosts file.  The default",
"  location is:",
" ",
"    \\v(common)ssh/known_hosts2",
" ",
"SET SSH V2 HOSTKEY-ALGORITHMS { ECDSA-SHA2-NISTP256, ECDSA-SHA2-NISTP384, ",
"     ECDSA-SHA2-NISTP521, RSA-SHA2-256, RSA-SHA2-512, SSH-DSS, SSH-ED25519, ",
"     SSH-RSA }",
"  Specifies an ordered list of hostkey algorithms to be used to verify",
"  the identity of the host.  The default list is",
" ",
"    ssh-ed25519 ecdsa-sha2-nistp521 ecdsa-sha2-nistp384 ecdsa-sha2-nistp256",
"    rsa-sha2-512 rsa-sha2-256 ssh-rsa",
" ",
"SET SSH V2 KEY-EXCHANGE-METHODS { CURVE25519-SHA256, ",
"     CURVE25519-SHA256@LIBSSH.ORG, DIFFIE-HELLMAN-GROUP1-SHA1, ",
"     DIFFIE-HELLMAN-GROUP14-SHA1, DIFFIE-HELLMAN-GROUP14-SHA256, ",
"     DIFFIE-HELLMAN-GROUP16-SHA512, DIFFIE-HELLMAN-GROUP18-SHA512, ",
"     DIFFIE-HELLMAN-GROUP-EXCHANGE-SHA1, ",
"     DIFFIE-HELLMAN-GROUP-EXCHANGE-SHA256, ECDH-SHA2-NISTP256, ",
"     ECDH-SHA2-NISTP384, ECDH-SHA2-NISTP521 }",
"  Specifies an ordered list of Key Exchange Methods to be used to generate ",
"  per-connection keys. The default list is:",
" ",
"    curve25519-sha256 curve25519-sha256@libssh.org ecdh-sha2-nistp256 ",
"    ecdh-sha2-nistp384 ecdh-sha2-nistp521 diffie-hellman-group18-sha512",
"    diffie-hellman-group16-sha512 diffie-hellman-group-exchange-sha256",
"    diffie-hellman-group14-sha256 diffie-hellman-group14-sha1 ",
"    diffie-hellman-group1-sha1 ext-info-c",
" ",
"SET SSH V2 MACS { HMAC-SHA1, HMAC-SHA1-ETM@OPENSSH.COM, HMAC-SHA2-256, ",
"     HMAC-SHA2-256-ETM@OPENSSH.COM, HMAC-SHA2-512, ",
"     HMAC-SHA2-512-ETM@OPENSSH.COM, NONE }",
"  Specifies an ordered list of Message Authentication Code algorithms to",
"  be used for integrity  protection of the established connection.  The",
"  default list is:",
" ",
"    hmac-sha2-256-etm@openssh.com hmac-sha2-512-etm@openssh.com ",
"    hmac-sha1-etm@openssh.com hmac-sha2-256 hmac-sha2-512 hmac-sha1",
" ",
"SET SSH V2 USER-KNOWN-HOSTS-FILE filename",
"  Specifies the location of the user-known-hosts file.  The default",
"  location is:",
" ",
"    \\v(appdata)ssh/known_hosts2",
" ",
"SET SSH VERBOSE level",
"  Specifies how many messages should be generated by the OpenSSH engine.",
"  The level can range from 0 to 7.  The default value is 2.",
" ",
/* Feature: SSH_FEAT_SSH_V1 */
"SET SSH VERSION { 1, 2, AUTOMATIC }",
"   Specifies which SSH version should be negotiated.  The default is",
"   AUTOMATIC which means use version 2 if supported; otherwise to fall",
"   back to version 1.",
" ",
/* Feature: SSH_FEAT_X11_FWD */
"SET SSH X11-FORWARDING { ON, OFF }",
"  Specifies whether X Windows System Data is to be forwarded across the",
"  established SSH connection.  The default is OFF.  When ON, the DISPLAY",
"  value is set using the SET TELNET ENV DISPLAY command.",
" ",
/* Feature: SSH_FEAT_X11_XAUTH */
"SET SSH XAUTH-LOCATION filename",
"  Specifies the location of the xauth executable (if provided with the",
"  X11 Server software.)",
/* Last line of help text must be an empty string to terminate */
""
    };

    return hmxyssh;
}

/** This function should return the text for "HELP SSH". Any commands not
 * supported should be excluded.
 *
 * @return Help text for HELP SSH.
 */
const char** ssh_get_help(void) {

    /*
     * TODO: Remove help text for any commands reported as not supported by
     *  ssh_feature_supported(), and adjust the parameter lists to match
     *  those returned by ssh_get_keytab()
     */

    static const char * hmxxssh[] = {
"Syntax: SSH { ADD, AGENT, CLEAR, KEY, [ OPEN ], V2 } operands...",
"  Performs an SSH-related action, depending on the keyword that follows:",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH ADD LOCAL-PORT-FORWARD local-port host port",
"  Adds a port forwarding triplet to the local port forwarding list.",
"  The triplet specifies a local port to be forwarded and the hostname /",
"  ip-address and port number to which the port should be forwarded from",
"  the remote host.  Port forwarding is activated at connection",
"  establishment and continues until the connection is terminated.",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH ADD REMOTE-PORT-FORWARD remote-port host port",
"  Adds a port forwarding triplet to the remote port forwarding list.",
"  The triplet specifies a remote port to be forwarded and the",
"  hostname/ip-address and port number to which the port should be",
"  forwarded from the local machine.  Port forwarding is activated at",
"  connection establishment and continues until the connection is",
"  terminated.",
" ",
/* Feature: SSH_FEAT_AGENT_MGMT */
"SSH AGENT ADD [ identity-file ]",
"  Adds the contents of the identity-file (if any) to the SSH AGENT",
"  private key cache.  If no identity-file is specified, all files",
"  specified with SET SSH IDENTITY-FILE are added to the cache.",
" ",
/* Feature: SSH_FEAT_AGENT_MGMT */
"SSH AGENT DELETE [ identity-file ]",
"  Deletes the contents of the identity-file (if any) from the SSH AGENT",
"  private key cache.  If no identity-file is specified, all files",
"  specified with SET SSH IDENTITY-FILE are deleted from the cache.",
" ",
/* Feature: SSH_FEAT_AGENT_MGMT */
"SSH AGENT LIST [ /FINGERPRINT ]",
"  Lists the contents of the SSH AGENT private key cache.  If /FINGERPRINT",
"  is specified, the fingerprint of the private keys are displayed instead",
"  of the keys.",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH CLEAR LOCAL-PORT-FORWARD",
"  Clears the local port forwarding list.",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH CLEAR REMOTE-PORT-FORWARD",
"  Clears the remote port forwarding list.",
" ",
/* Feature: SSH_FEAT_KEY_MGMT */
"SSH KEY commands:",
"  The SSH KEY commands create and manage public and private key pairs",
"  (identities).  There are four forms of SSH keys.  Each key pair is",
"  stored in its own set of files:",
" ",
"   Key Type      Private Key File           Public Key File",
"    RSA keys      \\v(home).ssh/id_rsa       \\v(home).ssh/id_rsa.pub",
"    DSA keys      \\v(home).ssh/id_dsa       \\v(home).ssh/id_dsa.pub",
"    ECDSA keys    \\v(home).ssh/id_ecdsa     \\v(home).ssh/id_ecdsa.pub",
"    ED25519 keys  \\v(home).ssh/id_ed25519   \\v(home).ssh/id_ed25519.pub",
" ",
"  Keys are stored using the OpenSSH keyfile format.  The private key",
"  files can be (optionally) protected by specifying a passphrase.  A",
"  passphrase is a longer version of a password.  English text provides",
"  no more than 2 bits of key data per character.  56-bit keys can be",
"  broken by a brute force attack in approximately 24 hours.  When used,",
"  private key files should therefore be protected by a passphrase of at",
"  least 40 characters (about 80 bits).",
" ",
"  To install a public key file on the host, you must transfer the file",
"  to the host and append it to your \"authorized_keys\" file.  The file",
"  permissions must be 600 (or equivalent).",
" ",
/* Feature: SSH_FEAT_KEY_MGMT */
"SSH KEY CHANGE-PASSPHRASE [ /NEW-PASSPHRASE:passphrase",
"      /OLD-PASSPHRASE:passphrase ] filename",
"  This re-encrypts the specified private key file with a new passphrase.",
"  The old passphrase is required.  If the passphrases (and filename) are",
"  not provided Kermit prompts your for them.",
" ",
/* Feature: SSH_FEAT_KEY_MGMT */
"SSH KEY CREATE [ /BITS:bits /PASSPHRASE:passphrase",
"    /TYPE:{ DSS, ECDSA, ED25519, RSA } ] filename",
"  This command creates a new private/public key pair.  The defaults is",
"  TYPE:ED25519.  The filename is the name of the private key file.  The",
"  The public key is created with the same name with .pub appended to it.",
"  If a filename is not specified Kermit prompts you for it. Key length ",
"  options (/BITS:) depends on the key type:",
" ",
"    ECDSA: 256 (default), 384, 521",
"    RSA: 1024, 2048, 3072 (default), 4096, 8192",
"    DSS: 1024 (default), 2048",
" ",
"  ED25519 does not support being given a key length and any value supplied",
"  via /BITS: will be ignored.",
" ",
/* Feature: SSH_FEAT_KEY_MGMT */
"SSH KEY DISPLAY [ /FORMAT:{FINGERPRINT,IETF,OPENSSH,SSH.COM} ] filename",
"  This command displays the contents of a public or private key file.",
"  The default format is OPENSSH.",
" ",
/* Feature: SSH_FEAT_SSH_V1 */
"SSH KEY V1 SET-COMMENT filename comment",
"  This command replaces the comment associated with a V1 RSA key file.",
" ",
"SSH [ OPEN ] host [ port ] [ /COMMAND:command /USER:username",
"      /PASSWORD:pwd /VERSION:{ 1, 2 } /X11-FORWARDING:{ ON, OFF } ]",
"  This command establishes a new connection using SSH version 1 or",
"  version 2 protocol.  The connection is made to the specified host on",
"  the SSH port (you can override the port by including a port name or",
"  number after the host name).  Once the connection is established the",
"  authentication negotiations begin.  If the authentication is accepted,",
"  the local and remote port forwarding lists are used to establish the",
"  desired connections.  If X11 Forwarding is active, this results in a",
"  remote port forwarding between the X11 clients on the remote host and",
"  X11 Server on the local machine.  If a /COMMAND is provided, the",
"  command is executed on the remote host in place of your default shell.",
" ",
"  An example of a /COMMAND to execute C-Kermit in SERVER mode is:",
"     SSH OPEN hostname /COMMAND:{kermit -x -l 0}",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH REMOVE LOCAL-PORT-FORWARD local-port",
"  Removes the local port forward with the specified local-port from",
"  the local port forwarding list. This has no effect on any active ",
"  connection.",
" ",
/* Feature: SSH_FEAT_PORT_FWD */
"SSH REMOVE REMOTE-PORT-FORWARD remote-port",
"  Removes the remote port forward with the specified remote-port from",
"  the remote port forwarding list. This has no effect on any active ",
"  connection.",
" ",
/* Feature: SSH_FEAT_REKEY_MANUAL */
"SSH V2 REKEY",
"  Requests that an existing SSH V2 connection generate new session keys.",
/* Last line must be empty to terminate */
""
    };
    return hmxxssh;
}
