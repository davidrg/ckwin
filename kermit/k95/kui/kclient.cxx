#include "kclient.hxx"
#include "kappwin.hxx"
#include "kscroll.hxx"
#include "khwndset.hxx"
#include "ksysmets.hxx"
#include "kfont.hxx"
#include "karray.hxx"
#include "ikterm.h"
#include "ikcmd.h"
#include "ikextern.h"

#ifdef CK_HAVE_GDIPLUS
#include <gdiplus.h>
using namespace Gdiplus;
#endif /* CK_HAVE_GDIPLUS */

typedef struct _K_WORK_STORE {
    int               offset;
    int               length;
    int               x;
    int               y;
    cell_video_attr_t attr;
    unsigned short    effect;
} K_WORK_STORE;

extern UINT keyArray [];    // from kuikey.cxx

extern "C" {
#define K_DNONE      110		/* Screen rollback: down one line */
#define K_UPONE      112		/* Screen rollback: Up one line */

#define DECSTGLT_MONO           0
#define DECSTGLT_ALTERNATE      1
#define DECSTGLT_COLOR          3

extern int tt_cursor;
extern int cursorena[];
extern int tt_cursor_blink;
extern int tt_scrsize[];	/* Scrollback buffer size */
extern int tt_status[];
extern int tt_update;
extern vscrn_t vscrn[];
extern int scrollflag[];
extern enum markmodes markmodeflag[] ;
extern BYTE vmode;
extern int win32ScrollUp, win32ScrollDown;
extern int trueblink, trueunderline, trueitalic, truedim, truebold,
        truecrossedout;
extern int decstglt, decatcbm, decatcum;
cell_video_attr_t geterasecolor(int);
int tt_old_update;
extern int tt_sync_output;  /* ckoco3.c */
extern int tt_sync_output_timeout;

extern DWORD VscrnClean( int vmode );
extern void scrollback( BYTE, int );
extern DWORD VscrnIsDirty( int );

extern int colorpalette; /* ckoco3.c */
extern cell_video_attr_t  colorcursor;  /* ckoco3.c */

#ifdef CK_COLORS_DEBUG
extern ULONG RGBTable256[256];
#endif /* CK_COLORS_DEBUG */

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
LRESULT CALLBACK KClientWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    // debug(F111,"KClientWndProc()","msg",msg);
    KClient* client = (KClient*) kglob->hwndset->find( hwnd );
    if( client && client->message( hwnd, msg, wParam, lParam ) )
        return client->msgret();

    return CallWindowProc( DefWindowProc, hwnd, msg, wParam, lParam );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
// In Visual C++ 2002 (7.0) and up for 32bit windows, UINT_PTR is unsigned int
// In Visual C++ 6.0 (and perhaps earlier) UINT_PTR is unsigned long (which
// results in a build error)
#if defined(_MSC_VER) && _MSC_VER < 1300
VOID CALLBACK KTimerProc( HWND hwnd, UINT msg, UINT id, DWORD dwtime )
#else
VOID CALLBACK KTimerProc( HWND hwnd, UINT msg, UINT_PTR id, DWORD dwtime )
#endif
{
    /* Guard against the terminal getting 'stuck' because something turns
     * synchronised output on and leaving it on forever. */
    if (tt_sync_output) {
        if (tt_sync_output_timeout > 0) {
            tt_sync_output_timeout -= tt_update;
        }
        if (tt_sync_output_timeout <= 0) {
            tt_sync_output = FALSE;
            tt_sync_output_timeout = 0;
            VscrnIsDirty(VTERM);
        }
    }

    // debug(F111,"KTimerProc()","msg",msg);
    // debug(F111,"KTimerProc()","id",id);
    // debug(F111,"KTimerProc()","dwtime",dwtime);
    KClient* client = (KClient*) kglob->hwndset->find( hwnd );
    if( client ) {
        if( ::VscrnClean( vmode /* client->getClientID() */ )
            && (!tt_sync_output || vmode != VTERM))
            client->getDrawInfo();
        else
            client->checkBlink();
    }
#ifndef NOKVERBS
    if ( win32ScrollUp ) {
        putkverb(vmode, F_KVERB | K_UPONE);
    } else if ( win32ScrollDown ) {
        putkverb(vmode, F_KVERB | K_DNONE);
    }
#endif /* NOKVERBS */

    if (client && tt_update != tt_old_update ) {
        // Interval has changed, restart the timer.
        client->startTimer();
    }
}

}   // end of extern "C"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KClient::KClient( K_GLOBAL* kg, BYTE cid )
    : KWin( kg )
    , _hdc( 0 )
    , _hdcScreen( 0 )
    , maxpHeight( 0 )
    , maxpWidth( 0 )
    , maxHeight( 0 )
    , maxWidth( 0 )
    , _inFocus( FALSE )
    , hrgnPaint( 0 )
    , disabledBrush( 0 )
    , bgBrush( 0 )
    , savebgcolor( 0 )
    , _margin( 2 )      // default margin around display area
    , clientID( cid )
    , saveTermHeight( 0 )
    , saveTermWidth( 0 )
    , saveLineSpacing(1.0)
    , saveHorzIsV(0)
    , font( 0 )
    , processKey( FALSE )
#ifndef CK_COLORS_24BIT
    , prevAttr( cell_video_attr_init_vio_attribute(255) )
#endif /* CK_COLORS_24BIT */
    , prevEffect( uchar(-1) )
    , _xoffset( 0 )
    , _yoffset( 0 )
    , _msgret( 1 )
    , ws_blinking( 0 )
    , cursor_displayed( 0 )
{
#ifdef CK_COLORS_24BIT
#if _MSC_VER < 1800
    prevAttr = cell_video_attr_from_vio_attribute(255);
#else
     prevAttr = cell_video_attr_init_vio_attribute(255);
#endif
#endif /* CK_COLORS_24BIT */
    vert = new KScroll( kg, TRUE, TRUE );
    horz = new KScroll( kg, FALSE, TRUE );

    // structure used to pass data to the paint function
    //
    clientPaint = new K_CLIENT_PAINT;
    memset( clientPaint, '\0', sizeof(K_CLIENT_PAINT) );

    long maxcells = ::getMaxDim();
    workTempSize = allocateClientPaintBuffers(clientPaint, maxcells, &workTemp);
    textBuffer = clientPaint->textBuffer;
    attrBuffer = clientPaint->attrBuffer;
    effectBuffer = clientPaint->effectBuffer;
    lineAttr = clientPaint->lineAttr;

    workStore = new K_WORK_STORE[ maxcells ];
    memset( workStore, '\0', sizeof(K_WORK_STORE) * maxcells );

    memset( &cursorRect, '\0', sizeof(RECT) );
    cursorCount = 0;

    maxCursorCount = 1100;
    blinkInterval  = 600;

    /* Invalid vscrn ID to always use the current vscrn */
    ikterm = new IKTerm( vmode /* clientID */, clientPaint );
    wc = 0;
    vscrollpos = hscrollpos = 0;

    /* Save initial window size so we can tell when it changes */
    getEndSize(previousWidth, previousHeight);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KClient::~KClient()
{
    delete font;
    ReleaseDC( hWnd, _hdcScreen );
    DeleteDC( _hdc );
    DeleteDC( _hdcScratch );
    DeleteObject( compatBitmap );
    DeleteObject( scratchBitmap );
    DeleteObject( hrgnPaint );
    DeleteObject( disabledBrush );
    DeleteObject( bgBrush );

    if( timerID )
        KillTimer( hWnd, timerID );
    delete horz;
    delete vert;

    delete clientPaint;

    delete workStore;
    delete workTemp;

    delete ikterm;
}

void KClient::stopTimer() {
    if( timerID )
        KillTimer( hWnd, timerID );
    timerID = NULL;
}

void KClient::startTimer() {
    stopTimer();
    tt_old_update = tt_update;
    timerID = SetTimer( hWnd, IDT_CLIENTTIMER, tt_update, KTimerProc );
}

size_t KClient::allocateClientPaintBuffers(K_CLIENT_PAINT* clientPaint,
            long maxcells, uchar **workTempOut) {
    // allocate the space in one chunck so it can be cleared in one call
    //
    int column, row;
    ::getMaxSizes( &column, &row );
    //  workTempSize = (maxcells * sizeof(ushort) * 5) + (row * sizeof(ushort));  // This seems to allocate almost twice as much memory as actually needed!
    size_t workTempSize = (maxcells * ((sizeof(ushort) * 2) + sizeof(cell_video_attr_t))) + (MAXSCRNROW * sizeof(ushort));
    uchar* workTemp = new uchar[ workTempSize ];
    memset( workTemp, '\0', workTempSize);

    clientPaint->textBuffer =   (ushort*)            &(workTemp[0]); /* One per cell */
    clientPaint->attrBuffer =   (cell_video_attr_t*) &(workTemp[ maxcells * sizeof(ushort)]); /* One per cell */
    clientPaint->effectBuffer = (ushort*)            &(workTemp[maxcells * (sizeof(cell_video_attr_t) + sizeof(ushort))]); /* One per cell */
    clientPaint->lineAttr =     (ushort*)            &(workTemp[ maxcells * (sizeof(cell_video_attr_t) + 2 * sizeof(ushort))]); /* One per line */

    *workTempOut = workTemp;
    return workTempSize;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KClient::getMaxpDim( int& rw, int& rh )
{ 
    if( maxpWidth == 0 && maxpHeight == 0 )
        return FALSE;

    rw = maxpWidth;
    rh = maxpHeight;
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::setFont( KFont* f )
{
    if( font == f )
        return;

    if( font )
        delete font;
    font = f;
    font->setFont( _hdc );

    int twid, thi;
    ::getDimensions( vmode /* clientID */, &twid, &thi );

    horz->setValues( font->getFontW(), font->getFontW() * 10 );
    vert->setValues( 1, thi - tt_status[vmode]);

    setInterSpacing( f );
    setDimensions( TRUE );

    prevEffect = uchar(-1);     // reset the effect flag
	saveTermHeight = saveTermWidth = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::setInterSpacing( KFont* f )
{
    int fontw = f->getFontW();
    for( int i = 0; i < MAXNUMCOL; i++ )
        interSpace[i] = fontw;
}

/*------------------------------------------------------------------------
	calculate the appropriate size for this client window...
	includes margin and scrollbars
------------------------------------------------------------------------*/
void KClient::calculateSize( int& w, int& h )
{
    int clientWidth = 0, clientHeight = 0;
    ::getDimensions( vmode /* clientID */, &clientWidth, &clientHeight );

    w = (clientWidth * font->getFontW()) 
        + (vert->isVisible() ? kglob->sysMets->vscrollWidth() : 0) 
        + (2 * kglob->sysMets->edgeWidth()) + margin();

    // determine if the horizontal scrollbar is visible
    //
    int horzh = horz->isVisible() ? kglob->sysMets->hscrollHeight() : 0;

    h = clientHeight * font->getFontSpacedH()
        + horzh
        + (2 * kglob->sysMets->edgeHeight()) + margin();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::setDimensions( Bool sizeparent )
{
    if( inCreate() )
        return;

    RECT rectp, rectc;
    GetWindowRect(parent->hwnd(), &rectp);
    GetWindowRect( hWnd, &rectc );

    // find the differences in dimensions between the parent and client
    //
    int dw = ( rectp.right - rectp.left ) - ( rectc.right - rectc.left );
    int dh = ( rectp.bottom - rectp.top ) - ( rectc.bottom - rectc.top );

    // add the border around the display area
    //
    calculateSize( maxWidth, maxHeight );

    maxpWidth = maxWidth + dw;
    maxpHeight = maxHeight + dh;

    if( sizeparent ) {
        SetWindowPos( parent->hwnd(), 0, 0, 0, maxpWidth, maxpHeight
            , SWP_NOMOVE | SWP_NOZORDER );

    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::getCreateInfo( K_CREATEINFO* info )
{
    info->classname = KWinClassName;
#ifndef CKT_NT35_OR_31
    info->exStyle = WS_EX_CLIENTEDGE;
#endif
    info->style = WS_CHILD | WS_VISIBLE 
        | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::createWin( KWin* par )
{
    inCreate( TRUE );

    KWin::createWin( par );

#ifdef _WIN64
    WNDPROC _clientProc = (WNDPROC) SetWindowLongPtr(
            hWnd, GWLP_WNDPROC, (LONG_PTR) KClientWndProc );
#else /* _WIN64 */
    WNDPROC _clientProc = (WNDPROC) SetWindowLong( hWnd, GWL_WNDPROC
                    , (LONG)KClientWndProc );
#endif /* _WIN64 */

    // to make the updates look clean, use a memory DC for the drawing
    // and then BitBlt() it onto the screen DC.
    //
    _hdcScreen = GetDC( hWnd );
    _hdc = CreateCompatibleDC( _hdcScreen );
    compatBitmap = CreateCompatibleBitmap( _hdcScreen
        , kglob->sysMets->screenWidth()
        , kglob->sysMets->screenHeight() );
    _hdcScratch = CreateCompatibleDC( _hdcScreen );
    scratchBitmap = CreateCompatibleBitmap( _hdcScreen
        , kglob->sysMets->screenWidth()
        , kglob->sysMets->screenHeight() );

    SelectObject( _hdc, compatBitmap );
    SelectObject( _hdcScratch, scratchBitmap );

    HBITMAP bitmap = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP1) );
    disabledBrush = CreatePatternBrush( bitmap );
    DWORD rgb = cell_video_attr_background_rgb(geterasecolor(vmode));
    bgBrush = CreateSolidBrush( rgb );
    savebgcolor = rgb;

    DeleteObject( bitmap );

    clearPaintRgn();    // clear the paint region

    vert->createWin( this );
    horz->createWin( this );

    vert->setCallback( KWinMethodOf(KClient,vertScroll) );
    horz->setCallback( KWinMethodOf(KClient,horzScroll) );

    // set the default font to terminal (8x12)
    //
    KFont* defaultFont;
    if ( kglob->faceName[0] ) 
        defaultFont = new KFont(kglob->faceName, kglob->fontHeight,
                                 kglob->fontWidth);
    else
        defaultFont = new KFont();

    char * facename = defaultFont->getFaceName();
    // Lucida substs only take place if the font is Lucida Console
    win95lucida = !ckstrcmp(facename,"lucida console",-1,0);
    // HSL substs occur whenever the font does not support HSLs
    win95hsl = ckstrcmp(facename,"everson mono terminal",-1,0);

    setFont( defaultFont );

    int width, height;
    calculateSize( width, height );

    SetWindowPos( hWnd, 0, 0, 0, width, height, SWP_NOZORDER );

    /* This timer determines how frequently we attempt to update the screen */
    /* At some point this was every 10ms but in K95 2.1.3 at least it was fixed
     * to once every 100ms which wasn't very nice. Now the interval is
     * configurable via set terminal screen-update */
    startTimer();

    inCreate( FALSE );
}

/*------------------------------------------------------------------------
    clear the memory DC
------------------------------------------------------------------------*/
void KClient::clearPaintRgn()
{
    if( !hrgnPaint ) {
        hrgnPaint = CreateRectRgn( 0, 0
            , kglob->sysMets->screenWidth()
            , kglob->sysMets->screenHeight() );
    }

    PaintRgn( _hdc, hrgnPaint );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
long KClient::vertScroll( long val )
{
    if( val > vscrollpos ) {
        do {
            if (!scrollflag[vmode])
                break;
            ::scrollback( vmode /* clientID */, K_DNONE );
            val--;
        } while( val > vscrollpos );
    }
    else if( val >= 0 && val < vscrollpos ) {
        while( val < vscrollpos ) {
            ::scrollback( vmode /* clientID */, K_UPONE );
            val++;
        }
    }

    paint();
    return 0L;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
long KClient::horzScroll( long val )
{
    if( val > hscrollpos ) {
        do {
            if (!scrollflag[vmode])
                break;
            ::dokverb(vmode, K_RTONE );
            val--;
        } while( val > hscrollpos );
    }
    else if( val >= 0 && val < hscrollpos ) {
        while( val < hscrollpos ) {
            ::dokverb(vmode, K_LFONE );
            val++;
        }
    }
    paint();
    return 0L;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::size( int width, int height )
{
    SetWindowPos( hWnd, 0
        , 0, 0
        , width, height
        , SWP_NOMOVE | SWP_NOZORDER );

#if 0
    if( kglob->mouseEffect == TERM_MOUSE_NO_EFFECT ) {
        horz->setRange( maxWidth, width );

        int twid, thi;
        ::getDimensions( vmode /* clientID */, &twid, &thi );

        // determine if the horizontal scrollbar is visible
        //
        int horzh = horz->isVisible() ? kglob->sysMets->hscrollHeight() : 0;

        int truh = height - horzh
            - (2 * kglob->sysMets->edgeHeight()) - margin();
        vert->setRange( thi, truh / font->getFontSpacedH() );
    }
#endif
    paint();
}

/*------------------------------------------------------------------------
    change the dimensions of the terminal
	When doAnyway == FALSE, means the user was dragging the window
	to change the size.  If TRUE, means the system is changing
	the size (in writeMe() ).
------------------------------------------------------------------------*/
void KClient::endSizing( Bool doAnyway )
{
    if( kglob->mouseEffect != TERM_MOUSE_CHANGE_DIMENSION && !doAnyway )
        return;

    if( vmode /* clientID */ == VCS )
        return;
    if( vmode /* clientID */ == VSTATUS)
        return;

    int w, h;
    getEndSize( w, h );

    if (w == previousWidth && h == previousHeight
            && saveTermWidth == w && saveTermHeight == h){

        debug(F100, "endSizing: size not changed - doing nothing", "", 0);
        return;
    }

    previousWidth = w;
    previousHeight = h;

    if (kglob->mouseEffect == TERM_MOUSE_CHANGE_DIMENSION ) {
        kui_setheightwidth(w,h);
    } else if (kglob->mouseEffect == TERM_MOUSE_CHANGE_FONT ) {
        if ( w != saveTermWidth || h != saveTermHeight ) {
            saveTermWidth = saveTermHeight = 0;
        } else {
            kui_setheightwidth(w,h);
        }
    }
    vert->setValues( 1, h - tt_status[vmode]);
    ::VscrnIsDirty(vmode);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::getEndSize( int& w, int& h )
{
    if ( !font ) {
        h = w = 0;
        return;
    }

    RECT rect;
    GetWindowRect( hWnd, &rect );

    int width = ( rect.right - rect.left )
        - (vert->isVisible() ? kglob->sysMets->vscrollWidth() : 0)
        - 2 * kglob->sysMets->edgeWidth()
//        - 2 * kglob->sysMets->sizeframeWidth()
        - margin();

    // determine if the horizontal scrollbar is visible
    int horzh = horz->isVisible() ? kglob->sysMets->hscrollHeight() : 0;

    int height = ( rect.bottom - rect.top )
        - horzh
        - 2 * kglob->sysMets->edgeHeight()
//        - 2 * kglob->sysMets->sizeframeHeight()
        - margin();

    div_t divt = div( width, font->getFontW());
    w = divt.quot;
    if (divt.rem * 2 > font->getFontW())
        w ++;

    if( w < 20 )        // check for boundary conditions
        w = 20;
    else if( w > MAXSCRNCOL )
        w = MAXSCRNCOL;

    if( w % 2 )         // can't accept odd widths
        w -= 1;

    divt = div( height, font->getFontSpacedH());
    h = divt.quot;
    if (divt.rem * 2 > font->getFontSpacedH())
        h ++;

    if( h < 8 )         // check for boundary conditions
        h = 8;
    else if( h > MAXSCRNROW )
        h = MAXSCRNROW;
}

/*------------------------------------------------------------------------
    paint the window (not an update of data)
------------------------------------------------------------------------*/
Bool KClient::paint()
{
    clearPaintRgn();
    if (tt_sync_output && vmode == VTERM) {
        /* in synchronized output mode - keep re-rendering the existing display
         * until we exit synchronized output mode. */
        writeMe();
    } else {
        getDrawInfo();
    }
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::getDrawInfo()
{
    //debug(F100,"KClient::getDrawInfo()","",0);
    memset( workTemp, '\0', workTempSize );
    if( ikterm->getDrawInfo() )
        writeMe();
}

void KClient::ToggleCursor( HDC hdc, LPRECT lpRect )
{
    /* Draw the cursor in the scratch hdc rather than directly on the screen
     * like we used to - this makes it blink nicer. No more wiping in and out
     * from top to bottom. */
    BitBlt(_hdcScratch,
           lpRect->left, lpRect->top,
           lpRect->right, lpRect->bottom,
           hdc,
           lpRect->left, lpRect->top,
           SRCCOPY);
    for (int y = lpRect->top; y < lpRect->bottom; y++) {
        for ( int x = lpRect->left ; x < lpRect->right ; x++ ) {
            COLORREF color = GetPixel(_hdcScratch, x, y);
			int cursorbg = cell_video_attr_background_rgb(colorcursor);

            SetPixel(_hdcScratch, x, y, color^cursorbg);
        }
    }
    BitBlt(hdc,
           lpRect->left, lpRect->top,
           lpRect->right, lpRect->bottom,
           _hdcScratch,
           lpRect->left, lpRect->top,
           SRCCOPY);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::checkBlink()
{
    //debug(F100,"KClient::checkBlink()","",0);

    Bool blinkOn = FALSE;

    if( cursorCount < maxCursorCount ) {
        cursorCount += tt_update;
        if(cursorCount >= blinkInterval)
            blinkOn = TRUE;
    }
    else
        cursorCount = 0;

    /* ws_blinking indicates if the screen currently contains blinking elements
     * besides the cursor.*/
    if (ws_blinking && ((blinkOn && cursorCount == blinkInterval) || (cursorCount == 0)) )
        writeMe();

    else if (cursorCount%300 == 0) {
        if (ikterm->getCursorPos() && (_inFocus || (!_inFocus && cursor_displayed)))
        {
            int adjustedH = font->getFontH();
            if( tt_cursor == 2 )        // full
                ;
            else if( tt_cursor == 1 )   // half
                adjustedH /= 2;
            else                        // underline
                adjustedH = kglob->sysMets->borderHeight() + 1;

            cursorRect.left = clientPaint->cursorPt.x * font->getFontW()
                - _xoffset;
            cursorRect.top = clientPaint->cursorPt.y * font->getFontSpacedH()
                + ( font->getFontH() - adjustedH );
            cursorRect.right = cursorRect.left + font->getFontW();
            cursorRect.bottom = cursorRect.top + adjustedH;

            if ( cursorena[vmode] && cursor_on_visible_page(vmode) || markmodeflag[vmode] != notmarking ) {
                if ( !tt_cursor_blink ) {
                    if (!cursor_displayed && _inFocus || cursor_displayed && !_inFocus) {
                        ToggleCursor( hdcScreen(), &cursorRect );
                        cursor_displayed = _inFocus;
                    } 
                } else {
                    ToggleCursor( hdcScreen(), &cursorRect );
                    cursor_displayed = !cursor_displayed;

                }
            } else { /* cursor disabled */
                if (cursor_displayed) {
                    ToggleCursor( hdcScreen(), &cursorRect );
                    cursor_displayed = 0;
                }
            }
        }
    }
}

void KClient::syncSize()
{
    if (!IsZoomed(parent->hwnd()) || kglob->mouseEffect == TERM_MOUSE_NO_EFFECT )
        setDimensions( TRUE );

    if ( kglob->mouseEffect == TERM_MOUSE_CHANGE_FONT ) {
        RECT rect;
        if ( IsZoomed(parent->hwnd()) ) {
            GetWindowRect(GetDesktopWindow(), &rect);
        } else {
            GetWindowRect(parent->hwnd(), &rect);
        }
        parent->sizeFont(&rect, TRUE);
    } 
    if ( !IsZoomed(parent->hwnd()) || kglob->mouseEffect == TERM_MOUSE_NO_EFFECT )
        endSizing( TRUE );	// pass in TRUE to force processing
}

/*------------------------------------------------------------------------
    update the memory DC with all the drawing and then copy it 
    into the screen DC.  Also update the cursor position.
------------------------------------------------------------------------*/
void KClient::writeMe()
{
    int twid, thi, hisv;
    //debug(F100,"KClient::writeMe()","",0);

    ::getDimensions( vmode /* clientID */, &twid, &thi );
    hisv = horz->isVisible();

    if( thi != saveTermHeight || twid != saveTermWidth || 
        tt_linespacing[vmode] != saveLineSpacing || 
        hisv != saveHorzIsV)
    {
        //if ( deblog ) {
        //    char buf[128];
        //    sprintf(buf,"thi=%d(%d) twid=%d(%d) linespace=%f(%f)",thi,saveTermHeight,
        //             twid,saveTermWidth,tt_linespacing[vmode],saveLineSpacing);
        //    debug(F111,"KClient::writeMe() param change",buf,0);
        //}
        saveTermHeight = thi;
        saveTermWidth = twid;
        saveLineSpacing = tt_linespacing[vmode];
        saveHorzIsV = hisv;

        syncSize();
    }

    int w, h;
    getSize( w, h );

    // Erase the background with default color
    RECT r;
    r.left = 0;
    r.top = 0;
    r.right = w;
    r.bottom = h;

    DWORD rgb = cell_video_attr_background_rgb(geterasecolor(vmode));
    if ( rgb != savebgcolor ) {
        DeleteObject( bgBrush );
        bgBrush = CreateSolidBrush( rgb );
        savebgcolor = rgb;
    }
    FillRect( hdc(), &r, bgBrush); 

    // Then paint the data
    wc = 0;
    int xpos, i;
    int totlen = clientPaint->len;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(255);
    ushort lattr = ushort(-1);
    ushort effect = ushort(-1);
    for( i = 0; i < totlen; i++ )
    {
        xpos = i % twid;
        if( !xpos || !cell_video_attr_equal(attrBuffer[i], attr) || effectBuffer[i] != effect )
        {
            kws = &(workStore[wc]);

            kws->x = xpos * font->getFontW() - _xoffset;
            kws->y = (i / twid) * font->getFontSpacedH();

            kws->offset = i;
            if( wc )
                workStore[wc-1].length = i - workStore[wc-1].offset;
            attr = kws->attr = attrBuffer[i];
            effect = kws->effect = effectBuffer[i];
            wc++;
        }
    }
    if( wc )
        workStore[wc-1].length = i - workStore[wc-1].offset;

    Bool blinkOn = FALSE;
    Bool blink = FALSE;
    if( cursorCount < maxCursorCount ) {
        cursorCount += tt_update;
        if( cursorCount >= blinkInterval )
            blinkOn = TRUE;
    }
    else
        cursorCount = 0;

    if (cursorCount%300 == 0) {
        if (ikterm->getCursorPos() && (_inFocus || (!_inFocus && cursor_displayed)))
        {
            if ( cursorena[vmode] && cursor_on_visible_page(vmode) || markmodeflag[vmode] != notmarking) {
                if ( !tt_cursor_blink ) {
                    if (!cursor_displayed && _inFocus || cursor_displayed && !_inFocus) {
                        cursor_displayed = _inFocus;
                    } 
                } else {
                    cursor_displayed = !cursor_displayed;

                }
            } else { /* cursor disabled */
                cursor_displayed = 0;
            }
        }
    }

    RECT rect;
    for( i = 0; i < wc; i++ )
    {
        kws = &(workStore[i]);
        if( !cell_video_attr_equal(prevAttr, kws->attr) )
        {
            prevAttr = kws->attr;

            /* These are the default colors used by the console window, set by   */
            /* the SET GUI RGB commands and a few escape sequences               */
			SetBkColor( hdc(), cell_video_attr_background_rgb(prevAttr));
			SetTextColor( hdc(), cell_video_attr_foreground_rgb(prevAttr));
        }

        if( prevEffect != kws->effect )
        {
            prevEffect = kws->effect;
            Bool normal = (prevEffect == VT_CHAR_ATTR_NORMAL) ? TRUE : FALSE;
            Bool bold = truebold && ((prevEffect & VT_CHAR_ATTR_BOLD) ? TRUE : FALSE);
            Bool dim = truedim && ((prevEffect & VT_CHAR_ATTR_DIM) ? TRUE : FALSE);
            Bool underline = trueunderline && ((prevEffect & VT_CHAR_ATTR_UNDERLINE) ? TRUE : FALSE);
            Bool italic = trueitalic && ((prevEffect & VT_CHAR_ATTR_ITALIC) ? TRUE : FALSE);
			Bool crossedOut = truecrossedout && ((prevEffect & VT_CHAR_ATTR_CROSSEDOUT) ? TRUE : FALSE);
            blink = trueblink && ((prevEffect & VT_CHAR_ATTR_BLINK) ? TRUE : FALSE);

            if (decstglt == DECSTGLT_ALTERNATE) {
                // DECSTGLT says we should show attributes as colors. DECATCUM
                // and DECATCBM *may* say we should still do true underline and
                // true blink even while doing these as colors.
                bold = FALSE; dim = FALSE; italic = FALSE;

                underline = decatcum && ((prevEffect & VT_CHAR_ATTR_UNDERLINE) ? TRUE : FALSE);
                blink = decatcbm && ((prevEffect & VT_CHAR_ATTR_BLINK) ? TRUE : FALSE);

                normal = !underline && !blink;
            }

			if (dim) {
				// Cut the colours intensity by dividing each component by 2.
				// We can just quickly do this with a right-shift. Because there
			    // are three separate numbers packed in we need to mask out the
				// high bit of each as part of this so that the low bit of each
				// value to the left is erased.
				SetTextColor( hdc(),
					(cell_video_attr_foreground_rgb(prevAttr) >> 1) & 0x7F7F7F);
			}

            if( normal )
                getFont()->resetFont( hdc() );
            else if (crossedOut) {
                if( bold && underline && italic )
                    getFont()->setCrossedOutBoldUnderlineItalic( hdc() );
                else if( bold && underline )
                    getFont()->setCrossedOutBoldUnderline( hdc() );
                else if( underline && italic )
                    getFont()->setCrossedOutUnderlineItalic( hdc() );
                else if( bold && italic )
                    getFont()->setCrossedOutBoldItalic( hdc() );
                else if( bold )
                    getFont()->setCrossedOutBold( hdc() );
                else if( underline )
                    getFont()->setCrossedOutUnderline( hdc() );
                else if ( italic )
                    getFont()->setCrossedOutItalic( hdc() );
		    	else
                    getFont()->setCrossedOut( hdc() );
            }
            else if( bold && underline && italic )
                getFont()->setBoldUnderlineItalic( hdc() );
            else if( bold && underline )
                getFont()->setBoldUnderline( hdc() );
            else if( underline && italic )
                getFont()->setUnderlineItalic( hdc() );
            else if( bold && italic )
                getFont()->setBoldItalic( hdc() );
            else if( bold )
                getFont()->setBold( hdc() );
            else if( underline )
                getFont()->setUnderline( hdc() );
            else if ( italic )
                getFont()->setItalic( hdc() );
        }

        rect.left = kws->x;
        rect.top = kws->y;
        rect.right = rect.left + kws->length * font->getFontW();
        rect.bottom = rect.top + font->getFontSpacedH();

        if( rect.right == twid * font->getFontW() )
            rect.right += margin();

        if( rect.bottom == thi * font->getFontSpacedH() )
            rect.bottom += margin();

        if( !blink || blinkOn ) {
            ExtTextOutW( hdc(), rect.left, rect.top, 
			ETO_CLIPPED | ETO_OPAQUE, 
			&rect,
			(wchar_t*) &(textBuffer[ kws->offset ]),
			kws->length,
			(int*)&interSpace );
        }
        else {
            ExtTextOut( hdc(), rect.left, rect.top, 
			ETO_CLIPPED | ETO_OPAQUE, 
			&rect, 0, 0, 0 );
        }
    }

    for( i = 0; i < thi; i++ )
    {
        lattr = lineAttr[i];
        if( lattr == VT_LINE_ATTR_NORMAL )
            continue;

        Bool dblWide = lattr & VT_LINE_ATTR_DOUBLE_WIDE ? TRUE : FALSE;
        Bool dblHigh = lattr & VT_LINE_ATTR_DOUBLE_HIGH ? TRUE : FALSE;
        Bool upper = lattr & VT_LINE_ATTR_UPPER_HALF ? TRUE : FALSE;
        Bool lower = lattr & VT_LINE_ATTR_LOWER_HALF ? TRUE : FALSE;
    
        int destx = 0;
        int desty = i * font->getFontSpacedH();
        int destw = twid * font->getFontW();
        int desth = font->getFontSpacedH();

        int srcx = 0;
        int srcy = upper ? desty : (desty + font->getFontSpacedH()/2);
        int srcw = dblWide ? (destw/2) : destw;
        int srch = dblHigh ? (desth/2) : desth;
        if( dblWide && !dblHigh )
            srcy = desty;

        StretchBlt( _hdcScratch, destx, desty, destw, desth
            , hdc(), srcx, srcy, srcw, srch, SRCCOPY );

        StretchBlt( hdc(), destx, desty, destw, desth
            , _hdcScratch, destx, desty, destw, desth, SRCCOPY );

        if( i == thi - 1 ) {
            desty += font->getFontSpacedH();
            StretchBlt( _hdcScratch, 0, 0, destw, margin()
                , hdc(), 0, desty, srcw, margin(), SRCCOPY );

            StretchBlt( hdc(), 0, desty, destw, margin()
                , _hdcScratch, 0, 0, destw, margin(), SRCCOPY );
        }
    }


    if( clientPaint->cursorVisible && cursor_displayed && _inFocus) {
        int adjustedH = font->getFontH();
        if( tt_cursor == 2 )        // full
            ;
        else if( tt_cursor == 1 )   // half
            adjustedH /= 2;
        else                        // underline
            adjustedH = kglob->sysMets->borderHeight() + 1;

        cursorRect.left = clientPaint->cursorPt.x * font->getFontW() 
                - _xoffset;
        cursorRect.top = clientPaint->cursorPt.y * font->getFontSpacedH() 
                + ( font->getFontH() - adjustedH );
        cursorRect.right = cursorRect.left + font->getFontW();
        cursorRect.bottom = cursorRect.top + adjustedH;
        ToggleCursor( hdc(), &cursorRect );
        cursor_displayed = 1;
    } else
        cursor_displayed = 0;

    if( vmode /* clientID */ == VTERM && !::isConnected() )
        drawDisabledState( w, h );

#ifdef COMMENT
    if ( IsZoomed(parent->hwnd()) ) {
        int cx, cy, cw, ch;
        ((KAppWin *)parent)->getClientCoord( cx, cy, cw, ch );

        StretchBlt( hdcScreen(), 0, 0, cw, ch, hdc(), 0, 0, w, h, SRCCOPY );
    } else 
#endif /* COMMENT */
    {
        BitBlt( hdcScreen(), 0, 0, w, h, hdc(), 0, 0, SRCCOPY );
    }

    // adjust the vertical scrollbar
    //
    int max = (clientPaint->beg == 0) ? clientPaint->end + 1 : clientPaint->page_length;
    vert->setRange( max, thi - (tt_status[vmode]?1:0), FALSE );
    horz->setRange( clientPaint->maxWidth, VscrnGetWidth(vmode) );

    vscrollpos = max - (thi - (tt_status[vmode]?1:0));
    if( scrollflag[vmode /* clientID */] )
    {
        if( clientPaint->scrolltop >= clientPaint->beg ) {
            vscrollpos = clientPaint->scrolltop - clientPaint->beg;
        }
        else if( clientPaint->scrolltop < clientPaint->end ) {
            vscrollpos = tt_scrsize[vmode /* clientID */] - clientPaint->beg 
                    + clientPaint->scrolltop;
        }
    }

    vert->setPos( vscrollpos );
    hscrollpos = VscrnGetScrollHorz(vmode);
    horz->setPos( hscrollpos );

    // Determine if this screen contains any blinking data
    ws_blinking = 0;
    for( i = 0; i < wc; i++ ) {
        if( workStore[i].effect & VT_CHAR_ATTR_BLINK ) {
            ws_blinking = 1;
            break;
        }
    }
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Renders the specified vscrn to a device context for saving.
 *  - Does not render the cursor
 *  - All blinking elements are rendered as visible
 *  - Uses the current font and font size
 * Its based on a copy&paste of writeMe() with blinking and cursor related
 * bits removed and its own copy of IKTerm and the K_CLIENT_PAINT to grab
 * a single snapshot of the state of the vscrn.
 *
 * TODO: Someday writeMe() and this function should be refactored into a
 *       bunch of methods and/or classes to try and maximise code sharing.
 */
BOOL KClient::renderToDc(HDC hdc, KFont *font, int vnum, int margin) {
    int twid, thi;
    BOOL success = TRUE;

    // Initialise font
    font->setFont(hdc);

    // Get dimensions of the vscrn
    // Can't use ::getDimensions as that always returns the current vscrn
    twid = VscrnGetWidth(vnum);
    thi = VscrnGetDisplayHeight(vnum);

    // Get dimensions
    int w, h;
    h = thi * font->getFontSpacedH();
    w = twid * font->getFontW();

    // Erase the background with default color
    RECT r;
    r.left = 0;
    r.top = 0;
    r.right = w;
    r.bottom = h;

    DWORD rgb = cell_video_attr_background_rgb(geterasecolor(vnum));
    HBRUSH bgBrush = CreateSolidBrush( rgb );
    FillRect( hdc, &r, bgBrush);
    DeleteObject( bgBrush );

    // Grab a snapshot of the terminal screen
    long maxcells = ::getMaxDim();
    K_CLIENT_PAINT clientPaint;
    uchar *workBuf;
    allocateClientPaintBuffers(&clientPaint, maxcells, &workBuf);
    ushort* textBuffer            = clientPaint.textBuffer;
    cell_video_attr_t* attrBuffer = clientPaint.attrBuffer;
    ushort* effectBuffer          = clientPaint.effectBuffer;
    ushort* lineAttr              = clientPaint.lineAttr;

    K_WORK_STORE *workStore = new K_WORK_STORE[ maxcells ];
    memset( workStore, '\0', sizeof(K_WORK_STORE) * maxcells );

    IKTerm ikterm(vnum, &clientPaint);
    if (!ikterm.getDrawInfo(vnum)) {
        // TODO: Retry a few times in case we just couldn't get the mutex in time.
        delete workBuf;
        delete workStore;
        return FALSE;
    }

    // Collect data up into runs of text with the same attributes
    int xoffset = 0, yoffset = 0;  // Used by scrolling
    int wc = 0;
    int xpos, i;
    int totlen = clientPaint.len;
    _K_WORK_STORE* kws;

    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(255);
    ushort lattr = ushort(-1);
    ushort effect = ushort(-1);
    for( i = 0; i < totlen; i++ )
    {
        xpos = i % twid;
        if( !xpos || !cell_video_attr_equal(attrBuffer[i], attr) || effectBuffer[i] != effect )
        {
            kws = &(workStore[wc]);

            kws->x = xpos * font->getFontW() - xoffset;
            kws->y = (i / twid) * font->getFontSpacedH();

            kws->offset = i;
            if( wc )
                workStore[wc-1].length = i - workStore[wc-1].offset;
            attr = kws->attr = attrBuffer[i];
            effect = kws->effect = effectBuffer[i];
            wc++;
        }
    }
    if( wc )
        workStore[wc-1].length = i - workStore[wc-1].offset;

    int interSpace[MAXNUMCOL];
    int fontw = font->getFontW();
    for( i = 0; i < MAXNUMCOL; i++ )
        interSpace[i] = fontw;

    // Output text in runs with matching attributes
    RECT rect;
    cell_video_attr_t prevAttr = cell_video_attr_init_vio_attribute(255);
    ushort prevEffect = uchar(-1);
    BOOL blink;
    for( i = 0; i < wc; i++ )
    {
        kws = &(workStore[i]);
        if( !cell_video_attr_equal(prevAttr, kws->attr) )
        {
            prevAttr = kws->attr;

            /* These are the default colors used by the console window           */
            /* This needs to be replaced by a class that allows the color values */
            /* to be set by the user and stored somewhere.                       */
            /* The RGBTable is now set via SET GUI RGB commands.                 */
            SetBkColor( hdc, cell_video_attr_background_rgb(prevAttr));
            SetTextColor( hdc, cell_video_attr_foreground_rgb(prevAttr));
        }

        if( prevEffect != kws->effect )
        {
            prevEffect = kws->effect;
            Bool normal = (prevEffect == VT_CHAR_ATTR_NORMAL) ? TRUE : FALSE;
            Bool bold = truebold && ((prevEffect & VT_CHAR_ATTR_BOLD) ? TRUE : FALSE);
            Bool dim = truedim && ((prevEffect & VT_CHAR_ATTR_DIM) ? TRUE : FALSE);
            Bool underline = trueunderline && ((prevEffect & VT_CHAR_ATTR_UNDERLINE) ? TRUE : FALSE);
            Bool italic = trueitalic && ((prevEffect & VT_CHAR_ATTR_ITALIC) ? TRUE : FALSE);
			Bool crossedOut = truecrossedout && ((prevEffect & VT_CHAR_ATTR_CROSSEDOUT) ? TRUE : FALSE);
            blink = trueblink && ((prevEffect & VT_CHAR_ATTR_BLINK) ? TRUE : FALSE);

            if (decstglt == DECSTGLT_ALTERNATE) {
                // DECSTGLT says we should show attributes as colors. DECATCUM
                // and DECATCBM *may* say we should still do true underline and
                // true blink even while doing these as colors.
                bold = FALSE; dim = FALSE; italic = FALSE;

                underline = decatcum && ((prevEffect & VT_CHAR_ATTR_UNDERLINE) ? TRUE : FALSE);
                blink = decatcbm && ((prevEffect & VT_CHAR_ATTR_BLINK) ? TRUE : FALSE);

                normal = !underline && !blink;
            }

			if (dim) {
				// Cut the colours intensity by dividing each component by 2.
				// We can just quickly do this with a right-shift. Because there
			    // are three separate numbers packed in we need to mask out the
				// high bit of each as part of this so that the low bit of each
				// value to the left is erased.
				SetTextColor( hdc,
					(cell_video_attr_foreground_rgb(prevAttr) >> 1) & 0x7F7F7F);
			}

            if( normal )
                font->resetFont( hdc );
            else if (crossedOut) {
                if( bold && underline && italic )
                    font->setCrossedOutBoldUnderlineItalic( hdc );
                else if( bold && underline )
                    font->setCrossedOutBoldUnderline( hdc );
                /*else if( dim && underline && italic )
                    font->setCrossedOutDimUnderlineItalic( hdc );
                else if( dim && underline )
                    font->setCrossedOutDimUnderline( hdc );*/
                else if( underline && italic )
                    font->setCrossedOutUnderlineItalic( hdc );
                else if( bold && italic )
                    font->setCrossedOutBoldItalic( hdc );
                else if( bold )
                    font->setCrossedOutBold( hdc );
                /*else if( dim && italic )
                    font->setCrossedOutDimItalic( hdc );
                else if( dim )
                    font->setCrossedOutDim( hdc );*/
                else if( underline )
                    font->setCrossedOutUnderline( hdc );
                else if ( italic )
                    font->setCrossedOutItalic( hdc );
		    	else
                    font->setCrossedOut( hdc );
            }
            else if( bold && underline && italic )
                font->setBoldUnderlineItalic( hdc );
            else if( bold && underline )
                font->setBoldUnderline( hdc );
            /*else if( dim && underline && italic )
                font->setDimUnderlineItalic( hdc );
            else if( dim && underline )
                font->setDimUnderline( hdc );*/
            else if( underline && italic )
                font->setUnderlineItalic( hdc );
            else if( bold && italic )
                font->setBoldItalic( hdc );
            else if( bold )
                font->setBold( hdc );
            /*else if( dim && italic )
                font->setDimItalic( hdc );
            else if( dim )
                font->setDim( hdc );*/
            else if( underline )
                font->setUnderline( hdc );
            else if ( italic )
                font->setItalic( hdc );
        }

        rect.left = kws->x;
        rect.top = kws->y;
        rect.right = rect.left + kws->length * font->getFontW();
        rect.bottom = rect.top + font->getFontSpacedH();

        if( rect.right == twid * font->getFontW() )
            rect.right += margin;

        if( rect.bottom == thi * font->getFontSpacedH() )
            rect.bottom += margin;

        // This is where writeMe() decides whether or not to render blinking text.
        ExtTextOutW( hdc, rect.left, rect.top,
                     ETO_CLIPPED | ETO_OPAQUE,
                     &rect,
                     (wchar_t*) &(textBuffer[ kws->offset ]),
                     kws->length,
                     (int*)&interSpace );
    }

    // Stretch the contents of any double-height or double-wide lines to
    // double-height or double-wide size. The EMF output doesn't support doing
    // this.
    // TODO: Implement this using a different font size instead, and update the
    //       HELP SAVE content accordingly.
    if ((GetDeviceCaps(hdc, RASTERCAPS) & RC_STRETCHBLT) &&
            GetObjectType(hdc) != OBJ_ENHMETADC) {
        HDC hdcScratch = CreateCompatibleDC( hdc );
        HBITMAP scratchBitmap = CreateCompatibleBitmap( hdc, w, h );
        SelectObject( hdcScratch, scratchBitmap );
        for( i = 0; i < thi; i++ )
        {
            lattr = lineAttr[i];
            if( lattr == VT_LINE_ATTR_NORMAL )
                continue;

            Bool dblWide = lattr & VT_LINE_ATTR_DOUBLE_WIDE ? TRUE : FALSE;
            Bool dblHigh = lattr & VT_LINE_ATTR_DOUBLE_HIGH ? TRUE : FALSE;
            Bool upper = lattr & VT_LINE_ATTR_UPPER_HALF ? TRUE : FALSE;
            Bool lower = lattr & VT_LINE_ATTR_LOWER_HALF ? TRUE : FALSE;

            int destx = 0;
            int desty = i * font->getFontSpacedH();
            int destw = twid * font->getFontW();
            int desth = font->getFontSpacedH();

            int srcx = 0;
            int srcy = upper ? desty : (desty + font->getFontSpacedH()/2);
            int srcw = dblWide ? (destw/2) : destw;
            int srch = dblHigh ? (desth/2) : desth;
            if( dblWide && !dblHigh )
                srcy = desty;

            StretchBlt( hdcScratch, destx, desty, destw, desth
                , hdc, srcx, srcy, srcw, srch, SRCCOPY );

            StretchBlt( hdc, destx, desty, destw, desth
                , hdcScratch, destx, desty, destw, desth, SRCCOPY );

            if( i == thi - 1 ) {
                desty += font->getFontSpacedH();
                StretchBlt( hdcScratch, 0, 0, destw, margin
                    , hdc, 0, desty, srcw, margin, SRCCOPY );

                StretchBlt( hdc, 0, desty, destw, margin
                    , hdcScratch, 0, 0, destw, margin, SRCCOPY );
            }
        }
        DeleteDC( hdcScratch );
        DeleteObject( scratchBitmap );
    }

    success = GdiFlush();

    delete workBuf;
    delete workStore;

    return success;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Renders the the specified vscrn to a device-independent bitmap (DIB)
 * suitable for saving. Returns an HBITMAP, and the pixel data via
 * outPixels. If an error occurs, NULL is returned.
 */
HBITMAP KClient::renderToBitmap(int vnum, DWORD **outPixels) {
    KFont font(this->font->getLogFont());

    HDC hDC = GetDC(NULL);
    HDC hMDC = CreateCompatibleDC(hDC);

    font.setFont(hMDC); // Required for getFontW/getFontSpacedH to work

    int h = VscrnGetDisplayHeight(vnum) * font.getFontSpacedH();
    int w = VscrnGetWidth(vnum) * font.getFontW();

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    DWORD *pixels;
    HBITMAP hbmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, NULL);
    *outPixels = pixels;

    ReleaseDC(NULL, hDC);

    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMDC, hbmp);
    DeleteObject(hOldBmp);

    BOOL success = renderToDc(hMDC, &font, vnum, margin());

    DeleteObject(hMDC);

    if (!success) {
        DeleteObject(hbmp);
        hbmp = NULL;
    }

    return hbmp;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Renders the specified vscrn to an EMF file with the specified filename.
 * If an error occurs, FALSE is returned. Double Height/Double Wide lines are
 * rendered as Single Height/Single Wide due to limitations in GDIs EMF
 * recorder. */
BOOL KClient::renderToEmfFile(int vnum, char* filename) {

    HDC hdcEMF = CreateEnhMetaFile(NULL, filename, NULL,
        vnum == VTERM ? "Kermit-95\0Terminal Screen\0"
            : vnum == VCMD ?  "Kermit-95\0Command Screen\0"
                : "Kermit-95\0Other Screen\0");

    KFont font(this->font->getLogFont());

    BOOL success = renderToDc(hdcEMF, &font, vnum, margin());

    HENHMETAFILE hemf = CloseEnhMetaFile(hdcEMF);
    DeleteEnhMetaFile(hemf);

    return success;
}


#ifdef CK_HAVE_GDIPLUS
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Gets the Class Identifier for the specified GDI+ Image Encoder. This
 * comes from Microsoft sample code:
 *   https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
 */
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Renders to an image file using GDI+
 *    vnum        The vscrn to render
 *    filename    File to save to
 *    format      Image format, eg image/png
 * If an error occurs, FALSE is returned.
 */
BOOL KClient::renderToImageFile(int vnum, char* filename, const wchar_t* format) {

    // GDI+ wants the filename as a wide string.
    size_t newsize = strlen(filename) + 1;
    wchar_t* wcfilename = new wchar_t[newsize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcfilename, newsize, filename, _TRUNCATE);

    BOOL success = TRUE;
    Status  stat;

    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Render vscrn to a bitmap
    DWORD *pixels;
    HBITMAP hbmp = renderToBitmap(vnum, &pixels);

    // Convert to a GDI+ bitmap
    Bitmap * bmp = Bitmap::FromHBITMAP(hbmp, NULL);

    // Get the CLSID of the PNG encoder.
    CLSID   encoderClsid;
    if (GetEncoderClsid(format, &encoderClsid)) {
        // Save to PNG file
        stat = bmp->Save(wcfilename, &encoderClsid, NULL);

        if(stat != Ok) {
            printf("Failed to save image: stat = %d\n", stat);
            success = FALSE;
        }
    } else {
        printf("Failed to save image: could not get encoder\n");
        success = FALSE;
    }

    delete wcfilename;
    delete bmp;
    DeleteObject(hbmp);
    GdiplusShutdown(gdiplusToken);

    return success;
}

BOOL KClient::renderToPngFile(int vnum, char* filename) {

    return renderToImageFile(vnum, filename, L"image/png");
}

BOOL KClient::renderToGifFile(int vnum, char* filename) {
    return renderToImageFile(vnum, filename, L"image/gif");
}

#endif /* CK_HAVE_GDIPLUS */

/* GDI+ supposedly has a bitmap encoder which would be much less code than
 * all of the stuff below, but I couldn't get it to work in my brief attempt.
 * The code below would have still been required for when GDI+ isn't available
 * anyhow. */


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Creates a suitable BITMAPINFO struct for the specified HBITMAP. The
 * code comes from a Microsoft sample:
 *   https://learn.microsoft.com/en-us/windows/win32/gdi/storing-an-image
 */
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height.
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) {
        return NULL;
    }

    // Convert the color format to a count of bits.
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD
    // data structures.)

     if (cClrBits < 24)
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
                    sizeof(BITMAPINFOHEADER) +
                    sizeof(RGBQUAD) * (1<< cClrBits));

     // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel

     else
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
                    sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color
    // indices and store the result in biSizeImage.
    // The width must be DWORD aligned unless the bitmap is RLE
    // compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the
    // device colors are important.
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
 }

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
/* Renders the specified vscrn to a Windows device-independent bitmap
 * (.bmp or .dib) file with the specified name. If an error occurs, FALSE
 * is returned. */
BOOL KClient::renderToBmpFile(int vnum, char* filename) {
    DWORD *pixels;
    HBITMAP hbmp = renderToBitmap(vnum, &pixels);

    if (hbmp == NULL) {
        printf("Failed to render bitmap\n");
        return FALSE;
    }

    PBITMAPINFO pbi = NULL;
    PBITMAPINFOHEADER pbih = NULL;     // bitmap info-header
    DWORD dwTotal = 0;                 // total count of bytes
    DWORD cb = 0;                      // incremental count of bytes
    DWORD dwTmp = 0;
    int ret = 0;

    pbi = CreateBitmapInfoStruct(NULL, hbmp);
    if(pbi == NULL) {
        printf("Failed to create bitmap\n");
        DeleteObject(hbmp);
        return FALSE;
    }
    pbih = (PBITMAPINFOHEADER) pbi;

    HANDLE hf = CreateFile(filename,
                           GENERIC_READ | GENERIC_WRITE,
                           (DWORD) 0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           (HANDLE) NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        printf("Failed to create output file %s\n", filename);
        free(pbi);
        DeleteObject(hbmp);
        return FALSE;
    }

    BITMAPFILEHEADER hdr;
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
    // Compute the size of the entire file.
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +
                 pbih->biSize + pbih->biClrUsed
                 * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
                    pbih->biSize + pbih->biClrUsed
                    * sizeof (RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD) &dwTmp,  NULL)) {
        printf("Error writing to output file %s\n", filename);
        CloseHandle(hf);
        free(pbi);
        DeleteObject(hbmp);
        return FALSE;
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER)
                  + pbih->biClrUsed * sizeof (RGBQUAD),
                  (LPDWORD) &dwTmp, ( NULL))) {
        printf("Error writing to output file %s\n", filename);
        CloseHandle(hf);
        free(pbi);
        DeleteObject(hbmp);
        return FALSE;
    }

    // Copy the array of color indices into the .BMP file.
    dwTotal = cb = pbih->biSizeImage;

    if (!WriteFile(hf, (LPSTR) pixels, (int) cb, (LPDWORD) &dwTmp,NULL)) {
        printf("Error writing to output file %s\n", filename);
        CloseHandle(hf);
        free(pbi);
        DeleteObject(hbmp);
        return FALSE;
    }

    CloseHandle(hf);
    free(pbi);
    DeleteObject(hbmp);

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::drawDisabledState( int w, int h )
{
    RECT rect;
    rect.top = rect.left = 0;
    rect.right = w;
    rect.bottom = h;
    FillRect( _hdcScratch, &rect, disabledBrush );
    BitBlt( hdc(), 0, 0, w, h, _hdcScratch, 0, 0, SRCPAINT );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KClient::message( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool done = FALSE;
    _msgret = 1;
    //debug(F111,"KClient::message","msg",msg);
    switch( msg )
    {
    case WM_PAINT:
        {
            //debug(F111,"KClient::message","WM_PAINT",msg);
            PAINTSTRUCT ps;
            if (BeginPaint( hwnd, &ps ))
                paint();
            EndPaint( hwnd, &ps );
            done = TRUE;
            break;
        }

    case WM_ERASEBKGND:
        //debug(F111,"KClient::message","WM_ERASEBKGND",msg);
        done = TRUE;
        break;
        
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        debug(F111,"KClient::message","WM_{LRM}BUTTONDOWN (fall-through)",msg);
        takeFocus();
        // NOTE: FALL THROUGH !!!
        //

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_MOUSEMOVE:
        {
            debug(F111,"KClient::message","WM_{LRM}BUTTON{UP,DBLCLK},MOUSEMOVE",msg);
            short bitx = (short) LOWORD(lParam);
            short bity = (short) HIWORD(lParam);
            int x = bitx > 0 ? bitx / font->getFontW() : 0;
            int y = bity > 0 ? bity / font->getFontSpacedH() : 0;
            ikterm->mouseEvent( hwnd, msg, wParam, x, y );
            break;
        }

#ifdef WM_MOUSEWHEEL
    case WM_MOUSEWHEEL:
        {
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            ScreenToClient(hwnd, &pt);

            debug(F111,"KClient::message","WM_MOUSEWHEEL",msg);
            int x = pt.x > 0 ? pt.x / font->getFontW() : 0;
            int y = pt.y > 0 ? pt.y / font->getFontSpacedH() : 0;
            ikterm->mouseEvent( hwnd, msg, wParam, x, y );
            break;
        }
#endif

#ifdef COMMENT
    /* This is the obvious, simple and easy way to do it and this does work. But
     * I decided to instead pretend scrolling up one notch is one click of the
     * 4th mouse button, and scrolling down one notch is one click of the 5th
     * mouse button, plus some UI changes to treat buttons 4 and 5 specially
     * so you can't configure the click type (click, doubleclick, drag) for
     * them. This way scroll events can be mapped just like any button click.
     * Leaving this here in case the complicated way turns out to be broken
     * horribly.
     * */
    case WM_MOUSEWHEEL: {
          int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
          debug(F111, "WM_MOUSEWHEEL", "zDelta", zDelta);
          if (zDelta > 0) {
            do {
                //if (!scrollflag[vmode]) break;
                ::scrollback( vmode /* clientID */, K_UPONE );
                zDelta--;
            } while (zDelta > 0);
          } else {
            do {
              if (!scrollflag[vmode]) break;
              ::scrollback( vmode /* clientID */, K_DNONE );
              zDelta++;
            } while (zDelta < 0);
          }
          paint();
          done=1;
          break;
      }
#endif

    case WM_SETFOCUS:
        //debug(F111,"KClient::message","WM_SETFOCUS",msg);
        _inFocus = TRUE;
        ::dokverb(vmode, K_FOCUS_IN );
        break;

    case WM_KILLFOCUS:
        //debug(F111,"KClient::message","WM_KILLFOCUS",msg);
        _inFocus = FALSE;
        ::dokverb(vmode, K_FOCUS_OUT );
        break;

    case WM_VSCROLL:
        //debug(F111,"KClient::message","WM_VSCROLL",msg);
        done = vert->message( hwnd, msg, wParam, lParam );
        break;

    case WM_HSCROLL:
        //debug(F111,"KClient::message","WM_HSCROLL",msg);
        done = horz->message( hwnd, msg, wParam, lParam );
        break;

#ifdef COMMENT
    case WM_SYSKEYDOWN:     // needed for VK_F10
        //debug(F111,"KClient::message","WM_SYSKEYDOWN",msg);
        {
            // check the static array for VK_ key
            // VK_SCROLL is the last key in the array
            //
            if( wParam <= VK_SCROLL && keyArray[ wParam ] ) {
                done = ikterm->virtkeyEvent( wParam, lParam, TRUE );
                done = TRUE;
                //if( done )
                _msgret = 0;
                processKey = FALSE;
            }
            else
                processKey = TRUE;
            break;
            }

    case WM_KEYDOWN:
        //debug(F111,"KClient::message","WM_KEYDOWN",msg);
        {
            // check the static array for VK_ key
            // VK_SCROLL is the last key in the array
            //
            if( wParam <= VK_SCROLL && keyArray[ wParam ] ) {
                done = ikterm->virtkeyEvent( wParam, lParam, TRUE );
                done = TRUE;
                //if( done )
                _msgret = 0;
                processKey = FALSE;
            }
            else
                processKey = TRUE;
            break;
        }

    case WM_SYSDEADCHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_CHAR:
        //debug(F111,"KClient::message","WM_{[SYS],[DEAD]}CHAR",msg);
        {
            if( processKey ) {
                done = ikterm->keyboardEvent( wParam, lParam, TRUE );
                done = TRUE;
                //if( done )
                _msgret = 0;
                // processKey = 0;
            }
            break;
        }       

    case WM_SYSKEYUP:     // needed for VK_F10
    case WM_KEYUP:
        //debug(F111,"KClient::message","WM_[SYS]KEYUP",msg);
        {
            if( wParam <= VK_SCROLL && keyArray[ wParam ] )
                done = ikterm->virtkeyEvent( wParam, lParam, FALSE );
            else if ( processKey )
                done = ikterm->keyboardEvent( wParam, lParam, FALSE );
            done = TRUE;
            break;
        }
#else /* COMMENT */
    case WM_SYSKEYDOWN:     // needed for VK_F10
        //debug(F111,"KClient::message","WM_SYSKEYDOWN",msg);
        done = ikterm->newKeyboardEvent( (int)wParam, (long)lParam, TRUE, TRUE );
        break;
    case WM_KEYDOWN:
        //debug(F111,"KClient::message","WM_KEYDOWN",msg);
        done = ikterm->newKeyboardEvent( (int)wParam, (long)lParam, TRUE, FALSE );
        break;

    case WM_SYSKEYUP:     // needed for VK_F10
        //debug(F111,"KClient::message","WM_SYSKEYUP",msg);
        done = ikterm->newKeyboardEvent( (int)wParam, (long)lParam, FALSE, TRUE );
        break;
    case WM_KEYUP:
        //debug(F111,"KClient::message","WM_KEYUP",msg);
        done = ikterm->newKeyboardEvent( (int)wParam, (long)lParam, FALSE, FALSE );
        break;
#endif /* COMMENT */
    }
    return done;
}
