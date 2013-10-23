#ifndef kscroll_hxx_included
#define kscroll_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kscroll.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: client area scrollbar.

========================================================================*/

#include "kwin.hxx"

class KScroll : public KWin
{
public:
    KScroll( K_GLOBAL*, Bool isVertical, Bool thumbtrack );
    ~KScroll();

    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void setRange( int max, int viewable, Bool trackPos = TRUE );
    void setValues( long line, long page );
    void setCallback( KWinMethod );

    Bool lineDown();
    Bool lineUp();
    Bool pageDown();
    Bool pageUp();
    Bool track( long val );
    Bool endScroll( long val );

    void setPos( long val );
    long getPos();
    Bool isVisible();

private:
    Bool vertical;
    Bool thumbTrack;
    long minVal;
    long maxVal;
    long currentPos;
    long lineAmt;
    long pageAmt;
    KWinMethod method;

    long prevMaxVal;
    Bool disableNoScroll;
};

#endif
