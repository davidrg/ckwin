/*  C K C D E B . H  */

/*
  Fri Sep  6 23:23:05 1996

  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in system-specific modules when you they
  are supported.
*/

/*
  This file is included by all C-Kermit modules, including the modules
  that aren't specific to Kermit (like the command parser and the ck?tio and
  ck?fio modules).  It should be include BEFORE any other C-Kermit header
  files.  It specifies format codes for debug(), tlog(), and similar
  functions, and includes any necessary definitions to be used by all C-Kermit
  modules, and also includes some feature selection compile-time switches, and
  also system- or compiler-dependent definitions, plus #includes and prototypes
  required by all C-Kermit modules.
*/

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

/*
  Etymology: The name of this file means "C-Kermit Common-C-Language Debugging
  Header", because originally it contained only the formats (F000-F111) for
  the debug() and tlog() functions.  See how it has grown...
*/

#ifndef CKCDEB_H			/* Don't include me more than once. */
#define CKCDEB_H

#ifdef NOLOCAL               /* Things not needed for a remote-only version */
#ifndef NODIAL
#define NODIAL
#endif /* NODIAL */
#ifndef NOSCRIPT
#define NOSCRIPT
#endif /* NOSCRIPT */
#ifndef NOAPC
#define NOAPC
#endif /* NOAPC */
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#ifndef NOXMIT
#define NOXMIT
#endif /* NOXMIT */
#ifdef CK_CURSES
#undef CK_CURSES
#endif /* CK_CURSES */
#ifdef NETCONN
#undef NETCONN
#endif /* NETCONN */
#ifdef TCPSOCKET
#undef TCPSOCKET
#endif /* TCPSOCKET */
#endif /* NOLOCAL */

#ifdef NONET
#ifdef NETCONN
#undef NETCONN
#endif /* NETCONN */
#ifdef TCPSOCKET
#undef TCPSOCKET
#endif /* TCPSOCKET */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef STRATUSX25
#undef STRATUSX25
#endif /* STRATUSX25 */
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#ifdef NPIPE
#undef NPIPE
#endif /* NPIPE */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */


#endif /* NONET */

#ifndef DEFPAR				/* Default parity */
#define DEFPAR 0			/* Must be here because it is used */
#endif /* DEFPAR */			/* by all classes of modules */

#ifdef NT
#ifndef OS2ORWIN32
#define OS2ORWIN32
#endif /* OS2ORWIN32 */
#ifndef OS2
#define WIN32ONLY
#endif /* OS2 */
#endif /* NT */

#ifdef OS2				/* For OS/2 debugging */
#ifndef OS2ORWIN32
#define OS2ORWIN32
#endif /* OS2ORWIN32 */
#include "ckoker.h"
#ifdef NT
#include <windows.h>
#define NTSIG   
#else /* NT */
#define OS2ONLY
#include <os2def.h> 
#endif /* NT */
#ifndef OS2ORUNIX
#define OS2ORUNIX
#endif /* OS2ORUNIX */
#endif /* OS2 */

#include <stdio.h>			/* Begin by including this. */
#include <ctype.h>			/* and this. */

#ifdef MAC
/*
 * The MAC doesn't use standard stdio routines.
 */
#undef getchar
#define getchar()   mac_getchar()
#undef putchar
#define putchar(c)	mac_putchar(c)
#define printf		mac_printf
#define perror		mac_perror
#define puts		mac_puts
extern int mac_putchar (int c);
extern int mac_puts (const char *string);
extern int mac_printf(const char *, ...);
extern int mac_getchar (void);
#endif /* MAC */

#ifdef OS2
#define printf Vscrnprintf
#define fprintf Vscrnfprintf
extern int Vscrnprintf(const char *, ...);
extern int Vscrnfprintf(FILE *, const char *, ...);
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) Vscrnprintf("%c",x)
#define puts(x)    Vscrnprintf(x)
#define perror(x)  Vscrnperror(x)
#endif /* OS2 */

/* System-type compilation switches */

#ifdef FT21				/* Fortune For:Pro 2.1 implies 1.8 */
#ifndef FT18
#define FT18
#endif /* FT18 */
#endif /* FT21 */

#ifdef AIXPS2				/* AIXPS2 implies AIX370 */
#ifndef AIX370
#define AIX370
#endif /* AIX370 */
#endif /* AIXPS2 */

#ifdef AIX370				/* AIX PS/2 or 370 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* AIX370 */

#ifdef AIXESA				/* AIX/ESA implies BSD4.4 */
#ifndef BSD44
#define BSD44
#endif /* BSD44 */
#endif /* AIXESA */

#ifdef DGUX540				/* DG UX 5.40 implies Sys V R 4 */
#ifndef SVR4
#define SVR4
#endif /* SVR4 */
#endif /* DGUX540 */

#ifdef SUNOS41				/* SUNOS41 implies SUNOS4 */
#ifndef SUNOS4
#define SUNOS4
#endif /* SUNOS4 */
#endif /* SUNOS41 */

#ifdef SUN4S5				/* Sun-4 System V environment */
#ifndef SVR3				/* implies System V R3 or later */
#define SVR3
#endif /* SVR3 */
#endif /* SUN4S5 */

#ifdef MIPS				/* MIPS System V environment */
#ifndef SVR3				/* implies System V R3 or later */
#define SVR3
#endif /* SVR3 */
#endif /* MIPS */

#ifdef HPUX9				/* HP-UX 9.x */
#ifndef SVR3
#define SVR3
#endif /* SVR3 */
#ifndef HPUX
#define HPUX
#endif /* HPUX */
#endif /* HPUX9 */

#ifdef HPUX10				/* HP-UX 10.x */
#ifndef HPUX1010			/* If anything higher is defined */
#ifdef HPUX1020				/* define HPUX1010 too. */
#define HPUX1010
#endif /* HPUX1020 */
#ifdef HPUX1030
#define HPUX1010
#endif /* HPUX1030 */
#endif /* HPUX1010 */

#ifndef SVR4
#define SVR4
#endif /* SVR4 */
#ifndef HPUX
#define HPUX
#endif /* HPUX */
#endif /* HPUX10 */

#ifdef QNX				/* QNX Software Systems Inc */
#ifndef POSIX				/* QNX 4.0 or later is POSIX */
#define POSIX
#endif /* POSIX */
#ifndef __386__				/* Comes in 16-bit and 32-bit */
#define __16BIT__
#define CK_QNX16
#else
#define __32BIT__
#define CK_QNX32
#endif /* __386__ */
#endif /* QNX */

/*
  4.4BSD is a mixture of System V R4, POSIX, and 4.3BSD.
*/
#ifdef BSD44				/* 4.4 BSD */
#ifndef SVR4				/* BSD44 implies SVR4 */
#define SVR4
#endif /* SVR4 */
#ifndef NOSETBUF			/* NOSETBUF is safe */
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef DIRENT				/* Uses <dirent.h> */
#define DIRENT
#endif /* DIRENT */
#endif /* BSD44 */

#ifdef SVR3				/* SVR3 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* SVR3 */

#ifdef SVR4				/* SVR4 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#ifndef SVR3				/* ...as well as SVR3 */
#define SVR3
#endif /* SVR3 */
#endif /* SVR4 */

#ifdef OXOS
#ifndef ATTSV
#define ATTSV				/* OXOS implies ATTSV */
#endif /* ! ATTSV */
#define SW_ACC_ID			/* access() wants privs on */
#define kill priv_kill			/* kill() wants privs on */
#ifndef NOSETBUF
#define NOSETBUF			/* NOSETBUF is safe */
#endif /* ! NOSETBUF */
#endif /* OXOS */

#ifdef UTSV				/* UTSV implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* UTSV */

#ifdef XENIX				/* XENIX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* XENIX */

#ifdef AUX				/* AUX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* AUX */

#ifdef ATT7300				/* ATT7300 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ATT7300 */

#ifdef ATT6300				/* ATT6300 implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ATT6300 */

#ifdef HPUX				/* HPUX implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* HPUX */

#ifdef ISIII				/* ISIII implies ATTSV */
#ifndef ATTSV
#define ATTSV
#endif /* ATTSV */
#endif /* ISIII */

#ifdef NEXT33				/* NEXT33 implies NEXT */
#ifndef NEXT
#define NEXT
#endif /* NEXT */
#endif /* NEXT33 */

#ifdef NEXT				/* NEXT implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* NEXT */

#ifdef SUNOS4				/* SUNOS4 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* SUNOS4 */

#ifdef BSD41				/* BSD41 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* BSD41 */

#ifdef BSD43				/* BSD43 implies BSD4 */
#ifndef BSD4
#define BSD4
#endif /* BSD4 */
#endif /* BSD43 */

#ifdef BSD4				/* BSD4 implies ANYBSD */
#ifndef ANYBSD
#define ANYBSD
#endif /* ANYBSD */
#endif /* BSD4 */

#ifdef BSD29				/* BSD29 implies ANYBSD */
#ifndef ANYBSD
#define ANYBSD
#endif /* ANYBSD */
#endif /* BSD29 */

#ifdef ATTSV				/* ATTSV implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* ATTSV */

#ifdef ANYBSD				/* ANYBSD implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* ANYBSD */

#ifdef POSIX				/* POSIX implies UNIX */
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#ifndef DIRENT				/* and DIRENT, i.e. <dirent.h> */
#ifndef SDIRENT
#define DIRENT
#endif /* SDIRENT */
#endif /* DIRENT */
#ifndef NOFILEH				/* POSIX doesn't use <sys/file.h> */
#define NOFILEH
#endif /* NOFILEH */
#endif /* POSIX */

#ifdef V7
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* V7 */

#ifdef COHERENT
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* COHERENT */

#ifdef MINIX
#ifndef UNIX
#define UNIX
#endif /* UNIX */
#endif /* MINIX */
/*
  The symbol SVORPOSIX is defined for both AT&T and POSIX compilations
  to make it easier to select items that System V and POSIX have in common,
  but which BSD, V7, etc, do not have.
*/
#ifdef ATTSV
#ifndef SVORPOSIX
#define SVORPOSIX
#endif /* SVORPOSIX */
#endif /* ATTSV */

#ifdef POSIX
#ifndef SVORPOSIX
#define SVORPOSIX
#endif /* SVORPOSIX */
#endif /* POSIX */

/*
  The symbol SVR4ORPOSIX is defined for both AT&T System V R4 and POSIX 
  compilations to make it easier to select items that System V R4 and POSIX 
  have in common, but which BSD, V7, and System V R3 and earlier, etc, do
  not have.
*/
#ifdef POSIX
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif /* SVR4ORPOSIX */
#endif /* POSIX */
#ifdef SVR4
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif /* SVR4ORPOSIX */
#endif /* SVR4 */

/*
  The symbol BSD44ORPOSIX is defined for both 4.4BSD and POSIX compilations
  to make it easier to select items that 4.4BSD and POSIX have in common,
  but which System V, BSD, V7, etc, do not have.
*/
#ifdef BSD44
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif /* BSD44ORPOSIX */
#endif /* BSD44 */

#ifdef POSIX
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif /* BSD44ORPOSIX */
#endif /* POSIX */

#ifdef UNIX				/* For items common to OS/2 and UNIX */
#ifndef OS2ORUNIX
#define OS2ORUNIX
#endif /* OS2ORUNIX */
#endif /* UNIX */

#ifdef OS2
#define CK_ANSIC            /* OS/2 supports ANSIC and more extensions */
#endif /* OS2 */

#ifdef OSF40		/* Newer OSF/1 versions imply older ones */
#ifndef OSF32
#define OSF32
#endif /* OSF32 */
#endif /* OSF40 */

#ifdef OSF32
#ifndef OSF30
#define OSF30
#endif /* OSF30 */
#endif /* OSF32 */

#ifdef OSF30
#ifndef OSF20
#define OSF20
#endif /* OSF20 */
#endif /* OSF30 */

#ifdef OSF20
#ifndef OSF10
#define OSF10
#endif /* OSF10 */
#endif /* OSF20 */

#ifdef __DECC				/* For DEC Alpha AXP VMS or OSF/1 */
#ifndef CK_ANSIC
#define CK_ANSIC			/* Even with /stand=vaxc, need ansi */
#endif /* CKANSIC */
#ifndef SIG_V
#define SIG_V				/* and signal type is VOID */
#endif /* SIG_V */
#ifndef CK_ANSILIBS
#define CK_ANSILIBS			/* (Martin Zinser, Feb 1995) */
#endif /* CK_ANSILIBS */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif /* _POSIX_C_SOURCE */
#endif	/* __DECC */

#ifdef apollo				/* May be ANSI-C, check further */
#ifdef __STDCPP__
#define CK_ANSIC			/* Yes, this is real ANSI-C */
#define SIG_V
#else
#define NOANSI				/* Nope, not ANSI */
#undef __STDC__				/* Even though it say it is! */
#define SIG_I
#endif /* __STDCPP__ */
#endif /* apollo */

#ifdef POSIX				/* -DPOSIX on cc command line */
#ifndef _POSIX_SOURCE			/* Implies _POSIX_SOURCE */
#define _POSIX_SOURCE
#endif /* _POSIX_SOURCE */
#endif /* POSIX */

/*
  ANSI C?  That is, do we have function prototypes, new-style
  function declarations, and parameter type checking and coercion?
*/
#ifdef MAC				/* MPW C is ANSI */
#ifndef NOANSI
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* NOANSI */
#endif /* MAC */

#ifdef STRATUS				/* Stratus VOS */
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* STRATUS */

#ifndef NOANSI
#ifdef __STDC__				/* __STDC__ means ANSI C */
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* __STDC__ */
#endif /* NOANSI */
/*
  _PROTOTYP() is used for forward declarations of functions so we can have
  parameter and return value type checking if the compiler offers it.
  __STDC__ should be defined by the compiler only if function prototypes are
  allowed.  Otherwise, we get old-style forward declarations.  Our own private
  CK_ANSIC symbol tells whether we use ANSI C prototypes.  To force use of
  ANSI prototypes, include -DCK_ANSIC on the cc command line.  To disable the
  use of ANSI prototypes, include -DNOANSI.
*/
#ifdef CK_ANSIC
#define _PROTOTYP( func, parms ) func parms
#else /* Not ANSI C */
#define _PROTOTYP( func, parms ) func()
#endif /* CK_ANSIC */

/*
  Altos-specific items: 486, 586, 986 models...
*/
#ifdef A986
#define M_VOID
#define void int
#define CHAR char
#define SIG_I
#endif /* A986 */

/* Signal handling */

#ifdef QNX
#ifndef CK_POSIX_SIG
#define CK_POSIX_SIG
#endif /* CK_POSIX_SIG */
#endif /* QNX */

/* Void type */

#ifndef VOID				/* Used throughout all C-Kermit */
#ifdef CK_ANSIC				/* modules... */
#define VOID void
#else
#define VOID int
#endif /* CK_ANSIC */
#endif /* VOID */

/* Signal type */

#ifndef SIG_V				/* signal() type, if not def'd yet */
#ifndef SIG_I
#ifdef OS2
#define SIG_V
#else
#ifdef POSIX
#define SIG_V
#else
#ifdef SVR3				/* System V R3 and later */
#define SIG_V
#else
#ifdef SUNOS4				/* SUNOS V 4.0 and later */
#ifndef sun386
#define SIG_V 
#else
#define SIG_I
#endif /* sun386 */
#else
#ifdef NEXT				/* NeXT */
#define SIG_V
#else
#ifdef AIX370
#include <signal.h>
#define SIG_V
#define SIGTYP __SIGVOID		/* AIX370 */
#else
#ifdef STRATUS				/* Stratus VOS */
#define SIG_V
#else
#ifdef MAC
#define SIGTYP long
#define SIG_I
#ifndef MPW33
#define SIG_IGN 0
#endif /* MPW33 */
#define SIGALRM 1
#ifndef MPW33
#define SIGINT  2
#endif /* MPW33 */
#else /* Everything else */
#define SIG_I
#endif /* MAC */
#endif /* STRATUS */
#endif /* AIX370 */
#endif /* NEXT */
#endif /* SUNOS4 */
#endif /* SVR3 */
#endif /* POSIX */
#endif /* OS2 */
#endif /* SIG_I */
#endif /* SIG_V */

#ifdef SIG_I
#define SIGRETURN return(0)
#ifndef SIGTYP
#define SIGTYP int
#endif /* SIGTYP */
#endif /* SIG_I */

#ifdef SIG_V
#define SIGRETURN return
#ifndef SIGTYP
#define SIGTYP void
#endif /* SIGTYP */
#endif /* SIG_V */

#ifdef NT
#ifndef SIGTYP
#define SIGTYP void
#endif /* SIGTYP */

#define strdup _strdup
#endif /* NT */

#ifndef SIGTYP
#define SIGTYP int
#endif /* SIGTYP */

#ifndef SIGRETURN
#define SIGRETURN return(0)
#endif /* SIGRETURN */

#ifdef CKNTSIG
/* This does not work, so don't use it. */
#define signal ckntsignal
SIGTYP (*ckntsignal(int type, SIGTYP (*)(int)))(int);
#endif /* CKNTSIG */

/* We want all characters to be unsigned if the compiler supports it */

#ifdef KUI
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
#define CHAR unsigned char
#else
#ifdef PROVX1
typedef char CHAR;
/* typedef long LONG; */
typedef int void;
#else
#ifdef MINIX
typedef unsigned char CHAR;
#else
#ifdef V7
typedef char CHAR;
#else
#ifdef C70
typedef char CHAR;
/* typedef long LONG; */
#else
#ifdef BSD29
typedef char CHAR;
/* typedef long LONG; */
#else
#ifdef datageneral
#define CHAR unsigned char			/* 3.22 compiler */ 
#else
#ifdef HPUX
#define CHAR unsigned char
#else
#ifdef OS2
#ifdef NT     
#define CHAR unsigned char
#else /* NT */
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
typedef unsigned char CHAR;
#endif /* NT */
#else /* OS2 */
#ifdef VMS
typedef char CHAR;
#else
#ifdef CHAR
#undef CHAR
#endif /* CHAR */
typedef unsigned char CHAR;
#endif /* VMS */
#endif /* OS2 */
#endif /* HPUX */
#endif /* datageneral */
#endif /* BSD29 */
#endif /* C70 */
#endif /* V7 */
#endif /* MINIX */
#endif /* PROVX1 */
#endif /* KUI */

#ifdef OS2
_PROTOTYP( void bleep, (short) );
#else /* OS2 */
#define bleep(x) putchar('\07')
#endif /* OS2 */

#ifdef MAC				/* Macintosh file routines */
#ifndef CKWART_C			/* But not in "wart"... */
#ifdef feof
#undef feof
#endif /* feof */
#define feof mac_feof
#define rewind mac_rewind
#define fgets mac_fgets
#define fopen mac_fopen
#define fclose mac_fclose
int mac_feof();
void mac_rewind();
char *mac_fgets();
FILE *mac_fopen();
int mac_fclose();
#endif /* CKCPRO_W */
#endif /* MAC */
/*
   Systems whose mainline modules have access to the communication-line
   file descriptor, ttyfd.
*/
#ifndef CK_TTYFD
#ifdef UNIX
#define CK_TTYFD
#else
#ifdef OS2
#define CK_TTYFD
#else
#ifdef VMS
#define CK_TTYFD
#endif /* VMS */
#endif /* OS2 */
#endif /* UNIX */
#endif /* CK_TTYFD */

/* Temporary-directory-for-RECEIVE feature ... */
/* This says whether we have the isdir() function defined. */

#ifdef UNIX				/* UNIX has it */
#ifndef CK_TMPDIR
#ifndef pdp11
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* pdp11 */
#endif /* CK_TMPDIR */
#endif /* UNIX */

#ifdef VMS				/* VMS too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* VMS */

#ifdef OS2				/* OS two too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 129
#endif /* CK_TMPDIR */
#endif /* OS2 */

#ifdef STRATUS				/* Stratus VOS too. */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* STRATUS */

#ifdef OSK				/* OS-9 too */
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif /* CK_TMPDIR */
#endif /* OSK */

#ifdef CK_TMPDIR			/* Needs command parser */
#ifdef NOICP
#undef CK_TMPDIR
#endif /* NOICP */
#endif /* CK_TMPDIR */

/*
 Debug and transaction logging is included automatically unless you define
 NODEBUG or NOTLOG.  Do this if you want to save the space and overhead.
 (Note, in version 4F these definitions changed from "{}" to the null string
 to avoid problems with semicolons after braces, as in: "if (x) tlog(this);
 else tlog(that);"
*/
#ifndef NODEBUG
#ifndef DEBUG
#define DEBUG
#endif /* DEBUG */
#else
#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */
#endif /* NODEBUG */

#ifndef NOTLOG
#ifndef TLOG
#define TLOG
#endif /* TLOG */
#endif /* NOTLOG */

/* debug() macro style selection. */

#ifdef VMS
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* VMS */

#ifdef MAC
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* MAC */

#ifdef OS2
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* OS2 */

#ifdef OXOS				/* tst is faster than jsr */
#ifndef IFDEBUG
#define IFDEBUG
#endif /* IFDEBUG */
#endif /* OXOS */

#ifndef DEBUG
/* Compile all the debug() statements away.  Saves a lot of space and time. */
#define debug(a,b,c,d)
#else
#ifndef CKCMAI
/* Debugging included.  Declare debug log flag in main program only. */
extern int deblog;
#endif /* CKCMAI */
/* Now define the debug() macro. */
#ifdef IFDEBUG
/* Use this form to avoid function calls: */
#define debug(a,b,c,d) if (deblog) dodebug(a,b,(char *)c,(long)d)
#else
/* Use this form to save space: */
#define debug(a,b,c,d) dodebug(a,b,(char *)c,(long)d)
#endif /* MAC */

#ifdef COMMENT
#ifdef BEBOX
#undef debug
#define debug(a,b,c,d) if (deblog) printf("a=%1.1x:b=%s,c=%s,d=%0d\n",a,b,c,d)
#endif /* BEBOX */
#endif /* COMMENT */

_PROTOTYP(int dodebug,(int, char *, char *, long));
#endif /* DEBUG */

#ifndef TLOG
#define tlog(a,b,c,d)
#else
_PROTOTYP(VOID tlog,(int, char *, char *, long));
#endif /* TLOG */

/* Formats for debug() and tlog() */

#define F000 0
#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

/* Kermit feature selection */

#ifdef VMS				/* Features for all VMS builds */
#ifndef NOJC
#define NOJC
#endif /* NOJC */
#ifndef NOSETBUF
#define NOSETBUF
#endif /* NOSETBUF */
#ifndef DYNAMIC
#define DYNAMIC
#endif /* DYNAMIC */
#ifndef KANJI
#define KANJI
#endif /* KANJI */
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */

#endif /* VMS */

/*
  GETMSEC means getmsec() is available, which tells elapsed time in
  milliseconds.  Added in 6.0.192 - used only in debug log, to see how
  long a packet read or write takes.  Results are sometimes surprising.
*/
#ifdef DEBUG
#ifdef SUNOS41				
#ifndef GETMSEC
#define GETMSEC
#endif /*  GETMSEC */
#else
#ifdef NT
#ifndef GETMSEC
#define GETMSEC
#endif /*  GETMSEC */
#endif /* NT */
#endif /* SUNOS41 */
#endif /* DEBUG */

#ifdef GETMSEC
_PROTOTYP( long getmsec, (void) );
#endif /* GETMSEC */

#ifndef NOCKTIMERS			/* Dynamic timeouts */
#ifndef CK_TIMERS
#define CK_TIMERS
#endif /* CK_TIMERS */
#endif /* NOCKTIMERS */

#define CK_SPEED			/* Control-prefix removal */
#ifdef NOCKSPEED
#undef CK_SPEED
#endif /* NOCKSPEED */

#ifdef MAC				/* For Macintosh, no escape */
#define NOPUSH				/* to operating system */
#endif /* MAC */

/* Systems where we can call zmkdir() to create directories. */

#ifndef CK_MKDIR
#ifndef NOMKDIR

#ifdef UNIX
#ifndef pdp11
#define CK_MKDIR
#endif /* pdp11 */
#endif /* UNIX */

#ifdef OS2
#define CK_MKDIR
#endif /* OS2 */

#ifdef VMS
#define CK_MKDIR
#endif /* VMS */

#ifdef STRATUS
#define CK_MKDIR
#endif /* STRATUS */

#ifdef OSK
#define CK_MKDIR
#endif /* OSK */

#endif /* CK_MKDIR */
#endif /* NOMKDIR */

#ifdef NOMKDIR				/* Allow for command-line override */
#ifdef CK_MKDIR
#undef CK_MKDIR
#endif /* CK_MKDIR */
#endif /* NOMKDIR */

/* Systems for which we can enable the REDIRECT command automatically */

#ifndef CK_REDIR
#ifdef __linux__			/* Linux */
#define CK_REDIR
#else
#ifdef SUNOS41				/* SunOS 4.1 */
#define CK_REDIR
#else
#ifdef __bsdi__				/* BSD/386 */
#define CK_REDIR
#else
#ifdef SVR4				/* System V R4 */
#define CK_REDIR
#else
#ifdef ultrix				/* DEC ULTRIX */
#define CK_REDIR
#else
#ifdef AIXRS				/* RS/6000 AIX */
#define CK_REDIR
#else
#ifdef OSF				/* OSF/1 */
#define CK_REDIR
#else
#ifdef OS2				/* OS/2 */
#define CK_REDIR
#else
#ifdef NEXT				/* NEXT */
#define CK_REDIR
#endif /* NEXT */
#endif /* OS2 */
#endif /* OSF */
#endif /* AIXRS */
#endif /* ultrix */
#endif /* SVR4 */
#endif /* __bsdi__ */
#endif /* SUNOS41 */
#endif /* __linux__ */
#endif /* CK_REDIR */

#ifdef NOPUSH				/* But... REDIRECT command is not */
#ifdef CK_REDIR				/*  allowed if NOPUSH is defined. */
#undef CK_REDIR
#endif /* CK_REDIR */
#endif /* NOPUSH */

/* The following allows automatic enabling of REDIRECT to be overridden... */

#ifdef NOREDIRECT
#ifdef CK_REDIR
#undef CK_REDIR
#endif /* CK_REDIR */
#endif /* NOREDIRECT */

/* Versions where we support the RESEND command */

#ifndef NORESEND
#ifndef CK_RESEND
#ifdef UNIX
#ifndef pdp11
#define CK_RESEND
#endif /* pdp11 */
#endif /* UNIX */

#ifdef VMS
#define CK_RESEND
#endif /* VMS */

#ifdef OS2
#define CK_RESEND
#endif /* OS2 */

#ifdef AMIGA
#define CK_RESEND
#endif /* AMIGA */

#ifdef datageneral
#define CK_RESEND
#endif /* datageneral */

#ifdef STRATUS
#define CK_RESEND
#endif /* STRATUS */

#ifdef OSK
#define CK_RESEND
#endif /* OSK */

#endif /* CK_RESEND */
#endif /* NORESEND */

/* Systems implementing "Doomsday Kermit" protocol ... */

#ifndef DOOMSDAY
#ifdef UNIX
#define DOOMSDAY
#else
#ifdef VMS
#define DOOMSDAY
#else
#ifdef OS2
#define DOOMSDAY
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* DOOMSDAY */

/* Systems where we want the Thermometer to be used for fullscreen */

#ifdef OS2
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* CK_PCT_BAR */
#endif /* OS2 */

/* Systems where we have a REXX command */

#ifdef OS2
#ifdef __32BIT__
#ifndef NOREXX
#define CK_REXX
#endif /* NOREXX */
#endif /* __32BIT__ */
#endif /* OS2 */

/* Systems that have a ZRENAME function */

#define ZRENAME				/* They all do */

/* Systems that have a ZCOPY function */

#ifdef OS2
#define ZCOPY
#endif /* OS2 */

/* Systems that have ttgwsiz() (they all should but they don't) */

#ifndef CK_TTGWSIZ
#ifdef UNIX
#define CK_TTGWSIZ
#else
#ifdef VMS
#define CK_TTGWSIZ
#else
#ifdef OS2
#define CK_TTGWSIZ
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
#endif /* CK_TTGWSIZ */

/* OS/2 C-Kermit features not available in 16-bit version... */

#ifdef OS2
#ifndef __32BIT__
#ifdef PCFONTS				/* PC Font support */
#undef PCFONTS
#endif /* PCFONTS */
#ifdef NPIPE				/* Named Pipes communication */
#undef NPIPE
#endif /* NPIPE */
#ifdef CK_NETBIOS			/* NETBIOS communication */
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef OS2MOUSE				/* Mouse */
#undef OS2MOUSE
#endif /* OS2MOUSE */
#ifdef OS2PM				/* Presentation Manager */
#undef OS2PM
#endif /* OS2PM */
#ifdef CK_REXX				/* Rexx */
#undef CK_REXX
#endif /* CK_REXX */
#endif /* __32BIT__ */
#endif /* OS2 */

/* OS/2 C-Kermit features not available in Windows NT version... */

#ifdef OS2
#ifdef NT
#ifdef PCFONTS				/* PC Font support */
#undef PCFONTS
#endif /* PCFONTS */
#ifdef NPIPE				/* Named Pipes communication */
#undef NPIPE
#endif /* NPIPE */
#ifdef CK_NETBIOS			/* NETBIOS communication */
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#ifdef OS2PM				/* Presentation Manager */
#undef OS2PM
#endif /* OS2PM */
#ifdef CK_REXX				/* Rexx */
#undef CK_REXX
#endif /* CK_REXX */
#endif /* NT */
#endif /* OS2 */

/*
  Systems that have select().
  This is used for both msleep() and for read-buffer checking in in_chk().
*/
#define CK_SLEEPINT 250 /* milliseconds - set this to something that 
                           divides evenly into 1000 */
#ifndef SELECT
#ifndef NOSELECT
#ifdef __linux__
#define SELECT
#else
#ifdef SUNOS4
#define SELECT
#else
#ifdef NEXT
#define SELECT
#else
#ifdef HPUX
#define SELECT
#else
#ifdef AIXRS
#define SELECT
#else
#ifdef BSD44
#define SELECT
#else
#ifdef BSD4
#define SELECT
#else
#ifdef OXOS
#define SELECT
#else
#ifdef OS2
#define SELECT
#endif /* OS2 */
#endif /* OXOS */
#endif /* BSD4 */
#endif /* BSD44 */
#endif /* AIXRS */
#endif /* HPUX */
#endif /* NEXT */
#endif /* __linux__ */
#endif /* SUNOS4 */
#endif /* NOSELECT */
#endif /* SELECT */

/*
  CK_NEED_SIG is defined if the system cannot check the console to
  to see if characters are waiting.  This is used during local-mode file
  transfer to interrupt the transfer, refresh the screen display, etc.
  If CK_NEED_SIG is defined, then file-transfer interruption characters
  have to be preceded a special character, e.g. the SIGQUIT character.
  CK_NEED_SIG should be defined if the conchk() function is not operational.
*/
#ifdef NOPOLL				/* For overriding CK_POLL definition */
#ifdef CK_POLL
#undef CK_POLL
#endif /* CK_POLL */
#endif /* NOPOLL */

#ifndef CK_POLL				/* If we don't have poll() */
#ifndef RDCHK				/* And we don't have rdchk() */
#ifndef SELECT				/* And we don't have select() */
#ifdef ATTSV
#ifndef aegis
#ifndef datageneral
#ifndef OXOS
#define CK_NEED_SIG
#endif /* OXOS */
#endif /* datageneral */
#endif /* aegis */
#endif /* ATTSV */
#ifdef POSIX
#ifndef CK_NEED_SIG
#define CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* POSIX */
#endif /* SELECT */
#endif /* RDCHK */
#endif /* CK_POLL */

#ifdef HPUX				/* HP-UX has select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* HPUX */

#ifdef AIXRS				/* AIX has select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* AIXRS */

#ifdef BSD44				/* 4.4BSD has FIONREAD */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* BSD44 */

#ifdef QNX				/* QNX has FIONREAD and select() */
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif /* CK_NEED_SIG */
#endif /* QNX */

/*
  Automatic parity detection.
  This actually implies a lot more now: length-driven packet reading,
  "Doomsday Kermit" IBM Mainframe file transfer through 3270 data streams, etc.
*/
#ifdef UNIX				/* For Unix */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* UNIX */

#ifdef VMS				/* ... and VMS */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#ifdef __GNUC__
#define VMSGCC
#endif /* __GNUC__ */
#endif /* VMS */

#ifdef MAC				/* and Macintosh */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* MAC */

#ifdef STRATUS				/* and Stratus VOS */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* STRATUS */

#ifdef OS2				/* and OS/2, finally */
#ifndef NOPARSEN
#define PARSENSE
#endif /* NOPARSEN */
#endif /* OS2 */

#ifdef DYNAMIC				/* If DYNAMIC is defined */
#define DCMDBUF				/* then also define this. */
#endif /* DYNAMIC */

#ifndef CK_LBRK				/* Can send Long BREAK */

#ifdef UNIX				/* (everybody but OS-9) */
#define CK_LBRK
#endif /* UNIX */
#ifdef VMS
#define CK_LBRK
#endif /* VMS */
#ifdef datageneral
#define CK_LBRK
#endif /* datageneral */
#ifdef GEMDOS
#define CK_LBRK
#endif /* GEMDOS */
#ifdef OS2
#define CK_LBRK
#endif /* OS2 */
#ifdef AMIGA
#define CK_LBRK
#endif /* AMIGA */

#endif /* CK_LBRK */

/* Carrier treatment */
/* These are defined here because they are shared by the system dependent */
/* and the system independent modules. */

#define  CAR_OFF 0	/* Off: ignore carrier always. */
#define  CAR_ON  1      /* On: heed carrier always, except during DIAL. */
#define  CAR_AUT 2      /* Auto: heed carrier, but only if line is declared */
			/* to be a modem line, and only during CONNECT. */

/* Hangup by modem command supported by default */

#ifndef NODIAL
#ifndef NOMDMHUP
#ifndef MDMHUP
#define MDMHUP 
#endif /* MDMHUP */
#endif /* NOMDMHUP */
#endif /* NODIAL */

/*
  Serial interface speeds available.  We assume that everybody supports:
  0, 110, 300, 600, 1200, 2400, 4800, and 9600 bps.  Symbols for other speeds
  are defined here.  You can also add definitions on the CC command lines.
  These definitions affect the SET SPEED keyword table, and are not necessarily
  usable in the system-dependent speed-setting code in the ck?tio.c modules,
  which depends on system-specific symbols like (in UNIX) B19200.  In other
  words, just defining it doesn't mean it'll work -- you also have to supply
  the supporting code in ttsspd() and ttgspd() in ck?tio.c.

  The symbols have the form BPS_xxxx, where xxxx is the speed in bits per
  second, or (for bps values larger than 9999) thousands of bps followed by K.
  The total symbol length should be 8 characters or less.  Some values are
  enabled automatically below.  You can disable a particular value by defining
  NOB_xxxx on the CC command line.
*/
#ifndef NOB_50
#define BPS_50				/* 50 bps */
#endif

#ifndef NOB_75
#define BPS_75				/* 75 bps */
#endif

#ifndef NOB7512
#ifdef ANYBSD
#define BPS_7512			/* 75/1200 Split Speed */
#endif /* ANYBSD */
#endif /* NOB7512 */

#undef BPS_134				/* 134.5 bps (IBM 2741) */

#ifndef NOB_150
#define BPS_150				/* 150 bps */
#endif

#ifndef NOB_200
#define BPS_200				/* 200 bps */
#endif

#ifndef NOB_1800
#ifdef MAC
#define BPS_1800			/* 1800 bps */
#endif
#endif

#ifndef NOB_3600
#define BPS_3600			/* 3600 bps */
#endif

#ifndef NOB_7200
#define BPS_7200			/* 7200 bps */
#endif

#ifndef NOB_14K
#ifdef OS2
#define BPS_14K				/* 14400 bps */
#else
#ifdef NEXT
#define BPS_14K
#else
#ifdef MAC
#define BPS_14K
#else
#ifdef AMIGA
#define BPS_14K
#endif /* AMIGA */
#endif /* MAC */
#endif /* NEXT */
#endif /* OS2 */
#endif /* NOB_14K */

#ifndef NOB_19K
#define BPS_19K				/* 19200 bps */
#endif

#ifndef NOB_28K
#ifdef NEXT
#define BPS_28K				/* 28800 bps */
#else
#ifdef MAC
#define BPS_28K				/* 28800 bps */
#endif /* MAC */
#endif /* NEXT */
#endif /* NOB_28K */

#ifndef NOB_38K
#define BPS_38K				/* 38400 bps */
#endif

/*
  Speeds of 57600 and higher are supported in Linux 0.99.15 and later, but
  it seems to do no harm to enable them for earlier releases too, in which
  case commands like "set speed 57600" simply fail with an "Unsupported line
  speed" message.
*/
#ifndef NOHISPEED
#ifdef __linux__
#define LINUXHISPEED
#endif /* __linux__ */
#endif /* NOHISPEED */

#ifndef NOB_57K
#ifdef Plan9
#define BPS_57K
#endif /* Plan9 */
#ifdef VMS
#define BPS_57K				/* 57600 bps */
#else
#ifdef OS2
#define BPS_57K
#else
#ifdef __linux__
#ifdef LINUXHISPEED
#define BPS_57K
#endif /* LINUXHISPEED */
#else
#ifdef HPUX
#define BPS_57K
#else
#ifdef NEXT
#define BPS_57K
#else
#ifdef __386BSD__
#define BPS_57K
#else
#ifdef __FreeBSD__
#define BPS_57K
#else
#ifdef __NetBSD__
#define BPS_57K
#else
#ifdef MAC
#define BPS_57K
#else
#ifdef QNX
#define BPS_57K
#else
#ifdef BEBOX
#define BPS_57K
#endif /* BEBOX */
#endif /* QNX */
#endif /* MAC */
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* NEXT */
#endif /* HPUX */
#endif /* __linux__ */
#endif /* OS2 */
#endif /* VMS */
#endif /* NOB_57K */

#ifndef NOB_76K
#ifdef Plan9
#define BPS_76K
#endif /* Plan9 */
#ifdef VMS
#define BPS_76K				/* 76800 bps */
#endif /* VMS */
#ifdef OS2
#ifdef __32BIT__
#define BPS_76K
#endif /* __32BIT__ */
#endif /* OS2 */
#ifdef QNX
#define BPS_76K
#endif /* QNX */
#endif /* NOB_76K */

#ifndef NOB_115K
#ifdef Plan9
#define BPS_115K
#endif /* Plan9 */
#ifdef VMS
#define BPS_115K			/* 115200 bps */
#else
#ifdef QNX
#define BPS_115K
#else
#ifdef HPUX
#define BPS_115K
#else
#ifdef __linux__
#ifdef LINUXHISPEED
#define BPS_115K
#endif /* LINUXHISPEED */
#else
#ifdef __386BSD__
#define BPS_115K
#else
#ifdef __FreeBSD__
#define BPS_115K
#else
#ifdef __NetBSD__
#define BPS_115K
#else
#ifdef OS2
#ifdef __32BIT__
#define BPS_115K
#endif /* __32BIT__ */
#else
#ifdef BEBOX
#define BPS_115K
#endif /* BEBOX */
#endif /* OS2 */
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* __linux__ */
#endif /* HPUX */
#endif /* QNX */
#endif /* VMS */
#endif /* NOB_115K */

#ifndef NOB_230K			/* 230400 bps */
#ifdef OS2
#ifdef __32BIT__            
#define BPS_230K
#endif /* __32BIT__ */
#else 
#undef BPS_230K
#endif /* OS2 */
#endif /* NOB_230K */

#ifdef BPS_230K				/* Maximum speed defined */
#define MAX_SPD 230400L
#else
#ifdef BPS_115K
#define MAX_SPD 115200L
#else
#ifdef BPS_76K
#define MAX_SPD 76800L
#else
#ifdef BPS_57K
#define MAX_SPD 57600L
#else
#ifdef BPS_38K
#define MAX_SPD 38400L
#else
#ifdef BPS_28K
#define MAX_SPD 28800L
#else
#ifdef BPS_19K
#define MAX_SPD 19200L
#else
#ifdef BPS_14K
#define MAX_SPD 14400L
#else
#define MAX_SPD 9600L
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifndef CONGSPD				/* Systems that can call congspd() */
#ifdef UNIX
#define CONGSPD
#endif /* UNIX */
#ifdef VMS
#define CONGSPD
#endif /* VMS */
#endif /* CONGSPD */

/* Types of flow control available */

#define CK_XONXOFF			/* Everybody can do this, right? */

#ifdef AMIGA				/* Commodore Amiga */
#define CK_RTSCTS			/* has RTS/CTS */
#endif /* AMIGA */

#ifdef SUN4S5				/* SunOS in System V environment */
#define CK_RTSCTS
#else					/* SunOS 4.0/4.1 in BSD environment */
#ifdef SUNOS4				/* SunOS 4.0+later supports RTS/CTS */
#ifdef SUNOS41				/* Easy in 4.1 and later */
#define CK_RTSCTS
#else					/* Harder in 4.0 */
#ifndef __GNUC__			/* (see tthflow() in ckutio.c) */
#ifndef GNUC
#define CK_RTSCTS			/* Only if not using GNU gcc */
#endif /* __GNUC__ */
#endif /* GNUC */
#endif /* SUNOS41 */
#endif /* SUNOS4 */
#endif /* SUN4S5 */

#ifdef BSD44				/* And in 4.4 BSD, including BSDI */
#define CK_RTSCTS
#endif /* BSD44 */

#ifdef TERMIOX				/* Sys V R4 <termiox.h> */
#define CK_RTSCTS			/* has RTS/CTS */
#define CK_DTRCD			/* and DTR/CD */
#endif /* TERMIOX */
#ifdef STERMIOX				/* Sys V R4 <sys/termiox.h> */
#define CK_RTSCTS			/* Ditto. */
#define CK_DTRCD
#endif /* STERMIOX */

#ifdef OXOS				/* Olivetti X/OS R2 struct termios */
#define CK_RTSCTS			/* Ditto. */
#define CK_DTRCD
#endif /* OXOS */

#ifdef AIXRS				/* RS/6000 with AIX 3.x */
#define CK_RTSCTS			/* Has its own peculiar method... */
#endif /* AIXRS */

#ifdef __linux__			/* Linux */
#define CK_RTSCTS
#endif /* __linux__ */
/*
  Hardware flow control is not defined in POSIX.1.  Nevertheless, a certain
  style API for hardware flow control, using tcsetattr() and the CRTSCTS
  bit(s), seems to be gaining currency on POSIX-based UNIX systems.  The
  following code defines the symbol POSIX_CRTSCTS for such systems.
*/
#ifdef __bsdi__				/* BSDI, a.k.a. BSD/386 */
#define POSIX_CRTSCTS
#endif /* __bsdi__ */
#ifdef __linux__			/* Linux */
#define POSIX_CRTSCTS
#endif /* __linux__ */
#ifdef __NetBSD__			/* NetBSD */
#define POSIX_CRTSCTS
#endif /* __NetBSD__ */
#ifdef BEBOX
#define POSIX_CRTSCTS
/* BEBOX defines CRTSFL as (CTSFLOW & RTSFLOW) */
#define CRTSCTS CRTSFL
#endif /* BEBOX */

/* Implementations that have implemented the ttsetflow() function. */

#ifndef CK_TTSETFLOW
#ifdef UNIX
#define CK_TTSETFLOW
#endif /* UNIX */
#endif /* CK_TTSETFLOW */

/*
 Systems where we can expand tilde at the beginning of file or directory names
*/
#ifdef POSIX
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* POSIX */
#ifdef BSD4
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* BSD4 */
#ifdef ATTSV
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* ATTSV */
#ifdef OSK
#ifndef DTILDE
#define DTILDE
#endif /* DTILDE */
#endif /* OSK */
#ifdef HPUX				/* I don't know why this is */
#ifndef DTILDE				/* necessary, since -DHPUX */
#define DTILDE				/* automatically defines ATTSV */
#endif /* DTILDE */			/* (see above) ... */
#endif /* HPUX */

/*
  This is mainly for the benefit of ckufio.c (UNIX and OS/2 file support).
  Systems that have an atomic rename() function, so we don't have to use
  link() and unlink().
*/
#ifdef POSIX
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* POSIX */

#ifdef OS2
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* OS2 */

#ifdef SUNOS41
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* SUNOS41 */

#ifdef SVR4
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* SVR4 */

#ifdef AIXRS
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* AIXRS */

#ifdef BSD44
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* BSD44 */

#ifdef NORENAME				/* Allow for compile-time override */
#ifdef RENAME
#undef RENAME
#endif /* RENAME */
#endif /* NORENAME */

#ifdef STRATUS				/* Stratus VOS */
#ifndef RENAME
#define RENAME
#endif /* RENAME */
#endif /* STRATUS */

/* Line delimiter for text files */

/*
 If the system uses a single character for text file line delimitation,
 define NLCHAR to the value of that character.  For text files, that
 character will be converted to CRLF upon output, and CRLF will be converted
 to that character on input during text-mode (default) packet operations.
*/
#ifdef MAC                              /* Macintosh */
#define NLCHAR 015
#else
#ifdef OSK				/* OS-9/68K */
#define NLCHAR 015
#else                                   /* All Unix-like systems */
#define NLCHAR 012
#endif /* OSK */
#endif /* MAC */

/*
 At this point, if there's a system that uses ordinary CRLF line
 delimitation AND the C compiler actually returns both the CR and
 the LF when doing input from a file, then #undef NLCHAR.
*/
#ifdef OS2				/* OS/2 */
#undef NLCHAR
#endif /* OS2 */

#ifdef GEMDOS				/* Atari ST */
#undef NLCHAR
#endif /* GEMDOS */

/*
  VMS file formats are so complicated we need to do all the conversion 
  work in the CKVFIO module, so we tell the rest of C-Kermit not to fiddle
  with the bytes.
*/

#ifdef vms
#undef NLCHAR
#endif /* vms */

/* The device name of a job's controlling terminal */
/* Special for VMS, same for all Unixes (?), not used by Macintosh */

#ifdef vms
#define CTTNAM "TT:"
#else
#ifdef datageneral
#define CTTNAM "@output"
#else
#ifdef OSK
extern char myttystr[];
#define CTTNAM myttystr
#else
#ifdef OS2
#define CTTNAM "con"
#else
#ifdef UNIX
#define CTTNAM "/dev/tty"
#else
#ifdef GEMDOS
#define CTTNAM "aux:"
#else
#ifdef STRATUS
extern char myttystr[];
#define CTTNAM myttystr
#else /* Anyone else... */
#define CTTNAM "stdout"			/* This is a kludge used by Mac */
#endif /* STRATUS */
#endif /* GEMDOS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* OSK */
#endif /* datageneral */
#endif /* vms */

#ifndef ZFCDAT				/* zfcdat() function available? */
#ifdef UNIX
#define  ZFCDAT
#else
#ifdef STRATUS
#define  ZFCDAT
#else
#ifdef GEMDOS
#define  ZFCDAT
#else
#ifdef AMIGA
#define  ZFCDAT
#else
#ifdef OS2
#define  ZFCDAT
#else
#ifdef datageneral
#define  ZFCDAT
#else
#ifdef VMS
#define  ZFCDAT
#endif /* VMS */
#endif /* datageneral */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* GEMDOS */
#endif /* STRATUS */
#endif /* UNIX */
#endif /* ZFCDAT */

#ifdef SUNS4S5
#define tolower _tolower
#define toupper _toupper
#endif /* SUNS4S5 */

/* Error number */

#ifdef _CRAY				
#ifdef _CRAYCOM				/* Cray Computer Corp. */
extern int errno;
#else /* _CRAYCOM */
#include <errno.h>			/* Cray Research UNICOS defines */
					/* errno as a function. */
#endif /* _CRAYCOM */			/* OK for UNICOS 6.1 and 7.0. */
#else /* _CRAY */
#ifdef STRATUS				/* Stratus VOS */
#include <errno.h>
#else /* not STRATUS */
#ifndef VMS
#ifndef OS2
/*
  The following declaration causes problems for VMS and OS/2, in which
  errno is an "extern volatile int noshare"...
*/
extern int errno;			/* Needed by most modules. */
#endif /* OS2 */
#endif /* VMS */
#endif /* STRATUS */
#endif /* _CRAY */

#ifndef BIGBUFOK			/* Platforms with lots of memory */

#ifdef sparc				/* SPARC processors */
#define BIGBUFOK
#endif /* sparc */

#ifdef mips				/* MIPS processors */
#define BIGBUFOK
#endif /* mips */

#ifdef HPUX10				/* HP-UX 10.0 PA-RISC */
#define BIGBUFOK
#endif /* HPUX10 */

#ifdef NEXT				/* NeXTSTEP */
#ifdef mc68000				/* on NEXT platforms... */
#define BIGBUFOK
#endif /* mc68000 */
#endif /* NEXT */

#ifdef OS2				/* 32-bit OS/2 2.x */
#ifdef __32BIT__
#define BIGBUFOK
#endif /* __32BIT__ */
#ifdef NT
#define BIGBUFOK
#endif /* NT */
#endif /* OS2 */

#ifdef Plan9				/* Plan 9 is OK */
#define BIGBUFOK
#endif /* Plan9 */

#ifdef VMS				/* Any VMS is OK */
#define BIGBUFOK
#endif /* VMS */

#ifdef __alpha				/* DEC 64-bit Alpha AXP, e.g. OSF/1 */
#ifndef BIGBUFOK			/* Might already be defined for VMS */
#define BIGBUFOK
#endif /* BIGBUFOK */
#endif /* __alpha */

#ifdef sgi				/* SGI with IRIX 4.0 or later */
#define BIGBUFOK
#endif /* sgi */

#endif /* BIGBUFOK */

/* File System Defaults */

#ifdef VMS
#define DBLKSIZ 512
#define DLRECL 512
#else
#define DBLKSIZ 0
#define DLRECL 0
#endif

/* Program return codes for DECUS C and UNIX (VMS uses UNIX codes) */

#ifdef decus
#define GOOD_EXIT   IO_NORMAL
#define BAD_EXIT    IO_ERROR
#else
#define GOOD_EXIT   0
#define BAD_EXIT    1
#endif /* decus */

/* Special hack for Fortune, which doesn't have <sys/file.h>... */

#ifdef FT18
#define FREAD 0x01
#define FWRITE 0x10
#endif /* FT18 */

/* Special hack for OS-9/68k */
#ifdef OSK
#ifndef _UCC
#define SIGALRM 30			/* May always cancel I/O */
#endif /* _UCC */
#define SIGARB	1234			/* Arbitrary for I/O */
SIGTYP (*signal())();
#endif /* OSK */

#ifdef OS2
#ifdef putchar                  /* MSC 5.1 simply uses a macro which causes */
#undef putchar                  /* no problems. */
#endif /* putchar */
#endif /* OS2 */

#ifdef MINIX
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(c) {putc(c,stdout);fflush(stdout);}
#endif /* MINIX */

#ifdef datageneral			/* Data General AOS/VS */
#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(c) conoc(c)
#endif /* datageneral */

/* Escape/quote character used by the command parser */

#define CMDQ '\\' 

/* Symbols for RS-232 modem signals */

#define KM_FG    1			/* Frame ground */
#define KM_TXD   2			/* Transmit */
#define KM_RXD   3			/* Receive */
#define KM_RTS   4			/* Request to Send */
#define KM_CTS   5			/* Clear to Send */
#define KM_DSR   6			/* Data Set Ready */
#define KM_SG    7			/* Signal ground */
#define KM_DCD   8			/* Carrier Detect */
#define KM_DTR  20			/* Data Terminal Ready */
#define KM_RI   22			/* Ring Indication */

/* Bit mask values for modem signals */

#define BM_CTS   0001			/* Clear to send       (From DCE) */
#define BM_DSR   0002			/* Dataset ready       (From DCE) */
#define BM_DCD   0004			/* Carrier             (From DCE) */
#define BM_RNG   0010			/* Ring Indicator      (From DCE) */
#define BM_DTR   0020			/* Data Terminal Ready (From DTE) */
#define BM_RTS   0040			/* Request to Send     (From DTE) */

/* Codes for full duplex flow control */

#define FLO_NONE 0			/* None */
#define FLO_XONX 1			/* Xon/Xoff (soft) */
#define FLO_RTSC 2			/* RTS/CTS (hard) */
#define FLO_DTRC 3			/* DTR/CD (hard) */
#define FLO_ETXA 4			/* ETX/ACK (soft) */
#define FLO_STRG 5			/* String-based (soft) */
#define FLO_DIAL 6			/* DIALing kludge */
#define FLO_DIAX 7			/* Cancel dialing kludge */
#define FLO_DTRT 8			/* DTR/CTS (hard) */
#define FLO_KEEP 9			/* Keep, i.e. don't touch or change */
#define FLO_AUTO 10			/* Figure out automatically */

/* And finally... */

#ifdef COMMENT				/* Make sure this is NOT defined! */
#undef COMMENT
#endif /* COMMENT */

/* Structure definitions for Kermit file attributes */
/* All strings come as pointer and length combinations */
/* Empty string (or for numeric variables, -1) = unused attribute. */

struct zstr {             /* string format */
    int len;	          /* length */
    char *val;            /* value */
};
struct zattr {            /* Kermit File Attribute structure */
    long lengthk;         /* (!) file length in K */
    struct zstr type;     /* (") file type (text or binary) */
    struct zstr date;     /* (#) file creation date yyyymmdd[ hh:mm[:ss]] */
    struct zstr creator;  /* ($) file creator id */
    struct zstr account;  /* (%) file account */
    struct zstr area;     /* (&) area (e.g. directory) for file */
    struct zstr password; /* (') password for area */
    long blksize;         /* (() file blocksize */
    struct zstr xaccess;  /* ()) file access: new, supersede, append, warn */
    struct zstr encoding; /* (*) encoding (transfer syntax) */
    struct zstr disp;     /* (+) disposition (mail, message, print, etc) */
    struct zstr lprotect; /* (,) protection (local syntax) */
    struct zstr gprotect; /* (-) protection (generic syntax) */
    struct zstr systemid; /* (.) ID for system of origin */
    struct zstr recfm;    /* (/) record format */
    struct zstr sysparam; /* (0) system-dependent parameter string */
    long length;          /* (1) exact length on system of origin */
    struct zstr charset;  /* (2) transfer syntax character set */
#ifdef OS2
    struct zstr longname; /* OS/2 longname if applicable */
#endif /* OS2 */ 
    struct zstr reply;    /* This goes last, used for attribute reply */
};

/* Kermit file information structure */

struct filinfo {
  int bs;				/* Blocksize */
  int cs;				/* Character set */
  long rl;				/* Record length */
  int org;				/* Organization */
  int fmt;				/* Record format */
  int cc;				/* Carriage control */
  int typ;				/* Type (text/binary) */
  int dsp;				/* Disposition */
  char *os_specific;			/* OS-specific attributes */
#ifdef OS2
  unsigned long int lblopts; /* LABELED FILE options bitmask */
#else
  int lblopts; 
#endif /* OS2 */
};

#ifndef ZFNQFP				/* Versions that have zfnqfp() */
#ifdef UNIX
#define ZFNQFP
#else
#ifdef VMS
#define ZFNQFP
#else
#ifdef OS2
#define ZFNQFP
#endif /* OS2 */
#endif /* VMS */
#endif /* UNIX */
struct zfnfp {
   int len;
   char * fpath;
   char * fname;
};
#endif /* ZFNQFP */

/* Systems that support FILE TYPE LABELED */

#ifdef VMS
#define CK_LABELED
#else
#ifdef OS2
#ifdef __32BIT__
#ifndef NT
#define CK_LABELED
#endif /* NT */
#endif /* __32BIT__ */
#endif /* OS2 */
#endif /* VMS */

/* LABELED FILE options bitmask */

#ifdef VMS				/* For VMS */
#define LBL_NAM  1			/* Ignore incoming name if set */
#define LBL_PTH  2			/* Use complete path if set */
#define LBL_ACL  4			/* Preserve ACLs if set */
#define LBL_BCK  8			/* Preserve backup date if set */
#define LBL_OWN 16			/* Preserve ownership if set */

#else

#ifdef OS2				/* Ditto for OS/2 */
#define LBL_NOR  0x0000			/* Normal file */
#define LBL_ARC  0x0020			/* Archive */
#define LBL_DIR  0x0010			/* Directory */
#define LBL_HID  0x0002			/* Hidden file */
#define LBL_RO   0x0001			/* Read only file */
#define LBL_SYS  0x0004			/* System file */
#define LBL_EXT  0x0040			/* Extended */
#endif /* OS2 */
#endif /* VMS */

/*
  Data types.  First the header file for data types so we can pick up the
  types used for pids, uids, and gids.  Override this section by putting
  -DCKTYP_H=xxx on the command line to specify the header file where your
  system defines these types.
*/
#ifndef STRATUS
#ifdef __ALPHA
#ifdef MULTINET
#define CK_TGV_AXP
#endif /* MULTINET */
#endif /* __ALPHA */

#ifdef CK_TGV_AXP			/* Alpha, VMS, MultiNet */
/*
  Starting in DECC 5.0, <stdlib.h> no longer includes <types.h>.
  But before that an elaborate workaround is required, which results in
  including <types.h> sometimes but not others, evidently depending on whether
  <types.h> protects itself against multiple inclusion, which in turn probably
  differentiates between DECC <types.h> and TGV <types.h>.  Unfortunately I
  don't remember the details.  (fdc, 25 Oct 96)
*/
#ifdef COMMENT
/*
  Previously the test here was for DEC version prior to 4.0, but since the
  test involved an "#if" statement, it was not portable and broke some non-VMS
  builds.  In any case, condition was never satisfied, so the result of
  commenting this section out is the same as the previous "#if" condition.
*/
#ifndef __TYPES_LOADED
#define __TYPES_LOADED			/* Work around bug in .h files */
#endif /* __TYPES_LOADED */
#endif /* COMMENT */
#include <sys/types.h>
#else					/* !CK_TGV_AXP */
#ifdef OSK				/* OS-9 */
#include <types.h>
#else					/* General case, not OS-9 */
#ifndef CKTYP_H
#ifndef VMS
#ifndef MAC
#ifndef AMIGA
#define CKTYP_H <sys/types.h>
#endif /* AMIGA */
#endif /* MAC */
#endif /* VMS */
#endif /* CKTYP_H */

#ifdef GEMDOS
#undef CKTYP_H
#include <types.h>
#endif /* GEMDOS */

#ifdef OS2
#undef CKTYP_H
#include <sys/types.h>
#endif /* OS2 */

#ifdef CKTYP_H				/* Include it. */
#ifdef COHERENT				/* Except for COHERENT */
#include <sys/types.h>
#else
#ifdef datageneral			/* AOS/VS */
#include <sys/types.h>
#else
#ifdef __bsdi__				/* BSDI */
#ifdef POSIX
#undef _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#include CKTYP_H
#ifdef __bsdi__
#ifdef POSIX
#define _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#endif /* datageneral */
#endif /* COHERENT */
#endif /* CKTYP_H */

#endif /* OSK */
#endif /* CK_TGV_AXP */
#endif /* STRATUS */			/* End of types.h section */

/*
  Data type for pids.  If your system uses a different type, put something
  like -DPID_T=pid_t on command line, or override here.
*/
#ifndef PID_T
#define PID_T int
#endif /* PID_T */
/*
  Data types for uids and gids.  Same deal as for pids.
  Wouldn't be nice if there was a preprocessor test to find out if a
  typedef existed?
*/
#ifdef VMS
/* Not used in VMS so who cares */
#define UID_T int
#define GID_T int
#endif /* VMS */

#ifdef POSIX
/* Or would it be better (or worse?) to use _POSIX_SOURCE here? */
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not POSIX */
#ifdef SVR4
/* SVR4 and later have uid_t and gid_t. */
/* SVR3 and earlier use int, or unsigned short, or.... */
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not SVR4 */
#ifdef BSD43
#ifndef UID_T
#define UID_T uid_t
#endif /* UID_T */
#ifndef GID_T
#define GID_T gid_t
#endif /* GID_T */
#else /* Not BSD43 */
/* Default these to int for older UNIX versions */
#ifndef UID_T
#define UID_T int
#endif /* UID_T */
#ifndef GID_T
#define GID_T int
#endif /* GID_T */
#endif /* BSD43 */
#endif /* SVR4  */
#endif /* POSIX */

/* 
  getpwuid() arg type, which is not necessarily the same as UID_T,
  e.g. in SCO UNIX SVR3, it's int.
*/
#ifndef PWID_T
#define PWID_T UID_T
#endif /* PWID_T */

#ifdef NEXT
#define MACHWAIT
#else
#ifdef MACH
#define MACHWAIT
#endif /* MACH */
#endif /* NEXT */

#ifdef MACHWAIT				/* WAIT_T argument for wait() */
#include <sys/wait.h>
#define CK_WAIT_H
typedef union wait WAIT_T;
#else
#ifdef POSIX
#include <sys/wait.h>
#define CK_WAIT_H
#ifndef WAIT_T
typedef int WAIT_T;
#endif /* WAIT_T */
#else /* !POSIX */
typedef int WAIT_T;
#endif /* POSIX */
#endif /* MACHWAIT */

/* Forward declarations of system-dependent functions callable from all */
/* C-Kermit modules. */

/* File-related functions from system-dependent file i/o module */

#ifndef CKVFIO_C
/* For some reason, this does not agree with DEC C */
_PROTOTYP( int zkself, (void) );
#endif /* CKVFIO_C */
_PROTOTYP( int zopeni, (int, char *) );
_PROTOTYP( int zopeno, (int, char *, struct zattr *, struct filinfo *) );
_PROTOTYP( int zclose, (int) );
#ifndef MAC
_PROTOTYP( int zchin, (int, int *) );
#endif /* MAC */
_PROTOTYP( int zsinl, (int, char *, int) );
_PROTOTYP( int zinfill, (void) );
_PROTOTYP( int zsout, (int, char*) );
_PROTOTYP( int zsoutl, (int, char*) );
_PROTOTYP( int zsoutx, (int, char*, int) );
_PROTOTYP( int zchout, (int, char) );
_PROTOTYP( int zoutdump, (void) );
_PROTOTYP( int zsyscmd, (char *) );
_PROTOTYP( int zshcmd, (char *) );
_PROTOTYP( int chkfn, (int) );
_PROTOTYP( long zchki, (char *) );
_PROTOTYP( int iswild, (char *) );
_PROTOTYP( int isdir, (char *) );
_PROTOTYP( int zchko, (char *) );
_PROTOTYP( int zdelet, (char *) );
_PROTOTYP( VOID zrtol, (char *,char *) );
_PROTOTYP( VOID zltor, (char *,char *) );
_PROTOTYP( VOID zstrip, (char *,char **) );
_PROTOTYP( int zchdir, (char *) );
_PROTOTYP( char * zhome, (void) );
_PROTOTYP( char * zgtdir, (void) );
_PROTOTYP( int zxcmd, (int, char *) );
#ifndef MAC
_PROTOTYP( int zclosf, (int) );
#endif /* MAC */
_PROTOTYP( int zxpand, (char *) );
_PROTOTYP( int znext, (char *) );
_PROTOTYP( int zchkspa, (char *, long) );
_PROTOTYP( VOID znewn, (char *, char **) );
_PROTOTYP( int zrename, (char *, char *) );
_PROTOTYP( int zcopy, (char *, char *) );
_PROTOTYP( int zsattr, (struct zattr *) );
_PROTOTYP( int zfree, (char *) );
_PROTOTYP( char * zfcdat, (char *) );
_PROTOTYP( int zstime, (char *, struct zattr *, int) );
_PROTOTYP( int zmail, (char *, char *) ); 
_PROTOTYP( int zprint, (char *, char *) ); 
_PROTOTYP( char * tilde_expand, (char *) ); 
_PROTOTYP( int zmkdir, (char *) ) ;
_PROTOTYP( int zfseek, (long) ) ;
_PROTOTYP( struct zfnfp * zfnqfp, (char *, int, char * ) ) ;
#ifdef OS2
_PROTOTYP( int os2setlongname, ( char * fn, char * ln ) ) ;
_PROTOTYP( int os2getlongname, ( char * fn, char ** ln ) ) ;
_PROTOTYP( int os2rexx, ( char *, char *, int ) ) ;
_PROTOTYP( int os2rexxfile, ( char *, char *, char *, int) ) ;
_PROTOTYP( int os2geteas, (char *) ) ;
_PROTOTYP( int os2seteas, (char *) ) ;
_PROTOTYP( char * get_os2_vers, (void) ) ;
_PROTOTYP( int do_label_send, (char *) ) ;
_PROTOTYP( int do_label_recv, (void) ) ;
#ifdef OS2MOUSE
_PROTOTYP( unsigned long os2_mouseon, (void) );
_PROTOTYP( unsigned long os2_mousehide, (void) );
_PROTOTYP( unsigned long os2_mouseshow, (void) );
_PROTOTYP( unsigned long os2_mouseoff, (void) );
_PROTOTYP( void os2_mouseevt, (void *) );
#endif /* OS2MOUSE */
#endif /* OS2 */

/* Functions from system-dependent terminal i/o module */

_PROTOTYP( int ttopen, (char *, int *, int, int) );  /* tty functions */
#ifndef MAC
_PROTOTYP( int ttclos, (int) );
#endif /* MAC */
_PROTOTYP( int tthang, (void) );
_PROTOTYP( int ttres, (void) );
_PROTOTYP( int ttpkt, (long, int, int) );
#ifndef MAC
_PROTOTYP( int ttvt, (long, int) );
#endif /* MAC */
_PROTOTYP( int ttsspd, (int) );
_PROTOTYP( long ttgspd, (void) );
_PROTOTYP( int ttflui, (void) );
_PROTOTYP( int ttfluo, (void) );
_PROTOTYP( int ttgwsiz, (void) );
_PROTOTYP( int ttchk, (void) );
_PROTOTYP( int ttxin, (int, CHAR *) );
_PROTOTYP( int ttxout, (CHAR *, int) );
_PROTOTYP( int ttol, (CHAR *, int) );
_PROTOTYP( int ttoc, (char) );
_PROTOTYP( int ttinc, (int) );
_PROTOTYP( int ttscarr, (int) );
_PROTOTYP( int ttgmdm, (void) );
_PROTOTYP( int ttsndb, (void) );
_PROTOTYP( int ttsndlb, (void) );
#ifdef PARSENSE
#ifdef UNIX
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef VMS
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef STRATUS
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef OS2
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
#ifdef OSK
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR, int) );
#else
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR, CHAR) );
#endif /* OSK */
#endif /* OS2 */
#endif /* STRATUS */
#endif /* VMS */
#endif /* UNIX */
#else /* ! PARSENSE */
_PROTOTYP( int ttinl, (CHAR *, int, int, CHAR) );
#endif /* PARSENSE */

/* XYZMODEM support */

/*
  CK_XYZ enables the various commands and data structures.
  XYZ_INTERNAL means these protocols are built-in; if not defined,
  then they are external.  XYZ_DLL is used to indicate a separate
  loadable library containing the XYZmodem protocol code.
*/
#ifndef NOCKXYZ

#ifdef pdp11				/* No room for this in PDP-11 */
#define NOCKXYZ
#endif /* pdp11 */

#ifndef CK_XYZ
#ifdef UNIX
#define CK_XYZ
#else
#ifdef OS2
#define CK_XYZ
#define XYZ_INTERNAL			/* Internal and DLL */
#ifndef NOXYZDLL
#define XYZ_DLL
#endif /* NOXYZDLL */
#endif /* OS2 */
#endif /* UNIX */
#endif /* CK_XYZ */
#endif /* NOCKXYZ */

#ifdef XYZ_INTERNAL			/* This ensures that XYZ_INTERNAL */
#ifndef CK_XYZ				/* is defined only if CK_XYZ is too */
#undef XYZ_INTERNAL
#endif /* CK_XYZ */
#endif /* XYZ_INTERNAL */
#ifdef XYZ_DLL				/* This ensures XYZ_DLL is defined */
#ifndef XYZ_INTERNAL			/* only if XYZ_INTERNAL is too */
#undef XYZ_DLL
#endif /* XYZ_INTERNAL */
#endif /* XYZ_DLL */

/* Console functions */

_PROTOTYP( int congm, (void) );
#ifdef COMMENT
_PROTOTYP( VOID conint, (SIGTYP (*)(int, int), SIGTYP (*)(int, int)) );
#else
_PROTOTYP( VOID conint, (SIGTYP (*)(int), SIGTYP (*)(int)) );
#endif /* COMMENT */
_PROTOTYP( VOID connoi, (void) );
_PROTOTYP( int concb, (char) );
#ifdef CONGSPD
_PROTOTYP( long congspd, (void) );
#endif /* CONGSPD */
_PROTOTYP( int conbin, (char) );
_PROTOTYP( int conres, (void) );
_PROTOTYP( int conoc, (char) );
_PROTOTYP( int conxo, (int, char *) );
_PROTOTYP( int conol, (char *) );
_PROTOTYP( int conola, (char *[]) );
_PROTOTYP( int conoll, (char *) );
_PROTOTYP( int conchk, (void) );
_PROTOTYP( int coninc, (int) );
_PROTOTYP( char * conkbg, (void) );
_PROTOTYP( int psuspend, (int) );
_PROTOTYP( int priv_ini, (void) );
_PROTOTYP( int priv_on, (void) );
_PROTOTYP( int priv_off, (void) );
_PROTOTYP( int priv_can, (void) );
_PROTOTYP( int priv_chk, (void) );
_PROTOTYP( int priv_opn, (char *, int) );

_PROTOTYP( int sysinit, (void) );	/* Misc Kermit functions */
_PROTOTYP( int syscleanup, (void) );
_PROTOTYP( int msleep, (int) );
_PROTOTYP( VOID rtimer, (void) );
_PROTOTYP( int gtimer, (void) );
_PROTOTYP( VOID ttimoff, (void) );
_PROTOTYP( VOID ztime, (char **) );
_PROTOTYP( int parchk, (CHAR *, CHAR, int) );
_PROTOTYP( VOID doexit, (int, int) );
_PROTOTYP( int askmore, (void) );
_PROTOTYP( VOID fatal, (char *) );
_PROTOTYP( VOID fatal2, (char *, char *) );
_PROTOTYP( int ckindex, (char *, char *, int, int, int) );
#ifdef VMS
_PROTOTYP( int ck_cancio, (void) );
#endif /* VMS */

/* Key mapping support */

#ifdef NOICP
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#endif /* NOICP */

#ifdef MAC
#ifndef NOSETKEY
#define NOSETKEY
#endif /* NOSETKEY */
#endif /* MAC */

_PROTOTYP( int congks, (int) );
#ifndef NOSETKEY
#ifdef OS2
#define KMSIZE 8916
typedef ULONG KEY;
typedef CHAR *MACRO;
extern int wideresult;
#else /* Not OS2 */
/*
  Catch-all for systems where we don't know how to read keyboard scan
  codes > 255.  Note: CHAR (i.e. unsigned char) is very important here.
*/
#define KMSIZE 256
typedef CHAR KEY;
typedef CHAR * MACRO;
#define congks coninc
#endif /* OS2 */
#endif /* NOSETKEY */

#ifndef NOKVERBS			/* No \Kverbs unless... */
#define NOKVERBS
#endif /* NOKVERBS */

#ifdef OS2				/* \Kverbs are supported in OS/2 */
#undef NOKVERBS
/*
  Note: this value chosen to be bigger than PC BIOS key modifier bits,
  but still fit in 16 bits without affecting sign.

  As of K95 1.1.5, this no longer fits in 16 bits, good thing we are 32 bit.
*/
#define F_MACRO 0x2000          /* Bit indicating a macro indice */
#define IS_MACRO(x) (x & F_MACRO)
#define F_KVERB 0x4000			/* Bit indicating a keyboard verb */
#define IS_KVERB(x) (x & F_KVERB)	/* Test this bit */
#endif /* OS2 */

#define F_ESC   0x8000		/* Bit indicating ESC char combination */
#define IS_ESC(x) (x & F_ESC)
#define F_CSI   0x10000		/* Bit indicating CSI char combination */
#define IS_CSI(x) (x & F_CSI)

#ifdef NOSPL				/* This might be overkill.. */
#ifndef NOKVERBS			/* Not all \Kverbs require */
#define NOKVERBS			/* the script programming language. */
#endif /* NOKVERBS */
#endif /* NOSPL */

/*
  Function prototypes for system and library functions.
*/
#ifdef _POSIX_SOURCE
#ifndef VMS
#ifndef MAC
#define CK_ANSILIBS
#endif /* MAC */
#endif /* VMS */
#endif /* _POSIX_SOURCE */

#ifdef NEXT
#define CK_ANSILIBS
#endif /* NEXT */

#ifdef SVR4
#define CK_ANSILIBS
#endif /* SVR4 */

#ifdef STRATUS				/* Stratus VOS uses ANSI libraries */
#define CK_ANSILIBS
#endif /* STRATUS */

#ifdef OS2
#define CK_ANSILIBS
#define MYCURSES
#define CK_RTSCTS
#ifdef __IBMC__
#define S_IFMT 0xF000
#define timezone _timezone
#endif /* __IBMC__ */
#include <fcntl.h>
#include <io.h>
#ifdef __EMX__
#ifndef __32BIT__
#define __32BIT__
#endif /* __32BIT__ */
#include <sys/timeb.h>
#else
#include <direct.h>
#ifdef OS2
#undef SIGALRM
#endif /* OS2 */
#ifndef SIGUSR1
#define SIGUSR1 7
#endif /* SIGUSR1 */
#define SIGALRM SIGUSR1
_PROTOTYP( unsigned alarm, (unsigned) );
_PROTOTYP( unsigned sleep, (unsigned) );
#endif /* __EMX__ */
#ifdef OS2
_PROTOTYP( unsigned long zdskspace, (int) );
#else
_PROTOTYP( long zdskspace, (int) );
#endif /* OS2 */
_PROTOTYP( int zchdsk, (int) );
_PROTOTYP( int conincraw, (int) );
_PROTOTYP( int ttiscom, (int f) );
_PROTOTYP( int IsFileNameValid, (char *) );
_PROTOTYP( void ChangeNameForFAT, (char *) );
_PROTOTYP( char *GetLoadPath, (void) );
#endif /* OS2 */

/* Fullscreen file transfer display items... */

#ifdef MYCURSES				/* MYCURSES implies CK_CURSES */
#ifndef CK_CURSES
#define CK_CURSES
#endif /* CK_CURSES */
#endif /* MYCURSES */

/*
  The CK_WREFRESH symbol is defined if the curses library provides
  clearok() and wrefresh() functions, which are used in repainting
  the screen.
*/
#ifdef NOWREFRESH			/* Override CK_WREFRESH */

#ifdef CK_WREFRESH			/* If this is defined, */
#undef CK_WREFRESH			/* undefine it. */
#endif /* CK_WREFRESH */

#else /* !NOWREFRESH */			/* No override... */

#ifndef CK_WREFRESH			/* If CK_WREFRESH not defined */
/*
  Automatically define it for systems known to have it ...
*/
#ifdef VMS				/* DEC (Open)VMS has it */
#define CK_WREFRESH
#else
#ifdef ultrix				/* DEC ULTRIX has it */
#else
#ifdef SVR3				/* System V has it */
#define CK_WREFRESH
#else
#ifdef BSD44				/* 4.4 BSD has it */
#define CK_WREFRESH
#else
#ifdef NEXT				/* Define it for NeXTSTEP */
#define CK_WREFRESH
#else
#ifdef SUNOS4				/* SunOS 4.x... */
#define CK_WREFRESH
#else
#ifdef AIXRS				/* RS/6000 AIX ... */
#define CK_WREFRESH
#else
#ifdef RTAIX				/* RT PC AIX ... */
#define CK_WREFRESH
#else
#ifdef OSF				/* DEC OSF/1 ... */
#define CK_WREFRESH

/* Add more here, or just define CK_WREFRESH on the CC command line... */

#endif /* OSF */
#endif /* RTAIX */
#endif /* AIXRS */
#endif /* SUNOS4 */
#endif /* NEXT */
#endif /* BSD44 */
#endif /* SVR3 */
#endif /* ultrix */
#endif /* VMS */

#else /* CK_WREFRESH is defined */

#ifndef CK_CURSES			/* CK_WREFRESH implies CK_CURSES */
#define CK_CURSES
#endif /* CK_CURSES */

#endif /* CK_WREFRESH */
#endif /* NOWREFRESH */

#ifdef CK_CURSES			/* Termcap buffer size for curses */
#ifndef TRMBUFL
#define TRMBUFL 1024
#endif /* TRMBUFL */
#endif /* CK_CURSES */

#ifdef CK_ANSILIBS
/*
  String library functions.
  For ANSI C, get prototypes from <string.h>.
  Otherwise, skip the prototypes.
*/
#include <string.h>

/*
  Prototypes for other commonly used library functions, such as
  malloc, free, getenv, atol, atoi, and exit.  Otherwise, no prototypes.
*/
#include <stdlib.h>
#ifdef DIAB /* DIAB DS90 */
/* #include <commonC.h>  */
#include <sys/wait.h>
#define CK_WAIT_H
#ifdef COMMENT
extern void exit(int status);
extern void _exit(int status);
extern int uname(struct utsname *name);
#endif /* COMMENT */
extern int chmod(char *path, int mode);
extern int ioctl(int fildes, int request, ...);
extern int rdchk(int ttyfd);
extern int nap(int m);
#ifdef COMMENT
extern int getppid(void);
#endif /* COMMENT */
extern int _filbuf(FILE *stream);
extern int _flsbuf(char c,FILE *stream);
#endif /* DIAB */

/*
  Prototypes for UNIX functions like access, alarm, chdir, sleep, fork,
  and pause.  Otherwise, no prototypes.
*/
#ifdef VMS
#include <unixio.h>
#endif /* VMS */

#ifdef NEXT
#ifndef NEXT33
#include <libc.h>
#endif /* NEXT33 */
#else
#ifndef AMIGA
#ifndef OS2
#ifdef STRATUS
#include <c_utilities.h>
#else /* !STRATUS */
#include <unistd.h>
#endif /* STRATUS */
#endif /* OS2 */
#endif /* AMIGA */
#endif /* NEXT */

#else /* Not ANSI libs... */

#ifdef MAC
#include <String.h>
#include <StdLib.h>
#endif /* MAC */

#ifdef SUNOS41
#include <unistd.h>
#include <stdlib.h>
#else
#ifndef MAC
/*
  It is essential that these are declared correctly!
  Which is not always easy.  Take malloc() for instance ...
*/
#ifdef PYRAMID
#ifdef SVR4
#ifdef __STDC__
#define SIZE_T_MALLOC
#endif /* __STDC__ */
#endif /* SVR4 */
#endif /* PYRAMID */
/*
  Maybe some other environments need the same treatment for malloc.
  If so, define SIZE_T_MALLOC for them here or in compiler CFLAGS.
*/
#ifdef SIZE_T_MALLOC
_PROTOTYP( void * malloc, (size_t) );
#else
_PROTOTYP( char * malloc, (unsigned int) );
#endif /* SIZE_T_MALLOC */

_PROTOTYP( char * getenv, (char *) );
_PROTOTYP( long atol, (char *) );
#endif /* !MAC */
#endif /* SUNOS41 */
#endif /* CK_ANSILIBS */

#ifndef NULL				/* In case NULL is still not defined */
#define NULL 0L
/* or #define NULL 0 */
/* or #define NULL ((char *) 0) */
/* or #define NULL ((void *) 0) */
#endif /* NULL */

/* Maximum length for a fully qualified filename, not counting \0 at end. */
/*
  This is a rough cut, and errs on the side of being too big.  We don't 
  want to pull in hundreds of header files looking for many and varied
  symbols, for fear of introducing unnecessary conflicts.
*/
#ifndef CKMAXPATH
#ifdef MAXPATHLEN			/* (it probably isn't) */
#define CKMAXPATH MAXPATHLEN
#else
#ifdef MAC
#define CKMAXPATH 63
#else
#ifdef pdp11
#define CKMAXPATH 255
#else
#ifdef UNIX				/* Even though some are way less... */
#define CKMAXPATH 1023
#else
#ifdef VMS
#define CKMAXPATH 675			/* (derivation is complicated...) */
#else
#ifdef STRATUS
#define CKMAXPATH 256			/* == $MXPL from PARU.H */
#else
#ifdef datageneral
#define CKMAXPATH 256			/* == $MXPL from PARU.H */
#else
#define CKMAXPATH 255
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIX */
#endif /* pdp11 */
#endif /* MAC */
#endif /* MAXPATHLEN */
#endif /* CKMAXPATH */

/* Funny names for library functions department... */

#ifdef ZILOG
#define setjmp setret
#define longjmp longret
#define jmp_buf ret_buf
#define getcwd curdir
#endif /* ZILOG */

#ifdef STRATUS
/* The C-runtime conflicts with things we do in Stratus VOS ckltio.c ... */
#define printf vosprtf
_PROTOTYP( int vosprtf, (char *fmt, ...) );
#define perror(txt) printf("%s\n", txt)
/* char_varying is a string type from PL/I that VOS uses extensively */
#define CV char_varying
#endif /* STRATUS */

#ifdef NT
extern int OSVer;
#define isWin95() (OSVer==VER_PLATFORM_WIN32_WINDOWS)
#else
#define isWin95() (0)
#endif /* NT */

#endif /* CKCDEB_H */

/* End of ckcdeb.h */
