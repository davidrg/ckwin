#ifdef __cplusplus
#extern "C" {
#endif 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef VOID
#define VOID void
#endif

#define _PROTOTYP( func, parms ) func parms

#include "ckoetc.h"

#ifdef NT
char *cketcv = "Win32 etc module, 5A(001), 13 Sep 1995";
#else 
char *cketcv = "OS/2 etc module, 5A(001), 13 Sep 1995";
#endif /* NT */
/*
  Copyright (C) 1995, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/


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

#ifdef __cplusplus
}
#endif 

