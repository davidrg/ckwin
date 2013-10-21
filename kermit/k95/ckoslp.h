/* C K O S L P -- Kermit interface to the IBM SLIP driver */

/*
  Authors:
    Jeffrey E Altman (jaltman@secure-endpoints.com),
      Secure Endpoints Inc., New York City, and
    David Bolen (db3l@ans.net),
      Advanced Networking and Services, Inc.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

_PROTOTYP( APIRET SlipOpen, ( char * ) ) ;
_PROTOTYP( APIRET PPPOpen, ( char * ) ) ;
_PROTOTYP( void PPPSlipClose, (void) ) ;
