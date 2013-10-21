/*
  C K O S S L . H --  OpenSSL Interface Header for Kermit 95

  Copyright (C) 2000, 2004, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.

  Copyright (C) 2006,2007  Secure Endpoints Inc.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com),
           Secure Endpoints Inc., New York City
*/

#ifdef OS2
#ifdef CK_SSL
#ifdef SSLDLL
#define ERR_print_errors                 ck_ERR_print_errors
#define ERR_print_errors_fp              ck_ERR_print_errors_fp
#define ERR_error_string                 ck_ERR_error_string
#define ERR_get_error                    ck_ERR_get_error

#define BIO_printf                       ck_BIO_printf
#define BIO_ctrl                         ck_BIO_ctrl
#define BIO_new                          ck_BIO_new
#define BIO_s_file                       ck_BIO_s_file
#define BIO_s_mem                        ck_BIO_s_mem
#define BIO_s_null                       ck_BIO_s_null
#define BIO_read                         ck_BIO_read
#define BIO_write                        ck_BIO_write
#define BIO_new_file                     ck_BIO_new_file
#define BIO_free                         ck_BIO_free
#define BIO_free_all                     ck_BIO_free_all

#define X509_get_issuer_name             ck_X509_get_issuer_name
#define X509_verify_cert_error_string    ck_X509_verify_cert_error_string
#define X509_NAME_oneline                ck_X509_NAME_oneline
#define X509_NAME_get_text_by_NID        ck_X509_NAME_get_text_by_NID
#define X509_get_subject_name            ck_X509_get_subject_name
#define X509_STORE_CTX_new               ck_X509_STORE_CTX_new
#define X509_STORE_CTX_free              ck_X509_STORE_CTX_free
#define X509_STORE_CTX_get_current_cert  ck_X509_STORE_CTX_get_current_cert
#define X509_STORE_CTX_get_error         ck_X509_STORE_CTX_get_error
#define X509_STORE_CTX_get_error_depth   ck_X509_STORE_CTX_get_error_depth
#define X509_get_default_cert_dir        ck_X509_get_default_cert_dir
#define X509_free                        ck_X509_free
#define X509_print_ex                    ck_X509_print_ex
#define X509_NAME_print_ex               ck_X509_NAME_print_ex

#define RSA_free                         ck_RSA_free
#define RSA_generate_key                 ck_RSA_generate_key
#define RSA_public_decrypt               ck_RSA_public_decrypt 

#define DH_new                           ck_DH_new
#define DH_free                          ck_DH_free
#define DH_generate_key                  ck_DH_generate_key
#define DH_generate_parameters           ck_DH_generate_parameters

#define DSA_free                         ck_DSA_free
#define DSA_generate_key                 ck_DSA_generate_key
#define DSA_generate_parameters          ck_DSA_generate_parameters

#define PEM_read_bio_DHparams            ck_PEM_read_bio_DHparams
#define PEM_ASN1_read_bio                ck_PEM_ASN1_read_bio

#define d2i_DHparams                     ck_d2i_DHparams
#define ASN1_TIME_print                  ck_ASN1_TIME_print

#ifndef sk_GENERAL_NAME_free
#define sk_GENERAL_NAME_free             ck_sk_GENERAL_NAME_free
#endif /* sk_GENERAL_NAME_free */
#define X509V3_EXT_cleanup               ck_X509V3_EXT_cleanup
#ifndef sk_GENERAL_NAME_value
#define sk_GENERAL_NAME_value            ck_sk_GENERAL_NAME_value
#endif /* sk_GENERAL_NAME_value */
#ifndef sk_GENERAL_NAME_num
#define sk_GENERAL_NAME_num              ck_sk_GENERAL_NAME_num
#endif /* sk_GENERAL_NAME_num */
#define X509V3_EXT_d2i                   ck_X509V3_EXT_d2i
#define X509V3_add_standard_extensions   ck_X509V3_add_standard_extensions
#define X509_get_ext                     ck_X509_get_ext
#define X509_get_ext_by_NID              ck_X509_get_ext_by_NID
#define ASN1_INTEGER_get                 ck_ASN1_INTEGER_get
#define ASN1_STRING_cmp                  ck_ASN1_STRING_cmp
#define X509_get_serialNumber            ck_X509_get_serialNumber
#ifndef sk_X509_REVOKED_value
#define sk_X509_REVOKED_value            ck_sk_X509_REVOKED_value
#endif /* sk_X509_REVOKED_value */
#ifndef sk_X509_REVOKED_num
#define sk_X509_REVOKED_num              ck_sk_X509_REVOKED_num
#endif /* sk_X509_REVOKED_num */
#define X509_cmp_current_time            ck_X509_cmp_current_time
#define X509_OBJECT_free_contents        ck_X509_OBJECT_free_contents
#define X509_STORE_CTX_set_error         ck_X509_STORE_CTX_set_error
#define X509_CRL_verify                  ck_X509_CRL_verify
#define X509_get_pubkey                  ck_X509_get_pubkey
#define X509_STORE_CTX_cleanup           ck_X509_STORE_CTX_cleanup
#define X509_STORE_get_by_subject        ck_X509_STORE_get_by_subject
#define X509_STORE_CTX_init              ck_X509_STORE_CTX_init
#define PEM_read_X509                    ck_PEM_read_X509
#define X509_LOOKUP_hash_dir             ck_X509_LOOKUP_hash_dir
#define X509_LOOKUP_ctrl                 ck_X509_LOOKUP_ctrl
#define X509_STORE_add_lookup            ck_X509_STORE_add_lookup
#define X509_LOOKUP_file                 ck_X509_LOOKUP_file
#define X509_STORE_new                   ck_X509_STORE_new
#define X509_STORE_free                  ck_X509_STORE_free
#define X509_STORE_load_locations        ck_X509_STORE_load_locations
#define X509_STORE_set_default_paths     ck_X509_STORE_set_default_paths
#ifdef ASN1_INTEGER_cmp
#undef ASN1_INTEGER_cmp
#endif /* ASN1_INTEGER_cmp */
#define ASN1_INTEGER_cmp                 ck_ASN1_INTEGER_cmp
#define X509_subject_name_hash           ck_X509_subject_name_hash
#define X509_issuer_name_hash            ck_X509_issuer_name_hash
#define X509_to_user                     ck_X509_to_user
#define X509_userok                      ck_X509_userok
#define CRYPTO_set_mem_functions         ck_CRYPTO_set_mem_functions
#define CRYPTO_set_locking_callback      ck_CRYPTO_set_locking_callback
#define CRYPTO_set_dynlock_create_callback  ck_CRYPTO_set_dynlock_create_callback  
#define CRYPTO_set_dynlock_lock_callback    ck_CRYPTO_set_dynlock_lock_callback    
#define CRYPTO_set_dynlock_destroy_callback ck_CRYPTO_set_dynlock_destroy_callback 
#define RAND_screen                      ck_RAND_screen
#define CRYPTO_num_locks                 ck_CRYPTO_num_locks
#define RAND_status                      ck_RAND_status
#define RAND_file_name                   ck_RAND_file_name
#define RAND_egd                         ck_RAND_egd
#define RAND_load_file                   ck_RAND_load_file
#define RAND_write_file                  ck_RAND_write_file
#define RAND_seed                        ck_RAND_seed
#define sk_free                          ck_sk_free
#define sk_value                         ck_sk_value
#define sk_num                           ck_sk_num
#ifdef ZLIB
#define COMP_zlib                        ck_COMP_zlib
#endif
#define COMP_rle                         ck_COMP_rle
#define ERR_peek_error                   ck_ERR_peek_error
#define sk_pop_free                      ck_sk_pop_free
#define PEM_read_bio_X509                ck_PEM_read_bio_X509
#define SSLeay                           ck_SSLeay
#define SSLeay_version                   ck_SSLeay_version

/* need to  be added for OpenSSH */
#define BN_new                            ck_BN_new
#define BN_bin2bn                         ck_BN_bin2bn
#define BN_bn2bin                         ck_BN_bn2bin
#define BN_num_bits                       ck_BN_num_bits
#define BN_cmp                            ck_BN_cmp
#define BN_add_word                       ck_BN_add_word
#define BN_lshift                         ck_BN_lshift
#define BN_set_word                       ck_BN_set_word
#define BN_dec2bn                         ck_BN_dec2bn
#define BN_bn2dec                         ck_BN_bn2dec
#define BN_copy                           ck_BN_copy
#define BN_CTX_new                        ck_BN_CTX_new
#define BN_CTX_free                       ck_BN_CTX_free
#define BN_CTX_get                        ck_BN_CTX_get
#define BN_CTX_start                      ck_BN_CTX_start
#define BN_CTX_end                        ck_BN_CTX_end
#define BN_div                            ck_BN_div
#define BN_sub                            ck_BN_sub
#define BN_value_one                      ck_BN_value_one
#define BN_free                           ck_BN_free
#define BN_hex2bn                         ck_BN_hex2bn
#define BN_is_bit_set                     ck_BN_is_bit_set
#define BN_rand                           ck_BN_rand
#define BN_clear                          ck_BN_clear
#define BN_set_word     ck_BN_set_word
#define BN_bn2hex       ck_BN_bn2hex
#define BN_lshift       ck_BN_lshift
#define BN_add          ck_BN_add
#define BN_mul          ck_BN_mul
#ifndef BN_mod
#define BN_mod          ck_BN_mod
#endif /* BN_mod may be a macro */
#define BN_mod_word     ck_BN_mod_word
#define BN_mod_mul      ck_BN_mod_mul
#define BN_mod_exp      ck_BN_mod_exp
#define BN_is_prime     ck_BN_is_prime
#define BN_ucmp         ck_BN_ucmp
#define BN_mul_word     ck_BN_mul_word
#define BN_rshift1      ck_BN_rshift1
#define BN_dup          ck_BN_dup

#define MD5_Final                         ck_MD5_Final
#define MD5_Update                        ck_MD5_Update
#define MD5_Init                          ck_MD5_Init
#define BN_clear_free                     ck_BN_clear_free
#define EVP_DigestFinal                   ck_EVP_DigestFinal
#define EVP_DigestUpdate                  ck_EVP_DigestUpdate
#define EVP_DigestInit                    ck_EVP_DigestInit
#define EVP_sha1                          ck_EVP_sha1
#define EVP_md5                           ck_EVP_md5
#define EVP_des_ede3_cbc                  ck_EVP_des_ede3_cbc
#define EVP_PKEY_free                     ck_EVP_PKEY_free
#define EVP_PKEY_get1_DSA                 ck_EVP_PKEY_get1_DSA
#define EVP_PKEY_get1_RSA                 ck_EVP_PKEY_get1_RSA
#define EVP_get_digestbyname              ck_EVP_get_digestbyname
#define EVP_ripemd160                     ck_EVP_ripemd160

#define RSA_new                           ck_RSA_new
#define RSA_public_encrypt                ck_RSA_public_encrypt
#define RSA_private_encrypt               ck_RSA_private_encrypt
#define RSA_private_decrypt               ck_RSA_private_decrypt
#define RSA_sign                          ck_RSA_sign
#define RSA_size                          ck_RSA_size
#define RSA_verify                        ck_RSA_verify

#define DSA_new                           ck_DSA_new
#define DSA_SIG_free                      ck_DSA_SIG_free
#define DSA_do_sign                       ck_DSA_do_sign
#define DSA_do_verify                     ck_DSA_do_verify
#define DSA_SIG_new                       ck_DSA_SIG_new

#ifndef HMAC_cleanup
#define HMAC_cleanup                      ck_HMAC_cleanup
#endif
#define HMAC_CTX_cleanup                  ck_HMAC_CTX_cleanup
#define HMAC_Final                        ck_HMAC_Final
#define HMAC_Update                       ck_HMAC_Update
#define HMAC_Init                         ck_HMAC_Init

#define RAND_add                          ck_RAND_add
#define RAND_bytes                        ck_RAND_bytes

#define RSA_blinding_on                   ck_RSA_blinding_on              
#define EVP_aes_256_cbc                   ck_EVP_aes_256_cbc              
#define EVP_aes_192_cbc                   ck_EVP_aes_192_cbc              
#define EVP_aes_128_cbc                   ck_EVP_aes_128_cbc              
#define EVP_rc4                           ck_EVP_rc4                      
#define EVP_cast5_cbc                     ck_EVP_cast5_cbc                
#define EVP_bf_cbc                        ck_EVP_bf_cbc                   
#define EVP_des_cbc                       ck_EVP_des_cbc                  
#define EVP_enc_null                      ck_EVP_enc_null                 
#define EVP_CipherInit                    ck_EVP_CipherInit               
#define EVP_CIPHER_CTX_set_key_length     ck_EVP_CIPHER_CTX_set_key_length
#define EVP_CIPHER_CTX_init               ck_EVP_CIPHER_CTX_init          
#define EVP_CIPHER_CTX_cleanup            ck_EVP_CIPHER_CTX_cleanup       
#define EVP_CIPHER_CTX_key_length	  ck_EVP_CIPHER_CTX_key_length
#define EVP_Cipher			  ck_EVP_Cipher
#define EVP_CIPHER_CTX_set_app_data       ck_EVP_CIPHER_CTX_set_app_data
#define EVP_CIPHER_CTX_get_app_data	  ck_EVP_CIPHER_CTX_get_app_data
#define EVP_CIPHER_CTX_iv_length          ck_EVP_CIPHER_CTX_iv_length

#ifndef CKCFTP_C
#ifndef CKUATH_C
#ifndef CKOATH_C
#ifdef des_set_key
#undef des_set_key
#endif
#define des_set_key                       ck_des_set_key
#ifdef des_ncbc_encrypt
#undef des_ncbc_encrypt
#endif
#define des_ncbc_encrypt                  ck_des_ncbc_encrypt
#ifdef des_ede3_cbc_encrypt
#undef des_ede3_cbc_encrypt
#endif
#define des_ede3_cbc_encrypt              ck_des_ede3_cbc_encrypt
#endif /* CKOATH_C */
#endif /* CKUATH_C */
#endif /* CKCFTP_C */

#define BF_set_key                        ck_BF_set_key
#define BF_cbc_encrypt                    ck_BF_cbc_encrypt
#define RC4_set_key                       ck_RC4_set_key
#define RC4                               ck_RC4
#define CAST_set_key                      ck_CAST_set_key
#define CAST_cbc_encrypt                  ck_CAST_cbc_encrypt
#define OBJ_nid2sn                        ck_OBJ_nid2sn
#define OBJ_create                        ck_OBJ_create
#define DH_compute_key                    ck_DH_compute_key
#define DH_size                           ck_DH_size
#define ERR_get_error                     ck_ERR_get_error
#define ERR_load_crypto_strings           ck_ERR_load_crypto_strings
#define CRYPTO_free                       ck_CRYPTO_free
#define PEM_write_DSAPrivateKey           ck_PEM_write_DSAPrivateKey
#define PEM_write_RSAPrivateKey           ck_PEM_write_RSAPrivateKey
#define PEM_write_bio_X509                ck_PEM_write_bio_X509
#define PEM_read_PrivateKey               ck_PEM_read_PrivateKey
#define X509_get_default_cert_dir_env     ck_X509_get_default_cert_dir_env
#define X509_get_default_cert_file_env    ck_X509_get_default_cert_file_env
#define EVP_MD_size                       ck_EVP_MD_size

#define SHA1_Init               ck_SHA1_Init
#define SHA1_Update             ck_SHA1_Update
#define SHA1_Final              ck_SHA1_Final

#define OpenSSL_add_all_ciphers           ck_OpenSSL_add_all_ciphers
#define OpenSSL_add_all_digests           ck_OpenSSL_add_all_digests
#define OPENSSL_add_all_algorithms_noconf ck_OPENSSL_add_all_algorithms_noconf
#define OPENSSL_add_all_algorithms_conf   ck_OPENSSL_add_all_algorithms_conf

int     ck_ASN1_INTEGER_cmp(ASN1_INTEGER *x, ASN1_INTEGER *y);
void ck_ERR_print_errors(BIO *bp);
void ck_ERR_print_errors_fp(FILE *fp);
unsigned long ck_ERR_get_error(void);
char *ck_ERR_error_string(unsigned long e,char *buf);

int ck_BIO_printf( BIO * bio, const char *, ... );
long    ck_BIO_ctrl(BIO *bp,int cmd,long larg,char *parg);
BIO *   ck_BIO_new(BIO_METHOD *type);
BIO_METHOD *ck_BIO_s_file(void );
BIO_METHOD *ck_BIO_s_mem(void );
BIO_METHOD *ck_BIO_s_null(void );
int     ck_BIO_read(BIO *b, char *data, int len);
int     ck_BIO_write(BIO *b, const char *data, int len);
int     ck_BIO_free(BIO *a);
int     ck_BIO_free_all(BIO *a);
BIO *   ck_BIO_new_file(char *filename, char *mode);
X509_NAME * ck_X509_get_issuer_name(X509 *a);
char *  ck_X509_verify_cert_error_string(int);
char *      ck_X509_NAME_oneline(X509_NAME *a,char *buf,int len);
int     ck_X509_NAME_print_ex(BIO *out, X509_NAME *nm, int indent, unsigned long flags);
int         ck_X509_NAME_get_text_by_NID(X509_NAME *a,int NID,char *buf,int len);
X509_NAME * ck_X509_get_subject_name(X509 *a);
X509 *      ck_X509_STORE_CTX_get_current_cert(X509_STORE_CTX *ctx);
int         ck_X509_STORE_CTX_get_error(X509_STORE_CTX *ctx);
int         ck_X509_STORE_CTX_get_error_depth(X509_STORE_CTX *ctx);
int         ck_ASN1_TIME_print( BIO * bio, ASN1_TIME *a);
void        ck_X509_free(X509 *a);
const char * ck_X509_get_default_cert_dir();
int         ck_X509_print_ex(BIO *bio, X509 *cert, unsigned long, unsigned long);

void    ck_RSA_free(RSA *r);
RSA *   ck_RSA_generate_key(int bits, unsigned long e,
                                   void(*callback) (int,int,char *),
                                   char *cb_arg);
DH *    ck_DH_new(void);
void    ck_DH_free(DH *r);
DH *    ck_DH_generate_parameters(int bits, unsigned long e,
                                         void(*callback) (int,int,char *),
                                         char *cb_arg);
int     ck_DH_generate_key(DH * dh);
void    ck_DSA_free(DSA *r);
DSA *   ck_DSA_generate_parameters( int bits,
                                          unsigned char * seed,
                                          int seed_len,
                                          int *counter_ret,
                                          unsigned long *h_ret,
                                          void(*callback) (int,int,char *),
                                          char *cb_arg);
int     ck_DSA_generate_key(DSA * dh);
DH * ck_d2i_DHparams(DH **a,unsigned char **pp, long length);
DH * ck_PEM_read_bio_DHparams(BIO *bp,DH **x,
                              int (*cb) (char *,int,int,void *),void *u);
char * ck_PEM_ASN1_read_bio(char *(*d2i) (),
                               const char *name,BIO *bp,char **x,
                               pem_password_cb *cb, void *u);

void ck_sk_GENERAL_NAME_free(STACK_OF(GENERAL_NAME) * sk);
GENERAL_NAME * ck_sk_GENERAL_NAME_value(const STACK_OF(GENERAL_NAME) * sk,
                                         int n);
int ck_sk_GENERAL_NAME_num(STACK_OF(GENERAL_NAME) * sk);
X509_REVOKED * ck_sk_X509_REVOKED_value(STACK_OF(X509_REVOKED) * sk,int n);
int ck_sk_X509_REVOKED_num(STACK_OF(X509_REVOKED) *sk);
long ck_ASN1_INTEGER_get(ASN1_INTEGER *a);
int ck_ASN1_STRING_cmp(ASN1_STRING *a, ASN1_STRING *b);
X509 *ck_PEM_read_X509(FILE *fp,char **x,int (*cb)(char *,int,int,void *),
                        void *u);
X509_EXTENSION * ck_X509_get_ext(X509 *x, int loc);
int ck_X509_get_ext_by_NID(X509 * x, int nid, int lastpos);
ASN1_INTEGER * ck_X509_get_serialNumber(X509 *x);
int ck_X509_cmp_current_time(ASN1_UTCTIME *s);
EVP_PKEY * ck_X509_get_pubkey(X509 *x);
int ck_X509_CRL_verify(X509_CRL *a, EVP_PKEY *r);
int ck_X509_STORE_get_by_subject(X509_STORE_CTX *vs,int type,X509_NAME *name,
                                  X509_OBJECT *ret);

X509_STORE_CTX * ck_X509_STORE_CTX_new();
void ck_X509_STORE_CTX_free(X509_STORE_CTX *ctx);
void ck_X509_STORE_CTX_cleanup(X509_STORE_CTX *ctx);
void ck_X509_STORE_CTX_set_error(X509_STORE_CTX *ctx,int s);
void ck_X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store,X509 *x509,
                             STACK_OF(X509) *chain);
void ck_X509_OBJECT_free_contents(X509_OBJECT *a);
int ck_X509V3_add_standard_extensions(void);
void ck_X509V3_EXT_cleanup(void);
void *ck_X509V3_EXT_d2i(X509_EXTENSION *ext);

X509_LOOKUP_METHOD * ck_X509_LOOKUP_hash_dir(void);
int ck_X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc,
                           long argl, char ** ret);
X509_LOOKUP_METHOD * ck_X509_LOOKUP_file(void);
X509_LOOKUP * ck(X509_STORE *v, X509_LOOKUP_METHOD *m);
X509_STORE * ck_X509_STORE_new(void);
void ck_X509_STORE_free(X509_STORE *);
int  ck_X509_STORE_load_locations(X509_STORE *,char *,char *);
int  ck_X509_STORE_set_default_paths(X509_STORE *);
int  ck_X509_subject_name_hash(X509 *);
int  ck_X509_issuer_name_hash(X509 *);
int  ck_X509_to_user(X509 *peer_cert, char *userid, int len);
int  ck_X509_userok(X509 *peer_cert, const char *userid);
void ck_CRYPTO_set_mem_functions(void *(*m)(unsigned int),
                                 void *(*r)(void *,unsigned int),
                                 void (*free_func)(void *));
void ck_CRYPTO_set_locking_callback(void (*func)(int mode,int type,
                                    const char *file,int line));
void ck_RAND_screen(void);
int  ck_CRYPTO_num_locks(void);

int  ck_RAND_status(void);
char * ck_RAND_file_name(char *,int);
int  ck_RAND_egd(char *);
int  ck_RAND_load_file(char *,int);
int  ck_RAND_write_file(char *);
void ck_RAND_seed(const void * buf, int num);
void ck_sk_free(STACK *);
char * ck_sk_value(const STACK *, int);
int ck_sk_num(const STACK *);

#ifdef ZLIB
COMP_METHOD * ck_COMP_zlib(void );
#endif /* ZLIB */
COMP_METHOD * ck_COMP_rle(void );

unsigned long ck_ERR_peek_error(void);
void ck_sk_pop_free(STACK *st, void (*func)(void *));
X509 * ck_PEM_read_bio_X509(BIO *bp, X509** x509, pem_password_cb *cb, void *u);
unsigned long ck_SSLeay(void);
const char * ck_SSLeay_version(int);

BIGNUM *ck_BN_new(void);
BIGNUM * ck_BN_bin2bn(const unsigned char *s,int len,BIGNUM *ret);
int     ck_BN_bn2bin(const BIGNUM *a, unsigned char *to);
char *  ck_BN_bn2hex(const BIGNUM *a);
int     ck_BN_num_bits(const BIGNUM *a);
int     ck_BN_cmp(const BIGNUM *a, const BIGNUM *b);
int     ck_BN_add_word(BIGNUM *a, BN_ULONG w);
int     ck_BN_lshift(BIGNUM *r, const BIGNUM *a, int n);
int     ck_BN_set_word(BIGNUM *a, BN_ULONG w);
int     ck_BN_dec2bn(BIGNUM **a, const char *str);
char *  ck_BN_bn2dec(const BIGNUM *a);
BIGNUM *ck_BN_copy(BIGNUM *a, const BIGNUM *b);
BN_CTX *ck_BN_CTX_new(void);
void    ck_BN_CTX_free(BN_CTX *c);
void    ck_BN_CTX_start(BN_CTX *ctx);
BIGNUM *ck_BN_CTX_get(BN_CTX *ctx);
void    ck_BN_CTX_end(BN_CTX *ctx);
int     ck_BN_div(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,BN_CTX *ctx);
int     ck_BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
const BIGNUM *ck_BN_value_one(void);
void    ck_BN_free(BIGNUM *a);
int     ck_BN_hex2bn(BIGNUM **a, const char *str);
int     ck_BN_is_bit_set(const BIGNUM *a, int n);
int     ck_BN_rand(BIGNUM *rnd, int bits, int top,int bottom);
int     ck_BN_set_word(BIGNUM *a, BN_ULONG w);
char *  ck_BN_bn2hex(const BIGNUM *a);
int     ck_BN_lshift(BIGNUM *r, const BIGNUM *a, int n);
int     ck_BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
int     ck_BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);
int     ck_BN_mod(BIGNUM *rem, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx);
BN_ULONG ck_BN_mod_word(const BIGNUM *a, BN_ULONG w);
int     ck_BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m, BN_CTX *ctx);
int     ck_BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,const BIGNUM *m,BN_CTX *ctx);
int     ck_BN_is_prime(const BIGNUM *p,int nchecks,void (*callback)(int,int,void *),BN_CTX *ctx,void *cb_arg);
int     ck_BN_ucmp(const BIGNUM *a, const BIGNUM *b);
int     ck_BN_mul_word(BIGNUM *a, BN_ULONG w);
int     ck_BN_rshift1(BIGNUM *r, const BIGNUM *a);
BIGNUM *ck_BN_dup(const BIGNUM *a);

int ck_MD5_Init(MD5_CTX *c);
int ck_MD5_Update(MD5_CTX *c, const void *data, unsigned long len);
int ck_MD5_Final(unsigned char *md, MD5_CTX *c);
unsigned char *ck_MD5(const unsigned char *d, unsigned long n, unsigned char *md);

int     ck_EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type);
int     ck_EVP_DigestUpdate(EVP_MD_CTX *ctx,const void *d,
                         unsigned int cnt);
int     ck_EVP_DigestFinal(EVP_MD_CTX *ctx,unsigned char *md,unsigned int *s);
const EVP_MD *ck_EVP_sha1(void);
const EVP_MD *ck_EVP_md5(void);
const EVP_CIPHER *ck_EVP_des_ede3_cbc(void);
const EVP_MD *ck_EVP_ripemd160(void);
const EVP_MD *ck_EVP_get_digestbyname(const char *name);
RSA *           ck_EVP_PKEY_get1_RSA(EVP_PKEY *pkey);
DSA *           ck_EVP_PKEY_get1_DSA(EVP_PKEY *pkey);
void            ck_EVP_PKEY_free(EVP_PKEY *pkey);

RSA *   ck_RSA_new(void);
int     ck_RSA_public_encrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding);
int     ck_RSA_private_encrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding);
int     ck_RSA_private_decrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding);
int     ck_RSA_public_decrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding);
int     ck_RSA_size(const RSA *);
int ck_RSA_sign(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigret, unsigned int *siglen, RSA *rsa);
int ck_RSA_verify(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigbuf, unsigned int siglen, RSA *rsa);

DSA *   ck_DSA_new(void);
DSA_SIG * ck_DSA_SIG_new(void);
void    ck_DSA_SIG_free(DSA_SIG *a);
DSA_SIG * ck_DSA_do_sign(const unsigned char *dgst,int dlen,DSA *dsa);
int     ck_DSA_do_verify(const unsigned char *dgst,int dgst_len,
                      DSA_SIG *sig,DSA *dsa);

void ck_HMAC_Init(HMAC_CTX *ctx, const void *key, int len,
               const EVP_MD *md);
void ck_HMAC_Update(HMAC_CTX *ctx, const unsigned char *data, int len);
void ck_HMAC_Final(HMAC_CTX *ctx, unsigned char *md, unsigned int *len);
void ck_HMAC_cleanup(HMAC_CTX *ctx);
void ck_HMAC_CTX_cleanup(HMAC_CTX *ctx);
void ck_RAND_add(const void *buf,int num,double entropy);
int  ck_RAND_bytes(unsigned char *buf,int num);
#ifdef CKOSSLC_C
#ifndef const_des_cblock
#define const_des_cblock des_cblock
#endif
int  ck_des_set_key(const_des_cblock *k,des_key_schedule s);
void ck_des_ncbc_encrypt(const unsigned char *input,unsigned char *output,
                      long length,des_key_schedule schedule,des_cblock *ivec,
                      int enc);
void ck_des_ede3_cbc_encrypt(const unsigned char *input,unsigned char *output,
                          long length,
                          des_key_schedule ks1,des_key_schedule ks2,
                          des_key_schedule ks3,des_cblock *ivec,int enc);
#endif /* CKOSSLC_C */
void ck_BF_set_key(BF_KEY *key, int len, const unsigned char *data);
void ck_BF_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
        const BF_KEY *schedule, unsigned char *ivec, int enc);
void ck_RC4_set_key(RC4_KEY *key, int len, const unsigned char *data);
void ck_RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata,
                unsigned char *outdata);
void ck_CAST_set_key(CAST_KEY *key, int len, const unsigned char *data);
void ck_CAST_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
                      CAST_KEY *ks, unsigned char *iv, int enc);
int  ck_DH_compute_key(unsigned char *key,const BIGNUM *pub_key,DH *dh);
int  ck_DH_size(const DH *dh);
const char *    ck_OBJ_nid2sn(int n);
int             ck_OBJ_create(const char *oid,const char *sn,const char *ln);
unsigned long ck_ERR_get_error(void );
void ck_ERR_load_crypto_strings(void);
void    ck_BN_clear_free(BIGNUM *a);
void    ck_BN_clear(BIGNUM *a);
void ck_CRYPTO_free(void *);

int ck_PEM_write_DSAPrivateKey(FILE *fp, char *x,
             const EVP_CIPHER *enc, unsigned char *kstr, int klen,
             pem_password_cb *callback, void *u);
int ck_PEM_write_RSAPrivateKey(FILE *fp, char *x,
             const EVP_CIPHER *enc, unsigned char *kstr, int klen,
             pem_password_cb *callback, void *u);
int ck_PEM_write_bio_X509(BIO *a, X509 *x);
EVP_PKEY *ck_PEM_read_PrivateKey(FILE *fp, EVP_PKEY **x, pem_password_cb *cb, void *u);

int ck_RSA_blinding_on(RSA *rsa, BN_CTX *ctx);
const EVP_CIPHER *ck_EVP_aes_256_cbc(void);
const EVP_CIPHER *ck_EVP_aes_192_cbc(void);
const EVP_CIPHER *ck_EVP_aes_128_cbc(void);
const EVP_CIPHER *ck_EVP_rc4(void);
const EVP_CIPHER *ck_EVP_cast5_cbc(void); 
const EVP_CIPHER *ck_EVP_bf_cbc(void);  
const EVP_CIPHER *ck_EVP_des_cbc(void);    
const EVP_CIPHER *ck_EVP_enc_null(void);   
int   ck_EVP_CipherInit(EVP_CIPHER_CTX *ctx,const EVP_CIPHER *cipher,
                       const unsigned char *key,const unsigned char *iv,
                       int enc);
void ck_EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *a);
int  ck_EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a);
int  ck_EVP_CIPHER_CTX_set_key_length(EVP_CIPHER_CTX *x, int keylen);
int  ck_EVP_CIPHER_CTX_key_length(const EVP_CIPHER_CTX *ctx);
int  ck_EVP_Cipher(EVP_CIPHER_CTX *c, unsigned char *out, 
		   const unsigned char *in, unsigned int inl);
int  ck_EVP_CIPHER_CTX_iv_length(const EVP_CIPHER_CTX *ctx);
void *ck_EVP_CIPHER_CTX_get_app_data(const EVP_CIPHER_CTX *ctx);
void ck_EVP_CIPHER_CTX_set_app_data(EVP_CIPHER_CTX *ctx, void *data);

const char *ck_X509_get_default_cert_dir_env(void);
const char *ck_X509_get_default_cert_file_env(void);
int  ck_EVP_MD_size(const EVP_MD *md);

void ck_SHA1_Init(SHA_CTX *c);
void ck_SHA1_Update(SHA_CTX *c, const void *data, unsigned long len);
void ck_SHA1_Final(unsigned char *md, SHA_CTX *c);
void ck_OpenSSL_add_all_ciphers(void);
void ck_OpenSSL_add_all_digests(void);
void ck_OPENSSL_add_all_algorithms_noconf(void);
void ck_OPENSSL_add_all_algorithms_conf(void);

extern int crypto_dll_loaded;
#ifdef NT
extern HINSTANCE hCRYPTO;
#else /* NT */
extern HMODULE hCRYPTO;
#endif /* NT */
#endif /* OS2    */
#endif /* CK_SSL */
#endif /* SSLDLL */
