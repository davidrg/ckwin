/*
C K O A T H . H  --  Authentication for Kermit 95

Copyright (C) 1998, 2004, Trustees of Columbia University in the City of New
    York.
  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
           Secure Endpoints Inc., New York City

  Specify the function mappings you need to have applied by #define's
  set before including this file:

    MAP_DES
    MAP_SRP
    MAP_KRYPTO
    MAP_KRB4
    MAP_KRB5
    MAP_NTLM
    MAP_GSSAPI
    MAP_CRYPT
*/


#ifdef OS2
#ifdef MAP_DES
#define des_new_random_key            ck_des_new_random_key
#define des_set_random_generator_seed ck_des_set_random_generator_seed
#ifdef FTP_KRB4
#define des_key_sched                 ck_k4_des_key_sched
#else /* FTP_KRB4 */
#define des_key_sched                 ck_des_key_sched
#endif /* FTP_KRB4 */
#define des_ecb_encrypt               ck_des_ecb_encrypt
#define des_string_to_key             ck_des_string_to_key
#ifdef des_fixup_key_parity
#undef des_fixup_key_parity
#endif
#define des_fixup_key_parity          ck_des_fixup_key_parity
#define des_pcbc_encrypt              ck_des_pcbc_encrypt

int ck_des_new_random_key(Block B);
void ck_des_set_random_generator_seed(Block B);
int ck_des_key_sched(Block B, Schedule S);
int ck_k4_des_key_sched(Block B, Schedule S);
void ck_des_ecb_encrypt(Block B1, Block B2, Schedule S, int I);
int ck_des_string_to_key(char * p, Block B);
void ck_des_fixup_key_parity(Block B);
void ck_des_pcbc_encrypt(Block input, Block output, long length,
                    Schedule schedule, Block ivec, int enc);
#endif /* MAP_DES */

#ifdef MAP_SRP
#define t_clientresponse  ck_t_clientresponse
#define t_clientgetkey    ck_t_clientgetkey
#define t_clientpasswd    ck_t_clientpasswd
#define t_clientgenexp    ck_t_clientgenexp
#define t_clientopen      ck_t_clientopen
#define t_clientverify    ck_t_clientverify

unsigned char * ck_t_clientresponse(struct t_client *);
unsigned char * ck_t_clientgetkey(struct t_client *, struct t_num *);
void ck_t_clientpasswd(struct t_client *, char *);
struct t_num * ck_t_clientgenexp(struct t_client *);
struct t_client * ck_t_clientopen(const char *, struct t_num *, struct t_num *, struct t_num *);
int ck_t_clientverify(struct t_client *, unsigned char *);
#endif /* MAP_SRP */

#ifdef MAP_KRYPTO
#define cipher_getdescbyid    (struct _cipher_desc *)ck_cipher_getdescbyid
#define cipher_getlist        (unsigned char *)ck_cipher_getlist
#define cipher_getdescbyname  (struct _cipher_desc *)ck_cipher_getdescbyname
#define hash_getdescbyid      (struct _hash_desc *)ck_hash_getdescbyid
#define hash_getlist          (unsigned char *)ck_hash_getlist
#define hash_getdescbyname    (struct _hash_desc *)ck_hash_getdescbyname
#define krypto_delete         ck_krypto_delete
#define krypto_new            (struct _krypto_context *)ck_krypto_new
#define hash_supported        ck_hash_supported
#define cipher_supported      ck_cipher_supported
#define krypto_msg_safe       ck_krypto_msg_safe
#define krypto_msg_priv       ck_krypto_msg_priv
#endif /* MAP_KRYPTO */

#ifdef MAP_KRB4
#ifndef ADDR_SZ
#define ADDR_SZ 40      /* From Leash krb.h */
#endif /* ADDR_SZ */

/* The Leash implementation of Kerberos 4 used by Kermit 95 */
/* has an extended Credentials structure that includes the  */
/* ip address of the ticket in readable form.               */
struct leash_credentials {
    char    service[ANAME_SZ];  /* Service name */
    char    instance[INST_SZ];  /* Instance */
    char    realm[REALM_SZ];    /* Auth domain */
    C_Block session;            /* Session key */
    int     lifetime;           /* Lifetime */
    int     kvno;               /* Key version number */
    KTEXT_ST ticket_st;         /* The ticket itself */
    long    issue_date;         /* The issue time */
    char    pname[ANAME_SZ];    /* Principal's name */
    char    pinst[INST_SZ];     /* Principal's instance */
    char    address[ADDR_SZ];   /* IP Address in ticket */
};

typedef struct leash_credentials LEASH_CREDENTIALS;

#define krb_get_err_text_entry       ck_krb_get_err_text_entry
#define krb_get_cred                 ck_krb_get_cred
#define krb_mk_req                   ck_krb_mk_req
#define krb_realmofhost              ck_krb_realmofhost
#define krb_get_phost                ck_krb_get_phost
#define krb_get_pw_in_tkt            ck_krb_get_pw_in_tkt
#define krb_get_pw_in_tkt_preauth    ck_krb_get_pw_in_tkt_preauth
#define krb_get_lrealm               ck_krb_get_lrealm
#define krb_get_err_text             ck_krb_get_err_text
#define kname_parse                  ck_kname_parse
#define dest_tkt                     ck_dest_tkt
#define krb_get_tf_realm             ck_krb_get_tf_realm
#define krb_get_tf_fullname          ck_krb_get_tf_fullname
#define tf_get_pname                 ck_tf_get_pname
#define tf_get_pinst                 ck_tf_get_pinst
#define tf_get_cred                  ck_tf_get_cred
#define tf_close                     ck_tf_close
#define tf_init                      ck_tf_init
#define tkt_string                   ck_tkt_string
#define krb_check_serv               ck_krb_check_serv
#define kuserok                      ck_kuserok
#define krb_kntoln                   ck_krb_kntoln
#define krb_rd_req                   ck_krb_rd_req
#define krb_sendauth                 ck_krb_sendauth
#define set_krb_debug                ck_set_krb_debug
#define set_krb_ap_req_debug         ck_set_krb_ap_req_debug
#define krb_mk_safe                  ck_krb_mk_safe
#define krb_mk_priv                  ck_krb_mk_priv
#define krb_rd_priv                  ck_krb_rd_priv
#define krb_rd_safe                  ck_krb_rd_safe
#define krb_in_tkt                   ck_krb_in_tkt
#define krb_save_credentials         ck_krb_save_credentials

const char * ck_krb_get_err_text_entry(int _errno);
int ck_krb_get_cred(char *service, char *instance,
                        char *realm, LEASH_CREDENTIALS *c);
int ck_krb_mk_req(KTEXT authent, char *service,
                     char *instance, char *realm,
                     KRB4_32 checksum);
char * ck_krb_realmofhost(char *host);
char * ck_krb_get_phost(char *alias);
int ck_krb_get_pw_in_tkt(char  *user, char  *instance, char  *realm,
                          char  *service, char  *sinstance,
                          int life, char  *password);
int ck_krb_get_pw_in_tkt_preauth
    (char  *user, char  *instance, char  *realm,
                   char  *service, char  *sinstance,
                   int life, char  *password);
int ck_krb_get_lrealm(char  *realm, int index);
const char * ck_krb_get_err_text(int _errno);
int ck_krb_in_tkt(char *, char *, char *);
int ck_krb_save_credentials(char *, char *, char *, C_Block, int, int, KTEXT, long);
int ck_kname_parse(char  *name, char  *inst,
                              char  *realm,char  *fullname);
int ck_dest_tkt(void);

int ck_tf_get_pname(char*);
int ck_tf_get_pinst(char*);
int ck_tf_get_cred(LEASH_CREDENTIALS*);
void ck_tf_close(void);
int ck_tf_init(char *,int);
char * ck_tkt_string(void);
int ck_krb_get_tf_realm(char *ticket_file,
                                             char *realm);
int ck_krb_get_tf_fullname(char *ticket_file,
                                     char *name, char *inst,
                                       char *realm);
int ck_krb_check_serv(char * service);
int ck_kuserok(AUTH_DAT *kdata, char *luser);
int ck_k95_k4_userok( const char * princ_name,
                                const char * princ_inst,
                                const char * princ_realm,
                                const char * local_realm,
                                const char *luser);
int ck_krb_kntoln(AUTH_DAT *kdata, char *luser);
int ck_k95_k4_princ_to_userid( const char * princ_name,
                               const char * princ_inst,
                               const char * princ_realm,
                               const char * local_realm,
                               char *luser, int len);
int ck_krb_rd_req(KTEXT, char *service, char *inst,
                   unsigned int from_addr, AUTH_DAT  *,
                   char  *srvtab);
int ck_krb_sendauth(long,int,KTEXT,CHAR *,CHAR *,CHAR *,
                     unsigned long,MSG_DAT *,LEASH_CREDENTIALS *, Key_schedule *,
                     struct sockaddr_in *,struct sockaddr_in *, CHAR *);
int ck_set_krb_debug(int);
int ck_set_krb_ap_req_debug(int);
long ck_krb_mk_safe(char * in, char * out,
                     unsigned long length,
                     C_Block block,
                     struct sockaddr_in * sender,
                     struct sockaddr_in * receiver);
long ck_krb_mk_priv(char * in, char * out,
                     unsigned long length,
                     Schedule sched,
                     C_Block block,
                     struct sockaddr_in * sender,
                     struct sockaddr_in * receiver);
long ck_krb_rd_priv(char * in, unsigned long in_length,
                     Schedule sched,
                     C_Block block,
                     struct sockaddr_in * sender,
                     struct sockaddr_in * receiver,
                     MSG_DAT * m_data);
long ck_krb_rd_safe(char * in, unsigned long in_length,
                     C_Block block,
                     struct sockaddr_in * sender,
                     struct sockaddr_in * receiver,
                     MSG_DAT * m_data);
#endif /* MAP_KRB4 */

#ifdef MAP_KRB5
#ifndef krb5_const
#define krb5_const const
#endif
#define com_err                       ck_com_err
#define error_message                 ck_error_message
#define krb5_free_creds               ck_krb5_free_creds
#define krb5_free_data                ck_krb5_free_data
#define krb5_free_data_contents       ck_krb5_free_data_contents
#define krb5_copy_keyblock            ck_krb5_copy_keyblock
#define krb5_free_keyblock            ck_krb5_free_keyblock
#define krb5_auth_con_getlocalsubkey  ck_krb5_auth_con_getlocalsubkey
#define krb5_mk_req_extended          ck_krb5_mk_req_extended
#define krb5_mk_req                   ck_krb5_mk_req
#define krb5_auth_con_setflags        ck_krb5_auth_con_setflags
#define krb5_auth_con_init            ck_krb5_auth_con_init
#define krb5_auth_con_free            ck_krb5_auth_con_free
#define krb5_get_credentials          ck_krb5_get_credentials
#define krb5_free_cred_contents       ck_krb5_free_cred_contents
#define krb5_sname_to_principal       ck_krb5_sname_to_principal
#define krb5_cc_default               ck_krb5_cc_default
#define krb5_free_ap_rep_enc_part     ck_krb5_free_ap_rep_enc_part
#define krb5_rd_rep                   ck_krb5_rd_rep
#define krb5_init_context             ck_krb5_init_context
#define krb5_init_ets                 ck_krb5_init_ets
#define krb5_free_context             ck_krb5_free_context
#define krb5_free_unparsed_name       ck_krb5_free_unparsed_name
#define krb5_free_principal           ck_krb5_free_principal
#define krb5_fwd_tgt_creds            ck_krb5_fwd_tgt_creds
#define krb5_auth_con_genaddrs        ck_krb5_auth_con_genaddrs

#define krb5_get_host_realm             ck_krb5_get_host_realm
#define krb5_free_host_realm            ck_krb5_free_host_realm
#define krb5_get_in_tkt_with_keytab     ck_krb5_get_in_tkt_with_keytab
#define krb5_get_in_tkt_with_password   ck_krb5_get_in_tkt_with_password
#define krb5_read_password              ck_krb5_read_password
#define krb5_build_principal_ext        ck_krb5_build_principal_ext
#define krb5_unparse_name               ck_krb5_unparse_name
#define krb5_parse_name                 ck_krb5_parse_name
#define krb5_cc_resolve                 ck_krb5_cc_resolve
#define krb5_cc_default_name            ck_krb5_cc_default_name
#define krb5_string_to_timestamp        ck_krb5_string_to_timestamp
#define krb5_kt_resolve                 ck_krb5_kt_resolve
#define krb5_string_to_deltat           ck_krb5_string_to_deltat
#define krb5_timeofday                  ck_krb5_timeofday
#define krb5_get_credentials_renew      ck_krb5_get_credentials_renew
#define krb5_get_credentials_validate   ck_krb5_get_credentials_validate
#define krb5_copy_principal             ck_krb5_copy_principal
#define krb5_timestamp_to_sfstring      ck_krb5_timestamp_to_sfstring
#define krb5_kt_default                 ck_krb5_kt_default
#define krb5_free_ticket                ck_krb5_free_ticket
#define decode_krb5_ticket              ck_decode_krb5_ticket

#ifdef CHECKADDRS
#define krb5_os_localaddr               ck_krb5_os_localaddr
#define krb5_address_search             ck_krb5_address_search
#define krb5_free_addresses             ck_krb5_free_addresses
#endif /* CHECKADDRS */

#define krb5_auth_con_getremotesubkey   ck_krb5_auth_con_getremotesubkey
#define krb5_mk_rep                     ck_krb5_mk_rep
#define krb5_free_authenticator         ck_krb5_free_authenticator
#define krb5_verify_checksum            ck_krb5_verify_checksum
#define krb5_auth_con_getkey            ck_krb5_auth_con_getkey
#define krb5_auth_con_getauthenticator  ck_krb5_auth_con_getauthenticator
#define krb5_rd_req                     ck_krb5_rd_req
#define krb5_auth_con_setrcache         ck_krb5_auth_con_setrcache
#define krb5_get_server_rcache          ck_krb5_get_server_rcache
#define krb5_auth_con_getrcache         ck_krb5_auth_con_getrcache
#define krb5_free_tgt_creds             ck_krb5_free_tgt_creds
#define krb5_rd_cred                    ck_krb5_rd_cred
#define krb5_c_enctype_compare          ck_krb5_c_enctype_compare
#define krb5_c_encrypt_length           ck_krb5_c_encrypt_length
#define krb5_write_message              ck_krb5_write_message
#define krb5_free_error                 ck_krb5_free_error
#define krb5_sendauth                   ck_krb5_sendauth
#define krb5_process_key                ck_krb5_process_key
#define krb5_use_enctype                ck_krb5_use_enctype
#define krb5_encrypt                    ck_krb5_encrypt
#define krb5_decrypt                    ck_krb5_decrypt
#define krb5_encrypt_size               ck_krb5_encrypt_size
#define krb5_kuserok                    ck_krb5_kuserok
#define krb5_aname_to_localname         ck_krb5_aname_to_localname

#define krb5_appdefault_boolean         ck_krb5_appdefault_boolean
#define krb5_appdefault_string          ck_krb5_appdefault_string
#define krb5_get_init_creds_password              ck_krb5_get_init_creds_password
#define krb5_get_init_creds_opt_set_address_list  ck_krb5_get_init_creds_opt_set_address_list
#define krb5_get_renewed_creds                    ck_krb5_get_renewed_creds
#define krb5_get_validated_creds                  ck_krb5_get_validated_creds
#define krb5_get_init_creds_opt_set_tkt_life      ck_krb5_get_init_creds_opt_set_tkt_life
#define krb5_get_init_creds_opt_set_forwardable   ck_krb5_get_init_creds_opt_set_forwardable
#define krb5_get_init_creds_opt_set_proxiable     ck_krb5_get_init_creds_opt_set_proxiable
#define krb5_get_init_creds_opt_set_renew_life    ck_krb5_get_init_creds_opt_set_renew_life
#define krb5_get_init_creds_opt_init              ck_krb5_get_init_creds_opt_init
#define krb5_get_init_creds_opt_set_etype_list    ck_krb5_get_init_creds_opt_set_etype_list
#define krb5_get_init_creds_opt_set_preauth_list  ck_krb5_get_init_creds_opt_set_preauth_list
#define krb5_get_init_creds_opt_set_salt          ck_krb5_get_init_creds_opt_set_salt
#define krb5_rd_safe                              ck_krb5_rd_safe
#define krb5_mk_safe                              ck_krb5_mk_safe
#define krb5_rd_priv                              ck_krb5_rd_priv
#define krb5_mk_priv                              ck_krb5_mk_priv
#define krb5_auth_con_setuseruserkey              ck_krb5_auth_con_setuseruserkey
#define krb5_read_message                         ck_krb5_read_message
#define krb5_get_profile                          ck_krb5_get_profile
#define profile_get_relation_names                ck_profile_get_relation_names
#define profile_get_subsection_names              ck_profile_get_subsection_names
#define krb5_free_keyblock_contents               ck_krb5_free_keyblock_contents
#define krb5_c_encrypt                            ck_krb5_c_encrypt
#define krb5_c_decrypt                            ck_krb5_c_decrypt
#define krb5_c_make_random_key                    ck_krb5_c_make_random_key
#define krb5_c_random_seed                        ck_krb5_c_random_seed
#define krb5_c_block_size                         ck_krb5_c_block_size
#define krb5_kt_default_name                      ck_krb5_kt_default_name

#ifdef  krb5_cc_get_principal
#define NO_DEF_KRB5_CCACHE
#undef  krb5_cc_get_principal
#endif
#define krb5_cc_get_principal                     ck_krb5_cc_get_principal
#ifdef krb5_cc_store_cred
#undef krb5_cc_store_cred
#endif
#define krb5_cc_store_cred                        ck_krb5_cc_store_cred
#ifdef krb5_cc_initialize
#undef krb5_cc_initialize
#endif
#define krb5_cc_initialize                        ck_krb5_cc_initialize
#ifdef krb5_cc_destroy
#undef krb5_cc_destroy
#endif
#define krb5_cc_destroy                           ck_krb5_cc_destroy
#ifdef krb5_cc_end_seq_get
#undef krb5_cc_end_seq_get
#endif
#define krb5_cc_end_seq_get                       ck_krb5_cc_end_seq_get
#ifdef krb5_cc_next_cred
#undef krb5_cc_next_cred
#endif
#define krb5_cc_next_cred                         ck_krb5_cc_next_cred
#ifdef krb5_cc_start_seq_get
#undef krb5_cc_start_seq_get
#endif
#define krb5_cc_start_seq_get                     ck_krb5_cc_start_seq_get
#ifdef krb5_cc_get_type
#undef krb5_cc_get_type
#endif
#define krb5_cc_get_type                          ck_krb5_cc_get_type
#ifdef krb5_cc_get_name
#undef krb5_cc_get_name
#endif
#define krb5_cc_get_name                          ck_krb5_cc_get_name
#ifdef krb5_cc_set_flags
#undef krb5_cc_set_flags
#endif
#define krb5_cc_set_flags                         ck_krb5_cc_set_flags
#ifdef krb5_cc_close
#undef krb5_cc_close
#endif
#define krb5_cc_close                             ck_krb5_cc_close

#ifdef krb5_kt_get_type
#define NO_DEF_KRB5_KT
#undef krb5_kt_get_type
#endif
#define krb5_kt_get_type                          ck_krb5_kt_get_type
#ifdef krb5_kt_get_name
#undef krb5_kt_get_name
#endif
#define krb5_kt_get_name                          ck_krb5_kt_get_name
#ifdef krb5_kt_close
#undef krb5_kt_close
#endif
#define krb5_kt_close                             ck_krb5_kt_close
#ifdef krb5_kt_get_entry
#undef krb5_kt_get_entry
#endif
#define krb5_kt_get_entry                         ck_krb5_kt_get_entry
#ifdef krb5_kt_start_seq_get
#undef krb5_kt_start_seq_get
#endif
#define krb5_kt_start_seq_get                     ck_krb5_kt_start_seq_get
#ifdef krb5_kt_next_entry
#undef krb5_kt_next_entry
#endif
#define krb5_kt_next_entry                        ck_krb5_kt_next_entry
#ifdef krb5_kt_end_seq_get
#undef krb5_kt_end_seq_get
#endif
#define krb5_kt_end_seq_get                       ck_krb5_kt_end_seq_get
#define krb5_build_principal                      ck_krb5_build_principal

#define krb524_init_ets                           ck_krb524_init_ets
#define krb524_convert_creds_kdc                  ck_krb524_convert_creds_kdc

void
ck_com_err(const char *, errcode_t, const char *, ...);
void ck_com_err_va(const char *, errcode_t, const char *, va_list);
const char * ck_error_message(errcode_t);
void ck_krb5_free_creds(krb5_context, krb5_creds*);
void ck_krb5_free_data(krb5_context, krb5_data *);
void ck_krb5_free_data_contents(krb5_context, krb5_data *);
krb5_error_code ck_krb5_copy_keyblock(krb5_context,
                                       krb5_const krb5_keyblock  *,
                                       krb5_keyblock  *  *);
void ck_krb5_free_keyblock(krb5_context, krb5_keyblock  *);
krb5_error_code ck_krb5_auth_con_getlocalsubkey(krb5_context,
                                                 krb5_auth_context,
                                                 krb5_keyblock  *  *);
krb5_error_code ck_krb5_mk_req_extended (krb5_context,
                                          krb5_auth_context  *,
                                          krb5_const krb5_flags,
                                          krb5_data  *,
                                          krb5_creds  *,
                                          krb5_data  * );
krb5_error_code ck_krb5_mk_req(krb5_context,
                krb5_auth_context *,
                krb5_const krb5_flags,
                char *,
                char *,
                krb5_data *,
                krb5_ccache,
                krb5_data * );
krb5_error_code
ck_krb5_auth_con_setflags(krb5_context,
                                        krb5_auth_context,
                                        krb5_int32);
krb5_error_code
ck_krb5_auth_con_init(krb5_context,
                                    krb5_auth_context  *);
krb5_error_code
ck_krb5_auth_con_free(krb5_context,
                                    krb5_auth_context);
krb5_error_code
ck_krb5_get_credentials(krb5_context,
                                     krb5_const krb5_flags,
                                     krb5_ccache,
                                     krb5_creds  *,
                                     krb5_creds  *  *);
void
ck_krb5_free_cred_contents(krb5_context, krb5_creds  *);
krb5_error_code
ck_krb5_sname_to_principal(krb5_context,
                                        krb5_const char  *,
                                        krb5_const char  *,
                                        krb5_int32,
                                        krb5_principal  *);
krb5_error_code
ck_krb5_cc_default(krb5_context,
                                krb5_ccache  *);
void
ck_krb5_free_ap_rep_enc_part(krb5_context,
                                          krb5_ap_rep_enc_part  *);
krb5_error_code
ck_krb5_rd_rep(krb5_context,
                            krb5_auth_context,
                            krb5_const krb5_data  *,
                            krb5_ap_rep_enc_part  *  *);
krb5_error_code ck_krb5_init_context(krb5_context *);
void ck_krb5_free_context(krb5_context);
krb5_error_code ck_krb5_init_ets(krb5_context);
void ck_krb5_free_principal(krb5_context, krb5_principal );
void ck_krb5_free_unparsed_name(krb5_context, char *);
krb5_error_code ck_krb5_fwd_tgt_creds(krb5_context,
                                     krb5_auth_context,
                                     char  *,
                                     krb5_principal,
                                     krb5_principal,
                                     krb5_ccache,
                                     int forwardable,
                                     krb5_data  *);
krb5_error_code
ck_krb5_auth_con_genaddrs(krb5_context,
                           krb5_auth_context,
                           int, int);

krb5_error_code
ck_krb5_get_host_realm(krb5_context,
                        const char *,
                        char ***);
krb5_error_code
ck_krb5_free_host_realm(krb5_context,
                         char **);
krb5_error_code
ck_krb5_get_in_tkt_with_keytab(krb5_context,
                                krb5_const krb5_flags,
                                krb5_address * krb5_const *,
                                krb5_enctype *,
                                krb5_preauthtype *,
                                krb5_const krb5_keytab,
                                krb5_ccache,
                                krb5_creds *,
                                krb5_kdc_rep ** );

krb5_error_code
ck_krb5_get_in_tkt_with_password(krb5_context,
                                  krb5_const krb5_flags,
                                  krb5_address * krb5_const *,
                                  krb5_enctype *,
                                  krb5_preauthtype *,
                                  krb5_const char *,
                                  krb5_ccache,
                                  krb5_creds *,
                                  krb5_kdc_rep **);

krb5_error_code
ck_krb5_read_password(krb5_context,
                       const char *,
                       const char *,
                       char *,
                       int * );

krb5_error_code
ck_krb5_build_principal_ext(krb5_context,
                             krb5_principal *,
                             int,
                             krb5_const char *,
                             int n2,
                             krb5_const char * sz2,
                             int n3,
                             krb5_const char * sz3,
                             ...);

krb5_error_code
ck_krb5_unparse_name(krb5_context,
                      krb5_const_principal,
                      char ** );

krb5_error_code
ck_krb5_parse_name(krb5_context,
                    krb5_const char *,
                    krb5_principal *);

krb5_error_code
ck_krb5_cc_resolve(krb5_context,
                    char *,
                    krb5_ccache *);

const char *
ck_krb5_cc_default_name(krb5_context);

krb5_error_code
ck_krb5_string_to_timestamp(char *,
                             krb5_timestamp *);

krb5_error_code
ck_krb5_kt_resolve(krb5_context,
                    krb5_const char *,
                    krb5_keytab *);

krb5_error_code
ck_krb5_string_to_deltat(char *, krb5_deltat *);

krb5_error_code
ck_krb5_timeofday(krb5_context,
                   krb5_int32 *);

krb5_error_code
ck_krb5_get_credentials_renew(krb5_context,
                               krb5_const krb5_flags,
                               krb5_ccache,
                               krb5_creds *,
                               krb5_creds **);

krb5_error_code
ck_krb5_get_credentials_validate(krb5_context,
                                  krb5_const krb5_flags,
                                  krb5_ccache,
                                  krb5_creds *,
                                  krb5_creds **);

krb5_error_code
ck_krb5_copy_principal(krb5_context,
                        krb5_const_principal,
                        krb5_principal *);

krb5_error_code
ck_krb5_timestamp_to_sfstring(krb5_timestamp,
                               char *,
                               size_t,
                               char *);

krb5_error_code
ck_krb5_kt_default(krb5_context,
                    krb5_keytab *);

krb5_error_code
ck_krb5_free_ticket(krb5_context, krb5_ticket *);

krb5_error_code
ck_decode_krb5_ticket(const krb5_data *code,
                       krb5_ticket **rep);

krb5_error_code
ck_cygnus_decode_krb5_ticket(const krb5_data *code,
                              krb5_ticket **rep);

#ifdef CHECKADDRS
krb5_error_code
ck_krb5_os_localaddr(krb5_context con,
                      krb5_address *** ppp);

krb5_boolean
ck_krb5_address_search(krb5_context con,
                          krb5_const krb5_address * p,
                          krb5_address * krb5_const * pp);

void
ck_krb5_free_addresses(krb5_context con,
                        krb5_address ** pp);
#endif /* CHECKADDRS */

krb5_error_code
ck_krb5_auth_con_getremotesubkey(krb5_context,
                                  krb5_auth_context,
                                  krb5_keyblock **);

krb5_error_code
ck_krb5_mk_rep(krb5_context, krb5_auth_context,
                krb5_data *);

krb5_error_code
ck_krb5_free_authenticator(krb5_context,
                            krb5_authenticator *);

krb5_error_code
ck_krb5_verify_checksum(krb5_context context,
                         krb5_const krb5_cksumtype ctype,
                         krb5_const krb5_checksum * cksum,
                         krb5_const krb5_pointer in, krb5_const size_t in_length,
                         krb5_const krb5_pointer seed, krb5_const size_t seed_length);

krb5_error_code
ck_krb5_auth_con_getkey(krb5_context, krb5_auth_context, krb5_keyblock **);

krb5_error_code
ck_krb5_auth_con_getauthenticator
(krb5_context, krb5_auth_context, krb5_authenticator **);

krb5_error_code
ck_krb5_rd_req
(krb5_context, krb5_auth_context *, krb5_const krb5_data *,
    krb5_const_principal, krb5_keytab, krb5_flags *,
    krb5_ticket **);

krb5_error_code
ck_krb5_auth_con_setrcache
(krb5_context, krb5_auth_context, krb5_rcache);

krb5_error_code
ck_krb5_get_server_rcache
(krb5_context, krb5_const krb5_data *, krb5_rcache *);

krb5_error_code
ck_krb5_auth_con_getrcache
(krb5_context, krb5_auth_context, krb5_rcache *);

krb5_error_code
ck_krb5_free_tgt_creds
(krb5_context, krb5_creds ** );

krb5_error_code
ck_krb5_rd_cred
(krb5_context,krb5_auth_context,krb5_data *, krb5_creds ***,
    krb5_replay_data *);

krb5_error_code
ck_krb5_c_enctype_compare
(krb5_context, krb5_enctype, krb5_enctype, krb5_boolean *);

krb5_error_code
ck_krb5_write_message
(krb5_context, krb5_pointer, krb5_data *);

krb5_error_code
ck_krb5_free_error
(krb5_context, krb5_error *);

krb5_error_code
ck_krb5_sendauth
(krb5_context,
    krb5_auth_context *,
    krb5_pointer,
    char *,
    krb5_principal,
    krb5_principal,
    krb5_flags,
    krb5_data *,
    krb5_creds *,
    krb5_ccache,
    krb5_error **,
    krb5_ap_rep_enc_part **,
    krb5_creds **);

krb5_error_code
ck_krb5_process_key
(krb5_context, krb5_encrypt_block *, const krb5_keyblock *);

krb5_error_code
ck_krb5_use_enctype
(krb5_context, krb5_encrypt_block *, const krb5_enctype);

krb5_error_code
ck_krb5_encrypt
(krb5_context context,
    krb5_const krb5_pointer inptr,
    krb5_pointer outptr,
    krb5_const size_t size,
    krb5_encrypt_block * eblock,
    krb5_pointer ivec);

size_t
ck_krb5_encrypt_size
(krb5_const size_t length,
    krb5_enctype crypto);

krb5_error_code
ck_krb5_decrypt
(krb5_context context,
    krb5_const krb5_pointer inptr,
    krb5_pointer outptr,
    krb5_const size_t size,
    krb5_encrypt_block  * eblock,
    krb5_pointer ivec);

krb5_boolean
ck_krb5_kuserok
(krb5_context context,krb5_principal princ, const char * p);

krb5_error_code
ck_krb5_aname_to_localname
(krb5_context context, krb5_const_principal aname, const int lnsize, char *lname);

int
ck_k95_k5_userok
(const char * princ_name, const char * princ_inst, const char * princ_realm,
   const char * local_realm, const char * userid);

int
ck_k95_k5_principal_to_localname
(const char * princ_name, const char * local_realm, char * userid, int len);

void
ck_krb5_appdefault_boolean
(krb5_context,const char *,const krb5_data *,const char *,
     int,int * );
void
ck_krb5_appdefault_string
(krb5_context,const char *,const krb5_data *,const char *,
    char ** );

void ck_krb5_get_init_creds_opt_init
(krb5_get_init_creds_opt *opt);

void ck_krb5_get_init_creds_opt_set_tkt_life
(krb5_get_init_creds_opt *opt,
                 krb5_deltat tkt_life);

void ck_krb5_get_init_creds_opt_set_renew_life
(krb5_get_init_creds_opt *opt,
        krb5_deltat renew_life);

void ck_krb5_get_init_creds_opt_set_forwardable
(krb5_get_init_creds_opt *opt,
        int forwardable);

void ck_krb5_get_init_creds_opt_set_proxiable
(krb5_get_init_creds_opt *opt,
        int proxiable);

void ck_krb5_get_init_creds_opt_set_etype_list
(krb5_get_init_creds_opt *opt,
        krb5_enctype *etype_list,
        int etype_list_length);

void ck_krb5_get_init_creds_opt_set_address_list
(krb5_get_init_creds_opt *opt, krb5_address **addresses);

void ck_krb5_get_init_creds_opt_set_preauth_list
(krb5_get_init_creds_opt *opt,
        krb5_preauthtype *preauth_list,
        int preauth_list_length);

void ck_krb5_get_init_creds_opt_set_salt
(krb5_get_init_creds_opt *opt, krb5_data *salt);

krb5_error_code ck_krb5_get_init_creds_password
(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        char *password,
        krb5_prompter_fct prompter,
        void *data,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options);

krb5_error_code ck_krb5_get_init_creds_keytab
(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_keytab arg_keytab,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options);

krb5_error_code ck_krb5_get_validated_creds
(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service);

krb5_error_code ck_krb5_get_renewed_creds
(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service);

krb5_error_code ck_krb5_rd_safe
(krb5_context,
       krb5_auth_context,
       krb5_const krb5_data  *,
       krb5_data  *,
       krb5_replay_data  *);

krb5_error_code ck_krb5_mk_safe
(krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *);

krb5_error_code ck_krb5_rd_priv
(krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *);

krb5_error_code ck_krb5_mk_priv
(krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *);

krb5_error_code ck_krb5_auth_con_setuseruserkey
(krb5_context,
        krb5_auth_context,
        krb5_keyblock *);

krb5_error_code ck_krb5_read_message
(krb5_context,
        krb5_pointer,
        krb5_data *);

krb5_error_code ck_krb5_get_profile
(krb5_context, profile_t *);

long ck_profile_get_relation_names
(profile_t profile, const char **names, char ***ret_names);

long ck_profile_get_subsection_names
(profile_t profile, const char **names, char ***ret_names);

void ck_krb5_free_keyblock_contents
(krb5_context, krb5_keyblock FAR *);

krb5_error_code ck_krb5_c_encrypt
(krb5_context context, krb5_const krb5_keyblock *key,
        krb5_keyusage usage, krb5_const krb5_data *ivec,
        krb5_const krb5_data *input, krb5_enc_data *output);

krb5_error_code ck_krb5_c_decrypt
(krb5_context context, krb5_const krb5_keyblock *key,
        krb5_keyusage usage, krb5_const krb5_data *ivec,
        krb5_const krb5_enc_data *input, krb5_data *output);

krb5_error_code ck_krb5_c_block_size
(krb5_context context, krb5_enctype enctype,
        size_t *blocksize);

krb5_error_code ck_krb5_c_make_random_key
(krb5_context context, krb5_enctype enctype,
       krb5_keyblock *random_key);

krb5_error_code ck_krb5_c_random_seed
(krb5_context context, krb5_data *data);

krb5_error_code ck_krb5_c_encrypt_length
(krb5_context context, krb5_enctype enctype,
 size_t inputlen, size_t *length);

krb5_error_code ck_krb5_kt_default_name
(krb5_context context, char *, int);

char * ck_krb5_cc_get_name
(krb5_context context, krb5_ccache cache);

krb5_error_code ck_krb5_cc_gen_new
    (krb5_context context, krb5_ccache *cache);

krb5_error_code ck_krb5_cc_initialize
    (krb5_context context, krb5_ccache cache,
                    krb5_principal principal);

krb5_error_code ck_krb5_cc_destroy
    (krb5_context context, krb5_ccache cache);

krb5_error_code ck_krb5_cc_close
    (krb5_context context, krb5_ccache cache);

krb5_error_code ck_krb5_cc_store_cred
    (krb5_context context, krb5_ccache cache,
                     krb5_creds *creds);

krb5_error_code ck_krb5_cc_retrieve_cred
    (krb5_context context, krb5_ccache cache,
      krb5_flags flags, krb5_creds *mcreds,
      krb5_creds *creds);

krb5_error_code ck_krb5_cc_get_principal
    (krb5_context context, krb5_ccache cache,
      krb5_principal *principal);

krb5_error_code ck_krb5_cc_start_seq_get
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor);

krb5_error_code ck_krb5_cc_next_cred
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor, krb5_creds *creds);

krb5_error_code ck_krb5_cc_end_seq_get
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor);

krb5_error_code ck_krb5_cc_remove_cred
    (krb5_context context, krb5_ccache cache, krb5_flags flags,
      krb5_creds *creds);

krb5_error_code ck_krb5_cc_set_flags
    (krb5_context context, krb5_ccache cache, krb5_flags flags);

const char * ck_krb5_cc_get_type
    (krb5_context context, krb5_ccache cache);

const char * ck_krb5_kt_get_type
    (krb5_context, krb5_keytab);

krb5_error_code ck_krb5_kt_get_name
    (krb5_context, krb5_keytab, char *, unsigned int);

krb5_error_code ck_krb5_kt_close
    (krb5_context, krb5_keytab);

krb5_error_code ck_krb5_kt_get_entry
    (krb5_context, krb5_keytab,
                 krb5_principal,
                 krb5_kvno,
                 krb5_enctype,
                 krb5_keytab_entry *);

krb5_error_code ck_krb5_kt_start_seq_get
    (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *);
krb5_error_code ck_krb5_kt_next_entry
        (krb5_context,
                 krb5_keytab,
                 krb5_keytab_entry *,
                 krb5_kt_cursor *);

krb5_error_code ck_krb5_kt_end_seq_get
        (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *);

krb5_error_code ck_krb5_build_principal
        (krb5_context, krb5_principal *, unsigned int, krb5_const char *,
          krb5_const char *,krb5_const char *,krb5_const char *);

#ifdef MAP_KRB4
int ck_krb524_init_ets(krb5_context context);
int ck_krb524_convert_creds_kdc(krb5_context context, krb5_creds *v5creds,
                                LEASH_CREDENTIALS *v4creds);
#endif /* MAP_KRB4 */
#endif /* MAP_KRB5 */

#ifdef MAP_GSSAPI
#define gss_release_buffer    ck_gss_release_buffer
#define gss_seal              ck_gss_seal
#define gss_unseal            ck_gss_unseal
#define gss_release_name      ck_gss_release_name
#define gss_init_sec_context  ck_gss_init_sec_context
#define gss_import_name       ck_gss_import_name
#define gss_display_status    ck_gss_display_status
#define gss_nt_service_name   ck_gss_nt_service_name

#define gss_verify_mic           ck_gss_verify_mic
#define gss_test_oid_set_member  ck_gss_test_oid_set_member
#define gss_indicate_mechs       ck_gss_indicate_mechs
#define gss_add_oid_set_member   ck_gss_add_oid_set_member
#define gss_create_empty_oid_set ck_gss_create_empty_oid_set
#define gss_release_cred         ck_gss_release_cred
#define gss_delete_sec_context   ck_gss_delete_sec_context
#define gss_accept_sec_context   ck_gss_accept_sec_context
#define gss_release_oid_set      ck_gss_release_oid_set
#define gss_acquire_cred         ck_gss_acquire_cred
#define gss_display_name         ck_gss_display_name
#define gss_get_mic              ck_gss_get_mic

unsigned long ck_gss_release_buffer
    ( unsigned long *, gss_buffer_t );
unsigned long ck_gss_seal(unsigned long *, gss_ctx_id_t, int, int,
                           gss_buffer_t, int *, gss_buffer_t );
unsigned long ck_gss_unseal(unsigned long *,
                                                gss_ctx_id_t, gss_buffer_t,
                                                gss_buffer_t, int *, int * );
unsigned long ck_gss_release_name(unsigned long *,
                                                      gss_name_t *);
unsigned long ck_gss_init_sec_context
    (unsigned long *, gss_cred_id_t,
      gss_ctx_id_t *, gss_name_t,
      gss_OID, OM_uint32, OM_uint32,
      gss_channel_bindings_t,
      gss_buffer_t, gss_OID *,
      gss_buffer_t,
      unsigned long *,
      unsigned long * );
unsigned long ck_gss_import_name(unsigned long *,
                                                     gss_buffer_t, gss_OID,
                                                     gss_name_t * );
unsigned long ck_gss_display_status(unsigned long *,
                                                        unsigned long,
                                                        int, gss_OID,
                                                        unsigned long *,
                                                        gss_buffer_t);
unsigned long ck_gss_indicate_mechs
    (OM_uint32 * minor_status, gss_OID_set * mech_set);


OM_uint32 ck_gss_verify_mic
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_buffer_t,               /* message_buffer */
            gss_buffer_t,               /* message_token */
            gss_qop_t *                 /* qop_state */
           );

OM_uint32 ck_gss_release_cred
(OM_uint32 *,           /* minor_status */
            gss_cred_id_t *             /* cred_handle */
           );

OM_uint32 ck_gss_delete_sec_context
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t *,             /* context_handle */
            gss_buffer_t                /* output_token */
           );

OM_uint32 ck_gss_accept_sec_context
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t *,             /* context_handle */
            gss_cred_id_t,              /* acceptor_cred_handle */
            gss_buffer_t,               /* input_token_buffer */
            gss_channel_bindings_t,     /* input_chan_bindings */
            gss_name_t *,               /* src_name */
            gss_OID *,          /* mech_type */
            gss_buffer_t,               /* output_token */
            OM_uint32 *,                /* ret_flags */
            OM_uint32 *,                /* time_rec */
            gss_cred_id_t *             /* delegated_cred_handle */
           );

OM_uint32 ck_gss_acquire_cred
(OM_uint32 *,           /* minor_status */
            gss_name_t,                 /* desired_name */
            OM_uint32,                  /* time_req */
            gss_OID_set,                /* desired_mechs */
            gss_cred_usage_t,           /* cred_usage */
            gss_cred_id_t *,    /* output_cred_handle */
            gss_OID_set *,              /* actual_mechs */
            OM_uint32 *         /* time_rec */
           );

OM_uint32 ck_gss_get_mic
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_qop_t,                  /* qop_req */
            gss_buffer_t,               /* message_buffer */
            gss_buffer_t                /* message_token */
           );

OM_uint32 ck_gss_display_name
(OM_uint32 *,           /* minor_status */
            gss_name_t,                 /* input_name */
            gss_buffer_t,               /* output_name_buffer */
            gss_OID *           /* output_name_type */
           );

OM_uint32 ck_gss_test_oid_set_member
(OM_uint32 *,           /* minor_status */
            gss_OID,                    /* member */
            gss_OID_set,                /* set */
            int *                       /* present */
           );

OM_uint32 ck_gss_add_oid_set_member
(OM_uint32 *,           /* minor_status */
            gss_OID,                    /* member_oid */
            gss_OID_set *               /* oid_set */
           );

OM_uint32 ck_gss_create_empty_oid_set
(OM_uint32 *,           /* minor_status */
            gss_OID_set *               /* oid_set */
           );

OM_uint32 ck_gss_release_oid_set
(OM_uint32 *,           /* minor_status */
            gss_OID_set *               /* set */
           );


#ifdef GSS_OIDS
/** exported constants defined in gssapi_krb5{,_nx}.h **/

/* these are bogus, but will compile */

/*
 * The OID of the draft krb5 mechanism, assigned by IETF, is:
 *      iso(1) org(3) dod(5) internet(1) security(5)
 *      kerberosv5(2) = 1.3.5.1.5.2
 * The OID of the krb5_name type is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_name(1) = 1.2.840.113554.1.2.2.1
 * The OID of the krb5_principal type is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_principal(2) = 1.2.840.113554.1.2.2.2
 * The OID of the proposed standard krb5 mechanism is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) = 1.2.840.113554.1.2.2
 * The OID of the proposed standard krb5 v2 mechanism is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5v2(3) = 1.2.840.113554.1.2.3
 *
 */

/*
 * Encoding rules: The first two values are encoded in one byte as 40
 * * value1 + value2.  Subsequent values are encoded base 128, most
 * significant digit first, with the high bit (\200) set on all octets
 * except the last in each value's encoding.
 */

static CONST gss_OID_desc
ck_krb5_gss_oid_array[] = {
   /* this is the official, rfc-specified OID */
   {9, "\052\206\110\206\367\022\001\002\002"},
   /* this is the unofficial, wrong OID */
   {5, "\053\005\001\005\002"},
   /* this is the v2 assigned OID */
   {9, "\052\206\110\206\367\022\001\002\003"},
   /* these two are name type OID's */
   {10, "\052\206\110\206\367\022\001\002\002\001"},
   {10, "\052\206\110\206\367\022\001\002\002\002"},
   { 0, 0 }
};

static CONST gss_OID_desc * CONST ck_gss_mech_krb5 = ck_krb5_gss_oid_array+0;
static CONST gss_OID_desc * CONST ck_gss_mech_krb5_old = ck_krb5_gss_oid_array+1;
static CONST gss_OID_desc * CONST ck_gss_mech_krb5_v2 = ck_krb5_gss_oid_array+2;
static CONST gss_OID_desc * CONST ck_gss_nt_krb5_name = ck_krb5_gss_oid_array+3;
static CONST gss_OID_desc * CONST ck_gss_nt_krb5_principal = ck_krb5_gss_oid_array+4;

/*
 * See krb5/gssapi_krb5.c for a description of the algorithm for
 * encoding an object identifier.
 */

/*
 * The OID of user_name is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      generic(1) user_name(1) = 1.2.840.113554.1.2.1.1
 * machine_uid_name:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      generic(1) machine_uid_name(2) = 1.2.840.113554.1.2.1.2
 * string_uid_name:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      generic(1) string_uid_name(3) = 1.2.840.113554.1.2.1.3
 * service_name:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      generic(1) service_name(4) = 1.2.840.113554.1.2.1.4
 * exported_name:
 *      1(iso), 3(org), 6(dod), 1(internet), 5(security), 6(nametypes),
 *          4(gss-api-exported-name)
 * host_based_service_name (v2):
 *      iso (1) org (3), dod (6), internet (1), security (5), nametypes(6),
 *      gss-host-based-services(2)
 */

static gss_OID_desc ck_oids[] = {
   {10, "\052\206\110\206\367\022\001\002\001\001"},
   {10, "\052\206\110\206\367\022\001\002\001\002"},
   {10, "\052\206\110\206\367\022\001\002\001\003"},
   {10, "\052\206\110\206\367\022\001\002\001\004"},
   { 6, "\053\006\001\005\006\004"},
   { 6, "\053\006\001\005\006\002"},
};

static gss_OID ck_gss_nt_user_name = ck_oids+0;
static gss_OID ck_gss_nt_machine_uid_name = ck_oids+1;
static gss_OID ck_gss_nt_string_uid_name = ck_oids+2;
static gss_OID ck_gss_nt_service_name = ck_oids+3;
static gss_OID ck_gss_nt_exported_name = ck_oids+4;
static gss_OID ck_gss_nt_service_name_v2 = ck_oids+5;
#endif /* GSS_OIDS */
#endif /* MAP_GSSAPI */

#ifdef NT
extern HINSTANCE hKRB5_32;
extern HINSTANCE hKRB4_32;
extern HINSTANCE hCOMERR32;
extern HINSTANCE hPROFILE;
extern HINSTANCE hKRB42UID;
extern HINSTANCE hKRB52UID;
extern HINSTANCE hCRYPT;
extern HINSTANCE hGSSAPI;
extern HINSTANCE hLIBKRYPTO;
#else /* NT */
extern HMODULE hKRB5_32;
extern HMODULE hKRB4_32;
extern HMODULE hCOMERR32;
extern HMODULE hPROFILE;
extern HMODULE hKRB42UID;
extern HMODULE hKRB52UID;
extern HMODULE hCRYPT;
extern HMODULE hGSSAPI;
extern HMODULE hLIBKRYPTO;
#endif /* NT */

#ifdef MAP_CRYPT
int
ck_encrypt_parse(unsigned char * s, int n);
void
ck_encrypt_init(kstream ks, int type);
int
ck_encrypt_session_key(Session_Key * key, int n);
int
ck_encrypt_dont_support(int);
void
ck_encrypt_send_support(void);
void
ck_encrypt_send_request_start(void);
void
ck_encrypt_request_start(void);
void
ck_encrypt_send_request_end(void);
void
ck_encrypt_send_end(void);
int
ck_encrypt_is_encrypting(void);
int
ck_encrypt_is_decrypting(void);
int
ck_get_crypt_table(struct keytab ** pTable, int * pN);
int
ck_des_is_weak_key(Block B);

#define encrypt_parse                 ck_encrypt_parse
#define encrypt_init                  ck_encrypt_init
#define encrypt_session_key           ck_encrypt_session_key
#define encrypt_dont_support          ck_encrypt_dont_support
#define encrypt_send_support          ck_encrypt_send_support
#define encrypt_send_end              ck_encrypt_send_end
#define encrypt_send_request_end      ck_encrypt_send_request_end
#define encrypt_send_request_start    ck_encrypt_send_request_start
#define encrypt_request_start         ck_encrypt_request_start
#define encrypt_is_encrypting         ck_encrypt_is_encrypting
#define encrypt_is_decrypting         ck_encrypt_is_decrypting
#endif /* MAP_CRYPT */

#ifdef MAP_NTLM
#ifdef NT
#define SECURITY_WIN32
#include <security.h>
#ifdef NTLM
extern SecBuffer     NTLMSecBuf[1];
extern HINSTANCE hSSPI;

int
ntlm_is(unsigned char *data, int cnt);
int
ntlm_reply(int how, unsigned char *data, int cnt);
#endif /* NTLM */
#endif /* NT */
#endif /* MAP_NTLM */
#endif /* OS2 */
