/*
  C K O S S L . H --  OpenSSL Interface Header for Kermit 95

  Copyright (C) 2000, 2004, Trustees of Columbia University in the City of New
  York.

  Copyright (C) 2006, 2007  Secure Endpoints Inc.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com),
           Secure Endpoints Inc., New York City
*/

#ifdef OS2
#ifdef CK_SSL
#ifdef SSLDLL
#define SSL_library_init                 ck_SSL_library_init
#ifdef OpenSSL_add_all_algorithms
#undef OpenSSL_add_all_algorithms
#endif
#define OpenSSL_add_all_algorithms        ck_SSL_library_init
#ifdef SSLeay_add_all_algorithms
#undef SSLeay_add_all_algorithms
#endif
#define SSLeay_add_all_algorithms         ck_SSL_library_init
#define SSL_get_error                    ck_SSL_get_error
#define SSL_read                         ck_SSL_read
#define SSL_peek                         ck_SSL_peek
#define SSL_connect                      ck_SSL_connect
#define SSL_set_fd                       ck_SSL_set_fd
#define SSL_free                         ck_SSL_free
#define SSL_shutdown                     ck_SSL_shutdown
#define SSL_write                        ck_SSL_write
#define SSL_pending                      ck_SSL_pending
#define SSL_load_error_strings           ck_SSL_load_error_strings
#define SSL_get_peer_certificate         ck_SSL_get_peer_certificate
#define SSL_CIPHER_get_name              ck_SSL_CIPHER_get_name
#define SSL_CIPHER_get_bits              ck_SSL_CIPHER_get_bits
#define SSL_CIPHER_get_version           ck_SSL_CIPHER_get_version
#define SSL_get_current_cipher           ck_SSL_get_current_cipher
#define SSL_get_shared_ciphers           ck_SSL_get_shared_ciphers
#define SSL_get_ciphers                  ck_SSL_get_ciphers
#define SSL_get_cipher_list              ck_SSL_get_cipher_list
#define SSL_CTX_set_default_verify_paths ck_SSL_CTX_set_default_verify_paths
#define SSL_use_RSAPrivateKey_file       ck_SSL_use_RSAPrivateKey_file
#define SSL_use_DSAPrivateKey_file       ck_SSL_use_DSAPrivateKey_file
#define SSL_use_PrivateKey_file          ck_SSL_use_PrivateKey_file
#define SSL_use_certificate_file         ck_SSL_use_certificate_file
#define SSL_CTX_use_PrivateKey_file      ck_SSL_CTX_use_PrivateKey_file
#define SSL_CTX_check_private_key        ck_SSL_CTX_check_private_key
#define SSL_check_private_key            ck_SSL_check_private_key
#define SSL_CTX_use_certificate_file     ck_SSL_CTX_use_certificate_file
#define SSL_set_verify                   ck_SSL_set_verify
#define SSL_CTX_set_verify               ck_SSL_CTX_set_verify
#define SSL_new                          ck_SSL_new
#define SSL_CTX_ctrl                     ck_SSL_CTX_ctrl
#define SSL_CTX_new                      ck_SSL_CTX_new
#define SSL_CTX_free                     ck_SSL_CTX_free
#define SSL_CTX_set_default_passwd_cb    ck_SSL_CTX_set_default_passwd_cb
#define SSLv23_method                    ck_SSLv23_method
#define SSLv3_method                     ck_SSLv3_method
#define TLSv1_method                     ck_TLSv1_method
#define SSLv23_client_method             ck_SSLv23_client_method
#define SSLv3_client_method              ck_SSLv3_client_method
#define TLSv1_client_method              ck_TLSv1_client_method
#define SSLv23_server_method             ck_SSLv23_server_method
#define SSLv3_server_method              ck_SSLv3_server_method
#define TLSv1_server_method              ck_TLSv1_server_method
#define SSL_state_string                 ck_SSL_state_string
#define SSL_state_string_long            ck_SSL_state_string_long
#define SSL_accept                       ck_SSL_accept
#define SSL_set_cipher_list              ck_SSL_set_cipher_list
#define SSL_CTX_set_tmp_dh_callback      ck_SSL_CTX_set_tmp_dh_callback
#define SSL_CTX_set_tmp_rsa_callback     ck_SSL_CTX_set_tmp_rsa_callback
#define SSL_set_tmp_dh_callback          ck_SSL_set_tmp_dh_callback
#define SSL_set_tmp_rsa_callback         ck_SSL_set_tmp_rsa_callback
#define SSL_CTX_load_verify_locations    ck_SSL_CTX_load_verify_locations
#define SSL_CTX_set_default_passwd_cb_userdata ck_SSL_CTX_set_default_passwd_cb_userdata
#define SSL_get_wbio                     ck_SSL_get_wbio
#define SSL_get_finished                 ck_SSL_get_finished
#define SSL_get_peer_finished            ck_SSL_get_peer_finished
#define SSL_copy_session_id              ck_SSL_copy_session_id
#define SSL_renegotiate                  ck_SSL_renegotiate
#define SSL_get_session                  ck_SSL_get_session
#define SSL_set_session                  ck_SSL_set_session
#define SSL_CTX_set_session_id_context   ck_SSL_CTX_set_session_id_context
#define SSL_get_certificate              ck_SSL_get_certificate
#define SSL_get_privatekey               ck_SSL_get_privatekey
#define SSL_clear                        ck_SSL_clear
#define SSL_set_accept_state             ck_SSL_set_accept_state
#define SSL_CIPHER_description           ck_SSL_CIPHER_description
#ifdef SSL_KRB5
#define kssl_ctx_setstring               ck_kssl_ctx_setstring
#define kssl_ctx_setprinc                ck_kssl_ctx_setprinc
#define kssl_ctx_new                     ck_kssl_ctx_new
#define kssl_ctx_free                    ck_kssl_ctx_free
#endif /* SSL_KRB5 */
#define SSL_COMP_add_compression_method  ck_SSL_COMP_add_compression_method
#define SSL_CTX_use_certificate_chain_file   ck_SSL_CTX_use_certificate_chain_file
#define SSL_CTX_set_info_callback        ck_SSL_CTX_set_info_callback

void ck_SSL_library_init(void);
int     ck_SSL_get_error(SSL *ssl,int num);
int     ck_SSL_read(SSL *ssl,char *buf,int num);
int     ck_SSL_connect(SSL *ssl);
int     ck_SSL_set_fd(SSL *s, int fd);
void    ck_SSL_free(SSL *ssl);
int     ck_SSL_shutdown(SSL *s);
int     ck_SSL_write(SSL *ssl,char *buf,int num);
int     ck_SSL_pending(SSL *ssl);
int     ck_SSL_peek(SSL *ssl,char * buf,int num);
void    ck_SSL_load_error_strings(void );
X509 *  ck_SSL_get_peer_certificate(SSL *s);
const char * ck_SSL_CIPHER_get_name(SSL_CIPHER *c);
int     ck_SSL_CIPHER_get_bits(SSL_CIPHER *c,int *);
char *  ck_SSL_CIPHER_get_version(SSL_CIPHER *c);
SSL_CIPHER * ck_SSL_get_current_cipher(SSL *s);
char *  ck_SSL_get_shared_ciphers(SSL *s, char *buf, int len);
char *  ck_SSL_get_ciphers(SSL *s);
const char * ck_SSL_get_cipher_list(SSL *s, int i);
int     ck_SSL_CTX_set_default_verify_paths(SSL_CTX *ctx);
int     ck_SSL_use_RSAPrivateKey_file(SSL *ssl, char *file, int type);
int     ck_SSL_use_DSAPrivateKey_file(SSL *ssl, char *file, int type);
int     ck_SSL_use_PrivateKey_file(SSL *ssl, char *file, int type);
int     ck_SSL_use_certificate_file(SSL *ssl, char *file, int type);
int     ck_SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, char *file, int type);
int     ck_SSL_CTX_use_certificate_file(SSL_CTX *ctx, char *file, int type);
int     ck_SSL_CTX_check_private_key(SSL_CTX *ctx);
int     ck_SSL_check_private_key(SSL *ssl);
void    ck_SSL_set_verify(SSL *s, int mode, int (*callback) ());
void    ck_SSL_CTX_set_verify(SSL_CTX *ctx, int mode, int (*callback) ());
SSL *   ck_SSL_new(SSL_CTX *ctx);
long    ck_SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, char *parg);
SSL_CTX * ck_SSL_CTX_new(SSL_METHOD *meth);
void    ck_SSL_CTX_free(SSL_CTX *ctx);
void    ck_SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx,int (*cb) ());
SSL_METHOD * ck_SSLv23_method(void);
SSL_METHOD * ck_SSLv3_method(void);
SSL_METHOD * ck_TLSv1_method(void);
SSL_METHOD * ck_SSLv23_client_method(void);
SSL_METHOD * ck_SSLv3_client_method(void);
SSL_METHOD * ck_TLSv1_client_method(void);
SSL_METHOD * ck_SSLv23_server_method(void);
SSL_METHOD * ck_SSLv3_server_method(void);
SSL_METHOD * ck_TLSv1_server_method(void);
char *  ck_SSL_state_string(const SSL *s);
char *  ck_SSL_state_string_long(const SSL *s);
int     ck_SSL_accept(SSL *ssl);
int     ck_SSL_set_cipher_list(SSL *ssl,char *s);
void ck_SSL_CTX_set_tmp_rsa_callback(SSL_CTX *ctx,
                                        RSA *(*cb) (SSL *ssl,int is_export, int keylength));
void ck_SSL_set_tmp_rsa_callback(SSL *ssl,
                                    RSA *(*cb) (SSL *ssl,int is_export,int keylength));
void ck_SSL_CTX_set_tmp_dh_callback(SSL_CTX *ctx,
                                       DH *(*dh) (SSL *ssl,int is_export,int keylength));
void ck_SSL_set_tmp_dh_callback(SSL *ssl,
                                   DH *(*dh) (SSL *ssl,int is_export, int keylength));

int  ck_SSL_CTX_load_verify_locations(SSL_CTX *,char *,char *);
void ck_SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *, void *);
int  ck_X509_to_user(X509 *peer_cert, char *userid, int len);
int  ck_X509_userok(X509 *peer_cert, const char *userid);
BIO *ck_SSL_get_wbio(SSL *);
size_t ck_SSL_get_finished(SSL *,void *buf, size_t count);
size_t ck_SSL_get_peer_finished(SSL *,void *buf, size_t count);
void ck_SSL_copy_session_id(SSL *to,SSL *from);
int  ck_SSL_renegotiate(SSL *);
int  ck_SSL_set_session(SSL *, SSL_SESSION *);
SSL_SESSION * ck_SSL_get_session(SSL *);
int ck_SSL_CTX_set_session_id_context(SSL_CTX *ctx,const unsigned char *sid_ctx,
                                       unsigned int sid_ctx_len);
EVP_PKEY * ck_SSL_get_privatekey(SSL *);
X509 * ck_SSL_get_certificate(SSL *);
int ck_SSL_clear(SSL *);
void ck_SSL_set_accept_state(SSL *);
char * ck_SSL_CIPHER_description(SSL_CIPHER *,char *buf,int size);

void ck_SSL_CTX_set_info_callback(SSL_CTX *ctx, void (*cb)(const SSL *ssl,int type,int val));

#ifdef SSL_KRB5
#ifndef OPENSSL_NO_KRB5
KSSL_CTX * ck_kssl_ctx_new(void);
KSSL_CTX * ck_kssl_ctx_free(KSSL_CTX *kssl_ctx);
krb5_error_code ck_kssl_ctx_setstring(KSSL_CTX *kssl_ctx, int which, char *text);
krb5_error_code ck_kssl_ctx_setprinc(KSSL_CTX *kssl_ctx, int which,
                                krb5_data *realm, krb5_data *entity);
#endif /* OPENSSL_NO_KRB5 */
#endif /* SSL_KRB5 */
int ck_SSL_COMP_add_compression_method(int id,COMP_METHOD *cm);
int ck_SSL_CTX_use_certificate_chain_file(SSL_CTX *ctx, const char *file);

extern int ssl_dll_loaded;
extern int ssl_finished_messages;

#ifdef NT
extern HINSTANCE hSSL;
#else /* NT */
extern HMODULE hSSL;
#endif /* NT */
#endif /* OS2    */
#endif /* CK_SSL */
#endif /* SSLDLL */
