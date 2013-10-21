/* C K N T E L  --  Kermit Telephony interface for MS Win32 TAPI systems */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
          Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/

#ifndef COMMENT
#define CK_TAPI
#endif

_PROTOTYP( int cktapiinit, (void) ) ;
_PROTOTYP( int cktapiopen, (void) ) ;
_PROTOTYP( int cktapiclose, (void) ) ;
_PROTOTYP( int cktapiload, (void) ) ;
_PROTOTYP( int cktapiunload, (void) ) ;
_PROTOTYP( int cktapidial, (char *) ) ;
_PROTOTYP( int cktapiBuildLineTable, (struct keytab **, int *) ) ;
_PROTOTYP( int cktapiBuildLocationTable, (struct keytab **, int *) ) ;

#define SUCCESS 0
extern int TAPIAvail ;
