#ifndef ksplash_hxx_included
#define ksplash_hxx_included
/*========================================================================
    Copyright (c) 1996, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: splash.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: February 13, 1996

    Description: Splash Screen

========================================================================*/
#include "kuidef.h"

class KSplash
{
public:
    KSplash( HINSTANCE );
    ~KSplash();

    void createWin( int parentx, int parenty );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );
    void init();
	HWND hwnd() { return hWnd; }

protected:
	HBITMAP LoadDIBitmap( HINSTANCE hInst, LPCTSTR bitmapname );
	HPALETTE CreateDIBPalette( LPBITMAPINFO lpbmi, LPINT lpiNumColors );

private:
    HINSTANCE hInst;
    HWND hWnd;

    HDC hdc;
    HDC hMemDC;
    HPALETTE hPalette;
    HBITMAP hBitmap;

	int bitmapWidth;
	int bitmapHeight;
	UINT timerID;

	HWND hwndTerminal;
};

#endif
