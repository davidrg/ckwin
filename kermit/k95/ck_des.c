/*
  C K _ D E S . C  -  libDES interface for Kermit 95"

  Copyright (C) 1998, 2023, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.

  Author:
  Jeffrey E Altman (jaltman@secure-endpoints.com)
  Last update: 13 December 2022 David Goodwin.
*/

/*
   This file contains wrappers so that the following functions will be imported
   into the k95crypt.dll/k2crypt.dll files in such a form that they can be
   re-exported to k95.exe/k2.exe.  This subset of the DES library is needed to
   provide DES based Kerberos authentication.
*/


#ifdef LIBDES
/* The following is specific to my installation, but since I'm the only one */
/* that uses this file ...                                                  */
#include "ckcdeb.h"
#include "ckuath.h"
#define CK_DES_C
#include "ckuat2.h"

#ifdef LIBDES_USE_OPENSSL
#else
#include <des/des.h>
#endif

#include "ckucmd.h"                     /* xx_strp */
#include "ckcfnp.h"                     /* Prototypes */

/* This is required by ckclib.c and normally defined in ckuus4.c */
#ifdef CRYPT_DLL
int fp_digits = 0;		      /* Digits of floating point precision */
#else
extern int fp_digits;
#endif /* CRYPT_DLL */

int
libdes_random_key(des_cblock B)
{
    des_random_key(B);
    return(0);
}

void
libdes_random_seed(des_cblock B)
{
    des_random_seed(B);
}

void
libdes_key_sched(des_cblock * B, des_key_schedule S)
{
    des_key_sched(B,S);
}

void
libdes_ecb_encrypt(des_cblock * B1, des_cblock * B2, des_key_schedule S, int n)
{
    des_ecb_encrypt(B1,B2,S,n);
}

int
libdes_string_to_key(char * s, des_cblock * B)
{
    des_string_to_key(s,B);
    return(0);
}

void
libdes_fixup_key_parity(des_cblock * B)
{
    des_set_odd_parity(B);
}

void
libdes_pcbc_encrypt(des_cblock *input, des_cblock *output, long length,
                     des_key_schedule schedule, des_cblock *ivec, int enc)
{
    des_pcbc_encrypt(input,output,length,schedule,ivec,enc);
}

#ifdef CRYPT_DLL
void
libdes_dll_init(crypt_dll_init_data * init)
{
    init->callbackp_install_dllfunc("libdes_random_key",libdes_random_key);
    init->callbackp_install_dllfunc("libdes_random_seed",libdes_random_seed);
    init->callbackp_install_dllfunc("libdes_key_sched",libdes_key_sched);
    init->callbackp_install_dllfunc("libdes_ecb_encrypt",libdes_ecb_encrypt);
    init->callbackp_install_dllfunc("libdes_string_to_key",libdes_string_to_key);
    init->callbackp_install_dllfunc("libdes_fixup_key_parity",libdes_fixup_key_parity);
    init->callbackp_install_dllfunc("libdes_pcbc_encrypt",libdes_pcbc_encrypt);

}
#endif /* CRYPT_DLL */
#endif /* LIBDES */
