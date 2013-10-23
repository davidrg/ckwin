#ifdef NT
char *ckzv = "Win32 File support, 8.0.187, 18 July 2004";
#else /* NT */
char *ckzv = "OS/2 File support, 8.0.187, 18 July 2004";
#endif /* NT */

/* C K O F I O  --  Kermit file system support for OS/2 */


/*
  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in system-specific modules when you they
  are supported.
*/

/*
  Authors: Frank da Cruz (fdc@columbia.edu),
             Columbia University Academic Information Systems, New York City,
           Jeffrey E Altman <jaltman@secure-endpoints.com>
             Secure Endpoints Inc., New York City
  and others noted below.  Note: CUCCA = Previous name of Columbia University
  Academic Information Systems.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

/* Include Files */

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"
#ifndef NOCSETS
#include "ckcxla.h"
#endif /* NOCSETS */
#include "ckuusr.h"
#ifdef CK_LOGIN                         /* For AUTH_VALID, AUTHTYPE_xxx, ... */
#include "ckctel.h"
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */
#endif /* CK_LOGIN */
#include <stdio.h>
#include <errno.h>
_PROTOTYP( int os2settitle, (char *, int) );
extern int priority;
#include <signal.h>

/*

  C-Kermit's OS/2 support originally by Chris Adie <C.Adie@uk.ac.edinburgh>
  Edinburgh University Computing Service, Scotland, for C-Kermit 4F.  Adapted
  to C-Kermit 5A and integrated into the UNIX support module by Kai Uwe Rommel
  <rommel@informatik.tu-muenchen.de>, Muenchen, Germany, December 1991.

  And then split away from the Unix modules for C-Kermit 6.1 because the
  amount of disparate code was just too great.
*/

/*
  Directory Separator macros, to allow this module to work with both UNIX and
  OS/2: Because of ambiguity with the command line editor escape \ character,
  the directory separator is currently left as / for OS/2 too, because the
  OS/2 kernel also accepts / as directory separator.  But this is subject to
  change in future versions to conform to the normal OS/2 style.

  Win32 also allows the Directory Separator to be / when it is used for
  local disk operations.  However, if a UNC is in use, then \ must be used.
  So we have two choices, we can return UNC names with \ quoted which might
  break if the command quoting is turned off, or we can performed the reverse
  translation in all file i/o functions if the first two characters are "//"
  indicating a UNC.
*/
#ifndef DIRSEP
#define DIRSEP       '/'
#endif /* DIRSEP */
#ifndef ISDIRSEP
#define ISDIRSEP(c)  ((c)=='/'||(c)=='\\')
#endif /* ISDIRSEP */

#ifdef SDIRENT
#define DIRENT
#endif /* SDIRENT */

#include "ckodir.h"

extern int binary;                      /* We need to know this for open() */
#ifdef CK_CTRLZ
extern int eofmethod;
#endif /* CK_CTRLZ */
extern int k95stdin,k95stdout;

#include <sys/utime.h>
#include <stdlib.h>
#include <process.h>
#include <share.h>
extern int fncact;                      /* Need this for zchkspa()         */
extern int tlevel, cmdlvl;              /* Need this for external commands */
#ifdef __IBMC__
extern FILE *popen(char *, char *);
extern int pclose(FILE *);
#else
#define popen    _popen
#define pclose   _pclose
#define fopen(n, m)  _fsopen(n, m, _SH_DENYWR)
#endif /* __IBMC__ */
#ifdef NT
#include <io.h>
#endif /* NT */

#define TIMESTAMP                       /* Can do file dates */
#include <time.h>                       /* Need this */
#include <sys/timeb.h>                  /* Need this too */
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __IBMC__
#ifdef system
#undef system
#endif
#ifdef COMMENT
#ifdef stat
#undef stat
#endif
#define stat _stat
#endif /* COMMENT */
#endif

#ifdef NT
#define timezone _timezone
#define write _write
#define fileno _fileno
#define stricmp _stricmp
#define setmode _setmode
#define access _access
#define unlink _unlink
#define chdir  _chdir
#define getcwd _getcwd
#define utime  _utime
#define rmdir  _rmdir
#define utimbuf _utimbuf
#define stat    _stat
#ifndef SEM_INDEFINITE_WAIT
#define SEM_INDEFINITE_WAIT INFINITE
#endif /* SEM_INDEFINITE_WAIT */
#endif /* NT */

/* Because standard stat has trouble with trailing /'s we have to wrap it */
int os2stat(char *, struct stat *);

/* Is `y' a leap year? */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

#ifdef DEBUG
extern int deblog;
#else
#define deblog 0
#endif /* DEBUG */
extern int xferlog;
extern char * xferfile;
int iklogopen = 0;
static time_t timenow;
static int logged_in = 0;
static char iksdmsg[CKMAXPATH+512];
extern char exedir[CKMAXPATH];
#ifndef XFERFILE
#define XFERFILE "iksd.log"
#endif /* XFERFILE */

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
   nzxpand(string,flags) -- Expands the given wildcard string into a list of files.
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
   zlink(source,destination) -- Link a file.
   zcopy(source,destination) -- Copy a file.
   zmkdir(path)       -- Create the directory path if possible
   zfnqfp(fname,len,fullpath) - Determine full path for file name.
*/

/* Kermit-specific includes */
/*
  Definitions here supersede those from system include files.
  ckcdeb.h is included above.
*/
#include "ckcker.h"                     /* Kermit definitions */
#include "ckucmd.h"                     /* For sys-dependent keyword tables */
#include "ckuver.h"                     /* Version herald */

char *ckzsys = HERALD;

#include <fcntl.h>

/* Define macros for getting file type */

#ifdef ISDIRBUG                         /* Also allow this from command line */
#ifdef S_ISREG
#undef S_ISREG
#endif /* S_ISREG */
#ifdef S_ISDIR
#undef S_ISDIR
#endif /*  S_ISDIR */
#endif /* ISDIRBUG */

#ifndef S_IFREG
#ifdef  _S_IFREG
#define S_IFREG _S_IFREG
#endif
#endif

#ifndef S_IFDIR
#ifdef  _S_IFDIR
#define S_IFDIR _S_IFDIR
#endif
#endif

#ifndef S_IFMT
#ifdef _S_IFMT
#define S_IFMT _S_IFMT
#endif
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* S_ISREG */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* S_ISDIR */

/* More macros we might need */
#ifndef _O_APPEND
#define _O_APPEND  O_APPEND
#endif
#ifndef _O_WRONLY
#define _O_WRONLY  O_WRONLY
#endif
#ifndef _O_CREAT
#define _O_CREAT   O_CREAT
#endif
#ifndef _O_TEXT
#define _O_TEXT    O_TEXT
#endif
#ifndef _SH_DENYNO
#define _SH_DENYNO SH_DENYNO
#endif
#ifndef _O_BINARY
#define _O_BINARY  O_BINARY
#endif

/* Longest pathname ... */
#ifdef MAXPATHLEN
#ifdef MAXPATH
#undef MAXPATH
#endif /* MAXPATH */
#define MAXPATH MAXPATHLEN
#else
#ifdef PATH_MAX
#define MAXPATH PATH_MAX
#else
#define MAXPATH 255
#endif /* PATH_MAX */
#endif /* MAXPATHLEN */

#ifndef NOPUSH
extern int nopush;
#endif /* NOPUSH */
_PROTOTYP( char * zdtstr, (time_t) );
_PROTOTYP( time_t zstrdt, (char *, int) );

/* Some systems define these symbols in include files, others don't... */

#ifndef R_OK
#define R_OK 4                          /* For access */
#endif /* R_OK */

#ifndef W_OK
#define W_OK 2
#endif /* W_OK */

#ifndef O_RDONLY
#define O_RDONLY 000
#endif /* O_RDONLY */

#ifdef CKROOT
static char ckroot[CKMAXPATH+1] = { NUL, NUL };
static int ckrootset = 0;
int ckrooterr = 0;
#endif /* CKROOT */

    /* syslog and wtmp items for Internet Kermit Service */
extern char * clienthost;               /* From ckcmai.c. */
extern int ckxlogging;
static char fullname[CKMAXPATH+1];
static char tmp2[CKMAXPATH+1];

#ifdef CK_LOGIN
int ckxanon = 0;                        /* Anonymous login not ok */
char * anonacct = NULL;
#ifdef NT
char * iks_domain = NULL;               /* Default domain to use for logins */
#endif /* NT */
#ifdef CK_PERMS
int ckxperms = 0040;
#endif /* CK_PERMS */
#ifdef UNIX
int ckxpriv = 1;                        /* Allow Root logins? */
#endif /* UNIX */
#endif /* CK_LOGIN */

#ifndef CKWTMP
int ckxwtmp = 0;
#else
int ckxwtmp = 1;
#include <utmp.h>
#define WTMPFILE "/usr/adm/wtmp"        /* I hope this is portable */
char * wtmpfile = NULL;

static int wtmpfd = 0;
static char cksysline[32] = { NUL, NUL };

VOID
logwtmp(line, name, host) char *line, *name, *host; {
    struct utmp ut;
    struct stat buf;
    time_t time();

    if (!ckxwtmp)
      return;

    if (!wtmpfile)
      makestr(&wtmpfile,WTMPFILE);

    if (!line) line = "";
    if (!name) name = "";
    if (!host) host = "";

    if (!wtmpfd && (wtmpfd = open(wtmpfile, O_WRONLY|O_APPEND, 0)) < 0) {
        ckxwtmp = 0;
        debug(F110,"WTMP open failed",line,0);
        return;
    }
    if (!fstat(wtmpfd, &buf)) {
        ckstrncpy(ut.ut_line, line, sizeof(ut.ut_line));
        ckstrncpy(ut.ut_name, name, sizeof(ut.ut_name));
        ckstrncpy(ut.ut_host, host, sizeof(ut.ut_host));
        time(&ut.ut_time);
        if (write(wtmpfd, (char *)&ut, sizeof(struct utmp)) !=
            sizeof(struct utmp)) {
            ftruncate(wtmpfd, buf.st_size); /* Error, undo any partial write */
            debug(F110,"WTMP write error",line,0);
        } else {
            debug(F110,"WTMP record OK",line,0);
            return;
        }
    }
}
#endif /* CKWTMP */

/* Declarations */

int mskrename = 0;                      /* MS-Kermit file collision renaming off */
int maxnam = MAXNAMLEN;                 /* Available to the outside */
int maxpath = MAXPATH;
int ck_znewn = -1;
int pexitstat = -2;                     /* Process exit status */

FILE *fp[ZNFILS] = {                    /* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/* Flags for each file indicating whether it was opened with popen() */
int ispipe[ZNFILS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Buffers and pointers used in buffered file input and output. */
#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer;
#else
extern char zinbuffer[], zoutbuffer[];
#endif /* DYNAMIC */
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt;

static long iflen = -1L;                /* Input file length */

static PID_T pid = 0;                   /* pid of child fork */
static int fcount[2] = {0,0};           /* Number of files left in search */
static int fcntsav[2] = {0,0};
static int fcountstream[2] = {0,0};
static int fcntstreamsav[2] = {0,0};
static char nambuf[MAXNAMLEN+4];        /* Buffer for a filename */
static char UNCnam[MAXNAMLEN+1];        /* Scratch buffer for UNC names */
#ifndef NOFRILLS
static char zmbuf[200];                 /* For mail, remote print strings */
#endif /* NOFRILLS */

#ifdef  OS2ONLY
#define INCL_KBD
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
typedef unsigned short WORD;
#undef COMMENT

/* Get/Set All Extended Attributes support */

#define FreeMem(p) DosFreeMem(p)

#define MAX_GEA            500L  /* Max size for a GEA List */
#define MAXEACOUNT          128  /* Max number of EA's supported */
#define Ref_ASCIIZ            1  /* Reference type for DosEnumAttribute */

/* Definition of level specifiers, required for File Info */

#define GetInfoLevel1         1            /* Get info from SFT */
#define GetInfoLevel2         2            /* Get size of FEAlist */
#define GetInfoLevel3         3            /* Get FEAlist given the GEAlist */
#define GetInfoLevel4         4            /* Get whole FEAlist */
#define GetInfoLevel5         5            /* Get FSDname */

#define SetInfoLevel1         1            /* Set info in SFT */
#define SetInfoLevel2         2            /* Set FEAlist */

FEA2LIST *pFEAList = 0;  /* Pointer to buffer containing all EA information */
ULONG os2attrs = FILE_NORMAL;
extern unsigned int lf_opts;
#endif /* OS2ONLY */

#ifdef NT
#include <windows.h>
#ifdef CK_LOGIN
#define SECURITY_WIN32
#include <security.h>
#include <ntsecapi.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#endif /* CK_LOGIN */
#endif /* NT */
char os2filename[MAXPATH];

#ifdef IKSD
extern int inserver, local;
#endif /* IKSD */

extern int server, en_mkd, en_cwd, en_del;

extern char uidbuf[];           /* from ckcmai.c... */
#ifdef CK_LOGIN
#ifdef CKROOT
extern char * anonroot;
#endif /* CKROOT */
extern int isguest;
#define GUESTPASS_LEN 256
static char guestpass[GUESTPASS_LEN] = "";
#endif /* CK_LOGIN */

_PROTOTYP(char * whoami, (void));

/*  Z K S E L F  --  Kill Self: log out own job, if possible.  */

int
zkself() {                              /* For "bye", but no guarantee! */
    _exit(3);
    return(0);
}

/* Z C H K P I D -- Check to see if a PID is valid */
/* returns 1 if pid is valid and active, 0 if not  */

int
zchkpid(unsigned long pid) {
#ifdef NT
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,0,pid);
    if ( hProcess ) {
        CloseHandle(hProcess);
        return(1);
    }
    return(0);
#else /* NT */
#endif /* NT */
    return(1);
}

/*  D O I K L O G  --  Open Kermit-specific ftp-like transfer log. */

VOID                                    /* called in ckcmai.c */
doiklog() {
    if (iklogopen)                      /* Already open? */
      return;
    debug(F111,"doiklog",xferfile,xferlog);
    if (xferlog) {                      /* Open iksd log if requested */
        if (!xferfile) {                /* If no pathname given */
          char buf[MAXPATH];
          ckstrncpy(buf,exedir,MAXPATH);
          ckstrncat(buf,XFERFILE,MAXPATH);
          makestr(&xferfile,buf);       /* use this default */
        }
        if (*xferfile) {
            xferlog = sopen(xferfile, _O_WRONLY | _O_APPEND | _O_CREAT | _O_TEXT,
                             _SH_DENYNO, 666 );
            debug(F111,"doiklog open()",xferfile,xferlog);
            if (xferlog < 0) {
#ifdef CKSYSLOG
                cksyslog(SYSLG_FC, 0, "xferlog open failure",xferfile,NULL);
#endif /* CKSYSLOG */
                debug(F101,"doiklog open errno","",errno);
                xferlog = 0;
            } else
              iklogopen = 1;
        } else
          xferlog = 0;
#ifdef CKSYSLOG
        if (ckxlogging) {
            if ( xferlog )
                cksyslog(SYSLG_FC, 1, "xferlog: open ok", xferfile, NULL);
            else
                cksyslog(SYSLG_FC, 0, "xferlog: open failed", xferfile, NULL);
        }
#endif /* CKSYSLOG */
    }
}

static VOID
getfullname(name) char * name; {
    char *p = (char *)fullname;
    int len = 0;
    fullname[0] = '\0';
    /* If necessary we could also chase down symlinks here... */
#ifdef COMMENT
    /* This works but is incompatible with wuftpd */
    if (isguest && anonroot) {
        ckstrncpy(fullname,anonroot,CKMAXPATH);
        len = strlen(fullname);
        if (len > 0)
          if (fullname[len-1] == '/')
            len--;
    }
    p += len;
#endif /* COMMENT */
    zfnqfp(name, CKMAXPATH - len, p);
    while (*p) {
        if (*p < '!') *p = '_';
        p++;
    }
}

/*  Z O P E N I  --  Open an existing file for input. */

/* Returns 1 on success, 0 on failure */

int
zopeni(n,name) int n; char *name; {
    int x, y;

    debug(F111,"zopeni name",name,n);
    debug(F101,"zopeni fp","", (unsigned) fp[n]);
    if (chkfn(n) != 0) {
        debug(F110,"zopeni chkfn()","file is open",0);
        return(0);
    }
    zincnt = 0;                         /* Reset input buffer */
    if (n == ZSYSFN) {                  /* Input from a system function? */
/*** Note, this function should not be called with ZSYSFN ***/
/*** Always call zxcmd() directly, and give it the real file number ***/
/*** you want to use.  ***/
        debug(F110,"zopeni called with ZSYSFN, failing!",name,0);
        *nambuf = '\0';                 /* No filename. */
        return(0);                      /* fail. */
#ifdef COMMENT
        return(zxcmd(n,name));          /* Try to fork the command */
#endif
    }
    if (n == ZSTDIO) {                  /* Standard input? */
        if (is_a_tty(0)) {
            fprintf(stderr,"Terminal input not allowed");
            debug(F110,"zopeni: attempts input from unredirected stdin","",0);
            return(0);
        }
        fp[ZIFILE] = stdin;
        ispipe[ZIFILE] = 0;
        setmode(fileno(stdin),_O_BINARY);
        return(1);
    }

#ifdef CKROOT
    debug(F111,"zopeni setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zopeni setroot violation",name,0);
            return(0);
        }
#endif /* CKROOT */

    if (n == ZIFILE || n == ZRFILE) {
        ckstrncpy( os2filename, name, MAXPATH ) ;
        errno = 0;
#ifdef NT
        fp[n] = _fsopen(name,"rb",_SH_DENYWR);          /* Binary mode */
        if (fp[n])
            _setmode(_fileno(fp[n]),_O_SEQUENTIAL);
        else {
            debug(F111,"zopeni ZI/ZR _fsopen failed","GetLastError",GetLastError());
        }
#else
        fp[n] = fopen(name,"rb");/* Binary mode */
#endif /* NT */
        ispipe[ZIFILE] = 0;
        if (fp[ZIFILE]) {
            x = ferror(fp[ZIFILE]);
#ifdef ZDEBUG
            printf("ZOPENI errno=%d\n",errno);
            printf("ZOPENI ferror=%d\n",x);
#endif /* ZDEBUG */
        }
#ifdef CK_LABELED
        if (binary == XYFT_L) {
            os2getattr(name);
            os2geteas(name);
        }
#endif /* CK_LABELED */
    } else {
#ifdef NT
        fp[n] = _fsopen(name,"rb",_SH_DENYWR); /* Real file, open it. */
        if (fp[n])
            _setmode(_fileno(fp[n]),_O_SEQUENTIAL);
        else {
            debug(F111,"zopeni _fsopen failed","GetLastError",GetLastError());
        }
#else
        fp[n] = fopen(name,"rb");/* Real file, open it. */
#endif /* NT */
    }
    debug(F111,"zopeni fopen", name, fp[n]);
#ifdef ZDEBUG
    printf("ZOPENI fp[%d]=%ld\n",n,fp[n]);
#endif /* ZDEBUG */
    ispipe[n] = 0;

    if (xferlog
#ifdef CKSYSLOG
        || ckxsyslog >= SYSLG_FA && ckxlogging
#endif /* CKSYSLOG */
        ) {
        getfullname(name);
        debug(F110,"zopeni fullname",fullname,0);
    }

    if (fp[n] == NULL) {
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FA && ckxlogging) {
            char buf[1024];
            sprintf(buf,"file[%d] %s: open failed",n,fullname);
            cksyslog(SYSLG_FA, 0, buf,NULL,NULL);
        }
        perror(fullname);
#else
        perror(name);
#endif /* CKSYSLOG */
        return(0);
    } else {
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FA && ckxlogging) {
          char buf[1024];
          sprintf(buf,"file[%d] %s: open read ok", n, fullname);
          cksyslog(SYSLG_FA, 1, buf, NULL, NULL);
        }
#endif /* CKSYSLOG */
        clearerr(fp[n]);
        return(1);
    }
}

/*  Z O P E N O  --  Open a new file for output.  */

int
zopeno(n,name,zz,fcb)
/* zopeno */  int n; char *name; struct zattr *zz; struct filinfo *fcb; {

    char p[8];                          /* (===OS2 change===) */
    int append = 0;

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
#endif /* DEBUG */

    if (chkfn(n) != 0)                  /* Already open? */
      return(0);                        /* Nothing to do. */

    if ((n == ZCTERM) || (n == ZSTDIO)) {   /* Terminal or standard output */
        fp[ZOFILE] = stdout;
        ispipe[ZOFILE] = 0;
#ifdef DEBUG
        if (n != ZDFILE)
          debug(F101,"zopeno fp[n]=stdout","",fp[n]);
#endif /* DEBUG */
        zoutcnt = 0;
        zoutptr = zoutbuffer;
        return(1);
    }

/* A real file.  Open it in desired mode (create or append). */
#ifdef CKROOT
    debug(F111,"zopeno setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zopeno setroot violation",name,0);
            return(0);
        }
#endif /* CKROOT */

    ckstrncpy(p,"w",8);                      /* Assume write/create mode */
    if (fcb) {                          /* If called with an FCB... */
        if (fcb->dsp == XYFZ_A) {       /* Does it say Append? */
            ckstrncpy(p,"a",8);              /* Yes. */
            debug(F100,"zopeno append","",0);
            append = 1;
        }
    }
    if (n == ZOFILE || n == ZSFILE) {   /* OS/2 binary mode */
        ckstrncpy( os2filename, name, MAXPATH ) ;
        ckstrncat(p,"b",8);
    }

    if (xferlog
#ifdef CKSYSLOG
        || ckxsyslog >= SYSLG_FC && ckxlogging
#endif /* CKSYSLOG */
        ) {
        getfullname(name);
        debug(F110,"zopeno fullname",fullname,0);
    }
    debug(F110,"zopeno fopen arg",p,0);

#ifdef NT_COMMENT
    /*
    There is a good reason I never did this before.  And that is
    because Windows 95 does not support Overlapped I/O for disk
    files nor does the C Run Time Library.
    */
    if ( !isWin95() )
    {
        SECURITY_ATTRIBUTES security ;
        HANDLE hFile = NULL;

        memset(&security, 0, sizeof(SECURITY_ATTRIBUTES));
        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.lpSecurityDescriptor = NULL ;
        security.bInheritHandle = FALSE ;

        hFile = CreateFile(name,
                            GENERIC_WRITE | GENERIC_READ,
                            FALSE, /* do not share */
                            &security,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL |
                            /* FILE_FLAG_OVERLAPPED |*/
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            fp[n] = NULL;
        }
        else {
            int fd = _open_osfhandle( (int) hFile,
                                      (n == ZOFILE || n == ZSFILE) ?
                                      _O_BINARY : _O_TEXT );
            if ( fd == -1 )
                fp[n] = NULL;
            else
                fp[n] = _fdopen(fd, p);
        }
    } else
#endif /* NT_COMMENT */
    {
        fp[n] = fopen(name,p);          /* Try to open the file */
    }
    ispipe[ZIFILE] = 0;

#ifdef ZDEBUG
    printf("ZOPENO fp[%d]=%ld\n",n,fp[n]);
#endif /* ZDEBUG */

    if (fp[n] == NULL) {                /* Failed */
        debug(F101,"zopeno failed errno","",errno);
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FC && ckxlogging) {
            char buf[1024];
            sprintf(buf, "file[%d] %s: %s failed (%m)",
                 n,
                 fullname,
                 append ? "append" : "create"
                 );
            cksyslog(SYSLG_FC, 0, buf, NULL, NULL);
        }
#endif /* CKSYSLOG */
    } else {                            /* Succeeded */
#ifdef NT
        if ( n == ZOFILE )
            _setmode(_fileno(fp[n]),_O_SEQUENTIAL);
#endif /* NT */
        if (n == ZDFILE ||              /* If it's the debug log */
            n == ZTFILE )               /* or the transaction log */
          setbuf(fp[n],NULL);           /* make it unbuffered */
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FC && ckxlogging) {
            char buf[1024];
            sprintf(buf, "file[%d] %s: %s ok",
                     n, fullname, append ? "append" : "create");
            cksyslog(SYSLG_FC, 1, buf, NULL, NULL);
        }
#endif /* CKSYSLOG */
        debug(F100, "zopeno ok", "", 0);
    }
    zoutcnt = 0;                        /* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */

int
zclose(n) int n; {
    int x, x2;
    extern long ffc;

    debug(F101,"zclose","",n);
    if (chkfn(n) < 1) return(0);        /* Check range of n */
    if ((n == ZOFILE) && (zoutcnt > 0)) /* (PWP) output leftovers */
      x2 = zoutdump();
    else
      x2 = 0;

    x = 0;                              /* Initialize return code */
    if (fp[ZSYSFN] || ispipe[n]) {      /* If file is really pipe */
        x = zclosf(n);                  /* do it specially */
        debug(F101,"zclose zclosf","",x);
        debug(F101,"zclose zclosf fp[n]","",fp[n]);
    } else {
        if ((fp[n] != stdout) && (fp[n] != stdin))
          x = fclose(fp[n]);
        fp[n] = NULL;
#ifdef CK_LABELED
        if (binary == XYFT_L) {
            debug(F111,"zclose LABELED","file number",n);
            if (n == ZOFILE) {
                debug(F111,"zclose LABELED",
                      "lf_opts && LBL_EXT",
                      lf_opts && LBL_EXT
                      );
                if (lf_opts && LBL_EXT)
                  os2seteas(os2filename);
                os2setattr(os2filename);
            } else if (n == ZIFILE && pFEAList) {
                FreeMem(pFEAList);
                pFEAList = 0;
            }
        }
#endif /* CK_LABELED */
    }
    iflen = -1L;                        /* Invalidate file length */
    if (x == EOF) {                     /* if we got a close error */
        debug(F101,"zclose fclose fails","",x);
        return(-1);
    } else if (x2 < 0) {                /* or error flushing last buffer */
        debug(F101,"zclose error flushing last buffer","",x2);
        return(-1);                     /* then return an error */
    } else {
        /* Print log record compatible with wu-ftpd */
        if (xferlog && (n == ZIFILE || n == ZOFILE)) {
            char * s, *p;
            extern char ttname[];
            if (!iklogopen) (VOID) doiklog(); /* Open log if necessary */
            debug(F101,"zclose iklogopen","",iklogopen);
            if (iklogopen) {
                int len;
                timenow = time(NULL);
#ifdef CK_LOGIN
                if (logged_in)
                  s = clienthost;
                else
#endif /* CK_LOGIN */
                  s = (char *)ttname;
                if (!s) s = "";
                if (!*s) s = "*";
#ifdef CK_LOGIN
                if (logged_in) {
                    p = guestpass;
                    if (!*p) p = "*";
                } else
#endif /* CK_LOGIN */
                  p = whoami();

                len = 24 + 12 + strlen(s) + 16
                    + strlen(fullname) + 1 + 1 + 1 + 1
                    + strlen(p) + 6 + 2 + 12;
                if ( len > sizeof(iksdmsg) )
                    sprintf(iksdmsg, /* SAFE */
                            "%.24s [BUFFER WOULD OVERFLOW]\n",
                             ctime(&timenow));
                else
                    sprintf(iksdmsg, /* SAFE */
                        "%.24s %d %s %ld %s %c %s %c %c %s %s %d %s\n",
                        ctime(&timenow),        /* date/time */
                        gtimer(),               /* elapsed secs */
                        s,                      /* peer name */
                        ffc,                    /* byte count */
                        fullname,               /* full pathname of file */
                        (binary ? 'b' : 'a'),   /* binary or ascii */
                        "_",                    /* options = none */
                        n == ZIFILE ? 'o' : 'i', /* in/out */
#ifdef CK_LOGIN
                        (isguest ? 'a' : 'r'),  /* User type */
#else
                        'r',
#endif /* CK_LOGIN */
                        p,                      /* Username or guest passwd */
#ifdef CK_LOGIN
                        logged_in ? "iks" : "kermit", /* Record ID */
#else
                        "kermit",
#endif /* CK_LOGIN */
                        0,              /* User ID on client system unknown */
                        "*"             /* Ditto */
                        );
                debug(F110,"zclose iksdmsg",iksdmsg,0);
                write(xferlog, iksdmsg, (int)strlen(iksdmsg));
            }
        }
        debug(F101,"zclose returns","",1);
        return(1);
    }
}

/*  Z C H I N  --  Get a character from the input file.  */

/*  Returns -1 if EOF, 0 otherwise with character returned in argument  */

int
zchin(n,c) int n; int *c; {
    int a;
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        a = coninc(0);
        if (a < 0)
            return(-1);
    } else
#endif /* IKSD */
    /* (PWP) Just in case this gets called when it shouldn't. */
    if (n == ZIFILE) {
        a = zminchar();
        if (a < 0)
            return(-1);
    } else {
        a = getc(fp[n]);
        if (a == EOF)
            return(-1);
    }
#ifdef CK_CTRLZ
    /* If SET FILE EOF CTRL-Z, first Ctrl-Z marks EOF */
    if (!binary && a == 0x1A && eofmethod == XYEOF_Z) {
        debug(F100,"zchin found Ctrl-Z","",0);
        return(-1);
    }
#endif /* CK_CTRLZ */
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
    int a, z = 0;                       /* z is return code. */
    int count = 0;
    extern CHAR feol;                   /* Line terminator */

    if (!s || chkfn(n) < 1)             /* Make sure file is open, etc */
        return(-1);
    s[0] = '\0';                        /* Don't return junk */

    a = -1;                             /* Current character, none yet. */
    while (x--) {                       /* Up to given length */
        int old=0;
        if (feol)                       /* Previous character */
             old = a;

        if (zchin(n,&a) < 0) {          /* Read a character from the file */
            debug(F101,"zsinl zchin fail","",count);
            if (count == 0)
                z = -1;                 /* EOF or other error */
            break;
        } else
            count++;

        if (feol) {                     /* Single-character line terminator */
            if (a == feol)
              break;
        } else {                        /* CRLF line terminator */
            if (a == '\015')            /* CR, get next character */
              continue;
            if (old == '\015') {        /* Previous character was CR */
                if (a == '\012')        /* This one is LF, so we have a line */
                  break;
                else                    /* Not LF, deposit CR */
                  *s++ = '\015';
            }
            if (a == '\012') break;     /* Here break on single LF too */
        }
        *s = a;                         /* Deposit character */
        s++;
    }
    *s = '\0';                          /* Terminate the string */
    return(z);
}

/*  Z X I N  --  Read x bytes from a file  */

/*
  Reads x bytes (or less) from channel n and writes them
  to the address provided by the caller.
  Returns number of bytes read on success, 0 on EOF or error.
*/
int
zxin(n,s,x) int n, x; char *s; {
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        int a, i;
        a = ttchk();
        for (i = 0 ;i < a && i < x; i++) {
            s[i] = coninc(0);
        }
        return(i);
    }
#endif /* IKSD */
    return(fread(s, sizeof (char), x, fp[n]));
}

int
ttwait(int fd, int secs) {
#ifdef NT
    DWORD  bytesAvail = 0;
    HANDLE hFile = (HANDLE)_get_osfhandle(fd);
    DWORD  type  = GetFileType(hFile);
    if (type != FILE_TYPE_PIPE)
        return(-1);

    do {
        if ( PeekNamedPipe(hFile,NULL,0,NULL,&bytesAvail,NULL) ) {
            if ( bytesAvail > 0 )
                return(1);
            else
                if ( secs-- > 0 )
                    sleep(1);
        }
        else {
            return(-1);
        }
    } while ( secs > 0 );

    return(0);
#else /* NT */
    return(-1);
#endif /* NT */
}

/*
  Z I N F I L L  --  Buffered file input.

  (re)fill the file input buffer with data.  All file input
  should go through this routine, usually by calling the zminchar()
  macro defined in ckcker.h.  Returns:

  Value 0..255 on success, the character that was read.
  -1 on end of file.
  -2 on any kind of error other than end of file.
  -3 timeout when reading from pipe (Kermit packet mode only).
*/
int
zinfill() {
    int x;
    extern int kactive;

    errno = 0;

#ifdef ZDEBUG
    printf("ZINFILL fp[%d]=%ld\n",ZIFILE,fp[ZIFILE]);
#endif /* ZDEBUG */

#ifdef CK_LABELED
    debug(F101,"zinfill: binary","",binary);
    debug(F101,"zinfill: pFEAList","",pFEAList);
    if ( binary == XYFT_L && pFEAList ) {
        zinptr = zinbuffer ;
        do_label_send(os2filename) ;
        if (feof(fp[ZIFILE]))
          return(-1);
        clearerr(fp[ZIFILE]);
        zincnt += fread(zinptr, sizeof (char), INBUFSIZE - zincnt, fp[ZIFILE]);
    } else {
#endif /* CK_LABELED */

#ifdef IKSD
        if (inserver && !local && fp[ZIFILE] == stdin) {
            int a, i;
            a = ttchk();
            for (i = 0 ;i < a && i < INBUFSIZE; i++) {
                zinbuffer[i] = coninc(0);
            }
            zincnt = i;
            zinptr = zinbuffer;    /* set pointer to beginning, (== &zinbuffer[0]) */
            if (zincnt == 0) return(-1);
            zincnt--;                           /* One less char in buffer */
            return((int)(*zinptr++) & 0377);    /* because we return the first */
        }
#endif /* IKSD */

        if (feof(fp[ZIFILE])) {
            debug(F100,"ZINFILL feof","",0);
#ifdef ZDEBUG
            printf("ZINFILL EOF\n");
#endif /* ZDEBUG */
            return(-1);
        }
        clearerr(fp[ZIFILE]);
        if (kactive && ispipe[ZIFILE]) {
            int secs, z = 0;
            secs = 1;
            debug(F101,"zinfill calling ttwait","",secs);
            if ((z = ttwait(fileno(fp[ZIFILE]),secs)) == 0) {
                debug(F101,"zinfill ttwait","",z);
                return(-3);
            }
        }

        zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
#ifdef CK_CTRLZ
        /* If SET FILE EOF CTRL-Z, first Ctrl-Z marks EOF */
        if (!binary && eofmethod == XYEOF_Z) {
            int i;
            for ( i=0 ; i<zincnt ; i++ ) {
                if (zinbuffer[i] == SUB) {
                    zincnt = i;         /* do not count Ctrl-Z */

                    if (i == 0) {
                        debug(F100,"ZINFILL Ctrl-Z first char in read","",0);
                        return(-1);
                    }
                    break;
                }
            }
        }
#endif /* CK_CTRLZ */
#ifdef DEBUG
        if (deblog)
            debug(F101,"ZINFILL fread","",zincnt);
#endif /* DEBUG */
#ifdef ZDEBUG
        printf("FREAD=%d\n",zincnt);
#endif /* ZDEBUG */
#ifdef OS2
#ifdef CK_LABELED
    }
#endif /* CK_LABELED */
#endif /* OS2 */

    if (ferror(fp[ZIFILE])) {
        debug(F100,"ZINFILL ferror","",0);
        debug(F100,"ZINFILL errno","",errno);
#ifdef ZDEBUG
        printf("ZINFILL errno=%d\n",errno);
#endif /* ZDEBUG */
        return(-2);
    }
    /* In case feof() didn't work just above -- sometimes it doesn't... */
    if (zincnt == 0) {
       if (feof(fp[ZIFILE]) ) {
           debug(F100,"ZINFILL count 0 EOF return -1","",0);
           return (-1);
       } else {
           debug(F100,"ZINFILL count 0 not EOF return -2","",0);
           return(-2);
       }
    }
    zinptr = zinbuffer;    /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;                           /* One less char in buffer */
    return((int)(*zinptr++) & 0377);    /* because we return the first */
}

/*  Z S O U T  --  Write a string out to the given file, buffered.  */

int
zsout(n,s) int n; char *s; {
    int rc = 0;
    if (chkfn(n) < 1) return(-1);       /* Keep this, prevents memory faults */
    if (!s) return(0);                  /* Null pointer, do nothing, succeed */
    if (!*s) return(0);                 /* empty string, ditto */
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        return(ttol(s,strlen(s))<0?-1:0);
    }
#endif /* IKSD */

    if (n == ZCTERM)
      return(Vscrnprintf("%s",s));
    if (n == ZSFILE)
      return(write(fileno(fp[n]),s,(int)strlen(s)));
    rc = fputs(s,fp[n]) == EOF ? -1 : 0;
#ifdef COMMENT
    /* Frank believes this can be commented out safely.
     * Doing so will prevent the file from being up to date
     * until the file is closed.  That is probably just fine
     * since the file is locked anyway while open.  5/11/2004
     */
    if (n == ZWFILE)
      fflush(fp[n]);
#endif /* COMMENT */
    return(rc);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

int
zsoutl(n,s) int n; char *s; {
    if (zsout(n,s) < 0)
        return(-1);
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        return(ttoc(LF));
    }
#endif /* IKSD */

    if (n == ZCTERM)
      return(Vscrnprintf("\n"));
    if (n == ZSFILE)                    /* But session log is unbuffered */
      return(write(fileno(fp[n]),"\n",1));
    if (fputs("\n",fp[n]) == EOF)
        return(-1); /* (===OS2 ? \r\n) */
    return(0);
}

/*  Z S O U T X  --  Write x characters to file, unbuffered.  */

int
zsoutx(n,s,x) int n, x; char *s; {
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        return(ttol(s,x));
    }
#endif /* IKSD */

    if (n == ZCTERM) {
        int i;
        for (i = 0; i < x; i++)
          if (!Vscrnprintf("%c",s[i]))
            return(-1);
        return(x);
    }
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
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        return(ttoc(c));
    }
#endif /* IKSD */

    if ( n==ZCTERM )
        return(Vscrnprintf("%c",c));
    if (n == ZSFILE)                    /* Use unbuffered for session log */
        return(write(fileno(fp[n]),&c,1) == 1 ? 0 : -1);
                                /* Buffered for everything else */
        if (putc(c,fp[n]) == EOF)       /* If true, maybe there was an error */
        return(ferror(fp[n])?-1:0);     /* Check to make sure */
        else                                    /* Otherwise... */
        return(0);                      /* There was no error. */
}

/* (PWP) buffered character output routine to speed up file IO */

int
zoutdump() {
    int x;
    zoutptr = zoutbuffer;               /* Reset buffer pointer in all cases */
#ifdef DEBUG
    if (deblog)
      debug(F101,"zoutdump chars","",zoutcnt);
#endif /* DEBUG */
    if (zoutcnt == 0) {                 /* Nothing to output */
        return(0);
    } else if (zoutcnt < 0) {           /* Unexpected negative argument */
        zoutcnt = 0;                    /* Reset output buffer count */
        return(-1);                     /* and fail. */
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
#endif /* CK_LABELED */

#ifdef IKSD
    if (inserver && !local && fp[ZOFILE] == stdout) {
        x = ttol(zoutbuffer,zoutcnt);
        zoutcnt = 0;
        return( x > 0 ? 0 : -1);
    }
#endif /* IKSD */

#ifndef COMMENT
    /*
      Frank Prindle suggested that replacing this fwrite() by an fflush()
      followed by a write() would improve the efficiency, especially when
      writing to stdout.  Subsequent tests showed a 5-fold improvement.

      This appears to slow down NT.  So I changed it back.
    */
    x = fwrite(zoutbuffer, 1, zoutcnt, fp[ZOFILE]);
#else /* COMMENT */
    fflush(fp[ZOFILE]);
    x = write(fileno(fp[ZOFILE]),zoutbuffer,zoutcnt)
#endif /* COMMENT */
    if (x == zoutcnt) {
#ifdef DEBUG
        if (deblog)                     /* Save a function call... */
          debug(F101,"zoutdump write ok","",zoutcnt);
#endif /* DEBUG */
        zoutcnt = 0;                    /* Reset output buffer count */
        return(0);                      /* write() worked OK */
    } else {
#ifdef DEBUG
        if (deblog) {
            debug(F101,"zoutdump write error","",errno);
            debug(F101,"zoutdump write returns","",x);
        }
#endif /* DEBUG */
        zoutcnt = 0;                    /* Reset output buffer count */
        return(-1);                     /* write() failed */
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
    if (n != ZDFILE) debug(F101,"chkfn","",n);
    if (n < 0 || n >= ZNFILS) {
        if (n != ZDFILE) debug(F101,"chkfn out of range","",n);
        return(-1);
    } else {
        if (n != ZDFILE) debug(F101,"chkfn fp[n]","",fp[n]);
        return((fp[n] == NULL) ? 0 : 1);
    }
}

/*  Z G E T F S -- Return file size regardless of accessibility */
/*
  Returns:
    >= 0 if the file size can be determined
      -1 if the file size is unknown
      -2 if the file is a directory
*/

CK_OFF_T
zgetfs(name) char *name;
{
    struct stat buf;
    int x;

    if (!name)
        return(-1);
    if (!*name)
        return(-1);

    x = os2stat(name,&buf);
    if (x < 0) {
        return(-1);                             /* stat failed */
    }
    else if (!S_ISREG (buf.st_mode)             /* Must be regular file */
#ifdef S_ISFIFO
              && !S_ISFIFO (buf.st_mode)        /* or FIFO */
#endif /* S_ISFIFO */
              ) {
        return(-2);
    }
    return(buf.st_size);                        /* return size */
}


/*  Z C H K I  --  Check if input file exists and is readable  */

/*
  Returns:
   >= 0 if the file can be read (returns the size).
     -1 if file doesn't exist or can't be accessed,
     -2 if file exists but is not readable (e.g. a directory file).
     -3 if file exists but protected against read access.

  For Berkeley Unix, a file must be of type "regular" to be readable.
  Directory files, special files, and symbolic links are not readable.
*/

CK_OFF_T
zchki(name) char *name; {
    struct stat buf;
    int x, itsadir = 0;
    extern int zchkid, diractive, matchfifo;

    if (!name) return(-1);

#ifdef CKROOT
    debug(F111,"zchki setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zchki setroot violation",name,0);
            return(-1);
        }
#endif /* CKROOT */

    x = os2stat(name,&buf);
    if (x < 0) {
        debug(F111,"zchki stat fails",name,errno);
        return(-1);
    }

    if (S_ISDIR (buf.st_mode))
      itsadir = 1;

    if (!(itsadir && zchkid)) {         /* Unless this... */
        if (!S_ISREG (buf.st_mode)      /* Must be regular file */
#ifdef S_ISFIFO
            && (!matchfifo || !S_ISFIFO (buf.st_mode))  /* or FIFO */
#endif /* S_ISFIFO */
            ) {
            debug(F111,"zchki not regular file (or fifo)",name,matchfifo);
            return(-2);
        }
    }
    debug(F111,"zchki stat ok:",name,x);

    if (diractive) {                    /* If listing don't check access */
        x = 1;
    } else {
#ifdef SW_ACC_ID
        debug(F100,"zchki swapping ids for access()","",0);
        priv_on();
#endif /* SW_ACC_ID */
        x = access(name,R_OK);
#ifdef SW_ACC_ID
        priv_off();
        debug(F100,"zchki swapped ids restored","",0);
#endif /* SW_ACC_ID */
    }
    if (x < 0) {        /* Is the file accessible? */
        debug(F111,"zchki access failed:",name,x); /* No */
        return(-3);
    } else {
        iflen = buf.st_size;            /* Yes, remember size */
        ckstrncpy(nambuf,name,MAXNAMLEN); /* and name globally. */
        debug(F111,"zchki access ok:",name,iflen);
        return( (iflen > -1L) ? iflen : 0L );
    }
}

/*  Z C H K O  --  Check if output file can be created  */

/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
int
zchko(name) char *name; {
    int i, x, itsadir=0;
    char *s;
    extern int zchkod;                  /* Used by IF WRITEABLE */

    if (!name) return(-1);              /* Watch out for null pointer. */

#ifdef CKROOT
    debug(F111,"zchko setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zchko setroot violation",name,0);
            errno = EACCES;
            return(-1);
        }
#endif /* CKROOT */

    debug(F111,"zchkod",name,zchkod);

    /*
    zchkod is a global flag meaning we're checking not to see if the directory
    file is writeable, but if it's OK to create files IN the directory.
    */
    if (!zchkod && isdir(name))         /* Directories are not writeable */
      return(-1);

    x = (int)strlen(name);              /* Get length of filename */
    s = malloc(x+3);                    /* Must copy because we can't */
    if (!s) {                           /* write into our argument. */
        fprintf(stderr,"zchko: Malloc error 46\n");
        return(-1);
    }
    ckstrncpy(s,name,x+3);

    for (i = x; i > 0; i--) {           /* Strip filename from right. */
        if (ISDIRSEP(s[i-1])) {
            itsadir = 1;
            break;
        }
    }
    debug(F101,"zchko i","",i);

#ifdef COMMENT
/*
  The following does not work for "foo/bar" where the foo directory does
  not exist even though we could create it: access("foo/.") fails, but
  access("foo") works OK.
*/
/* So now we use "path/." if path given, or "." if no path given. */
    s[i++] = '.';                       /* Append "." to path. */
    s[i] = '\0';
#else
/* So NOW we strip path segments from the right as long as they don't */
/* exist -- we only call access() for path segments that *do* exist.. */
/* (But this isn't quite right either since now zchko(/foo/bar/baz/xxx) */
/* succeeds when I have write access to foo and bar but baz doesn't exit.) */

    if (itsadir && i > 0) {
        s[i-1] = '\0';
        while (s[0] && !isdir(s)) {
            for (i = (int)strlen(s); i > 0; i--) {
                if (ISDIRSEP(s[i-1])) {
                    s[i-1] = '\0';
                    break;
                }
            }
            if (i == 0)
              s[0] = '\0';
        }
    } else {
        s[i++] = '.';                   /* Append "." to path. */
        s[i] = '\0';
    }
#endif /* COMMENT */

    if (!s[0])
        strcpy(s,".");

#ifdef NT
    {
        /* We are only checking directory access at this point. 
         * The file attribute READ_ONLY is meaningless for directories
         * in Windows.  So all we can do is see whether we can access
         * the directory information at all.  We can't use access()
         * for this because it is broken.
         */
        DWORD attr = GetFileAttributes((LPTSTR)s);
        x = (attr == 0xffffffff) ? -1 : 0;
    }
#else /* NT */
    x = 0;              /* No unwritable directories in OS/2 */
#endif /* NT */

    if (x < 0)
      debug(F111,"zchko access failed:",s,errno);
    else
      debug(F111,"zchko access ok:",s,x);
    free(s);                            /* Free temporary storage */
    return((x < 0) ? -1 : 0);           /* and return. */
}

static char *
UNCname(char * name) {
    int i;
    for ( i=0; name[i] && i <= MAXNAMLEN; i++ )
        UNCnam[i] = (name[i] == '/' ? '\\' : name[i]);
    UNCnam[i] = '\0';
    return(UNCnam);
}

/*  Z D E L E T  --  Delete the named file.  */

int
zdelet(name) char *name; {
    int x;
#ifdef CK_LOGIN
    if (isguest)
      return(-1);
#endif /* CK_LOGIN */

#ifdef CKROOT
    debug(F111,"zdelet setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zdelet setroot violation",name,0);
            return(-1);
        }
#endif /* CKROOT */

    if ( !strncmp(name,"//",2) || !strncmp(name,"\\\\",2))
        x =unlink(UNCname(name));
    else
        x = unlink(name);
    debug(F111,"zdelet",name,x);
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_FC && ckxlogging) {
        char buf[1024];
        fullname[0] = '\0';
        zfnqfp(name,CKMAXPATH,fullname);
        debug(F110,"zdelet fullname",fullname,0);
        if (x < 0) {
            sprintf(buf, "file[] %s: delete failed", fullname);
            cksyslog(SYSLG_FC, 0, buf, NULL, NULL);
        } else {
            sprintf(buf, "file[] %s: delete ok", fullname);
            cksyslog(SYSLG_FC, 1, buf, NULL, NULL);
        }
    }
#endif /* CKSYSLOG */
    return(x);
}

void
ChangeNameForFAT(char *name) {
    char *src, *dst, *next, *ptr, *dot, *start;
    static char invalid[] = ":;,=+\"[]<>| \t";


    /* Skip over the drive letter (if any) */
    if ( isalpha(name[0]) && (name[1] == ':') )
        start = name + 2;
    else
        start = name;

    /* Absolute path? */
    src = dst = start;
    if ( ISDIRSEP(*src) )
        src++, dst++;

    while ( *src )
    {
        /* Find the end of the directory or file name */
        for ( next = src;
              *next && !(ISDIRSEP(*next));
              next++ );

        /* Find the last dot in this name */
        for ( ptr = src, dot = NULL; ptr < next; ptr++ )
            if ( *ptr == '.' )
            {
                dot = ptr; /* remember last dot */
                *ptr = '_';
            }

        /* If no DOT then treat an Underscore as a DOT */
        if ( dot == NULL )
            for ( ptr = src; ptr < next; ptr++ )
                if ( *ptr == '_' )
                    dot = ptr; /* remember last _ as if it were a dot */

        /* Does the name meet 8.3 format? */
        if ( dot && (dot > src) &&
             ((next - dot <= 4) ||
               ((next - src > 8) && (dot - src > 3))) )
        {
            if ( dot )
                *dot = '.';

            for ( ptr = src; (ptr < dot) && ((ptr - src) < 8); ptr++ )
                *dst++ = *ptr;

            for ( ptr = dot; (ptr < next) && ((ptr - dot) < 4); ptr++ )
                *dst++ = *ptr;
        }
        else
        {
            if ( dot && (next - src == 1) )
                *dot = '.';           /* special case: "." as a path component */

            for ( ptr = src; (ptr < next) && ((ptr - src) < 8); ptr++ )
                *dst++ = *ptr;
        }

        *dst++ = *next; /* either '/' or 0 */

        if ( *next )
        {
            src = next + 1;

            if ( *src == 0 ) /* handle trailing '/' on dirs ! */
                *dst = 0;
        }
        else
            break;
    }

    /* Finally, convert invalid characters */
    for ( src = start; *src != 0; ++src )
        if ( strchr(invalid, *src) != NULL )
            *src = '_';
}


int
IsFileNameValid(char *name)
{
    int rc = FALSE;
    static char prevpath[CKMAXPATH+4] = { '\0', '\0' };
    static int prevstat = -1;

#ifdef NT
    HANDLE hf ;
    DWORD lasterror = 0 ;
#else /* NT */
    HFILE hf;
    ULONG uAction;
#endif /* NT */

    if ( prevstat > -1 ) {
        if ( name[0] == prevpath[0] ) {
            if ( !strcmp(name,prevpath) ) {
                debug(F111,"isdir cache hit",name,prevstat);
                return(prevstat);
            }
        }
    }

#ifdef NT
    /* A difference between Win9x and NT/2000 is that on Win9x a
       ERROR_FILE_NOT_FOUND when the name is invalid such as a name
       surrounded by double-quotes when OPEN_EXISTING is used instead
       of OPEN_ALWAYS.
    */
    hf = CreateFile( name,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow Sharing */
                     NULL, /* No Security Attributes specified */
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL ) ;
    lasterror = GetLastError();
    if ( hf == INVALID_HANDLE_VALUE )
    {
        switch ( lasterror ) {
        case ERROR_INVALID_PARAMETER:
            debug(F111,"IsFileNameValid() Invalid Handle","Invalid Parameter",lasterror);
            break;
        case ERROR_INVALID_NAME:
            debug(F111,"IsFileNameValid() Invalid Handle","Invalid Name",lasterror);
            break;
        case ERROR_FILENAME_EXCED_RANGE:
            debug(F111,"IsFileNameValid() Invalid Handle","FileName Exceeds Range",lasterror);
            break;
        case ERROR_FILE_NOT_FOUND:
            debug(F111,"IsFileNameValid() Invalid Handle","File Not Found",lasterror);
            if ( isWin95() ) {
                hf = CreateFile( name,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow Sharing */
                                 NULL, /* No Security Attributes specified */
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL ) ;
                if ( hf != INVALID_HANDLE_VALUE ) {
                                        CloseHandle(hf);
                    debug(F111,"IsFileNameValid() Win9x Valid Handle",
                           "File Created",lasterror);
                    rc = TRUE;
                    DeleteFile( name ) ;
                }
                break;
            }
            /* fall through */
        default:
            debug(F111,"IsFileNameValid() Invalid Handle","returning TRUE",lasterror);
            rc = TRUE;
        }
    }
    else
    {
        debug(F101,"IsFileNameValid() Valid Handle","",lasterror);
        CloseHandle( hf ) ;
        rc = TRUE ;
    }
#else /* NT */
    switch( DosOpen(name, &hf, &uAction, 0, 0, FILE_OPEN,
                     OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0) )
    {
    case ERROR_INVALID_NAME:
    case ERROR_FILENAME_EXCED_RANGE:
    case ERROR_INVALID_PARAMETER:
        break;          /* FALSE */
    case NO_ERROR:
        DosClose(hf);
    default:
        rc = TRUE;
    }
#endif /* NT */
    ckstrncpy(prevpath,name,CKMAXPATH+4);
    prevstat = rc;
    return rc;
}

unsigned long
zdskspace(int drive) {
#ifdef NT
    DWORD spc, bps, fc, c ;
    char rootpath[4] ;
    int gle;
    BOOL (_stdcall *pGetDiskFreeSpaceEx)
        ( LPCTSTR lpDirectoryName,                 // directory name
          PULARGE_INTEGER lpFreeBytesAvailable,    // bytes available to caller
          PULARGE_INTEGER lpTotalNumberOfBytes,    // bytes on disk
          PULARGE_INTEGER lpTotalNumberOfFreeBytes // free bytes on disk
          )=NULL;
    ULARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
    (FARPROC) pGetDiskFreeSpaceEx = GetProcAddress( GetModuleHandle("kernel32.dll"),
                                          "GetDiskFreeSpaceExA");

    if (drive)
    {
        rootpath[0] = '@' + drive ;
        rootpath[1] = ':' ;
        rootpath[2] = '\\' ;
        rootpath[3] = '\0' ;

        debug(F110,"zdskspace() rootpath",rootpath,0);

        if (pGetDiskFreeSpaceEx)
        {
            debug(F110,"zdskspace()","pGetDiskFreeSpaceEx",0);

            if ( !pGetDiskFreeSpaceEx (rootpath,
                                        (PULARGE_INTEGER)&i64FreeBytesToCaller,
                                        (PULARGE_INTEGER)&i64TotalBytes,
                                        (PULARGE_INTEGER)&i64FreeBytes)) {
                gle = GetLastError();
                debug(F111,"zdskspace() failure","lastError",gle);
                return (1) ;
            }
            return(i64FreeBytesToCaller.HighPart ? (unsigned long)-1 :
                    i64FreeBytesToCaller.LowPart);
        } else {
            debug(F110,"zdskspace()","GetDiskFreeSpace",0);
            if ( !GetDiskFreeSpace( rootpath, &spc, &bps, &fc, &c )) {
                gle = GetLastError();
                debug(F111,"zdskspace() failure","lastError",gle);
                return (1) ;
            }
            return spc * bps * fc ;
        }
    }
    else {
        debug(F110,"zdskspace()","no drive specified",0);

        if (pGetDiskFreeSpaceEx)
        {
            debug(F110,"zdskspace()","pGetDiskFreeSpaceEx",0);
            if ( !pGetDiskFreeSpaceEx (NULL,
                                        (PULARGE_INTEGER)&i64FreeBytesToCaller,
                                        (PULARGE_INTEGER)&i64TotalBytes,
                                        (PULARGE_INTEGER)&i64FreeBytes)) {
                gle = GetLastError();
                debug(F111,"zdskspace() failure","lastError",gle);
                return (1) ;
            }
            return(i64FreeBytesToCaller.HighPart  ? (unsigned long)-1 :
                    i64FreeBytesToCaller.LowPart);
        } else {
            debug(F110,"zdskspace()","GetDiskFreeSpace",0);
            if ( !GetDiskFreeSpace( NULL, &spc, &bps, &fc, &c ))
                gle = GetLastError();
                debug(F111,"zdskspace() failure","lastError",gle);
                return (1) ;
        }
        return spc * bps * fc ;
    }
#else /* NT */
    FSALLOCATE fsa;
    if ( DosQueryFSInfo(drive, 1, (PBYTE) &fsa, sizeof(fsa)) )
        return 0;
    return fsa.cUnitAvail * fsa.cSectorUnit * fsa.cbSector;
#endif /* NT */
}

char *
GetLoadPath(void) {
#ifdef NT
   static char filename[512] ;
   filename[0] = '\0' ;
   GetModuleFileName( NULL, filename, 512 ) ;
   debug(F111,"GetLoadPath",filename,strlen(filename));
   return filename;
#else /* NT */
   PTIB pptib;
   PPIB pppib;
   char *szPath;

   DosGetInfoBlocks(&pptib, &pppib);

   szPath = pppib -> pib_pchenv;

   while (*szPath)
      szPath = strchr(szPath, 0) + 1;

   return szPath + 1;
#endif /* NT */
}


/*  Z R T O L  --  Convert remote filename into local form  */

VOID
zrtol(name,name2) char *name, *name2; {
    nzrtol(name,name2,1,0,CKMAXPATH);
}

VOID
nzrtol(name,name2,fncnv,fnrpath,max)
    char *name, *name2; int fncnv, fnrpath, max;
{ /* nzrtol */
    char *p; int flag = 0, n = 0;
    char tmpbuf[CKMAXPATH+1], *tmp;
    int devnull = 0;
    int acase = 0;
    if (!name2) return;
    if (!name) name = "";
    debug(F110,"zrtol original name",name,0);

    /* Handle the path -- we don't have to convert its format, since */
    /* the standard path format and our internal format are the same. */

    tmpbuf[0] = NUL;
    tmp = tmpbuf;
    devnull = !strcmp(name,"/dev/null");

    if (!devnull && fnrpath == PATH_OFF) {/* RECEIVE PATHNAMES OFF */
        zstrip(name,&p);
        ckstrncpy(tmpbuf,p,CKMAXPATH);
    } else if (!devnull &&
                fnrpath == PATH_ABS) {  /* RECEIVE PATHNAMES ABSOLUTE */
        ckstrncpy(tmpbuf,name,CKMAXPATH);
    } else if (!devnull &&
                isabsolute(name)) {     /* RECEIVE PATHNAMES RELATIVE */
        sprintf(tmpbuf,".%s",name);
    } else {                            /* Ditto */
        ckstrncpy(tmpbuf,name,CKMAXPATH);
    }
    tmpbuf[CKMAXPATH] = NUL;
    debug(F110,"zrtol tmpbuf",tmpbuf,0);

    /* Even if we are not converting need to make sure that  */
    /* the directory separator is '/' and that a few special */
    /* characters are not used.                              */

    p = tmpbuf;
    while ( *p ) {
        if ( *p == '\\')
            *p = '/';
        p++;
    }

    if (!fncnv || devnull) {            /* Not converting */
        ckstrncpy(name2,tmpbuf,max);    /* We're done. */
        if (!IsFileNameValid(name2)) {
            /* Lets first try just altering some illegal characters*/
            p = name2;
            while ( *p ) {
                switch ( *p ) {
                case ':':
                case '*':
                case '?':
                case '|':
                case '"':
                case '<':
                case '>':
                    *p = '_';
                    break;
                }
                p++;
            }
            if (!IsFileNameValid(name2))
                ChangeNameForFAT(name2);
        }
        debug(F110,"zrtol non-converted name",name2,0);
        return;
    }
    name = tmpbuf;                      /* Converting */

    p = name2;
    for (; *name != '\0' && n < maxnam; name++) {
        if (*name > SP) flag = 1;       /* Strip leading blanks and controls */
        if (flag == 0 && *name < '!')
          continue;
        if (fncnv > 0) {
            if (isupper(*name))             /* Check for mixed case */
                acase |= 1;
            else if (islower(*name))
                acase |= 2;
        }
        switch ( *name ) {
        case ':':                       /* Convert illegal chars */
        case '*':
        case '?':
        case '|':
        case '"':
        case '<':
        case '>':
        case ' ':                       /* And Space */
                break;
            *p = '_';                   /* To underscore */
            break;
        default:
            *p = *name;
        }
        p++;
        n++;
    }
    *p-- = '\0';                        /* Terminate */
    while (*p < '!' && p > name2)       /* Strip trailing blanks & controls */
      *p-- = '\0';

    if (*name2 == '\0') {               /* Nothing left? */
        strcpy(name2,"NONAME");         /* do this... */
    } else if (acase == 1) {            /* All uppercase? */
        p = name2;                      /* So convert all letters to lower */
        while (*p) {
            if (isupper(*p))
              *p = tolower(*p);
            p++;
        }
    }
    if (!IsFileNameValid(name2))
      ChangeNameForFAT(name2);
    debug(F110,"zrtol new name",name2,0);
}

/*  Z S T R I P  --  Strip device & directory name from file specification */

/*  Strip pathname from filename "name", return pointer to result in name2 */

#ifdef pdp11
#define ZSTRPLEN 64
#else
#define ZSTRPLEN MAXPATH
#endif /* pdp11 */
static char work[ZSTRPLEN+1];

VOID
zstrip(name,name2) char *name, **name2; {
    char *cp, *pp;
    int n = 0;
    debug(F110,"zstrip before",name,0);
    if (!name) { *name2 = ""; return; }
    pp = work;
    /* Strip disk letter and colon */
    if (isalpha(*name) && (*(name+1) == ':')) name += 2;
    debug(F110,"zstrip after disk-stripping",name,0);

    for (cp = name; *cp; cp++) {
        if (ISDIRSEP(*cp)) {
            pp = work;
            n = 0;
        } else {
            *pp++ = *cp;
            if (n++ >= ZSTRPLEN)
              break;
        }
    }
    *pp = '\0';                         /* Terminate the string */
    *name2 = work;
    debug(F110,"zstrip after",*name2,0);
}

/*  Z L T O R  --  Local TO Remote */

VOID
zltor(name,name2) char *name, *name2; {
    nzltor(name,name2,1,0,CKMAXPATH);
}

/*  N Z L T O R  --  New Local TO Remote */

/* fncnv = 0 for no conversion, > 0 for regular conversion, < 0 for minimal */

VOID
nzltor(name,name2,fncnv,fnspath,max)
    char *name, *name2; int fncnv, fnspath, max;
{ /* nzltor */
    char *cp=NULL, *pp=NULL;
    int n = 0;
    char *dotp = NULL;
    char tmpbuf[CKMAXPATH+1];
    char *p=NULL;

    CHAR c;

#ifndef NOCSETS
    extern int fcharset, /* tcharset, */ language;
    int langsv;
    _PROTOTYP ( CHAR (*sxo), (CHAR) ) = NULL; /* Translation functions */
#ifdef CK_ANSIC
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);
#else
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();
#endif /* CK_ANSIC */
    langsv = language;
    language = L_USASCII;
#ifdef COMMENT
    /* Proper translation of filenames must be done elsewhere */
    n = tcharset ? tcharset : TC_USASCII;
    sxo = xls[n][fcharset];
#else
    sxo = xls[TC_USASCII][fcharset];
#endif /* COMMENT */
#endif /* NOCSETS */

    debug(F110,"nzltor",name,0);

    /* Handle pathname */

    tmpbuf[0] = NUL;
    if (fnspath == PATH_OFF) {          /* PATHNAMES OFF */
        zstrip(name,&p);
        ckstrncpy(tmpbuf,p,CKMAXPATH);
        tmpbuf[CKMAXPATH] = NUL;
    } else {
        int x = 0;
        if ( name[0] == '.' && ISDIRSEP(name[1]) )
            x = 2;
        else if ( name[0] == '.' && name[1] == '.' && ISDIRSEP(name[2]) )
            x = 3;
        if ( fnspath == PATH_ABS ) {    /* ABSOLUTE */
            zfnqfp(name+x,CKMAXPATH,tmpbuf);
        } else {                        /* RELATIVE */
            ckstrncpy(tmpbuf,name+x,CKMAXPATH);
        }
    }
    debug(F110,"nzltor tmpbuf",tmpbuf,0);

    if (!fncnv) {                       /* Not converting */
        if (isalpha(*tmpbuf) && (*(tmpbuf+1) == ':')) /* Strip disk name */
            ckstrncpy(name2,&tmpbuf[2],max);    /* We're done. */
        else
            ckstrncpy(name2,tmpbuf,max);
#ifndef NOCSETS
        langsv = language;
#endif /* NOCSETS */
        return;
    }
    name = tmpbuf;                      /* Converting */
    if (isalpha(*name) && (*(name+1) == ':')) /* Strip disk name */
        name += 2;

    pp = work;
    for (cp = name, n=0; *cp && n < max; cp++, n++) {        /* Strip path name */
        c= *cp;
#ifndef NOCSETS
        if (sxo) c = (*sxo)(c);         /* Convert to ASCII */
#endif /* NOCSETS */
        if (fncnv > 0 && islower(c))       /* Uppercase letters */
            *pp++ = toupper(c);               /* Change tilde to 'X' */
        else if (c == '~')
            *pp++ = 'X';
        else if (fncnv > 0 && c == '#')            /* Change number sign to 'X' */
            *pp++ = 'X';
        else if (c == '*' || c == '?')
            *pp++ = 'X';
        else if (c <= ' ')            /* Change space and controls to 'X' */
            *pp++ = '_';                /* was 'X' */
        else if (fncnv > 0 && c == '.') {          /* Change dot to underscore */
            dotp = pp;                  /* Remember where we last did this */
            *pp++ = '_';
        } else
            *pp++ = c;
    }
    *pp = NUL;                          /* Tie it off. */
    if (dotp) *dotp = '.';              /* Restore last dot (if any) */
    cp = name2;                         /* If nothing before dot, */
    if (*work == '.') *cp++ = 'X';      /* insert 'X' */
    ckstrncpy(cp,work,max);
#ifndef NOCSETS
    language = langsv;
#endif /* NOCSETS */
    debug(F110,"zltor name2",name2,0);
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
#ifndef NOSPL
    extern struct mtab *mactab;             /* Main macro table */
    extern int nmac;                        /* Number of macros */
#endif /* NOSPL */
    char hd[CKMAXPATH+1];
    char *p;
    int len;

    debug(F110,"zchdir",dirnam,0);
    if (dirnam == NULL || dirnam == "" || *dirnam == '\0') /* If arg is null */
      dirnam = zhome();                 /* use user's home directory. */
    debug(F110,"zchdir 2",dirnam,0);

    ckstrncpy(hd,dirnam,CKMAXPATH+1);
    debug(F110,"zchdir 3",hd,0);

#ifdef CKROOT
    debug(F111,"zchdir setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(hd)) {
            debug(F110,"zchdir setroot violation",hd,0);
            return(0);
        }
#endif /* CKROOT */

    if (isalpha(hd[0]) && hd[1] == ':') {
        if (zchdsk(hd[0]))
          return(0);
        if (hd[2] == 0)
          return(1);                    /* Handle drive-only case */
    }


    len = strlen(hd) ;
    if ( !strncmp(hd,"//",2) || !strncmp(hd,"\\\\",2)) {
        int i;
        if ( ISDIRSEP(hd[len-1]) ) {
            /* We now need to strip the trailing directory separator if it is not */
            /* part of the machine or object name.                                */
            for ( i=2 ; i<len && !(ISDIRSEP(hd[i])) ; i++ ); /* machine */
            for ( i++ ; i<len && !(ISDIRSEP(hd[i])) ; i++ ); /* object */
            if (i < len-1 )
                hd[len-1] = '\0';
        }
    } else {
        /* strip trailing DIRSEP except after d:; chdir() doesn't like it */
        if ( len > 1 && ISDIRSEP(hd[len - 1]) && hd[len - 2] != ':')
            hd[len - 1] = 0;
    }

    if (chdir(hd) == 0) {
#ifdef IKSDB
        extern int ikdbopen;
        if (inserver && ikdbopen) {
#ifdef CKROOT
            slotdir(isguest ? anonroot : "", zgtdir());
#else
            slotdir("", zgtdir());
#endif /* CKROOT */
        }
#endif /* IKSDB */

#ifndef NOSPL
        if (nmac) {             /* Any macros defined? */
            int k;                /* Yes */
            static on_cd = 0;
            if ( !on_cd ) {
                on_cd = 1;
                k = mlook(mactab,"on_cd",nmac); /* Look this up */
                if (k >= 0) {                   /* If found, */
                    if (dodo(k,zgtdir(),0) > -1)  /* set it up, */
                        parser(1);                  /* and execute it */
                }
                on_cd = 0;
            }
        }
#endif /* NOSPL */
        return(1);              /* Try to cd */
    } else
        return(0);
}

/*  Z H O M E  --  Return pointer to user's home directory  */

static char homedir[CKMAXPATH+1]="";

char *
zhome() {
    extern char startupdir[];
    int i;
    char * home;

#ifdef CKROOT
    if (ckrootset)
      return((char *)ckroot);
#endif /* CKROOT */

    if (homedir[0])
        return(homedir);

    home = getenv("HOME");
    if (home) {
        ckstrncpy(homedir,home,CKMAXPATH);
    } else {
#ifdef NT
        char * homedrive = (char *)GetHomeDrive();
        char * homepath  = (char *)GetHomePath();
        char * profile   = getenv("USERPROFILE");
        char * personal  = (char *)GetPersonal();
        if (homedrive && homepath)
            sprintf(homedir,"%s%s",homedrive,homepath);
        else if ( profile )
            sprintf(homedir,"%s",profile);
        else if ( personal )
            ckstrncpy(homedir,personal,CKMAXPATH);
        else
#endif /* NT */
            ckstrncpy(homedir,startupdir,CKMAXPATH);
    }

#ifdef NT
    GetShortPathName(homedir,homedir,CKMAXPATH);
#endif /* NT */

    /* we know have the directory, but need to make it consistent */
    /* with all of the other Kermit directory variables: forward  */
    /* slashes and a trailing slash.                              */
    i = strlen(homedir)-1;
    if ( !(ISDIRSEP(homedir[i])) ) {
        homedir[i+1] = '/';
        homedir[i+2] = NUL;
    }
    for (i-- ; i >= 0 ; i--) {
        if ( homedir[i] == '\\' )
          homedir[i] = '/';
    }

    debug(F110,"zhome sets homedir to",homedir,0);
    return(homedir);
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

#define CWDBL MAXPATH
static char cwdbuf[CWDBL+1];

char *
zgtdir() {
    char *buf;
    int x=0,len=0;

#ifndef __IBMC__ /* which has a macro for this */
    extern char *getcwd();
#endif /* __IBMC__ */
    buf = cwdbuf;
    debug(F101,"zgtdir CWDBL","",CWDBL);
    getcwd(buf,CWDBL);
    len = strlen(buf);
    for ( x=len-1;x >= 0; x-- )
        if ( buf[x] == '\\' )
            buf[x] = '/';
    if ( buf[len-1] != '/' ) {
        buf[len] = '/';
        buf[len+1] = NUL;
    }
    debug(F110,"zgtdir getcwd()",buf,0);
    return(buf);
}

/*  Z X C M D -- Run a system command so its output can be read like a file */

int
zxcmd(filnum,comand) int filnum; char *comand; {
    int out, rc;
    int pipes[2];

    debug(F111,"zxcmd",comand,filnum);
    if (chkfn(filnum) < 0) return(-1);  /* Need a valid Kermit file number. */
    if (filnum == ZSTDIO || filnum == ZCTERM) /* But not one of these. */
      return(0);

    /* popen() invalidates the StdHandle temporarily, don't do screen updates */
    /* while it starts the child process                                      */
    rc = RequestScreenMutex( 5000 );
    if ( rc )
    {
        debug(F100,"zxcmd() unable to retrieve Screen Mutex","",0);
        return -1;
    }

    out = (filnum == ZIFILE || filnum == ZRFILE) ? 0 : 1 ;
    debug(F101,"zxcmd out",comand,out);

    if (!out) {                         /* Input from a command */
        if (priv_chk() ||
             ((fp[filnum] = popen(comand,"rb")) == NULL)
             ) {
            ReleaseScreenMutex();
            return(0);
        }
        zincnt = 0;                     /* (PWP) reset input buffer */
        zinptr = zinbuffer;
    } else { /* Output to a command */
        if (priv_chk() ||
             ((fp[filnum] = popen(comand,"wb")) == NULL)
             ) {
            ReleaseScreenMutex();
            return(0);
        }
        zoutcnt = 0;                    /* (PWP) reset input buffer */
        zoutptr = zoutbuffer;
    }
    ReleaseScreenMutex();
    fp[ZSYSFN] = fp[filnum];            /* remember the pipe handle */
    ispipe[filnum] = 1;
    return(1);
} /* zxcmd */

/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */

/*  Used internally by zclose - returns -1 on failure, 1 on success. */

int
zclosf(filnum) int filnum; {
    int wstat, out;
    debug(F101,"zclosf filnum","",filnum);
    out = (filnum == ZIFILE || filnum == ZRFILE) ? 0 : 1 ;
    debug(F101,"zclosf out","",out);

#ifndef NOPOPEN
    if (ispipe[filnum]) {
        int x;
        x = pclose(fp[filnum]);
        pexitstat = x;                  /* *** CHECK THIS *** */
        fp[filnum] = NULL;
        ispipe[filnum] = 0;
        return((x != 0) ? -1 : 1);
    }
#endif /* NOPOPEN */
    debug(F101,"zclosf fp[filnum]","", fp[filnum]);
    debug(F101,"zclosf fp[ZSYSFN]","", fp[ZSYSFN]);

    fclose(fp[filnum]);
    fp[filnum] = NULL;
    ispipe[filnum] = 0;
    debug(F101,"zclosf fp[filnum]","",fp[filnum]);
#ifdef CK_CHILD
    return(pexitstat == 0 ? 1 : 0);
#else
    return(1);
#endif /* CK_CHILD */
}

#ifdef NT
static HANDLE hKernel = INVALID_HANDLE_VALUE;
static DWORD (WINAPI *p_GetLongPathNameA)(
    LPCSTR lpFileName,
    LPSTR lpBuffer,
    DWORD cchBuffer
    ) = NULL;

DWORD 
ckGetLongPathName(LPCSTR lpFileName, LPSTR lpBuffer, DWORD cchBuffer)
{
    if ( !p_GetLongPathNameA ) {
        if (hKernel == INVALID_HANDLE_VALUE)
            hKernel = LoadLibrary("kernel32.dll");
        if (hKernel != INVALID_HANDLE_VALUE)
            (FARPROC) p_GetLongPathNameA =
                GetProcAddress( hKernel, "GetLongPathNameA" );
    }

    if ( !p_GetLongPathNameA ) {
        DWORD len, i;
        if ( !lpFileName || !lpBuffer )
            return(0);

        len = strlen(lpFileName);
        if ( len + 1 <= cchBuffer )
            ckstrncpy(lpBuffer,lpFileName,cchBuffer);
        return (len);
    } else {
        return(p_GetLongPathNameA(lpFileName,lpBuffer,cchBuffer));
    }
}
#endif /* NT */

/*  Z X P A N D  --  Expand a wildcard string into an array of strings  */
/*
  Returns the number of files that match fn1, with data structures set up
  so that first file (if any) will be returned by the next znext() call.

  Depends on external variable wildxpand: 0 means we expand wildcards
  internally, nonzero means we call the shell to do it.
  OS2 ignores wildxpand since it always expands the list internally.
*/

int win95_8_3 = 0;              /* Do not use 8.3 FAT file name notation */

static char findpath[2][MAXPATH+4];
static char findpathrel[2][MAXPATH+4];
static char findfspec[2][MAXPATH];
static char findfspec2[2][MAXPATH];
static int findpathwild[2];
#define MAX_FIND_LEVEL 63
/* findlevel indicates how deeply nested we are in the directory tree */
/* it is used as an index into the other structures.                  */
static int  findlevel[2] = {-1,-1};

/* findeop - (end of path) is a pointer to the end of the path at the */
/* current depth in the tree.  It is used as a method for quickly     */
/* restoring the previous level when setting up the next query.       */
static char * findeop[2][MAX_FIND_LEVEL+1] = {{
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL
},{
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL
}};

#ifdef NT
/* finddata - the directory entry from the most recent query */
static WIN32_FIND_DATA finddata[2];

/* findhandle - a stack of handles for each of the outstanding queries */
static HANDLE findhandle[2][MAX_FIND_LEVEL+1] = {{
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE
},{
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE
}};
#else /* NT */
/* finddata - the directory entry from the most recent query */
static FILEFINDBUF3    finddata[2];

/* findhandle - a stack of handles for each of the outstanding queries */
static HDIR            findhandle[2][MAX_FIND_LEVEL+1] = {{
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE
},{
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,
    HDIR_CREATE,HDIR_CREATE,HDIR_CREATE,HDIR_CREATE
}};
static ULONG           findcount=0;
#endif /* NT */
static int os2dironly[2]={0,0};         /* only report directories?   */
static int os2fileonly[2]={0,0};        /* only report files?         */
static int os2recursive[2]={0,0};       /* recursively traverse tree? */
static char localfn[2][MAXPATH+4];
static char rewindfn[2][MAXPATH+4];
static char rewindpath[2][MAXPATH+4];
static char rewindpathrel[2][MAXPATH+4];
static char rewindfspec[2][MAXPATH];
static char stream[2][MAXPATH];
static int lasterror[2]={0,0};
int zxpn = 0;                       /* 0 or 1, allows separate searches */

#ifdef RECURSIVE
int
os2findpush(void) {
    if ( findlevel[zxpn] >= MAX_FIND_LEVEL )
      return(0);
    findlevel[zxpn]++;
    findeop[zxpn][findlevel[zxpn]] = findpath[zxpn] + strlen(findpath[zxpn]);
#ifdef NT
    ckstrncat(findpath[zxpn],finddata[zxpn].cFileName,MAXPATH+4);
#else /* NT */
    ckstrncat(findpath[zxpn],finddata[zxpn].achName,MAXPATH+4 );
#endif /* NT */
    ckstrncat(findpath[zxpn],"/",MAXPATH+4);
    ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
    ckstrncat(localfn[zxpn],findpathrel[zxpn],MAXPATH+4);
    ckstrncat(localfn[zxpn],findfspec[zxpn],MAXPATH+4);
    debug(F111,"os2findpush",findpath[zxpn],findlevel[zxpn]);
    return(1);
}

int
os2findpop(void) {
    debug(F111,"os2findpop",findpath[zxpn],findlevel[zxpn]);
    if ( findlevel[zxpn] <= 0 )
      return(0);
    *findeop[zxpn][findlevel[zxpn]] = NUL;
    findeop[zxpn][findlevel[zxpn]] = NULL;
    findlevel[zxpn]--;
    ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
    ckstrncat(localfn[zxpn],findpathrel[zxpn],MAXPATH+4);
    ckstrncat(localfn[zxpn],findfspec[zxpn],MAXPATH+4);
    return(1);
}
#endif /* RECURSIVE */

int
os2findfirstfile(char * fn) {
#ifndef NT
    HDIR hfind = HDIR_CREATE;
#endif /* NT */
    debug(F110,"os2findfirstfile searching for",fn,0);

    if ( findlevel[zxpn] < 0 )
        findlevel[zxpn] = 0;
#ifdef NT
    findhandle[zxpn][findlevel[zxpn]] = FindFirstFile( fn, &finddata[zxpn] );
    if ( findhandle[zxpn][findlevel[zxpn]] == INVALID_HANDLE_VALUE ) {
        return(0);
    }
    debug(F110,"os2findfirstfile found",finddata[zxpn].cFileName,0);
    debug(F110,"os2findfirstfile found",
           finddata[zxpn].cAlternateFileName,0);
    if ( finddata[zxpn].cAlternateFileName[0] == '\0' )
        ckstrncpy(finddata[zxpn].cAlternateFileName,finddata[zxpn].cFileName,14);
#else /* NT */
    findcount = 1;                      /* one entry at a time */
    lasterror[zxpn] =
      DosFindFirst(fn,
                   &hfind,
                   (ULONG) FILE_NORMAL | FILE_ARCHIVED | FILE_DIRECTORY |
                   FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                   (PVOID) &finddata[zxpn],
                   (ULONG) sizeof(finddata[0]),
                   &findcount, FIL_STANDARD);
    if (lasterror[zxpn] || findcount != 1) {
        DosFindClose(hfind);
        return(0);
    }
    findhandle[zxpn][findlevel[zxpn]] = hfind;
#endif /* NT */
    return(1);
}

int
os2findnextfile(void) {
#ifdef NT
    int rc = 0;
    if ( findlevel[zxpn] < 0 )
        return 0;
    if ( findhandle[zxpn][findlevel[zxpn]] == INVALID_HANDLE_VALUE)
      return 0;
    rc = FindNextFile( findhandle[zxpn][findlevel[zxpn]], &finddata[zxpn] );
    debug(F111,"os2findnextfile",finddata[zxpn].cFileName,rc);
    debug(F111,"os2findnextfile (alternate)",
           finddata[zxpn].cAlternateFileName,rc);
    if ( finddata[zxpn].cAlternateFileName[0] == '\0' ) {
        ckstrncpy(finddata[zxpn].cAlternateFileName,finddata[zxpn].cFileName,14);
        strupr(finddata[zxpn].cAlternateFileName);
    }
    return(rc);
#else /* NT */
    HDIR hfind = findhandle[zxpn][findlevel[zxpn]];
    if (hfind == HDIR_CREATE)
      return 0;
    findcount = 1;
    lasterror[zxpn] =
      DosFindNext(hfind,
                  (PVOID) &(finddata[zxpn]),
                  (ULONG) sizeof(finddata[0]),
                  &findcount);
    return(!lasterror[zxpn] && (findcount == 1));
#endif /* NT */
}

int
os2findclose(void) {
    debug(F111,"os2findclose","findlevel",findlevel[zxpn]);
    if ( findlevel[zxpn] < 0)
        return(0);
#ifdef NT
    FindClose(findhandle[zxpn][findlevel[zxpn]]);
    findhandle[zxpn][findlevel[zxpn]] = INVALID_HANDLE_VALUE;
#else
    DosFindClose(findhandle[zxpn][findlevel[zxpn]]);
    findhandle[zxpn][findlevel[zxpn]] = HDIR_CREATE;
#endif /* NT */
    return 1;
}

int
os2findisdir(void) {
#ifdef NT
    return (finddata[zxpn].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else /* NT */
    return (finddata[zxpn].attrFile & FILE_DIRECTORY);
#endif /* NT */
}

/* Define DOTDIRS if you want znext() to return the "." and ".." entries */
/* in the directory pointed to by the relative path of 'fn'.             */

int
zxrewind() {
    int i,j,len;
    CHAR * FileName=NULL;

    /* Cancel previous zxpand if necessary */
    for (; findlevel[zxpn] >= 0; findlevel[zxpn]-- ) {
        if (findhandle[zxpn][findlevel[zxpn]] !=
#ifdef NT
             INVALID_HANDLE_VALUE
#else /* NT */
             HDIR_CREATE
#endif /* NT */
             )
        {
            os2findclose();
        }
        findeop[zxpn][findlevel[zxpn]] = NULL;
    }

    /* Initialize variables for expannsion */
    fcount[zxpn] = fcntsav[zxpn];
    fcountstream[zxpn] = fcntstreamsav[zxpn];
    strcpy(localfn[zxpn],rewindfn[zxpn]);
    strcpy(findpath[zxpn],rewindpath[zxpn]);
    strcpy(findpathrel[zxpn],rewindpathrel[zxpn]);
    strcpy(findfspec[zxpn],rewindfspec[zxpn]);
    debug(F111,"zxrewind","fcntsav[zxpn]",fcntsav[zxpn]);
    findlevel[zxpn] = 0;
    findeop[zxpn][0] = findpath[zxpn];

    /* Now that we know how many, reset for the call to znext() */
    if (!os2findfirstfile(localfn[zxpn])) {
#ifdef RECURSIVE
        if (os2recursive[zxpn]) {
            /* However, we are looking at subdirectories   */
            /* so we must now switch to the next directory */
            os2findclose();

          findfirstsubdir:
            /* prepare to list all subdirectories */
            if (findpath[zxpn][0])
              ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
            else
              ckstrncpy(localfn[zxpn],"./",MAXPATH+4);
            ckstrncat(localfn[zxpn],"*",MAXPATH+4);

            /* find the first potential directory */
            if (os2findfirstfile(localfn[zxpn])>0) {
                do {
#ifdef NT
                    if ( win95_8_3 )
                        FileName = finddata[zxpn].cAlternateFileName;
                    else
                        FileName = finddata[zxpn].cFileName;
#else /* NT */
                    FileName = finddata[zxpn].achName;
#endif /* NT */
                    if (os2findisdir() && strcmp(".",FileName) &&
                         strcmp("..",FileName))
                    {
                        /* we have a directory that is not . or .. */
                        if (!os2findpush()) {
                            fcntsav[zxpn] = fcount[zxpn] = 
                                fcountstream[zxpn] =
                                fcntstreamsav[zxpn] = -1;
                            return(fcount[zxpn]+1);
                        }
                        if (os2findfirstfile(localfn[zxpn]) > 0) {
                            debug(F100,"","",0);
                            goto findfirstdone;
                        } else {
                            debug(F100,"","",0);
                            goto findfirstsubdir;
                        }
                    }
                } while ( os2findnextfile() );
            }

          findfirstdir2:
            /* There are no more subdirectories of the current dir */
            os2findclose();
            if (findlevel[zxpn] > 0) {
                os2findpop();
                while (os2findnextfile()) {
                    if (os2findisdir()) {
                        if (!os2findpush()) {
                            fcntsav[zxpn] = fcount[zxpn] = 
                                fcountstream[zxpn] =
                                fcntstreamsav[zxpn] = -1;
                            return(fcount[zxpn]+1);
                        }
                        if (os2findfirstfile(localfn[zxpn])>0) {
                            debug(F100,"","",0);
                            goto findfirstdone;
                        } else {
                            debug(F100,"","",0);
                            goto findfirstsubdir;
                        }
                    }
                }
                goto findfirstdir2;
            }
          findfirstdone:;
        }
        else
#endif /* RECURSIVE */
        {
            fcntsav[zxpn] = fcount[zxpn] = 
                fcountstream[zxpn] =
                    fcntstreamsav[zxpn] = -1;
            return(0);
        }
    }
    return(fcount[zxpn]);
}

/*  N Z X P A N D  --  Expland a file list, with options.  */
/*
  Call with:
   s = pointer to filename or pattern.
   flags = option bits:

     flags & ZX_FILONLY   Match regular files
     flags & ZX_DIRONLY   Match directories
     flags & ZX_RECURSE   Descend through directory tree
     flags & ZX_MATCHDOT  Match "dot files"
     flags & ZX_NOBACKUP  Don't match "backup files"

   Returns the number of files that match s, with data structures set up
   so that first file (if any) will be returned by the next znext() call.
*/
int
nzxpand(CHAR * fn, int flags) {
    int i,j,k,len,brace=0,bracket=0;
    CHAR * FileName=NULL;

    if (!fn)
      return(0);
    if (!(*fn))
      return(0);
    if (strlen(fn) > MAXPATH)
      return(-1);

    debug(F110,"nzxpand fn",fn,0);
    debug(F101,"nzxpand flags","",flags);

#ifdef CKROOT
    debug(F111,"zxpand setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(fn)) {
        debug(F110,"zxpand setroot violation",fn,0);
        return(0);
    }
#endif /* CKROOT */

    /* Cancel previous zxpand if necessary */
    for (; findlevel[zxpn] >= 0; findlevel[zxpn]-- ) {
        if (findhandle[zxpn][findlevel[zxpn]] !=
#ifdef NT
             INVALID_HANDLE_VALUE
#else /* NT */
             HDIR_CREATE
#endif /* NT */
             )
        {
            os2findclose();
        }
    }

    /* Initialize variables for expannsion */
    findlevel[zxpn] = 0;
    findeop[zxpn][0] = findpath[zxpn];
    fcntsav[zxpn] = fcount[zxpn] = 
        fcountstream[zxpn] =
            fcntstreamsav[zxpn] = 0;
    findpathwild[zxpn] = 0;
    findpathrel[zxpn][0] = NUL;
    stream[zxpn][0] = NUL;

    /* save values for calls to znext() */
    os2dironly[zxpn] = (flags & ZX_DIRONLY);
#ifdef RECURSIVE
    os2recursive[zxpn] = (flags & ZX_RECURSE);
#endif /* RECURSIVE */
    os2fileonly[zxpn] = (flags & ZX_FILONLY);

    /* findpath is just the directory */
    ckstrncpy(findpath[zxpn], fn, MAXPATH); /* Leave room to add slash-star */

    /* localfn is the path and the file specification */
    ckstrncpy(localfn[zxpn], fn, MAXPATH);

    /* is fn just a directory? */
    if (isdir(localfn[zxpn]))
    {
        /* if yes, but its really a drive, append "./*" */
        if (strlen(localfn[zxpn]) == 2 && localfn[zxpn][1] == ':' ) {
            ckstrncat(localfn[zxpn], "./*",MAXPATH+4);
        } else {
            /* append "/*" */
            ckstrncat(findpath[zxpn], "/",MAXPATH+4);
            ckstrncat(localfn[zxpn], "/*",MAXPATH+4);
        }
        /* to specify that we want to find all files */
    }
    else {
        /* could be a file name?  or a path/filename?  */
        /* or a path with wildcards?                   */
        /* we can't handle the wildcards yet.          */

        /* look for a directory separator or drive indicator */
        for (i = strlen(findpath[zxpn]); i >= 0; i--) {
            if (ISDIRSEP(findpath[zxpn][i]) ||
                 (i == 1 && findpath[zxpn][i] == ':'))
            {
                /* we found a directory separator or drive indicator. */
                /* are we at the last character in the string? */
                if (!findpath[zxpn][i+1])
                    ckstrncat(localfn[zxpn], "*",MAXPATH+4); /* yes, we want all files */
                else
                    findpath[zxpn][i+1] = '\0'; /* no, isolate the path */
                break;
            }
        }

        /* if we didn't find a path.  assume its just a filename */
        /* so prepend the current directory to the local filename */
        if (i < 0 && !ISDIRSEP(localfn[zxpn][0]) && localfn[zxpn][1] != ':') {
            findpath[zxpn][0] = '\0';
            ckstrncpy(nambuf, "./",MAXNAMLEN+4);
            ckstrncat(nambuf, localfn[zxpn],MAXPATH+4);
            ckstrncpy(localfn[zxpn], nambuf,MAXPATH+4);
        }
    }

    /* Now that we know the path and the filename we need to look for */
    /* patterns in the file name.                                     */
    for (i = strlen(localfn[zxpn]); i >= 0; i--) {
        if (ISDIRSEP(localfn[zxpn][i]) || localfn[zxpn][i] == ':') {

            /* NTFS supports streams which are indicated by
             *
             *    filename:stream
             */

            if (i > 1 && localfn[zxpn][i] == ':') {
                /* Assume we have a stream only if the ':' is found
                 * beyond the traditional drive indicator.  We copy
                 * the ':' because it makes it easier to append later on.
                 */
                ckstrncpy(stream[zxpn],localfn[zxpn]+i,MAXPATH);
                localfn[zxpn][i] = NUL;
                continue;
            }

            /* Save the file specification */
            ckstrncpy(findfspec[zxpn],localfn[zxpn]+i+1,MAXPATH+4);

            /* Now create the file specification with {} and [] replaced */
            /* with * and ? respectively.                                */
            for ( j=0,k=0,len=strlen(findfspec[zxpn]),brace=0,bracket=0;
                  j<=len ; j++)
            {
                /* copy everything including the trailing NUL */
                switch( findfspec[zxpn][j] ) {
                case '{':
                    if ( !brace && !bracket ) {
                        findfspec2[zxpn][k++] = '*';
                    }
                    brace++;
                    break;
                case '}':
                    brace--;
                    break;
                case '[':
                    if ( !bracket && !brace ) {
                        findfspec2[zxpn][k++] = '?';
                    }
                    bracket++;
                    break;
                case ']':
                    bracket--;
                    break;
                default:
                    if ( !bracket && !brace ) {
                        findfspec2[zxpn][k++] = findfspec[zxpn][j];
                    }
                }
            }

            /* and then replace the local filename string with the modified */
            /* pattern if a set of braces or brackets were found. */
            if ( strcmp(findfspec[zxpn],findfspec2[zxpn]) )
                strcpy(localfn[zxpn]+i+1,findfspec2[zxpn]);
            else
                findfspec2[zxpn][0] = '\0';
            break;
        }
    }

    /* At this point we should have a path in findpath[zxpn] if only "." */
    /* If not, something has gone wrong above.                           */

    /* Is there a relative path? */
    if ( !isdir(findpath[zxpn]) ){ /* The path is not a valid directory */
        i = strlen(findpath[zxpn]);
        if (i) {
            for (i = strlen(findpath[zxpn]) ; i >= 0 ; i--) {
                if ( findpath[zxpn][i] == '*' ||
                     findpath[zxpn][i] == '?')
                    findpathwild[zxpn] = 1;

                if (ISDIRSEP(findpath[zxpn][i])) {
                    char saved = findpath[zxpn][i];
                    findpath[zxpn][i] = NUL;
                    if (isdir(findpath[zxpn])) {
                        strcpy(findpathrel[zxpn],&findpath[zxpn][i+1]);
                        break;
                    } else {
                        findpath[zxpn][i] = saved;
                    }
                } else if (findpath[zxpn][i] == ':') {
                    char saved = findpath[zxpn][i+1];
                    findpath[zxpn][i+1] = NUL;
                    if (isdir(findpath[zxpn])) {
                        findpath[zxpn][i+1] = saved;
                        strcpy(findpathrel[zxpn],&findpath[zxpn][i+1]);
                        findpath[zxpn][i+1] = NUL;
                        break;
                    } else {
                        findpath[zxpn][i+1] = saved;
                    }
                }
            }
            if (i < 0) {
                strcpy(findpathrel[zxpn],findpath[zxpn]);
                findpath[zxpn][0] = NUL;
            }
        }
    }

    /* Get the first entry: either file or directory */
  findfirstfile:
    debug(F100,"zxpand about to FindFirst","",0);
    if (os2findfirstfile(localfn[zxpn])) {

/* Count the entry if it meets the requirements of dironly and fileonly */

#ifdef NT
        if ( win95_8_3 )
            FileName = finddata[zxpn].cAlternateFileName;
        else
            FileName = finddata[zxpn].cFileName;
#else /* NT */
        FileName = finddata[zxpn].achName;
#endif /* NT */

        if ((!(os2dironly[zxpn] || os2fileonly[zxpn]) ||
             (os2dironly[zxpn] && os2findisdir()) ||
             (os2fileonly[zxpn] && !os2findisdir())) &&
            (
#ifdef DOTDIRS
              (findlevel[zxpn] == 0) ||
              !os2recursive[zxpn] ||
#endif /* DOTDIRS */
             (strcmp(".",FileName) && strcmp("..",FileName)) ))
        {
            if ( !findfspec2[zxpn][0] ||
                 ckmatch(findfspec[zxpn], FileName, 0, 1) ) 
            {
                fcount[zxpn]++;
#ifdef STREAMS
                if ( stream[zxpn][0] )
                    fcountstream[zxpn] += StreamCount(findpath[zxpn],findpathrel[zxpn],
                                                      FileName, stream[zxpn]);
#endif /* STREAMS */
            } else
                debug(F111,"zxpand rejected",FileName,fcount[zxpn]);
        }
        else
            debug(F111,"zxpand rejected",FileName,fcount[zxpn]);

        /* Findnextfile: */
        while (os2findnextfile()) {
#ifdef NT
            if ( win95_8_3 )
                FileName = finddata[zxpn].cAlternateFileName;
            else
                FileName = finddata[zxpn].cFileName;
#else /* NT */
            FileName = finddata[zxpn].achName;
#endif /* NT */

            if ((!(os2dironly[zxpn] || os2fileonly[zxpn]) ||
                 (os2dironly[zxpn] && os2findisdir()) ||
                 (os2fileonly[zxpn] && !os2findisdir())) &&
                (
#ifdef DOTDIRS
                  (findlevel[zxpn] == 0) ||
                  !os2recursive[zxpn] ||
#endif /* DOTDIRS */
                 (strcmp(".",FileName) && strcmp("..",FileName)) ))
            {
                if ( !findfspec2[zxpn][0] ||
                     ckmatch(findfspec[zxpn],FileName,0, 1) )
                {
                    fcount[zxpn]++;
#ifdef STREAMS
                    if ( stream[zxpn][0] )
                        fcountstream[zxpn] += StreamCount(findpath[zxpn],findpathrel[zxpn],
                                                      FileName, stream[zxpn]);
#endif /* STREAMS */
                } else
                    debug(F111,"zxpand rejected",FileName,fcount[zxpn]);
            }
            else
                debug(F111,"zxpand rejected",FileName,fcount[zxpn]);
        }
    }
#ifdef RECURSIVE
    if (os2recursive[zxpn]) {
/*
  Done expanding the files for the current directory,
  now look for subdirectories.
 */
        os2findclose();
        if (findpath[zxpn][0])
          ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
        else
          ckstrncpy(localfn[zxpn],"./",MAXPATH+4);
        ckstrncat(localfn[zxpn],"*",MAXPATH+4);

        if (os2findfirstfile(localfn[zxpn])) {
        do {
#ifdef NT
            if ( win95_8_3 )
                FileName = finddata[zxpn].cAlternateFileName;
            else
                FileName = finddata[zxpn].cFileName;
#else /* NT */
            FileName = finddata[zxpn].achName;
#endif /* NT */
            if (os2findisdir() && strcmp(".",FileName) && strcmp("..",FileName))
            {
                if (!os2findpush()) {
                    fcntsav[zxpn] = fcount[zxpn] = 
                        fcountstream[zxpn] =
                            fcntstreamsav[zxpn] = 0;
                    return(fcount[zxpn]);
                } else {
                    debug(F100,"","",0);
                    goto findfirstfile;
                }
            }
        } while (os2findnextfile());
                }

      findnextdir:
        if (findlevel[zxpn] > 0) {
            os2findclose();
            os2findpop();
            while (os2findnextfile()) {
                if (os2findisdir()) {
                    if (!os2findpush()) {
                        fcntsav[zxpn] = fcount[zxpn] = 
                            fcountstream[zxpn] =
                                fcntstreamsav[zxpn] = 0;
                        return(fcount[zxpn]);
                    } else {
                        debug(F100,"","",0);
                        goto findfirstfile;
                    }
                }
            }
            goto findnextdir;
        }
    }
#endif /* RECURSIVE */

#ifdef NT
    lasterror[zxpn] = GetLastError();
#endif /* NT */
    os2findclose();

    if (lasterror[zxpn] != ERROR_NO_MORE_FILES) {
        fcntsav[zxpn] = fcount[zxpn] = 
            fcountstream[zxpn] =
                fcntstreamsav[zxpn] = 0;
        return(fcount[zxpn]);
    }
    debug(F111,"zxpand","fcount[zxpn]",fcount[zxpn]);
    fcntsav[zxpn] = fcount[zxpn];
    fcntstreamsav[zxpn] = fcountstream[zxpn];
    strcpy(rewindfn[zxpn],localfn[zxpn]);
    strcpy(rewindpath[zxpn],findpath[zxpn]);
    strcpy(rewindpathrel[zxpn],findpathrel[zxpn]);
    strcpy(rewindfspec[zxpn],findfspec[zxpn]);

    /* Before we leave fix findpath[] so that it does not contain    */
    /* extraneous information such as drive letter or . and .. paths */
    strcpy(nambuf,findpath[zxpn]);
    i = 0;
    len = strlen(nambuf);
    for ( j=0; i <= len ; i++,j++ ) {
        while ((i == 0 || nambuf[i-1] == ':' || ISDIRSEP(nambuf[i-1])) &&
                nambuf[i] == '.' && ISDIRSEP(nambuf[i+1]))
            i+=2;
        findpath[zxpn][j] = nambuf[i];
    }

    zxrewind(); /* places its return value into fcount[] */
    return(fcount[zxpn]>0?fcount[zxpn]:0);
}

/*  Z N E X T  --  Get name of next file from list created by zxpand(). */
/*
   Returns > 0 if there's another file, with its name copied into the argument
   string, or 0 if no more files in list.
*/
int
znext(fn) char *fn; {
    extern int zxpn;
    char * fn_file = NULL;
    CHAR * FileName;

    debug(F111,"znext","fcount[zxpn]",fcount[zxpn]);

    if ( fn == NULL )
        return(-1);
    fn[0] = '\0';               /* initialize the buffer in case of error */

  findnextfile3:
    if ( findlevel[zxpn] < 0)
        return(-1);

    if (fcount[zxpn]-- > 0
#ifdef NT
        && findhandle[zxpn][findlevel[zxpn]] != INVALID_HANDLE_VALUE
#else
        && findhandle[zxpn][findlevel[zxpn]] != HDIR_CREATE
#endif /* NT */
        ) {
        fcountstream[zxpn]--;

      findnextfile2:
        if ( deblog && fn[0] ) {
            debug(F111,"znext rejected",fn,fcount[zxpn]);
        }
        ckstrncpy(fn,findpath[zxpn],CKMAXPATH);

        if (os2recursive[zxpn])
            ckstrncat(fn,findpathrel[zxpn],CKMAXPATH);

#ifdef NT
        if ( win95_8_3 )
            FileName = finddata[zxpn].cAlternateFileName;
        else
            FileName = finddata[zxpn].cFileName;
#else /* NT */
        FileName = finddata[zxpn].achName;
#endif /* NT */
        while (os2dironly[zxpn] && !os2findisdir() ||
               ((
#ifdef DOTDIRS
                 os2recursive[zxpn] &&
                 (findlevel[zxpn] > 0) &&
#endif /* DOTDIRS */
                 os2findisdir() &&
#ifdef NT
                 (!strcmp(".",finddata[zxpn].cFileName) ||
                  !strcmp("..",finddata[zxpn].cFileName))
#else /* NT */
                 (!strcmp(".",finddata[zxpn].achName) ||
                  !strcmp("..",finddata[zxpn].achName))
#endif /* NT */
                 )) ||
               os2fileonly[zxpn] && os2findisdir()
               ) {
            /* The current file does not meet the necessary criteria */
            /* therefore we must look at the next file.              */
#ifdef NT
            if ( win95_8_3 )
                FileName = finddata[zxpn].cAlternateFileName;
            else
                FileName = finddata[zxpn].cFileName;
#else /* NT */
            FileName = finddata[zxpn].achName;
#endif /* NT */
            debug(F111,"znext rejected",FileName,fcount[zxpn]);

            if (!os2findnextfile()) {
                /* The next file does not exist */
#ifdef RECURSIVE
                if (os2recursive[zxpn]) {
                    /* However, we are looking at subdirectories   */
                    /* so we must now switch to the next directory */
                    os2findclose();

                  findnextsubdir:
                    /* prepare to list all subdirectories */
                    if (findpath[zxpn][0])
                      ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
                    else
                      ckstrncpy(localfn[zxpn],"./",MAXPATH+4);
                    ckstrncat(localfn[zxpn],"*",MAXPATH+4);

                    /* Find the first potential directory */
                    if (os2findfirstfile(localfn[zxpn]) > 0)
                    do {
#ifdef NT
                        if ( win95_8_3 )
                            FileName = finddata[zxpn].cAlternateFileName;
                        else
                            FileName = finddata[zxpn].cFileName;
#else /* NT */
                        FileName = finddata[zxpn].achName;
#endif /* NT */
                        if (os2findisdir() &&
                            strcmp(".",FileName) &&
                            strcmp("..",FileName)
                            ) {
                            /* we have a directory that is not . or .. */
                            if (!os2findpush()) {
                                debug(F111,"znext bad 1",fn,fcount[zxpn]+1);
                                fcount[zxpn] = -1;
                                fcountstream[zxpn] = -1;
                                return(fcount[zxpn]);
                            }
                            if (os2findfirstfile(localfn[zxpn])>0) {
                                goto findnextfile2;
                            } else {
                                goto findnextsubdir;
                            }
                        }
                    } while ( os2findnextfile() );

                  findnextdir:
                    /* There are no more subdirectories of the current dir */
                    os2findclose();
                    if (findlevel[zxpn] > 0) {
                        os2findpop();
                        while (os2findnextfile()) {
                            if (os2findisdir()) {
                                if (!os2findpush()) {
                                    debug(F111,"znext bad 2",fn,fcount[zxpn]+1);
                                    fcount[zxpn] = -1;
                                    fcountstream[zxpn] = -1;
                                    return(fcount[zxpn]);
                                }
                                if (os2findfirstfile(localfn[zxpn])>0) {
                                    goto findnextfile2;
                                } else {
                                    goto findnextsubdir;
                                }
                            }
                        }
                        goto findnextdir;
                    }
                } else
#endif /* RECURSIVE */
                {
                    fcount[zxpn] = -1;
                    fcountstream[zxpn] = -1;
                    debug(F111,"znext bad 3",fn,fcount[zxpn]+1);
                    return(0);
                }
            }
#ifdef NT
            if ( win95_8_3 )
                FileName = finddata[zxpn].cAlternateFileName;
            else
                FileName = finddata[zxpn].cFileName;
#else /* NT */
            FileName = finddata[zxpn].achName;
#endif /* NT */
        }

        /* Later when we call ckmatch() we need to pass only the filename */
        /* and not the entire path, so remember where the filename is now */
        fn_file = fn + strlen(fn);
#ifdef NT
        if ( win95_8_3 )
            FileName = finddata[zxpn].cAlternateFileName;
        else
            FileName = finddata[zxpn].cFileName;
#else /* NT */
        FileName = finddata[zxpn].achName;
#endif /* NT */
        ckstrncat(fn,FileName,CKMAXPATH);
        if ( stream[0] )
            ckstrncat(fn,stream[zxpn],CKMAXPATH);

        if (!os2findnextfile()) {
            /* The next file does not exist */
#ifdef RECURSIVE
            if (os2recursive[zxpn]) {
                /* However, we are looking at subdirectories   */
                /* so we must now switch to the next directory */
                os2findclose();

              findnextsubdir2:
                /* prepare to list all subdirectories */
                if (findpath[zxpn][0])
                  ckstrncpy(localfn[zxpn],findpath[zxpn],MAXPATH+4);
                else
                  ckstrncpy(localfn[zxpn],"./",MAXPATH+4);
                ckstrncat(localfn[zxpn],"*",MAXPATH+4);

                /* Find the first potential directory */
                if (os2findfirstfile(localfn[zxpn])>0)
                  do {
#ifdef NT
                      if ( win95_8_3 )
                          FileName = finddata[zxpn].cAlternateFileName;
                      else
                          FileName = finddata[zxpn].cFileName;
#else /* NT */
                      FileName = finddata[zxpn].achName;
#endif /* NT */
                      if ( os2findisdir() &&
                          strcmp(".",FileName) &&
                          strcmp("..",FileName)
                          ) {
                          /* we have a directory that is not . or .. */
                          if (!os2findpush()) {
                              debug(F111,"znext bad 4",fn,fcount[zxpn]+1);
                              fcount[zxpn] = -1;
                              fcountstream[zxpn] = -1;
                              return(fcount[zxpn]);
                          }
                          if(os2findfirstfile(localfn[zxpn])>0) {
                              goto findnextdone;
                          } else {
                              goto findnextsubdir2;
                          }
                      }
                  } while ( os2findnextfile() );

              findnextdir2:
                /* There are no more subdirectories of the current dir */
                os2findclose();
                if (findlevel[zxpn] > 0) {
                    os2findpop();
                    while (os2findnextfile()) {
                        if (os2findisdir()) {
                            if (!os2findpush()) {
                                debug(F111,"znext bad 5",fn,fcount[zxpn]+1);
                                fcount[zxpn] = -1;
                                fcountstream[zxpn] = -1;
                                return(fcount[zxpn]);
                            }
                            if (os2findfirstfile(localfn[zxpn])>0) {
                                goto findnextdone;
                            } else {
                                goto findnextsubdir2;
                            }
                        }
                    }
                    goto findnextdir2;
                }
              findnextdone:;
            }
#endif /* RECURSIVE */
        }
    } else {
        *fn = '\0';
        fcount[zxpn] = -1;
        fcountstream[zxpn] = -1;
    }

    /* if the current fn is not in the matchlist */
    /* then try the next file name until we find */
    /* the next match.                           */
    if ( *fn && findfspec2[zxpn][0] &&
         !ckmatch(findfspec[zxpn], fn_file, 0, 1) ) {
#ifdef NT
        if ( win95_8_3 )
            FileName = finddata[zxpn].cAlternateFileName;
        else
            FileName = finddata[zxpn].cFileName;
#else /* NT */
        FileName = finddata[zxpn].achName;
#endif /* NT */
        debug(F111,"znext rejected",FileName,fcount[zxpn]);
        fcount[zxpn]++;                 /* we didn't really find the file */
#ifdef STREAMS
        if ( stream[zxpn][0] )
            fcountstream[zxpn] += StreamCount(findpath[zxpn],findpathrel[zxpn],
                                               FileName, stream[zxpn]);
#endif /* STREAMS */
        goto findnextfile3;
    }

#ifdef NT
    if ( win95_8_3 ) {
        char name8_3[CKMAXPATH+1];
        DWORD len;

        len = GetShortPathName(fn,name8_3,CKMAXPATH+1);
        if ( len > 0 && len <= CKMAXPATH )
            ckstrncpy(fn,name8_3,CKMAXPATH+1);
    }
#endif /* NT */

    debug(F111,"znext",fn,fcount[zxpn]+1);
    return(fcount[zxpn]);
}


/*  Z C H K S P A  --  Check if there is enough space to store the file  */

/*
 Call with file specification f, size n in bytes.
 Returns -1 on error, 0 if not enough space, 1 if enough space.
*/
int
zchkspa(char *f, CK_OFF_T n)
{
/* OS/2 gives us an easy way to do this. */
    unsigned long x, filesize = 0L;
    debug(F111,"zchkspa",f,n);
    if (isalpha(f[0]) && f[1] == ':') {
        x = zdskspace(toupper(f[0]) - 'A' + 1);
        debug(F111,"zchkspa disk","size",x);
    } else {
        x = zdskspace(0);
        debug(F111,"zchkspa no disk","size",x);
    }
    if ( x > 0 && x < 1024 )    /* zdskspace() unable to return space */
        return(1);

    if (x > n)                  /* plenty of space */
        return(1);

    if (fncact == XYFX_U || fncact == XYFX_X) { /* Update or Replace */
      filesize = zchki(f);
      return((x+filesize >= n) ? 1 : 0);
    }

    return(0);
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

  On systems which use 8.3 notation, or when SET MSKERMIT ... 
  is used we borrow the technique used in MS-DOS Kermit:
  "The idea is to pad out the main name part (8 chars) with ascii zeros and
   then change the last chars successively to a 1, 2, etc. until
   a unique name is found. All registers are preserved"
*/

VOID
znewn(fn,s) char *fn, **s; {
#define ZNEWNBL 255
#define ZNEWNMD 4
    static char buf[ZNEWNBL+1];
    char *bp, *xp, *yp;
    static char localfn[MAXPATH+1];
    char *tp=NULL, *zp=NULL, ch, temp[14];
    int len = 0, d = 0, n, t, i, j, k, power = 1;

    int max = MAXNAMLEN;                /* Maximum name length */

    if (max < 14) max = 14;             /* Make it reasonable */
    if (max > ZNEWNBL) max = ZNEWNBL;
    bp = buf;                           /* Buffer for building new name */
    yp = fn;
    while (*yp) {                       /* Copy old name into buffer */
        *bp++ = *yp++;
        if (len++ > ZNEWNBL) break;     /* ...up to buffer length */
    }
    *s = NULL;
    for (i = 1; i < ZNEWNMD + 1; i++) { /* Version numbers up to 10**i - 1 */
        power *= 10;                    /* Next power of 10 */
        j = max - len;                  /* Space left for version number */
        k = 3 + i;                      /* Space needed for it */
        if (j < k) {                    /* Make room if necessary */
            len -= (k - j);             /* Adjust length of filename */
            bp = buf + len;             /* Point to new end */
        }
        *bp++ = '*';                    /* Put a star on the end (UNIX) */
        *bp-- = '\0';                   /* Terminate with null */

        debug(F110,"znewn: about to expand",buf,0);
        n = nzxpand(buf,0);             /* Expand the resulting wild name */
                                        /* n is the number of matches */
        debug(F101,"znewn matches","",n);
        while (n-- > 0) {               /* Find any existing name.~n~ files */
            znext(localfn);
            xp = localfn;
            xp += len;                  /* Look for .~<n>~ at the end of it */
            if (*xp == '.' && *(xp+1) == '~') { /* Has a version number */
                t = atoi(xp+2);                 /* Get it */
                if (t > d) d = t;       /* Save d = highest version number */
            }
        }
        if (d < power-1) {              /* Less than maximum possible? */
            debug(F110,"znewn number ok",buf,0);
            sprintf(bp,".~%d~",d+1);    /* Yes, make "name.~<d+1>~" */
            *s = buf;                   /* Point to new name */
            ck_znewn = d+1;             /* Also make it available globally */
            break;                      /* Done, return it */
        }
    }
    if (*s == NULL) {
        debug(F110,"znewn: too many names",buf,0);
        sprintf(bp,".~xxxx~");          /* Too many, use xxxx. */
        ck_znewn = -1;                  /* Also make it available globally */
        *s = buf;
    }
    {
        char *p;
        p = (char *) malloc(ZNEWNBL+1); /* Get fully qualified name */
        if (p) {
            if (zfnqfp(buf, ZNEWNBL, p))
              ckstrncpy(buf,p,ZNEWNBL);
            free(p);
        }
    }
    if (!mskrename && IsFileNameValid(buf)) {
        debug(F110,"znewn: os2 filename valid",buf,0);
        return; /* HPFS */
    }

    /* otherwise make FAT 8.3 name */
    debug(F110,"znewn: os2 filename invalid",buf,0);
    xp = bp = buf;
    yp = fn;
    while (*yp) {                       /* Copy name into buf */
        ch = *bp++ = *yp++;
        if (ISDIRSEP(ch) || (ch == ':')) xp=bp;
    }
    *bp = '\0';

    if ( mskrename ) {
        zp = yp = xp;
        i = 1;
        while (*yp && (*yp != '.')) {
            yp++;
            if (++i<=8)
                zp=yp;
        }
        /* zp points to 8th character in name, or yp, whichever occurs first. */
        strcpy(temp,yp);                    /* Copy extension, if any */

        /* pad out with zeros and make the initial filename '1'. */
        while (zp != xp+8) {
			if ( zp == xp )
				*zp++='X';
            if ( zp < xp+7 ) 
                *zp++='0';
            else 
                *zp++='1';
        }
        strcpy(zp--,temp);                    /* Get the extension back */
        
        while (1) {
            n = nzxpand(buf,0);                 /* Expand the resulting wild name */
            debug(F101,"znewn: matches","",n);
            if (n == 0) {
                debug(F110,"znewn: os2 file name is MS-Kermit",buf,0);
                return;
            }

            for ( zp = xp+7; zp > xp; zp--) {
                if (*zp == '9') {
                    *zp = '0';
                } else {
                    (*zp)++;
                    break;
                }
            }
            if ( zp == xp ) {
                debug(F110,"znewn: os2 file name is MS-Kermit (overflow)",buf,0);
                return;
            }
        }
    } else {
        zp = yp = xp;
        i = 1;
        while (*yp && (*yp != '.')) {
            yp++;
            if (++i<=6)
                zp=yp;
        }
        /* zp points to 6th character in name, or yp, whichever occurs first. */
        strcpy(temp,yp);                    /* Copy extension, if any */

        while (zp != xp+8) {
            if ( zp < xp+5 ) *zp++='0';
            else *zp++='?';                 /* Pad out with wild cards */
        }
        strcpy(zp,temp);                    /* Get the extension back */
        debug(F110,"znewn: about to expand",buf,0);
        n = nzxpand(buf,0);                 /* Expand the resulting wild name */
        debug(F101,"znewn: matches","",n);
        d = 0;                              /* Index number */
        debug(F110,"znewn: temp",temp,0);
        while (znext(temp)) {
            if ( tp = strrchr( temp, '/' ) )
                tp++;
            else
                tp = temp;
            i = atoi(tp+5);
            debug(F111,"znewn: tp=atoi(tp+5)",tp,i);
            if (i > d) d = i;
            debug(F101,"znewn: d","",d);
        }
        sprintf(temp,"%03d",d+1);           /* Get the number into a string */
        ck_znewn = d+1;
        memcpy(xp+5, temp, 3);
        debug(F110,"znewn: os2 file name is FAT",buf,0);
    }
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
    char *p, *s;
    int x;
    int len;

    if (!old) old = "";
    if (!new) new = "";
    debug(F110,"zrename old",old,0);
    debug(F110,"zrename new",new,0);
    if (!*old) return(-1);
    if (!*new) return(-1);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest)
      return(-1);
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef CKROOT
    debug(F111,"zrename setroot",ckroot,ckrootset);
    if (ckrootset) {
        if (!zinroot(old)) {
            debug(F110,"zrename old: setroot violation",old,0);
            return(-1);
        }
        if (!zinroot(new)) {
            debug(F110,"zrename new: setroot violation",new,0);
            return(-1);
        }
    }
#endif /* CKROOT */

    p = NULL;

    if (isdir(new)) {
        char *q = NULL;

        x = strlen(new);
        len = strlen(new) + strlen(old) + 2;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,new,len);                  /* Directory part */
        if (!ISDIRSEP(*(new+x-1)))      /* Separator, if needed */
          ckstrncat(p,"/",len);
        zstrip(old,&q);                 /* Strip path part from old name */
        ckstrncat(p,q,len);                    /* Concatenate to new directory */
        debug(F110,"zrename dir",p,0);
    } else {
#ifdef DEBUG
        debug(F110,"zrename no dir",new,0);
#endif
        len = strlen(new) + 64;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,new,len);
    }
    s = p;

#ifdef IKSD
        if ( inserver && (!ENABLED(en_del)) ) {
            if ( zchki(s) > -1 ) {
                if (p) free(p);
                return(-1);
            }
        }
#endif /* IKSD */
    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2))
         ckstrncpy(s,UNCname(s),len);
    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2) )
        old = UNCname(old);
/*
  Atomic, preferred, uses a single system call, rename(), if available.
  OS/2 rename() returns nonzero, but not necessarily -1 (?), on failure.
*/
    x = rename(old,s);
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_FC && ckxlogging) {
        char buf[1024];
        fullname[0] = '\0';
        zfnqfp(old,CKMAXPATH,fullname);
        tmp2[0] = '\0';
        zfnqfp(s,CKMAXPATH,tmp2);
        if (x) {
            sprintf(buf,"file[] %s: rename to %s failed (%m)",fullname,tmp2);
            cksyslog(SYSLG_FC, 0, buf, NULL, NULL);
        } else {
            sprintf(buf,"file[] %s: renamed to %s ok", fullname, tmp2);
            cksyslog(SYSLG_FC, 1, buf, NULL, NULL);
        }
    }
#endif /* CKSYSLOG */

    if (p) free(p);
    return(x ? -1 : 0);
}

/*  Z C O P Y  --  Copy a file  */
/*
   Call with source and destination names.
   If destination name is the name of a directory, the source file is
   copied to that directory with the original name.
   Returns 0 on success, -1 on failure.
*/
int
zcopy(source,destination) char *source, *destination; {
    char *p = NULL, *s;
    int x;
    int len;
#ifdef NT
    BOOL bCancel = 0;
    static BOOL (WINAPI * p_CopyFileExA)(LPCSTR lpExistingFileName,
                                          LPCSTR lpNewFileName,
                                          LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
                                          LPVOID lpData OPTIONAL,
                                          LPBOOL pbCancel OPTIONAL,
                                          DWORD dwCopyFlags
                                          )=NULL;
#endif /* NT */

    if (!source) source = "";
    if (!destination) destination = "";

    debug(F110,"zcopy src arg",source,0);
    debug(F110,"zcopy dst arg",destination,0);

    if (!*source) return(-1);
    if (!*destination) return(-1);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest)
      return(-4);
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef CKROOT
    debug(F111,"zcopy setroot",ckroot,ckrootset);
    if (ckrootset) {
        if (!zinroot(source)) {
            debug(F110,"zcopy source: setroot violation",source,0);
            return(-1);
        }
        if (!zinroot(destination)) {
            debug(F110,"zcopy destination: setroot violation",destination,0);
            return(-1);
        }
    }
#endif /* CKROOT */

    s = destination;

    if (isdir(destination)) {
        char *q = NULL;

        x = strlen(destination);
        len = strlen(destination) + strlen(source) + 2;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,destination,len);          /* Directory part */
        if (!ISDIRSEP(*(destination+x-1))) /* Separator, if needed */
          ckstrncat(p,"/",len);
        zstrip(source,&q);              /* Strip path part from old name */
        ckstrncat(p,q,len);                    /* Concatenate to new directory */
        debug(F110,"zcopy dir",p,0);
    } else {
        debug(F110,"zcopy no dir",destination,0);
        len = strlen(destination) + 64;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,destination,len);
    }

    s = p;

    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2) )
         ckstrncpy(s,UNCname(s),len);
    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2) )
        source = UNCname(source);

#ifdef IKSD
    if ( inserver && (!ENABLED(en_del)) ) {
        if ( zchki(s) > -1 ) {
            if (p) free(p);
            return(-4);
        }
    }
#endif /* IKSD */

#ifndef NT
    x = (DosCopy( source, s, DCPY_FAILEAS ) ? -1 : 0);
#else /* NT */
    if ( !p_CopyFileExA ) {
        if (hKernel == INVALID_HANDLE_VALUE)
            hKernel = LoadLibrary("kernel32.dll");
        if (hKernel != INVALID_HANDLE_VALUE)
            (FARPROC) p_CopyFileExA =
                GetProcAddress( hKernel, "CopyFileExA" );
    }

    if ( p_CopyFileExA ) {
        x = (p_CopyFileExA( source, s, NULL, NULL, &bCancel, 0) ? 0 : -1);
        if (x<0) {
            DWORD lasterror = GetLastError();
            debug(F111,"CopyFileEx failed to",s,lasterror);
            if (lasterror == ERROR_CALL_NOT_IMPLEMENTED)
                goto copyfile;
        }
    } else {
      copyfile:
        x = (CopyFile( source, s, FALSE ) ? 0 : -1);
        if (x<0)
            debug(F111,"CopyFile failed to",s,GetLastError());
    }
#endif /* NT */

#ifdef CKSYSLOG
    if (x > -1 && ckxsyslog >= SYSLG_FC && ckxlogging) {
        char buf[1024];
        if (x) {
            sprintf(buf,"file[] %s: copy to %s failed", fullname, tmp2);
            cksyslog(SYSLG_FC, 0, buf, NULL, NULL);
        } else {
            sprintf(buf,"file[] %s: copy to %s ok", fullname, tmp2);
            cksyslog(SYSLG_FC, 1, buf, NULL, NULL);
        }
    }
#endif /* CKSYSLOG */

    if (p) free(p);
    return(x);
}

/*  Z L I N K  --  Link a file  */
/*
   Call with source and destination names.
   If destination name is the name of a directory, the source file is
   copied to that directory with the original name.
   Returns 0 on success, -1 on failure.
*/
int
zlink(source,destination) char *source, *destination; {
#ifdef OS2ONLY
    return -1;
#else /* OS2ONLY */
    char *p = NULL, *s;
    int x;
    int len;
#ifdef NT
    BOOL bCancel = 0;
#endif /* NT */
    static BOOL (WINAPI * p_CreateHardLinkA)(LPCSTR lpFileName,
                                             LPCSTR lpExistingFileName,
                                             LPSECURITY_ATTRIBUTES lpSecurityAttributes
                                             )=NULL;
    static HANDLE hKernel = INVALID_HANDLE_VALUE;
    
    if ( !p_CreateHardLinkA ) {
        if (hKernel == INVALID_HANDLE_VALUE)
            hKernel = LoadLibrary("kernel32.dll");
        if (hKernel != INVALID_HANDLE_VALUE)
            (FARPROC) p_CreateHardLinkA =
                GetProcAddress( hKernel, "CreateHardLinkA" );
    }

    if ( !p_CreateHardLinkA )
        return -1;

    if (!source) source = "";
    if (!destination) destination = "";

    debug(F110,"zlink src arg",source,0);
    debug(F110,"zlink dst arg",destination,0);

    if (!*source) return(-1);
    if (!*destination) return(-1);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest)
      return(-4);
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef CKROOT
    debug(F111,"zlink setroot",ckroot,ckrootset);
    if (ckrootset) {
        if (!zinroot(source)) {
            debug(F110,"zlink source: setroot violation",source,0);
            return(-1);
        }
        if (!zinroot(destination)) {
            debug(F110,"zlink destination: setroot violation",destination,0);
            return(-1);
        }
    }
#endif /* CKROOT */

    s = destination;

    if (isdir(destination)) {
        char *q = NULL;

        x = strlen(destination);
        len = strlen(destination) + strlen(source) + 2;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,destination,len);          /* Directory part */
        if (!ISDIRSEP(*(destination+x-1)))      /* Separator, if needed */
          ckstrncat(p,"/",len);
        zstrip(source,&q);                      /* Strip path part from old name */
        ckstrncat(p,q,len);                    /* Concatenate to new directory */
        debug(F110,"zlink dir",p,0);
    } else {
        debug(F110,"zlink no dir",destination,0);
        len = strlen(destination) + 64;
        if (!(p = malloc(len)))
          return(-1);
        ckstrncpy(p,destination,len);
    }

    s = p;

    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2) )
         ckstrncpy(s,UNCname(s),len);
    if ( !strncmp(s,"//",2) || !strncmp(s,"\\\\",2) )
        source = UNCname(source);

#ifdef IKSD
    if ( inserver && (!ENABLED(en_del)) ) {
        if ( zchki(s) > -1 ) {
            if (p) free(p);
            return(-4);
        }
    }
#endif /* IKSD */

    x = (p_CreateHardLinkA(s,source,NULL) ? 0 : -1 );

#ifdef CKSYSLOG
    if (x > -1 && ckxsyslog >= SYSLG_FC && ckxlogging) {
        char buf[1024];
        if (x) {
            sprintf(buf,"file[] %s: link to %s failed", fullname, tmp2);
            cksyslog(SYSLG_FC, 0, buf, NULL, NULL);
        } else {
            sprintf(buf,"file[] %s: link to %s ok", fullname, tmp2);
            cksyslog(SYSLG_FC, 1, buf, NULL, NULL);
        }
    }
#endif /* CKSYSLOG */

    if (p) free(p);
    return(x);
#endif /* OS2ONLY */
}

/*  Z S A T T R */
/*
 Fills in a Kermit file attribute structure for the file which is to be sent.
 Returns 0 on success with the structure filled in, or -1 on failure.
 If any string member is null, then it should be ignored.
 If any numeric member is -1, then it should be ignored.
*/
#ifdef CK_PERMS

#ifdef CK_GPERMS
#undef CK_GPERMS
#endif /* CK_GPERMS */

#ifdef S_IRUSR
#ifdef S_IWUSR
#ifdef S_IXUSR
#define CK_GPERMS
#endif /* S_IXUSR */
#endif /* S_IWUSR */
#endif /* S_IRUSR */

static char gperms[2];

#endif /* CK_GPERMS */

static char lperms[24];

#ifdef CK_PERMS
static char xlperms[24];

char *
zgperm(f) char *f; {
    int x; char *s = (char *)xlperms;
    struct stat buf;
    debug(F110,"zgperm",f,0);
    if (!f) return("----------");
    if (!*f) return("----------");

#ifdef CKROOT
    debug(F111,"zgperm setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(f)) {
        debug(F110,"zgperm setroot violation",f,0);
        return("----------");
    }
#endif /* CKROOT */

    x = os2stat(f,&buf);
    if (x < 0)
      return(NULL);
    sprintf(s,"%o",buf.st_mode);
    debug(F110,"zgperm",s,0);
    return(s);
}
#else
char *
zgperm(f) char *f; {
    return(NULL);
}
#endif /* CK_PERMS */

int
zsattr(xx) struct zattr *xx; {
    long k; int x;
    struct stat buf;

    k = iflen % 1024L;                  /* File length in K */
    if (k != 0L) k = 1L;
    xx->lengthk = (iflen / 1024L) + k;
    xx->type.len = 0;                   /* File type can't be filled in here */
    xx->type.val = "";
    if (*nambuf) {
        xx->date.val = zfcdat(nambuf);  /* File creation date */
        xx->date.len = (int)strlen(xx->date.val);
    } else {
        xx->date.len = 0;
        xx->date.val = "";
    }
    xx->creator.len = 0;                /* File creator */
    xx->creator.val = "";
    xx->account.len = 0;                /* File account */
    xx->account.val = "";
    xx->area.len = 0;                   /* File area */
    xx->area.val = "";
    xx->password.len = 0;               /* Area password */
    xx->password.val = "";
    xx->blksize = -1L;                  /* File blocksize */
    xx->xaccess.len = 0;                /* File access */
    xx->xaccess.val = "";
    xx->encoding.len = 0;               /* Transfer syntax */
    xx->encoding.val = 0;
    xx->disp.len = 0;                   /* Disposition upon arrival */
    xx->disp.val = "";
    xx->lprotect.len = 0;               /* Local protection */
    xx->lprotect.val = "";
    xx->gprotect.len = 0;               /* Generic protection */
    xx->gprotect.val = "";
    debug(F111,"zsattr lperms",xx->lprotect.val,xx->lprotect.len);
    debug(F111,"zsattr gperms",xx->gprotect.val,xx->gprotect.len);
#ifdef NT
    xx->systemid.val = "UN";            /* UN = Win32 */
#else
    xx->systemid.val = "UO";            /* UO = OS/2 */
#endif /* NT */
    xx->systemid.len = 2;               /* System ID */
    xx->recfm.len = 0;                  /* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;               /* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;                 /* Length */
    return(0);
}

/* Z F C D A T  --  Get file creation date */
/*
  Call with pointer to filename.
  On success, returns pointer to modification date in yyyymmdd hh:mm:ss format.
  On failure, returns pointer to null string.
*/
static char datbuf[40];

char *
#ifdef CK_ANSIC
zdtstr(time_t time)
#else
zdtstr(time) time_t time;
#endif /* CK_ANSIC */
/* zdtstr */ {
    struct tm * time_stamp;
    struct tm * localtime();
    int yy, ss;
    struct tm lts;

    debug(F101,"zdatstr time","",time);
    if (time < 0)
      return("");
    time_stamp = localtime(&(time));
    if (!time_stamp) {
        debug(F100,"localtime returns null","",0);
        return("");
    }
    memcpy(&lts,time_stamp,sizeof(struct tm));
    time_stamp = &lts;
    yy = time_stamp->tm_year;           /* Year - 1900 */
    yy += 1900;
    debug(F101,"zdatstr year","",yy);

    if (time_stamp->tm_mon  < 0 || time_stamp->tm_mon  > 11)
      return("");
    if (time_stamp->tm_mday < 0 || time_stamp->tm_mday > 31)
      return("");
    if (time_stamp->tm_hour < 0 || time_stamp->tm_hour > 23)
      return("");
    if (time_stamp->tm_min  < 0 || time_stamp->tm_min  > 59)
      return("");
    ss = time_stamp->tm_sec;            /* Seconds */
    if (ss < 0 || ss  > 59)             /* Some systems give a BIG number */
      ss = 0;
    sprintf(datbuf,
            "%04d%02d%02d %02d:%02d:%02d",
            yy,
            time_stamp->tm_mon + 1,
            time_stamp->tm_mday,
            time_stamp->tm_hour,
            time_stamp->tm_min
            , ss
            );
    yy = (int)strlen(datbuf);
    debug(F111,"zdatstr",datbuf,yy);
    if (yy > 17) datbuf[17] = '\0';
    return(datbuf);
}

char *
zfcdat(name) char *name; {
    struct stat buffer;

    datbuf[0] = '\0';

#ifdef CKROOT
    debug(F111,"zfcdat setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(name)) {
            debug(F110,"zfcdat setroot violation",name,0);
            return(0);
        }
#endif /* CKROOT */

    if (os2stat(name,&buffer) != 0) {
        debug(F110,"zfcdat stat failed",name,0);
        return("");
    }
    return(zdtstr(buffer.st_mtime));
}

static char zjdbuf[32] = { NUL, NUL };  /* Julian date buffer */

time_t
zstrdt(date,len) char * date; int len; {
/*
  To do: adapt code from OS-9 Kermit's ck9fio.c zstime function, which
  is more flexible, allowing [yy]yymmdd[ hh:mm[:ss]].
*/
    long tmx=0, days;
    int i, n, isleapyear;
                   /*       J  F  M  A   M   J   J   A   S   O   N   D   */
                   /*      31 28 31 30  31  30  31  31  30  31  30  31   */
    static
    int monthdays [13] = {  0,0,31,59,90,120,151,181,212,243,273,304,334 };
    char s[5];
    struct tm *time_stamp;

#ifdef NT
struct _utimbuf tp;
#else /* NT */
struct utimbuf tp;
#endif /* NT */

#ifdef ANYBSD
    long timezone = 0L;
    static struct timeb tbp;
#endif /* ANYBSD */

    debug(F111,"zstrdt",date,len);

    if ((len == 0)
        || (len != 17)
        || (date[8] != ' ')
        || (date[11] != ':')
        || (date[14] != ':') ) {
        debug(F111,"Bad creation date ",date,len);
        return(-1);
    }
    debug(F111,"zstrdt date check 1",date,len);
    for(i = 0; i < 8; i++) {
        if (!isdigit(date[i])) {
            debug(F111,"Bad creation date ",date,len);
            return(-1);
        }
    }
    debug(F111,"zstrdt date check 2",date,len);
    i++;

    for (; i < 16; i += 3) {
        if ((!isdigit(date[i])) || (!isdigit(date[i + 1]))) {
            debug(F111,"Bad creation date ",date,len);
            return(-1);
        }
    }
    debug(F111,"zstrdt date check 3",date,len);
    debug(F100,"zstrdt so far so good","",0);

    s[4] = '\0';
    for (i = 0; i < 4; i++)             /* Fix the year */
      s[i] = date[i];

    n = atoi(s);
    debug(F111,"zstrdt year",s,n);
    if (n < 1970) {
        debug(F100,"zstrdt fails - year","",n);
        return(-1);
    }

/*  Previous year's leap days.  This won't work after year 2100. */

    isleapyear = (( n % 4 == 0 && n % 100 !=0) || n % 400 == 0);
    days = (long) (n - 1970) * 365;
    days += (n - 1968 - 1) / 4 - (n - 1900 - 1) / 100 + (n - 1600 - 1) / 400;

    s[2] = '\0';

    for (i = 4; i < 16; i += 2) {
        s[0] = date[i];
        s[1] = date[i + 1];
        n = atoi(s);
        switch (i) {
          case 4:                       /* MM: month */
            if ((n < 1 ) || ( n > 12)) {
                debug(F111,"zstrdt 4 bad date ",date,len);
                return(-1);
            }
            days += monthdays [n];
            if (isleapyear && n > 2)
              ++days;
            continue;

          case 6:                       /* DD: day */
            if ((n < 1 ) || ( n > 31)) {
                debug(F111,"zstrdt 6 bad date ",date,len);
                return(-1);
            }
            tmx = (days + n - 1) * 24L * 60L * 60L;
            i++;                        /* Skip the space */
            continue;

          case 9:                       /* hh: hour */
            if ((n < 0 ) || ( n > 23)) {
                debug(F111,"zstrdt 9 bad date ",date,len);
                return(-1);
            }
            tmx += n * 60L * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 12:                      /* mm: minute */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zstrdt 12 bad date ",date,len);
                return(-1);
            }
            tmx += timezone;
            tmx += n * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 15:                      /* ss: second */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zstrdt 15 bad date ",date,len);
                return(-1);
            }
            tmx += n;
        }
        time_stamp = localtime(&tmx);
        debug(F101,"zstrdt tmx 1","",tmx);
        if (!time_stamp)
          return(-1);
        time_stamp = localtime(&tmx);
        debug(F101,"zstrdt tmx 2","",tmx);
        if (time_stamp->tm_isdst)
          tmx -= 60L * 60L;             /* Adjust for daylight savings time */
        n = time_stamp->tm_year;
        if (n < 300) n += 1900;
        sprintf(zjdbuf,"%04d%03d",n,(time_stamp->tm_yday)+1);
    }
    return(tmx);
}

#ifdef COMMENT
char *
zjdate(date) char * date; {
    int x;
    if (!date) date = "";
    x = strlen(date);
    if (x < 1) return("0");
    if (zstrdt(date,x) < 0)
      return("-1");
    else
      return((char *)zjdbuf);
}
#endif /* COMMENT */

#ifdef ZLOCALTIME
/* Z L O C A L T I M E  --  GMT/UTC time string to local time string */

/*
   Call with: "yyyymmdd hh:mm:ss" GMT/UTC date-time.
   Returns:   "yyyymmdd hh:mm:ss" local date-time on success, NULL on failure.
*/
static char zltimbuf[64];

char *
zlocaltime(gmtstring) char * gmtstring; {
    time_t tmx;
    long days;
    int i, n, x, isleapyear;
                   /*       J  F  M  A   M   J   J   A   S   O   N   D   */
                   /*      31 28 31 30  31  30  31  31  30  31  30  31   */
    static
    int monthdays [13] = {  0,0,31,59,90,120,151,181,212,243,273,304,334 };
    char s[5];
    struct tm *time_stamp;

    char * date = gmtstring;
    int len;

    len = strlen(date);
    debug(F111,"zlocaltime",date,len);

    if ((len == 0)
        || (len != 17)
        || (date[8] != ' ')
        || (date[11] != ':')
        || (date[14] != ':') ) {
        debug(F111,"Bad creation date ",date,len);
        return(NULL);
    }
    debug(F111,"zlocaltime date check 1",date,len);
    for(i = 0; i < 8; i++) {
        if (!isdigit(date[i])) {
            debug(F111,"Bad creation date ",date,len);
            return(NULL);
        }
    }
    debug(F111,"zlocaltime date check 2",date,len);
    i++;

    for (; i < 16; i += 3) {
        if ((!isdigit(date[i])) || (!isdigit(date[i + 1]))) {
            debug(F111,"Bad creation date ",date,len);
            return(NULL);
        }
    }
    debug(F111,"zlocaltime date check 3",date,len);

    debug(F100,"zlocaltime so far so good","",0);

    s[4] = '\0';
    for (i = 0; i < 4; i++)             /* Fix the year */
      s[i] = date[i];

    n = atoi(s);
    debug(F111,"zlocaltime year",s,n);
    if (n < 1970) {
        debug(F100,"zlocaltime fails - year","",n);
        return(NULL);
    }

/*  Previous year's leap days.  This won't work after year 2100. */

    isleapyear = (( n % 4 == 0 && n % 100 !=0) || n % 400 == 0);
    days = (long) (n - 1970) * 365;
    days += (n - 1968 - 1) / 4 - (n - 1900 - 1) / 100 + (n - 1600 - 1) / 400;

    s[2] = '\0';

    for (i = 4; i < 16; i += 2) {
        s[0] = date[i];
        s[1] = date[i + 1];
        n = atoi(s);
        switch (i) {
          case 4:                       /* MM: month */
            if ((n < 1 ) || ( n > 12)) {
                debug(F111,"zlocaltime 4 bad date ",date,len);
                return(NULL);
            }
            days += monthdays [n];
            if (isleapyear && n > 2)
              ++days;
            continue;

          case 6:                       /* DD: day */
            if ((n < 1 ) || ( n > 31)) {
                debug(F111,"zlocaltime 6 bad date ",date,len);
                return(NULL);
            }
            tmx = (days + n - 1) * 24L * 60L * 60L;
            i++;                        /* Skip the space */
            continue;

          case 9:                       /* hh: hour */
            if ((n < 0 ) || ( n > 23)) {
                debug(F111,"zlocaltime 9 bad date ",date,len);
                return(NULL);
            }
            tmx += n * 60L * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 12:                      /* mm: minute */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zlocaltime 12 bad date ",date,len);
                return(NULL);
            }
            tmx += n * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 15:                      /* ss: second */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zlocaltime 15 bad date ",date,len);
                return(NULL);
            }
            tmx += n;
        }

/*
  At this point tmx is the time_t representation of the argument date-time
  string without any timezone or DST adjustments.  Therefore it should be
  the same as the time_t representation of the GMT/UTC time.  Now we should
  be able to feed it to localtime() and have it converted to a struct tm
  representing the local time equivalent of the given UTC time.
*/
        time_stamp = localtime(&tmx);
        if (!time_stamp)
          return(NULL);
    }

/* Now we simply reformat the struct tm to a string */

    x = time_stamp->tm_year;
    if (time_stamp->tm_year < 70 || time_stamp->tm_year > 8099)
      return(NULL);
    if (time_stamp->tm_mon < 0 || time_stamp->tm_mon > 11)
      return(NULL);
    if (time_stamp->tm_mday < 1 || time_stamp->tm_mday > 31)
      return(NULL);
    if (time_stamp->tm_hour < 0 || time_stamp->tm_hour > 24)
      return(NULL);
    if (time_stamp->tm_min < 0 || time_stamp->tm_min > 60)
      return(NULL);
    if (time_stamp->tm_sec < 0 || time_stamp->tm_sec > 60)
      return(NULL);
    sprintf(zltimbuf,"%04d%02d%02d %02d:%02d:%02d",
            time_stamp->tm_year + 1900,
            time_stamp->tm_mon + 1,
            time_stamp->tm_mday,
            time_stamp->tm_hour,
            time_stamp->tm_min,
            time_stamp->tm_sec
            );
    return((char *)zltimbuf);
}
#endif /* ZLOCALTIME */

/* Z S T I M E  --  Set modification date/time+permissions for incoming file */
/*
 Call with:
 f  = pointer to name of existing file.
 yy = pointer to a Kermit file attribute structure in which yy->date.val
      is a date of the form yyyymmdd hh:mm:ss, e.g. 19900208 13:00:00.
      yy->lprotect.val & yy->gprotect.val are permission/protection values.
 x  = is a function code: 0 means to set the file's attributes as given.
      1 means compare the date in struct yy with the file creation date.
 Returns:
 -1 on any kind of error.
  0 if x is 0 and the attributes were set successfully.
  0 if x is 1 and date from attribute structure <= file creation date.
  1 if x is 1 and date from attribute structure > file creation date.
*/
int
zstime(f,yy,x) char *f; struct zattr *yy; int x;
/* zstime */ {
    int r = -1;                         /* Return code */
#ifdef CK_PERMS
    int setperms = 0;
#endif /* CK_PERMS */
    int setdate = 0;
    struct stat sb;
    struct utimbuf tp;
    long tm=0;

    debug(F110,"zstime",f,0);
    debug(F111,"zstime date",yy->date.val,yy->date.len);

#ifdef CKROOT
    debug(F111,"zstime setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(f)) {
            debug(F110,"zstime setroot violation",f,0);
            return(0);
        }
#endif /* CKROOT */

    if (os2stat(f,&sb)) {                       /* Get the time for the file */
        debug(F110,"zstime: Can't stat file:",f,0);
        return(-1);
    }

    if (yy->date.len == 0) {            /* No date in struct */
        if (yy->lprotect.len != 0) {    /* So go do permissions */
            goto zsperms;
        } else {
            debug(F100,"zstime: nothing to do","",0);
            return(0);
        }
    }
    if ((tm = zstrdt(yy->date.val,yy->date.len)) < 0) {
        debug(F101,"zstime: zstrdt fails","",0);
        return(-1);
    }
    debug(F101,"zstime: tm","",tm);
    debug(F111,"zstime: A-pkt date ok ",yy->date.val,yy->date.len);

    setdate = 1;

  zsperms:
#ifdef CK_PERMS
    {
        int i, x = 0, xx, flag = 0;
        char * s;
#ifdef DEBUG
        char obuf[24];
        if (deblog) {
            debug(F111,"zstime lperms",yy->lprotect.val,yy->lprotect.len);
            debug(F111,"zstime gperms",yy->gprotect.val,yy->gprotect.len);
            debug(F110,"zstime system id",yy->systemid.val,0);
            sprintf(obuf,"%o",sb.st_mode);
            debug(F110,"zstime file perms before",obuf,0);
        }
#endif /* DEBUG */
        if ((yy->lprotect.len > 0 &&    /* Have local-format permissions */
            yy->systemid.len > 0 &&     /* from A-packet... */
            0 ) || (yy->lprotect.len < 0) /* OR by inheritance from old file */
            ) {
            flag = 1;
            s = yy->lprotect.val;       /* UNIX filemode */
            xx = yy->lprotect.len;
            if (xx < 0)                 /* len < 0 means inheritance */
              xx = 0 - xx;
            for (i = 0; i < xx; i++) {  /* Decode octal string */
                if (*s <= '7' && *s >= '0') {
                    x = 8 * x + (int)(*s) - '0';
                } else {
                    flag = 0;
                    break;
                }
                s++;
            }
#ifdef DEBUG
            sprintf(obuf,"%o",x);
            debug(F110,"zstime octal lperm",obuf,0);
#endif /* DEBUG */
        } else if (!flag && yy->gprotect.len > 0) {
            int g;
            g = xunchar(*(yy->gprotect.val));
            debug(F101,"zstime gprotect","",g);
#ifdef S_IRUSR
            debug(F100,"zstime S_IRUSR","",0);
            if (g & 1) x |= S_IRUSR;    /* Read permission */
            flag = 1;
#endif /* S_IRUSR */
#ifdef S_IWUSR
            debug(F100,"zstime S_IWUSR","",0);
            if (g & 2) x |= S_IWUSR;    /* Write permission */
            if (g & 16) x |= S_IWUSR;   /* Delete permission */
            flag = 1;
#endif /* S_IWUSR */
#ifdef S_IXUSR
            debug(F100,"zstime S_IXUSR","",0);
            if (g & 4) x |= S_IXUSR;
            flag = 1;
#endif /* S_IXUSR */
        }
        if (flag) {
#ifdef S_IFMT
            sb.st_mode = (sb.st_mode & S_IFMT) | x;
            setperms = 1;
#else
#ifdef _IFMT
            sb.st_mode = (sb.st_mode & _IFMT) | x;
            setperms = 1;
#endif /* _IFMT */
#endif /* S_IFMT */
        }
#ifdef DEBUG
        sprintf(obuf,"%o",sb.st_mode);
        debug(F110,"zstime file perms after",obuf,0);
#endif /* DEBUG */
    }
#endif /* CK_PERMS */

    debug(F101,"zstime: sb.st_atime","",sb.st_atime);

    tp.modtime = tm;                    /* Set modif. time to creation date */
    tp.actime = sb.st_atime;            /* Don't change the access time */

    switch (x) {                        /* Execute desired function */
      case 0:                           /* Set the creation date of the file */
#ifdef CK_PERMS                         /* And permissions */
/*
  NOTE: If we are inheriting permissions from a previous file, and the
  previous file was a directory, this would turn the new file into a directory
  too, but it's not, so we try to unset the right bit.  Luckily, this code
  will probably never be executed since the upper level modules do not allow
  reception of a file that has the same name as a directory.
*/
        {
            int x;
            debug(F101,"zstime setperms","",setperms);
            if (S_ISDIR(sb.st_mode)) {
                debug(F101,"zstime DIRECTORY bit on","",sb.st_mode);
                sb.st_mode ^= 0040000;
                debug(F101,"zstime DIRECTORY bit off","",sb.st_mode);
            }
            if (setperms) {
                x = chmod(f,sb.st_mode);
                debug(F101,"zstime chmod","",x);
            }
        }
        if (x < 0) return(-1);
#endif /* CK_PERMS */

        if (!setdate)                   /* We don't have a date */
          return(0);                    /* so skip the following... */

        if (utime(f,&tp)) {             /* Fix modification time */
            debug(F110,"zstime 0: can't set modtime for file",f,0);
            r = -1;
        } else  {
            debug(F110,"zstime 0: modtime set for file",f,0);
            r = 0;
        }
        break;

      case 1:                           /* Compare the dates */
/*
  This was st_atime, which was wrong.  We want the file-data modification
  time, st_mtime.
*/
        debug(F111,"zstime 1: compare",f,sb.st_mtime);
        debug(F111,"zstime 1: compare","packet",tm);

/*
  In OS/2, sb.st_mtime, at least on a FAT file system, is always even.
  In that case, if the incoming file is only one second newer than the
  local file, consider them the same (yuk).
*/
#ifdef COMMENT
          /* In IBM C 3.6 time_t is of type double.  This can't be done */
        if ((sb.st_mtime & 1) == 0)
#endif /* COMMENT */
          if ((tm - sb.st_mtime) == 1)
            tm--;
        r = (sb.st_mtime < tm) ? 0 : 1;
        break;

      default:                          /* Error */
        r = -1;
    }
    return(r);
}

/* Find initialization file. */

#ifndef NOFRILLS
int
zmail(p,f) char *p; char *f; {          /* Send file f as mail to address p */
/*
  Returns 0 on success
   2 if mail delivered but temp file can't be deleted
  -2 if mail can't be delivered
  The UNIX version always returns 0 because it can't get a good return
  code from zsyscmd.
*/

#ifdef CK_LOGIN
    if (isguest)
      return(-2);
#endif /* CK_LOGIN */

    if (!f) f = "";
    if (!*f) return(-1);

#ifdef CKROOT
    debug(F111,"zmail setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(f)) {
            debug(F110,"zmail setroot violation",f,0);
            return(-1);
        }
#endif /* CKROOT */

/* The idea is to use /usr/ucb/mail, rather than regular mail, so that   */
/* a subject line can be included with -s.  Since we can't depend on the */
/* user's path, we use the convention that /usr/ucb/Mail = /usr/ucb/mail */
/* and even if Mail has been moved to somewhere else, this should still  */
/* find it...  The search could be made more reliable by actually using  */
/* access() to see if /usr/ucb/Mail exists. */

/* Should also make some check on zmbuf overflow... */

    sprintf(zmbuf,"Mail -s %c%s%c %s < %s", '"', f, '"', p, f);
    zsyscmd(zmbuf);
    return(-1);
}
#endif /* NOFRILLS */

#ifndef NOFRILLS
#ifdef NT
_PROTOTYP(int Win32PrtFile,( char *, char *));
#endif /* NT */


int
zprint(p,f) char *p; char *f; {         /* Print file f with options p */
    extern char * printername;          /* From ckuus3.c */
    extern int printpipe;

    debug(F110,"zprint file",f,0);
    debug(F110,"zprint flags",p,0);
    debug(F110,"zprint printername",printername,0);
    debug(F101,"zprint printpipe","",printpipe);

#ifdef CK_LOGIN
    if (isguest)
      return(-2);
#endif /* CK_LOGIN */

    if (!f) f = "";
    if (!*f) return(-1);

#ifdef CKROOT
    debug(F111,"zprint setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(f)) {
            debug(F110,"zprint setroot violation",f,0);
            return(-1);
        }
#endif /* CKROOT */

    prtfile(f);
    *zmbuf = '\0';
    return(0);
}
#endif /* NOFRILLS */

char *
whoami()
{
    static char realname[MAXPATH];
#ifdef NT
    DWORD len = MAXPATH;
    GetUserName( realname, &len );
#else /* NT */
    char * env = getenv("USER");
    if (env)
        ckstrncpy(realname,env,MAXPATH);
    else
        strcpy(realname,"os2user");
#endif /* NT */
    return realname;
}

/*  T I L D E _ E X P A N D  --  expand ~user to the user's home directory. */

char *
tilde_expand(dirname) char *dirname; {
    return(NULL);
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
#ifndef NOPUSH
    extern int vmode;

/*
  We must set the priority back to normal.  Otherwise all of children
  processes are going to inherit our FOREGROUNDSERVER priority and that
  would not be good for the system or ourselves.
*/
   if (!priv_chk()) {
       ULONG rc = 0;
#ifndef KUI
       char title[80];
       title[0] = '\0';
       os2gettitle( title, 80 );
       msleep(100);
       RequestScreenMutex(SEM_INDEFINITE_WAIT);
       KbdHandlerCleanup();
#endif /* KUI */
       ResetThreadPrty();
       rc = system(s);
       SetThreadPrty(priority,15);
#ifndef KUI
       KbdHandlerInit();
       /* Reset mode and buffering for stdio */
       if ( k95stdin ) {
#ifdef NT
           _setmode(_fileno(stdin),_O_BINARY);
#else
           setmode(fileno(stdin),_O_BINARY);
#endif /* NT */
       }
       if ( k95stdout ) {
#ifdef NT
           _setmode(_fileno(stdout),_O_TEXT);
#else
           setmode(fileno(stdout),_O_TEXT);
#endif /* NT */
           setbuf(stdout,NULL);
       }
       if (
#ifndef NOSPL
        cmdlvl == 0
#else
        tlevel < 0
#endif /* NOSPL */
        )
           OS2WaitForKey();
       ReleaseScreenMutex();
       VscrnForceFullUpdate();
       VscrnIsDirty(vmode);
       os2settitle(title, FALSE);
#endif /* KUI */
       return(rc);
   }
   return(0);
#else /* NOPUSH */
    return(-1);
#endif /* NOPUSH */
}

/*
  Original UNIX code by H. Fischer; copyright rights assigned to Columbia U.
  Adapted to use getpwuid to find login shell because many systems do not
  have SHELL in environment, and to use direct calling of shell rather
  than intermediate system() call. -- H. Fischer (1985); many changes since
  then.  Call with s pointing to command to execute.
*/

int
_zshcmd(s,wait) char *s; int wait; {
#ifndef NOPUSH
    PID_T pid;
    int rc;
    char title[80];
#ifdef NT
    SIGTYP (* savint)(int);
#endif /* NT */
    char *shell = getenv("SHELL");
    extern int vmode;

    if ( !shell )
       shell = getenv("COMSPEC");

    pexitstat = -3;                     /* Initialize process exit status */

    if (!priv_chk()) {
#ifndef KUI
        os2gettitle( title, 80 );
        msleep(100);
        RequestScreenMutex(SEM_INDEFINITE_WAIT);
        KbdHandlerCleanup();
#endif /* KUI */

        ResetThreadPrty();
#ifdef NT
        savint = signal( SIGINT, SIG_IGN );
#endif /* NT */

        if (!s || *s == '\0')
          pexitstat = system(shell);    /* was _spawnlp(P_WAIT, shell, NULL) */
        else
          pexitstat = system(s);
#ifdef NT
        signal( SIGINT, savint );
#endif /* NT */

        SetThreadPrty(priority,15);

#ifndef KUI
        KbdHandlerInit();
        if ( k95stdin ) {
#ifdef NT
            _setmode(_fileno(stdin),_O_BINARY);
#else
            setmode(fileno(stdin),_O_BINARY);
#endif /* NT */
        }
        if ( k95stdout ) {
#ifdef NT
            _setmode(_fileno(stdout),_O_TEXT);
#else
            setmode(fileno(stdout),_O_TEXT);
#endif /* NT */
            setbuf(stdout,NULL);
        }
        if ( wait && cmdsrc() == 0 )
            OS2WaitForKey();
        ReleaseScreenMutex();
        VscrnForceFullUpdate();
        VscrnIsDirty(vmode);
        os2settitle( title, FALSE );
#endif /* KUI */
    }
#else /* NOPUSH */
    pexitstat = 1;
#endif /* NOPUSH */
    return(pexitstat == 0);
}

int
zshcmd(s) char *s; {
    return(_zshcmd(s,1));
}

/*  I S W I L D  --  Check if filespec is "wild"  */

/*
  Returns 0 if it is a single file, 1 if it contains wildcard characters.
  Note: must match the algorithm used by match(), hence no [a-z], etc.
*/
int
iswild(filespec) char *filespec; {
    char c; int x; char *p;
    int quo = 0;

    while ((c = *filespec++) != '\0') {
        if (c == '\\' && quo == 0) {
            quo = 1;
            continue;
        }
        if (!quo && (c == '*' || c == '?'
#ifdef CKREGEX
                      || c == '[' || c == '{'
#endif /* CKREGEX */
                      ))
            return(1);
        quo = 0;
    }
    return(0);
}


/*
   Tell if string pointer s is the name of an existing directory.
   Returns 1 if directory, 0 if not a directory.
*/
int
isdir(s) char *s; {
#ifdef NT
    DWORD attrs;
#else /* NT */
    int x;
    struct stat statbuf;
#endif /* NT */

    if (!s) return(0);
    if (!*s) return(0);

    /* Disk letter like A: is top-level directory on a disk */
    if (((int)strlen(s) == 2) && (isalpha(*s)) && (*(s+1) == ':')) {
        return(1);
    }
#ifdef NT
    attrs = GetFileAttributes(s);
    
/* Visual C++ 6 doesn't know about this */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES -1
#endif
    
    if ( attrs == INVALID_FILE_ATTRIBUTES )
        return(0);
    return(attrs & FILE_ATTRIBUTE_DIRECTORY ? 1 : 0);

#else
    x = os2stat(s,&statbuf);
    debug(F111,"isdir stat",s,x);
    if (x == -1) {
        debug(F101,"isdir errno","",errno);
        return(0);
    } else {
        debug(F101,"isdir statbuf.st_mode","",statbuf.st_mode);
        return (S_ISDIR (statbuf.st_mode) ? 1 : 0);
    }
#endif
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
    0 or greater on success, i.e. the number of directories created.
   -1 on failure to create the directory
*/
int
zmkdir(path) char *path; {
    char *xp, *tp, c;
    int x, count = 0, i;

    if (!path) path = "";
    if (!*path) return(-1);

#ifdef CKROOT
    debug(F111,"zmkdir setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(path)) {
            debug(F110,"zmkdir setroot violation",path,0);
            return(-1);
        }
#endif /* CKROOT */

    x = strlen(path);
    debug(F111,"zmkdir",path,x);
    if (x < 1 || x > MAXPATH)           /* Check length */
      return(-1);
    if (!(tp = malloc(x+1)))            /* Make a temporary copy */
      return(-1);
    strcpy(tp,path);

    /* Fixup for UNC if necessary */
    if ( !strncmp(tp,"//",2) || !strncmp(tp,"\\\\",2) ) { 
        strcpy(tp,UNCname(tp));

        xp = &tp[2];                    /* */
        while ( !(ISDIRSEP(*xp)) )      /* Find end of machine name */
            xp++;
        xp++;                           /* Skip to beginning of object name */
        while ( !(ISDIRSEP(*xp)) )      /* Find end of object name */
            xp++;
        xp++;                           /* Skip to first directory name */

        if ( xp-tp > x ) {
            free(tp);
            return(-1);
        }
    } else {
        xp = tp;
        if (ISDIRSEP(*xp))              /* Don't create root directory! */
            xp++;
    }

    /* Go thru filespec from left to right... */
    for (; *xp; xp++) {                 /* Create parts that don't exist */
        if (!ISDIRSEP(*xp))             /* Find next directory separator */
          continue;
        c = *xp;                        /* Got one. */
        *xp = NUL;                      /* Make this the end of the string. */
        if (!isdir(tp)) {               /* This directory exists already? */
#ifdef CK_LOGIN
            if (isguest)                /* Not allowed for guests */
                return(-1);
#ifndef NOXFER
            else                    /* Nor if MKDIR and/or CD are disabled */
#endif /* CK_LOGIN */
                if ((server
#ifdef IKSD
                      || inserver
#endif /* IKSD */
                      ) && (!ENABLED(en_mkd) || !ENABLED(en_cwd)))
                    return(-1);
#endif /* IKSD */

            debug(F110,"zmkdir making",tp,0);
            x =                         /* No, try to create it */
              _mkdir(tp);               /* The IBM way */
            if (x < 0) {
                debug(F101,"zmkdir failed, errno","",errno);
                free(tp);               /* Free temporary buffer. */
                tp = NULL;
                return(-1);             /* Return failure code. */
            } else
              count++;
        }
        *xp = c;                        /* Replace the separator. */
    }
    free(tp);                           /* Free temporary buffer. */
    return(count);                      /* Return success code. */
}
#endif /* CK_MKDIR */

int
zrmdir(char * path) {
#ifdef CK_LOGIN
    if (isguest)
      return(-1);
#endif /* CK_LOGIN */

    if (!path) path = "";
    if (!*path) return(-1);

#ifdef CKROOT
    debug(F111,"zrmdir setroot",ckroot,ckrootset);
    if (ckrootset)
        if (!zinroot(path)) {
            debug(F110,"zrmdir setroot violation",path,0);
            return(-1);
        }
#endif /* CKROOT */


    if ( !strncmp(path,"//",2) || !strncmp(path,"\\\\",2) )
        path = UNCname(path);
    return(rmdir(path));
}

/* Z F S E E K  --  Position input file pointer */
/*
   Call with:
    Long int, 0-based, indicating desired position.
   Returns:
    0 on success.
   -1 on failure.
*/
#ifdef CK_RESEND
int
zfseek(CK_OFF_T pos)
{
#ifdef NT
    int rc;
	/* ** TODO: Restore use of fsetpos (pos is an __int64, fpos_t is a struct. 
	 * This sort of assignment is not allowed)
	
    fpos_t fpos = pos;
	
	*/
#endif /* NT */

    zincnt = -1 ;               /* must empty the input buffer */

    debug(F101,"zfseek","",pos);
#ifdef NT
    if (GetFileType((HANDLE)_get_osfhandle(_fileno(fp[ZIFILE])))
         == FILE_TYPE_PIPE) {
        debug(F100,"zfseek FILE_TYPE_PIPE","",0);
        return(-1);
    }
	/* ** TODO: Restore use of fsetpos
	
    rc = fsetpos(fp[ZIFILE], &fpos);
	
	*/
	rc = fseek(fp[ZIFILE], pos, 0);

    if (rc == 0) {
        debug(F100,"zfseek success","",0);
        return(0);
    } else {
        debug(F100,"zfseek failed","",GetLastError());
        return(-1);
    }
#else
    return(fseek(fp[ZIFILE], pos, 0)?-1:0);
#endif
}
#endif /* CK_RESEND */

struct zfnfp *
zfnqfp(fname, buflen, buf)  char * fname; int buflen; char * buf; {
    int x = 0, y = 0;
    char * xp;
    static struct zfnfp fnfp;

    if (!fname)
      return(NULL);

    /* initialize the data structure */
    fnfp.len = buflen;
    fnfp.fpath = buf;
    fnfp.fname = NULL;

#ifdef NT
    if ( GetFullPathName( fname, buflen, fnfp.fpath, &fnfp.fname ) ) {
        while ( fnfp.fpath[y] ) {
            if ( fnfp.fpath[y] == '\\' )
                fnfp.fpath[y] = '/';
            y++;
        }
	if (isdir(fnfp.fpath)) {
	    if (fnfp.fpath[y-1] != '/' && y < (buflen - 1)) {
		fnfp.fpath[y++] = '/';
		fnfp.fpath[y] = NUL;
	    }
	    fnfp.fname = NULL;
	}
        fnfp.len = strlen(fnfp.fpath);
        return(&fnfp);
    }
    else
#else /* NT */
      if (!DosQueryPathInfo(fname,5 /* Full Path Info */, fnfp.fpath,buflen)) {
          x = y = strlen(fnfp.fpath);
          for (x = x + 1; x >= 0; x--) {/* Find where the filename starts */
              if (fnfp.fpath[x] == '/') /* There is guaranteed to be one */
                  fnfp.fname = fnfp.fpath + x; /* Got it, set pointer */
              else if (fnfp.fpath[x] == '\\') {
                  fnfp.fpath[x] = '/';
                  fnfp.fname = fnfp.fpath + x; /* Got it, set pointer */
              }
          }
	  if (isdir(fnfp.fpath)) {
	      fnfp.fpath = NULL;
	      if (fnfp.fpath[y-1] != '/' && y < (buflen - 1)) {
		  fnfp.fpath[y++] = '/';
		  fnfp.fpath[y] = NUL;
	      }
	  }
          fnfp.len = strlen(fnfp.fpath);
          return(&fnfp);                /* and return. */
      } else
#endif /* NT */
    return(NULL);
}

/*  Z C M P F N  --  Compare two filenames  */

/*  Returns 1 if the two names refer to the same existing file, 0 otherwise. */

int
zcmpfn(s1,s2) char * s1, * s2; {
    char buf1[CKMAXPATH+1];
    char buf2[CKMAXPATH+1];
    char linkname[CKMAXPATH+1];
    int x, rc = 0;
    struct stat buf;

    if (!s1) s1 = "";
    if (!s2) s2 = "";
    if (!*s1 || !*s2) return(0);

    if (zfnqfp(s1,CKMAXPATH,buf1)) {    /* Convert to full pathname */

        if (zfnqfp(s2,CKMAXPATH,buf2)) {
            debug(F110,"zcmpfn s1",buf1,0);
            debug(F110,"zcmpfn s2",buf2,0);
            if (!strncmp(buf1,buf2,CKMAXPATH))
              rc = 1;
        }
    }
    debug(F101,"zcmpfn result","",rc);
    return(rc);
}


#ifdef CKROOT

/* User-mode chroot() implementation */

int
zsetroot(s) char * s; {                 /* Sets the root */
    char buf[CKMAXPATH+1];
    if (!s) return(-1);
    if (!*s) return(-1);
    debug(F110,"zsetroot",s,0);
    if (!isdir(s)) return(-2);
    if (!zfnqfp(s,CKMAXPATH,buf))       /* Get full, real path */
      return(-3);
    if (access(buf,R_OK) < 0) {         /* Check access */
        debug(F110,"zsetroot access denied",buf,0);
        return(-4);
    }
    s = buf;
    if (ckrootset) {                    /* If root already set */
        if (!zinroot(s)) {              /* make sure new root is in it */
            debug(F110,"zsetroot new root not in root",ckroot,0);
            return(-5);
        }
    }
    if (zchdir(buf) < 1) return(-4);    /* Change directory to new root */
    ckrootset = ckstrncpy(ckroot,buf,CKMAXPATH); /* Now set the new root */
    if (ckroot[ckrootset-1] != '/') {
        ckroot[ckrootset++] = '/';
        ckroot[ckrootset] = '\0';
    }
    debug(F111,"zsetroot rootset",ckroot,ckrootset);
    ckrooterr = 0;                      /* Reset error flag */
    return(1);
}

char *
zgetroot() {                            /* Returns the root */
    if (!ckrootset)
      return(NULL);
    return((char *)ckroot);
}

int
zinroot(s) char * s; {                  /* Checks if file s is in the root */
    int x, n;
    struct zfnfp * f = NULL;
    char buf[CKMAXPATH+2];

    debug(F111,"zinroot setroot",ckroot,ckrootset);
    ckrooterr = 0;                      /* Reset global error flag */
    if (!ckrootset)                     /* Root not set */
      return(1);                        /* so it's ok - no need to check */
    if (!(f = zfnqfp(s,CKMAXPATH,buf))) /* Get full and real pathname */
      return(0);                        /* Fail if we can't  */
    n = f->len;                         /* Length of full pathname */
    debug(F111,"zinroot n",buf,n);
    if (n < (ckrootset - 1) || n > CKMAXPATH) { /* Bad length */
        ckrooterr = 1;                          /* Fail */
        return(0);
    }
    if (isdir(buf) && buf[n-1] != '/') {  /* If it's a directory name */
        buf[n++] = '/';                   /* make sure it ends with '/' */
        buf[n] = '\0';
    }
    x = ckstrcmp(buf,ckroot,ckrootset,0); /* Compare, case-insensitive */
    debug(F111,"zinroot checked",buf,x);
    if (x == 0)                         /* OK */
      return(1);
    ckrooterr = 1;                      /* Not OK */
    return(0);
}
#endif /* CKROOT */

#ifdef NT
static int
UnicodeToOEM(LPWSTR InString, char *OutString, int OutStringSize)
{
    return WideCharToMultiByte(CP_OEMCP, 0, InString, -1,
                                OutString, OutStringSize, NULL, NULL);
}

static int
OEMToUnicode(char *InString, LPWSTR OutString, int OutStringSize)
{
    return MultiByteToWideChar(CP_OEMCP, 0, InString, -1,
                                OutString, OutStringSize);
}

static int
UnicodeToANSI(LPWSTR InString, char *OutString, int OutStringSize)
{
    return WideCharToMultiByte(CP_ACP, 0, InString, -1,
                                OutString, OutStringSize, NULL, NULL);
}

static int
ANSIToUnicode(char *InString, LPWSTR OutString, int OutStringSize)
{
    return MultiByteToWideChar(CP_ACP, 0, InString, -1,
                                OutString, OutStringSize);
}
#endif /* NT */

#ifdef CK_LOGIN
_PROTOTYP(const char * SSPLogonDomain,(VOID));
_PROTOTYP(int IsSSPLogonAvail,(VOID));

#ifdef NT
static HANDLE hLoggedOn = INVALID_HANDLE_VALUE;
static PSID pSid = NULL;
static PSID pPriGroupSid = NULL;
static PTOKEN_GROUPS pTokenGroups = NULL;
static SID_NAME_USE SidNameUse;
static char HomeDir[MAXPATH] = "", ProfilePath[MAXPATH] = "";
static DWORD PrimaryGroupId;
CHAR * pReferenceDomainName = NULL;
static CHAR * pPDCName = NULL;

#ifndef PROFILEINFO
#define PI_NOUI 1                       // Prevents displaying of profile error messages.
#define PI_APPLYPOLICY 2                // Applies a Windows NT 4.0-style policy.

typedef struct _PROFILEINFOA {
    DWORD       dwSize;                 // Set to sizeof(PROFILEINFO) before calling
    DWORD       dwFlags;                // See flags above
    LPSTR       lpUserName;             // User name (required)
    LPSTR       lpProfilePath;          // Roaming profile path (optional, can be NULL)
    LPSTR       lpDefaultPath;          // Default user profile path (optional, can be NULL)
    LPSTR       lpServerName;           // Validating domain controller name in netbios format (optional, can be NULL but group NT4 style policy won't be applied)
    LPSTR       lpPolicyPath;           // Path to the NT4 style policy file (optional, can be NULL)
    HANDLE      hProfile;               // Filled in by the function.  Registry key handle open to the root.
} PROFILEINFOA, FAR * LPPROFILEINFOA;
#define PROFILEINFO PROFILEINFOA
#define LPPROFILEINFO LPPROFILEINFOA
#endif /* PROFILEINFO */
PROFILEINFO profinfo = { sizeof profinfo, 0, 0, 0, 0, 0, 0 };
VOID      * pEnvBlock = NULL;
static HINSTANCE hUserEnv=NULL, hAdvApi=NULL;
static BOOL (WINAPI * p_CreateEnvironmentBlock)(void **, HANDLE, BOOL)=NULL;
static BOOL (WINAPI * p_DestroyEnvironmentBlock)(void *)=NULL;
static BOOL (WINAPI * p_LoadUserProfileA)(HANDLE, PROFILEINFO *)=NULL;
static BOOL (WINAPI * p_UnloadUserProfileA)(HANDLE, HANDLE)=NULL;
static BOOL (WINAPI * p_DuplicateTokenEx)(HANDLE hExistingToken,
					   DWORD dwDesiredAccess,
					   LPSECURITY_ATTRIBUTES lpTokenAttributes,
					   SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
					   TOKEN_TYPE TokenType,
					   PHANDLE phNewToken)=NULL;

BOOL GetTextualSid(
    PSID pSid,            // binary Sid
    LPTSTR TextualSid,    // buffer for Textual representation of Sid
    LPDWORD lpdwBufferLen // required/provided TextualSid buffersize
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

    // Validate the binary SID.

    if(!IsValidSid(pSid)) return FALSE;

    // Get the identifier authority value from the SID.

    psia = GetSidIdentifierAuthority(pSid);

    // Get the number of subauthorities in the SID.

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

    // Compute the buffer length.
    // S-SID_REVISION- + IdentifierAuthority- + subauthorities- + NULL

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

    // Check input buffer length.
    // If too small, indicate the proper size and set last error.

    if (*lpdwBufferLen < dwSidSize)
    {
        *lpdwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    // Add 'S' prefix and revision number to the string.

    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

    // Add SID identifier authority to the string.

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

    // Add SID subauthorities to the string.
    //
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}

#ifdef NTCREATETOKEN
#define MAXSIZE 4096
#define gle (GetLastError())
static BOOL getSids( TOKEN_PRIVILEGES **tokenPrivs, PSID *ownerSid, PSID *priGroupSid, TOKEN_GROUPS **tokenGroups, LUID *logonSessionId );
static BOOL showToken( HANDLE ht );
static void showSid( int indent, PSID ps );
static BOOL Sid2Text( PSID ps, char *buf, int bufSize );
static BOOL IsLogonSid( PSID ps );
static BOOL IsLocalSid( PSID ps );
static BOOL IsInteractiveSid( PSID ps );
static BOOL getPriv( const char *privName );

HANDLE
CreateTokenForUser(char * szUsername, char * szDomain, PSID luserSid, PSID preGroupSid,
                    TOKEN_GROUPS * userTokenGroups) {
    HINSTANCE hlib;
    DWORD rc = 0;
    PSID ownerSid = 0, iksdSid = 0;
    PSID admingrpSid = 0, worldSid = 0;
    LUID logonSessionId;
    TOKEN_PRIVILEGES *tokenPrivs;
    SID_IDENTIFIER_AUTHORITY NTsia = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY worldsia = SECURITY_WORLD_SID_AUTHORITY;
    char *domainName;
    DWORD luserSidSize;
    DWORD domainNameSize;
    SID_NAME_USE luserSidType;
    // returned handle
    HANDLE tokenHandle = INVALID_HANDLE_VALUE;
    // permissions with which the handle is opened
    DWORD requestedPermissions = TOKEN_ALL_ACCESS;
    PACL newTokenDacl;
    SECURITY_DESCRIPTOR newTokenSD;
    SECURITY_QUALITY_OF_SERVICE sqos;
    LSA_OBJECT_ATTRIBUTES ObjAttr;
    TOKEN_TYPE tokenType;
    LUID sessionLuid, serviceSessionLuid = { 0x3e7, 0 };
    FILETIME expirationTime;
    SID_AND_ATTRIBUTES tokenUser;
    PSID newOwnerSid, newPriGroupSid;
    PACL newDefaultDacl;
    TOKEN_SOURCE tokenSource = { { '*', 'i', 'k', 's', 'd', '*' }, { 0, 0 } }; // TOKEN_SOURCE


    typedef DWORD (__stdcall *tNCT)( HANDLE                     *phToken,               // returned handle
                                     DWORD                      *pReqPermissionMask,    // requested permissions for returned handle
                                     LSA_OBJECT_ATTRIBUTES      *pObjAttr,              // contains SD controlling access to the new token
                                     TOKEN_TYPE                 tokenType,              // TokenPrimary == 1 == primary, TokenImpersonation == 2 == impersonation
                                     LUID                       *pLogonSessionLuid,     // LUID for logon session, goes into the token
                                     FILETIME                   *expirationTime,        // token expiration deadline. unsupported -- set to 2^63 - 1
                                     SID_AND_ATTRIBUTES         *pTokenUser,            // returned by GetTokenInformation( TokenUser )
                                     TOKEN_GROUPS               *pTokenGroups,          // exploded list of groups for the user
                                     TOKEN_PRIVILEGES           *pTokenPrivileges,      // list of privs assigned (not necessarily enabled)
                                     PSID                       *pOwnerSid,             // owner SID
                                     PSID                       *pPrimaryGroupSid,      // primary group SID
                                     PACL                       *pDefaultDacl,          // will be used as default if a process using this token assigns no security to new objects
                                     TOKEN_SOURCE               *pTokenSource           // contains creator and creator-specified LUID
                                     );
    tNCT pNCT;

    debug(F110,"GetTokenForUser szDomain",szDomain,0);
    debug(F110,"GetTokenForUser szUsername",szUsername,0);

    // extract a few useful things from our current process token
    //
    if (!getSids( &tokenPrivs, &ownerSid,
                  pPriGroupSid ? NULL : &pPriGroupSid,
                  userTokenGroups ? NULL : &userTokenGroups,
                  &logonSessionId )) {
#ifdef BETADEBUG
        printf("  getSids() failed\r\n");
#endif /* BETADEBUG */
        return(INVALID_HANDLE_VALUE);
    }

    // the next sid is really the domain admin -- check the reserved RID at the end
    AllocateAndInitializeSid( &NTsia, 5, 21, 484763869, 764733703, 1202660629, 500, 0, 0, 0, &iksdSid );

    // local admins group
    AllocateAndInitializeSid( &NTsia, 2, 0x20, 0x220, 0, 0, 0, 0, 0, 0, &admingrpSid );

    // and a crowd pleaser
    AllocateAndInitializeSid( &NTsia, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &worldSid );

    // SeCreateTokenPrivilege must be explicitly enabled
    getPriv( SE_CREATE_TOKEN_NAME );

    // init guessed arguments

    // this DACL determines who may mess with the token-to-be
    newTokenDacl = (PACL) malloc( MAXSIZE );
    InitializeAcl( newTokenDacl, MAXSIZE, ACL_REVISION );
    AddAccessAllowedAce( newTokenDacl, ACL_REVISION, TOKEN_ALL_ACCESS, worldSid );
    AddAccessAllowedAce( newTokenDacl, ACL_REVISION, TOKEN_ALL_ACCESS, iksdSid );
    AddAccessAllowedAce( newTokenDacl, ACL_REVISION, TOKEN_ALL_ACCESS, admingrpSid );
    // AddAccessAllowedAce( newTokenDacl, ACL_REVISION, TOKEN_ALL_ACCESS, luserSid );

    // this SD says that Administrators is the owner of the token itself
    // (the "owner" below is what gets used as the default owner for, say, files created under this token)
    newTokenSD.Revision = 1;
    newTokenSD.Sbz1 = 0;
    newTokenSD.Control = 4;
    newTokenSD.Owner = admingrpSid;
    newTokenSD.Group = 0;
    newTokenSD.Sacl = 0;
    newTokenSD.Dacl = newTokenDacl;

    // this says we wish to create a token that we can impersonate, and which reflects
    // ongoing changes in permissions/groups/... if the security provider can do that,
    // and which allows an impersonating server to enable/disable groups or privs
    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    // OBJECT_ATTRIBUTES is the NT bog-standard way of passing the name
    // and security for an object to be created or opened
    ObjAttr.Length = sizeof(ObjAttr);
    ObjAttr.RootDirectory = 0;
    ObjAttr.ObjectName = 0;
    ObjAttr.Attributes = 0;
    ObjAttr.SecurityDescriptor = &newTokenSD;
    ObjAttr.SecurityQualityOfService = &sqos;

    // primary tokens are usable for CreateProcessAsUser()
    tokenType = TokenPrimary;

    // logon session ID
    // use either the service logon ID, or the LUID extracted from our own token:
    // sessionLuid = serviceSessionLuid; // this would be the logon session for services
    sessionLuid = logonSessionId; // this would be the logon session for services

    // FILETIME format for token expiration time. 2^63-1 means no expiration.
    // NTLM does not support this -- Kerberos should, but I didn't test it
    expirationTime.dwLowDateTime = 0xffffffffUL;
    expirationTime.dwHighDateTime = 0x7fffffffUL;

    // token user -- the user who will be impersonated or whatever
    tokenUser.Sid = luserSid;
    tokenUser.Attributes = 0;

    // optional: remove our logon SID from processTokenGroups, add logon SID as per session LUID above
    // or collect group info for account and hand-build group list

    // optional: filter tokenPrivs

    // owner SID -- will be stamped on all objects that get created without an explicit SD under our token
    newOwnerSid = luserSid;

    // primary group SID -- will be stamped on all objects that get created without an explicit SD under our token
    newPriGroupSid = pPriGroupSid;

    // this DACL will be stamped on all objects that get created without an explicit SD under our token
    newDefaultDacl = (PACL) malloc( MAXSIZE );
    InitializeAcl( newDefaultDacl, MAXSIZE, ACL_REVISION );
    AddAccessAllowedAce( newDefaultDacl, ACL_REVISION, MAXDWORD, worldSid );


    hlib = LoadLibrary( "ntdll.dll" );
    if ( hlib ) {
        pNCT = (tNCT) GetProcAddress( hlib, "NtCreateToken" );
        if ( pNCT )
        {
            debug(F100,"calling NtCreateToken()","",0);
            rc = pNCT( &tokenHandle, &requestedPermissions, &ObjAttr, tokenType,
                       &sessionLuid, &expirationTime, &tokenUser, userTokenGroups, tokenPrivs,
                       &newOwnerSid, &newPriGroupSid, &newDefaultDacl, &tokenSource );
            debug(F111,"NtCreateToken() returns","rc",rc);
            if ( rc == 0 )
            {
                HANDLE ht = 0;
#ifdef BETADEBUG
                printf( "token handle: %08lXh\n", tokenHandle );
#endif /* BETADEBUG */
                // odd thing: the tokenHandle is unusable ... but it _can_ be
                // duplicated with full access. Need to check if this is only
                // because we are the owner or whatever!
                if ( DuplicateHandle( GetCurrentProcess(), tokenHandle,
                                      GetCurrentProcess(), &ht, TOKEN_ALL_ACCESS, FALSE, DUPLICATE_CLOSE_SOURCE ) )
                {
                    if (ht != tokenHandle) {
                      CloseHandle( tokenHandle );
                      tokenHandle = ht;
#ifdef BETADEBUG
                      printf( "dup token handle: %08lXh\n", tokenHandle );
#endif /* BETADEBUG */
                    }
                }
#ifdef BETADEBUG
                else
                   printf( "DH(): gle = %lu\n", gle );
#endif /* BETADEBUG */
            }
#ifdef BETADEBUG
            else {
               printf( "NCT(): gle = %lu\n", gle );
               printf( "NCT(): rc = %08lXh\n", rc );
            }
#endif /* BETADEBUG */
        }
        FreeLibrary( hlib );
    }

    if ( luserSid != pSid )
        free(luserSid);

#ifdef BETADEBUG
    printf( "token handle: %08lXh\n", tokenHandle );
#endif /* BETADEBUG */
    return(tokenHandle);
}


BOOL
getSids( TOKEN_PRIVILEGES **tokenPrivs, PSID *ownerSid, PSID *priGroupSid, TOKEN_GROUPS **processTokenGroups, LUID *logonSessionId )
{
    DWORD needed;
    HANDLE ht;

    TOKEN_STATISTICS tstat;
    TOKEN_OWNER *pto;
    TOKEN_PRIMARY_GROUP *ptpg;
    TOKEN_GROUPS *ptg;
    TOKEN_PRIVILEGES *ptp;

    debug(F100,"getSids","",0);

    // open process token

    if ( ! OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &ht ) )
    {
        if ( ! OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &ht ) )
        {
            DWORD error = gle;
            if (error != ERROR_CALL_NOT_IMPLEMENTED)
                printf("  OpenProcessToken(): gle = %lu\n", error);
            return FALSE;
        }
    }

    // grab token information

    // logon session LUID
    if ( logonSessionId ) {
        if ( ! GetTokenInformation( ht, TokenStatistics, &tstat, sizeof tstat, &needed ) )
            printf( "  GetTokenInformation( TokenStatistics ): gle = %lu\n", gle );
        else
        {
            *logonSessionId = tstat.AuthenticationId;
        }
    }

    // token owner
    if ( ownerSid ) {
        pto = (TOKEN_OWNER *) malloc( MAXSIZE );
        if ( ! GetTokenInformation( ht, TokenOwner, pto, MAXSIZE, &needed ) )
            printf( "  GetTokenInformation( TokenOwner ): gle = %lu\n", gle );
        else
        {
            *ownerSid = (PSID) malloc( MAXSIZE );
            CopySid( MAXSIZE, *ownerSid, pto->Owner );
        }
        free(pto);
    }

    // token primary group
    if ( priGroupSid ) {
        ptpg = (TOKEN_PRIMARY_GROUP *) malloc( MAXSIZE );
        if ( ! GetTokenInformation( ht, TokenPrimaryGroup, ptpg, MAXSIZE, &needed ) )
            printf( "  GetTokenInformation( TokenPrimaryGroup ): gle = %lu\n", gle );
        else
        {
            *priGroupSid = (PSID) malloc( MAXSIZE );
            CopySid( MAXSIZE, *priGroupSid, ptpg->PrimaryGroup );
        }
        free(ptpg);
    }

    // token groups
    if ( processTokenGroups ) {
        ptg = (TOKEN_GROUPS *) malloc( MAXSIZE );
        if ( ! GetTokenInformation( ht, TokenGroups, ptg, MAXSIZE, &needed ) )
            printf( "  GetTokenInformation( TokenGroups ): gle = %lu\n", gle );
        else
        {
            *processTokenGroups = ptg;
        }
    }

    if ( tokenPrivs ) {
        ptp = (TOKEN_PRIVILEGES *) malloc( MAXSIZE );
        if ( ! GetTokenInformation( ht, TokenPrivileges, ptp, MAXSIZE, &needed ) )
            printf( "  GetTokenInformation( TokenPrivileges ): gle = %lu\n", gle );
        else
        {
            *tokenPrivs = ptp;
        }
    }

    CloseHandle( ht );
    return TRUE;
}



const char *formatTime( FILETIME *t )
{
    SYSTEMTIME st;
    DWORD len = MAXSIZE;
    static char buf[MAXSIZE];

    if ( t->dwHighDateTime > 0x7fffffffUL ||
         ( t->dwHighDateTime == 0x7fffffffUL && t->dwLowDateTime == 0xffffffffUL ) )
    {
        ckstrncpy( buf, "-none-", MAXSIZE );
        return buf;
    }

    FileTimeToSystemTime( t, &st );
    GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, 0, buf, len );
    ckstrncat( buf, " ", MAXSIZE );
    len -= strlen( buf );
    GetTimeFormat( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &st, 0, &buf[strlen( buf )], len );
    return buf;
}



BOOL
showToken( HANDLE ht )
{
    DWORD needed;
    DWORD i;

    TOKEN_SOURCE ts;
    TOKEN_STATISTICS tstat;
    TOKEN_OWNER *pto;
    TOKEN_USER *ptu;
    TOKEN_PRIMARY_GROUP *ptpg;
    TOKEN_GROUPS *ptg;

    // dump token information

    // creating subsystem
    if ( ! GetTokenInformation( ht, TokenSource, &ts, sizeof ts, &needed ) )
        printf( "  GetTokenInformation( TokenSource ): gle = %lu\n", gle );
    else
        printf( "  Token source: \"%-8.8s\" (luid = %I64d)\n",
                ts.SourceName, ts.SourceIdentifier );

    // logon session LUID
    if ( ! GetTokenInformation( ht, TokenStatistics, &tstat, sizeof tstat, &needed ) )
        printf( "  GetTokenInformation( TokenStatistics ): gle = %lu\n", gle );
    else
    {
        printf( "  Token Statistics:\n" );
        printf( "    TokenId:            %I64d (%lu-%lu)\n", *( (__int64 *) &tstat.TokenId ),
                tstat.TokenId.HighPart, tstat.TokenId.LowPart );
        printf( "    AuthenticationId:   %I64d (%lu-%lu)\n", *( (__int64 *) &tstat.AuthenticationId ),
                tstat.AuthenticationId.HighPart, tstat.AuthenticationId.LowPart );
        printf( "    ExpirationTime:     %016I64Xh (%s)\n", *( (__int64 *) &tstat.ExpirationTime ),
                formatTime( (FILETIME *) &tstat.ExpirationTime ) );
        printf( "    TokenType:          %d (%s)\n", tstat.TokenType,
                tstat.TokenType == 1? "Primary":
                ( tstat.TokenType == 2? "Impersonation": "unknown" ) );
        printf( "    ImpersonationLevel: %d (%s)\n", tstat.ImpersonationLevel,
                tstat.ImpersonationLevel == SecurityAnonymous? "Anonymous":
                ( tstat.ImpersonationLevel == SecurityIdentification? "Identification":
                  ( tstat.ImpersonationLevel == SecurityImpersonation? "Impersonation":
                    ( tstat.ImpersonationLevel == SecurityDelegation? "Delegation": "unknown" ) ) ) );
        printf( "    DynamicCharged:     %lu bytes\n", tstat.DynamicCharged );
        printf( "    DynamicAvailable:   %lu bytes\n", tstat.DynamicAvailable );
        printf( "    GroupCount:         %lu\n", tstat.GroupCount );
        printf( "    PrivilegeCount:     %lu\n", tstat.PrivilegeCount );
        printf( "    ModifiedId:         %I64d (%lu-%lu)\n", *( (__int64 *) &tstat.ModifiedId ),
                tstat.ModifiedId.HighPart, tstat.ModifiedId.LowPart );
    }

    // token owner
    pto = (TOKEN_OWNER *) malloc( MAXSIZE );
    if ( ! GetTokenInformation( ht, TokenOwner, pto, MAXSIZE, &needed ) )
        printf( "  GetTokenInformation( TokenOwner ): gle = %lu\n", gle );
    else
    {
        printf( "  Token owner:\n    " );
        showSid( 4, pto->Owner );
        putchar( '\n' );
    }

    // token user
    ptu = (TOKEN_USER *) malloc( MAXSIZE );
    if ( ! GetTokenInformation( ht, TokenUser, ptu, MAXSIZE, &needed ) )
        printf( "  GetTokenInformation( TokenUser ): gle = %lu\n", gle );
    else
    {
        printf( "  Token user:\n    " );
        showSid( 4, ptu->User.Sid );
        putchar( '\n' );
    }

    // token primary group
    ptpg = (TOKEN_PRIMARY_GROUP *) malloc( MAXSIZE );
    if ( ! GetTokenInformation( ht, TokenPrimaryGroup, ptpg, MAXSIZE, &needed ) )
        printf( "  GetTokenInformation( TokenPrimaryGroup ): gle = %lu\n", gle );
    else
    {
        printf( "  Token primary group:\n    " );
        showSid( 4, ptpg->PrimaryGroup );
        putchar( '\n' );
    }

    // token groups
    ptg = (TOKEN_GROUPS *) malloc( MAXSIZE );
    if ( ! GetTokenInformation( ht, TokenGroups, ptg, MAXSIZE, &needed ) )
        printf( "  GetTokenInformation( TokenGroups ): gle = %lu\n", gle );
    else
    {
        if ( ptg->GroupCount == 0 )
            printf( "  Token groups: (none)\n" );
        else
        {
            printf( "  Token groups:\n" );
            for ( i = 0; i < ptg->GroupCount; ++ i )
            {
                printf( "    %08lXh  ", ptg->Groups[i].Attributes );
                showSid( 15, ptg->Groups[i].Sid );
                putchar( '\n' );
            }
        }
    }

    free( pto );
    free( ptu );
    free( ptpg );
    free( ptg );

    return TRUE;
}

void showSid( int indent, PSID ps )
{
    char textSid[MAX_PATH], user[MAX_PATH], domain[MAX_PATH], buf[MAX_PATH];
    DWORD sizeUser, sizeDomain;
    SID_NAME_USE snu;
    DWORD charsleft;
    const char *t;
    const char *sep = "\\"; // separator for domain\user display

    Sid2Text( ps, textSid, sizeof textSid );
    fputs( textSid, stdout );
    charsleft = 78 - indent - strlen( textSid );

    sizeUser = sizeof user;
    sizeDomain = sizeof domain;
    if ( ! LookupAccountSid( NULL, ps, user, &sizeUser, domain, &sizeDomain, &snu ) )
    {
        DWORD rc = gle;

        if ( IsLogonSid( ps ) )
            strcpy( buf, "(interactive logon session SID)" );
        else
            _snprintf( buf, sizeof(buf)-1, "[LAS(): gle = %lu]", rc );
        if ( strlen( buf ) > charsleft )
            printf( "\n%*s", indent, "" );
        else
            putchar( ' ' );
        fputs( buf, stdout );
        return;
    }

    switch ( snu )
    {
    case SidTypeUser:
        t = "user";
        break;
    case SidTypeGroup:
        t = "group";
        break;
    case SidTypeDomain:
        t = "domain";
        break;
    case SidTypeAlias:
        t = "alias";
        break;
    case SidTypeWellKnownGroup:
        t = "well-known group";
        break;
    case SidTypeDeletedAccount:
        t = "deleted";
        break;
    case SidTypeInvalid:
        t = "invalid";
        break;
    case SidTypeUnknown:
        t = "unknown";
        break;
#ifndef _M_ALPHA
#ifndef _M_PPC
    case SidTypeComputer:
        t = "computer";
        break;
#endif /* _M_PPC */
#endif /* _M_ALPHA */
    default:
        t = "*?unknown?*";
        break;
    }

    if ( domain[0] == '\0' || user[0] == '\0' )
        sep = "";

    _snprintf( buf, sizeof(buf)-1, "\"%s%s%s\" (%s)", domain, sep, user, t );
    if ( strlen( buf ) > charsleft )
        printf( "\n%*s", indent, "" );
    else
        putchar( ' ' );
    fputs( buf, stdout );
}



BOOL IsLocalSid( PSID ps )
{
    static PSID pComparisonSid = NULL;

    if ( pComparisonSid == NULL )
    {
        // build "BUILTIN\LOCAL" SID for comparison: S-1-2-0
        SID_IDENTIFIER_AUTHORITY sia = SECURITY_LOCAL_SID_AUTHORITY;
        AllocateAndInitializeSid( &sia, 1, 0, 0, 0, 0, 0, 0, 0, 0, &pComparisonSid );
    }

    return EqualSid( ps, pComparisonSid );
}



BOOL IsInteractiveSid( PSID ps )
{
    static PSID pComparisonSid = NULL;

    if ( pComparisonSid == NULL )
    {
        // build "BUILTIN\LOCAL" SID for comparison: S-1-5-4
        SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY; // "-5-"
        AllocateAndInitializeSid( &sia, 1, 4, 0, 0, 0, 0, 0, 0, 0, &pComparisonSid );
    }

    return EqualSid( ps, pComparisonSid );
}



BOOL IsLogonSid( PSID ps )
{
    static SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;

    // a logon SID has: sia = 5, subauth count = 3, first subauth = 5
    // the following three lines test these three conditions
    if ( ! memcmp( GetSidIdentifierAuthority( ps ), &sia, sizeof sia )  && // is sia == 5?
         *GetSidSubAuthorityCount( ps ) == 3                                                            && // is subauth count == 3?
         *GetSidSubAuthority( ps, 0 ) == 5                                                                      )  // first subauth == 5?
        return TRUE;
    else
        return FALSE;
}



// nearly straight from the SDK
BOOL Sid2Text( PSID ps, char *buf, int bufSize )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev = SID_REVISION;
    DWORD i;
    int n, size;
    char *p;

    // Validate the binary SID.

    if ( ! IsValidSid( ps ) )
        return FALSE;

    // Get the identifier authority value from the SID.

    psia = GetSidIdentifierAuthority( ps );

    // Get the number of subauthorities in the SID.

    dwSubAuthorities = *GetSidSubAuthorityCount( ps );

    // Compute the buffer length.
    // S-SID_REVISION- + IdentifierAuthority- + subauthorities- + NULL

    size = 15 + 12 + ( 12 * dwSubAuthorities ) + 1;

    // Check input buffer length.
    // If too small, indicate the proper size and set last error.

    if ( bufSize < size )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    // Add 'S' prefix and revision number to the string.

    size = sprintf( buf, "S-%lu-", dwSidRev );
    p = buf + size;

    // Add SID identifier authority to the string.

    if ( psia->Value[0] != 0 || psia->Value[1] != 0 )
    {
        n = sprintf( p, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                     (USHORT) psia->Value[0], (USHORT) psia->Value[1],
                     (USHORT) psia->Value[2], (USHORT) psia->Value[3],
                     (USHORT) psia->Value[4], (USHORT) psia->Value[5] );
        size += n;
        p += n;
    }
    else
    {
        n = sprintf( p, "%lu", ( (ULONG) psia->Value[5] ) +
                     ( (ULONG) psia->Value[4] << 8 ) + ( (ULONG) psia->Value[3] << 16 ) +
                     ( (ULONG) psia->Value[2] << 24 ) );
        size += n;
        p += n;
    }

    // Add SID subauthorities to the string.

    for ( i = 0; i < dwSubAuthorities; ++ i )
    {
        n = sprintf( p, "-%lu", *GetSidSubAuthority( ps, i ) );
        size += n;
        p += n;
    }

    return TRUE;
}

BOOL
getPriv( const char *privName )
{
    BOOL rc;
    HANDLE hToken;
    LUID privValue;
    TOKEN_PRIVILEGES tkp;

    if ( ! OpenProcessToken( GetCurrentProcess(),
                             TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) ) {
        return FALSE;
    }

    if ( !LookupPrivilegeValue( NULL, privName, &privValue ) )
    {
        CloseHandle( hToken );
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = privValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    rc = !! AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL );
    CloseHandle( hToken );
    return rc;
}
#endif /* NTCREATETOKEN */
#endif /* NT */

/*  Z V U S E R  --  Verify user, Returns 1 if user OK, 0 otherwise.  */

int
zvuser(username) char *username; {
#ifdef NT
    int rc = 0;
    DWORD dwSid = 0;
    DWORD dwName = 0;
    DWORD LastError;
    char buf[4096], *p, *name;

    if (logged_in)
        return(0);

#ifdef CKSYSLOG
    if (ckxsyslog && ckxlogging) {
        sprintf(buf, "login: user %s",username);
        cksyslog(SYSLG_LI, 1, buf, NULL, NULL);
    }
#endif /* CKSYSLOG */

    /* Cleanup from previous calls (if any) */

    if ( !p_CreateEnvironmentBlock || !p_DestroyEnvironmentBlock ||
         !p_LoadUserProfileA || !p_UnloadUserProfileA ) {

        if ( !hUserEnv )
            hUserEnv = LoadLibrary("userenv.dll");
        if ( hUserEnv ) {
            (FARPROC) p_CreateEnvironmentBlock =
                GetProcAddress( hUserEnv, "CreateEnvironmentBlock" );
            (FARPROC) p_DestroyEnvironmentBlock =
                GetProcAddress( hUserEnv, "DestroyEnvironmentBlock" );
            (FARPROC) p_LoadUserProfileA =
                GetProcAddress( hUserEnv, "LoadUserProfileA" );
            (FARPROC) p_UnloadUserProfileA =
                GetProcAddress( hUserEnv, "UnloadUserProfileA" );
        }
    }

    if ( hLoggedOn != INVALID_HANDLE_VALUE ) {
        debug(F110,"zvuser()","calling RevertToSelf()",0);
        RevertToSelf();

        if ( p_UnloadUserProfileA && profinfo.hProfile )
            p_UnloadUserProfileA(hLoggedOn, profinfo.hProfile);
        memset(&profinfo,0,sizeof(profinfo));
        profinfo.dwSize = sizeof(profinfo);

        if ( p_DestroyEnvironmentBlock && pEnvBlock ) {
            p_DestroyEnvironmentBlock(pEnvBlock);
            pEnvBlock = NULL;
        }
        CloseHandle(hLoggedOn);
        hLoggedOn = INVALID_HANDLE_VALUE;
    }
    if (pSid) {
        free(pSid);
        pSid = NULL;
    }
    if (pPriGroupSid) {
        free(pPriGroupSid);
        pPriGroupSid = NULL;
    }
    if (pTokenGroups) {
        int i;
        for (i=0 ; i < pTokenGroups->GroupCount; i++ )
            free(pTokenGroups->Groups[i].Sid);
        free(pTokenGroups);
        pTokenGroups = NULL;
    }

    if ( pReferenceDomainName ) {
        free(pReferenceDomainName);
        pReferenceDomainName = NULL;
    }
    if ( pPDCName ) {
        free(pPDCName);
        pPDCName = NULL;
    }

    /* Now start the login process */
    debug(F110,"zvuser() username",username,0);
    if ( ( !stricmp("anonymous",username)
#ifdef COMMENT
           || !stricmp("guest",username) || !stricmp("ftp",username)
#endif /* COMMENT */
           )) {
        if ( ckxanon )
            name = anonacct ? anonacct : "guest";
        else {
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                sprintf(buf,
                       "login: anonymous login not allowed: %s",
                       clienthost ? clienthost : "(unknown host)"
                       );
                cksyslog(SYSLG_LI, 0, buf, NULL, NULL);

            }
#endif /* CKSYSLOG */
            return(0);
        }
    }
    else
        name = username;

    debug(F110,"zvuser() name",name,0);

    if ( name == NULL || name[0] == '\0' )
        return(0);

    if ( !isWin95() ) {
        for (p = name; *p && (*p != '\\') ; p++) ;
        if ( *p ) {
            /* a domain has already been specified */
            strcpy(buf,name);
        } else if (iks_domain && iks_domain[0]) {
            sprintf(buf,"%s\\%s",iks_domain,name);
        } else {
            strcpy(buf,name);
        }
        rc = LookupAccountName( NULL,buf,
                                pSid,&dwSid,pReferenceDomainName,&dwName,
                                &SidNameUse);
        pSid = (PSID) malloc(dwSid+1);
        if ( !pSid )
            return(0);
        pReferenceDomainName = (CHAR *) malloc(dwName+1);
        if ( !pReferenceDomainName ) {
            free(pSid);
            pSid = NULL;
            return(0);
        }
        rc = LookupAccountName( NULL,buf,
                                pSid,&dwSid,pReferenceDomainName,
                                &dwName,&SidNameUse);
        if ( rc && (SidNameUse == SidTypeDomain
#ifndef _M_ALPHA
#ifndef _M_PPC
                     || SidNameUse == SidTypeComputer
#endif /* _M_PPC */
#endif /* _M_ALPHA */
                     ) )
        {
            /* Username equals Machine name */
            sprintf(buf,"%s\\%s",name,name);
            debug(F110,"zvuser SidNameUse == SidTypeDomain",buf,0);
            free(pSid);
            pSid = NULL;
            free(pReferenceDomainName);
            pReferenceDomainName = NULL;
            dwSid = dwName = 0;
            LookupAccountName( NULL,buf,
                               pSid,&dwSid,pReferenceDomainName,&dwName,
                               &SidNameUse);
            pSid = (PSID) malloc(dwSid+1);
            if ( !pSid )
                return(0);
            pReferenceDomainName = (CHAR *) malloc(dwName+1);
            if ( !pReferenceDomainName ) {
                free(pSid);
                pSid = NULL;
                return(0);
            }
            rc = LookupAccountName( NULL,buf,
                                    pSid,&dwSid,pReferenceDomainName,
                                    &dwName,&SidNameUse);
        }
        if ( !rc ) {
            LastError = GetLastError();
            debug(F111,"zvuser","LookupAccount()",LastError);
            if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                LastError,
                                MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                                buf,
                                4096,
                                (va_list *) NULL
                                ) )
            {
                printf("\r\n  (%s) %s\r\n",name,buf);
            }
            free(pSid);
            pSid = NULL;
            free(pReferenceDomainName);
            pReferenceDomainName = NULL;

            if ( name != anonacct ) {
                for ( p = name; *p ; p++ ) {
                    if ( *p == '\\' ) {
                        *p = '\0';
                        makestr(&pReferenceDomainName,name);
                        p++;
                        strcpy(buf,p);
                        strcpy(name,buf);
                        break;
                    }
                }
                if ( !pReferenceDomainName && iks_domain )
                    makestr(&pReferenceDomainName,iks_domain);
            }
            return(0);
        }
        pReferenceDomainName[dwName] = '\0';
        debug(F110,"zvuser() Reference Domain Name",pReferenceDomainName,0);

        /* Strip off the domain if it is part of the name */
        if ( name != anonacct ) {
            for ( p=name; *p ; p++ ) {
                if ( *p == '\\' ) {
                    p++;
                    strcpy(buf,p);
                    strcpy(name,buf);
                    break;
                }
            }
        }
    } else {    /* Windows 95/98 */
        if ( name != anonacct ) {
            for ( p = name; *p ; p++ ) {
                if ( *p == '\\' ) {
                    *p = '\0';
                    makestr(&pReferenceDomainName,name);
                    p++;
                    strcpy(buf,p);
                    strcpy(name,buf);
                    break;
                }
            }
            if (!pReferenceDomainName) {
                if (iks_domain)
                    makestr(&pReferenceDomainName,iks_domain);
                else if ( IsSSPLogonAvail() )
                    makestr(&pReferenceDomainName,SSPLogonDomain());
            }
        }
    }
    if ( name == anonacct )
        strcpy(uidbuf,"anonymous");
    else if (uidbuf != name)
        ckstrncpy((char *)uidbuf,(char *)name,UIDBUFLEN);
#else /* NT */
    char * name;

    debug(F110,"zvuser() username",username,0);
    if ( !stricmp("anonymous",username) && ckxanon && anonacct )
        name = anonacct;
    else
        name = username;

    debug(F110,"zvuser() name",name,0);

    if ( name == anonacct )
        strcpy(uidbuf,"anonymous");
    else if (uidbuf != name) {
        ckstrncpy((char *)uidbuf,(char *)name,UIDBUFLEN);
    }
#endif /* NT */
    debug(F100,"zvuser() success","",0);
    return(1);
}

/*  Z V P A S S  --  Verify password; returns 1 if OK, 0 otherwise  */

int
zvpass(passwd) char *passwd; {
    int rc = 0;
    int ntlm=0;
    char * uid, * pwd;
    char buf[4096];
    char username[256];
#ifdef IKSDB
    extern int ikdbopen;
    extern unsigned long mydbslot;
#endif /* IKSDB */
#ifdef NT
    DWORD LastError, LogonUserError;
#endif /* NT */
#ifdef CK_SRP
    int srp_verified = 0;
#endif /* CK_SRP */

    debug(F110,"zvpass uidbuf",uidbuf,0);
    if (!stricmp("anonymous",uidbuf)) {
        /* Anonymous login */

        if ( !ckxanon ) {
            printf("Login unavailable: anonymous login refused.\r\n");
            debug(F110,"zvpass()","invalid anonymous login refused",0);
            return(0);
        }

        if (!passwd[0]) {
            printf("Login refused: e-mail address must be provided as password.\r\n");
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                char buf[1024];
                sprintf(buf,"login: anonymous guests must specify a password");
                cksyslog(SYSLG_LI, 0, buf, NULL, NULL);
            }
#endif /* CKSYSLOG */
            return(0);
        }

        uid = anonacct ? anonacct : "guest";
        pwd = "";
        ckstrncpy(guestpass,passwd,GUESTPASS_LEN);
        isguest = 1;
    } else {
        uid = uidbuf;
        pwd = passwd ? passwd : "";
        guestpass[0] = '\0';
        isguest = 0;
    }

#ifdef NT
    if (pReferenceDomainName &&
        !ckstrcmp(uid,pReferenceDomainName,strlen(pReferenceDomainName),0) &&
         strlen(uid) != strlen(pReferenceDomainName) )
        uid = uid + strlen(pReferenceDomainName) + 1;
#endif /* NT */

    debug(F110,"zvpass",isguest ? guestpass : (pwd ? "xxxxxx" : "(null)"),isguest);

#ifdef NT
    if ( isWin95() ) {
        if (isguest || (ck_tn_auth_valid() == AUTH_VALID) && !pwd[0])
            /* already authenticated */
            goto logged_in;

        if (IsSSPLogonAvail()) {
            BOOL SSPLogonUser( LPTSTR DomainName, LPTSTR UserName, LPTSTR Password );
            const char * sspi_errstr(SECURITY_STATUS ss);

            LogonUserError = GetLastError();
            rc = SSPLogonUser(pReferenceDomainName?pReferenceDomainName:".",uid,pwd);
            if ( !rc ) { /* Failure */
                extern int SSPLogonError;
                printf("\r\n  %s\r\n",sspi_errstr(SSPLogonError));
#ifdef CKSYSLOG
                if (ckxsyslog >= SYSLG_LI && ckxlogging)
                    cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,
                              (char *)sspi_errstr(SSPLogonError));
#endif /* CKSYSLOG */
                debug(F110,"zvpass","SSPLogonUser failed",0);
                return(0);
            } else {
                ntlm_impersonate();
            }
        } else {
            debug(F110,"zvpass","SSPLogon not available",0);
#ifdef CK_SRP
            rc = t_verifypw (uid, pwd);
#ifndef PRE_SRP_1_4_4
            if (rc > -1)
                endtpent();
#endif /* PRE_SRP_1_4_4 */
            if (rc > 0) {
                srp_verified = 1;
                goto logged_in;
            }
#endif /* CK_SRP */

            printf("\r\n  Login denied\r\n");
#ifdef CKSYSLOG
            if (ckxsyslog >= SYSLG_LI && ckxlogging)
                cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,
                          "Login denied");
#endif /* CKSYSLOG */
            return(0);
        }
    } else {
        /* Get the User Info now so that we have the group list for the call */
        /* to CreateTokenForUser()                                           */
        GetUserInfo(pReferenceDomainName, uid);

        if (!isguest && (ck_tn_auth_valid() == AUTH_VALID) && !pwd[0]) {
            /* already authenticated */

            switch (ck_tn_authenticated()) {
            case AUTHTYPE_NTLM: {
                /* If we authenticated by NTLM SSPI then we do not have a Token yet    */
                /* So get one from the current thread after we impersonate the user    */
                HANDLE hThread, hThreadToken;
                DWORD foo;

		if ( !p_DuplicateTokenEx ) {
		    if ( !hAdvApi )
			hAdvApi = LoadLibrary("advapi32.dll");
		    if ( hAdvApi ) {
			(FARPROC) p_DuplicateTokenEx =
			    GetProcAddress( hAdvApi, "DuplicateTokenEx" );
		    }
		}

		ntlm_impersonate();
                hThread = GetCurrentThread();
                foo = OpenThreadToken(hThread, TOKEN_ALL_ACCESS, FALSE, &hThreadToken );
                debug(F111,"zvpass","OpenThreadToken()",foo ? 0 : GetLastError());
		if ( p_DuplicateTokenEx ) {
		    foo = p_DuplicateTokenEx(hThreadToken, MAXIMUM_ALLOWED, NULL, 
					     SecurityImpersonation,
					     TokenPrimary, &hLoggedOn);
		    debug(F111,"zvpass","DuplicateTokenEx()",foo ? 0 : GetLastError());
		}
                CloseHandle(hThreadToken);
                CloseHandle(hThread);
                ntlm = 1;
                break;
            }
#ifdef NTCREATETOKEN
            case AUTHTYPE_KERBEROS_V4:
            case AUTHTYPE_KERBEROS_V5:
            case AUTHTYPE_SRP:
                hLoggedOn = CreateTokenForUser(uid,pReferenceDomainName,
                                                pSid,pPriGroupSid,pTokenGroups);
                break;

            case AUTHTYPE_NULL:
            case AUTHTYPE_AUTO:
#ifdef CK_SSL
                if ((ssl_active_flag && ssl_get_subject_name(ssl_con))||
                     (tls_active_flag && ssl_get_subject_name(tls_con)))
                {
                    hLoggedOn = CreateTokenForUser(uid,pReferenceDomainName,
                                                    pSid,pPriGroupSid,pTokenGroups);
                    break;
                }
#endif /* CK_SSL */
#endif /* NTCREATETOKEN */
            default:
                rc = LogonUser(uid,pReferenceDomainName,pwd,
                                LOGON32_LOGON_INTERACTIVE,
                                LOGON32_PROVIDER_DEFAULT,
                                &hLoggedOn);
                if ( !rc ) {
                    debug(F110,"zvpass","AUTH_VALID login failed",0);

                    LastError = GetLastError();
                    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                       NULL,
                                       LastError,
                                       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                                       buf,
                                       4096,
                                       (va_list *) NULL
                                       ) )
                    {
                        printf("\r\n  %s\r\n",buf);
#ifdef CKSYSLOG
                        if (ckxsyslog >= SYSLG_LI && ckxlogging)
                            cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,buf);
#endif /* CKSYSLOG */
                    }
                    return(0);
                }
            }
        } else {
            rc = LogonUser(uid,
                            pReferenceDomainName,
                            pwd,
                            LOGON32_LOGON_INTERACTIVE,
                            LOGON32_PROVIDER_DEFAULT,
                            &hLoggedOn);
            debug(F111,"LogonUser()","rc",rc);
            if ( !rc ) {
                LastError = GetLastError();
                debug(F111,"zvpass LogonUser() failed",uid,LastError);

                if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   LastError,
                                   MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                                   buf,
                                   4096,
                                   (va_list *) NULL
                                   ) )
                {
                    printf("\r\n  %s\r\n",buf);
#ifdef CKSYSLOG
                        if (ckxsyslog >= SYSLG_LI && ckxlogging)
                            cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,buf);
#endif /* CKSYSLOG */
                }
                return(0);
            }
        }

        /* Get the user's profile and environment information */
        /* If we have a Token for the user, then attempt to retrieve the  */
        /* user's profile (including home directory) and the Environment  */
        /* variables.  Be sure to do this before we Impersonate the user  */
        if ( p_LoadUserProfileA ) {
            profinfo.dwFlags = PI_NOUI;
            profinfo.lpUserName = uid;
            profinfo.lpServerName = pPDCName;

            debug(F100,"calling LoadUserProfileA()","",0);
            rc = p_LoadUserProfileA(hLoggedOn,&profinfo);
            debug(F111,"LoadUserProfileA() returns","rc",rc);
            LastError = GetLastError();
            if ( !rc && LastError )
            {
                if ( LastError != ERROR_INVALID_HANDLE &&
                     FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    LastError,
                                    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                                    buf,
                                    4096,
                                    (va_list *) NULL
                                    ) )
                {
                    printf("\r\n  Profile not loaded: %s\r\n",buf);
#ifdef CKSYSLOG
                    if (ckxsyslog >= SYSLG_LI && ckxlogging)
                        cksyslog(SYSLG_LI, 0, "PROFILE LOAD FAILURE",uid,buf);
#endif /* CKSYSLOG */
                }
                else {
#ifdef BETATEST
                    printf("\r\nLoadUserProfile Error: Invalid Handle\r\n");
#endif /* BETATEST */
#ifdef COMMENT
#ifdef CKSYSLOG
                    if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                        sprintf(buf,"LoadUserProfile() Error = %d",LastError);
                        cksyslog(SYSLG_LI, 0, "PROFILE LOAD FAILURE",uid,buf);
                    }
#endif /* CKSYSLOG */
#endif /* COMMENT */
                }
                debug(F111,"zvpass","LoadUserProfile failed",LastError);
            }
        }

        if ( p_CreateEnvironmentBlock ) {
            debug(F100,"calling CreateEnvironmentBlock()","",0);
            rc = p_CreateEnvironmentBlock(&pEnvBlock,hLoggedOn,0);
            debug(F111,"CreateEnvironmentBlock() returns","rc",rc);
            if ( rc ) {
                char buf[1024];
                unsigned char *p, *q;
                unsigned short ch;
                int i;

                q = (char *) &ch;
                p = (char *) pEnvBlock;
                q[0] = p[0];
                q[1] = p[1];
                if ( ch ) {
                    i = 0;
                    buf[0] = '\0';
                    do {
                        if ( ch ) {
                            if ( ch >= 256 )
                                buf[i] = ((ch / 256) & 0xFF);
                            else
                                buf[i] = (ch & 0xFF);
                            i++;
                        } else {
                            buf[i] = '\0';
                            if ( buf[0] )
                                _putenv(buf);
                            i = 0;
                        }
                        p += 2;
                        q[0] = p[0];
                        q[1] = p[1];
                    } while ( !((i == 0) && (ch == 0) ) );
                }
            } else
                pEnvBlock = NULL;
        }

        if ( !ntlm ) {
            /* Impersonate the user */
            debug(F100,"calling ImpersonateLoggedOnUser()","",0);
            rc = ImpersonateLoggedOnUser(hLoggedOn);
            debug(F111,"ImpersonateLoggedOnUser() returns","rc",rc);
            if ( !rc ) {
                LastError = GetLastError();
                if ( LastError != ERROR_INVALID_HANDLE &&
                     FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    LastError,
                                    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                                    buf,
                                    4096,
                                    (va_list *) NULL
                                    ) )
                {
                    printf("\r\n  %s\r\n",buf);
#ifdef CKSYSLOG
                    if (ckxsyslog >= SYSLG_LI && ckxlogging)
                        cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,buf);
#endif /* CKSYSLOG */
                } else {
#ifdef BETATEST
#endif /* BETATEST */
#ifdef CKSYSLOG
                    if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                        sprintf(buf,"ImpersonateLoggedOnUser() Error = %d",LastError);
                        cksyslog(SYSLG_LI, 0, "LOGIN FAILURE",uid,buf);
                    }
#endif /* CKSYSLOG */
                }
                debug(F111,"zvpass","ImpersonateLoggedOnUser failed",LastError);
                return(0);
            }
        }
    }
  logged_in:
    debug(F110,"zvpass()","logged_in",0);
    logged_in = 1;

    if ( HomeDir[0] ) {
        int i;

        ckstrncpy(homedir,HomeDir,CKMAXPATH+1);
#ifdef NT
        GetShortPathName(homedir,homedir,CKMAXPATH);
#endif /* NT */

        /* we know have the directory, but need to make it consistent */
        /* with all of the other Kermit directory variables: forward  */
        /* slashes and a trailing slash.                              */
        i = strlen(homedir)-1;
        if ( !(ISDIRSEP(homedir[i])) ) {
            homedir[i+1] = '/';
            homedir[i+2] = NUL;
        }
        for (i-- ; i >= 0 ; i--) {
            if ( homedir[i] == '\\' )
                homedir[i] = '/';
        }
    } else
        ckstrncpy(homedir,zgtdir(),CKMAXPATH+1);

    debug(F110,"zvpass() homedir",homedir,0);

    zchdir(zhome());

    if ( isguest ) {
#ifdef CKROOT
        if ( anonroot ) {
            if (zsetroot(anonroot) < 0) {
                printf("Login unavailable: invalid anonymous root directory\r\n");
                debug(F110,"zvpass()","invalid anonymous root directory",0);
                return(0);
            }
        }
#else /* CKROOT */
        debug(F110,"zvpass()","no anonymous root support",0);
#endif /* CKROOT */
    }
#else /* NT */
    if (!isguest && ck_tn_auth_valid() == AUTH_VALID &&
         (!passwd || !(*passwd))) { /* already authenticated */
        switch (ck_tn_authenticated()) {
        case AUTHTYPE_NTLM:
        case AUTHTYPE_KERBEROS_V4:
        case AUTHTYPE_KERBEROS_V5:
        case AUTHTYPE_SRP:
            break;
        case AUTHTYPE_NULL:
        case AUTHTYPE_AUTO:
#ifdef CK_SSL
            if ((ssl_active_flag && ssl_get_subject_name(ssl_con))||
                 (tls_active_flag && ssl_get_subject_name(tls_con)))
                break;
#endif /* CK_SSL */
        default:
            debug(F110,"zvpass","AUTH_VALID login failed",0);
            return(0);
        }
    } else {
#ifdef CK_SRP
        rc = t_verifypw (uid, pwd);
#ifndef PRE_SRP_1_4_4
        if (rc > -1)
            endtpent();
#endif /* PRE_SRP_1_4_4 */
        if (rc <= 0)
#endif /* CK_SRP */
        {
            debug(F110,"zvpass","login failed",0);
            return(0);
        }
    }

#ifdef CKROOT
    if ( isguest ) {
        if ( anonroot ) {
            if (zsetroot(anonroot) < 0) {
                printf("Login unavailable: invalid anonymous root directory\r\n");
                debug(F110,"zvpass()","invalid anonymous root directory",0);
                return(0);
            }
        }
    }
#endif /* CKROOT */
#endif /* NT */

#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_LI && ckxlogging)
        cksyslog(SYSLG_LI, 1, "LOGIN",uid,clienthost);
#endif /* CKSYSLOG */

#ifdef IKSDB
    if (ikdbopen) {
        char * p, * q;
        int k;
        extern char dbrec[];
        extern unsigned long myflags, mydbslot;
        extern struct iksdbfld dbfld[];
        extern unsigned long myaflags, myamode, myatype;

        debug(F101,"XXX guest","",isguest);
        debug(F110,"XXX zvuname",uid,0);
        debug(F110,"XXX guestpass",guestpass,0);

        myflags |= DBF_LOGGED;
        p = isguest ? guestpass : uid;
        if (isguest) {
            p = (char *)guestpass;
            myflags &= ~DBF_USER;
        } else {
            p = (char *)uid;
            myflags |= DBF_USER;
        }
        k = strlen(p);
        strncpy(&dbrec[DB_ULEN],ulongtohex((unsigned long)k,4),4);
        lset(&dbrec[dbfld[db_USER].off],p,1024,32);
        strncpy(&dbrec[DB_FLAGS],ulongtohex(myflags,4),4);
#ifdef CK_AUTHENTICATION
        myamode = ck_tn_auth_valid();
        strncpy(&dbrec[DB_AMODE],ulongtohex(myamode,4),4);
        myatype = ck_tn_authenticated();
        strncpy(&dbrec[DB_ATYPE],ulongtohex(myatype,4),4);
#endif /* CK_AUTHENTICATION */
        p = zgtdir();

        strncpy(&dbrec[DB_DLEN],
                ulongtohex((unsigned long)strlen(p),4),
                4
                );
        lset(&dbrec[dbfld[db_DIR].off],p,1024,32);
        updslot(mydbslot);
    }
#endif /* IKSDB */
    debug(F110,"zvpass()","success",0);
    return(1);
}

VOID
zvlogout() {
#ifdef NT
    extern int haveNTLMContext;
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_LI && ckxlogging) {
        cksyslog(SYSLG_LI, 1, "LOGOUT",(char *) isguest ? anonacct : uidbuf,
                  clienthost);
    }
#endif /* CKSYSLOG */

    if ( haveNTLMContext ) {
        ntlm_logout();
    }
    if ( hLoggedOn != INVALID_HANDLE_VALUE ) {
        RevertToSelf();

        if ( p_UnloadUserProfileA && profinfo.hProfile )
            p_UnloadUserProfileA(hLoggedOn, profinfo.hProfile);
        memset(&profinfo,0,sizeof(profinfo));
        profinfo.dwSize = sizeof(profinfo);

        if ( p_DestroyEnvironmentBlock && pEnvBlock ) {
            p_DestroyEnvironmentBlock(pEnvBlock);
            pEnvBlock = NULL;
        }

        CloseHandle(hLoggedOn);
        hLoggedOn = INVALID_HANDLE_VALUE;
    }
    if (pSid) {
        free(pSid);
        pSid = NULL;
    }
    if ( pReferenceDomainName ) {
        free(pReferenceDomainName);
        pReferenceDomainName = NULL;
    }
#endif /* NT */
    homedir[0] = '\0';
    uidbuf[0] = '\0';
    isguest = 0;
    logged_in = 0;
    return;
}

#ifdef NT
/* From Peter Runestig */

static HINSTANCE hNetApi32=NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetUserGetInfo)( LPWSTR, LPWSTR, DWORD, LPBYTE *)=NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetGetDCName)(LPCWSTR, LPCWSTR, LPBYTE *)=NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetApiBufferFree)( LPVOID ) = NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetUserGetLocalGroups)(LPCWSTR, LPCWSTR, DWORD, DWORD, LPBYTE *,
                                             DWORD, LPDWORD, LPDWORD)=NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetUserGetGroups)(LPCWSTR, LPCWSTR, DWORD, LPBYTE *,
                                             DWORD, LPDWORD, LPDWORD)=NULL;
static NET_API_STATUS
(NET_API_FUNCTION *p_NetGroupGetInfo)(LPCWSTR, LPCWSTR, DWORD, LPBYTE)=NULL;


static int
LoadNetApi32()
{
    if ( !hNetApi32 ) {
        debug(F110,"LoadNetApi32","loading netapi32.dll",0);
        hNetApi32 = LoadLibrary( "netapi32.dll" );
        if ( hNetApi32 ) {
            (FARPROC) p_NetUserGetInfo = GetProcAddress( hNetApi32, "NetUserGetInfo" );
            (FARPROC) p_NetApiBufferFree = GetProcAddress( hNetApi32, "NetApiBufferFree" );
            (FARPROC) p_NetGetDCName = GetProcAddress( hNetApi32, "NetGetDCName" );
            (FARPROC) p_NetUserGetLocalGroups = GetProcAddress( hNetApi32, "NetUserGetLocalGroups" );
            (FARPROC) p_NetUserGetGroups = GetProcAddress( hNetApi32, "NetUserGetGroups" );
            (FARPROC) p_NetGroupGetInfo = GetProcAddress( hNetApi32, "NetGroupGetInfo" );

            if ( !p_NetUserGetInfo || !p_NetApiBufferFree || !p_NetGetDCName ||
                 !p_NetUserGetLocalGroups || !p_NetUserGetGroups || !p_NetGroupGetInfo )
            {
                debug(F110,"LoadNetApi32","loading netapi32.dll unable to load function",0);
                FreeLibrary(hNetApi32);
                hNetApi32 = NULL;
            }
        }
        if ( !hNetApi32 ) {
            debug(F110,"LoadNetApi32","loading netapi32.dll failed",0);
            return(0);
        }
    }
    return(1);
}

static int
_GetUserGroups(LPWSTR wCompName, LPWSTR wUserName)
{
    int err;
    LPLOCALGROUP_USERS_INFO_0 lpLocalGroupInfo=NULL;
    LPGROUP_USERS_INFO_0 lpGroupInfo=NULL;
    /*
     * typedef struct _LOCAL_GROUP_USERS_INFO_0 {
     *   LPWSTR  lgrui0_name;
     * } LOCALGROUP_USERS_INFO_0, *PLOCALGROUP_USERS_INFO_0, *LPLOCALGROUP_USERS_INFO_0;
     *
     * typedef struct _GROUP_USERS_INFO_0 {
     *   LPWSTR  grui0_name;
     * } GROUP_USERS_INFO_0, *PGROUP_USERS_INFO_0, *LPGROUP_USERS_INFO_0;
     */
    DWORD localEntriesRead, localEntriesTotal, remoteEntriesRead, remoteEntriesTotal;
    DWORD i,j;
    CHAR buf[256], compName[256];
    char group[UNLEN], domain[255];
    unsigned char sid[MAXSIZE], *sc;
    DWORD sz_sid = MAXSIZE, sz_domain = sizeof(domain), *sa;
    SID_NAME_USE sid_use;


    UnicodeToOEM(wCompName,compName,256);
    err = p_NetUserGetLocalGroups(wCompName, wUserName, 0, LG_INCLUDE_INDIRECT,
                                   (LPBYTE *)&lpLocalGroupInfo,
                                   MAX_PREFERRED_LENGTH, &localEntriesRead, &localEntriesTotal);
    switch ( err ) {
    case NERR_Success:
        debug(F111,"_GetUserGroups","localEntriesRead",localEntriesRead);
        debug(F111,"_GetUserGroups","localEntriesTotal",localEntriesTotal);
        break;
    case ERROR_ACCESS_DENIED:
        debug(F110,"_GetUserGroups","Access Denied",0);
        localEntriesRead = 0;
        break;
    case ERROR_MORE_DATA:
        debug(F110,"_GetUserGroups","More Data",0);
        localEntriesRead = 0;
        break;
    case NERR_InvalidComputer:
        debug(F110,"_GetUserGroups","Invalid Computer",0);
        localEntriesRead = 0;
        break;
    case NERR_UserNotFound:
        debug(F110,"_GetUserGroups","User Not Found",0);
        localEntriesRead = 0;
        break;
    default:
        debug(F111,"_GetUserGroups","other error",err);
        localEntriesRead = 0;
    }

    err = p_NetUserGetGroups(wCompName, wUserName, 0,
                              (LPBYTE *)&lpGroupInfo,
                              MAX_PREFERRED_LENGTH, &remoteEntriesRead, &remoteEntriesTotal);
    switch ( err ) {
    case NERR_Success:
        debug(F111,"_GetUserGroups","remoteEntriesRead",remoteEntriesRead);
        debug(F111,"_GetUserGroups","remoteEntriesTotal",remoteEntriesTotal);
        break;
    case ERROR_ACCESS_DENIED:
        debug(F110,"_GetUserGroups","Access Denied",0);
        remoteEntriesRead = 0;
        break;
    case ERROR_MORE_DATA:
        debug(F110,"_GetUserGroups","More Data",0);
        remoteEntriesRead = 0;
        break;
    case NERR_InvalidComputer:
        debug(F110,"_GetUserGroups","Invalid Computer",0);
        remoteEntriesRead = 0;
        break;
    case NERR_UserNotFound:
        debug(F110,"_GetUserGroups","User Not Found",0);
        remoteEntriesRead = 0;
        break;
    default:
        debug(F111,"_GetUserGroups","other error",err);
        remoteEntriesRead = 0;
    }

    pTokenGroups = (PTOKEN_GROUPS) malloc(sizeof(DWORD)
                 + (localEntriesRead + remoteEntriesRead) * sizeof(SID_AND_ATTRIBUTES));
    pTokenGroups->GroupCount = localEntriesRead + remoteEntriesRead;

    for ( i=0, j=0; i<localEntriesRead ; i++,j++ ) {
        UnicodeToOEM(lpLocalGroupInfo[i].lgrui0_name,buf,256);
#ifdef COMMENT
        printf("Local Groups: %s\r\n",buf);
#endif /* COMMENT */
        debug(F110,"_GetUserGroups LocalGroup",buf,0);
        // get the SID for each group
        UnicodeToOEM(lpLocalGroupInfo[i].lgrui0_name, group, sizeof(group));
        sz_sid = MAXSIZE;
        err = LookupAccountName(compName, group, (PSID) sid, &sz_sid, domain,
                                 &sz_domain, &sid_use);

        debug(F110,"_GetUserGroups LookupAccountName() domain",domain,0);

        pTokenGroups->Groups[j].Sid = (PSID) malloc( sz_sid );
        CopySid( sz_sid, pTokenGroups->Groups[j].Sid, sid );
        pTokenGroups->Groups[j].Attributes = SE_GROUP_ENABLED;

        // compare the last subauthority in the group's SID with the primary
        // group RID from the USER_INFO_3 struct
        sc = GetSidSubAuthorityCount((PSID) sid);
        sa = GetSidSubAuthority((PSID) sid, *sc - 1);
        if (*sa == PrimaryGroupId) {
            pPriGroupSid = (PSID) malloc( sz_sid );
            CopySid( sz_sid, pPriGroupSid, sid );
#ifdef COMMENT
            printf("Primary group is %s!\n", group);
#endif /* COMMENT */
            debug(F110,"_GetUserGroups PrimaryLocalGroup",buf,0);
        }
    }
    for ( i=0; i<remoteEntriesRead ; i++, j++ ) {
        UnicodeToOEM(lpGroupInfo[i].grui0_name,buf,256);
#ifdef COMMENT
        printf("Groups: %s\r\n",buf);
#endif /* COMMENT */
        debug(F110,"_GetUserGroups DomainGroup",buf,0);
        // get the SID for each group
        UnicodeToOEM(lpGroupInfo[i].grui0_name, group, sizeof(group));
        sz_sid = MAXSIZE;
        err = LookupAccountName(compName, group, (PSID) sid, &sz_sid, domain,
                                 &sz_domain, &sid_use);

        debug(F110,"_GetUserGroups LookupAccountName() domain",domain,0);

        pTokenGroups->Groups[j].Sid = (PSID) malloc( sz_sid );
        CopySid( sz_sid, pTokenGroups->Groups[j].Sid, sid );
        pTokenGroups->Groups[j].Attributes = SE_GROUP_ENABLED;

        // compare the last subauthority in the group's SID with the primary
        // group RID from the USER_INFO_3 struct
        sc = GetSidSubAuthorityCount((PSID) sid);
        sa = GetSidSubAuthority((PSID) sid, *sc - 1);
        if (*sa == PrimaryGroupId) {
            pPriGroupSid = (PSID) malloc( sz_sid );
            CopySid( sz_sid, pPriGroupSid, sid );
#ifdef COMMENT
            printf("Primary group is %s!\n", group);
#endif /* COMMENT */
            debug(F110,"_GetUserGroups DomainLocalGroup",buf,0);
        }
    }

    p_NetApiBufferFree(lpGroupInfo);
    p_NetApiBufferFree(lpLocalGroupInfo);

    return(err);
}

static int
_GetUserInfo(LPWSTR wCompName, LPWSTR wUserName)
{
  int err;
  LPUSER_INFO_3 UserInfo = NULL;
/*typedef struct _USER_INFO_3 {
    LPWSTR    usri3_name;
    LPWSTR    usri3_password;
    DWORD     usri3_password_age;
    DWORD     usri3_priv;
    LPWSTR    usri3_home_dir;
    LPWSTR    usri3_comment;
    DWORD     usri3_flags;
    LPWSTR    usri3_script_path;
    DWORD     usri3_auth_flags;
    LPWSTR    usri3_full_name;
    LPWSTR    usri3_usr_comment;
    LPWSTR    usri3_parms;
    LPWSTR    usri3_workstations;
    DWORD     usri3_last_logon;
    DWORD     usri3_last_logoff;
    DWORD     usri3_acct_expires;
    DWORD     usri3_max_storage;
    DWORD     usri3_units_per_week;
    PBYTE     usri3_logon_hours;
    DWORD     usri3_bad_pw_count;
    DWORD     usri3_num_logons;
    LPWSTR    usri3_logon_server;
    DWORD     usri3_country_code;
    DWORD     usri3_code_page;
    DWORD     usri3_user_id;
    DWORD     usri3_primary_group_id;
    LPWSTR    usri3_profile;
    LPWSTR    usri3_home_dir_drive;
    DWORD     usri3_password_expired;
}USER_INFO_3, *PUSER_INFO_3, *LPUSER_INFO_3;*/

    err = p_NetUserGetInfo(wCompName, wUserName, 3, (LPBYTE *)&UserInfo);
    debug(F111,"_GetUserInfo","NetUserGetInfo returns",err);
    if (!err) {
        debug(F111,"_GetUserInfo","UserInfo",UserInfo);
        UnicodeToOEM(UserInfo->usri3_home_dir, HomeDir, sizeof(HomeDir));
        debug(F110,"_GetUserInfo HomeDir",HomeDir,0);
        UnicodeToOEM(UserInfo->usri3_profile, ProfilePath, sizeof(ProfilePath));
        debug(F110,"_GetUserInfo ProfilePath",ProfilePath,0);
        PrimaryGroupId = UserInfo->usri3_primary_group_id;
        p_NetApiBufferFree(UserInfo);
    }
    return err;
}

int
GetUserInfo(LPSTR DomainName, LPSTR UserName)
{
    unsigned short wDomainName[128], wUserName[128], *wPDCName;
    unsigned char  LocalMachine[128]="";
    DWORD dwSize = 128;
    int err;

    if ( !LoadNetApi32() )
        return(-1);

    wDomainName[0] = 0;
    wUserName[0] = 0;

    OEMToUnicode(UserName, wUserName, sizeof(wUserName));

    debug(F110,"GetUserInfo DomainName",DomainName,0);
    debug(F110,"GetUserInfo UserName",UserName,0);

    if (DomainName) {
        GetComputerName(LocalMachine,&dwSize);
        if ( ckstrcmp(DomainName,LocalMachine, -1, 0) )
            OEMToUnicode(DomainName, wDomainName, sizeof(wDomainName));
    }

    debug(F110,"GetUserInfo","calling _GetUserInfo() with wDomainName",0);
    if (_GetUserInfo(wDomainName[0] ? wDomainName : NULL, wUserName)) {
        debug(F110,"GetUserInfo","calling NetGetDCName()",0);
        err = p_NetGetDCName(NULL, wDomainName, (LPBYTE *)&wPDCName);
        debug(F111,"GetUserInfo","NetGetDCName() returns",err);
        if (!err) {
            debug(F110,"GetUserInfo","calling _GetUserInfo() with wPDCName",0);
            if(!_GetUserInfo(wPDCName, wUserName)) {
                _GetUserGroups(wPDCName, wUserName);

                /* Lets save the PDCName */
                UnicodeToOEM(wPDCName, LocalMachine, sizeof(LocalMachine));
                makestr(&pPDCName,LocalMachine);
                debug(F110,"GetUserInfo PDCName",pPDCName,0);
            }
            p_NetApiBufferFree(wPDCName);
        } else {
            debug(F110,"GetUserInfo","calling _GetUserInfo() with NULL",0);
            if(!_GetUserInfo(NULL, wUserName)) {
                _GetUserGroups(NULL, wUserName);
            }
        }
    } else {
        _GetUserGroups(wDomainName, wUserName);
    }
    debug(F100,"GetUserInfo returns","",0);
    return 0;
}

VOID
setntcreds() {
    extern int haveNTLMContext;

    if ( !logged_in )
        return;

    if ( hLoggedOn != INVALID_HANDLE_VALUE ) {
        ImpersonateLoggedOnUser(hLoggedOn);
    } else if ( haveNTLMContext ) {
        ntlm_impersonate();
    }
#ifdef CK_AUTHENTICATE
    else if ( ck_tn_auth_valid() == AUTH_VALID &&
              ck_tn_authenticated() == AUTHTYPE_NTLM ) {
        ntlm_impersonate();
    }
#endif /* CK_AUTHENTICATE */
}
#endif /* NT */
#endif /* CK_LOGIN */

#ifdef CKSYSLOG
HANDLE hSysLog = INVALID_HANDLE_VALUE;
static char pidstr[16]="";

VOID
zsyslog() {
    if (ckxsyslog && !ckxlogging) {
        sprintf(pidstr,"PID=%d",os2getpid());
#ifdef NT
        if ( !isWin95() ) {
            hSysLog = RegisterEventSource(NULL,
#ifdef IKSD
                                           inserver ? "IKSD" :
#endif /* IKSD */
                                           "K95");
            ckxlogging = 1;
            debug(F100,"zsyslog syslog opened","",0);
        } else
#endif /* NT */
        {
            printf("syslog not available\r\n");
            debug(F100,"zsyslog syslog not opened","",0);
        }
    }
}
/*
  C K S Y S L O G  --  C-Kermit system logging function,

  For use by other modules.
  This module can, but doesn't have to, use it.
  Call with:
    n = SYSLG_xx values defined in ckcdeb.h
    s1, s2, s3: strings.
*/
VOID
cksyslog(n, m, s1, s2, s3) int n, m; char * s1, * s2, * s3; {
#ifdef IKSDB
    extern int what, ikdbopen;
#endif /* IKSDB */
    int level;
    char * messages[5];

#ifdef IKSDB
    if (inserver && ikdbopen) {
        slotstate(what,s1,s2,s3);
    }
#endif /* IKSDB */
    if (!ckxlogging) return;

    if (!s1) s1 = "";
    if (!s2) s2 = "";
    if (!s3) s3 = "";

    if ( n == SYSLG_LI ) {
        if ( m )
            level = EVENTLOG_AUDIT_SUCCESS;
        else
            level = EVENTLOG_AUDIT_FAILURE;
    } else {
        level = EVENTLOG_INFORMATION_TYPE;
    }

    debug(F110,"cksyslog s1",s1,0);
    debug(F110,"cksyslog s2",s2,0);
    debug(F110,"cksyslog s3",s3,0);

    messages[0] = pidstr;
    messages[1] = s1;
    messages[2] = s2;
    messages[3] = s3;
    messages[4] = "\0";

    ReportEvent(hSysLog,level, 0,0,
#ifdef CK_LOGIN
                 pSid,
#else /* CK_LOGIN */
                 NULL,
#endif /* CK_LOGIN */
                 5,
                 0,
                 messages,
                 NULL
                 );
    debug(F101,"cksyslog errno","",GetLastError());
}
#endif /* CKSYSLOG */


/*  Z C H D S K  --  Change currently selected disk device */

/* Returns -1 if error, otherwise 0 */

zchdsk(c) int c; {
    int i = toupper(c) - 64;
    return( _chdrive(i));
}

#ifdef NT
long
StreamSize(char * filename, char * streamname)
{
    HANDLE hf ;
    DWORD lasterror = 0 ;
    WIN32_STREAM_ID sid;
    DWORD dwStreamHeaderSize, dwRead, dw1, dw2;
    BOOL  bContinue;
    VOID *lpContext = NULL;
    WCHAR wszStreamName[CKMAXPATH+1];
    WCHAR wszStat[CKMAXPATH+1];
    char  stream[CKMAXPATH+1];
    int   i, diff;
    DWORD dwStreamId = 0xFFFFFFFF;
    long  size = -1;

    ckmakmsg(stream,CKMAXPATH+1,":",streamname,":$DATA",NULL);
    ANSIToUnicode(stream,wszStat,sizeof(wszStat));

    hf = CreateFile( filename,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow Sharing */
                     NULL, /* No Security Attributes specified */
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL ) ;

    if ( hf != INVALID_HANDLE_VALUE ) {
        ZeroMemory(&sid, sizeof(WIN32_STREAM_ID));
        dwStreamHeaderSize = (LPBYTE)&sid.cStreamName - (LPBYTE)&sid;

        for ( ;; ) {
            bContinue = BackupRead(hf, (LPBYTE) &sid, dwStreamHeaderSize,
                                    &dwRead, FALSE, FALSE, &lpContext);
            if (!bContinue ||
                 sid.dwStreamNameSize > sizeof(wszStreamName))
                break;

            dwStreamId = sid.dwStreamId;
            bContinue = BackupRead(hf, (LPBYTE) &wszStreamName,
                                    sid.dwStreamNameSize,
                                    &dwRead, FALSE, FALSE, &lpContext);
            if ( !bContinue )
                break;

            if (sid.dwStreamNameSize) {
                wszStreamName[sid.dwStreamNameSize/2] = 0;
                UnicodeToANSI(wszStreamName,stream,sizeof(stream));
                debug(F110,"StreamSize ntfs stream found",stream,0);
                if (!_wcsicmp(wszStreamName,wszStat)) {
                    size = (sid.Size.HighPart << 32) + sid.Size.LowPart;
                    break;
                }
            }
            if (!BackupSeek(hf, sid.Size.LowPart, sid.Size.HighPart, &dw1, &dw2, &lpContext))
                break;
        }

        /* Close the backup context */
        BackupRead(hf, (LPBYTE) &sid, 0,
                    &dwRead, TRUE, FALSE, &lpContext);
        CloseHandle(hf);
    }
    return size;
}

#ifdef STREAMS
const char *
StreamType(DWORD StreamId)
{
    static char msg[32];

    switch (StreamId) {
    case BACKUP_INVALID:
        return "Invalid stream";
    case BACKUP_DATA:
        return "Standard data";
    case BACKUP_EA_DATA:
        return "Extended attribute data";
    case BACKUP_SECURITY_DATA:
        return "Security descriptor data";
    case BACKUP_ALTERNATE_DATA:
        return "Alternative data streams";
    case BACKUP_LINK:
        return "Hard link information";
    case BACKUP_PROPERTY_DATA:
        return "Property data";
    case BACKUP_OBJECT_ID:
        return "Objects identifiers";
    case BACKUP_REPARSE_DATA:
        return "Reparse points";
    case BACKUP_SPARSE_BLOCK:
        return "Sparse file.";
    default:
        sprintf(msg,"StreamType %u", StreamId);
        return msg;
    }
}

long
StreamCount(char * path, char * relpath, char * filename, char * pattern)
{
    HANDLE hf ;
    DWORD lasterror = 0 ;
    WIN32_STREAM_ID sid;
    DWORD dwStreamHeaderSize, dwRead, dw1, dw2;
    BOOL  bContinue;
    VOID *lpContext = NULL;
    WCHAR wszStreamName[CKMAXPATH+1];
    char  szStreamName[CKMAXPATH+1];
    char  streamsearch[CKMAXPATH+1], fullpath[CKMAXPATH+1];
    int   i, diff;
    long  count = 0;

    if ( !pattern )
        pattern = ":*";

    ckmakmsg(streamsearch,CKMAXPATH+1,pattern,":$DATA",NULL,NULL);
    ckmakmsg(fullpath,CKMAXPATH+1,path,relpath,filename,NULL);

    hf = CreateFile( fullpath,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, /* Allow Sharing */
                     NULL, /* No Security Attributes specified */
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL ) ;

    if ( hf != INVALID_HANDLE_VALUE ) {
        dwStreamHeaderSize = (LPBYTE)&sid.cStreamName - (LPBYTE)&sid;

        for ( ;; ) {
            ZeroMemory(&sid, sizeof(WIN32_STREAM_ID));
            wszStreamName[0] = 0;

            bContinue = BackupRead(hf, (LPBYTE) &sid, dwStreamHeaderSize,
                                    &dwRead, FALSE, FALSE, &lpContext);
            if (!bContinue || dwRead == 0 ||
                sid.dwStreamNameSize > sizeof(wszStreamName))
                break;

            bContinue = BackupRead(hf, (LPBYTE) &wszStreamName,
                                    sid.dwStreamNameSize,
                                    &dwRead, FALSE, FALSE, &lpContext);
            if ( !bContinue )
                break;

            debug(F111,"StreamSize ntfs stream found",szStreamName,sid.dwStreamId);
            if (sid.dwStreamNameSize) {
                wszStreamName[sid.dwStreamNameSize/2] = 0;
                UnicodeToANSI(wszStreamName,szStreamName,CKMAXPATH+1);
                if (ckmatch(streamsearch, szStreamName, 0, 1)) {
                    count++;
                }
            } else {
                if ( sid.dwStreamId == BACKUP_DATA &&
                     (!pattern[0] || 
                      pattern[0] == ':' && pattern[1] == '*' && !pattern[2]) )  {
                    count++;
                }
            }

            if (!BackupSeek(hf, sid.Size.LowPart, sid.Size.HighPart, &dw1, &dw2, &lpContext))
                break;
        }

        /* Close the backup context */
        BackupRead(hf, (LPBYTE) &sid, 0,
                    &dwRead, TRUE, FALSE, &lpContext);
        CloseHandle(hf);
    }

    return(count);
}
#endif /* STREAMS */
#endif /* NT */

int
os2stat(char *path, struct stat *st) {
    char local[MAXPATH];
    int len;
#ifdef NT
    int stream_idx=0;
#endif /* NT */
    int rc;

    if (!path)
        return(-1);

    if ( !strncmp(path,"//",2) || !strncmp(path,"\\\\",2)) {
        int i;
        len = ckstrncpy(local, UNCname(path), MAXPATH);
        if ( ISDIRSEP(local[len-1]) ) {
            /* We now need to strip the trailing directory separator if it is not */
            /* part of the machine or object name.                                */
            for ( i=2; i<len && !(ISDIRSEP(local[i])); i++ ); /* machine */
            for ( i++; i<len && !(ISDIRSEP(local[i])); i++ ); /* object */

            if (i < len-1 )
                local[len-1] = '\0';
        }
    } else {
        len = ckstrncpy(local, path, MAXPATH);

        if ( len == 2 && local[1] == ':' )
            local[2] = DIRSEP, local[3] = 0; /* if drive only, append / */
        else if ( len == 0 )
            local[0] = DIRSEP, local[1] = 0; /* if empty path, take / instead */
        else if ( len > 1 && ISDIRSEP(local[len - 1]) && local[len - 2] != ':')
            local[len - 1] = 0; /* strip trailing / except after d: */

#ifdef NT
        /* NTFS supports streams which are indicated by
         *    filename:stream
         */

        for (stream_idx = len; stream_idx > 1 ; stream_idx--) {
            if (local[stream_idx] == ':') {     /* found a stream */
                local[stream_idx] = NUL;
                break;
            }
        }
#endif /* NT */
    }
    rc = stat(local, st);
    debug(F111,"os2stat","rc",rc);
#ifdef NT
    if ( stream_idx > 1 ) {
        long size = StreamSize(local,&local[stream_idx+1]);
        if ( size >= 0 )
            st->st_size = size;
        else
            rc = -1;
    }
#endif /* NT */
    return(rc);
}

#ifdef CK_LABELED
/* O S 2 S E T L O N G N A M E -- Set .LONGNAME Extended Attribute */

/* Returns -1 if error, otherwise 0 */

int
os2setlongname( char * fn, char * ln ) {
   EAOP2         FileInfoBuf;
   ULONG         FileInfoSize;
   ULONG         PathInfoFlags;
   APIRET        rc;
   char          FEA2List[1024];
   FEA2 *        pfea2;
   WORD *        pEAdata;

   debug(F110,"os2setlongname: filename is",fn,0);
   debug(F110,"os2setlongname: longname is",ln,0);

   FileInfoSize = sizeof( EAOP2 );
   PathInfoFlags = DSPI_WRTTHRU; /* complete write operation before return */

   FileInfoBuf.fpGEA2List = 0;
   FileInfoBuf.fpFEA2List = (PFEA2LIST) &FEA2List;
   pfea2 = FileInfoBuf.fpFEA2List->list;

   pfea2->oNextEntryOffset = 0;
   pfea2->fEA = 0;
   pfea2->cbName = 9; /* length of ".LONGNAME" without \0 */
   pfea2->cbValue = strlen( ln ) + 2 * sizeof( WORD );
   strcpy( pfea2->szName, ".LONGNAME" );

   pEAdata = (WORD *) pfea2->szName + 10/sizeof(WORD);
   *pEAdata = EAT_ASCII;
   pEAdata++;
   *pEAdata = strlen( ln );
   pEAdata++;
   strcpy( (char *) pEAdata, ln );
   pEAdata += (strlen( ln )+1)/sizeof(WORD);

   FileInfoBuf.fpFEA2List->cbList = (char *) pEAdata -
                                    (char *) FileInfoBuf.fpFEA2List;

   rc = DosSetPathInfo( fn, 2, &FileInfoBuf, FileInfoSize, PathInfoFlags );
   debug(F101,"os2setlongname: rc=","",rc);
   if ( !rc )
      return 0;
   else
      return -1;
}

/* O S 2 G E T L O N G N A M E -- Get .LONGNAME Extended Attribute */

/* Returns -1 if error, otherwise 0 */

int
os2getlongname( char * fn, char ** ln ) {
   static char   * LongNameBuf = 0;
   EAOP2         FileInfoBuf;
   ULONG         FileInfoSize;
   ULONG         PathInfoFlags;
   APIRET        rc;
   char          FEA2List[1024];
   FEA2 *        pfea2;
   char          GEA2List[1024];
   GEA2 *        pgea2;
   WORD *        pEAdata;
   WORD          LongNameLength;

   *ln = 0;
   if ( !LongNameBuf )
      LongNameBuf = strdup( "Initialization of LongNameBuf" );
   debug(F110,"os2getlongname: filename is",fn,0);

   FileInfoSize = sizeof( EAOP2 );
   PathInfoFlags = DSPI_WRTTHRU; /* Complete write operation before return */

   FileInfoBuf.fpGEA2List = (PGEA2LIST) &GEA2List;
   FileInfoBuf.fpFEA2List = (PFEA2LIST) &FEA2List;
   pgea2 = FileInfoBuf.fpGEA2List->list;
   pfea2 = FileInfoBuf.fpFEA2List->list;

   pfea2->oNextEntryOffset = 0;
   pfea2->fEA = 0;
   pfea2->cbName = 9;                   /* Length of ".LONGNAME" without \0 */
   pfea2->cbValue = MAXPATH;
   strcpy( pfea2->szName, ".LONGNAME" );

   FileInfoBuf.fpGEA2List->cbList = sizeof(GEA2LIST)
                                  + pgea2->cbName + 1;
   pgea2->oNextEntryOffset = 0;
   pgea2->cbName = pfea2->cbName;
   strcpy(pgea2->szName,pfea2->szName);

   FileInfoBuf.fpFEA2List->cbList = 1024;

   rc = DosQueryPathInfo(fn,FIL_QUERYEASFROMLIST,&FileInfoBuf,FileInfoSize );
   LongNameLength =
     *(WORD *)((char *)pfea2 + sizeof(FEA2) + pfea2->cbName + sizeof(WORD));
   debug(F101,"os2getlongname: rc=","",rc);
   debug(F101,"   cbValue:","",pfea2->cbValue);
   debug(F101,"   cbName:","",pfea2->cbName);
   debug(F101,"   EA Value Length:","",LongNameLength );
   debug(F110,"   EA Value:",(char *)pfea2 + sizeof(FEA2)
                             + pfea2->cbName + (2 * sizeof(WORD)),0 );
   if ( rc ) {
       return -1;
   } else if ( pfea2->cbValue ) {
       if (LongNameBuf) {
           free(LongNameBuf);
           LongNameBuf = NULL;
       }
       LongNameBuf = (char *) malloc( LongNameLength + 1 );
       if (LongNameBuf) {
           ckstrncpy(LongNameBuf, (char *)pfea2 + sizeof(FEA2)
                   + pfea2->cbName + (2 * sizeof(WORD)),
                   LongNameLength
                   );
           debug(F110,"os2getlongname: longname is",LongNameBuf,0);
       } else
         debug(F100,"os2getlongname: malloc failed","",0);
   } else {
       if ( LongNameBuf )
         free( LongNameBuf );
       LongNameBuf = strdup( "" );
       debug(F110,
             "os2getlongname: there is no longname attribute",
             LongNameBuf,
             0
             );
   }
   *ln = LongNameBuf;
   return 0;
}


_PROTOTYP( VOID GetMem, (PVOID *, ULONG) );

/* O S 2 G E T E A S - Get all OS/2 Extended Attributes */

/* Returns 0 on success, -1 on failure */

int
os2geteas( char * name ) {
    CHAR *pAllocc=NULL; /* Holds the FEA struct returned by DosEnumAttribute */
                        /*  used to create the GEA2LIST for DosQueryPathInfo */

    ULONG ulEntryNum = 1; /* count of current EA to read (1-relative)        */
    ULONG ulEnumCnt;      /* Number of EAs for Enum to return, always 1      */

    ULONG FEAListSize = sizeof(ULONG);/* starting size of buffer necessary to
                                                        hold all FEA blocks */
    ULONG GEAListSize = MAXEACOUNT * sizeof(GEA2) + sizeof(ULONG);

    FEA2 *pFEA;           /* Used to read from Enum's return buffer          */
    GEA2 *pGEA, *pLastGEAIn; /* Used to write to pGEAList buffer             */
    GEA2LIST *pGEAList;/*Ptr used to set up buffer for DosQueryPathInfo call */
    EAOP2  eaopGet;       /* Used to call DosQueryPathInfo                   */
    APIRET rc;
    int offset;

                                /* Allocate enough room for any GEA List */
    GetMem((PPVOID)&pAllocc, MAX_GEA);
    pFEA = (FEA2 *) pAllocc;               /* pFEA always uses pAlloc buffer */

    GetMem((PPVOID)&pGEAList, GEAListSize ); /* alloc buffer for GEA2 list */
    pGEAList->cbList = GEAListSize;
    pGEA = pGEAList->list;
    pLastGEAIn = 0;

    if ( !pAllocc || ! pGEAList ) {
        FreeMem( pAllocc );
        FreeMem( pGEAList );
        return -1;
    }
    if ( pFEAList ) {
        FreeMem( pFEAList );
        pFEAList = 0;
   }
   while(TRUE) {              /* Loop continues until there are no more EAs */
       ulEnumCnt = 1;                   /* Only want to get one EA at a time */
       if(DosEnumAttribute(Ref_ASCIIZ,            /* Read into pAlloc Buffer */
                           name,                  /* Note that this does not */
                           ulEntryNum,            /* get the aValue field,   */
                           pAllocc,               /* so DosQueryPathInfo must*/
                           MAX_GEA,               /* be called to get it.    */
                           &ulEnumCnt,
                           (LONG) GetInfoLevel1)) {
           FreeMem(pGEAList);           /* There was some sort of error */
           FreeMem(pAllocc);
           return (-1);
       }
       if(ulEnumCnt != 1)               /* All the EAs have been read */
         break;

       ulEntryNum++;
       FEAListSize += sizeof(FEA2LIST) + pFEA->cbName+1 +
                      pFEA->cbValue + 4;

       if (pLastGEAIn)
         pLastGEAIn->oNextEntryOffset = (BYTE *)pGEA - (BYTE *)pLastGEAIn;
       pLastGEAIn = pGEA;

       pGEA->oNextEntryOffset = 0L;
       pGEA->cbName = pFEA->cbName;
       strcpy(pGEA->szName, pFEA->szName);

       /* must align GEA2 blocks on double word boundaries */
       offset = sizeof(GEA2) + pGEA->cbName + 1;
       offset += ( offset % 4 ? (4 - offset % 4) : 0 );
       pGEA = (GEA2 *) ((BYTE *) pGEA + offset);
   }
   debug(F111,"os2geteas: EA count",name,ulEntryNum-1);
   GetMem( (PPVOID) &pFEAList, FEAListSize );
   pFEAList->cbList = FEAListSize;

   eaopGet.fpGEA2List = pGEAList;
   eaopGet.fpFEA2List = pFEAList;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel3,
                   (PVOID) &eaopGet,
                    sizeof(EAOP2));

   debug(F111,"os2geteas: DosQueryPathInfo",name,rc);
   FreeMem( pGEAList );
   FreeMem( pAllocc );
   return ( rc ? -1 : 0 );
}

/* O S 2 S E T E A S - Set all OS/2 Extended Attributes */

/* Returns 0 on success, -1 on failure                  */

int
os2seteas( char * name ) {
   EAOP2  eaopSet;       /* Used to call DosSetPathInfo */
   APIRET rc;

   if ( !pFEAList ) {
     debug(F100,"os2seteas: EA List is empty","",0);
     return 0;
   }
   eaopSet.fpGEA2List = 0;
   eaopSet.fpFEA2List = pFEAList;

   rc = DosSetPathInfo(name,            /* Set the EA info */
                    SetInfoLevel2,
                   (PVOID) &eaopSet,
                    sizeof(EAOP2),
                    DSPI_WRTTHRU);
   debug(F111,"os2seteas: DosSetPathInfo",name,rc);

   if ( !rc ) {
      FreeMem( pFEAList );
      pFEAList = 0;
   }
   return ( rc ? -1 : 0 );
}

/* O S 2 G E T A T T R - Get all OS/2 Normal Attributes */

/* Returns 0 on success, -1 on failure                  */

int
os2getattr( char * name ) {
   FILESTATUS3 FileInfoBuf;
   APIRET rc;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                    sizeof(FILESTATUS3));

   if ( !rc ) {
      os2attrs = FileInfoBuf.attrFile;
      return 0;
      }
   else {
      os2attrs = FILE_NORMAL;
      return -1;
      }
}

/* O S 2 S E T A T T R - Set all OS/2 Normal Attributes */

/* Returns 0 on success, -1 on failure                  */

int
os2setattr( char * name ) {
   FILESTATUS3 FileInfoBuf;
   APIRET rc;

   rc = DosQueryPathInfo(name,                   /* Get the complete EA info */
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                    sizeof(FILESTATUS3));

   if ( !rc ) {
      FileInfoBuf.attrFile = lf_opts & os2attrs;
      rc = DosSetPathInfo( name,
                   GetInfoLevel1,
                   (PVOID) &FileInfoBuf,
                   sizeof(FILESTATUS3),
                   0);
      if ( !rc )
         return 0;
      }
   return -1;
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

extern CHAR os2version[50];
extern long vernum;

/* D O _ L A B E L _ S E N D - Generate Label Header from EA List     */

/* Return 0 on success, -1 on failure                                 */

int
do_label_send(char * name) {
    char scratch[100];
    long buffreespace = INBUFSIZE;
    long easleft = 0;
    long memtocpy = 0;
    static BYTE * p = 0;

    debug(F110,"do_label_send",name,0);
    if (!pFEAList) {
        debug(F101,"do_label_send no EA list","",pFEAList);
        return(0);
    }
    if (!p) {
        debug(F100,"do_label_send: generate header","",0);
        zinptr += sprintf(zinptr,"KERMIT LABELED FILE:02UO04VERS");

        sprintf(scratch,"%d",strlen(get_os2_vers()));
        zinptr += sprintf(zinptr,"%02d%d%s",
                          strlen(scratch),
                          strlen(get_os2_vers()),
                          get_os2_vers()
                          );
        sprintf(scratch,"%d",vernum);
        zinptr += sprintf(zinptr,"05KVERS02%02d%d", strlen(scratch), vernum);
        sprintf(scratch,"%d",strlen(name));
        zinptr += sprintf(zinptr,"08FILENAME%02d%d%s",
                          strlen(scratch),
                          strlen(name), name
                          );
        zinptr += sprintf(zinptr,"04ATTR%02d",sizeof(ULONG));
        memcpy(zinptr, (BYTE *) &os2attrs, sizeof(ULONG));
        zinptr += sizeof(ULONG);

        sprintf( scratch, "%d", pFEAList->cbList );
        zinptr += sprintf(zinptr,"09EABUFSIZE%02d%ld",
                          strlen(scratch),
                          pFEAList->cbList
                          );
        p = (BYTE *) pFEAList;
        buffreespace -= (BYTE *) zinptr - (BYTE *) zinbuffer;
    }
    easleft = pFEAList->cbList - ( (BYTE *) p - (BYTE *) pFEAList );
    memtocpy = buffreespace > easleft ? easleft : buffreespace;
    memcpy( zinptr, p, memtocpy );
    zinptr = (BYTE *) zinptr + memtocpy;
    p = (BYTE *) p + memtocpy;
    buffreespace -= memtocpy;

    if (buffreespace > 0) {
        p = 0;
        FreeMem(pFEAList);
        pFEAList = 0;
        debug(F100,"do_label_send: terminate header","",0);
    }
    zincnt = (zinptr - zinbuffer);              /* Size of this beast */
    return(0);
}

/* D O _ L A B E L _ R E C V - Receive label info and create EA List  */

/* Return 0 on success, -1 on failure */

int
do_label_recv() {
    char *recv_ptr = zoutbuffer;
    int lblen;
    char buffer[16];
    size_t memtocpy, lefttocpy;
    static BYTE * pFEA = 0;

    if ( !pFEAList
         && strncmp(zoutbuffer,"KERMIT LABELED FILE:02UO04VERS",30) != 0) {
        debug(F100,"do_label_recv: not a labeled file","",0);
        return(0);                      /* Just continue if unlabeled */
    }

    if ( !pFEAList ) {
       recv_ptr += 30;                  /* start at front of buffer */
       zoutcnt  -= 30;

       /* get length of length of OS/2 version */
       memcpy(buffer, recv_ptr, 2);
       recv_ptr += 2;
       zoutcnt  -= 2;
       buffer[2] = '\0';
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
       debug(F111,"do_label_recv file created under OS/2",buffer,lblen);

       /* check sync with Kermit Version */
       memcpy(buffer, recv_ptr, 7);
       recv_ptr += 7;
       zoutcnt  -= 7;
       if (strncmp(buffer, "05KVERS", 7) != 0) {
           debug(F111,"do_label_recv lost sync at KVERS",recv_ptr-7,zoutcnt+7);
           return(-1);
       }

       /* get length of length of C-Kermit version */
       memcpy(buffer, recv_ptr, 2);
       recv_ptr += 2;
       zoutcnt  -= 2;
       buffer[2] = '\0';
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
       debug(F111,"do_label_recv file created with OS/2 C-Kermit",
             buffer,lblen);

       /* check sync with FILENAME */
       memcpy(buffer, recv_ptr, 10);
       recv_ptr += 10;
       zoutcnt  -= 10;
       if (strncmp(buffer, "08FILENAME", 10) != 0) {
           debug(F111,"do_label_recv lost sync at FILENAME",
                 recv_ptr-10,zoutcnt+10);
           return(-1);
       }

       /* get length of length of Filename */
       memcpy(buffer, recv_ptr, 2);
       recv_ptr += 2;
       zoutcnt  -= 2;
       buffer[2] = '\0';
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
       memcpy(buffer, recv_ptr, 2);
       recv_ptr += 2;
       zoutcnt  -= 2;
       buffer[2] = '\0';
       lblen = atoi(buffer);
       if ( lblen != sizeof(ULONG) ) {
           debug(F101,"   Attributes have wrong length","",lblen);
           return(-1);
       }

       /* get attributes */
       memcpy(&os2attrs, recv_ptr, sizeof(ULONG));
       recv_ptr += sizeof(ULONG);
       zoutcnt  -= sizeof(ULONG);

       /* check sync with EABUFSIZE */
       memcpy(buffer, recv_ptr, 11);
       recv_ptr += 11;
       zoutcnt  -= 11;
       if (strncmp(buffer, "09EABUFSIZE", 11) != 0) {
           debug(F111,"  lost sync at EABUFSIZE",recv_ptr-11,zoutcnt+11);
           return(-1);
       }

       /* get length of length of EA Buffer Size */
       memcpy(buffer, recv_ptr, 2);
       recv_ptr += 2;
       zoutcnt  -= 2;
       buffer[2] = '\0';
       lblen = atoi(buffer);

       /* get length of EA Buffer Size */
       memcpy(buffer, recv_ptr, lblen);
       recv_ptr += lblen;
       zoutcnt  -= lblen;
       buffer[lblen] = '\0';
       lblen = atoi(buffer);
       debug(F101,"  EA Buffer Size:","",lblen);

       GetMem( (PPVOID) &pFEAList, (ULONG) lblen );
       if ( !pFEAList ) {
         debug(F101,"   pFEAList","",pFEAList);
         return -1;
       }
       pFEAList->cbList = lblen;
       pFEA = (BYTE *) pFEAList;
   }

    if ( pFEAList && pFEA ) {
       /* get EA Buffer */
       lefttocpy = pFEAList->cbList - ( (BYTE *) pFEA - (BYTE *) pFEAList );
       memtocpy = ( zoutcnt < lefttocpy ? zoutcnt : lefttocpy );
       memcpy(pFEA, recv_ptr, memtocpy);
       recv_ptr += memtocpy;
       zoutcnt  -= memtocpy;
       pFEA += memtocpy;
       debug(F101,"   memtocpy","",memtocpy);
       debug(F101,"   zoutcnt","",zoutcnt);

       if ( pFEA == ( (BYTE *) pFEAList + pFEAList->cbList ) ) {
          pFEA = 0;  /* we are done copying the EA's to the EA List */
          debug(F100,"   done copying EA's","",0);
       }
    }

    /* if we have processed some of the data in the output buffer */
    /* then move the data in the buffer so that it is properly    */
    /* aligned with the beginning of the buffer and reset the ptr */
    if ( recv_ptr != zoutbuffer ) {
        memmove(zoutbuffer, recv_ptr, zoutcnt);
        if ( pFEA ) {
            zoutptr = zoutbuffer + zoutcnt ;
            return(1);                          /* Go fill some more */
        }
    }
    return (0);
}
#endif /* CK_LABELED */

#ifdef OS2ONLY
/*
 * given a template of the form "fnamXXXXXX", insert number on end
 * of template, insert unique letter if needed until unique filename
 * found or run out of letters.  The number is generated from the
 * Process ID.
 */

char *
mktemp(char * template)
{
    char *string = template;
    unsigned number;
    int letter = 'a';
    int xcount = 0;
    int olderrno;

    if (template == NULL || template[0] == '\0')
        return(NULL);

    number = os2getpid();

    while (*string)
        string++;

    /* replace last five X's */
    while (*--string == 'X' && xcount < 5)
    {
        xcount++;
        *string = (char)((number % 10) + '0');
        number /= 10;
    }

    /* too few X's ? */
    if (*string != 'X' || xcount < 5)
        return(NULL);

    /* set first X */
    *string = letter++;

    olderrno = errno;       /* save current errno */
    errno = 0;              /* make sure errno isn't EACCESS */

    /* check all the files 'a'-'z' */
    while ((access(template,0) == 0) || (errno == EACCES))
        /* while file exists */
    {
        errno = 0;
        if (letter == 'z' + 1) {
            errno = olderrno;
            return(NULL);
        }

        *string = (char)letter++;
    }

    errno = olderrno;
    return(template);
}
#endif /* OS2ONLY */
