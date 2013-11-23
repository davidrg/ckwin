/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/
#ifndef CKOETC_H
#define CKOETC_H

_PROTOTYP( VOID ck_encrypt, (char *) );
_PROTOTYP( VOID ck_decrypt, (char *) );

_PROTOTYP( void Blowfish_encipher, (unsigned long *xl, unsigned long *xr) ) ;
_PROTOTYP( void Blowfish_decipher, (unsigned long *xl, unsigned long *xr) ) ;
_PROTOTYP( short InitializeBlowfish, (unsigned char key[], short keybytes) );
#endif /* CKOETC_H */
