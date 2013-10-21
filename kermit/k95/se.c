//SE.CPP
#include <windows.h>
/*
typedef struct _SHELLEXECUTEINFO {    
    DWORD cbSize;     
    ULONG fMask;
    HWND hwnd;     
    LPCTSTR lpVerb;     
    LPCTSTR lpFile;
    LPCTSTR lpParameters;     
    LPCTSTR lpDirectory;     
    int nShow;
    HINSTANCE hInstApp;      // Optional members     
    LPVOID lpIDList;
    LPCSTR lpClass;     
    HKEY hkeyClass;
    DWORD dwHotKey; 	
    union {
        HANDLE hIcon;		
        HANDLE hMonitor;	
    };    
    HANDLE hProcess;
} SHELLEXECUTEINFO, FAR *LPSHELLEXECUTEINFO;  

BOOL ShellExecuteEx(LPSHELLEXECUTEINFO lpExecInfo);

*/

int 
main( int argc, char * argv[] )
{
    HINSTANCE error = ShellExecute(NULL, "open", argv[1],
                                    (argc > 2) ? argv[2] : NULL,
                                    (argc > 3) ? argv[3] : NULL,
                                    SW_SHOW);

    if ( ((DWORD) error) <= 32 ) {
        switch ( (DWORD) error ) {
        case 0:	
            printf("The operating system is out of memory or resources.\n");
            break;
        case ERROR_BAD_FORMAT:
            printf("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).\n");
            break;
        case SE_ERR_ACCESSDENIED:
            printf("The operating system denied access to the specified file.\n");
            break;
        case SE_ERR_ASSOCINCOMPLETE:
            printf("The filename association is incomplete or invalid.\n");
            break;
        case SE_ERR_DDEBUSY:
            printf("The DDE transaction could not be completed because other DDE transactions were being processed.\n");
            break;
        case SE_ERR_DDEFAIL:
            printf("The DDE transaction failed.\n");
            break;
        case SE_ERR_DDETIMEOUT:
            printf("The DDE transaction could not be completed because the request timed out.\n");
            break;
        case SE_ERR_DLLNOTFOUND:
            printf("The specified dynamic-link library was not found.\n");
            break;
        case SE_ERR_FNF:
            printf("The specified file was not found.\n");
            break;
        case SE_ERR_NOASSOC:
            printf("There is no application associated with the given filename extension.\n");
            break;
        case SE_ERR_OOM:
            printf("There was not enough memory to complete the operation.\n");
            break;
        case SE_ERR_PNF:
            printf("The specified path was not found.\n");
            break;
        case SE_ERR_SHARE:
            printf("A sharing violation occurred.\n");
            break;
        default:
            printf("Unknown error\n");
        }
        return 1;
    }
    else {
        CloseHandle((HINSTANCE)error);
        return 0;
    }
}

