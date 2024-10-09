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
* xxx_dummy = NULL;

#ifdef SSH_DLL
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

/** Called by Kermit 95 when the DLL is loaded. This should make
 * the DLL ready for use by storing copies of all the needed
 * utility functions supplied by Kermit 95, and supplying to
 * Kermit 95 via the p_install_funcs all of the SSH functions
 * this DLL provides.
 * @param params SSH initialisation parameters from Kermit 95
 */
int ssh_dll_init(ssh_init_parameters_t *params) {
    p_get_current_terminal_dimensions = params->p_get_current_terminal_dimensions;
    p_get_current_terminal_type = params->p_get_current_terminal_type;
    p_ssh_get_uid = params->p_ssh_get_uid;
    p_ssh_get_pw = params->p_ssh_get_pw;
    p_ssh_get_nodelay_enabled = params->p_ssh_get_nodelay_enabled;
    p_dodebug = params->p_dodebug;
    p_vscrnprintf = params->p_vscrnprintf;
    p_uq_txt = params->p_uq_txt;
    p_uq_mtxt = params->p_uq_mtxt;
    p_uq_ok = params->p_uq_ok;
    p_uq_file = params->p_uq_file;
    p_zmkdir = params->p_zmkdir;
    p_ckmakxmsg = params->p_ckmakxmsg;
    p_whoami = params->p_whoami;
    p_GetAppData = params->p_GetAppData;
    p_GetHomePath = params->p_GetHomePath;
    p_GetHomeDrive = params->p_GetHomeDrive;
    p_ckstrncpy = params->p_ckstrncpy;
    p_debug_logging = params->p_debug_logging;
    p_get_display = params->p_get_display;
    p_parse_displayname = params->p_parse_displayname;

    params->p_install_funcs("ssh_set_iparam", ssh_set_iparam);
    params->p_install_funcs("ssh_get_iparam", ssh_get_iparam);
    params->p_install_funcs("ssh_set_sparam", ssh_set_sparam);
    params->p_install_funcs("ssh_get_sparam", ssh_get_sparam);
    params->p_install_funcs("ssh_set_identity_files", ssh_set_identity_files);
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
#ifdef COMMENT
    /* These functions are all optional */
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
    params->p_install_funcs("sshkey_v1_change_comment", sshkey_v1_change_comment);
#endif
    /* params->p_install_funcs("sshkey_default_file", sshkey_default_file); */
    params->p_install_funcs("ssh_v2_rekey", ssh_v2_rekey);
    params->p_install_funcs("ssh_agent_delete_file", ssh_agent_delete_file);
    params->p_install_funcs("ssh_agent_delete_all", ssh_agent_delete_all);
    params->p_install_funcs("ssh_agent_add_file", ssh_agent_add_file);
    params->p_install_funcs("ssh_agent_list_identities", ssh_agent_list_identities);
    params->p_install_funcs("ssh_unload", ssh_unload);
#endif /* COMMENT */
    params->p_install_funcs("ssh_dll_ver", ssh_dll_ver);
    params->p_install_funcs("ssh_get_keytab", ssh_get_keytab);
    params->p_install_funcs("ssh_feature_supported", ssh_feature_supported);

    return 0;
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
#else
/* These live in ckossh.c */
unsigned char* get_display();
int parse_displayname(char *displayname, int *familyp, char **hostp,
                      int *dpynump, int *scrnump, char **restp);
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

/** This is the equivalent of ssh_dll_init - when the SSH module is
 * compiled into the K95 executable (SSH_DLL not defined), this is
 * called on application startup to give the SSH subsystem an
 * opportunity to set sensible defaults, etc.
 */
void ssh_initialise() {

}

/** Opens an SSH connection. Connection parameters are passed through global
 * variables
 *
 * @return An error code (0 = success)
 */
int ssh_open(){
    return 0;
}

/** Closes any existing SSH Session.
 *
 * @return  0 on success, < 0 on failure.
 */
int ssh_clos() {
    return 0;
}

/** Network I/O Check. This function is polled by K95 to get the status
 * of the connection and find out how many bytes are waiting to be
 * processed. It is called frequently so must return quickly.
 *
 * @return >= 0 indicates number of bytes waiting to be read
 *          < 0 indicates a fatal error and the connection should be closed.
 */
int ssh_tchk() {
    return 0;
}

/** Flush input
 *
 * @return 0 on success, < 0 on error
 */
int ssh_flui() {
    return 0;
}

/** Network Break - send a break signal to the remote process.
 * This may or may not do anything. Supported on SSH V2 only.
 *
 * @return
 */
int ssh_break() {
    return 0;
}

/** Input Character. Reads one character from the input queue
 *
 * @param timeout 0 for none (block until there is a character to read),
 *                positive for seconds, negative for milliseconds
 * @return -1 for timeout, >= 0 is a valid character, < -1 is a fatal error
 */
int ssh_inc(int timeout) {
    return 0;
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
    return 0;
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
int ssh_snaws() {
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
const ssh_port_forward_t* ssh_fwd_get_ports() {
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
#endif

/** Manually re-key the SSH connection
 */
void ssh_v2_rekey() {

}

/** Return the current protocol version.
 *
 * @return Current protocol version (eg, "SSH-2.0")
 */
const char * ssh_proto_ver() {
    return NULL;
}

/** Return the current SSH implementation version, which may be different from
 * the SSH backend version (ssh_dll_ver())
 *
 * @return SSH implementation version
 */
const char * ssh_impl_ver() {
    return "Null SSH";
}

int ssh_agent_delete_file(const char *filename) {
    return 0;
}

int ssh_agent_delete_all() {
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
void ssh_unload() {

}

/** Returns version information for this SSH backend DLL which may be different
 * from the SSH Implementation version (ssh_impl_ver()).
 *
 * @return
 */
const char * ssh_dll_ver() {
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
        case SSH_FEAT_OPENSSH_CONF:
        case SSH_FEAT_KEY_MGMT:
        case SSH_FEAT_REKEY_AUTO:
        case SSH_FEAT_SSH_V1:
        case SSH_FEAT_PROXY_CMD:
        case SSH_FEAT_ADV_KERBEROS4:
        case SSH_FEAT_ADV_KERBEROS5:
        case SSH_FEAT_REKEY_MANUAL:
        case SSH_FEAT_FROM_PRIV_PRT:
        case SSH_FEAT_GSSAPI_KEYEX:
        case SSH_FEAT_PORT_FWD:
        case SSH_FEAT_X11_FWD:
        case SSH_FEAT_AGENT_FWD:
        case SSH_FEAT_GSSAPI_DELEGAT:
        case SSH_FEAT_AGENT_MGMT:
        default:
            return TRUE;
    }
}
