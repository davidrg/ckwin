#include "kwin.hxx"
#include "khwndset.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KWin::KWin( K_GLOBAL* kg )
    : hWnd( 0 )
    , kglob( kg )
    , parent( 0 )
    , hInst( kg->hInst )
    , _inCreate( FALSE )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KWin::~KWin()
{
    kglob->hwndset->remove( this );
    if( hWnd )
        DestroyWindow( hWnd );
}

/*------------------------------------------------------------------------
    initialize child windows
------------------------------------------------------------------------*/
void KWin::init()
{
}

/*------------------------------------------------------------------------
    virtual callback used by createWin() to retrieve window info
------------------------------------------------------------------------*/
void KWin::getCreateInfo( K_CREATEINFO* info )
{
}

/*------------------------------------------------------------------------
    create the window
------------------------------------------------------------------------*/
void KWin::createWin( KWin* par )
{
    parent = par;

    K_CREATEINFO info;
    memset( &info, '\0', sizeof( K_CREATEINFO ) );
    getCreateInfo( &info );

    hWnd = CreateWindowEx( info.exStyle
                         , info.classname
                         , info.text
                         , info.style
                         , info.x
                         , info.y
                         , info.width
                         , info.height
                         , parent ? parent->hwnd() : NULL
                         , (HMENU) info.objId
                         , kglob->hInst
                         , (LPVOID) 0 );

    associateHwnd( hWnd );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::associateHwnd( HWND hwnd )
{
    kglob->hwndset->add( this, hwnd );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::show( Bool bVisible )
{
    ShowWindow( hWnd, bVisible ? SW_SHOW : SW_HIDE );
}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::takeFocus()
{
    if (SetFocus( hWnd ) == NULL)
        SetForegroundWindow(hWnd);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::size( int width, int height )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::getSize( int& w, int& h )
{
    RECT rect;
    GetWindowRect( hWnd, &rect );
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KWin::getPos( int& x, int& y )
{
    RECT rect;
    GetWindowRect( hWnd, &rect );
    x = rect.left;
    y = rect.top;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KWin::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    return FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KWin::paint()
{
    return FALSE;
}
