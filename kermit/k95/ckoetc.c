static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

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

#ifdef NT
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* Character set xlate */
    extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* functions. */
    extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
#endif /* NT */


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
