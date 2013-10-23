/*
  C K O S S L C . C --  OpenSSL Interface for Kermit 95

  Copyright (C) 2000, 2004 Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.

  Copyright (C) 2006, 2007  Secure Endpoints Inc.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City
*/

#define CKOSSLC_C

#include "ckcdeb.h"
#define OPENSSL_ENABLE_OLD_DES_SUPPORT
#ifdef CK_SSL
#include "openssl/des.h"
#endif
#include "ck_ssl.h"

#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#include <os2.h>
#endif /* NT */
#include <stdarg.h>

#ifdef OS2
#ifdef CK_SSL
#ifdef SSLDLL

#ifdef NT
HINSTANCE hCRYPTO = NULL;
#else /* NT */
HMODULE hCRYPTO = NULL;
#endif /* NT */

int crypto_dll_loaded=0;

#ifdef NOT_KERMIT
static int deblog = 0;
#ifdef debug
#undef debug
#endif
#define debug(x,a,b,c)
#ifdef printf
#undef printf
#endif
static unsigned long startflags = 0;
#endif /* NOT_KERMIT */


void (*p_ERR_print_errors)(BIO *bp)=NULL;
void (*p_ERR_print_errors_fp)(FILE *fp)=NULL;
unsigned long (*p_ERR_get_error)(void)=NULL;
char *(*p_ERR_error_string)(unsigned long e,char *buf);

int (*p_BIO_printf)( BIO * bio, const char *, ... )=NULL;
long    (*p_BIO_ctrl)(BIO *bp,int cmd,long larg,char *parg)=NULL;
BIO *   (*p_BIO_new)(BIO_METHOD *type)=NULL;
BIO_METHOD *(*p_BIO_s_file)(void )=NULL;
BIO_METHOD *(*p_BIO_s_mem)(void )=NULL;
BIO_METHOD *(*p_BIO_s_null)(void )=NULL;
int     (*p_BIO_read)(BIO *b, char *data, int len)=NULL;
int     (*p_BIO_write)(BIO *b, const char *data, int len)=NULL;
int     (*p_BIO_free)(BIO *a)=NULL;
int     (*p_BIO_free_all)(BIO *a)=NULL;
BIO *   (*p_BIO_new_file)(char *filename, char *mode)=NULL;
X509_NAME * (*p_X509_get_issuer_name)(X509 *a)=NULL;
char * (*p_X509_verify_cert_error_string)(int)=NULL;
char *      (*p_X509_NAME_oneline)(X509_NAME *a,char *buf,int len)=NULL;
int     (*p_X509_NAME_print_ex)(BIO *out, X509_NAME *nm, int indent, unsigned long flags)=NULL;
int (*p_X509_NAME_get_text_by_NID)(X509_NAME *a,int NID,char *buf,int len)=NULL;
X509_NAME * (*p_X509_get_subject_name)(X509 *a)=NULL;
X509 *      (*p_X509_STORE_CTX_get_current_cert)(X509_STORE_CTX *ctx)=NULL;
int         (*p_X509_STORE_CTX_get_error)(X509_STORE_CTX *ctx)=NULL;
int         (*p_X509_STORE_CTX_get_error_depth)(X509_STORE_CTX *ctx)=NULL;
int         (*p_ASN1_TIME_print)( BIO * bio, ASN1_TIME *a)=NULL;
void        (*p_X509_free)(X509 *a)=NULL;
const char * (*p_X509_get_default_cert_dir)()=NULL;
int         (*p_X509_print_ex)(BIO *bio, X509 *cert, unsigned long, unsigned long)=NULL;

void    (*p_RSA_free)(RSA *r)=NULL;
RSA *   (*p_RSA_generate_key)(int bits, unsigned long e,
                               void(*callback)(int,int,char *),
                               char *cb_arg)=NULL;
DH *    (*p_DH_new)(void)=NULL;
void    (*p_DH_free)(DH *r)=NULL;
DH *    (*p_DH_generate_parameters)(int bits, unsigned long e,
                                     void(*callback)(int,int,char *),
                                     char *cb_arg)=NULL;
int     (*p_DH_generate_key)(DH * dh)=NULL;
void    (*p_DSA_free)(DSA *r)=NULL;
DSA *   (*p_DSA_generate_parameters)( int bits,
                                      unsigned char * seed,
                                      int seed_len,
                                      int *counter_ret,
                                      unsigned long *h_ret,
                                      void(*callback)(int,int,char *),
                                      char *cb_arg)=NULL;
int     (*p_DSA_generate_key)(DSA * dh)=NULL;
DH * (*p_d2i_DHparams)(DH **a,unsigned char **pp, long length)=NULL;
DH * (*p_PEM_read_bio_DHparams)(BIO *bp,DH **x,int (*cb)(),void *u)=NULL;
char * (*p_PEM_ASN1_read_bio)(char *(*d2i)(),
                             const char *name,BIO *bp,char **x,
                             pem_password_cb *cb, void *u)=NULL;
BIGNUM * (*p_BN_bin2bn)(const unsigned char *s,int len,BIGNUM *ret)=NULL;
void (*p_sk_GENERAL_NAME_free)(STACK_OF(GENERAL_NAME) * sk)=NULL;
GENERAL_NAME * (*p_sk_GENERAL_NAME_value)(const STACK_OF(GENERAL_NAME) * sk,int n)=NULL;
int (*p_sk_GENERAL_NAME_num)(STACK_OF(GENERAL_NAME) * sk)=NULL;
X509_REVOKED * (*p_sk_X509_REVOKED_value)(STACK_OF(X509_REVOKED) * sk,int n)=NULL;
int (*p_sk_X509_REVOKED_num)(STACK_OF(X509_REVOKED) *sk)=NULL;
long (*p_ASN1_INTEGER_get)(ASN1_INTEGER *a)=NULL;
int (*p_ASN1_STRING_cmp)(ASN1_STRING *a, ASN1_STRING *b)=NULL;
X509 *(*p_PEM_read_X509)(FILE *fp,char **x,int (*cb)(char *,int,int,void *),void *u)=NULL;
X509_EXTENSION * (*p_X509_get_ext)(X509 *x, int loc)=NULL;
int (*p_X509_get_ext_by_NID)(X509 * x, int nid, int lastpos)=NULL;
ASN1_INTEGER * (*p_X509_get_serialNumber)(X509 *x)=NULL;
int (*p_X509_cmp_current_time)(ASN1_UTCTIME *s)=NULL;
EVP_PKEY * (*p_X509_get_pubkey)(X509 *x)=NULL;
int (*p_X509_CRL_verify)(X509_CRL *a, EVP_PKEY *r)=NULL;
int (*p_X509_STORE_get_by_subject)(X509_STORE_CTX *vs,int type,X509_NAME *name,X509_OBJECT *ret)=NULL;
X509_STORE_CTX * (*p_X509_STORE_CTX_new)()=NULL;
void (*p_X509_STORE_CTX_free)(X509_STORE_CTX *ctx)=NULL;
void (*p_X509_STORE_CTX_cleanup)(X509_STORE_CTX *ctx)=NULL;
void (*p_X509_STORE_CTX_set_error)(X509_STORE_CTX *ctx,int s)=NULL;
void (*p_X509_STORE_CTX_init)(X509_STORE_CTX *ctx, X509_STORE *store,X509 *x509, STACK_OF(X509) *chain)=NULL;
void (*p_X509_OBJECT_free_contents)(X509_OBJECT *a)=NULL;
int (*p_X509V3_add_standard_extensions)(void)=NULL;
void (*p_X509V3_EXT_cleanup)(void)=NULL;
void *(*p_X509V3_EXT_d2i)(X509_EXTENSION *ext)=NULL;

X509_LOOKUP_METHOD * (*p_X509_LOOKUP_hash_dir)(void)=NULL;
int (*p_X509_LOOKUP_ctrl)(X509_LOOKUP *ctx, int cmd, const char *argc,
                           long argl, char ** ret)=NULL;
X509_LOOKUP_METHOD * (*p_X509_LOOKUP_file)(void)=NULL;
X509_LOOKUP * (*p_X509_STORE_add_lookup)(X509_STORE *v, X509_LOOKUP_METHOD *m)=NULL;
X509_STORE * (*p_X509_STORE_new)(void)=NULL;
void (*p_X509_STORE_free)(X509_STORE *)=NULL;
int  (*p_X509_STORE_load_locations)(X509_STORE *,char *,char *)=NULL;
int  (*p_X509_STORE_set_default_paths)(X509_STORE *)=NULL;
int  (*p_ASN1_INTEGER_cmp)(ASN1_INTEGER * x, ASN1_INTEGER *y)=NULL;
int  (*p_X509_subject_name_hash)(X509 *)=NULL;
int  (*p_X509_issuer_name_hash)(X509 *)=NULL;
void (*p_CRYPTO_set_mem_functions)(void *(*m)(unsigned int),
                                   void *(*r)(void *,unsigned int),
                                   void (*free_func)(void *))=NULL;
void (*p_CRYPTO_set_locking_callback)(void (*func)(int mode,int type,
                                      const char *file,int line))=NULL;
void (*p_CRYPTO_set_dynlock_create_callback)(struct CRYPTO_dynlock_value *(*func)
					     (const char *file, int line))=NULL;
void (*p_CRYPTO_set_dynlock_lock_callback)(void (*func)(int mode,
	struct CRYPTO_dynlock_value *l, const char *file, int line))=NULL;
void (*p_CRYPTO_set_dynlock_destroy_callback)(void (*func)
	(struct CRYPTO_dynlock_value *l, const char *file, int line))=NULL;

void (*p_RAND_screen)(void)=NULL;
int  (*p_CRYPTO_num_locks)(void)=NULL;
int  (*p_RAND_status)(void)=NULL;
char * (*p_RAND_file_name)(char *,int)=NULL;
int  (*p_RAND_egd)(char *)=NULL;
int  (*p_RAND_load_file)(char *,int)=NULL;
int  (*p_RAND_write_file)(char *)=NULL;
void (*p_RAND_seed)(const void * buf, int num)=NULL;
void (*p_sk_free)(STACK *)=NULL;
char * (*p_sk_value)(const STACK *, int)=NULL;
int (*p_sk_num)(const STACK *)=NULL;

#ifdef ZLIB
COMP_METHOD * (*p_COMP_zlib)(void )=NULL;
#endif /* ZLIB */
COMP_METHOD * (*p_COMP_rle)(void )=NULL;
unsigned long (*p_ERR_peek_error)(void)=NULL;
void (*p_sk_pop_free)(STACK *st, void (*func)(void *))=NULL;
X509 * (*p_PEM_read_bio_X509)(BIO *bp, X509** x509, pem_password_cb *cb, void *u)=NULL;
unsigned long (*p_SSLeay)(void)=NULL;
const char * (*p_SSLeay_version)(int)=NULL;

BIGNUM *(*p_BN_new)(void)=NULL;
int     (*p_BN_bn2bin)(const BIGNUM *a, unsigned char *to)=NULL;
int     (*p_BN_num_bits)(const BIGNUM *a)=NULL;
int     (*p_BN_cmp)(const BIGNUM *a, const BIGNUM *b)=NULL;
int     (*p_BN_add_word)(BIGNUM *a, BN_ULONG w)=NULL;
int     (*p_BN_lshift)(BIGNUM *r, const BIGNUM *a, int n)=NULL;
int     (*p_BN_set_word)(BIGNUM *a, BN_ULONG w)=NULL;
int     (*p_BN_dec2bn)(BIGNUM **a, const char *str)=NULL;
char *  (*p_BN_bn2dec)(const BIGNUM *a)=NULL;
BIGNUM *(*p_BN_copy)(BIGNUM *a, const BIGNUM *b)=NULL;
void    (*p_BN_CTX_free)(BN_CTX *c)=NULL;
void    (*p_BN_CTX_start)(BN_CTX *ctx)=NULL;
BIGNUM *(*p_BN_CTX_get)(BN_CTX *ctx)=NULL;
void    (*p_BN_CTX_end)(BN_CTX *ctx)=NULL;
int     (*p_BN_div)(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,BN_CTX *ctx)=NULL;
int     (*p_BN_sub)(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)=NULL;
const BIGNUM *(*p_BN_value_one)(void)=NULL;
BN_CTX *(*p_BN_CTX_new)(void)=NULL;
void    (*p_BN_free)(BIGNUM *a)=NULL;
int     (*p_BN_hex2bn)(BIGNUM **a, const char *str)=NULL;
int     (*p_BN_is_bit_set)(const BIGNUM *a, int n)=NULL;
int     (*p_BN_rand)(BIGNUM *rnd, int bits, int top,int bottom)=NULL;
char *  (*p_BN_bn2hex)(const BIGNUM *a)=NULL;
int     (*p_BN_add)(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)=NULL;
int     (*p_BN_mul)(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx)=NULL;
int     (*p_BN_mod)(BIGNUM *rem, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)=NULL;
BN_ULONG (*p_BN_mod_word)(const BIGNUM *a, BN_ULONG w)=NULL;
int     (*p_BN_mod_mul)(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)=NULL;
int     (*p_BN_mod_exp)(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,const BIGNUM *m,BN_CTX *ctx)=NULL;
int     (*p_BN_is_prime)(const BIGNUM *p,int nchecks,void (*callback)(int,int,void *),BN_CTX *ctx,void *cb_arg)=NULL;
int     (*p_BN_ucmp)(const BIGNUM *a, const BIGNUM *b)=NULL;
int     (*p_BN_mul_word)(BIGNUM *a, BN_ULONG w)=NULL;
int     (*p_BN_rshift1)(BIGNUM *r, const BIGNUM *a)=NULL;
BIGNUM * (*p_BN_dup)(const BIGNUM *a)=NULL;

int (*p_MD5_Init)(MD5_CTX *c)=NULL;
int (*p_MD5_Update)(MD5_CTX *c, const void *data, unsigned long len)=NULL;
int (*p_MD5_Final)(unsigned char *md, MD5_CTX *c)=NULL;
unsigned char *(*p_MD5)(const unsigned char *d, unsigned long n, unsigned char *md)=NULL;

int     (*p_EVP_DigestInit)(EVP_MD_CTX *ctx, const EVP_MD *type)=NULL;
int     (*p_EVP_DigestUpdate)(EVP_MD_CTX *ctx,const void *d,
                             unsigned int cnt)=NULL;
int     (*p_EVP_DigestFinal)(EVP_MD_CTX *ctx,unsigned char *md,unsigned int *s)=NULL;
const EVP_MD *(*p_EVP_sha1)(void)=NULL;
const EVP_MD *(*p_EVP_md5)(void)=NULL;
const EVP_CIPHER *(*p_EVP_des_ede3_cbc)(void)=NULL;
const EVP_MD *(*p_EVP_ripemd160)(void)=NULL;
const EVP_MD *(*p_EVP_get_digestbyname)(const char *name)=NULL;
RSA *           (*p_EVP_PKEY_get1_RSA)(EVP_PKEY *pkey)=NULL;
DSA *           (*p_EVP_PKEY_get1_DSA)(EVP_PKEY *pkey)=NULL;
void            (*p_EVP_PKEY_free)(EVP_PKEY *pkey)=NULL;

RSA *   (*p_RSA_new)(void)=NULL;
int     (*p_RSA_public_encrypt)(int flen, const unsigned char *from,
                               unsigned char *to, RSA *rsa,int padding)=NULL;
int     (*p_RSA_private_encrypt)(int flen, const unsigned char *from,
                                unsigned char *to, RSA *rsa,int padding)=NULL;
int     (*p_RSA_private_decrypt)(int flen, const unsigned char *from,
                                      unsigned char *to, RSA *rsa,int padding)=NULL;
int     (*p_RSA_public_decrypt)(int flen, const unsigned char *from,
                               unsigned char *to, RSA *rsa,int padding)=NULL;
int     (*p_RSA_size)(const RSA *)=NULL;
int (*p_RSA_sign)(int type, const unsigned char *m, unsigned int m_len,
                 unsigned char *sigret, unsigned int *siglen, RSA *rsa)=NULL;
int (*p_RSA_verify)(int type, const unsigned char *m, unsigned int m_len,
                   unsigned char *sigbuf, unsigned int siglen, RSA *rsa)=NULL;

DSA *   (*p_DSA_new)(void)=NULL;
DSA_SIG * (*p_DSA_SIG_new)(void)=NULL;
void    (*p_DSA_SIG_free)(DSA_SIG *a)=NULL;
DSA_SIG * (*p_DSA_do_sign)(const unsigned char *dgst,int dlen,DSA *dsa)=NULL;
int     (*p_DSA_do_verify)(const unsigned char *dgst,int dgst_len,
                          DSA_SIG *sig,DSA *dsa)=NULL;

void (*p_HMAC_Init)(HMAC_CTX *ctx, const void *key, int len,
                   const EVP_MD *md)=NULL;
void (*p_HMAC_Update)(HMAC_CTX *ctx, const unsigned char *data, int len)=NULL;
void (*p_HMAC_Final)(HMAC_CTX *ctx, unsigned char *md, unsigned int *len)=NULL;
void (*p_HMAC_cleanup)(HMAC_CTX *ctx)=NULL;
void (*p_HMAC_CTX_cleanup)(HMAC_CTX *ctx)=NULL;
void (*p_RAND_add)(const void *buf,int num,double entropy)=NULL;
int  (*p_RAND_bytes)(unsigned char *buf,int num)=NULL;
int  (*p_des_set_key)(const_des_cblock *k,des_key_schedule s)=NULL;
void (*p_des_ncbc_encrypt)(const unsigned char *input,unsigned char *output,
                      long length,des_key_schedule schedule,des_cblock *ivec,
                      int enc)=NULL;
void (*p_des_ede3_cbc_encrypt)(const unsigned char *input,unsigned char *output,
                          long length,
                          des_key_schedule ks1,des_key_schedule ks2,
                          des_key_schedule ks3,des_cblock *ivec,int enc)=NULL;
void (*p_BF_set_key)(BF_KEY *key, int len, const unsigned char *data)=NULL;
void (*p_BF_cbc_encrypt)(const unsigned char *in, unsigned char *out, long length,
        const BF_KEY *schedule, unsigned char *ivec, int enc)=NULL;
void (*p_RC4_set_key)(RC4_KEY *key, int len, const unsigned char *data)=NULL;
void (*p_RC4)(RC4_KEY *key, unsigned long len, const unsigned char *indata,
                unsigned char *outdata)=NULL;
void (*p_CAST_set_key)(CAST_KEY *key, int len, const unsigned char *data)=NULL;
void (*p_CAST_cbc_encrypt)(const unsigned char *in, unsigned char *out, long length,
                      CAST_KEY *ks, unsigned char *iv, int enc)=NULL;
int  (*p_DH_compute_key)(unsigned char *key,const BIGNUM *pub_key,DH *dh)=NULL;
int  (*p_DH_size)(const DH *dh)=NULL;
const char *    (*p_OBJ_nid2sn)(int n)=NULL;
int (*p_OBJ_create)(const char *oid,const char *sn,const char *ln)=NULL;
void (*p_ERR_load_crypto_strings)(void)=NULL;
void    (*p_BN_clear_free)(BIGNUM *a)=NULL;
void    (*p_BN_clear)(BIGNUM *a)=NULL;
void (*p_CRYPTO_free)(void *)=NULL;
int (*p_PEM_write_DSAPrivateKey)(FILE *fp, char *x,
                                const EVP_CIPHER *enc, unsigned char *kstr, int klen,
                                pem_password_cb *callback, void *u)=NULL;
int (*p_PEM_write_RSAPrivateKey)(FILE *fp, char *x,
                                const EVP_CIPHER *enc, unsigned char *kstr, int klen,
                                pem_password_cb *callback, void *u)=NULL;
int (*p_PEM_write_bio_X509)(BIO *a, X509 *x)=NULL;
EVP_PKEY *(*p_PEM_read_PrivateKey)(FILE *fp, EVP_PKEY **x, pem_password_cb *cb, void *u)=NULL;

void (*p_SHA1_Init)(SHA_CTX *c)=NULL;
void (*p_SHA1_Update)(SHA_CTX *c, const void *data, unsigned long len)=NULL;
void (*p_SHA1_Final)(unsigned char *md, SHA_CTX *c)=NULL;

int               (*p_RSA_blinding_on)(RSA *rsa, BN_CTX *ctx)=NULL;
const EVP_CIPHER *(*p_EVP_aes_256_cbc)(void)=NULL;
const EVP_CIPHER *(*p_EVP_aes_192_cbc)(void)=NULL;
const EVP_CIPHER *(*p_EVP_aes_128_cbc)(void)=NULL;
const EVP_CIPHER *(*p_EVP_rc4)(void)=NULL;
const EVP_CIPHER *(*p_EVP_cast5_cbc)(void)=NULL; 
const EVP_CIPHER *(*p_EVP_bf_cbc)(void)=NULL;  
const EVP_CIPHER *(*p_EVP_des_cbc)(void)=NULL;    
const EVP_CIPHER *(*p_EVP_enc_null)(void)=NULL;   
int   (*p_EVP_CipherInit)(EVP_CIPHER_CTX *ctx,const EVP_CIPHER *cipher,
                       const unsigned char *key,const unsigned char *iv,
                       int enc)=NULL;
void (*p_EVP_CIPHER_CTX_init)(EVP_CIPHER_CTX *a)=NULL;
int  (*p_EVP_CIPHER_CTX_cleanup)(EVP_CIPHER_CTX *a)=NULL;
int  (*p_EVP_CIPHER_CTX_set_key_length)(EVP_CIPHER_CTX *x, int keylen)=NULL;
int  (*p_EVP_CIPHER_CTX_key_length)(const EVP_CIPHER_CTX *ctx)=NULL;
int  (*p_EVP_Cipher)(EVP_CIPHER_CTX *c, unsigned char *out, 
		    const unsigned char *in, unsigned int inl)=NULL;
int  (*p_EVP_CIPHER_CTX_iv_length)(const EVP_CIPHER_CTX *ctx)=NULL;
void *(*p_EVP_CIPHER_CTX_get_app_data)(const EVP_CIPHER_CTX *ctx)=NULL;
void (*p_EVP_CIPHER_CTX_set_app_data)(EVP_CIPHER_CTX *ctx, void *data)=NULL;


const char *(*p_X509_get_default_cert_dir_env)(void)=NULL;
const char *(*p_X509_get_default_cert_file_env)(void)=NULL;

void (*p_OpenSSL_add_all_ciphers)(void)=NULL;
void (*p_OpenSSL_add_all_digests)(void)=NULL;
void (*p_OPENSSL_add_all_algorithms_noconf)(void)=NULL;
void (*p_OPENSSL_add_all_algorithms_conf)(void)=NULL;
int  (*p_EVP_MD_size)(const EVP_MD *md) = NULL;

void
ck_ERR_print_errors(BIO *bp)
{
    if ( p_ERR_print_errors )
        p_ERR_print_errors(bp);
}

void
ck_ERR_print_errors_fp(FILE *fp)
{
    if ( p_ERR_print_errors_fp )
        p_ERR_print_errors_fp(fp);
}

unsigned long
ck_ERR_get_error(void)
{
    if ( p_ERR_get_error )
        return p_ERR_get_error();
    return 0;
}

char *
ck_ERR_error_string(unsigned long e,char *buf)
{
    if ( p_ERR_error_string )
        return p_ERR_error_string(e,buf);
    else
        return "";
}

static char bioprtfstr[1024];

int
ck_BIO_printf(BIO * bio, const char *format, ...)
{
    int rc=0;
    va_list ap;
    if ( p_BIO_printf && bio != NULL) {
        va_start(ap, format);
        rc = vsprintf( bioprtfstr, format, ap ) ;
        rc = p_BIO_printf(bio,bioprtfstr);
        va_end(ap);
    }
    return (rc);
}

long
ck_BIO_ctrl(BIO *bp,int cmd,long larg,char *parg)
{
    if ( p_BIO_ctrl )
        return p_BIO_ctrl(bp,cmd,larg,parg);
    return(0);
}

BIO *
ck_BIO_new(BIO_METHOD *type)
{
    if ( p_BIO_new )
        return p_BIO_new(type);
    return NULL;
}

BIO_METHOD *
ck_BIO_s_file(void )
{
    if ( p_BIO_s_file )
        return p_BIO_s_file();
    return NULL;
}

BIO_METHOD *
ck_BIO_s_mem(void )
{
    if ( p_BIO_s_mem )
        return p_BIO_s_mem();
    return NULL;
}

BIO_METHOD *
ck_BIO_s_null(void )
{
    if ( p_BIO_s_null )
        return p_BIO_s_null();
    return NULL;
}

int
ck_BIO_read(BIO *b, char *data, int len)
{
    if ( p_BIO_read )
        return(p_BIO_read(b,data,len));
    else
        return(-1);
}

int
ck_BIO_write(BIO *b, const char *data, int len)
{
    if ( p_BIO_write )
        return(p_BIO_write(b,data,len));
    else
        return(-1);
}

int
ck_BIO_free(BIO *a)
{
    if ( p_BIO_free )
        return(p_BIO_free(a));
    else
        return(-1);
}

int
ck_BIO_free_all(BIO *a)
{
    if ( p_BIO_free_all )
        return(p_BIO_free_all(a));
    else
        return(-1);
}

BIO *
ck_BIO_new_file(char *filename, char *mode)
{
    if ( p_BIO_new_file )
        return(p_BIO_new_file(filename,mode));
    else
        return(NULL);
}

X509_NAME *
ck_X509_get_issuer_name(X509 *a)
{
    if ( p_X509_get_issuer_name )
        return p_X509_get_issuer_name(a);
    return NULL;
}

int
ck_X509_print_ex(BIO *bio, X509 *cert, unsigned long cert_ext, unsigned long name_ext)
{
    if ( p_X509_print_ex )
        return p_X509_print_ex(bio,cert,cert_ext,name_ext);
    return 0;
}

char *
ck_X509_verify_cert_error_string(int error)
{
    if ( p_X509_verify_cert_error_string )
        return p_X509_verify_cert_error_string(error);
    return "<unable to determine error string>";
}

char *
ck_X509_NAME_oneline(X509_NAME *a,char *buf,int len)

{
    if ( p_X509_NAME_oneline )
        return p_X509_NAME_oneline(a,buf,len);
    return "";
}

int     
ck_X509_NAME_print_ex(BIO *out, X509_NAME *nm, int indent, unsigned long flags)
{
    if ( p_X509_NAME_print_ex )
        return p_X509_NAME_print_ex(out, nm, indent, flags);
    return 0;
}

int
ck_X509_NAME_get_text_by_NID(X509_NAME *a,int NID,char *buf,int len)

{
    if ( p_X509_NAME_get_text_by_NID )
        return p_X509_NAME_get_text_by_NID(a,NID,buf,len);
    return(0);
}

X509_NAME *
ck_X509_get_subject_name(X509 *a)
{
    if ( p_X509_get_subject_name )
        return p_X509_get_subject_name(a);
    return NULL;
}

X509_STORE_CTX *
ck_X509_STORE_CTX_new()
{
    if ( p_X509_STORE_CTX_new )
        return(p_X509_STORE_CTX_new());
    else
        return((X509_STORE_CTX *)malloc(sizeof(X509_STORE_CTX)));
}

void
ck_X509_STORE_CTX_free(X509_STORE_CTX *ctx)
{
    if ( p_X509_STORE_CTX_free )
        p_X509_STORE_CTX_free(ctx);
    else if (ctx)
        free(ctx);
}

X509 *
ck_X509_STORE_CTX_get_current_cert(X509_STORE_CTX *ctx)
{
    if ( p_X509_STORE_CTX_get_current_cert )
        return p_X509_STORE_CTX_get_current_cert(ctx);
    return NULL;
}

int
ck_X509_STORE_CTX_get_error(X509_STORE_CTX *ctx)
{
    if ( p_X509_STORE_CTX_get_error )
        return p_X509_STORE_CTX_get_error(ctx);
    return -1;
}

int
ck_X509_STORE_CTX_get_error_depth(X509_STORE_CTX *ctx)
{
    if ( p_X509_STORE_CTX_get_error_depth )
        return p_X509_STORE_CTX_get_error_depth(ctx);
    return -1;
}

int
ck_ASN1_TIME_print( BIO * bio, ASN1_TIME *a)
{
    if ( p_ASN1_TIME_print )
        return p_ASN1_TIME_print(bio,a);
    return(-1);
}

const char *
ck_X509_get_default_cert_dir()
{
    if ( p_X509_get_default_cert_dir )
        return p_X509_get_default_cert_dir();
    return NULL;
}

void
ck_X509_free(X509 *a)
{
    if ( p_X509_free )
        p_X509_free(a);
}

void
ck_RSA_free (RSA *r)
{
    if ( p_RSA_free )
        p_RSA_free(r);
}

RSA *
ck_RSA_generate_key(int bits, unsigned long e,
                     void(*callback)(int,int,char *),
                     char *cb_arg)
{
    if ( p_RSA_generate_key )
        return p_RSA_generate_key(bits,e,callback,cb_arg);
    return NULL;
}

DH *
ck_DH_new(void)
{
    if ( p_DH_new )
        return(p_DH_new());
    else
        return(NULL);
}

void
ck_DH_free (DH *r)
{
    if ( p_DH_free )
        p_DH_free(r);
}

DH *
ck_DH_generate_parameters(int bits, unsigned long e,
                           void(*callback)(int,int,char *),
                           char *cb_arg)
{
    if ( p_DH_generate_parameters )
        return p_DH_generate_parameters(bits,e,callback,cb_arg);
    return NULL;
}

int
ck_DH_generate_key(DH * dh) {
    if ( p_DH_generate_key )
        return p_DH_generate_key(dh);
    else
        return(-1);
}

void
ck_DSA_free (DSA *dsa)
{
    if ( p_DSA_free )
        p_DSA_free(dsa);
}

DSA *
ck_DSA_generate_parameters( int bits,
                            unsigned char * seed,
                            int seed_len,
                            int *counter_ret,
                            unsigned long *h_ret,
                            void(*callback)(int,int,char *),
                            char *cb_arg)
{
    if ( p_DSA_generate_parameters )
        return p_DSA_generate_parameters(bits,seed,seed_len,
                                          counter_ret, h_ret,
                                          callback,cb_arg);
    return NULL;
}

int
ck_DSA_generate_key(DSA * dsa) {
    if ( p_DSA_generate_key )
        return p_DSA_generate_key(dsa);
    else
        return(-1);
}

DH *
ck_d2i_DHparams(DH **a,unsigned char **pp, long length)
{
    if ( p_d2i_DHparams )
        return(p_d2i_DHparams(a,pp,length));
    else
        return(NULL);
}

char *
ck_PEM_ASN1_read_bio(char *(*d2i)(),
                      const char *name,BIO *bp,char **x,
                      pem_password_cb *cb, void *u)
{
    if ( p_PEM_ASN1_read_bio )
        return(p_PEM_ASN1_read_bio(d2i,name,bp,x,cb,u));
    else
        return(NULL);
}

DH *
ck_PEM_read_bio_DHparams(BIO *bp,DH **x,int (*cb)(char *,int,int,void *),void *u)
{
    return((DH *)ck_PEM_ASN1_read_bio( (char *(*)())ck_d2i_DHparams,
                                    PEM_STRING_DHPARAMS,
                                    bp,(char **)x,cb,u));
}

BIGNUM *
ck_BN_bin2bn(const unsigned char *s,int len,BIGNUM *ret)
{
    if ( p_BN_bin2bn )
        return(p_BN_bin2bn(s,len,ret));
    else
        return(NULL);
}

void
ck_sk_GENERAL_NAME_free(STACK_OF(GENERAL_NAME) * sk)
{
    if ( p_sk_GENERAL_NAME_free )
        p_sk_GENERAL_NAME_free(sk);
}

GENERAL_NAME *
ck_sk_GENERAL_NAME_value(const STACK_OF(GENERAL_NAME) * sk,int n)
{
    if ( p_sk_GENERAL_NAME_value )
        return(p_sk_GENERAL_NAME_value(sk,n));
    else
        return(NULL);
}

int
ck_sk_GENERAL_NAME_num(STACK_OF(GENERAL_NAME) * sk)
{
    if ( p_sk_GENERAL_NAME_num )
        return(p_sk_GENERAL_NAME_num(sk));
    else
        return(0);
}

X509_REVOKED *
ck_sk_X509_REVOKED_value(STACK_OF(X509_REVOKED) * sk,int n)
{
    if ( p_sk_X509_REVOKED_value )
        return(p_sk_X509_REVOKED_value(sk,n));
    else
        return(NULL);
}

int
ck_sk_X509_REVOKED_num(STACK_OF(X509_REVOKED) *sk)
{
    if ( p_sk_X509_REVOKED_num )
        return(p_sk_X509_REVOKED_num(sk));
    else
        return(0);
}

long
ck_ASN1_INTEGER_get(ASN1_INTEGER *a)
{
    if ( p_ASN1_INTEGER_get )
        return(p_ASN1_INTEGER_get(a));
    else
        return(0);
}

int
ck_ASN1_STRING_cmp(ASN1_STRING *a, ASN1_STRING *b)
{
    if ( p_ASN1_STRING_cmp )
        return(p_ASN1_STRING_cmp(a,b));
    else
        return(-1);
}

X509 *
ck_PEM_read_X509(FILE *fp,char **x,int (*cb)(char *,int,int,void *),void *u)
{
    if ( p_PEM_read_X509 )
        return(p_PEM_read_X509(fp,x,cb,u));
    else
        return(NULL);
}

X509_EXTENSION *
ck_X509_get_ext(X509 *x, int loc)
{
    if ( p_X509_get_ext )
        return(p_X509_get_ext(x,loc));
    else
        return(NULL);
}

int
ck_X509_get_ext_by_NID(X509 * x, int nid, int lastpos)
{
    if ( p_X509_get_ext_by_NID )
        return(p_X509_get_ext_by_NID(x,nid,lastpos));
    else
        return(0);
}

ASN1_INTEGER *
ck_X509_get_serialNumber(X509 *x)
{
    if ( p_X509_get_serialNumber )
        return(p_X509_get_serialNumber(x));
    else
        return(NULL);
}

int
ck_X509_cmp_current_time(ASN1_UTCTIME *s)
{
    if ( p_X509_cmp_current_time )
        return(p_X509_cmp_current_time(s));
    else
        return(0);
}

EVP_PKEY *
ck_X509_get_pubkey(X509 *x)
{
    if ( p_X509_get_pubkey )
        return(p_X509_get_pubkey(x));
    else
        return(NULL);
}

int
ck_X509_CRL_verify(X509_CRL *a, EVP_PKEY *r)
{
    if ( p_X509_CRL_verify )
        return(p_X509_CRL_verify(a,r));
    else
        return(0);
}

int
ck_X509_STORE_get_by_subject(X509_STORE_CTX *vs,int type,X509_NAME *name,X509_OBJECT *ret)
{
    if ( p_X509_STORE_get_by_subject )
        return(p_X509_STORE_get_by_subject(vs,type,name,ret));
    else
        return(0);
}

void
ck_X509_STORE_CTX_cleanup(X509_STORE_CTX *ctx)
{
    if ( p_X509_STORE_CTX_cleanup )
        p_X509_STORE_CTX_cleanup(ctx);
}

void
ck_X509_STORE_CTX_set_error(X509_STORE_CTX *ctx,int s)
{
    if ( p_X509_STORE_CTX_set_error )
        p_X509_STORE_CTX_set_error(ctx,s);
}

void
ck_X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store,X509 *x509, STACK_OF(X509) *chain)
{
    if ( p_X509_STORE_CTX_init )
        p_X509_STORE_CTX_init(ctx,store,x509,chain);
}

void
ck_X509_OBJECT_free_contents(X509_OBJECT *a)
{
    if ( p_X509_OBJECT_free_contents )
        p_X509_OBJECT_free_contents(a);
}

int
ck_X509V3_add_standard_extensions(void)
{
    if ( p_X509V3_add_standard_extensions )
        return(p_X509V3_add_standard_extensions());
    else
        return(0);
}

void
ck_X509V3_EXT_cleanup(void)
{
    if ( p_X509V3_EXT_cleanup )
        p_X509V3_EXT_cleanup();
}

void *
ck_X509V3_EXT_d2i(X509_EXTENSION *ext)
{
    if ( p_X509V3_EXT_d2i )
        return(p_X509V3_EXT_d2i(ext));
    else
        return(NULL);
}

X509_LOOKUP_METHOD *
ck_X509_LOOKUP_hash_dir(void)
{
    if ( p_X509_LOOKUP_hash_dir )
        return(p_X509_LOOKUP_hash_dir());
    else
        return(NULL);
}

int
ck_X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc,
                    long argl, char ** ret)
{
    if ( p_X509_LOOKUP_ctrl )
        return(p_X509_LOOKUP_ctrl(ctx,cmd,argc,argl,ret));
    else
        return(0);
}

X509_LOOKUP_METHOD *
ck_X509_LOOKUP_file(void)
{
    if ( p_X509_LOOKUP_file )
        return(p_X509_LOOKUP_file());
    else
        return(NULL);
}

X509_LOOKUP *
ck_X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m)
{
    if ( p_X509_STORE_add_lookup )
        return(p_X509_STORE_add_lookup(v,m));
    else
        return(NULL);
}

X509_STORE *
ck_X509_STORE_new(void)
{
    if ( p_X509_STORE_new )
        return(p_X509_STORE_new());
    else
        return(NULL);
}

void
ck_X509_STORE_free(X509_STORE * s)
{
    if ( p_X509_STORE_free )
        p_X509_STORE_free(s);
}

int
ck_X509_STORE_load_locations(X509_STORE * s,char * f,char * d)
{
    if ( p_X509_STORE_load_locations )
        return(p_X509_STORE_load_locations(s,f,d));
    else
        return(0);
}

int
ck_X509_STORE_set_default_paths(X509_STORE * s)
{
    if ( p_X509_STORE_set_default_paths )
        return(p_X509_STORE_set_default_paths(s));
    else
        return(0);
}

int
ck_ASN1_INTEGER_cmp(ASN1_INTEGER * x, ASN1_INTEGER *y)
{
    if ( p_ASN1_INTEGER_cmp )
        return(p_ASN1_INTEGER_cmp(x,y));
    else if ( p_ASN1_STRING_cmp )
        return(p_ASN1_STRING_cmp(x,y));
    else return(-1);
}

int
ck_X509_subject_name_hash(X509 * x)
{
    if ( p_X509_subject_name_hash )
        return(p_X509_subject_name_hash(x));
    return(0);
}

int
ck_X509_issuer_name_hash(X509 * x)
{
    if ( p_X509_issuer_name_hash )
        return(p_X509_issuer_name_hash(x));
    return(0);
}

void
ck_CRYPTO_set_mem_functions(void *(*m)(unsigned int),
                            void *(*r)(void *,unsigned int),
                            void (*free_func)(void *))
{
    if ( p_CRYPTO_set_mem_functions )
        p_CRYPTO_set_mem_functions(m,r,free_func);
}

void
ck_RAND_screen(void)
{
    if ( p_RAND_screen )
        p_RAND_screen();
}

void
ck_CRYPTO_set_locking_callback(void (*func)(int mode,int type,
                               const char *file,int line))
{
    if ( p_CRYPTO_set_locking_callback )
        p_CRYPTO_set_locking_callback(func);
}

void 
ck_CRYPTO_set_dynlock_create_callback(struct CRYPTO_dynlock_value *(*func)(const char *file, int line))
{
    if ( p_CRYPTO_set_dynlock_create_callback )
	p_CRYPTO_set_dynlock_create_callback(func);
}

void 
ck_CRYPTO_set_dynlock_lock_callback(void (*func)(int mode,struct CRYPTO_dynlock_value *l, const char *file, int line))
{
    if ( p_CRYPTO_set_dynlock_lock_callback )
	p_CRYPTO_set_dynlock_lock_callback(func);
}

void 
ck_CRYPTO_set_dynlock_destroy_callback(void (*func)(struct CRYPTO_dynlock_value *l, const char *file, int line))
{
    if ( p_CRYPTO_set_dynlock_destroy_callback )
	p_CRYPTO_set_dynlock_destroy_callback(func);
}


#ifdef NT
static HANDLE * lock_cs = 0;
static void
win32_locking_callback(int mode, int type, const char *file, int line)
{
    if ( type >= CRYPTO_num_locks() )
        return;

    if (mode & CRYPTO_LOCK)
    {
        WaitForSingleObject(lock_cs[type],INFINITE);
    }
    else if (mode & CRYPTO_UNLOCK)
    {
        ReleaseMutex(lock_cs[type]);
    }
}

struct CRYPTO_dynlock_value {
    HANDLE hLock;
};

static struct CRYPTO_dynlock_value *
win32_dynlock_create_callback(const char *file,int line) 
{
    struct CRYPTO_dynlock_value * value = malloc(sizeof(struct CRYPTO_dynlock_value));
    if ( value )
	value->hLock = CreateMutex(NULL,FALSE,NULL);
    return value;
}

static void 
win32_dynlock_lock_callback(int mode,struct CRYPTO_dynlock_value *l, const char *file,int line)
{
    if (mode & CRYPTO_LOCK)
    {
        WaitForSingleObject(l->hLock,INFINITE);
    }
    else if (mode & CRYPTO_UNLOCK)
    {
        ReleaseMutex(l->hLock);
    }
}

static void 
win32_dynlock_destroy_callback(struct CRYPTO_dynlock_value *l,const char *file,int line)
{
    if ( l ) {
	CloseHandle(l->hLock);
	free(l);
    }
}
#else /* NT */
static HMTX * lock_cs = 0;
static void
os2_locking_callback(int mode, int type, const char *file, int line)
{
    if ( type >= CRYPTO_num_locks() )
        return;

    if (mode & CRYPTO_LOCK)
    {
        DosRequestMutexSem(lock_cs[type],SEM_INDEFINITE_WAIT);
    }
    else
    {
        DosReleaseMutexSem(lock_cs[type]);
    }
}

struct CRYPTO_dynlock_value {
    HMTX hLock;
};

static struct CRYPTO_dynlock_value *
os2_dynlock_create_callback(const char *file,int line) 
{
    struct CRYPTO_dynlock_value * value = malloc(sizeof(struct CRYPTO_dynlock_value));
    if ( value )
	DosCreateMutexSem(NULL, &value->hLock, 0, FALSE);
    return value;
}

static void 
os2_dynlock_lock_callback(int mode,struct CRYPTO_dynlock_value *l, const char *file,int line)
{
    if (mode & CRYPTO_LOCK)
    {
        DosRequestMutexSem(l->hLock,SEM_INDEFINITE_WAIT);
    }
    else if (mode & CRYPTO_UNLOCK)
    {
        DosReleaseMutexSem(l->hLock);
    }
}

static void 
os2_dynlock_destroy_callback(struct CRYPTO_dynlock_value *l,const char *file,int line)
{
    if ( l ) {
	DosCloseMutexSem(l->hLock);
	free(l);
    }
}
#endif /* NT */

int
ck_CRYPTO_num_locks(void)
{
    if ( p_CRYPTO_num_locks )
        return(p_CRYPTO_num_locks());
    else
        return(CRYPTO_NUM_LOCKS);
}

int
ck_RAND_status(void)
{
    if ( p_RAND_status )
        return(p_RAND_status());
    else
        return(1);
}

char *
ck_RAND_file_name(char * name, int len)
{
    if ( p_RAND_file_name )
        return(p_RAND_file_name(name, len));
    else
        return(NULL);
}

int
ck_RAND_egd(char * name)
{
    if ( p_RAND_egd )
        return(p_RAND_egd(name));
    else
        return(0);
}

int
ck_RAND_load_file(char * name, int len)
{
    if ( p_RAND_load_file )
        return(p_RAND_load_file(name,len));
    else
        return(0);
}

int
ck_RAND_write_file(char * name)
{
    if ( p_RAND_write_file )
        return(p_RAND_write_file(name));
    else
        return(0);
}

void
ck_RAND_seed(const void * buf, int num)
{
    if ( p_RAND_seed )
        p_RAND_seed(buf,num);
}

void
ck_sk_free(STACK * s)
{
    if ( p_sk_free )
        p_sk_free(s);
}

char *
ck_sk_value(const STACK * s, int n)
{
    if ( p_sk_value )
        return(p_sk_value(s,n));
    else
        return(NULL);
}

int
ck_sk_num(const STACK * s)
{
    if ( p_sk_num )
        return(p_sk_num(s));
    else
        return(-1);
}

#ifdef ZLIB
COMP_METHOD *
ck_COMP_zlib(void )
{
    if ( p_COMP_zlib )
        return(p_COMP_zlib());
    else
        return(0);
}
#endif /* ZLIB */

COMP_METHOD *
ck_COMP_rle(void )
{
    if ( p_COMP_rle )
        return(p_COMP_rle());
    else
        return(0);
}

unsigned long
ck_ERR_peek_error(void)
{
    if ( p_ERR_peek_error )
        return p_ERR_peek_error();
    return(0);
}

void
ck_sk_pop_free(STACK *st, void (*func)(void *))
{
    if ( p_sk_pop_free )
        p_sk_pop_free(st,func);
}

X509 *
ck_PEM_read_bio_X509(BIO *bp, X509** x509, pem_password_cb *cb, void *u)
{
    if ( p_PEM_read_bio_X509 )
        return p_PEM_read_bio_X509(bp,x509,cb,u);
    return(NULL);
}

unsigned long
ck_SSLeay(void)
{
    if ( p_SSLeay )
        return(p_SSLeay());
    return(0);
}

const char *
ck_SSLeay_version(int t)
{
    if ( p_SSLeay_version )
        return p_SSLeay_version(t);
    return("function not loaded");
}

BIGNUM *
ck_BN_new(void)
{
    if ( p_BN_new )
        return(p_BN_new());
    else
        return NULL;
}

int
ck_BN_bn2bin(const BIGNUM *a, unsigned char *to)
{
    if ( p_BN_bn2bin )
        return(p_BN_bn2bin(a,to));
    else
        return(0);
}

int
ck_BN_num_bits(const BIGNUM *a)
{
    if ( p_BN_num_bits )
        return p_BN_num_bits(a);
    else
        return(0);
}

int
ck_BN_cmp(const BIGNUM *a, const BIGNUM *b)
{
    if ( p_BN_cmp )
        return p_BN_cmp(a,b);
    else
        return(0);
}

int
ck_BN_add_word(BIGNUM *a, BN_ULONG w)
{
    if ( p_BN_add_word )
        return p_BN_add_word(a,w);
    else
        return(0);
}
int
ck_BN_lshift(BIGNUM *r, const BIGNUM *a, int n)
{
    if ( p_BN_lshift )
        return p_BN_lshift(r,a,n);
    else
        return(0);
}

int
ck_BN_set_word(BIGNUM *a, BN_ULONG w)
{
    if ( p_BN_set_word )
        return p_BN_set_word(a,w);
    else
        return(0);
}

int
ck_BN_dec2bn(BIGNUM **a, const char *str)
{
    if ( p_BN_dec2bn )
        return p_BN_dec2bn(a,str);
    else
        return(0);
}

char *
ck_BN_bn2dec(const BIGNUM *a)
{
    if ( p_BN_bn2dec )
        return p_BN_bn2dec(a);
    else
        return NULL;
}

BIGNUM *
ck_BN_copy(BIGNUM *a, const BIGNUM *b)
{
    if ( p_BN_copy )
        return p_BN_copy(a,b);
    else
        return NULL;
}

void
ck_BN_CTX_free(BN_CTX *c)
{
    if ( p_BN_CTX_free )
        p_BN_CTX_free(c);
}

int
ck_BN_div(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,BN_CTX *ctx)
{
    if ( p_BN_div )
        return p_BN_div(dv,rem,m,d,ctx);
    else
        return 0;
}

int
ck_BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
    if ( p_BN_sub )
        return p_BN_sub(r,a,b);
    else
        return 0;
}

const BIGNUM *
ck_BN_value_one(void)
{
    if ( p_BN_value_one )
        return p_BN_value_one();
    return NULL;
}

BN_CTX *
ck_BN_CTX_new(void)
{
    if ( p_BN_CTX_new )
        return p_BN_CTX_new();
    else
        return NULL;
}

void
ck_BN_CTX_start(BN_CTX *ctx)
{
    if ( p_BN_CTX_start )
        p_BN_CTX_start(ctx);
}

BIGNUM *
ck_BN_CTX_get(BN_CTX *ctx)
{
    if ( p_BN_CTX_get )
        return(p_BN_CTX_get(ctx));
    else
        return(NULL);
}

void
ck_BN_CTX_end(BN_CTX *ctx)
{
    if ( p_BN_CTX_end )
        p_BN_CTX_end(ctx);
}

void
ck_BN_free(BIGNUM *a)
{
    if ( p_BN_free )
        p_BN_free(a);
}

int
ck_BN_hex2bn(BIGNUM **a, const char *str)
{
    if ( p_BN_hex2bn )
        return p_BN_hex2bn(a,str);
    else
        return 0;
}

int
ck_BN_is_bit_set(const BIGNUM *a, int n)
{
    if ( p_BN_is_bit_set )
        return p_BN_is_bit_set(a,n);
    else
        return 0;
}

int
ck_BN_rand(BIGNUM *rnd, int bits, int top,int bottom)
{
    if ( p_BN_rand )
        return p_BN_rand(rnd,bits,top,bottom);
    else
        return 0;
}

char *
ck_BN_bn2hex(const BIGNUM *a)
{
    if ( p_BN_bn2hex )
        return(p_BN_bn2hex(a));
    else
        return NULL;
}

int
ck_BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
    if ( p_BN_add )
        return(p_BN_add(r,a,b));
    else
        return 0;
}

int
ck_BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx)
{
    if ( p_BN_mul )
        return(p_BN_mul(r,a,b,ctx));
    else
        return 0;
}

int
ck_BN_mod(BIGNUM *rem, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)
{
    if ( p_BN_mod )
        return(p_BN_mod(rem,m,d,ctx));
    else if ( p_BN_div )
        return(p_BN_div(NULL,rem,m,d,ctx));
    else
        return 0;
}

BN_ULONG
ck_BN_mod_word(const BIGNUM *a, BN_ULONG w)
{
    if ( p_BN_mod_word )
        return(p_BN_mod_word(a,w));
    else
        return 0;
}

int
ck_BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
{
    if ( p_BN_mod_mul )
        return(p_BN_mod_mul(ret,a,b,m,ctx));
    else
        return 0;
}

int
ck_BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,const BIGNUM *m,BN_CTX *ctx)
{
    if ( p_BN_mod_exp )
        return(p_BN_mod_exp(r,a,p,m,ctx));
    else
        return 0;
}

int
ck_BN_is_prime(const BIGNUM *p,int nchecks,void (*callback)(int,int,void *),BN_CTX *ctx,void *cb_arg)
{
    if ( p_BN_is_prime )
        return(p_BN_is_prime(p,nchecks,callback,ctx,cb_arg));
    else
        return 0;
}

int
ck_BN_ucmp(const BIGNUM *a, const BIGNUM *b)
{
    if ( p_BN_ucmp )
        return(p_BN_ucmp(a,b));
    else
        return 0;
}

int
ck_BN_mul_word(BIGNUM *a, BN_ULONG w)
{
    if ( p_BN_mul_word )
        return(p_BN_mul_word(a,w));
    else
        return 0;
}

int
ck_BN_rshift1(BIGNUM *r, const BIGNUM *a)
{
    if ( p_BN_rshift1 )
        return(p_BN_rshift1(r,a));
    else
        return 0;
}

BIGNUM *
ck_BN_dup(const BIGNUM *a)
{
    if ( p_BN_dup )
        return p_BN_dup(a);
    else
        return NULL;
}

int
ck_MD5_Init(MD5_CTX *c)
{
    if ( p_MD5_Init )
        return p_MD5_Init(c);
    else
        return 0;
}

int
ck_MD5_Update(MD5_CTX *c, const void *data, unsigned long len)
{
    if ( p_MD5_Update )
        return p_MD5_Update(c,data,len);
    else
        return 0;
}

int
ck_MD5_Final(unsigned char *md, MD5_CTX *c)
{
    if ( p_MD5_Final )
        return p_MD5_Final(md,c);
    else
        return 0;
}

unsigned char *
ck_MD5(const unsigned char *d, unsigned long n, unsigned char *md)
{
    if ( p_MD5 )
        return p_MD5(d,n,md);
    else
        return NULL;
}

int
ck_EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type)
{
    if ( p_EVP_DigestInit )
        return p_EVP_DigestInit(ctx,type);
    return 0;
}

int
ck_EVP_DigestUpdate(EVP_MD_CTX *ctx,const void *d,
                    unsigned int cnt)
{
    if ( p_EVP_DigestUpdate )
        return p_EVP_DigestUpdate(ctx,d,cnt);
    return 0;
}
int
ck_EVP_DigestFinal(EVP_MD_CTX *ctx,unsigned char *md,unsigned int *s)
{
    if ( p_EVP_DigestFinal )
        return p_EVP_DigestFinal(ctx,md,s);
    return 0;
}

const EVP_MD *
ck_EVP_sha1(void)
{
    if ( p_EVP_sha1 )
        return p_EVP_sha1();
    return NULL;
}

const EVP_MD *
ck_EVP_md5(void)
{
    if ( p_EVP_md5 )
        return p_EVP_md5();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_des_ede3_cbc(void)
{
    if ( p_EVP_des_ede3_cbc )
        return p_EVP_des_ede3_cbc();
    return NULL;
}

const EVP_MD *
ck_EVP_ripemd160(void)
{
    if ( p_EVP_ripemd160 )
        return p_EVP_ripemd160();
    return NULL;
}

const EVP_MD *
ck_EVP_get_digestbyname(const char *name)
{
    if ( p_EVP_get_digestbyname )
        return p_EVP_get_digestbyname(name);
    return NULL;
}

RSA *
ck_EVP_PKEY_get1_RSA(EVP_PKEY *pkey)
{
    if ( p_EVP_PKEY_get1_RSA )
        return p_EVP_PKEY_get1_RSA(pkey);
    return NULL;

}

DSA *
ck_EVP_PKEY_get1_DSA(EVP_PKEY *pkey)
{
    if ( p_EVP_PKEY_get1_DSA )
        return p_EVP_PKEY_get1_DSA(pkey);
    return NULL;
}

void
ck_EVP_PKEY_free(EVP_PKEY *pkey)
{
    if ( p_EVP_PKEY_free )
        p_EVP_PKEY_free(pkey);
}

RSA *
ck_RSA_new(void)
{
    if ( p_RSA_new )
        return p_RSA_new();
    return NULL;
}

int
ck_RSA_public_encrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding)
{
    if ( p_RSA_public_encrypt )
        return p_RSA_public_encrypt(flen,from,to,rsa,padding);
    return 0;
}

int
ck_RSA_private_encrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding)
{
    if ( p_RSA_private_encrypt )
        return p_RSA_private_encrypt(flen,from,to,rsa,padding);
    return 0;
}

int
ck_RSA_private_decrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding)
{
    if ( p_RSA_private_decrypt )
        return p_RSA_private_decrypt(flen,from,to,rsa,padding);
    return 0;
}

int     
ck_RSA_public_decrypt(int flen, const unsigned char *from,
                unsigned char *to, RSA *rsa,int padding)
{
    if ( p_RSA_public_decrypt )
        return p_RSA_public_decrypt(flen, from, to, rsa, padding);
    return 0;
}

int
ck_RSA_size(const RSA * rsa)
{
    if ( p_RSA_size )
        return p_RSA_size(rsa);
    return 0;
}

int
ck_RSA_sign(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
    if ( p_RSA_sign )
        return p_RSA_sign(type,m,m_len,sigret,siglen,rsa);
    return 0;
}

int
ck_RSA_verify(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
{
    if ( p_RSA_verify )
        return p_RSA_verify(type,m,m_len,sigbuf,siglen,rsa);
    return 0;
}

DSA *
ck_DSA_new(void)
{
    if ( p_DSA_new )
        return p_DSA_new();
    return NULL;
}

DSA_SIG *
ck_DSA_SIG_new(void)
{
    if ( p_DSA_new )
        return p_DSA_SIG_new();
    return NULL;
}

void
ck_DSA_SIG_free(DSA_SIG *a)
{
    if ( p_DSA_SIG_free )
        p_DSA_SIG_free(a);
}

DSA_SIG *
ck_DSA_do_sign(const unsigned char *dgst,int dlen,DSA *dsa)
{
    if ( p_DSA_do_sign )
        return p_DSA_do_sign(dgst,dlen,dsa);
    return NULL;
}

int
ck_DSA_do_verify(const unsigned char *dgst,int dgst_len,
                  DSA_SIG *sig,DSA *dsa)
{
    if ( p_DSA_do_verify )
        return p_DSA_do_verify(dgst,dgst_len,sig,dsa);
    return 0;
}

void
ck_HMAC_Init(HMAC_CTX *ctx, const void *key, int len,
               const EVP_MD *md)
{
    if ( p_HMAC_Init )
        p_HMAC_Init(ctx,key,len,md);
}

void
ck_HMAC_Update(HMAC_CTX *ctx, const unsigned char *data, int len)
{
    if ( p_HMAC_Update )
        p_HMAC_Update(ctx,data,len);
}

void
ck_HMAC_Final(HMAC_CTX *ctx, unsigned char *md, unsigned int *len)
{
    if ( p_HMAC_Final )
        p_HMAC_Final(ctx,md,len);
}

void
ck_HMAC_cleanup(HMAC_CTX *ctx)
{
    if ( p_HMAC_cleanup )
        p_HMAC_cleanup(ctx);
}

void
ck_HMAC_CTX_cleanup(HMAC_CTX *ctx)
{
    if ( p_HMAC_CTX_cleanup )
        p_HMAC_CTX_cleanup(ctx);
}

void
ck_RAND_add(const void *buf,int num,double entropy)
{
    if ( p_RAND_add )
        p_RAND_add(buf,num,entropy);
}

int
ck_RAND_bytes(unsigned char *buf,int num)
{
    if ( p_RAND_bytes )
        return p_RAND_bytes(buf,num);
    return 0;
}

int
ck_des_set_key(const_des_cblock *k,des_key_schedule s)
{
    if ( p_des_set_key )
        return p_des_set_key(k,s);
    return 0;
}

void
ck_des_ncbc_encrypt(const unsigned char *input,unsigned char *output,
                     long length,des_key_schedule schedule,des_cblock *ivec,
                     int enc)
{
    if ( p_des_ncbc_encrypt )
        p_des_ncbc_encrypt(input,output,length,schedule,ivec,enc);
}

void
ck_des_ede3_cbc_encrypt(const unsigned char *input,unsigned char *output,
                         long length,
                         des_key_schedule ks1,des_key_schedule ks2,
                         des_key_schedule ks3,des_cblock *ivec,int enc)
{
    if ( p_des_ede3_cbc_encrypt )
        p_des_ede3_cbc_encrypt(input,output,length,ks1,ks2,ks3,ivec,enc);
}

void
ck_BF_set_key(BF_KEY *key, int len, const unsigned char *data)
{
    if ( p_BF_set_key )
        p_BF_set_key(key,len,data);
}

void
ck_BF_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
        const BF_KEY *schedule, unsigned char *ivec, int enc)
{
    if ( p_BF_cbc_encrypt )
        p_BF_cbc_encrypt(in,out,length,schedule,ivec,enc);
}

void
ck_RC4_set_key(RC4_KEY *key, int len, const unsigned char *data)
{
    if ( p_RC4_set_key )
        p_RC4_set_key(key,len,data);
}

void
ck_RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata,
        unsigned char *outdata)
{
    if ( p_RC4 )
        p_RC4(key,len,indata,outdata);
}

void
ck_CAST_set_key(CAST_KEY *key, int len, const unsigned char *data)
{
    if ( p_CAST_set_key )
        p_CAST_set_key(key,len,data);
}

void
ck_CAST_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
                     CAST_KEY *ks, unsigned char *iv, int enc)
{
    if ( p_CAST_cbc_encrypt )
        p_CAST_cbc_encrypt(in,out,length,ks,iv,enc);
}

int
ck_DH_compute_key(unsigned char *key,const BIGNUM *pub_key,DH *dh)
{
    if ( p_DH_compute_key )
        return p_DH_compute_key(key,pub_key,dh);
    return 0;
}

int
ck_DH_size(const DH *dh)
{
    if ( p_DH_size )
        return p_DH_size(dh);
    return 0;
}

const char *
ck_OBJ_nid2sn(int n)
{
    if ( p_OBJ_nid2sn )
        return p_OBJ_nid2sn(n);
    return NULL;
}

int
ck_OBJ_create(const char *oid,const char *sn,const char *ln)
{
    if ( p_OBJ_create )
        return p_OBJ_create(oid,sn,ln);
    return 0;
}

void
ck_ERR_load_crypto_strings(void)
{
    if ( p_ERR_load_crypto_strings )
        p_ERR_load_crypto_strings();
}

void
ck_BN_clear_free(BIGNUM *a)
{
    if ( p_BN_clear_free )
        p_BN_clear_free(a);
}

void
ck_BN_clear(BIGNUM *a)
{
    if ( p_BN_clear )
        p_BN_clear(a);
}

void
ck_CRYPTO_free(void * p)
{
    if ( p_CRYPTO_free )
        p_CRYPTO_free(p);
}

int
ck_PEM_write_DSAPrivateKey(FILE *fp, char *x,
             const EVP_CIPHER *enc, unsigned char *kstr, int klen,
             pem_password_cb *callback, void *u)
{
    if ( p_PEM_write_DSAPrivateKey )
        return p_PEM_write_DSAPrivateKey(fp,x,enc,kstr,klen,callback,u);
    return 0;
}

int
ck_PEM_write_RSAPrivateKey(FILE *fp, char *x,
             const EVP_CIPHER *enc, unsigned char *kstr, int klen,
             pem_password_cb *callback, void *u)
{
    if ( p_PEM_write_RSAPrivateKey )
        return p_PEM_write_RSAPrivateKey(fp,x,enc,kstr,klen,callback,u);
    return 0;
}

int
ck_PEM_write_bio_X509(BIO *a, X509 *x)
{
    if ( p_PEM_write_bio_X509 )
        return p_PEM_write_bio_X509(a,x);
    return 0;
}

EVP_PKEY *
ck_PEM_read_PrivateKey(FILE *fp,EVP_PKEY **x, pem_password_cb *cb, void *u)
{
    if ( p_PEM_read_PrivateKey )
        return p_PEM_read_PrivateKey(fp,x,cb,u);
    return NULL;
}

const char *
ck_X509_get_default_cert_dir_env(void)
{
    if ( p_X509_get_default_cert_dir_env )
        return(p_X509_get_default_cert_dir_env());
    return("SSL_CERT_DIR");
}

const char *
ck_X509_get_default_cert_file_env(void)
{
    if ( p_X509_get_default_cert_file_env )
        return(p_X509_get_default_cert_file_env());
    return("SSL_CERT_FILE");
}

void
ck_SHA1_Init(SHA_CTX *c)
{
    if ( p_SHA1_Init )
        p_SHA1_Init(c);
}

void
ck_SHA1_Update(SHA_CTX *c, const void *data, unsigned long len)
{
    if ( p_SHA1_Update )
        p_SHA1_Update(c,data,len);
}

void
ck_SHA1_Final(unsigned char *md, SHA_CTX *c)
{
    if ( p_SHA1_Final )
        p_SHA1_Final(md,c);
}

void 
ck_OpenSSL_add_all_ciphers(void)
{
    if ( p_OpenSSL_add_all_ciphers )
        p_OpenSSL_add_all_ciphers();
}

void 
ck_OpenSSL_add_all_digests(void)
{
    if ( p_OpenSSL_add_all_digests )
        p_OpenSSL_add_all_digests();
}

void 
ck_OPENSSL_add_all_algorithms_noconf(void)
{
    if ( p_OPENSSL_add_all_algorithms_noconf )
        p_OPENSSL_add_all_algorithms_noconf();
}

void 
ck_OPENSSL_add_all_algorithms_conf(void)
{
    if ( p_OPENSSL_add_all_algorithms_conf )
        p_OPENSSL_add_all_algorithms_conf();
}

int 
ck_RSA_blinding_on(RSA *rsa, BN_CTX *ctx)
{
    if ( p_RSA_blinding_on )
        return p_RSA_blinding_on(rsa,ctx);
    return -1;
}

const EVP_CIPHER *
ck_EVP_aes_256_cbc(void)
{
    if ( p_EVP_aes_256_cbc )
        return p_EVP_aes_256_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_aes_192_cbc(void)
{
    if ( p_EVP_aes_192_cbc )
        return p_EVP_aes_192_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_aes_128_cbc(void)
{
    if ( p_EVP_aes_128_cbc )
        return p_EVP_aes_128_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_rc4(void)
{
    if ( p_EVP_rc4 )
        return p_EVP_rc4();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_cast5_cbc(void)
{
    if ( p_EVP_cast5_cbc )
        return p_EVP_cast5_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_bf_cbc(void)
{
    if ( p_EVP_bf_cbc )
        return p_EVP_bf_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_des_cbc(void)
{
    if ( p_EVP_des_cbc )
        return p_EVP_des_cbc();
    return NULL;
}

const EVP_CIPHER *
ck_EVP_enc_null(void)
{
    if ( p_EVP_enc_null )
        return p_EVP_enc_null();
    return NULL;
}

int
ck_EVP_CipherInit(EVP_CIPHER_CTX *ctx,const EVP_CIPHER *cipher,
                  const unsigned char *key,const unsigned char *iv,
                  int enc)
{
    if ( p_EVP_CipherInit )
        return p_EVP_CipherInit(ctx,cipher,key,iv,enc);
    return -1;
}

void 
ck_EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *a)
{
    if ( p_EVP_CIPHER_CTX_init )
        p_EVP_CIPHER_CTX_init(a);
}       

int  
ck_EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a)
{
    if (p_EVP_CIPHER_CTX_cleanup)
        return p_EVP_CIPHER_CTX_cleanup(a);
    return -1;
}

int  
ck_EVP_CIPHER_CTX_set_key_length(EVP_CIPHER_CTX *x, int keylen)
{
    if (p_EVP_CIPHER_CTX_set_key_length)
        return p_EVP_CIPHER_CTX_set_key_length(x,keylen);
    return -1;
}

int  
ck_EVP_CIPHER_CTX_key_length(const EVP_CIPHER_CTX *ctx)
{
    if (p_EVP_CIPHER_CTX_key_length)
	    return p_EVP_CIPHER_CTX_key_length(ctx);
    return -1;
}	


int  
ck_EVP_Cipher(EVP_CIPHER_CTX *c, unsigned char *out,
		   const unsigned char *in, unsigned int inl)
{
    if (p_EVP_Cipher)
	    return p_EVP_Cipher(c, out, in, inl);
    return -1;
}

int  
ck_EVP_CIPHER_CTX_iv_length(const EVP_CIPHER_CTX *ctx)
{
    if (p_EVP_CIPHER_CTX_iv_length)
	    return p_EVP_CIPHER_CTX_iv_length(ctx);
    return -1;
}

void *
ck_EVP_CIPHER_CTX_get_app_data(const EVP_CIPHER_CTX *ctx)
{
    if (p_EVP_CIPHER_CTX_get_app_data)
	    return p_EVP_CIPHER_CTX_get_app_data(ctx);
    return NULL;
}

void 
ck_EVP_CIPHER_CTX_set_app_data(EVP_CIPHER_CTX *ctx, void *data)
{
    if (p_EVP_CIPHER_CTX_set_app_data)
	    p_EVP_CIPHER_CTX_set_app_data(ctx, data);
}

int  
ck_EVP_MD_size(const EVP_MD *md)
{
    if ( p_EVP_MD_size )
        return(p_EVP_MD_size(md));
    else
        return(-1);
}

int
ck_crypto_loaddll_eh(void)
{
#ifdef NT
    if ( hCRYPTO ) {
        FreeLibrary(hCRYPTO);
        hCRYPTO = NULL;
    }
#endif  /* NT */

    p_ERR_print_errors=NULL;
    p_ERR_print_errors_fp=NULL;
    p_ERR_get_error=NULL;
    p_ERR_error_string=NULL;

    p_BIO_printf=NULL;
    p_BIO_ctrl=NULL;
    p_BIO_new=NULL;
    p_BIO_s_file=NULL;
    p_BIO_s_mem=NULL;
    p_BIO_s_null=NULL;
    p_BIO_read = NULL;
    p_BIO_write = NULL;
    p_BIO_free = NULL;
    p_BIO_free_all = NULL;
    p_BIO_new_file = NULL;

    p_X509_get_issuer_name=NULL;
    p_X509_verify_cert_error_string=NULL;
    p_X509_NAME_oneline=NULL;
    p_X509_NAME_print_ex=NULL;
    p_X509_NAME_get_text_by_NID=NULL;
    p_X509_get_subject_name=NULL;
    p_X509_STORE_CTX_get_current_cert=NULL;
    p_X509_STORE_CTX_get_error=NULL;
    p_X509_STORE_CTX_get_error_depth=NULL;
    p_X509_free=NULL;
    p_X509_get_default_cert_dir=NULL;
    p_X509_print_ex=NULL;

    p_RSA_free=NULL;
    p_RSA_generate_key=NULL;
    p_DH_free=NULL;
    p_DH_generate_parameters=NULL;
    p_DH_generate_key=NULL;
    p_DH_new=NULL;
    p_DSA_free=NULL;
    p_DSA_generate_parameters=NULL;
    p_DSA_generate_key=NULL;

#ifdef COMMENT

    p_PEM_read_bio_DHparams=NULL;
#endif
    p_d2i_DHparams=NULL;
    p_PEM_ASN1_read_bio=NULL;
    p_BN_bin2bn=NULL;

    p_ASN1_TIME_print=NULL;

    p_sk_GENERAL_NAME_free=NULL;
    p_X509V3_EXT_cleanup=NULL;
    p_sk_GENERAL_NAME_value=NULL;
    p_sk_GENERAL_NAME_num=NULL;
    p_X509V3_EXT_d2i=NULL;
    p_X509V3_add_standard_extensions=NULL;
    p_X509_get_ext=NULL;
    p_X509_get_ext_by_NID=NULL;
    p_ASN1_INTEGER_get=NULL;
    p_ASN1_INTEGER_cmp=NULL;
    p_ASN1_STRING_cmp=NULL;
    p_X509_get_serialNumber=NULL;
    p_sk_X509_REVOKED_value=NULL;
    p_sk_X509_REVOKED_num=NULL;
    p_X509_cmp_current_time=NULL;
    p_X509_OBJECT_free_contents=NULL;
    p_X509_STORE_CTX_set_error=NULL;
    p_X509_CRL_verify=NULL;
    p_X509_get_pubkey=NULL;
    p_X509_STORE_CTX_new=NULL;
    p_X509_STORE_CTX_free=NULL;
    p_X509_STORE_CTX_cleanup=NULL;
    p_X509_STORE_get_by_subject=NULL;
    p_X509_STORE_CTX_init=NULL;
    p_PEM_read_X509=NULL;
    p_X509_LOOKUP_hash_dir=NULL;
    p_X509_LOOKUP_ctrl=NULL;
    p_X509_STORE_add_lookup=NULL;
    p_X509_LOOKUP_file=NULL;
    p_X509_STORE_new=NULL;
    p_X509_STORE_free=NULL;
    p_X509_STORE_load_locations=NULL;
    p_X509_STORE_set_default_paths=NULL;
    p_X509_subject_name_hash=NULL;
    p_X509_issuer_name_hash=NULL;
    p_CRYPTO_set_mem_functions=NULL;
    p_CRYPTO_set_locking_callback=NULL;
    p_CRYPTO_set_dynlock_create_callback=NULL;
    p_CRYPTO_set_dynlock_lock_callback=NULL;
    p_CRYPTO_set_dynlock_destroy_callback=NULL;
    p_RAND_screen=NULL;
    p_CRYPTO_num_locks=NULL;
    p_RAND_status=NULL;
    p_RAND_file_name=NULL;
    p_RAND_egd=NULL;
    p_RAND_load_file=NULL;
    p_RAND_write_file=NULL;
    p_RAND_seed = NULL;

    p_sk_num = NULL;
    p_sk_value = NULL;
    p_sk_free = NULL;

#ifdef ZLIB
    p_COMP_zlib=NULL;
#endif /* ZLIB */
    p_COMP_rle=NULL;
    p_ERR_peek_error=NULL;
    p_sk_pop_free=NULL;
    p_PEM_read_bio_X509=NULL;
    p_SSLeay = NULL;
    p_SSLeay_version = NULL;

    p_BN_new                = NULL;
    p_BN_bn2bin             = NULL;
    p_BN_num_bits           = NULL;
    p_BN_cmp                = NULL;
    p_BN_add_word           = NULL;
    p_BN_lshift             = NULL;
    p_BN_set_word           = NULL;
    p_BN_dec2bn             = NULL;
    p_BN_bn2dec             = NULL;
    p_BN_copy               = NULL;
    p_BN_CTX_free           = NULL;
    p_BN_div                = NULL;
    p_BN_sub                = NULL;
    p_BN_value_one          = NULL;
    p_BN_CTX_new            = NULL;
    p_BN_CTX_get            = NULL;
    p_BN_CTX_start          = NULL;
    p_BN_CTX_end            = NULL;
    p_BN_free               = NULL;
    p_BN_hex2bn             = NULL;
    p_BN_is_bit_set         = NULL;
    p_BN_rand               = NULL;
    p_BN_bn2hex             = NULL;
    p_BN_add                = NULL;
    p_BN_mul                = NULL;
    p_BN_mod                = NULL;
    p_BN_mod_word           = NULL;
    p_BN_mod_mul            = NULL;
    p_BN_mod_exp            = NULL;
    p_BN_is_prime           = NULL;
    p_BN_ucmp               = NULL;
    p_BN_mul_word           = NULL;
    p_MD5_Final             = NULL;
    p_MD5_Update            = NULL;
    p_MD5_Init              = NULL;
    p_BN_clear_free         = NULL;
    p_BN_clear              = NULL;
    p_BN_rshift1            = NULL;
    p_BN_dup                = NULL;

    p_EVP_DigestFinal       = NULL;
    p_EVP_DigestUpdate      = NULL;
    p_EVP_DigestInit        = NULL;
    p_EVP_sha1              = NULL;
    p_EVP_md5               = NULL;
    p_EVP_des_ede3_cbc      = NULL;
    p_EVP_PKEY_free         = NULL;
    p_EVP_PKEY_get1_DSA     = NULL;
    p_EVP_PKEY_get1_RSA     = NULL;
    p_EVP_get_digestbyname  = NULL;
    p_EVP_ripemd160         = NULL;
    p_RSA_new               = NULL;
    p_RSA_public_encrypt    = NULL;
    p_RSA_private_encrypt   = NULL;
    p_RSA_private_decrypt   = NULL;
    p_RSA_public_decrypt    = NULL;
    p_RSA_sign              = NULL;
    p_RSA_size              = NULL;
    p_RSA_verify            = NULL;
    p_DSA_new               = NULL;
    p_DSA_SIG_free          = NULL;
    p_DSA_do_sign           = NULL;
    p_DSA_do_verify         = NULL;
    p_DSA_SIG_new           = NULL;
    p_HMAC_cleanup          = NULL;
    p_HMAC_CTX_cleanup      = NULL;
    p_HMAC_Final            = NULL;
    p_HMAC_Update           = NULL;
    p_HMAC_Init             = NULL;
    p_RAND_add              = NULL;
    p_RAND_bytes            = NULL;
    p_des_set_key           = NULL;
    p_des_ncbc_encrypt      = NULL;
    p_des_ede3_cbc_encrypt  = NULL;
    p_BF_set_key            = NULL;
    p_BF_cbc_encrypt        = NULL;
    p_RC4_set_key           = NULL;
    p_RC4                   = NULL;
    p_CAST_set_key          = NULL;
    p_CAST_cbc_encrypt      = NULL;
    p_OBJ_nid2sn            = NULL;
    p_OBJ_create            = NULL;
    p_DH_compute_key        = NULL;
    p_DH_size               = NULL;
    p_ERR_load_crypto_strings    = NULL;
    p_CRYPTO_free = NULL;
    p_PEM_write_DSAPrivateKey = NULL;
    p_PEM_write_RSAPrivateKey = NULL;
    p_PEM_write_bio_X509 = NULL;
    p_PEM_read_PrivateKey = NULL;
    p_X509_get_default_cert_dir_env = NULL;
    p_X509_get_default_cert_file_env = NULL;
    p_SHA1_Init = NULL;
    p_SHA1_Update = NULL;
    p_SHA1_Final = NULL;
    p_OpenSSL_add_all_ciphers = NULL;
    p_OpenSSL_add_all_digests = NULL;
    p_OPENSSL_add_all_algorithms_noconf = NULL;
    p_OPENSSL_add_all_algorithms_conf = NULL;

    p_RSA_blinding_on                 = NULL;
    p_EVP_aes_256_cbc                 = NULL;
    p_EVP_aes_192_cbc                 = NULL;
    p_EVP_aes_128_cbc                 = NULL;
    p_EVP_rc4                         = NULL;
    p_EVP_cast5_cbc                   = NULL;
    p_EVP_bf_cbc                      = NULL;
    p_EVP_des_cbc                     = NULL;
    p_EVP_enc_null                    = NULL;
    p_EVP_CipherInit                  = NULL;
    p_EVP_CIPHER_CTX_set_key_length   = NULL;
    p_EVP_CIPHER_CTX_init             = NULL;
    p_EVP_CIPHER_CTX_cleanup          = NULL;
    p_EVP_CIPHER_CTX_key_length       = NULL;
    p_EVP_Cipher                      = NULL;
    p_EVP_CIPHER_CTX_iv_length        = NULL;
    p_EVP_CIPHER_CTX_get_app_data     = NULL;
    p_EVP_CIPHER_CTX_set_app_data     = NULL;
    p_EVP_MD_size = NULL;

    return(1);
}

int
ck_crypto_loaddll( void )
{
    ULONG rc = 0 ;
    extern unsigned long startflags;
    int load_error = 0;
#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "SSLEAY2";
    CHAR errbuf[256];
#endif /* OS2ONLY */

    if ( crypto_dll_loaded )
        return(1);

    if ( startflags & 8 )       /* do not load if we are not supposed to */
        return(0);

#ifdef NT
    hCRYPTO = LoadLibrary("LIBEAY32");
    if ( !hCRYPTO ) {
        rc = GetLastError() ;
        debug(F111, "OpenSSL LoadLibrary failed","LIBEAY32",rc) ;
        return(0);
    } else {
        if (((FARPROC) p_ERR_print_errors =
              GetProcAddress( hCRYPTO, "ERR_print_errors" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_print_errors",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ERR_print_errors_fp =
              GetProcAddress( hCRYPTO, "ERR_print_errors_fp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_print_errors_fp",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ERR_get_error =
              GetProcAddress( hCRYPTO, "ERR_get_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_get_error",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ERR_error_string =
              GetProcAddress( hCRYPTO, "ERR_error_string" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_error_string",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_printf =
              GetProcAddress( hCRYPTO, "BIO_printf" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_printf",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_ctrl =
              GetProcAddress( hCRYPTO, "BIO_ctrl" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_ctrl",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_new =
              GetProcAddress( hCRYPTO, "BIO_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_s_file =
              GetProcAddress( hCRYPTO, "BIO_s_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_s_mem =
              GetProcAddress( hCRYPTO, "BIO_s_mem" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_mem",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_s_null =
              GetProcAddress( hCRYPTO, "BIO_s_null" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_null",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_read =
              GetProcAddress( hCRYPTO, "BIO_read" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_read",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_write =
              GetProcAddress( hCRYPTO, "BIO_write" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_write",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_free =
              GetProcAddress( hCRYPTO, "BIO_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_free_all =
              GetProcAddress( hCRYPTO, "BIO_free_all" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_free_all",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BIO_new_file =
              GetProcAddress( hCRYPTO, "BIO_new_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_new_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_issuer_name =
              GetProcAddress( hCRYPTO, "X509_get_issuer_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_issuer_name",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_print_ex =
              GetProcAddress( hCRYPTO, "X509_print_ex" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_print_ex",rc);
        }
        if (((FARPROC) p_X509_verify_cert_error_string =
              GetProcAddress( hCRYPTO, "X509_verify_cert_error_string" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_verify_cert_error_string",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_NAME_oneline =
              GetProcAddress( hCRYPTO, "X509_NAME_oneline" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_oneline",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_NAME_print_ex =
              GetProcAddress( hCRYPTO, "X509_NAME_print_ex" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_print_ex",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_NAME_get_text_by_NID =
              GetProcAddress( hCRYPTO, "X509_NAME_get_text_by_NID" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_get_text_by_NID",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_subject_name =
              GetProcAddress( hCRYPTO, "X509_get_subject_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_subject_name",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_new =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_new" )) == NULL )
        {
            /* New to 0.9.5 */
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_new",rc);
        }
        if (((FARPROC) p_X509_STORE_CTX_free =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_free" )) == NULL )
        {
            /* New to 0.9.5 */
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_free",rc);
        }
        if (((FARPROC) p_X509_STORE_CTX_get_current_cert =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_get_current_cert" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_current_cert",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_get_error =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_get_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_error",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_get_error_depth =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_get_error_depth" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_error_depth",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_free =
              GetProcAddress( hCRYPTO, "X509_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_default_cert_dir =
              GetProcAddress( hCRYPTO, "X509_get_default_cert_dir" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_default_cert_dir",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_free =
              GetProcAddress( hCRYPTO, "RSA_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","RSA_free",rc);
        }
        if (((FARPROC) p_RSA_generate_key =
              GetProcAddress( hCRYPTO, "RSA_generate_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","RSA_generate_key",rc);
        }
        if (((FARPROC) p_DH_new =
              GetProcAddress( hCRYPTO, "DH_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DH_free =
              GetProcAddress( hCRYPTO, "DH_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DH_generate_parameters =
              GetProcAddress( hCRYPTO, "DH_generate_parameters" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_generate_parameters",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DH_generate_key =
              GetProcAddress( hCRYPTO, "DH_generate_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_generate_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_free =
              GetProcAddress( hCRYPTO, "DSA_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_generate_parameters =
              GetProcAddress( hCRYPTO, "DSA_generate_parameters" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_generate_parameters",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_generate_key =
              GetProcAddress( hCRYPTO, "DSA_generate_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_generate_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_d2i_DHparams =
              GetProcAddress( hCRYPTO, "d2i_DHparams" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "d2i_DHparams",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_ASN1_read_bio =
              GetProcAddress( hCRYPTO, "PEM_ASN1_read_bio" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_ASN1_read_bio",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_bin2bn =
              GetProcAddress( hCRYPTO, "BN_bin2bn" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_bin2bn",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ASN1_TIME_print =
              GetProcAddress( hCRYPTO, "ASN1_TIME_print" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_TIME_print",rc);
            load_error = 1;
        }

        if (((FARPROC) p_sk_GENERAL_NAME_free =
              GetProcAddress( hCRYPTO, "sk_GENERAL_NAME_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_free",rc);
            /* no longer exported in 0.9.6 */
        }
        if (((FARPROC) p_X509V3_EXT_cleanup =
              GetProcAddress( hCRYPTO, "X509V3_EXT_cleanup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_EXT_cleanup",rc);
            load_error = 1;
        }
        if (((FARPROC) p_sk_GENERAL_NAME_value =
              GetProcAddress( hCRYPTO, "sk_GENERAL_NAME_value" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_value",rc);
            /* no longer exported in 0.9.6 */
        }
        if (((FARPROC) p_sk_GENERAL_NAME_num =
              GetProcAddress( hCRYPTO, "sk_GENERAL_NAME_num" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_num",rc);
            /* no longer exported in 0.9.6 */
        }
        if (((FARPROC) p_X509V3_EXT_d2i =
              GetProcAddress( hCRYPTO, "X509V3_EXT_d2i" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_EXT_d2i",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509V3_add_standard_extensions =
              GetProcAddress( hCRYPTO, "X509V3_add_standard_extensions" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_add_standard_extensions",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_ext =
              GetProcAddress( hCRYPTO, "X509_get_ext" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_ext",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_ext_by_NID =
              GetProcAddress( hCRYPTO, "X509_get_ext_by_NID" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_ext_by_NID",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ASN1_INTEGER_get =
              GetProcAddress( hCRYPTO, "ASN1_INTEGER_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_INTEGER_get",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ASN1_INTEGER_cmp =
              GetProcAddress( hCRYPTO, "ASN1_INTEGER_cmp" )) == NULL )
        {
            /* New to OpenSSL 0.9.5 */
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_INTEGER_cmp",rc);
        }
        if (((FARPROC) p_ASN1_STRING_cmp =
              GetProcAddress( hCRYPTO, "ASN1_STRING_cmp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_STRING_cmp",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_serialNumber =
              GetProcAddress( hCRYPTO, "X509_get_serialNumber" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_serialNumber",rc);
            load_error = 1;
        }
        if (((FARPROC) p_sk_X509_REVOKED_value =
              GetProcAddress( hCRYPTO, "sk_X509_REVOKED_value" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_X509_REVOKED_value",rc);
            /* no longer exported in 0.9.6 */
        }
        if (((FARPROC) p_sk_X509_REVOKED_num =
              GetProcAddress( hCRYPTO, "sk_X509_REVOKED_num" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_X509_REVOKED_num",rc);
            /* no longer exported in 0.9.6 */
        }
        if (((FARPROC) p_X509_cmp_current_time =
              GetProcAddress( hCRYPTO, "X509_cmp_current_time" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_cmp_current_time",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_OBJECT_free_contents =
              GetProcAddress( hCRYPTO, "X509_OBJECT_free_contents" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_OBJECT_free_contents",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_set_error =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_set_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_set_error",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_CRL_verify =
              GetProcAddress( hCRYPTO, "X509_CRL_verify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_CRL_verify",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_pubkey =
              GetProcAddress( hCRYPTO, "X509_get_pubkey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_pubkey",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_cleanup =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_cleanup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_cleanup",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_get_by_subject =
              GetProcAddress( hCRYPTO, "X509_STORE_get_by_subject" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_get_by_subject",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_CTX_init =
              GetProcAddress( hCRYPTO, "X509_STORE_CTX_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_init",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_read_X509 =
              GetProcAddress( hCRYPTO, "PEM_read_X509" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_read_X509",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_LOOKUP_hash_dir =
              GetProcAddress( hCRYPTO, "X509_LOOKUP_hash_dir" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_hash_dir",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_LOOKUP_ctrl =
              GetProcAddress( hCRYPTO, "X509_LOOKUP_ctrl" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_ctrl",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_add_lookup =
              GetProcAddress( hCRYPTO, "X509_STORE_add_lookup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_add_lookup",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_LOOKUP_file =
              GetProcAddress( hCRYPTO, "X509_LOOKUP_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_new =
              GetProcAddress( hCRYPTO, "X509_STORE_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_free =
              GetProcAddress( hCRYPTO, "X509_STORE_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_load_locations =
              GetProcAddress( hCRYPTO, "X509_STORE_load_locations" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_load_locations",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_STORE_set_default_paths =
              GetProcAddress( hCRYPTO, "X509_STORE_set_default_paths" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_set_default_paths",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_subject_name_hash =
              GetProcAddress( hCRYPTO, "X509_subject_name_hash" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_subject_name_hash",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_issuer_name_hash =
              GetProcAddress( hCRYPTO, "X509_issuer_name_hash" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_issuer_name_hash",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_set_mem_functions =
              GetProcAddress( hCRYPTO, "CRYPTO_set_mem_functions" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_mem_functions",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_set_locking_callback =
              GetProcAddress( hCRYPTO, "CRYPTO_set_locking_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_locking_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_set_dynlock_destroy_callback =
              GetProcAddress( hCRYPTO, "CRYPTO_set_dynlock_destroy_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_destroy_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_set_dynlock_create_callback =
              GetProcAddress( hCRYPTO, "CRYPTO_set_dynlock_create_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_create_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_set_dynlock_lock_callback =
              GetProcAddress( hCRYPTO, "CRYPTO_set_dynlock_lock_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_lock_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_num_locks =
              GetProcAddress( hCRYPTO, "CRYPTO_num_locks" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_num_locks",rc);
        }
        if (((FARPROC) p_RAND_screen =
              GetProcAddress( hCRYPTO, "RAND_screen" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_screen",rc);
        }
        if (((FARPROC) p_RAND_status =
              GetProcAddress( hCRYPTO, "RAND_status" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_status",rc);
        }
        if (((FARPROC) p_RAND_file_name =
              GetProcAddress( hCRYPTO, "RAND_file_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_file_name",rc);
        }
        if (((FARPROC) p_RAND_egd =
              GetProcAddress( hCRYPTO, "RAND_egd" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_egd",rc);
        }
        if (((FARPROC) p_RAND_load_file =
              GetProcAddress( hCRYPTO, "RAND_load_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_load_file",rc);
        }
        if (((FARPROC) p_RAND_write_file =
              GetProcAddress( hCRYPTO, "RAND_write_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_write_file",rc);
        }
        if (((FARPROC) p_RAND_seed =
              GetProcAddress( hCRYPTO, "RAND_seed" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_seed",rc);
        }
        if (((FARPROC) p_sk_num =
              GetProcAddress( hCRYPTO, "sk_num" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_num",rc);
        }
        if (((FARPROC) p_sk_value =
              GetProcAddress( hCRYPTO, "sk_value" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_value",rc);
        }
        if (((FARPROC) p_sk_free =
              GetProcAddress( hCRYPTO, "sk_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_free",rc);
        }
#ifdef ZLIB
        if (((FARPROC) p_COMP_zlib =
              GetProcAddress( hCRYPTO, "COMP_zlib" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "COMP_zlib",rc);
        }
#endif /* ZLIB */
        if (((FARPROC) p_COMP_rle =
              GetProcAddress( hCRYPTO, "COMP_rle" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "COMP_rle",rc);
        }
        if (((FARPROC) p_ERR_peek_error =
              GetProcAddress( hCRYPTO, "ERR_peek_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ERR_peek_error",rc);
        }
        if (((FARPROC) p_sk_pop_free =
              GetProcAddress( hCRYPTO, "sk_pop_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_pop_free",rc);
        }
        if (((FARPROC) p_PEM_read_bio_X509 =
              GetProcAddress( hCRYPTO, "PEM_read_bio_X509" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_read_bio_X509",rc);
        }
        if (((FARPROC) p_SSLeay =
              GetProcAddress( hCRYPTO, "SSLeay" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SSLeay",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSLeay_version =
              GetProcAddress( hCRYPTO, "SSLeay_version" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SSLeay_version",rc);
            load_error = 1;
        }

        if (((FARPROC) p_SHA1_Init =
              GetProcAddress( hCRYPTO, "SHA1_Init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SHA1_Init",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SHA1_Update =
              GetProcAddress( hCRYPTO, "SHA1_Update" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SHA1_Update",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SHA1_Final =
              GetProcAddress( hCRYPTO, "SHA1_Final" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SHA1_Final",rc);
            load_error = 1;
        }

        if (((FARPROC) p_BN_new =
              GetProcAddress( hCRYPTO, "BN_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_dup =
              GetProcAddress( hCRYPTO, "BN_dup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_dup",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_bn2bin =
              GetProcAddress( hCRYPTO, "BN_bn2bin" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_bn2bin",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_num_bits =
              GetProcAddress( hCRYPTO, "BN_num_bits" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_num_bits",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_cmp =
              GetProcAddress( hCRYPTO, "BN_cmp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_cmp",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_ucmp =
              GetProcAddress( hCRYPTO, "BN_ucmp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_ucmp",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_add_word =
              GetProcAddress( hCRYPTO, "BN_add_word" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_add_word",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_mul_word =
              GetProcAddress( hCRYPTO, "BN_mul_word" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mul_word",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_lshift =
              GetProcAddress( hCRYPTO, "BN_lshift" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_lshift",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_set_word =
              GetProcAddress( hCRYPTO, "BN_set_word" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_set_word",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_dec2bn =
              GetProcAddress( hCRYPTO, "BN_dec2bn" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_dec2bn",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_bn2dec =
              GetProcAddress( hCRYPTO, "BN_bn2dec" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_bn2dec",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_copy =
              GetProcAddress( hCRYPTO, "BN_copy" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_copy",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_CTX_free =
              GetProcAddress( hCRYPTO, "BN_CTX_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_CTX_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_div =
              GetProcAddress( hCRYPTO, "BN_div" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_div",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_sub =
              GetProcAddress( hCRYPTO, "BN_sub" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_sub",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_value_one =
              GetProcAddress( hCRYPTO, "BN_value_one" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_value_one",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_CTX_new =
              GetProcAddress( hCRYPTO, "BN_CTX_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_CTX_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_CTX_get =
              GetProcAddress( hCRYPTO, "BN_CTX_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_CTX_get",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_CTX_start =
              GetProcAddress( hCRYPTO, "BN_CTX_start" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_CTX_start",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_CTX_end =
              GetProcAddress( hCRYPTO, "BN_CTX_end" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_CTX_end",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_free =
              GetProcAddress( hCRYPTO, "BN_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_hex2bn =
              GetProcAddress( hCRYPTO, "BN_hex2bn" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_hex2bn",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_is_bit_set =
              GetProcAddress( hCRYPTO, "BN_is_bit_set" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_is_bit_set",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_rand =
              GetProcAddress( hCRYPTO, "BN_rand" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_rand",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_bn2hex =
              GetProcAddress( hCRYPTO, "BN_bn2hex" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_bn2hex",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_add =
              GetProcAddress( hCRYPTO, "BN_add" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_add",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_mul =
              GetProcAddress( hCRYPTO, "BN_mul" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mul",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_mod =
              GetProcAddress( hCRYPTO, "BN_mod" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mod",rc);
        }
        if (((FARPROC) p_BN_mod_word =
              GetProcAddress( hCRYPTO, "BN_mod_word" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mod_word",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_mod_mul =
              GetProcAddress( hCRYPTO, "BN_mod_mul" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mod_mul",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_mod_exp =
              GetProcAddress( hCRYPTO, "BN_mod_exp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_mod_exp",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_is_prime =
              GetProcAddress( hCRYPTO, "BN_is_prime" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_is_prime",rc);
            load_error = 1;
        }

        if (((FARPROC) p_MD5_Final =
              GetProcAddress( hCRYPTO, "MD5_Final" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "MD5_Final",rc);
            load_error = 1;
        }
        if (((FARPROC) p_MD5_Update =
              GetProcAddress( hCRYPTO, "MD5_Update" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "MD5_Update",rc);
            load_error = 1;
        }
        if (((FARPROC) p_MD5_Init =
              GetProcAddress( hCRYPTO, "MD5_Init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "MD5_Init",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_clear_free =
              GetProcAddress( hCRYPTO, "BN_clear_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_clear_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_rshift1 =
              GetProcAddress( hCRYPTO, "BN_rshift1" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_rshift1",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BN_clear =
              GetProcAddress( hCRYPTO, "BN_clear" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_clear",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_DigestFinal =
              GetProcAddress( hCRYPTO, "EVP_DigestFinal" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_DigestFinal",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_DigestUpdate =
              GetProcAddress( hCRYPTO, "EVP_DigestUpdate" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_DigestUpdate",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_DigestInit =
              GetProcAddress( hCRYPTO, "EVP_DigestInit" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_DigestInit",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_sha1 =
              GetProcAddress( hCRYPTO, "EVP_sha1" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_sha1",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_md5 =
              GetProcAddress( hCRYPTO, "EVP_md5" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_md5",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_des_ede3_cbc =
              GetProcAddress( hCRYPTO, "EVP_des_ede3_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_des_ede3_cbc",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_PKEY_free =
              GetProcAddress( hCRYPTO, "EVP_PKEY_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_PKEY_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_PKEY_get1_DSA =
              GetProcAddress( hCRYPTO, "EVP_PKEY_get1_DSA" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_PKEY_get1_DSA",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_PKEY_get1_RSA =
              GetProcAddress( hCRYPTO, "EVP_PKEY_get1_RSA" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_PKEY_get1_RSA",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_get_digestbyname =
              GetProcAddress( hCRYPTO, "EVP_get_digestbyname" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_get_digestbyname",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_ripemd160 =
              GetProcAddress( hCRYPTO, "EVP_ripemd160" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_ripemd160",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_new =
              GetProcAddress( hCRYPTO, "RSA_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_public_encrypt =
              GetProcAddress( hCRYPTO, "RSA_public_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_public_encrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_public_decrypt =
              GetProcAddress( hCRYPTO, "RSA_public_decrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_public_decrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_private_encrypt =
              GetProcAddress( hCRYPTO, "RSA_private_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_private_encrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_private_decrypt =
              GetProcAddress( hCRYPTO, "RSA_private_decrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_private_decrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_sign =
              GetProcAddress( hCRYPTO, "RSA_sign" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_sign",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_size =
              GetProcAddress( hCRYPTO, "RSA_size" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_size",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RSA_verify =
              GetProcAddress( hCRYPTO, "RSA_verify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_verify",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_new =
              GetProcAddress( hCRYPTO, "DSA_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DSA_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_SIG_free =
              GetProcAddress( hCRYPTO, "DSA_SIG_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DSA_SIG_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_do_sign =
              GetProcAddress( hCRYPTO, "DSA_do_sign" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DSA_do_sign",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_do_verify =
              GetProcAddress( hCRYPTO, "DSA_do_verify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DSA_do_verify",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DSA_SIG_new =
              GetProcAddress( hCRYPTO, "DSA_SIG_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DSA_SIG_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_HMAC_cleanup =
              GetProcAddress( hCRYPTO, "HMAC_cleanup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "HMAC_cleanup",rc);
        }
        if (((FARPROC) p_HMAC_CTX_cleanup =
              GetProcAddress( hCRYPTO, "HMAC_CTX_cleanup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "HMAC_CTX_cleanup",rc);
        }
        if (((FARPROC) p_HMAC_Final =
              GetProcAddress( hCRYPTO, "HMAC_Final" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "HMAC_Final",rc);
            load_error = 1;
        }
        if (((FARPROC) p_HMAC_Update =
              GetProcAddress( hCRYPTO, "HMAC_Update" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "HMAC_Update",rc);
            load_error = 1;
        }
        if (((FARPROC) p_HMAC_Init =
              GetProcAddress( hCRYPTO, "HMAC_Init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "HMAC_Init",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RAND_add =
              GetProcAddress( hCRYPTO, "RAND_add" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_add",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RAND_bytes =
              GetProcAddress( hCRYPTO, "RAND_bytes" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_bytes",rc);
            load_error = 1;
        }
        if (((FARPROC) p_des_set_key =
              GetProcAddress( hCRYPTO, "des_set_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "des_set_key",rc);
            if (((FARPROC) p_des_set_key =
                  GetProcAddress( hCRYPTO, "_ossl_old_des_set_key" )) == NULL )
            {
                rc = GetLastError() ;
                debug(F111,"OpenSSL Crypto GetProcAddress failed",
                       "_ossl_old_des_set_key",rc);
                load_error = 1;
            }
        }
        if (((FARPROC) p_des_ncbc_encrypt =
              GetProcAddress( hCRYPTO, "des_ncbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "des_ncbc_encrypt",rc);
            if (((FARPROC) p_des_ncbc_encrypt =
                  GetProcAddress( hCRYPTO, "_ossl_old_des_ncbc_encrypt" )) == NULL )
            {
                rc = GetLastError() ;
                debug(F111,"OpenSSL Crypto GetProcAddress failed",
                       "_ossl_old_des_ncbc_encrypt",rc);
                load_error = 1;
            }
        }
        if (((FARPROC) p_des_ede3_cbc_encrypt =
              GetProcAddress( hCRYPTO, "des_ede3_cbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "des_ede3_cbc_encrypt",rc);
            if (((FARPROC) p_des_ede3_cbc_encrypt =
                  GetProcAddress( hCRYPTO, "_ossl_old_des_ede3_cbc_encrypt" )) == NULL )
            {
                rc = GetLastError() ;
                debug(F111,"OpenSSL Crypto GetProcAddress failed",
                       "_ossl_old_des_ede3_cbc_encrypt",rc);
                load_error = 1;
            }
        }
        if (((FARPROC) p_BF_set_key =
              GetProcAddress( hCRYPTO, "BF_set_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BF_set_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_BF_cbc_encrypt =
              GetProcAddress( hCRYPTO, "BF_cbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BF_cbc_encrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RC4_set_key =
              GetProcAddress( hCRYPTO, "RC4_set_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RC4_set_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_RC4 =
              GetProcAddress( hCRYPTO, "RC4" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RC4",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CAST_set_key =
              GetProcAddress( hCRYPTO, "CAST_set_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CAST_set_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CAST_cbc_encrypt =
              GetProcAddress( hCRYPTO, "CAST_cbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CAST_cbc_encrypt",rc);
            load_error = 1;
        }
        if (((FARPROC) p_OBJ_nid2sn =
              GetProcAddress( hCRYPTO, "OBJ_nid2sn" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OBJ_nid2sn",rc);
            load_error = 1;
        }
        if (((FARPROC) p_OBJ_create =
              GetProcAddress( hCRYPTO, "OBJ_create" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OBJ_create",rc);
        }
        if (((FARPROC) p_DH_compute_key =
              GetProcAddress( hCRYPTO, "DH_compute_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DH_compute_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_DH_size =
              GetProcAddress( hCRYPTO, "DH_size" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "DH_size",rc);
            load_error = 1;
        }
        if (((FARPROC) p_ERR_load_crypto_strings =
              GetProcAddress( hCRYPTO, "ERR_load_crypto_strings" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ERR_load_crypto_strings",rc);
            load_error = 1;
        }
        if (((FARPROC) p_CRYPTO_free =
              GetProcAddress( hCRYPTO, "CRYPTO_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_write_DSAPrivateKey =
              GetProcAddress( hCRYPTO, "PEM_write_DSAPrivateKey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_write_DSAPrivateKey",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_write_RSAPrivateKey =
              GetProcAddress( hCRYPTO, "PEM_write_RSAPrivateKey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_write_RSAPrivateKey",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_write_bio_X509 =
              GetProcAddress( hCRYPTO, "PEM_write_bio_X509" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_write_bio_X509",rc);
            load_error = 1;
        }
        if (((FARPROC) p_PEM_read_PrivateKey =
              GetProcAddress( hCRYPTO, "PEM_read_PrivateKey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_read_PrivateKey",rc);
            load_error = 1;
        }
        if (((FARPROC) p_X509_get_default_cert_dir_env =
              GetProcAddress( hCRYPTO, "X509_get_default_cert_dir_env" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_default_cert_dir_env",rc);
        }
        if (((FARPROC) p_X509_get_default_cert_file_env =
              GetProcAddress( hCRYPTO, "X509_get_default_cert_file_env" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_default_cert_file_env",rc);
        }
        if (((FARPROC) p_OpenSSL_add_all_ciphers =
              GetProcAddress( hCRYPTO, "OpenSSL_add_all_ciphers" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OpenSSL_add_all_ciphers",rc);
        }
        if (((FARPROC) p_OpenSSL_add_all_digests =
              GetProcAddress( hCRYPTO, "OpenSSL_add_all_digests" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OpenSSL_add_all_digests",rc);
        }
        if (((FARPROC) p_OPENSSL_add_all_algorithms_noconf =
              GetProcAddress( hCRYPTO, "OPENSSL_add_all_algorithms_noconf" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OPENSSL_add_all_algorithms_noconf",rc);
        }
        if (((FARPROC) p_OPENSSL_add_all_algorithms_conf =
              GetProcAddress( hCRYPTO, "OPENSSL_add_all_algorithms_conf" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OPENSSL_add_all_algorithms_conf",rc);
        }

        if (((FARPROC) p_RSA_blinding_on =
              GetProcAddress( hCRYPTO, "RSA_blinding_on" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RSA_blinding_on",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_aes_256_cbc =
              GetProcAddress( hCRYPTO, "EVP_aes_256_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_aes_256_cbc",rc);
        }
        if (((FARPROC) p_EVP_aes_192_cbc =
              GetProcAddress( hCRYPTO, "EVP_aes_192_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_aes_192_cbc",rc);
        }
        if (((FARPROC) p_EVP_aes_128_cbc =
              GetProcAddress( hCRYPTO, "EVP_aes_128_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_aes_128_cbc",rc);
        }
        if (((FARPROC) p_EVP_rc4 =
              GetProcAddress( hCRYPTO, "EVP_rc4" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_rc4",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_cast5_cbc =
              GetProcAddress( hCRYPTO, "EVP_cast5_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_cast5_cbc",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_bf_cbc =
              GetProcAddress( hCRYPTO, "EVP_bf_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_bf_cbc",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_des_cbc =
              GetProcAddress( hCRYPTO, "EVP_des_cbc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_des_cbc",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_enc_null =
              GetProcAddress( hCRYPTO, "EVP_enc_null" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_enc_null",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_CipherInit =
              GetProcAddress( hCRYPTO, "EVP_CipherInit" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CipherInit",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_CIPHER_CTX_init =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_init",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_CIPHER_CTX_cleanup =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_cleanup" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_cleanup",rc);
            load_error = 1;
        }
        if (((FARPROC) p_EVP_CIPHER_CTX_set_key_length =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_set_key_length" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_set_key_length",rc);
            load_error = 1;
        }

        if (((FARPROC) p_EVP_CIPHER_CTX_key_length =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_key_length" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_key_length",rc);
            load_error = 1;
        }

        if (((FARPROC) p_EVP_Cipher =
              GetProcAddress( hCRYPTO, "EVP_Cipher" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_Cipher",rc);
            load_error = 1;
        }

        if (((FARPROC) p_EVP_CIPHER_CTX_iv_length =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_iv_length" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_iv_length",rc);
            load_error = 1;
        }

        if (((FARPROC) p_EVP_CIPHER_CTX_get_app_data =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_get_app_data" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_get_app_data",rc);
            load_error = 1;
        }

        if (((FARPROC) p_EVP_CIPHER_CTX_set_app_data =
              GetProcAddress( hCRYPTO, "EVP_CIPHER_CTX_set_app_data" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "EVP_CIPHER_CTX_set_app_data",rc);
            load_error = 1;
        }

        if ( load_error ) {
            ck_crypto_loaddll_eh();
            return 0;
        }
    }
    if (((FARPROC) p_EVP_MD_size =
	  GetProcAddress( hCRYPTO, "EVP_MD_size" )) == NULL )
    {
	rc = GetLastError() ;
	debug(F111,"OpenSSL SSL GetProcAddress failed",
	       "EVP_MD_size",rc);
    }
#else /* NT */
    exe_path = GetLoadPath();
    sprintf(path, "%.*s%s.DLL", (int)get_dir_len(exe_path), exe_path,dllname);
    rc = DosLoadModule(errbuf, 256, path, &hCRYPTO);
    if (rc) {
        debug(F111, "OpenSSL LoadLibrary failed",path,rc) ;
        rc = DosLoadModule(errbuf, 256, dllname, &hCRYPTO);
    }
    if ( rc ) {
        debug(F111, "OpenSSL LoadLibrary failed",errbuf,rc) ;
        return(0);
    } else {
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ERR_print_errors",
                                  (PFN*) &p_ERR_print_errors) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_print_errors",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ERR_print_errors_fp",
                                  (PFN*) &p_ERR_print_errors_fp) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_print_errors_fp",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ERR_get_error",
                                  (PFN*) &p_ERR_get_error) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_get_error",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ERR_error_string",
                                  (PFN*) &p_ERR_error_string) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","ERR_error_string",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_printf",
                                  (PFN*) &p_BIO_printf) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_printf",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_ctrl",
                                  (PFN*) &p_BIO_ctrl) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_ctrl",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_new",
                                  (PFN*) &p_BIO_new) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_new",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_s_file",
                                  (PFN*) &p_BIO_s_file) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_s_mem",
                                  (PFN*) &p_BIO_s_mem) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_mem",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_s_null",
                                  (PFN*) &p_BIO_s_null) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_s_null",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_read",
                                  (PFN*) &p_BIO_read) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_read",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_write",
                                  (PFN*) &p_BIO_write) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_write",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_free",
                                  (PFN*) &p_BIO_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_free_all",
                                  (PFN*) &p_BIO_free_all) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_free_all",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BIO_new_file",
                                  (PFN*) &p_BIO_new_file) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","BIO_new_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_issuer_name",
                                  (PFN*) &p_X509_get_issuer_name) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_issuer_name",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_print_ex",
                                  (PFN*) &p_X509_print_ex) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_print_ex",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_verify_cert_error_string",
                                  (PFN*) &p_X509_verify_cert_error_string) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_verify_cert_error_string",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_NAME_oneline",
                                  (PFN*) &p_X509_NAME_oneline) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_oneline",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_NAME_print_ex",
                                  (PFN*) &p_X509_NAME_print_ex) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_print_ex",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_NAME_get_text_by_NID",
                                  (PFN*) &p_X509_NAME_get_text_by_NID) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_NAME_get_text_by_NID",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_subject_name",
                                  (PFN*) &p_X509_get_subject_name) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_subject_name",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_new",
                                  (PFN*) &p_X509_STORE_CTX_new) )
        {
            /* New to 0.9.5 */
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_new",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_free",
                                  (PFN*) &p_X509_STORE_CTX_free) )
        {
            /* New to 0.9.5 */
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_free",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_get_current_cert",
                                  (PFN*) &p_X509_STORE_CTX_get_current_cert) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_current_cert",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_get_error",
                                  (PFN*) &p_X509_STORE_CTX_get_error) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_error",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_get_error_depth",
                                  (PFN*) &p_X509_STORE_CTX_get_error_depth) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_get_error_depth",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_free",
                                  (PFN*) &p_X509_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_default_cert_dir",
                                  (PFN*) &p_X509_get_default_cert_dir) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","X509_get_default_cert_dir",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "RSA_free",
                                  (PFN*) &p_RSA_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","RSA_free",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "RSA_generate_key",
                                  (PFN*) &p_RSA_generate_key) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","RSA_generate_key",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DH_new",
                                  (PFN*) &p_DH_new) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_new",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DH_free",
                                  (PFN*) &p_DH_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DH_generate_parameters",
                                  (PFN*) &p_DH_generate_parameters) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_generate_parameters",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DH_generate_key",
                                  (PFN*) &p_DH_generate_key) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DH_generate_key",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DSA_free",
                                  (PFN*) &p_DSA_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DSA_generate_parameters",
                                  (PFN*) &p_DSA_generate_parameters) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_generate_parameters",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "DSA_generate_key",
                                  (PFN*) &p_DSA_generate_key) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed","DSA_generate_key",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "d2i_DHparams",
                                  (PFN*) &p_d2i_DHparams) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "d2i_DHparams",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "PEM_ASN1_read_bio",
                                  (PFN*) &p_PEM_ASN1_read_bio) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_ASN1_read_bio",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "BN_bin2bn",
                                  (PFN*) &p_BN_bin2bn) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "BN_bin2bn",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ASN1_TIME_print",
                                  (PFN*) &p_ASN1_TIME_print) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_TIME_print",rc);
            load_error = 1;
        }

        if (rc = DosQueryProcAddr( hCRYPTO, 0, "sk_GENERAL_NAME_free",
                                  (PFN*) &p_sk_GENERAL_NAME_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509V3_EXT_cleanup",
                                  (PFN*) &p_X509V3_EXT_cleanup) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_EXT_cleanup",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "sk_GENERAL_NAME_value",
                                  (PFN*) &p_sk_GENERAL_NAME_value) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_value",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "sk_GENERAL_NAME_num",
                                  (PFN*) &p_sk_GENERAL_NAME_num) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_GENERAL_NAME_num",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509V3_EXT_d2i",
                                  (PFN*) &p_X509V3_EXT_d2i) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_EXT_d2i",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509V3_add_standard_extensions",
                                  (PFN*) &p_X509V3_add_standard_extensions) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509V3_add_standard_extensions",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_ext",
                                  (PFN*) &p_X509_get_ext) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_ext",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_ext_by_NID",
                                  (PFN*) &p_X509_get_ext_by_NID) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_ext_by_NID",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ASN1_INTEGER_get",
                                  (PFN*) &p_ASN1_INTEGER_get) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_INTEGER_get",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ASN1_INTEGER_cmp",
                                  (PFN*) &p_ASN1_INTEGER_cmp) )
        {
            /* New to OpenSSL 0.9.5 */
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_INTEGER_cmp",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "ASN1_STRING_cmp",
                                  (PFN*) &p_ASN1_STRING_cmp) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ASN1_STRING_cmp",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_serialNumber",
                                  (PFN*) &p_X509_get_serialNumber) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_serialNumber",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "sk_X509_REVOKED_value",
                                  (PFN*) &p_sk_X509_REVOKED_value) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_X509_REVOKED_value",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "sk_X509_REVOKED_num",
                                  (PFN*) &p_sk_X509_REVOKED_num) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_X509_REVOKED_num",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_cmp_current_time",
                                  (PFN*) &p_X509_cmp_current_time) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_cmp_current_time",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_OBJECT_free_contents",
                                  (PFN*) &p_X509_OBJECT_free_contents) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_OBJECT_free_contents",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_set_error",
                                  (PFN*) &p_X509_STORE_CTX_set_error) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_set_error",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_CRL_verify",
                                  (PFN*) &p_X509_CRL_verify) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_CRL_verify",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_get_pubkey",
                                  (PFN*) &p_X509_get_pubkey) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_pubkey",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_cleanup",
                                  (PFN*) &p_X509_STORE_CTX_cleanup) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_cleanup",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_get_by_subject",
                                  (PFN*) &p_X509_STORE_get_by_subject) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_get_by_subject",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_CTX_init",
                                  (PFN*) &p_X509_STORE_CTX_init) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_CTX_init",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "PEM_read_X509",
                                  (PFN*) &p_PEM_read_X509) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_read_X509",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_LOOKUP_hash_dir",
                                  (PFN*) &p_X509_LOOKUP_hash_dir) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_hash_dir",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_LOOKUP_ctrl",
                                  (PFN*) &p_X509_LOOKUP_ctrl) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_ctrl",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_add_lookup",
                                  (PFN*) &p_X509_STORE_add_lookup) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_add_lookup",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_LOOKUP_file",
                                  (PFN*) &p_X509_LOOKUP_file) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_LOOKUP_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_new",
                                  (PFN*) &p_X509_STORE_new) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_new",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_free",
                                  (PFN*) &p_X509_STORE_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_load_locations",
                                  (PFN*) &p_X509_STORE_load_locations) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_load_locations",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_STORE_set_default_paths",
                                  (PFN*) &p_X509_STORE_set_default_paths) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_STORE_set_default_paths",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_subject_name_hash",
                                  (PFN*) &p_X509_subject_name_hash) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_subject_name_hash",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "X509_issuer_name_hash",
                                  (PFN*) &p_X509_issuer_name_hash) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_issuer_name_hash",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0, "CRYPTO_set_mem_functions",
                                  (PFN*) &p_CRYPTO_set_mem_functions) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_mem_functions",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "CRYPTO_set_locking_callback",
                                  (PFN*) &p_CRYPTO_set_locking_callback) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_locking_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "CRYPTO_set_dynlock_create_callback",
                                  (PFN*) &p_CRYPTO_set_dynlock_create_callback) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_create_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "CRYPTO_set_dynlock_lock_callback",
                                  (PFN*) &p_CRYPTO_set_dynlock_lock_callback) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_lock_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "CRYPTO_set_dynlock_destroy_callback",
                                  (PFN*) &p_CRYPTO_set_dynlock_destroy_callback) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_set_dynlock_destroy_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "CRYPTO_num_locks",
                                  (PFN*) &p_CRYPTO_num_locks) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "CRYPTO_num_locks",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_screen",
                                  (PFN*) &p_RAND_screen) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_screen",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_status",
                                  (PFN*) &p_RAND_status) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_status",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_file_name",
                                  (PFN*) &p_RAND_file_name) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_file_name",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_egd",
                                  (PFN*) &p_RAND_egd) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_egd",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_load_file",
                                  (PFN*) &p_RAND_load_file) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_load_file",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_write_file",
                                  (PFN*) &p_RAND_write_file) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_write_file",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "RAND_seed",
                                  (PFN*) &p_RAND_seed) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "RAND_seed",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "sk_num",
                                  (PFN*) &p_sk_num) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_num",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "sk_value",
                                  (PFN*) &p_sk_value) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_value",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "sk_free",
                                  (PFN*) &p_sk_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_free",rc);
        }
#ifdef ZLIB
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "COMP_zlib",
                                  (PFN*) &p_COMP_zlib) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "COMP_zlib",rc);
        }
#endif /* ZLIB */
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "COMP_rle",
                                  (PFN*) &p_COMP_rle) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "COMP_rle",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "ERR_peek_error",
                                  (PFN*) &p_ERR_peek_error) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "ERR_peek_error",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "sk_pop_free",
                                  (PFN*) &p_sk_pop_free) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "sk_pop_free",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "PEM_read_bio_X509",
                                  (PFN*) &p_PEM_read_bio_X509) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "PEM_read_bio_X509",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "SSLeay",
                                  (PFN*) &p_SSLeay) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SSLeay",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "SSLeay_version",
                                  (PFN*) &p_SSLeay_version) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "SSLeay_version",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "X509_get_default_cert_dir_env",
                                  (PFN*) &p_X509_get_default_cert_dir_env) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_default_cert_dir_env",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "X509_get_default_cert_file_env",
                                  (PFN*) &p_X509_get_default_cert_file_env) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "X509_get_default_cert_file_env",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "OpenSSL_add_all_ciphers",
                                  (PFN*) &p_OpenSSL_add_all_ciphers) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OpenSSL_add_all_ciphers",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "OpenSSL_add_all_digests",
                                  (PFN*) &p_OpenSSL_add_all_digests) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OpenSSL_add_all_digests",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "OPENSSL_add_all_algorithms_noconf",
                                  (PFN*) &p_OPENSSL_add_all_algorithms_noconf) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OPENSSL_add_all_algorithms_noconf",rc);
        }
        if (rc = DosQueryProcAddr( hCRYPTO, 0,
                                  "OPENSSL_add_all_algorithms_conf",
                                  (PFN*) &p_OPENSSL_add_all_algorithms_conf) )
        {
            debug(F111,"OpenSSL Crypto GetProcAddress failed",
                   "OPENSSL_add_all_algorithms_conf",rc);
        }

        if ( load_error ) {
            ck_crypto_loaddll_eh();
            return 0;
        }
    }
#endif /* NT */

    crypto_dll_loaded = 1;
    if ( deblog ) {
        printf("OpenSSL Crypto DLL Loaded\n");
        debug(F100,"OpenSSL Crypto DLL Loaded","",0);
    }

    /* Initialize memory allocation functions so */
    /* we do not have RTL incompatibilities      */
    CRYPTO_set_mem_functions(malloc,realloc,free);

    /* Initialize multithread support */
    {
        int i,n;

        n = CRYPTO_num_locks();

#ifdef NT
        lock_cs = malloc(n * sizeof(HANDLE));
#else
        lock_cs = malloc(n * sizeof(HMTX));
#endif
		if (lock_cs == NULL)
			fatal("Unable to malloc memory for CRYPTO_num_locks\n");

        for (i=0; i<n; i++) {
#ifdef NT
            lock_cs[i]=CreateMutex(NULL,FALSE,NULL);
#else
            DosCreateMutexSem( NULL, &lock_cs[i], 0, FALSE );
#endif
        }

#ifdef NT
        CRYPTO_set_locking_callback(win32_locking_callback);
	CRYPTO_set_dynlock_create_callback(win32_dynlock_create_callback);
	CRYPTO_set_dynlock_lock_callback(win32_dynlock_lock_callback);
	CRYPTO_set_dynlock_destroy_callback(win32_dynlock_destroy_callback);
#else
        CRYPTO_set_locking_callback((void (*)(int,int,char *,int))os2_locking_callback);
	CRYPTO_set_dynlock_create_callback(os2_dynlock_create_callback);
	CRYPTO_set_dynlock_lock_callback(os2_dynlock_lock_callback);
	CRYPTO_set_dynlock_destroy_callback(os2_dynlock_destroy_callback);
#endif
        /* id callback defined */
    }

    /* initialize PRNG */
    RAND_screen();
    return(1);
}

int
ck_crypto_unloaddll( void )
{
    if ( !crypto_dll_loaded )
        return(1);

    /* Clean up Semaphores */
    {
        int i,n;

        n = CRYPTO_num_locks();

	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);
        CRYPTO_set_locking_callback(NULL);
        for (i=0; i<n; i++) {
#ifdef NT
            CloseHandle(lock_cs[i]);
#else
            DosCloseMutexSem(lock_cs[i]);
#endif
        }
        free(lock_cs);
        lock_cs = 0;
    }

    /* unload dlls */
    ck_crypto_loaddll_eh();

    /* success */
    crypto_dll_loaded = 0;
    return(1);
}
#endif /* SSLDLL */
#endif /* CK_SSL */

#undef malloc
#undef realloc
#undef free
#undef strdup

#ifdef NOT_KERMIT
static void
fatal(char *msg) {
    if (!msg) msg = "";

    printf(msg);
    exit(1);        /* Exit indicating failure */
}
#endif /* NOT_KERMIT */

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

#endif /* OS2 */
