/*  C K C D E B . H  */
/*
 This file is included by all C-Kermit modules, including the modules
 that aren't specific to Kermit (like the command parser and the ck?tio and
 ck?fio modules.  It specifies format codes for debug(), tlog(), and similar
 functions, and includes any necessary typedefs to be used by all C-Kermit
 modules, and also includes some feature selection compile-time switches.
*/
/*
 Copyright (C) 1987, 1989, Trustees of Columbia University in the City of New 
 York. Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/

/*
 DEBUG and TLOG should be defined in the Makefile if you want debugging
 and transaction logs.  Don't define them if you want to save the space
 and overhead.  (Note, in version 4F these definitions changed from "{}"
 to the null string to avoid problems with semicolons after braces, as in:
 "if (x) tlog(this); else tlog(that);"
*/
#ifndef DEBUG
#define debug(a,b,c,d)
#endif

#ifndef TLOG
#define tlog(a,b,c,d)
#endif

/* Formats for debug(), tlog(), etc */

#define F000 0
#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

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
    struct zstr date;     /* (#) file creation date [yy]yymmdd[ hh:mm[:ss]] */
    struct zstr creator;  /* ($) file creator id */
    struct zstr account;  /* (%) file account */
    struct zstr area;     /* (&) area (e.g. directory) for file */
    struct zstr passwd;   /* (') password for area */
    long blksize;         /* (() file blocksize */
    struct zstr access;   /* ()) file access: new, supersede, append, warn */
    struct zstr encoding; /* (*) encoding (transfer syntax) */
    struct zstr disp;     /* (+) disposition (mail, message, print, etc) */
    struct zstr lprotect; /* (,) protection (local syntax) */
    struct zstr gprotect; /* (-) protection (generic syntax) */
    struct zstr systemid; /* (.) ID for system of origin */
    struct zstr recfm;    /* (/) record format */
    struct zstr sysparam; /* (0) system-dependent parameter string */
    long length;          /* (1) exact length on system of origin */
};

/* Unix Version Dependencies */

/* signal() type, void or int? */
#ifdef SVR3
typedef void SIGTYP;			/* System V R3 and later */
#else
#ifdef SUNOS4
typedef void SIGTYP;			/* SUNOS V 4.0 and later */
#else
typedef int SIGTYP;
#endif
#endif

/* Systems that expand tilde at the beginning of file or directory names */
#ifdef BSD4
#define DTILDE
#endif
#ifdef UXIII
#define DTILDE
#endif
#ifdef OSK
#define DTILDE
#endif

/* C Compiler Dependencies */

#ifdef ZILOG
#define setjmp setret
#define longjmp longret
#define jmp_buf ret_buf
#define getcwd curdir
/* typedef int ret_buf[10]; (apparently duplicated in setret.h) */
#endif /* zilog */

#ifdef PROVX1
typedef char CHAR;
typedef long LONG;
typedef int void;
#else
#ifdef V7
typedef char CHAR;
typedef long LONG;
#else
#ifdef C70
typedef char CHAR;
typedef long LONG;
#else
#ifdef BSD29
typedef char CHAR;
typedef long LONG;
#else
typedef unsigned char CHAR;
typedef long LONG;
#endif
#endif
#endif
#endif

#ifdef TOWER1
typedef int void;
#endif

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
#endif
#endif

/*
 At this point, if there's a system that uses ordinary CRLF line
 delimitation AND the C compiler actually returns both the CR and
 the LF when doing input from a file, then #undef NLCHAR.
*/
#ifdef OS2
#undef NLCHAR
#endif

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
#define CTTNAM "/dev/tty"
#endif
#endif
#endif

/* Some special includes for VAX/VMS */

#ifndef vms
/* The following #includes cause problems for some preprocessors. */
/*
#endif
#ifdef vms
#include ssdef
#include stsdef
#endif
#ifndef vms
*/
#endif

/* Program return codes for VMS, DECUS C, and Unix */

#ifdef vms
#define GOOD_EXIT   (SS$_NORMAL | STS$M_INHIB_MSG)
#define BAD_EXIT    SS$_ABORT
#else
#ifdef decus
#define GOOD_EXIT   IO_NORMAL
#define BAD_EXIT    IO_ERROR
#else
#define GOOD_EXIT   0
#define BAD_EXIT    1
#endif
#endif

/* Special hack for Fortune, which doesn't have <sys/file.h>... */

#ifdef FT18
#define FREAD 0x01
#define FWRITE 0x10
#endif

/* special hack for os9/68k */
#ifdef OSK
#define SIGARB	5342			/* arbitrary user signal */
#define SIGALRM 5343			/* and another */
#endif
