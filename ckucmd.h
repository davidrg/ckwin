/*  C K U C M D . H  --  Header file for Unix cmd package  */
 
/*
  Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1993, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/
 
#ifndef CKUCMD_H
#define CKUCMD_H

/* Special getchars... */
 
#ifdef DYNAMIC				/* Dynamic command buffers */
/*
  Use malloc() to allocate the many command-related buffers in ckucmd.c.
*/
#ifdef pdp11				/* Not enough room */
#define NORECALL
#endif /* pdp11 */

#ifndef NORECALL
#define CK_RECALL
#else
#ifdef CK_RECALL
#undef CK_RECALL
#endif /* CK_RECALL */
#endif /* NORECALL */
#endif /* DYNAMIC */

#ifdef VMS
#ifdef getchar				/* This is for VMS GCC */
#undef getchar
#endif /* getchar */
#define getchar()   vms_getchar()
#endif /* VMS */
 
#ifdef aegis
#undef getchar
#define getchar()   coninc(0)
#endif /* aegis */
 
#ifdef AMIGA
#undef getchar
#define getchar() coninc(0)
#endif /* AMIGA */

/* Sizes of things */
 
#ifndef CMDDEP
#define CMDDEP  20			/* Maximum command recursion depth */
#endif /* CMDDEP */
#define HLPLW   78			/* Width of ?-help line */
#define HLPCW   19			/* Width of ?-help column */
#define HLPBL  100			/* Help string buffer length */
#define ATMBL  256			/* Command atom buffer length*/
#ifdef NOSPL
/* No script programming language, save some space */
#define CMDBL 512			/* Command buffer length */
#else
#define CMDBL 1024			/* Command buffer length */
#endif /* NOSPL */
 
/* Special characters */
 
#define RDIS 0022			/* Redisplay   (^R) */
#define LDEL 0025			/* Delete line (^U) */
#define WDEL 0027			/* Delete word (^W) */
#ifdef CK_RECALL
#define C_UP 0020			/* Go Up in recall buffer (^P) */
#define C_UP2 0002			/* Alternate Go Up (^B) for VMS */
#define C_DN 0016			/* Go Down in recall buffer (^N) */
#endif /* CK_RECALL */ 
/* Keyword table flags */
 
#define CM_INV 1			/* Invisible keyword */
#define CM_ABR 2			/* Abbreviation */
 
/* Token flags */

#define CMT_COM 0			/* Comment (; or #) */
#define CMT_SHE 1			/* Shell escape (!) */
#define CMT_LBL 2			/* Label (:) */
#define CMT_FIL 3			/* Indirect filespec (@) */

/* Keyword Table Template */
 
struct keytab {				/* Keyword table */
    char *kwd;				/* Pointer to keyword string */
    int kwval;				/* Associated value */
    int flgs;				/* Flags (as defined above) */
};

/* Function prototypes */

#ifdef CK_ANSIC				/* ANSI C */
#ifdef M_SYSV				/* SCO Microsoft C wants no args */
typedef int (*xx_strp)();
#else
typedef int (*xx_strp)(char *, char **, int *);
#endif /* M_SYSV */
#else					/* Not ANSI C */
typedef int (*xx_strp)();
#endif /* CK_ANSIC */

_PROTOTYP( int xxesc, (char **) );
_PROTOTYP( int cmrini, (int) );
_PROTOTYP( VOID cmsetp, (char *) );
_PROTOTYP( VOID cmsavp, (char [], int) );
_PROTOTYP( VOID prompt, (xx_strp) );
_PROTOTYP( VOID pushcmd, (void) );
_PROTOTYP( VOID cmres, (void) );
_PROTOTYP( VOID cmini, (int) );
_PROTOTYP( int cmpush, (void) );
_PROTOTYP( int cmpop, (void) );
_PROTOTYP( VOID untab, (char *) );
_PROTOTYP( int cmnum, (char *, char *, int, int *, xx_strp ) );
_PROTOTYP( int cmofi, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmifi, (char *, char *, char **, int *, xx_strp ) );
_PROTOTYP( int cmifi2,(char *, char *, char **, int *, int, xx_strp ) );
_PROTOTYP( int cmdir, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmfld, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmtxt, (char *, char *, char **, xx_strp ) );
_PROTOTYP( int cmkey, (struct keytab [], int, char *, char *, xx_strp) );
_PROTOTYP( int cmkey2,(struct keytab [], int, char *, char *, char *,xx_strp));
_PROTOTYP( int chktok, (char *) );
_PROTOTYP( int cmcfm, (void) );
_PROTOTYP( int rdigits, (char *) );
_PROTOTYP( int chknum, (char *) );
_PROTOTYP( int lower, (char *) );
_PROTOTYP( int lookup, (struct keytab [], char *, int, int *) );
_PROTOTYP( int ungword, (void) );
_PROTOTYP( int cmdsquo, (int) );
_PROTOTYP( int cmdgquo, (void) );

#ifdef DCMDBUF
_PROTOTYP( int cmsetup, (void) );
#endif /* DCMDBUF */

#endif /* CKUCMD_H */

/* End of ckucmd.h */
