#ifndef kprogres_hxx_included
#define kprogres_hxx_included

#include "kuidef.h"

class KProgress
{
public:
    KProgress();
    ~KProgress();

    void createWin( HWND, HWND );
    void paint( int );

private:
    HWND parHwnd;

    HDC hdc;
    HDC hMemDC;
    HBITMAP hBitmap;
    HBRUSH fillBrush;
    HBRUSH backBrush;
    HPEN whitePen;
    HPEN dkgrayPen;
    
    int percent;
    int width;
    int height;
    RECT progressRect;

    COLORREF back;
    COLORREF fore;
};

#endif
