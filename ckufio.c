#ifdef OS2
char *ckzv = "OS/2 File support, 5A(098) 4 Oct 94";
#else
#ifdef aegis
char *ckzv = "Aegis File support, 5A(098) 4 Oct 94";
#else
char *ckzv = "UNIX File support, 5A(098) 4 Oct 94";
#endif /* aegis */
#endif /* OS2 */

/* C K U F I O  --  Kermit file system support for UNIX, OS/2, and Aegis */


/*
  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in system-specific modules when you they
  are supported.
*/

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

/* Include Files */

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"

#include <signal.h>

#ifdef MINIX
#include <limits.h>
#endif /* MINIX */
#ifdef POSIX
#include <limits.h>
#endif /* POSIX */

/* Directory structure header file */

#ifdef OS2
/*

  C-Kermit's OS/2 support originally by Chris Adie <C.Adie@uk.ac.edinburgh>
  Edinburgh University Computing Service, Scotland, for C-Kermit 4F.  Adapted
  to C-Kermit 5A and integrated into the UNIX support module by Kai Uwe Rommel
  <rommel@informatik.tu-muenchen.de>, Muenchen, Germany, December 1991.
*/

/*
  Directory Separator macros, to allow this module to work with both UNIX and
  OS/2: Because of ambiguity with the command line editor escape \ character,
  the directory separator is currently left as / for OS/2 too, because the
  OS/2 kernel also accepts / as directory separator.  But this is subject to
  change in future versions to conform to the normal OS/2 style.
*/
#define DIRSEP       '/'
/* #define DIRSEP       '\\' */
#define ISDIRSEP(c)  ((c)=='/'||(c)=='\\')
#else /* not OS2 */
#define DIRSEP       '/'
#define ISDIRSEP(c)  ((c)=='/')
#endif /* OS2 */

#ifdef SDIRENT
#define DIRENT
#endif /* SDIRENT */

#ifdef XNDIR
#include <sys/ndir.h>
#else /* !XNDIR */
#ifdef NDIR
#include <ndir.h>
#else /* !NDIR, !XNDIR */
#ifdef RTU
#include "/usr/lib/ndir.h"
#else /* !RTU, !NDIR, !XNDIR */
#ifdef DIRENT
#ifdef SDIRENT
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif /* SDIRENT */
#else
#ifdef OS2
#define OPENDIR
#define DIRENT
#include "ckodir.h"
#else/* !RTU, !NDIR, !XNDIR, !DIRENT, !OS2, i.e. all others */
#include <sys/dir.h>
#endif /* OS2 */
#endif /* DIRENT */
#endif /* RTU */
#endif /* NDIR */
#endif /* XNDIR */

#ifdef OS2				/* OS/2 file system interface */
#define BSD4				/* is like Berkeley UNIX */
#define NOFILEH				/* with no <file.h> */
#include <sys/utime.h>
#include <stdlib.h>
#include <process.h>
extern int binary;			/* We need to know this for open() */
extern int fncact;			/* Need this for zchkspa()         */
#ifdef __IBMC__
extern FILE *popen(char *, char *);
extern int pclose(FILE *);
#else
#ifndef __EMX__
#define popen    _popen
#define pclose   _pclose
#include <share.h>
#define fopen(n, m)  _fsopen(n, m, SH_DENYWR)
#endif /* __EMX__ */
#endif /* __IBMC__ */
#else
#include <pwd.h>			/* Password file for shell name */
#endif /* OS2 */

#ifdef POSIX
#define UTIMEH
#endif /* POSIX */

#ifndef OS2
#ifdef SYSUTIMEH			/* <sys/utime.h> if requested,  */
#include <sys/utime.h>			/* for extra fields required by */
#else					/* 88Open spec. */
#ifdef UTIMEH				/* or <utime.h> if requested */
#include <utime.h>			/* (SVR4, POSIX) */
#define SYSUTIMEH			/* Use this for both cases. */
#endif /* UTIMEH */
#endif /* SYSUTIMEH */
#endif /* OS2 */

#ifdef POSIX
#define TIMESTAMP
#endif /* POSIX */

#ifdef BSD44				/* BSD 4.4 */
#ifndef TIMESTAMP
#define TIMESTAMP			/* Can do file dates */
#endif /* TIMESTAMP */
#include <sys/time.h>
#include <sys/timeb.h>

#else  /* Not BSD44 */

#ifdef BSD4				/* BSD 4.3 and below */
#define TIMESTAMP			/* Can do file dates */
#include <time.h>			/* Need this */
#include <sys/timeb.h>			/* Need this if really BSD */

#else  /* Not BSD 4.3 and below */

#ifdef SVORPOSIX			/* System V or POSIX */
#ifndef TIMESTAMP
#define TIMESTAMP
#endif /* TIMESTAMP */
#include <time.h>

/* void tzset(); (the "void" type upsets some compilers) */
#ifndef ultrix
#ifndef CONVEX9
/* ConvexOS 9.0, supposedly POSIX, has extern char *timezone(int,int) */
extern long timezone;
#endif /* CONVEX9 */
#endif /* ultrix */
#endif /* SVORPOSIX */
#endif /* BSD4 */
#endif /* BSD44 */

/* Is `y' a leap year? */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

#ifdef COMMENT /* not used */
/* Number of days in each month of the year. */
static char monlens[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif /* COMMENT */

#ifdef CIE
#include <stat.h>			/* File status */
#else
#include <sys/stat.h>

#ifdef OS2
#include <sys/types.h>
/* because standard stat has trouble with trailing /'s we have to wrap it */
int os2stat(char *, struct stat *);
#ifdef __IBMC__
#ifdef system
#undef system
#endif
#ifdef stat
#undef stat
#define __IBMC__STAT
#endif
#endif
#define stat(path, buf) os2stat(path, buf)
#endif /* OS2 */
#endif /* CIE */

/*
  Functions (n is one of the predefined file numbers from ckcker.h):

   zopeni(n,name)   -- Opens an existing file for input.
   zopeno(n,name,attr,fcb) -- Opens a new file for output.
   zclose(n)        -- Closes a file.
   zchin(n,&c)      -- Gets the next character from an input file.
   zsinl(n,&s,x)    -- Read a line from file n, max len x, into address s.
   zsout(n,s)       -- Write a null-terminated string to output file, buffered.
   zsoutl(n,s)      -- Like zsout, but appends a line terminator.
   zsoutx(n,s,x)    -- Write x characters to output file, unbuffered.
   zchout(n,c)      -- Add a character to an output file, unbuffered.
   zchki(name)      -- Check if named file exists and is readable, return size.
   zchko(name)      -- Check if named file can be created.
   zchkspa(name,n)  -- Check if n bytes available to create new file, name.
   znewn(name,s)    -- Make a new unique file name based on the given name.
   zdelet(name)     -- Delete the named file.
   zxpand(string)   -- Expands the given wildcard string into a list of files.
   znext(string)    -- Returns the next file from the list in "string".
   zxcmd(n,cmd)     -- Execute the command in a lower fork on file number n.
   zclosf()         -- Close input file associated with zxcmd()'s lower fork.
   zrtol(n1,n2)     -- Convert remote filename into local form.
   zltor(n1,n2)     -- Convert local filename into remote form.
   zchdir(dirnam)   -- Change working directory.
   zhome()          -- Return pointer to home directory name string.
   zkself()         -- Kill self, log out own job.
   zsattr(struct zattr *) -- Return attributes for file which is being sent.
   zstime(f, struct zattr *, x) - Set file creation date from attribute packet.
   zrename(old, new) -- Rename a file.
   zmkdir(path)       -- Create the directory path if possible  
*/

/* Kermit-specific includes */
/*
  Definitions here supersede those from system include files.
  ckcdeb.h is included above.
*/
#include "ckcker.h"			/* Kermit definitions */
#include "ckucmd.h"			/* For sys-dependent keyword tables */
#include "ckuver.h"			/* Version herald */

char *ckzsys = HERALD;

/*
  File access checking ...  There are two calls to access() in this module.
  If this program is installed setuid or setgid on a Berkeley-based UNIX
  system that does NOT incorporate the saved-original-effective-uid/gid
  feature, then, when we have swapped the effective and original uid/gid,
  access() fails because it uses what it thinks are the REAL ids, but we have
  swapped them.  This occurs on systems where ANYBSD is defined, NOSETREU
  is NOT defined, and SAVEDUID is NOT defined.  So, in theory, we should take
  care of this situation like so:

    ifdef ANYBSD
    ifndef NOSETREU
    ifndef SAVEDUID
    define SW_ACC_ID
    endif
    endif
    endif

  But we can't test such a general scheme everywhere, so let's only do this
  when we know we have to...
*/
#ifdef NEXT				/* NeXTSTEP 1.0-3.0 */
#define SW_ACC_ID
#endif /* NEXT */

/* Support for tilde-expansion in file and directory names */

#ifdef POSIX
#define NAMEENV "LOGNAME"
#endif /* POSIX */

#ifdef BSD4
#define NAMEENV "USER"
#endif /* BSD4 */

#ifdef ATTSV
#define NAMEENV "LOGNAME"
#endif /* ATTSV */

/* Berkeley Unix Version 4.x */
/* 4.1bsd support from Charles E Brooks, EDN-VAX */

#ifdef BSD4
#ifdef MAXNAMLEN
#define BSD42
#endif /* MAXNAMLEN */
#endif /* BSD4 */

/* Definitions of some system commands */

#ifdef OS2
char *DELCMD = "del ";			/* For file deletion */
char *PWDCMD = "chdir ";		/* For saying where I am */
char *TYPCMD = "type ";			/* For typing a file */
char *DIRCMD = "dir ";			/* For directory listing */
char *DIRCM2 = "dir ";			/* For directory listing, no args */
char *WHOCMD = "@echo  C-Kermit for OS/2 Server"; /* Who's there? */
char *SPACMD = "dir | find \"bytes free\""; /* For space on disk */
char *SPACM2 = "dir | find \"bytes free\""; /* For space on disk */

#else /* Not OS2, ergo UNIX (or Aegis) */

char *DELCMD = "rm -f ";		/* For file deletion */
char *PWDCMD = "pwd ";			/* For saying where I am */

#ifdef COMMENT
#ifdef HPUX10
char *DIRCMD = "/usr/bin/ls -l ";	/* For directory listing */
char *DIRCM2 = "/usr/bin/ls -l ";	/* For directory listing, no args */
#else
char *DIRCMD = "/bin/ls -l ";		/* For directory listing */
char *DIRCM2 = "/bin/ls -l ";		/* For directory listing, no args */
#endif /* HPUX10 */
#else
char *DIRCMD = "ls -l ";		/* For directory listing */
char *DIRCM2 = "ls -l ";		/* For directory listing, no args */
#endif /* COMMENT */

char *TYPCMD = "cat ";			/* For typing a file */

#ifdef FT18				/* Fortune For:Pro 1.8 */
#undef BSD4
#endif /* FT18 */

#ifdef BSD4
char *SPACMD = "pwd ; df .";		/* Space in current directory */
#else
#ifdef FT18
char *SPACMD = "pwd ; du ; df .";
#else
char *SPACMD = "df ";
#endif /* FT18 */
#endif /* BSD4 */

char *SPACM2 = "df ";			/* For space in specified directory */

#ifdef FT18
#define BSD4
#endif /* FT18 */

#ifdef BSD4
char *WHOCMD = "finger ";
#else
char *WHOCMD = "who ";
#endif /* BSD4 */

#endif /* OS2 */

#ifdef DTILDE				/* For tilde expansion */
_PROTOTYP( char * tilde_expand, (char *) );
#endif /* DTILDE */

/* More system-dependent includes, which depend on symbols defined */
/* in the Kermit-specific includes.  Oh what a tangled web we weave... */

#ifdef COHERENT				/* <sys/file.h> */
#define NOFILEH
#endif /* COHERENT */

#ifdef MINIX
#define NOFILEH
#endif /* MINIX */

#ifdef aegis
#define NOFILEH
#endif /* aegis */

#ifdef unos
#define NOFILEH
#endif /* unos */

#ifndef NOFILEH
#include <sys/file.h>
#endif /* NOFILEH */

#ifndef is68k				/* Whether to include <fcntl.h> */
#ifndef BSD41				/* All but a couple UNIXes have it. */
#ifndef FT18
#ifndef COHERENT
#include <fcntl.h>
#endif /* COHERENT */
#endif /* FT18  */
#endif /* BSD41 */
#endif /* not is68k */

#ifdef COHERENT
#ifdef _I386
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif /* _I386 */
#endif /* COHERENT */

/*
  Change argument to "(const char *)" if this causes trouble.
  Or... if it causes trouble, then maybe it was already declared 
  in a header file after all, so you can remove this prototype.
*/
#ifndef NDGPWNAM /* If not defined No Declare getpwnam... */
#ifndef _POSIX_SOURCE
#ifndef NEXT
#ifndef SVR4
/* POSIX <pwd.h> already gave prototypes for these. */
#ifdef IRIX40
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef IRIX51
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef M_UNIX
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef HPUX9
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef HPUX10
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef DCGPWNAM
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
_PROTOTYP( struct passwd * getpwnam, (char *) );
#endif /* DCGPWNAM */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* M_UNIX */
#endif /* IRIX51 */
#endif /* IRIX40 */
#ifndef SUNOS4
#ifndef HPUX9
#ifndef HPUX10
_PROTOTYP( struct passwd * getpwuid, (PWID_T) );
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* SUNOS4 */
_PROTOTYP( struct passwd * getpwent, (void) );
#endif /* SVR4 */
#endif /* NEXT */
#endif /* _POSIX_SOURCE */
#endif /* NDGPWNAM */

/* Define macros for getting file type */

#ifdef OXOS
/*
  Olivetti X/OS 2.3 has S_ISREG and S_ISDIR defined
  incorrectly, so we force their redefinition.
*/
#undef S_ISREG
#undef S_ISDIR
#endif /* OXOS */

#ifdef UTSV				/* Same deal for Amdahl UTSV */
#undef S_ISREG
#undef S_ISDIR
#endif /* UTSV */

#ifdef UNISYS52				/* And for UNISYS UTS V 5.2 */
#undef S_ISREG
#undef S_ISDIR
#endif /* UNISYS52 */

#ifdef ICLSVR3				/* And for old ICL versions */
#undef S_ISREG
#undef S_ISDIR
#endif /* ICLSVR3 */

#ifdef ISDIRBUG				/* Also allow this from command line */
#ifdef S_ISREG
#undef S_ISREG
#endif /* S_ISREG */
#ifdef S_ISDIR
#undef S_ISDIR
#endif /*  S_ISDIR */
#endif /* ISDIRBUG */

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* S_ISREG */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* S_ISDIR */

/* Define maximum length for a file name if not already defined */

#ifdef QNX
#ifdef _MAX_FNAME
#define MAXNAMLEN _MAX_FNAME
#else
#define MAXNAMLEN 48
#endif /* _MAX_FNAME */
#else
#ifndef MAXNAMLEN
#ifdef sun
#define MAXNAMLEN 255
#else
#ifdef FILENAME_MAX
#define MAXNAMLEN FILENAME_MAX
#else
#ifdef NAME_MAX
#define MAXNAMLEN NAME_MAX
#else
#ifdef _POSIX_NAME_MAX
#define MAXNAMLEN _POSIX_NAME_MAX
#else
#ifdef _D_NAME_MAX
#define MAXNAMLEN _D_NAME_MAX
#else
#ifdef DIRSIZ
#define MAXNAMLEN DIRSIZ
#else
#define MAXNAMLEN 14
#endif /* DIRSIZ */
#endif /* _D_NAME_MAX */
#endif /* _POSIX_NAME_MAX */
#endif /* NAME_MAX */
#endif /* FILENAME_MAX */
#endif /* sun */
#endif /* MAXNAMLEN */
#endif /* QNX */

/* Longest pathname ... */
/*
  Beware: MAXPATHLEN is one of UNIX's dirty little secrets.  Where is it
  defined?  Who knows...  <param.h>, <mod.h>, <unistd.h>, <limits.h>, ...
  There is not necessarily even a definition for it anywhere, or it might have
  another name.  If you get it wrong, bad things happen with getcwd() and/or
  getwd().  If you allocate a buffer that is too short, getwd() might write
  over memory and getcwd() will fail with ERANGE.  The definitions of these
  functions (e.g. in SVID or POSIX.1) do not tell you how to determine the
  maximum path length in order to allocate a buffer that is the right size.
*/
#ifdef BSD44
#include <sys/param.h>			/* For MAXPATHLEN */
#endif /* BSD44 */

#ifdef COHERENT
#include <sys/param.h>	/* for MAXPATHLEN, needed for -DDIRENT */
#endif /* COHERENT */

#ifdef MAXPATHLEN
#ifdef MAXPATH
#undef MAXPATH
#endif /* MAXPATH */
#define MAXPATH MAXPATHLEN
#else
#ifdef PATH_MAX
#define MAXPATH PATH_MAX
#else
#ifdef _POSIX_PATH_MAX
#define MAXPATH _POSIX_PATH_MAX
#else
#ifdef BSD42
#define MAXPATH 1024
#else
#ifdef SVR4  
#define MAXPATH 1024
#else
#define MAXPATH 255
#endif /* SVR4 */
#endif /* BSD42 */
#endif /* _POSIX_PATH_MAX */
#endif /* PATH_MAX */
#endif /* MAXPATHLEN */

/* Maximum number of filenames for wildcard expansion */

#ifdef PROVX1
#define MAXWLD 50
#else
#ifdef pdp11
#define MAXWLD 50
#else
#ifdef BIGBUFOK
#define MAXWLD 4096
#else
#define MAXWLD 1000
#endif /* BIGBUFOK */
#endif /* pdp11 */
#endif /* PROVX1 */

/* More internal function prototypes */
/*
 * The path structure is used to represent the name to match.
 * Each slash-separated segment of the name is kept in one
 * such structure, and they are linked together, to make
 * traversing the name easier.
 */
struct path {
    char npart[MAXNAMLEN+4];		/* name part of path segment */
    struct path *fwd;			/* forward ptr */
};
#ifndef NOPUSH
_PROTOTYP( int shxpand, (char *, char *[], int ) );
#endif /* NOPUSH */
_PROTOTYP( static int fgen, (char *, char *[], int ) );
_PROTOTYP( static VOID traverse, (struct path *, char *, char *) );
_PROTOTYP( static VOID addresult, (char *) );
_PROTOTYP( static int match, (char *, char *) );
_PROTOTYP( static char * whoami, (void) );
#ifdef aegis
_PROTOTYP( static char * xindex, (char *, char) );
#endif /* aegis */
_PROTOTYP( UID_T real_uid, (void) );
_PROTOTYP( struct path *splitpath, (char *p) );

/* Some systems define these symbols in include files, others don't... */

#ifndef R_OK
#define R_OK 4				/* For access */
#endif

#ifndef W_OK
#define W_OK 2
#endif

#ifndef O_RDONLY
#define O_RDONLY 000
#endif

/* Declarations */

int maxnam = MAXNAMLEN;			/* Available to the outside */
int maxpath = MAXPATH;
int ck_znewn = -1;

FILE *fp[ZNFILS] = { 			/* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#ifdef OS2
int ispipe[ZNFILS];			/* Flag for file is a pipe */
#endif /* OS2 */

/* Buffers and pointers used in buffered file input and output. */
#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer;
#else
extern char zinbuffer[], zoutbuffer[];
#endif /* DYNAMIC */
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt;
extern int wildxpand;

extern UID_T real_uid();

static long iflen = -1L;		/* Input file length */

static PID_T pid = 0;			/* pid of child fork */
static int fcount;			/* Number of files in wild group */
static char nambuf[MAXNAMLEN+4];	/* Buffer for a filename */
#ifndef NOFRILLS
static char zmbuf[200];			/* For mail, remote print strings */
#endif /* NOFRILLS */

/* static */				/* Not static, must be global now. */
char *mtchs[MAXWLD],			/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */

#ifdef OS2
#ifdef __32BIT__
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
typedef unsigned short WORD;
extern TID KbdHandlerThreadID ;
extern HMUX hmuxKeyboard ;
#undef COMMENT

/*--------------------------------------------------------------*\
 *  Get/Set All Extended Attributes support                     *
\*--------------------------------------------------------------*/
#define FreeMem(p)      DosFreeMem(p)

#define MAX_GEA            500L  /* Max size for a GEA List                 */
#define MAXEACOUNT          128  /* Max number of EA's supported           */
#define Ref_ASCIIZ            1  /* Reference type for DosEnumAttribute     */

/* definition of level specifiers. required for File Info */

#define GetInfoLevel1         1            /* Get info from SFT             */
#define GetInfoLevel2         2            /* Get size of FEAlist           */
#define GetInfoLevel3         3            /* Get FEAlist given the GEAlist */
#define GetInfoLevel4         4            /* Get whole FEAlist             */
#define GetInfoLevel5         5            /* Get FSDname                   */

#define SetInfoLevel1         1            /* Set info in SFT               */
#define SetInfoLevel2         2            /* Set FEAlist                   */

FEA2LIST *pFEAList =0 ;       /* ptr to buffer containing all EA information */
ULONG    os2attrs = FILE_NORMAL ;
extern unsigned int lf_opts ;

#endif /* __32BIT__ */
char os2filename[MAXPATH];
#endif /* OS2 */

/*  Z K S E L F  --  Kill Self: log out own job, if possible.  */

/* Note, should get current pid, but if your system doesn't have */
/* getppid(), then just kill(0,9)...  */

#ifndef SVR3
#ifndef POSIX
#ifndef OSFPC
/* Already declared in unistd.h for SVR3 and POSIX */
#ifdef CK_ANSIC
extern PID_T getppid(void);
#else
#ifndef PS2AIX10
extern PID_T getppid();
#endif /* PS2AIX10 */
#endif /* CK_ANSIC */
#endif /* OSFPC */
#endif /* POSIX */
#endif /* SVR3 */
int
zkself() {				/* For "bye", but no guarantee! */
#ifdef PROVX1
    return(kill(0,9));
#else
#ifdef V7
    return(kill(0,9));
#else
#ifdef TOWER1
    return(kill(0,9));
#else
#ifdef FT18
    return(kill(0,9));
#else
#ifdef aegis
    return(kill(0,9));
#else
#ifdef COHERENT
    return(kill((PID_T)getpid(),1));
#else
#ifdef OS2
    exit(3);
#else
#ifdef PID_T
    exit(kill((PID_T)getppid(),1));
#else
    exit(kill(getppid(),1));
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
}

/*  Z O P E N I  --  Open an existing file for input. */

int
zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","", fp[n]);
    if (chkfn(n) != 0) return(0);
    zincnt = 0;				/* Reset input buffer */
    if (n == ZSYSFN) {			/* Input from a system function? */
/*** Note, this function should not be called with ZSYSFN ***/
/*** Always call zxcmd() directly, and give it the real file number ***/
/*** you want to use.  ***/
        debug(F110,"zopeni called with ZSYSFN, failing!",name,0);
	*nambuf = '\0';			/* No filename. */
	return(0);			/* fail. */
#ifdef COMMENT
	return(zxcmd(n,name));		/* Try to fork the command */
#endif
    }
    if (n == ZSTDIO) {			/* Standard input? */
	if (isatty(0)) {
	    fprintf(stderr,"Terminal input not allowed");
	    debug(F110,"zopeni: attempts input from unredirected stdin","",0);
	    return(0);
	}
	fp[ZIFILE] = stdin;
#ifdef OS2
	setmode(fileno(stdin),O_BINARY);
#endif /* OS2 */
	return(1);
    }
#ifdef OS2
    if (n == ZIFILE || n == ZRFILE) {
	strncpy( os2filename, name, MAXPATH ) ;
	fp[n] = fopen(name,"rb");		/* Binary mode */
#ifdef CK_LABELED
	if ( binary == XYFT_L ) {
     os2getattr(name) ;
	  os2geteas(name) ;
     }
#endif /* CK_LABELED */
    } else
#endif /* OS2 */
      fp[n] = fopen(name,"r");		/* Real file, open it. */
    debug(F111," zopeni", name, fp[n]);
    if (fp[n] == NULL) perror("zopeni");
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z O P E N O  --  Open a new file for output.  */

int
zopeno(n,name,zz,fcb)
/* zopeno */  int n; char *name; struct zattr *zz; struct filinfo *fcb; {

    char p[8];				/* (===OS2 change===) */

/* As of Version 5A, the attribute structure and the file information */
/* structure are included in the arglist. */

#ifdef DEBUG
    debug(F111,"zopeno",name,n);
    if (fcb) {
	debug(F101,"zopeno fcb disp","",fcb->dsp);
	debug(F101,"zopeno fcb type","",fcb->typ);
	debug(F101,"zopeno fcb char","",fcb->cs);
    } else {
	debug(F100,"zopeno fcb is NULL","",0);
    }
    if (n != ZDFILE)
      debug(F111," zopeno",name,n);
#endif /* DEBUG */

    if (chkfn(n) != 0)			/* Already open? */
      return(0);			/* Nothing to do. */

    if ((n == ZCTERM) || (n == ZSTDIO)) {   /* Terminal or standard output */
	fp[ZOFILE] = stdout;
#ifdef DEBUG
	if (n != ZDFILE)
	  debug(F101," fp[]=stdout", "", fp[n]);
#endif /* DEBUG */
	zoutcnt = 0;
	zoutptr = zoutbuffer;
	return(1);
    }

/* A real file.  Open it in desired mode (create or append). */

    strcpy(p,"w");			/* Assume write/create mode */
    if (fcb) {				/* If called with an FCB... */
	if (fcb->dsp == XYFZ_A)		/* Does it say Append? */
	  strcpy(p,"a");		/* Yes. */
    }
#ifdef OS2
    if (n == ZOFILE || n == ZSFILE) {	/* OS/2 binary mode */
	strncpy( os2filename, name, MAXPATH ) ;
	strcat(p,"b");
    }
#endif /* OS2 */
    fp[n] = fopen(name,p);		/* Try to open the file */

    if (fp[n] == NULL) {		/* Failed */
	debug(F101,"zopeno failed errno","",errno);
#ifdef COMMENT				/* Let upper levels print message. */
        perror("Can't open output file");
#endif /* COMMENT */
    } else {				/* Succeeded */
        if (n == ZDFILE)		/* If it's the debug log */
	  setbuf(fp[n],NULL);		/* make it unbuffered */
	else
	  debug(F100, "zopeno ok", "", 0);
    }
    zoutcnt = 0;			/* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */

int
zclose(n) int n; {
    int x, x2;
    if (chkfn(n) < 1) return(0);	/* Check range of n */

    if ((n == ZOFILE) && (zoutcnt > 0))	/* (PWP) output leftovers */
      x2 = zoutdump();
    else
      x2 = 0;

    x = 0;				/* Initialize return code */
    if (fp[ZSYSFN]) {			/* If file is really pipe */
    	x = zclosf(n);			/* do it specially */
    } else {
    	if ((fp[n] != stdout) && (fp[n] != stdin)) x = fclose(fp[n]);
	fp[n] = NULL;
#ifdef OS2
#ifdef CK_LABELED
	if (binary == XYFT_L) {
	    if ( n == ZOFILE ) {
	      if ( lf_opts && LBL_EXT )
            os2seteas(os2filename) ;      
         os2setattr(os2filename) ;
         }
	    else if ( n == ZIFILE && pFEAList ) {
		FreeMem( pFEAList ) ;
		pFEAList = 0 ;
	    }
	}
#endif /* CK_LABELED */
#endif /* OS2 */
    }
    iflen = -1L;			/* Invalidate file length */
    if (x == EOF)			/* if we got a close error */
      return(-1);
    else if (x2 < 0)		/* or an error flushing the last buffer */
      return(-1);		/* then return an error */
    else
      return(1);
}

/*  Z C H I N  --  Get a character from the input file.  */

/*  Returns -1 if EOF, 0 otherwise with character returned in argument  */

int
zchin(n,c) int n; int *c; {
    int a, x;

    /* (PWP) Just in case this gets called when it shouldn't. */
    if (n == ZIFILE) {
	x = zminchar();
	*c = x;
	return(x);
    }
    /* if (chkfn(n) < 1) return(-1); */
    a = getc(fp[n]);
    if (a == EOF) return(-1);
#ifdef OS2
    if (!binary && a == 0x1A)		/* Ctrl-Z marks EOF for text mode*/
      return(-1);
#endif
    *c = (CHAR) a & 0377;
    return(0);
}

/*  Z S I N L  --  Read a line from a file  */

/*
  Writes the line into the address provided by the caller.
  n is the Kermit "channel number".
  Writing terminates when newline is encountered, newline is not copied.
  Writing also terminates upon EOF or if length x is exhausted.
  Returns 0 on success, -1 on EOF or error.
*/
int
zsinl(n,s,x) int n, x; char *s; {
    int a, z = 0;			/* z is return code. */

    if (chkfn(n) < 1) {			/* Make sure file is open */
	return(-1);
    }
    a = -1;				/* Current character, none yet. */
    while (x--) {			/* Up to given length */
#ifndef NLCHAR
	int old;
	old = a;			/* Previous character */
#endif
	if (zchin(n,&a) < 0) {		/* Read a character from the file */
	    debug(F101,"zsinl","",a);
	    z = -1;			/* EOF or other error */
	    break;
	}
#ifdef NLCHAR
	if (a == (char) NLCHAR) break;	/* Single-character line terminator */
#else					/* CRLF line terminator */
	if (a == '\015') continue;	/* CR, get next character */
	if (old == '\015') {		/* Previous character was CR */
	    if (a == '\012') break;	/* This one is LF, so we have a line */
	    else *s++ = '\015';		/* Not LF, deposit CR */
	}
#ifdef OS2
/*
  I'm not sure I understand this one, so let's keep it only for OS/2 for now.
*/
	if (a == '\012') break;		/* Break on single LF too */
#endif /* OS2 */
#endif /* NLCHAR */
	*s = a;				/* Deposit character */
	s++;
    }
    *s = '\0';				/* Terminate the string */
    return(z);
}

/*
 * (PWP) (re)fill the buffered input buffer with data.  All file input
 * should go through this routine, usually by calling the zminchar()
 * macro (in ckcker.h).
 */

/*
 * Suggestion: if fread() returns 0, call ferror to find out what the
 * problem was.  If it was not EOF, then return -2 instead of -1.
 * Upper layers (getpkt function in ckcfns.c) should set cxseen flag
 * if it gets -2 return from zminchar macro.
 */
int
zinfill() {
    int x;

    errno = 0;

#ifdef OS2
#ifdef CK_LABELED
    debug(F101,"zinfill: binary","",binary);
    debug(F101,"zinfill: pFEAList","",pFEAList);
    if ( binary == XYFT_L && pFEAList ) {
	zinptr = zinbuffer ;
	do_label_send(os2filename) ;
	zincnt += fread(zinptr, sizeof (char), INBUFSIZE - zincnt, fp[ZIFILE]);
    } else
#endif /* CK_LABELED */
#endif /* OS2 */
    zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
#ifdef COMMENT
    debug(F101,"zinfill fp","",fp[ZIFILE]);
    debug(F101,"zinfill zincnt","",zincnt);
#endif
    if (zincnt == 0) {
#ifndef UTEK
#ifdef ferror
	x = ferror(fp[ZIFILE]);
	debug(F101,"zinfill errno","",errno);
	debug(F101,"zinfill ferror","",x);
	if (x) return(-2);
#endif /* ferror */
#else
 	x = feof(fp[ZIFILE]);
 	debug(F101,"zinfill errno","",errno);
 	debug(F101,"zinfill feof","",x);
 	if (!x && ferror(fp[ZIFILE])) return(-2);
#endif /* UTEK */
	return(-1);
    }
    zinptr = zinbuffer;	   /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;			/* one less char in buffer */
    return((int)(*zinptr++) & 0377); /* because we return the first */
}

/*  Z S O U T  --  Write a string out to the given file, buffered.  */

int
zsout(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);	/* Keep this, prevents memory faults */
    if (!s) return(0);			/* Null pointer, do nothing, succeed */
    if (!*s) return(0);			/* empty string, ditto */
#ifndef OS2
    if (n == ZSFILE)
      return(write(fileno(fp[n]),s,(int)strlen(s)));
    else
#endif /* OS2 */
      return(fputs(s,fp[n]) == EOF ? -1 : 0);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

int
zsoutl(n,s) int n; char *s; {
    if (zsout(n,s) < 0) return(-1);
#ifndef OS2
    if (n == ZSFILE)			/* But session log is unbuffered */
      return(write(fileno(fp[n]),"\n",1)); /* except in OS/2... */
    else
#endif /* OS2 */
      if (fputs("\n",fp[n]) == EOF) return(-1); /* (===OS2 ? \r\n) */
    return(0);
}

/*  Z S O U T X  --  Write x characters to file, unbuffered.  */

int
zsoutx(n,s,x) int n, x; char *s; {
#ifdef COMMENT
    if (chkfn(n) < 1) return(-1);
    return(write(fp[n]->_file,s,x));
#endif
    return(write(fileno(fp[n]),s,x) == x ? x : -1);
}


/*  Z C H O U T  --  Add a character to the given file.  */

/*  Should return 0 or greater on success, -1 on failure (e.g. disk full)  */

int
#ifdef CK_ANSIC
zchout(register int n, char c)
#else
zchout(n,c) register int n; char c;
#endif /* CK_ANSIC */
/* zchout() */ {
    /* if (chkfn(n) < 1) return(-1); */
#ifndef OS2
    if (n == ZSFILE) {			/* Use unbuffered for session log */
    	return(write(fileno(fp[n]),&c,1) == 1 ? 0 : -1);
    } else {				/* Buffered for everything else */
#endif /* OS2 */
	if (putc(c,fp[n]) == EOF)	/* If true, maybe there was an error */
	  return(ferror(fp[n])?-1:0);	/* Check to make sure */
	else				/* Otherwise... */
	  return(0);			/* There was no error. */
#ifndef OS2
    }
#endif /* OS2 */
}

/* (PWP) buffered character output routine to speed up file IO */

int
zoutdump() {
    int x;
    zoutptr = zoutbuffer;		/* Reset buffer pointer in all cases */
    debug(F101,"zoutdump chars","",zoutcnt);
    if (zoutcnt == 0) {			/* Nothing to output */
	return(0);
    } else if (zoutcnt < 0) {		/* Unexpected negative argument */
	zoutcnt = 0;			/* Reset output buffer count */
	return(-1);			/* and fail. */
    }

#ifdef CK_LABELED
    if (binary == XYFT_L ) {
	x = do_label_recv() ;
	if ( x > 0 ) { /* more room in the buffer */
	    debug(F101,"zoutdump do_label_recv unfilled buffer","",zoutcnt);
	    return 0 ; 
	} else if ( x < 0 ) {
	    debug(F101,"zoutdump do_label_recv error","",x);
	    return -1 ; 
	}
    }
    fflush(fp[ZOFILE]);
    if ((x = write(fileno(fp[ZOFILE]),zoutbuffer,zoutcnt)) == zoutcnt) {
	debug(F101,"zoutdump write ok","",zoutcnt);
	zoutcnt = 0;			/* Reset output buffer count */
	return(0);			/* write() worked OK */
    } else {
	debug(F101,"zoutdump write error","",errno);
	debug(F101,"zoutdump write returns","",x);
	zoutcnt = 0;			/* Reset output buffer count */
	return(-1);			/* write() failed */
#else
/* Frank Prindle suggested that replacing this fwrite() by an fflush() */
/* followed by a write() would improve the efficiency, especially when */
/* writing to stdout.  Subsequent tests showed a 5-fold improvement!   */
/* if (x = fwrite(zoutbuffer, 1, zoutcnt, fp[ZOFILE])) {              */

    fflush(fp[ZOFILE]);
    if ((x = write(fileno(fp[ZOFILE]),zoutbuffer,zoutcnt)) == zoutcnt) {
	debug(F101,"zoutdump write ok","",zoutcnt);
	zoutcnt = 0;			/* Reset output buffer count */
	return(0);			/* write() worked OK */
    } else {
	debug(F101,"zoutdump write error","",errno);
	debug(F101,"zoutdump write returns","",x);
	zoutcnt = 0;			/* Reset output buffer count */
	return(-1);			/* write() failed */
#endif /* CK_LABELED */
    }
}

/*  C H K F N  --  Internal function to verify file number is ok  */

/*
 Returns:
  -1: File number n is out of range
   0: n is in range, but file is not open
   1: n in range and file is open
*/
int
chkfn(n) int n; {
#ifdef COMMENT				/* Save some stack space */
    switch (n) {
	case ZCTERM:
	case ZSTDIO:
	case ZIFILE:
	case ZOFILE:
	case ZDFILE:
	case ZTFILE:
	case ZPFILE:
	case ZSFILE:
	case ZSYSFN:
        case ZRFILE:
        case ZWFILE:
#ifdef OS2
        case ZPRINT:
#endif /* OS2 */
	    break;
	default:
	    debug(F101,"chkfn: file number out of range","",n);
	    fprintf(stderr,"?File number out of range - %d\n",n);
	    return(-1);
    }
    return( (fp[n] == NULL) ? 0 : 1 );
#else
    if (n < 0 || n >= ZNFILS)
      return(-1);
    else return( (fp[n] == NULL) ? 0 : 1 );
#endif /* COMMENT */
}

/*  Z C H K I  --  Check if input file exists and is readable  */

/*
  Returns:
   >= 0 if the file can be read (returns the size).
     -1 if file doesn't exist or can't be accessed,
     -2 if file exists but is not readable (e.g. a directory file).
     -3 if file exists but protected against read access.
*/
/*
 For Berkeley Unix, a file must be of type "regular" to be readable.
 Directory files, special files, and symbolic links are not readable.
*/
long
zchki(name) char *name; {
    struct stat buf;
    int x;

#ifdef UNIX
    x = strlen(name);
    if (x == 9 && !strcmp(name,"/dev/null"))
      return(0);
#endif /* UNIX */

    x = stat(name,&buf);
    if (x < 0) {
	debug(F111,"zchki stat fails",name,errno);
	return(-1);
    }
    if (!S_ISREG (buf.st_mode)) {	/* Must be regular file */
	debug(F111,"zchki skipping:",name,x);
	return(-2);
    }
    debug(F111,"zchki stat ok:",name,x);

#ifdef SW_ACC_ID
    debug(F100,"zchki swapping ids for access()","",0);
    priv_on();
#endif /* SW_ACC_ID */
    x = access(name,R_OK);
#ifdef SW_ACC_ID
    priv_off();
    debug(F100,"zchki swapped ids restored","",0);
#endif /* SW_ACC_ID */
    if (x < 0) { 	/* Is the file accessible? */
	debug(F111," access failed:",name,x); /* No */
    	return(-3);
    } else {
	iflen = buf.st_size;		      /* Yes, remember size */
	strncpy(nambuf,name,MAXNAMLEN);	      /* and name globally. */
	debug(F111," access ok:",name,(int) iflen);
	return( (iflen > -1L) ? iflen : 0L );
    }
}

/*  Z C H K O  --  Check if output file can be created  */

/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
int
zchko(name) char *name; {
    int i, x;
    char *s;

    if (!name) return(-1);		/* Watch out for null pointer. */
    x = (int)strlen(name);		/* Get length of filename */
    debug(F101," length","",x);

#ifdef UNIX
/*
  Writing to null device is OK.
*/
    if (x == 9 && !strcmp(name,"/dev/null"))
      return(0);
#endif /* UNIX */

    if (isdir(name))			/* Directories are not writeable */
      return(-1);

    s = malloc(x+3);			/* Must copy because we can't */
    if (!s) {				/* write into our argument. */
	fprintf(stderr,"Malloc error 46\n");
	return(-1);
    }
    strcpy(s,name);

    for (i = x; i > 0; i--)		/* Strip filename from right. */
      if (ISDIRSEP(s[i-1])) break;
    debug(F101," i","",i);

#ifdef COMMENT
/* X/OPEN XPG3-compliant systems fail if argument ends with "/"...  */
    if (i == 0)				/* If no path, use current directory */
      strcpy(s,"./");
    else				/* Otherwise, use given one. */
      s[i] = '\0';
#else
/* So now we use "path/." if path given, or "." if no path given. */
    s[i++] = '.';			/* Append "." to path. */
    s[i] = '\0';
#endif /* COMMENT */

#ifdef SW_ACC_ID
    debug(F100,"zchko swapping ids for access()","",0);
    priv_on();
#endif /* SW_ACC_ID */
#ifdef OS2				/* No unwritable directories in OS/2 */
    x = 0;
#else
    x = access(s,W_OK);			/* Check access of path. */
#endif /* OS2 */
#ifdef SW_ACC_ID
    priv_off();
    debug(F100,"zchko swapped ids restored","",0);
#endif /* SW_ACC_ID */
    if (x < 0)
      debug(F111,"zchko access failed:",s,errno);
    else
      debug(F111,"zchko access ok:",s,x);
    free(s);				/* Free temporary storage */
    return((x < 0) ? -1 : 0);		/* and return. */
}

/*  Z D E L E T  --  Delete the named file.  */

int
zdelet(name) char *name; {
    return(unlink(name));
}


/*  Z R T O L  --  Convert remote filename into local form  */

VOID
zrtol(name,name2) char *name, *name2; {
    char *p; int flag = 0, n = 0;
    if (!name || !name2) return;
    debug(F101,"zrtol original name","",name);
    p = name2;
    for ( ; *name != '\0' && n < maxnam; name++) {
	if (*name > ' ') flag = 1;	/* Strip leading blanks and controls */
	if (flag == 0 && *name < '!') continue;
    	*p++ = isupper(*name) ? tolower(*name) : *name;
	n++;
    }
    *p-- = '\0';			/* Terminate */
    while (*p < '!' && p > name2)	/* Strip trailing blanks & controls */
      *p-- = '\0';
    if (*name2 == '\0') strcpy(name2,"NONAME");
#ifdef OS2
    if (!IsFileNameValid(name2))
      ChangeNameForFAT(name2);
#endif /* OS2 */
    debug(F110,"zrtol new name",name2,0);
}


/*  Z S T R I P  --  Strip device & directory name from file specification */

/*  Strip pathname from filename "name", return pointer to result in name2 */

#ifdef pdp11
static char work[100];		/* buffer for use by zstrip and zltor */
#else
static char work[257];
#endif /* pdp11 */

VOID
zstrip(name,name2) char *name, **name2; {
    char *cp, *pp;
    debug(F110,"zstrip before",name,0);
    pp = work;
#ifdef DTILDE
    /* Strip leading tilde */
    if (*name == '~') name++;
    debug(F110,"zstrip after tilde-stripping",name,0);
#else
#ifdef OS2
    /* Strip disk letter and colon */
    if (isalpha(*name) && (*(name+1) == ':')) name += 2;
    debug(F110,"zstrip after disk-stripping",name,0);
#endif /* OS2 */
#endif /* DTILDE */
    for (cp = name; *cp; cp++) {
    	if (ISDIRSEP(*cp))
	  pp = work;
	else
	  *pp++ = *cp;
    }
    *pp = '\0';				/* Terminate the string */
    *name2 = work;
    debug(F110,"zstrip after",*name2,0);
}

/*  Z L T O R  --  Local TO Remote */

VOID
zltor(name,name2) char *name, *name2; {
    char *cp, *pp;
    int dc = 0;
#ifdef aegis
    char *namechars;
    int tilde = 0, bslash = 0;

    if ((namechars = getenv("NAMECHARS")) != NULL) {
    	if (xindex(namechars, '~' ) != NULL) tilde  = '~';
    	if (xindex(namechars, '\\') != NULL) bslash = '\\';
    } else {
        tilde = '~';
        bslash = '\\';
    }
#endif /* aegis */

    debug(F110,"zltor",name,0);
    pp = work;
#ifdef aegis
    cp = name;
    if (tilde && *cp == tilde)
      ++cp;
    for (; *cp != '\0'; cp++) {
    	if (*cp == '/' || *cp == bslash) { /* Strip path name */
#else
#ifdef OS2
    /* Strip disk name */
    if (isalpha(*name) && (*(name+1) == ':')) name += 2;
#endif /* OS2 */
    for (cp = name; *cp != '\0'; cp++) { /* Strip path name */
    	if (ISDIRSEP(*cp)) {
#endif /* aegis */
	    dc = 0;
	    pp = work;
	}
	else if (islower(*cp)) *pp++ = toupper(*cp); /* Uppercase letters */
	else if (*cp == '~') *pp++ = 'X';	/* Change tilde to 'X' */
	else if (*cp == '#') *pp++ = 'X';	/* Change number sign to 'X' */
	else if ((*cp == '.') && (++dc > 1)) *pp++ = 'X'; /* & extra dots */
	else *pp++ = *cp;
    }
    *pp = '\0';				/* Tie it off. */
    cp = name2;				/* If nothing before dot, */
    if (*work == '.') *cp++ = 'X';	/* insert 'X' */
    strcpy(cp,work);
    debug(F110," name2",name2,0);
}


/*  Z C H D I R  --  Change directory  */
/*
  Call with:
    dirnam = pointer to name of directory to change to,
      which may be "" or NULL to indicate user's home directory.
  Returns:
    0 on failure
    1 on success
*/
int
zchdir(dirnam) char *dirnam; {
    char *hd, *sp, *p;

    debug(F110,"zchdir",dirnam,0);
    if (dirnam == NULL || dirnam == "" || *dirnam == '\0') /* If arg is null */
      dirnam = zhome();			/* use user's home directory. */
    sp = dirnam;
    debug(F110,"zchdir 2",dirnam,0);

#ifdef DTILDE
    hd = tilde_expand(dirnam);		/* Attempt to expand tilde */
    if (*hd == '\0') hd = dirnam;	/* in directory name. */
#else
    hd = dirnam;
#endif /* DTILDE */
    debug(F110,"zchdir 3",hd,0);
#ifdef pdp11
    /* Just to save some space */
    return((chdir(hd) == 0) ? 1 : 0);
#else
#ifdef OS2
    if (isalpha(hd[0]) && hd[1] == ':') {
        if (zchdsk(hd[0]))
	  return(0);
	if (hd[2] == 0)
	  return(1);			/* Handle drive-only case */
    }
#endif /* OS2 */
    if (chdir(hd) == 0) return(1);	/* Try to cd */ /* (===OS2===) */
    p = sp;				/* Failed, lowercase it. */
    while (*p) {
	if (isupper(*p)) *p = tolower(*p);
	p++;
    }
    debug(F110,"zchdir 4",hd,0);
#ifdef DTILDE
    hd = tilde_expand(sp);		/* Try again to expand tilde */
    if (*hd == '\0') hd = sp;
#else
    hd = sp;				/* Point to result */
#endif /* DTILDE */
    debug(F110,"zchdir 5",hd,0);
    return((chdir(hd) == 0) ? 1 : 0);
#endif /* pdp11 */
}

/*  Z H O M E  --  Return pointer to user's home directory  */

char *
zhome() {
    char *home = getenv("HOME");
#ifdef OS2
    extern char startupdir[];
    return(home ? home : startupdir);
#else
    return(home ? home : ".");
#endif
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

#ifdef pdp11
#define CWDBL 80			/* Save every byte we can... */
#else
#define CWDBL MAXPATH
#endif /* pdp11 */
static char cwdbuf[CWDBL+1];

char *
zgtdir() {
    char *buf;

#ifdef BSD44
    extern char *getcwd();
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    return(getcwd(buf,CWDBL));
#else
#ifdef SVORPOSIX
    extern char *getcwd();
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    return(getcwd(buf,CWDBL));
#else
#ifdef COHERENT
#ifdef _I386
    extern char *getcwd();
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    return(getcwd(buf,CWDBL));
#else
    extern char *getwd();
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    return(getwd(buf));
#endif /* _I386 */
#else
#ifdef OS2
#ifndef __IBMC__ /* which has a macro for this */
    extern char *getcwd();
#endif /* __IBMC__ */
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    return(getcwd(buf,CWDBL));
#else
#ifdef BSD4
    extern char *getwd();
    buf = cwdbuf;
    debug(F100,"zgtdir getwd()","",0);
    return(getwd(buf));
#else
    return("directory unknown");
#endif /* BSD4 */
#endif /* OS2 */
#endif /* COHERENT */
#endif /* SYSVORPOSIX */
#endif /* BSD44 */
}

/*  Z X C M D -- Run a system command so its output can be read like a file */

int
zxcmd(filnum,comand) int filnum; char *comand; {
#ifdef OS2
    if (chkfn(filnum) < 0) return(-1);	/* Need a valid Kermit file number. */
    if (filnum == ZSTDIO || filnum == ZCTERM) /* But not one of these. */
      return(0);
    if (filnum == ZIFILE || filnum == ZRFILE) { /* Input from a command */
	if (priv_chk() || ((fp[filnum] = popen(comand,"rb")) == NULL))
	  return(0);
    } else { /* Output to a command */
	if (priv_chk() || ((fp[filnum] = popen(comand,"w")) == NULL))
	  return(0);
    }
    ispipe[filnum] = 1;
    return(1);
#else /* Not OS2 */
    int pipes[2];
    int out;

    if (chkfn(filnum) < 0) return(-1);	/* Need a valid Kermit file number. */
    if (filnum == ZSTDIO || filnum == ZCTERM) /* But not one of these. */
      return(0);

    out = (filnum == ZIFILE || filnum == ZRFILE) ? 0 : 1 ;

/* Output to a command */

    if (out) {				/* Need popen() to do this. */
#ifdef NOPOPEN
	return(0);			/* no popen(), fail. */
#else
/* Use popen() to run the command. */

#ifdef _POSIX_SOURCE
/* Strictly speaking, popen() is not available in POSIX.1 */
#define DCLPOPEN
#endif /* _POSIX_SOURCE */

#ifdef COHERENT
#define DCLPOPEN
FILE * fdopen();
#endif /* COHERENT */

#ifdef DCLPOPEN
/* popen() needs declaring because it's not declared in <stdio.h> */
	FILE *popen();
#endif /* DCLPOPEN */

	if (priv_chk() || ((fp[filnum] = popen(comand,"w")) == NULL))
	  return(0);
	else return(1);
#endif /* NOPOPEN */
    }

/* Input from a command */

    if (pipe(pipes) != 0) {
	debug(F100,"zxcmd pipe failure","",0);
	return(0);			/* can't make pipe, fail */
    }

/* Create a fork in which to run the named process */

    if ((
#ifdef aegis
	 pid = vfork()			/* child */
#else
	 pid = fork()			/* child */
#endif /* aegis */
	 ) == 0) {

/* We're in the fork. */

	char *shpath, *shname, *shptr;	/* Find user's preferred shell */
#ifndef aegis
	struct passwd *p;
	char *defshell;
#ifdef HPUX10				/* Default shell */
	defshell = "/usr/bin/sh";
#else
	defshell = "/bin/sh";
#endif /* HPUX10 */
#endif /* aegis */
	if (priv_can()) exit(1);	/* Turn off any privileges! */
	debug(F101,"zxcmd pid","",pid);
	close(pipes[0]);		/* close input side of pipe */
	close(0);			/* close stdin */
	if (open("/dev/null",0) < 0) return(0); /* replace input by null */
#ifndef OXOS
#ifndef SVORPOSIX
	dup2(pipes[1],1);		/* BSD: replace stdout & stderr */
	dup2(pipes[1],2);		/* by the pipe */
#else
	close(1);			/* AT&T: close stdout */
	if ( dup(pipes[1]) != 1 )	/* Send stdout to the pipe */
	  return(0);
	close(2);			/* Send stderr to the pipe */
	if ( dup(pipes[1]) != 2 )
	  return(0);
#endif /* SVORPOSIX */
#else /* OXOS */
	dup2(pipes[1],1);
	dup2(pipes[1],2);
#endif /* OXOS */
	close(pipes[1]);		/* Don't need this any more. */

#ifdef aegis
	if ((shpath = getenv("SERVERSHELL")) == NULL)
	  shpath = "/bin/sh";
#else
	shpath = getenv("SHELL");	/* What shell? */
	if (shpath == NULL) {
	    p = getpwuid( real_uid() );	/* Get login data */
	    if (p == (struct passwd *)NULL || !*(p->pw_shell))
	      shpath = defshell;
	    else shpath = p->pw_shell;
        }
#endif /* aegis */
	shptr = shname = shpath;
	while (*shptr != '\0')
	  if (*shptr++ == '/')
	    shname = shptr;
	debug(F100,"zxcmd...","",0);
	debug(F110,shpath,shname,0);

	execl(shpath,shname,"-c",comand,(char *)NULL); /* Execute the cmd */
	exit(0);			/* just punt if it failed. */
    } else if (pid == (PID_T) -1) {
	debug(F100,"zxcmd fork failure","",0);
	return(0);
    }
    debug(F101,"zxcmd pid","",pid);
    if (out) {
	close(pipes[0]);		/* Don't need the input side */
	fp[filnum] = fdopen(pipes[1],"w"); /* Open a stream for output. */
	fp[ZSYSFN] = fp[filnum];	/* Remember. */
	zoutcnt = 0;			/* (PWP) reset input buffer */
	zoutptr = zoutbuffer;
    } else {
	close(pipes[1]);		/* Don't need the output side */
	fp[filnum] = fdopen(pipes[0],"r"); /* Open a stream for input. */
	fp[ZSYSFN] = fp[filnum];	/* Remember. */
	zincnt = 0;			/* (PWP) reset input buffer */
	zinptr = zinbuffer;
    }
    return(1);
#endif /* OS2 */
} /* zxcmd */

/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */

int
zclosf(filnum) int filnum; {
    int wstat;
    debug(F101,"zclosf filnum","",filnum);
#ifndef NOPOPEN
#ifdef OS2
    if (ispipe[filnum]) {
	int x;
	x = pclose(fp[filnum]);
	fp[filnum] = NULL;
	ispipe[filnum] = 0;
#else
    if (filnum == ZWFILE) {
	int x;
	x = pclose(fp[filnum]);
	fp[filnum] = fp[ZSYSFN] = NULL;
#endif /* OS2 */
	return((x < 0) ? 0 : 1);
    }
#endif /* NOPOPEN */
    debug(F101,"zclosf fp[filnum]","", fp[filnum]);
    debug(F101,"zclosf fp[ZSYSFN]","", fp[ZSYSFN]);
#ifdef OS2
    fclose(fp[filnum]);
    fp[filnum] = NULL;
#else
    if (pid != (PID_T) 0) {
	debug(F101,"zclosf killing pid","",pid);
	kill(pid,9);
        while ((wstat = wait((WAIT_T *)0)) != pid && wstat != -1) ;
        pid = 0;
    }
    fclose(fp[filnum]);
    fp[filnum] = fp[ZSYSFN] = NULL;
#endif /* OS2 */
    return(1);
}

/*  Z X P A N D  --  Expand a wildcard string into an array of strings  */
/*
  Returns the number of files that match fn1, with data structures set up
  so that first file (if any) will be returned by the next znext() call.
  Depends on external variable wildxpand: 0 means we expand wildcards
  internally, nonzero means we call the shell to do it.
*/

int
zxpand(fn) char *fn; {
    char *p;

#ifdef DTILDE				/* Built with tilde-expansion? */
    char *tnam;
#endif /* DTILDE */
    debug(F111,"zxpand entry",fn,wildxpand);
#ifdef DTILDE				/* Built with tilde-expansion? */
    if (*fn == '~') {			/* Starts with tilde? */
	tnam = tilde_expand(fn);	/* Try to expand it. */
	if (tnam) fn = tnam;
    }
    debug(F110,"zxpand after tilde_x",fn,0);
#endif /* DTILDE */
#ifndef OS2
#ifndef NOPUSH
    if (wildxpand)			/* Who is expanding wildcards? */
      fcount = shxpand(fn,mtchs,MAXWLD); /* Shell */
    else
#endif /* NOPUSH */
#endif /* OS2 */
      fcount = fgen(fn,mtchs,MAXWLD);	/* Kermit */
    if (fcount > 0) {
	mtchptr = mtchs;		/* Save pointer for next. */
    }
    if (fcount > 0) {
	debug(F111,"zxpand ok",mtchs[0],fcount);
	return(fcount);
    }
    debug(F111,"zxpand fgen1",fn,fcount); /* Didn't get one, or got too many */
    p = malloc((int)strlen(fn) + 10);	/* Make space */
    if (!p) return(0);
    zrtol(fn,p);			/* Try again, maybe lowercase */
#ifndef OS2
#ifndef NOPUSH
    if (wildxpand)
      fcount = shxpand(p,mtchs,MAXWLD); /* Shell */
    else
#endif /* NOPUSH */
#endif /* OS2 */
      fcount = fgen(p,mtchs,MAXWLD);	/* Kermit */
    if (fcount > 0) {			/* Got one? */
	mtchptr = mtchs;		/* Save pointer for next. */
	debug(F111,"zxpand fgen2 ok",mtchs[0],fcount);
    } else debug(F111,"zxpand 2 not ok",p,fcount);
    free(p);
    return(fcount);
}


/*  Z N E X T  --  Get name of next file from list created by zxpand(). */
/*
 Returns >0 if there's another file, with its name copied into the arg string,
 or 0 if no more files in list.
*/
int
znext(fn) char *fn; {
    if (fcount-- > 0) strcpy(fn,*mtchptr++);
    else *fn = '\0';
    debug(F111,"znext",fn,fcount+1);
    return(fcount+1);
}


/*  Z C H K S P A  --  Check if there is enough space to store the file  */

/*
 Call with file specification f, size n in bytes.
 Returns -1 on error, 0 if not enough space, 1 if enough space.
*/
int
#ifdef CK_ANSIC
zchkspa(char *f, long n)
#else
zchkspa(f,n) char *f; long n;
#endif /* CK_ANSIC */
/* zchkspa() */ {
#ifdef OS2
/* OS/2 gives us an easy way to do this. */
    long x, filesize = 0L;
    debug(F111,"zchkspa",f,n);
    if (isalpha(f[0]) && f[1] == ':') {
	x = zdskspace(toupper(f[0]) - 'A' + 1);
	debug(F101,"zchkspa disk: size","",x);
    } else {
	x = zdskspace(0);
	debug(F101,"zchkspa no disk size","",x);
    }
    if (fncact == XYFX_U || fncact == XYFX_X) /* Update or Replace */
      filesize = zchki(f) ; 

   return((x+filesize >= n) ? 1 : 0);
#else
/* In UNIX there is no good (and portable) way. */
    return(1);				/* Always say OK. */
#endif /* OS2 */
}


/*  Z N E W N  --  Make a new name for the given file  */

/*
  Given the name, fn, of a file that already exists, this function builds a
  new name of the form "<oldname>.~<n>~", where <oldname> is argument name
  (fn), and <n> is a version number, one higher than any existing version
  number for that file, up to 9999.  This format is consistent with that used
  by GNU EMACS.  If the constructed name is too long for the system's maximum,
  enough characters are truncated from the end of <fn> to allow the version
  number to fit.  If no free version numbers exist between 1 and 9999, a
  version number of "xxxx" is used.  Returns a pointer to the new name in
  argument s.
*/

VOID
znewn(fn,s) char *fn, **s; {
#ifdef pdp11
#define ZNEWNBL 63			/* Name buffer length */
#define ZNEWNMD 3			/* Max digits for version number */
#else
#define ZNEWNBL 255
#define ZNEWNMD 4
#endif /* pdp11 */

    static char buf[ZNEWNBL+1];
    char *bp, *xp, *yp;
#ifdef OS2
    char *tp, *zp, ch, temp[14];
#endif /* OS2 */
    int len = 0, d = 0, n, t, i, j, k, power = 1;

    int max = MAXNAMLEN;		/* Maximum name length */

    if (max < 14) max = 14;		/* Make it reasonable */
    if (max > ZNEWNBL) max = ZNEWNBL;
    bp = buf;				/* Buffer for building new name */
    yp = fn;
    while (*yp) {			/* Copy old name into buffer */
	*bp++ = *yp++;
	if (len++ > ZNEWNBL) break;	/* ...up to buffer length */
    }
    *s = NULL;
    for (i = 1; i < ZNEWNMD + 1; i++) {	/* Version numbers up to 10**i - 1 */
	power *= 10;			/* Next power of 10 */
	j = max - len;			/* Space left for version number */
	k = 3 + i;			/* Space needed for it */
	if (j < k) {			/* Make room if necessary */
	    len -= (k - j);		/* Adjust length of filename */
	    bp = buf + len;		/* Point to new end */
	}
	*bp++ = '*';			/* Put a star on the end (UNIX) */
	*bp-- = '\0';			/* Terminate with null */

	debug(F110,"znewn: about to expand",buf,0);
	n = zxpand(buf);		/* Expand the resulting wild name */
					/* n is the number of matches */
	debug(F101,"znewn matches","",n);
	while (n-- > 0) {		/* Find any existing name.~n~ files */
	    xp = *mtchptr++;		/* Point at matching name */
	    xp += len;			/* Look for .~<n>~ at the end of it */
	    if (*xp == '.' && *(xp+1) == '~') {	/* Has a version number */
		t = atoi(xp+2);		        /* Get it */
		if (t > d) d = t;	/* Save d = highest version number */
	    }
	}
	if (d < power-1) {		/* Less than maximum possible? */
	    debug(F110,"znewn number ok",buf,0);
	    sprintf(bp,".~%d~",d+1);	/* Yes, make "name.~<d+1>~" */
	    *s = buf;			/* Point to new name */
	    ck_znewn = d+1;		/* Also make it available globally */
	    break;			/* Done, return it */
	}
    }
    if (*s == NULL) {
	debug(F110,"znewn: too many names",buf,0);
	sprintf(bp,".~xxxx~");		/* Too many, use xxxx. */
	ck_znewn = -1;			/* Also make it available globally */
	*s = buf;
    }
#ifdef OS2
    if (IsFileNameValid(buf)) {
        debug(F110,"znewn: os2 filename valid",buf,0);
        return; /* HPFS */
    }
    /* otherwise make FAT 8.3 name */
    debug(F110,"znewn: os2 filename invalid",buf,0);
    xp = bp = buf;
    yp = fn;
    while (*yp) {			/* Copy name into buf */
	ch = *bp++ = *yp++;
	if (ISDIRSEP(ch) || (ch == ':')) xp=bp;
    }
    *bp = '\0';
    yp = xp;
    i = 1;
    while (*yp && (*yp != '.')) {
	yp++;
	if (++i<=6) zp=yp;
    }
    /* zp points to 6th character in name, or yp, whichever occurs first. */
    strcpy(temp,yp);			/* Copy extension, if any */
    while (zp != xp+8) {
    	if ( zp < xp+5 ) *zp++='0';
    	else *zp++='?';			/* Pad out with wild cards */
    }
    strcpy(zp,temp);			/* Get the extension back */
    debug(F110,"znewn: about to expand",buf,0);
    n = zxpand(buf);			/* Expand the resulting wild name */
    debug(F101,"znewn: matches","",n);
    d = 0;				/* Index number */
    debug(F110,"znewn: temp",temp,0);
    while (znext(temp)) {
#ifdef COMMENT
    	i = atoi(temp+5);
    	if (i > d) d = i;
#else
	if ( tp = strrchr( temp, '/' ) ) 
	  tp++;
	else
	  tp = temp ;   
    	i = atoi(tp+5);
	debug(F111,"znewn: tp=atoi(tp+5)",tp,i);
    	if (i > d) d = i;
	debug(F101,"znewn: d","",d);
#endif /* COMMENT */
    }
    sprintf(temp,"%03d",d+1);		/* Get the number into a string */
    ck_znewn = d+1;
    memcpy(xp+5, temp, 3);
    debug(F110,"znewn: os2 file name is FAT",buf,0);
#endif /* OS2 */
    return;
}


/*  Z R E N A M E  --  Rename a file  */
/*
   Call with old and new names.
   If new name is the name of a directory, the 'old' file is moved to
   that directory.
   Returns 0 on success, -1 on failure.
*/
int
zrename(old,new) char *old, *new; {
    char *p = NULL, *s = new;
    int x;

    debug(F110,"zrename old",old,0);
    debug(F110,"zrename new",s,0);
    if (isdir(new)) {
	char *q = NULL;
	x = strlen(new);
	if (!(p = malloc(strlen(new) + strlen(old) + 2)))
	  return(-1);
	strcpy(p,new);			/* Directory part */
	if (!ISDIRSEP(*(new+x-1)))	/* Separator, if needed */
	  strcat(p,"/");
	zstrip(old,&q);			/* Strip path part from old name */
	strcat(p,q);			/* Concatenate to new directory */
	s = p;
	debug(F110,"zrename dir",s,0);
    } else debug(F110,"zrename no dir",s,0);
#ifdef RENAME
/*
  Atomic, preferred, uses a single system call, rename(), if available.
  OS/2 rename() returns nonzero, but not necessarily -1 (?), on failure.
*/
    x = rename(old,s);
    if (p) free(p);
    return(x ? -1 : 0);
#else /* !RENAME */
/*
  This way has a window of vulnerability.
*/
    x = -1;				/* Return code. */
    if (link(old,s) < 0) {		/* Make a link with the new name. */
	debug(F111,"zrename link fails, errno",old,errno);
    } else if (unlink(old) < 0) {	/* Unlink the old name. */
	debug(F111,"zrename unlink fails, errno",old,errno);
    } else x = 0;
    if (p) free(p);
    return(x);
#endif /* RENAME */
}

/*  Z S A T T R */
/*
 Fills in a Kermit file attribute structure for the file which is to be sent.
 Returns 0 on success with the structure filled in, or -1 on failure.
 If any string member is null, then it should be ignored.
 If any numeric member is -1, then it should be ignored.
*/
int
zsattr(xx) struct zattr *xx; {
    long k;

    k = iflen % 1024L;			/* File length in K */
    if (k != 0L) k = 1L;
    xx->lengthk = (iflen / 1024L) + k;
    xx->type.len = 0;			/* File type can't be filled in here */
    xx->type.val = "";
    if (*nambuf) {
	xx->date.val = zfcdat(nambuf);	/* File creation date */
	xx->date.len = (int)strlen(xx->date.val);
    } else {
	xx->date.len = 0;
	xx->date.val = "";
    }
    xx->creator.len = 0;		/* File creator */
    xx->creator.val = "";
    xx->account.len = 0;		/* File account */
    xx->account.val = "";
    xx->area.len = 0;			/* File area */
    xx->area.val = "";
    xx->password.len = 0;		/* Area password */
    xx->password.val = "";
    xx->blksize = -1L;			/* File blocksize */
    xx->access.len = 0;			/* File access */
    xx->access.val = "";
    xx->encoding.len = 0;		/* Transfer syntax */
    xx->encoding.val = 0;
    xx->disp.len = 0;			/* Disposition upon arrival */
    xx->disp.val = "";
    xx->lprotect.len = 0;		/* Local protection */
    xx->lprotect.val = "";
    xx->gprotect.len = 0;		/* Generic protection */
    xx->gprotect.val = "";
    xx->systemid.len = 2;		/* System ID */
#ifdef OS2
    xx->systemid.val = "UO";		/* UO = OS/2 */
#else
    xx->systemid.val = "U1";		/* U1 = UNIX */
#endif /* OS2 */
    xx->recfm.len = 0;			/* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;		/* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;			/* Length */
    return(0);
}

/* Z F C D A T  --  Get file creation date */
/*
  Call with pointer to filename.
  On success, returns pointer to modification date in yyyymmdd hh:mm:ss format.
  On failure, returns pointer to null string.
*/
static char datbuf[40];

/* static */				/* (===OS2 change===) */
char *
zfcdat(name) char *name; {

#ifdef TIMESTAMP
    struct stat buffer;
    struct tm *time_stamp, *localtime();
    int yy, ss;

    datbuf[0] = '\0';
    if(stat(name,&buffer) != 0) {
	debug(F110,"zfcdat stat failed",name,0);
	return("");
    }
    time_stamp = localtime(&(buffer.st_mtime));
    yy = time_stamp->tm_year;
    if (yy < 100)			/* In case it returns 2-digit year? */
      yy += 1900;
    if (yy < 0 || yy > 9999) {		/* Make sure year is ok */
	debug(F110,"zfcdat date failed",name,0);
	return("");
    }
    if (time_stamp->tm_mon  < 0 || time_stamp->tm_mon  > 11)
      return("");
    if (time_stamp->tm_mday < 0 || time_stamp->tm_mday > 31)
      return("");
    if (time_stamp->tm_hour < 0 || time_stamp->tm_hour > 23)
      return("");
    if (time_stamp->tm_min  < 0 || time_stamp->tm_min  > 59)
      return("");
    ss = time_stamp->tm_sec;		/* Seconds */
    if (ss < 0 || ss  > 59)		/* Some systems give a BIG number */
      ss = 0;
    sprintf(datbuf,
#ifdef pdp11
/* For some reason, 2.1x BSD sprintf gets the last field wrong. */
	    "%04d%02d%02d %02d:%02d:00",
#else
	    "%04d%02d%02d %02d:%02d:%02d",
#endif /* pdp11 */
	    yy,
	    time_stamp->tm_mon + 1,
	    time_stamp->tm_mday,
	    time_stamp->tm_hour,
	    time_stamp->tm_min
#ifndef pdp11
	    , ss
#endif /* pdp11 */
	    );
    yy = (int)strlen(datbuf);
    debug(F111,"zfcdat",datbuf,yy);
    if (yy > 17) datbuf[17] = '\0';
    return(datbuf);
#else
    return("");
#endif /* TIMESTAMP */
}

/* Z S T I M E  --  Set creation date for incoming file */
/*
 Call with:
 f  = pointer to name of existing file.
 yy = pointer to a Kermit file attribute structure in which yy->date.val
      is a date of the form yyyymmdd hh:mm:ss, e.g. 19900208 13:00:00.
 x  = is a function code: 0 means to set the file's creation date as given.
      1 means compare the given date with the file creation date.
 Returns:
 -1 on any kind of error.
  0 if x is 0 and the file date was set successfully.
  0 if x is 1 and date from attribute structure <= file creation date.
  1 if x is 1 and date from attribute structure > file creation date.
*/
int
zstime(f,yy,x)
#ifdef HPUX10
#ifdef CK_ANSIC
  const 
#endif /* CK_ANSIC */
#endif /* HPUX10 */
  char *f; struct zattr *yy; int x;
/* zstime */ {
    int r = -1;				/* Return code */
/*
  It is ifdef'd TIMESTAMP because it might not work on V7. bk@kullmar.se.
*/
#ifdef TIMESTAMP
/*
  To do: adapt code from OS-9 Kermit's ck9fio.c zstime function, which
  is more flexible, allowing [yy]yymmdd[ hh:mm[:ss]].
*/
#ifndef OS2
#ifdef M_UNIX
/*
  SCO UNIX 3.2v2.0 and ODT 2.0 lack prototypes for ftime().
  ODT 3.0 (3.2v4.2 OS) has a prototype, which may vary in
  dependence on the XPG4 supplement presence.  So always use
  what the system header file supplies in ODT 3.0...
*/
#ifndef ODT30
    extern void ftime();  /* extern void ftime(struct timeb *) */
#endif /* ODT30 */
#else
    extern int ftime();    
#endif /* M_UNIX */
    extern int stat();

#ifdef BSD44
    extern int utimes();
#else
    extern int utime();
#endif /* BSD44 */
    /* at least, the declarations for int functions are not needed anyway */
    extern struct tm *localtime();
    /* and this should have been declared always through a header file */
#endif /* OS2 */
    long tm, days;
    int i, n, isleapyear;
                   /*       J  F  M  A   M   J   J   A   S   O   N   D   */
                   /*      31 28 31 30  31  30  31  31  30  31  30  31   */
    static
    int monthdays [13] = {  0,0,31,59,90,120,151,181,212,243,273,304,334 };
    char s[5];
    struct stat sb;
#ifdef BSD44
    struct timeval tp[2];
    long xtimezone;
#else
#ifdef OS2
    struct utimbuf tp;
#ifdef __EMX__
    long timezone;
    struct timeb tbp;
#endif /* __EMX__ */
#else
#ifdef V7
    struct utimbuf {
      time_t timep[2];		/* New access and modificaton time */
    } tp;
    char *tz;
    long timezone;		/* In case timezone not defined in .h file */
#else
#ifdef SYSUTIMEH
    struct utimbuf tp;
#else
    struct utimbuf {
	time_t atime;
	time_t mtime;
    } tp;
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* OS2 */
#endif /* BSD44 */

#ifdef ANYBSD
    long timezone;
    static struct timeb tbp;
#endif /* ANYBSD */

    debug(F110,"zstime",f,0);

    if ((yy->date.len == 0)
        || (yy->date.len != 17)
        || (yy->date.val[8] != ' ')
        || (yy->date.val[11] != ':')
        || (yy->date.val[14] != ':') ) {
        debug(F111,"Bad creation date ",yy->date.val,yy->date.len);
        return(-1);
    }
    debug(F111,"zstime date check 1",yy->date.val,yy->date.len);
    for(i = 0; i < 8; i++) {
	if (!isdigit(yy->date.val[i])) {
	    debug(F111,"Bad creation date ",yy->date.val,yy->date.len);
	    return(-1);
	}
    }
    debug(F111,"zstime date check 2",yy->date.val,yy->date.len);
    i++;

    for (; i < 16; i += 3) {
	if ((!isdigit(yy->date.val[i])) || (!isdigit(yy->date.val[i + 1]))) {
	    debug(F111,"Bad creation date ",yy->date.val,yy->date.len);
	    return(-1);
	}
    }
    debug(F111,"zstime date check 3",yy->date.val,yy->date.len);

#ifdef BSD44
    {
	int x;
	struct timezone tzp;
	x = gettimeofday(NULL, &tzp);
	debug(F101,"zstime BSD44 gettimeofday","",x);
	if (x > -1)
	  xtimezone = tzp.tz_minuteswest * 60L;
	else
	  xtimezone = 0L;
	debug(F101,"zstime BSD44 timezone","",xtimezone);
    }
#else
#ifdef ANYBSD
    debug(F100,"zstime BSD calling ftime","",0);
    ftime(&tbp);
    debug(F100,"zstime BSD back from ftime","",0);
    timezone = tbp.timezone * 60L;
    debug(F101,"zstime BSD timezone","",timezone);
#else
#ifdef OS2
#ifdef __EMX__
    ftime(&tbp);
    timezone = tbp.timezone * 60L;
#endif /* __EMX__ */
#else
#ifdef SVORPOSIX
    tzset();				/* Set timezone */
#else
#ifdef V7
    if ((tz = getenv("TZ")) == NULL)
      timezone = 0;			/* UTC/GMT */
    else
      timezone = atoi(&tz[3]);		/* Set 'timezone'. */
    timezone *= 60L;
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* OS2 */
#endif /* ANYBSD */
#endif /* BSD44 */

    debug(F100,"zstime so far so good","",0);

    s[4] = '\0';
    for (i = 0; i < 4; i++)		/* Fix the year */
      s[i] = yy->date.val[i];

    n = atoi(s);

    debug(F111,"zstime year",s,n);

/*  Previous year's leap days.  This won't work after year 2100. */

    isleapyear = (( n % 4 == 0 && n % 100 !=0) || n % 400 == 0);
    days = (long) (n - 1970) * 365;
    days += (n - 1968 - 1) / 4 - (n - 1900 - 1) / 100 + (n - 1600 - 1) / 400;

    s[2] = '\0';

    for (i = 4 ; i < 16; i += 2) {
	s[0] = yy->date.val[i];
	s[1] = yy->date.val[i + 1];
	n = atoi(s);
	switch (i) {
	  case 4:			/* MM: month */
	    if ((n < 1 ) || ( n > 12)) {
		debug(F111,"zstime 4 bad date ",yy->date.val,yy->date.len);
		return(-1);
	    }
	    days += monthdays [n];
	    if (isleapyear && n > 2)
	      ++days;
	    continue;

	  case 6:			/* DD: day */
	    if ((n < 1 ) || ( n > 31)) {
		debug(F111,"zstime 6 bad date ",yy->date.val,yy->date.len);
		return(-1);
	    }
	    tm = (days + n - 1) * 24L * 60L * 60L;
	    i++;			/* Skip the space */
	    continue;

	  case 9:			/* hh: hour */
	    if ((n < 0 ) || ( n > 23)) {
		debug(F111,"zstime 9 bad date ",yy->date.val,yy->date.len);
		return(-1);
	    }
	    tm += n * 60L * 60L;
	    i++;			/* Skip the colon */
	    continue;

	  case 12:			/* mm: minute */
	    if ((n < 0 ) || ( n > 59)) {
		debug(F111,"zstime 12 bad date ",yy->date.val,yy->date.len);
		return(-1);
	    }
#ifdef BSD44				/* Correct for time zone */
	    tm += xtimezone;
	    debug(F101,"zstime BSD44 tm","",tm);
#else
#ifdef ANYBSD
	    tm += timezone;
#else
#ifndef CONVEX9 /* Don't yet know how to do this here */
#ifdef ultrix
	    tm += (long) timezone;
#else
	    tm += timezone;
#endif /* ultrix */
#endif /* CONVEX9 */
#endif /* BSD44 */
#endif /* ANYBSD */
	    tm += n * 60L;
	    i++;			/* Skip the colon */
	    continue;

	  case 15:			/* ss: second */
	    if ((n < 0 ) || ( n > 59)) {
		debug(F111,"zstime 15 bad date ",yy->date.val,yy->date.len);
		return(-1);
	    }
	    tm += n;
	}
	if (localtime(&tm)->tm_isdst)
	  tm -= 60L * 60L;		/* Adjust for daylight savings time */
    }
    debug(F101,"tm","",tm);
    debug(F111,"A-pkt date ok ",yy->date.val,yy->date.len);

    if (stat(f,&sb)) {			/* Get the time for the file */
	debug(F110,"Can't stat file:",f,0);
	return(-1);
    }
    debug(F101,"sb.st_atime","",sb.st_atime);

#ifdef OS2
    tp.modtime = tm;			/* Set modif. time to creation date */
    tp.actime = sb.st_atime;		/* Don't change the access time */
#else
#ifdef SYSUTIMEH
    tp.modtime = tm;			/* Set modif. time to creation date */
    tp.actime = sb.st_atime;		/* Don't change the access time */
#else
#ifdef V7
    tp.timep[0] = tm;			/* Set modif. time to creation date */
    tp.timep[1] = sb.st_atime;		/* Don't change the access time */
#else
#ifdef BSD44
    tp[0].tv_sec = sb.st_atime;		/* Access time first */
    tp[1].tv_sec = tm;			/* Update time second */
#else
    tp.mtime = tm;			/* Set modif. time to creation date */
    tp.atime = sb.st_atime;		/* Don't change the access time */
#endif /* BSD44 */
#endif /* V7 */
#endif /* SYSUTIMEH */
#endif /* OS2 */

    switch (x) {			/* Execute desired function */
      case 0:				/* Set the creation date of the file */
	if (
#ifdef BSD44
	    utimes(f,tp)
#else
	    utime(f,&tp)
#endif /* BSD44 */
	    ) {		/* Fix modification time */
	    debug(F110,"Can't set modification time for file: ",f,0);
	    r = -1;
	} else  {
	    debug(F110,"Modification time is set for file: ",f,0);
	    r = 0;
	}
	break;
      case 1:				/* Compare the dates */
/*
  This was st_atime, which was wrong.  We want the file-data modification
  time, st_mtime.
*/
	debug(F111,"zstime compare",f,sb.st_mtime);
	debug(F111,"zstime compare","packet",tm);
#ifdef OS2
/*
  In OS/2, sb.st_mtime, at least on a FAT file system, is always even.
  In that case, if the incoming file is only one second newer than the
  local file, consider them the same.  (THERE MUST BE A BETTER FIX FOR THIS!)
*/
	if ((sb.st_mtime & 1) == 0)
	  if ((tm - sb.st_mtime) == 1)
	    tm--;
#endif /* OS2 */
	r = (sb.st_mtime < tm) ? 0 : 1;
	break;

      default:				/* Error */
	r = -1;
    }
#endif /* TIMESTAMP */
    return(r);
}

/* Find initialization file. */

#ifdef NOTUSED
int
zkermini() {
/*  nothing here for Unix.  This function added for benefit of VMS Kermit.  */
    return(0);
}
#endif /* NOTUSED */

#ifndef NOFRILLS
int
zmail(p,f) char *p; char *f; {		/* Send file f as mail to address p */
/*
  Returns 0 on success
   2 if mail delivered but temp file can't be deleted
  -2 if mail can't be delivered
  The UNIX version always returns 0 because it can't get a good return
  code from zsyscmd.
*/
#ifdef BSD4
/* The idea is to use /usr/ucb/mail, rather than regular mail, so that   */
/* a subject line can be included with -s.  Since we can't depend on the */
/* user's path, we use the convention that /usr/ucb/Mail = /usr/ucb/mail */
/* and even if Mail has been moved to somewhere else, this should still  */
/* find it...  The search could be made more reliable by actually using  */
/* access() to see if /usr/ucb/Mail exists. */

/* Should also make some check on zmbuf overflow... */

#ifdef DGUX540
    sprintf(zmbuf,"mailx -s %c%s%c %s < %s", '"', f, '"', p, f);
#else
    sprintf(zmbuf,"Mail -s %c%s%c %s < %s", '"', f, '"', p, f);
#endif /* DGUX540 */
    zsyscmd(zmbuf);
#else
#ifdef SVORPOSIX
#ifndef OXOS
    sprintf(zmbuf,"mail %s < %s", p, f);
#else /* OXOS */
    sprintf(zmbuf,"mailx -s %c%s%c %s < %s", '"', f, '"', p, f);
#endif /* OXOS */
    zsyscmd(zmbuf);
#else
    *zmbuf = '\0';
#endif
#endif
    return(0);
}
#endif /* NOFRILLS */

#ifndef NOFRILLS
int
zprint(p,f) char *p; char *f; {		/* Print file f with options p */

#ifdef OS2
    sprintf(zmbuf,"print %s %s", p, f); /* Construct print command */
    zsyscmd(zmbuf);
#else
#ifdef UNIX
#ifdef ANYBSD				/* BSD uses lpr to spool */
#ifdef DGUX540				/* And DG/UX */
#define SPOOLER "lp"
#else
#define SPOOLER "lpr"
#endif /* DGUX540 */
#else					/* Sys V uses lp */
#ifdef TRS16				/* except for Tandy-16/6000... */
#define SPOOLER "lpr"
#else
#define SPOOLER "lp"
#endif
#endif
/*
  Note use of standard input redirection.  In some systems, lp[r] runs
  setuid to lp (or ...?), so if user has sent a file into a directory
  that lp does not have read access to, it can't be printed unless it is
  feed to lp[r] as standard input.
*/
    sprintf(zmbuf,"%s %s < %s", SPOOLER, p, f); /* Construct print command */
    zsyscmd(zmbuf);
#else /* Not UNIX */
    *zmbuf = '\0';
#endif /* UNIX */
#endif /* OS2 */
    return(0);
}
#endif /* NOFRILLS */

/*
  Wildcard expansion functions.  C-Kermit used to insist on doing this itself
  New code (version 5A, 1990-91) gives user option to ask UNIX to do it.
  This lets users use the wildcard expansion features of their favorite shell.
  Operation is slower because of the forking & piping, but flexibility is
  greater and program is smaller.  For OS/2, C-Kermit still does this itself.
*/
static char scratch[MAXPATH+4];		/* Used by both methods */

#ifndef OS2
static int oldmtchs = 0;		/* Let shell (ls) expand them. */
#ifdef COMMENT
static char *lscmd = "/bin/ls -d"; 	/* Command to use. */
#else
static char *lscmd = "echo";		/* Command to use. */
#endif /* COMMENT */

#ifndef NOPUSH
int
shxpand(pat,namlst,len) char *pat, *namlst[]; int len; {
    char *fgbuf = NULL;			/* Buffer for forming ls command */
    char *p, *q;			/* Workers */
    int i, x, retcode; char c;		/* ... */

    x = (int)strlen(pat) + (int)strlen(lscmd) + 3; /* Length of ls command */
    for (i = 0; i < oldmtchs; i++)	/* Free previous file list */
      free(namlst[i]);
    fgbuf = malloc(x);			/* Get buffer for command */
    if (!fgbuf) return(-1);		/* Fail if cannot */
    sprintf(fgbuf,"%s %s",lscmd,pat);	/* Form the command */
    zxcmd(ZIFILE,fgbuf);		/* Start the command */
    i = 0;				/* File counter */
    p = scratch;			/* Point to scratch area */
    retcode = -1;			/* Assume failure */
    while ((x = zminchar()) != -1) {	/* Read characters from command */
	c = (char) x;
	if (c == ' ' || c == '\n') {	/* Got newline or space? */
	    *p = '\0';			/* Yes, terminate string */
	    p = scratch;		/* Point back to beginning */
	    if (zchki(p) == -1)		/* Does file exist? */
	      continue;			/* No, continue */
	    x = (int)strlen(p);		/* Yes, get length of name */
	    q = malloc(x+1);		/* Allocate space for it */
	    if (!q) goto shxfin;	/* Fail if space can't be obtained */
	    strcpy(q,scratch);		/* Copy name to space */
	    namlst[i++] = q;		/* Copy pointer to name into array */
	    if (i > len) goto shxfin;	/* Fail if too many */
	} else {			/* Regular character */
	    *p++ = c;			/* Copy it into scratch area */
	}
    }
    retcode = i;			/* Return number of matching files */
shxfin:					/* Common exit point */
    free(fgbuf);			/* Free command buffer */
    zclosf(ZIFILE);			/* Delete the command fork. */
    oldmtchs = i;			/* Remember how many files */
    return(retcode);
}
#endif /* NOPUSH */
#endif /* OS2 */

/* Directory Functions for Unix, written by Jeff Damens, CUCCA, 1984. */

/* Define the size of the string space for filename expansion. */

#ifndef DYNAMIC
#ifdef PROVX1
#define SSPACE 500
#else
#ifdef BSD29
#define SSPACE 500
#else
#ifdef pdp11
#define SSPACE 500
#else
#ifdef aegis
#define SSPACE 10000			/* size of string-generating buffer */
#else					/* Default static buffer size */
#define SSPACE 2000			/* size of string-generating buffer */
#endif /* aegis */
#endif /* pdp11 */
#endif /* BSD29 */
#endif /* PROVX1 */
static char sspace[SSPACE];             /* buffer for generating filenames */
#else /* DYNAMIC */
#define SSPACE 10000
static char *sspace = (char *)0;
#endif /* DYNAMIC */
#ifdef aegis
static char bslash;
#endif /* aegis */
static int ssplen = SSPACE;		/* length of string space buffer */

static char *freeptr,**resptr;         	/* copies of caller's arguments */
static int remlen;                      /* remaining length in caller's array*/
static int numfnd;                      /* number of matches found */

/*
 * splitpath:
 *  takes a string and splits the slash-separated portions into
 *  a list of path structures.  Returns the head of the list.  The
 *  structures are allocated by malloc, so they must be freed.
 *  Splitpath is used internally by the filename generator.
 *
 * Input: A string.
 * Returns: A linked list of the slash-separated segments of the input.
 */

struct path *
splitpath(p) char *p; {
    struct path *head,*cur,*prv;
    int i;

    debug(F110,"splitpath",p,0);

    head = prv = NULL;
    if (ISDIRSEP(*p)) p++;			/* skip leading slash */
    while (*p != '\0') {
	cur = (struct path *) malloc(sizeof (struct path));
	debug(F101,"splitpath malloc","",cur);
	if (cur == NULL) {
	    debug(F100,"splitpath malloc failure","",0);
	    return((struct path *)NULL);
	}
	cur -> fwd = NULL;
	if (head == NULL)
	  head = cur;
	else
	  prv -> fwd = cur;		/* link into chain */
	prv = cur;
#ifdef aegis
	/* treat backslash as "../" */
	if (bslash && *p == bslash) {
	    strcpy(cur->npart, "..");
	    ++p;
	} else {
	    for (i=0; i < MAXNAMLEN && *p && *p != '/' && *p != bslash; i++)
	      cur -> npart[i] = *p++;
	    cur -> npart[i] = '\0';	/* end this segment */
	    if (i >= MAXNAMLEN)
	      while (*p && *p != '/' && *p != bslash)
		p++;
	}
	if (*p == '/') p++;
#else
#ifdef OS2
	for (i = 0;
	     i < MAXNAMLEN && !ISDIRSEP(*p) && *p != ':' && *p != '\0';
	     i++ )
	    cur -> npart[i] = *p++;
        if ( *p == ':' ) {
            cur -> npart[i++] = *p++;
            if ( !ISDIRSEP(*p) )
                cur -> npart[i++] = '.';
        }
#else
	for (i=0; i < MAXNAMLEN && !ISDIRSEP(*p) && *p != '\0'; i++) {
	    cur -> npart[i] = *p++;
	}
#endif /* OS2 */
	cur -> npart[i] = '\0';		/* end this segment */
	if (i >= MAXNAMLEN)
	  while (!ISDIRSEP(*p) && *p != '\0') p++;
	if (ISDIRSEP(*p))
	  p++;

#endif /* aegis */
    }
    return(head);
}

/*
 * fgen:
 *  This is the actual name generator.  It is passed a string,
 *  possibly containing wildcards, and an array of character pointers.
 *  It finds all the matching filenames and stores them into the array.
 *  The returned strings are allocated from a static buffer local to
 *  this module (so the caller doesn't have to worry about deallocating
 *  them); this means that successive calls to fgen will wipe out
 *  the results of previous calls.  This isn't a problem here
 *  because we process one wildcard string at a time.
 *
 * Input: a wildcard string, an array to write names to, the
 *        length of the array.
 * Returns: the number of matches.  The array is filled with filenames
 *          that matched the pattern.  If there wasn't enough room in the
 *	    array, -1 is returned.
 * Originally by: Jeff Damens, CUCCA, 1984.  Many changes since then.
 */
static int
fgen(pat,resarry,len) char *pat,*resarry[]; int len; {
    struct path *head;
    char *sptr;
#ifdef aegis
    char *namechars;
    int tilde = 0, bquote = 0;

    if ((namechars = getenv("NAMECHARS")) != NULL) {
	if (xindex(namechars, '~' ) != NULL) tilde  = '~';
	if (xindex(namechars, '\\') != NULL) bslash = '\\';
	if (xindex(namechars, '`' ) != NULL) bquote = '`';
    } else {
	tilde = '~'; bslash = '\\'; bquote = '`';
    }

    sptr = scratch;

    /* copy "`node_data", etc. anchors */
    if (bquote && *pat == bquote)
      while (*pat && *pat != '/' && *pat != bslash)
	*sptr++ = *pat++;
    else if (tilde && *pat == tilde)
      *sptr++ = *pat++;
    while (*pat == '/')
      *sptr++ = *pat++;
    if (sptr == scratch) {
	strcpy(scratch,"./");
	sptr = scratch+2;
    }					/* init buffer correctly */
    if (!(head = splitpath(pat))) return(-1);
#else /* not aegis */
    debug(F110,"fgen pat",pat,0);
    if (!(head = splitpath(pat))) return(-1);
    sptr = scratch;
    if (!ISDIRSEP(*pat))
	*sptr++ = '.';                  /* init buffer correctly */
    *sptr++ = DIRSEP;
#ifdef OS2
    if (isalpha(pat[0]) && pat[1] == ':')
        sptr = scratch;                 /* reset in case of leading drive: */
#endif /* OS2 */
#endif /* aegis */
    numfnd = 0;				/* none found yet */
#ifdef DYNAMIC
    if (!sspace) {			/* Need to allocate string space? */
	while (ssplen > 50) {
	    if ((sspace = malloc(ssplen+2))) { /* Got it. */
		debug(F101,"fgen string space","",ssplen);
		break;
	    }
	    ssplen = (ssplen / 2) + (ssplen / 4); /* Didn't, reduce by 3/4 */
	}
	if (ssplen <= 50) {		/* Did we get it? */
	    fprintf(stderr,"fgen can't malloc string space\n");
	    return(-1);
	}
    }
#endif /* DYNAMIC */
    freeptr = sspace;			/* this is where matches are copied */
    resptr = resarry;			/* static copies of these so */
    remlen = len;			/* recursive calls can alter them */
    traverse(head,scratch,sptr);	/* go walk the directory tree */
#ifdef COMMENT
/*
  This code, circa 1984, has never worked right - it references the head
  pointer after it has already been freed.  Lord knows what might have been
  happening because of this.  Thanks to Steve Walton for finding & fixing
  this bug.
*/
    for (; head != NULL; head = head -> fwd)
      free(head);			/* return the path segments */
#else
    while (head != NULL) {
	struct path *next = head -> fwd;
	free(head);
	head = next;
    }
#endif /* COMMENT */
    return(numfnd);			/* and return the number of matches */
}

/* traverse:
 *  Walks the directory tree looking for matches to its arguments.
 *  The algorithm is, briefly:
 *   If the current pattern segment contains no wildcards, that
 *   segment is added to what we already have.  If the name so far
 *   exists, we call ourselves recursively with the next segment
 *   in the pattern string; otherwise, we just return.
 *
 *   If the current pattern segment contains wildcards, we open the name
 *   we've accumulated so far (assuming it is really a directory), then read
 *   each filename in it, and, if it matches the wildcard pattern segment, add
 *   that filename to what we have so far and call ourselves recursively on the
 *   next segment.
 *
 *   Finally, when no more pattern segments remain, we add what's accumulated
 *   so far to the result array and increment the number of matches.
 *
 * Input: a pattern path list (as generated by splitpath), a string
 *	  pointer that points to what we've traversed so far (this
 *	  can be initialized to "/" to start the search at the root
 *	  directory, or to "./" to start the search at the current
 *	  directory), and a string pointer to the end of the string
 *	  in the previous argument.
 * Returns: nothing.
 */
static VOID
traverse(pl,sofar,endcur) struct path *pl; char *sofar, *endcur; {

/* Define LONGFN (long file names) automatically for BSD 2.9 and 4.2 */
/* LONGFN can also be defined on the cc command line. */

#ifdef BSD29
#ifndef LONGFN
#define LONGFN
#endif
#endif

#ifdef BSD42
#ifndef LONGFN
#define LONGFN
#endif
#endif

/* Appropriate declarations for directory routines and structures */
/* #define OPENDIR means to use opendir(), readdir(), closedir()  */
/* If OPENDIR not defined, we use open(), read(), close() */

#ifdef DIRENT				/* New way, <dirent.h> */
#define OPENDIR
    DIR *fd, *opendir();
    struct dirent *dirbuf;
    struct dirent *readdir();
#else /* !DIRENT */
#ifdef LONGFN				/* Old way, <dir.h> with opendir() */
#define OPENDIR
    DIR *fd, *opendir();
    struct direct *dirbuf;
#else /* !LONGFN */
    int fd;				/* Old way, <dir.h> with open() */
    struct direct dir_entry;
    struct direct *dirbuf = &dir_entry;
#endif /* LONGFN */
#endif /* DIRENT */

    struct stat statbuf;		/* for file info */

    if (pl == NULL) {
	*--endcur = '\0';		/* end string, overwrite trailing / */
	addresult(sofar);
	return;
    }
    if (!iswild(pl -> npart)) {
	strcpy(endcur,pl -> npart);
	endcur += (int)strlen(pl -> npart);
	*endcur = '\0';			/* end current string */
	if (stat(sofar,&statbuf) == 0) { /* if current piece exists */
#ifdef OS2
	    if (endcur - sofar == 3 && endcur[-1] == '.' && endcur[-2] == ':')
	      endcur--;
	    else
#endif /* OS2 */
	      *endcur++ = DIRSEP;	/* add slash to end */
	    *endcur = '\0';		/* and end the string */
	    traverse(pl -> fwd,sofar,endcur);
	}
	return;
    }

    /* Segment contains wildcards, have to search directory */

    *endcur = '\0';                        	/* end current string */
    if (stat(sofar,&statbuf) == -1) return;   	/* doesn't exist, forget it */
    if (!S_ISDIR (statbuf.st_mode)) return; 	/* not a directory, skip */

#ifdef OPENDIR
    if ((fd = opendir(sofar)) == NULL) return; /* Can't open, fail. */
    while (dirbuf = readdir(fd))
#else /* !OPENDIR */
    if ((fd = open(sofar,O_RDONLY)) < 0) return; /* Can't open, fail. */
    while (read(fd, (char *)dirbuf, sizeof dir_entry))
#endif /* OPENDIR */
      {
	  /* Get null-terminated copy!!! */
	  strncpy(nambuf,dirbuf->d_name,MAXNAMLEN);
	  nambuf[MAXNAMLEN] = '\0';
#ifdef unos
	  if (dirbuf->d_ino != -1 && match(pl -> npart,nambuf))
#else
/* #ifdef _POSIX_SOURCE */
/*
  Directory reading is not specified in POSIX.1.  POSIX.2 gives us glob() and
  fnmatch(), which are not yet supported by C-Kermit.  Meanwhile, maybe POSIX
  implementations should force "set wildcard shell" and remove all of this
  code.
*/
#ifdef QNX
	  if (dirbuf->d_stat.st_ino != 0 && match(pl -> npart,nambuf))
#else
#ifdef SOLARIS
	  if (dirbuf->d_ino != 0 && match(pl -> npart,nambuf))
#else
#ifdef sun
	  if (dirbuf->d_fileno != 0 && match(pl -> npart,nambuf))
#else
#ifdef bsdi
	  if (dirbuf->d_fileno != 0 && match(pl -> npart,nambuf))
#else
#ifdef __386BSD__
	  if (dirbuf->d_fileno != 0 && match(pl -> npart,nambuf))
#else
#ifdef __FreeBSD__
	  if (dirbuf->d_fileno != 0 && match(pl -> npart,nambuf))
#else
#ifdef ultrix
	  if (dirbuf->gd_ino != 0 && match(pl -> npart,nambuf))
#else
	  if (dirbuf->d_ino != 0 && match(pl -> npart,nambuf))
#endif /* ultrix */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* bsdi */
#endif /* sun */
#endif /* SOLARIS */
#endif /* QNX */

/* #else */ /* not _POSIX_SOURCE */
/*	  if (dirbuf->d_ino != 0 && match(pl -> npart,nambuf)) */
/* #endif */ /* _POSIX_SOURCE */

#endif /* unos */
	  {
	      char *eos;
	      strcpy(endcur,nambuf);
	      eos = endcur + (int)strlen(nambuf);
	      *eos++ = DIRSEP;		/* end this segment */
	      traverse(pl -> fwd,sofar,eos);
	  }
      }
#ifdef OPENDIR
    closedir(fd);
#else /* !OPENDIR */
    close(fd);
#endif /* OPENDIR */
}

/*
 * addresult:
 *  Adds a result string to the result array.  Increments the number
 *  of matches found, copies the found string into our string
 *  buffer, and puts a pointer to the buffer into the caller's result
 *  array.  Our free buffer pointer is updated.  If there is no
 *  more room in the caller's array, the number of matches is set to -1.
 * Input: a result string.
 * Returns: nothing.
 */
static VOID
addresult(str) char *str; {
    int l;
    debug(F111,"addresult",str,remlen);
    if (str[0] == '.' && ISDIRSEP(str[1])) str += 2; /* (===OS2 change===) */
    if (--remlen < 0) {
	numfnd = -1;
	return;
    }
    l = (int)strlen(str) + 1;		/* size this will take up */
    if ((freeptr + l) > (sspace + ssplen)) {
	numfnd = -1;			/* do not record if not enough space */
	return;
    }
    strcpy(freeptr,str);
    *resptr++ = freeptr;
    freeptr += l;
    numfnd++;
}

/*
 * match:
 *  pattern matcher.  Takes a string and a pattern possibly containing
 *  the wildcard characters '*' and '?'.  Returns true if the pattern
 *  matches the string, false otherwise.
 * by: Jeff Damens, CUCCA, 1984
 * skipping over dot files and backslash quoting added by fdc, 1990.
 *
 * Input: a string and a wildcard pattern.
 * Returns: 1 if match, 0 if no match.
 */
static int
match(pattern,string) char *pattern,*string; {
    char *psave,*ssave;			/* back up pointers for failure */
    int q = 0;				/* quote flag */

    debug(F110,"match str",string,0);
    psave = ssave = NULL;
#ifndef MATCHDOT
    if (*string == '.' && *pattern != '.') {
	debug(F110,"match skip",string,0);
	return(0);
    }
#endif
    while (1) {
#ifdef OS2
	for (; tolower(*pattern) == tolower(*string); pattern++,string++)
#else
	for (; *pattern == *string; pattern++,string++)  /* skip first */
#endif /* OS2 */
	    if (*string == '\0') return(1);	/* end of strings, succeed */

	if (*pattern == '\\' && q == 0) { /* Watch out for quoted */
	    q = 1;			/* metacharacters */
	    pattern++;			/* advance past quote */
	    if (*pattern != *string) return(0);
	    continue;
	} else q = 0;

	if (q) {
	    return(0);
	} else {
	    if (*string != '\0' && *pattern == '?') {
		pattern++;		/* '?', let it match */
		string++;
	    } else if (*pattern == '*') { /* '*' ... */
		psave = ++pattern;	/* remember where we saw it */
		ssave = string;		/* let it match 0 chars */
	    } else if (ssave != NULL && *ssave != '\0') { /* if not at end  */
					/* ...have seen a star */
		string = ++ssave;	/* skip 1 char from string */
		pattern = psave;	/* and back up pattern */
	    } else return(0);		/* otherwise just fail */
	}
    }
}

/*
  The following two functions are for expanding tilde in filenames
  Contributed by Howie Kaye, CUCCA, developed for CCMD package.
*/

/*  W H O A M I  --  Get user's username.  */

/*
  1) Get real uid
  2) See if the $USER environment variable is set ($LOGNAME on AT&T)
  3) If $USER's uid is the same as ruid, realname is $USER
  4) Otherwise get logged in user's name
  5) If that name has the same uid as the real uid realname is loginname
  6) Otherwise, get a name for ruid from /etc/passwd
*/
static char *
whoami () {
#ifdef DTILDE
#ifdef pdp11
#define WHOLEN 100
#else
#define WHOLEN 257
#endif /* pdp11 */
    static char realname[256];		/* user's name */
    static int ruid = -1;		/* user's real uid */
    char loginname[256], envname[256];	/* temp storage */
    char *c;
    struct passwd *p;
    _PROTOTYP(extern char * getlogin, (void) );

    if (ruid != -1)
      return(realname);

    ruid = real_uid();			/* get our uid */

  /* how about $USER or $LOGNAME? */
    if ((c = getenv(NAMEENV)) != NULL) { /* check the env variable */
	strcpy (envname, c);
	if ((p = getpwnam(envname)) != NULL) {
	    if (p->pw_uid == ruid) {	/* get passwd entry for envname */
		strcpy (realname, envname); /* if the uid's are the same */
		return(realname);
	    }
	}
    }

  /* can we use loginname() ? */

    if ((c =  getlogin()) != NULL) {	/* name from utmp file */
	strcpy (loginname, c);
	if ((p = getpwnam(loginname)) != NULL) /* get passwd entry */
	  if (p->pw_uid == ruid) {	/* for loginname */
	      strcpy (realname, loginname); /* if the uid's are the same */
	      return(realname);
	  }
    }

  /* Use first name we get for ruid */

    if ((p = getpwuid(ruid)) == NULL) { /* name for uid */
	realname[0] = '\0';		/* no user name */
	ruid = -1;
	return(NULL);
    }
    strcpy (realname, p->pw_name);
    return(realname);
#else
    return(NULL);
#endif /* DTILDE */
}

/*  T I L D E _ E X P A N D  --  expand ~user to the user's home directory. */

char *
tilde_expand(dirname) char *dirname; {
#ifdef DTILDE
#ifdef pdp11
#define BUFLEN 100
#else
#define BUFLEN 257
#endif /* pdp11 */
    struct passwd *user;
    static char olddir[BUFLEN];
    static char oldrealdir[BUFLEN];
    static char temp[BUFLEN];
    int i, j;

    debug(F111,"tilde_expand",dirname,dirname[0]);

    if (dirname[0] != '~')		/* Not a tilde...return param */
      return(dirname);
    if (!strcmp(olddir,dirname)) {	/* Same as last time */
      return(oldrealdir);		/* so return old answer. */
    } else {
	j = (int)strlen(dirname);
	for (i = 0; i < j; i++)		/* find username part of string */
	  if (!ISDIRSEP(dirname[i]))
	    temp[i] = dirname[i];
	  else break;
	temp[i] = '\0';			/* tie off with a NULL */
	if (i == 1) {			/* if just a "~" */
	    user = getpwnam(whoami());	/*  get info on current user */
	} else {
	    user = getpwnam(&temp[1]);	/* otherwise on the specified user */
	}
    }
    if (user != NULL) {			/* valid user? */
	strcpy(olddir, dirname);	/* remember the directory */
	strcpy(oldrealdir,user->pw_dir); /* and their home directory */
	strcat(oldrealdir,&dirname[i]);
	return(oldrealdir);
    } else {				/* invalid? */
	strcpy(olddir, dirname);	/* remember for next time */
	strcpy(oldrealdir, dirname);
	return(oldrealdir);
    }
#else
    return(NULL);
#endif /* DTILDE */
}

/*
  Functions for executing system commands.
  zsyscmd() executes the system command in the normal, default way for
  the system.  In UNIX, it does what system() does.  Thus, its results
  are always predictable.
  zshcmd() executes the command using the user's preferred shell.
*/
int
zsyscmd(s) char *s; {
#ifdef aegis
    if (!priv_chk()) return(system(s));
#else
#ifdef OS2
    if (!priv_chk()) return(system(s));
#else
    PID_T shpid;
#ifdef COMMENT
/* This doesn't work... */
    WAIT_T status;
#else
    int status;
#endif /* COMMENT */

    if (shpid = fork()) {
	if (shpid < (PID_T)0) return(-1); /* Parent */
	while (shpid != (PID_T) wait(&status))
	  ;
	return(status);
    }
    if (priv_can()) {			/* Child: cancel any priv's */
	printf("?Privilege cancellation failure\n");
	_exit(255);
    }
#ifdef HPUX10
    execl("/usr/bin/sh","sh","-c",s,NULL);
    perror("/usr/bin/sh");
#else
    execl("/bin/sh","sh","-c",s,NULL);
    perror("/bin/sh");
#endif /* HPUX10 */
    _exit(255);
    return(0);				/* Shut up ANSI compilers. */
#endif /* OS2 */
#endif /* aegis */
}

/*
  UNIX code by H. Fischer; copyright rights assigned to Columbia Univ.
  Adapted to use getpwuid to find login shell because many systems do not
  have SHELL in environment, and to use direct calling of shell rather
  than intermediate system() call. -- H. Fischer
  Call with s pointing to command to execute.
*/

int
zshcmd(s) char *s; {
    PID_T pid;

#ifdef OS2
    int rc ;
    char *shell = getenv("COMSPEC");
    if (!priv_chk()) {
#ifdef __32BIT__
      DosRequestMutexSem( hmuxKeyboard, SEM_INDEFINITE_WAIT ) ;
      /* DosSuspendThread(KbdHandlerThreadID) ; */
#endif /* __32BIT__ */
       if (*s == '\0')
#ifndef COMMENT
         rc = system(shell) == 0 ;
#else /* COMMENT */
#ifdef __32BIT__
         rc = _spawnlp(P_WAIT, shell, NULL) == 0;
#else
         rc = spawnl(P_WAIT, shell, NULL) == 0;
#endif /* __32BIT__ */
#endif /* COMMENT */
      else
       rc = system(s) == 0;
#ifdef __32BIT__
      /* DosResumeThread(KbdHandlerThreadID) ; */
      DosReleaseMutexSem( hmuxKeyboard ) ;
#endif /* __32BIT__ */
      }
#else
#ifdef AMIGA
    if (!priv_chk()) system(s);
#else
#ifdef datageneral
    if (priv_chk) return(1);
    if (*s == '\0')			/* Interactive shell requested? */
#ifdef mvux
	system("/bin/sh ");
#else
        system("x :cli prefix Kermit_Baby:");
#endif /* mvux */
    else				/* Otherwise, */
        system(s);			/* Best for aos/vs?? */

#else
#ifdef aegis
    if ((pid = vfork()) == 0) {		/* Make child quickly */
	char *shpath, *shname, *shptr;	/* For finding desired shell */

	if (priv_can()) exit(1);	/* Turn off privs. */
        if ((shpath = getenv("SHELL")) == NULL) shpath = "/com/sh";

#else					/* All Unix systems */
    if ((pid = fork()) == 0) {		/* Make child */
	char *shpath, *shname, *shptr;	/* For finding desired shell */
	struct passwd *p;
#ifdef HPUX10				/* Default */
	char *defshell = "/usr/bin/sh";
#else
	char *defshell = "/bin/sh";
#endif /* HPUX10 */
	if (priv_can()) exit(1);	/* Turn off privs. */
#ifdef COMMENT
/* Old way always used /etc/passwd shell */
	p = getpwuid(real_uid());	/* Get login data */
	if (p == (struct passwd *) NULL || !*(p->pw_shell))
	  shpath = defshell;
	else
	  shpath = p->pw_shell;
#else
/* New way lets user override with SHELL variable, but does not rely on it. */
/* This allows user to specify a different shell. */
	shpath = getenv("SHELL");	/* What shell? */
	if (shpath == NULL) {
	    p = getpwuid( real_uid() );	/* Get login data */
	    if (p == (struct passwd *)NULL || !*(p->pw_shell))
	      shpath = defshell;
	    else shpath = p->pw_shell;
        }
#endif /* COMMENT */
#endif /* aegis */
	shptr = shname = shpath;
	while (*shptr != '\0')
	  if (*shptr++ == DIRSEP)
	    shname = shptr;
	if (s == NULL || *s == '\0') {	/* Interactive shell requested? */
	    execl(shpath,shname,"-i",NULL); /* Yes, do that */
	} else {			/* Otherwise, */
	    execl(shpath,shname,"-c",s,NULL); /* exec the given command */
	}				/* If execl() failed, */
	exit(BAD_EXIT);			/* return bad return code. */

    } else {				/* Parent */

    	int wstat;			/* ... must wait for child */
	SIGTYP (*istat)(), (*qstat)();

	if (pid == (PID_T) -1) return(0); /* fork() failed? */

	istat = signal(SIGINT,SIG_IGN);	/* Let the fork handle keyboard */
	qstat = signal(SIGQUIT,SIG_IGN); /* interrupts itself... */

    	while (((wstat = wait((WAIT_T *)0)) != pid) && (wstat != -1))
	  ;				/* Wait for fork */
	signal(SIGINT,istat);		/* Restore interrupts */
	signal(SIGQUIT,qstat);
    }
#endif
#endif
#endif
    return(1);
}

#ifdef aegis
/*
 Replacement for strchr() and index(), neither of which seem to be universal.
*/
static char *
#ifdef CK_ANSIC
xindex(char * s, char c)
#else
xindex(s,c) char *s, c;
#endif /* CK_ANSIC */
/* xindex */ {
    while (*s != '\0' && *s != c) s++;
    if (*s == c) return(s); else return(NULL);
}
#endif /* aegis */

/*  I S W I L D  --  Check if filespec is "wild"  */

/*
  Returns 0 if it is a single file, 1 if it contains wildcard characters.
  Note: must match the algorithm used by match(), hence no [a-z], etc.
*/
int
iswild(filespec) char *filespec; {
    char c; int x; char *p;
    if (wildxpand) {
	if ((x = zxpand(filespec)) > 1) return(1);
	if (x == 0) return(0);		/* File does not exist */
	p = malloc(MAXNAMLEN + 20);
	znext(p);
#ifdef OS2
	x = (stricmp(filespec,p) != 0);
#else
	x = (strcmp(filespec,p) != 0);
#endif /* OS2 */
	free(p);
	return(x);
    } else {
	while ((c = *filespec++) != '\0')
	  if (c == '*' || c == '?') return(1);
	return(0);
    }
}

/*
   Tell if string pointer s is the name of an existing directory.
   Returns 1 if directory, 0 if not a directory.
*/
int
isdir(s) char *s; {
    int x;
    struct stat statbuf;

    if (!s) return(0);
    if (!*s) return(0);

#ifdef OS2
    /* Disk letter like A: is top-level directory on a disk */
    if (((int)strlen(s) == 2) && (isalpha(*s)) && (*(s+1) == ':'))
      return(1);
#endif /* OS2 */
    x = stat(s,&statbuf);
    debug(F111,"isdir stat",s,x);
    if (x == -1) {
	debug(F101,"isdir errno","",errno);
	return(0);
    } else {
	debug(F101,"isdir statbuf.st_mode","",statbuf.st_mode);
	return( S_ISDIR (statbuf.st_mode) ? 1 : 0 );
    }
}

#ifdef CK_MKDIR
/* Some systems don't have mkdir(), e.g. Tandy Xenix 3.2.. */

/* Z M K D I R  --  Create directory(s) if necessary */
/*
   Call with:
    A pointer to a file specification that might contain directory
    information.  The filename is expected to be included.
    If the file specification does not include any directory separators,
    then it is assumed to be a plain file.
    If one or more directories are included in the file specification,
    this routine tries to create them if they don't already exist.
   Returns:
    0 on success, i.e. the directory was created
   -1 on failure to create the directory
*/
int
zmkdir(path) char *path; {
    char *xp, *tp, c;
    int x;

    x = strlen(path);
    debug(F111,"zmkdir",path,x);
    if (x < 1 || x > MAXPATH)		/* Check length */
      return(-1);
    if (!(tp = malloc(x+1)))		/* Make a temporary copy */
      return(-1);
    strcpy(tp,path);
#ifdef DTILDE
    if (*tp == '~') {			/* Starts with tilde? */
	xp = tilde_expand(tp);		/* Attempt to expand tilde */
	if (*xp) {
	    char *zp;
	    debug(F110,"zmkdir tilde_expand",xp,0);
	    if (!(zp = malloc(strlen(xp) + 1))) { /* Make a place for it */
		free(tp);
		return(-1);
	    }
	    free(tp);			/* Free previous buffer */
	    tp = zp;			/* Point to new one */
	    strcpy(tp,xp);		/* Copy expanded name to new buffer */
	}
	debug(F110,"zmkdir tp after tilde_expansion",tp,0);
    }
#endif /* DTILDE */
    xp = tp;
    if (ISDIRSEP(*xp))			/* Don't create root directory! */
      xp++;

    /* Go thru filespec from left to right... */

    for (; *xp; xp++) {			/* Create parts that don't exist */
	if (!ISDIRSEP(*xp))		/* Find next directory separator */
	  continue;
	c = *xp;			/* Got one. */
	*xp = NUL;			/* Make this the end of the string. */
	if (!isdir(tp)) {		/* This directory exists already? */
	    debug(F110,"zmkdir making",tp,0);
	    x =				/* No, try to create it */
#ifdef NOMKDIR
	       -1			/* Systems without mkdir() */
#else
#ifdef OS2				/* OS/2 */
#ifdef __32BIT__
	      _mkdir(tp)		/* The IBM way */
#else
	       mkdir(tp)		/* The Microsoft way */
#endif /* __32BIT__ */
#else
	       mkdir(tp,0777)		/* UNIX */
#endif /* OS2 */
#endif /* NOMKDIR */
		 ;
	    if (x < 0) {
		debug(F101,"zmkdir failed, errno","",errno);
		free(tp);		/* Free temporary buffer. */
		return(-1);		/* Freturn failure code. */
	    }
	}
	*xp = c;			/* Replace the separator. */
    }
    free(tp);				/* Free temporary buffer. */
    return(0);				/* Return success code. */
}
#endif /* CK_MKDIR */

/* Z F S E E K  --  Position input file pointer */
/*
   Call with:
    Long int, 0-based, indicating desired position.
   Returns:
    0 on success.
   -1 on failure.
*/
#ifndef NORESEND
int
#ifdef CK_ANSIC
zfseek(long pos)
#else
zfseek(pos) long pos;
#endif /* CK_ANSIC */
/* zfseek */ {
    debug(F101,"zfseek","",pos);
    return(fseek(fp[ZIFILE], pos, 0));
}
#endif /* NORESEND */

#ifdef OS2

/*  Z C H D S K  --  Change currently selected disk device */

/* Returns -1 if error, otherwise 0 */

zchdsk(c) int c; {
    int i = toupper(c) - 64;
    return( _chdrive(i));
}

#undef stat
#ifdef __IBMC__STAT
#define stat(p, s) _stat(p, s)
#endif /* __IBMC__STAT */

os2stat(char *path, struct stat *st) {
    char local[MAXPATH];
    int len;

    strcpy(local, path);
    len = strlen(local);

    if ( len == 2 && local[1] == ':' )
        local[2] = DIRSEP, local[3] = 0; /* if drive only, append / */
    else if ( len == 0 )
        local[0] = DIRSEP, local[1] = 0; /* if empty path, take / instead */
    else if ( len > 1 && ISDIRSEP(local[len - 1]) && local[len - 2] != ':' )
        local[len - 1] = 0; /* strip trailing / except after d: */

    return stat(local, st);
}

#ifdef CK_LABELED
/* O S 2 S E T L O N G N A M E -- Set .LONGNAME Extended Attribute */

/* Returns -1 if error, otherwise 0 */

int
os2setlongname( char * fn, char * ln ) {
   EAOP2         FileInfoBuf;
   ULONG         FileInfoSize;
   ULONG         PathInfoFlags;
   APIRET        rc ;
   char          FEA2List[1024];
   FEA2 *        pfea2;
   WORD *        pEAdata;

   debug(F110,"os2setlongname: filename is",fn,0);
   debug(F110,"os2setlongname: longname is",ln,0);

   FileInfoSize = sizeof( EAOP2 ) ;
   PathInfoFlags = DSPI_WRTTHRU ; /* complete write operation before return */

   FileInfoBuf.fpGEA2List = 0 ;
   FileInfoBuf.fpFEA2List = (PFEA2LIST) &FEA2List;
   pfea2 = FileInfoBuf.fpFEA2List->list;

   pfea2->oNextEntryOffset = 0 ;
   pfea2->fEA = 0 ;
   pfea2->cbName = 9 ; /* length of ".LONGNAME" without \0 */
   pfea2->cbValue = strlen( ln ) + 2 * sizeof( WORD ) ;
   strcpy( pfea2->szName, ".LONGNAME" ) ;

   pEAdata = (WORD *) pfea2->szName + 10/sizeof(WORD) ;
   *pEAdata = EAT_ASCII ;
   pEAdata++ ;
   *pEAdata = strlen( ln ) ;
   pEAdata++ ;
   strcpy( (char *) pEAdata, ln ) ;
   pEAdata += (strlen( ln )+1)/sizeof(WORD) ;

   FileInfoBuf.fpFEA2List->cbList = (char *) pEAdata -
                                    (char *) FileInfoBuf.fpFEA2List ;
     
   rc = DosSetPathInfo( fn, 2, &FileInfoBuf, FileInfoSize, PathInfoFlags ) ;
   debug(F101,"os2setlongname: rc=","",rc);
   if ( !rc )
      return 0 ;
   else
      return -1 ;
}

/* O S 2 G E T L O N G N A M E -- Get .LONGNAME Extended Attribute */

/* Returns -1 if error, otherwise 0 */

int
os2getlongname( char * fn, char ** ln ) {
   static char   * LongNameBuf = 0 ;
   EAOP2         FileInfoBuf;
   ULONG         FileInfoSize;
   ULONG         PathInfoFlags;
   APIRET        rc ;
   char          FEA2List[1024];
   FEA2 *        pfea2;
   char          GEA2List[1024];
   GEA2 *        pgea2;
   WORD *        pEAdata;
   WORD          LongNameLength ;

   *ln = 0 ;
   if ( !LongNameBuf )
      LongNameBuf = strdup( "Initialization of LongNameBuf" );
   debug(F110,"os2getlongname: filename is",fn,0);

   FileInfoSize = sizeof( EAOP2 ) ;
   PathInfoFlags = DSPI_WRTTHRU ; /* Complete write operation before return */

   FileInfoBuf.fpGEA2List = (PGEA2LIST) &GEA2List;
   FileInfoBuf.fpFEA2List = (PFEA2LIST) &FEA2List;
   pgea2 = FileInfoBuf.fpGEA2List->list;
   pfea2 = FileInfoBuf.fpFEA2List->list;

   pfea2->oNextEntryOffset = 0 ;
   pfea2->fEA = 0 ;
   pfea2->cbName = 9 ;			/* Length of ".LONGNAME" without \0 */
   pfea2->cbValue = MAXPATH ;
   strcpy( pfea2->szName, ".LONGNAME" ) ;

   FileInfoBuf.fpGEA2List->cbList = sizeof(GEA2LIST)
                                  + pgea2->cbName + 1 ;
   pgea2->oNextEntryOffset = 0;
   pgea2->cbName = pfea2->cbName ;
   strcpy(pgea2->szName,pfea2->szName);

   FileInfoBuf.fpFEA2List->cbList = 1024 ;

   rc = DosQueryPathInfo(fn,FIL_QUERYEASFROMLIST,&FileInfoBuf,FileInfoSize );
   LongNameLength =
     *(WORD *)((char *)pfea2 + sizeof(FEA2) + pfea2->cbName + sizeof(WORD)) ;
   debug(F101,"os2getlongname: rc=","",rc);
   debug(F101,"   cbValue:","",pfea2->cbValue);
   debug(F101,"   cbName:","",pfea2->cbName);
   debug(F101,"   EA Value Length:","",LongNameLength );
   debug(F110,"   EA Value:",(char *)pfea2 + sizeof(FEA2)
                             + pfea2->cbName + (2 * sizeof(WORD)),0 ) ;
   if ( rc ) {
       return -1 ;
   } else if ( pfea2->cbValue ) {
       if ( LongNameBuf )
         free( LongNameBuf ) ;
       LongNameBuf = (char *) malloc( LongNameLength + 1 ) ;
       if ( LongNameBuf ) {
         strncpy( LongNameBuf, (char *)pfea2 + sizeof(FEA2)
                             + pfea2->cbName + (2 * sizeof(WORD)),
                              LongNameLength );
         LongNameBuf[LongNameLength] = '\0' ;
         debug(F110,"os2getlongname: longname is",LongNameBuf,0);
         }
       else
         debug(F100,"os2getlongname: malloc failed","",0);
   } else {
       if ( LongNameBuf )
          free( LongNameBuf ) ;
       LongNameBuf = strdup( "" ) ;
       debug(F110,"os2getlongname: there is no longname attribute",
                  LongNameBuf,0);
   }

   *ln = LongNameBuf ;
   return 0 ;
}


_PROTOTYP( VOID GetMem, (PVOID *, ULONG) ) ;

/* O S 2 G E T E A S - Get all OS/2 Extended Attributes */

/* Returns 0 on success, -1 on failure */

int
os2geteas( char * name ) {
    CHAR *pAllocc=NULL; /* Holds the FEA struct returned by DosEnumAttribute */
                        /*  used to create the GEA2LIST for DosQueryPathInfo */

    ULONG ulEntryNum = 1; /* count of current EA to read (1-relative)        */
    ULONG ulEnumCnt;      /* Number of EAs for Enum to return, always 1      */

    ULONG FEAListSize = sizeof(ULONG) ;/* starting size of buffer necessary to 
                                                        hold all FEA blocks */
    ULONG GEAListSize = MAXEACOUNT * sizeof(GEA2) + sizeof(ULONG) ;

    FEA2 *pFEA;           /* Used to read from Enum's return buffer          */
    GEA2 *pGEA, *pLastGEAIn; /* Used to write to pGEAList buffer             */
    GEA2LIST *pGEAList;/*Ptr used to set up buffer for DosQueryPathInfo call */
    EAOP2  eaopGet;       /* Used to call DosQueryPathInfo                   */
    APIRET rc ;
    int offset ;

                                /* Allocate enough room for any GEA List */
    GetMem((PPVOID)&pAllocc, MAX_GEA);
    pFEA = (FEA2 *) pAllocc;               /* pFEA always uses pAlloc buffer */

    GetMem((PPVOID)&pGEAList, GEAListSize ) ; /* alloc buffer for GEA2 list */
    pGEAList->cbList = GEAListSize ;
    pGEA = pGEAList->list ;
    pLastGEAIn = 0 ;

    if ( !pAllocc || ! pGEAList ) {
	FreeMem( pAllocc ) ;
	FreeMem( pGEAList ) ;
	return -1 ;
    }
    if ( pFEAList ) {
	FreeMem( pFEAList ) ;
	pFEAList = 0 ;
   }
   while(TRUE) {              /* Loop continues until there are no more EAs */
       ulEnumCnt = 1;                   /* Only want to get one EA at a time */
       if(DosEnumAttribute(Ref_ASCIIZ,            /* Read into pAlloc Buffer */
			   name,               /* Note that this does not */
			   ulEntryNum,            /* get the aValue field,   */
			   pAllocc,               /* so DosQueryPathInfo must*/
			   MAX_GEA,               /* be called to get it.    */
			   &ulEnumCnt,
			   (LONG) GetInfoLevel1)) {
                              /* There was some sort of error */
	   FreeMem( pGEAList ) ;
	   FreeMem(pAllocc);
	   return (-1);
       }
       if(ulEnumCnt != 1)		/* All the EAs have been read */
         break;

       ulEntryNum++;
       FEAListSize += sizeof(FEA2LIST) + pFEA->cbName+1 +
	              pFEA->cbValue + 4 ;

       if (pLastGEAIn)
         pLastGEAIn->oNextEntryOffset = (BYTE *)pGEA - (BYTE *)pLastGEAIn ;
       pLastGEAIn = pGEA ;

       pGEA->oNextEntryOffset = 0L;
       pGEA->cbName = pFEA->cbName;
       strcpy(pGEA->szName, pFEA->szName);
      
       /* must align GEA2 blocks on double word boundaries */
       offset = sizeof(GEA2) + pGEA->cbName + 1 ;
       offset += ( offset % 4 ? (4 - offset % 4) : 0 ) ;
       pGEA = (GEA2 *) ((BYTE *) pGEA + offset) ;
   }
   debug(F111,"os2geteas: EA count",name,ulEntryNum-1);
   GetMem( (PPVOID) &pFEAList, FEAListSize ) ;
   pFEAList->cbList = FEAListSize ;

   eaopGet.fpGEA2List = pGEAList;
   eaopGet.fpFEA2List = pFEAList;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel3,
                   (PVOID) &eaopGet,
                    sizeof(EAOP2));

   debug(F111,"os2geteas: DosQueryPathInfo",name,rc) ;
   FreeMem( pGEAList ) ;
   FreeMem( pAllocc ) ;
   return ( rc ? -1 : 0 ) ;
}

/* O S 2 S E T E A S - Set all OS/2 Extended Attributes */

/* Returns 0 on success, -1 on failure                  */

int
os2seteas( char * name ){
   EAOP2  eaopSet;       /* Used to call DosSetPathInfo */
   APIRET rc ;

   if ( !pFEAList ) {
     debug(F100,"os2seteas: EA List is empty","",0);
     return 0 ;
   }
   eaopSet.fpGEA2List = 0 ;
   eaopSet.fpFEA2List = pFEAList;

   rc = DosSetPathInfo(name,		/* Set the EA info */
                    SetInfoLevel2,
                   (PVOID) &eaopSet,
                    sizeof(EAOP2),
                    DSPI_WRTTHRU);
   debug(F111,"os2seteas: DosSetPathInfo",name,rc) ;

   if ( !rc ) {
      FreeMem( pFEAList ) ;
      pFEAList = 0 ;
   }
   return ( rc ? -1 : 0 ) ;
}

/* O S 2 G E T A T T R - Get all OS/2 Normal Attributes */

/* Returns 0 on success, -1 on failure                  */

int 
os2getattr( char * name ){
   FILESTATUS3 FileInfoBuf ;
   APIRET rc ;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                    sizeof(FILESTATUS3));

   if ( !rc ) {
      os2attrs = FileInfoBuf.attrFile ;
      return 0 ;
      }
   else {
      os2attrs = FILE_NORMAL ;
      return -1 ;
      }
}

/* O S 2 S E T A T T R - Set all OS/2 Normal Attributes */

/* Returns 0 on success, -1 on failure                  */

int 
os2setattr( char * name ){
   FILESTATUS3 FileInfoBuf ;
   APIRET rc ;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                    sizeof(FILESTATUS3));

   if ( !rc ) {
      FileInfoBuf.attrFile = lf_opts & os2attrs ;
      rc = DosSetPathInfo( name,
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                   sizeof(FILESTATUS3),
                   0);
      if ( !rc )
         return 0 ;
      }
   return -1 ;
}

/****************************************************************\
 *                                                              *
 *  Name:    GetMem (ppv, cb)                                   *
 *                                                              *
 *  Purpose: This routine returns a pointer to a available*     *
 *           memory space.                                      *
 *                                                              *
 *  Usage  :                                                    *
 *  Method : Routine should be bullet proof as it does its own  *
 *           error checking. It assumes that hwnd points to the *
 *           correct window with the name listbox in it.        *
 *                                                              *
 *  Returns: The current EA as determined by querying the l-box *
 *           selector.                                          *
 *                                                              *
\****************************************************************/
VOID GetMem (PVOID *ppv, ULONG cb) {
    BOOL        f;

    f =(BOOL) DosAllocMem(ppv, cb, fPERM|PAG_COMMIT);
    if (f) {
        *ppv = NULL;
        return;
    }
    return;
}

extern CHAR os2version[50] ;
extern long vernum ;

/* D O _ L A B E L _ S E N D - Generate Label Header from EA List     */

/* Return 0 on success, -1 on failure                                 */

int
do_label_send(char * name) {
    char scratch[100] ;
    long buffreespace = INBUFSIZE ;
    long easleft = 0 ;
    long memtocpy = 0 ;
    static BYTE * p = 0 ;

    debug(F110,"do_label_send",name,0) ;
    if ( !pFEAList ){
    debug(F101,"   no EA list","",pFEAList) ;
      return 0 ;
    }

    if ( !p ) {
       debug(F100,"do_label_send: generate header","",0);
       zinptr += sprintf(zinptr,"KERMIT LABELED FILE:02UO04VERS");

       sprintf(scratch,"%d",strlen(get_os2_vers()));
       zinptr += sprintf(zinptr,"%02d%d%s",
          strlen(scratch),
			 strlen(get_os2_vers()), 
          get_os2_vers());

       sprintf(scratch,"%d",vernum);
       zinptr += sprintf(zinptr,"05KVERS02%02d%d", strlen(scratch), vernum);

       sprintf(scratch,"%d",strlen(name));
       zinptr += sprintf(zinptr,"08FILENAME%02d%d%s", strlen(scratch),
          strlen(name), name);

       zinptr += sprintf(zinptr,"04ATTR%02d",sizeof(ULONG));
       memcpy( zinptr, (BYTE *) &os2attrs, sizeof(ULONG) ) ;
       zinptr += sizeof(ULONG) ;

       sprintf( scratch, "%d", pFEAList->cbList ) ;
       zinptr += sprintf(zinptr,"09EABUFSIZE%02d%ld", strlen(scratch), 
                pFEAList->cbList);
       p = (BYTE *) pFEAList ;
       buffreespace -= (BYTE *) zinptr - (BYTE *) zinbuffer ;
    }

    easleft = pFEAList->cbList - ( (BYTE *) p - (BYTE *) pFEAList ) ;
    memtocpy = buffreespace > easleft ? easleft : buffreespace ;
    memcpy( zinptr, p, memtocpy ) ;
    zinptr = (BYTE *) zinptr + memtocpy ;
    p = (BYTE *) p + memtocpy ;
    buffreespace -= memtocpy ;

    if ( buffreespace > 0 ) {
       p = 0 ;
       FreeMem( pFEAList ) ;
       pFEAList = 0 ;
       debug(F100,"do_label_send: terminate header","",0);
    }
    zincnt = (zinptr - zinbuffer);		/* Size of this beast */
    return(0);
}

/* D O _ L A B E L _ R E C V - Receive label info and create EA List  */

/* Return 0 on success, -1 on failure */

int
do_label_recv() {
    char *recv_ptr = zoutbuffer;
    int lblen ;
    char buffer[16];
    size_t memtocpy, lefttocpy ;
    static BYTE * pFEA = 0 ;

    if ( !pFEAList 
         && strncmp(zoutbuffer,"KERMIT LABELED FILE:02UO04VERS",30) != 0) {
	debug(F100,"do_label_recv: not a labeled file","",0);
	return(0);			/* Just continue if unlabeled */
    }

    if ( !pFEAList ) {
       recv_ptr += 30;			/* start at front of buffer */
       zoutcnt  -= 30 ;

       /* get length of length of OS/2 version */
       memcpy(buffer, recv_ptr, 2) ;
       recv_ptr += 2 ;
       zoutcnt  -= 2 ;
       buffer[2] = '\0' ;
       lblen = atoi(buffer);

       /* get length of OS/2 version */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       lblen = atoi(buffer);

       /* get os2 version */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       debug(F111,"  file created under OS/2: ",buffer,lblen);

       /* check sync with Kermit Version */
       memcpy(buffer, recv_ptr, 7);
       recv_ptr += 7;
       zoutcnt  -= 7;
       if (strncmp(buffer, "05KVERS", 7) != 0) {
	   debug(F111,"  lost sync at KVERS",recv_ptr-7,zoutcnt+7);
	   return(-1);
       }

       /* get length of length of C-Kermit version */
       memcpy(buffer, recv_ptr, 2) ;
       recv_ptr += 2 ;
       zoutcnt  -= 2 ;
       buffer[2] = '\0' ;
       lblen = atoi(buffer);

       /* get length of C-Kermit version */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       lblen = atoi(buffer);

       /* get C-Kermit version */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       debug(F111,"  file created with OS/2 C-Kermit: ",buffer,lblen);

       /* check sync with FILENAME */
       memcpy(buffer, recv_ptr, 10);
       recv_ptr += 10;
       zoutcnt  -= 10;
       if (strncmp(buffer, "08FILENAME", 10) != 0) {
		debug(F111,"  lost sync at FILENAME",recv_ptr-10,zoutcnt+10);
		return(-1);
       }

       /* get length of length of Filename */
       memcpy(buffer, recv_ptr, 2) ;
       recv_ptr += 2 ;
       zoutcnt  -= 2 ;
       buffer[2] = '\0' ;
       lblen = atoi(buffer);

       /* get length of File Name */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       lblen = atoi(buffer);

       /* get File Name */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       debug(F111,"  file sent with name: ",buffer,lblen);

       /* check sync with ATTR */
       memcpy(buffer, recv_ptr, 6);
       recv_ptr += 6;
       zoutcnt  -= 6;
       if (strncmp(buffer, "04ATTR", 6) != 0) {
	   debug(F111,"  lost sync at ATTR",recv_ptr-6,zoutcnt+6);
	   return(-1);
       }

       /* get length of attributes - should be sizeof(ULONG) */
       memcpy(buffer, recv_ptr, 2) ;
       recv_ptr += 2 ;
       zoutcnt  -= 2 ;
       buffer[2] = '\0' ;
       lblen = atoi(buffer);
       if ( lblen != sizeof(ULONG) ) {
	   debug(F101,"   Attributes have wrong length","",lblen);
	   return(-1);
       }

       /* get attributes */
       memcpy(&os2attrs, recv_ptr, sizeof(ULONG)) ;
       recv_ptr += sizeof(ULONG) ;
       zoutcnt  -= sizeof(ULONG) ;

       /* check sync with EABUFSIZE */
       memcpy(buffer, recv_ptr, 11);
       recv_ptr += 11;
       zoutcnt  -= 11;
       if (strncmp(buffer, "09EABUFSIZE", 11) != 0) {
	   debug(F111,"  lost sync at EABUFSIZE",recv_ptr-11,zoutcnt+11);
	   return(-1);
       }

       /* get length of length of EA Buffer Size */
       memcpy(buffer, recv_ptr, 2) ;
       recv_ptr += 2 ;
       zoutcnt  -= 2 ;
       buffer[2] = '\0' ;
       lblen = atoi(buffer);

       /* get length of EA Buffer Size */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       lblen = atoi(buffer);
       debug(F101,"  EA Buffer Size:","",lblen) ;

       GetMem( (PPVOID) &pFEAList, (ULONG) lblen ) ;
       if ( !pFEAList ) {
         debug(F101,"   pFEAList","",pFEAList) ;
         return -1 ;
       }
       pFEAList->cbList = lblen ;
       pFEA = (BYTE *) pFEAList ;
    }

    if ( pFEAList && pFEA ) {
       /* get EA Buffer */
       lefttocpy = pFEAList->cbList - ( (BYTE *) pFEA - (BYTE *) pFEAList ) ;
       memtocpy = ( zoutcnt < lefttocpy ? zoutcnt : lefttocpy ) ;
       memcpy(pFEA, recv_ptr, memtocpy);
       recv_ptr += memtocpy;
       zoutcnt  -= memtocpy;
       pFEA += memtocpy ;
       debug(F101,"   memtocpy","",memtocpy);
       debug(F101,"   zoutcnt","",zoutcnt);

       if ( pFEA == ( (BYTE *) pFEAList + pFEAList->cbList ) ) {
          pFEA = 0 ;  /* we are done copying the EA's to the EA List */
          debug(F100,"   done copying EA's","",0);
       }
    }

    if ( pFEA && (recv_ptr != zoutbuffer) ) {
       memmove(zoutbuffer, recv_ptr, zoutcnt);
       zoutptr = zoutbuffer + zoutcnt;
       return(1);				/* Go fill some more */
    }
    else return (0) ;
}
#endif /* CK_LABELED */
#endif /* OS2 */
