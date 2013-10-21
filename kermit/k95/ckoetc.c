#ifdef TEST
/*
  The preliminaries and the main program are just for testing ...
  Otherwise, compile and link with Kermit and everything should work.
  Definitions and prototypes that might need to be shared with other
  modules are in ckoetc.h.
*/
#include <stdio.h>
#ifdef sparc
#define SUNOS41
#include <string.h>
#include <stdlib.h>
#ifdef __STDC__
#define VOID void
#else
#define VOID int
#endif /* __STDC__ */
#endif /* sparc */

#ifndef NULL
#define NULL 0L
#endif /* NULL */

#include "ckcdeb.h"

#define DECLARE_CHK3
#define _PROTOTYP( func, parms ) func parms
#endif /* TEST */

/* ---(cut here for not testing, and again below)--- */

static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

#ifdef REXXDLL
#include <stdio.h>

#ifndef NULL
#define NULL 0L
#endif /* NULL */

#define DECLARE_CHK3
#define _PROTOTYP( func, parms ) func parms
#endif /* REXXDLL */

#ifndef TEST
#include "ckcdeb.h"
#endif /* TEST */
#include "ckuusr.h"
#include "ckoetc.h"
#ifdef NT
#include "ckcxla.h"
#include "ckuxla.h"
#endif

#ifdef NT
char *cketcv = "Win32 etc module 8.0.003, 8 Feb 1999";
#else
char *cketcv = "OS/2 etc module, 8.0.003, 8 Feb 1999";
#endif /* NT */
/*
  Copyright (C) 1996,2004 Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

#ifdef NOXFER
#define DECLARE_CHK3
#endif /* NOXFER */

#ifdef DECLARE_CHK3

static long crcta[16] = { 0L, 010201L, 020402L, 030603L, 041004L,
  051205L, 061406L, 071607L, 0102010L, 0112211L, 0122412L, 0132613L, 0143014L,
  0153215L, 0163416L, 0173617L };

static long crctb[16] = { 0L, 010611L, 021422L, 031233L, 043044L,
  053655L, 062466L, 072277L, 0106110L, 0116701L, 0127532L, 0137323L, 0145154L,
  0155745L, 0164576L, 0174367L };

unsigned int
chk3(register CHAR *pkt, register int len)  {
    register long c, crc;
    for (crc = 0; len-- > 0; pkt++) {
        c = crc ^ (long)(*pkt);
        crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    return((unsigned int) (crc & 0xFFFF));
}
#endif /* DECLARE_CHK3 */

/*
 This is the real registration struct that will be written over
 in the registration procedure.
*/
static struct                           /* A pre-Initialized */
ck_registration ck_reg = {              /* registration structure */
  "====758914360207174068160528073",    /* Ident */
  9999,                                    /* Recognizable strings... */
  "^H^H^H^H^H^H^_{({({({(^^^^^^^^^",    /* Serial number */
  "^C^C^C____^_^_^_^____)})})})})}",    /* Name */
  "$(((((((((((())))))))))))^@^@^@",    /* Company */
  0x5555,                               /* Dummy (and invalid) CRC */
  0,                                    /* Install Time */
};

static struct ck_registration reg_cpy;

static char snbuf[32]="";               /* Serial number buffer */

/*
  M A K S N

  Given a numeric serial number (long), make a Kermit product serial number.
  Returns a pointer to the Kermit serial number, or a null pointer if
  there is any error.

  The format of the Kermit product serial number is:

    PPPPnnnnXnnYnnnVVVV

  where:

    PPPP = 4-character product ID
    nnnnnnnnn = 9-digit sequential serial number (decimal)
    X = final digit of decimal 16-bit CRC
    Y = penultimate digit of decimal 16-bit CRC
    VVVV = 4-character revision ID

  The CRC is taken on PPPPnnnnnnnnn, and starts with the magic OFFSET.
*/
char *
maksn(char * product, long n, char * revcode) { /* Make serial number */
    char tmpbuf[32];
    unsigned int sum = XOFFSET;
    int x, y;
    if (n < 1L)
      return(NULL);
    sprintf(tmpbuf,"%4s%09ld",product,n);
    sum += chk3(tmpbuf,strlen(tmpbuf));
    x = sum % 10;
    y = (sum / 10) % 10;
    strncpy(snbuf,tmpbuf,8);
    snbuf[8] = x + '0';
    snbuf[9] = tmpbuf[8];
    snbuf[10] = tmpbuf[9];
    snbuf[11] = y + '0';
    strncpy(snbuf+12,tmpbuf+10,3);
    strncpy(snbuf+15,revcode,4);
    return((char *)snbuf);
}

/*
  C H K S N

  Given a Kermit serial number, checks to see if it is valid.
  Returns a pointer to struct ck_sn, in which the ok member is
  set to 0 if the serial number is not valid, and nonzero if it is valid,
  with the other members set to the product code, sequential serial number,
  and revision code.
*/
static struct ck_sn sn;

struct ck_sn *
chksn(char * s) {
    char tmpbuf[32];
    int x=0, y=0;
    int sum = XOFFSET;
    static struct ck_sn * p=NULL;

    p = &sn;

    memset(tmpbuf,0,32) ;
    strncpy(tmpbuf,s,8);
    x = s[8] - '0';
    tmpbuf[8] = s[9];
    tmpbuf[9] = s[10];
    y = s[11] - '0';
    strncpy(tmpbuf+10,s+12,3);
    sum += chk3(tmpbuf,strlen(tmpbuf));
    sn.ok = 0;
    if (x != sum % 10) {
        debug(F100,"chksn x","",0);
        return(NULL);
    }
    if (y != (sum / 10) % 10) {
        debug(F100,"chksn y","",0);
        return(NULL);
    }
    sn.ok = 1;
    strncpy(sn.product,s,3);
    strncpy(sn.serial,tmpbuf+4,9);
    strncpy(sn.revcode,s+16,3);

    debug(F100,"chksn z","",0);
    return(p);
}

/*
  C K _ E N C R Y P T

  Encrypt a string in place.
  If given only 7-bit ascii graphic characters, this will produce a
  string that still contains only 7-bit ascii graphic characters --
  no control or 8-bit characters.
*/
VOID
ck_encrypt(char * s) {
    while (*s) {
        if (*s != 'p' && *s != '\\' && *s != 'S')
          *s ^= 0x0F;
        s++;
    }
}

/*   C K _ D E C R Y P T  -  Decrypt a string in place */

VOID
ck_decrypt(char * s) {
    while (*s) {
        if (*s != 'p' && *s != '\\' && *s != 'S')
          *s ^= 0x0F;
        s++;
    }
}

/*  S E T C R C  -  Puts the CRC into the registration struct.  */
/* This function is called after the ck_registration structure  */
/* has been encrypted. */

int
setcrc(struct ck_registration * r) {
    char * p;
    p = malloc(94);
    memset( p, 0, 94 ) ;
    if (p) {
       strcpy(p,r->serial);
       strcpy(p+31,r->name);
       strcpy(p+62,r->corp);
       r->crc = chk3(p,strlen(p));
       free(p);
       return(1);
    } else r->crc = 0;
    return(0);
}

/*
  I S R E G I S T E R E D

  Call with pointer to a registration struct.
  Returns:
   0 if not set
  -2 if set but crc is bad
  -1 if set but serial number is invalid
   >0 if set and serial number is valid and crc is good
     return value is decoded serial sequence number ;
*/

#ifdef NT
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* Character set xlate */
    extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* functions. */
    extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
#endif /* NT */

char *
get_reg_name(void) {
    static char reg_name[32];
#ifdef NT
    int i;
    CHAR (*cs)(CHAR) = xlr[(tcsl == TX_CP852) ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
#endif
    strncpy( reg_name, (char *) ck_reg.name, 32 ) ;
    ck_decrypt( reg_name ) ;
#ifdef NT
#ifndef COMMENT
    if (cs != NULL) {
        for ( i=0;i<32;i++ )
            reg_name[i] = (*cs)(reg_name[i]);
    }
#else
    CharToOemBuff( reg_name, reg_name, 32 );
#endif
#endif
    debug(F110,"get_reg_name",reg_name,0);
    return(reg_name);
}
char *
get_reg_corp(void) {
   static char reg_corp[32] ;
#ifdef NT
    int i;
    CHAR (*cs)(CHAR) = xlr[tcsl == TX_CP852 ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
#endif
    strncpy( reg_corp, (char *) ck_reg.corp, 32 ) ;
    ck_decrypt( reg_corp ) ;
#ifdef NT
    if (cs != NULL) {
        for ( i=0;i<32;i++ )
            reg_corp[i] = (*cs)(reg_corp[i]);
    }
#endif
    debug(F110,"get_reg_corp",reg_corp,0);
    return(reg_corp);
}
char *
get_reg_sn(void) {
   static char serial[32] ;
#ifdef NT
    int i;
    CHAR (*cs)(CHAR) = xlr[tcsl == TX_CP852 ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
#endif
    strncpy( serial, (char *) ck_reg.serial, 32 ) ;
    ck_decrypt( serial ) ;
#ifdef NT
    if (cs != NULL) {
        for ( i=0;i<32;i++ )
            serial[i] = (*cs)(serial[i]);
    }
#endif
    debug(F110,"get_reg_sn",serial,0);
    return(serial);
}

int
get_reg_count(void) {
    debug(F101,"get_reg_count","",ck_reg.is_set);
    return ck_reg.is_set ;
}

void
reg_copy(void)
{       
    memcpy(&reg_cpy,&ck_reg,sizeof(struct ck_registration));
}



void 
regdump(char * s)
{       
#ifdef BETATEST
    hexdump(s,&ck_reg,sizeof(struct ck_registration));
    hexdump("reg_cpy",&reg_cpy,sizeof(struct ck_registration));
#endif /* BETATEST */
}

int
isregistered(struct ck_registration * r) {
    unsigned int crc = 0;
    char * p;
    struct ck_sn * sn ;
    char serial[32] ;

    if (!r) r = &ck_reg;
    regdump("ck_reg");
    if (r->is_set == 0) {
        debug(F101,"isregistered 1","",0);
        return(0);
    }
    strncpy( serial, (char *) r->serial, 32 ) ;
    ck_decrypt( serial ) ;
    if (!(sn = chksn(serial))) {
        debug(F101,"isregistered 2","",-1);
        return(-1);
    }
    p = malloc(94);
    memset( p, 0, 94 ) ;
    if (p) {
       strcpy(p,r->serial);
       strcpy(p+31,r->name);
       strcpy(p+62,r->corp);
       crc = chk3(p,strlen(p));
       free(p);
    }
    if (crc != r->crc) {
        debug(F101,"isregistered 3","",-2);
        return(-2);
    }
    debug(F101,"isregistered 4","",atol( sn->serial ));
    return( atol( sn->serial ) );
}

unsigned long
regtime(struct ck_registration * r) {
    if (!r) r = &ck_reg;
    debug(F101,"regtime","",r->time);
    return(r->time);
}

#define MAXELENGTH 511
unsigned char ibuf[MAXELENGTH+1];
unsigned char ebuf[MAXELENGTH+MAXELENGTH+5];
unsigned char dbuf[MAXELENGTH+1];

CHAR *
ck_oox(s, key) char * s; char * key; {

    unsigned long left, right;
    unsigned int i, k, l, n;
    CHAR *p, *q;

    if (!key) key = "";
    if (!s) s = "";

    if ((k = (int)strlen(s)) > MAXELENGTH)
      return((CHAR *)"");

    memset(ibuf,'\0',MAXELENGTH+1);     /* 0-padded source buffer */
    memset(ebuf,'\0',MAXELENGTH+MAXELENGTH+5); /* Clear destination buffer */
    InitializeBlowfish((CHAR *)key, (short)strlen(key));
    strncpy(ibuf,(CHAR *)s,MAXELENGTH); /* Copy source to padded cell */
    p = ibuf;                           /* Point to padded source string */
    n = k / 8;                          /* Number of 64-bit blocks */
    if (k % 8) n++;                     /* Including possibly padded last */
    q = ebuf;                           /* Pointer to destination buffer */
    for (l = 0; l < n; l++) {           /* Loop for each block */
        left = right = 0;               /* Init to all 0's */
#ifdef COMMENT
        memcpy((CHAR *)&left,  p,   4);
        memcpy((CHAR *)&right, p+4, 4);
#else
        left  = *p + (*(p+1)<<8) + (*(p+2)<<16) + (*(p+3)<<24);
        right = *(p+4) + (*(p+5)<<8) + (*(p+6)<<16) + (*(p+7)<<24);
#endif
        Blowfish_encipher(&left, &right); /* Output in hex because  */


        for (i = 0; i < 8; i++) {         /* it might contain zeros */
            *(q+i)   = hexdigits[(left  >> (4 * (7 - i))) & 0x0f];
            *(q+i+8) = hexdigits[(right >> (4 * (7 - i))) & 0x0f];
        }
        p += 8;
        q += 16;
    }
    return ((CHAR *) ebuf);
}


#ifdef REXXDLL
#define  INCL_REXXSAA
#include <rexxsaa.h>
ULONG APIENTRY
RexxValidateSN(
    PUCHAR Name,
    ULONG Argc,
    PRXSTRING Argv,
    PSZ Queuename,
    PRXSTRING Retstr) {
    int rc = 0 ;
    int i ;

    if ( Argc != 1 || RXZEROLENSTRING(Argv[0])) {
        MAKERXSTRING( *Retstr, strdup(""), 0 ) ;
        return -1;
    }
    if ( chksn( RXSTRPTR(Argv[0])) ) {
        MAKERXSTRING( *Retstr, strdup("1"), 1 ) ;
    } else {
        MAKERXSTRING( *Retstr, strdup("0"), 1 ) ;
    }
    return 0 ;
}
#endif /* REXXDLL */

/* ---(cut here for not testing, to end)--- */

#ifdef TEST
static struct                           /* Test */
ck_registration ck_reg2 = {             /* A good registration structure */
  "====758914360207174068160528073",    /* Ident */
  1,                                    /* Has been set */
  "K95-00000000000-1.1            ",    /* Invalid serial number */
  "Fred J. Dobbs                  ",
  "Rio Grande Mining Company      ",
  0
};

static struct                           /* Test */
ck_registration ck_reg3 = {             /* A bad registration structure */
  "====758914360207174068160528073",    /* Ident */
  1,                                    /* Has been set */
  "K95-13240563789-1.1            ",    /* Invalid serial number */
  "Fred J. Dobbs                  ",
  "Rio Grande Mining Company      ",
  0
};

static struct                           /* Test */
ck_registration ck_reg4 = {             /* A good registration structure */
  "====758914360207174068160528073",    /* Ident */
  1,                                    /* Has been set */
  "K95-12345567789-1.1            ",    /* Valid serial number */
  "Bill Clinton                   ",
  "USA, Inc.                      ",
  666                                   /* Bad CRC */
};

static struct                           /* Test */
ck_registration ck_reg5 = {             /* A good registration structure */
  "====758914360207174068160528073",    /* Ident */
  1,                                    /* Has been set */
  "K95-12345567789-1.1            ",    /* Valid serial number */
  "Bill Gates                     ",
  "Microsoft Corporation          ",
  0                                     /* Dummy CRC */
};

struct kuid {
    unsigned char * user;
    unsigned char * password;
};

struct kuid uids[] = {
    "BLOWFISH", "abcdefghijklmnopqrstuvwxyz",
    "BLOWFISH", "abcdefghijklmnopqrstuvwxyz",
    "BLOWFISH", "Who is John Galt?",
    "BLOWFISH", "Who is John Galt?",
    "BLOWFISH", "Who is John Galt?",
    "abc", "xyz",
    "abc", "xyz",
    "abc", "xyz",
    "abc", "xyz",
    "12345678", "12345678",
    "12345678", "12345678",
    "longername", "longerpassword",
    "longername", "longerpassword",
    "longername", "longerpassword",
    "veryveryverylongname", "",
    "veryveryverylongname", "",
    "four", "eightxxx",
    "four", "eightxxx"
};
int nuids = (sizeof(uids) / sizeof(struct kuid));

char unsigned *
oodecrypt(s, key, k) unsigned char *s; unsigned char *key; int k; {
    unsigned long left, right;
    unsigned int i, j, l, n, x;
    unsigned char *p, *q, c1, c2;

    n = k / 8;                          /* Number of blocks */
    if (k % 8) n++;

    memset(dbuf,'\0',MAXELENGTH+1);     /* Destination buffer */
    q = dbuf;                           /* and pointer */

    p = s;                              /* Source pointer */

    for (l = 0; l < n; l++) {           /* Loop thru each block */
        left = right = 0;
        sscanf(p,"%8x",&left);
        sscanf(p+8,"%8x",&right);
        Blowfish_decipher(&left, &right);
#ifdef COMMENT
        memcpy(q,  (unsigned char *)&left,4);
        memcpy(q+4,(unsigned char *)&right,4);
#else
        *q = left&0xFF;
        *(q+1) = (left>>8)&0xFF;
        *(q+2) = (left>>16)&0xFF;
        *(q+3) = (left>>24)&0xFF;
        *(q+4) = right&0xFF;
        *(q+5) = (right>>8)&0xFF;
        *(q+6) = (right>>16)&0xFF;
        *(q+7) = (right>>24)&0xFF;
#endif
        q += 8;
        p += 16;
    }
    return((unsigned char *)dbuf);
}

VOID
main(int argc, char ** argv) {

    unsigned int i, j, k, n;
    unsigned char *q;

    long xx;
    int x;
    char * p;

    char astring[] = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%^&*()_-=+[]\\{};:'\",<.>/?" ;
    char bstring[98];

    struct ck_sn * sp;

    if (argc < 2)
      xx = 123456789L;
    else
      xx = atol(argv[1]);
    p = maksn(PRODUCT,xx,REVCODE);
    printf("%s\n",p);
    sp = chksn(p);

    printf("ok = %d\n",sp->ok);
    printf("product=\"%s\"\n",sp->product);
    printf("serial =\"%s\"\n",sp->serial);
    printf("revcode=\"%s\"\n",sp->revcode);

    strcpy(bstring,astring);
    printf("String    = \"%s\"\n",astring);
    ck_encrypt(astring);
    printf("Encrypted = \"%s\"\n",astring);
    ck_decrypt(astring);
    printf("Decrypted = \"%s\"\n",astring);
    if (!strcmp(astring,bstring))
      printf("Decryption OK\n");
    else
      printf("Decryption failed\n");
    x = isregistered(&ck_reg);
    printf("isregistered 1: %d\n",x);
    x = isregistered(&ck_reg2);
    printf("isregistered 2: %d\n",x);
    x = isregistered(&ck_reg3);
    printf("isregistered 3: %d\n",x);
    x = isregistered(&ck_reg4);
    printf("isregistered 4: %d\n",x);
    x = setcrc(&ck_reg5);
    x = isregistered(&ck_reg5);
    printf("isregistered 5: %d\n",x);

    for (i = 0; i < nuids; i++) {
        n = strlen(uids[i].user);
        printf("%s_%s\n",uids[i].user,uids[i].password);
        q = ck_oox(uids[i].user,uids[i].password);
        printf("encrypted=[%s]\n",q);
/*
        for (j = 0; j < n; j++)
          printf("%02x",(unsigned int) q[j]);
        printf("]\n");
*/
        q = oodecrypt(q,uids[i].password,n);
        printf("decrypted=[%s]\n",q);
    }
}
#endif /* TEST */
