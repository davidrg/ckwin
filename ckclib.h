/* ckclib.h -- C-Kermit library routine prototypes */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKCLIB_H
#define CKCLIB_H

/* Base-64 conversion needed for script programming and HTTP */

#ifdef CK_ANSIC
_PROTOTYP( int ckstrncpy, (char *, const char *, int) );
#else
_PROTOTYP( int ckstrncpy, (char *, char *, int) );
#endif /* CK_ANSIC */
_PROTOTYP( char * chartostr, (int) );
_PROTOTYP( int cklower, (char *) );
_PROTOTYP( int ckindex, (char *, char *, int, int, int) );
_PROTOTYP( char * ckitoa, (int) );
_PROTOTYP( char * ckltoa, (long) );
_PROTOTYP( int ckmatch, (char *, char *, int, int ) );
_PROTOTYP( VOID ckmemcpy, (char *, char *, int) );
_PROTOTYP( char * ckstrchr, (char *, char) );
_PROTOTYP( int ckrchar, (char *) );
_PROTOTYP( int ckstrcmp, (char *, char *, int, int) );
#define xxstrcmp(a,b,c) ckstrcmp(a,b,c,0)
_PROTOTYP( int ckstrpre, (char *, char *) );
_PROTOTYP( VOID sh_sort, (char **, char **, int, int, int, int) );
_PROTOTYP( char * brstrip, (char *) );
_PROTOTYP( VOID makelist, (char *, char *[], int) );
#ifndef CK_ANSIC
_PROTOTYP( VOID makestr, (char **, char *) );
_PROTOTYP( VOID xmakestr, (char **, char *) );
#else /* CK_ANSIC */
_PROTOTYP( VOID makestr, (char **, const char *) );
_PROTOTYP( VOID xmakestr, (char **, const char *) );
#endif /* CK_ANSIC */
_PROTOTYP( int fileselect, (char *,
			    char *, char *, char *, char *,
			    long, long,
			    int, int,
			    char **) );
_PROTOTYP( int chknum, (char *) );
_PROTOTYP( int rdigits, (char *) );
_PROTOTYP( char * ckradix, (char *,int,int) );

#ifndef NOB64
_PROTOTYP( int b8tob64, (char *,int,char *,int));
_PROTOTYP( int b64tob8, (char *,int,char *,int));
#endif /* NOB64 */

#ifdef CKFLOAT
_PROTOTYP( int isfloat, (char *,int) );
#ifndef CKCLIB_C
#ifndef CKWART_C
extern CKFLOAT floatval;
#endif /* CKWART_C */
#endif /* CKCLIB_C */
#endif /* CKFLOAT */

_PROTOTYP( char * parnam, (char) );
_PROTOTYP( char *hhmmss, (long) );

_PROTOTYP( VOID lset, (char *, char *, int, int) );
_PROTOTYP( VOID rset, (char *, char *, int, int) );
_PROTOTYP( char * ulongtohex, (unsigned long, int) );
_PROTOTYP( long hextoulong, (char *, int) );

#endif /* CKCLIB_H */
