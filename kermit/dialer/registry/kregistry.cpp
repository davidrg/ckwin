#include <ui_win.hpp>
#include <shellapi.h>
#include <shlguid.h>
#include <shlobj.h>
#include <objbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <direct.h>
#define USE_WINDOW_REGISTRY
#include "registry.hpp"
#include "kregistry.hpp"


const int K_WINDOW::KSC_ASSOC         = 10001;
const int K_WINDOW::TELNET_DEFAULT    = 10002;
const int K_WINDOW::REMOVE_K95        = 10003;
const int K_WINDOW::DIALER_SHORTCUT   = 10004;
const int K_WINDOW::K95_SHORTCUT      = 10005;
const int K_WINDOW::DIALER_START_MENU = 10006;
const int K_WINDOW::K95_START_MENU    = 10007;
const int K_WINDOW::DOCS_START_MENU   = 10008;

K_WINDOW::
K_WINDOW(void) : UIW_WINDOW("WINDOW_REGISTRY",defaultStorage)
{

}

K_WINDOW::
~K_WINDOW(void) 
{
        
}

EVENT_TYPE K_WINDOW::
Event( const UI_EVENT & event )
{
    EVENT_TYPE retval = event.type;
    ZIL_SCREENID frameID = NULL ;
    ZIL_UINT32 success = 0;

    switch ( event.type ) {
    case KSC_ASSOC:
        success = CreateAssociations();
        break;
    case TELNET_DEFAULT:
        success = MakeK95DefaultTelnet();
        break;
    case REMOVE_K95:
        success = DeleteAssociations();
        break;
    case DIALER_SHORTCUT:
        success = CreateShortcut("Kermit 95 Dialer","k95dial.exe",1,0,1,0);
        break;
    case K95_SHORTCUT:
        success = CreateShortcut("Kermit 95","k95.exe",1,0,1,0);
        break;
    case DIALER_START_MENU:
        success = CreateShortcut("Kermit 95 Dialer","k95dial.exe",0,1,1,0);
        break;
    case K95_START_MENU:
        success = CreateShortcut("Kermit 95","k95.exe",0,1,1,0);
        break;
    case DOCS_START_MENU:
        success = CreateDocAssociations();
        success &= CreateShortcut("Kermit 95 Hyperlink Manual",
                                   "docs\\manual\\kermit95.htm",0,2,0,0);
        success &= CreateShortcut("Read Me!","docs\\manual\\readme.htm",0,2,0,0);
        success &= CreateShortcut("Known Bugs","docs\\manual\\k95bugs.htm",0,2,0,0);
        success &= CreateShortcut("Updates to Using C-Kermit 2nd Edition",
                        "docs\\manual\\ckermit2.htm",0,2,0,0);
        success &= CreateShortcut("Updates to Kermit-95","docs\\manual\\updates.htm",0,2,0,0);
        break;
    default:
        return UIW_WINDOW::Event(event);
    }
    if ( success )
        Success();
    else 
        Failure();
    return retval;
}


HRESULT K_WINDOW::
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
        psl->SetShowCmd(SW_SHOWNORMAL);
        psl->SetIconLocation(lpszIcon,IconIndex);
        hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hres)) {
            WORD wsz[MAX_PATH];
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

#define K95_APPID "Kermit.Script"
#define K95_APPID_DEFAULT "Columbia University Kermit 95 Script"
#define K95_MIME  "application/kermit"
#define K95_MIME_PATH  "MIME\\Database\\Content Type\\application/kermit"

ZIL_UINT8 K_WINDOW::
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
                           (unsigned char *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(K95_APPID,(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8 K_WINDOW::
VerifyClassAppID( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0;
    CHAR lpszKeyValue[256];
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
                           (unsigned char *)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(K95_APPID_DEFAULT,(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8 K_WINDOW::
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
                           (unsigned char*)lpszKeyValue, &dwSize ))
    {
        RegCloseKey( hkCommandKey );
        return FALSE;
    }

    rc = !strcmp(".KSC",(char*)lpszKeyValue);

    RegCloseKey( hkCommandKey );
    return rc;
}

ZIL_UINT8 K_WINDOW::
MakeK95DefaultTelnet( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    CHAR  lpszKeyValue[256];
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;
    ZIL_ICHAR  K95Path[257];

    /* Compute K95.EXE path */
    GetModuleFileName( NULL, K95Path, 257 ) ;
    for ( int i = strlen( (char*)K95Path ) ; i > 0 ; i-- )
	if ( K95Path[i] == '\\' )
	{
            K95Path[i+1] = '\0' ;
            break;;
	}
    strcat((char*)K95Path,"k95.exe");

    /* Telnet */
    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "telnet", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key does not already exist */
        if ( RegCreateKeyEx(HKEY_CLASSES_ROOT,
                             "telnet",0,0,REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,0,
                             &hkCommandKey, &dwDisposition) )
            return FALSE;
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
            return FALSE;
        }
    }

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (unsigned char*)K95_APPID_DEFAULT, 
                   sizeof(K95_APPID_DEFAULT));
    sprintf((char*)lpszKeyValue,"%s,%d",K95Path,1);
    RegSetValueEx(hkCommandKey, "DefaultIcon", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, sizeof(lpszKeyValue));


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

    sprintf((char*)lpszKeyValue,"%s -J \"%%1\"",K95Path);
    RegSetValueEx(hkSubKey3, "", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, 
                   strlen((char*)lpszKeyValue));

    RegCloseKey( hkSubKey3 );
    RegCloseKey( hkSubKey2 );
    RegCloseKey( hkSubKey );
    RegCloseKey( hkCommandKey );

    return TRUE;
}


ZIL_UINT8 K_WINDOW::
CreateDocAssociations( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;

    /* .ME as in READ.ME */
    /* Key does not already exist */
    if ( !RegCreateKeyEx(HKEY_CLASSES_ROOT,
                         ".me",0,0,REG_OPTION_NON_VOLATILE,
                         KEY_READ | KEY_WRITE,0,
                          &hkCommandKey, &dwDisposition) ) {
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
        }
        else {
            RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                           (unsigned char*)"txtfile", 8);
            RegSetValueEx(hkCommandKey, "Content Type", 0, REG_SZ, 
                           (unsigned char*)"text/plain",10);
            RegCloseKey( hkCommandKey );
        }
    }

    /* .UPD as in READ.ME */
    /* Key does not already exist */
    if ( !RegCreateKeyEx(HKEY_CLASSES_ROOT,
                          ".upd",0,0,REG_OPTION_NON_VOLATILE,
                          KEY_READ | KEY_WRITE,0,
                          &hkCommandKey, &dwDisposition) ) {
        if ( dwDisposition != REG_CREATED_NEW_KEY ) {
            RegCloseKey( hkCommandKey );
        }
        else {
            RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                           (unsigned char*)"txtfile", 8);
            RegSetValueEx(hkCommandKey, "Content Type", 0, REG_SZ, 
                           (unsigned char*)"text/plain",10);
            RegCloseKey( hkCommandKey );
        }   
    }
    return TRUE;
}

ZIL_UINT8 K_WINDOW::
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
    ZIL_ICHAR  K95Path[257];

    /* Compute K95.EXE path */
    GetModuleFileName( NULL, K95Path, 257 ) ;
    for ( int i = strlen( (char*)K95Path ) ; i > 0 ; i-- )
	if ( K95Path[i] == '\\' )
	{
            K95Path[i+1] = '\0' ;
            break;;
	}
    strcat((char*)K95Path,"k95.exe");

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

    RegSetValueEx(hkCommandKey, "", 0, REG_SZ, 
                   (unsigned char*)K95_APPID, sizeof(K95_APPID));
    RegSetValueEx(hkCommandKey, "Content Type", 0, REG_SZ, 
                   (unsigned char*)K95_MIME, sizeof(K95_MIME));

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

    RegSetValueEx(hkCommandKey, "Extension", 0, REG_SZ, 
                   (unsigned char *)".KSC", 4);
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
                   (unsigned char*)K95_APPID_DEFAULT, 
                   sizeof(K95_APPID_DEFAULT));
    sprintf((char*)lpszKeyValue,"%s,%d",K95Path,3);
    RegSetValueEx(hkCommandKey, "DefaultIcon", 0, REG_SZ, 
                   (unsigned char *)lpszKeyValue, sizeof(lpszKeyValue));


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
                   (unsigned char *)lpszKeyValue, 
                   strlen((char*)lpszKeyValue));

    RegCloseKey( hkSubKey3 );
    RegCloseKey( hkSubKey2 );
    RegCloseKey( hkSubKey );
    RegCloseKey( hkCommandKey );

    return TRUE;
}

ZIL_UINT8 K_WINDOW::
DeleteAssociations( void )
{
    HKEY  hkCommandKey=0;
    HKEY  hkSubKey=0,hkSubKey2=0,hkSubKey3=0,hkSubKey4=0;
    DWORD dwType=0;
    DWORD dwSize=0;
    DWORD dwDisposition=0;
    char *lpszValueName=NULL;
    ZIL_UINT8 rc = FALSE;

    /* .KSC */
    RegDeleteKey(HKEY_CLASSES_ROOT, ".ksc");

    /* MIME: application/kermit */
    RegDeleteKey(HKEY_CLASSES_ROOT, K95_MIME_PATH);

    /* Kermit.Script */
    if ( !RegOpenKeyEx(HKEY_CLASSES_ROOT,
                       "Kermit.Script", 0,
                       KEY_READ | KEY_WRITE, &hkCommandKey) ) {
        /* Key exists */
        if ( !RegOpenKeyEx(hkCommandKey,
                            "shell", 0,
                            KEY_READ | KEY_WRITE, &hkSubKey) ) {
            /* Key exists */
            if ( !RegOpenKeyEx(hkSubKey,
                               "open", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "open" );
            if ( !RegOpenKeyEx(hkSubKey,
                               "print", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "print" );
            if ( !RegOpenKeyEx(hkSubKey,
                               "edit", 0,
                               KEY_READ | KEY_WRITE, &hkSubKey2) ) {
                /* Key exists */
                RegDeleteKey( hkSubKey2, "command" );
                RegCloseKey( hkSubKey2 );
            }
            RegDeleteKey( hkSubKey, "edit" );
            RegCloseKey( hkSubKey );
        }
        RegDeleteKey( hkCommandKey, "shell");
        RegCloseKey( hkCommandKey );
    }
    RegDeleteKey(HKEY_CLASSES_ROOT,"Kermit.Script");

    return TRUE;
}

ZIL_UINT8 K_WINDOW::
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

ZIL_UINT32 K_WINDOW::
CreateShortcut( ZIL_ICHAR * username, ZIL_ICHAR * exename, 
                ZIL_INT8 Desktop, ZIL_INT8 StartMenu,
                ZIL_INT8 IconID,  ZIL_INT8 UseNotepad)
{
    FILE  *OutFile      = NULL;
    DWORD  BytesWritten = 0;                    /* Used for i/o */
    ZAF_MESSAGE_WINDOW * message = NULL ;       /* Used to error messages */
    ZIL_INT8 i=0;                               /* Workers */
    ZIL_ICHAR *p = NULL;
    DWORD ExitCode = 0;
    ZIL_ICHAR Path[512];

    p = exename;          /* Name of this entry */

    /* Get the exedir XXXXX */
    GetModuleFileName( NULL, Path, 256 ) ;

    for ( i = strlen( Path ) ; i > 0 ; i-- )
	if ( Path[i] == '\\' )
	{
            Path[i+1] = '\0' ;
            break;;
	}

    ZIL_ICHAR K95ExeDir[256];
    strcpy( K95ExeDir, Path );

    ZIL_ICHAR K95Icon[256];
    strcpy( K95Icon, Path );
    strcat( K95Icon, "K95dial.exe" );

    if ( UseNotepad ) {
        sprintf(Path, "notepad.exe %s%s",K95ExeDir,exename);
    }
    else 
        strcat( Path, exename );

    if ( Desktop ) {
        HKEY hkCommandKey=0;
        HKEY hkSubKey=0;
        ZIL_ICHAR lpszDesktop[1024];
        DWORD dwType=0;
        DWORD dwSize=0;
        CHAR *lpszValueName=NULL;

        /* Get the address of the desktop directory */
        if ( RegOpenKeyEx( HKEY_CURRENT_USER,
                           "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkCommandKey) )
            return NULL;            /* failed */

        dwSize = sizeof(lpszDesktop);
        if ( RegQueryValueEx( hkCommandKey, "Desktop", NULL, &dwType,
                              (unsigned char *)lpszDesktop, &dwSize ))
        {
            RegCloseKey( hkCommandKey );
            return(NULL);
        }
        RegCloseKey( hkCommandKey );

        strcat(lpszDesktop,"\\");
        strcat(lpszDesktop,username);
        strcat(lpszDesktop,".lnk");
        CreateLink( Path,    /* Script file */
                    lpszDesktop,            /* Link file */
                    username,               /* Shortcut Name */
                    K95ExeDir,              /* Work Directory */
                    K95Icon,IconID);        /* Icon */
    }

    if ( StartMenu) {
        HKEY hkCommandKey=0;
        HKEY hkSubKey=0;
        ZIL_ICHAR lpszStartMenu[1024];
        DWORD dwType=0;
        DWORD dwSize=0;
        CHAR *lpszValueName=NULL;

        /* Get the address of the desktop directory */
        if ( RegOpenKeyEx( HKEY_CURRENT_USER,
                           "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                           0, KEY_READ, &hkCommandKey) )
            return NULL;            /* failed */

        dwSize = sizeof(lpszStartMenu);
        if ( RegQueryValueEx( hkCommandKey, "Programs", NULL, &dwType,
                              (unsigned char *)lpszStartMenu, &dwSize ))
        {
            RegCloseKey( hkCommandKey );
            return(NULL);
        }
        RegCloseKey( hkCommandKey );

        strcat(lpszStartMenu,"\\Kermit 95");
        mkdir(lpszStartMenu);
        if ( StartMenu == 2 ) {
            strcat(lpszStartMenu,"\\Documentation");
            mkdir(lpszStartMenu);
        }
        strcat(lpszStartMenu,"\\");
        strcat(lpszStartMenu,username);
        strcat(lpszStartMenu,".lnk");
        CreateLink( Path,    /* Script file */
                    lpszStartMenu,            /* Link file */
                    username,           /* Shortcut Name */
                    K95ExeDir,              /* Work Directory */
                    K95Icon,IconID);
    }
    return(TRUE);
}

void K_WINDOW::
Success( void ) {
    ZAF_MESSAGE_WINDOW * message = 
        new ZAF_MESSAGE_WINDOW( "Success",
                                ZIL_NULLP(ZIL_ICHAR), 
                                ZIL_MSG_OK, ZIL_MSG_OK,
                                "Operation complete.") ;
    message->Control() ;        
    delete message ;
}

void K_WINDOW::
Failure( void ) {
    ZAF_MESSAGE_WINDOW * message = 
        new ZAF_MESSAGE_WINDOW( "Failure",
                                ZIL_NULLP(ZIL_ICHAR), 
                                ZIL_MSG_OK, ZIL_MSG_OK,
                                "Operation failed.") ;
    message->Control() ;        
    delete message ;
}

