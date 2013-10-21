#include <windows.h>
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0L
#endif /* NULL */

#define _PROTOTYP( func, parms ) func parms

void Blowfish_encipher(unsigned long *xl, unsigned long *xr);
void Blowfish_decipher(unsigned long *xl, unsigned long *xr);
short InitializeBlowfish(unsigned char key[], short keybytes);

static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

static long crcta[16] = { 0L, 010201L, 020402L, 030603L, 041004L,
  051205L, 061406L, 071607L, 0102010L, 0112211L, 0122412L, 0132613L, 0143014L,
  0153215L, 0163416L, 0173617L };

static long crctb[16] = { 0L, 010611L, 021422L, 031233L, 043044L,
  053655L, 062466L, 072277L, 0106110L, 0116701L, 0127532L, 0137323L, 0145154L,
  0155745L, 0164576L, 0174367L };


/* 16-bit CRC */

unsigned int
chk3(register unsigned char *pkt, register int len)  {
    register long c, crc;
    for (crc = 0; len-- > 0; pkt++) {
	c = crc ^ (long)(*pkt);
	crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    return((unsigned int) (crc & 0xFFFF));
}

int
isWin95( void ) {
    int OSVer = 0;
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    OSVer = osverinfo.dwPlatformId ;
    return(OSVer==VER_PLATFORM_WIN32_WINDOWS);
}

#define PRODUCT "K95-"			/* Must be 4 chars   */
#define REVCODE "-1.1"			/* Must be 4 chars   */
#define XOFFSET 37			/* Never change this */

struct
ck_registration {			/* Registration structure */
    char ident[32] ;
    int is_set;
    char serial[32];
    char name[32];
    char corp[32];
    unsigned int crc;
    unsigned long time;
};

struct ck_sn {				/* Serial number structure */
    int ok;
    char product[5];
    char serial[10];
    char revcode[5];
};

/*
 This is the real registration struct that will be written over
 in the registration procedure.
*/
static struct                           /* A pre-Initialized */
ck_registration ck_reg = {              /* registration structure */
#ifdef COMMENT
  "====758914360207174068160528073",    /* Ident */
  0,                                    /* Recognizable strings... */
  "^H^H^H^H^H^H^_{({({({(^^^^^^^^^",    /* Serial number */
  "^C^C^C____^_^_^_^____)})})})})}",    /* Name */
  "$(((((((((((())))))))))))^@^@^@",    /* Company */
  0x5555,                               /* Dummy (and invalid) CRC */
  0,                                    /* Install Time */
#else
    "",0,"","","",0,0
#endif
};

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

    if (!s)
        return(NULL);
    if (!*s)
        return(NULL);
    if (strlen(s) != 19)
        return(NULL);

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
    if (x != sum % 10)
        return(NULL);
    if (y != (sum / 10) % 10)
        return(NULL);
    sn.ok = 1;
    strncpy(sn.product,s,3);
    strncpy(sn.serial,tmpbuf+4,9);
    strncpy(sn.revcode,s+16,3);

    return(p);
}

/*
  C K _ E N C R Y P T

  Encrypt a string in place.
  If given only 7-bit ascii graphic characters, this will produce a
  string that still contains only 7-bit ascii graphic characters --
  no control or 8-bit characters.
*/
void
ck_encrypt(char * s) {
    while (*s) {
        if (*s != 'p' && *s != '\\' && *s != 'S')
          *s ^= 0x0F;
        s++;
    }
}

/*   C K _ D E C R Y P T  -  Decrypt a string in place */

void
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
    extern unsigned char (*xls[MAXTCSETS+1][MAXFCSETS+1])(unsigned char);  /* Character set xlate */
    extern unsigned char (*xlr[MAXTCSETS+1][MAXFCSETS+1])(unsigned char);  /* functions. */
    extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
#endif /* NT */

char *
get_reg_name(void) {
    static char reg_name[32];
#ifdef NT
    int i;
    unsigned char (*cs)(unsigned char) = xlr[(tcsl == TX_CP852) ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
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
   return(reg_name);
}
char *
get_reg_corp(void) {
   static char reg_corp[32] ;
#ifdef NT
    int i;
    unsigned char (*cs)(unsigned char) = xlr[tcsl == TX_CP852 ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
#endif
   strncpy( reg_corp, (char *) ck_reg.corp, 32 ) ;
   ck_decrypt( reg_corp ) ;
#ifdef NT
   if (cs != NULL) {
    for ( i=0;i<32;i++ )
        reg_corp[i] = (*cs)(reg_corp[i]);
   }
#endif
   return(reg_corp);
}
char *
get_reg_sn(void) {
   static char serial[32] ;
#ifdef NT
    int i;
    unsigned char (*cs)(unsigned char) = xlr[tcsl == TX_CP852 ? TC_2LATIN : TC_1LATIN][tx2fc(tcsl)];
#endif
   strncpy( serial, (char *) ck_reg.serial, 32 ) ;
   ck_decrypt( serial ) ;
#ifdef NT
   if (cs != NULL) {
    for ( i=0;i<32;i++ )
        serial[i] = (*cs)(serial[i]);
   }
#endif
   return(serial);
}

int
get_reg_count(void) {
   return ck_reg.is_set ;
}

int
isregistered(struct ck_registration * r) {
    unsigned int crc = 0;
    char * p;
    struct ck_sn * sn ;
    char serial[32] ;

    if (!r) r = &ck_reg;
    if (!(r->is_set))
      return(0);
    strncpy( serial, (char *) r->serial, 32 ) ;
    ck_decrypt( serial ) ;
    if (!(sn = chksn(serial)))
      return(-1);
    p = malloc(94);
    memset( p, 0, 94 ) ;
    if (p) {
       strcpy(p,r->serial);
       strcpy(p+31,r->name);
       strcpy(p+62,r->corp);
       crc = chk3(p,strlen(p));
       free(p);
    }
    if (crc != r->crc)
      return(-2);
    return( atol( sn->serial ) );
}

unsigned long
regtime(struct ck_registration * r) {
    if (!r) r = &ck_reg;
    return(r->time);
}

#define MAXELENGTH 511
unsigned char ibuf[MAXELENGTH+1];
unsigned char ebuf[MAXELENGTH+MAXELENGTH+5];
unsigned char dbuf[MAXELENGTH+1];

unsigned char *
ck_oox(s, key) char * s; char * key; {

    unsigned long left, right;
    unsigned int i, k, l, n;
    unsigned char *p, *q;

    if (!key) key = "";
    if (!s) s = "";

    if ((k = (int)strlen(s)) > MAXELENGTH)
      return((unsigned char *)"");

    memset(ibuf,'\0',MAXELENGTH+1);     /* 0-padded source buffer */
    memset(ebuf,'\0',MAXELENGTH+MAXELENGTH+5); /* Clear destination buffer */
    InitializeBlowfish((unsigned char *)key, (short)strlen(key));
    strncpy(ibuf,(unsigned char *)s,MAXELENGTH); /* Copy source to padded cell */
    p = ibuf;                           /* Point to padded source string */
    n = k / 8;                          /* Number of 64-bit blocks */
    if (k % 8) n++;                     /* Including possibly padded last */
    q = ebuf;                           /* Pointer to destination buffer */
    for (l = 0; l < n; l++) {           /* Loop for each block */
        left = right = 0;               /* Init to all 0's */
#ifdef COMMENT
        memcpy((unsigned char *)&left,  p,   4);
        memcpy((unsigned char *)&right, p+4, 4);
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
    return ((unsigned char *) ebuf);
}

/*
  Blowfish algorithm and code by Bruce Schneier, used by permission.
  Adapted & fixed by fdc, Jan 96.
*/
#define ORDER_DCBA
#define inline _inline

#define MAXKEYBYTES   56                /* = 448 bits */
#define bf_N          16

#define UWORD_32bits  unsigned long
#define UBYTE_08bits  unsigned char

static UWORD_32bits bf_P[bf_N + 2];
static UWORD_32bits bf_P_Init[bf_N + 2] = {
  0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
  0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
  0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
  0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
  0x9216d5d9, 0x8979fb1b
};

static UWORD_32bits bf_S[4][256];
static UWORD_32bits bf_S_Init[4][256] = {
  0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7,
  0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99,
  0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16,
  0x636920d8, 0x71574e69, 0xa458fea3, 0xf4933d7e,
  0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee,
  0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013,
  0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef,
  0x8e79dcb0, 0x603a180e, 0x6c9e0e8b, 0xb01e8a3e,
  0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60,
  0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440,
  0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce,
  0xa15486af, 0x7c72e993, 0xb3ee1411, 0x636fbc2a,
  0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e,
  0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677,
  0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193,
  0x61d809cc, 0xfb21a991, 0x487cac60, 0x5dec8032,
  0xef845d5d, 0xe98575b1, 0xdc262302, 0xeb651b88,
  0x23893e81, 0xd396acc5, 0x0f6d6ff3, 0x83f44239,
  0x2e0b4482, 0xa4842004, 0x69c8f04a, 0x9e1f9b5e,
  0x21c66842, 0xf6e96c9a, 0x670c9c61, 0xabd388f0,
  0x6a51a0d2, 0xd8542f68, 0x960fa728, 0xab5133a3,
  0x6eef0b6c, 0x137a3be4, 0xba3bf050, 0x7efb2a98,
  0xa1f1651d, 0x39af0176, 0x66ca593e, 0x82430e88,
  0x8cee8619, 0x456f9fb4, 0x7d84a5c3, 0x3b8b5ebe,
  0xe06f75d8, 0x85c12073, 0x401a449f, 0x56c16aa6,
  0x4ed3aa62, 0x363f7706, 0x1bfedf72, 0x429b023d,
  0x37d0d724, 0xd00a1248, 0xdb0fead3, 0x49f1c09b,
  0x075372c9, 0x80991b7b, 0x25d479d8, 0xf6e8def7,
  0xe3fe501a, 0xb6794c3b, 0x976ce0bd, 0x04c006ba,
  0xc1a94fb6, 0x409f60c4, 0x5e5c9ec2, 0x196a2463,
  0x68fb6faf, 0x3e6c53b5, 0x1339b2eb, 0x3b52ec6f,
  0x6dfc511f, 0x9b30952c, 0xcc814544, 0xaf5ebd09,
  0xbee3d004, 0xde334afd, 0x660f2807, 0x192e4bb3,
  0xc0cba857, 0x45c8740f, 0xd20b5f39, 0xb9d3fbdb,
  0x5579c0bd, 0x1a60320a, 0xd6a100c6, 0x402c7279,
  0x679f25fe, 0xfb1fa3cc, 0x8ea5e9f8, 0xdb3222f8,
  0x3c7516df, 0xfd616b15, 0x2f501ec8, 0xad0552ab,
  0x323db5fa, 0xfd238760, 0x53317b48, 0x3e00df82,
  0x9e5c57bb, 0xca6f8ca0, 0x1a87562e, 0xdf1769db,
  0xd542a8f6, 0x287effc3, 0xac6732c6, 0x8c4f5573,
  0x695b27b0, 0xbbca58c8, 0xe1ffa35d, 0xb8f011a0,
  0x10fa3d98, 0xfd2183b8, 0x4afcb56c, 0x2dd1d35b,
  0x9a53e479, 0xb6f84565, 0xd28e49bc, 0x4bfb9790,
  0xe1ddf2da, 0xa4cb7e33, 0x62fb1341, 0xcee4c6e8,
  0xef20cada, 0x36774c01, 0xd07e9efe, 0x2bf11fb4,
  0x95dbda4d, 0xae909198, 0xeaad8e71, 0x6b93d5a0,
  0xd08ed1d0, 0xafc725e0, 0x8e3c5b2f, 0x8e7594b7,
  0x8ff6e2fb, 0xf2122b64, 0x8888b812, 0x900df01c,
  0x4fad5ea0, 0x688fc31c, 0xd1cff191, 0xb3a8c1ad,
  0x2f2f2218, 0xbe0e1777, 0xea752dfe, 0x8b021fa1,
  0xe5a0cc0f, 0xb56f74e8, 0x18acf3d6, 0xce89e299,
  0xb4a84fe0, 0xfd13e0b7, 0x7cc43b81, 0xd2ada8d9,
  0x165fa266, 0x80957705, 0x93cc7314, 0x211a1477,
  0xe6ad2065, 0x77b5fa86, 0xc75442f5, 0xfb9d35cf,
  0xebcdaf0c, 0x7b3e89a0, 0xd6411bd3, 0xae1e7e49,
  0x00250e2d, 0x2071b35e, 0x226800bb, 0x57b8e0af,
  0x2464369b, 0xf009b91e, 0x5563911d, 0x59dfa6aa,
  0x78c14389, 0xd95a537f, 0x207d5ba2, 0x02e5b9c5,
  0x83260376, 0x6295cfa9, 0x11c81968, 0x4e734a41,
  0xb3472dca, 0x7b14a94a, 0x1b510052, 0x9a532915,
  0xd60f573f, 0xbc9bc6e4, 0x2b60a476, 0x81e67400,
  0x08ba6fb5, 0x571be91f, 0xf296ec6b, 0x2a0dd915,
  0xb6636521, 0xe7b9f9b6, 0xff34052e, 0xc5855664,
  0x53b02d5d, 0xa99f8fa1, 0x08ba4799, 0x6e85076a,
  0x4b7a70e9, 0xb5b32944, 0xdb75092e, 0xc4192623,
  0xad6ea6b0, 0x49a7df7d, 0x9cee60b8, 0x8fedb266,
  0xecaa8c71, 0x699a17ff, 0x5664526c, 0xc2b19ee1,
  0x193602a5, 0x75094c29, 0xa0591340, 0xe4183a3e,
  0x3f54989a, 0x5b429d65, 0x6b8fe4d6, 0x99f73fd6,
  0xa1d29c07, 0xefe830f5, 0x4d2d38e6, 0xf0255dc1,
  0x4cdd2086, 0x8470eb26, 0x6382e9c6, 0x021ecc5e,
  0x09686b3f, 0x3ebaefc9, 0x3c971814, 0x6b6a70a1,
  0x687f3584, 0x52a0e286, 0xb79c5305, 0xaa500737,
  0x3e07841c, 0x7fdeae5c, 0x8e7d44ec, 0x5716f2b8,
  0xb03ada37, 0xf0500c0d, 0xf01c1f04, 0x0200b3ff,
  0xae0cf51a, 0x3cb574b2, 0x25837a58, 0xdc0921bd,
  0xd19113f9, 0x7ca92ff6, 0x94324773, 0x22f54701,
  0x3ae5e581, 0x37c2dadc, 0xc8b57634, 0x9af3dda7,
  0xa9446146, 0x0fd0030e, 0xecc8c73e, 0xa4751e41,
  0xe238cd99, 0x3bea0e2f, 0x3280bba1, 0x183eb331,
  0x4e548b38, 0x4f6db908, 0x6f420d03, 0xf60a04bf,
  0x2cb81290, 0x24977c79, 0x5679b072, 0xbcaf89af,
  0xde9a771f, 0xd9930810, 0xb38bae12, 0xdccf3f2e,
  0x5512721f, 0x2e6b7124, 0x501adde6, 0x9f84cd87,
  0x7a584718, 0x7408da17, 0xbc9f9abc, 0xe94b7d8c,
  0xec7aec3a, 0xdb851dfa, 0x63094366, 0xc464c3d2,
  0xef1c1847, 0x3215d908, 0xdd433b37, 0x24c2ba16,
  0x12a14d43, 0x2a65c451, 0x50940002, 0x133ae4dd,
  0x71dff89e, 0x10314e55, 0x81ac77d6, 0x5f11199b,
  0x043556f1, 0xd7a3c76b, 0x3c11183b, 0x5924a509,
  0xf28fe6ed, 0x97f1fbfa, 0x9ebabf2c, 0x1e153c6e,
  0x86e34570, 0xeae96fb1, 0x860e5e0a, 0x5a3e2ab3,
  0x771fe71c, 0x4e3d06fa, 0x2965dcb9, 0x99e71d0f,
  0x803e89d6, 0x5266c825, 0x2e4cc978, 0x9c10b36a,
  0xc6150eba, 0x94e2ea78, 0xa5fc3c53, 0x1e0a2df4,
  0xf2f74ea7, 0x361d2b3d, 0x1939260f, 0x19c27960,
  0x5223a708, 0xf71312b6, 0xebadfe6e, 0xeac31f66,
  0xe3bc4595, 0xa67bc883, 0xb17f37d1, 0x018cff28,
  0xc332ddef, 0xbe6c5aa5, 0x65582185, 0x68ab9802,
  0xeecea50f, 0xdb2f953b, 0x2aef7dad, 0x5b6e2f84,
  0x1521b628, 0x29076170, 0xecdd4775, 0x619f1510,
  0x13cca830, 0xeb61bd96, 0x0334fe1e, 0xaa0363cf,
  0xb5735c90, 0x4c70a239, 0xd59e9e0b, 0xcbaade14,
  0xeecc86bc, 0x60622ca7, 0x9cab5cab, 0xb2f3846e,
  0x648b1eaf, 0x19bdf0ca, 0xa02369b9, 0x655abb50,
  0x40685a32, 0x3c2ab4b3, 0x319ee9d5, 0xc021b8f7,
  0x9b540b19, 0x875fa099, 0x95f7997e, 0x623d7da8,
  0xf837889a, 0x97e32d77, 0x11ed935f, 0x16681281,
  0x0e358829, 0xc7e61fd6, 0x96dedfa1, 0x7858ba99,
  0x57f584a5, 0x1b227263, 0x9b83c3ff, 0x1ac24696,
  0xcdb30aeb, 0x532e3054, 0x8fd948e4, 0x6dbc3128,
  0x58ebf2ef, 0x34c6ffea, 0xfe28ed61, 0xee7c3c73,
  0x5d4a14d9, 0xe864b7e3, 0x42105d14, 0x203e13e0,
  0x45eee2b6, 0xa3aaabea, 0xdb6c4f15, 0xfacb4fd0,
  0xc742f442, 0xef6abbb5, 0x654f3b1d, 0x41cd2105,
  0xd81e799e, 0x86854dc7, 0xe44b476a, 0x3d816250,
  0xcf62a1f2, 0x5b8d2646, 0xfc8883a0, 0xc1c7b6a3,
  0x7f1524c3, 0x69cb7492, 0x47848a0b, 0x5692b285,
  0x095bbf00, 0xad19489d, 0x1462b174, 0x23820e00,
  0x58428d2a, 0x0c55f5ea, 0x1dadf43e, 0x233f7061,
  0x3372f092, 0x8d937e41, 0xd65fecf1, 0x6c223bdb,
  0x7cde3759, 0xcbee7460, 0x4085f2a7, 0xce77326e,
  0xa6078084, 0x19f8509e, 0xe8efd855, 0x61d99735,
  0xa969a7aa, 0xc50c06c2, 0x5a04abfc, 0x800bcadc,
  0x9e447a2e, 0xc3453484, 0xfdd56705, 0x0e1e9ec9,
  0xdb73dbd3, 0x105588cd, 0x675fda79, 0xe3674340,
  0xc5c43465, 0x713e38d8, 0x3d28f89e, 0xf16dff20,
  0x153e21e7, 0x8fb03d4a, 0xe6e39f2b, 0xdb83adf7,
  0xe93d5a68, 0x948140f7, 0xf64c261c, 0x94692934,
  0x411520f7, 0x7602d4f7, 0xbcf46b2e, 0xd4a20068,
  0xd4082471, 0x3320f46a, 0x43b7d4b7, 0x500061af,
  0x1e39f62e, 0x97244546, 0x14214f74, 0xbf8b8840,
  0x4d95fc1d, 0x96b591af, 0x70f4ddd3, 0x66a02f45,
  0xbfbc09ec, 0x03bd9785, 0x7fac6dd0, 0x31cb8504,
  0x96eb27b3, 0x55fd3941, 0xda2547e6, 0xabca0a9a,
  0x28507825, 0x530429f4, 0x0a2c86da, 0xe9b66dfb,
  0x68dc1462, 0xd7486900, 0x680ec0a4, 0x27a18dee,
  0x4f3ffea2, 0xe887ad8c, 0xb58ce006, 0x7af4d6b6,
  0xaace1e7c, 0xd3375fec, 0xce78a399, 0x406b2a42,
  0x20fe9e35, 0xd9f385b9, 0xee39d7ab, 0x3b124e8b,
  0x1dc9faf7, 0x4b6d1856, 0x26a36631, 0xeae397b2,
  0x3a6efa74, 0xdd5b4332, 0x6841e7f7, 0xca7820fb,
  0xfb0af54e, 0xd8feb397, 0x454056ac, 0xba489527,
  0x55533a3a, 0x20838d87, 0xfe6ba9b7, 0xd096954b,
  0x55a867bc, 0xa1159a58, 0xcca92963, 0x99e1db33,
  0xa62a4a56, 0x3f3125f9, 0x5ef47e1c, 0x9029317c,
  0xfdf8e802, 0x04272f70, 0x80bb155c, 0x05282ce3,
  0x95c11548, 0xe4c66d22, 0x48c1133f, 0xc70f86dc,
  0x07f9c9ee, 0x41041f0f, 0x404779a4, 0x5d886e17,
  0x325f51eb, 0xd59bc0d1, 0xf2bcc18f, 0x41113564,
  0x257b7834, 0x602a9c60, 0xdff8e8a3, 0x1f636c1b,
  0x0e12b4c2, 0x02e1329e, 0xaf664fd1, 0xcad18115,
  0x6b2395e0, 0x333e92e1, 0x3b240b62, 0xeebeb922,
  0x85b2a20e, 0xe6ba0d99, 0xde720c8c, 0x2da2f728,
  0xd0127845, 0x95b794fd, 0x647d0862, 0xe7ccf5f0,
  0x5449a36f, 0x877d48fa, 0xc39dfd27, 0xf33e8d1e,
  0x0a476341, 0x992eff74, 0x3a6f6eab, 0xf4f8fd37,
  0xa812dc60, 0xa1ebddf8, 0x991be14c, 0xdb6e6b0d,
  0xc67b5510, 0x6d672c37, 0x2765d43b, 0xdcd0e804,
  0xf1290dc7, 0xcc00ffa3, 0xb5390f92, 0x690fed0b,
  0x667b9ffb, 0xcedb7d9c, 0xa091cf0b, 0xd9155ea3,
  0xbb132f88, 0x515bad24, 0x7b9479bf, 0x763bd6eb,
  0x37392eb3, 0xcc115979, 0x8026e297, 0xf42e312d,
  0x6842ada7, 0xc66a2b3b, 0x12754ccc, 0x782ef11c,
  0x6a124237, 0xb79251e7, 0x06a1bbe6, 0x4bfb6350,
  0x1a6b1018, 0x11caedfa, 0x3d25bdd8, 0xe2e1c3c9,
  0x44421659, 0x0a121386, 0xd90cec6e, 0xd5abea2a,
  0x64af674e, 0xda86a85f, 0xbebfe988, 0x64e4c3fe,
  0x9dbc8057, 0xf0f7c086, 0x60787bf8, 0x6003604d,
  0xd1fd8346, 0xf6381fb0, 0x7745ae04, 0xd736fccc,
  0x83426b33, 0xf01eab71, 0xb0804187, 0x3c005e5f,
  0x77a057be, 0xbde8ae24, 0x55464299, 0xbf582e61,
  0x4e58f48f, 0xf2ddfda2, 0xf474ef38, 0x8789bdc2,
  0x5366f9c3, 0xc8b38e74, 0xb475f255, 0x46fcd9b9,
  0x7aeb2661, 0x8b1ddf84, 0x846a0e79, 0x915f95e2,
  0x466e598e, 0x20b45770, 0x8cd55591, 0xc902de4c,
  0xb90bace1, 0xbb8205d0, 0x11a86248, 0x7574a99e,
  0xb77f19b6, 0xe0a9dc09, 0x662d09a1, 0xc4324633,
  0xe85a1f02, 0x09f0be8c, 0x4a99a025, 0x1d6efe10,
  0x1ab93d1d, 0x0ba5a4df, 0xa186f20f, 0x2868f169,
  0xdcb7da83, 0x573906fe, 0xa1e2ce9b, 0x4fcd7f52,
  0x50115e01, 0xa70683fa, 0xa002b5c4, 0x0de6d027,
  0x9af88c27, 0x773f8641, 0xc3604c06, 0x61a806b5,
  0xf0177a28, 0xc0f586e0, 0x006058aa, 0x30dc7d62,
  0x11e69ed7, 0x2338ea63, 0x53c2dd94, 0xc2c21634,
  0xbbcbee56, 0x90bcb6de, 0xebfc7da1, 0xce591d76,
  0x6f05e409, 0x4b7c0188, 0x39720a3d, 0x7c927c24,
  0x86e3725f, 0x724d9db9, 0x1ac15bb4, 0xd39eb8fc,
  0xed545578, 0x08fca5b5, 0xd83d7cd3, 0x4dad0fc4,
  0x1e50ef5e, 0xb161e6f8, 0xa28514d9, 0x6c51133c,
  0x6fd5c7e7, 0x56e14ec4, 0x362abfce, 0xddc6c837,
  0xd79a3234, 0x92638212, 0x670efa8e, 0x406000e0,
  0x3a39ce37, 0xd3faf5cf, 0xabc27737, 0x5ac52d1b,
  0x5cb0679e, 0x4fa33742, 0xd3822740, 0x99bc9bbe,
  0xd5118e9d, 0xbf0f7315, 0xd62d1c7e, 0xc700c47b,
  0xb78c1b6b, 0x21a19045, 0xb26eb1be, 0x6a366eb4,
  0x5748ab2f, 0xbc946e79, 0xc6a376d2, 0x6549c2c8,
  0x530ff8ee, 0x468dde7d, 0xd5730a1d, 0x4cd04dc6,
  0x2939bbdb, 0xa9ba4650, 0xac9526e8, 0xbe5ee304,
  0xa1fad5f0, 0x6a2d519a, 0x63ef8ce2, 0x9a86ee22,
  0xc089c2b8, 0x43242ef6, 0xa51e03aa, 0x9cf2d0a4,
  0x83c061ba, 0x9be96a4d, 0x8fe51550, 0xba645bd6,
  0x2826a2f9, 0xa73a3ae1, 0x4ba99586, 0xef5562e9,
  0xc72fefd3, 0xf752f7da, 0x3f046f69, 0x77fa0a59,
  0x80e4a915, 0x87b08601, 0x9b09e6ad, 0x3b3ee593,
  0xe990fd5a, 0x9e34d797, 0x2cf0b7d9, 0x022b8b51,
  0x96d5ac3a, 0x017da67d, 0xd1cf3ed6, 0x7c7d2d28,
  0x1f9f25cf, 0xadf2b89b, 0x5ad6b472, 0x5a88f54c,
  0xe029ac71, 0xe019a5e6, 0x47b0acfd, 0xed93fa9b,
  0xe8d3c48d, 0x283b57cc, 0xf8d56629, 0x79132e28,
  0x785f0191, 0xed756055, 0xf7960e44, 0xe3d35e8c,
  0x15056dd4, 0x88f46dba, 0x03a16125, 0x0564f0bd,
  0xc3eb9e15, 0x3c9057a2, 0x97271aec, 0xa93a072a,
  0x1b3f6d9b, 0x1e6321f5, 0xf59c66fb, 0x26dcf319,
  0x7533d928, 0xb155fdf5, 0x03563482, 0x8aba3cbb,
  0x28517711, 0xc20ad9f8, 0xabcc5167, 0xccad925f,
  0x4de81751, 0x3830dc8e, 0x379d5862, 0x9320f991,
  0xea7a90c2, 0xfb3e7bce, 0x5121ce64, 0x774fbe32,
  0xa8b6e37e, 0xc3293d46, 0x48de5369, 0x6413e680,
  0xa2ae0810, 0xdd6db224, 0x69852dfd, 0x09072166,
  0xb39a460a, 0x6445c0dd, 0x586cdecf, 0x1c20c8ae,
  0x5bbef7dd, 0x1b588d40, 0xccd2017f, 0x6bb4e3bb,
  0xdda26a7e, 0x3a59ff45, 0x3e350a44, 0xbcb4cdd5,
  0x72eacea8, 0xfa6484bb, 0x8d6612ae, 0xbf3c6f47,
  0xd29be463, 0x542f5d9e, 0xaec2771b, 0xf64e6370,
  0x740e0d8d, 0xe75b1357, 0xf8721671, 0xaf537d5d,
  0x4040cb08, 0x4eb4e2cc, 0x34d2466a, 0x0115af84,
  0xe1b00428, 0x95983a1d, 0x06b89fb4, 0xce6ea048,
  0x6f3f3b82, 0x3520ab82, 0x011a1d4b, 0x277227f8,
  0x611560b1, 0xe7933fdc, 0xbb3a792b, 0x344525bd,
  0xa08839e1, 0x51ce794b, 0x2f32c9b7, 0xa01fbac9,
  0xe01cc87e, 0xbcc7d1f6, 0xcf0111c3, 0xa1e8aac7,
  0x1a908749, 0xd44fbd9a, 0xd0dadecb, 0xd50ada38,
  0x0339c32a, 0xc6913667, 0x8df9317c, 0xe0b12b4f,
  0xf79e59b7, 0x43f5bb3a, 0xf2d519ff, 0x27d9459c,
  0xbf97222c, 0x15e6fc2a, 0x0f91fc71, 0x9b941525,
  0xfae59361, 0xceb69ceb, 0xc2a86459, 0x12baa8d1,
  0xb6c1075e, 0xe3056a0c, 0x10d25065, 0xcb03a442,
  0xe0ec6e0e, 0x1698db3b, 0x4c98a0be, 0x3278e964,
  0x9f1f9532, 0xe0d392df, 0xd3a0342b, 0x8971f21e,
  0x1b0a7441, 0x4ba3348c, 0xc5be7120, 0xc37632d8,
  0xdf359f8d, 0x9b992f2e, 0xe60b6f47, 0x0fe3f11d,
  0xe54cda54, 0x1edad891, 0xce6279cf, 0xcd3e7e6f,
  0x1618b166, 0xfd2c1d05, 0x848fd2c5, 0xf6fb2299,
  0xf523f357, 0xa6327623, 0x93a83531, 0x56cccd02,
  0xacf08162, 0x5a75ebb5, 0x6e163697, 0x88d273cc,
  0xde966292, 0x81b949d0, 0x4c50901b, 0x71c65614,
  0xe6c6c7bd, 0x327a140a, 0x45e1d006, 0xc3f27b9a,
  0xc9aa53fd, 0x62a80f00, 0xbb25bfe2, 0x35bdd2f6,
  0x71126905, 0xb2040222, 0xb6cbcf7c, 0xcd769c2b,
  0x53113ec0, 0x1640e3d3, 0x38abbd60, 0x2547adf0,
  0xba38209c, 0xf746ce76, 0x77afa1c5, 0x20756060,
  0x85cbfe4e, 0x8ae88dd8, 0x7aaaf9b0, 0x4cf9aa7e,
  0x1948c25c, 0x02fb8a8c, 0x01c36ae4, 0xd6ebe1f9,
  0x90d4f869, 0xa65cdea0, 0x3f09252d, 0xc208e69f,
  0xb74e6132, 0xce77e25b, 0x578fdfe3, 0x3ac372e6
};

/* Choose a byte order for your hardware */

union aword {
    UWORD_32bits word;
    UBYTE_08bits byte [4];
    struct {
#ifdef ORDER_ABCD                       /* ABCD - Big endian - Motorola */
        UBYTE_08bits byte0;
        UBYTE_08bits byte1;
        UBYTE_08bits byte2;
        UBYTE_08bits byte3;
#else
#ifdef ORDER_DCBA                       /* DCBA - Little endian - Intel */
        UBYTE_08bits byte3;
        UBYTE_08bits byte2;
        UBYTE_08bits byte1;
        UBYTE_08bits byte0;
#else
#ifdef ORDER_BADC               /* BADC - VAX */
        UBYTE_08bits byte1;
        UBYTE_08bits byte0;
        UBYTE_08bits byte3;
        UBYTE_08bits byte2;
#else
#ifdef ORDER_CDAB               /* CDAB - who knows */
        UBYTE_08bits byte2;
        UBYTE_08bits byte3;
        UBYTE_08bits byte0;
        UBYTE_08bits byte1;
#else
        BYTE_ORDER_IS_NOT_DEFINED
#endif  /* ORDER_CDAB */
#endif  /* ORDER_BADC */
#endif  /* ORDER_DCBA */
#endif  /* ORDER_ABCD */
    } w;
};


/*********************blowfish.c*********************/

#define S(x,i) (bf_S[i][(x.word>>(24-8*i))&(0x00FF)])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.word ^= bf_F(b) ^ bf_P[n])

void
Blowfish_encipher(UWORD_32bits *xl, UWORD_32bits *xr) {
    union aword Xl;
    union aword Xr;

    Xl.word = *xl;
    Xr.word = *xr;
    Xl.word ^= bf_P[0];

    ROUND (Xr, Xl, 1);
    ROUND (Xl, Xr, 2);
    ROUND (Xr, Xl, 3);
    ROUND (Xl, Xr, 4);
    ROUND (Xr, Xl, 5);
    ROUND (Xl, Xr, 6);
    ROUND (Xr, Xl, 7);
    ROUND (Xl, Xr, 8);
    ROUND (Xr, Xl, 9);
    ROUND (Xl, Xr, 10);
    ROUND (Xr, Xl, 11);
    ROUND (Xl, Xr, 12);
    ROUND (Xr, Xl, 13);
    ROUND (Xl, Xr, 14);
    ROUND (Xr, Xl, 15);
    ROUND (Xl, Xr, 16);
    Xr.word ^= bf_P[17];

    *xr = Xl.word;
    *xl = Xr.word;
}

void
Blowfish_decipher(UWORD_32bits *xl, UWORD_32bits *xr) {
    union aword  Xl;
    union aword  Xr;

    Xl.word = *xl;
    Xr.word = *xr;
    Xl.word ^= bf_P[17];

    ROUND (Xr, Xl, 16);  ROUND (Xl, Xr, 15);
    ROUND (Xr, Xl, 14);  ROUND (Xl, Xr, 13);
    ROUND (Xr, Xl, 12);  ROUND (Xl, Xr, 11);
    ROUND (Xr, Xl, 10);  ROUND (Xl, Xr, 9);
    ROUND (Xr, Xl, 8);   ROUND (Xl, Xr, 7);
    ROUND (Xr, Xl, 6);   ROUND (Xl, Xr, 5);
    ROUND (Xr, Xl, 4);   ROUND (Xl, Xr, 3);
    ROUND (Xr, Xl, 2);   ROUND (Xl, Xr, 1);
    Xr.word ^= bf_P[0];

    *xl = Xr.word;
    *xr = Xl.word;
}

short
InitializeBlowfish(UBYTE_08bits key[], short keybytes) {
    short         i;
    short         j;
    UWORD_32bits  data;
    UWORD_32bits  datal;
    UWORD_32bits  datar;
    union aword   temp;

    if (keybytes < 1) keybytes = 1;
    memcpy(bf_P,bf_P_Init,sizeof(bf_P_Init));
    memcpy(bf_S,bf_S_Init,sizeof(bf_S_Init));

    j = 0;
    for (i = 0; i < bf_N + 2; ++i) {
        temp.word = 0;
        temp.w.byte0 = key[j];
        temp.w.byte1 = key[(j+1)%keybytes];
        temp.w.byte2 = key[(j+2)%keybytes];
        temp.w.byte3 = key[(j+3)%keybytes];
        data = temp.word;
        bf_P[i] = bf_P[i] ^ data;
        j = (j + 4) % keybytes;
    }
    datal = 0x00000000;
    datar = 0x00000000;

    for (i = 0; i < bf_N + 2; i += 2) {
        Blowfish_encipher(&datal, &datar);
        bf_P[i] = datal;
        bf_P[i + 1] = datar;
    }
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 256; j += 2) {
            Blowfish_encipher(&datal, &datar);
            bf_S[i][j] = datal;
            bf_S[i][j + 1] = datar;
        }
    }
    return 0;
}

/* START EXPORTED FUNCTIONS */

__declspec(dllexport) DWORD _stdcall
validate_serial_no(char * serialno) 
{
    struct ck_sn * sn;

    if ( sn = chksn(serialno) ) {
        if (sn->ok && !strcmp("K95",sn->product) && !strcmp("1.1",sn->revcode) )
            return(1);
    }
    return(0);
}       

#ifdef COMMENT
typedef struct _PRINTER_INFO_2 { // pri2
    LPTSTR    pServerName;
    LPTSTR    pPrinterName;
    LPTSTR    pShareName;
    LPTSTR    pPortName;
    LPTSTR    pDriverName;
    LPTSTR    pComment;
    LPTSTR    pLocation;
    LPDEVMODE pDevMode;
    LPTSTR    pSepFile;
    LPTSTR    pPrintProcessor;
    LPTSTR    pDatatype;
    LPTSTR    pParameters;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD     Attributes;
    DWORD     Priority;
    DWORD     DefaultPriority;
    DWORD     StartTime;
    DWORD     UntilTime;
    DWORD     Status;
    DWORD     cJobs;
    DWORD     AveragePPM;
} PRINTER_INFO_2;
#endif

static char ** printer_list = NULL;
static int     printer_cnt  = 0;

__declspec(dllexport) DWORD _stdcall
init_printer_list(void)
{
    DWORD  dwBytesNeeded;
    DWORD  dwPrtRet2;
    LPTSTR lpName = NULL;
    DWORD  dwEnumFlags = PRINTER_ENUM_LOCAL;
    DWORD  dwLevel =2;
    LPPRINTER_INFO_2 pPrtInfo2=NULL;
    int i;

    if ( printer_list )
    {
        for ( i=0 ; i < printer_cnt ; i++ )
            free( printer_list[i] ) ;
        free ( printer_list )  ;
    }
    printer_list = NULL;
    printer_cnt = 0;

    if ( !isWin95() )
        dwEnumFlags |= PRINTER_ENUM_CONNECTIONS;

    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //

    EnumPrinters (dwEnumFlags, lpName, dwLevel, NULL, 0, &dwBytesNeeded,
                   &dwPrtRet2);

    //
    // (simple error checking, if these work assume rest will too)
    //

    if (!(pPrtInfo2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwBytesNeeded)))
    {
        return(0);
    }

    if (!EnumPrinters (dwEnumFlags, lpName, dwLevel,
                      (LPBYTE) pPrtInfo2,
                        dwBytesNeeded, &dwBytesNeeded, &dwPrtRet2))
    {
        LocalFree( (LPBYTE) pPrtInfo2 );
        return(0);
    }

    /* we now have an enumeration of all printer names */

    printer_list = (char **) malloc( dwPrtRet2 * sizeof(char *) );
    if ( !printer_list )
    {
        LocalFree( (LPBYTE) pPrtInfo2 );
        return(0);
    }

    for ( i=0 ; i<dwPrtRet2 ; i++ )
         printer_list[i] = strdup( pPrtInfo2[i].pPrinterName );
    printer_cnt = dwPrtRet2 ;

    LocalFree( (LPBYTE) pPrtInfo2 );
    return(printer_cnt);
}

__declspec(dllexport) DWORD _stdcall
get_printer_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= printer_cnt || !printer_list )
        return(0);

    if ( len < strlen(printer_list[index]) + 1 )
        return(-(long)strlen(printer_list[index]));

    strncpy(str,printer_list[index],len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_printer_config_string(int index, char * str, int len)
{
    char * s;

    if ( index < 0 || index >= printer_cnt || !printer_list )
        return(0);

    if ( len < strlen(printer_list[index]) + 1 )
        return(-(long)strlen(printer_list[index]));

    strncpy(str,printer_list[index],len);
    str[len-1] = '\0';

    for (s = str; *s; s++) {
        switch ( *s ) {
        case ' ':
            *s = '_';
            break;
        case ',':
            *s = '.';
            break;
        case ';':
            *s = ':';
            break;
        case '\\':
            *s = '/';
            break;
        case '?':
            *s = '!';
            break;
        case '{':
            *s = '[';
            break;
        case '}':
            *s = ']';
            break;
        default:
            *s = tolower(*s);
        }
    }
    return(1);
}


/* BEGIN TAPI CODE */

LONG (WINAPI *cklineInitialize)(LPHLINEAPP, HINSTANCE, LINECALLBACK, LPCSTR, LPDWORD ) = NULL ;
LONG (WINAPI *cklineNegotiateAPIVersion)(HLINEAPP, DWORD, DWORD, DWORD, LPDWORD, LPLINEEXTENSIONID) = NULL ;
LONG (WINAPI *cklineGetDevCaps)(HLINEAPP, DWORD, DWORD, DWORD, LPLINEDEVCAPS) = NULL ;
LONG (WINAPI *cklineShutdown)(HLINEAPP) = NULL ;
LONG (WINAPI *cklineOpen)(HLINEAPP, DWORD, LPHLINE, DWORD, DWORD, DWORD, DWORD, DWORD, 
                  LPLINECALLPARAMS) = NULL ;
LONG (WINAPI *cklineMakeCall)(HLINE hLine, LPHCALL lphCall, LPCSTR lpszDestAddress, 
                      DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams) = NULL ;
LONG (WINAPI *cklineDial)(HCALL hCall, LPCSTR lpszDestAddress, DWORD dwCountryCode) = NULL ;
LONG (WINAPI *cklineDrop)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineAnswer)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineAccept)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize) = NULL ;
LONG (WINAPI *cklineDeallocateCall)(HCALL hCall) = NULL ;
LONG (WINAPI *cklineSetCallPrivilege)(HCALL,DWORD) = NULL ;
LONG (WINAPI *cklineClose)(HLINE hLine) = NULL ;
LONG (WINAPI *cklineHandoff)(HCALL,LPCSTR,DWORD) = NULL ;
LONG (WINAPI *cklineGetID)(HLINE hLine, DWORD dwAddressID, HCALL hCall, 
               DWORD dwSelect, LPVARSTRING lpDeviceID, LPCSTR lpszDeviceClass) = NULL ;
LONG (WINAPI *cklineGetTranslateCaps)( HLINEAPP hLineApp, DWORD, 
                                       LPLINETRANSLATECAPS lpLineTranslateCaps) = NULL ;
LONG (WINAPI *cklineSetCurrentLocation)( HLINEAPP hLineApp, DWORD dwLocationID ) = NULL ;
LONG (WINAPI *cklineSetStatusMessages)( HLINE hLine, DWORD dwLineStates, 
                                        DWORD dwAddressStates ) = NULL ;
LONG (WINAPI *cklineConfigDialog)( DWORD dwLine, HWND hwin, LPCSTR lpszTypes ) = NULL ;
LONG (WINAPI *cklineTranslateDialog)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
				      HWND hwndOwner, LPCSTR lpszAddressIn ) = NULL ;
LONG (WINAPI *cklineTranslateAddress)( HLINEAPP hTAPI, DWORD dwLine, DWORD dwVersionToUse,
				       LPCSTR lpszAddressIn, DWORD dwCard,
				       DWORD dwTranslateOptions,
				       LPLINETRANSLATEOUTPUT lpTranslateOutput) = NULL ;
LONG (WINAPI *cklineGetCountry)( DWORD, DWORD, LPLINECOUNTRYLIST ) = NULL;
LONG (WINAPI *cklineGetDevConfig)(DWORD, LPVARSTRING, LPCSTR) = NULL;
LONG (WINAPI *cklineGetLineDevStatus)(HLINE hLine,LPLINEDEVSTATUS lpLineDevStatus)=NULL;
LONG (WINAPI *cklineSetDevConfig)(DWORD,LPVOID const,DWORD,LPCSTR)=NULL;
LONG (WINAPI *cklineGetCallInfo)(HCALL, LPLINECALLINFO)=NULL;
LONG (WINAPI *cklineMonitorMedia)(HCALL,DWORD)=NULL;
LONG (WINAPI *cklineGetAppPriority)(LPCSTR,DWORD,LPLINEEXTENSIONID,
				     DWORD,LPVARSTRING,LPDWORD)=NULL;
LONG (WINAPI *cklineSetAppPriority)(LPCSTR,DWORD,LPLINEEXTENSIONID,
				     DWORD,LPCSTR,DWORD)=NULL;
LONG (WINAPI *cklineGetNumRings)(HLINE,DWORD,LPDWORD)=NULL;
LONG (WINAPI *cklineSetNumRings)(HLINE,DWORD,DWORD)=NULL;
LONG (WINAPI *cklineSetCallParams)(HCALL,DWORD,DWORD,DWORD,LPLINEDIALPARAMS)=NULL;

void 
CALLBACK
cklineCallbackFunc( DWORD dwDevice, DWORD dwMsg, 
                    DWORD dwCallbackInstance,
                    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    return;
}

void
OutputDebugLastError( DWORD x, char * string )
{
    return;
}

void
HandleNoMem( ) 
{
    return;
}

void 
OutputDebugLineError( long param, char * str )
{
    return;
}

BOOL 
HandleLineErr(long lLineErr)
{
    if ( lLineErr )
	return FALSE;
    else 
        return TRUE ;
}

BOOL 
HandleNoDevicesInstalled(void)
{
    return FALSE;
}


HINSTANCE hLib = NULL ;
HLINEAPP g_hLineApp = NULL;
BOOL     g_bInitializing = FALSE;
BOOL     g_bClosing = FALSE;
int tapiopen = 0 ;
int ttyfd=-1 ; /* this holds the HLINE hLine */
#define MAXDEVS 64
LPLINEDEVCAPS g_lpLineDevCaps[64] ;
DWORD   g_dwNumDevs = -1;
HLINE g_hLine = NULL;

int 
cktapiunload(void)
{
    FreeLibrary( hLib ) ;
    hLib = NULL ;
    cklineInitialize = NULL ;
    return TRUE ;
}


int cktapiload(void) 
{
   DWORD rc = 0 ;
    hLib = LoadLibrary("tapi32") ;
   if ( !hLib )
   {
      rc = GetLastError() ;
      return FALSE;
   }

   if (((FARPROC) cklineInitialize = GetProcAddress( hLib, "lineInitialize" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineNegotiateAPIVersion = GetProcAddress( hLib, "lineNegotiateAPIVersion" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineGetDevCaps = GetProcAddress( hLib, "lineGetDevCaps" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineShutdown = GetProcAddress( hLib, "lineShutdown" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineOpen = GetProcAddress( hLib, "lineOpen" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineMakeCall = GetProcAddress( hLib, "lineMakeCall" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineDial = GetProcAddress( hLib, "lineDial" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineDrop = GetProcAddress( hLib, "lineDrop" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
    if (((FARPROC) cklineAnswer = GetProcAddress( hLib, "lineAnswer" )) == NULL )
    {
	rc = GetLastError() ;
        return FALSE;
    }
    if (((FARPROC) cklineAccept = GetProcAddress( hLib, "lineAccept" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineDeallocateCall = GetProcAddress( hLib, "lineDeallocateCall" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineClose = GetProcAddress( hLib, "lineClose" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineGetID = GetProcAddress( hLib, "lineGetID" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineGetTranslateCaps = GetProcAddress( hLib, "lineGetTranslateCaps" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineSetCurrentLocation = GetProcAddress( hLib, "lineSetCurrentLocation" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
   if (((FARPROC) cklineSetStatusMessages = GetProcAddress( hLib, "lineSetStatusMessages" )) == NULL )
   {
      rc = GetLastError() ;
      return FALSE;
   }
    if (((FARPROC) cklineConfigDialog = GetProcAddress( hLib, "lineConfigDialog" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineTranslateDialog = GetProcAddress( hLib, "lineTranslateDialog" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineTranslateAddress = 
	  GetProcAddress( hLib, "lineTranslateAddress" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetCountry = 
	  GetProcAddress( hLib, "lineGetCountry" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetLineDevStatus = 
	  GetProcAddress( hLib, "lineGetLineDevStatus" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetDevConfig = 
	  GetProcAddress( hLib, "lineGetDevConfig" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineSetDevConfig = 
	  GetProcAddress( hLib, "lineSetDevConfig" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineHandoff = 
	  GetProcAddress( hLib, "lineHandoff" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineSetCallPrivilege = 
	  GetProcAddress( hLib, "lineSetCallPrivilege" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetCallInfo = 
	  GetProcAddress( hLib, "lineGetCallInfo" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineMonitorMedia = 
	  GetProcAddress( hLib, "lineMonitorMedia" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetAppPriority = 
	  GetProcAddress( hLib, "lineGetAppPriority" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineSetAppPriority = 
	  GetProcAddress( hLib, "lineSetAppPriority" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineGetNumRings = 
	  GetProcAddress( hLib, "lineGetNumRings" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineSetNumRings = 
	  GetProcAddress( hLib, "lineSetNumRings" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    if (((FARPROC) cklineSetCallParams = 
	  GetProcAddress( hLib, "lineSetCallParams" )) == NULL )
    {
	rc = GetLastError() ;
	return FALSE;
    }
    return TRUE;
}

BOOL g_bLineInitialized = FALSE;
void
wintapiinit( void )
{
    long lReturn;
    BOOL bTryReInit = TRUE ;
	
    // Initialize TAPI
    do
    {
        lReturn = (*cklineInitialize)(&g_hLineApp, GetModuleHandle(NULL), 
             cklineCallbackFunc, "K95 Setup", &g_dwNumDevs);
	OutputDebugLastError(lReturn,"lineInitialize:");

        // If we get this error, its because some other app has yet
        // to respond to the REINIT message.  Wait 5 seconds and try
        // again.  If it still doesn't respond, tell the user.
        if (lReturn == LINEERR_REINIT)
        {
            if (bTryReInit)
            {
		Sleep(5000);	/* wait 5 secs, try again */
                bTryReInit = FALSE;
                continue;
            }
            else
            {
                g_bInitializing = FALSE;
                return;
            }
        }

        if (lReturn == LINEERR_NODEVICE)
        {
            if (HandleNoDevicesInstalled())
                continue;
            else
            {
                g_bInitializing = FALSE;
                return;
            }
        }

        if (HandleLineErr(lReturn))
            continue;
        else
        {
            OutputDebugLineError(lReturn, 
                "lineInitialize unhandled error: ");
            g_bInitializing = FALSE;
            return;
        }
    }
    while(lReturn != 0);

    g_bLineInitialized = TRUE;
}

int 
cktapiopen(void)
{
    if ( g_bClosing ) {
	while ( g_bClosing )
	    Sleep(500);
    }
    if ( tapiopen )
    {
	tapiopen++ ;
	return TRUE ;
    }

    g_bLineInitialized = FALSE;
    wintapiinit();

    if ( !g_bLineInitialized )
	return FALSE;

    tapiopen++;
    return TRUE;
}

int 
cktapidevenum( void )
{
    int i = 0 ;
    DWORD dwAPIVersion ;                    
    LINEEXTENSIONID ExtensionID ;
    int datalines = 0 ;
    LONG rc;


    /* Free existing LineDevCaps */
    for ( i = 0 ; i < g_dwNumDevs ; i++ )
    {
	if ( g_lpLineDevCaps[i] )
	{
	    free( g_lpLineDevCaps[i] ) ;
	    g_lpLineDevCaps[i] = NULL ;
	}
    }

    /* Enumerate current LineDevCaps */
    for ( i=0 ; i < g_dwNumDevs ; i++ )
    {
	g_lpLineDevCaps[i] = (LPLINEDEVCAPS) malloc (sizeof(LINEDEVCAPS)) ;
	g_lpLineDevCaps[i]->dwTotalSize = sizeof(LINEDEVCAPS) ;
	if ( rc = (*cklineNegotiateAPIVersion)(g_hLineApp, i, 
					   TAPI_CURRENT_VERSION, 
					   TAPI_CURRENT_VERSION,
					   &dwAPIVersion, &ExtensionID))
	{
	    OutputDebugLastError(rc,"lineNegotiateAPIVersion:");
	    free(g_lpLineDevCaps[i]);
	    g_lpLineDevCaps[i] = NULL ;
	    continue;
	}
	if (rc = (*cklineGetDevCaps)(g_hLineApp, i,
				 dwAPIVersion, 0, g_lpLineDevCaps[i]))
	{
	    OutputDebugLastError(rc,"lineGetDevCaps:");
	    free(g_lpLineDevCaps[i]);
	    g_lpLineDevCaps[i] = NULL ;
	    continue;
	}

	if ( g_lpLineDevCaps[i]->dwNeededSize > g_lpLineDevCaps[i]->dwTotalSize )
	{
	    DWORD NeededSize = g_lpLineDevCaps[i]->dwNeededSize;
	    free(g_lpLineDevCaps[i]) ;
	    g_lpLineDevCaps[i] = (LPLINEDEVCAPS) malloc (NeededSize) ;
	    g_lpLineDevCaps[i]->dwTotalSize = NeededSize ;

	    if ((*cklineGetDevCaps)(g_hLineApp, i,
                               dwAPIVersion, 0, g_lpLineDevCaps[i]))
	    {
		OutputDebugLastError(rc,"lineGetDevCaps:");
		free(g_lpLineDevCaps[i]);
		g_lpLineDevCaps[i] = NULL ;
		continue;
	    }
	}

      /* We now have a successful LineDevCaps structure */
	if ( g_lpLineDevCaps[i]->dwMediaModes & LINEMEDIAMODE_DATAMODEM )
	{
         /* then this is a valid line to use for data connections */
	    datalines++ ;
	}
    }
    return datalines ;
}

int 
cktapiclose(void)
{
    DWORD rc ;

    if ( g_bClosing ) {
	while ( g_bClosing )
	    Sleep(500);
	return TRUE;
    }

    if ( tapiopen > 0 )
    {
	tapiopen-- ;

	if ( tapiopen == 1 ) {
	    g_bClosing = TRUE;
	    if ( g_hLine )
	    {	
		rc = (*cklineClose)( g_hLine );
		OutputDebugLastError(rc,"lineClose:");
		g_hLine = NULL ;
		ttyfd = -1;
	    }
	}
	if ( tapiopen == 0 ) {
	    rc = (*cklineShutdown)( g_hLineApp ) ;
	    OutputDebugLastError(rc,"lineShutdown:");
	    g_hLineApp = NULL ;
	}
	g_bClosing = FALSE;
    }
    return TRUE;
}

static char ** tapi_list = NULL;
static int     tapi_cnt  = 0;

__declspec(dllexport) DWORD _stdcall
init_tapi_list(void)
{
    int i, n ;
    if ( tapi_list )
    {
        for ( i=0 ; i < tapi_cnt ; i++ )
            free( tapi_list[i] ) ;
        free ( tapi_list )  ;
    }

    tapi_list = NULL ;
    tapi_cnt = 0 ;

    if (!cktapiload())
        return(0);

    cktapiopen() ;
    n = cktapidevenum() ;
    cktapiclose() ;

    if ( n )
    {
        tapi_list = malloc( sizeof(char *) * n ) ;

        for ( i=0 ; i < g_dwNumDevs ; i++ ) {
            if ( g_lpLineDevCaps[i] && (g_lpLineDevCaps[i]->dwMediaModes & LINEMEDIAMODE_DATAMODEM) )
            {
                tapi_list[tapi_cnt++] = _strdup( ((char *)(g_lpLineDevCaps[i]))+g_lpLineDevCaps[i]->dwLineNameOffset) ;
            }
       }

       if ( tapi_cnt == 0 ) {
           /* TAPI Devices exist, but none can be used by Kermit */
           free ( tapi_list )  ;
           tapi_list = NULL;
       }
    }

    cktapiunload();
    return(tapi_cnt);
}

__declspec(dllexport) DWORD _stdcall
get_tapi_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= tapi_cnt || !tapi_list )
        return(0);

    if ( len < strlen(tapi_list[index]) + 1 )
        return(-(long)strlen(tapi_list[index]));

    strncpy(str,tapi_list[index],len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_tapi_config_string(int index, char * str, int len)
{
    char * s;

    if ( index < 0 || index >= tapi_cnt || !tapi_list )
        return(0);

    if ( len < strlen(tapi_list[index]) + 1 )
        return(-(long)strlen(tapi_list[index]));

    strncpy(str,tapi_list[index],len);
    str[len-1] = '\0';

    for (s = str; *s; s++) {
        switch ( *s ) {
        case ' ':
            *s = '_';
            break;
        case ',':
            *s = '.';
            break;
        case ';':
            *s = ':';
            break;
        case '\\':
            *s = '/';
            break;
        case '?':
            *s = '!';
            break;
        case '{':
            *s = '[';
            break;
        case '}':
            *s = ']';
            break;
        default:
            *s = tolower(*s);
        }
    }
    return(1);
}


/* BEGIN MODEM CODE */
struct mdm {				/* Modem info */
    char * menuname;			/* Name to print in menu */
    char * kermitname;			/* Kermit SET MODEM TYPE name */
    long speed;				/* Default speed */
} mdmtab[] = {
    "3COM US Robotics/Megahertz 56K", "3com-usr-megahertz-56k", 115200L,
    "Atlas Newcom 33,600ifxC", "atlas-newcom-33600ifxC", 57600L,
    "AT&T 1900 STU III",    "att-1900-stu-iii",     57600L,
    "AT&T 1910 STU III",    "att-1910-stu-iii",     57600L,
    "AT&T 7300",            "att-7300",         57600L,
    "AT&T Dataport",        "att-dataport",     57600L,
    "AT&T DTDM",            "att-dtdm",         57600L,
    "AT&T ISN",             "att-isn",          57600L,
    "AT&T KeepInTouch",     "att-keepintouch",  57600L,
    "AT&T Switched Net",    "att-switched-net", 57600L,
    "Best Data",            "bestdata",         57600L,
    "Boca",                 "boca",             57600L,
    "Cardinal",		    "cardinal",         57600L,
    "Compaq Data+Fax (Presario)",
                            "compaq",           57600L,
    "Digitel DT22",         "digitel-dt22",      2400L,
    "Fujitsu",              "fujitsu",          57600L,
    "Gateway Telepath",     "gateway-telepath", 57600L,
    "Generic High Speed",   "generic-high-speed", 57600L,
    "Hayes 1200",           "hayes-1200",        1200L,
    "Hayes 2400",           "hayes-2400",        2400L,
    "Hayes Ultra, Optima, or Accura",
                            "hayes-high-speed", 57600L,
    "IBM Mwave",            "mwave",            57600L,
    "Intel 14400 Faxmodem", "intel",            57600L,
    "ITU-T (CCITT) V25bis", "itu-t-v25bis",             2400L,
    "ITU-T (CCITT) V25ter/v250", "itu-t-v25ter/v250", 2400L,
    "Maxtech",              "maxtech",          57600L,
    "Megahertz AT&T V.34",  "megahertz-att-v34",57600L,
    "Megahertz XJack 33.6", "megahertz-xjack-33.6",  57600L,
    "Megahertz XJack 56k",  "megahertz-xjack-56k",  115200L,
    "Microcom in AT mode",  "microcom-at",      57600L,
    "Microcom in SX mode",  "microcom-sx",      57600L,
    "Microlink",            "microlink",        57600L,
    "Microlink V250",       "microlink-v250",        57600L,
    "Motorola Codex",       "motorola-codex",   57600L,
    "Motorola Fastalk",     "motorola-fastalk", 57600L,
    "Motorola Lifestyle",   "motorola-lifestyle", 57600L,
    "Motorola Montana",     "motorola-montana", 57600L,
    "Multitech",            "multitech",        57600L,
    "Practical Peripherals","ppi",              57600L,
    "QuickComm Spirit II",  "spirit-ii",        57600L,
    "Rockwell V32",         "rockwell-v32",     57600L,
    "Rockwell V32bis",      "rockwell-v32bis",  57600L,
    "Rockwell V34",         "rockwell-v34",     57600L,
    "Rockwell V90",         "rockwell-v90",    115200L,
    "Rolm 244pc",           "rolm-244pc",       19200L,
    "Rolm 600 series",      "rolm-600-series",  19200L,
    "Rolm DCM",             "rolm-dcm",         19200L,
    "Supra Fax Modem",      "suprafaxmodem",    57600L,
    "SupraSonic 288+",      "suprasonic",       57600L,
    "Telebit (old models)", "old-telebit",      19200L,
    "Telebit (newer models)","telebit",         38400L,
    "Unknown",              "unknown",           2400L,
    "US Robotics/Megahertz 56k", "usr-megahertz-56k", 115200L,
    "US Robotics Sportster or Courier",
                            "usrobotics",       57600L,
    "Zoltrix",              "zoltrix",          57600L,
    "Zoom",                 "zoom",             57600L,
    "Zyxel",                "zyxel",            57600L
};
int modems = (sizeof(mdmtab) / sizeof(struct mdm));

char * speedtab[] = {			/* Legal speeds */
    "110",
    "300",
    "1200",
    "2400",
    "3600",
    "7200",
    "9600",
    "14400",
    "19200",
    "28800",
    "38400",
    "57600",
    "76800",
    "115200",
    "230400",
    "460800"
};
int speeds = (sizeof(speedtab) / sizeof(long));

char * printers[] = {
    "PRN", "LPT1", "LPT2", "LPT3", "NUL", "KERMIT.PRN"
};
int nprinters = (sizeof(printers)/sizeof(char *));

__declspec(dllexport) DWORD _stdcall
init_modem_list(void)
{
    return(modems);
}

__declspec(dllexport) DWORD _stdcall
get_modem_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= modems )
        return(0);

    if ( len < strlen(mdmtab[index].menuname) + 1 )
        return(-(long)strlen(mdmtab[index].menuname));

    strncpy(str,mdmtab[index].menuname,len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_modem_config_string(int index, char * str, int len)
{
    if ( index < 0 || index >= modems )
        return(0);

    if ( len < strlen(mdmtab[index].kermitname) + 1 )
        return(-(long)strlen(mdmtab[index].kermitname));

    strncpy(str,mdmtab[index].kermitname,len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_modem_baud_rate(int index)
{
    if ( index < 0 || index >= modems )
        return(0);

    return(mdmtab[index].speed);
}


__declspec(dllexport) DWORD _stdcall
init_baud_rate_list(void)
{
    return(speeds);
}

__declspec(dllexport) DWORD _stdcall
get_baud_rate_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= speeds )
        return(0);

    if ( len < strlen(speedtab[index]) + 1 )
        return(-(long)strlen(speedtab[index]));

    strncpy(str,speedtab[index],len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_baud_rate_config_string(int index, char * str, int len)
{
    return(get_baud_rate_display_string(index,str,len));
}

static char ** serial_list = NULL;
static int     serial_cnt  = 0;

__declspec(dllexport) DWORD _stdcall
init_serial_port_list(void)
{
    HKEY  hk=0;
    DWORD dwType=0;
    DWORD dwValueSize=0, dwDataSize=0;
    CHAR *lpszValueName=NULL;
    char  serialdevice[64]="";
    char  dosname[12]="";
    int i;
    DWORD rc;

    if ( serial_list )
    {
        for ( i=0 ; i < serial_cnt ; i++ )
            free( serial_list[i] ) ;
        free ( serial_list )  ;
    }
    serial_list = NULL;
    serial_cnt = 0;

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0,
                        KEY_READ, &hk) )
        goto init_exit;

    for ( ;; ) {
        dwValueSize = sizeof(serialdevice);
        dwDataSize = sizeof(dosname);
        rc = RegEnumValue(hk, serial_cnt, serialdevice, &dwValueSize, 0,
                           &dwType, dosname, &dwDataSize);
        if ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA )
            serial_cnt++;
        else 
            break;
    }
    if (serial_cnt == 0)
        goto init_exit;

    serial_list = (char **) malloc( serial_cnt * sizeof(char *) );
    if ( !serial_list ) {
        serial_cnt = 0;
        goto init_exit;
    }

    for ( i=0;i<serial_cnt;i++ ) {
        dwValueSize = sizeof(serialdevice);
        dwDataSize = sizeof(dosname);
        rc = RegEnumValue(hk, i, serialdevice, &dwValueSize, 0,
                           &dwType, dosname, &dwDataSize);
        if ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA ) {
            serial_list[i] = strdup(dosname);
        }
    }

  init_exit:
    RegCloseKey( hk );
    return(serial_cnt);
}

__declspec(dllexport) DWORD _stdcall
get_serial_port_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= serial_cnt || !serial_list )
        return(0);

    if ( len < strlen(serial_list[index]) + 1 )
        return(-(long)strlen(serial_list[index]));

    strncpy(str,serial_list[index],len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_serial_port_config_string(int index, char * str, int len)
{
    return(get_serial_port_display_string(index,str,len));
}


__declspec(dllexport) DWORD _stdcall
init_dos_printer_list(void)
{
    return(nprinters);
}

__declspec(dllexport) DWORD _stdcall
get_dos_printer_display_string(int index, char * str, int len)
{
    if ( index < 0 || index >= nprinters )
        return(0);

    if ( len < strlen(printers[index]) + 1 )
        return(-(long)strlen(printers[index]));

    strncpy(str,printers[index],len);
    str[len-1] = '\0';
    return(1);
}

__declspec(dllexport) DWORD _stdcall
get_dos_printer_config_string(int index, char * str, int len)
{
    return(get_dos_printer_display_string(index,str,len));
}
/* END MODEM CODE */

/* BEGIN REGISTER EXE CODE */

#define FILECOUNT 4
#define FILECOUNT_REQUIRED 2

CHAR * FileName[FILECOUNT] ;
struct ck_registration * RegBlock[FILECOUNT] ;
HANDLE hFile[FILECOUNT] ;
HANDLE hFileMapping[FILECOUNT] ;
CHAR * pView[FILECOUNT] ;
FILETIME ftLastAccess[FILECOUNT], ftCreation[FILECOUNT], ftLastWrite[FILECOUNT] ;

static void
reg_init(char * installdir) {
    int i;
    static int init = 0;

    if (init)
        return;

    if (installdir == NULL || installdir[0] == '\0')
        installdir = ".";

    for ( i=0 ; i < FILECOUNT ; i++ )
    {
        FileName[i] = (char *)malloc(strlen(installdir) + 32);
        if (FileName[i] == NULL)
            return;
	hFile[i]        = NULL ;
	hFileMapping[i] = NULL ;
	pView[i]        = NULL ;
	RegBlock[i] = NULL ;
    }

    sprintf(FileName[0],"%s\\%s",installdir,"k95.exe");
    sprintf(FileName[1],"%s\\%s",installdir,"k95dial.exe");
    sprintf(FileName[2],"%s\\%s",installdir,"k95g.exe");
    sprintf(FileName[3],"%s\\%s",installdir,"k95c.exe");

    init = 1;
}

static int
openfile(int i)
{
   hFile[i] = CreateFile(
                   FileName[i],	// address of name of the file 
                   GENERIC_READ | GENERIC_WRITE,
                   0, 	// share mode 
                   NULL,// address of security descriptor 
                   OPEN_EXISTING,	// how to create 
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,	// file attributes 
                   NULL 	// handle of file with attributes to copy  
   );

   if ( !hFile[i] )
      return 0;
    
   GetFileTime( hFile[i], &ftCreation[i], &ftLastAccess[i], &ftLastWrite[i] ) ;
   hFileMapping[i] = CreateFileMapping( hFile[i],
                                            NULL,
                                            PAGE_READWRITE,
                                            0,
                                            0,
                                            NULL );

   pView[i] = (CHAR *) MapViewOfFile( hFileMapping[i],
                                 FILE_MAP_WRITE,
                                 0,0,0);

   if ( !pView[i] )
      return 0;
   return 1;
}

static int 
closefile( int i ) 
{
   if (pView[i])
      UnmapViewOfFile( pView[i] ) ;
   if (hFileMapping[i])
      CloseHandle( hFileMapping[i] ) ;
   if (hFile[i])
   {
      SetFileTime( hFile[i], &ftCreation[i], &ftLastAccess[i], &ftLastWrite[i] ) ;
      CloseHandle( hFile[i] ) ;
   }
   return 1;
}

static int 
SearchForRegBlock( int i ) 
{
    CHAR * SearchString =  
#ifdef COMMENT
        "====758914360207174068160528073\0\0\0\0\0^H^H^H^H^H^H^_{({({({(^^^^^^^^^\0^C^C^C____^_^_^_^____)})})})})}\0$(((((((((((())))))))))))^@^@^@"
#else
        "====758914360207174068160528073"
#endif
        ;

    //  sizeof ck_registration struct is 140 bytes.
    int SearchLength = strlen(SearchString) ;
    BY_HANDLE_FILE_INFORMATION fileInfo ;
    CHAR * start;
    CHAR * p;
    int found;
    ULONG k;

    GetFileInformationByHandle( hFile[i], &fileInfo ) ;

    //  We only search for the first null terminated portion of the search string 
    //  because we want to be able to find a pre-registered copy

    if ( !pView[i] )
        return 0 ;

    start = pView[i] ;
    p = pView[i] ;
    found = 0 ;

    k = 0 ;
    while ( !found && k < fileInfo.nFileSizeLow )
    {
        LONG j = 0 ;
        if ( p[k] == SearchString[0] )
        {
            start = &p[k] ;
            for ( j = 0 ; j < SearchLength ; j++ )
            {
                if ( start[j] != SearchString[j] )
                    break;
            }
            if ( j == SearchLength )
                found = 1 ;
        }
        k++ ;
    }

    if ( found )
	RegBlock[i] = (struct ck_registration *) start ;
    return found;
}

static int
RegisterExe( struct ck_registration * ck_reg, int i )
{
#ifdef COMMENT
    if ( isregistered( RegBlock[i] ) > 0 )
    {
        // Do nothing, already registered.
        return 0;
    }
#endif /* COMMENT */
    memcpy( RegBlock[i], ck_reg, sizeof(struct ck_registration) ) ;
    return (1) ;
}

/*
 *  Return 0 - success
 *  Return 1 - Invalid Product Prefix in Serial Number
 *  Return 2 - Invalid Name
 *  Return 3 - Invalid Corp
 *  Return 4 - Invalid Serial Number
 *  Return 5 - Unable to store registration info
 */

__declspec(dllexport) DWORD _stdcall
register_k95(char * installdir, char * serial, char * name, char * corp, int license )
{
    struct ck_registration ri ;
    int x;

    reg_init(installdir);

    memset(&ri,0,sizeof(ri));

    ri.is_set = 0 ;
    memset(ri.serial,' ',96) ;

    if ( !serial || strncmp( serial, "K95", 3 )) {
        return 1;
    }

    if ( !name || strlen(name) == 0 ) {
        return 2;
    }

    ck_encrypt( name ) ;
    strncpy( ri.name, name, 31 ) ;
    ri.name[31] = '\0' ;
    for ( x = strlen(ri.name) ; x < 31 ; x++ )
        ri.name[x] = '\0' ;
    ri.name[31] = '\0' ;

    if ( !corp || strlen(corp) == 0 )
    {
        return 3;
    }


    ck_encrypt( corp ) ;
    strncpy( ri.corp, corp, 31 ) ;
    ri.corp[31] = '\0' ;
    for ( x = strlen(ri.corp) ; x < 31 ; x++ )
        ri.corp[x] = '\0' ;
    ri.corp[31] = '\0' ;

    strncpy( ri.ident, "====758914360207174068160528073", 32 ) ;

    if ( !chksn( serial ) ) {
        return(4);
    } else {
        int regd, i;

        ck_encrypt( serial ) ;
        strncpy( ri.serial, serial, 31 ) ;
        for ( x = strlen(ri.serial) ; x < 31 ; x++ )
            ri.serial[x] = '\0' ;
        ri.serial[31] = '\0' ;

        ri.is_set = license ? license : 1 ;
        setcrc( &ri );
        regd = 0 ;
        for ( i=0 ; i<FILECOUNT ; i++ )
        {
            if ( !openfile(i) )
            {
                if ( i >= FILECOUNT_REQUIRED )
                    continue;
                else
                    break;
            }
            SearchForRegBlock(i) ;
            if (RegBlock[i])
                regd += RegisterExe( &ri, i ) ;
            closefile(i) ;
        }

        if ( !regd )
        {
            return(5);
        }
    }
    return(0);
}

/* 
 *  Return 0 - success
 *  Return 1 - not registered
 *  Return 2 - unable to find required files
 *  Return 3 - invalid file
 *  Return 4 - invalid parameters
 *  
 *  Bulk License has numbers: 100 -> 10000 || > 10000000
 */
__declspec(dllexport) DWORD _stdcall
get_reg_info(char * installdir, 
             char * serial, int serial_len,
             char * name, int name_len,
             char * corp, int corp_len )
{
    int registered = 0, i ;

    reg_init(installdir);

    if ( !name || !serial || !corp ||
         !name_len || !serial_len || !corp_len )
        return(4);

    for ( i=0 ; registered == 0 && i < FILECOUNT ; i++ )
    {
        int dsn;

	if ( !openfile(i) )
	{
            if ( i >= FILECOUNT_REQUIRED )
                continue;
	    return 2;
	}

	SearchForRegBlock(i) ;
	if ( !RegBlock[i] )
	{
	    return 3 ;
	}

        dsn = isregistered( RegBlock[i] );
	if ( dsn > 0 && dsn != 98 )
	{
	    char temp[32] ;

            registered++;

	    strcpy( temp, RegBlock[i]->name ) ;
	    ck_decrypt( temp );
            strncpy(name,temp,name_len);
            name[name_len-1] = 0;

	    strcpy( temp, RegBlock[i]->corp ) ;
	    ck_decrypt( temp ) ;
            strncpy(corp,temp,corp_len);
            corp[corp_len-1] = 0;

	    strcpy( temp, RegBlock[i]->serial ) ;
	    ck_decrypt( temp ) ;
            strncpy(serial,temp,serial_len);
            serial[serial_len-1] = 0;
	}

        closefile(i);
    }
    return (registered ? 0 : 1);
}

#define K95_APPID "Kermit.Script"
#define K95_APPID_DEFAULT "Columbia University Kermit 95 Script"
#define K95_MIME  "application/kermit"
#define K95_MIME_PATH  "MIME\\Database\\Content Type\\application/kermit"

__declspec(dllexport) int _stdcall
assoc_k95_with_telnet(char * K95Path)
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    unsigned char rc = FALSE;

    /* Telnet */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "telnet", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             "telnet",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (unsigned char*)K95_APPID_DEFAULT, 
                   sizeof(K95_APPID_DEFAULT));
    sprintf((char*)lpszKeyValue,"%s,%d",K95Path,1);
    RegSetValueEx(hkCommandKey, "DefaultIcon", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, sizeof(lpszKeyValue));


    if ( RegOpenKeyEx(hkCommandKey,
                       "shell", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkCommandKey,
                             "shell",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey,
                       "open", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey2) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey,
                             "open",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey2, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey2,
                       "command", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey3) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey2,
                             "command",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey3, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey3 );
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    sprintf((char*)lpszKeyValue,"%s -J \"%%1\"",K95Path);
    RegSetValueEx(hkSubKey3, "", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, 
                   strlen((char*)lpszKeyValue));

    RegCloseKey( hkSubKey3 );
    RegCloseKey( hkSubKey2 );
    RegCloseKey( hkSubKey );
    RegCloseKey( hkCommandKey );

    return TRUE;
}

__declspec(dllexport) int _stdcall
assoc_k95_with_ksc(char * K95Path)
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    unsigned char rc = FALSE;

    /* .KSC */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       ".ksc", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             ".ksc",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (unsigned char*)K95_APPID, sizeof(K95_APPID));
    RegSetValueEx(hkCommandKey, "Content Type", 0, REG_SZ, 
                   (unsigned char*)K95_MIME, sizeof(K95_MIME));

    RegCloseKey( hkCommandKey );

    /* MIME: application/kermit */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       K95_MIME_PATH, 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             K95_MIME_PATH,0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "Extension", 0, REG_SZ, 
                   (unsigned char *)".KSC", 4);
    RegCloseKey( hkCommandKey );

    /* Kermit.Script */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "Kermit.Script", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             "Kermit.Script",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (unsigned char*)K95_APPID_DEFAULT, 
                   sizeof(K95_APPID_DEFAULT));
    sprintf((char*)lpszKeyValue,"%s,%d",K95Path,3);
    RegSetValueEx(hkCommandKey, "DefaultIcon", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, sizeof(lpszKeyValue));


    if ( RegOpenKeyEx(hkCommandKey,
                       "shell", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkCommandKey,
                             "shell",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey,
                       "open", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey2) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey,
                             "open",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey2, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey2,
                       "command", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey3) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey2,
                             "command",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey3, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey3 );
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    sprintf((char*)lpszKeyValue,"%s \"%%1\"",K95Path);
    RegSetValueEx(hkSubKey3, "", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, 
                   strlen((char*)lpszKeyValue));

    RegCloseKey( hkSubKey3 );
    RegCloseKey( hkSubKey2 );
    RegCloseKey( hkSubKey );
    RegCloseKey( hkCommandKey );

    return TRUE;
}

__declspec(dllexport) int _stdcall
delete_associations(void)
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    unsigned char rc = FALSE;

    /* .KSC */
    RegDeleteKey(HKEY_CLASSES_ROOT, ".ksc");

    /* MIME: application/kermit */
    RegDeleteKey(HKEY_CLASSES_ROOT, K95_MIME_PATH);

    /* Kermit.Script */
    if ( !RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "Kermit.Script", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key exists */
        if ( !RegOpenKeyEx(hkCommandKey,
                            "shell", 0,
                            KEY_READ | KEY_WRITE, &hkSubKey) ) {
            /* Key exists */
            if ( !RegOpenKeyEx(hkSubKey,
                               "open", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "open" );
            if ( !RegOpenKeyEx(hkSubKey,
                               "print", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "print" );
            if ( !RegOpenKeyEx(hkSubKey,
                               "edit", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "edit" );
            RegCloseKey( hkSubKey );
        }
        RegDeleteKey( hkCommandKey, "shell");
        RegCloseKey( hkCommandKey );
    }
    RegDeleteKey(HKEY_CLASSES_ROOT,"Kermit.Script");

    return TRUE;
}

