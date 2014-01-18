/*
  C K O A T H . C  --  Authentication for Kermit 95

  Copyright (C) 1998, 2004, Trustees of Columbia University in the City of New
  York.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City
*/


#include "ckcdeb.h"

#ifdef CK_AUTHENTICATION
#define CKOATH_C
#include "ckcker.h"
#include "ckuusr.h"
#include "ckucmd.h"                             /* For struct keytab */
#include "ckcnet.h"
#include "ckctel.h"

#ifdef CK_DES
#ifdef CK_SSL
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */
#endif /* CK_SSL */
#endif /* CK_DES */

#ifdef CRYPT_DLL
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */
#ifdef OS2
#ifdef NT
#include <windows.h>
#define SECURITY_WIN32
#include <security.h>
#ifndef NTLMSP_NAME_A
#define NTLMSP_NAME_A "NTLM"
#endif /* NTLMSP_NAME_A */
#else /* NT */
#define INCL_DOSMODULEMGR
#include <os2.h>
#endif /* NT */
#endif /* OS2 */
#endif /* CRYPT_DLL */

#include "ckosyn.h"

#ifdef NT
#define KRB5_AUTOCONF__
#define NTLM
#endif /* NT */

#ifndef CK_KERBEROS
#ifdef KRB4
#undef KRB4
#endif /* KRB4 */
#ifdef KRB5
#undef KRB5
#endif /* KRB5 */
#ifdef KRB524
#undef KRB524
#endif /* KRB524 */
#endif /* CK_KERBEROS */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>

#ifdef KRB5
#include "krb5.h"
#include "profile.h"
#include "com_err.h"

#ifdef KRB5_GET_INIT_CREDS_OPT_TKT_LIFE
#define KRB5_HAVE_GET_INIT_CREDS
#else
#define krb5_free_unparsed_name(con,val) krb5_xfree((char *)(val))
#endif

#ifndef KRB5_HAVE_GET_INIT_CREDS
#define krb5_free_data_contents(c,v) krb5_xfree((char *)(v)->data)
#endif
#endif /* KRB5 */

#ifdef KRB4
#define  des_cblock Block
#define  des_key_schedule Schedule
#ifdef NT
#define _WINDOWS
#include "kerberosIV/krb.h"
#else /* NT */
#ifdef KRB524
#include "kerberosIV/krb.h"
_PROTOTYP(const char * krb_get_err_text_entry, (int));
#else /* KRB524 */
#include "krb.h"
#define krb_get_err_text_entry krb_get_err_text
#endif /* KRB524 */
#endif /* NT */
#else /* KRB4 */
#ifdef CK_SSL
#define  des_cblock Block
#define  des_key_schedule Schedule
#endif /* CK_SSL */
#endif /* KRB4 */

#include "ckuath.h"
#ifdef CK_KERBEROS
#ifndef KRB5
#define NOBLOCKDEF
#endif /* KRB5 */
#ifdef KRB524
#define NOBLOCKDEF
#endif /* KRB524 */
#endif /* CK_KERBEROS */
#include "ckuat2.h"

#ifdef CK_SSL
#ifdef LIBDES
#ifndef HEADER_DES_H
#define HEADER_DES_H
#endif /* HEADER_DES_H */
#endif /* LIBDES */
#include "ck_ssl.h"
#endif /* SSL */

#ifdef CK_SRP
#include <t_pwd.h>
#include <t_client.h>
#include <t_server.h>
#endif /* CK_SRP */

#ifdef OS2
#ifdef CK_ENCRYPTION
#define MAP_DES
#endif /* CK_ENCRYPTION */
#ifdef KRB4
#define MAP_KRB4
#endif /* KRB4 */
#ifdef SRPDLL
#define MAP_SRP
#endif /* SRPDLL */
#ifdef KRB5
#define MAP_KRB5
#endif /* KRB5 */
#ifdef CRYPT_DLL
#define MAP_CRYPT
#endif /* CRYPT_DLL */
#define MAP_NTLM
#include "ckoath.h"

extern char tn_msg[], hexbuf[];         /* from ckcnet.c */
extern int deblog, debses, tn_deb;

extern int authentication_version;
extern int auth_type_user[AUTHTYPLSTSZ];
extern int auth_how;
extern int auth_crypt;
extern int auth_fwd;
extern int accept_complete;

#define AUTHTMPBL 2048
static char strTmp[AUTHTMPBL+1];
extern char szUserNameRequested[UIDBUFLEN+1];    /* for incoming connections */
extern char szUserNameAuthenticated[UIDBUFLEN+1];/* for incoming connections */
extern char szHostName[UIDBUFLEN+1];

#ifdef CK_ENCRYPTION
extern int encrypt_flag;
#endif

#ifdef KRB5
extern krb5_context k5_context;
extern char * krb5_d_principal;         /* Default principal */
extern char * krb5_d_instance;          /* Default instance */
extern char * krb5_d_realm;             /* Default realm */
extern char * krb5_d_cc;                /* Default credentials cache */
extern char * krb5_d_srv;               /* Default service name */
extern int    krb5_d_lifetime;          /* Default lifetime */
extern int    krb5_d_forwardable;
extern int    krb5_d_proxiable;
extern int    krb5_d_renewable;
extern int    krb5_autoget;
extern int    krb5_checkaddrs;
extern int    krb5_d_getk4;
extern int    krb5_d_no_addresses;
extern char * krb5_d_addrs[];

extern int    krb5_errno;
extern char * krb5_errmsg;
#endif /* KRB5 */

#ifdef KRB4
extern char * krb4_d_principal;         /* Default principal */
extern char * krb4_d_realm;             /* Default realm */
extern char * krb4_d_srv;               /* Default service name */
extern int    krb4_d_lifetime;          /* Default lifetime */
extern int    krb4_d_preauth;
extern char * krb4_d_instance;
extern int    krb4_autoget;
extern int    krb4_checkaddrs;
extern char * k4_keytab;

extern int    krb4_errno;
extern char * krb4_errmsg;
#endif /* KRB4 */

#ifdef CK_KERBEROS
#define CHECKADDRS
#endif /* CK_KERBEROS */

#ifdef OS2ONLY
#ifdef KRB5_CALLCONV
#undef KRB5_CALLCONV
#define KRB5_CALLCONV  __cdecl
#endif
#ifdef KRB5_CALLCONV_C
#undef KRB5_CALLCONV_C
#endif
#define KRB5_CALLCONV_C __cdecl
#endif

#ifndef KRB5_CALLCONV
#define KRB5_CALLCONV __stdcall
#endif /* KRB5_CALLCONV */
#ifndef KRB5_CALLCONV_C
#define KRB5_CALLCONV_C __cdecl
#endif /* KRB5_CALLCONV_C */

#ifdef OS2ONLY
static char fail[_MAX_PATH];
#endif /* OS2ONLY */

int copy_for_net(unsigned char *to, unsigned char *from, int c);

#ifdef SRPDLL
/* These functions are located in DLLs which we are not linking to at */
/* compile time.  Therefore, we have to redirect them to locally      */
/* defined functions which will call the real thing only if they are  */
/* available via Run-Time Linking.                                    */
/*
    From SRP.DLL:
       unresolved external symbol _t_clientresponse
       unresolved external symbol _t_clientgetkey
       unresolved external symbol _t_clientpasswd
       unresolved external symbol _t_clientgenexp
       unresolved external symbol _t_clientopen
       unresolved external symbol _t_clientverify
*/

static unsigned char * (*p_t_clientresponse)(struct t_client *)=NULL;
static unsigned char *
 (*p_t_clientgetkey)(struct t_client *, struct t_num *)=NULL;
static void (*p_t_clientpasswd)(struct t_client *, char *)=NULL;
static struct t_num * (*p_t_clientgenexp)(struct t_client *)=NULL;
static struct t_client * (*p_t_clientopen)
    (const char *, struct t_num *, struct t_num *, struct t_num *)=NULL;
static int (*p_t_clientverify)(struct t_client *, unsigned char *)=NULL;

unsigned char *
ck_t_clientresponse(struct t_client * client) {
    if ( p_t_clientresponse )
        return(p_t_clientresponse(client));
    else
        return(NULL);
}

unsigned char *
ck_t_clientgetkey(struct t_client * client, struct t_num * num) {
    if ( p_t_clientgetkey )
        return(p_t_clientgetkey(client,num));
    return(NULL);
}

void ck_t_clientpasswd(struct t_client * client, char * pwd) {
    if ( p_t_clientpasswd )
        p_t_clientpasswd(client,pwd);
}

struct t_num *
ck_t_clientgenexp(struct t_client * client) {
    if ( p_t_clientgenexp )
        return(p_t_clientgenexp(client));
    else
        return(NULL);
}

struct t_client *
ck_t_clientopen(const char * str, struct t_num * A,
                 struct t_num * B, struct t_num * C) {
    if ( p_t_clientopen )
        return(p_t_clientopen(str,A,B,C));
    else
        return(NULL);
}

int
ck_t_clientverify(struct t_client * client, unsigned char * str) {
    if ( p_t_clientverify )
        return(p_t_clientverify(client,str));
    else
        return(0);
}
#endif /* SRPDLL */

#ifdef KRB4
/* These functions are located in DLLs which we are not linking to at */
/* compile time.  Therefore, we have to redirect them to locally      */
/* defined functions which will call the real thing only if they are  */
/* available via Run-Time Linking.                                    */
/*
    From KRB4_32.DLL:
       unresolved external symbol __imp__krb_get_err_text@4
       unresolved external symbol __imp__krb_get_cred@16
       unresolved external symbol __imp__krb_mk_req@20
       unresolved external symbol __imp__krb_realmofhost@4
       unresolved external symbol __imp__krb_get_phost@4

       unresolved external symbol _des_ecb_encrypt
       unresolved external symbol __imp__des_key_sched@8
       unresolved external symbol _des_new_random_key
       unresolved external symbol _des_set_random_generator_seed

       Note: kerberosiv/krb.h contains a reference to errno which
       must be renamed to compile under Windows NT with errno.h

       unresolved external symbol __imp__krb_get_pw_in_tkt@28
       unresolved external symbol __imp__krb_get_pw_in_tkt_preauth@28
       unresolved external symbol __imp__krb_get_lrealm@8
       unresolved external symbol __imp__krb_get_err_text@4
       unresolved external symbol __imp__kname_parse@16
       unresolved external symbol __imp__dest_tkt

       unresolved external symbol __imp__krb_get_tf_realm@8

       Leash specific (not defined in Kerberos V/KerberosIV
       include files
         unresolved external symbol _tf_get_cred
         unresolved external symbol _tf_get_pinst
         unresolved external symbol _tf_get_pname
         unresolved external symbol _tf_close
         unresolved external symbol _tf_init
         unresolved external symbol _tkt_string

         C:\kerberos\leash\KRB4\INCLUDE\KRB.H
         int tf_get_pname(char*);
         int tf_get_pinst(char*);
         int tf_get_cred(CREDENTIALS*);
         void tf_close(void);
         int tf_init(char *,int);
         char * tkt_string(void);
         int krb_check_serv(char *);

      Added when we put in server side support:
        unresolved external symbol __imp__kuserok@8
        unresolved external symbol _krb_kntoln
        unresolved external symbol __imp__krb_rd_req@24
*/


/* these were all KRB5_CALLCONV */

static const char * (KRB5_CALLCONV * p_krb_get_err_text_entry)(int _errno)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_cred)(char *service, char *instance,
                                         char *realm, CREDENTIALS *c)=NULL;
static int (KRB5_CALLCONV * p_krb_mk_req)(KTEXT authent, char *service,
                                       char *instance, char *realm,
                                       KRB4_32 checksum)=NULL;
static char * (KRB5_CALLCONV * p_krb_realmofhost)(char *host)=NULL;
static char * (KRB5_CALLCONV_C * p_krb_get_phost)(char *alias)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_pw_in_tkt)
    (char  *user, char  *instance, char  *realm,
      char  *service, char  *sinstance,
      int life, char  *password)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_pw_in_tkt_preauth)
    (char  *user, char  *instance, char  *realm,
                   char  *service, char  *sinstance,
                   int life, char  *password)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_lrealm)(char  *realm, int index)=NULL;
static const char * (KRB5_CALLCONV * p_krb_get_err_text)(int _errno)=NULL;
static int (KRB5_CALLCONV_C * p_kname_parse)(char  *name, char  *inst,
                              char  *realm,char  *fullname)=NULL;
static int (KRB5_CALLCONV_C * p_dest_tkt)(void)=NULL;

static int (KRB5_CALLCONV_C * p_tf_get_pname)(char*)=NULL;
static int (KRB5_CALLCONV_C * p_tf_get_pinst)(char*)=NULL;
static int (KRB5_CALLCONV_C * p_tf_get_cred)(CREDENTIALS*)=NULL;
static void (KRB5_CALLCONV_C * p_tf_close)(void)=NULL;
static int (KRB5_CALLCONV_C * p_tf_init)(char *,int)=NULL;
static char * (KRB5_CALLCONV_C * p_tkt_string)(void)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_tf_realm)(char *ticket_file,
                                             char *realm)=NULL;
static int (KRB5_CALLCONV_C * p_krb_get_tf_fullname)(char *ticket_file,
                                     char *name, char *inst,
                                       char *realm)=NULL;
static int (KRB5_CALLCONV_C * p_krb_check_serv)(char * service)=NULL;
static int (KRB5_CALLCONV_C * p_kuserok)(AUTH_DAT *kdata, char *luser)=NULL;
static int (KRB5_CALLCONV_C * p_k95_k4_userok)( const char * princ_name,
                                const char * princ_inst,
                                const char * princ_realm,
                                const char * local_realm,
                                const char *luser)=NULL;
static int (KRB5_CALLCONV_C * p_krb_kntoln)(AUTH_DAT *kdata, char *luser)=NULL;
static int (KRB5_CALLCONV_C * p_k95_k4_princ_to_userid)( const char * princ_name,
                                         const char * princ_inst,
                                         const char * princ_realm,
                                         const char * local_realm,
                                         char *luser, int len)=NULL;
static int (KRB5_CALLCONV_C * p_krb_rd_req)(KTEXT, char *service, char *inst,
                             unsigned KRB4_32 from_addr, AUTH_DAT  *,
                             char  *srvtab)=NULL;
static int (KRB5_CALLCONV * p_krb_sendauth)(long,int,KTEXT,CHAR *,CHAR *,CHAR *,
                         unsigned long,MSG_DAT *,CREDENTIALS *, Key_schedule *,
                         struct sockaddr_in *,struct sockaddr_in *, CHAR *)=NULL;
static int (KRB5_CALLCONV_C * p_set_krb_debug)(int)=NULL;
static int (KRB5_CALLCONV_C * p_set_krb_ap_req_debug)(int)=NULL;
static long (KRB5_CALLCONV_C *p_krb_mk_safe)(char * in, char * out,
                              unsigned long length,
                              C_Block block,
                              struct sockaddr_in * sender,
                              struct sockaddr_in * receiver)=NULL;
static long (KRB5_CALLCONV_C *p_krb_mk_priv)(char * in, char * out,
                              unsigned long length,
                              Schedule sched,
                              C_Block block,
                              struct sockaddr_in * sender,
                              struct sockaddr_in * receiver)=NULL;
static long (KRB5_CALLCONV_C *p_krb_rd_priv)(char * in, unsigned long in_length,
                              Schedule sched,
                              C_Block block,
                              struct sockaddr_in * sender,
                              struct sockaddr_in * receiver,
                              MSG_DAT * m_data)=NULL;
static long (KRB5_CALLCONV_C *p_krb_rd_safe)(char * in, unsigned long in_length,
                              C_Block block,
                              struct sockaddr_in * sender,
                              struct sockaddr_in * receiver,
                              MSG_DAT * m_data)=NULL;
static int (KRB5_CALLCONV_C *p_krb_in_tkt)(char *, char *, char *)=NULL;
static int (KRB5_CALLCONV_C *p_krb_save_credentials)(char *, char *, char *, C_Block, int, int, KTEXT, long)=NULL;

int 
ck_krb_in_tkt(char *a, char *b, char *c)
{
    if ( p_krb_in_tkt )
        return(p_krb_in_tkt(a,b,c));
    else
        return(KFAILURE);
}

int 
ck_krb_save_credentials(char *a, char *b, char *c, C_Block block, int n, int m, KTEXT k, long l)
{
    if ( p_krb_save_credentials )
        return(p_krb_save_credentials(a,b,c,block,n,m,k,l));
    else
        return(KFAILURE);
}

const char *
ck_krb_get_err_text_entry(int _errno)
{
    if (_errno == -1 )
        return("Function not supported by Kerberos installation");
    if ( p_krb_get_err_text_entry )
        return(p_krb_get_err_text_entry(_errno));
    if ( p_krb_get_err_text )
        return(p_krb_get_err_text(_errno));
    else
        return("");
}

int
ck_krb_get_cred(char *service, char *instance,
                 char *realm, LEASH_CREDENTIALS *c)
{
    if ( p_krb_get_cred )
        return(p_krb_get_cred(service,instance,realm,(CREDENTIALS *)c));
    else
        return(-1);
}

int
ck_krb_mk_req(KTEXT authent, char *service,
               char *instance, char *realm,
               KRB4_32 checksum)
{
    if ( p_krb_mk_req )
        return(p_krb_mk_req(authent,service,
                             instance,realm,checksum));
    else
        return(-1);
}

char *
ck_krb_realmofhost(char *host)
{
    debug(F110,"ck_krb_realmofhost",host,0);
    if ( p_krb_realmofhost )
        return(p_krb_realmofhost(host));
    else
        return("");
}

char *
ck_krb_get_phost(char *alias)
{
    if ( p_krb_get_phost )
        return(p_krb_get_phost(alias));
    else
        return("");
}

int
ck_krb_get_pw_in_tkt( char  *user, char  *instance, char  *realm,
                      char  *service, char  *sinstance,
                      int life, char  *password)
{
    if ( p_krb_get_pw_in_tkt )
        return(p_krb_get_pw_in_tkt(user,instance,realm,service,
                                    sinstance,life,password));
    else
        return(-1);
}

int
ck_krb_get_pw_in_tkt_preauth( char  *user, char  *instance, char  *realm,
                              char  *service, char  *sinstance,
                              int life, char  *password)
{
    if ( p_krb_get_pw_in_tkt_preauth )
        return(p_krb_get_pw_in_tkt_preauth(user,instance,realm,service,
                                            sinstance,life,password));
    else
        return(-1);
}

int
ck_krb_get_lrealm(char  *realm, int index)
{
    if ( p_krb_get_lrealm )
        return(p_krb_get_lrealm(realm,index));
    else
        return(-1);
}

const char *
ck_krb_get_err_text(int _errno)
{
    if ( p_krb_get_err_text )
        return(p_krb_get_err_text(_errno));
    if ( p_krb_get_err_text_entry )
        return(p_krb_get_err_text_entry(_errno));
    else
        return("");
}
int
ck_kname_parse(char  *name, char  *inst,
                char  *realm,char  *fullname)
{
    if ( p_kname_parse )
        return(p_kname_parse(name,inst,realm,fullname));
    return(-1);
}

int
ck_dest_tkt(void)
{
    if ( p_dest_tkt )
        return(p_dest_tkt());
    else
        return(-1);
}

int
ck_tf_get_pname(char* pname)
{
    if ( p_tf_get_pname )
        return(p_tf_get_pname(pname));
    else
        return(-1);
}

int
ck_tf_get_pinst(char* pinst)
{
    if ( p_tf_get_pinst )
        return(p_tf_get_pinst(pinst));
    else
        return(-1);
}

int
ck_tf_get_cred(LEASH_CREDENTIALS* cred)
{
    if ( p_tf_get_cred )
        return(p_tf_get_cred((CREDENTIALS *)cred));
    else
        return(-1);
}

void
ck_tf_close(void)
{
    if ( p_tf_close )
        p_tf_close();
}

int
ck_tf_init(char * p,int n)
{
    if ( p_tf_init )
        return(p_tf_init(p,n));
    else
        return(-1);
}

char *
ck_tkt_string(void)
{
    if ( p_tkt_string )
        return(p_tkt_string());
    else
        return("");
}

int
ck_krb_get_tf_realm(char *ticket_file, char *realm)
{
    if ( p_krb_get_tf_realm )
        return(p_krb_get_tf_realm(ticket_file,realm));
    else
        return(-1);
}

int
ck_krb_get_tf_fullname(char *tf, char * name, char * inst, char *realm)
{
    if ( p_krb_get_tf_fullname )
        return(p_krb_get_tf_fullname(tf,name,inst,realm));
    else
        return(-1);
}

int
ck_krb_check_serv(char * service)
{
    if ( p_krb_check_serv )
        return(p_krb_check_serv(service));
    else
        return(-1);
}

static char lrealm[REALM_SZ] = "";

int
ck_kuserok(AUTH_DAT *kdata, char *luser)
{
    if ( p_k95_k4_userok ) {
        ckstrncpy(lrealm,ck_krb4_getrealm(),REALM_SZ);
        return(p_k95_k4_userok(kdata->pname, kdata->pinst, kdata->prealm,
                                lrealm, luser));
    }
    else if ( p_kuserok )
        return(p_kuserok(kdata,luser));
    else {
        return(stricmp(luser,kdata->pname));
    }
}

int
ck_krb_kntoln(AUTH_DAT *kdata, char *luser)
{

    if ( p_k95_k4_princ_to_userid ) {
        if (krb_get_lrealm(lrealm,1) == KFAILURE)
            lrealm[0] = '\0';
         return(p_k95_k4_princ_to_userid(kdata->pname, kdata->pinst,
                                          kdata->prealm, lrealm,
                                          luser, ANAME_SZ));
    }
    else if ( p_krb_kntoln )
        return(p_krb_kntoln(kdata,luser));
    else {

        if (krb_get_lrealm(lrealm,1) == KFAILURE)
            return(KFAILURE);

        if (strcmp(kdata->pinst,""))
            return(KFAILURE);
        if (strcmp(kdata->prealm,lrealm))
            return(KFAILURE);
        (void) ckstrncpy(luser,kdata->pname,ANAME_SZ);
        return(KSUCCESS);
    }
}

int
ck_krb_rd_req(KTEXT authent, char *service, char *inst,
                             unsigned int from_addr, AUTH_DAT * adat,
                             char *srvtab)
{
    if ( p_krb_rd_req )
        return(p_krb_rd_req(authent, service, inst, from_addr,
                             adat, srvtab));
    else
        return(-1);

}

int
ck_krb_sendauth(long opts, int socket, KTEXT ticket, CHAR * service,
                 CHAR * hostname, CHAR * realm, unsigned long pid,
                 MSG_DAT * msg_dat, LEASH_CREDENTIALS * cred,
                 Schedule * sched, struct sockaddr_in * l_addr,
                 struct sockaddr_in * r_addr, CHAR * str)
{
    if ( p_krb_sendauth )
        return(p_krb_sendauth(opts,socket,ticket,service,hostname,
                               realm,pid, msg_dat, (CREDENTIALS *)cred, sched,
                               l_addr, r_addr, str));
    else
        return(-1);
}

int
ck_set_krb_debug(int x)
{
    if ( p_set_krb_debug )
        return(p_set_krb_debug(x));
    else
        return(0);
}

int
ck_set_krb_ap_req_debug(int x)
{
    if ( p_set_krb_ap_req_debug )
        return(p_set_krb_ap_req_debug(x));
    else
        return(0);
}

long
ck_krb_mk_safe(char * in, char * out,
                unsigned long length,
                C_Block block,
                struct sockaddr_in * sender,
                struct sockaddr_in * receiver)
{
    if ( p_krb_mk_safe )
        return(p_krb_mk_safe(in,out,length,block,sender,receiver));
    else
        return(-1);
}

long
ck_krb_mk_priv(char * in, char * out,
                unsigned long length,
                Schedule sched,
                C_Block block,
                struct sockaddr_in * sender,
                struct sockaddr_in * receiver)
{
    if ( p_krb_mk_priv )
        return(p_krb_mk_priv(in,out,length,sched,block,sender,receiver));
    else
        return(-1);
}

long
ck_krb_rd_priv(char * in, unsigned long in_length,
                Schedule sched,
                C_Block block,
                struct sockaddr_in * sender,
                struct sockaddr_in * receiver,
                MSG_DAT * m_data)
{
    if ( p_krb_rd_priv )
        return(p_krb_rd_priv(in, in_length,sched, block, sender, receiver, m_data));
    else
        return(-1);
}

long
ck_krb_rd_safe(char * in, unsigned long in_length,
                C_Block block,
                struct sockaddr_in * sender,
                struct sockaddr_in * receiver,
                MSG_DAT * m_data)
{
    if ( p_krb_rd_safe )
        return(p_krb_rd_safe(in, in_length, block, sender, receiver,m_data));
    else
        return(-1);
}
#endif /* KRB4 */

#ifdef KRB5
/* These functions are located in DLLs which we are not linking to at */
/* compile time.  Therefore, we have to redirect them to locally      */
/* defined functions which will call the real thing only if they are  */
/* available via Run-Time Linking.                                    */
/*
    From COMERR32.DLL
       unresolved external symbol __imp__com_err
       unresolved external symbol __imp__error_message@4

    From KRB5_32.DLL:
       unresolved external symbol __imp__krb5_free_creds@8
       unresolved external symbol __imp__krb5_copy_keyblock@12
       unresolved external symbol __imp__krb5_free_keyblock@8
       unresolved external symbol __imp__krb5_auth_con_getlocalsubkey@12
       unresolved external symbol __imp__krb5_mk_req_extended@24
       unresolved external symbol __imp__krb5_auth_con_setflags@12
       unresolved external symbol __imp__krb5_auth_con_init@8
       unresolved external symbol __imp__krb5_auth_con_free@8
       unresolved external symbol __imp__krb5_get_credentials@20
       unresolved external symbol __imp__krb5_free_cred_contents@8
       unresolved external symbol __imp__krb5_sname_to_principal@20
       unresolved external symbol __imp__krb5_cc_default@8
       unresolved external symbol __imp__krb5_free_ap_rep_enc_part@8
       unresolved external symbol __imp__krb5_rd_rep@16

       unresolved external symbol __imp__krb5_get_in_tkt_with_keytab@36
       unresolved external symbol __imp__krb5_get_in_tkt_with_password@36
       unresolved external symbol __imp__krb5_read_password@20
       unresolved external symbol __imp__krb5_build_principal_ext
       unresolved external symbol __imp__krb5_unparse_name@12
       unresolved external symbol __imp__krb5_parse_name@12
       unresolved external symbol __imp__krb5_cc_resolve@12
       unresolved external symbol __imp__krb5_string_to_timestamp@8
       unresolved external symbol __imp__krb5_kt_resolve@12
       unresolved external symbol __imp__krb5_string_to_deltat@8
       unresolved external symbol __imp__krb5_timeofday@8
       unresolved external symbol __imp__krb5_get_credentials_renew@20
       unresolved external symbol __imp__krb5_get_credentials_validate@20
       unresolved external symbol __imp__krb5_copy_principal@12
       unresolved external symbol __imp__krb5_timestamp_to_sfstring@16
       unresolved external symbol __imp__krb5_kt_default@8
       unresolved external symbol __imp__krb5_free_ticket@8
       unresolved external symbol _decode_krb5_ticket

     These were added when Server Side functionality was added:
       unresolved external symbol __imp__krb5_auth_con_getremotesubkey@12
       unresolved external symbol __imp__krb5_mk_rep@12
       unresolved external symbol __imp__krb5_free_authenticator@8
       unresolved external symbol __imp__krb5_verify_checksum@28
       unresolved external symbol __imp__krb5_auth_con_getkey@12
       unresolved external symbol __imp__krb5_auth_con_getauthenticator@12
       unresolved external symbol __imp__krb5_rd_req@28
       unresolved external symbol _krb5_auth_con_setrcache
       unresolved external symbol _krb5_get_server_rcache
       unresolved external symbol _krb5_auth_con_getrcache
       unresolved external symbol __imp__krb5_free_tgt_creds@8
       unresolved external symbol __imp__krb5_rd_cred@20

     These were added for 7.1.198 so we can support hardware preauth
     __imp__krb5_get_init_creds_password@36
     __imp__krb5_get_init_creds_opt_set_address_list@8
     __imp__krb5_get_renewed_creds@20
     __imp__krb5_get_validated_creds@20
     __imp__krb5_get_init_creds_opt_set_tkt_life@8
     __imp__krb5_get_init_creds_opt_set_forwardable@8
     __imp__krb5_get_init_creds_opt_set_proxiable@8
     __imp__krb5_get_init_creds_opt_set_renew_life@8
     __imp__krb5_get_init_creds_opt_init@4

     These were added 7-17-2000

     __imp__krb5_cc_get_principal@12
     __imp__krb5_cc_store_cred@12
     __imp__krb5_cc_initialize@12
     __imp__krb5_cc_destroy@8
     __imp__krb5_cc_end_seq_get@12
     __imp__krb5_cc_next_cred@16
     __imp__krb5_cc_start_seq_get@12
     __imp__krb5_cc_get_type
     __imp__krb5_cc_get_name@8
     __imp__krb5_cc_set_flags@12
*/


static void
(KRB5_CALLCONV * p_com_err)(const char *, errcode_t, const char *, ...)=NULL;
static void (KRB5_CALLCONV * p_com_err_va)
  (const char *, errcode_t, const char *, va_list)=NULL;
static const char *
(KRB5_CALLCONV *p_error_message)(errcode_t)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_creds)(krb5_context, krb5_creds*)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_data)(krb5_context, krb5_data *)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_data_contents)(krb5_context, krb5_data *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_copy_keyblock)(krb5_context,
                         krb5_const krb5_keyblock  *,
                         krb5_keyblock  *  *)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_keyblock) (krb5_context, krb5_keyblock  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_auth_con_getlocalsubkey)(krb5_context,
                                                krb5_auth_context,
                                                krb5_keyblock  *  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_mk_req_extended) (krb5_context,
                                         krb5_auth_context  *,
                                         krb5_const krb5_flags,
                                         krb5_data  *,
                                         krb5_creds  *,
                                         krb5_data  * )=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_mk_req)
        (krb5_context,
                krb5_auth_context *,
                krb5_const krb5_flags,
                char *,
                char *,
                krb5_data *,
                krb5_ccache,
                krb5_data * )=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_auth_con_setflags) (krb5_context,
                                           krb5_auth_context,
                                           krb5_int32)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_auth_con_init) (krb5_context,
                                       krb5_auth_context  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_auth_con_free) (krb5_context,
                                       krb5_auth_context)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_credentials)(krb5_context,
                                        krb5_const krb5_flags,
                                        krb5_ccache,
                                        krb5_creds  *,
                                        krb5_creds  *  *)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_cred_contents)(krb5_context, krb5_creds  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_sname_to_principal)(krb5_context,
                                           krb5_const char  *,
                                           krb5_const char  *,
                                           krb5_int32,
                                           krb5_principal  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_cc_default)(krb5_context,
                                   krb5_ccache  *)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_ap_rep_enc_part)(krb5_context,
                                             krb5_ap_rep_enc_part  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_rd_rep)(krb5_context,
                               krb5_auth_context,
                               krb5_const krb5_data  *,
                               krb5_ap_rep_enc_part  *  *)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_init_context)(krb5_context *)=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_context)(krb5_context)=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_init_ets)(krb5_context)=NULL;

static void
(KRB5_CALLCONV *p_krb5_free_principal) P((krb5_context, krb5_principal ))=NULL;
static void
(KRB5_CALLCONV *p_krb5_free_unparsed_name) P((krb5_context, char *))=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_fwd_tgt_creds)P((krb5_context,
                                     krb5_auth_context,
                                     char  *,
                                     krb5_principal,
                                     krb5_principal,
                                     krb5_ccache,
                                     int forwardable,
                                     krb5_data  *))=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_auth_con_genaddrs)P((krb5_context,
                                         krb5_auth_context,
                                         int, int))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_host_realm)P((krb5_context,
                                    const char *,
                                    char ***))=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_free_host_realm)P((krb5_context,
                                    char **))=NULL;
static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_in_tkt_with_keytab)P((krb5_context,
                krb5_const krb5_flags,
                krb5_address * krb5_const *,
                krb5_enctype *,
                krb5_preauthtype *,
                krb5_const krb5_keytab,
                krb5_ccache,
                krb5_creds *,
                krb5_kdc_rep ** ))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_in_tkt_with_password)P((krb5_context,
                krb5_const krb5_flags,
                krb5_address * krb5_const *,
                krb5_enctype *,
                krb5_preauthtype *,
                krb5_const char *,
                krb5_ccache,
                krb5_creds *,
                krb5_kdc_rep **))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_read_password)P((krb5_context,
                const char *,
                const char *,
                char *,
                int * ))=NULL;

static krb5_error_code
(KRB5_CALLCONV_C *p_krb5_build_principal_ext)P((krb5_context,
                                 krb5_principal *,
                                 int,
                                 krb5_const char *,
                                 ...))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_unparse_name)P((krb5_context,
                krb5_const_principal,
                char ** ))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_parse_name)P((krb5_context,
                krb5_const char *,
                krb5_principal *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_cc_resolve)P((krb5_context,
                char *,
                krb5_ccache *))=NULL;

static const char *
(KRB5_CALLCONV *p_krb5_cc_default_name)P((krb5_context))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_string_to_timestamp)P((char *,
                                           krb5_timestamp *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_kt_resolve)P((krb5_context,
                krb5_const char *,
                krb5_keytab *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_string_to_deltat)P((char *, krb5_deltat *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_timeofday)P((krb5_context,
                krb5_int32 *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_credentials_renew)P((krb5_context,
                krb5_const krb5_flags,
                krb5_ccache,
                krb5_creds *,
                krb5_creds **))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_get_credentials_validate)P((krb5_context,
                krb5_const krb5_flags,
                krb5_ccache,
                krb5_creds *,
                krb5_creds **))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_copy_principal)P((krb5_context,
                krb5_const_principal,
                krb5_principal *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_timestamp_to_sfstring)P((krb5_timestamp,
                                             char *,
                                             size_t,
                                             char *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_kt_default)P((krb5_context,
                krb5_keytab *))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_free_ticket)P((krb5_context, krb5_ticket *))=NULL;

static krb5_error_code
(KRB5_CALLCONV_C *p_decode_krb5_ticket)P((const krb5_data *code,
                                     krb5_ticket **rep))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_cygnus_decode_krb5_ticket)P((const krb5_data *code,
                                     krb5_ticket **rep))=NULL;

#ifdef CHECKADDRS
static krb5_error_code
(KRB5_CALLCONV *p_krb5_os_localaddr)P((krb5_context con,
                                    krb5_address *** ppp))=NULL;

static krb5_boolean
(KRB5_CALLCONV_C  *p_krb5_address_search)P((krb5_context con,
                krb5_const krb5_address * p,
                krb5_address * krb5_const * pp))=NULL;

static void
(KRB5_CALLCONV * p_krb5_free_addresses)P((krb5_context con,
                                       krb5_address ** pp))=NULL;
#endif /* CHECKADDRS */

static krb5_error_code
(KRB5_CALLCONV * p_krb5_auth_con_getremotesubkey)P((krb5_context,
                                                 krb5_auth_context,
                                                 krb5_keyblock **))
                                              = NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_mk_rep)P((krb5_context, krb5_auth_context,
                               krb5_data *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_free_authenticator)P((krb5_context,
                                           krb5_authenticator *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_verify_checksum)
P((krb5_context context,
    krb5_const krb5_cksumtype ctype,
    krb5_const krb5_checksum * cksum,
    krb5_const krb5_pointer in, krb5_const size_t in_length,
    krb5_const krb5_pointer seed, krb5_const size_t seed_length))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_auth_con_getkey)
P((krb5_context, krb5_auth_context, krb5_keyblock **))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_auth_con_getauthenticator)
P((krb5_context, krb5_auth_context, krb5_authenticator **))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_rd_req)
P((krb5_context, krb5_auth_context *, krb5_const krb5_data *,
    krb5_const_principal, krb5_keytab, krb5_flags *,
    krb5_ticket **))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_auth_con_setrcache)
P((krb5_context, krb5_auth_context, krb5_rcache))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_get_server_rcache)
P((krb5_context, krb5_const krb5_data *, krb5_rcache *))=NULL;

static krb5_error_code
(KRB5_CALLCONV_C * p_krb5_auth_con_getrcache)
P((krb5_context, krb5_auth_context, krb5_rcache *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_free_tgt_creds)
P((krb5_context, krb5_creds ** ))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_rd_cred)
P((krb5_context,krb5_auth_context,krb5_data *, krb5_creds ***,
    krb5_replay_data *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_c_enctype_compare)
P((krb5_context, krb5_enctype, krb5_enctype, krb5_boolean *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_free_error)
P((krb5_context, krb5_error *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_sendauth)
P((krb5_context,
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
    krb5_creds **))=NULL;

static krb5_error_code
(KRB5_CALLCONV *p_krb5_process_key)
P((krb5_context, krb5_encrypt_block *, const krb5_keyblock *))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_use_enctype)
P((krb5_context, krb5_encrypt_block *, const krb5_enctype))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_encrypt)
P((krb5_context context,
    krb5_const krb5_pointer inptr,
    krb5_pointer outptr,
    krb5_const size_t size,
    krb5_encrypt_block * eblock,
    krb5_pointer ivec))=NULL;

static size_t
(KRB5_CALLCONV * p_krb5_encrypt_size)
P((krb5_const size_t length,
    krb5_enctype crypto))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_decrypt)
P((krb5_context context,
    krb5_const krb5_pointer inptr,
    krb5_pointer outptr,
    krb5_const size_t size,
    krb5_encrypt_block  * eblock,
    krb5_pointer ivec))=NULL;

static krb5_boolean
(KRB5_CALLCONV * p_krb5_kuserok)
P((krb5_context context,krb5_principal princ, const char * p))=NULL;

static krb5_error_code
(KRB5_CALLCONV * p_krb5_aname_to_localname)
P((krb5_context context, krb5_const_principal aname, const int lnsize, char *lname))=NULL;

static int
(KRB5_CALLCONV_C * p_k95_k5_userok)
P((const char * princ_name, const char * princ_inst, const char * princ_realm,
   const char * local_realm, const char * userid))=NULL;

static int
(KRB5_CALLCONV_C * p_k95_k5_principal_to_localname)
P((const char * princ_name, const char * local_realm, char * userid, int len))=NULL;

static void
(KRB5_CALLCONV_C * p_krb5_appdefault_boolean)
P((krb5_context,const char *,const krb5_data *,const char *,
     int,int * ))=NULL;
static void
(KRB5_CALLCONV_C * p_krb5_appdefault_string)
P((krb5_context,const char *,const krb5_data *,const char *,
    char ** ))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_init)
    P((krb5_get_init_creds_opt *opt))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_tkt_life)
    P((krb5_get_init_creds_opt *opt,
                 krb5_deltat tkt_life))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_renew_life)
    P((krb5_get_init_creds_opt *opt,
        krb5_deltat renew_life))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_forwardable)
    P((krb5_get_init_creds_opt *opt,
        int forwardable))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_proxiable)
    P((krb5_get_init_creds_opt *opt,
        int proxiable))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_etype_list)
    P((krb5_get_init_creds_opt *opt,
        krb5_enctype *etype_list,
        int etype_list_length))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_address_list)
    P((krb5_get_init_creds_opt *opt, krb5_address **addresses))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_preauth_list)
    P((krb5_get_init_creds_opt *opt,
        krb5_preauthtype *preauth_list,
        int preauth_list_length))=NULL;

static void (KRB5_CALLCONV * p_krb5_get_init_creds_opt_set_salt)
    P((krb5_get_init_creds_opt *opt, krb5_data *salt))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_get_init_creds_password)
    P((krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        char *password,
        krb5_prompter_fct prompter,
        void *data,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_get_init_creds_keytab)
    P((krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_keytab arg_keytab,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_get_validated_creds)
    P((krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_get_renewed_creds)
    P((krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_rd_safe)
   P((krb5_context,
       krb5_auth_context,
       krb5_const krb5_data  *,
       krb5_data  *,
       krb5_replay_data  *))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_mk_safe)
    P((krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_rd_priv)
    P((krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_mk_priv)
    P((krb5_context,
        krb5_auth_context,
        krb5_const krb5_data  *,
        krb5_data  *,
        krb5_replay_data  *))=NULL;

static krb5_error_code (KRB5_CALLCONV *p_krb5_auth_con_setuseruserkey)
    P((krb5_context,
        krb5_auth_context,
        krb5_keyblock *))=NULL;

static krb5_error_code (KRB5_CALLCONV *p_krb5_get_profile)
    P((krb5_context, profile_t *))=NULL;

static long (KRB5_CALLCONV *p_profile_get_relation_names)
    P((profile_t profile, const char **names, char ***ret_names))=NULL;

static long (KRB5_CALLCONV *p_profile_get_subsection_names)
    P((profile_t profile, const char **names, char ***ret_names))=NULL;

static void (KRB5_CALLCONV *p_krb5_free_keyblock_contents)
P((krb5_context, krb5_keyblock FAR *))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_c_encrypt)
    P((krb5_context context, krb5_const krb5_keyblock *key,
        krb5_keyusage usage, krb5_const krb5_data *ivec,
        krb5_const krb5_data *input, krb5_enc_data *output))=NULL;

static krb5_error_code (KRB5_CALLCONV *p_krb5_c_decrypt)
    P((krb5_context context, krb5_const krb5_keyblock *key,
        krb5_keyusage usage, krb5_const krb5_data *ivec,
        krb5_const krb5_enc_data *input, krb5_data *output))=NULL;

static krb5_error_code (KRB5_CALLCONV *p_krb5_c_block_size)
    P((krb5_context context, krb5_enctype enctype,
        size_t *blocksize))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_c_make_random_key)
    P((krb5_context context, krb5_enctype enctype,
       krb5_keyblock *random_key))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_c_random_seed)
    P((krb5_context context, krb5_data *data))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_c_encrypt_length)
    P((krb5_context context, krb5_enctype enctype,
      size_t inputlen, size_t *length))=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_default_name)
    P((krb5_context context, char *, int))=NULL;

static char * (KRB5_CALLCONV * p_krb5_cc_get_name )
    (krb5_context context, krb5_ccache cache)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_gen_new )
    (krb5_context context, krb5_ccache *cache)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_initialize)
    (krb5_context context, krb5_ccache cache,
                    krb5_principal principal)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_destroy )
    (krb5_context context, krb5_ccache cache)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_close )
    (krb5_context context, krb5_ccache cache)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_store_cred )
    (krb5_context context, krb5_ccache cache,
                     krb5_creds *creds)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_retrieve_cred )
    (krb5_context context, krb5_ccache cache,
      krb5_flags flags, krb5_creds *mcreds,
      krb5_creds *creds)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_get_principal )
    (krb5_context context, krb5_ccache cache,
      krb5_principal *principal)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_start_seq_get )
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_next_cred )
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor, krb5_creds *creds)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_end_seq_get )
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_remove_cred )
    (krb5_context context, krb5_ccache cache, krb5_flags flags,
      krb5_creds *creds)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_cc_set_flags )
    (krb5_context context, krb5_ccache cache, krb5_flags flags)=NULL;

static const char * (KRB5_CALLCONV * p_krb5_cc_get_type )
    (krb5_context context, krb5_ccache cache)=NULL;

static const char * (KRB5_CALLCONV * p_krb5_kt_get_type )
    (krb5_context context, krb5_keytab)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_get_name)
    (krb5_context, krb5_keytab, char *, int)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_close)
    (krb5_context, krb5_keytab)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_get_entry)
    (krb5_context, krb5_keytab,
                 krb5_principal,
                 krb5_kvno,
                 krb5_enctype,
                 krb5_keytab_entry *)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_start_seq_get)
    (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *)=NULL;
static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_next_entry)
        (krb5_context,
                 krb5_keytab,
                 krb5_keytab_entry *,
                 krb5_kt_cursor *)=NULL;

static krb5_error_code (KRB5_CALLCONV * p_krb5_kt_end_seq_get)
        (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *)=NULL;

static krb5_error_code (KRB5_CALLCONV_C * p_krb5_build_principal)
    (krb5_context, krb5_principal *, unsigned int, krb5_const char *, ...)=NULL;


static int (KRB5_CALLCONV_C *p_krb524_init_ets)(krb5_context context)=NULL;
static int (KRB5_CALLCONV_C *p_krb524_convert_creds_kdc)(krb5_context context, 
                                                         krb5_creds *v5creds,
                                                         CREDENTIALS *v4creds)=NULL;

const char *
ck_error_message(errcode_t ec)
{
    if ( ec == 0 )
        return("OK");
    else if ( ec == -1 )
        return("Entry point not found in Dynamic Link Library");
    else if ( p_error_message )
        return(p_error_message(ec));
    else
        return ckitoa(ec);
}

void
ck_com_err(const char * s1, errcode_t ec, const char * s2, ...)
{
    va_list ap;
    const char * msg= NULL;

    va_start( ap, s2 );
    msg = error_message(ec);

    ckmakxmsg(tn_msg,TN_MSG_LEN, (char *)(s1?s1:""), 
               (char *)(s1?" ":""), (char *)msg, (char *)(s2?" ":""), 
               (char *)(s2?s2:""),
               NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    debug(F111,"KERBEROS",tn_msg,ec);

    ckmakmsg(tn_msg,TN_MSG_LEN,"?Kerberos 5 error: ",(char *)msg,s2?" ":"",(char *)(s2?s2:""));
    if (tn_deb || debses) tn_debug(tn_msg);
    printf("%s\n",tn_msg);
}

void
ck_krb5_free_creds(krb5_context CO, krb5_creds  * CR)
{
    if ( p_krb5_free_creds )
        p_krb5_free_creds(CO,CR);
}

void
ck_krb5_free_data(krb5_context CO, krb5_data  * data)
{
    if ( p_krb5_free_data )
        p_krb5_free_data(CO,data);
}

void
ck_krb5_free_data_contents(krb5_context CO, krb5_data  * data)
{
    if ( p_krb5_free_data_contents )
        p_krb5_free_data_contents(CO,data);
}

krb5_error_code
ck_krb5_copy_keyblock(krb5_context CO,
                       krb5_const krb5_keyblock  * pKB,
                       krb5_keyblock ** ppKB)
{
    if ( p_krb5_copy_keyblock )
        return(p_krb5_copy_keyblock(CO,pKB,ppKB));
    else
        return KRB5KRB_ERR_GENERIC;
}

void
ck_krb5_free_keyblock (krb5_context CO, krb5_keyblock  * pKB)
{
    if ( p_krb5_free_keyblock )
        p_krb5_free_keyblock(CO,pKB);

}
krb5_error_code
ck_krb5_auth_con_getlocalsubkey(krb5_context CO,
                                 krb5_auth_context ACO,
                                 krb5_keyblock  ** ppKB)
{
    if ( p_krb5_auth_con_getlocalsubkey )
        return(p_krb5_auth_con_getlocalsubkey(CO,ACO,ppKB));
    else
        return KRB5KRB_ERR_GENERIC;

}
krb5_error_code
ck_krb5_mk_req_extended (krb5_context CO,
                          krb5_auth_context  * pACO,
                          krb5_const krb5_flags F,
                          krb5_data  * pD1,
                          krb5_creds  * pC,
                          krb5_data  * pD2)
{
    if ( p_krb5_mk_req_extended )
        return(p_krb5_mk_req_extended(CO,pACO,F,pD1,pC,pD2));
    else
        return KRB5KRB_ERR_GENERIC;
}
krb5_error_code
ck_krb5_mk_req(krb5_context CO,
                krb5_auth_context *pACO,
                krb5_const krb5_flags F,
                char * s1,
                char * s2,
                krb5_data * pD1,
                krb5_ccache pC,
                krb5_data * pD2)
{
    if ( p_krb5_mk_req )
        return(p_krb5_mk_req(CO,pACO,F,s1,s2,pD1,pC,pD2));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_setflags (krb5_context CO,
                            krb5_auth_context ACO,
                            krb5_int32 I)
{
    if ( p_krb5_auth_con_setflags )
        return(p_krb5_auth_con_setflags(CO,ACO,I));
    else
        return KRB5KRB_ERR_GENERIC;
}
krb5_error_code
ck_krb5_auth_con_init(krb5_context CO,
                       krb5_auth_context  * pACO)
{
    if ( p_krb5_auth_con_init )
        return(p_krb5_auth_con_init(CO,pACO));
    else
        return KRB5KRB_ERR_GENERIC;
}
krb5_error_code
ck_krb5_auth_con_free (krb5_context CO,
                        krb5_auth_context ACO)
{
    if ( p_krb5_auth_con_free )
        return(p_krb5_auth_con_free(CO,ACO));
    else
        return KRB5KRB_ERR_GENERIC;
}
krb5_error_code
ck_krb5_get_credentials(krb5_context CO,
                         krb5_const krb5_flags F,
                         krb5_ccache CC,
                         krb5_creds  * pCR,
                         krb5_creds  ** ppCR)
{
    if ( p_krb5_get_credentials )
        return(p_krb5_get_credentials(CO,F,CC,pCR,ppCR));
    else
        return KRB5KRB_ERR_GENERIC;
}
void
ck_krb5_free_cred_contents(krb5_context CO, krb5_creds  * pCR)
{
    if ( p_krb5_free_cred_contents )
        p_krb5_free_cred_contents(CO,pCR);
}

krb5_error_code
ck_krb5_sname_to_principal(krb5_context CO,
                            krb5_const char  * pC1,
                            krb5_const char  * pC2,
                            krb5_int32 I,
                            krb5_principal  * pPR)
{
    if ( p_krb5_sname_to_principal )
        return(p_krb5_sname_to_principal(CO,pC1,pC2,I,pPR));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_cc_default(krb5_context CO,
                    krb5_ccache  * pCC)
{
    if ( p_krb5_cc_default )
        return(p_krb5_cc_default(CO,pCC));
    else
        return KRB5KRB_ERR_GENERIC;
}

void
ck_krb5_free_ap_rep_enc_part(krb5_context CO, krb5_ap_rep_enc_part * pAP)
{
    if ( p_krb5_free_ap_rep_enc_part )
        p_krb5_free_ap_rep_enc_part(CO,pAP);
}

krb5_error_code
ck_krb5_rd_rep(krb5_context CO,
                krb5_auth_context AC,
                krb5_const krb5_data  * pD,
                krb5_ap_rep_enc_part  ** ppAP)
{
    if ( p_krb5_rd_rep )
        return(p_krb5_rd_rep(CO,AC,pD,ppAP));
    else
        return KRB5KRB_ERR_GENERIC;
}


krb5_error_code
ck_krb5_init_context(krb5_context * pCO)
{
    if ( p_krb5_init_context )
        return(p_krb5_init_context(pCO));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_init_ets(krb5_context CO)
{
    if ( p_krb5_init_ets )
        return(p_krb5_init_ets(CO));
    else
        return KRB5KRB_ERR_GENERIC;
}

void
ck_krb5_free_context(krb5_context CO)
{

    if ( p_krb5_free_context )
        p_krb5_free_context(CO);
}

void
ck_krb5_free_principal(krb5_context c, krb5_principal p)
{
    if ( p_krb5_free_principal )
        p_krb5_free_principal(c,p);
}

void
ck_krb5_free_unparsed_name(krb5_context c, char * p)
{
    if ( p_krb5_free_unparsed_name )
        p_krb5_free_unparsed_name(c,p);
    else
        free(p);
}

krb5_error_code
ck_krb5_fwd_tgt_creds(krb5_context con,
                       krb5_auth_context acon,
                       char  * ptr,
                       krb5_principal prin1,
                       krb5_principal prin2,
                       krb5_ccache cc,
                       int fwd,
                       krb5_data  * data)
{
    if ( p_krb5_fwd_tgt_creds )
        return(p_krb5_fwd_tgt_creds(con,acon,ptr,
                                     prin1,prin2,cc,fwd,data));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_genaddrs(krb5_context con,
                           krb5_auth_context acon,
                           int n, int m)
{
    if ( p_krb5_auth_con_genaddrs )
        return(p_krb5_auth_con_genaddrs(con,acon,n,m));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_host_realm(krb5_context con,
                        const char * host,
                        char *** realm)
{
    if ( p_krb5_get_host_realm )
        return(p_krb5_get_host_realm(con,host,realm));
    else {
        if ( realm )
            *realm = NULL;
        return KRB5KRB_ERR_GENERIC;
    }
}

krb5_error_code
ck_krb5_free_host_realm(krb5_context con,
                        char ** realmlist)
{
    if ( p_krb5_free_host_realm )
        return(p_krb5_free_host_realm(con,realmlist));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_in_tkt_with_keytab(krb5_context con,
                krb5_const krb5_flags flags,
                krb5_address * krb5_const * addr,
                krb5_enctype * enc,
                krb5_preauthtype * preauth,
                krb5_const krb5_keytab kt,
                krb5_ccache cc,
                krb5_creds * creds,
                krb5_kdc_rep ** rep)
{
    if ( p_krb5_get_in_tkt_with_keytab )
        return(p_krb5_get_in_tkt_with_keytab(con,flags,addr,enc,preauth,
                                              kt,cc,creds,rep));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_in_tkt_with_password(krb5_context con,
                krb5_const krb5_flags flags,
                krb5_address * krb5_const * addr,
                krb5_enctype * enc,
                krb5_preauthtype * preauth,
                krb5_const char * sz,
                krb5_ccache cc,
                krb5_creds * creds,
                krb5_kdc_rep ** rep)
{
    if ( p_krb5_get_in_tkt_with_password )
        return(p_krb5_get_in_tkt_with_password(con,flags,addr,enc,
                                                preauth,sz,cc,creds,rep));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_read_password(krb5_context con,
                const char * sz1,
                const char * sz2,
                char * sz3,
                int * pI)
{
    if ( p_krb5_read_password )
        return(p_krb5_read_password(con,sz1,sz2,sz3,pI));
    else
        return KRB5KRB_ERR_GENERIC;
}

/* We can only due this because we know how many parameters */
/* we are going to use in the module.                       */

krb5_error_code
ck_krb5_build_principal_ext(krb5_context con,
                             krb5_principal * princ,
                             int n,
                             krb5_const char * sz,
                             int n2,
                             krb5_const char * sz2,
                             int n3,
                             krb5_const char * sz3,
                             ...)
{
    if ( p_krb5_build_principal_ext )
        return(p_krb5_build_principal_ext(con,princ,n,sz,n2,sz2,n3,sz3,0));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_unparse_name(krb5_context con,
                krb5_const_principal princ,
                char ** psz)
{
    if ( p_krb5_unparse_name )
        return(p_krb5_unparse_name(con,princ,psz));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_parse_name(krb5_context con,
                krb5_const char * sz,
                krb5_principal * princ)

{
    if ( p_krb5_parse_name )
        return(p_krb5_parse_name(con,sz,princ));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_cc_resolve(krb5_context con,
                char * sz,
                krb5_ccache * cc)
{
    if ( p_krb5_cc_resolve )
        return(p_krb5_cc_resolve(con,sz,cc));
    else
        return KRB5KRB_ERR_GENERIC;
}

const char *
ck_krb5_cc_default_name(krb5_context con)
{
    if ( p_krb5_cc_default_name )
        return(p_krb5_cc_default_name(con));
    else
        return NULL;
}

krb5_error_code
ck_krb5_string_to_timestamp(char * sz,
                             krb5_timestamp * ts)
{
    if ( p_krb5_string_to_timestamp )
        return(p_krb5_string_to_timestamp(sz,ts));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_kt_resolve(krb5_context con,
                    krb5_const char * sz,
                    krb5_keytab * kt)
{
    if ( p_krb5_kt_resolve )
        return(p_krb5_kt_resolve(con,sz,kt));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_string_to_deltat(char * sz,
                          krb5_deltat * dt)
{
    if ( p_krb5_string_to_deltat )
        return(p_krb5_string_to_deltat(sz,dt));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_timeofday(krb5_context con,
                krb5_int32 * pN)
{
    if ( p_krb5_timeofday )
        return(p_krb5_timeofday(con,pN));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_credentials_renew(krb5_context con,
                krb5_const krb5_flags flags,
                krb5_ccache cc,
                krb5_creds * pCreds,
                krb5_creds ** ppCreds)
{
    if ( p_krb5_get_credentials_renew )
        return(p_krb5_get_credentials_renew(con,flags,cc,pCreds,ppCreds));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_credentials_validate(krb5_context con,
                krb5_const krb5_flags flags,
                krb5_ccache cc,
                krb5_creds * pCreds,
                krb5_creds ** ppCreds)
{
    if ( p_krb5_get_credentials_validate )
        return(p_krb5_get_credentials_validate(con,flags,cc,pCreds,ppCreds));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_copy_principal(krb5_context con,
                krb5_const_principal princ,
                krb5_principal * pPrinc)
{
    if ( p_krb5_copy_principal )
        return(p_krb5_copy_principal(con,princ,pPrinc));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_timestamp_to_sfstring(krb5_timestamp ts,
                               char * sz1,
                               size_t size,
                               char * sz2)
{
    if ( p_krb5_timestamp_to_sfstring )
        return(p_krb5_timestamp_to_sfstring(ts,sz1,size,sz2));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_kt_default(krb5_context con,
                    krb5_keytab * kt)
{
    if ( p_krb5_kt_default )
        return(p_krb5_kt_default(con,kt));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_free_ticket(krb5_context con,
                     krb5_ticket * kt)
{
    if ( p_krb5_free_ticket )
        return(p_krb5_free_ticket(con,kt));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_decode_krb5_ticket(const krb5_data *code,
                       krb5_ticket **rep)
{
    if ( p_decode_krb5_ticket )
        return(p_decode_krb5_ticket(code,rep));
    else if ( p_cygnus_decode_krb5_ticket )
        return(p_cygnus_decode_krb5_ticket(code,rep));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_os_localaddr(krb5_context con, krb5_address *** ppp)
{
    if ( p_krb5_os_localaddr )
        return(p_krb5_os_localaddr(con,ppp));
    else
        return KRB5KRB_ERR_GENERIC;
}

BOOL
ck_krb5_address_compare(krb5_context context,
                         const krb5_address *addr1,
                         const krb5_address *addr2)
{
    if( addr1->addrtype != addr2->addrtype )
        return(FALSE);
    if( addr1->length != addr2->length )
        return(FALSE);
    if( memcmp((char *)addr1->contents,(char *)addr2->contents, addr1->length))
        return FALSE;
    else
        return TRUE;
}

krb5_boolean
ck_krb5_address_search(krb5_context context,
                krb5_const krb5_address * addr,
                krb5_address * krb5_const * addrlist)
{
    if ( p_krb5_address_search )
        return(p_krb5_address_search(context,addr,addrlist));
    else {
        if( !addrlist )
            return TRUE;
        for( ; *addrlist ; addrlist++ )
        {
            if( ck_krb5_address_compare(context, addr, *addrlist) )
                return TRUE;
        }
    }
    return FALSE;
}


void
ck_krb5_free_addresses(krb5_context con, krb5_address ** pp)
{
    if ( p_krb5_free_addresses )
        p_krb5_free_addresses(con,pp);
}

krb5_error_code
ck_krb5_auth_con_getremotesubkey(krb5_context con,
                                  krb5_auth_context acon,
                                  krb5_keyblock ** keyb)
{
    if ( p_krb5_auth_con_getremotesubkey )
        return(p_krb5_auth_con_getremotesubkey(con,acon,keyb));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_mk_rep(krb5_context con, krb5_auth_context auth_con,
                               krb5_data * data)
{
    if ( p_krb5_mk_rep )
        return(p_krb5_mk_rep(con,auth_con, data));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_free_authenticator(krb5_context con, krb5_authenticator * pauth)
{
    if ( p_krb5_free_authenticator )
        return(p_krb5_free_authenticator(con,pauth));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_verify_checksum(krb5_context context,
                         krb5_const krb5_cksumtype ctype,
                         krb5_const krb5_checksum * cksum,
                         krb5_const krb5_pointer in,
                         krb5_const size_t in_length,
                         krb5_const krb5_pointer seed,
                         krb5_const size_t seed_length)
{
    if ( p_krb5_verify_checksum )
        return(p_krb5_verify_checksum(context, ctype, cksum, in,
                                      in_length, seed, seed_length));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_getkey(krb5_context con, krb5_auth_context acon,
                         krb5_keyblock ** ppkeyb)
{
    if ( p_krb5_auth_con_getkey )
        return(p_krb5_auth_con_getkey(con,acon,ppkeyb));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_getauthenticator(krb5_context con, krb5_auth_context acon,
                                   krb5_authenticator ** ppauth)
{
    if ( p_krb5_auth_con_getauthenticator )
        return(p_krb5_auth_con_getauthenticator(con,acon,ppauth));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_rd_req(krb5_context con, krb5_auth_context * pacon,
                krb5_const krb5_data * data,
                krb5_const_principal princ, krb5_keytab keytab,
                krb5_flags * flags, krb5_ticket ** pptkt)
{
    if ( p_krb5_rd_req )
        return(p_krb5_rd_req(con,pacon,data,princ,keytab,flags,pptkt));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_setrcache(krb5_context con, krb5_auth_context acon,
                            krb5_rcache rcache)
{
    if ( p_krb5_auth_con_setrcache )
        return(p_krb5_auth_con_setrcache(con,acon,rcache));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_get_server_rcache(krb5_context con, krb5_const krb5_data * data,
                       krb5_rcache * rcache)
{
    if ( p_krb5_get_server_rcache )
        return(p_krb5_get_server_rcache(con,data,rcache));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_auth_con_getrcache(krb5_context con, krb5_auth_context acon,
                            krb5_rcache * prcache)
{
    if ( p_krb5_auth_con_getrcache )
        return(p_krb5_auth_con_getrcache(con,acon, prcache));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_free_tgt_creds(krb5_context con, krb5_creds ** ppcreds)
{
    if ( p_krb5_free_tgt_creds )
        return(p_krb5_free_tgt_creds(con,ppcreds));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_rd_cred(krb5_context con, krb5_auth_context acon,
                 krb5_data * data, krb5_creds *** pppcreds,
                 krb5_replay_data * replay_data)
{
    if ( p_krb5_rd_cred )
        return(p_krb5_rd_cred(con,acon,data,pppcreds,replay_data));
    else
        return KRB5KRB_ERR_GENERIC;
}

krb5_error_code
ck_krb5_c_enctype_compare(krb5_context con, krb5_enctype e1, krb5_enctype e2,
                           krb5_boolean * pb)
{
    if ( p_krb5_c_enctype_compare )
        return(p_krb5_c_enctype_compare(con, e1,e2, pb));
    else
        return(-1);
}

krb5_error_code
ck_krb5_write_message(krb5_context con, krb5_pointer ptr, krb5_data *data)
{
    int fd = *((int *)ptr);
    long msglen;

    msglen = htonl(data->length);
    if (net_write(fd,(CHAR *)&msglen,4) != 4) {
        return(-1);
    }
    if ( data->length ) {
        if (net_write(fd,data->data,data->length) != data->length) {
            return(-1);
        }
    }
    return(0);
}

krb5_error_code
ck_krb5_read_message( krb5_context context,
                      krb5_pointer ptr,
                      krb5_data * data)
{
    extern int ttyfd;
    int fd = *((int *)ptr);
    long msglen;
    char *p;
    int i, rc;

    if (net_read(fd,&msglen,4) < 0)
        return(-1);

    data->length = ntohl(msglen);
    if ( data->length ) {
        data->data = malloc(data->length);

        i = 0;
        while ( i < data->length ) {
            if ((rc = net_read(fd,&data->data[i],(data->length - i))) < 0)
                return(-1);
            i += rc;
        }
    }
    return(0);
}

krb5_error_code
ck_krb5_free_error(krb5_context con, krb5_error * error)
{
    if ( p_krb5_free_error )
        return(p_krb5_free_error(con,error));
    else
        return(-1);
}

krb5_error_code
ck_krb5_sendauth(krb5_context con,
                  krb5_auth_context * auth,
                  krb5_pointer ptr,
                  char * str,
                  krb5_principal princ1,
                  krb5_principal princ2,
                  krb5_flags flags,
                  krb5_data * data,
                  krb5_creds * creds,
                  krb5_ccache cc,
                  krb5_error ** error,
                  krb5_ap_rep_enc_part ** ap_rep,
                  krb5_creds ** pcreds)
{
    if ( p_krb5_sendauth )
        return(p_krb5_sendauth(con,auth,ptr,str,princ1,princ2,flags,
                                data, creds, cc, error, ap_rep, pcreds));
    else
        return(-1);
}


krb5_error_code
ck_krb5_process_key(krb5_context con,
                     krb5_encrypt_block * eblock, const krb5_keyblock * kblock)
{
    if ( p_krb5_process_key )
        return(p_krb5_process_key(con,eblock,kblock));
    else
        return(-1);
}

krb5_error_code
ck_krb5_use_enctype(krb5_context con, krb5_encrypt_block * eblock,
                     const krb5_enctype etype)
{
    if ( p_krb5_use_enctype )
        return(p_krb5_use_enctype(con,eblock,etype));
    else
        return(-1);
}

krb5_error_code
ck_krb5_encrypt(krb5_context context,
                 krb5_const krb5_pointer inptr,
                 krb5_pointer outptr,
                 krb5_const size_t size,
                 krb5_encrypt_block * eblock,
                 krb5_pointer ivec)
{
    if ( p_krb5_encrypt )
        return(p_krb5_encrypt(context,inptr,outptr,size,eblock,ivec));
    else
        return(-1);
}

size_t
ck_krb5_encrypt_size(krb5_const size_t length,
                      krb5_enctype crypto)
{
    if ( p_krb5_encrypt_size )
        return(p_krb5_encrypt_size(length,crypto));
    else
        return((size_t)-1);
}

krb5_error_code
ck_krb5_decrypt(krb5_context context,
                 krb5_const krb5_pointer inptr,
                 krb5_pointer outptr,
                 krb5_const size_t size,
                 krb5_encrypt_block * eblock,
                 krb5_pointer ivec)
{
    if ( p_krb5_decrypt )
        return(p_krb5_decrypt(context,inptr,outptr,size,eblock,ivec));
    else
        return(-1);
}

#ifdef COMMENT
krb5_error_code
ck_k5_princ_to_userid(krb5_context context, krb5_principal princ, char ** userid)
{
    krb5_error_code code = 0;

    if ( p_k95_k5_princ_to_userid ) {
        char * princ_name;
        char * princ_inst;
        char * princ_realm;
        char * new_userid;
        int r;
        int len;
        char * data;

        /* Principal Name */
        len =  krb5_princ_component(k5_context, princ,0)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_component(k5_context, princ,0)->data;
        princ_name = (char *) malloc(len + 1);
        if (!princ_name)
            goto exitpoint;
        ckstrncpy(princ_name,data,len+1);

        /* Principal Instance */
        len =  krb5_princ_component(k5_context, princ,1)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_component(k5_context, princ,1)->data;
        princ_inst = (char *) malloc(len + 1);
        if (!princ_inst)
            goto exitpoint;
        ckstrncpy(princ_inst,data,len+1);

        /* Principal Realm */
        len =  krb5_princ_realm(k5_context, princ)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_realm(k5_context, princ)->data;
        princ_realm = (char *) malloc(len + 1);
        if (!princ_realm)
            goto exitpoint;
        ckstrncpy(princ_realm,data,len+1);

        new_userid = (char *) malloc(256);
        if ( !new_userid )
            goto exitpoint;

        r = p_k95_k5_princ_to_userid(princ_name,princ_inst,princ_realm,
                                      krb5_d_realm ? krb5_d_realm :
                                      ck_krb5_getrealm(krb5_d_cc),
                                      new_userid,256);
        if (r == 0)
            *userid = new_userid;
        else
            free(new_userid);

      exitpoint:
        if ( princ_name )
            free(princ_name);
        if ( princ_inst )
            free(princ_inst);
        if ( princ_realm )
            free(princ_realm);
        return(r);
    } else {
        if (code = krb5_unparse_name( k5_context, princ, userid))
            *userid = 0;
    }
    return(code);
}
#endif

krb5_boolean
ck_krb5_kuserok(krb5_context context,krb5_principal princ, const char * p)
{
    if ( p_k95_k5_userok ) {
        char * princ_name;
        char * princ_inst;
        char * princ_realm;
        int r;
        int len;
        char * data;

        /* Principal Name */
        len =  krb5_princ_component(k5_context, princ,0)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_component(k5_context, princ,0)->data;
        princ_name = (char *) malloc(len + 1);
        if (!princ_name)
            goto exitpoint;
        ckstrncpy(princ_name,data,len+1);

        /* Principal Instance */
        len =  krb5_princ_component(k5_context, princ,1)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_component(k5_context, princ,1)->data;
        princ_inst = (char *) malloc(len + 1);
        if (!princ_inst)
            goto exitpoint;
        ckstrncpy(princ_inst,data,len+1);

        /* Principal Realm */
        len =  krb5_princ_realm(k5_context, princ)->length;
        if ( len <= 0 ) {
            len = 0;
            data = "";
        } else
            data = krb5_princ_realm(k5_context, princ)->data;
        princ_realm = (char *) malloc(len + 1);
        if (!princ_realm)
            goto exitpoint;
        ckstrncpy(princ_realm,data,len+1);

        r = p_k95_k5_userok(princ_name,princ_inst,princ_realm,
                             krb5_d_realm ? krb5_d_realm :
                             ck_krb5_getrealm(krb5_d_cc),p);

      exitpoint:
        if ( princ_name )
            free(princ_name);
        if ( princ_inst )
            free(princ_inst);
        if ( princ_realm )
            free(princ_realm);
        return(r);
    } else if ( p_krb5_kuserok )
        return(p_krb5_kuserok(context,princ,p));
    else if ( !strncmp(p,
        krb5_princ_component(k5_context,
                             princ,0)->data,
        krb5_princ_component(k5_context,
                             princ,0)->length)
              )
        return(1);
    else
        return(0);
}


krb5_error_code
ck_krb5_aname_to_localname(krb5_context context, krb5_const_principal aname,
                           const int lnsize, char *lname)
{
#ifdef COMMENT
    if ( p_k95_k5_aname_to_localname ) {
        return(p_k95_k5_aname_to_localname());
    } else
#endif
        if ( p_krb5_aname_to_localname )
        return(p_krb5_aname_to_localname(context,aname,lnsize,lname));
    else {
        char * name = NULL;
        if (krb5_unparse_name( context,
                               aname,
                               &name))
            name = 0;

        if ( name ) {
            ckstrncpy(lname,name,lnsize);

            krb5_free_unparsed_name(context,name);
            return(0);
        }
        else
            return(KRB5_LNAME_NOTRANS);
    }
}



/*
app_name - the name of the application
realm - The realm for this application
optionname - the name of the option
default - the default value if it isn't found.
variable - a pointer to the returned value

the way this appears in the configuration file is:

[appdefaults]

        option = false

        REALM = {
                option = false
                app_name = {
                        option = true
                }
        }

        app_name = {
                option = true
        }


For telnet, the app name I use is "telnet".  The realm is the realm of the
default principal.

The options I use in the Unix telnet are:

        forward - Forward your credentials if TRUE
        forwardable - Make the credentials on the remote end forwardable if TRUE
        encrypt - Attempt to negotiate encryption if TRUE
        autologin - Attempt to perform AUTHENTICATION option if true
        forceencrypt - Fail if encryption does not succeed

  ftp is "ftp" and only 'forward' is defined.

*/
void
ck_krb5_appdefault_boolean( krb5_context context, const char * app_name,
                            const krb5_data * realm, const char * optionname,
                            int def, int * variable)
{
    if ( p_krb5_appdefault_boolean )
        p_krb5_appdefault_boolean( context,app_name,realm,
                                   optionname,def,variable );
    else
        *variable = def;
}

void
ck_krb5_appdefault_string( krb5_context context, const char * app_name,
                           const krb5_data * realm, const char * optionname,
                           char ** value)
{
    if ( p_krb5_appdefault_string )
        p_krb5_appdefault_string( context,app_name,realm,
                                   optionname,value );
}

void
ck_krb5_get_init_creds_opt_init(krb5_get_init_creds_opt *opt)
{
    if ( p_krb5_get_init_creds_opt_init )
        p_krb5_get_init_creds_opt_init(opt);
}

void
ck_krb5_get_init_creds_opt_set_tkt_life(krb5_get_init_creds_opt *opt,
                 krb5_deltat tkt_life)
{
    if ( p_krb5_get_init_creds_opt_set_tkt_life )
        p_krb5_get_init_creds_opt_set_tkt_life(opt,tkt_life);
}

void
ck_krb5_get_init_creds_opt_set_renew_life(krb5_get_init_creds_opt *opt,
        krb5_deltat renew_life)
{
    if ( p_krb5_get_init_creds_opt_set_renew_life )
        p_krb5_get_init_creds_opt_set_renew_life(opt,renew_life);
}

void
ck_krb5_get_init_creds_opt_set_forwardable(krb5_get_init_creds_opt *opt,
        int forwardable)
{
    if ( p_krb5_get_init_creds_opt_set_forwardable )
        p_krb5_get_init_creds_opt_set_forwardable(opt,forwardable);
}

void
ck_krb5_get_init_creds_opt_set_proxiable(krb5_get_init_creds_opt *opt,
        int proxiable)
{
    if ( p_krb5_get_init_creds_opt_set_proxiable )
        p_krb5_get_init_creds_opt_set_proxiable(opt,proxiable);
}

void
ck_krb5_get_init_creds_opt_set_etype_list(krb5_get_init_creds_opt *opt,
        krb5_enctype *etype_list,
        int etype_list_length)
{
    if ( p_krb5_get_init_creds_opt_set_etype_list)
        p_krb5_get_init_creds_opt_set_etype_list(opt,etype_list,etype_list_length);
}

void
ck_krb5_get_init_creds_opt_set_address_list(krb5_get_init_creds_opt *opt, krb5_address **addresses)
{
    if ( p_krb5_get_init_creds_opt_set_address_list )
        p_krb5_get_init_creds_opt_set_address_list(opt,addresses);
}

void
ck_krb5_get_init_creds_opt_set_preauth_list(krb5_get_init_creds_opt *opt,
        krb5_preauthtype *preauth_list,
        int preauth_list_length)
{
    if ( p_krb5_get_init_creds_opt_set_preauth_list )
        p_krb5_get_init_creds_opt_set_preauth_list(opt,preauth_list,preauth_list_length);
}

void
ck_krb5_get_init_creds_opt_set_salt(krb5_get_init_creds_opt *opt, krb5_data *salt)
{
    if ( p_krb5_get_init_creds_opt_set_salt )
        p_krb5_get_init_creds_opt_set_salt(opt,salt);
}

krb5_error_code
ck_krb5_get_init_creds_password(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        char *password,
        krb5_prompter_fct prompter,
        void *data,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options)
{
    if ( p_krb5_get_init_creds_password )
        return(p_krb5_get_init_creds_password(context,creds,client,password,
                                               prompter,data,start_time,
                                               in_tkt_service,options));
    else
     return(-1);
}

krb5_error_code
ck_krb5_get_init_creds_keytab(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_keytab arg_keytab,
        krb5_deltat start_time,
        char *in_tkt_service,
        krb5_get_init_creds_opt *options)
{
    if ( p_krb5_get_init_creds_keytab )
        return(p_krb5_get_init_creds_keytab(context,creds,client,
                                             arg_keytab,start_time,
                                             in_tkt_service,options));
    else
        return(-1);
}

krb5_error_code
ck_krb5_get_validated_creds(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service)
{
    if ( p_krb5_get_validated_creds )
        return(p_krb5_get_validated_creds(context,creds,client,ccache,in_tkt_service));
    else
        return(-1);
}

krb5_error_code
ck_krb5_get_renewed_creds(krb5_context context,
        krb5_creds *creds,
        krb5_principal client,
        krb5_ccache ccache,
        char *in_tkt_service)
{
    if ( p_krb5_get_renewed_creds )
        return(p_krb5_get_renewed_creds(context,creds,client,ccache,in_tkt_service));
    else
        return(-1);
}

krb5_error_code
ck_krb5_rd_safe( krb5_context context,
                 krb5_auth_context auth,
                 krb5_const krb5_data  * data1,
                 krb5_data  * data2,
                 krb5_replay_data  * replay)
{
    if ( p_krb5_rd_safe )
        return(p_krb5_rd_safe(context,auth,data1,data2,replay));
    else
        return(-1);

}

krb5_error_code
ck_krb5_mk_safe( krb5_context context,
                 krb5_auth_context auth,
                 krb5_const krb5_data  * data1,
                 krb5_data  * data2,
                 krb5_replay_data  * replay)
{
    if ( p_krb5_mk_safe )
        return(p_krb5_mk_safe( context, auth, data1, data2, replay));
    else
        return(-1);
}

krb5_error_code
ck_krb5_rd_priv( krb5_context context,
                 krb5_auth_context auth,
                 krb5_const krb5_data  * data1,
                 krb5_data  * data2,
                 krb5_replay_data  * replay)
{
    if ( p_krb5_rd_priv )
        return(p_krb5_rd_priv(context,auth,data1,data2,replay));
    else
        return(-1);

}

krb5_error_code
ck_krb5_mk_priv( krb5_context context,
                 krb5_auth_context auth,
                 krb5_const krb5_data  * data1,
                 krb5_data  * data2,
                 krb5_replay_data  * replay)
{
    if ( p_krb5_mk_priv )
        return(p_krb5_mk_priv( context, auth, data1, data2, replay));
    else
        return(-1);
}

krb5_error_code
ck_krb5_auth_con_setuseruserkey( krb5_context context,
                                 krb5_auth_context auth,
                                 krb5_keyblock * key)
{
    if ( p_krb5_auth_con_setuseruserkey )
        return(p_krb5_auth_con_setuseruserkey(context,auth,key));
    else
        return(-1);
}

krb5_error_code
ck_krb5_get_profile(krb5_context context, profile_t * profile)
{
    if ( p_krb5_get_profile )
        return(p_krb5_get_profile(context,profile));
    else
        return(-1);
}

long
ck_profile_get_relation_names(profile_t profile, const char **names, char ***ret_names)
{
    if ( p_profile_get_relation_names )
        return(p_profile_get_relation_names(profile,names,ret_names));
    else
        return(-1);
}

long
ck_profile_get_subsection_names(profile_t profile, const char **names, char ***ret_names)
{
    if ( p_profile_get_subsection_names )
        return(p_profile_get_subsection_names(profile,names,ret_names));
    else
        return(-1);
}

void ck_krb5_free_keyblock_contents(krb5_context context, krb5_keyblock * keyblock)
{
    if ( p_krb5_free_keyblock_contents )
        p_krb5_free_keyblock_contents(context,keyblock);
}

krb5_error_code ck_krb5_c_encrypt(krb5_context context, krb5_const krb5_keyblock *key,
        krb5_keyusage usage, krb5_const krb5_data *ivec,
        krb5_const krb5_data *input, krb5_enc_data *output)
{
    if ( p_krb5_c_encrypt )
        return(p_krb5_c_encrypt(context,key,usage,ivec,input,output));
    else
        return(-1);
}

krb5_error_code ck_krb5_c_decrypt(krb5_context context, krb5_const krb5_keyblock *key,
                                   krb5_keyusage usage, krb5_const krb5_data *ivec,
                                   krb5_const krb5_enc_data *input, krb5_data *output)
{
    if ( p_krb5_c_decrypt )
        return(p_krb5_c_decrypt(context,key,usage,ivec,input,output));
    else
        return(-1);
}

krb5_error_code ck_krb5_c_block_size(krb5_context context, krb5_enctype enctype,
                                      size_t *blocksize)
{
    if ( p_krb5_c_block_size )
        return(p_krb5_c_block_size(context,enctype,blocksize));
    else
        return(-1);
}

krb5_error_code ck_krb5_c_make_random_key(krb5_context context, krb5_enctype enctype,
                                           krb5_keyblock *random_key)
{
    if ( p_krb5_c_make_random_key )
        return(p_krb5_c_make_random_key(context,enctype,random_key));
    else
        return(-1);
}

krb5_error_code
ck_krb5_c_random_seed(krb5_context context, krb5_data *data)
{
    if ( p_krb5_c_random_seed )
        return(p_krb5_c_random_seed(context,data));
    else
        return(-1);
}

krb5_error_code 
ck_krb5_c_encrypt_length(krb5_context context, krb5_enctype enctype,
                          size_t inputlen, size_t *length)
{
    if ( p_krb5_c_encrypt_length )
        return(p_krb5_c_encrypt_length(context,enctype,inputlen,length));
    else
        return(-1);
}

krb5_error_code
ck_krb5_kt_default_name(krb5_context context, char * name, int len)
{
    krb5_error_code code;
    krb5_keytab kt;

    if ( p_krb5_kt_default_name ) {
        code = p_krb5_kt_default_name(context,name,len);
        return(code);
    } else {
        code = krb5_kt_default(context,&kt);
        if ( code )
            return(code);
        krb5_kt_get_name(context,kt,name,len);
        krb5_kt_close(context,kt);
        return(0);
    }
}

#ifndef krb5_x
#define krb5_x(ptr,args) ((ptr)?((*(ptr)) args):(abort(),1))
#define krb5_xc(ptr,args) ((ptr)?((*(ptr)) args):(abort(),(char*)0))
#endif

#ifndef NO_DEF_KRB5_CCACHE
typedef krb5_pointer    krb5_cc_cursor; /* cursor for sequential lookup */

typedef struct _krb5_ccache {
    krb5_magic magic;
    struct _krb5_cc_ops FAR *ops;
    krb5_pointer data;
} *krb5_ccache;

typedef struct _krb5_cc_ops {
    krb5_magic magic;
    char  *prefix;
    char  * (KRB5_CALLCONV *get_name) (krb5_context, krb5_ccache);
    krb5_error_code (KRB5_CALLCONV *resolve) (krb5_context, krb5_ccache  *,
                                            const char  *);
    krb5_error_code (KRB5_CALLCONV *gen_new) (krb5_context, krb5_ccache  *);
    krb5_error_code (KRB5_CALLCONV *init) (krb5_context, krb5_ccache,
                                            krb5_principal);
    krb5_error_code (KRB5_CALLCONV *destroy) (krb5_context, krb5_ccache);
    krb5_error_code (KRB5_CALLCONV *close) (krb5_context, krb5_ccache);
    krb5_error_code (KRB5_CALLCONV *store) (krb5_context, krb5_ccache,
                                            krb5_creds  *);
    krb5_error_code (KRB5_CALLCONV *retrieve) (krb5_context, krb5_ccache,
                                            krb5_flags, krb5_creds  *,
                                            krb5_creds  *);
    krb5_error_code (KRB5_CALLCONV *get_princ) (krb5_context, krb5_ccache,
                                            krb5_principal  *);
    krb5_error_code (KRB5_CALLCONV *get_first) (krb5_context, krb5_ccache,
                                            krb5_cc_cursor  *);
    krb5_error_code (KRB5_CALLCONV *get_next) (krb5_context, krb5_ccache,
                                            krb5_cc_cursor  *, krb5_creds  *);
    krb5_error_code (KRB5_CALLCONV *end_get) (krb5_context, krb5_ccache,
                                            krb5_cc_cursor  *);
    krb5_error_code (KRB5_CALLCONV *remove_cred) (krb5_context, krb5_ccache,
                                            krb5_flags, krb5_creds  *);
    krb5_error_code (KRB5_CALLCONV *set_flags) (krb5_context, krb5_ccache,
                                            krb5_flags);
} krb5_cc_ops;
#endif /* NO_DEF_KRB5_CCACHE */

char * ck_krb5_cc_get_name
    (krb5_context context, krb5_ccache cache)
{
    if ( p_krb5_cc_get_name )
        return(p_krb5_cc_get_name(context,cache));
    else
        return(krb5_xc((cache)->ops->get_name,(context, cache)));
}


krb5_error_code ck_krb5_cc_gen_new
    (krb5_context context, krb5_ccache *cache)
{
    if ( p_krb5_cc_gen_new )
        return(p_krb5_cc_gen_new(context,cache));
    else
        return(krb5_x ((*cache)->ops->gen_new,(context, cache)));
}


krb5_error_code ck_krb5_cc_initialize
    (krb5_context context, krb5_ccache cache,
                    krb5_principal principal)
{
    if ( p_krb5_cc_initialize )
        return(p_krb5_cc_initialize(context,cache,principal));
    else
        return(krb5_x ((cache)->ops->init,(context, cache, principal)));
}


krb5_error_code ck_krb5_cc_destroy
    (krb5_context context, krb5_ccache cache)
{
    if ( p_krb5_cc_destroy )
        return(p_krb5_cc_destroy(context,cache));
    else
        return(krb5_x ((cache)->ops->destroy,(context, cache)));
}


krb5_error_code ck_krb5_cc_close
    (krb5_context context, krb5_ccache cache)
{
    if ( p_krb5_cc_close )
        return(p_krb5_cc_close(context,cache));
    else
        return(krb5_x ((cache)->ops->close,(context, cache)));
}


krb5_error_code ck_krb5_cc_store_cred
    (krb5_context context, krb5_ccache cache,
                     krb5_creds *creds)
{
    if ( p_krb5_cc_store_cred )
        return(p_krb5_cc_store_cred(context,cache,creds));
    else
        return(krb5_x ((cache)->ops->store,(context, cache, creds)));
}


krb5_error_code ck_krb5_cc_retrieve_cred
    (krb5_context context, krb5_ccache cache,
      krb5_flags flags, krb5_creds *mcreds,
      krb5_creds *creds)
{
    if ( p_krb5_cc_retrieve_cred )
        return(p_krb5_cc_retrieve_cred(context,cache,flags,mcreds,creds));
    else
        return(krb5_x ((cache)->ops->retrieve,(context, cache, flags, mcreds, creds)));
}


krb5_error_code ck_krb5_cc_get_principal
    (krb5_context context, krb5_ccache cache,
      krb5_principal *principal)
{
    if ( p_krb5_cc_get_principal )
        return(p_krb5_cc_get_principal(context,cache,principal));
    else
        return(krb5_x ((cache)->ops->get_princ,(context, cache, principal)));
}


krb5_error_code ck_krb5_cc_start_seq_get
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor)
{
    if ( p_krb5_cc_start_seq_get )
        return(p_krb5_cc_start_seq_get(context,cache,cursor));
    else
        return(krb5_x ((cache)->ops->get_first,(context, cache, cursor)));
}


krb5_error_code ck_krb5_cc_next_cred
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor, krb5_creds *creds)
{
    if ( p_krb5_cc_next_cred )
        return(p_krb5_cc_next_cred(context,cache,cursor,creds));
    else
        return(krb5_x ((cache)->ops->get_next,(context, cache, cursor, creds)));
}


krb5_error_code ck_krb5_cc_end_seq_get
    (krb5_context context, krb5_ccache cache,
      krb5_cc_cursor *cursor)
{
    if ( p_krb5_cc_end_seq_get )
        return(p_krb5_cc_end_seq_get(context,cache,cursor));
    else
        return(krb5_x ((cache)->ops->end_get,(context, cache, cursor)));
}


krb5_error_code ck_krb5_cc_remove_cred
    (krb5_context context, krb5_ccache cache, krb5_flags flags,
      krb5_creds *creds)
{
    if ( p_krb5_cc_remove_cred )
        return(p_krb5_cc_remove_cred(context,cache,flags,creds));
    else
        return(krb5_x ((cache)->ops->remove_cred,(context, cache,flags, creds)));
}


krb5_error_code ck_krb5_cc_set_flags
    (krb5_context context, krb5_ccache cache, krb5_flags flags)
{
    if ( p_krb5_cc_set_flags )
        return(p_krb5_cc_set_flags(context,cache,flags));
    else
        return(krb5_x ((cache)->ops->set_flags,(context, cache, flags)));
}

const char * ck_krb5_cc_get_type
    (krb5_context context, krb5_ccache cache)
{
    if ( p_krb5_cc_get_type )
        return(p_krb5_cc_get_type(context,cache));
    else
        return(((cache)->ops->prefix));
}

#ifndef NO_DEF_KRB5_KT
typedef krb5_pointer krb5_kt_cursor;    /* XXX */

typedef struct old_krb5_keytab_entry_st {
    krb5_magic magic;
    krb5_principal principal;   /* principal of this key */
    krb5_timestamp timestamp;   /* time entry written to keytable */
    krb5_kvno vno;              /* key version number */
    krb5_keyblock key;          /* the secret key */
} old_krb5_keytab_entry;

typedef struct _old_krb5_kt {
    krb5_magic magic;
    struct _krb5_kt_ops FAR *ops;
    krb5_pointer data;
} *old_krb5_keytab;

typedef struct _krb5_kt_ops {
    krb5_magic magic;
    char *prefix;
    /* routines always present */
    krb5_error_code (KRB5_CALLCONV *resolve)
        (krb5_context,
                 krb5_const char *,
                 krb5_keytab *);
    krb5_error_code (KRB5_CALLCONV *get_name)
        (krb5_context,
                 krb5_keytab,
                 char *,
                 int);
    krb5_error_code (KRB5_CALLCONV *close)
        (krb5_context,
                 krb5_keytab);
    krb5_error_code (KRB5_CALLCONV *get)
        (krb5_context,
                 krb5_keytab,
                 krb5_principal,
                 krb5_kvno,
                 krb5_enctype,
                 krb5_keytab_entry *);
    krb5_error_code (KRB5_CALLCONV *start_seq_get)
        (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *);
    krb5_error_code (KRB5_CALLCONV *get_next)
        (krb5_context,
                          krb5_keytab,
                          krb5_keytab_entry *,
                          krb5_kt_cursor *);
    krb5_error_code (KRB5_CALLCONV *end_get)
        (krb5_context,
                 krb5_keytab,
                 krb5_kt_cursor *);
    /* routines to be included on extended version (write routines) */
    krb5_error_code (KRB5_CALLCONV *add)
        (krb5_context,
                 krb5_keytab,
                 krb5_keytab_entry *);
    krb5_error_code (KRB5_CALLCONV *remove)
        (krb5_context,
                 krb5_keytab,
                  krb5_keytab_entry *);

    /* Handle for serializer */
    void * serializer;
} krb5_kt_ops;
#else
#define old_krb5_keytab krb5_keytab
#endif /* NO_DEF_KRB5_KT */

const char * ck_krb5_kt_get_type
     (krb5_context context, krb5_keytab keytab)
{
    if ( p_krb5_kt_get_type )
        return(p_krb5_kt_get_type(context,keytab));
    else
        return(((old_krb5_keytab)keytab)->ops->prefix);
}

krb5_error_code ck_krb5_kt_get_name
     (krb5_context context, krb5_keytab keytab, char * name, unsigned int namelen)
{
    if ( p_krb5_kt_get_name )
        return(p_krb5_kt_get_name(context,keytab,name,namelen));
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->get_name,(context, keytab,name,namelen)));
}

krb5_error_code ck_krb5_kt_close(krb5_context context, krb5_keytab keytab)
{
    if ( p_krb5_kt_close )
        return(p_krb5_kt_close(context,keytab));
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->close,(context, keytab)));
}

krb5_error_code ck_krb5_kt_get_entry(krb5_context context, krb5_keytab keytab,
                 krb5_principal princ,
                 krb5_kvno kvno,
                 krb5_enctype enc,
                 krb5_keytab_entry * entry)
{
    if ( p_krb5_kt_get_entry )
        return(p_krb5_kt_get_entry(context,keytab,princ,kvno,enc,entry));
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->get,(context, keytab, princ, kvno, enc, entry)));
}

krb5_error_code ck_krb5_kt_start_seq_get(krb5_context context,
                 krb5_keytab keytab,
                 krb5_kt_cursor * cursor)
{
    if ( p_krb5_kt_start_seq_get )
        return(p_krb5_kt_start_seq_get(context,keytab,cursor));
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->start_seq_get,(context, keytab, cursor)));
}
krb5_error_code ck_krb5_kt_next_entry(krb5_context context,
                 krb5_keytab keytab,
                 krb5_keytab_entry * entry,
                 krb5_kt_cursor * cursor)
{
    if ( p_krb5_kt_next_entry )
        return(p_krb5_kt_next_entry(context,keytab,entry,cursor) );
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->get_next,(context, keytab, entry, cursor)));
}

krb5_error_code ck_krb5_kt_end_seq_get(krb5_context context,
                 krb5_keytab keytab,
                 krb5_kt_cursor * cursor)
{
    if ( p_krb5_kt_end_seq_get )
        return(p_krb5_kt_end_seq_get(context,keytab,cursor));
    else
        return(krb5_x(((old_krb5_keytab)keytab)->ops->end_get,(context, keytab, cursor)));
}

krb5_error_code ck_krb5_build_principal(krb5_context context,
          krb5_principal * princ, unsigned int len, krb5_const char * data1,
          krb5_const char * data2, krb5_const char * data3, krb5_const char *data4)
{
    if ( p_krb5_build_principal )
        return(p_krb5_build_principal(context,princ,len,data1,data2,data3,data4));
    else
        return(-1);
}

int 
ck_krb524_init_ets(krb5_context context)
{
    if ( p_krb524_init_ets )
        return(p_krb524_init_ets(context));
    else
        return(-1);
}

int
ck_krb524_convert_creds_kdc(krb5_context context,
                             krb5_creds *v5creds,
                             LEASH_CREDENTIALS *v4creds)
{
    if ( p_krb524_convert_creds_kdc )
        return(p_krb524_convert_creds_kdc(context,v5creds,(CREDENTIALS *)v4creds));
    else
        return(-1);
}
#endif /* KRB5 */

#ifdef CK_DES
/* These functions are located in DLLs which we are not linking to at */
/* compile time.  Therefore, we have to redirect them to locally      */
/* defined functions which will call the real thing only if they are  */
/* available via Run-Time Linking.                                    */
/*
   From KRB5_32.DLL:
       unresolved external symbol __imp__des_ecb_encrypt@16
       unresolved external symbol __imp__des_new_random_key@4
       unresolved external symbol __imp__des_key_sched@8
       unresolved external symbol __imp__des_set_random_generator_seed@4

*/

static int
(KRB5_CALLCONV *p_des_new_random_key) P((Block))=NULL;
static void
(KRB5_CALLCONV *p_des_set_random_generator_seed) P((Block))=NULL;
static int
(KRB5_CALLCONV *p_des_key_sched) P((Block, Schedule))=NULL;
static void
(KRB5_CALLCONV *p_des_ecb_encrypt) P((Block, Block, Schedule, int))=NULL;
static void
(KRB5_CALLCONV *p_des_pcbc_encrypt)
P((Block, Block, long, Schedule, Block, int))=NULL;
static int
(KRB5_CALLCONV *p_des_string_to_key) P((char *, Block))=NULL;
static void
(KRB5_CALLCONV *p_des_fixup_key_parity) P((Block))=NULL;

/* these were KRB5_CALLCONV */
static int
(KRB5_CALLCONV_C  *p_k4_des_new_random_key) P((Block))=NULL;
static void
(KRB5_CALLCONV_C  *p_k4_des_set_random_generator_seed) P((Block))=NULL;
static int
(KRB5_CALLCONV_C  *p_k4_des_key_sched) P((Block, Schedule))=NULL;
static void
(KRB5_CALLCONV_C  *p_k4_des_ecb_encrypt) P((Block, Block, Schedule, int))=NULL;
static void
(KRB5_CALLCONV_C  *p_k4_des_pcbc_encrypt) P((Block, Block, long, Schedule, Block, int))=NULL;
static int
(KRB5_CALLCONV_C  *p_k4_des_string_to_key) P((char *, Block))=NULL;
static void
(KRB5_CALLCONV_C  *p_k4_des_fixup_key_parity) P((Block))=NULL;

#ifdef LIBDES
#ifdef CRYPT_DLL
int   (*libdes_random_key)(Block)=NULL;
void  (*libdes_random_seed)(Block)=NULL;
int   (*libdes_key_sched)(Block, Schedule)=NULL;
void  (*libdes_ecb_encrypt)(Block, Block, Schedule, int)=NULL;
int   (*libdes_string_to_key)(char *, Block)=NULL;
int   (*libdes_fixup_key_parity)(Block)=NULL;
int   (*libdes_pcbc_encrypt)(Block, Block, long, Schedule, Block, int)=NULL;
#else /* CRYPT_DLL */
int   libdes_random_key(Block);
void  libdes_random_seed(Block);
int   libdes_key_sched(Block, Schedule);
void  libdes_ecb_encrypt(Block, Block, Schedule, int);
int   libdes_string_to_key(char *, Block);
int   libdes_fixup_key_parity(Block);
int   libdes_pcbc_encrypt(Block, Block, long, Schedule, Block, int);
#endif /* CRYPT_DLL */
#endif /* LIBDES */

int
ck_des_new_random_key(Block B)
{
    int rc=0;
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_random_key ) {
        rc = libdes_random_key(B);
        return(rc);
    }
    else
        return(-1);
#else /* CRYPT_DLL */
    rc = libdes_random_key(B);
    return(rc);
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_new_random_key == NULL )
            return(-1);
        else {
            rc = p_des_new_random_key(B);
            return(rc);
        }
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_new_random_key == NULL )
            return(-1);
        else {
            rc = p_k4_des_new_random_key(B);
            return(rc);
        }
    }
    return(-1);
}
void
ck_des_set_random_generator_seed(Block B)
{
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_random_seed ) {
        libdes_random_seed(B);
        return;
    }
#else /* CRYPT_DLL */
    libdes_random_seed(B);
    return;
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_set_random_generator_seed )
            p_des_set_random_generator_seed(B);
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_set_random_generator_seed )
            p_k4_des_set_random_generator_seed(B);
    }
}
int
ck_des_key_sched(Block B, Schedule S)
{
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_key_sched ) {
        return libdes_key_sched(B,S);
    }
    else
        return(-3);
#else /* CRYPT_DLL */
    return libdes_key_sched(B,S);
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_key_sched )
            return p_des_key_sched(B,S);
        else
            return(-3);
    }
    else if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_key_sched )
            return p_k4_des_key_sched(B,S);
        else
            return(-3);
    }
    return(-3);
}

int
ck_k4_des_key_sched(Block B, Schedule S)
{
    if ( p_k4_des_key_sched )
        return p_k4_des_key_sched(B,S);
    else
        return(-3);
}

void
ck_des_ecb_encrypt(Block B1, Block B2, Schedule S, int I)
{
    if ( I ) {      /* Encrypting */
        ckhexdump("des_ecb_encrypt() encrypting",B1,sizeof(Block));
    } else {        /* Decrypting */
        ckhexdump("des_ecb_encrypt() decrypting",B1,sizeof(Block));
    }

#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_ecb_encrypt ) {
        libdes_ecb_encrypt(B1,B2,S,I);
        goto exit_des_ecb_encrypt;
    }
#else /* CRYPT_DLL */
    libdes_ecb_encrypt(B1,B2,S,I);
    goto exit_des_ecb_encrypt;
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_ecb_encrypt )
            p_des_ecb_encrypt(B1,B2,S,I);
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_ecb_encrypt )
            p_k4_des_ecb_encrypt(B1,B2,S,I);
    }
  exit_des_ecb_encrypt:
    ckhexdump("des_ecb_encrypt() B1",B1,sizeof(Block));
    ckhexdump("des_ecb_encrypt() B2",B2,sizeof(Block));
}

int
ck_des_string_to_key(char * p, Block B)
{
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_string_to_key )
        return(libdes_string_to_key(p,B));
    else
        return(0);
#else /* CRYPT_DLL */
    return(libdes_string_to_key(p,B));
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_string_to_key )
            return(p_des_string_to_key(p,B));
        else
            return(-1);
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_string_to_key )
            return(p_k4_des_string_to_key(p,B));
        else
            return(-1);
    }
}

void
ck_des_fixup_key_parity(Block B)
{
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_fixup_key_parity ) {
        libdes_fixup_key_parity(B);
        return;
    }
#else /* CRYPT_DLL */
    libdes_fixup_key_parity(B);
    return;
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_fixup_key_parity )
            p_des_fixup_key_parity(B);
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_fixup_key_parity )
            p_k4_des_fixup_key_parity(B);
    }
}

void
ck_des_pcbc_encrypt(Block input, Block output, long length,
                    Schedule schedule, Block ivec, int enc)
{
#ifdef LIBDES
#ifdef CRYPT_DLL
    if ( libdes_pcbc_encrypt ) {
        libdes_pcbc_encrypt(input,output,length,schedule,ivec,enc);
        return;
    }
#else /* CRYPT_DLL */
    libdes_pcbc_encrypt(input,output,length,schedule,ivec,enc);
    return;
#endif /* CRYPT_DLL */
#endif /* LIBDES */
    if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
        if ( p_des_pcbc_encrypt ) {
            p_des_pcbc_encrypt(input,output,length,schedule,ivec,enc);
            return;
        }
    }
    if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
        if ( p_k4_des_pcbc_encrypt ) {
            p_k4_des_pcbc_encrypt(input,output,length,schedule,ivec,enc);
            return;
        }
    }
}
#endif /* CK_DES */

#ifdef CRYPT_DLL
static int  (*p_crypt_dll_init)(struct _crypt_dll_init *)=NULL;
static int  (*p_encrypt_parse)(unsigned char *, int)=NULL;
static void (*p_encrypt_init)(kstream,int)=NULL;
static int  (*p_encrypt_session_key)(Session_Key *, int)=NULL;
static int  (*p_encrypt_dont_support)(int)=NULL;
static void (*p_encrypt_send_request_start)(void)=NULL;
static int  (*p_encrypt_request_start)(void)=NULL;
static int  (*p_encrypt_send_request_end)(void)=NULL;
static void (*p_encrypt_send_end)(void)=NULL;
static void (*p_encrypt_send_support)(void)=NULL;
static int  (*p_encrypt_is_encrypting)(void)=NULL;
static int  (*p_encrypt_is_decrypting)(void)=NULL;
static int  (*p_get_crypt_table)(struct keytab ** pTable, int * pN)=NULL;
static int  (*p_des_is_weak_key)(Block)=NULL;
static char * (*p_crypt_dll_version)()=NULL;

int
ck_encrypt_parse(unsigned char * s, int n)
{
    if ( p_encrypt_parse )
        return(p_encrypt_parse(s,n));
    else
        return(0);
}

void
ck_encrypt_init(kstream ks, int type)
{
    if ( p_encrypt_init )
        p_encrypt_init(ks,type);
}

int
ck_encrypt_dont_support(int type)
{
    if ( p_encrypt_dont_support )
        return p_encrypt_dont_support(type);
    return(0);
}

int
ck_encrypt_session_key(Session_Key * key, int n)
{
    if ( p_encrypt_session_key )
        return p_encrypt_session_key(key,n);
    return(0);
}

void
ck_encrypt_send_support(void)
{
    if ( p_encrypt_send_support )
        p_encrypt_send_support();
}

void
ck_encrypt_send_request_start(void)
{
    if ( p_encrypt_send_request_start )
        p_encrypt_send_request_start();
}

void
ck_encrypt_request_start(void)
{
    if ( p_encrypt_request_start )
        p_encrypt_request_start();
}

void
ck_encrypt_send_request_end(void)
{
    if ( p_encrypt_send_request_end )
        p_encrypt_send_request_end();
}

void
ck_encrypt_send_end(void)
{
    if ( p_encrypt_send_end )
        p_encrypt_send_end();
}

int
ck_encrypt_is_encrypting(void)
{
    if ( p_encrypt_is_encrypting )
        return(p_encrypt_is_encrypting());
    else
        return(0);
}

int
ck_encrypt_is_decrypting(void)
{
    if ( p_encrypt_is_decrypting )
        return(p_encrypt_is_decrypting());
    else
        return(0);
}

int
ck_get_crypt_table(struct keytab ** pTable, int * pN)
{
    if (p_get_crypt_table) {
        return(p_get_crypt_table(pTable,pN));
    } else {
        int i = 0;
        if (*pTable) {
            for (i = 0; i < *pN; i++)
              free((*pTable)[i].kwd);
            free (*pTable);
        }
        *pTable = NULL;
        *pN = 0;

        *pTable = malloc( sizeof(struct keytab) * 2);
        if (!(*pTable))
          return(0);
        (*pTable)[0].kwd = strdup("automatic");
        (*pTable)[0].kwval = ENCTYPE_ANY;
        (*pTable)[0].flgs = 0;
        (*pTable)[1].kwd = strdup("none");
        (*pTable)[1].kwval = -1;
        (*pTable)[1].flgs = 0;
        (*pN) = 2;
        return(2);
    }
}

int
ck_des_is_weak_key(Block B)
{
    if ( p_des_is_weak_key )
        return(p_des_is_weak_key(B));
    else
        return(0);      /* assume it is not */
}

void
crypt_install_funcs(char * name, void * func)
{
#ifdef NT
    if ( !strcmp(name,"encrypt_parse") )
        (FARPROC) p_encrypt_parse = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_init") )
        (FARPROC) p_encrypt_init = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_session_key") )
        (FARPROC) p_encrypt_session_key = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_dont_support") )
        (FARPROC) p_encrypt_dont_support = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_send_request_start") )
        (FARPROC) p_encrypt_send_request_start = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_request_start") )
        (FARPROC) p_encrypt_request_start = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_send_request_end") )
        (FARPROC) p_encrypt_send_request_end = (FARPROC) func;
    else if ( !strcmp(name, "encrypt_send_end") )
        (FARPROC) p_encrypt_send_end = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_send_support") )
        (FARPROC) p_encrypt_send_support = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_is_encrypting") )
        (FARPROC) p_encrypt_is_encrypting = (FARPROC) func;
    else if ( !strcmp(name,"encrypt_is_decrypting") )
        (FARPROC) p_encrypt_is_decrypting = (FARPROC) func;
    else if ( !strcmp(name,"get_crypt_table") )
        (FARPROC) p_get_crypt_table = (FARPROC) func;
    else if ( !strcmp(name,"des_is_weak_key") )
        (FARPROC) p_des_is_weak_key = (FARPROC) func;
    else if ( !strcmp(name,"libdes_random_key") )
        (FARPROC) libdes_random_key = (FARPROC) func;
    else if ( !strcmp(name,"libdes_random_seed") )
        (FARPROC) libdes_random_seed = (FARPROC) func;
    else if ( !strcmp(name,"libdes_key_sched") )
        (FARPROC) libdes_key_sched = (FARPROC) func;
    else if ( !strcmp(name,"libdes_ecb_encrypt") )
        (FARPROC) libdes_ecb_encrypt = (FARPROC) func;
    else if ( !strcmp(name,"libdes_string_to_key") )
        (FARPROC) libdes_string_to_key = (FARPROC) func;
    else if ( !strcmp(name,"libdes_fixup_key_parity") )
        (FARPROC) libdes_fixup_key_parity = (FARPROC) func;
    else if ( !strcmp(name,"libdes_pcbc_encrypt") )
        (FARPROC) libdes_pcbc_encrypt = (FARPROC) func;
    else if ( !strcmp(name,"crypt_dll_version") )
        (FARPROC) p_crypt_dll_version = (FARPROC) func;
#else /* NT */
    if ( !strcmp(name,"encrypt_parse") )
        p_encrypt_parse = (PFN*) func;
    else if ( !strcmp(name,"encrypt_init") )
        p_encrypt_init = (PFN*) func;
    else if ( !strcmp(name,"encrypt_session_key") )
        p_encrypt_session_key = (PFN*) func;
    else if ( !strcmp(name,"encrypt_dont_support") )
        p_encrypt_dont_support = (PFN*) func;
    else if ( !strcmp(name,"encrypt_send_request_start") )
        p_encrypt_send_request_start = (PFN*) func;
    else if ( !strcmp(name,"encrypt_request_start") )
        p_encrypt_request_start = (PFN*) func;
    else if ( !strcmp(name,"encrypt_send_request_end") )
        p_encrypt_send_request_end = (PFN*) func;
    else if ( !strcmp(name, "encrypt_send_end") )
        p_encrypt_send_end = (PFN*) func;
    else if ( !strcmp(name,"encrypt_send_support") )
        p_encrypt_send_support = (PFN*) func;
    else if ( !strcmp(name,"encrypt_is_encrypting") )
        p_encrypt_is_encrypting = (PFN*) func;
    else if ( !strcmp(name,"encrypt_is_decrypting") )
        p_encrypt_is_decrypting = (PFN*) func;
    else if ( !strcmp(name,"get_crypt_table") )
        p_get_crypt_table = (PFN*) func;
    else if ( !strcmp(name,"des_is_weak_key") )
        p_des_is_weak_key = (PFN*) func;
    else if ( !strcmp(name,"libdes_random_key") )
        libdes_random_key = (PFN*) func;
    else if ( !strcmp(name,"libdes_random_seed") )
        libdes_random_seed = (PFN*) func;
    else if ( !strcmp(name,"libdes_key_sched") )
        libdes_key_sched = (PFN*) func;
    else if ( !strcmp(name,"libdes_ecb_encrypt") )
        libdes_ecb_encrypt = (PFN*) func;
    else if ( !strcmp(name,"libdes_string_to_key") )
        libdes_string_to_key = (PFN*) func;
    else if ( !strcmp(name,"libdes_fixup_key_parity") )
        libdes_fixup_key_parity = (PFN*) func;
    else if ( !strcmp(name,"libdes_pcbc_encrypt") )
        libdes_pcbc_encrypt = (PFN*) func;
    else if ( !strcmp(name,"crypt_dll_version") )
        p_crypt_dll_version = (PFN*) func;
#endif /* NT */
}

char *
ck_crypt_dll_version()
{
    if ( p_crypt_dll_version )
        return(p_crypt_dll_version());
    else
        return("Encryption provided via external DLL");
}

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

#ifdef NT
HINSTANCE hCRYPT = NULL;
#else /* NT */
HMODULE hCRYPT = 0;
#endif /* NT */

static void
ck_crypt_dll_loaddll_eh(void)
{
    if ( hCRYPT ) {
#ifdef NT
        FreeLibrary(hCRYPT);
        hCRYPT = NULL;
#else /* NT */
        DosFreeModule(hCRYPT);
        hCRYPT = 0;
#endif /* NT */
    }
    p_crypt_dll_init=NULL;
    p_encrypt_parse=NULL;
    p_encrypt_init=NULL;
    p_encrypt_session_key=NULL;
    p_encrypt_dont_support=NULL;
    p_encrypt_send_support=NULL;
    p_encrypt_send_request_start=NULL;
    p_encrypt_request_start=NULL;
    p_encrypt_send_request_end=NULL;
    p_encrypt_send_end=NULL;
    p_encrypt_is_encrypting=NULL;
    p_encrypt_is_decrypting=NULL;
    p_get_crypt_table = NULL;
    p_des_is_weak_key = NULL;

    libdes_random_key=NULL;
    libdes_random_seed=NULL;
    libdes_key_sched=NULL;
    libdes_ecb_encrypt=NULL;
    libdes_string_to_key=NULL;
    libdes_fixup_key_parity=NULL;
    libdes_pcbc_encrypt=NULL;

    p_crypt_dll_version=NULL;

    encrypt_flag = 0;
}

static int crypt_dll_loaded=0;

int
ck_crypt_loaddll( void )
{
    ULONG rc = 0 ;
    struct _crypt_dll_init init;
    extern unsigned long startflags;
    int load_error = 0, len;
#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "K2CRYPT";
#endif /* OS2ONLY */

    if ( crypt_dll_loaded )
        return(1);

    if ( startflags & 8 )       /* do not load if we are not supposed to */
        return(0);

#ifdef NT
    hCRYPT = LoadLibrary("K95CRYPT");
    if ( !hCRYPT ) {
        rc = GetLastError() ;
        debug(F101, "K95 Crypt LoadLibrary failed","",rc) ;
        return(0);
    } else {
        if (((FARPROC) p_crypt_dll_init =
              GetProcAddress( hCRYPT, "crypt_dll_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"K95 Crypt GetProcAddress failed","crypt_dll_init",rc);
            load_error = 1;
        }
        if ( load_error ) {
            ck_crypt_dll_loaddll_eh();
            return 0;
        }
    }
#else /* NT */
    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,dllname);         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hCRYPT);
    if (rc) {
        debug(F111, "K95 Crypt LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hCRYPT);
    }
    if ( rc ) {
        debug(F111, "K95 Crypt LoadLibrary failed",fail,rc) ;
        return(0);
    } else {
        debug(F111, "K95 Crypt LoadLibrary success",fail,rc) ;
        if (rc = DosQueryProcAddr(hCRYPT,0,"crypt_dll_init",(PFN*)&p_crypt_dll_init))
        {
            debug(F111,"K95 Crypt GetProcAddress failed","crypt_dll_init",rc);
            load_error = 1;
            ck_crypt_dll_loaddll_eh();
            return 0;
        }
    }
#endif /* NT */

    init.version = 6;
    /* Version 1 */
    init.p_ttol = ttol;
#ifndef NODEBUG
    init.p_dodebug = dodebug;
    init.p_dohexdump = dohexdump;
#else /* NODEBUG */
    init.p_dodebug = NULL;
    init.p_dohexdump = NULL;
#endif /* NODEBUG */
    init.p_tn_debug = tn_debug;
    init.p_vscrnprintf = Vscrnprintf;
    /* Version 2 */
#ifdef KRB5
    init.p_k5_context = &k5_context;
#else /* KRB5 */
    init.p_k5_context = NULL;
#endif /* KRB5 */
    /* Version 3 */
    init.p_install_funcs = crypt_install_funcs;
    /* Version 5 */
    init.p_reqtelmutex = RequestTelnetMutex;
    init.p_reltelmutex = ReleaseTelnetMutex;
    /* Version 6 - adds p_encrypt_dont_support */

    if (!p_crypt_dll_init) {
        debug(F110,"K95 Crypt initialization failed",
              "p_crypt_dll_init is NULL",0);
        ck_crypt_dll_loaddll_eh();
        return(0);
    }

    debug(F100,"Calling p_crypt_dll_init()","",0);
    if ( !p_crypt_dll_init(&init) ) {
        debug(F110,"K95 Crypt initialization failed",
              "error in crypt init",0);
        ck_crypt_dll_loaddll_eh();
        return(0);
    }

    if ( p_crypt_dll_init==NULL ||
         p_encrypt_parse==NULL ||
         p_encrypt_init==NULL ||
         p_encrypt_session_key==NULL ||
         p_encrypt_dont_support==NULL ||
         p_encrypt_send_support==NULL ||
         p_encrypt_send_request_start==NULL ||
         p_encrypt_request_start==NULL ||
         p_encrypt_send_request_end==NULL ||
         p_encrypt_send_end==NULL ||
         p_encrypt_is_encrypting==NULL ||
         p_encrypt_is_decrypting==NULL ||
         p_get_crypt_table==NULL ||
         p_des_is_weak_key==NULL ||

         libdes_random_key==NULL ||
         libdes_random_seed==NULL ||
         libdes_key_sched==NULL ||
         libdes_ecb_encrypt==NULL ||
         libdes_string_to_key==NULL ||
         libdes_fixup_key_parity==NULL ||
         libdes_pcbc_encrypt==NULL
         /* p_crypt_dll_version is an optional function */
         )
    {
        debug(F110,"K95 Crypt initialization failed",
               "not all functions loaded",0);
        ck_crypt_dll_loaddll_eh();
        return(0);
    }

    encrypt_flag = 1;
    crypt_dll_loaded = 1;
    if ( deblog )
        printf("Encryption DLL Loaded\n");
    return(1);
}

int
ck_crypt_unloaddll( void )
{
    if ( !crypt_dll_loaded )
        return(1);

    /* unload dlls */
    ck_crypt_dll_loaddll_eh();

    /* success */
    crypt_dll_loaded = 0;
    return(1);
}
#endif /* CRYPT_DLL */

static int security_dll_loaded = 0;

#ifdef NTLM
#ifdef NT
/* Where did I get these? */
#define NTLMSSP_REQUEST_INIT_RESPONSE       0x100000    /* get back session keys  */
#define NTLMSSP_REQUEST_ACCEPT_RESPONSE     0x200000    /* get back session key, LUID */
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY  0x400000    /* request non-nt session keys */

HINSTANCE hSSPI = NULL;
#else /* NT */
NTLM is not supported on non-windows platforms
#endif /* NT */

static int sspi_dll_loaded = 0;
static PSecurityFunctionTable (*p_SSPI_InitSecurityInterface)(void)=NULL;
static PSecurityFunctionTable p_SSPI_Func = NULL;
CredHandle hNTLMCred = { 0, 0 };
static TimeStamp  NTLMTimeStampCred;
static CtxtHandle hNTLMContext;
int    haveNTLMContext = 0;
static SecBufferDesc NTLMSecBufDesc;
SecBuffer     NTLMSecBuf[1];
static UCHAR      NTLMBuffer[1024];
static UCHAR      NTLMBuffer2[1024];
static SecBufferDesc NTLMInSecBufDesc;
static SecBuffer     NTLMInSecBuf;
static UCHAR      NTLMInBuffer[512];
static ULONG      NTLMContextAttrib;
static TimeStamp  NTLMTimeStampContext;
static SECURITY_STATUS ss = SEC_E_OK;
static PBYTE ntlm_pClientBuf = NULL;
static PBYTE ntlm_pServerBuf = NULL;
static DWORD ntlm_cbMaxToken = 0;


static void
ck_sspi_loaddll_eh( void )
{
    if ( hSSPI ) {
        FreeLibrary(hSSPI);
        hSSPI = NULL;
    }

    p_SSPI_InitSecurityInterface = NULL;
    p_SSPI_Func = NULL;
}

static int
ck_ntlm_unloaddll( void )
{
    if ( !sspi_dll_loaded )
        return(1);

    /* unload dlls */
    ck_sspi_loaddll_eh();

    /* success */
    sspi_dll_loaded = 0;
    return(1);
}

const char *
sspi_errstr(SECURITY_STATUS ss)
{
    switch ( ss ) {
    case SEC_E_OK:
        return("OK");
    case SEC_E_INSUFFICIENT_MEMORY:
        return("Not enough memory is available to complete this request");
    case SEC_E_INVALID_HANDLE:
        return("The handle specified is invalid");
    case SEC_E_UNSUPPORTED_FUNCTION:
        return("The function requested is not supported");
    case SEC_E_TARGET_UNKNOWN:
        return("The specified target is unknown or unreachable");
    case SEC_E_INTERNAL_ERROR:
        return("The Local Security Authority cannot be contacted");
    case SEC_E_SECPKG_NOT_FOUND:
        return("The requested security package does not exist");
    case SEC_E_NOT_OWNER:
        return("The caller is not the owner of the desired credentials");
    case SEC_E_CANNOT_INSTALL:
        return("The security pakage failed to initialize, and cannot be installed");
    case SEC_E_INVALID_TOKEN:
        return("The token supplied to the function is invalid");
    case SEC_E_CANNOT_PACK:
        return("The security package is not ablle to marshall the logon buffer so the logon attempt has failed");
    case SEC_E_QOP_NOT_SUPPORTED:
        return("The per-message Quality of Protection is not supported by the security package");
    case SEC_E_NO_IMPERSONATION:
        return("The security context does not allow impersonation of the client");
    case SEC_E_LOGON_DENIED:
        return("The logon attempt failed");
    case SEC_E_UNKNOWN_CREDENTIALS:
        return("The credentials supplied to the package were not recognized");
    case SEC_E_NO_CREDENTIALS:
        return("No credentials are available in the security package");
    case SEC_E_MESSAGE_ALTERED:
        return("The message supplied for verification has been altered");
    case SEC_E_OUT_OF_SEQUENCE:
        return("The message supplied for verification is out of sequence");
    case SEC_E_NO_AUTHENTICATING_AUTHORITY:
        return("No authority could be contacted for authentication");
    case SEC_I_CONTINUE_NEEDED:
        return("The function completed successfully, but must be called again to complete the context");
    case SEC_I_COMPLETE_NEEDED:
        return("The function completed successfully, but CompleteToken must be called");
    case SEC_I_COMPLETE_AND_CONTINUE:
        return("The function completed successfully, but both CompleteToken and this function must be called again");
    case SEC_I_LOCAL_LOGON:
        return("The logon was completed, but no network authority was available.  The logon was made using locally known information");
    case SEC_E_BAD_PKGID:
        return("The requested security package does not exist");
    case SEC_E_CONTEXT_EXPIRED:
        return("The context has expired and can no longer be used");
    case SEC_E_INCOMPLETE_MESSAGE:
        return("The supplied message is incomplete.  The signature was not verified");
    case SEC_E_INCOMPLETE_CREDENTIALS:
        return("The credentials supplied were not complete, and could not be verified.  The context could not be initialized");
    case SEC_E_BUFFER_TOO_SMALL:
        return("The buffers supplied to the function were too small");
    case SEC_I_INCOMPLETE_CREDENTIALS:
        return("The credentials supplied were not complete, and could not be verified.  Additional information can be returned from the context");
    case SEC_I_RENEGOTIATE:
        return("The context data must be renegotiated with the user");
    case SEC_E_WRONG_PRINCIPAL:
        return("The context data must be renegotiated with the peer");
    default:
        return("Unknown security error");
    }
}

int
ck_sspi_loaddll( void )
{
    ULONG rc = 0 ;
    ULONG Packages;              /* receives the number of packages */
    PSecPkgInfo pPackageInfo;    /* receives array of information   */
    SECURITY_STATUS ss = SEC_E_NOT_SUPPORTED;
    int ntlm_found = 0;
    int load_error = 0;


    if ( isWin95() )
        hSSPI = LoadLibrary("SECUR32");
    else
        hSSPI = LoadLibrary("SECURITY") ;

    if ( !hSSPI ) {
        rc = GetLastError() ;
        debug(F101, "NTLM LoadLibrary failed","",rc) ;
        return(0);
    }

    /* when we define UNICODE we must load the Unicode version of this function */

    if (((FARPROC) p_SSPI_InitSecurityInterface =
          GetProcAddress( hSSPI, "InitSecurityInterfaceA" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "NTLM GetProcAddress failed",
               "InitSecurityInterfaceA",rc) ;
        load_error = 1;
    } else {
        p_SSPI_Func = p_SSPI_InitSecurityInterface();
    }

    if ( !p_SSPI_Func )
    {
        rc = GetLastError() ;
        debug(F101, "NTLM InitSecurityInterface() failed","",rc) ;
        load_error = 1;
    } else {
        ss = p_SSPI_Func->EnumerateSecurityPackages(&Packages,
                                            &pPackageInfo);
    }

    if ( ss != SEC_E_OK )
    {
        debug(F101, "NTLM EnumerateSecurityPackages() failed",
               sspi_errstr(ss),ss) ;
        load_error = 1;
    } else {
        debug(F101,"Microsoft Security Providers","",Packages);
        for (;Packages--;) {
            debug(F111,pPackageInfo[Packages].Name,
                   pPackageInfo[Packages].Comment,
                   Packages);
            if ( !strcmp(NTLMSP_NAME_A,pPackageInfo[Packages].Name) ) {
                ntlm_found = 1;
                ntlm_cbMaxToken = pPackageInfo[Packages].cbMaxToken;
            }
        }
        p_SSPI_Func->FreeContextBuffer(pPackageInfo);
    }

    if ( !ntlm_found || load_error ) {
        debug(F100, "NTLM not installed","",0) ;
        ck_sspi_loaddll_eh();
        return 0;
    }

    if ( deblog ) {
        if ( hSSPI )
            printf("NTLM available\n");
    }
    return(1);

}

int
ck_ntlm_is_valid(int query_user) {
    extern int sstelnet;
    extern char szUserName[];
    extern char pwbuf[];
    extern int  pwflg, pwcrypt;

    SEC_WINNT_AUTH_IDENTITY AuthIdentity;
    char prompt[128];
    char domain[128]="",name[128]="",pwd[128]="", *p;
    char localuser[128]="";
    DWORD dw;
    int len;

    if ( !p_SSPI_Func || (isWin95() && sstelnet) )
        return(0);

    p_SSPI_Func->FreeCredentialHandle( &hNTLMCred );

    if (!sstelnet) {
        len = strlen(localuser);
        dw = sizeof(localuser)-len;
        GetUserName(&localuser[len],&dw);

        len = strlen(szUserName);
        if (len > 0 && query_user) {
            p = szUserName + len - 1;
            while ( p > szUserName ) {
                if ( *p == '\\' || *p == '/' ) {
                    char q;
                    /* we found a domain */
                    ckstrncpy(name,p+1,sizeof(name));
                    q = *p;
                    *p = '\0';
                    ckstrncpy(domain,szUserName,sizeof(domain));
                    *p = q;
                }
                p--;
            }

            if (!name[0])
                ckstrncpy(name,szUserName,sizeof(name));

            if (pwbuf[0] && pwflg) {
                ckstrncpy(pwd,pwbuf,sizeof(pwd));
#ifdef OS2
                if ( pwcrypt )
                    ck_encrypt((char *)pwd);
#endif /* OS2 */
                if (!domain[0])
                    strcpy(domain,".");
            } else {
#ifdef KUI
                struct txtbox tb[3];
                int ok;

                tb[0].t_buf = domain;
                tb[0].t_len = sizeof(domain);
                tb[0].t_lbl = "Domain: ";
                tb[0].t_dflt = domain;
                tb[0].t_echo = 1;
                tb[1].t_buf = name;
                tb[1].t_len = sizeof(name);
                tb[1].t_lbl = "Username: ";
                tb[1].t_dflt = name;
                tb[1].t_echo = 1;
                tb[2].t_buf = pwd;
                tb[2].t_len = sizeof(pwd);
                tb[2].t_lbl = "Passphrase: ";
                tb[2].t_dflt = NULL;
                tb[2].t_echo = 2;

                ok = uq_mtxt("NTLM Authentication Required\n",
                              NULL, 3, tb);

#else /* KUI */
                if (!domain[0]) {
                    ckmakmsg(prompt,sizeof(prompt),name,"'s NTLM Domain: ",NULL,NULL);
                    readtext(prompt,domain,sizeof(domain));
                }

                if (domain[0]) {
                    ckmakmsg(prompt,sizeof(prompt),domain,"\\",name,"'s NTLM password: ");
                } else {
                    ckmakmsg(prompt,sizeof(prompt),name,"'s NTLM password: ",NULL,NULL);
                    strcpy(domain,".");
                }
                readpass(prompt,pwd,sizeof(pwd));
#endif /* KUI */
            }

            ZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );
            AuthIdentity.Domain = domain;
            AuthIdentity.DomainLength = lstrlen(domain);
            AuthIdentity.User = name;
            AuthIdentity.UserLength = lstrlen(name);
            AuthIdentity.Password = pwd;
            AuthIdentity.PasswordLength = lstrlen(pwd);
            AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
        }
    }

    ss = p_SSPI_Func->
        AcquireCredentialsHandle( NULL,
                                  NTLMSP_NAME_A,
                                  sstelnet?
                                    SECPKG_CRED_INBOUND:
                                    SECPKG_CRED_OUTBOUND,
                                  NULL,
                                  (!sstelnet && query_user && szUserName[0]) ?
                                  &AuthIdentity : NULL,
                                  NULL,
                                  NULL,
                                  (PCredHandle) &hNTLMCred,
                                  &NTLMTimeStampCred);
    debug(F111,"ck_ntlm_is_valid AcquireCredentialsHandle",sspi_errstr(ss),ss);
    return(ss == SEC_E_OK);
}

void
ntlm_reject(int how) {
    unsigned char buf[12];

    sprintf(buf, "%c%c%c%c%c%c%c%c%c",
             IAC, SB, TELOPT_AUTHENTICATION,
             TELQUAL_IS,
             authentication_version,
             how,
             NTLM_REJECT, IAC, SE);
    if (deblog || tn_deb || debses) {
        int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK);
        ckmakxmsg(tn_msg,TN_MSG_LEN,
                  "TELNET SENT SB ", TELOPT(TELOPT_AUTHENTICATION),
                   " IS ", AUTHTYPE_NAME(authentication_version),
                   " ", AUTHMODE_NAME(mode),
                   " NTLM_REJECT IAC SE",
                   NULL,NULL,NULL,NULL,NULL
                   );
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol((char *)buf, 9);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
}

int
ntlm_auth_send() {
    DWORD context_requirements = 0;

    if ( !p_SSPI_Func ) {
        debug(F110,"ntlm_auth_send() failed","NTLM not installed",0);
        return(-1);
    }

    //
    // Set up the Buffer Descriptor
    //
    NTLMSecBufDesc.ulVersion = SECBUFFER_VERSION;
    NTLMSecBufDesc.cBuffers = 1;
    NTLMSecBufDesc.pBuffers = NTLMSecBuf;

    NTLMSecBuf[0].cbBuffer = 1024;
    NTLMSecBuf[0].BufferType = SECBUFFER_TOKEN;
    NTLMSecBuf[0].pvBuffer = NTLMBuffer;

    p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
    memset(&hNTLMContext,0,sizeof(hNTLMContext));
    memset(&NTLMContextAttrib,0,sizeof(NTLMContextAttrib));
    memset(&NTLMTimeStampContext,0,sizeof(NTLMTimeStampContext));

    if ( !isWin95() ) {
        context_requirements =
#ifdef COMMENT
            ISC_REQ_USE_DCE_STYLE |
            ISC_REQ_DELEGATE |
            ISC_REQ_MUTUAL_AUTH |
            ISC_REQ_REPLAY_DETECT |
            ISC_REQ_SEQUENCE_DETECT |
            ISC_REQ_CONFIDENTIALITY |
            ISC_REQ_CONNECTION |
            ISC_REQ_USE_SESSION_KEY |
            ISC_REQ_EXTENDED_ERROR |
            NTLMSSP_REQUEST_INIT_RESPONSE |
            NTLMSSP_REQUEST_ACCEPT_RESPONSE |
            NTLMSSP_REQUEST_NON_NT_SESSION_KEY,
#else
            ISC_REQ_USE_DCE_STYLE |
            ISC_REQ_DELEGATE |
            ISC_REQ_MUTUAL_AUTH |
            ISC_REQ_REPLAY_DETECT |
            ISC_REQ_SEQUENCE_DETECT |
            ISC_REQ_CONFIDENTIALITY |
            ISC_REQ_CONNECTION |
            ISC_REQ_USE_SESSION_KEY |
            ISC_REQ_EXTENDED_ERROR
#endif
                ;
    }

    ss = p_SSPI_Func->InitializeSecurityContext(&hNTLMCred,
                0,
                NULL,
                context_requirements,
                0,
                SECURITY_NETWORK_DREP,
                0,
                0,
                &hNTLMContext,
                &NTLMSecBufDesc,
                &NTLMContextAttrib,
                &NTLMTimeStampContext);
    debug(F111,"ntlm_auth_send() InitializeSecurityContext",
           sspi_errstr(ss),ss);

    ckhexdump("ntlm_auth_send() InitializeSecurityContext Context Requirements",
           &context_requirements,sizeof(context_requirements));
    ckhexdump("ntlm_auth_send() InitializeSecurityContext Context Attributes",
           &NTLMContextAttrib,sizeof(NTLMContextAttrib));

    switch ( ss ) {
    case SEC_E_OK:
    case SEC_I_CONTINUE_NEEDED:
    case SEC_I_COMPLETE_AND_CONTINUE:
        break;
    default:
        printf("ISC: %s\n",sspi_errstr(ss));
        debug(F111,"ntlm_auth_send() failed",
               sspi_errstr(ss),ss);
        return(-1);
    }

    debug(F111,"ntlm_auth_send sending","NTLMSecBuf[0].cbBuffer",
           NTLMSecBuf[0].cbBuffer);
    debug(F111,"ntlm_auth_send sending","NTLMSecBuf[0].BufferType",
           NTLMSecBuf[0].BufferType);
    ckhexdump("ntlm_auth_send sending NTLMSecBuf[0].pvBuffer",
             NTLMSecBuf[0].pvBuffer,
             NTLMSecBuf[0].cbBuffer);

    /* The rest of the work is continued in auth_send() */
    return(0);
}

int
#ifdef CK_ANSIC
ntlm_reply(int how, unsigned char *data, int cnt)
#else
ntlm_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    DWORD context_requirements = 0;
    unsigned char buf[512], *p;
    int length = 0, iaccnt = 0, i;

    data += 4;                          /* Point to status byte */
    cnt  -= 4;

    if(cnt < 8  && ss != SEC_E_OK) {
        p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
        p_SSPI_Func->FreeCredentialHandle( &hNTLMCred );

        ntlm_reject(how);
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }

    if ( !isWin95() ) {
        context_requirements =
#ifdef COMMENT
                ISC_REQ_USE_DCE_STYLE |
                ISC_REQ_DELEGATE |
                ISC_REQ_MUTUAL_AUTH |
                ISC_REQ_REPLAY_DETECT |
                ISC_REQ_SEQUENCE_DETECT |
                ISC_REQ_CONFIDENTIALITY |
                ISC_REQ_CONNECTION |
                ISC_REQ_USE_SESSION_KEY |
                ISC_REQ_EXTENDED_ERROR |
                NTLMSSP_REQUEST_INIT_RESPONSE |
                NTLMSSP_REQUEST_ACCEPT_RESPONSE |
                NTLMSSP_REQUEST_NON_NT_SESSION_KEY
#else
                ISC_REQ_USE_DCE_STYLE |
                ISC_REQ_DELEGATE |
                ISC_REQ_MUTUAL_AUTH |
                ISC_REQ_REPLAY_DETECT |
                ISC_REQ_SEQUENCE_DETECT |
                ISC_REQ_CONFIDENTIALITY |
                ISC_REQ_CONNECTION |
                ISC_REQ_USE_SESSION_KEY |
                ISC_REQ_EXTENDED_ERROR
#endif
                    ;
    }

    cnt--;
    switch(*data++) {
    case 1:     /* CONTINUE */
        memcpy(buf,data,cnt);
        p = buf;

        NTLMInSecBufDesc.ulVersion = SECBUFFER_VERSION;
        NTLMInSecBufDesc.cBuffers = 1;
        NTLMInSecBufDesc.pBuffers = &NTLMInSecBuf;

        p = (unsigned char *)&NTLMInSecBuf.cbBuffer;
        p[0] = *data++;
        p[1] = *data++;
        p[2] = *data++;
        p[3] = *data++;

        p = (unsigned char *)&NTLMInSecBuf.BufferType;
        p[0] = *data++;
        p[1] = *data++;
        p[2] = *data++;
        p[3] = *data++;

        NTLMInSecBuf.pvBuffer = data;

        NTLMSecBufDesc.ulVersion = SECBUFFER_VERSION;
        NTLMSecBufDesc.cBuffers = 1;
        NTLMSecBufDesc.pBuffers = NTLMSecBuf;

        NTLMSecBuf[0].cbBuffer = 1024;
        NTLMSecBuf[0].BufferType = SECBUFFER_TOKEN;
        NTLMSecBuf[0].pvBuffer = NTLMBuffer;

        debug(F111,"ntlm_reply received","NTLMInSecBuf.cbBuffer",
               NTLMSecBuf[0].cbBuffer);
        debug(F111,"ntlm_reply received","NTLMInSecBuf.BufferType",
               NTLMSecBuf[0].BufferType);
        ckhexdump("ntlm_reply received NTLMInSecBuf[0].pvBuffer",
                 NTLMInSecBuf.pvBuffer,
                 NTLMInSecBuf.cbBuffer);


        ss = p_SSPI_Func->InitializeSecurityContext(0,
                &hNTLMContext,
                0,
                context_requirements,
                0,
                SECURITY_NETWORK_DREP,
                &NTLMInSecBufDesc,
                0,
                &hNTLMContext,
                &NTLMSecBufDesc,
                &NTLMContextAttrib,
                &NTLMTimeStampContext);
        debug(F111,"ntlm_reply InitializeSecurityContext",sspi_errstr(ss),ss);

        ckhexdump("ntlm_reply() InitializeSecurityContext Context Requirements",
                 &context_requirements,sizeof(context_requirements));
        ckhexdump("ntlm_reply() InitializeSecurityContext Context Attributes",
                 &NTLMContextAttrib,sizeof(NTLMContextAttrib));


        switch ( ss ) {
        case SEC_E_OK:
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
            break;
        default:
            debug(F111,"ntlm_auth_reply() failed",sspi_errstr(ss),ss);
            p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
            p_SSPI_Func->FreeCredentialHandle( &hNTLMCred );

            ntlm_reject(how);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }

        sprintf(buf, "%c%c%c%c%c%c%c",
                 IAC, SB, TELOPT_AUTHENTICATION,
                 TELQUAL_IS,
                 authentication_version,
                 how,
                 NTLM_RESPONSE);                        /* safe */

        debug(F111,"ntlm_reply sending","NTLMSecBuf[0].cbBuffer",
               NTLMSecBuf[0].cbBuffer);
        debug(F111,"ntlm_reply sending","NTLMSecBuf[0].BufferType",
               NTLMSecBuf[0].BufferType);
        ckhexdump("ntlm_reply sending NTLMSecBuf[0].pvBuffer",
                 NTLMSecBuf[0].pvBuffer,
                 NTLMSecBuf[0].cbBuffer);

        for ( i=0 ; i<NTLMSecBuf[0].cbBuffer ; i++ ) {
            if ( ((char *)NTLMSecBuf[0].pvBuffer)[i] == IAC )
                iaccnt++;
        }

        if ( NTLMSecBuf[0].cbBuffer + 2 * sizeof(ULONG) + 10 + iaccnt <
             sizeof(buf) ) {
            length = copy_for_net(&buf[7], (char *) &NTLMSecBuf[0],
                                   2*sizeof(ULONG));
            length += copy_for_net(&buf[7+length], NTLMSecBuf[0].pvBuffer,
                                    NTLMSecBuf[0].cbBuffer);
        }
        sprintf(&buf[7+length], "%c%c", IAC, SE);       /* safe */

        if (deblog || tn_deb || debses) {
            int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK);
            ckmakxmsg(tn_msg,TN_MSG_LEN,"TELNET SENT SB ",
                       TELOPT(TELOPT_AUTHENTICATION)," IS ",
                       AUTHTYPE_NAME(authentication_version)," ",
                       AUTHMODE_NAME(mode), " NTLM_RESPONSE ",
                       NULL,NULL,NULL,NULL,NULL );
            tn_hex(tn_msg,TN_MSG_LEN,&buf[7],length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((char *)buf, length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    case NTLM_ACCEPT:     /* ACCEPT */
        auth_finished(AUTH_VALID);
        accept_complete = 1;
        return AUTH_SUCCESS;
    case NTLM_REJECT:     /* REJECT */
        if (cnt > 0) {
            data[cnt] = '\0';
            ckmakmsg(strTmp,sizeof(strTmp),"NTLM refuses authentication (",
                     data,")\r\n",NULL);
        } else {
            sprintf(strTmp,"NTLM refuses authentication\r\n");
        }
        printf("NTLM authentication failed!\r\n%s\r\n",strTmp);

    default:
        auth_finished(AUTH_REJECT);
        accept_complete = 1;
        return AUTH_FAILURE;
        break;
    }

#ifdef COMMENT
    if ( ss == SEC_E_OK ) {

        SecPkgContext_Sizes sizes;
        SecPkgContext_Names names;
        SecPkgContext_Lifespan lifespan;
        SecPkgContext_DceInfo dce;
        SecPkgContext_StreamSizes stream;
        SecPkgContext_Authority authority;
        SecPkgContext_KeyInfo keyinfo;
        SecPkgContext_ProtoInfo proto;
        SecPkgContext_PasswordExpiry pwd;
        SecPkgContext_SessionKey skey;
        SecPkgContext_PackageInfo pkginf;

        /* The Server has now authenticated the Client's User */
        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_SIZES,&sizes);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_SIZES",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_NAMES,&names);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_NAMES",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_LIFESPAN,&lifespan);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_LIFESPAN",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_DCE_INFO,&dce);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_DCE_INFO",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_STREAM_SIZES,&stream);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_STREAM_SIZES",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_KEY_INFO,&keyinfo);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_KEY_INFO",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_AUTHORITY,&authority);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_AUTHORITY",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PROTO_INFO,&proto);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_PROTO_INFO",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PASSWORD_EXPIRY,&pwd);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_PASSWORD_EXPIRY",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_SESSION_KEY,&skey);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_SESSION_KEY",sspi_errstr(ss),ss);

        ss = p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PACKAGE_INFO,&pkginf);
        debug(F111,"SSPI QueryConextAttributes SECPKG_ATTR_PACKAGE_INFO",sspi_errstr(ss),ss);
        ss = SEC_E_OK;
    }
#endif /* COMMENT */

    return AUTH_SUCCESS;
}

int
ntlm_impersonate()
{
    if ( p_SSPI_Func ) {
        ss = p_SSPI_Func->ImpersonateSecurityContext( &hNTLMContext );
        debug(F111,"ntlm_impersonate() ImpersonateSecurityContext","ss",ss);
        return(ss == SEC_E_OK);
    }
    return(0);
}

int
ntlm_revert()
{
    if ( p_SSPI_Func ) {
        ss = p_SSPI_Func->RevertSecurityContext( &hNTLMContext );
        debug(F111,"ntlm_revert() RevertSecurityContext","ss",ss);
        return(ss == SEC_E_OK);
    }
    return(0);
}

int
#ifdef CK_ANSIC
ntlm_is(unsigned char *data, int cnt)
#else
ntlm_is(data,cnt) unsigned char *data; int cnt;
#endif
{
    DWORD context_requirements = 0;
    unsigned char buf[512], *p;
    int length = 0;
    int round = -1;
    int i, iaccnt = 0;

    data += 4;                          /* Point to status byte */
    cnt  -= 4;

    if (cnt < 8) {
        p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
        p_SSPI_Func->FreeCredentialHandle( &hNTLMCred );
        ntlm_reject(auth_how);
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }

    cnt--;
    round = *data++;
    debug(F111,"ntlm_is","round",round);
    switch(round) {
    case 0:
    case 2:
        NTLMInSecBufDesc.ulVersion = 0;
        NTLMInSecBufDesc.cBuffers = 1;
        NTLMInSecBufDesc.pBuffers = &NTLMInSecBuf;

        p = (unsigned char *)&NTLMInSecBuf.cbBuffer;
        p[0] = *data++;
        p[1] = *data++;
        p[2] = *data++;
        p[3] = *data++;
        cnt -= 4;
        debug(F111,"ntlm_is","NTLMInSecBuf.cbBuffer",NTLMInSecBuf.cbBuffer);

        p = (unsigned char *)&NTLMInSecBuf.BufferType;
        p[0] = *data++;
        p[1] = *data++;
        p[2] = *data++;
        p[3] = *data++;
        cnt -= 4;
        debug(F111,"ntlm_is","NTLMInSecBuf.BufferType",NTLMInSecBuf.BufferType);

        ckhexdump("ntlm_is received NTLMInSecBuf.pvBuffer",data,cnt);
        NTLMInSecBuf.pvBuffer = data;

        NTLMSecBufDesc.ulVersion = SECBUFFER_VERSION;
        NTLMSecBufDesc.cBuffers = 1;
        NTLMSecBufDesc.pBuffers = NTLMSecBuf;

        NTLMSecBuf[0].cbBuffer = 1024;
        NTLMSecBuf[0].BufferType = SECBUFFER_TOKEN;
        NTLMSecBuf[0].pvBuffer = NTLMBuffer;

        if ( !isWin95() ) {
            context_requirements =
#ifdef COMMENT
                                   ASC_REQ_USE_DCE_STYLE |
                                   ASC_REQ_DELEGATE |
                                   ASC_REQ_MUTUAL_AUTH |
                                   ASC_REQ_REPLAY_DETECT |
                                   ASC_REQ_SEQUENCE_DETECT |
                                   ASC_REQ_CONFIDENTIALITY |
                                   ASC_REQ_CONNECTION |
                                   ASC_REQ_USE_SESSION_KEY |
                                   ASC_REQ_EXTENDED_ERROR |
                                   NTLMSSP_REQUEST_INIT_RESPONSE |
                                   NTLMSSP_REQUEST_ACCEPT_RESPONSE |
                                   NTLMSSP_REQUEST_NON_NT_SESSION_KEY
#else
                                   ASC_REQ_USE_DCE_STYLE |
                                   ASC_REQ_MUTUAL_AUTH |
                                   ASC_REQ_DELEGATE |
                                   ASC_REQ_SEQUENCE_DETECT |
                                   ASC_REQ_CONFIDENTIALITY |
                                   ASC_REQ_CONNECTION |
                                   ASC_REQ_USE_SESSION_KEY |
                                   ASC_REQ_EXTENDED_ERROR
#endif
                                       ;
        }

        ss = p_SSPI_Func->
            AcceptSecurityContext(
#ifdef COMMENT
                                   round == 0 ? &hNTLMCred : 0,
#else COMMENT
                                   &hNTLMCred,
#endif /* COMMENT */
                                   round == 2 ? &hNTLMContext : 0,
                                   &NTLMInSecBufDesc,
                                   context_requirements,
                                   SECURITY_NETWORK_DREP,
                                   &hNTLMContext,
                                   &NTLMSecBufDesc,
                                   &NTLMContextAttrib,
                                   &NTLMTimeStampContext);

        ckhexdump("ntlm_is() AcceptSecurityContext Context Requirements",
                 &context_requirements,sizeof(context_requirements));
        ckhexdump("ntlm_is() AcceptSecurityContext Context Attributes",
                 &NTLMContextAttrib,sizeof(NTLMContextAttrib));

        debug(F111,"ntlm_is AcceptSecurityContext",sspi_errstr(ss),ss);
        switch ( ss ) {
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
            sprintf(buf, "%c%c%c%c%c%c%c",
                     IAC, SB, TELOPT_AUTHENTICATION,
                     TELQUAL_REPLY,
                     authentication_version,
                     auth_how,
                     NTLM_CHALLENGE);                   /* safe */

            for ( i=0 ; i<NTLMSecBuf[0].cbBuffer ; i++ ) {
                if ( ((char *)NTLMSecBuf[0].pvBuffer)[i] == IAC )
                    iaccnt++;
            }
            if ( NTLMSecBuf[0].cbBuffer + 2*sizeof(ULONG) + iaccnt + 10 <
                 sizeof(buf)) {
                debug(F111,"ntlm_is sending","NTLMSecBuf[0].cbBuffer",
                         NTLMSecBuf[0].cbBuffer);
                debug(F111,"ntlm_is sending","NTLMSecBuf[0].BufferType",
                         NTLMSecBuf[0].BufferType);
                length = copy_for_net(&buf[7], (char *) &NTLMSecBuf[0],
                                       2*sizeof(ULONG));
                ckhexdump("ntlm_is sending NTLMSecBuf[0].pvBuffer",NTLMSecBuf[0].pvBuffer,
                                        NTLMSecBuf[0].cbBuffer);
                length += copy_for_net(&buf[7+length], NTLMSecBuf[0].pvBuffer,
                                        NTLMSecBuf[0].cbBuffer);
            }
            break;
        case SEC_E_OK:
            ss = p_SSPI_Func->ImpersonateSecurityContext( &hNTLMContext );
            debug(F111,"ntlm_is ImpersonateSecurityContext",sspi_errstr(ss),ss);
            if ( ss == SEC_E_OK ) {
                sprintf(buf, "%c%c%c%c%c%c%c",
                         IAC, SB, TELOPT_AUTHENTICATION,
                         TELQUAL_REPLY,
                         authentication_version,
                         auth_how,
                         NTLM_ACCEPT);                  /* safe */

                ss = p_SSPI_Func->RevertSecurityContext( &hNTLMContext );
                debug(F111,"ntlm_is RevertSecurityContext",sspi_errstr(ss),ss);
                break;
            }
            /* If there was an error Impersonating, fall through */
        default:
            printf("?NTLM Accept failed: %s\r\n",sspi_errstr(ss));
            p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
            p_SSPI_Func->FreeCredentialHandle( &hNTLMCred );
            length = strlen(sspi_errstr(ss));
            if ( length + 10 >= sizeof(buf) )
                length = 0;
            sprintf(buf, "%c%c%c%c%c%c%c%s",
                     IAC, SB, TELOPT_AUTHENTICATION,
                     TELQUAL_REPLY,
                     authentication_version,
                     auth_how,
                     NTLM_REJECT,
                     (length ? sspi_errstr(ss) : ""));  /* safe */
        }
        sprintf(&buf[7+length], "%c%c", IAC, SE);       /* safe */

        if (deblog || tn_deb || debses) {
            int i;
            int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK);
            char * s = NULL;

            switch (buf[6]) {
            case NTLM_CHALLENGE:
                s = "NTLM_CHALLENGE";
                break;
            case NTLM_ACCEPT:
                s = "NTLM_ACCEPT";
                break;
            case NTLM_REJECT:
                s = "NTLM_REJECT";
                break;
            }
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ", TELOPT(TELOPT_AUTHENTICATION),
                       " REPLY ", AUTHTYPE_NAME(authentication_version),
                       " ", AUTHMODE_NAME(mode),
                       " ", s," ",
                       NULL,NULL,NULL);
            tn_hex(tn_msg,TN_MSG_LEN,&buf[7],length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((char *)buf, length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
    }

    switch (buf[6]) {
    case NTLM_ACCEPT: {
        char * p;
        SecPkgContext_Sizes sizes;
        SecPkgContext_Names names;
        SecPkgContext_Lifespan lifespan;
        SecPkgContext_DceInfo dce;
        SecPkgContext_StreamSizes stream;
        SecPkgContext_Authority authority;
        SecPkgContext_KeyInfo keyinfo;
        SecPkgContext_ProtoInfo proto;
        SecPkgContext_PasswordExpiry pwd;
        SecPkgContext_SessionKey skey;
        SecPkgContext_PackageInfo pkginf;
#ifdef IKSD
        extern CHAR * pReferenceDomainName;
        extern int inserver;
#endif /* IKSD */

        /* The Server has now authenticated the Client's User */
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_SIZES,&sizes);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_NAMES,&names);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_LIFESPAN,&lifespan);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_DCE_INFO,&dce);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_STREAM_SIZES,&stream);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_KEY_INFO,&keyinfo);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_AUTHORITY,&authority);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PROTO_INFO,&proto);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PASSWORD_EXPIRY,&pwd);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_SESSION_KEY,&skey);
        p_SSPI_Func->QueryContextAttributes(&hNTLMContext,SECPKG_ATTR_PACKAGE_INFO,&pkginf);

#ifdef IKSD
        /* Format of sUserName is HOST\user or DOMAIN\user */
        if ( !inserver ) {
            p = names.sUserName + strlen(names.sUserName);
            while ( p != names.sUserName ) {
                if ( *p == '\\' ) {
                    *p = '\0';
                    makestr(&pReferenceDomainName,names.sUserName);
                    *p = '\\';
                    p++;
                    break;
                }
                p--;
            }
        } else
#endif /* IKSD */
        {
            /* Let the Reference Domain Name be extracted by zvuser() */
            /* if we are running as IKSD */
            p = names.sUserName;
        }
        ckstrncpy(szUserNameRequested,p,UIDBUFLEN);
        cklower(szUserNameRequested);
        ckstrncpy(szUserNameAuthenticated,names.sUserName,UIDBUFLEN);

        auth_finished(AUTH_VALID);
        accept_complete = 1;
        return AUTH_SUCCESS;
    }
    case NTLM_REJECT:
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    case NTLM_CHALLENGE:
        return AUTH_SUCCESS;
    }
    return AUTH_FAILURE;
}

/* The following code is used to implement the SSPLogonUser() function */

// structure storing the state of the authentication sequence
//
int SSPLogonError = SEC_E_OK;

typedef struct _AUTH_SEQ {
    BOOL _fNewConversation;
    CredHandle _hcred;
    BOOL _fHaveCredHandle;
    BOOL _fHaveCtxtHandle;
    struct _SecHandle  _hctxt;
} AUTH_SEQ, *PAUTH_SEQ;


typedef struct _node_tag  {
    DWORD dwKey;
    PVOID *pData;
    struct _node_tag *pNext;
} NODE, *PNODE;

static NODE Head = {(DWORD)-1, NULL, NULL};

BOOL GetEntry (DWORD dwKey, PVOID *ppData)
{
    PNODE pCurrent;

    pCurrent = Head.pNext;
    while (NULL != pCurrent) {
        if (dwKey == pCurrent->dwKey)  {
            *ppData = pCurrent->pData;
            return(TRUE);
        }
        pCurrent = pCurrent->pNext;
    }

    return(FALSE);
}

BOOL AddEntry (DWORD dwKey, PVOID pData)
{
    PNODE pTemp;

    pTemp = (PNODE) malloc (sizeof (NODE));
    if (NULL == pTemp)  {
        return(FALSE);
    }

    pTemp->dwKey = dwKey;
    pTemp->pData = pData;
    pTemp->pNext = Head.pNext;
    Head.pNext = pTemp;

    return(TRUE);
}

BOOL DeleteEntry (DWORD dwKey, PVOID *ppData)
{
    PNODE pCurrent, pTemp;

    pTemp = &Head;
    pCurrent = Head.pNext;

    while (NULL != pCurrent) {
        if (dwKey == pCurrent->dwKey)  {
            pTemp->pNext = pCurrent->pNext;
            *ppData = pCurrent->pData;
            free (pCurrent);
            return(TRUE);
        }
        else {
            pTemp = pCurrent;
            pCurrent = pCurrent->pNext;
        }
    }

    return(FALSE);
}

BOOL InitSession (DWORD dwKey)
/*++

 Routine Description:

    Initializes the context associated with a key and adds it to the
        collection.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
    PAUTH_SEQ pAS;

    pAS = (PAUTH_SEQ) malloc (sizeof (AUTH_SEQ));
    if (NULL == pAS)
        return(FALSE);

    pAS->_fNewConversation = TRUE;
    pAS->_fHaveCredHandle = FALSE;
    pAS->_fHaveCtxtHandle = FALSE;

    if (!AddEntry (dwKey, (PVOID)pAS))  {
        free (pAS);
        return(FALSE);
    }

    return(TRUE);
}

BOOL TermSession (DWORD dwKey)
/*++

 Routine Description:

    Releases the resources associated with a key and removes it from
        the collection.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
    PAUTH_SEQ pAS;

    if (!DeleteEntry (dwKey, (LPVOID*)&pAS))
        return(FALSE);

    if (pAS->_fHaveCtxtHandle) {
        SSPLogonError = p_SSPI_Func->DeleteSecurityContext (&pAS->_hctxt);
        debug(F111,"TermSession DeleteSecurityContext",
               sspi_errstr(SSPLogonError),SSPLogonError);
    }

    if (pAS->_fHaveCredHandle) {
        SSPLogonError = p_SSPI_Func->FreeCredentialHandle (&pAS->_hcred);
        debug(F111,"TermSession FreeCredentials Handle",
               sspi_errstr(SSPLogonError),SSPLogonError);
    }

    free (pAS);
    return(TRUE);
}

BOOL GenClientContext ( DWORD dwKey,
                        SEC_WINNT_AUTH_IDENTITY *pAuthIdentity,
                        BYTE *pIn,
                        DWORD cbIn,
                        BYTE *pOut,
                        DWORD *pcbOut,
                        BOOL *pfDone)
/*++

 Routine Description:

    Optionally takes an input buffer coming from the server and returns
        a buffer of information to send back to the server.  Also returns
        an indication of whether or not the context is complete.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
    SECURITY_STATUS     ss;
    TimeStamp           Lifetime;
    SecBufferDesc       OutBuffDesc;
    SecBuffer           OutSecBuff;
    SecBufferDesc       InBuffDesc;
    SecBuffer           InSecBuff;
    ULONG               ContextAttributes;
    PAUTH_SEQ           pAS;

    // Lookup pAS based on Key
    //
    if (!GetEntry (dwKey, (PVOID*) &pAS)) {
        debug(F110,"GenClientContext","GetEntry failed",0);
        return(FALSE);
    }

    if (pAS->_fNewConversation)  {
        ss = p_SSPI_Func->AcquireCredentialsHandle (NULL,       // principal
                                                     NTLMSP_NAME_A,
                                                     SECPKG_CRED_OUTBOUND,
                                                     NULL,      // LOGON id
                                                     pAuthIdentity, // auth data
                                                     NULL,      // get key fn
                                                     NULL,      // get key arg
                                                     &pAS->_hcred,
                                                     &Lifetime
                                                     );
        debug(F111,"GenClientContext AcquireCredentialsHandle",sspi_errstr(ss),ss);
        if (ss >= 0)
            pAS->_fHaveCredHandle = TRUE;
        else {
            SSPLogonError = ss;
            return(FALSE);
        }
    }

    // prepare output buffer
    //
    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers = 1;
    OutBuffDesc.pBuffers = &OutSecBuff;

    OutSecBuff.cbBuffer = *pcbOut;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer = pOut;

    // prepare input buffer
    //
    if (!pAS->_fNewConversation)  {
        InBuffDesc.ulVersion = 0;
        InBuffDesc.cBuffers = 1;
        InBuffDesc.pBuffers = &InSecBuff;

        InSecBuff.cbBuffer = cbIn;
        InSecBuff.BufferType = SECBUFFER_TOKEN;
        InSecBuff.pvBuffer = pIn;
    }

    ss = p_SSPI_Func->InitializeSecurityContext ( &pAS->_hcred,
                                                  pAS->_fNewConversation ? NULL : &pAS->_hctxt,
                                                  NULL,
                                                  0,    // context requirements
                                                  0,    // reserved1
                                                  SECURITY_NATIVE_DREP,
                                                  pAS->_fNewConversation ? NULL : &InBuffDesc,
                                                  0,    // reserved2
                                                  &pAS->_hctxt,
                                                  &OutBuffDesc,
                                                  &ContextAttributes,
                                                  &Lifetime
                                                  );
    if ( pAS->_fNewConversation )
        debug(F111,"GenClientContext InitializeSecurityContext (new conversation)",sspi_errstr(ss),ss);
    else
        debug(F111,"GenClientContext InitializeSecurityContext (old conversation)",sspi_errstr(ss),ss);

    if (ss < 0)  {
        SSPLogonError = ss;
        return FALSE;
    }

    pAS->_fHaveCtxtHandle = TRUE;

    // Complete token -- if applicable
    //
    if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))  {
        if (p_SSPI_Func->CompleteAuthToken) {
            ss = p_SSPI_Func->CompleteAuthToken (&pAS->_hctxt, &OutBuffDesc);
            debug(F111,"GenClientContext CompleteAuthToken",sspi_errstr(ss),ss);
            if (ss < 0)  {
                SSPLogonError = ss;
                return FALSE;
            }
        }
        else {
            SSPLogonError = SEC_E_UNSUPPORTED_FUNCTION;
            debug(F110,"GenClientContext CompleteAuthToken missing","Logon not supported",0);
            return FALSE;
        }
    }

    *pcbOut = OutSecBuff.cbBuffer;

    if (pAS->_fNewConversation)
        pAS->_fNewConversation = FALSE;

    *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) ||
                 (SEC_I_COMPLETE_AND_CONTINUE == ss));

    SSPLogonError = SEC_E_OK;
    return TRUE;
}

BOOL GenServerContext ( DWORD dwKey,
                        BYTE *pIn,
                        DWORD cbIn,
                        BYTE *pOut,
                        DWORD *pcbOut,
                        BOOL *pfDone,
                        CtxtHandle * phContext)
/*++

 Routine Description:

    Takes an input buffer coming from the client and returns a buffer
        to be sent to the client.  Also returns an indication of whether or
        not the context is complete.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
    SECURITY_STATUS     ss;
    TimeStamp           Lifetime;
    SecBufferDesc       OutBuffDesc;
    SecBuffer           OutSecBuff;
    SecBufferDesc       InBuffDesc;
    SecBuffer           InSecBuff;
    ULONG               ContextAttributes;
    PAUTH_SEQ           pAS;

    // Lookup pAS based on Key
    //
    if (!GetEntry (dwKey, (PVOID*) &pAS)) {
        SSPLogonError = SEC_E_INVALID_HANDLE;
        debug(F110,"GenServerContext","GetEntry failed",0);
        return(FALSE);
    }

    if (pAS->_fNewConversation)  {
        ss = p_SSPI_Func->AcquireCredentialsHandle ( NULL,      // principal
                                                     NTLMSP_NAME_A,
                                                     SECPKG_CRED_INBOUND,
                                                     NULL,      // LOGON id
                                                     NULL,      // auth data
                                                     NULL,      // get key fn
                                                     NULL,      // get key arg
                                                     &pAS->_hcred,
                                                     &Lifetime
                                                     );
        debug(F111,"GenServerContext AcquireCredentialsHandle()",
               sspi_errstr(ss),ss);
        if (ss >= 0)
            pAS->_fHaveCredHandle = TRUE;
        else {
            SSPLogonError = ss;
            return(FALSE);
        }
    }

    // prepare output buffer
    //
    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers = 1;
    OutBuffDesc.pBuffers = &OutSecBuff;

    OutSecBuff.cbBuffer = *pcbOut;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer = pOut;

    // prepare input buffer
    //
    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers = 1;
    InBuffDesc.pBuffers = &InSecBuff;

    InSecBuff.cbBuffer = cbIn;
    InSecBuff.BufferType = SECBUFFER_TOKEN;
    InSecBuff.pvBuffer = pIn;

    ss = p_SSPI_Func->AcceptSecurityContext ( &pAS->_hcred,
                                              pAS->_fNewConversation ? NULL : &pAS->_hctxt,
                                              &InBuffDesc,
                                              0,        // context requirements
                                              SECURITY_NATIVE_DREP,
                                              &pAS->_hctxt,
                                              &OutBuffDesc,
                                              &ContextAttributes,
                                              &Lifetime
                                              );
    if ( pAS->_fNewConversation )
        debug(F111,"GenServerContext AcceptSecurityContext() (new conversation)",
               sspi_errstr(ss),ss);
    else
        debug(F111,"GenServerContext AcceptSecurityContext() (old conversation)",
               sspi_errstr(ss),ss);
    if (ss < 0)  {
        SSPLogonError = ss;
        return FALSE;
    }

    pAS->_fHaveCtxtHandle = TRUE;

    // Complete token -- if applicable
    //
    if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))  {
        if (p_SSPI_Func->CompleteAuthToken) {
            ss = p_SSPI_Func->CompleteAuthToken (&pAS->_hctxt, &OutBuffDesc);
            debug(F111,"GenServerContext CompleteAuthToken()",
                   sspi_errstr(ss),ss);
            if (ss < 0)  {
                SSPLogonError = ss;
                return FALSE;
            }
        }
        else {
            SSPLogonError = SEC_E_UNSUPPORTED_FUNCTION;
            debug(F110,"GenServerContext CompleteAuthToken missing",
                   "Logon Not Supported",0);
            return FALSE;
        }
    }

    *pcbOut = OutSecBuff.cbBuffer;

    if (pAS->_fNewConversation)
        pAS->_fNewConversation = FALSE;

    *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) ||
                 (SEC_I_COMPLETE_AND_CONTINUE == ss));

    if (*pfDone && phContext) {
        haveNTLMContext = 1;
        memcpy(phContext,&pAS->_hctxt,sizeof(CtxtHandle));
        ZeroMemory(&pAS->_hctxt,sizeof(CtxtHandle));
        pAS->_fHaveCtxtHandle = FALSE;
    }
    SSPLogonError = SEC_E_OK;
    return TRUE;
}

void
ntlm_logout() {
    if ( haveNTLMContext ) {
        p_SSPI_Func->RevertSecurityContext(&hNTLMContext);
        p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
    }
    haveNTLMContext = 0;
    SSPLogonError = SEC_E_OK;
}

BOOL
SSPLogonUser( LPTSTR DomainName,
              LPTSTR UserName,
              LPTSTR Password )
{

    BOOL done = FALSE;
    DWORD cbOut, cbIn;
    char szUser[80];
    DWORD cbUser = 80;
    SEC_WINNT_AUTH_IDENTITY AuthIdentity;

    if ( !p_SSPI_Func ) {
        SSPLogonError = SEC_E_SECPKG_NOT_FOUND;
        debug(F110,"SSPLogonUser","NTLM SSPI not installed",0);
        return(FALSE);
    }

    if(!InitSession(0)) {
        debug(F110,"SSPLogonUser","unable to Init Session 0",0);
        SSPLogonError = SEC_E_INTERNAL_ERROR;
        return(FALSE);
    }

    if(!InitSession(1)) {
        debug(F110,"SSPLogonUser","unable to Init Session 1",0);
        SSPLogonError = SEC_E_INTERNAL_ERROR;
        return(FALSE);
    }

    ntlm_pClientBuf = (PBYTE) malloc(ntlm_cbMaxToken);
    ntlm_pServerBuf = (PBYTE) malloc(ntlm_cbMaxToken);

    if ( haveNTLMContext ) {
        SSPLogonError = p_SSPI_Func->DeleteSecurityContext(&hNTLMContext);
        debug(F110,"SSPLogonUser DeleteSecurityContext",sspi_errstr(SSPLogonError),SSPLogonError);
    }

    ZeroMemory( &hNTLMContext,sizeof(hNTLMContext));
    ZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );

    if ( DomainName != NULL )
    {
        AuthIdentity.Domain = DomainName;
        AuthIdentity.DomainLength = lstrlen(DomainName);
    }

    if ( UserName != NULL )
    {
        AuthIdentity.User = UserName;
        AuthIdentity.UserLength = lstrlen(UserName);
    }

    if ( Password != NULL )
    {
        AuthIdentity.Password = Password;
        AuthIdentity.PasswordLength = lstrlen(Password);
    }

#ifdef UNICODE
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

    //
    // Prepare client message (negotiate).
    //
    cbOut = ntlm_cbMaxToken;
    if (!GenClientContext (0,
                            &AuthIdentity,
                            NULL,
                            0,
                            ntlm_pClientBuf,
                            &cbOut,
                            &done))
    {
        debug(F111,"SSPLogonUser","GenClientContext failed (negotiate)",
               SSPLogonError);
        return(FALSE);
     }

    cbIn = cbOut;
    //
    // Prepare server message (challenge).
    //
    cbOut = ntlm_cbMaxToken;
    if (!GenServerContext (1,
                            ntlm_pClientBuf,
                            cbIn,
                            ntlm_pServerBuf,
                            &cbOut,
                            &done,
                            NULL))
    {
    //
    // Most likely failure: AcceptServerContext fails with
    // SEC_E_LOGON_DENIED in the case of bad username or password
    //
    // Unexpected Result: Logon will succeed if you pass in a bad
    // username and the guest account is enabled in the specified
    // domain.
    //
        debug(F111,"SSPLogonUser","GenServerContext failed (challenge)",
               SSPLogonError);
        return(FALSE);
    }

    cbIn = cbOut;
    //
    // Prepare client message (authenticate).
    //
    cbOut = ntlm_cbMaxToken;
    if (!GenClientContext (0,
                            &AuthIdentity,
                            ntlm_pServerBuf,
                            cbIn,
                            ntlm_pClientBuf,
                             &cbOut,
                            &done))
    {
        debug(F111,"SSPLogonUser","GenClientContext failed (authenticate)",
               SSPLogonError);
        return(FALSE);
    }

    cbIn = cbOut;
    //
    // Prepare server message (authentication).
    //
    cbOut = ntlm_cbMaxToken;
    if (!GenServerContext (1,
                            ntlm_pClientBuf,
                            cbIn,
                            ntlm_pServerBuf,
                             &cbOut,
                            &done,
                            &hNTLMContext))
    {
        debug(F111,"SSPLogonUser","GenServerContext failed (authentication)",
               SSPLogonError);
        return(FALSE);
    }

    TermSession(0);
    TermSession(1);

    free(ntlm_pClientBuf);
    free(ntlm_pServerBuf);

    SSPLogonError = SEC_E_OK;
    return(TRUE);
}
#endif /* NTLM */

#ifdef NT
HINSTANCE hKRB5_32 = NULL;
HINSTANCE hKRB4_32 = NULL;
HINSTANCE hCOMERR32 = NULL;
HINSTANCE hPROFILE = NULL;
HINSTANCE hKRB42UID = NULL;
HINSTANCE hKRB52UID = NULL;
HINSTANCE hGSSAPI = NULL;
HINSTANCE hKRB524 = NULL;
#else /* NT */
HMODULE hKRB5_32 = NULL;
HMODULE hKRB4_32 = NULL;
HMODULE hCOMERR32 = NULL;
HMODULE hPROFILE = NULL;
HMODULE hKRB42UID = NULL;
HMODULE hKRB52UID = NULL;
HMODULE hGSSAPI = NULL;
HMODULE hEMX = NULL;
#endif /* NT */
static int cygnus = 0;

static void
ck_krb4_loaddll_eh( void )
{
    if ( hKRB4_32 ) {
#ifdef NT
        FreeLibrary(hKRB4_32);
        hKRB4_32 = NULL;
#else /* NT */
        DosFreeModule(hKRB4_32);
        hKRB4_32 = 0;
#endif  /* NT */
    }
    if ( hKRB42UID ) {
#ifdef NT
        FreeLibrary(hKRB42UID);
        hKRB42UID = NULL;
#else /* NT */
        DosFreeModule(hKRB42UID);
        hKRB42UID = 0;
#endif  /* NT */
    }

#ifdef KRB4
    p_krb_get_err_text_entry         = NULL;
    p_krb_get_cred                   = NULL;
    p_krb_mk_req                     = NULL;
    p_krb_realmofhost                = NULL;
    p_krb_get_phost                  = NULL;
    p_k4_des_new_random_key             = NULL;
    p_k4_des_set_random_generator_seed  = NULL;
    p_k4_des_key_sched                  = NULL;
    p_k4_des_ecb_encrypt                = NULL;
    p_k4_des_pcbc_encrypt               = NULL;
    p_k4_des_string_to_key            = NULL;
    p_k4_des_fixup_key_parity         = NULL;
    p_krb_get_pw_in_tkt               = NULL;
    p_krb_get_pw_in_tkt_preauth       = NULL;
    p_krb_get_lrealm                  = NULL;
    p_krb_get_err_text                = NULL;
    p_kname_parse                     = NULL;
    p_dest_tkt                        = NULL;
    p_krb_get_tf_realm                = NULL;
    p_krb_get_tf_fullname             = NULL;
    p_tf_get_pname                    = NULL;
    p_tf_get_pinst                    = NULL;
    p_tf_get_cred                     = NULL;
    p_tf_close                        = NULL;
    p_tf_init                         = NULL;
    p_tkt_string                      = NULL;
    p_krb_check_serv                  = NULL;
    p_kuserok                         = NULL;
    p_krb_kntoln                      = NULL;
    p_krb_rd_req                      = NULL;
    p_krb_sendauth                    = NULL;
    p_set_krb_debug                   = NULL;
    p_set_krb_ap_req_debug            = NULL;
    p_krb_mk_safe      = NULL;
    p_krb_mk_priv      = NULL;
    p_krb_rd_priv      = NULL;
    p_krb_rd_safe      = NULL;
    p_krb_in_tkt       = NULL;
    p_krb_save_credentials = NULL;

    p_k95_k4_princ_to_userid = NULL;
    p_k95_k4_userok = NULL;
#endif /* KRB4 */
}

static void
ck_krb5_loaddll_eh( void )
{
#ifdef OS2ONLY
    if ( hEMX ) {
        DosFreeModule(hEMX);
        hEMX = 0;
    }
#endif /* OS2 */
    if ( hPROFILE ) {
#ifdef NT
        FreeLibrary(hPROFILE);
        hPROFILE = NULL;
#else /* NT */
        DosFreeModule(hPROFILE);
        hPROFILE = 0;
#endif  /* NT */
    }
    if ( hKRB5_32 ) {
#ifdef NT
        FreeLibrary(hKRB5_32);
        hKRB5_32 = NULL;
#else /* NT */
        DosFreeModule(hKRB5_32);
        hKRB5_32 = 0;
#endif  /* NT */
    }
    if ( hCOMERR32 ) {
#ifdef NT
        FreeLibrary(hCOMERR32);
        hCOMERR32 = NULL;
#else /* NT */
        DosFreeModule(hCOMERR32);
        hCOMERR32 = 0;
#endif /* NT */
    }
#ifdef NT
    if ( hKRB524 ) {
        FreeLibrary(hKRB524);
        hKRB524 = NULL;
    }
#endif /* NT */

    if ( hKRB52UID ) {
#ifdef NT
        FreeLibrary(hKRB52UID);
        hKRB52UID = NULL;
#else /* NT */
        DosFreeModule(hKRB52UID);
        hKRB52UID = 0;
#endif  /* NT */
    }

    cygnus = 0;
#ifdef KRB5
    p_com_err                        = NULL;
    p_com_err_va                     = NULL;
    p_error_message                  = NULL;
    p_krb5_free_creds                = NULL;
    p_krb5_free_data                 = NULL;
    p_krb5_free_data_contents        = NULL;
    p_krb5_copy_keyblock             = NULL;
    p_krb5_free_keyblock             = NULL;
    p_krb5_auth_con_getlocalsubkey   = NULL;
    p_krb5_mk_req_extended           = NULL;
    p_krb5_mk_req                    = NULL;
    p_krb5_auth_con_setflags         = NULL;
    p_krb5_auth_con_init             = NULL;
    p_krb5_auth_con_free             = NULL;
    p_krb5_get_credentials           = NULL;
    p_krb5_free_cred_contents        = NULL;
    p_krb5_sname_to_principal        = NULL;
    p_krb5_cc_default                = NULL;
    p_krb5_free_ap_rep_enc_part      = NULL;
    p_krb5_rd_rep                    = NULL;
    p_krb5_init_context              = NULL;
    p_krb5_init_ets                  = NULL;
    p_krb5_free_context              = NULL;
    p_krb5_free_principal            = NULL;
    p_krb5_free_unparsed_name        = NULL;
    p_krb5_fwd_tgt_creds             = NULL;
    p_krb5_auth_con_genaddrs         = NULL;
    p_des_new_random_key             = NULL;
    p_des_set_random_generator_seed  = NULL;
    p_des_key_sched                  = NULL;
    p_des_ecb_encrypt                = NULL;
    p_des_pcbc_encrypt               = NULL;
    p_des_string_to_key              = NULL;
    p_des_fixup_key_parity           = NULL;

    p_krb5_get_host_realm            = NULL;
    p_krb5_free_host_realm           = NULL;
    p_krb5_get_in_tkt_with_keytab    = NULL;
    p_krb5_get_in_tkt_with_password  = NULL;
    p_krb5_read_password             = NULL;
    p_krb5_build_principal_ext       = NULL;
    p_krb5_unparse_name              = NULL;
    p_krb5_parse_name                = NULL;
    p_krb5_cc_resolve                = NULL;
    p_krb5_cc_default_name           = NULL;
    p_krb5_string_to_timestamp       = NULL;
    p_krb5_kt_resolve                = NULL;
    p_krb5_string_to_deltat          = NULL;
    p_krb5_timeofday                 = NULL;
    p_krb5_get_credentials_renew     = NULL;
    p_krb5_get_credentials_validate  = NULL;
    p_krb5_copy_principal            = NULL;
    p_krb5_timestamp_to_sfstring     = NULL;
    p_krb5_kt_default                = NULL;
    p_krb5_free_ticket               = NULL;
    p_decode_krb5_ticket             = NULL;
    p_cygnus_decode_krb5_ticket      = NULL;

#ifdef CHECKADDRS
    p_krb5_os_localaddr              = NULL;
    p_krb5_address_search            = NULL;
    p_krb5_free_addresses            = NULL;
#endif /* CHECKADDRS */

    p_krb5_auth_con_getremotesubkey  = NULL;
    p_krb5_mk_rep                    = NULL;
    p_krb5_free_authenticator        = NULL;
    p_krb5_verify_checksum           = NULL;
    p_krb5_auth_con_getkey           = NULL;
    p_krb5_auth_con_getauthenticator = NULL;
    p_krb5_rd_req                    = NULL;
    p_krb5_auth_con_setrcache        = NULL;
    p_krb5_get_server_rcache         = NULL;
    p_krb5_auth_con_getrcache        = NULL;
    p_krb5_free_tgt_creds            = NULL;
    p_krb5_rd_cred                   = NULL;

    p_krb5_c_enctype_compare         = NULL;
    p_krb5_free_error                = NULL;
    p_krb5_sendauth                  = NULL;
    p_krb5_process_key               = NULL;
    p_krb5_use_enctype               = NULL;
    p_krb5_encrypt                   = NULL;
    p_krb5_encrypt_size              = NULL;
    p_krb5_decrypt                   = NULL;
    p_krb5_kuserok                   = NULL;
    p_krb5_appdefault_boolean        = NULL;
    p_krb5_appdefault_string         = NULL;

    p_krb5_get_init_creds_password             = NULL;
    p_krb5_get_init_creds_opt_set_address_list = NULL;
    p_krb5_get_renewed_creds                   = NULL;
    p_krb5_get_validated_creds                 = NULL;
    p_krb5_get_init_creds_opt_set_tkt_life     = NULL;
    p_krb5_get_init_creds_opt_set_forwardable  = NULL;
    p_krb5_get_init_creds_opt_set_proxiable    = NULL;
    p_krb5_get_init_creds_opt_set_renew_life   = NULL;
    p_krb5_get_init_creds_opt_init             = NULL;
    p_krb5_get_init_creds_opt_set_etype_list   = NULL;
    p_krb5_get_init_creds_opt_set_preauth_list = NULL;
    p_krb5_get_init_creds_opt_set_salt         = NULL;

    p_krb5_rd_safe                  = NULL;
    p_krb5_mk_safe                  = NULL;
    p_krb5_rd_priv                  = NULL;
    p_krb5_mk_priv                  = NULL;
    p_krb5_auth_con_setuseruserkey  = NULL;
    p_krb5_get_profile              = NULL;

    p_profile_get_subsection_names = NULL;
    p_profile_get_relation_names   = NULL;

    p_k95_k5_principal_to_localname = NULL;
    p_k95_k5_userok = NULL;

    p_krb5_free_keyblock_contents = NULL;
    p_krb5_c_encrypt = NULL;
    p_krb5_c_decrypt = NULL;
    p_krb5_c_make_random_key = NULL;
    p_krb5_c_random_seed = NULL;
    p_krb5_c_encrypt_length = NULL;
    p_krb5_c_block_size = NULL;
    p_krb5_kt_default_name = NULL;

    p_krb5_cc_get_principal   = NULL;
    p_krb5_cc_store_cred      = NULL;
    p_krb5_cc_initialize      = NULL;
    p_krb5_cc_destroy         = NULL;
    p_krb5_cc_end_seq_get     = NULL;
    p_krb5_cc_next_cred       = NULL;
    p_krb5_cc_start_seq_get   = NULL;
    p_krb5_cc_get_type        = NULL;
    p_krb5_cc_get_name        = NULL;
    p_krb5_cc_set_flags       = NULL;
    p_krb5_cc_close           = NULL;

    p_krb5_kt_get_type        = NULL;
    p_krb5_kt_get_name        = NULL;
    p_krb5_kt_close           = NULL;
    p_krb5_kt_get_entry       = NULL;
    p_krb5_kt_start_seq_get   = NULL;
    p_krb5_kt_next_entry      = NULL;
    p_krb5_kt_end_seq_get     = NULL;
    p_krb5_build_principal    = NULL;

    p_krb524_init_ets         = NULL;
    p_krb524_convert_creds_kdc = NULL;
#endif /* KRB5 */
}

int
ck_krb4_loaddll( void )
{
#ifdef KRB4
    ULONG rc = 0 ;
    int load_error = 0, len;
#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "KRB4_32";
#endif /* OS2ONLY */

#ifdef NT
    HINSTANCE hLEASH;

    if ( !(hKRB4_32 = LoadLibrary("KRBV4W32")) ) {
        rc = GetLastError() ;
        debug(F111, "Kerberos LoadLibrary failed","KRBV4W32",rc) ;
        ck_krb4_loaddll_eh();
    }
    if ( !hKRB4_32 &&
         !(hKRB4_32 = LoadLibrary("KRB4_32"))) {
        rc = GetLastError() ;
        debug(F111, "Kerberos LoadLibrary failed","KRB4_32",rc) ;
        ck_krb4_loaddll_eh();
    }
    if (hKRB4_32) {
        debug(F100,"Kerberos IV support provided by MIT Leash","",0);
        if (((FARPROC) p_krb_get_err_text_entry =
              GetProcAddress( hKRB4_32, "get_krb_err_txt_entry" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,
                  "Kerberos GetProcAddress failed","get_krb_err_txt_entry",rc);
            load_error = 1;
        }
        if (((FARPROC) p_krb_get_cred =
              GetProcAddress( hKRB4_32, "krb_get_cred" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","krb_get_cred",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_mk_req =
              GetProcAddress( hKRB4_32, "krb_mk_req" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","krb_mk_req",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_realmofhost =
              GetProcAddress( hKRB4_32, "krb_realmofhost" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"Kerberos GetProcAddress failed","krb_realmofhost",rc);
            load_error = 1;
        }
        if (((FARPROC) p_krb_get_phost =
              GetProcAddress( hKRB4_32, "krb_get_phost" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","krb_get_phost",rc) ;
            load_error = 1;
        }

        /* The DES functions are not required.  But if we do not have */
        /* them and do not have the CRYPT DLL we will not be able to  */
        /* perform mutual authentication.                             */
        if (((FARPROC) p_k4_des_ecb_encrypt =
              GetProcAddress( hKRB4_32, "des_ecb_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"Kerberos GetProcAddress failed","des_ecb_encrypt",rc);
        }
        if (((FARPROC) p_k4_des_pcbc_encrypt =
              GetProcAddress( hKRB4_32, "des_pcbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"Kerberos GetProcAddress failed","des_pcbc_encrypt",rc);
        }
        if (((FARPROC) p_k4_des_new_random_key =
              GetProcAddress( hKRB4_32, "des_new_random_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,
                  "Kerberos GetProcAddress failed","des_new_random_key",rc) ;
        }
        if (((FARPROC) p_k4_des_key_sched =
              GetProcAddress( hKRB4_32, "des_key_sched" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","des_key_sched",rc) ;
        }
        if (((FARPROC) p_k4_des_set_random_generator_seed =
             GetProcAddress(hKRB4_32,"des_set_random_generator_seed")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "des_set_random_generator_seed",rc) ;
        }

        if (((FARPROC) p_krb_get_pw_in_tkt =
              GetProcAddress( hKRB4_32, "krb_get_pw_in_tkt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_pw_in_tkt",rc) ;
            load_error = 1;
        }
        /* Not supported in Leash */
        if (((FARPROC) p_krb_get_pw_in_tkt_preauth =
             GetProcAddress( hKRB4_32, "krb_get_pw_in_tkt_preauth" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_pw_in_tkt_preauth",rc) ;
        }
        if (((FARPROC) p_krb_get_lrealm =
              GetProcAddress( hKRB4_32, "krb_get_lrealm" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_lrealm",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_get_err_text =
              GetProcAddress( hKRB4_32, "krb_get_err_text" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_err_text",rc) ;
        }
        if (((FARPROC) p_kname_parse =
              GetProcAddress( hKRB4_32, "kname_parse" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "kname_parse",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_dest_tkt =
              GetProcAddress( hKRB4_32, "dest_tkt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "dest_tkt",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_get_tf_realm =
              GetProcAddress( hKRB4_32, "krb_get_tf_realm" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_tf_realm",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_get_tf_fullname =
              GetProcAddress( hKRB4_32, "krb_get_tf_fullname" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_get_tf_fullname",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tf_get_pname =
              GetProcAddress( hKRB4_32, "tf_get_pname" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tf_get_pname",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tf_get_pinst =
              GetProcAddress( hKRB4_32, "tf_get_pinst" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tf_get_pinst",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tf_get_cred =
              GetProcAddress( hKRB4_32, "tf_get_cred" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tf_get_cred",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tf_close =
              GetProcAddress( hKRB4_32, "tf_close" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tf_close",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tf_init =
              GetProcAddress( hKRB4_32, "tf_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tf_init",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_tkt_string =
              GetProcAddress( hKRB4_32, "tkt_string" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "tkt_string",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_check_serv =
              GetProcAddress( hKRB4_32, "krb_check_serv" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_check_serv",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_kuserok =
              GetProcAddress( hKRB4_32, "kuserok" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "kuserok",rc) ;
        }
        if (((FARPROC) p_krb_kntoln =
              GetProcAddress( hKRB4_32, "krb_kntoln" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_kntoln",rc) ;
        }
        if (((FARPROC) p_krb_rd_req =
              GetProcAddress( hKRB4_32, "krb_rd_req" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_rd_req",rc) ;
        }
        if (((FARPROC) p_krb_sendauth =
              GetProcAddress( hKRB4_32, "krb_sendauth" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_sendauth",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_set_krb_debug =
              GetProcAddress( hKRB4_32, "set_krb_debug" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "set_krb_debug",rc) ;
        }
        if (((FARPROC) p_set_krb_ap_req_debug =
              GetProcAddress( hKRB4_32, "set_krb_ap_req_debug" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "set_krb_ap_req_debug",rc) ;
        }
        if (((FARPROC) p_krb_mk_safe =
              GetProcAddress( hKRB4_32, "krb_mk_safe" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_mk_safe",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_mk_priv =
              GetProcAddress( hKRB4_32, "krb_mk_priv" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_mk_priv",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_rd_priv =
              GetProcAddress( hKRB4_32, "krb_rd_priv" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_rd_priv",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_rd_safe =
              GetProcAddress( hKRB4_32, "krb_rd_safe" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_rd_safe",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_in_tkt =
              GetProcAddress( hKRB4_32, "krb_in_tkt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_in_tkt",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb_save_credentials =
              GetProcAddress( hKRB4_32, "krb_save_credentials" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb_save_credentials",rc) ;
            load_error = 1;
        }

        if ( load_error ) {
            ck_krb4_loaddll_eh();
            return 0;
        }
    }
#else /* NT */
    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,dllname);         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hKRB4_32);
    if (rc) {
        debug(F111, "Kerberos IV LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hKRB4_32);
    }

    if (rc) {
        debug(F111, "Kerberos IV LoadLibrary failed",fail,rc) ;
        ck_krb4_loaddll_eh();
    } else {
        debug(F100,"Kerberos IV support provided by MIT","",0);
        if (rc = DosQueryProcAddr(hKRB4_32,0,"get_krb_err_txt_entry",
                                   (PFN*)&p_krb_get_err_text_entry))
        {
            debug(F111,"Kerberos GetProcAddress failed","get_krb_err_txt_entry",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_cred",
                                   (PFN*)&p_krb_get_cred))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_cred",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_mk_req",
                                   (PFN*)&p_krb_mk_req))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_mk_req",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_realmofhost",
                                   (PFN*)&p_krb_realmofhost))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_realmofhost",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_phost",
                                   (PFN*)&p_krb_get_phost))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_phost",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"des_ecb_encrypt",
                                   (PFN*)&p_k4_des_ecb_encrypt))
        {
            debug(F111,"Kerberos GetProcAddress failed","des_ecb_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"des_pcbc_encrypt",
                                   (PFN*)&p_k4_des_pcbc_encrypt))
        {
            debug(F111,"Kerberos GetProcAddress failed","des_pcbc_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"des_new_random_key",
                                   (PFN*)&p_k4_des_new_random_key))
        {
            debug(F111,"Kerberos GetProcAddress failed","des_new_random_key",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"des_key_sched",
                                   (PFN*)&p_k4_des_key_sched))
        {
            debug(F111,"Kerberos GetProcAddress failed","des_key_sched",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"des_set_random_generator_seed",
                                   (PFN*)&p_k4_des_set_random_generator_seed))
        {
            debug(F111,"Kerberos GetProcAddress failed","des_set_random_generator_seed",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_pw_in_tkt",
                                   (PFN*)&p_krb_get_pw_in_tkt))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_pw_in_tkt",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_pw_in_tkt_preauth",
                                   (PFN*)&p_krb_get_pw_in_tkt_preauth))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_pw_in_tkt_preauth",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_lrealm",
                                   (PFN*)&p_krb_get_lrealm))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_lrealm",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_err_text",
                                   (PFN*)&p_krb_get_err_text))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_err_text",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"kname_parse",
                                   (PFN*)&p_kname_parse))
        {
            debug(F111,"Kerberos GetProcAddress failed","kname_parse",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"dest_tkt",
                                   (PFN*)&p_dest_tkt))
        {
            debug(F111,"Kerberos GetProcAddress failed","dest_tkt",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_tf_realm",
                                   (PFN*)&p_krb_get_tf_realm))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_tf_realm",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_get_tf_fullname",
                                   (PFN*)&p_krb_get_tf_fullname))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_get_tf_fullname",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tf_get_pname",
                                   (PFN*)&p_tf_get_pname))
        {
            debug(F111,"Kerberos GetProcAddress failed","tf_get_pname",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tf_get_pinst",
                                   (PFN*)&p_tf_get_pinst))
        {
            debug(F111,"Kerberos GetProcAddress failed","tf_get_pinst",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tf_get_cred",
                                   (PFN*)&p_tf_get_cred))
        {
            debug(F111,"Kerberos GetProcAddress failed","tf_get_cred",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tf_close",
                                   (PFN*)&p_tf_close))
        {
            debug(F111,"Kerberos GetProcAddress failed","tf_close",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tf_init",
                                   (PFN*)&p_tf_init))
        {
            debug(F111,"Kerberos GetProcAddress failed","tf_init",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"tkt_string",
                                   (PFN*)&p_tkt_string))
        {
            debug(F111,"Kerberos GetProcAddress failed","tkt_string",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_check_serv",
                                   (PFN*)&p_krb_check_serv))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_check_serv",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"kuserok",
                                   (PFN*)&p_kuserok))
        {
            debug(F111,"Kerberos GetProcAddress failed","kuserok",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_kntoln",
                                   (PFN*)&p_krb_kntoln))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_kntoln",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_rd_req",
                                   (PFN*)&p_krb_rd_req))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_rd_req",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_sendauth",
                                   (PFN*)&p_krb_sendauth))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_sendauth",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"set_krb_debug",
                                   (PFN*)&p_set_krb_debug))
        {
            debug(F111,"Kerberos GetProcAddress failed","set_krb_debug",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"set_krb_ap_req_debug",
                                   (PFN*)&p_set_krb_ap_req_debug))
        {
            debug(F111,"Kerberos GetProcAddress failed","set_krb_ap_req_debug",rc);
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_mk_safe",
                                   (PFN*)&p_krb_mk_safe))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_mk_safe",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_mk_priv",
                                   (PFN*)&p_krb_mk_priv))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_mk_priv",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_rd_priv",
                                   (PFN*)&p_krb_rd_priv))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_rd_priv",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB4_32,0,"krb_rd_safe",
                                   (PFN*)&p_krb_rd_safe))
        {
            debug(F111,"Kerberos GetProcAddress failed","krb_rd_safe",rc);
            load_error = 1;
        }
        if ( load_error ) {
            ck_krb4_loaddll_eh();
            return 0;
        }
    }
#endif /* NT */

    if ( deblog ) {
        if ( hKRB4_32 )
            printf("MIT Kerberos 4 available\n");
    }

    /* Attempt to load a krb4_principal_to_userid() function */
#ifdef NT
    hKRB42UID = LoadLibrary("KRB42UID");
    if ( hKRB42UID ) {
        if (((FARPROC) p_k95_k4_princ_to_userid =
              GetProcAddress( hKRB42UID, "krb4_principal_to_userid" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"KRB42UID GetProcAddress failed","krb4_principal_to_userid",rc);
        }
        if (((FARPROC) p_k95_k4_userok =
              GetProcAddress( hKRB42UID, "krb4_userok" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"KRB42UID GetProcAddress failed","krb4_userok",rc);
        }
    }
#else /* NT */
    dllname = "KRB42UID";
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,dllname);         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hKRB42UID);
    if (rc) {
        debug(F111, "KRB42UID LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hKRB42UID);
    }
    if ( rc ) {
        debug(F111, "KRB42UID LoadLibrary failed",fail,rc) ;
    } else {
        debug(F111, "KRB42UID LoadLibrary success",fail,rc) ;
        if (rc = DosQueryProcAddr( hKRB42UID, 0, "krb4_principal_to_userid",
                                  (PFN*) &p_k95_k4_princ_to_userid))
        {
            debug(F111,"KRB42UID GetProcAddress failed",
                  "krb4_principal_to_userid",rc);
        }
        if (rc = DosQueryProcAddr( hKRB42UID, 0, "krb4_userok",
                                  (PFN*) &p_k95_k4_userok))
        {
            debug(F111,"KRB42UID GetProcAddress failed",
                  "krb4_userok",rc);
        }
    }


    /* Initialize Kerberos 4 ticket options based upon MIT Leash selections */
    hLEASH = LoadLibrary("LEASHW32");
    if ( hLEASH )
    {
        DWORD (* pLeash_get_default_lifetime)(void);

        (FARPROC) pLeash_get_default_lifetime =
            GetProcAddress(hLEASH, "Leash_get_default_lifetime");

        if ( pLeash_get_default_lifetime )
        krb4_d_lifetime = pLeash_get_default_lifetime();
        FreeLibrary("LEASHW32");
    }
#endif /* NT */
    return(1);
#else /* KRB4 */
    return(0);
#endif /* KRB4 */
}

int
ck_krb5_loaddll( void )
{
#ifdef KRB5
    ULONG rc = 0 ;
    int load_error = 0, len;
#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "KRB5_32";
    static void (_System * p_emx_init)(void)=NULL;
#endif /* OS2ONLY */

#ifdef NT
    HINSTANCE hLEASH = NULL;
    
    hKRB5_32 = LoadLibrary("KRB5_32") ;
    if ( !hKRB5_32 ) {
        /* Try Cygnus Solutions version */
        hKRB5_32 = LoadLibrary("LIBKRB5");
        cygnus = 1;
    }
    if ( !hKRB5_32 )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos LoadLibrary failed","KRB5_32",rc) ;
    }

    hKRB524 = LoadLibrary("KRB524") ;
    if ( !hKRB524 )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos LoadLibrary failed","KRB524",rc) ;
    } else {
        if (((FARPROC) p_krb524_init_ets =
              GetProcAddress( hKRB524, "krb524_init_ets" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb524_init_ets",rc);
        }
        if (((FARPROC) p_krb524_convert_creds_kdc =
              GetProcAddress( hKRB524, "krb524_convert_creds_kdc" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb524_convert_creds_kdc",rc);
        }
    }

    if ( hKRB5_32 != NULL ) {
    if ( cygnus ) {
        debug(F100,"Kerberos V support provided by Cygnus Solutions","",0);
        if (((FARPROC) p_com_err =
              GetProcAddress( hKRB5_32, "com_err" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","com_err",rc) ;
            load_error = 1;
        }
#ifdef COMMENT
        /* Cygnus Solutions does not have this function in their DLL */
        if (((FARPROC) p_com_err_va =
              GetProcAddress( hKRB5_32, "com_err_va" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","com_err_va",rc) ;
            load_error = 1;
        }
#endif /* COMMENT */
        if (((FARPROC) p_error_message =
              GetProcAddress( hKRB5_32, "error_message" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","error_message",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_init_ets =
              GetProcAddress( hKRB5_32, "krb5_init_ets" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","krb5_init_ets",rc) ;
            /* krb5_init_ets is a private function as of 1.2.5 */
        }
        if (((FARPROC) p_cygnus_decode_krb5_ticket=
              GetProcAddress( hKRB5_32, "decode_krb5_ticket" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "decode_krb5_ticket",rc) ;
            load_error = 1;
        }
    }
    else {
        debug(F100,"Kerberos V support provided by MIT","",0);
        hCOMERR32 = LoadLibrary("COMERR32") ;
        if ( !hCOMERR32 )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos LoadLibrary failed","COMERR32",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_com_err =
              GetProcAddress( hCOMERR32, "com_err" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","com_err",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_com_err_va =
              GetProcAddress( hCOMERR32, "com_err_va" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","com_err_va",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_error_message =
              GetProcAddress( hCOMERR32, "error_message" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","error_message",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_decode_krb5_ticket =
              GetProcAddress( hKRB5_32, "decode_krb5_ticket" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "decode_krb5_ticket",rc) ;
            if (((FARPROC) p_cygnus_decode_krb5_ticket =
                  GetProcAddress( hKRB5_32, "krb5_decode_ticket" )) == NULL )
            {
                rc = GetLastError() ;
                debug(F111, "Kerberos GetProcAddress failed",
                       "krb5_decode_ticket",rc) ;
                load_error = 1;
            }
        }
        if (((FARPROC) p_krb5_mk_rep =
              GetProcAddress( hKRB5_32, "krb5_mk_rep" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_mk_rep",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_free_authenticator=
              GetProcAddress( hKRB5_32, "krb5_free_authenticator" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_authenticator",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_verify_checksum=
              GetProcAddress( hKRB5_32, "krb5_verify_checksum" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_verify_checksum",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_rd_req=
              GetProcAddress( hKRB5_32, "krb5_rd_req" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_rd_req",rc) ;
            load_error = 1;
        }
    }

    if (((FARPROC) p_krb5_free_creds =
          GetProcAddress( hKRB5_32, "krb5_free_creds" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_free_creds",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_data =
          GetProcAddress( hKRB5_32, "krb5_free_data" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_free_data",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_data_contents =
          GetProcAddress( hKRB5_32, "krb5_free_data_contents" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_free_data_contents",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_copy_keyblock =
          GetProcAddress( hKRB5_32, "krb5_copy_keyblock" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_copy_keyblock",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_keyblock =
          GetProcAddress( hKRB5_32, "krb5_free_keyblock" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_free_keyblock",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_auth_con_getlocalsubkey =
          GetProcAddress( hKRB5_32, "krb5_auth_con_getlocalsubkey" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_auth_con_getlocalsubkey",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_mk_req_extended =
          GetProcAddress( hKRB5_32, "krb5_mk_req_extended" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"Kerberos GetProcAddress failed","krb5_mk_req_extended",rc);
        load_error = 1;
    }
    if (((FARPROC) p_krb5_mk_req =
          GetProcAddress( hKRB5_32, "krb5_mk_req" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"Kerberos GetProcAddress failed","krb5_mk_req",rc);
        load_error = 1;
    }
    if (((FARPROC) p_krb5_auth_con_setflags =
      GetProcAddress( hKRB5_32, "krb5_auth_con_setflags" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_auth_con_setflags",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_auth_con_init =
          GetProcAddress( hKRB5_32, "krb5_auth_con_init" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_auth_con_init",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_auth_con_free =
          GetProcAddress( hKRB5_32, "krb5_auth_con_free" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_auth_con_free",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_get_credentials =
          GetProcAddress( hKRB5_32, "krb5_get_credentials" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"Kerberos GetProcAddress failed","krb5_get_credentials",rc);
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_cred_contents =
          GetProcAddress( hKRB5_32, "krb5_free_cred_contents" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,
              "Kerberos GetProcAddress failed","krb5_free_cred_contents",rc);
        load_error = 1;
    }
    if (((FARPROC) p_krb5_sname_to_principal =
          GetProcAddress( hKRB5_32, "krb5_sname_to_principal" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,
              "Kerberos GetProcAddress failed","krb5_sname_to_principal",rc);
        load_error = 1;
    }
    if (((FARPROC) p_krb5_cc_default =
          GetProcAddress( hKRB5_32, "krb5_cc_default" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_cc_default",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_ap_rep_enc_part =
      GetProcAddress( hKRB5_32, "krb5_free_ap_rep_enc_part" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_free_ap_rep_enc_part",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_rd_rep =
          GetProcAddress( hKRB5_32, "krb5_rd_rep" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_rd_rep",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_init_context =
          GetProcAddress( hKRB5_32, "krb5_init_context" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_init_context",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_context =
          GetProcAddress( hKRB5_32, "krb5_free_context" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed","krb5_free_context",rc) ;
        load_error = 1;
    }

    if (((FARPROC) p_krb5_free_principal =
          GetProcAddress( hKRB5_32, "krb5_free_principal" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_free_principal",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_free_unparsed_name =
          GetProcAddress( hKRB5_32, "krb5_free_unparsed_name" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_free_unparsed_name",rc) ;
#ifdef COMMENT
        load_error = 1;
#endif /* COMMENT */
    }
    if (((FARPROC) p_krb5_fwd_tgt_creds =
          GetProcAddress( hKRB5_32, "krb5_fwd_tgt_creds" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_fwd_tgt_creds",rc) ;
        load_error = 1;
    }
    if (((FARPROC) p_krb5_auth_con_genaddrs =
          GetProcAddress( hKRB5_32, "krb5_auth_con_genaddrs" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111, "Kerberos GetProcAddress failed",
               "krb5_auth_con_genaddrs",rc) ;
        load_error = 1;
    }

        /* The DES functions are not required.  But if we do not have */
        /* them and do not have the CRYPT DLL we will not be able to  */
        /* perform mutual authentication.                             */
        if (((FARPROC) p_des_ecb_encrypt =
              GetProcAddress( hKRB5_32, "des_ecb_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","des_ecb_encrypt",rc);
        }
        if (((FARPROC) p_des_pcbc_encrypt =
              GetProcAddress( hKRB5_32, "des_pcbc_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","des_pcbc_encrypt",rc);
        }
        if (((FARPROC) p_des_new_random_key =
          GetProcAddress( hKRB5_32, "des_new_random_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","des_new_random_key",rc) ;
        }
        if (((FARPROC) p_des_key_sched =
              GetProcAddress( hKRB5_32, "des_key_sched" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed","des_key_sched",rc) ;
        }
        if (((FARPROC) p_des_set_random_generator_seed =
              GetProcAddress(hKRB5_32, "des_set_random_generator_seed" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "des_set_random_generator_seed",rc) ;
        }

        if (((FARPROC) p_krb5_get_host_realm=
              GetProcAddress(hKRB5_32,"krb5_get_host_realm")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_host_realm",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_free_host_realm=
              GetProcAddress(hKRB5_32,"krb5_free_host_realm")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_host_realm",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_get_in_tkt_with_keytab=
              GetProcAddress(hKRB5_32,"krb5_get_in_tkt_with_keytab")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_in_tkt_with_keytab",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_get_in_tkt_with_password=
             GetProcAddress(hKRB5_32,"krb5_get_in_tkt_with_password")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_in_tkt_with_password",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_read_password=
              GetProcAddress( hKRB5_32, "krb5_read_password" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_read_password",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_build_principal_ext=
              GetProcAddress( hKRB5_32, "krb5_build_principal_ext" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_build_principal_ext",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_unparse_name=
              GetProcAddress( hKRB5_32, "krb5_unparse_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_unparse_name",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_parse_name=
              GetProcAddress( hKRB5_32, "krb5_parse_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_parse_name",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_cc_resolve=
              GetProcAddress( hKRB5_32, "krb5_cc_resolve" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_resolve",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_cc_default_name=
              GetProcAddress( hKRB5_32, "krb5_cc_default_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_default_name",rc) ;
        }
        if (((FARPROC) p_krb5_string_to_timestamp=
              GetProcAddress( hKRB5_32, "krb5_string_to_timestamp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_string_to_timestamp",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_kt_resolve=
              GetProcAddress( hKRB5_32, "krb5_kt_resolve" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_resolve",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_string_to_deltat=
              GetProcAddress( hKRB5_32, "krb5_string_to_deltat" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_string_to_deltat",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_timeofday=
              GetProcAddress( hKRB5_32, "krb5_timeofday" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_timeofday",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_get_credentials_renew=
             GetProcAddress(hKRB5_32, "krb5_get_credentials_renew" )) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_credentials_renew",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_get_credentials_validate=
             GetProcAddress(hKRB5_32,"krb5_get_credentials_validate")) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_credentials_validate",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_copy_principal=
              GetProcAddress( hKRB5_32, "krb5_copy_principal" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_copy_principal",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_timestamp_to_sfstring=
              GetProcAddress(hKRB5_32,"krb5_timestamp_to_sfstring" )) == NULL)
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_timestamp_to_sfstring",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_kt_default=
              GetProcAddress( hKRB5_32, "krb5_kt_default" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_default",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_free_ticket=
              GetProcAddress( hKRB5_32, "krb5_free_ticket" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_ticket",rc) ;
            load_error = 1;
        }

#ifdef CHECKADDRS
        if (((FARPROC) p_krb5_os_localaddr=
              GetProcAddress( hKRB5_32, "krb5_os_localaddr" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_os_localaddr",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_address_search=
              GetProcAddress( hKRB5_32, "krb5_address_search" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_address_search",rc) ;

            /* This is not a fatal error since we expect that most builds */
            /* of krb5 dll will have an error in the export list that     */
            /* prevents this function from being found.  Therefore a      */
            /* built in version is used if it is missing                  */
        }
        if (((FARPROC) p_krb5_free_addresses=
              GetProcAddress( hKRB5_32, "krb5_free_addresses" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_addresses",rc) ;
            load_error = 1;
        }

#endif /* CHECKADDRS */
        if (((FARPROC) p_krb5_auth_con_getremotesubkey=
              GetProcAddress( hKRB5_32, "krb5_auth_con_getremotesubkey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_getremotesubkey",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_auth_con_getkey=
              GetProcAddress( hKRB5_32, "krb5_auth_con_getkey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_getkey",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_auth_con_getauthenticator=
              GetProcAddress( hKRB5_32, "krb5_auth_con_getauthenticator" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_getauthenticator",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_auth_con_setrcache=
              GetProcAddress( hKRB5_32, "krb5_auth_con_setrcache" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_setrcache",rc) ;
        }
        if (((FARPROC) p_krb5_get_server_rcache=
              GetProcAddress( hKRB5_32, "krb5_get_server_rcache" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_server_rcache",rc) ;
        }
        if (((FARPROC) p_krb5_auth_con_getrcache=
              GetProcAddress( hKRB5_32, "krb5_auth_con_getrcache" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_getrcache",rc) ;
        }
        if (((FARPROC) p_krb5_free_tgt_creds=
              GetProcAddress( hKRB5_32, "krb5_free_tgt_creds" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_tgt_creds",rc) ;
        }
        if (((FARPROC) p_krb5_rd_cred=
              GetProcAddress( hKRB5_32, "krb5_rd_cred" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_rd_cred",rc) ;
        }

        if (((FARPROC) p_krb5_c_enctype_compare=
              GetProcAddress( hKRB5_32, "krb5_c_enctype_compare" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_enctype_compare",rc) ;
        }
        if (((FARPROC) p_krb5_free_error=
              GetProcAddress( hKRB5_32, "krb5_free_error" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_error",rc) ;
        }
        if (((FARPROC) p_krb5_sendauth=
              GetProcAddress( hKRB5_32, "krb5_sendauth" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_sendauth",rc) ;
        }
        if (((FARPROC) p_krb5_process_key=
              GetProcAddress( hKRB5_32, "krb5_process_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_process_key",rc) ;
        }
        if (((FARPROC) p_krb5_use_enctype=
              GetProcAddress( hKRB5_32, "krb5_use_enctype" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_use_enctype",rc) ;
        }
        if (((FARPROC) p_krb5_encrypt=
              GetProcAddress( hKRB5_32, "krb5_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_encrypt",rc) ;
        }
        if (((FARPROC) p_krb5_decrypt=
              GetProcAddress( hKRB5_32, "krb5_decrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_decrypt",rc) ;
        }
        if (((FARPROC) p_krb5_encrypt_size=
              GetProcAddress( hKRB5_32, "krb5_encrypt_size" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_encrypt_size",rc) ;
        }
        if (((FARPROC) p_krb5_kuserok=
              GetProcAddress( hKRB5_32, "krb5_kuserok" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kuserok",rc) ;
        }
        if (((FARPROC) p_krb5_appdefault_boolean =
              GetProcAddress( hKRB5_32, "krb5_appdefault_boolean" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_appdefault_boolean",rc) ;
            /* NRL only */
        }
        if (((FARPROC) p_krb5_appdefault_string =
              GetProcAddress( hKRB5_32, "krb5_appdefault_string" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_appdefault_string",rc) ;
            /* NRL only */
        }
        if (((FARPROC) p_krb5_get_renewed_creds =
              GetProcAddress( hKRB5_32, "krb5_get_renewed_creds" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_renewed_creds",rc) ;
            /* Not in NRL or NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_validated_creds =
              GetProcAddress( hKRB5_32, "krb5_get_validated_creds" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_validated_creds",rc) ;
            /* Not in NRL or NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_password =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_password" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_password",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_address_list =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_address_list" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_address_list",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_tkt_life =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_tkt_life" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_tkt_life",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_forwardable =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_forwardable" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_forwardable",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_proxiable =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_proxiable" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_proxiable",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_renew_life =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_renew_life" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_renew_life",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_init =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_init" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_init",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_etype_list =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_etype_list" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_etype_list",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_preauth_list =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_preauth_list" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_preauth_list",rc) ;
            /* Not in NT-ALPHA-2 */
        }
        if (((FARPROC) p_krb5_get_init_creds_opt_set_salt =
              GetProcAddress( hKRB5_32, "krb5_get_init_creds_opt_set_salt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_init_creds_opt_set_salt",rc) ;
            /* Not in NT-ALPHA-2 */
        }

        if (((FARPROC) p_krb5_rd_safe =
              GetProcAddress( hKRB5_32, "krb5_rd_safe" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_rd_safe",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_mk_safe =
              GetProcAddress( hKRB5_32, "krb5_mk_safe" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_mk_safe",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_rd_priv =
              GetProcAddress( hKRB5_32, "krb5_rd_priv" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_rd_priv",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_mk_priv =
              GetProcAddress( hKRB5_32, "krb5_mk_priv" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_mk_priv",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_auth_con_setuseruserkey =
              GetProcAddress( hKRB5_32, "krb5_auth_con_setuseruserkey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_auth_con_setuseruserkey",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_krb5_get_profile =
              GetProcAddress( hKRB5_32, "krb5_get_profile" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_get_profile",rc) ;
        }

        if (((FARPROC) p_krb5_free_keyblock_contents =
              GetProcAddress( hKRB5_32, "krb5_free_keyblock_contents" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_free_keyblock_contents",rc) ;
        }
        if (((FARPROC) p_krb5_c_encrypt =
              GetProcAddress( hKRB5_32, "krb5_c_encrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_encrypt",rc) ;
        }
        if (((FARPROC) p_krb5_c_decrypt =
              GetProcAddress( hKRB5_32, "krb5_c_decrypt" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_decrypt",rc) ;
        }
        if (((FARPROC) p_krb5_c_make_random_key =
              GetProcAddress( hKRB5_32, "krb5_c_make_random_key" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_make_random_key",rc) ;
        }
        if (((FARPROC) p_krb5_c_random_seed =
              GetProcAddress( hKRB5_32, "krb5_c_random_seed" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_random_seed",rc) ;
        }
        if (((FARPROC) p_krb5_c_block_size =
              GetProcAddress( hKRB5_32, "krb5_c_block_size" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_block_size",rc) ;
        }
        if (((FARPROC) p_krb5_c_encrypt_length =
              GetProcAddress( hKRB5_32, "krb5_c_encrypt_length" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_c_encrypt_length",rc) ;
        }
        if (((FARPROC) p_krb5_kt_default_name =
              GetProcAddress( hKRB5_32, "krb5_kt_default_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_default_name",rc);
        }

        if (((FARPROC) p_krb5_cc_get_principal =
              GetProcAddress( hKRB5_32, "krb5_cc_get_principal" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_get_principal",rc);
        }
        if (((FARPROC) p_krb5_cc_store_cred =
              GetProcAddress( hKRB5_32, "krb5_cc_store_cred" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_store_cred",rc);
        }
        if (((FARPROC) p_krb5_cc_initialize =
              GetProcAddress( hKRB5_32, "krb5_cc_initialize" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_initialize",rc);
        }
        if (((FARPROC) p_krb5_cc_destroy =
              GetProcAddress( hKRB5_32, "krb5_cc_destroy" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_destroy",rc);
        }
        if (((FARPROC) p_krb5_cc_end_seq_get =
              GetProcAddress( hKRB5_32, "krb5_cc_end_seq_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_end_seq_get",rc);
        }
        if (((FARPROC) p_krb5_cc_next_cred =
              GetProcAddress( hKRB5_32, "krb5_cc_next_cred" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_next_cred",rc);
        }
        if (((FARPROC) p_krb5_cc_start_seq_get =
              GetProcAddress( hKRB5_32, "krb5_cc_start_seq_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_start_seq_get",rc);
        }
        if (((FARPROC) p_krb5_cc_get_type =
              GetProcAddress( hKRB5_32, "krb5_cc_get_type" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_get_type",rc);
        }
        if (((FARPROC) p_krb5_cc_get_name =
              GetProcAddress( hKRB5_32, "krb5_cc_get_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_get_name",rc);
        }
        if (((FARPROC) p_krb5_cc_set_flags =
              GetProcAddress( hKRB5_32, "krb5_cc_set_flags" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_set_flags",rc);
        }
        if (((FARPROC) p_krb5_cc_close =
              GetProcAddress( hKRB5_32, "krb5_cc_close" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_cc_close",rc);
        }

        if (((FARPROC) p_krb5_kt_get_type =
              GetProcAddress( hKRB5_32, "krb5_kt_get_type" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_get_type",rc);
        }
        if (((FARPROC) p_krb5_kt_get_name =
              GetProcAddress( hKRB5_32, "krb5_kt_get_name" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_get_name",rc);
        }
        if (((FARPROC) p_krb5_kt_close =
              GetProcAddress( hKRB5_32, "krb5_kt_close" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_close",rc);
        }
        if (((FARPROC) p_krb5_kt_get_entry =
              GetProcAddress( hKRB5_32, "krb5_kt_get_entry" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_get_entry",rc);
        }
        if (((FARPROC) p_krb5_kt_start_seq_get =
              GetProcAddress( hKRB5_32, "krb5_kt_start_seq_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_start_seq_get",rc);
        }
        if (((FARPROC) p_krb5_kt_next_entry =
              GetProcAddress( hKRB5_32, "krb5_kt_next_entry" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_next_entry",rc);
        }
        if (((FARPROC) p_krb5_kt_end_seq_get =
              GetProcAddress( hKRB5_32, "krb5_kt_end_seq_get" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_kt_end_seq_get",rc);
        }
        if (((FARPROC) p_krb5_build_principal =
              GetProcAddress( hKRB5_32, "krb5_build_principal" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "krb5_build_principal",rc);
        }

        if ( load_error ) {
            ck_krb5_loaddll_eh();
            return 0;
        }
    }

    hPROFILE = LoadLibrary("XPPROF32") ;
    if ( hPROFILE ) {
        if (((FARPROC) p_profile_get_relation_names =
              GetProcAddress( hPROFILE, "profile_get_relation_names" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "profile_get_relation_names",rc) ;
        }
        if (((FARPROC) p_profile_get_subsection_names =
              GetProcAddress( hPROFILE, "profile_get_subsection_names" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "Kerberos GetProcAddress failed",
                   "profile_get_subsection_names",rc) ;
        }
    }
#else /* NT */
    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen("KRB5_32") + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,"KRB5_32");         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hKRB5_32);
    if (rc) {
        debug(F111, "Kerberos V LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), "KRB5_32", &hKRB5_32);
    }
    if (rc) {
        debug(F111, "Kerberos V LoadLibrary failed",fail,rc) ;
    } else {
        exe_path = GetLoadPath();
        len = get_dir_len(exe_path);
        if ( len + strlen("COMERR32") + 4 > sizeof(path) )
            return(0);
        sprintf(path, "%.*s%s.DLL", len, exe_path,"COMERR32");         /* safe */
        rc = DosLoadModule(fail, sizeof(fail), path, &hCOMERR32);
        if (rc) {
            debug(F111, "Kerberos V Common Error LoadLibrary failed",fail,rc) ;
            rc = DosLoadModule(fail, sizeof(fail), "COMERR32", &hCOMERR32);
        }
        if (rc) {
            debug(F111, "Kerberos V Common Error LoadLibrary failed",fail,rc) ;
        } else {
            exe_path = GetLoadPath();
            len = get_dir_len(exe_path);
            if ( len + strlen("EMX") + 4 > sizeof(path) )
                return(0);
            sprintf(path, "%.*s%s.DLL", len, exe_path,"EMX");         /* safe */
            rc = DosLoadModule(fail, sizeof(fail), path, &hEMX);
            if (rc) {
                debug(F111, "EMX LoadLibrary failed",fail,rc) ;
                rc = DosLoadModule(fail, sizeof(fail), "EMX", &hEMX);
            }

            if (rc) {
                debug(F111, "EMX LoadLibrary failed",fail,rc) ;
            } else {
                if (rc = DosQueryProcAddr(hEMX,0,"emx_init",
                                          (PFN*)&p_emx_init))
                {
                    debug(F111,"EMX GetProcAddress failed","emx_init",rc);
                } else {
                    debug(F100,"calling EMX init routines","",0);
                    p_emx_init();
                }
            }
        }
    }
    if (rc) {
        ck_krb5_loaddll_eh();
    } else {
        debug(F100,"Kerberos V support provided by MIT","",0);
        if (rc = DosQueryProcAddr(hCOMERR32,0,"com_err",
                                   (PFN*)&p_com_err))
        {
            debug(F111,"Kerberos V GetProcAddress failed","com_err",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hCOMERR32,0,"com_err_va",
                                   (PFN*)&p_com_err_va))
        {
            debug(F111,"Kerberos V GetProcAddress failed","com_err_va",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hCOMERR32,0,"error_message",
                                   (PFN*)&p_error_message))
        {
            debug(F111,"Kerberos V GetProcAddress failed","error_message",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"decode_krb5_ticket",
                                   (PFN*)&p_decode_krb5_ticket))
        {
            debug(F111,"Kerberos V GetProcAddress failed","decode_krb5_ticket",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_decode_ticket",
                                   (PFN*)&p_cygnus_decode_krb5_ticket))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_decode_ticket",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_rep",
                                   (PFN*)&p_krb5_mk_rep))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_rep",rc);
            load_error = 1;
        }

        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_rep",
                                   (PFN*)&p_krb5_mk_rep))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_rep",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_authenticator",
                                   (PFN*)&p_krb5_free_authenticator))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_authenticator",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_verify_checksum",
                                   (PFN*)&p_krb5_verify_checksum))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_verify_checksum",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_rd_req",
                                   (PFN*)&p_krb5_rd_req))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_rd_req",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_creds",
                                   (PFN*)&p_krb5_free_creds))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_creds",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_data",
                                   (PFN*)&p_krb5_free_data))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_data",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_data_contents",
                                   (PFN*)&p_krb5_free_data_contents))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_data_contents",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_copy_keyblock",
                                   (PFN*)&p_krb5_copy_keyblock))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_copy_keyblock",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_keyblock",
                                   (PFN*)&p_krb5_free_keyblock))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_keyblock",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_getlocalsubkey",
                                   (PFN*)&p_krb5_auth_con_getlocalsubkey))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_getlocalsubkey",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_req_extended",
                                   (PFN*)&p_krb5_mk_req_extended))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_req_extended",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_req",
                                   (PFN*)&p_krb5_mk_req))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_req",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_setflags",
                                   (PFN*)&p_krb5_auth_con_setflags))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_setflags",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_init",
                                   (PFN*)&p_krb5_auth_con_init))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_init",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_free",
                                   (PFN*)&p_krb5_auth_con_free))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_free",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_credentials",
                                   (PFN*)&p_krb5_get_credentials))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_credentials",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_cred_contents",
                                   (PFN*)&p_krb5_free_cred_contents))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_cred_contents",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_sname_to_principal",
                                   (PFN*)&p_krb5_sname_to_principal))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_sname_to_principal",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_default",
                                   (PFN*)&p_krb5_cc_default))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_default",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_ap_rep_enc_part",
                                   (PFN*)&p_krb5_free_ap_rep_enc_part))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_ap_rep_enc_part",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_rd_rep",
                                   (PFN*)&p_krb5_rd_rep))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_rd_rep",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_init_context",
                                   (PFN*)&p_krb5_init_context))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_init_context",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_context",
                                   (PFN*)&p_krb5_free_context))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_context",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_principal",
                                   (PFN*)&p_krb5_free_principal))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_principal",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_unparsed_name",
                                   (PFN*)&p_krb5_free_unparsed_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_unparsed_name",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_fwd_tgt_creds",
                                   (PFN*)&p_krb5_fwd_tgt_creds))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_fwd_tgt_creds",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_genaddrs",
                                   (PFN*)&p_krb5_auth_con_genaddrs))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_genaddrs",rc);
            load_error = 1;
        }
        /* The DES functions are not required.  But if we do not have */
        /* them and do not have the CRYPT DLL we will not be able to  */
        /* perform mutual authentication.                             */
        if (rc = DosQueryProcAddr(hKRB5_32,0,"des_ecb_encrypt",
                                   (PFN*)&p_des_ecb_encrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","des_ecb_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"des_pcbc_encrypt",
                                   (PFN*)&p_des_pcbc_encrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","des_pcbc_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"des_new_random_key",
                                   (PFN*)&p_des_new_random_key))
        {
            debug(F111,"Kerberos V GetProcAddress failed","des_new_random_key",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"des_key_sched",
                                   (PFN*)&p_des_key_sched))
        {
            debug(F111,"Kerberos V GetProcAddress failed","des_key_sched",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"des_set_random_generator_seed",
                                   (PFN*)&p_des_set_random_generator_seed))
        {
            debug(F111,"Kerberos V GetProcAddress failed","des_set_random_generator_seed",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_host_realm",
                                   (PFN*)&p_krb5_get_host_realm))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_host_realm",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_host_realm",
                                   (PFN*)&p_krb5_free_host_realm))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_host_realm",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_in_tkt_with_keytab",
                                   (PFN*)&p_krb5_get_in_tkt_with_keytab))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_in_tkt_with_keytab",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_in_tkt_with_password",
                                   (PFN*)&p_krb5_get_in_tkt_with_password))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_in_tkt_with_password",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_read_password",
                                   (PFN*)&p_krb5_read_password))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_read_password",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_build_principal_ext",
                                   (PFN*)&p_krb5_build_principal_ext))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_build_principal_ext",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_unparse_name",
                                   (PFN*)&p_krb5_unparse_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_unparse_name",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_parse_name",
                                   (PFN*)&p_krb5_parse_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_parse_name",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_resolve",
                                   (PFN*)&p_krb5_cc_resolve))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_resolve",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_default_name",
                                   (PFN*)&p_krb5_cc_default_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_default_name",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_string_to_timestamp",
                                   (PFN*)&p_krb5_string_to_timestamp))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_string_to_timestamp",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_resolve",
                                   (PFN*)&p_krb5_kt_resolve))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_resolve",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_string_to_deltat",
                                   (PFN*)&p_krb5_string_to_deltat))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_string_to_deltat",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_timeofday",
                                   (PFN*)&p_krb5_timeofday))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_timeofday",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_credentials_renew",
                                   (PFN*)&p_krb5_get_credentials_renew))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_credentials_renew",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_credentials_validate",
                                   (PFN*)&p_krb5_get_credentials_validate))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_credentials_validate",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_copy_principal",
                                   (PFN*)&p_krb5_copy_principal))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_copy_principal",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_timestamp_to_sfstring",
                                   (PFN*)&p_krb5_timestamp_to_sfstring))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_timestamp_to_sfstring",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_default",
                                   (PFN*)&p_krb5_kt_default))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_default",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_ticket",
                                   (PFN*)&p_krb5_free_ticket))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_ticket",rc);
            load_error = 1;
        }
#ifdef CHECKADDRS
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_os_localaddr",
                                   (PFN*)&p_krb5_os_localaddr))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_os_localaddr",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_address_search",
                                   (PFN*)&p_krb5_address_search))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_address_search",rc);
            /* This is not a fatal error since we expect that most builds */
            /* of krb5 dll will have an error in the export list that     */
            /* prevents this function from being found.  Therefore a      */
            /* built in version is used if it is missing                  */
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_addresses",
                                   (PFN*)&p_krb5_free_addresses))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_addresses",rc);
            load_error = 1;
        }
#endif /* CHECKADDRS */
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_getremotesubkey",
                                   (PFN*)&p_krb5_auth_con_getremotesubkey))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_getremotesubkey",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_getkey",
                                   (PFN*)&p_krb5_auth_con_getkey))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_getkey",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_getauthenticator",
                                   (PFN*)&p_krb5_auth_con_getauthenticator))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_getauthenticator",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_setrcache",
                                   (PFN*)&p_krb5_auth_con_setrcache))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_setrcache",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_server_rcache",
                                   (PFN*)&p_krb5_get_server_rcache))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_server_rcache",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_getrcache",
                                   (PFN*)&p_krb5_auth_con_getrcache))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_getrcache",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_tgt_creds",
                                   (PFN*)&p_krb5_free_tgt_creds))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_tgt_creds",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_rd_cred",
                                   (PFN*)&p_krb5_rd_cred))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_rd_cred",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_enctype_compare",
                                   (PFN*)&p_krb5_c_enctype_compare))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_enctype_compare",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_error",
                                   (PFN*)&p_krb5_free_error))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_error",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_sendauth",
                                   (PFN*)&p_krb5_sendauth))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_sendauth",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_process_key",
                                   (PFN*)&p_krb5_process_key))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_process_key",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_use_enctype",
                                   (PFN*)&p_krb5_use_enctype))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_use_enctype",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_encrypt",
                                   (PFN*)&p_krb5_encrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_decrypt",
                                   (PFN*)&p_krb5_decrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_decrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_encrypt_size",
                                   (PFN*)&p_krb5_encrypt_size))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_encrypt_size",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kuserok",
                                   (PFN*)&p_krb5_kuserok))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kuserok",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_appdefault_boolean",
                                   (PFN*)&p_krb5_appdefault_boolean))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_appdefault_boolean",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_appdefault_string",
                                   (PFN*)&p_krb5_appdefault_string))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_appdefault_string",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_renewed_creds",
                                   (PFN*)&p_krb5_get_renewed_creds))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_renewed_creds",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_validated_creds",
                                   (PFN*)&p_krb5_get_validated_creds))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_validated_creds",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_password",
                                   (PFN*)&p_krb5_get_init_creds_password))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_password",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_address_list",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_address_list))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_address_list",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_tkt_life",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_tkt_life))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_tkt_life",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_forwardable",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_forwardable))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_forwardable",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_proxiable",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_proxiable))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_proxiable",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_renew_life",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_renew_life))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_renew_life",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_init",
                                   (PFN*)&p_krb5_get_init_creds_opt_init))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_init",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_etype_list",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_etype_list))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_etype_list",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_preauth_list",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_preauth_list))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_preauth_list",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_init_creds_opt_set_salt",
                                   (PFN*)&p_krb5_get_init_creds_opt_set_salt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_init_creds_opt_set_salt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_rd_safe",
                                   (PFN*)&p_krb5_rd_safe))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_rd_safe",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_safe",
                                   (PFN*)&p_krb5_mk_safe))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_safe",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_rd_priv",
                                   (PFN*)&p_krb5_rd_priv))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_rd_priv",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_mk_priv",
                                   (PFN*)&p_krb5_mk_priv))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_mk_priv",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_auth_con_setuseruserkey",
                                   (PFN*)&p_krb5_auth_con_setuseruserkey))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_auth_con_setuseruserkey",rc);
            load_error = 1;
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_get_profile",
                                   (PFN*)&p_krb5_get_profile))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_get_profile",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_free_keyblock_contents",
                                   (PFN*)&p_krb5_free_keyblock_contents))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_free_keyblock_contents",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_encrypt",
                                   (PFN*)&p_krb5_c_encrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_encrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_decrypt",
                                   (PFN*)&p_krb5_c_decrypt))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_decrypt",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_make_random_key",
                                   (PFN*)&p_krb5_c_make_random_key))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_make_random_key",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_random_seed",
                                   (PFN*)&p_krb5_c_random_seed))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_random_seed",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_block_size",
                                   (PFN*)&p_krb5_c_block_size))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_block_size",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_c_encrypt_length",
                                   (PFN*)&p_krb5_c_encrypt_length))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_c_encrypt_length",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_default_name",
                                   (PFN*)&p_krb5_kt_default_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_default_name",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_get_principal",
                                   (PFN*)&p_krb5_cc_get_principal))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_get_principal",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_store_cred",
                                   (PFN*)&p_krb5_cc_store_cred))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_store_cred",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_initialize",
                                   (PFN*)&p_krb5_cc_initialize))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_initialize",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_destroy",
                                   (PFN*)&p_krb5_cc_destroy))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_destroy",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_end_seq_get",
                                   (PFN*)&p_krb5_cc_end_seq_get))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_end_seq_get",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_next_cred",
                                   (PFN*)&p_krb5_cc_next_cred))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_next_cred",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_start_seq_get",
                                   (PFN*)&p_krb5_cc_start_seq_get))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_start_seq_get",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_get_type",
                                   (PFN*)&p_krb5_cc_get_type))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_get_type",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_get_name",
                                   (PFN*)&p_krb5_cc_get_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_get_name",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_set_flags",
                                   (PFN*)&p_krb5_cc_set_flags))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_set_flags",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_cc_close",
                                   (PFN*)&p_krb5_cc_close))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_cc_close",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_get_type",
                                   (PFN*)&p_krb5_kt_get_type))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_get_type",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_get_name",
                                   (PFN*)&p_krb5_kt_get_name))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_get_name",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_close",
                                   (PFN*)&p_krb5_kt_close))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_close",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_get_entry",
                                   (PFN*)&p_krb5_kt_get_entry))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_get_entry",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_start_seq_get",
                                   (PFN*)&p_krb5_kt_start_seq_get))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_start_seq_get",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_next_entry",
                                   (PFN*)&p_krb5_kt_next_entry))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_next_entry",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_kt_end_seq_get",
                                   (PFN*)&p_krb5_kt_end_seq_get))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_kt_end_seq_get",rc);
        }
        if (rc = DosQueryProcAddr(hKRB5_32,0,"krb5_build_principal",
                                   (PFN*)&p_krb5_build_principal))
        {
            debug(F111,"Kerberos V GetProcAddress failed","krb5_build_principal",rc);
        }
    }

    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen("XPPROF32") + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,"XPPROF32");         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hPROFILE);
    if (rc) {
        debug(F111, "Kerberos V Profile LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), "XPPROF32", &hPROFILE);
    }

    if ( rc ) {
        debug(F111, "Kerberos V Profile LoadLibrary failed",fail,rc) ;
    } else {
        if (rc = DosQueryProcAddr(hPROFILE,0,"profile_get_relation_names",
                                   (PFN*)&p_profile_get_relation_names))
        {
            debug(F111,"Kerberos V GetProcAddress failed","profile_get_relation_names",rc);
        }
        if (rc = DosQueryProcAddr(hPROFILE,0,"profile_get_subsection_names",
                                   (PFN*)&p_profile_get_subsection_names))
        {
            debug(F111,"Kerberos V GetProcAddress failed","profile_get_subsection_names",rc);
        }
    }
#endif /* NT */

    if ( deblog ) {
        if ( hKRB5_32 ) {
            if ( cygnus )
                printf("Cygnus Kerberos 5 available\n");
            else
                printf("MIT Kerberos 5 available\n");
        }
        if ( hKRB524 )
            printf("MIT Kerberos 524 available\n");
    }

    /* Attempt to load a krb5_principal_to_userid() function */
#ifdef NT
    hKRB52UID = LoadLibrary("KRB52UID");
    if ( hKRB52UID ) {
        if (((FARPROC) p_k95_k5_principal_to_localname =
              GetProcAddress( hKRB52UID, "krb5_principal_to_localname" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"KRB52UID GetProcAddress failed",
                   "krb5_principal_to_localname",rc);
        }
        if (((FARPROC) p_k95_k5_userok =
              GetProcAddress( hKRB52UID, "krb5_kuserok" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111,"KRB52UID GetProcAddress failed","krb5_kuserok",rc);
        }
    }

    /* Initialize Kerberos 5 ticket options based upon MIT Leash selections */
    hLEASH = LoadLibrary("LEASHW32");
    if ( hLEASH )
    {
        DWORD (* pLeash_get_default_lifetime)(void);
        DWORD (* pLeash_get_default_forwardable)(void);
        DWORD (* pLeash_get_default_proxiable)(void);
        DWORD (* pLeash_get_default_renewable)(void);
        DWORD (* pLeash_get_default_renew_till)(void);
        DWORD (* pLeash_get_default_noaddresses)(void);
        DWORD (* pLeash_get_default_publicip)(void);
        DWORD (* pLeash_get_default_use_krb4)(void);

        (FARPROC) pLeash_get_default_lifetime =
            GetProcAddress(hLEASH, "Leash_get_default_lifetime");
        (FARPROC) pLeash_get_default_forwardable =
            GetProcAddress(hLEASH, "Leash_get_default_forwardable");
        (FARPROC) pLeash_get_default_proxiable =
            GetProcAddress(hLEASH, "Leash_get_default_proxiable");
        (FARPROC) pLeash_get_default_renewable =
            GetProcAddress(hLEASH, "Leash_get_default_renewable");
        (FARPROC) pLeash_get_default_renew_till =
            GetProcAddress(hLEASH, "Leash_get_default_renew_till");
        (FARPROC) pLeash_get_default_noaddresses =
            GetProcAddress(hLEASH, "Leash_get_default_noaddresses");
        (FARPROC) pLeash_get_default_publicip =
            GetProcAddress(hLEASH, "Leash_get_default_publicip");
        (FARPROC) pLeash_get_default_use_krb4 =
            GetProcAddress(hLEASH, "Leash_get_default_use_krb4");

        if ( pLeash_get_default_lifetime )
            krb5_d_lifetime =    pLeash_get_default_lifetime();
        if ( pLeash_get_default_forwardable )
            krb5_d_forwardable = pLeash_get_default_forwardable();
        if ( pLeash_get_default_proxiable )
            krb5_d_proxiable   = pLeash_get_default_proxiable();
        if ( pLeash_get_default_renewable && pLeash_get_default_renew_till)
            krb5_d_renewable   = pLeash_get_default_renewable() ? pLeash_get_default_renew_till() : 0;
        if ( pLeash_get_default_noaddresses )
            krb5_d_no_addresses = pLeash_get_default_noaddresses();
        if ( pLeash_get_default_publicip && pLeash_get_default_publicip() )
        {
            DWORD dwValue = pLeash_get_default_publicip();
            int i;
            char buf[17];
            for ( i = 0;
                  i < KRB5_NUM_OF_ADDRS && krb5_d_addrs[i];
                  i++) {
                if (krb5_d_addrs[i])
                    free(krb5_d_addrs[i]);
                krb5_d_addrs[i] = NULL;
            }

            sprintf(buf,"%d.%d.%d.%d",
                     (dwValue&0xFF000000)>>24,
                     (dwValue&0x00FF0000)>>16,
                     (dwValue&0x0000FF00)>>8,
                     (dwValue&0x000000FF));
            makestr(&krb5_d_addrs[0],buf);
        }
        if ( pLeash_get_default_use_krb4 )
            krb5_d_getk4 = pLeash_get_default_use_krb4();
        FreeLibrary(hLEASH);
    }
#else /* NT */
    dllname = "KRB52UID";
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,dllname);
    rc = DosLoadModule(fail, sizeof(fail), path, &hKRB52UID);
    if (rc) {
        debug(F111, "KRB52UID LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hKRB52UID);
    }
    if ( rc ) {
        debug(F111, "KRB52UID LoadLibrary failed",fail,rc) ;
    } else {
        if (rc = DosQueryProcAddr( hKRB52UID, 0, "krb5_principal_to_userid",
                                  (PFN*) &p_k95_k5_principal_to_localname))
        {
            debug(F111,"KRB52UID GetProcAddress failed",
                  "krb5_principal_to_userid",rc);
        }
        if (rc = DosQueryProcAddr( hKRB52UID, 0, "krb5_userok",
                                  (PFN*) &p_k95_k5_userok))
        {
            debug(F111,"KRB52UID GetProcAddress failed",
                  "krb5_userok",rc);
        }
    }
#endif /* NT */

    return(1);
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}

#ifdef CK_SRP
#ifdef SRPDLL
static HINSTANCE hSRP = NULL;
static int srp_dll_loaded = 0;
#endif /* SRPDLL */

static int
ck_srp_loaddll( void )
{
#ifdef SRPDLL
    ULONG rc = 0 ;
    int load_error = 0;
#ifdef NT
    hSRP = LoadLibrary("SRP");
    if ( !hSRP ) {
        rc = GetLastError() ;
        debug(F111, "SRP LoadLibrary failed","SRP",rc) ;
    } else {
        if (((FARPROC) p_t_clientresponse =
              GetProcAddress( hSRP, "t_clientresponse" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientresponse",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_t_clientgetkey =
              GetProcAddress( hSRP, "t_clientgetkey" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientgetkey",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_t_clientpasswd =
              GetProcAddress( hSRP, "t_clientpasswd" )) == NULL )
        {                                                                      x2
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientpasswd",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_t_clientgenexp =
              GetProcAddress( hSRP, "t_clientgenexp" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientgenexp",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_t_clientopen =
              GetProcAddress( hSRP, "t_clientopen" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientopen",rc) ;
            load_error = 1;
        }
        if (((FARPROC) p_t_clientverify =
              GetProcAddress( hSRP, "t_clientverify" )) == NULL )
        {
            rc = GetLastError() ;
            debug(F111, "SRP GetProcAddress failed","t_clientverify",rc) ;
            load_error = 1;
        }

        if ( load_error ) {
            ck_srp_loaddll_eh();
            return 0;
        }
        if ( deblog )
            printf("SRP available\n");
    }
#else /* NT */
    Not yet implemented
#endif /* NT */
#else /* SRPDLL */
    if ( deblog )
        printf("SRP available\n");
#endif /* SRPDLL */
    return(1);
}

#ifdef SRPDLL
static void
ck_srp_loaddll_eh( void )
{
    if ( hSRP ) {
#ifdef NT
        FreeLibrary(hSRP);
        hSRP = NULL;
#else /* NT */
        DosFreeModule(hSRP);
        hSRP = 0;
#endif /* NT */
    }
#ifdef SRP
    p_t_clientresponse=NULL;
    p_t_clientgetkey=NULL;
    p_t_clientpasswd=NULL;
    p_t_clientgenexp=NULL;
    p_t_clientopen=NULL;
    p_t_clientverify=NULL;
#endif /* SRP */
}
#endif /* SRPDLL */

int
ck_srp_unloaddll( void )
{
#ifdef SRPDLL
    if ( !srp_dll_loaded )
        return(1);

    /* unload dlls */
    ck_srp_dll_loaddll_eh();

    /* success */
    srp_dll_loaded = 0;
#endif /* SRPDLL */
    return(1);
}

#include <krypto.h>

static struct _cipher_desc * (*p_cipher_getdescbyid)(unsigned char id)=NULL;
static unsigned char * (*p_cipher_getlist)()=NULL;
static struct _cipher_desc * (*p_cipher_getdescbyname)(char * name)=NULL;
static struct _hash_desc * (*p_hash_getdescbyid)(unsigned char id)=NULL;
static unsigned char * (*p_hash_getlist)()=NULL;
static struct _hash_desc * (*p_hash_getdescbyname)(char * name)=NULL;
static void (*p_krypto_delete)(krypto_context * cc)=NULL;
static krypto_context * (*p_krypto_new)( unsigned cid, unsigned hid,
                                         unsigned char *key, unsigned keylen,
                                         unsigned char *iv, unsigned ivlen,
                                         unsigned char *seq, unsigned state)=NULL;
static int (*p_hash_supported)(unsigned char * a, unsigned char b)=NULL;
static int (*p_cipher_supported)(unsigned char *a, unsigned char b)=NULL;
static int (*p_krypto_msg_safe)(krypto_context *cc, unsigned char *in,
                                 unsigned char * out, int len)=NULL;
static int (*p_krypto_msg_priv)(krypto_context *cc, unsigned char *in,
                                 unsigned char * out, int len)=NULL;

struct _cipher_desc *
ck_cipher_getdescbyid(unsigned char id)
{
    if ( p_cipher_getdescbyid )
        return(p_cipher_getdescbyid(id));
    else
        return(NULL);
}

unsigned char *
ck_cipher_getlist()
{
    if ( p_cipher_getlist )
        return(p_cipher_getlist());
    else
        return(NULL);
}

struct _cipher_desc *
ck_cipher_getdescbyname(char * name)
{
    if ( p_cipher_getdescbyname )
        return(p_cipher_getdescbyname(name));
    else
        return(NULL);
}

struct _hash_desc *
ck_hash_getdescbyid(unsigned char id)
{
    if ( p_hash_getdescbyid )
        return(p_hash_getdescbyid(id));
    return(NULL);
}

unsigned char *
ck_hash_getlist()
{
    if ( p_hash_getlist )
        return(p_hash_getlist());
    else
        return(NULL);
}

struct _hash_desc *
ck_hash_getdescbyname(char * name)
{
    if ( p_hash_getdescbyname )
        return(p_hash_getdescbyname(name));
    else
        return(NULL);
}

void
ck_krypto_delete(krypto_context * cc)
{
    if ( p_krypto_delete && cc)
        p_krypto_delete(cc);
}

krypto_context *
ck_krypto_new( unsigned cid, unsigned hid,
               unsigned char *key, unsigned keylen,
               unsigned char *iv, unsigned ivlen,
               unsigned char *seq, unsigned state)
{
    if ( p_krypto_new )
        return(p_krypto_new(cid, hid, key, keylen, iv, ivlen, seq, state));
    else
        return(NULL);
}

int
ck_hash_supported(unsigned char * a, unsigned char b)
{
    if ( p_hash_supported )
        return(p_hash_supported(a,b));
    else
        return(0);
}

int
ck_cipher_supported(unsigned char *a, unsigned char b)
{
    if ( p_cipher_supported )
        return(p_cipher_supported(a,b));
    else
        return(0);
}

int
ck_krypto_msg_safe(krypto_context *cc, unsigned char *in,
                    unsigned char * out, int len)
{
    if ( p_krypto_msg_safe )
        return(p_krypto_msg_safe(cc,in,out,len));
    else
        return(-1);
}

int
ck_krypto_msg_priv(krypto_context *cc, unsigned char *in,
                    unsigned char * out, int len)
{
    if ( p_krypto_msg_priv )
        return(p_krypto_msg_priv(cc,in,out,len));
    else
        return(-1);
}

static int krypto_loaded=0;
#ifdef NT
HINSTANCE hLIBKRYPTO = NULL;
#else
HMODULE hLIBKRYPTO = 0;
#endif /* NT */

static void
load_krypto_eh()
{
    if ( hLIBKRYPTO ) {
#ifdef NT
        FreeLibrary(hLIBKRYPTO);
        hLIBKRYPTO = NULL;
#else
        DosFreeModule(hLIBKRYPTO);
        hLIBKRYPTO = 0;
#endif /* NT */
    }

    p_cipher_getdescbyid   = NULL;
    p_cipher_getlist       = NULL;
    p_cipher_getdescbyname = NULL;
    p_hash_getdescbyid     = NULL;
    p_hash_getlist         = NULL;
    p_hash_getdescbyname   = NULL;
    p_krypto_delete        = NULL;
    p_krypto_new           = NULL;
    p_hash_supported       = NULL;
    p_cipher_supported     = NULL;
    p_krypto_msg_safe      = NULL;
    p_krypto_msg_priv      = NULL;

    krypto_loaded = 0;
}


static int
ck_krypto_loaddll()
{
    ULONG rc = 0;
    int load_error = 0;

#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "KRYPTO";
#endif /* OS2ONLY */

    if ( krypto_loaded )
        return(1);

#ifdef NT
    hLIBKRYPTO = LoadLibrary("KRYPTO");
    if ( !hLIBKRYPTO )
        return(0);

    if (((FARPROC) p_cipher_getdescbyid =
          GetProcAddress( hLIBKRYPTO, "cipher_getdescbyid" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_cipher_getlist =
          GetProcAddress( hLIBKRYPTO, "cipher_getlist" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_cipher_getdescbyname =
          GetProcAddress( hLIBKRYPTO, "cipher_getdescbyname" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_hash_getdescbyid =
          GetProcAddress( hLIBKRYPTO, "hash_getdescbyid" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_hash_getlist =
          GetProcAddress( hLIBKRYPTO, "hash_getlist" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_hash_getdescbyname =
          GetProcAddress( hLIBKRYPTO, "hash_getdescbyname" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_krypto_delete =
          GetProcAddress( hLIBKRYPTO, "krypto_delete" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_krypto_new =
          GetProcAddress( hLIBKRYPTO, "krypto_new" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_hash_supported =
          GetProcAddress( hLIBKRYPTO, "hash_supported" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_cipher_supported =
          GetProcAddress( hLIBKRYPTO, "cipher_supported" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_krypto_msg_safe =
          GetProcAddress( hLIBKRYPTO, "krypto_msg_safe" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if (((FARPROC) p_krypto_msg_priv =
          GetProcAddress( hLIBKRYPTO, "krypto_msg_priv" )) == NULL )
    {
        rc = GetLastError() ;
        load_error = 1;
    }
    if ( load_error ) {
        load_krypto_eh();
        return 0;
    }
    krypto_loaded = 1;
#else /* NT */
    return(0);
#endif /* NT */
    return(1);
}

static int
ck_krypto_unloaddll()
{
    if ( !krypto_loaded )
        return(1);

    load_krypto_eh();
    return(1);
}

int krypto_avail()
{
    return krypto_loaded;
}
#else
int krypto_avail()
{
    return 0;
}
#endif /* CK_SRP */

#ifdef KRB5
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_generic.h>

static unsigned long (KRB5_CALLCONV * p_gss_release_buffer)
    ( unsigned long *, gss_buffer_t )= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_seal)(unsigned long *, gss_ctx_id_t, int, int,
                           gss_buffer_t, int *, gss_buffer_t )= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_unseal)(unsigned long *,
                                                gss_ctx_id_t, gss_buffer_t,
                                                gss_buffer_t, int *, int * )= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_release_name)(unsigned long *,
                                                      gss_name_t *)= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_init_sec_context)
    (unsigned long *, gss_cred_id_t,
      gss_ctx_id_t *, gss_name_t,
      gss_OID, OM_uint32, OM_uint32,
      gss_channel_bindings_t,
      gss_buffer_t, gss_OID *,
      gss_buffer_t,
      unsigned long *,
      unsigned long * )= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_import_name)(unsigned long *,
                                                     gss_buffer_t, gss_OID,
                                                     gss_name_t * )= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_display_status)(unsigned long *,
                                                        unsigned long,
                                                        int, gss_OID,
                                                        unsigned long *,
                                                        gss_buffer_t)= NULL;
static unsigned long (KRB5_CALLCONV * p_gss_indicate_mechs)
    (OM_uint32 * minor_status, gss_OID_set * mech_set)= NULL;


OM_uint32 (KRB5_CALLCONV * p_gss_verify_mic)
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_buffer_t,               /* message_buffer */
            gss_buffer_t,               /* message_token */
            gss_qop_t *                 /* qop_state */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_release_cred)
(OM_uint32 *,           /* minor_status */
            gss_cred_id_t *             /* cred_handle */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_delete_sec_context)
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t *,             /* context_handle */
            gss_buffer_t                /* output_token */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_accept_sec_context)
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
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_acquire_cred)
(OM_uint32 *,           /* minor_status */
            gss_name_t,                 /* desired_name */
            OM_uint32,                  /* time_req */
            gss_OID_set,                /* desired_mechs */
            gss_cred_usage_t,           /* cred_usage */
            gss_cred_id_t *,    /* output_cred_handle */
            gss_OID_set *,              /* actual_mechs */
            OM_uint32 *         /* time_rec */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_get_mic)
(OM_uint32 *,           /* minor_status */
            gss_ctx_id_t,               /* context_handle */
            gss_qop_t,                  /* qop_req */
            gss_buffer_t,               /* message_buffer */
            gss_buffer_t                /* message_token */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_display_name)
(OM_uint32 *,           /* minor_status */
            gss_name_t,                 /* input_name */
            gss_buffer_t,               /* output_name_buffer */
            gss_OID *           /* output_name_type */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_test_oid_set_member)
(OM_uint32 *,           /* minor_status */
            gss_OID,                    /* member */
            gss_OID_set,                /* set */
            int *                       /* present */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_add_oid_set_member)
(OM_uint32 *,           /* minor_status */
            gss_OID,                    /* member_oid */
            gss_OID_set *               /* oid_set */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_create_empty_oid_set)
(OM_uint32 *,           /* minor_status */
            gss_OID_set *               /* oid_set */
           )=NULL;

OM_uint32 (KRB5_CALLCONV * p_gss_release_oid_set)
(OM_uint32 *,           /* minor_status */
            gss_OID_set *               /* set */
           )=NULL;


unsigned long
ck_gss_release_buffer( unsigned long * c, gss_buffer_t b)
{
    if ( p_gss_release_buffer )
        return(p_gss_release_buffer(c,b));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_seal(unsigned long * a, gss_ctx_id_t b, int c, int d,
             gss_buffer_t e, int * f, gss_buffer_t g)
{
    if ( p_gss_seal )
        return(p_gss_seal(a,b,c,d,e,f,g));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_unseal(unsigned long * a, gss_ctx_id_t b, gss_buffer_t c,
                          gss_buffer_t d, int * e, int * f )
{
    if ( p_gss_unseal )
        return(p_gss_unseal(a,b,c,d,e,f));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_release_name(unsigned long * a, gss_name_t * b)
{
    if ( p_gss_release_name )
        return(p_gss_release_name(a,b));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_init_sec_context(unsigned long * a, gss_cred_id_t b,
                                       gss_ctx_id_t * c, gss_name_t d,
                                       gss_OID e, OM_uint32 f, OM_uint32 g,
                                       gss_channel_bindings_t h,
                                       gss_buffer_t i, gss_OID * j,
                                       gss_buffer_t k,
                                       unsigned long * l,
                                       unsigned long * m)
{
    if ( p_gss_init_sec_context )
        return(p_gss_init_sec_context(a,b,c,d,e,f,g,h,i,j,k,l,m));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_import_name(unsigned long * a, gss_buffer_t b, gss_OID c,
                                  gss_name_t * d)
{
    if ( p_gss_import_name )
        return(p_gss_import_name(a,b,c,d));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_display_status(unsigned long * a, unsigned long b,
                                     int c, gss_OID d, unsigned long * e,
                                     gss_buffer_t f )
{
    if ( p_gss_display_status )
        return(p_gss_display_status(a,b,c,d,e,f));
    else
        return(GSS_S_UNAVAILABLE);
}

unsigned long
ck_gss_indicate_mechs(OM_uint32 * minor_status, gss_OID_set * mech_set)
{
    if ( p_gss_indicate_mechs )
        return(p_gss_indicate_mechs(minor_status,mech_set));
    else
        return(GSS_S_UNAVAILABLE);
}


OM_uint32
ck_gss_verify_mic(OM_uint32 * status,           /* minor_status */
            gss_ctx_id_t context,               /* context_handle */
            gss_buffer_t buf,               /* message_buffer */
            gss_buffer_t tok,               /* message_token */
            gss_qop_t *  qop               /* qop_state */
           )
{
    if ( p_gss_verify_mic )
        return(p_gss_verify_mic(status,context,buf,tok,qop));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_release_cred(OM_uint32 * status,           /* minor_status */
            gss_cred_id_t * cred             /* cred_handle */
           )
{
    if ( p_gss_release_cred )
        return(p_gss_release_cred(status,cred));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_delete_sec_context(OM_uint32 * status,           /* minor_status */
            gss_ctx_id_t * context,             /* context_handle */
            gss_buffer_t   tok             /* output_token */
           )
{
    if ( p_gss_delete_sec_context )
        return(p_gss_delete_sec_context(status,context,tok));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_accept_sec_context(OM_uint32 * status,           /* minor_status */
            gss_ctx_id_t * context,             /* context_handle */
            gss_cred_id_t  acceptor,              /* acceptor_cred_handle */
            gss_buffer_t   buf,               /* input_token_buffer */
            gss_channel_bindings_t binding,     /* input_chan_bindings */
            gss_name_t * name,               /* src_name */
            gss_OID * type,          /* mech_type */
            gss_buffer_t out,               /* output_token */
            OM_uint32 * ret,                /* ret_flags */
            OM_uint32 * time,                /* time_rec */
            gss_cred_id_t * delcred             /* delegated_cred_handle */
           )
{
    if ( p_gss_accept_sec_context )
        return(p_gss_accept_sec_context(status,context,acceptor,buf,binding,
                                         name, type, out, ret, time, delcred));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_acquire_cred(OM_uint32 * status,           /* minor_status */
            gss_name_t name,                 /* desired_name */
            OM_uint32 timereq,                  /* time_req */
            gss_OID_set dmechs,                /* desired_mechs */
            gss_cred_usage_t usage,           /* cred_usage */
            gss_cred_id_t * outcred,    /* output_cred_handle */
            gss_OID_set * amechs,              /* actual_mechs */
            OM_uint32 *   timerec      /* time_rec */
           )
{
    if ( p_gss_acquire_cred )
        return(p_gss_acquire_cred(status,name,timereq,dmechs,usage,outcred,amechs,timerec));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_get_mic(OM_uint32 * status,           /* minor_status */
            gss_ctx_id_t context,               /* context_handle */
            gss_qop_t qop,                  /* qop_req */
            gss_buffer_t buf,               /* message_buffer */
            gss_buffer_t tok               /* message_token */
           )
{
    if ( p_gss_get_mic )
        return(p_gss_get_mic(status,context,qop,buf,tok));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_display_name(OM_uint32 * status,           /* minor_status */
            gss_name_t      inname,                 /* input_name */
            gss_buffer_t    outname,               /* output_name_buffer */
            gss_OID *       outtype              /* output_name_type */
           )
{
    if ( p_gss_display_name )
        return(p_gss_display_name(status,inname,outname,outtype));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_test_oid_set_member(OM_uint32 * status,           /* minor_status */
            gss_OID member,                    /* member */
            gss_OID_set set,                /* set */
            int * present                      /* present */
           )
{
    if ( p_gss_test_oid_set_member )
        return(p_gss_test_oid_set_member(status,member,set,present));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_add_oid_set_member(OM_uint32 * status,           /* minor_status */
            gss_OID member,                    /* member_oid */
            gss_OID_set * set              /* oid_set */
           )
{
    if ( p_gss_add_oid_set_member )
        return(p_gss_add_oid_set_member(status,member,set));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_create_empty_oid_set(OM_uint32 * status,           /* minor_status */
            gss_OID_set * set              /* oid_set */
           )
{
    if ( p_gss_create_empty_oid_set )
        return(p_gss_create_empty_oid_set(status,set));
    else
        return(GSS_S_UNAVAILABLE);
}

OM_uint32
ck_gss_release_oid_set(OM_uint32 * status,           /* minor_status */
            gss_OID_set *  set             /* set */
           )
{
    if ( p_gss_release_oid_set )
        return(p_gss_release_oid_set(status,set));
    else
        return(GSS_S_UNAVAILABLE);
}


static int gssapi_loaded = 0;

static void
load_gssapi_eh()
{
    if ( hGSSAPI) {
#ifdef NT
        FreeLibrary(hGSSAPI);
        hGSSAPI = NULL;
#else
        DosFreeModule(hGSSAPI);
        hGSSAPI = 0;
#endif /* NT */
    }

    p_gss_release_buffer = NULL;
    p_gss_seal = NULL;
    p_gss_unseal = NULL;
    p_gss_release_name = NULL;
    p_gss_init_sec_context = NULL;
    p_gss_display_status = NULL;
    p_gss_indicate_mechs = NULL;

    p_gss_verify_mic             = NULL;
    p_gss_test_oid_set_member    = NULL;
    p_gss_add_oid_set_member     = NULL;
    p_gss_create_empty_oid_set   = NULL;
    p_gss_release_cred           = NULL;
    p_gss_delete_sec_context     = NULL;
    p_gss_accept_sec_context     = NULL;
    p_gss_release_oid_set        = NULL;
    p_gss_acquire_cred           = NULL;
    p_gss_display_name           = NULL;
    p_gss_get_mic                = NULL;

    gssapi_loaded = 0;
}

int
ck_gssapi_loaddll()
{
    ULONG rc = 0;
    int load_error = 0;
    gss_OID_set mech_set=NULL;
    OM_uint32 minor_status=0;

#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "GSSAPI32";
    int len;
#endif /* OS2ONLY */

    if ( gssapi_loaded )
        return(1);

#ifdef NT
    hGSSAPI = LoadLibrary("GSSKRB5");
    if ( !hGSSAPI )
        hGSSAPI = LoadLibrary("GSSAPI32");
    if ( !hGSSAPI )
        return(0);

    if (((FARPROC) p_gss_release_buffer =
          GetProcAddress( hGSSAPI, "gss_release_buffer" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_seal =
          GetProcAddress( hGSSAPI, "gss_seal" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_unseal =
          GetProcAddress( hGSSAPI, "gss_unseal" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_release_name =
          GetProcAddress( hGSSAPI, "gss_release_name" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_init_sec_context =
          GetProcAddress( hGSSAPI, "gss_init_sec_context" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_import_name =
          GetProcAddress( hGSSAPI, "gss_import_name" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_display_status =
          GetProcAddress( hGSSAPI, "gss_display_status" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_indicate_mechs =
          GetProcAddress( hGSSAPI, "gss_indicate_mechs" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_verify_mic =
          GetProcAddress( hGSSAPI, "gss_verify_mic" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_test_oid_set_member =
          GetProcAddress( hGSSAPI, "gss_test_oid_set_member" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_add_oid_set_member =
          GetProcAddress( hGSSAPI, "gss_add_oid_set_member" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_create_empty_oid_set =
          GetProcAddress( hGSSAPI, "gss_create_empty_oid_set" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_release_cred =
          GetProcAddress( hGSSAPI, "gss_release_cred" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_delete_sec_context =
          GetProcAddress( hGSSAPI, "gss_delete_sec_context" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_accept_sec_context =
          GetProcAddress( hGSSAPI, "gss_accept_sec_context" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_release_oid_set =
          GetProcAddress( hGSSAPI, "gss_release_oid_set" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_acquire_cred =
          GetProcAddress( hGSSAPI, "gss_acquire_cred" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_display_name =
          GetProcAddress( hGSSAPI, "gss_display_name" )) == NULL )
    {
        load_error = 1;
    }
    if (((FARPROC) p_gss_get_mic =
          GetProcAddress( hGSSAPI, "gss_get_mic" )) == NULL )
    {
        load_error = 1;
    }
#else /* NT */
    exe_path = GetLoadPath();
    len = get_dir_len(exe_path);
    if ( len + strlen(dllname) + 4 > sizeof(path) )
        return(0);
    sprintf(path, "%.*s%s.DLL", len, exe_path,dllname);         /* safe */
    rc = DosLoadModule(fail, sizeof(fail), path, &hGSSAPI);
    if (rc) {
        debug(F111, "GSSAPI LoadLibrary failed",fail,rc) ;
        rc = DosLoadModule(fail, sizeof(fail), dllname, &hGSSAPI);
    }

    if (rc) {
        debug(F111, "GSSAPI LoadLibrary failed",fail,rc) ;
        return(0);
    }

    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_release_buffer",
                              (PFN*)&p_gss_release_buffer))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_release_buffer",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_seal",
                              (PFN*)&p_gss_seal))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_seal",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_unseal",
                              (PFN*)&p_gss_unseal))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_unseal",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_release_name",
                              (PFN*)&p_gss_release_name))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_release_name",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_init_sec_context",
                              (PFN*)&p_gss_init_sec_context))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_init_sec_context",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_import_name",
                              (PFN*)&p_gss_import_name))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_import_name",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_display_status",
                              (PFN*)&p_gss_display_status))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_display_status",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr(hGSSAPI,0,"gss_indicate_mechs",
                              (PFN*)&p_gss_indicate_mechs))
    {
        debug(F111,"GSSAPI GetProcAddress failed","gss_indicate_mechs",rc);
        load_error = 1;
    }
#endif /* NT */

    if ( load_error ) {
        load_gssapi_eh();
        return 0;
    }
    gssapi_loaded = 1;
    return(1);
}

int
ck_gssapi_unloaddll()
{
    if ( !gssapi_loaded )
        return(1);

    load_gssapi_eh();
    return(1);
}

int gssapi_avail() {
    return gssapi_loaded;
}
#else /* GSSAPI */
int ck_gssapi_unloaddll() { return 0; }
int gssapi_avail() { return 0; }
#endif /* GSSAPI */

int
ck_security_loaddll( void )
{
    ULONG rc = 0 ;
    extern unsigned long startflags;

    if ( security_dll_loaded )
        return(1);

    if ( startflags & 8 )       /* do not load if we are not supposed to */
        return(0);

    /* Cygnus Solutions builds the libraries differently from MIT */
    /* Cygnus combines KRB5_32 and COMERR32 into a single DLL     */
    /* called LIBKRB5.                                            */

    /* load dlls */
#ifdef CRYPT_DLL
    ck_crypt_loaddll();
#endif /* CRYPT_DLL */

#ifdef NTLM
    ck_sspi_loaddll();
#endif /* NTLM */

#ifdef CK_SRP
    if (ck_srp_loaddll() && hCRYPT)
        ck_krypto_loaddll();
#endif /* CK_SRP */

#ifdef CK_KERBEROS
#ifdef KRB5
    ck_krb5_loaddll();
    ck_gssapi_loaddll();
#endif /* KRB5 */
#ifdef KRB4
    ck_krb4_loaddll();
#endif /* KRB4 */
#endif /* CK_KERBEROS */

#ifdef CK_SSL
    ck_crypto_loaddll();
    ck_ssl_loaddll();
#endif /* CK_SSL */

#ifdef ZLIB
    ck_zlib_loaddll();
#endif /* ZLIB */

    /* success */
    security_dll_loaded = 1;
    return(1);
}

int
ck_security_unloaddll( void )
{
    if ( !security_dll_loaded )
        return(1);

    /* unload dlls */
    ck_krb4_loaddll_eh();
    ck_gssapi_unloaddll();
    ck_krb5_loaddll_eh();

#ifdef CK_SSL
    ck_ssl_unloaddll();
    ck_crypto_unloaddll();
#endif /* CK_SSL */

#ifdef ZLIB
    ck_zlib_unloaddll();
#endif /* ZLIB */

#ifdef CRYPT_DLL
    ck_crypt_unloaddll();
#endif /* CRYPT_DLL */

#ifdef NTLM
    ck_ntlm_unloaddll();
#endif /* NTLM */

#ifdef CK_SRP
    ck_srp_unloaddll();
    ck_krypto_unloaddll();
#endif /* CK_SRP */

    /* success */
    security_dll_loaded = 0;
    return(1);
}

#ifdef KRB5
int
is_NRL_KRB5()
{
    return !cygnus && p_decode_krb5_ticket && p_des_set_random_generator_seed &&
        p_krb5_appdefault_boolean && p_krb5_appdefault_string &&
        !p_krb5_free_unparsed_name && !p_krb5_auth_con_getrcache &&
        !p_krb5_c_enctype_compare  && !p_krb5_kuserok &&
        !p_krb5_get_renewed_creds && !p_krb5_get_validated_creds;
}

int
has_NEW_KRB5_CRYPTO()
{
    return(p_krb5_c_encrypt && p_krb5_c_decrypt && p_krb5_c_make_random_key &&
            p_krb5_c_random_seed && p_krb5_c_block_size && p_krb5_c_encrypt_length);
}
#endif /* KRB5 */
#endif /* End OS2 DLL Function Pointers */


#ifdef COMMENT
Undocumented function:

From: "Maxim S. Shatskih" <maxim__s@mtu-net.ru>
Looks like calling NtCreateToken from your service running under
LocalSystem is the only way.
The function is completely undocumented - looks like it accepts parameters
like user's SID, all groups SID's (to which this user belongs) and all
privilige
IDs - which this user posesses.
Calling the function will bypass NT security & log the specified user on
without
any password validation (you are free to use your own instead of NTs
standard) & audit trail. In fact, this means that you implement your own LSA
:-)


// untested
HCRYPTPROV hProvider = 0;
BYTE randomBytes[128];
CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
CryptGenRandom(hProvider, sizeof(randomBytes), randomBytes);

#endif /* COMMENT */
#endif /* CK_AUTHENTICATION */
