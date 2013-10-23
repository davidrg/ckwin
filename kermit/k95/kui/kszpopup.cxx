#include "kszpopup.hxx"
#include "kfont.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSizePopup::KSizePopup( K_GLOBAL* k )
    : KWin( k )
    , font( 0 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSizePopup::~KSizePopup()
{
    delete font;
    ReleaseDC( hWnd, hdc );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSizePopup::getCreateInfo( K_CREATEINFO* info )
{
    info->classname = KWinClassName;
    info->exStyle   = WS_EX_TOPMOST;
    info->style     = WS_POPUP | WS_BORDER;
    info->x         = 0;
    info->y         = 0;
    info->width     = 100;
    info->height    = 100;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSizePopup::createWin( KWin* par )
{
    inCreate( TRUE );
    KWin::createWin( par );

    hdc = GetDC( hWnd );
    LOGFONT logfont;
    if( SystemParametersInfo( SPI_GETICONTITLELOGFONT
                    , sizeof( LOGFONT )
                    , &logfont
                    , 0 ) ) {
        font = new KFont( &logfont );
        font->setFont( hdc );
    }

    inCreate( FALSE );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSizePopup::paint( int w, int h )
{
    sprintf( tmp, " %d x %d ", w, h );
    size_t len = strlen( tmp );
    SIZE size;
    GetTextExtentPoint32( hdc, tmp, len, &size );

    TextOut( hdc, 0, 0, tmp, len );

    RECT rect;
    GetWindowRect( parent->hwnd(), &rect );
    int x = rect.left + (( (rect.right - rect.left) - size.cx ) / 2);
    int y = rect.top + (( (rect.bottom - rect.top) - size.cy ) / 2);

    SetWindowPos( hWnd, 0, x, y, size.cx + 2, size.cy + 2
        , SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE );
}
