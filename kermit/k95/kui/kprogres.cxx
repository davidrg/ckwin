#include "kprogres.hxx"
#include "ksysmets.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KProgress::KProgress()
    : percent( 0 )
    , width( 0 )
    , height( 0 )
    , hdc( 0 )
    , hMemDC( 0 )
    , hBitmap( 0 )
    , fillBrush( 0 )
    , backBrush( 0 )
    , whitePen( 0 )
    , dkgrayPen( 0 )
    , parHwnd( 0 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KProgress::~KProgress()
{
    DeleteObject( whitePen );
    DeleteObject( dkgrayPen );
    DeleteObject( fillBrush );
    DeleteObject( backBrush );
    DeleteObject( hBitmap );
    DeleteDC( hMemDC );
    ReleaseDC( parHwnd, hdc );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KProgress::createWin( HWND ph, HWND h )
{
    parHwnd = ph;
    RECT prect, rect;
    GetWindowRect( parHwnd, &prect );
    GetWindowRect( h, &rect );
    hdc = GetDC( parHwnd );

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    progressRect.top    = rect.top - prect.top - kglob->sysMets->captionHeight();
    progressRect.left   = rect.left - prect.left;
    progressRect.right  = progressRect.left + width;
    progressRect.bottom = progressRect.top + height;

    hMemDC = CreateCompatibleDC( hdc );
    hBitmap = CreateCompatibleBitmap( hdc, width, height * 3 );

    SelectObject( hMemDC, hBitmap );
    SetBkMode( hMemDC, TRANSPARENT );

    back = RGB(192,192,192);
    fore = RGB(0,0,128);

    fillBrush = CreateSolidBrush( back );
    backBrush = CreateSolidBrush( fore );

    whitePen = (HPEN) GetStockObject( WHITE_PEN );
    dkgrayPen = CreatePen( PS_SOLID, 0, RGB(130,130,130) );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KProgress::paint( int p )
{
    if( p >= 0 )
        percent = p;

    char c[10];
    _ltoa( percent, c, 10 );
    strcat( c, " %" );

    int gradepercent = (width * percent) / 100;

    RECT rect1;
    rect1.top = 0;
    rect1.left = 0;
    rect1.bottom = height;
    rect1.right = width;
    FillRect( hMemDC, &rect1, backBrush );

    SetTextColor( hMemDC, back );
    DrawTextEx( hMemDC, c, -1, &rect1
        , DT_VCENTER | DT_SINGLELINE | DT_CENTER, NULL );

    RECT rect2;
    rect2.top = rect1.bottom + 10;
    rect2.left = 0;
    rect2.bottom = rect2.top + height;
    rect2.right = width;
    FillRect( hMemDC, &rect2, fillBrush );

    SetTextColor( hMemDC, fore );
    DrawTextEx( hMemDC, c, -1, &rect2
        , DT_VCENTER | DT_SINGLELINE | DT_CENTER, NULL );

    //combine images
    //
    StretchBlt( hMemDC, gradepercent, 0, width - gradepercent, height
        , hMemDC, gradepercent, rect2.top
        , width - gradepercent, height, SRCCOPY );

    // draw the border
    //
    SelectObject( hMemDC, dkgrayPen );
    MoveToEx( hMemDC, 0, height - 1, NULL );
    LineTo( hMemDC, 0, 0 );
    LineTo( hMemDC, width - 1, 0 );

    SelectObject( hMemDC, whitePen );
    MoveToEx( hMemDC, 0, height - 1, NULL );
    LineTo( hMemDC, width - 1, height - 1 );
    LineTo( hMemDC, width - 1, -1 );

    // blt it to the screen
    //
    StretchBlt( hdc, progressRect.left, progressRect.top
        , width, height, hMemDC, 0, 0, width, height, SRCCOPY );
}
