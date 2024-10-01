#ifndef WIN32
#define INCL_DOSSESMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_WINWORKPLACE
#define INCL_WINSHELLDATA
#define INCL_DOSMODULEMGR
#endif /* WIN32 */
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_WINDOW_DIALER
#define USE_HELP_CONTEXTS
#define USE_WINDOW_PASSWORD
#ifdef WIN32
#include <shellapi.h>
#ifndef CKT_NT31
#include <shlguid.h>
#include <shlobj.h>
#endif /* CKT_NT31 */
#include <objbase.h>
#endif /* WIN32 */
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <direct.h>
#include <sys/stat.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetdlg.hpp"
#include "kconnect.hpp"
#include "klocation.hpp"
#include "kmodem.hpp"
#include "kmdmdlg.hpp"
#include "ksetgeneral.hpp"
#include "ksetterminal.hpp"
#include "ksetserial.hpp"
#include "ksettelnet.hpp"
#include "ksettcp.hpp"
#include "ksettls.hpp"
#include "ksetlogin.hpp"
#include "ksetlogs.hpp"
#include "ksetprinter.hpp"
#include "ksetkeyboard.hpp"
#include "ksetkerberos.hpp"
#include "ksetxfer.hpp"
#include "ksetssh.hpp"
#include "ksetftp.hpp"
#include "ksetgui.hpp"
#include "kdialopt.hpp"
#include "kdconfig.hpp"
#include "kquick.hpp"
#include "usermsg.hpp"
#include "kdirdial.hpp"
#include "kdirnet.hpp"
#include "kabout.hpp"
#include "kstatus.hpp"
#include "kappl.hpp"
#include "kdemo.hpp"

extern "C" {
#define OS2
#ifdef WIN32
#define NT
#endif /* WIN32 */
#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckucmd.h"
#include "ckoetc.h"
#undef printf
#undef fprintf
#include "kmdminf.h"
MDMINF * FindMdmInf( char * name );
}

#define DIRSEP       '/'
#define ISDIRSEP(c)  ((c)=='/'||(c)=='\\')
#define NUL          '\0'

extern K_CONNECTOR  *connector;
extern ZIL_UINT8 kd_major, kd_minor;

#ifdef WIN32
extern "C" {
#include <windows.h>            	/* Windows Definitions */
#ifndef NODIAL
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>
#include <mcx.h>
#include "ktapi.h"
    extern struct keytab * tapilinetab, * _tapilinetab;
    extern int ntapiline;

    void 
    ReloadTapiLocations( void )
    {
        connector->LoadTapiLocations();
        connector->UpdateLocationMenuItems();
    }
#endif
}
#endif /* WIN32 */

void CopyMdmInfToModem( MDMINF *, K_MODEM * );
extern ZIL_UINT8 TapiAvail, Desktop, StartMenu;
extern "C" {
VOID ck_decrypt( char * );
VOID ck_encrypt( char * );
}

ZIL_STORAGE * K_CONNECTOR::_userFile = ZIL_NULLP(ZIL_STORAGE) ;
ZIL_STORAGE_READ_ONLY * K_CONNECTOR::_preloadFile = ZIL_NULLP(ZIL_STORAGE) ;
ZIL_STORAGE_READ_ONLY * K_CONNECTOR::_orgloadFile = ZIL_NULLP(ZIL_STORAGE) ;

#ifdef WIN32
PROCESS_INFORMATION StartKermitProcessInfo ;
#endif

extern "C" {
char *
GetLoadPath(void) {
#ifdef WIN32
   static char filename[512] ;
   filename[0] = '\0' ;
   GetModuleFileName( NULL, filename, 512 ) ;
   return filename;
#else /* WIN32 */
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

static char *
UNCname(char * name) {
    static char UNCnam[CKMAXPATH+1];        /* Scratch buffer for UNC names */
    int i;

    for ( i=0; name[i] && i <= CKMAXPATH; i++ )
        UNCnam[i] = (name[i] == '/' ? '\\' : name[i]);
    UNCnam[i] = '\0';
    return(UNCnam);
}


#ifndef WIN32
#ifndef S_IFMT
/* Watcom defines this as 0170000 (octal) which is 0xF000 (hex). Same value,
 * different representation, warning about macro definition not being identical */
#define S_IFMT 0xF000
#endif /* S_IFMT */
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* S_ISDIR */

int
os2stat(char *path, struct stat *st) {
    char local[CKMAXPATH];
    int len;
    int rc;

    if (!path)
        return(-1);

    if ( !strncmp(path,"//",2) ) {
        int i;
        strncpy((char *)local, UNCname(path), CKMAXPATH);
        len = strlen((char *)local);
        if ( ISDIRSEP(local[len-1]) ) {
            /* We now need to strip the trailing directory separator if it is not */
            /* part of the machine or object name.                                */
            for ( i=2; i<len && !(ISDIRSEP(local[i])); i++ ); /* machine */
            for ( i++; i<len && !(ISDIRSEP(local[i])); i++ ); /* object */

            if (i < len-1 )
                local[len-1] = '\0';
        }
    } else {
        strncpy((char *)local, path, CKMAXPATH);
        len = strlen((char *) local);
        if ( len == 2 && local[1] == ':' )
            local[2] = DIRSEP, local[3] = 0; /* if drive only, append / */
        else if ( len == 0 )
            local[0] = DIRSEP, local[1] = 0; /* if empty path, take / instead */
        else if ( len > 1 && ISDIRSEP(local[len - 1]) && local[len - 2] != ':')
            local[len - 1] = 0; /* strip trailing / except after d: */
    }
    rc = stat(local, st);
    return(rc);
}

/*
   Tell if string pointer s is the name of an existing directory.
   Returns 1 if directory, 0 if not a directory.
*/
int
isdir(char *s) {
    int x;
    struct stat statbuf;
    static char prevpath[CKMAXPATH+4] = { '\0', '\0' };
    static int prevstat = -1;

    if (!s) return(0);
    if (!*s) return(0);

    if ( prevstat > -1 ) {
        if ( s[0] == prevpath[0] ) {
            if ( !strcmp(s,prevpath) ) {
                return(prevstat);
            }
        }
    }

    /* Disk letter like A: is top-level directory on a disk */
    if (((int)strlen(s) == 2) && (isalpha(*s)) && (*(s+1) == ':')) {
        prevstat = 1;
        strncpy(prevpath,s,CKMAXPATH+4);
        return(1);
    }
    x = os2stat(s,&statbuf);
    if (x == -1) {
        prevstat = -1;
        return(0);
    } else {
        prevstat = (S_ISDIR (statbuf.st_mode) ? 1 : 0);
        strncpy(prevpath,s,CKMAXPATH+4);
        return( prevstat );
    }
}

VOID
kstrip(char *name, char **name2) {
    static char work[CKMAXPATH+1];
    char *cp, *pp;
    int n = 0;
    if (!name) { *name2 = ""; return; }
    pp = work;
    /* Strip disk letter and colon */
    if (isalpha(*name) && (*(name+1) == ':')) name += 2;

    for (cp = name; *cp; cp++) {
        if (ISDIRSEP(*cp)) {
            pp = work;
            n = 0;
        } else {
            *pp++ = *cp;
            if (n++ >= CKMAXPATH)
              break;
        }
    }
    *pp = '\0';                         /* Terminate the string */
    *name2 = work;
}

int
kcopy(char *source, char *destination) {
    char *p = NULL, *s;
    int x;
    int len;

    printf("kcopy %s => %s\n",source,destination);

    if (!source) source = "";
    if (!destination) destination = "";

    if (!*source) return(-1);
    if (!*destination) return(-1);

    s = destination;

    if (isdir(destination)) {
        char *q = NULL;

        x = strlen(destination);
        len = strlen(destination) + strlen(source) + 2;
        if (!(p = (char *)malloc(len)))
          return(-1);
        strcpy(p,destination);          /* Directory part */
        if (!ISDIRSEP(*(destination+x-1))) /* Separator, if needed */
            strcat(p,"\\");
        kstrip(source,&q);              /* Strip path part from old name */
        strcat(p,q);                    /* Concatenate to new directory */
    } else {
        len = strlen(destination) + 64;
        if (!(p = (char *)malloc(len)))
          return(-1);
        strcpy(p,destination);
    }

    s = p;

    if ( !strncmp(s,"//",2 ) )
         strcpy(s,UNCname(s));
    if ( !strncmp(s,"//",2 ) )
        source = UNCname(source);

    printf("kcopy %s => %s\n",source,s);
#ifndef WIN32
    x = (DosCopy( source, s, DCPY_FAILEAS ) ? -1 : 0);
#else /* WIN32 */
    x = (CopyFile( source, s, FALSE ) ? 0 : -1);
#endif /* WIN32 */

    printf("CopyFile() returns %d\n",x);

    if (p) free(p);
    return(x);
}

int
kmkdir(const char *path) {
    char *xp, *tp, c;
    int x, count = 0;

    if (!path) path = "";
    if (!*path) return(-1);

    x = strlen(path);
    if (x < 1 || x > CKMAXPATH)           /* Check length */
      return(-1);
    if (!(tp = (char *)malloc(x+1)))            /* Make a temporary copy */
      return(-1);
    strcpy(tp,path);

    if ( !strncmp(tp,"//",2) ) {        /* Fixup for UNC if necessary */
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
            x =                         /* No, try to create it */
              _mkdir(tp);               /* The IBM way */
            if (x < 0) {
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

#ifdef WIN32
char *
GetAppData( int common )
{
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static char  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL, *env;
    int i;

    printf("GetAppData common = %d\n",common);
#ifdef COMMENT
    env = getenv(common ? "ALLUSERSPROFILE" : "APPDATA");
    if (env) {
        printf("GetAppData env = %s\n",env);
        strncpy(lpszKeyValue,env,CKMAXPATH);
        if ( common )
            strncat(lpszKeyValue,"\\Application Data",sizeof(lpszKeyValue));
    } else 
#endif /* COMMENT */    
    {
        printf("GetAppData env = (null)\nCalling RegOpenKeyEx()\n");
        if ( RegOpenKeyEx(common ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkShellKey) ) {
            printf("GetAppData RegOpenKeyEx() failed\n");
            if ( !common )
                return NULL;

            printf("Calling RegCreateKeyEx()\n");
            if (RegCreateKeyEx(common ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                                NULL, &hkShellKey, NULL)) {
                printf("GetAppData RegCreateKeyEx() failed\n");
                return NULL;            /* failed */
            }
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = common ? "Common AppData" : "AppData";
        printf("Calling RegQueryValueEx(%s)\n",lpszValueName);
        if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                              (CHAR *)lpszKeyValue, &dwSize ))
        {
            printf("RegQueryValueEx(%s) failed\n",lpszValueName);
            lpszValueName = common ? "Common Desktop" : "Desktop";
            printf("Calling RegQueryValueEx(%s)\n",lpszValueName);
            if ( !RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                                  (CHAR *)lpszKeyValue, &dwSize ))
            {
                /* success */
                printf("RegQueryValueEx(%s) succeeds = %s\n",
                        lpszValueName,lpszKeyValue);
                for (i = strlen(lpszKeyValue); 
                     lpszKeyValue[i] != '\\' && lpszKeyValue[i] != '/';
                     i-- );
                lpszKeyValue[i] = '\0';
                strncat(lpszKeyValue,"Application Data",CKMAXPATH+1);
            } else {
                /* failure */
                printf("Calling RegQueryValueEx(%s) failed\n",lpszValueName);
                printf("Calling GetWindowsDirectory()\n");
                GetWindowsDirectory(lpszKeyValue,sizeof(lpszKeyValue));
                printf("GetWindowsDirectory() == %s\n",lpszKeyValue);
                strncat(lpszKeyValue,"\\Application Data",CKMAXPATH+1);
            }

            printf("Calling RegSetValueEx(%s)\n",lpszKeyValue);
			lpszValueName = common ? "Common AppData" : "AppData";
            RegSetValueEx( hkShellKey, 
                           lpszValueName, 
                           0, REG_SZ,
                           (CHAR *)lpszKeyValue, strlen(lpszKeyValue)+1);
            printf("RegSetValueEx(%s) done\n",lpszValueName);
            strncat(lpszKeyValue,"\\",CKMAXPATH+1);
            kmkdir(lpszKeyValue);
        }
        RegCloseKey( hkShellKey );
    }

    printf("Registry routines finished\n");

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '\\';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '/' )
          lpszKeyValue[i] = '\\';
    }

    printf("GetAppData() returns %s\n",lpszKeyValue);
    return(lpszKeyValue);
}

char *
GetHomeDrive(void)
{
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static char lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;

    env = (CHAR *)getenv("HOMEDRIVE");
    if (env) {
        strncpy(lpszKeyValue,(const char *)env,sizeof(lpszKeyValue));
    } else {

        if ( RegOpenKeyEx(HKEY_CURRENT_USER,
                          "Volatile Environment", 
                           0, KEY_READ, &hkShellKey) )
            return NULL;            /* failed */

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = (CHAR *)"HOMEDRIVE";
        if ( RegQueryValueEx( hkShellKey, (char *)lpszValueName, NULL, &dwType,
                              (CHAR *)lpszKeyValue, &dwSize ))
        {
            RegCloseKey( hkShellKey );
            return(NULL);
        }
        RegCloseKey( hkShellKey );
    }
    return(lpszKeyValue);
}

char *
GetHomePath(void)
{
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static char lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;
    int   i;

    env = (CHAR *)getenv("HOMEPATH");
    if (env) {
        strncpy(lpszKeyValue,(const char *)env,sizeof(lpszKeyValue));
    } else {
        if ( RegOpenKeyEx(HKEY_CURRENT_USER,
                          "Volatile Environment", 
                           0, KEY_READ, &hkShellKey) )
            return NULL;            /* failed */

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = (CHAR *)"HOMEPATH";
        if ( RegQueryValueEx( hkShellKey, (char *)lpszValueName, NULL, &dwType,
                              (CHAR *)lpszKeyValue, &dwSize ))
        {
            RegCloseKey( hkShellKey );
            return(NULL);
        }

        RegCloseKey( hkShellKey );
    }

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '\\';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '/' )
          lpszKeyValue[i] = '\\';
    }

    return(lpszKeyValue);
}

char *
GetPersonal(void)
{
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static char lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;
    int   i;

    if ( RegOpenKeyEx(HKEY_CURRENT_USER,
                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                      0, KEY_READ, &hkShellKey) )
        return NULL;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = (CHAR *)"Personal";
    if ( RegQueryValueEx( hkShellKey, (char *)lpszValueName, NULL, &dwType,
                          (CHAR *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkShellKey );
        return(NULL);
    }

    RegCloseKey( hkShellKey );

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '\\';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '/' )
          lpszKeyValue[i] = '\\';
    }

    return(lpszKeyValue);
}
#endif /* WIN32 */
} /* "C" */

K_PASSWORD::
K_PASSWORD( K_CONNECTOR * Parent, ZIL_ICHAR * password, ZIL_UINT8 * ok )
    : ZAF_DIALOG_WINDOW("WINDOW_PASSWORD",defaultStorage), 
    _parent(Parent), _password(password), _ok(ok)
{
    windowManager->Center(this);
}

EVENT_TYPE K_PASSWORD::
Event( const UI_EVENT & event )
{
   EVENT_TYPE ccode = event.type ;
   UIW_STRING * string = NULL ;

   switch ( event.type ) {
      case OPT_DIALING_OK: {
	  UIW_STRING * string = (UIW_STRING *) Get( FIELD_PASSWORD );
          strncpy(_password,string->DataGet(),PASSWD_SZ);
          _password[PASSWD_SZ-1] = 0;
	  *windowManager - this ;
          *_ok = 1;
	  break;
      }

      default:
         ccode = UIW_WINDOW::Event(event);
   }
   return ccode ;
}

K_PASSWORD::
~K_PASSWORD( void )
{
    
}

ZIL_INT32
get_dir_len(ZIL_ICHAR *path)
{

  ZIL_INT32 i;

  i = strlen(path);
  while (--i >= 0) {
    if (path[i] == '\\' || path[i] == '/' || path[i] == ':')
      break;
  }
  return(i + 1);
}


ZIL_ICHAR K_CONNECTOR::_className[] = "K_CONNECTOR";

K_CONNECTOR::K_CONNECTOR(void)
   : UIW_WINDOW("WINDOW_DIALER",defaultStorage)
{
    printf("Entering K_CONNECTOR(void)\n");
    connector = this;
    StartKermitErrorCode = 0 ;
    StartKermitFileName = NULL ;
    searchID = ID_DIALER_STORAGE ;
#ifdef WIN32
    _hwndNextConnect = 0;
    _dwNextConnectPid = 0;
#endif 

    windowManager->Center(this) ;

    StatusName =   (UIW_STRING *) Get( STATUS_NAME ) ;
    StatusTime =   (UIW_TIME *) Get( STATUS_TIME ) ;
    StatusString = (UIW_STRING *) Get( STATUS_TEXT ) ;
    ToolBar =      (UIW_TOOL_BAR *) Get( TOOL_BAR ) ;
    VtList =       (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 

#ifdef WIN32
    Information( I_SET_TEXT, "Kermit 95 Dialer" ) ;
#else
    Information( I_SET_TEXT, "Kermit-95 for OS/2 Dialer" ) ;
#endif /* WIN32 */

    StatusTime->tmFlags |= TMF_SECONDS | TMF_TWELVE_HOUR ;

    UIW_PULL_DOWN_MENU * Menu = (UIW_PULL_DOWN_MENU *) Get( MENU );
    UIW_PULL_DOWN_ITEM * MenuItem = (UIW_PULL_DOWN_ITEM *) Get( MENU_REGISTER );
    *Menu - MenuItem;

    if ( !IsManualInstalled() ) {
        printf("Manual is not installed\n");
        UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_HELP_MANUAL );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
    }

#ifdef WIN32
    /* 
     *  What we will do is check to see if the Common and User App Data
     *  directories exist.  If they do not, we will create them.
     */

    char dir[CKMAXPATH+1], file[CKMAXPATH+1], *appdata, *common;

    printf("Calling GetAppData(1)\n");
    common = (char *)GetAppData(1);
    printf("GetAppData(1) == %s\n",common ? common : "(null)");
    if ( common ) {
        sprintf(dir,"%s%s",(char *)common, "Kermit 95\\");
        if ( !isdir(dir) ) {
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\SSH\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\CERTS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\CRLS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\KEYMAPS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\PHONES\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)common, "Kermit 95\\SCRIPTS\\");
            kmkdir(dir);
        }

    }
    printf("Calling GetAppData(0)\n");
    appdata = (char *)GetAppData(0);
    printf("GetAppData(0) == %s\n",appdata ? appdata : "(null)");
    if ( appdata ) {
        sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\");
        if ( !isdir(dir) ) {
            kmkdir(dir);

            if ( common = GetAppData(1) ) {
                sprintf(file,"%s%s",(char *)common, "Kermit 95\\K95CUSTOM.INI");
                kcopy(file,dir);
                appdata = GetAppData(0);
            }

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\SSH\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\CERTS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\CRLS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\KEYMAPS\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\PHONES\\");
            kmkdir(dir);

            sprintf(dir,"%s%s",(char *)appdata, "Kermit 95\\SCRIPTS\\");
            kmkdir(dir);
        }

    }
    printf("Calling GetPersonal()\n");
    appdata = (char *)GetPersonal();
    printf("GetAppData(0) == %s\n",appdata ? appdata : "(null)");
    if ( appdata ) {
        sprintf(dir,"%s%s",(char *)appdata, "DOWNLOAD\\");
        if ( !isdir(dir) )
            kmkdir(dir);
    }

    printf("Searching for dialusr.dat\n");
    /* Find and Load the DIALUSR.DAT file */
    ZIL_ICHAR path[300]="";
    ZIL_STORAGE_READ_ONLY * _userFileRO;
    sprintf(path,"%sKermit 95\\dialusr.dat",GetAppData(0));
    _userFileRO = new ZIL_STORAGE_READ_ONLY(path) ;
    if (_userFileRO->storageError) {
		delete _userFileRO;
        sprintf(path,"%sKermit 95\\dialusr.dat",GetAppData(1));
        _userFileRO = new ZIL_STORAGE_READ_ONLY(path) ;
    }
    if (_userFileRO->storageError) {
		delete _userFileRO;
        sprintf(path,"dialusr.dat");
        _userFileRO = new ZIL_STORAGE_READ_ONLY(path) ;
    }
    if (_userFileRO->storageError) {
        /* First Use code goes here */
        sprintf(path,"%sKermit 95\\dialusr.dat",GetAppData(0));
    }
    delete _userFileRO;
#else /* WIN32 */
    ZIL_ICHAR path[300]="";
    sprintf(path,"dialusr.dat");
#endif /* WIN32 */

    printf("dialusr.dat == %s\n",path);
    _userFile = new ZIL_STORAGE(path, UIS_OPENCREATE | UIS_READWRITE ) ;
    if ( _userFile->storageError ) {
        ZAF_MESSAGE_WINDOW * message =
            new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to create/open DIALUSR.DAT (%d)",
								    _userFile->storageError) ;
        message->Control();
        delete message ;
		delete _userFile;
        exit(16);
    }
    printf("dialusr.dat open\n");

#ifdef WIN32
    sprintf(path,"%sKermit 95\\dialorg.dat",GetAppData(1));
    _orgloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
    if ( _orgloadFile->storageError ) {
        delete _orgloadFile;
        sprintf(path,"%sKermit 95\\dialorg.dat",GetAppData(0));
        _orgloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
        if ( _orgloadFile->storageError ) {
            delete _orgloadFile;
            sprintf(path,"dialorg.dat");
            _orgloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
        }
    }

    sprintf(path,"%sKermit 95\\dialinf.dat",GetAppData(1));
    _preloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
    if ( _preloadFile->storageError ) {
        delete _preloadFile;
        sprintf(path,"%sKermit 95\\dialinf.dat",GetAppData(0));
        _preloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
        if ( _preloadFile->storageError ) {
            delete _preloadFile;
            sprintf(path,"dialinf.dat");
            _preloadFile = new ZIL_STORAGE_READ_ONLY(path) ;
        }
    }
#else /* WIN32 */
    _orgloadFile = new ZIL_STORAGE_READ_ONLY("dialorg.dat");
    _preloadFile = new ZIL_STORAGE_READ_ONLY("dialinf.dat");
#endif /* WIN32 */
	if (_orgloadFile->storageError) {
		delete _orgloadFile;
		_orgloadFile = NULL;
	}
	if (_preloadFile->storageError) {
		delete _preloadFile;
		_preloadFile = NULL;
	}

    _userFile->MkDir("Config");
    _userFile->MkDir("Modems");
    _userFile->MkDir("Locations");
    LoadConfig() ;
    if ( _config->_convert_location )
        GenerateDefaultLocationFromConfig();
    LoadLocations();
#if defined(WIN32)
#ifndef NODIAL
    LoadTapiLocations();
#endif /* NODIAL */
#endif
    LoadModems();
#if defined(WIN32)
#ifndef NODIAL
    LoadTapiModems();
#endif /* NODIAL */
#endif
    /* Make sure there is at least one modem */
    if ( _modem_list.Count() == 0 ) {
        K_MODEM * modem = (K_MODEM *) new K_MODEM();
        if ( !modem )
            OutofMemory("Unable to create DEFAULT MODEM");
        strncpy(modem->_name,"DEFAULT",60);
        strncpy(modem->_port,"COM1",60);
        strncpy(modem->_type,"generic-high-speed",60);
        MDMINF * mdminf = FindMdmInf( modem->_type );
        CopyMdmInfToModem( mdminf, modem );
        _modem_list + modem;
    }
    LoadEntries() ;

    KD_LIST_ITEM * defentry = FindEntry( "DEFAULT" );
    if ( defentry == NULL ) {
        defentry = new KD_LIST_ITEM;
        ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
				     "DEFAULT",
				     ID_KD_LIST_ITEM, 
				     UIS_OPENCREATE | UIS_READWRITE ) ;
		defentry->Store("DEFAULT", K_CONNECTOR::_userFile, &_fileObj,
                            ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM));
        K_CONNECTOR::_userFile->Save() ;
        LoadEntries() ;
    }
    if ( ReadKermitInfo( _config, defentry ) )
    {
        if ( defentry ) {
            ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
				     "DEFAULT",
				     ID_KD_LIST_ITEM, 
				     UIS_OPENCREATE | UIS_READWRITE ) ;
            defentry->Store("DEFAULT", K_CONNECTOR::_userFile, &_fileObj,
                            ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM));
            K_CONNECTOR::_userFile->Save() ;
            LoadEntries() ;
        }
        GenerateDefaultLocationFromConfig();
        SaveConfig() ;
    }
    if ( _config->_current_entry[0] )
    {
        *VtList + FindEntry( _config->_current_entry, TRUE ) ;
    }
    UpdateLocationMenuItems();
#ifdef WIN32
    UpdateTapiLineConfigMenuItems();
    HINSTANCE hLib = LoadLibrary("WSHELP32");
    if ( hLib ) {
        _dns_srv_avail = 1;
        // FreeLibrary(hLib);
    } else
#endif
        _dns_srv_avail = 0;
#ifdef WIN32
    HINSTANCE hK5 = LoadLibrary("KRB5_32");
    if ( hK5 ) {
        _krb5_avail = 1;
        // FreeLibrary(hLib);
    } else
#endif
        _krb5_avail = 0;
#ifdef WIN32
    HINSTANCE hK4 = LoadLibrary("KRBV4W32");
    if ( hK4 ) {
        _krb4_avail = 1;
        // FreeLibrary(hLib);
    } else
#endif
        _krb4_avail = 0;
#ifdef WIN32
    _krb5_avail = 1;
    _libeay_avail = 1;
    _ssh_avail = 1;
    /*
     * The old method of figuring out if we have OpenSSL
     * and LibSSH available no longer works - the OpenSSL
     * DLL name keeps changing as we upgrade to new versions
     * and most people are using 64bit builds of C-Kermit
     * now which means 64bit versions of OpenSSL and LibSSH.
     * The dialer is permanantly stuck in 32bit land which
     * means even if we know the filename we can't
     * LoadLibrary it.
     *
    HINSTANCE hEAY = LoadLibrary("LIBEAY32");
    if ( hEAY ) {
        _libeay_avail = 1;
        // FreeLibrary(hLib);
    } else
        _libeay_avail = 0;

    HINSTANCE hSSH = LoadLibrary("ssh");
    if ( hSSH ) {
        _ssh_avail = 1;
         FreeLibrary(hSSH);
    } else
        hSSH = 0;
        */
#else
    char *exe_path;
    char * dllname = "SSLEAY2";
    char errbuf[256];
    int  rc;
    HMODULE hSSL;

    exe_path = GetLoadPath();
    sprintf(path, "%.*s%s.DLL", get_dir_len(exe_path), exe_path, dllname);
    rc = DosLoadModule(errbuf, 256, path, &hSSL);
    if (rc)
        rc = DosLoadModule(errbuf, 256, dllname, &hSSL);
    _libeay_avail = !rc;
#endif 

    if ( !(_krb4_avail || _krb5_avail) ) {
        UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_EDIT_KERBEROS );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
    }
    if ( !_libeay_avail ) {
        UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_EDIT_SSH );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
        popup = (UIW_POP_UP_ITEM *) Get( MENU_EDIT_TLS );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
    }
#ifndef WIN32
    // Not supported on OS/2
    {
        UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_EDIT_SSH );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
        popup = (UIW_POP_UP_ITEM *) Get( MENU_EDIT_GUI );
        popup->woFlags |= WOF_NON_SELECTABLE;
        popup->Information( I_CHANGED_FLAGS, NULL );
    }
#endif /* WIN32 */

    UpdateStatusLine(TRUE) ;

    os2InitFromRegistry();
}

K_CONNECTOR::~K_CONNECTOR(void)
{
	
    SaveConfig();
    _userFile->Save(2) ;
    delete _userFile ;
    _userFile=NULL;
	if (_preloadFile) {
		delete _preloadFile ;
		_preloadFile = NULL;
	}
    if (_orgloadFile) {
		delete _orgloadFile ;
		_orgloadFile = NULL;
	}
}

void
K_CONNECTOR::GenerateDefaultLocationFromConfig( void )
{
    ZIL_ICHAR *LocationName = "K95 Default";
    K_LOCATION * OldLoc = FindLocation( LocationName );
    if ( OldLoc ) {
        _location_list - OldLoc;
        delete OldLoc;
        OldLoc = NULL;
    }

    K_LOCATION * NewLoc = new K_LOCATION();
    if ( !NewLoc )
        OutofMemory("Unable to create LOCATION");
    strncpy(NewLoc->_name,LocationName,60);
    NewLoc->_name[60] = 0;
    strncpy(NewLoc->_country_code,_config->_country_code,3) ;      
    NewLoc->_country_code[3] = 0;
    strncpy(NewLoc->_area_code,_config->_area_code,8);
    NewLoc->_area_code[8] = 0;
    strncpy(NewLoc->_dial_prefix,_config->_dial_prefix,32);
    NewLoc->_dial_prefix[32] = 0;
    strncpy(NewLoc->_dial_suffix,_config->_dial_suffix,32);
    NewLoc->_dial_suffix[32] = 0;
    NewLoc->_dial_method = _config->_dial_method;      
    NewLoc->_dial_timeout = _config->_dial_timeout;         
    NewLoc->_redial_count = _config->_redial_count;         
    NewLoc->_redial_interval = _config->_redial_interval;      

    strncpy(NewLoc->_local_area_prefix,_config->_local_area_prefix,32); 
    NewLoc->_local_area_prefix[32] = 0;
    strncpy(NewLoc->_local_area_suffix,_config->_local_area_suffix,32); 
    NewLoc->_local_area_suffix[32] = 0;
    
    strncpy(NewLoc->_long_dist_prefix,_config->_long_dist_prefix,32);
    NewLoc->_long_dist_prefix[32] = 0;
    strncpy(NewLoc->_long_dist_suffix,_config->_long_dist_suffix,32); 
    NewLoc->_long_dist_suffix[32] = 0;
    
    strncpy(NewLoc->_intl_prefix,_config->_intl_prefix,32);
    NewLoc->_intl_prefix[32] = 0;
    strncpy(NewLoc->_intl_suffix,_config->_intl_suffix,32);      
    NewLoc->_intl_suffix[32] = 0;
    
    strncpy(NewLoc->_dial_tf_area,_config->_dial_tf_area,32);    
    NewLoc->_dial_tf_area[32] = 0;
    strncpy(NewLoc->_dial_tf_prefix,_config->_dial_tf_prefix,32);
    NewLoc->_dial_tf_prefix[32] = 0;
                           
    NewLoc->_use_pbx = _config->_use_pbx ;
    strncpy(NewLoc->_pbx_out,_config->_pbx_out,32);
    NewLoc->_pbx_out[32] = 0;
    strncpy(NewLoc->_pbx_exch,_config->_pbx_exch,32);
    NewLoc->_pbx_exch[32] = 0;
    strncpy(NewLoc->_pbx_in,_config->_pbx_in,32);
    NewLoc->_pbx_in[32] = 0;

    if (K_CONNECTOR::_userFile->ChDir("~Locations") == 0) {
        ZIL_STORAGE_OBJECT _fileObj( *_userFile, LocationName, ID_K_LOCATION,
                                     UIS_OPENCREATE | UIS_READWRITE ) ;
        NewLoc->Store( LocationName, _userFile, &_fileObj,
                       ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
        if ( _userFile->storageError )
        {
            // What went wrong?
            ZAF_MESSAGE_WINDOW * message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to Write location \"%s\": errorno %d", 
                                   LocationName,
                                   _userFile->storageError ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW 1");
            message->Control();
            delete message ;
        }   
        K_CONNECTOR::_userFile->ChDir("~");
        _location_list + NewLoc;
    }
}

EVENT_TYPE 
KLocationMenuUser( UI_WINDOW_OBJECT * obj, UI_EVENT & event,
                  EVENT_TYPE ccode )
{
    UIW_POP_UP_ITEM * item = (UIW_POP_UP_ITEM *) obj ;
    K_LOCATION * location = NULL;


    switch ( ccode )
    {
    case L_SELECT: 
        /* a new location has been selected */
        for ( location = (K_LOCATION *) connector->_location_list.First();
              location; location = (K_LOCATION *) location->Next()) {
            if ( !strcmp( location->_name, item->DataGet()) ) {
                connector->_location_list.SetCurrent( location );
#if defined(WIN32)
#ifndef NODIAL
                if ( TapiAvail && location->_is_tapi )
                    cktapiSetCurrentLocationID( location->_tapi_location_id );
#endif
#endif  
                break;
            }
        }    
        break;

    case S_CURRENT:
        break;

    case S_NON_CURRENT:
        break;
    }    
    return 0;
}

void
K_CONNECTOR::UpdateLocationMenuItems( void )
{
    UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_LOCATION_CURRENT );
    UIW_POP_UP_MENU * menu = &(popup->menu);
    UIW_POP_UP_ITEM * current = NULL;
    for ( popup = (UIW_POP_UP_ITEM *) menu->First();
         popup; 
         popup = (UIW_POP_UP_ITEM *) menu->First()) {
        *menu - popup;
        delete popup;
    }

    K_LOCATION * location;
    for ( location = (K_LOCATION *) _location_list.First();
          location;
          location = (K_LOCATION *) location->Next()) 
    {
        popup = new UIW_POP_UP_ITEM( location->_name, MNIF_CHECK_MARK,
                                     BTF_NO_FLAGS, WOF_NO_FLAGS,
                                     KLocationMenuUser);

        if ( !popup )
            OutofMemory("Unable to create POP_UP_ITEM 1");
        if ( popup )
            *menu + popup;

        if ( location == _location_list.Current() )
            popup->woStatus |= WOS_SELECTED;
        popup->Information( I_CHANGED_FLAGS, NULL );
    }

    if ( current )
        *menu + current;
}

#ifdef WIN32
#ifndef NODIAL
void
K_CONNECTOR::LoadTapiModems( void )
{
    if ( !TapiAvail )
        return;

    cktapiBuildLineTable( &tapilinetab, &_tapilinetab, &ntapiline );

    for ( int i=0 ; i<ntapiline ; i++ ) { 
        K_MODEM * modem = FindModem( tapilinetab[i].kwd );
        if ( !modem ) {
            modem = new K_MODEM();
            if ( !modem )
                OutofMemory("Unable to create MODEM while loading TAPI");
            strncpy(modem->_name, tapilinetab[i].kwd,60);
            modem->_name[60] = 0;
            strncpy(modem->_type, "tapi",60);
            modem->_type[60] = 0;
            strncpy(modem->_port, _tapilinetab[i].kwd,60);
            modem->_port[60] = 0;
            modem->_is_tapi = 1;

            if (K_CONNECTOR::_userFile->ChDir("~Modems") == 0) {
                ZIL_STORAGE_OBJECT * _fileObj = 
                    new ZIL_STORAGE_OBJECT( *_userFile, modem->_name, ID_K_MODEM ) ;
                if ( !_fileObj )
                    OutofMemory("Unable to create Modems STORAGE_OBJECT");
                modem->Store( modem->_name, K_CONNECTOR::_userFile, _fileObj,
                              ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
                delete _fileObj;
                K_CONNECTOR::_userFile->ChDir("~");

                _modem_list + modem;
            }
        }
    }

}

void
K_CONNECTOR::LoadTapiLocations( void )
{
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    LPLINELOCATIONENTRY lpLocationEntry = NULL;
    LPLINECOUNTRYLIST  lpCountryList  = NULL;
    LPLINECOUNTRYENTRY lpCountryEntry = NULL;
    DWORD dwSizeofCountryList = sizeof(LINECOUNTRYLIST);
    int                 NLocation,NCountry,i;
    K_LOCATION          *current=NULL;

    if ( !TapiAvail )
        return;

    if ( !cktapiGetLocationEntries( &lpTranslateCaps, &lpLocationEntry, &NLocation ) ||
	 !cktapiGetCountryEntries( &lpCountryList, &NCountry ))
    	return;

    printf("%d TAPI Locations\r\n", NLocation);
    printf("%d TAPI Countries\r\n", NCountry);

    if ( NLocation > 0 )
    {
	// enumerate all the locations
	for( i=0; i < NLocation; i++ )
	{
	    // Put each one into the keytab
	    char * newstr = _strdup((const char *)(((LPBYTE) lpTranslateCaps) + 
				      lpLocationEntry[i].dwLocationNameOffset)) ;
	    int    newval = lpLocationEntry[i].dwPermanentLocationID;
	    int    j=0, len=0;

            if ( !newstr )
                OutofMemory("Unable to create string while enumerating locations");
	    /* Use Underscores instead of spaces */
	    len = strlen(newstr);
	    for ( j=0 ; j<len ; j++ )
	    {
		switch ( newstr[j] ) {
		case ' ':
		    newstr[j] = '_';
		    break;
		case ',':
		    newstr[j] = '.';
		    break;
		case ';':
		    newstr[j] = ':';
		    break;
		case '\\':
		    newstr[j] = '/';
		    break;
		case '?':
		    newstr[j] = '!';
		    break;
		case '{':
		    newstr[j] = '[';
		    break;
		case '}':
		    newstr[j] = ']';
		    break;
		}
	    }

	    /* Find the Country Entry */
	    for ( j=0; j<NCountry; j++)
	    {
		lpCountryEntry = (LPLINECOUNTRYENTRY) ((BYTE *)lpCountryList +
							lpCountryList->dwCountryListOffset +
							(j * sizeof(LINECOUNTRYENTRY)));
                if ( lpCountryEntry->dwCountryID == lpLocationEntry[i].dwCountryID )
                    break;
            }
            if ( j == lpCountryList->dwNumCountries )
                lpCountryEntry = NULL;

	    /* 
	       Now need to find the location in _location_list
	       and if it isn't there create a new location and
	       add it to the list.

	       Still not sure what to do about marking things
	       TAPI or not, and whether or not to save them in
	       the database.  What about changes to the TAPI
	       Location entry between sessions?
	    */
	    K_LOCATION * location = FindLocation( newstr );
            K_LOCATION * oldloc   = NULL;
	    if ( location ) {
		/* Then there is already an entry, so better 
		   check to see if things have changed.  
		*/
		if ( location->_tapi_location_id != lpLocationEntry[i].dwPermanentLocationID ||
		     location->_tapi_country_id  != lpLocationEntry[i].dwCountryCode ||
		     strcmp(location->_tapi_area_code,(((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwCityCodeOffset)) ||
		     location->_tapi_preferred_card_id != lpLocationEntry[i].dwPreferredCardID ||
		     location->_tapi_options != lpLocationEntry[i].dwOptions ||
		     strcmp(location->_tapi_local_access_code,
			     (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwLocalAccessCodeOffset)) ||
		     strcmp(location->_tapi_long_dist_code,
			     (((LPSTR) lpTranslateCaps) +
		       lpLocationEntry[i].dwLongDistanceAccessCodeOffset)) ||
		     strcmp(location->_tapi_toll_prefix_list,
			     (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwTollPrefixListOffset)) ||
		     strcmp(location->_tapi_cancel_call_waiting,
			     (((LPSTR) lpTranslateCaps) +
			       lpLocationEntry[i].dwCancelCallWaitingOffset))
		     ) 
		{
		    /* something has changed in the TAPI database   */
		    /* so we need to update things.  Best way to do */
		    /* that is to delete the current entry and rebuild it */
		    /* from scratch */
		    _location_list - location;
		    delete location;
		    location = NULL;
		}

		if ( location && lpCountryEntry ) {
		    /* we haven't detected a change yet.  But we still have  */
		    /* to check the Country information.  Something might    */
		    /* have changed at the registry level.                   */
		    if ( strcmp(location->_tapi_country_name,
				 ((LPSTR) lpCountryList +
			       lpCountryEntry->dwCountryNameOffset)) ||
			 strcmp(location->_tapi_same_area_rule,
				 ((LPSTR) lpCountryList +
			       lpCountryEntry->dwSameAreaRuleOffset)) ||
			 strcmp(location->_tapi_long_dist_rule,
				 ((LPSTR) lpCountryList +
			       lpCountryEntry->dwLongDistanceRuleOffset)) ||
			 strcmp(location->_tapi_intl_rule,
				 ((LPSTR) lpCountryList +
			       lpCountryEntry->dwInternationalRuleOffset))
			 )
		    {
			/* something has changed in the TAPI database   */
			/* so we need to update things.  Best way to do */
			/* that is to delete the current entry and rebuild it */
			/* from scratch */
			_location_list - location;
                        oldloc = location;
			location = NULL;
		    }
		}
	    }

	    if ( !location ) {
		/* Either we couldn't find the location or we detected a change */
		/* So build a new one */
                if ( oldloc )
                    location = new K_LOCATION(*oldloc);
                else
                    location = new K_LOCATION();
                if ( !location )
                    OutofMemory("Unable to create LOCATION while loading TAPI");
		strncpy(location->_name,newstr,60);
                location->_name[60] = 0;
		location->_is_tapi = TRUE;
		location->_tapi_location_id = lpLocationEntry[i].dwPermanentLocationID;
		strncpy(location->_tapi_name, 
			 (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwLocationNameOffset),
			 lpLocationEntry[i].dwLocationNameSize>256 ? 256 : lpLocationEntry[i].dwLocationNameSize);
                location->_tapi_name[255] = 0;
		location->_tapi_country_id = lpLocationEntry[i].dwCountryID;
		location->_tapi_country_code = lpLocationEntry[i].dwCountryCode;

		if ( lpCountryEntry ) {
		    /* 'E' = Country Code */
		    /* 'F' = Area Code    */
		    /* 'G' = Local Number */
		    strncpy(location->_tapi_country_name,
			     ((LPSTR) lpCountryList +
			       lpCountryEntry->dwCountryNameOffset),
			     lpCountryEntry->dwCountryNameSize> 32 ? 32 : lpCountryEntry->dwCountryNameSize);
                    location->_tapi_country_name[31] = 0;
		    strncpy(location->_tapi_same_area_rule,
			     ((LPSTR) lpCountryList +
			       lpCountryEntry->dwSameAreaRuleOffset),
			     lpCountryEntry->dwSameAreaRuleSize > 32 ? 32 : lpCountryEntry->dwSameAreaRuleSize);
                    location->_tapi_same_area_rule[31] = 0;
		    strncpy(location->_tapi_long_dist_rule,
			     ((LPSTR) lpCountryList +
			       lpCountryEntry->dwLongDistanceRuleOffset),
			     lpCountryEntry->dwLongDistanceRuleSize > 32 ? 32 : lpCountryEntry->dwLongDistanceRuleSize);
                    location->_tapi_long_dist_rule[31] = 0;
		    strncpy(location->_tapi_intl_rule,
			     ((LPSTR) lpCountryList +
			       lpCountryEntry->dwInternationalRuleOffset),
			     lpCountryEntry->dwInternationalRuleSize > 32 ? 32 : lpCountryEntry->dwInternationalRuleSize);
                    location->_tapi_intl_rule[31] = 0;
		}
		else {
		    location->_tapi_country_name[0] = '\0';
		    location->_tapi_same_area_rule[0] = '\0';
		    location->_tapi_long_dist_rule[0] = '\0';
		    location->_tapi_intl_rule[0] = '\0';
		}

		strncpy(location->_tapi_area_code,
			 (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwCityCodeOffset),
			 lpLocationEntry[i].dwCityCodeSize > 32 ? 32 : lpLocationEntry[i].dwCityCodeSize);
                location->_tapi_area_code[31] = 0;

		location->_tapi_preferred_card_id = lpLocationEntry[i].dwPreferredCardID;
		location->_tapi_options = lpLocationEntry[i].dwOptions;

		strncpy(location->_tapi_local_access_code, 
			 (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwLocalAccessCodeOffset),
			 lpLocationEntry[i].dwLocalAccessCodeSize > 32 ? 32 : lpLocationEntry[i].dwLocalAccessCodeSize);
                location->_tapi_local_access_code[31] = 0;

		strncpy(location->_tapi_long_dist_code, 
		     (((LPSTR) lpTranslateCaps) +
		       lpLocationEntry[i].dwLongDistanceAccessCodeOffset),
		     lpLocationEntry[i].dwLongDistanceAccessCodeSize > 32 ? 32 : lpLocationEntry[i].dwLongDistanceAccessCodeSize);
                location->_tapi_long_dist_code[31] = 0;

                strncpy(location->_tapi_toll_prefix_list, 
                         (((LPSTR) lpTranslateCaps) +
                           lpLocationEntry[i].dwTollPrefixListOffset),
                         lpLocationEntry[i].dwTollPrefixListSize > 4096 ? 4096 : lpLocationEntry[i].dwTollPrefixListSize);
                location->_tapi_toll_prefix_list[4095] = 0;

		strncpy(location->_tapi_cancel_call_waiting, 
			 (((LPSTR) lpTranslateCaps) +
			   lpLocationEntry[i].dwCancelCallWaitingOffset),
			 lpLocationEntry[i].dwCancelCallWaitingSize > 32 ? 32 : lpLocationEntry[i].dwCancelCallWaitingSize);
                location->_tapi_cancel_call_waiting[31] = 0;

		/* We have copied the TAPI strings to the K_LOCATION */
		/* object.  Now we have to build the new object data */
		/* from it.                                          */

		itoa( location->_tapi_country_code, location->_country_code, 10 );
		_location_list + location;	/* only do this if it is new */

		strncpy( location->_area_code, location->_tapi_area_code, 32 );
                location->_area_code[31] = 0;

		/* The true Long Distance Prefix is the tapiLongDistAccessCode */
		/* plus the prefix portion of the tapiLongDistanceRule         */
		strncpy( location->_long_dist_prefix, location->_tapi_long_dist_code, 32 );
		strncat( location->_long_dist_prefix, location->_tapi_long_dist_rule, 32 );
                location->_long_dist_prefix[31] = 0;

		ZIL_ICHAR * p = location->_long_dist_prefix;
		/* find the end of the prefix and null terminate it */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) 
		    p++;
		*p = '\0';

		/* The Long Distance Suffix is the suffix portion of the       */
		/* tapiLongDistanceRule (if any) 				   */
		p = location->_tapi_long_dist_rule;
		/* skip the prefix */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) {
		    p++;
		}
		/* and then the dialing rule */
		while ( *p == 'E' || *p == 'F' || *p == 'G' || *p == 'N' || *p == 'S') {
		    p++;
		}
		strncpy( location->_long_dist_suffix, p, 33);
                location->_long_dist_suffix[32] = 0;

		/* The International Prefix is the prefix portion of the       */
		/* tapiInternationalRule         */
		/* The true International Prefix is the tapiLongDistAccessCode */
		/* plus the prefix portion of the tapiInternationalRule        */
		strncpy( location->_intl_prefix, location->_tapi_long_dist_code, 33);
		strncat( location->_intl_prefix, location->_tapi_intl_rule, 33 );
                location->_intl_prefix[32] = 0;
		p = location->_intl_prefix;
		/* find the end of the prefix and null terminate it */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) 
		    p++;
		*p = '\0';

		/* The International Suffix is the suffix portion of the       */
		/* tapiInternationalRule (if any) 				   */
		p = location->_tapi_intl_rule;
		/* skip the prefix */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) {
		    p++;
		}
		/* and then the dialing rule */
		while ( *p == 'E' || *p == 'F' || *p == 'G' || *p == 'N' || *p == 'S') {
		    p++;
		}
		strncpy( location->_intl_suffix, p, 33 );
                location->_intl_suffix[32] = 0;

		/* The true Local Prefix is the tapiLocalAccessCode */
		/* plus the prefix portion of the tapiSameAreaRule         */
		strncpy( location->_local_area_prefix, location->_tapi_local_access_code, 33 );
		strncat( location->_local_area_prefix, location->_tapi_same_area_rule, 33 );
                location->_local_area_prefix[32] = 0;
		p = location->_local_area_prefix;
		/* find the end of the prefix and null terminate it */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) 
		    p++;
		*p = '\0';

		/* The Local Access Suffix is the suffix portion of the       */
		/* tapiLocalAccessRule (if any) 				   */
		p = location->_tapi_same_area_rule;
		/* skip the prefix */
		while ( *p && *p != 'E' && *p != 'F' && *p != 'G' && *p != 'N' && *p != 'S' ) {
		    p++;
		}
		/* and then the dialing rule */
		while ( *p == 'E' || *p == 'F' || *p == 'G' || *p == 'N' || *p == 'S') {
		    p++;
		}
		strncpy( location->_local_area_suffix, p, 33 );
                location->_local_area_suffix[32] = 0;

		strncpy( location->_dial_prefix, location->_tapi_cancel_call_waiting, 33 );
                location->_dial_prefix[32] = 0;

		if ( location->_tapi_options & LINELOCATIONOPTION_PULSEDIAL )
		    location->_dial_method = Pulse ;
		else
		    location->_dial_method = Tone ;

                /* Handle special cases */
                switch ( lpLocationEntry[i].dwCountryCode ) {
                case 1:         /* US and other NANP countries */
                    if ( !location->_dial_tf_prefix[0] )
                        strcpy( location->_dial_tf_prefix, "1" );
                    if ( !location->_dial_tf_area[0] )
                        strcpy( location->_dial_tf_area, "800 888 877 866" );
                    break;
                case 33:        /* France */
                    location->_force_long_dist = 1;
                    break;
                }
	    }

	    if ( lpTranslateCaps->dwCurrentLocationID ==
		 location->_tapi_location_id ) {
		current = location;
	    }

            if ( oldloc )
                delete oldloc;

            if (_userFile->ChDir("~Locations") == 0) {
                ZIL_STORAGE_OBJECT * _fileObj = 
                    new ZIL_STORAGE_OBJECT( *_userFile, location->_name, ID_K_LOCATION ) ;
                if ( !_fileObj )
                    OutofMemory("Unable create Locations STORAGE_OBJECT");
                location->Store( location->_name, _userFile, _fileObj,
                                 ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
                delete _fileObj;
                _userFile->ChDir("~");
            }
            free(newstr);
            newstr = NULL;
	}
    }
    if ( lpCountryList )
        LocalFree(lpCountryList);
    if ( lpTranslateCaps )
        LocalFree(lpTranslateCaps);

    if ( current )
	_location_list.SetCurrent(current);
}

EVENT_TYPE 
KTapiLineConfigMenuUser( UI_WINDOW_OBJECT * obj, UI_EVENT & event,
		   EVENT_TYPE ccode )
{
    UIW_POP_UP_ITEM * item = (UIW_POP_UP_ITEM *) obj ;

    switch ( ccode )
    {
    case L_SELECT: {
	/* a new  has been selected */
	cktapiConfigureLine( item->value );
    }
	break;
    }
    return 0;
}
#endif /* NODIAL */

void
K_CONNECTOR::UpdateTapiLineConfigMenuItems( void )
{
    UIW_POP_UP_ITEM * popup = (UIW_POP_UP_ITEM *) Get( MENU_TAPI_LINE_CONFIG );
    UIW_POP_UP_MENU * menu = &(popup->menu);
    UIW_POP_UP_ITEM * current = NULL;

    for ( popup = (UIW_POP_UP_ITEM *) menu->First();
	  popup; 
	  popup = (UIW_POP_UP_ITEM *) menu->First()) {
	*menu - popup;
	delete popup;
    }

#ifdef NODIAL
    return;
#else
    if ( !TapiAvail )
        return;

    cktapiBuildLineTable( &tapilinetab, &_tapilinetab, &ntapiline );
    for ( int i=0 ; i<ntapiline ; i++ ) { 
	popup = new UIW_POP_UP_ITEM( tapilinetab[i].kwd, MNIF_CHECK_MARK,
				    BTF_NO_TOGGLE, WOF_NO_FLAGS,
				    KTapiLineConfigMenuUser,tapilinetab[i].kwval);
        if ( !popup )
            OutofMemory("Unable to create POP_UP_ITEM for TAPI LINES");
	if ( popup )
	    *menu + popup;
    }
#endif /* NODIAL */
}
#endif



void
K_CONNECTOR::LoadLocations( void ) 
{
    UI_LIST * list = &_location_list ; 
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    K_LOCATION * item = NULL ;
    ZIL_ICHAR * locationName = NULL ;
    ZIL_ICHAR currentName[61] = "";

    K_LOCATION * current = (K_LOCATION *) list->Current() ;
    if ( current )
	strncpy( currentName, current->_name, 60 );
    else
	strcpy( currentName, _config->_current_location);
    
   list->Destroy() ;

   list->compareFunction = K_LOCATION::Compare ;
   
    for (locationName = _userFile->FindFirstID(ID_K_LOCATION); 
	  locationName; locationName = _userFile->FindFirstID(ID_K_LOCATION))
    {
	_userFile->DestroyObject( locationName );
    }
    
    if (_userFile->ChDir("~Locations") == 0) {
        int location_cnt = 0;
        for (locationName = _userFile->FindFirstID(ID_K_LOCATION); 
	  locationName; locationName = _userFile->FindNextID())
        {
            location_cnt++;

            ZIL_STORAGE_OBJECT * _fileObj = 
                new ZIL_STORAGE_OBJECT( *_userFile, locationName, ID_K_LOCATION ) ;
            if ( !_fileObj )
                OutofMemory("Unable to create Locations STORAGE_OBJECT while loading");
            item = new K_LOCATION( locationName, _userFile, _fileObj ) ;
            if ( !item )
                OutofMemory("Unable to create LOCATION while loading");
            delete _fileObj ;
            _fileObj = NULL ;

            if ( strncmp(locationName,item->_name, 12) ) {
                /* Something has gone seriously wrong !!!!!!!!  */
                /* The item that was loaded is corrupted        */
                /* the file is probably bad.  Or it will become */
                /* that way if we continue.                     */
                ZAF_MESSAGE_WINDOW * 
                    MessageBox =
                        new ZAF_MESSAGE_WINDOW( "Error",
					    "EXCLAMATION", 
					    ZIL_MSG_NO | ZIL_MSG_YES, ZIL_MSG_NO,
					    "Location \"%s\" is corrupted in DIALUSR.DAT.  Delete entry?",
					    locationName) ;
                if ( !MessageBox )
                    OutofMemory("Unable to create MESSAGE_WINDOW while loading locations");
                EVENT_TYPE msg = MessageBox->Control() ;
                delete MessageBox ;
                switch ( msg ) {
                case ZIL_DLG_YES:
                    // Delete the bad entry
                    if ( _userFile->DestroyObject( locationName ) < 0 )
                    {
                        // What went wrong?
                        MessageBox =
                            new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                                    "Unable to remove location \"%s\": errorno %d", 
                                                    locationName,
                                                    _userFile->storageError ) ;
                        if ( !MessageBox )
                            OutofMemory("Unable to create MESSAGE_WINDOW while loading locations 2");
                        MessageBox->Control();
                        delete MessageBox ;
                    }
                    else {
                        /* we need to start the load process over again */
                        _userFile->ChDir("~");
                        LoadLocations();
                        return;
                    }
                    break;
                case ZIL_DLG_NO:
                default:
                    ; // do nothing
                }
                continue;
            }		
            *list + item ;
        }   
        K_CONNECTOR::_userFile->ChDir("~");

        // Apparently garbage has been written to some of the Locations Directory
        // entries on some systems.  If we can't find any locations its probably 
        // because the directory entry is damaged.  So let's destory it and 
        // recreate it.

        if ( location_cnt == 0 ) {
            _userFile->RmDir( "Locations" );
            _userFile->MkDir("Locations");
        }
    }
    /* I'm not sure if UI_LIST has a notion of Current */
    K_LOCATION * firstitem = (K_LOCATION *) list->First() ;
    if ( !currentName[0] && firstitem )
	list->SetCurrent( firstitem );  /* make first item current */
    else
    {
	K_LOCATION * listitem = (K_LOCATION *) list->First() ;
	K_LOCATION * lastlistitem = (K_LOCATION *) list->Last() ;
	if ( listitem )
	    do { 
		if ( !strcmp( currentName, listitem->_name ) )
		{
		    list->SetCurrent(listitem);
		    break;
		}
		if (listitem == lastlistitem )
		    break ;
		else
		    listitem = (K_LOCATION *) listitem->Next() ;
	    } while ( listitem );
    }
}

void
K_CONNECTOR::LoadModems( void ) 
{
    UI_LIST * list = &_modem_list ; 
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    K_MODEM * item = NULL ;
    ZIL_ICHAR * modemName = NULL ;

    list->Destroy() ;

    list->compareFunction = K_MODEM::Compare ;

    /* This code is only meant to clean up after some beta builds that */
    /* tried to store the modems in the root directory of the DAT file */
    for (modemName = _userFile->FindFirstID(ID_K_MODEM); 
         modemName; modemName = _userFile->FindFirstID(ID_K_MODEM))
    {
        _userFile->DestroyObject(modemName);
    }

    if (_userFile->ChDir("~Modems") == 0) {
        int modem_cnt = 0;
    for (modemName = _userFile->FindFirstID(ID_K_MODEM); 
         modemName; modemName = _userFile->FindNextID())
    {
        modem_cnt++;
        ZIL_STORAGE_OBJECT * _fileObj = 
            new ZIL_STORAGE_OBJECT( *_userFile, modemName, ID_K_MODEM ) ;
        if ( !_fileObj )
            OutofMemory("Unable to create Modems STORAGE_OBJECT while loading");
        item = new K_MODEM( modemName, _userFile, _fileObj ) ;
        if ( !item )
            OutofMemory("Unable to create MODEM while loading");
        delete _fileObj ;
        _fileObj = NULL ;

	if ( strncmp(modemName,item->_name, 12) ) {
	    /* Something has gone seriously wrong !!!!!!!!  */
	    /* The item that was loaded is corrupted        */
	    /* the file is probably bad.  Or it will become */
	    /* that way if we continue.                     */
	    ZAF_MESSAGE_WINDOW * 
		MessageBox =
		    new ZAF_MESSAGE_WINDOW( "Error",
					    "EXCLAMATION", 
					    ZIL_MSG_NO | ZIL_MSG_YES, ZIL_MSG_NO,
					    "Modem \"%s\" is corrupted in DIALUSR.DAT.  Delete entry?",
					    modemName) ;
            if ( !MessageBox )
                OutofMemory("Unable to create MESSAGE_WINDOW while loading modems");
	    EVENT_TYPE msg = MessageBox->Control() ;
	    delete MessageBox ;
	    switch ( msg ) {
	    case ZIL_DLG_YES:
		// Delete the bad entry
		if ( _userFile->DestroyObject( modemName ) < 0 )
		{
		    // What went wrong?
		    MessageBox =
			new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
						"Unable to remove modem \"%s\": errorno %d", 
						modemName,
						_userFile->storageError ) ;
                    if ( !MessageBox )
                        OutofMemory("Unable to create MESSAGE_WINDOW while loading modems 2");
		    MessageBox->Control();
		    delete MessageBox ;
		}
		else {
		    /* we need to start the load process over again */
		    _userFile->ChDir("~");
		    LoadModems();
		    return;
		}
		break;
	    case ZIL_DLG_NO:
	    default:
		; // do nothing
	    }
	    continue;
	}    
        if ( !FindModem( item->_name ) )
            *list + item ;
	else
	    delete item;
    }
        _userFile->ChDir("~");
        // Could be garbage

        if ( modem_cnt == 0 ) {
            _userFile->RmDir( "Modems" );
            _userFile->MkDir("Modems");
        }
    }
}

void
K_CONNECTOR::LoadEntries( void ) 
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    UI_EVENT ReDisplay(S_REDISPLAY) ;
    KD_LIST_ITEM * item = NULL ;
    ZIL_ICHAR * entryName = NULL ;
    ZIL_ICHAR currentName[29] = "";

    if ( !list )
        return;

    KD_LIST_ITEM * current = (KD_LIST_ITEM *) list->Current() ;
    if ( current )
        strncpy( currentName, current->_name, 28 );

   list->Information( I_DESTROY_LIST, NULL ) ;
   _entry_list.Destroy() ;

   _entry_list.compareFunction = KD_LIST_ITEM::Compare ;
   list->compareFunction = KD_LIST_ITEM::Compare ;
   
    for (entryName = _userFile->FindFirstID(ID_KD_LIST_ITEM); 
	  entryName; entryName = _userFile->FindNextID())
    {
	ZIL_STORAGE_OBJECT * _fileObj = 
	    new ZIL_STORAGE_OBJECT( *_userFile, entryName, ID_KD_LIST_ITEM ) ;
        if ( !_fileObj )
            OutofMemory("Unable to create STORAGE_OBJECT while loading entries");
	item = new KD_LIST_ITEM( entryName, _userFile, _fileObj ) ;
        if ( !item )
            OutofMemory("Unable to create LIST_ITEM while loading entries");
	if ( item->_converted_1_11 ) {	/* Modem Information Updated */
            _fileObj->Seek(0);
	    item->Store( item->_name, _userFile, _fileObj,
			 ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
	    _userFile->Save(2) ;
	    if ( _userFile->storageError )
	    {
		// What went wrong?
		ZAF_MESSAGE_WINDOW * message =
		    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					    ZIL_MSG_OK, ZIL_MSG_OK,
					    "Unable to Open %s: errorno %d", 
					    item->_name,
					    _userFile->storageError ) ;
                if ( !message )
                    OutofMemory("Unable to create MESSAGE_WINDOW while loading entries");
		message->Control() ;
		delete message ;
	    }
	}
	delete _fileObj ;
	_fileObj = NULL ;

	if ( strncmp(entryName,item->_name, 12) ) {
	    /* Something has gone seriously wrong !!!!!!!!  */
	    /* The item that was loaded is corrupted        */
	    /* the file is probably bad.  Or it will become */
	    /* that way if we continue.                     */
	    ZAF_MESSAGE_WINDOW * 
		MessageBox =
		    new ZAF_MESSAGE_WINDOW( "Error",
					    "EXCLAMATION", 
					    ZIL_MSG_NO | ZIL_MSG_YES, ZIL_MSG_NO,
					    "Entry %s is corrupted in DIALUSR.DAT.  Delete entry?",
					    entryName) ;
            if ( !MessageBox )
                OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 2");
	    EVENT_TYPE msg = MessageBox->Control() ;
	    delete MessageBox ;
	    switch ( msg ) {
	    case ZIL_DLG_YES:
		// Delete the bad entry
		if ( _userFile->DestroyObject( entryName ) < 0 )
		{
		    // What went wrong?
		    MessageBox =
			new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
						"Unable to remove %s: errorno %d", 
						entryName,
						_userFile->storageError ) ;
                    if ( !MessageBox )
                        OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 3");
		    MessageBox->Control();
		    delete MessageBox ;
		}
		else {
		    /* we need to start the load process over again */
		    LoadEntries();
		    return;
		}
		break;
	    case ZIL_DLG_NO:
	    default:
		; // do nothing
	    }
	    continue;
	}		

	item->_userdefined = TRUE ;

	if ( _config->_load_userdefined && 
	     (( _config->_load_templates && item->_template ) ||
	     (!item->_template &&
	       ( _config->_load_dialup && item->_access == PHONE ||
		 _config->_load_direct && item->_access == DIRECT ||
		 _config->_load_network && 
		 (item->_access == TCPIP || item->_access == SUPERLAT || 
                  item->_access == SSH) ||
                 _config->_load_ftp && _config->_load_ftp))))
            *list + item ;
	else
            _entry_list + item ;
    }

    entryName = _preloadFile ? _preloadFile->FindFirstID(ID_KD_LIST_ITEM) : NULL;
    for (; entryName; entryName = _preloadFile->FindNextID())
    {
	int duplicate = 0 ;
	ZIL_STORAGE_OBJECT_READ_ONLY _fileObj( *_preloadFile, entryName, ID_KD_LIST_ITEM ) ;
	item = new KD_LIST_ITEM( entryName, _preloadFile, &_fileObj ) ;
        if ( !item )
            OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 4");

	if ( strncmp(entryName,item->_name, 12) ) {
	    /* Something has gone seriously wrong !!!!!!!!  */
	    /* The item that was loaded is corrupted        */
	    /* the file is probably bad.  Or it will become */
	    /* that way if we continue.                     */
	    ZAF_MESSAGE_WINDOW * 
		MessageBox = new ZAF_MESSAGE_WINDOW( "Error", 
						     "EXCLAMATION", 
						     ZIL_MSG_OK, ZIL_MSG_OK,
						     "Entry %s is corrupted in DIALINF.DAT",
						     entryName) ;
            if ( !MessageBox )
                OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 4");
	    MessageBox->Control() ;
	    delete MessageBox ;
	    exit(10);
	}		

	item->_userdefined = FALSE ;

	KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
	if ( listitem )
	{
            if ( !KD_LIST_ITEM::Compare( item, listitem ) )
		duplicate = 1 ;
            else 
            {
		KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
		do { 
		    listitem = listitem->Next() ;
		    duplicate = !KD_LIST_ITEM::Compare( item, listitem ) ;
		} while ( !duplicate && listitem && listitem != lastlistitem );
            }
	}
	if ( !duplicate ) 
	{
	    listitem = (KD_LIST_ITEM *) _entry_list.First() ;
	    if ( listitem )
	    {
		if ( !KD_LIST_ITEM::Compare( item, listitem ) )
		    duplicate = 1 ;
		else 
		{
		    KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) _entry_list.Last() ;
		    do { 
			listitem = listitem->Next() ;
			duplicate = !KD_LIST_ITEM::Compare( item, listitem ) ;
		    } while ( !duplicate && listitem && listitem != lastlistitem );
		}
	    }
	}
	if ( !duplicate )
	{
            if ( _config->_load_predefined && 
		 (( _config->_load_templates && item->_template ) ||
		   (!item->_template && 
		     ( _config->_load_dialup && item->_access == PHONE ||
		       _config->_load_direct && item->_access == DIRECT ||
		       _config->_load_network && (item->_access == TCPIP ||
						   item->_access == SUPERLAT ||
                                                   item->_access == SSH ||
                                                   item->_access == FTP)))))
		*list + item ;
            else
		_entry_list + item ;
	}
    }

    entryName = _orgloadFile ? _orgloadFile->FindFirstID(ID_KD_LIST_ITEM) : NULL;
    for (; entryName; entryName = _orgloadFile->FindNextID())
    {
	int duplicate = 0 ;
	ZIL_STORAGE_OBJECT_READ_ONLY _fileObj( *_orgloadFile, entryName, ID_KD_LIST_ITEM ) ;
	item = new KD_LIST_ITEM( entryName, _orgloadFile, &_fileObj ) ;
        if ( !item )
            OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 5");

	if ( strncmp(entryName,item->_name, 12) ) {
	    /* Something has gone seriously wrong !!!!!!!!  */
	    /* The item that was loaded is corrupted        */
	    /* the file is probably bad.  Or it will become */
	    /* that way if we continue.                     */
	    ZAF_MESSAGE_WINDOW * 
		MessageBox = new ZAF_MESSAGE_WINDOW( "Error", 
						     "EXCLAMATION", 
						     ZIL_MSG_OK, ZIL_MSG_OK,
						     "Entry %s is corrupted in DIALINF.DAT",
						     entryName) ;
            if ( !MessageBox )
                OutofMemory("Unable to create MESSAGE_WINDOW while loading entries 6");
	    MessageBox->Control() ;
	    delete MessageBox ;
	    exit(10);
	}		

	item->_userdefined = FALSE ;

	KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
	if ( listitem )
	{
            if ( !KD_LIST_ITEM::Compare( item, listitem ) )
		duplicate = 1 ;
            else 
            {
		KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
		do { 
		    listitem = listitem->Next() ;
		    duplicate = !KD_LIST_ITEM::Compare( item, listitem ) ;
		} while ( !duplicate && listitem && listitem != lastlistitem );
            }
	}
	if ( !duplicate ) 
	{
	    listitem = (KD_LIST_ITEM *) _entry_list.First() ;
	    if ( listitem )
	    {
		if ( !KD_LIST_ITEM::Compare( item, listitem ) )
		    duplicate = 1 ;
		else 
		{
		    KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) _entry_list.Last() ;
		    do { 
			listitem = listitem->Next() ;
			duplicate = !KD_LIST_ITEM::Compare( item, listitem ) ;
		    } while ( !duplicate && listitem && listitem != lastlistitem );
		}
	    }
	}
	if ( !duplicate )
	{
            if ( _config->_load_predefined && 
		 (( _config->_load_templates && item->_template ) ||
		   (!item->_template && 
		     ( _config->_load_dialup && item->_access == PHONE ||
		       _config->_load_direct && item->_access == DIRECT ||
		       _config->_load_network && (item->_access == TCPIP ||
						   item->_access == SUPERLAT ||
                                                   item->_access == SSH ||
                                                   item->_access == FTP)))))
		*list + item ;
            else
		_entry_list + item ;
	}
    }

    KD_LIST_ITEM * firstitem = (KD_LIST_ITEM *) list->First() ;
    if ( !currentName[0] && firstitem )
	*list + firstitem ;  /* make first item current */
    else
    {
	KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
	KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
	if ( listitem )
	    do { 
		if ( !strcmp( currentName, listitem->_name ) )
		{
		    *list + listitem ;
		    break;
		}
		if (listitem == lastlistitem )
		    break ;
		else
		    listitem = listitem->Next() ;
	    } while ( listitem );
    }
    list->Event( ReDisplay ) ;
}

void
K_CONNECTOR::
LoadConfig( void )
{

    ZIL_ICHAR *ConfigName = _userFile->FindFirstID(ID_KD_CONFIG);	/* Old Location */
    if ( ConfigName )
    {
	ZIL_STORAGE_OBJECT_READ_ONLY * fileObj =
	    new ZIL_STORAGE_OBJECT_READ_ONLY( *_userFile, ConfigName, ID_KD_CONFIG ) ;
        if ( !fileObj )
            OutofMemory("Unable to create STORAGE_OBJECT_READ_ONLY while loading config");
	_config = new KD_CONFIG( ConfigName, _userFile, fileObj ) ;
        if ( !_config )
            OutofMemory("Unable to create KD_CONFIG while loading config");
	delete fileObj;

	if (_userFile->ChDir("~Config") == 0) {
            ZIL_STORAGE_OBJECT * newFileObj =
                new ZIL_STORAGE_OBJECT( *_userFile, ConfigName, ID_KD_CONFIG ) ;
            if ( !newFileObj )
                OutofMemory("Unable to create STORAGE_OBJECT while loading config");
            _config->Store( ConfigName, _userFile, newFileObj,
                            ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
            if ( !_userFile->storageError ) {
                _userFile->ChDir("~");
                _userFile->DestroyObject(ConfigName);
            }
            else
                _userFile->ChDir("~");
            delete newFileObj;
        }
    }
    else
    {
	if (_userFile->ChDir("~Config") == 0) {
            ConfigName = _userFile->FindFirstID(ID_KD_CONFIG);	/* New Location */
            if ( ConfigName )
            {
                ZIL_STORAGE_OBJECT_READ_ONLY _fileObj( *_userFile, ConfigName, ID_KD_CONFIG ) ;
                _config = new KD_CONFIG( ConfigName, _userFile, &_fileObj ) ;
                _userFile->ChDir("~");
            }
            else
            {
                _userFile->ChDir("~");
                // Config directory could be garbage
                _userFile->RmDir("Config");
                _userFile->MkDir("Config");
                _config = new KD_CONFIG() ;
            }
            if ( !_config )
                OutofMemory("Unable to create KD_CONFIG while loading config 2");
        }
    }

    UIW_POP_UP_ITEM * popup ;
    if ( _config->_load_templates ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_TEMPLATES ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_network ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_NETWORK ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_ftp ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_FTP ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_dialup ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_DIAL ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_direct ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_DIRECT ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_predefined ) 
    {	
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_PREDEFINED ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_load_userdefined ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_VIEW_USERDEFINED ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_confirm_number ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_CONFIRM ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
#ifdef COMMENT
    if ( _config->_convert_dir ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_CONVERT ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
#endif /* COMMENT */
    if ( _config->_display_dial ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_DISPLAY ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_hangup ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_HANGUP ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_sort_dir ) 
    {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_SORT ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }

    if ( _config->_left > -1 && _config->_left < 1600 &&
	     (_config->_right - _config->_left) > 125 &&
		 (_config->_bottom - _config->_top) > 32) 
    {
    	relative.left = _config->_left ;
	relative.right = _config->_right ;
	relative.top = _config->_top ;
	relative.bottom = _config->_bottom ;
	Information( I_CHANGED_FLAGS, NULL );
    }

    if ( _config->_min_on_use ) {
	popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_MIN_ON_USE ) ;
	popup->woStatus |= WOS_SELECTED ;
	popup->Information(I_CHANGED_STATUS, NULL);
    }

    if ( TapiAvail ) {
        if ( _config->_tapi_conversions == ConvWindows ) {
            popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_TAPI_CONV ) ;
            popup->woStatus |= WOS_SELECTED ;
            popup->Information(I_CHANGED_STATUS, NULL);
        }
        if ( _config->_tapi_dialing == DialWindows ) {
            popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_TAPI_PASS ) ;
            popup->woStatus |= WOS_SELECTED ;
            popup->Information(I_CHANGED_STATUS, NULL);
        }
    }
    else {
        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_TAPI_CONV ) ;
        popup->woFlags |= WOF_NON_SELECTABLE ;
        popup->Information(I_CHANGED_FLAGS, NULL);

        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_DIAL_TAPI_PASS ) ;
        popup->woFlags |= WOF_NON_SELECTABLE ;
        popup->Information(I_CHANGED_FLAGS, NULL);

        popup = (UIW_POP_UP_ITEM *) Get( MENU_TAPI ) ;
        popup->woFlags |= WOF_NON_SELECTABLE ;
        popup->Information(I_CHANGED_FLAGS, NULL);
    }

    if ( _config->_shortcut_desktop ) {
        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_SHORTCUTS_DESKTOP ) ;
        popup->woStatus |= WOS_SELECTED ;
        popup->Information(I_CHANGED_STATUS, NULL);
    }
    if ( _config->_shortcut_startmenu ) {
        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_SHORTCUTS_STARTMEN ) ;
        popup->woStatus |= WOS_SELECTED ;
        popup->Information(I_CHANGED_STATUS, NULL);
    }

    if (!Desktop) {
        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_SHORTCUTS_DESKTOP ) ;
        popup->woFlags |= WOF_NON_SELECTABLE ;
        popup->Information(I_CHANGED_FLAGS, NULL);
    }

    if (!StartMenu) {
        popup = (UIW_POP_UP_ITEM *) Get( MENU_OPTIONS_SHORTCUTS_STARTMEN ) ;
        popup->woFlags |= WOF_NON_SELECTABLE ;
        popup->Information(I_CHANGED_FLAGS, NULL);
    }
}

void
K_CONNECTOR::SaveConfig( void ) 
{
    ZIL_ICHAR *ConfigName = "DialerConfig";

    _config->_left = relative.left ;
    _config->_top  = relative.top ;
    _config->_right = relative.right ;
    _config->_bottom = relative.bottom ;

    KD_LIST_ITEM * current = (KD_LIST_ITEM *) VtList->Current() ;
    if ( current ) {
	strncpy( _config->_current_entry, current->_name, 65 );
        _config->_current_entry[64] = '\0';
    } else
	_config->_current_entry[0] = '\0' ;

    K_LOCATION * location = (K_LOCATION *) _location_list.Current() ;
    if ( location ) {
	strncpy( _config->_current_location, location->_name, 60 );
        _config->_current_location[59] = '\0';
    } else
	_config->_current_location[0] = '\0' ;

    if (_userFile->ChDir("~Config") == 0) {
        ZIL_STORAGE_OBJECT _fileObj( *_userFile, ConfigName, ID_KD_CONFIG,
                                 UIS_OPENCREATE | UIS_READWRITE ) ;
        _config->Store( ConfigName, _userFile, &_fileObj,
                        ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
        if ( _userFile->storageError )
        {
            // What went wrong?
            ZAF_MESSAGE_WINDOW * message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to Write %s: errorno %d", ConfigName, 
                                   _userFile->storageError ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW while loading config");
            message->Control();
            delete message ;
        }   
        _userFile->ChDir("~");
    }
}   

EVENT_TYPE K_CONNECTOR::
Event( const UI_EVENT & event )
{
   EVENT_TYPE retval = event.type;
   ZIL_SCREENID frameID = NULL ;
   ZAF_MESSAGE_WINDOW * message = NULL ;
   K_STATUS * entry = NULL ;

    printf("K_CONNECTOR::Event()\n");

   switch ( event.type ) {
   case E_MSWINDOWS:
   case E_OS2:
       switch ( 
#ifdef WIN32
	       event.message.message 
#else
	       event.message.msg
#endif
	       )
       {
#ifdef WIN32
       case WM_DESTROY: {
           K_STATUS * status = K_STATUS::List;
           while ( status )
           {
               PostMessage( status->_hwnd, OPT_DIALER_EXIT, 0, 0 ) ;
               status = status->_next ;
           }
	   retval = UIW_WINDOW::Event( event ) ;
           break;
       }
#ifdef WM_CONTEXTMENU
       /* 2022-11-06 DavidG
        * For some reason the context menu broke sometime after Kermit 95
        * v2.1.2. The code I'm starting with *should* be what was used to build
        * the final commercial release of Kermit 95 but perhaps its not 100%
        * identical and a bug crept in?
        *
        * Or perhaps its an OpenZinc issue. The Dialer was previously built with
        * Zinc 4.2 - maybe OpenZinc 1.0 changed something which broke
        * WM_RBUTTONDOWN events being forwarded to list items? I diffed all the
        * OpenZinc 1.0 code against the Zinc 4.2 code previously used to build
        * the dialer and I don't see anything significant.
        *
        * So I have *no idea* why it stopped working. But fine, WM_RBUTTONDOWN
        * is the wrong event to be listening for to trigger a context menu on
        * 32bit windows anyway - WM_CONTEXTMENU is what we should be doing. So
        * thats why all this code here:
        */

/* These *should* come from the windows header but for whatever reason that
 * isn't guaranteed. */
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif
       case WM_CONTEXTMENU: {
           UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ;

           POINT p = {
               GET_X_LPARAM(event.message.lParam),
               GET_Y_LPARAM(event.message.lParam)
           };
           ScreenToClient(list->screenID, &p);
           UI_POSITION position = { p.x, p.y };

           for (KD_LIST_ITEM *item = (KD_LIST_ITEM*)list->First(); item; item = (KD_LIST_ITEM*)item->Next())
           {
               int idx = list->Index(item);

               // After a context menu is opened for the first time, the
               // trueRegion value gets corrupted for every list item preventing
               // item->trueRegion.Overlap(position) from working. I've spent
               // a bunch of time and I have absolutely no idea why it's broken
               // or what the correct solution is. But at the end of the day
               // there is a Win32 list box under the UIW_VT_LIST object (on
               // windows) so we'll just get the correct coordinates from
               // Windows and fix up Zincs numbers so that the context menu
               // works more than once:
               RECT rect;
               SendMessage(list->screenID, LB_GETITEMRECT, idx, (LPARAM)&rect);
               item->trueRegion.left = rect.left;
               item->trueRegion.top = rect.top;
               item->trueRegion.right = rect.right;
               item->trueRegion.bottom = rect.bottom;

               if (item->trueRegion.Overlap(position) &&
                    !FlagSet(item->woFlags, WOF_NON_SELECTABLE))
               {
                   // Before we show the context menu, the list item we're doing
                   // a context menu for must be selected. In Zinc, we do this:
                   list->SetCurrent(item);

                   // Problem is, that won't take effect until *after* the
                   // context menu is displayed and the user either selects an
                   // option or dismisses it. Its kind fo weird having the list
                   // item appear selected *after* the context menu, so we'll
                   // reach around and tell Windows to go ahead and do the
                   // select right now so it appears selected when the context
                   // menu appears:
                   SendMessage(list->screenID, LB_SETCURSEL, idx, NULL);

                   item->ShowContextMenu(GET_X_LPARAM(event.message.lParam),
                                           GET_Y_LPARAM(event.message.lParam));
                   retval = UIW_WINDOW::Event( event ) ;
                   break;
               }
           }
       }
#endif /* WM_CONTEXTMENU */
#endif /* WIN32 */
       case OPT_KERMIT_SUCCESS:
	   break;
       case OPT_KERMIT_FAILURE:
           break;
       case OPT_KERMIT_FG:
#ifdef WIN32
#ifndef CKT_NT31
           ShowWindowAsync(event.message.hwnd,SW_SHOWNORMAL);
#else
           ShowWindow(event.message.hwnd,SW_SHOWNORMAL);
#endif /* CKT_NT31 */
           SetForegroundWindow(event.message.hwnd);
#endif /* WIN32 */
           break;
       case OPT_KERMIT_PID:
#ifdef WIN32
           _dwNextConnectPid = (DWORD) event.message.lParam; 
#endif /* WIN32 */
           break;
       case OPT_KERMIT_CONNECT:
#ifdef WIN32
	     entry = K_STATUS::Find( event.message.wParam ) ;
#else
	     entry = K_STATUS::Find( SHORT1FROMMP(event.message.mp1) ) ;
#endif

            if ( entry )
            {
               if ( entry->_state != K_STATUS::CONNECTED )
               {
		   entry->_state = K_STATUS::CONNECTED ;
		   ZIL_UTIME NowTime ;  /* default constructor uses current time */
		   entry->_connect_time = NowTime ;
		   UpdateStatusLine(FALSE);
               }
            }
            break;
       case OPT_KERMIT_HANGUP :
#ifdef WIN32
	     entry = K_STATUS::Find( event.message.wParam ) ;
#else
	   entry = K_STATUS::Find( SHORT1FROMMP(event.message.mp1) ) ;
#endif
            if ( entry )
            {
               if ( entry->_state != K_STATUS::IDLE )
               {
		   ZIL_UTIME NowTime ;  /* default constructor uses current time */
		   entry->_state = K_STATUS::IDLE ;
		   entry->_disconnect_time = NowTime ;
		   UpdateStatusLine(FALSE);
               }
            }
            break;
       case OPT_KERMIT_EXIT:
	   /* if nothing else, remove status entry from list */
#ifdef WIN32
	   entry = K_STATUS::Find( event.message.wParam ) ;
#else /* then its OS2 */
	   entry = K_STATUS::Find( SHORT1FROMMP(event.message.mp1) ) ;
#endif /* WIN32 */
	   if ( entry )
	   {
#ifndef WIN32
	       if ( entry->_hwnd ) {
		   DosClose( entry->_hwnd ) ;
		   entry->_hwnd = 0L ;
	       }
	       if ( entry->_state != K_STATUS::IDLE ) {
		   ZIL_UTIME NowTime ;  /* default constructor uses current time */
		   entry->_state = K_STATUS::IDLE;
		   entry->_disconnect_time = NowTime ;
	       }
#endif /* !WIN32 */
	       K_STATUS::Remove( entry->_kermit_id ) ;
	       UpdateStatusLine(FALSE);
	   }
	   break;

       case OPT_KERMIT_HWND2:
#ifdef WIN32
           _hwndNextConnect = (HWND) event.message.lParam; 
#endif /* WIN32 */
           break;
       case OPT_KERMIT_HWND:
#ifdef WIN32 
	   entry = K_STATUS::Find( event.message.wParam ) ;
	   if ( entry )
	   {
	       entry->_hwnd = (HWND) event.message.lParam;
	       entry->_state = K_STATUS::IDLE ;
	   }
#ifdef COMMENT
           // this was removed for k95 2.1 to prevent auto-reuse of
           // an existing session.  now an HWND2 message will be sent
           // if reuse is desired.
           else {
               _hwndNextConnect = (HWND) event.message.lParam; 
           }
#endif /* COMMENT */
#else /* WIN32 */
	   entry = K_STATUS::Find( SHORT1FROMMP(event.message.mp1) ) ;
	   if ( entry )
	   {
	       /* Compute the name of the Named Pipe and open it */
	       ULONG Action, rc;
	       ZIL_ICHAR PipeName[256] ;
	       sprintf( PipeName, "\\PIPE\\K2DIAL\\%d",entry->_kermit_id );
	       rc = DosOpen( PipeName, &entry->_hwnd, &Action, 0, 0,
			     OPEN_ACTION_OPEN_IF_EXISTS,
			     OPEN_FLAGS_FAIL_ON_ERROR | 
			     OPEN_FLAGS_WRITE_THROUGH |
			     OPEN_FLAGS_RANDOM | OPEN_SHARE_DENYNONE | 
			     OPEN_ACCESS_WRITEONLY,
			     0 );
	       if ( rc ) {
		   ZAF_MESSAGE_WINDOW * message =
		       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					       ZIL_MSG_OK, ZIL_MSG_OK,
					       "Unable to Open to %s: %d", 
					       PipeName, rc ) ;
                   if ( !message )
                       OutofMemory("Unable to create MESSAGE_WINDOW event KERMIT_HWND 1");
		   message->Control() ;
		   delete message ;
	       }
	   }
	   else 
	   {
	       ZAF_MESSAGE_WINDOW * message =
		   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					   ZIL_MSG_OK, ZIL_MSG_OK,
					   "Unable to find entry: %d", 
					   SHORT1FROMMP(event.message.mp1) ) ;
               if ( !message )
                   OutofMemory("Unable to create MESSAGE_WINDOW event KERMIT_HWND 2");
	       message->Control() ;
	       delete message ;
	   }
#endif /* WIN32 */
	   break;
       default:
	   retval = UIW_WINDOW::Event( event ) ;
       }
       break;
   case E_TIMER:
	   UpdateStatusLine(FALSE) ;
       break;

   case OPT_ADD_ENTRY:
       retval = AddEntry( event ) ;
       break;

   case OPT_EDIT_ENTRY:
   case OPT_EDIT_GENERAL:
   case OPT_EDIT_TERMINAL:
   case OPT_EDIT_FILE_TRANSFER:
   case OPT_EDIT_SERIAL:
   case OPT_EDIT_TELNET:
   case OPT_EDIT_TCP:
   case OPT_EDIT_KERBEROS:
   case OPT_EDIT_TLS:
   case OPT_EDIT_KEYBOARD:
   case OPT_EDIT_LOGIN:
   case OPT_EDIT_PRINTER:
   case OPT_EDIT_LOGS:
   case OPT_EDIT_SSH:
   case OPT_EDIT_FTP:
   case OPT_EDIT_GUI:
       retval = EditEntry( event ) ;
       break;

   case OPT_CLONE_ENTRY:
       retval = CloneEntry( event ) ;
       break;

   case OPT_REMOVE_ENTRY:
       retval = RemoveEntry( event ) ;
       break;

   case OPT_CONNECT_TO_ENTRY:
       retval = Connect( event ) ;
       break;

   case OPT_SCRIPT_FILE:
       retval = ScriptFile( event ) ;
       break;

   case OPT_SHORTCUT_TO_ENTRY:
       retval = Shortcut( event ) ;
       break;

   case OPT_HANGUP_CONNECTION:
       retval = Disconnect( event ) ;
       break;

   case OPT_QUICK_CONNECT:
       retval = QuickConnect( event ) ;
       break;

   case OPT_MENU_OPTION_APPS:
       retval = SetApplications( event ) ;
       break;

   case OPT_SET_DIR_DIAL:
       retval = SetDirDial( event ) ;
       break;

   case OPT_SET_DIR_NETWORK:
       retval = SetDirNetwork( event ) ;
       break;

   case OPT_MENU_SCRIPT_MODEM:
       ExportModems();
       break;

   case OPT_MENU_SCRIPT_LOCATION:
       ExportLocations();
       break;

   case OPT_MENU_OPTIONS_DIAL:
       retval = SetDialOptions( event ) ;
       break;

   case OPT_MENU_MODEM:
       retval = SetModemOptions( event ) ;
       break;

   case OPT_MENU_HELP_ABOUT:
       retval = HelpAbout( event ) ;
       break;

   case OPT_MENU_HELP_MENU:
       helpSystem->DisplayHelp( windowManager, HELP_MENU ) ;
       break;

   case OPT_MENU_HELP_TOOLBAR:
       helpSystem->DisplayHelp( windowManager, HELP_TOOLBAR ) ;
       break;

   case OPT_DIAL_CONFIRM:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_CONFIRM)->woStatus, WOS_SELECTED) )
       {
	   _config->_confirm_number = TRUE ;
       }
       else
       {
	   _config->_confirm_number = FALSE ;
       }
       SaveConfig() ;
       break;
#ifdef COMMENT
   case OPT_DIAL_CONVERT:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_CONVERT)->woStatus, WOS_SELECTED) )
       {
	   _config->_convert_dir = TRUE ;
       }
       else
       {
	   _config->_convert_dir = FALSE ;
       }
       SaveConfig() ;
       break;
#endif /* COMMENT */
   case OPT_DIAL_DISPLAY:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_DISPLAY)->woStatus, WOS_SELECTED) )
       {
	   _config->_display_dial = TRUE ;
       }
       else
       {
	   _config->_display_dial = FALSE ;
       }
       SaveConfig() ;
       break;
   case OPT_DIAL_HANGUP:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_HANGUP)->woStatus, WOS_SELECTED) )
       {
	   _config->_hangup = TRUE ;
       }
       else
       {
	   _config->_hangup = FALSE ;
       }
       SaveConfig() ;
       break;
   case OPT_DIAL_SORT:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_SORT)->woStatus, WOS_SELECTED) )
       {
	   _config->_sort_dir = TRUE ;
       }
       else
       {
	   _config->_sort_dir = FALSE ;
       }
       SaveConfig() ;
       break;

   case OPT_VIEW_USERDEF:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_USERDEFINED)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_userdefined = TRUE ;
       }
       else
       {
	   _config->_load_userdefined = FALSE ;
       }
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_PREDEF:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_PREDEFINED)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_predefined = TRUE ;
       }	
       else
       {
	   _config->_load_predefined = FALSE ;
       }	
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_NETWORK:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_NETWORK)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_network = TRUE ;
       }
       else
       {
	   _config->_load_network = FALSE ;
       }	
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_FTP:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_FTP)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_ftp = TRUE ;
       }
       else
       {
	   _config->_load_ftp = FALSE ;
       }	
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_TEMPLATE:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_TEMPLATES)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_templates = TRUE ;
       }
       else
       {
	   _config->_load_templates = FALSE ;
       }
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_DIALUP:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_DIAL)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_dialup = TRUE ;
       }
       else
       {
	   _config->_load_dialup = FALSE ;
       }
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;
   case OPT_VIEW_DIRECT:
       if ( FlagSet(Get(MENU_OPTIONS_VIEW_DIRECT)->woStatus, WOS_SELECTED) )
       {
	   _config->_load_direct = TRUE ;
       }
       else
       {
	   _config->_load_direct = FALSE ;
       }
       SaveConfig() ;
       LoadEntries() ;
       UpdateStatusLine(TRUE);
       break;

   case OPT_MINIMIZE_ON_USE:
       if ( FlagSet(Get(MENU_OPTIONS_MIN_ON_USE)->woStatus, WOS_SELECTED) )
       {
	   _config->_min_on_use = TRUE ;
       }
       else
       {
	   _config->_min_on_use = FALSE ;
       }
       SaveConfig() ;
       break;

#if defined(WIN32)
#ifndef NODIAL
   case OPT_DIAL_TAPI_PASS:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_TAPI_PASS)->woStatus, WOS_SELECTED) )
       {
	   _config->_tapi_dialing = DialWindows ;
       }
       else
       {
	   _config->_tapi_dialing = DialKermit ;
       }
       SaveConfig() ;
       break;

   case OPT_DIAL_TAPI_CONV:
       if ( FlagSet(Get(MENU_OPTIONS_DIAL_TAPI_CONV)->woStatus, WOS_SELECTED) )
       {
	   _config->_tapi_conversions = ConvWindows ;
       }
       else
       {
	   _config->_tapi_conversions = ConvKermit ;
       }
       SaveConfig() ;
       break;

   case OPT_MENU_TAPI_DIALING:
       if ( TapiAvail )
	   cktapiDialingProp();
       break;
#endif /* NODIAL */
#endif

   case OPT_MENU_SHORTCUT_DESKTOP:
       if ( FlagSet(Get(MENU_OPTIONS_SHORTCUTS_DESKTOP)->woStatus, WOS_SELECTED) )
       {
	   _config->_shortcut_desktop = 1 ;
       }
       else
       {
	   _config->_shortcut_desktop = 0 ;
       }
       SaveConfig() ;
       break;

   case OPT_MENU_SHORTCUT_STARTMENU:
       if ( FlagSet(Get(MENU_OPTIONS_SHORTCUTS_STARTMEN)->woStatus, WOS_SELECTED) )
       {
	   _config->_shortcut_startmenu = 1 ;
       }
       else
       {
	   _config->_shortcut_startmenu = 0 ;
       }
       SaveConfig() ;
       break;

   case OPT_MENU_HELP_WEB_PROJECT:
       Browse("http://www.kermitproject.org/");
       break;
   case OPT_MENU_HELP_WEB_K95:
       Browse("http://www.kermitproject.org/ckw10beta.html");
       break;
   case OPT_MENU_HELP_WEB_NEWS:
       Browse("http://www.kermitproject.org/ckw10beta.html");
       break;
   case OPT_MENU_HELP_WEB_PATCH:
       Browse("http://www.kermitproject.org/ckw10beta.html");
       break;
   case OPT_MENU_HELP_WEB_TECHFAQ:
       Browse("http://www.kermitproject.org/k95faq.html");
       break;
   case OPT_MENU_HELP_MANUAL:
       ReadManual();
       break;
   case OPT_MENU_REGISTER_WEB:
       Browse("http://www.kermitproject.org/k95reg.html");
       break;
   case OPT_MENU_REGISTER_PRINT:
       break;
   case OPT_MENU_REGISTER_PHONE:
       break;
   case OPT_MENU_REGISTER_SERIAL:
       break;

   default:
       printf("fallthrough %d\n", event.type);
       retval = UIW_WINDOW::Event(event) ;
   }	
    return retval ; 
}

ZIL_UINT8 K_CONNECTOR::
IsManualInstalled(void)
{
    ZIL_ICHAR exepath[256];
    int i;
#ifdef WIN32
    struct _stat buf;
#else
    struct stat buf;
#endif

#ifdef WIN32
    GetModuleFileName( NULL, exepath, 256 ) ;
#else
    PTIB pptib;
    PPIB pppib;
    char *szPath;

    DosGetInfoBlocks(&pptib, &pppib);

    szPath = pppib -> pib_pchcmd;
    strncpy(exepath,szPath,256);
    exepath[255] = '\0';
#endif

    for ( i = strlen(exepath) ; i > 0 ; i-- ) {
        if ( exepath[i] == '\\' )
        {
            exepath[i+1] = '\0' ;
            break;
        }
    }

    /* Need to change directory to the DOCS\\MANUAL directory */
    /* before starting the manual.  Otherwise, Netscape may   */
    /* be unable to find the referential links.               */

#ifdef WIN32
    strcat(exepath, "DOCS\\MANUAL\\CKWIN.HTM");
#else
    strcat(exepath, "DOCS\\MANUAL\\CKOS2.HTM");
#endif

    if ( stat(exepath,&buf) >= 0 ) {
#ifdef COMMENT
        sprintf( manpath, "FILE://%s", exepath);
#else
        sprintf( manpath, "%s", exepath);
#endif
    }
    else 
        manpath[0] = '\0';

    return(manpath[0] != 0);
}

ZIL_UINT32 K_CONNECTOR::
ReadManual(void)
{
    if ( manpath[0] ) {
        int i;
        int drive;
        ZIL_ICHAR cwd[256], cwd2[256];

        printf("manpath = %s\n",manpath);

        drive = _getdrive();
        printf("drive = %c\n",'A'+drive-1);

        if ( drive != (tolower(manpath[0])-'a'+1) ) {
            _chdrive(tolower(manpath[0])-'a'+1);
            printf("new drive = %c\n",'A'+_getdrive()-1);
        }
        _getcwd( cwd, 255 );
        printf("cwd = %s\n",cwd);

        for ( i=strlen(manpath); i > 0 && manpath[i] != '\\'; i-- );

        if ( i != 0 ) {
            manpath[i] = '\0';
            _chdir(&manpath[2]);  /* skip the drive letter */
            printf("cd %s\n",&manpath[2]);
            manpath[i] = '\\';
            _getcwd( cwd2, 255 );
            printf("cwd2 = %s\n",cwd2);
        }

        Browse(manpath);
        printf("browse\n");

        _chdir(cwd);
        printf("cd %s\n",cwd);

        _chdrive(drive);
        printf("drive = %c\n",'A'+_getdrive()-1);
        
        return(1);
    }
    return(0);
}

void K_CONNECTOR::
ExecuteBrowser( void * _object )
{
    char * url = (char *) _object;
    system(url);
}

ZIL_UINT32 K_CONNECTOR::
Browse( ZIL_ICHAR * url )
{
    ZIL_ICHAR tmpbuf[256];
    ZIL_ICHAR cmdbuf[256];
    ZIL_ICHAR * s;

#ifdef WIN32
    if ( !_config->_app_www[0] ) {
        return(Win32ShellExecute( url ));
    }
#endif /* WIN32 */

    if ( _config->_app_www[0] ) {
        s = strstr("%s",(char *)_config->_app_www);	
        if ( s )
            sprintf(cmdbuf,_config->_app_www,url);
        else
            sprintf(cmdbuf,"%s %s",_config->_app_www,url);
#ifdef WIN32
        return(system(cmdbuf));
#endif
    }
    else {
        if (browsopts[0]) {
            s = strstr("%1",(char *)browsopts);
            if (s)
                *s = 's';
            else
                s = strstr("%s",(char *)browsopts);	
        }
        if (s)
            sprintf(tmpbuf,browsopts,url);
        else
            sprintf(tmpbuf,"%s %s",browsopts,url);
        sprintf(cmdbuf,"%s %s",browser,tmpbuf);
    }
#ifdef WIN32
    return (_beginthread(ExecuteBrowser, 65535, (void *)cmdbuf) != -1);
#else
    return (_beginthread(ExecuteBrowser, 0, 65535, (void *)cmdbuf) != -1);
#endif
}


EVENT_TYPE K_CONNECTOR::
Connect( const UI_EVENT & event )
{
    /* Get current entry from List */
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
    Connect( entry ) ;
    return  event.type;
}
    
void 
K_CONNECTOR::
Connect( KD_LIST_ITEM * entry )
{
   ZAF_MESSAGE_WINDOW * message = NULL ;
   KD_LIST_ITEM * def_template = entry ;

   if ( entry )
   {
       if ( entry->_access == PHONE ) {
           K_MODEM * modem = FindModem( entry->_modem );
           if ( !modem ) {
               message =
                   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                      "\"%s\" references a modem \"%s\" that does not exist.", 
                                      entry->_name,
                                      entry->_modem ) ;
               if ( !message )
                   OutofMemory("Unable to create MESSAGE_WINDOW Connect 1");
               message->Control() ;
               delete message ;
               return;
           }

           if ( !TapiAvail && modem->_is_tapi ) {
               message =
                   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                           "\"%s\" references a TAPI modem \"%s\".", 
                                      entry->_name,
                                      entry->_modem ) ;
               if ( !message )
                   OutofMemory("Unable to create MESSAGE_WINDOW Connect 2");
               message->Control() ;
               delete message ;
               return;
           }
       }
       def_template = FindEntry( entry->_line_device ) ;
       ZIL_UINT32 process_id = StartKermit( entry, _config, def_template ) ; 
       if ( !process_id )
       {
           if ( StartKermitErrorCode == -1005 ) {
               message =
                   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                           "Cancelled by User", 
                                           entry->_name) ;
               if ( !message )
                   OutofMemory("Unable to create MESSAGE_WINDOW Connect 3");
               message->Control() ;
               delete message ;
           }
           else if (  StartKermitErrorCode )
           {
               message =
                   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                           "Unable to start K95: session: %s errorno %d", 
                                           entry->_name,
                                           StartKermitErrorCode ) ;
               if ( !message )
                   OutofMemory("Unable to create MESSAGE_WINDOW Connect 4");
               message->Control() ;
               delete message ;
           }
       }
       else 
       {
           /* Entry added to list in the constructor */
           new K_STATUS( entry->_name, process_id, &K_STATUS::List ) ;
           if ( _config->_min_on_use ) {
               UI_EVENT event(S_MINIMIZE) ;
               Event( event ) ;
           }
       }
   }
   else 
   {
      message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                           "No entry selected") ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW Connect 5");
       message->Control() ;
      delete message ;
   }
}

EVENT_TYPE K_CONNECTOR::
Shortcut( const UI_EVENT & event )
{
    /* Get current entry from List */
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
    Shortcut( entry ) ;
    return  event.type;
}
    
void 
K_CONNECTOR::
Shortcut( KD_LIST_ITEM * entry )
{
   ZAF_MESSAGE_WINDOW * message = NULL ;
   KD_LIST_ITEM * def_template = entry ;

   if ( entry )
   {
       def_template = FindEntry( entry->_line_device ) ;
       ZIL_UINT32 process_id = CreateShortcut( entry, _config, def_template ) ; 
       if ( !process_id )
       {
           if ( StartKermitErrorCode == -1005 ) {
               message =
                   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Cancelled by User", 
                                          entry->_name) ;
               if ( !message )
                  OutofMemory("Unable to create MESSAGE_WINDOW Shortcut 1");
               message->Control() ;
               delete message ;
           }
           else if (  StartKermitErrorCode )
           {
              message =
                  new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to create K95 Shortcut: session: %s errorno %d", 
                                          entry->_name,
                                          StartKermitErrorCode ) ;
              if ( !message )
                  OutofMemory("Unable to create MESSAGE_WINDOW Shortcut 2");
              message->Control() ;
              delete message ;
           }
       }
   }
   else 
   {
      message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
				 "No entry selected") ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW Shortcut 2");
       message->Control() ;
      delete message ;
   }
}

EVENT_TYPE K_CONNECTOR::
ScriptFile( const UI_EVENT & event )
{
    /* Get current entry from List */
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
    ScriptFile( entry ) ;
    return  event.type;
}
    
void 
K_CONNECTOR::
ScriptFile( KD_LIST_ITEM * entry )
{
   ZAF_MESSAGE_WINDOW * message = NULL ;
   KD_LIST_ITEM * def_template = entry ;

   if ( entry )
   {
      def_template = FindEntry( entry->_line_device ) ;
      ZIL_UINT32 process_id = CreateScriptFile( entry, _config, def_template ) ; 
      if ( !process_id )
      {
          if ( StartKermitErrorCode == -1005 ) {
              message =
                  new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Cancelled by User", 
                                          entry->_name) ;
              if ( !message )
                  OutofMemory("Unable to create MESSAGE_WINDOW ScriptFile 1");
              message->Control() ;
              delete message ;
          }
          else if (  StartKermitErrorCode )
          {
              message =
                  new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to create K95 ScriptFile: session: %s errorno %d", 
                                          entry->_name,
                                          StartKermitErrorCode ) ;
              if ( !message )
                  OutofMemory("Unable to create MESSAGE_WINDOW ScriptFile 2");
              message->Control() ;
              delete message ;
          }
      }
   }
   else 
   {
      message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
				 "No entry selected") ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW ScriptFile 3");
       message->Control() ;
      delete message ;
   }
}

EVENT_TYPE K_CONNECTOR::
QuickConnect( const UI_EVENT & event )
{
    *windowManager + new K_QUICK( _config, NULL );
    return  event.type;
}

EVENT_TYPE K_CONNECTOR::Disconnect( const UI_EVENT & event ) 
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
    K_STATUS * status = K_STATUS::Find( entry->_name ) ;
    ZIL_UINT8 done = 0 ;

    while ( status && !done ) {
	ZAF_MESSAGE_WINDOW * confirm = 
	    new ZAF_MESSAGE_WINDOW( "Hangup?", "QUESTION", 
				    ZIL_MSG_YES | ZIL_MSG_NO | ZIL_MSG_CANCEL,
				    ZIL_MSG_NO,
				    "Okay to hangup session %d?",
				    status->_kermit_id) ;
        if ( !confirm )
            OutofMemory("Unable to create MESSAGE_WINDOW Disconnect 1");
	EVENT_TYPE msg = confirm->Control() ;
	switch ( msg ) {
	case ZIL_DLG_CANCEL:
	    delete confirm ;
	    return 0;
	case ZIL_DLG_YES:
	    done = 1 ;
	    break;
	case ZIL_DLG_NO:
	    status = K_STATUS::Next( status ) ;
	    break;
	}
	delete confirm;
	confirm = NULL ;
    }

    if ( status )
    {
#ifdef WIN32
	PostMessage( status->_hwnd, OPT_KERMIT_HANGUP, 0, 0 ) ;
#else
	ULONG bytes ;
	APIRET rc ;
	rc = DosWrite( (HFILE) status->_hwnd, 
		 "set exit warning off\rhangup\rexit\r",
		 34, &bytes ); 
	if ( rc ) {
	   ZAF_MESSAGE_WINDOW * message =
	       new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
				       ZIL_MSG_OK, ZIL_MSG_OK,
				       "Unable to Write to Pipe: %d", rc ) ; 
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW Disconnect 2");
            message->Control() ;
	   delete message ;
	}
#endif
    }
    return  event.type;
}

EVENT_TYPE K_CONNECTOR::AddEntry( const UI_EVENT & event ) 
{
    KD_LIST_ITEM * def_entry = FindEntry( "DEFAULT" ) ;
    K95_SETTINGS_DIALOG * dialog = NULL;

    if ( dialog ) {
        delete dialog;
        dialog = NULL;
    }

    if ( !def_entry ) {
        def_entry = new KD_LIST_ITEM;
        ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, 
				     "DEFAULT",
				     ID_KD_LIST_ITEM, 
				     UIS_OPENCREATE | UIS_READWRITE ) ;
        if ( _fileObj.objectError )
        {
            // What went wrong?
            ZAF_MESSAGE_WINDOW * message =
               new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                       "Unable to Open %s: errorno %d", "DEFAULT", _fileObj.objectError ) ;
            message->Control() ;
            delete message ;
        }  
        else 
        {
            def_entry->Store( "DEFAULT", K_CONNECTOR::_userFile, &_fileObj,
			  ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
            K_CONNECTOR::_userFile->Save() ;
            if ( K_CONNECTOR::_userFile->storageError )
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to Open DEFAULT: errorno %d",
                                          K_CONNECTOR::_userFile->storageError ) ;
                message->Control() ;
                delete message ;
            } else {
                AddEntry( def_entry );
            }
        }
    }
    KD_LIST_ITEM * new_entry = new KD_LIST_ITEM(*def_entry);
    strcpy( new_entry->_line_device, "DEFAULT" ) ;
    dialog = new K_DIALOG_GENERAL_SETTINGS(new_entry, ADD_ENTRY);
    if ( !dialog )
        OutofMemory("Unable to create GENERAL_SETTINGS AddEntry");
    dialog->Control();
    ZIL_UINT8 next = !dialog->_cancelled;
    delete dialog;
    dialog = NULL;
    if ( !next )
        return event.type;

    if ( next ) {
        dialog = new K_DIALOG_TERMINAL_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create TERMINAL_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_TRANSFER_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create TRANSFER_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && !(new_entry->_access != PHONE && new_entry->_access != DIRECT)) {
        dialog = new K_DIALOG_SERIAL_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create SERIAL_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && !(new_entry->_access == PHONE || new_entry->_access == SUPERLAT || 
                   new_entry->_access == DIRECT || new_entry->_access == SSH ||
                   new_entry->_access == FTP)) {
        dialog = new K_DIALOG_TELNET_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create TELNET_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && new_entry->_access == FTP ) {
        dialog = new K_DIALOG_FTP_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create FTP_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && new_entry->_access == SSH && _libeay_avail ) {
        dialog = new K_DIALOG_SSH_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create SSH_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && !(new_entry->_access == PHONE || new_entry->_access == SUPERLAT || 
                   new_entry->_access == DIRECT)) {
        dialog = new K_DIALOG_TCP_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create TCP_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && (_krb4_avail || _krb5_avail) &&
         !(new_entry->_access == PHONE || new_entry->_access == SUPERLAT || 
           new_entry->_access == DIRECT)) {
        dialog = new K_DIALOG_KERBEROS_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create KERBEROS_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next && _libeay_avail &&
         !(new_entry->_access == PHONE || new_entry->_access == SUPERLAT || 
           new_entry->_access == DIRECT)) {
        dialog = new K_DIALOG_TLS_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create TLS_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_KEYBOARD_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create KEYBOARD_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_LOGIN_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create LOGIN_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_PRINTER_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create PRINTER_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_LOGS_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create LOGS_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }
    if ( next ) {
        dialog = new K_DIALOG_GUI_SETTINGS(new_entry, ADD_ENTRY);
        if ( !dialog )
            OutofMemory("Unable to create GUI_SETTINGS AddEntry");
        dialog->Control();
        next = !dialog->_cancelled;
        delete dialog;
        dialog = NULL;
    }

    ZAF_MESSAGE_WINDOW * message =
        new ZAF_MESSAGE_WINDOW( "Reminder", "INFORMATION", 
                                ZIL_MSG_OK, ZIL_MSG_OK,
                                "Use the Connection->Edit menu to edit additional settings.") ;
    message->Control() ;
    delete message ;
    return  event.type;
}

EVENT_TYPE K_CONNECTOR::CloneEntry( const UI_EVENT & event ) 
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    KD_LIST_ITEM * entry = new KD_LIST_ITEM(*((KD_LIST_ITEM *) list->Current()));
    if ( !entry ) {
        ZAF_MESSAGE_WINDOW * message =
            new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                    "No entry selected") ;
        if ( !message )
            OutofMemory("Unable to create MESSAGE_WINDOW CloneEntry");
        message->Control() ;
        delete message ;
        return  event.type;
    }

    K95_SETTINGS_DIALOG * dialog;
    dialog = new K_DIALOG_GENERAL_SETTINGS(entry, CLONE_ENTRY);
    if ( !dialog )
        OutofMemory("Unable to create SETTINGS_NOTEBOOK Clone Entry");
    dialog->Control();
    ZIL_UINT8 cancelled = dialog->_cancelled;
    delete dialog;
    dialog = NULL;

    if ( !cancelled ) {
        ZAF_MESSAGE_WINDOW * message =
            new ZAF_MESSAGE_WINDOW( "Reminder", "INFORMATION", 
                                ZIL_MSG_OK, ZIL_MSG_OK,
                                "Use the Connection->Edit menu to edit additional settings.") ;
        message->Control() ;
        delete message ;
    }
    return  event.type;
}

EVENT_TYPE K_CONNECTOR::EditEntry( const UI_EVENT & event ) 
{
    printf("K_CONNECTOR::EditEntry()\n");

    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
    if ( !list ) {
        printf("K_CONNECTOR::EditEntry() Get(LIST_ENTRIES) failed\n");
        return event.type;
    }
    KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
    if ( entry ) {
        printf("K_CONNECTOR::EditEntry() calling new K_SETTINGS_NOTEBOOK(%s)\n",
                entry->_name);

        K95_SETTINGS_DIALOG * dialog = NULL;

        switch ( event.type ) {
        case OPT_EDIT_GENERAL:
            dialog = new K_DIALOG_GENERAL_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_TERMINAL:
            dialog = new K_DIALOG_TERMINAL_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_FILE_TRANSFER:
            dialog = new K_DIALOG_TRANSFER_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_SERIAL:
            dialog = new K_DIALOG_SERIAL_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_TELNET:
            dialog = new K_DIALOG_TELNET_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_TCP:
            dialog = new K_DIALOG_TCP_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_KERBEROS:
            dialog = new K_DIALOG_KERBEROS_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_TLS:
            dialog = new K_DIALOG_TLS_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_KEYBOARD:
            dialog = new K_DIALOG_KEYBOARD_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_LOGIN:
            dialog = new K_DIALOG_LOGIN_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_PRINTER:
            dialog = new K_DIALOG_PRINTER_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_LOGS:
            dialog = new K_DIALOG_LOGS_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_SSH:
            dialog = new K_DIALOG_SSH_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_FTP:
            dialog = new K_DIALOG_FTP_SETTINGS(entry, EDIT_ENTRY);
            break;
        case OPT_EDIT_GUI:
            dialog = new K_DIALOG_GUI_SETTINGS(entry, EDIT_ENTRY);
            break;
        }
        if ( dialog ) {
            if ( !dialog )
                OutofMemory("Unable to create dialog to allow editing");
            printf("K_CONNECTOR::EditEntry() calling dialog->Control()\n");
            dialog->Control();
            delete dialog;
        }
    }
   else 
   {
      ZAF_MESSAGE_WINDOW * message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                           "No entry selected") ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW EditEntry");
       message->Control() ;
       delete message ;
   }    
   return  event.type;
}

EVENT_TYPE K_CONNECTOR::RemoveEntry( const UI_EVENT & event ) 
{
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
   KD_LIST_ITEM * entry = (KD_LIST_ITEM *) list->Current();
   if ( entry )
   {
      if ( RemoveEntry( entry ) )
         // entry is removed from list by the previous call
         delete entry ; 
   }
   else 
   {
      ZAF_MESSAGE_WINDOW * message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                           "No entry selected") ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW RemoveEntry");
       message->Control() ;
      delete message ;
   }
   return  event.type;
}

EVENT_TYPE K_CONNECTOR::SetApplications( const UI_EVENT & event ) 
{
    K_APPLICATIONS * appl = new K_APPLICATIONS( _config ) ;
    if ( !appl )
        OutofMemory("Unable to create APPLICATIONS");
    appl->Control();
    delete appl;
    return event.type;
}

EVENT_TYPE K_CONNECTOR::SetDirDial( const UI_EVENT & event ) 
{
    K_DIAL_DIR * dir = new K_DIAL_DIR( _config ) ;
    if ( !dir )
        OutofMemory("Unable to create DIAL_DIR");
    dir->Control();
    delete dir;
    return event.type;
}

EVENT_TYPE K_CONNECTOR::SetDirNetwork( const UI_EVENT & event ) 
{
    K_NETWORK_DIR * dir = new K_NETWORK_DIR( _config ) ;
    if ( !dir )
        OutofMemory("Unable to create NETWORK_DIR");
    dir->Control();
    delete dir;
    return event.type;
}

EVENT_TYPE K_CONNECTOR::SetDialOptions( const UI_EVENT & event ) 
{
    K_DIAL_OPTIONS * options = new K_DIAL_OPTIONS( &_location_list ) ;
    if ( !options )
        OutofMemory("Unable to create DIAL_OPTIONS");
    options->Control();
    delete options;
    return event.type;
}

EVENT_TYPE K_CONNECTOR::SetModemOptions( const UI_EVENT & event ) 
{
    K_MODEM_DIALOG * dialog = new K_MODEM_DIALOG( &_modem_list ) ;
    if ( !dialog )
        OutofMemory("Unable to create MODEM_DIALOG");
    dialog->Control();
    delete dialog;
    return event.type;
}

EVENT_TYPE K_CONNECTOR::HelpAbout( const UI_EVENT & event ) 
{
   *windowManager + new K_ABOUT() ;
    return event.type;
}

void K_CONNECTOR::AddEntry( KD_LIST_ITEM * entry ) 
{
   UI_EVENT addEvent(S_ADD_OBJECT), redisplayEvent( S_REDISPLAY ) ;
//
//   addEvent.windowObject = entry ;
//   list->Event(addEvent);
   *VtList + entry ;
   VtList->Sort() ;
    *VtList + entry ;
   VtList->Event( redisplayEvent ) ;
}

ZIL_INT32
K_CONNECTOR::RemoveEntry( KD_LIST_ITEM * entry )
{
   if ( !entry->_userdefined )
   {
      ZAF_MESSAGE_WINDOW * message = 
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                 "Unable to remove Pre-Defined entry." ) ;

       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW RemoveEntry 1");
       message->Control();
      delete message ;
      return 0;
   }

   if ( !strcmp( entry->_name, "DEFAULT" ) )
   {                       
      ZAF_MESSAGE_WINDOW * message = 
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                 "Unable to remove DEFAULT template/entry." ) ;

       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW RemoveEntry 2");
       message->Control();
      delete message ;
      return 0;
   }

   /* Get user confirmation */
    ZAF_MESSAGE_WINDOW * confirm = 
	new ZAF_MESSAGE_WINDOW( "Remove?", "QUESTION", ZIL_MSG_OK | ZIL_MSG_CANCEL, 
				ZIL_MSG_CANCEL,
				"Okay to remove %s from phone book?",
				entry->_name) ;
    if ( !confirm )
        OutofMemory("Unable to create MESSAGE_WINDOW RemoveEntry 3");
    if ( confirm->Control() != ZIL_DLG_OK ) {
	delete confirm ;
	return 0;
    }
    delete confirm;

   if ( _userFile->DestroyObject( entry->_name ) )
   {
      // What went wrong?
      ZAF_MESSAGE_WINDOW * message =
         new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to remove %s: errorno %d", 
                                   entry->_name,
                                   _userFile->storageError ) ;
       if ( !message )
           OutofMemory("Unable to create MESSAGE_WINDOW RemoveEntry 4");
       message->Control();
      delete message ;
      return 0;
   }

   _userFile->Save(2) ;
   UI_EVENT redisplayEvent( S_REDISPLAY ) ;
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ; 
   *list - entry ;
   list->Event( redisplayEvent ) ;
   return 1;
}


/*
   S t a r t K e r m i t  --

   Call with:
     1. Pointer to a Dialer entry.
     2. Pointer to global configuration.
   
   Builds a command file based on Dialer entry info and starts Kermit,
   telling it to execute the file.

   Returns FALSE on failure, with global StartKermitErrorCode containing
   the error code.

   Returns a process handle if successful, with global StartKermitFileName
   containing a pointer to the output filename.  The caller presumably
   manages the process handles and output filename pointers in a
   multiple-process structure of some kind.  The output filename pointer is
   to a malloc'd chunk of memory, and should be copied to that structure
   before calling this routine again.  It can be freed if desired, but this
   routine will not do any freeing.

   Copyright (C) 1995, Trustees of Columbia University in the City of
   New York.  All rights reserved. */

/* Macros */

/* It is safe to not use a format of "%s" because when we need to preserve format */
/* characters we call BuildOutStr() first.                                        */

#ifdef WIN32
#define OUTFILE(x) if(fprintf(OutFile,x) < 0) { StartKermitErrorCode = GetLastError(); return(FALSE); }
#else
#define OUTFILE(x) if(fprintf(OutFile,x) < 0) { StartKermitErrorCode = errno; return(FALSE); }
#endif /* WIN32 */
#define BUFFERSIZE 3000

char *
BuildOutStr( char * buf, char * prefix, char * def )
{
    int i=0,l1=0,l2=0;
    l1 = strlen(prefix);
    strcpy(buf,prefix);
    l2 = strlen(def);
    for ( ;i<l2;i++,l1++ ) {
	buf[l1] = def[i];
	if ( def[i] == '%' )
	    buf[++l1] = '%';
    }
    buf[l1++] = '\n';
    buf[l1] = '\0';
    return buf;
}

char *
BuildOutFileStr( char * buf, char * prefix, char * def )
{
    int i=0,l1=0,l2=0;
    l1 = strlen(prefix);
    strcpy(buf,prefix);
    l2 = strlen(def);
    for ( ;i<l2;i++,l1++ ) {
	buf[l1] = def[i];
	if ( def[i] == '%' )
	    buf[++l1] = '%';
#ifdef COMMENT
        /* Can't do this.  It changes \v(foo) to /v(foo) */
        else if ( def[i] == '\\' )
	    buf[++l1] = '/';
#endif /* COMMENT */
    }
    buf[l1++] = '\n';
    buf[l1] = '\0';
    return buf;
}

ZIL_UINT32 K_CONNECTOR::
GenerateModem( K_MODEM * modem, KD_LIST_ITEM * entry, FILE * OutFile )
{
    ZIL_ICHAR buf[BUFFERSIZE];

    if ( !modem || !OutFile )
	return -1;

    sprintf(buf,"if not equal \"\\v(modem)\" \"%s\" set modem type %s\n", 
	     modem->_type, modem->_type);
    OUTFILE(buf);

    if ( modem->_is_tapi ) {
	sprintf(buf, "set tapi line %s\n", modem->_port);
        OUTFILE(buf);
	sprintf(buf, "if fail end 1 TAPI line %s already in use.\n", modem->_port);
        OUTFILE(buf);
    } else {
	sprintf(buf, "set port %s\n", modem->_port);
        OUTFILE(buf);
	sprintf(buf, "if fail end 1 Port %s already in use.\n", modem->_port);
        OUTFILE(buf);
    }

    if ( (!entry || entry->_use_mdm_carrier) ?
	 modem->_carrier : entry->_carrier) {
	OUTFILE("set carrier auto\n");
	 } else {
	     OUTFILE("set carrier off\n");
	 }

    if ((!entry || entry->_use_mdm_ec) ? 
	 modem->_correction : entry->_correction) {
	OUTFILE("set modem error-correction on\n");
	 } else {
	     OUTFILE("set modem error-correction off\n");
	 }
    if ((!entry || entry->_use_mdm_dc) ? 
	 modem->_compression : entry->_compression) {
	OUTFILE("set modem compression on\n");
	 } else {
	     OUTFILE("set modem compression off\n");
	 }

    sprintf(buf, "set speed %ld\n", 
	     (!entry || entry->_use_mdm_speed) ? modem->_speed : entry->_speed);
    OUTFILE(buf);

    switch ((!entry || entry->_use_mdm_parity) ? 
	     modem->_parity : entry->_parity) {           /* Parity */
	     case NO_PARITY:
	         OUTFILE("set parity none\n");
		 break;
	     case SPACE:
		 OUTFILE("set parity space\n");
		 break;
	     case MARK:
		 OUTFILE("set parity mark\n");
		 break;
	     case EVEN:
		 OUTFILE("set parity even\n");
		 break;
	     case ODD:
		 OUTFILE("set parity odd\n");
		 break;
	     case SPACE_8:
		 OUTFILE("set parity hardware space\n");
		 break;
	     case MARK_8:
		 OUTFILE("set parity hardware mark\n");
		 break;
	     case EVEN_8:
		 OUTFILE("set parity hardware even\n");
		 break;
	     case ODD_8:
		 OUTFILE("set parity hardware odd\n");
		 break;
	     }

    switch ((!entry || entry->_use_mdm_stopbits) ? 
	     modem->_stopbits : entry->_stopbits) {           /* StopBits */
	     case STOPBITS_1_0:
	         OUTFILE("set stop-bits 1\n");
		 break;
	     case STOPBITS_2_0:
	         OUTFILE("set stop-bits 2\n");
		 break;
	     }

    switch ((!entry || entry->_use_mdm_flow) ? 
	     modem->_flow : entry->_flow) {         /* Flow control */
	     case NO_FLOW:
	         OUTFILE("set flow none\n");
		 break;
	     case XON_XOFF:
		 OUTFILE("set flow xon/xoff\n");
		 break;
	     case RTS_CTS:
		 OUTFILE("set flow rts/cts\n");
		 break;
	     case AUTO_FLOW:
		 OUTFILE("set flow auto\n");
		 break;
	     }

    switch ( modem->_volume ) {
    case 1:
        OUTFILE("set modem volume low\n");
        break;
    case 2:
        OUTFILE("set modem volume medium\n");
        break;
    case 3:
        OUTFILE("set modem volume high\n");
        break;
    }
    if ( modem->_speaker ) {
        OUTFILE("set modem speaker on\n");
    }
    else {
        OUTFILE("set modem speaker off\n");
    }
    sprintf(buf,"set modem escape-char %d\n", modem->_escape_char);
    OUTFILE(buf);
    if ( modem->_speed_match ) {
        OUTFILE("set modem speed-match on\n");
    }
    else {
        OUTFILE("set modem speed-match off\n");
    }

    /* Now the modem definitions -- we have already issued the SET MODEM TYPE command */
    /* Only do this is the modem is not a TAPI device.  That is handled internally by */
    /* Kermit-95. */
    if ( !modem->_is_tapi ) {
        sprintf(buf,"set modem name %s\n", modem->_name);
        OUTFILE(buf);
        sprintf(buf,"set modem maximum-speed %d\n",modem->_max_speed);
        OUTFILE(buf);
	sprintf(buf,"set modem command autoanswer on %s\n",
		 modem->_aa_on_str);
	OUTFILE(buf);
	OUTFILE(BuildOutStr(buf,"set modem command autoanswer off ",
			     modem->_aa_off_str));
	OUTFILE(BuildOutStr(buf,"set modem command compression on ",
			     modem->_dc_on_str));
	OUTFILE(BuildOutStr(buf,"set modem command compression off ",
			     modem->_dc_off_str));
	OUTFILE(BuildOutStr(buf,"set modem command error-correction on ",
			     modem->_ec_on_str));
	OUTFILE(BuildOutStr(buf,"set modem command error-correction off ",
			     modem->_ec_off_str));
	OUTFILE(BuildOutStr(buf,";set modem command speed-buffering on ",
			     modem->_sb_on_str));
	OUTFILE(BuildOutStr(buf,";set modem command speed-buffering off ",
			     modem->_sb_off_str));
	OUTFILE(BuildOutStr(buf,"set modem command speaker on ",
			     modem->_sp_on_str));
	OUTFILE(BuildOutStr(buf,"set modem command speaker off ",
			     modem->_sp_off_str));
	OUTFILE(BuildOutStr(buf,"set modem command dial-mode-prompt ",
			     modem->_dial_mode_prompt_str));
	OUTFILE(BuildOutStr(buf,"set modem command dial-mode-string ",
			     modem->_dial_mode_str));
	OUTFILE(BuildOutStr(buf,"set modem dial-command ",modem->_dial_str));
	OUTFILE(BuildOutStr(buf,"set modem command hangup ",
			     modem->_hup_str));
	OUTFILE(BuildOutStr(buf,"set modem command hardware-flow ",
			     modem->_hwfc_str));
	OUTFILE(BuildOutStr(buf,"set modem command software-flow ",
			     modem->_swfc_str));
	OUTFILE(BuildOutStr(buf,"set modem command no-flow-control ",
			     modem->_nofc_str));
	OUTFILE(BuildOutStr(buf,"set modem command init-string ",
			     modem->_init_str));
	OUTFILE(BuildOutStr(buf,"set modem command pulse ",
			     modem->_pulse_str));
	OUTFILE(BuildOutStr(buf,"set modem command tone ",
			     modem->_tone_str));
	OUTFILE(BuildOutStr(buf,"set modem command volume low ",
			     modem->_vol1_str));
	OUTFILE(BuildOutStr(buf,"set modem command volume medium ",
			     modem->_vol2_str));
	OUTFILE(BuildOutStr(buf,"set modem command volume high ",
			     modem->_vol3_str));
	OUTFILE(BuildOutStr(buf,"set modem command ignore-dialtone ",
			     modem->_ignore_dial_tone_str));
	OUTFILE("set modem capabilities");
	if ( modem->_at ) OUTFILE(" at-commands");
	if ( modem->_hw ) OUTFILE(" hardware-flow");
	if ( modem->_sb ) OUTFILE(" speed-buffering");
	if ( modem->_dc ) OUTFILE(" compression");
	if ( modem->_ks ) OUTFILE(" kermit-spoof");
	if ( modem->_tb ) OUTFILE(" telebit");
	if ( modem->_ec ) OUTFILE(" error-correction");
	if ( modem->_sw ) OUTFILE(" software-flow");
	if ( modem->_v25 ) OUTFILE(" v25bis-commands");
	OUTFILE("\n");
    }
    return 0;
}

ZIL_UINT32 K_CONNECTOR::
GenerateLocation( K_LOCATION * location, FILE * OutFile )
{
    ZIL_ICHAR buf[BUFFERSIZE];
    if ( location ) {
	if (location->_dial_method == Pulse) { /* Dial method */
	    OUTFILE("set dial method pulse\n");
	} else {
	    OUTFILE("set dial method tone\n");
	}

        /* Locale and codes... */

	sprintf(buf, "set dial country %s\n", location->_country_code);
	OUTFILE(buf);
	sprintf(buf, "set dial area %s\n", location->_area_code);
	OUTFILE(buf);
	sprintf(buf, "set dial force-long-dist %s\n", 
		 location->_force_long_dist ? "on" : "off");
	OUTFILE(buf);
	sprintf(buf, "set dial ignore-dialtone %s\n", 
		 location->_ignore_dial_tone ? "on" : "off");
	OUTFILE(buf);
	sprintf(buf, "set dial lc-area-codes %s\n", location->_local_area_codes_list);
	OUTFILE(buf);
	sprintf(buf, "set dial lc-prefix %s\n", location->_local_area_prefix);
	OUTFILE(buf);
	sprintf(buf, "set dial lc-suffix %s\n", location->_local_area_suffix);
	OUTFILE(buf);
	sprintf(buf, "set dial ld-prefix %s\n", location->_long_dist_prefix);
	OUTFILE(buf);
	sprintf(buf, "set dial ld-suffix %s\n", location->_long_dist_suffix);
	OUTFILE(buf);
	sprintf(buf, "set dial intl-prefix %s\n", location->_intl_prefix);
	OUTFILE(buf);
	sprintf(buf, "set dial intl-suffix %s\n", location->_intl_suffix);
	OUTFILE(buf);
	sprintf(buf, "set dial prefix %s\n", location->_dial_prefix);
	OUTFILE(buf);
	sprintf(buf, "set dial suffix %s\n", location->_dial_suffix);
	OUTFILE(buf);
	sprintf(buf, "set dial tf-prefix %s\n", location->_dial_tf_prefix);
	OUTFILE(buf);
	sprintf(buf, "set dial tf-area %s\n", location->_dial_tf_area);
	OUTFILE(buf);

	if (location->_use_pbx) {
	    sprintf(buf, "set dial pbx-outside %s\n", location->_pbx_out);
	    OUTFILE(buf);
	    sprintf(buf, "set dial pbx-exchange %s\n", location->_pbx_exch);
	    OUTFILE(buf);
	    sprintf(buf, "set dial pbx-inside %s\n", location->_pbx_in);
	    OUTFILE(buf);
	}

	sprintf(buf, "set dial timeout %d\n", (int)(location->_dial_timeout));
	OUTFILE(buf);
	sprintf(buf, "set dial retries %d\n", (int)(location->_redial_count));
	OUTFILE(buf);
	sprintf(buf, "set dial interval %d\n",(int)(location->_redial_interval));
	OUTFILE(buf);
    }
    return 0;
}


ZIL_UINT32 K_CONNECTOR::
GenerateScript( KD_LIST_ITEM * entry, KD_CONFIG * config, 
		KD_LIST_ITEM * def_entry, FILE * OutFile )
{
    ZAF_MESSAGE_WINDOW * MessageBox = NULL ;       /* Used to error messages */
    ZIL_ICHAR buf[BUFFERSIZE];
    ZIL_ICHAR tmp[BUFFERSIZE];
    ZIL_ICHAR *p;
    K_LOCATION * location = (K_LOCATION *) _location_list.Current();

    sprintf(buf,"; Kermit 95 Dialer Generated Script - Version %d.%d\n",
             kd_major,kd_minor);
    OUTFILE(buf);

    /* start logs */
    if ( entry->_log_debug )
    {
	sprintf(buf, "log debug {%s}%s\n",entry->_log_debug_fname,
		 entry->_log_debug_append?" append":"");
	OUTFILE(buf)
    }
    if ( entry->_log_connection )
    {
	sprintf(buf, "log connection {%s}%s\n",entry->_log_connection_fname,
		 entry->_log_connection_append?" append":"");
	OUTFILE(buf)
    }
    if ( entry->_log_session )
    {
	sprintf(buf, "log session {%s}%s\n",entry->_log_session_fname,
		 entry->_log_session_append?" append":"");
        OUTFILE(buf);
        switch ( entry->_log_session_type ) {
        case LOG_TEXT:
            OUTFILE("set session-log text\n");
            break;
        case LOG_BINARY:
            OUTFILE("set session-log binary\n");
            break;
        case LOG_DEBUG:
            OUTFILE("set session-log debug\n");
            break;
        }
    }
    if ( entry->_log_packet )
    {
        sprintf(buf, "log packet {%s}%s\n",entry->_log_packet_fname,
		 entry->_log_packet_append?" append":"");
        OUTFILE(buf);
    }
    if ( entry->_log_transaction )
    {
        sprintf(buf, "set transaction-log ",
                 entry->_log_transaction_brief?"brief":"verbose");
        OUTFILE(buf);
	sprintf(buf, "log transaction {%s}%s\n",
                             entry->_log_transaction_fname,
                             entry->_log_transaction_append?" append":"");
        OUTFILE(buf);
    }

    /* Start with the Session Title */
    sprintf(buf, "set title %s\n",entry->_name);
    OUTFILE(buf)

    /* Write the Command settings even before the terminal ones */
    sprintf(buf, "set command color %s %s\n",
            entry->_color_command_fg.Name(),
            entry->_color_command_bg.Name());
    OUTFILE(buf);

    sprintf(buf,"clear command\n");
    OUTFILE(buf);

#ifdef WIN32
    if ( entry->_use_gui ) {
        OUTFILE("if gui {\n");
        sprintf(buf,"set gui window resize-mode %s\n",
                 entry->_gui_resize == RES_SCALE_FONT ? "scale-font" : "change-dimensions");
        OUTFILE(buf);
        sprintf(buf,"set gui window run-mode %s\n",
                 entry->_gui_run == RUN_MAX ? "maximize" :
                 entry->_gui_run == RUN_MIN ? "minimize" : "restore");
        OUTFILE(buf);

        sprintf(buf,"set gui rgbcolor black %d %d %d\n", 
                 entry->_rgb[0][0],
                 entry->_rgb[0][1],
                 entry->_rgb[0][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor blue %d %d %d\n", 
                 entry->_rgb[1][0],
                 entry->_rgb[1][1],
                 entry->_rgb[1][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor green %d %d %d\n", 
                 entry->_rgb[2][0],
                 entry->_rgb[2][1],
                 entry->_rgb[2][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor cyan %d %d %d\n", 
                 entry->_rgb[3][0],
                 entry->_rgb[3][1],
                 entry->_rgb[3][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor red %d %d %d\n", 
                 entry->_rgb[4][0],
                 entry->_rgb[4][1],
                 entry->_rgb[4][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor magenta %d %d %d\n", 
                 entry->_rgb[5][0],
                 entry->_rgb[5][1],
                 entry->_rgb[5][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor brown %d %d %d\n", 
                 entry->_rgb[6][0],
                 entry->_rgb[6][1],
                 entry->_rgb[6][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightgray %d %d %d\n", 
                 entry->_rgb[7][0],
                 entry->_rgb[7][1],
                 entry->_rgb[7][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor darkgray %d %d %d\n", 
                 entry->_rgb[8][0],
                 entry->_rgb[8][1],
                 entry->_rgb[8][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightblue %d %d %d\n", 
                 entry->_rgb[9][0],
                 entry->_rgb[9][1],
                 entry->_rgb[9][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightgreen %d %d %d\n", 
                 entry->_rgb[10][0],
                 entry->_rgb[10][1],
                 entry->_rgb[10][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightcyan %d %d %d\n", 
                 entry->_rgb[11][0],
                 entry->_rgb[11][1],
                 entry->_rgb[11][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightred %d %d %d\n", 
                 entry->_rgb[12][0],
                 entry->_rgb[12][1],
                 entry->_rgb[12][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor lightmagenta %d %d %d\n", 
                 entry->_rgb[13][0],
                 entry->_rgb[13][1],
                 entry->_rgb[13][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor yellow %d %d %d\n", 
                 entry->_rgb[14][0],
                 entry->_rgb[14][1],
                 entry->_rgb[14][2]);
        OUTFILE(buf);
        sprintf(buf,"set gui rgbcolor white %d %d %d\n", 
                 entry->_rgb[15][0],
                 entry->_rgb[15][1],
                 entry->_rgb[15][2]);
        OUTFILE(buf);
        OUTFILE("}\n");
    }
#endif /* WIN32 */
    OUTFILE("cd \\v(home)\n");

    /* Write the applications */
#ifdef COMMENT
    // No longer supported; replaced by built-in ftp support
    if ( config->_app_ftp[0] ) {
        sprintf(tmp,"set ftp {%s}\n", config->_app_ftp);
        OUTFILE(BuildOutStr(buf,"",tmp));
    }
#endif /* COMMENT */
    if ( config->_app_www[0] ) {
        sprintf(tmp,"set browser {%s}\n", config->_app_www);
        OUTFILE(BuildOutFileStr(buf,"",tmp));
    }
    if ( config->_app_edit[0] ) {
        sprintf(tmp,"set editor {%s}\n", config->_app_edit);
        OUTFILE(BuildOutFileStr(buf,"",tmp));
    }

    /* Write the startup script file - Terminal settings first */

    switch (entry->_terminal) {         /* Terminal Type */
    case VT320:
        OUTFILE("set terminal type vt320\n");
        break;
    case VT220:
        OUTFILE("set terminal type vt220\n");
        break;
    case VT102:
        OUTFILE("set terminal type vt102\n");
        break;
    case VT100:
        OUTFILE("set terminal type vt100\n");
        break;
    case VT52:
        OUTFILE("set terminal type vt52\n");
        break;
    case ANSI:
        OUTFILE("set terminal type ansi\n");
        break;
    case TTY:
        OUTFILE("set terminal type tty\n");
        break;
    case WY30:
        OUTFILE("set terminal type wy30\n");
        break;
    case WY50:
        OUTFILE("set terminal type wy50\n");
        break;
    case WY60:
        OUTFILE("set terminal type wy60\n");
        break;
    case WY160:
        OUTFILE("set terminal type wy160\n");
        break;
    case WY370:
        OUTFILE("set terminal type wy370\n");
        break;
    case SCOANSI:
        OUTFILE("set terminal type scoansi\n");
        break;
    case AT386:
        OUTFILE("set terminal type at386\n");
        break;
    case DG200:
        OUTFILE("set terminal type dg200\n");
        break;
    case AVATAR:
        OUTFILE("set terminal type avatar\n");
        break;
    case DG210:
        OUTFILE("set terminal type dg210\n");
        break;
    case DG217:
        OUTFILE("set terminal type dg217\n");
        break;
    case HEATH19:
        OUTFILE("set terminal type heath19\n");
        break;
    case HP2621:
        OUTFILE("set terminal type hp2621\n");
        break;
    case HPTERM:
        OUTFILE("set terminal type hpterm\n");
        break;
    case HZ1500:
        OUTFILE("set terminal type hz1500\n");
        break;
    case LINUX:
        OUTFILE("set terminal type linux\n");
        break;
    case TVI910:
        OUTFILE("set terminal type tvi910\n");
        break;
    case TVI925:
        OUTFILE("set terminal type tvi925\n");
        break;
    case TVI950:
        OUTFILE("set terminal type tvi950\n");
        break;
    case VC404:
        OUTFILE("set terminal type vc404\n");
        break;
    case VIP7809:
        OUTFILE("set terminal type vip7809\n");
        break;
    case BEOS:
        OUTFILE("set terminal type beos-ansi\n");
        break;
    case QNX:
        OUTFILE("set terminal type qnx\n");
        break;
    case QANSI:
        OUTFILE("set terminal type qansi\n");
        break;
    case SNI97801:
        OUTFILE("set terminal type sni-97801\n");
        break;
    case BA80:
        OUTFILE("set terminal type ba80\n");
        break;
    case SUN:
        OUTFILE("set terminal type sun\n");
        break;
    case ANNARBOR:
        OUTFILE("set terminal type annarbor\n");
        break;
    case AIXTERM:
        OUTFILE("set terminal type aixterm\n");
        break;
    case HFT:
        OUTFILE("set terminal type hft\n");
        break;
    case VTNT:
#ifdef WIN32
        OUTFILE("set terminal type vtnt\n");
#else 
        OUTFILE("echo VTNT is not supported on OS/2\nstop\n");
#endif 
        break;
    case IBM3151:
        OUTFILE("set terminal type ibm3151\n");
        break;
    case ADM3A:
        OUTFILE("set terminal type adm3a\n");
        break;
    default:
        OUTFILE("echo No terminal type specified\nstop\n");
    }

   /* Telnet and TCP options */
    if ( entry->_access == TCPIP || entry->_access == FTP ) 
    {
        if ( entry->_telnet_ttype[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet terminal-type ", 
                                 entry->_telnet_ttype));
        }

        switch ( entry->_telnet_auth_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt authentication accept\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt authentication refuse\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt authentication request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt authentication require\n");
            break;
        }
        OUTFILE(buf);

        switch ( entry->_telnet_binary_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt binary accepted accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt binary refused refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt binary requested accept\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt binary require accept\n");
            break;
        }
        OUTFILE(buf);

        switch ( entry->_telnet_encrypt_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt encryption accepted accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt encryption refused refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt encryption request request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt encryption require require\n");
            break;
        }
        OUTFILE(buf);

        switch ( entry->_telnet_kermit_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt kermit accepted accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt kermit refused refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt kermit request request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt kermit require require\n");
            break;
        }
        OUTFILE(buf);

        switch ( entry->_telnet_fwdx_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt forward-x accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt forward-x refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt forward-x request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt forward-x require\n");
            break;
        }
        OUTFILE(buf);

        switch ( entry->_telnet_cpc_mode ) {
        case TelnetAccept:
            sprintf(buf, "set telopt com-port-control accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "set telopt com-port-control refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "set telopt com-port-control request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "set telopt com-port-control require\n");
            break;
        }
        OUTFILE(buf);

        if ( entry->_telnet_debug ) {
            OUTFILE("set telnet debug on\n");
        } else {
            OUTFILE("set telnet debug off\n");
        }

        if ( entry->_telnet_sb_delay ) {
            OUTFILE("set telnet delay-sb on\n");
        } else {
            OUTFILE("set telnet delay-sb off\n");
        }

        if ( entry->_telnet_location[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet location ", 
                                 entry->_telnet_location));
        }

        if ( !entry->_default_ip_address ) {
            char tmp[17]; int i=0;
            if ( entry->_tcp_ip_address[0] != '0' )
                tmp[i++] = entry->_tcp_ip_address[0];
            if ( entry->_tcp_ip_address[1] != 0 )
                tmp[i++] = entry->_tcp_ip_address[1];
            tmp[i++] = entry->_tcp_ip_address[2];
            tmp[i++] = entry->_tcp_ip_address[3];
            if ( entry->_tcp_ip_address[4] != '0' )
                tmp[i++] = entry->_tcp_ip_address[4];
            if ( entry->_tcp_ip_address[5] != 0 )
                tmp[i++] = entry->_tcp_ip_address[5];
            tmp[i++] = entry->_tcp_ip_address[6];
            tmp[i++] = entry->_tcp_ip_address[7];
            if ( entry->_tcp_ip_address[8] != '0' )
                tmp[i++] = entry->_tcp_ip_address[8];
            if ( entry->_tcp_ip_address[9] != 0 )
                tmp[i++] = entry->_tcp_ip_address[9];
            tmp[i++] = entry->_tcp_ip_address[10];
            tmp[i++] = entry->_tcp_ip_address[11];
            if ( entry->_tcp_ip_address[12] != '0' )
                tmp[i++] = entry->_tcp_ip_address[12];
            if ( entry->_tcp_ip_address[13] != 0 )
                tmp[i++] = entry->_tcp_ip_address[13];
            tmp[i++] = entry->_tcp_ip_address[14];
            tmp[i++] = entry->_tcp_ip_address[15];
            tmp[i++] = '\0';
            sprintf( buf, "set tcp address %s\n", tmp);
            OUTFILE(buf);
        }

        sprintf( buf, "set tcp sendbuf %d\n", entry->_tcp_sendbuf);
        OUTFILE(buf);
        sprintf( buf, "set tcp recvbuf %d\n", entry->_tcp_recvbuf);
        OUTFILE(buf);

        switch ( entry->_tcp_rdns ) {
        case AUTO:
            sprintf( buf, "set tcp reverse-dns-lookup auto\n");
            break;
        case ON:
            sprintf( buf, "set tcp reverse-dns-lookup on\n");
            break;
        case OFF:
            sprintf( buf, "set tcp reverse-dns-lookup off\n");
            break;
        }
        OUTFILE(buf);

        if ( entry->_socks_ipaddress[0] ) {
            if ( entry->_socks_ipport[0] )
                sprintf( buf, "set tcp socks-server /user:%s /pass:%s %s:%s\n", 
                         entry->_socks_user, entry->_socks_pass,
                         entry->_socks_ipaddress, entry->_socks_ipport);
            else
                sprintf( buf, "set tcp socks-server /user:%s /pass:%s %s\n", 
                         entry->_socks_user, entry->_socks_pass,
                         entry->_socks_ipaddress);
            OUTFILE(buf);
        }

        if ( entry->_proxy_ipaddress[0] ) {
            if ( entry->_proxy_ipport[0] )
                sprintf( buf, "set tcp http-proxy /user:%s /pass:%s %s:%s\n", 
                         entry->_proxy_user, entry->_proxy_pass,
                         entry->_proxy_ipaddress, entry->_proxy_ipport);
            else
                sprintf( buf, "set tcp http-proxy /user:%s /pass:%s %s\n", 
                         entry->_proxy_user, entry->_proxy_pass,
                         entry->_proxy_ipaddress);
            OUTFILE(buf);
        }

        if ( _dns_srv_avail ) {
            if ( entry->_tcp_dns_srv ) {
                OUTFILE("set tcp dns-service-records on\n");
            }
            else {
                OUTFILE("set tcp dns-service-records off\n");
            }
        }
        if ( entry->_fwd_cred ) {
            OUTFILE("set telnet authentication forwarding on\n");
        }
        else {
            OUTFILE("set telnet authentication forwarding off\n");
        }

        if ( entry->_telnet_wait ) {
            OUTFILE("set telnet wait-for-negotiations on\n");
        }
        else {
            OUTFILE("set telnet wait-for-negotiations off\n");
        }

        if ( entry->_telnet_acct[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet environment acct ",
                     entry->_telnet_acct));
        }

        if ( entry->_telnet_disp[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet environment disp ",
                     entry->_telnet_disp));
        }

        if ( entry->_telnet_job[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet environment job ",
                     entry->_telnet_job));
        }
    
        if ( entry->_telnet_printer[0] ) {
            OUTFILE(BuildOutStr( buf, "set telnet environment printer ",
                     entry->_telnet_printer));
        }

        OUTFILE("if available kerberos4 {\n");
        if ( entry->_k4_princ[0] ) {
            OUTFILE(BuildOutStr( buf, "  set authentication kerberos4 princ ",
                     entry->_k4_princ));
        }
        if ( entry->_k4_realm[0] ) {
            OUTFILE(BuildOutStr( buf, "  set authentication kerberos4 realm ",
                     entry->_k4_realm));
        }
        if ( entry->_k4_autoget ) {
            OUTFILE("  set authentication kerberos4 autoget on\n");
        }
        else {
            OUTFILE("  set authentication kerberos4 autoget off\n");
        }

        if ( entry->_k4_autodestroy ) {
            OUTFILE("  set authentication kerberos4 autodestroy on-close\n");
        }
        else {
            OUTFILE("  set authentication kerberos4 autodestroy never\n");
        }
        sprintf( buf, "  set authentication kerberos4 lifetime %d\n",
                 entry->_k4_lifetime);
        OUTFILE(buf);
        OUTFILE("}\n");


        OUTFILE("if available kerberos5 {\n");
        if ( entry->_k5_princ[0] ) {
            OUTFILE(BuildOutStr( buf, "  set authentication kerberos5 princ ",
                     entry->_k5_princ));
        }
        if ( entry->_k5_realm[0] ) {
            OUTFILE(BuildOutStr( buf, "  set authentication kerberos5 realm ",
                     entry->_k5_realm));
        }
        if ( entry->_k5_cache[0] ) {
            sprintf(tmp,"  set authentication k5 credentials-cache {%s}",
                     entry->_k5_cache);
            OUTFILE(BuildOutStr( buf, "", tmp));
        }
        if ( entry->_k5_autoget ) {
            OUTFILE("  set authentication kerberos5 autoget on\n");
        }
        else {
            OUTFILE("  set authentication kerberos5 autoget off\n");
        }

        if ( entry->_k5_autodestroy ) {
            OUTFILE("  set authentication kerberos5 autodestroy on-close\n");
        }
        else {
            OUTFILE("  set authentication kerberos5 autodestroy never\n");
        }
        sprintf( buf, "  set authentication kerberos5 lifetime %d\n",
                 entry->_k5_lifetime);
        OUTFILE(buf);
        if ( entry->_k5_forwardable ) {
            OUTFILE("  set authentication kerberos5 forwardable on\n");
        }
        else {
            OUTFILE("  set authentication kerberos5 forwardable off\n");
        }
        if ( entry->_k5_getk4tkt ) {
            OUTFILE("  set authentication kerberos5 get-k4-tgt on\n");
        }
        else {
            OUTFILE("  set authentication kerberos5 get-k4-tgt off\n");
        }
        OUTFILE("}\n");

        OUTFILE("if available ssl {\n");
        switch ( entry->_telnet_starttls_mode ) {
        case TelnetAccept:
            sprintf(buf, "  set telopt start-tls accepted\n");
            break;
        case TelnetRefuse:
            sprintf(buf, "  set telopt start-tls refused\n");
            break;
        case TelnetRequest:
            sprintf(buf, "  set telopt start-tls request\n");
            break;
        case TelnetRequire:
            sprintf(buf, "  set telopt start-tls require\n");
            break;
        }
        OUTFILE(buf);

        if ( entry->_tls_cipher[0] ) {
            sprintf(tmp,"  set auth ssl cipher {%s}",
                     entry->_tls_cipher);
            OUTFILE(BuildOutStr( buf, "", tmp));
        }
        if ( entry->_tls_cert_file[0] ) {
            sprintf(tmp,"  set auth ssl dsa-cert-file {%s}",
                     entry->_tls_cert_file);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        if ( entry->_tls_key_file[0] ) {
            sprintf(tmp,"  set auth ssl dsa-key-file {%s}",
                     entry->_tls_key_file);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        if ( entry->_tls_verify_file[0] ) {
            sprintf(tmp,"  set auth ssl verify-file {%s}",
                     entry->_tls_verify_file);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        if ( entry->_tls_verify_dir[0] ) {
            sprintf(tmp, "  set auth ssl verify-dir {%s}",
                     entry->_tls_verify_dir);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        if ( entry->_tls_crl_file[0] ) {
            sprintf(tmp,"  set auth ssl crl-file %{s}",
                     entry->_tls_crl_file);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        if ( entry->_tls_crl_dir[0] ) {
            sprintf(tmp, "  set auth ssl crl-dir {%s}",
                     entry->_tls_crl_dir);
            OUTFILE(BuildOutFileStr( buf, "", tmp));
        }
        switch ( entry->_tls_verify_mode ) {
        case TLS_VERIFY_NO:
            sprintf(buf, "  set auth ssl verify no\n");
            break;
        case TLS_VERIFY_PEER:
            sprintf(buf, "  set auth ssl verify peer-cert\n");
            break;
        case TLS_VERIFY_FAIL:
            sprintf(buf, "  set auth ssl verify fail-if-no-peer-cert\n");
            break;
        }
        OUTFILE(buf);
        if ( entry->_tls_verbose ) {
            OUTFILE("  set auth ssl verbose on\n");
        } else {
            OUTFILE("  set auth ssl verbose off\n");
        }
        if ( entry->_tls_debug ) {
            OUTFILE("  set auth ssl debug on\n");
        } else {
            OUTFILE("  set auth ssl debug off\n");
        }

        OUTFILE("}\n");
    }

   /* Terminal Byte Size */
   sprintf( buf, "set term bytesize %d\n", entry->_charsize ) ;
   OUTFILE(buf);

   /* Terminal Echo */
   sprintf( buf, "set term echo %s\n", entry->_local_echo ? "on" : "off" ) ;
   OUTFILE(buf);

   /* Terminal Wrap */
   sprintf( buf, "set term wrap %s\n", entry->_auto_wrap ? "on" : "off" ) ;
   OUTFILE(buf);

   /* Terminal APC */
   sprintf( buf, "set term apc %s\n", entry->_apc_enabled ? "on" : "off" ) ;
   OUTFILE(buf);

   /* Terminal AutoDownload */
   sprintf( buf, "set term autodown %s\n", entry->_auto_download == YNA_YES ? "on" : 
            entry->_auto_download == YNA_NO ? "off" : "ask" ) ;
   OUTFILE(buf);

    /* Terminal Status Line */
    sprintf( buf, "set term status %s\n", entry->_status_line ? "on" : "off" ) ;
    OUTFILE(buf);

   /* Terminal Width */
   sprintf( buf, "set term width %d\n", entry->_width ) ;
   OUTFILE(buf);

   /* Terminal Height */
   sprintf( buf, "set term height %d\n", entry->_height ) ;
   OUTFILE(buf);

   /* Terminal Cursor */
   switch ( entry->_cursor ) {
   case FULL_CURSOR:
      sprintf( buf, "set term cursor full\n") ;
      break;
   case HALF_CURSOR:
      sprintf( buf, "set term cursor half\n") ;
      break;
   case UNDERLINE_CURSOR:
      sprintf( buf, "set term cursor underline\n") ;
      break;
   }
   OUTFILE(buf);


    /* Terminal color */

    sprintf(buf, "set term color term %s %s\n",
            entry->_color_term_fg.Name(),
            entry->_color_term_bg.Name()
            );
    OUTFILE(buf);
    sprintf(buf, "set term color status %s %s\n",
            entry->_color_status_fg.Name(),
            entry->_color_status_bg.Name()
            );
    OUTFILE(buf);
    sprintf(buf, "set term color help %s %s\n",
            entry->_color_popup_fg.Name(),
            entry->_color_popup_bg.Name()
            );
    OUTFILE(buf);

    sprintf(buf, "set term color selection %s %s\n",
            entry->_color_select_fg.Name(),
            entry->_color_select_bg.Name()
            );
    OUTFILE(buf);

    sprintf(buf, "set term color underline %s %s\n",
            entry->_color_underline_fg.Name(),
            entry->_color_underline_bg.Name()
            );
    OUTFILE(buf);

    /* Terminal Scrollback */
    sprintf( buf, "set term scrollback %d\n", entry->_scrollback ) ;
    OUTFILE(buf);

    OUTFILE("set term remote-char ");
    switch (entry->_term_charset) {     /* Terminal character-set */
    case T_ARABIC_ISO:
        OUTFILE("arabic-iso");
        break;
      case T_ASCII:
        OUTFILE("ascii");
        break;
      case T_BRITISH:
        OUTFILE("british");
        break;
      case T_CANADIAN_FRENCH:
        OUTFILE("canadian-french");
        break;
      case T_CP437:
        OUTFILE("cp437");
        break;
      case T_CP850:
        OUTFILE("cp850");
        break;
      case T_CP852:
        OUTFILE("cp852");
        break;
    case T_CP855:
        OUTFILE("cp855");
        break;
    case T_CP856:
        OUTFILE("cp856");
        break;
    case T_CP857:
        OUTFILE("cp857");
        break;
      case T_CP862_HEBREW:
        OUTFILE("cp862");
        break;
    case T_CP864:
        OUTFILE("cp864");
        break;
      case T_CP866_CYRILLIC:
        OUTFILE("cp866");
        break;
    case T_CP869:
        OUTFILE("cp869");
        break;
      case T_CYRILLIC_ISO:
        OUTFILE("cyrillic");
        break;
      case T_DANISH:
        OUTFILE("danish");
        break;
      case T_DEC_MULTINATIONAL:
        OUTFILE("dec-multinational");
        break;
      case T_DG_INTERNATIONAL:
        OUTFILE("dg-international");
        break;
      case T_DUTCH:
        OUTFILE("dutch");
        break;
      case T_FINNISH:
        OUTFILE("finnish");
        break;
      case T_FRENCH:
        OUTFILE("french");
        break;
      case T_GERMAN:
        OUTFILE("german");
        break;
      case T_HEBREW_7:
        OUTFILE("hebrew-7");
        break;
      case T_HEBREW_ISO:
        OUTFILE("hebrew-iso");
        break;
      case T_HP_ROMAN_8:
        OUTFILE("hproman8");
        break;
      case T_HUNGARIAN:
        OUTFILE("hungarian");
        break;
      case T_ITALIAN:
        OUTFILE("italian");
        break;
    case T_JAPANESE_ROMAN:
        OUTFILE("japanese-roman");
        break;
    case T_KATAKANA:
        OUTFILE("katakana");
        break;
      case T_KIO8:
        OUTFILE("kio8");
        break;
      case T_LATIN1_ISO:
        OUTFILE("latin1");
        break;
      case T_LATIN2_ISO:
        OUTFILE("latin2");
        break;
    case T_LATIN3_ISO:
        OUTFILE("latin3");
        break;
    case T_LATIN4_ISO:
        OUTFILE("latin4");
        break;
    case T_LATIN5_ISO:
        OUTFILE("latin5");
        break;
    case T_LATIN6_ISO:
        OUTFILE("latin6");
        break;
      case T_MACINTOSH_LATIN:
        OUTFILE("macintosh");
        break;
      case T_NEXT_MULTINATIONAL:
        OUTFILE("next");
        break;
      case T_NORWEGIAN:
        OUTFILE("norwegian");
        break;
      case T_PORTUGUESE:
        OUTFILE("portuguese");
        break;
      case T_SHORT_KOI:
        OUTFILE("short-koi");
        break;
      case T_SPANISH:
        OUTFILE("spanish");
        break;
      case T_SWEDISH:
        OUTFILE("swedish");
        break;
      case T_SWISS:
        OUTFILE("swiss");
        break;
      case T_TRANSPARENT:
        OUTFILE("transparent");
        break;
    case  T_DEC_SPECIAL:
        OUTFILE("dec-special");
        break;
    case  T_DEC_TECHNICAL:
        OUTFILE("dec-technical");
        break;
    case T_DG_SPECIAL:
        OUTFILE("dg-specialgraphics");
        break;
    case T_DG_LINEDRAW:
        OUTFILE("dg-linedrawing");
        break;
    case T_DG_WORDPROC:
        OUTFILE("dg-wordprocessing");
        break;
    case T_ELOT927_GREEK:
        OUTFILE("elot927-greek");
        break;
    case T_GREEK_ISO:
        OUTFILE("greek-iso");
        break;
    case T_CP1250:
        OUTFILE("cp1250");
        break;
    case T_CP1251:
        OUTFILE("cp1251");
        break;
    case T_CP1252:
        OUTFILE("cp1252");
        break;
    case T_CP1253:
        OUTFILE("cp1253");
        break;
    case T_CP1254:
        OUTFILE("cp1254");
        break;
    case T_CP1257:
        OUTFILE("cp1257");
        break;
    case T_QNX_CONSOLE:
        OUTFILE("qnx-console");
        break;
    case T_SNI_BRACKETS:
        OUTFILE("sni-brackets");
        break;
    case T_SNI_EURO:
        OUTFILE("sni-euro");
        break;
    case T_SNI_FACET:
        OUTFILE("sni-facet");
        break;
    case T_SNI_IBM:
        OUTFILE("sni-ibm");
        break;
    case T_HP_MATH:
        OUTFILE("hp-line-drawing");
        break;
    case T_HP_LINE:
        OUTFILE("hp-math/technical");
        break;
    case T_CP819:
        OUTFILE("cp819");
        break;
    case T_CP912:
        OUTFILE("cp912");
        break;
    case T_CP913 :
        OUTFILE("cp913");
        break;
    case T_CP914:
        OUTFILE("cp914");
        break;
    case T_CP915:
        OUTFILE("cp915");
        break;
    case T_CP1089:
        OUTFILE("cp1089");
        break;
    case T_CP813:
        OUTFILE("cp813");
        break;
    case T_CP916:
        OUTFILE("cp916");
        break;
    case T_CP920:
        OUTFILE("cp920");
        break;
    case T_CP1051:
        OUTFILE("cp1051");
        break;
    case T_CP858:
        OUTFILE("cp858");
        break;
    case T_8859_15:
        OUTFILE("latin9-iso");
        break;
    case T_CP923:
        OUTFILE("cp923");
        break;
    case T_ELOT928:
        OUTFILE("elot928-greek");
        break;
    case T_CP10000:
        OUTFILE("cp10000");
        break;
    case T_CP37:
        OUTFILE("cp037");
        break;
    case T_CP1255:
        OUTFILE("cp1255");
        break;
    case T_CP1256:
        OUTFILE("cp1256");
        break;
    case T_CP1258:
        OUTFILE("cp1258");
        break;
    case T_UTF8:
        OUTFILE("utf8");
        break;
    case T_KOI8_U:
        OUTFILE("koi8u");
        break;
    case T_KOI8_R:
        OUTFILE("koi8r");
        break;
    case T_MAZOVIA:
        OUTFILE("mazovia-pc");
        break;
    case T_SNI_BLANKS:
        OUTFILE("sni-blanks");
        break;
    case T_UTF7:
        OUTFILE("utf7");
        break;
    }
    switch ( entry->_terminal ) {
    case SCOANSI:
    case AT386:
    case ANSI:
	if ( entry->_term_charset != T_TRANSPARENT ) {
	    OUTFILE(" g1\n");
	}
	else
	    OUTFILE("\n");
	break;
    default:
	OUTFILE("\n");
	break;
    }

    /* Exit Setting */
    sprintf( buf, "set exit on-disconnect %s\n", 
             entry->_autoexit ? "on" : "off" ) ;
    OUTFILE(buf);

    /* Printer Settings */

    switch ( entry->_printer_type ) {
    case PrinterWindows:
        if ( !strcmp(entry->_printer_windows,"<default>") )
            sprintf( buf, "set printer /WINDOWS-QUEUE:\n") ;
        else
            BuildOutStr( buf, "set printer /WINDOWS-QUEUE:", entry->_printer_windows ) ;
	OUTFILE(buf);
        break;
    case PrinterPipe:
        if ( entry->_printer_dos[0] )
            sprintf( buf, "set printer /PIPE:{%s}\n", entry->_printer_dos ) ;
        else
            sprintf( buf, "set printer /NONE ; Pipe printer name not specified\n");
	OUTFILE(buf);
        break;
    case PrinterFile:
        if ( entry->_printer_dos[0] )
            sprintf( buf, "set printer /FILE:{%s}\n", entry->_printer_dos ) ;
        else
            sprintf(buf, "set printer /NONE ; File printer name not specified\n");
	OUTFILE(buf);
        break;
    case PrinterDOS: {
        char * flow;
        char * parity;

        switch (entry->_printer_parity) {           /* Parity */
        case NO_PARITY:
            parity = "none";
            break;
        case SPACE:
            parity = "space";
            break;
        case MARK:
            parity = "mark";
            break;
        case EVEN:
            parity = "even";
            break;
        case ODD:
            parity = "odd";
            break;
        }

        switch (entry->_printer_flow) {         /* Flow control */
        case NO_FLOW:
            flow = "none";
            break;
        case XON_XOFF:
            flow = "xon/xoff";
            break;
        case RTS_CTS:
            flow = "rts/cts";
            break;
        case AUTO_FLOW:
            flow = "auto";
            break;
        }

        sprintf( buf,
                 "set printer /DOS-DEVICE:{%s} %s/SPEED:%d /PARITY:%s /FLOW:%s\n",
                 entry->_printer_dos[0] ? entry->_printer_dos : "prn", 
                 entry->_printer_bidi?"/BIDIRECTIONAL ":"",
                 entry->_printer_speed, parity, flow ) ;

        OUTFILE(buf);
        break;
    }
    case PrinterNone:
	sprintf( buf, "set printer /NONE\n") ;
	OUTFILE(buf);
        break;
    default:
        if ( entry->_printer_dos[0] )
            sprintf( buf, "set printer {%s}\n", entry->_printer_dos ) ;
        else
            sprintf( buf, "set printer /NONE\n");
	OUTFILE(buf);
    }

    if ( entry->_printer_separator[0] ) {
        sprintf( buf, "set printer /JOB-HEADER-FILE:{%s}\n", entry->_printer_separator ) ;
        OUTFILE(buf);
    }

    if ( entry->_printer_formfeed ) {
        sprintf( buf, "set printer /END-OF-JOB-STRING:{\\012}\n" ) ;
        OUTFILE(buf);
    }
    else if ( entry->_printer_terminator[0] ) {
        sprintf( buf, "set printer /END-OF-JOB-STRING:{%s}\n", entry->_printer_terminator ) ;
        OUTFILE(buf);
    }

    sprintf( buf, "set printer /TIMEOUT:%d\n", entry->_printer_timeout ) ;
    OUTFILE(buf);

    if ( entry->_printer_ps ) {
        sprintf( buf, "set printer /POSTSCRIPT /WIDTH:%d /LENGTH:%d\n",
                 entry->_printer_width,entry->_printer_length);
        OUTFILE(buf);
    }

    OUTFILE( "set printer /CHARACTER-SET:" ) ;
    switch (entry->_printer_charset) {     /* Printer character-set */
    case T_ARABIC_ISO:
        OUTFILE("arabic-iso");
        break;
      case T_ASCII:
        OUTFILE("ascii");
        break;
      case T_BRITISH:
        OUTFILE("british");
        break;
      case T_CANADIAN_FRENCH:
        OUTFILE("canadian-french");
        break;
      case T_CP437:
        OUTFILE("cp437");
        break;
      case T_CP850:
        OUTFILE("cp850");
        break;
      case T_CP852:
        OUTFILE("cp852");
        break;
    case T_CP855:
        OUTFILE("cp855");
        break;
    case T_CP856:
        OUTFILE("cp856");
        break;
    case T_CP857:
        OUTFILE("cp857");
        break;
      case T_CP862_HEBREW:
        OUTFILE("cp862");
        break;
    case T_CP864:
        OUTFILE("cp864");
        break;
      case T_CP866_CYRILLIC:
        OUTFILE("cp866");
        break;
    case T_CP869:
        OUTFILE("cp869");
        break;
      case T_CYRILLIC_ISO:
        OUTFILE("cyrillic");
        break;
      case T_DANISH:
        OUTFILE("danish");
        break;
      case T_DEC_MULTINATIONAL:
        OUTFILE("dec-multinational");
        break;
      case T_DG_INTERNATIONAL:
        OUTFILE("dg-international");
        break;
      case T_DUTCH:
        OUTFILE("dutch");
        break;
      case T_FINNISH:
        OUTFILE("finnish");
        break;
      case T_FRENCH:
        OUTFILE("french");
        break;
      case T_GERMAN:
        OUTFILE("german");
        break;
      case T_HEBREW_7:
        OUTFILE("hebrew-7");
        break;
      case T_HEBREW_ISO:
        OUTFILE("hebrew-iso");
        break;
      case T_HP_ROMAN_8:
        OUTFILE("hproman8");
        break;
      case T_HUNGARIAN:
        OUTFILE("hungarian");
        break;
      case T_ITALIAN:
        OUTFILE("italian");
        break;
    case T_JAPANESE_ROMAN:
        OUTFILE("japanese-roman");
        break;
    case T_KATAKANA:
        OUTFILE("katakana");
        break;
      case T_KIO8:
        OUTFILE("kio8");
        break;
      case T_LATIN1_ISO:
        OUTFILE("latin1");
        break;
      case T_LATIN2_ISO:
        OUTFILE("latin2");
        break;
    case T_LATIN3_ISO:
        OUTFILE("latin3");
        break;
    case T_LATIN4_ISO:
        OUTFILE("latin4");
        break;
    case T_LATIN5_ISO:
        OUTFILE("latin5");
        break;
    case T_LATIN6_ISO:
        OUTFILE("latin6");
        break;
      case T_MACINTOSH_LATIN:
        OUTFILE("macintosh");
        break;
      case T_NEXT_MULTINATIONAL:
        OUTFILE("next");
        break;
      case T_NORWEGIAN:
        OUTFILE("norwegian");
        break;
      case T_PORTUGUESE:
        OUTFILE("portuguese");
        break;
      case T_SHORT_KOI:
        OUTFILE("short-koi");
        break;
      case T_SPANISH:
        OUTFILE("spanish");
        break;
      case T_SWEDISH:
        OUTFILE("swedish");
        break;
      case T_SWISS:
        OUTFILE("swiss");
        break;
      case T_TRANSPARENT:
        OUTFILE("transparent");
        break;
    case  T_DEC_SPECIAL:
        OUTFILE("dec-special");
        break;
    case  T_DEC_TECHNICAL:
        OUTFILE("dec-technical");
        break;
    case T_DG_SPECIAL:
        OUTFILE("dg-specialgraphics");
        break;
    case T_DG_LINEDRAW:
        OUTFILE("dg-linedrawing");
        break;
    case T_DG_WORDPROC:
        OUTFILE("dg-wordprocessing");
        break;
    case T_ELOT927_GREEK:
        OUTFILE("elot927-greek");
        break;
    case T_GREEK_ISO:
        OUTFILE("greek-iso");
        break;
    case T_CP1250:
        OUTFILE("cp1250");
        break;
    case T_CP1251:
        OUTFILE("cp1251");
        break;
    case T_CP1252:
        OUTFILE("cp1252");
        break;
    case T_CP1253:
        OUTFILE("cp1253");
        break;
    case T_CP1254:
        OUTFILE("cp1254");
        break;
    case T_CP1257:
        OUTFILE("cp1257");
        break;
    case T_QNX_CONSOLE:
        OUTFILE("qnx-console");
        break;
    case T_SNI_BRACKETS:
        OUTFILE("sni-brackets");
        break;
    case T_SNI_EURO:
        OUTFILE("sni-euro");
        break;
    case T_SNI_FACET:
        OUTFILE("sni-facet");
        break;
    case T_SNI_IBM:
        OUTFILE("sni-ibm");
        break;
    case T_HP_MATH:
        OUTFILE("hp-line-drawing");
        break;
    case T_HP_LINE:
        OUTFILE("hp-math/technical");
        break;
    case T_CP819:
        OUTFILE("cp819");
        break;
    case T_CP912:
        OUTFILE("cp912");
        break;
    case T_CP913 :
        OUTFILE("cp913");
        break;
    case T_CP914:
        OUTFILE("cp914");
        break;
    case T_CP915:
        OUTFILE("cp915");
        break;
    case T_CP1089:
        OUTFILE("cp1089");
        break;
    case T_CP813:
        OUTFILE("cp813");
        break;
    case T_CP916:
        OUTFILE("cp916");
        break;
    case T_CP920:
        OUTFILE("cp920");
        break;
    case T_CP1051:
        OUTFILE("cp1051");
        break;
    case T_CP858:
        OUTFILE("cp858");
        break;
    case T_8859_15:
        OUTFILE("latin9-iso");
        break;
    case T_CP923:
        OUTFILE("cp923");
        break;
    case T_ELOT928:
        OUTFILE("elot928-greek");
        break;
    case T_CP10000:
        OUTFILE("cp10000");
        break;
    case T_CP37:
        OUTFILE("cp037");
        break;
    case T_CP1255:
        OUTFILE("cp1255");
        break;
    case T_CP1256:
        OUTFILE("cp1256");
        break;
    case T_CP1258:
        OUTFILE("cp1258");
        break;
    case T_UTF8:
        OUTFILE("utf8");
        break;
    case T_KOI8_U:
        OUTFILE("koi8u");
        break;
    case T_KOI8_R:
        OUTFILE("koi8r");
        break;
    case T_MAZOVIA:
        OUTFILE("mazovia-pc");
        break;
    case T_SNI_BLANKS:
        OUTFILE("sni-blanks");
        break;
    case T_UTF7:
        OUTFILE("utf7");
        break;
    }
    OUTFILE("\n");

    /* File Settings */

    sprintf(tmp, "set file download-directory {%s}", entry->_startup_dir ) ;
    OUTFILE(BuildOutFileStr(buf, "", tmp));

    switch (entry->_xfermode) {         /* Binary / Text */
      case BINARY:
        OUTFILE("set file type binary\n");
        break;
      case TEXT:
        OUTFILE("set file type text\n");
        break;
    }

    switch (entry->_collision) {        /* File Collision */
      case COL_APPEND:
        OUTFILE("set file collision append\n");
        break;
      case COL_BACKUP:
        OUTFILE("set file collision backup\n");
        break;
      case COL_DISCARD:
        OUTFILE("set file collision discard\n");
        break;
      case COL_OVERWRITE:
        OUTFILE("set file collision overwrite\n");
        break;
      case COL_RENAME:
        OUTFILE("set file collision rename\n");
        break;
      case COL_UPDATE:
        OUTFILE("set file collision update\n");
        break;
    }

    if (entry->_keep_incomplete) {        /* File Incomplete */
        OUTFILE("set file incomplete auto\n");
    } else {
        OUTFILE("set file incomplete discard\n");
    }

    if (entry->_streaming) {        /* STREAMING */
        OUTFILE("set streaming auto\n");
    } else {
        OUTFILE("set streaming off\n");
    }

    if (!entry->_clear_channel) {        /* CLEAR-CHANNEL */
        OUTFILE("set clear-channel auto\n");
    } else {
        OUTFILE("set clear-channel off\n");
    }

    if (entry->_fname_literal) {        /* File Names */
        OUTFILE("set file names literal\n");
    } else {
        OUTFILE("set file names converted\n");
    }

    if (entry->_pathnames) {            /* Send/Receive Pathnames */
        OUTFILE("set receive pathnames on\n");
        OUTFILE("set send pathnames on\n");
    } else {
        OUTFILE("set receive pathnames off\n");
        OUTFILE("set send pathnames off\n");
    }

    switch (entry->_file_charset) {     /* File character-set */
      case F_ASCII:
        OUTFILE("set file char ascii\n");
        break;
      case F_BRITISH:
        OUTFILE("set file char british\n");
        break;
      case F_CANADIAN_FRENCH:
        OUTFILE("set file char canadian-french\n");
        break;
      case F_CP437:
        OUTFILE("set file char cp437\n");
        break;
      case F_CP850:
        OUTFILE("set file char cp850\n");
        break;
      case F_CP852:
        OUTFILE("set file char cp852\n");
        break;
      case F_CP862_HEBREW:
        OUTFILE("set file char cp862\n");
        break;
      case F_CP866_CYRILLIC:
        OUTFILE("set file char cp866\n");
        break;
      case F_CYRILLIC_ISO:
        OUTFILE("set file char cyrillic\n");
        break;
      case F_DANISH:
        OUTFILE("set file char danish\n");
        break;
      case F_DEC_KANJI:
        OUTFILE("set file char dec-kanji\n");
        break;
      case F_DEC_MULTINATIONAL:
        OUTFILE("set file char dec-multinational\n");
        break;
      case F_DG_INTERNATIONAL:
        OUTFILE("set file char dg-international\n");
        break;
      case F_DUTCH:
        OUTFILE("set file char dutch\n");
        break;
    case F_ELOT927_GREEK:
          OUTFILE("set file char elot927-greek\n");
          break;
    case F_FINNISH:
        OUTFILE("set file char finnish\n");
        break;
      case F_FRENCH:
        OUTFILE("set file char french\n");
        break;
      case F_GERMAN:
        OUTFILE("set file char german\n");
        break;
    case F_GREEK_ISO:
          OUTFILE("set file char greek-iso\n");
          break;
    case F_HEBREW_7:
        OUTFILE("set file char hebrew-7\n");
        break;
      case F_HEBREW_ISO:
        OUTFILE("set file char hebrew-iso\n");
        break;
      case F_HP_ROMAN_8:
        OUTFILE("set file char hproman8\n");
        break;
      case F_HUNGARIAN:
        OUTFILE("set file char hungarian\n");
        break;
      case F_ITALIAN:
        OUTFILE("set file char italian\n");
        break;
      case F_JAPAN_EUC:
        OUTFILE("set file char japanese-euc\n");
        break;
      case F_JIS7_KANJI:
        OUTFILE("set file char jis7-kanji\n");
        break;
      case F_KIO8_CYRILLIC:
        OUTFILE("set file char kio8\n");
        break;
      case F_LATIN1_ISO:
        OUTFILE("set file char latin1\n");
        break;
      case F_LATIN2_ISO:
        OUTFILE("set file char latin2\n");
        break;
      case F_MACINTOSH_LATIN:
        OUTFILE("set file char macintosh\n");
        break;
      case F_NEXT_MULTINATIONAL:
        OUTFILE("set file char next\n");
        break;
      case F_NORWEGIAN:
        OUTFILE("set file char norwegian\n");
        break;
      case F_PORTUGUESE:
        OUTFILE("set file char portuguese\n");
        break;
      case F_SHIFT_JIS_KANJI:
        OUTFILE("set file char shift-jis-kanji\n");
        break;
      case F_SHORT_KOI:
        OUTFILE("set file char short-koi\n");
        break;
      case F_SPANISH:
        OUTFILE("set file char spanish\n");
        break;
      case F_SWEDISH:
        OUTFILE("set file char swedish\n");
        break;
      case F_SWISS:
        OUTFILE("set file char swiss\n");
        break;

    case F_CP1250:
        OUTFILE("set file char cp1250\n");
        break;
    case F_CP1251:
        OUTFILE("set file char cp1251\n");
        break;
    case F_CP1252:
        OUTFILE("set file char cp1252\n");
        break;
    case F_CP855:
        OUTFILE("set file char cp855\n");
        break;
    case F_CP858:
        OUTFILE("set file char cp858\n");
        break;
    case F_ELOT928_GREEK:
        OUTFILE("set file char elot928-greek\n");
        break;
    case F_EUC_JP:
        OUTFILE("set file char euc-jp\n");
        break;
    case F_ISO2022_JP:
        OUTFILE("set file char iso2022jp-kanji\n");
        break;
    case F_KOI8_R:
        OUTFILE("set file char koi8r\n");
        break;
    case F_KOI8_U:
        OUTFILE("set file char koi8u\n");
        break;
    case F_LATIN9_ISO:
          OUTFILE("set file char latin9-iso\n");
          break;
    case F_MAZOVIA_PC:
        OUTFILE("set file char mazovia-pc\n");
        break;
    case F_UCS2:
        OUTFILE("set file char ucs2\n");
        break;
    case F_UTF8:
        OUTFILE("set file char utf8\n");
        break;
    case F_BULGARIA_PC:
        OUTFILE("set file char bulgaria-pc\n");
        break;
    case F_CP869:
        OUTFILE("set file char cp869-greek\n");
        break;
    }

    /* Protocol settings */

    int ucc = 0;                        /* Output Unprefix CC selection */
    switch (entry->_protocol) {         /* Protocol selection */
    case K_FAST:
        OUTFILE("set protocol kermit\n");
        OUTFILE("do fast\n");
        break;
    case K_CUSTOM:
        /* Window size */
        sprintf(buf,"set window-size %d\n", entry->_max_windows);
        OUTFILE(buf);
        /* Packet length */
        sprintf(buf,"set receive packet-length %d\n", entry->_packet_length);
        OUTFILE(buf);
        ucc = 1;
        break;
    case K_CAUTIOUS:
        OUTFILE("set protocol kermit\n");
        OUTFILE("do fast\n");
        break;
    case K_ROBUST:
        OUTFILE("set protocol kermit\n");
        OUTFILE("do robust\n");
        break;
    case Z:
        OUTFILE("set protocol zmodem\n");
        /* Window size */
        sprintf(buf,"set window-size %d\n", entry->_max_windows*1024);
        OUTFILE(buf);
        ucc = 1;
        break;
    case Y:
        OUTFILE("set protocol ymodem\n");
        /* Packet length */
        sprintf(buf,"set send packet-length %d\n", entry->_packet_length);
        OUTFILE(buf);
        ucc = 1;
        break;
    case X:
        OUTFILE("set protocol xmodem\n");
        /* Packet length */
        sprintf(buf,"set send packet-length %d\n", entry->_packet_length);
        OUTFILE(buf);
        ucc = 1;
        break;
    case YG:
        OUTFILE("set protocol ymodem-g\n");
        /* Packet length */
        sprintf(buf,"set send packet-length %d\n", entry->_packet_length);
        OUTFILE(buf);
        ucc = 1;
        break;
    }

    if ( ucc ) {
        switch (entry->_unprefix_cc) {      /* Control-character (Un)prefixing */
        case NEVER:
            OUTFILE("set prefixing all\n");
            break;
        case CAUTIOUSLY:
            OUTFILE("set prefixing cautious\n");
            break;
        case MOST:
            OUTFILE("set prefixing minimal\n");
            break;
        }
    }

    switch (entry->_xfer_charset) {     /* Transfer character-set */
    case X_ASCII:
        OUTFILE("set xfer char ascii\n");
        break;
    case X_CYRILLIC_ISO:
        OUTFILE("set xfer char cyrillic\n");
        break;
    case X_GREEK_ISO:
          OUTFILE("set xfer char greek-iso\n");
          break;
    case X_HEBREW_ISO:
        OUTFILE("set xfer char hebrew-iso\n");
        break;
    case X_JAPANESE_EUC:
        OUTFILE("set xfer char japanese-euc\n");
        break;
    case X_LATIN1_ISO:
        OUTFILE("set xfer char latin1\n");
        break;
    case X_LATIN2_ISO:
        OUTFILE("set xfer char latin2\n");
        break;
    case X_TRANSPARENT:
        OUTFILE("set xfer char transparent\n");
        break;
    case X_EUC_JP:
        OUTFILE("set xfer char euc-jp\n");
        break;
    case X_LATIN9_ISO:
        OUTFILE("set xfer char latin9-iso\n");
        break;
    case X_UCS2:
        OUTFILE("set xfer char ucs2\n");
        break;
    case X_UTF8:
        OUTFILE("set xfer char utf8\n");
        break;
    }

    if (entry->_mouse) {                /* Mouse */
        OUTFILE("set mouse activate on\n");
    } else {
        OUTFILE("set mouse activate off\n");
    }

    switch (entry->_enter) {            /* Terminal newline-mode */
    case CR:
        OUTFILE("set key \\269  \\13\n");
        OUTFILE("set term newline off\n");
        break;
    case CRLF:
        OUTFILE("set key \\269  \\13\n");
        OUTFILE("set term newline on\n");
        break;
    case LF:
        OUTFILE("set key \\269  \\10\n");
        OUTFILE("set term newline off\n");
        break;
    }

    /* Use the new backhanded SET DIALER BACKSPACE command to set */
    /* the value of the Backspace key, so that it can be done in  */
    /* a terminal type specific manner.  This command does the    */
    /* equivalent of SET TERM KEY except in the case of WYSE and  */
    /* TVI terminals which by default have a Kverb assigned.  So  */
    /* for those two terminal families we must manipulate the UDK */
    /* table.   (What a hack!!!)                                  */

    switch (entry->_backspace) {        	/* Backspace key */
    case CTRL_H:                      	
        OUTFILE("set dialer backspace \\8\n");  
        break;
    case DEL:
        OUTFILE("set dialer backspace \\127\n");
        break;
    case CTRL_Y:
	OUTFILE("set dialer backspace \\25\n");  
	break;
    }

    /* The keymap file must take precedence over the setting here  */
    /* because otherwise there would be no way to set backspace to */
    /* a value other than BS or DEL                                */

    switch (entry->_keymap) {           /* Key map */
    case VT100_KEY: {
        int i=0,j=0,goteol=0;
        int len = strlen(entry->_keyscript);
        for ( ;i<len;i++,j++ ) {
            switch ( entry->_keyscript[i] ) {
            case 13:
                if ( goteol ) {
                    j--;
                    continue;
                }
                if ( entry->_keyscript[i+1] == 10 )
                    i++;
                buf[j] = 10;
                goteol = 1;
                break;
            case 10:
                if ( goteol ) {
                    j--;
                    continue;
                }
                buf[j] = 10;
                goteol = 1;
                break;
            default:
                buf[j] = entry->_keyscript[i];
                goteol = 0;
            }
        }
        if ( !goteol )
            buf[j++] = 10;
        buf[j] = 0;
        OUTFILE(buf);
        break;                          /* Nothing to do */
    }
    case MAPFILE:
        if ( entry->_keymap_file[0] ) {
            sprintf(buf,"take {%s}\n", entry->_keymap_file);
            OUTFILE(buf);
        }
        break;
    }

    /* Set the LOGIN parameters */

    if ( entry->_userid ) {
        OUTFILE("set command quoting off\n");
        sprintf(buf, "set login userid %s\n", entry->_userid);
        OUTFILE(buf);
        OUTFILE("set command quoting on\n");
    }

    int anonymous = !stricmp(entry->_userid,"anonymous") ||
        !stricmp(entry->_userid,"ftp");

    if ( entry->_password && !anonymous ) {
        if ( !entry->_prompt_for_password )
            sprintf(tmp, "set login password {%s}", entry->_password );
        else {
            /* Prompt for Password */
            ZIL_ICHAR    password[PASSWD_SZ];
            ZIL_UINT8    ok=0;
            K_PASSWORD * pword = new K_PASSWORD( this, password, &ok );
            if ( !pword )
                OutofMemory("Unable to create PASSWORD");
            pword->Control();
            delete pword;
            if ( !ok ) {
                return(FALSE);
            }
            ck_encrypt(password);
            sprintf(tmp, "set login password {%s}", password );
        }
        OUTFILE("set command quoting off\n");
        OUTFILE(BuildOutStr(buf, "", tmp));
        OUTFILE("set command quoting on\n");
    }
    if ( entry->_prompt ) {
        sprintf(tmp, "set login prompt {%s}", entry->_prompt);
        OUTFILE(BuildOutStr(buf, "", tmp));
    }

    /* Set up the connection */

    BOOL DefDevice = (def_entry != NULL) ;
    switch (entry->_access) {
      case DIRECT:                      /* Direct serial */
        if (entry->_carrier) {
            OUTFILE("set carrier auto\n");
        } else {
            OUTFILE("set carrier off\n");
        }

         /* If _line_device is DEFAULT, then we use the
            values from def_entry instead of entry 
         */

	  if ( entry->_tapi_line ) 
	      sprintf(buf, "set tapi modem-dialing off\nset tapi line %s\n", 
		     entry->_line_device);
	else
	    sprintf(buf, "set port %s\n", entry->_line_device);

        OUTFILE(buf);
        sprintf(buf, "set speed %ld\n", entry->_speed);
        OUTFILE(buf);

	  switch (entry->_parity) {           /* Parity */
	  case NO_PARITY:
	      OUTFILE("set parity none\n");
	      break;
	  case SPACE:
	      OUTFILE("set parity space\n");
	      break;
	  case MARK:
	      OUTFILE("set parity mark\n");
	      break;
	  case EVEN:
	      OUTFILE("set parity even\n");
	      break;
	  case ODD:
	      OUTFILE("set parity odd\n");
	      break;
          case SPACE_8:
              OUTFILE("set parity hardware space\n");
              break;
          case MARK_8:
              OUTFILE("set parity hardware mark\n");
              break;
          case EVEN_8:
              OUTFILE("set parity hardware even\n");
              break;
          case ODD_8:
              OUTFILE("set parity hardware odd\n");
              break;
	  }

          switch (entry->_stopbits) {           /* StopBits */
          case STOPBITS_1_0:
              OUTFILE("set stop-bits 1\n");
              break;
          case STOPBITS_2_0:
              OUTFILE("set stop-bits 2\n");
              break;
          }

	  switch (entry->_flow) {         /* Flow control */
	  case NO_FLOW:
	      OUTFILE("set flow none\n");
	      break;
	  case XON_XOFF:
	      OUTFILE("set flow xon/xoff\n");
	      break;
	  case RTS_CTS:
	      OUTFILE("set flow rts/cts\n");
	      break;
	  case AUTO_FLOW:
	      OUTFILE("set flow auto\n");
	    break;
        }
        break;

    case PHONE: {                      /* Dialed serial */
	K_MODEM * modem = FindModem( entry->_modem );

#if defined(WIN32)
	if ( config->_tapi_conversions == ConvWindows && TapiAvail )	{
	    OUTFILE("set tapi phone-number-conversions on\n");
	}
	else if ( config->_tapi_conversions == ConvKermit || !TapiAvail ) {
	    OUTFILE("set tapi phone-number-conversions off\n");
	}
	else {
	    OUTFILE("set tapi phone-number-conversions auto\n");
	}

	if ( config->_tapi_dialing == DialWindows && TapiAvail ) {
	    OUTFILE("set tapi modem-dialing on\n");
	}
	else {
	    OUTFILE("set tapi modem-dialing off\n");
	}
#endif
	GenerateModem( modem, entry, OutFile );
	GenerateLocation( location, OutFile );

        /* List of dialing directory files (phone lists) */

        if (config->_use_phone_dir) {   /* Semicolon-separated list */
            char *s;
            s = p = (char *) config->_phone_dir;
           /* We use loop just in case */
            while (*p) {
                if (*p == ';')
                  *p = (char) 32;
                p++;
            }
            sprintf(buf, "set dial directory %s\n", s);
            OUTFILE(buf);
        } else {
            OUTFILE("set dial directory\n");
        }

        if (config->_confirm_number) {
            OUTFILE("set dial confirm on\n");
        } else {
            OUTFILE("set dial confirm off\n");
        }
#ifdef COMMENT
        if (config->_convert_dir) {
            OUTFILE("set dial convert on\n");
        } else {
            OUTFILE("set dial convert off\n");
        }
#endif /* COMMENT */
        if (config->_display_dial) {
            OUTFILE("set dial display on\n");
        } else {
            OUTFILE("set dial display off\n");
        }
        if (config->_hangup) {
            OUTFILE("set dial hangup on\n");
        } else {
            OUTFILE("set dial hangup off\n");
        }
        if (config->_sort_dir) {
            OUTFILE("set dial sort on\n");
        } else {
            OUTFILE("set dial sort off\n");
        }
        break;
    }

    case TCPIP:
          if (config->_use_network_dir) { /* Semicolon-separated list */
              char *s;
              s = p = (char *) config->_network_dir;
           /* we use loop just in case */
              while (*p) {
                  if (*p == ';')
                      *p = (char) 32;
                  p++;
              }
              sprintf(buf, "set network directory %s\n", s);
              OUTFILE(buf);
          } else {
              OUTFILE("set network directory\n");
          }
              
	  sprintf(buf, "set network tcp/ip\n" ) ;
	  OUTFILE(buf);
	  OUTFILE("if fail end 1 TCP/IP Failed\n");


          ZIL_ICHAR * port, * proto;
          switch ( entry->_tcpproto ) {
          case TCP_DEFAULT:
              port = "telnet";
              proto = "/default";
              break;
          case TCP_EK4LOGIN:
              port = "eklogin";
              proto = "/ek4login";
              break;
          case TCP_EK5LOGIN:
              port = "eklogin";
              proto = "/ek5login";
              break;
          case TCP_K4LOGIN:
              port = "klogin";
              proto = "/k4login";
              break;
          case TCP_K5LOGIN:
              port = "klogin";
              proto = "/k5login";
              break;
          case TCP_RAW:
              port = "telnet";
              proto = "/raw-socket";
              break;
          case TCP_RLOGIN:
              port = "login";
              proto = "/rlogin";
              break;
          case TCP_TELNET:
              port = "telnet";
              proto = "/telnet";
              break;
          case TCP_TELNET_NO_INIT:
              port = "telnet";
              proto = "/no-telnet-init";
              break;
          case TCP_TELNET_SSL:
              port = "telnets";
              proto = "/ssl-telnet";
              break;
          case TCP_TELNET_TLS:
              port = "telnets";
              proto = "/tls-telnet";
              break;
          case TCP_SSL:
              port = "https";
              proto = "/ssl";
              break;
          case TCP_TLS:
              port = "https";
              proto = "/tls";
              break;
          }

          {
              ZIL_ICHAR tmp[512], *p;
              p = entry->_ipaddress;
              while ( *p == ' ' )
                  p++;
              strncpy(tmp,p,512);
              tmp[511] = 0;
              p = tmp + strlen(tmp) - 1;
              if ( p < tmp ) {
                  sprintf(buf,"end 1 No host provided\n");
              } else {
                  while ( *p == ' ' ) {
                      *p = '\0';
                      p--;
                  }

                  if ( strchr(entry->_ipaddress,':') ||
                       strchr(entry->_ipaddress,' ') )
                      sprintf(buf, "set host %s\n",entry->_ipaddress);
                  else
                      sprintf(buf, "set host %s %s %s\n", 
                               entry->_ipaddress,
                               entry->_ipport[0]?entry->_ipport:port,
                               proto);
              }
              OUTFILE(buf);
          }
        break;

    case SSH: {
        ZIL_ICHAR tmp[512], *p;

        if (config->_use_network_dir) { /* Semicolon-separated list */
            char *s;
            s = p = (char *) config->_network_dir;
            /* we use loop just in case */
            while (*p) {
                if (*p == ';')
                    *p = (char) 32;
                p++;
            }
            sprintf(buf, "set network directory %s\n", s);
            OUTFILE(buf);
        } else {
            OUTFILE("set network directory\n");
        }
        OUTFILE("set network type ssh\n" ) ;
        OUTFILE("if fail end 1 SSH Failed\n");

        switch ( entry->_sshproto ) {
        case SSH_AUTO:
            OUTFILE("set ssh version automatic\n");
            break;
        /*case SSH_V1:
            OUTFILE("set ssh version 1\n");
            break;*/
        case SSH_V2:
            OUTFILE("set ssh version 2\n");
            break;
        }     

        if ( entry->_ssh_compress ) {
            OUTFILE("set ssh compression on\n");
        } else {
            OUTFILE("set ssh compression off\n");
        }
        if ( entry->_ssh_x11 ) {
            OUTFILE("set ssh x11-forwarding on\n");
        } else {
            OUTFILE("set ssh x11-forwarding off\n");
        }
        /*switch ( entry->_ssh1_cipher ) {
        case SSH1_CIPHER_3DES:
            OUTFILE("set ssh v1 cipher 3des\n");
            break;
        case SSH1_CIPHER_BLOWFISH:
            OUTFILE("set ssh v1 cipher blowfish\n");
            break;
        case SSH1_CIPHER_DES:   
            OUTFILE("set ssh v1 cipher des\n");
            break;
        default:
            OUTFILE("end 1 unknown set ssh v1 cipher\n");
        }*/

        switch ( entry->_ssh_host_check ) {
        case HC_STRICT:
            OUTFILE("set ssh strict-host-key-check on\n");
            break;
        case HC_ASK:
            OUTFILE("set ssh strict-host-key-check ask\n");
            break;
        case HC_NONE:
            OUTFILE("set ssh strict-host-key-check off\n");
            break;
        default:
            OUTFILE("end 1 unknown set strict-host-key-check\n");
        }

        if ( entry->_ssh1_id_file[0] && entry->_ssh2_id_file[0] )
        {
            sprintf(tmp, "set ssh identity-file {%s} {%s}", 
                     entry->_ssh1_id_file, entry->_ssh2_id_file ) ;
            OUTFILE(BuildOutFileStr(buf, "", tmp));
        } else if ( entry->_ssh1_id_file[0] ) {
            sprintf(tmp, "set ssh identity-file {%s}", 
                     entry->_ssh1_id_file ) ;
            OUTFILE(BuildOutFileStr(buf, "", tmp));

        } else if ( entry->_ssh2_id_file[0] ) {
            sprintf(tmp, "set ssh identity-file {%s}", 
                     entry->_ssh2_id_file ) ;
            OUTFILE(BuildOutFileStr(buf, "", tmp));
        }

        /*if ( entry->_ssh1_kh_file[0] ) {
            sprintf(tmp, "set ssh v1 user-known-hosts-file {%s}", 
                     entry->_ssh1_kh_file ) ;
            OUTFILE(BuildOutFileStr(buf, "", tmp));
        }*/

        if ( entry->_ssh2_kh_file[0] ) {
            sprintf(tmp, "set ssh v2 user-known-hosts-file {%s}", 
                     entry->_ssh2_kh_file ) ;
            OUTFILE(BuildOutFileStr(buf, "", tmp));
        }

        /* The following is no longer supported as of Kermit 95 v3.0:
         *   external-keyx, srp-gex-sha1, hostbased
         * sprintf(buf,"set ssh v2 authentication %s%s%s%s%s%s%s\n",
                 entry->_ssh2_auth_external_keyx ? "external-keyx " : "",
                 entry->_ssh2_auth_gssapi ? "gssapi " : "",
                 entry->_ssh2_auth_srp_gex_sha1 ? "srp-gex-sha1 " : "",
                 entry->_ssh2_auth_publickey ? "publickey " : "",
                 entry->_ssh2_auth_password ? "password " : "",
                 entry->_ssh2_auth_keyboard_interactive ? "keyboard-interactive " : "",
                 entry->_ssh2_auth_hostbased ? "hostbased " : "");*/
        sprintf(buf,"set ssh v2 authentication %s%s%s%s\n",
                entry->_ssh2_auth_gssapi ? "gssapi " : "",
                entry->_ssh2_auth_publickey ? "publickey " : "",
                entry->_ssh2_auth_password ? "password " : "",
                entry->_ssh2_auth_keyboard_interactive ? "keyboard-interactive " : "");
        OUTFILE(buf);

        /* Changes for Kermit 95 v3.0 (v2.1):
         * Removed: blowfish-cbc, cast128-cbc, arcfour
         * Retained (for now): aes128-cbc, 3des-cbc, aes192-cbc, aes256-cbc
         * New: aes128-ctr              aes256-gcm@openssh.com
         *      aes192-ctr              chachae20-poly1305
         *      aes256-ctr              aes128-gcm@openssh.com
         */
        sprintf(buf,"set ssh v2 ciphers %s%s%s%s%s%s%s%s%s%s\n",
                 entry->_ssh2_cipher_chachae20_poly1305 ? "chachae20-poly1305 " : "",
                 entry->_ssh2_cipher_aes256_gcm_openssh ? "aes256-gcm@openssh.com " : "",
                 entry->_ssh2_cipher_aes128_gcm_openssh ? "aes128-gcm@openssh.com " : "",
                 entry->_ssh2_cipher_aes256ctr ? "aes256-ctr " : "",
                 entry->_ssh2_cipher_aes192ctr ? "aes192-ctr " : "",
                 entry->_ssh2_cipher_aes128ctr ? "aes128-ctr " : "",
                 entry->_ssh2_cipher_aes256 ? "aes256-cbc " : "", /* old */
                 entry->_ssh2_cipher_aes192 ? "aes192-cbc " : "", /* old */
                 entry->_ssh2_cipher_aes128 ? "aes128-cbc " : "", /* old */
                 entry->_ssh2_cipher_3des ? "3des-cbc " : "" /* old */
                 //entry->_ssh2_cipher_blowfish ? "blowfish-cbc " : "", /* gone */
                 //entry->_ssh2_cipher_cast128 ? "cast128-cbc " : "", /* gone */
                 //entry->_ssh2_cipher_arcfour ? "arcfour " : "", /* gone */
                 );
        OUTFILE(buf);

        sprintf(buf,"set ssh v2 macs %s%s%s%s%s%s%s\n",
                 entry->_ssh2_mac_sha2_256_etm_openssh ? "hmac-sha2-256-etm@openssh.com " : "",
                 entry->_ssh2_mac_sha2_512_etm_openssh ? "hmac-sha2-512-etm@openssh.com " : "",
                 entry->_ssh2_mac_sha1_etm_openssh ? "hmac-sha1-etm@openssh.com " : "",
                 entry->_ssh2_mac_sha2_256 ? "hmac-sha2-256 " : "",
                 entry->_ssh2_mac_sha2_512 ? "hmac-sha2-512 " : "",
                 entry->_ssh2_mac_sha1 ? "hmac-sha1 " : "", /* old */
                 entry->_ssh2_mac_none ? "none " : ""

                //entry->_ssh2_mac_md5 ? "hmac-md5 " : "", /* gone */
                 //entry->_ssh2_mac_ripemd160 ? "hmac-ripemd160 " : "", /* gone */
                 //entry->_ssh2_mac_sha1_96 ? "hmac-sha1-96 " : "", /* gone */
                 //entry->_ssh2_mac_md5_96 ? "hmac-md5-96 " : "" /* gone */
                 );
        OUTFILE(buf);

        sprintf(buf, "set ssh v2 hostkey-algorithms %s%s%s%s%s%s%s%s\n",
                 entry->_ssh2_hka_ssh_ed25519 ? "ssh-ed25519 " : "",
                 entry->_ssh2_hka_ecdsa_sha2_nistp521 ? "ecdsa-sha2-nistp521 " : "",
                 entry->_ssh2_hka_ecdsa_sha2_nistp384 ? "ecdsa-sha2-nistp384 " : "",
                 entry->_ssh2_hka_ecdsa_sha2_nistp256 ? "ecdsa-sha2-nistp256 " : "",
                 entry->_ssh2_hka_rsa_sha2_512 ? "rsa-sha2-512 " : "",
                 entry->_ssh2_hka_rsa_sha2_256 ? "rsa-sha2-256 " : "",
                 entry->_ssh2_hka_rsa ? "ssh-rsa " : "",  /* Old */
                 entry->_ssh2_hka_dss ? "ssh-dss " : ""); /* Deprecated */
        OUTFILE(buf);

        /* New in v3.0 */
        /* TODO: Build a UI to configure this
         *  Until then we'll comment out the command so as to not override the
         *  defaults */
        sprintf(buf,"; set ssh v2 key-exchange-methods %s\n",
                entry->_ssh2_kex_curve25519_sha256 ? "curve25519-sha256 " : "",
                entry->_ssh2_kex_curve25519_sha256_libssh ? "curve25519-sha256@libssh.org " : "",
                entry->_ssh2_kex_ecdh_sha2_nistp256 ? "ecdh-sha2-nistp256 " : "",
                entry->_ssh2_kex_ecdh_sha2_nistp384 ? "ecdh-sha2-nistp384 " : "",
                entry->_ssh2_kex_ecdh_sha2_nistp521 ? "ecdh-sha2-nistp521 " : "",
                entry->_ssh2_kex_dh_group18_sha512 ? "diffie-hellman-group18-sha512 " : "",
                entry->_ssh2_kex_dh_group16_sha512 ? "diffie-hellman-group16-sha512 " : "",
                entry->_ssh2_kex_dh_group_exchange_sha256 ? "diffie-hellman-group-exchange-sha256 " : "",
                entry->_ssh2_kex_dh_group14_sha256 ? "diffie-hellman-group14-sha256 " : "",
                entry->_ssh2_kex_dh_group14_sha1 ? "diffie-hellman-group14-sha1 " : "",
                entry->_ssh2_kex_dh_group1_sha1 ? "diffie-hellman-group1-sha1 " : "",
                entry->_ssh2_kex_ext_info_c ? "ext-info-c " : "",
                entry->_ssh2_kex_dh_group_exchange_sha1 ? "diffie-hellman-group-exchange-sha1 " : ""
        );
        OUTFILE(buf);

        if ( entry->_ssh_credfwd && entry->_ssh2_auth_gssapi &&
             entry->_sshproto != SSH_V1 ) 
        {
            OUTFILE("set ssh gssapi delegate-credentials on\n");
        } else {
            OUTFILE("set ssh gssapi delegate-credentials off\n");
        }
        /*if ( entry->_ssh_credfwd && entry->_sshproto != SSH_V2 ) {
            OUTFILE("set ssh kerberos5 tgt-passing on\n");
            OUTFILE("set ssh kerberos4 tgt-passing on\n");
        } else {
            OUTFILE("set ssh kerberos5 tgt-passing off\n");
            OUTFILE("set ssh kerberos4 tgt-passing off\n");
        }*/

        p = entry->_ipaddress;
        while ( *p == ' ' )
            p++;
        strncpy(tmp,p,512);
        tmp[511] = 0;
        p = tmp + strlen(tmp) - 1;
        if ( p < tmp ) {
            sprintf(buf,"end 1 No host provided\n");
        } else {
            while ( *p == ' ' ) {
                *p = '\0';
                p--;
            } 

            if ( strchr(entry->_ipaddress,':') ||
                 strchr(entry->_ipaddress,' ') )
                sprintf(buf, "set host %s\n",entry->_ipaddress);
            else
                sprintf(buf, "set host %s %s\n",
                         entry->_ipaddress,
                         entry->_sshport[0]?entry->_sshport:"ssh");
        } 
        OUTFILE(buf);

        break;
    }

    case FTP: {
        ZIL_ICHAR tmp[512], *p;


        if (config->_use_network_dir) { /* Semicolon-separated list */
            char *s;
            s = p = (char *) config->_network_dir;
            /* we use loop just in case */
            while (*p) {
                if (*p == ';')
                    *p = (char) 32;
                p++;
            }
            sprintf(buf, "set network directory %s\n", s);
            OUTFILE(buf);
        } else {
            OUTFILE("set network directory\n");
        }

        if ( entry->_ftp_autologin ) {
            OUTFILE("set ftp autologin on\n");
        } else {
            OUTFILE("set ftp autologin off\n");
        }

        if ( entry->_ftp_passive ) {
            OUTFILE("set ftp passive on\n");
        } else {
            OUTFILE("set ftp passive off\n");
        }

        if ( entry->_ftp_autoauth ) {
            OUTFILE("set ftp autoauth on\n");
        } else {
            OUTFILE("set ftp autoauth off\n");
        }

        if ( entry->_ftp_autoenc ) {
            OUTFILE("set ftp autoenc on\n");
        } else {
            OUTFILE("set ftp autoenc off\n");
        }

        if ( entry->_ftp_credfwd ) {
            OUTFILE("set ftp credential-forwarding on\n");
        } else {
            OUTFILE("set ftp credential-forwarding off\n");
        }

        if ( entry->_ftp_debug ) {
            OUTFILE("set ftp debug on\n");
        } else {
            OUTFILE("set ftp debug off\n");
        }

        if ( entry->_ftp_dates ) {
            OUTFILE("set ftp dates on\n");
        } else {
            OUTFILE("set ftp dates off\n");
        }

        if ( entry->_ftp_literal ) {
            OUTFILE("set ftp filenames literal\n");
        } else {
            OUTFILE("set ftp filenames automatic\n");
        }

        if ( entry->_ftp_verbose ) {
            OUTFILE("set ftp verbose on\n");
        } else {
            OUTFILE("set ftp verbose off\n");
        }

        sprintf(buf,"set ftp authtype %s%s%s%s%s\n",
                 entry->_ftp_auth_gssk5 ? "gssapi-krb5 " : "",
                 entry->_ftp_auth_k4 ? "kerberos4 " : "",
                 entry->_ftp_auth_srp ? "srp " : "",
                 entry->_ftp_auth_ssl ? "ssl " : "",
                 entry->_ftp_auth_tls ? "tls " : "");
        OUTFILE(buf);

        switch (entry->_ftp_charset) {     /* FTP server character-set */
        case F_ASCII:
            OUTFILE("set ftp server-character-set ascii\n");
            break;
        case F_BRITISH:
            OUTFILE("set ftp server-character-set british\n");
            break;
        case F_CANADIAN_FRENCH:
            OUTFILE("set ftp server-character-set canadian-french\n");
            break;
        case F_CP437:
            OUTFILE("set ftp server-character-set cp437\n");
            break;
        case F_CP850:
            OUTFILE("set ftp server-character-set cp850\n");
            break;
        case F_CP852:
            OUTFILE("set ftp server-character-set cp852\n");
            break;
        case F_CP862_HEBREW:
            OUTFILE("set ftp server-character-set cp862\n");
            break;
        case F_CP866_CYRILLIC:
            OUTFILE("set ftp server-character-set cp866\n");
            break;
        case F_CYRILLIC_ISO:
            OUTFILE("set ftp server-character-set cyrillic\n");
            break;
        case F_DANISH:
            OUTFILE("set ftp server-character-set danish\n");
            break;
        case F_DEC_KANJI:
            OUTFILE("set ftp server-character-set dec-kanji\n");
            break;
        case F_DEC_MULTINATIONAL:
            OUTFILE("set ftp server-character-set dec-multinational\n");
            break;
        case F_DG_INTERNATIONAL:
            OUTFILE("set ftp server-character-set dg-international\n");
            break;
        case F_DUTCH:
            OUTFILE("set ftp server-character-set dutch\n");
            break;
        case F_ELOT927_GREEK:
            OUTFILE("set ftp server-character-set elot927-greek\n");
            break;
        case F_FINNISH:
            OUTFILE("set ftp server-character-set finnish\n");
            break;
        case F_FRENCH:
            OUTFILE("set ftp server-character-set french\n");
            break;
        case F_GERMAN:
            OUTFILE("set ftp server-character-set german\n");
            break;
        case F_GREEK_ISO:
            OUTFILE("set ftp server-character-set greek-iso\n");
            break;
        case F_HEBREW_7:
            OUTFILE("set ftp server-character-set hebrew-7\n");
            break;
        case F_HEBREW_ISO:
            OUTFILE("set ftp server-character-set hebrew-iso\n");
            break;
        case F_HP_ROMAN_8:
            OUTFILE("set ftp server-character-set hproman8\n");
            break;
        case F_HUNGARIAN:
            OUTFILE("set ftp server-character-set hungarian\n");
            break;
        case F_ITALIAN:
            OUTFILE("set ftp server-character-set italian\n");
            break;
        case F_JAPAN_EUC:
            OUTFILE("set ftp server-character-set japanese-euc\n");
            break;
        case F_JIS7_KANJI:
            OUTFILE("set ftp server-character-set jis7-kanji\n");
            break;
        case F_KIO8_CYRILLIC:
            OUTFILE("set ftp server-character-set kio8\n");
            break;
        case F_LATIN1_ISO:
            OUTFILE("set ftp server-character-set latin1\n");
            break;
        case F_LATIN2_ISO:
            OUTFILE("set ftp server-character-set latin2\n");
            break;
        case F_MACINTOSH_LATIN:
            OUTFILE("set ftp server-character-set macintosh\n");
            break;
        case F_NEXT_MULTINATIONAL:
            OUTFILE("set ftp server-character-set next\n");
            break;
        case F_NORWEGIAN:
            OUTFILE("set ftp server-character-set norwegian\n");
            break;
        case F_PORTUGUESE:
            OUTFILE("set ftp server-character-set portuguese\n");
            break;
        case F_SHIFT_JIS_KANJI:
            OUTFILE("set ftp server-character-set shift-jis-kanji\n");
            break;
        case F_SHORT_KOI:
            OUTFILE("set ftp server-character-set short-koi\n");
            break;
        case F_SPANISH:
            OUTFILE("set ftp server-character-set spanish\n");
            break;
        case F_SWEDISH:
            OUTFILE("set ftp server-character-set swedish\n");
            break;
        case F_SWISS:
            OUTFILE("set ftp server-character-set swiss\n");
            break;

        case F_CP1250:
            OUTFILE("set ftp server-character-set cp1250\n");
            break;
        case F_CP1251:
            OUTFILE("set ftp server-character-set cp1251\n");
            break;
        case F_CP1252:
            OUTFILE("set ftp server-character-set cp1252\n");
            break;
        case F_CP855:
            OUTFILE("set ftp server-character-set cp855\n");
            break;
        case F_CP858:
            OUTFILE("set ftp server-character-set cp858\n");
            break;
        case F_ELOT928_GREEK:
            OUTFILE("set ftp server-character-set elot928-greek\n");
            break;
        case F_EUC_JP:
            OUTFILE("set ftp server-character-set euc-jp\n");
            break;
        case F_ISO2022_JP:
            OUTFILE("set ftp server-character-set iso2022jp-kanji\n");
            break;
        case F_KOI8_R:
            OUTFILE("set ftp server-character-set koi8r\n");
            break;
        case F_KOI8_U:
            OUTFILE("set ftp server-character-set koi8u\n");
            break;
        case F_LATIN9_ISO:
            OUTFILE("set ftp server-character-set latin9-iso\n");
            break;
        case F_MAZOVIA_PC:
            OUTFILE("set ftp server-character-set mazovia-pc\n");
            break;
        case F_UCS2:
            OUTFILE("set ftp server-character-set ucs2\n");
            break;
        case F_UTF8:
            OUTFILE("set ftp server-character-set utf8\n");
            break;
        case F_BULGARIA_PC:
            OUTFILE("set ftp server-character-set bulgaria-pc\n");
            break;
        case F_CP869:
            OUTFILE("set ftp server-character-set cp869-greek\n");
            break;
        }

        if ( entry->_ftp_xlat ) {
            OUTFILE("set ftp character-set-translation on\n");
        } else {
            OUTFILE("set ftp character-set-translation off\n");
        }

        p = entry->_ipaddress;
        while ( *p == ' ' )
            p++;
        strncpy(tmp,p,512);
        tmp[511] = 0;
        p = tmp + strlen(tmp) - 1;
        if ( p < tmp ) {
            sprintf(buf,"end 1 No host provided\n");
        } else {
            while ( *p == ' ' ) {
                *p = '\0';
                p--;
            } 

            if ( anonymous && entry->_password[0] ) 
                ck_decrypt(entry->_password);

            if ( strchr(entry->_ipaddress,':') ||
                 strchr(entry->_ipaddress,' ') )
                sprintf(buf, "ftp open %s%s%s%s%s\n",entry->_ipaddress,
                         anonymous ? " /anonymous" : (entry->_userid[0]?" /user:":""),
                         (!anonymous && entry->_userid[0])?entry->_userid:"",
                         (anonymous && entry->_password[0]?" /password:":""),
                         (anonymous && entry->_password[0]?entry->_password:""));
            else
                sprintf(buf, "ftp open %s %s%s%s%s%s\n",
                         entry->_ipaddress,
                         entry->_ftpport[0]?entry->_ftpport:"ftp",
                         anonymous ? " /anonymous" : (entry->_userid[0]?" /user:":""),
                         (!anonymous && entry->_userid[0])?entry->_userid:"",
                         (anonymous && entry->_password[0]?" /password:":""),
                         (anonymous && entry->_password[0]?entry->_password:""));

            if ( anonymous && entry->_password[0] ) 
                ck_encrypt(entry->_password);
        } 
        OUTFILE(buf);

        OUTFILE("if success {\n");
        switch ( entry->_ftp_cpl ) {
        case PL_CLEAR:  
            OUTFILE("set ftp command-protection-level clear\n");
            break;
        case PL_CONFIDENTIAL:
            OUTFILE("set ftp command-protection-level confidential\n");
            break;
        case PL_SAFE:
            OUTFILE("set ftp command-protection-level safe\n");
            break;
        case PL_PRIVATE:
            OUTFILE("set ftp command-protection-level private\n");
            break;
        }

        switch ( entry->_ftp_dpl ) {
        case PL_CLEAR:  
            OUTFILE("set ftp data-protection-level clear\n");
            break;
        case PL_CONFIDENTIAL:
            OUTFILE("set ftp data-protection-level confidential\n");
            break;
        case PL_SAFE:
            OUTFILE("set ftp data-protection-level safe\n");
            break;
        case PL_PRIVATE:
            OUTFILE("set ftp data-protection-level private\n");
            break;
        }


        OUTFILE("}\n");
        break;
    }

    case SUPERLAT:
        if (config->_use_network_dir) { /* Semicolon-separated list */
            char *s;
            s = p = (char *) config->_network_dir;
           /* we use loop just in case */
            while (*p) {
                if (*p == ';')
                  *p = (char) 32;
                p++;
            }
            sprintf(tmp, "set network directory {%s}", s);
            OUTFILE(BuildOutFileStr(buf, "", tmp));
        } else {
            OUTFILE("set network directory\n");
        }
#ifdef WIN32
	  OUTFILE( "set network type superlat\n" ) ;
          OUTFILE( "if fail set network type pathworks lat\n" );
	  OUTFILE("if fail end 1 Both SUPERLAT and PATHWORKS LAT Failed\n");
#else
	  sprintf(buf, "set network type pathworks lat\n" ) ;
	  OUTFILE(buf);
	  OUTFILE("if fail end 1 PATHWORKS LAT Failed\n");
#endif
	  sprintf(buf, "set host %s\n", entry->_lataddress);
	  OUTFILE(buf);
	  break;
    }

    /* Make the connection */

    if (entry->_script_file && entry->_script_fname[0]) {          /* Have login script */
        switch (entry->_access) {
	case TCPIP:
        case SSH:
	case SUPERLAT:
        case FTP:
	case DIRECT:
            if ( entry->_script_fname[0] ) {
                OUTFILE("xif success {\n");
                sprintf(tmp, "take {%s}", entry->_script_fname);
                OUTFILE(BuildOutFileStr(buf, "", tmp));
                OUTFILE("if success connect\n");
                OUTFILE("}\n");
            } else
                OUTFILE("if success connect\n");
	    break;
	case PHONE:
            sprintf(buf, "dial %s\n", entry->_phone_number);
            OUTFILE(buf);
            OUTFILE("if fail end 1 DIAL Failed\n");
            if ( entry->_script_fname[0] ) {
                sprintf(tmp, "take {%s}", entry->_script_fname);
                OUTFILE(BuildOutFileStr(buf, "", tmp));
            }
            OUTFILE("if success connect\n");
	    break;
        }

    } else if (entry->_script[0] && entry->_access != FTP) {          /* Have script */
	switch (entry->_access) {
	case TCPIP:
        case SSH:
        case FTP:
	case SUPERLAT:
	case DIRECT: {
	    OUTFILE("xif success {\n");
            int i=0,j=0,goteol=0;
            int len = strlen(entry->_script);
            for ( ;i<len;i++,j++ ) {
                switch ( entry->_script[i] ) {
                case 13:
                    if ( goteol ) {
                        j--;
                        continue;
                    }
                    if ( entry->_script[i+1] == 10 )
                        i++;
                    buf[j] = 10;
                    goteol = 1;
                    break;
                case 10:
                    if ( goteol ) {
                        j--;
                        continue;
                    }
                    buf[j] = 10;
                    goteol = 1;
                    break;
                default:
                    buf[j] = entry->_script[i];
                    goteol = 0;
                }
            }
            if ( !goteol )
                buf[j++] = 10;
            buf[j] = 0;
            OUTFILE(buf);
	    OUTFILE("if success connect\n");
	    OUTFILE("}\n");
	    break;
        }
	case PHONE: {
	    sprintf(buf, "dial %s\n", entry->_phone_number);
	    OUTFILE(buf);
	    OUTFILE("if fail end 1 DIAL Failed\n");
            int i=0,j=0,goteol=0;
            int len = strlen(entry->_script);
            for ( ;i<len;i++,j++ ) {
                switch ( entry->_script[i] ) {
                case 13:
                    if ( goteol ) {
                        j--;
                        continue;
                    }
                    if ( entry->_script[i+1] == 10 )
                        i++;
                    buf[j] = 10;
                    goteol = 1;
                    break;
                case 10:
                    if ( goteol ) {
                        j--;
                        continue;
                    }
                    buf[j] = 10;
                    goteol = 1;
                    break;
                default:
                    buf[j] = entry->_script[i];
                    goteol = 0;
                }
            }
            if ( !goteol )
                buf[j++] = 10;
            buf[j] = 0;
            OUTFILE(buf);
	    OUTFILE("if success connect\n");
	    break;
        }
	}

    } else {                            /* No login script */
        switch (entry->_access) {
	case TCPIP:
        case SSH:
	case DIRECT:
	case SUPERLAT:
            OUTFILE("if success connect\n");
             break;
	case PHONE:
            sprintf(buf, "dial %s\n", entry->_phone_number);
            OUTFILE(buf);
            OUTFILE("if success connect\n");
	    break;
        case FTP:
            break;
        }
    }
    return(TRUE);
}

ZIL_UINT32 K_CONNECTOR::
StartKermit( KD_LIST_ITEM * entry, KD_CONFIG * config, KD_LIST_ITEM * def_entry ) 
{
    FILE *OutFile;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * MessageBox = NULL ;       /* Used to error messages */
    ZIL_INT8 i;                                 /* Workers */
    ZIL_ICHAR *p;
    ZIL_ICHAR buf[BUFFERSIZE];
    DWORD ExitCode = 0;

    p = entry->_name;          /* Name of this entry */

   if ( entry->_template )
   {
      StartKermitErrorCode = 0 ;
      MessageBox = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to Connect to a Template" ) ;
       if ( !MessageBox )
           OutofMemory("Unable to create MESSAGE_WINDOW StartKermit 1");
       MessageBox->Control() ;
      delete MessageBox ;
      return FALSE;
   }

   K_STATUS * status = K_STATUS::Find( entry->_name ) ;
   if ( status && 
	(entry->_access == PHONE || entry->_access == DIRECT ))
   {
#ifdef WIN32
    /* OS/2 cannot create a Parent Child relationship between a PM  */
    /* and a Console process.  So there is nothing for us to check. */
       GetExitCodeProcess( (HANDLE) status->_process, &ExitCode ) ;
#endif /* WIN32 */
      if ( 
#ifdef WIN32
	   ExitCode == STILL_ACTIVE 
#else
	   status->_hwnd && status->_state != K_STATUS::IDLE
#endif
	   )
      {
         StartKermitErrorCode = 0 ;
         MessageBox = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                              "Already connected to %s.", entry->_name ) ;
          if ( !MessageBox )
              OutofMemory("Unable to create MESSAGE_WINDOW StartKermit 2");
          MessageBox->Control() ;
         delete MessageBox ;
         return FALSE;
      }
      else
      {
         K_STATUS::Remove( status->_kermit_id ) ;
         status = NULL ;
      }
   }

    if (!StartKermitFileName && 
	 !(StartKermitFileName = (ZIL_ICHAR *) new ZIL_ICHAR[BUFFERSIZE])) {

        if ( !StartKermitFileName )
            OutofMemory("Unable to create MESSAGE_WINDOW StartKermit 3");

#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else
	StartKermitErrorCode = errno;
#endif
        return(FALSE);
    }

   /* Make sure that K95DIR\\TMP exists */
#ifdef WIN32 
    ZIL_ICHAR * env = getenv("K95TMP");
#else
    ZIL_ICHAR * env = getenv("K2TMP");
#endif

   if ( !env )
      env = getenv("TEMP");
   if ( !env )
      env = getenv("TMP");
   if ( env )
   {
       strncpy( StartKermitFileName, env, BUFFERSIZE ) ;
       if ( env[strlen(env)-1] != '\\' )
	   strcat( StartKermitFileName, "\\" ) ;
   }
   else
   {
#ifdef WIN32
       sprintf(StartKermitFileName,"%s%s",(char *)GetAppData(0), "Kermit 95\\TMP\\");
#else
       PTIB pptib;
       PPIB pppib;
       char *szPath;

       DosGetInfoBlocks(&pptib, &pppib);

       szPath = pppib -> pib_pchcmd;
       strncpy(StartKermitFileName,szPath,BUFFERSIZE);
       for ( i = strlen( StartKermitFileName ) ; i > 0 ; i-- )
           if ( StartKermitFileName[i] == '\\' )
           {
               StartKermitFileName[i+1] = '\0' ;
               break;;
           }
       strcat( StartKermitFileName, "TMP\\" ) ;
#endif
   }
    kmkdir( StartKermitFileName );
    if ( StartKermitFileName[strlen(StartKermitFileName)-1] != '\\' )
	 strcat(StartKermitFileName,"\\");

   int len = strlen( StartKermitFileName ) ;
   for (i = 0; p[i] && i < (BUFFERSIZE - 7); i++) { /* Use entry name */
        if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
            p[i] == '\\' ||              /* by underscores ... */
            p[i] == '/'  ||
            p[i] == ':')                 
          StartKermitFileName[i + len] = '_';
        else
          StartKermitFileName[i + len] = p[i];
    }
    i+=len;
    StartKermitFileName[i++] = '0' + (K_STATUS::Instance/10);
    StartKermitFileName[i++] = '0' + (K_STATUS::Instance%10);
    StartKermitFileName[i++] = '.';     /* Add .KSC to end */
    StartKermitFileName[i++] = 'K';
    StartKermitFileName[i++] = 'S';
    StartKermitFileName[i++] = 'C';
    StartKermitFileName[i] = (char) 0;

    if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) 
    {
	/* Try again but this time use first 6 chars only */
	for (i = 0; p[i] && i < 6; i++) { /* Use entry name */
	    if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
		 p[i] == '\\' ||              /* by underscores ... */
		 p[i] == '/'  ||
		 p[i] == ':')                 
		StartKermitFileName[i + len] = '_';
	    else
		StartKermitFileName[i + len] = p[i];
	}
	i+=len;
	StartKermitFileName[i++] = '0' + (K_STATUS::Instance/10);
	StartKermitFileName[i++] = '0' + (K_STATUS::Instance%10);
	StartKermitFileName[i++] = '.';     /* Add .KSC to end */
	StartKermitFileName[i++] = 'K';
	StartKermitFileName[i++] = 'S';
	StartKermitFileName[i++] = 'C';
	StartKermitFileName[i] = (char) 0;
    	
	if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) {
#ifdef WIN32
	    StartKermitErrorCode = GetLastError();
#else
	    StartKermitErrorCode = errno;
#endif
	    return FALSE ;
	}
    }

    if ( !GenerateScript(entry,config,def_entry,OutFile) ) {
        fclose(OutFile);
        StartKermitErrorCode = -1005;
	return(FALSE);
    }

    if (fclose(OutFile)) {        /* Done, close the output file. */
        StartKermitErrorCode = errno;
        return(FALSE);
    }

#ifdef WIN32
    ZIL_SCREENID frameID ;
    Information(I_GET_FRAMEID, &frameID);

    if ( _hwndNextConnect ) {
        ATOM atom = GlobalAddAtom(StartKermitFileName);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,_dwNextConnectPid);

        PostMessage(_hwndNextConnect, OPT_DIALER_HWND, K_STATUS::Instance, (LONG) frameID);
        PostMessage(_hwndNextConnect, OPT_DIALER_CONNECT, 0, atom);
        _hwndNextConnect = 0;
        _dwNextConnectPid = 0;
	return ((ZIL_UINT32) hProcess);
    }

   /*
      This really bad hack of code is here only because Windows 95 doesn't do
      what it is supposed to be doing, namely properly constructing the 
      console window for its child process.  Therefore, we must do it 
      manually.  
   */
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    
    STARTUPINFO si ;
    memset( &si, 0, sizeof(STARTUPINFO) ) ;
    si.cb = sizeof(STARTUPINFO);

    HANDLE hOut, hIn ;
    if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
	AllocConsole() ;

	hOut = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE, 
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0) ;
	hIn = CreateFile( "CONIN$", GENERIC_READ | GENERIC_WRITE, 
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0) ;

	si.dwFlags = (DWORD) STARTF_USESTDHANDLES ;
	DuplicateHandle( GetCurrentProcess(), hOut, GetCurrentProcess(), 
			 &si.hStdOutput,
                       DUPLICATE_SAME_ACCESS, TRUE, 
			 DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) ;
	si.hStdError = si.hStdOutput ;
	DuplicateHandle( GetCurrentProcess(), hIn, GetCurrentProcess(), 
			 &si.hStdInput,
                       DUPLICATE_SAME_ACCESS, TRUE, 
			 DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) ;
    }
    si.dwXCountChars = (DWORD) 80;
    si.dwYCountChars = (DWORD) entry->_height+(entry->_status_line?1:0) ;
    si.dwFillAttribute = (DWORD) (entry->_color_command_fg) | (entry->_color_command_bg<<4) ;
    si.dwFlags |= (DWORD) STARTF_USECOUNTCHARS | STARTF_USEFILLATTRIBUTE |
	STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMAXIMIZED ;

#ifdef WIN32
    if ( entry->_use_gui ) {
        char tmp[128];
        int len, j;
        strcpy(tmp,entry->_facename);
        for ( len = strlen(tmp), j=0 ; j<len ; j++) {
            tmp[j] = tolower(tmp[j]);
            switch ( tmp[j] ) {
            case ' ':
                tmp[j] = '_';
                break;
            case ',':
                tmp[j] = '.';
                break;
            case ';':
                tmp[j] = ':';
                break;
            case '\\':
                tmp[j] = '/';
                break;
            case '?':
                tmp[j] = '!';
                break;
            case '{':   
                tmp[j] = '[';
                break;
            case '}':
                tmp[j] = ']';
                break;
            }
        }
        if ( entry->_startpos_auto )
            sprintf( buf, "k95g.exe \"%s\" -W %d %d --facename:%s --fontsize:%d", 
                     StartKermitFileName,frameID,K_STATUS::Instance,
                     tmp, entry->_fontsize) ;
        else {
            sprintf( buf, "k95g.exe \"%s\" -W %d %d --xpos:%d --ypos:%d --facename:%s --fontsize:%d", 
                     StartKermitFileName,frameID,K_STATUS::Instance,
                     entry->_startpos_x, entry->_startpos_y, 
                     tmp, entry->_fontsize) ;
        }
    } else 
#endif /* WIN32 */
        sprintf( buf, "k95.exe \"%s\" -W %d %d", StartKermitFileName,frameID,K_STATUS::Instance ) ;

    if (!CreateProcess((LPSTR)NULL,          /* start K-95  */
			(LPSTR)buf, /* give it the file */
			(LPSECURITY_ATTRIBUTES)NULL, /* fix if necessary */
			(LPSECURITY_ATTRIBUTES)NULL, /* fix if necessary */
			FALSE,               	       /* fix if necessary */
			(DWORD) /* CREATE_NEW_CONSOLE | */ CREATE_NEW_PROCESS_GROUP,    
			(LPVOID)NULL,                /* fix if necessary */
			(LPSTR)NULL,               /* Current directory */
			&si,         /* Startup info, fix */
			&StartKermitProcessInfo                       /* Process info */
			)) 
    {
	StartKermitErrorCode = GetLastError();
	if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
	    FreeConsole();
	}
	return(FALSE);
    } else
    {
	if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
	    FreeConsole();
	}
	return ((ZIL_UINT32) StartKermitProcessInfo.hProcess);
    }
#else /* not WIN32 */
    ZIL_SCREENID frameID ;
    Information(I_GET_FRAMEID, &frameID);
    sprintf( buf, "%s -W %d %d", StartKermitFileName,frameID,K_STATUS::Instance ) ;

    STARTDATA SData       = {0};
    PSZ       PgmTitle    = NULL,       /* Title of new session     */
              PgmName     = "K2.EXE";   /* This starts an OS/2 session  */
    APIRET    rc          = NO_ERROR;   /* Return code                  */
    PID       pid         = 0;          /* PID returned                 */
    ULONG     ulSessID    = 0;          /* Session ID returned          */
    char      achObjBuf[256] = {0};     /* Error data if DosStart fails */
 
    SData.Length  = sizeof(STARTDATA);
    SData.Related = SSF_RELATED_INDEPENDENT; /* start a Child for PID */
                                             /* not SSF_RELATED_CHILD */
    SData.FgBg    = SSF_FGBG_FORE;           /* start session in foreground  */
    SData.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */
                                             /* Start an OS/2 session using "CMD.EXE /K" */
    SData.PgmTitle = PgmTitle;
    SData.PgmName = PgmName;

#ifdef __WATCOMC__
    /* ZIL_ICHAR is probably char (though it could be wchar_t if ZIL_UNICODE is defined)
     * PgmInputs is PBYTE which is probably unsigned char*
     * Watcom cares about the difference */
    SData.PgmInputs = (PBYTE)buf;               /* Keep session up           */
#else
    SData.PgmInputs = buf;                      /* Keep session up           */
#endif

    SData.TermQ = 0;                            /* No termination queue      */
    SData.Environment = 0;                      /* No environment string     */
    SData.InheritOpt = SSF_INHERTOPT_PARENT;    /* Inherit parent's environ.  */
    SData.SessionType = SSF_TYPE_WINDOWABLEVIO; /* Windowed VIO session      */
    SData.IconFile = 0;                         /* No icon association       */
    SData.PgmHandle = 0;
    /* Open the session VISIBLE and MAXIMIZED */
    SData.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;
    SData.InitXPos  = 30;     /* Initial window coordinates              */
    SData.InitYPos  = 40;
    SData.InitXSize = 200;    /* Initial window size */
    SData.InitYSize = 140;
    SData.Reserved = 0;
    SData.ObjectBuffer  = achObjBuf; /* Contains info if DosExecPgm fails */
    SData.ObjectBuffLen = (ULONG) sizeof(achObjBuf);
 
    rc = DosStartSession(&SData, &ulSessID, &pid);  /* Start the session */
 
    if (rc != NO_ERROR) {
	StartKermitErrorCode = rc;
	return(FALSE);
    }
    else {
	return ((ZIL_UINT32) (pid==0?-1:pid));
    }
#endif
}


void K_CONNECTOR::FillListWithTemplateNames( UIW_VT_LIST * linelist ) 
{
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ;
   KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
   KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
   if ( listitem )
      do { 
         if ( listitem->_template )
            *linelist + new UIW_BUTTON( 0,0,0,listitem->_name,
					BTF_NO_3D | BTF_SEND_MESSAGE, 
					WOF_NO_FLAGS,
                                        ZIL_NULLF(ZIL_USER_FUNCTION),
                                        OPT_LINE_DEVICE_IS_TEMPLATE) ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
    
   listitem = (KD_LIST_ITEM *) _entry_list.First() ;
   lastlistitem = (KD_LIST_ITEM *) _entry_list.Last() ;
   if ( listitem )
      do { 
         if ( listitem->_template )
            *linelist + new UIW_BUTTON( 0,0,0,listitem->_name,
					BTF_NO_3D|BTF_SEND_MESSAGE, 
					WOF_NO_FLAGS,
					ZIL_NULLF(ZIL_USER_FUNCTION),
					OPT_LINE_DEVICE_IS_TEMPLATE) ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
}

ZIL_UINT8 K_CONNECTOR::IsATemplate( ZIL_ICHAR * string ) 
{
   UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ;
   KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
   KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
   if ( listitem )
      do { 
         if ( listitem->_template && !strcmp( string, listitem->_name ) )
            return TRUE ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
    
   listitem = (KD_LIST_ITEM *) _entry_list.First() ;
   lastlistitem = (KD_LIST_ITEM *) _entry_list.Last() ;
   if ( listitem )
      do { 
         if ( listitem->_template && !strcmp( string, listitem->_name ) )
            return TRUE ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
   return FALSE ;
}

KD_LIST_ITEM * 
K_CONNECTOR::FindEntry( ZIL_ICHAR * string, ZIL_UINT8 listonly )
{
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_ENTRIES ) ;
    KD_LIST_ITEM * listitem = (KD_LIST_ITEM *) list->First() ;
    KD_LIST_ITEM * lastlistitem = (KD_LIST_ITEM *) list->Last() ;
    if ( listitem ) {
      do { 
         if ( !strcmp( string, listitem->_name ) )
            return listitem ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
    }
    if ( !listonly ) {
    listitem = (KD_LIST_ITEM *) _entry_list.First() ;
    lastlistitem = (KD_LIST_ITEM *) _entry_list.Last() ;
    if ( listitem ) {
      do { 
         if ( !strcmp( string, listitem->_name ) )
            return listitem ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = listitem->Next() ;
      } while ( listitem );
    }
    }
   return NULL ;
}

K_MODEM * 
K_CONNECTOR::FindModem( ZIL_ICHAR * string )
{
    UI_LIST * list = &_modem_list;
    K_MODEM * listitem = (K_MODEM *) list->First() ;
    K_MODEM * lastlistitem = (K_MODEM *) list->Last() ;

    if ( !string || !string[0] )
	return listitem;

    if ( listitem ) {
      do { 
         if ( !strcmp( string, listitem->_name ) )
            return listitem ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = (K_MODEM *) listitem->Next() ;
      } while ( listitem );
    }

    return (K_MODEM *) NULL ;
}

K_LOCATION * 
K_CONNECTOR::FindLocation( ZIL_ICHAR * string )
{
    UI_LIST * list = &_location_list;
    K_LOCATION * listitem = (K_LOCATION *) list->First() ;
    K_LOCATION * lastlistitem = (K_LOCATION *) list->Last() ;

    if ( !string || !string[0] )
	return listitem;

    if ( listitem ) {
      do { 
         if ( !strcmp( string, listitem->_name ) )
            return listitem ;
         if (listitem == lastlistitem )
            break ;
         else
            listitem = (K_LOCATION *) listitem->Next() ;
      } while ( listitem );
    }

    return (K_LOCATION *) NULL ;
}

/*
   ReadKermitInfo()  --  K95DIAL.INF reader.

   This is in the form of a test program, that is debugged and works ok.
   To make it work in Zinc, define FOR_REAL.  It's not tested there.

   It reads the file and is supposed to set the associated variables, at
   least the ones for the dialing config page.

   I didn't know what variables to set for the Port, Modem, and Speed.

   If the file k95dial.inf exists in the current directory, it is processed
   and then renamed to k95config.inf.  If it does not exist, nothing happens.
   Thus the dialer is configured only the first time it runs after SETUP.
*/

#define KC_DIRECTORY    0
#define KC_MODEM        1
#define KC_PORT         2
#define KC_SPEED        3
#define KC_COUNTRY      4
#define KC_AREA         5
#define KC_LDPREFIX     6
#define KC_TFAREA       7
#define KC_TFPREFIX     8
#define KC_INTLPREFIX   9
#define KC_DIALMETHOD  10
#define KC_PRINTER     11

struct _k_config_info {
    char * tag;
    int val;
} k_config_info[] = {
    "directory",   KC_DIRECTORY,
    "modem",       KC_MODEM,
    "port",        KC_PORT,
    "speed",       KC_SPEED,
    "country",     KC_COUNTRY,
    "area",        KC_AREA,
    "ldprefix",    KC_LDPREFIX,
    "tfarea",      KC_TFAREA,
    "tfprefix",    KC_TFPREFIX,
    "intlprefix",  KC_INTLPREFIX,
    "method",      KC_DIALMETHOD,
    "printer",     KC_PRINTER
};
int ntags = (sizeof(k_config_info) / sizeof(struct _k_config_info));

ZIL_UINT32
K_CONNECTOR::ReadKermitInfo( KD_CONFIG * config, KD_LIST_ITEM * defentry )
/* ReadKermitInfo */ {
#define READBUFLEN 256

    FILE * InFile = NULL;
    char buf[READBUFLEN + 1];
    int bufn = 0;
    int i, x, flag;
    char c;
    char * bufp, * p;

   if ( !config )
      return FALSE ;

    InFile = fopen( 
#ifdef WIN32
		    "k95dial.inf"
#else
		    "k2dial.inf"
#endif
		    , "r" ) ;

    if ( InFile ) {
	K_MODEM * modem = FindModem( "DEFAULT" );
	ZIL_INT8  newmdm = FALSE;
	if ( !modem ) {
	    modem = new K_MODEM();
            if ( !modem )
                OutofMemory("Unable to create MODEM ReadKermitInfo");
	    strcpy(modem->_name,"DEFAULT");
	    newmdm = TRUE;
	}
        while (1) {
	    if ( feof(InFile) )
		break;
            if (bufn > READBUFLEN)
              break;
            c = fgetc(InFile);
            buf[bufn++] = c;
            if (c == (char) 10) {       /* Have line */
                buf[bufn - 1] = (char) 0; /* Trim CRLF */
                bufn = 0;               /* Reset buffer pointer */
                p = buf;
                while (*p <= (char) 32) /* Trim any leading space */
                  p++;
                bufp = p;
                while (*p > (char) 32)  /* Null-terminate the tag */
                  p++;
                *p = (char) 0;
                for (flag = i = 0; i < ntags; i++) {
                    if (!strcmp(k_config_info[i].tag,bufp)) {
                        x = k_config_info[i].val;
                        flag = 1;
                        break;
                    }
                }
                if (flag) {
                    bufp = p + 1;       /* Point to value */
                    switch (x) {
		    case KC_DIRECTORY:
                        /* I don't know where to put this one */
                        /* You probably don't need it anyway. */
                        break;
		    case KC_MODEM:
			strncpy( modem->_type, bufp, 60 ) ;
                        if ( !strcmp( "tapi", bufp ) )
                            modem->_is_tapi = 1;
                        break;
		    case KC_PORT:
			strncpy( modem->_port, bufp, 60 );
                        break;
		    case KC_SPEED:
			modem->_speed = atoi( bufp ) ;
                        break;
		    case KC_COUNTRY:
                        strncpy(config->_country_code, bufp, 3);
                        break;
		    case KC_AREA:
                        strncpy(config->_area_code, bufp, 8);
                        break;
		    case KC_LDPREFIX:
                        strncpy(config->_long_dist_prefix, bufp, 32);
                        break;
		    case KC_TFAREA:
                        strncpy(config->_dial_tf_area, bufp, 32);
                        break;
		    case KC_TFPREFIX:
                        strncpy(config->_dial_tf_prefix, bufp, 32);
                        break;
		    case KC_INTLPREFIX:
                        strncpy(config->_intl_prefix, bufp, 32);
                        break;
		    case KC_PRINTER:
			if ( defentry )
			    strncpy(defentry->_printer_dos, bufp, 256);
			break;
		    case KC_DIALMETHOD:
			strlwr( bufp ) ;
			if ( !strcmp( bufp, "tone" ) )
                            config->_dial_method = Tone ;
			else if ( !strcmp( bufp, "pulse" ) )
                            config->_dial_method = Pulse ;
                    /* Ignore anything else */
                    }
                }
            }
        }

	MDMINF * mdminf = FindMdmInf( modem->_type );
	CopyMdmInfToModem( mdminf, modem );

	if (_userFile->ChDir("~Modems") == 0) {
            ZIL_STORAGE_OBJECT _fileObj( *_userFile, 
				     modem->_name, ID_K_MODEM,
				     UIS_OPENCREATE | UIS_READWRITE ) ;
            modem->Store( modem->_name, _userFile, &_fileObj,   
                          ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM)) ;
            if ( _userFile->storageError )
            {
                // What went wrong?
                ZAF_MESSAGE_WINDOW * message =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					"Unable to Write modem \"%s\": errorno %d", 
					modem->_name,
					_userFile->storageError ) ;
                if ( !message )
                    OutofMemory("Unable to create MESSAGE_WINDOW ReadKermitInfo 1");
                message->Control() ;	
                delete message ;
            }   
            _userFile->ChDir("~");
        }
        if ( newmdm )
            _modem_list + modem;
        
        fclose(InFile);
        if ( 
#ifdef WIN32
        rename("k95dial.inf","k95config.inf")
#else
	rename("k2dial.inf","k2config.inf")
#endif
             != 0 )
        {
	    ZAF_MESSAGE_WINDOW * message =
		new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
					"Unable to rename %s to %s", 
#ifdef WIN32
                                        "k95dial.inf","k95config.inf"
#else
                                        "k2dial.inf","k2config.inf"
#endif
                                        ) ;
	    message->Control() ;	
	    delete message ;
        }
    }
    else
    {
        return FALSE ;
    }
   
   return TRUE ;
}

void K_CONNECTOR::UpdateStatusLine( ZIL_UINT32 UpdateName )
{
    KD_LIST_ITEM * current = (KD_LIST_ITEM *) VtList->Current() ;
    K_STATUS * status = NULL ;
    if ( UpdateName )
    {
        if ( !current )
        {
            if ( strcmp( StatusName->DataGet(), "" ) )
                StatusName->DataSet("") ;
        }
        else 
        {
            if ( strcmp( StatusName->DataGet(), current->_name ) )
                StatusName->DataSet( current->_name ) ;
            status = K_STATUS::Find( current->_name ) ;
        }
    }
    else
    {
        if ( current )
            status = K_STATUS::Find( current->_name ) ;
    }

    ZIL_UTIME NowUTime ; 
    ZIL_TIME  NowTime ;
    ZIL_TIME  ZeroTime((int)0L) ;

    if ( !status )
    {
        StatusTime->tmFlags |= TMF_TWENTY_FOUR_HOUR ;
        StatusTime->DataSet( &ZeroTime ) ;

        if ( UpdateName )
        {
            if ( !current )
            {
                StatusString->DataSet( "" ) ;
            }
            else if ( current->_template )
            {
                if ( strcmp( StatusString->DataGet(), "Template" ) )
                    StatusString->DataSet( "Template" ) ;
            }
            else
            {
                if ( strcmp( StatusString->DataGet(), "Not connected" ) )
                    StatusString->DataSet( "Not connected" ) ;
            }
        }
    }
    else 
    {
        switch ( status->_state )
        {
        case K_STATUS::CONNECTED:
            {
                ZIL_UTIME DiffUTime (NowUTime) ;
                DiffUTime - status->_connect_time ;
                int year, month, day, hour, min, sec, msec ;
                DiffUTime.Export( &year, &month, &day, &hour, &min, &sec, &msec ) ;
                ZIL_TIME DiffTime( hour, min, sec ) ;

                StatusTime->tmFlags |= TMF_TWENTY_FOUR_HOUR ;
                StatusTime->DataSet( &DiffTime ) ;

                if ( strcmp( StatusString->DataGet(), "OnLine" ) )
                    StatusString->DataSet( "OnLine" ) ;
                break;
            }

        case K_STATUS::IDLE: {
            ZIL_UTIME DiffUTime (status->_disconnect_time) ;
            DiffUTime - status->_connect_time ;
            int year, month, day, hour, min, sec, msec ;
            DiffUTime.Export( &year, &month, &day, &hour, &min, &sec, &msec ) ;
            ZIL_TIME DiffTime( hour, min, sec ) ;

            StatusTime->tmFlags |= TMF_TWENTY_FOUR_HOUR ;
            StatusTime->DataSet( &DiffTime ) ;

            if ( strcmp( StatusString->DataGet(), "Disconnected" ) )
                StatusString->DataSet( "Disconnected" ) ;
            break;
        }	
                  
        default:
            StatusTime->tmFlags |= TMF_TWENTY_FOUR_HOUR ;
            StatusTime->DataSet( &ZeroTime ) ;
            if ( strcmp( StatusString->DataGet(), "Connecting" ) )
                StatusString->DataSet( "Connecting" ) ;
        }	
    }
}

#ifdef WIN32
#define SHORTCUTDEBUG

#define K95_APPID "Kermit.Script"
#define K95_APPID_DEFAULT "Kermit 95 Script"
#define K95_MIME  "application/kermit"
#define K95_MIME_PATH  "MIME\\Database\\Content Type\\application/kermit"

ZIL_UINT8
VerifyClassKSC( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       ".ksc", 0,
                        KEY_READ, &hkCommandKey) )
        return FALSE;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL ;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(K95_APPID,(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8
VerifyClassAppID( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       K95_APPID, 0,
                       KEY_READ, &hkCommandKey) )
        return FALSE;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL ;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(K95_APPID_DEFAULT,(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8
VerifyClassMIME( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    CHAR  lpszKeyValue[512];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       K95_MIME_PATH, 0,
                       KEY_READ, &hkCommandKey) )
        return FALSE;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Extension" ;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(".KSC",(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8
CreateAssociations( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;
    ZIL_ICHAR  K95Path[257]="\"";

    /* Compute K95.EXE path */
    GetModuleFileName( NULL, &K95Path[1], 257 ) ;
    for ( int i = strlen( (char*)K95Path ) ; i > 0 ; i-- )
	if ( K95Path[i] == '\\' )
	{
            K95Path[i+1] = '\0' ;
            break;;
	}
    strcat((char*)K95Path,"k95.exe\"");

    /* .KSC */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       ".ksc", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             ".ksc",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, (CHAR*)K95_APPID, sizeof(K95_APPID));
    RegSetValueEx(hkCommandKey, "Content Type", 0, REG_SZ, 
                   (CHAR*)K95_MIME, sizeof(K95_MIME));

    RegCloseKey( hkCommandKey );

    /* MIME: application/kermit */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       K95_MIME_PATH, 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             K95_MIME_PATH,0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "Extension", 0, REG_SZ, (CHAR *)".KSC", 4);
    RegCloseKey( hkCommandKey );

    /* Kermit.Script */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "Kermit.Script", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             "Kermit.Script",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (CHAR*)K95_APPID_DEFAULT, sizeof(K95_APPID_DEFAULT));
    sprintf((char*)lpszKeyValue,"%s,%d",K95Path,3);
    RegSetValueEx(hkCommandKey, "DefaultIcon", 0, REG_SZ, 
                   (CHAR *)lpszKeyValue, sizeof(lpszKeyValue));


    if ( RegOpenKeyEx(hkCommandKey,
                       "shell", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkCommandKey,
                             "shell",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey,
                       "open", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey2) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey,
                             "open",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey2, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    if ( RegOpenKeyEx(hkSubKey2,
                       "command", 0,
                       KEY_READ | KEY_WRITE, &hkSubKey3) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(hkSubKey2,
                             "command",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkSubKey3, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkSubKey3 );
            RegCloseKey( hkSubKey2 );
            RegCloseKey( hkSubKey );
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    sprintf((char*)lpszKeyValue,"%s \"%%1\"",K95Path);
    RegSetValueEx(hkSubKey3, "", 0, REG_SZ, 
                   (CHAR *)lpszKeyValue, strlen((char*)lpszKeyValue));

    RegCloseKey( hkSubKey3 );
    RegCloseKey( hkSubKey2 );
    RegCloseKey( hkSubKey );
    RegCloseKey( hkCommandKey );

    return TRUE;
}

ZIL_UINT8
VerifyAssociations( void )
{
    ZIL_UINT8 bKeyKSC = 0, bKeyMime = 0, bKeyApp = 0;

    bKeyKSC = VerifyClassKSC();
    bKeyMime = VerifyClassMIME();
    bKeyApp = VerifyClassAppID();

    if ( !(bKeyKSC && bKeyMime && bKeyApp) ) {
        ZAF_MESSAGE_WINDOW * 
            MessageBox =
                new ZAF_MESSAGE_WINDOW( "Error",
                                        "EXCLAMATION", 
                                        ZIL_MSG_NO | ZIL_MSG_YES, ZIL_MSG_NO,
"Shortcuts require an Association for .KSC files be added to the Registry.  Continue?");
        EVENT_TYPE msg = MessageBox->Control() ;
        delete MessageBox ;
        switch ( msg ) {
        case ZIL_DLG_YES:
            if ( !CreateAssociations() ) {
                MessageBox =
                    new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                            "Unable to create an Association for .KSC files");
                MessageBox->Control();
                delete MessageBox ;
                return FALSE;
            }
            break;
        case ZIL_DLG_NO:
        default:
            return FALSE;
        }

    }
    return TRUE;
}

#ifndef CKT_NT31
HRESULT
CreateLink(LPSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc,
            LPSTR lpszWorkDir, LPSTR lpszIcon, ZIL_UINT8 IconIndex)
{
    HRESULT hres;
    IShellLink * psl;

#ifdef SHORTCUTDEBUG
    DWORD ec = 0L;			/* Error code */

    printf("CreateLink entry...\n");
    printf("PathObj=[%s]\n", lpszPathObj);
    printf("PathLink=[%s]\n", lpszPathLink);
    printf("Description=[%s]\n", lpszDesc);
    printf("WorkDir=[%s]\n",lpszWorkDir);
    printf("Icon=[%s,%d]\n",lpszIcon,IconIndex);
#endif /* SHORTCUTDEBUG */

    hres = CoInitialize( NULL );

    hres = CoCreateInstance(CLSID_ShellLink,
			    NULL,
			    CLSCTX_INPROC_SERVER,
			    IID_IShellLink,
			    (void **)&psl
			    );
#ifdef SHORTCUTDEBUG
    if (hres == S_OK)
      printf("CoCreateInstance returns S_OK\n");
#endif /* SHORTCUTDEBUG */

    if (SUCCEEDED(hres)) {
        IPersistFile * ppf;
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);
        psl->SetWorkingDirectory(lpszWorkDir);
        psl->SetShowCmd(SW_SHOWMAXIMIZED);
        psl->SetIconLocation(lpszIcon,IconIndex);
        hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hres)) {
            wchar_t wsz[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);
            hres = ppf->Save(wsz, TRUE);
#ifdef SHORTCUTDEBUG
            if (SUCCEEDED(hres))
                printf("Save succeeded\n");
            else {
                ec = GetLastError();
                printf("Save failed: %ld\n", ec);
            }
#endif /* SHORTCUTDEBUG */
            ppf->Release();
        }
#ifdef SHORTCUTDEBUG
        else {
            ec = GetLastError();
            printf("QueryInterface failed: %ld\n", ec);
        }
#endif /* SHORTCUTDEBUG */
        psl->Release();
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
    CoUninitialize();
    return(hres);
}
#endif /* CKT_NT31 */
#else /* WIN32 */
HOBJECT
CreateShadow( ZIL_ICHAR * ScriptFile )
{
    HOBJECT   hShadow=0 ;
    HOBJECT   hScript= NULLHANDLE;
    HOBJECT   hDesktop = NULLHANDLE;

    hDesktop = WinQueryObject( "<WP_DESKTOP>" );
    hScript = WinQueryObject( ScriptFile );
    hShadow = WinCreateShadow( hScript, hDesktop, NULL );
    return hShadow;
}
#endif /* WIN32 */


ZIL_UINT32 K_CONNECTOR::
ExportLocations(void) 
{
    FILE *OutFile;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * MessageBox = NULL ;       /* Used to error messages */
    DWORD ExitCode = 0;

    if ( _location_list.Count() < 1 )
    {
	StartKermitErrorCode = 0 ;
	MessageBox = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					     ZIL_MSG_OK, ZIL_MSG_OK,
                                   "No Locations to Export" ) ;
        if ( !MessageBox )
            OutofMemory("Unable to create MESSAGE_WINDOW ExportLocations 1");
	MessageBox->Control() ;
	delete MessageBox ;
	return FALSE;
    }

    if (!StartKermitFileName && 
	 !(StartKermitFileName = (ZIL_ICHAR *) new ZIL_ICHAR[BUFFERSIZE])) {
        if ( !StartKermitFileName )
            OutofMemory("Unable to create ICHAR ExportLocations 1");

#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else	
	StartKermitErrorCode = errno;
#endif
        return(FALSE);
    }


    /* Get the exedir XXXXX */
#ifdef WIN32
    sprintf(StartKermitFileName, "%s%s", GetAppData(0), "Kermit 95\\SCRIPTS\\");
#else
    PTIB pptib;
    PPIB pppib;
    char *szPath;

    DosGetInfoBlocks(&pptib, &pppib);

    szPath = pppib -> pib_pchcmd;
    strncpy(StartKermitFileName,szPath,BUFFERSIZE);

    for (int i = strlen( StartKermitFileName ) ; i > 0 ; i-- )
	if ( StartKermitFileName[i] == '\\' )
	{
            StartKermitFileName[i+1] = '\0' ;
            break;;
	}
    strcat( StartKermitFileName, "SCRIPTS\\" ) ;
#endif
    kmkdir( StartKermitFileName );
    strcat(StartKermitFileName,"location.ksc");

    if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) {
#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else
	StartKermitErrorCode = errno;
#endif
	return FALSE ;
    }

    K_LOCATION * location = (K_LOCATION *) _location_list.First();
    ZIL_ICHAR buf[128];
    ZIL_ICHAR name[64];
    for ( ; location ; location = (K_LOCATION *) location->Next() ) {
	strcpy(name,location->_name);
	for ( char * p = name; *p ; p++ )
	    if ( *p == ' ' ||
		 *p == '\\' ||
		 *p == '/' ||
		 *p == '[' ||
		 *p == ']' ||
		 *p == '{' ||
		 *p == '}' )
		*p = '-';
	sprintf(buf, "define location-%s {\n", name);
	OUTFILE(buf);
	GenerateLocation(location,OutFile);
	sprintf(buf, "}\n\n");
	OUTFILE(buf);
    }

    MessageBox = new ZAF_MESSAGE_WINDOW( "Success", 
					 ZIL_NULLP(ZIL_ICHAR), 
					 ZIL_MSG_OK, ZIL_MSG_OK,
					 "Location Scriptfile %s created.",
					 StartKermitFileName) ;
    if ( !MessageBox )
        OutofMemory("Unable to create MESSAGE_WINDOW ExportLocations 2");
    MessageBox->Control() ;
    delete MessageBox ;


    if (fclose(OutFile)) {        /* Done, close the output file. */
        StartKermitErrorCode = errno;
        return(FALSE);
    }
    return(TRUE);
}


ZIL_UINT32 K_CONNECTOR::
ExportModems(void) 
{
    FILE *OutFile;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * MessageBox = NULL ;       /* Used to error messages */
    DWORD ExitCode = 0;

    if ( _modem_list.Count() < 1 )
    {
	StartKermitErrorCode = 0 ;
	MessageBox = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					     ZIL_MSG_OK, ZIL_MSG_OK,
                                   "No Modems to Export" ) ;
        if ( !MessageBox )
            OutofMemory("Unable to create MESSAGE_WINDOW ExportModems 1");
	MessageBox->Control() ;
	delete MessageBox ;
	return FALSE;
    }

    if (!StartKermitFileName && 
	 !(StartKermitFileName = (ZIL_ICHAR *) new ZIL_ICHAR[BUFFERSIZE])) {
        if ( !StartKermitFileName )
            OutofMemory("Unable to create ICHAR ExportModems");
#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else	
	StartKermitErrorCode = errno;
#endif
        return(FALSE);
    }


    /* Get the exedir XXXXX */
#ifdef WIN32
    sprintf(StartKermitFileName, "%s%s", GetAppData(0), "Kermit 95\\SCRIPTS\\");
#else
    PTIB pptib;
    PPIB pppib;
    char *szPath;

    DosGetInfoBlocks(&pptib, &pppib);

    szPath = pppib -> pib_pchcmd;
    strncpy(StartKermitFileName,szPath,BUFFERSIZE);

    for (int i = strlen( StartKermitFileName ) ; i > 0 ; i-- )
	if ( StartKermitFileName[i] == '\\' )
	{
            StartKermitFileName[i+1] = '\0' ;
            break;;
	}
    strcat( StartKermitFileName, "SCRIPTS\\" ) ;
#endif
    kmkdir( StartKermitFileName );
    strcat(StartKermitFileName,"modem.ksc");

    if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) {
#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else
	StartKermitErrorCode = errno;
#endif
	return FALSE ;
    }

    K_MODEM * modem = (K_MODEM *) _modem_list.First();
    ZIL_ICHAR buf[128];
    ZIL_ICHAR name[64];
    for ( ; modem ; modem = (K_MODEM *) modem->Next() ) {
	strcpy(name,modem->_name);
	for ( char * p = name; *p ; p++ )
	    if ( *p == ' ' ||
		 *p == '\\' ||
		 *p == '/' ||
		 *p == '[' ||
		 *p == ']' ||
		 *p == '{' ||
		 *p == '}' )
		*p = '-';
	sprintf(buf, "define modem-%s {\n", name);
	OUTFILE(buf);
	GenerateModem(modem,NULL,OutFile);
	sprintf(buf, "}\n\n");
	OUTFILE(buf);
    }

    MessageBox = new ZAF_MESSAGE_WINDOW( "Success", 
					 ZIL_NULLP(ZIL_ICHAR), 
					 ZIL_MSG_OK, ZIL_MSG_OK,
					 "Location Scriptfile %s created.",
					 StartKermitFileName) ;
    if ( !MessageBox )
        OutofMemory("Unable to create MESSAGE_WINDOW ExportModems 2");
    MessageBox->Control() ;
    delete MessageBox ;


    if (fclose(OutFile)) {        /* Done, close the output file. */
        StartKermitErrorCode = errno;
        return(FALSE);
    }
    return(TRUE);
}


ZIL_UINT32 K_CONNECTOR::
CreateShortcut( KD_LIST_ITEM * entry, KD_CONFIG * config, KD_LIST_ITEM * def_entry ) 
{
    FILE  *OutFile      = NULL;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * message = NULL ;       /* Used to error messages */
    ZIL_INT8 i=0;                               /* Workers */
    ZIL_ICHAR *p = NULL;
    DWORD ExitCode = 0;
    ZIL_ICHAR name[256];


#ifdef WIN32
    if ( !VerifyAssociations() )
        return FALSE;
#endif /* WIN32 */

    printf("CreateShortcut for connection: %s\r\n", entry->_name);

    strcpy(name,entry->_name);          /* Name of this entry */
    p = name;
    for (i = 0; p[i] ; i++) {           /* Use entry name */
        if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
            p[i] == '\\' ||              /* by underscores ... */
            p[i] == '/'  ||
            p[i] == ':')                 
          p[i] = '_';
    }

    if ( entry->_template )
    {
        printf("CreateShortcut entry is template\r\n");
	StartKermitErrorCode = 0 ;
	message = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					     ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to Create a Shortcut for a Template" ) ;
        if ( !message )
            OutofMemory("Unable to create MESSAGE_WINDOW CreateShortcut 1");
	message->Control() ;
	delete message ;
	return FALSE;
    }

    if ( entry->_access == PHONE ) {
        K_MODEM * modem = FindModem( entry->_modem );
        if ( !modem ) {
            message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                        "\"%s\" references a modem \"%s\" that does not exist.", 
                                        entry->_name,
                                        entry->_modem ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW CreateShortcut 2");
            message->Control() ;
            delete message ;
            return(FALSE);
        }

        if ( !TapiAvail && modem->_is_tapi ) {
            message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                        "\"%s\" references a TAPI modem \"%s\".", 
                                        entry->_name,
                                        entry->_modem ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW CreateShortcut 3");
            message->Control() ;
            delete message ;
            return(FALSE);
        }
    }

    if (!StartKermitFileName && 
	 !(StartKermitFileName = (ZIL_ICHAR *) new ZIL_ICHAR[BUFFERSIZE])) {
        if ( !StartKermitFileName )
            OutofMemory("Unable to create ICHAR CreateShortcut");
#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else	
	StartKermitErrorCode = errno;
#endif
        return(FALSE);
    }

    ZIL_ICHAR K95Icon[256];
    ZIL_ICHAR K95ExeDir[256];

    /* Get the exedir XXXXX */
#ifdef WIN32
    GetModuleFileName( NULL, K95Icon, BUFFERSIZE ) ;
    GetModuleFileName( NULL, K95ExeDir, BUFFERSIZE ) ;
    for ( i = strlen( K95ExeDir ) ; i > 0 ; i-- )
	if ( K95ExeDir[i] == '\\' )
	{
            K95ExeDir[i+1] = '\0' ;
            break;;
	}
    sprintf(StartKermitFileName, "%s%s", GetAppData(0), "Kermit 95\\SCRIPTS\\");
#else
    PTIB pptib;
    PPIB pppib;
    char *szPath;

    DosGetInfoBlocks(&pptib, &pppib);

    szPath = pppib -> pib_pchcmd;
    strncpy(StartKermitFileName,szPath,BUFFERSIZE);
    strcpy( K95Icon, StartKermitFileName );

    for ( i = strlen( StartKermitFileName ) ; i > 0 ; i-- )
	if ( StartKermitFileName[i] == '\\' )
	{
            StartKermitFileName[i+1] = '\0' ;
            break;;
	}
    strcpy( K95ExeDir, StartKermitFileName );

    strcat( StartKermitFileName, "SCRIPTS\\" ) ;
#endif

    kmkdir( StartKermitFileName );

    int len = strlen( StartKermitFileName ) ;
    for (i = 0; p[i] && i < (BUFFERSIZE - 5); i++) { /* Use entry name */
        if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
            p[i] == '\\' ||              /* by underscores ... */
            p[i] == '/'  ||
            p[i] == ':')                 
          StartKermitFileName[i + len] = '_';
        else
          StartKermitFileName[i + len] = p[i];
    }
    i+=len;
    StartKermitFileName[i++] = '.';     /* Add .KSC to end */
    StartKermitFileName[i++] = 'K';
    StartKermitFileName[i++] = 'S';
    StartKermitFileName[i++] = 'C';
    StartKermitFileName[i] = (char) 0;

    printf("CreateShortcut filename = %s\r\n", StartKermitFileName);
    if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) 
    {
	/* Try again but this time use first 8 chars only */
	for (i = 0; p[i] && i < 8; i++) { /* Use entry name */
	    if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
		 p[i] == '\\' ||              /* by underscores ... */
		 p[i] == '/'  ||
		 p[i] == ':')                 
		StartKermitFileName[i + len] = '_';
	    else
		StartKermitFileName[i + len] = p[i];
	}
	i+=len;
	StartKermitFileName[i++] = '.';     /* Add .KSC to end */
	StartKermitFileName[i++] = 'K';
	StartKermitFileName[i++] = 'S';
	StartKermitFileName[i++] = 'C';
	StartKermitFileName[i] = (char) 0;
    	
        printf("CreateShortcut filename (2) = %s\r\n", StartKermitFileName);
	if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) {
#ifdef WIN32
	    StartKermitErrorCode = GetLastError();
#else
	    StartKermitErrorCode = errno;
#endif
	    return FALSE ;
	}
    }

    printf("CreateShortcut Generating Scriptfile\r\n");
    if ( !GenerateScript(entry,config,def_entry,OutFile) ) {
        printf("CreateShortcut GenerateScript failed\r\n");
        fclose(OutFile);
	return(FALSE);
    }


    if (fclose(OutFile)) {        /* Done, close the output file. */
        StartKermitErrorCode = errno;
        return(FALSE);
    }

#ifdef WIN32
#ifndef CKT_NT31
    if ( _config->_shortcut_desktop && Desktop ) {
        HKEY hkCommandKey=0;
        HKEY hkSubKey=0;
        ZIL_ICHAR lpszDesktop[1024];
        DWORD dwType=0;
        DWORD dwSize=0;
        CHAR *lpszValueName=NULL;

        /* Get the address of the desktop directory */
        if ( RegOpenKeyEx( HKEY_CURRENT_USER,
                           "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkCommandKey) ) {
            printf("CreateShortcut RegOpenKeyEx Desktop failed\r\n");
            return NULL;            /* failed */
        }

        dwSize = sizeof(lpszDesktop);
        if ( RegQueryValueEx( hkCommandKey, "Desktop", NULL, &dwType,
                              (CHAR *)lpszDesktop, &dwSize ))
        {
            printf("CreateShortcut RegQueryValueEx Desktop failed\r\n");
            RegCloseKey( hkCommandKey );
            return(NULL);
        }
        RegCloseKey( hkCommandKey );

        strcat(lpszDesktop,"\\");
        strcat(lpszDesktop,name);
        strcat(lpszDesktop,".lnk");
        kmkdir(lpszDesktop);

        printf("CreateShortcut Desktop link: %s\r\n",
                lpszDesktop);
        CreateLink( StartKermitFileName,    /* Script file */
                    lpszDesktop,            /* Link file */
                    entry->_name,           /* Shortcut Name */
                    K95ExeDir,              /* Work Directory */
                    K95Icon,3);               /* Icon */
    }

    if ( _config->_shortcut_startmenu && StartMenu) {
        HKEY hkCommandKey=0;
        HKEY hkSubKey=0;
        ZIL_ICHAR lpszStartMenu[1024];
        DWORD dwType=0;
        DWORD dwSize=0;
        CHAR *lpszValueName=NULL;
        ZIL_ICHAR name[256];
        int i;

        strcpy(name,entry->_name);          /* Name of this entry */
        for (i = 0; name[i] ; i++) {           /* Use entry name */
            if (name[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
                 name[i] == '\\' ||              /* by underscores ... */
                 name[i] == '/'  ||
                 name[i] == ':')                 
                name[i] = '_';
        }

        /* Get the address of the desktop directory */
        if ( RegOpenKeyEx( HKEY_CURRENT_USER,
                           "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkCommandKey) ) {
            printf("CreateShortcut RegOpenKeyEx Shortcut failed\r\n");
            return NULL;            /* failed */
        }

        dwSize = sizeof(lpszStartMenu);
        if ( RegQueryValueEx( hkCommandKey, "Programs", NULL, &dwType,
                              (CHAR *)lpszStartMenu, &dwSize ))
        {
            printf("CreateShortcut RegQueryValueEx Shortcut failed\r\n");
            RegCloseKey( hkCommandKey );
            return(NULL);
        }
        RegCloseKey( hkCommandKey );

        strcat(lpszStartMenu,"\\Kermit 95\\Shortcuts\\");
        strcat(lpszStartMenu,name);
        strcat(lpszStartMenu,".lnk");
        kmkdir(lpszStartMenu);
        printf("CreateShortcut StartMenu link: %s\r\n",
                lpszStartMenu);
        CreateLink( StartKermitFileName,    /* Script file */
                    lpszStartMenu,            /* Link file */
                    entry->_name,           /* Shortcut Name */
                    K95ExeDir,              /* Work Directory */
                    K95Icon,3);
    }
#endif /* CKT_NT31 */
#else /* WIN32 */
    if ( _config->_shortcut_desktop && Desktop ) {
        CreateShadow( StartKermitFileName );
    }
#endif /* WIN32 */

    message = new ZAF_MESSAGE_WINDOW( "Success", 
					 ZIL_NULLP(ZIL_ICHAR), 
					 ZIL_MSG_OK, ZIL_MSG_OK,
					 "Shortcut to scriptfile %s created.",
					 StartKermitFileName) ;
    if ( !message )
        OutofMemory("Unable to create MESSAGE_WINDOW CreateShortcut 4");
    message->Control() ;
    delete message ;

    return(TRUE);
}

ZIL_UINT32 K_CONNECTOR::
CreateScriptFile( KD_LIST_ITEM * entry, KD_CONFIG * config, KD_LIST_ITEM * def_entry ) 
{
    FILE  *OutFile      = NULL;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * message = NULL ;       /* Used to error messages */
    ZIL_INT8 i=0;                               /* Workers */
    ZIL_ICHAR *p = NULL;
    DWORD ExitCode = 0;

    p = entry->_name;          /* Name of this entry */

    if ( entry->_template )
    {
	StartKermitErrorCode = 0 ;
	message = new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
					     ZIL_MSG_OK, ZIL_MSG_OK,
                                   "Unable to Create a Shortcut for a Template" ) ;
        if ( !message )
            OutofMemory("Unable to create MESSAGE_WINDOW CreateScriptFile 1");
	message->Control() ;
	delete message ;
	return FALSE;
    }

    if ( entry->_access == PHONE ) {
        K_MODEM * modem = FindModem( entry->_modem );
        if ( !modem ) {
            message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                        "\"%s\" references a modem \"%s\" that does not exist.", 
                                        entry->_name,
                                        entry->_modem ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW CreateScriptFile 2");
            message->Control() ;
            delete message ;
            return(FALSE);
        }

        if ( !TapiAvail && modem->_is_tapi ) {
            message =
                new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                        "\"%s\" references a TAPI modem \"%s\".", 
                                        entry->_name,
                                        entry->_modem ) ;
            if ( !message )
                OutofMemory("Unable to create MESSAGE_WINDOW CreateScriptFile 3");
            message->Control() ;
            delete message ;
            return(FALSE);
        }
    }

    if (!StartKermitFileName && 
	 !(StartKermitFileName = (ZIL_ICHAR *) new ZIL_ICHAR[BUFFERSIZE])) {
        if ( !StartKermitFileName )
            OutofMemory("Unable to create ICHAR CreateScriptFile");
#ifdef WIN32
	StartKermitErrorCode = GetLastError();
#else	
	StartKermitErrorCode = errno;
#endif
        return(FALSE);
    }


    /* Get the exedir XXXXX */
#ifdef WIN32
    sprintf(StartKermitFileName, "%s%s", GetAppData(0), "Kermit 95\\SCRIPTS\\");
#else
    PTIB pptib;
    PPIB pppib;
    char *szPath;

    DosGetInfoBlocks(&pptib, &pppib);

    szPath = pppib -> pib_pchcmd;
    strncpy(StartKermitFileName,szPath,BUFFERSIZE);

    for ( i = strlen( StartKermitFileName ) ; i > 0 ; i-- )
	if ( StartKermitFileName[i] == '\\' )
	{
            StartKermitFileName[i+1] = '\0' ;
            break;;
	}
    strcat( StartKermitFileName, "SCRIPTS\\" ) ;
#endif

    kmkdir( StartKermitFileName );

    int len = strlen( StartKermitFileName ) ;
    for (i = 0; p[i] && i < (BUFFERSIZE - 5); i++) { /* Use entry name */
        if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
            p[i] == '\\' ||              /* by underscores ... */
            p[i] == '/'  ||
            p[i] == ':')                 
          StartKermitFileName[i + len] = '_';
        else
          StartKermitFileName[i + len] = p[i];
    }
    i+=len;
    StartKermitFileName[i++] = '.';     /* Add .KSC to end */
    StartKermitFileName[i++] = 'K';
    StartKermitFileName[i++] = 'S';
    StartKermitFileName[i++] = 'C';
    StartKermitFileName[i] = (char) 0;

    if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) 
    {
	/* Try again but this time use first 8 chars only */
	for (i = 0; p[i] && i < 8; i++) { /* Use entry name */
	    if (p[i] <= (char) 32 ||        /* but with spaces, etc, replaced */
		 p[i] == '\\' ||              /* by underscores ... */
		 p[i] == '/'  ||
		 p[i] == ':')                 
		StartKermitFileName[i + len] = '_';
	    else
		StartKermitFileName[i + len] = p[i];
	}
	i+=len;
	StartKermitFileName[i++] = '.';     /* Add .KSC to end */
	StartKermitFileName[i++] = 'K';
	StartKermitFileName[i++] = 'S';
	StartKermitFileName[i++] = 'C';
	StartKermitFileName[i] = (char) 0;
    	
	if ((OutFile = fopen(StartKermitFileName,"w")) == NULL) {
#ifdef WIN32
	    StartKermitErrorCode = GetLastError();
#else
	    StartKermitErrorCode = errno;
#endif
	    return FALSE ;
	}
    }

    if ( !GenerateScript(entry,config,def_entry,OutFile) ) {
        fclose(OutFile);
        return(FALSE);
    }


    if (fclose(OutFile)) {        /* Done, close the output file. */
        StartKermitErrorCode = errno;
        return(FALSE);
    }

    message = new ZAF_MESSAGE_WINDOW( "Success", 
					 ZIL_NULLP(ZIL_ICHAR), 
					 ZIL_MSG_OK, ZIL_MSG_OK,
					 "Scriptfile %s created.",
					 StartKermitFileName) ;
    if ( !message )
        OutofMemory("Unable to create MESSAGE_WINDOW CreateScriptFile 4");
    message->Control() ;
    delete message ;

    return(TRUE);
}

ZIL_UINT8 K_CONNECTOR::
IsModemInUse( ZIL_ICHAR * name ) 
{
    KD_LIST_ITEM * item;
    for ( item = (KD_LIST_ITEM *) VtList->First();
	  item;
	  item = (KD_LIST_ITEM *) item->Next() ) {
	if ( !strcmp( name, item->_modem ) &&
             item->_access == PHONE)
	    return TRUE;
    }

    for ( item = (KD_LIST_ITEM *) _entry_list.First();
	  item;
	  item = (KD_LIST_ITEM *) item->Next() ) {
	if ( !strcmp( name, item->_modem ) && 
                      item->_access == PHONE )
	    return TRUE;
    }
    return FALSE;
}

ZIL_ICHAR * K_CONNECTOR::
GetEditorCommand( void )
{
#ifdef WIN32
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static ZIL_ICHAR lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT, 
                       "txtfile\\shell\\open\\command", 0,
			KEY_READ, &hkCommandKey) )
	return NULL;		/* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL ;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
			   (CHAR *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

#ifdef BETATEST
    printf("Editor: %s\n", lpszKeyValue);
#endif 

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static char exename[256];
    _searchenv("epm.exe","PATH",exename);
    if ( !exename[0] )
	_searchenv("e.exe","PATH",exename);
    if ( exename[0] )
	return exename;
    else
	return NULL;
#endif /* NT */
}

ZIL_ICHAR * K_CONNECTOR::
GetBrowserCommand( void )
{
#ifdef WIN32
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static ZIL_ICHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT, 
                       "http\\shell\\open\\command", 0,
			KEY_READ, &hkCommandKey) )
	return NULL;		/* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType, 
			   (CHAR *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

#ifdef BETATEST
    	printf("Browser: %s\n", lpszKeyValue);
#endif

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static char exename[256];
    ZIL_UINT32 size = 256;
    char * env = NULL;

    if ( PrfQueryProfileData( HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
			      "DefaultBrowserExe", exename, &size ) )
	return exename;
    env = getenv("BROWSER");
    if ( env ) {
        strncpy(exename,env,256);
        exename[255] = 0;
    }
    if ( !exename[0] )
        _searchenv("netscape.exe","PATH",exename);
    if ( !exename[0] )
	_searchenv("explore.exe","PATH",exename);
    if ( exename[0] )
	return exename;
    else
	return NULL;
#endif /* NT */
}

ZIL_ICHAR * K_CONNECTOR::
GetFtpCommand( void )
{
#ifdef WIN32
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static ZIL_ICHAR lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    char *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT, 
                       "ftp\\shell\\open\\command", 0,
			KEY_READ, &hkCommandKey) )
	return NULL;		/* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType, 
			   (CHAR *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

#ifdef BETATEST
    	printf("FTP: %s\n", lpszKeyValue);
#endif

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static char exename[256] ;
    ULONG size = 256 ;

    if ( PrfQueryProfileData( HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
			      "DefaultFtpExe", exename, &size ) )
	return exename;
    _searchenv("ftp.exe","PATH",exename);
    if ( exename[0] )
	return exename;
    else
	return NULL;
#endif /* NT */
}

#ifdef WIN32
static HWND MyFrameID=(HWND)0;

void K_CONNECTOR::
Real_Win32ShellExecute( void * _object )
{
#ifdef COMMENT
    SHELLEXECUTEINFO info;
    BOOL  rc;
#endif
    HINSTANCE error;
    char * object = (char *) _object;

#ifndef COMMENT
    error = ShellExecute(MyFrameID, 0, object, 0, 0, SW_SHOWNORMAL);
#else /* COMMENT */
    memset(&info,0,sizeof(SHELLEXECUTEINFO));
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_DOENVSUBST;
    info.nShow = SW_SHOWNORMAL;
    info.lpFile = object;
    info.hwnd = MyFrameID;
    rc = ShellExecuteEx( &info );
    error = info.hInstApp;
    CloseHandle(info.hProcess);
#endif /* COMMENT */

    if (((DWORD)error) <= 32)
    {
        printf("%s %s %d","Win32 ShellExecute failure",object,error);
        switch ( (DWORD)error ) {
        case 0:	
            printf("%s %s %d","Win32 ShellExecute","The operating system is out of memory or resources.",0);
            break;
        case ERROR_BAD_FORMAT:
            printf("%s %s %d","Win32 ShellExecute","The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).",0);
            break;
#ifdef SE_ERR_ACCESSDENIED
        case SE_ERR_ACCESSDENIED:
            printf("%s %s %d","Win32 ShellExecute","The operating system denied access to the specified file.",0);
            break;
#endif
        case SE_ERR_ASSOCINCOMPLETE:
            printf("%s %s %d","Win32 ShellExecute","The filename association is incomplete or invalid.",0);
            break;
        case SE_ERR_DDEBUSY:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction could not be completed because other DDE transactions were being processed.",0);
            break;
        case SE_ERR_DDEFAIL:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction failed.",0);
            break;
        case SE_ERR_DDETIMEOUT:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction could not be completed because the request timed out.",0);
            break;
#ifdef SE_ERR_DLLNOTFOUND
        case SE_ERR_DLLNOTFOUND:
            printf("%s %s %d","Win32 ShellExecute","The specified dynamic-link library was not found.",0);
            break;
#endif
#ifdef SE_ERR_FNF
        case SE_ERR_FNF:
            printf("%s %s %d","Win32 ShellExecute","The specified file was not found.",0);
            break;
#endif
        case SE_ERR_NOASSOC:
            printf("%s %s %d","Win32 ShellExecute","There is no application associated with the given filename extension.",0);
            break;
#ifdef SE_ERR_OOM
        case SE_ERR_OOM:
            printf("%s %s %d","Win32 ShellExecute","There was not enough memory to complete the operation.",0);
            break;
#endif
#ifdef SE_ERR_PNF
        case SE_ERR_PNF:
            printf("%s %s %d","Win32 ShellExecute","The specified path was not found.",0);
            break;
#endif
        case SE_ERR_SHARE:
            printf("%s %s %d","Win32 ShellExecute","A sharing violation occurred.",0);
            break;
        default:
            printf("%s %s %d","Win32 ShellExecute","Unknown error",0);
        }
    }
    else {
        printf("%s %s %d","Win32 ShellExecute success",object,error);
    }
    CloseHandle((HINSTANCE)error);
}

ZIL_INT32 K_CONNECTOR::
Win32ShellExecute( ZIL_ICHAR * object )
{
    ZIL_SCREENID frameID ;
    Information(I_GET_FRAMEID, &frameID);
    MyFrameID = frameID;
    return (_beginthread( Real_Win32ShellExecute, 65535, (void *)object) != -1);
}
#endif /* WIN32 */

void K_CONNECTOR::
os2InitFromRegistry( void )
{
    char * str,*p;
    int quote = 0;

    str = GetEditorCommand();
    if ( str ) {
        quote=0;
        for ( p = str ; *p && (quote || *p != ' ') ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
        }
        if ( *p == ' ' ) {
            *p = '\0';
            strncpy( editor, str, 256 );
            editor[255] = 0;
            p++;
            strncpy( editopts, p, 256 );
            editopts[255] = 0;
        }
        else {
            strncpy( editor, str, 256 );
            editor[255] = 0;
        }
    }
    printf("%s %s\n",editor,editopts);

    /* In Windows we default to nothing so that the ShellExecute code */
    /* will be used instead of a specific application name.           */
    str = GetBrowserCommand();
    if ( str ) {
        quote=0;
        for ( p = str ; *p && (quote || *p != ' ') ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
        }
        if ( *p == ' ' ) {
            *p = '\0';
            strncpy( browser, str, 256 );
            browser[255] = 0;
            p++;
            strncpy( browsopts, p, 256 );
            browsopts[255] = 0;
        }
        else {
            strncpy( browser, str, 256 );
            browser[255] = 0;
        }
    }
    printf("%s %s\n",browser,browsopts);

    str = GetFtpCommand();
    if ( str ) {
        quote=0;
        for ( p = str ; *p && (quote || *p != ' ') ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
        }
        if ( *p == ' ' ) {
            *p = '\0';
            strncpy( ftpapp, str, 256 );
            ftpapp[255] = 0;
            p++;
            strncpy( ftpopts, p, 256 );
            ftpopts[255] = 0;
        }
        else {
            strncpy( ftpapp, str, 256 );
            ftpapp[255] = 0;
        }
    }
    printf("%s %s\n",ftpapp,ftpopts);
}


void K_CONNECTOR::
OutofMemory(ZIL_ICHAR * errstr)
{
    ZIL_ICHAR buf[256];

    printf("Fatal Error!!! %s\n",errstr);

    sprintf(buf,"Fatal Error: %s",errstr);
    ZAF_MESSAGE_WINDOW * message =
        new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", 
                                ZIL_MSG_OK, ZIL_MSG_OK,
                                buf) ;
    if ( message ) {
        message->Control() ;
        delete message ;
    }
    exit(15);
}

ZIL_UINT32 K_CONNECTOR::
GetPopupOffset(void) 
{
    return 0;
}
