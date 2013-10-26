#include "kstatus.hxx"
#include "khwndset.hxx"
#include "ksysmets.hxx"
#include "kcustdlg.hxx"
#include "karray.hxx"

typedef struct _paneInfo {
    long width;
    char text[128];
} paneInfo;

extern "C" {

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
LRESULT CALLBACK KStatusProc( HWND hWnd, UINT umsg, WPARAM wparam, LPARAM lparam )
{
    KStatus* win = (KStatus*) kglob->hwndset->find( hWnd );
    if( !win )
        return 0L;

    Bool ret = win->message( hWnd, umsg, wparam, lparam );
    if( !ret )
        return CallWindowProc( win->getdefproc(), hWnd, umsg, wparam, lparam );
    return 1;
}

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KStatus::KStatus( K_GLOBAL* kg )
    : KWin( kg )
    , numParts( 0 )
    , defproc( 0 )
    , partDrag( -1 )
    , mouseOverPart( FALSE )
    , _customize( FALSE )
    , custdlg( 0 )
    , customHwnd( 0 )
    , custXOffset( 0 )
    , custXWidth( 0 )
    , whichPane( -1 )
{
    paneArray = new KArray();
    offsets = 0;
    statusWidths = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KStatus::~KStatus()
{
    if( customHwnd )
        DestroyWindow( customHwnd );

    delete custdlg;

    delete [] offsets;
    delete [] statusWidths;

    paneInfo* pi;
    for( long i = 0; i < numParts; i++ )
    {
        pi = (paneInfo*) paneArray->get( i );
        delete pi;
    }
    delete paneArray;
}

/*------------------------------------------------------------------------
    virtual callback used by createWin() to retrieve window info
------------------------------------------------------------------------*/
void KStatus::getCreateInfo( K_CREATEINFO* info )
{
    info->classname = STATUSCLASSNAME;
    info->objId = ::GetGlobalID();
    info->style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE
            | SBS_SIZEGRIP | SBT_NOBORDERS; // | WS_BORDER | CCS_BOTTOM;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::createWin( KWin* par )
{
    KWin::createWin( par );

    // subclass the statusbar
    //
    defproc = (WNDPROC) SetWindowLong( hWnd, GWL_WNDPROC, (LONG)KStatusProc );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::setParts( int num, int* partOffsets )
{
    numParts = num;
    offsets = new int[numParts];
    statusWidths = new int[numParts];

    paneInfo* pi;
    for( int i = 0; i < numParts; i++ )
    {
        pi = new paneInfo;
        memset( pi, '\0', sizeof(paneInfo) );
        offsets[i] = partOffsets[i];
        paneArray->put( (void*)pi, i );
    }

    // initialize the number of parts
    //
    SendMessage( hWnd, SB_SETPARTS, numParts, (LPARAM)statusWidths );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::size( int width, int height )
{
    // Resize status bar to fit along bottom of the client area.
    int w, h;
    getSize( w, h );

    long style = SWP_NOZORDER;
    if( width == w ) {
        style |= SWP_NOSIZE;
    }

    SetWindowPos( hWnd, 0, 0, height - h, width, h, style );

    long wid = 0L;
    paneInfo* pi;
    int val;
    for( int i = 0; i < numParts; i++ )
    {
        pi = (paneInfo*) paneArray->get( i );

        val = width - offsets[i];
        val = val > 0 ? val : 0;
        if( i == numParts - 1 )
            val -= kglob->sysMets->vscrollWidth();

        statusWidths[i] = val;

        if( !i )
            wid = statusWidths[0];
        else
            wid = statusWidths[i] - statusWidths[i-1];

        pi->width = wid;
    }

    SendMessage( hWnd, SB_SETPARTS, numParts, (LPARAM)statusWidths );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::setText( int part, char* text )
{
    paneInfo* pi = (paneInfo*) paneArray->get( part );
    strcpy( pi->text, text );
    PostMessage( hWnd, SB_SETTEXT, part, (LPARAM)pi->text );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::customize()
{
    if( _customize )
        return;

    _customize = TRUE;

    if( !customHwnd )
    {
        // create the window that's used to represent the draggable
        // pane window
        //
        int w, h;
        getSize( w, h );
        customHwnd = CreateWindowEx( 0
                     , STATUSCLASSNAME
                     , NULL
                     , WS_CHILD | WS_CLIPSIBLINGS | CCS_NORESIZE
                     , 0, 0
                     , w, h
                     , hWnd
                     , (HMENU) ::GetGlobalID()
                     , kglob->hInst
                     , (LPVOID) 0 );
        SendMessage( customHwnd, SB_SETTEXT, SBT_POPOUT, (LPARAM)NULL );
    }

    if( custdlg )
        delete custdlg;

    // create the dialog box used to store unused pane windows
    //
    custdlg = new KStatusCustomDlg( kglob, this );
    custdlg->createWin( parent );
    custdlg->show();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KStatus::endCustomize()
{
    _customize = FALSE;
    delete custdlg;
    custdlg = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::mouseMove( long wParam, long x, long y )
{
    int w, h;
    getSize( w, h );
    if( wParam & MK_LBUTTON )   // left mouse button down
    {
        if( partDrag != -1 )
        {
            // partition drag operation (resizing pane windows)
            // adjust the size of the panes
            //
            int ew = kglob->sysMets->edgeWidth();
            int max = partDrag < numParts - 2 ? w - offsets[partDrag+1] - 2 * ew 
                : w - kglob->sysMets->vscrollWidth() - 2 * ew;

            int min = partDrag > 0 ? w - offsets[partDrag-1] + 2 * ew: 2 * ew;
            if( x > max )
                x = max;
            else if( x < min )
                x = min;

            statusWidths[partDrag] = x;
            offsets[partDrag] = w - x;
            SendMessage( hWnd, SB_SETPARTS, numParts, (LPARAM)statusWidths );
        }
        else if( whichPane != -1 )
        {
            // move operation of pane window (y-axis constant)
            //
            long start = x - custXOffset;
            
            if( start < 0 )
                start = 0;
            else if( start + custXWidth > w )
                start = w - custXWidth;

            SetWindowPos( customHwnd, HWND_TOP, start, 0, 0, 0
                , SWP_SHOWWINDOW | SWP_NOSIZE );
        }
    }
    else
    {
        // determine if the cursor is over a partition
        //
        mouseOverPart = FALSE;
        int checkx;
        int edge = kglob->sysMets->edgeWidth();
        int borderw = kglob->sysMets->borderWidth();
        for( int i = 0; i < numParts - 1; i++ )
        {
            checkx = statusWidths[i] + borderw;
            if( checkx > x - edge && checkx < x + 2 * edge ) {
                SetCursor( LoadCursor( kglob->hInst
                    , MAKEINTRESOURCE(IDC_CURSORPARTDRAG) ) );
                mouseOverPart = TRUE;
                break;  // from for() loop
            }
        }
    }

    return FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::lButtonDown( long wParam, long x, long y )
{
    SetCapture( hWnd );

    // determine if the cursor is over a partition (sizing pane window)
    //
    int checkx;
    int edge = kglob->sysMets->edgeWidth();
    int borderw = kglob->sysMets->borderWidth();
    for( int i = 0; i < numParts - 1; i++ )
    {
        checkx = statusWidths[i] + borderw;
        if( checkx > x - edge && checkx < x + 2 * edge ) {
            partDrag = i;
            break;  // from for() loop
        }
    }

    // if its not over a partition, it may be over a pane
    //
    if( partDrag == -1 )
        findDragPane( wParam, x, y );

    return FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::lButtonUp( long wParam, long x, long y )
{
    ReleaseCapture();
    paneInfo* pi;
    int w, h;
    getSize( w, h );

    ShowWindow( customHwnd, SW_HIDE );
    if( partDrag != -1 ) {
        partDrag = -1;
    }
    else if( whichPane != -1 ) {
        // find where the new pane goes
        //
        long newpos = whichPane;
        long start = x - custXOffset;
        if( start < 0 )
            start = 0;
        else if( start + custXWidth > w )
            start = w - custXWidth;
        long comp;
        for( int i = 0; i <= numParts; i++ )
        {
            if( !i )
                comp = 0;
            else
                comp = statusWidths[i-1];

            if( start <= comp )
            {
                newpos = i;
                if( whichPane < i ) {
                    newpos--;
                }
                else if( whichPane == i ) {
                    if( start <= statusWidths[i] && start >= comp ) {
                        // same position... to the right
                        newpos = whichPane;
                    }
                    else if( start <= comp ) {
                        int secondcomp;
                        if( i <= 1 )
                            secondcomp = 0;
                        else
                            secondcomp = statusWidths[i-2];

                        if( start > secondcomp ) {
                            // same position... to the left
                            newpos = whichPane;
                        }
                    }
                }
                break;
            }
        }

        
        if( newpos != whichPane )
        {
            // reorder the status panes
            //
            paneInfo* pisave = (paneInfo*) paneArray->remove( whichPane );
            paneArray->insert( (void*)pisave, newpos );
        }

        for(int i = 0; i < numParts; i++ )
        {
            pi = (paneInfo*) paneArray->get( i );
            if( !i )
                statusWidths[0] = pi->width;
            else
                statusWidths[i] = pi->width + statusWidths[i-1];
            SendMessage( hWnd, SB_SETTEXT, i, (LPARAM)pi->text );
        }

        whichPane = -1;
    }


    // redraw the status bar
    //
    long wid;
    for( int i = 0; i < numParts; i++ )
    {
        if( !i )
            wid = statusWidths[0];
        else
            wid = statusWidths[i] - statusWidths[i-1];
        pi = (paneInfo*) paneArray->get( i );
        pi->width = wid;

        if( i == numParts - 1 )
            offsets[i] = 0;
        else
            offsets[i] = w - statusWidths[i];

    }
    SendMessage( hWnd, SB_SETPARTS, numParts, (LPARAM)statusWidths );

    return FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::rButtonDown( long wParam, long x, long y )
{
    if( numParts <= 0 ) // nothing more can be moved from the status bar
        return FALSE;

    SetCursor( LoadCursor( NULL, IDC_ARROW ) );

    findDragPane( wParam, x, y );

    // find the offsets from the mouse pointer
    //
    int off = -kglob->sysMets->edgeWidth();
    if( whichPane > 0 )
        off = statusWidths[whichPane-1];
    int xoff = x - off - 2 * kglob->sysMets->edgeWidth();
    int yoff = y - 2 * kglob->sysMets->edgeHeight();

    // copy the pane's text into the bugger
    //
    paneInfo* pi = (paneInfo*)paneArray->get( whichPane );
    strcpy( custdlg->getDragPane()->text, pi->text );

    custdlg->beginPaneMove( x, y, xoff, yoff );
    return FALSE;
}

/*------------------------------------------------------------------------
    check to see if the mouse is over a pane
------------------------------------------------------------------------*/
Bool KStatus::findDragPane( long wParam, long x, long y )
{
    int edge = kglob->sysMets->edgeWidth();
    int comp;
    for( int i = numParts - 1; i >= 0; i-- )
    {
        comp = i ? statusWidths[i-1]: 0;
        if( x > comp )
        {
            // setup the draggable pane window
            //
            int off = 0;
            if( i > 0 )
                off = statusWidths[i-1];
            custXOffset = x - off - edge;
            custXWidth = statusWidths[i] - off - edge;
            if( i == 0 ) {
                custXWidth += edge;
                custXOffset += edge;
            }
            whichPane = i;

            char buf[256];
            SendMessage( hWnd, SB_GETTEXT, i, (LPARAM)buf );

            // set the proper text and make the pane appear raised
            //
            SendMessage( customHwnd, SB_SETTEXT, SBT_POPOUT, (LPARAM)buf );

            // resize it to the proper dimensions
            //
            int w, h;
            getSize( w, h );
            long start = x - custXOffset;
            SetWindowPos( customHwnd, HWND_TOP, start, 0, custXWidth, h
            , SWP_SHOWWINDOW );

            // OWNERDRAW message WM_DRAWITEM gets sent through kappwin
            //
            SendMessage( hWnd, SB_SETTEXT, i | SBT_OWNERDRAW, (LPARAM)buf );
            return TRUE;
        }
    }
    return FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::addPane( long xpos )
{
    // need to dynamically resize (all?) the panels to make
    // the addition fit
    //
    int newPane = numParts;
    Bool adjusted = FALSE;
    int i;
    int ew = kglob->sysMets->edgeWidth();
    int iconw = kglob->sysMets->iconWidth();
    paneInfo* pi;

    // locate the proper position for the new pane
    //
    int comp;
    for( i = 0; i < numParts; i++ )
    {
        comp = i ? statusWidths[i-1] : 0;
        if( xpos < comp ) {
            newPane = i;
            break;
        }
    }

    // look for space to the right
    //
    for( i = newPane; (i < numParts) && !adjusted; i++ )
    {
        pi = (paneInfo*) paneArray->get( i );
        if( pi->width - iconw >= iconw )
        {
            adjusted = TRUE;
            pi->width -= iconw;
        }
    }

    if( !adjusted ) {   // cannot add any more panes to the right
        // look for space to the left
        //
        for( i = newPane - 1; i >= 0 && !adjusted; i-- )
        {
            pi = (paneInfo*) paneArray->get( i );
            if( pi->width - iconw >= iconw )
            {
                adjusted = TRUE;
                pi->width -= iconw;
            }
        }
    }

    if( !adjusted && numParts ) // cannot add any more panes to the left or right
        return FALSE;

    numParts++;
    pi = new paneInfo;
    pi->width = iconw;
    strcpy( pi->text, custdlg->getDragPane()->text );
    paneArray->insert( (void*)pi, newPane );

    Bool done = TRUE;

    int w, h;
    getSize( w, h );

    if( numParts == 1 )
        pi->width = w - kglob->sysMets->vscrollWidth();

    // recalculate all the offsets
    //
    int* tmp = new int[numParts];
    int* tmp2 = new int[numParts];
    for( i = 0; i < numParts; i++ ) {
        pi = (paneInfo*) paneArray->get( i );
        if( i > 0 ) {
            tmp[i] = tmp[i-1] + pi->width;
            tmp2[i] = w - (tmp[i-1] + pi->width);
        }
        else {
            tmp[i] = pi->width;
            tmp2[i] = w - pi->width;
        }
    }

    delete [] statusWidths;
    delete [] offsets;
    statusWidths = tmp;
    offsets = tmp2;

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::removePane()
{
    if( numParts <= 0 )
        return FALSE;

    Bool done = TRUE;
    paneInfo* pi;

    int w, h;
    getSize( w, h );

    paneInfo* pisave = (paneInfo*) paneArray->remove( whichPane );
    numParts--;
    if( numParts ) {
        pi = (paneInfo*) paneArray->get( whichPane ? whichPane - 1 : 0 );
        pi->width += pisave->width;
    }

    for( int i = 0; i < numParts; i++ ) {
        pi = (paneInfo*) paneArray->get( i );
        if( i > 0 ) {
            statusWidths[i] = statusWidths[i-1] + pi->width;
            offsets[i] = w - (offsets[i-1] + pi->width);
        }
        else {
            statusWidths[i] = pi->width;
            offsets[i] = w - pi->width;
        }
    }

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::endPaneMove( eAction action, long xpos )
{
    Bool done = TRUE;

    switch( action )
    {
        case ADDPANE:
            done = addPane( xpos );
            break;

        case REMOVEPANE:
            done = removePane();
            break;

        case NOACTION:
        default:
            break;
    }

    if( numParts ) {
        SendMessage( hWnd, SB_SETPARTS, numParts, (LPARAM)statusWidths );
        offsets[numParts-1] = 0;
    }
    else {
        SendMessage( hWnd, SB_SETPARTS, 0, (LPARAM)(-1) );
        SendMessage( hWnd, SB_SETTEXT, SBT_NOBORDERS, (LPARAM)NULL );
        InvalidateRect( hWnd, NULL, TRUE );
        offsets[0] = 0;
    }

    paneInfo* pi;
    for( int i = 0; i < numParts; i++ )
    {
        pi = (paneInfo*) paneArray->get( i );
        SendMessage( hWnd, SB_SETTEXT, i, (LPARAM)pi->text );
    }

    whichPane = -1;
    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KStatus::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool ret = FALSE;
    switch( msg )
    {    
        case WM_MOUSEMOVE:
            if( _customize )
                ret = mouseMove( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;

        case WM_LBUTTONDOWN:
            if( _customize )
                ret = lButtonDown( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;

        case WM_LBUTTONUP:
            if( _customize )
                ret = lButtonUp( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;

        case WM_RBUTTONDOWN:
            if( _customize )
                ret = rButtonDown( wParam, (short)LOWORD(lParam), (short)HIWORD(lParam) );
            break;

        case WM_SETCURSOR:
            if( partDrag != -1 || mouseOverPart )
                ret = TRUE;
            break;

        case WM_ERASEBKGND:
            break;

        case WM_DRAWITEM:
        {
            // these messages get sent by kappwin (OWNERDRAW)
            //
            LPDRAWITEMSTRUCT draws = (LPDRAWITEMSTRUCT) lParam;
            FillRect( draws->hDC, &draws->rcItem, (HBRUSH) GetStockObject( WHITE_BRUSH ) );
            ret = TRUE;
            break;
        }

        default:
            break;
    }

    return ret;
}

