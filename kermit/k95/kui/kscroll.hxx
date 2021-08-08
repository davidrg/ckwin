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
    KScroll( K_GLOBAL*, bool isVertical, bool thumbtrack );
    ~KScroll();

    void createWin( KWin* par );
    bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void setRange( int max, int viewable, bool trackPos = TRUE );
    void setValues( long line, long page );
    void setCallback( KWinMethod );

    bool lineDown();
    bool lineUp();
    bool pageDown();
    bool pageUp();
    bool track( long val );
    bool endScroll( long val );

    void setPos( long val );
    long getPos();
    bool isVisible();

private:
    bool vertical;
    bool thumbTrack;
    long minVal;
    long maxVal;
    long currentPos;
    long lineAmt;
    long pageAmt;
    KWinMethod method;

    long prevMaxVal;
    bool disableNoScroll;
};

#endif
