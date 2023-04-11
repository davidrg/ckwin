#include "kdwnload.hxx"
#include "commdlg.h"
#include "ikextern.h"
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME);
#endif

extern "C" {
    /* This is declared in ckotio.c and set to 1 when we're on NT 3.51
     * (and 3.50 and 3.1) */
    extern int nt351;
}

KDownLoad* download;
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY KDownDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if( download )
        return (BOOL) download->message( hwnd, msg, wParam, lParam );
    return (BOOL)FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY KSaveAsDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool ret = FALSE;
    if( download ) {
        ret = download->saveAsMsg( hwnd, msg, wParam, lParam );
        if( !ret ) {
            return CallWindowProc( download->getOldProc()
                    , hwnd, msg, wParam, lParam );
        }
    }
    return (BOOL)ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KDownLoad::KDownLoad( K_GLOBAL* kg, BOOL dlButton, BOOL openExisting )
    : KWin( kg )
    , hdownload( 0 )
    , downloadID( 999 )     // unique identifier for download button
    , optionID( 0x40E )     // it's not IDHELP!
    , downloadButton(dlButton)
    , openFile(openExisting)
{
#ifndef CKT_NT35_OR_31
    OSVERSIONINFO osverinfo ;
#endif /* CKT_NT35_OR_31 */

    oldSaveAsProc = (WNDPROC)0;
    download = this;
    memset( &OpenFileName, '\0', sizeof(OPENFILENAME) );
    memcpy(szFilter,"All Files (*.*)\0*.*\0",21);
    szFile[0] = 0;
    szDir[0] = 0;
    strcpy(szTitle,"Save File As ...");
    success = FALSE;
    errorCode = 0;

#ifndef CKT_NT35_OR_31
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;

    if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && osverinfo.dwMinorVersion == 90) {
        /* Windows ME - this supports modern file dialogs */
        downloadButton = FALSE;
    } else if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osverinfo.dwMajorVersion != 4) {
        /* Windows NT version 3.x OR Windows 2000 or newer
         * Windows 2000+ supports modern file dialogs, while NT 3.51 does not
         * support customisable file dialogs (at least, not in the way we're
         * custominsing it) */
        downloadButton = FALSE;
    }
#endif /* CKT_NT35_OR_31 */

    if (openExisting) {
        downloadButton = FALSE;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KDownLoad::~KDownLoad()
{
    DestroyWindow( hdownload );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::createWin( KWin* par )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::setInitialDirectory( char * dir )
{
    ckstrncpy(szDir,dir,MAX_PATH);

    /* Make sure we're using DOS/Windows path separators for the initial
     * directory */
    char* p;
    p = szDir;
    while (*p) {
        if (*p == '/') {*p = '\\'; }
        p++;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::setTitle( char * title )
{
    ckstrncpy(szTitle,title,MAX_PATH);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::setInitialFileName( char * file )
{
    ckstrncpy(szFile,file,MAX_PATH);

    /* Make sure we're using DOS/Windows path separators for the initial
     * filename */
    char* p;
    p = szFile;
    while (*p) {
        if (*p == '/') {*p = '\\'; }
        p++;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::show( Bool bVisible )
{
    char szCurrentDir[MAX_PATH];

    OpenFileName.lStructSize       = OPENFILENAME_SIZE_VERSION_400;
    OpenFileName.hwndOwner         = parent->hwnd();
    OpenFileName.hInstance         = hInst;
    OpenFileName.lpstrFilter       = szFilter;
    OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
    OpenFileName.nMaxCustFilter    = 0L;
    OpenFileName.nFilterIndex      = 0L;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = MAX_PATH;
    OpenFileName.lpstrFileTitle    = 0;
    OpenFileName.nMaxFileTitle     = 0;
    OpenFileName.lpstrInitialDir   = szDir[0] ? szDir : szCurrentDir;
    OpenFileName.lpstrTitle        = szTitle[0] ? szTitle : 0;
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = 0;
    OpenFileName.lCustData         = 0;

    OpenFileName.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY
            | OFN_NOCHANGEDIR | OFN_SHAREAWARE ;

    if (!openFile) {
        OpenFileName.Flags = OpenFileName.Flags | OFN_CREATEPROMPT
                | OFN_OVERWRITEPROMPT | OFN_NOTESTFILECREATE;
    }


    OpenFileName.lpfnHook = 0;
    if (!nt351 && downloadButton) {
        /* NT 3.51 and earlier don't support OFNHOOKPROC */
        OpenFileName.Flags = OpenFileName.Flags | OFN_ENABLEHOOK
#ifndef CKT_NT35_OR_31
            | OFN_EXPLORER
#endif
            ;
        OpenFileName.lpfnHook = (LPOFNHOOKPROC)KDownDlgProc;
    }

//    OpenFileName.lpTemplateName = (LPTSTR)MAKEINTRESOURCE(IDD_DOWNLOAD);

    GetCurrentDirectory(sizeof(szCurrentDir),szCurrentDir);

    if (openFile) {
        success = GetOpenFileName(&OpenFileName);
    } else {
        success = GetSaveFileName(&OpenFileName);
    }
    errorCode = GetLastError();
    SetCurrentDirectory(szCurrentDir);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KDownLoad::initDialog( HWND hwnd )
{
    hWnd = hwnd;
    hwndpar = GetParent( hwnd );
    HWND hsave = GetDlgItem( hwndpar, IDOK );
    HWND hhelp = GetDlgItem( hwndpar, optionID );

    SetWindowText( hhelp, "&Options" );

    // create a new 'Download' button because the 'Save' button doesn't
    // seem to behave correctly
    // can't change the text to 'Download'
    //
    RECT rect, parRect;
    GetWindowRect( hwndpar, &parRect );
    GetWindowRect( hsave, &rect );
    int x = rect.left - parRect.left - GetSystemMetrics( SM_CXDLGFRAME );
    int y = rect.top - parRect.top - GetSystemMetrics( SM_CYDLGFRAME )
        - GetSystemMetrics( SM_CYCAPTION );
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    hdownload = CreateWindowEx( WS_EX_NOPARENTNOTIFY
                 , "Button"
                 , "&Download"
                 , WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON
                 , x, y, w, h
                 , hwndpar
                 , (HMENU) downloadID
                 , hInst
                 , (LPVOID) 0 );

    // set the default pushbutton
    SendMessage( hsave, BM_SETSTYLE, (WPARAM) BS_PUSHBUTTON, (LPARAM)1L );
    SendMessage( hwndpar, DM_SETDEFID, (WPARAM)downloadID, (LPARAM)0 );

    // set the font
    HANDLE curfont = (HANDLE) SendMessage( hsave, WM_GETFONT
            , (WPARAM)0, (LPARAM)0 );
    SendMessage( hdownload, WM_SETFONT, (WPARAM)curfont, MAKELPARAM( FALSE, 0 ) );
    SetWindowPos( hdownload, hsave, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
    ShowWindow( hdownload, SW_SHOWNORMAL );

    ShowWindow( hsave, SW_HIDE );
    ShowWindow( hhelp, SW_HIDE );

    HWND hstc2 = GetDlgItem( hwndpar, stc2 );
    ShowWindow( hstc2, SW_HIDE );

    HWND hcmb1 = GetDlgItem( hwndpar, cmb1 );
    ShowWindow( hcmb1, SW_HIDE );

    // subclass the 'save as' dialog proc
    // WNDPROC ???? it doesn't work with DLGPROC !!!! don't know why
    //
#ifdef _WIN64
    oldSaveAsProc = (WNDPROC) SetWindowLongPtr(
            hwndpar, DWLP_DLGPROC, (LONG_PTR) KSaveAsDlgProc );
#else /* _WIN64 */
    oldSaveAsProc = (WNDPROC) SetWindowLong( hwndpar, DWL_DLGPROC, (LONG) KSaveAsDlgProc );
#endif /* _WIN64 */
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KDownLoad::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_INITDIALOG:
            initDialog( hwnd );
            done = TRUE;
            break;

//        case WM_NOTIFY:
//        {
//            LPOFNOTIFY not = (LPOFNOTIFY)lParam;
//            switch( not->hdr.code )
//            {
//                case CDN_FILEOK:
//                {
//                    // don't close the dialog box
//                    //SetWindowLong( hwnd, DWL_MSGRESULT, 1L );
//                    done = TRUE;
//                    break;
//                }
//                case CDN_INITDONE:
//                    break;
//                case CDN_SELCHANGE:
//                case CDN_TYPECHANGE:
//                case CDN_FOLDERCHANGE:
//                default:
//                    break;
//            }
//            break;
//        }
    
    }
    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KDownLoad::saveAsMsg( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    if( msg == WM_COMMAND )
    {
        int cid = LOWORD(wParam);
        if( cid == downloadID )
        {
            // EndDialog( hwndpar, TRUE );
            // set the current directory for downloading
            //

            // issue the 'recieve' command
            //

            done = TRUE;
            return CallWindowProc( getOldProc()
                    , hwnd, msg, MAKEWPARAM(IDOK,0), lParam );
        }
    }

    return done;
}

