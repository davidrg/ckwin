/*
  C K O S S L . C --  OpenSSL Interface for Kermit 95

  Copyright (C) 2000, 2004 Trustees of Columbia University in the City of New
  York.

  Copyright (C) 2006, 2007  Secure Endpoints Inc.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City
*/

#include "ckcdeb.h"
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
HINSTANCE hSSL = NULL;
#else /* NT */
HMODULE hSSL = NULL;
#endif /* NT */

int ssl_dll_loaded=0;

#ifdef NT
HINSTANCE hX5092UID=NULL;
#define SEM_INDEFINITE_WAIT INFINITE
#else /* NT */
HMODULE hX5092UID=NULL;
#endif /* NT */

#ifdef NOT_KERMIT
static int deblog = 0;
#ifdef debug
#undef debug
#endif
#define debug(x,a,b,c)
#ifdef printf
#undef printf
#endif
static int ssl_finished_messages = 0;
static unsigned long startflags = 0;
#endif /* NOT_KERMIT */

int     (*p_SSL_get_error)(SSL *ssl,int num)=NULL;
int     (*p_SSL_read)(SSL *ssl,char *buf,int num)=NULL;
int     (*p_SSL_connect)(SSL *ssl)=NULL;
int     (*p_SSL_set_fd)(SSL *s, int fd)=NULL;
void    (*p_SSL_free)(SSL *ssl)=NULL;
int     (*p_SSL_shutdown)(SSL *s)=NULL;
int     (*p_SSL_write)(SSL *ssl,char *buf,int num)=NULL;
int     (*p_SSL_pending)(SSL *ssl)=NULL;
int     (*p_SSL_peek)(SSL *ssl,char * buf,int num)=NULL;
void    (*p_SSL_load_error_strings)(void )=NULL;
X509 *  (*p_SSL_get_peer_certificate)(SSL *s)=NULL;
const char * (*p_SSL_CIPHER_get_name)(SSL_CIPHER *c)=NULL;
int     (*p_SSL_CIPHER_get_bits)(SSL_CIPHER *c,int *)=NULL;
char *  (*p_SSL_CIPHER_get_version)(SSL_CIPHER *c)=NULL;
SSL_CIPHER * (*p_SSL_get_current_cipher)(SSL *s)=NULL;
char *  (*p_SSL_get_shared_ciphers)(SSL *s, char *buf, int len)=NULL;
char *  (*p_SSL_get_ciphers)(SSL *s)=NULL;
const char * (*p_SSL_get_cipher_list)(SSL *s, int i)=NULL;
int     (*p_SSL_CTX_set_default_verify_paths)(SSL_CTX *ctx)=NULL;
int     (*p_SSL_use_RSAPrivateKey_file)(SSL *ssl, char *file, int type)=NULL;
int     (*p_SSL_use_DSAPrivateKey_file)(SSL *ssl, char *file, int type)=NULL;
int     (*p_SSL_use_PrivateKey_file)(SSL *ssl, char *file, int type)=NULL;
int     (*p_SSL_use_certificate_file)(SSL *ssl, char *file, int type)=NULL;
int     (*p_SSL_CTX_use_PrivateKey_file)(SSL_CTX *ctx, char *file, int type)=NULL;
int     (*p_SSL_CTX_use_certificate_file)(SSL_CTX *ctx, char *file, int type)=NULL;
int     (*p_SSL_CTX_check_private_key)(SSL_CTX *ctx)=NULL;
int     (*p_SSL_check_private_key)(SSL *ssl)=NULL;
void    (*p_SSL_set_verify)(SSL *s, int mode, int (*callback) ())=NULL;
void    (*p_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode, int (*callback) ())=NULL;
SSL *   (*p_SSL_new)(SSL_CTX *ctx)=NULL;
long    (*p_SSL_CTX_ctrl)(SSL_CTX *ctx,int cmd, long larg, char *parg)=NULL;
SSL_CTX * (*p_SSL_CTX_new)(SSL_METHOD *meth)=NULL;
void    (*p_SSL_CTX_free)(SSL_CTX *ctx)=NULL;
void    (*p_SSL_CTX_set_default_passwd_cb)(SSL_CTX *ctx,int (*cb)())=NULL;
SSL_METHOD * (*p_SSLv23_method)(void)=NULL;
SSL_METHOD * (*p_SSLv3_method)(void)=NULL;
SSL_METHOD * (*p_TLSv1_method)(void)=NULL;
SSL_METHOD * (*p_SSLv23_client_method)(void)=NULL;
SSL_METHOD * (*p_SSLv3_client_method)(void)=NULL;
SSL_METHOD * (*p_TLSv1_client_method)(void)=NULL;
SSL_METHOD * (*p_SSLv23_server_method)(void)=NULL;
SSL_METHOD * (*p_SSLv3_server_method)(void)=NULL;
SSL_METHOD * (*p_TLSv1_server_method)(void)=NULL;
void (*p_SSL_library_init)(void)=NULL;
char *  (*p_SSL_state_string)(const SSL *s)=NULL;
char *  (*p_SSL_state_string_long)(const SSL *s)=NULL;
int     (*p_SSL_accept)(SSL *ssl)=NULL;
int     (*p_SSL_set_cipher_list)(SSL *ssl,char *s)=NULL;
void (*p_SSL_CTX_set_tmp_rsa_callback)(SSL_CTX *ctx,
     RSA *(*cb)(SSL *ssl,int is_export, int keylength))=NULL;
void (*p_SSL_set_tmp_rsa_callback)(SSL *ssl,
     RSA *(*cb)(SSL *ssl,int is_export,int keylength))=NULL;
void (*p_SSL_CTX_set_tmp_dh_callback)(SSL_CTX *ctx,
     DH *(*dh)(SSL *ssl,int is_export,int keylength))=NULL;
void (*p_SSL_set_tmp_dh_callback)(SSL *ssl,
     DH *(*dh)(SSL *ssl,int is_export, int keylength))=NULL;

int  (*p_SSL_CTX_load_verify_locations)(SSL_CTX *,char *,char *)=NULL;
void (*p_SSL_CTX_set_default_passwd_cb_userdata)(SSL_CTX *, void *)=NULL;
int  (*p_X509_to_user)(X509 *peer_cert, char *userid, int len)=NULL;
int  (*p_X509_userok)(X509 *peer_cert, const char *userid)=NULL;
BIO *(*p_SSL_get_wbio)(SSL *)=NULL;
size_t (*p_SSL_get_finished)(SSL *,void *buf, size_t count)=NULL;
size_t (*p_SSL_get_peer_finished)(SSL *,void *buf, size_t count)=NULL;
void (*p_SSL_copy_session_id)(SSL *to,SSL *from)=NULL;
int  (*p_SSL_renegotiate)(SSL * ssl)=NULL;
int  (*p_SSL_set_session)(SSL *, SSL_SESSION *)=NULL;
SSL_SESSION * (*p_SSL_get_session)(SSL *)=NULL;
int (*p_SSL_CTX_set_session_id_context)(SSL_CTX *ctx,
    const unsigned char *sid_ctx,unsigned int sid_ctx_len)=NULL;
EVP_PKEY * (*p_SSL_get_privatekey)(SSL *)=NULL;
X509 * (*p_SSL_get_certificate)(SSL *)=NULL;
int (*p_SSL_clear)(SSL *)=NULL;
void (*p_SSL_set_accept_state)(SSL *)=NULL;
char * (*p_SSL_CIPHER_description)(SSL_CIPHER *,char *buf,int size)=NULL;
void (*p_SSL_CTX_set_info_callback)(SSL_CTX *ctx, void (*cb)(const SSL *ssl,int type,int val))=NULL;

#ifdef SSL_KRB5
KSSL_CTX * (*p_kssl_ctx_new)(void)=NULL;
KSSL_CTX * (*p_kssl_ctx_free)(KSSL_CTX *kssl_ctx)=NULL;
krb5_error_code (*p_kssl_ctx_setstring)(KSSL_CTX *kssl_ctx, int which, char *text)=NULL;
krb5_error_code (*p_kssl_ctx_setprinc)(KSSL_CTX *kssl_ctx, int which,
                                      krb5_data *realm, krb5_data *entity)=NULL;
#endif /* SSL_KRB5 */
int (*p_SSL_COMP_add_compression_method)(int id,COMP_METHOD *cm)=NULL;
int (*p_SSL_CTX_use_certificate_chain_file)(SSL_CTX *ctx, const char *file)=NULL;

static int ssl_shutdown_in_progress = 0;

int
ck_SSL_get_error(SSL *ssl,int num)
{
    if ( p_SSL_get_error ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_get_error(ssl,num);
        ReleaseSSLMutex();
#else
        rc = p_SSL_get_error(ssl,num);
#endif
        return(rc);
    }
    return(0);
}

int
ck_SSL_read(SSL *ssl,char *buf,int num)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_read ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_read(ssl,buf,num);;
        ReleaseSSLMutex();
#else
        rc = p_SSL_read(ssl,buf,num);;
#endif
        return(rc);
    }
    return(0);
}

int
ck_SSL_peek(SSL *ssl,char *buf,int num)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_peek ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_peek(ssl,buf,num);
        ReleaseSSLMutex();
#else
        rc = p_SSL_peek(ssl,buf,num);
#endif
        return(rc);
    }
    return(0);
}

int
ck_SSL_connect(SSL *ssl)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_connect ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_connect(ssl);
        ReleaseSSLMutex();
#else
        rc = p_SSL_connect(ssl);
#endif
        return(rc);
    }
    return(0);
}

int
ck_SSL_set_fd(SSL *ssl, int fd)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_set_fd ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_set_fd(ssl,fd);
        ReleaseSSLMutex();
#else
        rc = p_SSL_set_fd(ssl,fd);
#endif
        return(rc);
    }
    return(0);
}

void
ck_SSL_free(SSL *ssl)
{
    if ( p_SSL_free ) {
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return;
        p_SSL_free(ssl);
        ReleaseSSLMutex();
#else
        p_SSL_free(ssl);
#endif
    }
}

int
ck_SSL_shutdown(SSL *s)
{
    int rc = 0;
    if ( p_SSL_shutdown ) {
        ssl_shutdown_in_progress = 1;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_shutdown(s);
        ReleaseSSLMutex();
#else
        rc = p_SSL_shutdown(s);
#endif
        ssl_shutdown_in_progress = 0;
    }
    return(rc);
}

int
ck_SSL_write(SSL *ssl,char *buf,int num)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_write ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_write(ssl,buf,num);
        ReleaseSSLMutex();
#else
        rc = p_SSL_write(ssl,buf,num);
#endif
        return(rc);
    }
    return(0);
}

int
ck_SSL_pending(SSL *ssl)
{
    if ( ssl && !ssl_shutdown_in_progress && p_SSL_pending ) {
        int rc;
#ifdef SSL_MUTEX
        if (RequestSSLMutex(20000))
            return(0);
        rc = p_SSL_pending(ssl);
        ReleaseSSLMutex();
#else
        rc = p_SSL_pending(ssl);
#endif
        return(rc);
    }
    return(0);
}

void
ck_SSL_load_error_strings(void )
{
    if ( p_SSL_load_error_strings )
        p_SSL_load_error_strings();
}

X509 *
ck_SSL_get_peer_certificate(SSL *s)
{
    if ( p_SSL_get_peer_certificate )
        return p_SSL_get_peer_certificate(s);
    return(NULL);
}

const char *
ck_SSL_CIPHER_get_name(SSL_CIPHER *c)
{
    if ( p_SSL_CIPHER_get_name )
        return p_SSL_CIPHER_get_name(c);
    return(NULL);
}

int
ck_SSL_CIPHER_get_bits(SSL_CIPHER *c, int * alg_bits)
{
    if ( p_SSL_CIPHER_get_bits )
        return p_SSL_CIPHER_get_bits(c,alg_bits);
    return(0);
}

char *
ck_SSL_CIPHER_get_version(SSL_CIPHER *c)
{
    if ( p_SSL_CIPHER_get_version )
        return p_SSL_CIPHER_get_version(c);
    return(NULL);
}

SSL_CIPHER *
ck_SSL_get_current_cipher(SSL *s)
{
    if ( p_SSL_get_current_cipher )
        return p_SSL_get_current_cipher(s);
    return NULL;
}

char *
ck_SSL_get_shared_ciphers(SSL *s, char *buf, int len)
{
    if ( p_SSL_get_shared_ciphers )
        return(p_SSL_get_shared_ciphers(s,buf,len));
    else
        return(NULL);
}

char *
ck_SSL_get_ciphers(SSL *s)
{
    if ( p_SSL_get_ciphers )
        return(p_SSL_get_ciphers(s));
    else
        return(NULL);
}

const char *
ck_SSL_get_cipher_list(SSL *s, int i)
{
    if ( p_SSL_get_cipher_list )
        return(p_SSL_get_cipher_list(s,i));
    else
        return(NULL);
}

int
ck_SSL_CTX_set_default_verify_paths(SSL_CTX *ctx)
{
    if ( p_SSL_CTX_set_default_verify_paths )
        return p_SSL_CTX_set_default_verify_paths(ctx);
    return 0;
}

int
ck_SSL_use_RSAPrivateKey_file(SSL *ssl, char *file, int type)
{
    if ( p_SSL_use_RSAPrivateKey_file )
        return p_SSL_use_RSAPrivateKey_file(ssl, file, type);
    return(0);
}

int
ck_SSL_use_DSAPrivateKey_file(SSL *ssl, char *file, int type)
{
    if ( p_SSL_use_DSAPrivateKey_file )
        return p_SSL_use_DSAPrivateKey_file(ssl, file, type);
    return(0);
}

int
ck_SSL_use_PrivateKey_file(SSL *ssl, char *file, int type)
{
    if ( p_SSL_use_PrivateKey_file )
        return p_SSL_use_PrivateKey_file(ssl, file, type);
    return(0);
}

int
ck_SSL_use_certificate_file(SSL *ssl, char *file, int type)
{
    if ( p_SSL_use_certificate_file )
        return p_SSL_use_certificate_file(ssl,file,type);
    return(0);
}

int
ck_SSL_CTX_use_certificate_chain_file(SSL_CTX *ctx, const char *file)
{
    if ( p_SSL_CTX_use_certificate_chain_file )
        return p_SSL_CTX_use_certificate_chain_file(ctx,file);
    return(0);
}

int
ck_SSL_CTX_use_PrivateKey_file(SSL_CTX *ssl, char *file, int type)
{
    if ( p_SSL_CTX_use_PrivateKey_file )
        return p_SSL_CTX_use_PrivateKey_file(ssl, file, type);
    return(0);
}

int
ck_SSL_CTX_use_certificate_file(SSL_CTX *ssl, char *file, int type)
{
    if ( p_SSL_CTX_use_certificate_file )
        return p_SSL_CTX_use_certificate_file(ssl,file,type);
    return(0);
}

int
ck_SSL_CTX_check_private_key(SSL_CTX *ctx) {
    if ( p_SSL_CTX_check_private_key )
        return p_SSL_CTX_check_private_key(ctx);
    return(0);
}

int
ck_SSL_check_private_key(SSL *ssl) {
    if ( p_SSL_check_private_key )
        return p_SSL_check_private_key(ssl);
    return(0);
}

void
ck_SSL_set_verify(SSL *s, int mode, int (*callback) ())
{
    if ( p_SSL_set_verify )
        p_SSL_set_verify(s,mode,callback);
}

void
ck_SSL_CTX_set_verify(SSL_CTX *s, int mode, int (*callback) ())
{
    if ( p_SSL_CTX_set_verify )
        p_SSL_CTX_set_verify(s,mode,callback);
}

SSL *
ck_SSL_new(SSL_CTX *ctx)
{
    if ( p_SSL_new )
        return p_SSL_new(ctx);
    return(NULL);
}

long
ck_SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, char *parg)
{
    if ( p_SSL_CTX_ctrl )
        return p_SSL_CTX_ctrl(ctx,cmd,larg,parg);
    return(0);
}

SSL_CTX *
ck_SSL_CTX_new(SSL_METHOD *meth)
{
    if ( p_SSL_CTX_new )
        return p_SSL_CTX_new(meth);
    return(NULL);
}

void
ck_SSL_CTX_free(SSL_CTX * ctx)
{
    if ( p_SSL_CTX_free )
        p_SSL_CTX_free(ctx);
}

void
ck_SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx,int (*cb)())
{
    if ( p_SSL_CTX_set_default_passwd_cb )
        p_SSL_CTX_set_default_passwd_cb(ctx,cb);
}

SSL_METHOD *
ck_SSLv23_method(void)
{
    if ( p_SSLv23_method )
        return p_SSLv23_method();
    return(NULL);
}

SSL_METHOD *
ck_SSLv3_method(void)
{
    if ( p_SSLv3_method )
        return p_SSLv3_method();
    return(NULL);
}

SSL_METHOD *
ck_TLSv1_method(void)
{
    if ( p_TLSv1_method )
        return p_TLSv1_method();
    return(NULL);
}

SSL_METHOD *
ck_SSLv23_client_method(void)
{
    if ( p_SSLv23_client_method )
        return p_SSLv23_client_method();
    return(NULL);
}

SSL_METHOD *
ck_SSLv3_client_method(void)
{
    if ( p_SSLv3_client_method )
        return p_SSLv3_client_method();
    return(NULL);
}

SSL_METHOD *
ck_TLSv1_client_method(void)
{
    if ( p_TLSv1_client_method )
        return p_TLSv1_client_method();
    return(NULL);
}

SSL_METHOD *
ck_SSLv23_server_method(void)
{
    if ( p_SSLv23_server_method )
        return p_SSLv23_server_method();
    return(NULL);
}

SSL_METHOD *
ck_SSLv3_server_method(void)
{
    if ( p_SSLv3_server_method )
        return p_SSLv3_server_method();
    return(NULL);
}

SSL_METHOD *
ck_TLSv1_server_method(void)
{
    if ( p_TLSv1_server_method )
        return p_TLSv1_server_method();
    return(NULL);
}

void
ck_SSL_library_init(void)
{
    if ( p_SSL_library_init )
        p_SSL_library_init();
}

char *
ck_SSL_state_string(const SSL *s)
{
    if ( p_SSL_state_string )
        return p_SSL_state_string(s);
    return NULL;
}

char *
ck_SSL_state_string_long(const SSL *s)
{
    if ( p_SSL_state_string_long )
        return p_SSL_state_string_long(s);
    return NULL;
}

int
ck_SSL_accept(SSL *ssl)
{
    if ( p_SSL_accept )
        return p_SSL_accept(ssl);
    return(0);
}

int
ck_SSL_set_cipher_list(SSL *ssl,char *s)
{
    if ( p_SSL_set_cipher_list )
        return p_SSL_set_cipher_list(ssl,s);
    return(0);
}

void
ck_SSL_CTX_set_tmp_rsa_callback(SSL_CTX *ctx,
         RSA *(*cb)(SSL *ssl,int is_export, int keylength))
{
    if ( p_SSL_CTX_set_tmp_rsa_callback )
        p_SSL_CTX_set_tmp_rsa_callback(ctx,cb);
}

void
ck_SSL_set_tmp_rsa_callback(SSL *ssl,
         RSA *(*cb)(SSL *ssl,int is_export, int keylength))
{
    if ( p_SSL_set_tmp_rsa_callback )
        p_SSL_set_tmp_rsa_callback(ssl,cb);
}

void
ck_SSL_CTX_set_tmp_dh_callback(SSL_CTX *ctx,
         DH *(*dh)(SSL *ssl,int is_export, int keylength))
{
    if ( p_SSL_CTX_set_tmp_dh_callback )
        p_SSL_CTX_set_tmp_dh_callback(ctx,dh);
}

void
ck_SSL_set_tmp_dh_callback(SSL *ssl,
         DH *(*dh)(SSL *ssl,int is_export, int keylength))
{
    if ( p_SSL_set_tmp_dh_callback )
        p_SSL_set_tmp_dh_callback(ssl,dh);

}

int
ck_SSL_CTX_load_verify_locations(SSL_CTX * ctx,char *f,char *p)
{
    if ( p_SSL_CTX_load_verify_locations )
        return(p_SSL_CTX_load_verify_locations(ctx,f,p));
    return(0);
}

void
ck_SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX * ctx, void * userdata)
{
    if ( p_SSL_CTX_set_default_passwd_cb_userdata )
        p_SSL_CTX_set_default_passwd_cb_userdata(ctx,userdata);
}

int
ck_X509_userok(X509 *peer_cert, const char *userid)
{
    if ( p_X509_userok )
        return(p_X509_userok(peer_cert,userid));
    else
        return(0);
}

int
ck_X509_to_user(X509 *peer_cert, char *userid, int len)
{
    if ( p_X509_to_user )
        return(p_X509_to_user(peer_cert,userid,len));
    else
        return(-1);
}

BIO *
ck_SSL_get_wbio(SSL * s)
{
    if ( p_SSL_get_wbio )
        return(p_SSL_get_wbio(s));
    else
        return(NULL);
}

size_t
ck_SSL_get_finished(SSL * s,void *buf, size_t count)
{
    if ( p_SSL_get_finished )
        return(p_SSL_get_finished(s,buf,count));
    else
        return(0);
}

size_t
ck_SSL_get_peer_finished(SSL * s,void *buf, size_t count)
{
    if ( p_SSL_get_peer_finished )
        return(p_SSL_get_peer_finished(s,buf,count));
    else
        return(0);
}

void
ck_SSL_copy_session_id(SSL *to,SSL *from)
{
    if ( p_SSL_copy_session_id )
        p_SSL_copy_session_id(to,from);
}

int
ck_SSL_renegotiate(SSL * ssl)
{
    if ( p_SSL_renegotiate )
        return(p_SSL_renegotiate(ssl));
    else
        return(-1);
}

int
ck_SSL_set_session(SSL *ssl, SSL_SESSION * session)
{
    if ( p_SSL_set_session )
        return(p_SSL_set_session(ssl,session));
    else
        return(-1);
}
SSL_SESSION *
ck_SSL_get_session(SSL * ssl)
{
    if ( p_SSL_get_session )
        return(p_SSL_get_session(ssl));
    else
        return(NULL);
}

int
ck_SSL_CTX_set_session_id_context(SSL_CTX *ctx,const unsigned char *sid_ctx,
                                   unsigned int sid_ctx_len)
{
    if ( p_SSL_CTX_set_session_id_context )
        return(p_SSL_CTX_set_session_id_context(ctx,sid_ctx,sid_ctx_len));
    else
        return(-1);
}

EVP_PKEY *
ck_SSL_get_privatekey(SSL * s)
{
    if ( p_SSL_get_privatekey )
        return(p_SSL_get_privatekey(s));
    else
        return(NULL);
}
X509 *
ck_SSL_get_certificate(SSL * s)
{
    if ( p_SSL_get_certificate )
        return(p_SSL_get_certificate(s));
    else
        return(NULL);
}

int
ck_SSL_clear(SSL * s)
{
    if ( p_SSL_clear )
        return(p_SSL_clear(s));
    else
        return(-1);
}

void
ck_SSL_set_accept_state(SSL * s)
{
    if ( p_SSL_set_accept_state )
        p_SSL_set_accept_state(s);
}

char *
ck_SSL_CIPHER_description(SSL_CIPHER * c,char *buf,int size)
{
    if (p_SSL_CIPHER_description)
        return(p_SSL_CIPHER_description(c,buf,size));
    else
        return(NULL);
}

#ifdef SSL_KRB5
KSSL_CTX *
ck_kssl_ctx_new(void)
{
    if ( p_kssl_ctx_new )
        return(p_kssl_ctx_new());
    else
        return(NULL);
}

KSSL_CTX *
ck_kssl_ctx_free(KSSL_CTX *kssl_ctx)
{
    if ( p_kssl_ctx_free )
        return(p_kssl_ctx_free(kssl_ctx));
    else
        return(NULL);
}

krb5_error_code
ck_kssl_ctx_setstring(KSSL_CTX *kssl_ctx, int which, char *text)
{
    if ( p_kssl_ctx_setstring )
        return(p_kssl_ctx_setstring(kssl_ctx,which,text));
    else
        return(-1);
}

krb5_error_code
ck_kssl_ctx_setprinc(KSSL_CTX *kssl_ctx, int which,
                      krb5_data *realm, krb5_data *entity)
{
    if ( p_kssl_ctx_setprinc )
        return(p_kssl_ctx_setprinc(kssl_ctx, which, realm, entity));
    else
        return(-1);
}
#endif /* SSL_KRB5 */
int
ck_SSL_COMP_add_compression_method(int id,COMP_METHOD *cm)
{
    if ( p_SSL_COMP_add_compression_method )
        return(p_SSL_COMP_add_compression_method(id,cm));
    else
        return(-1);
}

void 
ck_SSL_CTX_set_info_callback(SSL_CTX *ctx, void (*cb)(const SSL *ssl,int type,int val))
{
    if ( p_SSL_CTX_set_info_callback )
        p_SSL_CTX_set_info_callback(ctx,cb);
}


int
ck_ssl_loaddll_eh(void)
{
    if ( hSSL ) {
#ifdef NT
        FreeLibrary(hSSL);
        hSSL = NULL;
#else /* NT */
        DosFreeModule(hSSL);
        hSSL = 0;
#endif  /* NT */
    }

    if ( hX5092UID ) {
#ifdef NT
        FreeLibrary(hX5092UID);
        hX5092UID = NULL;
#else /* NT */
        DosFreeModule(hX5092UID);
        hX5092UID = 0;
#endif  /* NT */
    }
    p_SSL_get_error=NULL;
    p_SSL_read=NULL;
    p_SSL_peek=NULL;
    p_SSL_connect=NULL;
    p_SSL_set_fd=NULL;
    p_SSL_free=NULL;
    p_SSL_shutdown=NULL;
    p_SSL_write=NULL;
    p_SSL_pending=NULL;
    p_SSL_load_error_strings=NULL;
    p_SSL_get_peer_certificate=NULL;
    p_SSL_CIPHER_get_name=NULL;
    p_SSL_CIPHER_get_bits=NULL;
    p_SSL_CIPHER_get_version=NULL;
    p_SSL_get_current_cipher=NULL;
    p_SSL_get_shared_ciphers=NULL;
    p_SSL_get_ciphers=NULL;
    p_SSL_get_cipher_list=NULL;
    p_SSL_CTX_set_default_verify_paths=NULL;
    p_SSL_use_RSAPrivateKey_file=NULL;
    p_SSL_use_DSAPrivateKey_file=NULL;
    p_SSL_use_PrivateKey_file=NULL;
    p_SSL_use_certificate_file=NULL;
    p_SSL_check_private_key=NULL;
    p_SSL_CTX_use_PrivateKey_file=NULL;
    p_SSL_CTX_use_certificate_file=NULL;
    p_SSL_CTX_use_certificate_chain_file=NULL;
    p_SSL_CTX_check_private_key=NULL;
    p_SSL_set_verify=NULL;
    p_SSL_CTX_set_verify=NULL;
    p_SSL_new=NULL;
    p_SSL_CTX_ctrl=NULL;
    p_SSL_CTX_new=NULL;
    p_SSL_CTX_free=NULL;
    p_SSL_CTX_set_default_passwd_cb=NULL;
    p_SSLv23_method=NULL;
    p_SSLv3_method=NULL;
    p_TLSv1_method=NULL;
    p_SSLv23_server_method=NULL;
    p_SSLv3_server_method=NULL;
    p_TLSv1_server_method=NULL;
    p_SSLv23_client_method=NULL;
    p_SSLv3_client_method=NULL;
    p_TLSv1_client_method=NULL;
    p_SSL_library_init=NULL;
    p_SSL_state_string=NULL;
    p_SSL_state_string_long=NULL;
    p_SSL_accept=NULL;
    p_SSL_set_cipher_list=NULL;

    p_SSL_CTX_set_tmp_rsa_callback=NULL;
    p_SSL_set_tmp_rsa_callback=NULL;
    p_SSL_CTX_set_tmp_dh_callback=NULL;
    p_SSL_set_tmp_dh_callback=NULL;

    p_SSL_CTX_load_verify_locations=NULL;
    p_SSL_CTX_set_default_passwd_cb_userdata=NULL;
    p_X509_to_user=NULL;
    p_X509_userok=NULL;
    p_SSL_get_wbio=NULL;
    p_SSL_get_finished=NULL;
    p_SSL_get_peer_finished=NULL;
    p_SSL_copy_session_id=NULL;
    p_SSL_renegotiate=NULL;
    p_SSL_get_session=NULL;
    p_SSL_set_session=NULL;
    p_SSL_CTX_set_session_id_context=NULL;
    p_SSL_get_privatekey=NULL;
    p_SSL_get_certificate=NULL;
    p_SSL_clear=NULL;
    p_SSL_set_accept_state=NULL;
    p_SSL_CIPHER_description=NULL;

#ifdef SSL_KRB5
    p_kssl_ctx_new = NULL;
    p_kssl_ctx_free = NULL;
    p_kssl_ctx_setstring = NULL;
    p_kssl_ctx_setprinc = NULL;
#endif /* SSL_KRB5 */
    p_SSL_COMP_add_compression_method=NULL;
    p_SSL_CTX_set_info_callback = NULL;
    return(1);
}

int
ck_ssl_loaddll( void )
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

    if ( ssl_dll_loaded )
        return(1);

    if ( startflags & 8 )       /* do not load if we are not supposed to */
        return(0);

#ifdef NT
    hSSL = LoadLibrary("SSLEAY32");
    if ( !hSSL ) {
        rc = GetLastError() ;
        debug(F111, "OpenSSL LoadLibrary failed","SSLEAY32",rc) ;
        return(0);
    } else {
        if (((FARPROC) p_SSL_get_error =
              GetProcAddress( hSSL, "SSL_get_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_error",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_read =
              GetProcAddress( hSSL, "SSL_read" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_read",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_peek =
              GetProcAddress( hSSL, "SSL_peek" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_peek",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_connect =
              GetProcAddress( hSSL, "SSL_connect" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_connect",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_set_fd =
              GetProcAddress( hSSL, "SSL_set_fd" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_fd",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_free =
              GetProcAddress( hSSL, "SSL_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_shutdown =
              GetProcAddress( hSSL, "SSL_shutdown" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_shutdown",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_write =
              GetProcAddress( hSSL, "SSL_write" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_write",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_pending =
              GetProcAddress( hSSL, "SSL_pending" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_pending",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_load_error_strings =
              GetProcAddress( hSSL, "SSL_load_error_strings" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_load_error_strings",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_peer_certificate =
              GetProcAddress( hSSL, "SSL_get_peer_certificate" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_peer_certificate",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CIPHER_get_name =
              GetProcAddress( hSSL, "SSL_CIPHER_get_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_name",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CIPHER_get_bits =
              GetProcAddress( hSSL, "SSL_CIPHER_get_bits" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_bits",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CIPHER_get_version =
              GetProcAddress( hSSL, "SSL_CIPHER_get_version" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_version",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_current_cipher =
              GetProcAddress( hSSL, "SSL_get_current_cipher" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_current_cipher",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_shared_ciphers =
              GetProcAddress( hSSL, "SSL_get_shared_ciphers" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_shared_ciphers",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_ciphers =
              GetProcAddress( hSSL, "SSL_get_ciphers" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_ciphers",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_cipher_list =
              GetProcAddress( hSSL, "SSL_get_cipher_list" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_cipher_list",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_set_default_verify_paths =
              GetProcAddress( hSSL, "SSL_CTX_set_default_verify_paths" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_default_verify_paths",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_use_RSAPrivateKey_file =
              GetProcAddress( hSSL, "SSL_use_RSAPrivateKey_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_use_RSAPrivateKey_file",rc);
        }
        if (((FARPROC) p_SSL_use_PrivateKey_file =
              GetProcAddress( hSSL, "SSL_use_PrivateKey_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_use_PrivateKey_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_use_certificate_file =
              GetProcAddress( hSSL, "SSL_use_certificate_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_use_certificate_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_use_PrivateKey_file =
              GetProcAddress( hSSL, "SSL_CTX_use_PrivateKey_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_use_PrivateKey_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_use_certificate_file =
              GetProcAddress( hSSL, "SSL_CTX_use_certificate_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_use_certificate_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_use_certificate_chain_file =
              GetProcAddress( hSSL, "SSL_CTX_use_certificate_chain_file" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_use_certificate_chain_file",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_check_private_key =
              GetProcAddress( hSSL, "SSL_CTX_check_private_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_check_private_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_check_private_key =
              GetProcAddress( hSSL, "SSL_check_private_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_check_private_key",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_set_verify =
              GetProcAddress( hSSL, "SSL_set_verify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_verify",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_set_verify =
              GetProcAddress( hSSL, "SSL_CTX_set_verify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_set_verify",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_new =
              GetProcAddress( hSSL, "SSL_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_ctrl =
              GetProcAddress( hSSL, "SSL_CTX_ctrl" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_ctrl",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_new =
              GetProcAddress( hSSL, "SSL_CTX_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_new",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_free =
              GetProcAddress( hSSL, "SSL_CTX_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_free",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_set_default_passwd_cb =
              GetProcAddress( hSSL, "SSL_CTX_set_default_passwd_cb" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_set_default_passwd_cb",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSLv23_method =
              GetProcAddress( hSSL, "SSLv23_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_method",rc);
        }
        if (((FARPROC) p_SSLv3_method =
              GetProcAddress( hSSL, "SSLv3_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_method",rc);
        }
        if (((FARPROC) p_TLSv1_method =
              GetProcAddress( hSSL, "TLSv1_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_method",rc);
            load_error = 1;
        }

        if (((FARPROC) p_SSLv23_client_method =
              GetProcAddress( hSSL, "SSLv23_client_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_client_method",rc);
        }
        if (((FARPROC) p_SSLv3_client_method =
              GetProcAddress( hSSL, "SSLv3_client_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_client_method",rc);
        }
        if (((FARPROC) p_TLSv1_client_method =
              GetProcAddress( hSSL, "TLSv1_client_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_client_method",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSLv23_server_method =
              GetProcAddress( hSSL, "SSLv23_server_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_server_method",rc);
        }
        if (((FARPROC) p_SSLv3_server_method =
              GetProcAddress( hSSL, "SSLv3_server_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_server_method",rc);
        }
        if (((FARPROC) p_TLSv1_server_method =
              GetProcAddress( hSSL, "TLSv1_server_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_server_method",rc);
            load_error = 1;
        }

        if (((FARPROC) p_SSL_library_init =
              GetProcAddress( hSSL, "SSL_library_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_library_init",rc);
            if (((FARPROC) p_SSL_library_init =
              GetProcAddress( hSSL, "OpenSSL_add_ssl_algorithms" )) == NULL )
            {
                rc = GetLastError() ;
                debug(F111,"OpenSSL SSL GetProcAddress failed",
                       "OpenSSL_add_ssl_algorithms",rc);
                load_error = 1;
            }
        }
        if (((FARPROC) p_SSL_state_string =
              GetProcAddress( hSSL, "SSL_state_string" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_state_string",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_state_string_long =
              GetProcAddress( hSSL, "SSL_state_string_long" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_state_string_long",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_accept =
              GetProcAddress( hSSL, "SSL_accept" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_accept",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_set_cipher_list =
              GetProcAddress( hSSL, "SSL_set_cipher_list" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_cipher_list",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_set_tmp_rsa_callback =
              GetProcAddress( hSSL, "SSL_CTX_set_tmp_rsa_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_tmp_rsa_callback",rc);
        }
        if (((FARPROC) p_SSL_set_tmp_rsa_callback =
              GetProcAddress( hSSL, "SSL_set_tmp_rsa_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_tmp_rsa_callback",rc);
        }
        if (((FARPROC) p_SSL_CTX_set_tmp_dh_callback =
              GetProcAddress( hSSL, "SSL_CTX_set_tmp_dh_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_tmp_dh_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_set_tmp_dh_callback =
              GetProcAddress( hSSL, "SSL_set_tmp_dh_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_tmp_dh_callback",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_load_verify_locations =
              GetProcAddress( hSSL, "SSL_CTX_load_verify_locations" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_load_verify_locations",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_CTX_set_default_passwd_cb_userdata =
              GetProcAddress( hSSL,
                    "SSL_CTX_set_default_passwd_cb_userdata" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_default_passwd_cb_userdata",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_wbio =
              GetProcAddress( hSSL, "SSL_get_wbio" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_wbio",rc);
            load_error = 1;
        }
        if (((FARPROC) p_SSL_get_finished =
              GetProcAddress( hSSL, "SSL_get_finished" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_finished",rc);
        }
        if (((FARPROC) p_SSL_get_peer_finished =
              GetProcAddress( hSSL, "SSL_get_peer_finished" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_peer_finished",rc);
        }
        if (((FARPROC) p_SSL_copy_session_id =
              GetProcAddress( hSSL, "SSL_copy_session_id" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_copy_session_id",rc);
        }
        if (((FARPROC) p_SSL_renegotiate =
              GetProcAddress( hSSL, "SSL_renegotiate" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_renegotiate",rc);
        }
        if (((FARPROC) p_SSL_get_session =
              GetProcAddress( hSSL, "SSL_get_session" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_session",rc);
        }
        if (((FARPROC) p_SSL_set_session =
              GetProcAddress( hSSL, "SSL_set_session" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_session",rc);
        }
        if (((FARPROC) p_SSL_CTX_set_session_id_context =
              GetProcAddress( hSSL, "SSL_CTX_set_session_id_context" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                  "SSL_CTX_set_session_id_context",rc);
        }
        if (((FARPROC) p_SSL_get_privatekey =
              GetProcAddress( hSSL, "SSL_get_priavatekey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_privatekey",rc);
        }
        if (((FARPROC) p_SSL_get_certificate =
              GetProcAddress( hSSL, "SSL_get_certificate" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_certificate",rc);
        }
        if (((FARPROC) p_SSL_clear =
              GetProcAddress( hSSL, "SSL_clear" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_clear",rc);
        }
        if (((FARPROC) p_SSL_set_accept_state =
              GetProcAddress( hSSL, "SSL_set_accept_state" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_accept_state",rc);
        }
        if (((FARPROC) p_SSL_CIPHER_description =
              GetProcAddress( hSSL, "SSL_CIPHER_description" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CIPHER_description",rc);
        }
        if (((FARPROC) p_SSL_CTX_set_info_callback =
              GetProcAddress( hSSL, "SSL_CTX_set_info_callback" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_info_callback",rc);
        }

#ifdef SSL_KRB5
        if (((FARPROC) p_kssl_ctx_new =
              GetProcAddress( hSSL, "kssl_ctx_new" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "kssl_ctx_new",rc);
        }
        if (((FARPROC) p_kssl_ctx_free =
              GetProcAddress( hSSL, "kssl_ctx_free" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "kssl_ctx_free",rc);
        }
        if (((FARPROC) p_kssl_ctx_setstring =
              GetProcAddress( hSSL, "kssl_ctx_setstring" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "kssl_ctx_setstring",rc);
        }
        if (((FARPROC) p_kssl_ctx_setprinc =
              GetProcAddress( hSSL, "kssl_ctx_setprinc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "kssl_ctx_setprinc",rc);
        }
#endif /* SSL_KRB5 */
        if (((FARPROC) p_SSL_COMP_add_compression_method =
              GetProcAddress( hSSL, "SSL_COMP_add_compression_method" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_COMP_add_compression_method",rc);
        }

        if ( load_error ) {
            ck_ssl_loaddll_eh();
            return 0;
        }
    }
#else /* NT */
    exe_path = GetLoadPath();
    sprintf(path, "%.*s%s.DLL", (int)get_dir_len(exe_path), exe_path,dllname);
    rc = DosLoadModule(errbuf, 256, path, &hSSL);
    if (rc) {
        debug(F111, "OpenSSL LoadLibrary failed",path,rc) ;
        rc = DosLoadModule(errbuf, 256, dllname, &hSSL);
    }
    if ( rc ) {
        debug(F111, "OpenSSL LoadLibrary failed",errbuf,rc) ;
        return(0);
    } else {
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_error",
                                  (PFN*) &p_SSL_get_error))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                  "SSL_get_error",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_read",
                                  (PFN*) &p_SSL_read))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_read",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_peek",
                                  (PFN*) &p_SSL_peek))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_peek",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_connect",
                                  (PFN*) &p_SSL_connect))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_connect",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_set_fd",
                                  (PFN*) &p_SSL_set_fd))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_fd",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_free",
                                  (PFN*) &p_SSL_free))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_shutdown",
                                  (PFN*) &p_SSL_shutdown))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_shutdown",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_write",
                                  (PFN*) &p_SSL_write))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_write",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_pending",
                                  (PFN*) &p_SSL_pending))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_pending",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_load_error_strings",
                                  (PFN*) &p_SSL_load_error_strings ))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_load_error_strings",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_peer_certificate",
                                  (PFN*) &p_SSL_get_peer_certificate))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_peer_certificate",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CIPHER_get_name",
                                  (PFN*) &p_SSL_CIPHER_get_name))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_name",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CIPHER_get_bits",
                                  (PFN*) &p_SSL_CIPHER_get_bits))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_bits",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CIPHER_get_version",
                                  (PFN*) &p_SSL_CIPHER_get_version))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CIPHER_get_version",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_current_cipher",
                                  (PFN*) &p_SSL_get_current_cipher))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_current_cipher",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_shared_ciphers",
                                  (PFN*) &p_SSL_get_shared_ciphers))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_shared_ciphers",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_ciphers",
                                  (PFN*) &p_SSL_get_ciphers))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_get_ciphers",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_get_cipher_list",
                                  (PFN*) &p_SSL_get_cipher_list))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_cipher_list",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_set_default_verify_paths",
                                  (PFN*) &p_SSL_CTX_set_default_verify_paths))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_default_verify_paths",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_use_RSAPrivateKey_file",
                                  (PFN*) &p_SSL_use_RSAPrivateKey_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                  "SSL_use_RSAPrivateKey_file",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_use_PrivateKey_file",
                                  (PFN*) &p_SSL_use_PrivateKey_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_use_PrivateKey_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_use_certificate_file",
                                  (PFN*) &p_SSL_use_certificate_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_use_certificate_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_use_PrivateKey_file",
                                  (PFN*) &p_SSL_CTX_use_PrivateKey_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_use_PrivateKey_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_use_certificate_file",
                                  (PFN*) &p_SSL_CTX_use_certificate_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_use_certificate_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_use_certificate_chain_file",
                                  (PFN*) &p_SSL_CTX_use_certificate_chain_file))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_use_certificate_chain_file",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_check_private_key",
                                  (PFN*) &p_SSL_CTX_check_private_key))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_check_private_key",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_check_private_key",
                                  (PFN*) &p_SSL_check_private_key))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_check_private_key",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_set_verify",
                                  (PFN*) &p_SSL_set_verify))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_verify",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_set_verify",
                                  (PFN*) &p_SSL_CTX_set_verify))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_set_verify",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_new",
                                  (PFN*) &p_SSL_new))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_new",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_ctrl",
                                  (PFN*) &p_SSL_CTX_ctrl))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_ctrl",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_new",
                                  (PFN*) &p_SSL_CTX_new))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_new",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_free",
                                  (PFN*) &p_SSL_CTX_free))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_set_default_passwd_cb",
                                  (PFN*) &p_SSL_CTX_set_default_passwd_cb))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_CTX_set_default_passwd_cb",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv23_method",
                                  (PFN*) &p_SSLv23_method))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv3_method",
                                  (PFN*) &p_SSLv3_method))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "TLSv1_method",
                                  (PFN*) &p_TLSv1_method))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_method",rc);
            load_error = 1;
        }

        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv23_client_method",
                                  (PFN*) &p_SSLv23_client_method))
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_client_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv3_client_method",
                                  (PFN*) &p_SSLv3_client_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_client_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "TLSv1_client_method",
                                  (PFN*) &p_TLSv1_client_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_client_method",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv23_server_method",
                                  (PFN*) &p_SSLv23_server_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv23_server_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSLv3_server_method",
                                  (PFN*) &p_SSLv3_server_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSLv3_server_method",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "TLSv1_server_method",
                                  (PFN*) &p_TLSv1_server_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","TLSv1_server_method",rc);
            load_error = 1;
        }

        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_library_init",
                                  (PFN*) &p_SSL_library_init) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_library_init",rc);

            if (rc = DosQueryProcAddr( hSSL, 0, "OpenSSL_add_ssl_algorithms",
                                      (PFN*) &p_SSL_library_init) )
            {
                debug(F111,"OpenSSL SSL GetProcAddress failed",
                       "OpenSSL_add_ssl_algorithms",rc);
                load_error = 1;
            }
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_state_string",
                                  (PFN*) &p_SSL_state_string) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_state_string",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_state_string_long",
                                  (PFN*) &p_SSL_state_string_long) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_state_string_long",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_accept",
                                  (PFN*) &p_SSL_accept) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_accept",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_set_cipher_list",
                                  (PFN*) &p_SSL_set_cipher_list) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed","SSL_set_cipher_list",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_set_tmp_rsa_callback",
                                  (PFN*) &p_SSL_CTX_set_tmp_rsa_callback) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_tmp_rsa_callback",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_set_tmp_rsa_callback",
                                  (PFN*) &p_SSL_set_tmp_rsa_callback) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_tmp_rsa_callback",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_set_tmp_dh_callback",
                                  (PFN*) &p_SSL_CTX_set_tmp_dh_callback) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_tmp_dh_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_set_tmp_dh_callback",
                                  (PFN*) &p_SSL_set_tmp_dh_callback) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_tmp_dh_callback",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0, "SSL_CTX_load_verify_locations",
                                  (PFN*) &p_SSL_CTX_load_verify_locations) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_load_verify_locations",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_CTX_set_default_passwd_cb_userdata",
                                  (PFN*) &p_SSL_CTX_set_default_passwd_cb_userdata) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_default_passwd_cb_userdata",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_wbio",
                                  (PFN*) &p_SSL_get_wbio) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_wbio",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_finished",
                                  (PFN*) &p_SSL_get_finished) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_finished",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_peer_finished",
                                  (PFN*) &p_SSL_get_peer_finished) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_peer_finished",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_copy_session_id",
                                  (PFN*) &p_SSL_copy_session_id) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_copy_session_id",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_renegotiate",
                                  (PFN*) &p_SSL_renegotiate) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_renegotiate",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_session",
                                  (PFN*) &p_SSL_get_session) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_session",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_set_session",
                                  (PFN*) &p_SSL_set_session) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_session",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_CTX_set_session_id_context",
                                  (PFN*) &p_SSL_CTX_set_session_id_context) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CTX_set_session_id_context",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_privatekey",
                                  (PFN*) &p_SSL_get_privatekey) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_privatekey",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_get_certificate",
                                  (PFN*) &p_SSL_get_certificate) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_get_certificate",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_clear",
                                  (PFN*) &p_SSL_clear) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_clear",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_set_accept_state",
                                  (PFN*) &p_SSL_set_accept_state) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_set_accept_state",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_CIPHER_description",
                                  (PFN*) &p_SSL_CIPHER_description) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_CIPHER_description",rc);
        }
        if (rc = DosQueryProcAddr( hSSL, 0,
                                  "SSL_COMP_add_compression_method",
                                  (PFN*) &p_SSL_COMP_add_compression_method) )
        {
            debug(F111,"OpenSSL SSL GetProcAddress failed",
                   "SSL_COMP_add_compression_method",rc);
        }

        if ( load_error ) {
            ck_ssl_loaddll_eh();
            return 0;
        }
    }
#endif /* NT */

    ssl_dll_loaded = 1;
    if ( deblog ) {
        printf("OpenSSL DLLs Loaded\n");
        debug(F100,"OpenSSL DLLs Loaded","",0);
    }

    if ( p_SSL_get_finished &&
         p_SSL_get_peer_finished) {
        ssl_finished_messages = 1;
        if ( deblog ) {
            printf("OpenSSL Finished Messages available\n");
            debug(F100,"OpenSSL Finished Messages available","",0);
        }
    }

    /* Attempt to load a X509_to_user() function */
#ifdef NT
    hX5092UID = LoadLibrary("X5092UID");
    if ( hX5092UID ) {
        if (((FARPROC) p_X509_to_user =
              GetProcAddress( hX5092UID, "X509_to_user" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"X5092UID GetProcAddress failed","X509_to_user",rc);
        }
        if (((FARPROC) p_X509_userok =
              GetProcAddress( hX5092UID, "X509_userok" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"X5092UID GetProcAddress failed","X509_userok",rc);
        }
    }
#else /* NT */
    dllname = "X5092UID";
    sprintf(path, "%.*s%s.DLL", (int)get_dir_len(exe_path), exe_path,dllname);
    rc = DosLoadModule(errbuf, 256, path, &hX5092UID);
    if (rc) {
        debug(F111, "X5092UID LoadLibrary failed",path,rc) ;
        rc = DosLoadModule(errbuf, 256, dllname, &hX5092UID);
    }
    if ( !rc ) {
        if (rc = DosQueryProcAddr( hX5092UID, 0, "X509_to_user",
                                  (PFN*) &p_X509_to_user))
        {
            debug(F111,"X5092UID GetProcAddress failed",
                  "X509_to_user",rc);
        }
        if (rc = DosQueryProcAddr( hX5092UID, 0, "X509_userok",
                                  (PFN*) &p_X509_userok))
        {
            debug(F111,"X5092UID GetProcAddress failed",
                  "X509_userok",rc);
        }
    }
#endif /* NT */
    return(1);
}

int
ck_ssl_unloaddll( void )
{
    if ( !ssl_dll_loaded )
        return(1);

    /* unload dlls */
    ck_ssl_loaddll_eh();

    /* success */
    ssl_dll_loaded = 0;
    ssl_finished_messages = 0;
    return(1);
}
#endif /* SSLDLL */
#endif /* CK_SSL */
#endif /* OS2 */
