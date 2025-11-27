#include "kscroll.hxx"
#include "ikextern.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KScroll::KScroll( K_GLOBAL* kg, Bool isVertical, Bool thumbtrack )
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
	, prevTtScroll( 1 )
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
void KScroll::setRange( int max, int viewable, Bool trackPos )
{
    int scroll_enabled;
    if ( !parent )
        return;

    scroll_enabled = tt_scroll;
    if (vmode == VTERM && !tt_term_scroll) {
        scroll_enabled = 0;
    }

    maxVal = max - 1/* viewable */;

    if( maxVal == prevMaxVal && scroll_enabled == prevTtScroll )
        return;

    if( vertical ) {
        if( max == viewable )
            currentPos = 0;
        else
            currentPos += maxVal - prevMaxVal;
    }
    prevMaxVal = maxVal;
	prevTtScroll = scroll_enabled;

#ifndef CKT_NT35_OR_31
    UINT mask = SIF_PAGE | SIF_RANGE | SIF_POS;
    if( vertical ) {
        /* This is what hides (or doesn't show) the horizontal scroll bar */
        mask |= SIF_DISABLENOSCROLL;
        disableNoScroll = 1;
    }

    SCROLLINFO info;
    memset( &info, '\0', sizeof(SCROLLINFO) );
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = mask;
    if ( scroll_enabled ) {
        info.nMin = minVal;
        info.nMax = maxVal;
        info.nPage = viewable;
        info.nPos = currentPos;
    }

    SetScrollInfo( parent->hwnd()
        , vertical ? SB_VERT : SB_HORZ
        , &info
        , TRUE );
#else
    SendMessage(parent->hwnd(), SBM_SETRANGE, minVal, maxVal);
    SendMessage(parent->hwnd(), SBM_SETPOS, currentPos, TRUE);
#endif /* CKT_NT35_OR_31 */

    if( trackPos )
        track( currentPos );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::isVisible()
{
    if ( !parent )
        return FALSE;

#ifndef CKT_NT35_OR_31
    SCROLLINFO info;
    memset( &info, '\0', sizeof(SCROLLINFO) );
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = SIF_RANGE | SIF_PAGE;
    GetScrollInfo( parent->hwnd()
                   , vertical ? SB_VERT : SB_HORZ
                   , &info );

    if( !disableNoScroll && info.nPage && (info.nMax <= (int)info.nPage) )
        return FALSE;

    if ( !vertical ) {
        /* As far as I can tell the horizontal scrollbar has never been present
         * in any released version of K95. The code in setRange specifically
         * enables the vertical scrollbar only, so the horizontal scrollbar
         * can never be visisble so we should *always* be returning FALSE. But
         * it turns out K95 did return TRUE here occasionally - mostly during
         * pp startup before the scrollbar was properly initialised, but also
         * after window size change if the terminal was on a page other than
         * page 0. This would cause the window to resize to accommodate a
         * horizontal scroll bar even though none was visible and there was
         * nothing to scroll. So now we just always return FALSE.
         *
         * In the future if we wanted to make the Horizontal Scrollbar work,
         * that code in setRange() would need to change, and we'd also need to
         * clear disableNoScroll on window size so that the horizontal scroll
         * bar doesn't automatically reappear on window resize before we know
         * if there is anything to scroll. */
        return FALSE;
    }
#else
    /* TODO: do the equivalent of the above */
#endif /* CKT_NT35_OR_31 */

    return TRUE;
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
Bool KScroll::track( long val )
{
    setPos( val );
    if( parent && method )
        (parent->*method) ( val );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::endScroll( long val )
{
    setPos( val );
    if( parent && method )
        (parent->*method) ( val );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::lineDown()
{
    long i = getPos() + lineAmt;
    if( i > maxVal ) {
        i = maxVal;
    }
    else if( i < minVal ) {
        i = minVal;
    }
    track( i );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::lineUp()
{
    long i = getPos() - lineAmt;
    if( i < minVal ) {
        i = minVal;
    }
    else if( i > maxVal ) {
        i = maxVal;
    }
    track( i );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::pageDown()
{
    long i = getPos() + pageAmt;
    if( i > maxVal ) {
        i = maxVal;
    }
    else if( i < minVal ) {
        i = minVal;
    }
    track( i );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::pageUp()
{
    long i = getPos() - pageAmt;
    if( i < minVal ) {
        i = minVal;
    }
    else if( i > maxVal ) {
        i = maxVal;
    }
    track( i );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KScroll::message( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    int     nScrollCode   = (int)     LOWORD(wParam);
    short   nPos          = (short)   HIWORD(wParam);
//    HWND    hwndScrollBar = (HWND)    lParam;

    Bool done = FALSE;
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

