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

typedef struct _K_WORK_STORE {
    int             offset;
    int             length;
    int             x;
    int             y;
    unsigned char   attr;
    unsigned short  effect;
} K_WORK_STORE;

extern UINT keyArray [];    // from kuikey.cxx

extern "C" {
#define K_DNONE      110		/* Screen rollback: down one line */
#define K_UPONE      112		/* Screen rollback: Up one line */

extern int tt_cursor;
extern int cursorena[];
extern int tt_cursor_blink;
extern int tt_scrsize[];	/* Scrollback buffer size */
extern int tt_status[];
extern int scrollflag[];
extern int vmode;
extern int win32ScrollUp, win32ScrollDown;
extern int trueblink, trueunderline, trueitalic;
unsigned char geterasecolor(int);

extern DWORD VscrnClean( int vmode );
extern void scrollback( BYTE, int );
extern DWORD VscrnIsDirty( int );

ULONG RGBTable[16] = {
    0x00000000,
    0x00800000,                
    0x00008000,
    0x00808000,
    0x00000080,
    0x00800080,
    0x00008080,
    0x00c0c0c0,
    0x00808080,
    0x00ff0000,
    0x0000ff00,
    0x00ffff00,
    0x000000ff,
    0x00ff00ff,
    0x0000ffff,
    0x00ffffff
};

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
#define TIMER_PROC_MSEC 100
VOID CALLBACK KTimerProc( HWND hwnd, UINT msg, UINT id, DWORD dwtime )
{
    // debug(F111,"KTimerProc()","msg",msg);
    // debug(F111,"KTimerProc()","id",id);
    // debug(F111,"KTimerProc()","dwtime",dwtime);
    KClient* client = (KClient*) kglob->hwndset->find( hwnd );
    if( client ) {
        if( ::VscrnClean( vmode /* client->getClientID() */ ) )
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
    , prevAttr( uchar(-1) )
    , prevEffect( uchar(-1) )
    , _xoffset( 0 )
    , _yoffset( 0 )
    , _msgret( 1 )
    , ws_blinking( 0 )
    , cursor_displayed( 0 )
{
    vert = new KScroll( kg, TRUE, TRUE );
    horz = new KScroll( kg, FALSE, TRUE );

    // structure used to pass data to the paint function
    //
    clientPaint = new K_CLIENT_PAINT;
    memset( clientPaint, '\0', sizeof(K_CLIENT_PAINT) );

    // allocate the space in one chunck so it can be cleared in one call
    //
    long maxcells = ::getMaxDim();
    int column, row;
    ::getMaxSizes( &column, &row );
    workTempSize = (maxcells * sizeof(ushort) * 5) + (row * sizeof(ushort));
    workTemp = new uchar[ workTempSize ];
    memset( workTemp, '\0', workTempSize);

    clientPaint->textBuffer = textBuffer = (ushort*) &(workTemp[0]);
    clientPaint->attrBuffer = attrBuffer = &(workTemp[ maxcells * sizeof(ushort)]);
    clientPaint->effectBuffer = effectBuffer = (ushort*) &(workTemp[maxcells * (sizeof(uchar) + sizeof(ushort))]);
    clientPaint->lineAttr = lineAttr = (ushort*) &(workTemp[ maxcells * (sizeof(uchar) + 2 * sizeof(ushort))]);

    workStore = new K_WORK_STORE[ maxcells ];
    memset( workStore, '\0', sizeof(K_WORK_STORE) * maxcells );

    memset( &cursorRect, '\0', sizeof(RECT) );
    cursorCount = 0;

    maxCursorCount = 11;
    blinkInterval  = 6;

    ikterm = new IKTerm( vmode /* clientID */, clientPaint );
    wc = 0;
    vscrollpos = hscrollpos = 0;
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
    info->exStyle = WS_EX_CLIENTEDGE;
    info->style = WS_CHILD | WS_VISIBLE 
        | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::createWin( KWin* par )
{
    inCreate( TRUE );

    KWin::createWin( par );

    WNDPROC _clientProc = (WNDPROC) SetWindowLong( hWnd, GWL_WNDPROC
                    , (LONG)KClientWndProc );

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
    DWORD rgb = RGBTable[(geterasecolor(vmode)&0xF0)>>4];
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

    /* This constant determines how frequently we attempt to update the screen */
    /* The original value was 10 */
    timerID = SetTimer( hWnd, IDT_CLIENTTIMER, TIMER_PROC_MSEC, KTimerProc );

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

    int w, h;
    getEndSize( w, h );

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
    getDrawInfo();
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
    for (int y = lpRect->top; y < lpRect->bottom; y++) {
        for ( int x = lpRect->left ; x < lpRect->right ; x++ ) {
            COLORREF color = GetPixel(hdc, x, y);

            SetPixel(hdc, x, y, color^0x00808080);
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClient::checkBlink()
{
    //debug(F100,"KClient::checkBlink()","",0);

    Bool blinkOn = FALSE;
    if( cursorCount++ < maxCursorCount ) {
        if(cursorCount >= blinkInterval)
            blinkOn = TRUE;
    }
    else
        cursorCount = 0;

    if (ws_blinking && ((blinkOn && cursorCount == blinkInterval) || (cursorCount == 0)) )
        writeMe();

    else if (cursorCount%3 == 0) {
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

            if ( cursorena[vmode] ) {
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

    DWORD rgb = RGBTable[(geterasecolor(vmode)&0xF0)>>4];
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
    uchar attr = uchar(-1);
    ushort lattr = ushort(-1);
    ushort effect = ushort(-1);
    for( i = 0; i < totlen; i++ )
    {
        xpos = i % twid;
        if( !xpos || attrBuffer[i] != attr || effectBuffer[i] != effect )
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
    if( cursorCount++ < maxCursorCount ) {
        if( cursorCount >= blinkInterval )
            blinkOn = TRUE;
    }
    else
        cursorCount = 0;

    if (cursorCount%3 == 0) {
        if (ikterm->getCursorPos() && (_inFocus || (!_inFocus && cursor_displayed)))
        {
            if ( cursorena[vmode] ) {
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
        if( prevAttr != kws->attr )
        {
            prevAttr = kws->attr;

            /* These are the default colors used by the console window           */ 
            /* This needs to be replaced by a class that allows the color values */
            /* to be set by the user and stored somewhere.                       */
            /* The RGBTable is now set via SET GUI RGB commands.                 */
            SetBkColor( hdc(), RGBTable[(prevAttr&0xF0)>>4]);
            SetTextColor( hdc(), RGBTable[(prevAttr&0x0F)]);
        }

        if( prevEffect != kws->effect )
        {
            prevEffect = kws->effect;
            Bool normal = (prevEffect == VT_CHAR_ATTR_NORMAL) ? TRUE : FALSE;
            Bool bold = (prevEffect & VT_CHAR_ATTR_BOLD) ? TRUE : FALSE;
            Bool dim = (prevEffect & VT_CHAR_ATTR_DIM) ? TRUE : FALSE;
            Bool underline = trueunderline && ((prevEffect & VT_CHAR_ATTR_UNDERLINE) ? TRUE : FALSE);
            Bool italic = trueitalic && ((prevEffect & VT_CHAR_ATTR_ITALIC) ? TRUE : FALSE);
            blink = trueblink && ((prevEffect & VT_CHAR_ATTR_BLINK) ? TRUE : FALSE);

            if( normal )
                getFont()->resetFont( hdc() );
            else if( bold && underline && italic )
                getFont()->setBoldUnderlineItalic( hdc() );
            else if( bold && underline )
                getFont()->setBoldUnderline( hdc() );
            else if( dim && underline && italic )
                getFont()->setDimUnderlineItalic( hdc() );
            else if( dim && underline )
                getFont()->setDimUnderline( hdc() );
            else if( underline && italic )
                getFont()->setUnderlineItalic( hdc() );
            else if( bold && italic )
                getFont()->setBoldItalic( hdc() );
            else if( bold )
                getFont()->setBold( hdc() );
            else if( dim && italic )
                getFont()->setDimItalic( hdc() );
            else if( dim )
                getFont()->setDim( hdc() );
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
			(ushort*) &(textBuffer[ kws->offset ]),
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
    int max = (clientPaint->beg == 0) ? clientPaint->end + 1 : tt_scrsize[vmode];
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
Bool KClient::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
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

    case WM_SETFOCUS:
        //debug(F111,"KClient::message","WM_SETFOCUS",msg);
        _inFocus = TRUE;
        break;

    case WM_KILLFOCUS:
        //debug(F111,"KClient::message","WM_KILLFOCUS",msg);
        _inFocus = FALSE;
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
        done = ikterm->newKeyboardEvent( wParam, lParam, TRUE, TRUE );
        break;
    case WM_KEYDOWN:
        //debug(F111,"KClient::message","WM_KEYDOWN",msg);
        done = ikterm->newKeyboardEvent( wParam, lParam, TRUE, FALSE );
        break;

    case WM_SYSKEYUP:     // needed for VK_F10
        //debug(F111,"KClient::message","WM_SYSKEYUP",msg);
        done = ikterm->newKeyboardEvent( wParam, lParam, FALSE, TRUE );
        break;
    case WM_KEYUP:
        //debug(F111,"KClient::message","WM_KEYUP",msg);
        done = ikterm->newKeyboardEvent( wParam, lParam, FALSE, FALSE );
        break;
#endif /* COMMENT */
    }
    return done;
}
