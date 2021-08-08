#include "kscroll.hxx"
#include "ikextern.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KScroll::KScroll( K_GLOBAL* kg, bool isVertical, bool thumbtrack )
    : KWin( kg )
    , vertical( isVertical )
    , thumbTrack( thumbtrack )
    , minVal( 0L )
    , maxVal( 0L )
    , currentPos( 0L )
    , lineAmt( 1L )
    , pageAmt( 1L )
    , method( 0 )
    , prevMaxVal( 0L )
    , disableNoScroll( 0 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KScroll::~KScroll()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KScroll::createWin( KWin* par )
{
    parent = par;
    setRange( 0, 0 );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KScroll::setCallback( KWinMethod m )
{
    method = m;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KScroll::setValues( long line, long page )
{
    lineAmt = line > 0 ? line : 1;
    pageAmt = page > 0 ? page : 1;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KScroll::setRange( int max, int viewable, bool trackPos )
{
    if ( !parent )
        return;

    maxVal = max - 1/* viewable */;

    if( maxVal == prevMaxVal )
        return;

    if( vertical ) {
        if( max == viewable )
            currentPos = 0;
        else
            currentPos += maxVal - prevMaxVal;
    }
    prevMaxVal = maxVal;

    UINT mask = SIF_PAGE | SIF_RANGE | SIF_POS;
    if( vertical ) {
        mask |= SIF_DISABLENOSCROLL;
        disableNoScroll = 1;
    }

    SCROLLINFO info;
    memset( &info, '\0', sizeof(SCROLLINFO) );
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = mask;
    if ( tt_scroll ) {
        info.nMin = minVal;
        info.nMax = maxVal;
        info.nPage = viewable;
        info.nPos = currentPos;
    }

    SetScrollInfo( parent->hwnd()
        , vertical ? SB_VERT : SB_HORZ
        , &info
        , TRUE );

    if( trackPos )
        track( currentPos );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::isVisible()
{
    if ( !parent )
        return false;

    SCROLLINFO info;
    memset( &info, '\0', sizeof(SCROLLINFO) );
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = SIF_RANGE | SIF_PAGE;
    GetScrollInfo( parent->hwnd()
                   , vertical ? SB_VERT : SB_HORZ
                   , &info );

    if (!disableNoScroll && info.nPage && (info.nMax <= (int)info.nPage))
        return false;

    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
long KScroll::getPos()
{
    return currentPos;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KScroll::setPos( long val )
{
    if ( !parent )
        return;

    currentPos = val;
    SetScrollPos( parent->hwnd()
        , vertical ? SB_VERT : SB_HORZ
        , val
        , TRUE );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::track( long val )
{
    setPos( val );
    if( parent && method )
        (parent->*method) ( val );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::endScroll( long val )
{
    setPos( val );
    if( parent && method )
        (parent->*method) ( val );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::lineDown()
{
    long i = getPos() + lineAmt;
    if( i > maxVal ) {
        i = maxVal;
    }
    else if( i < minVal ) {
        i = minVal;
    }
    track( i );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::lineUp()
{
    long i = getPos() - lineAmt;
    if( i < minVal ) {
        i = minVal;
    }
    else if( i > maxVal ) {
        i = maxVal;
    }
    track( i );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::pageDown()
{
    long i = getPos() + pageAmt;
    if( i > maxVal ) {
        i = maxVal;
    }
    else if( i < minVal ) {
        i = minVal;
    }
    track( i );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::pageUp()
{
    long i = getPos() - pageAmt;
    if( i < minVal ) {
        i = minVal;
    }
    else if( i > maxVal ) {
        i = maxVal;
    }
    track( i );
    return true;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
bool KScroll::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    int     nScrollCode   = (int)     LOWORD(wParam);
    short   nPos          = (short)   HIWORD(wParam);
//    HWND    hwndScrollBar = (HWND)    lParam;

    bool done = false;
    switch( nScrollCode )
    {
        case SB_LINEUP:
            done = lineUp();
            break;
        case SB_LINEDOWN:
            done = lineDown();
            break;
        case SB_PAGEUP:
            done = pageUp();
            break;
        case SB_PAGEDOWN:
            done = pageDown();
            break;
        case SB_THUMBPOSITION:
            done = endScroll( nPos );
            break;
        case SB_THUMBTRACK:
            if( thumbTrack )
                done = track( nPos );
            break;
        case SB_ENDSCROLL:
            if( thumbTrack )
                done = endScroll( getPos() );
            break;
    }

    return done;
}

