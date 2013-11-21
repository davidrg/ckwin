char *cknwin = "Win32 GUI Support 8.0.029, 10 March 2004";
/* C K N W I N   --  Kermit GUI Windows support for Win32 systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include <windows.h>
#include <tapi.h>
#include <commctrl.h>
#include "ckcdeb.h"
#include "ckcker.h"
#include "ckcasc.h"
#include "cknwin.h"
#include "ckowin.h"
#include "ckntap.h"
#include "ckocon.h"
#include "ckuusr.h"
#include "ckokey.h"
#include "ckokvb.h"
#include "ckosyn.h"
#include "richedit.h"

/* Visual C++ 6 fixes */
#ifndef DS_SHELLFONT
#define DS_SHELLFONT        (DS_SETFONT | DS_FIXEDSYS)
#endif
#ifndef DWORD_PTR
typedef unsigned long DWORD_PTR, *PDWORD_PTR;
#endif

/* Global variable */

HINSTANCE   hInst = 0 ;
extern HINSTANCE hInstance ;
HWND        hwndGUI = NULL ;
HWND        hwndConsole = NULL ;

extern HWND hwndDialer ;
extern LONG KermitDialerID ;
extern int  DeleteStartupFile;
extern char cmdfil[];

#ifdef KUI
#include "kui/ikcmd.h"
#include "kui/ikui.h"
extern struct _kui_init kui_init;
#endif /* KUI */

/* Function prototypes */

#ifdef CK_WIN
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif

InitApplication(HINSTANCE);
InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

/* Application entry point */

#ifdef COMMENT
int WINAPI WinMain(hinstance, hPrevInstance, lpCmdLine, nCmdShow)
HINSTANCE hinstance;
HINSTANCE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;
#endif


#ifndef CK_WIN
HANDLE WindowThread = INVALID_HANDLE_VALUE;
DWORD  WindowThreadId = -1 ;

void
ckWindowThread( void * hInstance )
{
    HINSTANCE hinstance = (HINSTANCE) hInstance ;
    int nCmdShow = SW_SHOW ;
    MSG msg;
    extern int SysInited;

    setint();

    if (!InitApplication(hinstance))
    {
        debug( F100, "ckWindowThread InitApplication failed","",0 );
        WindowThreadId = 0;
        _endthread();
    }

    if (!InitInstance(hinstance, nCmdShow))
    {
        debug( F100, "ckWindowThread InitInstance failed","",0 );
        WindowThreadId = 0;
        _endthread();
    }

    debug( F100, "ckWindowThread Init successful","",0 );
    WindowThreadId = GetCurrentThreadId();

    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#ifndef NOKVERBS
    if ( SysInited ) {
        debug(F110,"ckWindowThread","dokverb QUIT",0);
        dokverb( VTERM, K_QUIT );
    }
    else
#endif /* NOKVERBS */
        debug(F110,"ckWindowThread","quitting",0);
}

int
WindowThreadInit( void * hInstance )
{
    WindowThread = (HANDLE) _beginthread( ckWindowThread, 65535, hInstance ) ;
    do {
        msleep(50);
    } while ( WindowThreadId == -1 );

    if ( WindowThreadId == 0 )
        return FALSE;

#ifdef COMMENT
    if (!AttachThreadInput( GetCurrentThreadId(), WindowThreadId, TRUE ))
        printf("AttachThreadInput error = %d\n",GetLastError());
#endif /* COMMENT */
    return TRUE ;
}

void
WindowThreadClose( void )
{
    debug(F100,"Window Thread Close","",0);
    PostMessage(hwndGUI, WM_QUIT, 0, 0);
    msleep(50);
}
#else
HANDLE MainThread = NULL ;
#ifdef COMMENT
int
WindowThreadInit( void )
{
   return TRUE ;
}
#else /* COMMENT */
HANDLE WindowThread = INVALID_HANDLE_VALUE;
DWORD  WindowThreadId = -1 ;

void
ckWindowThread( void * hInstance )
{
    HINSTANCE hinstance = (HINSTANCE) hInstance ;
    int nCmdShow = SW_HIDE ;
    MSG msg;

    WindowThreadId = GetCurrentThreadId();

    if (!InitApplication(hinstance))
    {
       debug( F100, "ckWindowThread InitApplication failed","",0 );
       _endthread();
    }

    if (!InitInstance(hinstance, nCmdShow))
    {
       debug( F100, "ckWindowThread InitInstance failed","",0 );
       _endthread();
    }

    debug( F100, "ckWindowThread Init successful","",0 );

    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int
WindowThreadInit( void * hInstance )
{
    WindowThread = (HANDLE) _beginthread( ckWindowThread, 65535, hInstance ) ;
    while ( WindowThreadId == -1 )
        msleep(50);
    if (!AttachThreadInput( GetCurrentThreadId(), WindowThreadId, TRUE ))
        printf("AttachThreadInput error = %d\n",GetLastError());
    return TRUE ;
}
#endif /* COMMENT */

extern void Main( int, char ** ) ;

void
ckMainThread( void * param )
{
    LPTSTR CmdLine = GetCommandLine() ;
    HANDLE hOut, hIn ;
    char ** argv;
    int    argc = 0 ;
    int i = 0, quote = 0 ;
    char * p = CmdLine ;

    debug(F111,"GetCommandLine()",CmdLine,strlen(CmdLine));
    hInstance = (HINSTANCE) param ;

    while( *p )
    {
       if ( *p == '"' && !quote ) {
           quote++;
       } else if ( quote && *p == '"' ) {
           quote = 0;
           while ( *(p+1) == ' ' )
               p++ ;
           if ( *(p+1) != '\0' )
               argc++;
       } else if ( !quote && *p == ' ' && *(p+1) != '\0' ) {
           argc++ ;
           while ( *(p+1) == ' ' )
               p++ ;
       }
       p++;
    }
    argc++ ;
    argv = malloc( (argc+1) * sizeof(char *) ) ;
    debug(F111,"ckMainThread","argc",argc);
    if ( argv == NULL ) {
		fprintf(stderr,"Out of memory\n");
        exit(1);
    } else {
        argv[argc] = "" ;

        p = CmdLine ;
        for ( i = 0; i < argc ; i++ )
        {
            if ( *p == '"' ) {
                p++;
                argv[i] = p ;
                while (*p && (*p != '"'))
                    p++;
                if (*p)
                    *p++ = '\0';
                while (*p && (*p == ' '))
                    p++;
            } else {
                argv[i] = p ;
                while ( *p && (*p != ' ') )
                    p++ ;
                if (*p)
                    *p++ = '\0';
                while (*p && (*p == ' '))
                    p++;
            }
            debug(F111,"ckMainThread argv",argv[i],i);
        }
    }

#ifndef KUI
    if ( AllocConsole() )
    {
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

        SetStdHandle( STD_INPUT_HANDLE, hIn ) ;
        SetStdHandle( STD_OUTPUT_HANDLE, hOut ) ;
        SetStdHandle( STD_ERROR_HANDLE, hOut );
    }
#endif /* KUI */
    Main( argc, argv ) ;
}

HANDLE
MainThreadInit( HINSTANCE hInst )
{
    MainThread = (HANDLE) _beginthread( ckMainThread, 65535, hInst ) ;
    return(MainThread);
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef KUI
    extern int deblog ;

    kui_init.nCmdShow = nCmdShow;
    ckMainThread( hInstance ) ;
    return 0;
#else /* KUI */
    MSG msg;
    HINSTANCE hModule = GetModuleHandle(NULL);

    if (!InitApplication(hInstance))
    {
       debug( F100, "InitApplication failed","",0 );
       return 0;
    }

    if (!InitInstance(hInstance, nCmdShow))
    {
       debug( F100, "InitInstance failed","",0 );
       return 0;
    }

   MainThreadInit(hInstance) ;

    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        printf("GetMessage\n");
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
   return msg.wParam ;
#endif /* KUI */
}
#endif

BOOL InitApplication(hinstance)
HINSTANCE hinstance;
{
    WNDCLASSEX wcx;

    /*
     * Fill in the window class structure with parameters
     * that describe the main window.
     */

    wcx.cbSize = sizeof(wcx);          /* size of structure      */
    wcx.style = CS_HREDRAW |
        CS_VREDRAW;                    /* redraw if size changes */
    wcx.lpfnWndProc = MainWndProc;     /* points to window proc. */
    wcx.cbClsExtra = 0;                /* no extra class memory  */
    wcx.cbWndExtra = 0;                /* no extra window memory */
    wcx.hInstance = hinstance;         /* handle of instance     */
    wcx.hIcon = LoadIcon(NULL,
        MAKEINTRESOURCE(1));           /* kermit 95 icon   */
    wcx.hCursor = LoadCursor(NULL,
        IDC_ARROW);                    /* predefined arrow       */
    wcx.hbrBackground = GetStockObject(
        WHITE_BRUSH);                  /* white background brush */
    wcx.lpszMenuName =  "CkMainMenu";    /* name of menu resource  */
    wcx.lpszClassName = "CkMainWClass";  /* name of window class   */
    wcx.hIconSm = LoadImage(hinstance, /* small class icon       */
        MAKEINTRESOURCE(5),
                             IMAGE_ICON,

        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);

    /* Register the window class. */

    return RegisterClassEx(&wcx);
}

BOOL
InitInstance(hinstance, nCmdShow)
HINSTANCE hinstance;
int nCmdShow;
{
    /* Save the application-instance handle. */

    hInst = hinstance;

    /* Create the main window. */

#ifndef CK_WIN
   hwndGUI = CreateWindow(
        "CkMainWClass",      /* name of window class        */
        "Kermit 95",         /* title-bar string            */
        WS_CHILD, /* top-level window            */
        CW_USEDEFAULT,       /* default horizontal position */
        CW_USEDEFAULT,       /* default vertical position   */
        CW_USEDEFAULT,       /* default width               */
        CW_USEDEFAULT,       /* default height              */
        (HWND) hwndConsole,         /* console window             */
        (HMENU) 1,        /* use class menu              */
        hinstance,           /* handle of app. instance     */
        (LPVOID) NULL);      /* no window-creation data     */
#else
   hwndGUI = CreateWindow(
        "CkMainWClass",      /* name of window class        */
        "Kermit 95",         /* title-bar string            */
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,       /* default horizontal position */
        CW_USEDEFAULT,       /* default vertical position   */
        CW_USEDEFAULT,       /* default width               */
        CW_USEDEFAULT,       /* default height              */
        (HWND) NULL,
        (HMENU) NULL,        /* use class menu              */
        hinstance,           /* handle of app. instance     */
        (LPVOID) NULL);      /* no window-creation data     */
#endif

    if (!hwndGUI)
        return FALSE;

   ShowWindow( hwndGUI, SW_HIDE ) ;
   /* Let the dialer know */
   if ( StartedFromDialer )
      DialerSend( OPT_KERMIT_HWND, (LONG) hwndGUI ) ;

     /*
      * Show the window and send a WM_PAINT message to the window
      * procedure.
      */

    ShowWindow(hwndGUI, nCmdShow);
    UpdateWindow(hwndGUI);
    return TRUE;

}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0 ;
    extern unsigned long startflags;

    switch ( msg ) {
    case WM_QUERYENDSESSION:
        debug(F111,"MainWndProc","WM_QUERYENDSESSION",msg);
        result = TRUE;
        if ( lparam & ENDSESSION_LOGOFF ) {
            debug(F100,"ENDSESSION_LOGOFF","",0);
            if ( startflags & 128 ) {
                debug(F100,"startflags & 128","",0);
                result = FALSE;
            }
        }
        break;

    case WM_ENDSESSION:
        debug(F111,"MainWndProc","WM_QUERYENDSESSION",msg);
        if ( wparam && (lparam & ENDSESSION_LOGOFF) ) {
            debug(F100,"ENDSESSION_LOGOFF","",0);
            if ( startflags & 128 ) {
                debug(F100,"startflags & 128","",0);
                result = TRUE;
            }
        }
        break;

#ifdef CK_TAPI
    case OPT_TAPI_INIT:
        debug(F110,"K95 WndProc","OPT_TAPI_INIT",0);
        wintapiinit();
        result = TRUE;
        break;

    case OPT_TAPI_SHUTDOWN:
        debug(F110,"K95 WndProc","OPT_TAPI_SHUTDOWN",0);
        wintapishutdown();
        result = TRUE;
        break;
#endif /* CK_TAPI */
    case OPT_KERMIT_HANGUP:
        debug(F110,"K95 WndProc","OPT_KERMIT_HANGUP",0);
#ifndef NOLOCAL
        if ( IsConnectMode() )
            SetConnectMode( FALSE, CSX_USERDISC ) ;
#endif /* NOLOCAL */
        putkeystr( VCMD, "hangup\rexit\ryes\r" ) ;
        DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
        result = TRUE;
        break;

#ifndef NOLOCAL
    case OPT_DIALER_EXIT:
        debug(F110,"K95 WndProc","OPT_DIALER_EXIT",0);
        StartedFromDialer = 0;
        hwndDialer = 0;
        KermitDialerID = 0;
        result = TRUE;
        break;

    case OPT_DIALER_HWND:
        debug(F110,"K95 WndProc","OPT_DIALER_HWND",0);
        StartedFromDialer = 1;
        hwndDialer = (HWND)lparam;
        KermitDialerID = wparam;
        result = TRUE;
        break;

    case OPT_DIALER_CONNECT: {
        char buf[300], file[256];
        debug(F110,"K95 WndProc","OPT_DIALER_CONNECT",0);
        if ( GlobalGetAtomName((ATOM)lparam,file,sizeof(file)) ) {
            debug(F110,"Take File",file,0);
            ckmakmsg(buf,300,"take ",file,"\r",NULL);
            GlobalDeleteAtom((ATOM)lparam);
            putkeystr( VCMD, buf );
            if ( IsConnectMode() )
                SetConnectMode(0,CSX_ESCAPE);

            if ( DeleteStartupFile ) {
                zdelet(cmdfil);
                ckstrncpy(cmdfil,file,CKMAXPATH+1);
            } else {
                ckstrncpy(cmdfil,file,CKMAXPATH+1);
                DeleteStartupFile = 1;
            }
            DialerSend( OPT_KERMIT_HWND, (LONG) hwnd ) ;
        }
        result = TRUE;
        break;
    }
#endif /* NOLOCAL */

    default:
        debug(F111,"K95 WndProc","unknown message",msg);
        debug(F111,"K95 WndProc","wparam",wparam);
        debug(F111,"K95 WndProc","lparam",lparam);
        result = DefWindowProc( hwnd, msg, wparam, lparam );
   }
   return result ;
}

// Checks to see if the dialer (k95dial.exe) exists in the executables directory.
BOOL DialerExists() {
	char buf[MAX_PATH];
	extern char exedir[CKMAXPATH];
	char* lastCharacter;
	BOOL found;
	WIN32_FIND_DATA findFileData;
	HANDLE handle;
	int exedir_len = strlen(exedir);
	

	// Buffer is too small to do anything. Give up.
	if (exedir_len + 12 > MAX_PATH)
		return FALSE;
	
	strncpy(buf, exedir, sizeof(buf));
	// Find the last real character in the exe dir
	lastCharacter = strchr(buf, '\0');
	lastCharacter--;

	if (*lastCharacter != '\\')
		strcat(buf, "\\");
	strcat(buf, "k95dial.exe");

	// Go see if the dialer executable exists.
	handle = FindFirstFile(buf, &findFileData);
	
	found = handle != INVALID_HANDLE_VALUE;

	if(found)
		FindClose(handle);

	return found;
}

void
StartDialer(void)
{
    extern int cmdlvl;
#ifdef DCMDBUF
    extern struct cmdptr *cmdstk;
#else
    extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */
    extern char exedir[CKMAXPATH];
    extern int nopush;
    HWND   _hwndDialer = 0;
    int    reuse;

    if ( nopush )
        return;

#ifndef NOSPL
    reuse = (cmdlvl == 0) && (ttchk() < 0);
#else
    reuse = (tlevel == -1) && (ttchk() < 0);
#endif /* NOSPL */

    if ( StartedFromDialer ) {
        if ( reuse ) {
            DialerSend(OPT_KERMIT_HWND2, (unsigned long)hwndGUI);
            DialerSend(OPT_KERMIT_PID,  GetCurrentProcessId());
        }
        ShowWindowAsync(hwndDialer,SW_SHOWNORMAL);
        SetForegroundWindow(hwndDialer);
    } else if (_hwndDialer = FindWindow(NULL, "Kermit-95 Dialer")) {
        StartedFromDialer = 1;
        hwndDialer = _hwndDialer;
        KermitDialerID = 0;
        DialerSend(OPT_KERMIT_HWND, (unsigned long)hwndGUI);
        if ( reuse ) {
            DialerSend(OPT_KERMIT_HWND2, (unsigned long)hwndGUI);
            DialerSend(OPT_KERMIT_PID,  GetCurrentProcessId());
        }
        ShowWindowAsync(hwndDialer,SW_SHOWNORMAL);
        SetForegroundWindow(hwndDialer);
        StartedFromDialer = 0;
    } else {
        static char buf[512] = "start ", *p, *q;

        for ( p = exedir, q = buf + 6; *p; p++, q++ ) {
            if ( *p == '/' ) {
                *q = '\\';
            } else
                *q = *p;
            if ( *q == '\\' ) {
                q++;
                *q = '\\';
            }
        }
        for ( p = "k95dial.exe"; *p ; p++, q++ )
            *q = *p;
        if ( reuse ) {
            for ( p = " -k "; *p ; p++, q++ )
                *q = *p;
            for ( p = ckultoa((LONG) hwndGUI); *p ; p++, q++ )
                *q = *p;
            *q++ = ' ';
            for ( p = ckultoa((LONG) GetCurrentProcessId()); *p ; p++, q++ )
                *q = *p;
        }
        *q = '\0';
        _zshcmd(buf,0);
    }
}

HWND
GetConsoleHwnd( void )
{
#ifdef KUI
    return getHwndKUI();
#else /* KUI */
    HWND hwndFound = NULL ;
    char NewWindowTitle[1024] ;
    char OldWindowTitle[1024] ;
    int i = 0 ;

    GetConsoleTitle( OldWindowTitle, 1024 ) ;

    for ( i=0 ; i < 20 ; i++ )
    {
      wsprintf( NewWindowTitle, "%d/%d",
                GetTickCount(),
                GetCurrentProcessId());
      SetConsoleTitle(NewWindowTitle);
      Sleep(50);
      hwndFound = FindWindow( NULL, NewWindowTitle ) ;
      if ( hwndFound )
         break;
    }
    debug(F111,"GetConsoleHwnd","Hwnd",hwndFound) ;
    SetConsoleTitle( OldWindowTitle ) ;
    return (hwndFound);
#endif /* KUI */
}

#ifdef KUI
static LPWORD 
lpwAlign( LPWORD lpIn )
{
    ULONG ul;

    ul = (ULONG) lpIn;
    ul += 3;
    ul >>=2;
    ul <<=2;
    return (LPWORD) ul;;
}

static char * sid_buf = NULL;
static int    sid_len = 0;
static int    sid_timeout = 0;
static UINT   sid_timer = 0;

#define ID_TEXT       150
#define ID_SID_TEXT   200
#define ID_SID_TIMER  (WM_USER+300)

static BOOL CALLBACK 
SingleInputDialogProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch ( message ) {
    case WM_INITDIALOG:
        if ( sid_timeout ) {
            sid_timer = SetTimer( hwndDlg, ID_SID_TIMER, sid_timeout * 1000, 
                                  NULL);
        }
        if ( GetDlgCtrlID((HWND) wParam) != ID_SID_TEXT )
        {
            SetFocus(GetDlgItem( hwndDlg, ID_SID_TEXT));
            return FALSE;
        }
        return TRUE;

    case WM_COMMAND:
        switch ( LOWORD(wParam) ) {
        case IDOK:
            if ( !GetDlgItemText(hwndDlg, ID_SID_TEXT, sid_buf, sid_len) )
                *sid_buf = '\0';
            /* fallthrough */
        case IDCANCEL:
            if ( sid_timeout )
                KillTimer(hwndDlg,sid_timer);
            EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    case WM_TIMER:
        if ( sid_timeout )
            KillTimer(hwndDlg,sid_timer);
        EndDialog(hwndDlg, 0);
        return TRUE;
    }
    return FALSE;
}

/* 
 * dialog widths are measured in 1/4 character widths
 * dialog height are measured in 1/8 character heights
 */

static LRESULT
SingleInputDialog( HINSTANCE hinst, HWND hwndOwner, 
                   char * ptext[], int numlines, int width, char * prompt, 
                   int echo, char * dflt)
{
    HGLOBAL hgbl;
    LPDLGTEMPLATE lpdt;
    LPDLGITEMTEMPLATE lpdit;
    LPWORD lpw;
    LPWSTR lpwsz;
    LRESULT ret;
    int nchar, i;
    char * p;

    hgbl = GlobalAlloc(GMEM_ZEROINIT, 4096);
    if (!hgbl)
        return -1;
 
    lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
 
    // Define a dialog box.
 
    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU
                   | DS_MODALFRAME | WS_CAPTION | DS_CENTER 
                   | DS_SETFOREGROUND | DS_3DLOOK
                   | DS_SHELLFONT | DS_NOFAILCREATE;
    lpdt->cdit = numlines + 4;  // number of controls
    lpdt->x  = 10;  
    lpdt->y  = 10;
    lpdt->cx = 20 + width * 4; 
    lpdt->cy = 20 + (numlines + 4) * 14;

    lpw = (LPWORD) (lpdt + 1);
    *lpw++ = 0;   // no menu
    *lpw++ = 0;   // predefined dialog box class (by default)

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, prompt, 
                                    -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = 8;                        // font size (points)
    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "MS Shell Dlg", 
                                    -1, lpwsz, 128);
    lpw   += nchar;


    //-----------------------
    // Define an OK button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)/4 - 20; 
    lpdit->y  = 10 + (numlines + 3) * 14;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDOK;  // OK button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "OK", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    //-----------------------
    // Define an Cancel button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)*3/4 - 20; 
    lpdit->y  = 10 + (numlines + 3) * 14;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDCANCEL;  // CANCEL button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "Cancel", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    /* Add controls for preface data */
    for ( i=0; i<numlines; i++) {
        /*-----------------------
         * Define a static text control.
         *-----------------------*/
        lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
        lpdit = (LPDLGITEMTEMPLATE) lpw;
        lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
        lpdit->dwExtendedStyle = 0;
        lpdit->x  = 10; 
        lpdit->y  = 10 + i * 14;
        lpdit->cx = strlen(ptext[i]) * 4 + 10; 
        lpdit->cy = 14;
        lpdit->id = ID_TEXT + i;  // text identifier

        lpw = (LPWORD) (lpdit + 1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0082;                         // static class

        lpwsz = (LPWSTR) lpw;
        nchar = MultiByteToWideChar (CP_ACP, 0, ptext[i], 
                                         -1, lpwsz, 2*width);
        lpw   += nchar;
        *lpw++ = 0;           // no creation data
    }
    
    /* Prompt */
    /*-----------------------
    * Define a static text control.
    *-----------------------*/
    lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = 10; 
    lpdit->y  = 10 + (numlines + 1) * 14;
    lpdit->cx = strlen(prompt) * 4; 
    lpdit->cy = 14;
    lpdit->id = ID_TEXT + (numlines);  // text identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0082;                         // static class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, prompt, -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data


    /*-----------------------
    * Define an edit control.
    *-----------------------*/
    lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | WS_BORDER
        | (echo == 1 ? 0L : ES_PASSWORD);
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = 10 + (strlen(prompt) + 1) * 4; 
    lpdit->y  = 10 + (numlines + 1) * 14;
    lpdit->cx = (width - (strlen(prompt) + 1)) * 4; 
    lpdit->cy = 14;
    lpdit->id = ID_SID_TEXT;                 // identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0081;                         // edit class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, dflt ? dflt : "", -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    GlobalUnlock(hgbl); 
    ret = DialogBoxIndirect(hinst, (LPDLGTEMPLATE) hgbl, 
        hwndOwner, (DLGPROC) SingleInputDialogProc); 
    GlobalFree(hgbl); 

    switch ( ret ) {
    case 0:     /* Timeout */
        return -1;
    case IDOK:
        return 1;
    case IDCANCEL:
        return 0;
    default: {
        char buf[256];
        ckmakmsg(buf,256,"DialogBoxIndirect() failed ",ckitoa(GetLastError()), NULL, NULL);
        MessageBox(hwndOwner,
                    buf,
                    "GetLastError()",
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        return -1;
    }
    }
}

int
gui_txt_dialog(char * preface, char * prompt, int echo, 
               char * buf, int buflen, char * dflt, int timeout)
{
    int maxwidth = 0;
    int numlines = 0;
    char * plines[16], *p = preface ? preface : "";
    int i;

    if ( !buf || buflen <= 0 || buflen < (dflt ? strlen(dflt) : 0) )
        return(-1);

    sid_buf = buf;
    sid_len = buflen;
    sid_timeout = timeout;

    for ( i=0; i<16; i++ ) 
        plines[i] = NULL;

    while (*p && numlines < 16) {
        plines[numlines++] = p;
        for ( ;*p && *p != '\r' && *p != '\n'; p++ );
        if ( *p == '\r' && *(p+1) == '\n' ) {
            *p++ = '\0';
            p++;
        } else if ( *p == '\n' ) {
            *p++ = '\0';
        } 
        if ( strlen(plines[numlines-1]) > maxwidth )
            maxwidth = strlen(plines[numlines-1]);
    }

    i = strlen(prompt) + 1 + (buflen > 40 ? 40 : buflen);
    if ( maxwidth < i )
        maxwidth = i;

    return(SingleInputDialog(hInstance, hwndConsole, 
                              plines, numlines, maxwidth, prompt, echo, dflt));
}


static int             mid_cnt = 0;
static struct txtbox * mid_tb = NULL;

#define ID_MID_TEXT 300

static BOOL CALLBACK 
MultiInputDialogProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int i;

    switch ( message ) {
    case WM_INITDIALOG:
        if ( GetDlgCtrlID((HWND) wParam) != ID_MID_TEXT )
        {
            SetFocus(GetDlgItem( hwndDlg, ID_MID_TEXT));
            return FALSE;
        }
        return TRUE;

    case WM_COMMAND:
        switch ( LOWORD(wParam) ) {
        case IDOK:
            for ( i=0; i < mid_cnt ; i++ ) {
                if ( !GetDlgItemText(hwndDlg, ID_MID_TEXT+i, mid_tb[i].t_buf, mid_tb[i].t_len) )
                    *mid_tb[i].t_buf = '\0';
            }
            /* fallthrough */
        case IDCANCEL:
            ShowWindowAsync(hwndConsole,SW_SHOWNORMAL);
            SetForegroundWindow(hwndConsole);
            EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;
        }
    }
    return FALSE;
}

/* 
 * dialog widths are measured in 1/4 character widths
 * dialog height are measured in 1/8 character heights
 */

static LRESULT
MultiInputDialog( HINSTANCE hinst, HWND hwndOwner, 
                  char * ptext[], int numlines, int width, 
                  int tb_cnt, struct txtbox * tb)
{
    HGLOBAL hgbl;
    LPDLGTEMPLATE lpdt;
    LPDLGITEMTEMPLATE lpdit;
    LPWORD lpw;
    LPWSTR lpwsz;
    LRESULT ret;
    int nchar, i, pwid;
    char * p;

    hgbl = GlobalAlloc(GMEM_ZEROINIT, 4096);
    if (!hgbl)
        return -1;
 
    mid_cnt = tb_cnt;
    mid_tb = tb;

    lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
 
    // Define a dialog box.
 
    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU
                   | DS_MODALFRAME | WS_CAPTION | DS_CENTER 
                   | DS_SETFOREGROUND | DS_3DLOOK
                   | DS_SHELLFONT | DS_NOFAILCREATE;
    lpdt->cdit = numlines + (2 * tb_cnt) + 2;  // number of controls
    lpdt->x  = 10;  
    lpdt->y  = 10;
    lpdt->cx = 20 + width * 4; 
    lpdt->cy = 20 + (numlines + tb_cnt + 4) * 14;

    lpw = (LPWORD) (lpdt + 1);
    *lpw++ = 0;   // no menu
    *lpw++ = 0;   // predefined dialog box class (by default)

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "", -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = 8;                        // font size (points)
    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "MS Shell Dlg", 
                                    -1, lpwsz, 128);
    lpw   += nchar;

    //-----------------------
    // Define an OK button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)/4 - 20; 
    lpdit->y  = 10 + (numlines + tb_cnt + 2) * 14;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDOK;  // OK button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "OK", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    //-----------------------
    // Define an Cancel button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)*3/4 - 20; 
    lpdit->y  = 10 + (numlines + tb_cnt + 2) * 14;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDCANCEL;  // CANCEL button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "Cancel", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    /* Add controls for preface data */
    for ( i=0; i<numlines; i++) {
        /*-----------------------
         * Define a static text control.
         *-----------------------*/
        lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
        lpdit = (LPDLGITEMTEMPLATE) lpw;
        lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
        lpdit->dwExtendedStyle = 0;
        lpdit->x  = 10; 
        lpdit->y  = 10 + i * 14;
        lpdit->cx = strlen(ptext[i]) * 4 + 10; 
        lpdit->cy = 14;
        lpdit->id = ID_TEXT + i;  // text identifier

        lpw = (LPWORD) (lpdit + 1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0082;                         // static class

        lpwsz = (LPWSTR) lpw;
        nchar = MultiByteToWideChar (CP_ACP, 0, ptext[i], 
                                         -1, lpwsz, 2*width);
        lpw   += nchar;
        *lpw++ = 0;           // no creation data
    }
    
    for ( i=0, pwid = 0; i<tb_cnt; i++) {
        if ( pwid < strlen(tb[i].t_lbl) )
            pwid = strlen(tb[i].t_lbl);
    }

    for ( i=0; i<tb_cnt; i++) {
        /* Prompt */
        /*-----------------------
         * Define a static text control.
         *-----------------------*/
        lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
        lpdit = (LPDLGITEMTEMPLATE) lpw;
        lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
        lpdit->dwExtendedStyle = 0;
        lpdit->x  = 10; 
        lpdit->y  = 10 + (numlines + i + 1) * 14;
        lpdit->cx = pwid * 4; 
        lpdit->cy = 14;
        lpdit->id = ID_TEXT + numlines + i;  // text identifier

        lpw = (LPWORD) (lpdit + 1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0082;                         // static class

        lpwsz = (LPWSTR) lpw;
        nchar = MultiByteToWideChar (CP_ACP, 0, tb[i].t_lbl ? tb[i].t_lbl : "", 
                                     -1, lpwsz, 128);
        lpw   += nchar;
        *lpw++ = 0;           // no creation data

        /*-----------------------
         * Define an edit control.
         *-----------------------*/
        lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
        lpdit = (LPDLGITEMTEMPLATE) lpw;
        lpdit->style = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP | WS_BORDER | (tb[i].t_echo == 1 ? 0L : ES_PASSWORD);
        lpdit->dwExtendedStyle = 0;
        lpdit->x  = 10 + (pwid + 1) * 4; 
        lpdit->y  = 10 + (numlines + i + 1) * 14;
        lpdit->cx = (width - (pwid + 1)) * 4; 
        lpdit->cy = 14;
        lpdit->id = ID_MID_TEXT + i;             // identifier

        lpw = (LPWORD) (lpdit + 1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0081;                         // edit class

        lpwsz = (LPWSTR) lpw;
        nchar = MultiByteToWideChar (CP_ACP, 0, tb[i].t_dflt ? tb[i].t_dflt : "", 
                                     -1, lpwsz, 128);
        lpw   += nchar;
        *lpw++ = 0;           // no creation data
    }

    GlobalUnlock(hgbl); 
    ret = DialogBoxIndirect(hinst, (LPDLGTEMPLATE) hgbl, 
        hwndOwner, (DLGPROC) MultiInputDialogProc); 
    GlobalFree(hgbl); 

    switch ( ret ) {
    case 0:     /* Timeout */
        return -1;
    case IDOK:
        return 1;
    case IDCANCEL:
        return 0;
    default: {
        char buf[256];
        ckmakmsg(buf,256,"DialogBoxIndirect() failed ",ckitoa(GetLastError()), NULL, NULL);
        MessageBox(hwndOwner,
                    buf,
                    "GetLastError()",
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        return -1;
    }
    }
}

int
gui_mtxt_dialog(char * preface, int n, struct txtbox tb[])
{
    int maxwidth = 0;
    int numlines = 0;
    int len;
    char * plines[16], *p = preface ? preface : "";
    int i;

    for ( i=0; i<16; i++ ) 
        plines[i] = NULL;

    while (*p && numlines < 16) {
        plines[numlines++] = p;
        for ( ;*p && *p != '\r' && *p != '\n'; p++ );
        if ( *p == '\r' && *(p+1) == '\n' ) {
            *p++ = '\0';
            p++;
        } else if ( *p == '\n' ) {
            *p++ = '\0';
        } 
        if ( strlen(plines[numlines-1]) > maxwidth )
            maxwidth = strlen(plines[numlines-1]);
    }

    for ( i=0;i<n;i++ ) {
        len = strlen(tb[i].t_lbl) + 1 + (tb[i].t_len > 40 ? 40 : tb[i].t_len);
        if ( maxwidth < len )
            maxwidth = len;
    }

    return(MultiInputDialog(hInstance, hwndConsole, 
                              plines, numlines, maxwidth, n, tb));
}


static BOOL CALLBACK 
VideoPopupDialogProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch ( message ) {
    case WM_INITDIALOG:
        if ( sid_timeout ) {
            sid_timer = SetTimer( hwndDlg, ID_SID_TIMER, sid_timeout * 1000, 
                                  NULL);
        }
        return TRUE;

    case WM_COMMAND:
        switch ( LOWORD(wParam) ) {
        case IDOK:
        case IDCANCEL:
            if ( sid_timeout )
                KillTimer(hwndDlg,sid_timer);
            EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    case WM_TIMER:
        if ( sid_timeout )
            KillTimer(hwndDlg,sid_timer);
        EndDialog(hwndDlg, 0);
        return TRUE;
    }
    return FALSE;
}

/* 
 * dialog widths are measured in 1/4 character widths
 * dialog height are measured in 1/8 character heights
 */

static LRESULT
VideoPopupDialog( HINSTANCE hinst, HWND hwndOwner, videopopup * vp)
{
    HGLOBAL hgbl;
    LPDLGTEMPLATE lpdt;
    LPDLGITEMTEMPLATE lpdit;
    LPWORD lpw;
    LPWSTR lpwsz;
    LRESULT ret;
    int nchar, i, j;
    char * p;

    hgbl = GlobalAlloc(GMEM_ZEROINIT, 8192);
    if (!hgbl)
        return -1;
 
    lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
 
    // Define a dialog box.
 
    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU
                   | DS_MODALFRAME | WS_CAPTION | DS_CENTER 
                   | DS_SETFOREGROUND | DS_3DLOOK
                   | DS_SETFONT | DS_NOFAILCREATE;
    lpdt->cdit = vp->height + 2;  // number of controls
    lpdt->x  = 10;  
    lpdt->y  = 10;
    lpdt->cx = 20 + vp->width * 4; 
    lpdt->cy = 20 + (vp->height + 1) * 8 + 14;

    lpw = (LPWORD) (lpdt + 1);
    *lpw++ = 0;   // no menu
    *lpw++ = 0;   // predefined dialog box class (by default)

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "", -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = 8;                        // font size (points)
    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "Lucida Console", -1, lpwsz, 128);
    lpw   += nchar;


    //-----------------------
    // Define an OK button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)/4 - 20; 
    lpdit->y  = 10 + (vp->height + 1) * 8;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDOK;  // OK button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "OK", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    //-----------------------
    // Define an Cancel button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = (lpdt->cx - 14)*3/4 - 20; 
    lpdit->y  = 10 + (vp->height + 1) * 8;
    lpdit->cx = 40; 
    lpdit->cy = 14;
    lpdit->id = IDCANCEL;  // CANCEL button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "Cancel", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    /* Add controls for preface data */
    for ( i=0; i<vp->height; i++) {
        /*-----------------------
         * Define a static text control.
         *-----------------------*/
        lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
        lpdit = (LPDLGITEMTEMPLATE) lpw;
        lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
        lpdit->dwExtendedStyle = 0;
        lpdit->x  = 10; 
        lpdit->y  = 10 + i * 8;
        lpdit->cx = vp->width * 4 + 10; 
        lpdit->cy = 8;
        lpdit->id = ID_TEXT + i;  // text identifier

        lpw = (LPWORD) (lpdit + 1);
        *lpw++ = 0xFFFF;
        *lpw++ = 0x0082;                         // static class

        lpwsz = (LPWSTR) lpw;
        for ( j=0;j<vp->width && vp->c[i][j];j++ )
            *lpwsz++ = vp->c[i][j];
        *lpwsz++ = 0;
        lpw   = lpwsz;

        *lpw++ = 0;           // no creation data
    }
    
    GlobalUnlock(hgbl); 
    ret = DialogBoxIndirect(hinst, (LPDLGTEMPLATE) hgbl, 
        hwndOwner, (DLGPROC) VideoPopupDialogProc); 
    GlobalFree(hgbl); 

    switch ( ret ) {
    case 0:     /* Timeout */
        return -1;
    case IDOK:
        return F_KVERB | K_HELP;
    case IDCANCEL:
        return 0;
    default: {
        char buf[256];
        ckmakmsg(buf,256,"DialogBoxIndirect() failed ",ckitoa(GetLastError()), NULL, NULL);
        MessageBox(hwndOwner,
                    buf,
                    "GetLastError()",
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        return -1;
    }
    }
}

int
gui_videopopup_dialog(videopopup * vp, int timeout)
{
    int i;

    sid_timeout = timeout;

    return(VideoPopupDialog(hInstance, hwndConsole, vp));
}

int
gui_position(int x, int y)
{
    KuiSetTerminalPosition(x,y);
    msleep(50);
    return(1);
}

int
gui_resize_pixels(int x, int y)
{
    KuiSetTerminalSize(x,y);
    msleep(50);
    return(1);
}

int
gui_resize_mode(int x)
{
    KuiSetTerminalResizeMode(x);
    msleep(50);
    return(1);
}


int
gui_win_run_mode(int x)
{
    KuiSetTerminalRunMode(x);
    return(1);
}

int
gui_saveas_dialog(char * preface, char * prompt, int fc,
                  char * def, char * result, int rlength)
{
    return KuiDownloadDialog(prompt,def,result,rlength);
}

int 
get_gui_window_pos_x(void)
{
    RECT r;

    if ( GetWindowRect(getHwndKUI(), &r) ) {
        return r.left;
    } else 
        return 0;
}

int 
get_gui_window_pos_y(void)
{
    RECT r;

    if ( GetWindowRect(getHwndKUI(), &r) ) {
        return r.top;
    } else 
        return 0;
}

int 
get_gui_resize_mode(void)
{
    return KuiGetTerminalResizeMode();
}


static HWND hwndRichEdit = INVALID_HANDLE_VALUE;
static HWND hwndTextDlg  = INVALID_HANDLE_VALUE;
static HANDLE hRichEditLib = INVALID_HANDLE_VALUE;

static BOOL CALLBACK 
TextDialogProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch ( message ) {
    case WM_INITDIALOG:
        if ( sid_timeout ) {
            sid_timer = SetTimer( hwndDlg, ID_SID_TIMER, sid_timeout * 1000, 
                                  NULL);
        }

        hwndTextDlg  = hwndDlg;
        hwndRichEdit = GetDlgItem( hwndDlg, ID_TEXT );
        PostRichEditInitSem();
        return TRUE;

    case WM_COMMAND:
        switch ( LOWORD(wParam) ) {
        case IDOK:
            if ( sid_timeout )
                KillTimer(hwndDlg,sid_timer);
            hwndTextDlg = hwndRichEdit = INVALID_HANDLE_VALUE;
            EndDialog(hwndDlg, LOWORD(wParam));
            PostRichEditCloseSem();
            return TRUE;
        }
        break;

    case WM_CLOSE:
    case WM_TIMER:
        if ( sid_timeout )
            KillTimer(hwndDlg,sid_timer);
        hwndTextDlg = hwndRichEdit = INVALID_HANDLE_VALUE;
        EndDialog(hwndDlg, 0);
        PostRichEditCloseSem();
        return TRUE;
    }
    return FALSE;
}

/* 
 * dialog widths are measured in 1/4 character widths
 * dialog height are measured in 1/8 character heights
 */

static LRESULT
TextPopupDialog( HINSTANCE hinst, HWND hwndOwner, char * title, int h, int w)
{
    HGLOBAL hgbl;
    LPDLGTEMPLATE lpdt;
    LPDLGITEMTEMPLATE lpdit;
    LPWORD lpw;
    LPWSTR lpwsz;
    LRESULT ret;
    int nchar, i, j, font = 12;
    char * p;

    if ( !w ) w = 80;
    if ( !h ) h = 25;

    hgbl = GlobalAlloc(GMEM_ZEROINIT, 8192);
    if (!hgbl)
        return -1;
 
    lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
 
    // Define a dialog box.
 
    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU
                   | DS_MODALFRAME | WS_CAPTION | DS_CENTER 
                   | DS_SETFOREGROUND | DS_3DLOOK
                   | DS_SETFONT | DS_NOFAILCREATE;
    lpdt->cdit = 2;  // number of controls
    lpdt->x  = 0;  
    lpdt->y  = 0;
    lpdt->cx = 14 + w * 4; 
    lpdt->cy = 32 + h * font;

    lpw = (LPWORD) (lpdt + 1);
    *lpw++ = 0;   // no menu
    *lpw++ = 0;   // predefined dialog box class (by default)

    lpwsz = (LPWSTR) lpw;   /* caption */
    nchar = MultiByteToWideChar (CP_ACP, 0, title, -1, lpwsz, 128);
    lpw   += nchar;
    *lpw++ = font;                        // font size (points)
    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "Lucida Console", -1, lpwsz, 128);
    lpw   += nchar;


    /*-----------------------
     * Define a Rich Edit 2.0 control.
     *-----------------------*/
    lpw = lpwAlign (lpw); /* align DLGITEMTEMPLATE on DWORD boundary */
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | 
                   ES_NUMBER | WS_BORDER | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP |
                   WS_VISIBLE;
    lpdit->dwExtendedStyle = 0; // WS_EX_CLIENTEDGE;
    lpdit->x  = 7; 
    lpdit->y  = 7;
    lpdit->cx = w * 4; 
    lpdit->cy = h * font;
    lpdit->id = ID_TEXT;

    lpw = (LPWORD) (lpdit + 1);
    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "RichEdit20W", -1, lpwsz, 50);
    lpw   += nchar;

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "", -1, lpwsz, 50);
    lpw   += nchar;

    *lpw++ = 0;           // no creation data
    
    //-----------------------
    // Define an OK button.
    //-----------------------
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP;
    lpdit->dwExtendedStyle = 0;
    lpdit->x  = 7 + w * 2 - 25; 
    lpdit->y  = 14 + h * font;
    lpdit->cx = 50; 
    lpdit->cy = 14;
    lpdit->id = IDOK;  // OK button identifier

    lpw = (LPWORD) (lpdit + 1);
    *lpw++ = 0xFFFF;
    *lpw++ = 0x0080;    // button class

    lpwsz = (LPWSTR) lpw;
    nchar = MultiByteToWideChar (CP_ACP, 0, "OK", -1, lpwsz, 50);
    lpw   += nchar;
    *lpw++ = 0;           // no creation data

    GlobalUnlock(hgbl); 
    ret = DialogBoxIndirect(hinst, (LPDLGTEMPLATE) hgbl, 
                            hwndOwner, (DLGPROC) TextDialogProc); 
    GlobalFree(hgbl); 

    switch ( ret ) {
    case 0:     /* Timeout */
        return -1;
    case IDOK:
        return 1;
    default: {
        char buf[256];
        ckmakmsg(buf,256,"DialogBoxIndirect() failed ",ckitoa(GetLastError()), NULL, NULL);
        MessageBox(hwndOwner,
                    buf,
                    "GetLastError()",
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
        return -1;
    }
    }
}


static struct {
    char title[256];
    int  h;
    int  w;
} gui_text_params;

void 
gui_text_popup_thread(void * dummy)
{
    TextPopupDialog( hInstance, hwndConsole, 
                     gui_text_params.title, gui_text_params.h, gui_text_params.w );
}

static USHORT * EditStreamBuffer = NULL;
static int EditStreamMaxLen = 0;
static int EditStreamLen = 0;
static int stream_complete = 0;

static DWORD CALLBACK
EditStreamCallback(DWORD_PTR dwCookie,
                          LPBYTE pbBuff,
                          LONG cb,
                          LONG *pcb
                          )
{
  start:
    RequestRichEditMutex(INFINITE);
    if ( stream_complete && EditStreamLen == 0 ) {
        *pcb = 0;
    } else {
        if ( EditStreamLen == 0 ) {
            ReleaseRichEditMutex();
            Sleep(100);
            goto start;
        }

        if ( cb % 2 == 1 )
            cb--;

        if ( cb >= EditStreamLen * 2 ) {
            memcpy(pbBuff, EditStreamBuffer, EditStreamLen * 2);
            *pcb = EditStreamLen * 2;
            EditStreamLen = 0;
        } else {
            int i, j;
            memcpy(pbBuff, EditStreamBuffer, cb);
            *pcb = cb;
            for (i = 0, j = cb / 2; j <= EditStreamLen; i++, j++) {
                EditStreamBuffer[i] = EditStreamBuffer[j];
            }
            EditStreamLen = i;
        }
    }
    ReleaseRichEditMutex();
    return 0;
}

static EDITSTREAM EditStream = { 0, 0, EditStreamCallback };

int
gui_text_popup_create(char * title, int h, int w)
{
    HANDLE thread;

    if ( hRichEditLib == INVALID_HANDLE_VALUE ) {
        hRichEditLib = LoadLibrary("riched20.dll");
        if ( hRichEditLib == INVALID_HANDLE_VALUE ) {
            MessageBox(hwndConsole,
                        "The library RICHED20.DLL required for GUI Text Edit controls is not present on this system",
                        "Error",
                        MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
            return -1;
        }
    }

    CreateRichEditInitSem( FALSE );
    CreateRichEditCloseSem( FALSE );
    CreateRichEditMutex(FALSE);
    stream_complete = 0;

    gui_text_params.h = h;
    gui_text_params.w = w;
    ckstrncpy(gui_text_params.title,title,256);

    thread = (HANDLE) _beginthread( gui_text_popup_thread, 65535, NULL ) ;
    if ( thread != INVALID_HANDLE_VALUE )
        WaitRichEditInitSem( INFINITE );
    CloseRichEditInitSem();

    if ( hwndRichEdit != INVALID_HANDLE_VALUE ) {
        PostMessage(hwndRichEdit, EM_AUTOURLDETECT, (WPARAM) TRUE, (LPARAM) 0);
        PostMessage(hwndRichEdit, EM_STREAMIN, (WPARAM)SF_TEXT | SF_UNICODE, (LPARAM)&EditStream);
    }
    return 0;
}

int 
gui_text_popup_append(unsigned short uch)
{
  start:
    RequestRichEditMutex(INFINITE);
    if ( EditStreamMaxLen == 0 ) {
        EditStreamBuffer = malloc(4096 * sizeof(USHORT));
        EditStreamMaxLen = 4096;
        EditStreamLen = 0;
    }

    if ( EditStreamLen < EditStreamMaxLen ) {
        static int found_cr = 0;
        if ( uch == CR ) {
            found_cr = 1;
            EditStreamBuffer[EditStreamLen++] = 0x2028;
        } else if ( uch == LF ) {
            if ( !found_cr )
                EditStreamBuffer[EditStreamLen++] = 0x2028;
            found_cr = 0;
        } else {
            EditStreamBuffer[EditStreamLen++] = uch;
            found_cr = 0;
        }
    } else {
        ReleaseRichEditMutex();
        Sleep(100);
        goto start;
    }
    ReleaseRichEditMutex();
    return 0;
}

int
gui_text_popup_close(void)
{
    if ( hwndTextDlg != INVALID_HANDLE_VALUE )
        PostMessage(hwndTextDlg, WM_CLOSE, 0, 0);
    CloseRichEditCloseSem();
    CloseRichEditMutex();
    free(EditStreamBuffer);
    EditStreamBuffer = NULL;
    EditStreamLen = EditStreamMaxLen = 0;
    return 0;
}

int
gui_text_popup_wait(int seconds)
{
    RequestRichEditMutex(INFINITE);
    stream_complete = 1;
    ReleaseRichEditMutex();

    if ( hwndRichEdit != INVALID_HANDLE_VALUE ) {
        WaitRichEditCloseSem(seconds == -1 ? INFINITE : seconds * 1000);
        return 0;
    }
    return -1;
}
#endif /* KUI */
