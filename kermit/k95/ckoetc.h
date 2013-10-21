/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/
#ifndef CKOETC_H
#define CKOETC_H

#define PRODUCT "K95-"                  /* Must be 4 chars   */
#define REVCODE "-1.1"                  /* Must be 4 chars   */
#define XOFFSET 37                      /* Never change this */

struct
ck_registration {                       /* Registration structure */
    char ident[32] ;
    int is_set;
    char serial[32];
    char name[32];
    char corp[32];
    unsigned int crc;
    unsigned long time;
};

struct ck_sn {                          /* Serial number structure */
    int ok;
    char product[5];
    char serial[10];
    char revcode[5];
};

_PROTOTYP( char * maksn, (char *,long, char *) );
_PROTOTYP( struct ck_sn * chksn, (char *) );
_PROTOTYP( VOID ck_encrypt, (char *) );
_PROTOTYP( VOID ck_decrypt, (char *) );
_PROTOTYP( int isregistered, (struct ck_registration *) );
_PROTOTYP( unsigned long regtime, (struct ck_registration *) );
_PROTOTYP( int setcrc, (struct ck_registration * ) ) ;
_PROTOTYP( char * get_reg_name, (void) ) ;
_PROTOTYP( char * get_reg_sn, (void) ) ;
_PROTOTYP( char * get_reg_corp, (void) ) ;
_PROTOTYP( int get_reg_count, (void) ) ;

_PROTOTYP( void Blowfish_encipher, (unsigned long *xl, unsigned long *xr) ) ;
_PROTOTYP( void Blowfish_decipher, (unsigned long *xl, unsigned long *xr) ) ;
_PROTOTYP( short InitializeBlowfish, (unsigned char key[], short keybytes) );
#endif /* CKOETC_H */
