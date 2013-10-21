/* C K O R E G  --  Kermit interface for MS Win32 Registry */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include <string.h>
#include <process.h>
#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_WINSHELLDATA
#include <os2.h>
#undef COMMENT
#endif /* NT */

#define DIRSEP       '/'
#define ISDIRSEP(c)  ((c)=='/'||(c)=='\\')

#ifdef NT
static HINSTANCE hAdvApi32 = NULL;
static LONG (WINAPI * p_RegOpenCurrentUser)(REGSAM samDesired, PHKEY phkResult)=NULL;
#endif /* NT */

char *
GetAppData( int common )
{
#ifdef NT
    HKEY  hkCurrentUser=0;
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;
    int i;

#ifdef COMMENT
    env = getenv(common ? "ALLUSERSPROFILE" : "APPDATA");
    if (env) {
        ckstrncpy(lpszKeyValue,env,CKMAXPATH);
        if ( common )
            ckstrncat(lpszKeyValue,"\\Application Data",sizeof(lpszKeyValue));
        debug(F111,"GetAppData env",lpszKeyValue,common);
    } else 
#endif /* COMMENT */    
    {
        if ( !common && !isWin95() ) {
            if ( !p_RegOpenCurrentUser ) {
                if ( !hAdvApi32 )
                    hAdvApi32 = LoadLibrary("advapi32.dll");
                if (hAdvApi32)
                    (FARPROC) p_RegOpenCurrentUser =
                        GetProcAddress( hAdvApi32, "RegOpenCurrentUser");
            }
            if (p_RegOpenCurrentUser)
                p_RegOpenCurrentUser(KEY_READ,&hkCurrentUser);
        }

        if ( RegOpenKeyEx(common ? HKEY_LOCAL_MACHINE : 
                           (hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER),
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkShellKey) ) {
            if ( !common ) {
                if (hkCurrentUser)
                    RegCloseKey(hkCurrentUser);
                return NULL;
            }

            if (RegCreateKeyEx(common ? HKEY_LOCAL_MACHINE : 
                                (hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER),
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                                NULL, &hkShellKey, NULL)) 
            {
                if (hkCurrentUser)
                    RegCloseKey(hkCurrentUser);
                return NULL;            /* failed */
            }
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = common ? "Common AppData" : "AppData";
        if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                              lpszKeyValue, &dwSize ))
        {
            lpszValueName = common ? "Common Desktop" : "Desktop";
            if ( !RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                                  lpszKeyValue, &dwSize ))
            {
                for (i = strlen(lpszKeyValue); 
                     lpszKeyValue[i] != '\\' && lpszKeyValue[i] != '/';
                     i-- );
                lpszKeyValue[i] = '\0';
                ckstrncat(lpszKeyValue,"Application Data",CKMAXPATH+1);
            }
            else {
                GetWindowsDirectory(lpszKeyValue,sizeof(lpszKeyValue));
                ckstrncat(lpszKeyValue,"\\Application Data",CKMAXPATH+1);
            }
            dwSize = strlen(lpszKeyValue)+1;
            RegSetValueEx( hkShellKey, 
                           common ? "Common AppData" : "AppData", 
                           0, dwType,
                           lpszKeyValue, dwSize);
            ckstrncat(lpszKeyValue,"\\",CKMAXPATH+1);
            zmkdir(lpszKeyValue);
        }

        if ( dwType == REG_EXPAND_SZ ) {
            char * env_buf = malloc( dwSize+1 );
            if (env_buf) {
                strcpy(env_buf, lpszKeyValue);
                ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
                free( env_buf );
            }
        }

        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        RegCloseKey( hkShellKey );
        debug(F111,"GetAppData registry",lpszKeyValue,common);
    }

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '/';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '\\' )
          lpszKeyValue[i] = '/';
    }
    debug(F111,"GetAppData returns",lpszKeyValue,common);
    return(lpszKeyValue);
#else /* NT */
    return NULL;
#endif /* NT */
}

char *
GetHomeDrive(void)
{
#ifdef NT
    HKEY  hkCurrentUser=0;
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;

    env = getenv("HOMEDRIVE");
    if (env) {
        ckstrncpy(lpszKeyValue,env,sizeof(lpszKeyValue));
    } else 
    {
        if ( !isWin95() ) {
            if ( !p_RegOpenCurrentUser ) {
                if ( !hAdvApi32 )
                    hAdvApi32 = LoadLibrary("advapi32.dll");
                if (hAdvApi32)
                    (FARPROC) p_RegOpenCurrentUser =
                        GetProcAddress( hAdvApi32, "RegOpenCurrentUser");
            }
            if (p_RegOpenCurrentUser)
                p_RegOpenCurrentUser(KEY_READ,&hkCurrentUser);
        }

        if ( RegOpenKeyEx(hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER,
                      "Volatile Environment", 
                      0, KEY_READ, &hkShellKey) )
        {
            if (hkCurrentUser)
                RegCloseKey(hkCurrentUser);
            return NULL;            /* failed */
        }

        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "HOMEDRIVE";
        if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                              lpszKeyValue, &dwSize ))
        {
            if (hkCurrentUser)
                RegCloseKey(hkCurrentUser);
            RegCloseKey( hkShellKey );
            return(NULL);
        }

        if ( dwType == REG_EXPAND_SZ ) {
            char * env_buf = malloc( dwSize+1 );
            if (env_buf) {
                strcpy(env_buf, lpszKeyValue);
                ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
                free( env_buf );
            }
        }

        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        RegCloseKey( hkShellKey );
    }
    return(lpszKeyValue);
#else /* NT */
    return NULL;
#endif /* NT */
}

char *
GetHomePath(void)
{
#ifdef NT
    HKEY  hkCurrentUser=0;
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;
    int   i;

    env = getenv("HOMEPATH");
    if (env) {
        ckstrncpy(lpszKeyValue,env,sizeof(lpszKeyValue));
    } else 
    {
        if ( !isWin95() ) {
            if ( !p_RegOpenCurrentUser ) {
                if ( !hAdvApi32 )
                    hAdvApi32 = LoadLibrary("advapi32.dll");
                if (hAdvApi32)
                    (FARPROC) p_RegOpenCurrentUser =
                        GetProcAddress( hAdvApi32, "RegOpenCurrentUser");
            }
            if (p_RegOpenCurrentUser)
                p_RegOpenCurrentUser(KEY_READ,&hkCurrentUser);
        }

        if ( RegOpenKeyEx(hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER,
                           "Volatile Environment", 
                           0, KEY_READ, &hkShellKey) ) 
        {
            if (hkCurrentUser)
                RegCloseKey(hkCurrentUser);
            return NULL;            /* failed */
        }
        dwSize = sizeof(lpszKeyValue);
        lpszValueName = "HOMEPATH";
        if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
        {
            if (hkCurrentUser)
                RegCloseKey(hkCurrentUser);
            RegCloseKey( hkShellKey );
            return(NULL);
        }

        if ( dwType == REG_EXPAND_SZ ) {
            char * env_buf = malloc( dwSize+1 );
            if (env_buf) {
                strcpy(env_buf, lpszKeyValue);
                ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
                free( env_buf );
            }
        }

        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        RegCloseKey( hkShellKey );
    }

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '/';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '\\' )
          lpszKeyValue[i] = '/';
    }

    return(lpszKeyValue);
#else
    return NULL;
#endif /* NT */
}

char *
GetPersonal(void)
{
#ifdef NT
    HKEY  hkCurrentUser=0;
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;
    int   i;

    if ( !isWin95() ) {
        if ( !p_RegOpenCurrentUser ) {
            if ( !hAdvApi32 )
                hAdvApi32 = LoadLibrary("advapi32.dll");
            if (hAdvApi32)
                (FARPROC) p_RegOpenCurrentUser =
                    GetProcAddress( hAdvApi32, "RegOpenCurrentUser");
        }
        if (p_RegOpenCurrentUser)
            p_RegOpenCurrentUser(KEY_READ,&hkCurrentUser);
    }

    if ( RegOpenKeyEx(hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER,
                       "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                       0, KEY_READ, &hkShellKey) ) 
    {
        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        return NULL;            /* failed */
    }
    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Personal";
    if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        RegCloseKey( hkShellKey );
        return(NULL);
    }

    if ( dwType == REG_EXPAND_SZ ) {
        char * env_buf = malloc( dwSize+1 );
        if (env_buf) {
            strcpy(env_buf, lpszKeyValue);
            ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
            free( env_buf );
        }
    }

    if (hkCurrentUser)
        RegCloseKey(hkCurrentUser);
    RegCloseKey( hkShellKey );

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '/';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '\\' )
          lpszKeyValue[i] = '/';
    }

    return(lpszKeyValue);
#else
    return NULL;
#endif /* NT */
}

char *
GetDesktop(void)
{
#ifdef NT
    HKEY  hkCurrentUser=0;
    HKEY  hkShellKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1]="";
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL, *env;
    int   i;

    if ( !isWin95() ) {
        if ( !p_RegOpenCurrentUser ) {
            if ( !hAdvApi32 )
                hAdvApi32 = LoadLibrary("advapi32.dll");
            if (hAdvApi32)
                (FARPROC) p_RegOpenCurrentUser =
                    GetProcAddress( hAdvApi32, "RegOpenCurrentUser");
        }
        if (p_RegOpenCurrentUser)
            p_RegOpenCurrentUser(KEY_READ,&hkCurrentUser);
    }

    if ( RegOpenKeyEx(hkCurrentUser ? hkCurrentUser : HKEY_CURRENT_USER,
                       "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                       0, KEY_READ, &hkShellKey) ) 
    {
        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        return NULL;            /* failed */
    }
    dwSize = sizeof(lpszKeyValue);
    lpszValueName = "Desktop";
    if ( RegQueryValueEx( hkShellKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        if (hkCurrentUser)
            RegCloseKey(hkCurrentUser);
        RegCloseKey( hkShellKey );
        return(NULL);
    }

    if ( dwType == REG_EXPAND_SZ ) {
        char * env_buf = malloc( dwSize+1 );
        if (env_buf) {
            strcpy(env_buf, lpszKeyValue);
            ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
            free( env_buf );
        }
    }

    if (hkCurrentUser)
        RegCloseKey(hkCurrentUser);
    RegCloseKey( hkShellKey );

    i = strlen(lpszKeyValue)-1;
    if ( !(ISDIRSEP(lpszKeyValue[i])) ) {
        lpszKeyValue[i+1] = '/';
        lpszKeyValue[i+2] = '\0';
    }
    for (i-- ; i >= 0 ; i--) {
        if ( lpszKeyValue[i] == '\\' )
          lpszKeyValue[i] = '/';
    }

    return(lpszKeyValue);
#else
    return NULL;
#endif /* NT */
}

#ifdef CK_LOGIN
int
IsSSPLogonAvail( void )
{
#ifdef NT
    HKEY  hkCommandKey=0;
    DWORD  dwKeyValue;
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;

    if ( !isWin95() )
        return(1);

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "Security\\Provider", 0,
                        KEY_READ, &hkCommandKey) )
        return(-1);            /* failed */

    dwSize = sizeof(dwKeyValue);
    lpszValueName = "Platform_Type";
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           (char *)&dwKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(-1);
    }

    RegCloseKey( hkCommandKey );
    return(dwKeyValue);
#else /* NT */
    return(0);
#endif /* NT */
}

const char *
SSPLogonDomain( void )
{
#ifdef NT
    HKEY  hkCommandKey=0;
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;
    static char domain[64]="";

    if ( !isWin95() )
        return(NULL);

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "Security\\Provider", 0,
                        KEY_READ, &hkCommandKey) )
        return(NULL);            /* failed */

    dwSize = 64;
    lpszValueName = "Container";
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           (char *)&domain, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

    RegCloseKey( hkCommandKey );
    return(domain[0] ? domain : NULL);
#else /* NT */
    return(NULL);
#endif /* NT */
}
#endif /* CK_LOGIN */

#ifndef NOLOCAL
char *
GetEditorCommand( void )
{
#ifdef NT
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "txtfile\\shell\\open\\command", 0,
                        KEY_READ, &hkCommandKey) )
        return NULL;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL ;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

    if ( dwType == REG_EXPAND_SZ ) {
        char * env_buf = malloc( dwSize+1 );
        if (env_buf) {
            strcpy(env_buf, lpszKeyValue);
            ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
            free( env_buf );
        }
    }

#ifdef BETADEBUG
    printf("Editor: %s\n", lpszKeyValue);
#endif

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static CHAR exename[CKMAXPATH+1];
    _searchenv("epm.exe","PATH",exename);
    if ( !exename[0] )
        _searchenv("e.exe","PATH",exename);
    if ( exename[0] )
        return exename;
    else
        return NULL;
#endif /* NT */
}

char *
GetBrowserCommand( void )
{
#ifdef NT
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "http\\shell\\open\\command", 0,
                        KEY_READ, &hkCommandKey) )
        return NULL;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

    if ( dwType == REG_EXPAND_SZ ) {
        char * env_buf = malloc( dwSize+1 );
        if (env_buf) {
            strcpy(env_buf, lpszKeyValue);
            ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
            free( env_buf );
        }
    }

#ifdef BETADEBUG
        printf("Browser: %s\n", lpszKeyValue);
#endif

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static CHAR exename[CKMAXPATH+1] ;
    ULONG size = CKMAXPATH+1 ;

    if ( PrfQueryProfileData( HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
                              "DefaultBrowserExe", exename, &size ) )
        return exename;
    _searchenv("netscape.exe","PATH",exename);
    if ( !exename[0] )
        _searchenv("explore.exe","PATH",exename);
    if ( exename[0] )
        return exename;
    else
        return NULL;
#endif /* NT */
}

char *
GetFtpCommand( void )
{
#ifdef NT
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    static CHAR  lpszKeyValue[CKMAXPATH+1];
    DWORD dwType=0;
    DWORD dwSize=0;
    CHAR *lpszValueName=NULL;

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "ftp\\shell\\open\\command", 0,
                        KEY_READ, &hkCommandKey) )
        return NULL;            /* failed */

    dwSize = sizeof(lpszKeyValue);
    lpszValueName = NULL;
    if ( RegQueryValueEx( hkCommandKey, lpszValueName, NULL, &dwType,
                           lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return(NULL);
    }

    if ( dwType == REG_EXPAND_SZ ) {
        char * env_buf = malloc( dwSize+1 );
        if (env_buf) {
            strcpy(env_buf, lpszKeyValue);
            ExpandEnvironmentStrings( env_buf, lpszKeyValue, sizeof(lpszKeyValue) );
            free( env_buf );
        }
    }

#ifdef BETADEBUG
        printf("FTP: %s\n", lpszKeyValue);
#endif

    RegCloseKey( hkCommandKey );
    return(lpszKeyValue);
#else /* NT */
    static CHAR exename[CKMAXPATH+1] ;
    ULONG size = CKMAXPATH+1 ;

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

#ifdef BROWSER
#ifdef NT
void
Real_Win32ShellExecute( char * object )
{
    extern HWND hwndConsole;
    extern int priority;
#ifdef COMMENT
    SHELLEXECUTEINFO info;
    BOOL  rc;
#endif
    HINSTANCE error;

    SetThreadPrty(priority,isWin95() ? 3 : 11);

#ifndef COMMENT
    error = ShellExecute(hwndConsole, 0, object, 0, 0, SW_SHOWNORMAL);
#else /* COMMENT */
    memset(&info,0,sizeof(SHELLEXECUTEINFO));
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_DOENVSUBST;
    info.nShow = SW_SHOWNORMAL;
    info.lpFile = object;
    info.hwnd = hwndConsole;
    rc = ShellExecuteEx( &info );
    error = info.hInstApp;
    CloseHandle(info.hProcess);
#endif /* COMMENT */

    if (((DWORD)error) <= 32)
    {
        debug(F111,"Win32 ShellExecute failure",object,error);
        switch ( (DWORD)error ) {
        case 0:
            debug(F110,"Win32 ShellExecute","The operating system is out of memory or resources.",0);
            break;
        case ERROR_BAD_FORMAT:
            debug(F110,"Win32 ShellExecute","The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).",0);
            break;
        case SE_ERR_ACCESSDENIED:
            debug(F110,"Win32 ShellExecute","The operating system denied access to the specified file.",0);
            break;
        case SE_ERR_ASSOCINCOMPLETE:
            debug(F110,"Win32 ShellExecute","The filename association is incomplete or invalid.",0);
            break;
        case SE_ERR_DDEBUSY:
            debug(F110,"Win32 ShellExecute","The DDE transaction could not be completed because other DDE transactions were being processed.",0);
            break;
        case SE_ERR_DDEFAIL:
            debug(F110,"Win32 ShellExecute","The DDE transaction failed.",0);
            break;
        case SE_ERR_DDETIMEOUT:
            debug(F110,"Win32 ShellExecute","The DDE transaction could not be completed because the request timed out.",0);
            break;
        case SE_ERR_DLLNOTFOUND:
            debug(F110,"Win32 ShellExecute","The specified dynamic-link library was not found.",0);
            break;
        case SE_ERR_FNF:
            debug(F110,"Win32 ShellExecute","The specified file was not found.",0);
            break;
        case SE_ERR_NOASSOC:
            debug(F110,"Win32 ShellExecute","There is no application associated with the given filename extension.",0);
            break;
        case SE_ERR_OOM:
            debug(F110,"Win32 ShellExecute","There was not enough memory to complete the operation.",0);
            break;
        case SE_ERR_PNF:
            debug(F110,"Win32 ShellExecute","The specified path was not found.",0);
            break;
        case SE_ERR_SHARE:
            debug(F110,"Win32 ShellExecute","A sharing violation occurred.",0);
            break;
        default:
            debug(F110,"Win32 ShellExecute","Unknown error",0);
        }
    }
    else {
        debug(F111,"Win32 ShellExecute success",object,error);
#ifdef COMMENT
        Don't close the handle.  We have seen the handle returned from
        Shell execute be equal to the KbdHandle or Video Handle.  Bad news.
        CloseHandle((HINSTANCE)error);
#endif /* COMMENT */
    }
}

int
Win32ShellExecute( char * object )
{
    int rc = 0;
    DWORD tid = _beginthread( Real_Win32ShellExecute,
#ifndef NT
                            0,
#endif /* NT */
                            65535,
                            (void *)object
                            ) ;
    rc = (DWORD)tid != 0xffffffff;
    return (rc);
}
#endif /* NT */
#endif /* BROWSER */

void
os2InitFromRegistry( void )
{
#ifndef NOPUSH
#ifdef BROWSER
    extern char browser[];
    extern char browsopts[];
#endif /* BROWSER */
#ifndef NOFTP
#ifdef SYSFTP
    extern char ftpapp[];
    extern char ftpopts[];
#endif /* SYSFTP */
#endif /* NOFTP */
    extern char editor[];
    extern char editopts[];
    char * str,*p;
    int quote = 0;


    str = GetEditorCommand();
    debug(F110,"GetEditorCommand",str,0);
    if ( str ) {
        quote=0;
        for ( p = str ; *p ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
            if (!quote && *p == ' ') {
                *p = '\0';
                p++;
                strcpy( editopts, p );
                break;
            }
        }
        strcpy( editor, str );
    }

#ifdef BROWSER
    /* In Windows we default to nothing so that the ShellExecute code */
    /* will be used instead of a specific application name.           */
    str = GetBrowserCommand();
    debug(F110,"GetBrowserCommand",str,0);
    if ( str ) {
        quote=0;
        for ( p = str ; *p ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
            if (!quote && *p == ' ') {
                *p = '\0';
                p++;
                strcpy( browsopts, p );
                break;
            }
        }
        strcpy( browser, str );
    }
#endif /* BROWSER */

#ifndef NOFTP
#ifdef SYSFTP
    str = GetFtpCommand();
    debug(F110,"GetFtpCommand",str,0);
#ifdef COMMENT
    if ( str ) {
        quote=0;
        for ( p = str ; *p ; p++ )
        {
            if ( *p == '"' )
                quote = !quote;
            if (!quote && *p == ' ') {
                *p = '\0';
                p++;
                strcpy( ftpopts, p );
                break;
            }
        }
        strcpy( ftpapp, str );
    }
#else
    strcpy(ftpapp,"ftp.exe");
#endif
#endif /* SYSFTP */
#endif /* NOFTP */
#endif /* NOPUSH */
}
#endif /* NOLOCAL */
