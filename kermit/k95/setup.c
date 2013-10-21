#define FOR_REAL /* For testing, don't define this. */

/*  Kermit 95: Text-mode setup program  */

/* For version 1.1.21 */

/*
  Copyright (C) 1997, 2000 Trustees of Columbia University in the City of
  New York.  All rights reserved.
*/
#define SPACE_NEEDED 6000000L		/* Free disk bytes needed to install */

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
/* all functions in this module return TRUE to indicate success */
/* or FALSE to indicate failure */
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckucmd.h"
#include "ckuusr.h"
#include "ckowin.h"
#include "ckntap.h"             /* Kermit Telephony */

#ifdef isWin95
#undef isWin95
#endif
#ifdef printf
#undef printf
#endif
#ifdef fprintf
#undef fprintf
#endif

#ifdef CLINK
/* These were only needed for CreateLink, which doesn't work. */
#include <shellapi.h>
#include <shlguid.h>
#include <shlobj.h>
#endif /* COMMENT */

#else /* not NT */

#define INCL_WIN
#define	INCL_WINSWITCHLIST
#define	INCL_ERRORS
#define	INCL_KBD
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */
#define	INCL_VIO
#define	INCL_DOSMISC
#define	INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define	INCL_DOSQUEUES
#define INCL_DOSEXCEPTIONS
#define	INCL_DOSSIGNALS
#define	INCL_DOSDEVICES
#define	INCL_DOSDEVIOCTL
#define	INCL_DOSNLS
#define INCL_DOSASYNCTIMER
#define INCL_DOSDATETIME
#define INCL_DOSFILEMGR
#define INCL_WINWORKPLACE
#ifdef OS2PM
#define  INCL_DOSNMPIPES
#endif /* OS2PM */
#include <os2.h>
#undef COMMENT
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#define S_IFMT 0xF000
#endif /* NT */
#endif /* OS2 */

#ifndef MAXPATH
#define MAXPATH 256
#endif /* MAXPATH */

#ifdef NT
#define K95EXE  "K95.EXE"
#define ZIPFILE "K95PACK.EXE"
#define ZIPFILE1 "K95PACK1.EXE"
#define ZIPFILE2 "K95PACK2.EXE"
#define ZIPFILE3 "K95PACK3.EXE"
#define ZIPFILE4 "K95PACK4.EXE"
#define DISKLABEL "K95 DISK 1"
#define DISK2LABEL "K95 DISK 2"
#define DISK3LABEL "K95 DISK 3"
#define DISK4LABEL "K95 DISK 4"
#define PRODUCT   "Kermit 95"
#define ABBR      "K95"
#else
#define K95EXE  "K2.EXE"
#define ZIPFILE "K2PACK.EXE"
#define ZIPFILE1 "K2PACK1.EXE"
#define ZIPFILE2 "K2PACK2.EXE"
#define ZIPFILE3 "K2PACK3.EXE"
#define ZIPFILE4 "K2PACK4.EXE"
#define DISKLABEL "K2 DISK 1"
#define DISK2LABEL "K2 DISK 2"
#define DISK3LABEL "K2 DISK 3"
#define DISK4LABEL "K2 DISK 4"
#define PRODUCT   "Kermit/2"
#define ABBR      "K2"
#endif 

unsigned char bar[81];

#ifdef NT
/* extern */
extern struct keytab * tapilinetab, * _tapilinetab;
extern int ntapiline;
#endif /* NT */

/* Global Declarations */

struct _kermitinfo {			/* Struct for returning our  */
    char install_dir[260];		/* info to other programs... */
    int dir_exists;			/* Target directory already exists */
    char modem_name[80];		/* Null string if none */
    int com_port;			/* 0 if none */
    long port_speed;			/* 0 if none */
    int c_code;				/* Country code */
    char a_code[256];
    char l_prefix[256];
    char i_prefix[256];
    char t_prefix[16];
    char t_area[16];
    char t_lc_area_codes[256];
} kermitinfo;


struct mdm {				/* Modem info */
    char * menuname;			/* Name to print in menu */
    char * kermitname;			/* Kermit SET MODEM TYPE name */
    long speed;				/* Default speed */
} mdmtab[] = {
    "3COM US Robotics/Megahertz 56K", "3com-usr-megahertz-56k", 115200L,
    "Atlas Newcom 33,600ifxC", "atlas-newcom-33600ifxC", 57600L,
    "AT&T 1900 STU III",    "att-1900-stu-iii",     57600L,
    "AT&T 1910 STU III",    "att-1910-stu-iii",     57600L,
    "AT&T 7300",            "att-7300",         57600L,
    "AT&T Dataport",        "att-dataport",     57600L,
    "AT&T DTDM",            "att-dtdm",         57600L,
    "AT&T ISN",             "att-isn",          57600L,
    "AT&T KeepInTouch",     "att-keepintouch",  57600L,
    "AT&T Switched Net",    "att-switched-net", 57600L,
    "Best Data",            "bestdata",         57600L,
    "Boca",                 "boca",             57600L,
    "Cardinal",		    "cardinal",         57600L,
    "Compaq Data+Fax (Presario)",
                            "compaq",           57600L,
    "Digitel DT22",         "digitel-dt22",      2400L,
    "Fujitsu",              "fujitsu",          57600L,
    "Gateway Telepath",     "gateway-telepath", 57600L,
    "Generic High Speed",   "generic-high-speed", 57600L,
    "Hayes 1200",           "hayes-1200",        1200L,
    "Hayes 2400",           "hayes-2400",        2400L,
    "Hayes Ultra, Optima, or Accura",
                            "hayes-high-speed", 57600L,
    "IBM Mwave",            "mwave",            57600L,
    "Intel 14400 Faxmodem", "intel",            57600L,
    "ITU-T (CCITT) V25bis", "itu-t-v25bis",             2400L,
    "ITU-T (CCITT) V25ter/v250", "itu-t-v25ter/v250", 2400L,
    "Maxtech",              "maxtech",          57600L,
    "Megahertz AT&T V.34",  "megahertz-att-v34",57600L,
    "Megahertz XJack 33.6", "megahertz-xjack-33.6",  57600L,
    "Megahertz XJack 56k",  "megahertz-xjack-56k",  115200L,
    "Microcom in AT mode",  "microcom-at",      57600L,
    "Microcom in SX mode",  "microcom-sx",      57600L,
    "Microlink",            "microlink",        57600L,
    "Microlink V250",       "microlink-v250",        57600L,
    "Motorola Codex",       "motorola-codex",   57600L,
    "Motorola Fastalk",     "motorola-fastalk", 57600L,
    "Motorola Lifestyle",   "motorola-lifestyle", 57600L,
    "Motorola Montana",     "motorola-montana", 57600L,
    "Multitech",            "multitech",        57600L,
    "Practical Peripherals","ppi",              57600L,
    "QuickComm Spirit II",  "spirit-ii",        57600L,
    "Rockwell V32",         "rockwell-v32",     57600L,
    "Rockwell V32bis",      "rockwell-v32bis",  57600L,
    "Rockwell V34",         "rockwell-v34",     57600L,
    "Rockwell V90",         "rockwell-v90",    115200L,
    "Rolm 244pc",           "rolm-244pc",       19200L,
    "Rolm 600 series",      "rolm-600-series",  19200L,
    "Rolm DCM",             "rolm-dcm",         19200L,
    "Supra Fax Modem",      "suprafaxmodem",    57600L,
    "SupraSonic 288+",      "suprasonic",       57600L,
    "Telebit (old models)", "old-telebit",      19200L,
    "Telebit (newer models)","telebit",         38400L,
    "Unknown",              "unknown",           2400L,
    "US Robotics/Megahertz 56k", "usr-megahertz-56k", 115200L,
    "US Robotics Sportster or Courier",
                            "usrobotics",       57600L,
    "Zoltrix",              "zoltrix",          57600L,
    "Zoom",                 "zoom",             57600L,
    "Zyxel",                "zyxel",            57600L
};
int modems = (sizeof(mdmtab) / sizeof(struct mdm));

long speedtab[] = {			/* Legal speeds */
    110L,
    300L,
    1200L,
    2400L,
    3600L,
    7200L,
    9600L,
    14400L,
    19200L,
    28800L,
    38400L,
    57600L,
    76800L,
    115200L,
    230400L,
    460800L
};
int speeds = (sizeof(speedtab) / sizeof(long));

char * printers[] = {
    "PRN", "LPT1", "LPT2", "LPT3", "NUL", "KERMIT.PRN"
};
int nprinters = (sizeof(printers)/sizeof(char *));

FILE *fp;
#ifdef COMMENT
FILE *fopen();
#endif 

char buf[256];


int
isWin95( void ) {
#ifdef NT
    int OSVer = 0;
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    OSVer = osverinfo.dwPlatformId ;
    return(OSVer==VER_PLATFORM_WIN32_WINDOWS);
#else
    return 0;
#endif
}

#ifdef OS2
#define DIRSEP       '/'
/* #define DIRSEP       '\\' */
#define ISDIRSEP(c)  ((c)=='/'||(c)=='\\')
#else /* not OS2 */
#define DIRSEP       '/'
#define ISDIRSEP(c)  ((c)=='/')
#endif /* OS2 */

/*
   i s d i r  --
   Tells if string pointer s is the name of an existing directory.
   Returns 1 if directory, 0 if not a directory.
*/
#include <sys/stat.h>

#ifdef OS2
#include <sys/types.h>
#ifdef NT
int os2stat(char *, struct _stat *);
#else /* NT */
int os2stat(char *, struct stat *);
#endif /* NT */
#endif /* OS2 */

#ifdef OS2
os2stat(char * path,
#ifdef NT
	struct _stat *st
#else
	struct stat *st
#endif /* NT */
	) {
    char local[MAXPATH + 1];
    int len;

    if (!path) path = "";
    strcpy(local, path);
    len = strlen(local);

    if ( len == 2 && local[1] == ':' )
      local[2] = DIRSEP, local[3] = 0;	/* If drive only, append / */
    else if ( len == 0 )
      local[0] = DIRSEP, local[1] = 0;	/* If empty path, take / instead */
    else if ( len > 1 && ISDIRSEP(local[len - 1]) && local[len - 2] != ':' )
      local[len - 1] = 0; /* strip trailing / except after d: */

#ifdef NT
    return _stat(local, st);
#else /* NT */
    return stat(local, st);
#endif /* NT */
}
#define stat(x,y) os2stat(x,y)
#endif /* OS2 */

int
isdir(char * s) {
    int x;

#ifdef NT
    struct _stat statbuf;
#else
    struct stat statbuf;
#endif /* NT */

    if (!s) return(0);
    if (!*s) return(0);

#ifdef OS2
    /* Disk letter like A: is top-level directory on a disk */
    if (((int)strlen(s) == 2) && (isalpha(*s)) && (*(s+1) == ':'))
      return(1);
#endif /* OS2 */
    x = stat(s,&statbuf);
    if (x == -1) {
	return(0);
    } else {
	return((((statbuf.st_mode) & _S_IFMT) == _S_IFDIR) ? 1 : 0 );
    }
}

#ifdef OS2
unsigned long
zdskspace(int diskletter) {
#ifdef NT
    DWORD spc, bps, fc, c ;
    char rootpath[4] ;
    int drive;
    drive = diskletter - 'A' + 1;
	 
    if (drive) {
	rootpath[0] = '@' + drive ;
	rootpath[1] = ':' ;
	rootpath[2] = '\\' ;
	rootpath[3] = '\0' ;
	if ( !GetDiskFreeSpace( rootpath, &spc, &bps, &fc, &c ))
	  return 0 ;
    } else {
	if ( !GetDiskFreeSpace( NULL, &spc, &bps, &fc, &c ))
	  return 0 ;
    }
    return spc * bps * fc ;
#else /* NT */
    int drive = diskletter - 'A' + 1;
    FSALLOCATE fsa;
    if ( DosQueryFSInfo(drive, 1, (PBYTE) &fsa, sizeof(fsa)) )
      return 0;
    return fsa.cUnitAvail * fsa.cSectorUnit * fsa.cbSector;
#endif /* NT */
}
#endif /* OS2 */

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

#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif /* S_ISREG */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif /* S_ISDIR */

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
#ifdef NT
    struct _stat buf;
#else
    struct stat buf;
#endif /* NT */
    int x;

#ifdef UNIX
    x = strlen(name);
    if (x == 9 && !strcmp(name,"/dev/null"))
      return(0);
#endif /* UNIX */

    x = stat(name,&buf);
    if (x < 0) {
       return(-1);
    }
    if (!S_ISREG (buf.st_mode)		/* Must be regular file */
#ifdef S_ISFIFO
	&& !S_ISFIFO (buf.st_mode)	/* or FIFO */
#endif /* S_ISFIFO */
	) {				
       return(-2);
    }

#ifdef SW_ACC_ID
    priv_on();
#endif /* SW_ACC_ID */
#ifdef NT
    x = _access(name,R_OK);
#else
    x = access(name,R_OK);
#endif /* NT */
#ifdef SW_ACC_ID
    priv_off();
#endif /* SW_ACC_ID */
    if (x < 0) { 	/* Is the file accessible? */
    	return(-3);
    } else {
       return( (buf.st_size > -1L) ? buf.st_size : 0L );
    }
}

/*  Z C H D S K  --  Change currently selected disk device */

/* Returns -1 if error, otherwise 0 */

zchdsk(c) int c; {
    int i = toupper(c) - 64;
    return( _chdrive(i));
}

/*  Z H O M E  --  Return pointer to user's home directory  */

char *
zhome() {
    char *home = getenv("HOME");
    return(home ? home : ".");
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

    if (dirnam == NULL || dirnam == "" || *dirnam == '\0') /* If arg is null */
      dirnam = zhome();			/* use user's home directory. */
    sp = dirnam;

#ifdef DTILDE
    hd = tilde_expand(dirnam);		/* Attempt to expand tilde */
    if (*hd == '\0') hd = dirnam;	/* in directory name. */
#else
    hd = dirnam;
#endif /* DTILDE */
#ifdef OS2
    if (isalpha(hd[0]) && hd[1] == ':') {
        if (zchdsk(hd[0]))
	  return(0);
	if (hd[2] == 0)
	  return(1);			/* Handle drive-only case */
	hd += 2 ;			/* Move pointer forward since we already
	                                   handled the drive letter */
    }
    {
    /* strip trailing DIRSEP except after d:; chdir() doesn't like it */
    int len = strlen(hd) ;
    if ( len > 1 && ISDIRSEP(hd[len - 1]) && hd[len - 2] != ':' )
        hd[len - 1] = 0; 
    }
#endif /* OS2 */
    if (
#ifdef NT
	_chdir(hd)
#else
	chdir(hd)
#endif /* NT */
	== 0) return(1);	/* Try to cd */ /* (===OS2===) */
    p = sp;				/* Failed, lowercase it. */
    while (*p) {
	if (isupper(*p)) *p = tolower(*p);
	p++;
    }
#ifdef DTILDE
    hd = tilde_expand(sp);		/* Try again to expand tilde */
    if (*hd == '\0') hd = sp;
#else
    hd = sp;				/* Point to result */
#endif /* DTILDE */
    return((
#ifdef NT
	    _chdir(hd)
#else
	    chdir(hd)
#endif /* NT */
	    == 0) ? 1 : 0);
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

#define CWDBL 256
static char cwdbuf[CWDBL+1];

char *
zgtdir() {
    char *buf;

#ifdef OS2
#ifndef __IBMC__ /* which has a macro for this */
    extern char *getcwd();
#endif /* __IBMC__ */
    buf = cwdbuf;
#ifdef NT
    return(_getcwd(buf,CWDBL));
#else
    return(getcwd(buf,CWDBL));
#endif /* NT */
#endif /* OS2 */
}

int
isdiskok(char c) {
    char buf[3] = "C:";
    int x; unsigned long z;
    buf[0] = c;
    if (!isdir(buf))			/* (this never seems to fail...) */
      return(0);
    z = zdskspace(c);
    return((z > SPACE_NEEDED) ? 1 : 0);
}

char *
mypwd(void) {
#ifdef OS2
#ifdef NT    
    if (_getcwd(buf,256))
      return((char *)buf);
    else
      return("");
#else
    extern char * getcwd();
    return(getcwd(buf,256));
#endif /* NT */
#else
    return("");
#endif /* OS2 */
}

char *
getvolid(char c) {
#ifdef NT
/*
  This does not seem to work when given a disk volume root path as the
  first argument.  Therefore the argument to getvolid() is ignored and it
  always goes to the current disk.
*/
    char disk[5];
    char tmp[20];
    DWORD volser = 0;
    DWORD maxlength = MAXPATH;
    DWORD flags = 0;
    disk[0] = c;
    disk[1] = ':';
    disk[2] = '\\';
    disk[3] = (char) 0;
    if (!GetVolumeInformation( /* (char *) disk, */ NULL,
			      (char *) buf,
			      (DWORD) 256,
			      &volser,
			      &maxlength,
			      &flags,
			      (char *) tmp,
			      (DWORD) 20
			      )) {

	return("");
    } else {
	return((char *)buf);
    }
#else
    return("");	
#endif /* NT */
}

#ifdef NT
HANDLE ConsoleHandle = 0;
DWORD  ConsoleMode   = 0;
int  KnowConsoleMode = 0;
#endif /* NT */

int
mysystem(char * cmd) {
    int rc = 0;
#ifdef FOR_REAL
    rc = system(cmd) == 0;
#ifdef NT
    if (KnowConsoleMode)		/* Clean up after system()... */
      SetConsoleMode(ConsoleHandle, ConsoleMode);
#endif
#else
    rc = 1;
    printf("%s\n", cmd);
#endif /* FOR_REAL */
    return(rc);
}

int
mymkdir(char * dir) {
    int rc = 0;;
#ifdef FOR_REAL
#ifdef OS2
    rc = _mkdir(dir);
#else
    rc = mkdir(dir,0777);
#endif /* OS2 */
#else
    printf("mkdir %s\n", dir);
#endif /* FOR_REAL */
    return((rc < 0) ? 0 : 1);
}

int
inchar(char * prompt, char defaultval) {
    int x=0, val, toomany;
    toomany = 0;
    val = 0;

    if (defaultval)
      printf("%s [%c]: ", prompt, defaultval);
    else
      printf("%s: ", prompt);
    while (val == 0) {
	while (1) {
	    if ((x = getchar()) == EOF)	/* Error */
	      return(-1);
	    if (x == 10 || x == 13) {	/* Enter key */
#ifdef NODEFAULT
		printf("%s", prompt);	/* Print the prompt again */
		continue;
#else
		return(defaultval);
#endif /* NODEFAULT */
	    } else if (x > 32) {	/* Printable character */
		break;
	    }				/* Otherwise keep looping */
	}
	val = x;			/* The character we got */
	while (1) {
	    if ((x = getchar()) == EOF)	/* Get terminating Enter key */
	      return(-1);
	    if (x == 32)		/* Trailing spaces */
	      continue;
	    else if (x == 10 || x == 13) /* Enter key */
	      return(toomany ? -1 : val);
	    else if (x > 32) {		/* Extraneous character */
		if (!toomany)
		  printf("Just one character please.\n");
		toomany = 1;
		continue;
	    }
	}
    }
    return(val);
}

int
getnum(char * prompt, int defaultval) {
    int x=0, val = 0;

    printf("%s [%d]: ", prompt, defaultval);
    while (1) {
	if ((x = getchar()) == EOF)	/* Error */
	  return(-1);
	if (x == 10 || x == 13) {	/* Enter key */
#ifdef NODEFAULT
	    printf("%s", prompt);	/* Print the prompt again */
	    continue;
#else
	    return(defaultval);
#endif /* NODEFAULT */
	} else if (x < 33) {		/* Space or control character */
	    continue;
	} else if (x >= (char)'0' && x <= (char)'9') { /* Got a digit */
	    break;
	} else {			/* Not a digit */
	    printf("%c - Not a digit\n", (char) x);
	    continue;
	}
    }
    val = x - '0';			/* The character we got */
    while (1) {
	if ((x = getchar()) == EOF)	/* Error */
	  return(-1);
	if (x == 10 || x == 13) {	/* Enter key */
	    return(val);
	} else if (x < 32) {
	    continue;
	} else if (x >= (char)'0' && x <= (char)'9') {
	    val = 10 * val + x - '0';
	    continue;
	} else {
	    printf("%c - Not a digit\n", (char) x);
	    continue;
	}
    }
}

void
upper(char * c) {			/* Uppercase a character */
    if (*c >= 'a' && *c <= 'z')
      *c -= 'a' - 'A';
}

int
getok(char * prompt, char defaultval) {	/* Get Y or N answer to question */
    int x, rc = -1, flag = 0;
    char c;

    if (!prompt)
      prompt = "Is that OK (y/n)";
    do {
	if ((x = inchar(prompt, defaultval)) < 0) {
	    continue;
	}	    
	c = (char) x;			/* Convert to char */
	upper (&c);			/* Uppercase it */
	if (c == 'Y')			/* Yes */
	  rc = 1;
	else if (c == 'N')		/* No */
	  rc = 0;
	else
	  printf("Please type Y for Yes or N for No.\n");
    } while (rc < 0);
    return(rc);
}

void
initbar() {
    int i;
    for (i = 0; i < 79; i++)
      bar[i] =
#ifdef OS2
    (char) /* 196 */ 205	/* 196 = single, 205 = double */
#else
    '_'
#endif /* OS2 */
    ;
    bar[79] = (char) 0;
}

void
drawbar() {
#ifdef OS2
    printf("\n");
#endif /* OS2 */
    printf("%s\n\n", bar);
}

int
getdialinfo() {				/* This can change */
    return(0);				/* when we have TAPI going... */
}

#ifdef CLINK
/* Doesn't work in console app... */

#define SHORTCUTDEBUG

HRESULT
CreateLink(LPCSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc) {
    HRESULT hres;
    IShellLink * psl;

#ifdef SHORTCUTDEBUG
    DWORD ec = 0L;			/* Error code */

    printf("CreateLink entry...\n");
    printf("PathObj=[%s]\n", lpszPathObj);
    printf("PathLink=[%s]\n", lpszPathLink);
    printf("Description=[%s]\n", lpszDesc);
#endif /* SHORTCUTDEBUG */

    hres = CoCreateInstance(&CLSID_ShellLink,
			    NULL,
			    CLSCTX_INPROC_SERVER,
			    &IID_IShellLink,
			    &psl
			    );
#ifdef SHORTCUTDEBUG
    if (hres == S_OK)
      printf("CoCreateInstance returns S_OK\n");
#endif /* SHORTCUTDEBUG */

    if (SUCCEEDED(hres)) {
	IPersistFile * ppf;
	psl->lpVtbl->SetPath(psl, lpszPathObj);
	psl->lpVtbl->SetDescription(psl, lpszDesc);
	hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
	if (SUCCEEDED(hres)) {
	    WORD wsz[MAX_PATH];
	    MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);
	    hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
#ifdef SHORTCUTDEBUG
	    if (SUCCEEDED(hres))
	      printf("Save succeeded\n");
	    else {
		ec = GetLastError();
		printf("Save failed: %ld\n", ec);
	    }
#endif /* SHORTCUTDEBUG */
	    ppf->lpVtbl->Release(ppf);
	}
#ifdef SHORTCUTDEBUG
	else {
	    ec = GetLastError();
	    printf("QueryInterface failed: %ld\n", ec);
	}
#endif /* SHORTCUTDEBUG */
	psl->lpVtbl->Release(psl);
    }
#ifdef SHORTCUTDEBUG
    else {
	ec = GetLastError();
	printf("CoCreateInstance failed: %ld\n", ec);
	if (hres == REGDB_E_CLASSNOTREG)
	  printf("Class not registered\n");
	else if (hres == E_OUTOFMEMORY)
	  printf("Out of memory\n");
	else if (hres == E_INVALIDARG)
	  printf("Invalid argument\n");
	else if (hres == E_UNEXPECTED)
	  printf("Unexpected error\n");
	else if (hres == CLASS_E_NOAGGREGATION)
	  printf("This class cannot be created as part of an aggregate\n");
    }
#endif /* SHORTCUTDEBUG */
    return(hres);
}
#endif /* COMMENT */

void
doexit(int rc, int dummy) {
    int x;
    inchar("Press the Enter key to exit SETUP.", 0);
    exit(rc);
}

#ifdef NT
int
Win32EnumPrt( struct keytab ** pTable, int * pN )
{
    DWORD  dwBytesNeeded;
    DWORD  dwPrtRet2;
    DWORD  dwMaxPrt;
    LPTSTR lpName = NULL;
    DWORD  dwEnumFlags = PRINTER_ENUM_LOCAL;
    LPPRINTER_INFO_2 pPrtInfo2=NULL;
    int i, n ;

    if ( *pTable )
    {
	for ( i=0 ; i < *pN ; i++ )
	    free( (*pTable)[i].kwd ) ;
	free ( *pTable )  ;
    }
    *pTable = NULL;
    *pN = 0;

    if ( !isWin95() ) {
        dwEnumFlags |= PRINTER_ENUM_CONNECTIONS;
    }

    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //

    EnumPrinters (dwEnumFlags, lpName, 2, NULL, 0, &dwBytesNeeded,
                   &dwPrtRet2);
    
    //
    // (simple error checking, if these work assume rest will too)
    //

    if (!(pPrtInfo2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwBytesNeeded)))
    {
        return(FALSE);
    }

    if (!EnumPrinters (dwEnumFlags, lpName, 2, (LPBYTE) pPrtInfo2,
                        dwBytesNeeded, &dwBytesNeeded, &dwPrtRet2))
    {
        LocalFree( pPrtInfo2 );
        return(FALSE);
    }

    /* we now have an enumeration of all printer names */

    (*pTable) = (struct keytab *) malloc( dwPrtRet2 * sizeof(struct keytab) );
    if ( !(*pTable) )
    {
        LocalFree( pPrtInfo2 );
        return(FALSE);
    }

    for ( i=0 ; i<dwPrtRet2 ; i++ ) {
        (*pTable)[i].kwd = strdup( pPrtInfo2[i].pPrinterName );
        (*pTable)[i].kwval = i;
        (*pTable)[i].flgs = 0;
    }
    *pN = dwPrtRet2 ;

    LocalFree( pPrtInfo2 );
    return(TRUE);
}
struct keytab * w32prttab = NULL;
int             nw32prt = 0;
#endif /* NT */



#ifndef NT
void
os2Folder(char * diskdir, int new_install, int windowable) {
    ICONINFO iconinfo ;
    UCHAR    exefile[256] ;
    UCHAR    iconfile[256] ;
    UCHAR    title[256];
    UCHAR    classname[256] ;
    UCHAR    location[256] ;
    UCHAR    setup[256] ;
    APIRET   rc ;

    printf("Assigning the K95F_OS2 icon to %s.EXE\n",ABBR);
    sprintf(exefile,"%s\\%s.EXE",diskdir,ABBR);
    sprintf(iconfile,"%s\\ICONS\\K95F_OS2.ICO",diskdir);
    iconinfo.cb = sizeof(ICONINFO);
    iconinfo.fFormat = ICON_FILE ;
    iconinfo.pszFileName = iconfile;
    iconinfo.hmod = NULL ;
    iconinfo.resid = 0 ;
    iconinfo.cbIconData = 0 ;
    iconinfo.pIconData = NULL ;
    if ( !WinSetFileIcon( exefile, &iconinfo ) ) {
	printf(" ERROR: Unable to assign icon to %s.EXE\n",
		ABBR);
    }

    printf("Assigning the K95F_OS2 icon to %sDIAL.EXE\n",ABBR);
    sprintf(exefile,"%s\\%sDIAL.EXE",diskdir,ABBR);
    sprintf(iconfile,"%s\\ICONS\\K95F_OS2.ICO",diskdir);
    iconinfo.cb = sizeof(ICONINFO);
    iconinfo.fFormat = ICON_FILE ;
    iconinfo.pszFileName = iconfile;
    iconinfo.hmod = NULL ;
    iconinfo.resid = 0 ;
    iconinfo.cbIconData = 0 ;
    iconinfo.pIconData = NULL ;
    if ( !WinSetFileIcon( exefile, &iconinfo ) ) {
	printf(" ERROR: Unable to assign icon to %sDIAL.EXE\n",ABBR);
    }

    printf("Building %s Folder\n",PRODUCT);
    sprintf(title,"%s 1.1",PRODUCT);
    sprintf(classname,"WPFolder");
    sprintf(location,"<WP_DESKTOP>");
    sprintf(setup,"OBJECTID=<%sFLDR>",ABBR);
    if ( !WinCreateObject( classname, title, setup, location, CO_UPDATEIFEXISTS) ) {
	printf(" ERROR: Unable to create Folder\n");
    }

    printf("Building Kermit 95 Hyperlink Manual object\n",PRODUCT);
    sprintf(title,"Kermit 95 Hyperlink Manual");
    sprintf(classname,"WPProgram");
    sprintf(location,"<%sFLDR>",ABBR);
    sprintf(setup,
            "EXENAME=NETSCAPE.EXE;STARTUPDIR=%s\\docs\\manual;PARAMETERS=%s\\docs\\manual\\kermit95.htm",
	     diskdir,diskdir);
    if ( !WinCreateObject( classname, title, setup, location, CO_UPDATEIFEXISTS) ) {
	printf(" ERROR: Unable to create Program Object\n");
    }

    printf("Building Kermit/2 Program Object\n");
    sprintf(title,"Kermit/2");
    sprintf(classname,"WPProgram");
    sprintf(location,"<%sFLDR>",ABBR);
    if (windowable) 
	sprintf(setup,"EXENAME=%s\\%s.EXE;STARTUPDIR=%s;PARAMETERS=%%;PROGTYPE=WINDOWABLEVIO;ICONFILE=%s\\ICONS\\%s.ICO;MAXIMIZE=YES;CCVIEW=YES",
		 diskdir,ABBR,diskdir,diskdir,"K95F_OS2");
    else /* fullscreen */
	sprintf(setup,"EXENAME=%s\\%s.EXE;STARTUPDIR=%s;PARAMETERS=%%;PROGTYPE=FULLSCREEN;ICONFILE=%s\\ICONS\\%s.ICO;MAXIMIZE=YES;CCVIEW=YES",
		 diskdir,ABBR,diskdir,diskdir,ABBR); 
    if ( !WinCreateObject( classname, title, setup, location, CO_UPDATEIFEXISTS) ) {
	printf(" ERROR: Unable to create Program Object\n");
    }

    printf("Building Kermit/2 Dialer Program Object\n");
    sprintf(title,"Kermit/2 Dialer");
    sprintf(classname,"WPProgram");
    sprintf(location,"<%sFLDR>",ABBR);
    sprintf(setup,"EXENAME=%s\\%sDIAL.EXE;STARTUPDIR=%s;PARAMETERS=%%;ICONFILE=%s\\%s.ICO",
	     diskdir,ABBR,diskdir,diskdir,"K95F_OS2");
    if ( !WinCreateObject( classname, title, setup, location, CO_UPDATEIFEXISTS) ) {
	printf(" ERROR: Unable to create Program Object\n");
    }

    printf("Building %s PM Clipboard Server Program Object\n",PRODUCT);
    sprintf(title,"%s PM Clipboard Server",PRODUCT);
    sprintf(classname,"WPProgram");
    sprintf(location,"<%sFLDR>",ABBR);
    sprintf(setup,"EXENAME=%s\\k2clip.exe;STARTUPDIR=%s;PARAMETERS=%%;ICONFILE=%s\\ICONS\\k2clip.ICO",
	     diskdir,diskdir,diskdir);
    if ( !WinCreateObject( classname, title, setup, location, CO_UPDATEIFEXISTS) ) {
	printf(" ERROR: Unable to create Program Object\n");
    }

}

int
os2UpdateConfigSys( char * destination )
{
    UCHAR backup[256] = "" ;
    UCHAR source[256] = "" ;
    UCHAR tempfile[256] = "" ;
    UCHAR pwd[256] = "" ; 
    int   drive ;
    UCHAR filespec[256] = "" ;
    UCHAR line[2048] = "" ;
    UCHAR newline[2048] = "" ;
    UCHAR *envstring = NULL ;
    int i=0;
    ULONG bootdrive ; 
    FILE * pfTemp, * pfConfig ;
    int len;
    int timeslice = 0 ;
    int k2ini = 0 ;
    int update = 0 ;

    envstring = (char *) malloc( 2048 ) ;
    strcpy( envstring, "ENVSTR=" ) ;

    printf( "\n" );
    printf( "Your CONFIG.SYS file defines several environment variables that determine\n");
    printf( "how OS/2 and your applications work.  With your permission, this program\n");
    printf( "will make the following changes:\n");
    printf( "\n");
    printf( " 1. The Kermit/2 directory, %s, will be added to your PATH\n",destination);
    printf( "    to allow Kermit/2 to be executed from any OS/2 Command Line session.\n");
    printf( "\n");
    printf( " 2. The Kermit/2 directory, %s, will be added to your LIBPATH\n",destination);
    printf( "    to allow Kermit/2 (or external protocols) to find necessary DLLs.\n");
    printf( "\n");
    printf( " 3. The Kermit/2 directory, %s, will be added to your DPATH\n",destination);
    printf( "    to allow the Kermit/2 to find your initialization files.\n");
    printf( "\n");
#ifdef COMMENT
    printf( " 4. The Kermit/2 directory, %s, will be added to your BOOKSHELF\n",destination);
    printf( "    to allow the view command to find the INF files.\n");
    printf( "\n");
#endif /* COMMENT */
    printf( "Please enter 'Yes' to allow this program to update your CONFIG.SYS file.\n");
    printf( "A backup copy will be made in the root directory of your boot drive.\n");
    printf( "\n");
    if (!getok("Update CONFIG.SYS?",'Y'))
    {
        printf( "\n" );
        printf( "  OK, a modified CONFIG.SYS file will be placed in %s.\n",destination );
    }
    else
	update = 1 ;
    
    printf( "\n" );

    DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &bootdrive, sizeof(ULONG)) ;

    if ( update ) {
	/* find a backup file name */
	for ( i=0;i<1000;i++ ) {
	    if ( i == 0 ) 
		sprintf(backup, "%c:\\CONFIG.BAK", 'A'+bootdrive-1 );
	    else if ( i <= 9 )
		sprintf(backup, "%c:\\CONFIG.BK%d", 'A'+bootdrive-1, i ) ;
	    else if ( i <= 99 )
		sprintf(backup, "%c:\\CONFIG.B%d", 'A'+bootdrive-1, i ) ;
	    else
		sprintf(backup, "%c:\\CONFIG.%d", 'A'+bootdrive-1, i ) ;
	    pfTemp = fopen(backup, "r");
	    if ( !pfTemp )
		break;
	    fclose( pfTemp ) ;
	}

	/* Now we have the backup file name */
	sprintf( source, "%c:\\CONFIG.SYS", 'A'+bootdrive-1 ) ;
	DosCopy( source, backup, 0 ) ;

	for ( i=0;i<1000;i++ ) {
	    sprintf(tempfile, "%c:\\K2TEMP.%d", 'A'+bootdrive-1, i ) ;
	    pfTemp = fopen(tempfile, "r");
	    if ( !pfTemp )
		break;
	    fclose( pfTemp ) ;
	}
    }
    else {
	sprintf(tempfile, "%s\\CONFIG.SYS", destination);
	sprintf( source, "%c:\\CONFIG.SYS", 'A'+bootdrive-1 ) ;
    }
    pfTemp = fopen(tempfile, "w+");
    pfConfig = fopen(source, "r" ) ;
    
    /* Generate the temporary CONFIG.SYS file. */
    if (update)
	printf( "Updating CONFIG.SYS...\n" );
    else
	printf( "Constructing %s\\CONFIG.SYS...\n", destination ) ;
    
    /* Change to a neutral directory so that SearchPath searches will succeed */

    drive = _getdrive() ;
    _getcwd( pwd, 256 ) ;
    _chdrive( bootdrive ) ;
    chdir( "\\OS2" ) ;

    /* Setup Environment String for Searches */
    putenv(envstring);

    /* Make sure that destination is upper case */
    strupr( destination ) ;

    /* Process the CONFIG.SYS file making changes as necessary */
    while ( !feof(pfConfig) ) {
	line[0] = '\0';
	fgets(line, 2048, pfConfig ) ;
	len = strlen( line ) ;
	if ( !strnicmp( "SET PATH", line, 8 ) ) {
	    putenv("ENVSTR");
	    strcpy( &envstring[7], &line[9] ) ;
	    putenv(envstring);
	    _searchenv( "k2.exe", "ENVSTR", filespec );
	    if ( filespec[0] ) {	/* Found */
		if ( !strnicmp( destination, filespec, strlen(destination) ) ) {
		    /* No changes necessary - already in PATH */
		    fprintf(pfTemp, "%s", line ) ;
		}
		else {
		    int matchpos ;
		    char * lastslash ;
		    strcpy( newline, line ) ;
		    strupr( newline ) ;
		    strupr( filespec ) ;
		    lastslash = strrchr( filespec, '\\' ) ;
		    if ( lastslash )
			*lastslash = '\0' ;
		    matchpos = strstr( newline, filespec ) - newline ;
		    if ( matchpos >= 0 && matchpos < 2048 ) {
			strncpy( newline, line, matchpos ) ;
			newline[matchpos] = '\0' ;
			strcat( newline, destination ) ;
			strcat( newline, line+matchpos+strlen(filespec) ) ;
			fprintf(pfTemp, "%s", newline ) ;
			printf( "\n");
			printf( "  WARNING:\n");
			printf( "    An earlier version of 'K2.EXE' exists in directory %s.\n",
				filespec);
		    }
		    else {
			printf("\n   ERROR: PATH matchpos = %d\n",matchpos ) ;
		    }
		}
	    }
	    else { /* Not Found - Add install directory to PATH */
		if ( line[len-1] == '\n' ) {
		    line[--len] = '\0' ;
		}
		if ( line[len-1] != ';' )
		    strcat( line, ";" ) ;
		strcat( line, destination ) ;
		strcat( line, ";\n" ) ;
		fprintf(pfTemp, "%s", line ) ;
	    }
	}
	else if ( !strnicmp( "SET DPATH", line, 9 ) ) {
	    putenv("ENVSTR");
	    strcpy( &envstring[7], &line[10] );
	    putenv(envstring);
	    _searchenv( "k2.ini", "ENVSTR", filespec );
	    if ( filespec[0] ) {	/* Found */
		if ( !strnicmp( destination, filespec, strlen(destination) ) ) {
		    /* No changes necessary - already in DPATH */
		    fprintf(pfTemp, "%s", line ) ;
		}
		else {
		    int matchpos ;
		    char * lastslash ;
		    strcpy( newline, line ) ;
		    strupr( newline ) ;
		    strupr( filespec ) ;
		    lastslash = strrchr( filespec, '\\' ) ;
		    if ( lastslash )
			*lastslash = '\0' ;
		    matchpos = strstr( newline, filespec ) - newline ;
		    if ( matchpos >= 0 && matchpos < 2048 ) {
			strncpy( newline, line, matchpos ) ;
			newline[matchpos] = '\0' ;
			strcat( newline, destination ) ;
			strcat( newline, line+matchpos+strlen(filespec) ) ;
			fprintf(pfTemp, "%s", newline ) ;
			printf( "\n");
			printf( "  WARNING:\n");
			printf( "    An earlier version of 'K2.INI' exists in directory %s.\n",
				filespec);
		    }
		    else {
			printf("\n   ERROR: DPATH matchpos = %d\n",matchpos ) ;
		    }
		}
	    }
	    else { /* Not Found - Add install directory to DPATH */
		if ( line[len-1] == '\n' ) {
		    line[--len] = '\0' ;
		}
		if ( line[len-1] != ';' )
		    strcat( line, ";" ) ;
		strcat( line, destination ) ;
		strcat( line, ";\n" ) ;
		fprintf(pfTemp, "%s", line ) ;
	    }
	} 
#ifdef COMMENT
	else if ( !strnicmp( "SET BOOKSHELF", line, 13 ) ) {
	    putenv("ENVSTR");
	    strcpy( &envstring[7], &line[14] ) ;
	    putenv(envstring);
	    _searchenv( "k2.inf", "ENVSTR", filespec );
	    if ( filespec[0] ) {	/* Found */
		if ( !strnicmp( destination, filespec, strlen(destination) ) ) {
		    /* No changes necessary - already in BOOKSHELF */
		    fprintf(pfTemp, "%s", line ) ;
		}
		else {
		    int matchpos ;
		    char * lastslash ;
		    strcpy( newline, line ) ;
		    strupr( newline ) ;
		    strupr( filespec ) ;
		    lastslash = strrchr( filespec, '\\' ) ;
		    if ( lastslash )
			*lastslash = '\0' ;
		    matchpos = strstr( newline, filespec ) - newline ;
		    if ( matchpos >= 0 && matchpos < 2048 ) {
			newline[matchpos] = '\0' ;
			strncpy( newline, line, matchpos ) ;
			strcat( newline, destination ) ;
			strcat( newline, line+matchpos+strlen(filespec) ) ;
			fprintf(pfTemp, "%s", newline ) ;
			printf( "\n");
			printf( "  WARNING:\n");
			printf( "    An earlier version of 'K2.INF' exists in directory %s.\n",
				filespec);
		    }
		    else {
			printf("\n   ERROR: BOOKSHELF matchpos = %d\n",matchpos ) ;
		    }

		}
	    }
	    else { /* Not Found - Add install directory to PATH */
		if ( line[len-1] == '\n' ) {
		    line[--len] = '\0' ;
		}
		if ( line[len-1] != ';' )
		    strcat( line, ";" ) ;
		strcat( line, destination ) ;
		strcat( line, "\\DOCS;\n" ) ;
		fprintf(pfTemp, "%s", line ) ;
	    }
	}
#endif /* COMMENT */
	else if ( !strnicmp( "LIBPATH", line, 7 ) ) {
	    putenv("ENVSTR");
	    strcpy( &envstring[7], &line[8] ) ;
	    putenv(envstring);
	    _searchenv( "cko32rt.dll", "ENVSTR", filespec );
	    if ( filespec[0] ) {	/* Found */
		if ( !strnicmp( destination, filespec, strlen(destination) ) ) {
		    /* No changes necessary - already in LIBPATH */
		    fprintf(pfTemp, "%s", line ) ;
		}
		else {
		    int matchpos ;
		    char * lastslash ;
		    strcpy( newline, line ) ;
		    strupr( newline ) ;
		    strupr( filespec ) ;
		    lastslash = strrchr( filespec, '\\' ) ;
		    if ( lastslash )
			*lastslash = '\0' ;
		    matchpos = strstr( newline, filespec ) - newline ;
		    if ( matchpos >= 0 && matchpos < 2048 ) {
			strncpy( newline, line, matchpos ) ;
			newline[matchpos] = '\0' ;
			strcat( newline, destination ) ;
			strcat( newline, line+matchpos+strlen(filespec) ) ;
			fprintf(pfTemp, "%s", newline ) ;
			printf( "\n");
			printf( "  WARNING:\n");
			printf( "     %s is located in the LIBPATH\n",filespec);
			printf( "     and it appears to be a previous installation of Kermit/2.\n");
			printf( "     If you have difficulties starting Kermit/2, try removing\n");
			printf( "     %s from the LIBPATH statement in CONFIG.SYS\n",filespec);
			printf( "     and restart OS/2.\n");
		    }
		    else {
			printf("\n   ERROR: LIBPATH matchpos = %d\n",matchpos ) ;
		    }
		}
	    }
	    else { /* Not Found - Add install directory to LIBPATH */
		if ( line[len-1] == '\n' ) {
		    line[--len] = '\0' ;
		}
		if ( line[len-1] != ';' )
		    strcat( line, ";" ) ;
		strcat( line, destination ) ;
		strcat( line, ";\n" ) ;
		fprintf(pfTemp, "%s", line ) ;
	    }
	}
	else if ( !strnicmp( "TIMESLICE", line, 9 ) ) {
	    timeslice = 1 ;
	    fprintf(pfTemp, "%s", line ) ;
	}
	else if ( !strnicmp( "SET CKERMIT.INI", line, 15 ) ||
		  !strnicmp( "SET K2.INI", line, 10 )) {
	    k2ini= 1 ;
	    fprintf(pfTemp, "%s", line ) ;
	}
	else 
	    fprintf(pfTemp, "%s", line ) ;
    };

    /* Free envstring from the environment */
    putenv("ENVSTR");
    free(envstring) ;
    envstring = NULL ;

    fprintf(pfTemp, "\nREM Begin Kermit/2 configuration and suggestions\n");
    if ( k2ini )
	fprintf(pfTemp, "REM ");
    fprintf(pfTemp, "SET K2.INI=%s\\K2.INI\n", destination ) ;
    /* if ( timeslice ) */
	fprintf(pfTemp, "REM ");
    fprintf(pfTemp, "TIMESLICE=32,125\n");
    fprintf(pfTemp, "REM PRIORITY_DISK_IO=NO\n");
    fprintf(pfTemp, "REM PRIORITY=DYNAMIC\n");
    fprintf(pfTemp, "REM MAXWAIT=3\n");
    fprintf(pfTemp, "REM End Kermit/2\n");

    fclose( pfTemp ) ;
    fclose( pfConfig ) ;

    if ( update ) {
	remove( source ) ;
	rename( tempfile, source ) ;
    }
    
    _chdrive( drive ) ;
    chdir( pwd ) ;
    
    if ( update ) {
	printf( "\n");
	printf( "  Your CONFIG.SYS file has been updated.\n");
	printf( "  Your old CONFIG.SYS has been copied to %s.\n",backup);
	printf( "  OS/2 must be shutdown and restart to activate the changes.\n");
    }
    return 1;
}
#endif /* NT */


/* Main function */

int
main(int argc, char ** argv) {
    int flag, x, y, i;
    long xx;

    char * s, * p;
    char * prompt;
    char c;

    int modem = -1;
    int port = 0;
    int printer = 0;
    int tone = 0;
    char * method = NULL;
    long portspeed = 0;
    char disk;
    char dir[257];
    char cwd[257];
    char diskdir[260];
    char *dirptr;
    int country = 0;			/* Country code */
    char area[256] = { (char) 0};
    char ld_prefix[256] = { (char) 0};
    char intl_prefix[256] = { (char) 0};

    int zip_install = 0 ;
    int new_install = 1 ;
#ifdef NT
    int altgr       = 0 ;
    int tapiavail   = 0 ;
#endif /* NT */
#ifndef NT
    int windowable  = 1 ;
#endif
    
    kermitinfo.dir_exists = 0;

#ifdef NT
    ConsoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (GetConsoleMode(ConsoleHandle, &ConsoleMode))
      KnowConsoleMode = 1;
    if ( tapiavail = cktapiinit() ) {
        cktapiopen();
        cktapiFetchLocationInfoByID(-1);    
        cktapiBuildLineTable( &tapilinetab, &_tapilinetab, &ntapiline );
        cktapiclose();
    }
#else
    setbuf( stdout, NULL );
#endif /* NT */

    s = argv[0];
    c = *s++;
    upper(&c);
    if (*s == ':') {
	s = mypwd();
	if (*(s+1) == ':') {
	    if (*s != c) {
		printf("\nERROR -\n\n");
		printf("  Please make \"%c:\" your current disk first.\n", c);
		printf("  Then run SETUP again.\n\n");
		doexit(1,0);
	    }
	}
    }

   if ( zchki(ZIPFILE) > 0 )
   {
      zip_install = 1 ;
   }
   else if ( zchki(ZIPFILE1) > 0 )
   {
      zip_install = 2 ;
   }
   else
   {
      s = getvolid(c);
      if (strcmp(s,DISKLABEL)) 
      {

         printf("\n");
         printf("Sorry, you must run SETUP from \"%s\".\n",DISKLABEL);
         doexit(1,0);
      }
   }
    mysystem("cls");
    initbar();
    drawbar();

#ifdef NT
    printf("Welcome to Kermit 95 for Windows(tm) %s.\n\n",
	   isWin95() ? "95" : "NT"
	   );
#else
    printf("Welcome to Kermit95 for OS/2 (Kermit/2).\n\n");
#endif

    printf("  This setup program will ask you a few questions.\n");
    printf("  Each question has a default answer, shown in [brackets].\n");
    printf("  To accept the default answer, just press the Enter key.\n");
    printf("\n");
    printf("  After you have answered the questions, your answers will be\n");
    printf(
"  summarized for you, and then you can choose to install the software\n");
    printf(
"  or not.  If you choose not to, nothing is done to your hard disk, and\n");
    printf(
"  you can run the setup program again if you wish.\n\n");

    printf("If this is a reinstallation, please be sure to shut down Kermit 95 and\n");
    printf("any related utilities before proceeding; otherwise they can not be updated.\n");

    drawbar();

    if (!getok("Do you wish to install Kermit?",'Y'))
        exit(0);

    printf("On which hard disk drive would you like to install %s?\n",PRODUCT);
    printf("Please type the disk-drive letter (C, D, E, ...).\n\n");

    prompt = " Disk letter";
    do {
       flag = 0;
       do {
	    if ((x = inchar(prompt, 'C')) < 0)
	      continue;
	    c = (char) x;
	    if (c == 13 || x == 10) {
		continue;
	    } else {
		upper(&c);
		if (c >= 'C' && c <= 'Z') {
		    if (!isdiskok(c)) {
			printf(
"\nSorry, disk %c is not available or has insufficient space.\n", c
			       );
			printf(
"Choose a different disk, or use Ctrl-C or Ctrl-Break to cancel SETUP.\n\n"
			       );
			continue;
		    } else {
			flag = 1;
		    }
		} else {
		    printf("Please type a letter, C - Z.\n");
		}
	    }
	} while (flag == 0);
	  printf(
"\n%s will be installed on disk %c.\n", PRODUCT, c);
    } while (!getok(NULL,'Y'));

    disk = c;

    printf("\n");
    printf("In which directory on disk %c should %s be installed?\n",
	   disk, PRODUCT );
    s = dir;
    do {
       dir[0] = (char) 0;
       s = dir;
       flag = 0 ;
       do {
	   printf("\n");
	   printf(
"Please type the directory name or press the Enter key to accept the\n"
);
	   printf("default directory name, %s.\n\n",ABBR);
	   printf(" Directory name [%s]: ",ABBR);
	   if (!gets(s))
	       continue;
	   if (!dir[0])
	       strcpy(dir,ABBR);
	   x = strlen(dir);
	   if ( x > 255 )
	   {
	       printf("\nplease limit the PATH to 255 characters.\n\n");
	       continue;
	   }	
	   {
	       for ( i = 0, y = 0; i<=8 && y < x && dir[y]; i++, y++ ) {
		   if ( dir[y] == '\\' || dir[y] == '/' )
		       i = -1 ;
		   else if ( dir[y] == '.' ) {
		       int j = 0 ;
			   i=0;
		       for ( y++ ; j<3 && y<x && dir[y]; j++, y++ ) {
			   if ( dir[y] == '\\' || dir[y] == '/' ) {
				   i = -1;
				   j = 0;
			       break;
			   }
		       }
		       if ( j == 3 && dir[y] ) 
			   i = 99 ;
		   }
	       }
	       if ( i > 8) {
		   printf("\nplease limit directories to 8.3 notation.\n\n");
		   continue;
	       }
	   }
	   flag = 1;
       } while (flag == 0);
       printf("\n");
       s = dir + x;
       while ((*s < (char) 33) && (s > dir))
          *s-- = (char) 0;
       dirptr = dir;
       while (*dirptr && (*dirptr < (char) 33))
          dirptr++;
       printf("%s will be installed in:\n\n",PRODUCT);
       if ( dir[0] == '\\' )
       {
          printf("  %c:%s\n\n", c, dirptr);
       }
       else 
       {
          printf("  %c:\\%s\n\n", c, dirptr);
       }
    } while (!getok(NULL,'Y'));
   if ( dir[0] == '\\' )
   {
      sprintf(diskdir, "%c:%s", disk, dir);
   }
   else
   { 
      sprintf(diskdir, "%c:\\%s", disk, dir);
   } 
   if (isdir(diskdir)) {
      printf("\n");
      printf("WARNING - %s already exists.\n\n", diskdir);
      if (!getok("  Do you want to continue? ",'N')) {
         printf("\n");
         printf("  OK, run SETUP again to do over.\n\n");
         printf("Bye.");
         drawbar();
         doexit(1,0);
      }
      kermitinfo.dir_exists = 1;
   }
   printf("\n");

    /* Check for install type: NEW or UPDATE */
    /* If CKERMIT.INI or K95.INI or K2.INI exists in the destination directory */
    /* assume that it is an Update, otherwise assume New install */

   drawbar();
   if (getok("Will you be using a modem? ",'Y')) {
      printf("\n");
#ifdef NT
       if ( tapiavail && ntapiline ) {
           printf("Please choose one of the following TAPI modems:\n\n");
           for ( i=0; i<ntapiline ; i++ ) {
               printf(" %2d. %-60s\n",i+1,tapilinetab[i].kwd);
           }
           printf("\n");
           prompt = " Enter a modem number";
           do {
               while (1) {
                   if ((x = getnum(prompt, 1)) < 0)
                       continue;
                   if (x < 1 || x > ntapiline) {
                       printf("Please enter a number between 1 and %d.\n",
                               ntapiline);
                       continue;
                   } else
                       break;
               }
               printf("\n");
               printf("You picked %d, %s\n", x, tapilinetab[x-1].kwd);
           } while (!getok(NULL,'Y'));
           modem = x - 1;

           printf("\n");
           drawbar();
       }
       else {
#endif /* NT */
      printf("Please choose one of the following modem types.\n");
      printf(
"If none of them matches yours or you don't know, pick Generic-High-Speed:\n\n"
);
      x = modems / 2;
      if (modems % 2) x++;
#ifdef COMMENT
	if (x > 21) {
	    printf("ERROR - Too many modems!\n");
	    doexit(1,0);
	}
#endif /* COMMENT */
	for (i = 0; i < x; i++) {
	    if (i + x < modems) {
		printf(" %2d. %-34s  %2d. %s\n",
		       i+1,
		       mdmtab[i].menuname,
		       i+x+1,
		       mdmtab[i+x].menuname);
	    } else {
		printf(" %2d. %s\n",i+1,mdmtab[i].menuname);
	    }
	}
	printf("\n");
	prompt = " Enter a modem number";
	do {
	    while (1) {
		if ((x = getnum(prompt, 15)) < 0)
		  continue;
		if (x < 1 || x > modems) {
		    printf("Please enter a number between 1 and %d.\n",modems);
		    continue;
		} else
		  break;
	    }
	    printf("\n");
	    printf("You picked %d, %s\n", x, mdmtab[x-1].menuname);
	} while (!getok(NULL,'Y'));
	modem = x - 1;

	printf("\n");
	drawbar();
	printf("Should your modem use Tone or Pulse dialing?\n\n");
	printf(" 1. Tone\n");
	printf(" 2. Pulse\n");
	printf(" 3. I don't know\n");
	printf("\n");

	printf("Please select 1, 2, or 3.\n");
	do {
	    while (1) {
		if ((x = getnum(" Dialing method selection", 1)) < 0)
		  continue;
		if (x < 1 || x > 3) {
		    printf("Please enter a number between 1 and 3.\n");
		    continue;
		} else
		  break;
	    }
	    printf("\n");
	    if (x == 1)
	      method = "Tone";
	    else if (x == 2)
	      method = "Pulse";
	    else if (x == 3)
	      method = "Don't know";
	    printf("You picked %d, %s\n", x, method);
	    if (x == 3) {
		printf("\n");
		printf(
" Then I will choose Pulse for you, since pulse dialing should work\n"
		       );
		printf(
" everywhere, whereas Tone dialing is only available in certain locations.\n"
		       );
		printf("\n");
		x = 2;
		method = "Pulse";
	    }
	} while (!getok(NULL,'Y'));
	if (x > 2) x = 0;
	tone = x;

	printf("\n");
	drawbar();
	printf("Which communications port is the modem attached to?\n\n");
	printf(" 1. COM1\n");
	printf(" 2. COM2\n");
	printf(" 3. COM3\n");
	printf(" 4. COM4\n");
	printf("\n");

	printf("Please enter a COM port number, 1 - 4\n");
	do {
	    while (1) {
		if ((x = getnum(" COM port number", 1)) < 0)
		  continue;
		if (x < 1 || x > 4) {
		    printf("Please enter a number between 1 and 4.\n");
		    continue;
		} else
		  break;
	    }
	    printf("\n");
	    printf("You picked %d, COM%d\n", x, x);
	} while (!getok(NULL,'Y'));
	port = x;

	printf("\n");
	drawbar();
	printf(
"The communication speed between COM%d and the modem will be:\n\n", port
	       );
	printf("  %ld\n\n", mdmtab[modem].speed);
	printf("bits per second (bps)\n\n");
	if (getok("Would you like to change the speed? ",'N')) {
	    printf("\n");
	    printf("Please choose one of the following speeds by number:\n");
	    printf("\n");
	    x = speeds / 2;
	    if (speeds % 2) x++;
	    for (i = 0; i < x; i++) {
		if (i + x < speeds)
		  printf("%2d. %6ld     %2d. %6ld\n",
			 i+1, speedtab[i], i+x+1, speedtab[i+x]
			 );
		else
		  printf("%2d. %6ld\n", i+1, speedtab[i]);
	    }
	    printf("\n");
	    printf("Please enter speed number, 1 - %d\n", speeds);
	    prompt = " Speed number";
	    do {
		while (1) {
		    if ((x = getnum(prompt, 4)) < 0)
		      continue;
		    if (x < 1 || x > speeds) {
			printf("Please enter a number between 1 and %d.\n",
			       speeds
			       );
			continue;
		    } else
		      break;
		}
		printf("\n");
		printf("You picked %d, %ld\n", x, speedtab[x-1]);
	    } while (!getok(NULL,'Y'));
	    portspeed = speedtab[x-1];
	} else
	  portspeed = mdmtab[modem].speed;


	if (!getdialinfo()) {

	    printf("\n");
	    drawbar();
	    printf(
"Now I will ask you a few questions about your dialing location, so I can\n"
		   );
	    printf(
"tell the difference between local, long-distance, and international phone\n"
		   );
	    printf(
"numbers, and so I can dial them correctly.  You might already have\n"
		   );
	    printf(
"answered some of these questions before when you configured your modem\n"
		   );
	    printf(
"for Windows 95, so if you are not sure of the answers, look in the\n"
		   );
	    printf(
"Control Panel modem configuration.  These questions are just the mininum\n"
		   );
	    printf(
"to get you dialing.  For further detail, see Dialing Options menu.\n"
		   );
	    printf("\n");
	    printf(
"Please enter your country code: a 1-, 2-, or 3-digit number that is used\n"
		   );
	    printf(
"to call into your country from the outside.  Some examples are: 1 for the\n"
		   );
	    printf(
"USA and Canada (etc), 44 for the UK, 49 for Germany, 7 for Russia.\n"
		   );
	    printf("\n");

	    do {
		while (1) {
		    if ((x = getnum(" Country code", 1)) < 0)
		      continue;
		    if (x < 1 || x > 999) {
			printf("Please enter a number between 1 and 999.\n");
			continue;
		    } else
		      break;
		}
		printf("\n");
		printf("You picked country code %d\n", x);
	    } while (!getok(NULL,'Y'));
	    country = x;
	    
	    printf("\n");
	    printf("Now please enter your area or city code.\n");
	    if (country != 1)
	      printf(
"If your country does not have city or areas codes, just press Enter\n"
		   );
	    printf("\n");
	    flag = 0;
	    s = "";
	    do {
		buf[0] = (char) 0;
		s = buf;
		do {
		    printf("\n");
		    printf(" Area code: ");
		    if (!gets(s))
		      continue;
		    if (!buf[0] && country == 1) {
			printf(
"An area code is required for country code %d\n\n", country
			       );
			continue;
		    }
		    flag = 1;
		} while (flag == 0);
		printf("\n");
		if (!buf[0]) {
		    printf("Your country has no area codes.\n");
		} else {
		    printf("Your area code is %s\n", buf);
		}
	    } while (!getok(NULL,'Y'));
	    strcpy(area,buf);

	    printf("\n");
	    printf(
"Please enter your long-distance dialing prefix, for dialing long-distance\n"
		   );
	    printf(
"calls within your country.  If your country does not have long-distance\n"
		   );
	    printf(
"calls, just press the Enter key.\n"
		   );
	    printf("\n");
	    flag = 0;
	    s = "";
	    do {
		buf[0] = (char) 0;
		s = buf;
		do {
		    printf("\n");
		    if (country == 1)
		      printf(" Long-distance prefix [1]: ");
		    else
		      printf(" Long-distance prefix: ");
		    if (!gets(s))
		      continue;
		    if (!buf[0] && country == 1)
		      strcpy(buf,"1");
		    flag = 1;
		} while (flag == 0);
		printf("\n");
		if (!buf[0]) {
		    printf("No long-distance prefix..\n");
		} else {
		    printf("Your long-distance prefix is %s\n", buf);
		}
	    } while (!getok(NULL,'Y'));
	    strcpy(ld_prefix,buf);

	    printf("\n");
	    printf(
"Finally, please enter your international dialing prefix, for dialing\n"
		   );
	    printf(
"numbers that are outside of your country.  If you don't know, or you do not\n"
		   );
	    printf(
"plan to make international calls, just press Enter.\n"
		   );
	    printf("\n");
	    flag = 0;
	    s = "";
	    do {
		buf[0] = (char) 0;
		s = buf;
		do {
		    printf("\n");
		    if (country == 1)
		      printf(" International prefix [011]: ");
		    else
		      printf(" International prefix: ");
		    if (!gets(s))
		      continue;
		    if (!buf[0] && country == 1)
		      strcpy(buf,"011");
		    flag = 1;
		} while (flag == 0);
		printf("\n");
		if (!buf[0]) {
		    printf("No international prefix..\n");
		} else {
		    printf("Your international prefix is %s\n", buf);
		}
	    } while (!getok(NULL,'Y'));
	    strcpy(intl_prefix,buf);
	}
           printf("\n");
           drawbar();
#ifdef NT
       }
#endif /* NT */
   }

    /* Lets get the default printer */
    printf("Which printer/port would you like as your default?\n\n");
    printf(" 1. PRN  (DOS device)\n");
    printf(" 2. LPT1 (DOS device)\n");
    printf(" 3. LPT2 (DOS device)\n");
    printf(" 4. LPT3 (DOS device)\n");
    printf(" 5. NUL  (DOS device)\n");
    printf(" 6. KERMIT.PRN (file)\n");
#ifdef NT
    if ( Win32EnumPrt( &w32prttab, &nw32prt ) ) {
        int i;
        for ( i=0 ; i<nw32prt ; i++ ) {
            printf("%2d. %s (Windows printer)\n",i+nprinters+1,w32prttab[i].kwd);
        }
    }
#endif /* NT */
    printf("\n");

    printf("Please enter a printer number, 1 - %d\n",
#ifdef NT
            nprinters + nw32prt 
#else
            nprinters
#endif
            );
    do {
        while (1) {
            if ((x = getnum(" printer number", 1)) < 0)
                continue;
            if (x < 1 || x > 
#ifdef NT
                 nprinters + nw32prt 
#else
                 nprinters
#endif
                 ) {
                printf("Please enter a number between 1 and %d.\n",
#ifdef NT
                        nprinters + nw32prt 
#else
                        nprinters
#endif
                        );
                continue;
            } else
                break;
        }
        printf("\n");
        printf("You picked %d, %s\n", x,
#ifdef NT
                ( x <= nprinters ) ? printers[x-1] : w32prttab[x-nprinters-1].kwd
#else
                printers[x-1]
#endif
                );
    } while (!getok(NULL,'Y'));
    printer = x;

#ifdef NT
    /* Lets take care of the Alt-Gr key situation on Windows 95 */
    if ( isWin95() && country != 1 ) {
	printf("\n");
	drawbar();
	printf("Microsoft Windows 95 has a bug in the Console Agent which affects\n");
	printf("Kermit-95's ability to recognize characters generated by Alt-Gr key\n");
        printf("combinations.  To implement a work-around for this bug, Kermit-95\n");
	printf("needs to know if your keyboard has an Alt-Gr key.\n");
	printf("\n");
	do {
	    altgr = getok("Does your keyboard have an Alt-Gr key?",
                           tapiavail?'N':'Y');
	    printf("\n");
	    printf("You picked %s\n",altgr?"yes, you have an Alt-Gr key":
		    "no, you don't have an Alt-Gr key");
	} while ( !getok(NULL,'Y') );
    }
#endif /* NT */

#ifndef NT
    printf("\n");
    drawbar();
    printf("How would you like %s to be executed?\n\n", PRODUCT);
    printf(" 1. In a Window\n");
    printf(" 2. Full Screen\n");
    printf("\n");

    printf("Please enter a number, 1 - 2\n");
    do {
	while (1) {
	    if ((x = getnum(" Run mode", 1)) < 0)
		continue;
	    if (x < 1 || x > 2) {
		printf("Please enter a number between 1 and 2.\n");
		continue;
	    } else
		break;
	}
	printf("\n");
	printf("You picked %d, %s\n", x, x==1?"In a Window":"Full Screen");
    } while (!getok(NULL,'Y'));
    windowable = (x==1);
#endif /* NT */

    printf("\n");
    drawbar();
    printf("SUMMARY\n\n");
    printf("You have chosen:\n\n");
    printf(" Disk:                 %c\n", disk);
    printf(" Directory:            %s\n\n", dirptr);
    if (modem > -1) {
#ifdef NT
        if ( tapiavail && ntapiline ) {
            printf(" Modem:                %s\n", _tapilinetab[modem].kwd);
        }
        else {
#endif
            printf(" Modem:                %s\n", mdmtab[modem].menuname);
            printf(" Port:                 COM%d\n", port);
            printf(" Speed:                %ld\n\n", portspeed);
            printf(" Dialing Method:       %s\n", method);
            printf(" Country Code:         %d\n", country);
            printf(" Area Code:            %s\n", area);
            printf(" Long-distance prefix: %s\n", ld_prefix);
            printf(" International prefix: %s\n", intl_prefix);
#ifdef NT
        }
#endif
    } else {
	printf(" Modem:                (none)\n");
    }
    printf(" Printer:              %s\n", 
#ifdef NT
            ( printer <= nprinters ) ? printers[printer-1] : 
            w32prttab[printer-nprinters-1].kwd
#else
            printers[printer-1]
#endif
            );
#ifdef NT
    if ( isWin95() && country != 1 ) {
	printf(" Alt-Gr key:           %s\n",altgr?"Yes":"No");
    }
#endif /* NT */
#ifndef NT
    printf(    " Run Mode:             %s\n", windowable?"In a Window":"Full Screen");
#endif

    printf("\n");
    if (!getok(
#ifdef NT
        "May I install Kermit 95 now? "
#else
        "May I install Kermit/2 now? "           
#endif
        ,'Y')) {
	printf("\n");
	printf("  %s will NOT be installed.\n",PRODUCT);
	printf("  Run SETUP again to do over.\n");
	printf("\n");	
	printf("Bye!\n");
	drawbar();
	doexit(0,0);
    }
    printf("\n");
    drawbar();
    if (!kermitinfo.dir_exists) {
	printf("Creating %s...\n", diskdir);
	if (!mymkdir(diskdir)) {
	    printf("ERROR - Can't create directory: %s\n\n", diskdir);
	    printf(
"  Please check writeability and space on disk %c and then run SETUP again.\n",
		   disk
		   );
	    printf("\nBye.\n");
	    drawbar();
	    doexit(1,0);
	}
    }
    printf("\n");
   if ( zip_install )
   {
      strcpy(cwd, zgtdir() ) ;

      if ( zip_install == 1 ) /* Packed in one file */
      {
         printf("Expanding files from %s distribution to %s...\n\n",
		 PRODUCT,diskdir);

	  if ( !zchdir(diskdir) ) {
	     printf("Unable to cd to %s\n",diskdir);
	      doexit(1,0);
	  }

         if ( cwd[strlen(cwd)-1] == '\\' )
            sprintf(buf,"\"%s%s\"", cwd, ZIPFILE ) ;
         else
            sprintf(buf,"\"%s\\%s\"", cwd, ZIPFILE ) ;
         if ( !mysystem(buf) )
         {
            zchdir( cwd ) ;
            printf("\nFile expansion failed.\n");
            printf("Please check your %c disk and try again.\n",disk);
            printf("\n");
            doexit(1,0) ;
         }
      }
      else
      {
         /* Expand first Pack File */
         printf("Expanding files from %s distribution to %s...\n\n",
		 PRODUCT,
                 diskdir);

         zchdir(diskdir) ;

         if ( cwd[strlen(cwd)-1] == '\\' )
            sprintf(buf,"\"%s%s\"", cwd, ZIPFILE1 ) ;
         else
            sprintf(buf,"\"%s\\%s\"", cwd, ZIPFILE1 ) ;
         if ( !mysystem(buf) )
         {
            zchdir( cwd ) ;
            printf("\nFile expansion failed.\n");
            printf("Please check your %c disk and try again.\n\n",disk);
            doexit(1,0) ;
         }

          if ( zchki(K95EXE) <= 0 ) {
              zchdir( cwd ) ;

              while (zchki(ZIPFILE2) <= 0) {	   /* Get second disk */
                  printf("\nPlease insert %s Diskette 2.\n",PRODUCT);    
                  while (!getok("Ready? (y/n)",'Y'))
                      ;
              }

              /* Expand second Pack File */
              printf("\nExpanding files from %s distribution to %s...\n\n",
                      PRODUCT, diskdir);

              zchdir(diskdir) ;

              if ( cwd[strlen(cwd)-1] == '\\' )
                  sprintf(buf,"\"%s%s\"", cwd, ZIPFILE2 ) ;
              else
                  sprintf(buf,"\"%s\\%s\"", cwd, ZIPFILE2 ) ;
              if ( !mysystem(buf) )
              {
                  zchdir( cwd ) ;
                  printf("\nFile expansion failed.\n");
                  printf("Please check your %c disk and try again.\n",disk);
                  printf("\n");
                  doexit(1,0) ;
              }
          }
          if ( zchki(K95EXE) <= 0 ) {
              zchdir( cwd ) ;

              while (zchki(ZIPFILE3) <= 0) {	   /* Get second disk */
                  printf("\nPlease insert %s Diskette 3.\n",PRODUCT);    
                  while (!getok("Ready? (y/n)",'Y'))
                      ;
              }

              /* Expand second Pack File */
              printf("\nExpanding files from %s distribution to %s...\n\n",
                      PRODUCT, diskdir);

              zchdir(diskdir) ;

              if ( cwd[strlen(cwd)-1] == '\\' )
                  sprintf(buf,"\"%s%s\"", cwd, ZIPFILE3 ) ;
              else
                  sprintf(buf,"\"%s\\%s\"", cwd, ZIPFILE3 ) ;
              if ( !mysystem(buf) )
              {
                  zchdir( cwd ) ;
                  printf("\nFile expansion failed.\n");
                  printf("Please check your %c disk and try again.\n",disk);
                  printf("\n");
                  doexit(1,0) ;
              }
          }
          if ( zchki(K95EXE) <= 0 ) {
              zchdir( cwd ) ;

              while (zchki(ZIPFILE4) <= 0) {	   /* Get second disk */
                  printf("\nPlease insert %s Diskette 4.\n",PRODUCT);    
                  while (!getok("Ready? (y/n)",'Y'))
                      ;
              }

              /* Expand second Pack File */
              printf("\nExpanding files from %s distribution to %s...\n\n",
                      PRODUCT, diskdir);

              zchdir(diskdir) ;

              if ( cwd[strlen(cwd)-1] == '\\' )
                  sprintf(buf,"\"%s%s\"", cwd, ZIPFILE4 ) ;
              else
                  sprintf(buf,"\"%s\\%s\"", cwd, ZIPFILE4 ) ;
              if ( !mysystem(buf) )
              {
                  zchdir( cwd ) ;
                  printf("\nFile expansion failed.\n");
                  printf("Please check your %c disk and try again.\n",disk);
                  printf("\n");
                  doexit(1,0) ;
              }
          }
      }
       zchdir( cwd ) ;
   }
   else
   {
      printf("Copying files from %s diskette 1 to %s...\n\n",PRODUCT,diskdir);

      sprintf(buf,"XCOPY *.* \"%s\" /S", diskdir);
      if (!mysystem(buf)) {
         printf("\nFile copy failed.\n");
         printf("Please check your %c disk and try again.\n", disk);
         printf("\n");
         doexit(1,0);
      }
      while (1) {				/* Get second disk */
         printf("\n");
         printf("Please insert %s Diskette 2.\n",PRODUCT);    
         while (!getok("Ready? (y/n)",'Y'))
            ;
         s = getvolid(c);
         if (strcmp(s,DISK2LABEL)) {
            printf("\n");
            printf("Sorry, \"%s\" is not \"%s\".\n",s,DISK2LABEL);
            continue;
         } else
            break;
      }
      printf("\n");
      printf("Copying files from %s diskette 2 to %s...\n\n",PRODUCT,diskdir);
      sprintf(buf,"XCOPY *.* \"%s\" /S", diskdir);
      if (!mysystem(buf)) {
         printf("\nFile copy failed.\n");
         printf("Please check your %c disk and try again.\n", disk);
         printf("\n");
         doexit(1,0);
      }
    
   }

    /* Create INIT file for Dialer */
    sprintf(buf,"%s\\%sDIAL.INF",diskdir,ABBR);
#ifdef FOR_REAL
    if (!(fp = fopen(buf,"w"))) {
	printf("WARNING - Can't create %s\n", buf);
	/* doexit(1,0); */
    }
#else
    fp = stdout;
    fprintf(fp, "\n");
#endif /* FOR_REAL */

    if (modem > -1) {
#ifdef NT
        if ( tapiavail && ntapiline ) {
            fprintf(fp, "directory %s\n",diskdir);
            fprintf(fp, "modem tapi\n"); 
            fprintf(fp, "port %s\n", _tapilinetab[modem].kwd);
            fprintf(fp," printer %s\n", 
#ifdef NT
                ( printer < nprinters ) ? printers[printer-1] : 
                     w32prttab[printer-nprinters-1].kwd
#else
                printers[printer-1]
#endif
                    );
        }
        else {
#endif /* NT */
            fprintf(fp, "directory %s\n",diskdir);
            fprintf(fp, "modem %s\n", mdmtab[modem].kermitname);
            fprintf(fp, "port com%d\n", port); 
            fprintf(fp," printer %s\n", 
#ifdef NT
                ( printer < nprinters ) ? printers[printer-1] : 
                     w32prttab[printer-nprinters-1].kwd
#else
                printers[printer-1]
#endif
                    );
            if (method && tone > 0)
                fprintf(fp, "method %s\n", method);
            fprintf(fp, "speed %ld\n", portspeed);
            fprintf(fp, "country %d\n", country);
            fprintf(fp, "area %s\n", area);
            fprintf(fp, "ldprefix %s\n", ld_prefix);
            if (country == 1) {
                fprintf(fp, "tfprefix %s\n", ld_prefix);
	    fprintf(fp, "tfarea %s\n", "800 888 877 866");
            }
            fprintf(fp, "intlprefix %s\n", intl_prefix);
#ifdef NT
        }
#endif /* NT */
    }
    else {
	fprintf(fp, "directory %s\n",diskdir);
	fprintf(fp, "modem generic-high-speed\n");
	fprintf(fp, "port com1\n", port); 
        fprintf(fp," printer %s\n", 
#ifdef NT
                ( printer < nprinters ) ? printers[printer-1] : 
                 w32prttab[printer-nprinters-1].kwd
#else
                printers[printer-1]
#endif
                    );
	fprintf(fp, "method Pulse\n");
	fprintf(fp, "speed 57600\n");
    }
#ifdef FOR_REAL
    fclose(fp);
#endif /* FOR_REAL */

#ifdef FOR_REAL
    sprintf(buf,"%s\\%sCUSTOM.INI",diskdir,ABBR);

    if ( zchki(buf) >= 0 ) {
	printf("WARNING - %s already exists.\n",buf);
	sprintf(buf,"%s\\%sCUSTOM.INS",diskdir,ABBR);
	printf("          Output from SETUP will be placed in %s.\n",buf);
    }
    if (!(fp = fopen(buf,"w"))) {
	printf("WARNING - Can't create %s\n", buf);
    }
#else
    fp = stdout;
    fprintf(fp, "\n");
#endif /* FOR_REAL */
    fprintf(fp, "; FILE %sCUSTOM.INI -- %s Customizations\n\n",ABBR,PRODUCT);
    fprintf(fp, "; Created by SETUP.EXE\n\n");
    
    if ( modem > -1 ) {
#ifdef NT
        if ( tapiavail && ntapiline ) {
            fprintf(fp, "set modem type none\n");
            fprintf(fp, "xif not started-from-dialer {\n");
            fprintf(fp, "set tapi line %s\n", _tapilinetab[modem].kwd); 
            fprintf(fp, "set modem type tapi\n");
            fprintf(fp, "}\n");
        }
        else {
#endif /* NT */
	fprintf(fp, "set modem type %s\n", mdmtab[modem].kermitname);
	fprintf(fp, "xif not started-from-dialer {\n");
	fprintf(fp, "set port com%d\n", port); 
	fprintf(fp, "set speed %ld\n", portspeed);
	fprintf(fp, "}\n");

	if (tone > 0)
	    fprintf(fp, "set dial method %s\n", method);
	fprintf(fp, "set dial country-code %d\n", country);
	fprintf(fp, "set dial area-code %s\n", area);
	fprintf(fp, "set dial ld-prefix %s\n", ld_prefix);
	if (country == 1) {
	    fprintf(fp, "set dial tf-prefix %s\n", ld_prefix);
	    fprintf(fp, "set dial tf-area %s\n", "800 888 877 866");
	}
	fprintf(fp, "set dial intl-prefix %s\n", intl_prefix);
#ifdef NT
        }
#endif /* NT */
    }
    else {
	fprintf(fp, ";set modem type <modem_type>\n");
	fprintf(fp, ";xif not started-from-dialer {\n");
	fprintf(fp, ";set port <port>\n"); 
	fprintf(fp, ";set speed <speed>\n");
	fprintf(fp, ";}\n");
	fprintf(fp, ";set dial method <tone or pulse>\n");
	fprintf(fp, ";set dial country-code <country_code>\n");
	fprintf(fp, ";set dial area-code <area_code>\n");
	fprintf(fp, ";set dial ld-prefix <long_dist_prefix>\n");
	fprintf(fp, ";set dial tf-prefix <toll_free_prefix>\n");
	fprintf(fp, ";set dial tf-area <800 and/or 888>\n");
	fprintf(fp, ";set dial intl-prefix <international_prefix>\n");
    }

#ifdef NT
    if ( printer < nprinters ) 
        fprintf(fp,"set printer /DOS-DEVICE:{%s}\n", printers[printer-1] );
    else
        fprintf(fp,"set printer /WINDOWS-QUEUE:{%s}\n", 
                 w32prttab[printer-nprinters-1].kwd);
#else
    fprintf(fp,"set printer {%s}\n", printers[printer-1] );
#endif
#ifdef NT
    if ( altgr )
	fprintf(fp, "set win95 alt-gr on\n");
#endif /* NT */
    fprintf(fp, "\n");
    fprintf(fp, "; (Fill in your other customization commands here...)\n");
    fprintf(fp, "End ; of %sCUSTOM.INI\n",ABBR);
#ifdef FOR_REAL
    fclose(fp);
#endif /* FOR_REAL */

    strcpy(kermitinfo.install_dir,diskdir);
    strcpy(kermitinfo.modem_name,
	   (modem > -1) ?
	   mdmtab[modem].kermitname :
	   ""
	   );
    kermitinfo.com_port = port;
    kermitinfo.port_speed = portspeed;
    kermitinfo.c_code = country;
    strcpy(kermitinfo.a_code,area);
    strcpy(kermitinfo.l_prefix,ld_prefix);
    if (country == 1) {
	strcpy(kermitinfo.t_area,"800 888 877 866");
	strcpy(kermitinfo.t_prefix,ld_prefix);
    }
    strcpy(kermitinfo.i_prefix,intl_prefix);

    zchdir(diskdir);			/* Change to the K-95 directory */

#ifdef NT
    /* Make sure that CTL3D32.DLL is installed */
    drawbar();
    printf("\n");
    mysystem("ctl3dins.exe");
    printf("\n");
#endif /* NT */

#ifdef CLINK
/* This doesn't work in a console app */
    if (isWin95()) {			/* Put shortcut on desktop */
	char * WinDir;
	if (WinDir = getenv("winbootdir")) {
	    sprintf(buf,"%s\\Desktop", WinDir);
	    printf("Creating Shortcut: \"%s\" => K95DIAL.EXE...\n", buf);
	    (void) CreateLink((LPCSTR) "k95dial.exe",
			      (LPSTR) buf,
			      (LPSTR) "K-95 Dialer"
			      );
	}
    }
    printf("\n");
#endif /* COMMENT */
#ifdef NT
    drawbar();
    printf("\n");
    printf(
" Now please enter your name, your company (optional), and %s serial\n",PRODUCT
	   );
    printf(
" number into the Registration Window...\n\n"
	   );    
   printf(
" In the Serial Number box, type your entire %s serial number, beginning with\n",ABBR);
printf(" %s and ending with 1.1, over the template that is shown in the box,\n",ABBR);
printf(" including the punctuation.\n\n");

printf(" When you have finished registering, click the mouse back on this window to\n");
printf(" continue SETUP (if necessary)...\n");

    mysystem("ckreg");

    printf("\n");
    drawbar();
    printf("\n");
    mysystem("k95regtl");

#else
    mysystem("k2reg");
#endif
    printf("\n");
    drawbar();
    printf("\n");

#ifndef NT
    os2Folder(diskdir,new_install,windowable);
    os2UpdateConfigSys(diskdir);
    printf("\n");
    drawbar();
    printf("\n");
#endif

    if (getok("Would you like to read the README.TXT file now? (y/n)",'Y')) {
#ifdef NT
	sprintf(buf,"edit README.TXT",diskdir);
#else /* NT */
	sprintf(buf,"e README.TXT",diskdir);
#endif /* NT */
	mysystem(buf);
    }

    printf("\n");
    printf(
"  %s has been installed in %s.\n", PRODUCT,diskdir);
    printf(
"  To run it, open the folder and click on the colorful %sDIAL icon.\n",ABBR);
    printf(
"  To create a shortcut icon for %s on your desktop, follow the\n",PRODUCT);
    printf(
"  instructions in the README.TXT file.\n");
    printf("\n");
    printf(
"  Please be sure to fill out and mail in your registration card with one\n");
    printf(
"  of your serial-number stickers attached to it.  This gives you access\n");
    printf(
"  to the %s support BBS for news and patches, and it entitles you\n",PRODUCT);
    printf(
"   to a discount on the next major release.\n");
    printf("\n");
    printf("Thank you for choosing %s!\n",PRODUCT);
    drawbar();
    printf("\n");
#ifdef NT
    if (getok("Would you like to start Kermit 95 now? (y/n)",'Y')) {
	mysystem("k95dial.exe");
    } else {
	printf("\n");
    }
#else
    if (getok("Would you like to start Kermit/2 now? (y/n)",'Y')) {
       mysystem("k2dial.exe");
    } else {
	printf("\n");
    }
#endif
    doexit(0,0);
    return(0);
}
