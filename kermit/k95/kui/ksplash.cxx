#include "ksplash.hxx"
#include "kfont.hxx"
#include "ksysmets.hxx"
#include "ikterm.h"
#include "khwndset.hxx"

extern "C" {
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL CALLBACK SplashProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if( splash )
        return (BOOL)splash->message( hwnd, msg, wParam, lParam );
    return (BOOL)FALSE;
}
}   // end extern "C"
 
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSplash::KSplash( HINSTANCE h )
    : hInst( h )
    , hWnd( 0 )
{
    hdc = 0;
    hMemDC = 0;
    hPalette = 0;
    hBitmap = 0;
    bitmapWidth = 0;
    bitmapHeight = 0;
    timerID = 0;
    hwndTerminal = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSplash::~KSplash()
{
//    DeleteObject( hBitmap );
//    DeleteDC( hMemDC );
//    ReleaseDC( hWnd, hdc );
//    DeleteObject( hPalette );

//	if( timerID )
//		KillTimer( hWnd, timerID );

//    if( hWnd )
//        DestroyWindow( hWnd );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSplash::createWin( int parentx, int parenty )
{
    CreateDialog( hInst, 
		  MAKEINTRESOURCE(IDD_SPLASH), 
		  NULL, 
		  SplashProc );

    // place the splash screen in the middle of the terminal window
    //
    int clientWidth = 0, clientHeight = 0;
    ::getDimensions( VTERM, &clientWidth, &clientHeight );
    if( clientWidth <= 0 && clientHeight <= 0 ) {
        clientWidth = 80;
        clientHeight = 24;
    }

    KFont font( (PLOGFONT)0 );
    font.setFont( hdc );

    int maxWidth = (clientWidth * font.getFontW()) 
        + kglob->sysMets->vscrollWidth() 
        + (2 * kglob->sysMets->edgeWidth());

    int maxHeight = (clientHeight * font.getFontH())
        + kglob->sysMets->hscrollHeight()
        + (2 * kglob->sysMets->edgeHeight())
        + (3 * kglob->sysMets->captionHeight())
        + kglob->sysMets->menuHeight()
        + (2 * kglob->sysMets->sizeframeHeight());

    int x = ((maxWidth - bitmapWidth) / 2) + parentx;
    int y = ((maxHeight - bitmapHeight) /2) + parenty;

    SetWindowPos( hWnd, HWND_TOP, x, y, 0, 0
        , SWP_NOSIZE | SWP_SHOWWINDOW );

    MSG msg;
    while( GetMessage( (LPMSG) &msg, NULL, 0, 0 ) != FALSE )
    {
        if ( msg.message == WM_KILLSPLASHSCREEN ) {
	    hwndTerminal = (HWND)(msg.lParam);
	    SetTimer( hWnd, IDD_SPLASH, 500, NULL );
	    continue;
	}

        TranslateMessage( (LPMSG) &msg );
        DispatchMessage( (LPMSG) &msg );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSplash::init()
{
    hBitmap = LoadDIBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAPKERMIT95) );

    hdc = GetDC( hWnd );
    hMemDC = CreateCompatibleDC( hdc );

    SelectPalette( hdc, hPalette, FALSE );
    RealizePalette( hdc );
    SelectPalette( hMemDC, hPalette, FALSE );
    RealizePalette( hMemDC );

    SelectObject( hMemDC, (HGDIOBJ)hBitmap );
    BitBlt( hdc, 0, 0, bitmapWidth, bitmapHeight, hMemDC, 0, 0, SRCCOPY );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
HBITMAP KSplash::LoadDIBitmap( HINSTANCE hInst, LPCTSTR bitmapname )
{
    HRSRC  hRsrc;
    HGLOBAL hGlobal;
    HBITMAP hBitmapFinal = NULL;
    LPBITMAPINFOHEADER  lpbi;
    HDC hdc;
    int iNumColors;
 
    if (hRsrc = FindResource(hInst, bitmapname, RT_BITMAP))
       {
       hGlobal = LoadResource(hInst, hRsrc);
       lpbi = (LPBITMAPINFOHEADER)LockResource(hGlobal);
 
       hdc = GetDC(NULL);
       hPalette =  CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
       if (hPalette)
          {
          SelectPalette(hdc,hPalette,FALSE);
          RealizePalette(hdc);
          }
 
       hBitmapFinal = CreateDIBitmap(hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
                   (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
 
       bitmapWidth = lpbi->biWidth;
       bitmapHeight = lpbi->biHeight;

       ReleaseDC(NULL,hdc);
       UnlockResource(hGlobal);
       FreeResource(hGlobal);
       }
    return (hBitmapFinal);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
HPALETTE KSplash::CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors)
{
   LPBITMAPINFOHEADER  lpbi;
   LPLOGPALETTE     lpPal;
   HPALETTE         hPal = NULL;
   int              i;
   char* buf;
 
   lpbi = (LPBITMAPINFOHEADER)lpbmi;
   if (lpbi->biBitCount <= 8)
       *lpiNumColors = (1 << lpbi->biBitCount);
   else
       *lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
   if (*lpiNumColors)
      {
      buf = new char [GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors)];
	  lpPal = (LPLOGPALETTE) buf;

      lpPal->palVersion    = 0x300;
      lpPal->palNumEntries = *lpiNumColors;
 
      for (i = 0;  i < *lpiNumColors;  i++)
         {
         lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
         lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
         lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
         lpPal->palPalEntry[i].peFlags = 0;
         }
      hPal = CreatePalette (lpPal);
	  delete buf;
   }
   return hPal;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KSplash::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
    case WM_INITDIALOG:
	hWnd = hwnd;
	init();
	break;

    case WM_PAINT: {
	PAINTSTRUCT ps;
	BeginPaint( hwnd, &ps );
	BitBlt( hdc, 0, 0, bitmapWidth, bitmapHeight, hMemDC, 0, 0, SRCCOPY );
	EndPaint( hwnd, &ps );
	done = TRUE;
	break;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_TIMER:
	if( timerID )
	    KillTimer( hWnd, timerID );
	PostMessage( hwnd, WM_CLOSE, 0, 0 ) ;
	break;

    case WM_CLOSE:
	if( hwndTerminal ) {
	    SetForegroundWindow( hwndTerminal );
	    ShowWindow(hWnd,SW_HIDE);
	}
	if( hWnd )
	    DestroyWindow( hWnd );
	break;

    case WM_DESTROY:
	DeleteObject( hBitmap );
	DeleteDC( hMemDC );
	ReleaseDC( hWnd, hdc );
	DeleteObject( hPalette );

	PostQuitMessage(0);
	break;

    default:
	break;
    }

    return done;
}
