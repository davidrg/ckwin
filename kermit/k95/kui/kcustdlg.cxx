#include "kcustdlg.hxx"
#include "kstatus.hxx"
#include "khwndset.hxx"
#include "kui.hxx"
#include "ksysmets.hxx"

char* testLBStrings[] = {
  "1one"
, "2two"
, "3three"
, "4four"
, "5five"
, "6six"
, "7seven"
, "8a very long string to see what the listbox looks like"
, "9nine"
, "10ten"
, "11eleven"
, "12twelve"
, "13thirteen"
, "14fourteen"
, "15fifteen"
, "16sixteen"
, "17seventeen"
, "18eighteen"
, "19nineteen"
, "20twenty"
, 0
};
extern "C" {
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY CustomizeDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    KWin* win = (KWin*) kglob->hwndset->find( hwnd );
    if( !win )
        return 0;

    Bool ret = win->message( hwnd, msg, wParam, lParam );
    return (BOOL) ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY CustomizeDragDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    KWin* win = (KWin*) kglob->hwndset->find( hwnd );
    if( !win )
        return 0;

    Bool ret = win->message( hwnd, msg, wParam, lParam );
    return (BOOL) ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
LRESULT CALLBACK PaneListWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    KStatusCustomDlg* win = (KStatusCustomDlg*) kglob->hwndset->find( hwnd );
    if( !win )
        return CallWindowProc( DefWindowProc, hwnd, msg, wParam, lParam );

    Bool done = FALSE;
    switch( msg )
    {
        case WM_MOUSEMOVE:
            done = win->mouseMove( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;
        case WM_RBUTTONDOWN:
            done = win->rButtonDown( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;
        case WM_RBUTTONUP:
            done = win->rButtonUp( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;
    }

    if( !done )
        return CallWindowProc( win->getLBProc(), hwnd, msg, wParam, lParam );
    
    return (LRESULT)TRUE;
}
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KStatusCustomDlg::KStatusCustomDlg( K_GLOBAL* kg, KStatus* st )
    : KWin( kg )
    , status( st )
    , LBProc( 0 )
    , dragPopup( 0 )
    , xoff( 0 )
    , yoff( 0 )
    , disposition( NONE )
{
    numEntries = 20;    // temporary for bogus data
    hList = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KStatusCustomDlg::~KStatusCustomDlg()
{
    kui->setDialogHwnd( 0 );
    if( dragPopup )
        DestroyWindow( dragPopup );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatusCustomDlg::createWin( KWin* par )
{
    parent = par;

    hWnd = CreateDialog( kglob->hInst
                , MAKEINTRESOURCE(IDD_CUSTOMIZESTATUS)
                , parent->hwnd()
                , CustomizeDlgProc );

    kui->setDialogHwnd( hWnd );

    associateHwnd( hWnd );

    hList = GetDlgItem( hWnd, IDC_LISTPANE );
    LBProc = (WNDPROC) SetWindowLong( hList, GWL_WNDPROC, (LONG) PaneListWndProc );
    associateHwnd( hList );

    // populate with dummy data for now
    //
    int i = 0;
    char* c = testLBStrings[i];
    while( c )
    {
        SendMessage( hList, LB_ADDSTRING, 0, (WPARAM)c );
        c = testLBStrings[++i];
    }
    SendMessage( hList, LB_SETCURSEL, 0, 0 );

    // popup used for the draggable status pane
    //
    dragPopup = CreateDialog( kglob->hInst
                , MAKEINTRESOURCE(IDD_CUSTOMIZESTATUSDRAG)
                , hWnd
                , CustomizeDragDlgProc );

    // reposition the dialog above the status bar
    //
    int px, py, pw, ph, x, y, w, h;
    parent->getPos( px, py );
    parent->getSize( pw, ph );
    getSize( w, h );

    RECT rect;
    GetWindowRect( status->getCustomHwnd(), &rect );

    x = ((pw - w)/2) + px;
    y = py + ph - h - 2*(rect.bottom - rect.top);

    if( y < 0 )
        y = 0;

    SetWindowPos( hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatusCustomDlg::endCustomize()
{
    show( FALSE );
    DestroyWindow( dragPopup );
    dragPopup = 0;
    EndDialog( hWnd, 0 );
    status->endCustomize();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatusCustomDlg::mouseMove( long wParam, long x, long y )
{
    Bool done = FALSE;
    if( wParam & MK_RBUTTON ) {  // right mouse button down
        RECT rect;
        GetWindowRect( hList, &rect );
        int px = rect.left;
        int py = rect.top;

        SetWindowPos( dragPopup, HWND_TOP
            , px + x - xoff, py + y - yoff
            , 0, 0
            , SWP_NOSIZE );
        done = TRUE;
    }

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatusCustomDlg::beginPaneMove( long x, long y, long xx, long yy )
{
    disposition = FROMSTATUSBAR;

    xoff = xx;
    yoff = yy;

    long eh = kglob->sysMets->edgeHeight();
    RECT rect;
    GetWindowRect( hList, &rect );
    int px = rect.left;
    int py = rect.top;

    SetCapture( hList );
    HWND customHwnd = status->getCustomHwnd();
    SetParent( customHwnd, dragPopup );

    GetWindowRect( customHwnd, &rect );

    SetWindowPos( dragPopup, HWND_TOP
        , px + x - xoff, py + y - yoff
        , rect.right - rect.left, rect.bottom - rect.top - eh
        , SWP_SHOWWINDOW );

    SetWindowPos( customHwnd, NULL, 0, -eh, 0, 0
        , SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatusCustomDlg::rButtonDown( long wParam, long x, long y )
{
    disposition = TOSTATUSBAR;

    SetCapture( hList );
    HWND customHwnd = status->getCustomHwnd();
    SetParent( customHwnd, dragPopup );

    // get the selected text from the listbox
    //
    int cursel = (int)SendMessage( hList, LB_GETCURSEL, 0, 0 );
    if( cursel == LB_ERR )
        return FALSE;
    SendMessage( hList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)dragPane.text );
    SendMessage( customHwnd, SB_SETTEXT, SBT_POPOUT, (LPARAM)dragPane.text );

    RECT rect;
    GetWindowRect( customHwnd, &rect );
    long eh = kglob->sysMets->edgeHeight();
    long h = rect.bottom - rect.top - eh;
    long iconw = kglob->sysMets->iconWidth();

    xoff = iconw / 2;
    yoff = h / 2;

    SetWindowPos( dragPopup, HWND_TOP, x, y, iconw, h
        , SWP_SHOWWINDOW );

    SetWindowPos( customHwnd, NULL, 0, -eh, iconw, h + eh
        , SWP_SHOWWINDOW | SWP_NOZORDER );

    return TRUE;
}

/*------------------------------------------------------------------------
    user released the mouse button (with capture going to listbox)
    end of drag operation:
    1. on top of the listbox (from status bar)
    2. on top of the status bar (from the listbox)
------------------------------------------------------------------------*/
Bool KStatusCustomDlg::rButtonUp( long wParam, long x, long y )
{
    ReleaseCapture();
    xoff = yoff = 0;
    HWND customHwnd = status->getCustomHwnd();

    long xpos = 0;
    RECT comprect, dragrect, interrect;
    GetWindowRect( customHwnd, &dragrect );

    KStatus::eAction action = KStatus::NOACTION;
    if( disposition == FROMSTATUSBAR ) {
        GetWindowRect( hList, &comprect );
        IntersectRect( &interrect, &comprect, &dragrect );
        if( !IsRectEmpty( &interrect ) )
            action = KStatus::REMOVEPANE;

    }
    else if( disposition == TOSTATUSBAR ) {
        GetWindowRect( status->hwnd(), &comprect );
        IntersectRect( &interrect, &comprect, &dragrect );
        if( !IsRectEmpty( &interrect ) ) {
            action = KStatus::ADDPANE;
            xpos = dragrect.left - comprect.left;
        }
    }

    // adjust the status bar
    //
    ShowWindow( dragPopup, SW_HIDE );
    ShowWindow( customHwnd, SW_HIDE );
    SetParent( customHwnd, status->hwnd() );

    if( !status->endPaneMove( action, xpos ) )
    {
        MessageBox( hWnd, "Cannot add any more panes!"
            , "Status Bar Error", MB_APPLMODAL | MB_ICONHAND | MB_OK );
        return TRUE;
    }


    if( action == KStatus::REMOVEPANE ) {
        numEntries++;
        int idx = (int)SendMessage( hList, LB_ADDSTRING, 0, (LPARAM)dragPane.text );
        SendMessage( hList, LB_SETCURSEL, (WPARAM)idx, 0 );
    }
    else if( action == KStatus::ADDPANE && numEntries > 0 ) {
        numEntries--;
        int cursel = (int)SendMessage( hList, LB_GETCURSEL, 0, 0 );
        SendMessage( hList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)dragPane.text );
        int max = (int)SendMessage( hList, LB_DELETESTRING, (WPARAM)cursel, 0 );
        if( cursel > max - 1 )
            cursel = max - 1;
        SendMessage( hList, LB_SETCURSEL, cursel, 0 );
    }

    disposition = NONE;
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatusCustomDlg::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool ret = FALSE;
    switch( msg )
    {    
        case WM_CLOSE:
            endCustomize();
            ret = TRUE;
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDOK:
                    endCustomize();
                    break;
                case IDCANCEL:
                    endCustomize();
                    break;
                case IDRESET:
                    break;
                case IDC_LISTPANE:
                {
                    if( HIWORD(wParam) == LBN_SELCHANGE ) {
                        int sel = (int) SendMessage( hList, LB_GETCURSEL, 0, 0 );
                        char buf[128];
                        SendMessage( hList, LB_GETTEXT, sel, (LPARAM)buf );
                        HWND hdesc = GetDlgItem( hWnd, IDC_STATICDESCRIPTION );
                        SetWindowText( hdesc, buf );
                    }
                    break;
                }
            }
            ret = TRUE;
            break;
    }
    return ret;
}
