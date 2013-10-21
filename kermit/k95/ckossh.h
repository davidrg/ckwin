/*  C K O S S H   --  Kermit interface to OpenSSH Header */

/*
  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com),
           Secure Endpoints Inc., New York City

  COPYRIGHT NOTICE:

  Copyright (C) 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

#ifdef SSHBUILTIN
extern int                              /* SET SSH variables */
  ssh_afw,                              /* agent forwarding */
  ssh_xfw,                              /* x-11 forwarding */
  ssh_prp,                              /* use privileged-port? */
  ssh_cmp,                              /* use compression? */
  ssh_cas,                              /* command-as-subsystem? */
  ssh_shh,                              /* quiet? */
  ssh_ver,                              /* Version: 0 auto, 1, 2 */
  ssh_vrb,                              /* Verbose */
  ssh_chkip,                            /* SSH Check Host IP flag */
  ssh_gwp,                              /* gateway ports */
  ssh_dyf,                              /* dynamic forwarding */
  ssh_gsd,                              /* gssapi delegate credentials */
  ssh_k4tgt,                            /* k4 tgt passing */
  ssh_k5tgt,                            /* k5 tgt passing */
  ssh_shk,                              /* Strict host key */
  ssh2_ark,                             /* Auto re-key */
  ssh_cas,                              /* command as subsys */
  ssh_cfg,                              /* use OpenSSH config */
  ssh_gkx,                              /* gssapi key exchange */
  ssh_k5_is_k4,                         /* some OpenSSH use same codes */
  ssh_hbt,                              /* heartbeat */
  ssh_dummy;                            /* bottom of list */

char                                    /* The following are to be malloc'd */
  * ssh1_cif,                           /* v1 cipher */
  * ssh2_cif,                           /* v2 cipher list */
  * ssh2_mac,                           /* v2 mac list */
  * ssh2_auth,                          /* v2 authentication list */
  * ssh2_hka,                           /* v2 Host Key Algorithm list */
  * ssh_hst,                            /* hostname */
  * ssh_prt,                            /* port/service */
  * ssh_cmd,                            /* command to execute */
  * ssh_xal,                            /* xauth-location */
  * ssh1_gnh,                           /* v1 global known hosts file */
  * ssh1_unh,                           /* v1 user known hosts file */
  * ssh2_gnh,                           /* v2 global known hosts file */
  * ssh2_unh,                           /* v2 user known hosts file */
  * xxx_dummy;

extern char * ssh_idf[32];              /* identity files */
extern int ssh_idf_n;

extern int    ssh_pf_lcl_n,
              ssh_pf_rmt_n;
extern struct ssh_pf ssh_pf_lcl[32];    /* Port forwarding structs */
extern struct ssh_pf ssh_pf_rmt[32];    /* (declared in ckuusr.c) */

extern int ssh_sock;                    /* SSH socket */

_PROTOTYP(int ssh_open,(VOID));
_PROTOTYP(int ssh_clos,(VOID));
_PROTOTYP(int ssh_tchk,(VOID));
_PROTOTYP(int ssh_flui,(VOID));
_PROTOTYP(int ssh_break,(VOID));
_PROTOTYP(int ssh_inc,(int));
_PROTOTYP(int ssh_xin,(int,char *));
_PROTOTYP(int ssh_toc,(int));
_PROTOTYP(int ssh_tol,(char *,int));
_PROTOTYP(VOID ssh_terminfo,(char *,int, int));
_PROTOTYP(CONST char * ssh_version,(VOID));
_PROTOTYP(CONST char * ssh_errorstr,(int));
_PROTOTYP(int ssh_ttvt,(VOID));
_PROTOTYP(int ssh_ttpkt,(VOID));
_PROTOTYP(int ssh_ttres,(VOID));
_PROTOTYP(VOID ssh_usage,(VOID));
_PROTOTYP(int sshkey_create,(char * filename, int bits, char * pp,
                   int type, char * cmd_comment));
_PROTOTYP(int sshkey_display_fingerprint,(char * filename, int babble));
_PROTOTYP(int sshkey_display_public,(char * filename, char *identity_passphrase));
_PROTOTYP(int sshkey_display_public_as_ssh2,(char * filename,char *identity_passphrase));
_PROTOTYP(int sshkey_change_passphrase,(char * filename, char * oldpp, char * newpp));
_PROTOTYP(int sshkey_v1_change_comment,(char * filename, char * comment, char * pp));
_PROTOTYP(char * sshkey_default_file,(int));
_PROTOTYP(int ssh_fwd_local_port,(int,char *,int));
_PROTOTYP(int ssh_few_remote_port,(int,char *,int));
_PROTOTYP(void ssh_v2_rekey,(void));
_PROTOTYP(char * ssh_proto_ver,(void));

_PROTOTYP(int ssh_agent_delete_file,(const char *filename));
_PROTOTYP(int ssh_agent_delete_all, (void));
_PROTOTYP(int ssh_agent_add_file, (const char *filename));
_PROTOTYP(int ssh_agent_list_identities,(int do_fp));
#endif /* SSHBUILTIN */


